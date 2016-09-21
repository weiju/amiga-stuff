#pragma once
#ifndef __COMMON_DEFS_H__
#define __COMMON_DEFS_H__

// Custom Chip Registers
#define BPL1PTH       0x0e0
#define BPL1PTL       0x0e2
#define BPL2PTH       0x0e4
#define BPL2PTL       0x0e6

#define BPLCON0       0x100
#define COLOR00       0x180
#define SPR0PTH       0x120
#define SPR0PTL       0x122


#define USE_PAL 1
#define DDFSTRT_VALUE      0x0038
#define DDFSTOP_VALUE      0x00d0
#define DIWSTRT_VALUE      0x2c81
#define DIWSTOP_VALUE_PAL  0x2cc1
#define DIWSTOP_VALUE_NTSC 0xf4c1

#define EXEC_BASE (4L)

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

#endif /* __COMMON_DEFS_H__ */
