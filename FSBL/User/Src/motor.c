#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/param.h>

#include "L6470.h"
#include "cmsis_os2.h"
#include "main.h"
#include "microstepping_motor.h"
#include "motor.h"
#include "stm32n6xx_hal_gpio.h"
#include "xnucleoihm02a1.h"

#define MOTORS_PER_BOARD \
	L6470DAISYCHAINSIZE /**< Number of L6470 daisy-chained on one IHM02A1 board. */
static_assert(MOTOR_NUM <= EXPBRD_MOUNTED_NR * MOTORS_PER_BOARD,
	      "not enough L6470 boards declared for MOTOR_NUM");

/**
 * @brief Starter L6470 config for the SANMOTION SF2422-10B, same for every arm.
 *
 * Electrical values are the course example ones, already run on this motor.
 */
static const MotorParameterData_t motor_cfg = {
	/* Motor characteristics */
	.motorvoltage = 12.0f, /* Must match the power supply voltage */
	.fullstepsperrevolution = 200, /* See doc for this 1.8 deg/step. */

	/* Driver config */
	/* Same acc/dec: arms start/stop together. */
	.acc = 800.0f,
	.dec = 800.0f,
	.maxspeed = 400.0f, /* 2 rev/s. */
	.minspeed = 0.0f,

	.fsspd = 1000.0f, /* Never happens because it is > maxspeed */
	.kvalhold = 3.06f,
	.kvalrun = 3.06f,
	.kvalacc = 3.06f,
	.kvaldec = 3.06f,
	.intspeed = 61.52f,
	.stslp = 392.1569e-6f,
	.fnslpacc = 643.1372e-6f,
	.fnslpdec = 643.1372e-6f,
	.kterm = 0,
	.ocdth = 3366.0f,
	.stallth = 3060.0f,
	.step_sel = MICROSTEP_1_16, /* 3200 steps/rev. */
	.alarmen = 0xFF,
	.config = 0x2E88,
};

static osThreadId_t evt_waiter;

int motor_init(void)
{
	MotorParameterData_t *cfg = (MotorParameterData_t *)&motor_cfg;

	BSP_L6470_Init(); /* Leaves board 0 selected. */
	BSP_L6470_Config_0_0(cfg);
	BSP_L6470_Config_0_1(cfg);
#if EXPBRD_MOUNTED_NR > 1 /** If we have stacked another board */
	BSP_L6470_Select(BSP_L6470_EXPBRD_ID(1));
	BSP_L6470_Config_1_0(cfg);
	BSP_L6470_Config_1_1(cfg);
#endif

	/* A reset latches UVLO low until GetStatus is read, which holds the
	 * shared FLAG line low and fakes an alarm on the first goto. */
	BSP_L6470_GetStatus(BSP_L6470_EXPBRD_ID(0), 0);
	BSP_L6470_GetStatus(BSP_L6470_EXPBRD_ID(0), 1);
#if EXPBRD_MOUNTED_NR > 1
	BSP_L6470_GetStatus(BSP_L6470_EXPBRD_ID(1), 0);
	BSP_L6470_GetStatus(BSP_L6470_EXPBRD_ID(1), 1);
#endif

	return 0;
}

static uint32_t deg_to_steps(float target_angle_deg)
{
	uint32_t steps_per_rev = (uint32_t)motor_cfg.fullstepsperrevolution
				 << motor_cfg.step_sel;

	float steps_per_deg = (float)steps_per_rev / 360.0f;
	int32_t absolute_steps = (int32_t)(target_angle_deg * steps_per_deg);

	return (uint32_t)absolute_steps;
}

/**
 * @brief Send a GoTo to the first @p n L6470 of one board.
 */
static void board_goto(uint8_t board, const float *angles, uint8_t n)
{
	for (uint8_t i = 0; i < n; i++) {
		L6470_PrepareGoTo(i, deg_to_steps(angles[i]));
	}

	BSP_L6470_PerformPreparedApplicationCommand(board);
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

	for (uint8_t m = 0; m < MOTOR_NUM; m += MOTORS_PER_BOARD) {
		board_goto(m / MOTORS_PER_BOARD, &angles[m],
			   MIN(MOTOR_NUM - m, MOTORS_PER_BOARD));
	}

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
