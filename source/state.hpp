#pragma once

struct state_t {
    bool finished;
    int at;
    int x, y;
    int choice[4];
};

enum { L, R, B, T };
