#include "rng.hpp"
#include "grid.hpp"
#include "stack.hpp"
#include "queue.hpp"
#include "solver.hpp"
#include <math.h>

#define ACTIVE  0xcc241d
#define FOUND   0x076678
#define DEAD    0xfabd2f

SOLVER_INIT_FUNC(Solver::InitDFS)
{
    State s = {};
    s.dfs.dir = 0;
    stack.Push(s);
    grid(0, 0) = ACTIVE;
}

SOLVER_STEP_FUNC(Solver::StepDFS)
{
    auto &cstate = stack.Peek();
    auto x0 = cstate.dfs.x;
    auto y0 = cstate.dfs.y;

    if (cstate.dfs.found || (x0 == grid.hcells - 1 && y0 == grid.vcells - 1)) {
        grid(x0, y0) = FOUND;
        stack.Pop();
        if (!stack.IsEmpty())
            stack.Peek().dfs.found = true;
        return;
    }

    bool next = false;
    State nstate = {};

    auto setnext = [x0, y0, &grid, &nstate](int dx, int dy) -> bool {
        auto x = x0 + dx, y = y0 + dy;
        if (grid.PointInBounds(x, y) && grid(x, y) == PATH) {
            nstate.dfs.x = x;
            nstate.dfs.y = y;
            return true;
        }
        return false;
    };

    while (!next && cstate.dfs.dir < 4) {
        switch (cstate.dfs.dir) {
            case L: next = setnext(-1,  0); break;
            case R: next = setnext( 1,  0); break;
            case B: next = setnext( 0, -1); break;
            case T: next = setnext( 0,  1); break;
        }
        cstate.dfs.dir ++;
    }

    if (next) {
        grid(nstate.dfs.x, nstate.dfs.y) = ACTIVE;
        stack.Push(nstate);
    } else {
        grid(x0, y0) = DEAD;
        stack.Pop();
    }
}

SOLVER_INIT_FUNC(Solver::InitBFS)
{
    State s = {};
    s.bfs.vertices = new Vertex[grid.hcells * grid.vcells];
    stack.Push(s);

    grid(0, 0) = ACTIVE;

    Qitem q = {};
    q.x = 0;
    q.y = 0;
    queue.Enqueue(q);
}

SOLVER_STEP_FUNC(Solver::StepBFS)
{
    auto &p = stack.Peek();
    if ((!p.bfs.found && queue.IsEmpty()) || (p.bfs.found && p.bfs.x == 0 && p.bfs.y == 0)) {
        delete[] p.bfs.vertices;
        stack.Pop();
        queue.Clear();
        return;
    }

    if (p.bfs.found) {
        switch (p.bfs.dir) {
            case 0: p.bfs.x += 1; break;
            case 1: p.bfs.x -= 1; break;
            case 2: p.bfs.y += 1; break;
            case 3: p.bfs.y -= 1; break;
        }
        p.bfs.dir = p.bfs.vertices[p.bfs.y * grid.hcells + p.bfs.x].dir;
        grid(p.bfs.x, p.bfs.y) = FOUND;
        return;
    }

    auto i = queue.Dequeue();

    if (i.x == grid.hcells - 1 && i.y == grid.vcells - 1) {
        p.bfs.found = true;
        p.bfs.x = grid.hcells - 1;
        p.bfs.y = grid.vcells - 1;
        p.bfs.dir = grid(p.bfs.x, p.bfs.y) + 0x50;
        grid(p.bfs.x, p.bfs.y) = FOUND;
        return;
    }

    auto enqueue = [&queue, &grid, &p](int x, int y, unsigned c) {
        if (!grid.PointInBounds(x, y) || grid(x, y) != PATH)
            return;
        p.bfs.vertices[y * grid.hcells + x].dir = c;

        Qitem q = {};
        q.x = x;
        q.y = y;
        grid(x, y) = ACTIVE;
        queue.Enqueue(q);
        return;
    };

    grid(i.x, i.y) = DEAD;
    enqueue(i.x - 1, i.y, 0);
    enqueue(i.x + 1, i.y, 1);
    enqueue(i.x, i.y - 1, 2);
    enqueue(i.x, i.y + 1, 3);
}

static void _InitGreedy(Grid &grid, Queue<Solver::Qitem> &queue, Stack<Solver::State> &stack, Solver::Heuristic hf)
{
    using namespace Solver;
    State s = {};
    s.greedy.heuristic = hf;
    s.greedy.vertices = new Vertex[grid.hcells * grid.vcells];
    s.greedy.vertices[0].hval = hf(grid, 0, 0);
    stack.Push(s);

    grid(0, 0) = ACTIVE;

    Qitem q = {};
    q.x = 0;
    q.y = 0;
    q.v = s.greedy.vertices;
    queue.Enqueue(q);
    queue.SetCompareFunc([](auto a, auto b) -> bool {return a.v->hval < b.v->hval;});
}

