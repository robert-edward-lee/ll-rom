#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "bash_format.h"
#include "common.h"
#include "readlink.h"
#include "scandir.h"
#include "service_printing.h"
#include "signal_handling.h"
#include "version.h"

#ifndef _WIN32
#include <grp.h>
#include <linux/limits.h>
#include <linux/version.h>
#include <pwd.h>
#include <sys/sysmacros.h>
#include <sys/xattr.h>
#else
#define lstat stat
#define NAME_MAX FILENAME_MAX
#endif

#define MAX_REC_COUNT 40
#define max(a, b) ((a) < (b) ? (b) : (a))

typedef enum {
    NOT_THE_SAME_DIR = 1 << 1,
    DESCRIPTION = ~0,
} opts_et;

typedef struct {
    /* Имя файла */
    char name[PATH_MAX];
    /* Имя куда ссылается ссылка, если файл это ссылка */
    char link[PATH_MAX];
    /* Данные файла */
    struct stat f_stat;
    /* Данные куда ссылается ссылка, если файл это ссылка */
    struct stat l_stat;
} file_info_st;

char dir_path[PATH_MAX] = ".";

const char ABOUT[] = "ll v" VERSION "\n"
                     "Usage:\n"
                     "\tll -h       - call help\n"
                     "\tll [DIR]    - show files list\n";

int link_align = 0;
int user_align = 0;
int group_align = 0;
int size_align = 0;
int major_align = 0;
int minor_align = 0;

int current_year;

opts_et get_opts(int argc, const char *argv[], opts_et *opts);
int dirent_comparator(const struct dirent **lhs, const struct dirent **rhs);
struct stat handle_link(char *path);
void print_stat(const file_info_st *file_info);
int put_colored_text(char *out_text, const char *in_text, char file_type, mode_t file_mode);
char get_file_type(const char *path);
void update_alignments(const file_info_st *file_info);

int main(int argc, const char *argv[]) {
    opts_et opts;
    struct dirent **name_list;
    int names, i;
    file_info_st *file_list;

    register_signal_handler();

    if(get_opts(argc, argv, &opts) == -1) {
        PrintWarn("Cannot get opts");
        return -1;
    }
    if(opts == DESCRIPTION) {
        fputs(ABOUT, stdout);
        return 0;
    }
    if(opts & NOT_THE_SAME_DIR) {
        chdir(argv[1]);
    }

    current_year = localtime(&(time_t){time(NULL)})->tm_year;

    getcwd(dir_path, PATH_MAX);
    names = scandir(dir_path, &name_list, NULL, dirent_comparator);
    if(names == -1) {
        PrintErr("Cannot scan the directory: %s", dir_path);
        return -1;
    }

    file_list = calloc(names, sizeof(*file_list));
    for(i = 0; i < names; i++) {
        /* Заполняем инфу о файле */
        strncpy(file_list[i].name, name_list[i]->d_name, PATH_MAX);
        lstat(file_list[i].name, &file_list[i].f_stat);
        /* Если это символьная ссылка, то получаем данные куда ссылка ведёт */
        if(get_file_type(file_list[i].name) == 'l') {
            strncpy(file_list[i].link, file_list[i].name, PATH_MAX);
            file_list[i].l_stat = handle_link(file_list[i].link);
        }
        update_alignments(&file_list[i]);
        free(name_list[i]);
    }
    free(name_list);

    for(i = 0; i < names; i++) {
        print_stat(&file_list[i]);
    }

    free(file_list);
    return 0;
}

opts_et get_opts(int argc, const char *argv[], opts_et *opts) {
    int ret = 0;
    struct stat dir_stat;

    *opts = 0;
    if(argc == 2) {
        if(!strncmp(argv[1], "-h", sizeof("-h"))) {
            *opts |= DESCRIPTION;
        } else if((stat(argv[1], &dir_stat) == 0) && (dir_stat.st_mode & S_IFDIR)) {
            *opts |= NOT_THE_SAME_DIR;
        } else {
            PrintErr("Wrong directory: %s", argv[1]);
            ret = -1;
        }
    }
    return ret;
}

int dirent_comparator(const struct dirent **lhs, const struct dirent **rhs) {
    int n = NAME_MAX;
    uint8_t *l = (void *)(**lhs).d_name;
    uint8_t *r = (void *)(**rhs).d_name;
    /* избавляемся от пунктуации в начале слова, а если всё слово состоит из пунктуации, то возвращаем указатель на
    начало */
    for(; *l && ispunct(*l); l++) {}
    if(!*l) {
        l = (void *)(**lhs).d_name;
    }
    for(; *r && ispunct(*r); r++) {}
    if(!*r) {
        r = (void *)(**rhs).d_name;
    }

    for(; *l && *r && n && tolower(*l) == tolower(*r); l++, r++, n--) {}
    return tolower(*l) - tolower(*r);
}

