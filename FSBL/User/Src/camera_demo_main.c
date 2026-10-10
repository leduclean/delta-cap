/*
 * camera_demo.c
 *
 *  Created on: 14 sept. 2026
 *      Author: mancinis
 */

#include "camera_demo_pipeline.h"
#include "display_demo_init.h"
#include "cmw_camera.h"

int main_camera_demo(void)
{
	printf("Init Display\n\r");
	uint32_t screen_size[2];
	Display_Demo_Init(screen_size);
	printf("Display init %d %d\n\r", screen_size[0], screen_size[1]);
	Demo_Camera_Pipeline_Init(screen_size);
	Demo_Camera_Pipeline_Start(Display_Demo_BG_Buffer(),
				   CMW_MODE_CONTINUOUS); //CMW_MODE_SNAPSHOT
	int cam_count = 0;
	//Demo_Camera_Pipeline_IspUpdate();
	while (1) {
		Demo_Camera_Pipeline_IspUpdate();
		/* Print the number of received frames since last iteration (should be 25 fps) */
		printf("cnt %d frame %d\n\r", cam_count++,
		       demo_camera_frame_received);
		demo_camera_frame_received = 0;
		Display_Demo_Text();
		Display_Demo_Update();
		HAL_Delay(200);
	}
}
