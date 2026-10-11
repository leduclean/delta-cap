/*
 * app.c
 *
 *  Created on: 31 août 2026
 *      Author: mancinis
 */

#include "main.h"
#include <assert.h>

#include "FreeRTOS.h"
#include "task.h"
#include "stm32n6xx_nucleo.h"

#include "cmsis_os2.h"
#include "camera.h"
#include "cmw_camera.h"

#include "display.h"
#include "console.h"
#include <stdio.h>
#include <string.h>

osThreadId_t console_task_handle;
const osThreadAttr_t Task_attributes_Demo_Console_Display_Camera = {
	.name = "democd",
	.priority = (osPriority_t)osPriorityNormal,
	.stack_size = 512 * 4
};

osThreadId_t display_task_handle;
const osThreadAttr_t Task_attributes_Demo_Display_Camera = {
	.name = "demodd",
	.priority = (osPriority_t)osPriorityNormal,
	.stack_size = 256 * 4
};

void console_task(void *argument);
void display_task(void *argument);

int app_start(void)
{
	int res;
	res = display_init();

	if (res < 0) {
		return res;
	}

	res = camera_init();
	if (res < 0) {
		return res;
	}

	console_task_handle =
		osThreadNew(console_task, NULL,
			    &Task_attributes_Demo_Console_Display_Camera);
	display_task_handle = osThreadNew(display_task, NULL,
					  &Task_attributes_Demo_Display_Camera);
	return 0;
}

#define MAX_CMD_SIZE 64

void console_task(void *argument)
{
	char line[MAX_CMD_SIZE];

	console_rx_flush();
	for (;;) {
		printf("> ");
		if (console_scan_line(line, sizeof(line)) <= 0) {
			printf("Invalid Input \n");
			continue;
		}
		printf("cmd is: %s \r\n", line);
	}
}

void display_task(void *argument)
{
	int res;
	camera_start(display_get_bg_buff(), CMW_MODE_CONTINUOUS);
	if (res < 0) {
		printf("camera_start failed: %d\r\n", res);
		Error_Handler();
	}
	for (;;) {
		camera_run();
		camera_wait_for_frame();
		display_add_overlay();
		display_draw();
	}
}
