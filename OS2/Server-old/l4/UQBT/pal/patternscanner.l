/*==============================================================================
 * FILE:          patternscanner.l
 * OVERVIEW:      Defines a scanner to parse the tokens of a pattern
 *                specification.
 *
 * Copyright (C) 1998-1999, The University of Queensland, BT group
 *============================================================================*/
 
%name PatternScanner

%define DEBUG 0
%define DEBUG_INIT 1

// stuff to go in patternscanner.h 
%header{
#include "patternparser.h"
#define MAX_LINE 1024   	// the longest pattern spec line
%}

// stuff to go in patternscanner.cc
%{
#include "patternscanner.h"
%}

%define CONSTRUCTOR_PARAM FILE* inFile, bool trace = false
%define CONSTRUCTOR_INIT : theLine(1), traceLines(trace)
%define CONSTRUCTOR_CODE  yyin = inFile;
%define MEMBERS \
public: \
	int theLine; 		/* the current line number */\
	char lineBuf[MAX_LINE];	/* the current line */ \
	bool traceLines;	/* echo each lines as it is scanned */

%define LEX_PARAM YY_PatternParser_STYPE &yylval 

%%

	/* literal keyword tokens */

"TYPES"      {
                 return PatternParser::TYPES;
             }
"PATTERNS"   {
                 return PatternParser::PATTERNS;
             }
"IS"         {
                 return PatternParser::IS;
             }
"NAMES"      {
                 return PatternParser::NAMES;
             }
"NONE"       {
                 return PatternParser::NONE;
             }
".."         {
                 return PatternParser::UPTO;
             }

"?"          {   return PatternParser::WILD; }
[><(),@{}=*;|] {   return yytext[0]; }
"$"          {   return yytext[0]; }
"["          {   return yytext[0]; }
"]"          {   return yytext[0]; }

[A-Za-z][A-Za-z0-9._]* {
                 yylval.str = strdup(yytext);
                 return PatternParser::IDENT;
             }
-?[0-9]+     {
                 yylval.num = strtol(yytext,0,10);
                 return PatternParser::NUM;
			}

0x[A-F0-9]+ {
                 // a hexadecimal number
                 yylval.num = strtol(yytext,0,16);
                 return PatternParser::NUM;
            }
[ \t]+     {
                 // white space at the beginning of a line
            }
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
			}
%%                
