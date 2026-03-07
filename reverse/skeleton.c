#include <stdlib.h>

// Reverse the array arr of length len in place.
// If the input is invalid (len < 0 or arr == NULL), do nothing.

void reverse(int len, int arr[len]){
    if (len < 0 || arr == NULL) {
        return;
    }

    for (int i = 0; i < len / 2; i++) {
        int tmp = arr[i];
        arr[i] = arr[len - 1 - i];
        arr[len - 1 - i] = tmp;
    }
}