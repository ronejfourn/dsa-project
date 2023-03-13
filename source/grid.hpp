#pragma once

#include <climits>

enum CellState : unsigned {

    _MAX = 360,

    PATH = UINT_MAX - 1,
    WALL = UINT_MAX - 0,
};

struct SDL_Renderer;

struct Grid {
    unsigned hcells;
    unsigned vcells;
    unsigned *cells;

     Grid(unsigned h, unsigned v);
    ~Grid();

    void Render(SDL_Renderer *);
    void Fill(unsigned);
    void ClearPaths();
    unsigned &operator() (int x, int y);
};
