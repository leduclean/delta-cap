#include <string.h>

#include "FreeRTOS.h"
#include "task.h"

#include "ai.h"
#include "display.h"
#include "scrl.h"
#include "stm32_lcd_ex.h"
#include "stm32n6xx.h"

#define LCD_BG_WIDTH SCREEN_WIDTH
#define LCD_BG_HEIGHT SCREEN_HEIGHT
#define LCD_FG_WIDTH SCREEN_WIDTH
#define LCD_FG_HEIGHT SCREEN_HEIGHT

#define UTIL_LCD_COLOR_TRANSPARENT 0

__attribute__((aligned(32))) //__attribute__((section(".video_data")))
static uint8_t lcd_bg_buffer[LCD_BG_WIDTH * LCD_BG_HEIGHT * 3];

__attribute__((aligned(32))) //__attribute__((section(".video_data")))
static uint8_t lcd_fg_buffer[2][LCD_FG_WIDTH * LCD_FG_HEIGHT * 2];

__attribute__((aligned(32))) //__attribute__((section(".video_data")))
static uint8_t screen_buffer[LCD_FG_WIDTH * LCD_FG_HEIGHT * 3];

static SCRL_LayerConfig bg_layer_cfg = {
	.origin = { 0, 0 },
	.size = { LCD_BG_WIDTH, LCD_BG_HEIGHT },
	.format = SCRL_RGB888,
	.address = lcd_bg_buffer,
};

static SCRL_LayerConfig fg_layer_cfg = {
	.origin = { 0, 0 },
	.size = { LCD_FG_WIDTH, LCD_FG_HEIGHT },
	.format = SCRL_ARGB4444, /* alpha needed to overlay on the camera */
	.address = lcd_fg_buffer[0],
};

static SCRL_ScreenConfig screen_config = {
	.size = { LCD_FG_WIDTH, LCD_FG_HEIGHT },
	.format =
		SCRL_RGB565, /* Use SCRL_RGB565 if host support this format to reduce cpu load */
	.address = screen_buffer,
	.fps = SCREEN_FPS,
};

int display_init(void)
{
	/* Initialize the LCD to black */
	memset(screen_buffer, 0, sizeof(screen_buffer));
	SCB_CleanDCache_by_Addr(screen_buffer, sizeof(screen_buffer));

	int res;
	res = SCRL_Init((SCRL_LayerConfig *[2]){ &bg_layer_cfg, &fg_layer_cfg },
			&screen_config);
	if (res < 0) {
		return res;
	}

	UTIL_LCD_SetLayer(SCRL_LAYER_1);
	UTIL_LCD_Clear(UTIL_LCD_COLOR_TRANSPARENT);
	UTIL_LCD_SetFont(&Font12);
	UTIL_LCD_SetBackColor(0x40000000);
	UTIL_LCD_SetTextColor(UTIL_LCD_COLOR_WHITE);

	return 0;
}

void display_draw(void)
{
	SRCL_Update();
}

uint8_t *display_get_bg_buff(void)
{
	return lcd_bg_buffer;
}

/*
 * Overlay double buffering: SCRL keeps two pointers per layer, the one
 * UTIL_LCD_* draws into and the one DMA2D reads to compose the frame.
 * We draw into the back buffer while DMA2D reads the front one, then swap.
 */
static int fg_back_idx = 0;

/* Point drawing at the back buffer */
static int overlay_begin_draw(void)
{
	taskENTER_CRITICAL();
	int res = SCRL_SetAddress_NoReload(lcd_fg_buffer[fg_back_idx],
					   SCRL_LAYER_1);
	taskEXIT_CRITICAL();
	return res;
}

/* Swap the back buffer with the front buffer */
static int overlay_swap(void)
{
	SCB_CleanDCache_by_Addr(lcd_fg_buffer[fg_back_idx],
				sizeof(lcd_fg_buffer[0]));
	taskENTER_CRITICAL();
	int res = SCRL_ReloadLayer(SCRL_LAYER_1);
	taskEXIT_CRITICAL();
	fg_back_idx = 1 - fg_back_idx;
	return res;
}

static void display_draw_detections(const struct detection *detections,
				    size_t count)
{
	const float scale = 240.0f;
	const float offset_x = 40.0f;

	for (size_t i = 0; i < count; i++) {
		const struct detection *d = &detections[i];

		/* Convertir centre + taille en coins */
		float x0 = d->x - d->w / 2.0f;
		float y0 = d->y - d->h / 2.0f;
		float x1 = d->x + d->w / 2.0f;
		float y1 = d->y + d->h / 2.0f;

		/* Limiter le rectangle a l'image du reseau */
		if (x0 < 0.0f)
			x0 = 0.0f;
		if (y0 < 0.0f)
			y0 = 0.0f;
		if (x1 > 1.0f)
			x1 = 1.0f;
		if (y1 > 1.0f)
			y1 = 1.0f;

		if (x1 <= x0 || y1 <= y0) {
			continue;
		}

		/* Conversion vers l'ecran 320 x 240 */
		uint32_t sx0 = (uint32_t)(offset_x + x0 * scale + 0.5f);
		uint32_t sy0 = (uint32_t)(y0 * scale + 0.5f);
		uint32_t sx1 = (uint32_t)(offset_x + x1 * scale + 0.5f);
		uint32_t sy1 = (uint32_t)(y1 * scale + 0.5f);

		uint32_t largeur = sx1 - sx0;
		uint32_t hauteur = sy1 - sy0;

		/* Dessiner le rectangle */
		UTIL_LCD_DrawRect(sx0, sy0, largeur, hauteur,
				  UTIL_LCD_COLOR_GREEN);

		/* Afficher classe et confiance */
		uint32_t label_y = (sy0 >= 12U) ? sy0 - 12U : sy0;
		unsigned score = (unsigned)(d->conf * 100.0f + 0.5f);

		UTIL_LCD_SetTextColor(UTIL_LCD_COLOR_GREEN);
		UTIL_LCDEx_PrintfAt(sx0, label_y, LEFT_MODE, "cls %d %u%%",
				    d->cls, score);
	}

	UTIL_LCD_SetTextColor(UTIL_LCD_COLOR_WHITE);
}

static int display_count = 0;

int display_add_overlay(struct detection *det, int count)
{
	int res = overlay_begin_draw();
	if (res < 0) {
		return res;
	}

	/* Effacer les dessins de la frame precedente */
	UTIL_LCD_Clear(UTIL_LCD_COLOR_TRANSPARENT);

	/* Affichage du compteur existant */
	UTIL_LCDEx_PrintfAt(0, LINE(1), CENTER_MODE, "display count %d",
			    display_count++);

	/* Dessiner les rectangles sur le calque */
	display_draw_detections(det, count);

	/* Afficher le nouveau calque */
	return overlay_swap();
}
