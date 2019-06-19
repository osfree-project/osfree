/* $RCSfile: OPEN95.C $
   $Locker: ska $ $Name:  $   $State: Exp $

   int open95(char *fnam, long omode [, int flags] )

   Open the file "fname" in the mode "omode" supporting LFN of Win95.
   If Win95 is not running, DOS 4 functionality is used.

   Note: This function does NOT support DOS prior version 4!

   $Log: OPEN95.C $
   Revision 1.1  2000/01/11 09:10:09  ska
   Auto Check-in

*/

#include <assert.h>
#include <fcntl.h>
#include <dos.h>     // REGPACK
#include <string.h>
#include <io.h>
#include <stdlib.h>

#ifdef USE_IO95
#undef USE_IO95
#endif
#define IO95_NO_OPEN_PROTOTYPE
#include "io95.h"

#define OPEN_CREATE (1 << 4)
#define OPEN_TRUNC (1 << 1)
#define OPEN_OPEN (1 << 0)
#define OPEN_FLAGS (O95_AUTOCOMMIT | O95_NOCRIT | O95_NOCOMPRESS | O95_UNBUFFERED | 0xff)
#define OPEN_SHARE (7<<4)

#ifndef lint
static char const rcsid[] = 
   "$Id: OPEN95.C 1.1 2000/01/11 09:10:09 ska Exp ska $";
#endif


/**************** API
--------D-21716C-----------------------------                                   
INT 21 - Windows95 - LONG FILENAME - CREATE OR OPEN FILE                        
        AX = 716Ch                                                              
        BX = access mode and sharing flags (see #1122,also AX=6C00h)            
        CX = attributes                                                         
        DX = action (see #1121)                                                 
        DS:SI -> ASCIZ filename                                                 
        DI = alias hint (number to append to short filename for disambiguation) 
Return: CF clear if successful                                                  
            AX = file handle                                                    
            CX = action taken                                                   
                0001h file opened                                               
                0002h file created                                              
                0003h file replaced                                             
        CF set on error                                                         
            AX = error code (see #1020)                                         
                7100h if function not supported                                 
SeeAlso: AX=6C00h,AX=7141h,AX=7156h,AX=71A9h                                    
                                                                                
Bitfields for Windows95 long-name open action:                                  
Bit(s)  Description     (Table 1121)                                            
 0      open file (fail if file does not exist)                                 
 1      truncate file if it already exists (fail if file does not exist)        
 4      create new file if file does not already exist (fail if exists)         
Note:   the only valid combinations of multiple flags are bits 4&0 and 4&1      
                                                                                
Bitfields for Windows95 file access/sharing modes:                              
Bit(s)  Description     (Table 1122)                                            
 2-0    file access mode                                                        
        000 read-only                                                           
        001 write-only                                                          
        010 read-write                                                          
        100 read-only, do not modify file's last-access time                    
 6-4    file sharing modes                                                      
 7      no-inherit flag                                                         
 8      do not buffer data (requires that all reads/writes be exact physical    
          sectors)                                                              
 9      do not compress file even if volume normally compresses files           
 10     use alias hint in DI as numeric tail for short-name alias               
 12-11  unused??? (0)                                                           
 13     return error code instead of generating INT 24h if critical error       
          while opening file                                                    
 14     commit file after every write operation                                 
SeeAlso: #0749                                                                  


--------D-216C00-----------------------------                                   
INT 21 - DOS 4.0+ - EXTENDED OPEN/CREATE                                        
        AX = 6C00h                                                              
        BL = open mode as in AL for normal open (see also AH=3Dh)               
            bit 7: inheritance                                                  
            bits 4-6: sharing mode                                              
            bit 3 reserved                                                      
            bits 0-2: access mode                                               
                100 read-only, do not modify file's last-access time (DOS 7.0)  
        BH = flags                                                              
            bit 6 = auto commit on every write (see also AH=68h)                
            bit 5 = return error rather than doing INT 24h                      
            bit 4 = (FAT32) extended size (>= 2GB)                              
        CX = create attribute (see #1111)                                       
        DL = action if file exists/does not exist (see #1112)                   
        DH = 00h (reserved)                                                     
        DS:SI -> ASCIZ file name                                                
Return: CF set on error                                                         
           AX = error code (see #1020 at AH=59h/BX=0000h)                       
        CF clear if successful                                                  
           AX = file handle                                                     
           CX = status (see #1110)                                              
Notes:  the PC LAN Program only supports existence actions (in DL) of 01h,      
          10h with sharing=compatibility, and 12h                               
        DR DOS reportedly does not support this function and does not return    
          an "invalid function call" error when this function is used.          
        the documented bits of BX are stored in the SFT when the file is opened 
          (see #0982,#0983)                                                     
BUG:    this function has bugs (at least in DOS 5.0 and 6.2) when used with     
          drives handled via the network redirector (INT 2F/AX=112Eh):          
            - CX (attribute) is not passed to the redirector if DL=11h,         
            - CX does not return the status, it is returned unchanged because   
              DOS does a PUSH CX/POP CX when calling the redirector.            
SeeAlso: AH=3Ch,AH=3Dh,AX=6C01h,AH=71h,INT 2F/AX=112Eh                          
                                                                                
(Table 1110)                                                                    
Values for extended open function status:                                       
 01h    file opened                                                             
 02h    file created                                                            
 03h    file replaced                                                           
                                                                                
Bitfields for file create attribute:                                            
Bit(s)  Description     (Table 1111)                                            
 6-15   reserved                                                                
 5      archive                                                                 
 4      reserved                                                                
 3      volume label                                                            
 2      system                                                                  
 1      hidden                                                                  
 0      readonly                                                                
                                                                                
Bitfields for action:                                                           
Bit(s)  Description     (Table 1112)                                            
 7-4    action if file does not exist                                           
        0000 fail                                                               
        0001 create                                                             
 3-0    action if file exists                                                   
        0000 fail                                                               
        0001 open                                                               
        0010 replace/open                                                       
**************/

