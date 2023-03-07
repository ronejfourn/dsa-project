#pragma once

namespace RNG {
    unsigned Get();
    unsigned Get(unsigned l, unsigned h);
    void Shuffle(int n, int *a);
}
