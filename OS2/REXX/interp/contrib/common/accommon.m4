dnl ---------------------------------------------------------------------------
dnl This file offers the following common macros...
dnl ---------------------------------------------------------------------------
dnl CHECK_REXX
dnl MH_IPC
dnl MH_CHECK_X_INC
dnl MH_CHECK_X_LIB
dnl MH_HAVE_PROTO
dnl MH_HAVE_BROKEN_CXX_WITH_STRING_H
dnl MH_PROG_CC
dnl MH_PROG_CXX
dnl MH_CHECK_X_HEADERS
dnl MH_CHECK_X_KEYDEFS
dnl MH_CHECK_X_TYPEDEF
dnl MH_TRY_LINK
dnl MH_CHECK_LIB
dnl MH_HOWTO_SHARED_LIBRARY
dnl MH_SHARED_LIBRARY
dnl MH_HOWTO_DYN_LINK
dnl MH_CHECK_CC_O
dnl MH_CHECK_LEADING_USCORE
dnl MH_SHLPST
dnl MH_CHECK_POSIX_THREADS
dnl MH_CHECK_RPM
dnl MH_CHECK_PACKAGE_BUILDERS
dnl MH_C_LONG_LONG
dnl MH_SHOW_STATUS
dnl MH_CHECK_OSX_ARCH
dnl MH_GET_KERNEL_NAME
dnl MH_GET_DISTRO_NAME
dnl MH_CHECK_CURSES
dnl MH_CHECK_FUNCS
dnl MH_CHECK_TYPES
dnl MH_SET_FULL_PACKAGE_NAME
dnl MH_LARGE_FILE_SUPPORT

dnl
dnl include the stdint.h wrapper
  # before sinclude(common/ax_create_stdint_h.m4)
sinclude(common/ax_create_stdint_h.m4)
dnl include the compare_version test
  # before sinclude(common/ax_compare_version.m4)
sinclude(common/ax_compare_version.m4)
dnl
dnl add our expansion macro for directories
dnl
  # before sinclude(common/ac_define_dir.m4)
sinclude(common/ac_define_dir.m4)
dnl ---------------------------------------------------------------------------
dnl Check REXX library and header files
dnl ---------------------------------------------------------------------------
AC_DEFUN([CHECK_REXX],
[
AC_REQUIRE([AC_CANONICAL_SYSTEM])
dnl
dnl Setup various things for different interpreters
dnl
extra_rexx_libs=""
extra_rexx_defines=""
extra_incdirs=""
extra_libdirs=""
REXX_USING_CONFIG=no
REXX_LIBS=""
REXX_INCLUDES=""
REXX_VER="unknown"
REXX_ADDONS="$libdir"
REXX_VER=""
case "$with_rexx" in
   regina)               dnl -------- Regina
      AC_DEFINE(USE_REGINA)
      AC_CHECK_PROG(regina_config, [regina-config], yes, no)
      rexx_h="rexxsaa.h"
      rexx_l="regina"
      REXX_INT="Regina"
      REXX_TARGET="Regina"
      if test "$ac_cv_prog_regina_config" = yes; then
         REXX_INCLUDES=`regina-config --cflags`
         numlines=`regina-config --libs$bitflag | wc -l`
         if test $numlines -eq 1; then
            REXX_LIBS=`regina-config --libs$bitflag`
         else
            REXX_LIBS=`regina-config --libs`
         fi
         REXX_INT="Regina (using regina-config)"
         REXX_VER=`regina-config --version`
         REXX_USING_CONFIG=yes
         regina_prefix=`regina-config --prefix`
         if test "$with_brew_addon_dir" = no ; then
            regina_addons=`regina-config --addons`
         else
            regina_addons="$with_brew_addon_dir"
         fi
         REGINA_ADDONS="$regina_addons"
         AX_COMPARE_VERSION([$REXX_VER],[gt],[3.6],[REXX_ADDONS=$REGINA_ADDONS],[REXX_ADDONS=$libdir])
      else
         case "$target" in
            *nto-qnx*)
               AC_SEARCH_LIBS(dlopen,dl)
            ;;
            *qnx*)
            ;;
            *hp-hpux*)
               AC_SEARCH_LIBS(shl_load,dld)
            ;;
            *)
               AC_SEARCH_LIBS(dlopen,dl)
            ;;
         esac
         AC_SEARCH_LIBS(crypt,crypt)
         AC_CHECK_PROG(flex,[flex],yes,no)
         if test "$ac_cv_prog_flex" = yes; then
            LEXLIB="-fl"
            AC_SUBST(LEXLIB)
         fi
      fi
      dnl for FreeBSD, the use of -pthread with ld doesn't work,
      dnl so remove it.
      case "$target" in
         *freebsd*)
            REXX_LIBS="`echo ${REXX_LIBS} | sed -e s/-pthread//`"
         ;;
         *)
         ;;
      esac
   ;;
   rexxtrans)            dnl -------- Rexx/Trans
      AC_DEFINE(USE_REXXTRANS)
      AC_CHECK_PROG(rexxtrans_config, [rexxtrans-config], yes, no)
      rexx_h="rexxtrans.h"
      rexx_l="rexxtrans"
      REXX_INT="Rexx/Trans"
      REXX_TARGET="RexxTrans"
      if test "$ac_cv_prog_rexxtrans_config" = yes; then
         REXX_INCLUDES=`rexxtrans-config --cflags`
dnl
dnl We now link with the static rexxtrans library
dnl
#         numlines=`rexxtrans-config --libs$bitflag | wc -l`
#         if test $numlines -eq 1; then
#            REXX_LIBS=`rexxtrans-config --libs$bitflag`
#         else
#            REXX_LIBS=`rexxtrans-config --libs`
#         fi
         REXX_LIBS=`rexxtrans-config --libs-static`
         REXX_INT="Rexx/Trans - static (using rexxtrans-config)"
         REXX_VER=`rexxtrans-config --version`
         REXX_USING_CONFIG=yes
      else
         AC_SEARCH_LIBS(dlopen,dl)
      fi
   ;;
   objrexx | orexx | objectrexx)              dnl -------- Object Rexx
   with_rexx="objrexx"
      AC_DEFINE(USE_OREXX)
      rexx_h="rexx.h"
      rexx_l="rexxapi"
      REXX_INT="Object Rexx"
      REXX_TARGET="ObjectRexx"
      extra_rexx_libs="-lrexx"
      extra_incdirs="/opt/orexx /usr/local/orexx /usr/lpp/orexx/include"
      extra_libdirs="/opt/orexx/lib /usr/local/orexx/lib /usr/lpp/orexx/lib"
      case "$target" in
         *linux*)
            extra_rexx_defines="-DLINUX"
         ;;
         *)
         ;;
      esac
   AC_SEARCH_LIBS(pthread_create,pthread pthreads thread)
   ;;
   oorexx)              dnl -------- Open Object Rexx
   with_rexx="oorexx"
      AC_DEFINE(USE_OOREXX) dnl ---- Open Object Rexx
      AC_CHECK_PROG(oorexx_config, [oorexx-config], yes, no)
      rexx_h="rexx.h"
      rexx_l="rexxapi"
      REXX_INT="Open Object Rexx"
      REXX_TARGET="ooRexx"
      if test "$ac_cv_prog_oorexx_config" = yes; then
         REXX_INCLUDES="-DLINUX `oorexx-config --cflags`"
         REXX_LIBS="`oorexx-config --libs` -ldl"
         REXX_INT="ooRexx (using oorexx-config)"
         REXX_VER=`oorexx-config --version`
         dnl if we are using ooRexx 4.x and greater treat it as another interpreter :-(
         REXX_MAJOR=`echo $REXX_VER | cut -d. -f1`
         if test "x$REXX_MAJOR" = "x4"; then
            AC_DEFINE(OOREXX_40)
         fi
         if test "x$REXX_MAJOR" = "x5"; then
            AC_DEFINE(OOREXX_40)
         fi
         REXX_USING_CONFIG=yes
      else
         extra_rexx_libs="-lrexx"
         extra_incdirs="/usr/local/include/ooRexx /usr/include/ooRexx /opt/ooRexx/include"
         extra_libdirs="/usr/local/lib/ooRexx /usr/lib/ooRexx /opt/ooRexx/lib/ooRexx"
         extra_rexx_defines="-DLINUX"
      fi
   AC_SEARCH_LIBS(pthread_create,pthread pthreads thread)
   ;;
   rexximc)              dnl -------- Rexx/imc
      AC_DEFINE(USE_REXXIMC)
      rexx_h="rexxsaa.h"
      rexx_l="rexx"
      REXX_INT="REXX/imc"
      REXX_TARGET="REXXimc"
      AC_SEARCH_LIBS(dlopen,dl)
   ;;
   rexx6000)             dnl -------- REXX6000
dnl
dnl Check that the OS supports REXX/6000
dnl Only supported it on AIX
dnl
      if test "$with_rexx6000" = yes ; then
      case "$target" in
         *aix*)
         ;;
         *)
            AC_MSG_ERROR(REXX/6000 support only available on AIX; cannot configure)
            ;;
      esac
   fi
      AC_DEFINE(USE_REXX6000)
      rexx_h="rexxtype.h"
      rexx_l="rexx"
      REXX_INT="REXX/6000"
      REXX_TARGET="REXX6000"
   ;;
   unirexx)              dnl -------- uni-REXX
      AC_DEFINE(USE_UNIREXX)
      rexx_h="rexxsaa.h"
      rexx_l="rx"
      REXX_INT="uni-REXX"
      REXX_TARGET="uni-REXX"
      AC_SEARCH_LIBS(dlopen,dl)
   ;;
   none)                 dnl -------- No Rexx interpreter
      AC_DEFINE(NOREXX)
      rexx_h=""
      rexx_l=""
      REXX_INT="No Rexx Support"
      REXX_TARGET=""
   ;;
   *)
      AC_MSG_ERROR(No Rexx interpreter specified with --with-rexx=int : must be one of: regina rexximc oorexx objrexx unirexx rexx6000 rexxtrans none)
   ;;
esac
AC_SUBST(REXX_TARGET)
AC_SUBST(REXX_ADDONS)
REXX_BUILD=$with_rexx
AC_SUBST(REXX_BUILD)
AC_SUBST(REXX_VER)

dnl look for REXX header and library, exit if not found

if test "xx$rexx_h" != "xx" -a "x$REXX_INCLUDES" = "x"; then
dnl look for REXX header and library, exit if not found
   AC_MSG_CHECKING(for location of Rexx header file: $rexx_h)
   mh_rexx_inc_dir=""
   mh_inc_dirs="\
       ${REXXINCDIR}             \
       ${extra_incdirs}          \
       ${HOME}/include           \
       ${HOME}/${MACHTYPE}/include     \
       /usr/local/include        \
       /usr/contrib/include      \
       /opt/include              \
       /opt/sfw/include          \
       /sw/include               \
       /usr/include              \
       /usr/unsupported/include"
dnl
dnl Provide for user supplying directory
dnl
   if test "$with_rexxincdir" != no ; then
      mh_inc_dirs="$with_rexxincdir $mh_inc_dirs"
   fi
