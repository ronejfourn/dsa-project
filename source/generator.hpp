#pragma once

struct Grid;
struct State;
template <typename T> struct Stack;

#define GENERATOR_FUNC(name) void name(Grid &grid, Stack<State> &stack)

namespace Generator {
    typedef GENERATOR_FUNC((*Func));
    GENERATOR_FUNC(RandomizedDFS);
};
