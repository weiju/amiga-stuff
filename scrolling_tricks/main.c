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
#include "map.h"


#define ARG_TEMPLATE "SPEED/S,NTSC/S,HOW/S,SKY/S,FMODE/N/K"
#define ARG_SPEED 0
#define ARG_NTSC  1
#define ARG_HOW   2
#define ARG_SKY   3
#define ARG_FMODE 4
#define NUM_ARGS  5

#define MAPNAME		"maps/large.raw"
#define BLOCKSNAME	"blocks/demoblocks.raw"

#define SCREENWIDTH  320
#define SCREENHEIGHT 256
#define EXTRAWIDTH 32
#define SCREENBYTESPERROW (SCREENWIDTH / 8)

#define BITMAPWIDTH ((SCREENWIDTH + EXTRAWIDTH) * 2)
#define BITMAPBYTESPERROW (BITMAPWIDTH / 8)
#define HALFBITMAPWIDTH (BITMAPWIDTH / 2)
#define BITMAPHEIGHT SCREENHEIGHT

#define BLOCKSWIDTH 320
#define BLOCKSHEIGHT 256
#define BLOCKSDEPTH 4
#define BLOCKSCOLORS (1L << BLOCKSDEPTH)
#define BLOCKWIDTH 16
#define BLOCKHEIGHT 16
#define BLOCKSBYTESPERROW (BLOCKSWIDTH / 8)
#define BLOCKSPERROW (BLOCKSWIDTH / BLOCKWIDTH)

#define NUMSTEPS BLOCKWIDTH

#define BITMAPBLOCKSPERROW (BITMAPWIDTH / BLOCKWIDTH)
#define HALFBITMAPBLOCKSPERROW (BITMAPBLOCKSPERROW / 2)
#define BITMAPBLOCKSPERCOL (BITMAPHEIGHT / BLOCKHEIGHT)

#define BITMAPPLANELINES (BITMAPHEIGHT * BLOCKSDEPTH)
#define BLOCKPLANELINES  (BLOCKHEIGHT * BLOCKSDEPTH)

#define DIWSTART 0x2981
#define DIWSTOP  0x29C1

#define PALSIZE (BLOCKSCOLORS * 2)
#define BLOCKSFILESIZE (BLOCKSWIDTH * BLOCKSHEIGHT * BLOCKSPLANES / 8 + PALSIZE)

struct Screen *scr;
struct RastPort *ScreenRastPort;
struct BitMap *BlocksBitmap, *ScreenBitmap;
struct RawMap *Map;
UBYTE	 *frontbuffer,*blocksbuffer;

WORD	mapposx,videoposx;

LONG	mapwidth;
UBYTE *mapdata;

UWORD	colors[BLOCKSCOLORS];

LONG	Args[NUM_ARGS];

BOOL	option_ntsc,option_how,option_speed,option_sky;
WORD	option_fetchmode;

BPTR	MyHandle;
char	s[256];

struct FetchInfo
{
	WORD	ddfstart;
	WORD	ddfstop;
	WORD	modulooffset;
	WORD	bitmapoffset;
	WORD	scrollpixels;

} fetchinfo [] =
{
	{0x30,0xD0,2,0,16},	/* normal         */
	{0x28,0xC8,4,16,32},	/* BPL32          */
	{0x28,0xC8,4,16,32},	/* BPAGEM         */
	{0x18,0xB8,8,48,64}	/* BPL32 + BPAGEM */
};

/********************* MACROS ***********************/

#define ROUND2BLOCKWIDTH(x) ((x) & ~(BLOCKWIDTH - 1))

/********************* COMPATIBILITY ***********************/

ULONG is_bitmap_interleaved(struct BitMap *bitmap)
{
    return (GetBitMapAttr(bitmap, BMA_FLAGS) & BMF_INTERLEAVED) == BMF_INTERLEAVED;
}

