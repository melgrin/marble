#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include <stb_image.h>

#define QOI_IMPLEMENTATION
#include <qoi.h>

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image_resize2.h>

#include "./replace_extension.h"

#define OUT_EXT ".resized.qoi"

bool make(const char* in, const float xfactor, const float yfactor) {
    if (xfactor <= 0) return false;
    if (yfactor <= 0) return false;
    bool res = false;

    char* out = replace_extension(in, OUT_EXT);
    printf("in:  %s\n", in);
    printf("out: %s\n", out);

    printf("reading %s...", in);
    fflush(stdout);
    int x, y, n;
    unsigned char* data = stbi_load(in, &x, &y, &n, 0);
    if (!data) {
        printf("failed to load %s: %s\n", in, stbi_failure_reason());
        goto end;
    }
    printf("done\n");
    assert(x > 0);
    assert(y > 0);
    assert(n > 0);
    assert(n <= 4);

    if (xfactor != 1 || yfactor != 1) {
        const int w = (int) (x * xfactor);
        const int h = (int) (y * yfactor);
        printf("resizing from %dx%d to %dx%d...", x, y, w, h);
        fflush(stdout);
        unsigned char* resized = stbir_resize_uint8_srgb(
            data, x, y, 0,
            NULL, w, h, 0,
            (stbir_pixel_layout) n);
        if (!resized) {
            printf("failed to resize\n", in);
            goto end;
        }
        printf("done\n");
        free(data);
        data = resized;
        x = w;
        y = h;
    }

    printf("writing %s...", out);
    fflush(stdout);
    int encoded = qoi_write(out, data, &(qoi_desc){
        .width = x,
        .height = y,
        .channels = n,
        .colorspace = QOI_SRGB,
    });
    if (!encoded) {
        printf("failed to write %s\n", out);
        goto end;
    }
    printf("done\n");

    res = true;
end:
    if (data) free(data);
    free(out);
    return res;
}

int main(int argc, char** argv) {
    int res = 0;
    for (int i = 1; i < argc; ++i) {
        if (!make(argv[i], 0.5, 0.5)) res += 1;
    }
    return res;
}
