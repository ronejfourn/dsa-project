#include "rng.hpp"
#include "grid.hpp"
#include "stack.hpp"
#include "solver.hpp"
#include "generator.hpp"
#include "application.hpp"

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

    int m_stepsPerFrame = 4;
    int m_gridScale = 16;

    struct {
        Stack<Generator::State> stack;
        Generator::StepFunc step = nullptr;
    } m_generator;

    struct {
        Stack<Solver::State> stack;
        Solver::StepFunc step = nullptr;
    } m_solver;

    void (MazeApp::*m_updateFunc)() = &MazeApp::UpdateIdle;

    Grid m_grid = Grid(45, 45);
    SDL_Texture *m_gridTexture;

    bool OnInit() override
    {
        m_gridTexture = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, m_grid.hcells, m_grid.vcells);
        return m_gridTexture != NULL;
    }

    void OnUpdate() override
    {
        ImGui::Begin("Generators");
        {
            using namespace Generator;
            static auto GeneratorItem = [this](const char *n, InitFunc init, StepFunc step) {
                if (ImGui::Button(n, ImVec2(ImGui::GetContentRegionAvail().x, 0)) && m_state == Idle) {
                    m_state = Generating;
                    m_updateFunc = &MazeApp::UpdateGenerating;
                    m_generator.step = step;
                    m_generator.stack.Clear();
                    init(m_grid, m_generator.stack);
                }
            };

            GeneratorItem("Randomized DFS"    , InitRandomizedDFS    , StepRandomizedDFS    );
            GeneratorItem("Recursive Division", InitRecursiveDivision, StepRecursiveDivision);
            GeneratorItem("Randomized Kruskal", InitRandomizedKruskal, StepRandomizedKruskal);
        }
        ImGui::End();

        ImGui::Begin("Solvers");
        {
            using namespace Solver;
            static auto SolverItem = [this](const char *n, InitFunc init, StepFunc step) {
                if (ImGui::Button(n, ImVec2(ImGui::GetContentRegionAvail().x, 0)) && m_state == Idle) {
                    m_state = Solving;
                    m_updateFunc = &MazeApp::UpdateSolving;
                    m_solver.step = step;
                    m_solver.stack.Clear();
                    init(m_grid, m_solver.stack);
                }
            };

            SolverItem("Depth First Search", InitDFS, StepDFS);
        }
        ImGui::End();

        ImGui::Begin("Maze");
        {
            ImGui::TextUnformatted(
                    m_state == Generating ? "State: Generating" :
                    m_state == Solving ? "State: Solving" :
                    "State: Idle");

            ImVec2 dim = ImVec2(m_grid.hcells * m_gridScale, m_grid.vcells * m_gridScale);
            ImVec2 pos = {
                (ImGui::GetWindowSize().x - dim.x) * 0.5f,
                (ImGui::GetWindowSize().y - dim.y) * 0.5f,
            };
            ImGui::SetCursorPos(pos);
            ImGui::Image(m_gridTexture, dim);
        }
        ImGui::End();

        ImGui::Begin("Settings");
        {
            ImGui::DragInt("Steps per Frame", &m_stepsPerFrame, 1.0f, 1, 512);
            ImGui::DragInt("Grid Scale", &m_gridScale, 2.0f, 2, 32);
        }
        ImGui::End();

        (this->*m_updateFunc)();
    }

    void OnEvent(SDL_Event *event) override
    {
    }

    void OnDestroy() override
    {
    }

    void UpdateGenerating()
    {
        for (int i = 0; i < m_stepsPerFrame && !m_generator.stack.IsEmpty(); i ++)
            m_generator.step(m_grid, m_generator.stack);

        if (m_generator.stack.IsEmpty()) {
            m_state = Idle;
            m_updateFunc = &MazeApp::UpdateIdle;
        }

        SDL_SetRenderTarget(m_renderer, m_gridTexture);
        m_grid.Render(m_renderer);
        SDL_SetRenderTarget(m_renderer, nullptr);
    }

    void UpdateSolving()
    {
        for (int i = 0; i < m_stepsPerFrame && !m_solver.stack.IsEmpty(); i ++)
            m_solver.step(m_grid, m_solver.stack);

        if (m_solver.stack.IsEmpty()) {
            m_state = Idle;
            m_updateFunc = &MazeApp::UpdateIdle;
        }

        SDL_SetRenderTarget(m_renderer, m_gridTexture);
        m_grid.Render(m_renderer);
        SDL_SetRenderTarget(m_renderer, nullptr);
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
