#ifndef melgrin_marble_common_h
#define melgrin_marble_common_h

#include <stdint.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef struct Vec2 { double x; double y; } Vec2;
typedef struct Vec2i { int x; int y; } Vec2i;
typedef struct Rect { int x; int y; int w; int h; } Rect;

#define Vec2Unpack(vec) vec.x, vec.y
#define Vec3Unpack(vec) vec.x, vec.y, vec.z

typedef struct {
    u8* data; // w * h * n
    int w; // width, pixels
    int h; // height, pixels
    int n; // number of components (3 = RGB, etc)
} Img; // raylib took the good name

#define swap(type,a,b) do { \
    type tmp = a; \
    a = b; \
    b = tmp; \
} while(0);

#define arraylen(a) (sizeof(a)/sizeof((a)[0]))

#define clamp(var, min, max) (var > max ? max : var < min ? min : var)

#endif // melgrin_marble_common_h
