#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "stm32n6xx_nucleo.h"
#include "task.h"

#include "app.h"
#include "camera.h"
#include "cmsis_os2.h"
#include "cmw_camera.h"
#include "main.h"

#include "console.h"
#include "display.h"

const osThreadAttr_t console_task_attr = {
	.name = "cli",
	.priority = (osPriority_t)osPriorityNormal,
	.stack_size = 512 * 4
};

const osThreadAttr_t camera_task_attr = {
	.name = "camera",
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

	if (!osThreadNew(console_task, NULL, &console_task_attr)) {
		return -EINVAL;
	}

	if (!osThreadNew(display_task, NULL, &camera_task_attr)) {
		return -EINVAL;
	}

	return 0;
}

#define MAX_CMD_SIZE 64

void console_task([[maybe_unused]] void *argument)
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

void display_task([[maybe_unused]] void *argument)
{
	int res = camera_start(display_get_bg_buff(), CMW_MODE_CONTINUOUS);
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
