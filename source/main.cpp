#include "maze.hpp"
#include "solver.hpp"
#include "generator.hpp"
#include "application.hpp"
#include <chrono>

class MazeApp : public BaseApplication
{
public:
    MazeApp()
    {
        m_wConfig.title  = "Maze Generators & Solvers";
        m_wConfig.width  = 1280;
        m_wConfig.height = 720;
    }

private:
    struct State {
        enum Enum {
            Generating,
            Solving,
            Idle,
        } state = Idle;

        int  scale     = 5;
        bool animate   = true;
        int  heuristic = 0;

        float time = 0;
        float step = 1;
        std::chrono::high_resolution_clock clock;
        std::chrono::high_resolution_clock::time_point then;
    } m_state;

    Generator m_generator;
    Solver m_solver;

    static constexpr unsigned DEF_W = 125;
    static constexpr unsigned DEF_H = 125;

    Maze m_maze = Maze(DEF_W, DEF_H);
    SDL_Texture *m_mazeTexture = nullptr;

    bool OnInit() override
    {
        m_mazeTexture = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, m_maze.hcells, m_maze.vcells);
        bool ok = m_mazeTexture != NULL;
        if (ok) RenderMaze();

        return ok;
    }

    void OnUpdate() override
    {
        ImGui::ShowDemoWindow();
        ImGui::Begin("Maze");

        {
            ImGui::Checkbox("Animate", &m_state.animate);
            if (m_state.animate)
                ImGui::SliderFloat("Time Per Step(ms)", &m_state.step, 1, 1000);

            if (!m_state.animate && m_state.state != State::Idle) {
                if (m_state.state == State::Generating)
                    while (m_generator.Step());
                else
                    while (m_solver.Step());

                SwitchState(State::Idle);
                RenderMaze();
            }
        }

        ImGui::NewLine();

        if (ImGui::CollapsingHeader("Generators"))
        {
            static auto GeneratorItem = [this](const char *n, Generator::Type type) {
                bool btn = ImGui::Button(n, ImVec2(ImGui::GetContentRegionAvail().x, 0));
                if (!btn || m_state.state != State::Idle)
                    return;
                m_generator.Init(&m_maze, type);
                SwitchState(State::Generating);
            };

            GeneratorItem("Random"            , Generator::Type::Random           );
            GeneratorItem("Randomized DFS"    , Generator::Type::RandomizedDFS    );
            GeneratorItem("Recursive Division", Generator::Type::RecursiveDivision);
            GeneratorItem("Randomized Kruskal", Generator::Type::RandomizedKruskal);
            GeneratorItem("Randomized Prim"   , Generator::Type::RandomizedPrim   );
        }

        ImGui::NewLine();

        if (ImGui::CollapsingHeader("Solvers"))
        {
            const char *heuristicNames[] = {
                "None",
                "Manhattan",
                "Euclidean"
            };

            Solver::Heuristic heuristicFuncs[] = {
                [](int x0, int y0, int x1, int y1) -> float {
                    (void)x0, (void)y0, (void)x1, (void)y1;
                    return 0;
                },

                [](int x0, int y0, int x1, int y1) -> float {
                    return abs(x1 - x0) + abs(y1 - y0);
                },

                [](int x0, int y0, int x1, int y1) -> float {
                    return sqrtf((x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0));
                },
            };

            ImGui::Combo("Heuristic", &m_state.heuristic, heuristicNames, 3);

            static auto SolverItem = [this, &heuristicFuncs](const char *n, Solver::Type t) {
                bool btn = ImGui::Button(n, ImVec2(ImGui::GetContentRegionAvail().x, 0));
                if (!btn || m_state.state != State::Idle)
                    return;
                m_maze.ClearPaths();
                m_solver.Init(&m_maze, t, heuristicFuncs[m_state.heuristic]);
                SwitchState(State::Solving);
            };

            SolverItem("Depth First Search"  , Solver::Type::DepthFirst     );
            SolverItem("Breadth First Search", Solver::Type::BreadthFirst   );
            SolverItem("Dijkstra"            , Solver::Type::Dijkstra       );
            SolverItem("A*"                  , Solver::Type::AStar          );
            SolverItem("Greedy Best First"   , Solver::Type::GreedyBestFirst);
        }

        ImGui::End();

        if (m_state.state != State::Idle) {
            auto now = m_state.clock.now();
            std::chrono::duration<float, std::milli> d = now - m_state.then;
            m_state.time += d.count();
            m_state.then = now;

            bool resume = true;
            if (m_state.state == State::Generating)
                for (; m_state.time > 0 && resume; m_state.time -= m_state.step)
                    resume = m_generator.Step();
            else
                for (; m_state.time > 0 && resume; m_state.time -= m_state.step)
                    resume = m_solver.StepAndTrace();

            if (!resume)
                SwitchState(State::Idle);

            RenderMaze();
        }
    }

    void OnRender() override
    {
        SDL_Rect src = { 0, 0, (int)(m_maze.hcells), (int)(m_maze.vcells) };
        SDL_Rect dst = { 0, 0, (int)(m_maze.hcells) * m_state.scale, (int)(m_maze.vcells) * m_state.scale };

        int w, h;
        SDL_GetWindowSize(m_window, &w, &h);
        dst.x = (w - dst.w) / 2;
        dst.y = (h - dst.h) / 2;
        SDL_RenderCopy(m_renderer, m_mazeTexture, &src, &dst);

        dst.x -= 4;
        dst.y -= 4;
        dst.w += 8;
        dst.h += 8;
        SDL_SetRenderDrawColor(m_renderer, 0xff, 0xff, 0xff, 0xff);
        SDL_RenderDrawRect(m_renderer, &dst);
    }

    void SwitchState(State::Enum state)
    {
        m_state.state = (State::Enum)state;
        m_state.time  = 0;
        m_state.then  = m_state.clock.now();
    }

    void RenderMaze()
    {
        SDL_SetRenderTarget(m_renderer, m_mazeTexture);
        m_maze.Render(m_renderer);
        SDL_SetRenderTarget(m_renderer, nullptr);
    }
};

int main(int argc, char **argv)
{
    (void) argc, (void)argv;

    MazeApp app;
    if (app.Init())
        app.Run();
    app.Destroy();
    return 0;
}
