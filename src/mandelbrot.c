#include "mandelbrot.h"
#include "SDL3/SDL_stdinc.h"
#include <SDL3/SDL.h>
#include <math.h>

#include <stdbool.h>
#include <complex.h>
#include <stdlib.h>
#include <pthread.h>

//
// Internal structs
//

/**
 * @brief The range of the Mandelbrot window a thread is responsible
 * for computing
 */
typedef struct Mandelbrot_Range
{
    size_t x_min; /**< The min x value to start computation at */
    size_t x_max; /**< The max x value to start computation at */
    size_t y_min; /**< The min y value to start computation at */
    size_t y_max; /**< The max y value to start computation at */
} Mandelbrot_Range;

/**
 * @brief Internal state data of the Mandelbrot lib
 */
struct Mandelbrot_Handle
{
    size_t view_width;  /**< The width of the Mandelbrot view in pixels. (0,0) is top left */
    size_t view_height; /**< The height of the Mandelbrot view in pixels. (0,0) is top left */
    size_t iterations;  /**< The number of iterations to run before deciding if a
                          point is in the set */
    size_t segments;    /**< The number of segments to split x and y axes into */
    double plane_c_r; /**< The center real coordinate of the complex plane */
    double plane_c_i; /**< The center imaginary coordinate of the complex plane */
    double plane_size; /**< The "radius" of the complex plane to compute */
    Mandelbrot_Color *plane; /**< The color data for each point in the Mandelbrot
                               view. Obtained after computation */
};

/**
 * @brief Wrapper around handle and range used
 * to give the state to computation threads
 */
typedef struct Mandelbrot_ThreadArg
{
    Mandelbrot_Handle* handle; /**< Pointer to the internal state handle */
    Mandelbrot_Range*  range;  /**< Pointer to the range window this this should compute */
} Mandelbrot_ThreadArg;

//
// Internal function prototypes
//

/**
 * @brief Determines if a point is in the Mandelbrot set or not
 * @param c A normalized complex coordinate
 * @returns The number of iterations spend on computation. If equal
 * to handle->iterations, then the point is in the set
 */
static size_t Mandelbrot_point_in_set(Mandelbrot_Handle* handle, double complex c);

/**
 * @brief Compute Mandelbrot values over a specific range. Used for multithreading
 * @param arg Mandelbrot_ThreadArg wrapper around the handle and range
 */
void* Mandelbrot_computeRangeWorker(void* arg);

//
// Internal inline functions
//

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
        handle->segments = segments;
        handle->plane = (Mandelbrot_Color*) malloc(
                sizeof(Mandelbrot_Color) * view_height * view_width);

        // Main set view
        handle->plane_c_r = -0.75f;
        handle->plane_c_i = 0.0f;
        handle->plane_size = 1.25f;

        // Seahorse valley
        // handle->plane_c_r = -.745;
        // handle->plane_c_i = .113;
        // handle->plane_size = .02;

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

Mandelbrot_ReturnType Mandelbrot_deinit(Mandelbrot_Handle* handle)
{
    Mandelbrot_ReturnType result = E_NOT_OK;

    // Prevent deallocating a NULL pointer
    if (handle == NULL)
    {
        return result;
    }

    // If plane data not NULL, then deallocate it
    if (handle->plane != NULL)
    {
        free(handle->plane);
    }

    // Deallocate the state data
    free(handle);

    result = E_OK;
    return result;
}

Mandelbrot_ReturnType Mandelbrot_compute(Mandelbrot_Handle* handle)
{
    Mandelbrot_ReturnType result = E_NOT_OK;

    // Prevent computation if handle not valid
    if (handle == NULL)
    {
        return result;
    }

    size_t computation_windows = handle->segments * handle->segments;
    Mandelbrot_Range ranges[computation_windows];
    pthread_t threads[computation_windows];
    Mandelbrot_ThreadArg thread_args[computation_windows];

    size_t view_width_step = (handle->view_width / handle->segments);
    size_t view_height_step = (handle->view_height / handle->segments);

    // Set up viewing ranges
    for (size_t i = 0; i < handle->segments; i++)
    {
        for (size_t j = 0; j < handle->segments; j++)
        {
            size_t idx = (i * handle->segments) + j;
            ranges[idx].x_min = i * view_width_step;
            ranges[idx].x_max = (i * view_width_step) + view_width_step;
            ranges[idx].y_min = j * view_height_step;
            ranges[idx].y_max = (j * view_height_step) + view_height_step;

            thread_args[idx].handle = handle;
            thread_args[idx].range  = &ranges[idx];
        }
    }

    // Spawn threads to run the computation
    for (size_t i = 0; i < computation_windows; i++)
    {
        pthread_create(&threads[i], NULL, Mandelbrot_computeRangeWorker, (void*)(&thread_args[i]));
    }

    // Wait for threads to finish
    for (size_t i = 0; i < computation_windows; i++)
    {
        pthread_join(threads[i], NULL);
    }

    result = E_OK;
    return result;
}

