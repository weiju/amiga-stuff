#pragma once
#ifndef __GLOBAL_DEFS_H__
#define __GLOBAL_DEFS_H__

#define ARG_TEMPLATE "SPEED/S,NTSC/S,HOW/S,SKY/S,FMODE/N/K"
#define ARG_SPEED 0
#define ARG_NTSC  1
#define ARG_HOW   2
#define ARG_SKY   3
#define ARG_FMODE 4
#define NUM_ARGS  5

#define DEMO_BLOCKS_PATH	"blocks/demoblocks.raw"
#define LARGE_MAP_PATH      "maps/large.raw"
#define SCROLLER_MAP_PATH   "maps/scroller.raw"
#define RACE_BLOCKS_PATH	"blocks/raceblocks.raw"
#define RACE_MAP_PATH       "maps/race.raw"

#define SCREENWIDTH  320
#define SCREENHEIGHT 256
#define SCREENBYTESPERROW (SCREENWIDTH / 8)

#define BLOCKSDEPTH 4
#define BLOCKSCOLORS (1L << BLOCKSDEPTH)

#define BLOCKWIDTH 16
#define BLOCKHEIGHT 16

#define NUMSTEPS BLOCKWIDTH

#define DIWSTART 0x2981
#define DIWSTOP  0x29C1

#define PALSIZE (BLOCKSCOLORS * 2)

#define IS_BITMAP_INTERLEAVED(bitmap)  ((GetBitMapAttr(bitmap, BMA_FLAGS) & BMF_INTERLEAVED) == BMF_INTERLEAVED)
#define ROUND2BLOCKWIDTH(x) ((x) & ~(BLOCKWIDTH - 1))
#define ROUND2BLOCKHEIGHT(x) ((x) & ~(BLOCKHEIGHT - 1))

#endif /* __GLOBAL_DEFS_H__ */
