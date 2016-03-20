#ifndef FLAT_H
#define FLAT_H

#include <stdint.h>

extern const char * FLAT_MAGIC;
extern const char * DDS_MAGIC;

#pragma pack(push)
#pragma pack(1)
struct flat_header
{
  char magic[4];
  uint32_t unk1;
  uint32_t unk2;
  uint32_t numDDS;
  uint32_t crc;
};
#pragma pack(pop)

#endif
