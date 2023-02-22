#include "rng.hpp"
#include "grid.hpp"
#include "state.hpp"
#include "stack.hpp"
#include "solver.hpp"

void solver::dfs(grid_t &grid, stack_t<state_t> &stack) {
    state_t cstate = {};
    if (!stack.is_empty())
        cstate = stack.pop();

    int x = cstate.x;
    int y = cstate.y;

    unsigned char wall = WALL;
    auto cell = [&grid, &wall](int x, int y) -> unsigned char& {
        if (x >= 0 && x <= grid.hcells - 1 && y >= 0 && y <= grid.vcells - 1)
            return grid.cells[y * grid.hcells + x];
        return wall;
    };

    if (cstate.finished || (x == grid.hcells - 1 && y == grid.vcells - 1)) {
        cell(x, y) = CORRECT;
        if (!stack.is_empty()) {
            auto s = stack.pop();
            s.finished = true;
            stack.push(s);
        }
        return;
    }

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
    state_t nstate = {};

    switch (cstate.choice[cstate.at]) {
        case L:
            if (cell(x - 1, y) != WALL && cell(x - 1, y) != VISITED) {
                nstate.x = x - 1;
                nstate.y = y;
                next = true;
            } break;
        case B:
            if (cell(x, y - 1) != WALL && cell(x, y - 1) != VISITED) {
                nstate.x = x;
                nstate.y = y - 1;
                next = true;
            } break;
        case R:
            if (cell(x + 1, y) != WALL && cell(x + 1, y) != VISITED) {
                nstate.x = x + 1;
                nstate.y = y;
                next = true;
            } break;
        case T:
            if (cell(x, y + 1) != WALL && cell(x, y + 1) != VISITED) {
                nstate.x = x;
                nstate.y = y + 1;
                next = true;
            } break;
    }

    cstate.at ++;
    stack.push(cstate);
    if (next) stack.push(nstate);
}
