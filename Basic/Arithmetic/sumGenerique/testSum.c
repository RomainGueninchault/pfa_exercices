#include <stdio.h>
#include <assert.h>

#define GREEN   "\033[32m"
#define CLEAR   "\033[0m"
#define BOLD "\033[1m"

extern int sum(int aVal1, int aVal2);

int main() {

    assert(sum(1, 2) == 3);
    assert(sum(0, 0) == 0);
    assert(sum(-1, 1) == 0);
    assert(sum(-5, -3) == -8);
    assert(sum(-10, 5) == -5);
    assert(sum(1000, 2000) == 3000);
    assert(sum(12345, 54321) == 66666);
    printf(BOLD GREEN "sum: test passé." CLEAR "\n");
    return 0;
}
