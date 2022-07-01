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
#include <sys/sysmacros.h>
#include <time.h>
#include <unistd.h>

#include "bash_format.h"
#include "service_printing.h"
#include "version.h"

#define MAX_REC_COUNT 40

typedef enum
{
    NOT_THE_SAME_DIR = 1 << 1,
    DESCRIPTION = ~NOT_THE_SAME_DIR
}
opts_et;

typedef struct
{
    /* Имя файла */
    char name[PATH_MAX];
    /* Имя куда ссылается ссылка, если файл это ссылка */
    char link[PATH_MAX];
    /* Данные файла */
    struct stat f_stat;
    /* Данные куда ссылается ссылка, если файл это ссылка */
    struct stat l_stat;
}
file_info_st;


char DirPath[PATH_MAX] = ".";

const char ABOUT[] = "ll v." VERSION "\n";

int linkAlign = 0;
int userAlign = 0;
int groupAlign = 0;
int sizeAlign = 0;
int majorAlign = 0;
int minorAlign = 0;


opts_et getOpts(int argc, const char *argv[], opts_et *opts);
void tolowerWord(char *word);
int sortDirent(const struct dirent** dirent1, const struct dirent** dirent2);
struct stat handleLink(char *path);
void printStat(const file_info_st *file_info);
int putGayText(char *out_text, const char *in_text, char file_type, __mode_t file_mode);
char getFileType(const char *path);
void signalHandler(int signal, siginfo_t *signalInfo, void *userContext);
void signalCatcher(void);
void setAlignment(const file_info_st *file_info);


