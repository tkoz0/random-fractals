#include <assert.h>

#include "types.h"

// machine epsilon
const float _EMACH32 = 1.0F / (float)(1 << 23); // 1.1920928955078125e-07
const double _EMACH64 = 1.0 / (float)(1L << 52); // 2.220446049250313e-16

// pi based constants
const num_t _PI =    3.141592653589793;
const num_t _2PI =   6.283185307179586;
const num_t _1_PI =  0.3183098861837907;
const num_t _2_PI =  0.6366197723675814;
const num_t _PI_2 =  1.5707963267948966;
const num_t _PI_3 =  1.0471975511965976;
const num_t _PI_4 =  0.7853981633974483;
const num_t _PI_6 =  0.5235987755982988;
const num_t _PI_12 = 0.2617993877991494;

// e based constants
const num_t _E =    2.718281828459045;
const num_t _1_E =  0.36787944117144233;
const num_t _LOG2 = 0.6931471805599453;

// sqrt based constants
const num_t _SQRT2 =   1.4142135623730951;
const num_t _SQRT3 =   1.7320508075688772;
const num_t _SQRT5 =   2.23606797749979;
const num_t _1_SQRT2 = 0.7071067811865475;
const num_t _1_SQRT3 = 0.5773502691896258;
const num_t _1_SQRT5 = 0.4472135954999579;
const num_t _PHI =     1.618033988749895;
const num_t _1_PHI =   0.6180339887498948;

// small number for avoiding division by zero
const num_t _EPS = 1e-10;

// identity affine transformation (x,y) -> (x,y)
const affine_params null_affine = { 1.0, 0.0, 0.0, 0.0, 1.0, 0.0 };
