#pragma once

struct State {
    bool finished;
    int at;
    int x, y;
    int choice[4];
};

enum { L, R, B, T };
