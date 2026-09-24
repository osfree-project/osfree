/*! @file doxy-lint.c
 *  @brief Checks C/C++ source files for missing Doxygen documentation.
 *
 *  Style: Qt only.  Recognized forms:
 *    - leading:  /*! ... *\/   or  //! ...
 *    - trailing: /*!< ... *\/  or  //!< ...
 *  Trailing forms document the preceding declaration.  If a trailing
 *  block contains no command at all, its whole text is taken as the
 *  @brief value.  Javadoc style ("/**", "///") and Javadoc trailing
 *  forms ("/**<", "///<") are reported as wrong style.  Banner comments
 *  like "/*****" and "/////..." are treated as ordinary comments.
 *
 *  A doc block with @file is a file header; it is not merged into a
 *  following doc block.  Consecutive doc blocks without @file are
 *  treated as one documentation block for the next declaration, as
 *  Doxygen does.
 *
 *  Two-pass analysis.
 *
 *  Pass 1 reads the file exactly as written by the author.  Two kinds
 *  of information are collected here:
 *    - return statements, so that an identifier like TRUE or
 *      DLIST_SUCCESS is seen as an identifier and a numeric literal
 *      like 0 is seen as a number;
 *    - top-level #define directives that carry a body or parameters,
 *      because the preprocessor removes them from its output.
 *
 *  Pass 2 runs the file through the Open Watcom preprocessor
 *  (wcc386 for C, wpp386 for C++) with -pcl: preprocess, preserve
 *  comments, insert #line directives.  Macro expansions become
 *  visible (for instance an initializer macro such as INIT is no
 *  longer mistaken for a function name).  Only lines whose #line
 *  names the original file are kept; lines that came from an
 *  #include'd header are dropped.  Function and variable declarations
 *  are located in this pass.
 *
 *  The command line sent to the preprocessor is built from the raw
 *  command line of doxy-lint itself, obtained with getcmd().  The
 *  tokens that begin with '-' are forwarded verbatim, quotes and all,
 *  so that a value such as -dVER_DATE="2026-08-27" keeps its quotes
 *  and the hyphens inside the value are not mistaken for separate
 *  options.  On Linux system() runs /bin/sh, which would strip a
 *  double quote as a grouping character before wcc386 sees it; each
 *  double quote in the option text is therefore escaped with a
 *  backslash on that target only.  On Win32 and OS/2 cmd.exe does not
 *  use \ as an escape, so the option text is passed through unchanged.
 *  Set DOXY_LINT_DEBUG=1 to print the command line before running it.
 *
 *  Before invoking the preprocessor, the current directory is changed
 *  to the directory of the source file, so that relative -i= options
 *  and #include "..." behave exactly as in the project's makefiles.
 *  Temporary files (.i and .err) are written to $TMPDIR / %TEMP% /
 *  %TMP% and always removed; nothing is ever written into the current
 *  or source directory.
 *
 *  Enforces:
 *    - @file   required in every source and header file;
 *    - @brief  required before (or trailing after) every top-level
 *             declaration;
 *    - @def   required in the doc block of every #define that has a
 *             body or parameters; the argument of @def must match the
 *             macro name.  Object-like macros without a body are not
 *             checked: include guards and similar markers carry no
 *             value worth documenting;
 *    - @param  required for every named parameter of a function-like
 *             macro (#define NAME(p1, p2) ...), even when its body is
 *             empty;
 *    - @param  required for every named function parameter;
 *    - direction qualifier [in]/[out]/[in,out] is mandatory for every
 *             @param;
 *    - @return required for every non-void function;
 *    - @retval required for every simple value returned by a body.
 *             A value is an integer literal, a signed integer literal,
 *             or an all-uppercase identifier (the project convention
 *             for return constants).  Function calls and other complex
 *             expressions are not checked.  @retval is not applicable
 *             to void functions and is reported as an error.
 *
 *  A function is considered to return void when the keyword "void"
 *  occurs in the return type and is not followed by "*".
 *
 *  Usage:
 *    doxy-lint <file>... [preprocessor options...]
 *
 *  Every argument that begins with '-' is passed through to the
 *  Watcom preprocessor unchanged.  All other arguments are treated as
 *  source files and checked in order.
 *
 *  Diagnostics go to stdout.  Exit status is always 0.
 *
 *  Targets: Linux, Win32, OS/2.  Compiler: Open Watcom 1.9.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <process.h>

#if defined(__LINUX__)
#  include <unistd.h>
#  include <sys/types.h>
#  include <sys/wait.h>
#  define DIRSEP '/'
#  define PLATFORM_CHDIR(p)     chdir(p)
#  define PLATFORM_GETCWD(b,s)  getcwd(b,s)
#elif defined(__NT__) || defined(__OS2__)
#  include <direct.h>
#  define DIRSEP '\\'
#  define PLATFORM_CHDIR(p)     _chdir(p)
#  define PLATFORM_GETCWD(b,s)  _getcwd(b,s)
#else
#  error "Unsupported target: expected Linux, Win32 or OS/2"
#endif

#define MAXTOK       65536
#define MAXPARAM     128
#define MAXRET       128
#define NAMELEN      64
#define EXPRLEN      128
#define PATHBUF      1024
#define PRELINE      8192
#define MAX_LINE_NO  65536

#define T_EOF           0
#define T_IDENT         1
#define T_NUMBER        2
#define T_STRING        3
#define T_CHARLIT       4
#define T_PUNCT         5
#define T_DOC           6
#define T_COMMENT       7
#define T_PREPROC       8
#define T_DOC_JAVADOC   9
#define T_DOC_TRAILING 10

/*! @brief One lexical token: type, text position and source line. */
typedef struct {
    int type;
    int line;
    const char *p;
    int len;
} Token;

/*! @brief Array of all tokens of the current file. */
static Token toks[MAXTOK];

/*! @brief Number of valid entries in toks[]. */
static int ntok;

/*! @brief Contents of the current file (NUL-terminated). */
static char *src;

/*! @brief Length of src in bytes, excluding the terminator. */
static long srclen;

/*! @brief Non-zero after the one-time preprocessing warning. */
static int wpp_warned = 0;

/*! @brief Raw option text of doxy-lint's own command line.
 *
 * Filled once in main() from getcmd().  Contains the tokens of the
 * original command line that begin with '-', copied verbatim, quotes
 * and all, separated by single spaces.  Forwarded to the preprocessor
 * so that the characters the user typed reach Watcom unchanged.
 */
static char raw_opts[PRELINE];

/*! @brief Simple return value of the raw file, indexed by line.
 *
 * Filled during pass 1 from the file exactly as written.  NULL when
 * the line has no simple return value.  Only one value is kept per
 * line; a second simple return on the same line is ignored.
 */
static char *raw_returns[MAX_LINE_NO];

/*! @brief Counter used to build unique temporary file names. */
static int temp_counter = 0;

/*! @brief Copies the option tokens of a raw command line into out.
 *
 * Walks raw, splitting it into tokens with the rules the C run-time
 * uses: whitespace separates tokens, a double quote starts a grouped
 * token, a backslash escapes the next character inside a group.  A
 * token that begins with '-' is an option; it is copied into out
 * verbatim, including any inner double quotes.  Tokens that do not
 * begin with '-' are ignored.  Copied tokens are separated by a
 * single space.
 *
 *  @param[in]  raw     Raw command line, without the program name.
 *  @param[out] out     Receiver for the option tokens.
 *  @param[in]  outsize Size of out in bytes.
 */
