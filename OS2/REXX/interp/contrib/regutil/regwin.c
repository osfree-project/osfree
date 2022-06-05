/* NT Message Box and Exec functions for regutil
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
 * Portions created by Patrick McPhee are Copyright © 2001
 * Patrick TJ McPhee. All Rights Reserved.
 *
 * Contributors:
 *
 * $Header: /opt/cvs/Regina/regutil/regwin.c,v 1.1 2009/10/07 07:52:21 mark Exp $
 */

#include "rxproto.h"

/* rxmessagebox(text, [title], [button], [icon]) */

rxfunc(rxmessagebox)
{
   char * text, *but = NULL, *ic = NULL;
   char * title = "Error!";
   int button = MB_OK;
   int icon = MB_ICONHAND;
   register int i;
   static const struct {
      char * text;
      int value;
   } bs[] = {
      "OK", MB_OK,
      "OKCANCEL", MB_OKCANCEL,
      "ABORTRETRYIGNORE", MB_ABORTRETRYIGNORE,
      "YESNOCANCEL", MB_YESNOCANCEL,
      "YESNO", MB_YESNO,
      "RETRYCANCEL", MB_RETRYCANCEL,
   }, is[] = {
      "HAND", MB_ICONHAND,
      "QUESTION", MB_ICONQUESTION,
      "EXCLAMATION", MB_ICONEXCLAMATION,
      "ASTERISK", MB_ICONASTERISK,
      "INFORMATION", MB_ICONINFORMATION,
      "STOP", MB_ICONSTOP,
   };

   checkparam(1,4);

   rxstrdup(text, argv[0]);
   if (argc > 1 && argv[1].strptr) {
      rxstrdup(title, argv[1]);
   }
   if (argc > 2 && argv[2].strptr) {
      rxstrdup(but, argv[2]);
   }
   if (argc > 3 && argv[3].strptr) {
      rxstrdup(ic, argv[3]);
   }

   if (ic) {
      strupr(ic);
      for (i = 0; i < DIM(is); i++) {
         if (!strcmp(ic, is[i].text)) {
            icon = is[i].value;
            break;
         }
      }
   }

   if (but) {
      strupr(but);
      for (i = 0; i < DIM(bs); i++) {
         if (!strcmp(but, bs[i].text)) {
            button = bs[i].value;
            break;
         }
      }
   }

   result->strlength = sprintf(result->strptr, "%d",
      MessageBox(NULL, text, title, icon|button));

   return 0;
}


/* call winexec with the specified arguments. The same effect can be had
 * using the start command
 * rc = rxwinexec(cmd[, flags])
 */
rxfunc(rxwinexec)
{
   char * cmd, *sflags = NULL;
   int flags = SW_SHOWNORMAL;
   register int i;
   static const struct {
      char * text;
      int value;
   } fs[] = {
      "SHOWNORMAL", SW_SHOWNORMAL,
      "SHOWNOACTIVATE", SW_SHOWNOACTIVATE,
      "SHOWMINNOACTIVE", SW_SHOWMINNOACTIVE,
      "SHOWMINIMIZED", SW_SHOWMINIMIZED,
      "SHOWMAXIMIZED", SW_SHOWMAXIMIZED,
      "HIDE", SW_HIDE,
      "MINIMIZE", SW_MINIMIZE
   };

   checkparam(1,2);

   rxstrdup(cmd, argv[0]);
   if (argc > 1 && argv[1].strptr) {
      rxstrdup(sflags, argv[1]);
   }

   if (sflags) {
      strupr(sflags);
      for (i = 0; i < DIM(fs); i++) {
         if (!strcmp(sflags, fs[i].text)) {
            flags = fs[i].value;
            break;
         }
      }
   }

   /* return the rc of winexec */
   result->strlength = sprintf(result->strptr, "%d", WinExec(cmd, flags));

   return 0;
}
