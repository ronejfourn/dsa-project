#include "grid.hpp"
#include <SDL2/SDL_render.h>
#include <stdlib.h>
#include <memory.h>

Grid::Grid(int h, int v) {
    hcells = h, vcells = v;
    active.x = -1;
    active.y = -1;
    cells = new unsigned char[hcells * vcells];
    memset(cells, 0, hcells * vcells);
}

Grid::~Grid() {
    delete []cells;
}

void Grid::Render(SDL_Renderer *r) {
    for (int y = 0; y < vcells; y++) {
        for (int x = 0; x < hcells; x++) {
            switch (cells[y * hcells + x]) {
                case WALL   : SDL_SetRenderDrawColor(r, 0x00, 0x00, 0x00, 0x00); break;
                case PATH   : SDL_SetRenderDrawColor(r, 0xff, 0xff, 0xff, 0x00); break;
                case VISITED: SDL_SetRenderDrawColor(r, 0x00, 0x00, 0xff, 0x00); break;
                case CORRECT: SDL_SetRenderDrawColor(r, 0x00, 0xff, 0x00, 0x00); break;
            }
            SDL_RenderDrawPoint(r, x, y);
        }
    }

    if (active.x >= 0 && active.x < hcells && active.y >= 0 && active.y < vcells) {
        SDL_SetRenderDrawColor(r, 0xff, 0x00, 0x00, 0x00);
        SDL_RenderDrawPoint(r, active.x, active.y);
    }
}
