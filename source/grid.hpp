#pragma once

#include "renderer.hpp"

enum cell_state_t{
    WALL,
    PATH,
    VISITED,
    CORRECT,
};

struct grid_t {
    rect_t cell_rect;
    int border;
    int hcells;
    int vcells;
    rect_t boundary;
    unsigned char *cells;

     grid_t(int cw, int ch, int hc, int vc, int bo = 4);
    ~grid_t();

    void center(int w, int h);
    void render();
    bool screen_to_grid(int sx, int sy, int &gx, int &gy);
    bool grid_to_screen(int gx, int gy, int &sx, int &sy);
};
