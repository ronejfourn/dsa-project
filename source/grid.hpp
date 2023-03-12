#pragma once

enum CellState{
    WALL,
    PATH,
    VISITED,
    CORRECT,
};

struct SDL_Renderer;

struct Grid {
    unsigned hcells;
    unsigned vcells;
    unsigned char *cells;
    struct {int x, y;} active;

     Grid(unsigned h, unsigned v);
    ~Grid();

    void Render(SDL_Renderer *);
};
