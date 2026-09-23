/*!
 * @file emxdoc.h
 * @brief Global header file.
 *
 * Copyright (c) 1993-1999 Eberhard Mattes
 *
 * This file is part of emxdoc.
 *
 * emxdoc is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * emxdoc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with emxdoc; see the file COPYING.  If not, write to
 * the Free Software Foundation, 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */


#if !defined (EXTERN)
#define EXTERN extern
#define INIT(X)
#endif

/*!
 * @brief Boolean false value.
 */
#define FALSE 0
/*!
 * @brief Boolean true value.
 */
#define TRUE  1

/*!
 * @brief Maximum environment nesting depth.
 */
#define ENV_STACK_SIZE          8
/*!
 * @brief Maximum highlighting nesting depth.
 */
#define HL_STACK_SIZE           8
/*!
 * @brief Maximum style nesting depth.
 */
#define STYLE_STACK_SIZE        8
/*!
 * @brief Maximum number of heading levels.
 */
#define SECTION_LEVELS          3

/*!
 * @brief Bold highlight bit.
 */
#define HL_BF                   0x01
/*!
 * @brief Slanted highlight bit.
 */
#define HL_SL                   0x02
/*!
 * @brief Typewriter highlight bit.
 */
#define HL_TT                   0x04
/*!
 * @brief Underline highlight bit.
 */
#define HL_UL                   0x08
/*!
 * @brief Emphasize highlight bit.
 */
#define HL_EM                   0x10

/*!
 * @brief Unnumbered heading flag.
 */
#define HF_UNNUMBERED           0x01
/*!
 * @brief Hidden heading flag.
 */
#define HF_HIDE                 0x02

/*!
 * @brief Abbreviation word flag.
 */
#define WF_ABBREV               0x01

#ifdef __GNUC__
/*!
 * @brief Mark a function as taking printf-style arguments.
 */
#define PRINTF(x,y) __attribute__ ((format (printf, x, y)))
/*!
 * @brief Mark a function as never returning.
 */
#define NORETURN2 __attribute__ ((noreturn))
#else
#define PRINTF(x,y)
#define NORETURN2
#endif

/*!
 * @brief Unsigned byte type.
 */
typedef unsigned char uchar;

/*!
 * @brief Tag names recognized by the input parser.
 */
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

/*!
 * @brief Text styles.
 */
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

/*!
 * @brief Element kinds in the formatting element list.
 */
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

/*!
 * @brief Environment kinds.
 */
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

/*!
 * @brief Document languages.
 */
enum lang
{
  LANG_ENGLISH,
  LANG_GERMAN
};

/*!
 * @brief Syntax highlighting styles.
 */
enum syntax
{
  SYNTAX_EMX,
  SYNTAX_DVIDRV
};

/*!
 * @brief Per-backend replacement strings for a word.
 */
struct special
{
  uchar *text;                  /*!< Text replacement. */
  uchar *ipf;                   /*!< IPF replacement. */
  uchar *latex;                 /*!< LaTeX replacement. */
  uchar *html;                  /*!< HTML replacement. */
};

/*!
 * @brief A word in the global word table.
 */
struct word
{
  struct word *next;            /*!< Next word in hash bucket. */
  struct word *database;        /*!< Database (from xref) or NULL. */
  enum style style;             /*!< Formatting style. */
  uchar *str;                   /*!< The word text. */
  uchar *repl;                  /*!< Replacement text or NULL. */
  struct special *special;      /*!< Per-backend special text or NULL. */
  struct word_table *subidx;    /*!< Sub-index table or NULL. */
  int ref, idx;                 /*!< Reference and index IDs. */
  uchar flags;                  /*!< Flag bits, e.g. WF_ABBREV. */
};

/*!
 * @brief Hash table of words.
 */
struct word_table;

/*!
 * @brief One element in the current output element list.
 */
struct element
{
  enum el el;                   /*!< Element kind. */
  int n;                        /*!< Count, index, or reference. */
  struct word *wp;              /*!< Associated word. */
};

