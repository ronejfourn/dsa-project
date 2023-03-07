#pragma once

enum CellState{
    WALL,
    PATH,
    VISITED,
    CORRECT,
};

struct SDL_Renderer;

struct Grid {
    int hcells;
    int vcells;
    unsigned char *cells;
    struct {int x, y;} active;

     Grid(int h, int v);
    ~Grid();

    void Render(SDL_Renderer *);
};
