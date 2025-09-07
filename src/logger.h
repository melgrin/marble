#ifndef melgrin_marble_logger_h
#define melgrin_marble_logger_h

#include <stdio.h>

typedef struct {
    FILE* file;
} Logger;

Logger new_log_file(const char* directory);

void remove_old_log_files(const char* directory, int number_to_keep);

void log_info(Logger* logger, const char* format, ...);

#endif // melgrin_marble_logger_h
