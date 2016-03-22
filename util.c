#include "util.h"

#include "fs.h"

#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void fatal(char * msg, ...)
{
  va_list list;
  va_start(list, msg);

  printf("fatal: ");
  vprintf(msg, list);
  printf("\n");

  exit(1);
}

// Get's the last path element
// Finds the last period (.) and returns every thing before it if extension is true
char * basename(const char * path, bool extension)
{
  if(strlen(path) == 0)
    return strdup("");

  const char * start = path+strlen(path);
  const char * riter = start-1; // skip null

  while(riter >= path) {
    char c = *riter;

  // windows can have either / or \ in the path name
#ifdef PLATFORM_WINDOWS
    if(c == '\\' || c == '/')
      break;
#elif defined(PLATFORM_LINUX)
    if(c == '/')
      break;
#endif

    start = riter;
    riter--;
  }

  char * copy = strdup(start);

  if(extension) {
    char * ptr = get_extension(copy);

    *ptr = '\0';
  }

  return copy;
}

char * string_cat(const char * l, const char * r)
{
  size_t lSz = strlen(l);
  size_t rSz = strlen(r);

  char * newString = malloc(lSz+rSz+1);

  strncpy(newString, l, lSz+rSz);
  strncpy(newString+lSz, r, rSz);

  newString[lSz+rSz] = '\0';

  return newString;
}

char * get_extension(char * path)
{
    char * ptr = strrchr(path, '.');

    // if not found, return pointer to end of string (no extension)
    if(ptr == NULL)
      return path+strlen(path);

    return ptr;
}
