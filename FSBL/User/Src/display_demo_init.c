#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>

#include "stm32n6xx_hal.h"
#include "stm32_lcd.h"
#include "stm32_lcd_ex.h"
#include "scrl.h"
#include "display_demo_init.h"

#define LCD_BG_WIDTH SCREEN_WIDTH
#define LCD_BG_HEIGHT SCREEN_HEIGHT
#define LCD_FG_WIDTH SCREEN_WIDTH
#define LCD_FG_HEIGHT SCREEN_HEIGHT

#define LCD_FG_FRAMEBUFFER_SIZE (LCD_FG_WIDTH * LCD_FG_HEIGHT * 2)

#define UTIL_LCD_COLOR_TRANSPARENT 0

typedef struct {
	uint32_t X0;
	uint32_t Y0;
	uint32_t XSize;
	uint32_t YSize;
} Rectangle_TypeDef;

/* Lcd Background area */
Rectangle_TypeDef lcd_bg_area = {
	.X0 = 0,
	.Y0 = 0,
	.XSize = LCD_BG_WIDTH,
	.YSize = LCD_BG_HEIGHT,
};

/* Lcd Foreground area */
Rectangle_TypeDef lcd_fg_area = {
	.X0 = 0,
	.Y0 = 0,
	.XSize = LCD_FG_WIDTH,
	.YSize = LCD_FG_HEIGHT,
};
static int lcd_fg_buffer_rd_idx = 0;

#if 1
/* Lcd Background Buffer */
__attribute__((aligned(32))) //__attribute__((section(".video_data")))
uint8_t lcd_bg_buffer[LCD_BG_WIDTH * LCD_BG_HEIGHT * 3];
/* Lcd Foreground Buffer */
__attribute__((aligned(32))) //__attribute__((section(".video_data")))
static uint8_t lcd_fg_buffer[2][LCD_FG_WIDTH * LCD_FG_HEIGHT * 2];
/* screen buffer */
__attribute__((aligned(32))) //__attribute__((section(".video_data")))
static uint8_t screen_buffer[LCD_FG_WIDTH * LCD_FG_HEIGHT * 3];
#endif
#if 0
__attribute__ ((aligned (32))) //__attribute__((section(".video_data")))
static uint8_t screen_buffer_alt[LCD_FG_WIDTH * LCD_FG_HEIGHT*2];
#endif

int Display_Demo_Init(uint32_t screen_size[2])
{
	SCRL_LayerConfig layers_config[2] = {
		{
			.origin = { lcd_bg_area.X0, lcd_bg_area.Y0 },
			.size = { lcd_bg_area.XSize, lcd_bg_area.YSize },
			.format = SCRL_RGB888,
			.address = lcd_bg_buffer,
		},
		{
			.origin = { lcd_fg_area.X0, lcd_fg_area.Y0 },
			.size = { lcd_fg_area.XSize, lcd_fg_area.YSize },
			.format = SCRL_ARGB4444,
			.address = lcd_fg_buffer[lcd_fg_buffer_rd_idx],
		},
	};

	SCRL_ScreenConfig screen_config = {
		.size = { LCD_FG_WIDTH, LCD_FG_HEIGHT },
#ifdef SCR_LIB_USE_SPI
		.format = SCRL_RGB565,
#else
		.format =
			SCRL_RGB565, /* Use SCRL_RGB565 if host support this format to reduce cpu load */
#endif
		.address = screen_buffer,
		.fps = SCREEN_FPS,
	};
	int ret;
	screen_size[0] = screen_config.size.width;
	screen_size[1] = screen_config.size.height;

	/* Initialize the LCD to black */
#ifdef SCR_LIB_USE_SPI
	memset(screen_buffer, 0, sizeof(screen_buffer));
	SCB_CleanDCache_by_Addr(screen_buffer, sizeof(screen_buffer));
#else
	uint32_t *p_screen_buffer = (uint32_t *)screen_buffer;
	for (int i = 0; i < sizeof(screen_buffer) / 4; i++) {
		p_screen_buffer[i] = 0x0; //0x80108010;
	}
	for (int x = 0; x < 20; x++)
		for (int y = 0; y < LCD_FG_HEIGHT; y++)
			screen_buffer[x * 10 + y * LCD_FG_WIDTH * 2] = 0xFF;
	SCB_CleanDCache_by_Addr(screen_buffer, sizeof(screen_buffer));
#endif

	ret = SCRL_Init((SCRL_LayerConfig *[2]){ &layers_config[0],
						 &layers_config[1] },
			&screen_config);
	assert(ret == 0);
	UTIL_LCD_SetLayer(SCRL_LAYER_1);
	UTIL_LCD_Clear(UTIL_LCD_COLOR_TRANSPARENT);
	UTIL_LCD_SetFont(&Font12);
	UTIL_LCD_SetBackColor(0x40000000);
	UTIL_LCD_SetTextColor(UTIL_LCD_COLOR_WHITE);
}

