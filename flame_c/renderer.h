/*
Renderer
*/

#pragma once

#include "types.h"

void optimize_flame(flame_t *flame);

void render_basic(flame_t *flame, uint32_t *histogram, jrand_t *jrand);

