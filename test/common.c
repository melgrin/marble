#include <stdio.h>
#include <stdint.h>

#include "../src/common.c"

#define test_one_rstrip(input, expected) do { \
    char buf[128] = {0}; \
    strcat(buf, input); \
    rstrip(buf); \
    if (0 == strcmp(buf, expected)) { \
        printf("Pass: '%s' -> '%s'\n", input, buf); \
    } else { \
        printf("Fail: '%s' -> '%s' != '%s'\n", input, buf, expected); \
        return 1; \
    } \
} while (0);

int test_rstrip() {
    test_one_rstrip("hello", "hello");
    test_one_rstrip("hello ", "hello");
    test_one_rstrip(" hello", " hello");
    test_one_rstrip(" hello ", " hello");

    test_one_rstrip("hello\r\n", "hello");
    test_one_rstrip("hello\n", "hello");

    test_one_rstrip("hell\no\n", "hell\no");

    test_one_rstrip("", "");
    test_one_rstrip("\t\v\r\n  \n  \n ", "");

    return 0;
}

int main() {
    return test_rstrip();
}

