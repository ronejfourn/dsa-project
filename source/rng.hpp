#pragma once

namespace RNG {
    unsigned Get();
    unsigned Get(unsigned l, unsigned h);

    template <typename T>
    void Shuffle(unsigned n, T *a) {
        T temp;
        for (unsigned i = n - 1; i > 1; i --) {
            unsigned r = RNG::Get() % i;
            temp = a[i];
            a[i] = a[r];
            a[r] = temp;
        }
    }
}
