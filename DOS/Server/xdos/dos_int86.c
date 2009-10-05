/*    
	dos_int86.c	1.12
    	Copyright 1997 Willows Software, Inc. 

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.


For more information about the Willows Twin Libraries.

	http://www.willows.com	

To send email to the maintainer of the Willows Twin Libraries.

	mailto:twin@willows.com 

 */

#include	"windows.h"

#include	"Log.h"
#include	"kerndef.h"
#include	"BinTypes.h"
#include	"xdos.h"
#include	"xdosproc.h"

BINADDR make_binary_thunk(LPDWORD, DWORD);
DWORD make_native_thunk(DWORD, DWORD);
void invoke_binary(void);

#ifdef LATER
	Allocate space, rather than hard code this...
	Should we also map to user space?
#endif
void (*IVT[256])( int, ENV * );

void
IVTInit()
{
	int i;
	ENV env;

	for ( i = 0;  i < 256;  i++ )
	      IVT[i] = INTTrap;

	env.reg.ax = 0x2510;
	SetVector( (DWORD)int_10, &env );
	env.reg.ax = 0x2511;
	SetVector( (DWORD)int_11, &env );
	env.reg.ax = 0x2512;
	SetVector( (DWORD)int_12, &env );
	env.reg.ax = 0x2513;
	SetVector( (DWORD)int_13, &env );
	env.reg.ax = 0x2515;
	SetVector( (DWORD)int_15, &env );
	env.reg.ax = 0x251a;
	SetVector( (DWORD)int_1a, &env );
	env.reg.ax = 0x2521;
	SetVector( (DWORD)int_21, &env );
	env.reg.ax = 0x2525;
	SetVector( (DWORD)int_25, &env );
	env.reg.ax = 0x252a;
	SetVector( (DWORD)int_2a, &env );
	env.reg.ax = 0x252f;
	SetVector( (DWORD)int_2f, &env );
	env.reg.ax = 0x2531;
	SetVector( (DWORD)int_31, &env );
	env.reg.ax = 0x253d;
	SetVector( (DWORD)int_3d, &env );
	env.reg.ax = 0x255c;
	SetVector( (DWORD)int_5c, &env );
}


void
hsw_int_bin_to_nat( ENV *env, LONGPROC int_handler )
{
	(* int_handler)( env );
}


void
hsw_int_nat_to_bin( ENV *env )
{
	invoke_binary();
}


/*
 *	The address of this function is provided when INT 21h Fn 35h
 *	is issued and appropriate entry in IVT equals INTTrap.
 */
static void
INTStub( int i, ENV *p )
{
}


void
INT_handler( int nInt, ENV *env )
{
        LOGSTR((LF_INTCALL,"INT[%x]: AX=%x BX=%x CX=%x DX=%x\n",
		nInt, env->reg.ax, env->reg.bx, env->reg.cx,env->reg.dx));

	(*IVT[nInt])( nInt, env ); 

        LOGSTR((LF_INTRET,"INT[%x]: AX=%x BX=%x CX=%x DX=%x FLAG=%x\n",
		nInt, env->reg.ax, env->reg.bx, env->reg.cx,env->reg.dx,env->reg.flags));

	env->reg.sp += WORD_86 + RET_86;
}


DWORD
GetVector( int nInt )
{
	void (*lpfnEntry)( int, ENV * );

	if ( IVT[nInt] == INTTrap )
	     lpfnEntry = INTStub;
	else
	     lpfnEntry = IVT[nInt];

	return ((DWORD)make_binary_thunk( (LPDWORD)lpfnEntry,
					  (DWORD)hsw_int_bin_to_nat ));
}


void
SetVector( DWORD handler, ENV *env )
{
	DWORD lpfnHandler;


	if ( handler )
	     /*  Set native handler  */
	     lpfnHandler = handler;
	else
	     /*  Set binary handler  */
	     lpfnHandler = (DWORD)make_native_thunk(
					MAKELONG(env->reg.dx, env->reg.ds),
					(DWORD)hsw_int_nat_to_bin );

	IVT[LOBYTE(LOWORD(env->reg.ax))] = (void (*)(int, ENV *))lpfnHandler;
} 


void
INTTrap( int nInt, ENV *env )
{
        char buf[50];

        wsprintf( buf, "TRAP:  Unsupported INT %x   AH = %.2x", 
		  nInt, (env->reg.ax & 0xff00)>>8 );
        MessageBox( (HWND)NULL, (LPCSTR)buf, (LPCSTR)"TRAP", MB_OKCANCEL);
}

