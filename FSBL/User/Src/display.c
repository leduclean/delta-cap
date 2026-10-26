#include "camera.h"

#include "FreeRTOS.h"
#include "task.h"

#include "ai.h"
#include "display.h"
#include "scrl.h"
#include "stm32_lcd_ex.h"
#include "stm32n6xx.h"

#define UTIL_LCD_COLOR_TRANSPARENT 0 /**< Transparent color used to clear */

/* Camera frame, in RAM2 (not zeroed at boot, the DCMIPP fills it) */
[[gnu::section(".ram2")]] alignas(32) static uint8_t
	lcd_bg_buffer[SCREEN_SIZE * 2];
/**
 * @brief Background of the display.
 */
static SCRL_LayerConfig bg_layer_cfg = {
	.origin = { 0, 0 },
	.size = { SCREEN_WIDTH, SCREEN_HEIGHT },
	.format = SCRL_RGB565, /* same as the screen, no loss */
	.address = lcd_bg_buffer,
};

/**
 * @brief Double buffer for overlay display.
 */
alignas(32) static uint8_t lcd_fg_buffer[2][SCREEN_SIZE * 2];
static SCRL_LayerConfig fg_layer_cfg = {
	.origin = { 0, 0 },
	.size = { SCREEN_WIDTH, SCREEN_HEIGHT },
	.format = SCRL_ARGB4444, /* alpha needed to overlay on the camera */
	.address = lcd_fg_buffer[0],
};
/**
 * @brief Flag to track the current ready to draw buffer.
 */
static int fg_ready = 0;

/**
 * @brief Buffer used by the DMA2D to refresh the screen.
 */
alignas(32) static uint8_t screen_buffer[SCREEN_SIZE * 2];
static SCRL_ScreenConfig screen_config = {
	.size = { SCREEN_WIDTH, SCREEN_HEIGHT },
	.format = SCRL_RGB565,
	.address = screen_buffer,
	.fps = CAMERA_FPS,
};

int display_init(void)
{
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

/* Point drawing at the back buffer */
static int overlay_begin_draw(void)
{
	taskENTER_CRITICAL();
	int res =
		SCRL_SetAddress_NoReload(lcd_fg_buffer[fg_ready], SCRL_LAYER_1);
	taskEXIT_CRITICAL();
	return res;
}

/* Swap the back buffer with the front buffer */
static int overlay_swap(void)
{
	SCB_CleanDCache_by_Addr(lcd_fg_buffer[fg_ready],
				sizeof(lcd_fg_buffer[0]));
	taskENTER_CRITICAL();
	int res = SCRL_ReloadLayer(SCRL_LAYER_1);
	taskEXIT_CRITICAL();
	fg_ready ^= 1;
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
		UTIL_LCDEx_PrintfAt(sx0, label_y, LEFT_MODE, "cls %s %u%%",
				    d->label, score);
	}

	UTIL_LCD_SetTextColor(UTIL_LCD_COLOR_WHITE);
}

static inline void display_draw_inf_time(int inf_time)
{
	UTIL_LCD_SetTextColor(UTIL_LCD_COLOR_CYAN);
	UTIL_LCDEx_PrintfAt(0, 0, RIGHT_MODE, "NPU+PP: %lu ms", inf_time);
}

int display_add_overlay(const struct ai_result *result)
{
	int res = overlay_begin_draw();
	if (res < 0) {
		return res;
	}

	/* Clear the previous overlay */
	UTIL_LCD_Clear(UTIL_LCD_COLOR_TRANSPARENT);

	/* Add rectangle on the overlay */
	display_draw_detections(result->det, result->n);
	display_draw_inf_time(result->ms);

	/* Display the new overlay */
	return overlay_swap();
}
