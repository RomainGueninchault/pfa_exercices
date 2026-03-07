#include <stdlib.h>

// Returns 1 if a, b, and c satisfy the Pythagorean theorem; otherwise returns 0.

int pythagore(int a, int b, int c){
    if (a <= 0 || b <= 0 || c <= 0) {
        return EXIT_FAILURE;
    }

    if (a*a == b*b + c*c) return 1;
    if (b*b == a*a + c*c) return 1;
    if (c*c == a*a + b*b) return 1;

    return 0;
}