dnl MH_HAVE_PROTO
dnl MH_PROG_CC
dnl MH_CHECK_CC_O
dnl MH_CHECK__SIGHANDLER_T
dnl MH_STRUCT_RANDOM_DATA
dnl MH_GETGRGID_R_INT_RETURN_5_PARAMS
dnl MH_GETGRGID_R_INT_RETURN_4_PARAMS
dnl MH_GETGRGID_R_STRUCT_RETURN
dnl MH_GETPWUID_R_INT_RETURN
dnl MH_GETPWUID_R_STRUCT_RETURN
dnl MH_GETHOSTBYNAME_R_INT_RETURN_6_PARAMS
dnl MH_GETHOSTBYNAME_R_STRUCT_RETURN_5_PARAMS
dnl MH_CHECK_POSIX_THREADS
dnl MH_CHECK_LEADING_USCORE
dnl MH_HOWTO_DYN_LINK
dnl MH_HOWTO_SHARED_LIBRARY
dnl MH_CHECK_LIB
dnl MH_CHECK_CRYPT
dnl MH_SHLPST
dnl MH_SET_SHLPST
dnl MH_SHARED_LIBRARY
dnl MH_STATIC_LOADING
dnl MH_CHECK_UNSIGNED_CHAR_COMPILER_SWITCH
dnl MH_CHECK_RPM
dnl MH_C_LONG_LONG
dnl MH_CHECK_TYPE_SOCKLEN_T
dnl MH_CHECK_F_MNTFROMNAME
dnl MH_LARGE_FILE_SUPPORT
dnl MH_CHECK_OSX_ARCH
dnl MH_GET_DISTRO_NAME
dnl
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
dnl ---------------------------------------------------------------------------
dnl Determine the best C compiler to use given a list
dnl ---------------------------------------------------------------------------
AC_DEFUN([MH_PROG_CC],
[
mh_sysv_incdir=""
mh_sysv_libdir=""
all_words="clang xlc gcc c99 c89 acc cc"
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
dnl Check if C compiler supports -c -o file.ooo
dnl ---------------------------------------------------------------------------
AC_DEFUN([MH_CHECK_CC_O],
[
AC_MSG_CHECKING(whether $CC understands -c and -o together)
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
        O2TSO=""
        O2TSAVE=""
        TSAVE2O=""
        CC2O="-o $"'@'
        AC_MSG_RESULT(yes)
else
        O2SHO="-mv \`basename "$'@'" .so.o\`.o "$'@'
        O2SAVE="-mv \`basename "$'@'" .so.o\`.o \`basename "$'@'" .so.o\`.o.save"
        SAVE2O="-mv \`basename "$'@'" .so.o\`.o.save \`basename "$'@'" .so.o\`.o"
        O2TSO="-mv \`basename "$'@'" .tso\`.o "$'@'
        O2TSAVE="-mv \`basename "$'@'" .tso\`.o \`basename "$'@'" .tso\`.tso.save"
        TSAVE2O="-mv \`basename "$'@'" .tso\`.tso.save \`basename "$'@'" .tso\`.o"
        CC2O=""
        AC_MSG_RESULT(no)
fi
])
dnl ---------------------------------------------------------------------------
dnl Check if gcc compiler supports -fno-strict-aliasing
dnl ---------------------------------------------------------------------------
AC_DEFUN([MH_CHECK_GCC_STRICT_ALIASING],
[
AC_MSG_CHECKING(whether gcc understands -fno-strict-aliasing)
AC_CACHE_VAL(
[mh_strict_aliasing],
[
mh_save_libs="$LIBS"
LIBS="-fno-strict-aliasing"
AC_LINK_IFELSE(
[AC_LANG_PROGRAM([#include <stdio.h>],
[exit(0)])],
[mh_strict_aliasing=yes],
[mh_strict_aliasing=no]
)
LIBS="$mh_save_libs"
if test "$mh_strict_aliasing" = yes; then
	MH_STRICT_ALIASING="-fno-strict-aliasing"
else
	MH_STRICT_ALIASING=""
fi
AC_SUBST(MH_STRICT_ALIASING)
])
AC_MSG_RESULT($mh_strict_aliasing)
])
dnl ---------------------------------------------------------------------------
dnl Check if compiler supports __builtin_return_address
dnl ---------------------------------------------------------------------------
AC_DEFUN([MH_CHECK_BUILTIN_RETURN_ADDRESS],
[
AC_MSG_CHECKING(whether compiler supports __builtin_return_address inline function)
AC_CACHE_VAL(
[mh_cv_builtin_return_address],
[
AC_TRY_LINK(
[#include <stdio.h>],
[__builtin_return_address(0)],
[mh_cv_builtin_return_address=yes],
[mh_cv_builtin_return_address=no]
)
])
AC_MSG_RESULT($mh_cv_builtin_return_address)
if test "$mh_cv_builtin_return_address" = yes; then
	AC_DEFINE(HAVE_BUILTIN_RETURN_ADDRESS)
fi
])

dnl ---------------------------------------------------------------------------
dnl Determine compiler's "unsigned char" is default switch
dnl ---------------------------------------------------------------------------
AC_DEFUN([MH_CHECK_UNSIGNED_CHAR_COMPILER_SWITCH],
[
AC_C_CHAR_UNSIGNED()
AC_MSG_CHECKING(compiler switch for unsigned char)
#
# QNX4 Watcom compiler is unsigned char by default
#
if test "$on_qnx4" = yes -a "$ac_cv_prog_CC" = "cc" ; then
	qnx4_char_unsigned="yes"
fi

if test "$ac_cv_c_char_unsigned" = yes -o "$qnx4_char_unsigned" = yes; then
	MH_UNSIGNED_CHAR_SWITCH=""
	msg="not needed - char is unsigned by default"
else
	case "$ac_cv_prog_CC" in
		gcc)
			MH_UNSIGNED_CHAR_SWITCH="-funsigned-char"
			msg="$MH_UNSIGNED_CHAR_SWITCH"
			;;
		xlC)
			MH_UNSIGNED_CHAR_SWITCH="-qchars=unsigned"
			msg="$MH_UNSIGNED_CHAR_SWITCH"
			;;
		*)
			save_cflags="$CFLAGS"
			mh_have_unsigned_char_switch=no
			mh_unsigned_flags="-funsigned-char -xchar=unsigned"
			for mh_flag in $mh_unsigned_flags; do
				CFLAGS="$CFLAGS $mh_flag"
AC_TRY_COMPILE([#include <stdio.h>],
[char x;],
  MH_UNSIGNED_CHAR_SWITCH="$mh_flag"; msg="$MH_UNSIGNED_CHAR_SWITCH"; mh_have_unsigned_char_switch=yes )
				CFLAGS="$save_cflags"
				if test "$mh_have_unsigned_char_switch" = yes; then
					break
				fi
			done
			if test "$mh_have_unsigned_char_switch" = no; then
				msg="unknown - assuming unsigned by default but running execiser will confirm"
			fi
			;;
	esac
fi
AC_MSG_RESULT("$msg")
AC_SUBST(MH_UNSIGNED_CHAR_SWITCH)
])
dnl ---------------------------------------------------------------------------
dnl Check if gcc compiler supports --version-script
dnl ---------------------------------------------------------------------------
AC_DEFUN([MH_CHECK_GCC_VERSION_SCRIPT],
[
AC_MSG_CHECKING(whether gcc understands --version-script)
AC_CACHE_VAL(
[mh_cv_version_script],
[
echo "TEST {" > conftest.def
echo "global:" >> conftest.def
echo "Test;" >> conftest.def
echo "};" >> conftest.def
mh_save_libs="$LDFLAGS"
LDFLAGS="$LDFLAGS -Wl,--version-script=conftest.def"
AC_LINK_IFELSE(
[AC_LANG_PROGRAM([#include <stdio.h>],
[exit(0);])],
[mh_cv_version_script=yes],
[mh_cv_version_script=no]
)
LDFLAGS="$mh_save_libs"
rm -f conftest.def
])
if test "$mh_cv_version_script" = yes; then
	SHL_SCRIPT="-Wl,--version-script=${srcdir}/regina_elf.def"
else
	SHL_SCRIPT=""
fi
AC_SUBST(SHL_SCRIPT)
AC_MSG_RESULT($mh_cv_version_script)
])

dnl ---------------------------------------------------------------------------
dnl Check if __sighandler_t is defined
dnl ---------------------------------------------------------------------------
AC_DEFUN([MH_CHECK__SIGHANDLER_T],
[
AC_MSG_CHECKING(whether __sighandler_t is defined)

AC_CACHE_VAL(
[mh_cv__sighandler_t],
[
AC_TRY_COMPILE(
[#include <sys/types.h>]
[#include <signal.h>],
[__sighandler_t fred],
[mh_cv__sighandler_t=yes],
[mh_cv__sighandler_t=no]
)
])
AC_MSG_RESULT($mh_cv__sighandler_t)
if test "$mh_cv__sighandler_t" = yes ; then
        AC_DEFINE(HAVE__SIGHANDLER_T)
fi
])


dnl ---------------------------------------------------------------------------
dnl Work out if struct random_data is defined
dnl ---------------------------------------------------------------------------
AC_DEFUN(MH_STRUCT_RANDOM_DATA,
[AC_CACHE_CHECK([for struct random_data], mh_cv_struct_random_data,
[AC_TRY_COMPILE([#include <stdlib.h>
#include <stdio.h>],
[struct random_data s;],
mh_cv_struct_random_data=yes, mh_cv_struct_random_data=no)])
if test $mh_cv_struct_random_data = yes; then
  AC_DEFINE(HAVE_RANDOM_DATA)
fi
])

dnl ---------------------------------------------------------------------------
dnl Work out if getgrgid_r returns an int and has 5 parameters
dnl ---------------------------------------------------------------------------
AC_DEFUN(MH_GETGRGID_R_INT_RETURN_5_PARAMS,
[AC_CACHE_CHECK([if getgrgid_r has 5 args and returns an int], mh_cv_getgrgid_r_int5,
[AC_TRY_COMPILE([#include <sys/types.h>
#include <grp.h>
#include <pwd.h>],
[
struct group *ptr,result;
int rc;
gid_t gid;
char buf[100];
rc = getgrgid_r(gid,&result,buf,sizeof(buf),&ptr);
],
mh_cv_getgrgid_r_int5=yes, mh_cv_getgrgid_r_int5=no)])
if test $mh_cv_getgrgid_r_int5 = yes; then
  AC_DEFINE(HAVE_GETGRGID_R_RETURNS_INT_5_PARAMS)
fi
])

dnl ---------------------------------------------------------------------------
dnl Work out if getgrgid_r returns an int and has 4 parameters
dnl ---------------------------------------------------------------------------
AC_DEFUN(MH_GETGRGID_R_INT_RETURN_4_PARAMS,
[AC_CACHE_CHECK([if getgrgid_r has 4 args and returns an int], mh_cv_getgrgid_r_int4,
[AC_TRY_COMPILE([#include <sys/types.h>
#include <grp.h>
#include <pwd.h>],
[
struct group *ptr,result;
int rc;
gid_t gid;
char buf[100];
rc = getgrgid_r(gid,&result,buf,sizeof(buf));
],
mh_cv_getgrgid_r_int4=yes, mh_cv_getgrgid_r_int4=no)])
if test $mh_cv_getgrgid_r_int4 = yes; then
  AC_DEFINE(HAVE_GETGRGID_R_RETURNS_INT_4_PARAMS)
fi
])

dnl ---------------------------------------------------------------------------
dnl Work out if getgrgid_r returns a struct group *
dnl ---------------------------------------------------------------------------
AC_DEFUN(MH_GETGRGID_R_STRUCT_RETURN,
[AC_CACHE_CHECK([if getgrgid_r returns a struct group *], mh_cv_getgrgid_r_struct,
[AC_TRY_COMPILE([#include <sys/types.h>
#include <grp.h>
#include <pwd.h>],
[
struct group *ptr,result;
int rc;
gid_t gid;
char buf[100];
ptr = getgrgid_r(gid,&result,buf,sizeof(buf));
],
mh_cv_getgrgid_r_struct=yes, mh_cv_getgrgid_r_struct=no)])
if test $mh_cv_getgrgid_r_struct = yes; then
  AC_DEFINE(HAVE_GETGRGID_R_RETURNS_STRUCT)
fi
])

dnl ---------------------------------------------------------------------------
dnl Work out if getpwuid_r returns an int
dnl ---------------------------------------------------------------------------
AC_DEFUN(MH_GETPWUID_R_INT_RETURN,
[AC_CACHE_CHECK([if getpwuid_r returns an int], mh_cv_getpwuid_r_int,
[AC_TRY_COMPILE([#include <sys/types.h>
#include <grp.h>
#include <pwd.h>],
[
struct passwd *ptr,*result;
int rc;
uid_t uid;
char buf[100];
rc = getpwuid_r(uid,&result,buf,sizeof(buf),&ptr);
],
mh_cv_getpwuid_r_int=yes, mh_cv_getpwuid_r_int=no)])
if test $mh_cv_getpwuid_r_int = yes; then
  AC_DEFINE(HAVE_GETPWUID_R_RETURNS_INT)
fi
])

dnl ---------------------------------------------------------------------------
dnl Work out if getpwuid_r returns a struct group *
dnl ---------------------------------------------------------------------------
AC_DEFUN(MH_GETPWUID_R_STRUCT_RETURN,
[AC_CACHE_CHECK([if getpwuid_r returns a struct passwd *], mh_cv_getpwuid_r_struct,
[AC_TRY_COMPILE([#include <sys/types.h>
#include <grp.h>
#include <pwd.h>],
[
struct group *ptr,*result;
int rc;
uid_t uid;
char buf[100];
ptr = getpwuid_r(uid,&result,buf,sizeof(buf));
],
mh_cv_getpwuid_r_struct=yes, mh_cv_getpwuid_r_struct=no)])
if test $mh_cv_getpwuid_r_struct = yes; then
  AC_DEFINE(HAVE_GETPWUID_R_RETURNS_STRUCT)
fi
])

dnl ---------------------------------------------------------------------------
dnl Work out if gethostbyname_r returns a struct and has 6 parameters
dnl ---------------------------------------------------------------------------
AC_DEFUN(MH_GETHOSTBYNAME_R_INT_RETURN_6_PARAMS,
[AC_CACHE_CHECK([if gethostbyname_r has 6 args and returns an int], mh_cv_gethostbyname_r_int6,
[AC_TRY_COMPILE([#include <sys/types.h>
#include <netdb.h>],
[
int herr;
struct hostent *he,*rc;
char buf[8192];
struct hostent retval;
rc = gethostbyname_r("fred",&retval,buf,sizeof(buf),&he,&herr);
],
mh_cv_gethostbyname_r_int6=yes, mh_cv_gethostbyname_r_int6=no)])
if test $mh_cv_gethostbyname_r_int6 = yes; then
  AC_DEFINE(HAVE_GETHOSTBYNAME_R_RETURNS_INT_6_PARAMS)
fi
])

dnl ---------------------------------------------------------------------------
dnl Work out if gethostbyname_r returns a struct and has 5 parameters
dnl ---------------------------------------------------------------------------
AC_DEFUN(MH_GETHOSTBYNAME_R_STRUCT_RETURN_5_PARAMS,
[AC_CACHE_CHECK([if gethostbyname_r has 5 args and returns a struct], mh_cv_gethostbyname_r_struct5,
[AC_TRY_COMPILE([#include <sys/types.h>
#include <netdb.h>],
[
int herr;
struct hostent *he,*rc;
char buf[8192];
struct hostent retval;
rc = gethostbyname_r("fred",&retval,buf,sizeof(buf),&herr);
],
mh_cv_gethostbyname_r_struct5=yes, mh_cv_gethostbyname_r_struct5=no)])
if test $mh_cv_gethostbyname_r_struct5 = yes; then
  AC_DEFINE(HAVE_GETHOSTBYNAME_R_RETURNS_STRUCT_5_PARAMS)
fi
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
MT_FILE="mt_notmt"
if test "$enable_posix_threads" = yes; then
   dnl
   dnl -lrt is needed by posix threads on Solaris
   dnl
   AC_CHECK_LIB(rt,main,mh_rt_lib_found=yes,mh_rt_lib_found=no)
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
         THREADING_COMPILE="-D_REENTRANT -DPOSIX -DREGINA_REENTRANT"
         THREADING_LINK=""
         THREADSAFE_TARGET="threader${binarybitprefix}${EXEEXT}"
         MT_FILE="mt_posix"
         case "$target" in
            *solaris*)
               if test "$ac_cv_prog_CC" = "gcc" -o "$ac_cv_prog_CC" = "g++" -o "$ac_cv_prog_CC" = "clang"; then
                  THREADING_COMPILE="-D_REENTRANT -DPOSIX -DREGINA_REENTRANT"
                  if test "$mh_rt_lib_found" = "yes"; then
                     THREADING_LINK="-lrt"
                  fi
               else
                  THREADING_COMPILE="-mt -D_REENTRANT -DREENTRANT -D_PTHREAD_SEMANTICS -DREGINA_REENTRANT"
                  THREADING_LINK="-mt"
               fi
               ;;
            *hp-hpux1*)
               if test "$ac_cv_prog_CC" = "gcc" -o "$ac_cv_prog_CC" = "g++" -o "$ac_cv_prog_CC" = "clang"; then
                  THREADING_COMPILE="-D_REENTRANT -DPOSIX -DREGINA_REENTRANT"
               fi
               if test "$mh_rt_lib_found" = "yes"; then
                  THREADING_LINK="-lrt"
               fi
               ;;
            *linux*)
               ;;
            *freebsd*)
               THREADING_COMPILE="-pthread -D_REENTRANT -DPOSIX -DREGINA_REENTRANT"
               THREADING_LINK="-pthread"
               ;;
            *cygwin*)
               THREADING_COMPILE="-D_REENTRANT -DPOSIX -DREGINA_REENTRANT"
               THREADING_LINK=""
               ;;
            *)
               if test "$ac_cv_prog_CC" = "gcc" -o "$ac_cv_prog_CC" = "g++" -o "$ac_cv_prog_CC" = "clang"; then
                  THREADING_COMPILE="-D_REENTRANT -DPOSIX -DREGINA_REENTRANT"
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
AC_SUBST(THREADSAFE_TARGET)
AC_SUBST(MH_MT_LIBS)
AC_SUBST(MT_FILE)
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
   AC_RUN_IFELSE([AC_LANG_SOURCE([
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
#       mh_dyn_link='${CC} -Wl,-shared -o conftest.rxlib conftest.o -lc 1>&AC_FD_CC'
        if AC_TRY_EVAL(mh_dyn_link) && test -s conftest.rxlib; then
                LD_RXLIB_A1="ld -shared -o \$(@)"
                LD_RXLIB_A2="ld -shared -o \$(@)"
                LD_RXLIB_UTILA="ld -shared -o \$(@)"
#               LD_RXLIB1="${CC} -Wl,-shared"
                LD_RXLIB_B1="-L. -l${SHLFILE}"
                LD_RXLIB_B2="-L. -l${SHLFILE}"
                LD_RXLIB_UTILB="-L. -l${SHLFILE}"
                SHLPRE="lib"
                SHLPST=".so"
        else
                mh_dyn_link='ld -G -o conftest.rxlib conftest.o 1>&AC_FD_CC'
#               mh_dyn_link='${CC} -Wl,-G -o conftest.rxlib conftest.o 1>&AC_FD_CC'
                if AC_TRY_EVAL(mh_dyn_link) && test -s conftest.rxlib; then
                        LD_RXLIB_A1="ld -G -o \$(@)"
                        LD_RXLIB_A2="ld -G -o \$(@)"
                        LD_RXLIB_UTILA="ld -G -o \$(@)"
#                       LD_RXLIB1="${CC} -Wl,-G"
                        LD_RXLIB_B1="-L. -l${SHLFILE}"
                        LD_RXLIB_B2="-L. -l${SHLFILE}"
                        LD_RXLIB_UTILB="-L. -l${SHLFILE}"
                        SHLPRE="lib"
                        SHLPST=".so"
                else
                        LD_RXLIB_A1=""
                        LD_RXLIB_A2=""
                        LD_RXLIB_UTILA=""
                        LD_RXLIB_B1=""
                        LD_RXLIB_B2=""
                        LD_RXLIB_UTILB=""
                        SHLPRE=""
                        SHLPST=""
                fi
        fi
fi
rm -f conftest*
])dnl

dnl ---------------------------------------------------------------------------
dnl Work out how to create a shared library
dnl ---------------------------------------------------------------------------
AC_DEFUN(MH_HOWTO_SHARED_LIBRARY,
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
#       mh_dyn_link='${CC} -Wl,-shared -o conftest.so.1.0 conftest.o -lc 1>&AC_FD_CC'
        if AC_TRY_EVAL(mh_dyn_link) && test -s conftest.so.1.0; then
                SHL_LD="ld -shared -o ${SHLPRE}${SHLFILE}${SHLPST} "'$('SHOFILES')'" -lc"
#               SHL_LD="${CC} -Wl,-shared -o ${SHLPRE}${SHLFILE}${SHLPST} "'$('SHOFILES')'" -lc"
        else
                mh_dyn_link='ld -G -o conftest.so.1.0 conftest.o 1>&AC_FD_CC'
#               mh_dyn_link='${CC} -Wl,-G -o conftest.so.1.0 conftest.o 1>&AC_FD_CC'
                if AC_TRY_EVAL(mh_dyn_link) && test -s conftest.so.1.0; then
                        SHL_LD="ld -G -o ${SHLPRE}${SHLFILE}${SHLPST} "'$('SHOFILES')'
#                       SHL_LD="${CC} -Wl,-G -o ${SHLPRE}${SHLFILE}${SHLPST} "'$('SHOFILES')'
                else
                        mh_dyn_link='ld -o conftest.so.1.0 -shared -no_archive conftest.o  -lc 1>&AC_FD_CC'
#                       mh_dyn_link='${CC} -o conftest.so.1.0 -Wl,-shared,-no_archive conftest.o  -lc 1>&AC_FD_CC'
                        if AC_TRY_EVAL(mh_dyn_link) && test -s conftest.so.1.0; then
                                SHL_LD="ld -o ${SHLPRE}${SHLFILE}${SHLPST} -shared -no_archive "'$('SHOFILES')'" -lc"
#                               SHL_LD="${CC} -o ${SHLPRE}${SHLFILE}${SHLPST} -Wl,-shared,-no_archive "'$('SHOFILES')'" -lc"
                        else
                                mh_dyn_link='ld -b -o conftest.so.1.0 conftest.o 1>&AC_FD_CC'
#                               mh_dyn_link='${CC} -Wl,-b -o conftest.so.1.0 conftest.o 1>&AC_FD_CC'
                                if AC_TRY_EVAL(mh_dyn_link) && test -s conftest.so.1.0; then
                                        SHL_LD="ld -b -o ${SHLPRE}${SHLFILE}${SHLPST} "'$('SHOFILES')'
#                                       SHL_LD="${CC} -Wl,-b -o ${SHLPRE}${SHLFILE}.${SHLPST} "'$('SHOFILES')'
                                else
                                        mh_dyn_link='ld -Bshareable -o conftest.so.1.0 conftest.o 1>&AC_FD_CC'
#                                       mh_dyn_link='${CC} -Wl,-Bshareable -o conftest.so.1.0 conftest.o 1>&AC_FD_CC'
                                        if AC_TRY_EVAL(mh_dyn_link) && test -s conftest.so.1.0; then
                                                SHL_LD="ld -Bshareable -o ${SHLPRE}${SHLFILE}${SHLPST} "'$('SHOFILES')'
#                                               SHL_LD="${CC} -Wl,-Bshareable -o ${SHLPRE}${SHLFILE}${SHLPST} "'$('SHOFILES')'
                                        else
                                                mh_dyn_link='ld -assert pure-text -o conftest.so.1.0 conftest.o 1>&AC_FD_CC'
#                                               mh_dyn_link='${CC} -Wl,-assert pure-text -o conftest.so.1.0 conftest.o 1>&AC_FD_CC'
                                                if AC_TRY_EVAL(mh_dyn_link) && test -s conftest.so.1.0; then
                                                        SHL_LD="ld -assert pure-text -o ${SHLPRE}${SHLFILE}${SHLPST} "'$('SHOFILES')'
#                                                       SHL_LD="${CC} -Wl,-assert pure-text -o ${SHLPRE}${SHLFILE}${SHLPST} "'$('SHOFILES')'
                                                else
                                                        SHL_LD=""
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
dnl Check for presence of various libraries
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
dnl Check for presence of various libraries
dnl ---------------------------------------------------------------------------
AC_DEFUN([MH_CHECK_CRYPT],
[
mh_save_libs="$LIBS"

if test "$on_qnx4" = no; then
   AC_CHECK_LIB(crypt,main,mh_crypt_found=yes,mh_crypt_found=no)
   if test "$mh_crypt_found" = yes; then
      LIBS="${LIBS} -lcrypt"
   fi
fi

AC_MSG_CHECKING(for crypt function)

AC_CACHE_VAL(
[mh_cv_func_crypt],
[
        AC_TRY_LINK(
[#include <unistd.h>],
[crypt("fred","aa")],
        [mh_cv_func_crypt=yes],
        [mh_cv_func_crypt=no]
        )
])dnl
AC_MSG_RESULT($mh_cv_func_crypt)
STATIC_BINARY_LIBS=""
if test "$mh_cv_func_crypt" = yes ; then
        AC_DEFINE(HAVE_CRYPT)
        if test "$mh_crypt_found" = yes; then
                MH_EXTRA_LIBS="${MH_EXTRA_LIBS} -lcrypt"
                STATIC_BINARY_LIBS="-lcrypt"
        fi
fi

LIBS="$mh_save_libs"
AC_SUBST(STATIC_BINARY_LIBS)
])dnl

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
                SHLPST=""
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
# MH                MODPST=".so"
                ;;
esac
AC_MSG_RESULT($SHLPST/$MODPST)
])

dnl ---------------------------------------------------------------------------
dnl Sets SHLPST and MODPST
dnl ---------------------------------------------------------------------------
AC_DEFUN([MH_SET_SHLPST],
[
AC_SUBST(SHLPST)
AC_SUBST(MODPST)
])

dnl ---------------------------------------------------------------------------
dnl Determine how to build shared libraries etc..
dnl ---------------------------------------------------------------------------
AC_DEFUN([MH_SHARED_LIBRARY],
[
AC_REQUIRE([MH_SET_SHLPST])
dnl
dnl If compiler is gcc, then flags should be the same for all platforms
dnl (just guessing on this)
dnl
SHLPRE="lib"
LIBFILE="regina"
SHLFILE="regina"
OSAVE=".o.save"
OBJ="o"
LIBPST=".a"
LIBPRE="lib"
LIBEXE="ar"
GETOPT=""
OBJECTS='$('OFILES')'
STATIC_LDFLAGS=""
AIX_DYN="no"
BEOS_DYN="no"

AC_REQUIRE([AC_CANONICAL_SYSTEM])
case "$target" in
        *hp-hpux9*)
                SYS_DEFS="-D_HPUX_SOURCE +e" # Extended ANSI mode for c89
                AC_DEFINE(SELECT_IN_TIME_H)
                ;;
        *hp-hpux*)
               if test "$ac_cv_prog_CC" = "gcc" -o "$ac_cv_prog_CC" = "g++" -o "$ac_cv_prog_CC" = "clang"; then
                  SYS_DEFS="-D_HPUX_SOURCE"
                else
                  SYS_DEFS="-D_HPUX_SOURCE +e" # Extended ANSI mode for c89
                fi
                ;;
        *ibm-aix3*|*ibm-aix4.1*)
                SYS_DEFS="-D_ALL_SOURCE -DAIX"
                ;;
        *ibm-aix*)
                SYS_DEFS="-D_ALL_SOURCE -DAIX"
                DYN_COMP="-DDYNAMIC"
                ;;
        *dec-osf*)
               if test "$ac_cv_prog_CC" = "gcc" -o "$ac_cv_prog_CC" = "g++" -o "$ac_cv_prog_CC" = "clang"; then
                        SYS_DEFS="-D_POSIX_SOURCE -D_XOPEN_SOURCE"
                else
                        SYS_DEFS="-D_POSIX_SOURCE -D_XOPEN_SOURCE -Olimit 800"
                fi
                ;;
        sparc*sunos*)
                SYS_DEFS="-DSUNOS -DSUNOS_STRTOD_BUG"
                ;;
        *nto-qnx*)
                ;;
        *skyos*)
                DYN_COMP="-DDYNAMIC"
                ;;
        *qnx*)
                LIBPRE=""
                LIBPST=".lib"
                SHLPRE=""
                SHLPST=".lib"
                SHLFILE="rexx"
                ;;
        *cygwin*)
                LIBPRE="lib"
                SHLPRE="cyg"
                DYN_COMP="-DDYNAMIC"
                LIBPST=".a"
                ;;
        *darwin*)
                DYN_COMP="-fno-common"
                ;;
esac

dnl
dnl set variables for compiling if we don't already have it
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
        CFLAGS="$save_cflags"
        rm -f conftest.*
fi

dnl
dnl set variables for linking
dnl
SHL_LD=""
TEST1EXPORTS=""
TEST2EXPORTS=""
TEST1EXP=""
TEST2EXP=""
REGINAEXP=""
REGUTILEXP=""

# If the build OS can handle version numbers in the shared library name,
# then set SHL_BASE="${SHLPRE}${SHLFILE}${SHLPST}.\$(ABI)"
SHL_BASE="${SHLPRE}${SHLFILE}${SHLPST}"

# OTHER_INSTALLS is set to install a non-version numbered shared library
# by default; ie. no .\$(ABI) suffix. If the regina executable is not built,
# then there is no shared library. Set OTHER_INSTALLS="installabilib" if you
# are building a version numbered shared library.
RXSTACK_INSTALL="installrxstack"
OTHER_INSTALLS="installlib"
BASE_INSTALL="installbase"
BASE_BINARY="binarybase"
USE_ABI="no"
BUNDLE=""
EXTRATARGET=""
REGINA_PACKAGE_NAME="Regina-REXX"
case "$target" in
        *hp-hpux*)
                SHLPRE="lib"
                if test "$ac_cv_prog_CC" = "gcc"; then
                   LD_RXLIB_A1="${CC} -fPIC -shared ${LDFLAGS} -o \$(@)"
                   LD_RXLIB_A2="${CC} -fPIC -shared ${LDFLAGS} -o \$(@)"
                   LD_RXLIB_UTILA="${CC} -fPIC -shared ${LDFLAGS} -o \$(@)"
                   LD_RXLIB_B1="-L. -l${SHLFILE}"
                   LD_RXLIB_B2="-L. -l${SHLFILE}"
                   LD_RXLIB_UTILB="-L. -l${SHLFILE}"
                   SHL_LD="${CC} -o ${SHLPRE}${SHLFILE}${SHLPST} -fPIC -shared ${LDFLAGS} \$(SHOFILES)"
                else
                   EEXTRA="-Wl,-E"
                   LD_RXLIB_A1="ld -b -q -n -o \$(@)"
                   LD_RXLIB_A2="ld -b -q -n -o \$(@)"
                   LD_RXLIB_UTILA="ld -b -q -n -o \$(@)"
                   LD_RXLIB_B1=""
                   LD_RXLIB_B2=""
                   LD_RXLIB_UTILB=""
                   DYNAMIC_LDFLAGS="-Wl,+s"
                   SHL_LD="ld -b -o ${SHLPRE}${SHLFILE}${SHLPST} "'$('SHOFILES')'
                fi
                ;;
        *ibm-aix3*)
                STATIC_LDFLAGS="-bnso -bI:/lib/syscalls.exp"
                LD_RXLIB_A1="ld -bnoentry -bM:SRE -bT:512 -bH:512 -bI:regina.exp -o \$(@)"
                LD_RXLIB_A2="ld -bnoentry -bM:SRE -bT:512 -bH:512 -bI:regina.exp -o \$(@)"
                LD_RXLIB_UTILA="ld -bnoentry -bM:SRE -bT:512 -bH:512 -bI:regina.exp -o \$(@)"
                LD_RXLIB_B1="${SHLPRE}${SHLFILE}${SHLPST} -lc"
                LD_RXLIB_B2="${SHLPRE}${SHLFILE}${SHLPST} -lc"
                LD_RXLIB_UTILB="${SHLPRE}${SHLFILE}${SHLPST} -lc"
                SHLPRE="lib"
                TEST1EXPORTS="-bE:test1.exp"
                TEST2EXPORTS="-bE:test2.exp"
                TEST1EXP="test1.exp"
                TEST2EXP="test2.exp"
                REGINAEXP="regina.exp"
                REGUTILEXP="regutil.exp"
                if test "$ac_cv_header_dlfcn_h" = "yes" -o "$HAVE_DLFCN_H" = "1"; then
                        AIX_DYN="yes"
                        DYN_COMP="-DDYNAMIC"
                        SHL_LD="ld -o ${SHLPRE}${SHLFILE}${SHLPST} -bnoentry -bE:regina.exp -bM:SRE -bT:512 -bH:512 "'$('SHOFILES')'" -lc"
                else
                        SHL_LD=" "'$('LIBEXE')'" "'$('LIBFLAGS')'" "'$('SHOFILES')'
                        DYN_COMP=""
                fi
                ;;
        *ibm-aix*)
