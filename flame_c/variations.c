#define _GNU_SOURCE
#include <math.h>

#include "jrand.h"
#include "types.h"
#include "variations.h"

// squared 2-norm
static inline num_t _r2(num_t x, num_t y)
{
    return x*x+y*y;
}

// 2-norm
static inline num_t _r(num_t x, num_t y)
{
    return sqrtf(_r2(x,y));
}

// angles

// atan2(x,y)
static inline num_t _theta(num_t x, num_t y)
{
    return atan2(x,y);
}

// atan2(y,x)
static inline num_t _phi(num_t x, num_t y)
{
    return atan2(y,x);
}

// tables for random variables
const num_t _OMEGA_TABLE[2] = { 0, 3.141592653589793 };
const num_t _LAMBDA_TABLE[2] = { 1.0, -1.0 };

// random variables

// random in [0,1)
static inline num_t _psi(jrand_t *j)
{
    return jrand_next_float(j);
}

// 0 or pi
static inline num_t _omega(jrand_t *j)
{
    return _OMEGA_TABLE[jrand_next_bool(j)];
}

// 1 or -1
static inline num_t _lambda(jrand_t *j)
{
    return _LAMBDA_TABLE[jrand_next_bool(j)];
}

void var0_linear(iter_state_t *state, num_t weight)
{
    state->vx += weight * state->tx;
    state->vy += weight * state->ty;
}

void var1_sinusoidal(iter_state_t *state, num_t weight)
{
    state->vx += weight * sin(state->tx);
    state->vy += weight * sin(state->ty);
}

void var2_spherical(iter_state_t *state, num_t weight)
{
    num_t r = weight / (_r2(state->tx,state->ty) + _EPS);
    state->vx += r * state->tx;
    state->vy += r * state->ty;
}

void var3_swirl(iter_state_t *state, num_t weight)
{
    num_t r2 = _r2(state->tx,state->ty);
    num_t sr,cr;
    sincosf(r2,&sr,&cr);
    state->vx += weight * (sr*state->tx - cr*state->ty);
    state->vy += weight * (cr*state->tx + sr*state->ty);
}

void var4_horseshoe(iter_state_t *state, num_t weight)
{
    num_t r = weight / (_r(state->tx,state->ty) + _EPS);
    state->vx += (state->tx-state->ty)*(state->tx+state->ty) * r;
    state->vy += 2.0*state->tx*state->ty * r;
}

void var5_polar(iter_state_t *state, num_t weight)
{
    num_t a = _theta(state->tx,state->ty);
    num_t r = _r(state->tx,state->ty);
    state->vx += weight * a * _1_PI;
    state->vy += weight * (r - 1.0);
}

const char *VAR_NAME[NUM_VARIATIONS] =
{
    "linear",
    "sinusoidal",
    "spherical",
    "swirl",
    "horseshoe",
    "polar"
};

const var_func_t VAR_FUNCS[NUM_VARIATIONS] =
{
    var0_linear,
    var1_sinusoidal,
    var2_spherical,
    var3_swirl,
    var4_horseshoe,
    var5_polar
};
