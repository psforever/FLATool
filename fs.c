#include "fs.h"

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>

#include "util.h"

#ifdef PLATFORM_WINDOWS

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

bool file_exists(const char * path)
{
  int res = GetFileAttributes(path);

  if(res < 0) {
    if(GetLastError() == ERROR_FILE_NOT_FOUND)
      return false;
  }

  return true;
}

#elif defined(PLATFORM_LINUX)

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
      return false;
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

#endif

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
// This is quite broken and insecure
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

size_t get_files_in_dir_with_ext(const char * name, char ** files[], const char * ext)
{
  char ** localFiles = NULL;
  size_t amt = get_files_in_dir(name, &localFiles);

  if(amt == 0)
    return amt;

  // create a new list of files matching what we want
  char ** newFiles = calloc(amt, sizeof(char*));

  size_t i = 0, j = 0;

  for(i = 0; i < amt; i++) {

    if(strlen(localFiles[i]) <= strlen(ext)) {
      free(localFiles[i]);
      continue;
    }

    // find last .
    char * newPtr = strstr(localFiles[i], ".");
    char * ptr = NULL;

    // find the last .
    while(newPtr) {
      ptr = newPtr;
      newPtr = strstr(newPtr+1, ".");
    }

    if(!ptr) {
      free(localFiles[i]);
      continue;
    }

    // +1 to skip dot
    if(strcmp(ptr+1, ext) == 0) {
      newFiles[j++] = localFiles[i];
    } else {
      free(localFiles[i]);
    }
  }

  free(localFiles);

  *files = newFiles;
  return j;
}

