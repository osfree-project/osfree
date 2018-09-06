/* win 3.1-style ini-file-reading code for unix
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
 * Portions created by Patrick McPhee are Copyright © 2002
 * Patrick TJ McPhee. All Rights Reserved.
 *
 * Contributors:
 *
 * $Header: /opt/cvs/Regina/regutil/inifile.h,v 1.1 2009/10/07 07:51:41 mark Exp $
 */
#ifndef _INIFILE_H
#define _INIFILE_H

typedef struct inif_T * inif_t;

inif_t ini_open(const char * name);
void ini_close(inif_t);

char * ini_get_val(inif_t fit, const char * secname, const char * valname);
void ini_set_val(inif_t fit, const char * secname, const char * valname, const char * value);
void ini_del_val(inif_t fit, const char * secname, const char * valname);
char ** ini_enum_val(inif_t fit, const char * secname, int * pcount);
void ini_del_sec(inif_t fit, const char * secname);
char ** ini_enum_sec(inif_t fit, int * pcount);

#endif
