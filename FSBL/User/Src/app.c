/*
 * app.c
 *
 *  Created on: 31 août 2026
 *      Author: mancinis
 */

#include "FreeRTOS.h"
#include "task.h"
#include "stm32n6xx_nucleo.h"

#include "cmsis_os2.h"
#include "camera.h"
#include "cmw_camera.h"

#include "display.h"
#include "console.h"

osThreadId_t console_task_handle;
const osThreadAttr_t Task_attributes_Demo_Console_Display_Camera = {
	.name = "democd",
	.priority = (osPriority_t)osPriorityNormal,
	.stack_size = 512 * 4
};

osThreadId_t led_task_handle;
const osThreadAttr_t Task_attributes_Demo_Console_LED_Display_Camera = {
	.name = "democledd",
	.priority = (osPriority_t)osPriorityNormal,
	.stack_size = 256 * 4
};

osThreadId_t display_task_handle;
const osThreadAttr_t Task_attributes_Demo_Display_Camera = {
	.name = "demodd",
	.priority = (osPriority_t)osPriorityNormal,
	.stack_size = 256 * 4
};
void console_task(void *argument);
void led_task(void *argument);
void display_task(void *argument);

volatile int led_r_tog = 0;
volatile int led_g_tog = 1;
volatile int led_b_tog = 0;

int app_start(void)
{
	uint32_t screen_size[2];
	display_init(screen_size);
	camera_init(screen_size);
	console_task_handle =
		osThreadNew(console_task, NULL,
			    &Task_attributes_Demo_Console_Display_Camera);
	led_task_handle =
		osThreadNew(led_task, NULL,
			    &Task_attributes_Demo_Console_LED_Display_Camera);
	display_task_handle = osThreadNew(display_task, NULL,
					  &Task_attributes_Demo_Display_Camera);
	return 0;
}

void console_task(void *argument)
{
	console_rx_flush();
	for (;;) {
		switch (console_get_key()) {
		/* add commands here: case 'x': ...; break; */
		default:
			break;
		}
	}
}
void led_task(void *argument)
{
	for (;;) {
		if (led_r_tog)
			BSP_LED_Toggle(LED_RED);
		if (led_g_tog)
			BSP_LED_Toggle(LED_GREEN);
		if (led_b_tog)
			BSP_LED_Toggle(LED_BLUE);
		osDelay(300);
	}
}

void display_task(void *argument)
{
	camera_start(display_bg_buffer(),
		     CMW_MODE_CONTINUOUS); //CMW_MODE_SNAPSHOT
	for (;;) {
		camera_isp_update();
		/*
		 * camera_frame_count is incremented in CMW_CAMERA_PIPE_FrameEventCallback
		 * file camera.c
		 * This callback is executed each time a frame is received
		 * Beware : its an interrupt service routine (ISR)
		 * Application code should not be executed in ISR
		 * Application code runs in a task => wait for a new frame
		 *     * polling with delay
		 *     or
		 *     * synchronize with a semaphore
		 *  The camera frame are stored in array of address  display_bg_buffer()
		 *  uint8_t *frame= display_bg_buffer();
		 */
		display_text();
		display_update();
		osDelay(100);
	}
}
