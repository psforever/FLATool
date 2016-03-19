#ifndef UTIL_H
#define UTIL_H

#include <stdbool.h>

void fatal(char * msg, ...);
char * basename(const char * path, bool extension);

#define min(x, y) (((x) < (y)) ? (x) : (y))
#endif
