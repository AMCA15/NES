#pragma once

#include <stddef.h>

typedef void* FileStreamHandle;

#define FILE_MODE_READ  "rb"
#define FILE_MODE_WRITE "wb"

#define FILE_SEEK_SET 0
#define FILE_SEEK_CUR 1
#define FILE_SEEK_END 2

typedef struct {
    FileStreamHandle (*open_file)(const char* filename, const char* mode);
    int (*close_file)(FileStreamHandle stream);
    long long (*seek_file)(FileStreamHandle stream, long long offset, int whence);
    size_t (*read_file)(FileStreamHandle stream, void* buffer, size_t size);
    
} FileAPI;

FileAPI* file_get_api(void);
