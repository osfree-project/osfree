/*    
	Exec.c	2.24
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

#include <stdio.h>
#include <signal.h>
#include <setjmp.h>
#include "windows.h"

#include "DPMI.h"
#include "kerndef.h"		/* BINADDR */
#include "BinTypes.h"		/* ENV structure */
#include "Log.h"
#include "dos.h"
#include "Exec.h"
#include "Log.h"

#define CARRY_FLAG		0x0001

#define INITLDTSIZE 0x300

#define	IDT_FAULT	0x2
#define	LDT_FAULT	0x4

/* exported for the outside world */
BOOL DPMIInit();
BOOL DPMI_Notify(UINT,WORD);
WORD native_cs,native_ss,native_ds;
extern DSCR *LDT;
extern int nLDTSize;

/* internal functions */
static void DumpRegisters(struct TWIN_i386_context_s *);
static void HandleInterrupt(UINT, struct TWIN_i386_context_s *);

/* internal variables */

static BYTE return_gate[8] =
	{ 0x66,		/* operand override prefix */
	  0x9a,		/* far call */
	  0,0,0,0,	/* 32-bit target offset */
	  0,0		/* 16-bit code selector */
	};

static BYTE native_gate[8] =
	{ 0x66,		/* operand override prefix */
	  0x9a,		/* far call */
	  0,0,0,0,	/* 32-bit target offset */
	  0,0		/* 16-bit code selector */
	};

/* external functions */
extern BYTE read_seg(DWORD, DWORD);
extern BOOL LoadSegment(UINT);
extern void invoke_native(void);
extern void return_to_native(void);
extern DWORD get_sel_limit(UINT);
extern BOOL InitThunks(void);
extern REGISTER PortIO(DWORD, DWORD, UINT, BOOL);
extern WORD get_native_cs();
extern WORD get_native_ds();
extern WORD get_native_ss();

/* external variables */
extern int nLDTSize;

BOOL 
DPMI_Notify(UINT uAction,WORD wSel)
{
	struct ssd req;
	DWORD   dwSize;
	DWORD	dwBase;
	WORD    wFlags;
	WORD	wType;

	int 	   ret = 0;
	int	nNumEntries,i;
	LPSTR   lpBuf;
	
	if(uAction == DN_INIT)
		return DPMIInit();

	dwBase = (DWORD) GetPhysicalAddress(wSel);
	dwSize = GetSelectorLimit(wSel);
	wFlags = GetSelectorFlags(wSel);	
	wType =  GetSelectorType(wSel);	

	/* Selectors allocated via AllocSelector etc. have base == -2 */
	if (dwBase == (DWORD) -1)
	    return FALSE;

	switch(uAction) {
		case DN_FREE:
			req.sel = wSel;
			req.bo =   0;
			req.ls =   0;
			req.acc1 = 0;
			req.acc2 = 0;
			ret =  sysi86(SI86DSCR, &req);
			break;
		case DN_ASSIGN:
		case DN_MODIFY:
			req.sel =  wSel;
			req.bo =  (unsigned int) dwBase;
			req.ls =  (unsigned int) dwSize;
			req.acc1 = LOBYTE(wFlags);
			req.acc2 = HIBYTE(wFlags);
			ret =  sysi86(SI86DSCR, &req);
			break;
	}
		
	if (ret)
		FatalAppExit(0,"DPMI failure\n");

	if (uAction == DN_ASSIGN) {
	    switch (wType) {
		case TRANSFER_RETURN:
		    lpBuf = (LPSTR)dwBase;
		    memcpy(lpBuf,(LPSTR)&return_gate[0],sizeof(return_gate));
		    *((LPDWORD)(lpBuf+sizeof(return_gate))) = native_ds;
		    break;

		case TRANSFER_CALLBACK:
		case TRANSFER_BINARY:
		    lpBuf = (LPSTR) dwBase+4;
		    *((LPDWORD)lpBuf) = native_ds;
		    nNumEntries = dwSize/sizeof(native_gate);
		    nNumEntries--;

		    lpBuf += 4;
		    for (i = 0; i < nNumEntries; i++) {
		         memcpy(lpBuf,native_gate,sizeof(native_gate));
			 lpBuf += sizeof(native_gate);
		    }
		    break;

		default:
		    return FALSE;
	    }
	}
	return TRUE;
}

void
LoadSignalHandlers()
{
	TWIN_RegisterGPFSignal(SIGILL);
	TWIN_RegisterGPFSignal(SIGSEGV);
	TWIN_RegisterGPFSignal(SIGFPE);

#ifdef DEBUG
	logstr(-1,"Twin Debugger enabled...\n");

	TWIN_RegisterDebugSignal(SIGINT);
	TWIN_RegisterDebugSignal(SIGTRAP);

#endif
}

