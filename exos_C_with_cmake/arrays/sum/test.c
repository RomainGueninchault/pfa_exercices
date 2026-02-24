#include <stdlib.h>
#include <assert.h>
extern int sum(int len, int *arr);
int sum_corr(int len, int *arr)
{
    if (len <= 0) {
        return EXIT_FAILURE;
    }
    int total = 0;
    for (int i = 0; i < len; i++) {
        total += arr[i];
    }
    return total;
}
int main()
{
    {
        int arr[] = {1, 2, 3, 4, 5};
        int len = 5;
        assert(sum(len, arr) == sum_corr(len, arr));
    }
    {
        int arr[] = {42};
        int len = 1;
        assert(sum(len, arr) == sum_corr(len, arr));
    }
    {
        int arr[] = {-1, -2, -3, -4, -5};
        int len = 5;
        assert(sum(len, arr) == sum_corr(len, arr));
    }
    {
        int arr[] = {-3, 0, 3};
        int len = 3;
        assert(sum(len, arr) == sum_corr(len, arr));
    }
    {
        int arr[] = {0};
        int len = 0;
        assert(sum(len, arr) == sum_corr(len, arr));
    }
    return 0;
}
