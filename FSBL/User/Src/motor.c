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

#define STEP_SEL MICROSTEP_1_4
#define FULLSTEP_PER_REVOLUTION 200

#define MOTOR_NUM 3 /**< Number of motors */
#define MOTOR_TIMEOUT (5 * osKernelGetTickFreq())

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

uint32_t motor_angle_to_steps(float target_angle_deg)
{
	uint32_t steps_per_rev =
		(uint32_t)(FULLSTEP_PER_REVOLUTION * (1 << STEP_SEL));

	float steps_per_deg = (float)steps_per_rev / 360.0f;
	int32_t absolute_steps = (int32_t)(target_angle_deg * steps_per_deg);

	return (uint32_t)absolute_steps;
}

void motor_goto_angle(enum motor_t motor, float target_angle_deg)
{
	int32_t absolute_steps =
		(int32_t)motor_angle_to_steps(target_angle_deg);
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

static int32_t motor_wait_evt(uint32_t evt, uint32_t timeout)
{
	uint32_t f = osThreadFlagsWait(evt, osFlagsWaitAny, timeout);
	if (f & osFlagsError) {
		return -ETIMEDOUT;
	}

	return f;
}

int motors_goto_angles(float angles[], size_t size, uint8_t mselect)
{
	evt_waiter = osThreadGetId();
	osThreadFlagsClear(MOTOR_FLAGS_DONE | MOTOR_FLAGS_ALARM);
	size_t m0 = mselect & MOTOR0_BIT;
	size_t m1 = mselect & MOTOR1_BIT;
	size_t m2 = mselect & MOTOR2_BIT;
	// Checks if the array size is the same as the flags size.
	if (size != (m0 + m1 + m2)) {
		return -EINVAL;
	}

	int masks[] = { MOTOR0_BIT, MOTOR1_BIT, MOTOR2_BIT };
	int x = 0;

	for (int i = 0; i < 3; i++) {
		if (mselect & masks[i]) {
			motor_goto_angle(angles[x++], masks[i]);
		}
	}
	BSP_L6470_PerformPreparedApplicationCommand(0);
	BSP_L6470_PerformPreparedApplicationCommand(1);

	int32_t f = motor_wait_evt(MOTOR_FLAGS_DONE | MOTOR_FLAGS_ALARM,
				   MOTOR_TIMEOUT);
	if (f < 0) {
		return f;
	}

	if (f & MOTOR_FLAGS_ALARM) {
		return -1;
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
