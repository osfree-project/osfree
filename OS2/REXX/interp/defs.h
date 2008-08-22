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
 * $Id: defs.h,v 1.32 2004/02/22 09:25:26 florian Exp $
 */

#define X_NULL            1
#define X_PROGRAM         2
#define X_STATS           3
#define X_COMMAND         4
#define X_ADDR_V          5
#define X_ADDR_N          6
#define X_ADDR_S          7
#define X_CALL            8
#define X_DO              9
#define X_REP            10
#define X_REP_FOREVER    11
#define X_DO_TO          12
#define X_DO_BY          13
#define X_DO_FOR         14
#define X_WHILE          15
#define X_UNTIL          16
#define X_DROP           17
#define X_EXIT           18
#define X_IF             19
#define X_IPRET          20
#define X_ITERATE        21
#define X_LABEL          22
#define X_LEAVE          23
#define X_NUM_D          24
#define X_NUM_F          25
#define X_NUM_FUZZ       26
#define X_NUM_SCI        27
#define X_NUM_ENG        28
#define X_PARSE          29
#define X_PARSE_ARG      30
#define X_PARSE_EXT      31
#define X_PARSE_PULL     32
#define X_PARSE_SRC      33
#define X_PARSE_VAR      34
#define X_PARSE_VAL      35
#define X_PARSE_VER      36
#define X_PROC           37
#define X_PULL           38
#define X_PUSH           39
#define X_QUEUE          40
#define X_RETURN         41
#define X_SAY            42
#define X_SELECT         43
#define X_WHENS          44
#define X_WHEN           45
#define X_OTHERWISE      46
#define X_SIG_VAL        47
#define X_SIG_LAB        48
#define X_SIG_SET        49
#define X_ON             50
#define X_OFF            51
#define X_S_ERROR        52
#define X_S_HALT         53
#define X_S_NOVALUE      54
#define X_S_SYNTAX       55
#define X_TRACE          56
#define X_UPPER_VAR      57
#define X_ASSIGN         58
#define X_LOG_NOT        59
#define X_PLUSS          60
#define X_EQUAL          61
#define X_MINUS          62
#define X_MULT           63
#define X_DEVIDE         64
#define X_MODULUS        65
#define X_LOG_OR         66
#define X_LOG_AND        67
#define X_LOG_XOR        68
#define X_EXP            69
#define X_CONCAT         70
#define X_SPACE          71
#define X_GTE            72
#define X_LTE            73
#define X_GT             74
#define X_LT             75
#define X_DIFF           76
#define X_SIM_SYMBOL     77
#define X_CON_SYMBOL     78
#define X_STRING         79
#define X_U_MINUS        80
#define X_S_EQUAL        81
#define X_S_DIFF         82
#define X_INTDIV         83
#define X_EX_FUNC        84
#define X_IN_FUNC        85
#define X_TPL_SOLID      86
#define X_TPL_MVE        87
#define X_TPL_VAR        88
#define X_TPL_SYMBOL     89
#define X_TPL_POINT      90
#define X_POS_OFFS       91
#define X_NEG_OFFS       92
#define X_ABS_OFFS       93
#define X_EXPRLIST       94
#define X_S_NOTREADY     95
#define X_S_FAILURE      96
#define X_END            97
#define X_CALL_SET       98
#define X_NO_OTHERWISE   99
#define X_IND_SYMBOL    100
#define X_IS_INTERNAL   101
#define X_IS_BUILTIN    102
#define X_IS_EXTERNAL   103
#define X_CTAIL_SYMBOL  104
#define X_VTAIL_SYMBOL  105
#define X_HEAD_SYMBOL   106
#define X_STEM_SYMBOL   107
#define X_SEQUAL        108
#define X_SDIFF         109
#define X_SGT           110
#define X_SGTE          111
#define X_SLT           112
#define X_SLTE          113
#define X_NEQUAL        114
#define X_NDIFF         115
#define X_NGT           116
#define X_NGTE          117
#define X_NLT           118
#define X_NLTE          119
#define X_NASSIGN       120
#define X_CEXPRLIST     121
#define X_U_PLUSS       122
#define X_OPTIONS       123
#define X_NUM_V         124
#define X_NUM_DDEF      125
#define X_NUM_FDEF      126
#define X_NUM_FRMDEF    127
#define X_S_NGT         128
#define X_S_NLT         129
#define X_S_GT          130
#define X_S_GTE         131
#define X_S_LT          132
#define X_S_LTE         133
#define X_ADDR_WITH     134
#define X_S_LOSTDIGITS  135
#define X_DO_EXPR       136


