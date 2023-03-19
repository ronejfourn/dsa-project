#include "rng.hpp"
#include "grid.hpp"
#include "stack.hpp"
#include "queue.hpp"
#include "solver.hpp"

#define ACTIVE  0xcc241d
#define FOUND   0x076678
#define DEAD    0xfabd2f

SOLVER_INIT_FUNC(Solver::InitDFS)
{
    State s = {};
    s.dfs.choice[0] = L;
    s.dfs.choice[1] = R;
    s.dfs.choice[2] = B;
    s.dfs.choice[3] = T;
    RNG::Shuffle(4, s.dfs.choice);
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

    while (!next && cstate.dfs.at < 4) {
        switch (cstate.dfs.choice[cstate.dfs.at]) {
            case L: next = setnext(-1,  0); break;
            case R: next = setnext( 1,  0); break;
            case B: next = setnext( 0, -1); break;
            case T: next = setnext( 0,  1); break;
        }
        cstate.dfs.at ++;
    }

    if (next) {
        grid(nstate.dfs.x, nstate.dfs.y) = ACTIVE;

        nstate.dfs.choice[0] = L;
        nstate.dfs.choice[1] = R;
        nstate.dfs.choice[2] = B;
        nstate.dfs.choice[3] = T;
        RNG::Shuffle(4, nstate.dfs.choice);
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

SOLVER_INIT_FUNC(Solver::InitDijkstra)
{
    State s = {};
    s.dij.vertices = new Vertex[grid.hcells * grid.vcells];
    s.dij.vertices[0].cost = 0;
    stack.Push(s);

    Qitem q = {};
    q.x = 0;
    q.y = 0;
    q.v = s.dij.vertices;
    queue.Enqueue(q);
    grid(0, 0) = ACTIVE;
    queue.SetCompareFunc([](auto a, auto b) -> bool {return a.v->cost < b.v->cost;});
}

SOLVER_STEP_FUNC(Solver::StepDijkstra)
{
    auto &p = stack.Peek();
    if ((!p.dij.found && queue.IsEmpty()) || (p.dij.found && (p.dij.x == 0 && p.dij.y == 0))) {
        delete[] p.dij.vertices;
        stack.Pop();
        queue.Clear();
        return;
    }

    if (p.dij.found == 1) {
        switch (p.dij.dir) {
            case 0: p.dij.x += 1; break;
            case 1: p.dij.x -= 1; break;
            case 2: p.dij.y += 1; break;
            case 3: p.dij.y -= 1; break;
        }
        p.dij.dir = p.dij.vertices[p.dij.y * grid.hcells + p.dij.x].dir;
        grid(p.dij.x, p.dij.y) = FOUND;
        return;
    }

    auto i = queue.PriorityDequeue();
    i.v->inQueue = false;

    if (i.x == grid.hcells - 1 && i.y == grid.vcells - 1) {
        p.dij.found = true;
        p.dij.x = grid.hcells - 1;
        p.dij.y = grid.vcells - 1;
        p.dij.dir = p.dij.vertices[p.dij.y * grid.hcells + p.dij.x].dir;
        grid(i.x, i.y) = FOUND;
        return;
    }

    auto enqueue = [&queue, &grid, &p, &i](int x, int y, unsigned c) {
        if (!grid.PointInBounds(x, y) || grid(x, y) == WALL)
            return;

        auto &ref = p.dij.vertices[y * grid.hcells + x];
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

SOLVER_INIT_FUNC(Solver::InitAStar)
{
    State s = {};
    s.astr.vertices = new Vertex[grid.hcells * grid.vcells];
    s.astr.vertices[0].cost = 0;
    s.astr.vertices[0].heuristic = grid.hcells + grid.vcells - 2;
    stack.Push(s);

    Qitem q = {};
    q.x = 0;
    q.y = 0;
    q.v = s.astr.vertices;
    q.v->inQueue = true;
    queue.Enqueue(q);
    grid(0, 0) = ACTIVE;
    queue.SetCompareFunc([](auto a, auto b) -> bool {return (a.v->cost + a.v->heuristic) < (b.v->cost + b.v->heuristic);});
}

template <typename T>
inline T abs(T a) {return a < 0 ? -a : a;}

SOLVER_STEP_FUNC(Solver::StepAStar)
{
    auto &p = stack.Peek();
    if ((!p.astr.found && queue.IsEmpty()) || (p.astr.found && (p.astr.x == 0 && p.astr.y == 0))) {
        delete[] p.astr.vertices;
        stack.Pop();
        queue.Clear();
        return;
    }

    if (p.astr.found) {
        switch (p.astr.dir) {
            case 0: p.astr.x += 1; break;
            case 1: p.astr.x -= 1; break;
            case 2: p.astr.y += 1; break;
            case 3: p.astr.y -= 1; break;
        }
        p.astr.dir = p.astr.vertices[p.astr.y * grid.hcells + p.astr.x].dir;
        grid(p.astr.x, p.astr.y) = FOUND;
        return;
    }

    auto i = queue.PriorityDequeue();
    i.v->inQueue = false;

    if (i.x == grid.hcells - 1 && i.y == grid.vcells - 1) {
        p.astr.found = true;
        p.astr.x = grid.hcells - 1;
        p.astr.y = grid.vcells - 1;
        p.astr.dir = p.astr.vertices[p.astr.y * grid.hcells + p.astr.x].dir;
        grid(i.x, i.y) = FOUND;
        return;
    }

    auto enqueue = [&queue, &grid, &p, &i](int x, int y, unsigned c) {
        if (!grid.PointInBounds(x, y) || grid(x, y) == WALL)
            return;

        auto &ref = p.astr.vertices[y * grid.hcells + x];
        if (ref.cost != -1 && ref.cost <= i.v->cost + 1)
            return;

        ref.cost = i.v->cost + 1;
        ref.dir  = c;
        ref.heuristic = abs(grid.hcells - 1 - x) + abs(grid.vcells - 1 - y);

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