dnl
dnl Try to determine the directory containing Rexx header
dnl
   for ac_dir in $mh_inc_dirs ; do
     if test -r $ac_dir/$rexx_h; then
       mh_rexx_inc_dir=$ac_dir
       break
     fi
   done
   if test "x$mh_rexx_inc_dir" != "x" ; then
      REXX_INCLUDES="-I$mh_rexx_inc_dir $extra_rexx_defines"
      AC_MSG_RESULT(found in $mh_rexx_inc_dir)
   else
      AC_MSG_ERROR(Cannot find Rexx header file: $rexx_h; cannot configure)
   fi
fi
AC_SUBST(REXX_INCLUDES)

if test "xx$rexx_h" != "xx" -a "x$REXX_LIBS" = "x"; then
   AC_MSG_CHECKING(for location of Rexx library file: $rexx_l)
   mh_rexx_lib_dir=""
   mh_lib_dirs="\
       ${REXXLIBDIR}             \
       ${extra_libdirs}          \
       ${HOME}/lib64             \
       ${HOME}/lib32             \
       ${HOME}/lib               \
       ${HOME}/${MACHTYPE}/lib64 \
       ${HOME}/${MACHTYPE}/lib32 \
       ${HOME}/${MACHTYPE}/lib   \
       /usr/local/lib64          \
       /usr/local/lib32          \
       /usr/local/lib            \
       /usr/contrib/lib          \
       /opt/lib64                \
       /opt/lib32                \
       /opt/lib                  \
       /opt/sfw/lib              \
       /sw/lib                   \
       /usr/lib64                \
       /usr/lib32                \
       /usr/lib                  \
       /usr/unsupported/lib"
dnl
dnl Provide for user supplying directory
dnl
   if test "$with_rexxlibdir" != no ; then
      mh_lib_dirs="$with_rexxlibdir $mh_lib_dirs"
   fi
dnl
dnl Try to determine the directory containing Rexx library
dnl
   for ac_dir in $mh_lib_dirs ; do
      for mh_ext in lib${rexx_l}.a lib${rexx_l}.so lib${rexx_l}.sl ${rexx_l}.lib lib${rexx_l}.dylib; do
        if test -r $ac_dir/$mh_ext; then
           mh_rexx_lib_dir=$ac_dir
         break 2
        fi
      done
   done
   if test "x$mh_rexx_lib_dir" != "x" ; then
      REXX_LIBS="-L$mh_rexx_lib_dir -l$rexx_l $extra_rexx_libs"
      AC_MSG_RESULT(found in $mh_rexx_lib_dir)
      AC_SUBST(rexxtrans_lib_name)
   else
      AC_MSG_ERROR(Cannot find Rexx library file: $rexx_l; cannot configure)
   fi
fi
AC_SUBST(REXX_LIBS)
])dnl

dnl ---------------------------------------------------------------------------
dnl Determine if the system has System V IPC. ie sys/ipc.h and sys/shm.h
dnl headers.
dnl ---------------------------------------------------------------------------
AC_DEFUN([MH_IPC],
[
AC_MSG_CHECKING(for System V IPC support)
AC_CHECK_HEADER(sys/ipc.h)
if test $ac_cv_header_sys_ipc_h = no; then
   AC_MSG_ERROR(Cannot find required header file sys/ipc.h; XCurses cannot be configured)
fi
])dnl

dnl ---------------------------------------------------------------------------
dnl Set up the correct X header file location
dnl ---------------------------------------------------------------------------
AC_DEFUN([MH_CHECK_X_INC],
[
AC_MSG_CHECKING(for location of X headers)
mh_x11_dir=""
mh_x11_xaw_dir=""
dnl
dnl specify latest release of X directories first
dnl
mh_inc_dirs="\
    $HOME/include/X11         \
    $HOME/include             \
    /tmp/include/X11          \
    /tmp/include              \
    /usr/X11R6/include        \
    /usr/include/X11R6        \
    /usr/local/X11R6/include  \
    /usr/local/include/X11R6  \

    /usr/X11R5/include        \
    /usr/include/X11R5        \
    /usr/local/X11R5/include  \
    /usr/local/include/X11R5  \
    /usr/local/x11r5/include  \

    /usr/X11R4/include        \
    /usr/include/X11R4        \
    /usr/local/X11R4/include  \
    /usr/local/include/X11R4  \
                              \
    /usr/X11/include          \
    /usr/include/X11          \
    /usr/local/X11/include    \
    /usr/local/include/X11    \
                              \
    /usr/X386/include         \
    /usr/x386/include         \
    /usr/XFree86/include/X11  \
                              \
    /usr/include              \
    /usr/local/include        \
    /usr/unsupported/include  \
    /usr/athena/include       \
    /usr/lpp/Xamples/include  \
                              \
    /usr/openwin/include      \
    /usr/openwin/share/include"
dnl
dnl Provide for user supplying directory
dnl
if test "x$x_includes" != xNONE ; then
   mh_inc_dirs="$x_includes $mh_inc_dirs"
fi

dnl
dnl Try to determine the directory containing X headers
dnl We will append X11 to all the paths above as an extra check
dnl
for ac_dir in $mh_inc_dirs ; do
  if test -r $ac_dir/Intrinsic.h; then
    mh_x11_dir=$ac_dir
    break
  fi
  if test -r $ac_dir/X11/Intrinsic.h; then
    mh_x11_dir="$ac_dir/X11"
    break
  fi
done

dnl
dnl Try to determine the directory containing Xaw headers
dnl We will append X11 to all the paths above as an extra check
dnl
if test "$with_xaw3d" = yes; then
   mh_xaw_dir="Xaw3d"
else
   if test "$with_nextaw" = yes; then
      mh_xaw_dir="neXtaw"
   else
      mh_xaw_dir="Xaw"
   fi
fi
for ac_dir in $mh_inc_dirs ; do
  if test -r $ac_dir/$mh_xaw_dir/Box.h; then
    mh_x11_xaw_dir=$ac_dir
    break
  fi
  if test -r $ac_dir/X11/$mh_xaw_dir/Box.h; then
    mh_x11_xaw_dir="$ac_dir/X11"
    break
  fi
done

if test "x$mh_x11_dir" != "x" ; then
   mh_x11_dir_no_x11=`echo $mh_x11_dir | sed 's/\/X11$//'`
dnl
dnl Test to see if $mh_x11_dir_no_x11 is /usr/include and we are using gcc
dnl under Solaris. If so, ignore it.
   AC_REQUIRE([AC_CANONICAL_SYSTEM])
   mh_solaris_gcc_usr_include="no"
   case "$target" in
      *solaris*)
         if test "$ac_cv_prog_CC" = "gcc" -a "$mh_x11_dir_no_x11" = "/usr/include" ; then
            mh_solaris_gcc_usr_include="yes"
         fi
         ;;
   esac
   if test "$mh_x11_dir_no_x11" != "$mh_x11_dir" -a "$mh_solaris_gcc_usr_include" = "no" ; then
      MH_XINC_DIR="-I$mh_x11_dir -I$mh_x11_dir_no_x11"
   else
      MH_XINC_DIR="-I$mh_x11_dir"
   fi
else
   AC_MSG_ERROR(Cannot find required header file Intrinsic.h; XCurses cannot be configured)
fi

if test "x$mh_x11_xaw_dir" != "x" ; then
   MH_XINC_DIR="-I$mh_x11_xaw_dir $MH_XINC_DIR"
else
   AC_MSG_ERROR(Cannot find required Xaw header file Box.h; XCurses cannot be configured)
fi

   AC_MSG_RESULT(found in $mh_x11_dir $mh_x11_xaw_dir)
   AC_SUBST(MH_XINC_DIR)

])dnl

dnl ---------------------------------------------------------------------------
dnl Set up the correct X library file location
dnl ---------------------------------------------------------------------------
AC_DEFUN([MH_CHECK_X_LIB],
[
dnl
dnl Some systems require extra libraries...
dnl
mh_solaris_flag=no
mh_hpux9_flag=no
AC_REQUIRE([AC_CANONICAL_SYSTEM])
case "$target" in
   *solaris*)
      mh_solaris_flag=yes
      ;;
   *pc-sco*)
      extra_x_libs="Xext"
      ;;
   sparc*sunos*)
      extra_x_libs="Xext"
      if test "$ac_cv_prog_CC" = "gcc" ; then
         extra_ld_flags="-Wl,-Bstatic"
         extra_ld_flags2="-Wl,-Bdynamic"
      else
         extra_ld_flags="-Bstatic"
         extra_ld_flags2="-Bdynamic"
      fi
      ;;
   *hpux9*)
      mh_hpux9_flag=yes
      ;;
esac

AC_MSG_CHECKING(for location of X libraries)
if test "$with_xaw3d" = yes; then
   MH_X11_LIBS="Xaw3d Xmu Xt X11"
else
   if test "$with_nextaw" = yes; then
      MH_X11_LIBS="neXtaw Xmu Xt X11"
   else
      MH_X11_LIBS="Xaw Xmu Xt X11"
   fi
fi
MH_X11R6_LIBS="Xext Xpm"
mh_x11r6=no

which dpkg-architecture > /dev/null
if test $? -eq 0; then
   multiarch_libdir="/usr/lib/`dpkg-architecture -qDEB_HOST_MULTIARCH`"
else
   multiarch_libdir=""
fi
dnl
dnl specify latest release of X directories first
dnl
mh_lib_dirs="$multiarch_libdir \
    $HOME/lib64           \
    $HOME/lib32           \
    $HOME/lib             \
    /tmp/lib              \
    /usr/X11R6/lib64      \
    /usr/X11R6/lib32      \
    /usr/X11R6/lib        \
    /usr/lib64/X11R6      \
    /usr/lib32/X11R6      \
    /usr/lib/X11R6        \
    /usr/local/X11R6/lib64  \
    /usr/local/X11R6/lib32  \
    /usr/local/X11R6/lib  \
    /usr/local/lib64/X11R6  \
    /usr/local/lib32/X11R6  \
    /usr/local/lib/X11R6  \

    /usr/X11R5/lib        \
    /usr/lib/X11R5        \
    /usr/local/X11R5/lib  \
    /usr/local/lib/X11R5  \
    /usr/local/x11r5/lib  \

    /usr/X11R4/lib        \
    /usr/lib/X11R4        \
    /usr/local/X11R4/lib  \
    /usr/local/lib/X11R4  \
                          \
    /usr/X11/lib          \
    /usr/lib/X11          \
    /usr/local/X11/lib    \
    /usr/local/lib/X11    \
                          \
    /usr/X386/lib         \
    /usr/x386/lib         \
    /usr/XFree86/lib/X11  \
                          \
    /usr/lib64            \
    /usr/lib32            \
    /usr/lib              \
    /usr/local/lib64      \
    /usr/local/lib32      \
    /usr/local/lib        \
    /usr/unsupported/lib  \
    /usr/athena/lib       \
    /usr/lpp/Xamples/lib  \
                          \
    /usr/openwin/lib      \
    /usr/openwin/share/lib"
dnl
dnl Provide for user supplying directory
dnl
if test "x$x_libraries" != xNONE ; then
   mh_lib_dirs="$x_libraries $mh_lib_dirs"
fi

dnl
dnl try to find libSM.[a,sl,so,dylib]. If we find it we are using X11R6
dnl
for ac_dir in $mh_lib_dirs ; do
   for mh_xsm in libSM.a libSM.so libSM.sl libSM.dylib; do
     if test -r $ac_dir/$mh_xsm; then
       mh_x11r6=yes
       break 2
     fi
   done
