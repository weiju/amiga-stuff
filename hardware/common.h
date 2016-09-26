#pragma once
#ifndef __COMMON_DEFS_H__
#define __COMMON_DEFS_H__

// Custom Chip Registers
#define BLTDDAT       0x000
#define DMACONR       0x002
#define VPOSR         0x004
#define VHPOSR        0x006

#define DMACON        0x096
#define BPL1PTH       0x0e0
#define BPL1PTL       0x0e2
#define BPL2PTH       0x0e4
#define BPL2PTL       0x0e6
#define BPL3PTH       0x0e8
#define BPL3PTL       0x0ea
#define BPL4PTH       0x0ec
#define BPL4PTL       0x0ee
#define BPL5PTH       0x0f0
#define BPL5PTL       0x0f2

#define BPLCON0       0x100
#define SPR0PTH       0x120
#define SPR0PTL       0x122
#define SPR1PTH       0x124
#define SPR1PTL       0x126
#define SPR2PTH       0x128
#define SPR2PTL       0x12a
#define SPR3PTH       0x12c
#define SPR3PTL       0x12e
#define SPR4PTH       0x130
#define SPR4PTL       0x132
#define SPR5PTH       0x134
#define SPR5PTL       0x136
#define SPR6PTH       0x138
#define SPR6PTL       0x13a
#define SPR7PTH       0x13c
#define SPR7PTL       0x13e
#define COLOR00       0x180


#define USE_PAL 1
#define DDFSTRT_VALUE      0x0038
#define DDFSTOP_VALUE      0x00d0
#define DIWSTRT_VALUE      0x2c81
#define DIWSTOP_VALUE_PAL  0x2cc1
#define DIWSTOP_VALUE_NTSC 0xf4c1

// max priority for this task
#define TASK_PRIORITY 127

#define VFREQ_PAL 50
#define WB_SCREEN_NAME "Workbench"

#ifdef USE_PAL
#define DIWSTOP_VALUE DIWSTOP_VALUE_PAL
#define NUM_RASTER_LINES 256
#else
#define DIWSTOP_VALUE DIWSTOP_VALUE_NTSC
#define NUM_RASTER_LINES 200
#endif

#define BPLCON0_COMPOSITE_COLOR (1 << 9)

// Macros

#define COP_MOVE(addr, data) addr, data
#define COP_WAIT_END  0xffff, 0xfffe

extern BOOL init_display(void);
extern void reset_display(void);
extern void waitmouse(void);

#endif /* __COMMON_DEFS_H__ */
