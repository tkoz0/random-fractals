#include <assert.h>
#include <math.h>
#include <stdlib.h>

#include "jrand.h"
#include "types.h"

#define SETTLE_ITERS 20

static inline void _biunit_rand(num_t s, jrand_t *j, num_t *x, num_t *y)
{
    *x = s*(jrand_next_float(j)*2.0 - 1.0);
    *y = s*(jrand_next_float(j)*2.0 - 1.0);
}

static inline void _apply_affine(affine_params *af, num_t *xn, num_t *yn, num_t x, num_t y)
{
    *xn = af->a*x + af->b*y + af->c;
    *yn = af->d*x + af->e*y + af->f;
}

static inline void _apply_xform_basic(iter_state_t *state, xform_t *xf)
{
    // transform point
    _apply_affine(&(xf->pre_affine),&(state->tx),&(state->ty),state->x,state->y);
    var_func_t *var = xf->vars;
    num_t *weight = xf->varw;
    state->vx = 0.0;
    state->vy = 0.0;
    while (var) // sum variations
    {
        (*var)(state,*weight);
        ++var;
        ++weight;
    }
    // update point
    _apply_affine(&(xf->post_affine),&(state->x),&(state->y),state->vx,state->vy);
}

void _normalize_xform_weights(xform_t *xforms, uint32_t len)
{
    assert(len > 0);
    num_t wsum = 0.0;
    xform_t *xf, *xf2;
    for (xf = xforms; len--; ++xf)
        wsum += xf->weight;
    for (xf2 = xforms; xf2 != xf; ++xf2)
        xf2->weight /= wsum;
}

// randomly select flame based on cumulative weights
static inline uint32_t _pick_xform(num_t *cw, jrand_t *jrand)
{
    uint32_t ret = 0;
    num_t rand = jrand_next_float(jrand);
    while (cw[ret] < rand)
        ++ret;
    return ret;
}

// histogram length == flame->size_x * flame->size_y
// histogram indexed by (flame->size_x * y_pos) + x_pos
void render_basic(flame_t *flame, uint32_t *histogram, jrand_t *jrand, uint64_t samples)
{
    num_t xmul = (float) flame->size_x / (flame->xmax - flame->xmin);
    num_t ymul = (float) flame->size_y / (flame->ymax - flame->ymin);
    _normalize_xform_weights(flame->xforms,flame->xforms_len);
    // form cumulative weights array for random xform selection
    num_t *cw = malloc(sizeof(*cw)*flame->xforms_len);
    num_t s = 0.0;
    for (uint32_t i = 0; i < flame->xforms_len; ++i)
    {
        s += flame->xforms[i].weight;
        cw[i] = s;
    }
    cw[flame->xforms_len-1] = 1.0; // to correct for rounding error
    iter_state_t state;
    _biunit_rand(1.0,jrand,&(state.x),&(state.y));
    for (uint32_t i = 0; i < SETTLE_ITERS; ++i)
        _apply_xform_basic(&state,flame->xforms+_pick_xform(cw,jrand));
    while (samples--)
    {
        _apply_xform_basic(&state,flame->xforms+_pick_xform(cw,jrand));
        if (state.x < flame->xmin || state.x >= flame->xmax
            || state.y < flame->ymin || state.y >= flame->ymax)
            continue;
        uint32_t x = (state.x - flame->xmin) * xmul;
        uint32_t y = (state.y - flame->ymin) * ymul;
        ++histogram[(flame->size_x*y)+x];
    }
}
