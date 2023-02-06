#include <SDL2/SDL.h>
#include <stdlib.h>

#define ARGB(_argb) (((_argb) >> 16) & 0xff), (((_argb) >> 8) & 0xff), ((_argb) & 0xff), (((_argb) >> 24) & 0xff)

#define DEFAULT_WINDOW_WIDTH  800
#define DEFAULT_WINDOW_HEIGHT 800

SDL_Window *window;
SDL_Renderer *renderer;

struct grid_t {
    SDL_Rect cell_rect;
    int border;
    int hcells;
    int vcells;
    SDL_Rect boundary;
    unsigned char *cells;
};

// *Really* minimal PCG32 code / (c) 2014 M.E. O'Neill / pcg-random.org
// Licensed under Apache License 2.0 (NO WARRANTY, etc. see website)

typedef struct { uint64_t state;  uint64_t inc; } pcg32_random_t;

uint32_t pcg32_random_r(pcg32_random_t* rng)
{
    uint64_t oldstate = rng->state;
    // Advance internal state
    rng->state = oldstate * 6364136223846793005ULL + (rng->inc|1);
    // Calculate output function (XSH RR), uses old state for max ILP
    uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
    uint32_t rot = oldstate >> 59u;
    return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}

unsigned random_int() {
    static pcg32_random_t rng = {123456789, 987654321};
    return pcg32_random_r(&rng);
}

void shuffle(int n, int *a) {
    for (int i = n - 1; i > 1; i --) {
        int r = random_int() % i;
        int t = a[i];
        a[i]  = a[r];
        a[r]  = t;
    }
}

enum dirns_t {
    L, R, B, T
};

struct state_t {
    int at;
    int x, y;
    int choice[4];
};

struct stack_t {
    struct node_t {
        state_t state;
        node_t *next;
    };

    node_t *top;

    stack_t() : top(nullptr) {}

    void push(const state_t &s) {
        node_t *n = new node_t();
        n->state = s;
        n->next = top;
        top = n;
    }

    state_t pop() {
        node_t *tmp = top;
        state_t r = top->state;
        top = top->next;
        delete tmp;
        return r;
    }

    bool is_empty() {
        return top == nullptr;
    }
};

enum {
    WALL,
    PATH,
};

namespace generator {
    void randomized_dfs(grid_t &grid, stack_t &stack) {
        state_t cstate, nstate;
        if (stack.is_empty()) {
            memset(grid.cells, WALL, grid.hcells * grid.vcells);
            cstate.x  = 0;
            cstate.y  = 0;
            cstate.at = 0;
        } else {
            cstate = stack.pop();
        }

        int x = cstate.x;
        int y = cstate.y;

        auto cell = [grid](int x, int y) -> unsigned char& {
            return grid.cells[y * grid.hcells + x];
        };

        if (cstate.at == 4) {
            return;
        } else if (cstate.at == 0) {
            cstate.choice[0] = L;
            cstate.choice[1] = R;
            cstate.choice[2] = B;
            cstate.choice[3] = T;
            shuffle(4, cstate.choice);
            cell(x, y) = PATH;
        }

        bool next = false;

        switch (cstate.choice[cstate.at]) {
            case L:
                if (!(x == 0 || cell(x - 2, y) != WALL)) {
                    cell(x - 1, y) = PATH;
                    nstate = {0, x - 2, y}, next = true;
                }
                break;
            case R:
                if (!(x > grid.hcells - 3 || cell(x + 2, y) != WALL)) {
                    cell(x + 1, y) = PATH;
                    nstate = {0, x + 2, y}, next = true;
                }
                break;
            case B:
                if (!(y == 0 || cell(x, y - 2) != WALL)) {
                    cell(x, y - 1) = PATH;
                    nstate = {0, x, y - 2}, next = true;
                }
                break;
            case T:
                if (!(y > grid.vcells - 3 || cell(x, y + 2) != WALL)) {
                    cell(x, y + 1) = PATH;
                    nstate = {0, x, y + 2}, next = true;
                }
                break;
        }

        cstate.at ++;
        stack.push(cstate);
        if (next) stack.push(nstate);
    }
}

namespace solver {
}

