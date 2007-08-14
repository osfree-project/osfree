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
dnl MH_GET_SHLPST
dnl MH_SET_SHLPST
dnl MH_SHARED_LIBRARY
dnl MH_STATIC_LOADING
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
all_words="$CC_LIST"
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
[echo 'foo(){}' > conftest.c
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
        O2SHO="-mv \`basename "$'@'" .sho\`.o "$'@'
        O2SAVE="-mv \`basename "$'@'" .sho\`.o \`basename "$'@'" .sho\`.o.save"
        SAVE2O="-mv \`basename "$'@'" .sho\`.o.save \`basename "$'@'" .sho\`.o"
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
AC_TRY_LINK(
[#include <stdio.h>],
[exit(0)],
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
dnl Check if gcc compiler supports --version-script
dnl ---------------------------------------------------------------------------
AC_DEFUN([MH_CHECK_GCC_VERSION_SCRIPT],
[
AC_MSG_CHECKING(whether gcc understands --version-script)
AC_CACHE_VAL(
[mh_version_script],
[
echo "TEST {" > conftest.def
echo "global:" >> conftest.def
echo "Test;" >> conftest.def
echo "};" >> conftest.def
mh_save_libs="$LIBS"
LIBS="-Wl,--version-script=conftest.def"
AC_TRY_LINK(
[#include <stdio.h>],
[exit(0)],
[mh_version_script=yes],
[mh_version_script=no]
)
LIBS="$mh_save_libs"
rm -f conftest.def
])
if test "$mh_version_script" = yes; then
	SHL_SCRIPT="-Wl,--version-script=${srcdir}/regina_elf.def"
else
	SHL_SCRIPT=""
fi
AC_SUBST(SHL_SCRIPT)
AC_MSG_RESULT($mh_version_script)
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
])dnl
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
   AC_CHECK_LIB("rt",main,mh_rt_lib_found=yes,mh_rt_lib_found=no)
   AC_MSG_CHECKING(whether Posix Threads are supported)
   tmpLIBS=$LIBS
   save_cflags="$CFLAGS"
   pthread_libs="pthreads pthread c_r"
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
         THREADSAFE_TARGET="threader"
         MT_FILE="mt_posix"
         case "$target" in
            *solaris*)
               if test "$ac_cv_prog_CC" = "gcc"; then
                  THREADING_COMPILE="-D_REENTRANT -DPOSIX"
                  if test "$mh_rt_lib_found" = "yes"; then 
                     THREADING_LINK="-lrt"
                  fi
               else
                  THREADING_COMPILE="-mt -D_REENTRANT -DREENTRANT -D_PTHREAD_SEMANTICS"
                  THREADING_LINK="-mt"
               fi
               ;;
            *linux*)
               ;;
            *freebsd*)
               THREADING_COMPILE="-pthread -D_REENTRANT -DPOSIX"
               THREADING_LINK="-pthread"
               ;;
            *cygwin*)
               THREADING_COMPILE="-D_REENTRANT -DPOSIX"
               THREADING_LINK=""
               THREADSAFE_TARGET="threader.exe"
               ;;
            *)
               if test "$ac_cv_prog_CC" = "gcc"; then
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
   AC_TRY_RUN_NATIVE([
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
   mh_cv_uscore=yes,
   mh_cv_uscore=no
   )
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
#               LD_RXLIB1="${CC} -Wl,-shared"
                LD_RXLIB_B1="-L. -l${SHLFILE}"
                LD_RXLIB_B2="-L. -l${SHLFILE}"
                SHLPRE="lib"
                SHLPST=".so"
        else
                mh_dyn_link='ld -G -o conftest.rxlib conftest.o 1>&AC_FD_CC'
#               mh_dyn_link='${CC} -Wl,-G -o conftest.rxlib conftest.o 1>&AC_FD_CC'
                if AC_TRY_EVAL(mh_dyn_link) && test -s conftest.rxlib; then
                        LD_RXLIB_A1="ld -G -o \$(@)"
                        LD_RXLIB_A2="ld -G -o \$(@)"
#                       LD_RXLIB1="${CC} -Wl,-G"
                        LD_RXLIB_B1="-L. -l${SHLFILE}"
                        LD_RXLIB_B2="-L. -l${SHLFILE}"
                        SHLPRE="lib"
                        SHLPST=".so"
                else
                        LD_RXLIB_A1=""
                        LD_RXLIB_A2=""
                        LD_RXLIB_B1=""
                        LD_RXLIB_B2=""
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
if test "$mh_cv_func_crypt" = yes ; then
        AC_DEFINE(HAVE_CRYPT)
        if test "$mh_crypt_found" = yes; then
                MH_EXTRA_LIBS="${MH_EXTRA_LIBS} -lcrypt"
        fi
fi

LIBS="$mh_save_libs"

])dnl

