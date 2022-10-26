/*
Variations
*/

#pragma once

#include "types.h"

#define NUM_VARIATIONS 8

void var0_linear(iter_state_t *state, num_t weight);
void var1_sinusoidal(iter_state_t *state, num_t weight);
void var2_spherical(iter_state_t *state, num_t weight);
void var3_swirl(iter_state_t *state, num_t weight);
void var4_horseshoe(iter_state_t *state, num_t weight);
void var5_polar(iter_state_t *state, num_t weight);
void var6_handkerchief(iter_state_t *state, num_t weight);
void var7_heart(iter_state_t *state, num_t weight);

typedef struct
{
    const char *name;
    const var_func_t func;
    const uint32_t flags;
}
var_info_t;

extern const var_info_t VARIATIONS[NUM_VARIATIONS];
