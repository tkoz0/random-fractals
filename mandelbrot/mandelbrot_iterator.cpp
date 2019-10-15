#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <quadmath.h>

// require defining FP64 or FP128 for floating point precision

#ifdef FP64
typedef double num_t;
#else
#ifdef FP128
typedef __float128 num_t;
#endif
#endif

// complex number type
typedef struct { num_t x, y; } complex_t;

// rendering boundaries
num_t xmin, xmax, ymin, ymax;

// center points
num_t xcenter, ycenter;

// rendering region dimensions (widths)
num_t xwidth, ywidth;

// image dimensions
uint32_t xpix, ypix;

// number of iterations to render
uint32_t iterlim;

// file to output to
char *out_file;

// store the iterated values
// if x is infinity, y stores the iteration count
// otherwise, x and y store the complex number reached through iteration
// the sequence is determined to diverge when it exceeds magnitude 2
// data is ordered first by y decreasing, then by x increasing
// the order of each is dependeng on the signs of the provided xwidth and ywidth
complex_t *values = nullptr;
complex_t *points = nullptr;

// array for PPM image output
uint8_t *img_bytes = nullptr;

void init_arr() // array initialization
{
    if (!values) values = (complex_t*) malloc(xpix * ypix * sizeof(*values));
    if (!points) points = (complex_t*) malloc(xpix * ypix * sizeof(*points));
    assert(values && points);
    complex_t *ptr1 = values;
    complex_t *ptr2 = points;
    for (uint32_t y = ypix; y--;)
        for (uint32_t x = 0; x < xpix; ++x)
            *(ptr1++) = *(ptr2++) = { xmin + (x * xwidth) / (xpix - 1), ymin + (y * ywidth) / (ypix - 1) };
    if (!img_bytes) img_bytes = (uint8_t*) malloc(3 * xpix * ypix * sizeof(*img_bytes));
}

void render_ST() // single threaded renderer
{
    for (uint32_t iter = 0; iter < iterlim; ++iter)
    {
        complex_t *vptr = values; // value pointer
        complex_t *pptr = points; // point coordinates pointer
        complex_t *vptr_end = vptr + (xpix * ypix);
        for (; vptr < vptr_end; ++vptr, ++pptr)
        {
            uint64_t *vptr64 = (uint64_t*) vptr; // for storing integers
#ifdef FP64
            if (vptr64[1] == (uint64_t) -1) continue; // zeroed out y bytes = diverged
#else
#ifdef FP128
            if (vptr64[2] == (uint64_t) -1) continue; // first half of y bytes zeroed out
#endif
#endif
            num_t xsquare = vptr->x * vptr->x; // compute, used to iterate
            num_t ysquare = vptr->y * vptr->y;
            if (xsquare + ysquare > 4.0) // diverged (magnitude > 2)
            {
                *vptr64 = (uint64_t) iter; // iterations in lowest bytes
                *(++vptr64) = -1; // put ones in higher bytes
#ifdef FP128
                *(++vptr64) = -1;
                *(++vptr64) = -1;
#endif
            }
            else // iterate
                // (x,y) --> (x*x-y*y,2*x*y) + (px,py)
                *vptr = { xsquare - ysquare + pptr->x, 2.0 * vptr->x * vptr->y + pptr->y };
        }
    }
}

// color palette
uint8_t cR[6] = {0xFF,0xFF,0xFF,0x00,0x00,0xFF};
uint8_t cG[6] = {0x00,0x77,0xFF,0xFF,0x00,0x00};
uint8_t cB[6] = {0x00,0x00,0x00,0x00,0xFF,0xFF};

void output_data() // write PPM binary image (P6)
{
    printf("P6\n");
    printf("%u %u\n",xpix,ypix);
    printf("255\n");
    // convert sequence data into an image
    uint8_t *iptr = img_bytes;
    complex_t *vptr = values, *vptr_end = values + (xpix * ypix);
    for (; vptr < vptr_end; ++vptr)
    {
        uint64_t *vptr64 = (uint64_t*) vptr; // for storing integers
        uint8_t r, g, b;
        uint64_t iters = *vptr64;
#ifdef FP64
        if (vptr64[1] == (uint64_t) -1) // ones in y bytes = diverged
#else
#ifdef FP128
        if (vptr64[2] == (uint64_t) -1) // first half of y bytes are all ones
#endif
#endif
        {
            r = cR[iters%6];
            g = cG[iters%6];
            b = cB[iters%6];
        }
        else r = g = b = 0; // black for points in the set
        *(iptr++) = r;
        *(iptr++) = g;
        *(iptr++) = b;
    }
    // write ppm pixel binary data
    fwrite(img_bytes,sizeof(*img_bytes),3*xpix*ypix,stdout);
}

void cleanup() // not actually needed because the OS frees process memory anyway
{
    assert(values);
    free(values);
    assert(points);
    free(points);
    assert(img_bytes);
    free(img_bytes);
}

int main(int argc, char **argv)
{
    // expect arguments in this order for now
    // ./a.out <file> <xpix> <ypix> <x1> <y1> <xwidth> <ywidth> <iter>
    if (argc != 8)
    {
        printf("Usage: ./a.out <xpix> <ypix> <xcenter> <ycenter> <xwidth> <ywidth> <iter>\n");
        return 1;
    }

    // parse command line arguments
    xpix = atoi(argv[1]);
    ypix = atoi(argv[2]);
#ifdef FP64
    xcenter = atof(argv[3]);
    ycenter = atof(argv[4]);
    xwidth = atof(argv[5]);
    ywidth = atof(argv[6]);
#else
#ifdef FP128
    xcenter = strtoflt128(argv[3],nullptr);
    ycenter = strtoflt128(argv[4],nullptr);
    xwidth = strtoflt128(argv[5],nullptr);
    ywidth = strtoflt128(argv[6],nullptr);
#endif
#endif
    iterlim = atoi(argv[7]);

    // check command line arguments
    assert(xpix > 1 && xpix <= 10000); // for now, cap image size
    assert(ypix > 1 && ypix <= 10000);
    assert(iterlim > 0 && iterlim <= 1000000); // cap iterations for now
    // dont check xwidth and ywidth, allow negatives to reflect image

    // finish variable assignment
    xmin = xcenter - xwidth/2.0;
    ymin = ycenter - ywidth/2.0;
    xmax = xmin + xwidth;
    ymax = ymin + ywidth;

    init_arr(); // initialize array
    render_ST(); // single threaded render
    output_data(); // write file
    cleanup(); // deallocate memory

    return 0;
}
