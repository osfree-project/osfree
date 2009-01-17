dnl
dnl Common configure switches related to debugging
dnl
dnl Uses following functions:
dnl

dnl --------------- allow --enable-debug to compile in debug mode --------
AC_ARG_ENABLE(debug,
   [  --enable-debug          turn on debugging],
   [with_debug=$enableval],
   [with_debug=no],
)
DEBUG=""
DEBUGGING=""
cflags_g="`echo $CFLAGS | grep -c '\-g'`"
cflags_O="`echo $CFLAGS | grep -c '\-O'`"

if test "$with_debug" = yes; then
   DEBUG="-DDEBUG"
   DEBUGGING="-g"
   if test "$cflags_g" = "0"; then
      CFLAGS="${CFLAGS} -g"
   fi
   if test "$cflags_O" != "0"; then
      CFLAGS="`echo ${CFLAGS} | sed -e s/-O.//`"
   fi
else
   DEBUG="-DNDEBUG"
   if test "$cflags_O" = "0"; then
      CFLAGS="${CFLAGS} -O"
   fi
   if test "$cflags_g" != "0"; then
      CFLAGS="`echo ${CFLAGS} | sed -e s/-g//`"
   fi
fi
if test "$ac_cv_prog_CC" = "gcc" -o "$ac_cv_prog_CC" = "g++"; then
   if test "$with_debug" = yes; then
      CFLAGS="${CFLAGS} -Wall"
   else
      CFLAGS="${CFLAGS} -O3 -Wall -fomit-frame-pointer -fno-strict-aliasing -Wno-char-subscripts"
   fi
fi
if test "$on_qnx4" = yes; then
   if test "$ac_cv_prog_CC" = "gcc"; then
      QNX4_CFLAGS=""
   else
      QNX4_CFLAGS="-Wc,-r -b -j -Wc,-ei -N4096000 -zp4 -mf -DNO_REGEX -DQNX -DQNX4 -D__QNX__ -DSTRINGS_ALIGNED -Q"
   fi
   if test "$with_debug" = yes; then
      CFLAGS="-g $QNX4_CFLAGS"
   else
      CFLAGS="-Otax $QNX4_CFLAGS"
   fi
fi
if test "$on_beos" = yes; then
   if test "$with_debug" = yes; then
      CFLAGS="${CFLAGS} -Wall"
   else
      CFLAGS="${CFLAGS} -O2 -Wall"
   fi
   if test "$datadir" = "\${prefix}/share"; then
      datadir="/boot/home/config/add-ons"
   fi
fi
AC_SUBST(DEBUG)
AC_SUBST(DEBUGGING)

dnl --------------- allow --enable-purify to enable linking with Purify
AC_ARG_ENABLE(purify,
   [  --enable-purify         link with Purify (TM)],
   [with_purify=$enableval],
   [with_purify=no],
)
if test "$with_purify" = yes; then
   PURIFY="purify"
else
   PURIFY=""
fi
AC_SUBST(PURIFY)

dnl --------------- allow --enable-efence to enable linking with Electric Fence
AC_ARG_ENABLE(efence,
   [  --enable-efence         link with Electric Fence],
   [with_efence=$enableval],
   [with_efence=no],
)
if test "$with_efence" = yes; then
   EFENCE="-lefence"
else
   EFENCE=""
fi
AC_SUBST(EFENCE)

dnl --------------- enable --enable-hpux-portable to enable HP-UX binaries to run on all chipsets
AC_ARG_ENABLE(hpux_portable,
	[  --enable-hpux-portable  make HP-UX binaries portable],
	[enable_hpux_portable=$enableval],
	[enable_hpux_portable=no],
)

dnl --------------- allow --with-dlfcnincdir to enable compiling with dlfcn.h on platforms that don't provide dl*() functions natively
AC_ARG_WITH(dlfcnincdir,
   [  --with-dlfcnincdir      location of dlfcn.h],
   [with_dlfcnincdir=$withval],
   [with_dlfcnincdir=no],
)
if test "$with_dlfcnincdir" = no; then
   DLFCNINCDIR=""
   AC_CHECK_HEADERS( dlfcn.h )
   if test "$ac_cv_header_dlfcn_h" = "yes" -o "$HAVE_DLFCN_H" = "1"; then
      HAVE_DLFCN_H="1"
   else
      #
      # Ignore the warning messages for specified platforms
      #
      case "$target" in
         *beos*)
            ;;
         *)
            AC_MSG_WARN(Could not find dlfcn.h in the standard header locations.)
            AC_MSG_WARN(If your platform does support dynamic loading ie you have dlfcn.h )
            AC_MSG_WARN(then you may need to specify the location of dlfcn.h with the --with-dlfcnincdir switch)
            AC_MSG_WARN(and the location of libdl.so with the --with-dlfcnlibdir switch.)
            ;;
      esac
   fi
else
   if test -f "$with_dlfcnincdir/dlfcn.h"; then
      DLFCNINCDIR="-I$with_dlfcnincdir"
      HAVE_DLFCN_H="1"
      AC_DEFINE(HAVE_DLFCN_H)
   else
      AC_MSG_ERROR(dlfcn.h not found in \"$with_dlfcnincdir\" as specified in --with-dlfcnincdir switch; cannot continue)
   fi
fi
AC_SUBST(DLFCNINCDIR)

MH_SHLPST
dnl --------------- allow --with-dlfcnlibdir to enable compiling with dlfcn.h on platforms that don't provide dl*() functions natively
AC_ARG_WITH(dlfcnlibdir,
   [  --with-dlfcnlibdir      location of libdl.a(AIX) libdl.dylib(MacOSX)],
   [with_dlfcnlibdir=$withval],
   [with_dlfcnlibdir=no],
)
if test "$with_dlfcnlibdir" = no; then
   DLFCNLIBDIR=""
else
   if test -f "$with_dlfcnlibdir/libdl$SHLPST"; then
      DLFCNLIBDIR="-L$with_dlfcnlibdir -ldl"
   else
      AC_MSG_ERROR(libdl$SHLPST not found in \"$with_dlfcnlibdir\" as specified in --with-dlfcnlibdir switch; cannot continue)
   fi
fi
AC_SUBST(DLFCNLIBDIR)