int main(int argc, const char *argv[])
{
    opts_et opts;
    struct dirent** name_list;
    int names, i;
    file_info_st *data_files;

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

    names = scandir(DirPath, &name_list, NULL,
#ifdef ARM
        (int (*)(const void*, const void*))
#endif
        sortDirent);
    if (names == -1)
    {
        PrintErr("Cannot scan the directory: %s", DirPath);
        goto ret_err;
    }
    else
    {
        data_files = (file_info_st*)calloc(names, sizeof(file_info_st));
        for (i = 0; i < names; i++)
        {
            /* Заполняем инфу о файле */
            strncpy(data_files[i].name, name_list[i]->d_name, PATH_MAX);
            lstat(name_list[i]->d_name, &(data_files[i].f_stat));
            /* Если это символьная ссылка, то получаем данные куда ссылка ведёт */
            if (getFileType(data_files[i].name) == 'l')
            {
                strncpy(data_files[i].link, data_files[i].name, PATH_MAX);
                data_files[i].l_stat = handleLink(data_files[i].link);
            }
            free(name_list[i]);
        }
        free(name_list);
    }

    for (i = 0; i < names; i++)
    {
        setAlignment(&data_files[i]);
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

opts_et getOpts(int argc, const char *argv[], opts_et *opts)
{
    int ret = 0;
    struct stat dir_stat;

    *opts = 0;
    if (argc == 2)
    {
        if (strncmp(argv[1], "-h", sizeof("-h")) == 0)
        {
            ret = DESCRIPTION;
        }
        else if ((stat(argv[1], &dir_stat) == 0) && (dir_stat.st_mode & S_IFDIR))
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
    char *file_one_p = file_one;
    char *file_two_p = file_two;
    size_t i;

    memcpy(file_one, (**dirent1).d_name, NAME_MAX);
    memcpy(file_two, (**dirent2).d_name, NAME_MAX);

    tolowerWord(file_one);
    tolowerWord(file_two);

    for(i = 0; i != NAME_MAX && file_one[i] != '\0'; i++)
    {
        if(isalpha(file_one[i]))
        {
            file_one_p = &file_one[i];
            break;
        }
    }

    for(i = 0; i != NAME_MAX && file_two[i] != '\0'; i++)
    {
        if(isalpha(file_two[i]))
        {
            file_two_p = &file_two[i];
            break;
        }
    }

    return strncmp(file_one_p, file_two_p, NAME_MAX);
}

struct stat handleLink(char *path)
{
    static int rec_count = 0;
    struct stat ret_stat;
    char link[PATH_MAX] = {0};

    DbgPrint("%s -> ", path);
    if (rec_count == MAX_REC_COUNT)
    {
        PrintWarn("Too many nested symbolic link: %s", path);
        exit(EXIT_FAILURE);
    }

    if (getFileType(path) == 'l')
    {
        readlink(path, link, PATH_MAX);
        sprintf(path, link);
        memset(link, 0, PATH_MAX);
        rec_count++;
        handleLink(path);
    }
    else
    {
        lstat(path, &ret_stat);
        rec_count = 0;
    }
    return ret_stat;
}

void tolowerWord(char *word)
{
    int i = 0;

    while ((word[i] != 0) || i < NAME_MAX + 1)
    {
        word[i] = tolower(word[i]);
        i++;
    }
}

void printStat(const file_info_st *file_info)
{
    char file_type;
    char text[PATH_MAX];
    char *text_offset = text;
    struct tm local_time, global_time;

    time_t tmp_t = time(NULL);
    global_time = *localtime(&tmp_t);

    /* печать типа файла */
    file_type = getFileType(file_info->name);
    text_offset += sprintf(text_offset, "%c", file_type);
    /* печать прав доступа пользователя */
    text_offset += sprintf(text_offset, "%s", (file_info->f_stat.st_mode & S_IRUSR) ? "r" : "-");
    text_offset += sprintf(text_offset, "%s", (file_info->f_stat.st_mode & S_IWUSR) ? "w" : "-");
    if(file_info->f_stat.st_mode & S_IXUSR)
    {
        text_offset += sprintf(text_offset, "%s", (file_info->f_stat.st_mode & S_ISUID) ? "s" : "x");
    }
    else
    {
        text_offset += sprintf(text_offset, "%s", (file_info->f_stat.st_mode & S_ISUID) ? "S" : "-");
    }
    /* печать прав доступа группы */
    text_offset += sprintf(text_offset, "%s", (file_info->f_stat.st_mode & S_IRGRP) ? "r" : "-");
    text_offset += sprintf(text_offset, "%s", (file_info->f_stat.st_mode & S_IWGRP) ? "w" : "-");
    if(file_info->f_stat.st_mode & S_IXGRP)
    {
        text_offset += sprintf(text_offset, "%s", (file_info->f_stat.st_mode & S_ISGID) ? "s" : "x");
    }
    else
    {
        text_offset += sprintf(text_offset, "%s", (file_info->f_stat.st_mode & S_ISGID) ? "S" : "-");
    }
    /* печать прав доступа для остальных */
    text_offset += sprintf(text_offset, "%s", (file_info->f_stat.st_mode & S_IROTH) ? "r" : "-");
    text_offset += sprintf(text_offset, "%s", (file_info->f_stat.st_mode & S_IWOTH) ? "w" : "-");
    if(file_info->f_stat.st_mode & S_IXOTH)
    {
        text_offset += sprintf(text_offset, "%s", (file_info->f_stat.st_mode & S_ISVTX) ? "t" : "x");
    }
    else
    {
        text_offset += sprintf(text_offset, "%s", (file_info->f_stat.st_mode & S_ISVTX) ? "T" : "-");
    }

    text_offset += sprintf(text_offset, " ");
    /* печать числа символьных ссылок */
    text_offset += sprintf(text_offset,
#ifdef ARM
        "%*d ",
#else
        "%*ld ",
#endif
        linkAlign, file_info->f_stat.st_nlink);
    /* печать пользователя и группы */
    text_offset += sprintf(text_offset, "%-*s %-*s ", userAlign,  getpwuid(file_info->f_stat.st_uid)->pw_name,
                                                      groupAlign, getgrgid(file_info->f_stat.st_gid)->gr_name);
    /* печать размера файла или его номеров, если это устройство */
    text_offset += (file_type == 'l' || file_type == 'd' || file_type == '-')
                ? sprintf(text_offset, "%*ld ",
                            sizeAlign > majorAlign + minorAlign + 2 ? sizeAlign : majorAlign + minorAlign + 2,
                            file_info->f_stat.st_size)
                : sprintf(text_offset, "%*u, %*u ", majorAlign, major(file_info->f_stat.st_rdev), minorAlign,
                            minor(file_info->f_stat.st_rdev));
    /* печать времени последнего изменения */
    local_time = *localtime(&(file_info->f_stat.st_mtim.tv_sec));
    text_offset += (local_time.tm_year == global_time.tm_year)
                ? strftime(text_offset, NAME_MAX, "%b %-2d %H:%M ", &local_time)
                : strftime(text_offset, NAME_MAX, "%b %-2d %-5Y ", &local_time);
    /* печать имени файла */
    text_offset += putGayText(text_offset, file_info->name, file_type, file_info->f_stat.st_mode);
    if (file_type == 'l')
    {
        file_type = getFileType(file_info->link);
        text_offset += putGayText(text_offset, file_info->link, file_type,
                                    (file_info->l_stat.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)));
    }

    fprintf(stdout, text);
}

int putGayText(char *out_text, const char *in_text, char file_type, __mode_t file_mode)
{
    int offset = 0;

    switch (file_type)
    {
        case 'b':
        case 'c':
            offset = sprintf(out_text, BASH_LYELLOW BASH_BGBLACK "%s" BASH_DEFAULT "\n", in_text);
            break;
        case 'd':
            if((file_mode & S_ISVTX) && (file_mode & S_IWOTH))
            {
                offset = sprintf(out_text, BASH_BLACK BASH_BGGREEN "%s" BASH_DEFAULT "/\n", in_text);
            }
            else if(file_mode & S_ISVTX)
            {
                offset = sprintf(out_text, BASH_WHITE BASH_BGBLUE "%s" BASH_DEFAULT "/\n", in_text);
            }
            else
            {
                offset = sprintf(out_text, BASH_LBLUE "%s" BASH_DEFAULT "/\n", in_text);
            }
            break;
        case 'l':
            offset = sprintf(out_text, BASH_LCYAN "%s" BASH_DEFAULT " -> ", in_text);
            break;
        case 'p':
            offset = sprintf(out_text, BASH_YELLOW BASH_BGBLACK "%s" BASH_DEFAULT "|\n", in_text);
            break;
        case 's':
            offset = sprintf(out_text, BASH_MAGENTA "%s" BASH_DEFAULT "=\n", in_text);
            break;
        case '-':
            if(file_mode & S_ISUID)
            {
                offset = sprintf(out_text, BASH_WHITE BASH_BGRED "%s" BASH_DEFAULT, in_text);
            }
            else if(file_mode & S_ISGID)
            {
                offset = sprintf(out_text, BASH_BLACK BASH_BGGRAY "%s" BASH_DEFAULT, in_text);
            }
            else if(file_mode & (S_IXUSR | S_IXGRP | S_IXOTH))
            {
                offset = sprintf(out_text, BASH_LGREEN "%s" BASH_DEFAULT, in_text);
            }
            else
            {
                offset = sprintf(out_text, "%s", in_text);
            }
            offset += sprintf(out_text + offset, "%s\n", (file_mode & (S_IXUSR | S_IXGRP | S_IXOTH)) ? "*" : "");
            break;
        default:
            break;
    }
    return offset;
}

char getFileType(const char *path)
{
    char file_type;
    struct stat f_stat;

    lstat(path, &f_stat);
    if      ((f_stat.st_mode & S_IFMT) == S_IFLNK)   file_type = 'l';
    else if ((f_stat.st_mode & S_IFMT) == S_IFCHR)   file_type = 'c';
    else if ((f_stat.st_mode & S_IFMT) == S_IFDIR)   file_type = 'd';
    else if ((f_stat.st_mode & S_IFMT) == S_IFBLK)   file_type = 'b';
    else if ((f_stat.st_mode & S_IFMT) == S_IFIFO)   file_type = 'p';
    else if ((f_stat.st_mode & S_IFMT) == S_IFSOCK)  file_type = 's';
    else                                             file_type = '-';

    return file_type;
}

void signalHandler(int signal, siginfo_t *signalInfo, void *userContext)
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
#ifndef ARM
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
#endif
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

void setAlignment(const file_info_st *file_info)
{
    char buff[NAME_MAX];
    int tmp;

    sprintf(buff,
#ifdef ARM
        "%d ",
#else
        "%ld ",
#endif
        file_info->f_stat.st_nlink);
    tmp = strlen(buff);
    linkAlign = linkAlign < tmp ? tmp : linkAlign;

    tmp = strlen(getpwuid(file_info->f_stat.st_uid)->pw_name);
    userAlign = userAlign < tmp ? tmp : userAlign;

    tmp = strlen(getgrgid(file_info->f_stat.st_gid)->gr_name);
    groupAlign = groupAlign < tmp ? tmp : groupAlign;

    sprintf(buff, "%ld", file_info->f_stat.st_size);
    tmp = strlen(buff);
    sizeAlign = sizeAlign < tmp ? tmp : sizeAlign;

    sprintf(buff, "%d", major(file_info->f_stat.st_rdev));
    tmp = strlen(buff);
    majorAlign = majorAlign < tmp ? tmp : majorAlign;

    sprintf(buff, "%d", minor(file_info->f_stat.st_rdev));
    tmp = strlen(buff);
    minorAlign = minorAlign < tmp ? tmp : minorAlign;
}
