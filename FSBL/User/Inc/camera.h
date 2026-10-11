#pragma once
#define USE_DCACHE
#define CAMERA_FPS 25

#include <stdint.h>

#define CAMERA_FLIP CMW_MIRRORFLIP_NONE

/**
 * @brief Init the camera driver.
 *
 * @return 0 on SUCCESS, error code < 0 on failure.
 */
[[nodiscard]] int camera_init(void);

/**
 * @brief Start the camera giving it a pipe to send data.
 *
 * @param display_pipe_dst The pipe destination buffer.
 * @param cam_mode The mode of the camera.
 * @return 0 on SUCCESS, error code < 0 on failure.
 */
[[nodiscard]] int camera_start(uint8_t *display_pipe_dst, uint32_t cam_mode);

/**
 * @brief Run the camera driver.
 *
 * @return 0 on SUCCESS, error code < 0 on failure.
 */
int camera_run(void);

/**
 * @brief Wait for an available frame.
 */
void camera_wait_for_frame(void);

/**
 * @brief CSI interrupt handler, called from the vector table.
 */
void CSI_IRQHandler(void);

/**
 * @brief DCMIPP interrupt handler, called from the vector table.
 */
void DCMIPP_IRQHandler(void);