#                STATIC_LDFLAGS="-bnso -bI:/lib/syscalls.exp"
                SHLPRE="lib"
                if test "$ac_cv_prog_CC" = "gcc"; then
                   LD_RXLIB_A1="${CC} -shared ${LDFLAGS} -o \$(@)"
                   LD_RXLIB_A2="${CC} -shared ${LDFLAGS} -o \$(@)"
                   LD_RXLIB_UTILA="${CC} -shared ${LDFLAGS} -o \$(@)"
                   LD_RXLIB_B1="-L. -l${SHLFILE}"
                   LD_RXLIB_B2="-L. -l${SHLFILE}"
                   LD_RXLIB_UTILB="-L. -l${SHLFILE}"
                else
                   LD_RXLIB_A1="ld -bnoentry -bM:SRE ${LDFLAGS} -o \$(@)"
                   LD_RXLIB_A2="ld -bnoentry -bM:SRE ${LDFLAGS} -o \$(@)"
                   LD_RXLIB_UTILA="ld -bnoentry -bM:SRE ${LDFLAGS} -o \$(@)"
                   LD_RXLIB_B1="${SHLPRE}${SHLFILE}${SHLPST} -lc"
                   LD_RXLIB_B2="${SHLPRE}${SHLFILE}${SHLPST} -lc"
                   LD_RXLIB_UTILB="${SHLPRE}${SHLFILE}${SHLPST} -lc"
                   TEST1EXPORTS="-bE:test1.exp"
                   TEST2EXPORTS="-bE:test2.exp"
                   TEST1EXP="test1.exp"
                   TEST2EXP="test2.exp"
                   REGUTILEXPORTS="-bE:regutil.exp"
                   REGUTILEXP="regutil.exp"
                fi
                REGINAEXP="regina.exp"
                if test "$ac_cv_header_dlfcn_h" = "yes" -o "$HAVE_DLFCN_H" = "1"; then
                   AIX_DYN="yes"
                   DYN_COMP="-DDYNAMIC"
                   if test "$ac_cv_prog_CC" = "gcc"; then
                      SHL_LD="${CC} -shared -o ${SHLPRE}${SHLFILE}${SHLPST} -Wl,-bnoentry -Wl,-bE:regina.exp -Wl,-bM:SRE ${LDFLAGS} \$(SHOFILES) -lc \$(SHLIBS) \$(MH_MT_LIBS)"
                   else
                      SHL_LD="ld -o ${SHLPRE}${SHLFILE}${SHLPST} -bnoentry -bE:regina.exp -bM:SRE ${LDFLAGS} \$(SHOFILES) -lc \$(SHLIBS) \$(MH_MT_LIBS)"
                   fi
                else
                   SHL_LD=" "'$('LIBEXE')'" "'$('LIBFLAGS')'" "'$('SHOFILES')'
                   DYN_COMP=""
                fi
                ;;
        *dec-osf*)
                LD_RXLIB_A1="ld -shared -o \$(@)"
                LD_RXLIB_A2="ld -shared -o \$(@)"
                LD_RXLIB_UTILA="ld -shared -o \$(@)"
                LD_RXLIB_B1="-lc -L. -l${SHLFILE}"
                LD_RXLIB_B2="-lc -L. -l${SHLFILE}"
                LD_RXLIB_UTILB="-lc -L. -l${SHLFILE}"
                SHLPRE="lib"
                SHL_LD="ld -o ${SHLPRE}${SHLFILE}${SHLPST} -shared -no_archive "'$('SHOFILES')'" -lc"
                ;;
        *esix*)
                LD_RXLIB_A1="ld -G -o \$(@)"
                LD_RXLIB_A2="ld -G -o \$(@)"
                LD_RXLIB_UTILA="ld -G -o \$(@)"
                LD_RXLIB_B1=""
                LD_RXLIB_B2=""
                LD_RXLIB_UTILB=""
                DYNAMIC_LDFLAGS=""
                SHLPRE="lib"
                SHL_LD="ld -G -o ${SHLPRE}${SHLFILE}${SHLPST} "'$('SHOFILES')'
                ;;
        *dgux*)
                LD_RXLIB_A1="ld -G -o \$(@)"
                LD_RXLIB_A2="ld -G -o \$(@)"
                LD_RXLIB_UTILA="ld -G -o \$(@)"
                LD_RXLIB_B1=""
                LD_RXLIB_B2=""
                LD_RXLIB_UTILB=""
                DYNAMIC_LDFLAGS=""
                SHLPRE="lib"
                SHL_LD="ld -G -o ${SHLPRE}${SHLFILE}${SHLPST} "'$('SHOFILES')'
                ;;
        *pc-sco*)
                LD_RXLIB_A1="ld -dy -G -o \$(@)"
                LD_RXLIB_A2="ld -dy -G -o \$(@)"
                LD_RXLIB_UTILA="ld -dy -G -o \$(@)"
                LD_RXLIB_B1=""
                LD_RXLIB_B2=""
                LD_RXLIB_UTILB=""
                DYNAMIC_LDFLAGS=""
                SHLPRE="lib"
                SHL_LD="ld -dy -G -o ${SHLPRE}${SHLFILE}${SHLPST} "'$('SHOFILES')'
                ;;
        *solaris*)
               if test "$ac_cv_prog_CC" = "gcc" -o "$ac_cv_prog_CC" = "g++" -o "$ac_cv_prog_CC" = "clang"; then
                   LD_RXLIB_A1="$ac_cv_prog_CC -shared ${LDFLAGS} -o \$(@)"
                   LD_RXLIB_A2="$ac_cv_prog_CC -shared ${LDFLAGS} -o \$(@)"
                   LD_RXLIB_UTILA="$ac_cv_prog_CC -shared ${LDFLAGS} -o \$(@)"
                   SHL_LD="$ac_cv_prog_CC -shared ${LDFLAGS} -o ${SHLPRE}${SHLFILE}${SHLPST} "'$('SHOFILES')'
                else
                   LD_RXLIB_A1="ld -G ${LDFLAGS} -o \$(@)"
                   LD_RXLIB_A2="ld -G ${LDFLAGS} -o \$(@)"
                   LD_RXLIB_UTILA="ld -G ${LDFLAGS} -o \$(@)"
                   SHL_LD="ld -G ${LDFLAGS} -o ${SHLPRE}${SHLFILE}${SHLPST} "'$('SHOFILES')'
                fi
                LD_RXLIB_B1=""
                LD_RXLIB_B2=""
                LD_RXLIB_UTILB=""
                DYNAMIC_LDFLAGS=""
                SHLPRE="lib"
                ;;
        sparc*sunos*)
                LD_RXLIB_A1="ld -o \$(@)"
                LD_RXLIB_A2="ld -o \$(@)"
                LD_RXLIB_UTILA="ld -o \$(@)"
                LD_RXLIB_B1=""
                LD_RXLIB_B2=""
                LD_RXLIB_UTILB=""
                SHLPRE="lib"
                SHL_LD="ld -assert pure-text -o ${SHLPRE}${SHLFILE}${SHLPST} "'$('SHOFILES')'
                ;;
        *-freebsd* | *openbsd*)
                if test "$ac_cv_prog_CC" = "gcc" -o "$ac_cv_prog_CC" = "g++" -o "$ac_cv_prog_CC" = "clang"; then
                   LD_RXLIB_A1="$ac_cv_prog_CC -shared ${LDFLAGS} -o \$(@)"
                   LD_RXLIB_A2="$ac_cv_prog_CC -shared ${LDFLAGS} -o \$(@)"
                   LD_RXLIB_UTILA="$ac_cv_prog_CC -shared ${LDFLAGS} -o \$(@)"
                   SHL_LD="$ac_cv_prog_CC -shared ${LDFLAGS} -o ${SHLPRE}${SHLFILE}${SHLPST} "'$('SHOFILES')'
                else
                   LD_RXLIB_A1="ld -Bdynamic -Bshareable ${LDFLAGS} -o \$(@)"
                   LD_RXLIB_A2="ld -Bdynamic -Bshareable ${LDFLAGS} -o \$(@)"
                   LD_RXLIB_UTILA="ld -Bdynamic -Bshareable ${LDFLAGS} -o \$(@)"
                   LD_RXLIB_B1="-lc -L. -l${SHLFILE}"
                   LD_RXLIB_B2="-lc -L. -l${SHLFILE}"
                   LD_RXLIB_UTILB="-lc -L. -l${SHLFILE}"
                   SHL_LD="ld -Bdynamic -Bshareable ${LDFLAGS} -o ${SHLPRE}${SHLFILE}${SHLPST} "'$('SHOFILES')'
                fi
                STATIC_LDFLAGS="-static"
                SHLPRE="lib"
                ;;
        *linux*|*kfreebsd*-gnu*)
                LD_RXLIB_A1="${CC} -shared ${LDFLAGS} -o \$(@)"
                LD_RXLIB_A2="${CC} -shared ${LDFLAGS} -o \$(@)"
                LD_RXLIB_UTILA="${CC} -shared ${LDFLAGS} -o \$(@)"
