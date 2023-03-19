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

        struct {
            int *costs;
            int *dirs;
            bool finished;

            int x, y;
            unsigned char dir, s;
        } dij;
    };

    union Qitem {
        struct {
            int x, y;
        } bfs;

        struct {
            int x, y;
            int cost;
        } dij;
    };

    typedef SOLVER_INIT_FUNC((*InitFunc));
    SOLVER_INIT_FUNC(InitDFS);
    SOLVER_INIT_FUNC(InitBFS);
    SOLVER_INIT_FUNC(InitDijkstra);

    typedef SOLVER_STEP_FUNC((*StepFunc));
    SOLVER_STEP_FUNC(StepDFS);
    SOLVER_STEP_FUNC(StepBFS);
    SOLVER_STEP_FUNC(StepDijkstra);
}
