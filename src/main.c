/*
 * This example code creates an SDL window and renderer, and then clears the
 * window to a different color every frame, so you'll effectively get a window
 * that's smoothly fading between colors.
 *
 * This code is public domain. Feel free to use it for any purpose!
 */

#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "mandelbrot.h"

#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 1000
#define ITERATIONS 1000

/* We will use this renderer to draw into this window every frame. */
static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    SDL_AppResult result = SDL_APP_FAILURE;

    SDL_SetAppMetadata("Mandelbrot Renderer", "1.0", "com.mandelbrot.renderer");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return result;
    }

    if (!SDL_CreateWindowAndRenderer("examples/renderer/clear", 
                WINDOW_WIDTH, WINDOW_HEIGHT, 0, &window, &renderer)) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return result;
    }
    SDL_SetRenderLogicalPresentation(renderer, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_LOGICAL_PRESENTATION_LETTERBOX);

    // Initialize the mandlebrot library
    Mandelbrot_Handle* handle = Mandelbrot_init(
            WINDOW_WIDTH, WINDOW_HEIGHT,
            ITERATIONS, 4
        );

    if (handle != NULL)
    {
        if (Mandelbrot_compute(handle) == E_OK)
        {
            result = SDL_APP_CONTINUE;
            *appstate = (void*) handle;
        }
    }

    return result;  /* carry on with the program! */
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    // Continue program operation by default
    SDL_AppResult result = SDL_APP_CONTINUE;

    if (event->type == SDL_EVENT_QUIT) {
        result = SDL_APP_SUCCESS;  /* end the program, reporting success to the OS. */
    }
    return result;
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void *appstate)
{
    SDL_AppResult result = SDL_APP_FAILURE;
    Mandelbrot_Handle* handle = (Mandelbrot_Handle*) appstate;

    const uint64_t now = SDL_GetTicks();
    static uint64_t then = 0U;

    // Only refresh at 60Hz
    if ((now - then) <= 16U)
    {
        return SDL_APP_CONTINUE;
    }
    then = now;

    /* choose the color for the frame we will draw. 
     * The sine wave trick makes it fade between colors smoothly. */
    SDL_SetRenderDrawColorFloat(renderer, 0, 
            0, 0, SDL_ALPHA_OPAQUE_FLOAT);  /* new color, full alpha. */

    /* clear the window to the draw color. */
    SDL_RenderClear(renderer);

    for (size_t x = 0; x < WINDOW_WIDTH; x++)
    {
        for (size_t y = 0; y < WINDOW_HEIGHT; y++)
        {
            Mandelbrot_Color* color = Mandelbrot_getPointColor(handle, x, y);

            if (color != NULL)
            {
                SDL_SetRenderDrawColorFloat(
                        renderer,
                        color->r,
                        color->g,
                        color->b,
                        SDL_ALPHA_OPAQUE_FLOAT
                    );

                SDL_RenderPoint(renderer, x, y);
            }
            else
            {
                break;
            }

            // If all points were successfully processed,
            // then we can continue operation
            if ((x == WINDOW_WIDTH - 1) && (y == WINDOW_HEIGHT - 1))
            {
                result = SDL_APP_CONTINUE;
            }

        }
    }

    /* put the newly-cleared rendering on the screen. */
    if (result == SDL_APP_CONTINUE)
    {
        SDL_RenderPresent(renderer);
    }

    return result;  /* carry on with the program! */
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    /* SDL will clean up the window/renderer for us. */
    Mandelbrot_Handle* handle = (Mandelbrot_Handle*) appstate;
    Mandelbrot_deinit(handle);
}

