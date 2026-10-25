#include "mandelbrot.h"
#include "mandelbrot_kernel.h"
#include <cuda_runtime.h>

#define DIV_UP(a, b) (((a) + (b) - 1) / (b))

__global__ void mandelbrot_worker_kernel(mandelbrot_run_params_t params);
__device__ static mandelbrot_result_t mandelbrot_device_test_point(double c_r, double c_i, size_t max_iterations);

void mandelbrot_run_kernel(mandelbrot_run_params_t* params)
{
    dim3 block(16,16);
    dim3 grid(DIV_UP(params->out_x_size, block.x), DIV_UP(params->out_y_size, block.y));

    mandelbrot_worker_kernel<<<grid, block>>>(*params);

    cudaDeviceSynchronize();
}

__global__ void mandelbrot_worker_kernel(mandelbrot_run_params_t params)
{

    const size_t x = blockIdx.x * blockDim.x + threadIdx.x;
    const size_t y = blockIdx.y * blockDim.y + threadIdx.y;

    if ((x >= params.out_x_size) || (y >= params.out_y_size))
    {
        return;
    }

    const size_t width = params.out_x_size;
    const size_t height = params.out_y_size;

    // The step to take in coordinate space between each
    // entry in the result array
    const double r_step = (double)(params.r_max - params.r_min) / (double)width;
    const double i_step = (double)(params.i_max - params.i_min) / (double)height;

    double r = params.r_min + (x*r_step);
    double i = params.i_max - (y*i_step);

    params.out[y*width + x] =
        mandelbrot_device_test_point(r, i, params.max_iterations);
}

__device__ static mandelbrot_result_t mandelbrot_device_test_point(double c_r, double c_i, size_t max_iterations)
{
    mandelbrot_result_t result = {.in_set = false, .iterations = max_iterations};

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

