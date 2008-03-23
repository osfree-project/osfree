/* Lexical scanner for command line parsing in the Wine debugger
 *
 * Modification history:
 *    9/93 Eric Youngdale	Original version
 *    6/95 Bob Amstadt		Added seg:off addressing,
 *				Added ability to create aliases
 *				
 */

%{
#include <stdio.h>
#include <string.h>
#include "dbg.tab.h"
#include "regpos.h"
#include "windows.h"
#include "alias.h"

#ifdef USE_READLINE
#undef YY_INPUT
#define YY_INPUT(buf,result,max_size) \
	if ( (result = dbg_read((char *) buf, max_size )) < 0 ) \
	    YY_FATAL_ERROR( "read() in flex scanner failed" );
#endif

char *force_parse_string;
int   force_exit = 0;
int   lex_syntax_error;
int   debug_parse_literal = 0;
int   debug_parse_restofline = 0;

extern char * readline(char *);
static char * make_symbol(char *);
void flush_symbols();
extern int GetAddress(int,int);
extern int yylval;
extern WORD native_cs;
extern WORD native_ds;
extern WORD native_ss;
extern unsigned int * regval;
extern FILE *debug_outfp;
extern FILE *init_file;
int dbg_read(char * , int );
void add_history (char *);

%}

%x PARSELITERAL
%x ROL

DIGIT	[0-9]
HEXDIGIT [0-9a-fA-F]
IDENTIFIER [_a-zA-Z][_a-zA-Z0-9]*
QUOTEDSTRING \"[^\"]*\"

%%

	if ( debug_parse_literal )
	    BEGIN(PARSELITERAL);
        else if ( debug_parse_restofline )
            BEGIN(ROL);
        else
	    BEGIN(INITIAL);

<PARSELITERAL>[^ 	\n]* {
                                 yylval = (int) make_symbol(yytext); 
				 return LITERAL;
			     }

<ROL>[^\n]*		     { 
				 yylval = (int) make_symbol(yytext); 
				 return LITERAL;
			     }

<INITIAL,PARSELITERAL,ROL>\n { 
                                 lex_syntax_error = 0; 
				 debug_parse_literal = 0;
				 debug_parse_restofline = 0;
				 return '\n';  /* Indicate end of command */
			     }

"+"		{ return '+'; } 

"-"		{ return '-'; } 

"/"		{ return '/'; } 

"="		{ return '='; } 

"("		{ return '('; } 

")"		{ return ')'; } 

"*"		{ return '*'; } 

"?"		{ return HELP; }

":"		{ return ':'; }

"."		{ return '.'; }

","		{ return ','; }

"!"		{ return '!'; }

"!="		{ return NOTEQUAL; }

"#"		{ return '#'; }

{HEXDIGIT}+":"{HEXDIGIT}+ {
    		int seg, off;
		sscanf(yytext, "%x:%x", &seg, &off);
		yylval = GetAddress(seg, off);
		return NUM;
	        }

"$cs:"{HEXDIGIT}+ {
    		int off;
		sscanf(yytext, "cs:%x", &off);
                if (SC_CS != native_cs)
                    yylval = GetAddress(SC_CS, off);
                else
                    yylval = off;
		return NUM;
	        }

"$ds:"{HEXDIGIT}+ {
    		int off;
		sscanf(yytext, "ds:%x", &off);
                if (SC_DS != native_ds)
                    yylval = GetAddress(SC_DS, off);
                else
                    yylval = off;
		return NUM;
	        }

"$es:"{HEXDIGIT}+ {
    		int off;
		sscanf(yytext, "es:%x", &off);
                if (SC_ES != native_ds)
                    yylval = GetAddress(SC_ES, off);
                else
                    yylval = off;
		return NUM;
	        }

"$fs:"{HEXDIGIT}+ {
    		int off;
		sscanf(yytext, "fs:%x", &off);
                if (SC_FS != native_ds)
                    yylval = GetAddress(SC_FS, off);
                else
                    yylval = off;
		return NUM;
	        }

"$gs:"{HEXDIGIT}+ {
    		int off;
		sscanf(yytext, "gs:%x", &off);
                if (SC_GS != native_ds)
                    yylval = GetAddress(SC_GS, off);
                else
                    yylval = off;
		return NUM;
	        }

"$ss:"{HEXDIGIT}+ {
    		int off;
		sscanf(yytext, "ss:%x", &off);
                if (SC_SS != native_ss)
                    yylval = GetAddress(SC_SS, off);
                else
                    yylval = off;
		return NUM;
	        }

"0x"+{HEXDIGIT}+   {
		sscanf(yytext, "%x", &yylval);
		return NUM;
		}

{DIGIT}+   {
		sscanf(yytext, "%d", &yylval);
		return NUM;
		}

$pc		{ yylval = RN_EIP; return REG;}
$sp		{ yylval = RN_ESP; return REG;}
$eip		{ yylval = RN_EIP; return REG;}
$esp		{ yylval = RN_ESP; return REG;}
$ebp		{ yylval = RN_EBP; return REG;}
$eax		{ yylval = RN_EAX; return REG;}
$ebx		{ yylval = RN_EBX; return REG;}
$ecx		{ yylval = RN_ECX; return REG;}
$edx		{ yylval = RN_EDX; return REG;}
$esi		{ yylval = RN_ESI; return REG;}
$edi		{ yylval = RN_EDI; return REG;}

