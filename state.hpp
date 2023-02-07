#pragma once

struct state_t {
    int at;
    int x, y;
    int choice[4];
};

enum { L, R, B, T };
