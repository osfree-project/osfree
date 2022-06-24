
// 32 bits OS/2 device driver and IFS support. Provides 32 bits kernel
// services (DevHelp) and utility functions to 32 bits OS/2 ring 0 code
// (device drivers and installable file system drivers).
// Copyright (C) 1995, 1996, 1997  Matthieu WILLM (willm@ibm.net)
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.



#ifndef __os2_types_h
#define __os2_types_h

#define INLINE       _Inline

#define DH32ENTRY    _System                            /* DevHlp32_* entry points     */
#define DH32ENTRY2   _Optlink                           /* DevHlp32_* entry points     */
#define FS32ENTRY    _Far32 _Pascal                     /* fs32_* entry points         */
#define FS32INIT     _System                            /* fsh32_* entry points        */
#define DEV32ENTRY   _System                            /* device_init entry point     */
#define FSH32ENTRY   _System                            /* fsh32_* entry points        */
#define FSH32ENTRY2  _Optlink                           /* fsh32_* entry points        */
#define DRV32ENTRY   _Far32 _Pascal _loadds             /* strategy or IDC entry point */
#define FS32CALLBACK _Far32 _Pascal

#pragma pack(1)
typedef struct {
    unsigned short ofs;
    unsigned short seg;
} PTR16;
#pragma pack()

#define far
#define pascal
typedef char           INT8;
typedef char *pchar;
typedef unsigned char  UINT8;
typedef short          INT16;
typedef unsigned short UINT16;
typedef int            INT32;
typedef unsigned int   UINT32;
typedef UINT32        *PUINT32;
typedef INT32         *PINT32;

#define __const__  const          // for os2/fnmatch.h
#define __inline__ INLINE         // for os2/ctype.h

#endif /* __os2_types_h */
