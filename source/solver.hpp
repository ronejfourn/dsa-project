#pragma once

struct Grid;
template <typename T> struct Stack;

#define SOLVER_FUNC(name) void name(Grid &grid, Stack<Solver::State> &stack)

namespace Solver {

    enum {L, R, B, T};
    struct State {
        int x, y;
        int at;
        bool finished;
    };

    typedef SOLVER_FUNC((*Func));
    SOLVER_FUNC(DFS);
}
