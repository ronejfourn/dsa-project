#pragma once

struct Grid;
template <typename T> class Stack;
template <typename T> class Queue;

#define SOLVER_INIT_FUNC(name) void name(Grid &grid, Stack<Solver::State> &stack, Queue<Solver::Qitem> &queue)
#define SOLVER_STEP_FUNC(name) void name(Grid &grid, Stack<Solver::State> &stack, Queue<Solver::Qitem> &queue)

namespace Solver {

    enum {L, R, B, T};
    union State {
        struct {
            int x, y;
            int at;
            bool finished;
        } dfs;

        struct {
            unsigned char *verts;
            unsigned nitems;
            unsigned s;
            unsigned c;
            int x, y;
        } bfs;

    };
    union Qitem {
        struct {
            int x, y;
        } bfs;

    };

    typedef SOLVER_INIT_FUNC((*InitFunc));
    SOLVER_INIT_FUNC(InitDFS);
    SOLVER_INIT_FUNC(InitBFS);

    typedef SOLVER_STEP_FUNC((*StepFunc));
    SOLVER_STEP_FUNC(StepDFS);
    SOLVER_STEP_FUNC(StepBFS);
}
