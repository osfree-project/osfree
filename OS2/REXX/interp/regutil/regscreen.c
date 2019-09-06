/* NT screen update functions for regutil
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
 * $Header: /opt/cvs/Regina/regutil/regscreen.c,v 1.2 2009/11/02 22:40:18 mark Exp $
 */
#include "regutil.h"

/* ******************************************************************** */
/* ************************** Screen Update *************************** */
/* ******************************************************************** */

/* using the windows console api */

/* hold information about the screen */
#ifdef _WIN32

#include <conio.h>

static HANDLE c_input = NULL, c_output = NULL;
static WORD oldattrs = 0, width = 0, height=0;
static int cmode_in;

static void sethandles(void)
{
   CONSOLE_SCREEN_BUFFER_INFO csbi;

   if (c_input == NULL) {
      c_input = GetStdHandle(STD_INPUT_HANDLE);
      c_output = GetStdHandle(STD_OUTPUT_HANDLE);

      /* get the old screen attributes, and save the width and height while
       * we're at it */
      GetConsoleScreenBufferInfo(c_output, &csbi);
      height = csbi.dwSize.Y;
      width = csbi.dwSize.X;
      oldattrs = csbi.wAttributes;

      /* and the input mode */
      GetConsoleMode(c_input, &cmode_in);
   }
}

#endif

/* syscls() */
rxfunc(syscls)
{
   COORD pos;
   long res;

   pos.X=0;
   pos.Y=0;

   sethandles();

   FillConsoleOutputCharacter(c_output, ' ', (width*height)-1, pos, &res);
   FillConsoleOutputAttribute(c_output, oldattrs, (width*height)-1, pos, &res);
   SetConsoleCursorPosition(c_output, pos);

   result_zero();


   return 0;
}


/* syscurpos([row],[column]) */
rxfunc(syscurpos)
{
   CONSOLE_SCREEN_BUFFER_INFO csbi;
   char * row, *column;
   COORD newpos;

   checkparam(0, 2);

   sethandles();

   GetConsoleScreenBufferInfo(c_output, &csbi);
   result->strlength = sprintf(result->strptr, "%d %d", csbi.dwCursorPosition.Y, csbi.dwCursorPosition.X);

   memcpy(&newpos, &csbi.dwCursorPosition, sizeof(COORD));

   if (argc > 0 && RXSTRLEN(argv[0])) {
      rxstrdup(row, argv[0]);
      newpos.Y = atoi(row);
   }
   if (argc > 1 && RXSTRLEN(argv[1])) {
      rxstrdup(column, argv[1]);
      newpos.X = atoi(column);
   }


   if (argc > 0)
      SetConsoleCursorPosition(c_output, newpos);

   return 0;
}


/* syscurstate(state) */
rxfunc(syscurstate)
{
   CONSOLE_CURSOR_INFO cci;
   int rc;
   char * onoff;
   rc = 0;

   checkparam(1, 1);

   sethandles();

   rxstrdup(onoff, argv[0]);
   strupr(onoff);


   GetConsoleCursorInfo(c_output, &cci);

   if (!strcmp(onoff, "OFF"))
      cci.bVisible = FALSE;
   else if (!strcmp(onoff, "ON"))
      cci.bVisible = TRUE;
   else
      rc = BADGENERAL;

   if (!rc) {
      SetConsoleCursorInfo(c_output, &cci);
      result_zero();
   }

   return rc;
}

/*Hides or displays the cursor.

 state
        The new cursor state. Allowed states are:

        `ON'           Display the cursor
        `OFF'          Hide the cursor
*/



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

