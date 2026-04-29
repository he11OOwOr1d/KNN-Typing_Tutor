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

// Russian peasant multiplication — O(log b) instead of O(b)
// Handles both positive and negative values correctly
int my_multiply(int a, int b) {
    int negative = 0;
    if (b < 0) { negative = !negative; b = -b; }
    if (a < 0) { negative = !negative; a = -a; }

    int result = 0;
    while (b > 0) {
        if (b & 1) result += a;
        a += a;  // a = a * 2
        b >>= 1; // b = b / 2
    }
    return negative ? -result : result;
}

int my_divide(int a, int b) {
    int count = 0;
    if (b == 0) return 0;
    while (a >= b) {
        a -= b;
        count++;
    }
    return count;
}
