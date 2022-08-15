#pragma once

/**
 * Computes the absolute value of `x`.
 *
 * It should be ensured that the minimum value of a signed integer type is not
 * used as an argument because this will overflow (ex: INT_MIN, LONG_MIN, etc).
 */
#define ABS(x) (x < 0 ? -x : x)
