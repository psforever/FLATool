# FLATool
A simple C tool for extracting and creating FlatFile archives (.FAT) (pronounced flat-tool). This file format is used by PlanetSide 1 to store .DDS files (textures)

Based on original FAT extractor by itburnz.

## Compatibility
Tested on Cygwin and built using Mingw (GCC). Code is meant to be cross platform (hence `fs.c`).
License: BSD.

## Usage
There are two modes of operation, similar to gnu `tar`: extract and create. Extraction will extract out individual .DDS files from the .FAT archive. Create will combine multiple .DDS files in to one .FAT archive. See the usage statement for more details.

## Building
This will not build with Visual Studio. Tough luck.
You will need a real GCC or Mingw GCC. Cygwin GCC can be used, but you MUST NOT release binaries using the Cygwin compiler (cygwin1.dll required).

      $ make all

Or with an optional prefix (i.e for releasing binaries)

      $ PREFIX=i686-w64-mingw32- make all