static ULONG get_mode_id_os3(void)
{
    ULONG modeid = INVALID_ID;
	struct DimensionInfo diminfo;
	DisplayInfoHandle	dih;

    if ((dih = FindDisplayInfo(VGAPRODUCT_KEY))) {
        if (GetDisplayInfoData(dih,(APTR)&diminfo,sizeof(diminfo),DTAG_DIMS,0))	{
            if (diminfo.MaxDepth >= BLOCKSDEPTH) modeid = VGAPRODUCT_KEY;
        }
    }
    return modeid;
}

ULONG get_mode_id(void)
{
    ULONG modeid;

	if (option_how)	{
		modeid = get_mode_id_os3();

		if (modeid == INVALID_ID) {
			if (option_ntsc) modeid = NTSC_MONITOR_ID | HIRESLACE_KEY;
			else modeid = PAL_MONITOR_ID | HIRESLACE_KEY;
		}
	} else {
		if (option_ntsc) modeid = NTSC_MONITOR_ID;
		else modeid = PAL_MONITOR_ID;
	}
    return modeid;
}

/************* SETUP/CLEANUP ROUTINES ***************/

static void Cleanup(char *msg)
{
	WORD rc;

	if (msg) {
		printf("Error: %s\n",msg);
		rc = RETURN_WARN;
	} else rc = RETURN_OK;

	if (scr) CloseScreen(scr);

	if (ScreenBitmap) {
		WaitBlit();
        FreeBitMap(ScreenBitmap);
	}

	if (BlocksBitmap) {
		WaitBlit();
		FreeBitMap(BlocksBitmap);
	}

	if (Map) free(Map);
	if (MyHandle) Close(MyHandle);
	exit(rc);
}

static void GetArguments(void)
{
	struct RDArgs *MyArgs;

	if (!(MyArgs = ReadArgs(ARG_TEMPLATE,Args,0))) {
		Fault(IoErr(),0,s,255);
		Cleanup(s);
	}

	if (Args[ARG_SPEED]) option_speed = TRUE;
	if (Args[ARG_NTSC]) option_ntsc = TRUE;
	if (Args[ARG_HOW]) {
		option_how = TRUE;
		option_speed = FALSE;
	}
	if (Args[ARG_SKY] && (!option_speed)) {
		option_sky = TRUE;
	}
	if (Args[ARG_FMODE]) {
		option_fetchmode = *(LONG *)Args[ARG_FMODE];
	}
	FreeArgs(MyArgs);

	if (option_fetchmode < 0 || option_fetchmode > 3) {
		Cleanup("Invalid fetch mode. Must be 0 .. 3!");
	}
}

static void OpenMap(void)
{
	LONG l;

	if (!(MyHandle = Open(MAPNAME,MODE_OLDFILE))) {
		Fault(IoErr(),0,s,255);
		Cleanup(s);
	}

	Seek(MyHandle,0,OFFSET_END);
	l = Seek(MyHandle,0,OFFSET_BEGINNING);

	if (!(Map = calloc(l, sizeof(UBYTE)))) {
		Cleanup("Out of memory!");
	}

	if (Read(MyHandle,Map,l) != l) {
		Fault(IoErr(),0,s,255);
		Cleanup(s);
	}
	Close(MyHandle);
    MyHandle = 0;

	mapdata = Map->data;
	mapwidth = Map->mapwidth;
}

