#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/mman.h>
#ifdef linux
#include <sys/utsname.h>
#include <sys/ptrace.h>
#endif
#include <sys/user.h>
#include <windows.h>
#include "db_disasm.h"
#include "debugger.h"
#include "bpregs.h"
#include "regpos.h"
#include "info.h"

extern int dbg_mode;
extern WORD native_cs;
extern unsigned int dbg_mask;
extern unsigned int *regval;
extern char *force_parse_string;
extern int   force_exit;
extern int lex_syntax_error;
extern int TWIN_dbg_nerrs;
extern FILE *debug_outfp;

/*
 * Structure to keep track of INT 3 breakpoints.
 */
#define N_BP	64

struct break_bp
{
    unsigned long addr;
    unsigned long next_addr;
    char *conditional;
    char in_use;
    char enabled;
    char temporary;
    unsigned char databyte;
};

/*
 * Structure to keep track of debug register breakpoints.
 */
#define N_MP	4

struct break_mp
{
    unsigned long addr;
    unsigned long crval;
    char in_use;
    char enabled;
    char read_write;
};

static struct break_bp bbp[N_BP];
static struct break_mp bmp[N_MP];

static int current_bp = -1;
static int trace_mode = 0;

/* Execution mode:
 * 0 - continuous execution
 * 1 - advancing after breakpoint
 * 2 - single step - not implemented
 */
#define DBG_CONTMODE_CONTINUOUS	0
#define DBG_CONTMODE_ADVANCING	1
#define DBG_CONTMODE_SINGLESTEP	2
#define DBG_CONTMODE_TRACE	3

static int cont_mode = DBG_CONTMODE_CONTINUOUS;
static int debuggercontrol;
extern void TWIN_dbg_parse();


/* Soft copy of debug control register contents. */
static unsigned long break_crval_softc = 0;

void break_condition(int num, char *expr)
{
    /* Verify that the breakpoint exists */
    if (num >= N_BP || num < 0)
    {
	fprintf(debug_outfp, "Breakpoint number is out of range.\n");
	return;
    }

    if (!bbp[num].in_use)
    {
	fprintf(debug_outfp, "That breakpoint is not set.\n");
	return;
    }

    /* save conditional */
    bbp[num].conditional = (char *) strdup(expr);

    /* test expression for syntax errors */
    force_parse_string = malloc(strlen(expr) + 10);

    strcpy(force_parse_string, "print ");
    strcat(force_parse_string, expr);
    strcat(force_parse_string, "\n");

    force_exit = 1;

    fprintf(debug_outfp, "Current value of expression is:\n");
    TWIN_dbg_parse();

    if (TWIN_dbg_nerrs || lex_syntax_error)
    {
	free(bbp[num].conditional);
	bbp[num].conditional = NULL;
    }
}

void info_break(void)
{
    int j;


    fprintf(debug_outfp, "Memory points:\n");
    for (j = 0; j < N_MP; j++)
    {
	if (bmp[j].in_use)
	{
	    fprintf(debug_outfp, "%d: %c %c ", 
		    j, (bmp[j].enabled ? 'y' : 'n'), bmp[j].read_write);
	    TWIN_print_address(bmp[j].addr, debug_outfp, 
			  guess_segment_offset(bmp[j].addr) ? 16 : 32);
	    fprintf(debug_outfp, "\n");
	}
    }

    fprintf(debug_outfp, "\nBreak points:\n");
    for (j = 0; j < N_BP; j++)
    {
	if(bbp[j].in_use)
	{
	    fprintf(debug_outfp, "%d: %c ", j, (bbp[j].enabled ? 'y' : 'n'));
	    TWIN_print_address(bbp[j].addr, debug_outfp, 
			  guess_segment_offset(bbp[j].addr) ? 16 : 32);
	    if (bbp[j].conditional)
		fprintf(debug_outfp, "  IF %s", bbp[j].conditional);
	    fprintf(debug_outfp, "\n");
	}
    }
}

int add_break(unsigned long addr, char temporary)
{
    int j;

    for(j = 0; j < N_BP; j++)
    {
	if(!bbp[j].in_use)
	{
	    bbp[j].in_use = 1;
	    bbp[j].enabled = 1;
	    bbp[j].temporary = temporary;
	    bbp[j].addr = addr;
	    bbp[j].next_addr = 0;
	    bbp[j].conditional = NULL;

	    return j;
	}
    }

    if (!temporary)
	fprintf(debug_outfp,"No more breakpoints\n");

    return -1;
}

void delete_break(int num)
{
    if (num >= N_BP || num < 0)
    {
	fprintf(debug_outfp, "Breakpoint number is out of range.\n");
	return;
    }

    if (!bbp[num].in_use)
    {
	fprintf(debug_outfp, "That breakpoint is not set.\n");
	return;
    }

    bbp[num].in_use = 0;
}

