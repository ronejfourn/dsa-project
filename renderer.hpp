#pragma once

struct rect_t {
    int x, y, w, h;
};

struct SDL_Window;
struct SDL_Renderer;

class renderer_t {
public:
    static renderer_t &instance();
    void update();
    void draw(rect_t *rect, unsigned color);
    void fill(rect_t *rect, unsigned color);
    void clear(unsigned color);
private:
     renderer_t();
    ~renderer_t();
    SDL_Window *m_window;
    SDL_Renderer *m_renderer;
};
