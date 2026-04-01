//
// Created by romain on 22/02/2026.
//

#include <stdio.h>
#include <assert.h>

extern int is_prime(int n);

int is_prime_corr(int n)
{
    int i;

    if (n <= 1)
        return 0;
    i = 2;
    while (i < n)
    {
        if (n % i == 0)
            return 0;
        i++;
    }
    return 1;
}

int main()
{
    int n = 2;
    assert(is_prime(n) == is_prime_corr(n));

    n = 3;
    assert(is_prime(n) == is_prime_corr(n));

    n = 4;
    assert(is_prime(n) == is_prime_corr(n));

    n = 17;
    assert(is_prime(n) == is_prime_corr(n));

    n = 1;
    assert(is_prime(n) == is_prime_corr(n));

    n = 0;
    assert(is_prime(n) == is_prime_corr(n));

    n = -7;
    assert(is_prime(n) == is_prime_corr(n));

    n = 29;
    assert(is_prime(n) == is_prime_corr(n));

    n = 100;
    assert(is_prime(n) == is_prime_corr(n));
}