dnl ---------------------------------------------------------------------------
dnl Determines the file extension for shared libraries
dnl ---------------------------------------------------------------------------
AC_DEFUN([MH_GET_SHLPST],
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
EXE=""
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
                if test "$ac_cv_prog_CC" = "gcc"; then
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
                if test "$ac_cv_prog_CC" = "gcc"; then
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
        *qnx*)
                LIBPRE=""
                LIBPST=".lib"
                SHLPRE=""
                SHLPST=".lib"
                SHLFILE="rexx"
                ;;
        *cygwin*)
                LIBPRE="lib"
                SHLPRE=""
                DYN_COMP="-DDYNAMIC"
                LIBPST=".a"
                EXE=".exe"
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

# If the build OS can handle version numbers in the shared library name,
# then set SHL_BASE="${SHLPRE}${SHLFILE}${SHLPST}.\$(ABI)"
SHL_BASE="${SHLPRE}${SHLFILE}${SHLPST}"

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
case "$target" in
        *hp-hpux*)
                EEXTRA="-Wl,-E"
                LD_RXLIB_A1="ld -b -q -n -o \$(@)"
                LD_RXLIB_A2="ld -b -q -n -o \$(@)"
                LD_RXLIB_B1=""
                LD_RXLIB_B2=""
                DYNAMIC_LDFLAGS="-Wl,+s"
                SHLPRE="lib"
                SHL_LD="ld -b -o ${SHLPRE}${SHLFILE}${SHLPST} "'$('SHOFILES')'
                ;;
        *ibm-aix3*)
                STATIC_LDFLAGS="-bnso -bI:/lib/syscalls.exp"
                LD_RXLIB_A1="ld -bnoentry -bM:SRE -bT:512 -bH:512 -bI:regina.exp -o \$(@)"
                LD_RXLIB_A2="ld -bnoentry -bM:SRE -bT:512 -bH:512 -bI:regina.exp -o \$(@)"
                LD_RXLIB_B1="${SHLPRE}${SHLFILE}${SHLPST} -lc"
                LD_RXLIB_B2="${SHLPRE}${SHLFILE}${SHLPST} -lc"
                SHLPRE="lib"
                TEST1EXPORTS="-bE:test1.exp"
                TEST2EXPORTS="-bE:test2.exp"
                TEST1EXP="test1.exp"
                TEST2EXP="test2.exp"
                REGINAEXP="regina.exp"
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
                LD_RXLIB_A1="ld -bnoentry -bM:SRE -o \$(@)"
                LD_RXLIB_A2="ld -bnoentry -bM:SRE -o \$(@)"
                LD_RXLIB_B1="${SHLPRE}${SHLFILE}${SHLPST} -lc"
                LD_RXLIB_B2="${SHLPRE}${SHLFILE}${SHLPST} -lc"
                SHLPRE="lib"
                TEST1EXPORTS="-bE:test1.exp"
                TEST2EXPORTS="-bE:test2.exp"
                TEST1EXP="test1.exp"
                TEST2EXP="test2.exp"
                REGINAEXP="regina.exp"
                if test "$ac_cv_header_dlfcn_h" = "yes" -o "$HAVE_DLFCN_H" = "1"; then
                        AIX_DYN="yes"
                        DYN_COMP="-DDYNAMIC"
                        SHL_LD="ld -o ${SHLPRE}${SHLFILE}${SHLPST} -bnoentry -bE:regina.exp -bM:SRE \$(SHOFILES) -lc \$(SHLIBS) \$(MH_MT_LIBS)"
                else
                        SHL_LD=" "'$('LIBEXE')'" "'$('LIBFLAGS')'" "'$('SHOFILES')'
                        DYN_COMP=""
                fi
                ;;
        *dec-osf*)
                LD_RXLIB_A1="ld -shared -o \$(@)"
                LD_RXLIB_A2="ld -shared -o \$(@)"
                LD_RXLIB_B1="-lc -L. -l${SHLFILE}"
                LD_RXLIB_B2="-lc -L. -l${SHLFILE}"
                SHLPRE="lib"
                SHL_LD="ld -o ${SHLPRE}${SHLFILE}${SHLPST} -shared -no_archive "'$('SHOFILES')'" -lc"
                ;;
        *esix*)
                LD_RXLIB_A1="ld -G -o \$(@)"
                LD_RXLIB_A2="ld -G -o \$(@)"
                LD_RXLIB_B1=""
                LD_RXLIB_B2=""
                DYNAMIC_LDFLAGS=""
                SHLPRE="lib"
                SHL_LD="ld -G -o ${SHLPRE}${SHLFILE}${SHLPST} "'$('SHOFILES')'
                ;;
        *dgux*)
                LD_RXLIB_A1="ld -G -o \$(@)"
                LD_RXLIB_A2="ld -G -o \$(@)"
                LD_RXLIB_B1=""
                LD_RXLIB_B2=""
                DYNAMIC_LDFLAGS=""
                SHLPRE="lib"
                SHL_LD="ld -G -o ${SHLPRE}${SHLFILE}${SHLPST} "'$('SHOFILES')'
                ;;
        *pc-sco*)
                LD_RXLIB_A1="ld -dy -G -o \$(@)"
                LD_RXLIB_A2="ld -dy -G -o \$(@)"
                LD_RXLIB_B1=""
                LD_RXLIB_B2=""
                DYNAMIC_LDFLAGS=""
                SHLPRE="lib"
                SHL_LD="ld -dy -G -o ${SHLPRE}${SHLFILE}${SHLPST} "'$('SHOFILES')'
                ;;
        *solaris*)
                if test "$ac_cv_prog_CC" = "gcc"; then
                   LD_RXLIB_A1="gcc -shared -o \$(@)"
                   LD_RXLIB_A2="gcc -shared -o \$(@)"
                   SHL_LD="gcc -shared -o ${SHLPRE}${SHLFILE}${SHLPST} "'$('SHOFILES')'
                else
                   LD_RXLIB_A1="ld -G -o \$(@)"
                   LD_RXLIB_A2="ld -G -o \$(@)"
                   SHL_LD="ld -G -o ${SHLPRE}${SHLFILE}${SHLPST} "'$('SHOFILES')'
                fi
                LD_RXLIB_B1=""
                LD_RXLIB_B2=""
                DYNAMIC_LDFLAGS=""
                SHLPRE="lib"
                ;;
        sparc*sunos*)
                LD_RXLIB_A1="ld -o \$(@)"
                LD_RXLIB_A2="ld -o \$(@)"
                LD_RXLIB_B1=""
                LD_RXLIB_B2=""
                SHLPRE="lib"
                SHL_LD="ld -assert pure-text -o ${SHLPRE}${SHLFILE}${SHLPST} "'$('SHOFILES')'
                ;;
        *freebsd*)
                LD_RXLIB_A1="ld -Bdynamic -Bshareable -o \$(@)"
                LD_RXLIB_A2="ld -Bdynamic -Bshareable -o \$(@)"
                LD_RXLIB_B1="-lc -L. -l${SHLFILE}"
                LD_RXLIB_B2="-lc -L. -l${SHLFILE}"
                SHLPRE="lib"
                SHL_LD="ld -Bdynamic -Bshareable -o ${SHLPRE}${SHLFILE}${SHLPST} "'$('SHOFILES')'
                ;;
        *linux*)
                LD_RXLIB_A1="${CC} -shared -o \$(@)"
                LD_RXLIB_A2="${CC} -shared -o \$(@)"
                LD_RXLIB_B1="-L. -l${SHLFILE}"
                LD_RXLIB_B2="-L. -l${SHLFILE}"
                SHLPRE="lib"
                SHL_LD="${CC} -o ${SHLPRE}${SHLFILE}${SHLPST}.\$(ABI) -shared ${SHL_SCRIPT} \$(SHOFILES) -lc"
                SHL_BASE="${SHLPRE}${SHLFILE}${SHLPST}.\$(ABI)"
                OTHER_INSTALLS="installabilib"
                USE_ABI="yes"
                ;;
        *atheos*)
                LD_RXLIB_A1="${CC} -shared -o \$(@)"
                LD_RXLIB_A2="${CC} -shared -o \$(@)"
                LD_RXLIB_B1="-L. -l${SHLFILE} "'$('BOTHLIBS')'
                LD_RXLIB_B2="-L. -l${SHLFILE} "'$('BOTHLIBS')'
                SHLPRE="lib"
                SHL_LD="${CC} -o ${SHLPRE}${SHLFILE}${SHLPST} -shared \$(SHOFILES) "'$('BOTHLIBS')'
                SHL_BASE="${LIBPRE}${SHLFILE}${SHLPST}"
                BASE_BINARY="atheosbinary"
                ;;
        *beos*)
                LD_RXLIB_A1="${CC} -Wl,-shared -nostart -Xlinker -o\$(@)"
                LD_RXLIB_A2="${CC} -Wl,-shared -nostart -Xlinker -o\$(@)"
                LD_RXLIB_B1="-L. -l${SHLFILE}"
                LD_RXLIB_B2="-L. -l${SHLFILE}"
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
                LD_RXLIB_B1="-L. -l${SHLFILE}"
                LD_RXLIB_B2="-L. -l${SHLFILE}"
                SHLPRE="lib"
                SHL_LD="${CC} -o ${SHLPRE}${SHLFILE}${SHLPST} -shared \$(SHOFILES)"
                SHL_BASE="${SHLPRE}${SHLFILE}${SHLPST}"
                ;;
        *qnx*)
                LD_RXLIB_A1=""
                LD_RXLIB_A2=""
                LD_RXLIB_B1=""
                LD_RXLIB_B2=""
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
                LD_RXLIB_A1="dllwrap --target i386-cygwin32 --def ${srcdir}/test1_w32_dll.def --dllname test1.dll -o \$(@)"
                LD_RXLIB_A2="dllwrap --target i386-cygwin32 --def ${srcdir}/test2_w32_dll.def --dllname test2.dll -o \$(@)"
                LD_RXLIB_B1="-L. -lregina"
                LD_RXLIB_B2="-L. -lregina"
                SHLPRE=""
                SHL_LD="dllwrap --def ${srcdir}/regina_w32_dll.def --output-lib libregina.a --target i386-cygwin32 --dllname regina.dll -o regina.dll \$(SHOFILES)"
                EEXTRA=""
                LIBPRE="lib"
                LIBPST=".a"
                LIBFILE="rexx"
                SHL_BASE="${SHLPRE}${SHLFILE}${SHLPST}"
                TEST1EXPORTS=""
                TEST2EXPORTS=""
                TEST1EXP="test1_w32_dll.def"
                TEST2EXP="test2_w32_dll.def"
                REGINAEXP="regina_w32_dll.def"
                BASE_INSTALL="cygwininstall"
                BASE_BINARY="cygwinbinary"
                OTHER_INSTALLS=""
                ;;
        *apple-darwin*)
                # to test on platform other than real Mac OSX use: --build=ppc-apple-darwin10.1 --target=ppc-apple-darwin10.1
                #
                # Link switches for building "bundles"
                #