static int getachar(int timeout)
{
   INPUT_RECORD ir;
   int c = 0;
   long res;

   sethandles();
   SetConsoleMode(c_input, 0);

   /* have to look for key events, rather than just calling ReadConsole,
    * since ReadConsole doesn't return extended codes and it doesn't
    * handle extended characters or arrow keys, or much of anything.
    * There are too many key events, though, so we have to do some
    * filtering */
   for (ir.EventType = 0;
        ir.EventType != KEY_EVENT ||
         (!ir.Event.KeyEvent.bKeyDown ||
          ir.Event.KeyEvent.wVirtualKeyCode == VK_SHIFT ||
          ir.Event.KeyEvent.wVirtualKeyCode == VK_CONTROL ||
          ir.Event.KeyEvent.wVirtualKeyCode == VK_MENU);
        ReadConsoleInput(c_input, &ir, 1, &res))
      if (timeout && WaitForSingleObject(c_input, timeout)) {
         c = -1;
         break;
      }

   if (c != -1) {
      /* initialiase the return code to the Ascii character value */
      c = ir.Event.KeyEvent.uChar.AsciiChar;

      /* this is a `special' key if either the ascii value is 0 (indicating
       * a function key, arrow key, etc), or the alt key is pressed */
      if ((!ir.Event.KeyEvent.uChar.AsciiChar) ||
          (ir.Event.KeyEvent.dwControlKeyState & (RIGHT_ALT_PRESSED|LEFT_ALT_PRESSED))) {
         c |= SPECIALKEY | (((int)ir.Event.KeyEvent.wVirtualScanCode) << 8);
         if (ir.Event.KeyEvent.dwControlKeyState & (RIGHT_ALT_PRESSED|LEFT_ALT_PRESSED))
            c |= ALTKEY;
         if (ir.Event.KeyEvent.dwControlKeyState & SHIFT_PRESSED)
            c |= SHIFTKEY;
         if (ir.Event.KeyEvent.dwControlKeyState & (RIGHT_CTRL_PRESSED|LEFT_CTRL_PRESSED))
            c |= CTLKEY;
      }
   }

   SetConsoleMode(c_input, cmode_in);

   return c;
}


#ifdef NOT_LIKE_IBM
/* names for keys -- this has got to be sorted by scan code */
static struct scankeys {
   int code;
   char name[10];
   int len;
} scantbl[] = {
   0x3b, "f1", 2,
   0x3c, "f2", 2,
   0x3d, "f3", 2,
   0x3e, "f4", 2,
   0x3f, "f5", 2,
   0x40, "f6", 2,
   0x41, "f7", 2,
   0x42, "f8", 2,
   0x43, "f9", 2,
   0x44, "f10", 3,
   0x47, "Home", 4,
   0x48, "Up", 2,
   0x49, "Page Up", 7,
   0x4b, "Left", 4,
   0x4d, "Right", 5,
   0x4f, "End", 3,
   0x50, "Down", 4,
   0x51, "Page Down", 9,
   0x52, "Insert", 6,
   0x53, "Delete", 6,
   0x57, "f11", 3,
   0x58, "f12", 3,
};

static int scancmp(const void * const l, const void * const r)
{
   return ((struct scankeys *)l)->code - ((struct scankeys *)r)->code;
}
#endif


/* sysgetkey([opt],[timeout]) */
rxfunc(sysgetkey)
{
   register rxbool doecho = true;
   register int inchar;
   int timeout = 0;
# ifdef NOT_LIKE_IBM
   struct scankeys key, *res;
# else
   static int savescan;

   if (savescan) {
      result->strlength = 1;
      result->strptr[0] = savescan;
      savescan = 0;
      return 0;
   }
# endif

   checkparam(0,2);

   if (argc > 0 && argv[0].strptr && (argv[0].strptr[0] == 'N' || argv[0].strptr[0] == 'n'))
      doecho = false;

   if (argc > 1 && argv[1].strptr) {
      timeout = rxint(argv+1)*1000 + rxuint(argv+1)/1000;
   }

   inchar = getachar(timeout);

   if (inchar == -1) {
      result->strlength = 0;
      return 0;
   }


   /* echo ascii values */
   if (doecho && !(inchar&SPECIALKEY))
      putchar(inchar&0xff);

   if (!(inchar&SPECIALKEY)) {
      result->strlength = 1;
      result->strptr[0] = inchar;
   }
   else {
#    ifndef NOT_LIKE_IBM
      /* for compatibility with IBM's rexxutil, return 0 on the first call,
       * and then the scan code on the second for special characters */
      savescan = (inchar&0xff00) >> 8;
      result->strlength = 1;
      result->strptr[0] = 0;
#    else
      /* but it's easier if we return sensible names such as f1, f2, etc
       * for the function keys, Insert, Delete, Home, End, Page Up, Page
       * Down, Up, Down, Right, and Left, and then pre-pend them with C-
       * for control, A- for alt, and S- for shift. If more than one
       * key is held down, the order is A-C-S- */
      result->strlength = 0;
      if (inchar & ALTKEY) {
         result->strlength = 2;
         memcpy(result->strptr, "A-", 2);
      }
      /* if there was an ascii value, that's all we need */
      if (inchar & 0xff) {
         result->strptr[result->strlength] = inchar & 0xff;
         result->strlength++;
      }
      /* otherwise, we need to add the control and shift indicators
       * and decode the scan code */
      else {
         if (inchar & CTLKEY) {
            memcpy(result->strptr+result->strlength, "C-", 2);
            result->strlength += 2;
         }
         if (inchar & SHIFTKEY) {
            memcpy(result->strptr+result->strlength, "S-", 2);
            result->strlength += 2;
         }

         key.code = (inchar & 0xff00) >> 8;
         res = bsearch(&key, scantbl, DIM(scantbl), sizeof(key), scancmp);

         if (res) {
            memcpy(result->strptr+result->strlength, res->name, res->len);
            result->strlength += res->len;
         }
         else {
            result->strlength += sprintf(result->strptr+result->strlength,
                                         "%03d", key.code);
         }
      }

#    endif
   }

   return 0;
}


