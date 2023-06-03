#ifndef __COMMON_H
#define __COMMON_H

#define DO_PRAGMA(x) _Pragma(#x)

#if(defined(__GNUC__) && ((__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6))) || defined(__clang__)
#define IGNORE_WARNING_PUSH(warning) DO_PRAGMA(GCC diagnostic push) DO_PRAGMA(GCC diagnostic ignored warning)
#define IGNORE_WARNING_POP() DO_PRAGMA(GCC diagnostic pop)
#else
#define IGNORE_WARNING_PUSH(warning)
#define IGNORE_WARNING_POP()
#endif // __GNUC__

#ifndef __clang__
#define IGNORE_WFORMAT_PUSH() IGNORE_WARNING_PUSH("-Wformat=")
#define IGNORE_WFORMAT_POP() IGNORE_WARNING_POP()
#else
#define IGNORE_WFORMAT_PUSH()
#define IGNORE_WFORMAT_POP()
#endif // __clang__

#endif // __COMMON_H
