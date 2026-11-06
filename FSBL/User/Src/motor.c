#include <errno.h>
#include <stddef.h>
#include <stdint.h>

#include "cmsis_os2.h"
#include "main.h"
#include "motor.h"
#include "xnucleoihm02a1_params.h"

#include "L6470.h"
#include "microstepping_motor.h"
#include "stm32n6xx_hal_gpio.h"
#include "xnucleoihm02a1.h"

/**
 * @name Motor config constant, should match the init config of each motor.
 * @{
 */
#define MOTOR_CONFIG_STEP_MODE MICROSTEP_1_4 /**< Micro step mode selection. */
#define MOTOR_CONFIG_FSTEP_PREV 200 /**< Minimum nb of steps per revolution.  */
/** @} */

static osThreadId_t evt_waiter;

int motor_init(void)
{
	BSP_L6470_Init();
	MotorParameterData_t *p = GetMotorParameterInitData();
	//TODO: check the parameters of the lib
	for (uint8_t m = 0; m < MOTOR_NUM; m++) {
		BSP_L6470_Config(BSP_L6470_GetExpansionBoardHandle(m),
				 p + m * L6470DAISYCHAINSIZE);
	}

	return 0;
}

static uint32_t deg_to_steps(float target_angle_deg)
{
	uint32_t steps_per_rev = (uint32_t)(MOTOR_CONFIG_FSTEP_PREV *
					    (1 << MOTOR_CONFIG_STEP_MODE));

	float steps_per_deg = (float)steps_per_rev / 360.0f;
	int32_t absolute_steps = (int32_t)(target_angle_deg * steps_per_deg);

	return (uint32_t)absolute_steps;
}

static void prepare_goto(enum motor_t motor, float target_angle_deg)
{
	int32_t absolute_steps = (int32_t)deg_to_steps(target_angle_deg);
	L6470_PrepareGoTo(motor, absolute_steps);
}

/**
 * @name Motor flags used to signal motor event.
 * @{
 */
#define MOTOR_FLAGS_DONE \
	(1 << 0) /**< Flags that indicate that the command is done.  */
#define MOTOR_FLAGS_ALARM \
	(1 << 1) /**< Flags for an alarm signal of the motor  */
/** @} */

#define MOTOR_TIMEOUT \
	(5 * osKernelGetTickFreq()) /**< Motor operation timeout. */

static int32_t wait_evt(uint32_t evt, uint32_t timeout)
{
	uint32_t f = osThreadFlagsWait(evt, osFlagsWaitAny, timeout);
	if (f & osFlagsError) {
		return -ETIMEDOUT;
	}

	return f;
}

int motor_goto_angles(const float angles[MOTOR_NUM])
{
	evt_waiter = osThreadGetId();
	osThreadFlagsClear(MOTOR_FLAGS_DONE | MOTOR_FLAGS_ALARM);

	for (enum motor_t m = 0; m < MOTOR_NUM; m++) {
		prepare_goto(m, angles[m]);
	}

	BSP_L6470_PerformPreparedApplicationCommand(0);
	// BSP_L6470_PerformPreparedApplicationCommand(1);

	int32_t f =
		wait_evt(MOTOR_FLAGS_DONE | MOTOR_FLAGS_ALARM, MOTOR_TIMEOUT);
	if (f < 0) {
		return f;
	}

	if (f & MOTOR_FLAGS_ALARM) {
		//TODO: we should maybe return the source of the alarm in error cases
		return -EIO;
	}

	return 0;
}

/**
 * @brief Weak overload of the external rising interrupt callback.
 *
 * @param pin Pin of the inrerrupt.
 */
void HAL_GPIO_EXTI_Rising_Callback(uint16_t pin)
{
	if (pin == STEPPER_BUSY_N_Pin) {
		osThreadFlagsSet(evt_waiter, MOTOR_FLAGS_DONE);
	}
}

/**
 * @brief Weak overload of the external rising interrupt callback.
 *
 * @param pin Pin of the inrerrupt.
 */
void HAL_GPIO_EXTI_Falling_Callback(uint16_t pin)
{
	if (pin == STEPPER_FLAG_N_Pin) {
		osThreadFlagsSet(evt_waiter, MOTOR_FLAGS_ALARM);
	}
}