static void extract_raw_opts(const char *raw, char *out, size_t outsize)
{
    const char *p = raw;
    size_t outlen = 0;

    out[0] = 0;

    while (*p != 0) {
        const char *tok_start;
        size_t tok_len;

        while (*p == ' ' || *p == '\t') p++;
        if (*p == 0) break;

        tok_start = p;
        while (*p != 0) {
            if (*p == '"') {
                p++;
                while (*p != 0 && *p != '"') {
                    if (*p == '\\' && p[1] != 0) p++;
                    p++;
                }
                if (*p == '"') p++;
            } else if (*p == ' ' || *p == '\t') {
                break;
            } else {
                p++;
            }
        }
        tok_len = (size_t)(p - tok_start);

        if (tok_start[0] == '-') {
            if (outlen + tok_len + 2 > outsize) break;
            if (outlen > 0) out[outlen++] = ' ';
            memcpy(out + outlen, tok_start, tok_len);
            outlen += tok_len;
            out[outlen] = 0;
        }
    }
}

/*! @brief Escapes a raw option string for the target command processor.
 *
 * Linux: system() runs /bin/sh, which treats a double quote as a
 * grouping character and strips it before the child process sees it.
 * Every double quote in the option text is therefore rewritten as \"
 * so that a literal quote reaches wcc386 or wpp386.  Win32 and OS/2:
 * cmd.exe does not use backslash as an escape character, so the
 * option text is passed through unchanged.
 *
 *  @param[in]  in      Raw option text.
 *  @param[out] out     Receiver for the escaped text.
 *  @param[in]  outsize Size of out in bytes.
 */
static void escape_raw_opts(const char *in, char *out, size_t outsize)
{
    size_t i, j = 0;

    for (i = 0; in[i] != 0 && j + 2 < outsize; i++) {
#if defined(__LINUX__)
        if (in[i] == '"') out[j++] = '\\';
#endif
        out[j++] = in[i];
    }
    out[j] = 0;
}

/*! @brief Returns the platform temp directory, or 0 on failure.
 *
 * Linux: $TMPDIR, otherwise /tmp.  Win32 / OS-2: %TEMP%, otherwise
 * %TMP%.  Never falls back to the current directory.
 *
 *  @param[out] buf     Receiver for the directory path.
 *  @param[in]  bufsize Size of buf in bytes.
 *  @return Non-zero on success.
 *  @retval 0 No temp directory available, or the path does not fit.
 *  @retval 1 Directory path copied into buf.
 */
static int get_temp_dir(char *buf, size_t bufsize)
{
#if defined(__LINUX__)
    const char *t = getenv("TMPDIR");
    if (t == NULL || t[0] == 0) t = "/tmp";
    if (strlen(t) + 1 > bufsize) return 0;
    strcpy(buf, t);
    return 1;
#else
    const char *t = getenv("TEMP");
    if (t == NULL || t[0] == 0) t = getenv("TMP");
    if (t == NULL || t[0] == 0) return 0;
    if (strlen(t) + 1 > bufsize) return 0;
    strcpy(buf, t);
    return 1;
#endif
}

/*! @brief Turns a path into an absolute one using the current directory.
 *
 *  @param[in]  path    Input path. Not NULL.
 *  @param[out] out     Receiver for the absolute path.
 *  @param[in]  outsize Size of out in bytes.
 *  @return Non-zero on success.
 *  @retval 0 Path could not be made absolute, or does not fit.
 *  @retval 1 Absolute path copied into out.
 */
static int make_absolute(const char *path, char *out, size_t outsize)
{
    char cwd[PATHBUF];
    size_t clen, plen;

    if (path[0] == '\0') return 0;
    if (path[0] == '/' || path[0] == '\\') {
        if (strlen(path) + 1 > outsize) return 0;
        strcpy(out, path);
        return 1;
    }
    if (isalpha((unsigned char)path[0]) && path[1] == ':') {
        if (strlen(path) + 1 > outsize) return 0;
        strcpy(out, path);
        return 1;
    }
    if (PLATFORM_GETCWD(cwd, PATHBUF) == NULL) return 0;
    clen = strlen(cwd);
    plen = strlen(path);
    if (clen + 1 + plen + 1 > outsize) return 0;
    memcpy(out, cwd, clen);
    if (clen > 0 && cwd[clen - 1] != '/' && cwd[clen - 1] != '\\') {
        out[clen++] = DIRSEP;
    }
    memcpy(out + clen, path, plen + 1);
    return 1;
}

/*! @brief Appends a token to the global toks[] array.
 *  @param[in] type Token type constant.
 *  @param[in] p    Pointer into the src buffer.
 *  @param[in] len  Token length in bytes.
 *  @param[in] line Source line where the token starts.
 */
static void add_tok(int type, const char *p, int len, int line)
{
    if (ntok >= MAXTOK) {
        printf("doxy-lint: too many tokens, giving up\n");
        exit(0);
    }
    toks[ntok].type = type;
    toks[ntok].p = p;
    toks[ntok].len = len;
    toks[ntok].line = line;
    ntok++;
}

/*! @brief Returns the length of the punctuator starting at p.
 *  @param[in] p Pointer to the first character of the punctuator.
 *  @return Number of bytes consumed.
 *  @retval 0 End of string.
 *  @retval 1 Single-character punctuator.
 *  @retval 2 Two-character punctuator.
 *  @retval 3 Ellipsis "...".
 */
static int punct_len(const char *p)
{
    static const char *two[] = {
        "->", "++", "--", "==", "!=", "<=", ">=", "&&", "||",
        "<<", ">>", "+=", "-=", "*=", "/=", "%=", "&=", "|=", "^=",
        "::",
        NULL
    };
    int i;

    if (p[0] == '\0') return 0;
    if (p[0] == '.' && p[1] == '.' && p[2] == '.') return 3;
    if (p[1] == '\0') return 1;
    for (i = 0; two[i] != NULL; i++) {
        if (p[0] == two[i][0] && p[1] == two[i][1]) return 2;
    }
    return 1;
}

