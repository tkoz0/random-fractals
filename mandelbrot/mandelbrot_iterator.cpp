#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <quadmath.h>
#include <chrono>
#include <iostream>

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

// array for PPM image output
uint8_t *img_bytes = nullptr;

// forward declarations
void render_ST();
void output_data();

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

    fprintf(stderr,"image dimension: %u x %u\n",xpix,ypix);
    fprintf(stderr,"center location: %s %s\n",argv[3],argv[4]);
    fprintf(stderr,"complex plane widths: %s %s\n",argv[5],argv[6]);
    fprintf(stderr,"iterations computed: %u\n",iterlim);

    auto time_start = std::chrono::high_resolution_clock::now();

    img_bytes = (uint8_t*) malloc(3 * xpix * ypix * sizeof(*img_bytes));
    render_ST(); // single threaded renderer
    output_data();
    free(img_bytes);

    auto time_end = std::chrono::high_resolution_clock::now();
    uint64_t time_nano = std::chrono::duration_cast<std::chrono::nanoseconds>(time_end-time_start).count();
    fprintf(stderr,"computation time: %lf\n",time_nano/1.0E9);

    return 0;
}

// color palette
uint8_t cR[6] = {0xFF,0xFF,0xFF,0x00,0x00,0xFF};
uint8_t cG[6] = {0x00,0x77,0xFF,0xFF,0x00,0x00};
uint8_t cB[6] = {0x00,0x00,0x00,0x00,0xFF,0xFF};

void render_ST() // single threaded renderer
{
    uint8_t *pix_ptr = img_bytes; // current pixel pointer
    for (uint32_t y = ypix; y--;)
        for (uint32_t x = 0; x < xpix; ++x)
        {
            complex_t point = { xmin + (x * xwidth) / (xpix - 1), ymin + (y * ywidth) / (ypix - 1) };
            complex_t seq = point; // sequence iteration

            uint32_t iter = 0;
            for (; iter < iterlim; ++iter) // iterate sequence for this pixel
            {
                // precomputed values for optimization
                num_t xsq = seq.x * seq.x;
                num_t ysq = seq.y * seq.y;
                if (xsq + ysq > 4.0) break; // diverged when magnitude > 2.0
                // linear (x,y) --> (x,y) + (px,py)
                //seq = { seq.x + point.x, seq.y + point.y };
                // mandelbrot (x,y) --> (x*x-y*y,2*x*y) + (px,py)
                seq = { xsq - ysq + point.x, 2.0 * seq.x * seq.y + point.y };
                // tribrot (x,y) --> (x*(x*x-3*y*y),y*(3*x*x-y*y)) + (px,py)
                //seq = { seq.x * (xsq - 3.0 * ysq) + point.x, seq.y * (3.0 * xsq - ysq) + point.y };
                // quadbrot (x,y) --> ((x*x-y*y)^2-4*x*x*y*y,4*x*y*(x*x-y*y)) + (px,py)
                //num_t sqdif = xsq - ysq;
                //seq = { sqdif * sqdif - 4.0 * xsq * ysq + point.x, 4.0 * seq.x * seq.y * sqdif + point.y };
            }

            uint8_t r,g,b;
            if (iter == iterlim) r = g = b = 0; // stays bounded (in the set)
            else // diverged, color based on iterations
                r = cR[iter%6], g = cG[iter%6], b = cB[iter%6];
            *(pix_ptr++) = r;
            *(pix_ptr++) = g;
            *(pix_ptr++) = b;
        }
}

void output_data() // write PPM binary image (P6)
{
    printf("P6\n");
    printf("%u %u\n",xpix,ypix);
    printf("255\n");
    // write binary data from img_bytes array
    fwrite(img_bytes,sizeof(*img_bytes),3*xpix*ypix,stdout);
}
