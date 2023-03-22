#include "generator.hpp"
#include "rng.hpp"
#include "maze.hpp"

Generator:: Generator() {}
Generator::~Generator() {_reset();}

void Generator::Init(Maze *maze, Type type)
{
    _reset();
    m_maze = maze;

#define CASE(_NAME) case Type::_NAME : _init##_NAME(); _step = &Generator::_step##_NAME; break
    switch (type) {
        CASE(Random);
        CASE(RandomizedDFS);
        CASE(RecursiveDivision);
        CASE(RandomizedKruskal);
        CASE(RandomizedPrim);
    };
#undef CASE
}

bool Generator::Step()
{
    if ((this->*_step)()) {
        return true;
    } else {
        _reset();
        return false;
    }
}

void Generator::_reset()
{
    m_maze = nullptr;
    m_finished = false;

    delete [] m_graph.edges;
    delete [] m_graph.verts;
    m_graph = {};

    m_stack.Clear();
}

////////////////////////////////
// Random
////////////////////////////////

void Generator::_initRandom()
{
}

bool Generator::_stepRandom()
{
    unsigned m = m_maze->hcells * m_maze->vcells - 1;
    for (unsigned i = 0; i <= m; i ++)
        m_maze->cells[i] = (RNG::Get() & 3) == 0 ? WALL : PATH;

    m_maze->cells[0] = PATH;
    m_maze->cells[m] = PATH;
    return false;
}

////////////////////////////////
// Randomized DFS
////////////////////////////////

void Generator::_initRandomizedDFS()
{
    m_maze->Fill(WALL);

    Sitem s = {};
    s.dfs.choice[0] = L;
    s.dfs.choice[1] = R;
    s.dfs.choice[2] = B;
    s.dfs.choice[3] = T;
    RNG::Shuffle(4, s.dfs.choice);
    m_stack.Push(s);
}

bool Generator::_stepRandomizedDFS()
{
    if (m_stack.IsEmpty())
        return false;

    Sitem &cstate = m_stack.Peek();
    int x0 = cstate.dfs.x;
    int y0 = cstate.dfs.y;

    bool  next = false;
    Sitem nstate = {};
    (*m_maze)(x0, y0) = PATH;

    auto setnext = [this, x0, y0, &nstate](int dx, int dy) -> bool {
        int x = x0 + dx, y = y0 + dy;
        if (!m_maze->PointInBounds(x, y) || (*m_maze)(x, y) == PATH)
            return false;

        nstate.dfs.x = x;
        nstate.dfs.y = y;
        (*m_maze)(x0 + dx / 2, y0 + dy / 2) = PATH;
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
        m_stack.Push(nstate);
    } else {
        m_stack.Pop();
    }

    return true;
}

////////////////////////////////
// Recursive Division
////////////////////////////////

void Generator::_initRecursiveDivision()
{
    m_maze->Fill(PATH);

    Sitem s = {};
    s.div.w = m_maze->hcells - 1;
    s.div.h = m_maze->vcells - 1;
    s.div.vertical = RNG::Get() & 1;
    m_stack.Push(s);
}

bool Generator::_stepRecursiveDivision()
{
    if (m_stack.IsEmpty())
        return false;

    Sitem cstate = m_stack.Pop();
    int  x = cstate.div.x;
    int  y = cstate.div.y;
    int  w = cstate.div.w;
    int  h = cstate.div.h;
    bool v = cstate.div.vertical;

    int wmid = RNG::Get(x, w - 1) | 1;
    int hmid = RNG::Get(y, h - 1) | 1;

    auto next = [this](int x, int w, int y, int h) -> void {
        if (w - x <= 1 || h - y <= 1)
            return;

        Sitem n;
        n.div.x = x; n.div.y = y;
        n.div.w = w; n.div.h = h;
        n.div.vertical = w - x > h - y;
        m_stack.Push(n);
    };

    if (v) {
        for (int i = y; i <= h; i ++)
            (*m_maze)(wmid, i) = WALL;

        next(x, wmid - 1, y, h);
        next(wmid + 1, w, y, h);
        auto th = RNG::Get() % ((m_maze->vcells - 1) / (h - y));

        if (h - y > 5 && th == 0) {
            (*m_maze)(wmid, RNG::Get(y, y + (h - y) / 2) & (~1)) = PATH;
            (*m_maze)(wmid, RNG::Get(y + (h - y) / 2, h) & (~1)) = PATH;
        } else {
            (*m_maze)(wmid, RNG::Get(y, h) & (~1)) = PATH;
        }
    } else {
        for (int i = x; i <= w; i ++)
            (*m_maze)(i, hmid) = WALL;

        next(x, w, y, hmid - 1);
        next(x, w, hmid + 1, h);
        auto th = RNG::Get() % ((m_maze->hcells - 1) / (w - x));

        if (w - x > 5 && th == 0) {
            (*m_maze)(RNG::Get(x, x + (w - x) / 2) & (~1), hmid) = PATH;
            (*m_maze)(RNG::Get(x + (w - x) / 2, w) & (~1), hmid) = PATH;
        } else {
            (*m_maze)(RNG::Get(x, w) & (~1), hmid) = PATH;
        }
    }

    return true;
}

