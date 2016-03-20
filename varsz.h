#ifndef FLATOOL_VARSZ_H
#define FLATOOL_VARSZ_H

#include <stdio.h>

char * read_var_string(FILE * fp);
void write_var_string(FILE * fp, const char * str);

#endif
