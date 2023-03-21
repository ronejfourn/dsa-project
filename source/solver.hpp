#pragma once

struct Grid;
template <typename T> class Stack;
template <typename T> class Queue;

#define SOLVER_INIT_FUNC(name) void name(Grid &grid, Stack<Solver::State> &stack, Queue<Solver::Qitem> &queue)
#define SOLVER_STEP_FUNC(name) void name(Grid &grid, Stack<Solver::State> &stack, Queue<Solver::Qitem> &queue)

namespace Solver {

    enum {R, T, L, B};

    struct Vertex {
        int cost = -1;
        int hval = -1;
        unsigned char dir = 0;
        bool inQueue = false;
    };

    typedef int (*Heuristic)(const Grid &, int , int);

    union State {
        struct {
            int x, y;
            int dir;
            bool found;
        } dfs;

        struct QB {
            Heuristic heuristic;
            Vertex *vertices;
            int x, y;
            unsigned char dir;
            bool found;
            int total;
        };

        QB bfs;
        QB greedy;
        QB dijk;
        QB astar;
    };

    struct Qitem {
        int x, y;
        Vertex *v;
    };

    typedef SOLVER_INIT_FUNC((*InitFunc));
    SOLVER_INIT_FUNC(InitDFS);
    SOLVER_INIT_FUNC(InitBFS);
    SOLVER_INIT_FUNC(InitDijkstra);
    SOLVER_INIT_FUNC(InitAStarEuclidean);
    SOLVER_INIT_FUNC(InitAStarManhattan);
    SOLVER_INIT_FUNC(InitGreedyEuclidean);
    SOLVER_INIT_FUNC(InitGreedyManhattan);

    typedef SOLVER_STEP_FUNC((*StepFunc));
    SOLVER_STEP_FUNC(StepDFS);
    SOLVER_STEP_FUNC(StepBFS);
    SOLVER_STEP_FUNC(StepDijkstra);
    SOLVER_STEP_FUNC(StepAStar);
    SOLVER_STEP_FUNC(StepGreedy);
}
