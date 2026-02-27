#include <SDL.h>
#include <stdlib.h>
#include "../file.h"
#include "../../utils.h"


static FileStreamHandle sdl_open_file(const char* filename, const char* mode) {
    SDL_IOStream* stream = SDL_IOFromFile(filename, mode);
    if (!stream) {
        LOG(ERROR, "SDL_IOFromFile failed for '%s': %s", filename, SDL_GetError());
        return NULL;
    }
    return (FileStreamHandle)stream;
}

static int sdl_close_file(FileStreamHandle stream) {
    if (!stream) return -1;
    return SDL_CloseIO((SDL_IOStream*)stream);
}

static long long sdl_seek_file(FileStreamHandle stream, long long offset, int whence) {
    if (!stream) return -1;
    
    SDL_IOWhence sdl_whence;
    if (whence == FILE_SEEK_SET) {
        sdl_whence = SDL_IO_SEEK_SET;
    } else if (whence == FILE_SEEK_CUR) {
        sdl_whence = SDL_IO_SEEK_CUR;
    } else if (whence == FILE_SEEK_END) {
        sdl_whence = SDL_IO_SEEK_END;
    } else {
        return -1;
    }
    
    return SDL_SeekIO((SDL_IOStream*)stream, offset, sdl_whence);
}

static size_t sdl_read_file(FileStreamHandle stream, void* buffer, size_t size) {
    if (!stream) return 0;
    return SDL_ReadIO((SDL_IOStream*)stream, buffer, size);
}

static size_t sdl_write_file(FileStreamHandle stream, const void* buffer, size_t size) {
    if (!stream) return 0;
    return SDL_WriteIO((SDL_IOStream*)stream, buffer, size);
}

static FileStreamHandle sdl_open_memory(const void* data, size_t size) {
    SDL_IOStream* stream = SDL_IOFromMem((void*)data, size);
    if (!stream) {
        LOG(ERROR, "SDL_IOFromMem failed: %s", SDL_GetError());
        return NULL;
    }
    return (FileStreamHandle)stream;
}

static const FileAPI sdl_file_api = {
    .open_file = sdl_open_file,
    .close_file = sdl_close_file,
    .seek_file = sdl_seek_file,
    .read_file = sdl_read_file,
};

FileAPI* file_get_api(void) {
    return (FileAPI*)&sdl_file_api;
}
