#ifndef MANDELBROT_KERNEL_H
#define MANDELBROT_KERNEL_H

#include "mandelbrot.h"

#ifdef __cplusplus
extern "C" {
#endif

void mandelbrot_run_kernel(mandelbrot_run_params_t* params);

#ifdef __cplusplus
}
#endif

#endif