/* systextscreenread(row,column, len) */
rxfunc(systextscreenread)
{
   CONSOLE_SCREEN_BUFFER_INFO csbi;
   CHAR_INFO *Buffer;
   char * row, *column, *len;
   COORD dwBufferSize, dwBufferCoord;
   SMALL_RECT ReadRegion;
   register int i, j, length;

   sethandles();

   checkparam(3,3);

   rxstrdup(row, argv[0]);
   rxstrdup(column, argv[1]);
   rxstrdup(len, argv[2]);
   length = atoi(len);

   rxresize(result, length);

   /* how big is the screen -- this is the max number of characters to read */
   GetConsoleScreenBufferInfo(c_output, &csbi);
   Buffer = alloca(csbi.dwSize.X * csbi.dwSize.Y * sizeof(*Buffer));

   dwBufferSize = csbi.dwSize;
   dwBufferCoord.X = 0;
   dwBufferCoord.Y = 0;

   /* we read in two bits. Bit 1 gives the current row, and bit 2 gives the
    * remaining part of the screen */
   ReadRegion.Left = atoi(column);
   ReadRegion.Right = csbi.dwSize.X;
   ReadRegion.Top = atoi(row);
   ReadRegion.Bottom = ReadRegion.Top;

   ReadConsoleOutput(c_output, Buffer, dwBufferSize, dwBufferCoord, &ReadRegion);

   for (i = j = 0; i < (ReadRegion.Right - ReadRegion.Left + 1) && j < length; i++,j++) {
      result->strptr[j] = Buffer[i].Char.AsciiChar;
   }
   if (j < length)
      result->strptr[j++] = '\n';


   if (j < length) {
      ReadRegion.Left = 0;
      ReadRegion.Top++;
      ReadRegion.Bottom = csbi.dwSize.Y;

      ReadConsoleOutput(c_output, Buffer, dwBufferSize, dwBufferCoord, &ReadRegion);

      for (i = 0; j < length && i < (ReadRegion.Right * (ReadRegion.Bottom-ReadRegion.Top)); i++,j++) {
         result->strptr[j] = Buffer[i].Char.AsciiChar;
         if (!((i+1) % (ReadRegion.Right+1)))
            result->strptr[++j] = '\n';
      }
   }

   result->strlength = j;


   return 0;
}


/* systextscreensize() */
rxfunc(systextscreensize)
{
   CONSOLE_SCREEN_BUFFER_INFO csbi;
   char * row, *column;
   COORD newpos;

   checkparam(0,0);

   sethandles();

   GetConsoleScreenBufferInfo(c_output, &csbi);
   result->strlength = sprintf(result->strptr, "%d %d", csbi.dwSize.Y, csbi.dwSize.X);

   return 0;
}
