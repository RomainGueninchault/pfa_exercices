#include <stdlib.h>
//Compute the max of an array 'arr' which length is 'len'
// If the input is invalid, return EXIT_FAILURE

int sum(int len, int arr[len]){
    if (len <= 0) {
        return EXIT_FAILURE;
    }

    int s = 0;

    for (int i = 0; i < len; i++) {
        s += arr[i];
    }

    return s;
}