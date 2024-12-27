#include <stdio.h>
#include <string.h>
#include <assert.h>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include <stb_image.h>

#include "./raw_file.c"
#include "./replace_extension.h"

#define OUT_EXT "raw"

int main(int argc, char** argv) {
    int res = 0;

    for (int i = 1; i < argc; ++i) {
        const char* in = argv[i];
        char* out = replace_extension(in, OUT_EXT);
        printf("in:  %s\n", in);
        printf("out: %s\n", out);

        printf("reading %s...", in);
        fflush(stdout);
        int x, y, n;
        unsigned char *data = stbi_load(in, &x, &y, &n, 0);
        if (data) {
            printf("done\n");
            assert(x > 0);
            assert(y > 0);
            assert(n > 0);
            printf("writing %s...", out);
            fflush(stdout);
            if (raw_write(out, data, x, y, n)) {
                printf("done\n");
            } else {
                printf("failed to write %s\n", out);
                res += 1;
            }
        } else {
            printf("failed to load %s: %s\n", in, stbi_failure_reason());
            res += 1;
        }

        free(out);
    }
    return res;
}
