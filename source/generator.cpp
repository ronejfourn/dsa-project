#include "rng.hpp"
#include "grid.hpp"
#include "state.hpp"
#include "stack.hpp"
#include "generator.hpp"

#include <memory.h>

GENERATOR_FUNC(Generator::RandomizedDFS)
{
    State cstate = {};
    if (stack.IsEmpty()) {
        memset(grid.cells, WALL, grid.hcells * grid.vcells);
        cstate.x  = 0;
        cstate.y  = 0;
        cstate.at = 0;
    } else { cstate = stack.Pop(); }

    int x = cstate.x;
    int y = cstate.y;

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
    State nstate = {};
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
    stack.Push(cstate);
    if (next) stack.Push(nstate);
}
