#include "ai.h"
#include "FreeRTOS.h"
#include "app_config.h"
#include "app_postprocess.h"
#include "npu_cache.h"
#include "od_pp_output_if.h"
#include "stai.h"
#include "stai_network.h"
#include "stm32n6xx_nucleo_xspi.h"
#include "task.h"
#include <stdint.h>

STAI_NETWORK_CONTEXT_DECLARE(network_context, STAI_NETWORK_CONTEXT_SIZE)

static od_yolov2_pp_static_param_t pp_params;
static od_pp_out_t pp_output;

static stai_size nbr_outputs;
static_assert(STAI_NETWORK_OUT_NUM == 1,
	      "There should be a single output to the nn");
static stai_ptr nn_out;
static int32_t nn_out_size;

static int last_n;
static struct detection last_det[AI_OD_YOLOV2_PP_MAX_BOXES_LIMIT];

static stai_size nbr_inputs;
static stai_ptr nn_in;
static int32_t nn_in_size;

void *ai_input(void)
{
	return nn_in;
}

int32_t ai_input_size(void)
{
	return nn_in_size;
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

int ai_run(void)
{
	SCB_CleanDCache_by_Addr(nn_in, nn_in_size);
	if (stai_network_run(network_context, STAI_MODE_SYNC) != STAI_SUCCESS) {
		return -EIO;
	}

	if (app_postprocess_run((void **)&nn_out, nbr_outputs, &pp_output,
				&pp_params) != AI_OD_POSTPROCESS_ERROR_NO) {
		return -EIO;
	}
	SCB_InvalidateDCache_by_Addr(nn_out, nn_out_size);

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

[[nodiscard]] static int nn_init(stai_network_info *info)
{
	int res;
	res = stai_runtime_init();
	if (res != STAI_SUCCESS) {
		return -EIO;
	}
	res = stai_network_init(network_context);
	if (res != STAI_SUCCESS) {
		return -EIO;
	}
	res = stai_network_get_info(network_context, info);
	if (res != STAI_SUCCESS) {
		return -EIO;
	}
	if (info->n_inputs != 1) {
		return -EIO;
	}

	res = stai_network_get_inputs(network_context, &nn_in, &nbr_inputs);
	if (res != STAI_SUCCESS) {
		return -EIO;
	}

	res = stai_network_get_outputs(network_context, &nn_out, &nbr_outputs);
	if (res != STAI_SUCCESS) {
		return -EIO;
	}

	return 0;
}

static void extract_info(const stai_network_info *info)
{
	nn_out_size = info->outputs[0].size_bytes;
	nn_in_size = info->inputs[0].size_bytes;
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

	stai_network_info info;
	res = nn_init(&info);
	if (res < 0) {
		return res;
	}

	extract_info(&info);

	res = app_postprocess_init(&pp_params, &info);
	if (res != AI_OD_POSTPROCESS_ERROR_NO) {
		return -EIO;
	}
	return 0;
}
