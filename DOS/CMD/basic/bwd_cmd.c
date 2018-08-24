/***************************************************************
  
        bwd_cmd.c       Command Table
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


/* COMMAND TABLE */

#include "bwbasic.h"

CommandType IntrinsicCommandTable[ /* NUM_COMMANDS */ ] =
{
{
  C_APPEND,  /* UniqueID */
  "APPEND filename$",  /* Syntax */
  "Merges the BASIC program in filename$ into the current BASIC "
  "program; lines in filename$ replace any matching lines in the "
  "current program.",  /* Description */
  "APPEND",  /* Name */
  B15 | HB2 | R86 | T79  /* OptionVersionBitmask */
},
{
  C_APPEND,  /* UniqueID */
  "APPEND # filenumber",  /* Syntax */
  "Positions filenumber at EOF and sets the file to writing; filenumber "
  "<= 0 is ignored.",  /* Description */
  "APPEND",  /* Name */
  D64 | G74 | G65 | G67  /* OptionVersionBitmask */
},
{
  C_AS,  /* UniqueID */
  "AS",  /* Syntax */
  "Syntax Error.",  /* Description */
  "AS",  /* Name */
  B15 | B93 | H14 | M80 | R86 | D71 | T80 | T79  /* OptionVersionBitmask */
},
{
  C_AUTO,  /* UniqueID */
  "AUTO [start [, increment]]",  /* Syntax */
  "Automatic line numbering for manual program entry.  If the line "
  "already exists, then an asterisk is displayed and pressing ENTER "
  "leaves the line as-is.  If the line does not exist, then an asterisk "
  "is not displayed and pressing ENTER terminates AUTO mode.  Regardless "
  "whether the line exists, entering the command MAN will terminate "
  "AUTO mode.  AUTO mode is also terminated by any ERROR or by pressing "
  "control-C.",  /* Description */
  "AUTO",  /* Name */
  B15 | HB1 | HB2 | M80 | D71 | T80  /* OptionVersionBitmask */
},
{
  C_BACKSPACE,  /* UniqueID */
  "BACKSPACE # X",  /* Syntax */
  "Points the file to the previous item.",  /* Description */
  "BACKSPACE",  /* Name */
  D64 | G74 | G65 | G67  /* OptionVersionBitmask */
},
{
  C_BREAK,  /* UniqueID */
  "BREAK line [, ...]]",  /* Syntax */
  "Diagnostic command to stop execution at the specified line(s). "
  " BREAK only applies to user-numbered lines.  For multi-statement "
  "lines, BREAK only applies to the first statement.  BREAK effectively "
  "inserts a hidden STOP command immediately after the line number. "
  " Once a BREAK occurrs on a specified line, it is automatically "
  "removed.  To remove all existing BREAKs, execute BREAK without "
  "any line numbers.",  /* Description */
  "BREAK",  /* Name */
  B15 | HB1 | HB2  /* OptionVersionBitmask */
},
{
  C_BUILD,  /* UniqueID */
  "BUILD [start [, increment]]",  /* Syntax */
  "Automatic line numbering for manual program entry.  If the line "
  "already exists, then an asterisk is displayed and pressing ENTER "
  "leaves the line as-is.  If the line does not exist, then an asterisk "
  "is not displayed and pressing ENTER terminates BUILD mode.  Regardless "
  "whether the line exists, entering the command MAN will terminate "
  "BUILD mode.  BUILD mode is also terminated by any ERROR or by "
  "pressing control-C.",  /* Description */
  "BUILD",  /* Name */
  H80  /* OptionVersionBitmask */
},
{
  C_BYE,  /* UniqueID */
  "BYE",  /* Syntax */
  "Exits to the operating system.",  /* Description */
  "BYE",  /* Name */
  B15 | D64 | G74 | H14 | HB2 | H80 | G65 | G67 | D73 | D70 | D71  /* OptionVersionBitmask */
},
{
  C_CALL,  /* UniqueID */
  "CALL subname( [parameter [, ...] ] )",  /* Syntax */
  "Calls a subroutine that was defined by SUB and END SUB.",  /* Description */
  "CALL",  /* Name */
  B15 | B93 | H14 | HB2  /* OptionVersionBitmask */
},
{
  C_CASE,  /* UniqueID */
  "CASE constant [TO constant]",  /* Syntax */
  "Introduces an element of a SELECT CASE statement.  Multiple tests "
  "must be seperated a comma.  For example:  CASE 1, 2 TO 3, IS "
  "> 4, IF < 5",  /* Description */
  "CASE",  /* Name */
  B15 | B93 | E86 | H14  /* OptionVersionBitmask */
},
{
  C_CASE,  /* UniqueID */
  "CASE IF operator constant",  /* Syntax */
  "Introduces an element of a SELECT CASE statement.  Multiple tests "
  "must be seperated a comma.  For example:  CASE 1, 2 TO 3, IS "
  "> 4, IF < 5",  /* Description */
  "CASE",  /* Name */
  B15 | B93 | E86 | H14  /* OptionVersionBitmask */
},
{
  C_CASE,  /* UniqueID */
  "CASE IS operator constant",  /* Syntax */
  "Introduces an element of a SELECT CASE statement.  Multiple tests "
  "must be seperated a comma.  For example:  CASE 1, 2 TO 3, IS "
  "> 4, IF < 5",  /* Description */
  "CASE",  /* Name */
  B15 | B93 | E86 | H14  /* OptionVersionBitmask */
},
{
  C_CASE_ELSE,  /* UniqueID */
  "CASE ELSE",  /* Syntax */
  "Introduces a default SELECT CASE element.",  /* Description */
  "CASE ELSE",  /* Name */
  B15 | B93 | E86 | H14  /* OptionVersionBitmask */
},
{
  C_CHAIN,  /* UniqueID */
  "CHAIN filename$ [, linenumber]",  /* Syntax */
  "Load and execute another BASIC program, without clearing common "
  "variables.  For System/370, the syntax is CHAIN filename$,parameter$.",  /* Description */
  "CHAIN",  /* Name */
  B15 | B93 | S70 | C77 | G74 | HB1 | HB2 | H80 | G65 | G67 | M80 | R86 | D71 | I70 | I73 | T80 | T79  /* OptionVersionBitmask */
},
{
  C_CHANGE,  /* UniqueID */
  "CHANGE A$ TO X",  /* Syntax */
  "Changes a string to a numeric array.",  /* Description */
  "CHANGE",  /* Name */
  B15 | D64 | G74 | HB2 | G65 | G67 | D71  /* OptionVersionBitmask */
},
{
  C_CHANGE,  /* UniqueID */
  "CHANGE X TO A$",  /* Syntax */
  "Changes a numeric array to a string.",  /* Description */
  "CHANGE",  /* Name */
  B15 | D64 | G74 | HB2 | G65 | G67 | D71  /* OptionVersionBitmask */
},
{
  C_CLEAR,  /* UniqueID */
  "CLEAR",  /* Syntax */
  "Sets all numeric variables to 0, and all string variables to "
  "empty strings.",  /* Description */
  "CLEAR",  /* Name */
  B15 | B93 | H14 | HB1 | HB2 | H80 | M80 | R86 | D71 | T80 | T79  /* OptionVersionBitmask */
},
{
  C_CLOAD,  /* UniqueID */
  "CLOAD [filename$]",  /* Syntax */
  "Loads an ASCII BASIC program into memory.",  /* Description */
  "CLOAD",  /* Name */
  B15 | HB1 | HB2 | M80 | D71 | T80  /* OptionVersionBitmask */
},
{
  C_CLOAD8,  /* UniqueID */
  "CLOAD* arrayname",  /* Syntax */
  "Loads a numeric array from a file saved using CSAVE*.",  /* Description */
  "CLOAD*",  /* Name */
  B15 | HB1 | HB2 | M80 | D71 | T80  /* OptionVersionBitmask */
},
{
  C_CLOSE,  /* UniqueID */
  "CLOSE",  /* Syntax */
  "Closes all files.",  /* Description */
  "CLOSE",  /* Name */
  S70 | I70 | I73  /* OptionVersionBitmask */
},
{
  C_CLOSE,  /* UniqueID */
  "CLOSE filename$ [, ...]",  /* Syntax */
  "Closes a file.",  /* Description */
  "CLOSE",  /* Name */
  S70 | I70 | I73  /* OptionVersionBitmask */
},
{
  C_CLOSE,  /* UniqueID */
  "CLOSE filenumber [, ...]",  /* Syntax */
  "Closes a file.",  /* Description */
  "CLOSE",  /* Name */
  S70 | I70 | I73  /* OptionVersionBitmask */
},
{
  C_CLR,  /* UniqueID */
  "CLR",  /* Syntax */
  "Sets all numeric variables to 0, and all string variables to "
  "empty strings.",  /* Description */
  "CLR",  /* Name */
  HB1 | HB2  /* OptionVersionBitmask */
},
{
  C_CMDS,  /* UniqueID */
  "CMDS",  /* Syntax */
  "Prints a list of all implemented BASIC commands.",  /* Description */
  "CMDS",  /* Name */
  B15 | B93  /* OptionVersionBitmask */
},
{
  C_COMMON,  /* UniqueID */
  "COMMON variable [, ...]",  /* Syntax */
  "Designates variables to be passed to a CHAINed program.",  /* Description */
  "COMMON",  /* Name */
  B15 | B93 | C77 | E86 | HB2 | M80 | D71 | T80  /* OptionVersionBitmask */
},
{
  C_CONSOLE,  /* UniqueID */
  "CONSOLE [WIDTH width]",  /* Syntax */
  "Directs the output of PRINT commands to the console (stdout), "
  "and optionally changes the console width.",  /* Description */
  "CONSOLE",  /* Name */
  C77  /* OptionVersionBitmask */
},
{
  C_CONST,  /* UniqueID */
  "CONST variable [, ...] = value",  /* Syntax */
  "Assigns the value to variable.  Any later assignment to the variable "
  "causus a VARIABLE NOT DECLARED error.",  /* Description */
  "CONST",  /* Name */
  B15  /* OptionVersionBitmask */
},
{
  C_CONT,  /* UniqueID */
  "CONT",  /* Syntax */
  "Continue a BASIC program after a STOP has been executed.  Program "
  "resumes at the line after the STOP.",  /* Description */
  "CONT",  /* Name */
  B15 | HB1 | HB2 | M80 | R86 | D71 | T80 | T79  /* OptionVersionBitmask */
},
{
  C_CONTINUE,  /* UniqueID */
  "CONTINUE",  /* Syntax */
  "Continue a BASIC program after a STOP has been executed.  Program "
  "resumes at the line after the STOP.",  /* Description */
  "CONTINUE",  /* Name */
  H80  /* OptionVersionBitmask */
},
{
  C_COPY,  /* UniqueID */
  "COPY source$ TO target$",  /* Syntax */
  "Copy an exisitng file.",  /* Description */
  "COPY",  /* Name */
  H14  /* OptionVersionBitmask */
},
{
  C_CREATE,  /* UniqueID */
  "CREATE filename$ [ RECL reclen ] AS filenum\n[ BUFF number ] [ "
  "RECS size ]",  /* Syntax */
  "Creates the file and opens it.  If the file already exists, then "
  "it is ereased.  If the file cannot be created, then an error "
  "occurs.",  /* Description */
  "CREATE",  /* Name */
  C77  /* OptionVersionBitmask */
},
{
  C_CSAVE,  /* UniqueID */
  "CSAVE [filename$]",  /* Syntax */
  "Saves the current program into the file filename$ in ASCII format.",  /* Description */
  "CSAVE",  /* Name */
  B15 | HB1 | HB2 | M80 | D71 | T80  /* OptionVersionBitmask */
},
{
  C_CSAVE8,  /* UniqueID */
  "CSAVE* ArrayName",  /* Syntax */
  "Saves a numeric array into a file for later loading by CLOAD*.",  /* Description */
  "CSAVE*",  /* Name */
  B15 | HB1 | HB2 | M80 | D71 | T80  /* OptionVersionBitmask */
},
{
  C_DATA,  /* UniqueID */
  "DATA constant [, ...]",  /* Syntax */
  "Stores numeric and string constants to be accessed by READ.",  /* Description */
  "DATA",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_DEC,  /* UniqueID */
  "DEC variable [,...]",  /* Syntax */
  "Decrement a numeric variable.",  /* Description */
  "DEC",  /* Name */
  H14  /* OptionVersionBitmask */
},
{
  C_DEF,  /* UniqueID */
  "DEF FNname[( arg [,...] )] = value",  /* Syntax */
  "Defines a single-line function.  Single-line functions require "
  "an equal sign.",  /* Description */
  "DEF",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_DEF,  /* UniqueID */
  "DEF FNname[( arg [,...] )]",  /* Syntax */
  "Defines a multiline function.  Multi-line DEF functions do not "
  "have an equal sign and must end with FNEND.",  /* Description */
  "DEF",  /* Name */
  B15 | S70 | G74 | HB1 | HB2 | H80 | G67 | D73 | D70 | D71 | I70 | I73 | T80  /* OptionVersionBitmask */
},
{
  C_DEFBYT,  /* UniqueID */
  "DEFBYT letter[-letter] [, ...]",  /* Syntax */
  "Declares variables with single-letter names as numeric variables.",  /* Description */
  "DEFBYT",  /* Name */
  B15  /* OptionVersionBitmask */
},
{
  C_DEFCUR,  /* UniqueID */
  "DEFCUR letter[-letter] [, ...]",  /* Syntax */
  "Declares variables with single-letter names as numeric variables.",  /* Description */
  "DEFCUR",  /* Name */
  B15  /* OptionVersionBitmask */
},
{
  C_DEFDBL,  /* UniqueID */
  "DEFDBL letter[-letter] [, ...]",  /* Syntax */
  "Declares variables with single-letter names as numeric variables.",  /* Description */
  "DEFDBL",  /* Name */
  B15 | B93 | H14 | HB1 | HB2 | M80 | D71 | T80  /* OptionVersionBitmask */
},
{
  C_DEFINT,  /* UniqueID */
  "DEFINT  letter[-letter] [, ...]",  /* Syntax */
  "Declares variables with single-letter names as numeric variables.",  /* Description */
  "DEFINT",  /* Name */
  B15 | B93 | H14 | HB1 | HB2 | M80 | D71 | T80  /* OptionVersionBitmask */
},
{
  C_DEFLNG,  /* UniqueID */
  "DEFLNG letter[-letter] [, ...]",  /* Syntax */
  "Declares variables with single-letter names as numeric variables.",  /* Description */
  "DEFLNG",  /* Name */
  B15  /* OptionVersionBitmask */
},
{
  C_DEFSNG,  /* UniqueID */
  "DEFSNG letter[-letter] [, ...]",  /* Syntax */
  "Declares variables with single-letter names as numeric variables.",  /* Description */
  "DEFSNG",  /* Name */
  B15 | B93 | HB1 | HB2 | M80 | D71 | T80  /* OptionVersionBitmask */
},
{
  C_DEFSTR,  /* UniqueID */
  "DEFSTR letter[-letter] [, ...]",  /* Syntax */
  "Declares variables with single-letter names as string variables.",  /* Description */
  "DEFSTR",  /* Name */
  B15 | B93 | H14 | HB1 | HB2 | M80 | D71 | T80  /* OptionVersionBitmask */
},
{
  C_DELETE,  /* UniqueID */
  "DELETE filenumber",  /* Syntax */
  "Closes and deletes the file OPEN as filenumber.",  /* Description */
  "DELETE",  /* Name */
  C77  /* OptionVersionBitmask */
},
{
  C_DELETE,  /* UniqueID */
  "DELETE line [- line]",  /* Syntax */
  "Deletes program lines indicated by the argument(s).  All program "
  "lines have a number, which is visible with the LIST command. "
  " If line numbers are not provided, they are assigned beginning "
  "with 1.  Deleting a non-existing line does not cause an error.",  /* Description */
  "DELETE",  /* Name */
  B15 | B93 | H14 | HB1 | HB2 | H80 | M80 | D73 | D70 | D71 | T80  /* OptionVersionBitmask */
},
{
  C_DELIMIT,  /* UniqueID */
  "DELIMIT # X, A$",  /* Syntax */
  "Sets the delimiter for READ # and WRITE #.",  /* Description */
  "DELIMIT",  /* Name */
  G74 | G65 | G67  /* OptionVersionBitmask */
},
{
  C_DIM,  /* UniqueID */
  "DIM [# filenum,] variable([ lower TO ] upper)",  /* Syntax */
  "Declares variables and specifies the dimensions of array variables. "
  " For array variables, if the lower bound is not provided, then "
  "the OPTION BASE value is used.  If filenum is provided, then "
  "the variable is virtual.",  /* Description */
  "DIM",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_DISPLAY,  /* UniqueID */
  "DISPLAY source$",  /* Syntax */
  "Display an exisitng text file.",  /* Description */
  "DISPLAY",  /* Name */
  H14  /* OptionVersionBitmask */
},
{
  C_DO,  /* UniqueID */
  "DO UNTIL value",  /* Syntax */
  "Top of a DO - LOOP structure.  Exits when value is non-zero.",  /* Description */
  "DO",  /* Name */
  B15 | B93 | E86 | H14  /* OptionVersionBitmask */
},
{
  C_DO,  /* UniqueID */
  "DO",  /* Syntax */
  "Top of a DO - LOOP structure.  If the loop is not terminated "
  "by EXIT DO or LOOP UNTIL or LOOP WHILE, then it will loop forever.",  /* Description */
  "DO",  /* Name */
  B15 | B93 | E86 | H14  /* OptionVersionBitmask */
},
{
  C_DO,  /* UniqueID */
  "DO WHILE value",  /* Syntax */
  "Top of a DO - LOOP structure.  Exits when value is zero.",  /* Description */
  "DO",  /* Name */
  B15 | B93 | E86 | H14  /* OptionVersionBitmask */
},
{
  C_DOS,  /* UniqueID */
  "DOS",  /* Syntax */
  "Exits to the operating system.",  /* Description */
  "DOS",  /* Name */
  R86  /* OptionVersionBitmask */
},
{
  C_DSP,  /* UniqueID */
  "DSP variable [, ...]]",  /* Syntax */
  "Diagnostic command to display the value every time the variable "
  "is assigned.  To remove all existing DSPs, execute DSP without "
  "any variables.",  /* Description */
  "DSP",  /* Name */
  B15 | HB1 | HB2  /* OptionVersionBitmask */
},
{
  C_EDIT,  /* UniqueID */
  "EDIT",  /* Syntax */
  "implementation defined.",  /* Description */
  "EDIT",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | HB1 | HB2 | H80 | G65 | G67 | M80 | R86 | D71 | I70 | I73 | T80 | T79  /* OptionVersionBitmask */
},
{
  C_ELSE,  /* UniqueID */
  "ELSE",  /* Syntax */
  "Introduces a default condition in a multi-line IF statement.",  /* Description */
  "ELSE",  /* Name */
  B15 | B93 | C77 | E86 | H14 | HB1 | HB2 | H80 | M80 | D73 | D70 | R86 | D71 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_ELSEIF,  /* UniqueID */
  "ELSEIF",  /* Syntax */
  "Introduces a secondary condition in a multi-line IF statement.",  /* Description */
  "ELSEIF",  /* Name */
  B15 | B93 | C77 | E86 | H14 | HB1 | HB2 | H80 | M80 | D73 | D70 | R86 | D71 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_END,  /* UniqueID */
  "END",  /* Syntax */
  "Terminates program execution. If the BASIC program was executed "
  "from the operating system level, then control returns to the "
  "operating system, oterwise control reuturns to the BASIC prompt.",  /* Description */
  "END",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_END_FUNCTION,  /* UniqueID */
  "END FUNCTION",  /* Syntax */
  "Specifies the last line of a multi-line FUNCTION definition.",  /* Description */
  "END FUNCTION",  /* Name */
  B15 | B93 | E86 | H14  /* OptionVersionBitmask */
},
{
  C_END_IF,  /* UniqueID */
  "END IF",  /* Syntax */
  "Specifies the last line of a multi-line IF definition.",  /* Description */
  "END IF",  /* Name */
  B15 | B93 | C77 | E86 | H14 | HB1 | HB2 | H80 | M80 | D73 | D70 | R86 | D71 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_END_SELECT,  /* UniqueID */
  "END SELECT",  /* Syntax */
  "Specifies the last line of a multi-line SELECT CASE definition.",  /* Description */
  "END SELECT",  /* Name */
  B15 | B93 | E86 | H14  /* OptionVersionBitmask */
},
{
  C_END_SUB,  /* UniqueID */
  "END SUB",  /* Syntax */
  "Specifies the last line of a multi-line SUB definition.",  /* Description */
  "END SUB",  /* Name */
  B15 | B93 | E86 | H14  /* OptionVersionBitmask */
},
{
  C_ERASE,  /* UniqueID */
  "ERASE variable [, ...]",  /* Syntax */
  "Eliminates arrayed variables from a program.",  /* Description */
  "ERASE",  /* Name */
  B15 | B93 | H14 | HB1 | HB2 | M80 | D71 | T80  /* OptionVersionBitmask */
},
{
  C_EXCHANGE,  /* UniqueID */
  "EXCHANGE variable, variable",  /* Syntax */
  "Swaps the values of two variables. Both variables must be of "
  "the same type.",  /* Description */
  "EXCHANGE",  /* Name */
  B15 | HB2  /* OptionVersionBitmask */
},
{
  C_EXIT,  /* UniqueID */
  "EXIT",  /* Syntax */
  "Syntax Error.",  /* Description */
  "EXIT",  /* Name */
  B15 | B93 | E86 | H14 | HB2  /* OptionVersionBitmask */
},
{
  C_EXIT_DO,  /* UniqueID */
  "EXIT DO",  /* Syntax */
  "Immediately exits the inner-most DO-LOOP strucure.",  /* Description */
  "EXIT DO",  /* Name */
  B15 | B93 | E86 | H14  /* OptionVersionBitmask */
},
{
  C_EXIT_FOR,  /* UniqueID */
  "EXIT FOR",  /* Syntax */
  "Immediately exits the inner-most FOR-NEXT strucure.",  /* Description */
  "EXIT FOR",  /* Name */
  B15 | B93 | E86 | H14 | HB2  /* OptionVersionBitmask */
},
{
  C_EXIT_FUNCTION,  /* UniqueID */
  "EXIT FUNCTION",  /* Syntax */
  "Immediately exits the inner-most multi-line FUNCTION strucure.",  /* Description */
  "EXIT FUNCTION",  /* Name */
  B15 | B93 | E86 | H14  /* OptionVersionBitmask */
},
{
  C_EXIT_REPEAT,  /* UniqueID */
  "EXIT REPEAT",  /* Syntax */
  "Exit a REPEAT - UNTIL structure.",  /* Description */
  "EXIT REPEAT",  /* Name */
  B15 | H14 | HB2  /* OptionVersionBitmask */
},
{
  C_EXIT_SUB,  /* UniqueID */
  "EXIT SUB",  /* Syntax */
  "Immediately exits the inner-most multi-line SUB strucure.",  /* Description */
  "EXIT SUB",  /* Name */
  B15 | B93 | E86 | H14  /* OptionVersionBitmask */
},
{
  C_EXIT_WHILE,  /* UniqueID */
  "EXIT WHILE",  /* Syntax */
  "Immediately exits the inner-most WHILE-END strucure.",  /* Description */
  "EXIT WHILE",  /* Name */
  B15 | B93 | E86  /* OptionVersionBitmask */
},
{
  C_FEND,  /* UniqueID */
  "FEND",  /* Syntax */
  "Specifies the last line of a multi-line DEF function.",  /* Description */
  "FEND",  /* Name */
  C77  /* OptionVersionBitmask */
},
{
  C_FIELD,  /* UniqueID */
  "FIELD [#] filenum, number AS variable$ [, ...]",  /* Syntax */
  "Assigns number bytes in the buffer of random file filenum to "
  "the variable variable$.  GET will automatically update the variable, "
  "and PUT will automatically use the variable.",  /* Description */
  "FIELD",  /* Name */
  B15 | B93 | H14 | M80 | R86 | D71 | T80 | T79  /* OptionVersionBitmask */
},
{
  C_FILE,  /* UniqueID */
  "FILE filename$ [( reclen )]",  /* Syntax */
  "Opens the file in READ mode.",  /* Description */
  "FILE",  /* Name */
  C77  /* OptionVersionBitmask */
},
{
  C_FILE,  /* UniqueID */
  "FILE # X, A$",  /* Syntax */
  "If A$ is \"*\" then closes file # X.  If A$ is not \"*\" then opens "
  "the file named A$ in READ mode.",  /* Description */
  "FILE",  /* Name */
  C77 | D64 | G74 | G65 | G67  /* OptionVersionBitmask */
},
{
  C_FILES,  /* UniqueID */
  "FILES A$[, ...]",  /* Syntax */
  "If A$ is not \"*\" opens the file named A$ in READ mode.  The first "
  "filename of the first FILES command is assocated with file number "
  "1.  Note that multiple FILES commands accumulate.",  /* Description */
  "FILES",  /* Name */
  D64 | G74 | G65 | G67  /* OptionVersionBitmask */
},
{
  C_FLEX,  /* UniqueID */
  "FLEX",  /* Syntax */
  "Exits to the operating system.",  /* Description */
  "FLEX",  /* Name */
  T79  /* OptionVersionBitmask */
},
{
  C_FNCS,  /* UniqueID */
  "FNCS",  /* Syntax */
  "Prints a list of all pre-defined BASIC functions.",  /* Description */
  "FNCS",  /* Name */
  B15 | B93  /* OptionVersionBitmask */
},
{
  C_FNEND,  /* UniqueID */
  "FNEND",  /* Syntax */
  "Specifies the last line of a multi-line DEF function.",  /* Description */
  "FNEND",  /* Name */
  B15 | S70 | G74 | HB1 | HB2 | H80 | G67 | D73 | D70 | D71 | I70 | I73 | T80  /* OptionVersionBitmask */
},
{
  C_FOR,  /* UniqueID */
  "FOR variable = start TO finish [STEP increment]",  /* Syntax */
  "Top of a FOR - NEXT structure.  The loop will continue a fixed "
  "number of times, which is determined by the values of start, "
  "finish, and increment.",  /* Description */
  "FOR",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_FUNCTION,  /* UniqueID */
  "FUNCTION [ ( parameter [, ...  ] ) ]",  /* Syntax */
  "Top line of a multi-line FUNCTION definition.  The variable names "
  "specified are local to the FUNCTION definition, and are initialized "
  "BYVAL when the function is invoked by another routine.",  /* Description */
  "FUNCTION",  /* Name */
  B15 | B93 | E86 | H14  /* OptionVersionBitmask */
},
{
  C_GET,  /* UniqueID */
  "GET # file_number , [ byte_number ] , scalar [,...]",  /* Syntax */
  "Gets one (or more) values from a BINARY file.",  /* Description */
  "GET",  /* Name */
  H14  /* OptionVersionBitmask */
},
{
  C_GET,  /* UniqueID */
  "GET # file_number [ , RECORD record_number ]",  /* Syntax */
  "Gets one (or more) values from a file.",  /* Description */
  "GET",  /* Name */
  R86 | D71 | T79  /* OptionVersionBitmask */
},
{
  C_GET,  /* UniqueID */
  "GET # file_number [ , RECORD record_number ]",  /* Syntax */
  "Gets one (or more) values from a file.",  /* Description */
  "GET",  /* Name */
  R86 | D71 | T79  /* OptionVersionBitmask */
},
{
  C_GET,  /* UniqueID */
  "GET filename$ , scalar [, ...]",  /* Syntax */
  "Gets one (or more) values from a file.",  /* Description */
  "GET",  /* Name */
  S70 | I70 | I73  /* OptionVersionBitmask */
},
{
  C_GET,  /* UniqueID */
  "GET # file_number [ , record_number ]",  /* Syntax */
  "Gets one (or more) values from a RANDOM file.",  /* Description */
  "GET",  /* Name */
  H14  /* OptionVersionBitmask */
},
{
  C_GO,  /* UniqueID */
  "GO",  /* Syntax */
  "Syntax Error.",  /* Description */
  "GO",  /* Name */
  B15 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_GO_SUB,  /* UniqueID */
  "GO SUB line",  /* Syntax */
  "Initiates a subroutine call to the line specified.  The subroutine "
  "must end with RETURN.  The line may be a number or a label.",  /* Description */
  "GO SUB",  /* Name */
  B15 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_GO_TO,  /* UniqueID */
  "GO TO line",  /* Syntax */
  "Branches program execution to the specified line.  The line may "
  "be a number or a label.",  /* Description */
  "GO TO",  /* Name */
  B15 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_GOODBYE,  /* UniqueID */
  "GOODBYE",  /* Syntax */
  "Exits to the operating system.",  /* Description */
  "GOODBYE",  /* Name */
  B15 | D64 | G74 | HB2 | G65 | G67 | D71  /* OptionVersionBitmask */
},
{
  C_GOSUB,  /* UniqueID */
  "GOSUB line",  /* Syntax */
  "Initiates a subroutine call to the line specified.  The subroutine "
  "must end with RETURN.  The line may be a number or a label.",  /* Description */
  "GOSUB",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_GOTO,  /* UniqueID */
  "GOTO line",  /* Syntax */
  "Branches program execution to the specified line.  The line may "
  "be a number or a label.",  /* Description */
  "GOTO",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_HELP,  /* UniqueID */
  "HELP name",  /* Syntax */
  "Provides help on the specified name which is a command name or "
  "function name.",  /* Description */
  "HELP",  /* Name */
  B15  /* OptionVersionBitmask */
},
{
  C_IF,  /* UniqueID */
  "IF value THEN line1 [ELSE line2]",  /* Syntax */
  "Single line standard IF command.  If the value is non-zero, then "
  "branh to line1.  If the value is zero and ELSE is provided, then "
  "branch to line2.  Otherwise continue to the next line.  LABELS "
  "are not allowed.",  /* Description */
  "IF",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_IF_END,  /* UniqueID */
  "IF END # filenum THEN line1 [ELSE line2]",  /* Syntax */
  "Single line standard IF command.  If the file is at EOF , then "
  "branch to line1.  If the file is not at EOF and ELSE is provided, "
  "then branch to line2.  Otherwise continue to the next line.  "
  "LABELS are not allowed.",  /* Description */
  "IF END",  /* Name */
  C77 | D64 | G74 | G65 | G67  /* OptionVersionBitmask */
},
{
  C_IF_MORE,  /* UniqueID */
  "IF MORE # filenum THEN line1 [ELSE line2]",  /* Syntax */
  "Single line standard IF command.  If the file is not at EOF , "
  "then branch to line1.  If the file is at EOF and ELSE is provided, "
  "then branch to line2.  Otherwise continue to the next line.  "
  "LABELS are not allowed.",  /* Description */
  "IF MORE",  /* Name */
  D64 | G74 | G65 | G67  /* OptionVersionBitmask */
},
{
  C_IF8THEN,  /* UniqueID */
  "IF value THEN",  /* Syntax */
  "Top of a multi-line IF - END IF structure.  If the value is non-zero, "
  "then the program lines upto the next ELSE or ELSE IF command "
  "are executed, otherwise the program branches to the next ELSE "
  "or ELSE IF command.",  /* Description */
  "IF*THEN",  /* Name */
  B15 | B93 | C77 | E86 | H14 | HB1 | HB2 | H80 | M80 | D73 | D70 | R86 | D71 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_IMAGE,  /* UniqueID */
  "IMAGE \"format string\" ",  /* Syntax */
  "Provides format string for PRINT USING linenum.",  /* Description */
  "IMAGE",  /* Name */
  S70 | H14 | HB1 | HB2 | I70 | I73  /* OptionVersionBitmask */
},
{
  C_INC,  /* UniqueID */
  "INC variable [,...]",  /* Syntax */
  "Increment a numeric variable.",  /* Description */
  "INC",  /* Name */
  H14  /* OptionVersionBitmask */
},
{
  C_INPUT,  /* UniqueID */
  "INPUT \"prompt string\" , variable [, ...]",  /* Syntax */
  "Reads input from the terminal after displaying a prompt.",  /* Description */
  "INPUT",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | T79  /* OptionVersionBitmask */
},
{
  C_INPUT,  /* UniqueID */
  "INPUT # filenum , variable [, ...]s",  /* Syntax */
  "Reads input from the file specified by filenum.",  /* Description */
  "INPUT",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | T79  /* OptionVersionBitmask */
},
{
  C_INPUT,  /* UniqueID */
  "INPUT variable [, ...]",  /* Syntax */
  "Reads input from the terminal.",  /* Description */
  "INPUT",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | T79  /* OptionVersionBitmask */
},
{
  C_INPUT_LINE,  /* UniqueID */
  "INPUT LINE variable$",  /* Syntax */
  "Reads entire line from the terminal into variable$.",  /* Description */
  "INPUT LINE",  /* Name */
  B15 | H80 | M80 | R86 | D71 | T80 | T79  /* OptionVersionBitmask */
},
{
  C_INPUT_LINE,  /* UniqueID */
  "INPUT LINE # filenum , variable$",  /* Syntax */
  "Reads entire line from a file into variable$.",  /* Description */
  "INPUT LINE",  /* Name */
  B15 | H80 | M80 | R86 | D71 | T80 | T79  /* OptionVersionBitmask */
},
{
  C_INPUT_LINE,  /* UniqueID */
  "INPUT LINE \"prompt string\" , variable$",  /* Syntax */
  "Reads entire line from the terminal into variable$ after displaying "
  "a prompt",  /* Description */
  "INPUT LINE",  /* Name */
  B15 | H80 | M80 | R86 | D71 | T80 | T79  /* OptionVersionBitmask */
},
{
  C_LET,  /* UniqueID */
  "[LET] variable [, ...] = value",  /* Syntax */
  "Assigns the value to the variable.  The LET keyword is optional.",  /* Description */
  "LET",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_LINE,  /* UniqueID */
  "LINE",  /* Syntax */
  "Syntax Error.",  /* Description */
  "LINE",  /* Name */
  B15 | B93 | H14 | HB2 | H80 | M80 | R86 | D71 | T80 | T79  /* OptionVersionBitmask */
},
{
  C_LINE_INPUT,  /* UniqueID */
  "LINE INPUT [[#] filenum,][\"prompt string\";] variable$",  /* Syntax */
  "Reads entire line from the keyboard or a file into variable$.",  /* Description */
  "LINE INPUT",  /* Name */
  B15 | B93 | H14 | HB2 | H80 | M80 | R86 | D71 | T80 | T79  /* OptionVersionBitmask */
},
{
  C_LIST,  /* UniqueID */
  "LIST line1 [- line2]",  /* Syntax */
  "Lists BASIC program lines from line1 to line2 to the console "
  "on stdout.",  /* Description */
  "LIST",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | T79  /* OptionVersionBitmask */
},
{
  C_LISTNH,  /* UniqueID */
  "LISTNH line1 [- line2]",  /* Syntax */
  "Lists BASIC program lines from line1 to line2 to the console "
  "on stdout.",  /* Description */
  "LISTNH",  /* Name */
  B15 | S70 | C77 | D64 | G74 | G65 | G67 | D71 | I70 | I73  /* OptionVersionBitmask */
},
{
  C_LLIST,  /* UniqueID */
  "LLIST line1 [- line2]",  /* Syntax */
  "Lists BASIC program lines from line1 to line2 to the printer "
  "on stderr.",  /* Description */
  "LLIST",  /* Name */
  B15 | C77 | H14 | HB2 | M80 | D71 | T80  /* OptionVersionBitmask */
},
{
  C_LOAD,  /* UniqueID */
  "LOAD [filename$]",  /* Syntax */
  "Loads an ASCII BASIC program into memory.",  /* Description */
  "LOAD",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | G65 | G67 | M80 | R86 | D71 | I70 | I73 | T80 | T79  /* OptionVersionBitmask */
},
{
  C_LOCAL,  /* UniqueID */
  "LOCAL [# filenum,] variable([ lower TO ] upper)",  /* Syntax */
  "Declares variables and specifies the dimensions of array variables. "
  " For array variables, if the lower bound is not provided, then "
  "the OPTION BASE value is used.  If filenum is provided, then "
  "the variable is virtual.  Only supported inside a FUNCTION or "
  "SUB.",  /* Description */
  "LOCAL",  /* Name */
  H14  /* OptionVersionBitmask */
},
{
  C_LOOP,  /* UniqueID */
  "LOOP UNTIL value",  /* Syntax */
  "Bottom of a DO - LOOP structure.  Exits when value is nonz-zero.",  /* Description */
  "LOOP",  /* Name */
  B15 | B93 | E86 | H14  /* OptionVersionBitmask */
},
{
  C_LOOP,  /* UniqueID */
  "LOOP WHILE value",  /* Syntax */
  "Bottom of a DO - LOOP structure.  Exits when value is zero.",  /* Description */
  "LOOP",  /* Name */
  B15 | B93 | E86 | H14  /* OptionVersionBitmask */
},
{
  C_LOOP,  /* UniqueID */
  "LOOP",  /* Syntax */
  "Bottom of a DO - LOOP structure.  If the loop is not terminated "
  "by EXIT DO or DO UNTIL or DO WHILE, then it will loop forever.",  /* Description */
  "LOOP",  /* Name */
  B15 | B93 | E86 | H14  /* OptionVersionBitmask */
},
{
  C_LPRINT,  /* UniqueID */
  "LPRINT [USING format-string$;] value ...",  /* Syntax */
  "Send output to the printer (stderr).",  /* Description */
  "LPRINT",  /* Name */
  B15 | S70 | E78 | E86 | H14 | HB2 | M80 | D71 | I70 | I73 | T80  /* OptionVersionBitmask */
},
{
  C_LPRINTER,  /* UniqueID */
  "LPRINTER [WIDTH width]",  /* Syntax */
  "Directs the output of PRINT commands to the printer (stderr), "
  "and optionally changes the printer width.",  /* Description */
  "LPRINTER",  /* Name */
  C77  /* OptionVersionBitmask */
},
{
  C_LPT,  /* UniqueID */
  "LPT [filename$]",  /* Syntax */
  "Directs the PRINT commands to write to the printer (stderr), "
  "or optionally to the specified file.",  /* Description */
  "LPT",  /* Name */
  D73  /* OptionVersionBitmask */
},
{
  C_LSET,  /* UniqueID */
  "LSET variable$ = value",  /* Syntax */
  "Left-aligns the value into variable$.  If the length of the value "
  "is too short, then it is padded on the right with spaces.  If "
  "the length of the value is too long, then it is truncated on "
  "the right.  This is only for use with variables assigned to a "
  "random access buffer with FIELD command.",  /* Description */
  "LSET",  /* Name */
  B15 | B93 | H14 | M80 | R86 | D71 | T80 | T79  /* OptionVersionBitmask */
},
{
  C_MAINTAINER,  /* UniqueID */
  "MAINTAINER",  /* Syntax */
  "This command is reserved for use by the Bywater BASIC maintainer. "
  " It is not for the BASIC programmer.",  /* Description */
  "MAINTAINER",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_MAINTAINER_CMDS,  /* UniqueID */
  "MAINTAINER CMDS",  /* Syntax */
  "Syntax Error.",  /* Description */
  "MAINTAINER CMDS",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_MAINTAINER_CMDS_HTML,  /* UniqueID */
  "MAINTAINER CMDS HTML",  /* Syntax */
  "Dump COMMAND vs VERSION as HTML table",  /* Description */
  "MAINTAINER CMDS HTML",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_MAINTAINER_CMDS_ID,  /* UniqueID */
  "MAINTAINER CMDS ID",  /* Syntax */
  "Dump COMMAND #define.",  /* Description */
  "MAINTAINER CMDS ID",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_MAINTAINER_CMDS_MANUAL,  /* UniqueID */
  "MAINTAINER CMDS MANUAL",  /* Syntax */
  "Dump COMMAND manual.",  /* Description */
  "MAINTAINER CMDS MANUAL",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_MAINTAINER_CMDS_SWITCH,  /* UniqueID */
  "MAINTAINER CMDS_SWITCH",  /* Syntax */
  "Dump COMMAND switch.",  /* Description */
  "MAINTAINER CMDS SWITCH",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_MAINTAINER_CMDS_TABLE,  /* UniqueID */
  "MAINTAINER CMDS TABLE",  /* Syntax */
  "Dump COMMAND table.",  /* Description */
  "MAINTAINER CMDS TABLE",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_MAINTAINER_DEBUG,  /* UniqueID */
  "MAINTAINER DEBUG",  /* Syntax */
  "Syntax Error.",  /* Description */
  "MAINTAINER DEBUG",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_MAINTAINER_DEBUG_OFF,  /* UniqueID */
  "MAINTAINER DEBUG OFF",  /* Syntax */
  "Disable degug tracing.",  /* Description */
  "MAINTAINER DEBUG OFF",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_MAINTAINER_DEBUG_ON,  /* UniqueID */
  "MAINTAINER DEBUG ON",  /* Syntax */
  "Enable degug tracing.",  /* Description */
  "MAINTAINER DEBUG ON",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_MAINTAINER_FNCS,  /* UniqueID */
  "MAINTAINER FNCS",  /* Syntax */
  "Syntax Error.",  /* Description */
  "MAINTAINER FNCS",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_MAINTAINER_FNCS_HTML,  /* UniqueID */
  "MAINTAINER FNCS HTML",  /* Syntax */
  "Dump FUNCTION vs VERSION as HTML table.",  /* Description */
  "MAINTAINER FNCS HTML",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_MAINTAINER_FNCS_ID,  /* UniqueID */
  "MAINTAINER FNCS ID",  /* Syntax */
  "Dump FUNCTION #define.",  /* Description */
  "MAINTAINER FNCS ID",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_MAINTAINER_FNCS_MANUAL,  /* UniqueID */
  "MAINTAINER FNCS MANUAL",  /* Syntax */
  "Dump FUNCTION manual.",  /* Description */
  "MAINTAINER FNCS MANUAL",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_MAINTAINER_FNCS_SWITCH,  /* UniqueID */
  "MAINTAINER FNCS SWITCH",  /* Syntax */
  "Dump FUNCTION switch.",  /* Description */
  "MAINTAINER FNCS SWITCH",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_MAINTAINER_FNCS_TABLE,  /* UniqueID */
  "MAINTAINER FNCS TABLE",  /* Syntax */
  "Dump FUNCTION table.",  /* Description */
  "MAINTAINER FNCS TABLE",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_MAINTAINER_MANUAL,  /* UniqueID */
  "MAINTAINER MANUAL",  /* Syntax */
  "Dump manual for the currently selected OPTION VERSION.",  /* Description */
  "MAINTAINER MANUAL",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_MAINTAINER_STACK,  /* UniqueID */
  "MAINTAINER STACK",  /* Syntax */
  "Dump the BASIC stack.",  /* Description */
  "MAINTAINER STACK",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_MARGIN,  /* UniqueID */
  "MARGIN # filenumber, width",  /* Syntax */
  "Sets the file margin for writing; filenumber <= 0 is ignored.",  /* Description */
  "MARGIN",  /* Name */
  B15 | D64 | G74 | HB2 | G65 | G67  /* OptionVersionBitmask */
},
{
  C_MAT,  /* UniqueID */
  "MAT arrayname = value",  /* Syntax */
  "Matrix operations:\nMAT A = CON\nMAT A = IDN\nMAT A = ZER\nMAT A "
  "= INV B\nMAT A = TRN B\nMAT A = (k) * B\nMAT A = B\nMAT A = B + C\nMAT "
  "A = B - C\nMAT A = B * C",  /* Description */
  "MAT",  /* Name */
  S70 | D64 | G74 | H14 | HB1 | HB2 | G65 | G67 | D71 | I70 | I73  /* OptionVersionBitmask */
},
{
  C_MAT_GET,  /* UniqueID */
  "MAT GET filename$, arrayname",  /* Syntax */
  "Matrix get.",  /* Description */
  "MAT GET",  /* Name */
  S70 | G74 | G65 | G67 | I70 | I73  /* OptionVersionBitmask */
},
{
  C_MAT_INPUT,  /* UniqueID */
  "MAT INPUT arrayname",  /* Syntax */
  "Matrix input.",  /* Description */
  "MAT INPUT",  /* Name */
  S70 | D64 | G74 | H14 | HB1 | HB2 | G65 | G67 | D71 | I70 | I73  /* OptionVersionBitmask */
},
{
  C_MAT_PRINT,  /* UniqueID */
  "MAT PRINT arrayname",  /* Syntax */
  "Matrix print.",  /* Description */
  "MAT PRINT",  /* Name */
  S70 | D64 | G74 | H14 | HB1 | HB2 | G65 | G67 | D71 | I70 | I73  /* OptionVersionBitmask */
},
{
  C_MAT_PUT,  /* UniqueID */
  "MAT PUT filename$, arrayname",  /* Syntax */
  "Matrix put.",  /* Description */
  "MAT PUT",  /* Name */
  S70 | G74 | G65 | G67 | I70 | I73  /* OptionVersionBitmask */
},
{
  C_MAT_READ,  /* UniqueID */
  "MAT READ arrayname",  /* Syntax */
  "Matrix read.",  /* Description */
  "MAT READ",  /* Name */
  S70 | D64 | G74 | H14 | HB1 | HB2 | G65 | G67 | D71 | I70 | I73  /* OptionVersionBitmask */
},
{
  C_MAT_WRITE,  /* UniqueID */
  "MAT WRITE arrayname",  /* Syntax */
  "Matrix write.",  /* Description */
  "MAT WRITE",  /* Name */
  D64 | G74 | H14 | HB1 | HB2 | G65 | G67  /* OptionVersionBitmask */
},
{
  C_MERGE,  /* UniqueID */
  "MERGE filename$",  /* Syntax */
  "Merges the BASIC program in filename$ into the current BASIC "
  "program.  Lines in filename$ replace any matching lines in the "
  "current program.",  /* Description */
  "MERGE",  /* Name */
  B15 | B93 | C77 | M80 | D71 | T80  /* OptionVersionBitmask */
},
{
  C_MID4,  /* UniqueID */
  "MID$( variable$, start [, count ] ) = value",  /* Syntax */
  "Replaces a subtring of variable$ with value.",  /* Description */
  "MID$",  /* Name */
  B15 | B93 | C77 | H14 | H80 | M80 | D71 | T80  /* OptionVersionBitmask */
},
{
  C_MON,  /* UniqueID */
  "MON",  /* Syntax */
  "Exits to the operating system.",  /* Description */
  "MON",  /* Name */
  R86  /* OptionVersionBitmask */
},
{
  C_NAME,  /* UniqueID */
  "NAME old$ AS new$",  /* Syntax */
  "Changes the name of an existing file.",  /* Description */
  "NAME",  /* Name */
  B15 | B93 | H14 | M80 | D71 | T80  /* OptionVersionBitmask */
},
{
  C_NEW,  /* UniqueID */
  "NEW",  /* Syntax */
  "Deletes the program in memory and clears all variables.",  /* Description */
  "NEW",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | T79  /* OptionVersionBitmask */
},
{
  C_NEXT,  /* UniqueID */
  "NEXT [variable]",  /* Syntax */
  "The bottom line of a FOR - NEXT structure.",  /* Description */
  "NEXT",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OF,  /* UniqueID */
  "OF",  /* Syntax */
  "Syntax Error.",  /* Description */
  "OF",  /* Name */
  B15 | S70 | C77 | D64 | E78 | E86 | G74 | HB1 | HB2 | G65 | G67 | M80 | R86 | D71 | I70 | I73 | T80 | T79  /* OptionVersionBitmask */
},
{
  C_OLD,  /* UniqueID */
  "OLD [filename$]",  /* Syntax */
  "Loads an ASCII BASIC program into memory.",  /* Description */
  "OLD",  /* Name */
  D64 | G74 | H80 | G65 | G67 | D73 | D70 | D71  /* OptionVersionBitmask */
},
{
  C_ON,  /* UniqueID */
  "ON value GOSUB line [, ...]",  /* Syntax */
  "Calls based on the rounded value.",  /* Description */
  "ON",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_ON,  /* UniqueID */
  "ON value GOTO line [, ...]",  /* Syntax */
  "Branches based on the rounded value.",  /* Description */
  "ON",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_ON_ERROR,  /* UniqueID */
  "ON ERROR",  /* Syntax */
  "Syntax Error.",  /* Description */
  "ON ERROR",  /* Name */
  B15 | B93 | H14 | HB1 | HB2 | M80 | R86 | D71 | T80 | T79  /* OptionVersionBitmask */
},
{
  C_ON_ERROR_GOSUB,  /* UniqueID */
  "ON ERROR GOSUB errline",  /* Syntax */
  "When a trappable error occurs, execute GOSUB errline.  The error "
  "handler must terminate with a RETURN command.  If the line number "
  "is 0 (zerp), then use the default error handler.  Valid when "
  "OPTION ERROR GOSUB.",  /* Description */
  "ON ERROR GOSUB",  /* Name */
  B15 | B93  /* OptionVersionBitmask */
},
{
  C_ON_ERROR_GOTO,  /* UniqueID */
  "ON ERROR GOTO errline",  /* Syntax */
  "When a trappable error occurs, execute GOTO errline.  The error "
  "handler must terminate with a RESUME command.  If the line number "
  "is 0 (zerp), then use the default error handler.  Valid when "
  "OPTION ERROR GOTO.",  /* Description */
  "ON ERROR GOTO",  /* Name */
  B15 | H14 | HB1 | HB2 | M80 | R86 | D71 | T80 | T79  /* OptionVersionBitmask */
},
{
  C_ON_ERROR_RESUME,  /* UniqueID */
  "ON ERROR RESUME",  /* Syntax */
  "Syntax Error.",  /* Description */
  "ON ERROR RESUME",  /* Name */
  B15 | H14 | M80 | R86 | D71 | T80 | T79  /* OptionVersionBitmask */
},
{
  C_ON_ERROR_RESUME_NEXT,  /* UniqueID */
  "ON ERROR RESUME NEXT",  /* Syntax */
  "When a trappable error occurs, execution continues with the next "
  "line.  Valid when OPTION ERROR GOTO.",  /* Description */
  "ON ERROR RESUME NEXT",  /* Name */
  B15 | H14 | M80 | R86 | D71 | T80 | T79  /* OptionVersionBitmask */
},
{
  C_ON_ERROR_RETURN,  /* UniqueID */
  "ON ERROR RETURN",  /* Syntax */
  "Syntax Error.",  /* Description */
  "ON ERROR RETURN",  /* Name */
  B15  /* OptionVersionBitmask */
},
{
  C_ON_ERROR_RETURN_NEXT,  /* UniqueID */
  "ON ERROR RETURN NEXT",  /* Syntax */
  "When a trappable error occurs, execution continues with the next "
  "line.  Valid when OPTION ERROR GOSUB.",  /* Description */
  "ON ERROR RETURN NEXT",  /* Name */
  B15  /* OptionVersionBitmask */
},
{
  C_ON_TIMER,  /* UniqueID */
  "ON TIMER count GOSUB line",  /* Syntax */
  "Specifies a line (or label) to gosub when count seconds have "
  "elaspsed after TIMER ON is executed.  The interrupt routine should "
  "end with a RETURN command.  Timer events only occur in running "
  "BASIC programs.  The resolution of the clock is implementation "
  "defined.",  /* Description */
  "ON TIMER",  /* Name */
  B15  /* OptionVersionBitmask */
},
{
  C_OPEN,  /* UniqueID */
  "OPEN NEW|OLD|VIRTUAL filename$\nAS filenumber",  /* Syntax */
  "Opens a file for use.",  /* Description */
  "OPEN",  /* Name */
  R86 | T79  /* OptionVersionBitmask */
},
{
  C_OPEN,  /* UniqueID */
  "OPEN filename$\nFOR INPUT|OUTPUT|APPEND|BINARY|RANDOM|VIRTUAL\nAS [#] "
  "fileenumber\n[LEN [=] record-length]",  /* Syntax */
  "Opens a file for use.\nRANDOM requires LEN.",  /* Description */
  "OPEN",  /* Name */
  B15 | B93 | H14 | HB2 | M80 | T80  /* OptionVersionBitmask */
},
{
  C_OPEN,  /* UniqueID */
  "OPEN filename$\nFOR INPUT|OUTPUT|APPEND|BINARY|RANDOM|VIRTUAL\nAS FILE "
  "filenumber\n[ ,RECORDSIZE ignored ]\n[ ,CLUSTERSIZE ignored ]\n[ "
  ",MODE ignored ]",  /* Syntax */
  "Opens a file for use.",  /* Description */
  "OPEN",  /* Name */
  D71  /* OptionVersionBitmask */
},
{
  C_OPEN,  /* UniqueID */
  "OPEN filename\nFOR READ|WRITE|VIRTUAL\nAS FILE filenumber",  /* Syntax */
  "Opens a file for use.",  /* Description */
  "OPEN",  /* Name */
  H80  /* OptionVersionBitmask */
},
{
  C_OPEN,  /* UniqueID */
  "OPEN filename$\n[ RECL reclen ]\nAS filenumber\n[ BUFF ignored ]\n[ "
  "RECS ignored ]",  /* Syntax */
  "Opens a file for use.",  /* Description */
  "OPEN",  /* Name */
  C77  /* OptionVersionBitmask */
},
{
  C_OPEN,  /* UniqueID */
  "OPEN filenumber,\nfilename$,\nINPUT|OUTPUT|APPEND|VIRTUAL",  /* Syntax */
  "Opens a file for use.",  /* Description */
  "OPEN",  /* Name */
  S70 | I70 | I73  /* OptionVersionBitmask */
},
{
  C_OPTION,  /* UniqueID */
  "OPTION",  /* Syntax */
  "Syntax Error.",  /* Description */
  "OPTION",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_ANGLE,  /* UniqueID */
  "OPTION ANGLE",  /* Syntax */
  "Syntax Error.",  /* Description */
  "OPTION ANGLE",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_ANGLE_DEGREES,  /* UniqueID */
  "OPTION ANGLE DEGREES",  /* Syntax */
  "Configures these math functions to accept and return angles in "
  "degrees:  ACOS, ACS, ANGLE, ARCSIN, ASIN, ASN, ARCTAN, ATN, ATAN, "
  "COS, COT, CSC, SEC, SIN and TAN.",  /* Description */
  "OPTION ANGLE DEGREES",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_ANGLE_GRADIANS,  /* UniqueID */
  "OPTION ANGLE GRADIANS",  /* Syntax */
  "Configures these math functions to accept and return angles in "
  "gradians:  ACOS, ANGLE, ASIN, ASN, ATN, ATAN, COS, COT, CSC, "
  "SEC, SIN and TAN.",  /* Description */
  "OPTION ANGLE GRADIANS",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_ANGLE_RADIANS,  /* UniqueID */
  "OPTION ANGLE RADIANS",  /* Syntax */
  "Configures these math functions to accept and return angles in "
  "radians:  ACOS, ANGLE, ASIN, ASN, ATN, ATAN, COS, COT, CSC, SEC, "
  "SIN and TAN.",  /* Description */
  "OPTION ANGLE RADIANS",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_ARITHMETIC,  /* UniqueID */
  "OPTION ARITHMETIC",  /* Syntax */
  "Syntax Error.",  /* Description */
  "OPTION ARITHMETIC",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_ARITHMETIC_DECIMAL,  /* UniqueID */
  "OPTION ARITHMETIC DECIMAL",  /* Syntax */
  "Currently has no effect.",  /* Description */
  "OPTION ARITHMETIC DECIMAL",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_ARITHMETIC_FIXED,  /* UniqueID */
  "OPTION ARITHMETIC FIXED",  /* Syntax */
  "Currently has no effect.",  /* Description */
  "OPTION ARITHMETIC FIXED",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_ARITHMETIC_NATIVE,  /* UniqueID */
  "OPTION ARITHMETIC NATIVE",  /* Syntax */
  "Currently has no effect.",  /* Description */
  "OPTION ARITHMETIC NATIVE",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_BASE,  /* UniqueID */
  "OPTION BASE integer",  /* Syntax */
  "Sets the default lowest array subscript.",  /* Description */
  "OPTION BASE",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_BUGS,  /* UniqueID */
  "OPTION BUGS",  /* Syntax */
  "Syntax Error.",  /* Description */
  "OPTION BUGS",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_BUGS_BOOLEAN,  /* UniqueID */
  "OPTION BUGS BOOLEAN",  /* Syntax */
  "Boolean results are 1 or 0 instead of bitwise.",  /* Description */
  "OPTION BUGS BOOLEAN",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_BUGS_OFF,  /* UniqueID */
  "OPTION BUGS OFF",  /* Syntax */
  "Disables bugs commonly found in many BASIC dialects.",  /* Description */
  "OPTION BUGS OFF",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_BUGS_ON,  /* UniqueID */
  "OPTION BUGS ON",  /* Syntax */
  "Enables bugs commonly found in many BASIC dialects.",  /* Description */
  "OPTION BUGS ON",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_COMPARE,  /* UniqueID */
  "OPTION COMPARE",  /* Syntax */
  "Syntax Error.",  /* Description */
  "OPTION COMPARE",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_COMPARE_BINARY,  /* UniqueID */
  "OPTION COMPARE BINARY",  /* Syntax */
  "Causes string comparisons to be case-sensitive.",  /* Description */
  "OPTION COMPARE BINARY",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_COMPARE_DATABASE,  /* UniqueID */
  "OPTION COMPARE DATABASE",  /* Syntax */
  "Causes string comparisons to be case-insensitive.",  /* Description */
  "OPTION COMPARE DATABASE",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_COMPARE_TEXT,  /* UniqueID */
  "OPTION COMPARE TEXT",  /* Syntax */
  "Causes string comparisons to be case-insensitive.",  /* Description */
  "OPTION COMPARE TEXT",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_COVERAGE,  /* UniqueID */
  "OPTION COVERAGE",  /* Syntax */
  "Syntax Error.",  /* Description */
  "OPTION COVERAGE",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_COVERAGE_OFF,  /* UniqueID */
  "OPTION COVERAGE OFF",  /* Syntax */
  "Disables BASIC code coverage recording, displayed using the LIST "
  "command.",  /* Description */
  "OPTION COVERAGE OFF",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_COVERAGE_ON,  /* UniqueID */
  "OPTION COVERAGE ON",  /* Syntax */
  "Enables BASIC code coverage recording, displayed using the LIST "
  "command.",  /* Description */
  "OPTION COVERAGE ON",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_DATE,  /* UniqueID */
  "OPTION DATE format$",  /* Syntax */
  "Sets the date format string used by C strftime() for DATE$.",  /* Description */
  "OPTION DATE",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_DIGITS,  /* UniqueID */
  "OPTION DIGITS integer",  /* Syntax */
  "Sets the number of significant digits for PRINT.  Setting the "
  "value to zero restores the default.",  /* Description */
  "OPTION DIGITS",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_DISABLE,  /* UniqueID */
  "OPTION DISABLE",  /* Syntax */
  "Syntax Error.",  /* Description */
  "OPTION DISABLE",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_DISABLE_COMMAND,  /* UniqueID */
  "OPTION DISABLE COMMAND name$",  /* Syntax */
  "Disables the specified BASIC command.",  /* Description */
  "OPTION DISABLE COMMAND",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_DISABLE_FUNCTION,  /* UniqueID */
  "OPTION DISABLE FUNCTION name$",  /* Syntax */
  "Disables the specified BASIC function.",  /* Description */
  "OPTION DISABLE FUNCTION",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_DISABLE_OPERATOR,  /* UniqueID */
  "OPTION DISABLE OPERATOR name$",  /* Syntax */
  "Disables the specified BASIC operator.",  /* Description */
  "OPTION DISABLE OPERATOR",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_EDIT,  /* UniqueID */
  "OPTION EDIT string$",  /* Syntax */
  "Sets the program name used by the EDIT command.",  /* Description */
  "OPTION EDIT",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_ENABLE,  /* UniqueID */
  "OPTION ENABLE",  /* Syntax */
  "Syntax Error.",  /* Description */
  "OPTION ENABLE",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_ENABLE_COMMAND,  /* UniqueID */
  "OPTION ENABLE COMMAND name$",  /* Syntax */
  "Enables the specified BASIC command.",  /* Description */
  "OPTION ENABLE COMMAND",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_ENABLE_FUNCTION,  /* UniqueID */
  "OPTION ENABLE FUNCTION name$",  /* Syntax */
  "Enables the specified BASIC function.",  /* Description */
  "OPTION ENABLE FUNCTION",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_ENABLE_OPERATOR,  /* UniqueID */
  "OPTION ENABLE OPERATOR name$",  /* Syntax */
  "Enables the specified BASIC operator.",  /* Description */
  "OPTION ENABLE OPERATOR",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_ERROR,  /* UniqueID */
  "OPTION ERROR",  /* Syntax */
  "Syntax Error.",  /* Description */
  "OPTION ERROR",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_ERROR_GOSUB,  /* UniqueID */
  "OPTION ERROR GOSUB",  /* Syntax */
  "When an error occurs, GOSUB to the error handler.  The error "
  "handler exits with RETURN.",  /* Description */
  "OPTION ERROR GOSUB",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_ERROR_GOTO,  /* UniqueID */
  "OPTION ERROR GOTO",  /* Syntax */
  "When an error occurs, GOTO to the error handler.  The error handler "
  "exits with RESUME.",  /* Description */
  "OPTION ERROR GOTO",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_EXPLICIT,  /* UniqueID */
  "OPTION EXPLICIT",  /* Syntax */
  "All variables must be declared using DIM.",  /* Description */
  "OPTION EXPLICIT",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_EXTENSION,  /* UniqueID */
  "OPTION EXTENSION string$",  /* Syntax */
  "Sets the BASIC filename extension, commonly \".bas\".",  /* Description */
  "OPTION EXTENSION",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_FILES,  /* UniqueID */
  "OPTION FILES string$",  /* Syntax */
  "Sets the program name used by the FILES command.",  /* Description */
  "OPTION FILES",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_IMPLICIT,  /* UniqueID */
  "OPTION IMPLICIT",  /* Syntax */
  "Variables need not be declared using DIM, provided arrays have "
  "no more that 10 elements.  This is the opposite of OPTION EXPLICIT, "
  "and is the default for all versions of BASIC.",  /* Description */
  "OPTION IMPLICIT",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_INDENT,  /* UniqueID */
  "OPTION INDENT integer",  /* Syntax */
  "Sets indention level for LIST.  Zero means no indention.  Default "
  "is 2.",  /* Description */
  "OPTION INDENT",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_LABELS,  /* UniqueID */
  "OPTION LABELS",  /* Syntax */
  "Syntax Error.",  /* Description */
  "OPTION LABELS",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_LABELS_OFF,  /* UniqueID */
  "OPTION LABELS OFF",  /* Syntax */
  "Disables text labels.",  /* Description */
  "OPTION LABELS OFF",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_LABELS_ON,  /* UniqueID */
  "OPTION LABELS ON",  /* Syntax */
  "Enables text labels.",  /* Description */
  "OPTION LABELS ON",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_PROMPT,  /* UniqueID */
  "OPTION PROMPT string$",  /* Syntax */
  "Sets the BASIC prompt.",  /* Description */
  "OPTION PROMPT",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_PUNCT,  /* UniqueID */
  "OPTION PUNCT",  /* Syntax */
  "Syntax Error.",  /* Description */
  "OPTION PUNCT",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_PUNCT_AT,  /* UniqueID */
  "OPTION PUNCT AT char$",  /* Syntax */
  "Sets the PRINT AT character, commonly \"@\".",  /* Description */
  "OPTION PUNCT AT",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_PUNCT_BYTE,  /* UniqueID */
  "OPTION PUNCT BYTE char$",  /* Syntax */
  "Sets the suffix character that indicates a variable is of type "
  "BYTE, commonly \"~\".",  /* Description */
  "OPTION PUNCT BYTE",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_PUNCT_COMMENT,  /* UniqueID */
  "OPTION PUNCT COMMENT char$",  /* Syntax */
  "Sets the shortcut COMMENT character.",  /* Description */
  "OPTION PUNCT COMMENT",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_PUNCT_CURRENCY,  /* UniqueID */
  "OPTION PUNCT CURRENCY char$",  /* Syntax */
  "Sets the suffix character that indicates a variable is of type "
  "CURRENCY, commonly \"@\".",  /* Description */
  "OPTION PUNCT CURRENCY",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_PUNCT_DOUBLE,  /* UniqueID */
  "OPTION PUNCT DOUBLE char$",  /* Syntax */
  "Sets the suffix character that indicates a variable is of type "
  "DOUBLE, commonly \"#\".",  /* Description */
  "OPTION PUNCT DOUBLE",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_PUNCT_FILENUM,  /* UniqueID */
  "OPTION PUNCT FILENUM char$",  /* Syntax */
  "Sets the FILE NUMBER prefix character, commonly \"#\".",  /* Description */
  "OPTION PUNCT FILENUM",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_PUNCT_IMAGE,  /* UniqueID */
  "OPTION PUNCT IMAGE char$",  /* Syntax */
  "Sets the shortcut IMAGE character, commonly \":\".",  /* Description */
  "OPTION PUNCT IMAGE",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_PUNCT_INPUT,  /* UniqueID */
  "OPTION PUNCT INPUT char$",  /* Syntax */
  "Sets the shortcut INPUT character, commonly \"!\".",  /* Description */
  "OPTION PUNCT INPUT",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_PUNCT_INTEGER,  /* UniqueID */
  "OPTION PUNCT INTEGER char$",  /* Syntax */
  "Sets the suffix character that indicates a variable is of type "
  "INTEGER, commonly \"%\".",  /* Description */
  "OPTION PUNCT INTEGER",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_PUNCT_LONG,  /* UniqueID */
  "OPTION PUNCT LONG char$",  /* Syntax */
  "Sets the suffix character that indicates a variable is of type "
  "LONG, commonly \"&\".",  /* Description */
  "OPTION PUNCT LONG",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_PUNCT_LPAREN,  /* UniqueID */
  "OPTION PUNCT LPAREN char$",  /* Syntax */
  "Sets the LEFT PARENTHESIS character, commonly \"(\".",  /* Description */
  "OPTION PUNCT LPAREN",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_PUNCT_PRINT,  /* UniqueID */
  "OPTION PUNCT_PRINT char$",  /* Syntax */
  "Sets the shortcut PRINT character, commonly \"?\".",  /* Description */
  "OPTION PUNCT PRINT",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_PUNCT_QUOTE,  /* UniqueID */
  "OPTION PUNCT QUOTE char$",  /* Syntax */
  "Sets the QUOTE character, commonly \"\"\"",  /* Description */
  "OPTION PUNCT QUOTE",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_PUNCT_RPAREN,  /* UniqueID */
  "OPTION PUNCT RPAREN char$",  /* Syntax */
  "Sets the RIGHT PARENTHESIS character, commonly \")\".",  /* Description */
  "OPTION PUNCT RPAREN",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_PUNCT_SINGLE,  /* UniqueID */
  "OPTION PUNCT SINGLE char$",  /* Syntax */
  "Sets the suffix character that indicates a variable is of type "
  "SINGLE, commonly \"!\".",  /* Description */
  "OPTION PUNCT SINGLE",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_PUNCT_STATEMENT,  /* UniqueID */
  "OPTION PUNCT STATEMENT char$",  /* Syntax */
  "Sets the statement seperator character, commonly \":\".",  /* Description */
  "OPTION PUNCT STATEMENT",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_PUNCT_STRING,  /* UniqueID */
  "OPTION PUNCT STRING char$",  /* Syntax */
  "Sets the suffix character that indicates a variable is of type "
  "STRING, commonly \"$\".",  /* Description */
  "OPTION PUNCT STRING",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_RECLEN,  /* UniqueID */
  "OPTION RECLEN integer",  /* Syntax */
  "Sets the default RANDOM record length.",  /* Description */
  "OPTION RECLEN",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_RENUM,  /* UniqueID */
  "OPTION RENUM string$",  /* Syntax */
  "Sets the program name used by the RENUM command.",  /* Description */
  "OPTION RENUM",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_ROUND,  /* UniqueID */
  "OPTION ROUND",  /* Syntax */
  "Syntax Error.",  /* Description */
  "OPTION ROUND",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_ROUND_BANK,  /* UniqueID */
  "OPTION ROUND BANK",  /* Syntax */
  "Round using the Banker rule.",  /* Description */
  "OPTION ROUND BANK",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_ROUND_MATH,  /* UniqueID */
  "OPTION ROUND MATH",  /* Syntax */
  "Round using mathematical rules.",  /* Description */
  "OPTION ROUND MATH",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_ROUND_TRUNCATE,  /* UniqueID */
  "OPTION ROUND TRUNCATE",  /* Syntax */
  "Round using truncation.",  /* Description */
  "OPTION ROUND TRUNCATE",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_SCALE,  /* UniqueID */
  "OPTION SCALE integer",  /* Syntax */
  "Sets the number of digits to round after the decimal point for "
  "PRINT.  Setting the value to zero disables rounding.",  /* Description */
  "OPTION SCALE",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_SLEEP,  /* UniqueID */
  "OPTION SLEEP double",  /* Syntax */
  "Sets multiplier for SLEEP and WAIT.  Zero means no waiting.  "
  "Default is 1.",  /* Description */
  "OPTION SLEEP",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_STDERR,  /* UniqueID */
  "OPTION STDERR filename$",  /* Syntax */
  "Sets the file used for STDERR, which is used by LPRINT commands.",  /* Description */
  "OPTION STDERR",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_STDIN,  /* UniqueID */
  "OPTION STDIN filename$",  /* Syntax */
  "Sets the file used for STDIN, which is used by INPUT commands.",  /* Description */
  "OPTION STDIN",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_STDOUT,  /* UniqueID */
  "OPTION STDOUT filename$",  /* Syntax */
  "Sets the file used for STDOUT, which is used by PRINT commands.",  /* Description */
  "OPTION STDOUT",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_STRICT,  /* UniqueID */
  "OPTION STRICT",  /* Syntax */
  "Syntax Error.",  /* Description */
  "OPTION STRICT",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_STRICT_OFF,  /* UniqueID */
  "OPTION STRICT OFF",  /* Syntax */
  "Disables checking for implicit array creation without using the "
  "DIM command.",  /* Description */
  "OPTION STRICT OFF",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_STRICT_ON,  /* UniqueID */
  "OPTION STRICT ON",  /* Syntax */
  "Enables checking for implicit array creation without using the "
  "DIM command.",  /* Description */
  "OPTION STRICT ON",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_TERMINAL,  /* UniqueID */
  "OPTION TERMINAL",  /* Syntax */
  "Syntax Error.",  /* Description */
  "OPTION TERMINAL",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_TERMINAL_ADM,  /* UniqueID */
  "OPTION TERMINAL ADM",  /* Syntax */
  "Enables ADM-3A terminal control codes for CLS, COLOR, and LOCATE.",  /* Description */
  "OPTION TERMINAL ADM",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_TERMINAL_ANSI,  /* UniqueID */
  "OPTION TERMINAL ANSI",  /* Syntax */
  "Enables ANSI terminal control codes for CLS, COLOR, and LOCATE.",  /* Description */
  "OPTION TERMINAL ANSI",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_TERMINAL_NONE,  /* UniqueID */
  "OPTION TERMINAL NONE",  /* Syntax */
  "Disables terminal control codes for CLS, COLOR, and LOCATE.",  /* Description */
  "OPTION TERMINAL NONE",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_TIME,  /* UniqueID */
  "OPTION TIME format$",  /* Syntax */
  "Sets the time format string used by C strftime() for TIME$.",  /* Description */
  "OPTION TIME",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_TRACE,  /* UniqueID */
  "OPTION TRACE",  /* Syntax */
  "Syntax Error.",  /* Description */
  "OPTION TRACE",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_TRACE_OFF,  /* UniqueID */
  "OPTION TRACE OFF",  /* Syntax */
  "Disables displaying a stack trace when an ERROR occurs.",  /* Description */
  "OPTION TRACE OFF",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_TRACE_ON,  /* UniqueID */
  "OPTION TRACE ON",  /* Syntax */
  "Enables displaying a stack trace when an ERROR occurs.",  /* Description */
  "OPTION TRACE ON",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_USING,  /* UniqueID */
  "OPTION USING",  /* Syntax */
  "Syntax Error.",  /* Description */
  "OPTION USING",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_USING_ALL,  /* UniqueID */
  "OPTION USING ALL char$",  /* Syntax */
  "Specifies the magic ALL character for the PRINT USING command. "
  " A common value is \"&\".",  /* Description */
  "OPTION USING ALL",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_USING_COMMA,  /* UniqueID */
  "OPTION USING COMMA char$",  /* Syntax */
  "Specifies the magic COMMA character for the PRINT USING command. "
  " A common value is \",\".",  /* Description */
  "OPTION USING COMMA",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_USING_DIGIT,  /* UniqueID */
  "OPTION USING DIGIT char$",  /* Syntax */
  "Specifies the magic DIGIT character for the PRINT USING command. "
  " A common value is \"#\".",  /* Description */
  "OPTION USING DIGIT",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_USING_DOLLAR,  /* UniqueID */
  "OPTION USING DOLLAR char$",  /* Syntax */
  "Specifies the magic DOLLAR character for the PRINT USING command. "
  " A common value is \"$\".",  /* Description */
  "OPTION USING DOLLAR",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_USING_EXRAD,  /* UniqueID */
  "OPTION USING EXRAD char$",  /* Syntax */
  "Specifies the magic EXRAD character for the PRINT USING command. "
  " A common value is \"^\".",  /* Description */
  "OPTION USING EXRAD",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_USING_FILLER,  /* UniqueID */
  "OPTION USING FILLER char$",  /* Syntax */
  "Specifies the magic FILLER character for the PRINT USING command. "
  " A common value is \"*\".",  /* Description */
  "OPTION USING FILLER",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_USING_FIRST,  /* UniqueID */
  "OPTION USING FIRST char$",  /* Syntax */
  "Specifies the magic FIRST character for the PRINT USING command. "
  " A common value is \"!\".",  /* Description */
  "OPTION USING FIRST",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_USING_LENGTH,  /* UniqueID */
  "OPTION USING LENGTH char$",  /* Syntax */
  "Specifies the magic LENGTH character for the PRINT USING command. "
  " A common value is \"\\\".",  /* Description */
  "OPTION USING LENGTH",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_USING_LITERAL,  /* UniqueID */
  "OPTION USING LITERAL char$",  /* Syntax */
  "Specifies the magic LITERAL character for the PRINT USING command. "
  " A common value is \"_\".",  /* Description */
  "OPTION USING LITERAL",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_USING_MINUS,  /* UniqueID */
  "OPTION USING MINUS char$",  /* Syntax */
  "Specifies the magic MINUS character for the PRINT USING command. "
  " A common value is \"-\".",  /* Description */
  "OPTION USING MINUS",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_USING_PERIOD,  /* UniqueID */
  "OPTION USING PERIOD char$",  /* Syntax */
  "Specifies the magic PERIOD character for the PRINT USING command. "
  " A common value is \".\".",  /* Description */
  "OPTION USING PERIOD",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_USING_PLUS,  /* UniqueID */
  "OPTION USING PLUS char$",  /* Syntax */
  "Specifies the magic PLUS character for the PRINT USING command. "
  " A common value is \"+\".",  /* Description */
  "OPTION USING PLUS",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_VERSION,  /* UniqueID */
  "OPTION VERSION version$",  /* Syntax */
  "Selects a specific BASIC version, which is a "
  "combination of OPTION settings, commands, "
  "functions and operators.  If no version is specified, displays "
  "a list of the available versions.",  /* Description */
  "OPTION VERSION",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_OPTION_ZONE,  /* UniqueID */
  "OPTION ZONE integer",  /* Syntax */
  "Sets the PRINT zone width.  Setting the value to zero restores "
  "the default.",  /* Description */
  "OPTION ZONE",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_PAUSE,  /* UniqueID */
  "PAUSE [comment]",  /* Syntax */
  "Pauses processing until the ENTER key is presseed.",  /* Description */
  "PAUSE",  /* Name */
  S70 | I70 | I73  /* OptionVersionBitmask */
},
{
  C_PDEL,  /* UniqueID */
  "PDEL line [- line]",  /* Syntax */
  "Deletes program lines indicated by the argument(s).  All program "
  "lines have a number, which is visible with the LIST command. "
  " If line numbers are not provided, they are assigned beginning "
  "with 1.  Deleting a non-existing line does not cause an error. "
  "",  /* Description */
  "PDEL",  /* Name */
  R86 | T79  /* OptionVersionBitmask */
},
{
  C_POP,  /* UniqueID */
  "POP",  /* Syntax */
  "Pops one GOSUB from the return stack.",  /* Description */
  "POP",  /* Name */
  B15 | HB2  /* OptionVersionBitmask */
},
{
  C_PRINT,  /* UniqueID */
  "PRINT # filenum , [USING format$;] value ...",  /* Syntax */
  "Sends output to a file.",  /* Description */
  "PRINT",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | T79  /* OptionVersionBitmask */
},
{
  C_PRINT,  /* UniqueID */
  "PRINT [USING format$;] value ...",  /* Syntax */
  "Sends output to the screen.",  /* Description */
  "PRINT",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_PTP,  /* UniqueID */
  "PTP [filename$]",  /* Syntax */
  "Directs the PRINT commands to write to the paper tape punch file "
  "(\"PTP\"), or optionally to the specified file.",  /* Description */
  "PTP",  /* Name */
  D73  /* OptionVersionBitmask */
},
{
  C_PTR,  /* UniqueID */
  "PTR [filename$]",  /* Syntax */
  "Directs the INPUT commands to read from the paper tape reader "
  "file (\"PTR\"), or optionally from the specified file.",  /* Description */
  "PTR",  /* Name */
  D73  /* OptionVersionBitmask */
},
{
  C_PUT,  /* UniqueID */
  "PUT # file_number , [ byte_number ] , scalar [,...]",  /* Syntax */
  "Puts one (or more) values into a BINARY file.",  /* Description */
  "PUT",  /* Name */
  H14  /* OptionVersionBitmask */
},
{
  C_PUT,  /* UniqueID */
  "PUT filename$ , value [, ...]",  /* Syntax */
  "Puts one (or more) values into a file.",  /* Description */
  "PUT",  /* Name */
  S70 | I70 | I73  /* OptionVersionBitmask */
},
{
  C_PUT,  /* UniqueID */
  "PUT # file_number [ , RECORD record_number ]",  /* Syntax */
  "Puts one (or more) values into a file.",  /* Description */
  "PUT",  /* Name */
  R86 | D71 | T79  /* OptionVersionBitmask */
},
{
  C_PUT,  /* UniqueID */
  "PUT # file_number [ , record_number ]",  /* Syntax */
  "Puts one (or more) values into a RANDOM file.",  /* Description */
  "PUT",  /* Name */
  H14  /* OptionVersionBitmask */
},
{
  C_QUIT,  /* UniqueID */
  "QUIT",  /* Syntax */
  "Exits to the operating system.",  /* Description */
  "QUIT",  /* Name */
  B15 | S70 | C77 | D64 | E78 | E86 | G74 | G65 | G67 | D71 | I70 | I73  /* OptionVersionBitmask */
},
{
  C_READ,  /* UniqueID */
  "READ variable [, ...]",  /* Syntax */
  "Reads values from DATA statements.",  /* Description */
  "READ",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_RECALL,  /* UniqueID */
  "RECALL ArrayName",  /* Syntax */
  "Loads a numeric array from a file saved using STORE.",  /* Description */
  "RECALL",  /* Name */
  B15 | HB2  /* OptionVersionBitmask */
},
{
  C_REM,  /* UniqueID */
  "REM ...",  /* Syntax */
  "Remark.",  /* Description */
  "REM",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_RENAME,  /* UniqueID */
  "RENAME from$ TO to$",  /* Syntax */
  "Rename a file.",  /* Description */
  "RENAME",  /* Name */
  H14  /* OptionVersionBitmask */
},
{
  C_RENAME,  /* UniqueID */
  "RENAME [filename$]",  /* Syntax */
  "Changes the file name which will be used by SAVE.  Does not save "
  "the file.",  /* Description */
  "RENAME",  /* Name */
  D64 | G74 | G65 | G67 | D71  /* OptionVersionBitmask */
},
{
  C_RENUM,  /* UniqueID */
  "RENUM",  /* Syntax */
  "Implementation defined.",  /* Description */
  "RENUM",  /* Name */
  B15 | B93 | H14 | HB1 | HB2 | M80 | R86 | D71 | T80 | T79  /* OptionVersionBitmask */
},
{
  C_RENUMBER,  /* UniqueID */
  "RENUMBER",  /* Syntax */
  "Implementation defined.",  /* Description */
  "RENUMBER",  /* Name */
  HB1 | HB2  /* OptionVersionBitmask */
},
{
  C_REPEAT,  /* UniqueID */
  "REPEAT",  /* Syntax */
  "Top of a REPEAT - UNTIL structure.",  /* Description */
  "REPEAT",  /* Name */
  B15 | H14 | HB2  /* OptionVersionBitmask */
},
{
  C_REPLACE,  /* UniqueID */
  "REPLACE [filename$]",  /* Syntax */
  "Saves the current program into the file filename$ in ASCII format.",  /* Description */
  "REPLACE",  /* Name */
  H80  /* OptionVersionBitmask */
},
{
  C_RESET,  /* UniqueID */
  "RESET filename$ [, ...]",  /* Syntax */
  "Positions an input or output file to the bcginning.",  /* Description */
  "RESET",  /* Name */
  S70 | I70 | I73  /* OptionVersionBitmask */
},
{
  C_RESTORE,  /* UniqueID */
  "RESTORE [line]",  /* Syntax */
  "Resets the line used for the next READ statement.  line may be "
  "either a number or a label.",  /* Description */
  "RESTORE",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_RESUME,  /* UniqueID */
  "RESUME",  /* Syntax */
  "Used in an error handler to specify the next line to execute. "
  " Branch to ERL.",  /* Description */
  "RESUME",  /* Name */
  B15 | H14 | HB1 | HB2 | M80 | R86 | D71 | T80 | T79  /* OptionVersionBitmask */
},
{
  C_RESUME,  /* UniqueID */
  "RESUME line",  /* Syntax */
  "Used in an error handler to specify the next line to execute. "
  " Branch to the specified line.",  /* Description */
  "RESUME",  /* Name */
  B15 | H14 | HB1 | HB2 | M80 | R86 | D71 | T80 | T79  /* OptionVersionBitmask */
},
{
  C_RESUME,  /* UniqueID */
  "RESUME NEXT",  /* Syntax */
  "Used in an error handler to specify the next line to execute. "
  " Branch to the line after ERL.",  /* Description */
  "RESUME",  /* Name */
  B15 | H14 | HB1 | HB2 | M80 | R86 | D71 | T80 | T79  /* OptionVersionBitmask */
},
{
  C_RESUME,  /* UniqueID */
  "RESUME 0",  /* Syntax */
  "Used in an error handler to specify the next line to execute. "
  " Branch to ERL.",  /* Description */
  "RESUME",  /* Name */
  B15 | H14 | HB1 | HB2 | M80 | R86 | D71 | T80 | T79  /* OptionVersionBitmask */
},
{
  C_RETURN,  /* UniqueID */
  "RETURN",  /* Syntax */
  "Concludes a subroutine called by GOSUB.",  /* Description */
  "RETURN",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_RSET,  /* UniqueID */
  "RSET variable$ = value",  /* Syntax */
  "Right-aligns the value into variable$.  If the length of the "
  "value is too short, then it is padded on the left with spaces. "
  " If the length of the value is too long, then it is truncated "
  "on the right.  This is only for use with variables assigned to "
  "a random access buffer with FIELD command.",  /* Description */
  "RSET",  /* Name */
  B15 | B93 | H14 | M80 | R86 | D71 | T80 | T79  /* OptionVersionBitmask */
},
{
  C_RUN,  /* UniqueID */
  "RUN filename$",  /* Syntax */
  "Loads a new BAASIC program and executes the program from the "
  "start.",  /* Description */
  "RUN",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | T79  /* OptionVersionBitmask */
},
{
  C_RUN,  /* UniqueID */
  "RUN line",  /* Syntax */
  "Executes the program in memory beginning at line.",  /* Description */
  "RUN",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | T79  /* OptionVersionBitmask */
},
{
  C_RUN,  /* UniqueID */
  "RUN",  /* Syntax */
  "Executes the program in memory from the start.",  /* Description */
  "RUN",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | T79  /* OptionVersionBitmask */
},
{
  C_RUNNH,  /* UniqueID */
  "RUNNH line",  /* Syntax */
  "Executes the program in memory beginning at line.",  /* Description */
  "RUNNH",  /* Name */
  B15 | S70 | C77 | D64 | G74 | G65 | G67 | D71 | I70 | I73  /* OptionVersionBitmask */
},
{
  C_RUNNH,  /* UniqueID */
  "RUNNH filename$",  /* Syntax */
  "Loads a new BAASIC program and executes the program from the "
  "start.",  /* Description */
  "RUNNH",  /* Name */
  B15 | S70 | C77 | D64 | G74 | G65 | G67 | D71 | I70 | I73  /* OptionVersionBitmask */
},
{
  C_RUNNH,  /* UniqueID */
  "RUNNH",  /* Syntax */
  "Executes the program in memory from the start.",  /* Description */
  "RUNNH",  /* Name */
  B15 | S70 | C77 | D64 | G74 | G65 | G67 | D71 | I70 | I73  /* OptionVersionBitmask */
},
{
  C_SAVE,  /* UniqueID */
  "SAVE [filename$]",  /* Syntax */
  "Saves the current program into the file filename$ in ASCII format.",  /* Description */
  "SAVE",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | T79  /* OptionVersionBitmask */
},
{
  C_SCRATCH,  /* UniqueID */
  "SCRATCH [# X]",  /* Syntax */
  "SCRATCH Deletes the program in memory and clears all variables. "
  " SCRATCH # X Sets the file mode to writing.",  /* Description */
  "SCRATCH",  /* Name */
  G74 | HB2 | H80 | G65 | G67  /* OptionVersionBitmask */
},
{
  C_SELECT,  /* UniqueID */
  "SELECT",  /* Syntax */
  "Syntax Error.",  /* Description */
  "SELECT",  /* Name */
  B15 | B93 | E86 | H14  /* OptionVersionBitmask */
},
{
  C_SELECT_CASE,  /* UniqueID */
  "SELECT CASE value",  /* Syntax */
  "Introduces a multi-line conditional selection statement.",  /* Description */
  "SELECT CASE",  /* Name */
  B15 | B93 | E86 | H14  /* OptionVersionBitmask */
},
{
  C_STEP,  /* UniqueID */
  "STEP",  /* Syntax */
  "Syntax Error.",  /* Description */
  "STEP",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_STOP,  /* UniqueID */
  "STOP",  /* Syntax */
  "Interrupts program execution and displays the line number of "
  "the STOP command.  For use when debugging BASIC programs.  Whether "
  "STOP issues a SIGINT signal is implementation defined.",  /* Description */
  "STOP",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_STORE,  /* UniqueID */
  "STORE ArrayName",  /* Syntax */
  "Saves a numeric array into a file for later loading by RECALL.",  /* Description */
  "STORE",  /* Name */
  B15 | HB2  /* OptionVersionBitmask */
},
{
  C_SUB,  /* UniqueID */
  "SUB name [ ( parameter [,...] ) ]",  /* Syntax */
  "Top line of a multi-line SUB definition.  The variable names "
  "specified are local to the SUB definition, and are initialized "
  "BYVAL when the subroutine is invoked by another routine.",  /* Description */
  "SUB",  /* Name */
  B15 | B93 | E86 | H14 | HB2  /* OptionVersionBitmask */
},
{
  C_SUB_END,  /* UniqueID */
  "SUB END",  /* Syntax */
  "Specifies the last line of a multi-line SUB definition.  Same "
  "as END SUB.",  /* Description */
  "SUB END",  /* Name */
  H14  /* OptionVersionBitmask */
},
{
  C_SUB_EXIT,  /* UniqueID */
  "SUB EXIT",  /* Syntax */
  "Immediately exits the inner-most multi-line SUB strucure.  Same "
  "as EXIT SUB.",  /* Description */
  "SUB EXIT",  /* Name */
  H14  /* OptionVersionBitmask */
},
{
  C_SUBEND,  /* UniqueID */
  "SUBEND",  /* Syntax */
  "Specifies the last line of a multi-line SUB definition.  Same "
  "as END SUB.",  /* Description */
  "SUBEND",  /* Name */
  B15 | H14 | HB2  /* OptionVersionBitmask */
},
{
  C_SUBEXIT,  /* UniqueID */
  "SUBEXIT",  /* Syntax */
  "Immediately exits the inner-most multi-line SUB strucure.  Same "
  "as EXIT SUB.",  /* Description */
  "SUBEXIT",  /* Name */
  H14  /* OptionVersionBitmask */
},
{
  C_SWAP,  /* UniqueID */
  "SWAP variable, variable",  /* Syntax */
  "Swaps the values of two variables. Both variables must be of "
  "the same type.",  /* Description */
  "SWAP",  /* Name */
  B15 | B93 | H14 | HB2 | M80 | R86 | D71 | T80 | T79  /* OptionVersionBitmask */
},
{
  C_SYSTEM,  /* UniqueID */
  "SYSTEM",  /* Syntax */
  "Exits to the operating system.",  /* Description */
  "SYSTEM",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | G65 | G67 | M80 | D71 | I70 | I73 | T80 | V09  /* OptionVersionBitmask */
},
{
  C_TEXT,  /* UniqueID */
  "TEXT letter[-letter] [, ...]",  /* Syntax */
  "Declares variables with single-letter names as string variables.",  /* Description */
  "TEXT",  /* Name */
  HB1 | HB2  /* OptionVersionBitmask */
},
{
  C_THEN,  /* UniqueID */
  "THEN",  /* Syntax */
  "Syntax Error.",  /* Description */
  "THEN",  /* Name */
  B15 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_TIMER,  /* UniqueID */
  "TIMER",  /* Syntax */
  "Syntax Error.",  /* Description */
  "TIMER",  /* Name */
  B15  /* OptionVersionBitmask */
},
{
  C_TIMER_OFF,  /* UniqueID */
  "TIMER OFF",  /* Syntax */
  "TIMER OFF terminates the timer interrupt.",  /* Description */
  "TIMER OFF",  /* Name */
  B15  /* OptionVersionBitmask */
},
{
  C_TIMER_ON,  /* UniqueID */
  "TIMER ON",  /* Syntax */
  "TIMER ON enables the timer interrupt.  When the specified seconds "
  "have elapsed, TIMER STOP is internaly executed before the interrupt "
  "is taken.  TIMER ON should be executed just before the RETURN "
  "command if you want the interrupt to occur again.",  /* Description */
  "TIMER ON",  /* Name */
  B15  /* OptionVersionBitmask */
},
{
  C_TIMER_STOP,  /* UniqueID */
  "TIMER STOP",  /* Syntax */
  "TIMER STOP disables the interrupt, but the count continues.",  /* Description */
  "TIMER STOP",  /* Name */
  B15  /* OptionVersionBitmask */
},
{
  C_TLOAD,  /* UniqueID */
  "TLOAD [filename$]",  /* Syntax */
  "Loads an ASCII BASIC program into memory.",  /* Description */
  "TLOAD",  /* Name */
  B15 | HB2  /* OptionVersionBitmask */
},
{
  C_TO,  /* UniqueID */
  "TO",  /* Syntax */
  "Syntax Error.",  /* Description */
  "TO",  /* Name */
  B15 | B93 | S70 | C77 | D64 | E78 | E86 | G74 | H14 | HB1 | HB2 | H80 | G65 | G67 | M80 | D73 | D70 | R86 | D71 | I70 | I73 | T80 | V09 | T79  /* OptionVersionBitmask */
},
{
  C_TRACE,  /* UniqueID */
  "TRACE",  /* Syntax */
  "Enables tracing.",  /* Description */
  "TRACE",  /* Name */
  G74 | HB1 | HB2 | G67  /* OptionVersionBitmask */
},
{
  C_TRACE_OFF,  /* UniqueID */
  "TRACE OFF",  /* Syntax */
  "Disables tracing.",  /* Description */
  "TRACE OFF",  /* Name */
  G74 | HB1 | HB2 | G67  /* OptionVersionBitmask */
},
{
  C_TRACE_ON,  /* UniqueID */
  "TRACE ON",  /* Syntax */
  "Enables tracing.",  /* Description */
  "TRACE ON",  /* Name */
  G74 | HB1 | HB2 | G67  /* OptionVersionBitmask */
},
{
  C_TSAVE,  /* UniqueID */
  "TSAVE [filename$]",  /* Syntax */
  "Saves the current program into the file filename$ in ASCII format.",  /* Description */
  "TSAVE",  /* Name */
  B15 | HB2  /* OptionVersionBitmask */
},
{
  C_TTY,  /* UniqueID */
  "TTY",  /* Syntax */
  "Directs the PRINT commands to write to the console (stdout), "
  "and the INPUT commands to read from.the console (stdin).  This "
  "cancels LPT, PTP and PTR.",  /* Description */
  "TTY",  /* Name */
  D73  /* OptionVersionBitmask */
},
{
  C_TTY_IN,  /* UniqueID */
  "TTY IN",  /* Syntax */
  "Directs the INPUT commands to read from.the console (stdin). "
  " This cancels PTR.  It does not cancel LPT or PTP.",  /* Description */
  "TTY IN",  /* Name */
  D73  /* OptionVersionBitmask */
},
{
  C_TTY_OUT,  /* UniqueID */
  "TTY OUT",  /* Syntax */
  "Directs the PRINT commands to write to the console (stdout). "
  " This cancels LPT or PTP.  It does not cancel PTR.",  /* Description */
  "TTY OUT",  /* Name */
  D73  /* OptionVersionBitmask */
},
{
  C_UNTIL,  /* UniqueID */
  "UNTIL value",  /* Syntax */
  "Bottom of a REPEAT - UNTIL.  If the value is non-zero, then the "
  "loop is terminated.",  /* Description */
  "UNTIL",  /* Name */
  B15 | H14 | HB2  /* OptionVersionBitmask */
},
{
  C_USE,  /* UniqueID */
  "USE parameter$",  /* Syntax */
  "Receives the value from the calling CHAINing program.",  /* Description */
  "USE",  /* Name */
  S70 | I70 | I73  /* OptionVersionBitmask */
},
{
  C_VARS,  /* UniqueID */
  "VARS",  /* Syntax */
  "Prints a list of all global variables.",  /* Description */
  "VARS",  /* Name */
  B15 | B93  /* OptionVersionBitmask */
},
{
  C_WEND,  /* UniqueID */
  "WEND",  /* Syntax */
  "Bottom of a WHILE - WEND structure.",  /* Description */
  "WEND",  /* Name */
  B15 | B93 | C77 | H14 | HB1 | HB2 | M80 | D71 | T80  /* OptionVersionBitmask */
},
{
  C_WHILE,  /* UniqueID */
  "WHILE value",  /* Syntax */
  "Top of a WHILE - WEND structure.  If the value is non-zero, then "
  "the loop is terminated.",  /* Description */
  "WHILE",  /* Name */
  B15 | B93 | C77 | H14 | HB1 | HB2 | M80 | D71 | T80  /* OptionVersionBitmask */
},
{
  C_WRITE,  /* UniqueID */
  "WRITE # filenum, value [, .... ]",  /* Syntax */
  "Outputs value to a file.    Strings are quoted and each value "
  "is seperated by a omma.",  /* Description */
  "WRITE",  /* Name */
  B15 | B93 | G74 | H14 | G65 | G67 | M80 | D71 | T80  /* OptionVersionBitmask */
},
{
  C_WRITE,  /* UniqueID */
  "WRITE value [, .... ]",  /* Syntax */
  "Outputs value to the screen.  Strings are quoted and each value "
  "is seperated by a comma.",  /* Description */
  "WRITE",  /* Name */
  B15 | B93 | G74 | H14 | G65 | G67 | M80 | D71 | T80  /* OptionVersionBitmask */
},
};

const size_t NUM_COMMANDS =
  sizeof (IntrinsicCommandTable) / sizeof (CommandType);


/* EOF */
