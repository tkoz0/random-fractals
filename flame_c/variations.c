#define _GNU_SOURCE
#include <math.h>

#include "jrand.h"
#include "types.h"
#include "variations.h"

// squared 2-norm (precalc_sumsq)
static inline num_t _r2(num_t x, num_t y)
{
    return x*x + y*y;
}

// 2-norm (precalc_sqrt)
static inline num_t _r(num_t x, num_t y)
{
    return sqrtf(_r2(x,y));
}

// angles

// atan2(x,y) (precalc_atan)
static inline num_t _theta(num_t x, num_t y)
{
    return atan2(x,y);
}

// atan2(y,x) (precalc_atanyx)
static inline num_t _phi(num_t x, num_t y)
{
    return atan2(y,x);
}

// tables for random variables
static const num_t _OMEGA_TABLE[2] = { 0, 3.141592653589793 };
static const num_t _LAMBDA_TABLE[2] = { 1.0, -1.0 };

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

// helper macros (prefix _C_ for compute)
#define _X (S->tx)
#define _Y (S->ty)
#define _C_R2 _r2(S->tx,S->ty)
#define _C_R _r(S->tx,S->ty)
#define _C_ATAN _theta(S->tx,S->ty)
#define _C_ATANYX _phi(S->tx,S->ty)
// define this since the sincos depends on the type of num_t
// regular sin/cos can use the type generic macros
#define _SINCOS(x,y,z) sincosf(x,y,z)

void var0_linear(iter_state_t *S, num_t W)
{
    S->vx += W * _X;
    S->vy += W * _Y;
}

void var1_sinusoidal(iter_state_t *S, num_t W)
{
    S->vx += W * sin(_X);
    S->vy += W * sin(_Y);
}

void var2_spherical(iter_state_t *S, num_t W)
{
    num_t r = W / (_C_R2 + _EPS);
    S->vx += r * _X;
    S->vy += r * _Y;
}

void var3_swirl(iter_state_t *S, num_t W)
{
    num_t sr,cr;
    _SINCOS(_C_R2,&sr,&cr);
    S->vx += W * (sr*_X - cr*_Y);
    S->vy += W * (cr*_X + sr*_Y);
}

void var4_horseshoe(iter_state_t *S, num_t W)
{
    num_t r = W / (_C_R + _EPS);
    S->vx += (_X-_Y) * (_X+_Y) * r;
    S->vy += 2.0*_X*_Y * r;
}

void var5_polar(iter_state_t *S, num_t W)
{
    S->vx += W * _C_ATAN * _1_PI;
    S->vy += W * (_C_R - 1.0);
}

void var6_handkerchief(iter_state_t *S, num_t W)
{
    num_t a = _C_ATAN;
    num_t r = _C_R;
    num_t rw = W * r;
    S->vx += rw * sin(a+r);
    S->vy += rw * cos(a-r);
}

void var7_heart(iter_state_t *S, num_t W)
{
    num_t r = _C_R;
    num_t sin_a,cos_a;
    _SINCOS(r*_C_ATAN,&sin_a,&cos_a);
    r *= W;
    S->vx += r * sin_a;
    S->vy += -r * cos_a;
}

void var8_disc(iter_state_t *S, num_t W)
{
    num_t a = _C_ATAN * _1_PI * W;
    num_t sr,cr;
    _SINCOS(_PI*_C_R,&sr,&cr);
    S->vx += sr * a;
    S->vy += cr * a;
}

void var9_spiral(iter_state_t *S, num_t W)
{
    num_t a = _C_ATAN;
    num_t r = _C_R + _EPS;
    num_t sr,cr;
    _SINCOS(r,&sr,&cr);
    num_t r1 = W/r;
    S->vx += r1 * (cos(a) + sr);
    S->vy += r1 * (sin(a) - cr);
}

void var10_hyperbolic(iter_state_t *S, num_t W)
{
    num_t r = _C_R + _EPS;
    num_t a = _C_ATAN;
    S->vx += W * sin(a) / r;
    S->vy += W * cos(a) * r;
}

void var11_diamond(iter_state_t *S, num_t W)
{
    num_t a = _C_ATAN;
    num_t sr,cr;
    sincosf(_C_R,&sr,&cr);
    S->vx += W * sin(a) * cr;
    S->vy += W * cos(a) * sr;
}

void var12_ex(iter_state_t *S, num_t W)
{
    num_t a = _C_ATAN;
    num_t r = _C_R;
    num_t n0 = sin(a+r);
    num_t n1 = cos(a-r);
    num_t m0 = n0*n0*n0 * r;
    num_t m1 = n1*n1*n1 * r;
    S->vx += W * (m0 + m1);
    S->vy += W * (m0 - m1);
}

const var_info_t VARIATIONS[] =
{
    {"linear", &var0_linear, 0},
    {"sinusoidal", &var1_sinusoidal, 0},
    {"spherical", &var2_spherical, 0},
    {"swirl", &var3_swirl, 0},
    {"horseshoe", &var4_horseshoe, 0},
    {"polar", &var5_polar, 0},
    {"handkerchief", &var6_handkerchief, 0},
    {"heart", &var7_heart, 0},
    {"disc", &var8_disc, 0},
    {"spiral", &var9_spiral, 0},
    {"hyperbolic", &var10_hyperbolic, 0},
    {"diamond", &var11_diamond, 0},
    {"ex", &var12_ex, 0},
    {NULL,NULL,0}
};
