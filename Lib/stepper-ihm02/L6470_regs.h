#pragma once

/**
 * @file
 * @brief   L6470 register bits used by the motor configuration
 */

/**
 * @name    CONFIG (0x18)
 * @{
 */
#define L6470_CONFIG_EXT_CLK          (1 << 3)      /**< internal 16 MHz osc, 2 MHz on OSCOUT */
#define L6470_CONFIG_SW_MODE          (1 << 4)      /**< SW input only raises SW_EVN, no hard stop */
#define L6470_CONFIG_OC_SD            (1 << 7)      /**< overcurrent shuts the bridges down */
#define L6470_CONFIG_POW_SR_110V_US   (0b10 << 8)   /**< output slew rate 110 V/us */
#define L6470_CONFIG_F_PWM_DEC_X1     (0b011 << 10) /**< PWM multiplication factor 1 */
#define L6470_CONFIG_F_PWM_INT_DIV2   (0b001 << 13) /**< PWM division factor 2 */
/** @} */

/**
 * @name    ALARM_EN (0x17)
 * @{
 */
#define L6470_ALARM_EN_ALL            (0xFF)        /**< every alarm pulls FLAG low */
/** @} */
