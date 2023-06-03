#include "readlink.h"

#ifdef _WIN32
#include <windows.h>

ssize_t readlink(const char *restrict path, char *restrict link, size_t size) {
    HANDLE file;
    DWORD linkSize;

    file = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT, NULL);
    if(file == INVALID_HANDLE_VALUE) {
        return -1;
    }

    linkSize = GetFinalPathNameByHandleA(file, link, MAX_PATH, VOLUME_NAME_DOS);
    if(!linkSize && linkSize > size) {
        CloseHandle(file);
        return -1;
    }

    CloseHandle(file);
    return -1;
}
#endif
