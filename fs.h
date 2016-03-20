#ifndef FS_H
#define FS_H

#include <stdbool.h>
#include <stdlib.h>

#include "compat.h"

#ifdef PLATFORM_WINDOWS
#  define PATH_SEP "\\"
#elif defined(PLATFORM_LINUX)
#  define PATH_SEP "/"
#endif

bool file_exists(const char * path);
bool dir_exists(const char * path);
size_t file_size(const char * path);
bool create_dir(const char * dir);
char * path_cat(const char * path1, const char * path2);
bool path_single_level(const char * path);
size_t get_files_in_dir(const char * name, char ** files[]);
size_t get_files_in_dir_with_ext(const char * name, char ** files[], const char * ext);

#endif
