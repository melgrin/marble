#ifndef melgrin_marble_file_h
#define melgrin_marble_file_h

#include <stdbool.h>
#include <stdlib.h>

bool file_exists(const char* path);

bool read_entire_file(const char* filename, unsigned char** contents, size_t* length, int* perr, const char** pmsg);


typedef enum {
    FILE_IS_WRITABLE  = 1,
    FILE_IS_DIRECTORY = 2,
    FILE_IS_SYMLINK   = 4,
    //FILE_IS_HIDDEN    = 8,

    FILE_IS_WRITEABLE = FILE_IS_WRITABLE,
} FileAttributes;

typedef struct {
    char* name;
    FileAttributes attributes;
} FileInfo;

typedef struct {
    unsigned int count;
    FileInfo* items;
} FileInfos;

bool list_files(const char* directory, FileInfos* file_infos);

void free_file_infos(FileInfos* file_infos);

#endif // melgrin_marble_file_h
