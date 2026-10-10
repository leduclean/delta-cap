/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    ramcfg.c
  * @brief   This file provides code for the configuration
  *          of the RAMCFG instances.
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
#include "ramcfg.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

RAMCFG_HandleTypeDef hramcfg_SRAM1;
RAMCFG_HandleTypeDef hramcfg_SRAM2;
RAMCFG_HandleTypeDef hramcfg_SRAM3;
RAMCFG_HandleTypeDef hramcfg_SRAM4;
RAMCFG_HandleTypeDef hramcfg_SRAM5;
RAMCFG_HandleTypeDef hramcfg_SRAM6;

/* RAMCFG init function */
void MX_RAMCFG_Init(void)
{

  /* USER CODE BEGIN RAMCFG_Init 0 */

  /* USER CODE END RAMCFG_Init 0 */

  /* USER CODE BEGIN RAMCFG_Init 1 */

  /* USER CODE END RAMCFG_Init 1 */

  /** Initialize RAMCFG SRAM1
  */
  hramcfg_SRAM1.Instance = RAMCFG_SRAM1_AXI;
  if (HAL_RAMCFG_Init(&hramcfg_SRAM1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initialize RAMCFG SRAM2
  */
  hramcfg_SRAM2.Instance = RAMCFG_SRAM2_AXI;
  if (HAL_RAMCFG_Init(&hramcfg_SRAM2) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_RAMCFG_EnableAXISRAM(&hramcfg_SRAM2);

  /** Initialize RAMCFG SRAM3
  */
  hramcfg_SRAM3.Instance = RAMCFG_SRAM3_AXI;
  if (HAL_RAMCFG_Init(&hramcfg_SRAM3) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_RAMCFG_EnableAXISRAM(&hramcfg_SRAM3);

  /** Initialize RAMCFG SRAM4
  */
  hramcfg_SRAM4.Instance = RAMCFG_SRAM4_AXI;
  if (HAL_RAMCFG_Init(&hramcfg_SRAM4) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_RAMCFG_EnableAXISRAM(&hramcfg_SRAM4);

  /** Initialize RAMCFG SRAM5
  */
  hramcfg_SRAM5.Instance = RAMCFG_SRAM5_AXI;
  if (HAL_RAMCFG_Init(&hramcfg_SRAM5) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_RAMCFG_EnableAXISRAM(&hramcfg_SRAM5);

  /** Initialize RAMCFG SRAM6
  */
  hramcfg_SRAM6.Instance = RAMCFG_SRAM6_AXI;
  if (HAL_RAMCFG_Init(&hramcfg_SRAM6) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_RAMCFG_EnableAXISRAM(&hramcfg_SRAM6);
  /* USER CODE BEGIN RAMCFG_Init 2 */

  /* USER CODE END RAMCFG_Init 2 */

}

void HAL_RAMCFG_MspInit(RAMCFG_HandleTypeDef* ramcfgHandle)
{

  /* USER CODE BEGIN RAMCFG_MspInit 0 */

  /* USER CODE END RAMCFG_MspInit 0 */
    /* RAMCFG clock enable */
    __HAL_RCC_RAMCFG_CLK_ENABLE();
    if (ramcfgHandle->Instance == RAMCFG_SRAM1_AXI)
    {
      __HAL_RCC_AXISRAM1_MEM_CLK_ENABLE();

      HAL_RAMCFG_EnableAXISRAM(ramcfgHandle);
    }
    else if (ramcfgHandle->Instance == RAMCFG_SRAM2_AXI)
    {
      __HAL_RCC_AXISRAM2_MEM_CLK_ENABLE();

      HAL_RAMCFG_EnableAXISRAM(ramcfgHandle);
    }
    else if (ramcfgHandle->Instance == RAMCFG_SRAM3_AXI)
    {
      __HAL_RCC_AXISRAM3_MEM_CLK_ENABLE();

      HAL_RAMCFG_EnableAXISRAM(ramcfgHandle);
    }
    else if (ramcfgHandle->Instance == RAMCFG_SRAM4_AXI)
    {
      __HAL_RCC_AXISRAM4_MEM_CLK_ENABLE();

      HAL_RAMCFG_EnableAXISRAM(ramcfgHandle);
    }
    else if (ramcfgHandle->Instance == RAMCFG_SRAM5_AXI)
    {
      __HAL_RCC_AXISRAM5_MEM_CLK_ENABLE();

      HAL_RAMCFG_EnableAXISRAM(ramcfgHandle);
    }
    else if (ramcfgHandle->Instance == RAMCFG_SRAM6_AXI)
    {
      __HAL_RCC_AXISRAM6_MEM_CLK_ENABLE();

      HAL_RAMCFG_EnableAXISRAM(ramcfgHandle);
    }

  /* USER CODE BEGIN RAMCFG_MspInit 1 */

  /* USER CODE END RAMCFG_MspInit 1 */
}

void HAL_RAMCFG_MspDeInit(RAMCFG_HandleTypeDef* ramcfgHandle)
{

  /* USER CODE BEGIN RAMCFG_MspDeInit 0 */

  /* USER CODE END RAMCFG_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_RAMCFG_CLK_DISABLE();
  /* USER CODE BEGIN RAMCFG_MspDeInit 1 */

  /* USER CODE END RAMCFG_MspDeInit 1 */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