# MH                LD_RXLIB_A1="${CC} -bundle -flat_namespace -undefined suppress -o \$(@)"
# MH                LD_RXLIB_A2="${CC} -bundle -flat_namespace -undefined suppress -o \$(@)"
                LD_RXLIB_A1="${CC} -dynamiclib -o \$(@)"
                LD_RXLIB_A2="${CC} -dynamiclib -o \$(@)"
                LD_RXLIB_B1="-L. -l${SHLFILE} -lc \$(SHLIBS)"
                LD_RXLIB_B2="-L. -l${SHLFILE} -lc \$(SHLIBS)"
                DYN_COMP="-DDYNAMIC -fno-common"
                SHLPRE="lib"
# MH                BUNDLE=".so"
                BUNDLE=".junk"
                SHL_LD="${CC} -dynamiclib -install_name ${SHLPRE}${SHLFILE}${SHLPST} -o ${SHLPRE}${SHLFILE}${SHLPST} "'$('SHOFILES')'
                SHL_BASE="${LIBPRE}${SHLFILE}${SHLPST}"
# MH                EXTRATARGET="libregina$BUNDLE"
                OTHER_INSTALLS="installlib"
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
        *)
                LINKREG="${SHLPRE}${SHLFILE}${SHLPST}"
                ;;
