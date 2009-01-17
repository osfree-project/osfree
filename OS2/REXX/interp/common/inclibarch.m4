
dnl
dnl Determine if the platform can handle 64 bit binaries
dnl
dnl Assumes AC_CANONICAL_SYSTEM has already been called
dnl assumes we are on a 32bit system
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
   *linux*)
      mach="`uname -m`"
      if test "$mach" = "x86_64" -o "$mach" = "ia86" -o "$mach" = "alpha" -o "$mach" = "ppc64"; then
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
      ;;
esac

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
dnl
dnl following variable used to name 32bit binaries on a 64bit system
dnl allows 32bit and 64bit binaries t co-exist on a 64bit system
AC_SUBST(binarybitprefix)

dnl
dnl Now set the correct compiler flags
dnl
if test "$ac_cv_prog_CC" = "gcc" -o "$ac_cv_prog_CC" = "g++"; then
   if test "$bitflag" = "32"; then
      CFLAGS="${CFLAGS} -m32"
      LDFLAGS="${LDFLAGS} -m32"
   elif test "$bitflag" = "64"; then
      CFLAGS="${CFLAGS} -m64"
      LDFLAGS="${LDFLAGS} -m64"
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

dnl
dnl If the user hasn't specified libdir, check if there is a /usr/lib32 or /usr/lib64
dnl
if test "$libdir" = '${exec_prefix}/lib'; then
   if test "$osis64bit" = "yes"; then
      if test -d "/usr/lib64"; then
         libdir64='${exec_prefix}/lib64'
         libdir32='${exec_prefix}/lib'
      elif test -d "/usr/lib32"; then
         libdir32='${exec_prefix}/lib32'
         libdir64='${exec_prefix}/lib'
      else
         libdir64='${exec_prefix}/lib64'
         libdir32='${exec_prefix}/lib'
      fi
   else
      libdir64='${exec_prefix}/lib'
      libdir32='${exec_prefix}/lib'
   fi
   if test "$bitflag" = "64"; then
      libdir="$libdir64"
   else
      libdir="$libdir32"
   fi
else
   libdir64="$libdir"
   libdir32="$libdir"
fi
AC_SUBST(libdir64)
AC_SUBST(libdir32)
AC_SUBST(bitflag)