void enable_break(int num, int enabled_flag)
{
    if (num >= N_BP || num < 0)
    {
	fprintf(debug_outfp, "Breakpoint number is out of range.\n");
	return;
    }

    if (!bbp[num].in_use)
    {
	fprintf(debug_outfp, "That breakpoint is not set.\n");
	return;
    }

    bbp[num].enabled = enabled_flag;
}

void add_memorypoint(unsigned long addr, char type)
{
    int i;
    unsigned long read_write;
    
    /*
     * Find free debug register.
     */
    for (i = 0; i < N_MP; i++)
	if (!bmp[i].in_use)
	    break;
    
    if (i == N_MP)
    {
	fprintf(debug_outfp, "All debug registers are currently used.\n");
	return;
    }

    if (type == 'r')
	read_write = (DBGCTRL_RW_DATARDWR << DBGCTRL_RW_SHIFT(i));
    else
	read_write = (DBGCTRL_RW_DATAWRITE << DBGCTRL_RW_SHIFT(i));
    
    bmp[i].addr = (addr &= ~0x00000003);
    bmp[i].in_use = 1;
    bmp[i].enabled = 1;
    bmp[i].read_write = type;
    bmp[i].crval = (DBGCTRL_LNEN(i) | 
		    read_write |
		    (DBGCTRL_LEN_BYTE << DBGCTRL_LEN_SHIFT(i)));

    break_crval_softc &= ~DBGCTRL_MASK(i);
    break_crval_softc |= bmp[i].crval;
    
    if (break_crval_softc)
	break_crval_softc |= DBGCTRL_LEXACT;
    else
	break_crval_softc &= ~DBGCTRL_LEXACT;

#ifdef linux
    if (ptrace(PTRACE_POKEUSR, getpid(), DBGREG_REG(i), addr) < 0)
    {
	if(debuggercontrol)
		perror("debug address");
    }
#endif
}

void delete_memorypoint(int num)
{
    if (num >= N_MP || num < 0)
    {
	fprintf(debug_outfp, "Memorypoint number is out of range.\n");
	return;
    }

    if (!bmp[num].in_use)
    {
	fprintf(debug_outfp, "That memorypoint is not set.\n");
	return;
    }

    bmp[num].in_use = 0;
}

void enable_memorypoint(int num, int enabled_flag)
{
    if (num >= N_MP || num < 0)
    {
	fprintf(debug_outfp, "Memorypoint number is out of range.\n");
	return;
    }

    if (!bmp[num].in_use)
    {
	fprintf(debug_outfp, "That memorypoint is not set.\n");
	return;
    }

    bmp[num].enabled = enabled_flag;
}

static void bark()
{
    static int barked=0;

    if (barked)
	return;

    barked=1;
    perror("Sorry, can't set break point");
}

void insert_break(int flag)
{
    unsigned char *pnt;
    int j;

    /*
     * Go through all of the INT 3 breakpoints and either install or deinstall
     * each breakpoint.
     */
    for(j = 0; j < N_BP; j++)
    {
	if(bbp[j].enabled)
	{
	    /* There are a couple of problems with this. On Linux prior to
	       1.1.62, this call fails (ENOACCESS) due to a bug in fs/exec.c.
	       This code is currently not tested at all on BSD.
	       How do I determine the page size in a more symbolic manner?
	       And why does mprotect need that start address of the page
	       in the first place?
	       Not that portability matters, this code is i386 only anyways...
	       How do I get the old protection in order to restore it later on?
	     */
	    if(mprotect((caddr_t)(bbp[j].addr & (~4095)), 4096, 
			PROT_READ|PROT_WRITE|PROT_EXEC) == -1)
	    {
		bark();
		return;
	    }

	    pnt = (unsigned char *) bbp[j].addr;

	    if(flag) 
	    {
		bbp[j].databyte = *pnt;
		*pnt = 0xcc;  /* Change to an int 3 instruction */
	    } 
	    else 
	    {
		*pnt = bbp[j].databyte;
	    }

	    mprotect((caddr_t)(bbp[j].addr & ~4095), 4096, 
		     PROT_READ|PROT_EXEC);
	}
    }

    /*
     * Go through all of the debug register breakpoints and enable or disable
     * each breakpoint as is appropriate.
     */
    for (j = 0; j < N_MP; j++)
    {
	if (flag && bmp[j].in_use && bmp[j].enabled)
	    break_crval_softc |= DBGCTRL_LNEN(j);
	else
	    break_crval_softc &= ~DBGCTRL_LNEN(j);
    }

#ifdef linux
    if (ptrace(PTRACE_POKEUSR, getpid(), DBGREG_CTRL, break_crval_softc) < 0)
    {
	if(debuggercontrol)
		perror("debug control");
    }
#endif
}

