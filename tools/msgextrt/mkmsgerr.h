/****************************************************************************
 *
 *  mkmsgerr.h -- Make Message File Utilities
 *
 *  ========================================================================
 *
 *    Version 1.1       Michael K Greene <mikeos2@gmail.com>
 *                      September 2023 
 *
 *  ========================================================================
 *
 *  Description: MKMSGF and MKMSGD error codes
 *
 *  ========================================================================
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 ***************************************************************************/

#ifndef MKMSGERR_H
#define MKMSGERR_H

#define MKMSG_NOERROR           000
#define MKMSG_GETOPT_ERROR      001 // MKMSGF: DBCS not supported
#define MKMSG_NOINPUT_ERROR     002 // MKMSGF: something bad happened in getop
#define MKMSG_IN_OUT_COMPARE    003 // input and output file same name
#define MKMSG_BAD_TYPE          004 // Bad message type
#define MKMSG_LANG_OUT_RANGE    005 // Language family is outside of valid range
#define MKMSG_SUBID_OUT_RANGE   006 // Sub id is outside of valid codepage range
#define MKMSG_INPUT_ERROR       100 // MKMSG: Bad input file for decompile
#define MKMSG_OPEN_ERROR        101 // MKMSG: Error open decompile input file
#define MKMSG_OFFID_ERR         102 // Error open file offsetid routine
#define MKMSG_READ_ERROR        103 // MKMSG: Decompile input read error
#define MKMSG_HEADER_ERROR      104 // MKMSG: Decompile input signature error
#define MKMSG_INDEX_ERROR       105 // MKMSG: Decompile index size != actual
#define MKMSG_READHDR_ERR       106 // MKMSG: Decompile input read error
#define MKMSG_WRITEHDR_ERR      107 // MKMSG: Decompile input read error
#define MKMSG_ERRFILEWRITE      108 // MKMSG: Decompile input read error
#define MKMSG_MEM_ERROR1        200 // MKMSG: Decompile mem allocate error
#define MKMSG_MEM_ERROR2        201 // MKMSG: Decompile mem allocate error
#define MKMSG_MEM_ERROR3        202 // MKMSG: Decompile mem allocate error
#define MKMSG_MEM_ERROR4        203 // MKMSG: Decompile mem allocate error
#define MKMSG_MEM_ERROR5        204 // MKMSG: Decompile mem allocate error
#define MKMSG_MEM_ERROR6        205 // MKMSG: Decompile mem allocate error
#define MKMSG_MEM_ERROR7        206 // MKMSG: Decompile mem allocate error
#define MKMSG_MEM_ERROR8        207 // MKMSG: Decompile mem allocate error
#define MKMSG_MEM_ERROR9        208 // MKMSG: Decompile mem allocate error


#endif
