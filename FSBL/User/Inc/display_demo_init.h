/*
 * display_demo_init.h
 *
 *  Created on: 1 sept. 2026
 *      Author: mancinis
 */

#ifndef APPLICATION_USER_DISPLAY_UVC_DISPLAY_DEMO_INIT_H_
#define APPLICATION_USER_DISPLAY_UVC_DISPLAY_DEMO_INIT_H_

#include <stdint.h>

#define SCREEN_FPS 5
#define SCREEN_WIDTH (320)
#define SCREEN_HEIGHT (240)

int Display_Demo_Text(void);
int Display_Demo_FG_Erase(void);
uint8_t *Display_Demo_BG_Buffer(void);

int Display_Demo_Init(uint32_t screen_size[2]);
int Display_Demo_Update(void);

int main_display(void);

extern volatile int display_demo_count;

#endif /* APPLICATION_USER_DISPLAY_UVC_DISPLAY_DEMO_INIT_H_ */