done

if test "$mh_x11r6" = yes ; then
   mh_libs="$MH_X11_LIBS $MH_X11R6_LIBS"
else
   mh_libs="$MH_X11_LIBS $extra_x_libs"
fi

dnl
dnl Ensure that all required X libraries are found
dnl
mh_prev_dir=""
mh_where_found=""
mh_where_found_dirs=""
mh_solaris_path=""
for mh_lib in $mh_libs; do
  mh_lib_found=no
  for ac_dir in $mh_lib_dirs ; do
    for mh_ext in a so sl dll.a; do
      if test -r $ac_dir/lib$mh_lib.$mh_ext; then
        if test "x$mh_prev_dir" != "x$ac_dir" ; then
          if test "x$mh_prev_dir" = "x" ; then
             mh_where_found="$mh_where_found found in $ac_dir"
          else
             mh_where_found="$mh_where_found and in $ac_dir"
          fi
          mh_prev_dir=$ac_dir
          mh_where_found_dirs="$mh_where_found_dirs $ac_dir"
          MH_XLIBS="$MH_XLIBS -L$ac_dir"
          mh_solaris_path="${mh_solaris_path}:$ac_dir"
        fi
        MH_XLIBS="$MH_XLIBS -l$mh_lib"
        mh_lib_found=yes
        break 2
      fi
    done
  done
  if test "$mh_lib_found" = no; then
    AC_MSG_ERROR(Cannot find required X library; lib$mh_lib. XCurses cannot be configured)
  fi
done
AC_MSG_RESULT($mh_where_found)
mh_solaris_path=`echo $mh_solaris_path | sed 's/^://'`
if test "$mh_solaris_flag" = yes ; then
   MH_XLIBS="-R$mh_solaris_path $extra_ld_flags $MH_XLIBS $extra_libs $extra_ld_flags2"
else
   MH_XLIBS="$extra_ld_flags $MH_XLIBS $extra_libs $extra_ld_flags2"
fi
if test "$mh_hpux9_flag" = yes ; then
  grep -q XtSetLanguageProc $mh_x11_dir/Intrinsic.h
  if test $? -eq 0 ; then
    mh_found_xtshellstrings=no
    for mh_acdir in $mh_where_found_dirs ; do
      for mh_xaw in `ls $mh_acdir/libXaw.*` ; do
        nm $mh_xaw | grep XtShellStrings | grep -qv extern
        if test $? -eq 0 ; then
          mh_found_xtshellstrings=yes
        fi
      done
    done
    if test "$mh_found_xtshellstrings" = no ; then
      AC_MSG_WARN(The X11 development environment has not been installed correctly.)
      AC_MSG_WARN(The header file; Intrinsic.h, is for X11R5 while the Athena Widget)
      AC_MSG_WARN(Set library; libXaw is for X11R4.  This is a common problem with)
      AC_MSG_WARN(HP-UX 9.x.)
      AC_MSG_WARN(A set of required X11R5 library files can be obtained from the)
      AC_MSG_WARN(anonymous ftp sites listed on the PDCurses WWW home page:)
      AC_MSG_WARN(http://www.lightlink.com/hessling/)
      AC_MSG_WARN(The file is called HPUX-9.x-libXaw-libXmu.tar.Z)
      AC_MSG_ERROR(X11 installation incomplete; cannot continue)
    fi
  fi
fi
   AC_SUBST(MH_XLIBS)
])dnl

