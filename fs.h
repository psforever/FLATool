#ifndef FS_H
#define FS_H

#include <stdbool.h>
#include <stdlib.h>

#if defined(_WIN32) || defined(__CYGWIN32__)

#define PATH_SEP "\\"

#elif __linux__

#define PATH_SEP "/"

#else
#error "unsupported platform"
#endif

bool file_exists(const char * path);
bool dir_exists(const char * path);
size_t file_size(const char * path);
bool create_dir(const char * dir);
char * path_cat(const char * path1, const char * path2);
bool path_single_level(const char * path);
size_t get_files_in_dir(const char * name, char ** files[]);

#endif