/*!
 * @brief One nested environment on the environment stack.
 */
struct environment
{
  enum env env;                 /*!< Environment kind. */
  int tmargin;                  /*!< Top margin. */
  int imargin;                  /*!< Inner margin. */
  int start_line;               /*!< Starting line number. */
  int counter;                  /*!< Item counter. */
};

/*!
 * @brief One entry in the table of contents.
 */
struct toc
{
  struct toc *next;             /*!< Next entry. */
  int level;                    /*!< Heading level. */
  int ref;                      /*!< Reference ID. */
  int print;                    /*!< Non-zero to print this entry. */
  int global;                   /*!< Non-zero if the section is global. */
  unsigned flags;               /*!< Heading flags. */
  uchar *number;                /*!< Section number or NULL. */
  uchar *title;                 /*!< Section title. */
};

/*!
 * @brief Character encodings.
 */
enum enc { ENC_DEFAULT = -3, ENC_AMBIGUOUS = -2, ENC_UNSUPPORTED = -1,
           ENC_CP850 = 0, ENC_ISO8859_1, ENCODINGS };

/*!
 * @brief The escape character.
 */
EXTERN uchar escape INIT (0xfe);

/*!
 * @brief The current input line number.
 */
EXTERN int line_no INIT (0);

/*!
 * @brief The current input line.
 */
EXTERN uchar input[256];

/*!
 * @brief The input encoding.
 */
EXTERN enum enc input_encoding INIT (ENC_DEFAULT);

/*!
 * @brief The output encoding.
 */
EXTERN enum enc output_encoding INIT (ENC_DEFAULT);

/*!
 * @brief The processing mode.
 */
EXTERN char mode INIT (0);

/*!
 * @brief Concatenate instead of call .inf files.
 */
EXTERN int opt_a INIT (FALSE);

/*!
 * @brief Select line break algorithm for text output.
 */
EXTERN int opt_b INIT (0);

/*!
 * @brief Use color instead of slanted type.
 */
EXTERN int opt_c INIT (FALSE);

/*!
 * @brief French spacing.
 */
EXTERN int opt_f INIT (FALSE);

/*!
 * @brief Gather global information.
 */
EXTERN int opt_g INIT (FALSE);

/*!
 * @brief Display warning messages.
 */
EXTERN int opt_w INIT (0);

/*!
 * @brief Name of the current input file.
 */
EXTERN const char *input_fname INIT (NULL);

/*!
 * @brief The input stream.
 */
EXTERN FILE *input_file INIT (NULL);

/*!
 * @brief Non-zero if the end of the input file has been reached.
 */
EXTERN int end_of_file INIT (FALSE);

/*!
 * @brief The output stream.
 */
EXTERN FILE *output_file INIT (NULL);

/*!
 * @brief The global table of words.
 */
EXTERN struct word_table *word_top INIT (NULL);

/*!
 * @brief The highlighting stack.
 */
EXTERN int hl_stack[HL_STACK_SIZE];
/*!
 * @brief Stack pointer for hl_stack.
 */
EXTERN int hl_sp;

/*!
 * @brief Current line number of the output file.
 */
EXTERN int output_line_no;

/*!
 * @brief Current column number of the output file.
 */
EXTERN int output_x;

/*!
 * @brief Non-zero when writing output file (second pass).
 */
EXTERN int out;

/*!
 * @brief Non-zero while formatting a prototype.
 */
EXTERN int prototype_flag INIT (FALSE);

/*!
 * @brief The stack of nested environments.
 */
EXTERN struct environment env_stack[ENV_STACK_SIZE];

/*!
 * @brief Stack pointer for the environment stack.
 */
EXTERN int env_sp;

/*!
 * @brief Current list of formatting elements.
 */
EXTERN struct element *elements INIT (NULL);

/*!
 * @brief Non-zero if a new paragraph should be started.
 */
EXTERN int para_flag;

/*!
 * @brief Number of functions.
 */
