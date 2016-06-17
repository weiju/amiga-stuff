# Scrolling Tricks

## Description

This is a project to rework the Aminet Scrolling Tricks project by Georg Steger into a
version that has higher internal reuse, is smaller and compiles with VBCC. The purpose
is mainly to better understand the algorithms. By separating the common functionality such
as data loading and argument parsing the main source file can mainly focus on screen setup
and scrolling.

## System Requirements

This software requires V39 libraries and therefore only runs on the
3.x releases or greater of AmigaOS. The reason is that the original source
relies on the support for interleaved bitmaps in graphics.library, which was
not available in earlier versions.

I originally wanted to backport this to 1.x, but that would defeat the purpose
as a pure study project.

## Building

This project was built and tested on a cross-compiling system using VBCC on Linux.
Make sure your NDK_INC path is set to the directory that includes the NDK include
files for C.

The programs should all be built with

make

## Running

I used FS-UAE, but it should run on any classic 68k AmigaOS system starting from
version 3.0.


## Original source

Georg Steger's original Aminet submission:

http://aminet.net/package/dev/src/ScrollingTrick
