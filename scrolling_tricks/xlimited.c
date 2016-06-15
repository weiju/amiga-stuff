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
#include "cop.h"
#include "global_defs.h"
#include "common.h"


#define BITMAPWIDTH (SCREENWIDTH + EXTRAWIDTH)
#define BITMAPBYTESPERROW (BITMAPWIDTH / 8)

#define BITMAPBLOCKSPERROW (BITMAPWIDTH / BLOCKWIDTH)
#define BITMAPBLOCKSPERCOL (BITMAPHEIGHT / BLOCKHEIGHT)

#define BITMAPPLANELINES (BITMAPHEIGHT * BLOCKSDEPTH)
#define BLOCKPLANELINES  (BLOCKHEIGHT * BLOCKSDEPTH)

#define BLOCKSFILESIZE (BLOCKSWIDTH * BLOCKSHEIGHT * frontbuffer / 8 + PALSIZE)

#define DIRECTION_RIGHT 0
#define DIRECTION_LEFT  1

struct Screen *scr;
struct RastPort *ScreenRastPort;
struct BitMap *BlocksBitmap,*ScreenBitmap;
UBYTE	 *frontbuffer,*blocksbuffer;

WORD	mapposx,videoposx;
WORD	bitmapheight;
BYTE	previous_direction;
WORD	*savewordpointer;
WORD	saveword;

struct LevelMap level_map;

UWORD	colors[BLOCKSCOLORS];

struct PrgOptions options;
BPTR	MyHandle;
char	s[256];

#if EXTRAWIDTH == 32

	// bitmap width aligned to 32 Pixels
	#define MAX_FETCHMODE 2
	#define MAX_FETCHMODE_S "2"

#elif EXTRAWIDTH == 64

	// bitmap width aligned to 64 Pixels
	#define MAX_FETCHMODE 3
	#define MAX_FETCHMODE_S "3"

#else

	// bad extrawidth
	#error "EXTRAWIDTH must be either 32 or 64"

#endif

struct FetchInfo fetchinfo [] =
{
	{0x30,0xD0,2,0,16},	/* normal         */
	{0x28,0xC8,4,16,32},	/* BPL32          */
	{0x28,0xC8,4,16,32},	/* BPAGEM         */
	{0x18,0xB8,8,48,64}	/* BPL32 + BPAGEM */
};

/************* SETUP/CLEANUP ROUTINES ***************/

static void Cleanup (char *msg)
{
	WORD rc;

	if (msg) {
		printf("Error: %s\n",msg);
		rc = RETURN_WARN;
	} else {
		rc = RETURN_OK;
	}

	if (scr) CloseScreen(scr);

	if (ScreenBitmap) {
		WaitBlit();
		FreeBitMap(ScreenBitmap);
	}

	if (BlocksBitmap) {
		WaitBlit();
		FreeBitMap(BlocksBitmap);
	}

	if (level_map.raw_map) FreeVec(level_map.raw_map);

	if (MyHandle) Close(MyHandle);
	exit(rc);
}

static void OpenBlocks(void)
{
	LONG l;

	if (!(BlocksBitmap = AllocBitMap(BLOCKSWIDTH,
                                     BLOCKSHEIGHT,
                                     BLOCKSDEPTH,
                                     BMF_STANDARD | BMF_INTERLEAVED,
                                     0))) {
		Cleanup("Can't alloc blocks bitmap!");
	}

	if (!(MyHandle = Open(BLOCKSNAME,MODE_OLDFILE))) {
		Fault(IoErr(),0,s,255);
		Cleanup(s);
	}

	if (Read(MyHandle,colors,PALSIZE) != PALSIZE) {
		Fault(IoErr(),0,s,255);
		Cleanup(s);
	}

	l = BLOCKSWIDTH * BLOCKSHEIGHT * BLOCKSDEPTH / 8;

	if (Read(MyHandle, BlocksBitmap->Planes[0], l) != l) {
		Fault(IoErr(), 0, s, 255);
		Cleanup(s);
	}
	Close(MyHandle);
    MyHandle = 0;
	blocksbuffer = BlocksBitmap->Planes[0];
}

