#include <stdarg.h>
#include <stdio.h>

typedef struct {
    FILE* file;
} Logger;

void log_info(Logger* logger, const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(logger->file, format, args);
    va_end(args);
    fflush(logger->file);
}
