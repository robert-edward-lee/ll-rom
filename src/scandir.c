#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "scandir.h"

#ifdef _WIN32
int scandir(const char *restrict path,
            struct dirent ***restrict namelist,
            int (*selector)(const struct dirent *),
            int (*comparator)(const struct dirent **, const struct dirent **)) {
    DIR *dir;
    struct dirent *dir_element, **names = 0, **tmp;
    size_t cnt = 0, allocated = 0;
    int old_errno = errno;

    dir = opendir(path);
    if(!dir) {
        return -1;
    }

    while(!errno && (dir_element = readdir(dir))) {
        if(selector && !selector(dir_element)) {
            continue;
        }
        /* если выделено памяти меньше и или равно кол-во элементов, то выделяем больше */
        if(cnt >= allocated) {
            allocated = 2 * allocated + 1;
            if(allocated > SIZE_MAX / sizeof(*names)) {
                break;
            }

            tmp = realloc(names, allocated * sizeof(*names));
            if(!tmp) {
                break;
            }
            names = tmp;
        }
        /* выделяем память под отдельную запись */
        names[cnt] = malloc(sizeof(struct dirent));
        if(!names[cnt]) {
            break;
        }
        memcpy(names[cnt++], dir_element, sizeof(struct dirent));
    }

    closedir(dir);

    if(errno) {
        if(names) {
            for(; cnt--; free(names[cnt])) {}
        }
        free(names);
        return -1;
    }
    errno = old_errno;

    if(comparator) {
        qsort(names, cnt, sizeof(*names), (int (*)(const void *, const void *))comparator);
    }
    *namelist = names;
    return cnt;
}
#endif
