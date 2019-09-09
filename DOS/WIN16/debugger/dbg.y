%{

/* Parser for command lines in the Wine debugger
 *
 * Modification history:
 *    9/93 Eric Youngdale	Original version
 *    6/95 Bob Amstadt		Added ability to call functions,
 *				Added seg:off addressing,
 *				Improved auto 16/32 selection,
 *				Added ability to create aliases
 *				
 */

#include <stdio.h>
#include <signal.h>
#include "debugger.h"

#define YYSTYPE int

#include "regpos.h"
extern FILE * TWIN_dbg_in;
extern char *force_parse_string;
extern int debug_parse_literal;
extern int debug_parse_restofline;

unsigned int * regval = NULL;
unsigned int dbg_mask = 0;
unsigned int dbg_mode = 0;
unsigned int auto_dbg_mode = 1;
unsigned int issue_return_message;
FILE *debug_outfp = stderr;
FILE *trace_fp = NULL;
FILE *init_file;

static unsigned int current_address;
static          int last_bpnum;

void issue_prompt(void);
void mode_command(int);

void insert_break(int);
int  break_is_stepping();
int  should_continue(int);
int  get_bpnum(unsigned int);
int  break_continue();
int  break_is_tracing();
int  break_is_trace_mode(int);
int  flush_symbols();

extern void debuggerbreak();

int TWIN_dbg_error(char * s);

void break_step(int);
void break_trace(int);

void read_symboltable(char *);
void add_hash(int,int);
void add_break(char *,int);
void enable_break(char *,int);
void delete_break(char *);
void add_memorypoint(char *,int);
void enable_memorypoint(char *,int);
void delete_memorypoint(char *);
void backtrace();
void alias_change_alias(char *,char *);

void break_condition(int ,char *);
void examine_memory(int,int,char);
void application_not_running();

HINSTANCE find_module(char *);
char * translate_ordinal(HMODULE, int);
unsigned int translate_export(HMODULE, char *);

int find_module_segment(HMODULE,int);
unsigned int * find_export(char *);

unsigned int * find_hash(char *);

int debug_add_to_arglist();
int debug_call_function();
int debug_call_function_noargs();
void info_reg();
void info_stack();
void PrintLDT();

void info_break();
void info_modules();
void info_exports(HMODULE,BOOL);

void debug_help(void *);
int debug_help_add_arg(void *,char *);

%}


%token CONT
%token STEP
%token NEXT
%token QUIT
%token HELP
%token BACKTRACE
%token INFO
%token STACK
%token SEGMENTS
%token REG
%token REGS
%token NUM
%token ENABLE
%token DISABLE
%token DEL
%token BREAK
%token SET
%token MODE
%token PRINT
%token IDENTIFIER
%token NO_SYMBOL
%token SYMBOLFILE
%token DEFINE
%token ABORT
%token MP
%token MODULES
%token EXPORTS
%token LIST
%token ALL
%token AUTO
%token ALIAS
%token QUOTEDSTRING
%token CONDITION
%token IF
%token PARSEEXIT
%token TRACECMD
%token LITERAL

%left '=' NOTEQUAL
%left '-' '+'
%left '*' '/'
%left UNARY


%%

input:  /* empty */
	| input line  { issue_prompt(); }

