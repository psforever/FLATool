#ifndef UTIL_H
#define UTIL_H

#include <stdbool.h>
#include <inttypes.h>

#include "compat.h"

void fatal(char * msg, ...);
char * basename(const char * path, bool extension);
char * string_cat(const char * l, const char * r);
char * get_extension(char * path);

#define min(x, y) (((x) < (y)) ? (x) : (y))


#ifdef PLATFORM_WINDOWS

#ifdef PLATFORM_BITS_64
#define PRIuSZT "%lu"
#define PRIxSZT "%lx"
#else
#define PRIuSZT "%u"
#define PRIxSZT "%x"
#endif // PLATFORM_BITS_64

#elif defined(PLATFORM_LINUX)

#define PRIuSZT "%zu"
#define PRIxSZT "%zx"

#endif // PLATFORM_BITS_64

#endif // PLATFORM_WINDOWS
