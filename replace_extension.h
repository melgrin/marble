#include <stdlib.h>
#include <string.h>

char* replace_extension(const char* in, const char* ext) {
    char* out = calloc(strlen(in) + strlen(ext), 1);
    strcpy(out, in);
    char* p = strrchr(out, '.');
    if (p) {
        if (ext[0] != '.') p += 1;
        p[0] = 0;
    }
    else p = out;
    strcat(p, ext);
    return out;
}

