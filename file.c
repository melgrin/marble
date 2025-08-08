#ifndef melgrin_marble_file_c
#define melgrin_marble_file_c

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/stat.h>

bool file_exists(const char* path) {
    bool result = false;
    const char* desc = "";
    do {
        struct stat buf;
        errno = 0;
        int res = stat(path, &buf);
        if (errno == ENOENT) {
            desc = "no";
            result = false;
            break;
        } else if (res == -1) {
            fprintf(stderr, "Error: failed to get information for '%s': %s\n", path, strerror(errno));
            desc = "no (error: 'stat' failed)";
            result = false;
            break;
        } else {
            if (buf.st_mode & S_IFREG) {
                desc = "yes";
                result = true;
                break;
            } else {
                desc = "no (not a 'regular file' (S_IFREG))";
                result = false;
                break;
            }
        }
    } while (0);
    printf("[info] file_exists? %s - %s\n", path, desc);
    return result;
}

// reads everything in 'filename' into 'contents'.  total number of bytes read is placed in 'length'.
// 'contents' is heap allocated - you're responsible for freeing it.  or not.
// 'perr' is optional.  if provided, and there's an error, it will contain errno.
// 'pmsg' is optional.  if provided, and there's an error, it will contain a description of the error, providing more context.
bool read_entire_file(const char* filename, unsigned char** contents, size_t* length, int* perr, const char** pmsg) {
    bool res = false;
    int err = 0;
    const char* msg = "";
    void* data = NULL;

    if (!contents) {
        msg = "'contents' is a required argument";
        goto end;
    }
    if (!length) {
        msg = "'length' is a required argument";
        goto end;
    }

    *contents = NULL;
    *length = 0;

    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        err = errno;
        msg = "fopen";
        goto end;
    }

    fseek(file, 0, SEEK_END);
    long len = ftell(file);
    if (len < 0) {
        err = errno;
        msg = "ftell";
        goto end;
    }
    if (len == 0) {
        err = 0;
        msg = "zero length";
        goto end;
    }
    fseek(file, 0, SEEK_SET);

    size_t bytes = len + 1; // + 1 for null terminator
    data = malloc(bytes);
    if (data == NULL) {
        //fprintf(stderr, "read_entire_file: failed to allocate memory; wanted %ld bytes, but got error: %s\n", len, strerror(errno));
        err = errno;
        msg = "malloc";
        goto end;
    }
    //memset(data, 0, bytes);

    size_t nr = fread(data, 1, len, file);
    if (ferror(file)) {
        err = errno;
        msg = "fread";
        goto end;
    }
    if (nr != len) {
        //fprintf(stderr, "read_entire_file: failed to read the entire file %s; wanted %ld bytes, but read %zu\n", filename, len, nr);
        err = 0; // (int) nr;
        msg = "read size mismatch";
        goto end;
    }
    *contents = (unsigned char*) data;
    *length = len;

    res = true;
end:
    if (data && !res) free(data);
    if (file) fclose(file);
    if (perr) *perr = err;
    if (pmsg) *pmsg = msg;
    return res;
}

#endif // melgrin_marble_file_c
