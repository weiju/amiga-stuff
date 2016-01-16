#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([dontshout],[1.0.0],[weiju.wu@gmail.com])

AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AC_CONFIG_HEADERS([config.h])

# we allow for cross-compilation on AROS and AmigaOS
AC_CANONICAL_HOST
case "$host" in
  m68k-*-amigaos) CC=vc CFLAGS="+aos68k $CFLAGS" ;;
  i386-*-aros) CC=i386-aros-gcc ;;
  x86_64-*-aros) CC=x86_64-aros-gcc ;;
  ppc-*-morphos) CC=ppc-morphos-gcc ;;
esac

# for now, we assume that AROS is always cross-compiled using GCC
# and AmigaOS 68k is always cross-compiled using VBCC
AM_CONDITIONAL([AROS], [test $host_os = "aros"])
AM_CONDITIONAL([MORPHOS], [test $host_os = "morphos"])
AM_CONDITIONAL([AMIGAOS4], [test $host_os = "amigaos4"])
AM_CONDITIONAL([VBCC], [test $host_os = "amigaos"])

# Checks for programs.
AC_PROG_RANLIB
AC_PROG_CC

# Checks for libraries.

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