#                LD_RXLIB_B1="-L. -l${SHLFILE}"
#                LD_RXLIB_B2="-L. -l${SHLFILE}"
                LD_RXLIB_B1="${SHLPRE}${SHLFILE}${SHLPST}.\$(ABI)"
                LD_RXLIB_B2="${SHLPRE}${SHLFILE}${SHLPST}.\$(ABI)"
                LD_RXLIB_UTILB="${SHLPRE}${SHLFILE}${SHLPST}.\$(ABI)"
                SHLPRE="lib"
                SHL_LD="${CC} -o ${SHLPRE}${SHLFILE}${SHLPST}.\$(ABI) -shared ${LDFLAGS} ${SHL_SCRIPT} -Wl,-soname=${SHLPRE}${SHLFILE}${SHLPST}.\$(ABI_MAJOR) \$(SHOFILES)"
                SHL_BASE="${SHLPRE}${SHLFILE}${SHLPST}.\$(ABI)"
                OTHER_INSTALLS="installabilib"
                USE_ABI="yes"
                ;;
        *gnu*)
                LD_RXLIB_A1="${CC} -shared -o \$(@)"
                LD_RXLIB_A2="${CC} -shared -o \$(@)"
                LD_RXLIB_UTILA="${CC} -shared -o \$(@)"
                LD_RXLIB_B1="${SHLPRE}${SHLFILE}${SHLPST}.\$(ABI)"
                LD_RXLIB_B2="${SHLPRE}${SHLFILE}${SHLPST}.\$(ABI)"
                LD_RXLIB_UTILB="${SHLPRE}${SHLFILE}${SHLPST}.\$(ABI)"
                SHLPRE="lib"
                SHL_LD="${CC} -o ${SHLPRE}${SHLFILE}${SHLPST}.\$(ABI) -shared ${SHL_SCRIPT} -Wl,-soname=${SHLPRE}${SHLFILE}${SHLPST}.\$(ABI_MAJOR) \$(SHOFILES) -lc"
                SHL_BASE="${SHLPRE}${SHLFILE}${SHLPST}.\$(ABI)"
                OTHER_INSTALLS="installabilib"
                USE_ABI="yes"
                ;;
        *atheos* | *syllable*)
                LD_RXLIB_A1="${CC} -shared ${LDFLAGS} -o \$(@)"
                LD_RXLIB_A2="${CC} -shared ${LDFLAGS} -o \$(@)"
                LD_RXLIB_UTILA="${CC} -shared ${LDFLAGS} -o \$(@)"
                LD_RXLIB_B1="-L. -l${SHLFILE} "'$('BOTHLIBS')'
                LD_RXLIB_B2="-L. -l${SHLFILE} "'$('BOTHLIBS')'
                LD_RXLIB_UTILB="-L. -l${SHLFILE} "'$('BOTHLIBS')'
                SHLPRE="lib"
                SHL_LD="${CC} -o ${SHLPRE}${SHLFILE}${SHLPST} -shared ${LDFLAGS} \$(SHOFILES) "'$('BOTHLIBS')'
                SHL_BASE="${LIBPRE}${SHLFILE}${SHLPST}"
                BASE_BINARY="atheosbinary"
                ;;
        *beos* | *haiku*)
                LD_RXLIB_A1="${CC} -Wl,-shared -nostart -Xlinker -o\$(@)"
                LD_RXLIB_A2="${CC} -Wl,-shared -nostart -Xlinker -o\$(@)"
                LD_RXLIB_UTILA="${CC} -Wl,-shared -nostart -Xlinker -o\$(@)"
                LD_RXLIB_B1="-L. -l${SHLFILE}"
                LD_RXLIB_B2="-L. -l${SHLFILE}"
                LD_RXLIB_UTILB="-L. -l${SHLFILE}"
                SHLPRE="lib"
                SHL_LD="${CC} -o ${SHLPRE}${SHLFILE}${SHLPST} -Wl,-shared -nostart -Xlinker \$(SHOFILES)"
                SHL_BASE="${SHLPRE}${SHLFILE}${SHLPST}"
                BEOS_DYN="yes"
                BASE_INSTALL="beosinstall"
                BASE_BINARY="beosbinary"
                OTHER_INSTALLS=""
                ;;
        *nto-qnx*)
                LD_RXLIB_A1="${CC} -shared -o \$(@)"
                LD_RXLIB_A2="${CC} -shared -o \$(@)"
                LD_RXLIB_UTILA="${CC} -shared -o \$(@)"
                LD_RXLIB_B1="-L. -l${SHLFILE}"
                LD_RXLIB_B2="-L. -l${SHLFILE}"
                LD_RXLIB_UTILB="-L. -l${SHLFILE}"
                SHLPRE="lib"
                SHL_LD="${CC} -o ${SHLPRE}${SHLFILE}${SHLPST} -shared \$(SHOFILES)"
                SHL_BASE="${SHLPRE}${SHLFILE}${SHLPST}"
                ;;
        *skyos*)
                LD_RXLIB_A1="${CC} -shared -o \$(@)"
                LD_RXLIB_A2="${CC} -shared -o \$(@)"
                LD_RXLIB_UTILA="${CC} -shared -o \$(@)"
                LD_RXLIB_B1="-L. -l${SHLFILE}"
                LD_RXLIB_B2="-L. -l${SHLFILE}"
                LD_RXLIB_UTILB="-L. -l${SHLFILE}"
                SHLPRE="lib"
                SHL_LD="${CC} -o ${SHLPRE}${SHLFILE}${SHLPST} -shared \$(SHOFILES)"
                SHL_BASE="${SHLPRE}${SHLFILE}${SHLPST}"
                ;;
        *qnx*)
                LD_RXLIB_A1=""
                LD_RXLIB_A2=""
                LD_RXLIB_UTILA=""
                LD_RXLIB_B1=""
                LD_RXLIB_B2=""
                LD_RXLIB_UTILB=""
                SHLPRE=""
                # set BUNDLE to unique junk names to stop the Makefile
                # getting duplicate targets
                BUNDLE=".junk2"
                SHLPST=".junk"
                SHL_LD=""
                EEXTRA="-mf -N0x20000 -Q"
                LIBPRE=""
                LIBPST=".lib"
                LIBFILE="rexx"
                ;;
        *cygwin*)
                LD_RXLIB_A1="${CC} -shared -o \$(@)"
                LD_RXLIB_A2="${CC} -shared -o \$(@)"
                LD_RXLIB_UTILA="${CC} -shared -o \$(@)"
                LD_RXLIB_B1="-L. -lcygregina"
                LD_RXLIB_B2="-L. -lcygregina"
                LD_RXLIB_UTILB="-L. -lcygregina"
                SHLPRE="cyg"
                SHL_LD="${CC} -shared -o \$(@) -o cygregina.dll \$(SHOFILES)"
                EEXTRA=""
                LIBPRE="lib"
                LIBPST=".a"
                LIBFILE="rexx"
                SHL_BASE="${SHLPRE}${SHLFILE}${SHLPST}"
                TEST1EXPORTS=""
                TEST2EXPORTS=""
                REGUTILEXPORTS=""
                TEST1EXP="${srcdir}/test1_w32_dll.def"
                TEST2EXP="${srcdir}/test2_w32_dll.def"
                REGINAEXP="${srcdir}/regina_w32_dll.def"
                REGUTILEXP="${srcdir}/regutil/regutil.def"
                BASE_INSTALL="cygwininstall"
                OTHER_INSTALLS=""
                RXSTACK_INSTALL=""
                REGINA_PACKAGE_NAME="regina-rexx"
                ;;
        *apple-darwin*)
                # to test on platform other than real Mac OSX use: --build=ppc-apple-darwin10.1 --target=ppc-apple-darwin10.1
                #
                # Link switches for building "bundles"
                #
