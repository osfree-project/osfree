/***************************************************************
  
   bwb_tbl.c            OPTION VERSION table
                        for Bywater BASIC Interpreter
  
                        Copyright (c) 1993, Ted A. Campbell
                        Bywater Software
  
                        email: tcamp@delphi.com
  
        Copyright and Permissions Information:
  
        All U.S. and international rights are claimed by the author,
        Ted A. Campbell.
  
   This software is released under the terms of the GNU General
   Public License (GPL), which is distributed with this software
   in the file "COPYING".  The GPL specifies the terms under
   which users may copy and use the software in this distribution.
  
   A separate license is available for commercial distribution,
   for information on which you should contact the author.
  
***************************************************************/

/*---------------------------------------------------------------*/
/* NOTE: Modifications marked "JBV" were made by Jon B. Volkoff, */
/* 11/1995 (eidetics@cerf.net).                                  */
/*                                                               */
/* Those additionally marked with "DD" were at the suggestion of */
/* Dale DePriest (daled@cadence.com).                            */
/*                                                               */
/* Version 3.00 by Howard Wulf, AF5NE                            */
/*                                                               */
/* Version 3.10 by Howard Wulf, AF5NE                            */
/*                                                               */
/* Version 3.20 by Howard Wulf, AF5NE                            */
/*                                                               */
/*---------------------------------------------------------------*/



