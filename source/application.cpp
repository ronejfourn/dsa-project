#include "application.hpp"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer.h"

bool BaseApplication::Init() {
    if (m_inited) return true;
    if (SDL_Init(SDL_INIT_EVERYTHING)) return false;

    int wf =  SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI;
    m_window = SDL_CreateWindow(m_wConfig.title,
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            m_wConfig.width, m_wConfig.height, wf);
    if (!m_window) return false;

    int rf = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE;
    m_renderer = SDL_CreateRenderer(m_window, -1, rf);
    if (!m_renderer) return false;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForSDLRenderer(m_window, m_renderer);
    ImGui_ImplSDLRenderer_Init(m_renderer);

    m_inited = OnInit();
    return m_inited;
}

void BaseApplication::Run() {
    if (!m_inited) return;

    auto &io  = ImGui::GetIO();
    bool done = false;

    while (!done) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            ImGui_ImplSDL2_ProcessEvent(&ev);
            if (ev.type == SDL_QUIT)
                done = true;
            OnEvent(&ev);
        }

        ImGui_ImplSDLRenderer_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        OnUpdate();
        ImGui::Render();
        SDL_RenderSetScale(m_renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
        SDL_SetRenderDrawColor(m_renderer, 0x18, 0x18, 0x18, 0x18);
        SDL_RenderClear(m_renderer);
        ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
        SDL_RenderPresent(m_renderer);
    }
}

void BaseApplication::Destroy() {
    OnDestroy();
    ImGui_ImplSDLRenderer_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    SDL_DestroyRenderer(m_renderer);
    SDL_DestroyWindow(m_window);
    SDL_Quit();
    m_inited = false;
    m_window = nullptr;
    m_renderer = nullptr;
}
