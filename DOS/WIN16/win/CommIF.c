/*    
	CommIF.c	2.7
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

#include <string.h>

#undef NOCOMM
#include "windows.h"

#include "kerndef.h"
#include "BinTypes.h"
#include "Kernel.h"
#include "Comm.h"
#include "Log.h"
#include "Endian.h"
#include "DPMI.h"


static void
GetDCB(LPDCB lpdcb, LPBYTE lpbin)
{
	BYTE	byte;

	memset(lpdcb,0,sizeof(DCB));

	lpdcb->Id 	= *lpbin;
	lpdcb->BaudRate = GETWORD(lpbin+1);
	lpdcb->ByteSize = *(lpbin+3);
	lpdcb->Parity 	= *(lpbin+4);
	lpdcb->StopBits	= *(lpbin+5);
    	lpdcb->RlsTimeout = GETWORD(lpbin+6);
    	lpdcb->CtsTimeout = GETWORD(lpbin+8);
    	lpdcb->DsrTimeout= GETWORD(lpbin+10);

	byte = *(lpbin + 12);
	if(byte & 1)	lpdcb->fBinary      = 1;
	if(byte & 2)	lpdcb->fRtsDisable  = 1;
	if(byte & 4)	lpdcb->fParity 	    = 1;
	if(byte & 8)	lpdcb->fOutxCtsFlow = 1;
	if(byte & 0x10)	lpdcb->fOutxDsrFlow = 1;
	if(byte & 0x20)	lpdcb->fDummy       = 1;
	if(byte & 0x40)	lpdcb->fDtrDisable  = 1;

	byte = *(lpbin + 13);
	if(byte & 1)	lpdcb->fOutX    = 1;
	if(byte & 2)	lpdcb->fInX     = 1;
	if(byte & 4)	lpdcb->fPeChar 	= 1;
	if(byte & 8)	lpdcb->fNull    = 1;
	if(byte & 0x10)	lpdcb->fChEvt   = 1;
	if(byte & 0x20)	lpdcb->fDtrflow = 1;
	if(byte & 0x40)	lpdcb->fRtsflow = 1;
	if(byte & 0x80)	lpdcb->fDummy2  = 1;
	
	lpdcb->XonChar  = *(lpbin+14);
	lpdcb->XoffChar = *(lpbin+15);
	lpdcb->XonLim   = GETWORD(lpbin+16);
	lpdcb->XoffLim  = GETWORD(lpbin+18);
	lpdcb->PeChar   = *(lpbin+20);
	lpdcb->EofChar  = *(lpbin+21);
	lpdcb->EvtChar  = *(lpbin+22);
	lpdcb->TxDelay  = GETWORD(lpbin+23);
}

static void
PutDCB(LPBYTE lpbin, LPDCB lpdcb)
{
	BYTE byte;

	*lpbin = lpdcb->Id;
	PUTWORD(lpbin+1, lpdcb->BaudRate);
	*(lpbin + 3)     = lpdcb->ByteSize;
	*(lpbin + 4)     = lpdcb->Parity;
	*(lpbin + 5)     = lpdcb->StopBits;
	PUTWORD(lpbin+6, lpdcb->RlsTimeout);
	PUTWORD(lpbin+8, lpdcb->CtsTimeout);
	PUTWORD(lpbin+10,lpdcb->DsrTimeout);

	byte = 0;
	if(lpdcb->fBinary)      byte |=   1;
	if(lpdcb->fRtsDisable)  byte |=   2;
	if(lpdcb->fParity) 	byte |=   4;
	if(lpdcb->fOutxCtsFlow) byte |=   8;
	if(lpdcb->fOutxDsrFlow) byte |= 0x10;
	if(lpdcb->fDummy)       byte |= 0x20;
	if(lpdcb->fDtrDisable)  byte |= 0x40;
	*(lpbin + 12) = byte;

	byte = 0;
	if(lpdcb->fOutX)    byte |= 1;
	if(lpdcb->fInX)     byte |= 2;
	if(lpdcb->fPeChar)  byte |= 4;
	if(lpdcb->fNull)    byte |= 8;
	if(lpdcb->fChEvt)   byte |= 0x10;
	if(lpdcb->fDtrflow) byte |= 0x20;
	if(lpdcb->fRtsflow) byte |= 0x40;
	if(lpdcb->fDummy2)  byte |= 0x80;
	*(lpbin + 13) =  byte;
	
	*(lpbin+14) = lpdcb->XonChar;
	*(lpbin+15) = lpdcb->XoffChar;
	PUTWORD(lpbin+16, lpdcb->XonLim);
	PUTWORD(lpbin+18, lpdcb->XoffLim);
	*(lpbin+20) = lpdcb->PeChar;
	*(lpbin+21) = lpdcb->EofChar;
	*(lpbin+22) = lpdcb->EvtChar;
	PUTWORD(lpbin+23, lpdcb->TxDelay);
}


static void
GetCOMSTAT(COMSTAT *lpCS, LPBYTE lpbin)
{
	BYTE byte;
	memset(lpCS,0,sizeof(COMSTAT));

	if(lpbin) {
		byte = *(lpbin);
#ifdef NOTDEFINED
		if(byte & 1)	lpCS->fCtsHold   = 1;
		if(byte & 2)	lpCS->fDsrHold   = 1;
		if(byte & 4)	lpCS->fRlsdHold  = 1;
		if(byte & 8)	lpCS->fXoffHold  = 1;
		if(byte & 0x10)	lpCS->fXoffSent  = 1;
		if(byte & 0x20)	lpCS->fEof       = 1;
		if(byte & 0x40)	lpCS->fTxim      = 1;
#endif
		lpCS->status = byte;

		lpCS->cbInQue = GETWORD(lpbin+1);
		lpCS->cbOutQue = GETWORD(lpbin+3);
	}
}


static void
PutCOMSTAT(LPBYTE lpbin, COMSTAT *lpCS)
{
	BYTE byte;
	memset(lpCS,0,sizeof(COMSTAT));

	if(lpbin) {
		byte = 0;
#ifdef NOTDEFINED
		if(lpCS->fCtsHold)   byte = 1;
		if(lpCS->fDsrHold)   byte = 2;
		if(lpCS->fRlsdHold)  byte = 4;
		if(lpCS->fXoffHold)  byte = 8;
		if(lpCS->fXoffSent)  byte = 0x10;
		if(lpCS->fEof)       byte = 0x20;
		if(lpCS->fTxim)      byte = 0x40;
#endif
		byte = lpCS->status;
		*(lpbin) = byte;

		PUTWORD(lpbin+1, lpCS->cbInQue);
		PUTWORD(lpbin+3, lpCS->cbOutQue);
	}
}


void
IT_1LPDCB(ENV *envp, LONGPROC f)
{
	DWORD	rc;
	LPBYTE	lpStruct;
	DCB	dcb;

	lpStruct = (LPBYTE)GetAddress(GETWORD(SP+6),GETWORD(SP+4));
	GetDCB(&dcb, lpStruct);
	rc = f(&dcb);
	if ( !rc )
		PutDCB(lpStruct, &dcb);
	envp->reg.sp += LP_86;
	envp->reg.ax = LOWORD(rc);
	envp->reg.dx = HIWORD(rc);
}


void
IT_SETQUE(ENV *envp, LONGPROC f)
{
	DWORD	rc;

	rc = f();
	envp->reg.sp += INT_86 + LP_86;
	envp->reg.ax = LOWORD(rc);
	envp->reg.dx = HIWORD(rc);
}


void
IT_SETCOM(ENV *envp, LONGPROC f)
{
	DWORD	rc;
	LPBYTE	lpStruct;
	DCB	dcb;

	lpStruct = (LPBYTE)GetAddress(GETWORD(SP+6),GETWORD(SP+4));
	GetDCB(&dcb, lpStruct);
	rc = f(&dcb);
	if ( !rc )
		PutDCB(lpStruct, &dcb);
	envp->reg.sp += LP_86;
	envp->reg.ax = LOWORD(rc);
	envp->reg.dx = HIWORD(rc);
}

/* readcom writecom */
void
IT_1I1LPV1I(ENV *envp, LONGPROC f)
{
	int	rc;
	LPVOID	lpData;

	lpData = (LPSTR)GetAddress(GETWORD(SP+8),GETWORD(SP+6));
	rc = f(GETSHORT(SP+10), lpData, GETSHORT(SP+4));
	envp->reg.sp += 2*INT_86 + LP_86;
	envp->reg.ax = LOWORD(rc);
	envp->reg.dx = HIWORD(rc);
}


