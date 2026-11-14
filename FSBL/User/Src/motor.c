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
	.acc = 100.0f,
	.dec = 100.0f,
	.maxspeed = 400.0f, /* 2 rev/s. */
	.minspeed = 10.0f,

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
	.config = 0b10111010011000, /* See sL6470_ConfigRegister_t. */
};

static osThreadId_t evt_waiter;

int motor_init(void)
{
	MotorParameterData_t *cfg = (MotorParameterData_t *)&motor_cfg;
	BSP_L6470_Init(); /* Leaves board 0 selected. */

	BSP_L6470_Config_0_0(cfg);
	BSP_L6470_Config_0_1(cfg);

	BSP_L6470_Select(BSP_L6470_EXPBRD_ID(1));
	BSP_L6470_Config_1_0(cfg);
	BSP_L6470_Config_1_1(cfg);

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

typedef void (*prepare_func)(uint8_t motor, const void *ctx);

static int run_prepare(prepare_func prep, const void *ctx, uint32_t timeout)
{
	evt_waiter = osThreadGetId();
	osThreadFlagsClear(MOTOR_FLAGS_DONE | MOTOR_FLAGS_ALARM);

	for (uint8_t m = 0; m < MOTOR_NUM; m += MOTORS_PER_BOARD) {
		uint8_t n = MIN(MOTOR_NUM - m, MOTORS_PER_BOARD);
		for (uint8_t i = 0; i < n; i++) {
			prep(i, ctx);
		}
		BSP_L6470_PerformPreparedApplicationCommand(m /
							    MOTORS_PER_BOARD);
	}

	int32_t f = wait_evt(MOTOR_FLAGS_DONE | MOTOR_FLAGS_ALARM, timeout);
	if (f < 0) {
		if (f == -ETIMEDOUT) {
			for (uint8_t m = 0; m < MOTOR_NUM; m++) {
				BSP_L6470_HardStop(m / MOTORS_PER_BOARD, m);
			}
		}
		return f;
	}

	if (f & MOTOR_FLAGS_ALARM) {
		//TODO: we should maybe return the source of the alarm in error cases
		return -EIO;
	}

	return 0;
}

static void prep_goto(uint8_t m, const void *ctx)
{
	const float *angles = ctx;
	L6470_PrepareGoTo(m, deg_to_steps(angles[m]));
}

int motor_goto_angles(const float angles[MOTOR_NUM])
{
	return run_prepare(prep_goto, angles, MOTOR_TIMEOUT);
}

#define MOTOR_GO_UNTIL_STEP_S \
	100.0f /**< Speed used for the first homing phase. */

static void prep_go_until(uint8_t m, [[maybe_unused]] const void *ctx)
{
	//TODO: change the hardcoded min speed.
	L6470_PrepareGoUntil(m, L6470_ACT_RST_ID, L6470_DIR_FWD_ID,
			     Step_s_2_Speed(MOTOR_GO_UNTIL_STEP_S));
}

[[maybe_unused]] static void prep_release_sw(uint8_t m,
					     [[maybe_unused]] const void *ctx)
{
	L6470_PrepareReleaseSW(m, L6470_ACT_RST_ID, L6470_DIR_REV_ID);
}

int motor_homing(void)
{
	int res;

	res = run_prepare(prep_go_until, NULL, MOTOR_TIMEOUT);
	if (res < 0) {
		return res;
	}

	//TODO: Need to set up this timer.
	osDelay(5 * osKernelGetTickFreq());

	res = run_prepare(prep_release_sw, NULL, MOTOR_TIMEOUT);
	if (res < 0) {
		return res;
	}

	return 0;
}

bool motor_sw_closed(enum motor_t m)
{
	return BSP_L6470_CheckStatusRegisterFlag(m / MOTORS_PER_BOARD,
						 m % MOTORS_PER_BOARD, SW_F_ID);
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
