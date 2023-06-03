#ifndef __READLINK_H
#define __READLINK_H

#ifdef _WIN32
#include <stdint.h>
ssize_t readlink(const char *restrict path, char *restrict link, size_t size);
#else
#include <unistd.h>
#endif

#endif // __READLINK_H
