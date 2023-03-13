#include "grid.hpp"

#include <assert.h>
#include <SDL2/SDL_render.h>

unsigned &Grid::operator() (int x, int y) {
    assert(x >= 0 && y >= 0 && x < hcells && y < vcells);
    return cells[y * hcells + x];
};

void Grid::Fill(unsigned v) {
    for (unsigned i = 0; i < hcells * vcells; i ++)
        cells[i] = v;
}

void Grid::ClearPaths() {
    for (unsigned i = 0; i < hcells * vcells; i ++)
        cells[i] = cells[i] == WALL ? WALL : PATH;
}

Grid::Grid(unsigned h, unsigned v) {
    hcells = h, vcells = v;
    cells = new unsigned[hcells * vcells];
    Fill(PATH);
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
}