SOLVER_INIT_FUNC(Solver::InitGreedyEuclidean)
{
    _InitGreedy(grid, queue, stack, [](const Grid &grid, int x, int y) -> int {
        return (grid.hcells - x - 1) * (grid.hcells - x - 1) + (grid.vcells - y - 1) * (grid.vcells - y - 1);
    });
}

SOLVER_INIT_FUNC(Solver::InitGreedyManhattan)
{
    _InitGreedy(grid, queue, stack, [](const Grid &grid, int x, int y) -> int {
        return grid.hcells - x + grid.vcells - y - 2;
    });
}

SOLVER_STEP_FUNC(Solver::StepGreedy)
{
    auto &p = stack.Peek();
    if ((!p.greedy.found && queue.IsEmpty()) || (p.greedy.found && p.greedy.x == 0 && p.greedy.y == 0)) {
        delete[] p.greedy.vertices;
        stack.Pop();
        queue.Clear();
        return;
    }

    if (p.greedy.found) {
        switch (p.greedy.dir) {
            case 0: p.greedy.x += 1; break;
            case 1: p.greedy.x -= 1; break;
            case 2: p.greedy.y += 1; break;
            case 3: p.greedy.y -= 1; break;
        }
        p.greedy.dir = p.greedy.vertices[p.greedy.y * grid.hcells + p.greedy.x].dir;
        grid(p.greedy.x, p.greedy.y) = FOUND;
        return;
    }

    auto i = queue.PriorityDequeue();

    if (i.x == grid.hcells - 1 && i.y == grid.vcells - 1) {
        p.greedy.found = true;
        p.greedy.x = grid.hcells - 1;
        p.greedy.y = grid.vcells - 1;
        p.greedy.dir = grid(p.greedy.x, p.greedy.y) + 0x50;
        grid(p.greedy.x, p.greedy.y) = FOUND;
        return;
    }

    auto enqueue = [&queue, &grid, &p](int x, int y, unsigned c) {
        if (!grid.PointInBounds(x, y) || grid(x, y) != PATH)
            return;

        auto &ref = p.greedy.vertices[y * grid.hcells + x];
        ref.dir  = c;
        ref.hval = p.greedy.heuristic(grid, x, y);

        Qitem q = {};
        q.x = x;
        q.y = y;
        q.v = &ref;
        grid(x, y) = ACTIVE;
        queue.Enqueue(q);
        return;
    };

    grid(i.x, i.y) = DEAD;
    enqueue(i.x - 1, i.y, 0);
    enqueue(i.x + 1, i.y, 1);
    enqueue(i.x, i.y - 1, 2);
    enqueue(i.x, i.y + 1, 3);
}

SOLVER_INIT_FUNC(Solver::InitDijkstra)
{
    State s = {};
    s.dijk.vertices = new Vertex[grid.hcells * grid.vcells];
    s.dijk.vertices[0].cost = 0;
    stack.Push(s);

    Qitem q = {};
    q.x = 0;
    q.y = 0;
    q.v = s.dijk.vertices;
    queue.Enqueue(q);
    grid(0, 0) = ACTIVE;
    queue.SetCompareFunc([](auto a, auto b) -> bool {return a.v->cost < b.v->cost;});
}

SOLVER_STEP_FUNC(Solver::StepDijkstra)
{
    auto &p = stack.Peek();
    if ((!p.dijk.found && queue.IsEmpty()) || (p.dijk.found && (p.dijk.x == 0 && p.dijk.y == 0))) {
        delete[] p.dijk.vertices;
        stack.Pop();
        queue.Clear();
        return;
    }

    if (p.dijk.found == 1) {
        switch (p.dijk.dir) {
            case 0: p.dijk.x += 1; break;
            case 1: p.dijk.x -= 1; break;
            case 2: p.dijk.y += 1; break;
            case 3: p.dijk.y -= 1; break;
        }
        p.dijk.dir = p.dijk.vertices[p.dijk.y * grid.hcells + p.dijk.x].dir;
        grid(p.dijk.x, p.dijk.y) = FOUND;
        return;
    }

    auto i = queue.PriorityDequeue();
    i.v->inQueue = false;

    if (i.x == grid.hcells - 1 && i.y == grid.vcells - 1) {
        p.dijk.found = true;
        p.dijk.x = grid.hcells - 1;
        p.dijk.y = grid.vcells - 1;
        p.dijk.dir = p.dijk.vertices[p.dijk.y * grid.hcells + p.dijk.x].dir;
        grid(i.x, i.y) = FOUND;
        return;
    }

    auto enqueue = [&queue, &grid, &p, &i](int x, int y, unsigned c) {
        if (!grid.PointInBounds(x, y) || grid(x, y) == WALL)
            return;

        auto &ref = p.dijk.vertices[y * grid.hcells + x];
        if (ref.cost != -1 && ref.cost <= i.v->cost + 1)
            return;

        ref.cost = i.v->cost + 1;
        ref.dir  = c;

        if (!ref.inQueue) {
            ref.inQueue = true;
            Qitem q = {};
            q.x = x;
            q.y = y;
            q.v = &ref;
            grid(q.x, q.y) = ACTIVE;
            queue.Enqueue(q);
        }
    };

    grid(i.x, i.y) = DEAD;
    enqueue(i.x - 1, i.y, 0);
    enqueue(i.x + 1, i.y, 1);
    enqueue(i.x, i.y - 1, 2);
    enqueue(i.x, i.y + 1, 3);
}

