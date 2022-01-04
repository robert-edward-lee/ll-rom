#include <ctype.h>
#include <dirent.h>
#include <grp.h>
#include <linux/limits.h>
#include <pwd.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "bash_format.h"
#include "service_printing.h"
#include "version.h"


typedef enum
{
    NOT_THE_SAME_DIR = 1 << 1,
    DESCRIPTION = ~NOT_THE_SAME_DIR
}
opts_et;

typedef struct
{
    char name[PATH_MAX];
    struct stat f_stat;
}
file_info_st;


char DirPath[PATH_MAX] = ".";

const char ABOUT[] = "ll v." VERSION "\n";

opts_et getOpts(int argc, const char* argv[], opts_et* opts);

void tolowerWord(char* word);

int sortDirent(const struct dirent** dirent1, const struct dirent** dirent2);

void printStat(const file_info_st* file_info);

void signalHandler(int signal, siginfo_t* signalInfo, void* userContext);

void signalCatcher(void);

int main(int argc, const char* argv[])
{
    opts_et opts;
    struct dirent** name_list;
    int names, i;
    file_info_st* data_files;

    signalCatcher();

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
        data_files = (file_info_st*)malloc(names * sizeof(file_info_st));
        for (i = 0; i < names; i++)
        {
            strncpy(data_files[i].name, name_list[i]->d_name, PATH_MAX);
            stat(name_list[i]->d_name, &(data_files[i].f_stat));
            // printf("%ld - %s [%d] %d\n", name_list[i]->d_ino, name_list[i]->d_name, name_list[i]->d_type,
            //     name_list[i]->d_reclen);
            free(name_list[i]);
        }
        free(name_list);
    }

    for (i = 0; i < names; i++)
    {
        printStat(&data_files[i]);
    }

    free(data_files);
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
        if (strncmp(argv[1], "-h", sizeof("-h")) == 0)
        {
            fprintf(stdout, ABOUT);
            ret = DESCRIPTION;
            exit(EXIT_SUCCESS);
        }
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

int sortDirent(const struct dirent** dirent1, const struct dirent** dirent2)
{
    char file_one[NAME_MAX];
    char file_two[NAME_MAX];

    memcpy(file_one, (**dirent1).d_name, NAME_MAX);
    memcpy(file_two, (**dirent2).d_name, NAME_MAX);

    tolowerWord(file_one);
    tolowerWord(file_two);
    /* костыль: переписать нормальное сравнение выкинув из сравнения все не буквы */
    return strcmp(isalpha(file_one[0]) ? file_one : (file_one + 1), isalpha(file_two[0]) ? file_two : (file_two + 1));
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
    char text[PATH_MAX];
    char file_type;

    char* text_offset = text;
    struct tm* local_time;
    struct passwd* pw;
    struct group* gr;

    /* печать прав доступа */
    if ((file_info->f_stat.st_mode & S_IFMT) == S_IFBLK)        file_type = 'b';
    else if ((file_info->f_stat.st_mode & S_IFMT) == S_IFCHR)   file_type = 'c';
    else if ((file_info->f_stat.st_mode & S_IFMT) == S_IFDIR)   file_type = 'd';
    else if ((file_info->f_stat.st_mode & S_IFMT) == S_IFLNK)   file_type = 'l';
    else if ((file_info->f_stat.st_mode & S_IFMT) == S_IFIFO)   file_type = 'p';
    else if ((file_info->f_stat.st_mode & S_IFMT) == S_IFSOCK)  file_type = 's';
    else                                                        file_type = '-';
    text_offset += sprintf(text_offset, "%c", file_type);
    text_offset += sprintf(text_offset, "%s", (file_info->f_stat.st_mode & S_IRUSR) ? "r" : "-");
    text_offset += sprintf(text_offset, "%s", (file_info->f_stat.st_mode & S_IWUSR) ? "w" : "-");
    text_offset += sprintf(text_offset, "%s", (file_info->f_stat.st_mode & S_IXUSR) ? "x" : "-");
    text_offset += sprintf(text_offset, "%s", (file_info->f_stat.st_mode & S_IRGRP) ? "r" : "-");
    text_offset += sprintf(text_offset, "%s", (file_info->f_stat.st_mode & S_IWGRP) ? "w" : "-");
    text_offset += sprintf(text_offset, "%s", (file_info->f_stat.st_mode & S_IXGRP) ? "x" : "-");
    text_offset += sprintf(text_offset, "%s", (file_info->f_stat.st_mode & S_IROTH) ? "r" : "-");
    text_offset += sprintf(text_offset, "%s", (file_info->f_stat.st_mode & S_IWOTH) ? "w" : "-");
    text_offset += sprintf(text_offset, "%s", (file_info->f_stat.st_mode & S_IXOTH) ? "x" : "-");
    text_offset += sprintf(text_offset, " ");
    /* какое-то число */
    text_offset += sprintf(text_offset, "%2ld ", file_info->f_stat.st_nlink);
    /* печать пользователя и группы */
    pw = getpwuid(file_info->f_stat.st_uid);
    gr = getgrgid(file_info->f_stat.st_gid);
    text_offset += sprintf(text_offset, "%s %s", pw->pw_name, gr->gr_name);
    /* печать размера файла */
    text_offset += sprintf(text_offset, "%5ld", file_info->f_stat.st_size);
    text_offset += sprintf(text_offset, " ");
    /* печать времени последнего изменения */
    local_time = localtime(&(file_info->f_stat.st_mtim.tv_sec));
    text_offset += strftime(text_offset, NAME_MAX, "%b %-2d %H:%M ", local_time);
    /* печать имени файла */
    text_offset += (file_type == 'd')
        ? sprintf(text_offset, BASH_BLUE "%s" BASH_DEFAULT "/\n",file_info->name)
        : sprintf(text_offset, "%s\n",file_info->name);

    fprintf(stdout, text);
}

void signalHandler(int signal, siginfo_t* signalInfo, void* userContext)
{
    if (signal == SIGSEGV)
    {
        switch (signalInfo->si_code)
        {
            case SEGV_MAPERR:
                PrintWarn("Address not mapped to object(%s)", QUOTE(SEGV_MAPERR));
                break;
            case SEGV_ACCERR:
                PrintWarn("Invalid permissions for mapped object(%s)", QUOTE(SEGV_ACCERR));
                break;
            case SEGV_BNDERR:
                PrintWarn("Bounds checking failure(%s)", QUOTE(SEGV_BNDERR));
                break;
            case SEGV_PKUERR:
                PrintWarn("Protection key checking failure(%s)", QUOTE(SEGV_PKUERR));
                break;
            case SEGV_ACCADI:
                PrintWarn("ADI not enabled for mapped object(%s)", QUOTE(SEGV_ACCADI));
                break;
            case SEGV_ADIDERR:
                PrintWarn("Disrupting MCD error(%s)", QUOTE(SEGV_ADIDERR));
                break;
            case SEGV_ADIPERR:
                PrintWarn("Precise MCD exception(%s)", QUOTE(SEGV_ADIPERR));
                break;
            default:
                PrintWarn("Unknown error");
                break;
        }
        PrintWarn("Address of faulting memory reference: %p", signalInfo->si_addr);
    }
    exit(EXIT_FAILURE);
}

void signalCatcher(void)
{
    struct sigaction signalAction;

    memset(&signalAction, 0, sizeof (signalAction));
    sigaddset(&(signalAction.sa_mask), SIGSEGV);
    signalAction.sa_sigaction = signalHandler;
    signalAction.sa_flags = SA_SIGINFO;

    sigaction(SIGSEGV, &signalAction, NULL);
}
