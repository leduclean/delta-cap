#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>

#include "stm32n6xx_hal.h"
#include "stm32_lcd.h"
#include "stm32_lcd_ex.h"
#include "scrl.h"
#include "display.h"

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

/* Lcd Background Buffer */
__attribute__((aligned(32))) //__attribute__((section(".video_data")))
uint8_t lcd_bg_buffer[LCD_BG_WIDTH * LCD_BG_HEIGHT * 3];
/* Lcd Foreground Buffer */
__attribute__((aligned(32))) //__attribute__((section(".video_data")))
static uint8_t lcd_fg_buffer[2][LCD_FG_WIDTH * LCD_FG_HEIGHT * 2];
/* screen buffer */
__attribute__((aligned(32))) //__attribute__((section(".video_data")))
static uint8_t screen_buffer[LCD_FG_WIDTH * LCD_FG_HEIGHT * 3];

int display_init(uint32_t screen_size[2])
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
	return 0;
}

int display_update_buffer(void)
{
	__disable_irq();
	int ret = SCRL_SetAddress_NoReload(lcd_fg_buffer[lcd_fg_buffer_rd_idx],
					   SCRL_LAYER_1);
	assert(ret == HAL_OK);

	__enable_irq();
	return ret;
}
int display_refresh(void)
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

uint8_t *display_bg_buffer(void)
{
	return lcd_bg_buffer;
}

int display_update(void)
{
	return SRCL_Update();
}
volatile int display_count = 0;
int display_text(void)
{
	display_update_buffer();
	UTIL_LCD_Clear(4);

	UTIL_LCDEx_PrintfAt(0, LINE(1), CENTER_MODE, "display count %d",
			    display_count++);
	display_refresh();
	return 0;
}