/*! @brief Splits the global src buffer into toks[]. */
static void tokenize(void)
{
    long i = 0;
    int line = 1;
    int line_start = 1;

    while (i < srclen) {
        char c = src[i];

        if (c == '\n') { line++; i++; line_start = 1; continue; }
        if (c == ' ' || c == '\t' || c == '\r' || c == '\f' || c == '\v') {
            i++;
            continue;
        }

        if (c == '#' && line_start) {
            long start = i;
            int sl = line;
            while (i < srclen) {
                if (src[i] == '\n') {
                    long j = i - 1;
                    while (j > start &&
                           (src[j] == ' ' || src[j] == '\t' || src[j] == '\r')) j--;
                    if (j >= start && src[j] == '\\') {
                        line++;
                        i++;
                        while (i < srclen && (src[i] == ' ' || src[i] == '\t')) i++;
                        continue;
                    }
                    break;
                }
                i++;
            }
            add_tok(T_PREPROC, src + start, (int)(i - start), sl);
            line_start = 1;
            continue;
        }

        line_start = 0;

        if (c == '/' && i + 1 < srclen && src[i + 1] == '/') {
            int sl = line;
            int kind = 0;
            long cs;

            if (i + 2 < srclen && src[i + 2] == '!') {
                if (i + 3 < srclen && src[i + 3] == '<') kind = 2;
                else kind = 1;
            } else if (i + 2 < srclen && src[i + 2] == '/') {
                if (i + 3 < srclen && src[i + 3] == '/') kind = 0;
                else kind = 3;
            }
            i += 2;
            if (kind == 1 || kind == 2) i++;
            cs = i;
            while (i < srclen && src[i] != '\n') i++;
            if (kind == 1)
                add_tok(T_DOC, src + cs, (int)(i - cs), sl);
            else if (kind == 2)
                add_tok(T_DOC_TRAILING, src + cs, (int)(i - cs), sl);
            else if (kind == 3)
                add_tok(T_DOC_JAVADOC, src + cs, (int)(i - cs), sl);
            else
                add_tok(T_COMMENT, src + cs, (int)(i - cs), sl);
            continue;
        }

        if (c == '/' && i + 1 < srclen && src[i + 1] == '*') {
            int sl = line;
            int kind = 0;
            long cs, ce;

            if (i + 2 < srclen && src[i + 2] == '!') {
                if (i + 3 < srclen && src[i + 3] == '<') kind = 2;
                else kind = 1;
            } else if (i + 2 < srclen && src[i + 2] == '*') {
                if (i + 3 >= srclen) kind = 0;
                else if (src[i + 3] == '/') kind = 0;
                else if (src[i + 3] == '*') kind = 0;
                else kind = 3;
            }
            i += 2;
            if (kind == 1 || kind == 2) i++;
            cs = i;
            while (i + 1 < srclen && !(src[i] == '*' && src[i + 1] == '/')) {
                if (src[i] == '\n') line++;
                i++;
            }
            ce = i;
            if (i + 1 < srclen) i += 2;
            else i = srclen;
            if (kind == 1)
                add_tok(T_DOC, src + cs, (int)(ce - cs), sl);
            else if (kind == 2)
                add_tok(T_DOC_TRAILING, src + cs, (int)(ce - cs), sl);
            else if (kind == 3)
                add_tok(T_DOC_JAVADOC, src + cs, (int)(ce - cs), sl);
            else
                add_tok(T_COMMENT, src + cs, (int)(ce - cs), sl);
            continue;
        }

        if (c == '*' && i + 1 < srclen && src[i + 1] == '/') {
            i += 2;
            continue;
        }

        if (c == '"' || c == '\'') {
            long start = i;
            char q = c;
            int sl = line;
            i++;
            while (i < srclen && src[i] != q) {
                if (src[i] == '\\' && i + 1 < srclen) {
                    if (src[i + 1] == '\n') line++;
                    i += 2;
                    continue;
                }
                if (src[i] == '\n') line++;
                i++;
            }
            if (i < srclen) i++;
            add_tok(q == '"' ? T_STRING : T_CHARLIT,
                    src + start, (int)(i - start), sl);
            continue;
        }

        if (isalpha((unsigned char)c) || c == '_') {
            long start = i;
            int sl = line;
            while (i < srclen &&
                   (isalnum((unsigned char)src[i]) || src[i] == '_')) i++;
            add_tok(T_IDENT, src + start, (int)(i - start), sl);
            continue;
        }

        if (isdigit((unsigned char)c) ||
            (c == '.' && i + 1 < srclen &&
             isdigit((unsigned char)src[i + 1]))) {
            long start = i;
            int sl = line;
            while (i < srclen &&
                   (isalnum((unsigned char)src[i]) || src[i] == '.' ||
                    src[i] == '_')) i++;
            add_tok(T_NUMBER, src + start, (int)(i - start), sl);
            continue;
        }

        {
            int pl = punct_len(src + i);
            add_tok(T_PUNCT, src + i, pl, line);
            i += pl;
        }
    }
    add_tok(T_EOF, src + srclen, 0, line);
}

/*! @brief Tests whether token i is the identifier s.
 *  @param[in] i Index into toks[].
 *  @param[in] s Expected identifier text.
 *  @return Non-zero on match, zero otherwise.
 *  @retval 0 Token is not the given identifier.
 *  @retval 1 Token is the given identifier.
 */
static int tok_is(int i, const char *s)
{
    int len = (int)strlen(s);
    if (toks[i].type != T_IDENT) return 0;
    if (toks[i].len != len) return 0;
    return memcmp(toks[i].p, s, len) == 0;
}

/*! @brief Tests whether token i is the punctuator s.
 *  @param[in] i Index into toks[].
 *  @param[in] s Expected punctuator text.
 *  @return Non-zero on match, zero otherwise.
 *  @retval 0 Token is not the given punctuator.
 *  @retval 1 Token is the given punctuator.
 */
static int tok_punct(int i, const char *s)
{
    int len = (int)strlen(s);
    if (toks[i].type != T_PUNCT) return 0;
    if (toks[i].len != len) return 0;
    return memcmp(toks[i].p, s, len) == 0;
}

/*! @brief Tests whether a word is a C or C++ keyword.
 *  @param[in] s   Pointer to the word text.
 *  @param[in] len Word length in bytes.
 *  @return Non-zero for a keyword.
 *  @retval 0 Not a keyword.
 *  @retval 1 Keyword.
 */
static int is_keyword(const char *s, int len)
{
    static const char *kw[] = {
        "auto", "break", "case", "char", "const", "continue", "default",
        "do", "double", "else", "enum", "extern", "float", "for", "goto",
        "if", "int", "long", "register", "return", "short", "signed",
        "sizeof", "static", "struct", "switch", "typedef", "union",
        "unsigned", "void", "volatile", "while",
        "inline", "restrict", "_Bool", "_Complex", "_Imaginary",
        "class", "namespace", "template", "typename", "public",
        "private", "protected", "virtual", "operator", "new", "delete",
        "this", "try", "catch", "throw", "using", "friend", "explicit",
        "mutable", "wchar_t", "bool", "true", "false",
        NULL
    };
    int i;
    for (i = 0; kw[i] != NULL; i++) {
        if ((int)strlen(kw[i]) == len && memcmp(kw[i], s, len) == 0) return 1;
    }
    return 0;
}

/*! @brief Tests whether an identifier looks like a return constant.
 *
 * True when every character is an uppercase letter, a digit or an
 * underscore, and at least one letter is present.
 *
 *  @param[in] s   Pointer to the identifier text.
 *  @param[in] len Length in bytes.
 *  @return Non-zero when the identifier is an uppercase constant.
 *  @retval 0 Not an uppercase return constant.
 *  @retval 1 Uppercase return constant.
 */
static int is_upper_constant(const char *s, int len)
{
    int i;
    int has_letter = 0;

    if (len <= 0) return 0;
    for (i = 0; i < len; i++) {
        unsigned char c = (unsigned char)s[i];
        if (c >= 'A' && c <= 'Z') { has_letter = 1; continue; }
        if (c >= '0' && c <= '9') continue;
        if (c == '_') continue;
        return 0;
    }
    return has_letter;
}

/*! @brief Checks whether the return type is plain "void".
 *
 * A function returns void when the keyword "void" appears among the
 * tokens of its return type and is not followed by "*".
 *
 *  @param[in] start    Index of the first token of the return type.
 *  @param[in] name_idx Index of the function-name token.
 *  @return Non-zero if the return type is plain void.
 *  @retval 0 Return type is not plain void.
 *  @retval 1 Return type is plain void.
 */
static int returns_void(int start, int name_idx)
{
    int j;
    int saw_void = 0;
    int star_after_void = 0;

    for (j = start; j < name_idx; j++) {
        if (tok_is(j, "void")) { saw_void = 1; continue; }
        if (saw_void && tok_punct(j, "*")) { star_after_void = 1; }
    }
    return saw_void && !star_after_void;
}

/*! @brief Skips a balanced pair (open ... close).
 *  @param[in] open_idx  Index of the opening token.
 *  @param[in] open_str  Text of the opening punctuator.
 *  @param[in] close_str Text of the closing punctuator.
 *  @return Index of the first token after the closing one.
 */
static int skip_balanced(int open_idx, const char *open_str,
                         const char *close_str)
{
    int depth = 0;
    int i = open_idx;
    while (toks[i].type != T_EOF) {
        if (tok_punct(i, open_str)) depth++;
        else if (tok_punct(i, close_str)) {
            depth--;
            if (depth == 0) return i + 1;
        }
        i++;
    }
    return i;
}

/*! @brief Information extracted from one Doxygen comment. */
typedef struct {
    int has_file;
    int has_brief;
    int has_def;
    char def_name[NAMELEN];
    char params[MAXPARAM][NAMELEN];
    int  paramdir[MAXPARAM];
    int nparams;
    int has_return;
    char retvals[MAXRET][NAMELEN];
    int nretvals;
} DocInfo;

/*! @brief Parses the body of a Doxygen comment into info.
 *  @param[in]     p           Comment text without delimiters.
 *  @param[in]     len         Length of the comment text in bytes.
 *  @param[in,out] info        Destination for @file/@brief/@def/@param/@return/@retval.
 *  @param[in]     is_trailing Non-zero when the comment is a "/*!<" or "//!<"
 *                             trailing block.  In that case, if no command at
 *                             all is present, the whole text is treated as the
 *                             value of @brief.
 */
