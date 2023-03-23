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
    m_start  = { maze->start.x, maze->start.y };
    m_end    = { maze->end  .x, maze->end  .y };
    m_active = { maze->start.x, maze->start.y };
    pathLength = 0;
    vertsExpanded = 0;
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
    pathLength = 0;
    int x = m_active.x, y = m_active.y;
    (*m_maze)(x, y) = t;
    while (y != m_start.y || x != m_start.x) {
        switch (m_vertices[y * m_maze->hcells + x].dir) {
            case L: x ++; break;
            case R: x --; break;
            case B: y ++; break;
            case T: y --; break;
            default: x = m_start.x, y = m_start.y; break;
        }
        pathLength ++;
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
    int x = m_start.x;
    int y = m_start.y;
    Sitem s = {x, y};
    m_stack.Push(s);
    (*m_maze)(x, y) = ACTIVE;
}

bool Solver::_stepDepthFirst()
{
    if (m_stack.IsEmpty())
        return false;

    auto i = m_stack.Pop();
    m_active = {i.x, i.y};
    vertsExpanded ++;
    if (_isEnd(i.x, i.y))
        return false;

    auto push = [this](int x, int y, unsigned char dir) {
        if (!m_maze->PointInBounds(x, y) || (*m_maze)(x, y) != PATH)
            return;
        m_vertices[y * m_maze->hcells + x].dir = dir;
        m_stack.Push({x, y});
        (*m_maze)(x, y) = ACTIVE;
    };

    (*m_maze)(i.x, i.y) = DEAD;
    push(i.x - 1, i.y, L);
    push(i.x + 1, i.y, R);
    push(i.x, i.y - 1, B);
    push(i.x, i.y + 1, T);
    return true;
}

////////////////////////////////
// Breadth First Search
////////////////////////////////

void Solver::_initBreadthFirst()
{
    int x = m_start.x;
    int y = m_start.y;
    Qitem q = {x, y, 0};
    m_queue.Enqueue(q);
    (*m_maze)(x, y) = ACTIVE;
}

bool Solver::_stepBreadthFirst()
{
    if (m_queue.IsEmpty())
        return false;

    auto i = m_queue.Dequeue();
    vertsExpanded ++;
    m_active = {i.x, i.y};
    if (_isEnd(i.x, i.y))
        return false;

    auto enqueue = [this](int x, int y, unsigned char dir) {
        if (!m_maze->PointInBounds(x, y) || (*m_maze)(x, y) != PATH)
            return;

        m_vertices[y * m_maze->hcells + x].dir = dir;
        Qitem q = {x, y, 0};
        m_queue.Enqueue(q);
        (*m_maze)(x, y) = ACTIVE;
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
    int x = m_start.x;
    int y = m_start.y;
    m_vertices[y * m_maze->hcells + x].gval = 0;
    (*m_maze)(x, y) = ACTIVE;

    Qitem q = {x, y, m_vertices};
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
    vertsExpanded ++;
    m_active = {i.x, i.y};
    if (_isEnd(i.x, i.y))
        return false;

    auto gval = i.v->gval + 1;
    auto enqueue = [this, gval](int x, int y, unsigned char dir) {
        if (!m_maze->PointInBounds(x, y) || (*m_maze)(x, y) == WALL)
            return;

        auto &vert = m_vertices[y * m_maze->hcells + x];
        if (vert.gval == -1 || vert.gval > gval) {
            vert.gval = gval;
            vert.dir  = dir;
        }

        if ((*m_maze)(x, y) == PATH) {
            (*m_maze)(x, y) = ACTIVE;
            Qitem q = {x, y, &vert};
            m_queue.Enqueue(q);
        }
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
    int x = m_start.x;
    int y = m_start.y;
    m_vertices[y * m_maze->hcells + x].hval = _heuristic(x, y, m_end.x, m_end.y);
    (*m_maze)(x, y) = ACTIVE;

    Qitem q = {x, y, m_vertices};
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
    vertsExpanded ++;
    m_active = {i.x, i.y};
    if (_isEnd(i.x, i.y))
        return false;

    auto enqueue = [this](int x, int y, unsigned char dir) {
        if (!m_maze->PointInBounds(x, y) || (*m_maze)(x, y) != PATH)
            return;

        auto i = y * m_maze->hcells + x;
        m_vertices[i].dir  = dir;
        m_vertices[i].hval = _heuristic(x, y, m_end.x, m_end.y);

        Qitem q = {x, y, &m_vertices[i]};
        m_queue.Enqueue(q);
        (*m_maze)(x, y) = ACTIVE;
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
    int x = m_start.x;
    int y = m_start.y;
    m_vertices[y * m_maze->hcells + x].gval = 0;
    m_vertices[y * m_maze->hcells + x].hval = _heuristic(x, y, m_end.x, m_end.y);
    (*m_maze)(x, y) = ACTIVE;

    Qitem q = {x, y, m_vertices};
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
    vertsExpanded ++;
    m_active = {i.x, i.y};
    if (_isEnd(i.x, i.y))
        return false;

    auto gval = i.v->gval + 1;
    auto enqueue = [this, gval](int x, int y, unsigned char dir) {
        if (!m_maze->PointInBounds(x, y) || (*m_maze)(x, y) == WALL)
            return;

        auto &vert = m_vertices[y * m_maze->hcells + x];
        if (vert.gval == -1 || vert.gval > gval) {
            vert.gval = gval;
            vert.dir  = dir;
        }

        if ((*m_maze)(x, y) == PATH) {
            vert.hval = _heuristic(x, y, m_end.x, m_end.y);
            Qitem q = {x, y, &vert};
            m_queue.Enqueue(q);
            (*m_maze)(x, y) = ACTIVE;
        }
    };

    (*m_maze)(i.x, i.y) = DEAD;
    enqueue(i.x - 1, i.y, L);
    enqueue(i.x + 1, i.y, R);
    enqueue(i.x, i.y - 1, B);
    enqueue(i.x, i.y + 1, T);
    return true;
}
