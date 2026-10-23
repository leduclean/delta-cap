#include <app_config.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stm32n6xx_hal.h>

#include "FreeRTOS.h"
#include "task.h"

#include "ai.h"
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

const osThreadAttr_t display_task_attr = {
	.name = "display",
	.priority = (osPriority_t)osPriorityNormal,
	.stack_size = 256 * 4
};

const osThreadAttr_t ai_task_attr = {
	.name = "ai",
	/* Since ai is high cost task we need to put it with low priority
    * to ensure that we will display when a new frame arrives*/
	.priority = (osPriority_t)osPriorityBelowNormal,
	.stack_size = 256 * 4
};

#define MAX_CMD_SIZE 64

static void console_task([[maybe_unused]] void *argument)
{
	char line[MAX_CMD_SIZE];

	console_rx_flush();
	for (;;) {
		printf("> ");
		if (console_scan_line(line, sizeof(line)) <= 0) {
			printf("Invalid Input \n");
			continue;
		}
		/** Add some command handling via strcomp */
	}
}

static void ai_task([[maybe_unused]] void *argument)
{
	for (;;) {
		int res = camera_nn_capture(ai_input());
		if (res == 0)
			res = ai_run();
		if (res < 0)
			printf("ai: error %d\r\n", res);
	}
}

static void display_task([[maybe_unused]] void *argument)
{
	int res = camera_start(display_get_bg_buff(), CMW_MODE_CONTINUOUS);
	if (res < 0) {
		printf("camera_start failed: %d\r\n", res);
		Error_Handler();
	}

	for (;;) {
		camera_run();
		camera_wait_for_frame();

		struct detection det[AI_OD_ST_YOLOX_PP_MAX_BOXES_LIMIT];
		int n = ai_get_detections(det,
					  AI_OD_ST_YOLOX_PP_MAX_BOXES_LIMIT);

		display_add_overlay(det, n);
		display_draw();
	}
}

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

	res = ai_init();
	if (res < 0) {
		return res;
	}

	if (!osThreadNew(console_task, NULL, &console_task_attr)) {
		return -EINVAL;
	}

	if (!osThreadNew(display_task, NULL, &display_task_attr)) {
		return -EINVAL;
	}

	if (!osThreadNew(ai_task, NULL, &ai_task_attr)) {
		return -EINVAL;
	}

	return 0;
}
