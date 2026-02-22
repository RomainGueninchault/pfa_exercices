//
// Created by romain on 22/02/2026.
//

#include <stdio.h>
#include <assert.h>


extern int min(int len, int arr[len]);

int min_corr(int len, int arr[len])
{
    if (len <= 0) {
        return EXIT_FAILURE;
    }
    int min = arr[0];
    for (int i = 1; i < len; i++) {
        if (arr[i] < min) {
            min = arr[i];
        }
    }
    return min;
}

int main()
{
    int len = 5;
    int arr[1,2,3,4,5];
    int res_stu = min(len, arr);
    int res_corr = min_corr(len, arr);
    assert(res_stu == res_corr);

    int len = 0;
    int arr[];
    int res_stu = min(len, arr);
    int res_corr = min_corr(len, arr);
    assert(res_stu == res_corr);

    int len = 5;
    int arr[-1,-2,-3,-4,-5];
    int res_stu = min(len, arr);
    int res_corr = min_corr(len, arr);
    assert(res_stu == res_corr);
}