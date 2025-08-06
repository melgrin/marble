#ifndef melgrin_marble_common_c
#define melgrin_marble_common_c

#include <stdio.h>
#include <string.h> // memcpy
#include <stdlib.h> // malloc
#include <assert.h>

#include "./common.h"

void get_rect_into_buffer(u8* dst, const u8* src, u32 wsrc, u32 hsrc, u32 nsrc, u32 x0, u32 y0, u32 x1, u32 y1, u32 w, u32 h) {
    src += x0 * nsrc;
    src += wsrc * y0 * nsrc;
    u32 row;
    for (row = y0; row < y1 && row < hsrc; ++row) {
        memcpy(dst, src, w * nsrc);
        dst += w * nsrc;
        src += wsrc * nsrc;
    }
    printf("get_rect: x: %u-%u (w = %u), y: %u-%u (h = %u)\n", x0, x1, w, y0, y1, h);
    printf("row = %u, y1 = %u, hsrc = %u\n", row, y1, hsrc);
}

u8* get_rect(const u8* src, u32 wsrc, u32 hsrc, u32 nsrc, u32 x0, u32 y0, u32 x1, u32 y1) {
    if (x0 > x1) swap(u32, x0, x1);
    if (y0 > y1) swap(u32, y0, y1);

    u32 w = x1 - x0;
    u32 h = y1 - y0;
    u8* mem = malloc(w * h * nsrc);
    assert(mem);
    get_rect_into_buffer(mem, src, wsrc, hsrc, nsrc, x0, y0, x1, y1, w, h);
    return mem;
}

#ifdef _WIN32
#include <windows.h>
#endif // _WIN32
double get_time() {
#ifdef _WIN32
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft); // using this instead of QueryPerformanceCounter so I don't need a one-time init of QueryPerformanceFrequency (it's probably less accurate though)
    ULARGE_INTEGER tmp = { .LowPart = ft.dwLowDateTime, .HighPart = ft.dwHighDateTime };
    double sec = tmp.QuadPart / 10000000.;
    return sec;
#else
#error
#endif // _WIN32
}

#endif // melgrin_marble_common_c
