/* DOS screen update functions for regutil
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
 * $Header: /opt/cvs/Regina/regutil/regscreendos.c,v 1.1 2022/08/21 23:15:58 mark Exp $
 */
#include "regutil.h"

/* ******************************************************************** */
/* ************************** Screen Update *************************** */
/* ******************************************************************** */

/* hold information about the screen */

static const char notimp[] = "not implemented";
#define what() memcpy(result->strptr, notimp, sizeof(notimp)-1), result->strlength = sizeof(notimp)-1

/* syscls() */
rxfunc(syscls)
{
   what();
   return 0;
}


/* syscurpos([row],[column]) */
rxfunc(syscurpos)
{
   what();
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
   int rc = 0;
   what();
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

/* sysgetkey([opt],[timeout]) */
rxfunc(sysgetkey)
{
   what();
   return 0;
}

rxfunc(sysgetline)
{
   what();
   return 0;
}

rxfunc(sysgetlinehistory)
{
   what();
   return 0;
}


/* systextscreenread(row,column, len) */
rxfunc(systextscreenread)
{
   what();
   return 0;
}


/* systextscreensize() */
rxfunc(systextscreensize)
{
   what();
   return 0;
}
