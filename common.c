#ifndef melgrin_marble_common_c
#define melgrin_marble_common_c

#include <stdbool.h>
#include <stdio.h>
#include <string.h> // memcpy
#include <stdlib.h> // malloc
#include <assert.h>
#include <time.h>

#include "./common.h"

#ifdef _WIN32
#include <windows.h>
#endif // _WIN32

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

double get_time() {
#ifdef _WIN32
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft); // using this instead of QueryPerformanceCounter so I don't need a one-time init of QueryPerformanceFrequency (it's probably less accurate though)
    ULARGE_INTEGER tmp = { .LowPart = ft.dwLowDateTime, .HighPart = ft.dwHighDateTime };
    double time_in_seconds_since_1601 = tmp.QuadPart / (10.*1000.*1000.); // 100 ns intervals
    // libc 'time' uses 1970 as epoch, FILETIME uses 1601 as epoch
    // number of seconds from 1 Jan. 1601 00:00 to 1 Jan 1970 00:00 UTC (copied from Time.jai)
    const u64 epoch_delta_seconds = 11644473600;

#ifndef NDEBUG
    time_t time_in_seconds_since_1970 = time(NULL);
    double diff = time_in_seconds_since_1601 - time_in_seconds_since_1970 - epoch_delta_seconds;
    assert(diff >= 0 && diff < 1.0); // time_t doesn't have millisecond resolution, but otherwise, should match
#endif

    double result = time_in_seconds_since_1601 - epoch_delta_seconds;
    return result;
#else
#error
#endif // _WIN32
}

static bool get_timestruct(time_t seconds, struct tm *out) {
#if _WIN32
    errno_t error = localtime_s(out, &seconds);
    if (error) {
        printf("localtime_s failed: error %d: %s\n", error, strerror(errno));
        return false;
    }
#else
#error TODO localtime_r
#endif
    return true;
}

const char* get_time_string_not_threadsafe() {
    double t = get_time();
    time_t seconds = (time_t) t;

    struct tm tm;
    if (!get_timestruct(seconds, &tm)) return "";

    int milliseconds = (int) ((t - seconds) * 1000);
    assert(milliseconds >= 0);
    assert(milliseconds <= 999);

    // %T = 12:45:78.012 = 12 digits
    static char buf[13]; // this is why it's not threadsafe
    memset(buf, 0, sizeof(buf));
    strftime(buf, sizeof(buf), "%T", &tm);
    sprintf(buf + strlen(buf), ".%03d", milliseconds);

    return buf;
}

const char* get_date_string_not_threadsafe() {
    double t = get_time();
    time_t seconds = (time_t) t;

    struct tm tm;
    if (!get_timestruct(seconds, &tm)) return "";

    // %F = YYYY-MM-DD = 10 digits
    static char buf[11]; // this is why it's not threadsafe
    memset(buf, 0, sizeof(buf));
    strftime(buf, sizeof(buf), "%F", &tm);

    return buf;
}

#endif // melgrin_marble_common_c