dnl ---------------------------------------------------------------------------
dnl Determine if C compiler handles ANSI prototypes
dnl ---------------------------------------------------------------------------
AC_DEFUN([MH_HAVE_PROTO],
[
AC_MSG_CHECKING(if compiler supports ANSI prototypes)
dnl
dnl override existing value of $ac_compile so we use the correct compiler
dnl SHOULD NOT NEED THIS
dnl
ac_compile='$ac_cv_prog_CC conftest.$ac_ext $CFLAGS $CPPFLAGS -c 1>&5 2>&5'
AC_TRY_COMPILE([#include <stdio.h>],
[extern int xxx(int, char *);],
  mh_have_proto=yes; AC_DEFINE(HAVE_PROTO), mh_have_proto=no )
AC_MSG_RESULT($mh_have_proto)
])dnl

dnl -----------------------------------------------------
dnl Determine if compiling <string.h> breaks g++ compiler
dnl -----------------------------------------------------
AC_DEFUN([MH_HAVE_BROKEN_CXX_WITH_STRING_H],
[
AC_MSG_CHECKING(if g++ compiler cannot compile <string.h>)
dnl
dnl override existing value of $ac_compile so we use the correct compiler
dnl SHOULD NOT NEED THIS
dnl
ac_compile='$ac_cv_prog_CC conftest.$ac_ext $CFLAGS $CPPFLAGS -c 1>&5 2>&5'
AC_TRY_COMPILE([#include <string.h>],
[extern int xxx(int, char *);],
  mh_have_broken_cxx_with_string_h=yes; AC_DEFINE(HAVE_BROKEN_CXX_WITH_STRING_H), mh_have_broken_cxx_with_string_h=no )
AC_MSG_RESULT($mh_have_broken_cxx_with_string_h)
])dnl

dnl ---------------------------------------------------------------------------
dnl Determine the best C compiler to use given a list
dnl ---------------------------------------------------------------------------
AC_DEFUN([MH_PROG_CC],
[
mh_sysv_incdir=""
mh_sysv_libdir=""
all_words="clang gcc xlc c99 c89 acc cc"
ac_dir=""
AC_MSG_CHECKING(for one of the following C compilers: $all_words)
AC_CACHE_VAL(ac_cv_prog_CC,[
if test -n "$CC"; then
  ac_cv_prog_CC="$CC" # Let the user override the test.
else
  IFS="${IFS= 	}"; ac_save_ifs="$IFS"; IFS="${IFS}:"
  for mh_cc in $all_words; do
    for ac_dir in $PATH; do
      test -z "$ac_dir" && ac_dir=.
      if test -f $ac_dir/$mh_cc; then
        ac_cv_prog_CC="$mh_cc"
        if test "$ac_dir" = "/usr/5bin"; then
          mh_sysv_incdir="/usr/5include"
          mh_sysv_libdir="/usr/5lib"
        fi
        break 2
      fi
    done
  done
  IFS="$ac_save_ifs"
  test -z "$ac_cv_prog_CC" && ac_cv_prog_CC="cc"
fi
CC="$ac_cv_prog_CC"
])
AC_SUBST(CC)
if test "$ac_dir" = ""; then
   AC_MSG_RESULT(using $ac_cv_prog_CC specified in CC env variable)
else
   AC_MSG_RESULT(using $ac_dir/$ac_cv_prog_CC)
fi
])dnl

dnl ---------------------------------------------------------------------------
dnl Determine the best C++ compiler to use given a list
dnl ---------------------------------------------------------------------------
AC_DEFUN([MH_PROG_CXX],
[
mh_sysv_incdir=""
mh_sysv_libdir=""
all_words="$CC_LIST"
ac_dir=""
AC_MSG_CHECKING(for one of the following C++ compilers: $all_words)
AC_CACHE_VAL(ac_cv_prog_CXX,[
if test -n "$CXX"; then
  ac_cv_prog_CXX="$CC" # Let the user override the test.
else
  IFS="${IFS= 	}"; ac_save_ifs="$IFS"; IFS="${IFS}:"
  for mh_cc in $all_words; do
    for ac_dir in $PATH; do
      test -z "$ac_dir" && ac_dir=.
      if test -f $ac_dir/$mh_cc; then
        ac_cv_prog_CXX="$mh_cc"
        break 2
      fi
    done
  done
  IFS="$ac_save_ifs"
  test -z "$ac_cv_prog_CXX" && ac_cv_prog_CXX="c++"
fi
CXX="$ac_cv_prog_CXX"
])
AC_SUBST(CXX)
if test "$ac_dir" = ""; then
   AC_MSG_RESULT(using $ac_cv_prog_CXX specified in CXX env variable)
else
   AC_MSG_RESULT(using $ac_dir/$ac_cv_prog_CXX)
fi
])dnl

dnl ---------------------------------------------------------------------------
dnl Determine if the supplied X headers exist.
dnl ---------------------------------------------------------------------------
AC_DEFUN([MH_CHECK_X_HEADERS],
[
save_CPPFLAGS="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS $MH_XINC_DIR"
for mh_header in $1; do
   AC_CHECK_HEADERS($mh_header)
done
CPPFLAGS="$save_CPPFLAGS"
])dnl

dnl ---------------------------------------------------------------------------
dnl Determine if various key definitions exist in keysym.h
dnl ---------------------------------------------------------------------------
AC_DEFUN([MH_CHECK_X_KEYDEFS],
[
save_CPPFLAGS="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS $MH_XINC_DIR"
for mh_keydef in $1; do
   AC_MSG_CHECKING(for $mh_keydef in keysym.h)
   mh_upper_name="HAVE_`echo $mh_keydef | tr '[a-z]' '[A-Z]'`"
   AC_TRY_COMPILE([#include <keysym.h>],
[int i = $mh_keydef;],
  mh_have_key=yes; AC_DEFINE_UNQUOTED($mh_upper_name,1), mh_have_key=no )
   AC_MSG_RESULT($mh_have_key)
done
CPPFLAGS="$save_CPPFLAGS"
])dnl

dnl ---------------------------------------------------------------------------
dnl Determine if supplied types have been typedefed
dnl ---------------------------------------------------------------------------
AC_DEFUN([MH_CHECK_X_TYPEDEF],
[
save_CPPFLAGS="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS $SYS_DEFS $MH_XINC_DIR"
for td in $1 ; do
AC_MSG_CHECKING(if $td is typedefed:)
AC_TRY_COMPILE(
[#include "X11/Xlib.h"],
[$td fred],
   [mh_td=yes],
   [mh_td=no]
)
if test "$mh_td" = yes ; then
   TD_upper=`echo $td | tr a-z A-Z`
   AC_DEFINE_UNQUOTED(${TD_upper}_TYPEDEFED, 1)
fi
AC_MSG_RESULT($mh_td)
done
CPPFLAGS="$save_CPPFLAGS"
])dnl

dnl ---------------------------------------------------------------------------
dnl Provide our own AC_TRY_LINK
dnl ---------------------------------------------------------------------------
AC_DEFUN([MH_TRY_LINK],
[
mh_link='${LD-ld} -o conftest $CFLAGS $CPPFLAGS $LDFLAGS conftest.$ac_ext $LIBS 1>&AC_FD_CC'
save_CPPFLAGS="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS $MH_XINC_DIR"
for mh_keydef in $1; do
   AC_MSG_CHECKING(for $mh_keydef in keysym.h)
   mh_upper_name="HAVE_`echo $mh_keydef | tr '[a-z]' '[A-Z]'`"
   AC_TRY_COMPILE([#include <keysym.h>],
[int i = $mh_keydef;],
  mh_have_key=yes; AC_DEFINE_UNQUOTED($mh_upper_name,1), mh_have_key=no )
   AC_MSG_RESULT($mh_have_key)
done
CPPFLAGS="$save_CPPFLAGS"
])dnl

dnl ---------------------------------------------------------------------------
dnl Check for presense of various libraries
dnl ---------------------------------------------------------------------------
AC_DEFUN([MH_CHECK_LIB],
[
MH_EXTRA_LIBS=''
for mh_lib in $1; do
   if test "$on_qnx4" = yes -a "$ac_cv_prog_CC" != "gcc"; then
      AC_MSG_CHECKING(for library -l${mh_lib})
      if test -r /usr/lib/${mh_lib}3r.lib; then
         AC_MSG_RESULT(found)
         MH_EXTRA_LIBS="${MH_EXTRA_LIBS} -l${mh_lib}"
      else
         AC_MSG_RESULT(not found)
      fi
   else
      AC_CHECK_LIB($mh_lib,main,mh_lib_found=yes,mh_lib_found=no)
      if test "$mh_lib_found" = yes; then
         MH_EXTRA_LIBS="${MH_EXTRA_LIBS} -l${mh_lib}"
      fi
   fi
done
])dnl

dnl ---------------------------------------------------------------------------
dnl Work out how to create a shared library
dnl ---------------------------------------------------------------------------
AC_DEFUN([MH_HOWTO_SHARED_LIBRARY],
[
AC_MSG_CHECKING(how to create a shared library)
mh_compile='${CC-cc} -c $DYN_COMP conftest.$ac_ext 1>&AC_FD_CC'
cat > conftest.$ac_ext <<EOF
dnl [#]line __oline__ "[$]0"
[#]line __oline__ "configure"
int foo()
{
return(0);
}
EOF
if AC_TRY_EVAL(mh_compile) && test -s conftest.o; then
   mh_dyn_link='ld -shared -o conftest.so.1.0 conftest.o -lc 1>&AC_FD_CC'
   if AC_TRY_EVAL(mh_dyn_link) && test -s conftest.so.1.0; then
      SHL_LD="ld -shared -o ${SHLPRE}${SHLFILE}${SHLPST} "'$('SHOFILES')'" -lc"
   else
      mh_dyn_link='ld -G -o conftest.so.1.0 conftest.o 1>&AC_FD_CC'
      if AC_TRY_EVAL(mh_dyn_link) && test -s conftest.so.1.0; then
         SHL_LD="ld -G -o ${SHLPRE}${SHLFILE}${SHLPST} "'$('SHOFILES')'
      else
         mh_dyn_link='ld -o conftest.so.1.0 -shared -no_archive conftest.o  -lc 1>&AC_FD_CC'
         if AC_TRY_EVAL(mh_dyn_link) && test -s conftest.so.1.0; then
            SHL_LD="ld -o ${SHLPRE}${SHLFILE}${SHLPST} -shared -no_archive "'$('SHOFILES')'" -lc"
         else
            mh_dyn_link='ld -b -o conftest.so.1.0 conftest.o 1>&AC_FD_CC'
            if AC_TRY_EVAL(mh_dyn_link) && test -s conftest.so.1.0; then
               SHL_LD="ld -b -o ${SHLPRE}${SHLFILE}${SHLPST} "'$('SHOFILES')'
            else
               mh_dyn_link='ld -Bshareable -o conftest.so.1.0 conftest.o 1>&AC_FD_CC'
               if AC_TRY_EVAL(mh_dyn_link) && test -s conftest.so.1.0; then
                  SHL_LD="ld -Bshareable -o ${SHLPRE}${SHLFILE}${SHLPST} "'$('SHOFILES')'
               else
                  mh_dyn_link='ld -assert pure-text -o conftest.so.1.0 conftest.o 1>&AC_FD_CC'
                  if AC_TRY_EVAL(mh_dyn_link) && test -s conftest.so.1.0; then
                     SHL_LD="ld -assert pure-text -o ${SHLPRE}${SHLFILE}${SHLPST} "'$('SHOFILES')'
                  else
                     mh_dyn_link='${CC} -shared -o conftest.so.1.0 conftest.o -lc 1>&AC_FD_CC'
                     if AC_TRY_EVAL(mh_dyn_link) && test -s conftest.so.1.0; then
                        SHL_LD="${CC} -shared -o ${SHLPRE}${SHLFILE}${SHLPST} "'$('SHOFILES')'" -lc"
                     else
                        SHL_LD=""
                     fi
                  fi
               fi
            fi
         fi
      fi
   fi
fi
if test "$SHL_LD" = ""; then
   AC_MSG_RESULT(unknown)
else
   AC_MSG_RESULT(found)
fi
rm -f conftest*
])

dnl ---------------------------------------------------------------------------
dnl Work out how to create a dynamically loaded module
dnl ---------------------------------------------------------------------------
AC_DEFUN([MH_HOWTO_DYN_LINK],
[
mh_compile='${CC-cc} -c $DYN_COMP conftest.$ac_ext 1>&AC_FD_CC'
cat > conftest.$ac_ext <<EOF
dnl [#]line __oline__ "[$]0"
[#]line __oline__ "configure"
int foo()
{
return(0);
}
EOF
if AC_TRY_EVAL(mh_compile) && test -s conftest.o; then
   mh_dyn_link='ld -shared -o conftest.rxlib conftest.o -lc 1>&AC_FD_CC'
   if AC_TRY_EVAL(mh_dyn_link) && test -s conftest.rxlib; then
      LD_RXLIB1="ld -shared"
      LD_RXTRANSLIB1="$LD_RXLIB1"
      LD_RXLIB2="${REXX_LIBS}"
      SHLPRE="lib"
      SHLPST=".so"
      RXLIBLEN="6"
   else
      mh_dyn_link='ld -G -o conftest.rxlib conftest.o 1>&AC_FD_CC'
      if AC_TRY_EVAL(mh_dyn_link) && test -s conftest.rxlib; then
         LD_RXLIB1="ld -G"
         LD_RXTRANSLIB1="$LD_RXLIB1"
         LD_RXLIB2="${REXX_LIBS}"
         SHLPRE="lib"
         SHLPST=".so"
         RXLIBLEN="6"
      else
         mh_dyn_link='${CC} -shared -o conftest.rxlib conftest.o 1>&AC_FD_CC'
         if AC_TRY_EVAL(mh_dyn_link) && test -s conftest.rxlib; then
            LD_RXLIB1="${CC} -shared"
            LD_RXTRANSLIB1="$LD_RXLIB1"
            LD_RXLIB2="${REXX_LIBS}"
            SHLPRE="lib"
            SHLPST=".so"
            RXLIBLEN="6"
         else
            LD_RXLIB1=""
            LD_RXTRANSLIB1="$LD_RXLIB1"
            LD_RXLIB2=""
            SHLPRE=""
            SHLPST=""
            RXLIBLEN="0"
         fi
      fi
   fi
fi
rm -f conftest*
])dnl

dnl ---------------------------------------------------------------------------
dnl Determine how to build shared libraries etc..
dnl ---------------------------------------------------------------------------
AC_DEFUN([MH_SHARED_LIBRARY],
[
AC_REQUIRE([MH_SHLPST])
dnl
dnl If compiler is gcc, then flags should be the same for all platforms
dnl (just guessing on this)
dnl
OSAVE=".o.save"
OBJ="o"
EXE=""
STATIC_LDFLAGS=""
DYNAMIC_LDFLAGS=""
AIX_DYN="no"
BEOS_DYN="no"
SHLFILE="$1"
SHLFILES="$*"
RXPACKEXPORTS=""
BASE_INSTALL="installbase"
BASE_BINARY="binarybase"
SHLPRE="lib"
LD_RXLIB1=""
LD_RXTRANSLIB1="$LD_RXLIB1"
CAN_USE_ABI="no"
LIBEXE="ar"
LIBFLAGS="cr"
LIBPRE="lib"
LIBPST=".a"
RXDEFEXP1=""
# OTHER_INSTALLS is set to install a non-version numbered shared library
# by default; ie. no .\$(ABI) suffix. If the regina executable is not built,
# then there is no shared library. Set OTHER_INSTALLS="installabilib" if you
# are building a version numbered shared library.
OTHER_INSTALLS="installlib"
EXTRATARGET=""

AC_REQUIRE([AC_CANONICAL_SYSTEM])
case "$target" in
   *hp-hpux*)
      SYS_DEFS="-D_HPUX_SOURCE"
      EEXTRA="-Wl,-E"
      LD_RXLIB1="ld -b -q -n  ${LDFLAGS}"
      LD_RXTRANSLIB1="$LD_RXLIB1"
      DYNAMIC_LDFLAGS="-Wl,+s"
      ;;
   *ibm-aix*)
      if test "$with_rexx6000" = yes; then
         mh_entry="-eInitFunc"
      else
         mh_entry="-bnoentry"
      fi
      SYS_DEFS="-D_ALL_SOURCE -DAIX"
      AIX_DYN="yes"
      DYN_COMP="-DDYNAMIC"
      STATIC_LDFLAGS="-bnso -bI:/lib/syscalls.exp"
      if test "$ac_cv_prog_CC" = "gcc"; then
         LD_RXLIB1="${CC} -Wl,$mh_entry -Wl,-bM:SRE  ${LDFLAGS}"
      else
         LD_RXLIB1="${CC} $mh_entry -bM:SRE  ${LDFLAGS}"
      fi
      LD_RXTRANSLIB1="$LD_RXLIB1"
      RXPACKEXPORTS="-bE:$SHLFILE.exp"
      RXPACKEXP="$SHLFILE.exp"
      ;;
   powerpc-ibm-os400)
      LD_RXLIB1="${CC} -shared  ${LDFLAGS}"
      LD_RXTRANSLIB1="$LD_RXLIB1"
      ;;
   *dec-osf*)
      if test "$ac_cv_prog_CC" = "gcc"; then
         SYS_DEFS="-D_POSIX_SOURCE -D_XOPEN_SOURCE"
      else
         SYS_DEFS="-D_POSIX_SOURCE -D_XOPEN_SOURCE -Olimit 800"
      fi
      LD_RXLIB1="ld -shared  ${LDFLAGS}"
      LD_RXTRANSLIB1="$LD_RXLIB1"
      ;;
   *sequent-dynix*)
      LD_RXLIB1="ld -G  ${LDFLAGS}"
      LD_RXTRANSLIB1="$LD_RXLIB1"
      ;;
   *solaris*)
      if test "$ac_cv_prog_CC" = "gcc"; then
         LD_RXLIB1="gcc -shared  ${LDFLAGS}"
         LD_RXTRANSLIB1="$LD_RXLIB1"
      else
         LD_RXLIB1="ld -G  ${LDFLAGS}"
         LD_RXTRANSLIB1="$LD_RXLIB1"
      fi
      ;;
   *esix*)
      LD_RXLIB1="ld -G  ${LDFLAGS}"
      LD_RXTRANSLIB1="$LD_RXLIB1"
      ;;
   *dgux*)
      LD_RXLIB1="ld -G  ${LDFLAGS}"
      LD_RXTRANSLIB1="$LD_RXLIB1"
      ;;
   sparc*sunos*)
      SYS_DEFS="-DSUNOS -DSUNOS_STRTOD_BUG"
      LD_RXLIB1="ld  ${LDFLAGS}"
      LD_RXTRANSLIB1="$LD_RXLIB1"
      ;;
   *linux*|*kfreebsd*-gnu*)
      LD_RXLIB1="${CC} -shared  ${LDFLAGS} -Wl,-soname,\$(@)"
      LD_RXTRANSLIB1="$LD_RXLIB1"
      CAN_USE_ABI="yes"
      if test "$USE_ABI" = "yes"; then
         OTHER_INSTALLS="installabilib"
      fi
      ;;
   *atheos*)
      LD_RXLIB1="${CC} -shared  ${LDFLAGS}"
      LD_RXTRANSLIB1="$LD_RXLIB1"
      ;;
   *-freebsd* | *openbsd*)
      if test "$ac_cv_prog_CC" = "gcc"; then
         LD_RXLIB1="gcc -shared  ${LDFLAGS}"
         LD_RXTRANSLIB1="$LD_RXLIB1"
      else
         LD_RXLIB1="ld -Bdynamic -Bshareable  ${LDFLAGS}"
         LD_RXTRANSLIB1="$LD_RXLIB1"
      fi
      STATIC_LDFLAGS="-static"
      ;;
   *pc-sco*)
      LD_RXLIB1="ld -dy -G  ${LDFLAGS}"
      LD_RXTRANSLIB1="$LD_RXLIB1"
      ;;
   *beos*)
      LD_RXLIB1="${CC}  ${LDFLAGS} -Wl,-shared -nostart -Xlinker -soname=\$(@)"
      LD_RXTRANSLIB1="$LD_RXLIB1"
      BEOS_DYN="yes"
      BASE_INSTALL="beosinstall"
      BASE_BINARY="beosbinary"
      OTHER_INSTALLS=""
      ;;
   *haiku*)
      LD_RXLIB1="${CC} -shared  ${LDFLAGS} -Wl,-soname,\$(SONAME)"
      LD_RXTRANSLIB1="$LD_RXLIB1"
      CAN_USE_ABI="yes"
      if test "$USE_ABI" = "yes"; then
         OTHER_INSTALLS="installabilib"
      fi
      BASE_INSTALL="haikuinstall"
      BASE_BINARY="haikubinary"
      ;;
   *nto-qnx*)
      LD_RXLIB1="${CC} -shared  ${LDFLAGS}"
      LD_RXTRANSLIB1="$LD_RXLIB1"
      ;;
   *qnx*)
      LIBPRE=""
      LIBPST=".lib"
      SHLPRE=""
      DYN_COMP="-Q -DSTATIC_DYNAMIC_LIBRARY"   # force no check for dynamic loading
      SHLFILE=""
      EEXTRA="-mf -N0x20000 -Q"
      ;;
   *cygwin)
      LIBPRE="lib"
      SHLPRE=""
      DYN_COMP="-DDYNAMIC"
      LIBPST=".a"
      EXE=".exe"
      RXDEFEXP1="./cygwin.def"
