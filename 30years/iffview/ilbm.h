/* ilbm.h - Definitions for IFF ILBM handling module

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
#pragma once
#ifndef __ILBM_H__
#define __ILBM_H__

#ifdef __VBCC__

#include <exec/types.h>
#include <graphics/view.h>

#else

#include <sys/types.h>
#define ULONG u_int32_t
#define UWORD u_int16_t
#define UBYTE u_int8_t

#define LONG int32_t
#define WORD int16_t
#define BYTE int8_t

#define HAM  0x800
#define EXTRA_HALFBRITE 0x80

#endif

/*
 * From the specification
 */
typedef UBYTE Masking;  /* Choice of masking technique. */

#define mskNone   0
#define mskHasMask   1
#define mskHasTransparentColor   2
#define mskLasso  3

typedef UBYTE Compression;
#define cmpNone      0
#define cmpByteRun1  1

typedef struct {
  UWORD w, h;
  WORD  x, y;
  UBYTE nPlanes;
  Masking masking;
  Compression compression;
  UBYTE pad1;
  UWORD transparentColor;
  UBYTE xAspect, yAspect;
  WORD  pageWidth, pageHeight;
} BitMapHeader;

typedef struct {
  UBYTE red, green, blue;
} ColorRegister;

typedef struct {
  WORD  pad1;
  WORD  rate;
  WORD  flags;
  UBYTE low, high;
} CRange;

typedef struct _IFFData {
    BitMapHeader *bmheader;
    ColorRegister *colors;
    UBYTE *imgdata;
} IFFData;

extern IFFData *parse_file(const char *path);
extern void free_iffdata(IFFData *data);
#endif /* __ILBM_H__ */
