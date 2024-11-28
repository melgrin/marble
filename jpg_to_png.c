#include <stdio.h>
#include <string.h>
#include <assert.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

int main(int argc, char** argv) {
    int res = 0;

    //stbi_write_png_compression_level = 0; // doesn't seem to have an impact on file size vs 8, which is the default.  I was hoping if there was zero compression it would load faster.  but it maybe seems to take longer with 8?  intuitively it would, but with no file size difference idk what it's doing.

    for (int i = 1; i < argc; ++i) {
        const char* in = argv[i];
        char* out = strdup(in);
        char* p = strrchr(out, '.');
        if (p) {
            p += 1;
            assert(strlen(p) >= strlen("png")); // lazy
            p[0] = 0;
            strcat(p, "png");
        }
        printf("in:  %s\n", in);
        printf("out: %s\n", out);

        printf("reading %s...", in);
        fflush(stdout);
        int x, y, n;
        unsigned char *data = stbi_load(in, &x, &y, &n, 0);
        if (data) {
            printf("done.  x %d, y %d, n %d.\n", x, y, n);
            assert(x > 0);
            assert(y > 0);
            assert(n > 0);
            printf("writing %s...", out);
            fflush(stdout);
            if (stbi_write_png(out, x, y, n, data, x * n)) {
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