static void parse_doc_into(const char *p, int len, DocInfo *info,
                           int is_trailing)
{
    int i = 0;
    int at_line_start = 1;

    while (i < len) {
        char c = p[i];
        int cs, clen, as, alen;
        int cur_dir = 0;

        if (c == '\n') {
            at_line_start = 1;
            i++;
            continue;
        }
        if (c == ' ' || c == '\t' || c == '\r') {
            i++;
            continue;
        }
        if (c == '*' && at_line_start) {
            i++;
            continue;
        }

        if ((c == '@' || c == '\\') && at_line_start) {
            at_line_start = 0;
            i++;
            cs = i;
            while (i < len && isalpha((unsigned char)p[i])) i++;
            clen = i - cs;

            while (i < len && (p[i] == ' ' || p[i] == '\t')) i++;

            if (clen == 5 && memcmp(p + cs, "param", 5) == 0) {
                if (i < len && p[i] == '[') {
                    int bs = i + 1;
                    int be, jj;
                    int has_in = 0, has_out = 0;
                    while (i < len && p[i] != ']') i++;
                    be = i;
                    if (i < len) i++;
                    while (i < len && (p[i] == ' ' || p[i] == '\t')) i++;
                    for (jj = bs; jj < be; jj++) {
                        if (jj + 1 < be &&
                            p[jj] == 'i' && p[jj + 1] == 'n') has_in = 1;
                        if (jj + 2 < be &&
                            p[jj] == 'o' && p[jj + 1] == 'u' &&
                            p[jj + 2] == 't') has_out = 1;
                    }
                    if (has_in && has_out) cur_dir = 3;
                    else if (has_in) cur_dir = 1;
                    else if (has_out) cur_dir = 2;
                }
            }

            as = i;
            while (i < len && !isspace((unsigned char)p[i]) && p[i] != '*') i++;
            alen = i - as;

            if (clen == 4 && memcmp(p + cs, "file", 4) == 0) {
                info->has_file = 1;
            } else if (clen == 5 && memcmp(p + cs, "brief", 5) == 0) {
                info->has_brief = 1;
            } else if (clen == 3 && memcmp(p + cs, "def", 3) == 0) {
                info->has_def = 1;
                if (alen > 0 && alen < NAMELEN) {
                    memcpy(info->def_name, p + as, alen);
                    info->def_name[alen] = 0;
                }
            } else if (clen == 5 && memcmp(p + cs, "param", 5) == 0) {
                if (alen > 0 && alen < NAMELEN && info->nparams < MAXPARAM) {
                    memcpy(info->params[info->nparams], p + as, alen);
                    info->params[info->nparams][alen] = 0;
                    info->paramdir[info->nparams] = cur_dir;
                    info->nparams++;
                }
            } else if (clen == 6 && memcmp(p + cs, "return", 6) == 0) {
                info->has_return = 1;
            } else if (clen == 6 && memcmp(p + cs, "retval", 6) == 0) {
                if (alen > 0 && alen < NAMELEN && info->nretvals < MAXRET) {
                    memcpy(info->retvals[info->nretvals], p + as, alen);
                    info->retvals[info->nretvals][alen] = 0;
                    info->nretvals++;
                }
            }
            continue;
        }

        at_line_start = 0;
        while (i < len && p[i] != '\n') i++;
    }

    if (is_trailing &&
        !info->has_file && !info->has_brief && !info->has_def &&
        info->nparams == 0 && !info->has_return && info->nretvals == 0) {
        info->has_brief = 1;
    }
}

/*! @brief Extracts the name of one parameter and appends it to names.
 *
 * A parameter that is a bare type name (a single identifier such as
 * "phStack", or a type plus "*" with no declarator) has no parameter
 * name and is skipped silently.
 *
 *  @param[in]     start  Index of the first token of the parameter.
 *  @param[in]     end    Index just past the last token of the parameter.
 *  @param[in,out] names  Array receiving the parameter names.
 *  @param[in,out] nnames In/out counter of names already stored.
 */
static void extract_one_param(int start, int end,
                              char names[][NAMELEN], int *nnames)
{
    int j;
    int name_idx = -1;
    int in_tag = 0;
    int bdepth = 0;
    int n_nonkw = 0;
    int n_other = 0;

    if (end <= start) return;
    if (end - start == 1 && tok_is(start, "void")) return;
    if (end - start == 1 && tok_punct(start, "...")) return;
    if (end - start == 3 &&
        tok_punct(start, ".") && tok_punct(start + 1, ".") &&
        tok_punct(start + 2, ".")) return;

    for (j = start; j < end; j++) {
        if (tok_punct(j, "[")) { bdepth++; continue; }
        if (tok_punct(j, "]")) { if (bdepth > 0) bdepth--; continue; }
        if (bdepth > 0) continue;
        if (tok_punct(j, "*") || tok_punct(j, "(") || tok_punct(j, ")")) {
            continue;
        }
        if (toks[j].type != T_IDENT) {
            n_other++;
            continue;
        }
        if (tok_is(j, "struct") || tok_is(j, "union") || tok_is(j, "enum") ||
            tok_is(j, "class") || tok_is(j, "namespace")) {
            in_tag = 1;
            n_other++;
            continue;
        }
        if (in_tag) { in_tag = 0; n_other++; continue; }
        if (is_keyword(toks[j].p, toks[j].len)) { n_other++; continue; }
        name_idx = j;
        n_nonkw++;
    }

    if (n_nonkw == 1 && n_other == 0) {
        name_idx = -1;
    }

    if (name_idx >= 0 && *nnames < MAXPARAM) {
        int len = toks[name_idx].len;
        if (len >= NAMELEN) len = NAMELEN - 1;
        memcpy(names[*nnames], toks[name_idx].p, len);
        names[*nnames][len] = 0;
        (*nnames)++;
    }
}

/*! @brief Extracts all parameter names from a (...) group.
 *  @param[in]     open_idx  Index of the '(' token, or -1.
 *  @param[in]     close_idx Index of the matching ')'.
 *  @param[in,out] names     Array receiving the parameter names.
 *  @param[in,out] nnames    In/out counter of names already stored.
 */
static void extract_params(int open_idx, int close_idx,
                           char names[][NAMELEN], int *nnames)
{
    int j, pdepth, start;

    if (open_idx < 0 || close_idx < 0) return;

    pdepth = 0;
    start = open_idx + 1;
    j = start;
    while (j < close_idx) {
        if (tok_punct(j, "(") || tok_punct(j, "[")) pdepth++;
        else if (tok_punct(j, ")") || tok_punct(j, "]")) {
            if (pdepth > 0) pdepth--;
        } else if (tok_punct(j, ",") && pdepth == 0) {
            extract_one_param(start, j, names, nnames);
            start = j + 1;
        }
        j++;
    }
    if (start < close_idx) extract_one_param(start, close_idx, names, nnames);
}

/*! @brief Frees all recorded raw return values. */
static void raw_returns_reset(void)
{
    int i;
    for (i = 0; i < MAX_LINE_NO; i++) {
        if (raw_returns[i] != NULL) {
            free(raw_returns[i]);
            raw_returns[i] = NULL;
        }
    }
}

/*! @brief Collects simple return values from the raw file.
 *
 * Assumes that src, srclen, toks and ntok already hold the raw file
 * (pass 1).  For every return statement whose value is an integer
 * literal, a signed integer literal, or an all-uppercase identifier,
 * stores the textual value in raw_returns[line].  A second simple
 * return on the same line is ignored.
 */
