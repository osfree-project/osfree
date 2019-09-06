
dnl
dnl Determine if the platform can handle 64 bit binaries
dnl
dnl Assumes AC_CANONICAL_SYSTEM has already been called
dnl assumes we are on a 32bit system

dnl ---------------------- check 32/64 bit -----------------------
dnl allow --with-32bit to ensure that 32bit libraries are used
AC_ARG_ENABLE(32bit,
   [  --enable-32bit          specify if 32bit libraries are to be used],
   [bitflag32=yes],
   [bitflag32=no],
)
dnl allow --with-64bit to ensure that 64bit libraries are used
AC_ARG_ENABLE(64bit,
   [  --enable-64bit          specify if 64bit libraries are to be used],
   [bitflag64=yes],
   [bitflag64=no],
)

#
# Now do platform specific tests
#
gcc_64bit="-m64"
gcc_32bit="-m32"
on_osx="no"
osis64bit=no
bitflag=""
case "$target" in
   *hp-hpux*)
      ;;
   *ibm-aix*)
      rc=`lsconf -k | grep -c 64-bit`
      if test $rc -eq 1; then
         bitflag="64"
         osis64bit=yes
      fi
      #
      # AIX gcc has its own special bit switches :-(
      #
      gcc_64bit="-maix64"
      gcc_32bit="-maix32"
      ;;
   *dec-osf4*)
      ;;
   *dec-osf*)
      ;;
   *sequent-dynix*)
      ;;
   i*86*solaris*)
      ;;
   *solaris*)
      rc=`isainfo -v | grep -c 64-bit`
      if test $rc -eq 1; then
         bitflag="64"
         osis64bit=yes
      fi
      ;;
   sparc*sunos*)
      ;;
   *linux* | *kfreebsd*-gnu*)
      mach="`uname -m`"
      if test "$mach" = "x86_64" -o "$mach" = "ia86" -o "$mach" = "alpha" -o "$mach" = "ppc64" -o "$mach" = "s390x"; then
         bitflag="64"
         osis64bit=yes
      fi
      ;;
   *-freebsd* | *-openbsd*)
      mach="`uname -m`"
      if test "$mach" = "amd64"; then
         bitflag="64"
         osis64bit=yes
      fi
      ;;
   *nto-qnx*)
      ;;
   *qnx*)
      ;;
   *beos*)
      ;;
   *cygwin*)
      mach="`uname -m`"
      if test "$mach" = "x86_64"; then
         bitflag="64"
         osis64bit=yes
      fi
      ;;
   *apple-darwin*)
      on_osx="yes"
      osx_64bit=`sysctl hw.cpu64bit_capable | cut -f2 -d' '`
      if test $osx_64bit -eq 1; then
         bitflag="64"
         osis64bit=yes
      fi
      ;;
esac

if test "x$bitflag32" = "xyes" -a "x$bitflag64" = "xyes"; then
   AC_MSG_ERROR(--enable-32bit and --enable-64bit flags cannot both be specified.)
fi

if test "x$bitflag64" = "xyes" -a "$osis64bit" = "no"; then
   AC_MSG_ERROR(--enable-64bit cannot be specified on non-64bit OS.)
fi

binarybitprefix=""
if test "x$bitflag32" = "xyes"; then
   bitflag="32"
   if test "$osis64bit" = "yes"; then
      binarybitprefix="32"
   fi
elif test "x$bitflag64" = "xyes"; then
   bitflag="64"
fi
MACH_ARCH=`$ac_config_guess | cut -f1 -d-`
AC_SUBST(MACH_ARCH)
dnl
dnl following variable used to name 32bit binaries on a 64bit system
dnl allows 32bit and 64bit binaries t co-exist on a 64bit system
AC_SUBST(binarybitprefix)

dnl --------------- allow --with-arch to specify which architectures to build universal binaries
dnl
if test "$on_osx" = "yes"; then
   MH_CHECK_OSX_ARCH()
else
dnl
dnl Now set the correct compiler flags
dnl
   if test "$ac_cv_prog_CC" = "gcc" -o "$ac_cv_prog_CC" = "g++" -o "$ac_cv_prog_CC" = "clang"; then
      if test "$bitflag" = "32"; then
         CFLAGS="${CFLAGS} $gcc_32bit"
         LDFLAGS="${LDFLAGS} $gcc_32bit"
      elif test "$bitflag" = "64"; then
         CFLAGS="${CFLAGS} $gcc_64bit"
         LDFLAGS="${LDFLAGS} $gcc_64bit"
      else
         bitflag="32"
      fi
   elif test "$ac_cv_prog_CC" = "xlc" -o "$ac_cv_prog_CC" = "xlC"; then
      if test "$bitflag" = "32"; then
         CFLAGS="${CFLAGS} -q32"
         LDFLAGS="${LDFLAGS} -q32"
      elif test "$bitflag" = "64"; then
         CFLAGS="${CFLAGS} -q64"
         LDFLAGS="${LDFLAGS} -q64"
      else
         bitflag="32"
      fi
   fi
fi

dnl allow --disable-libdir-change to ensure that building a .deb works
#libdir_change=yes
AC_ARG_ENABLE(libdir-change,
   [  --enable-libdir-change  specify --disable-libdir-change if building a .deb],
   [libdir_change=$enableval],
   [libdir_change=yes],
)
if test "$libdir_change" = yes; then
   dnl
   dnl If the user hasn't specified libdir, check for specific lib dirs
   dnl
   if test "${libdir}" = '${exec_prefix}/lib'; then
   dnl
   dnl expand the active $prefix
   dnl
      myprefix=$prefix
      AC_DEFINE_DIR(MY_PREFIX,"$myprefix")
      if test "${myprefix}" = "NONE"; then
         myprefix="$ac_default_prefix"
      fi
   dnl
   dnl check for 64bit libdir
   dnl
      if test -d "${myprefix}/lib/x86_64-linux-gnu"; then
         libdir64="${myprefix}/lib/x86_64-linux-gnu"
      elif test -d "${myprefix}/lib64"; then
         libdir64="${myprefix}/lib64"
      else
         libdir64="${myprefix}/lib"
      fi
   dnl
   dnl check for 32bit libdir
   dnl
      if test -d "${myprefix}/lib/i386-linux-gnu"; then
         libdir32="${myprefix}/lib/i386-linux-gnu"
      elif test -d "${myprefix}/lib32"; then
         libdir32="${myprefix}/lib32"
      else
         libdir32="${myprefix}/lib"
      fi
   dnl
   dnl Depending on the build type, set the default libdir
   dnl
      if test "$bitflag" = "64"; then
         libdir="$libdir64"
      else
         libdir="$libdir32"
      fi
   else
      libdir64="$libdir"
      libdir32="$libdir"
   fi
else
   libdir64="$libdir"
   libdir32="$libdir"
fi
AC_SUBST(libdir64)
AC_SUBST(libdir32)
AC_SUBST(bitflag)