# MH                LD_RXLIB_A1="${CC} -bundle -flat_namespace -undefined suppress -o \$(@)"
# MH                LD_RXLIB_A2="${CC} -bundle -flat_namespace -undefined suppress -o \$(@)"
                LD_RXLIB_A1="${CC} ${EEXTRA} -dynamiclib -headerpad_max_install_names ${LDFLAGS} -o \$(@)"
                LD_RXLIB_A2="${CC} ${EEXTRA} -dynamiclib -headerpad_max_install_names ${LDFLAGS} -o \$(@)"
                LD_RXLIB_UTILA="${CC} ${EEXTRA} -dynamiclib -headerpad_max_install_names ${LDFLAGS} -o \$(@)"
                LD_RXLIB_B1="-L. -l${SHLFILE} -lc \$(SHLIBS)"
                LD_RXLIB_B2="-L. -l${SHLFILE} -lc \$(SHLIBS)"
                LD_RXLIB_UTILB="-L. -l${SHLFILE} -lc \$(SHLIBS) $REGUTIL_TERM_LIB"
                DYN_COMP="-DDYNAMIC -fno-common"
                SHLPRE="lib"
                BUNDLE=".junk"
                SHL_LD="${CC} ${EEXTRA} -dynamiclib ${LDFLAGS} -headerpad_max_install_names -o ${SHLPRE}${SHLFILE}${SHLPST} "'$('SHOFILES')'
                SHL_BASE="${LIBPRE}${SHLFILE}${SHLPST}"
                OTHER_INSTALLS="installmaclib"
                ;;
        *)
                MH_HOWTO_SHARED_LIBRARY()
                MH_HOWTO_DYN_LINK()
                ;;
