#pragma once

struct Grid;
template <typename T> struct Stack;

#define GENERATOR_FUNC(name) void name(Grid &grid, Stack<Generator::State> &stack)

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
        } krs;
    };

    typedef GENERATOR_FUNC((*Func));
    GENERATOR_FUNC(RandomizedDFS);
    GENERATOR_FUNC(RecursiveDivision);
    GENERATOR_FUNC(RandomizedKruskal);
};
