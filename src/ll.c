#include <ctype.h>
#include <dirent.h>
#include <linux/limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "service_printing.h"
#include "version.h"


typedef enum
{
    NOT_THE_SAME_DIR = 1 << 1
}
opts_et;

typedef struct
{
    char name[NAME_MAX];
    struct stat f_stat;
}
file_info_st;


char DirPath[PATH_MAX] = ".";

const char DESCRIPTION[] = "";

opts_et getOpts(int argc, const char* argv[], opts_et* opts);

void tolowerWord(char* word);

int sortDirent(const struct dirent **dirent1, const struct dirent **dirent2);

void printStat(const file_info_st* file_info);

int main(int argc, const char* argv[])
{
    opts_et opts;
    struct dirent** name_list;
    int names;
    file_info_st dir_info;

    if (getOpts(argc, argv, &opts) == -1)
    {
        PrintWarn("Cannot get opts");
        goto ret_err;
    }
    if (opts & NOT_THE_SAME_DIR)
    {
        chdir(argv[1]);
    }
    getcwd(DirPath, PATH_MAX);

    names = scandir(DirPath, &name_list, NULL, sortDirent);
    if (names == -1)
    {
        PrintErr("Cannot scan the directory: %s", DirPath);
        goto ret_err;
    }
    else
    {
        while (names)
        {
            names--;
            printf("%ld - %s [%d] %d\n", name_list[names]->d_ino, name_list[names]->d_name, name_list[names]->d_type,
                name_list[names]->d_reclen);
            free(name_list[names]);
        }
        free(name_list);
    }

    strncpy(dir_info.name, DirPath, PATH_MAX);
    stat(dir_info.name, &(dir_info.f_stat));
    printStat(&dir_info);

    return 0;
ret_err:
    return -1;
}

opts_et getOpts(int argc, const char* argv[], opts_et* opts)
{
    int ret = 0;
    struct stat dir_stat;

    *opts = 0;
    if (argc == 2)
    {
        if ((stat(argv[1], &dir_stat) == 0) && (dir_stat.st_mode & S_IFDIR))
        {
            *opts |= NOT_THE_SAME_DIR;
        }
        else
        {
            PrintErr("Wrong directory: %s", argv[1]);
            ret = -1;
        }
    }
    return ret;
}

int sortDirent(const struct dirent **dirent1, const struct dirent **dirent2)
{
    char file_one[NAME_MAX];
    char file_two[NAME_MAX];

    memcpy(file_one, (**dirent1).d_name, NAME_MAX);
    memcpy(file_two, (**dirent2).d_name, NAME_MAX);

    tolowerWord(file_one);
    tolowerWord(file_two);

    return -strcmp(file_one, file_two);
}

void tolowerWord(char* word)
{
    int i = 0;

    while ((word[i] != 0) || i < NAME_MAX + 1)
    {
        word[i] = tolower(word[i]);
        i++;
    }
}

void printStat(const file_info_st* file_info)
{
    printf("%s\n", file_info->name);
    printf("%d\n", file_info->f_stat.st_mode);
}
