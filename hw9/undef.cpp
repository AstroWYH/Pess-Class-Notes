#include <stdio.h>
#include <stdint.h>

int main() {
    // int i;
    int i = 100;
    printf("value of i=%d\n", i);
    printf("address &i=%p\n", &i);
    printf("hash of i=%ld\n", ((uintptr_t)&i) & 127);
    return 0;
}
