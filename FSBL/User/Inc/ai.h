#pragma once
#include <stdint.h>

/**
 * @class detection
 * @brief Output filled by the neural network.
 */
struct detection {
	float x;
	float y;
	float w;
	float h;
	float conf;
	int cls;
};

/**
 * @brief Initialize the neural network and it hardware components
 *
 * @return 0 on SUCCESS or error code < 0 on FAILURE
 *
 * @retval   0  Well Initialized neural network.
 * @retval -EIO Failed to Initialize hardware component.
 * @retval -ENOMEM If not enough RAM to install model.
 */
[[nodiscard]] int ai_init();

void ai_deinit(void);

/**
 * @brief Gives a pointer to the input buffer.
 * @note  You will need this pointer to give the input 
 * to the neural network.
 */
void *ai_input(void);

/**
 * @brief Gives the input size of the input buffer.
 *
 * @return The input size.
 */
int32_t ai_input_size(void);

/**
 * @brief Run an inference on the provided input stream 
 * and post process the results.
 *
 * @param frame The input frame received by the camera.
 * @return 0 on SUCCESS, -EIO on failure. 
 */
[[nodiscard]] int ai_run(const uint8_t *frame);

/**
 * @brief Gets the detections structure out of the last
 * inference run.
 *
 * @param out The detection table to fill.
 * @param max The max number of detections to fill.
 * @return The number of detections readed.
 */
int ai_get_detections(struct detection *out, int max);
