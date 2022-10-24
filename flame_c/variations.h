/*
Variations
*/

#pragma once

#include "types.h"

#define NUM_VARIATIONS 6

void var0_linear(iter_state_t *state, num_t weight);
void var1_sinusoidal(iter_state_t *state, num_t weight);
void var2_spherical(iter_state_t *state, num_t weight);
void var3_swirl(iter_state_t *state, num_t weight);
void var4_horseshoe(iter_state_t *state, num_t weight);
void var5_polar(iter_state_t *state, num_t weight);

// list of variation names in order
extern const char *VAR_NAME[NUM_VARIATIONS];

// list of variation function pointers in order
extern const var_func_t VAR_FUNCS[NUM_VARIATIONS];
