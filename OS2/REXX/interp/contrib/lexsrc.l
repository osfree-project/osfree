%{
/*
 *  The Regina Rexx Interpreter
 *  Copyright (C) 1992-1994  Anders Christensen <anders@pvv.unit.no>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
/*
 * If you change this file the following has to be done:
 * - regenerate lexsrc.c using an old version of Flex: 2.5.4. This is
 *   provided by flex-old package. Run: make lexsrc
 * - change the value of YY_BUF_SIZE to 101000 in lexsrc.c
 *   This value MUST be larger than BUFFERSIZE in rexx.h
 *   It is not set automatically because certain variables
 *   are defined with this value BEFORE rexx.h is included
 *   Increasing this value will slow down parsing a bit.
 *   This value will be blown away if you regenerate lexxsrc.c
 *   from this file.
 */

#include "rexx.h"
#include "yaccsrc.h"
#include <string.h>
#include <assert.h>
#include <errno.h>

/* Define ASCII_0_TERMINATES_STRING if you want that ASCII-0 terminates
 * an input string. Normally this should not happen. Input strings are
 * terminated by a length encoding. The string {"", length=1} is invalid for
 * the lexer (ASCII-0 is not allowed) while {"", length=0} is allowed (this
 * is an empty input).
 * ASCII_0_TERMINATES_STRING is only(!) for backward compatibility and
 * shouldn't be used under normal circumstances.
 * FGC
 */
#define ASCII_0_TERMINATES_STRING

#ifdef YYLMAX
# undef YYLMAX
#endif
#define YYLMAX BUFFERSIZE

#ifdef FLEX_SCANNER
#undef YY_CHAR
#define YY_CHAR YY_CHAR_TYPE
#undef YY_INPUT
#define YY_INPUT(buf,result,max_size) result=fill_buffer(buf,max_size)
#endif

/* NOTE: Every comment is replaced by a '`' character in the lower input
 * routines. These should check for such (illegal) characters.
 */
#define MY_ISBLANK(c) (((c)==' ')||((c)=='\t')||((c)=='\v')||((c)=='\f')|| \
                       ((c)=='\r')||((c)=='`'))

PROTECTION_VAR(regina_parser)
/* externals which are protected by regina_parser */
internal_parser_type parser_data = {NULL, };
int retlength=0 ;
char retvalue[BUFFERSIZE] ;
unsigned SymbolDetect = 0;
/* end of externals protected by regina_parser */

/* locals, they are protected by regina_parser, too */
static int nextline = 1;
static int nextstart = 1;
static int do_level = 0 ;
static int in_numform=0, next_numform=0 ;
static int obs_with=0, in_do=0, in_then=0;
static int in_parse=0 ;
static int in_trace=0, itflag=0 ;
static int in_signal=0, in_call=0 ;
static enum { not_in_address = 0,
              in_address_keyword, /* ADDRESS just seen */
              in_address_main, /* after the first word */
              in_address_value, /* like main but VALUE was seen */
              in_address_with} in_address = not_in_address,
                               last_in_address = not_in_address ;
static enum {no_seek_with = 0,
             seek_with_from_parse,
             seek_with_from_address} seek_with = no_seek_with ;
/*
 * expression_ended is set if a typical expression has ended and the next
 * token has to be preceeded by a CONCATENATE (abuttal) operator in most
 * cases.
 */
static int expression_ended=0;

/*
 * insert_abuttal is set if the last token was a possible end of an expression.
 * The next token may need to be preceeded by an additional CONCATENATE
 * operator under some conditions like not being "in_parse".
 */
static int insert_abuttal=0;

/*
 * A symbol may be pending after a CONTATENATE (abuttal) operator has been
 * returned. This symbol will be stored here. 0 indicates no pending symbol.
 * ASCII 0 will never been returned, there is no need for another indicator.
 */
static int delayed_symbol=0;

/*
 * inhibit_delayed_abuttal can be set only if delayed_symbol has been set,
 * but not always. An opening parenthesis must have been detected, too.
 * It inhibits the use of a CONCATENATE (abuttal) operator after the staring
 * parenthesis. Examples:
 * CONCATENATE {var_symbol}     <-- needs a CONCATENATE, another {var_symbol}
 *                                  may follow.
 * CONCATENATE {var_symbol} "(" <-- needs a CONCATENATE, another {var_symbol}
 *                                  is an expression's start.
 */
static int inhibit_delayed_abuttal=0;

static char ch;
static int kill_this_space=0, kill_next_space=1 ;
static int extnextline = -1, extnextstart; /* for a delayed line increment */
static int linenr=1 ;
static int contline = 0;
static int inEOF=0 ;
static int singlequote=0, doblequote=0 ;
static int firstln=0 ;
static int bufptr=0 ;
/* Previous bug. 8-bits clean combined with EOF ==> need an int */
static int chbuffer[LOOKAHEAD] ;
static int ipretflag=0, cch=0 ;
static const char *interptr=NULL ;
static const char *last_interptr=NULL ;
static const char *interptrmax ;
static int cchmax = 0 ;

static YY_CHAR_TYPE *rmspc( YY_CHAR_TYPE *instr ) ;
static void set_assignment( char delim );
static int process_number_or_const( const char *text, int len );
static int process_hex_or_bin_string( char *text, int len, int base );
static void compress_string( char *dest, const char *src );
static int fill_buffer( char *buf, int max_size ) ;
#define SET_NEXTSTART() (nextstart += yyleng)

#define YY_FATAL_ERROR(s) exiterror( ERR_PROG_UNREADABLE, 1, s )
%}


%start comm signal sgtype procd parse then with
%start numeric do1 other value1 ifcont signame nmform

%e 2500
%p 17000
%k 1500
%a 7000
%n 1000
%o 8000

%{
%}


/*
 * ANSI 6.2.2 definitions (partially with extensions)
 */
