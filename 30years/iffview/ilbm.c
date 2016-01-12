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

ColorRegister *read_CMAP(FILE *fp, int datasize, int *ncols)
{
  int num_colors = datasize / 3, bytes_read;
  ColorRegister *colors;

  colors = malloc(datasize);
  bytes_read = fread(colors, sizeof(ColorRegister), num_colors, fp);
  *ncols = num_colors;
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

UBYTE *read_BODY(FILE *fp, int datasize, BitMapHeader *bmheader, int *data_bytes)
{
  ULONG bytes_read;
  BYTE *buffer, *dst_buffer;
  int src_i = 0, dst_i = 0, dst_size;

  buffer = malloc(datasize);
  *data_bytes = datasize;
  dst_size = bmheader->w * bmheader->h * bmheader->nPlanes / 8;
  printf("target size: %d, data size: %d\n", dst_size, datasize);
  bytes_read = fread(buffer, sizeof(char), datasize, fp);

  if (bmheader->compression == cmpByteRun1) {
    BYTE b0, b1;
    int i;
    /* decompress data */
    dst_buffer = malloc(dst_size);
    *data_bytes = dst_size;
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

ILBMData *read_chunks(FILE *fp, int filesize, int total_read)
{
    // make sure that our char buffers are aligned on word boundaries
    unsigned char buffer[CHUNK_HEADER_SIZE], id[5];
    int i, bytes_read, datasize, imgdata_size;
    BitMapHeader *bmheader = NULL;
    ColorRegister *colors = NULL;
    UBYTE *imgdata = NULL;
    ILBMData *result = calloc(1, sizeof(ILBMData));

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
        else if (!strncmp("CMAP", buffer, 4)) colors = read_CMAP(fp, datasize, &result->num_colors);
        else if (!strncmp("CRNG", buffer, 4)) read_CRNG(fp, datasize);
        else if (!strncmp("CAMG", buffer, 4)) read_CAMG(fp, datasize);
        else if (!strncmp("BODY", buffer, 4)) imgdata = read_BODY(fp, datasize, bmheader, &imgdata_size);
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
    printf("total read: %d\n", total_read);
    result->imgdata = imgdata;
    result->data_bytes = imgdata_size;
    result->colors = colors;
    result->bmheader = bmheader;
    return result;
}

ILBMData *parse_file(const char *path)
{
  FILE *fp;
  char buffer[IFF_HEADER_SIZE];
  size_t bytes_read;
  ULONG filesize, total_read  = 0;
  ILBMData *result = NULL;

  fp = fopen(path, "rb");

  /* need to check whether we actually can read this much*/
  bytes_read = fread(buffer, sizeof(char), IFF_HEADER_SIZE, fp);
  total_read += bytes_read;

  if (!strncmp("FORM", buffer, 4)) {

#ifdef LITTLE_ENDIAN
    puts("little endian");
    filesize = __bswap_32(*((ULONG *) &buffer[4])) + CHUNK_HEADER_SIZE;
#else
    puts("big endian");
    filesize = *((ULONG *) &buffer[4]) + CHUNK_HEADER_SIZE;
#endif

    if (!strncmp("ILBM", &buffer[8], 4)) {
#ifdef DEBUG
      printf("IFF/ILBM file, file size: %d\n", (int) filesize);
#endif
      result = read_chunks(fp, filesize, total_read);
    } else {
      puts("not an IFF ILBM file");
    }
  } else {
    puts("not an IFF file");
  }
  fclose(fp);
  return result;
}

void free_ilbm_data(ILBMData *data)
{
    if (data) {
        if (data->colors) free(data->colors);
        if (data->bmheader) free(data->bmheader);
        if (data->imgdata) free(data->imgdata);
        free(data);
    }
}

void print_ilbm_info(ILBMData *data)
{
    printf("width: %d, height: %d, # planes: %d # colors: %d\n",
           (int) data->bmheader->w,
           (int) data->bmheader->h,
           (int) data->bmheader->nPlanes,
           data->num_colors);
}

/*
 * ILBM interleaved bit maps -> Intuition Image data
 * ILBM is interleaved
 *   - plane 0/row 0 -> plane 1/row 0 -> ... plane 0/row 1 -> plane 1/row 1 ->...
 *   - rows are multiples of 8
 *
 * but Intuition Image expects its data
 *   - plane-by-plane
 *   - as a multiple of 16
 */
void ilbm_to_image_data(char *dest, ILBMData *data, int dest_width, int dest_height)
{
    if (data->bmheader->w % 8 != 0) {
        puts("ERROR: source width must be multiple of 8");
        return;
    }
    if (dest_width % 16 != 0) {
        puts("ERROR: destination width must be multiple of 16");
        return;
    }
    if (dest_width < data->bmheader->w || dest_height < data->bmheader->h) {
        puts("ERROR: destination dimensions are smaller than source");
        return;
    }
    int src_bytes_per_row = data->bmheader->w / 8;
    int dest_bytes_per_row = dest_width / 8;
    char *src_ptr = data->imgdata;
    int src_offset, img_height = data->bmheader->h, num_planes = data->bmheader->nPlanes;
    int row_data_size = num_planes * src_bytes_per_row;

    for (int plane = 0; plane < num_planes; plane++) {
        // copy row-by-row
        for (int row = 0; row < img_height; row++) {
            src_offset = (row * row_data_size) + (plane * src_bytes_per_row);
            memcpy(dest, src_ptr + src_offset, src_bytes_per_row);
            dest += dest_bytes_per_row;
        }
    }
}

#ifdef STANDALONE
int main(int argc, char **argv)
{
  if (argc <= 1) {
      puts("usage: ilbm <image-file>");
  } else {
      ILBMData *data = parse_file(argv[1]);
      print_ilbm_info(data);

      int wordwidth = (data->bmheader->w + 16) / 16;
      int destWidth = wordwidth * 16;
      int destHeight = data->bmheader->h;
      int finalsize = wordwidth * data->bmheader->h * data->bmheader->nPlanes * 2;
      printf("loaded size: %d, final size: %d\n", data->data_bytes, finalsize);
      char *dest = calloc(finalsize, sizeof(char));
      ilbm_to_image_data(dest, data, destWidth, destHeight);
      free(dest);
      free_ilbm_data(data);
  }
}
#endif
