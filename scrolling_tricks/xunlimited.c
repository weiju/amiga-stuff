#include <exec/exec.h>
#include <dos/dos.h>
#include <intuition/intuition.h>
#include <graphics/gfx.h>
#include <hardware/custom.h>
#include <hardware/dmabits.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "hardware.h"
#include "cop_all.h"
#include "global_defs.h"
#include "common.h"

#define EXTRAWIDTH 32

#define BITMAPWIDTH ((SCREENWIDTH + EXTRAWIDTH) * 2)
#define BITMAPHEIGHT SCREENHEIGHT

#define BITMAPBYTESPERROW (BITMAPWIDTH / 8)
#define HALFBITMAPWIDTH (BITMAPWIDTH / 2)

#define BLOCKSWIDTH 320
#define BLOCKSHEIGHT 256
#define BLOCKSBYTESPERROW (BLOCKSWIDTH / 8)
#define BLOCKSPERROW (BLOCKSWIDTH / BLOCKWIDTH)

#define BITMAPBLOCKSPERROW (BITMAPWIDTH / BLOCKWIDTH)
#define HALFBITMAPBLOCKSPERROW (BITMAPBLOCKSPERROW / 2)
#define BITMAPBLOCKSPERCOL (BITMAPHEIGHT / BLOCKHEIGHT)

#define BITMAPPLANELINES (BITMAPHEIGHT * BLOCKSDEPTH)
#define BLOCKPLANELINES  (BLOCKHEIGHT * BLOCKSDEPTH)

#define BLOCKSFILESIZE (BLOCKSWIDTH * BLOCKSHEIGHT * BLOCKSPLANES / 8 + PALSIZE)

struct LevelMap   level_map;
struct PrgOptions options;
struct BitMap     *blocks_bitmap, *screen_bitmap;
struct RastPort   *screen_rastport;
struct Screen     *scr;

UBYTE *frontbuffer, *blocksbuffer;
WORD  mapposx, videoposx;
UWORD colors[BLOCKSCOLORS];
char  s[256];

struct FetchInfo fetchinfo [] =
{
	{0x30, 0xD0, 2,  0, 16}, /* normal         */
	{0x28, 0xC8, 4, 16, 32}, /* BPL32          */
	{0x28, 0xC8, 4, 16, 32}, /* BPAGEM         */
	{0x18, 0xB8, 8, 48, 64}  /* BPL32 + BPAGEM */
};

static void cleanup(char *msg)
{
	WORD rc;

	if (msg) {
		printf("Error: %s\n", msg);
		rc = RETURN_WARN;
	} else rc = RETURN_OK;

	if (scr) CloseScreen(scr);

	if (screen_bitmap) {
		WaitBlit();
        FreeBitMap(screen_bitmap);
	}

	if (blocks_bitmap) {
		WaitBlit();
		FreeBitMap(blocks_bitmap);
	}
	if (level_map.raw_map) free(level_map.raw_map);
	exit(rc);
}

static void open_display(void)
{
	if (!(screen_bitmap = AllocBitMap(BITMAPWIDTH, BITMAPHEIGHT + 3, BLOCKSDEPTH,
                                      BMF_STANDARD | BMF_INTERLEAVED | BMF_CLEAR, 0))) {
		cleanup("Can't alloc screen bitmap!");
	}
	frontbuffer = screen_bitmap->Planes[0];
	frontbuffer += (fetchinfo[options.fetchmode].bitmapoffset / 8);

	if (!(TypeOfMem(screen_bitmap->Planes[0]) & MEMF_CHIP)) {
		cleanup("Screen bitmap is not in CHIP RAM!?? If you have a gfx card try disabling \"planes to fast\" or similiar options in your RTG system!");
	}

	if (!IS_BITMAP_INTERLEAVED(screen_bitmap))	{
		cleanup("Screen bitmap is not in interleaved format!??");
	}
    ULONG modeid = get_mode_id(options.how, options.ntsc);

	if (!(scr = OpenScreenTags(0, SA_Width, BITMAPWIDTH,
                               SA_Height, BITMAPHEIGHT + 3,
                               SA_Depth, BLOCKSDEPTH,
                               SA_DisplayID, modeid,
                               SA_BitMap, screen_bitmap,
                               options.how ? SA_Overscan : TAG_IGNORE,OSCAN_TEXT,
                               options.how ? SA_AutoScroll : TAG_IGNORE,TRUE,
                               SA_Quiet,TRUE,
                               TAG_DONE))) {
		cleanup("Can't open screen!");
	}

	if (scr->RastPort.BitMap->Planes[0] != screen_bitmap->Planes[0])	{
		cleanup("Screen was not created with the custom bitmap I supplied!??");
	}
	screen_rastport = &scr->RastPort;
	LoadRGB4(&scr->ViewPort, colors, BLOCKSCOLORS);
}

