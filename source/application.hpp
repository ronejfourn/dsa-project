#pragma once

#include <SDL2/SDL.h>
#include "imgui.h"

class BaseApplication {
public:
    bool Init();
    void Run();
    void Destroy();
protected:
    struct {
        const char *title = "Default Application";
        int width  = 1280;
        int height = 720;
    } m_wConfig;

    virtual bool OnInit() { return true; }
    virtual void OnUpdate() {}
    virtual void OnRender() {}
    virtual void OnEvent(SDL_Event *) {}
    virtual void OnDestroy() {}

    SDL_Window *m_window = nullptr;
    SDL_Renderer *m_renderer = nullptr;

private:
    bool m_inited = false;
};
