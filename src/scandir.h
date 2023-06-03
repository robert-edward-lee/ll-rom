#ifndef __SCANDIR_H
#define __SCANDIR_H

#include <dirent.h>

#ifdef _WIN32
int scandir(const char *restrict path,
            struct dirent ***restrict namelist,
            int (*selector)(const struct dirent *),
            int (*comparator)(const struct dirent **, const struct dirent **));
#endif // _WIN32

#endif // __SCANDIR_H