BOOL
DPMIInit()
{
	if (!(LDT = (DSCR *)WinMalloc(INITLDTSIZE*sizeof(DSCR)))) {
		FatalAppExit(0,"DPMI: Cannot alloc memory for LDT\n");
	}

	memset((LPSTR)LDT,'\0',INITLDTSIZE*sizeof(DSCR));
	nLDTSize = INITLDTSIZE;
	SetPhysicalAddress(0,(LPBYTE)(-1));		/* sel=0 is illegal */
    	SetPhysicalAddress(0xf,(LPBYTE)(-1));
    	SetPhysicalAddress(0x17,(LPBYTE)(-1));
    	SetPhysicalAddress(0x1f,(LPBYTE)(-1));

	native_cs = get_native_cs();
	native_ds = get_native_ds();
	native_ss = get_native_ss();

    	SetPhysicalAddress(native_cs,(LPBYTE)(-1));
    	SetPhysicalAddress(native_ds,(LPBYTE)(-1));
    	SetPhysicalAddress(native_ss,(LPBYTE)(-1));

	LoadSignalHandlers();

	*(LPWORD)(&native_gate[6]) = native_cs;
	*(LPDWORD)(&native_gate[2]) = (DWORD)invoke_native;

	*(LPWORD)(&return_gate[6]) = native_cs;
	*(LPDWORD)(&return_gate[2]) = (DWORD)return_to_native;

	if (!InitThunks())
	    FatalAppExit(0,"DPMI: Could not init thunks\n");

	DEBUG_INIT();
	
	return TRUE;
}

void
TWIN_GPFHandler(int sig,struct TWIN_i386_context_s *sc)
{
	int 	limit;
	UINT	intcode;
	int	handled;
	UINT uSel;
	static 	int fault;

	if (!sc) {
	    logstr(LF_ERROR,"no signal context\n");
	    ExitWindows(-1,1);
	}

	if (fault > 0) {
	    logstr(LF_ERROR,"double fault\n");	/* faulted in handler?    */
	    ExitWindows(-1,1);		/* do nothing, just exit  */
	}

	fault++;
	handled = 0;

	uSel = (UINT)(sc->err & 0xffff);
	if (uSel) {
	    if (uSel & LDT_FAULT) {
		intcode = uSel & ~3;
		if (intcode == native_cs || intcode == native_ds ||
			intcode == native_ss || intcode == 0) {
		    ERRSTR((LF_ERROR,"General Protection Fault: %4.4x:%4.4x\n",
	    			sc->cs,sc->eip));
		    DumpRegisters(sc);
		    limit = (sc->cs)?get_sel_limit(sc->cs):0;
		    ERRSTR((LF_ERROR,"lsl(%x) = %x err=%x\n",
				sc->cs,limit,sc->err));
		    ExitWindows(-1,1);
		}
		if (LoadSegment(uSel)) {
		    fault = 0;
		    return;
		}
		else {
		    printf("Unable to load segment %x\n",uSel);
		    DumpRegisters(sc);
		    DEBUG_TASK(sig, sc);
		    ExitWindows(-1,1);
		}
	    }
	    else if (uSel & IDT_FAULT) {
		intcode = uSel >> 3;
		HandleInterrupt(intcode, sc);
		fault = 0;
		return;
	    }
	}

	intcode = read_seg( sc->cs, sc->eip);
	switch(intcode) {
	/* HLT, CLI and STI come here because of the kernel change */
	/* INs and OUTs at present may generate SIGSEGV */

#ifdef	LATER
	/* we have to get an operand size to support 32-bit IN/OUT */
#endif
		case 0xe4:	/* IN AL,imm8 */
		    sc->eax = ((sc->eax & 0xffffff00) |
			      (PortIO(read_seg(sc->cs,sc->eip+1) & 0xff,
				      0, 8, FALSE))) & 0xff;
		    sc->eip += 2;
		    handled++;
		    break;
		case 0xe5:	/* IN AX,imm8 */
		    sc->eax = ((sc->eax & 0xffff0000) |
			      (PortIO(read_seg(sc->cs,sc->eip+1) & 0xff,
				      0, 16, FALSE))) & 0xffff;
		    sc->eip += 2;
		    handled++;
		    break;
		case 0xe6:	/* OUT AL,imm8 */
		    PortIO(read_seg(sc->cs,sc->eip+1) & 0xff,
			(sc->eax & 0xff), 8, TRUE);
		    sc->eip += 2;
		    handled++;
		    break;
		case 0xe7:	/* OUT AX,imm8 */
		    PortIO(read_seg(sc->cs,sc->eip+1) & 0xff,
			(sc->eax & 0xffff), 16, TRUE);
		    sc->eip += 2;
		    handled++;
		    break;
		case 0xec:	/* IN AL,DX */
		    sc->eax = ((sc->eax & 0xffffff00) |
			      (PortIO(sc->edx & 0xffff, 0, 8, FALSE))) & 0xff;
		    sc->eip++;
		    handled++;
		    break;
		case 0xed:	/* IN AX,DX */
		    sc->eax = ((sc->eax & 0xffff0000) |
			      (PortIO(sc->edx & 0xffff,0,16,FALSE))) & 0xffff;
		    sc->eip++;
		    handled++;
		    break;
		case 0xee:	/* OUT AL,DX */
		    PortIO(sc->edx & 0xffff, (sc->eax & 0xff), 8, TRUE);
		    sc->eip++;
		    handled++;
		    break;
		case 0xef:	/* OUT AX,DX */
		    PortIO(sc->edx & 0xffff, (sc->eax & 0xffff), 16, TRUE);
		    sc->eip++;
		    handled++;
		    break;

		case 0xf4:	/* HLT */
		case 0xfa:	/* CLI */
		case 0xfb:	/* STI */
		    sc->eip++;
		    handled++;
		    break;

		default:
		    ERRSTR((LF_ERROR,"General Protection Fault: %4.4x:%4.4x\n",
	    			sc->cs,sc->eip));
		    DumpRegisters(sc);
		    limit = get_sel_limit(sc->cs);
		    ERRSTR((LF_ERROR,"lsl(%x) = %x err=%x\n",
			sc->cs,limit,sc->err));
		    ERRSTR((LF_ERROR,"opcode = %x\n",intcode));
		
		    DEBUG_TASK(sig, sc);

		    ExitWindows(-1,1);
	}	

	if (handled)
	    fault = 0;
}

