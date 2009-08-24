/*
    ASSIGN: Replace drive letters
    Copyright (C) 1995-97 Steffen Kaiser

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 1, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
/*
    Template for local message declaration file for msgComp
    Copyright (C) 1995 Steffen Kaiser

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 1, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/


/* $Id: YERROR.H 1.1 1997/01/27 08:09:37 ska Rel ska $
   $Locker: ska $	$Name:  $	$State: Rel $

   	Applic‘tion-private messages.

*/

#ifndef __DCL_MSGOR_H
#define __DCL_MSGOR_H

#include <msglib.h>

DCL_MSG(I_locMsgFile);
DCL_MSG(E_hlpScreen);
DCL_MSG(E_assignData);
DCL_MSG(W_mayhapInstalled);
DCL_MSG(W_notInstalled);
DCL_MSG(M_assignment);
DCL_MSG(E_mcbChain);
DCL_MSG(E_loadedModule);
DCL_MSG(E_notOnTop);
DCL_MSG(E_releaseBlock);
DCL_MSG(M_installByte);
DCL_MSG(M_inRecurs);
DCL_MSG(E_recursion);
DCL_MSG(E_lowDOSVers);
DCL_MSG(W_highDOSVers);

#endif
