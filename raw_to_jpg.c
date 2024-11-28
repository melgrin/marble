#include <stdio.h>
#include <string.h>
#include <assert.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

#include "./raw_file.c"

// double extension to avoid overwriting original
#define OUT_EXT ".raw.jpg"

int main(int argc, char** argv) {
    int res = 0;

    for (int i = 1; i < argc; ++i) {
        const char* in = argv[i];
        char* out = calloc(strlen(in) + strlen(OUT_EXT), 1);
        strcpy(out, in);
        char* p = strrchr(out, '.');
        if (p) {
            p[0] = 0;
            strcat(p, OUT_EXT);
        }
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
            const int quality = 100; // 1..100 // 50 looks purple
            if (stbi_write_jpg(out, info.width, info.height, info.channels, data, quality)) {
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