EXTERN int function_count;

/*!
 * @brief Next available reference number.
 */
EXTERN int ref_no INIT (1);

/*!
 * @brief Next available index number.
 */
EXTERN int idx_no INIT (0);

/*!
 * @brief The title of the document.
 */
EXTERN uchar *title INIT (NULL);

/*!
 * @brief The language of the document.
 */
EXTERN enum lang language INIT (LANG_ENGLISH);

/*!
 * @brief Current tag: the tag.
 */
EXTERN enum tag tg_tag;

/*!
 * @brief Current tag: style.
 */
EXTERN enum style tg_style;

/*!
 * @brief Current tag: level (for h-, h=, h1 through h3, i1, i2 and index).
 */
EXTERN int tg_level;

/*!
 * @brief Current tag: character for underlining (for h-, h= and h1 through h3).
 */
EXTERN int tg_underline;

/*!
 * @brief Current tag: flags (such as HF_UNNUMBERED).
 */
EXTERN unsigned tg_flags;


/*!
 * @brief Display an error message and stop.
 *
 * @param[in] fmt Format string. Not NULL.
 * @param[in] ... Format arguments.
 */
void fatal (const char *fmt, ...) NORETURN2 PRINTF (1, 2);
/*!
 * @brief Display an error message and set history to 1.
 *
 * @param[in] fmt Format string. Not NULL.
 * @param[in] ... Format arguments.
 */
void nonfatal (const char *fmt, ...) PRINTF (1, 2);
/*!
 * @brief Display a warning message if opt_w is at least @p level.
 *
 * @param[in] level Minimum warning level.
 * @param[in] fmt   Format string. Not NULL.
 * @param[in] ...   Format arguments.
 */
void warning (int level, const char *fmt, ...) PRINTF (2, 3);
/*!
 * @brief Allocate memory, quitting on failure.
 *
 * @param[in] n Number of bytes.
 *
 * @return Pointer to the allocated block.
 */
void *xmalloc (size_t n);
/*!
 * @brief Create a heap duplicate of a string, quitting on failure.
 *
 * @param[in] s String. Not NULL.
 *
 * @return Pointer to the duplicate.
 */
uchar *xstrdup (const uchar *s);

/*!
 * @brief Allocate a new word table.
 *
 * @param[in] hash_size Number of hash buckets.
 *
 * @return Pointer to the new table.
 */
struct word_table *wt_new (unsigned hash_size);
/*!
 * @brief Find a word in a table by hash.
 *
 * @param[in] wt   Word table. Not NULL.
 * @param[in] str  String to search for. Not NULL.
 * @param[in] hash Hash value of @p str.
 *
 * @return Pointer to the word, or NULL if not found.
 */
struct word *wt_find (struct word_table *wt, const uchar *str, unsigned hash);
/*!
 * @brief Compute the hash value of a string in a table.
 *
 * @param[in] wt  Word table. Not NULL.
 * @param[in] str String to hash. Not NULL.
 *
 * @return Hash value in the range 0..hash_size-1.
 */
unsigned wt_hash (struct word_table *wt, const uchar *str);
/*!
 * @brief Find or add a word in a table.
 *
 * @param[in] wt  Word table. Not NULL.
 * @param[in] str String. Not NULL.
 *
 * @return Pointer to the (possibly newly created) word.
 */
struct word *wt_add (struct word_table *wt, const uchar *str);
/*!
 * @brief Call a function for each word in a table.
 *
 * @param[in] wt       Word table. Not NULL.
 * @param[in] callback Function to call for each word. Not NULL.
 *
 * @return Result of the callback for the last word, or 0.
 */
int wt_walk (struct word_table *wt, int (*callback)(struct word *));
/*!
 * @brief Return the number of words in a table.
 *
 * @param[in] wt Word table. Not NULL.
 *
 * @return Number of words.
 */
int wt_count (const struct word_table *wt);

