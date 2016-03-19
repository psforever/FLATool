#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <stdbool.h>
#include <stdlib.h>
#include <getopt.h>

#include "fs.h"
#include "util.h"
#include "flat.h"

#define VERSION_NAME "0.1"
#define PROG_NAME "FLATool"
#define FILE_TYPE "FlatFile"

// globals
char * exename = NULL;
int g_verbose = 0;
#define BUFFER_SIZE 1024*16

// prototypes
void create(char *flatName, bool force, char * files[], size_t numFiles);
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

  printf("usage: %s [options] [-x file.fat] [-c newfile.fat [dds_dir] [file1.dds file2.dds ...]]\n", exename);
  printf("\n");
  printf("Examples:\n");
  printf("  %s -x dds_ui.fat # extracts files to dds_ui directory\n", exename);
  printf("  %s -c dds_new.fat dds_ui"PATH_SEP" # creates dds_new.fat from source directory\n", exename);
  printf("  %s -fvc dds_new.fat dds_ui"PATH_SEP"*.dds # creates dds_new.fat from the globbed .DDS files\n", exename);
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
      usage("input DDS files required");
    }

    char ** files = argv+optind;

    // pass in the list of files
    create(flatFilename, force, files, argc-optind);

  } else if(mode == MODE_EXTRACT) {
    extract(flatFilename, force);
  }

  return 0;
}

void create(char *flatName, bool force, char * files[], size_t numFiles)
{
  if(file_exists(flatName) && !force)
    fatal("not clobbering existing "FILE_TYPE" (use -f to force)");

  size_t i = 0;

  if(dir_exists(files[0])) {
    numFiles = get_files_in_dir(files[0], &files);
  }

  // make sure the files passed in exist
  for(i = 0; i < numFiles; i++)
    if(!file_exists(files[i]))
      fatal("file '%s' doesn't exist", files[i]);

  printf("Creating new "FILE_TYPE" %s from "PRIuSZT" files\n", flatName, numFiles);

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
    size_t offset = ftell(pFile);
    size_t ddsSize = file_size(files[i]);
    char * base = basename(files[i], false); // get the basename but keep extension

    FILE * inFile = fopen(files[i], "rb");

    if(!inFile)
      fatal("failed to open '%s' for reading", files[i]);

    // progress output
    if(g_verbose)
      printf("%s (size 0x"PRIxSZT" offset 0x"PRIxSZT")\n",
          base, ddsSize, offset);
    else
      fprintf(stderr, "\rExtracting...%d%%", (int)((float)i/numFiles*100));

    // write the variable string
    write_var_string(pFile, base);

    // write the file size
    if(fwrite(&ddsSize, sizeof(ddsSize), 1, pFile) != 1)
      fatal("failed to write the DDS size");

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
    printf("\n");

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

  if(!create_dir(base))
    fatal("failed to create output directory");

  printf("Extracting %s to directory %s"PATH_SEP"\n", flatName, base);

  char * buffer = malloc(BUFFER_SIZE);
  size_t i;

  for(i = 0; i < header.numDDS; i++) {
    size_t offset = ftell(pFile);
    char * name = read_var_string(pFile);
    uint32_t ddsSize = 0;

    if(fread(&ddsSize, sizeof(ddsSize), 1, pFile) != 1)
      fatal("failed to read DDS file size");

    // make sure there aren't any slashes before trusting the path
    if(!path_single_level(name))
      fatal("unexpected multi-level path");

    // progress output
    if(g_verbose)
      printf("%s (size 0x"PRIxSZT", offset 0x"PRIxSZT")\n",
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
    printf("\n");

  free(buffer);
  free(base);
  fclose(pFile);
}
