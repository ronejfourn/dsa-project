#include "rng.hpp"
#include "grid.hpp"
#include "stack.hpp"
#include "solver.hpp"

SOLVER_FUNC(Solver::DFS) {
    State cstate = {};
    if (!stack.IsEmpty())
        cstate = stack.Pop();
    else
        grid.ClearPaths();

    int x = cstate.x;
    int y = cstate.y;

    unsigned wall = WALL;
    auto cell = [&grid, &wall](int x, int y) -> unsigned& {
        if (x >= 0 && x <= grid.hcells - 1 && y >= 0 && y <= grid.vcells - 1)
            return grid(x, y);
        return wall;
    };

    if (cstate.finished || (x == grid.hcells - 1 && y == grid.vcells - 1)) {
        cell(x, y) = 0xff;
        if (!stack.IsEmpty())
            stack.Peek().finished = true;
        return;
    }

    if (cstate.at == 4) {
        cell(x, y) = 0x77;
        return;
    } else if (cstate.at == 0) {
        cell(x, y) = 0x33;
    }

    bool next = false;
    State nstate = {};

    switch (cstate.at) {
        case L:
            if (cell(x - 1, y) == PATH) {
                nstate.x = x - 1;
                nstate.y = y;
                next = true;
            } break;
        case R:
            if (cell(x + 1, y) == PATH) {
                nstate.x = x + 1;
                nstate.y = y;
                next = true;
            } break;
        case B:
            if (cell(x, y - 1) == PATH) {
                nstate.x = x;
                nstate.y = y - 1;
                next = true;
            } break;
        case T:
            if (cell(x, y + 1) == PATH) {
                nstate.x = x;
                nstate.y = y + 1;
                next = true;
            } break;
    }

    cstate.at ++;
    stack.Push(cstate);
    if (next) stack.Push(nstate);
}