int Display_Demo_Update_Buffer(void)
{
	__disable_irq();
	int ret = SCRL_SetAddress_NoReload(lcd_fg_buffer[lcd_fg_buffer_rd_idx],
					   SCRL_LAYER_1);
	assert(ret == HAL_OK);

	__enable_irq();
	return ret;
}
int Display_Demo_Refresh(void)
{
	SCB_CleanDCache_by_Addr(lcd_fg_buffer[lcd_fg_buffer_rd_idx],
				LCD_FG_FRAMEBUFFER_SIZE);
	__disable_irq();
	int ret = SCRL_ReloadLayer(SCRL_LAYER_1);
	assert(ret == HAL_OK);
	__enable_irq();
	lcd_fg_buffer_rd_idx = 1 - lcd_fg_buffer_rd_idx;
	return ret;
}

int Display_Demo_FG_Erase(void)
{
	memset(lcd_fg_buffer, 0, sizeof(lcd_bg_buffer));
}
uint8_t *Display_Demo_BG_Buffer(void)
{
	return lcd_bg_buffer;
}

int Display_Demo_Update(void)
{
	SRCL_Update();
}
volatile int display_demo_count_old = 0;
volatile int display_demo_count = 0;
int Display_Demo_BG_Test(void)
{
	lcd_bg_buffer[1] = 0xFF;
	lcd_bg_buffer[4] = 0xFF;
	for (int y = 0; y < LCD_FG_HEIGHT; y++)
		lcd_bg_buffer[((display_demo_count - 20 + LCD_FG_WIDTH) %
			       LCD_FG_WIDTH) *
				      10 +
			      y * LCD_FG_WIDTH * 3] = 0;
	//for(int x=0; x<20; x++)

	for (int y = 0; y < LCD_FG_HEIGHT; y++)
		lcd_bg_buffer[((display_demo_count) % LCD_FG_WIDTH) * 10 +
			      y * LCD_FG_WIDTH * 3] = 128;
}

int Display_Demo_Text(void)
{
	Display_Demo_Update_Buffer();
	UTIL_LCD_Clear(4);

	UTIL_LCD_DrawRect(10, 10, (100 + ((display_demo_count - 1 + 20) % 20)),
			  (100 + ((display_demo_count - 1 + 20) % 20)), 0);
	UTIL_LCD_DrawRect(10, 10, (100 + (display_demo_count % 20)),
			  (100 + (display_demo_count % 20)), 0xFFFFFFFF);

	UTIL_LCDEx_PrintfAt(0, LINE(1), CENTER_MODE, "display count %d",
			    display_demo_count++);
	Display_Demo_Refresh();
	printf("DD T %d\n\r", display_demo_count);
	return 0;
}

/*
 * The code to test the display
 * This loop produces an image made of two frame
 * The two frames are mixed to produce the one sent on display
 * The SRCL API create the mix by calling the hardware unit DMA2D
 * When this unit finishes the composition it calls the USB Video stuff to publish the image
 * From a user perspective
 */
int main_display(void)
{
	printf("========================================\n\r");
	uint32_t screen_size[2];
	Display_Demo_Init(screen_size);
	printf("Display init %d %d\n\r", screen_size[0], screen_size[1]);

	int cnt = 0;
	while (1) {
		printf("cnt%d\n\r", cnt++);
		Display_Demo_BG_Test();
		Display_Demo_Text();
		printf("buff %x %d\n\r", screen_buffer, display_demo_count++);
		SRCL_Update();
		HAL_Delay(200);
	}
	return 0;
}