line:	'\n'
	| function
	| PARSEEXIT '\n'		{ return 0; }
	| infocmd '\n'
	| error '\n'       		{ yyerrok; }
	| QUIT  '\n'       		{ exit(0); }
	| 'q' '\n'         		{ exit(0); }
	| helpcmd '\n'
	| CONT '\n'        		{ issue_return_message = 1; return 0; }
	| 'c' '\n'         		{ issue_return_message = 1; return 0; }
	| STEP '\n'	   		{ break_step(0); return 0; }
	| 's' '\n'	   		{ break_step(0); return 0; }
	| NEXT '\n'	   		{ break_step(1); return 0; }
	| 'n' '\n'	   		{ break_step(1); return 0; }
	| TRACECMD trace_args '\n'	{ break_trace($2); return 0; }
	| 't' trace_args '\n'		{ break_trace($2); return 0; }
	| ABORT '\n'       		{ kill(getpid(), SIGABRT); }
	| SYMBOLFILE IDENTIFIER '\n' 	{ read_symboltable((char *)$2); }
	| DEFINE IDENTIFIER expr '\n'  	{ add_hash($2, $3); }
	| MODE NUM	   		{ mode_command($2); }
	| MODE AUTO	   		{ mode_command(0); }
	| BREAK expr '\n'  		{ add_break((char *)$2, 0); }
	| ENABLE BREAK NUM		{ enable_break((char *)$3, 1); }
	| DISABLE BREAK NUM		{ enable_break((char *)$3, 0); }
	| DEL BREAK NUM 		{ delete_break((char *)$3); }
	| MP expr      	   		{ fprintf(stderr, 
						  "Use 'mp w' or 'mp r'\n");
					}
        | MP 'w' expr      	   	{ add_memorypoint((char *)$3, 'w'); }
        | MP 'r' expr      	   	{ add_memorypoint((char *)$3, 'r'); }
	| ENABLE MP NUM			{ enable_memorypoint((char *)$3, 1); }
	| DISABLE MP NUM		{ enable_memorypoint((char *)$3, 0); }
	| DEL MP NUM    		{ delete_memorypoint((char *)$3); }
	| x_command
	| BACKTRACE '\n'   		{ backtrace(); }
	| print_command
	| deposit_command
	| ALIAS IDENTIFIER 		{ debug_parse_restofline = 1; }
	  LITERAL 			{ 
	                                    debug_parse_restofline = 0;
					    alias_change_alias((char *)$2, (char *)$4); 
					}
	| CONDITION NUM 		{ debug_parse_restofline = 1; }
	  LITERAL 			{ 
	                                    debug_parse_restofline = 0;
					    break_condition($2, (char *)$4); 
					}
	| IF expr	{ if (!($2)) 
			      return 0; 
			  else
			  {
			      fprintf(debug_outfp, "Stopped on breakpoint %d\n", 
				      last_bpnum);
			      examine_memory(current_address, 1, 'i');
			  }
		        }

trace_args: /*empty*/			{ $$ = 0; }
	| NUM				{ $$ = $1; }

deposit_command:
	SET REG '=' expr '\n' { if(regval) 
				    regval[$2] = $4; 
	                        else 
				    application_not_running();
			      }
	| SET '*' expr '=' expr '\n' { *((unsigned int *) $3) = $5; }
	| SET symbol '=' expr '\n' { *((unsigned int *) $2) = $4; }


x_command:
	  'x' expr  '\n' { examine_memory($2, 1, 'x'); }
	| 'x' '/' fmt expr  '\n' { examine_memory($4, 1, $3); }
	| 'x' '/' NUM fmt expr  '\n' { examine_memory($5, $3, $4); }

print:
	  'p'
	| PRINT
	
print_command:
	  print expr '\n' { examine_memory(((unsigned int) &$2 ), 1, 'd'); }
	| print '/' fmt expr '\n' { 
	    examine_memory((unsigned int) &$4, 1, $3); 
	}
	| print '/' NUM fmt expr '\n' { 
	    examine_memory((unsigned int) &$5, $3, $4); 
	}

fmt:  'x'     { $$ = 'x'; }
	| 'd'  { $$ = 'd'; }
	| 'i'  { $$ = 'i'; }
	| 'w'  { $$ = 'w'; }
	| 's'  { $$ = 's'; }
	| 'c'  { $$ = 'c'; }
	| 'b' { $$ = 'b'; }

module: NUM                     { $$ = $1; }
 	| IDENTIFIER            { $$ = find_module((char *)$1);
                                  if ($$ < 0) {
				      fprintf(debug_outfp, "Module %s not found\n",
					      (char *)$1);
				      YYERROR;
				  }
	                        }

