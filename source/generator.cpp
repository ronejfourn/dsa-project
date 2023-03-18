#include "rng.hpp"
#include "grid.hpp"
#include "stack.hpp"
#include "generator.hpp"

GENERATOR_INIT_FUNC(Generator::InitRandomizedDFS)
{
    grid.Fill(WALL);
    State s = {};
    s.dfs.choice[0] = L;
    s.dfs.choice[1] = R;
    s.dfs.choice[2] = B;
    s.dfs.choice[3] = T;
    RNG::Shuffle(4, s.dfs.choice);

    stack.Push(s);
}

GENERATOR_STEP_FUNC(Generator::StepRandomizedDFS)
{
    State &cstate = stack.Peek();
    int x0 = cstate.dfs.x;
    int y0 = cstate.dfs.y;

    bool next = false;
    State nstate = {};
    grid(x0, y0) = PATH;

    auto setnext = [x0, y0, &grid, &nstate](int dx, int dy) -> bool {
        int x = x0 + dx, y = y0 + dy;
        if (!grid.PointInBounds(x, y) || grid(x, y) == PATH)
            return false;
        nstate.dfs.x = x;
        nstate.dfs.y = y;
        grid(x0 + dx / 2, y0 + dy / 2) = PATH;
        return true;
    };

    while (!next && cstate.dfs.at < 4) {
        switch (cstate.dfs.choice[cstate.dfs.at]) {
            case L: next = setnext(-2,  0); break;
            case R: next = setnext( 2,  0); break;
            case B: next = setnext( 0, -2); break;
            case T: next = setnext( 0,  2); break;
        }
        cstate.dfs.at ++;
    }

    if (next) {
        nstate.dfs.at = 0;
        nstate.dfs.choice[0] = L;
        nstate.dfs.choice[1] = R;
        nstate.dfs.choice[2] = B;
        nstate.dfs.choice[3] = T;
        RNG::Shuffle(4, nstate.dfs.choice);
        stack.Push(nstate);
    } else {
        stack.Pop();
    }
}

GENERATOR_INIT_FUNC(Generator::InitRecursiveDivision)
{
    grid.Fill(PATH);
    State s;
    s.div.x = 0;
    s.div.y = 0;
    s.div.w = grid.hcells - 1;
    s.div.h = grid.vcells - 1;
    s.div.vertical = RNG::Get() & 1;
    stack.Push(s);
}

GENERATOR_STEP_FUNC(Generator::StepRecursiveDivision)
{
    State popped = stack.Pop();
    auto &cstate = popped.div;
    int x = cstate.x;
    int y = cstate.y;
    int w = cstate.w;
    int h = cstate.h;
    bool v = cstate.vertical;

    if (w - x <= 1 || h - y <= 1)
        return;

    int m = RNG::Get(x, w - 1) | 1;
    int p = RNG::Get(y, h - 1) | 1;

    auto next = [&stack](int x, int w, int y, int h) -> void {
        State n;
        n.div.x = x; n.div.y = y;
        n.div.w = w; n.div.h = h;
        n.div.vertical = w - x > h - y;
        stack.Push(n);
    };

    if (v) {
        for (int i = y; i <= h; i ++)
            grid(m, i) = WALL;
        next(x, m - 1, y, h);
        next(m + 1, w, y, h);

        if (h - y > 5) {
            grid(m, RNG::Get(y, y + (h - y) / 2) & (~1)) = PATH;
            grid(m, RNG::Get(y + (h - y) / 2, h) & (~1)) = PATH;
        } else {
            grid(m, RNG::Get(y, h) & (~1)) = PATH;
        }
    } else {
        for (int i = x; i <= w; i ++)
            grid(i, p) = WALL;
        next(x, w, y, p - 1);
        next(x, w, p + 1, h);

        if (w - x > 5) {
            grid(RNG::Get(x, x + (w - x) / 2) & (~1), p) = PATH;
            grid(RNG::Get(x + (w - x) / 2, w) & (~1), p) = PATH;
        } else {
            grid(RNG::Get(x, w) & (~1), p) = PATH;
        }
    }
}

