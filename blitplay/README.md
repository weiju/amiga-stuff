# Blitter Playground

This is an application to tinker with the Blitter under Amiga OS 2.x/3.x.
It was inspired by Blitlab, an application originally written for
Amiga OS 1.x.

The application was written and tested with the VBCC cross compiler
on GNU/Linux, and requires two environment variables to be set:

  * NDK_INC should point to your NDK C include directory
  * MUI_INC should point to your MUI 3.8 include directory

If VBCC, NDK and MUI were setup correctly, you should be
able to build the application on a GNU/Linux system with a simple

<code>make</code>

The resulting blitplay application can be executed on a classic
680x0 Amiga system or the corresponding UAE configuration.

This application was tested on an Amiga 4000 configuration on
FS-UAE, but it should work on any other 2.x/3.x 68k system as well.

-- Wei-ju Wu April 23rd, 2013

