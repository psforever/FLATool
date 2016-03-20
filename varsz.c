#include "varsz.h"

#include "util.h"

#include <stdlib.h>
#include <string.h>

char * read_var_string(FILE * fp)
{
  uint32_t size = 0;
  size_t readAmt = 0;

  if((readAmt = fread(&size, sizeof(size), 1, fp)) != 1)
    fatal("failed to read var string");

  // just a hack to prevent extra long var strings (which shouldnt happen)
  if(size > 200)
    fatal("var string looks too big (got %u)...", size);

  char * string = malloc(size+1);

  if((readAmt = fread(string, sizeof(char), size, fp)) != size)
    fatal("ran out of bytes when reading var string (got %u, needed %u)",
        readAmt,
        size);

  // discard one byte (NULL)
  fgetc(fp);

  string[size] = '\0';

  return string;
}

void write_var_string(FILE * fp, const char * str)
{
  uint32_t size = strlen(str);

  if(fwrite(&size, sizeof(size), 1, fp) != 1)
    fatal("failed to write var string size");

  if(fwrite(str, sizeof(char), size, fp) != size)
    fatal("failed to write var string contents");

  fputc('\0', fp);
}
