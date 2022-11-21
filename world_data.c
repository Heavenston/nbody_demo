#include "world_data.h"

void add_body(
    WorldData *data,
    double x, double y,
    double vel_x, double vel_y,
    double mass, double radius,
    SDL_Color color
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

    data->colors = realloc(data->colors, sizeof(SDL_Color) * data->length);
    data->colors[i] = color;
}

void get_screen_coords(
    const WorldData * const data,
    const double ix, const double iy,
    int * const ox, int * const oy
) {
    *ox = (ix - data->view_x - data->pivot_x) / data->scale_x + data->pivot_x;
    *oy = (iy - data->view_y - data->pivot_y) / data->scale_y + data->pivot_y;
}

void get_world_coords(
    const WorldData * const data,
    const int ix, const int iy,
    double * const ox, double * const oy
) {
    *ox = ((double)ix - data->pivot_x) * data->scale_x + data->pivot_x + data->view_x;
    *ox = ((double)iy - data->pivot_y) * data->scale_y + data->pivot_y + data->view_y;
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
