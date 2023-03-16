#pragma once

struct Grid;
template <typename T> struct Stack;

#define SOLVER_INIT_FUNC(name) void name(Grid &grid, Stack<Solver::State> &stack)
#define SOLVER_STEP_FUNC(name) void name(Grid &grid, Stack<Solver::State> &stack)

namespace Solver {

    enum {L, R, B, T};
    struct State {
        int x, y;
        int at;
        bool finished;
    };

    typedef SOLVER_INIT_FUNC((*InitFunc));
    SOLVER_INIT_FUNC(InitDFS);

    typedef SOLVER_STEP_FUNC((*StepFunc));
    SOLVER_STEP_FUNC(StepDFS);
}
