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
    unsigned *cells;

     Grid(unsigned h, unsigned v);
    ~Grid();

    void Render(SDL_Renderer *);
    void Fill(unsigned);
    void ClearPaths();
    unsigned &operator() (int x, int y);
};
