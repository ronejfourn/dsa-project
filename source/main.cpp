#include "maze.hpp"
#include "solver.hpp"
#include "generator.hpp"
#include "application.hpp"
#include <chrono>
#include <cstdio>

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

        bool placeWalls = true;
        bool animate   = true;
        int  heuristic = 1;
        const char *algo = nullptr;

        float time = 0;
        int   step = 1;
        std::chrono::high_resolution_clock clock;
        std::chrono::high_resolution_clock::time_point then;
    } m_state;

    struct {
        float zoom = 8.0f;
        struct { float x, y; } pan = {0, 0};
    } m_zoompan;

    Generator m_generator;
    Solver m_solver;

    static constexpr unsigned DEF_W = 51;
    static constexpr unsigned DEF_H = 51;

    static constexpr unsigned MIN_W = 5;
    static constexpr unsigned MIN_H = 5;

    static constexpr unsigned MAX_W = 511;
    static constexpr unsigned MAX_H = 511;

    struct {
        Maze maze = Maze(DEF_W, DEF_H);
        SDL_Texture *texture = nullptr;
        unsigned w = DEF_W;
        unsigned h = DEF_H;
    } m_maze;

    bool OnInit() override
    {
        m_maze.texture = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_TARGET, 512, 512);
        bool ok = m_maze.texture != NULL;

        auto &style = ImGui::GetStyle();
        style.WindowBorderSize = 0;

        int w, h;
        SDL_GetWindowSize(m_window, &w, &h);
        m_zoompan.pan.x = -w / 2.0f / m_zoompan.zoom;
        m_zoompan.pan.y = -h / 2.0f / m_zoompan.zoom;

        return ok;
    }

    void OnUpdate() override
    {
        auto wflags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove;

        ImGui::SetNextWindowPos(ImVec2(10, 10));
        ImGui::SetNextWindowSize(ImVec2(300, 0));
        ImGui::Begin("Maze Generators & Solvers", nullptr, wflags);

        auto tflags = ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_NoTreePushOnOpen;
        if (ImGui::TreeNodeEx("Maze", tflags | ImGuiTreeNodeFlags_DefaultOpen))
        {
            bool isIdle = m_state.state == State::Idle;
            if (isIdle)
                ImGui::TextUnformatted("State: Idle");
            else
                ImGui::Text("State: %s using %s", m_state.state == State::Generating ? "Generating" : "Solving", m_state.algo);

            char buf[32] = {};
            auto flags = ImGuiSliderFlags_AlwaysClamp | ImGuiSliderFlags_NoInput;

            if (!isIdle) ImGui::BeginDisabled();
            ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);

            ImGui::Checkbox(m_state.placeWalls ? "Place Walls" : "Place Paths",
                    &m_state.placeWalls);
            if (ImGui::Button("Clear", ImVec2(ImGui::GetContentRegionAvail().x, 0)))
                m_maze.maze.Fill(PATH);

            ImGui::TextUnformatted("Dimensions");
            int w = m_maze.w >> 1;
            int h = m_maze.h >> 1;

            snprintf(buf, 32, "Width: %d", m_maze.w);
            ImGui::SliderInt("##wslider", &w, MIN_W >> 1, MAX_W >> 1, buf, flags);

            snprintf(buf, 32, "Height: %d", m_maze.h);
            ImGui::SliderInt("##hslider", &h, MIN_H >> 1, MAX_H >> 1, buf, flags);

            m_maze.w = (w << 1) | 1;
            m_maze.h = (h << 1) | 1;

            if (m_maze.w != m_maze.maze.hcells || m_maze.h != m_maze.maze.vcells)
                m_maze.maze.Resize(m_maze.w, m_maze.h);

            ImGui::PopItemWidth();
            ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - 16);

            ImGui::TextUnformatted("Start");
            int sx = m_maze.maze.start.x >> 1;
            int sy = m_maze.maze.start.y >> 1;

            snprintf(buf, 32, "%d", m_maze.maze.start.x);
            ImGui::SliderInt("X##start", &sx, 0, m_maze.maze.hcells >> 1, buf, flags);

            snprintf(buf, 32, "%d", m_maze.maze.start.y);
            ImGui::SliderInt("Y##start", &sy, 0, m_maze.maze.vcells >> 1, buf, flags);

            m_maze.maze.start.x = sx << 1;
            m_maze.maze.start.y = sy << 1;

            ImGui::TextUnformatted("End");
            int ex = m_maze.maze.end.x >> 1;
            int ey = m_maze.maze.end.y >> 1;

            snprintf(buf, 32, "%d", m_maze.maze.end.x);
            ImGui::SliderInt("X##end", &ex, 0, m_maze.maze.hcells >> 1, buf, flags);

            snprintf(buf, 32, "%d", m_maze.maze.end.y);
            ImGui::SliderInt("Y##end", &ey, 0, m_maze.maze.vcells >> 1, buf, flags);

            m_maze.maze.end.x = ex << 1;
            m_maze.maze.end.y = ey << 1;

            ImGui::PopItemWidth();
            if (!isIdle) ImGui::EndDisabled();
        }

        if (ImGui::TreeNodeEx("Animation", tflags))
        {
            ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
            ImGui::Checkbox("Animate", &m_state.animate);
            ImGui::SliderInt("##steptime", &m_state.step, 1, 100, "Time Per Step: %dms", ImGuiSliderFlags_AlwaysClamp);
            ImGui::PopItemWidth();

            if (!m_state.animate && m_state.state != State::Idle) {
                if (m_state.state == State::Generating)
                    while (m_generator.Step());
                else
                    while (m_solver.Step());

                SwitchState(State::Idle);
            }
        }

        if (ImGui::TreeNodeEx("Generators", tflags))
        {
            static auto GeneratorItem = [this](const char *n, Generator::Type type) {
                bool btn = ImGui::Button(n, ImVec2(ImGui::GetContentRegionAvail().x, 0));
                if (!btn || m_state.state != State::Idle)
                    return;
                m_state.algo = n;
                m_generator.Init(&m_maze.maze, type);
                SwitchState(State::Generating);
            };

            GeneratorItem("Random"            , Generator::Type::Random           );
            GeneratorItem("Randomized DFS"    , Generator::Type::RandomizedDFS    );
            GeneratorItem("Recursive Division", Generator::Type::RecursiveDivision);
            GeneratorItem("Randomized Kruskal", Generator::Type::RandomizedKruskal);
            GeneratorItem("Randomized Prim"   , Generator::Type::RandomizedPrim   );
        }

        if (ImGui::TreeNodeEx("Solvers", tflags))
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

            ImGui::Value("Vertices Expanded", m_solver.vertsExpanded);
            ImGui::Value("Path Length", m_solver.pathLength);

            ImGui::Combo("Heuristic", &m_state.heuristic, heuristicNames, 3);

            static auto SolverItem = [this, &heuristicFuncs](const char *n, Solver::Type t) {
                bool btn = ImGui::Button(n, ImVec2(ImGui::GetContentRegionAvail().x, 0));
                if (!btn || m_state.state != State::Idle)
                    return;
                m_state.algo = n;
                m_maze.maze.ClearPaths();
                m_solver.Init(&m_maze.maze, t, heuristicFuncs[m_state.heuristic]);
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
        }
    }

    void OnEvent(SDL_Event *event) override
    {
        auto &io = ImGui::GetIO();
        static bool lmbDown  = false;
        static bool ctrlDown = false;
        static struct { int x, y; } pan;
        static struct { int x, y; } cur;

        if (event->type == SDL_MOUSEBUTTONDOWN && event->button.button == SDL_BUTTON_LEFT) {
            pan = { event->button.x, event->button.y };
            lmbDown = true;
        }
        else if (event->type == SDL_MOUSEBUTTONUP && event->button.button == SDL_BUTTON_LEFT)
            lmbDown = false;

        else if (event->type == SDL_KEYDOWN && (event->key.keysym.sym == SDLK_LCTRL || event->key.keysym.sym == SDLK_RCTRL))
            ctrlDown = true;
        else if (event->type == SDL_KEYUP   && (event->key.keysym.sym == SDLK_LCTRL || event->key.keysym.sym == SDLK_RCTRL))
            ctrlDown = false;

        else if (event->type == SDL_MOUSEMOTION) {
            cur = { event->motion.x, event->motion.y };

            if (lmbDown && ctrlDown && !io.WantCaptureMouse) {
                m_zoompan.pan.x -= (cur.x - pan.x) / m_zoompan.zoom;
                m_zoompan.pan.y -= (cur.y - pan.y) / m_zoompan.zoom;
                pan.x = cur.x, pan.y = cur.y;
            }
        }

        else if (event->type == SDL_MOUSEWHEEL && !io.WantCaptureMouse && ctrlDown) {
            struct { float x, y; } before;
            struct { float x, y; } after;

            before.x = cur.x / m_zoompan.zoom + m_zoompan.pan.x;
            before.y = cur.y / m_zoompan.zoom + m_zoompan.pan.y;

            if (event->wheel.y > 0 && m_zoompan.zoom < 256) m_zoompan.zoom *= 1.1f;
            if (event->wheel.y < 0 && m_zoompan.zoom >   1) m_zoompan.zoom *= 0.9f;

            after.x  = cur.x / m_zoompan.zoom + m_zoompan.pan.x;
            after.y  = cur.y / m_zoompan.zoom + m_zoompan.pan.y;

            m_zoompan.pan.x += before.x - after.x;
            m_zoompan.pan.y += before.y - after.y;
        }

        else if (event->type == SDL_WINDOWEVENT && event->window.event == SDL_WINDOWEVENT_RESIZED) {
            m_zoompan.pan.x = -event->window.data1 / 2.0f / m_zoompan.zoom;
            m_zoompan.pan.y = -event->window.data2 / 2.0f / m_zoompan.zoom;
        }

        if (m_state.state == State::Idle && !io.WantCaptureMouse && lmbDown && !ctrlDown) {
            float x = cur.x / m_zoompan.zoom + m_zoompan.pan.x;
            float y = cur.y / m_zoompan.zoom + m_zoompan.pan.y;
            x += m_maze.w / 2.0f;
            y += m_maze.h / 2.0f;
            int ix = (int)x, iy = (int)y;

            if (m_maze.maze.PointInBounds(ix, iy))
                m_maze.maze(ix, iy) = m_state.placeWalls ? WALL : PATH;
        }
    }

    void OnRender() override
    {
        SDL_Rect src = { 0, 0, (int)(m_maze.w), (int)(m_maze.h) };
        SDL_FRect dst = {
            -(m_maze.w / 2.0f + m_zoompan.pan.x) * m_zoompan.zoom,
            -(m_maze.h / 2.0f + m_zoompan.pan.y) * m_zoompan.zoom,
            m_maze.w * m_zoompan.zoom,
            m_maze.h * m_zoompan.zoom,
        };

        SDL_Rect r = {0, 0, (int)m_maze.w, (int)m_maze.h};
        auto ps = m_maze.maze(m_maze.maze.start.x, m_maze.maze.start.y);
        auto pe = m_maze.maze(m_maze.maze.end  .x, m_maze.maze.end  .y);
        m_maze.maze(m_maze.maze.start.x, m_maze.maze.start.y) = 0xb16286;
        m_maze.maze(m_maze.maze.end  .x, m_maze.maze.end  .y) = 0xb8bb26;

        SDL_UpdateTexture(m_maze.texture, &r, m_maze.maze.cells, 4 * m_maze.w);
        SDL_RenderCopyF(m_renderer, m_maze.texture, &src, &dst);

        m_maze.maze(m_maze.maze.start.x, m_maze.maze.start.y) = ps;
        m_maze.maze(m_maze.maze.end  .x, m_maze.maze.end  .y) = pe;

        float d = 0.2 * m_zoompan.zoom;
        SDL_FRect rect = {0, 0, m_zoompan.zoom + 2 * d, m_zoompan.zoom + 2 * d};
        rect.x = dst.x + m_maze.maze.start.x * m_zoompan.zoom - d;
        rect.y = dst.y + m_maze.maze.start.y * m_zoompan.zoom - d;
        SDL_SetRenderDrawColor(m_renderer, 0xb1, 0x62, 0x86, 0x00);
        SDL_RenderDrawRectF(m_renderer, &rect);

        rect.x = dst.x + m_maze.maze.end.x * m_zoompan.zoom - d;
        rect.y = dst.y + m_maze.maze.end.y * m_zoompan.zoom - d;
        SDL_SetRenderDrawColor(m_renderer, 0xb8, 0xbb, 0x26, 0x00);
        SDL_RenderDrawRectF(m_renderer, &rect);
    }

    void SwitchState(State::Enum state)
    {
        m_state.state = (State::Enum)state;
        m_state.time  = 0;
        m_state.then  = m_state.clock.now();
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
