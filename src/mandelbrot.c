#include "mandelbrot.h"
#include <stdbool.h>
#include <complex.h>
#include <SDL3/SDL.h>
#include <stdlib.h>

//
// Internal structs
//

/**
 * @brief Internal state data of the Mandelbrot lib
 */
struct Mandelbrot_Handle
{
    size_t view_width;  /**< The width of the Mandelbrot view. (0,0) is top left */
    size_t view_height; /**< The height of the Mandelbrot view. (0,0) is top left */
    size_t iterations;  /**< The number of iterations to run before deciding if a
                          point is in the set */
    Mandelbrot_Color *plane; /**< The color data for each point in the Mandelbrot
                               view. Obtained after computation */
};

//
// Internal function prototypes
//

/**
 * @brief Determines if a point is in the Mandelbrot set or not
 * @param c A normalized complex coordinate
 * @retval true The point is in the Mandelbrot set
 * @retval false The point is not in the Mandelbrot set
 */
static bool Mandelbrot_point_in_set(Mandelbrot_Handle* handle, double complex c);

/**
 * @brief Maps a coordinate scale into another
 * @param in_min The minimum value of the input range
 * @param in_max The maximum value of the input range
 * @param out_min The minimum value of the output range
 * @param out_max The maximum value of the output range
 * @returns The remapped coordinate value
 */
static inline double Mandelbrot_map_range(double x,
        double in_min, double in_max, double out_min, double out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

//
// Function implementations
//

Mandelbrot_Handle* Mandelbrot_init(size_t view_width, size_t view_height,
        size_t iterations, size_t segments)
{
    Mandelbrot_ReturnType result = E_NOT_OK;

    // Initialize memory for the handle
    Mandelbrot_Handle* handle = (Mandelbrot_Handle*) malloc(sizeof(Mandelbrot_Handle));

    // Initialize memory for the result values
    if (handle)
    {
        handle->view_width = view_width;
        handle->view_height = view_height;
        handle->iterations = iterations;
        handle->plane = (Mandelbrot_Color*) malloc(
                sizeof(Mandelbrot_Color) * view_height * view_width);

        // If no memory for the plane data, then deallocate everything
        // and return NULL pointer
        if (!handle->plane)
        {
            free(handle);
            handle = NULL;
        }
    }

    return handle;
}

Mandelbrot_ReturnType Mandelbrot_compute(Mandelbrot_Handle* handle)
{
    Mandelbrot_ReturnType result = E_NOT_OK;

    for (size_t x = 0; x < handle->view_width; x++)
    {
        for (size_t y = 0; y < handle->view_width; y++)
        {
            // Convert the viewing window into abstract coordinates
            double x_scaled = Mandelbrot_map_range(x, 0,
                    handle->view_width - 1, -2.0f, 0.5f);
            double y_scaled = Mandelbrot_map_range(y, 
                    handle->view_height - 1, 0, -1.25, 1.25);

            double complex c = x_scaled + (y_scaled * I);

            // If the point is in the set, shade it white
            if (Mandelbrot_point_in_set(handle, c))
            {
                handle->plane[(x*handle->view_width) + y].r = 1.0f;
                handle->plane[(x*handle->view_width) + y].g = 1.0f;
                handle->plane[(x*handle->view_width) + y].b = 1.0f;
            }
            // Otherwise, shade it black
            else
            {
                handle->plane[(x*handle->view_width) + y].r = 0.0f;
                handle->plane[(x*handle->view_width) + y].g = 0.0f;
                handle->plane[(x*handle->view_width) + y].b = 0.0f;
            }
        }
    }

    result = E_OK;
    return result;
}

Mandelbrot_Color* Mandelbrot_getPointColor(Mandelbrot_Handle *handle, size_t x, size_t y)
{
    Mandelbrot_Color* result = NULL;

    if ((0 <= x) && (x < handle->view_width) &&
        (0 <= y) && (y < handle->view_height))
    {
        result = &(handle->plane[(x*handle->view_width) + y]);
    }

    return result;
}

static bool Mandelbrot_point_in_set(Mandelbrot_Handle* handle, double complex c)
{
    double mag = 0.0f;

    double complex z = 0.0f + (0.0f * I);

    // z_(n+1) = z_n^2 + c

    for (size_t i = 0; i < handle->iterations; i++)
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

