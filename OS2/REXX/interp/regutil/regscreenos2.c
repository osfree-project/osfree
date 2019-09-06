/* OS/2 screen update functions for regutil
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
 * License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is regutil.
 *
 * The Initial Developer of the Original Code is Patrick TJ McPhee.
 * Portions created by Patrick McPhee are Copyright © 1998, 2001
 * Patrick TJ McPhee. All Rights Reserved.
 *
 * Contributors:
 *
 * $Header: /opt/cvs/Regina/regutil/regscreenos2.c,v 1.3 2009/11/01 01:26:09 mark Exp $
 */
#define INCL_DOSMEMMGR
#define INCL_VIO

#include "regutil.h"

/* ******************************************************************** */
/* ************************** Screen Update *************************** */
/* ******************************************************************** */

/* using the OS/2 console api */

/* hold information about the screen */

static const char notimp[] = "not implemented";
#define what() memcpy(result->strptr, notimp, sizeof(notimp)-1), result->strlength = sizeof(notimp)-1

/* syscls() */
rxfunc(syscls)
{
   /*
    * Default to clear with spaces; attribute black background
    * TODO - use VioReadCellStr() from 0,0 to get current background and use that
    * attribute to clear
    */
   unsigned char Cell[2] = {0x20,0x07};

   checkparam(0, 0);

   VioScrollDn(0, 0, (unsigned short)0xFFFF, (unsigned short)0XFFFF, (unsigned short)0xFFFF, Cell, (SHANDLE) 0);
   VioSetCurPos(0, 0, (SHANDLE) 0);
   return 0;
}


/* syscurpos([row],[column]) */
rxfunc(syscurpos)
{
   unsigned short row;
   unsigned short col;
   long newrow;
   long newcol;
   char *srow, *scol;

   checkparam(0, 2);

   VioGetCurPos(&row, &col, (SHANDLE)0);
   newrow = row;
   newcol = col;
   result->strlength = sprintf(result->strptr, "%d %d", (int)row, (int)col);

   if (argc > 0 && RXSTRLEN(argv[0])) {
      rxstrdup(srow, argv[0]);
      newrow = atol(srow);
   }
   if (argc > 1 && RXSTRLEN(argv[1])) {
      rxstrdup(scol, argv[1]);
      newcol = atol(scol);
   }

   if (argc > 0 )
      VioSetCurPos((unsigned short)newrow, (unsigned short)newcol, (SHANDLE) 0);
   return 0;
}



/*Hides or displays the cursor.

 state
        The new cursor state. Allowed states are:

        `ON'           Display the cursor
        `OFF'          Hide the cursor
*/
/* syscurstate(state) */
rxfunc(syscurstate)
{
   unsigned short state;
   VIOCURSORINFO vci;
   char *onoff;
   int rc = 0;

   checkparam(1, 1);

   rxstrdup(onoff, argv[0]);
   strupr(onoff);
   if (!strcmp(onoff, "OFF"))
      state = -1;
   else if (!strcmp(onoff, "ON"))
      state = 0;
   else
      rc = BADGENERAL;

   if (!rc) {
      VioGetCurType(&vci, (HVIO) 0);
      vci.attr = state;
      VioSetCurType(&vci, (HVIO) 0);
      result_zero();
   }
   return rc;
}



/* read a keystroke from the input buffer and return the ascii character
 * or the scan code. If it's just a normal ascii character, return it.
 * otherwise, return the ascii character associated with the key (if
 * any) in the low byte, the scan code in the 2nd byte, and some flags
 * in the higher bytes.
 * Don't return 0, since that sucks.
 * this doesn't return mouse events, but it's not such a bad idea
 */

#define SPECIALKEY 0x10000
#define ALTKEY 0x20000
#define CTLKEY 0x40000
#define SHIFTKEY 0x80000

/* sysgetkey([opt],[timeout]) */
rxfunc(sysgetkey)
{
   what();
   return 0;
}


/* systextscreenread(row,column, len) */
rxfunc(systextscreenread)
{
   char *row, *col, *len;
   long length, line, column;

   checkparam(3, 3);

   rxstrdup(row, argv[0]);
   rxstrdup(col, argv[1]);
   rxstrdup(len, argv[2]);
   line = atol(row);
   column = atol(col);
   length = atol(len);
   /* TODO - should validate length against how many characters are left to read */
   rxresize(result, length);
   VioReadCharStr(result->strptr, (PUSHORT)&length, line, column, (HVIO) 0);
   return 0;
}


/* systextscreensize() */
rxfunc(systextscreensize)
{
   VIOMODEINFO vmi;

   checkparam(0, 0);

   vmi.cb = sizeof(vmi);
   VioGetMode(&vmi, (HVIO) 0);
   result->strlength = sprintf(result->strptr, "%d %d", (int)vmi.row,(int)vmi.col);
   return 0;
}
