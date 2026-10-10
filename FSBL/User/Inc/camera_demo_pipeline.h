/*
 * demo_camera_pipeline.h
 *
 *  Created on: 14 sept. 2026
 *      Author: mancinis
 */

#ifndef APPLICATION_USER_DEMO_CAMERA_CAMERA_DEMO_PIPELINE_H_
#define APPLICATION_USER_DEMO_CAMERA_CAMERA_DEMO_PIPELINE_H_
#define USE_DCACHE
#define CAMERA_FPS 25

#include <stdint.h>

extern int32_t demo_camera_frame_received;

/*Defines: CMW_MIRRORFLIP_NONE; CMW_MIRRORFLIP_FLIP; CMW_MIRRORFLIP_MIRROR; CMW_MIRRORFLIP_FLIP_MIRROR;*/
#define CAMERA_FLIP CMW_MIRRORFLIP_NONE

int main_camera_demo(void);

int Demo_Camera_Pipeline_Init(uint32_t screen_size[2]);
void Demo_Camera_Pipeline_Start(uint8_t *display_pipe_dst, uint32_t cam_mode);
void Demo_Camera_Pipeline_IspUpdate(void);

#endif /* APPLICATION_USER_DEMO_CAMERA_CAMERA_DEMO_PIPELINE_H_ */
