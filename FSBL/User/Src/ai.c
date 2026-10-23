#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"

#include "ai.h"
#include "app_config.h"
#include "app_postprocess.h"
#include "ll_aton_rt_user_api.h"
#include "npu_cache.h"
#include "od_pp_output_if.h"
#include "stai.h"
#include "stm32n6xx_nucleo_xspi.h"

#define AI_YOLOX_OUT_NUM 3

#ifndef AI_IMAGE_ADDR
/** Address of the runtime loadable image of the model, = ADDR in training/deploy/generate.sh */
#define AI_IMAGE_ADDR 0x70380000
#endif

/** Allocated ram to install the reloc model, in npuRAM3 (FSBL RAM is full
 * with the display buffers). Not zeroed at boot, the install writes it. */
#define EXEC_RAM_SIZE (300 * 1024)
__attribute__((section(".npuram3"),
	       aligned(32))) static uint8_t exec_ram[EXEC_RAM_SIZE];
static ll_aton_reloc_config reloc_config = {
	.exec_ram_addr = (uintptr_t)exec_ram,
	.exec_ram_size = sizeof(exec_ram),
	.mode = AI_RELOC_RT_LOAD_MODE_XIP,
};

/**
 * @brief Instance of the model feeded at install
 */
static NN_Instance_TypeDef nn_instance;

static stai_tensor out_tensors[AI_YOLOX_OUT_NUM];
/**
 * @brief Info for post processing.
 */
static stai_network_info nn_info = { .n_inputs = 1,
				     .n_outputs = AI_YOLOX_OUT_NUM,
				     .outputs = out_tensors };

static od_st_yolox_pp_static_param_t pp_params;
static od_pp_out_t pp_output;

/* Each pointer correspond to an out class */
static stai_ptr nn_out[AI_YOLOX_OUT_NUM];
static int32_t nn_out_size[AI_YOLOX_OUT_NUM];

static stai_ptr nn_in;
static int32_t nn_in_size;

static int last_n;
static struct detection last_det[AI_OD_ST_YOLOX_PP_MAX_BOXES_LIMIT];

/**
 * @brief Enable and reset the clk of npu cache
 * @note This function overrides the __weak one in
 * npu_cache.c.
 */
void npu_cache_enable_clocks_and_reset(void)
{
	__HAL_RCC_CACHEAXIRAM_MEM_CLK_ENABLE();
	__HAL_RCC_CACHEAXI_CLK_ENABLE();
	__HAL_RCC_CACHEAXI_FORCE_RESET();
	__HAL_RCC_CACHEAXI_RELEASE_RESET();
}

/**
 * @brief Disbale and reset the clk of npu cache
 * @note This function overrides the __weak one in
 * npu_cache.c.
 */
void npu_cache_disable_clocks_and_reset(void)
{
	__HAL_RCC_CACHEAXIRAM_MEM_CLK_DISABLE();
	__HAL_RCC_CACHEAXI_CLK_DISABLE();
	__HAL_RCC_CACHEAXI_FORCE_RESET();
}

static void npu_clk_enable(void)
{
	__HAL_RCC_NPU_CLK_ENABLE();
	__HAL_RCC_NPU_FORCE_RESET();
	__HAL_RCC_NPU_RELEASE_RESET();
}

/**
 * @brief Init the xspi which is the core controller for the
 * external ram where weights are stocked.
 */
[[nodiscard]] static int init_xspi(void)
{
	BSP_XSPI_NOR_Init_t nor = {
		.InterfaceMode = BSP_XSPI_NOR_OPI_MODE,
		.TransferRate = BSP_XSPI_NOR_DTR_TRANSFER,
	};

	int32_t bsp = BSP_XSPI_NOR_Init(0, &nor);
	if (bsp != BSP_ERROR_NONE) {
		return -EIO;
	}

	/* High Speed Low voltage fuse seems not programmed, hence the 1.8V xSPI IOs can't do 200 MHz.
     * Run at 200/(1+1) = 100 MHz: weight reads 2x slower*/
	if (HAL_XSPI_SetClockPrescaler(&hxspi_nor[0], 1) != HAL_OK) {
		return -EIO;
	}

	bsp = BSP_XSPI_NOR_EnableMemoryMappedMode(0);
	if (bsp != BSP_ERROR_NONE) {
		return -EIO;
	}

	return 0;
}

static bool is_buffer(const LL_Buffer_InfoTypeDef *info)
{
	return info != NULL && info->name != NULL;
}

[[nodiscard]] static int get_input_infos(void)
{
	const LL_Buffer_InfoTypeDef *buf_info;

	buf_info = ll_aton_reloc_get_input_buffers_info(&nn_instance, 0);
	if (!is_buffer(buf_info)) {
		return -EIO; // No input
	}
	nn_in = LL_Buffer_addr_start(buf_info);
	nn_in_size = LL_Buffer_len(buf_info);

	if (is_buffer(ll_aton_reloc_get_input_buffers_info(&nn_instance, 1))) {
		return -EIO; // More input than a YOLOX
	}
	return 0;
}

