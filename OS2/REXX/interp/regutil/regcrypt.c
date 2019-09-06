/* Decrypt/Encrypt functions for regutil
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
 * Portions created by Patrick McPhee are Copyright © 2003
 * Patrick TJ McPhee. All Rights Reserved.
 *
 * Contributors:
 *
 * $Header: /opt/cvs/Regina/regutil/regcrypt.c,v 1.1 2009/10/07 07:51:45 mark Exp $
 */
#include "rxproto.h"

/* Currently, this is implemented as a stub. I'm going to think about it a
 * bit more before I decide whether to proceed with an implementation.
 * I have issues because
 *  1. the interface implies purely trivial encryption of some sort;
 *  2. which nonetheless could result in the original data being destroyed;
 *  3. and likely wouldn't be portable between systems.
 */

static const char failure[] = "82";


/* rc = SysWinEncryptFile(filename) */
rxfunc(syswinencryptfile)
{
   checkparam(1, 1);

   memcpy(result->strptr, failure, sizeof(failure)-1);
   result->strlength = sizeof(failure)-1;

   return 0;
}

rxfunc(syswindecryptfile)
{
   checkparam(1, 1);

   memcpy(result->strptr, failure, sizeof(failure)-1);
   result->strlength = sizeof(failure)-1;

   return 0;
}
