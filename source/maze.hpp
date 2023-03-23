#pragma once

#include <climits>

enum CellState : unsigned {
    PATH = 0xfbf1c7,
    WALL = 0x1d2021,
};

struct SDL_Renderer;

struct Maze {
    unsigned hcells = 0;
    unsigned vcells = 0;
    unsigned *cells = nullptr;
    struct { int x, y; } start = {0, 0};
    struct { int x, y; } end   = {0, 0};

     Maze() {}
     Maze(unsigned h, unsigned v);
    ~Maze();

    void Fill(unsigned);
    void Resize(unsigned h, unsigned v);
    void ClearPaths();
    bool PointInBounds(int x, int y);
    unsigned &operator() (int x, int y);
};
