#include <stdio.h>
#include <stdlib.h>

#define GREEN "\033[32m"
#define RED "\033[31m"
#define RESET "\033[0m"

extern int factorial(int n);

int factorial_corr(int n)
{
    if (n < 0) {
        return EXIT_FAILURE;
    }

    int res = 1;

    for (int i = 2; i <= n; i++) {
        res *= i;
    }

    return res;
}

void run_test(int n, int *passed, int *total)
{
    (*total)++;

    int res_stu = factorial(n);
    int res_corr = factorial_corr(n);

    if (res_stu == res_corr) {
        (*passed)++;
    }
    else {
        printf(RED "Test %d FAILED\n" RESET, *total);
        printf("Input: %d\n", n);
        printf("Expected: %d\n", res_corr);
        printf("Received: %d\n\n", res_stu);
    }
}

int main()
{
    int passed = 0;
    int total = 0;

    run_test(0, &passed, &total);   // 1
    run_test(1, &passed, &total);   // 1
    run_test(2, &passed, &total);   // 2
    run_test(3, &passed, &total);   // 6
    run_test(4, &passed, &total);   // 24
    run_test(5, &passed, &total);   // 120
    run_test(6, &passed, &total);   // 720
    run_test(7, &passed, &total);   // 5040
    run_test(10, &passed, &total);  // 3628800
    run_test(-1, &passed, &total);  // erreur

    printf("\n");

    if (passed == total)
        printf(GREEN "[%d/%d]: tests passed! Well Done\n" RESET, passed, total);
    else
        printf(RED "[%d/%d]: tests passed\n" RESET, passed, total);

    return 0;
}