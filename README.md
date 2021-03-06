# FLATool [![Build Status](https://travis-ci.org/psforever/FLATool.svg?branch=master)](https://travis-ci.org/psforever/FLATool)
A simple C tool for extracting and creating FlatFile archives (.FAT) (pronounced flat-tool). This file format is used by PlanetSide 1 to store .DDS files (textures)

Based on original FAT extractor by itburnz.

## Compatibility
Tested on Cygwin and built using Mingw (GCC). Code is meant to be cross platform (hence `fs.c`).
License: BSD.

## Usage
There are two modes of operation, similar to gnu `tar`: extract and create. Extraction will extract out individual .DDS files from the .FAT archive. Create will combine multiple .DDS files in to one .FAT archive. See the usage statement for more details.

```
usage: ./FLATool [options] [-x file.fat] [-c newfile.fat dds_dir/]

Examples:
  ./FLATool -x dds_ui.fat # extracts files to dds_ui directory
  ./FLATool -c dds_new.fat dds_ui/ # creates dds_new.fat + dds_new.fdx from source directory
  ./FLATool -fc dds_ui.fat dds_ui/ # repacks dds_ui.fat + dds_ui.fdx in-place (make a copy before!)

Options:
  -x  extract a FlatFile to a directory of the same name
  -c  create a FlatFile from a set of .DDS files
  -v  enable verbose mode
  -f  force overwriting files
  -h  you're looking at it
```

## Building
This will not currently build with Visual Studio due to POSIX dependencies.
You will need a real GCC or Mingw GCC. Cygwin GCC can be used, but you MUST NOT release binaries using the Cygwin compiler (cygwin1.dll required).

      $ make all

Or with an optional prefix (i.e for releasing binaries)

      $ PREFIX=i686-w64-mingw32- make all
