#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#include "world_data.h"

/// Size of the "anti aliased" border of the spheres.
#define SPHERE_FALLOUT_START 3

/// The G constant, I think
#define G 0.0000000000667430

/** Draw a sphere with the given world coodinates onto the screen.
 */
void draw_sphere(
    SDL_Renderer * const renderer,
    const WorldData * const data,
    const double wx, const double wy, const double wradius,
    SDL_Color color
) {
    int sx, sy;
    get_screen_coords(data, wx, wy, &sx, &sy);
    int sradius_x = wradius / data->scale_x;
    int sradius_y = wradius / data->scale_y;

    double wradius2 = wradius*wradius;
    double sradius2 = sradius_x*sradius_y;
    
    for (int dsx = sx - sradius_x; dsx <= sx + sradius_x; dsx++) {
        for (int dsy = sy - sradius_y; dsy <= sy + sradius_y; dsy++) {
            double sdist2 = (dsx-sx)*(dsx-sx) + (dsy-sy)*(dsy-sy);
            double sdist = sqrt(sdist2);
            
            double fallout_dist = (sdist - (sradius_x - SPHERE_FALLOUT_START)) / SPHERE_FALLOUT_START;
            if (fallout_dist > 1)
                fallout_dist = 1.;
            if (fallout_dist < 0)
                fallout_dist = 0.;

            if (sdist2 <= sradius2) {
                int alpha = (int)floor(255. * (1. - fallout_dist));
                SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, alpha);
                
                SDL_RenderDrawPoint(
                    renderer,
                    dsx,
                    dsy
                );
            }
        }
    }
}

/** Does a physic step of the given time step.
 */
void update(WorldData * const data, const double dt) {
    for (size_t i = 1; i < data->length; i++) {
        for (size_t j = 0; j < i; j++) {
            double diffX = data->pos_x[j] - data->pos_x[i];
            double diffX2 = diffX * diffX;
            double diffY = data->pos_y[j] - data->pos_y[i];
            double diffY2 = diffY * diffY;

            double dist2 = diffX2 + diffY2;
            
            if (dist2 < 100) {
                printf("Collision %zu %zu!\n", i, j);
                if (data->masses[j] > data->masses[i]) {
                    data->masses[j] += data->masses[i];
                    float mass_factor = data->masses[i] / data->masses[j];
                    data->vel_x[j] += data->vel_x[i] * mass_factor;
                    data->vel_y[j] += data->vel_y[i] * mass_factor;
                    swap_remove_body(data, i);
                    i--;
                    return;
                }
                else {
                    data->masses[i] += data->masses[j];
                    float mass_factor = data->masses[j] / data->masses[i];
                    data->vel_x[i] += data->vel_x[j] * mass_factor;
                    data->vel_y[i] += data->vel_y[j] * mass_factor;
                    swap_remove_body(data, j);
                    j--;
                    return;
                }
            }
            
            double dist = sqrt(dist2);
            double x_dir = diffX / dist;
            double y_dir = diffY / dist;
            
            double i_f = G * data->masses[j] / dist2;
            double j_f = G * data->masses[i] / dist2;
            
            data->vel_x[i] += x_dir * i_f * dt;
            data->vel_y[i] += y_dir * i_f * dt;

            data->vel_x[j] -= x_dir * j_f * dt;
            data->vel_y[j] -= y_dir * j_f * dt;
        }
        
        skip_main: (void)0;
    }

    for (size_t i = 0; i < data->length; i++) {
        data->pos_x[i] += data->vel_x[i] * dt;
        data->pos_y[i] += data->vel_y[i] * dt;
    }
}

/** Draws all bodies from the world into the screen.
 */
void draw(SDL_Renderer * const renderer, const WorldData * const data) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    for (size_t i = 0; i < data->length; i++) {
        draw_sphere(
            renderer, data,
            data->pos_x[i], data->pos_y[i],
            data->radiuses[i],
            data->colors[i]
        );
    }
}

/** Used to update the position of the "camera" when the window resizes, also resets
 *  its position if the previous width or height is equal to 0.
 */
void update_viewport(
    WorldData * const data,
    int previous_width, int previous_height,
    int new_width, int new_height
) {
    double cx = data->view_x + ((double)previous_width  / 2);
    double cy = data->view_y + ((double)previous_height / 2);
    if (previous_height == 0 || previous_width == 0) {
        cx = 0, cy = 0;
        data->scale_x = 1;
        data->scale_y = 1;
    }

    data->pivot_x = ((double)new_width  / 2);
    data->pivot_y = ((double)new_height  / 2);
    data->view_x = cx - ((double)new_width  / 2);
    data->view_y = cy - ((double)new_height / 2);
}

