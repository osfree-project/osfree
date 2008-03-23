/*
 * Wine debugger utility routines
 * Eric Youngdale
 * 9/93
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include "debugger.h"
#include "db_disasm.h"
#include "regpos.h"
#include "BinTypes.h"
#include "info.h"

typedef struct frame16_s
{
    unsigned short saved_bp;
    unsigned short saved_ip;
    unsigned short saved_cs;
} FRAME16;
	
typedef struct frame32_s
{
    unsigned long saved_bp;
    unsigned long saved_ip;
    unsigned short saved_cs;
} FRAME32;

typedef struct extended_frame32_s
{
    unsigned long saved_edi;
    unsigned long saved_esi;
    unsigned long saved_bp;
    unsigned long saved_sp;
    unsigned long saved_ebx;
    unsigned long saved_edx;
    unsigned long saved_ecx;
    unsigned long saved_eax;
    unsigned long saved_ip;
    unsigned short saved_cs;
} EXTFRAME32;

extern LPOBJHEAD lpModuleHead;
extern int * regval;
extern unsigned int dbg_mask;
extern unsigned int dbg_mode;
extern unsigned int auto_dbg_mode;
extern WORD native_cs;
extern WORD native_ds;
extern WORD native_ss;
extern ENV *envp_global;
extern FILE *debug_outfp;

extern char * find_nearest_symbol(unsigned int *);
extern unsigned long guess_segment_offset(unsigned long laddr);
extern char *find_nearest_export(WORD seg, WORD off);
extern unsigned int *find_export(char *name);
extern void find_invoke_native_bounds(unsigned long *startp,
				      unsigned long *endp);

void application_not_running()
{
    fprintf(debug_outfp,"Application not running\n");
}

void TWIN_print_address(unsigned int addr, FILE * outfile, int addrlen)
{
    char *name = NULL;
    unsigned long segoff;

    if (addrlen == 16)
    {
	segoff = guess_segment_offset(addr);
	if (segoff)
	{
	    fprintf( outfile, "%4.4x:%4.4x", 
			(unsigned int) segoff >> 16, 
			(unsigned int) segoff & 0xffff );
	    if (GetSelectorType(segoff >> 16) != TRANSFER_BINARY)
		name = find_nearest_export(segoff >> 16, segoff & 0xffff);
	}
	else
	    fprintf(outfile,"0x%8.8x", addr);
    }
    else
    {
	fprintf(outfile,"0x%8.8x", addr);
	name = find_nearest_symbol((unsigned int *) addr);
    }

    if (name)
	fprintf(debug_outfp, "(%s)", name);
}


void info_reg(){

	  if(!regval) {
	    application_not_running();
	    return;
	  }

	fprintf(debug_outfp,"Register dump:\n");
	/* First get the segment registers out of the way */
	fprintf(debug_outfp,
		" CS:%4.4x SS:%4.4x DS:%4.4x ES:%4.4x GS:%4.4x FS:%4.4x\n", 
		SC_CS, SC_SS, SC_DS, SC_ES, SC_GS, SC_FS);

	/* Now dump the main registers */
	fprintf(debug_outfp," EIP:%8.8x ESP:%8.8x EBP:%8.8x EFLAGS:%8.8x\n", 
		SC_EIP(dbg_mask), SC_ESP(dbg_mask), 
		SC_EBP(dbg_mask), SC_EFLAGS);

	/* And dump the regular registers */

	fprintf(debug_outfp," EAX:%8.8x EBX:%8.8x ECX:%8.8x EDX:%8.8x\n", 
		SC_EAX(dbg_mask), SC_EBX(dbg_mask), 
		SC_ECX(dbg_mask), SC_EDX(dbg_mask));

	/* Finally dump these main registers */
	fprintf(debug_outfp," EDI:%8.8x ESI:%8.8x\n", 
		SC_EDI(dbg_mask), SC_ESI(dbg_mask));

}

void info_stack(){
	unsigned int * dump;
	int i;

	if(!regval) {
	  application_not_running();
	  return;
	}

	fprintf(debug_outfp,"Stack dump:\n");
	dump = (int*) SC_EIP(dbg_mask);
	for(i=0; i<22; i++) 
	{
	    fprintf(debug_outfp," %8.8x", *dump++);
	    if ((i % 8) == 7)
		fprintf(debug_outfp,"\n");
	}
	fprintf(debug_outfp,"\n");
}