$es		{ yylval = RN_ES;  return REG;}
$ds		{ yylval = RN_DS;  return REG;}
$cs		{ yylval = RN_CS;  return REG;}
$ss		{ yylval = RN_SS;  return REG;}

info|inf|in		{ return INFO; }
segments|segm           { return SEGMENTS; }
break|brea|bre|br|bp    { return BREAK; }
enable|enabl|enab|ena   { return ENABLE;}
disable|disabl|disab|disa|dis { return DISABLE; }
memorypoint|memory|memor|memo|mem|mp	{ return MP; }
delete|delet|dele|del	{ return DEL; }
modules|module|modul|modu { return MODULES; }
exports|export|expor|expo { return EXPORTS; }
list                      { return LIST; }
all			  { return ALL; }
auto			  { return AUTO; }
alias			  { return ALIAS; }
condition|conditio|conditi|condit|condi|cond|con|co { return CONDITION; }
if			  { return IF; }
trace|trac|tra|tr	  { return TRACECMD; }

quit|qui|qu 	{ return QUIT; }

help|hel|he	{ return HELP; }

set|se		{ return SET; }

bt		{ return BACKTRACE; }

cont|con|co	{ return CONT; }
step|ste|st	{ return STEP; }
next|nex|ne	{ return NEXT; }

symbolfile|symbolfil|symbolfi|symbolf|symbol|symbo|symb { return SYMBOLFILE; }

define|defin|defi|def|de        { return DEFINE; }
abort|abor|abo         	        { return ABORT; }
print|prin|pri|pr		{ return PRINT; }

mode				{ return MODE; }

regs|reg|re	{ return REGS; }

parseexit	{ return PARSEEXIT; }

stack|stac|sta|st     	{ return STACK; }

p  		{ return 'p'; }
x  		{ return 'x'; }
d		{ return 'd'; }
i		{ return 'i'; }
w		{ return 'w'; }
b		{ return 'b'; }
s		{ return 's'; }
c		{ return 'c'; }
q		{ return 'q'; }
n		{ return 'n'; }

{IDENTIFIER}	{yylval = (int) make_symbol(yytext); 
	          return IDENTIFIER;
	         }

{QUOTEDSTRING}	{ char *p;
    		  yylval = (int) make_symbol(yytext+1); 
		  p = (char *) yylval;
		  p[strlen(p) - 1] = '\0';
	          return QUOTEDSTRING;
	         }

[ \t]+        /* Eat up whitespace */

.		{ if(lex_syntax_error == 0) 
		  {
		      lex_syntax_error ++; 
		      fprintf(debug_outfp, "Syntax Error\n"); 
		  }
	        }

%%

#ifndef yywrap
int yywrap(void) { return 1; }
#endif

#ifndef whitespace
#define whitespace(c) (((c) == ' ') || ((c) == '\t'))

/* Strip whitespace from the start and end of STRING. */
static void stripwhite (char *string)
{
    register int i = 0;

    while (whitespace (string[i]))
	i++;

    if (i)
	strcpy (string, string + i);

    i = strlen (string) - 1;

    while (i > 0 && whitespace (string[i]))
	i--;

    string[++i] = '\0';
}

int
dbg_read(char * buf, int size)
{
    char aliasbuf[MAX_ALIAS_LENGTH + 1];
    char *line;
    char *p;
    char *replace;
    int i;

    do
    {
	debug_parse_literal = 0;
	flush_symbols();
	if (force_parse_string)
	{
	    line = force_parse_string;
	    force_parse_string = NULL;
	}
	else if (force_exit)
	{
	    line = (char *) strdup("parseexit\n");
	    force_exit = 0;
	}
	else if (init_file)
	{
	    line = (char *) malloc(256);
	    if (fgets(line, 256, init_file) == NULL)
	    {
		strcpy(line, "parseexit\n");
		fclose(init_file);
		init_file = NULL;
	    }
	}
	    
	else
	{
	    line = readline ("dbg> ");
	}
	
	if (!line)
	    return 0;
	else
	{
	    /* Remove leading and trailing whitespace from the line.
	       Then, if there is anything left, add it to the history list
	       and execute it. */
	    stripwhite (line);
	    
	    if (*line)
	    {
		add_history (line);

		/* Is there an alias at the beginning of the line? */
		for (i = 0, p = line; 
		     *p && !whitespace(*p) && i < MAX_ALIAS_LENGTH;
		     i++)
		{
		    aliasbuf[i] = *p++;
		}
		aliasbuf[i] = '\0';

		if (i < MAX_ALIAS_LENGTH && 
		    (replace = alias_substitute(aliasbuf)))
		{
		    if (strlen(replace) + strlen(p) + 1 > size)
		    {
			fprintf(debug_outfp,"Fatal readline goof.\n");
			exit(0);
		    }

		    strcpy(buf, replace);
		    if (*p)
			strcat(buf, p);
		}
		else
		{
		    if (strlen(line) + 1 > size)
		    {
			fprintf(debug_outfp,"Fatal readline goof.\n");
			exit(0);
		    }
		    strcpy(buf, line);
		}

		i = strlen(buf);
		buf[i] = '\n';
		buf[i+1] = '\0';
		free(line);
		return strlen(buf);
	    }
	}
	
    } 
    while (1==1);
}

static char *local_symbols[10];
static int next_symbol;

char *make_symbol(char *symbol)
{
    return local_symbols[next_symbol++] = (char *) strdup(symbol);
}

void flush_symbols()
{
    while(--next_symbol>= 0) 
	free(local_symbols[next_symbol]);
    next_symbol = 0;
}

#endif
