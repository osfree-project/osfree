/****************************************************************************
 *
 *  mkmsgf.h -- Make Message File Utility (MKMSGF) Clone
 *
 *  ========================================================================
 *
 *    Version 1.1       Michael K Greene <mikeos2@gmail.com>
 *                      September 2023
 *
 *    Version 1.0       July 2008
 *
 *  ========================================================================
 *
 *  Description: Header containing message file structures
 *
 *  Based on previous work:
 *      (C) 2002-2008 by Yuri Prokushev
 *      (C) 2001 Veit Kannegieser
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

#ifndef MKMSGF_H
#define MKMSGF_H

#include <stdint.h>

/* Basic msg file layout:

               ^
               |  Messages
               |  FILECOUNTRYINFO
               |
               |  Index to messages, each message uint16_t offset from
               |  start of file to beginning of mesasge
               |
               |  MSGHEADER
  --> start file
 */

#pragma pack(push, 1)

// Header of message file
typedef struct _MSGHEADER
{
    uint8_t magic_sig[8];  // Magic word signature
    uint8_t identifier[3]; // Identifier (SYS, DOS, NET, etc.)
    uint16_t numbermsg;    // Number of messages
    uint16_t firstmsg;     // Number of the first message
    int8_t offset16bit;    // Index table index uint16 == 1 or uint32 == 0
    uint16_t version;      // File version 2 - New Version 0 - Old Version
    uint16_t hdroffset;    // pointer - Offset of index table - size of _MSGHEADER
    uint16_t countryinfo;  // pointer - Offset of country info block (cp)
    uint32_t extenblock;   // pointer to ext block - 0 if none
    uint8_t reserved[5];   // Must be 0 (zero)
} MSGHEADER, *PMSGHEADER;

// Country Info block of message file
typedef struct _FILECOUNTRYINFO
{
    uint8_t bytesperchar;        // Bytes per char (1 - SBCS, 2 - DBCS)
    uint16_t country;            // ID country
    uint16_t langfamilyID;       // Language family ID (As in CPI Reference)
    uint16_t langversionID;      // Language version ID (As in CPI Reference)
    uint16_t codepagesnumber;    // Number of codepages
    uint16_t codepages[16];      // Codepages list (Max 16)
    uint8_t filename[_MAX_PATH]; // Name of file
    uint8_t filler;              // filler byte - not used
} FILECOUNTRYINFO, *PFILECOUNTRYINFO;

// extended header block
typedef struct _EXTHDR
{
    uint16_t hdrlen;    // length of ???
    uint16_t numblocks; // number of additional FILECOUNTRYINFO blocks
} EXTHDR, *PEXTHDR;

typedef struct suppinfo
{
    char langcode[4];
    int langfam;
    int langsub;
    char lang[20];
    char country[15];
};