symbol: module '.' NUM		{ $$ = (int)translate_ordinal($1, $3); }
	| module '.' IDENTIFIER	{ $$ = (int)translate_export($1, (char *)$3);
                                  if ($$ == 0xffffffff) {
				      fprintf(debug_outfp,
					      "Symbol %s not found\n", (char *)$1);
				      YYERROR;
				  }
	                        }
	| module '#' NUM        { $$ = find_module_segment($1, $3); }
	| IDENTIFIER		{ $$ = (int) find_export((char *)$1);
                                  if ($$ == 0xffffffff) {
				      $$ = (int)find_hash((char *)$1);
				      if ($$ == 0xffffffff) {
					  fprintf(debug_outfp,
						  "Symbol %s not found\n", (char *)$1);
					  YYERROR;
				      }
				  }
	                        }

arglist: arglist ',' expr	{ $$ = debug_add_to_arglist($3, $1); }
	| expr			{ $$ = debug_add_to_arglist($1, NULL); }

function: symbol '(' arglist ')' { $$ = debug_call_function($1, $3); }
	| symbol '(' ')' 	 { $$ = debug_call_function_noargs($1); }
    
expr:	 NUM			{ $$ = $1;	}
	| REG			{ $$ = regval[$1]; }
	| REG ':' REG		{ if ($1 > RN_DS && $1 != RN_CS && $1 != RN_SS)
				  {
				      fprintf(debug_outfp, "Non-segment register "
					      "used for segment value.\n");
				      YYERROR;
				  }
				  $$ = (int) GetAddress(regval[$1], 
							regval[$3]);
			        }
	| '.'			{ $$ = current_address; }
	| symbol   		{ $$ = $1; }
	| function		{ $$ = $1; }
	| QUOTEDSTRING		{ $$ = $1; }
	| expr '+' expr		{ $$ = $1 + $3; }
	| expr '-' expr		{ $$ = $1 - $3; }
	| expr '*' expr		{ $$ = $1 * $3; }
	| expr '/' expr		{ $$ = $1 / $3; }
	| expr '=' expr		{ $$ = ($1 == $3); }
	| expr NOTEQUAL expr	{ $$ = ($1 != $3); }
    	| '(' expr ')'		{ $$ = $2; }
	| '*' expr %prec UNARY	{ $$ = *((unsigned int *) $2); }
	| '-' expr %prec UNARY	{ $$ = -$2; }
	| '!' expr %prec UNARY	{ $$ = !$2; }
     
	
infocmd: INFO REGS     { info_reg(); }
	| INFO STACK    { info_stack(); }
	| INFO SEGMENTS { PrintLDT(); }
	| INFO BREAK    { info_break(); }
	| INFO MP	{ info_break(); }
        | INFO MODULES LIST               { info_modules(); }
        | INFO MODULES EXPORTS module ALL { info_exports($4, 1); }
        | INFO MODULES EXPORTS module     { info_exports($4, 0); }

helpcmd: HELP 			{
				  debug_parse_literal = 1; 
			        } 
	 helpargs		{ 
	     			  debug_parse_literal = 0; 
				  debug_help((void *)$3); 
			        }

helpargs: /*empty*/		{ $$ = 0; }
	| helpargs LITERAL	{ $$ = debug_help_add_arg((char *)$1, (char *)$2); }
	| LITERAL		{ $$ = debug_help_add_arg(0, (char *)$1); }
     

%%

void 
issue_prompt()
{
#ifndef USE_READLINE
    fprintf(debug_outfp,"dbg> ");
#endif
}

void mode_command(int newmode)
{
    if(newmode == 16)
    {
	dbg_mask = 0xffff;
	dbg_mode = 16;
	auto_dbg_mode = 0;
	return;
    }
    if(newmode == 32)
    { 
	dbg_mask = 0xffffffff;
	dbg_mode = 32;
	auto_dbg_mode = 0;
	return;
    }
    if(newmode == 0)
    { 
	auto_dbg_mode = 1;
	return;
    }
    
    fprintf(debug_outfp,"Invalid mode (use 16, 32 or AUTO)\n");
}

