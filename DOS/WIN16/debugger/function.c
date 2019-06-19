/* function.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include "debugger.h"
#include "db_disasm.h"
#include "regpos.h"
#include "BinTypes.h"
#include "info.h"

extern FILE *debug_outfp;

#define MAX_ARGS	20

struct debug_arglist
{
    int n_args;
    LONG arg_array[20];
};

typedef int (*debug_funcptr)(LONG a0, LONG a1, LONG a2, LONG a3, LONG a4, 
			     LONG a5, LONG a6, LONG a7, LONG a8, LONG a9,
			     LONG a10, LONG a11, LONG a12, LONG a13, LONG a14,
			     LONG a15, LONG a16, LONG a17, LONG a18, LONG a19);
typedef int (*debug_noarg_funcptr)(void);

#define A(x)	(arglist->arg_array[(x)])

struct debug_arglist *
debug_add_to_arglist(LONG arg, struct debug_arglist *arglist)
{
    if (arglist)
    {
	if (arglist->n_args == MAX_ARGS - 1)
	    return arglist;
	
	arglist->n_args++;
    }
    else
    {
	arglist = (struct debug_arglist *) malloc(sizeof(*arglist));
	arglist->n_args = 1;
    }
    
    arglist->arg_array[arglist->n_args - 1] = arg;

    return arglist;
}

unsigned long
debug_call_function(unsigned long funcaddr, struct debug_arglist *arglist)
{
    unsigned long rv;
    unsigned long segoff;
    
    /* Call the function */
#ifdef DEBUG_DEBUG
    {
    int i;

    fprintf(debug_outfp, "Call ");
    TWIN_print_address(funcaddr, debug_outfp, 
		  guess_segment_offset(funcaddr) == 0 ? 32 : 16);
    fprintf(debug_outfp, "( ");
    for (i = 0; i < arglist->n_args; i++)
    {
	if (i > 0)
	    fprintf(debug_outfp, ", ");
	fprintf(debug_outfp, "%x", arglist->arg_array[i]);
    }
    fprintf(debug_outfp, " )\n");
    }
#endif
    segoff = guess_segment_offset(funcaddr);
    /* binary function */
    if (segoff)
    {
	fprintf(debug_outfp, "Binary functions may not be called, yet.\n");
	rv = 0;
    }
    
    /* native function */
    else
    {
	debug_funcptr funcptr = (debug_funcptr) funcaddr;
	
	rv = (*funcptr)(A(0), A(1), A(2), A(3), A(4), 
			A(5), A(6), A(7), A(8), A(9),
			A(10), A(11), A(12), A(13), A(14), 
			A(15), A(16), A(17), A(18), A(19));
    }

    /* Free the argument list */
    free(arglist);

    return rv;
}

unsigned long
debug_call_function_noargs(unsigned long funcaddr)
{
    unsigned long rv;
    unsigned long segoff;
    
    /* Call the function */
#ifdef DEBUG_DEBUG
    fprintf(debug_outfp, "Call ");
    TWIN_print_address(funcaddr, debug_outfp, 
		  guess_segment_offset(funcaddr) == 0 ? 32 : 16);
    fprintf(debug_outfp, "()\n");
#endif
    segoff = guess_segment_offset(funcaddr);
    /* binary function */
    if (segoff)
    {
	fprintf(debug_outfp, "Binary functions may not be called, yet.\n");
	rv = 0;
    }
    
    /* native function */
    else
    {
	debug_noarg_funcptr funcptr = (debug_noarg_funcptr) funcaddr;
	
	rv = (*funcptr)();
    }

    return rv;
}
