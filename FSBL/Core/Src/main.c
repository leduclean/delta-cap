/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os2.h"
#include "app_freertos.h"
#include "dcmipp.h"
#include "i2c.h"
#include "icache.h"
#include "ramcfg.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "UART_Console.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */
extern DMA_HandleTypeDef handle_GPDMA1_Channel0;
extern DMA_HandleTypeDef handle_GPDMA1_Channel1;
extern DMA_HandleTypeDef handle_GPDMA1_Channel2;
extern DMA_HandleTypeDef handle_GPDMA1_Channel3;

/* Private variables ---------------------------------------------------------*/

COM_InitTypeDef BspCOMInit;
__IO uint32_t BspButtonState = BUTTON_RELEASED;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void SystemIsolation_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
	/* USER CODE BEGIN 1 */

	/* USER CODE END 1 */

	/* Enable the CPU Cache */

	/* Enable I-Cache---------------------------------------------------------*/
	SCB_EnableICache();

	/* Enable D-Cache---------------------------------------------------------*/
	SCB_EnableDCache();

	/* MCU Configuration--------------------------------------------------------*/
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_GPDMA1_Init();
	//MX_USB1_OTG_HS_PCD_Init//_DISABLED();
	//MX_USBX_Init//_DISABLED();
	//MX_DMA2D_Init//_DISABLED();
	MX_I2C1_Init();
	MX_RAMCFG_Init();
	MX_ICACHE_Init();
	MX_DCMIPP_Init();
	SystemIsolation_Config();
	/* USER CODE BEGIN 2 */

	/* USER CODE END 2 */

	/* Init scheduler */
	osKernelInitialize();
	/* Call init function for freertos objects (in app_freertos.c) */
	MX_FREERTOS_Init();

	/* Initialize leds */
	BSP_LED_Init(LED_BLUE);
	BSP_LED_Init(LED_RED);
	BSP_LED_Init(LED_GREEN);

	/* Initialize USER push-button, will be used to trigger an interrupt each time it's pressed.*/
	BSP_PB_Init(BUTTON_USER, BUTTON_MODE_EXTI);
#if 0
  /* Initialize COM1 port (115200, 8 bits (7-bit data + 1 stop bit), no parity */
  BspCOMInit.BaudRate   = 115200;
  BspCOMInit.WordLength = COM_WORDLENGTH_8B;
  BspCOMInit.StopBits   = COM_STOPBITS_1;
  BspCOMInit.Parity     = COM_PARITY_NONE;
  BspCOMInit.HwFlowCtl  = COM_HWCONTROL_NONE;
  if (BSP_COM_Init(COM1, &BspCOMInit) != BSP_ERROR_NONE)
  {
    Error_Handler();
  }
