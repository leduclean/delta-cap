#include <assert.h>
#include <errno.h>

#include "camera.h"
#include "cmsis_os2.h"
#include "cmw_camera.h"
#include "display.h"
#include "stm32n6xx_it.h"

/**
 * @brief DCMIPP prepocessing conf for the screen pipe.
 */
static CMW_DCMIPP_Conf_t screen_conf = {
	.output_width = SCREEN_WIDTH,
	.output_height = SCREEN_HEIGHT,
	.output_format = DCMIPP_PIXEL_PACKER_FORMAT_RGB565_1,
	.output_bpp = 2,

	.enable_gamma_conversion = 0,

	.mode = CMW_Aspect_ratio_fullscreen,
};

static int _setup_pipe(void)
{
	int res;

	uint32_t pitch;
	res = CMW_CAMERA_SetPipeConfig(DCMIPP_PIPE1, &screen_conf, &pitch);
	if (res < 0) {
		return res;
	}
	assert(screen_conf.output_width * screen_conf.output_bpp == pitch);

	return pitch;
}

/**
 * @brief DCMIPP prepocessing conf for the nn pipe.
 */
static CMW_DCMIPP_Conf_t nn_conf = {
	.output_width = NN_WIDTH,
	.output_height = NN_HEIGHT,
	.output_format = DCMIPP_PIXEL_PACKER_FORMAT_RGB888_YUV444_1,

	.output_bpp = 3,
	.enable_swap = 1, /* RGB */
	.enable_gamma_conversion = 0,

	.mode = CMW_Aspect_ratio_crop,

};

static int _setup_nn_pipe(void)
{
	int res;

	uint32_t pitch;
	res = CMW_CAMERA_SetPipeConfig(DCMIPP_PIPE2, &nn_conf, &pitch);
	if (res < 0) {
		return res;
	}

	if (pitch != NN_WIDTH * NN_BPP) {
		return -EINVAL;
	}

	return 0;
}

static CMW_CameraInit_t cam_conf = {
	.fps = CAMERA_FPS,
	.mirror_flip = CAMERA_FLIP,
};

int camera_init()
{
	int res;

	res = CMW_CAMERA_Init(&cam_conf, NULL);
	if (res < 0) {
		return res;
	}

	res = _setup_pipe();
	if (res < 0) {
		return res;
	}

	res = _setup_nn_pipe();
	if (res < 0) {
		return res;
	}

	return 0;
}

static osThreadId_t nn_waiter;
static osThreadId_t screen_waiter;

int camera_start_screen_pipe(uint8_t *pbuff)
{
	screen_waiter = osThreadGetId();
	return CMW_CAMERA_Start(DCMIPP_PIPE1, pbuff, CMW_MODE_CONTINUOUS);
}

/**
 * @brief Double buffer ai entries filled by the camera driver.
 */
alignas(32) static uint8_t nn_buf[2][NN_FRAME_SIZE];

/**
 * @brief Flag indicating the current ready double buffer index.
 */
static volatile uint8_t nn_ready;

int camera_start_nn_pipe(void)
{
	nn_waiter = osThreadGetId();
	return CMW_CAMERA_DoubleBufferStart(DCMIPP_PIPE2, nn_buf[0], nn_buf[1],
					    CMW_MODE_CONTINUOUS);
}

int camera_run(void)
{
	return CMW_CAMERA_Run();
}

/**
 * @name Timeouts for blocking function waiting for an available frame.
 * @{
 */

#define FRAME_TIMEOUT (5 * osKernelGetTickFreq() / CAMERA_FPS) /**< 5 frame. */
#define NN_FRAME_TIMEOUT (FRAME_TIMEOUT / 2) /**< Double buffering timeout. */
/** @} */

#define CAM_FLAG_FRAME 0x1U