/* enable notification and enablecommnotification */
void
IT_1I1H2I(ENV *envp, LONGPROC f)
{
	DWORD	rc;

	rc = f(GETSHORT(SP+10), (HWND)GETWORD(SP+8), GETWORD(SP+6),
		GETWORD(SP+4));
	envp->reg.sp += HANDLE_86 + 2*WORD_86 + INT_86;
	envp->reg.ax = LOWORD(rc);
	envp->reg.dx = HIWORD(rc);
}


/* opencomm */
void
IT_1LP2UI(ENV *envp, LONGPROC f)
{
	DWORD	rc;
	LPVOID	lpData;

	lpData = (LPSTR)GetAddress(GETWORD(SP+10),GETWORD(SP+8));
	rc = f(lpData, GETWORD(SP+6), GETWORD(SP+4));
	envp->reg.sp += LP_86 + 2*UINT_86;
	envp->reg.ax = LOWORD(rc);
	envp->reg.dx = HIWORD(rc);
}


/* getcommstate */
void
IT_1I1LPDCB(ENV *envp, LONGPROC f)
{
	DWORD	rc;
	LPVOID	lpData;
	DCB	dcb;

	lpData = (LPSTR)GetAddress(GETWORD(SP+6),GETWORD(SP+4));
	rc = f(GETSHORT(SP+8), &dcb);
	if(rc == 0)
		PutDCB(lpData,&dcb);

	envp->reg.sp += LP_86 + INT_86;
	envp->reg.ax = LOWORD(rc);
	envp->reg.dx = HIWORD(rc);
}

/* stacom getcommerror */
void
IT_1I1LPCOMSTAT(ENV *envp, LONGPROC f)
{
	DWORD	rc;
	LPVOID	lpData;
	COMSTAT comstat;

	lpData = (LPSTR)GetAddress(GETWORD(SP+6),GETWORD(SP+4));
	GetCOMSTAT(&comstat, lpData);
	rc = f(GETSHORT(SP+8), &comstat);
	PutCOMSTAT(lpData, &comstat);
	envp->reg.sp += LP_86 + INT_86;
	envp->reg.ax = LOWORD(rc);
	envp->reg.dx = HIWORD(rc);
}


/* setcommeventmask */
void
IT_1I1UI(ENV *envp, LONGPROC f)
{
	DWORD	rc;

	rc = f(GETSHORT(SP+6), GETWORD(SP+4));
	envp->reg.sp += INT_86 + UINT_86;
	envp->reg.ax = LOWORD(rc);
	envp->reg.dx = HIWORD(rc);
}
