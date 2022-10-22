/*
Flame fractal renderer.

<In progress>
Usage: ./a.out [-r <seed>]
*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "jrand.h"
#include "renderer.h"
#include "types.h"
#include "variations.h"

int main(int argc, char **argv)
{
    flame_t f;
    f.name = "test";
    f.size_x = f.size_y = 1024;
    f.xmin = f.ymin = -4.0;
    f.xmax = f.ymax = 4.0;
    f.xforms_len = 3;
    f.xforms = calloc(3,sizeof(xform_t));
    num_t vweight[2] = {1.0,-0.5};
    var_func_t vfuncs[2] = {&var0_linear,&var2_spherical};
    // 0,0
    f.xforms[0].weight = 1.0;
    f.xforms[0].vars = vfuncs;
    f.xforms[0].varw = vweight;
    f.xforms[0].var_len = 2;
    f.xforms[0].pre_affine = (affine_params){0.5,0.0,0.0,0.0,0.5,0.0};
    f.xforms[0].post_affine = null_affine;
    // 0,1
    f.xforms[1].weight = 1.0;
    f.xforms[1].vars = vfuncs;
    f.xforms[1].varw = vweight;
    f.xforms[1].var_len = 1;
    f.xforms[1].pre_affine = (affine_params){0.5,0.0,0.0,0.0,0.5,0.5};
    f.xforms[1].post_affine = null_affine;
    // 1,0
    f.xforms[2].weight = 1.0;
    f.xforms[2].vars = vfuncs;
    f.xforms[2].varw = vweight;
    f.xforms[2].var_len = 1;
    f.xforms[2].pre_affine = (affine_params){0.5,0.0,0.5,0.0,0.5,0.0};
    f.xforms[2].post_affine = null_affine;
    // random
    jrand_t j;
    jrand_init(&j);
    uint32_t *buf = calloc(f.size_x*f.size_y,sizeof(*buf));
    // render
    fprintf(stderr,"render start\n");
    render_basic(&f,buf,&j,1uL<<24);
    fprintf(stderr,"render done\n");
    uint64_t s = 0;
    uint32_t m = 0;
    for (uint64_t i = 0; i < f.size_x*f.size_y; ++i)
    {
        s += buf[i];
        if (buf[i] > m)
            m = buf[i];
    }
    fprintf(stderr,"samples = %lu\n",s);
    fprintf(stderr,"max value = %u\n",m);
    fprintf(stderr,"converting to image\n");
    num_t *log_scale = calloc(f.size_x*f.size_y,sizeof(*log_scale));
    num_t lm = 0.0;
    for (uint64_t i = 0; i < f.size_x*f.size_y; ++i)
    {
        log_scale[i] = log((double)(buf[i]+1));
        if (log_scale[i] > lm)
            lm = log_scale[i];
    }
    fprintf(stderr,"log scale up to %f\n",lm);
    // output image
    printf("P5\n%u %u\n255\n",f.size_x,f.size_y);
    uint8_t *img = calloc(f.size_x*f.size_y,sizeof(*img));
    uint8_t *img_ptr = img;
    for (uint64_t r = f.size_y; r--;)
        for (uint64_t c = 0; c < f.size_x; ++c)
        {
            num_t ls = log_scale[r*f.size_x+c];
            *(img_ptr++) = (uint8_t)(ls*255.5/lm);
        }
    fwrite(img,sizeof(*img),f.size_x*f.size_y,stdout);
    fflush(stdout);
    fprintf(stderr,"done\n");
    free(f.xforms);
    free(buf);
    free(log_scale);
    free(img);
    return 0;
}
