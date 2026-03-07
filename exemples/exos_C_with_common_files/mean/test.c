//
// Created by romain on 22/02/2026.
//

#include <stdio.h>
#include <assert.h>


extern int mean(int len, int arr[len]);

int meancorr(int len, int arr[len])
{
    if (len <= 0) {
        return EXIT_FAILURE;
    }
    int sum = 0;
    for (int k = 0 ; k < len ; k++)
    {
        sum += arr[k];
    }
    return(sum / len);
}

int main()
{
    int len = 5;
    int arr[1,2,3,4,5];
    int res_stu = mean(len, arr);
    int res_corr = mean_corr(len, arr);
    assert(res_stu == res_corr);

    int len = 0;
    int arr[];
    int res_stu = mean(len, arr);
    int res_corr = mean_corr(len, arr);
    assert(res_stu == res_corr);

    int len = 5;
    int arr[-1,-2,-3,-4,-5];
    int res_stu = mean(len, arr);
    int res_corr = mean_corr(len, arr);
    assert(res_stu == res_corr);
}