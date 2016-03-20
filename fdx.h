#ifndef FLATOOL_FDX_H
#define FLATOOL_FDX_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

struct fdx_entry {
  char * name;
  uint32_t dds_offset;
  uint32_t texture_type;
};

struct fdx_entries {
  size_t numEntries;
  struct fdx_entry ** entries;
};

bool fdx_parse(const char * filename, struct fdx_entries * entries);
bool fdx_pack(const char * filename, struct fdx_entries * entries);
void fdx_free(struct fdx_entries * entries);

#endif

