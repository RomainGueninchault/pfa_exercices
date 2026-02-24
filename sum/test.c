//
// Created by romain on 22/02/2026.
//

#include <stdio.h>
#include <assert.h>


extern int sum(int a, int b);

int sum_corr(int a, int b)
{
    return a + b;
}

int main()
{
    int a = 3;
    int b = 5;
    assert(sum(a, b) == sum_corr(a, b));

     a = -3;
     b = 5;
    assert(sum(a, b) == sum_corr(a, b));

    a = 0;
    b = 0;
    assert(sum(a, b) == sum_corr(a, b));

     a = -3;
     b = -5;
    assert(sum(a, b) == sum_corr(a, b));
}