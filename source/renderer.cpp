#include "renderer.hpp"
#include <SDL2/SDL.h>

#define ARGB(_argb) (((_argb) >> 16) & 0xff), (((_argb) >> 8) & 0xff), ((_argb) & 0xff), (((_argb) >> 24) & 0xff)

renderer_t &renderer_t::instance() {
    static renderer_t s_instance;
    return s_instance;
}

renderer_t::renderer_t() {
    if (SDL_Init(SDL_INIT_EVERYTHING))
        exit(1);
    m_window = SDL_CreateWindow("DSA Project",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            800, 800,
            SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (!m_window)
        exit(1);
    m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED);
    if (!m_renderer)
        exit(1);
}

renderer_t::~renderer_t() {
    SDL_DestroyRenderer(m_renderer);
    SDL_DestroyWindow(m_window);
    SDL_Quit();
}

void renderer_t::update() {
    SDL_RenderPresent(m_renderer);
    SDL_Delay(16);
}

void renderer_t::clear(unsigned color) {
    SDL_SetRenderDrawColor(m_renderer, ARGB(color));
    SDL_RenderClear(m_renderer);
}

void renderer_t::draw(rect_t *rect, unsigned color) {
    SDL_SetRenderDrawColor(m_renderer, ARGB(color));
    SDL_RenderDrawRect(m_renderer, (SDL_Rect *)rect);
}

void renderer_t::fill(rect_t *rect, unsigned color) {
    SDL_SetRenderDrawColor(m_renderer, ARGB(color));
    SDL_RenderFillRect(m_renderer, (SDL_Rect *)rect);
}