int main() {
    SDL_Init(SDL_INIT_EVERYTHING);

    window = SDL_CreateWindow("Maze Generator & Solver",
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT,
            SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_SetWindowMinimumSize(window, DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);

    grid_t grid;

    grid.cell_rect.w = 8;
    grid.cell_rect.h = 8;

    grid.border = 4;

    grid.hcells = 101;
    grid.vcells = 101;

    grid.boundary.x = 0;
    grid.boundary.y = 0;

    grid.boundary.w = grid.hcells * grid.cell_rect.w + 2 * grid.border;
    grid.boundary.h = grid.vcells * grid.cell_rect.h + 2 * grid.border;
    grid.boundary.x = (DEFAULT_WINDOW_WIDTH  - grid.boundary.w) / 2;
    grid.boundary.y = (DEFAULT_WINDOW_HEIGHT - grid.boundary.h) / 2;

    grid.cells = (unsigned char *)calloc(grid.hcells * grid.vcells, 1);

    stack_t stack;
    generator::randomized_dfs(grid, stack);

    int on_grid = 0;
    int mx, my, gx, gy;

    int steps_per_frame = 16;

    SDL_Event event;
    bool run = true;
    while (run) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                run = false;
            } else if (event.type == SDL_WINDOWEVENT) {
                if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    grid.boundary.x = (event.window.data1 - grid.boundary.w) / 2;
                    grid.boundary.y = (event.window.data2 - grid.boundary.h) / 2;
                }
            } else if (event.type == SDL_MOUSEMOTION) {
                mx = event.motion.x, my = event.motion.y;
                gx = (mx - grid.boundary.x - grid.border) / grid.cell_rect.w;
                gy = (my - grid.boundary.y - grid.border) / grid.cell_rect.h;
                on_grid = (mx > grid.boundary.x + grid.border && mx < grid.boundary.x + grid.boundary.w - grid.border &&
                           my > grid.boundary.y + grid.border && my < grid.boundary.y + grid.boundary.h - grid.border);
            } else if (event.type == SDL_MOUSEBUTTONDOWN) {
                if (on_grid) {
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        unsigned char &cell = grid.cells[gy * grid.hcells + gx];
                        cell = cell == WALL ? PATH : WALL;
                    }
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, ARGB(0x181818));
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, ARGB(0xdedede));
        SDL_RenderDrawRect(renderer, &grid.boundary);

        for (unsigned i = 0; i < grid.vcells; i ++) {
            grid.cell_rect.y = grid.boundary.y + grid.border + i * grid.cell_rect.h;
            for (unsigned j = 0; j < grid.hcells; j ++) {
                grid.cell_rect.x = grid.boundary.x + grid.border + j * grid.cell_rect.w;

                (grid.cells[i * grid.hcells + j] == WALL) ?
                    SDL_SetRenderDrawColor(renderer, ARGB(0x000000)):
                    SDL_SetRenderDrawColor(renderer, ARGB(0xffffff));

                SDL_RenderFillRect(renderer, &grid.cell_rect);
            }
        }

        for (int i = 0; i < steps_per_frame; i ++) {
            if (!stack.is_empty()) {
                state_t s = stack.top->state;
                grid.cell_rect.x = grid.boundary.x + grid.border + s.x * grid.cell_rect.w;
                grid.cell_rect.y = grid.boundary.y + grid.border + s.y * grid.cell_rect.h;
                SDL_SetRenderDrawColor(renderer, ARGB(0xff0000));
                SDL_RenderFillRect(renderer, &grid.cell_rect);
                generator::randomized_dfs(grid, stack);
            }
        }

        if (on_grid) {
            SDL_Rect sel1 = {
                grid.boundary.x + grid.border + gx * grid.cell_rect.w,
                grid.boundary.y + grid.border + gy * grid.cell_rect.h,
                grid.cell_rect.w, grid.cell_rect.h,
            };

            SDL_Rect sel2 = {
                sel1.x + 1, sel1.y + 1,
                sel1.w - 2, sel1.h - 2,
            };

            SDL_SetRenderDrawColor(renderer, ARGB(0xff0000));
            SDL_RenderDrawRect(renderer, &sel1);
            SDL_RenderDrawRect(renderer, &sel2);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_Quit();

    return 0;
}
