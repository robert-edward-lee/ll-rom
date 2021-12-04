#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "main.h"

int main(int argc, char* argv[])
{
    char* buf;

    switch (argc) {
        case 2:
            /* Меняем текущую директорию в случае появления аргумента */
            if (chdir(argv[1]) != 0) {
                printf("Wrong path: %s\n", argv[1]);
                goto ret_fail;
            }
        case 1:
            /* Получение текущего пути */
            buf = getcwd(DirPath, PATH_MAX);
            if (buf == NULL) {
                printf("%s\n", DirPath);
                goto ret_fail;
            }
            break;
        default:
            /* Ошибка */
            printf("Something wrong\n");
            goto ret_fail;
    }
    printf("Im here: %s\n", DirPath);

    return 0;
ret_fail:
    return -1;
}
