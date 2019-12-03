#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <stdbool.h>
#include <stdlib.h>
#include <getopt.h>

#include "fs.h"
#include "util.h"
#include "flat.h"
#include "fdx.h"
#include "varsz.h"

#define VERSION_NAME "0.2"
#define PROG_NAME "FLATool"
#define FILE_TYPE "FlatFile"

// globals
char * exename = NULL;
int g_verbose = 0;
#define BUFFER_SIZE 1024*16

// prototypes
void create(char *flatName, bool force, char * ddsDir);
void extract(char *flatName, bool force);

typedef enum _mode {
  MODE_NONE = 0,
  MODE_EXTRACT,
  MODE_CREATE
} flat_mode_t;

// utilities
void usage(char * reason)
{
  if(strlen(reason))
    printf("Error: %s\n", reason);

  printf("usage: %s [options] [-x file.fat] [-c newfile.fat dds_dir"PATH_SEP"]\n", exename);
  printf("\n");
  printf("Examples:\n");
  printf("  %s -x dds_ui.fat # extracts files to dds_ui directory\n", exename);
  printf("  %s -c dds_new.fat dds_ui"PATH_SEP" # creates dds_new.fat + dds_new.fdx from source directory\n", exename);
  printf("\n");
  printf("Options:\n");
  printf("  -x  extract a "FILE_TYPE" to a directory of the same name\n");
  printf("  -c  create a "FILE_TYPE" from a set of .DDS files\n");
  printf("  -v  enable verbose mode\n");
  printf("  -f  force overwriting files\n");
  printf("  -h  you're looking at it\n");

  exit(1);
}

void banner()
{
  printf(PROG_NAME" v"VERSION_NAME"\n");
}

// main
int main(int argc, char * argv[])
{
  int opt = 0;

  if(argc > 0)
    exename = argv[0];
  else
    fatal("first argument must be the exe");

  banner();

  // opt variables
  bool force = false;
  flat_mode_t mode = MODE_NONE;

  while((opt = getopt(argc, argv, "hfxcv")) != -1) {
    switch(opt) {
      case 'h':
        usage("");
        break;
      case 'c':
        mode = MODE_CREATE;
        break;
      case 'x':
        mode = MODE_EXTRACT;
        break;
      case 'f':
        force = true;
        break;
      case 'v':
        g_verbose = 1;
        break;
      default:
        usage("");
    }
  }

  if(mode == MODE_NONE) {
    usage("a mode needs to be selected");
  }

  if(optind >= argc) {
    usage(FILE_TYPE" must be specified");
  }

  char * flatFilename = argv[optind];
  optind++;

  // create requires DDS files
  if(mode == MODE_CREATE) {
    if(optind >= argc) {
      usage("input DDS directory required");
    }

    char * ddsDir = argv[optind];
    optind++;

    // pass in the list of files
    create(flatFilename, force, ddsDir);

  } else if(mode == MODE_EXTRACT) {
    extract(flatFilename, force);
  }

  return 0;
}

// comparison function for sorting a list of strings
int sort_ascending(const void * l, const void * r)
{
  char * left = *(char **)l;
  char * right = *(char **)r;

  return planetside_strcmp(left, right);
}

