#include "rng.hpp"
#include "grid.hpp"
#include "state.hpp"
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

    Stack<State> m_stack;
    Generator::Func m_generateFunc = nullptr;
    Solver::Func m_solveFunc = nullptr;
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
            static auto GeneratorItem = [this](const char *n, Generator::Func f) {
                if (ImGui::Button(n) && m_state == Idle) {
                    m_state = Generating;
                    m_generateFunc = f;
                    m_updateFunc = &MazeApp::UpdateGenerating;
                    f(m_grid, m_stack);
                }
            };

            GeneratorItem("Randomized DFS", Generator::RandomizedDFS);
        }
        ImGui::End();

        ImGui::Begin("Solvers");
        {
            static auto SolverItem = [this](const char *n, Solver::Func f) {
                if (ImGui::Button(n) && m_state == Idle) {
                    m_state = Solving;
                    m_solveFunc = f;
                    m_updateFunc = &MazeApp::UpdateSolving;
                    f(m_grid, m_stack);
                }
            };

            SolverItem("Depth First Search", Solver::DFS);
        }
        ImGui::End();

        ImGui::Begin("Maze");
        {
            ImGui::Text("State: %s",
                    m_state == Generating ? "Generating" :
                    m_state == Solving    ? "Solving"    : "Idle");
            ImGui::Image(m_gridTexture, ImVec2(m_grid.hcells * m_gridScale, m_grid.vcells * m_gridScale));
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
        for (int i = 0; i < m_stepsPerFrame && !m_stack.IsEmpty(); i ++)
            m_generateFunc(m_grid, m_stack);
        if (m_stack.IsEmpty()) {
            m_state = Idle;
            m_updateFunc = &MazeApp::UpdateIdle;
        }

        SDL_SetRenderTarget(m_renderer, m_gridTexture);
        m_grid.Render(m_renderer);
        SDL_SetRenderTarget(m_renderer, nullptr);
    }

    void UpdateSolving()
    {
        for (int i = 0; i < m_stepsPerFrame && !m_stack.IsEmpty(); i ++)
            m_solveFunc(m_grid, m_stack);
        if (m_stack.IsEmpty()) {
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

int main()
{
    MazeApp app;
    if (app.Init())
        app.Run();
    app.Destroy();
    return 0;
}
