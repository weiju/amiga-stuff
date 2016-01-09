/* ilbm.c - Universal IFF ILBM handling module

   This file is part of amiga30yrs.

   amiga30yrs is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   amiga30yrs is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with amiga30yrs.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ilbm.h"

#ifdef LITTLE_ENDIAN
#include <byteswap.h>
#endif

#define IFF_HEADER_SIZE 12
#define CHUNK_HEADER_SIZE 8

BitMapHeader *read_BMHD(FILE *fp, int datasize)
{
  BitMapHeader *header;
  int bytes_read;

  header = malloc(sizeof(BitMapHeader));
  bytes_read = fread(header, sizeof(char), datasize, fp);

#ifdef LITTLE_ENDIAN
  header->x = __bswap_16(header->x);
  header->y = __bswap_16(header->y);
  header->w = __bswap_16(header->w);
  header->h = __bswap_16(header->h);
  header->transparentColor = __bswap_16(header->transparentColor);
  header->pageWidth = __bswap_16(header->pageWidth);
  header->pageHeight = __bswap_16(header->pageHeight);
#endif

  /*
  printf("x: %d y: %d w: %d h: %d\n",
         header.x, header.y, header.w, header.h);
  printf("# planes: %d\n", header.nPlanes);
  printf("# transp col: %d\n", header.transparentColor);
  printf("pagewidth: %d pageheight: %d\n", header.pageWidth, header.pageHeight);
  */
  if (header->compression == cmpNone) puts("no compression");
  else if (header->compression == cmpByteRun1) puts("Byte Run 1 compression");
  else puts("unknown compression");
  return header;
}

ColorRegister *read_CMAP(FILE *fp, int datasize)
{
  int num_colors = datasize / 3, bytes_read;
  ColorRegister *colors;

  colors = malloc(datasize);
  bytes_read = fread(colors, sizeof(ColorRegister), num_colors, fp);
  return colors;
}

ULONG read_CAMG(FILE *fp, int datasize)
{
  ULONG flags, bytes_read;
  bytes_read = fread(&flags, sizeof(ULONG), 1, fp);
#ifdef LITTLE_ENDIAN
  flags = __bswap_32(flags);
#endif
  if (flags & HAM) puts("HAM mode !");
  else if (flags & EXTRA_HALFBRITE) puts("Extra Halfbrite mode !");

  return flags;
}

void read_CRNG(FILE *fp, int datasize)
{
  ULONG bytes_read;
  CRange color_reg_range;
  bytes_read = fread(&color_reg_range, sizeof(CRange), 1, fp);
#ifdef LITTLE_ENDIAN
  color_reg_range.rate = __bswap_16(color_reg_range.rate);
  color_reg_range.flags = __bswap_16(color_reg_range.flags);
#endif
#ifdef DEBUG
  printf("CRange, rate: %d, flags: %04x, low: %d, high: %d\n",
         (int) color_reg_range.rate, (int) color_reg_range.flags,
         (int) color_reg_range.low, (int) color_reg_range.high);
#endif
}

UBYTE *read_BODY(FILE *fp, int datasize, BitMapHeader *bmheader)
{
  ULONG bytes_read;
  BYTE *buffer = malloc(datasize), *dst_buffer;
  int src_i = 0, dst_i = 0, dst_size;

  dst_size = bmheader->w * bmheader->h * bmheader->nPlanes / 8;
  printf("target size: %d, data size: %d\n", dst_size, datasize);
  bytes_read = fread(buffer, sizeof(char), datasize, fp);

  if (bmheader->compression == cmpByteRun1) {
    BYTE b0, b1;
    int i;
    /* decompress data */
    dst_buffer = malloc(dst_size);
    while (src_i < datasize) {
      b0 = buffer[src_i++];
      if (b0 >= 0) {
        for (i = 0; i < b0 + 1; i++) dst_buffer[dst_i++] = buffer[src_i++];
      } else {
        b1 = buffer[src_i++];
        for (i = 0; i < -b0 + 1; i++) dst_buffer[dst_i++] = b1;
      }
    }
    free(buffer);
    return dst_buffer;
  }
  return buffer;
}

#define skip_chunk(fp, datasize) fseek(fp, datasize, SEEK_CUR)

void read_chunks(FILE *fp, int filesize, int total_read)
{
  char id[5], buffer[CHUNK_HEADER_SIZE];
  int i, bytes_read, datasize;
  BitMapHeader *bmheader = NULL;
  ColorRegister *colors = NULL;
  UBYTE *imgdata = NULL;

  while (total_read < filesize) {

    bytes_read = fread(buffer, sizeof(char), 8, fp);

    for (i = 0; i < 4; i++) id[i] = buffer[i];
    id[4] = 0;

#ifdef LITTLE_ENDIAN
    datasize = __bswap_32(*((ULONG *) &buffer[4]));
#else
    datasize = *((ULONG *) &buffer[4]);
#endif

    if (!strncmp("BMHD", buffer, 4)) bmheader = read_BMHD(fp, datasize);
    else if (!strncmp("CMAP", buffer, 4)) colors = read_CMAP(fp, datasize);
    else if (!strncmp("CRNG", buffer, 4)) read_CRNG(fp, datasize);
    else if (!strncmp("CAMG", buffer, 4)) read_CAMG(fp, datasize);
    else if (!strncmp("BODY", buffer, 4)) imgdata = read_BODY(fp, datasize, bmheader);
    else {
#ifdef DEBUG
      printf("WARNING - Unsupported chunk '%s', size: %d\n", id, datasize);
#endif
      skip_chunk(fp, datasize);
    }
    /* Padding to even if necessary */
    if (datasize % 2) {
      fseek(fp, 1, SEEK_CUR);
      datasize++;
    }
    total_read += datasize + CHUNK_HEADER_SIZE;
  }
  if (imgdata) free(imgdata);
  if (colors) free(colors);
  if (bmheader) free(bmheader);
}

void parse_file(const char *path)
{
  FILE *fp;
  char buffer[IFF_HEADER_SIZE];
  size_t bytes_read;
  ULONG filesize, total_read  = 0;

  fp = fopen(path, "rb");

  /* need to check whether we actually can read this much*/
  bytes_read = fread(buffer, sizeof(char), IFF_HEADER_SIZE, fp);
  total_read += bytes_read;

  if (!strncmp("FORM", buffer, 4)) {

#ifdef LITTLE_ENDIAN
    filesize = __bswap_32(*((ULONG *) &buffer[4])) + CHUNK_HEADER_SIZE;
#else
    filesize = *((ULONG *) &buffer[4]) + CHUNK_HEADER_SIZE;
#endif

    if (!strncmp("ILBM", &buffer[8], 4)) {
#ifdef DEBUG
      printf("IFF/ILBM file, file size: %d\n", (int) filesize);
#endif
      read_chunks(fp, filesize, total_read);
    } else {
      puts("not an IFF ILBM file");
    }
  } else {
    puts("not an IFF file");
  }
}

#ifdef STANDALONE
int main(int argc, char **argv)
{
  if (argc <= 1) {
    puts("usage: ilbm <image-file>");
  } else {
    parse_file(argv[1]);
  }
}
#endif
