/* $Id: FIND95.H 1.1 2000/01/11 09:10:09 ska Exp ska $
   $Locker: ska $ $Name:  $   $State: Exp $


   $Log: FIND95.H $
   Revision 1.1  2000/01/11 09:10:09  ska
   Auto Check-in

*/

#ifndef __DOSFIND_H
#define __DOSFIND_H

#define DTALEN 128
#define NOFIND95 0

typedef unsigned char QWORD[8];

#define FA95_RDONLY  1
#define FA95_HIDDEN 2
#define FA95_SYSTEM  4
#define FA95_LABEL   8
#define FA95_VOLUME FA95_LABEL
#define FA95_DIREC   16
#define FA95_ARCHIVE 32
#define FA95_ARCH FA95_ARCHIVE
#define FA95_ALL 0x3f


typedef struct ffblk95 {
   char   ff_fcb[0x15];
   unsigned char   ff_attr;
#define ff_attrib ff_attr
   unsigned short  ff_time;
#define ff_ftime ff_time
   unsigned short  ff_date;
#define ff_fdate ff_date
   long   ff_size;
#define ff_fsize ff_size
   char   ff_name[14];
/* block for Win95 */
   struct {
      long ff_attr95;
      long ff_creattime;
      long ff_creatdate;
      long ff_accesstime;
      long ff_accessdate;
      long ff_modtime;
      long ff_moddate;
      long ff_hisize;
      long ff_losize;
      char ff_dummy[8];
      char ff_longname[260];
      char ff_shortname[14];
      unsigned ff_status;
   } ff_95;
} FF_Block95;

#ifndef HAVE_GETSETDTA
/* Providing access to the process's DTA
*/
extern char far      *getdta     (void);
extern void        setdta     (char far*);
#endif

int findfirst95(char *, FF_Block95 *, int);
/* Start a search and fill the FF block

   If ff_95.ff_status != NOFIND95, the values of ff_95 are invalid.
   The "normal" values are always correct.

   Return:
      == 0: OK
      != 0: failure; OS error code
*/

int findnext95(FF_Block95 *);
/* Continue a previously startet search.
   Return:
      s. findfirst
*/

void findstop95(FF_Block95 *);
/* This function must be called, if the findfirst95/findnext95 loop
   shall be terminated before findfirst95() or findnext95() have
   returned with failure.
*/

#ifdef USE_IO95
#undef findfirst
#undef findnext
#undef findstop
#undef FF_Block
#undef ffblk
#undef FA_RDONLY
#undef FA_HIDDEN
#undef FA_SYSTEM
#undef FA_LABEL
#undef FA_VOLUME
#undef FA_DIREC
#undef FA_ARCHIVE
#undef FA_ARCH
#define findfirst findfirst95
#define findnext findnext95
#define findstop findstop95
#define FF_Block FF_Block95
#define ffblk ffblk95
#define FA_RDONLY FA95_RDONLY
#define FA_HIDDEN FA95_HIDDEN
#define FA_SYSTEM FA95_SYSTEM
#define FA_LABEL FA95_LABEL
#define FA_VOLUME FA95_VOLUME
#define FA_DIREC FA95_DIREC
#define FA_ARCHIVE FA95_ARCHIVE
#define FA_ARCH FA95_ARCH
#define FA_ALL FA95_ALL
#endif

#endif
