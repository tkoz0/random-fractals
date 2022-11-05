/*
Variations
*/

#pragma once

#include "types.h"

void var0_linear(iter_state_t*,num_t);
void var1_sinusoidal(iter_state_t*,num_t);
void var2_spherical(iter_state_t*,num_t);
void var3_swirl(iter_state_t*,num_t);
void var4_horseshoe(iter_state_t*,num_t);
void var5_polar(iter_state_t*,num_t);
void var6_handkerchief(iter_state_t*,num_t);
void var7_heart(iter_state_t*,num_t);
void var8_disc(iter_state_t*,num_t);
void var9_spiral(iter_state_t*,num_t);
void var10_hyperbolic(iter_state_t*,num_t);
void var11_diamond(iter_state_t*,num_t);
void var12_ex(iter_state_t*,num_t);

typedef struct
{
    const char *name;
    const var_func_t func;
    const uint32_t flags;
}
var_info_t;

extern const var_info_t VARIATIONS[];

// precalc flags
#define PC_THETA (1 << 0)
#define PC_PHI   (1 << 1)
#define PC_SINT  (1 << 2)
#define PC_COST  (1 << 3)
#define PC_R     (1 << 4)
#define PC_R2    (1 << 5)
