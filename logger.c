#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h> // qsort
#include <string.h>
#include <errno.h>
#include <time.h> // mktime
#include <assert.h>

#include "./logger.h"
#include "./common.h"
#include "./file.h"

#if _WIN32
#include <windows.h> // _alloca
#endif


void log_info(Logger* logger, const char* format, ...) {
    if (!logger || !logger->file || !format) return;
    va_list args;
    va_start(args, format);
    vfprintf(logger->file, format, args);
    va_end(args);
    fflush(logger->file);
}


Logger new_log_file(const char* directory) {
    const char* d0 = get_date_string_not_threadsafe(); // XXX assuming this matches the format that the logger wants (which it doesn't btw, see the char replacements below)
    const char* t0 = get_time_string_not_threadsafe(); // XXX assuming this matches the format that the logger wants (which it doesn't btw, see the char replacements below)
    char* d = _alloca(strlen(d0) + 1); memcpy(d, d0, strlen(d0) + 1);
    char* t = _alloca(strlen(t0) + 1); memcpy(t, t0, strlen(t0) + 1);
    for (char* c = d; *c; ++c) { if (*c == '-') *c = '_'; }
    for (char* c = t; *c; ++c) { if (*c == ':' || *c == '.') *c = '_'; }
    size_t n = strlen(directory) + strlen("/") + strlen(d) + strlen("_") + strlen(t) + strlen(".txt") + 1;
    char* log_filename = _alloca(n);
    snprintf(log_filename, n, "%s/%s_%s.txt", directory, d, t);
    Logger logger = {0};
    logger.file = fopen(log_filename, "wb");
    if (!logger.file) {
        printf("Failed to initialize logger: failed to open file '%s': error %d: %s\n", log_filename, errno, strerror(errno));
    }
    log_info(&logger, "\n%s %s session start\n", d0, t0);
    return logger;
}


typedef struct {
    FileInfo file_info;
    u64 time;
} FileSortHelper;

static int compare_file_times(const void* _a, const void* _b) {
    assert(_a && _b);
    if (!_a || !_b) return 0;
    const FileSortHelper* a = (const FileSortHelper*) _a;
    const FileSortHelper* b = (const FileSortHelper*) _b;
    // ascending
    if (a->time < b->time) return -1;
    if (a->time > b->time) return 1;
    return 0;
}

void remove_old_log_files(const char* directory, int number_to_keep) {
    FileInfos file_infos = {0};
    if (list_files(directory, &file_infos)) {
        FileSortHelper* matches = _alloca(file_infos.count * sizeof(FileSortHelper));
        assert(matches);
        int match_count = 0;
        for (u32 i = 0; i < file_infos.count; ++i) {
            FileInfo fi = file_infos.items[i];
            if (fi.attributes & (FILE_IS_DIRECTORY | FILE_IS_SYMLINK)) continue;
            if (strlen(fi.name) != strlen("yyyy_mm_dd_HH_MM_SS_sss.txt")) continue;
            int year, month, day, hour, minute, second, millisecond;
            char suffix[4] = {0};
            int n = sscanf(fi.name, "%d_%d_%d_%d_%d_%d_%d.%3s", &year, &month, &day, &hour, &minute, &second, &millisecond, suffix);
            if (n != 8) continue;
            if (year        < 1900 || year        > 3000) continue;
            if (month       < 1    || month       > 12)   continue;
            if (day         < 1    || day         > 31)   continue;
            if (hour        < 0    || hour        > 23)   continue;
            if (minute      < 0    || minute      > 60)   continue;
            if (second      < 0    || second      > 60)   continue;
            if (millisecond < 0    || millisecond > 999)  continue;
            if (0 != strcmp(suffix, "txt")) continue;

            struct tm tt = {
                .tm_year = year - 1900,
                .tm_mon = month - 1, // 0-11
                .tm_mday = day, // 1-31
                .tm_hour = hour, // 0-23
                .tm_min = minute, // 0-59
                .tm_sec = second, // 0-59
            };
            time_t mk = mktime(&tt);
            if (mk < 0) continue;
            u64 t = ((u64) mk) * 1000 + millisecond;

            matches[match_count].time = t;
            matches[match_count].file_info = fi;
            match_count += 1;
        }

        int delete_count = match_count - number_to_keep;
        if (delete_count > 0) {
            qsort(matches, match_count, sizeof(matches[0]), compare_file_times);

            for (int i = 0; i < delete_count; ++i) {
                char path[512];
                snprintf(path, sizeof(path), "%s/%s", directory, matches[i].file_info.name);
                remove(path);
            }
        }

        free_file_infos(&file_infos);
    }
}

