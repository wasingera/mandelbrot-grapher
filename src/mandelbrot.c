#include "mandelbrot.h"
#include <SDL3/SDL.h>

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
    size_t view_width;  /**< The width of the Mandelbrot view. (0,0) is top left */
    size_t view_height; /**< The height of the Mandelbrot view. (0,0) is top left */
    size_t iterations;  /**< The number of iterations to run before deciding if a
                          point is in the set */
    size_t segments;    /**< The number of segments to split x and y axes into */
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
 * @retval true The point is in the Mandelbrot set
 * @retval false The point is not in the Mandelbrot set
 */
static bool Mandelbrot_point_in_set(Mandelbrot_Handle* handle, double complex c);

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

static bool Mandelbrot_point_in_set(Mandelbrot_Handle* handle, double complex c)
{
    bool result = false;

    if (handle == NULL)
    {
        return result;
    }

    double complex z = 0.0f + (0.0f * I);

    // Mandlebrot computation: z_(n+1) = z_n^2 + c

    for (size_t i = 0; i < handle->iterations; i++)
    {
        // Calculate the next iteration
        z = cpow(z, 2) + c;

        // If z diverges, the magnitude will as well and
        // the point is not in the set
        if (cabs(z) >= 2.0f)
        {
            break;
        }

        // If we've reached the max number of iterations
        // and the point has not diverged, mark it as in
        // the set
        if (i == (handle->iterations - 1))
        {
            result = true;
        }
    }

    return result;
}

