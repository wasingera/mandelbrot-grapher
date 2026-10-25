#ifndef APP_DEFS_H
#define APP_DEFS_H

#include <stdbool.h>
#include "mandelbrot.h"

#define WINDOW_WIDTH    (1000U)
#define WINDOW_HEIGHT   (1000U)

#define MANDELBROT_THREADS          (0U)
#define MANDELBROT_MAX_ITERATIONS   (1000U)
#define MANDELBROT_ZOOM_FACTOR      (2.0)
#define MANDELBROT_PAN_FACTOR       (10.0)

typedef struct mandelbrot_state_t {
    double c_r;
    double c_i;
    double scale;
    bool update;

    mandelbrot_result_t *results;
} mandelbrot_state_t;

typedef struct app_state_t {
    float pixels[WINDOW_WIDTH][WINDOW_HEIGHT][3];

    mandelbrot_state_t mandelbrot;
} app_state_t;

#endif

