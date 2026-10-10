#pragma once
#define USE_DCACHE
#define CAMERA_FPS 25

#include <stdint.h>

extern int32_t camera_frame_count;

#define CAMERA_FLIP CMW_MIRRORFLIP_NONE

int camera_init(uint32_t screen_size[2]);

void camera_start(uint8_t *display_pipe_dst, uint32_t cam_mode);

void camera_isp_update(void);