static void OpenDisplay(void)
{
	ULONG						modeid;

	bitmapheight = BITMAPHEIGHT +
						(level_map.width / BITMAPBLOCKSPERROW / BLOCKSDEPTH) + 1 +
						3;

	if (!(ScreenBitmap = AllocBitMap(BITMAPWIDTH,bitmapheight,BLOCKSDEPTH,BMF_STANDARD | BMF_INTERLEAVED | BMF_CLEAR,0)))
	{
		Cleanup("Can't alloc screen bitmap!");
	}

	frontbuffer = ScreenBitmap->Planes[0];
	frontbuffer += (fetchinfo[options.fetchmode].bitmapoffset / 8);

	if (!(TypeOfMem(ScreenBitmap->Planes[0]) & MEMF_CHIP))
	{
		Cleanup("Screen bitmap is not in CHIP RAM!?? If you have a gfx card try disabling \"planes to fast\" or similiar options in your RTG system!");
	}

	if (!IS_BITMAP_INTERLEAVED(ScreenBitmap)) {
		Cleanup("Screen bitmap is not in interleaved format!??");
	}

    modeid = get_mode_id(options.how, options.ntsc);
	if (!(scr = OpenScreenTags(0,SA_Width,BITMAPWIDTH,
										  SA_Height,bitmapheight,
										  SA_Depth,BLOCKSDEPTH,
										  SA_DisplayID,modeid,
										  SA_BitMap,ScreenBitmap,
										  options.how ? SA_Overscan : TAG_IGNORE,OSCAN_TEXT,
										  options.how ? SA_AutoScroll : TAG_IGNORE,TRUE,
										  SA_Quiet,TRUE,
										  TAG_DONE))) {
		Cleanup("Can't open screen!");
	}

	if (scr->RastPort.BitMap->Planes[0] != ScreenBitmap->Planes[0]) {
		Cleanup("Screen was not created with the custom bitmap I supplied!??");
	}

	ScreenRastPort = &scr->RastPort;
	LoadRGB4(&scr->ViewPort,colors,BLOCKSCOLORS);
}

static void InitCopperlist(void)
{
	WORD	*wp;
	LONG l;

	WaitVBL();

	custom->dmacon = 0x7FFF;
	custom->beamcon0 = options.ntsc ? 0 : DISPLAYPAL;

	CopFETCHMODE[1] = options.fetchmode;

	// bitplane control registers
	CopBPLCON0[1] = ((BLOCKSDEPTH * BPL0_BPU0_F) & BPL0_BPUMASK) +
        ((BLOCKSDEPTH / 8) * BPL0_BPU3_F) +
        BPL0_COLOR_F +
        (options.speed ? 0 : BPL0_USEBPLCON3_F);

	CopBPLCON1[1] = 0;

	CopBPLCON3[1] = BPLCON3_BRDNBLNK;

	// bitplane modulos
	l = BITMAPBYTESPERROW * BLOCKSDEPTH -
		 SCREENBYTESPERROW - fetchinfo[options.fetchmode].modulooffset;

	CopBPLMODA[1] = l;
	CopBPLMODB[1] = l;

	// display window start/stop
	CopDIWSTART[1] = DIWSTART;
	CopDIWSTOP[1] = DIWSTOP;

	// display data fetch start/stop
	CopDDFSTART[1] = fetchinfo[options.fetchmode].ddfstart;
	CopDDFSTOP[1]  = fetchinfo[options.fetchmode].ddfstop;

	// plane pointers
	wp = CopPLANE1H;

	for (l = 0; l < BLOCKSDEPTH; l++) {
		wp[1] = (WORD)(((ULONG)ScreenBitmap->Planes[l]) >> 16);
		wp[3] = (WORD)(((ULONG)ScreenBitmap->Planes[l]) & 0xFFFF);
		wp += 4;
	}

	if (options.sky) {
		// activate copper sky
		CopSKY[0] = 0x290f;
	}
	custom->intena = 0x7FFF;
	custom->dmacon = DMAF_SETCLR | DMAF_BLITTER | DMAF_COPPER | DMAF_RASTER | DMAF_MASTER;
	custom->cop2lc = (ULONG)CopperList;
}

/******************* SCROLLING **********************/

static void DrawBlock(LONG x, LONG y, LONG mapx, LONG mapy)
{
	UBYTE block;

	// x = in pixels
	// y = in "planelines" (1 realline = BLOCKSDEPTH planelines)
	x = (x / 8) & 0xFFFE;
	y = y * BITMAPBYTESPERROW;

	block = level_map.data[mapy * level_map.width + mapx];

	mapx = (block % BLOCKSPERROW) * (BLOCKWIDTH / 8);
	mapy = (block / BLOCKSPERROW) * (BLOCKPLANELINES * BLOCKSBYTESPERROW);

	if (options.how) OwnBlitter();

	HardWaitBlit();

	custom->bltcon0 = 0x9F0;	// use A and D. Op: D = A
	custom->bltcon1 = 0;
	custom->bltafwm = 0xFFFF;
	custom->bltalwm = 0xFFFF;
	custom->bltamod = BLOCKSBYTESPERROW - (BLOCKWIDTH / 8);
	custom->bltdmod = BITMAPBYTESPERROW - (BLOCKWIDTH / 8);
	custom->bltapt  = blocksbuffer + mapy + mapx;
	custom->bltdpt	 = frontbuffer + y + x;
	custom->bltsize = BLOCKPLANELINES * 64 + (BLOCKWIDTH / 16);

	if (options.how) DisownBlitter();
}

static void FillScreen(void)
{
	WORD a, b, x, y;

	for (b = 0; b < BITMAPBLOCKSPERCOL; b++) {
		for (a = 0; a < BITMAPBLOCKSPERROW; a++) {
			x = a * BLOCKWIDTH;
			y = b * BLOCKPLANELINES;
			DrawBlock(x, y, a, b);
		}
	}
}

