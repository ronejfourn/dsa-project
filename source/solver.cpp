#include "solver.hpp"
#include "maze.hpp"

#define ACTIVE  0xcc241d
#define FOUND   0x076678
#define DEAD    0xfabd2f

Solver:: Solver() {}
Solver::~Solver() {_reset();}

void Solver::Init(Maze *maze, Type type, Heuristic h)
{
    _reset();
    m_maze = maze;
    m_end = { (int)(maze->hcells - 1), (int)(maze->vcells - 1) };
    m_vertices = new VertexData[maze->hcells * maze->vcells];
    _heuristic = h;

#define CASE(_NAME) case Type::_NAME : _init##_NAME(); _step = &Solver::_step##_NAME; break
    switch (type) {
        CASE(AStar);
        CASE(Dijkstra);
        CASE(DepthFirst);
        CASE(BreadthFirst);
        CASE(GreedyBestFirst);
    };
#undef CASE
}

void Solver::_trace(unsigned t)
{
    int x = m_active.x, y = m_active.y;
    (*m_maze)(x, y) = t;
    while (y != 0 || x != 0) {
        switch (m_vertices[y * m_maze->hcells + x].dir) {
            case L: x ++; break;
            case R: x --; break;
            case B: y ++; break;
            case T: y --; break;
        }
        (*m_maze)(x, y) = t;
    }
}

bool Solver::Step()
{
    bool res = (this->*_step)();
    if (!res) {
        _trace(FOUND);
        _reset();
    }
    return res;
}

bool Solver::StepAndTrace()
{
    _trace(DEAD);
    bool res = (this->*_step)();
    _trace(ACTIVE);

    if (!res) {
        _trace(FOUND);
        _reset();
    }
    return res;
}

void Solver::_reset()
{
    m_maze = nullptr;
    m_finished = false;
    m_pathLength = 0;
    m_vertsVisited = 0;
    m_active = {0, 0};
    delete [] m_vertices;
    m_vertices = nullptr;
    m_stack.Clear();
    m_queue.Clear();
}

bool Solver::_isEnd(int x, int y)
{
    return x == m_end.x && y == m_end.y;
}

////////////////////////////////
// Depth First Search
////////////////////////////////

void Solver::_initDepthFirst()
{
    m_stack.Push({});
    (*m_maze)(0, 0) = ACTIVE;
}

bool Solver::_stepDepthFirst()
{
    if (m_stack.IsEmpty())
        return false;

    auto &i = m_stack.Peek();
    m_active = {i.x, i.y};
    if (_isEnd(i.x, i.y))
        return false;

    bool next = false;
    Sitem nstate = {};

    auto setnext = [this, &i, &nstate](int x, int y, unsigned char dir) -> bool {
        if (!m_maze->PointInBounds(x, y) || (*m_maze)(x, y) != PATH)
            return false;

        m_vertices[y * m_maze->hcells + x].dir = dir;
        nstate.x = x;
        nstate.y = y;
        return true;
    };

    while (!next && i.dir < 4) {
        switch (i.dir++) {
            case L: next = setnext(i.x - 1, i.y, L); break;
            case R: next = setnext(i.x + 1, i.y, R); break;
            case B: next = setnext(i.x, i.y - 1, B); break;
            case T: next = setnext(i.x, i.y + 1, T); break;
        }
    }

    if (next) {
        (*m_maze)(nstate.x, nstate.y) = ACTIVE;
        m_stack.Push(nstate);
    } else {
        (*m_maze)(i.x, i.y) = DEAD;
        m_stack.Pop();
    }
    return true;
}

////////////////////////////////
// Breadth First Search
////////////////////////////////

void Solver::_initBreadthFirst()
{
    (*m_maze)(0, 0) = ACTIVE;
    m_queue.Enqueue({});
}

bool Solver::_stepBreadthFirst()
{
    if (m_queue.IsEmpty())
        return false;

    auto i = m_queue.Dequeue();
    m_active = {i.x, i.y};
    if (_isEnd(i.x, i.y))
        return false;

    auto enqueue = [this](int x, int y, unsigned char dir) {
        if (!m_maze->PointInBounds(x, y) || (*m_maze)(x, y) != PATH)
            return;

        m_vertices[y * m_maze->hcells + x].dir = dir;
        Qitem q = {};
        q.x = x;
        q.y = y;
        (*m_maze)(x, y) = ACTIVE;
        m_queue.Enqueue(q);
    };

    (*m_maze)(i.x, i.y) = DEAD;
    enqueue(i.x - 1, i.y, L);
    enqueue(i.x + 1, i.y, R);
    enqueue(i.x, i.y - 1, B);
    enqueue(i.x, i.y + 1, T);
    return true;
}

////////////////////////////////
// Dijkstra's Algorithm
////////////////////////////////

