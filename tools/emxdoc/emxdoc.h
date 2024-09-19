/* emxdoc.h -- Global header file
   Copyright (c) 1993-1999 Eberhard Mattes

This file is part of emxdoc.

emxdoc is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

emxdoc is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with emxdoc; see the file COPYING.  If not, write to
the Free Software Foundation, 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */


#if !defined (EXTERN)
#define EXTERN extern
#define INIT(X)
#endif

#define FALSE 0
#define TRUE  1

#define ENV_STACK_SIZE          8
#define HL_STACK_SIZE           8
#define STYLE_STACK_SIZE        8
#define SECTION_LEVELS          3

#define HL_BF                   0x01
#define HL_SL                   0x02
#define HL_TT                   0x04
#define HL_UL                   0x08
#define HL_EM                   0x10

#define HF_UNNUMBERED           0x01
#define HF_HIDE                 0x02

/* Flag bits for the `flags' field of struct word. */

#define WF_ABBREV               0x01

#ifdef __GNUC__
#define PRINTF(x,y) __attribute__ ((format (printf, x, y)))
#define NORETURN2 __attribute__ ((noreturn))
#else
#define PRINTF(x,y)
#define NORETURN2
#endif

typedef unsigned char uchar;

enum tag
{
  TAG_HEADING,
  TAG_TOC,
  TAG_TITLE,
  TAG_IPF,
  TAG_ENDIPF,
  TAG_ITEM,
  TAG_SAMPLECODE,
  TAG_ENDSAMPLECODE,
  TAG_VERBATIM,
  TAG_ENDVERBATIM,
  TAG_EXAMPLE,
  TAG_ENDEXAMPLE,
  TAG_HEADERS,
  TAG_ENDHEADERS,
  TAG_PROTOTYPE,
  TAG_ENDPROTOTYPE,
  TAG_DESCRIPTION,
  TAG_ENDDESCRIPTION,
  TAG_ENUMERATE,
  TAG_ENDENUMERATE,
  TAG_ITEMIZE,
  TAG_ENDITEMIZE,
  TAG_LIST,
  TAG_ENDLIST,
  TAG_INDENT,
  TAG_ENDINDENT,
  TAG_TYPEWRITER,
  TAG_ENDTYPEWRITER,
  TAG_TABLE,
  TAG_ENDTABLE,
  TAG_LABEL,
  TAG_TEXT,
  TAG_ENDTEXT,
  TAG_LATEX,
  TAG_ENDLATEX,
  TAG_HTML,
  TAG_ENDHTML,
  TAG_FORMAT,
  TAG_STYLE,
  TAG_REF,
  TAG_HPT,
  TAG_BREAK,
  TAG_INDEX,
  TAG_KEYWORD,
  TAG_FUNCTION,
  TAG_SEEALSO,
  TAG_COMPAT,
  TAG_SAMPLEFILE,
  TAG_RESTRICTIONS,
  TAG_IMPLEMENTATION,
  TAG_BUGS,
  TAG_RETURNVALUE,
  TAG_PARAM,
  TAG_LANGUAGE,
  TAG_SYNTAX,
  TAG_IPFMINITOC,
  TAG_HTMLMINITOC,
  TAG_MINITOC,
  TAG_HTMLFRAGMENT,
  TAG_SPECIAL,
  TAG_FULLSTOP,
  TAG_SET,
  TAG_REPLACE,
  TAG_ERRORS,
  TAG_HINTS
};

enum style
{
  STYLE_NORMAL,
  STYLE_BOLD,
  STYLE_TTY,
  STYLE_UNDERLINE,
  STYLE_SLANTED,
  STYLE_SYNTAX,
  STYLE_PARAM,
  STYLE_EMPHASIZE
};

enum el
{
  EL_END,
  EL_WORD,
  EL_PUNCT,
  EL_SPACE,
  EL_STYLE,
  EL_ENDSTYLE,
  EL_REF,
  EL_BREAK
};

enum env
{
  ENV_NONE,
  ENV_ENUMERATE,
  ENV_ITEMIZE,
  ENV_DESCRIPTION,
  ENV_LIST,
  ENV_INDENT,
  ENV_TYPEWRITER
};

enum lang
{
  LANG_ENGLISH,
  LANG_GERMAN
};

enum syntax
{
  SYNTAX_EMX,
  SYNTAX_DVIDRV
};

struct special
{
  uchar *text;
  uchar *ipf;
  uchar *latex;
  uchar *html;
};

struct word
{
  struct word *next;
  struct word *database;
  enum style style;
  uchar *str;
  uchar *repl;
  struct special *special;
  struct word_table *subidx;
  int ref, idx;
  uchar flags;
};

struct word_table;

struct element
{
  enum el el;
  int n;
  struct word *wp;
};

struct environment
{
  enum env env;
  int tmargin;
  int imargin;
  int start_line;
  int counter;
};

