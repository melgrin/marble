// swiss army knife of image conversions for my uses on this project
// spawned from the remains of 8 different jpg/png/raw/qoi mains
//
// caution before you replace this with raylib's LoadImage:
// - looks like their .qoi loading is wrong, always expects 4 channels.  maybe that's just a preferred parameter?  can be 0 too.
// - default raylib build doesn't include many image types, like .jpg

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>


#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image_resize2.h>

#define QOI_IMPLEMENTATION
#include <qoi.h>

#include "typedefs.h"

#include "replace_extension.h"
#include "raw_file.c"
#include "tiff_read.c"


#ifdef _WIN32
#include <windows.h>
double get_time() {
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft); // using this instead of QueryPerformanceCounter so I don't need a one-time init of QueryPerformanceFrequency (it's probably less accurate though)
    ULARGE_INTEGER tmp = { .LowPart = ft.dwLowDateTime, .HighPart = ft.dwHighDateTime };
    double sec = tmp.QuadPart / 10000000.;
    return sec;
}
#else
#error
#endif


#include <ctype.h>
static bool streqi(const char* a, const char* b) {
    if (!a && !b) return true;
    if (!a || !b) return false;
    if (strlen(a) != strlen(b)) return false;
    size_t n = strlen(a);
    for (size_t i = 0; i < n; ++i) {
        char A = toupper(a[i]);
        char B = toupper(b[i]);
        if (A != B) return false;
    }
    return true;
}


bool read_image(const char* filename, u8** pdata, u32* pwidth, u32* pheight, u32* pchannels) {
    double t0 = get_time();

    const char* ext = strrchr(filename, '.');
    if (!ext) {
        printf("file has no extension: %s.  need an extension to determine which image loading function to call.\n", filename);
        return false;
    }
    ext++;

    printf("reading %s...", filename);
    fflush(stdout);

    u8* data;
    u32 w, h, n;
    if (streqi(ext, "raw")) {
        RawImageInfo info;
        if (!raw_read(filename, &data, &info)) {
            printf("failed to load %s\n", filename);
            exit(1);
        }
        w = info.width;
        h = info.height;
        n = info.channels;
    } else if (streqi(ext, "qoi")) {
        qoi_desc desc;
        void* mem = qoi_read(filename, &desc, 0);
        if (!mem) {
            printf("failed to load %s\n", filename);
            exit(1);
        }
        data = mem;
        w = desc.width;
        h = desc.height;
        n = desc.channels;
    } else if (streqi(ext, "tif")) {
        if (!tiff_read(filename, &data, &w, &h, &n)) {
            printf("failed to load %s\n", filename);
            exit(1);
        }
    } else {
        int x, y, comp;
        data = stbi_load(filename, &x, &y, &comp, 0);
        if (!data) {
            printf("failed to load %s: %s\n", filename, stbi_failure_reason());
            exit(1);
        }
        assert(x > 0);
        assert(y > 0);
        w = (u32) x;
        h = (u32) y;
        n = (u32) comp;
    }

    double elapsed = get_time() - t0;
    printf("done in %.3f seconds; %u x %u, %u channels\n", elapsed, w, h, n);

    if (pdata) *pdata = data;
    if (pwidth) *pwidth = w;
    if (pheight) *pheight = h;
    if (pchannels) *pchannels = n;

    return true;
}

bool resize_image(const u8* in, const u32 inw, const u32 inh, u8** out, const u32 outw, const u32 outh, const u32 channels) {
    double t0 = get_time();
    printf("resizing: %d x %d -> %d x %d...", inw, inh, outw, outh);
    fflush(stdout);
    u8* resized = stbir_resize_uint8_srgb(
        in, inw, inh, 0,
        NULL, outw, outh, 0,
        (stbir_pixel_layout) channels);
    if (!resized) {
        printf("failed to resize\n", in);
        return false;
    }
    double elapsed = get_time() - t0;
    printf("done in %.3f seconds\n", elapsed);
    *out = resized;
    return true;
}

