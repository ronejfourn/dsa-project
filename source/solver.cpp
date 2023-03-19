#include "rng.hpp"
#include "grid.hpp"
#include "stack.hpp"
#include "queue.hpp"
#include "solver.hpp"

SOLVER_INIT_FUNC(Solver::InitDFS)
{
    grid.ClearPaths();
    State s = {};
    stack.Push(s);
    grid(0, 0) = 0x33;
}

SOLVER_STEP_FUNC(Solver::StepDFS)
{
    auto &cstate = stack.Peek();
    auto x0 = cstate.dfs.x;
    auto y0 = cstate.dfs.y;

    if (cstate.dfs.finished || (x0 == grid.hcells - 1 && y0 == grid.vcells - 1)) {
        grid(x0, y0) = 0xff;
        stack.Pop();
        if (!stack.IsEmpty())
            stack.Peek().dfs.finished = true;
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
        switch (cstate.dfs.at) {
            case L: next = setnext(-1,  0); break;
            case R: next = setnext( 1,  0); break;
            case B: next = setnext( 0, -1); break;
            case T: next = setnext( 0,  1); break;
        }
        cstate.dfs.at ++;
    }

    if (next) {
        grid(nstate.dfs.x, nstate.dfs.y) = 0x33;
        stack.Push(nstate);
    } else {
        grid(x0, y0) = 0x77;
        stack.Pop();
    }
}

SOLVER_INIT_FUNC(Solver::InitBFS)
{
    grid.ClearPaths();
    State s = {};
    s.bfs.nitems = 1;
    s.bfs.verts = new unsigned char[grid.hcells * grid.vcells];
    stack.Push(s);

    grid(0, 0) = 0x20;

    Qitem q = {};
    q.bfs.x = 0;
    q.bfs.y = 0;
    queue.Enqueue(q);
}

SOLVER_STEP_FUNC(Solver::StepBFS)
{
    auto &p = stack.Peek();
    if ((p.bfs.s == 0 && p.bfs.nitems == 0) || (p.bfs.s == 1 && p.bfs.x == 0 && p.bfs.y == 0)) {
        delete[] p.bfs.verts;
        stack.Pop();
        queue.Clear();
        return;
    }

    if (p.bfs.s == 1) {
        switch (p.bfs.c) {
            case 0: p.bfs.x += 1; break;
            case 1: p.bfs.x -= 1; break;
            case 2: p.bfs.y += 1; break;
            case 3: p.bfs.y -= 1; break;
        }
        p.bfs.c = p.bfs.verts[p.bfs.y * grid.hcells + p.bfs.x];
        grid(p.bfs.x, p.bfs.y) = 0xff;
        return;
    }

    auto i = queue.Dequeue();
    p.bfs.nitems --;

    if (i.bfs.x == grid.hcells - 1 && i.bfs.y == grid.vcells - 1) {
        p.bfs.s = 1;
        p.bfs.x = grid.hcells - 1;
        p.bfs.y = grid.vcells - 1;
        p.bfs.c = grid(p.bfs.x, p.bfs.y) + 0x50;
        grid(p.bfs.x, p.bfs.y) = 0xff;
        return;
    }

    auto enqueue = [&queue, &grid, &p](int x, int y, unsigned c) -> bool {
        if (!grid.PointInBounds(x, y) || grid(x, y) != PATH)
            return false;
        p.bfs.verts[y * grid.hcells + x] = c;

        Qitem q = {};
        q.bfs.x = x;
        q.bfs.y = y;
        grid(x, y) = 0x20;
        queue.Enqueue(q);
        return true;
    };
    grid(i.bfs.x, i.bfs.y) += 0x50;
    p.bfs.nitems += enqueue(i.bfs.x - 1, i.bfs.y, 0);
    p.bfs.nitems += enqueue(i.bfs.x + 1, i.bfs.y, 1);
    p.bfs.nitems += enqueue(i.bfs.x, i.bfs.y - 1, 2);
    p.bfs.nitems += enqueue(i.bfs.x, i.bfs.y + 1, 3);
}