GENERATOR_INIT_FUNC(Generator::InitRandomizedKruskal)
{
    State s;
    Edge e;
    grid.Fill(WALL);

    s.krs.hhcells = grid.hcells >> 1;
    s.krs.hvcells = grid.vcells >> 1;
    s.krs.nverts  = ((s.krs.hhcells + 1) * (s.krs.hvcells + 1));
    s.krs.nedges  = (s.krs.hvcells * s.krs.hhcells * 2) + s.krs.hhcells + s.krs.hvcells;

    s.krs.at = 0;
    s.krs.edges = new Edge[s.krs.nedges];
    s.krs.vertForest = new unsigned[s.krs.nverts];

    unsigned i = 0;
    for (int x = 0; x < grid.hcells - 2; x += 2) {
        e.x0 = x, e.x1 = x + 2;
        for (int y = 0; y < grid.vcells; y += 2) {
            e.y0 = e.y1 = y;
            s.krs.edges[i++] = e;
        }
    }

    for (int y = 0; y < grid.vcells - 2; y += 2) {
        e.y0 = y, e.y1 = y + 2;
        for (int x = 0; x < grid.hcells; x += 2) {
            e.x0 = e.x1 = x;
            s.krs.edges[i++] = e;
        }
    }

    for (unsigned i = 0; i < s.krs.nverts; i ++)
        s.krs.vertForest[i] = i;

    RNG::Shuffle(s.krs.nedges, s.krs.edges);
    stack.Push(s);
}

GENERATOR_STEP_FUNC(Generator::StepRandomizedKruskal)
{
    auto &s = stack.Peek();
    if (s.krs.at >= s.krs.nedges) {
        delete [] s.krs.edges;
        delete [] s.krs.vertForest;
        stack.Pop();
        return;
    }

    auto e  = s.krs.edges[s.krs.at ++];
    auto v0 = (e.y0 / 2) * (s.krs.hhcells + 1) + (e.x0 / 2);
    auto v1 = (e.y1 / 2) * (s.krs.hhcells + 1) + (e.x1 / 2);
    auto f0 = s.krs.vertForest[v0];
    auto f1 = s.krs.vertForest[v1];

    if (f0 != f1) {
        for (unsigned i = 0; i < s.krs.nverts; i++)
            if (s.krs.vertForest[i] == f1)
                s.krs.vertForest[i] = f0;

        grid(e.x0, e.y0) = PATH;
        grid(e.x1, e.y1) = PATH;
        grid(e.x0 + (e.x1 - e.x0) / 2, e.y0 + (e.y1 - e.y0) / 2) = PATH;
    }
}

GENERATOR_INIT_FUNC(Generator::InitRandomizedPrim)
{
    State s;
    Edge e;
    grid.Fill(WALL);

    auto hhcells = grid.hcells >> 1;
    auto hvcells = grid.vcells >> 1;
    s.prm.availableEdges = 0;
    s.prm.edges = new Edge[hvcells * hhcells * 2 + hhcells + hvcells];

    grid(0, 0) = PATH;
    s.prm.availableEdges = 2;
    s.prm.edges[0] = {0, 0, 2, 0};
    s.prm.edges[1] = {0, 0, 0, 2};

    stack.Push(s);
}

GENERATOR_STEP_FUNC(Generator::StepRandomizedPrim)
{
    auto &s = stack.Peek();
    if (s.prm.availableEdges == 0) {
        delete [] s.prm.edges;
        stack.Pop();
        return;
    }

    auto i = RNG::Get() % (s.prm.availableEdges --);
    auto e = s.prm.edges[i];
    s.prm.edges[i] = s.prm.edges[s.prm.availableEdges];

    if (grid(e.x1, e.y1) == PATH)
        return;

    auto xm = e.x0 + (e.x1 - e.x0) / 2, ym = e.y0 + (e.y1 - e.y0) / 2;

    grid(e.x1, e.y1) = PATH;
    grid(xm, ym) = PATH;

    auto addEdge = [&grid, &s, &e](int dx, int dy) {
        auto x2 = e.x1 + dx;
        auto y2 = e.y1 + dy;
        if (x2 < grid.hcells && x2 >= 0 && y2 < grid.vcells && y2 >= 0) {
            Edge n = { e.x1, e.y1, x2, y2 };
            s.prm.edges[s.prm.availableEdges ++] = n;
        }
    };

    addEdge( 0,  2);
    addEdge( 0, -2);
    addEdge( 2,  0);
    addEdge(-2,  0);
}