void* Mandelbrot_computeRangeWorker(void* arg)
{
    Mandelbrot_ThreadArg* thread_arg = (Mandelbrot_ThreadArg*) arg;
    Mandelbrot_Handle* handle = thread_arg->handle;
    Mandelbrot_Range*  range  = thread_arg->range;

    for (size_t x = range->x_min; x < range->x_max; x++)
    {
        for (size_t y = range->y_min; y < range->y_max; y++)
        {
            // Convert the viewing window into abstract coordinates
            double x_scaled = Mandelbrot_map_range(x,
                    0, handle->view_width - 1,
                    handle->plane_c_r - handle->plane_size, handle->plane_c_r + handle->plane_size);

            double y_scaled = Mandelbrot_map_range(y,
                    0, handle->view_width - 1,
                    handle->plane_c_i - handle->plane_size, handle->plane_c_i + handle->plane_size);

            double complex c = x_scaled + (y_scaled * I);

            // If the point is in the set, shade it white
            size_t iterations = Mandelbrot_point_in_set(handle, c);
            handle->plane[(x*handle->view_width) + y].r = log(iterations + 1.0f) / log(handle->iterations);
            handle->plane[(x*handle->view_width) + y].g = log(iterations + 1.0f) / log(handle->iterations);
            handle->plane[(x*handle->view_width) + y].b = log(iterations + 1.0f) / log(handle->iterations);

            if (iterations == handle->iterations)
            {
                handle->plane[(x*handle->view_width) + y].r = 0.0f;
                handle->plane[(x*handle->view_width) + y].g = 0.0f;
                handle->plane[(x*handle->view_width) + y].b = 0.0f;
            }
        }
    }

    return NULL;
}

Mandelbrot_Color* Mandelbrot_getPointColor(Mandelbrot_Handle *handle, size_t x, size_t y)
{
    Mandelbrot_Color* result = NULL;

    // Prevent computation if handle not valid
    if (handle == NULL)
    {
        return result;
    }

    if ((0 <= x) && (x < handle->view_width) &&
        (0 <= y) && (y < handle->view_height))
    {
        result = &(handle->plane[(x*handle->view_width) + y]);
    }

    return result;
}

static size_t Mandelbrot_point_in_set(Mandelbrot_Handle* handle, double complex c)
{
    if (handle == NULL)
    {
        return handle->iterations;
    }

    double complex z = 0.0f + (0.0f * I);

    // Mandlebrot computation: z_(n+1) = z_n^2 + c

    size_t i;
    for (i = 0; i < handle->iterations; i++)
    {
        // Calculate the next iteration
        z = cpow(z, 2) + c;

        // If z diverges, the magnitude will as well and
        // the point is not in the set
        if (cabs(z) >= 2.0f)
        {
            break;
        }
    }

    return i;
}

Mandelbrot_ReturnType Mandelbrot_zoom(Mandelbrot_Handle *handle, size_t x, size_t y)
{
    Mandelbrot_ReturnType result = E_NOT_OK;

    if (handle == NULL)
    {
        return result;
    }

    double r_min_old = handle->plane_c_r - (handle->plane_size);
    double r_max_old = handle->plane_c_r + (handle->plane_size);

    double i_min_old = handle->plane_c_i - (handle->plane_size);
    double i_max_old = handle->plane_c_i + (handle->plane_size);

    double c_r_new = Mandelbrot_map_range(x, 0, handle->view_width - 1, r_min_old, r_max_old);
    double c_i_new = Mandelbrot_map_range(y, 0, handle->view_height - 1, i_min_old, i_max_old);

    double plane_size_new = handle->plane_size / 3.0f;

    handle->plane_c_r = c_r_new;
    handle->plane_c_i = c_i_new;
    handle->plane_size = plane_size_new;

    result = E_OK;
    return result;
}

Mandelbrot_ReturnType Mandelbrot_pan(Mandelbrot_Handle* handle, double r_t, double i_t)
{
    Mandelbrot_ReturnType result = E_NOT_OK;

    if (handle == NULL)
    {
        return result;
    }

    double x_scaled = Mandelbrot_map_range(r_t,
            0, handle->view_width - 1,
            handle->plane_c_r - handle->plane_size, handle->plane_c_r + handle->plane_size);

    double y_scaled = Mandelbrot_map_range(r_t,
            0, handle->view_width - 1,
            handle->plane_c_i - handle->plane_size, handle->plane_c_i + handle->plane_size);

    handle->plane_c_r += r_t * .125 * handle->plane_size;
    handle->plane_c_i += i_t * .125 * handle->plane_size;

    result = E_OK;
    return result;
}

