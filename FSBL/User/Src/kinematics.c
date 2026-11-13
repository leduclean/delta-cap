#include "kinematics.h"
#include "motor.h"

#define KINEMATIC_DELTA_Z0 10.0 /**< Z0 of the delta robot. */


int kinematics_rev(float x, float y, float angles[MOTOR_NUM])
{
    (void)x;
    (void)y;
    (void)angles;
    return 0;
}