static inline stai_tensor tensor_from_info(const LL_Buffer_InfoTypeDef *info)
{
	return (stai_tensor){
		.size_bytes = LL_Buffer_len(info),
		.shape = { info->ndims, (const int32_t *)info->shape },
		.scale = { 1, info->scale },
		.zeropoint = { 1, info->offset },
		.name = info->name,
	};
}

[[nodiscard]] static int get_output_infos(void)
{
	const LL_Buffer_InfoTypeDef *buf_info;

	for (int i = 0; i < AI_YOLOX_OUT_NUM; i++) {
		buf_info =
			ll_aton_reloc_get_output_buffers_info(&nn_instance, i);
		if (!is_buffer(buf_info)) {
			return -EIO; // Fewer output than a YOLOX
		}
		nn_out[i] = LL_Buffer_addr_start(buf_info);
		nn_out_size[i] = LL_Buffer_len(buf_info);
		out_tensors[i] = tensor_from_info(buf_info);
	}

	if (is_buffer(ll_aton_reloc_get_output_buffers_info(
		    &nn_instance, AI_YOLOX_OUT_NUM))) {
		return -EIO; // More output than a YOLOX
	}
	return 0;
}

[[nodiscard]] static int nn_init(void)
{
	ll_aton_reloc_info rt;
	int res;

	res = ll_aton_reloc_get_info(AI_IMAGE_ADDR, &rt);
	if (res != AI_RELOC_RT_ERR_NONE) {
		return -EIO;
	}

	if (rt.rt_ram_xip > sizeof(exec_ram)) {
		// Not enough exec ram to install.
		return -ENOMEM;
	}

	/*WARNING: Before the install: it creates the OSAL cache mutex the install takes */
	LL_ATON_RT_RuntimeInit();

	res = ll_aton_reloc_install(AI_IMAGE_ADDR, &reloc_config, &nn_instance);
	if (res != AI_RELOC_RT_ERR_NONE) {
		return -EIO;
	}

	res = get_input_infos();
	if (res < 0) {
		return res;
	}

	res = get_output_infos();
	if (res < 0) {
		return res;
	}

	LL_ATON_RT_Init_Network(&nn_instance);

	return 0;
}

int ai_init(void)
{
	int res;

	npu_clk_enable();
	npu_cache_enable();
	res = init_xspi();
	if (res < 0) {
		return res;
	}

	res = nn_init();
	if (res < 0) {
		return res;
	}

	res = app_postprocess_init(&pp_params, &nn_info);
	if (res != AI_OD_POSTPROCESS_ERROR_NO) {
		return -EIO;
	}
	return 0;
}

void ai_deinit(void)
{
	LL_ATON_RT_DeInit_Network(&nn_instance);
	LL_ATON_RT_RuntimeDeInit();
}

void *ai_input(void)
{
	return nn_in;
}

int32_t ai_input_size(void)
{
	return nn_in_size;
}

int ai_run(void)
{
	/* Camera DMA wrote nn_in: push it out of the MCU cache before the NPU reads it */
	SCB_CleanDCache_by_Addr(nn_in, nn_in_size);

	LL_ATON_RT_RetValues_t ll_aton_ret;
	LL_ATON_RT_Reset_Network(&nn_instance);
	do {
		ll_aton_ret = LL_ATON_RT_RunEpochBlock(&nn_instance);
		if (ll_aton_ret == LL_ATON_RT_WFE) {
			LL_ATON_OSAL_WFE();
		}
	} while (ll_aton_ret != LL_ATON_RT_DONE);

	/* NPU wrote the outputs: drop stale MCU cache lines before reading them */
	for (int i = 0; i < AI_YOLOX_OUT_NUM; i++) {
		SCB_InvalidateDCache_by_Addr(nn_out[i], nn_out_size[i]);
	}

	if (app_postprocess_run((void **)nn_out, AI_YOLOX_OUT_NUM, &pp_output,
				&pp_params) != AI_OD_POSTPROCESS_ERROR_NO) {
		return -EIO;
	}

	taskENTER_CRITICAL();
	int n = pp_output.nb_detect;
	for (int i = 0; i < n; i++) {
		const od_pp_outBuffer_t *b = &pp_output.pOutBuff[i];
		last_det[i] = (struct detection){ .h = b->height,
						  .w = b->width,
						  .x = b->x_center,
						  .y = b->y_center,
						  .conf = b->conf,
						  .cls = b->class_index };
	}
	last_n = n;
	taskEXIT_CRITICAL();

	return 0;
}

int ai_get_detections(struct detection *out, int max)
{
	taskENTER_CRITICAL();
	int n = max < last_n ? max : last_n;
	for (int i = 0; i < n; i++) {
		out[i] = last_det[i];
	}
	taskEXIT_CRITICAL();
	return n;
}