void create(char *flatName, bool force, char * ddsDir)
{
  char * tmpFlatName = strdup(flatName);
  char * flatNameBase = get_extension(tmpFlatName);
  *flatNameBase = '\0'; // remove extension

  char * fdxName = string_cat(tmpFlatName, ".fdx");
  free(tmpFlatName);

  if(strcmp(flatName, fdxName) == 0)
    fatal("DDS and FDX names must be different");

  if(file_exists(flatName) && !force)
    fatal("not clobbering existing "FILE_TYPE" (use -f to force)");

  if(file_exists(fdxName) && !force)
    fatal("not clobbering existing FDX (%s) (use -f to force)", fdxName);

  size_t i = 0;

  if(!dir_exists(ddsDir)) {
    fatal("specified DDS directory %s does not exist", ddsDir);
  }

  // gather files from the passed in directory
  char ** files = NULL;
  size_t numFiles = get_files_in_dir_with_ext(ddsDir, &files, "dds");

  if(numFiles == 0)
    fatal("no files found in input directory");

  // sort the directory files
  qsort(files, numFiles, sizeof(char *), sort_ascending);

  // make sure the files passed in exist and are actually files
  for(i = 0; i < numFiles; i++) {
    if(!file_exists(files[i]))
      fatal("DDS file '%s' doesn't exist or isn't a file", files[i]);
  }

  struct fdx_entries fdxEntries;

  fdx_create(&fdxEntries);

  printf("Creating new "FILE_TYPE" %s from "PRIuSZT" files\n",
      flatName, numFiles);

  // Fix a weird display bug on cygwin (I hate it so much)
  fflush(stdout);

  FILE * pFile = fopen(flatName, "wb");

  struct flat_header header;
  memset(&header, 0, sizeof(header));

  memcpy(header.magic, FLAT_MAGIC, sizeof(header.magic));
  header.unk1 = 1;
  header.unk2 = 0;
  header.numDDS = numFiles;
  // I dont actually know the CRC algorithm that PlanetSide uses...luckily it
  // appears that the client doesn't verify the FLAT CRC! Yay!
  header.crc = 0xefbeadde;

  if(fwrite(&header, sizeof(header), 1, pFile) != 1)
    fatal("failed to write "FILE_TYPE" header");

  char * buffer = malloc(BUFFER_SIZE);

  for(i = 0; i < numFiles; i++) {
    uint32_t ddsSize = file_size(files[i]);
    char * base = basename(files[i], false); // get the basename but keep extension

    FILE * inFile = fopen(files[i], "rb");

    if(!inFile)
      fatal("failed to open '%s' for reading", files[i]);

    // write the variable string
    write_var_string(pFile, base);

    // write the file size
    if(fwrite(&ddsSize, sizeof(ddsSize), 1, pFile) != 1)
      fatal("failed to write the DDS size");

    // the offset to the beginning of the DDS file
    size_t offset = ftell(pFile);

    fdx_add(&fdxEntries, base, offset, ddsSize);

    // progress output
    if(g_verbose)
      printf("%s (size 0x%x, offset 0x"PRIxSZT")\n",
          base, ddsSize, offset);
    else
      fprintf(stderr, "\rPacking...%d%%", (int)((float)i/numFiles*100));

    // write the actual file
    size_t j = 0;
    size_t processStride = min(BUFFER_SIZE, ddsSize-j);

    for(j = 0; j < ddsSize; j += processStride) {
      processStride = min(BUFFER_SIZE, ddsSize-j);

      if(fread(buffer, sizeof(char), processStride, inFile) != processStride)
        fatal("unexpected end of DDS file");

      if(fwrite(buffer, sizeof(char), processStride, pFile) != processStride)
        fatal("failed to write out file chunk");
    }

    free(base);
    fclose(inFile);
  }

  if(!g_verbose)
    fprintf(stderr, "\n");

  if(!fdx_pack(fdxName, &fdxEntries))
    fatal("failed to write out the .FDX file");

  printf("Wrote new FDX %s from "PRIuSZT" files\n", fdxName, numFiles);

  free(buffer);
  fclose(pFile);
}

void extract(char *flatName, bool force)
{
  FILE * pFile = fopen(flatName, "rb");

  if(!pFile)
    fatal(FILE_TYPE" doesnt exist");

  struct flat_header header;
  size_t amt = fread(&header, sizeof(header), 1, pFile);

  if(amt != 1)
    fatal("failed to read "FILE_TYPE" header");

  if(header.magic[0] != FLAT_MAGIC[0] ||
     header.magic[1] != FLAT_MAGIC[1] ||
     header.magic[2] != FLAT_MAGIC[2] ||
     header.magic[3] != FLAT_MAGIC[3]) {

    fatal("invalid "FILE_TYPE" magic");
  }

  printf(FILE_TYPE" has %u DDS files\n", header.numDDS);

  char * base = basename(flatName, true);

  if(dir_exists(base) && !force)
    fatal("not clobbering directory %s"PATH_SEP" (use -f to force)", base);

  if(!dir_exists(base) && !create_dir(base))
    fatal("failed to create output directory");

  printf("Extracting %s to directory %s"PATH_SEP"\n", flatName, base);
  fflush(stdout);

  char * buffer = malloc(BUFFER_SIZE);
  size_t i;

  for(i = 0; i < header.numDDS; i++) {
    char * name = read_var_string(pFile);
    uint32_t ddsSize = 0;

    if(fread(&ddsSize, sizeof(ddsSize), 1, pFile) != 1)
      fatal("failed to read DDS file size");

    // make sure there aren't any slashes before trusting the path
    if(!path_single_level(name))
      fatal("unexpected multi-level path");

    // offset of the DDS file
    size_t offset = ftell(pFile);

    // progress output
    if(g_verbose)
      printf("%s (size 0x%x, offset 0x"PRIxSZT")\n",
          name, ddsSize, offset);
    else
      fprintf(stderr, "\rExtracting...%d%%", (int)((float)i/header.numDDS*100));

    // read and write the DDS file
    char * path = path_cat(base, name);
    FILE * outFile = fopen(path, "wb");

    size_t j = 0;
    size_t processStride = min(BUFFER_SIZE, ddsSize-j);

    for(j = 0; j < ddsSize; j += processStride) {
      processStride = min(BUFFER_SIZE, ddsSize-j);

      if(fread(buffer, sizeof(char), processStride, pFile) != processStride)
        fatal("unexpected end of "FILE_TYPE" file");

      if(fwrite(buffer, sizeof(char), processStride, outFile) != processStride)
        fatal("failed to write out file chunk");
    }

    fclose(outFile);
    free(name);
    free(path);
  }

  if(!g_verbose)
    fprintf(stderr, "\n");

  free(buffer);
  free(base);
  fclose(pFile);
}
