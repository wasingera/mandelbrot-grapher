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

typedef struct mandelbrot_result_t {
    bool in_set;        // Boolean determining if point is in the Mandelbrot set

    size_t iterations;  // If `in_set == true`, the total number of iterations
                        // needed to determine if point was in set.
                        //
                        // Else, `SIZE_MAX`.
} mandelbrot_result_t;

typedef struct mandelbrot_run_params_t {
    // Function parameters
    double r_min; // Lower X bound of view
    double r_max; // Upper X bound of view
    double i_min; // Lower Y bound of view
    double i_max; // Upper Y bound of view


    // Bounding parameters
    size_t max_iterations; // Maximum number of iterations to compute
                           // on a point before making a in/out of bounds
                           // decision

    size_t threads; // Number of threads to use for computation.
                    // 0 to run all computation on main thread

    // In/out parameters
    size_t out_x_size;  // X size of result array
    size_t out_y_size; // Y size of result array
    mandelbrot_result_t* out; // Buffer to store results in

    // Internal parameters -- modified by lib
    size_t thread_out_x_start;
    size_t thread_out_x_end;
    size_t thread_out_y_start;
    size_t thread_out_y_end;
} mandelbrot_run_params_t;


//
// Functions
//

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
//
void mandelbrot_run(mandelbrot_run_params_t* params);


#endif

