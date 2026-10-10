/**
 ******************************************************************************
 * @file    app_camerapipeline.c
 * @author  GPM Application Team
 *
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

#include <assert.h>

#include "camera_demo_pipeline.h"
#include "cmw_camera.h"

/* Leave the driver use the default resolution */
#define CAMERA_WIDTH 0
#define CAMERA_HEIGHT 0
/*
 * In this demo, only pipeline 1 is used
 * To use pipeline 0 (raw data) and pipeline 2 (scaled) see original demo : file app_camerapipeline.c
 * https://github.com/STMicroelectronics/STM32N6-GettingStarted-ImageClassification
 * https://github.com/STMicroelectronics/STM32N6-GettingStarted-ImageClassification/blob/main/Application/NUCLEO-N657X0-Q/Src/app_camerapipeline.c
 */
int32_t demo_camera_frame_received;

int Demo_Camera_Pipeline_Grab_Init(CMW_CameraInit_t *camConf,
				   uint32_t screen_size[2])
{
	CMW_Aspect_Ratio_Mode_t aspect_ratio;
	CMW_DCMIPP_Conf_t dcmipp_conf = { 0 };
	int ret;

	aspect_ratio = CMW_Aspect_ratio_fullscreen;
#define COLOR_BGR (0)
#define COLOR_RGB (1)
	dcmipp_conf.output_width = screen_size[0];
	dcmipp_conf.output_height = screen_size[1];
	dcmipp_conf.output_format = DCMIPP_PIXEL_PACKER_FORMAT_RGB888_YUV444_1;
	dcmipp_conf.output_bpp = 3;
	dcmipp_conf.mode = aspect_ratio;
	dcmipp_conf.enable_swap = COLOR_RGB;
	dcmipp_conf.enable_gamma_conversion = 0;
	uint32_t pitch;
	ret = CMW_CAMERA_SetPipeConfig(DCMIPP_PIPE1, &dcmipp_conf, &pitch);
	assert(ret == HAL_OK);
	assert(dcmipp_conf.output_width * dcmipp_conf.output_bpp == pitch);
	return pitch;
}

/**
 * @brief Init the camera and the 2 DCMIPP pipes
 * @param lcd_bg_width display width
 * @param lcd_bg_height display height
 * @param pitch_nn output pitch computed by the CMW
 */
int Demo_Camera_Pipeline_Init(uint32_t screen_size[2])
{
	int ret;
	CMW_CameraInit_t cam_conf;
	/* To set from camera manager */
	cam_conf.width = 0;
	cam_conf.height = 0;
	cam_conf.fps = CAMERA_FPS;
	cam_conf.mirror_flip = CAMERA_FLIP;

	ret = CMW_CAMERA_Init(&cam_conf, NULL);
	assert(ret == CMW_ERROR_NONE);
	int pitch = Demo_Camera_Pipeline_Grab_Init(&cam_conf, screen_size);
	return pitch;
}

void Demo_Camera_Pipeline_DeInit(void)
{
	int ret;
	ret = CMW_CAMERA_DeInit();
	assert(ret == CMW_ERROR_NONE);
}

void Demo_Camera_Pipeline_Start(uint8_t *display_pipe_dst, uint32_t cam_mode)
{
	int ret;
	ret = CMW_CAMERA_Start(DCMIPP_PIPE1, display_pipe_dst, cam_mode);
	assert(ret == CMW_ERROR_NONE);
}

void Demo_Camera_Pipeline_Stop()
{
	int ret;
	ret = CMW_CAMERA_Suspend(DCMIPP_PIPE1);
	assert(ret == CMW_ERROR_NONE);
}

void Demo_Camera_Pipeline_IspUpdate(void)
{
	int ret = CMW_ERROR_NONE;
	ret = CMW_CAMERA_Run();
	assert(ret == CMW_ERROR_NONE);
}

/**
 * @brief  Frame event callback
 * @param  hdcmipp pointer to the DCMIPP handle
 * @retval None
 */
int CMW_CAMERA_PIPE_FrameEventCallback(uint32_t pipe)
{
	int ret;

	switch (pipe) {
	case DCMIPP_PIPE1:
		demo_camera_frame_received++;

		//ret = SRCL_Update();
		//assert(ret == 0);
		break;
	}
	return 0;
}
#if 1
void CMW_CAMERA_PIPE_ErrorCallback(uint32_t pipe)
{
	/* FIXME : Need to tune sensor/ipplug so we can remove this implementation */
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

HAL_StatusTypeDef MX_DCMIPP_ClockConfig(DCMIPP_HandleTypeDef *hdcmipp)
{
	RCC_PeriphCLKInitTypeDef RCC_PeriphCLKInitStruct = { 0 };
	HAL_StatusTypeDef ret = HAL_OK;

	RCC_PeriphCLKInitStruct.PeriphClockSelection = RCC_PERIPHCLK_DCMIPP;
	RCC_PeriphCLKInitStruct.DcmippClockSelection = RCC_DCMIPPCLKSOURCE_IC17;
	RCC_PeriphCLKInitStruct.ICSelection[RCC_IC17].ClockSelection =
		RCC_ICCLKSOURCE_PLL2;
	RCC_PeriphCLKInitStruct.ICSelection[RCC_IC17].ClockDivider = 3;
	ret = HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphCLKInitStruct);
	if (ret) {
		return ret;
	}

	RCC_PeriphCLKInitStruct.PeriphClockSelection = RCC_PERIPHCLK_CSI;
	RCC_PeriphCLKInitStruct.ICSelection[RCC_IC18].ClockSelection =
		RCC_ICCLKSOURCE_PLL1;
	RCC_PeriphCLKInitStruct.ICSelection[RCC_IC18].ClockDivider = 40;
	ret = HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphCLKInitStruct);
	if (ret) {
		return ret;
	}

	return ret;
}
#endif