static int wait_frame(uint32_t timeout)
{
	if (osThreadFlagsWait(CAM_FLAG_FRAME, osFlagsWaitAny, timeout) &
	    osFlagsError) {
		return -ETIMEDOUT;
	}

	return 0;
}

int camera_wait_screen_frame(void)
{
	return wait_frame(FRAME_TIMEOUT);
}

uint8_t *camera_get_nn_frame(void)
{
	int res = wait_frame(NN_FRAME_TIMEOUT);
	if (res < 0) {
		return NULL;
	}
	return nn_buf[nn_ready];
}

/**
 * @brief Function called by the IRQ handler of the DCMIPP.
 *
 * @param pipe The pipe that produced the frame.
 *
 * @note This function overrides the default __weak of the CMW_CAMERA 
 * lib.
 */
int CMW_CAMERA_PIPE_FrameEventCallback(uint32_t pipe)
{
	switch (pipe) {
	case DCMIPP_PIPE1:
		osThreadFlagsSet(screen_waiter, CAM_FLAG_FRAME);
		break;
	case DCMIPP_PIPE2:
		nn_ready ^= 1;
		osThreadFlagsSet(nn_waiter, CAM_FLAG_FRAME);
		break;
	default:
		break;
	}

	return 0;
}

/*
 * A pipe error is an overrun: happens when a DCMIPP couldn't write the frame to RAM in time,
 * typically while the NPU saturates the AXI bus during an inference. 
 *
 * We just assume this is not a problem, the frame is lost, the next one is fine.
 *
 * @note This function overrides the default __weak that just assert(0).
 */
void CMW_CAMERA_PIPE_ErrorCallback([[maybe_unused]] uint32_t pipe)
{
	/** The frame is dropped. */
}

/**
 * @brief CSI interrupt handler, called from the vector table.
 * @note This function overrides the default __weak implem declared in 
 * fsbl ASM startup.
 */
void CSI_IRQHandler(void)
{
	DCMIPP_HandleTypeDef *hcamera_dcmipp = CMW_CAMERA_GetDCMIPPHandle();
	HAL_DCMIPP_CSI_IRQHandler(hcamera_dcmipp);
}

/**
 * @brief DCMIPP interrupt handler, called from the vector table.
 * @note This function overrides the default __weak implem declared in 
 * fsbl ASM startup.
 */
void DCMIPP_IRQHandler(void)
{
	DCMIPP_HandleTypeDef *hcamera_dcmipp = CMW_CAMERA_GetDCMIPPHandle();
	HAL_DCMIPP_IRQHandler(hcamera_dcmipp);
}

static const RCC_PeriphCLKInitTypeDef dcmipp_clk_conf = {
 .PeriphClockSelection = RCC_PERIPHCLK_DCMIPP,
 .DcmippClockSelection = RCC_DCMIPPCLKSOURCE_IC17,
 .ICSelection[RCC_IC17] = {
 .ClockSelection = RCC_ICCLKSOURCE_PLL2,
 .ClockDivider = 3,
 },
};

static const RCC_PeriphCLKInitTypeDef csi_clk_conf = {
 .PeriphClockSelection = RCC_PERIPHCLK_CSI,
 .ICSelection[RCC_IC18] = {
 .ClockSelection = RCC_ICCLKSOURCE_PLL1,
 .ClockDivider = 40,
 },
};

/**
 * @brief Setup the clock of DCMIPP and the csi.
 * @note This function overrides a __weak stub of the CMW_CAMERA lib.
 * And is called in @p CMW_CAMERA_Init().
*/
HAL_StatusTypeDef
MX_DCMIPP_ClockConfig([[maybe_unused]] DCMIPP_HandleTypeDef *hdcmipp)
{
	HAL_StatusTypeDef ret = HAL_RCCEx_PeriphCLKConfig(&dcmipp_clk_conf);
	if (ret)
		return ret;

	return HAL_RCCEx_PeriphCLKConfig(&csi_clk_conf);
}
