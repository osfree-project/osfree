/**************************************************************************
 *
 *  Copyright 2014, 2017 Yuri Prokushev
 *
 *  This file is part of osFree project
 *
 *  This program is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU Lesser General Public License as published by the
 *  Free Software Foundation, either version 3 of the License, or (at your
 *  option) any later version.
 * 
 *  This program is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 *  more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>
 *
 */

#include <sm.h>
#include <sctypes.h>

#include <scentry.xh>
#include <scclass.xh>
#include <emitlib.h>

SOMTEntryC * SOMLINK somtGetObjectWrapper(Entry * ep)
{
  SOMTClassEntryC * oCls;
  oCls=new SOMTClassEntryC();
  
  oCls->somtSetEntryStruct(ep);

  return oCls;
}

int SOMLINK somtfclose (FILE *fp)
{
  return fclose(fp);
}

char * SOMLINK somtStrCat(int count,...)
{
  return NULL;
}

void SOMLINK somtShowEntry(Entry * ep)
{
  
}

char * SOMLINK somtStrDup(char *str)
{
  return NULL;
}

char * SOMLINK somtMakeIncludeStr(boolean local, char *stem, char *suffix)
{
  return NULL;
}

char * SOMLINK somtNewSymbol(char *prefix, char *stem)
{
  return NULL;
}

char * SOMLINK somtGetFileStem(char *fullName)
{
  return NULL;
}

char * SOMLINK somtEntryTypeName(SOMTTypes type)
{
  return NULL;
}