esac

LIBFLAGS="cr ${LIBPRE}${LIBFILE}${LIBPST}"
LIBLINK="-L. -l${LIBFILE}"
LINKSHL="-L. -l${SHLFILE}"

case "$target" in
        *hp-hpux*)
                #
                # For HPUX, we must link the "regina" executable with -lregina
                # otherwise you can't move the shared library anywhere other
                # than where it was built. Fix by PJM
                #
                LINKREG="${LINKSHL}"
                ;;
        *cygwin*)
                #
                # For CygWin, we must link with the import library
                # not the actual DLL
                #
                LINKREG="${LINKSHL}"
                ;;
        *skyos*)
                #
                # SkyOS requires linking with -L. -lregina format
                #
                LINKREG="${LINKSHL}"
                ;;
        *linux*|*kfreebsd*-gnu*)
                LINKSHL="${SHLPRE}${SHLFILE}${SHLPST}.\$(ABI)"
                LINKREG="${LINKSHL}"
                ;;
        *)
                LINKREG="${SHLPRE}${SHLFILE}${SHLPST}"
                ;;
esac

if test "$SHL_LD" != ""; then
   SHL_LD="$SHL_LD \$(BOTHLIBS) \$(MH_MT_LIBS)"
fi

AC_MSG_CHECKING(if dynamic loading of external functions is supported)
if test "$HAVE_DLFCN_H" = "1" -o "$ac_cv_header_dl_h" = "yes" -o "$ac_cv_header_dlfcn_h" = "yes" -o "$AIX_DYN" = "yes" -o "$BEOS_DYN" = "yes"; then
   SHL_TARGETS="${SHL_BASE} regina${binarybitprefix}${EXEEXT} ${SHLPRE}regutil${MODPST} ${SHLPRE}rxtest1${MODPST} ${SHLPRE}rxtest2${MODPST} $EXTRATARGET"
   EXECISER_DEP="${SHL_BASE}"
   OTHER_INSTALLS="regina${binarybitprefix}${EXEEXT} $OTHER_INSTALLS"
   AC_MSG_RESULT("yes")