static void init_copper_list(void)
{
	wait_vbl();
	custom->dmacon = 0x7FFF;
	custom->beamcon0 = options.ntsc ? 0 : DISPLAYPAL;
	CopFETCHMODE[1] = options.fetchmode;

	// bitplane control registers
	CopBPLCON0[1] = ((BLOCKSDEPTH * BPL0_BPU0_F) & BPL0_BPUMASK) + ((BLOCKSDEPTH / 8) * BPL0_BPU3_F) +
        BPL0_COLOR_F + (options.speed ? 0 : BPL0_USEBPLCON3_F);

	CopBPLCON1[1] = 0;
	CopBPLCON3[1] = BPLCON3_BRDNBLNK;

	// bitplane modulos
	LONG l = BITMAPBYTESPERROW * BLOCKSDEPTH - SCREENBYTESPERROW -
        fetchinfo[options.fetchmode].modulooffset;

	CopBPLMODA[1] = l;
	CopBPLMODB[1] = l;

	// display window start/stop
	CopDIWSTART[1] = DIWSTART;
	CopDIWSTOP[1] = DIWSTOP;

	// display data fetch start/stop
	CopDDFSTART[1] = fetchinfo[options.fetchmode].ddfstart;
	CopDDFSTOP[1]  = fetchinfo[options.fetchmode].ddfstop;

	// plane pointers
	WORD *wp = CopPLANE1H;

	for (l = 0; l < BLOCKSDEPTH; l++)	{
		wp[1] = (WORD)(((ULONG) screen_bitmap->Planes[l]) >> 16);
		wp[3] = (WORD)(((ULONG) screen_bitmap->Planes[l]) & 0xFFFF);
		wp += 4;
	}

    // activate copper sky
	if (options.sky) CopSKY[0] = 0x290f;

	custom->intena = 0x7FFF;
	custom->dmacon = DMAF_SETCLR | DMAF_BLITTER | DMAF_COPPER | DMAF_RASTER | DMAF_MASTER;
	custom->cop2lc = (ULONG) CopperList;
}

static void draw_block(LONG x, LONG y, LONG mapx, LONG mapy)
{
	// x = in pixels
	// y = in "planelines" (1 realline = BLOCKSDEPTH planelines)
	x = x / 8;
	y = y * BITMAPBYTESPERROW;

	UBYTE block = level_map.data[mapy * level_map.width + mapx];

	mapx = (block % BLOCKSPERROW) * (BLOCKWIDTH / 8);
	mapy = (block / BLOCKSPERROW) * (BLOCKPLANELINES * BLOCKSBYTESPERROW);

	if (options.how) OwnBlitter();

	hard_wait_blit();

	custom->bltcon0 = 0x9F0;	// use A and D. Op: D = A
	custom->bltcon1 = 0;
	custom->bltafwm = 0xFFFF;
	custom->bltalwm = 0xFFFF;
	custom->bltamod = BLOCKSBYTESPERROW - (BLOCKWIDTH / 8);
	custom->bltdmod = BITMAPBYTESPERROW - (BLOCKWIDTH / 8);
	custom->bltapt  = blocksbuffer + mapy + mapx;
	custom->bltdpt  = frontbuffer + y + x;
	custom->bltsize = BLOCKPLANELINES * 64 + (BLOCKWIDTH / 16);

	if (options.how) DisownBlitter();
}

static void fill_screen(void)
{
	WORD a, b, x, y;

	for (b = 0; b < BITMAPBLOCKSPERCOL; b++) {
		for (a = 0; a < HALFBITMAPBLOCKSPERROW; a++) {
			x = a * BLOCKWIDTH;
			y = b * BLOCKPLANELINES;
			draw_block(x, y, a, b);
			draw_block(x + HALFBITMAPWIDTH, y, a, b);
		}
	}
}

