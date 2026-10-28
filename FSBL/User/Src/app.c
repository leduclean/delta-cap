#include <app_config.h>
#include <errno.h>
#include <stdio.h>
#include <stm32n6xx_hal.h>

#include "ai.h"
#include "app.h"
#include "camera.h"
#include "cmd.h"
#include "cmsis_os2.h"
#include "console.h"
#include "display.h"
#include "main.h"

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
	.stack_size = 4096,
};

#define MAX_CMD_SIZE 64

static void console_task([[maybe_unused]] void *argument)
{
	char line[MAX_CMD_SIZE];
	for (;;) {
		printf("(delta) ");
		if (console_scan_line(line, sizeof(line)) <= 0) {
			printf("Invalid Input \r\n");
			continue;
		}
		int res = cmd_execute(line);
		if (res < 0) {
			printf("Invalid command refear to 'help'\r\n");
		}
	}
}

static void ai_task([[maybe_unused]] void *argument)
{
	int res = camera_start_nn_pipe();
	if (res < 0) {
		printf("NN pipe start failed: %d\r\n", res);
		Error_Handler();
	}
	for (;;) {
		uint8_t *frame = camera_get_nn_frame();
		if (!frame) {
			printf("Frame took too much time to come\r\n");
			continue;
		}
		res = ai_run(frame);
		if (res < 0) {
			printf("AI error: %d\r\n", res);
		}
	}
}

static void display_task([[maybe_unused]] void *argument)
{
	int res = camera_start_screen_pipe(display_get_bg_buff());
	if (res < 0) {
		printf("camera_start failed: %d\r\n", res);
		Error_Handler();
	}

	for (;;) {
		camera_run();
		res = camera_wait_screen_frame();
		if (res < 0) {
			printf("screen frame timeout\r\n");
			continue;
		}

		struct ai_result result;
		ai_get_result(&result);

		display_add_overlay(&result);
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
