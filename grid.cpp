#include "grid.hpp"
#include <stdlib.h>
#include <memory.h>

grid_t::grid_t(int cw, int ch, int hc, int vc, int bo) {
    cell_rect.w = cw;
    cell_rect.h = ch;
    border = bo;
    hcells = hc;
    vcells = vc;
    boundary.x = 0;
    boundary.y = 0;
    boundary.w = hcells * cell_rect.w + 2 * border;
    boundary.h = vcells * cell_rect.h + 2 * border;
    cells = new unsigned char[hcells * vcells];
    memset(cells, 0, hcells * vcells);
}

grid_t::~grid_t() {
    delete []cells;
}

void grid_t::center(int w, int h) {
    boundary.x = (w - boundary.w) / 2;
    boundary.y = (h - boundary.h) / 2;
}

void grid_t::render() {
    renderer_t &ren = renderer_t::instance();
    ren.draw(&boundary, 0xdedede);

    for (unsigned i = 0; i < vcells; i ++) {
        cell_rect.y = boundary.y + border + i * cell_rect.h;
        for (unsigned j = 0; j < hcells; j ++) {
            cell_rect.x = boundary.x + border + j * cell_rect.w;

            (cells[i * hcells + j] == WALL) ?
                ren.fill(&cell_rect, 0x000000):
            (cells[i * hcells + j] == VISITED) ?
                ren.fill(&cell_rect, 0x0000ff):
            (cells[i * hcells + j] == CORRECT) ?
                ren.fill(&cell_rect, 0x00ff00):
                ren.fill(&cell_rect, 0xffffff);
        }
    }
}

bool grid_t::screen_to_grid(int sx, int sy, int &gx, int &gy) {
    gx = (sx - boundary.x - border) / cell_rect.w;
    gy = (sy - boundary.y - border) / cell_rect.h;
    return (sx > boundary.x + border && sx < boundary.x + boundary.w - border &&
            sy > boundary.y + border && sy < boundary.y + boundary.h - border);
}

bool grid_t::grid_to_screen(int gx, int gy, int &sx, int &sy) {
    sx = boundary.x + border + gx * cell_rect.w;
    sy = boundary.y + border + gy * cell_rect.h;
    return gx >= 0 && gx < hcells && gy >= 0 && gy < vcells;
}
