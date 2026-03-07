#include <stdio.h>
#include <stdlib.h>

#define GREEN "\033[32m"
#define RED "\033[31m"
#define RESET "\033[0m"

extern int sum(int len, int arr[len]);

int sum_corr(int len, int arr[len])
{
    if (len <= 0) {
        return EXIT_FAILURE;
    }

    int s = 0;

    for (int i = 0; i < len; i++) {
        s += arr[i];
    }

    return s;
}

void run_test(int len, int arr[len], int *passed, int *total)
{
    (*total)++;

    int res_stu = sum(len, arr);
    int res_corr = sum_corr(len, arr);

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

    int arr1[] = {1,2,3,4,5};
    run_test(5, arr1, &passed, &total);

    int arr2[] = {10,20,30};
    run_test(3, arr2, &passed, &total);

    int arr3[] = {-1,-2,-3,-4};
    run_test(4, arr3, &passed, &total);

    int arr4[] = {-5,10,-3,8};
    run_test(4, arr4, &passed, &total);

    int arr5[] = {0,0,0,0};
    run_test(4, arr5, &passed, &total);

    int arr6[] = {42};
    run_test(1, arr6, &passed, &total);

    int arr7[] = {100,-50,25,-10,5};
    run_test(5, arr7, &passed, &total);

    int arr8[] = {1,-1,1,-1,1,-1};
    run_test(6, arr8, &passed, &total);

    int arr9[] = {0, -1};
    run_test(2, arr9, &passed, &total);

    int arr10[] = {};
    run_test(0, arr10, &passed, &total);

    printf("\n");

    if (passed == total)
        printf(GREEN "[%d/%d]: tests passed! Well Done\n" RESET, passed, total);
    else
        printf(RED "[%d/%d]: tests passed\n" RESET, passed, total);

    return 0;
}