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

#ifndef PARSER_H_
#define PARSER_H_

typedef int TOKEN;

/* Parser input structure. */

struct LValueArray
  {
    TOKEN token;
    void (*func) (TOKEN rvalue);
  };

struct HeaderArray
  {
    int amount;
    TOKEN header;
    struct LValueArray *lvalues;
  };

/* Parser output structure. */

struct IniParserStruct
  {

    /* MEMORY */
    int UseEMS;			/* Use of EMS  granted.               */
    int UseXMS;			/* Use of XMS  granted.               */
    int UseSWAP;		/* Use of SWAP granted.               */

    /* OPTIONS */
    int audible;		/* Give an audible warning.           */
    int verify;			/* Verify writes.                     */
    int informative;		/* Give information on memory use.    */
    int overwrite;		/* Overwrite destination.             */
    int autoexit;		/* Always automatically exit.         */
    int askdisk;		/* Ask disk.                          */
    int asktdisk;		/* Ask target disk.                   */
    int mode;			/* Mode that diskcopy is in.          */
    int speed;			/* Copy speed: fast or full.          */
    int serialnumber;		/* Wether to update the serial number */

    /* GENERATION */
    int MakeDAT;		/* Wether or not to keep a compiled 
				   DAT file.                          */
  };

/* Tokens. */
#define TknNONE         -1	/* Invalid input.         */
#define TknDONE          0	/* End of buffer reached. */

#define TknSPACE         1	/* white space  */
#define TknASSIGN        2	/* =            */
#define TknYES           3	/* yes          */
#define TknNO            4	/* no           */
#define TknALWAYS        5	/* always       */
#define TknNEVER         6	/* never        */


#define TknMEMORYHEADER  7	/* [- MEMORY -] */
#define TknDISK          8	/* DISK/SWAP    */
#define TknEMS           9	/* EMS          */
#define TknXMS          10	/* XMS          */

#define TknOPTIONHEADER 11	/* [- OPTIONS -] */
#define TknAUDIBLE      12	/* audible       */
#define TknVERIFY       13	/* verify        */
#define TknINFORMATIVE  14	/* informative   */
#define TknOVERWRITE    15	/* overwrite     */
#define TknAUTOEXIT     16	/* autoexit      */
#define TknMODE         17	/* mode          */
#define TknASKDISK      18	/* ask disk      */
#define TknASKTARGET    19	/* ask target    */
#define TknSERIALNUMBER 20	/* serial number */
#define TknUPDATE       21	/* update        */
#define TknLEAVE        22	/* leave         */

#define TknRECOVERY     23	/* recovery      */
#define TknNORMAL       24	/* normal        */

#define TknRETURN       25	/* Return        */

#define TknGENERATEHEADER 26	/* [- GENERATE -] */
#define TknUSEDATFILE     27	/* usedatfile */

#define TknFAST      28
#define TknFULL      29
#define TknSPEED     30

#define TknOPENBLOCK  31
#define TknCLOSEBLOCK 32

#define PARSERSUCCESS       1

#define YES    1
#define NO     0

#define ALWAYS 1
#define NEVER  0

#define RECOVERY 1
#define NORMAL   0

#define UPDATE 1
#define LEAVE  0

int ParseIniFile (char *filename);
struct IniParserStruct *GetParsedData (void);

#endif
