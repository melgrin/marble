#include <stdio.h>
#include <string.h>
#include <assert.h>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include <stb/stb_image.h>
//#define STB_IMAGE_WRITE_IMPLEMENTATION
//#include <stb/stb_image_write.h>

#define QOI_IMPLEMENTATION
#include <qoi.h>


int main(int argc, char** argv) {
    int res = 0;

    for (int i = 1; i < argc; ++i) {
        const char* in = argv[i];
        char* out = strdup(in);
        char* p = strrchr(out, '.');
        if (p) {
            p += 1;
            assert(strlen(p) >= strlen("qoi")); // lazy
            p[0] = 0;
            strcat(p, "qoi");
        }
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

            int encoded = qoi_write(out, data, &(qoi_desc){
                .width = x,
                .height = y,
                .channels = n,
                .colorspace = QOI_SRGB,
            });
            if (encoded) {
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