digit             [0-9]
special           [,:;)(]
not               [\\^~\xAC\xAA]
extra             [#$@]
general           [_!?A-Za-z]|{extra}
dot               [.]

                  /*
                   * sigh, we need the reversed classes. flex isn't smart
                   * enough to provide it.
                   * Added "0-9" to notGeneral... to fix bug 724390
                   */
notGeneralDot     [^_!?A-Za-z0-9#$@.]
notGeneralParen   [^_!?A-Za-z0-9#$@.(]

space             [ \t]
blank             [\f\v\r]|{space}
EOL               ;|\r?\n

                  /*
                   * A comment is converted to a sequence of ` signs in the
                   * lower layer
                   */
comment           [`]
between           ({blank}|{comment})*

var_symbol_c      {dot}|{digit}|{general}
var_symbol        {general}{var_symbol_c}*
                  /*
                   * A number is a little bit different to detect signs in the
                   * exponent
                   */
const_symbol      ({dot}|{digit}){var_symbol_c}*
reserved_symbol   {dot}[a-zA-Z]+
symbol            {var_symbol}|{const_symbol}

mantissa          ({dot}?{digit}+)|({digit}+{dot}{digit}*)
exponent          [eE][-+]?{digit}+
                  /*
                   * read ANSI 6.2.1.1 carefully. We have to check the chars
                   * after the exponent for notGeneralDot below.
                   */
number            {mantissa}{exponent}?

hex_digit         {digit}|[a-fA-F]
bin_digit         [0-1]
hex_digit_tuple   {blank}*{hex_digit}{hex_digit}
bin_digit_tuple   {blank}*{bin_digit}{bin_digit}{bin_digit}{bin_digit}

hex_string_start  {hex_digit}{hex_digit}?
hex_string        ({hex_string_start}{hex_digit_tuple}*)?
bin_string_start  {bin_digit}({bin_digit}({bin_digit}{bin_digit}?)?)?
bin_string        ({bin_string_start}{bin_digit_tuple}*)?

text_string       '([^']|'')*'|\"([^"]|\"\")*\"
mintext_string    '([^']|'')+'|\"([^"]|\"\")+\"

                  /*
                   * Allow characters for any token in string passed back to TRACE instruction
                   * Fixes bug 3564586.
                   */
trace_char        {symbol}

                  /*
                   * The list of keywords.
                   */
key_address       [aA][dD][dD][rR][eE][sS][sS]
key_arg           [aA][rR][gG]
key_by            [bB][yY]
key_call          [cC][aA][lL][lL]
key_caseless      [cC][aA][sS][eE][lL][eE][sS][sS]
key_digits        [dD][iI][gG][iI][tT][sS]
key_do            [dD][oO]
key_dotline       {dot}[lL][iI][nN][eE]
key_drop          [dD][rR][oO][pP]
key_else          [eE][lL][sS][eE]
key_end           [eE][nN][dD]
key_engineering   [eE][nN][gG][iI][nN][eE][eE][rR][iI][nN][gG]
key_error         [eE][rR][rR][oO][rR]
key_exit          [eE][xX][iI][tT]
key_expose        [eE][xX][pP][oO][sS][eE]
key_external      [eE][xX][tT][eE][rR][nN][aA][lL]
key_failure       [fF][aA][iI][lL][uU][rR][eE]
key_for           [fF][oO][rR]
key_forever       [fF][oO][rR][eE][vV][eE][rR]
key_form          [fF][oO][rR][mM]
key_fuzz          [fF][uU][zZ][zZ]
key_halt          [hH][aA][lL][tT]
key_if            [iI][fF]
key_interpret     [iI][nN][tT][eE][rR][pP][rR][eE][tT]
key_iterate       [iI][tT][eE][rR][aA][tT][eE]
key_leave         [lL][eE][aA][vV][eE]
key_linein        [lL][iI][nN][eE][iI][nN]
key_lostdigits    [lL][oO][sS][tT][dD][iI][gG][iI][tT][sS]
key_lower         [lL][oO][wW][eE][rR]
key_name          [nN][aA][mM][eE]
key_nop           [nN][oO][pP]
key_notready      [nN][oO][tT][rR][eE][aA][dD][yY]
key_novalue       [nN][oO][vV][aA][lL][uU][eE]
key_numeric       [nN][uU][mM][eE][rR][iI][cC]
key_off           [oO][fF][fF]
key_on            [oO][nN]
key_options       [oO][pP][tT][iI][oO][nN][sS]
key_otherwise     [oO][tT][hH][eE][rR][wW][iI][sS][eE]
key_parse         [pP][aA][rR][sS][eE]
key_procedure     [pP][rR][oO][cC][eE][dD][uU][rR][eE]
key_pull          [pP][uU][lL][lL]
key_push          [pP][uU][sS][hH]
key_queue         [qQ][uU][eE][uU][eE]
key_return        [rR][eE][tT][uU][rR][nN]
key_say           [sS][aA][yY]
key_scientific    [sS][cC][iI][eE][nN][tT][iI][fF][iI][cC]
key_select        [sS][eE][lL][eE][cC][tT]
key_signal        [sS][iI][gG][nN][aA][lL]
key_source        [sS][oO][uU][rR][cC][eE]
key_syntax        [sS][yY][nN][tT][aA][xX]
key_then          [tT][hH][eE][nN]
key_to            [tT][oO]
key_trace         [tT][rR][aA][cC][eE]
key_until         [uU][nN][tT][iI][lL]
key_upper         [uU][pP][pP][eE][rR]
key_value         [vV][aA][lL][uU][eE]
key_var           [vV][aA][rR]
key_version       [vV][eE][rR][sS][iI][oO][nN]
key_when          [wW][hH][eE][nN]
key_while         [wW][hH][iI][lL][eE]
key_with          [wW][iI][tT][hH]


%%

   {
     if ( delayed_symbol )
     {
        int retval;
        retval = delayed_symbol;
        delayed_symbol = 0;
        /*
         * Pass the meaning of inhibit_delayed_abuttal to the normal algorithm
         * for automatic abuttal detection.
         */
        expression_ended = !inhibit_delayed_abuttal;
        inhibit_delayed_abuttal = 0;
        return retval;
     }

     if (next_numform)
     {
        in_numform = 1 ;
        next_numform = 0 ;
     }
     else
        in_numform = 0 ;

     last_in_address = in_address ; /* just for the "Environment" */
    /* there can't be an intermediate SPACE between ADDRESS and the next word*/
     if ( in_address == in_address_keyword )
        in_address = in_address_main ;

     kill_this_space = kill_next_space ;
     kill_next_space = 0 ;


     if (itflag)
     {
        in_trace = 0 ;
        seek_with = no_seek_with ;
     }
     itflag = (in_trace) ;

     if (extnextline != -1)
     {
        parser_data.tstart = nextstart = extnextstart;
        parser_data.tline = nextline = extnextline;
        extnextline = -1;
     }
     else
     {
        parser_data.tstart = nextstart ;
        parser_data.tline = nextline ;
     }
     insert_abuttal = expression_ended ;
     expression_ended = 0 ;
   }

{comment}* {
   SET_NEXTSTART() ; }

<ifcont>{between}{EOL}{between} {
   /*
    * Fixes bug 579597, "[;\r?\n]" instead of "{EOL}" causes the bug.
    */
   char *ptr;
   if ((ptr = strchr(yytext, '\n')) != NULL)
   {
      nextstart = yyleng - (int) (ptr - (char *) yytext) ;
      nextline++ ;
      if (extnextline != -1)
      {
         extnextline++;
         extnextstart = nextstart; /* fixes bug 938204 */
      }
   }
   else
      SET_NEXTSTART() ;
   return STATSEP ; }

{between}{EOL}{between} {
   char *ptr;
   BEGIN comm ;
   if ( obs_with )
   {
      parser_data.tline = linenr - 1 ; /* set tline for exiterror */
      exiterror( ERR_INVALID_TEMPLATE, 1, yytext )  ;
   }
   obs_with = in_do = 0 ;
   in_signal = in_call = 0 ;
   in_address = not_in_address;
   in_parse = 0 ;
   if ( seek_with == seek_with_from_address )
      seek_with = no_seek_with ;
   if ((ptr = strchr(yytext, '\n')) != NULL)
   {
      nextstart = yyleng - (int) (ptr - (char *) yytext) ;
      nextline++ ;
      if (extnextline != -1)
      {
         extnextline++;
         extnextstart = nextstart; /* fixes bug 938204 */
      }
   }
   else
      SET_NEXTSTART() ;
   return STATSEP ; }

{key_dotline}/{notGeneralDot} {
   int  i;
   char work[16];

   /* support C-like __LINE__ value */
   /* determine current source line, and create a numeric literal */

   parser_data.tline = linenr - 1 ; /* set tline for exiterror */

   sprintf (work, "%d", parser_data.tline);

   /* copy back work, right-to-left */

   for (i = strlen (work) - 1; i >= 0; i--)
   {
      unput (work[i]);
   }
}

<comm>{key_address}{between} {
   BEGIN value1 ; /* Allow a following VALUE keyword */
   seek_with = seek_with_from_address ;
   in_address = in_address_keyword ;
   in_call = 1 ; /* Allow the next words to be given as in CALL. */
   SET_NEXTSTART() ;
   return ADDRESS ; }

<comm>{key_arg}{between} {
   BEGIN other ;
   in_parse = 1 ;
   SET_NEXTSTART() ;
   return ARG ; }

<comm>{key_call}{between} {
   BEGIN signal ;
   in_call = 1 ;
   SET_NEXTSTART() ;
   return CALL ; }

<comm>{key_do}{between} {
   BEGIN do1 ;
   assert( do_level >=0 ) ;
   do_level++ ;
   in_do = 1 ;
   SET_NEXTSTART() ;
   return DO ; }

<comm>{key_drop}{between} {
   BEGIN other ;
   in_parse = 1 ;
   SET_NEXTSTART() ;
   return DROP ; }

<comm>{key_else}{between} {
   BEGIN comm ;
   SET_NEXTSTART() ;
   return ELSE ; }

<comm>{key_exit}{between} {
   BEGIN other ;
   SET_NEXTSTART() ;
   return EXIT ; }

<comm>{key_if}{between} {
   BEGIN ifcont ;
   in_then = 1 ;
   parser_data.if_linenr = linenr - 1;
   SET_NEXTSTART() ;
   return IF ; }

<comm>{key_interpret}{between} {
   BEGIN other ;
   SET_NEXTSTART() ;
   return INTERPRET ; }

<comm>{key_iterate}{between} {
   BEGIN other ;
   SET_NEXTSTART() ;
   return ITERATE ; }

<comm>{key_leave}{between} {
   BEGIN other ;
   SET_NEXTSTART() ;
   return LEAVE ; }

<comm>{key_options}{between} {
   BEGIN other ;
   SET_NEXTSTART() ;
   return OPTIONS ; }

<comm>{key_nop}{between} {
   BEGIN other ;
   SET_NEXTSTART() ;
   return NOP ; }

<comm>{key_numeric}{between} {
   BEGIN numeric ;
   SET_NEXTSTART() ;
   return NUMERIC ; }

<comm>{key_parse}{between} {
   BEGIN parse ;
   in_parse = 1 ;
   SET_NEXTSTART() ;
   return PARSE ; }

<comm>{key_procedure}{between} {
   BEGIN procd ;
   SET_NEXTSTART() ;
   return PROCEDURE ; }

<comm>{key_pull}{between} {
   BEGIN other ;
   in_parse = 1 ;
   SET_NEXTSTART() ;
   return PULL ; }

<comm>{key_push}{between} {
   BEGIN other ;
   SET_NEXTSTART() ;
   return PUSH ; }

<comm>{key_queue}{between} {
   BEGIN other ;
   SET_NEXTSTART() ;
   return QUEUE ; }

<comm>{key_return}{between} {
   BEGIN other ;
   SET_NEXTSTART() ;
   return RETURN ; }

<comm>{key_say}{between} {
   BEGIN other ;
   SET_NEXTSTART() ;
   return SAY ; }

<comm>{key_select}{between} {
   BEGIN other ;
   assert( do_level >= 0 ) ;
   do_level++ ; parser_data.select_linenr = linenr - 1;
   SET_NEXTSTART() ;
   return SELECT ; }

<comm>{key_signal}{between} {
   BEGIN signal ;
   in_signal = 1 ;
   SET_NEXTSTART() ;
   return SIGNAL ; }

<comm>{key_trace}{between} {
   BEGIN value1 ;
   in_trace = 1 ;
   SET_NEXTSTART() ;
   return TRACE ; }

<comm>{key_upper}{between} {
   BEGIN other ;
   in_parse = 1 ;
   SET_NEXTSTART() ;
   return UPPER ; }

<comm>{key_when}{between} {
   BEGIN ifcont ;
   in_then = 1 ;
   parser_data.when_linenr = linenr - 1;
   SET_NEXTSTART() ;
   return WHEN ; }

<comm>{key_otherwise}{between} {
   BEGIN comm ;
   SET_NEXTSTART() ;
   return OTHERWISE ; }

<comm>{key_end}{between} {
   BEGIN other ;
   assert( do_level >= 0 ) ;
   if ( !do_level )
   {
      parser_data.tline = linenr - 1 ; /* set tline for exiterror */
      exiterror( ERR_UNMATCHED_END, 1 ) ;
   }
   do_level-- ;
   SET_NEXTSTART() ;
   return END ; }

{between} {
   if (in_parse)
   {
      SET_NEXTSTART() ;
      return yylex() ;
   }
   else
      REJECT ; }

\. {
   if (in_parse)
   {
      SET_NEXTSTART() ;
      return PLACEHOLDER ;
   }
   else
   {
      REJECT ;
   }
 }

<comm>{const_symbol}{between}={between} {
   /* .var = */
   parser_data.tline = linenr - 1 ; /* set tline for exiterror */
   if ( yytext[0] == '.' )
   {
      set_assignment( '=' );
      /*
       * FIXME, FGC: ANSI 6.2.3.1 forces an error 50.1 if yytext[0]=='.'
       *             ANSI 6.3.2.8 forces this error 31.3 which is wrong
       *             in its content.
       *             We use 6.2.3.1
       */
      if ( !KNOWN_RESERVED( retvalue, retlength ) )
      {
         yytext[retlength] = '\0';
         exiterror( ERR_RESERVED_SYMBOL, 1, yytext );
      }
      else
      {
         yytext[retlength] = '\0';
         exiterror( ERR_INVALID_START, 3, yytext );
      }
   }
   else
   {
      yytext[retlength] = '\0';
      exiterror( ERR_INVALID_START, 2, yytext );
   }
   /* known reserved variable */
   SET_NEXTSTART() ;
   return ASSIGNMENTVARIABLE ; }

<comm>{var_symbol}{between}={between} {
   /* var = */
   BEGIN other ;

   set_assignment( '=' );

   SET_NEXTSTART() ;
   return ASSIGNMENTVARIABLE ; }

<comm>{const_symbol}{between}\+{blank}*={between} {
   /* .var += */
   parser_data.tline = linenr - 1 ; /* set tline for exiterror */
   if ( yytext[0] == '.' )
   {
      set_assignment( '+' );
      /*
       * FIXME, FGC: ANSI 6.2.3.1 forces an error 50.1 if yytext[0]=='.'
       *             ANSI 6.3.2.8 forces this error 31.3 which is wrong
       *             in its content.
       *             We use 6.2.3.1
       */
      if ( !KNOWN_RESERVED( retvalue, retlength ) )
      {
         yytext[retlength] = '\0';
         exiterror( ERR_RESERVED_SYMBOL, 1, yytext );
      }
      else
      {
         yytext[retlength] = '\0';
         exiterror( ERR_INVALID_START, 3, yytext );
      }
   }
   else
   {
      yytext[retlength] = '\0';
      exiterror( ERR_INVALID_START, 2, yytext );
   }
   /* known reserved variable */
   SET_NEXTSTART() ;
   return PLUSASSIGNMENTVARIABLE ; }

<comm>{var_symbol}{between}\+{blank}*={between} {
   /* var += */
   BEGIN other ;

   set_assignment( '+' );

   SET_NEXTSTART() ;
   return PLUSASSIGNMENTVARIABLE ; }

<comm>{const_symbol}{between}\-{blank}*={between} {
   /* .var -= */
   parser_data.tline = linenr - 1 ; /* set tline for exiterror */
   if ( yytext[0] == '.' )
   {
      set_assignment( '-' );
      /*
       * FIXME, FGC: ANSI 6.2.3.1 forces an error 50.1 if yytext[0]=='.'
       *             ANSI 6.3.2.8 forces this error 31.3 which is wrong
       *             in its content.
       *             We use 6.2.3.1
       */
      if ( !KNOWN_RESERVED( retvalue, retlength ) )
      {
         yytext[retlength] = '\0';
         exiterror( ERR_RESERVED_SYMBOL, 1, yytext );
      }
      else
      {
         yytext[retlength] = '\0';
         exiterror( ERR_INVALID_START, 3, yytext );
      }
   }
   else
   {
      yytext[retlength] = '\0';
      exiterror( ERR_INVALID_START, 2, yytext );
   }
   /* known reserved variable */
   SET_NEXTSTART() ;
   return MINUSASSIGNMENTVARIABLE ; }

<comm>{var_symbol}{between}\-{blank}*={between} {
   /* var -= */
   BEGIN other ;

   set_assignment( '-' );

   SET_NEXTSTART() ;
   return MINUSASSIGNMENTVARIABLE ; }

<comm>{const_symbol}{between}\*{blank}*={between} {
   /* .var *= */
   parser_data.tline = linenr - 1 ; /* set tline for exiterror */
   if ( yytext[0] == '.' )
   {
      set_assignment( '*' );
      /*
       * FIXME, FGC: ANSI 6.2.3.1 forces an error 50.1 if yytext[0]=='.'
       *             ANSI 6.3.2.8 forces this error 31.3 which is wrong
       *             in its content.
       *             We use 6.2.3.1
       */
      if ( !KNOWN_RESERVED( retvalue, retlength ) )
      {
         yytext[retlength] = '\0';
         exiterror( ERR_RESERVED_SYMBOL, 1, yytext );
      }
      else
      {
         yytext[retlength] = '\0';
         exiterror( ERR_INVALID_START, 3, yytext );
      }
   }
   else
   {
      yytext[retlength] = '\0';
      exiterror( ERR_INVALID_START, 2, yytext );
   }
   /* known reserved variable */
   SET_NEXTSTART() ;
   return MULTASSIGNMENTVARIABLE ; }

<comm>{var_symbol}{between}\*{blank}*={between} {
   /* var *= */
   BEGIN other ;

   set_assignment( '*' );

   SET_NEXTSTART() ;
   return MULTASSIGNMENTVARIABLE ; }

<comm>{const_symbol}{between}\/{blank}*={between} {
   /* .var /= */
   parser_data.tline = linenr - 1 ; /* set tline for exiterror */
   if ( yytext[0] == '.' )
   {
      set_assignment( '/' );
      /*
       * FIXME, FGC: ANSI 6.2.3.1 forces an error 50.1 if yytext[0]=='.'
       *             ANSI 6.3.2.8 forces this error 31.3 which is wrong
       *             in its content.
       *             We use 6.2.3.1
       */
      if ( !KNOWN_RESERVED( retvalue, retlength ) )
      {
         yytext[retlength] = '\0';
         exiterror( ERR_RESERVED_SYMBOL, 1, yytext );
      }
      else
      {
         yytext[retlength] = '\0';
         exiterror( ERR_INVALID_START, 3, yytext );
      }
   }
   else
   {
      yytext[retlength] = '\0';
      exiterror( ERR_INVALID_START, 2, yytext );
   }
   /* known reserved variable */
   SET_NEXTSTART() ;
   return DIVASSIGNMENTVARIABLE ; }

<comm>{var_symbol}{between}\/{blank}*={between} {
   /* var /= */
   BEGIN other ;

   set_assignment( '/' );

   SET_NEXTSTART() ;
   return DIVASSIGNMENTVARIABLE ; }

<comm>{const_symbol}{between}\%{blank}*={between} {
   /* .var %= */
   parser_data.tline = linenr - 1 ; /* set tline for exiterror */
   if ( yytext[0] == '.' )
   {
      set_assignment( '%' );
      /*
       * FIXME, FGC: ANSI 6.2.3.1 forces an error 50.1 if yytext[0]=='.'
       *             ANSI 6.3.2.8 forces this error 31.3 which is wrong
       *             in its content.
       *             We use 6.2.3.1
       */
      if ( !KNOWN_RESERVED( retvalue, retlength ) )
      {
         yytext[retlength] = '\0';
         exiterror( ERR_RESERVED_SYMBOL, 1, yytext );
      }
      else
      {
         yytext[retlength] = '\0';
         exiterror( ERR_INVALID_START, 3, yytext );
      }
   }
   else
   {
      yytext[retlength] = '\0';
      exiterror( ERR_INVALID_START, 2, yytext );
   }
   /* known reserved variable */
   SET_NEXTSTART() ;
   return INTDIVASSIGNMENTVARIABLE ; }

<comm>{var_symbol}{between}\%{blank}*={between} {
   /* var %= */
   BEGIN other ;

   set_assignment( '%' );

   SET_NEXTSTART() ;
   return INTDIVASSIGNMENTVARIABLE ; }

<comm>{const_symbol}{between}\/{blank}*\/{blank}*={between} {
   /* .var //= */
   parser_data.tline = linenr - 1 ; /* set tline for exiterror */
   if ( yytext[0] == '.' )
   {
      set_assignment( '/' );
      /*
       * FIXME, FGC: ANSI 6.2.3.1 forces an error 50.1 if yytext[0]=='.'
       *             ANSI 6.3.2.8 forces this error 31.3 which is wrong
       *             in its content.
       *             We use 6.2.3.1
       */
      if ( !KNOWN_RESERVED( retvalue, retlength ) )
      {
         yytext[retlength] = '\0';
         exiterror( ERR_RESERVED_SYMBOL, 1, yytext );
      }
      else
      {
         yytext[retlength] = '\0';
         exiterror( ERR_INVALID_START, 3, yytext );
      }
   }
   else
   {
      yytext[retlength] = '\0';
      exiterror( ERR_INVALID_START, 2, yytext );
   }
   /* known reserved variable */
   SET_NEXTSTART() ;
   return MODULUSASSIGNMENTVARIABLE ; }

<comm>{var_symbol}{between}\/{blank}*\/{blank}*={between} {
   /* var //= */
   BEGIN other ;

   set_assignment( '/' );

   SET_NEXTSTART() ;
   return MODULUSASSIGNMENTVARIABLE ; }

<comm>{const_symbol}{between}\|{blank}*={between} {
   /* .var |= */
   parser_data.tline = linenr - 1 ; /* set tline for exiterror */
   if ( yytext[0] == '.' )
   {
      set_assignment( '|' );
      /*
       * FIXME, FGC: ANSI 6.2.3.1 forces an error 50.1 if yytext[0]=='.'
       *             ANSI 6.3.2.8 forces this error 31.3 which is wrong
       *             in its content.
       *             We use 6.2.3.1
       */
      if ( !KNOWN_RESERVED( retvalue, retlength ) )
      {
         yytext[retlength] = '\0';
         exiterror( ERR_RESERVED_SYMBOL, 1, yytext );
      }
      else
      {
         yytext[retlength] = '\0';
         exiterror( ERR_INVALID_START, 3, yytext );
      }
   }
   else
   {
      yytext[retlength] = '\0';
      exiterror( ERR_INVALID_START, 2, yytext );
   }
   /* known reserved variable */
   SET_NEXTSTART() ;
   return ORASSIGNMENTVARIABLE ; }

<comm>{var_symbol}{between}\|{blank}*={between} {
   /* var |= */
   BEGIN other ;

   set_assignment( '|' );

   SET_NEXTSTART() ;
   return ORASSIGNMENTVARIABLE ; }

<comm>{const_symbol}{between}\&{blank}*={between} {
   /* .var &= */
   parser_data.tline = linenr - 1 ; /* set tline for exiterror */
   if ( yytext[0] == '.' )
   {
      set_assignment( '&' );
      /*
       * FIXME, FGC: ANSI 6.2.3.1 forces an error 50.1 if yytext[0]=='.'
       *             ANSI 6.3.2.8 forces this error 31.3 which is wrong
       *             in its content.
       *             We use 6.2.3.1
       */
      if ( !KNOWN_RESERVED( retvalue, retlength ) )
      {
         yytext[retlength] = '\0';
         exiterror( ERR_RESERVED_SYMBOL, 1, yytext );
      }
      else
      {
         yytext[retlength] = '\0';
         exiterror( ERR_INVALID_START, 3, yytext );
      }
   }
   else
   {
      yytext[retlength] = '\0';
      exiterror( ERR_INVALID_START, 2, yytext );
   }
   /* known reserved variable */
   SET_NEXTSTART() ;
   return ANDASSIGNMENTVARIABLE ; }

<comm>{var_symbol}{between}\&{blank}*={between} {
   /* var &= */
   BEGIN other ;

   set_assignment( '&' );

   SET_NEXTSTART() ;
   return ANDASSIGNMENTVARIABLE ; }

<comm>{const_symbol}{between}\&{blank}*\&{blank}*={between} {
   /* .var &&= */
   parser_data.tline = linenr - 1 ; /* set tline for exiterror */
   if ( yytext[0] == '.' )
   {
      set_assignment( '&' );
      /*
       * FIXME, FGC: ANSI 6.2.3.1 forces an error 50.1 if yytext[0]=='.'
       *             ANSI 6.3.2.8 forces this error 31.3 which is wrong
       *             in its content.
       *             We use 6.2.3.1
       */
      if ( !KNOWN_RESERVED( retvalue, retlength ) )
      {
         yytext[retlength] = '\0';
         exiterror( ERR_RESERVED_SYMBOL, 1, yytext );
      }
      else
      {
         yytext[retlength] = '\0';
         exiterror( ERR_INVALID_START, 3, yytext );
      }
   }
   else
   {
      yytext[retlength] = '\0';
      exiterror( ERR_INVALID_START, 2, yytext );
   }
   /* known reserved variable */
   SET_NEXTSTART() ;
   return XORASSIGNMENTVARIABLE ; }

<comm>{var_symbol}{between}\&{blank}*\&{blank}*={between} {
   /* var &&= */
   BEGIN other ;

   set_assignment( '&' );

   SET_NEXTSTART() ;
   return XORASSIGNMENTVARIABLE ; }

<comm>{const_symbol}{between}\|{blank}*\|{blank}*={between} {
   /* .var ||= */
   parser_data.tline = linenr - 1 ; /* set tline for exiterror */
   if ( yytext[0] == '.' )
   {
      set_assignment( '|' );
      /*
       * FIXME, FGC: ANSI 6.2.3.1 forces an error 50.1 if yytext[0]=='.'
       *             ANSI 6.3.2.8 forces this error 31.3 which is wrong
       *             in its content.
       *             We use 6.2.3.1
       */
      if ( !KNOWN_RESERVED( retvalue, retlength ) )
      {
         yytext[retlength] = '\0';
         exiterror( ERR_RESERVED_SYMBOL, 1, yytext );
      }
      else
      {
         yytext[retlength] = '\0';
         exiterror( ERR_INVALID_START, 3, yytext );
      }
   }
   else
   {
      yytext[retlength] = '\0';
      exiterror( ERR_INVALID_START, 2, yytext );
   }
   /* known reserved variable */
   SET_NEXTSTART() ;
   return CONCATASSIGNMENTVARIABLE ; }

<comm>{var_symbol}{between}\|{blank}*\|{blank}*={between} {
   /* var ||= */
   BEGIN other ;

   set_assignment( '|' );

   SET_NEXTSTART() ;
   return CONCATASSIGNMENTVARIABLE ; }

<nmform,signal,value1>{between}{key_value}{between} {
   if (in_call)
   {
      if ( ( last_in_address == in_address_keyword )
        && ( in_address == in_address_main ) )
      {
         BEGIN other ; /* the next useful expression will set it to "other"
                        * in either case. The BEGIN-states aren't very
                        * handy in most cases; they are not flexible enough.
                        */
         in_address = in_address_value ;
         in_call = 0;
         SET_NEXTSTART() ;
         return VALUE ;
      }
      REJECT ;
   }
   BEGIN other ;
   if ((!in_trace)&&(!in_address)&&(!in_signal)&&(!in_call)&&(!in_numform))
      obs_with = 1 ;
   in_trace = in_signal = in_call = 0 ;
   SET_NEXTSTART() ;
   return VALUE ; }

<signal>{key_on}{between} {
   BEGIN sgtype ;
   SET_NEXTSTART() ;
   return ON ; }

<signal>{key_off}{between} {
   BEGIN sgtype ;
   SET_NEXTSTART() ;
   return OFF ; }

<signame>{key_name}{between} {
   BEGIN other ;
   SET_NEXTSTART() ;
   return NAME ; }

<sgtype>{key_error}{between} {
   BEGIN signame ;
   SET_NEXTSTART() ;
   return ERROR ; }

<sgtype>{key_halt}{between} {
   BEGIN signame ;
   SET_NEXTSTART() ;
   return HALT ; }

<sgtype>{key_novalue}{between} {
   BEGIN signame ;
   SET_NEXTSTART() ;
   return NOVALUE ; }

<sgtype>{key_notready}{between} {
   BEGIN signame ;
   SET_NEXTSTART() ;
   return NOTREADY ; }

<sgtype>{key_failure}{between} {
   BEGIN signame ;
   SET_NEXTSTART() ;
   return FAILURE ; }

<sgtype>{key_syntax}{between} {
   BEGIN signame ;
   SET_NEXTSTART() ;
   return SYNTAX ; }

<sgtype>{key_lostdigits}{between} {
   BEGIN signame ;
   SET_NEXTSTART() ;
   return LOSTDIGITS ; }

<value1>{between}{trace_char}+{between} {
   if (!in_trace) REJECT ;
   strcpy(retvalue,rmspc( yytext )) ;
   SET_NEXTSTART() ;
   return WHATEVER ; }

<procd>{key_expose}{between} {
   BEGIN other ;
   in_parse = 1 ;
   SET_NEXTSTART() ;
   return EXPOSE ; }

<parse>{key_upper}{between} {
   SET_NEXTSTART() ;
   return UPPER ; }

<parse>{key_lower}{between} {
   SET_NEXTSTART() ;
   return LOWER ; }

<parse>{key_caseless}{between} {
   SET_NEXTSTART() ;
   return CASELESS ; }

<parse>{key_arg}{between} {
   BEGIN other ;
   SET_NEXTSTART() ;
   return ARG ; }

<parse>{key_numeric}{between} {
   BEGIN other ;
   SET_NEXTSTART() ;
   return NUMERIC ; }

<parse>{key_pull}{between} {
   BEGIN other ;
   SET_NEXTSTART() ;
   return PULL ; }

<parse>{key_source}{between} {
   BEGIN other ;
   SET_NEXTSTART() ;
   return SOURCE ; }

<parse>{key_external}{between} {
   BEGIN other ;
   SET_NEXTSTART() ;
   return EXTERNAL ; }

<parse>{key_linein}{between} {
   BEGIN other ;
   SET_NEXTSTART() ;
   return LINEIN ; }

<parse>{key_version}{between} {
   BEGIN other ;
   SET_NEXTSTART() ;
   return VERSION ; }

<parse>{key_var}{between} {
   BEGIN other ;
   in_parse = 2; /* accept a variable and treat func(a) as func (a) */
   SET_NEXTSTART() ;
   return VAR ; }

<parse>{key_value}{between} {
   seek_with = seek_with_from_parse ;
   in_trace = 0 ;
   in_parse = 0 ;
   BEGIN with ; /* in fact this works as a "not comm" */
   SET_NEXTSTART() ;
   return VALUE ; }

<comm>{between}{key_then}{between} {
   in_then = 0 ;
   SET_NEXTSTART() ;
   return THEN ; }

<other,ifcont>{between}{key_then}{between} {
   if (in_then!=1) REJECT ;
   BEGIN comm ;
   in_then = 0 ;
   SET_NEXTSTART() ;
   return THEN ; }

{between}{key_with}{between} {
   /*
    * Fixes bug 952380
    */
   if ((in_do)||(!seek_with))
      REJECT ;
   BEGIN other ;
   if ( seek_with == seek_with_from_parse )
      in_parse = 1 ;
   seek_with = no_seek_with ;
   if (in_address) /* any address state */
      in_address = in_address_with ; /* WITH seen */
   SET_NEXTSTART() ;
   return WITH ; }


<numeric>{key_digits}{between} {
   BEGIN other ;
   SET_NEXTSTART() ;
   return DIGITS ; }

<numeric>{key_form}{between} {
   BEGIN nmform ;
   next_numform = 1 ;
   SET_NEXTSTART() ;
   return FORM ; }

<nmform>{key_scientific}{between} {
   SET_NEXTSTART() ;
   return SCIENTIFIC ; }

<nmform>{key_engineering}{between} {
   SET_NEXTSTART() ;
   return ENGINEERING ; }

<numeric>{key_fuzz}{between} {
   BEGIN other ;
   SET_NEXTSTART() ;
   return FUZZ ; }

<do1>{key_forever}{between} {
   BEGIN other ;
   assert(in_do) ;
   in_do = 2 ;
   SET_NEXTSTART() ;
   return FOREVER ; }

{key_to}{between} {
   if ( in_do == 2 )
   {
      BEGIN other ;
      SET_NEXTSTART() ;
      return TO ;
   }
   else if ( in_do == 1 )
   {
      parser_data.tline = linenr - 1 ; /* set tline for exiterror */
      exiterror( ERR_INVALID_DO_SYNTAX, 1, "TO" )  ;
   }
   REJECT ; }

{key_by}{between} {
   if ( in_do == 2 )
   {
      BEGIN other ;
      SET_NEXTSTART() ;
      return BY ;
   }
   else if ( in_do == 1 )
   {
      parser_data.tline = linenr - 1 ; /* set tline for exiterror */
      exiterror( ERR_INVALID_DO_SYNTAX, 1, "BY" ) ;
   }
   REJECT ; }

{key_for}{between} {
   if ( in_do == 2 )
   {
      BEGIN other ;
      SET_NEXTSTART() ;
      return FOR ;
   }
   else if ( in_do == 1 )
   {
      parser_data.tline = linenr - 1 ; /* set tline for exiterror */
      exiterror( ERR_INVALID_DO_SYNTAX, 1, "FOR" ) ;
   }
   REJECT ; }

{key_while}{between} {
   if (in_do)
   {
      if ( in_do == 3 )
      {
         parser_data.tline = linenr - 1 ; /* set tline for exiterror */
         exiterror( ERR_INVALID_DO_SYNTAX, 1, "WHILE" )  ;
      }
      in_do=3 ;
      BEGIN other ;
      SET_NEXTSTART() ;
      return WHILE ;
   }
   REJECT ; }

{key_until}{between} {
   if (in_do) {
      if ( in_do == 3 )
      {
         parser_data.tline = linenr - 1 ; /* set tline for exiterror */
         exiterror( ERR_INVALID_DO_SYNTAX, 1, "UNTIL" )  ;
      }

      in_do=3 ;
      BEGIN other ;
      SET_NEXTSTART() ;
      return UNTIL ; }
   REJECT ; }


<do1>{reserved_symbol}{between}/= {
   BEGIN other ;
   in_do = 2 ;
   set_assignment( '=' );
   if ( !KNOWN_RESERVED( retvalue, retlength ) )
      exiterror( ERR_RESERVED_SYMBOL, 1, yytext )  ;
   SET_NEXTSTART() ;
   return DOVARIABLE ; }

<do1>{var_symbol}{between}/= {
   BEGIN other ;
   in_do = 2 ;
   strcpy(retvalue,rmspc( yytext )) ;
   SET_NEXTSTART() ;
   return DOVARIABLE ; }

<comm>{symbol}{between}:{between} {
   unsigned i;
   BEGIN comm ;

   for( i = 0; ( ch = yytext[i] ) != '\0'; i++ )
   {
      /*
       * A blank or ':' can't occur in the normal text. They are terminators.
       */
      if ( ( ch == ':' )
        || ( ch == '\n' )
        || ( MY_ISBLANK( ch ) ) )
         break;
      retvalue[i] = (char) rx_toupper( ch );
   }
   retvalue[i] = '\0' ;
   SET_NEXTSTART() ;
   return LABEL ; }


<comm>{mintext_string}{between}:{between} {
   BEGIN comm ;

   compress_string( retvalue, yytext );

   SET_NEXTSTART() ;
   return LABEL ; }


{mintext_string}{comment}*\( {
   BEGIN other ;

   compress_string( retvalue, yytext );

   kill_next_space = 1 ;
   if ( insert_abuttal ) {
      inhibit_delayed_abuttal = 1 ;
      delayed_symbol = EXFUNCNAME ;
      SET_NEXTSTART() ;
      return CONCATENATE ; }

   expression_ended = 0 ;
   SET_NEXTSTART() ;
   return EXFUNCNAME ; }



('{hex_string}'|\"{hex_string}\")[xX]/{notGeneralDot} {
   /*
    * fixes bug 617225.
    */
   return process_hex_or_bin_string( yytext, yyleng, 16 ); }



('{bin_string}'|\"{bin_string}\")[bB]/{notGeneralDot} {
   return process_hex_or_bin_string( yytext, yyleng, 2 ); }

{text_string}[xXbB]/{notGeneralParen} {
   char c;
   int i,base,charcount,first,tuple;

   base = ( rx_toupper( yytext[yyleng - 1] ) == 'X' ) ? 16 : 2;
   parser_data.tline = linenr - 1 ; /* set tline for exiterror */
   /*
    * We are sure to have an invalid string since the above patterns won't
    * match. We can either have invalid characters or misplaced spaces.
    *
    * Blanks as the first characters are forbidden.
    */
   if ( MY_ISBLANK( yytext[1] ) )
      exiterror( ERR_INVALID_HEX_CONST, ( base == 16 ) ? 1 : 2, 1 );

   tuple = ( base == 16 ) ? 2 : 4;
   for ( i = 1, first = 1, charcount = 0; i < yyleng - 2; i++ )
   {
      c = yytext[i];
      if ( MY_ISBLANK( c ) )
      {
         /*
          * The first tuple may have less than tuple chars
          */
         if ( ( ( charcount % tuple ) == 0 ) || first )
         {
            first = 0;
            charcount = 0;
            continue;
         }
         exiterror( ERR_INVALID_HEX_CONST, ( base == 16 ) ? 1 : 2, i );
      }
      charcount++;
      if ( base == 2 )
      {
         if ( ( c == '0' ) || ( c == '1' ) )
            continue;
      }
      if ( base == 16 )
      {
         if ( rx_isxdigit( c ) )
            continue;
      }
      exiterror( ERR_INVALID_HEX_CONST, ( base == 16 ) ? 3 : 4, c );
   }
   /*
    * We didn't match something like "1 12 34 "X Assume this as the error.
    * Look back to the first blank in the last sequence.
    */
   for ( i = yyleng - 2; i > 1; i-- )
      if ( !MY_ISBLANK( yytext[i - 1] ) )
         break;
   exiterror( ERR_INVALID_HEX_CONST, ( base == 16 ) ? 1 : 2, i ); }

{text_string} {
   BEGIN other ;

   compress_string( retvalue, yytext );

   strcpy( yytext, retvalue ); /* proper error display */

   if (in_numform)
   {
      parser_data.tline = linenr - 1 ; /* set tline for exiterror */
      exiterror( ERR_INV_SUBKEYWORD, 11, "ENGINEERING SCIENTIFIC", retvalue ) ;
   }

   /* fixes 1109372 */
   if ( insert_abuttal && !in_parse && !in_signal && !in_call ) {
      delayed_symbol = STRING ;
      SET_NEXTSTART() ;
      return CONCATENATE ; }

    if (in_call)
    {
       in_call = 0 ;
       kill_next_space = 1 ;
    }
    else
       expression_ended = 1 ;

   SET_NEXTSTART() ;
   return STRING ; }


{digit}+ {
   if (!in_parse)
      REJECT ;
   strcpy(retvalue,yytext) ;
   SET_NEXTSTART() ;
   return OFFSET ; }

{digit}+/{notGeneralDot} {
   /*
    * This is the same as of the "{digit}+" rule above. flex is very stupid.
    * (Or is it a feature?)
    * The number below will take precedence instead of a plain "{digit}+",
    * even of the fact that the above rule may match the same length of
    * characters. flex seems to count the next "expect characters" for the
    * comparison which rule shall be used.
    */
   if (!in_parse)
      REJECT ;
   strcpy(retvalue,yytext) ;
   SET_NEXTSTART() ;
   return OFFSET ; }

{number}/{notGeneralDot} {
   /*
    * must split two rule parts because of the "/" rule-part.
    * This fixes bug 602283.
    */
   return process_number_or_const( yytext, yyleng );
}

{const_symbol} {
   return process_number_or_const( yytext, yyleng );
}

{var_symbol} {
   int i,j;
   /*
    * this might be a symbol in front of a function, but only if next
    * char in input stream is "(".
    */

   if (in_trace) REJECT ;

   memcpy( retvalue, yytext, yyleng + 1 ); /* include terminating '\0' */
   mem_upper( retvalue, yyleng );

   if (in_numform)
   {
      parser_data.tline = linenr - 1 ; /* set tline for exiterror */
      exiterror( ERR_INV_SUBKEYWORD, 11, "ENGINEERING SCIENTIFIC", retvalue ) ;
   }

   if ( ( last_in_address == in_address_keyword )
     && ( in_address == in_address_main ) )
   {
      kill_next_space = 1 ;
      in_call = 0 ;
      SET_NEXTSTART() ;
      return SIMSYMBOL ;
   }

   BEGIN other;

   if ( in_call )
   {
      /*
       * This has precedence over checking the parenthesis below.
       * Fixes bug 521502.
       */
      in_call = 0;
      kill_next_space = 1;
      SET_NEXTSTART();
      return SIMSYMBOL;
   }

   if ( in_parse == 2 )
   {
      /*
       * This has precedence over checking the parenthesis below.
       * Fixes bug 1109335.
       */
      in_parse = 1;
      kill_next_space = 1;
      SET_NEXTSTART();
      return SIMSYMBOL;
   }

   /* We must check if a '(' follows. Remember the number of eaten chars. */
      j = 1;
#ifdef __cplusplus
   for (; ( i = yyinput() ) == '`';)
#else
   for (; ( i = input() ) == '`';)
#endif
     j++ ;
   if (i != '(')
   {
      j-- ;
      unput(i) ;
   }
   /* input() has destroyed the yytext-terminator re-set it */
   yytext[yyleng] = '\0';
   SET_NEXTSTART() ;
   nextstart += j ;

   if ( i == '(' )
   {
      kill_next_space = 1 ;
      if ( insert_abuttal )
      {
         inhibit_delayed_abuttal = 1 ;
         delayed_symbol = INFUNCNAME ;
         return CONCATENATE ;
      }
      expression_ended = 0 ;
      return INFUNCNAME ;
   }

   if ( insert_abuttal && !in_parse ) {
      delayed_symbol = SIMSYMBOL ;
      return CONCATENATE ; }

   expression_ended = 1 ;

   if ( in_address == in_address_with )
      kill_next_space = 1 ;
   if (SymbolDetect) /* allow a fast breakout */
   {
      /* We define a tricky preprocessor directive. This will give us
       * maximum performance without the loss of control or errors produced
       * by typos.
       */
#define RET_IF(s)  if ((SymbolDetect & SD_##s) &&      \
                       (yyleng == sizeof(#s) - 1) &&   \
                       (strncmp(retvalue,              \
                                #s,                    \
                                sizeof(#s) - 1) == 0)) \
                      return(s)
      /* e.g. RET_IF(INPUT); is replaced by:
       *  if ((SymbolDetect & SD_INPUT) &&
       *      (yyleng == sizeof("INPUT") - 1) &&
       *      (strncmp(retvalue,
       *               "INPUT",
       *               sizeof("INPUT") - 1) == 0))
       *     return(s);
       */
      RET_IF(INPUT);
      RET_IF(OUTPUT);
      RET_IF(ERROR);
      RET_IF(NORMAL);
      RET_IF(APPEND);
      RET_IF(REPLACE);
      RET_IF(STREAM);
      RET_IF(STEM);
      RET_IF(LIFO);
      RET_IF(FIFO);
      RET_IF(NOEOL);
#undef RET_IF
   }
   return SIMSYMBOL ; }

{reserved_symbol} {
   int i,j;
   /*
    * this is mainly the content of var_symbol but we do a REJECT for all but
    * normal variable names.
    */

   if ( in_trace || in_numform || in_call )
      REJECT;

   if ( ( last_in_address == in_address_keyword )
     && ( in_address == in_address_main ) )
      REJECT;

   set_assignment( '=' );
   if ( !KNOWN_RESERVED( retvalue, retlength ) )
      REJECT;

   /* We must check if a '(' follows. Remember the number of eaten chars. */
#ifdef __cplusplus
   for ( j = 1; ( i = yyinput() ) == '`'; )
#else
   for ( j = 1; ( i = input() ) == '`'; )
#endif
     j++;
   if ( i != '(' )
   {
      j--;
      unput( i );
   }
   /* input() has destroyed the yytext-terminator re-set it */
   yytext[yyleng] = '\0';
   SET_NEXTSTART();
   nextstart += j;

   if ( i == '(' )
      REJECT; /* function name can't start with '.' */

   BEGIN other;
   if ( insert_abuttal && !in_parse )
   {
      delayed_symbol = SIMSYMBOL;
      return CONCATENATE;
   }

   expression_ended = 1;
   return SIMSYMBOL;
   }

{between}\) {
   expression_ended = 1 ;
   SET_NEXTSTART() ;
   return ')' ; }

\({between} {
   BEGIN other ;
   if ( insert_abuttal )
   {
      inhibit_delayed_abuttal = 1 ;
      delayed_symbol = '(' ;
      SET_NEXTSTART() ;
      return CONCATENATE ;
   }
   SET_NEXTSTART() ;
   return '(' ; }

{between}\,{between} {
   SET_NEXTSTART() ;
   return ',' ; }

{between}\-{between} {
   BEGIN other ;
   SET_NEXTSTART() ;
   return '-' ; }

{between}\+{between} {
   BEGIN other ;
   SET_NEXTSTART() ;
   return '+' ; }

{between}\/{between} {
   SET_NEXTSTART() ;
   return '/' ; }

{between}%{between} {
   SET_NEXTSTART() ;
   return '%' ; }

{between}\*{between} {
   SET_NEXTSTART() ;
   return '*' ; }

{between}\|{between} {
   SET_NEXTSTART() ;
   return '|' ; }

{between}&{between} {
   SET_NEXTSTART() ;
   return '&' ; }

{between}={between} {
   SET_NEXTSTART() ;
   return '=' ; }

{not}{between} {
   /* why don't I have a {between} in the beginning of this re? bug? */
   BEGIN other ;
   SET_NEXTSTART() ;
   return NOT ; }

{between}\>{blank}*\>{between} {
   SET_NEXTSTART() ;
   return GTGT ; }

{between}\<{blank}*\<{between} {
   SET_NEXTSTART() ;
   return LTLT ; }

{between}{not}{blank}*\>{blank}*\>{between} {
   SET_NEXTSTART() ;
   return NOTGTGT ; }

{between}{not}{blank}*\<{blank}*\<{between} {
   SET_NEXTSTART() ;
   return NOTLTLT ; }

{between}\>{blank}*\>{blank}*={between} {
   SET_NEXTSTART() ;
   return GTGTE ; }

{between}\<{blank}*\<{blank}*={between} {
   SET_NEXTSTART() ;
   return LTLTE ; }

{between}(\>|{not}{blank}*(\<{blank}*=|={blank}*\<)){between} {
   SET_NEXTSTART() ;
   return GT ; }

{between}({not}{blank}*\<|={blank}*\>|\>{blank}*=){between} {
   SET_NEXTSTART() ;
   return GTE ; }

{between}(\<|{not}{blank}*(\>{blank}*=|={blank}*\>)){between} {
   SET_NEXTSTART() ;
   return LT ; }

{between}({not}{blank}*\>|={blank}*\<|\<{blank}*=){between} {
   SET_NEXTSTART() ;
   return LTE ; }

{between}({not}{blank}*=|\<{blank}*\>|\>{blank}*\<){between} {
   SET_NEXTSTART() ;
   return DIFFERENT ; }

{between}={blank}*={between} {
   SET_NEXTSTART() ;
   return EQUALEQUAL ; }

{between}{not}{blank}*={blank}*={between} {
   SET_NEXTSTART() ;
   return NOTEQUALEQUAL ; }

{between}\/{blank}*\/{between} {
   SET_NEXTSTART() ;
   return MODULUS ; }

{between}&{blank}*&{between} {
   SET_NEXTSTART() ;
   return XOR ; }

{between}\|{blank}*\|{between} {
   SET_NEXTSTART() ;
   return CONCATENATE ; }

{between}\*{blank}*\*{between} {
   SET_NEXTSTART() ;
   return EXP ; }

{between}{blank}{between} {
   if ( in_address == in_address_value ) /* Always allow spaces in the VALUE */
   {                                   /* part of the ADDRESS stmt.        */
      SET_NEXTSTART() ;
      return SPACE ;
   }
   if (kill_this_space)
   {
      SET_NEXTSTART() ;
      return yylex() ;
   }
   SET_NEXTSTART() ;
   return (((in_parse)&&(!seek_with)) ? yylex() : SPACE) ; }

['"] {
   SET_NEXTSTART() ;
   parser_data.tline = linenr - 1 ; /* set tline for exiterror */
   exiterror( ERR_UNMATCHED_QUOTE, 0 )  ; }


: {
   SET_NEXTSTART() ;
   parser_data.tline = linenr - 1 ; /* set tline for exiterror */
   exiterror( ERR_SYMBOL_EXPECTED, 1, yytext ) ;}

. {
   SET_NEXTSTART() ;
   parser_data.tline = linenr - 1 ; /* set tline for exiterror */
   exiterror( ERR_INVALID_CHAR, 1, yytext[0], yytext[0] ); }


%%

#define NORMALSTAT  0
#define COMMENTSTAT 1
#define SINGLEQUOTE 2
#define DOUBLEQUOTE 3

/*
 * set_assignment sets an assignment variable in retvalue from yytext with
 * a trailing {between}[={between}] stripped.
 * retlength is set, too.
 */
static void set_assignment( char delim )
{
   char ch;

   for( retlength = 0; ( ch = yytext[retlength] ) != 0; retlength++ )
   {
      /*
       * A blank or delim can't occur in the normal text. They are terminators.
       */
      if ( ( ch == delim )
        || ( ch == '\n' )
        || ( MY_ISBLANK( ch ) ) )
         break;
      retvalue[retlength] = (char) rx_toupper( ch );
   }
   retvalue[retlength] = '\0' ;
}

/* rmspc uppercases all characters and removes blanks from a string.
 * Returns the input string.
 */
static YY_CHAR_TYPE *rmspc( YY_CHAR_TYPE *instr )
{
   YY_CHAR_TYPE *retval=instr ,
                *dest  =instr ,
                c ;

   while ((c = *instr++) != '\0')
   {
      c = (YY_CHAR_TYPE) rx_toupper(c);
      /* These characters are treated as blanks: */
      if ((c!='`') && (c!=' ') && (c!=',') && (c!='\t') && (c!='\n'))
         *dest++ = c ;
   }
   *dest = '\0' ;

   return( retval ) ;
}

/*
 * process_number_or_const does the processing of a decimal number or a const.
 * The detection was to complicated to put it into one routine.
 * We have to read some more characters to decide whether or not we have
 * an INFUNCNAME.
 * Returns a lexical code.
 */
static int process_number_or_const( const char *text, int len )
{
   int eaten,c,symbol;

   /*
    * Copy and uppercase the return value.
    */
   memcpy( retvalue, text, len + 1 ); /* include terminating '\0' */
   mem_upper( retvalue, len );

   if ( in_numform )
   {
      parser_data.tline = linenr - 1; /* set tline for exiterror */
      exiterror( ERR_INV_SUBKEYWORD, 11, "ENGINEERING SCIENTIFIC", retvalue );
   }
   nextstart += len;

   BEGIN other;

   /*
    * Plain number or const_symbol.
    */
   if ( KNOWN_RESERVED( retvalue, len ) )
      symbol = SIMSYMBOL;
   else
      symbol = CONSYMBOL;

   if ( in_call )
   {
      /*
       * This has precedence over checking the parenthesis below.
       * Fixes bug 521502.
       */
      in_call = 0;
      kill_next_space = 1;
      SET_NEXTSTART();
      return symbol;
   }

   /*
    * We must check if a '(' follows. Remember the number of eaten chars.
    */
   eaten = 1;
#ifdef __cplusplus
   while ( ( c = yyinput() ) == '`' )
#else
   while ( ( c = input() ) == '`' )
#endif
   {
     eaten++;
   }
   if ( c != '(' )
   {
      eaten--;
      unput( c );
   }
   /*
    * input() has destroyed the yytext-terminator re-set it
    */
   yytext[yyleng] = '\0';
   nextstart += eaten;
   if ( c == '(' )
   {
      kill_next_space = 1;
      if ( insert_abuttal )
      {
         inhibit_delayed_abuttal = 1;
         delayed_symbol = INFUNCNAME;
         return CONCATENATE;
      }
      expression_ended = 0;
      return INFUNCNAME;
   }

   if ( insert_abuttal && !in_parse )
   {
      delayed_symbol = symbol;
      return CONCATENATE;
   }

   expression_ended = 1;
   return symbol;
}

/*
 * compress_string removes quotes or double quotes from the src and changes
 * double delimiter to one. The delimiter should be either ' or ".
 */
static void compress_string( char *dest, const char *src )
{
   char c, delim = *src++;

   for (;;)
   {
      if ( ( c = *src++ ) == delim )
      {
         if (*src == delim )
         {
            *dest++ = *src++;
         }
         else
         {
            break;
         }
      }
      else
      {
         if ( c == '\n' )
         {
            parser_data.tline = linenr - 1 ; /* set tline for exiterror */
            exiterror( ERR_UNMATCHED_QUOTE, 0 ) ;
         }
         *dest++ = c;
      }
   }
   *dest = '\0';
}

/*
 * process_hex_or_bin_string convert the string in text which is given as
 * a hexstring or a binstring to a "normal" string. base must be either 2 or
 * 16.
 * We rely on having a well-formed string. This must be ensured. It is
 * something of the form "content"x or 'content'b or similar.
 * Returns a lexical code. retvalue and retlength will be filled.
 */
static int process_hex_or_bin_string( char *text, int len, int base )
{
   char c;
   int i,left;
   unsigned char *dest,sum;

   BEGIN other;
   SET_NEXTSTART();

   text++;
   len -= 3;

   /*
    * First, count the number of valid chars to determine the missed leading
    * zeros of the first final character.
    */
   for ( i = 0, left = 0 ; i < len; i++ )
   {
      c = text[i];
      if ( !MY_ISBLANK( c ) )
         left++;
   }

   /*
    * left shall be the count of input char left to process one dest char.
    * Accessing one character after the content's end is allowed.
    */
   left %= ( base == 2 ) ? 8 : 2;

   dest = (unsigned char *) retvalue;
   retlength = 0;

   sum = 0;
   c = *text;
   for (;;)
   {
      while ( len && MY_ISBLANK( c ) )
      {
         c = *++text;
         len--;
      }
      if ( len == 0 )
         break;

      if ( base == 2 )
      {
         if ( left == 0 )
            left = 8;
         sum <<= 1;
         sum |= (unsigned char) ( c - '0' );
      }
      else
      {
         if ( left == 0 )
            left = 2;
         sum <<= 4;
         sum |= HEXVAL( c );
      }
      c = *++text;
      len--;

      if ( --left == 0 )
      {
         dest[retlength++] = sum;
         sum = 0;
      }
   }
   dest[retlength] = '\0';

   /* We must check if a '(' follows. Remember the number of eaten chars. */
   left = 1;
#ifdef __cplusplus
   for (; ( i = yyinput() ) == '`';)
#else
   for (; ( i = input() ) == '`';)
#endif
     left++;
   if (i != '(')
   {
      left--;
      unput(i);
   }
   /* input() has destroyed the yytext-terminator re-set it */
   text[len] = '\0';
   SET_NEXTSTART();
   nextstart += left;

   if (i == '(')
   {
      kill_next_space = 1;
      if (insert_abuttal)
      {
         inhibit_delayed_abuttal = 1;
         delayed_symbol = INFUNCNAME;
         return CONCATENATE;
      }
      expression_ended = 0;
      return INFUNCNAME;
   }

   if ( insert_abuttal && !in_parse && !in_call )
   {
      /* fix bug 456 - the delayed symbol MUST be BINSTRING or HEXSTRING */
      delayed_symbol = ( base == 2 ) ? BINSTRING : HEXSTRING;
      return CONCATENATE;
   }

   if ( in_call )
   {
      in_call = 0;
      kill_next_space = 1;
   }
   else
      expression_ended = 1;

   return ( base == 2 ) ? BINSTRING : HEXSTRING;
}

/* get_next_line: Lower level input fetcher.
 * Reads exactly one line from the input stream (file or string).
 * All EOL characters are removed and the result is stored in
 * last_source_line. A check for line overflow occurred here.
 * A special check is done for CP/M ^Z (DOS and friends use this for
 * "backward" compatibility, too).
 * line is filled with valid values on success.
 * max must reflect size of line and should be at least BUFFERSIZE + 2;
 * Returns -1 (no input) or the number of valid chars in line.
 */
static int get_next_line( char *line, int max, FILE *stream )
{
   lineboxptr newline ;
   offsrcline *incore_newline;
   int pos = 0;
   int c = 0;

   if (inEOF) /* You can't use myunputc if EOF is reached! */
      return EOF ;

   while (pos <= max - 2)
   {
      /* get next char */
      if (bufptr>0)
         c = chbuffer[--bufptr] ;
      else if (ipretflag)
      {
         if (interptr>=interptrmax)
            c = EOF ;
         else
#ifdef ASCII_0_TERMINATES_STRING
            if ((c = *interptr++) == '\0')
               c = EOF ;
#else
            c = *interptr++ ;
#endif
      }
      else
      {
         c = getc(stream) ;

         if ( parser_data.TSD->HaltRaised )
            halt_raised( parser_data.TSD );
      }

      if ((c=='\r') || (c=='\n') || (c==EOF))
         break ;
      line[pos++] = (char) (unsigned char) c ;
   }

   /* first, check for overflow */
   if ((c!='\r') && (c!='\n') && (c!=EOF))
   {
      parser_data.tline = linenr ; /* set tline for exiterror */
      exiterror( ERR_TOO_LONG_LINE, 0 )  ;
   }

   /* We have either a (first) line terminator or EOF */
   if (c==EOF)
   {
      if ((pos==1) && (line[0]=='\x1A')) /* CP/M ^Z EOF? */
         pos-- ;
      if (pos == 0)
      {
         inEOF = 1 ;
         return EOF ;
      }
      chbuffer[bufptr++] = EOF; /* push back EOF for reuse */
   }
   else
   {
      /* Maybe we have CRLF or LFCR. Check for the pair character. */
      char pairChar = (c == '\r') ? '\n' : '\r';


      /* get one more char and consume it if it is the pair of the EOL */
      if (bufptr > 0)
      {
         if (chbuffer[bufptr - 1] == (int) pairChar)
            bufptr--;
      }
      else if (ipretflag)
      {
         if ((interptr < interptrmax) && (*interptr == pairChar))
            interptr++;
      }
      else
      {
         int next = getc(stream);
         if (next != pairChar)
         {
            /* ungetc may break some runtime stuff. Use the internal lookahead*/
            chbuffer[bufptr++] = next;
         }
         if ( parser_data.TSD->HaltRaised )
            halt_raised( parser_data.TSD );
      }
   }


   cch = 0 ; /* not needed ? */
   line[pos++] = '\n';

   if (parser_data.incore_source)
   {
      /*
       * We can use the incore string to describe a source line, but we
       * MUST incement linenr otherwise .LINE doesn't work in instore macros.
       * This will probably also allow errors to be reported for the correct line number.
       */
      linenr++;
      incore_newline = FreshLine() ;
      incore_newline->length = pos - 1 ;
      incore_newline->offset = last_interptr - parser_data.incore_source ;
      last_interptr = interptr;
      return pos ;
   }

   newline = (lineboxptr)Malloc(sizeof(linebox)) ;
   newline->line = Str_make_TSD( parser_data.TSD, pos - 1 ) ;
   newline->line->len = pos - 1 ;
   memcpy(newline->line->value, line, pos - 1 ) ;
   newline->prev = parser_data.last_source_line ;
   newline->next = NULL ;
   newline->lineno = linenr++ ;

   if (parser_data.first_source_line==NULL)
      parser_data.first_source_line = newline ;
   else
      parser_data.last_source_line->next = newline ;
   parser_data.last_source_line = newline ;

   return pos ;
}

/* fill_buffer: Higher level input fetcher.
 * (To allow the C-file to compile, all Rexx comments in this comment
 *  are written as "{*" "*}" instead of the normal, C-like manner.)
 * Reads lines from the input stream (yyin or string) with get_next_line.
 * Only one line is returned to allow the saving of the line number.
 * This routine replaces all comments by '`' signs. This allows
 * the detection of a "pseudo" blank: The fragment "say x{* *}y" uses two
 * variables, not one called "xy". The parsing of comments must be done
 * here to check for the actual numbers of open and closes ("{*" and "*}").
 * While doing this we must always check for strings since "'{*'" is not part
 * of a comment.
 * Here is a problem: Is this a nested valid comment: "{* '{*' *} *}"?
 * I think so although you cannot remove the outer comment signs without an
 * error. Everything within a comment is a comment (per def.). Counting
 * opens and closes of comment signs is an ugly trick to help the user.
 * He/she must know what he/she is doing if nesting comments!
 *
 * max_size gives the maximum size of buf. This is filled up with input.
 * We never return less than one character until EOF is reached. Thus, we
 * read more than one true input line if a comment spans over more than one
 * line.
 * A line will either be terminated by a single '\n' or by a blank. The
 * later one replaces a line continuation (',' [spaces] EOL).
 * Errors in this low
 *
 * Conclusion: We have to fight very hard to set the expected line number.
 *             * Comments spanning over lines set them on getting the
 *               "follow" lines.
 *             * Concatenated lines set
 */
static int fill_buffer( char *buf, int max_size )
{
   /* statics protected by regina_parser */
   static char line[BUFFERSIZE+2] ; /* special buffer to allow max_size */
   static int pos = 0, max = 0 ;    /* being smaller than BUFFERSIZE+1  */
   static int nesting = 0;          /* nesting level of comments        */
   int nesting_start_line = 0;      /* start line of comment for errortext() */
   char *dest, c;
   int i, squote, dquote;
   int line_comment;

   if (firstln == 0)
   {
      firstln = 1;
      contline = 0;
      nesting = 0;
      pos = 0;
      max = get_next_line( line, sizeof(line), yyin ) ;
      if (max < 0) /* empty input file */
         return 0 ;
      /* test for both #! - fixes bug 1230639 */
      if ( max > 1
      &&  line[0] == '#'
      &&  line[1] == '!' )
      {  /* Ignore first line beginning this way for unix compat */
         max = 5;
         memcpy( line, "/**/\n", 5 );
      }
   }
   else if (pos < max) /* Are there still characters to transmit? */
   {
      /* Buffer already checked for correctness */
      if (max_size > max - pos)
         max_size = max - pos;
      memcpy(buf, line + pos, max_size);
      pos += max_size;
      return(max_size);
   }
   else /* Need next line */
   {
      if (contline && !nesting)
      {
         extnextline = ++nextline ;
         extnextstart = 1 ;
         contline = 0;
      }
      pos = 0;
      max = get_next_line( line, sizeof(line), yyin ) ;
      if (max < 0) /* empty input file */
      {
         if (nesting)
         {
            parser_data.tline = linenr - 1 ; /* set tline for exiterror */
            exiterror( ERR_UNMATCHED_QUOTE, 1 ) ;
         }
         return 0 ;
      }
   }

   /* A new line is available, check first for an ending comment */
   dest = line; /* we change comments in line */
   if (nesting) /* This may lead to more line reading */
   {
      /*
       * The first time extnextline is non-zero, we have the comment
       * starting sequence line. This is saved for use if no matching
       * ending comment sequence is found, so that the error message
       * reflects the start of the comment.
       * Regina feature request: #508788
       */
      if ( extnextline < 0 )
         nesting_start_line = nextline+1;
      extnextline = ++nextline ;
      extnextstart = 1; /* See Reference (*) below */
repeated_nesting:
      while (pos < max)
      {
         c = line[pos];
         if (c == '*') /* start of comment end? */
         {
            if (line[pos+1] == '/')
            {  /* pos+1 always exists, at least '\n' or '\0' */
               if (--nesting == 0)
               {
                  pos += 2;
                  *dest++ = '`';
                  *dest++ = '`';
                  break;
               }
               *dest++ = '`';
               pos++;
            }
         }
         else if (c == '/') /* start of new begin? */
         {
            if (line[pos+1] == '*')
            {
               nesting++;
               *dest++ = '`';
               pos++;
            }
         }
         *dest++ = '`';
         pos++;
      }
      if (pos >= max)
      {
         pos = 0;
         max = get_next_line( line, sizeof(line), yyin ) ;
         if (max < 0) /* empty input file */
         {
            if ( nesting_start_line )
               parser_data.tline = nesting_start_line ; /* set tline for exiterror */
            else
               parser_data.tline = linenr - 1 ; /* set tline for exiterror */
            exiterror( ERR_UNMATCHED_QUOTE, 1 ) ;
            return 0 ;
         }
         /* This is a comment continuation. If the lexer will return
          * something it already has a valid tline/tstart pair.
          * The lexer will return the current token and on the NEXT
          * call it expects a valid nextline/nextstart pair.
          */
         extnextline = ++nextline; extnextstart = 1;
         dest = line; /* we change comments in line */
         goto repeated_nesting;
      }
      extnextstart = pos + 1;
      if (contline)
      { /* Exception! Have a look at: "x='y',{*\n\n*}\n'z'". This should
         * result in "x = 'y' 'z'".
         * We must parse until EOL and check for whitespaces and comments...
         */
         while (pos < max)
         {
            c = line[pos];
            if (!rx_isspace(c))
            {
               if (c == '/')
               {
                  if (line[pos+1] == '*')
                  {
                     pos += 2;
                     nesting++;
                     goto repeated_nesting;
                  }
               }
               parser_data.tline = linenr - 1 ; /* set tline for exiterror */
               exiterror( ERR_YACC_SYNTAX, 1, parser_data.tline ) ; /* standard error */
            }
            pos++;
         }
         /* All done, it was a continuation line. */
         /* contline will be resetted by: */
         return fill_buffer( buf, max_size ) ;
      }
   }
   /* We have something to play with. Run through the input and check for
    * strings including comments.
    */
   squote = dquote = 0;
   line_comment = 0;
   while (pos < max)
   {
      /* We use selective loops to reduce comparisons */
      if (nesting)
         do
         {
            c = line[pos];
            if (c == '*') /* start of comment end? */
            {
               if (line[pos+1] == '/')
               {  /* pos+1 always exists, at least '\n' or '\0' */
                  if (--nesting == 0)
                  {
                     pos += 2;
                     *dest++ = '`';
                     *dest++ = '`';
                     break;
                  }
                  pos++;
               }
            }
            else if (c == '/') /* start of new begin? */
            {
               if (line[pos+1] == '*')
               {
                  nesting++;
                  pos++;
                  *dest++ = '`';
               }
            }
            pos++;
            *dest++ = '`';
         } while (pos < max);
      else if (squote)
         {
            while ((c = line[pos]) != '\'')
            {
               *dest++ = c;
               if (++pos >= max)
               {
                  parser_data.tline = linenr - 1 ; /* set tline for exiterror */
                  exiterror( ERR_UNMATCHED_QUOTE, 2 ) ;
               }
            }
            *dest++ = '\'';
            pos++;
            squote = 0;
         }
      else if (dquote)
         {
            while ((c = line[pos]) != '\"')
            {
               *dest++ = c;
               if (++pos >= max)
               {
                  parser_data.tline = linenr - 1 ; /* set tline for exiterror */
                  exiterror( ERR_UNMATCHED_QUOTE, 3 ) ;
               }
            }
            *dest++ = '\"';
            pos++;
            dquote = 0;
         }
      else if (line_comment)
         {
            while ((c = line[pos]) >= ' ')    /* not at end of line yet */
            {
               *dest++ = '`';
               if (++pos >= max)
               {
                  parser_data.tline = linenr - 1 ; /* set tline for exiterror */
                  exiterror( ERR_UNMATCHED_QUOTE, 3 ) ;
               }
            }
            *dest++ = c;    /* line terminator */
            pos++;
            line_comment = 0;
         }
      else
         while (pos < max)
            switch (c = line[pos])
            {
               case '\'':
                  *dest++ = c ;
                  squote = 1 ;
                  pos++ ;
                  goto outer_loop;

               case '\"':
                  *dest++ = c ;
                  dquote = 1 ;
                  pos++ ;
                  goto outer_loop;

               case '/':
                  if (line[pos + 1] == '*')
                  {
                     *dest++ = '`' ;
                     *dest++ = '`' ;
                     pos += 2 ;
                     nesting++ ;
                     goto outer_loop;
                  }
                  else
                  {
                     *dest++ = c;
                     pos++ ;
                  }
                  break ;

               case '-':    /* line "--" comments */
                  if ( line[pos + 1] == '-'
                  &&   get_options_flag( parser_data.TSD->currlevel, EXT_SINGLE_LINE_COMMENTS )
                  &&   !get_options_flag( parser_data.TSD->currlevel, EXT_STRICT_ANSI ) )
                  {
                     *dest++ = '`' ;
                     *dest++ = '`' ;
                     pos += 2 ;
                     line_comment = 1 ;
                     goto outer_loop;
                  }
                  else
                  {
                     *dest++ = c;
                     pos++ ;
                  }
                  break ;

               case '`':
                  parser_data.tline = linenr - 1 ; /* set tline for exiterror */
                  exiterror( ERR_INVALID_CHAR, 1, c, c ) ;

               default:
                  *dest++ = c;
                  pos++ ;
            }
outer_loop:
      ;
   }

   max = (int) (dest - line);

   /* Now we can replace a ',' [spaces|comments] '\n' with the line
    * continuation, but check for nesting first
    */
   if (nesting)
   { /* Don't leave ANY spaces at EOL. That would confuse the lexer. */
      i = max - 1;
      while ((i >= 0) && rx_isspace(line[i]))
         i--;
      max = i + 1;
      /* Of course, there is one exception: line continuation */
      while ((i >= 0) && (line[i] == '`'))
         i-- ;
      if ((i >= 0) && (line[i] == ','))
      {
         contline = 1;
         line[i] = ' ';
         max = i + 1;
      }
      /* (Reference (*) )
       * At this point the lexer can't determine the nextline since we eat up
       * the \n. This leads to an incorrect count. But either the '`'-signs
       * are ignored or they are follows of a "token", a valid word.
       * Look at "say x;say y ``". This will cause the lexer to
       * return at least 4 tokens (SAY "x" ";" SAY) before "y" will be
       * returned. We can only set nextline/nextstart at "y".
       * Result: We set this pair at the start of the next call to
       * fill_buffer such that the next call to yylex will set the correct
       * values.
       */
   }
   else
   {
      i = max - 1; /* on last valid char */
      while (i >= 0)
      {
         if (!MY_ISBLANK(line[i]) && (line[i] != '\n'))
            break;
         i--;
      }
      /* i now -1 or on last nonblank */
      if ((i >= 0) && (line[i] == ','))
      {  /* FIXME: What shall be do with "," followed by EOF? */
         max = i + 1;
         line[i] = ' ';
         contline = 1;
      }
   }

   if (max_size > max)
      max_size = max;
   memcpy(buf, line, max_size);
   pos = max_size;
   return(max_size);
}


/* yywrap MAY be called by the lexer is EOF encounters, see (f)lex docu */
int yywrap( void )
{
   assert( do_level>= 0 ) ;
   if (do_level>0)
   {
      parser_data.tline = linenr - 1 ; /* set tline for exiterror */
      exiterror( ERR_INCOMPLETE_STRUCT, 0 )  ;
   }
   return 1 ;
}

/******************************************************************************
 ******************************************************************************
 * global interface ***********************************************************
 ******************************************************************************
 *****************************************************************************/

/* initalize all local and global values */
static void init_it_all( tsd_t *TSD )
{
#if defined(FLEX_SCANNER) && defined(FLEX_DEBUG)
   if (__reginadebug)
      yy_flex_debug = 1;
     else
      yy_flex_debug = 0;
#endif
   inEOF = 0 ;
   in_numform = 0 ;
   next_numform = 0 ;
   expression_ended = 0 ;
   insert_abuttal = 0 ;
   obs_with = 0 ;
   in_do = 0 ;
   in_then = 0 ;
   inhibit_delayed_abuttal = 0 ;
   firstln = 0 ;
   in_parse = 0 ;
   in_trace = 0 ;
   itflag = 0 ;
   in_signal = 0 ;
   in_call = 0 ;
   in_address = not_in_address ;
   seek_with = no_seek_with ;
   kill_this_space = 0 ;
   ipretflag = 0 ;
   do_level = 0 ;
   singlequote = 0 ;
   doblequote = 0 ;
   cch = 0 ;
   bufptr = 0 ;
   cchmax = 0 ;
   ch = '\0',
   delayed_symbol = 0,
   contline = 0;
   extnextstart = 0;
   interptr = NULL ;
   interptrmax = NULL ;
                          /* non-zero values */
   linenr = 1 ;
   nextline = 1;
   nextstart = 1;
   kill_next_space = 1 ;
   extnextline = -1 ;
   SymbolDetect = 0;

   memset(&parser_data, 0, sizeof(internal_parser_type));
   parser_data.TSD = TSD;
}

/* fetch may only be called by fetch_protected. The parser and lexer are
 * already protected by regina_parser by fetch_protected.
 * This function prepares the lexer and parser and call them. The
 * result and all generated values are stored in result. The parser
 * tree isn't executed here.
 * Exactly fptr xor str must be non-null.
 */
static void fetch(tsd_t *TSD, FILE *fptr, const streng *str,
                  internal_parser_type *result)
{
   init_it_all( TSD ) ;

#ifdef FLEX_SCANNER
   yy_init = 1 ;
   yy_delete_buffer(YY_CURRENT_BUFFER) ;
   yyrestart(fptr) ;
#else
   yysptr = yysbuf ;
   yyin = fptr ;
#endif

   if (str != NULL)
   {
      ipretflag = 1 ;
      cchmax = str->len ;
      interptr = str->value ;
      last_interptr = interptr ;
      interptrmax = interptr + cchmax ;
      parser_data.incore_source = str->value; /* fixes bug 972796 */
      result->incore_source = str->value;
   }

   BEGIN comm ;
   NewProg();
   parser_data.result = __reginaparse();

#ifdef FLEX_SCANNER
   yy_delete_buffer(YY_CURRENT_BUFFER) ;
#else
   yysptr = yysbuf ;
#endif
   yyin = NULL ;

   *result = parser_data;
   /* Some functions assume null values if parsing isn't running: */
   memset(&parser_data, 0, sizeof(internal_parser_type));
}

/* This function serializes the parser/lexer requests of the process and
 * call fetch which will make the work. Look there.
 */
static void fetch_protected( tsd_t * volatile TSD, FILE *fptr,
                             const streng *str, internal_parser_type *result )
{
   volatile int panicked = 0;
   tsd_t * volatile saved_TSD;

   THREAD_PROTECT( regina_parser )
   TSD->in_protected = 1;
   memset(&parser_data, 0, sizeof(internal_parser_type));

   saved_TSD = TSD; /* vars used until here */
   if ( setjmp( TSD->protect_return ) )
   {
      TSD = saved_TSD; /* prevents bugs like  592393 */
      panicked = 1;
   }
   else
      fetch( TSD, fptr, str, result );

   TSD->in_protected = 0;
   THREAD_UNPROTECT( regina_parser )

   if ( !panicked )
      return;

   /*
    * We got a fatal condition while fetching the input.
    */
   memset(result, 0, sizeof(internal_parser_type));

   /*
    * FIXME: Currently no time to investigate it, but we have to do
    *        a cleanup of the node and source lines of parser_data
    *        here. Test this with
    *        interpret '"a='
    *        and with
    *        interpret 'nop;"a='
    */
   if ( TSD->delayed_error_type == PROTECTED_DelayedInterpreterExit )
      jump_interpreter_exit( TSD, TSD->expected_exit_error );
   if ( TSD->delayed_error_type == PROTECTED_DelayedRexxSignal )
      jump_rexx_signal( TSD );
   jump_script_exit( TSD, TSD->systeminfo->result );
}

/* fetch_file reads in a REXX file from disk (or a pipe). It returns without
 * executing the program. The parsed tree with all needed values including
 * the result of the parsing is copied to result.
 * fptr remains open after this call.
 * type is either PARSE_ONLY or PARSE_AND_TIN. In the later case a tinned variant of the
 * parsing tree is created, too.
 */
void fetch_file(tsd_t *TSD, FILE *fptr, internal_parser_type *result)
{
   fetch_protected(TSD, fptr, NULL, result);
}

/* fetch_string reads in a REXX macro from a streng. It returns without
 * executing the program. The parsed tree with all needed values including
 * the result of the parsing is copied to result.
 * type is either PARSE_ONLY or PARSE_AND_TIN. In the later case a tinned variant of the
 * parsing tree is created, too.
 * The function is typically called by an "INTERPRET" instruction.
 */
void fetch_string(tsd_t *TSD, const streng *str, internal_parser_type *result)
{
   fetch_protected(TSD, NULL, str, result);
}
