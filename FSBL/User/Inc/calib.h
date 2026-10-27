#pragma once

/**
 * @brief Gets the x, y real coordinate from the AI pipeline detection.
 *
 * @param u[in] x from the AI @p detection output.
 * @param v[in] y from the AI @p detection output.
 * @param x[out] real world x coordinate of the point.
 * @param y[out] real world y coordinate of the point.
 */
void calib_pixel_to_world(float u, float v, float *x, float *y);
