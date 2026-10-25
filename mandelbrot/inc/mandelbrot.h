#ifndef MANDELBROT_H
#define MANDELBROT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

//
// Types -- Enums
//

//
// Types -- Structs
//

typedef struct mandelbrot_run_params_t {
    // Function parameters
    double r_min; // Lower X bound of view
    double r_max; // Upper X bound of view
    double i_min; // Lower Y bound of view
    double i_max; // Upper Y bound of view

    // Output parameters
    size_t result_width;  // X size of result array
    size_t result_height; // Y size of result array

    // Bounding parameters
    size_t max_iterations; // Maximum number of iterations to compute
                           // on a point before making a in/out of bounds
                           // decision
} mandelbrot_run_params_t;

typedef struct mandelbrot_result_t {
    bool in_set;        // Boolean determining if point is in the Mandelbrot set

    size_t iterations;  // If `in_set == true`, the total number of iterations
                        // needed to determine if point was in set.
                        //
                        // Else, `SIZE_MAX`.
} mandelbrot_result_t;

//
// Functions
//

//
// Translates a point from one scale to another
//
static inline double mandelbrot_translate_point(double p,
    double in_min, double in_max, double out_min, double out_max)
{
    return (p - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

//
// Tests if a point is in the mandelbrot set
static mandelbrot_result_t mandelbrot_test_point(double c_r, double c_i, size_t max_iterations)
{
    mandelbrot_result_t result = {.iterations = max_iterations};

    double zr = 0;
    double zi = 0;

    size_t i;
    for (i = 0; i < max_iterations; i++)
    {
        // z_(n+1) = z_n * z_n + c
        // z_(n+1) = (z_r*z_r - z_i*z_i) + (2*z_r*z_i)I + c_r + c_i*I

        // z_(n+1)_r = (z_r*z_r - z_i*z_i) + c_r
        // z_(n+1)_i = (2*z_r*z_i) + c_i

        double zr_next = (zr*zr - zi*zi) + c_r;
        zi = 2*zr*zi + c_i;
        zr = zr_next;

        double z2 = zr*zr + zi*zi;

        // If z diverges, the point is not in the set
        if (z2 >= 4.0)
        {
            break;
        }
    }

    result.in_set = (i >= max_iterations) ? true : false;
    result.iterations = i;

    return result;
}

//
// Run the Mandelbrot solver
//
// Parameters:
//      `params`: Configuration parameters for computation
//
// Returns:
//      `mandelbrot_result_t[params->width * params->height]`
//
//          Returned in row-major order (i.e. address with y*width + x)
static mandelbrot_result_t* mandelbrot_run(mandelbrot_run_params_t* params)
{

    const size_t width = params->result_width;
    const size_t height = params->result_height;
    const size_t total_results = width*height;

    mandelbrot_result_t *result = malloc(sizeof(mandelbrot_result_t)*total_results);

    // The step to take in coordinate space between each
    // entry in the result array
    const double step_r = (double)(params->r_max - params->r_min) / (double)params->result_width;
    const double step_i = (double)(params->i_max - params->i_min) / (double)params->result_height;

    double curr_r = params->r_min;
    double curr_i = params->i_max;

    for (size_t x = 0; x < params->result_width; x++)
    {
        for (size_t y = 0; y < params->result_height; y++)
        {
            // 0 1 2 3
            // 4 5 6 7
            // 8 9 0 1
            // 2 3 4 5
            //
            // y*width + x stores rows in table contiguously
            result[y*width + x] = mandelbrot_test_point(curr_r, curr_i, params->max_iterations);

            // Advance to the next row
            curr_i -= step_i;
        }

        // Advance to the next column
        curr_i = params->i_max;
        curr_r += step_r;
    }

    return result;
}

static inline void mandelbrot_free_results(mandelbrot_result_t* results)
{
    free(results);
}

#endif