int open95(const char * const fnam, long omode, int flags)
{  struct REGPACK r, r1;
   int h;

   assert(fnam);
   /* Frist try the Win95 API */
   r.r_flags = 1;          /* make sure the carru is set */
   r.r_ds = FP_SEG(fnam);     /* pointer to file name */
   r.r_si = FP_OFF(fnam);
   r.r_ax = 0x716c;        /* Win95 open/create */
   r.r_cx = flags ^ S95_IWRITE;
   if(omode & O95_CREAT)         /* creation behaviour */
      r.r_dx = (omode & O95_EXCL)? OPEN_CREATE
       : (omode & O95_TRUNC)? OPEN_CREATE | OPEN_TRUNC
       : OPEN_CREATE | OPEN_OPEN;
   else
      r.r_dx = (omode & O95_TRUNC)? OPEN_TRUNC: OPEN_OPEN;
   r.r_bx = omode & OPEN_FLAGS;
   if(!(r.r_bx & OPEN_SHARE))
      r.r_bx |= O95_DENYWRITE;

   /* because the call the DOS 4 style function is nearly the same
      the structure is preserved for the next try */
   memcpy(&r1, &r, sizeof(r));

   intr(0x21, &r);      /* Perform the Win95 API call */

   if((r.r_flags & 1)         /* failed */
    && r.r_ax == 1) {      /* Win95 not present -> call DOS 4 API */
      r1.r_ax = 0x6c00;
         /* Suppress Win95 specific flags */
      r1.r_bx &= O95_AUTOCOMMIT | O95_NOCRIT | O95_NOINHERIT | OPEN_SHARE | 3;

      intr(0x21, &r1);  /* Perform DOS 4 API call */

      if(r1.r_flags & 1) { /* failed, too */
         errno = r1.r_ax;  /* preserve this error */
         return -1;
      }

      r.r_ax = r1.r_ax;    /* re-join both APIs */
   }

   /* One of the API returned successfully -> do whatever is needed */

   /* Now there is some problem with the O95_TEXT/O95_BINARY flags.
      Borland supports the one, but it is "internally" done, what means
      it has to be simulated somehow. */

   /* Well, we open another file with the original function and
      patch our handle onto that one. */
   if((h = open("nul", (omode & O95_TEXT? O_TEXT: O_BINARY) | (omode & 3))) == -1) {
      /* Oops, something went a bit wrong here -> bail out immediately */
      close(r.r_ax);
      return -1;
   }
   /* Now, we duplicate our handle over the returned one; DOS duplicates
      all necessary flags etc, but because we bypass the C library,
      the internal structure remains OK */
   r1.r_ax = 0x4600;    /* duplicate file handle to other file handle */
   r1.r_bx = r.r_ax;
   r1.r_cx = h;
   intr(0x21, &r1);
   if(r1.r_flags & 1) { /* should never arise */
      close(h);
      close(r.r_ax);
      return -1;
   }
   close(r.r_ax);       /* is not needed anylonger! */

   if(omode & O95_APPEND)     /* seek to EOF */
      lseek(h, 0l, 1);

   return h;
}