esac

if test "$SHL_LD" != ""; then
   SHL_LD="$SHL_LD \$(BOTHLIBS) \$(MH_MT_LIBS)"
fi

AC_MSG_CHECKING(if dynamic loading of external functions is supported)
if test "$HAVE_DLFCN_H" = "1" -o "$ac_cv_header_dl_h" = "yes" -o "$ac_cv_header_dlfcn_h" = "yes" -o "$AIX_DYN" = "yes" -o "$BEOS_DYN" = "yes"; then
   SHL_TARGETS="${SHL_BASE} regina${EXE} ${SHLPRE}test1${MODPST} ${SHLPRE}test2${MODPST} $EXTRATARGET"
   EXECISER_DEP="${SHL_BASE}"
   OTHER_INSTALLS="regina${EXE} $OTHER_INSTALLS"
   AC_MSG_RESULT("yes")
else
   SHL_TARGETS=""
   EXECISER_DEP="${STATICLIB}"
   OTHER_INSTALLS=""
   AC_MSG_RESULT("no")
fi

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
AC_SUBST(EXE)
AC_SUBST(EXECISER_DEP)
AC_SUBST(GETOPT)
AC_SUBST(LD_RXLIB_A1)
AC_SUBST(LD_RXLIB_A2)
AC_SUBST(LD_RXLIB_B1)
AC_SUBST(LD_RXLIB_B2)
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
AC_SUBST(USE_ABI)
AC_SUBST(RANLIB_DYNAMIC)
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
EXE=""
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
                if test "$ac_cv_prog_CC" = "gcc"; then
                        SYS_DEFS="-D_POSIX_SOURCE -D_XOPEN_SOURCE"
                else
                        SYS_DEFS="-D_POSIX_SOURCE -D_XOPEN_SOURCE -Olimit 800"
                fi
                ;;
        sparc*sunos*)
                SYS_DEFS="-DSUNOS -DSUNOS_STRTOD_BUG"
                ;;
        *atheos*)
                BASE_BINARY="atheosbinary"
                ;;
        *beos*)
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
                SHLPRE=""
                LIBPST=".a"
                EXE=".exe"
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
LD_RXLIB_A1="${LIBEXE} cr"
LD_RXLIB_A2="${LIBEXE} cr"
LD_RXLIB_B1=""
LD_RXLIB_B2=""
SHL_LD="${LIBEXE} cr ${SHLPRE}${SHLFILE}${SHLPST} \$(SHOFILES)"