struct suppinfo langinfo[] = {
    {"ARA", 1, 2, "Arabic", "Arab Countries"},
    {"BGR", 2, 1, "Bulgarian", "Bulgaria"},
    {"CAT", 3, 1, "Catalan", "Spain"},
    {"CHT", 4, 1, "Traditional Chinese", "R.O.C."},
    {"CHS", 4, 2, "Simplified Chinese", "P.R.C."},
    {"CSY", 5, 1, "Czech", "Czechoslovakia"},
    {"DAN", 6, 1, "Danish", "Denmark"},
    {"DEU", 7, 1, "German", "Germany"},
    {"DES", 7, 2, "Swiss German", "Switzerland"},
    {"EEL", 8, 1, "Greek", "Greece"},
    {"ENU", 9, 1, "US English", "United States"},
    {"ENG", 9, 2, "UK English", "United Kingdom"},
    {"ESP", 10, 1, "Castilian Spanish", "Spain"},
    {"ESM", 10, 2, "Mexican Spanish", "Mexico"},
    {"FIN", 11, 1, "Finnish", "Finland"},
    {"FRA", 12, 1, "French", "France"},
    {"FRB", 12, 2, "Belgian French", "Belgium"},
    {"FRC", 12, 3, "Canadian French", "Canada"},
    {"FRS", 12, 4, "Swiss French", "Switzerland"},
    {"HEB", 13, 1, "Hebrew", "Israel"},
    {"HUN", 14, 1, "Hungarian", "Hungary"},
    {"ISL", 15, 1, "Icelandic", "Iceland"},
    {"ITA", 16, 1, "Italian", "Italy"},
    {"ITS", 16, 2, "Swiss Italian", "Switzerland"},
    {"JPN", 17, 1, "Japanese", "Japan"},
    {"KOR", 18, 1, "Korean", "Korea"},
    {"NLD", 19, 1, "Dutch", "Netherlands"},
    {"NLB", 19, 2, "Belgian Dutch", "Belgium"},
    {"NOR", 20, 1, "Norwegian - Bokmal", "Norway"},
    {"NON", 20, 2, "Norwegian - Nynorsk", "Norway"},
    {"PLK", 21, 1, "Polish", "Poland"},
    {"PTB", 22, 1, "Brazilian Portugues", "Brazil"},
    {"PTG", 22, 2, "Portuguese", "Portugal"},
    {"RMS", 23, 1, "Rhaeto-Romanic", "Switzerland"},
    {"ROM", 24, 1, "Romanian", "Romania"},
    {"RUS", 25, 1, "Russian", "Russian"},
    {"SHL", 26, 1, "Croato-Serbian", "Yugoslavia"},
    {"SHC", 26, 2, "Serbo-Croatian", "Yugoslavia"},
    {"SKY", 27, 1, "Slovakian", "Czechoslovakia"},
    {"SQI", 28, 1, "Albanian", "Albania"},
    {"SVE", 29, 1, "Swedish", "Sweden"},
    {"THA", 30, 1, "Thai", "Thailand"},
    {"TRK", 31, 1, "Turkish", "Turkey"},
    {"URD", 32, 1, "Urdu", "Pakistan"},
    {"BAH", 33, 1, "Bahasa", "Indonesia"},
    {"SLO", 34, 1, "Slovene", "Slovenia"},
    {"MAX", 0, 0, "NONE", "NONE"},
};

#pragma pack(pop)

// Header of message file
typedef struct _MESSAGEINFO
{
    // mkmsgd options
    char infile[_MAX_PATH]; // input filename
    char indrive[_MAX_DRIVE];
    char indir[_MAX_DIR];
    char infname[_MAX_FNAME];
    char inext[_MAX_EXT];

    char outfile[_MAX_PATH]; // output filename

    uint8_t verbose; // how much to see?
    // compile/decompile info
    uint8_t identifier[3];       // Identifier (SYS, DOS, NET, etc.)
    uint16_t numbermsg;          // Number of messages
    uint16_t firstmsg;           // Number of the first message
    int8_t offsetid;             // Index table index uint16 == 1 or uint32 == 0
    uint16_t version;            // File version 2 - New Version 0 - Old Version
    uint16_t hdroffset;          // pointer - Offset of index table - size of _MSGHEADER
    uint16_t countryinfo;        // pointer - Offset of country info block (cp)
    uint32_t extenblock;         // better desc?
    uint8_t reserved[5];         // blank bytes?
    uint8_t bytesperchar;        // Bytes per char (1 - SBCS, 2 - DBCS)
    uint16_t country;            // ID country
    uint16_t langfamilyID;       // Language family ID (As in CPI Reference)
    uint16_t langversionID;      // Language version ID (As in CPI Reference)
    uint16_t codepagesnumber;    // Number of codepages
    uint16_t codepages[16];      // Codepages list (Max 16)
    uint8_t filename[_MAX_PATH]; // Name of file
    uint16_t extlength;          // length of ???
    uint16_t extnumblocks;       // number of additional sub FILECOUNTRYINFO blocks
    fpos_t indexoffset;          // okay dup of hdroffset
    uint16_t indexsize;          // size in bytes of index
    fpos_t msgoffset;            // offset to start of messages
    uint32_t msgfinalindex;      // offset to end of messages
    fpos_t msgstartline;         // start position for compile
    uint8_t langfamilyIDcode;    // Save array position for easy lookup
    uint8_t fakeextend;          // Append a fake extended header
    uint8_t fixlastline;         // Try and fix last line issues
} MESSAGEINFO;

// mkmsgf header signature - a valid MSG file alway starts with
// these 8 bytes 0xFF MKMSGF 0x00
char signature[] = {0xFF, 0x4D, 0x4B, 0x4D, 0x53, 0x47, 0x46, 0x00};

char extfake[] = {0x2E, 0x01, 0x00, 0x00};

#endif
