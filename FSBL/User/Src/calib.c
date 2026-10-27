#include "calib.h"

/**
 * @brief Calibration matrix to fine tune with the python script.
 */
static const float H[9] = {};

void calib_pixel_to_world(float u, float v, float *x, float *y)
{
	/** Perform the homographie with the folowing relation:
    * H(u, v, 1)^T = lambda * (x, y, 1)^T
    * */
	float lambda = H[6] * u + H[7] * v + H[8];
	*x = (H[0] * u + H[1] * v) / lambda;
	*y = (H[1] * u + H[2] * v) / lambda;
}
