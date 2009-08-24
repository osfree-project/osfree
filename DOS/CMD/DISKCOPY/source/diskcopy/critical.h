/*
   DISKCOPY.EXE, floppy diskette duplicator similar to MSDOS Diskcopy.
   Copyright (C) 1998, Matthew Stanford.
   Copyright (C) 1999, 2000, 2001 Imre Leber.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have recieved a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


   If you have any questions, comments, suggestions, or fixes please
   email me at: imre.leber@worldonline.be

 */

#ifndef CRITICAL_H_
#define CRITICAL_H_

/* Macro's to interpret the status code. */

/* Causes: */
#define WRITEPROTECTED       0	/* Medium is write protected.  */
#define UNKNOWNDEVICE        1	/* Device unknown.             */
#define DRIVENOTREADY        2	/* Drive not ready.            */
#define UNKNOWNINSTRUCTION   3	/* Unknown instruction.        */
#define CRC_ERROR            4	/* CRC error.                  */
#define WRONGLENGTH          5	/* Wrong length of data block. */
#define SEEKERROR            6	/* Seek error.                 */
#define UNKNOWNMEDIUM        7	/* Unknown medium.             */
#define SECTOR_NOT_FOUND     8	/* Sector not found.           */
#define NOPAPER              9	/* No paper left in printer.   */
#define WRITEERROR          10	/* Write error.                */
#define READERROR           11	/* Read error.                 */
#define GENERALFAILURE      12	/* General failure.            */

/* Status values. */

#define ERRORONWRITE  0x01	/* Application where the error occured. */
			      /* 0 = read, 1 = write.                 */

#define ERRORRANGE    0x06	/* Range involved.                      */
			      /*  0 = system data.                    */
			      /*  1 = FAT.                            */
			      /*  2 = directory.                      */
			      /*  3 = data range.                     */

#define ABORTALLOWED  0x08	/* Abort allowed.                       */
#define RETRYALLOWED  0x10	/* Retry allowed.                       */
#define IGNOREALLOWED 0x20	/* Ignore allowed.                      */

#define IGNORE  0x00
#define RETRY   0x01
#define ABORTIT 0x02
#define FAIL    0x03

/* In critical.asm */
void SetCriticalHandler (int (*handler) (int status));
void RenewCriticalHandler (int (*handler) (int status));

/* In hicritcl.asm */
void CriticalHandlerOn (void);
int CriticalErrorOccured (void);
int GetCriticalCause (void);
int GetCriticalStatus (void);

#endif
