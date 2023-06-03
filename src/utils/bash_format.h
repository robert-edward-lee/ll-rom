/*
    Интерфейсная библиотека для форматирования вывода bash
*/

#ifndef __BASH_FORMAT_H
#define __BASH_FORMAT_H

// clang-format off
#define BASH_DEFAULT                        "\033[0m"               /* отмена всех опций */
#define BASH_BOLD                           "\033[1m"               /* жирный шрифт */
#define BASH_DBOLD                          "\033[2m"               /* полу-яркий цвет */
#define BASH_NBOLD                          "\033[22m"              /* установить нормальную интенсивность  */
#define BASH_UNDERLINE                      "\033[4m"               /* подчеркивание */
#define BASH_NUNDERLINE                     "\033[4m"               /* отменить подчеркивание */
#define BASH_BLINK                          "\033[5m"               /* мигающий */
#define BASH_NBLINK                         "\033[5m"               /* отменить мигание */
#define BASH_INVERSE                        "\033[7m"               /* реверс цветов */
#define BASH_NINVERSE                       "\033[7m"               /* отмена реверсов цветов */
/* цвет текста */
#define BASH_BLACK                          "\033[0;30m"            /*  */
#define BASH_BLUE                           "\033[0;34m"            /*  */
#define BASH_CYAN                           "\033[0;36m"            /*  */
#define BASH_GRAY                           "\033[0;37m"            /*  */
#define BASH_GREEN                          "\033[0;32m"            /*  */
#define BASH_MAGENTA                        "\033[0;35m"            /*  */
#define BASH_RED                            "\033[0;31m"            /*  */
#define BASH_YELLOW                         "\033[0;33m"            /*  */
/* цвет текста (жирный) */
#define BASH_DEF                            "\033[0;39m"            /*  */
#define BASH_DGRAY                          "\033[1;30m"            /*  */
#define BASH_LBLUE                          "\033[1;34m"            /*  */
#define BASH_LCYAN                          "\033[1;36m"            /*  */
#define BASH_LGREEN                         "\033[1;32m"            /*  */
#define BASH_LMAGENTA                       "\033[1;35m"            /*  */
#define BASH_LRED                           "\033[1;31m"            /*  */
#define BASH_LYELLOW                        "\033[1;33m"            /*  */
#define BASH_WHITE                          "\033[1;37m"            /*  */
/* цвет фона */
#define BASH_BGBLACK                        "\033[40m"              /*  */
#define BASH_BGBLUE                         "\033[44m"              /*  */
#define BASH_BGBROWN                        "\033[43m"              /*  */
#define BASH_BGCYAN                         "\033[46m"              /*  */
#define BASH_BGDEF                          "\033[49m"              /*  */
#define BASH_BGGRAY                         "\033[47m"              /*  */
#define BASH_BGGREEN                        "\033[42m"              /*  */
#define BASH_BGMAGENTA                      "\033[45m"              /*  */
#define BASH_BGRED                          "\033[41m"              /*  */

#endif /* __BASH_FORMAT_H */