void Solver::_initDijkstra()
{
    m_vertices[0].gval = 0;
    (*m_maze)(0, 0) = ACTIVE;

    Qitem q = {};
    q.v = m_vertices;
    q.v->inQueue = true;
    m_queue.Enqueue(q);

    m_queue.SetCompareFunc([](auto a, auto b) -> bool {
        return a.v->gval < b.v->gval;
    });
}

bool Solver::_stepDijkstra()
{
    if (m_queue.IsEmpty())
        return false;

    auto i = m_queue.PriorityDequeue();
    i.v->inQueue = false;
    m_active = {i.x, i.y};
    if (_isEnd(i.x, i.y))
        return false;

    auto gval = i.v->gval + 1;
    auto enqueue = [this, gval](int x, int y, unsigned char dir) {
        if (!m_maze->PointInBounds(x, y) || (*m_maze)(x, y) == WALL)
            return;

        auto &vert = m_vertices[y * m_maze->hcells + x];
        if (vert.gval != -1 && vert.gval <= gval)
            return;

        vert.gval = gval;
        vert.dir  = dir;
        if (vert.inQueue)
            return;

        vert.inQueue = true;
        Qitem q = {};
        q.x = x;
        q.y = y;
        q.v = &vert;
        (*m_maze)(q.x, q.y) = ACTIVE;
        m_queue.Enqueue(q);
    };

    (*m_maze)(i.x, i.y) = DEAD;
    enqueue(i.x - 1, i.y, L);
    enqueue(i.x + 1, i.y, R);
    enqueue(i.x, i.y - 1, B);
    enqueue(i.x, i.y + 1, T);
    return true;
}

////////////////////////////////
// Greedy Best First Search
////////////////////////////////

void Solver::_initGreedyBestFirst()
{
    m_vertices[0].hval = _heuristic(0, 0, m_end.x, m_end.y);
    (*m_maze)(0, 0) = ACTIVE;

    Qitem q = {};
    q.v = m_vertices;
    m_queue.Enqueue(q);

    m_queue.SetCompareFunc([](auto a, auto b) -> bool {
        return a.v->hval < b.v->hval;
    });
}

bool Solver::_stepGreedyBestFirst()
{
    if (m_queue.IsEmpty())
        return false;

    auto i = m_queue.PriorityDequeue();
    m_active = {i.x, i.y};
    if (_isEnd(i.x, i.y))
        return false;

    auto enqueue = [this](int x, int y, unsigned char dir) {
        if (!m_maze->PointInBounds(x, y) || (*m_maze)(x, y) != PATH)
            return;

        auto i = y * m_maze->hcells + x;
        m_vertices[i].dir  = dir;
        m_vertices[i].hval = _heuristic(x, y, m_end.x, m_end.y);

        Qitem q = {};
        q.x = x;
        q.y = y;
        q.v = &m_vertices[i];
        (*m_maze)(x, y) = ACTIVE;
        m_queue.Enqueue(q);
    };

    (*m_maze)(i.x, i.y) = DEAD;
    enqueue(i.x - 1, i.y, L);
    enqueue(i.x + 1, i.y, R);
    enqueue(i.x, i.y - 1, B);
    enqueue(i.x, i.y + 1, T);
    return true;
}

////////////////////////////////
// A*
////////////////////////////////

void Solver::_initAStar()
{
    m_vertices[0].gval = 0;
    m_vertices[0].hval = _heuristic(0, 0, m_end.x, m_end.y);
    (*m_maze)(0, 0) = ACTIVE;

    Qitem q = {};
    q.v = m_vertices;
    q.v->inQueue = true;
    m_queue.Enqueue(q);

    m_queue.SetCompareFunc([](auto a, auto b) -> bool {
        return (a.v->gval + a.v->hval) < (b.v->gval + b.v->hval);
    });
}

bool Solver::_stepAStar()
{
    if (m_queue.IsEmpty())
        return false;

    auto i = m_queue.PriorityDequeue();
    i.v->inQueue = false;
    m_active = {i.x, i.y};
    if (_isEnd(i.x, i.y))
        return false;

    auto gval = i.v->gval + 1;
    auto enqueue = [this, gval](int x, int y, unsigned char dir) {
        if (!m_maze->PointInBounds(x, y) || (*m_maze)(x, y) == WALL)
            return;

        auto &vert = m_vertices[y * m_maze->hcells + x];
        if (vert.gval != -1 && vert.gval <= gval)
            return;

        vert.gval = gval;
        vert.dir  = dir;
        vert.hval = _heuristic(x, y, m_end.x, m_end.y);
        if (vert.inQueue)
            return;

        vert.inQueue = true;
        Qitem q = {};
        q.x = x;
        q.y = y;
        q.v = &vert;
        (*m_maze)(x, y) = ACTIVE;
        m_queue.Enqueue(q);
    };

    (*m_maze)(i.x, i.y) = DEAD;
    enqueue(i.x - 1, i.y, L);
    enqueue(i.x + 1, i.y, R);
    enqueue(i.x, i.y - 1, B);
    enqueue(i.x, i.y + 1, T);
    return true;
}
