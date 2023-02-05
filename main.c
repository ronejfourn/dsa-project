#include <SDL2/SDL.h>
#include <stdlib.h>

#define ARGB(_argb) (((_argb) >> 16) & 0xff), (((_argb) >> 8) & 0xff), ((_argb) & 0xff), (((_argb) >> 24) & 0xff)

#define MIN_WINDOW_WIDTH  1280
#define MIN_WINDOW_HEIGHT 720

#define BACKGROUND 0xff1d2021
#define FOREGROUND 0xffffffff
#define BORDER     0xffccff24

int main() {
    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_Window *window;
    SDL_Renderer *renderer;
    window = SDL_CreateWindow("Maze Generator & Solver",
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            MIN_WINDOW_WIDTH, MIN_WINDOW_HEIGHT,
            SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_SetWindowMinimumSize(window, MIN_WINDOW_WIDTH, MIN_WINDOW_HEIGHT);

    struct {
        SDL_Rect cell_rect;
        unsigned border;
        unsigned hcells;
        unsigned vcells;
        SDL_Rect boundary;
        unsigned char *cells;
    } grid;

    grid.cell_rect.w = 25;
    grid.cell_rect.h = 25;

    grid.border = 4;

    grid.hcells = 25;
    grid.vcells = 25;

    grid.boundary.w = grid.hcells * (grid.cell_rect.w + grid.border / 2) + 2 * grid.border - grid.border / 2;
    grid.boundary.h = grid.vcells * (grid.cell_rect.h + grid.border / 2) + 2 * grid.border - grid.border / 2;

    grid.cells = (unsigned char *)calloc(grid.hcells * grid.vcells, 1);

    int on_grid = 0;
    int mx, my, gx, gy;

    SDL_Event ev;
    int run = 1;
    while (run) {
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) {
                run = 0;
            } else if (ev.type == SDL_WINDOWEVENT) {
                if (ev.window.event == SDL_WINDOWEVENT_RESIZED) {
                    grid.boundary.x = (ev.window.data1 - grid.boundary.w) / 2;
                    grid.boundary.y = (ev.window.data2 - grid.boundary.h) / 2;
                }
            } else if (ev.type == SDL_MOUSEMOTION) {
                mx = ev.motion.x, my = ev.motion.y;
                gx = (mx - grid.boundary.x - grid.border) / (grid.cell_rect.w + grid.border / 2);
                gy = (my - grid.boundary.y - grid.border) / (grid.cell_rect.h + grid.border / 2);
                on_grid = (mx > grid.boundary.x + grid.border && mx < grid.boundary.x + grid.boundary.w - grid.border &&
                           my > grid.boundary.y + grid.border && my < grid.boundary.y + grid.boundary.h - grid.border);
            } else if (ev.type == SDL_MOUSEBUTTONDOWN) {
                if (on_grid) {
                    if (ev.button.button == SDL_BUTTON_LEFT)
                        grid.cells[gy * grid.hcells + gx] = !grid.cells[gy * grid.hcells + gx];
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, ARGB(BACKGROUND));
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, ARGB(BORDER));
        SDL_RenderDrawRect(renderer, &grid.boundary);

        for (unsigned i = 0; i < grid.vcells; i ++) {
            grid.cell_rect.y = grid.boundary.y + grid.border + i * (grid.cell_rect.h + grid.border / 2);
            for (unsigned j = 0; j < grid.hcells; j ++) {
                grid.cell_rect.x = grid.boundary.x + grid.border + j * (grid.cell_rect.w + grid.border / 2);

                (grid.cells[i * grid.hcells + j]) ?
                    SDL_SetRenderDrawColor(renderer, ARGB(0x0000ff)):
                    SDL_SetRenderDrawColor(renderer, ARGB(0x00ff00));

                SDL_RenderFillRect(renderer, &grid.cell_rect);
            }
        }

        if (on_grid) {
            SDL_Rect sel1 = {
                grid.boundary.x + grid.border + gx * (grid.cell_rect.w + grid.border / 2),
                grid.boundary.y + grid.border + gy * (grid.cell_rect.h + grid.border / 2),
                grid.cell_rect.w, grid.cell_rect.h,
            };

            SDL_Rect sel2 = {
                sel1.x + 1, sel1.y + 1,
                sel1.w - 2, sel1.h - 2,
            };

            SDL_SetRenderDrawColor(renderer, ARGB(0xffff0000));
            SDL_RenderDrawRect(renderer, &sel1);
            SDL_RenderDrawRect(renderer, &sel2);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_Quit();

    return 0;
}
