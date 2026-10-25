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

static inline void mandelbrot_free_results(mandelbrot_result_t* results)
{
    free(results);
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
void mandelbrot_run(mandelbrot_run_params_t* params, mandelbrot_result_t* out);


#endif

