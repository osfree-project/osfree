/*==============================================================================
 * FILE:          csrscanner.l
 * OVERVIEW:      Defines a scanner to parse the tokens of a Call Signature
 *                Recovery (CSR) specification; what is now called the 
 *                Procedural Abstraction Language (PAL) specification.
 *
 * Copyright (C) 1998-2001, The University of Queensland, BT group
 *============================================================================*/
 
/*
 * $Revision: 1.6 $
 * 08 Feb 00 - Mike: Added the GLOBAL keyword
 * 22 Feb 01 - Mike: Added the GLOBALOFFSET keyword
 * 17 Aug 01 - Mike: Extra keywords for floating point register parameters
 */

%name CSRScanner

%define DEBUG 0
%define DEBUG_INIT 1

// stuff to go in csrscanner.h 
%header{
#include "csrparser.h"
#define MAX_LINE 1024   	// the longest CSR spec line
%}

// stuff to go in csrscanner.cc
%{
#include "csrscanner.h"
%}

%define CONSTRUCTOR_PARAM FILE* inFile, bool trace /*= false */
%define CONSTRUCTOR_INIT : theLine(1), traceLines(trace)
%define CONSTRUCTOR_CODE  yyin = inFile;
%define MEMBERS \
public: \
	int theLine; 		/* the current line number */\
	char lineBuf[MAX_LINE];	/* the current line */ \
	bool traceLines;	/* echo each lines as it is scanned */

%define LEX_PARAM YY_CSRParser_STYPE &yylval 

%%

	/* literal keyword tokens */

"INTEGER"    {
                 return CSRParser::INTEGER;
             }
"ADDRESS"    {
                 return CSRParser::ADDRESS;
             }
"FLOAT"      {
                 return CSRParser::FLOAT;
             }
"DOUBLE"      {
                 return CSRParser::DOUBLE;
             }
"AGGREGATE"  {
                 return CSRParser::AGGREGATE;
             }
"FRAME"      {
                 return CSRParser::FRAME;
             }
"GLOBALOFFSET" {
                 return CSRParser::GLOBALOFFSET;
             }
"GLOBAL"     {
                 return CSRParser::GLOBAL;
             }
"ABSTRACTION" {
                 return CSRParser::ABSTRACTION;
             }
"LOCALS"     {
                 return CSRParser::LOCALS;
             }
"INIT"       {
                 return CSRParser::INIT;
             }
"PARAMETERS" {
                 return CSRParser::PARAMETERS;
             }
"REGISTERS"  {
                 return CSRParser::REGISTERS;
             }
"STACK"      {
                 return CSRParser::STACK;
             }
"BASE"       {
                 return CSRParser::BASE;
             }
"OFFSET"     {
                 return CSRParser::OFFSET;
             }
"RETURNS"    {
                 return CSRParser::RETURNS;
             }
"IN"         {
                 return CSRParser::IN;
             }
"PARENT"     {
                 return CSRParser::PARENT;
             }
"TO"         {
                 return CSRParser::TO;
             }
"STEP"       {
                 return CSRParser::STEP;
             }
"IS"         {
                return CSRParser::IS;
             }
"INVERTED"   {
                return CSRParser::INVERTED;
             }
"NORMAL"     {
                return CSRParser::NORMAL;
             }
"ALIGNMENT"  {
                return CSRParser::ALIGNMENT;
             }
"BYTES"      {
                return CSRParser::BYTES;
             }
"->"         {
                 return CSRParser::DEFINED_BY;
             }
"m["         {
                 return CSRParser::MEM_IDX;
             }
"%"[a-z][a-z0-9]* {
                 yylval.str = strdup(yytext);
                 return CSRParser::REG_ID;
		     }
"*"  |
"/"  |
"+"  |
"-"          {
                 yylval.str = strdup(yytext);
                 return CSRParser::ARITH_OP;
             }
[(),.@{}=]  {    return yytext[0];	}
"]"         {    return yytext[0];	}
"["         {    return yytext[0];	}


[A-Za-z][A-Za-z0-9_]* {
				yylval.str = strdup(yytext);
				return CSRParser::IDENT;
			}

-?[0-9]+    {
                // a decimal number
                yylval.num = strtol(yytext,0,10);
                return CSRParser::NUM;
			}

0x[A-F0-9]+ {
                // a hexadecimal number
                yylval.num = strtol(yytext,0,16);
                return CSRParser::NUM;
            }
^[ \t]+     {
                // white space at the beginning of a line
            }
[ \t]+      ;

#.*$        ;
.           ;
\n.*        {
				if (yyleng > MAX_LINE) {
			       	cerr << theLine + 1 << ": line too long.\n";
				exit(1);
                }

                // saves the next line (excluding any leading white space)
				strcpy(lineBuf, yytext+1+strspn(yytext+1," \t"));
				theLine++;
                                // give back all but the \n to rescan
				yyless(1);

				// echo the line if line tracing is turned on
				if (traceLines)
				cerr << lineBuf << endl;

                // Quell a warning
                yy_last_accepting_state = 0;
                yy_last_accepting_cpos = 0;

			}
%%
