#include "maze.hpp"

#include <assert.h>
#include <SDL2/SDL_render.h>

bool Maze::PointInBounds(int x, int y) {
    return x >= 0 && y >= 0 && (unsigned)x < hcells && (unsigned)y < vcells;
}

unsigned &Maze::operator() (int x, int y) {
    assert(PointInBounds(x, y));
    return cells[y * hcells + x];
};

void Maze::Fill(unsigned v) {
    for (unsigned i = 0; i < hcells * vcells; i ++)
        cells[i] = v;
}

void Maze::ClearPaths() {
    for (unsigned i = 0; i < hcells * vcells; i ++)
        cells[i] = cells[i] == WALL ? WALL : PATH;
}

Maze::Maze(unsigned h, unsigned v) {
    hcells = h, vcells = v;
    cells = new unsigned[hcells * vcells];
    Fill(PATH);
}

Maze::~Maze() {
    delete []cells;
}

void Maze::Render(SDL_Renderer *r) {
    for (unsigned y = 0; y < vcells; y++) {
        auto b = y * hcells;
        for (unsigned x = 0; x < hcells; x++) {
            auto t = cells[b + x];

            (t == PATH) ? SDL_SetRenderDrawColor(r, 0xfb, 0xf1, 0xc7, 0x00) :
            (t == WALL) ? SDL_SetRenderDrawColor(r, 0x1d, 0x20, 0x21, 0x00) :
            SDL_SetRenderDrawColor(r, (t >> 16) & 0xff, (t >> 8) & 0xff, (t >> 0) & 0xff, 0x00);

            SDL_RenderDrawPoint(r, x, y);
        }
    }
}
