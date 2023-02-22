#include "grid.hpp"
#include "state.hpp"
#include "stack.hpp"
#include "solver.hpp"
#include "generator.hpp"

#include <SDL2/SDL_events.h>

int main() {
    renderer_t &ren = renderer_t::instance();

    grid_t grid(16, 16, 35, 35);
    stack_t<state_t> stack;

    bool on_grid = false;
    int mx, my, gx, gy, sx, sy;

    int steps_per_frame = 4;

    int mode = -1;

    SDL_Event event;
    bool run = true;
    while (run) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                run = false;
            } else if (event.type == SDL_WINDOWEVENT) {
                if (event.window.event == SDL_WINDOWEVENT_RESIZED)
                    grid.center(event.window.data1, event.window.data2);
            } else if (event.type == SDL_MOUSEMOTION) {
                mx = event.motion.x, my = event.motion.y;
                on_grid = grid.screen_to_grid(mx, my, gx, gy);
            } else if (event.type == SDL_MOUSEBUTTONDOWN) {
                if (on_grid) {
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        unsigned char &cell = grid.cells[gy * grid.hcells + gx];
                        cell = cell == WALL ? PATH : WALL;
                    }
                }
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_RETURN && stack.is_empty()) {
                    mode = 0;
                    generator::randomized_dfs(grid, stack);
                }
            }
        }

        ren.clear(0x181818);

        grid.render();

        if (mode == 0) {
            for (int i = 0; i < steps_per_frame && !stack.is_empty(); i ++) {
                state_t s = stack.peek();
                grid.cell_rect.x = grid.boundary.x + grid.border + s.x * grid.cell_rect.w;
                grid.cell_rect.y = grid.boundary.y + grid.border + s.y * grid.cell_rect.h;
                ren.fill(&grid.cell_rect, 0xff0000);
                generator::randomized_dfs(grid, stack);
            }
        }

        if (mode == 0 && stack.is_empty()) {
            mode = 1;
            solver::dfs(grid, stack);
        }

        if (mode == 1) {
            for (int i = 0; i < steps_per_frame && !stack.is_empty(); i ++) {
                state_t s = stack.peek();
                grid.cell_rect.x = grid.boundary.x + grid.border + s.x * grid.cell_rect.w;
                grid.cell_rect.y = grid.boundary.y + grid.border + s.y * grid.cell_rect.h;
                ren.fill(&grid.cell_rect, 0xff0000);
                solver::dfs(grid, stack);
            }
        }

        if (on_grid) {
            grid.grid_to_screen(gx, gy, sx, sy);
            rect_t sel1 = {
                sx, sy,
                grid.cell_rect.w, grid.cell_rect.h,
            };
            rect_t sel2 = {
                sel1.x + 1, sel1.y + 1,
                sel1.w - 2, sel1.h - 2,
            };
            ren.draw(&sel1, 0xff0000);
            ren.draw(&sel2, 0xff0000);
        }

        ren.update();
    }

    return 0;
}