static void collect_raw_returns(void)
{
    int i;

    for (i = 0; i < ntok; i++) {
        int j, len;
        int line;
        char buf[EXPRLEN];

        if (toks[i].type != T_IDENT) continue;
        if (!tok_is(i, "return")) continue;
        j = i + 1;
        if (j >= ntok) continue;

        if (toks[j].type == T_NUMBER) {
            len = toks[j].len;
            if (len >= EXPRLEN) len = EXPRLEN - 1;
            memcpy(buf, toks[j].p, len);
            buf[len] = 0;
        } else if (toks[j].type == T_PUNCT && toks[j].len == 1 &&
                   (toks[j].p[0] == '-' || toks[j].p[0] == '+') &&
                   j + 1 < ntok && toks[j + 1].type == T_NUMBER) {
            len = toks[j + 1].len;
            if (len + 1 >= EXPRLEN) len = EXPRLEN - 2;
            buf[0] = toks[j].p[0];
            memcpy(buf + 1, toks[j + 1].p, len);
            buf[len + 1] = 0;
        } else if (toks[j].type == T_IDENT &&
                   is_upper_constant(toks[j].p, toks[j].len)) {
            len = toks[j].len;
            if (len >= EXPRLEN) len = EXPRLEN - 1;
            memcpy(buf, toks[j].p, len);
            buf[len] = 0;
        } else {
            continue;
        }

        line = toks[i].line;
        if (line > 0 && line < MAX_LINE_NO && raw_returns[line] == NULL) {
            size_t blen = strlen(buf);
            raw_returns[line] = (char *)malloc(blen + 1);
            if (raw_returns[line] != NULL) {
                memcpy(raw_returns[line], buf, blen + 1);
            }
        }
    }
}

/*! @brief Collects simple return values of a function body.
 *
 * The line range of the body (identified in pass 2) is used to look
 * up return values collected from the raw file in pass 1.
 *
 *  @param[in]     body_start Index of the opening '{' of the body.
 *  @param[in]     body_end   Index of the matching '}'.
 *  @param[in,out] exprs      Array receiving the value strings.
 *  @param[in,out] nexprs     In/out counter of values already stored.
 */
static void collect_returns(int body_start, int body_end,
                            char exprs[][EXPRLEN], int *nexprs)
{
    int L, start_line, end_line;

    if (body_start < 0 || body_end < 0) return;
    start_line = toks[body_start].line;
    end_line = toks[body_end].line;

    for (L = start_line; L <= end_line; L++) {
        int m, dup;
        if (L <= 0 || L >= MAX_LINE_NO) continue;
        if (raw_returns[L] == NULL) continue;
        dup = 0;
        for (m = 0; m < *nexprs; m++) {
            if (strcmp(exprs[m], raw_returns[L]) == 0) { dup = 1; break; }
        }
        if (!dup && *nexprs < MAXRET) {
            strcpy(exprs[*nexprs], raw_returns[L]);
            (*nexprs)++;
        }
    }
}

/*! @brief Reads the whole file fname into a heap buffer.
 *  @param[in]  fname   Path to the file.
 *  @param[out] out_len Receives the number of bytes read.
 *  @return Pointer to the buffer, or NULL on error.
 *  @retval NULL File could not be opened or memory could not be allocated.
 *  @retval buf  Buffer containing the file contents.
 */
static char *slurp(const char *fname, long *out_len)
{
    FILE *fp;
    char *buf;
    long sz;
    size_t n;

    fp = fopen(fname, "rb");
    if (fp == NULL) return NULL;
    fseek(fp, 0, SEEK_END);
    sz = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (sz < 0) sz = 0;
    buf = (char *)malloc((size_t)sz + 1);
    if (buf == NULL) { fclose(fp); return NULL; }
    n = fread(buf, 1, (size_t)sz, fp);
    fclose(fp);
    buf[n] = 0;
    *out_len = (long)n;
    return buf;
}

/*! @brief Returns the base name of a path.
 *
 * Both '/' and '\\' are treated as separators.
 *
 *  @param[in] p Path text. Not NULL.
 *  @return Pointer to the base name within p.
 */
static const char *base_name(const char *p)
{
    const char *last = p;
    while (*p != 0) {
        if (*p == '/' || *p == '\\') last = p + 1;
        p++;
    }
    return last;
}

/*! @brief Case-insensitive comparison of two base names.
 *
 * Treats '/' and '\\' as equivalent and folds ASCII case.
 *
 *  @param[in] a First name. Not NULL.
 *  @param[in] b Second name. Not NULL.
 *  @return Non-zero when the base names match.
 *  @retval 0 Names differ.
 *  @retval 1 Names match.
 */
static int path_names_match(const char *a, const char *b)
{
    const char *ba = base_name(a);
    const char *bb = base_name(b);
    while (*ba != 0 && *bb != 0) {
        char ca = *ba;
        char cb = *bb;
        if (ca >= 'A' && ca <= 'Z') ca = (char)(ca - 'A' + 'a');
        if (cb >= 'A' && cb <= 'Z') cb = (char)(cb - 'A' + 'a');
        if (ca != cb) return 0;
        ba++;
        bb++;
    }
    return *ba == 0 && *bb == 0;
}

/*! @brief Selects the Open Watcom compiler to preprocess a file.
 *
 * Returns "wpp386" for C++ source extensions, "wcc386" for everything
 * else.
 *
 *  @param[in] fname Source file name. Not NULL.
 *  @return Compiler name to use.
 */
static const char *preproc_compiler(const char *fname)
{
    const char *dot = strrchr(fname, '.');
    if (dot != NULL) {
        if (strcmp(dot, ".cpp") == 0 || strcmp(dot, ".cxx") == 0 ||
            strcmp(dot, ".cc") == 0 || strcmp(dot, ".c++") == 0 ||
            strcmp(dot, ".C") == 0 || strcmp(dot, ".CPP") == 0 ||
            strcmp(dot, ".CXX") == 0 || strcmp(dot, ".CC") == 0 ||
            strcmp(dot, ".inl") == 0 || strcmp(dot, ".ipp") == 0 ||
            strcmp(dot, ".tcc") == 0 || strcmp(dot, ".tpp") == 0)
            return "wpp386";
    }
    return "wcc386";
}

/*! @brief Extracts the macro name and parameters from a #define token.
 *
 * The token text begins with '#', possibly followed by whitespace,
 * then "define", then whitespace, then the macro name.  For a
 * function-like macro the name is followed by '(' and a
 * comma-separated list of parameter names; each name is copied into
 * params[].  Variadic macros are handled: the "..." is skipped and
 * does not produce a parameter.
 *
 * An object-like macro without a body is skipped: include guards and
 * similar markers carry no value worth documenting.  A function-like
 * macro with parameters is checked even when its body is empty: the
 * parameter list itself is part of the interface.  Other directives
 * ('#include', '#ifdef', ...) also return 0.
 *
 *  @param[in]  p       Preprocessor token text.
 *  @param[in]  len     Token length in bytes.
 *  @param[out] name    Buffer for the macro name.
 *  @param[in]  size    Size of name in bytes.
 *  @param[out] params  Array receiving macro parameter names.
 *  @param[out] nparams Receives the number of parameters.
 *  @return 1 if the token is a #define to be checked; 0 otherwise.
 *  @retval 0 Not a #define, object-like macro without body, or name
 *           too long.
 *  @retval 1 Parsed successfully.
 */