else
   SHL_TARGETS=""
   EXECISER_DEP="${STATICLIB}"
   OTHER_INSTALLS=""
   AC_MSG_RESULT("no")
fi
#
# Add $RXSTACK_INSTALL
#
OTHER_INSTALLS="$OTHER_INSTALLS $RXSTACK_INSTALL"

if test "$AIX_DYN" = "yes"; then
   STATICLIB=""
   LIBPST=".junk"
else
   STATICLIB="${LIBPRE}${LIBFILE}${LIBPST}"
fi

RANLIB_DYNAMIC=""
AC_SUBST(BASE_BINARY)
AC_SUBST(BASE_INSTALL)
AC_SUBST(BOTHLIBS)
AC_SUBST(CC2O)
AC_SUBST(CEXTRA)
AC_SUBST(DYNAMIC_LDFLAGS)
AC_SUBST(DYN_COMP)
AC_SUBST(EEXTRA)
AC_SUBST(EXECISER_DEP)
AC_SUBST(GETOPT)
AC_SUBST(LD_RXLIB_A1)
AC_SUBST(LD_RXLIB_A2)
AC_SUBST(LD_RXLIB_UTILA)
AC_SUBST(LD_RXLIB_B1)
AC_SUBST(LD_RXLIB_B2)
AC_SUBST(LD_RXLIB_UTILB)
AC_SUBST(LIBEXE)
AC_SUBST(LIBFILE)
AC_SUBST(LIBFLAGS)
AC_SUBST(LIBLINK)
AC_SUBST(LIBPRE)
AC_SUBST(LIBPST)
AC_SUBST(LINKSHL)
AC_SUBST(LINKREG)
AC_SUBST(O2SAVE)
AC_SUBST(O2TSAVE)
AC_SUBST(O2SHO)
AC_SUBST(O2TSO)
AC_SUBST(OBJ)
AC_SUBST(OBJECTS)
AC_SUBST(OSAVE)
AC_SUBST(OTHER_INSTALLS)
AC_SUBST(REGINAEXP)
AC_SUBST(SAVE2O)
AC_SUBST(SHLFILE)
AC_SUBST(SHLIBS)
AC_SUBST(SHLPRE)
AC_SUBST(BUNDLE)
AC_SUBST(SHL_LD)
AC_SUBST(SHL_TARGETS)
AC_SUBST(STATICLIB)
AC_SUBST(STATIC_LDFLAGS)
AC_SUBST(TEST1EXP)
AC_SUBST(TEST1EXPORTS)
AC_SUBST(TEST2EXP)
AC_SUBST(TEST2EXPORTS)
AC_SUBST(REGUTILEXP)
AC_SUBST(REGUTILEXPORTS)
AC_SUBST(USE_ABI)
AC_SUBST(RANLIB_DYNAMIC)
AC_SUBST(REGINA_PACKAGE_NAME)
])dnl

dnl ---------------------------------------------------------------------------
dnl Determine how to build shared libraries etc..
dnl ---------------------------------------------------------------------------
AC_DEFUN([MH_STATIC_LOADING],
[
SHLPRE="lib"
SHLPST=".a"
MODPST=".a"
LIBFILE="rexx"
SHLFILE="regina"
OSAVE=".o.save"
OBJ="o"
LIBPST=".a"
LIBPRE="lib"
LIBEXE="ar"
GETOPT=""
OBJECTS='$('OFILES')'
STATIC_LDFLAGS=""
AIX_DYN="no"
BEOS_DYN="no"
DYN_COMP="-DDYNAMIC"

dnl
dnl set variables for linking
dnl
SHL_LD=""
TEST1EXPORTS=""
TEST2EXPORTS=""
TEST1EXP=""
TEST2EXP=""
REGINAEXP=""
REGUTILEXPORTS=""
REGUTILEXP=""


# OTHER_INSTALLS is set to install a non-version numbered shared library
# by default; ie. no .\$(ABI) suffix. If the regina executable is not built,
# then there is no shared library. Set OTHER_INSTALLS="installabilib" if you
# are building a version numbered shared library.
OTHER_INSTALLS="installlib"
BASE_INSTALL="installbase"
BASE_BINARY="binarybase"
USE_ABI="no"
BUNDLE=""
EXTRATARGET=""
AC_REQUIRE([AC_CANONICAL_SYSTEM])
case "$target" in
        *hp-hpux9*)
                SYS_DEFS="-D_HPUX_SOURCE"
                AC_DEFINE(SELECT_IN_TIME_H)
                ;;
        *hp-hpux*)
                SYS_DEFS="-D_HPUX_SOURCE"
                ;;
        *ibm-aix*)
                SYS_DEFS="-D_ALL_SOURCE -DAIX"