struct toc
{
  struct toc *next;
  int level;
  int ref;
  int print;
  int global;
  unsigned flags;
  uchar *number;
  uchar *title;
};

enum enc { ENC_DEFAULT = -3, ENC_AMBIGUOUS = -2, ENC_UNSUPPORTED = -1,
           ENC_CP850 = 0, ENC_ISO8859_1, ENCODINGS };

/* This is the escape character. */
EXTERN uchar escape INIT (0xfe);

/* The current input line number. */
EXTERN int line_no INIT (0);

/* The current input line. */
EXTERN uchar input[256];

/* The input encoding. */
EXTERN enum enc input_encoding INIT (ENC_DEFAULT);

/* The output encoding. */
EXTERN enum enc output_encoding INIT (ENC_DEFAULT);

/* This is the processing mode. */
EXTERN char mode INIT (0);

/* Concatenate instead of call .inf files */
EXTERN int opt_a INIT (FALSE);

/* Select line break algorithm for text output. */
EXTERN int opt_b INIT (0);

/* Use color instead of slanted type. */
EXTERN int opt_c INIT (FALSE);

/* French spacing. */
EXTERN int opt_f INIT (FALSE);

/* Gather global information. */
EXTERN int opt_g INIT (FALSE);

/* Display warning messages. */
EXTERN int opt_w INIT (0);

EXTERN const char *input_fname INIT (NULL);

/* The input stream. */
EXTERN FILE *input_file INIT (NULL);

/* Non-zero if the end of the input file has been reached. */
EXTERN int end_of_file INIT (FALSE);

/* The output stream. */
EXTERN FILE *output_file INIT (NULL);

/* The global table of words. */
EXTERN struct word_table *word_top INIT (NULL);

EXTERN int hl_stack[HL_STACK_SIZE];
EXTERN int hl_sp;

/* Current line number of the output file. */
EXTERN int output_line_no;

/* Current column number of the output file. */
EXTERN int output_x;

/* Non-zero when writing output file (second pass). */
EXTERN int out;

/* Non-zero while formatting a prototype. */
EXTERN int prototype_flag INIT (FALSE);

/* The stack of nested environments. */
EXTERN struct environment env_stack[ENV_STACK_SIZE];

/* Stack pointer for stack of nested environments. */
EXTERN int env_sp;

/* Current list of formatting elements. */
EXTERN struct element *elements INIT (NULL);

/* Non-zero if a new paragraph should be started. */
EXTERN int para_flag;

/* Number of functions. */
EXTERN int function_count;

/* ... */
EXTERN int ref_no INIT (1);

/* ... */
EXTERN int idx_no INIT (0);

/* The title of the document. */
EXTERN uchar *title INIT (NULL);

/* The language of the document. */
EXTERN enum lang language INIT (LANG_ENGLISH);

/* Current tag: the tag. */
EXTERN enum tag tg_tag;

/* Current tag: style. */
EXTERN enum style tg_style;

/* Current tag: level (for h-, h=, h1 through h3, i1, i2 and index). */
EXTERN int tg_level;

/* Current tag: character for underlining (for h-, h= and h1 through h3). */
EXTERN int tg_underline;

/* Current tag: flags (such as HF_UNNUMBERED). */
EXTERN unsigned tg_flags;


void fatal (const char *fmt, ...) NORETURN2 PRINTF (1, 2);
void nonfatal (const char *fmt, ...) PRINTF (1, 2);
void warning (int level, const char *fmt, ...) PRINTF (2, 3);
void *xmalloc (size_t n);
uchar *xstrdup (const uchar *s);

struct word_table *wt_new (unsigned hash_size);
struct word *wt_find (struct word_table *wt, const uchar *str, unsigned hash);
unsigned wt_hash (struct word_table *wt, const uchar *str);
struct word *wt_add (struct word_table *wt, const uchar *str);
int wt_walk (struct word_table *wt, int (*callback)(struct word *));
int wt_count (const struct word_table *wt);

struct word *word_find (const uchar *str, unsigned hash);
unsigned word_hash (const uchar *str);
struct word *word_add (const uchar *str);
int word_walk (int (*callback)(struct word *));

void write_nl (void);
void write_break (void);
void write_nstring (const uchar *p, size_t n);
void write_string (const uchar *p);
void write_line (const uchar *p);
void write_fmt (const char *fmt, ...) PRINTF (1, 2);
void write_space (void);
void format_output (const uchar *p, int may_break);
void format_string (const uchar *p, int sty, int may_break);
void format_spaces (int n, enum style style, int may_break);
void make_elements (const uchar *p);
void start_hilite (int hilite);
void end_hilite (void);
struct word *use_reference (const uchar *p);
void open_input (const char *name);
void read_line (void);
void init_file (void);
int parse_tag (const uchar **ptr);
void start_index (void);
