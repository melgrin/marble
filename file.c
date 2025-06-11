#include <errno.h>
#include <stdbool.h>
#include <stdio.h>

#include <sys/stat.h>

bool file_exists(const char* path) {
    printf("[info] file_exists? %s\n", path);
    struct stat buf;
    errno = 0;
    int res = stat(path, &buf);
    if (errno == ENOENT) {
        return false;
    } else if (res == -1) {
        fprintf(stderr, "Error: failed to get information for '%s': %s\n", path, strerror(errno));
        return false;
    } else {
        if (buf.st_mode & S_IFREG) {
            return true;
        } else {
            return false;
        }
    }
}

