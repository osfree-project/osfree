/* help.c
 */

#include <stdio.h>
#include "help.h"
#include "stdlib.h"
#include "string.h"

help_menu_entry_t help_running_menu[] =
{
    {
	"cont", "continue execution",
	"cont\n\nReturns to executing the program.",
	NULL
    },
    {
	"trace", "continue execution and trace instructions",
	"trace\n\n"
	"Returns to executing the program and displays each instruction\n"
	"as it is executed.\n",
	NULL
    },
    {
	"step", "execute next instruction",
	"step\n\nExecute next instruction.",
	NULL
    },
    {
	"next", "execute next instruction skip over function calls",
	"next\n\nExecute next instruction or entire function if the\n"
	"next instruction is a function call.",
	NULL
    },
    {
	"abort", "send an ABORT signal to the process",
	"abort\n\nImmediately sends an ABORT signal to the process.",
	NULL
    },
    { NULL, NULL, NULL, NULL }
};

help_menu_entry_t help_breakpoints_menu[] =
{
    {
	"break", "set breakpoint",
	"break ADDRESS\n\nSets a breakpoint at the specified address.",
	NULL
    },
    {
	"mp", "set memorypoint",
	"mp ADDRESS\n\n"
	"Sets a memorypoint at the specified address.  Memorypoints cause\n"
	"program execution to stop when the address is read or written.",
	NULL
    },
    {
	"enable", "enable a breakpoint or memorypoint",
	"enable break NUMBER\nenable mp NUMBER\n\n"
	"Enables the break/memory point number NUMBER.",
	NULL
    },
    {
	"disable", "disable a breakpoint or memorypoint",
	"disable break NUMBER\ndisable mp NUMBER\n\n"
	"Disables the break/memory point number NUMBER.",
	NULL
    },
    {
	"delete", "delete a breakpoint or memorypoint",
	"delete break NUMBER\ndelete mp NUMBER\n\n"
	"Deletes the break/memory point number NUMBER.",
	NULL
    },
    {
	"condition", "specify a condition for a breakpoint",
	"condition NUMBER CONDITION\n\n"
	"Specifies a necessary condition to exist for the program to stop.\n"
	"at breakpoint NUMBER.",
	NULL
    },
    { "info break", "display a list of all break/memory points.", NULL, NULL },
    { NULL, NULL, NULL, NULL }
};

help_menu_entry_t help_info_menu[] =
{
    {
	"stack", "display the contents of the stack",
	"info stack\n\nDisplay the contents of the stack",
	NULL
    },
    {
	"regs", "display the contents of the processor registers",
	"info regs\n\nDisplay the contents of the processor registers",
	NULL
    },
    {
	"segments", "show all of the defined segments",
	"info segments\n\nShow a list of all defined segments",
	NULL
    },
    {
	"break", "show all defined break/memory points",
	"info break\n\n"
	"Show a list of all defined breakpoints and memorypoints",
	NULL
    },
    {
	"modules", "show defined modules and exports",
	"info modules\n"
	"info modules exports MODULE\n"
	"info modules exports MODULE all\n\n"
	"The first of the above commands displays the list of all modules.\n"
	"The second displays a list of all exports in a module that have a\n"
	"name.  The third displays a list of all exports in a module.",
	NULL
    },
    { NULL, NULL, NULL, NULL }
};

help_menu_entry_t help_examining_menu[] =
{
    { 
	"info", "get information about various things", 
	"info SUBCOMMAND\n\nThese are the available info subcommands:\n", 
	help_info_menu 
    },
    { 
	"x", "examine memory at the specified address",
	"x ADDRESS\nx/FORMAT ADDRESS\n\n"
	"Displays the contents of memory at the specified address.  FORMAT\n"
	"can any of the following optionally preceded by a count:\n"
	"	i	display memory as machine instructions\n"
	"	s	display memory as a character string\n"
	"	x	display memory as hexidecimal integers\n"
	"	d	display memory as decimal integers\n"
	"	w	display memory as hexidecimal 16-bit values\n"
	"	b	display memory as hexidecimal 8-it values\n"
	"	c	display memory as ASCII characters\n",
	NULL 
    },
    { 
	"print", "print value",
	"x VALUE\nx/FORMAT VALUE\n\n"
	"Prints the value.  Note that print *VALUE is equivalent to\n"
	"x VALUE.  FORMAT can any of the following optionally preceded\n"
	"by a count:\n"
	"	i	display value as machine instructions\n"
	"	s	display value as a character string\n"
	"	x	display value as hexidecimal integers\n"
	"	d	display value as decimal integers\n"
	"	w	display value as hexidecimal 16-bit values\n"
	"	b	display value as hexidecimal 8-it values\n"
	"	c	display value as ASCII characters\n",
	NULL 
    },
    { 
	"bt", "print a stack backtrace", 
	"bt\n\nDisplays a stack backtrace", NULL 
    },
    { NULL, NULL, NULL, NULL }
};

help_menu_entry_t help_altering_menu[] =
{
    { 
	"set", "Alter a memory location or register",
	"set REGISTER = EXPRESSSION\n"
	"set *ADDRESS = EXPRESSION\n"
        "set SYMBOL = EXPRESSION\n\n"
        "The first form alters the contents of a register.  Registers are\n"
        "$eax, $ebx, $ecx, $edx, $esi, $edi, $eip, $esp, $cs, $ds, $es\n"
        "and $ss.  The second form changes the memory contents at location\n"
        "ADDRESS.  The third form changes the contents of the address\n"
        "indicated by the value of SYMBOL.",
	NULL
    },
    { NULL, NULL, NULL, NULL }
};

