#include <assert.h>
#include <errno.h>

#include "display.h"

#include "FreeRTOS.h"
#include "cmsis_os2.h"
#include "camera.h"
#include "cmw_camera.h"

/**
 * @brief Semaphore used for interrupt released by the frame ISR
 * and taken by a call to camera_wait_for_frame().
 */
static osSemaphoreId_t frame_sem;
static StaticSemaphore_t frame_sem_cb;
static const osSemaphoreAttr_t frame_sem_attr = {
	.name = "frame sem",
	.cb_mem = &frame_sem_cb,
	.cb_size = sizeof(frame_sem_cb),
};

/* Leave the driver use the default resolution */
#define CAMERA_WIDTH 0
#define CAMERA_HEIGHT 0

static CMW_CameraInit_t cam_conf = {
	.width = CAMERA_WIDTH, /* 0 = filled in by the camera manager */
	.height = CAMERA_HEIGHT,
	.fps = CAMERA_FPS,
	.mirror_flip = CAMERA_FLIP,
};

static CMW_DCMIPP_Conf_t p_conf = {
	.output_width = SCREEN_WIDTH,
	.output_height = SCREEN_HEIGHT,
	.output_format = DCMIPP_PIXEL_PACKER_FORMAT_RGB888_YUV444_1,

	.output_bpp = 3,
	.enable_swap = 1, /* RGB */
	.enable_gamma_conversion = 0,

	.mode = CMW_Aspect_ratio_fullscreen,
};

static int _setup_pipe(void)
{
	int res;

	uint32_t pitch;
	res = CMW_CAMERA_SetPipeConfig(DCMIPP_PIPE1, &p_conf, &pitch);
	if (res < 0) {
		return res;
	}
	assert(p_conf.output_width * p_conf.output_bpp == pitch);

	return pitch;
}

int camera_init()
{
	int res;

	frame_sem = osSemaphoreNew(1, 0, &frame_sem_attr);
	if (!frame_sem) {
		return -EINVAL;
	}

	res = CMW_CAMERA_Init(&cam_conf, NULL);
	if (res < 0) {
		return res;
	}

	res = _setup_pipe();
	if (res < 0) {
		return res;
	}

	return 0;
}

int camera_start(uint8_t *display_pipe_dst, uint32_t cam_mode)
{
	return CMW_CAMERA_Start(DCMIPP_PIPE1, display_pipe_dst, cam_mode);
}

int camera_run(void)
{
	return CMW_CAMERA_Run();
}

void camera_wait_for_frame(void)
{
	osSemaphoreAcquire(frame_sem, osWaitForever);
}

int CMW_CAMERA_PIPE_FrameEventCallback(uint32_t pipe)
{
	switch (pipe) {
	case DCMIPP_PIPE1:
		osSemaphoreRelease(frame_sem);
		break;
	}
	return 0;
}

void CSI_IRQHandler(void)
{
	DCMIPP_HandleTypeDef *hcamera_dcmipp = CMW_CAMERA_GetDCMIPPHandle();
	HAL_DCMIPP_CSI_IRQHandler(hcamera_dcmipp);
}

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

HAL_StatusTypeDef
MX_DCMIPP_ClockConfig([[maybe_unused]] DCMIPP_HandleTypeDef *hdcmipp)
{
	HAL_StatusTypeDef ret = HAL_RCCEx_PeriphCLKConfig(&dcmipp_clk_conf);
	if (ret)
		return ret;

	return HAL_RCCEx_PeriphCLKConfig(&csi_clk_conf);
}
