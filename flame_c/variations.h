/*
Variations
*/

#pragma once

#include "types.h"

#define NUM_VARIATIONS 10

void var0_linear(iter_state_t *state, num_t weight);
void var1_sinusoidal(iter_state_t *state, num_t weight);
void var2_spherical(iter_state_t *state, num_t weight);
void var3_swirl(iter_state_t *state, num_t weight);
void var4_horseshoe(iter_state_t *state, num_t weight);
void var5_polar(iter_state_t *state, num_t weight);
void var6_handkerchief(iter_state_t *state, num_t weight);
void var7_heart(iter_state_t *state, num_t weight);
void var8_disc(iter_state_t *state, num_t weight);
void var9_spiral(iter_state_t *state, num_t weight);

typedef struct
{
    const char *name;
    const var_func_t func;
    const uint32_t flags;
}
var_info_t;

extern const var_info_t VARIATIONS[NUM_VARIATIONS+1];

// precalc flags
#define PC_THETA (1 << 0)
#define PC_PHI   (1 << 1)
#define PC_SINT  (1 << 2)
#define PC_COST  (1 << 3)
#define PC_R     (1 << 4)
#define PC_R2    (1 << 5)
