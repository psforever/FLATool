#include "fs.h"

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>

#if defined(_WIN32) || defined(__CYGWIN32__)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

bool dir_exists(const char * path)
{
    DWORD dwAttrib = GetFileAttributes(path);

      return (dwAttrib != INVALID_FILE_ATTRIBUTES && 
                   (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

bool create_dir(const char * dir)
{
  BOOL res = CreateDirectoryA(dir, NULL);

  if(!res && GetLastError() != ERROR_ALREADY_EXISTS)
    return false;
  else
    return true;
}

#elif __linux__

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

bool dir_exists(const char * path)
{
  struct stat s;

  if(stat(path, &s) < 0) {
    if(ENOENT == errno) {
      return false;
    } else {
      fatal("stat failure");
    }
  } else {
    // it's a dir
    if(S_ISDIR(s.st_mode)) {
      return true;
    } else {
      return false;
    }
  }
}

bool create_dir(const char * dir)
{
  return mkdir(dir, 0777) == 0;
}

#else
#error "unsupported platform"

#endif

bool file_exists(const char * path)
{
  FILE * fp = fopen(path, "r");

  if(fp) {
    fclose(fp);
    return true;
  } else {
    return false;
  }
}

size_t file_size(const char * path)
{
  FILE * fp = fopen(path, "r");

  if(!fp)
    return 0;

  fseek(fp, 0, SEEK_END);

  size_t size = ftell(fp);

  fclose(fp);

  return size;
}

// XXX: this is not a real path combiner...
char * path_cat(const char * path1, const char * path2)
{
  size_t left = strlen(path1);
  size_t right = strlen(path2);
  size_t len = left+right+1;
  char * outPath = malloc(len+1);

  strncpy(outPath, path1, len);
  outPath[left] = PATH_SEP[0];
  strncpy(outPath+left+1, path2, len-left-1);

  outPath[len] = '\0';
  return outPath;
}

// XXX: blacklist == BAD
bool path_single_level(const char * path)
{
  if(strstr(path, ".."PATH_SEP) != NULL)
    return false;

  if(strstr(path, PATH_SEP) != NULL)
    return false;

  return true;
}

// XXX: this limits us to GCC
size_t get_files_in_dir(const char * name, char ** files[])
{
  DIR *d;
  struct dirent *dir;

  d = opendir(name);

  size_t allocSize = 20;
  size_t numFiles = 0;
  char ** results = malloc(allocSize*sizeof(char*));

  if(d) {
    while((dir = readdir(d)) != NULL) {
      //if(dir->d_type == DT_REG) {
      //}

      if(numFiles >= allocSize) {
        allocSize += 20;
        results = realloc(results, allocSize*sizeof(char*));
      }

      char * filename = dir->d_name;

      // skip fake files
      if(strcmp(filename, ".") != 0 && strcmp(filename, "..") != 0) {
        results[numFiles] = path_cat(name, filename);
        numFiles++;
      }
    }

    closedir(d);

    // trim allocation
    if(allocSize > numFiles)
      results = realloc(results, numFiles*sizeof(char*));

    *files = results;
    return numFiles;
  } else {
    *files = NULL;
    return 0;
  }
}