/***********************************************************************/

static void
HandleInterrupt(UINT intcode, struct TWIN_i386_context_s *sc )
{
	ENV localenv,*env;
	
	env = &localenv;
	env->reg.ax = sc->eax & 0xffff;
	env->reg.bx = sc->ebx & 0xffff;
	env->reg.cx = sc->ecx & 0xffff;
	env->reg.dx = sc->edx & 0xffff;

	env->reg.si = sc->esi & 0xffff;
	env->reg.di = sc->edi & 0xffff;

	env->reg.ds = sc->ds & 0xffff;
	env->reg.es = sc->es & 0xffff;

	env->reg.flags =   0;

	xdoscall(XDOS_INT86,intcode, 0,(void *) env);

	/* restore registers */
	/* first unix trapped the int for us */
	/* so we only need to skip the opcode */
	sc->eip += 2;

	/* now restore the 32bit registers w/ 16bit values */
	sc->eax &= 0xffff0000;
	sc->eax |= LOWORD(env->reg.ax);

	sc->ebx &= 0xffff0000;
	sc->ebx |= LOWORD(env->reg.bx);

	sc->ecx &= 0xffff0000;
	sc->ecx |= LOWORD(env->reg.cx);

	sc->edx &= 0xffff0000;
	sc->edx |= LOWORD(env->reg.dx);

	sc->esi &= 0xffff0000;
	sc->esi |= LOWORD(env->reg.si);

	sc->edi &= 0xffff0000;
	sc->edi |= LOWORD(env->reg.di);

	sc->es = LOWORD(env->reg.es);

	/* does this set carry */ 
	sc->eflags &= 0xffff0000;
	sc->eflags |= env->reg.flags & CARRY_FLAG;
}

static void FixSegmentRegister(unsigned short *sreg)
{
    int ildt;
    
    ildt = *sreg >> 3;
    
    if (*sreg != native_cs &&
	*sreg != native_ss &&
	*sreg != native_ds &&
	(((*sreg & 0x7) != 0x7) || ildt >= nLDTSize || !LDT[ildt].lpSelBase))
    {
	*sreg = 0;
    }
}
    
void TWIN_ValidateSegmentRegisters(struct TWIN_i386_context_s *sc)
{
    FixSegmentRegister(&sc->es);
    FixSegmentRegister(&sc->fs);
    FixSegmentRegister(&sc->gs);
}

static void
DumpRegisters(struct TWIN_i386_context_s *sc)
{
	ERRSTR((LF_ERROR,"eax = 0x%x\necx = 0x%x\nedx = 0x%x\nebx = 0x%x\n",
		sc->eax,sc->ecx,sc->edx,sc->ebx));
	ERRSTR((LF_ERROR,"esp = 0x%x\nebp = 0x%x\nesi = 0x%x\nedi = 0x%x\n",
		sc->esp,sc->ebp,sc->esi,sc->edi));
	ERRSTR((LF_ERROR,"eip = 0x%x\ncs  = 0x%x\nflg = 0x%x\nss  = 0x%x\n",
		sc->eip,sc->cs,sc->eflags,sc->ss));
	ERRSTR((LF_ERROR,"ds  = 0x%x\nes  = 0x%x\nfs  = 0x%x\ngs  = 0x%x\n",
		sc->ds,sc->es,sc->fs,sc->gs));
}