# Only for Rexx/Trans do we produce an import library for the DLL we are
# building
      LD_RXLIB1="dllwrap --def \$(RXDEFEXP1) --target i386-cygwin32 --dllname \$(@)"
      LD_RXTRANSLIB1="dllwrap --def \$(srcdir)/\$(basename \$(@))w32.def --output-lib ${LIBPRE}\$(basename \$(@))${LIBPST} --target i386-cygwin32 --dllname \$(@)"
      BASE_INSTALL="cygwininstall"
      BASE_BINARY="cygwinbinary"
# cygwininstall target MUST install the shared library itself because
# it puts it into $(bindir) not $(libdir) as all other platforms
      OTHER_INSTALLS=""
      ;;
   *darwin*)
      DYN_COMP="-fno-common -DDYNAMIC"
# ensure we use two-name namespace on Mac so that when a function in a common source file is called the one
# in the shared library the call came from is executed, not one with the same name from another shared library
# otherwise you will get memory corruption.
      LD_RXLIB1="${CC}  ${LDFLAGS} ${EEXTRA} -dynamiclib -headerpad_max_install_names"
      LD_RXTRANSLIB1="$LD_RXLIB1"
      ;;
   *)
      ;;
esac

dnl
dnl determine what switches our compiler uses for building objects
dnl suitable for inclusion in shared libraries
dnl Only call this if DYN_COMP is not set. If we have set DYN_COMP
dnl above, then we know how to compile AND link for dynamic libraries
dnl
if test "$DYN_COMP" = ""; then
AC_MSG_CHECKING(compiler flags for a dynamic object)

cat > conftest.$ac_ext <<EOF
dnl [#]line __oline__ "[$]0"
[#]line __oline__ "configure"
int a=0
EOF

   DYN_COMP=""
   mh_cv_stop=no
   save_cflags="$CFLAGS"
   mh_cv_flags="-fPIC -KPIC +Z"
   for a in $mh_cv_flags; do
      CFLAGS="-c $a"

      mh_compile='${CC-cc} -c $CFLAGS conftest.$ac_ext > conftest.tmp 2>&1'
      if AC_TRY_EVAL(mh_compile); then
         DYN_COMP=""
      else
         slash="\\"
         mh_dyncomp="`egrep -c $slash$a conftest.tmp`"
         if test "$mh_dyncomp" = "0"; then
            DYN_COMP="$a -DDYNAMIC"
            AC_MSG_RESULT($a)
            break
         else
            DYN_COMP=""
         fi
      fi
   done
   if test "$DYN_COMP" = ""; then
      AC_MSG_RESULT(none of $mh_cv_flags supported)
   fi
   if test "$LD_RXLIB1" = ""; then
      MH_HOWTO_DYN_LINK()
   fi
   CFLAGS=$save_cflags
   rm -f conftest.*
fi


aix_exports="config.exports.aix"
echo "" > $aix_exports
if test "$ac_cv_header_dl_h" = "yes" -o "$ac_cv_header_dlfcn_h" = "yes" -o "$AIX_DYN" = "yes" -o "$BEOS_DYN" = "yes" -o "$DLFCNINCDIR" != "" -o "$DLFCNLIBDIR" != ""; then
   if test "$with_rexx" = "rexxtrans" -o "$with_rexx" = "regina" -o  "$with_rexx" = "objrexx" -o "$with_rexx" = "oorexx" -o "$with_rexx" = "rexx6000"; then
      EXPS="1,2,3,4,5,6,7,8,9"
      SHL_TARGETS=""
      for a in $SHLFILES
      do
         if test "$USE_ABI" = "yes" -a "$CAN_USE_ABI" = "yes"; then
            SHL_TARGETS="${SHL_TARGETS} ${SHLPRE}${a}${SHLPST}.\$(ABI)"
         else
            SHL_TARGETS="${SHL_TARGETS} ${SHLPRE}${a}${SHLPST}"
         fi
         this=`echo $EXPS | cut -d, -f1`
         EXPS=`echo $EXPS | cut -d, -f2-`
         if test "$AIX_DYN" = "yes"; then
            echo "RXPACKEXP$this=$a.exp" >> $aix_exports
            echo "RXPACKEXPORTS$this=-bE:$a.exp" >> $aix_exports
            if test "$ac_cv_prog_CC" = "gcc"; then
               echo "RXPACKEXPORTS$this=-Wl,-bE:$a.exp" >> $aix_exports
            else
               echo "RXPACKEXPORTS$this=-bE:$a.exp" >> $aix_exports
            fi
         else
            echo "RXPACKEXP$this=" >> $aix_exports
            echo "RXPACKEXPORTS$this=" >> $aix_exports
         fi
      done
   else
      SHL_TARGETS=""
   fi
else
   SHL_TARGETS=""
fi

if test "$AIX_DYN" = "yes"; then
   STATICLIB=""
   LIBPST=".junk"
else
   STATICLIB="${LIBPRE}${SHLFILE}${LIBPST}"
fi

LD_RXLIB1="$LD_RXLIB1 ${LDFLAGS}"

AC_SUBST_FILE(aix_exports)
AC_SUBST(EEXTRA)
AC_SUBST(CEXTRA)
AC_SUBST(OSAVE)
AC_SUBST(OBJ)
AC_SUBST(EXE)
AC_SUBST(DYN_COMP)
AC_SUBST(LIBS)
AC_SUBST(SHLIBS)
AC_SUBST(LD_RXLIB1)
AC_SUBST(LD_RXTRANSLIB1)
AC_SUBST(SHLPRE)
AC_SUBST(SHLPST)
AC_SUBST(LIBPST)
AC_SUBST(LIBPRE)
AC_SUBST(LIBEXE)
AC_SUBST(LIBFLAGS)
AC_SUBST(DYNAMIC_LDFLAGS)
AC_SUBST(STATIC_LDFLAGS)
AC_SUBST(STATICLIB)
AC_SUBST(SHL_TARGETS)
AC_SUBST(O2SAVE)
AC_SUBST(O2SHO)
AC_SUBST(CC2O)
AC_SUBST(BASE_INSTALL)
AC_SUBST(OTHER_INSTALLS)
AC_SUBST(BASE_BINARY)
AC_SUBST(SAVE2O)
AC_SUBST(RXPACKEXPORTS)
AC_SUBST(RXPACKEXP)
AC_SUBST(RXDEFEXP1)
AC_SUBST(CAN_USE_ABI)
])dnl