extern WORD native_cs;
extern WORD native_ds;
static int loaded_symbols = 0;

void
wine_debug_init()
{
    char fname[256];
    
    init_file = fopen(".windbginit", "r");
    if (!init_file)
    {
	sprintf(fname, "%s/.windbginit", getenv("HOME"));
	init_file = fopen(fname, "r");
    }
    
    if (init_file)
	debuggerbreak();
}

void
wine_debug(int signal, int * regs)
{
    static int dummy_regs[32];
    HWND capture_wnd;
    unsigned int addr;
#ifdef YYDEBUG
    TWIN_dbg_debug = 0;
#endif

    TWIN_dbg_in = stdin;
    regval = regs ? regs : dummy_regs;

    if (SC_CS == native_cs || GetSelectorType(SC_CS) == (ST_CODE32)) 
    {
	dbg_mask = 0xffffffff;
	dbg_mode = 32;
    } 
    else 
    {
	dbg_mask = 0xffff;
	dbg_mode = 16;
    }

    /* Where are we? */
    addr = SC_EIP(dbg_mask);
    if (SC_CS != native_cs)
	addr = (unsigned int) GetAddress(SC_CS, SC_EIP(dbg_mask));

    current_address = addr;

    /* This is intended to read the entry points from the Windows image, 
       and insert them in the hash table.  It does not work yet, so it 
       is commented out. 
     */
    if(!loaded_symbols)
    {
	loaded_symbols++;
	read_symboltable("win.sym");
    }

    /* Remove the breakpoints from memory... */
    insert_break(0);

    /* If we stopped on a breakpoint, report this fact */
    if (signal == SIGTRAP)
    {
	int bpnum;
	int stepping = break_is_stepping();

	if(should_continue(bpnum = get_bpnum(addr)))
	{
	    insert_break(1);
	    return;
	}

	if (bpnum < 0)
	{
	    regval[RN_EIP]++;
	    addr++;
	    if (bpnum > -5)
	    {
		fprintf(debug_outfp, "Stopped on memorypoint %d\n", -bpnum - 1);

		/* Stop tracing */
		break_continue();
	    }
	}
	else
	{ 
	    if (!stepping && !force_parse_string)
		fprintf(debug_outfp, "Stopped on breakpoint %d\n", bpnum);

	    /* Stop tracing */
	    break_continue();
	}
	
	last_bpnum = bpnum;
    }
    else
    {
	/* Stop tracing */
	break_continue();
    }

    /* Shut off single step mode */
    if (!break_is_tracing())
    {
	SC_EFLAGS &= ~0x0100;
	break_continue();
    }

    /* Show where we crashed */
    if (regs && !force_parse_string && 
	(!break_is_tracing() || break_is_trace_mode(dbg_mode)))
	examine_memory(addr, 1, 'i');

    if (break_is_tracing())
    {
	if (!trace_fp)
	    trace_fp = fopen("xwin.trace", "w");
	
	if (trace_fp && break_is_trace_mode(dbg_mode))
	{
	    debug_outfp = trace_fp;
	    examine_memory(addr, 1, 'i');
	    debug_outfp = stderr;
	}
    }
    else if (trace_fp)
    {
	fclose(trace_fp);
	trace_fp = NULL;
    }

    /* By default, don't show the "Returning..." when we continue */
    issue_return_message = 0;

    /* Enter the parser */
    if (!break_is_tracing())
    {
	capture_wnd = GetCapture();
	if (capture_wnd)
	    ReleaseCapture();

	issue_prompt();
	TWIN_dbg_parse();
	flush_symbols();

	if (capture_wnd)
	    SetCapture(capture_wnd);
    }

    /* Re-insert the breakpoints from memory... */
    insert_break(1);

    if (issue_return_message)
	fprintf(debug_outfp,"Returning...\n");
}


int TWIN_dbg_error(char * s)
{
    fprintf(debug_outfp,"%s\n", s);
    return 0;
}