////////////////////////////////
// Randomized Kruskal's
////////////////////////////////

void Generator::_initRandomizedKruskal()
{
    m_maze->Fill(WALL);
    auto hhcells = m_maze->hcells >> 1;
    auto hvcells = m_maze->vcells >> 1;
    m_graph.nverts  = ((hhcells + 1) * (hvcells + 1));
    m_graph.nedges  = (hvcells * hhcells * 2) + hhcells + hvcells;

    m_graph.at = 0;
    m_graph.edges = new Edge[m_graph.nedges];
    m_graph.verts = new unsigned[m_graph.nedges];

    unsigned i = 0;
    for (int x = 0; x < (int)m_maze->hcells - 2; x += 2)
        for (int y = 0; y < (int)m_maze->vcells; y += 2)
            m_graph.edges[i++] = {x, y, x + 2, y};

    for (int y = 0; y < (int)m_maze->vcells - 2; y += 2)
        for (int x = 0; x < (int)m_maze->hcells; x += 2)
            m_graph.edges[i++] = {x, y, x, y + 2};

    for (unsigned i = 0; i < m_graph.nverts; i ++)
        m_graph.verts[i] = i;

    RNG::Shuffle(m_graph.nedges, m_graph.edges);
}

bool Generator::_stepRandomizedKruskal()
{
    if (m_graph.at >= m_graph.nedges)
        return false;

    auto hhcells = m_maze->hcells >> 1;
    auto e  = m_graph.edges[m_graph.at ++];
    auto v0 = (e.y0 / 2) * (hhcells + 1) + (e.x0 / 2);
    auto v1 = (e.y1 / 2) * (hhcells + 1) + (e.x1 / 2);
    auto f0 = m_graph.verts[v0];
    auto f1 = m_graph.verts[v1];

    while (f0 == f1) {
        if (m_graph.at >= m_graph.nedges)
            return false;

        e  = m_graph.edges[m_graph.at ++];
        v0 = (e.y0 / 2) * (hhcells + 1) + (e.x0 / 2);
        v1 = (e.y1 / 2) * (hhcells + 1) + (e.x1 / 2);
        f0 = m_graph.verts[v0];
        f1 = m_graph.verts[v1];
    }

    for (unsigned i = 0; i < m_graph.nverts; i++)
        if (m_graph.verts[i] == f1)
            m_graph.verts[i] = f0;

    (*m_maze)(e.x0, e.y0) = PATH;
    (*m_maze)(e.x1, e.y1) = PATH;
    (*m_maze)(e.x0 + (e.x1 - e.x0) / 2, e.y0 + (e.y1 - e.y0) / 2) = PATH;
    return true;
}

////////////////////////////////
// Randomized Prim's
////////////////////////////////

void Generator::_initRandomizedPrim()
{
    m_maze->Fill(WALL);

    auto hhcells = m_maze->hcells >> 1;
    auto hvcells = m_maze->vcells >> 1;
    m_graph.edges = new Edge[hvcells * hhcells * 2 + hhcells + hvcells];

    (*m_maze)(0, 0) = PATH;
    m_graph.at = 2;
    m_graph.edges[0] = {0, 0, 2, 0};
    m_graph.edges[1] = {0, 0, 0, 2};
}

bool Generator::_stepRandomizedPrim()
{
    if (m_graph.at == 0)
        return false;

    auto i = RNG::Get() % (m_graph.at --);
    auto e = m_graph.edges[i];
    m_graph.edges[i] = m_graph.edges[m_graph.at];

    while ((*m_maze)(e.x1, e.y1) == PATH) {
        if (m_graph.at == 0)
            return false;

        i = RNG::Get() % (m_graph.at --);
        e = m_graph.edges[i];
        m_graph.edges[i] = m_graph.edges[m_graph.at];
    }

    auto xm = e.x0 + (e.x1 - e.x0) / 2, ym = e.y0 + (e.y1 - e.y0) / 2;
    (*m_maze)(e.x1, e.y1) = PATH;
    (*m_maze)(  xm,   ym) = PATH;

    auto addEdge = [this, &e](int dx, int dy) {
        auto x2 = e.x1 + dx;
        auto y2 = e.y1 + dy;

        if (m_maze->PointInBounds(x2, y2)) {
            Edge n = { e.x1, e.y1, x2, y2 };
            m_graph.edges[m_graph.at ++] = n;
        }
    };

    addEdge( 0,  2);
    addEdge( 0, -2);
    addEdge( 2,  0);
    addEdge(-2,  0);
    return true;
}
