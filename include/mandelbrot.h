#ifndef MANDELBROT_H
#define MANDELBROT_H

#include <stddef.h>

//
// Defines
//
#define WINDOW_WIDTH    640
#define WINDOW_HEIGHT   640
#define MANDELBROT_ITERATIONS   1000

//
// Types
//
typedef struct Mandelbrot_View {
    double center_x; /**< The central x coordinate of the view */
    double center_y; /**< The central y coordinate of the view */

    double plane[WINDOW_WIDTH][WINDOW_HEIGHT][3];
} Mandelbrot_View;

extern Mandelbrot_View state;

//
// Function definitions
//

/**
 * @brief   Initialize the state of the mandelbrot view
 */
void Mandelbrot_init(void);

/**
 * @brief   Compute the values for a mandelbrot set
 * @param   iterations  The number of iterations to perform
 */
void Mandelbrot_compute(size_t x_min, size_t x_max, size_t y_min,
        size_t y_max, size_t iterations);

#endif