static void OpenBlocks(void)
{
	LONG l;

	if (!(BlocksBitmap = AllocBitMap(BLOCKSWIDTH,
                                     BLOCKSHEIGHT,
                                     BLOCKSDEPTH,
                                     BMF_STANDARD | BMF_INTERLEAVED,
                                     0)))	{
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

	if (Read(MyHandle,BlocksBitmap->Planes[0],l) != l) {
		Fault(IoErr(),0,s,255);
		Cleanup(s);
	}
	Close(MyHandle);MyHandle = 0;
	blocksbuffer = BlocksBitmap->Planes[0];
}

static void OpenDisplay(void)
{
	ULONG				modeid;
	LONG				bmflags;

	if (!(ScreenBitmap = AllocBitMap(BITMAPWIDTH,BITMAPHEIGHT + 3,BLOCKSDEPTH,
                                     BMF_STANDARD | BMF_INTERLEAVED | BMF_CLEAR,0))) {
		Cleanup("Can't alloc screen bitmap!");
	}
	frontbuffer = ScreenBitmap->Planes[0];
	frontbuffer += (fetchinfo[option_fetchmode].bitmapoffset / 8);

	if (!(TypeOfMem(ScreenBitmap->Planes[0]) & MEMF_CHIP)) {
		Cleanup("Screen bitmap is not in CHIP RAM!?? If you have a gfx card try disabling \"planes to fast\" or similiar options in your RTG system!");
	}

	if (!is_bitmap_interleaved(ScreenBitmap))	{
		Cleanup("Screen bitmap is not in interleaved format!??");
	}
    modeid = get_mode_id();

	if (!(scr = OpenScreenTags(0,SA_Width,BITMAPWIDTH,
										  SA_Height,BITMAPHEIGHT + 3,
										  SA_Depth,BLOCKSDEPTH,
										  SA_DisplayID,modeid,
										  SA_BitMap,ScreenBitmap,
										  option_how ? SA_Overscan : TAG_IGNORE,OSCAN_TEXT,
										  option_how ? SA_AutoScroll : TAG_IGNORE,TRUE,
										  SA_Quiet,TRUE,
										  TAG_DONE))) {
		Cleanup("Can't open screen!");
	}

	if (scr->RastPort.BitMap->Planes[0] != ScreenBitmap->Planes[0])	{
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
	custom->beamcon0 = option_ntsc ? 0 : DISPLAYPAL;
	CopFETCHMODE[1] = option_fetchmode;

	// bitplane control registers
	CopBPLCON0[1] = ((BLOCKSDEPTH * BPL0_BPU0_F) & BPL0_BPUMASK) +
						 ((BLOCKSDEPTH / 8) * BPL0_BPU3_F) +
						 BPL0_COLOR_F +
						 (option_speed ? 0 : BPL0_USEBPLCON3_F);

	CopBPLCON1[1] = 0;
	CopBPLCON3[1] = BPLCON3_BRDNBLNK;

	// bitplane modulos
	l = BITMAPBYTESPERROW * BLOCKSDEPTH -
		 SCREENBYTESPERROW - fetchinfo[option_fetchmode].modulooffset;

	CopBPLMODA[1] = l;
	CopBPLMODB[1] = l;

	// display window start/stop
	CopDIWSTART[1] = DIWSTART;
	CopDIWSTOP[1] = DIWSTOP;

	// display data fetch start/stop
	CopDDFSTART[1] = fetchinfo[option_fetchmode].ddfstart;
	CopDDFSTOP[1]  = fetchinfo[option_fetchmode].ddfstop;

	// plane pointers

	wp = CopPLANE1H;

	for (l = 0;l < BLOCKSDEPTH;l++)	{
		wp[1] = (WORD)(((ULONG)ScreenBitmap->Planes[l]) >> 16);
		wp[3] = (WORD)(((ULONG)ScreenBitmap->Planes[l]) & 0xFFFF);
		wp += 4;
	}

	if (option_sky) {
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
	x = x / 8;
	y = y * BITMAPBYTESPERROW;

	block = mapdata[mapy * mapwidth + mapx];

	mapx = (block % BLOCKSPERROW) * (BLOCKWIDTH / 8);
	mapy = (block / BLOCKSPERROW) * (BLOCKPLANELINES * BLOCKSBYTESPERROW);

	if (option_how) OwnBlitter();

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

	if (option_how) DisownBlitter();
}

static void FillScreen(void)
{
	WORD a,b,x,y;

	for (b = 0;b < BITMAPBLOCKSPERCOL;b++) {
		for (a = 0;a < HALFBITMAPBLOCKSPERROW;a++) {
			x = a * BLOCKWIDTH;
			y = b * BLOCKPLANELINES;
			DrawBlock(x,y,a,b);
			DrawBlock(x + HALFBITMAPWIDTH,y,a,b);
		}
	}
}

static void ScrollLeft(void)
{
	WORD mapx,mapy,x,y;

	if (mapposx < 1) return;

	mapposx--;
	videoposx = mapposx % HALFBITMAPWIDTH;

	mapx = mapposx / BLOCKWIDTH;
	mapy = mapposx & (NUMSTEPS - 1);

	x = ROUND2BLOCKWIDTH(videoposx);
	y = mapy * BLOCKPLANELINES;

	DrawBlock(x,y,mapx,mapy);
	DrawBlock(x + HALFBITMAPWIDTH,y,mapx,mapy);
}

static void ScrollRight(void)
{
	WORD mapx,mapy,x,y;

	if (mapposx >= (mapwidth * BLOCKWIDTH - SCREENWIDTH - BLOCKWIDTH)) return;

	mapx = mapposx / BLOCKWIDTH + HALFBITMAPBLOCKSPERROW;
	mapy = mapposx & (NUMSTEPS - 1);

	x = ROUND2BLOCKWIDTH(videoposx);
	y = mapy * BLOCKPLANELINES;

	DrawBlock(x,y,mapx,mapy);
	DrawBlock(x + HALFBITMAPWIDTH,y,mapx,mapy);

	mapposx++;
	videoposx = mapposx % HALFBITMAPWIDTH;
}

static void CheckJoyScroll(void)
{
	WORD i,count;

	if (JoyFire()) count = 8; else count = 1;
	if (JoyLeft()) {
		for (i = 0; i < count; i++) ScrollLeft();
	}

	if (JoyRight())	{
		for (i = 0; i < count; i++) ScrollRight();
	}
}

static void UpdateCopperlist(void)
{
	ULONG pl;
	WORD	xpos,planeaddx,scroll,i;
	WORD	*wp;

	i = fetchinfo[option_fetchmode].scrollpixels;

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

	for (i = 0;i < BLOCKSDEPTH;i++)	{
		pl = ((ULONG)ScreenBitmap->Planes[i]) + planeaddx;
		wp[1] = (WORD)(pl >> 16);
		wp[3] = (WORD)(pl & 0xFFFF);
		wp += 4;
	}
}

static void ShowWhatCopperWouldDo(void)
{
	SetWriteMask(ScreenRastPort,1);
	SetAPen(ScreenRastPort,0);
	RectFill(ScreenRastPort,0,BITMAPHEIGHT + 1,BITMAPWIDTH - 1,BITMAPHEIGHT + 1);
	SetAPen(ScreenRastPort,1);
	RectFill(ScreenRastPort,videoposx + BLOCKWIDTH,BITMAPHEIGHT + 1,
             videoposx + BLOCKWIDTH + SCREENWIDTH - 1,BITMAPHEIGHT + 1);
}

static void MainLoop(void)
{
	if (!option_how) {
		// activate copperlist
		HardWaitBlit();
		WaitVBL();
		custom->copjmp2 = 0;
	}

	while (!LMBDown()) {
		if (!option_how) {
			WaitVBeam(199);
			WaitVBeam(200);
		} else Delay(1);

		if (option_speed) *(WORD *)0xdff180 = 0xFF0;

		CheckJoyScroll();

		if (option_speed) *(WORD *)0xdff180 = 0xF00;
		if (!option_how) UpdateCopperlist();
		else ShowWhatCopperWouldDo();
	}
}

/********************* MAIN *************************/

int main(int argc, char **argv)
{
	GetArguments();
	OpenMap();
	OpenBlocks();
	OpenDisplay();

	if (!option_how) {
		Delay(2*50);
		KillSystem();
		InitCopperlist();
	}
	FillScreen();
	MainLoop();

	if (!option_how) ActivateSystem();
	Cleanup(0);
    return 0;
}
