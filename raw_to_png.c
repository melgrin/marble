#include <stdio.h>
#include <string.h>
#include <assert.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include "./raw_file.c"
#include "./replace_extension.h"

// double extension to avoid overwriting original
#define OUT_EXT ".raw.png"

int main(int argc, char** argv) {
    int res = 0;

    for (int i = 1; i < argc; ++i) {
        const char* in = argv[i];
        char* out = replace_extension(in, OUT_EXT);
        printf("in:  %s\n", in);
        printf("out: %s\n", out);

        printf("reading %s...", in);
        fflush(stdout);
        RawImageInfo info;
        u8* data;
        if (raw_read(in, &data, &info)) {
            printf("done\n");
            printf("writing %s...", out);
            fflush(stdout);
            if (stbi_write_png(out, info.width, info.height, info.channels, data, info.width * info.channels)) {
                printf("done\n");
            } else {
                printf("failed to write %s\n", out);
                res += 1;
            }
        } else {
            printf("failed to load %s\n", in);
            res += 1;
        }

        free(out);
    }
    return res;
}
