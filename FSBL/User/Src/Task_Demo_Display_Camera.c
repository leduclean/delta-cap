/*
 * Task_Demo_Console.c
 *
 *  Created on: 31 août 2026
 *      Author: mancinis
 */

#include "FreeRTOS.h"
#include "task.h"
#include "stm32n6xx_nucleo.h"

#include "cmsis_os2.h"
#include "camera_demo_pipeline.h"
#include "cmw_camera.h"

#include "display_demo_init.h"
#include "UART_Console.h"

osThreadId_t TaskHandle_Demo_Console_Display_Camera;
const osThreadAttr_t Task_attributes_Demo_Console_Display_Camera = {
	.name = "democd",
	.priority = (osPriority_t)osPriorityNormal,
	.stack_size = 512 * 4
};

osThreadId_t TaskHandle_Demo_Console_LED_Display_Camera;
const osThreadAttr_t Task_attributes_Demo_Console_LED_Display_Camera = {
	.name = "democledd",
	.priority = (osPriority_t)osPriorityNormal,
	.stack_size = 256 * 4
};

osThreadId_t TaskHandle_Demo_Display_Camera;
const osThreadAttr_t Task_attributes_Demo_Display_Camera = {
	.name = "demodd",
	.priority = (osPriority_t)osPriorityNormal,
	.stack_size = 256 * 4
};
void Task_Demo_Console_Display_Camera(void *argument);
void Task_Demo_Console_LED_Display_Camera(void *argument);
void Task_Demo_Display_Camera(void *argument);

volatile int led_r_tog = 0;
volatile int led_g_tog = 1;
volatile int led_b_tog = 0;

int Task_Demo_Console_Display_Camera_Start(void)
{
	uint32_t screen_size[2];
	Display_Demo_Init(screen_size);
	Demo_Camera_Pipeline_Init(screen_size);
	TaskHandle_Demo_Console_Display_Camera =
		osThreadNew(Task_Demo_Console_Display_Camera, NULL,
			    &Task_attributes_Demo_Console_Display_Camera);
	TaskHandle_Demo_Console_LED_Display_Camera =
		osThreadNew(Task_Demo_Console_LED_Display_Camera, NULL,
			    &Task_attributes_Demo_Console_LED_Display_Camera);
	TaskHandle_Demo_Display_Camera =
		osThreadNew(Task_Demo_Display_Camera, NULL,
			    &Task_attributes_Demo_Display_Camera);
	return 0;
}

void Task_Demo_Console_Display_Camera(void *argument)
{
	printf("Demo Console _Camera\n\r");
	fflush(stdout);
	int count = 0;
	int key = 0;
	char line[128];
	UART_Rx_Flush();
	for (;;) {
		printf("\033[3J\033[H\033[2J");
		printf("count %d \n\r", count);
		printf("Key ? (crgbsD) \n\r");
		key = UART_Get_Key();
		//printf("%d \n\r", key);display_demo_count++
		if (key != -1) {
			switch (key) {
			case 'c':
				count = 0;
				break;

			case 'r':
				led_r_tog = !led_r_tog;
				break;

			case 'g':
				led_g_tog = !led_g_tog;
				break;

			case 'b':
				led_b_tog = !led_b_tog;
				break;

			case 's':
				/* to see the characters set terminal local echo on */
				printf("Enter line\n\r");
				UART_Scan_Line(line, 128);

				int value;
				char text[128];
				printf("\n\r|%s_\n\r", line);
				int s = sscanf(line, "%d %s", &value, text);
				if (s == 2)
					printf("you entered %d values: %s %d\n\r",
					       s, text, value);
				else
					printf("incorrect format %d: %s\n\r", s,
					       line);
				UART_Rx_Flush();
				osDelay(20000);
				break;

			case 'D':
				display_demo_count = 0;
				Display_Demo_FG_Erase();
				break;
			}
		}
		count++;
		osDelay(200);
	}
}
void Task_Demo_Console_LED_Display_Camera(void *argument)
{
	int tog_cnt = 0;
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

void Task_Demo_Display_Camera(void *argument)
{
	printf("========================================\n\r");
	printf("Display init\n\r");
	printf("Camera init\n\r");

	Demo_Camera_Pipeline_Start(Display_Demo_BG_Buffer(),
				   CMW_MODE_CONTINUOUS); //CMW_MODE_SNAPSHOT
	int cam_count = 0;
	int cnt = 0;
	/*
	 * BEWARE
	 * printf in this loop are not synchonised with printf with the console one
	 * change the code to suppress printf from here and centralise them in the console only
	 */
	for (;;) {
		printf("TDDCamcnt%d\n\r", cnt++);
		Demo_Camera_Pipeline_IspUpdate();
		/* Print the number of received frames since last iteration (should be 25 fps) */
		printf("received frames %d\n\r", demo_camera_frame_received);
		/*
		 * demo_camera_frame_received is incremented in CMW_CAMERA_PIPE_FrameEventCallback
		 * file camera_demo_pipeline.c
		 * This callback is executed each time a frame is received
		 * Beware : its an interrupt service routine (ISR)
		 * Application code should not be executed in ISR
		 * Application code runs in a task => wait for a new frame
		 *     * polling with delay
		 *     or
		 *     * synchronize with a semaphore
		 *  The camera frame are stored in array of address  Display_Demo_BG_Buffer()
		 *  uint8_t *frame= Display_Demo_BG_Buffer();
		 */
		demo_camera_frame_received = 0;
		//display_demo_count++;
		Display_Demo_Text();
		Display_Demo_Update();
		osDelay(100);
	}
}