static void scroll_left(void)
{
	if (mapposx < 1) return;

	mapposx--;
	videoposx = mapposx % HALFBITMAPWIDTH;

	WORD mapx = mapposx / BLOCKWIDTH;
	WORD mapy = mapposx & (NUMSTEPS - 1);

	WORD x = ROUND2BLOCKWIDTH(videoposx);
	WORD y = mapy * BLOCKPLANELINES;

	draw_block(x, y, mapx, mapy);
	draw_block(x + HALFBITMAPWIDTH, y, mapx, mapy);
}

static void scroll_right(void)
{
	if (mapposx >= (level_map.width * BLOCKWIDTH - SCREENWIDTH - BLOCKWIDTH)) return;

	WORD mapx = mapposx / BLOCKWIDTH + HALFBITMAPBLOCKSPERROW;
	WORD mapy = mapposx & (NUMSTEPS - 1);

	WORD x = ROUND2BLOCKWIDTH(videoposx);
	WORD y = mapy * BLOCKPLANELINES;

	draw_block(x, y, mapx, mapy);
	draw_block(x + HALFBITMAPWIDTH, y, mapx, mapy);

	mapposx++;
	videoposx = mapposx % HALFBITMAPWIDTH;
}

static void check_joy_scroll(void)
{
    WORD count = joy_fire() ? count = 8 : 1;
	if (joy_left())  for (int i = 0; i < count; i++) scroll_left();
	if (joy_right()) for (int i = 0; i < count; i++) scroll_right();
}

static void update_copper_list(void)
{
	WORD i = fetchinfo[options.fetchmode].scrollpixels;
	WORD xpos = videoposx + i - 1;
	WORD planeaddx = (xpos / i) * (i / 8);

	i = (i - 1) - (xpos & (i - 1));

	WORD scroll = (i & 15) * 0x11;
	if (i & 16) scroll |= (0x400 + 0x4000);
	if (i & 32) scroll |= (0x800 + 0x8000);

	// set scroll register in BPLCON1
	CopBPLCON1[1] = scroll;

	// set plane pointers
	WORD *wp = CopPLANE1H;
	ULONG pl;

	for (i = 0; i < BLOCKSDEPTH; i++)	{
		pl = ((ULONG) screen_bitmap->Planes[i]) + planeaddx;
		wp[1] = (WORD) (pl >> 16);
		wp[3] = (WORD) (pl & 0xFFFF);
		wp += 4;
	}
}

static void show_what_copper_would_do(void)
{
	SetWriteMask(screen_rastport,1);
	SetAPen(screen_rastport, 0);
	RectFill(screen_rastport, 0, BITMAPHEIGHT + 1, BITMAPWIDTH - 1, BITMAPHEIGHT + 1);
	SetAPen(screen_rastport, 1);
	RectFill(screen_rastport, videoposx + BLOCKWIDTH, BITMAPHEIGHT + 1,
             videoposx + BLOCKWIDTH + SCREENWIDTH - 1, BITMAPHEIGHT + 1);
}

static void main_loop(void)
{
	if (!options.how) {
		// activate copperlist
		hard_wait_blit();
		wait_vbl();
		custom->copjmp2 = 0;
	}

	while (!lmb_down()) {
		if (!options.how) {
			wait_vbeam(199);
			wait_vbeam(200);
		} else Delay(1);

		if (options.speed) *(WORD *) 0xdff180 = 0xff0;

		check_joy_scroll();

		if (options.speed) *(WORD *) 0xdff180 = 0xf00;
		if (!options.how) update_copper_list();
		else show_what_copper_would_do();
	}
}

int main(int argc, char **argv)
{
	BOOL res = get_arguments(&options, s);
    if (!res) cleanup(s);
	res = read_level_map(LARGE_MAP_PATH, &level_map, s);
    if (!res) cleanup(s);

	blocks_bitmap = read_blocks(DEMO_BLOCKS_PATH, colors, s, BLOCKSWIDTH, BLOCKSHEIGHT);
    if (!blocks_bitmap) cleanup(s);
	blocksbuffer = blocks_bitmap->Planes[0];

	open_display();

	if (!options.how) {
		Delay(2 * 50);
		kill_system();
		init_copper_list();
	}
	fill_screen();
	main_loop();

	if (!options.how) activate_system();
	cleanup(0);
    return 0;
}