int main(int argc, char *args[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "could not initialize sdl2: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow(
        "hello_sdl2",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        200, 200,
        SDL_WINDOW_SHOWN
    );
    if (window == NULL) {
        fprintf(stderr, "could not create window: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    
    size_t time_scale = 2;
    
    WorldData data = {
        .scale_x = 1.,
        .scale_y = 1.,
        .view_x = 0,
        .view_y = 0,
        .length = 0
    };
    
    add_body(&data, 0, 0, 0, 0,   30000000000000, 30, (SDL_Color) {
        .r = 255, .g = 255, .b = 0, .a = 255,
    });
    add_body(&data, 0, 200, 3, 0, 10000, 20, (SDL_Color) {
        .r = 100, .g = 100, .b = 255, .a = 255,
    });
    add_body(&data, 0, 400, 2, 0, 10000, 20, (SDL_Color) {
        .r = 255, .g = 100, .b = 100, .a = 255,
    });
    
    int window_width, window_height;
    SDL_GetWindowSize(window, &window_width, &window_height);
    update_viewport(&data, 0, 0, window_width, window_height);
    
    double click_start_x = NAN, click_start_y = NAN;
    
    bool running = true;
    Uint32 last_frame_ticks = SDL_GetTicks();
    while (running) {
        SDL_Event event;
        bool found_event = false;
        int delay = 5 - ((int)SDL_GetTicks() - (int)last_frame_ticks);
        found_event = SDL_WaitEventTimeout(
            &event, delay < 1 ? 1 : delay
        );

        while (found_event) {
            switch (event.type) {
                case SDL_QUIT:
                    running = false;
                    break;
                case SDL_KEYDOWN: {
                    double move_step = SDL_GetModState() & KMOD_SHIFT ? 50 : 10;
                    move_step *= data.scale_x;
                    double scale_step = SDL_GetModState() & KMOD_SHIFT ? 1.5 : 1.1;
                    switch (event.key.keysym.sym) {
                        case SDLK_h:
                            data.view_x -= move_step;
                        break;
                        case SDLK_l:
                            data.view_x += move_step;
                        break;
                        case SDLK_j:
                            data.view_y += move_step;
                        break;
                        case SDLK_k:
                            data.view_y -= move_step;
                        break;
                        case SDLK_r:
                            update_viewport(&data, 0, 0, window_width, window_height);
                        break;
                        case SDLK_RIGHT:
                            time_scale++;
                        break;
                        case SDLK_LEFT:
                            if (time_scale > 1)
                                time_scale--;
                        break;
                        case SDLK_UP:
                            data.scale_x /= scale_step;
                            data.scale_y /= scale_step;
                        break;
                        case SDLK_DOWN:
                            data.scale_x *= scale_step;
                            data.scale_y *= scale_step;
                        break;
                    }
                    break;
                }
                case SDL_MOUSEBUTTONDOWN: {
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        get_world_coords(&data, event.button.x, event.button.y, &click_start_x, &click_start_y);
                    }
                    else if (event.button.button == SDL_BUTTON_RIGHT) {
                        double click_x, click_y;
                        get_world_coords(&data, event.button.x, event.button.y, &click_x, &click_y);
                        for (size_t i = 0; i < data.length; i++) {
                            double x_dist = data.pos_x[i] - click_x;
                            double y_dist = data.pos_y[i] - click_y;
                            double dist2 = x_dist*x_dist + y_dist*y_dist;
                            if (dist2 < data.radiuses[i]*data.radiuses[i]) {
                                swap_remove_body(&data, i);
                                i--;
                            }
                        }
                    }
                    break;
                }
                case SDL_MOUSEBUTTONUP: {
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        double click_x, click_y;
                        get_world_coords(&data, event.button.x, event.button.y, &click_x, &click_y);
                        add_body(&data, click_start_x, click_start_y,
                            (click_start_x - click_x) / 10,
                            (click_start_y - click_y) / 10,
                            10000000000000, 20, (SDL_Color) {
                                .r = 255, .g = 255, .b = 255, .a = 255,
                            });
                        click_start_x = NAN,
                        click_start_y = NAN;
                    }
                    break;
                }
                case SDL_WINDOWEVENT:
                    switch (event.window.event) {
                        case SDL_WINDOWEVENT_RESIZED: {
                            update_viewport(
                                &data, 
                                window_width, window_height,
                                event.window.data1, event.window.data2
                            );
                            window_width = event.window.data1;
                            window_height = event.window.data2;
                            break;
                        }
                    }
                    break;
            }
            
            found_event = SDL_PollEvent(&event);
        }

        double dt = 0.005;
        dt *= 10;
        for (size_t i = 0; i < time_scale; i++)
            update(&data, dt);
        draw(renderer, &data);
        if (!isnan(click_start_x)) {
            int msx, msy;
            SDL_GetMouseState(&msx, &msy);
            int start_msx, start_msy;
            get_screen_coords(&data, click_start_x, click_start_y, &start_msx, &start_msy);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderDrawLine(renderer, msx, msy, start_msx, start_msy);
        }
        SDL_RenderPresent(renderer);

        last_frame_ticks = SDL_GetTicks();
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
