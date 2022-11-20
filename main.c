#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define SPHERE_FALLOUT_START 250

#define G 0.00000000006672

typedef struct {
    double view_x;
    double scale_x;
    double view_y;
    double scale_y;

    size_t length;
    double *vel_x;
    double *vel_y;
    double *pos_x;
    double *pos_y;
    double *masses;
    double *radiuses;
} WorldData;

void add_body(
    WorldData *data,
    double x, double y,
    double vel_x, double vel_y,
    double mass, double radius
) {
    size_t i = data->length;
    data->length++;

    data->vel_x = realloc(data->vel_x, sizeof(double) * data->length);
    data->vel_x[i] = vel_x;

    data->vel_y = realloc(data->vel_y, sizeof(double) * data->length);
    data->vel_y[i] = vel_y;

    data->pos_x = realloc(data->pos_x, sizeof(double) * data->length);
    data->pos_x[i] = x;

    data->pos_y = realloc(data->pos_y, sizeof(double) * data->length);
    data->pos_y[i] = y;

    data->masses = realloc(data->masses, sizeof(double) * data->length);
    data->masses[i] = mass;

    data->radiuses = realloc(data->radiuses, sizeof(double) * data->length);
    data->radiuses[i] = radius;
}

void get_screen_coords(
    const WorldData * const data, const double ix, const double iy, int * const ox, int * const oy) {
    *ox = (int)floor((ix - data->view_x) * data->scale_x);
    *oy = (int)floor((iy - data->view_y) * data->scale_y);
}

void swap_remove_body(WorldData *data, size_t i) {
    size_t j = data->length - 1;
    data->vel_x[i] = data->vel_x[j];
    data->vel_y[i] = data->vel_y[j];
    data->pos_x[i] = data->pos_x[j];
    data->pos_y[i] = data->pos_y[j];

    data->masses[i] = data->masses[j];
    data->radiuses[i] = data->radiuses[j];
    
    data->length--;
}

void draw_sphere(
    SDL_Renderer * const renderer,
    const WorldData * const data,
    const double x, const double y, const double radius
) {
    double radius2 = radius*radius;
    for (double dx = x - radius; dx <= x + radius; dx++) {
        for (double dy = y - radius; dy <= y + radius; dy++) {
            double dist = (dx-x)*(dx-x) + (dy-y)*(dy-y);
            
            double fallout_dist = (dist - (radius2 - SPHERE_FALLOUT_START)) / SPHERE_FALLOUT_START;
            if (fallout_dist > 1)
                fallout_dist = 1.;
            if (fallout_dist < 0)
                fallout_dist = 0.;
            double color = 1. - fallout_dist;

            if (dist <= radius2) {
                int color_2 = floor(color * 255);
                SDL_SetRenderDrawColor(renderer, color_2, color_2, color_2, 255);
                
                int idx, idy;
                get_screen_coords(data, dx, dy, &idx, &idy);
                SDL_RenderDrawPoint(
                    renderer,
                    idx,
                    idy
                );
            }
        }
    }
}

void update(WorldData * const data, double dt) {
    for (size_t i = 1; i < data->length; i++) {
        for (size_t j = 0; j < i; j++) {
            double diffX = data->pos_x[j] - data->pos_x[i];
            double diffX2 = diffX * diffX;
            double diffY = data->pos_y[j] - data->pos_y[i];
            double diffY2 = diffY * diffY;

            double dist2 = diffX2 + diffY2;
            
            if (dist2 < 250) {
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

void draw(SDL_Renderer * const renderer, const WorldData * const data) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    for (size_t i = 0; i < data->length; i++) {
        draw_sphere(
            renderer, data,
            data->pos_x[i], data->pos_y[i],
            data->radiuses[i]
        );
    }
}

void update_viewport(
    WorldData * const data,
    int previous_width, int previous_height,
    int new_width, int new_height
) {
    double cx = data->view_x + ((double)previous_width  / 2);
    double cy = data->view_y + ((double)previous_height / 2);
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
        SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
        SCREEN_HEIGHT, SDL_WINDOW_SHOWN
    );
    if (window == NULL) {
        fprintf(stderr, "could not create window: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);
    
    size_t time_scale = 2;
    
    WorldData data = {
        .scale_x = 1.,
        .scale_y = 1.,
        .length = 0
    };
    add_body(&data, 0, 0, 0, 0, 300000000000000000, 30);
    add_body(&data, 0, 200, 300, 0, 10000, 20);
    add_body(&data, 0, 400, 200, 0, 10000, 20);
    
    int window_width, window_height;
    SDL_GetWindowSize(window, &window_width, &window_height);
    update_viewport(&data, 0, 0, window_width, window_height);
    
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

        double dt = (double)(SDL_GetTicks() - last_frame_ticks) / 1000;
        for (size_t i = 0; i < time_scale; i++)
            update(&data, dt);
        draw(renderer, &data);
        SDL_RenderPresent(renderer);

        last_frame_ticks = SDL_GetTicks();
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
