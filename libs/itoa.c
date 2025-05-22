// itoa.c
#include <stdint.h>

char *itoa(int64_t value, char *str) {
    char *ptr = str + 20;
    *ptr = '\0';

    int64_t num = value;
    if (num == 0) {
        *--ptr = '0';
        return ptr;
    }

    int neg = 0;
    if (num < 0) {
        neg = 1;
        num = -num;
    }

    while (num > 0) {
        *--ptr = '0' + (num % 10);
        num /= 10;
    }

    if (neg) {
        *--ptr = '-';
    }

    return ptr;
}