#                STATIC_LDFLAGS="-bnso -bI:/lib/syscalls.exp"
                ;;
        *dec-osf*)
               if test "$ac_cv_prog_CC" = "gcc" -o "$ac_cv_prog_CC" = "g++" -o "$ac_cv_prog_CC" = "clang"; then
                   SYS_DEFS="-D_POSIX_SOURCE -D_XOPEN_SOURCE"
                else
                   SYS_DEFS="-D_POSIX_SOURCE -D_XOPEN_SOURCE -Olimit 800"
                fi
                ;;
        sparc*sunos*)
                SYS_DEFS="-DSUNOS -DSUNOS_STRTOD_BUG"
                ;;
        *atheos* | *syllable*)
                BASE_BINARY="atheosbinary"
                ;;
        *beos* | *haiku*)
                BASE_INSTALL="beosinstall"
                BASE_BINARY="beosbinary"
                OTHER_INSTALLS=""
                ;;
        *nto-qnx*)
                ;;
        *qnx*)
                SHLPRE=""
                SHLPST=".lib"
                MODPST=".lib"
                # getting duplicate targets
                BUNDLE=".junk2"
                EEXTRA="-mf -N0x20000 -Q"
                LIBPRE=""
                LIBPST=".lib"
                ;;
        *cygwin*)
                LIBPRE="lib"
                SHLPRE="cyg"
                LIBPST=".a"
                BASE_INSTALL="cygwininstall"
                BASE_BINARY="cygwinbinary"
                OTHER_INSTALLS=""
                ;;
        *apple-darwin*)
                # to test use: --build=ppc-apple-darwin10.1 --target=ppc-apple-darwin10.1
                #
                # Link switches for building "bundles"
                #
                OTHER_INSTALLS="installlib installmacosx"
                ;;
        *)
                ;;
esac
LD_RXLIB_A1="${LIBEXE} cr \$(@)"
LD_RXLIB_A2="${LIBEXE} cr \$(@)"
LD_RXLIB_UTILA="${LIBEXE} cr \$(@)"
LD_RXLIB_B1=""
LD_RXLIB_B2=""
LD_RXLIB_UTILB=""
SHL_LD="${LIBEXE} cr ${SHLPRE}${SHLFILE}${SHLPST} \$(SHOFILES)"

STATICLIB="${LIBPRE}${LIBFILE}${LIBPST}"

LIBFLAGS="cr ${LIBPRE}${LIBFILE}${LIBPST}"
LIBLINK="-L. -l${LIBFILE}"
LINKSHL="-L. -l${LIBFILE}" # ensure we link to static library for execiser
DYN_COMP="$DYN_COMP -DDYNAMIC_STATIC"

SHL_BASE="${SHLPRE}${SHLFILE}${SHLPST}"
SHL_TARGETS="${SHL_BASE}  ${SHLPRE}regutil${MODPST} regina${binarybitprefix}${EXEEXT} ${SHLPRE}rxtest1${MODPST} ${SHLPRE}rxtest2${MODPST} $EXTRATARGET"
EXECISER_DEP="${STATICLIB}"
OTHER_INSTALLS="regina${binarybitprefix}${EXEEXT} $OTHER_INSTALLS"
RANLIB_DYNAMIC="-\$(RANLIB) \$(@)"
dnl
dnl Settings for Regina test package
dnl
AC_ARG_WITH(testpackage,
    [     --with-testpackage   enable static linking with Regina test external function packages],
    [with_test=$withval],
    [with_testpackage=no],
)
MH_FUNC_LIBS=""
if test "$with_testpackage" = "yes"; then
   MH_FUNC_LIBS="$MH_FUNC_LIBS ${SHLPRE}rxtest1${MODPST} ${SHLPRE}rxtest2${MODPST}"
   AC_DEFINE(HAVE_TEST_PACKAGE)
   EXECISER_DEP="${EXECISER_DEP} ${SHLPRE}rxtest1${MODPST} ${SHLPRE}rxtest2${MODPST}"
fi
dnl
dnl Settings for Patrick McPhee's Regutil (now included with Regina)
dnl
AC_ARG_WITH(regutil,
    [     --with-regutil       enable static linking with RegUtil external function package],
    [with_regutil=$withval],
    [with_regutil=no],
)
if test "$with_regutil" = "yes" -o "x$1" = "xregutil"; then
   MH_FUNC_LIBS="$MH_FUNC_LIBS ${SHLPRE}regutil${MODPST}"
   AC_DEFINE(HAVE_REXXUTIL_PACKAGE)
fi
dnl
dnl Settings for Rexx/Curses
dnl
AC_ARG_WITH(rexxcurses,
    [     --with-rexxcurses    enable static linking with Rexx/Curses external function package],
    [with_rexxcurses=$withval],
    [with_rexxcurses=no],
)
if test "$with_rexxcurses" = "yes"; then
        AC_CHECK_PROG(rxcurses_config, [rxcurses-config], yes, no)
        if test "$ac_cv_prog_rxcurses_config" = yes; then
           EXTRA_LIB=`rxcurses-config --libs-static`
           MH_FUNC_LIBS="$MH_FUNC_LIBS $EXTRA_LIB"
           AC_DEFINE(HAVE_REXXCURSES_PACKAGE)
        else
           AC_MSG_ERROR(rxcurses-config not found. Cannot use --with-rexxcurses switch)
        fi
fi
dnl
dnl Settings for Rexx/Tk
dnl
AC_ARG_WITH(rexxtk,
    [     --with-rexxtk        enable static linking with Rexx/Tk external function package],
    [with_rexxtk=$withval],
    [with_rexxtk=no],
)
if test "$with_rexxtk" = "yes"; then
        AC_CHECK_PROG(rexxtk_config, [rexxtk-config], yes, no)
        if test "$ac_cv_prog_rexxtk_config" = yes; then
           EXTRA_LIB=`rexxtk-config --libs-static`
           MH_FUNC_LIBS="$MH_FUNC_LIBS $EXTRA_LIB"
           AC_DEFINE(HAVE_REXXTK_PACKAGE)
        else
           AC_MSG_ERROR(rexxtk-config not found. Cannot use --with-rexxtk switch)
        fi
fi
dnl
dnl Settings for Rexx/Gd
dnl
AC_ARG_WITH(rexxgd,
    [     --with-rexxgd        enable static linking with Rexx/Gd external function package],
    [with_rexxgd=$withval],
    [with_rexxgd=no],
)
if test "$with_rexxgd" = "yes"; then
        AC_CHECK_PROG(rexxgd_config, [rexxgd-config], yes, no)
        if test "$ac_cv_prog_rexxgd_config" = yes; then
           EXTRA_LIB=`rexxgd-config --libs-static`
           MH_FUNC_LIBS="$MH_FUNC_LIBS $EXTRA_LIB"
           AC_DEFINE(HAVE_REXXGD_PACKAGE)
        else
           AC_MSG_ERROR(rexxgd-config not found. Cannot use --with-rexxgd switch)
        fi
fi
dnl
dnl Settings for Rexx/CURL
dnl
AC_ARG_WITH(rexxcurl,
    [     --with-rexxcurl      enable static linking with Rexx/CURL external function package],
    [with_rexxcurl=$withval],
    [with_rexxcurl=no],
)
if test "$with_rexxcurl" = "yes"; then
        AC_CHECK_PROG(rexxcurl_config, [rexxcurl-config], yes, no)
        if test "$ac_cv_prog_rexxcurl_config" = yes; then
           EXTRA_LIB=`rexxcurl-config --libs-static`
           MH_FUNC_LIBS="$MH_FUNC_LIBS $EXTRA_LIB"
           AC_DEFINE(HAVE_REXXCURL_PACKAGE)
        else
           AC_MSG_ERROR(rexxcurl-config not found. Cannot use --with-rexxcurl switch)
        fi
fi
dnl
dnl Settings for Rexx/SQL
dnl
AC_ARG_WITH(rexxsql,
    [     --with-rexxsql       enable static linking with Rexx/SQL external function package],
    [with_rexxsql=$withval],
    [with_rexxsql=no],
)
if test "$with_rexxsql" = "yes"; then
        AC_CHECK_PROG(rexxsql_config, [rexxsql-config], yes, no)
        if test "$ac_cv_prog_rexxsql_config" = yes; then
           EXTRA_LIB=`rexxsql-config --libs-static`
           MH_FUNC_LIBS="$MH_FUNC_LIBS $EXTRA_LIB"
           AC_DEFINE(HAVE_REXXSQL_PACKAGE)
        else
           AC_MSG_ERROR(rexxsql-config not found. Cannot use --with-rexxsql switch)
        fi
fi
dnl
dnl Settings for Rexx/EEC
dnl
AC_ARG_WITH(rexxeec,
    [     --with-rexxeec       enable static linking with Rexx/EEC external function package],
    [with_rexxeec=$withval],
    [with_rexxeec=no],
)
if test "$with_rexxeec" = "yes"; then
        AC_CHECK_PROG(rexxeec_config, [rexxeec-config], yes, no)
        if test "$ac_cv_prog_rexxeec_config" = yes; then
           EXTRA_LIB=`rexxeec-config --libs-static`
           MH_FUNC_LIBS="$MH_FUNC_LIBS $EXTRA_LIB"
           AC_DEFINE(HAVE_REXXEEC_PACKAGE)
        else
           AC_MSG_ERROR(rexxeec-config not found. Cannot use --with-rexxeec switch)
        fi
