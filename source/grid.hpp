#pragma once

#include <climits>

enum CellState : unsigned {

    _MAX = 360,

    PATH = UINT_MAX - 1,
    WALL = UINT_MAX - 0,
};

struct SDL_Renderer;

struct Grid {
    unsigned hcells = 0;
    unsigned vcells = 0;
    unsigned *cells = nullptr;

     Grid() {}
     Grid(unsigned h, unsigned v);
    ~Grid();

    void Render(SDL_Renderer *);
    void Fill(unsigned);
    void ClearPaths();
    bool PointInBounds(int x, int y);
    unsigned &operator() (int x, int y);
};
