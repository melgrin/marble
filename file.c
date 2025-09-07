#ifndef melgrin_marble_file_c
#define melgrin_marble_file_c

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>

#include "./file.h"

#if _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h> // FindFirstFile, FindNextFile, FindClose
#include <direct.h> // mkdir
#endif

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

bool list_files(const char* directory, FileInfos* file_infos) {
    if (!directory || strlen(directory) == 0) return false;
#if _WIN32
    bool res = false;
    unsigned int count = 0;
    WIN32_FIND_DATA find;
    HANDLE h;
    FileInfos fis = {0};

    size_t pattern_length = strlen(directory) + strlen("/*") + 1;
    char* pattern = _alloca(pattern_length);
    if (!pattern) {
        fprintf(stderr, "list_files: failed to allocate memory for file pattern required by Win32 FindFirstFile; wanted %zu bytes, but got error: %s\n", pattern_length, strerror(errno));
        goto end;
    }
    memset(pattern, 0, pattern_length);
    strcat(pattern, directory);
    char last = directory[strlen(directory)-1];
    if (last == '/' || last == '\\') strcat(pattern, "*");
    else                             strcat(pattern, "/*");

    h = FindFirstFile(pattern, &find);
    if (h == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "list_files: failed to read directory '%s': Win32 FindFirstFile failed with error %u\n", directory, GetLastError());
        goto end;
    }
    while (1) {
        count += 1;
        if (!FindNextFile(h, &find)) break;
    }
    FindClose(h);

    fis.count = count;

    size_t bytes = count * sizeof(FileInfo);
    fis.items = malloc(bytes);
    if (fis.items == NULL) {
        fprintf(stderr, "list_files: failed to allocate memory for storing file information; wanted %zu bytes, but got error: %s\n", bytes, strerror(errno));
        goto end;
    }

    h = FindFirstFile(pattern, &find);
    if (h == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "list_files: failed to read directory '%s': Win32 FindFirstFile failed with error %u\n", directory, GetLastError());
        goto end;
    }
    for (unsigned int i = 0; i < count; ++i) {
        FileInfo fi = {0};

        fi.name = strdup(find.cFileName);

        if (find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) fi.attributes |= FILE_IS_DIRECTORY;
        if (find.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) fi.attributes |= FILE_IS_SYMLINK;
        //if (find.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) fi.attributes |= FILE_IS_HIDDEN;
        if (!(find.dwFileAttributes & FILE_ATTRIBUTE_READONLY)) fi.attributes |= FILE_IS_WRITABLE;

        //fi.size = nFileSizeLow + nFileSizeHigh * something;
        //fi.creation_time = win32_filetime_to_something(find.ftCreationTime);
        //fi.access_time = win32_filetime_to_something(find.ftLastAccessTime);
        //fi.modification_time = win32_filetime_to_something(find.ftLastWriteTime);

        fis.items[i] = fi;
        if (!FindNextFile(h, &find)) break;
    }
    FindClose(h);

    res = true;

end:
    if (res && file_infos) {
        *file_infos = fis;
    } else {
        free_file_infos(&fis);
    }
    return res;

#else
#error TODO - readdir, probably
#endif

}

void free_file_infos(FileInfos* file_infos) {
    if (file_infos && file_infos->items) {
        for (unsigned int i = 0; i < file_infos->count; ++i) {
            free(file_infos->items[i].name);
        }
        free(file_infos->items);
    }
}


bool create_directory(const char* path) {
    struct stat buf;
    errno = 0;
    int res = stat(path, &buf);
    if (errno == ENOENT) {
        res = mkdir(path);
        if (res == -1) {
            fprintf(stderr, "Error: failed to create directory '%s': %s\n", path, strerror(errno));
            return false;
        }
        return true;
    } else if (res == -1) {
        fprintf(stderr, "Error: failed to get information for '%s': %s\n", path, strerror(errno));
        return false;
    } else {
        if (buf.st_mode & S_IFDIR) {
            return true;
        }
        fprintf(stderr, "Error: wanted to make directory '%s', but it already exists as a file.\n", path);
        return false;
    }
}

#endif // melgrin_marble_file_c
