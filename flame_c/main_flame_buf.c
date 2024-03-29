/*
Flame fractal renderer.

<In progress>
Usage: ./a.out [-r <seed>]
*/

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "jrand.h"
#include "parser.h"
#include "renderer.h"
#include "types.h"
#include "utils.h"
#include "variations.h"

static inline num_t _scale_linear(uint32_t n)
{
    return (num_t)n;
}

static inline num_t _scale_log(uint32_t n)
{
    return log((num_t)(n+1));
}

static inline num_t _scale_loglog(uint32_t n)
{
    return log(_scale_log(n)+1.0);
}

static inline num_t _scale_logpow(uint32_t n, num_t p)
{
    return pow(_scale_log(n),p);
}

static inline num_t _scale_pow(uint32_t n, num_t p)
{
    return pow(_scale_linear(n),p);
}

static inline num_t _scale_arctan(uint32_t n, num_t d)
{
    return atan((num_t)(n)/d);
}

static inline num_t _scaleinv_recippow(uint32_t n, num_t p)
{
    return 1.0/pow((num_t)(n+1),p);
}

static inline num_t _scaleinv_reciplog(uint32_t n)
{
    return 1.0/_scale_log(n);
}

#define SCALE(n) _scale_log(n)

// given a flame, write the histogram (buf) and grayscale image (img)
void render_flame(flame_t *flame, uint32_t *buf, uint8_t *img)
{
    fprintf(stderr,"rendering flame: %s\n",flame->name);
    jrand_t j;
    jrand_init(&j);
    fprintf(stderr,"  starting...\n");
    memset(buf,0,flame->size_x*flame->size_y*sizeof(*buf));
    clock_t r_start = clock();
    render_basic(flame,buf,&j);
    clock_t r_end = clock();
    float r_secs = (r_end-r_start)/(float)CLOCKS_PER_SEC;
    fprintf(stderr,"  done (%f sec)\n",r_secs);
    fprintf(stderr,"  %f samples/sec\n",flame->samples/r_secs);
    uint64_t sample_count = 0;
    uint32_t max_sample = 0;
    for (uint64_t i = 0; i < flame->size_x*flame->size_y; ++i)
    {
        sample_count += buf[i];
        if (buf[i] > max_sample)
            max_sample = buf[i];
    }
    float percent = ((float) sample_count / (float) flame->samples) * 100.0;
    fprintf(stderr,"  samples in rectangle: %lu (%f%%)\n",sample_count,percent);
    fprintf(stderr,"  max sample value = %u\n",max_sample);
    num_t log_max = 0.0;
    for (uint64_t i = 0; i < flame->size_x*flame->size_y; ++i)
    {
        num_t log_val = SCALE(buf[i]);
        if (log_val > log_max)
            log_max = log_val;
    }
    fprintf(stderr,"  log max for scaling = %f\n",log_max);
    uint8_t *img_ptr = img;
    for (size_t r = flame->size_y; r--;)
        for (size_t c = 0; c < flame->size_x; ++c)
        {
            num_t log_scale = SCALE(buf[r*flame->size_x+c]);
            *(img_ptr++) = (uint8_t)(log_scale*255.5/log_max);
        }
    fprintf(stderr,"  wrote image buffer\n");
}

int main(int argc, char **argv)
{
    assert(argc > 1);
    char *filedata = read_text_file(argv[1]);
    assert(filedata);
    json_value jsondata = json_load(filedata);
    assert(jsondata);
    free(filedata);
    flame_list flames = flames_from_json(jsondata);
    json_destroy(jsondata);
    assert(flames);
    // buffer
    size_t size_x_max = 0;
    size_t size_y_max = 0;
    flame_list flame_ptr = flames;
    while (flame_ptr) // find max size for buffer
    {
        if (flame_ptr->value.size_x > size_x_max)
            size_x_max = flame_ptr->value.size_x;
        if (flame_ptr->value.size_y > size_y_max)
            size_y_max = flame_ptr->value.size_y;
        flame_ptr = flame_ptr->next;
    }
    uint32_t *buf = malloc(size_x_max*size_y_max*sizeof(*buf));
    uint8_t *img = malloc(size_x_max*size_y_max*sizeof(*img));
    // render flames
    flame_ptr = flames;
    while (flame_ptr)
    {
        flame_t *flame = &flame_ptr->value;
        render_flame(flame,buf,img);
        size_t name_len = strlen(flame->name);
        char *fname = malloc(name_len+5);
        memcpy(fname,flame->name,name_len);
        memcpy(fname+name_len,".pgm\0",5);
        FILE *out_file = fopen(fname,"wb");
        assert(out_file);
        fprintf(out_file,"P5\n%lu %lu\n255\n",flame->size_x,flame->size_y);
        fwrite(img,sizeof(*img),flame->size_x*flame->size_y,out_file);
        fclose(out_file);
        fprintf(stderr,"wrote %s\n",fname);
        memcpy(fname+name_len,".buf\0",5);
        out_file = fopen(fname,"wb");
        assert(out_file);
        fwrite(buf,sizeof(*buf),flame->size_x*flame->size_y,out_file);
        fclose(out_file);
        fprintf(stderr,"wrote %s\n",fname);
        free(fname);
        flame_ptr = flame_ptr->next;
    }
    free(buf);
    free(img);
    destroy_flame_list(flames);
    return 0;
}
