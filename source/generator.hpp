#pragma once

struct Grid;
template <typename T> struct Stack;

#define GENERATOR_INIT_FUNC(name) void name(Grid &grid, Stack<Generator::State> &stack)
#define GENERATOR_STEP_FUNC(name) void name(Grid &grid, Stack<Generator::State> &stack)

namespace Generator {

    enum {L, R, B, T};

    struct Edge { int x0, y0, x1, y1; };
    union State {
        struct {
            int x, y;
            int at;
            int choice[4];
        } dfs;

        struct {
            int x, y;
            int w, h;
            bool vertical;
        } div;

        struct {
            Edge *edges;
            unsigned *vertForest;
            unsigned at;
            unsigned hhcells, hvcells;
            unsigned nverts, nedges;
        } krs;

        struct {
            Edge *edges;
            unsigned availableEdges;
        } prm;
    };

    typedef GENERATOR_INIT_FUNC((*InitFunc));
    GENERATOR_INIT_FUNC(InitRandom);
    GENERATOR_INIT_FUNC(InitRandomizedDFS);
    GENERATOR_INIT_FUNC(InitRecursiveDivision);
    GENERATOR_INIT_FUNC(InitRandomizedKruskal);
    GENERATOR_INIT_FUNC(InitRandomizedPrim);

    typedef GENERATOR_STEP_FUNC((*StepFunc));
    GENERATOR_INIT_FUNC(StepRandom);
    GENERATOR_STEP_FUNC(StepRandomizedDFS);
    GENERATOR_STEP_FUNC(StepRecursiveDivision);
    GENERATOR_STEP_FUNC(StepRandomizedKruskal);
    GENERATOR_STEP_FUNC(StepRandomizedPrim);
};