void examine_memory(int addr, int count, char format)
{
    char * pnt;
    unsigned int * dump;
    unsigned short int * wdump;
    int old_dbg_mode = 0;
    int i;

    if (auto_dbg_mode)
    {
	unsigned long segoff = guess_segment_offset(addr);

	old_dbg_mode = dbg_mode;
	if (segoff)
	    dbg_mode = 16;
	else
	    dbg_mode = 32;
    }
    else if ((addr & 0xffff0000) == 0 && dbg_mode == 16)
	addr = (int) GetAddress((format == 'i' ? SC_CS : SC_DS), addr);

    if(format != 'i' && count > 1) 
    {
	TWIN_print_address(addr, debug_outfp, dbg_mode);
	fprintf(debug_outfp,":  ");
    };

    switch(format)
    {
      case 's':
	pnt = (char *) addr;
	if (count == 1) 
	    count = 256;
	while(*pnt && count) 
	{
	    fputc( *pnt++, debug_outfp);
	    count--;
	}
	fprintf(debug_outfp,"\n");
	dbg_mode = old_dbg_mode;
	return;

      case 'i':
	for(i=0; i<count; i++) 
	{
	    TWIN_print_address(addr, debug_outfp, dbg_mode);
	    fprintf(debug_outfp,":  ");
	    addr = db_disasm( addr, 0, (dbg_mode == 16), 0 );
	    fprintf(debug_outfp,"\n");
	};
	dbg_mode = old_dbg_mode;
	return;
    
      case 'x':
	dump = (unsigned int *) addr;
	for(i=0; i<count; i++) 
	{
	    fprintf(debug_outfp," %8.8x", *dump++);
	    addr += 4;
	    if ((i % 8) == 7) 
	    {
		fprintf(debug_outfp,"\n");
		TWIN_print_address(addr, debug_outfp, dbg_mode);
		fprintf(debug_outfp,":  ");
	    };
	}
	fprintf(debug_outfp,"\n");
	dbg_mode = old_dbg_mode;
	return;
	
      case 'd':
        dump = (unsigned int *) addr;
	for(i=0; i<count; i++) 
	{
	    fprintf(debug_outfp," %d", *dump++);
	    addr += 4;
	    if ((i % 8) == 7) 
	    {
		fprintf(debug_outfp,"\n");
		TWIN_print_address(addr, debug_outfp, dbg_mode);
		fprintf(debug_outfp,":  ");
	    };
	}
	fprintf(debug_outfp,"\n");
	dbg_mode = old_dbg_mode;
	return;
	
      case 'w':
	wdump = (unsigned short *) addr;
	for(i=0; i<count; i++) 
	{
	    fprintf(debug_outfp," %x", *wdump++);
	    addr += 2;
	    if ((i % 10) == 7) 
	    {
		fprintf(debug_outfp,"\n");
		TWIN_print_address(addr, debug_outfp, dbg_mode);
		fprintf(debug_outfp,":  ");
	    };
	}
	fprintf(debug_outfp,"\n");
	dbg_mode = old_dbg_mode;
	return;
	
      case 'c':
	  pnt = (char *) addr;
	for(i=0; i<count; i++) 
	{
	    if(*pnt < 0x20) 
	    {
		fprintf(debug_outfp,"  ");
		pnt++;
	    } 
	    else
		fprintf(debug_outfp," %c", *pnt++);
	    addr++;
	    if ((i % 32) == 7) 
	    {
		fprintf(debug_outfp,"\n");
		TWIN_print_address(addr, debug_outfp, dbg_mode);
		fprintf(debug_outfp,":  ");
	    };
	}
	fprintf(debug_outfp,"\n");
	dbg_mode = old_dbg_mode;
	return;
	
      case 'b':
	pnt = (char *) addr;
	for(i=0; i<count; i++) 
	{
	    fprintf(debug_outfp," %02x", (*pnt++) & 0xff);
	    addr++;
	    if ((i % 32) == 7) 
	    {
		fprintf(debug_outfp,"\n");
		TWIN_print_address(addr, debug_outfp, dbg_mode);
		fprintf(debug_outfp,":  ");
	    };
	}
	fprintf(debug_outfp,"\n");
	dbg_mode = old_dbg_mode;
	return;
    };
	
    /* The rest are fairly straightforward */
    
    fprintf(debug_outfp,"examine mem: %x %d %c\n", addr, count, format);
    dbg_mode = old_dbg_mode;
}

