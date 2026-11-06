#pragma once

#include <stddef.h>
#include <stdint.h>

/**
 * @name Motor Bitmask Flags
 * @{
 */
typedef enum {
	MOTOR_AXIS_1 = 0,
	MOTOR_AXIS_2 = 1,
	MOTOR_AXIS_3 = 2
} motor_axis_t;

/** @brief Bitmask flag representing Motor 0 (Axis 1). */
#define MOTOR0_BIT (1 << 0)

/** @brief Bitmask flag representing Motor 1 (Axis 2). */
#define MOTOR1_BIT (1 << 1)

/** @brief Bitmask flag representing Motor 2 (Axis 2). */
#define MOTOR2_BIT (1 << 2)

/** @} */

enum motor_t { MOTOR0 = 0, MOTOR1, MOTOR2 };

/**
 * @brief Initialize the motor driver and the L6470 expansion board.
 *
 * @return 0 on SUCCESS, error code < 0 on failure.
 */
[[nodiscard]] int motor_init(void);

/**
 * @brief Convert a target angle in degrees into absolute micro-steps.
 *
 * @param target_angle_deg The target angle in degrees.
 * @return The corresponding number of absolute micro-steps.
 */
uint32_t motor_angle_to_steps(float target_angle_deg);

/**
 * @brief Move the specified motor axis to an absolute angle using the GoTo command.
 * @param board_id The identifier of the L6470 expansion board.
 * @param motor_id The motor axis on the board.
 * @param target_angle_deg The absolute target angle in degrees.
 */
void motor_goto_angle(enum motor_t motor, float target_angle_deg);

/**
 * @brief Move multiple motor axes to absolute angles simultaneously based on a bitmask of flags.
 *
 * @param angles Pointer to an array of target angles in degrees, ordered by active motors.
 * @param size Number of elements in the angles array (must match the number of active bits in flags).
 * @param mselect Bitmask indicating which motors to move (combination of MOTOR0_BIT, MOTOR1_BIT, MOTOR2_BIT).
 * @return 0 on SUCCESS, or a negative error code (e.g., -EINVAL) on failure.
 */
[[nodiscard]] int motors_goto_angles(float *angles, size_t size,
				     uint8_t mselect);
