#include "mandelbrot.h"
#include <stdbool.h>
#include <math.h>
#include <complex.h>
#include <SDL3/SDL.h>

Mandelbrot_View state = { 0 };

static bool Mandelbrot_point_in_set(double complex c, size_t iterations);
static double Mandelbrot_map_range(double x,
        double in_min, double in_max, double out_min, double out_max);

double Mandelbrot_map_range(double x,
        double in_min, double in_max, double out_min, double out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void Mandelbrot_init(void)
{
    state.center_x = 0.0f;
    state.center_y = 0.0f;
}

void Mandelbrot_compute(size_t x_min, size_t x_max, size_t y_min,
        size_t y_max, size_t iterations)
{
    for (size_t x = 0; x < WINDOW_WIDTH; x++)
    {
        for (size_t y = 0; y < WINDOW_HEIGHT; y++)
        {
            double x_scaled = Mandelbrot_map_range(x, 0, WINDOW_WIDTH - 1, -2.0f, 0.5f);
            double y_scaled = Mandelbrot_map_range(y, WINDOW_HEIGHT - 1, 0, -1.25, 1.25);

            double complex c = x_scaled + (y_scaled * I);

            if (Mandelbrot_point_in_set(c, iterations))
            {
                state.plane[x][y][0] = 1.0f;
                state.plane[x][y][1] = 1.0f;
                state.plane[x][y][2] = 1.0f;
            }
            else
            {
                state.plane[x][y][0] = 0.0f;
                state.plane[x][y][1] = 0.0f;
                state.plane[x][y][2] = 0.0f;
            }
        }
    }
}

static bool Mandelbrot_point_in_set(double complex c, size_t iterations)
{
    double mag = 0.0f;

    double complex z = 0.0f + (0.0f * I);

    // z_(n+1) = z_n^2 + c

    for (size_t i = 0; i < iterations; i++)
    {
        // Calculate the next iteration
        z = cpow(z, 2) + c;

        // If z diverges, the magnitude will as well and
        // the point is not in the set
        if (cabs(z) >= 2.0f)
        {
            return false;
        }
    }

    return true;
}
