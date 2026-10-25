#include "mandelbrot.h"

//
// Tests if a point is in the mandelbrot set
//
static mandelbrot_result_t mandelbrot_test_point(double c_r, double c_i, size_t max_iterations);

void mandelbrot_run(mandelbrot_run_params_t* params, mandelbrot_result_t* out)
{

    const size_t width = params->result_width;
    const size_t height = params->result_height;

    // The step to take in coordinate space between each
    // entry in the result array
    const double step_r = (double)(params->r_max - params->r_min) / (double)width;
    const double step_i = (double)(params->i_max - params->i_min) / (double)height;

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
            out[y*width + x] = mandelbrot_test_point(curr_r, curr_i, params->max_iterations);

            // Advance to the next row
            curr_i -= step_i;
        }

        // Advance to the next column
        curr_i = params->i_max;
        curr_r += step_r;
    }
}

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
