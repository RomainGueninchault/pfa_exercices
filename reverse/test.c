#include <stdio.h>
#include <stdlib.h>

#define GREEN "\033[32m"
#define RED "\033[31m"
#define RESET "\033[0m"

extern void reverse(int len, int arr[len]);

void reverse_corr(int len, int arr[len])
{
    if (len < 0 || arr == NULL) {
        return;
    }

    for (int i = 0; i < len / 2; i++) {
        int tmp = arr[i];
        arr[i] = arr[len - 1 - i];
        arr[len - 1 - i] = tmp;
    }
}

int arrays_equal(int len, int a[len], int b[len])
{
    for (int i = 0; i < len; i++) {
        if (a[i] != b[i]) return 0;
    }
    return 1;
}

void copy_array(int len, int src[len], int dst[len])
{
    for (int i = 0; i < len; i++) {
        dst[i] = src[i];
    }
}

void run_test(int len, int input[len], int *passed, int *total)
{
    (*total)++;

    int stu[len];
    int corr[len];

    copy_array(len, input, stu);
    copy_array(len, input, corr);

    reverse(len, stu);
    reverse_corr(len, corr);

    if (arrays_equal(len, stu, corr)) {
        (*passed)++;
    } else {
        printf(RED "Test %d FAILED\n" RESET, *total);
        printf("Expected: ");
        for (int i = 0; i < len; i++) printf("%d ", corr[i]);
        printf("\nReceived: ");
        for (int i = 0; i < len; i++) printf("%d ", stu[i]);
        printf("\n\n");
    }
}

int main()
{
    int passed = 0;
    int total = 0;

    int arr1[] = {1,2,3,4,5};
    run_test(5, arr1, &passed, &total);

    int arr2[] = {1,2,3,4};
    run_test(4, arr2, &passed, &total);

    int arr3[] = {42};
    run_test(1, arr3, &passed, &total);

    int arr4[] = {};
    run_test(0, arr4, &passed, &total);

    int arr5[] = {-1,-2,-3,-4,-5};
    run_test(5, arr5, &passed, &total);

    int arr6[] = {5,5,5,5};
    run_test(4, arr6, &passed, &total);

    int arr7[] = {10,20};
    run_test(2, arr7, &passed, &total);

    int arr8[] = {3,1,4,1,5,9};
    run_test(6, arr8, &passed, &total);

    printf("\n");

    if (passed == total)
        printf(GREEN "[%d/%d]: tests passed! Well Done\n" RESET, passed, total);
    else
        printf(RED "[%d/%d]: tests passed\n" RESET, passed, total);

    return 0;
}