dnl ---------------------------------------------------------------------------
dnl Check if C compiler supports -c -o file.ooo
dnl ---------------------------------------------------------------------------
AC_DEFUN([MH_CHECK_CC_O],
[
AC_MSG_CHECKING(whether $CC understand -c and -o together)
set dummy $CC; ac_cc="`echo [$]2 |
changequote(, )dnl
sed -e 's/[^a-zA-Z0-9_]/_/g' -e 's/^[0-9]/_/'`"
changequote([, ])dnl
AC_CACHE_VAL(ac_cv_prog_cc_${ac_cc}_c_o,
[echo 'int foo(){}' > conftest.c
# We do the test twice because some compilers refuse to overwrite an
# existing .o file with -o, though they will create one.
eval ac_cv_prog_cc_${ac_cc}_c_o=no
ac_try='${CC-cc} -c conftest.c -o conftest.ooo 1>&AC_FD_CC'
if AC_TRY_EVAL(ac_try) && test -f conftest.ooo && AC_TRY_EVAL(ac_try);
then
  ac_try='${CC-cc} -c conftest.c -o conftest.ooo 1>&AC_FD_CC'
  if AC_TRY_EVAL(ac_try) && test -f conftest.ooo && AC_TRY_EVAL(ac_try);
  then
    eval ac_cv_prog_cc_${ac_cc}_c_o=yes
  fi
fi
rm -f conftest*
])dnl
if eval "test \"`echo '$ac_cv_prog_cc_'${ac_cc}_c_o`\" = yes"; then
   O2SHO=""
   O2SAVE=""
   SAVE2O=""
   CC2O="-o $"'@'
   AC_MSG_RESULT(yes)
else
   O2SHO="-mv \`basename "$'@'" .sho\`.o "$'@'
   O2SAVE="-mv \`basename "$'@'" .sho\`.o \`basename "$'@'" .sho\`.o.save"
   SAVE2O="-mv \`basename "$'@'" .sho\`.o.save \`basename "$'@'" .sho\`.o"
   CC2O=""
   AC_MSG_RESULT(no)
fi
])

dnl ---------------------------------------------------------------------------
dnl Work out if functions in dynamically loadable libraries need leading _
dnl Tests based on glib code and only valid for dlopen() mechanism
dnl ---------------------------------------------------------------------------
AC_DEFUN([MH_CHECK_LEADING_USCORE],
[
if test "$ac_cv_header_dlfcn_h" = "yes" -o "$HAVE_DLFCN_H" = "1"; then
   AC_MSG_CHECKING(if symbols need underscore prepended in loadable modules)
   tmpLIBS="$LIBS"
   save_cflags="$CFLAGS"
   LIBS="$LIBS $DLFCNLIBDIR"
   CFLAGS="$CFLAGS $DLFCNINCDIR"
   AC_CACHE_VAL(mh_cv_uscore,[
   AC_RUN_IFELSE(
     [AC_LANG_SOURCE([
       #include <dlfcn.h>
       int mh_underscore_test (void) { return 42; }
       int main() {
         void *f1 = (void*)0, *f2 = (void*)0, *handle;
         handle = dlopen ((void*)0, 0);
         if (handle) {
           f1 = dlsym (handle, "mh_underscore_test");
           f2 = dlsym (handle, "_mh_underscore_test");
         } return (!f2 || f1);
       }],
     [mh_cv_uscore=yes],
     [mh_cv_uscore=no],
     [mh_cv_uscore=no]
   )
   ],[mh_cv_uscore=yes],[mh_cv_uscore=no],mh_cv_uscore=no)
   ])
   AC_MSG_RESULT($mh_cv_uscore)
   if test "x$mh_cv_uscore" = "xyes"; then
     AC_DEFINE(MODULES_NEED_USCORE)
   fi
   LIBS="$tmpLIBS"
   CFLAGS="$save_cflags"
fi
])

dnl ---------------------------------------------------------------------------
dnl Determines the file extension for shared libraries
dnl ---------------------------------------------------------------------------
AC_DEFUN([MH_SHLPST],
[
AC_MSG_CHECKING(shared library/external function extensions)
SHLPST=".so"
MODPST=".so"
AC_REQUIRE([AC_CANONICAL_SYSTEM])
case "$target" in
   *hp-hpux*)
      SHLPST=".sl"
      MODPST=".sl"
      ;;
   *ibm-aix5*)
      SHLPST=".a"
      MODPST=".a"
      ;;
   *ibm-aix*)
      SHLPST=".a"
      MODPST=".a"
      ;;
   *nto-qnx*)
      ;;
   *qnx*)
      SHLPST=".junk"
      MODPST=""
      ;;
   *cygwin*)
      SHLPST=".dll"
      MODPST=".dll"
      ;;
   *skyos*)
      SHLPST=".dll"
      MODPST=".dll"
      ;;
   *darwin*)
      SHLPST=".dylib"
      MODPST=".dylib"
      ;;
esac
AC_SUBST(SHLPST)
AC_SUBST(MODPST)
AC_MSG_RESULT($SHLPST/$MODPST)
])

dnl ---------------------------------------------------------------------------
dnl Work out if POSIX Threads are supported
dnl ---------------------------------------------------------------------------
AC_DEFUN([MH_CHECK_POSIX_THREADS],
[
MH_MT_LIBS=""
THREADING_COMPILE=""
THREADING_LINK=""
THREADSAFE_TARGET=""
MT_FILE="rxmt_notmt"
if test "$enable_posix_threads" = yes; then
   dnl
   dnl -lrt is needed by posix threads on Solaris
   dnl
   AC_CHECK_LIB("rt",main,mh_rt_lib_found=yes,mh_rt_lib_found=no)
   AC_MSG_CHECKING(whether Posix Threads are supported)
   tmpLIBS=$LIBS
   save_cflags="$CFLAGS"
   pthread_libs="pthreads pthread c_r c"
   case "$target" in
      *freebsd*)
         CFLAGS="$CFLAGS -pthread"
         ;;
   esac
   for mh_lib in $pthread_libs; do
      LIBS="$tmpLIBS -l${mh_lib}"
      AC_TRY_LINK(
      [#include <pthread.h>],
      [pthread_create(NULL,NULL,NULL,NULL)],
      [mh_has_pthreads=yes],
      [mh_has_pthreads=no]
      )
      if test "$mh_has_pthreads" = yes; then
         MH_MT_LIBS="-l${mh_lib}"
         AC_REQUIRE([AC_CANONICAL_SYSTEM])
         THREADING_COMPILE="-D_REENTRANT -DPOSIX"
         THREADING_LINK=""
         MT_FILE="rxmt_posix"
         case "$target" in
            *solaris*)
               if test "$ac_cv_prog_CC" = "gcc" -o "$ac_cv_prog_CC" = "g++" -o "$ac_cv_prog_CC" = "clang"; then
                  THREADING_COMPILE="-D_REENTRANT -DPOSIX"
                  if test "$mh_rt_lib_found" = "yes"; then
                     THREADING_LINK="-lrt"
                  fi
               else
                  THREADING_COMPILE="-mt -D_REENTRANT -DREENTRANT -D_PTHREAD_SEMANTICS"
                  THREADING_LINK="-mt"
               fi
               ;;
            *hp-hpux1*)
               if test "$ac_cv_prog_CC" = "gcc" -o "$ac_cv_prog_CC" = "g++" -o "$ac_cv_prog_CC" = "clang"; then
                  THREADING_COMPILE="-D_REENTRANT -DPOSIX"
               fi
               if test "$mh_rt_lib_found" = "yes"; then
                  THREADING_LINK="-lrt"
               fi
               ;;
            *linux*)
               if test "$ac_cv_prog_CC" = "gcc" -o "$ac_cv_prog_CC" = "g++" -o "$ac_cv_prog_CC" = "clang"; then
                  THREADING_LINK="-lpthread"
               else
                  THREADING_LINK="-lpthread"
               fi
               ;;
            *freebsd*)
               THREADING_COMPILE="-pthread -D_REENTRANT -DPOSIX"
               THREADING_LINK="-pthread"
               ;;
            *cygwin*)
               THREADING_COMPILE="-D_REENTRANT -DPOSIX"
               THREADING_LINK=""
               ;;
            *)
               if test "$ac_cv_prog_CC" = "gcc" -o "$ac_cv_prog_CC" = "g++" -o "$ac_cv_prog_CC" = "clang"; then
                  THREADING_COMPILE="-D_REENTRANT -DPOSIX"
               fi
               ;;
         esac
         break
      fi
   done
   LIBS="$tmpLIBS"
   CFLAGS="$save_cflags"
   AC_MSG_RESULT($mh_has_pthreads)
fi
AC_SUBST(THREADING_COMPILE)
AC_SUBST(THREADING_LINK)
AC_SUBST(MH_MT_LIBS)
AC_SUBST(MT_FILE)
])

dnl ---------------------------------------------------------------------------
dnl Determines where rpm build files are located - use MH_CHECK_PACKAGE_BUILDERS instead
dnl ---------------------------------------------------------------------------
AC_DEFUN([MH_CHECK_RPM],
[
AC_CHECK_PROG(mh_rpm_exists, rpm, yes, no )
if test "$mh_rpm_exists" = yes; then
   AC_MSG_CHECKING(where rpms are built)
   RPMTOPDIR=`rpm --eval "%{_topdir}"`
   AC_MSG_RESULT($RPMTOPDIR)
   AC_SUBST(RPMTOPDIR)
fi
])

dnl ---------------------------------------------------------------------------
dnl Sets package directories within the source tree
dnl ---------------------------------------------------------------------------
AC_DEFUN([MH_CHECK_PACKAGE_BUILDERS],
[
#
# get our own abs_srcdir as autoconf sucks
#
here=`pwd`
cd $srcdir
absolute_srcdir=`pwd`
cd $here
#
packagesdir="$absolute_srcdir/packages"
AC_SUBST(packagesdir)
AC_MSG_CHECKING(where rpms are built)
AC_CHECK_PROG(mh_rpm_exists, rpmbuild, yes, no )
if test "$mh_rpm_exists" = yes; then
#   rpmtopdir=`rpm --eval "%{_topdir}"`
   rpmtopdir="${packagesdir}/RPM"
   AC_MSG_RESULT($rpmtopdir)
else
   rpmtopdir=""
   AC_MSG_RESULT("not avaliable")
fi
AC_SUBST(rpmtopdir)
#
AC_MSG_CHECKING(where deb packages are built)
AC_CHECK_PROG(mh_dpkg_exists, dpkg-buildpackage, yes, no )
if test "$mh_dpkg_exists" = yes; then
   debtopdir="${packagesdir}/DEB"
   AC_MSG_RESULT($debtopdir)
else
   debtopdir=""
   AC_MSG_RESULT("not avaliable")
fi
AC_SUBST(debtopdir)
#
AC_MSG_CHECKING(where apk packages are built)
AC_CHECK_PROG(mh_apk_exists, abuild, yes, no )
if test "$mh_apk_exists" = yes; then
   apktopdir="${packagesdir}/APK"
   AC_MSG_RESULT($apktopdir)
else
   apktopdir=""
   AC_MSG_RESULT("not avaliable")
fi
AC_SUBST(apktopdir)
# if we haven't set an environment variable; PACKAGE_RELEASE to a value,
# get a timestamp for use in eg RPM Spec file Release
AC_MSG_CHECKING(getting PACKAGE_RELEASE value)
PACKAGE_RELEASE=`echo $PACKAGE_RELEASE`
if test x"$PACKAGE_RELEASE" == "x"; then
   PACKAGE_RELEASE=`date +%y%m%d.%H%M`
fi
AC_MSG_RESULT($PACKAGE_RELEASE)
AC_SUBST(PACKAGE_RELEASE)
])

dnl ---------------------------------------------------------------------------
dnl Determines whether compiler supports long long
dnl ---------------------------------------------------------------------------
AC_DEFUN([MH_C_LONG_LONG],
[AC_CACHE_CHECK(for long long int, mh_cv_c_long_long,
 [if test "$ac_cv_prog_CC" = "gcc" -o "$ac_cv_prog_CC" = "g++" -o "$ac_cv_prog_CC" = "clang"; then
    mh_cv_c_long_long=yes
  else
    AC_TRY_COMPILE(,[long long int i;],
      mh_cv_c_long_long=yes,
      mh_cv_c_long_long=no)
  fi])
 if test $mh_cv_c_long_long = yes; then
   AC_DEFINE(HAVE_LONG_LONG, 1, [compiler understands long long])
 fi
])

dnl ---------------------------------------------------------------------------
dnl Shows the status of the configure options
dnl ---------------------------------------------------------------------------
AC_DEFUN([MH_SHOW_STATUS],
[
echo
echo "$FULL_PACKAGE_NAME has now been configured with the following options:"
echo
echo "                 Rexx Interpreter: $REXX_INT"

if test "$with_rexxincdir" != no ; then
   echo "                  rexx headers in: $with_rexxincdir"
fi

if test "$with_rexxlibdir" != no ; then
   echo "                rexx libraries in: $with_rexxlibdir"
fi

if test "$with_dlfcnincdir" != no ; then
   echo "                 dlfcn headers in: $with_dlfcnincdir"
fi
if test "$with_dlfcnlibdir" != no ; then
   echo "               dlfcn libraries in: $with_dlfcnlibdir"
fi

dnl
dnl Run the package_specific status shower
dnl
$1

myopts="$myopts ${bitflag}BIT"

if test "$with_debug" = yes ; then
   myopts="$myopts DEBUG"
fi

if test "$with_purify" = yes ; then
   myopts="$myopts PURIFY"
fi

if test "$with_efence" = yes; then
   myopts="$myopts EFENCE"
fi

if test "$with_hpux_portable" = yes ; then
   myopts="$myopts HPUX-PORTABLE"
fi

if test "x$found_arch_flags" != "x" ; then
   myopts="$myopts UNIVERSAL: $found_arch_flags"
fi

if test "$on_beos" = yes; then
   bindir="/boot/home/config/bin"
   libdir="/boot/home/config/lib"
fi

if test "$on_haiku" = yes; then
   bindir="/boot/home/config/non-packaged/bin"
   libdir="/boot/home/config/non-packaged/lib"
   addonsdir="/boot/home/config/non-packaged/lib/regina-rexx/addons"
fi

echo "                    Build options: $myopts"
echo "    binaries will be installed in: $bindir"
if test "x$binarybitprefix" = "x32"; then
   echo "   32bit binaries will have '32' appended to them."
fi
echo "   libraries will be installed in: $REXX_ADDONS"
echo "                         {prefix}: $prefix"
echo "                    {exec_prefix}: $exec_prefix"
if test "x$rpmtopdir" != "x" ; then
   echo "                       RPM topdir: $rpmtopdir"
fi
if test "x$debtopdir" != "x" ; then
   echo "                       DEB topdir: $debtopdir"
fi
if test "x$apktopdir" != "x" ; then
   echo "                       APK topdir: $apktopdir"
fi
echo
if test "x$2" != "xnotrailer"; then
   echo "To build the $MH_PACKAGE binaries, and dynamically loadable libraries, type 'make'"
   echo "To install $MH_PACKAGE in the above directories, type 'make install'"
   echo
fi
])

dnl ---------------------------------------------------------------------------
dnl Determines which -arch flags valid on Mac OSX with gcc
dnl ---------------------------------------------------------------------------
AC_DEFUN([MH_CHECK_OSX_ARCH],
[
valid_arch_flags=""
found_arch_flags=""
AC_ARG_WITH(arch,
   [  --with-arch=option      build universal binaries on MacOS, option: one of "all 32bit 64bit intel apple-silicon ppc ppc64 x86_64 i386 arm64"],
   [with_arch=$withval],
   [with_arch=none],
)
case "$with_arch" in
   none)
      arch_flags=""
      ;;
   all)
      arch_flags="-arch ppc -arch ppc64 -arch x86_64 -arch i386 -arch arm64"
      ;;
   32bit)
      arch_flags="-arch ppc -arch i386"
      ;;
   64bit)
      arch_flags="-arch ppc64 -arch x86_64 -arch arm64"
      ;;
   apple-silicon)
      arch_flags="-arch x86_64 -arch arm64"
      ;;
   intel)
      arch_flags="-arch i386 -arch x86_64"
      ;;
   apple-silicon)
      arch_flags="-arch x86_64 -arch arm64"
      ;;
   *)
      arch_flags="-arch $with_arch"
      ;;
esac

valid_arch_flags=""
found_arch_flags=""
if test $on_osx = "yes"; then
  AC_MSG_CHECKING(for which MacOS -arch flags are supported for universal binaries)
  for a in $arch_flags; do
    if test $a != "-arch"; then
        save_ldflags="$LDFLAGS"
        LDFLAGS="$LDFLAGS -arch $a"
        AC_LINK_IFELSE(
        [AC_LANG_PROGRAM([#include <stdio.h>
#include <stdlib.h>],
        [exit(0)])],
        [valid_arch_flags="$valid_arch_flags -arch $a";found_arch_flags="$found_arch_flags $a"],
        [a="$a"]
    )
        LDFLAGS="$save_ldflags"
    fi
  done
  AC_MSG_RESULT($found_arch_flags)
  AC_SUBST(valid_arch_flags)
fi
])

dnl ---------------------------------------------------------------------------
dnl Determines the kernel name from uname
dnl ---------------------------------------------------------------------------
AC_DEFUN([MH_GET_KERNEL_NAME],
[
AC_CHECK_PROG(uname, [uname], yes, no)
AC_MSG_CHECKING(kernel name)
if test "$ac_cv_prog_uname" = yes; then
   kn=`uname -s`
else
   kn="unknown"
fi
AC_DEFINE_UNQUOTED(MH_KERNEL_NAME, "$kn", [kernel name])
AC_MSG_RESULT($kn)
])

dnl ---------------------------------------------------------------------------
dnl Determines the Linux distribution name
dnl ---------------------------------------------------------------------------
AC_DEFUN([MH_GET_DISTRO_NAME],
[
AC_CHECK_PROG(lsb_release, [lsb_release], yes, no)
if test "$ac_cv_prog_lsb_release" = yes; then
   AC_MSG_CHECKING(OS distribution name)
   MYDISTRO="`lsb_release -i | cut -f 2`-`lsb_release -r | cut -f 2`"
   MYDISTRO="`echo $MYDISTRO | sed \"s/ /_/g\"`"
   AC_MSG_RESULT($MYDISTRO)
else
   case "$target" in
      *linux*)
         if test -f "/etc/os-release"; then
            myid="`grep ^ID= /etc/os-release | cut -f2 -d\=`"
            myver="`grep ^VERSION_ID= /etc/os-release | cut -f2 -d\= | sed 's/\"//g'`"
            MYDISTRO="$myid-$myver"
         else
            MYDISTRO="`echo $target | cut -f3 -d-`"
         fi
      ;;
      *freebsd* | *openbsd*)
         MYDISTRO="`echo $target | cut -f3 -d-`"
      ;;
      *darwin*)
         MYDISTRO="`echo $target | cut -f2-3 -d-`"
      ;;
      *pc-solaris2*)
         MYDISTRO="`echo $target | cut -f2- -d-`"
      ;;
      *cygwin*)
         MYDISTRO="`echo $target | cut -f2- -d-`"
      ;;
      *nto-qnx*)
         MYDISTRO="`uname -s`-`uname -r`"
      ;;
      *qnx*)
         MYDISTRO="`uname -s`-`uname -v`"
      ;;
      *)
         MYDISTRO="$target"
      ;;
   esac