/* The three first two numbers have not errortext attched to them */
#define ERR_PROG_UNREADABLE     3
#define ERR_PROG_INTERRUPT      4
#define ERR_STORAGE_EXHAUSTED   5
#define ERR_UNMATCHED_QUOTE     6
#define ERR_WHEN_EXPECTED       7
#define ERR_THEN_UNEXPECTED     8
#define ERR_WHEN_UNEXPECTED     9
#define ERR_UNMATCHED_END       10
#define ERR_FULL_CTRL_STACK     11
#define ERR_TOO_LONG_LINE       12
#define ERR_INVALID_CHAR        13
#define ERR_INCOMPLETE_STRUCT   14
#define ERR_INVALID_HEX_CONST   15
#define ERR_UNEXISTENT_LABEL    16
#define ERR_UNEXPECTED_PROC     17
#define ERR_THEN_EXPECTED       18
#define ERR_STRING_EXPECTED     19
#define ERR_SYMBOL_EXPECTED     20
#define ERR_EXTRA_DATA          21
/* The next to number have not been assigned an errortext */
#define ERR_INVALID_TRACE       24
#define ERR_INV_SUBKEYWORD      25
#define ERR_INVALID_INTEGER     26
#define ERR_INVALID_DO_SYNTAX   27
#define ERR_INVALID_LEAVE       28
#define ERR_ENVIRON_TOO_LONG    29
#define ERR_TOO_LONG_STRING     30
#define ERR_INVALID_START       31
#define ERR_INVALID_STEM        32
#define ERR_INVALID_RESULT      33
#define ERR_UNLOGICAL_VALUE     34
#define ERR_INVALID_EXPRESSION  35
#define ERR_UNMATCHED_PARAN     36
#define ERR_UNEXPECTED_PARAN    37
#define ERR_INVALID_TEMPLATE    38
#define ERR_STACK_OVERFLOW      39
#define ERR_INCORRECT_CALL      40
#define ERR_BAD_ARITHMETIC      41
#define ERR_ARITH_OVERFLOW      42
#define ERR_ROUTINE_NOT_FOUND   43
#define ERR_NO_DATA_RETURNED    44
#define ERR_DATA_NOT_SPEC       45
/* No errortexts have not been defined to the next to numbers */
#define ERR_SYSTEM_FAILURE      48
#define ERR_INTERPRETER_FAILURE 49
#define ERR_RESERVED_SYMBOL     50
#define ERR_UNQUOTED_FUNC_STOP  51
#define ERR_INVALID_OPTION      53
#define ERR_INVALID_STEM_OPTION 54

#define ERR_CANT_REWIND         60
#define ERR_IMPROPER_SEEK       61

#define ERR_YACC_SYNTAX         64

#define ERR_NON_ANSI_FEATURE    90
#define ERR_STREAM_COMMAND      93
#define ERR_EXTERNAL_QUEUE      94

#define ERR_RESTRICTED          95

#define ERR_UNKNOWN_FS_ERROR    100

#define ERR_MAX_NUMBER          100
/*
 #define ENV_BOURNE_SH  0
 #define ENV_C_SHELL    1
 #define ENV_COMMAND     2
 #define ENV_PATH        3
 #define ENV_SYSTEM      4
 */

#ifdef TRACEMEM
/*
 * NOTE: There is a close correspondance between these and the char
 *       array alloc in memory.c
 */
# define TRC_LEAKED     0
# define TRC_HASHTAB    1
# define TRC_PROCBOX    2
# define TRC_SOURCE     3
# define TRC_SOURCEL    4
# define TRC_TREENODE   5
# define TRC_VARVALUE   6
# define TRC_VARNAME    7
# define TRC_VARBOX     8
# define TRC_STACKBOX   9
# define TRC_STACKLINE 10
# define TRC_SYSINFO   11
# define TRC_FILEPTR   12
# define TRC_PROCARG   13
# define TRC_LABEL     14
# define TRC_STATIC    15
# define TRC_P_CACHE   16
# define TRC_MATH      17
# define TRC_ENVIRBOX  18
# define TRC_ENVIRNAME 19
# define TRC_SPCV_BOX  20
# define TRC_SPCV_NAME 21
# define TRC_SPCV_NUMB 22
# define TRC_SPCV_NUMC 23



# define MEMTRC_NONE    0
# define MEMTRC_ALL     1
# define MEMTRC_LEAKED  2

# define MEM_ALLOC      0
# define MEM_CURRENT    1
# define MEM_LEAKED     2
#endif /* TRACEMEM */


