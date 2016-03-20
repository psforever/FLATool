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

#ifdef __CYGWIN32__
    if(c == '\\' || c == '/')
      break;
#else
    if(c == PATH_SEP[0])
      break;
#endif

    start = riter;
    riter--;
  }

  if(extension) {
    char * newPtr = strstr(start, ".");
    char * ptr = NULL;

    // find the last .
    while(newPtr) {
      ptr = newPtr;
      newPtr = strstr(newPtr+1, ".");
    }

    if(ptr == NULL)
      return strdup(start);

    if(ptr == start)
      return strdup("");

    char * copy = strdup(start);
    size_t index = ptr-start;

    copy[index] = '\0';

    return copy;
  } else {
    return strdup(start);
  }
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
    char * newPtr = strstr(path, ".");
    char * ptr = NULL;

    // find the last .
    while(newPtr) {
      ptr = newPtr;
      newPtr = strstr(newPtr+1, ".");
    }

    if(ptr == NULL)
      return path+strlen(path);

    return ptr;
}
