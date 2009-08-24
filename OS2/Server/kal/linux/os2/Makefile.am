AM_CFLAGS = -I$(top_srcdir)/include
lib_LTLIBRARIES    = libos2.la
libos2_la_SOURCES  = datetime.c memmgr.c nls.c process.c semaphore.c exceptions.c  misc.c nmpipes.c queues.c sub.c filemgr.c modulemgr.c os2.c resources.c utility.c
libos2_la_LDFLAGS  = -version-info ${OS2LINUX_LTVERSION} -Xcompiler -nostartfiles
noinst_HEADERS	   = os2linux.h
