#include "rng.hpp"
#include "grid.hpp"
#include "state.hpp"
#include "stack.hpp"
#include "solver.hpp"

void solver::dfs(grid_t &grid, stack_t<state_t> &stack) {
    state_t cstate, nstate;
    if (stack.is_empty()) {
        cstate.x  = 0;
        cstate.y  = 0;
        cstate.at = 0;
    } else { cstate = stack.pop(); }

    int x = cstate.x;
    int y = cstate.y;

    if (x == grid.hcells - 1 && y == grid.vcells - 1) {
        stack.clear();
        return;
    }

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
        cell(x, y) = VISITED;
    }

    bool next = false;

    switch (cstate.choice[cstate.at]) {
        case L:
            if (x != 0)
                if (cell(x - 1, y) != WALL && cell(x - 1, y) != VISITED)
                    nstate = {0, x - 1, y}, next = true;
            break;
        case B:
            if (y != 0)
                if (cell(x, y - 1) != WALL && cell(x, y - 1) != VISITED)
                    nstate = {0, x, y - 1}, next = true;
            break;
        case R:
            if (x < grid.hcells - 1)
                if (cell(x + 1, y) != WALL && cell(x + 1, y) != VISITED)
                    nstate = {0, x + 1, y}, next = true;
            break;
        case T:
            if (y < grid.vcells - 1)
                if (cell(x, y + 1) != WALL && cell(x, y + 1) != VISITED)
                    nstate = {0, x, y + 1}, next = true;
            break;
    }

    cstate.at ++;
    stack.push(cstate);
    if (next) stack.push(nstate);
}