STATICLIB="${LIBPRE}${LIBFILE}${LIBPST}"

LIBFLAGS="cr ${LIBPRE}${LIBFILE}${LIBPST}"
LIBLINK="-L. -l${LIBFILE}"
LINKSHL="-L. -l${LIBFILE}" # ensure we link to static library for execiser
DYN_COMP="$DYN_COMP -DDYNAMIC_STATIC"

SHL_BASE="${SHLPRE}${SHLFILE}${SHLPST}"
SHL_TARGETS="${SHL_BASE} ${SHLPRE}test1${MODPST} ${SHLPRE}test2${MODPST} regina${EXE} $EXTRATARGET"
EXECISER_DEP="${STATICLIB}"
OTHER_INSTALLS="regina${EXE} $OTHER_INSTALLS"
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
   MH_FUNC_LIBS="$MH_FUNC_LIBS ${SHLPRE}test1${MODPST} ${SHLPRE}test2${MODPST}"
   AC_DEFINE(HAVE_TEST_PACKAGE)
   EXECISER_DEP="${EXECISER_DEP} ${SHLPRE}test1${MODPST} ${SHLPRE}test2${MODPST}"
fi
dnl
dnl Settings for Patrick McPhee's Regutil
dnl
AC_ARG_WITH(regutil,
    [     --with-regutil       enable static linking with RegUtil external function package],
    [with_regutil=$withval],
    [with_regutil=no],
)
if test "$with_regutil" = "yes"; then
        AC_CHECK_PROG(regutil_config, [regutil-config], yes, no)
        if test "$ac_cv_prog_regutil_config" = yes; then
           EXTRA_LIB=`regutil-config --libs`
           MH_FUNC_LIBS="$MH_FUNC_LIBS $EXTRA_LIB"
           AC_DEFINE(HAVE_REXXUTIL_PACKAGE)
        else
           AC_MSG_ERROR(regutil-config not found. Cannot use --with-regutil switch)
        fi
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
           EXTRA_LIB=`rxcurses-config --libs`
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
           EXTRA_LIB=`rexxtk-config --libs`
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
           EXTRA_LIB=`rexxgd-config --libs`
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
           EXTRA_LIB=`rexxcurl-config --libs`
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
           EXTRA_LIB=`rexxsql-config --libs`
           MH_FUNC_LIBS="$MH_FUNC_LIBS $EXTRA_LIB"
           AC_DEFINE(HAVE_REXXSQL_PACKAGE)
        else
           AC_MSG_ERROR(rexxsql-config not found. Cannot use --with-rexxsql switch)
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
           EXTRA_LIB=`rexxisam-config --libs`
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
           EXTRA_LIB=`rxsock-config --libs`
           MH_FUNC_LIBS="$MH_FUNC_LIBS $EXTRA_LIB"
           AC_DEFINE(HAVE_REXXISAM_PACKAGE)
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
           EXTRA_LIB=`rexxdw-config --libs`
           MH_FUNC_LIBS="$MH_FUNC_LIBS $EXTRA_LIB"
           AC_DEFINE(HAVE_REXXDW_PACKAGE)
        else
           AC_MSG_ERROR(rexxdw-config not found. Cannot use --with-rexxdw switch)
        fi
fi

LINKREG="${SHLPRE}${SHLFILE}${SHLPST} ${MH_FUNC_LIBS} ${SHLPRE}${SHLFILE}${SHLPST}"

AC_SUBST(BASE_BINARY)
AC_SUBST(BASE_INSTALL)
AC_SUBST(BOTHLIBS)
AC_SUBST(CC2O)
AC_SUBST(CEXTRA)
AC_SUBST(DYNAMIC_LDFLAGS)
AC_SUBST(DYN_COMP)
AC_SUBST(EEXTRA)
AC_SUBST(EXE)
AC_SUBST(EXECISER_DEP)
AC_SUBST(GETOPT)
AC_SUBST(LD_RXLIB_A1)
AC_SUBST(LD_RXLIB_A2)
AC_SUBST(LD_RXLIB_B1)
AC_SUBST(LD_RXLIB_B2)
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
AC_SUBST(USE_ABI)
AC_SUBST(RANLIB_DYNAMIC)

])
