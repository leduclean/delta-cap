/**
  ******************************************************************************
  * @file    xnucleoihm02a1_interface.h
  * @brief   This file is used as header of the interface between the 
  *          X-NUCLEO-IHM02A1 and the NUCLEO-F4xx board.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2014 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */ 

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __XNUCLEO_INTERFACE_H
#define __XNUCLEO_INTERFACE_H
#include "main.h"

/*
 * BEWARE
 * The pinout is modified beacuse Nucleo N6 analog arduino is in conflict with
 * IHM02 pinout. Some pins of IHM02 are on analog connector but NucleoN6 Analog is 1.8V
 * To correct the problem, NucleoN6 outputs and inputs have to be redirected to digital one
 * At the moment, to digital arduino
 * Later, use of ST connector to strap more closer (at the bottom of NucleoN6)
 * https://www.st.com/en/evaluation-tools/x-nucleo-ihm02a1.html
 */


#define HSPI                                  hspi5
extern SPI_HandleTypeDef hspi5;

/* GPIO settings for the nSTBY_nRST pins of the L6470 */
#define L6470_nSTBY_nRST_GPIO_PORT              STEPPER_RESET_N_GPIO_Port                 //!< nSTBY_nRST, GPIO Port
#define L6470_nSTBY_nRST_GPIO_PIN               STEPPER_RESET_N_Pin            //!< nSTBY_nRST, GPIO Pin

/* GPIO settings for the nBUSY_SYNC pin of the L6470 */
#define L6470_nBUSY_SYNC_GPIO_PORT              STEPPER_BUSY_N_GPIO_Port                 //!< nSTBY_nRST, GPIO Port
#define L6470_nBUSY_SYNC_GPIO_PIN               STEPPER_BUSY_N_Pin            //!< nSTBY_nRST, GPIO Pin
#define L6470_nBUSY_SYNC_IRQn                   EXTI5_IRQn            //!< nSTBY_nRST, IRQ number
#define L6470_nBUSY_SYNC_EXTI_IRQHandler        EXTI5_IRQHandler      //!< nSTBY_nRST, IRQ handler name

/* GPIO settings for the nFLAG pin of the L6470 */
#define L6470_nFLAG_GPIO_PORT                   STEPPER_FLAG_N_GPIO_Port                 //!< nFLAG, GPIO Port
#define L6470_nFLAG_GPIO_PIN                    STEPPER_FLAG_N_Pin            //!< nFLAG, GPIO Pin
#define L6470_nFLAG_IRQn                        EXTI7_IRQn            //!< nFLAG, IRQ number
#define L6470_nFLAG_EXTI_IRQHandler             EXTI7_IRQHandler      //!< nFLAG, IRQ handler name

/* GPIO settings for the options about the nCS pins of the L6470 */
#define L6470_nCS_OP0_GPIO_CLK_ENABLE           RCC_AHB1ENR_GPIOAEN   //!< nCS Option 0, RCC AHB peripheral clock register
#define L6470_nCS_OP0_GPIO_PORT                 STEPPER_CS_N_GPIO_Port                 //!< nCS Option 0, GPIO Port
#define L6470_nCS_OP0_GPIO_PIN                  STEPPER_CS_N_Pin            //!< nCS Option 0, GPIO Pin
#if 0
#define L6470_nCS_OP1_GPIO_CLK_ENABLE           RCC_AHB1ENR_GPIOAEN   //!< nCS Option 1, RCC AHB peripheral clock register
#define L6470_nCS_OP1_GPIO_PORT                 GPIOA                 //!< nCS Option 1, GPIO Port
#define L6470_nCS_OP1_GPIO_PIN                  GPIO_PIN_10           //!< nCS Option 1, GPIO Pin
#define L6470_nCS_OP2_GPIO_CLK_ENABLE           RCC_AHB1ENR_GPIOBEN   //!< nCS Option 2, RCC AHB peripheral clock register
#define L6470_nCS_OP2_GPIO_PORT                 GPIOB                 //!< nCS Option 2, GPIO Port
#define L6470_nCS_OP2_GPIO_PIN                  GPIO_PIN_6            //!< nCS Option 2, GPIO Pin
#define L6470_nCS_OP3_GPIO_CLK_ENABLE           RCC_AHB1ENR_GPIOBEN   //!< nCS Option 3, RCC AHB peripheral clock register
#define L6470_nCS_OP3_GPIO_PORT                 GPIOB                 //!< nCS Option 3, GPIO Port
#define L6470_nCS_OP3_GPIO_PIN                  GPIO_PIN_4            //!< nCS Option 3, GPIO Pin
#endif

 typedef struct {
  GPIO_TypeDef* port;         //!< GPIO Port
  uint16_t pin;               //!< GPIO Pin
} sL6470_GPIO;

extern sL6470_GPIO L6470_nSTBY_nRST_GPIO;
extern sL6470_GPIO L6470_nBUSY_SYNC_GPIO;
extern sL6470_GPIO L6470_nFLAG_GPIO;
extern sL6470_GPIO L6470_nCS_GPIO;
extern sL6470_GPIO L6470_nCS_GPIO_Array[];
#endif /* __XNUCLEO_INTERFACE_H */


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
