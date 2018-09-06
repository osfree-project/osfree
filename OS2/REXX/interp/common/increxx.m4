dnl
dnl Common configure switches related to Rexx interpreter
dnl
dnl Uses following functions:
dnl   CHECK_REXX

dnl ---------------------- check for REXX argument -----------------
dnl allow --with-rexx for Rexx support
AC_ARG_WITH(rexx,
	[  --with-rexx=int         specify Rexx interpeter (rexxtrans|regina|rexximc|oorexx|objrexx|unirexx|rexx6000)],
	[with_rexx=$withval],
	[with_rexx=no],
)
dnl allow --with-rexxincdir to specify where REXX header file is
AC_ARG_WITH(rexxincdir,
	[  --with-rexxincdir=DIR   specify where REXX header file is],
	[with_rexxincdir=$withval],
	[with_rexxincdir=no],
)
dnl allow --with-rexxlibdir to specify where REXX library file is
AC_ARG_WITH(rexxlibdir,
	[  --with-rexxlibdir=DIR   specify where REXX library file is],
	[with_rexxlibdir=$withval],
	[with_rexxlibdir=no],
)
CHECK_REXX
