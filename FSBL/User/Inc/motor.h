#pragma once

#include <microstepping_motor.h>
#include <stddef.h>
#include <stdint.h>

enum motor_t {
	MOTOR0 = 0,
	// MOTOR1,
	// MOTOR2,
	MOTOR_NUM
};

/**
 * @brief Initialize the motor driver and the L6470 expansion board.
 *
 * @return 0 on SUCCESS, error code < 0 on failure.
 */
[[nodiscard]] int motor_init(void);

/**
 * @brief Moves the motors to absolute angles simultaneously, then sleep waiting for a
 * response frome the motors.
 *
 * @param angles Target angle in degrees for each motor, indexed by enum motor_t.
 * @return 0 on SUCCESS, -EINVAL on invalid mselect, or another negative error code.
 *
 * @retval -EIO on Alarm signal from the motor.
 */
[[nodiscard]] int motor_goto_angles(const float angles[MOTOR_NUM]);



/**
* @brief Perform the homing with external entry detection.
* @return 0 on success, error codes < 0 on error.
* @retval -EIO on Alarm signal from the motor.
*/
int motor_go_home(void);

/**
 * @brief Helper to debug the sw status of one of the motors.
 *
 * @param m The motor to check.
 * @return Weither this is closed or not.
 */
bool motor_sw_closed(enum motor_t m);
