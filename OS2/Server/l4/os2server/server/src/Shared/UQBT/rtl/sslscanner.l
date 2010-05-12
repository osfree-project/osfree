/*
 * Copyright (C) 1997, Shane Sendall
 * Copyright (C) 1998-2001, The University of Queensland
 * Copyright (C) 2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:          sslscanner.l
 * OVERVIEW:      Defines a scanner to parse the tokens of a Semantic
 *                Specification Language specification.
 *
 * (C) 1997, Shane Sendall
 * Copyright (C) 1998-2001, The University of Queensland, BT group
 * Copyright (C) 2001, Sun Microsystems, Inc
 *============================================================================*/
 
/* 
 * Shane Sendall (original C version) Dec 1997
 * Doug Simon (C++ version) Jan 1998
 * 10 Feb 98, David - removed repeated #include that's already in rtl.h, 
 *                    due to conflict of <typeinfo> and <cmath>, code using 
 *                    the 'pow' function is replaced.
 * 8 Apr 98, David - added MODRM to the parser.
 * 28 Apr 98, David - added Flag RTLS to parser.
 * 23 Jun 98, Mike - Changes for semantic strings
 * 9 July 98, Mike - added TEMP token, else temp registers not distinguished
 *                   from PARMs, which they are not
 * 04 Nov 98, Mike - added ADDR token, for LEA instruction
 * 12 Dec 98, Shawn - completed additions of the float operators.
 * 12 Jan 99, Ian - Added new tokens "->", "COVERS" and "SHARES" for register
 *                  descriptors
 * 04 Mar 99 - Mike: Added CONV_FUNC (fsize, itof, ftoi), FPUSH, FPOP
 * 09 Mar 99 - Mike: Added TRANSCEND(), fround, floating point constants
 * 10 Mar 99 - Mike: SQRTs -> sqrt
 * 30 Mar 99, David - added InfiniteStack, @DEFINITION tokens.
 * 09 Jul 99 - Doug: Modified to remove need for *_base files
 * 19 Jan 00 - Mike: Added INTEGER and FLOAT keywords (for @REGISTER section);
 *              old FLOAT -> FLOATNUM
 * 31 Jan 01 - Mike: Removed @DEFINITION and Infinitestack tokens (UQDBT only)
 * 11 Feb 01 - Nathan: Added @OPERAND, changed PARM to allow uppercase letters
 * 15 Feb 01 - Nathan: Added support for explicit fetch-execute cycle
 * 05 Mar 01 - Simon: added lowercase 'integer' 'float' 'endianness' 'big'
 *              'little'
 * 05 Mar 01 - Simon: removed @REGISTERS @CONSTANTS @OPERANDS, added OPERAND
 * 05 Mar 01 - Simon: replaced "[alpha_num]\[IDX\]" with NAME_LOOKUP and
                introduced the more general NAME token and NAME_CALL
 * 05 Mar 01 - Simon: removed the necessity for an '_' for instruction names
 * 05 Mar 01 - Simon: replaced INSTRNAME with NAME
 * 20 Mar 01 - Mike: tmp's can have upper case letters now
 */

%name SSLScanner
 
%header{
#include "sslparser.h"
#define MAX_LINE 1024      // the longest SSL spec line
%}

// stuff to go in sslscanner.cc
%{
#include "sslscanner.h"
%}

%define CONSTRUCTOR_PARAM FILE* inFile, bool trace
%define CONSTRUCTOR_INIT : theLine(1), traceLines(trace)
%define CONSTRUCTOR_CODE  yyin = inFile;
%define MEMBERS \
public: \
    int theLine;        /* the current line number */\
    char lineBuf[MAX_LINE]; /* the current line */ \
    bool traceLines;    /* echo each lines as it is scanned */

%define LEX_PARAM YY_SSLParser_STYPE &yylval 

%%

"INTEGER"    |
"integer"    { return SSLParser::INTEGER; }
"FLOAT"      |
"float"      { return SSLParser::FLOAT; }
"OPERAND"    { return SSLParser::OPERAND; }
"ENDIANNESS" |
"endianness" {
               yylval.str = strdup(yytext);
               return SSLParser::ENDIANNESS;
             }
"BIG"        |
"big"        {
               yylval.str = strdup(yytext);
               return SSLParser::BIG;
             }
"LITTLE"     |
"little"     {
                yylval.str = strdup(yytext);
                return SSLParser::LITTLE;
             }
"COVERS"     { return SSLParser::COVERS; }
"SHARES"     { return SSLParser::SHARES; }
"FAST"       { return SSLParser::FAST; }
"FETCHEXEC"  { return SSLParser::FETCHEXEC; }

"*"[0-9]+"*" {
               yylval.str = strdup(yytext);
               return SSLParser::ASSIGNSIZE;
             }

"or"    |
"and"        {
               yylval.str = strdup(yytext);
               return SSLParser::LOG_OP;
             }
"="     |
"~="    |
"<"     |
">"     |
"<="    |
">="    |
"<u"    |
">u"    |
"<=u"   |
">=u"        {
               yylval.str = strdup(yytext);
               return SSLParser::COND_OP;
             }

