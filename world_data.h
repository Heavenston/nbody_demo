#pragma once

#include "SDL2/SDL.h"

typedef struct {
    /// The coordinates of the top left corner viewport/camera.
    double  view_x,  view_y;
    /// Pivot used for rotation (mathematical origin of the scaling)
    double pivot_x, pivot_y;
    /// The scaling of the camera (2 means the viewport is twice as big as the world)
    double scale_x, scale_y;

    /// Number of bodies in the world.
    size_t length;
    double *vel_x, *vel_y;
    double *pos_x, *pos_y;
    double *masses;
    double *radiuses;
    SDL_Color *colors;
} WorldData;

/** Adds a body the the world data by appending elements to the various arrays.
 */
void add_body(
    WorldData *data,
    double x, double y,
    double vel_x, double vel_y,
    double mass, double radius,
    SDL_Color color
);

/** Converts world coordinates into screen coordinates.
 */
void get_screen_coords(
    const WorldData * const data,
    const double ix, const double iy,
    int * const ox, int * const oy
);

/** Converts screen coordinates into world coordinates.
 */
void get_world_coords(
    const WorldData * const data,
    const int ix, const int iy,
    double * const ox, double * const oy
);

/** Removes the body at the given index by puting the last elements it its place.
 */
void swap_remove_body(WorldData *data, size_t i);