static int preproc_define_name(const char *p, int len,
                               char *name, int size,
                               char params[][NAMELEN], int *nparams)
{
    int i = 0, n = 0;
    int j;
    int saw_paren = 0;

    *nparams = 0;

    if (len < 7) return 0;
    if (p[0] != '#') return 0;
    i = 1;
    while (i < len && (p[i] == ' ' || p[i] == '\t')) i++;
    if (i + 6 > len) return 0;
    if (memcmp(p + i, "define", 6) != 0) return 0;
    i += 6;
    if (i >= len || (p[i] != ' ' && p[i] != '\t')) return 0;
    while (i < len && (p[i] == ' ' || p[i] == '\t')) i++;
    if (i >= len) return 0;
    if (!isalpha((unsigned char)p[i]) && p[i] != '_') return 0;
    while (i < len && (isalnum((unsigned char)p[i]) || p[i] == '_')) {
        if (n < size - 1) name[n] = p[i];
        n++;
        i++;
    }
    if (n == 0 || n >= size) return 0;
    name[n] = 0;

    if (i < len && p[i] == '(') {
        saw_paren = 1;
        i++;
        for (;;) {
            int pn = 0;
            while (i < len && (p[i] == ' ' || p[i] == '\t')) i++;
            if (i >= len || p[i] == ')') break;
            if (p[i] == '.') {
                while (i < len && p[i] != ')' && p[i] != ',') i++;
            } else if (isalpha((unsigned char)p[i]) || p[i] == '_') {
                while (i < len &&
                       (isalnum((unsigned char)p[i]) || p[i] == '_')) {
                    if (pn < NAMELEN - 1) params[*nparams][pn] = p[i];
                    pn++;
                    i++;
                }
                if (pn > 0 && pn < NAMELEN && *nparams < MAXPARAM) {
                    params[*nparams][pn] = 0;
                    (*nparams)++;
                }
            } else {
                break;
            }
            while (i < len && (p[i] == ' ' || p[i] == '\t')) i++;
            if (i < len && p[i] == ',') {
                i++;
                continue;
            }
            break;
        }
        if (i < len && p[i] == ')') i++;
    }

    j = i;
    while (j < len) {
        unsigned char c = (unsigned char)p[j];
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') { j++; continue; }
        if (c == '\\') {
            int k = j + 1;
            while (k < len && (p[k] == ' ' || p[k] == '\t')) k++;
            if (k < len && p[k] == '\n') { j = k + 1; continue; }
            return 1;
        }
        return 1;
    }

    if (saw_paren) return 1;
    return 0;
}

/*! @brief Checks documentation of top-level #define directives.
 *
 * Operates on the raw-file tokens (pass 1), because the preprocessor
 * removes #define directives from its output.  Every #define at file
 * scope that has a body or parameters must be preceded by a Qt-style
 * doc block containing @brief and @def; the argument of @def must
 * match the macro name.  For a function-like macro each parameter
 * must have a matching @param.  Object-like macros without a body
 * are skipped by preproc_define_name().
 *
 *  @param[in] fname File name, used in diagnostics. Not NULL.
 *  @return Number of warnings emitted.
 */
static int check_defines_raw(const char *fname)
{
    int errors = 0;
    int i = 0;
    int pending_start = -1;
    int pending_end = -1;

    while (toks[i].type != T_EOF) {

        if (toks[i].type == T_DOC_JAVADOC ||
            toks[i].type == T_DOC_TRAILING ||
            toks[i].type == T_COMMENT) {
            i++;
            continue;
        }

        if (toks[i].type == T_DOC) {
            int rs = -1;
            int re = i;
            int has_file_in_block = 0;
            DocInfo one;

            while (toks[i].type == T_DOC) {
                memset(&one, 0, sizeof(one));
                parse_doc_into(toks[i].p, toks[i].len, &one, 0);
                if (one.has_file) has_file_in_block = 1;
                if (rs < 0) rs = i;
                i++;
                re = i;
            }
            if (!has_file_in_block && rs >= 0) {
                pending_start = rs;
                pending_end = re;
            }
            continue;
        }

        if (toks[i].type == T_PREPROC) {
            char macname[NAMELEN];
            char mparams[MAXPARAM][NAMELEN];
            int nmparams = 0;

            if (preproc_define_name(toks[i].p, toks[i].len,
                                    macname, NAMELEN,
                                    mparams, &nmparams)) {
                DocInfo di;
                int has_doc = (pending_start >= 0);
                int decl_line = toks[i].line;
                int k;

                memset(&di, 0, sizeof(di));
                if (has_doc) {
                    for (k = pending_start; k < pending_end; k++) {
                        parse_doc_into(toks[k].p, toks[k].len, &di, 0);
                    }
                }

                if (!has_doc) {
                    printf("%s:%d: warning: macro '%s': missing Doxygen comment\n",
                           fname, decl_line, macname);
                    errors++;
                } else {
                    if (!di.has_brief) {
                        printf("%s:%d: warning: macro '%s': missing @brief\n",
                               fname, decl_line, macname);
                        errors++;
                    }
                    if (!di.has_def) {
                        printf("%s:%d: warning: macro '%s': missing @def\n",
                               fname, decl_line, macname);
                        errors++;
                    } else if (di.def_name[0] != 0 &&
                               strcmp(di.def_name, macname) != 0) {
                        printf("%s:%d: warning: macro '%s': @def name '%s' does not match\n",
                               fname, decl_line, macname, di.def_name);
                        errors++;
                    }
                    for (k = 0; k < nmparams; k++) {
                        int m, found = 0;
                        for (m = 0; m < di.nparams; m++) {
                            if (strcmp(mparams[k], di.params[m]) == 0) {
                                found = 1; break;
                            }
                        }
                        if (!found) {
                            printf("%s:%d: warning: macro '%s': missing @param %s\n",
                                   fname, decl_line, macname, mparams[k]);
                            errors++;
                        }
                    }
                }
            }

            pending_start = -1;
            pending_end = -1;
            i++;
            continue;
        }

        pending_start = -1;
        pending_end = -1;
        i++;
    }

    return errors;
}

/*! @brief Runs the preprocessor and keeps the original file's lines.
 *
 * Builds a single command line for the preprocessor and runs it with
 * system().  The option text comes from raw_opts, which main() filled
 * from the raw command line of doxy-lint via getcmd(); it is inserted
 * after being escaped for the target command processor.  On Linux the
 * double quotes inside the option text are escaped with a backslash,
 * because system() runs /bin/sh and the shell would otherwise strip
 * them; on Win32 and OS/2 the text is passed through unchanged.
 * Temporary output (.i) and error (.err) files live in the platform
 * temp directory and are always removed.  The current directory is
 * changed to the source file's directory first, so relative -i=
 * options and #include "..." behave exactly as in the project's
 * makefiles.
 *
 *  @param[in]  fname   Source file to preprocess. Not NULL.
 *  @param[out] out_len Receives the buffer length in bytes.
 *  @return Buffer, or NULL when preprocessing is unavailable or fails.
 *  @retval NULL No temp directory, compiler failed, temp file could
 *               not be read, or memory could not be allocated.
 *  @retval buf  Buffer with preprocessed source of fname.
 */
