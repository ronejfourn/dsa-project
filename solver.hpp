#pragma once

struct grid_t;
struct state_t;
template <typename T> struct stack_t;

namespace solver {
    void dfs(grid_t &grid, stack_t<state_t> &stack);
}
