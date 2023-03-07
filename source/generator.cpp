#include "rng.hpp"
#include "grid.hpp"
#include "stack.hpp"
#include "generator.hpp"

#include <memory.h>
#include <assert.h>

GENERATOR_FUNC(Generator::RandomizedDFS)
{
    State popped = {};
    if (stack.IsEmpty()) {
        memset(grid.cells, WALL, grid.hcells * grid.vcells);
        popped.dfs.x  = 0;
        popped.dfs.y  = 0;
        popped.dfs.at = 0;
    } else { popped = stack.Pop(); }

    auto &cstate = popped.dfs;
    int x = grid.active.x = cstate.x;
    int y = grid.active.y = cstate.y;

    auto cell = [&grid](int x, int y) -> unsigned char& {
        return grid.cells[y * grid.hcells + x];
    };

    if (cstate.at == 4) {
        return;
    } else if (cstate.at == 0) {
        cstate.choice[0] = L;
        cstate.choice[1] = R;
        cstate.choice[2] = B;
        cstate.choice[3] = T;
        RNG::Shuffle(4, cstate.choice);
        cell(x, y) = PATH;
    }

    bool next = false;
    auto nstate = cstate;
    nstate.at = 0;
    switch (cstate.choice[cstate.at]) {
        case L:
            if (!(x == 0 || cell(x - 2, y) != WALL)) {
                cell(x - 1, y) = PATH;
                nstate.x = x - 2, nstate.y = y;
                next = true;
            } break;
        case R:
            if (!(x > grid.hcells - 3 || cell(x + 2, y) != WALL)) {
                cell(x + 1, y) = PATH;
                nstate.x = x + 2, nstate.y = y;
                next = true;
            } break;
        case B:
            if (!(y == 0 || cell(x, y - 2) != WALL)) {
                cell(x, y - 1) = PATH;
                nstate.x = x, nstate.y = y - 2;
                next = true;
            } break;
        case T:
            if (!(y > grid.vcells - 3 || cell(x, y + 2) != WALL)) {
                cell(x, y + 1) = PATH;
                nstate.x = x, nstate.y = y + 2;
                next = true;
            } break;
    }

    cstate.at ++;
    stack.Push(popped);
    if (next) stack.Push({nstate});
}

GENERATOR_FUNC(Generator::RecursiveDivision)
{
    State popped = {};
    if (stack.IsEmpty()) {
        memset(grid.cells, PATH, grid.hcells * grid.vcells);
        popped.div.x = 0;
        popped.div.y = 0;
        popped.div.w = grid.hcells - 1;
        popped.div.h = grid.vcells - 1;
        popped.div.vertical = RNG::Get() & 1;
    } else { popped = stack.Pop(); }

    auto &cstate = popped.div;
    int x = cstate.x;
    int y = cstate.y;
    int w = cstate.w;
    int h = cstate.h;
    bool v = cstate.vertical;

    if (w - x <= 1 || h - y <= 1)
        return;

    auto cell = [&grid](int x, int y) -> unsigned char& {
        assert(x >= 0 && y >= 0 && x < grid.hcells && y < grid.vcells);
        return grid.cells[y * grid.hcells + x];
    };

    int m = RNG::Get(x, w - 1) | 1;
    int p = RNG::Get(y, h - 1) | 1;

    auto next = [&stack](int x, int w, int y, int h) -> void {
        State n;
        n.div.x = x; n.div.y = y;
        n.div.w = w; n.div.h = h;
        n.div.vertical = w - x > h - y;
        stack.Push(n);
    };

    if (v) {
        for (int i = y; i <= h; i ++)
            cell(m, i) = WALL;
        next(x, m - 1, y, h);
        next(m + 1, w, y, h);

        if (h - y > 5) {
            cell(m, RNG::Get(y, y + (h - y) / 2) & (~1)) = PATH;
            cell(m, RNG::Get(y + (h - y) / 2, h) & (~1)) = PATH;
        } else {
            cell(m, RNG::Get(y, h) & (~1)) = PATH;
        }
    } else {
        for (int i = x; i <= w; i ++)
            cell(i, p) = WALL;
        next(x, w, y, p - 1);
        next(x, w, p + 1, h);

        if (w - x > 5) {
            cell(RNG::Get(x, x + (w - x) / 2) & (~1), p) = PATH;
            cell(RNG::Get(x + (w - x) / 2, w) & (~1), p) = PATH;
        } else {
            cell(RNG::Get(x, w) & (~1), p) = PATH;
        }
    }
}