static char *preprocess_file(const char *fname, long *out_len)
{
    char tmpdir[PATHBUF];
    char tmpabs[PATHBUF];
    char tmpout[PATHBUF];
    char tmperr[PATHBUF];
    char dirpart[PATHBUF];
    char basepart[PATHBUF];
    char savedcwd[PATHBUF];
    char esc_opts[PRELINE * 2];
    char cmd[PRELINE * 2 + PATHBUF * 4];
    const char *slash1, *slash2, *slash;
    const char *cc;
    FILE *fp;
    char line[PRELINE];
    char *result;
    size_t cap, len, emitted_lines;
    long src_line;
    int in_orig;
    char cur_file[PATHBUF];
    int have_cwd;
    int rc;
    int uniq;

    slash1 = strrchr(fname, '/');
    slash2 = strrchr(fname, '\\');
    slash = slash1;
    if (slash2 != NULL && (slash == NULL || slash2 > slash)) slash = slash2;

    if (slash == NULL) {
        dirpart[0] = 0;
        strncpy(basepart, fname, PATHBUF - 1);
        basepart[PATHBUF - 1] = 0;
    } else {
        size_t dlen = (size_t)(slash - fname);
        if (dlen >= PATHBUF) dlen = PATHBUF - 1;
        memcpy(dirpart, fname, dlen);
        dirpart[dlen] = 0;
        strncpy(basepart, slash + 1, PATHBUF - 1);
        basepart[PATHBUF - 1] = 0;
    }

    if (!get_temp_dir(tmpdir, sizeof(tmpdir))) return NULL;
    if (!make_absolute(tmpdir, tmpabs, sizeof(tmpabs))) return NULL;

    {
        size_t dlen = strlen(tmpabs);
        if (dlen == 0 ||
            (tmpabs[dlen - 1] != '/' && tmpabs[dlen - 1] != '\\')) {
            if (dlen + 2 > sizeof(tmpabs)) return NULL;
            tmpabs[dlen] = DIRSEP;
            tmpabs[dlen + 1] = 0;
        }
    }

    uniq = ++temp_counter;
    sprintf(tmpout, "%sdoxy-lint-%d-%d.i",
            tmpabs, (int)getpid(), uniq);
    sprintf(tmperr, "%sdoxy-lint-%d-%d.err",
            tmpabs, (int)getpid(), uniq);

    cc = preproc_compiler(fname);
    escape_raw_opts(raw_opts, esc_opts, sizeof(esc_opts));

    snprintf(cmd, sizeof(cmd),
             "%s -pcl -fr=\"%s\" %s -fo=\"%s\" \"%s\"",
             cc, tmperr, esc_opts, tmpout, basepart);

    if (getenv("DOXY_LINT_DEBUG") != NULL) {
        fprintf(stderr, "doxy-lint: %s\n", cmd);
    }

    have_cwd = 0;
    if (dirpart[0] != 0) {
        if (PLATFORM_GETCWD(savedcwd, PATHBUF) != NULL) have_cwd = 1;
        if (PLATFORM_CHDIR(dirpart) != 0) have_cwd = 0;
    }

    rc = system(cmd);

    if (have_cwd) PLATFORM_CHDIR(savedcwd);

    if (rc != 0) {
        remove(tmpout);
        remove(tmperr);
        return NULL;
    }
    remove(tmperr);

    fp = fopen(tmpout, "rb");
    if (fp == NULL) {
        remove(tmpout);
        return NULL;
    }

    cap = 65536;
    result = (char *)malloc(cap);
    if (result == NULL) { fclose(fp); remove(tmpout); return NULL; }
    result[0] = 0;
    len = 0;
    emitted_lines = 0;

    cur_file[0] = 0;
    src_line = 1;
    in_orig = 0;

    while (fgets(line, sizeof(line), fp) != NULL) {
        size_t linelen = strlen(line);
        int is_directive = 0;
        long num;
        char new_file[PATHBUF];
        char *p, *end, *q;

        p = line;
        if (*p == '#') {
            p++;
            while (*p == ' ' || *p == '\t') p++;
            if (strncmp(p, "line", 4) == 0 &&
                (p[4] == ' ' || p[4] == '\t')) {
                p += 4;
                while (*p == ' ' || *p == '\t') p++;
            }
            if (isdigit((unsigned char)*p)) {
                num = strtol(p, &end, 10);
                while (*end == ' ' || *end == '\t') end++;
                if (*end == '"') {
                    q = new_file;
                    end++;
                    while (*end != '"' && *end != 0 && *end != '\n' &&
                           q < new_file + PATHBUF - 1) {
                        *q++ = *end++;
                    }
                    *q = 0;
                    strncpy(cur_file, new_file, PATHBUF - 1);
                    cur_file[PATHBUF - 1] = 0;
                    src_line = num;
                    in_orig = path_names_match(cur_file, fname);
                    is_directive = 1;
                }
            }
        }

        if (is_directive) continue;

        if (!in_orig) {
            src_line++;
            continue;
        }

        while ((long)emitted_lines < src_line - 1) {
            if (len + 2 > cap) {
                cap *= 2;
                result = (char *)realloc(result, cap);
                if (result == NULL) {
                    fclose(fp); remove(tmpout); return NULL;
                }
            }
            result[len++] = '\n';
            result[len] = 0;
            emitted_lines++;
        }

        if (len + linelen + 1 > cap) {
            while (len + linelen + 1 > cap) cap *= 2;
            result = (char *)realloc(result, cap);
            if (result == NULL) {
                fclose(fp); remove(tmpout); return NULL;
            }
        }
        memcpy(result + len, line, linelen);
        len += linelen;
        result[len] = 0;
        emitted_lines++;
        src_line++;
    }

    fclose(fp);
    remove(tmpout);

    *out_len = (long)len;
    return result;
}

/*! @brief Runs all checks on a single source file.
 *  @param[in] fname Path to the source file.
 *  @return Number of warnings emitted.
 *  @retval 1 The file could not be read.
 */
