#pragma once

#include <stdint.h>

#include "ai.h"

#define SCREEN_FPS 5
#define SCREEN_WIDTH (320)
#define SCREEN_HEIGHT (240)

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
 * @param det The detections from the ai inference.
 * @param count The number of detections.
 * @return 0 on SUCCESS, error code < 0 when failed to refresh.
 */
int display_add_overlay(struct detection *det, int count);

/**
 * @brief Draw the screen
 */
void display_draw(void);