static int is_call_instruction(unsigned int addr)
{
    unsigned char *ip = (unsigned char *) addr;
    int is_prefix = 1;
    
    /* skip past any prefix that may exist */
    while (is_prefix)
    {
	switch (*ip)
	{
	  case 0x26:
	  case 0x2E:
	  case 0x36:
	  case 0x3E:
	  case 0x64:
	  case 0x65:
	  case 0x66:
	  case 0x67:
	  case 0xF0:
	  case 0xF2:
	  case 0xF3:
	    ip++;
	    break;
	    
	  default:
	    is_prefix = 0;
	    break;
	}
    }

    /* These are definately call instructions */
    if (*ip == 0x9A || *ip == 0xE8)
	return 1;
    
    /* These instructions are decoded using bits 5,4,3 of the mod/rm byte */
    if (*ip == 0xFF)
    {
	int nnn = (*(ip+1) >> 3) & 7;

	if (nnn == 2 || nnn == 3)
	    return 1;
    }
    
    /* If we got this far then it isn't a call instruction */
    return 0;
}

void break_continue()
{
    cont_mode = DBG_CONTMODE_CONTINUOUS;
}

void break_trace(int num)
{
    SC_EFLAGS |= 0x0100;
    cont_mode = DBG_CONTMODE_TRACE;

    if (num == 16 || num == 32)
	trace_mode = num;
    else
	trace_mode = 0;
    return;
}

void break_step(int skip_calls)
{
    unsigned int addr;
    int i;

    /* What is the address of the current instruction? */
    if (SC_CS != native_cs)
	addr = (unsigned int) GetAddress(SC_CS, SC_EIP(dbg_mask));
    else
	addr = SC_EIP(dbg_mask);
    
    /* If we don't want to skip over a function call or if the next instruction
     * isn't a call, then just set the trace flag and continue.
     */
    if (!skip_calls || !is_call_instruction(addr))
    {
	SC_EFLAGS |= 0x0100;
	cont_mode = DBG_CONTMODE_SINGLESTEP;
	return;
    }

    /* If we are already at a breakpoint, then set single step mode
     * and continue.
     */
    if (current_bp >= 0 && !bbp[current_bp].temporary)
    {
	cont_mode = DBG_CONTMODE_SINGLESTEP;
	return;
    }
    
    /* What is the address of the next instruction? */
    addr = db_disasm( addr, 0, (dbg_mode == 16), 1 );
    
    /* We need to create a temporary breakpoint. */
    i = add_break(addr, 1);
    
    /* Set the continue mode to single step */
    cont_mode = DBG_CONTMODE_SINGLESTEP;
}

int break_is_stepping()
{
    return (cont_mode == DBG_CONTMODE_SINGLESTEP);
}

int break_is_tracing()
{
    return (cont_mode == DBG_CONTMODE_TRACE);
}

int break_is_trace_mode(int num)
{
    return (!trace_mode || trace_mode == num);
}

int get_bpnum(unsigned int addr)
{
    unsigned long break_srval;
    int j;

    /* Check the debug status register to see if we hit a memory point */
#ifdef linux
    errno = 0;
    break_srval = ptrace(PTRACE_PEEKUSR, getpid(), DBGREG_STAT, 0);
    if (errno && debuggercontrol)
	perror("debug status");

    if (break_srval)
	ptrace(PTRACE_POKEUSR, getpid(), DBGREG_STAT, 0);

    for (j = 0; j < N_MP; j++)
    {
	if ((break_srval & (1 << j)) && bmp[j].in_use && bmp[j].enabled)
	    return -1 - j;
    }
#endif

    /* Have we hit an INT 3 breakpoint? */
    for(j = 0; j < N_BP; j++)
	if(bbp[j].enabled && bbp[j].in_use && bbp[j].addr == addr) 
	    return j;

    /* Check for single step trap */
    if (break_srval & 0x4000)
	return -5;

    return -1;
}

void toggle_next(int num)
{
    unsigned int addr;

    addr = bbp[num].addr;
    if(bbp[num].next_addr == 0)
	bbp[num].next_addr = db_disasm( addr, 0, (dbg_mode == 16), 1 );

    bbp[num].addr = bbp[num].next_addr;
    bbp[num].next_addr = addr;
}

int should_continue(int bpnum)
{
    if (bpnum < 0)
	return 0;

    toggle_next(bpnum);
    if (bpnum == current_bp)
    {
        current_bp = -1;
	if (cont_mode == DBG_CONTMODE_SINGLESTEP)
	{
	    cont_mode = DBG_CONTMODE_CONTINUOUS;
	    return 0;
	}
	else
	{
	    cont_mode = DBG_CONTMODE_CONTINUOUS;
	    return 1;
	}
    }
    else
    {
	if (!bbp[bpnum].temporary)
	{
	    if (bbp[bpnum].conditional)
	    {
		force_parse_string = malloc(strlen(bbp[bpnum].conditional)+10);

		strcpy(force_parse_string, "if ");
		strcat(force_parse_string, bbp[bpnum].conditional);
		strcat(force_parse_string, "\n");
	    }
	    
	    cont_mode = DBG_CONTMODE_ADVANCING;
	    current_bp = bpnum;
	}
	else
	{
	    cont_mode = DBG_CONTMODE_CONTINUOUS;
	    bbp[bpnum].in_use = 0;
	}
	
	return 0;
    }
}
