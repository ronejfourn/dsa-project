#pragma once

struct Grid;
template <typename T> struct Stack;

#define GENERATOR_INIT_FUNC(name) void name(Grid &grid, Stack<Generator::State> &stack)
#define GENERATOR_STEP_FUNC(name) void name(Grid &grid, Stack<Generator::State> &stack)

namespace Generator {

    enum {L, R, B, T};

    struct Edge { unsigned x0, y0, x1, y1; };
    struct Vert { unsigned forest; };
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
            Vert *verts;
            unsigned at;
            unsigned hhcells, hvcells;
            unsigned nverts, nedges;
        } krs;
    };

    typedef GENERATOR_INIT_FUNC((*InitFunc));
    GENERATOR_INIT_FUNC(InitRandomizedDFS);
    GENERATOR_INIT_FUNC(InitRecursiveDivision);
    GENERATOR_INIT_FUNC(InitRandomizedKruskal);

    typedef GENERATOR_STEP_FUNC((*StepFunc));
    GENERATOR_STEP_FUNC(StepRandomizedDFS);
    GENERATOR_STEP_FUNC(StepRecursiveDivision);
    GENERATOR_STEP_FUNC(StepRandomizedKruskal);
};
