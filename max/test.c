#include <stdio.h>
#include <stdlib.h>

#define GREEN "\033[32m"
#define RED "\033[31m"
#define RESET "\033[0m"

extern int max(int len, int arr[len]);

int max_corr(int len, int arr[len])
{
    if (len <= 0) {
        return EXIT_FAILURE;
    }
    int max = arr[0];
    for (int i = 1; i < len; i++) {
        if (arr[i] > max) {
            max = arr[i];
        }
    }
    return max;
}

void run_test(int len, int arr[len], int *passed, int *total)
{
    (*total)++;

    int res_stu = max(len, arr);
    int res_corr = max_corr(len, arr);

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

    int arr2[] = {};
    run_test(0, arr2, &passed, &total);

    int arr3[] = {-1,-2,-3,-4,-5};
    run_test(5, arr3, &passed, &total);

    int arr4[] = {5,4,3,2,1};          
    run_test(5, arr4, &passed, &total);

    int arr5[] = {1,5,3,2,4};          
    run_test(5, arr5, &passed, &total);

    int arr6[] = {2,2,2,2,2};          
    run_test(5, arr6, &passed, &total);

    int arr7[] = {-10,4,-3,2,-1};      
    run_test(5, arr7, &passed, &total);

    int arr8[] = {0,0,0,1,0};          
    run_test(5, arr8, &passed, &total);

    int arr9[] = {42};                 
    run_test(1, arr9, &passed, &total);

    int arr10[] = {-5,-1,-9,-3,-7};    
    run_test(5, arr10, &passed, &total);

    int arr11[] = {100,50,100,20};     
    run_test(4, arr11, &passed, &total);

    printf("\n");

    if (passed == total)
        printf(GREEN "[%d/%d]: tests passed! Well Done\n" RESET, passed, total);
    else
        printf(RED "[%d/%d]: tests passed\n" RESET, passed, total);

    return 0;
}