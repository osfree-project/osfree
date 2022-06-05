/* Macro space functions for regutil
 *  note that you must add -DMACROSPACE to the CFLAGS to get useful
 *  versions of these
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
 * $Header: /opt/cvs/Regina/regutil/regmacrospace.c,v 1.1 2009/10/07 07:51:51 mark Exp $
 */
#include "regutil.h"

#include <ctype.h>

/* ******************************************************************** */
/* ********************** Macro Space Management ********************** */
/* ******************************************************************** */


/* functions defined in this file:
 *  sysaddrexxmacro(name, file, [order])  [support not in Regina]
 *  sysclearrexxmacrospace()
 *  sysdroprexxmacro(name)
 *  sysloadrexxmacrospace(file)
 *  sysqueryrexxmacro(name)
 *  sysreorderrexxmacro(name,order)
 *  syssaverexxmacrospace(file)
 */

#ifdef MACROSPACE

/* sysaddrexxmacro(name, file [, order]) -- add a an external macro into the
 * current macro space. The macro can be added to the front or back of the
 * search order, allowing it to be over-ridden. */
rxfunc(sysaddrexxmacro)
{
   char * fn, * file;
   unsigned long flags = RXMACRO_SEARCH_BEFORE;

   checkparam(2,3);
   rxstrdup(fn, argv[0]);
   rxstrdup(file, argv[1]);

   if (argc > 2 && tolower(argv[2].strptr[0]) == 'a') {
      flags = RXMACRO_SEARCH_AFTER;
   }

   result->strlength = sprintf(result->strptr, "%d", RexxAddMacro(fn, file, flags));

   return 0;
}

/* clears all definitions from the macro space */
rxfunc(sysclearrexxmacrospace)
{
   checkparam(0, 0);

   result->strlength = sprintf(result->strptr, "%d", RexxClearMacroSpace());
   return 0;
}

/* sysdroprexxmacro(name) removes the named macro from the macro space */
rxfunc(sysdroprexxmacro)
{
   char * fn;
   checkparam(1, 1);

   rxstrdup(fn, argv[0]);

   result->strlength = sprintf(result->strptr, "%d", RexxDropMacro(fn));
   return 0;
} 

/* sysloadrexxmacrospace(file) -- load a macro space from a previously saved file */
rxfunc(sysloadrexxmacrospace)
{
   char * file;

   checkparam(1,1);

   rxstrdup(file, argv[0]);

   result->strlength = sprintf(result->strptr, "%d", RexxLoadMacroSpace(0, NULL, file));
   return 0;
} 

/* sysqueryrexxmacro(name) -- determine whether a macro has been loaded (laden?) */
rxfunc(sysqueryrexxmacro)
{
   static const char bef[] = "Before", aft[] = "After";
   char * fn;
   unsigned short pos = 0;
   int rc;

   checkparam(1,1);

   rxstrdup(fn, argv[0]);

   rc = RexxQueryMacro(fn, &pos);

   if (pos == RXMACRO_SEARCH_AFTER) {
      memcpy(result->strptr, aft, sizeof(aft)-1);
      result->strlength = sizeof(aft)-1;
   }
   else if (pos == RXMACRO_SEARCH_BEFORE) {
      memcpy(result->strptr, bef, sizeof(bef)-1);
      result->strlength = sizeof(bef)-1;
   }
   else
      result->strlength = 0;

   return 0;
} 


/* sysreorderrexxmacro(name,order) -- move the macro to the start or the end
 * of the search order */
rxfunc(sysreorderrexxmacro)
{
   char * fn, * file;
   unsigned long flags;

   checkparam(2,2);
   rxstrdup(fn, argv[0]);

   if (tolower(argv[1].strptr[0]) == 'a') {
      flags = RXMACRO_SEARCH_AFTER;
   }
   else
      flags = RXMACRO_SEARCH_BEFORE;

   result->strlength = sprintf(result->strptr, "%d", RexxReorderMacro(fn, flags));

   return 0;
} 

/* syssaverexxmacrospace(file) -- writes the macro space out to a file */
rxfunc(syssaverexxmacrospace)
{
   char * file;

   checkparam(1,1);

   rxstrdup(file, argv[0]);

   result->strlength = sprintf(result->strptr, "%d", RexxSaveMacroSpace(0, NULL, file));
   return 0;
} 

#else

static const char notimp[] = "Macrospace Functions are not compiled. Add -DMACROSPACE to CFLAGS and recompile the library.";

#define STUB(x) rxfunc(x) { result->strlength = sizeof(notimp) - 1; memcpy(result->strptr, notimp, result->strlength); return 0; }

STUB(sysaddrexxmacro)
STUB(sysclearrexxmacrospace)
STUB(sysdroprexxmacro)
STUB(sysloadrexxmacrospace)
STUB(sysqueryrexxmacro)
STUB(sysreorderrexxmacro)
STUB(syssaverexxmacrospace)
#endif
