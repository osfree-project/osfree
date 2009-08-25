/* LTOOLS
   Programs to read, write, delete and change Linux extended 2 filesystems under DOS

   Module readdisk.c

   This file is does include the operation specific real
   readdisk-routines.

   Copyright information and copying policy see file README.TXT

   History see file MAIN.C
 */

extern int isDebug;
extern int DebugOut(int level, const char *fmt,...);


#ifndef UNIX
#ifdef __OS2__
#include "OS2disk.h"
#else
#include "DOSdisk.h"
#endif
#else
#define STDOUT stdout
#define STDERR stderr
#include "UNIXdisk.h"
#endif
