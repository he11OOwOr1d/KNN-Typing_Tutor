#include "math.h"

static unsigned int rand_state = 1;

void my_srand(unsigned int seed) {
    if (seed == 0) seed = 1;
    rand_state = seed;
}

// Xorshift32 PRNG (Requires extremely minimal system power, pure bitwise)
unsigned int my_rand() {
    rand_state ^= rand_state << 13;
    rand_state ^= rand_state >> 17;
    rand_state ^= rand_state << 5;
    return rand_state;
}

// Handles both positive and negative values correctly
int my_multiply(int a, int b) {
    int sign = ((a < 0) ^ (b < 0)) ? -1 : 1;

    unsigned int x = (a < 0) ? -a : a;
    unsigned int y = (b < 0) ? -b : b;

    // iterate on smaller number (fewer loop iterations)
    if (x < y) {
        unsigned int temp = x;
        x = y;
        y = temp;
    }

    int result = 0;

    while (y) {
        if (y & 1)
            result += x;

        x <<= 1;
        y >>= 1;
    }

    return sign * result;
}

int my_divide(int a, int b) {
    if (b == 0) return 0;

    int sign = ((a < 0) ^ (b < 0)) ? -1 : 1;

    unsigned int dividend = (a < 0) ? -a : a;
    unsigned int divisor  = (b < 0) ? -b : b;

    unsigned int result = 0;

    for (int i = 31; i >= 0; i--) {
        if ((dividend >> i) >= divisor) {
            dividend -= (divisor << i);
            result += (1U << i);
        }
    }

    return sign * result;
}