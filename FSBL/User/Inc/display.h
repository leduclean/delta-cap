/*
 * display.h
 *
 *  Created on: 1 sept. 2026
 *      Author: mancinis
 */

#pragma once

#include <stdint.h>

#define SCREEN_FPS 5
#define SCREEN_WIDTH (320)
#define SCREEN_HEIGHT (240)

int display_text(void);
uint8_t *display_bg_buffer(void);

int display_init(uint32_t screen_size[2]);
int display_update(void);


extern volatile int display_count;
