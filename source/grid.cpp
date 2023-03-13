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

static void SetColorHSV(SDL_Renderer *renderer, unsigned h, float s, float v)
{
    auto abs = [](int v) -> int {
        return v < 0 ? -v : v;
    };

    constexpr unsigned k = _MAX / 6;
    float c = v * s;
    float x = c * (1 - abs(((h / k) & 1) - 1));
    float m = v - c;
    float r, g, b;

    if (h < k)          r = c, g = x, b = 0;
    else if (h < 2 * k) r = x, g = c, b = 0;
    else if (h < 3 * k) r = 0, g = c, b = x;
    else if (h < 4 * k) r = 0, g = x, b = c;
    else if (h < 5 * k) r = x, g = 0, b = c;
    else                r = c, g = 0, b = x;

    r = (r + m) * 255;
    g = (g + m) * 255;
    b = (b + m) * 255;
    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
}

void Grid::Render(SDL_Renderer *r) {
    for (unsigned y = 0; y < vcells; y++) {
        auto b = y * hcells;
        for (unsigned x = 0; x < hcells; x++) {
            auto t = cells[b + x];
            auto s = t < PATH;
            auto v = t < WALL;
            auto h = t * (s && v);
            SetColorHSV(r, h, s, v);
            SDL_RenderDrawPoint(r, x, y);
        }
    }
}