static int check_file(const char *fname)
{
    int errors = 0;
    int i;
    int pending_start;
    int pending_end;
    int pending_bad_style;
    int saw_file_doc;

    raw_returns_reset();
    src = slurp(fname, &srclen);
    if (src == NULL) {
        printf("%s: cannot read\n", fname);
        return 1;
    }
    ntok = 0;
    tokenize();
    collect_raw_returns();
    errors += check_defines_raw(fname);
    free(src);
    src = NULL;

    src = preprocess_file(fname, &srclen);
    if (src == NULL) {
        if (!wpp_warned) {
            printf("doxy-lint: preprocessing unavailable, "
                   "using raw files\n");
            wpp_warned = 1;
        }
        src = slurp(fname, &srclen);
    }
    if (src == NULL) {
        printf("%s: cannot read\n", fname);
        return 1;
    }
    ntok = 0;
    tokenize();

    pending_start = -1;
    pending_end = -1;
    pending_bad_style = 0;
    saw_file_doc = 0;
    i = 0;
    while (toks[i].type != T_EOF) {
        DocInfo di;
        if (toks[i].type == T_COMMENT || toks[i].type == T_PREPROC ||
            toks[i].type == T_DOC_JAVADOC ||
            toks[i].type == T_DOC_TRAILING) {
            i++;
            continue;
        }
        if (toks[i].type == T_DOC) {
            memset(&di, 0, sizeof(di));
            while (toks[i].type == T_DOC) {
                parse_doc_into(toks[i].p, toks[i].len, &di, 0);
                i++;
            }
            if (di.has_file) saw_file_doc = 1;
            break;
        }
        break;
    }
    if (!saw_file_doc) {
        printf("%s: warning: missing @file comment at top of file\n", fname);
        errors++;
    }

    i = 0;
    while (toks[i].type != T_EOF) {

        if (toks[i].type == T_DOC_JAVADOC) {
            while (toks[i].type == T_DOC_JAVADOC) {
                printf("%s:%d: warning: wrong Doxygen style; "
                       "Qt style required (/*! ... */ or //! ...)\n",
                       fname, toks[i].line);
                errors++;
                i++;
            }
            pending_bad_style = 1;
            continue;
        }

        if (toks[i].type == T_DOC) {
            int rs = -1;
            int re = i;
            DocInfo one;

            while (toks[i].type == T_DOC) {
                memset(&one, 0, sizeof(one));
                parse_doc_into(toks[i].p, toks[i].len, &one, 0);
                if (one.has_file) {
                    if (rs < 0) {
                        i++;
                        re = i;
                        continue;
                    }
                    break;
                }
                if (rs < 0) rs = i;
                i++;
                re = i;
            }

            if (rs >= 0) {
                pending_start = rs;
                pending_end = re;
            }
            continue;
        }

        if (toks[i].type == T_DOC_TRAILING) {
            i++;
            continue;
        }

        if (toks[i].type == T_PREPROC) {
            i++;
            continue;
        }

        if (toks[i].type == T_COMMENT) {
            i++;
            continue;
        }

        {
            int dstart = i;
            int dend = -1;
            int has_body = 0;
            int body_start = -1, body_end = -1;
            int fn_paren = -1, close_paren = -1, fn_name_idx = -1;
            int pdepth = 0, bdepth = 0;
            int saw_eq = 0;
            int has_struct = 0, has_enum = 0;
            int last_ident = -1;
            int in_tag = 0;

            while (toks[i].type != T_EOF) {

                if (toks[i].type == T_IDENT) {
                    if (tok_is(i, "struct") || tok_is(i, "union") ||
                        tok_is(i, "class") || tok_is(i, "namespace")) {
                        has_struct = 1; in_tag = 1;
                    } else if (tok_is(i, "enum")) {
                        has_enum = 1; in_tag = 1;
                    } else if (in_tag) {
                        in_tag = 0;
                    } else if (!is_keyword(toks[i].p, toks[i].len)) {
                        last_ident = i;
                    }
                }

                if (tok_punct(i, "[")) {
                    bdepth++;
                } else if (tok_punct(i, "]")) {
                    if (bdepth > 0) bdepth--;
                } else if (tok_punct(i, "(")) {
                    if (pdepth == 0 && bdepth == 0 &&
                        fn_paren < 0 && !saw_eq) {
                        fn_paren = i;
                        fn_name_idx = last_ident;
                    }
                    pdepth++;
                } else if (tok_punct(i, ")")) {
                    if (pdepth == 1 && fn_paren >= 0 && close_paren < 0)
                        close_paren = i;
                    if (pdepth > 0) pdepth--;
                } else if (tok_punct(i, "{")) {
                    if (pdepth == 0 && bdepth == 0) {
                        if (saw_eq) {
                            i = skip_balanced(i, "{", "}");
                            continue;
                        }
                        if (has_struct || has_enum) {
                            i = skip_balanced(i, "{", "}");
                            continue;
                        }
                        if (fn_paren >= 0 && fn_name_idx >= 0) {
                            has_body = 1;
                            body_start = i;
                            body_end = skip_balanced(i, "{", "}") - 1;
                            i = body_end + 1;
                            if (tok_punct(i, ";")) i++;
                            break;
                        }
                        i = skip_balanced(i, "{", "}");
                        continue;
                    }
                    i = skip_balanced(i, "{", "}");
                    continue;
                } else if (tok_punct(i, "}")) {
                    break;
                } else if (tok_punct(i, ";")) {
                    if (pdepth == 0 && bdepth == 0) {
                        dend = i;
                        i++;
                        break;
                    }
                } else if (tok_punct(i, "=")) {
                    if (pdepth == 0 && bdepth == 0) saw_eq = 1;
                }
                i++;
            }

            if (dend < 0 && !has_body) break;

            {
                DocInfo di;
                int has_doc = (pending_start >= 0);
                int bad_style_here = pending_bad_style;
                int decl_line = toks[dstart].line;
                int k;

                memset(&di, 0, sizeof(di));
                if (has_doc) {
                    for (k = pending_start; k < pending_end; k++) {
                        parse_doc_into(toks[k].p, toks[k].len, &di, 0);
                    }
                }
                pending_start = -1;
                pending_end = -1;
                pending_bad_style = 0;

                if (toks[i].type == T_DOC_TRAILING) {
                    while (toks[i].type == T_DOC_TRAILING) {
                        parse_doc_into(toks[i].p, toks[i].len, &di, 1);
                        i++;
                    }
                    has_doc = 1;
                }

                if (fn_name_idx >= 0 && !saw_eq) {
                    char fname_buf[NAMELEN];
                    int nl = toks[fn_name_idx].len;
                    int is_void;

                    if (nl >= NAMELEN) nl = NAMELEN - 1;
                    memcpy(fname_buf, toks[fn_name_idx].p, nl);
                    fname_buf[nl] = 0;

                    if (!has_doc) {
                        if (!bad_style_here) {
                            printf("%s:%d: warning: function '%s': missing Doxygen comment\n",
                                   fname, decl_line, fname_buf);
                            errors++;
                        }
                    } else {
                        if (!di.has_brief) {
                            printf("%s:%d: warning: function '%s': missing @brief\n",
                                   fname, decl_line, fname_buf);
                            errors++;
                        }

                        {
                            char pnames[MAXPARAM][NAMELEN];
                            int npnames = 0;
                            extract_params(fn_paren, close_paren,
                                           pnames, &npnames);
                            for (k = 0; k < npnames; k++) {
                                int m, found = 0;
                                for (m = 0; m < di.nparams; m++) {
                                    if (strcmp(pnames[k],
                                               di.params[m]) == 0) {
                                        found = 1; break;
                                    }
                                }
                                if (!found) {
                                    printf("%s:%d: warning: function '%s': missing @param %s\n",
                                           fname, decl_line, fname_buf,
                                           pnames[k]);
                                    errors++;
                                }
                            }
                        }

                        for (k = 0; k < di.nparams; k++) {
                            if (di.paramdir[k] == 0) {
                                printf("%s:%d: warning: function '%s': @param %s lacks direction qualifier [in]/[out]/[in,out]\n",
                                       fname, decl_line, fname_buf,
                                       di.params[k]);
                                errors++;
                            }
                        }

                        is_void = returns_void(dstart, fn_name_idx);

                        if (is_void) {
                            if (di.has_return) {
                                printf("%s:%d: warning: function '%s': @return not applicable to void function\n",
                                       fname, decl_line, fname_buf);
                                errors++;
                            }
                            if (di.nretvals > 0) {
                                printf("%s:%d: warning: function '%s': @retval not applicable to void function\n",
                                       fname, decl_line, fname_buf);
                                errors++;
                            }
                        } else {
                            if (!di.has_return) {
                                printf("%s:%d: warning: function '%s': missing @return\n",
                                       fname, decl_line, fname_buf);
                                errors++;
                            }
                            if (has_body) {
                                char rexprs[MAXRET][EXPRLEN];
                                int nrexprs = 0;
                                collect_returns(body_start, body_end,
                                                rexprs, &nrexprs);
                                for (k = 0; k < nrexprs; k++) {
                                    int m, found = 0;
                                    for (m = 0; m < di.nretvals; m++) {
                                        if (strcmp(rexprs[k],
                                                   di.retvals[m]) == 0) {
                                            found = 1; break;
                                        }
                                    }
                                    if (!found) {
                                        printf("%s:%d: warning: function '%s': return '%s' not documented with @retval\n",
                                               fname, decl_line, fname_buf,
                                               rexprs[k]);
                                        errors++;
                                    }
                                }
                            }
                        }
                    }
                } else {
                    if (!has_doc) {
                        if (!bad_style_here) {
                            printf("%s:%d: warning: missing Doxygen comment\n",
                                   fname, decl_line);
                            errors++;
                        }
                    } else if (!di.has_brief) {
                        printf("%s:%d: warning: missing @brief\n",
                               fname, decl_line);
                        errors++;
                    }
                }
            }
        }
    }

    free(src);
    src = NULL;
    return errors;
}

/*! @brief Program entry point.
 *  @param[in] argc Argument count.
 *  @param[in] argv Argument vector.  Every argument that begins with
 *                  '-' is appended to the preprocessor options; every
 *                  other argument is treated as a source file to
 *                  check.
 *  @return Always 0: the tool never fails the build.
 *  @retval 0 Always.
 */
int main(int argc, char *argv[])
{
    const char *prog = (argc > 0 && argv[0] != NULL) ? argv[0] : "doxy-lint";
    char raw_cmd[PRELINE];
    int i;

    if (argc < 2) {
        printf("Usage: %s <file>... [preprocessor options...]\n", prog);
        printf("  Arguments beginning with '-' are passed to the\n");
        printf("  Watcom preprocessor as-is.  All other arguments are\n");
        printf("  treated as source files and checked in order.\n");
        printf("  Example:\n");
        printf("    %s foo.c bar.c -zq -q -d__WATCOM__ "
               "-i=tools\\shared -i=build\\host\\win32\\tools\n", prog);
        return 0;
    }

    raw_cmd[0] = 0;
    if (getcmd(raw_cmd) == NULL) raw_cmd[0] = 0;
    extract_raw_opts(raw_cmd, raw_opts, sizeof(raw_opts));

    for (i = 1; i < argc; i++) {
        if (argv[i][0] == '-') continue;
        check_file(argv[i]);
    }

    return 0;
}