struct stat handle_link(char *path) {
    static int rec_count = 0;
    struct stat ret_stat = {0};
    char link[PATH_MAX] = {0};

    DbgPrint("%s -> ", path);
    if(rec_count == MAX_REC_COUNT) {
        PrintWarn("Too many nested symbolic link: %s", path);
        exit(EXIT_FAILURE);
    }

    if(get_file_type(path) == 'l') {
        readlink(path, link, PATH_MAX);
        sprintf(path, "%s", link);
        memset(link, 0, PATH_MAX);
        rec_count++;
        handle_link(path);
    }

    lstat(path, &ret_stat);
    rec_count = 0;
    return ret_stat;
}

void print_stat(const file_info_st *file_info) {
    char file_type;
    char text[PATH_MAX];
    char *text_offset = text;
    struct tm *file_timestamp;

    /* печать типа файла */
    file_type = get_file_type(file_info->name);
    text_offset += sprintf(text_offset, "%c", file_type);
    /* печать прав доступа пользователя */
    text_offset += sprintf(text_offset, "%s", (file_info->f_stat.st_mode & S_IRUSR) ? "r" : "-");
    text_offset += sprintf(text_offset, "%s", (file_info->f_stat.st_mode & S_IWUSR) ? "w" : "-");
    if(file_info->f_stat.st_mode & S_IXUSR) {
        text_offset += sprintf(text_offset,
                               "%s",
#ifndef _WIN32
                               (file_info->f_stat.st_mode & S_ISUID) ? "s" :
#endif
                                                                     "x");
    } else {
        text_offset += sprintf(text_offset,
                               "%s",
#ifndef _WIN32
                               (file_info->f_stat.st_mode & S_ISUID) ? "S" :
#endif
                                                                     "-");
    }
    /* печать прав доступа группы */
    text_offset += sprintf(text_offset, "%s", (file_info->f_stat.st_mode & S_IRGRP) ? "r" : "-");
    text_offset += sprintf(text_offset, "%s", (file_info->f_stat.st_mode & S_IWGRP) ? "w" : "-");
    if(file_info->f_stat.st_mode & S_IXGRP) {
        text_offset += sprintf(text_offset,
                               "%s",
#ifndef _WIN32
                               (file_info->f_stat.st_mode & S_ISGID) ? "s" :
#endif
                                                                     "x");
    } else {
        text_offset += sprintf(text_offset,
                               "%s",
#ifndef _WIN32
                               (file_info->f_stat.st_mode & S_ISGID) ? "S" :
#endif
                                                                     "-");
    }
    /* печать прав доступа для остальных */
    text_offset += sprintf(text_offset, "%s", (file_info->f_stat.st_mode & S_IROTH) ? "r" : "-");
    text_offset += sprintf(text_offset, "%s", (file_info->f_stat.st_mode & S_IWOTH) ? "w" : "-");
    if(file_info->f_stat.st_mode & S_IXOTH) {
        text_offset += sprintf(text_offset,
                               "%s",
#ifndef _WIN32
                               (file_info->f_stat.st_mode & S_ISVTX) ? "t" :
#endif
                                                                     "x");
    } else {
        text_offset += sprintf(text_offset,
                               "%s",
#ifndef _WIN32
                               (file_info->f_stat.st_mode & S_ISVTX) ? "T" :
#endif
                                                                     "-");
    }
#ifndef _WIN32
    text_offset += sprintf(text_offset, "%s", listxattr(file_info->name, NULL, 0) ? "+" : " ");
#endif
    /* печать числа символьных ссылок */
    text_offset += sprintf(text_offset, "%*ld ", link_align, file_info->f_stat.st_nlink);
    /* печать пользователя и группы */
#ifndef _WIN32
    text_offset += sprintf(text_offset,
                           "%-*s %-*s ",
                           user_align,
                           getpwuid(file_info->f_stat.st_uid)->pw_name,
                           group_align,
                           getgrgid(file_info->f_stat.st_gid)->gr_name);
#endif
/* печать размера файла или его номеров, если это устройство */
#ifndef _WIN32
    if(file_type == 'l' || file_type == 'd' || file_type == '-') {
#endif
        text_offset += sprintf(text_offset, "%*ld ", size_align, file_info->f_stat.st_size);
#ifndef _WIN32
    } else {
        text_offset += sprintf(text_offset,
                               "%*u, %*u ",
                               major_align,
                               major(file_info->f_stat.st_rdev),
                               minor_align,
                               minor(file_info->f_stat.st_rdev));
    }
#endif
    /* печать времени последнего изменения */
#ifdef _WIN32
    file_timestamp = localtime(&file_info->f_stat.st_mtime);
#else
    file_timestamp = localtime(&file_info->f_stat.st_mtim.tv_sec);
#endif

    IGNORE_WFORMAT_PUSH()
    text_offset += (file_timestamp->tm_year == current_year)
                     ? strftime(text_offset, NAME_MAX, "%b %-2d %H:%M ", file_timestamp)
                     : strftime(text_offset, NAME_MAX, "%b %-2d %-5Y ", file_timestamp);
    IGNORE_WFORMAT_POP()
    /* печать имени файла */
    text_offset += put_colored_text(text_offset, file_info->name, file_type, file_info->f_stat.st_mode);
    if(file_type == 'l') {
        text_offset += put_colored_text(text_offset,
                                        file_info->link,
                                        get_file_type(file_info->link),
                                        file_info->l_stat.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH));
    }

    fputs(text, stdout);
}