static void _InitAStar(Grid &grid, Queue<Solver::Qitem> &queue, Stack<Solver::State> &stack, Solver::Heuristic hf)
{
    using namespace Solver;
    State s = {};
    s.astar.heuristic = hf;
    s.astar.vertices = new Vertex[grid.hcells * grid.vcells];
    s.astar.vertices[0].cost = 0;
    s.astar.vertices[0].hval = hf(grid, 0, 0);
    stack.Push(s);

    Qitem q = {};
    q.x = 0;
    q.y = 0;
    q.v = s.astar.vertices;
    q.v->inQueue = true;
    queue.Enqueue(q);
    grid(0, 0) = ACTIVE;
    queue.SetCompareFunc([](auto a, auto b) -> bool {return (a.v->cost + a.v->hval) < (b.v->cost + b.v->hval);});
}

SOLVER_INIT_FUNC(Solver::InitAStarEuclidean)
{
    _InitAStar(grid, queue, stack, [](const Grid &grid, int x, int y) -> int {
        return sqrt((grid.hcells - x - 1) * (grid.hcells - x - 1) + (grid.vcells - y - 1) * (grid.vcells - y - 1));
    });
}

SOLVER_INIT_FUNC(Solver::InitAStarManhattan)
{
    _InitAStar(grid, queue, stack, [](const Grid &grid, int x, int y) -> int {
        return (grid.hcells - x - 1) + (grid.vcells - y - 1);
    });
}

SOLVER_STEP_FUNC(Solver::StepAStar)
{
    auto &p = stack.Peek();
    if ((!p.astar.found && queue.IsEmpty()) || (p.astar.found && (p.astar.x == 0 && p.astar.y == 0))) {
        delete[] p.astar.vertices;
        stack.Pop();
        queue.Clear();
        return;
    }

    if (p.astar.found) {
        switch (p.astar.dir) {
            case 0: p.astar.x += 1; break;
            case 1: p.astar.x -= 1; break;
            case 2: p.astar.y += 1; break;
            case 3: p.astar.y -= 1; break;
        }
        p.astar.dir = p.astar.vertices[p.astar.y * grid.hcells + p.astar.x].dir;
        grid(p.astar.x, p.astar.y) = FOUND;
        p.astar.total ++;
        return;
    }

    auto i = queue.PriorityDequeue();
    i.v->inQueue = false;

    if (i.x == grid.hcells - 1 && i.y == grid.vcells - 1) {
        p.astar.found = true;
        p.astar.total = 0;
        p.astar.x = grid.hcells - 1;
        p.astar.y = grid.vcells - 1;
        p.astar.dir = p.astar.vertices[p.astar.y * grid.hcells + p.astar.x].dir;
        grid(i.x, i.y) = FOUND;
        return;
    }

    auto enqueue = [&queue, &grid, &p, &i](int x, int y, unsigned c) {
        if (!grid.PointInBounds(x, y) || grid(x, y) == WALL)
            return;

        auto &ref = p.astar.vertices[y * grid.hcells + x];
        if (ref.cost != -1 && ref.cost <= i.v->cost + 1)
            return;

        ref.cost = i.v->cost + 1;
        ref.dir  = c;
        ref.hval = p.astar.heuristic(grid, x, y);

        if (!ref.inQueue) {
            ref.inQueue = true;
            Qitem q = {};
            q.x = x;
            q.y = y;
            q.v = &ref;
            grid(q.x, q.y) = ACTIVE;
            queue.Enqueue(q);
        }
    };

    grid(i.x, i.y) = DEAD;
    enqueue(i.x - 1, i.y, 0);
    enqueue(i.x + 1, i.y, 1);
    enqueue(i.x, i.y - 1, 2);
    enqueue(i.x, i.y + 1, 3);
}
