#ifndef MANDELBROT_H
#define MANDELBROT_H

#include <stddef.h>

//
// Types -- Enums
//

/**
 * @brief Standard return types (PASS/FAIL) for Mandelbrot
 * library operations
 */
typedef enum Mandelbrot_ReturnType
{
    E_NOT_OK,    /**< The result of the operation was a failure */
    E_OK         /**< The result of the operation was a success */
} Mandelbrot_ReturnType;

//
// Types -- Structs
//

/**
 * @brief Handle for Mandlebrot library state
 */
typedef struct Mandelbrot_Handle Mandelbrot_Handle;

/**
 * @brief Container for the RGB values of a
 * point/pixel in the Mandelbrot viewing window
 */
typedef struct Mandelbrot_Color
{
    double r; /**< The red channel value */
    double g; /**< The green channel value */
    double b; /**< The blue channel value */
} Mandelbrot_Color;

//
// Function definitions
//

/**
 * @brief Initialize the state of the mandelbrot view
 * @param view_width The width of the Mandelbrot viewing window
 * @param view_height The height of the Mandelbrot viewing window
 * @param iterations The number of iterations to perform before
 * deciding if a point is in the set
 * @param segments The number of segments to divide the x and y
 * axes into. Will spawn segments^2 number of threads during
 * computation
 * @retval E_OK Initialization succeeded
 * @retval E_NOT_OK Initialization failed
 */
Mandelbrot_Handle* Mandelbrot_init(size_t view_width, size_t view_height,
        size_t iterations, size_t segments);

/**
 * @brief Compute the values for a mandelbrot set
 * @param handle A pointer to the Mandelbrot library handle
 * @retval E_OK Computation succeeded
 * @retval E_NOT_OK Computation failed
 */
Mandelbrot_ReturnType Mandelbrot_compute(Mandelbrot_Handle* handle);

/**
 * @brief Gets a pointer to the color data struct for a point
 * in the Mandelbrot view
 * @param handle A pointer to the Mandelbrot library handle
 * @param x The x coordinate of the window pixel
 * @param y The y coordinate of the window pixel
 * @returns A pointer to the Mandelbrot color structure for the point,
 * NULL if x or y is invalid/out of range
 */
Mandelbrot_Color* Mandelbrot_getPointColor(Mandelbrot_Handle* handle,
        size_t x, size_t y);

/**
 * @brief Deallocates the Mandelbrot library instance
 * @param handle A pointer to the Mandelbrot library handle
 * @retval E_OK Deinitialization succeeded
 * @retval E_NOT_OK Deinitialization failed
 */
Mandelbrot_ReturnType Mandelbrot_deinit(Mandelbrot_Handle* handle);

/**
 * @brief Zooms the view by 2x on the mouse cursor
 * @param handle A pointer to the Mandelbrot library handle
 * @retval E_OK Deinitialization succeeded
 * @retval E_NOT_OK Deinitialization failed
 */
Mandelbrot_ReturnType Mandelbrot_zoom(Mandelbrot_Handle* handle, size_t x, size_t y);

#endif