/*!
 * @brief Find a word in the global word table.
 *
 * @param[in] str  String. Not NULL.
 * @param[in] hash Hash value of @p str.
 *
 * @return Pointer to the word, or NULL if not found.
 */
struct word *word_find (const uchar *str, unsigned hash);
/*!
 * @brief Hash a string in the global word table.
 *
 * @param[in] str String. Not NULL.
 *
 * @return Hash value.
 */
unsigned word_hash (const uchar *str);
/*!
 * @brief Find or add a word in the global word table.
 *
 * @param[in] str String. Not NULL.
 *
 * @return Pointer to the (possibly newly created) word.
 */
struct word *word_add (const uchar *str);
/*!
 * @brief Call a function for each word in the global word table.
 *
 * @param[in] callback Function to call for each word. Not NULL.
 *
 * @return Result of the callback for the last word, or 0.
 */
int word_walk (int (*callback)(struct word *));

/*!
 * @brief Write a newline to the output file, tracking line length.
 */
void write_nl (void);
/*!
 * @brief Write a newline if the current line is not empty.
 */
void write_break (void);
/*!
 * @brief Write a string of given length to the output file.
 *
 * @param[in] p String. Must not contain a newline. Not NULL.
 * @param[in] n Length in bytes.
 */
void write_nstring (const uchar *p, size_t n);
/*!
 * @brief Write a NUL-terminated string to the output file.
 *
 * @param[in] p String. Must not contain a newline. Not NULL.
 */
void write_string (const uchar *p);
/*!
 * @brief Write a string followed by a newline.
 *
 * @param[in] p String. Must not contain a newline. Not NULL.
 */
void write_line (const uchar *p);
/*!
 * @brief Write a formatted string to the output file.
 *
 * @param[in] fmt Format string. Not NULL.
 * @param[in] ... Format arguments.
 */
void write_fmt (const char *fmt, ...) PRINTF (1, 2);
/*!
 * @brief Write a single space, breaking the line if it is too long.
 */
void write_space (void);
/*!
 * @brief Dispatch output of a string to the current backend.
 *
 * @param[in] p         String. Not NULL.
 * @param[in] may_break Non-zero if the backend may insert a line break.
 */
void format_output (const uchar *p, int may_break);
/*!
 * @brief Format a string with a given style.
 *
 * @param[in] p         String. Not NULL.
 * @param[in] sty       Style.
 * @param[in] may_break Non-zero if a line break may be inserted.
 */
void format_string (const uchar *p, int sty, int may_break);
/*!
 * @brief Format a run of spaces.
 *
 * @param[in] n         Number of spaces.
 * @param[in] style     Style.
 * @param[in] may_break Non-zero if a line break may be inserted.
 */
void format_spaces (int n, enum style style, int may_break);
/*!
 * @brief Build an element list from a string.
 *
 * @param[in] p Input string. Not NULL.
 */
void make_elements (const uchar *p);
/*!
 * @brief Push a highlighting attribute onto the stack.
 *
 * @param[in] hilite Highlight bit to push.
 */
void start_hilite (int hilite);
/*!
 * @brief Pop a highlighting attribute from the stack.
 */
void end_hilite (void);
/*!
 * @brief Resolve a reference to a document label.
 *
 * @param[in] p Label name. Not NULL.
 *
 * @return Pointer to the referenced word, or NULL if unresolved.
 */
struct word *use_reference (const uchar *p);
/*!
 * @brief Open an input file.
 *
 * @param[in] name File name. Not NULL.
 */
void open_input (const char *name);
/*!
 * @brief Read and preprocess the next input line.
 */
void read_line (void);
/*!
 * @brief Initialize state before processing a file.
 */
void init_file (void);
/*!
 * @brief Parse a tag at the start of a line.
 *
 * @param[in,out] ptr Pointer to the input pointer. Not NULL.
 *
 * @return Non-zero if a tag was parsed, zero otherwise.
 */
int parse_tag (const uchar **ptr);
/*!
 * @brief Emit the index heading.
 */
void start_index (void);
