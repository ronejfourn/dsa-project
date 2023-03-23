#include "maze.hpp"

#include <assert.h>
#include <SDL2/SDL_render.h>

bool Maze::PointInBounds(int x, int y) {
    return x >= 0 && y >= 0 && (unsigned)x < hcells && (unsigned)y < vcells;
}

unsigned &Maze::operator() (int x, int y) {
    if (!PointInBounds(x, y)) {
        printf("%d %d\n", x, y);
        assert(PointInBounds(x, y));
    }
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
    cells = nullptr;
    Resize(h, v);
}

void Maze::Resize(unsigned h, unsigned v) {
    start.x = 0, start.y = 0;
    end.x = h - 1, end.y = v - 1;
    hcells = h, vcells = v;
    delete[] cells;
    cells = new unsigned[hcells * vcells];
    Fill(PATH);
}

Maze::~Maze() {
    delete []cells;
}
