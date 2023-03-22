#pragma once

#include "stack.hpp"
#include "queue.hpp"

struct Maze;

class Solver {
public:
    typedef float (*Heuristic)(int x0, int y0, int x1, int y1);

    enum Type {
        AStar,
        Dijkstra,
        DepthFirst,
        BreadthFirst,
        GreedyBestFirst,
    };

     Solver();
    ~Solver();

    void Init(Maze *maze, Type type, Heuristic h);
    bool Step();
    bool StepAndTrace();

private:
    Maze *m_maze;
    bool m_finished = false;

    struct { int x, y; } m_end;
    struct { int x, y; } m_active;

    unsigned m_pathLength = 0;
    unsigned m_vertsVisited = 0;

    Heuristic _heuristic = nullptr;

    enum Direction : unsigned char {L, R, B, T};

    struct VertexData {
        float gval = -1;
        float hval = -1;
        unsigned char dir = 0;
        bool inQueue = false;
    };
    VertexData *m_vertices = nullptr;

    struct Sitem {
        int x, y;
        unsigned char dir;
    };

    struct Qitem {
        int x, y;
        VertexData *v;
    };

    Stack<Sitem> m_stack;
    Queue<Qitem> m_queue;

    void _reset();

    void _initAStar();
    void _initDijkstra();
    void _initDepthFirst();
    void _initBreadthFirst();
    void _initGreedyBestFirst();

    bool (Solver::*_step)() = nullptr;
    bool _stepAStar();
    bool _stepDijkstra();
    bool _stepDepthFirst();
    bool _stepBreadthFirst();
    bool _stepGreedyBestFirst();

    bool _isEnd(int x, int y);
    void _trace(unsigned);
};
