#include "grid.hpp"
#include "stack.hpp"
#include "queue.hpp"
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
    enum {
        Generating,
        Solving,
        Idle,
    } m_state = Idle;

    struct {
        int  scale = 5;
        int  stepsPerSecond = 32;
        bool instant = false;
    } m_settings;

    struct {
        Stack<Generator::State> stack;
        Generator::StepFunc step = nullptr;
        const char *name;
    } m_generator;

    struct {
        Stack<Solver::State> stack;
        Queue<Solver::Qitem> queue;
        Solver::StepFunc step = nullptr;
        const char *name;
    } m_solver;

    void (MazeApp::*m_updateFunc)() = &MazeApp::UpdateIdle;

    static constexpr unsigned DEF_W = 125;
    static constexpr unsigned DEF_H = 125;

    Grid m_grid = Grid(DEF_W, DEF_H);
    SDL_Texture *m_gridTexture;

    unsigned m_oldW = DEF_W;
    unsigned m_oldH = DEF_H;

    float m_accumTime = 0;
    std::chrono::high_resolution_clock m_clock;
    std::chrono::high_resolution_clock::time_point m_then;

    bool OnInit() override
    {
        m_gridTexture = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, m_grid.hcells, m_grid.vcells);
        bool ok = m_gridTexture != NULL;
        if (ok) RenderMaze();

        return ok;
    }

    void OnUpdate() override
    {
        ImGui::Begin("Maze", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

        {
            m_state == Generating ? ImGui::Text("State: Generating (%s)", m_generator.name) :
            m_state == Solving    ? ImGui::Text("State: Solving (%s)", m_solver.name) :
                                    ImGui::Text("State: Idle");

            ImGui::Checkbox("Instant", &m_settings.instant);
            ImGui::SameLine();

            ImVec2 v = {ImGui::GetContentRegionAvail().x, 0};

            if (ImGui::Button("Finish", v) && m_state != Idle) {
                if (m_state == Generating)
                    while (!m_generator.stack.IsEmpty())
                        m_generator.step(m_grid, m_generator.stack);
                else
                    while (!m_solver.stack.IsEmpty())
                        m_solver.step(m_grid, m_solver.stack, m_solver.queue);

                m_state = Idle;
                m_updateFunc = &MazeApp::UpdateIdle;
                RenderMaze();
            }

            ImGui::SliderInt("Steps per Second", &m_settings.stepsPerSecond, 1, 512);
        }

        ImGui::NewLine();

        if (ImGui::CollapsingHeader("Generators"))
        {
            using namespace Generator;

            static auto GeneratorItem = [this](const char *n, InitFunc init, StepFunc step) {
                if (!ImGui::Button(n, ImVec2(ImGui::GetContentRegionAvail().x, 0)) || m_state != Idle)
                    return;

                m_accumTime = 0;
                m_then = m_clock.now();

                m_generator.name = n;
                m_generator.stack.Clear();
                init(m_grid, m_generator.stack);

                if (m_settings.instant) {
                    while (!m_generator.stack.IsEmpty())
                        step(m_grid, m_generator.stack);
                    RenderMaze();
                } else {
                    m_state = Generating;
                    m_updateFunc = &MazeApp::UpdateGenerating;
                    m_generator.step = step;
                }
            };

            GeneratorItem("Random"            , InitRandom           , StepRandom           );
            GeneratorItem("Randomized DFS"    , InitRandomizedDFS    , StepRandomizedDFS    );
            GeneratorItem("Recursive Division", InitRecursiveDivision, StepRecursiveDivision);
            GeneratorItem("Randomized Kruskal", InitRandomizedKruskal, StepRandomizedKruskal);
            GeneratorItem("Randomized Prim"   , InitRandomizedPrim   , StepRandomizedPrim   );
        }

        ImGui::NewLine();

        if (ImGui::CollapsingHeader("Solvers"))
        {
            using namespace Solver;

            static auto SolverItem = [this](const char *n, InitFunc init, StepFunc step) {
                if (!ImGui::Button(n, ImVec2(ImGui::GetContentRegionAvail().x, 0)) || m_state != Idle)
                    return;

                m_accumTime = 0;
                m_then = m_clock.now();

                m_grid.ClearPaths();
                m_solver.name = n;
                m_solver.stack.Clear();
                m_solver.queue.Clear();
                init(m_grid, m_solver.stack, m_solver.queue);

                if (m_settings.instant) {
                    while (!m_solver.stack.IsEmpty())
                        step(m_grid, m_solver.stack, m_solver.queue);
                    RenderMaze();
                } else {
                    m_state = Solving;
                    m_updateFunc = &MazeApp::UpdateSolving;
                    m_solver.step = step;
                }
            };

            SolverItem("Depth First Search"  , InitDFS     , StepDFS     );
            SolverItem("Breadth First Search", InitBFS     , StepBFS     );
            SolverItem("Dijkstra"            , InitDijkstra, StepDijkstra);
            SolverItem("A*"                  , InitAStar   , StepAStar   );
        }

        ImGui::End();


        (this->*m_updateFunc)();
    }

    void OnRender() override
    {
        SDL_Rect src = { 0, 0, static_cast<int>(m_grid.hcells), static_cast<int>(m_grid.vcells) };
        SDL_Rect dst = { 0, 0, static_cast<int>(m_grid.hcells) * m_settings.scale, static_cast<int>(m_grid.vcells) * m_settings.scale };

        int w, h;
        SDL_GetWindowSize(m_window, &w, &h);
        dst.x = (w - dst.w) / 2;
        dst.y = (h - dst.h) / 2;

        SDL_RenderCopy(m_renderer, m_gridTexture, &src, &dst);

        dst.x -= 4;
        dst.y -= 4;
        dst.w += 8;
        dst.h += 8;
        SDL_SetRenderDrawColor(m_renderer, 0xff, 0xff, 0xff, 0xff);
        SDL_RenderDrawRect(m_renderer, &dst);
    }

    void RenderMaze()
    {
        SDL_SetRenderTarget(m_renderer, m_gridTexture);
        m_grid.Render(m_renderer);
        SDL_SetRenderTarget(m_renderer, nullptr);
    }

    void UpdateGenerating()
    {
        auto now = m_clock.now();
        std::chrono::duration<float, std::ratio<1, 1>> d = now - m_then;
        m_accumTime += d.count();

        float stepTime = 1.0f / m_settings.stepsPerSecond;
        for (; m_accumTime > 0 && !m_generator.stack.IsEmpty(); m_accumTime -= stepTime)
            m_generator.step(m_grid, m_generator.stack);

        if (m_generator.stack.IsEmpty()) {
            m_state = Idle;
            m_updateFunc = &MazeApp::UpdateIdle;
        }

        RenderMaze();
        m_then = now;
    }

    void UpdateSolving()
    {
        auto now = m_clock.now();
        std::chrono::duration<float, std::ratio<1, 1>> d = now - m_then;
        m_accumTime += d.count();

        float stepTime = 1.0f / m_settings.stepsPerSecond;
        for (; m_accumTime > 0 && !m_solver.stack.IsEmpty(); m_accumTime -= stepTime)
            m_solver.step(m_grid, m_solver.stack, m_solver.queue);

        if (m_solver.stack.IsEmpty()) {
            m_state = Idle;
            m_updateFunc = &MazeApp::UpdateIdle;
        }

        RenderMaze();
        m_then = now;
    }

    void UpdateIdle()
    {
    }
};

int main(int argc, char **argv)
{
    MazeApp app;
    if (app.Init())
        app.Run();
    app.Destroy();
    return 0;
}
