#pragma once

struct Grid;
struct State;
template <typename T> struct Stack;

#define SOLVER_FUNC(name) void name(Grid &grid, Stack<State> &stack)

namespace Solver {
    typedef SOLVER_FUNC((*Func));
    SOLVER_FUNC(DFS);
}
