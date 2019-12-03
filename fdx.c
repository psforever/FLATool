#include "fdx.h"

#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "varsz.h"
#include "util.h"

bool fdx_parse(const char * filename, struct fdx_entries * entries)
{
  assert(entries);

  FILE * fp = fopen(filename, "rb");

  if(!fp)
    return false;

  uint32_t nEntries = 0;
  struct fdx_entry ** readEntries = NULL;

  if(fread(&nEntries, sizeof(nEntries), 1, fp) != 1)
    goto fail;

  readEntries = calloc(nEntries, sizeof(struct fdx_entry *));

  if(!readEntries)
    goto fail;

  size_t i;
  for(i = 0; i < nEntries; i++) {
    struct fdx_entry * entry = calloc(1, sizeof(struct fdx_entry));

    entry->name = read_var_string(fp);

    if (fread(&entry->dds_offset, sizeof(entry->dds_offset), 1, fp) != 1)
      fatal("unable to read the DDS offset in FDX entry "PRIuSZT"\n", i);

    if (fread(&entry->dds_size, sizeof(entry->dds_size), 1, fp) != 1)
      fatal("unable to read the DDS size in FDX entry "PRIuSZT"\n", i);

    readEntries[i] = entry;
  }

  entries->numEntries = nEntries;
  entries->entries = readEntries;

  fclose(fp);

  return true;

fail:
  if(readEntries)
    free(readEntries);

  fclose(fp);

  return false;
}

bool fdx_create(struct fdx_entries * entries)
{
  assert(entries);

  entries->numEntries = 0;
  entries->entries = NULL;

  return true;
}

bool fdx_add(struct fdx_entries * entries, const char * name, uint32_t dds_offset, uint32_t dds_size)
{
  assert(entries);

  entries->entries = realloc(entries->entries, sizeof(struct fdx_entry *) * (entries->numEntries + 1));

  if (!entries->entries)
    return false;

  struct fdx_entry * entry = calloc(1, sizeof(struct fdx_entry));

  if (!entry)
    return false;

  entry->name = strdup(name);
  entry->dds_offset = dds_offset;
  entry->dds_size = dds_size;

  entries->entries[entries->numEntries] = entry;

  entries->numEntries += 1;

  return true;
}

bool fdx_pack(const char * filename, struct fdx_entries * entries)
{
  assert(entries);

  FILE * fp = fopen(filename, "wb");

  if(!fp)
    return false;

  fwrite(&entries->numEntries, sizeof(entries->numEntries), 1, fp);

  size_t i;
  for(i = 0; i < entries->numEntries; i++) {
    struct fdx_entry * e = entries->entries[i];

    write_var_string(fp, e->name);
    fwrite(&e->dds_offset, sizeof(e->dds_offset), 1, fp);
    fwrite(&e->dds_size, sizeof(e->dds_size), 1, fp);
  }

  fclose(fp);

  return true;
}

void fdx_free(struct fdx_entries * entries)
{
  if(!entries)
    return;

  size_t i = 0;

  for(i = 0; i < entries->numEntries; i++) {
    struct fdx_entry * e = entries->entries[i];

    if(e->name)
      free(e->name);

    free(e);
  }

  free(entries->entries);

  entries->numEntries = 0;
  entries->entries = NULL;
}