fi
AC_SUBST(MYDISTRO)
])

AC_DEFUN([MH_CHECK_NCURSES],
[
curs=$1
NCURSES_PKG="${curs}"
# look for pkg-config first
AC_CHECK_PROG(pkg_config, [pkg-config], "yes", no)
if test "$pkg_config" = yes; then
   # we have pkg_config, do we have ncurses[w]?
   pkg-config --exists ${NCURSES_PKG} > /dev/null
   if test $? -eq 0; then
      MH_CURSES_INC="`pkg-config --cflags ${NCURSES_PKG}`"
      MH_CURSES_LIB="`pkg-config --libs ${NCURSES_PKG}`"
      CURSES_BUILD=$with_curses
      CURSES_CONFIG="pkg-config ${NCURSES_PKG}"
      curses_h="curses.h"
      curses_l="$NCURSES_PKG"
      AC_DEFINE(USE_NCURSES)
   else
      # no pkg-config, what about ncurses-config?
      if test "$curs" = "ncursesw" -o "$curs" = "ncursesw8"; then
         curses_configs="ncursesw-config ncursesw6-config ncursesw5-config"
      else
         curses_configs="ncurses-config ncurses6-config ncurses5-config"
      fi
      for config_name in $curses_configs ;
         do
            AC_CHECK_PROG([config_var], [$config_name], [yes], no)
            if test "x$config_var" = "xyes"; then
               MH_CURSES_INC="`${config_name} --cflags`"
               MH_CURSES_LIB="`${config_name} --libs`"
               CURSES_BUILD=$with_curses
               CURSES_CONFIG="${config_name}"
               curses_h="curses.h"
               curses_l="$NCURSES_PKG"
               AC_DEFINE(USE_NCURSES)
               break
            fi
            # ensure variable is not cached
            unset ac_cv_prog_config_var
            unset config_var
         done
      if test "x$curses_h" = "x"; then
         AC_MSG_ERROR(unable to find configuration for ${NCURSES_PKG} package; cannot configure)
      fi
   fi
fi
])

AC_DEFUN([MH_CHECK_PDCURSES_X11],
[
curs=$1
builtin=$2
flags=$3
if test $builtin_curses = yes; then
   MH_CHECK_X_INC
   MH_CHECK_X_LIB
   MH_CURSES_INC="$flags -DXCURSES $MH_XINC_DIR"
   MH_CURSES_LIB="pdc-obj/libpdcurses.a $MH_XLIBS"
   CURSES_BUILD=$with_curses
   CURSES_CONFIG="builtin PDCursesMod"
   curses_h="curses.h"
   curses_l="pdcurses"
else
   PDCURSES_PKG="lib${curs}"
   # look for pkg-config first
   AC_CHECK_PROG(pkg_config, [pkg-config], yes, no)
   if test "$ac_cv_prog_pkg_config" = yes; then
      # we have pkg_config, do we have ncursesw?
      pkg-config --exists ${PDCURSES_PKG} > /dev/null
      if test $? -eq 0; then
         MH_CURSES_INC="`pkg-config --cflags ${PDCURSES_PKG}`"
         MH_CURSES_LIB="`pkg-config --libs ${PDCURSES_PKG}`"
         curses_h="curses.h" #xcurses.h
         curses_l="XCurses"
      else
         AC_MSG_ERROR(pkg-config cannot find ${PDCURSES_PKG} package; cannot configure)
      fi
   fi
fi
])

AC_DEFUN([MH_CHECK_PDCURSES_SDL],
[
curs=$1
builtin=$2
pkg=$3
flags=$4
if test $builtin_curses = yes; then
   # look for pkg-config first
   AC_CHECK_PROG(pkg_config, [pkg-config], yes, no)
   if test "$ac_cv_prog_pkg_config" = yes; then
      # we have pkg_config, do we have sdl or SDL_ttf?
      pkg-config --exists $pkg > /dev/null
      if test $? -eq 0; then
         MH_CURSES_INC="$flags `pkg-config --cflags $pkg`"
         MH_CURSES_LIB="pdc-obj/libpdcurses.a `pkg-config --libs $pkg`"
         CURSES_BUILD=$with_curses
         CURSES_CONFIG="builtin PDCursesMod"
         curses_h="curses.h"
         curses_l="pdcurses"
      else
         AC_MSG_ERROR(--with-curses=$with_curses requires $pkg package installed; THE cannot be configured)
      fi
   else
      AC_MSG_ERROR(--with-curses=$with_curses requires pkg-config and $pkg package installed; THE cannot be configured)
   fi
else
   PDCURSES_PKG="lib${curs}"
   # look for pkg-config first
   AC_CHECK_PROG(pkg_config, [pkg-config], yes, no)
   if test "$ac_cv_prog_pkg_config" = yes; then
      # we have pkg_config, do we have pdcurses package?
      pkg-config --exists ${PDCURSES_PKG} > /dev/null
      if test $? -eq 0; then
         MH_CURSES_INC="`pkg-config --cflags ${PDCURSES_PKG}`"
         MH_CURSES_LIB="`pkg-config --libs ${PDCURSES_PKG}`"
         CURSES_BUILD=$with_curses
         CURSES_CONFIG="pkg-config ${PDCURSES_PKG}"
         curses_h="curses.h"
         curses_l="pdcurses"
      fi
   fi
fi
])