#include "bwbasic.h"
VersionType bwb_vertable[ /* NUM_VERSIONS */ ] =
{
  {
   "BYWATER",                        /* Name */
   B15,                                /* OptionVersionBitmask */
   "B15",                        /* ID */
   "Bywater BASIC 3",                /* Description */
   "Bywater BASIC Interpreter, version 3.20",        /* Reference Title */
   "by Ted A. Campbell, Jon B. Volkoff, Paul Edwards, et al.",        /* Reference Author */
   "(c) 2014-2017, Howard Wulf, AF5NE",        /* Reference Copyright */
   "http://wwww.sourceforge.net/bwbasic/",        /* Reference URL1 */
   "bwbasic-3.20.zip",                /* Reference URL2 */
   OPTION_LABELS_ON | OPTION_BUGS_ON | OPTION_TRACE_ON,        /* OptionFlags */
   128,                                /* OptionReclenInteger */
   0,                                /* OptionBaseInteger */
   "%m/%d/%Y",                        /* OptionDateFormat */
   "%H:%M:%S",                        /* OptionTimeFormat */
   '$',                                /* OptionStringChar */
   '#',                                /* OptionDoubleChar */
   '!',                                /* OptionSingleChar */
   '@',                                /* OptionCurrencyChar */
   '&',                                /* OptionLongChar */
   '%',                                /* OptionIntegerChar */
   '~',                                /* OptionByteChar */
   '"',                                /* OptionQuoteChar */
   '\'',                        /* OptionCommentChar */
   ':',                                /* OptionStatementChar */
   '?',                                /* OptionPrintChar */
   '\0',                        /* OptionInputChar */
   '\0',                        /* OptionImageChar */
   '(',                                /* OptionLparenChar */
   ')',                                /* OptionRparenChar */
   '#',                                /* OptionFilenumChar */
   '@',                                /* OptionAtChar */
   '#',                                /* OptionUsingDigit */
   ',',                                /* OptionUsingComma */
   '.',                                /* OptionUsingPeriod */
   '+',                                /* OptionUsingPlus */
   '-',                                /* OptionUsingMinus */
   '^',                                /* OptionUsingExrad */
   '$',                                /* OptionUsingDollar */
   '*',                                /* OptionUsingFiller */
   '_',                                /* OptionUsingLiteral */
   '!',                                /* OptionUsingFirst */
   '&',                                /* OptionUsingAll */
   '\\',                        /* OptionUsingLength */
   }
  ,
  {
   "BYWATER-2",                        /* Name */
   B93,                                /* OptionVersionBitmask */
   "B93",                        /* ID */
   "Bywater BASIC 2",                /* Description */
   "Bywater BASIC Interpreter, version 2.61",        /* Reference Title */
   "by Ted A. Campbell, Jon B. Volkoff, Paul Edwards, et al.",        /* Reference Author */
   "Copyright (c) 1993-2014, Ted A. Campbell",        /* Reference Copyright */
   "http://wwww.sourceforge.net/bwbasic/",        /* Reference URL1 */
   "bwbasic-2.61.zip",                /* Reference URL2 */
   OPTION_LABELS_ON | OPTION_BUGS_ON | OPTION_ERROR_GOSUB,        /* OptionFlags */
   128,                                /* OptionReclenInteger */
   0,                                /* OptionBaseInteger */
   "%m/%d/%Y",                        /* OptionDateFormat */
   "%H:%M:%S",                        /* OptionTimeFormat */
   '$',                                /* OptionStringChar */
   '#',                                /* OptionDoubleChar */
   '!',                                /* OptionSingleChar */
   '\0',                        /* OptionCurrencyChar */
   '&',                                /* OptionLongChar */
   '%',                                /* OptionIntegerChar */
   '\0',                        /* OptionByteChar */
   '"',                                /* OptionQuoteChar */
   '\'',                        /* OptionCommentChar */
   ':',                                /* OptionStatementChar */
   '?',                                /* OptionPrintChar */
   '\0',                        /* OptionInputChar */
   '\0',                        /* OptionImageChar */
   '(',                                /* OptionLparenChar */
   ')',                                /* OptionRparenChar */
   '#',                                /* OptionFilenumChar */
   '\0',                        /* OptionAtChar */
   '#',                                /* OptionUsingDigit */
   ',',                                /* OptionUsingComma */
   '.',                                /* OptionUsingPeriod */
   '+',                                /* OptionUsingPlus */
   '-',                                /* OptionUsingMinus */
   '^',                                /* OptionUsingExrad */
   '$',                                /* OptionUsingDollar */
   '*',                                /* OptionUsingFiller */
   '_',                                /* OptionUsingLiteral */
   '!',                                /* OptionUsingFirst */
   '&',                                /* OptionUsingAll */
   '\\',                        /* OptionUsingLength */
   }
  ,
  {
   "CALL/360",                        /* Name */
   S70,                                /* OptionVersionBitmask */
   "S70",                        /* ID */
   "SBC CALL/360 Mainframe BASIC",        /* Description */
   "CALL/360: BASIC Reference Handbook",        /* Reference Title */
   "by International Business Machines Corporation",        /* Reference Author */
   "(c) 1970, The Service Bureau Corporation",        /* Reference Copyright */
   "http://bitsavers.trailing-edge.com/pdf/ibm/360/os/",        /* Reference URL1 */
   "call_360/CALL_360_BASIC_Reference_Handbook_1970.pdf",        /* Reference URL2 */
   FALSE,                        /* OptionFlags */
   0,                                /* OptionReclenInteger */
   1,                                /* OptionBaseInteger */
   "%Y/%m/%d",                        /* OptionDateFormat */
   "%H:%M:%S",                        /* OptionTimeFormat */
   '$',                                /* OptionStringChar */
   '\0',                        /* OptionDoubleChar */
   '\0',                        /* OptionSingleChar */
   '\0',                        /* OptionCurrencyChar */
   '\0',                        /* OptionLongChar */
   '\0',                        /* OptionIntegerChar */
   '\0',                        /* OptionByteChar */
   '"',                                /* OptionQuoteChar */
   '\0',                        /* OptionCommentChar */
   '\0',                        /* OptionStatementChar */
   '\0',                        /* OptionPrintChar */
   '\0',                        /* OptionInputChar */
   ':',                                /* OptionImageChar */
   '(',                                /* OptionLparenChar */
   ')',                                /* OptionRparenChar */
   '\0',                        /* OptionFilenumChar */
   '\0',                        /* OptionAtChar */
   '#',                                /* OptionUsingDigit */
   '\0',                        /* OptionUsingComma */
   '.',                                /* OptionUsingPeriod */
   '+',                                /* OptionUsingPlus */
   '-',                                /* OptionUsingMinus */
   '!',                                /* OptionUsingExrad */
   '\0',                        /* OptionUsingDollar */
   '\0',                        /* OptionUsingFiller */
   '\0',                        /* OptionUsingLiteral */
   '\0',                        /* OptionUsingFirst */
   '\0',                        /* OptionUsingAll */
   '\0',                        /* OptionUsingLength */
   }
  ,
  {
   "CBASIC-II",                        /* Name */
   C77,                                /* OptionVersionBitmask */
   "C77",                        /* ID */
   "CBASIC-II for CP/M",        /* Description */
   "CBASIC-II",                        /* Reference Title */
   "by Compiler Systems",        /* Reference Author */
   "(c) 1977, Compiler Systems",        /* Reference Copyright */
   "http://bitsavers.trailing-edge.com/pdf/digitalResearch/cb80/",        /* Reference URL1 */
   "CBASIC_Version_2_Jan81.pdf",        /* Reference URL2 */
   OPTION_BUGS_ON,                /* OptionFlags */
   0,                                /* OptionReclenInteger */
   0,                                /* OptionBaseInteger */
   "%Y/%m/%d",                        /* OptionDateFormat */
   "%H:%M:%S",                        /* OptionTimeFormat */
   '$',                                /* OptionStringChar */
   '#',                                /* OptionDoubleChar */
   '!',                                /* OptionSingleChar */
   '\0',                                /* OptionCurrencyChar */
   '&',                                /* OptionLongChar */
   '%',                                /* OptionIntegerChar */
   '\0',                                /* OptionByteChar */
   '"',                                /* OptionQuoteChar */
   '\'',                        /* OptionCommentChar */
   ':',                                /* OptionStatementChar */
   '?',                                /* OptionPrintChar */
   '\0',                        /* OptionInputChar */
   '\0',                        /* OptionImageChar */
   '(',                                /* OptionLparenChar */
   ')',                                /* OptionRparenChar */
   '#',                                /* OptionFilenumChar */
   '\0',                        /* OptionAtChar */
   '#',                                /* OptionUsingDigit */
   ',',                                /* OptionUsingComma */
   '.',                                /* OptionUsingPeriod */
   '+',                                /* OptionUsingPlus */
   '-',                                /* OptionUsingMinus */
   '^',                                /* OptionUsingExrad */
   '$',                                /* OptionUsingDollar */
   '*',                                /* OptionUsingFiller */
   '\\',                        /* OptionUsingLiteral */
   '!',                                /* OptionUsingFirst */
   '&',                                /* OptionUsingAll */
   '/',                                /* OptionUsingLength */
   }
  ,
  {
   "DARTMOUTH",                        /* Name */
   D64,                                /* OptionVersionBitmask */
   "D64",                        /* ID */
   "Dartmouth DTSS BASIC",        /* Description */
   "BASIC",                        /* Reference Title */
   "by Computation Center, Dartmouth College",        /* Reference Author */
   "(c) 1964, Trustees of Dartmouth College",        /* Reference Copyright */
   "http://www.bitsavers.org/pdf/dartmouth/",        /* Reference URL1 */
   "BASIC_Oct64.pdf",                /* Reference URL2 */
   FALSE,                        /* OptionFlags */
   128,                                /* OptionReclenInteger */
   0,                                /* OptionBaseInteger */
   "%m/%d/%y",                        /* OptionDateFormat */
   "%H:%M",                        /* OptionTimeFormat */
   '$',                                /* OptionStringChar */
   '#',                                /* OptionDoubleChar */
   '!',                                /* OptionSingleChar */
   '\0',                                /* OptionCurrencyChar */
   '&',                                /* OptionLongChar */
   '%',                                /* OptionIntegerChar */
   '\0',                                /* OptionByteChar */
   '"',                                /* OptionQuoteChar */
   '\0',                        /* OptionCommentChar */
   '\0',                        /* OptionStatementChar */
   '\0',                        /* OptionPrintChar */
   '\0',                        /* OptionInputChar */
   ':',                                /* OptionImageChar */
   '(',                                /* OptionLparenChar */
   ')',                                /* OptionRparenChar */
   '#',                                /* OptionFilenumChar */
   '\0',                        /* OptionAtChar */
   '#',                                /* OptionUsingDigit */
   ',',                                /* OptionUsingComma */
   '.',                                /* OptionUsingPeriod */
   '+',                                /* OptionUsingPlus */
   '-',                                /* OptionUsingMinus */
   '^',                                /* OptionUsingExrad */
   '$',                                /* OptionUsingDollar */
   '*',                                /* OptionUsingFiller */
   '_',                                /* OptionUsingLiteral */
   '!',                                /* OptionUsingFirst */
   '&',                                /* OptionUsingAll */
   '%',                                /* OptionUsingLength */
   }
  ,
  {
   "ECMA-55",                        /* Name */
   E78,                                /* OptionVersionBitmask */
   "E78",                        /* ID */
   "ANSI Minimal BASIC",        /* Description */
   "STANDARD ECMA-55: Minimal BASIC",        /* Reference Title */
   "by EUROPEAN COMPUTER MANUFACTURERS ASSOCIATION",        /* Reference Author */
   "(c) 1978, EUROPEAN COMPUTER MANUFACTURERS ASSOCIATION",        /* Reference Copyright */
   "http://www.ecma-international.org/publications/files/",        /* Reference URL1 */
   "ECMA-ST-WITHDRAWN/ECMA-55,%201st%20Edition,%20January%201978.pdf",        /* Reference URL2 */
   FALSE,                        /* OptionFlags */
   128,                                /* OptionReclenInteger */
   0,                                /* OptionBaseInteger */
   "%m/%d/%Y",                        /* OptionDateFormat */
   "%H:%M:%S",                        /* OptionTimeFormat */
   '$',                                /* OptionStringChar */
   '#',                                /* OptionDoubleChar */
   '!',                                /* OptionSingleChar */
   '\0',                                /* OptionCurrencyChar */
   '&',                                /* OptionLongChar */
   '%',                                /* OptionIntegerChar */
   '\0',                                /* OptionByteChar */
   '"',                                /* OptionQuoteChar */
   '\0',                        /* OptionCommentChar */
   '\0',                        /* OptionStatementChar */
   '\0',                        /* OptionPrintChar */
   '\0',                        /* OptionInputChar */
   '\0',                        /* OptionImageChar */
   '(',                                /* OptionLparenChar */
   ')',                                /* OptionRparenChar */
   '#',                                /* OptionFilenumChar */
   '\0',                        /* OptionAtChar */
   '#',                                /* OptionUsingDigit */
   ',',                                /* OptionUsingComma */
   '.',                                /* OptionUsingPeriod */
   '+',                                /* OptionUsingPlus */
   '-',                                /* OptionUsingMinus */
   '^',                                /* OptionUsingExrad */
   '$',                                /* OptionUsingDollar */
   '*',                                /* OptionUsingFiller */
   '_',                                /* OptionUsingLiteral */
   '!',                                /* OptionUsingFirst */
   '&',                                /* OptionUsingAll */
   '%',                                /* OptionUsingLength */
   }
  ,
  {
   "ECMA-116",                        /* Name */
   E86,                                /* OptionVersionBitmask */
   "E86",                        /* ID */
   "ANSI Full BASIC",                /* Description */
   "STANDARD ECMA-116: Full BASIC",        /* Reference Title */
   "by EUROPEAN COMPUTER MANUFACTURERS ASSOCIATION",        /* Reference Author */
   "(c) 1986, EUROPEAN COMPUTER MANUFACTURERS ASSOCIATION",        /* Reference Copyright */
   "http://www.ecma-international.org/publications/files/",        /* Reference URL1 */
   "ECMA-ST-WITHDRAWN/ECMA-116,%201st%20edition,%20June%201986.pdf",        /* Reference URL2 */
   OPTION_LABELS_ON | OPTION_STRICT_ON,        /* OptionFlags */
   128,                                /* OptionReclenInteger */
   1,                                /* OptionBaseInteger */
   "%Y-%m-%d",                        /* OptionDateFormat */
   "%H:%M:%S",                        /* OptionTimeFormat */
   '$',                                /* OptionStringChar */
   '#',                                /* OptionDoubleChar */
   '!',                                /* OptionSingleChar */
   '\0',                                /* OptionCurrencyChar */
   '&',                                /* OptionLongChar */
   '%',                                /* OptionIntegerChar */
   '\0',                                /* OptionByteChar */
   '"',                                /* OptionQuoteChar */
   '!',                                /* OptionCommentChar */
   '\0',                        /* OptionStatementChar */
   '\0',                        /* OptionPrintChar */
   '\0',                        /* OptionInputChar */
   '\0',                        /* OptionImageChar */
   '(',                                /* OptionLparenChar */
   ')',                                /* OptionRparenChar */
   '#',                                /* OptionFilenumChar */
   '\0',                        /* OptionAtChar */
   '#',                                /* OptionUsingDigit */
   ',',                                /* OptionUsingComma */
   '.',                                /* OptionUsingPeriod */
   '+',                                /* OptionUsingPlus */
   '-',                                /* OptionUsingMinus */
   '^',                                /* OptionUsingExrad */
   '$',                                /* OptionUsingDollar */
   '*',                                /* OptionUsingFiller */
   '_',                                /* OptionUsingLiteral */
   '!',                                /* OptionUsingFirst */
   '&',                                /* OptionUsingAll */
   '%',                                /* OptionUsingLength */
   }
  ,
  {
   "GCOS",                        /* Name */
   G74,                                /* OptionVersionBitmask */
   "G74",                        /* ID */
   "GE 600 Mainframe BASIC",        /* Description */
   "TIME-SHARING SYSTEM POCKET GUIDE, SERIES 600/6000 GCOS",        /* Reference Title */
   "by Honeywell",                /* Reference Author */
   "(c) 1974, Honeywell",        /* Reference Copyright */
   "http://www.trailingedge.com/misc/",        /* Reference URL1 */
   "GCOS-TSS-PocketGuide.pdf",        /* Reference URL2 */
   FALSE,                        /* OptionFlags */
   128,                                /* OptionReclenInteger */
   0,                                /* OptionBaseInteger */
   "%m/%d/%y",                        /* OptionDateFormat */
   "%H:%M",                        /* OptionTimeFormat */
   '$',                                /* OptionStringChar */
   '#',                                /* OptionDoubleChar */
   '!',                                /* OptionSingleChar */
   '\0',                                /* OptionCurrencyChar */
   '&',                                /* OptionLongChar */
   '%',                                /* OptionIntegerChar */
   '\0',                                /* OptionByteChar */
   '"',                                /* OptionQuoteChar */
   '\'',                        /* OptionCommentChar */
   '\0',                        /* OptionStatementChar */
   '\0',                        /* OptionPrintChar */
   '\0',                        /* OptionInputChar */
   ':',                                /* OptionImageChar */
   '(',                                /* OptionLparenChar */
   ')',                                /* OptionRparenChar */
   '#',                                /* OptionFilenumChar */
   '\0',                        /* OptionAtChar */
   '#',                                /* OptionUsingDigit */
   ',',                                /* OptionUsingComma */
   '.',                                /* OptionUsingPeriod */
   '+',                                /* OptionUsingPlus */
   '-',                                /* OptionUsingMinus */
   '^',                                /* OptionUsingExrad */
   '$',                                /* OptionUsingDollar */
   '*',                                /* OptionUsingFiller */
   '_',                                /* OptionUsingLiteral */
   '!',                                /* OptionUsingFirst */
   '&',                                /* OptionUsingAll */
   '%',                                /* OptionUsingLength */
   }
  ,
  {
   "HAARDT",                        /* Name */
   H14,                                /* OptionVersionBitmask */
   "H14",                        /* ID */
   "bas 2.4 by Michael Haardt",        /* Description */
   "BAS",                        /* Reference Title */
   "by by Michael Haardt",        /* Reference Author */
   "(c) 2014, by Michael Haardt",        /* Reference Copyright */
   "http://www.moria.de/~michael/bas/",        /* Reference URL1 */
   "bas-2.4.tar.gz",                /* Reference URL2 */
   OPTION_BUGS_ON,                /* OptionFlags */
   1,                                /* OptionReclenInteger */
   0,                                /* OptionBaseInteger */
   "%m/%d/%Y",                        /* OptionDateFormat */
   "%H:%M:%S",                        /* OptionTimeFormat */
   '$',                                /* OptionStringChar */
   '#',                                /* OptionDoubleChar */
   '!',                                /* OptionSingleChar */
   '\0',                        /* OptionCurrencyChar */
   '\0',                        /* OptionLongChar */
   '%',                                /* OptionIntegerChar */
   '\0',                        /* OptionByteChar */
   '"',                                /* OptionQuoteChar */
   '\'',                        /* OptionCommentChar */
   ':',                                /* OptionStatementChar */
   '?',                                /* OptionPrintChar */
   '\0',                        /* OptionInputChar */
   '\0',                        /* OptionImageChar */
   '(',                                /* OptionLparenChar */
   ')',                                /* OptionRparenChar */
   '#',                                /* OptionFilenumChar */
   '\0',                        /* OptionAtChar */
   '#',                                /* OptionUsingDigit */
   ',',                                /* OptionUsingComma */
   '.',                                /* OptionUsingPeriod */
   '+',                                /* OptionUsingPlus */
   '-',                                /* OptionUsingMinus */
   '^',                                /* OptionUsingExrad */
   '$',                                /* OptionUsingDollar */
   '*',                                /* OptionUsingFiller */
   '_',                                /* OptionUsingLiteral */
   '!',                                /* OptionUsingFirst */
   '&',                                /* OptionUsingAll */
   '\\',                        /* OptionUsingLength */
   }
  ,
  {
   "HANDBOOK1",                        /* Name */
   HB1,                                /* OptionVersionBitmask */
   "HB1",                        /* ID */
   "The BASIC Handbook, 1st Edition",        /* Description */
   "The BASIC Handbook: Encyclopedia of the BASIC Computer Language",        /* Reference Title */
   "by David A. Lien",                /* Reference Author */
   "(c) 1978, Compusoft Publishing",        /* Reference Copyright */
   "ISBN 0-932760-00-7",        /* Reference URL1 */
   "(1st Edition) 364 pages",        /* Reference URL2 */
   OPTION_BUGS_ON,                /* OptionFlags */
   128,                                /* OptionReclenInteger */
   0,                                /* OptionBaseInteger */
   "%m/%d/%Y",                        /* OptionDateFormat */
   "%H:%M:%S",                        /* OptionTimeFormat */
   '$',                                /* OptionStringChar */
   '#',                                /* OptionDoubleChar */
   '!',                                /* OptionSingleChar */
   '\0',                                /* OptionCurrencyChar */
   '&',                                /* OptionLongChar */
   '%',                                /* OptionIntegerChar */
   '\0',                                /* OptionByteChar */
   '"',                                /* OptionQuoteChar */
   '\'',                        /* OptionCommentChar */
   ':',                                /* OptionStatementChar */
   '?',                                /* OptionPrintChar */
   '\0',                        /* OptionInputChar */
   '\0',                        /* OptionImageChar */
   '(',                                /* OptionLparenChar */
   ')',                                /* OptionRparenChar */
   '#',                                /* OptionFilenumChar */
   '@',                                /* OptionAtChar */
   '#',                                /* OptionUsingDigit */
   ',',                                /* OptionUsingComma */
   '.',                                /* OptionUsingPeriod */
   '+',                                /* OptionUsingPlus */
   '-',                                /* OptionUsingMinus */
   '^',                                /* OptionUsingExrad */
   '$',                                /* OptionUsingDollar */
   '*',                                /* OptionUsingFiller */
   '_',                                /* OptionUsingLiteral */
   '!',                                /* OptionUsingFirst */
   '&',                                /* OptionUsingAll */
   '%',                                /* OptionUsingLength */
   }
  ,
  {
   "HANDBOOK2",                        /* Name */
   HB2,                                /* OptionVersionBitmask */
   "HB2",                        /* ID */
   "The BASIC Handbook, 2nd Edition",        /* Description */
   "The BASIC Handbook: Encyclopedia of the BASIC Computer Language",        /* Reference Title */
   "by David A. Lien",                /* Reference Author */
   "(c) 1981, CompuSoft Publishing",        /* Reference Copyright */
   "ISBN 0-932760-05-8",        /* Reference URL1 */
   "(2nd Edition) 480 pages",        /* Reference URL2 */
   OPTION_BUGS_ON,                /* OptionFlags */
   128,                                /* OptionReclenInteger */
   0,                                /* OptionBaseInteger */
   "%m/%d/%Y",                        /* OptionDateFormat */
   "%H:%M:%S",                        /* OptionTimeFormat */
   '$',                                /* OptionStringChar */
   '#',                                /* OptionDoubleChar */
   '!',                                /* OptionSingleChar */
   '\0',                                /* OptionCurrencyChar */
   '&',                                /* OptionLongChar */
   '%',                                /* OptionIntegerChar */
   '\0',                                /* OptionByteChar */
   '"',                                /* OptionQuoteChar */
   '\'',                        /* OptionCommentChar */
   ':',                                /* OptionStatementChar */
   '?',                                /* OptionPrintChar */
   '\0',                        /* OptionInputChar */
   '\0',                        /* OptionImageChar */
   '(',                                /* OptionLparenChar */
   ')',                                /* OptionRparenChar */
   '#',                                /* OptionFilenumChar */
   '@',                                /* OptionAtChar */
   '#',                                /* OptionUsingDigit */
   ',',                                /* OptionUsingComma */
   '.',                                /* OptionUsingPeriod */
   '+',                                /* OptionUsingPlus */
   '-',                                /* OptionUsingMinus */
   '^',                                /* OptionUsingExrad */
   '$',                                /* OptionUsingDollar */
   '*',                                /* OptionUsingFiller */
   '_',                                /* OptionUsingLiteral */
   '!',                                /* OptionUsingFirst */
   '&',                                /* OptionUsingAll */
   '%',                                /* OptionUsingLength */
   }
  ,
  {
   "HEATH",                        /* Name */
   H80,                                /* OptionVersionBitmask */
   "H80",                        /* ID */
   "Heath Benton Harbor BASIC",        /* Description */
   "Extended Benton Harbor BASIC",        /* Reference Title */
   "by Heath Company",                /* Reference Author */
   "(c) 1980, Heath Company",        /* Reference Copyright */
   "http://www.lesbird.com/sebhc/documentation/software/HDOS-2/",        /* Reference URL1 */
   "HDOS_Reference_Chapter6.pdf",        /* Reference URL2 */
   OPTION_BUGS_ON,                /* OptionFlags */
   0,                                /* OptionReclenInteger */
   0,                                /* OptionBaseInteger */
   "",                                /* OptionDateFormat */
   "",                                /* OptionTimeFormat */
   '$',                                /* OptionStringChar */
   '#',                                /* OptionDoubleChar */
   '!',                                /* OptionSingleChar */
   '\0',                                /* OptionCurrencyChar */
   '&',                                /* OptionLongChar */
   '%',                                /* OptionIntegerChar */
   '\0',                                /* OptionByteChar */
   '"',                                /* OptionQuoteChar */
   '\0',                        /* OptionCommentChar */
   ':',                                /* OptionStatementChar */
   '\0',                        /* OptionPrintChar */
   '\0',                        /* OptionInputChar */
   '\0',                        /* OptionImageChar */
   '(',                                /* OptionLparenChar */
   ')',                                /* OptionRparenChar */
   '#',                                /* OptionFilenumChar */
   '\0',                        /* OptionAtChar */
   '#',                                /* OptionUsingDigit */
   ',',                                /* OptionUsingComma */
   '.',                                /* OptionUsingPeriod */
   '+',                                /* OptionUsingPlus */
   '-',                                /* OptionUsingMinus */
   '^',                                /* OptionUsingExrad */
   '$',                                /* OptionUsingDollar */
   '*',                                /* OptionUsingFiller */
   '_',                                /* OptionUsingLiteral */
   '!',                                /* OptionUsingFirst */
   '&',                                /* OptionUsingAll */
   '%',                                /* OptionUsingLength */
   }
  ,
  {
   "MARK-I",                        /* Name */
   G65,                                /* OptionVersionBitmask */
   "G65",                        /* ID */
   "GE 265 Mainframe BASIC",        /* Description */
   "BASIC LANGUAGE REFERENCE MANUAL",        /* Reference Title */
   "by Time-Sharing Service, Information Service Department, General Electric",        /* Reference Author */
   "(c) 1965, Trustees of Dartmouth College",        /* Reference Copyright */
   "http://www.bitsavers.org/pdf/ge/MarkI_Timesharing/",        /* Reference URL1 */
   "202026A_BASIC_Language_Reference_Manual_Jan1967.pdf",        /* Reference URL2 */
   FALSE,                        /* OptionFlags */
   128,                                /* OptionReclenInteger */
   0,                                /* OptionBaseInteger */
   "%m/%d/%y",                        /* OptionDateFormat */
   "%H:%M",                        /* OptionTimeFormat */
   '$',                                /* OptionStringChar */
   '#',                                /* OptionDoubleChar */
   '!',                                /* OptionSingleChar */
   '\0',                                /* OptionCurrencyChar */
   '&',                                /* OptionLongChar */
   '%',                                /* OptionIntegerChar */
   '\0',                                /* OptionByteChar */
   '"',                                /* OptionQuoteChar */
   '\0',                        /* OptionCommentChar */
   '\0',                        /* OptionStatementChar */
   '\0',                        /* OptionPrintChar */
   '\0',                        /* OptionInputChar */
   ':',                                /* OptionImageChar */
   '(',                                /* OptionLparenChar */
   ')',                                /* OptionRparenChar */
   '#',                                /* OptionFilenumChar */
   '\0',                        /* OptionAtChar */
   '#',                                /* OptionUsingDigit */
   ',',                                /* OptionUsingComma */
   '.',                                /* OptionUsingPeriod */
   '+',                                /* OptionUsingPlus */
   '-',                                /* OptionUsingMinus */
   '^',                                /* OptionUsingExrad */
   '$',                                /* OptionUsingDollar */
   '*',                                /* OptionUsingFiller */
   '_',                                /* OptionUsingLiteral */
   '!',                                /* OptionUsingFirst */
   '&',                                /* OptionUsingAll */
   '%',                                /* OptionUsingLength */
   }
  ,
  {
   "MARK-II",                        /* Name */
   G67,                                /* OptionVersionBitmask */
   "G67",                        /* ID */
   "GE 435 Mainframe BASIC",        /* Description */
   "BASIC Language Reference Manual",        /* Reference Title */
   "by Time-Sharing Service, Information Service Department, General Electric",        /* Reference Author */
   "(c) 1968, General Electric Company and the Trustees of Dartmouth College",        /* Reference Copyright */
   "http://www.bitsavers.org/pdf/ge/MarkI_Timesharing/",        /* Reference URL1 */
   "711224A_BASICLanguageReferenceManual_Nov1970.pdf",        /* Reference URL2 */
   FALSE,                        /* OptionFlags */
   128,                                /* OptionReclenInteger */
   0,                                /* OptionBaseInteger */
   "%m/%d/%y",                        /* OptionDateFormat */
   "%H:%M",                        /* OptionTimeFormat */
   '$',                                /* OptionStringChar */
   '#',                                /* OptionDoubleChar */
   '!',                                /* OptionSingleChar */
   '\0',                                /* OptionCurrencyChar */
   '&',                                /* OptionLongChar */
   '%',                                /* OptionIntegerChar */
   '\0',                                /* OptionByteChar */
   '"',                                /* OptionQuoteChar */
   '\'',                        /* OptionCommentChar */
   '\0',                        /* OptionStatementChar */
   '\0',                        /* OptionPrintChar */
   '\0',                        /* OptionInputChar */
   ':',                                /* OptionImageChar */
   '(',                                /* OptionLparenChar */
   ')',                                /* OptionRparenChar */
   '#',                                /* OptionFilenumChar */
   '\0',                        /* OptionAtChar */
   '#',                                /* OptionUsingDigit */
   ',',                                /* OptionUsingComma */
   '.',                                /* OptionUsingPeriod */
   '+',                                /* OptionUsingPlus */
   '-',                                /* OptionUsingMinus */
   '^',                                /* OptionUsingExrad */
   '$',                                /* OptionUsingDollar */
   '*',                                /* OptionUsingFiller */
   '_',                                /* OptionUsingLiteral */
   '!',                                /* OptionUsingFirst */
   '&',                                /* OptionUsingAll */
   '%',                                /* OptionUsingLength */
   }
  ,
  {
   "MBASIC",                        /* Name */
   M80,                                /* OptionVersionBitmask */
   "M80",                        /* ID */
   "Microsoft BASIC-80 for Xenix",        /* Description */
   "Microsoft BASIC Reference Manual",        /* Reference Title */
   "by Microsoft",                /* Reference Author */
   "(c) 1982, Microsoft",        /* Reference Copyright */
   "http://chiclassiccomp.org/docs/content/computing/Microsoft/",        /* Reference URL1 */
   "Microsoft_Basic_8086Xenix_Reference.pdf",        /* Reference URL2 */
   OPTION_BUGS_ON,                /* OptionFlags */
   128,                                /* OptionReclenInteger */
   0,                                /* OptionBaseInteger */
   "%m/%d/%Y",                        /* OptionDateFormat */
   "%H:%M:%S",                        /* OptionTimeFormat */
   '$',                                /* OptionStringChar */
   '#',                                /* OptionDoubleChar */
   '!',                                /* OptionSingleChar */
   '\0',                                /* OptionCurrencyChar */
   '&',                                /* OptionLongChar */
   '%',                                /* OptionIntegerChar */
   '\0',                                /* OptionByteChar */
   '"',                                /* OptionQuoteChar */
   '\'',                        /* OptionCommentChar */
   ':',                                /* OptionStatementChar */
   '?',                                /* OptionPrintChar */
   '\0',                        /* OptionInputChar */
   '\0',                        /* OptionImageChar */
   '(',                                /* OptionLparenChar */
   ')',                                /* OptionRparenChar */
   '#',                                /* OptionFilenumChar */
   '\0',                        /* OptionAtChar */
   '#',                                /* OptionUsingDigit */
   ',',                                /* OptionUsingComma */
   '.',                                /* OptionUsingPeriod */
   '+',                                /* OptionUsingPlus */
   '-',                                /* OptionUsingMinus */
   '^',                                /* OptionUsingExrad */
   '$',                                /* OptionUsingDollar */
   '*',                                /* OptionUsingFiller */
   '_',                                /* OptionUsingLiteral */
   '!',                                /* OptionUsingFirst */
   '&',                                /* OptionUsingAll */
   '\\',                        /* OptionUsingLength */
   }
  ,
  {
   "PDP-8",                        /* Name */
   D73,                                /* OptionVersionBitmask */
   "D73",                        /* ID */
   "DEC PDP-8 BASIC",                /* Description */
   "DEC 8K BASIC",                /* Reference Title */
   "by Digital Equipement Corporation",        /* Reference Author */
   "(c) 1973, Digital Equipement Corporation",        /* Reference Copyright */
   "http://bitsavers.trailing-edge.com/pdf/dec/pdp8/basic/",        /* Reference URL1 */
   "DEC-08-LBSMA-A-D_8K_BASIC_Jul73.pdf",        /* Reference URL2 */
   OPTION_BUGS_ON,                /* OptionFlags */
   128,                                /* OptionReclenInteger */
   0,                                /* OptionBaseInteger */
   "",                                /* OptionDateFormat */
   "",                                /* OptionTimeFormat */
   '$',                                /* OptionStringChar */
   '#',                                /* OptionDoubleChar */
   '!',                                /* OptionSingleChar */
   '\0',                                /* OptionCurrencyChar */
   '&',                                /* OptionLongChar */
   '%',                                /* OptionIntegerChar */
   '\0',                                /* OptionByteChar */
   '"',                                /* OptionQuoteChar */
   '\0',                        /* OptionCommentChar */
   '\\',                        /* OptionStatementChar */
   '\0',                        /* OptionPrintChar */
   '\0',                        /* OptionInputChar */
   '\0',                        /* OptionImageChar */
   '(',                                /* OptionLparenChar */
   ')',                                /* OptionRparenChar */
   '#',                                /* OptionFilenumChar */
   '\0',                        /* OptionAtChar */
   '#',                                /* OptionUsingDigit */
   ',',                                /* OptionUsingComma */
   '.',                                /* OptionUsingPeriod */
   '+',                                /* OptionUsingPlus */
   '-',                                /* OptionUsingMinus */
   '^',                                /* OptionUsingExrad */
   '$',                                /* OptionUsingDollar */
   '*',                                /* OptionUsingFiller */
   '_',                                /* OptionUsingLiteral */
   '!',                                /* OptionUsingFirst */
   '&',                                /* OptionUsingAll */
   '\\',                        /* OptionUsingLength */
   }
  ,
  {
   "PDP-11",                        /* Name */
   D70,                                /* OptionVersionBitmask */
   "D70",                        /* ID */
   "DEC PDP-11 BASIC",                /* Description */
   "PDP-11 BASIC Programming Manual",        /* Reference Title */
   "by Digital Equipement Corporation",        /* Reference Author */
   "(c) 1970, Digital Equipement Corporation",        /* Reference Copyright */
   "http://bitsavers.trailing-edge.com/pdf/dec/pdp11/basic/",        /* Reference URL1 */
   "DEC-11-AJPB-D_PDP-11_BASIC_Programming_Manual_Dec70.pdf",        /* Reference URL2 */
   OPTION_BUGS_ON,                /* OptionFlags */
   128,                                /* OptionReclenInteger */
   0,                                /* OptionBaseInteger */
   "",                                /* OptionDateFormat */
   "",                                /* OptionTimeFormat */
   '$',                                /* OptionStringChar */
   '#',                                /* OptionDoubleChar */
   '!',                                /* OptionSingleChar */
   '\0',                                /* OptionCurrencyChar */
   '&',                                /* OptionLongChar */
   '%',                                /* OptionIntegerChar */
   '\0',                                /* OptionByteChar */
   '"',                                /* OptionQuoteChar */
   '\0',                        /* OptionCommentChar */
   ':',                                /* OptionStatementChar */
   '\0',                        /* OptionPrintChar */
   '\0',                        /* OptionInputChar */
   '\0',                        /* OptionImageChar */
   '(',                                /* OptionLparenChar */
   ')',                                /* OptionRparenChar */
   '#',                                /* OptionFilenumChar */
   '\0',                        /* OptionAtChar */
   '#',                                /* OptionUsingDigit */
   ',',                                /* OptionUsingComma */
   '.',                                /* OptionUsingPeriod */
   '+',                                /* OptionUsingPlus */
   '-',                                /* OptionUsingMinus */
   '^',                                /* OptionUsingExrad */
   '$',                                /* OptionUsingDollar */
   '*',                                /* OptionUsingFiller */
   '_',                                /* OptionUsingLiteral */
   '!',                                /* OptionUsingFirst */
   '&',                                /* OptionUsingAll */
   '\\',                        /* OptionUsingLength */
   }
  ,
  {
   "RBASIC",                        /* Name */
   R86,                                /* OptionVersionBitmask */
   "R86",                        /* ID */
   "Micronics RBASIC for 6809 FLEX",        /* Description */
   "RBASIC User's Manual",        /* Reference Title */
   "by R. Jones",                /* Reference Author */
   "(c) 1986, Micronics Research Corp.",        /* Reference Copyright */
   "http://www.evenson-consulting.com/swtpc/downloads/",        /* Reference URL1 */
   "RBASIC.zip",                /* Reference URL2 */
   OPTION_BUGS_ON,                /* OptionFlags */
   0,                                /* OptionReclenInteger */
   0,                                /* OptionBaseInteger */
   "%d-%b-%y",                        /* OptionDateFormat */
   "%H:%M:%S",                        /* OptionTimeFormat */
   '$',                                /* OptionStringChar */
   '#',                                /* OptionDoubleChar */
   '!',                                /* OptionSingleChar */
   '\0',                                /* OptionCurrencyChar */
   '&',                                /* OptionLongChar */
   '%',                                /* OptionIntegerChar */
   '\0',                                /* OptionByteChar */
   '"',                                /* OptionQuoteChar */
   '\0',                        /* OptionCommentChar */
   ':',                                /* OptionStatementChar */
   '?',                                /* OptionPrintChar */
   '!',                                /* OptionInputChar */
   '\0',                        /* OptionImageChar */
   '(',                                /* OptionLparenChar */
   ')',                                /* OptionRparenChar */
   '#',                                /* OptionFilenumChar */
   '\0',                        /* OptionAtChar */
   '#',                                /* OptionUsingDigit */
   ',',                                /* OptionUsingComma */
   '.',                                /* OptionUsingPeriod */
   '+',                                /* OptionUsingPlus */
   '-',                                /* OptionUsingMinus */
   '^',                                /* OptionUsingExrad */
   '$',                                /* OptionUsingDollar */
   '*',                                /* OptionUsingFiller */
   '_',                                /* OptionUsingLiteral */
   '!',                                /* OptionUsingFirst */
   '&',                                /* OptionUsingAll */
   '\\',                        /* OptionUsingLength */
   }
  ,
  {
   "RSTS-11",                        /* Name */
   D71,                                /* OptionVersionBitmask */
   "D71",                        /* ID */
   "DEC RSTS-11 BASIC-PLUS",        /* Description */
   "RSTS-11 User's Guide : BASIC-PLUS Programming Language",        /* Reference Title */
   "by Digital Equipment Corporation",        /* Reference Author */
   "(c) 1971, Digital Equipment Corporation",        /* Reference Copyright */
   "http://bitsavers.trailing-edge.com/pdf/dec/pdp11/rsts/",        /* Reference URL1 */
   "PL-11-71-01-01-A-D_RSTS-11UsersGuide_May71.pdf",        /* Reference URL2 */
   OPTION_BUGS_ON,                /* OptionFlags */
   512,                                /* OptionReclenInteger */
   0,                                /* OptionBaseInteger */
   "%d-%b-%y",                        /* OptionDateFormat */
   "%I:%M %p",                        /* OptionTimeFormat */
   '$',                                /* OptionStringChar */
   '#',                                /* OptionDoubleChar */
   '!',                                /* OptionSingleChar */
   '\0',                                /* OptionCurrencyChar */
   '&',                                /* OptionLongChar */
   '%',                                /* OptionIntegerChar */
   '\0',                                /* OptionByteChar */
   '"',                                /* OptionQuoteChar */
   '!',                                /* OptionCommentChar */
   ':',                                /* OptionStatementChar */
   '?',                                /* OptionPrintChar */
   '\0',                        /* OptionInputChar */
   '\0',                        /* OptionImageChar */
   '(',                                /* OptionLparenChar */
   ')',                                /* OptionRparenChar */
   '#',                                /* OptionFilenumChar */
   '\0',                        /* OptionAtChar */
   '#',                                /* OptionUsingDigit */
   ',',                                /* OptionUsingComma */
   '.',                                /* OptionUsingPeriod */
   '+',                                /* OptionUsingPlus */
   '-',                                /* OptionUsingMinus */
   '^',                                /* OptionUsingExrad */
   '$',                                /* OptionUsingDollar */
   '*',                                /* OptionUsingFiller */
   '_',                                /* OptionUsingLiteral */
   '!',                                /* OptionUsingFirst */
   '&',                                /* OptionUsingAll */
   '\\',                        /* OptionUsingLength */
   }
  ,
  {
   "SYSTEM/360",                /* Name */
   I70,                                /* OptionVersionBitmask */
   "I70",                        /* ID */
   "IBM System/360 Mainframe BASIC",        /* Description */
   "BASIC Language Reference Manual",        /* Reference Title */
   "by International Business Machines Corporation",        /* Reference Author */
   "(c) 1970, International Business Machines Corporation",        /* Reference Copyright */
   "http://bitsavers.org/pdf/ibm/360/os/tso/",        /* Reference URL1 */
   "GC28-6837-0_BASIC_Language_Ref_Man_Jun70.pdf",        /* Reference URL2 */
   FALSE,                        /* OptionFlags */
   0,                                /* OptionReclenInteger */
   1,                                /* OptionBaseInteger */
   "%Y/%m/%d",                        /* OptionDateFormat */
   "%H:%M:%S",                        /* OptionTimeFormat */
   '$',                                /* OptionStringChar */
   '\0',                        /* OptionDoubleChar */
   '\0',                        /* OptionSingleChar */
   '\0',                        /* OptionCurrencyChar */
   '\0',                        /* OptionLongChar */
   '\0',                        /* OptionIntegerChar */
   '\0',                        /* OptionByteChar */
   '"',                                /* OptionQuoteChar */
   '\0',                        /* OptionCommentChar */
   '\0',                        /* OptionStatementChar */
   '\0',                        /* OptionPrintChar */
   '\0',                        /* OptionInputChar */
   ':',                                /* OptionImageChar */
   '(',                                /* OptionLparenChar */
   ')',                                /* OptionRparenChar */
   '\0',                        /* OptionFilenumChar */
   '\0',                        /* OptionAtChar */
   '#',                                /* OptionUsingDigit */
   '\0',                        /* OptionUsingComma */
   '.',                                /* OptionUsingPeriod */
   '+',                                /* OptionUsingPlus */
   '-',                                /* OptionUsingMinus */
   '!',                                /* OptionUsingExrad */
   '\0',                        /* OptionUsingDollar */
   '\0',                        /* OptionUsingFiller */
   '\0',                        /* OptionUsingLiteral */
   '\0',                        /* OptionUsingFirst */
   '\0',                        /* OptionUsingAll */
   '\0',                        /* OptionUsingLength */
   }
  ,
  {
   "SYSTEM/370",                /* Name */
   I73,                                /* OptionVersionBitmask */
   "I73",                        /* ID */
   "IBM System/370 Mainframe BASIC",        /* Description */
   "IBM Virtual Machine Facility/370: BASIC Language Reference Manual",        /* Reference Title */
   "by International Business Machines Corporation",        /* Reference Author */
   "(c) 1973, International Business Machines Corporation",        /* Reference Copyright */
   "http://bitsavers.trailing-edge.com/pdf/ibm/370/VM_370/Release_1/",        /* Reference URL1 */
   "GC20-1803-1_VM370_BASIC_Language_Reference_Manual_Rel_1_Apr73.pdf",        /* Reference URL2 */
   FALSE,                        /* OptionFlags */
   0,                                /* OptionReclenInteger */
   1,                                /* OptionBaseInteger */
   "%Y/%m/%d",                        /* OptionDateFormat */
   "%H:%M:%S",                        /* OptionTimeFormat */
   '$',                                /* OptionStringChar */
   '\0',                        /* OptionDoubleChar */
   '\0',                        /* OptionSingleChar */
   '\0',                        /* OptionCurrencyChar */
   '\0',                        /* OptionLongChar */
   '\0',                        /* OptionIntegerChar */
   '\0',                        /* OptionByteChar */
   '"',                                /* OptionQuoteChar */
   '\0',                        /* OptionCommentChar */
   '\0',                        /* OptionStatementChar */
   '\0',                        /* OptionPrintChar */
   '\0',                        /* OptionInputChar */
   ':',                                /* OptionImageChar */
   '(',                                /* OptionLparenChar */
   ')',                                /* OptionRparenChar */
   '\0',                        /* OptionFilenumChar */
   '\0',                        /* OptionAtChar */
   '#',                                /* OptionUsingDigit */
   '\0',                        /* OptionUsingComma */
   '.',                                /* OptionUsingPeriod */
   '+',                                /* OptionUsingPlus */
   '-',                                /* OptionUsingMinus */
   '!',                                /* OptionUsingExrad */
   '\0',                        /* OptionUsingDollar */
   '\0',                        /* OptionUsingFiller */
   '\0',                        /* OptionUsingLiteral */
   '\0',                        /* OptionUsingFirst */
   '\0',                        /* OptionUsingAll */
   '\0',                        /* OptionUsingLength */
   }
  ,
  {
   "TRS-80",                        /* Name */
   T80,                                /* OptionVersionBitmask */
   "T80",                        /* ID */
   "TRS-80 Model I/III/4 LBASIC",        /* Description */
   "LBASIC USER'S GUIDE",        /* Reference Title */
   "by Galactic Software, Ltd.",        /* Reference Author */
   "(c) 1981, Galactic Software, Ltd.",        /* Reference Copyright */
   "http://www.tim-mann.org/trs80/doc/",        /* Reference URL1 */
   "lbasic.txt",                /* Reference URL2 */
   OPTION_BUGS_ON,                /* OptionFlags */
   128,                                /* OptionReclenInteger */
   0,                                /* OptionBaseInteger */
   "",                                /* OptionDateFormat */
   "%m/%d/%y %H:%M:%S",                /* OptionTimeFormat */
   '$',                                /* OptionStringChar */
   '#',                                /* OptionDoubleChar */
   '!',                                /* OptionSingleChar */
   '\0',                                /* OptionCurrencyChar */
   '&',                                /* OptionLongChar */
   '%',                                /* OptionIntegerChar */
   '\0',                                /* OptionByteChar */
   '"',                                /* OptionQuoteChar */
   '\'',                        /* OptionCommentChar */
   ':',                                /* OptionStatementChar */
   '?',                                /* OptionPrintChar */
   '\0',                        /* OptionInputChar */
   '\0',                        /* OptionImageChar */
   '(',                                /* OptionLparenChar */
   ')',                                /* OptionRparenChar */
   '#',                                /* OptionFilenumChar */
   '@',                                /* OptionAtChar */
   '#',                                /* OptionUsingDigit */
   ',',                                /* OptionUsingComma */
   '.',                                /* OptionUsingPeriod */
   '+',                                /* OptionUsingPlus */
   '-',                                /* OptionUsingMinus */
   '^',                                /* OptionUsingExrad */
   '$',                                /* OptionUsingDollar */
   '*',                                /* OptionUsingFiller */
   '_',                                /* OptionUsingLiteral */
   '!',                                /* OptionUsingFirst */
   '&',                                /* OptionUsingAll */
   '%',                                /* OptionUsingLength */
   }
  ,
  {
   "VINTAGE",                        /* Name */
   V09,                                /* OptionVersionBitmask */
   "V09",                        /* ID */
   "Vintage BASIC 1.0.1",        /* Description */
   "Vintage BASIC User's Guide 1.0.1",        /* Reference Title */
   "by Lyle Kopnicky",                /* Reference Author */
   "(c) 2009, Lyle Kopnicky",        /* Reference Copyright */
   "http://www.vintage-basic.net/",        /* Reference URL1 */
   "vintage-basic-1.0.1.tar.gz",        /* Reference URL2 */
   OPTION_BUGS_ON,                /* OptionFlags */
   128,                                /* OptionReclenInteger */
   0,                                /* OptionBaseInteger */
   "",                                /* OptionDateFormat */
   "",                                /* OptionTimeFormat */
   '$',                                /* OptionStringChar */
   '\0',                        /* OptionDoubleChar */
   '\0',                        /* OptionSingleChar */
   '\0',                        /* OptionCurrencyChar */
   '\0',                        /* OptionLongChar */
   '%',                                /* OptionIntegerChar */
   '\0',                        /* OptionByteChar */
   '"',                                /* OptionQuoteChar */
   '\0',                        /* OptionCommentChar */
   ':',                                /* OptionStatementChar */
   '\0',                        /* OptionPrintChar */
   '\0',                        /* OptionInputChar */
   '\0',                        /* OptionImageChar */
   '(',                                /* OptionLparenChar */
   ')',                                /* OptionRparenChar */
   '\0',                        /* OptionFilenumChar */
   '\0',                        /* OptionAtChar */
   '#',                                /* OptionUsingDigit */
   ',',                                /* OptionUsingComma */
   '.',                                /* OptionUsingPeriod */
   '+',                                /* OptionUsingPlus */
   '-',                                /* OptionUsingMinus */
   '^',                                /* OptionUsingExrad */
   '$',                                /* OptionUsingDollar */
   '*',                                /* OptionUsingFiller */
   '_',                                /* OptionUsingLiteral */
   '!',                                /* OptionUsingFirst */
   '&',                                /* OptionUsingAll */
   '\\',                        /* OptionUsingLength */
   }
  ,
  {
   "XBASIC",                        /* Name */
   T79,                                /* OptionVersionBitmask */
   "T79",                        /* ID */
   "TSC XBASIC for 6800 FLEX",        /* Description */
   "Extended BASIC User's Manual",        /* Reference Title */
   "by Technical Systems Consultants",        /* Reference Author */
   "(c) 1979, Technical Systems Consultants",        /* Reference Copyright */
   "http://www.swtpc.com/mholley/Setton/",        /* Reference URL1 */
   "xbasic.pdf",                /* Reference URL2 */
   OPTION_BUGS_ON,                /* OptionFlags */
   0,                                /* OptionReclenInteger */
   0,                                /* OptionBaseInteger */
   "%d-%b-%y",                        /* OptionDateFormat */
   "%H:%M:%S",                        /* OptionTimeFormat */
   '$',                                /* OptionStringChar */
   '#',                                /* OptionDoubleChar */
   '!',                                /* OptionSingleChar */
   '\0',                                /* OptionCurrencyChar */
   '&',                                /* OptionLongChar */
   '%',                                /* OptionIntegerChar */
   '\0',                                /* OptionByteChar */
   '"',                                /* OptionQuoteChar */
   '\0',                        /* OptionCommentChar */
   ':',                                /* OptionStatementChar */
   '?',                                /* OptionPrintChar */
   '\0',                        /* OptionInputChar */
   '\0',                        /* OptionImageChar */
   '(',                                /* OptionLparenChar */
   ')',                                /* OptionRparenChar */
   '#',                                /* OptionFilenumChar */
   '\0',                        /* OptionAtChar */
   '#',                                /* OptionUsingDigit */
   ',',                                /* OptionUsingComma */
   '.',                                /* OptionUsingPeriod */
   '+',                                /* OptionUsingPlus */
   '-',                                /* OptionUsingMinus */
   '^',                                /* OptionUsingExrad */
   '$',                                /* OptionUsingDollar */
   '*',                                /* OptionUsingFiller */
   '_',                                /* OptionUsingLiteral */
   '!',                                /* OptionUsingFirst */
   '&',                                /* OptionUsingAll */
   '\\',                        /* OptionUsingLength */
   }
  ,
};


const size_t NUM_VERSIONS = sizeof (bwb_vertable) / sizeof (VersionType);


/* EOF */