#endif
	/* USER CODE BEGIN BSP */
	UART_DMA_Init(COM1);

	/* -- Sample board code to send message over COM1 port ---- */
	printf("Welcome to STM32 world !\n\rBoot project is running...\n\r");

	/* -- Sample board code to switch on leds ---- */
	BSP_LED_On(LED_BLUE);
	BSP_LED_On(LED_RED);
	BSP_LED_On(LED_GREEN);

	/* USER CODE END BSP */

	/* Start scheduler */
	osKernelStart();

	/* We should never get here as control is now taken by the scheduler */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
	}
	/* USER CODE END 3 */
}
/* USER CODE BEGIN CLK 1 */
/* USER CODE END CLK 1 */

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
	RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = { 0 };
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Configure the System Power Supply
  */
	if (HAL_PWREx_ConfigSupply(PWR_EXTERNAL_SOURCE_SUPPLY) != HAL_OK) {
		Error_Handler();
	}

	/** Configure the main internal regulator output voltage
  */
	if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE0) !=
	    HAL_OK) {
		Error_Handler();
	}

	/* Enable HSI */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL1.PLLState = RCC_PLL_NONE;
	RCC_OscInitStruct.PLL2.PLLState = RCC_PLL_NONE;
	RCC_OscInitStruct.PLL3.PLLState = RCC_PLL_NONE;
	RCC_OscInitStruct.PLL4.PLLState = RCC_PLL_NONE;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Initializes TIMPRE when TIM is used as Systick Clock Source
  */
	PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_TIM;
	PeriphClkInitStruct.TIMPresSelection = RCC_TIMPRES_DIV1;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Get current CPU/System buses clocks configuration and if necessary switch
 to intermediate HSI clock to ensure target clock can be set
  */
	HAL_RCC_GetClockConfig(&RCC_ClkInitStruct);
	if ((RCC_ClkInitStruct.CPUCLKSource == RCC_CPUCLKSOURCE_IC1) ||
	    (RCC_ClkInitStruct.SYSCLKSource == RCC_SYSCLKSOURCE_IC2_IC6_IC11)) {
		RCC_ClkInitStruct.ClockType =
			(RCC_CLOCKTYPE_CPUCLK | RCC_CLOCKTYPE_SYSCLK);
		RCC_ClkInitStruct.CPUCLKSource = RCC_CPUCLKSOURCE_HSI;
		RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
		if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct) != HAL_OK) {
			/* Initialization Error */
			Error_Handler();
		}
	}

	/** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS_DIGITAL;
	RCC_OscInitStruct.PLL1.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL1.PLLSource = RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL1.PLLM = 2;
	RCC_OscInitStruct.PLL1.PLLN = 25;
	RCC_OscInitStruct.PLL1.PLLFractional = 0;
	RCC_OscInitStruct.PLL1.PLLP1 = 1;
	RCC_OscInitStruct.PLL1.PLLP2 = 1;
	RCC_OscInitStruct.PLL2.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL2.PLLSource = RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL2.PLLM = 8;
	RCC_OscInitStruct.PLL2.PLLN = 125;
	RCC_OscInitStruct.PLL2.PLLFractional = 0;
	RCC_OscInitStruct.PLL2.PLLP1 = 1;
	RCC_OscInitStruct.PLL2.PLLP2 = 1;
	RCC_OscInitStruct.PLL3.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL3.PLLSource = RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL3.PLLM = 8;
	RCC_OscInitStruct.PLL3.PLLN = 225;
	RCC_OscInitStruct.PLL3.PLLFractional = 0;
	RCC_OscInitStruct.PLL3.PLLP1 = 1;
	RCC_OscInitStruct.PLL3.PLLP2 = 2;
	RCC_OscInitStruct.PLL4.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL4.PLLSource = RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL4.PLLM = 8;
	RCC_OscInitStruct.PLL4.PLLN = 225;
	RCC_OscInitStruct.PLL4.PLLFractional = 0;
	RCC_OscInitStruct.PLL4.PLLP1 = 6;
	RCC_OscInitStruct.PLL4.PLLP2 = 6;

	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
  */
	RCC_ClkInitStruct.ClockType =
		RCC_CLOCKTYPE_CPUCLK | RCC_CLOCKTYPE_HCLK |
		RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 |
		RCC_CLOCKTYPE_PCLK2 | RCC_CLOCKTYPE_PCLK5 | RCC_CLOCKTYPE_PCLK4;
	RCC_ClkInitStruct.CPUCLKSource = RCC_CPUCLKSOURCE_IC1;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_IC2_IC6_IC11;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV1;
	RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV1;
	RCC_ClkInitStruct.APB5CLKDivider = RCC_APB5_DIV1;
	RCC_ClkInitStruct.IC1Selection.ClockSelection = RCC_ICCLKSOURCE_PLL1;
	RCC_ClkInitStruct.IC1Selection.ClockDivider = 1;
	RCC_ClkInitStruct.IC2Selection.ClockSelection = RCC_ICCLKSOURCE_PLL1;
	RCC_ClkInitStruct.IC2Selection.ClockDivider = 2;
	RCC_ClkInitStruct.IC6Selection.ClockSelection = RCC_ICCLKSOURCE_PLL2;
	RCC_ClkInitStruct.IC6Selection.ClockDivider = 1;
	RCC_ClkInitStruct.IC11Selection.ClockSelection = RCC_ICCLKSOURCE_PLL3;
	RCC_ClkInitStruct.IC11Selection.ClockDivider = 1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct) != HAL_OK) {
		Error_Handler();
	}
}

/**
  * @brief RIF Initialization Function
  * @param None
  * @retval None
  */
