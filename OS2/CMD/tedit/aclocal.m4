dnl ---------------------------------------------------------------------------
dnl Determine if curses library supports various functions
dnl ---------------------------------------------------------------------------
AC_DEFUN([MH_CURSES_FUNCS],
[
dnl
dnl Ensure that when compiling with XCurses, we have set the
dnl required extern for program name.
dnl
cat > xxxxx.h <<EOF
#ifdef XCURSES
	char *XCursesProgramName="test";
#endif
EOF
dnl
dnl Include before curses.h any include files required. This is
dnl often stdarg.h
dnl
for incfile in $mh_pre_curses_h_include ; do
	echo "#include <$incfile.h>" >> xxxxx.h
done

mh_save_libs="$LIBS"
mh_save_cflags="$CFLAGS"

if test "$with_xcurses" = yes; then
	CFLAGS="-DXCURSES $MH_CURSES_INC $SYS_DEFS"
	LIBS="$LIBS $MH_CURSES_LIB $MH_XLIBS $MH_EXTRA_LIBS"
else
	CFLAGS="$MH_CURSES_INC $SYS_DEFS"
	LIBS="$LIBS $MH_CURSES_LIB $MH_EXTRA_LIBS"
fi

AC_MSG_CHECKING(for System V curses)
AC_CACHE_VAL(
[mh_cv_sysv_curses],
[
AC_TRY_COMPILE(
[#include "xxxxx.h"]
[#include <$curses_h>],
[long xxx=(long)A_NORMAL],
[mh_cv_sysv_curses=yes],
[mh_cv_sysv_curses=no]
)
])dnl
AC_MSG_RESULT($mh_cv_sysv_curses)
if test "$mh_cv_sysv_curses" = no ; then
	AC_DEFINE(HAVE_BSD_CURSES)
fi

if test "$mh_cv_sysv_curses" = no ; then
	if test "$with_extcurses" = no ; then
		MH_CURSES_LIB="$MH_CURSES_LIB -ltermcap"
		LIBS="$LIBS -ltermcap"
	fi
fi

dnl
dnl If HAVE_BSD_CURSES, define BSD_STANDOUT as one of:
dnl _STANDOUT, __WSTANDOUT or __STANDOUT
dnl
if test "$mh_cv_sysv_curses" = no ; then
	bsd_standouts="_STANDOUT __WSTANDOUT __STANDOUT"
	for sout in $bsd_standouts ; do
AC_TRY_COMPILE(
[#include "xxxxx.h"]
[#include <$curses_h>],
[int xxx=$sout],
	[mh_bsd_sout=yes],
	[mh_bsd_sout=no]
	)
		if test "$mh_bsd_sout" = yes ; then
			AC_DEFINE_UNQUOTED(BSD_STANDOUT,$sout)
			break 2
		fi
	done
fi

AC_MSG_CHECKING(for wattrset in $curses_l library)
AC_CACHE_VAL(
[mh_cv_func_wattrset],
[
	AC_TRY_LINK(
[#include "xxxxx.h"]
[#include <$curses_h>],
[wattrset(stdscr,0)],
	[mh_cv_func_wattrset=yes],
	[mh_cv_func_wattrset=no]
	)
])dnl
AC_MSG_RESULT($mh_cv_func_wattrset)
if test "$mh_cv_func_wattrset" = yes ; then
	AC_DEFINE(HAVE_WATTRSET)
fi

AC_MSG_CHECKING(for keypad in $curses_l library)
AC_CACHE_VAL(
[mh_cv_func_keypad],
[
	AC_TRY_LINK(
[#include "xxxxx.h"]
[#include <$curses_h>],
[keypad(stdscr,TRUE)],
	[mh_cv_func_keypad=yes],
	[mh_cv_func_keypad=no]
	)
])dnl
AC_MSG_RESULT($mh_cv_func_keypad)
if test "$mh_cv_func_keypad" = yes ; then
	AC_DEFINE(HAVE_KEYPAD)
fi

AC_MSG_CHECKING(for beep in $curses_l library)
AC_CACHE_VAL(
[mh_cv_func_beep],
[
	AC_TRY_LINK(
[#include "xxxxx.h"]
[#include <$curses_h>],
[beep()],
	[mh_cv_func_beep=yes],
	[mh_cv_func_beep=no]
	)
])dnl
AC_MSG_RESULT($mh_cv_func_beep)
if test "$mh_cv_func_beep" = yes ; then
	AC_DEFINE(HAVE_BEEP)
fi

AC_MSG_CHECKING(for box in $curses_l library)
AC_CACHE_VAL(
[mh_cv_func_box],
[
	AC_TRY_LINK(
[#include "xxxxx.h"]
[#include <$curses_h>],
[box(stdscr,0,0)],
	[mh_cv_func_box=yes],
	[mh_cv_func_box=no]
	)
])dnl
AC_MSG_RESULT($mh_cv_func_box)
if test "$mh_cv_func_box" = yes ; then
	AC_DEFINE(HAVE_BOX)
fi

AC_MSG_CHECKING(for whline in $curses_l library)
AC_CACHE_VAL(
[mh_cv_func_whline],
[
	AC_TRY_LINK(
[#include "xxxxx.h"]
[#include <$curses_h>],
[whline(stdscr,0,1)],
	[mh_cv_func_whline=yes],
	[mh_cv_func_whline=no]
	)
])dnl
AC_MSG_RESULT($mh_cv_func_whline)
if test "$mh_cv_func_whline" = yes ; then
	AC_DEFINE(HAVE_WHLINE)
fi

AC_MSG_CHECKING(for curs_set in $curses_l library)
AC_CACHE_VAL(
[mh_cv_func_curs_set],
[
	AC_TRY_LINK(
[#include "xxxxx.h"]
[#include <$curses_h>],
[curs_set(0)],
	[mh_cv_func_curs_set=yes],
	[mh_cv_func_curs_set=no]
	)
])dnl
AC_MSG_RESULT($mh_cv_func_curs_set)
if test "$mh_cv_func_curs_set" = yes ; then
	AC_DEFINE(HAVE_CURS_SET)
fi

AC_MSG_CHECKING(for touchline in $curses_l library)
AC_CACHE_VAL(
[mh_cv_func_touchline],
[
	AC_TRY_LINK(
[#include "xxxxx.h"]
[#include <$curses_h>],
[touchline(stdscr,1,1)],
	[mh_cv_func_touchline=yes],
	[mh_cv_func_touchline=no]
	)
])dnl
AC_MSG_RESULT($mh_cv_func_touchline)
if test "$mh_cv_func_touchline" = yes ; then
	AC_DEFINE(HAVE_TOUCHLINE)
fi

AC_MSG_CHECKING(for typeahead in $curses_l library)
AC_CACHE_VAL(
[mh_cv_func_typeahead],
[
	AC_TRY_LINK(
[#include "xxxxx.h"]
[#include <$curses_h>],
[typeahead(1)],
	[mh_cv_func_typeahead=yes],
	[mh_cv_func_typeahead=no]
	)
])dnl
AC_MSG_RESULT($mh_cv_func_typeahead)
if test "$mh_cv_func_typeahead" = yes ; then
	AC_DEFINE(HAVE_TYPEAHEAD)
fi

AC_MSG_CHECKING(for notimeout in $curses_l library)
AC_CACHE_VAL(
[mh_cv_func_notimeout],
[
	AC_TRY_LINK(
[#include "xxxxx.h"]
[#include <$curses_h>],
[notimeout(stdscr,TRUE)],
	[mh_cv_func_notimeout=yes],
	[mh_cv_func_notimeout=no]
	)
])dnl
AC_MSG_RESULT($mh_cv_func_notimeout)
if test "$mh_cv_func_notimeout" = yes ; then
	AC_DEFINE(HAVE_NOTIMEOUT)
fi

AC_MSG_CHECKING(for ungetch in $curses_l library)
AC_CACHE_VAL(
[mh_cv_func_ungetch],
[
	AC_TRY_LINK(
[#include "xxxxx.h"]
[#include <$curses_h>],
[ungetch(10)],
	[mh_cv_func_ungetch=yes],
	[mh_cv_func_ungetch=no]
	)
])dnl
AC_MSG_RESULT($mh_cv_func_ungetch)
if test "$mh_cv_func_ungetch" = yes ; then
	AC_DEFINE(HAVE_UNGETCH)
fi

AC_MSG_CHECKING(for nodelay in $curses_l library)
AC_CACHE_VAL(
[mh_cv_func_nodelay],
[
	AC_TRY_LINK(
[#include "xxxxx.h"]
[#include <$curses_h>],
[nodelay(stdscr,TRUE)],
	[mh_cv_func_nodelay=yes],
	[mh_cv_func_nodelay=no]
	)
])dnl
AC_MSG_RESULT($mh_cv_func_nodelay)
if test "$mh_cv_func_nodelay" = yes ; then
	AC_DEFINE(HAVE_NODELAY)
fi

AC_MSG_CHECKING(for raw in $curses_l library)
AC_CACHE_VAL(
[mh_cv_func_raw],
[
	AC_TRY_LINK(
[#include "xxxxx.h"]
[#include <$curses_h>],
[raw()],
	[mh_cv_func_raw=yes],
	[mh_cv_func_raw=no]
	)
])dnl
AC_MSG_RESULT($mh_cv_func_raw)
if test "$mh_cv_func_raw" = yes ; then
	AC_DEFINE(HAVE_RAW)
fi

AC_MSG_CHECKING(for cbreak in $curses_l library)
AC_CACHE_VAL(
[mh_cv_func_cbreak],
[
	AC_TRY_LINK(
[#include "xxxxx.h"]
[#include <$curses_h>],
[cbreak()],
	[mh_cv_func_cbreak=yes],
	[mh_cv_func_cbreak=no]
	)
])dnl
AC_MSG_RESULT($mh_cv_func_cbreak)
if test "$mh_cv_func_cbreak" = yes ; then
	AC_DEFINE(HAVE_CBREAK)
fi

AC_MSG_CHECKING(for nocbreak in $curses_l library)
AC_CACHE_VAL(
[mh_cv_func_nocbreak],
[
	AC_TRY_LINK(
[#include "xxxxx.h"]
[#include <$curses_h>],
[nocbreak()],
	[mh_cv_func_nocbreak=yes],
	[mh_cv_func_nocbreak=no]
	)
])dnl
AC_MSG_RESULT($mh_cv_func_nocbreak)
if test "$mh_cv_func_nocbreak" = yes ; then
	AC_DEFINE(HAVE_NOCBREAK)
fi

AC_MSG_CHECKING(for waddchnstr in $curses_l library)
AC_CACHE_VAL(
[mh_cv_func_waddchnstr],
[
	AC_TRY_LINK(
[#include "xxxxx.h"]
[#include <$curses_h>],
[waddchnstr(stdscr,(chtype*)"A",1)],
	[mh_cv_func_waddchnstr=yes],
	[mh_cv_func_waddchnstr=no]
	)
])dnl
AC_MSG_RESULT($mh_cv_func_waddchnstr)
if test "$mh_cv_func_waddchnstr" = yes ; then
	AC_DEFINE(HAVE_WADDCHNSTR)
fi

AC_MSG_CHECKING(for wbkgd in $curses_l library)
AC_CACHE_VAL(
[mh_cv_func_wbkgd],
[
	AC_TRY_LINK(
[#include "xxxxx.h"]
[#include <$curses_h>],
[wbkgd(stdscr,0)],
	[mh_cv_func_wbkgd=yes],
	[mh_cv_func_wbkgd=no]
	)
])dnl
AC_MSG_RESULT($mh_cv_func_wbkgd)
if test "$mh_cv_func_wbkgd" = yes ; then
	AC_DEFINE(HAVE_WBKGD)
fi


AC_MSG_CHECKING(for wnoutrefresh in $curses_l library)
AC_CACHE_VAL(
[mh_cv_func_wnoutrefresh],
[
	AC_TRY_LINK(
[#include "xxxxx.h"]
[#include <$curses_h>],
[wnoutrefresh(stdscr)],
	[mh_cv_func_wnoutrefresh=yes],
	[mh_cv_func_wnoutrefresh=no]
	)
])dnl
AC_MSG_RESULT($mh_cv_func_wnoutrefresh)
if test "$mh_cv_func_wnoutrefresh" = yes ; then
	AC_DEFINE(HAVE_WNOUTREFRESH)
fi

AC_MSG_CHECKING(for doupdate in $curses_l library)
AC_CACHE_VAL(
[mh_cv_func_doupdate],
[
	AC_TRY_LINK(
[#include "xxxxx.h"]
[#include <$curses_h>],
[doupdate()],
	[mh_cv_func_doupdate=yes],
	[mh_cv_func_doupdate=no]
	)
])dnl
AC_MSG_RESULT($mh_cv_func_doupdate)
if test "$mh_cv_func_doupdate" = yes ; then
	AC_DEFINE(HAVE_DOUPDATE)
fi

AC_MSG_CHECKING(for reset_shell_mode in $curses_l library)
AC_CACHE_VAL(
[mh_cv_func_reset_shell_mode],
[
	AC_TRY_LINK(
[#include "xxxxx.h"]
[#include <$curses_h>],
[reset_shell_mode()],
	[mh_cv_func_reset_shell_mode=yes],
	[mh_cv_func_reset_shell_mode=no]
	)
])dnl
AC_MSG_RESULT($mh_cv_func_reset_shell_mode)
if test "$mh_cv_func_reset_shell_mode" = yes ; then
	AC_DEFINE(HAVE_RESET_SHELL_MODE)
fi

AC_MSG_CHECKING(for reset_prog_mode in $curses_l library)
AC_CACHE_VAL(
[mh_cv_func_reset_prog_mode],
[
	AC_TRY_LINK(
[#include "xxxxx.h"]
[#include <$curses_h>],
[reset_prog_mode()],
	[mh_cv_func_reset_prog_mode=yes],
	[mh_cv_func_reset_prog_mode=no]
	)
])dnl
AC_MSG_RESULT($mh_cv_func_reset_prog_mode)
if test "$mh_cv_func_reset_prog_mode" = yes ; then
	AC_DEFINE(HAVE_RESET_PROG_MODE)
fi

AC_MSG_CHECKING(for slk_init in $curses_l library)
AC_CACHE_VAL(
[mh_cv_func_slk_init],
[
	AC_TRY_LINK(
[#include "xxxxx.h"]
[#include <$curses_h>],
[slk_init(0)],
	[mh_cv_func_slk_init=yes],
	[mh_cv_func_slk_init=no]
	)
])dnl
AC_MSG_RESULT($mh_cv_func_slk_init)
if test "$mh_cv_func_slk_init" = yes ; then
	AC_DEFINE(HAVE_SLK_INIT)
fi

AC_MSG_CHECKING(for slk_attrset in $curses_l library)
AC_CACHE_VAL(
[mh_cv_func_slk_attrset],
[
	AC_TRY_LINK(
[#include "xxxxx.h"]
[#include <$curses_h>],
[slk_attrset(0)],
	[mh_cv_func_slk_attrset=yes],
	[mh_cv_func_slk_attrset=no]
	)
])dnl
AC_MSG_RESULT($mh_cv_func_slk_attrset)
if test "$mh_cv_func_slk_attrset" = yes ; then
	AC_DEFINE(HAVE_SLK_ATTRSET)
fi

AC_MSG_CHECKING(for sb_init in $curses_l library)
AC_CACHE_VAL(
[mh_cv_func_sb_init],
[
	AC_TRY_LINK(
[#include "xxxxx.h"]
[#include <$curses_h>],
[sb_init()],
	[mh_cv_func_sb_init=yes],
	[mh_cv_func_sb_init=no]
	)
])dnl
AC_MSG_RESULT($mh_cv_func_sb_init)
if test "$mh_cv_func_sb_init" = yes ; then
	AC_DEFINE(HAVE_SB_INIT)
fi

AC_MSG_CHECKING(for resize_term in $curses_l library)
AC_CACHE_VAL(
[mh_cv_func_resize_term],
[
	AC_TRY_LINK(
[#include "xxxxx.h"]
[#include <$curses_h>],
[resize_term(0,0)],
	[mh_cv_func_resize_term=yes],
	[mh_cv_func_resize_term=no]
	)
])dnl
AC_MSG_RESULT($mh_cv_func_resize_term)
if test "$mh_cv_func_resize_term" = yes ; then
	AC_DEFINE(HAVE_RESIZE_TERM)
fi

AC_MSG_CHECKING(for wvline in $curses_l library)
AC_CACHE_VAL(
[mh_cv_func_wvline],
[
	AC_TRY_LINK(
[#include "xxxxx.h"]
[#include <$curses_h>],
[wvline(stdscr,0,1)],
	[mh_cv_func_wvline=yes],
	[mh_cv_func_wvline=no]
	)
])dnl
AC_MSG_RESULT($mh_cv_func_wvline)
if test "$mh_cv_func_wvline" = yes ; then
	AC_DEFINE(HAVE_WVLINE)
fi

AC_MSG_CHECKING(if $curses_h WINDOW structure contains _begy)
AC_CACHE_VAL(
[mh_cv_have_underscore_begy],
[
AC_TRY_COMPILE(
[#include "xxxxx.h"]
[#include <$curses_h>],
[int xxx;WINDOW *w;xxx=w->_begy;],
[mh_cv_have_underscore_begy=yes],
[mh_cv_have_underscore_begy=no]
)
])dnl
AC_MSG_RESULT($mh_cv_have_underscore_begy)
if test "$mh_cv_have_underscore_begy" = yes ; then
	AC_DEFINE(HAVE_UNDERSCORE_BEGY)
fi

AC_MSG_CHECKING(if $curses_h WINDOW structure contains begy)
AC_CACHE_VAL(
[mh_cv_have_begy],
[
AC_TRY_COMPILE(
[#include "xxxxx.h"]
[#include <$curses_h>],
[int xxx;WINDOW *w;xxx=w->begy;],
[mh_cv_have_begy=yes],
[mh_cv_have_begy=no]
)
])dnl
AC_MSG_RESULT($mh_cv_have_begy)
if test "$mh_cv_have_begy" = yes ; then
	AC_DEFINE(HAVE_BEGY)
fi

AC_MSG_CHECKING(if $curses_h WINDOW structure contains _maxy)
AC_CACHE_VAL(
[mh_cv_have_underscore_maxy],
[
AC_TRY_COMPILE(
[#include "xxxxx.h"]
[#include <$curses_h>],
[int xxx;WINDOW *w;xxx=w->_maxy;],
[mh_cv_have_underscore_maxy=yes],
[mh_cv_have_underscore_maxy=no]
)
])dnl
AC_MSG_RESULT($mh_cv_have_underscore_maxy)
if test "$mh_cv_have_underscore_maxy" = yes ; then
	AC_DEFINE(HAVE_UNDERSCORE_MAXY)
fi

AC_MSG_CHECKING(if $curses_h WINDOW structure contains maxy)
AC_CACHE_VAL(
[mh_cv_have_maxy],
[
AC_TRY_COMPILE(
[#include "xxxxx.h"]
[#include <$curses_h>],
[int xxx;WINDOW *w;xxx=w->maxy;],
[mh_cv_have_maxy=yes],
[mh_cv_have_maxy=no]
)
])dnl
AC_MSG_RESULT($mh_cv_have_maxy)
if test "$mh_cv_have_maxy" = yes ; then
	AC_DEFINE(HAVE_MAXY)
fi

rm -f xxxxx.h
LIBS="$mh_save_libs"
CFLAGS="$mh_save_cflags"
])dnl

dnl ---------------------------------------------------------------------------
dnl Determine if curses defines "chtype"
dnl ---------------------------------------------------------------------------
AC_DEFUN([MH_CHECK_CHTYPE],
[
mh_save_libs="$LIBS"
mh_save_cflags="$CFLAGS"

cat > xxxxx.h <<EOF
#ifdef XCURSES
	char *XCursesProgramName="test";
#endif
EOF
dnl
dnl Include before curses.h any include files required. This is
dnl often stdarg.h
dnl
for incfile in $mh_pre_curses_h_include ; do
	echo "#include <$incfile.h>" >> xxxxx.h
done

if test "$with_xcurses" = yes; then
	CFLAGS="-DXCURSES $MH_CURSES_INC $SYS_DEFS"
	LIBS="$LIBS $MH_CURSES_LIB $MH_XLIBS"
else
	CFLAGS="$MH_CURSES_INC $SYS_DEFS"
	LIBS="$LIBS $MH_CURSES_LIB"
fi

AC_MSG_CHECKING(if $curses_h defines "chtype")
AC_TRY_COMPILE(
[#include "xxxxx.h"]
[#include <$curses_h>],
[chtype xxx],
[mh_have_chtype=yes],
[mh_have_chtype=no;AC_DEFINE(chtype,int)]
)
rm -f xxxxx.h
AC_MSG_RESULT($mh_have_chtype)
LIBS="$mh_save_libs"
CFLAGS="$mh_save_cflags"
])dnl

dnl ---------------------------------------------------------------------------
dnl Check curses library and header files
dnl ---------------------------------------------------------------------------
AC_DEFUN([MH_CHECK_CURSES],
[
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

if test "$with_xcurses" = yes; then
	AC_DEFINE(USE_XCURSES)
	AC_CHECK_PROG(xcurses_config, [xcurses-config], yes, no)
	curses_h="xcurses.h"
	curses_l="XCurses"
else
	if test "$with_ncurses" = yes; then
		AC_DEFINE(USE_NCURSES)
		curses_h="ncurses.h"
		curses_l="ncurses"
	else
		if test "$with_extcurses" = yes; then
			AC_DEFINE(USE_EXTCURSES)
			curses_h="cur00.h"
			curses_l="cur"
		else
			if test "$mh_curses_colr" = yes; then
				curses_h="curses.h"
				curses_l="cur_colr"
			else
				curses_h="curses.h"
				curses_l="curses"
			fi
		fi
	fi
fi

dnl look for curses header and library, exit if not found
AC_MSG_CHECKING(for location of $curses_h header file)
mh_curses_inc_dir=""
mh_inc_dirs="\
    ${CURSESINCDIR}           \
    ${mh_sysv_incdir}         \
    ${exec_prefix}/include    \
    ${HOME}/include           \
    /usr/local/include        \
    /usr/contrib/include      \
    /usr/include/curses_colr  \
    /opt/include              \
    /usr/include              \
    /usr/ucbinclude           \
    /usr/ucbinc               \
    /usr/unsupported/include"
dnl
dnl Provide for user supplying directory
dnl
if test "$with_cursesincdir" != no ; then
	mh_inc_dirs="$with_cursesincdir $mh_inc_dirs"
fi
dnl
dnl Try to determine the directory containing curses header
dnl
for ac_dir in $mh_inc_dirs ; do
  if test -r $ac_dir/$curses_h; then
    mh_curses_inc_dir=$ac_dir
    break
  fi
done
if test "x$mh_curses_inc_dir" != "x" ; then
	AC_MSG_RESULT(found in $mh_curses_inc_dir)
	MH_CURSES_INC="-I$mh_curses_inc_dir"
dnl
dnl If using gcc under Solaris 2, don't use -I/usr/include
dnl
case "$target" in
	*solaris*)
		if test "$ac_cv_prog_gcc" = yes ; then
			if test "x$mh_curses_inc_dir" = "x/usr/include" ; then
			MH_CURSES_INC=""
			fi
		fi
		;;
esac
	AC_SUBST(MH_CURSES_INC)
else
	AC_MSG_ERROR(Cannot find curses header file: $curses_h; THE cannot be configured)
fi

AC_MSG_CHECKING(for location of $curses_l library file)
mh_curses_lib_dir=""
mh_lib_dirs="\
    ${CURSESLIBDIR}           \
    ${mh_sysv_libdir}         \
    ${exec_prefix}/lib        \
    ${HOME}/lib               \
    /usr/local/lib            \
    /usr/contrib/lib          \
    /opt/lib                  \
    /usr/lib                  \
    /usr/ccs/lib              \
    /usr/ucblib               \
    /usr/unsupported/lib"
dnl
dnl Provide for user supplying directory
dnl
if test "$with_curseslibdir" != no ; then
	mh_lib_dirs="$with_curseslibdir $mh_lib_dirs"
fi
dnl
dnl Try to determine the directory containing curses library
dnl
for ac_dir in $mh_lib_dirs ; do
	for mh_ext in lib${curses_l}.a lib${curses_l}.so lib${curses_l}.sl ${curses_l}.lib ${curses_l}3r.lib; do
	  if test -r $ac_dir/$mh_ext; then
	     mh_curses_lib_dir=$ac_dir
	     break 2
	  fi
	done
done
if test "x$mh_curses_lib_dir" != "x" ; then
	MH_CURSES_LIB="-L$mh_curses_lib_dir -l$curses_l"
	AC_MSG_RESULT(found in $mh_curses_lib_dir)
	AC_SUBST(MH_CURSES_LIB)
else
	AC_MSG_ERROR(Cannot find curses library file: $curses_l; THE cannot be configured)
fi
])dnl

dnl ---------------------------------------------------------------------------
dnl Check for broken SYSVR4 curses implementations
dnl ---------------------------------------------------------------------------
AC_DEFUN([MH_CHECK_BROKEN_SYSVR4_CURSES],
[
AC_MSG_CHECKING(if $curses_l is a broken SYSVR4 curses)
dnl
dnl Known platform is Solaris 2.5+
dnl
case "$target" in
	*solaris2.5*)
		if test "$curses_l" = "curses" ; then
			mh_broken_sysvr4_curses=yes
		else
			mh_broken_sysvr4_curses=no
		fi
		;;
	*solaris2.6*)
		if test "$curses_l" = "curses" ; then
			mh_broken_sysvr4_curses=yes
		else
			mh_broken_sysvr4_curses=no
		fi
		;;
	*)mh_broken_sysvr4_curses=no
esac
if test "$mh_broken_sysvr4_curses" = yes ; then
	AC_DEFINE(HAVE_BROKEN_SYSVR4_CURSES)
fi
AC_MSG_RESULT($mh_broken_sysvr4_curses)
])dnl

dnl ---------------------------------------------------------------------------
dnl Check location of xterm for XCURSES version
dnl ---------------------------------------------------------------------------
AC_DEFUN([MH_FIND_XTERM],
[
if test "$with_xcurses" = yes; then
	mh_xterm_found=no
	AC_MSG_CHECKING(for location of xterm)
	mh_xterms="\
	    /usr/X11R6/bin/xterm      \
	    /usr/bin/X11R6/xterm      \
	    /usr/X11R5/bin/xterm      \
	    /usr/bin/X11R5/xterm      \
	    /usr/X11/bin/xterm        \
	    /usr/openwin/bin/xterm    \
	    /usr/bin/X11/xterm        \
	    /usr/local/bin/xterm      \
	    /usr/contrib/bin/xterm"
	for sout in $mh_xterms ; do
		if test -x $sout ; then
				mh_xterm_found=yes
			AC_DEFINE_UNQUOTED(XTERM_PROGRAM,"$sout")
			AC_MSG_RESULT(found in $sout)
			break 2
		fi
	done
	if test "$mh_xterm_found" = no ; then
		AC_DEFINE_UNQUOTED(XTERM_PROGRAM,"N/A")
		AC_MSG_RESULT(not found. You will need to run SET XTERMINAL before invoking a shell command within THE)
	fi
fi
])dnl

dnl ---------------------------------------------------------------------------
dnl Check for acl_get function under AIX
dnl ---------------------------------------------------------------------------
AC_DEFUN([MH_FUNC_ACL_GET],
[
mh_save_libs="$LIBS"
LIBS="$LIBS -ls"
mh_save_cflags="$CFLAGS"
CFLAGS="$CFLAGS -D_ALL_SOURCE"
AC_MSG_CHECKING(for function acl_get)
AC_CACHE_VAL(
[mh_cv_func_acl_get],
[
	AC_TRY_LINK(
[#include <sys/acl.h>],
[char *ptr=(char *)acl_get("XXX")],
	[mh_cv_func_acl_get=yes],
	[mh_cv_func_acl_get=no]
	)
])dnl
LIBS="$mh_save_libs"
CFLAGS="$mh_save_cflags"
AC_MSG_RESULT($mh_cv_func_acl_get)
if test "$mh_cv_func_acl_get" = yes ; then
	AC_DEFINE(HAVE_ACL_GET)
#	MH_EXTRA_LIBS="-ls"
#	AC_SUBST(MH_EXTRA_LIBS)
fi
])dnl

dnl ---------------------------------------------------------------------------
dnl Check for Checker
dnl ---------------------------------------------------------------------------
AC_DEFUN([MH_CHECK_CHECKER],
[
AC_CACHE_VAL(
[mh_cv_checker],
[
AC_MSG_CHECKING(for Checker)
if (checkergcc) >/dev/null 2>/dev/null; then
	mh_cv_checker=yes
	AC_MSG_RESULT(yes)
else
	mh_cv_checker=no
	AC_MSG_RESULT(no)
fi
])
])dnl

dnl
dnl include our own common routines
dnl
sinclude(common/accommon.m4)dnl