char * helptext[] = {
"The commands accepted by the win debugger are a small subset",
"of the commands that gdb would accept.  The commands currently",
"are:\n",
"  break *<addr>                        bt",
"  disable bpnum                        enable bpnum",
"  help                                 quit",
"  x <expr>                             cont",
"  mode [16,32]                         print <expr>",
"  set <reg> = <expr>                   set *<expr> = <expr>",
"  info [reg,stack,break,segments]      symbolfile <filename>",
"  define <identifier> <expr>",
"",
"The 'x' command accepts repeat counts and formats (including 'i') in the",
"same way that gdb does.",
"",
" The following are examples of legal expressions:",
" $eax     $eax+0x3   0x1000   ($eip + 256)  *$eax   *($esp + 3)",
" Also, a nm format symbol table can be read from a file using the",
" symbolfile command.  Symbols can also be defined individually with",
" the define command.",
"",
NULL};

void dbg_help(){
	int i;
	i = 0;
	while(helptext[i]) fprintf(debug_outfp,"%s\n", helptext[i++]);
}

/* Following code was added for Willows' library
 *
 * Copyright 1995 - Willows Software, Inc.
 */

void backtrace()
{
    static unsigned long invoke_native_start = 0;
    static unsigned long invoke_native_end   = 0;
    void *frame_p;
    FRAME16 *p16;
    FRAME32 *p32;
    EXTFRAME32 *ep32;
    WORD cs;
    WORD ss;
    ENV *next_envp = envp_global;
    int frameno = 0;

    fprintf(debug_outfp, "   frame    return address\n");

    /*
     * Get the initial frame pointer and code segment
     */
    if (SC_SS != native_ss)
	frame_p = GetAddress( SC_SS, SC_BP & ~1 );
    else
	frame_p = (void *) SC_EBP(dbg_mask);

    cs = SC_CS;
    ss = SC_SS;
    while(frame_p)
    {
	/*
	 * Print frame number.
	 */
	fprintf(debug_outfp,"%2d %08x ",frameno++, (unsigned int) frame_p);

	/*
	 * We need to handle things differently if we are on the native stack.
	 */
	if (cs == native_cs)
	{
	    extern int LoadTwinModDscr();


	    p32 = frame_p;

	    TWIN_print_address(p32->saved_ip, debug_outfp, 32);
	    fprintf(debug_outfp, "\n" );
	    if (p32->saved_ip >= ((unsigned long)LoadTwinModDscr) &&
		p32->saved_ip <= ((unsigned long)LoadTwinModDscr+1000)) 
	    {
		break;
	    }

	    /*
	     * If we are in invoke_native, then next return is return to
	     * binary mode.
	     */
	    if (!invoke_native_start)
	    {
		find_invoke_native_bounds(&invoke_native_start,
					  &invoke_native_end);
	    }
	    
	    if (invoke_native_start < p32->saved_ip &&
		invoke_native_end > p32->saved_ip)
	    {
		frame_p = (void *) next_envp->reg.bp;
		cs = LOWORD((void *) next_envp->reg.cs);
		ss = LOWORD((void *) next_envp->reg.ss);
	    }
	    else
		frame_p = (void *) p32->saved_bp;
	} 
	/*
	 * Otherwise we are on the binary stack.
	 */
	else 
	{
	    p16 = frame_p;
	    if (p16->saved_bp & 1) 
		cs = p16->saved_cs;

	    /*
	     * Is the return address to a Native mode function?
	     */
	    if (GetSelectorType(cs) == TRANSFER_RETURN)
	    {
		ep32 = (EXTFRAME32 *) next_envp->machine_stack;
		
		TWIN_print_address(ep32->saved_ip, debug_outfp, 32);
		fprintf(debug_outfp, "\n" );

		cs = native_cs;
		frame_p = (void *) ep32->saved_bp;

		next_envp = (ENV *) next_envp->prev_env;
	    }
	    else
	    {
		TWIN_print_address((unsigned int) GetAddress(cs, p16->saved_ip), 
			      debug_outfp, 16);
		fprintf(debug_outfp, "\n" );
		if (!p16->saved_bp) 
		    break;
		frame_p = GetAddress(ss, p16->saved_bp & ~1);
	    }
	}
    }

    fprintf(debug_outfp, "\n");
}

static char *selector_type_names[] =
{
    "UNKNOWN", "CODE16", "DATA16", "CODE32", "DATA32", "CALLBACK", "NATIVE",
    "BINARY", "RETURN"
};