fi
dnl
dnl Settings for Rexx/ISAM
dnl
AC_ARG_WITH(rexxisam,
    [     --with-rexxisam      enable static linking with Rexx/ISAM external function package],
    [with_rexxisam=$withval],
    [with_rexxisam=no],
)
if test "$with_rexxisam" = "yes"; then
        AC_CHECK_PROG(rexxisam_config, [rexxisam-config], yes, no)
        if test "$ac_cv_prog_rexxisam_config" = yes; then
           EXTRA_LIB=`rexxisam-config --libs-static`
           MH_FUNC_LIBS="$MH_FUNC_LIBS $EXTRA_LIB"
           AC_DEFINE(HAVE_REXXISAM_PACKAGE)
        else
           AC_MSG_ERROR(rexxisam-config not found. Cannot use --with-rexxisam switch)
        fi
fi
dnl
dnl Settings for RxSock
dnl
AC_ARG_WITH(rxsock,
    [     --with-rxsock        enable static linking with RxSock external function package],
    [with_rxsock=$withval],
    [with_rxsock=no],
)
if test "$with_rxsock" = "yes"; then
        AC_CHECK_PROG(rxsock_config, [rxsock-config], yes, no)
        if test "$ac_cv_prog_rxsock_config" = yes; then
           EXTRA_LIB=`rxsock-config --libs-static`
           MH_FUNC_LIBS="$MH_FUNC_LIBS $EXTRA_LIB"
           AC_DEFINE(HAVE_RXSOCK_PACKAGE)
        else
           AC_MSG_ERROR(rxsock-config not found. Cannot use --with-rxsock switch)
        fi
fi
dnl
dnl Settings for Rexx/DW
dnl
AC_ARG_WITH(rexxdw,
    [     --with-rexxdw        enable static linking with Rexx/DW external function package],
    [with_rexxdw=$withval],
    [with_rexxdw=no],
)
if test "$with_rexxdw" = "yes"; then
        AC_CHECK_PROG(rexxdw_config, [rexxdw-config], yes, no)
        if test "$ac_cv_prog_rexxdw_config" = yes; then
           EXTRA_LIB=`rexxdw-config --libs-static`
           MH_FUNC_LIBS="$MH_FUNC_LIBS $EXTRA_LIB"
           AC_DEFINE(HAVE_REXXDW_PACKAGE)
        else
           AC_MSG_ERROR(rexxdw-config not found. Cannot use --with-rexxdw switch)
        fi
fi
dnl
dnl Settings for Rexx/CSV
dnl
AC_ARG_WITH(rexxcsv,
    [     --with-rexxcsv       enable static linking with Rexx/CSV external function package],
    [with_rexxcsv=$withval],
    [with_rexxcsv=no],
)
if test "$with_rexxcsv" = "yes"; then
        AC_CHECK_PROG(rexxcsv_config, [rexxcsv-config], yes, no)
        if test "$ac_cv_prog_rexxcsv_config" = yes; then
           EXTRA_LIB=`rexxcsv-config --libs-static`
           MH_FUNC_LIBS="$MH_FUNC_LIBS $EXTRA_LIB"
           AC_DEFINE(HAVE_REXXCSV_PACKAGE)
        else
           AC_MSG_ERROR(rexxcsv-config not found. Cannot use --with-rexxcsv switch)
        fi
fi
dnl
dnl Settings for Rexx/PDF
dnl
AC_ARG_WITH(rexxpdf,
    [     --with-rexxpdf       enable static linking with Rexx/PDF external function package],
    [with_rexxpdf=$withval],
    [with_rexxpdf=no],
)
if test "$with_rexxpdf" = "yes"; then
        AC_CHECK_PROG(rexxpdf_config, [rexxpdf-config], yes, no)
        if test "$ac_cv_prog_rexxpdf_config" = yes; then
           EXTRA_LIB=`rexxpdf-config --libs-static`
           MH_FUNC_LIBS="$MH_FUNC_LIBS $EXTRA_LIB"
           AC_DEFINE(HAVE_REXXPDF_PACKAGE)
        else
           AC_MSG_ERROR(rexxpdf-config not found. Cannot use --with-rexxpdf switch)
        fi
fi

LINKREG="${SHLPRE}${SHLFILE}${SHLPST} ${MH_FUNC_LIBS} ${SHLPRE}${SHLFILE}${SHLPST}"

AC_SUBST(MH_FUNC_LIBS)
AC_SUBST(BASE_BINARY)
AC_SUBST(BASE_INSTALL)
AC_SUBST(BOTHLIBS)
AC_SUBST(CC2O)
AC_SUBST(CEXTRA)
AC_SUBST(DYNAMIC_LDFLAGS)
AC_SUBST(DYN_COMP)
AC_SUBST(EEXTRA)
AC_SUBST(EXECISER_DEP)
AC_SUBST(GETOPT)
AC_SUBST(LD_RXLIB_A1)
AC_SUBST(LD_RXLIB_A2)
AC_SUBST(LD_RXLIB_UTILA)
AC_SUBST(LD_RXLIB_B1)
AC_SUBST(LD_RXLIB_B2)
AC_SUBST(LD_RXLIB_UTILB)
AC_SUBST(LIBEXE)
AC_SUBST(LIBFILE)
AC_SUBST(LIBFLAGS)
AC_SUBST(LIBLINK)
AC_SUBST(LINKSHL)
AC_SUBST(LIBPRE)
AC_SUBST(LIBPST)
AC_SUBST(LINKREG)
AC_SUBST(O2SAVE)
AC_SUBST(O2SHO)
AC_SUBST(OBJ)
AC_SUBST(OBJECTS)
AC_SUBST(OSAVE)
AC_SUBST(OTHER_INSTALLS)
AC_SUBST(REGINAEXP)
AC_SUBST(SAVE2O)
AC_SUBST(SHLFILE)
AC_SUBST(SHLIBS)
AC_SUBST(SHLPRE)
AC_SUBST(BUNDLE)
AC_SUBST(SHL_LD)
AC_SUBST(SHL_TARGETS)
AC_SUBST(STATICLIB)
AC_SUBST(STATIC_LDFLAGS)
AC_SUBST(TEST1EXP)
AC_SUBST(TEST1EXPORTS)
AC_SUBST(TEST2EXP)
AC_SUBST(TEST2EXPORTS)
AC_SUBST(REGUTILEXP)
AC_SUBST(REGUTILEXPORTS)
AC_SUBST(USE_ABI)
AC_SUBST(RANLIB_DYNAMIC)

])

dnl ---------------------------------------------------------------------------
dnl Determines where rpm build files are located
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
dnl Determines if socklen_t is valid
dnl ---------------------------------------------------------------------------
AC_DEFUN([MH_CHECK_TYPE_SOCKLEN_T],
[AC_CACHE_CHECK([for socklen_t], ac_cv_type_socklen_t,
[
  AC_TRY_COMPILE(
  [#include <sys/types.h>
   #include <sys/socket.h>],
  [socklen_t len = 42; return 0;],
  ac_cv_type_socklen_t=yes,
  ac_cv_type_socklen_t=no)
])
  if test $ac_cv_type_socklen_t = yes; then
    AC_DEFINE(HAVE_SOCKLEN_T, 1, [compiler has socklen_t])
  fi
])

dnl ---------------------------------------------------------------------------
dnl Determines is struct statvfs contains f_mntfromname member
dnl ---------------------------------------------------------------------------
AC_DEFUN([MH_CHECK_F_MNTFROMNAME],
[AC_CHECKING(if statvfs.f_mntfromname exists)
AC_TRY_COMPILE([
  #include <sys/types.h>
  #include <sys/statvfs.h>
], [
  struct statvfs buf;
  char *p = buf.f_mntfromname;

  statvfs(".", &buf);
], [
  AC_DEFINE(HAVE_STATVFS_F_MNTFROMNAME,, Define if you have statvfs.f_mntfromname)
  AC_MSG_RESULT(yes)
], [
  AC_MSG_RESULT(no)
])
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

dnl ---------------------------------------------------------------------------
dnl Determines which -arch flags valid on Mac OSX with gcc
dnl ---------------------------------------------------------------------------
AC_DEFUN([MH_CHECK_OSX_ARCH],
[
valid_arch_flags=""
found_arch_flags=""
AC_ARG_WITH(arch,
   [  --with-arch=option      build universal binaries on OS X, option: one of "all 32bit 64bit intel ppc ppc64 x86_64 i386"],
   [with_arch=$withval],
   [with_arch=none],
)
case "$with_arch" in
   none)
      arch_flags=""
      ;;
   all)
      arch_flags="-arch ppc -arch ppc64 -arch x86_64 -arch i386"
      ;;
   32bit)
      arch_flags="-arch ppc -arch i386"
      ;;
   64bit)
      arch_flags="-arch ppc64 -arch x86_64"
      ;;
   intel)
      arch_flags="-arch i386 -arch x86_64"
      ;;
   *)
      arch_flags="-arch $with_arch"
      ;;
esac

valid_arch_flags=""
found_arch_flags=""
if test $on_osx = "yes"; then
  AC_MSG_CHECKING(for which Mac OSX -arch flags are supported for universal binaries)
  for a in $arch_flags; do
    save_ldflags="$LDFLAGS"
    LDFLAGS="$LDFLAGS -arch $a"
    AC_LINK_IFELSE(
    [AC_LANG_PROGRAM([#include <stdio.h>],
    [exit(0)])],
    [valid_arch_flags="$valid_arch_flags -arch $a";found_arch_flags="$found_arch_flags $a"],
    [a="$a"]
    )
    LDFLAGS="$save_ldflags"
  done
  AC_MSG_RESULT($found_arch_flags)
  AC_SUBST(valid_arch_flags)
fi
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
      *)
         MYDISTRO="$target"
      ;;
   esac
fi
AC_SUBST(MYDISTRO)
])

dnl
dnl add our expansion macro for addonsdir
dnl
sinclude(common/ac_define_dir.m4)
