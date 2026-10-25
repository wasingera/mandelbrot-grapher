/*
 * This example code creates an SDL window and renderer, and then clears the
 * window to a different color every frame, so you'll effectively get a window
 * that's smoothly fading between colors.
 *
 * This code is public domain. Feel free to use it for any purpose!
 */

#include "SDL3/SDL_render.h"
#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "app_defs.h"
#include "mandelbrot.h"
#include <math.h>

/* We will use this renderer to draw into this window every frame. */
static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

static inline double translate_point(double p, double in_min, double in_max, double out_min, double out_max)
{
    return ((p - in_min) * (out_max - out_min)) / (in_max - in_min) + out_min;
}

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    SDL_AppResult result = SDL_APP_CONTINUE;

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

    app_state_t *app_state_init = SDL_malloc(sizeof(app_state_t));

    // Set to true so view is drawn on first render
    app_state_init->mandelbrot.update = true;
    app_state_init->mandelbrot.c_r = -0.75;
    app_state_init->mandelbrot.c_i = -0.0;
    app_state_init->mandelbrot.scale = 3.0;

    *appstate = app_state_init;

    return result;  /* carry on with the program! */
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    app_state_t *app_state = (app_state_t*)appstate;

    // Continue program operation by default
    SDL_AppResult result = SDL_APP_CONTINUE;

    if (event->type == SDL_EVENT_QUIT) {
        result = SDL_APP_SUCCESS;  /* end the program, reporting success to the OS. */
    } else if (event->type == SDL_EVENT_KEY_DOWN)
    {
        float x_pos, y_pos;
        SDL_GetMouseState(&x_pos, &y_pos);

        double r_min = app_state->mandelbrot.c_r - (app_state->mandelbrot.scale / 2.0);
        double r_max = app_state->mandelbrot.c_r + (app_state->mandelbrot.scale / 2.0);
        double i_min = app_state->mandelbrot.c_i - (app_state->mandelbrot.scale / 2.0);
        double i_max = app_state->mandelbrot.c_i + (app_state->mandelbrot.scale / 2.0);


        switch(event->key.key)
        {
            case SDLK_SPACE:
                app_state->mandelbrot.c_r = translate_point(x_pos, 0, WINDOW_WIDTH, r_min, r_max);
                app_state->mandelbrot.c_i = translate_point(y_pos, 0, WINDOW_HEIGHT, i_max, i_min);
                app_state->mandelbrot.scale /= 2.0;
                app_state->mandelbrot.update = true;
                break;
            case SDLK_DOWN:
                app_state->mandelbrot.c_i -= app_state->mandelbrot.scale / 10.0f;
                app_state->mandelbrot.update = true;
                break;
            case SDLK_UP:
                app_state->mandelbrot.c_i += app_state->mandelbrot.scale / 10.0f;
                app_state->mandelbrot.update = true;
                break;
            case SDLK_LEFT:
                app_state->mandelbrot.c_r -= app_state->mandelbrot.scale / 10.0f;
                app_state->mandelbrot.update = true;
                break;
            case SDLK_RIGHT:
                app_state->mandelbrot.c_r += app_state->mandelbrot.scale / 10.0f;
                app_state->mandelbrot.update = true;
                break;
            default:
                break;
        }
    }
    return result;
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void *appstate)
{
    app_state_t *app_state = (app_state_t*)appstate;

    SDL_AppResult result = SDL_APP_CONTINUE;
    // Mandelbrot_Handle* handle = (Mandelbrot_Handle*) appstate;

    const uint64_t now = SDL_GetTicks();
    static uint64_t then = 0U;

    // Only refresh at 60Hz
    // 60Hz = 0.01666s/tick = 16ms
    if ((now - then) <= 16U)
    {
        return SDL_APP_CONTINUE;
    }
    then = now;

    mandelbrot_result_t *mandelbrot_results = malloc(sizeof(mandelbrot_result_t) * WINDOW_HEIGHT * WINDOW_WIDTH);

    if (app_state->mandelbrot.update)
    {
        // Clear the render buffer
        SDL_SetRenderDrawColorFloat(renderer, 0, 
                0, 0, SDL_ALPHA_OPAQUE_FLOAT);  /* new color, full alpha. */
        SDL_RenderClear(renderer);

        SDL_Log("Updating...\n");

        mandelbrot_run(&(mandelbrot_run_params_t){
                .result_height = WINDOW_HEIGHT,
                .result_width  = WINDOW_WIDTH,
                .r_min = app_state->mandelbrot.c_r - (app_state->mandelbrot.scale / 2.0),
                .r_max = app_state->mandelbrot.c_r + (app_state->mandelbrot.scale / 2.0),
                .i_min = app_state->mandelbrot.c_i - (app_state->mandelbrot.scale / 2.0),
                .i_max = app_state->mandelbrot.c_i + (app_state->mandelbrot.scale / 2.0),
                .max_iterations = 1000,
                .out = mandelbrot_results
                });

    }

    for (size_t x = 0; x < WINDOW_WIDTH; x++)
    {
        for (size_t y = 0; y < WINDOW_HEIGHT; y++)
        {
            // If view position has changed, then
            // update the pixel buffer
            if (app_state->mandelbrot.update)
            {
                mandelbrot_result_t *result
                    = &mandelbrot_results[y*WINDOW_WIDTH + x];

                app_state->pixels[x][y][0] = log(1.0f + result->iterations) / log(1000);
                app_state->pixels[x][y][1] = log(1.0f + result->iterations) / log(1000);
                app_state->pixels[x][y][2] = log(1.0f + result->iterations) / log(1000);

                SDL_SetRenderDrawColorFloat(renderer,
                        app_state->pixels[x][y][0],
                        app_state->pixels[x][y][1],
                        app_state->pixels[x][y][2],
                        1.0f);

                SDL_RenderPoint(renderer, x, y);
            }

        }
    }

    if (app_state->mandelbrot.update)
    {
        SDL_RenderPresent(renderer);
    }

    app_state->mandelbrot.update = false;
    mandelbrot_free_results(mandelbrot_results);

    return result;  /* carry on with the program! */
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    (void)result;

    app_state_t *app_state = (app_state_t*)appstate;

    /* SDL will clean up the window/renderer for us. */
    SDL_free(app_state);
}

