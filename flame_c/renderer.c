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
