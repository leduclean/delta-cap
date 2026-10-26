#pragma once

#include <stdint.h>

#include "ai.h"

#define SCREEN_WIDTH (320)
#define SCREEN_HEIGHT (240)
#define SCREEN_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT)

/**
 * @brief Get a pointer to the background buffer.
 */
uint8_t *display_get_bg_buff(void);

/**
 * @brief Init the display layer.
 *
 * @return 0 on SUCCESS, error code < 0 if SCRL failed.
 */
[[nodiscard]] int display_init(void);

/**
 * @brief Add the overlay to the main content.
 *
 * @param result The result of the last ai inference.
 * @return 0 on SUCCESS, error code < 0 when failed to refresh.
 */
int display_add_overlay(const struct ai_result *result);

/**
 * @brief Draw the screen
 */
void display_draw(void);
