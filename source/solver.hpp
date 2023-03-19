#pragma once

struct Grid;
template <typename T> class Stack;
template <typename T> class Queue;

#define SOLVER_INIT_FUNC(name) void name(Grid &grid, Stack<Solver::State> &stack, Queue<Solver::Qitem> &queue)
#define SOLVER_STEP_FUNC(name) void name(Grid &grid, Stack<Solver::State> &stack, Queue<Solver::Qitem> &queue)

namespace Solver {

    enum {L, R, B, T};

    struct Vertex {
        int cost = -1;
        int heuristic = -1;
        unsigned char dir = 0;
        bool inQueue = false;
    };

    union State {
        struct {
            int x, y;
            unsigned char choice[4];
            int at;
            bool found;
        } dfs;

        struct QB {
            Vertex *vertices;
            int x, y;
            unsigned char dir;
            bool found;
        };

        QB bfs;
        QB dij;
        QB astr;
    };

    struct Qitem {
        int x, y;
        Vertex *v;
    };

    typedef SOLVER_INIT_FUNC((*InitFunc));
    SOLVER_INIT_FUNC(InitDFS);
    SOLVER_INIT_FUNC(InitBFS);
    SOLVER_INIT_FUNC(InitDijkstra);
    SOLVER_INIT_FUNC(InitAStar);

    typedef SOLVER_STEP_FUNC((*StepFunc));
    SOLVER_STEP_FUNC(StepDFS);
    SOLVER_STEP_FUNC(StepBFS);
    SOLVER_STEP_FUNC(StepDijkstra);
    SOLVER_STEP_FUNC(StepAStar);
}
