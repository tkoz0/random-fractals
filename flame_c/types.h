/*
Types
*/

#pragma once

#include <stdint.h>

#include "jrand.h"

extern const float _EMACH32;
extern const double _EMACH64;

typedef float num_t;
extern const num_t _EPS;

extern const num_t _PI, _2PI;
extern const num_t _1_PI, _2_PI;
extern const num_t _PI_2, _PI_3, _PI_4, _PI_6, _PI_12;

extern const num_t _E, _1_E, _LOG2;

extern const num_t _SQRT2, _SQRT3, _SQRT5;
extern const num_t _1_SQRT2, _1_SQRT3, _1_SQRT5;
extern const num_t _PHI, _1_PHI;

// check for NaN and very large/small values
// TODO make this faster by checking only for +-inf and NaN
static inline bool bad_value(num_t n)
{
    return (n > 1e10) || (n < -1e10) || (n != n);
}

// parameters for affine transformation
typedef struct
{
    // (x,y) -> (a*x+b*y+c,d*x+e*y+f)
    num_t a, b, c, d, e, f;
}
affine_params;

// affine identity transformation (x,y) -> (x,y)
extern const affine_params null_affine;

// some helper types, currently unused
typedef struct { uint8_t r, g, b; } rgb_t;
typedef struct { uint8_t r, g, b, a; } rgba_t;
typedef struct { num_t x, y; } point_t;

// parameters for variations
// TODO add to this for more variations
typedef struct
{
}
var_params_t;

// iteration state variables
typedef struct
{
    num_t x, y; // current point
    num_t tx, ty; // pre affine transform applied
    num_t vx, vy; // variation sum
    // TODO precalc variables
    // TODO variation parameters
}
iter_state_t;

// variation function type
typedef void (*var_func_t)(iter_state_t*,num_t);

// xform
typedef struct
{
    num_t weight; // probability to select (will be normalized)
    var_func_t *vars; // function pointers to variations
    num_t *varw; // variation wewights
    uint32_t var_len; // number of variations
    //uint32_t *pc_flags; // TODO what to precompute for efficiency
    affine_params pre_affine;
    affine_params post_affine;
    var_params_t params;
}
xform_t;

// flame
typedef struct
{
    char *name;
    size_t size_x, size_y; // dimensions for histogram
    uint64_t samples; // number of iterations
    num_t xmin, xmax, ymin, ymax; // bounds for rectangle to render
    xform_t *xforms;
    size_t xforms_len;
}
flame_t;