/*
 * Note: must match the definition of  numeric_forms  in rexx.c
 */
#define NUM_FORM_SCI    0
#define NUM_FORM_ENG    1


#define SIGNAL_FATAL     -1
#define SIGNAL_ERROR      0
#define SIGNAL_FAILURE    1
#define SIGNAL_HALT       2
#define SIGNAL_NOVALUE    3
#define SIGNAL_NOTREADY   4
#define SIGNAL_SYNTAX     5
#define SIGNAL_LOSTDIGITS 6
#define SIGNALS           7

#define SIGTYPE_ON    0
#define SIGTYPE_OFF   1
#define SIGTYPE_DELAY 2


#define ENVIR_SHELL 1
#define ENVIR_PIPE 2

#define SUBENVIR_PATH 1
#define SUBENVIR_SYSTEM 2
#define SUBENVIR_COMMAND 3
#define SUBENVIR_REXX 4


#define REDIR_NONE      0x00000000
#define REDIR_INPUT     0x00000001
#define REDIR_OUTLIFO   0x00000002
#define REDIR_OUTFIFO   0x00000004
#define REDIR_OUTPUT    0x00000006
#define REDIR_CLEAR     0x00000008
#define REDIR_OUTSTRING 0x00000010


#define HOOK_LINEFEED  1
#define HOOK_NOLF      0

#define HOOK_GO_ON     1
#define HOOK_NOPE      0

#define HOOK_STDOUT    0
#define HOOK_STDERR    1
#define HOOK_TRCIN     2
#define HOOK_PULL      3
#define HOOK_INIT      4
#define HOOK_TERMIN    5
#define HOOK_SUBCOM    6
#define HOOK_FUNC      7
#define HOOK_GETENV    8
#define HOOK_SETENV    9
#define HOOK_GETCWD   10
#define HOOK_SETCWD   11

#define HOOK_MASK(a)   (1<<(a))

#define PARAM_TYPE_HARD 0
#define PARAM_TYPE_SOFT 1

#define INVO_COMMAND    0
#define INVO_FUNCTION   1
#define INVO_SUBROUTINE 2

#define SYMBOL_BAD      0
#define SYMBOL_CONSTANT 1
#define SYMBOL_STEM     2
#define SYMBOL_SIMPLE   3
#define SYMBOL_COMPOUND 4
#define SYMBOL_NUMBER   5


#define VFLAG_NONE   0x0000
#define VFLAG_STR    0x0001
#define VFLAG_NUM    0x0002

#define VFLAG_BOTH   (VFLAG_STR | VFLAG_NUM)

#define UNKNOWN_QUEUE -1
/*
 * For refering to the extensions.
 */
#define EXT_FLUSHSTACK                       0
#define EXT_LINEOUTTRUNC                     1
#define EXT_BUFTYPE_BIF                      2
#define EXT_DESBUF_BIF                       3
#define EXT_DROPBUF_BIF                      4
#define EXT_MAKEBUF_BIF                      5
#define EXT_CACHEEXT                         6
#define EXT_PRUNE_TRACE                      7
#define EXT_EXT_COMMANDS_AS_FUNCS            8
#define EXT_STDOUT_FOR_STDERR                9
#define EXT_TRACE_HTML                      10
#define EXT_FAST_LINES_BIF_DEFAULT          11
#define EXT_STRICT_ANSI                     12
#define EXT_INTERNAL_QUEUES                 13
#define EXT_REGINA_BIFS                     14
#define EXT_STRICT_WHITE_SPACE_COMPARISONS  15
#define EXT_AREXX_SEMANTICS                 16
#define EXT_AREXX_BIFS                      17
#define EXT_BROKEN_ADDRESS_COMMAND          18
#define EXT_CALLS_AS_FUNCS                  19
#define EXT_QUEUES_301                      20

/*
 * Regina 3.1 introduced "pool 0" variables. There exist only some and they
 * can be determined by known_reserved_variable. Here are the return
 * values of this function.
 */
#define POOL0_NOT_RESERVED    0  /* must be 0, following must be consecutive */
#define POOL0_RC              1  /* Never change the order, it is hard-wired */
#define POOL0_RESULT          2  /* used in variable.c                       */
#define POOL0_SIGL            3
#define POOL0_RS              4
#define POOL0_MN              5
#define POOL0_CNT             ( POOL0_MN + 1 )

/*
 * treenode.u.parseflags values which may be or'ed.
 */
#define PARSE_NORMAL    0
#define PARSE_UPPER     1
#define PARSE_LOWER     2
#define PARSE_CASELESS  4