bool write_image(const char* filename, u8* data, const u32 w, const u32 h, const u32 channels) {
    bool res = false;

    const char* ext = strrchr(filename, '.');
    if (!ext) {
        printf("file has no extension: %s.  need an extension to determine which image loading function to call.\n", filename);
        return false;
    }
    ext++;

    double t0 = get_time();

    printf("writing %s...", filename);
    fflush(stdout);

    if (streqi(ext, "raw")) {
        if (!raw_write(filename, data, w, h, channels)) {
            goto end;
        }
    } else if (streqi(ext, "qoi")) {
        if (h >= QOI_PIXELS_MAX / w) { // common failure case for this project because we're dealing with large images.  qoi checks this internally, but it's indistinguishable from other errors at the API level.
            printf("qoi does not support files larger than %d pixels.  wanted %u pixels (%ux%u).\n", QOI_PIXELS_MAX, w * h, w, h);
            goto end;
        }
        const qoi_desc desc = { .width = w, .height = h, .channels = channels, .colorspace = QOI_SRGB };
        if (!qoi_write(filename, data, &desc)) {
            goto end;
        }
    } else if (streqi(ext, "png")) {
        const u32 stride = w * channels;
        if (!stbi_write_png(filename, w, h, channels, data, stride)) {
            goto end;
        }
    } else if (streqi(ext, "jpg")) {
        const int quality = 80; // 1..100
        if (!stbi_write_jpg(filename, w, h, channels, data, quality)) {
            goto end;
        }
    } else if (streqi(ext, "bmp")) {
        if (!stbi_write_bmp(filename, w, h, channels, data)) {
            goto end;
        }
    } else {
        printf("unsupported file format '%s' (%s)\n", ext, filename);
        return false;
    }

    res = true;
end:
    if (res) {
        double elapsed = get_time() - t0;
        printf("done in %.3f seconds; %u x %u, %u channels\n", elapsed, w, h, channels);
    } else {
        printf("failed to write %s\n", filename);
    }
    return res;
}

#include "opt.c"

int main(int argc, char** argv) {
    int res = 0;

    const char* usage = "imgconv [-W, --width X] [-H, --height Y] <output file extension> <input file path>"
        "\n  -W, --width X     Width of output image, in pixels.  Default = same as input file."
        "\n  -H, --height Y    Height of output image, in pixels.  Default = same as input file."
        ;

    bool help = false;
    uint32_t width = 0;
    uint32_t height = 0;
    struct option options[] = {
        { "-h", "--help", OPT_FLAG, &help },
        { "-W", "--width", OPT_UINT32, &width },
        { "-H", "--height", OPT_UINT32, &height },
    };

    int64_t loaded_mask = options_load(argc, argv, sizeof(options)/sizeof(options[0]), options);
    if (loaded_mask < 0) return 1;

    if (help) {
        printf("%s\n", usage);
        return 0;
    }

    int num_args = 0;
    const char* ext = NULL;
    const char* in = NULL;
    for (int64_t i = 1; i < 64 && i < argc; ++i) {
        if (options_index_was_loaded(i, loaded_mask)) continue;
        if (num_args == 0) {
            ext = argv[i];
        } else if (num_args == 1) {
            in = argv[i];
        }
        num_args++;
    }
    if (num_args > 2) {
        printf("Too many arguments.  Expected 2 arguments, but got %d.\n", num_args);
        return 1;
    } else if (num_args < 2) {
        printf("Not enough arguments.  Expected 2 arguments, but got %d.\n", num_args);
        return 1;
    }

    if (ext[0] == '.') ext++;
    char* out = replace_extension(in, ext);
    printf("in:  %s\n", in);
    printf("out: %s\n", out);
    u8* data;
    u32 w, h, n;
    if (read_image(in, &data, &w, &h, &n)) {

        if (width <= 0) width = w;
        if (height <= 0) height = h;

        if (width != w || height != h) {
            u8* rsz;
            if (resize_image(data, w, h, &rsz, width, height, n)) {
                if (write_image(out, rsz, width, height, n)) {
                    printf("OK: %s -> %s\n", in, out);
                } else res++;
                free(rsz);
            } else res++;

        } else {
            if (write_image(out, data, w, h, n)) {
                printf("OK: %s -> %s\n", in, out);
            } else res++;
        }

        free(data);
    } else res++;
    free(out);

    return res;
}

// TODO: check that output is a supported file extension before loading input, because images are large and take a while to load, only to find out that you were doomed the whole time.
// TODO: option for output file name

