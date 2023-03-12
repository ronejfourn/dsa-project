#include "rng.hpp"
#include <stdint.h>

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

namespace RNG {
    static pcg32_random_t rng = {123456789, 987654321};
}

unsigned RNG::Get() {
    return pcg32_random_r(&rng);
}

unsigned RNG::Get(unsigned l, unsigned h) {
    if (l > h) {auto t = l; l = h; h = t;}
    return l + pcg32_random_r(&rng) % (h - l + 1);
}