void
info_modules()
{
    MODULEENTRY me;

    if (!ModuleFirst(&me))
    {
	fprintf(debug_outfp, "No modules.\n");
	return;
    }

    fprintf(debug_outfp, "HANDLE NAME                 SIZE     EXE PATH\n");

    do
    {
	fprintf(debug_outfp, "%04x   %-20s %08x",
		me.hModule, me.szModule, (unsigned int) me.dwSize);
	if (me.szExePath[0])
	    fprintf(debug_outfp, " %40s\n", me.szExePath);
	else
	    fprintf(debug_outfp, "\n");

	me.szExePath[0] = '\0';
    }
    while (ModuleNext(&me));
}

void info_exports(HMODULE hmod, BOOL all_flag)
{
    LPMODULEINFO mip;
    ENTRYTAB *ep;

    mip = GETMODULEINFO(hmod);
    if (!mip)
    {
	fprintf(debug_outfp, "No such module.\n");
	return;
    }

    fprintf(debug_outfp, "BINARY    NATIVE   NAME\n");

    for (ep = mip->lpEntryTable; ep->name; ep++)
    {
	if ((ep->name[0] || ep->sel || ep->fun) &&
	    (ep->name[0] || all_flag))
	{
	    if (ep->sel)
	    {
		fprintf(debug_outfp, "%04x:%04x ", 
			(mip->wSelBase << 3) + ep->sel - 1, ep->off);
	    }
	    else
		fprintf(debug_outfp, "          ");

	    if (ep->fun)
		fprintf(debug_outfp, "%08x %s\n", 
			(unsigned int) ep->fun, ep->name);
	    else
		fprintf(debug_outfp, "         %s\n", ep->name);
	}
    }
}

void
PrintLDT()
{
    WORD sel;

    printf("SEL  BASE     LIMIT HANDLE TYPE     FLAGS\n");

    for (sel = 7; sel < (nLDTSize << 3); sel += 8)
    {
	if (GetSelectorHandle(sel))
	{
	    fprintf(debug_outfp, "%04x %08x %04x  %04x   %8s ",
		   sel,
		   (unsigned int)GetPhysicalAddress(sel), 
		   (unsigned int)GetSelectorLimit(sel),
		   (unsigned int) GetSelectorHandle(sel),  
		   (char *) selector_type_names[GetSelectorType(sel)]);
	    if (GetSelectorFlags(sel) & DF_PRESENT)
		fprintf(debug_outfp, "PRESENT ");
	    if (GetSelectorFlags(sel) & DF_DPL)
		fprintf(debug_outfp, "DPL ");
	    if (GetSelectorFlags(sel) & DF_USER)
		fprintf(debug_outfp, "USER ");
	    if (GetSelectorFlags(sel) & DF_CODE)
	    {
		fprintf(debug_outfp, "CODE ");

		if (GetSelectorFlags(sel) & DF_CREADABLE)
		    fprintf(debug_outfp, "READABLE ");
	    }
	    if (GetSelectorFlags(sel) & DF_DATA)
	    {
		fprintf(debug_outfp, "DATA ");

		if (GetSelectorFlags(sel) & DF_DWRITEABLE)
		    fprintf(debug_outfp, "WRITEABLE ");
	    }
	    if (GetSelectorFlags(sel) & DF_EXPANDDOWN)
		fprintf(debug_outfp, "EXPANDDOWN ");
	    fprintf(debug_outfp, "\n");
	}
    }
}

HMODULE find_module(char *module_name)
{
    LPMODULEINFO mip;
    OBJHEAD *lpObjNext;
    
    mip = (LPMODULEINFO) lpModuleHead;
    while (mip)
    {
	if (strncasecmp(mip->lpModuleName, 
			module_name, strlen(module_name)) == 0)
	{
	    return mip->ObjHead.hObj;
	}

	if ((lpObjNext = (LPOBJHEAD)mip->ObjHead.lpObjNext))
	    mip = GETMODULEINFO(lpObjNext->hObj);
	else
	    mip = 0;
    }

    return 0;
}

unsigned long guess_segment_offset(unsigned long laddr)
{
    unsigned long seg_start;
    WORD sel;

    for (sel = 7; sel < (nLDTSize << 3); sel += 8)
    {
	if (GetSelectorHandle(sel))
	{
	    seg_start = (unsigned long) GetPhysicalAddress(sel);

	    if (GetSelectorFlags(sel) & DF_PRESENT &&
		seg_start <= laddr &&
		seg_start + GetSelectorLimit(sel) + 1 >= laddr)
	    {
		return (((unsigned long) sel << 16) +
			laddr - (unsigned long) seg_start);
	    }
	}
    }

    return 0;
}

