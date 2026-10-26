#pragma once
#include "app_config.h"
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
	int cls; /**< Class id, use this for sorting */
	const char *
		label; /**< Points into the static classes_table, display only */
};

/**
 * @class ai_result
 * @brief Snapshot of the last inference.
 */
struct ai_result {
	struct detection det[AI_OD_ST_YOLOX_PP_MAX_BOXES_LIMIT];
	int n; /**< Number of valid entries in det */
	uint32_t ms; /**< NPU + post process time */
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
 * @brief Gets the result of the last inference run.
 *
 * @param[out] out The result to fill.
 */
void ai_get_result(struct ai_result *out);
