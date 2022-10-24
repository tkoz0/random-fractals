/*
Renderer
*/

#pragma once

#include "types.h"

// makes some changes for better performance
void optimize_flame(flame_t *flame);

// renders histogram frequency data only
void render_basic(flame_t *flame, uint32_t *histogram, jrand_t *jrand);