static void SystemIsolation_Config(void)
{
	/* USER CODE BEGIN RIF_Init 0 */

	/* USER CODE END RIF_Init 0 */

	/* set all required IPs as secure privileged */
	__HAL_RCC_RIFSC_CLK_ENABLE();

	/*RIMC configuration*/
	RIMC_MasterConfig_t RIMC_master = { 0 };
	RIMC_master.MasterCID = RIF_CID_1;
	RIMC_master.SecPriv = RIF_ATTRIBUTE_SEC | RIF_ATTRIBUTE_PRIV;
	HAL_RIF_RIMC_ConfigMasterAttributes(RIF_MASTER_INDEX_DCMIPP,
					    &RIMC_master);

	HAL_RIF_RIMC_ConfigMasterAttributes(RIF_MASTER_INDEX_DMA2D,
					    &RIMC_master);

	RIMC_master.SecPriv = RIF_ATTRIBUTE_SEC | RIF_ATTRIBUTE_NPRIV;
	HAL_RIF_RIMC_ConfigMasterAttributes(RIF_MASTER_INDEX_ETH1,
					    &RIMC_master);

	RIMC_master.MasterCID = RIF_CID_0;
	RIMC_master.SecPriv = RIF_ATTRIBUTE_NSEC | RIF_ATTRIBUTE_PRIV;
	HAL_RIF_RIMC_ConfigMasterAttributes(RIF_MASTER_INDEX_LTDC1,
					    &RIMC_master);

	HAL_RIF_RIMC_ConfigMasterAttributes(RIF_MASTER_INDEX_LTDC2,
					    &RIMC_master);

	/*RISUP configuration*/
	HAL_RIF_RISC_SetSlaveSecureAttributes(RIF_RISC_PERIPH_INDEX_OTG1HS,
					      RIF_ATTRIBUTE_SEC |
						      RIF_ATTRIBUTE_PRIV);
	HAL_RIF_RISC_SetSlaveSecureAttributes(RIF_RISC_PERIPH_INDEX_ADC12,
					      RIF_ATTRIBUTE_SEC |
						      RIF_ATTRIBUTE_PRIV);
	HAL_RIF_RISC_SetSlaveSecureAttributes(RIF_RISC_PERIPH_INDEX_DCMIPP,
					      RIF_ATTRIBUTE_SEC |
						      RIF_ATTRIBUTE_PRIV);
	HAL_RIF_RISC_SetSlaveSecureAttributes(RIF_RISC_PERIPH_INDEX_DMA2D,
					      RIF_ATTRIBUTE_SEC |
						      RIF_ATTRIBUTE_PRIV);
	HAL_RIF_RISC_SetSlaveSecureAttributes(RIF_RISC_PERIPH_INDEX_LTDCL1,
					      RIF_ATTRIBUTE_SEC |
						      RIF_ATTRIBUTE_PRIV);
	HAL_RIF_RISC_SetSlaveSecureAttributes(RIF_RISC_PERIPH_INDEX_LTDCL2,
					      RIF_ATTRIBUTE_SEC |
						      RIF_ATTRIBUTE_PRIV);
	HAL_RIF_RISC_SetSlaveSecureAttributes(RIF_RISC_PERIPH_INDEX_NPU,
					      RIF_ATTRIBUTE_SEC |
						      RIF_ATTRIBUTE_PRIV);

	/* RISAF Config */
	RISAF_BaseRegionConfig_t risaf_base_config;
	__HAL_RCC_RISAF_CLK_ENABLE();

	/* set up base region configuration for CPUAXI_RAM1*/
	/* region1 is secure */
	risaf_base_config.EndAddress = 0xfffff;
	risaf_base_config.Filtering = RISAF_FILTER_ENABLE;
	risaf_base_config.ReadWhitelist = 2;
	risaf_base_config.WriteWhitelist = 2;
	risaf_base_config.Secure = RIF_ATTRIBUTE_SEC;
	risaf_base_config.PrivWhitelist = RIF_CID_NONE;
	risaf_base_config.StartAddress = 0x00000000;
	HAL_RIF_RISAF_ConfigBaseRegion(RISAF3, RISAF_REGION_1,
				       &risaf_base_config);

	/* set up base region configuration for CPUAXI_RAM0*/
	/* region2 is secure */
	HAL_RIF_RISAF_ConfigBaseRegion(RISAF2, RISAF_REGION_2,
				       &risaf_base_config);

	RISAF_SubRegionConfig_t risaf_sub_config;

	/* set up Sub region configuration for CPUAXI_RAM1*/
	risaf_sub_config.EndAddress = 0xfffff;
	risaf_sub_config.Filtering = RISAF_FILTER_DISABLE;
	risaf_sub_config.SecPriv = RIF_ATTRIBUTE_SEC | RIF_ATTRIBUTE_NPRIV;
	risaf_sub_config.ReadWrite = RISAF_READ_ENABLE | RISAF_WRITE_ENABLE;
	risaf_sub_config.StartAddress = 0x00000000;
	risaf_sub_config.Lock = RIF_LOCK_DISABLE;
	HAL_RIF_RISAF_ConfigSubRegion(RISAF3, RISAF_REGION_1, RISAF_SUBREGION_A,
				      &risaf_sub_config);

	/* set up Sub region configuration for CPUAXI_RAM0*/
	HAL_RIF_RISAF_ConfigSubRegion(RISAF2, RISAF_REGION_2, RISAF_SUBREGION_A,
				      &risaf_sub_config);

	/* RIF-Aware IPs Config */

	/* set up GPDMA configuration */
	/* set GPDMA1 channel 0 used by GPDMA1 */
	if (HAL_DMA_ConfigChannelAttributes(
		    &handle_GPDMA1_Channel0,
		    DMA_CHANNEL_SEC | DMA_CHANNEL_NPRIV | DMA_CHANNEL_SRC_SEC |
			    DMA_CHANNEL_DEST_SEC) != HAL_OK) {
		Error_Handler();
	}
	/* set GPDMA1 channel 1 used by GPDMA1 */
	if (HAL_DMA_ConfigChannelAttributes(
		    &handle_GPDMA1_Channel1,
		    DMA_CHANNEL_SEC | DMA_CHANNEL_NPRIV | DMA_CHANNEL_SRC_SEC |
			    DMA_CHANNEL_DEST_SEC) != HAL_OK) {
		Error_Handler();
	}
	/* set GPDMA1 channel 2 used by GPDMA1 */
	if (HAL_DMA_ConfigChannelAttributes(
		    &handle_GPDMA1_Channel2,
		    DMA_CHANNEL_SEC | DMA_CHANNEL_PRIV | DMA_CHANNEL_SRC_SEC |
			    DMA_CHANNEL_DEST_SEC) != HAL_OK) {
		Error_Handler();
	}
	/* set GPDMA1 channel 3 used by GPDMA1 */
	if (HAL_DMA_ConfigChannelAttributes(
		    &handle_GPDMA1_Channel3,
		    DMA_CHANNEL_SEC | DMA_CHANNEL_PRIV | DMA_CHANNEL_SRC_SEC |
			    DMA_CHANNEL_DEST_SEC) != HAL_OK) {
		Error_Handler();
	}

	/* set up GPIO configuration */
	HAL_GPIO_ConfigPinAttributes(GPIOB, GPIO_PIN_0,
				     GPIO_PIN_SEC | GPIO_PIN_NPRIV);
	HAL_GPIO_ConfigPinAttributes(GPIOB, GPIO_PIN_3,
				     GPIO_PIN_SEC | GPIO_PIN_NPRIV);
	HAL_GPIO_ConfigPinAttributes(GPIOB, GPIO_PIN_6,
				     GPIO_PIN_SEC | GPIO_PIN_NPRIV);
	HAL_GPIO_ConfigPinAttributes(GPIOB, GPIO_PIN_7,
				     GPIO_PIN_SEC | GPIO_PIN_NPRIV);
	HAL_GPIO_ConfigPinAttributes(GPIOB, GPIO_PIN_10,
				     GPIO_PIN_SEC | GPIO_PIN_NPRIV);
	HAL_GPIO_ConfigPinAttributes(GPIOB, GPIO_PIN_11,
				     GPIO_PIN_SEC | GPIO_PIN_NPRIV);
	HAL_GPIO_ConfigPinAttributes(GPIOC, GPIO_PIN_1,
				     GPIO_PIN_SEC | GPIO_PIN_PRIV);
	HAL_GPIO_ConfigPinAttributes(GPIOE, GPIO_PIN_3,
				     GPIO_PIN_SEC | GPIO_PIN_NPRIV);
	/* GPIOE Non Secure Ports Clock Enable */
	__HAL_RCC_GPIOE_CLK_ENABLE();
	HAL_GPIO_ConfigPinAttributes(GPIOE, GPIO_PIN_5,
				     GPIO_PIN_NSEC | GPIO_PIN_PRIV);
	HAL_GPIO_ConfigPinAttributes(GPIOE, GPIO_PIN_6,
				     GPIO_PIN_NSEC | GPIO_PIN_PRIV);
	/* GPIOF Non Secure Ports Clock Enable */
	__HAL_RCC_GPIOF_CLK_ENABLE();
	HAL_GPIO_ConfigPinAttributes(GPIOF, GPIO_PIN_11,
				     GPIO_PIN_NSEC | GPIO_PIN_PRIV);
	HAL_GPIO_ConfigPinAttributes(GPIOH, GPIO_PIN_9,
				     GPIO_PIN_SEC | GPIO_PIN_PRIV);

	/* USER CODE BEGIN RIF_Init 1 */

	/* USER CODE END RIF_Init 1 */
	/* USER CODE BEGIN RIF_Init 2 */

	/* USER CODE END RIF_Init 2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM16 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	/* USER CODE BEGIN Callback 0 */

	/* USER CODE END Callback 0 */
	if (htim->Instance == TIM16) {
		HAL_IncTick();
	}
	/* USER CODE BEGIN Callback 1 */

	/* USER CODE END Callback 1 */
}

/**
  * @brief BSP Push Button callback
  * @param Button Specifies the pressed button
  * @retval None
  */
void BSP_PB_Callback(Button_TypeDef Button)
{
	if (Button == BUTTON_USER) {
		BspButtonState = BUTTON_PRESSED;
	}
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param None
  * @retval None
  */
void Error_Handler(void)
{
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
	}
	/* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
	/* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
	/* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
