#include "motor.h"
#include <L6470.h>
#include <microstepping_motor.h>
#include <stddef.h>
#include <stdint.h>

#include "xnucleoihm02a1.h"
#include <errno.h>

#define STEP_SEL MICROSTEP_1_4
#define FULLSTEP_PER_REVOLUTION 200

int motor_init(void)
{
	BSP_L6470_Init();
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

int motors_goto_angles(float *angles, size_t size, uint8_t flags)
{
	size_t m0 = flags & MOTOR0_BIT;
	size_t m1 = flags & MOTOR1_BIT;
	size_t m2 = flags & MOTOR2_BIT;
	// Checks if the array size is the same as the flags size.
	if (size != (m0 + m1 + m2)) {
		return -EINVAL;
	}

	int masks[] = { MOTOR0_BIT, MOTOR1_BIT, MOTOR2_BIT };
	int x = 0;

	for (int i = 0; i < 3; i++) {
		if (flags & masks[i]) {
			motor_goto_angle(angles[x++], masks[i]);
		}
	}
	BSP_L6470_PerformPreparedApplicationCommand(0);
	BSP_L6470_PerformPreparedApplicationCommand(1);
	return 0;
}
