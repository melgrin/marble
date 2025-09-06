#include <stdbool.h>
#include "./common.h"

typedef struct {
    u32 magic;
    u32 version;
    u32 width;
    u32 height;
    u8 channels;
} RawImageInfo;

bool raw_read(const char* filename, u8** pdata, RawImageInfo* pinfo);
bool raw_write(const char* filename, u8* data, u32 width, u32 height, u8 channels);

#ifdef RAW_FILE_IMPLEMENTATION

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./file.h"

static bool read32(u8** pos, size_t* rem, u32* out) {
    if (!pos || !rem || !out) return false;
    if (*rem < sizeof(u32)) return false;
    *out = *((u32*)*pos);
    *pos += sizeof(u32);
    *rem -= sizeof(u32);
    return true;
}

static bool read8(u8** pos, size_t* rem, u8* out) {
    if (!pos || !rem || !out) return false;
    if (*rem < sizeof(u8)) return false;
    *out = *((u8*)*pos);
    *pos += sizeof(u8);
    *rem -= sizeof(u8);
    return true;
}


static const u32 VER = 1;

bool raw_read(const char* filename, u8** pdata, RawImageInfo* pinfo) {
    bool res = false;

    u8* data;
    size_t len;
    int err;
    const char* msg;
    RawImageInfo info = {0};

    if (!filename) goto end;

    if (!read_entire_file(filename, &data, &len, &err, &msg)) {
        printf("failed to read %s: %s, %s\n", filename, msg, strerror(err));
        goto end;
    }

    u8* pos = data;
    size_t rem = len;
    if (!read32(&pos, &rem, &info.magic)) goto end;
    if (!read32(&pos, &rem, &info.version)) goto end;
    if (!read32(&pos, &rem, &info.width)) goto end;
    if (!read32(&pos, &rem, &info.height)) goto end;
    if (!read8 (&pos, &rem, &info.channels)) goto end;

    if (0 != strncmp((char*) &info.magic, "MRBL", 4)) goto end;
    if (info.version != VER) goto end;
    if (info.width   == 0) goto end;
    if (info.height  == 0) goto end;
    if (info.channels > 4) goto end;

    if (rem != info.width * info.height * info.channels) goto end;

    res = true;
end:
    if (res && pdata) {
        u64 n = len - (pos - data);
        u8* contents = malloc(n); // this is so I can keep using read_entire_file, which is not a very good fit here, clearly.  (need to pass a free-able pointer to caller; can't pass pointer bumped by RawImageInfo size)
        memcpy(contents, pos, n);
        *pdata = contents;
    }
    free(data);

    if (pinfo) *pinfo = info;

    return res;
}

bool raw_write(const char* filename, u8* data, u32 width, u32 height, u8 channels) {
    if (!filename) return false;
    if (!data) return false;
    if (width == 0 || height == 0) return false;
    if (channels == 0 || channels > 4) return false;

    FILE* file = fopen(filename, "wb");
    if (!file) {
        printf("failed to open file %s: %s\n", filename, strerror(errno));
        return false;
    }

    bool res = false;

    if (!fwrite("MRBL", 1, 4, file)) goto end;
    if (!fwrite(&VER, sizeof(VER), 1, file)) goto end;
    if (!fwrite(&width, sizeof(width), 1, file)) goto end;
    if (!fwrite(&height, sizeof(height), 1, file)) goto end;
    if (!fwrite(&channels, sizeof(channels), 1, file)) goto end;
    if (!fwrite(data, channels, width * height, file)) goto end;

    res = true;
end:
    if (file) fclose(file);
    return res;
}

#endif // RAW_FILE_IMPLEMENTATION