help_menu_entry_t help_miscellaneous_menu[] =
{
    { 
	"quit", "exit the program",
	"quit\n\nexit the program", 
	NULL
    },
    { 
	"symbolfile", "read another symbol file",
	"symbolfile FILENAME\n\nRead another file into the symbol table", 
	NULL
    },
    { 
	"define", "create a new symbol",
	"define SYMBOL VALUE\n\n"
	"Create a new symbol that has the specified value", 
	NULL
    },
    { 
	"mode", "change the disassembly mode",
	"mode auto\nmode 16\nmode 32\n\n"
	"Change the disassembly mode of the debugger", 
	NULL
    },
    { 
	"alias", "create a command alias",
	"alias NAME REPLACEMENT_STRING\n\n"
	"Creates a new command that executes the replacement string.", 
	NULL
    },
    { NULL, NULL, NULL, NULL }
};

help_menu_entry_t help_top_menu[] =
{
    { "#cont", NULL, NULL, help_running_menu },
    { "#trace", NULL, NULL, help_running_menu },
    { "#step", NULL, NULL, help_running_menu },
    { "#next", NULL, NULL, help_running_menu },
    { "#abort", NULL, NULL, help_running_menu },
    { "#break", NULL, NULL, help_breakpoints_menu },
    { "#mp", NULL, NULL, help_breakpoints_menu },
    { "#enable", NULL, NULL, help_breakpoints_menu },
    { "#disable", NULL, NULL, help_breakpoints_menu },
    { "#delete", NULL, NULL, help_breakpoints_menu },
    { "#condition", NULL, NULL, help_breakpoints_menu },
    { "#info", NULL, NULL, help_examining_menu },
    { "#x", NULL, NULL, help_examining_menu },
    { "#bt", NULL, NULL, help_examining_menu },
    { "#print", NULL, NULL, help_examining_menu },
    { "#set", NULL, NULL, help_altering_menu },
    { "#quit", NULL, NULL, help_miscellaneous_menu },
    { "#symbolfile", NULL, NULL, help_miscellaneous_menu },
    { "#define", NULL, NULL, help_miscellaneous_menu },
    { "#mode", NULL, NULL, help_miscellaneous_menu },
    { "#alias", NULL, NULL, help_miscellaneous_menu },
    {
	"running", "commands that affect execution",
	"The following commands are used to run or stop the program:\n",
	help_running_menu
    },
    {
	"breakpoints", "commands used to set/unset breakpoints",
	"The following commands are used to change breakpoints:\n",
	help_breakpoints_menu
    },
    {
	"examining", "commands that are used to display memory or state",
	"The following commands are used to display information:\n",
	help_examining_menu
    },
    {
	"altering", "commands used to alter memory or registers",
	"The following commands are used to alter memory or registers",
	help_altering_menu
    },
    {
	"miscellaneous", "other commands",
	NULL, help_miscellaneous_menu
    },
    {
	"initializing", "setting up the debugger the way you want it.",
	"When the debugger is initialized by the library, it attempts\n"
	"to execute the commands from an initialization file.  It will\n"
	"only run one file, but it will try to find it in one of two\n"
	"locations.  The first place it tries is \"./.windbginit\".\n"
	"If that file does not exist, then it tries \"$(HOME)/.windbginit\".\n"
	"The commands in the initialization file are executed as if they\n"
	"were typed on the command line.", NULL
    },
    { NULL, NULL, NULL, NULL }
};

static help_arglist_t *help_arg_head;

void help_free_args()
{
    help_arglist_t *ha;
    help_arglist_t *trash;
    
    for (ha = help_arg_head; ha; )
    {
	trash = ha;
	ha = ha->next;
	free(trash->arg);
	free(trash);
    }

    help_arg_head = NULL;
}

help_arglist_t *debug_help_add_arg(help_arglist_t *head, char *arg)
{
    help_arglist_t *ha;
    help_arglist_t *tail;
    
    ha = (help_arglist_t *) malloc(sizeof(help_arglist_t));
    ha->next = NULL;
    ha->arg = (char *) strdup(arg);

    if (!head)
    {
	if (help_arg_head)
	    help_free_args();
	
	help_arg_head = ha;
	head = ha;
    }
    else
    {
	for (tail = head; tail->next; tail = tail->next)
	    ;
	
	tail->next = ha;
    }
    
    return head;
}

void debug_help(help_arglist_t *head)
{
    help_arglist_t *ha;
    help_menu_entry_t *me;
    help_menu_entry_t *top = help_top_menu;
    char *text;

    if (!head)
    {
	text = "Help is available on the following topics:\n";
	me = help_top_menu;
    }
    else
    {
	for (ha = head; ha; ha = ha->next)
	{
	    for (me = top; me->keyword; me++)
	    {
		if (me->keyword[0] == '#' &&
		    strncasecmp(me->keyword+1, ha->arg, strlen(ha->arg)) == 0)
		{
		    me = me->submenu;
		}

		if (strncasecmp(me->keyword, ha->arg, strlen(ha->arg)) == 0)
		    break;
	    }
	    
	    if (!me->keyword)
	    {
		fprintf(stderr, "No help on that topic.\n");
		return;
	    }
	
	    top = me->submenu;
	}

	text = me->help_text;
	me = me->submenu;
    }

    if (text)
	fprintf(stderr, "%s\n", text);

    for ( ; me && me->keyword; me++)
	if (me->description)
	    fprintf(stderr, "%-15s %s\n", me->keyword, me->description);

    help_free_args();
}