"rlc"   |
"rrc"   |
"rl"    |
"rr"    |
">>"    |
"<<"    |
">>A"   |
"|"     |
"|~"    |
"&"     |
"&~"    |
"^"     |
"^~"  {
        yylval.str = strdup(yytext);
        return SSLParser::BIT_OP;

      }
"%"     |
"*"     |
"/"     |
"*!"    |
"/!"    |
"%!"    |
"+"     |
"-"   {
        yylval.str = strdup(yytext);
        return SSLParser::ARITH_OP;
      }
"*f"    |
"*fd"   |
"*fq"   |
"*fsd"  |
"*fdq"  |
"/f"    |
"/fd"   |
"/fq"   |
"+f"    |
"+fd"   |
"+fq"   |
"-f"    |
"-fd"   |
"-fq" {
        yylval.str = strdup(yytext);
        return SSLParser::FARITH_OP;
      }
"~"   {
        return SSLParser::NOT;
      }
"=>"  {
        return SSLParser::THEN;
      }
"->"  {
        return SSLParser::INDEX;
       }
":="   {
        return SSLParser::EQUATE;
      }
"::="  {
        yylval.str = strdup(yytext);
        return SSLParser::ASSIGN;
      }
".."  {
        return SSLParser::TO;
      }
":"   {
        return SSLParser::COLON;
      }
"!"   {
        return SSLParser::S_E;
      }
"@"   {
        return SSLParser::AT;
      }
"addr("     {
                return SSLParser::ADDR;
            }
"fsize("  |
"itof("   |
"ftoi("   |
"fround(" |
"truncu(" |
"truncs(" |
"zfill("  |
"sgnex("     {
                yylval.str = strdup(yytext);
                yylval.str[strlen(yylval.str)-1] = '\0';
                return SSLParser::CONV_FUNC;
            }
"defineflags(" |
"undefineflags(" {
                return SSLParser::FLAGMACRO;
                }
"FPOP"      {   return SSLParser::FPOP; }
"FPUSH"     {   return SSLParser::FPUSH; }
"sin("      |
"cos("      |
"tan("      |
"arctan("   |
"log2("     |
"loge("     |
"log10("    |
"execute("  |
"sqrt("     {
                yylval.str = strdup(yytext);
                yylval.str[strlen(yylval.str)-1] = '\0';
                return SSLParser::TRANSCEND;
            }
"tmp"[a-zA-Z0-9_]*  {
                yylval.str = strdup(yytext);
                return SSLParser::TEMP;
            }

"r["        {   return SSLParser::REG_IDX; }
"m["        {   return SSLParser::MEM_IDX; }
"_"         {   return yytext[0];   }
"%"[A-Za-z][A-Za-z0-9]* {
                yylval.str = strdup(yytext);
                return SSLParser::REG_ID;
            }
_?[A-Za-z][A-Za-z0-9_]*"(" {
                yylval.str = strdup(yytext);
                yylval.str[strlen(yylval.str)-1] = '\0';
                return SSLParser::NAME_CALL;
            }
_?[A-Za-z][A-Za-z0-9_]*"[" {
                yylval.str = strdup(yytext);
                yylval.str[strlen(yylval.str)-1] = '\0';
                return SSLParser::NAME_LOOKUP;
            }
_?[A-Za-z][A-Za-z0-9_]* {
                yylval.str = strdup(yytext);
                return SSLParser::NAME;
            }
"."[A-Za-z][A-Za-z.0-9]* |
"^"\"[A-Za-z][A-Za-z]*\" {
                    yylval.str = strdup(yytext);
                    return SSLParser::DECOR;
            }
[?,{}()'"]  {   return yytext[0];   }
"]"         {   return yytext[0];   }
"["         {   return yytext[0];   }
";"         {   return yytext[0];   }
"$"         {   return yytext[0];   }
-?[0-9]+"."[0-9]+ {
        yylval.dbl = atof(yytext);
        return SSLParser::FLOATNUM;
     }
-?[0-9]+ {
        yylval.num = strtol(yytext,0,10);
        return SSLParser::NUM;
     }

0x[A-F0-9]+ {
        yylval.num = strtol(yytext,0,16);
        return SSLParser::NUM;
     }

-?"2**"[0-9]+   {
                    // a power of two
                    int sign = (yytext[0] == '-' ? -1 : 1);
                    char* start = yytext + (sign == -1 ? 4:3);

                        // David - changed to << for more efficient and 
                        // not depend on the <cmath> anymore.
                    yylval.num = 1 << (strtol(start,0,10) * sign);
                        // yylval.num = (int)pow(2,strtol(start,0,10)) * sign;
                    return SSLParser::NUM;
                }

[ \t]+          ;
#.*$            ;   /* comment */
.               ;
\n.*            {   // funky print-next-line-for-debug token
                    if (yyleng > MAX_LINE)
                    {
                        cerr << theLine + 1 << ": line too long.\n";
                        exit(1);
                    }
                    // saves the next line
                    strcpy(lineBuf, yytext+1);
                    theLine++;
                    // reparse from char 1
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