AC_DEFUN([MH_CHECK_PDCURSES_SDL1],
[
curs=$1
builtin=$2
pkg=$3
flags=$4
if test $builtin_curses = yes; then
   # look for pkg-config first
   AC_CHECK_PROG(pkg_config, [pkg-config], yes, no)
   if test "$ac_cv_prog_pkg_config" = yes; then
      # we have pkg_config, do we have sdl or SDL_ttf?
      pkg-config --exists $pkg > /dev/null
      if test $? -eq 0; then
         MH_CURSES_INC="-DPDC_WIDE `pkg-config --cflags $pkg`"
         MH_CURSES_LIB="pdc-obj/libpdcurses.a `pkg-config --libs $pkg`"
         CURSES_BUILD=$with_curses
         CURSES_CONFIG="builtin PDCursesMod"
         curses_h="curses.h"
         curses_l="pdcurses"
      else
         AC_MSG_ERROR(--with-curses=$with_curses requires $pkg package installed; THE cannot be configured)
      fi
   else
      AC_MSG_ERROR(--with-curses=$with_curses requires pkg-config and $pkg package installed; THE cannot be configured)
   fi
else
   PDCURSES_PKG="lib${curs}"
   # look for pkg-config first
   AC_CHECK_PROG(pkg_config, [pkg-config], yes, no)
   if test "$ac_cv_prog_pkg_config" = yes; then
      # we have pkg_config, do we have pdcurses package?
      pkg-config --exists ${PDCURSES_PKG} > /dev/null
      if test $? -eq 0; then
         MH_CURSES_INC="`pkg-config --cflags ${PDCURSES_PKG}`"
         MH_CURSES_LIB="`pkg-config --libs ${PDCURSES_PKG}`"
         CURSES_BUILD=$with_curses
         CURSES_CONFIG="pkg-config ${PDCURSES_PKG}"
         curses_h="curses.h"
         curses_l="pdcurses"
      fi
   fi
fi
])

AC_DEFUN([MH_CHECK_PDCURSES_SDL2],
[
curs=$1
builtin=$2
if test $builtin_curses = yes; then
   # look for pkg-config first
   AC_CHECK_PROG(pkg_config, [pkg-config], yes, no)
   if test "$ac_cv_prog_pkg_config" = yes; then
      # we have pkg_config, do we have SDL2_ttf?
      pkg-config --exists SDL2_ttf > /dev/null
      if test $? -eq 0; then
         flag=''
         case "$curs" in
            pdcurses-sdl2w)
               flag="-DPDC_WIDE"
               ;;
            pdcurses-sdl2w8)
               flag="-DPDC_WIDE -DPDC_FORCE_UTF8"
               ;;
            *)
               ;;
         esac
         MH_CURSES_INC="$flag `pkg-config --cflags SDL2_ttf`"
         MH_CURSES_LIB="pdc-obj/libpdcurses.a `pkg-config --libs SDL2_ttf`"
         CURSES_BUILD=$with_curses
         CURSES_CONFIG="builtin PDCursesMod"
         curses_h="curses.h"
         curses_l="pdcurses"
      else
         AC_MSG_ERROR(--with-curses=$with_curses requires SDL2_ttf package installed; THE cannot be configured)
      fi
   else
      AC_MSG_ERROR(--with-curses=$with_curses requires pkg-config and SDL2_ttf package installed; THE cannot be configured)
   fi
else
   PDCURSES_PKG="lib${curs}"
   # look for pkg-config first
   AC_CHECK_PROG(pkg_config, [pkg-config], yes, no)
   if test "$ac_cv_prog_pkg_config" = yes; then
      # we have pkg_config, do we have ncursesw?
      pkg-config --exists ${PDCURSES_PKG} > /dev/null
      if test $? -eq 0; then
         MH_CURSES_INC="`pkg-config --cflags ${PDCURSES_PKG}`"
         MH_CURSES_LIB="`pkg-config --libs ${PDCURSES_PKG}`"
         CURSES_BUILD=$with_curses
         CURSES_CONFIG="pkg-config ${PDCURSES_PKG}"
         curses_h="curses.h"
         curses_l="pdcurses"
      fi
   fi
fi
])

AC_DEFUN([MH_CHECK_PDCURSES_VT],
[
curs=$1
builtin_curses=$2
flags=$3
if test $builtin_curses = yes; then
   MH_CURSES_INC="$flags -I$srcdir/src/PDCursesMod"
   MH_CURSES_LIB="pdc-obj/libpdcurses.a"
   CURSES_BUILD=$with_curses
   CURSES_CONFIG="builtin PDCursesMod"
   curses_h="curses.h"
   curses_l="pdcurses"
else
   PDCURSES_PKG="lib${curs}"
   # look for pkg-config first
   AC_CHECK_PROG(pkg_config, [pkg-config], yes, no)
   if test "$ac_cv_prog_pkg_config" = yes; then
      # we have pkg_config, do we have ncursesw?
      pkg-config --exists ${PDCURSES_PKG} > /dev/null
      if test $? -eq 0; then
         MH_CURSES_INC="`pkg-config --cflags ${PDCURSES_PKG}`"
         MH_CURSES_LIB="`pkg-config --libs ${PDCURSES_PKG}`"
         CURSES_BUILD=$with_curses
         CURSES_CONFIG="pkg-config ${PDCURSES_PKG}"
         curses_h="curses.h"
         curses_l="pdcurses"
      fi
   fi
fi
])

AC_DEFUN([MH_CHECK_X_CONFIG],
[
MH_CHECK_X_INC
MH_CHECK_X_LIB
])

dnl ---------------------------------------------------------------------------
dnl Check curses library and header files
dnl ---------------------------------------------------------------------------
AC_DEFUN([MH_CHECK_CURSES],
[
AC_MSG_CHECKING(curses platform)
CURSES_BUILD=""
CURSES_CONFIG=""
case "$target" in
   *hpux1*)
         mh_curses_colr=yes
         mh_pre_curses_h_include="stdarg"
      ;;
   *aix*4.2*)
         mh_curses_colr=no
         mh_pre_curses_h_include="stdarg"
      ;;
   *)
         mh_curses_colr=no
         mh_pre_curses_h_include=""
esac
case "$with_curses" in
   ncurses|ncursesw|ncursesw8)
      MH_CHECK_NCURSES($with_curses)
      ;;
   pdcurses-x11)
      MH_CHECK_PDCURSES_X11($with_curses,$1,'')
      ;;
   pdcurses-x11w)
      MH_CHECK_PDCURSES_X11($with_curses,$1,'-DPDC_WIDE')
      ;;
   pdcurses-x11w8)
      MH_CHECK_PDCURSES_X11($with_curses,$1,'-DPDC_WIDE -DPDC_FORCE_UTF8')
      ;;
   pdcurses-sdl1)
      MH_CHECK_PDCURSES_SDL($with_curses,$1,sdl,'')
      ;;
   pdcurses-sdl1w)
      MH_CHECK_PDCURSES_SDL($with_curses,$1,SDL_ttf,'-DPDC_WIDE')
      ;;
   pdcurses-sdl1w8)
      MH_CHECK_PDCURSES_SDL($with_curses,$1,SDL_ttf,'-DPDC_WIDE -DPDC_FORCE_UTF8')
      ;;
   pdcurses-sdl2)
      MH_CHECK_PDCURSES_SDL($with_curses,$1,sdl2,'')
      ;;
   pdcurses-sdl2w)
      MH_CHECK_PDCURSES_SDL($with_curses,$1,SDL2_ttf,'-DPDC_WIDE')
      ;;
   pdcurses-sdl2w8)
      MH_CHECK_PDCURSES_SDL($with_curses,$1,SDL2_ttf,'-DPDC_WIDE -DPDC_FORCE_UTF8')
      ;;
   pdcurses-vt)
      MH_CHECK_PDCURSES_VT($with_curses,$1,'')
      ;;
   pdcurses-vtw)
      MH_CHECK_PDCURSES_VT($with_curses,$1,'-DPDC_WIDE')
      ;;
   pdcurses-vtw8)
      MH_CHECK_PDCURSES_VT($with_curses,$1,'-DPDC_WIDE -DPDC_FORCE_UTF8')
      ;;
   *)
      # none specified
esac
AC_SUBST(MH_CURSES_INC)
AC_SUBST(MH_CURSES_LIB)
CURSES_BUILD=$with_curses
AC_SUBST(CURSES_BUILD)
APK_CURSES_BUILD="`echo ${with_curses} | sed -e s/-/_/g`"
AC_SUBST(APK_CURSES_BUILD)
AC_MSG_RESULT($CURSES_BUILD)
])

dnl ---------------------------------------------------------------------------
dnl Wrapper for AC_CHECK_FUNCS.
dnl ---------------------------------------------------------------------------
AC_DEFUN([MH_CHECK_FUNCS],
[
dnl check a standard list of functions, then those passed in as args
AC_CHECK_FUNCS(memcpy memmove strerror getopt strtof $*)
])dnl

dnl ---------------------------------------------------------------------------
dnl Check if various types have been typedefed by supplied header files
dnl ---------------------------------------------------------------------------
AC_DEFUN([MH_CHECK_TYPES],
[
header=$1
inc=$2
types="LONG CHAR SHORT ULONG UCHAR USHORT $3"
if test "x$inc" != "x"; then
   save_CFLAGS="$CFLAGS"
   CFLAGS="$CFLAGS $inc"
fi
CFLAGS="$CFLAGS $DB_INCLUDES"
for mh_type in $types; do
   AC_MSG_CHECKING(if $mh_type already defined)
   if test "x$header" = "x"; then
      AC_TRY_COMPILE([],
         [${mh_type} var1;],
         mh_type_found=yes, mh_type_found=no )
   else
      AC_TRY_COMPILE([#include <$header>],
         [${mh_type} var1;],
         mh_type_found=yes, mh_type_found=no )
   fi
   AC_MSG_RESULT($mh_type_found)
   if test "$mh_type_found" = yes; then
      AC_DEFINE_UNQUOTED(${mh_type}_TYPEDEFED)
   fi
done
if test "x$inc" != "x"; then
   CFLAGS=$save_CFLAGS
fi
])dnl

dnl ---------------------------------------------------------------------------
dnl Set full package name from package name and version
dnl ---------------------------------------------------------------------------
AC_DEFUN([MH_SET_FULL_PACKAGE_NAME],
[
FULL_PACKAGE_NAME=$1-$2
AC_SUBST(FULL_PACKAGE_NAME)
])

dnl ---------------------------------------------------------------------------
dnl Check if large file support available
dnl ---------------------------------------------------------------------------
AC_DEFUN([MH_LARGE_FILE_SUPPORT],
[
LARGE_FILE_SUPPORT=""
AC_CHECK_PROG(getconf, [getconf], yes, no)
if test "$ac_cv_prog_getconf" = yes; then
   AC_MSG_CHECKING(large file support flags)
   LARGE_FILE_SUPPORT=`getconf LFS_CFLAGS`
   AC_MSG_RESULT($LARGE_FILE_SUPPORT)
fi
if test "$LARGE_FILE_SUPPORT" = "undefined"; then
   LARGE_FILE_SUPPORT=""
fi
AC_SUBST(LARGE_FILE_SUPPORT)
])dnl