static void ScrollLeft(void)
{
	WORD mapx,mapy,x,y;

	if (mapposx < 1) return;

	mapposx--;
	videoposx = mapposx;

	mapx = mapposx / BLOCKWIDTH;
	mapy = mapposx & (NUMSTEPS - 1);

	x = ROUND2BLOCKWIDTH(videoposx);
	y = mapy * BLOCKPLANELINES;

	if (previous_direction == DIRECTION_RIGHT) {
		HardWaitBlit();
		*savewordpointer = saveword;
	}
	savewordpointer = (WORD *)(frontbuffer + y * BITMAPBYTESPERROW + (x / 8));
	saveword = *savewordpointer;

	DrawBlock(x,y,mapx,mapy);
	previous_direction = DIRECTION_LEFT;
}

static void ScrollRight(void)
{
	WORD mapx,mapy,x,y;

	if (mapposx >= (level_map.width * BLOCKWIDTH - SCREENWIDTH - BLOCKWIDTH)) return;

	mapx = mapposx / BLOCKWIDTH + BITMAPBLOCKSPERROW;
	mapy = mapposx & (NUMSTEPS - 1);

	x = BITMAPWIDTH + ROUND2BLOCKWIDTH(videoposx);
	y = mapy * BLOCKPLANELINES;

	if (previous_direction == DIRECTION_LEFT) {
		HardWaitBlit();
		*savewordpointer = saveword;
	}
	savewordpointer = (WORD *)(frontbuffer + (y + BLOCKPLANELINES - 1) * BITMAPBYTESPERROW + (x / 8));
	saveword = *savewordpointer;

	DrawBlock(x,y,mapx,mapy);

	mapposx++;
	videoposx = mapposx;
	previous_direction = DIRECTION_RIGHT;
}

static void CheckJoyScroll(void)
{
	WORD i,count;

	if (JoyFire()) count = 8;
    else count = 1;

	if (JoyLeft()) {
		for (i = 0; i < count; i++) {
			ScrollLeft();
		}
	}

	if (JoyRight()) {
		for (i = 0;i < count;i++) {
			ScrollRight();
		}
	}
}

static void UpdateCopperlist(void)
{
	ULONG pl;
	WORD xpos, planeaddx, scroll, i;
	WORD *wp;

	i = fetchinfo[options.fetchmode].scrollpixels;

	xpos = videoposx + i - 1;

	planeaddx = (xpos / i) * (i / 8);
	i = (i - 1) - (xpos & (i - 1));
	scroll = (i & 15) * 0x11;

	if (i & 16) scroll |= (0x400 + 0x4000);
	if (i & 32) scroll |= (0x800 + 0x8000);

	// set scroll register in BPLCON1
	CopBPLCON1[1] = scroll;

	// set plane pointers
	wp = CopPLANE1H;

	for (i = 0; i < BLOCKSDEPTH; i++) {
		pl = ((ULONG)ScreenBitmap->Planes[i]) + planeaddx;
		wp[1] = (WORD)(pl >> 16);
		wp[3] = (WORD)(pl & 0xFFFF);
		wp += 4;
	}
}

static void ShowWhatCopperWouldDo(void)
{
	WORD x = (videoposx+16) % BITMAPWIDTH;

	SetWriteMask(ScreenRastPort,1);
	SetAPen(ScreenRastPort,0);
	RectFill(ScreenRastPort,0,bitmapheight - 3,BITMAPWIDTH-1,bitmapheight - 3);
	SetAPen(ScreenRastPort,1);

	if (x <= EXTRAWIDTH) {
		RectFill(ScreenRastPort,x,bitmapheight - 3,x+SCREENWIDTH-1,bitmapheight - 3);
	} else {
		RectFill(ScreenRastPort,x,bitmapheight - 3,BITMAPWIDTH-1,bitmapheight - 3);
		RectFill(ScreenRastPort,0,bitmapheight - 3,x - EXTRAWIDTH,bitmapheight - 3);
	}
}

static void MainLoop(void)
{
	if (!options.how) {
		HardWaitBlit();
		WaitVBL();

		// activate copperlist
		custom->copjmp2 = 0;
	}

	while (!LMBDown()) {
		if (!options.how) {
			WaitVBeam(199);
			WaitVBeam(200);
		} else {
			Delay(1);
		}

		if (options.speed) *(WORD *)0xdff180 = 0xFF0;

		CheckJoyScroll();

		if (options.speed) *(WORD *)0xdff180 = 0xF00;
		if (!options.how) UpdateCopperlist();
		else ShowWhatCopperWouldDo();
	}
}

/********************* MAIN *************************/

int main(int argc, char **argv)
{
	BOOL res = get_arguments(&options, s);
    if (!res) Cleanup(s);
	res = read_level_map(&level_map, s);
    if (!res) Cleanup(s);

	OpenBlocks();
	OpenDisplay();

	if (!options.how) {
		Delay(2*50);
		KillSystem();
		InitCopperlist();
	}
	FillScreen();
	MainLoop();

	if (!options.how) ActivateSystem();
	Cleanup(0);
    return 0;
}