int put_colored_text(char *out_text, const char *in_text, char file_type, mode_t file_mode) {
    int offset = 0;

    switch(file_type) {
        case 'b':
        case 'c':
            offset = sprintf(out_text, BASH_LYELLOW BASH_BGBLACK "%s" BASH_DEFAULT "\n", in_text);
            break;
        case 'd':
            if(
#ifndef _WIN32
                (file_mode & S_ISVTX) &&
#endif
                (file_mode & S_IWOTH)) {
                offset = sprintf(out_text, BASH_BLACK BASH_BGGREEN "%s" BASH_DEFAULT "/\n", in_text);
#ifndef _WIN32
            } else if(file_mode & S_ISVTX) {
                offset = sprintf(out_text, BASH_WHITE BASH_BGBLUE "%s" BASH_DEFAULT "/\n", in_text);
#endif
            } else {
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
#ifndef _WIN32
            if(file_mode & S_ISUID) {
                offset = sprintf(out_text, BASH_WHITE BASH_BGRED "%s" BASH_DEFAULT, in_text);
            } else if(file_mode & S_ISGID) {
                offset = sprintf(out_text, BASH_BLACK BASH_BGGRAY "%s" BASH_DEFAULT, in_text);
            } else
#endif
                if(file_mode & (S_IXUSR | S_IXGRP | S_IXOTH)) {
                offset = sprintf(out_text, BASH_LGREEN "%s" BASH_DEFAULT, in_text);
            } else {
                offset = sprintf(out_text, "%s", in_text);
            }
            offset += sprintf(out_text + offset, "%s\n", (file_mode & (S_IXUSR | S_IXGRP | S_IXOTH)) ? "*" : "");
            break;
        default:
            break;
    }
    return offset;
}

char get_file_type(const char *path) {
    char file_type;
    struct stat f_stat;

    lstat(path, &f_stat);
    switch(f_stat.st_mode & S_IFMT) {
#ifndef _WIN32
        case S_IFLNK:
            file_type = 'l';
            break;
        case S_IFSOCK:
            file_type = 's';
            break;
#endif
        case S_IFCHR:
            file_type = 'c';
            break;
        case S_IFDIR:
            file_type = 'd';
            break;
        case S_IFBLK:
            file_type = 'b';
            break;
        case S_IFIFO:
            file_type = 'p';
            break;
        case S_IFREG:
            file_type = '-';
            break;
        default:
            file_type = '\0';
            break;
    }

    return file_type;
}

void update_alignments(const file_info_st *file_info) {
    char buff[NAME_MAX];
    int tmp;

    sprintf(buff, "%ld ", file_info->f_stat.st_nlink);
    tmp = strlen(buff);
    link_align = max(link_align, tmp);
#ifndef _WIN32
    tmp = strlen(getpwuid(file_info->f_stat.st_uid)->pw_name);
    user_align = max(user_align, tmp);

    tmp = strlen(getgrgid(file_info->f_stat.st_gid)->gr_name);
    group_align = max(group_align, tmp);
#endif
    sprintf(buff, "%ld", file_info->f_stat.st_size);
    tmp = strlen(buff);
    size_align = max(size_align, tmp);
#ifndef _WIN32
    sprintf(buff, "%d", major(file_info->f_stat.st_rdev));
    tmp = strlen(buff);
    major_align = max(major_align, tmp);

    sprintf(buff, "%d", minor(file_info->f_stat.st_rdev));
    tmp = strlen(buff);
    minor_align = max(minor_align, tmp);
#endif
    size_align = max(size_align, major_align + minor_align + 2);
}
