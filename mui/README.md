# Amiga MUI examples

## Description

This is a collection of example programs for classic AmigaOS, written using
the MUI 3.8 application framework.

-- Wei-ju Wu

## System Requirements

AmigaOS is still in use today, either in the classic Motorola 680x0 variant
or its modern successors, namely AmigaOS 4.x and MorphOS for PowerPC and
AROS for x86/amd64.

Programs that are not processor/hardware-specific should be easy portable
to these operating systems.

The code in this repository is mostly written for the traditional 68k platforms
though, so they might assume the presence of the Blitter, Copper as well as
CIAs and Paula.

The examples were mostly developed on a GNU/Linux system with

  * VBCC 0.9b (Amiga 68k target) - http://sun.hasenbraten.de/vbcc/
  * MUI 3.8 - http://sasg.com/mui/index.html

and tested on either FS-UAE with an A4000 configuration or a real A1200 and A4000

## Compiling

On a Linux system, it should be possible to build the examples by

cd <i>directory</i>
make

On a classic Amiga system with properly installed SAS/C 6.x, it should be
possible to use

cd <i>directory</i>
smake

