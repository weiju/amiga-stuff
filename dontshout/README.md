## dontshout - A Commodity to remap Caps/Lock

### Description

Caps/Lock is a relic from typewriter times, a key that is not used often, overusing
it can be even considered as "shouting" and impolite. Unfortunately, it is also
one of the largest keys on modern keyboards.

Many users therefore remap Caps/lock to a more useful key, like Control or backspace
or deactivate it completely.

dontshout is a commodity for Amiga compatible computer systems, namely AmigaOS 2.x/3.x (68k),
Amiga 4.x (PPC), MorphOS (PPC) and AROS (i386, x86_64, 68k, ARM, PPC).

It maps Caps/Lock to a user-specified qualifier

### Usage

This program can be run from the CLI or from Workbench. Use it as you would
do with any other Commodity.

It recognizes the following tool types:

  * CX_PRIORITY - the commodities priority of the tool
  * QUALIFIER - the replacement qualifier (CONTROL|LSHIFT|RSHIFT|LALT|RALT|LAMIGA|RAMIGA)

### Compiling

Get the source code here: http://github.com/weiju/dontshout-amiga

To compile from this source directory, autoconf and automake are required, furthermore

  * AmigaOS 2.x/3.x: VBCC, NDK 3.9
  * AROS: cross tools and AROS SDK
  * AmigaOS 4.x: AmigaOS 4 SDK
  * MorphOS: MorphOS SDK

```
  autoreconf -i
  ./configure  --host=<your platform>
  make
```
