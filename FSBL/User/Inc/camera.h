#pragma once
#include <stdint.h>

//TODO try 30 FPS
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
 * @brief Start the camera pipe of the screen giving it the dst buffer.
 *
 * @param pbuff The pipe destination buffer.
 * @return 0 on SUCCESS, error code < 0 on failure.
 */
[[nodiscard]] int camera_start_screen_pipe(uint8_t *pbuff);

/**
 * @brief Start the camera pipe of the screen.
 *
 * @return 0 on SUCCESS, error code < 0 on failure.
 */
[[nodiscard]] int camera_start_nn_pipe();

/**
 * @brief Run the camera driver.
 *
 * @return 0 on SUCCESS, error code < 0 on failure.
 */
int camera_run(void);

/**
 * @brief Wait for an available displframe.
 *
 * @retval -EOK
 * @retval -ETIMEDOUT If the frame was too long to come.
 */
[[nodiscard]] int camera_wait_screen_frame(void);

/**
 * @brief Blocking function that wait for an available nn frame.
 *
 * @return A pointer to the available frame.
 * @retval -ETIMEDOUT If the frame was too long to come.
 */
uint8_t *camera_get_nn_frame(void);

/**
 * @brief Blocking capture of a frame at nn input format.
 *
 * @param dst An ai input.
 * @return 0 on SUCCESS, error code < 0 on failure.
 */
int camera_nn_capture(uint8_t *dst);
