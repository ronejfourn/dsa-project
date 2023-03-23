#pragma once

#include "stack.hpp"

struct Maze;

class Generator {
public:
    enum Type {
        Random,
        RandomizedDFS,
        RecursiveDivision,
        RandomizedKruskal,
        RandomizedPrim,
    };

     Generator();
    ~Generator();

    void Init(Maze *maze, Type type);
    bool Step();

private:
    bool m_finished = false;
    Maze *m_maze = nullptr;

    enum Direction : unsigned char {L, R, B, T};
    struct Edge { int x0, y0, x1, y1; };

    union Sitem {
        struct {
            int x, y, at;
            int choice[4];
        } dfs;

        struct {
            int x, y;
            int w, h;
            bool vertical;
        } div;
    };
    Stack<Sitem> m_stack;

    struct {
        unsigned nverts = 0, nedges = 0;
        unsigned *verts = nullptr;
        Edge     *edges = nullptr;
        unsigned at = 0;
    } m_graph;

    void _reset();

    void _initRandom();
    void _initRandomizedDFS();
    void _initRecursiveDivision();
    void _initRandomizedKruskal();
    void _initRandomizedPrim();

    bool (Generator::*_step)() = nullptr;
    bool _stepRandom();
    bool _stepRandomizedDFS();
    bool _stepRecursiveDivision();
    bool _stepRandomizedKruskal();
    bool _stepRandomizedPrim();
};
