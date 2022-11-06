/*
Variations
*/

#pragma once

#include "types.h"

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
