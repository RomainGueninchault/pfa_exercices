#include <stdio.h>
#include <stdlib.h>

#define GREEN "\033[32m"
#define RED "\033[31m"
#define RESET "\033[0m"

extern int pythagore(int a, int b, int c);

int pythagore_corr(int a, int b, int c)
{
    if (a <= 0 || b <= 0 || c <= 0) {
        return EXIT_FAILURE;
    }

    if (a*a == b*b + c*c) return 1;
    if (b*b == a*a + c*c) return 1;
    if (c*c == a*a + b*b) return 1;

    return 0;
}

void run_test(int a, int b, int c, int *passed, int *total)
{
    (*total)++;

    int res_stu = pythagore(a, b, c);
    int res_corr = pythagore_corr(a, b, c);

    if (res_stu == res_corr) {
        (*passed)++;
    }
    else {
        printf(RED "Test %d FAILED\n" RESET, *total);
        printf("Expected: %d\n", res_corr);
        printf("Received: %d\n\n", res_stu);
    }
}

int main()
{
    int passed = 0;
    int total = 0;

    run_test(3,4,5, &passed, &total);
    run_test(5,3,4, &passed, &total);
    run_test(4,5,3, &passed, &total);

    run_test(6,8,10, &passed, &total);
    run_test(8,6,10, &passed, &total);

    run_test(5,12,13, &passed, &total);
    run_test(7,24,25, &passed, &total);

    run_test(2,3,4, &passed, &total);
    run_test(10,10,10, &passed, &total);

    run_test(-3,4,5, &passed, &total);
    run_test(0,4,5, &passed, &total);

    printf("\n");

    if (passed == total)
        printf(GREEN "[%d/%d]: tests passed! Well Done\n" RESET, passed, total);
    else
        printf(RED "[%d/%d]: tests passed\n" RESET, passed, total);

    return 0;
}