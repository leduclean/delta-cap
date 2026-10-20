#pragma once
#include <stdint.h>

#define CAMERA_FPS 25
#define CAMERA_FLIP CMW_MIRRORFLIP_NONE

#define NN_WIDTH 224U
#define NN_HEIGHT 224U
#define NN_BPP 3U

#define NN_FRAME_SIZE (NN_WIDTH * NN_HEIGHT * NN_BPP)

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
 * @brief Blocking capture of a frame at nn input format.
 *
 * @param dst An ai input.
 * @return 0 on SUCCESS, error code < 0 on failure.
 */
int camera_nn_capture(uint8_t *dst);
