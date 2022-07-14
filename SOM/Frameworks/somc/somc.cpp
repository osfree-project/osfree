/**************************************************************************
 *
 *  Copyright 2022, Yuri Prokushev
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


#include <emitlib.h>


__declspec(dllexport) FILE * SOMLINK somtopenEmitFileSL (char *file, char *ext)
{
  // Здесь необходимо предварительно добавить или заменить расширение на ext
  return fopen(file, "w");
};

#ifndef SOM_SOMC_NO_BACKCOMPAT
// Under Win32 this is __fastcall?
__declspec(dllexport) FILE * somtopenEmitFile (char *file, char *ext)
{
  return somtopenEmitFileSL (file, ext);
};
#endif

__declspec(dllexport) int SOMLINK somtfcloseSL (FILE *fp)
{
  return fclose(fp);
}

#ifndef SOM_SOMC_NO_BACKCOMPAT
// Under Win32 this is __fastcall?
extern "C" __declspec(dllexport) int somtfclose (FILE *fp)
{
  return somtfcloseSL (fp);
}
#endif