unsigned int *
find_export(char *name)
{
    LPMODULEINFO mip;
    ENTRYTAB *ep;
    OBJHEAD *lpObjNext;
    
    mip = (LPMODULEINFO) lpModuleHead;
    while (mip)
    {
	if (mip->wSegCount > 1 ||
	    (GetSelectorType((mip->wSelBase << 3)) >= 1 &&
	      GetSelectorType((mip->wSelBase << 3)) <= 4))
	{
	    for (ep = mip->lpEntryTable; ep->name; ep++)
	    {
		if (ep->sel && strncasecmp(ep->name, name, strlen(name)) == 0)
		{
		    return ((unsigned int *) 
			    GetAddress((mip->wSelBase << 3) + ep->sel - 1, 
				       ep->off));
		}
	    }
	}

	if ((lpObjNext = (LPOBJHEAD)mip->ObjHead.lpObjNext))
	    mip = GETMODULEINFO(lpObjNext->hObj);
	else
	    mip = 0;
    }

    return (unsigned int *) 0xffffffff;
}

char *
find_nearest_export(WORD seg, WORD off)
{
    static char name_buf[128];
    LPMODULEINFO mip;
    ENTRYTAB *ep;
    ENTRYTAB *best_ep;
    OBJHEAD *lpObjNext;

    for (mip = (LPMODULEINFO) lpModuleHead;
	 mip->wSelBase > (seg >> 3) || 
	 mip->wSelBase + mip->wSegCount < (seg >> 3);
	 )
    {
	if ((lpObjNext = (LPOBJHEAD)mip->ObjHead.lpObjNext))
	    mip = GETMODULEINFO(lpObjNext->hObj);
	else
	{
	    mip = 0;
	    break;
	}
    }

    best_ep = NULL;
    if (mip)
    {
	for (ep = mip->lpEntryTable; ep->name; ep++)
	{
	    if ((mip->wSelBase << 3) + ep->sel - 1 == seg && 
		ep->off <= off && 
		(!best_ep || ep->off > best_ep->off))
	    {
		best_ep = ep;
	    }
	}

	if (best_ep)
	{
	    if (best_ep->name[0])
		sprintf(name_buf, "%s.%s+0x%x", 
			mip->lpModuleName, best_ep->name, off - best_ep->off);
	    else
		sprintf(name_buf, "%s.%d+0x%x", 
			mip->lpModuleName, 
			best_ep - (ENTRYTAB *) mip->lpEntryTable,
			off - best_ep->off);

	    return name_buf;
	}
    }

    return NULL;
}

int
find_module_segment(HMODULE hmod, int segidx)
{
    LPMODULEINFO mip;

    mip = GETMODULEINFO(hmod);
    if (!mip)
    {
	fprintf(debug_outfp, "No such module.\n");
	return 0;
    }

    if (segidx > mip->wSegCount)
    {
	fprintf(debug_outfp, "No such segment.\n");
	return 0;
    }

    return ((mip->wSelBase + segidx) << 3) - 1;
}

char *
translate_ordinal(HMODULE hmod, int ordinal)
{
    LPMODULEINFO mip;
    ENTRYTAB *ep;

    mip = GETMODULEINFO(hmod);
    if (!mip)
    {
	fprintf(debug_outfp, "No such module.\n");
	return (char *)0;
    }

    ep = mip->lpEntryTable + ordinal;
    return GetAddress((mip->wSelBase << 3) + ep->sel - 1, ep->off);
}

unsigned int *
translate_export(HMODULE hmod, char *name)
{
    LPMODULEINFO mip;
    ENTRYTAB *ep;

    mip = GETMODULEINFO(hmod);
    if (!mip)
    {
	fprintf(debug_outfp, "No such module.\n");
	return 0;
    }

    for (ep = mip->lpEntryTable; ep->name; ep++)
    {
	if (ep->sel && strncasecmp(ep->name, name, strlen(name)) == 0)
	{
	    return ((unsigned int *) 
		    GetAddress((mip->wSelBase << 3) + ep->sel - 1, 
			       ep->off));
	}
    }

    return (unsigned int *) 0xffffffff;
}
