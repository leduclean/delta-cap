#include <app_config.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stm32n6xx_hal.h>

#include "ai.h"
#include "app.h"
#include "calib.h"
#include "camera.h"
#include "cmd.h"
#include "cmsis_os2.h"
#include "console.h"
#include "display.h"
#include "kinematic.h"
#include "magnet.h"
#include "main.h"
#include "motor.h"

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

const osThreadAttr_t control_task_attr = {
	.name = "control",
	.priority = (osPriority_t)osPriorityNormal,
	.stack_size = 256 * 4,
};

#define MAX_CMD_SIZE 64

#define AI_TIMEOUT_FRAMES 3 /**< Missed frames before warning. */
#define TABLE_Z \
	10 /**< Table Z i.e where the objects are compare to the camera. */

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

[[maybe_unused]] static void ai_task([[maybe_unused]] void *argument)
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
		} else {
		}
	}
}

[[maybe_unused]] static void display_task([[maybe_unused]] void *argument)
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

static void control_task([[maybe_unused]] void *argument)
{
	int res;
	const uint32_t inf_timeout =
		osKernelGetTickFreq() * AI_TIMEOUT_FRAMES / CAMERA_FPS;

	res = motor_init();

	if (res < 0) {
		printf("Motor init failed: %d\r\n", res);
		Error_Handler();
	}

	for (;;) {
		struct ai_result ai_res;
		res = ai_wait_result(&ai_res, inf_timeout);
		if (res < 0) {
			printf("AI is running too slow, it should be running under %d\r\n",
			       CAMERA_FPS);
			continue;
		}
		if (ai_res.n == 0) {
			/* Nothing detected */
			continue;
		}

		struct detection choosen = ai_res.det[0];

		float x;
		float y;
		calib_pixel_to_world(choosen.x, choosen.y, &x, &y);

		float angles[MOTOR_NUM];
		bool reachable = kinematics_inverse(x, y, TABLE_Z, angles);
		if (!reachable) {
			printf("Unreachable: %.1f %.1f\r\n", (double)x,
			       (double)y);
			//TODO: handle this
			continue;
		}

		res = motor_goto_angles(angles);
		if (res < 0) {
			printf("We got unexpected behavior on the motors. \r\n");
		}

		//TODO: Magnet take and release + homing
	}
}

int app_start(void)
{
	int res;

	res = display_init();
	if (res < 0) {
		return res;
	}

	// camera/ai disabled to test the motor alone, without the whole camera setup
	// res = camera_init();
	// if (res < 0) {
	// 	return res;
	// }

	// res = ai_init();
	// if (res < 0) {
	// 	return res;
	// }

	//TODO: move this in the monitoring task
	res = motor_init();
	if (res < 0) {
		printf("Error during motor init \r\n");
		return res;
	}

	if (!osThreadNew(console_task, NULL, &console_task_attr)) {
		return -EINVAL;
	}

	// if (!osThreadNew(display_task, NULL, &display_task_attr)) {
	// 	return -EINVAL;
	// }

	// if (!osThreadNew(ai_task, NULL, &ai_task_attr)) {
	// 	return -EINVAL;
	// }

	if (!osThreadNew(control_task, NULL, &control_task_attr)) {
		return -EINVAL;
	}

	return 0;
}
