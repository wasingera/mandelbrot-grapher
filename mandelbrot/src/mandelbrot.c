#include "mandelbrot.h"
#include <pthread.h>

//
// Tests if a point is in the mandelbrot set
//
static mandelbrot_result_t mandelbrot_test_point(double c_r, double c_i, size_t max_iterations);

static void* mandelbrot_worker(void* arg);

void mandelbrot_run(mandelbrot_run_params_t* params)
{
    if (params->threads == 0)
    {
        // Compute entire range on this thread
        params->thread_out_x_start = 0;
        params->thread_out_x_end   = params->out_x_size;

        params->thread_out_y_start = 0;
        params->thread_out_y_end   = params->out_y_size;

        mandelbrot_worker(params);
        return;
    }

    const size_t thread_count = params->threads;

    // Divide the window into vertical strips
    const size_t x_step = params->out_x_size / thread_count;

    pthread_t threads[thread_count];
    mandelbrot_run_params_t thread_params[thread_count];

    for (size_t i = 0; i < thread_count; i++)
    {
        thread_params[i] = *params;

        thread_params[i].thread_out_x_start = i*x_step;
        thread_params[i].thread_out_x_end   = (i == thread_count-1) ? params->out_x_size : (i+1)*x_step;

        thread_params[i].thread_out_y_start = 0;
        thread_params[i].thread_out_y_end   = thread_params[i].out_y_size;
        pthread_create(&threads[i], NULL, mandelbrot_worker, (void*)(&thread_params[i]));
    }

    for (size_t i = 0; i < thread_count; i++)
    {
        pthread_join(threads[i], NULL);
    }
}

static void* mandelbrot_worker(void* arg)
{
    mandelbrot_run_params_t *params = (mandelbrot_run_params_t*)arg;

    const size_t width = params->out_x_size;
    const size_t height = params->out_y_size;

    // The step to take in coordinate space between each
    // entry in the result array
    const double r_step = (double)(params->r_max - params->r_min) / (double)width;
    const double i_step = (double)(params->i_max - params->i_min) / (double)height;

    double curr_r = params->r_min + (params->thread_out_x_start*r_step);
    double curr_i = params->i_max - (params->thread_out_y_start*r_step);

    const size_t x_start = params->thread_out_x_start, x_end = params->thread_out_x_end;
    const size_t y_start = params->thread_out_y_start, y_end = params->thread_out_y_end;
    for (size_t x = x_start; x < x_end; x++)
    {
        for (size_t y = y_start; y < y_end; y++)
        {
            // 0 1 2 3
            // 4 5 6 7
            // 8 9 0 1
            // 2 3 4 5
            //
            // y*width + x stores rows in table contiguously
            params->out[y*width + x] =
                mandelbrot_test_point(curr_r, curr_i, params->max_iterations);

            // Advance to the next row
            curr_i -= i_step;
        }

        // Advance to the next column
        curr_i = params->i_max;
        curr_r += r_step;
    }

    return NULL;
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
