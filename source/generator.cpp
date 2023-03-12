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

GENERATOR_FUNC(Generator::RandomizedKruskal)
{
    auto hhcells = grid.hcells >> 1;
    auto hvcells = grid.vcells >> 1;
    auto nverts  = ((hhcells + 1) * (hvcells + 1));
    auto nedges  = (hvcells * hhcells * 2) + hhcells + hvcells;

    if (stack.IsEmpty()) {
        State s; Edge e;
        memset(grid.cells, WALL, grid.hcells * grid.vcells);
        s.krs.at = 0;
        s.krs.edges = new Edge[nedges];
        s.krs.verts = new Vert[nverts];

        unsigned i = 0;
        for (int x = 0; x < grid.hcells - 2; x += 2) {
            e.x0 = x, e.x1 = x + 2;
            for (int y = 0; y < grid.vcells; y += 2) {
                e.y0 = e.y1 = y;
                s.krs.edges[i++] = e;
            }
        }

        for (int y = 0; y < grid.vcells - 2; y += 2) {
            e.y0 = y, e.y1 = y + 2;
            for (int x = 0; x < grid.hcells; x += 2) {
                e.x0 = e.x1 = x;
                s.krs.edges[i++] = e;
            }
        }

        for (unsigned i = 0; i < nverts; i ++)
            s.krs.verts[i].forest = i;

        RNG::Shuffle(nedges, s.krs.edges);
        stack.Push(s);
    }

    auto &s = stack.Peek();
    if (s.krs.at >= nedges) {
        delete [] s.krs.edges;
        delete [] s.krs.verts;
        stack.Pop();
        return;
    }

    auto cell = [&grid](int x, int y) -> unsigned char& {
        return grid.cells[y * grid.hcells + x];
    };

    auto e  = s.krs.edges[s.krs.at ++];
    auto v0 = (e.y0 / 2) * (hhcells + 1) + (e.x0 / 2);
    auto v1 = (e.y1 / 2) * (hhcells + 1) + (e.x1 / 2);
    auto f0 = s.krs.verts[v0].forest;
    auto f1 = s.krs.verts[v1].forest;

    if (f0 != f1) {
        for (unsigned i = 0; i < nverts; i++)
            if (s.krs.verts[i].forest == f1)
                s.krs.verts[i].forest = f0;

        cell(e.x0, e.y0) = PATH;
        cell(e.x1, e.y1) = PATH;
        cell(e.x0 + (e.x1 - e.x0) / 2, e.y0 + (e.y1 - e.y0) / 2) = PATH;
    }
}
