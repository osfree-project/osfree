/*! @file doxy-lint.c
 *  @brief Checks C/C++ source files for missing Doxygen documentation.
 *
 *  Style: Qt only.  Recognized forms:
 *    - leading:  /*! ... *\/   or  //! ...
 *    - trailing: /*!< ... *\/  or  //!< ...
 *  Trailing forms document the preceding declaration.  Javadoc style
 *  ("/**", "///") and Javadoc trailing forms ("/**<", "///<") are
 *  reported as wrong style.  Banner comments like "/*****" and
 *  "/////..." are treated as ordinary comments.
 *
 *  Enforces:
 *    - @file   required in every source and header file;
 *    - @brief  required before (or trailing after) every top-level
 *             declaration;
 *    - @param  required for every named function parameter;
 *    - @return required for every non-void function;
 *    - @retval required for every integer literal returned by a body.
 *
 *  Usage: doxy-lint <directory>
 *
 *  Scanned extensions (case-insensitive):
 *    .c .h .cpp .hpp .cc .hh .cxx .hxx .inl
 *
 *  All matching files in the given directory are scanned (non-recursive).
 *  Diagnostics go to stdout.  Exit status is always 0.
 *
 *  Targets: Linux, Win32, OS/2.  Compiler: Open Watcom 1.9.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#if defined(__LINUX__)
#  include <dirent.h>
#  define DIRSEP '/'
#elif defined(__NT__) || defined(__OS2__)
#  include <direct.h>
#  define DIRSEP '\\'
#else
#  error "Unsupported target: expected Linux, Win32 or OS/2"
#endif

#define MAXTOK   65536
#define MAXPARAM 128
#define MAXRET   128
#define NAMELEN  64
#define EXPRLEN  128
#define PATHBUF  1024

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

/*! @brief Appends a token to the global toks[] array.
 *  @param type Token type constant.
 *  @param p    Pointer into the src buffer.
 *  @param len  Token length in bytes.
 *  @param line Source line where the token starts.
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
 *  @param p Pointer to the first character of the punctuator.
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

        /* Line comment: //!<  //!  ///<  ///  // */
        if (c == '/' && i + 1 < srclen && src[i + 1] == '/') {
            int sl = line;
            int kind = 0; /* 0=plain, 1=qt, 2=qt_trailing, 3=javadoc */
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

        /* Block comment: /*!< ... *\/  /*! ... *\/  /** ... *\/  /* ... *\/ */
        if (c == '/' && i + 1 < srclen && src[i + 1] == '*') {
            int sl = line;
            int kind = 0;
            long cs, ce;

            if (i + 2 < srclen && src[i + 2] == '!') {
                if (i + 3 < srclen && src[i + 3] == '<') kind = 2;
                else kind = 1;
            } else if (i + 2 < srclen && src[i + 2] == '*') {
                if (i + 3 >= srclen) kind = 0;
                else if (src[i + 3] == '/') kind = 0;   /* empty */
                else if (src[i + 3] == '*') kind = 0;   /* banner */
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

        /* Orphan closing sequence of a comment that started inside a
         * preprocessor line without a line continuation.  Not a
         * punctuator.  Also guards against a stray "*"+"/" in code. */
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
 *  @param i Index into toks[].
 *  @param s Expected identifier text.
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
 *  @param i Index into toks[].
 *  @param s Expected punctuator text.
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
 *  @param s   Pointer to the word text.
 *  @param len Word length in bytes.
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

/*! @brief Tests whether a word is a storage-class specifier or qualifier.
 *  @param s   Pointer to the word text.
 *  @param len Word length in bytes.
 *  @return Non-zero on match.
 *  @retval 0 Not a storage-class specifier or qualifier.
 *  @retval 1 Storage-class specifier or qualifier.
 */
static int is_storage_or_qualifier(const char *s, int len)
{
    static const char *kw[] = {
        "static", "extern", "inline", "register", "auto",
        "const", "volatile", "restrict",
        "__inline", "__inline__", "__restrict", "__restrict__",
        "__cdecl", "__stdcall", "__fastcall", "__pascal",
        "_cdecl", "_stdcall", "_fastcall", "_pascal",
        "near", "far", "huge",
        NULL
    };
    int i;
    for (i = 0; kw[i] != NULL; i++) {
        if ((int)strlen(kw[i]) == len && memcmp(kw[i], s, len) == 0) return 1;
    }
    return 0;
}

/*! @brief Skips a balanced pair (open ... close).
 *  @param open_idx  Index of the opening token.
 *  @param open_str  Text of the opening punctuator.
 *  @param close_str Text of the closing punctuator.
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
    char params[MAXPARAM][NAMELEN];
    int nparams;
    int has_return;
    char retvals[MAXRET][NAMELEN];
    int nretvals;
} DocInfo;

/*! @brief Parses the body of a Doxygen comment into info.
 *  @param p    Comment text without delimiters.
 *  @param len  Length of the comment text in bytes.
 *  @param info Destination for @file/@brief/@param/@return/@retval.
 */
static void parse_doc_into(const char *p, int len, DocInfo *info)
{
    int i = 0;
    int at_line_start = 1;

    while (i < len) {
        char c = p[i];
        int cs, clen, as, alen;

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
                    while (i < len && p[i] != ']') i++;
                    if (i < len) i++;
                    while (i < len && (p[i] == ' ' || p[i] == '\t')) i++;
                }
            }

            as = i;
            while (i < len && !isspace((unsigned char)p[i]) && p[i] != '*') i++;
            alen = i - as;

            if (clen == 4 && memcmp(p + cs, "file", 4) == 0) {
                info->has_file = 1;
            } else if (clen == 5 && memcmp(p + cs, "brief", 5) == 0) {
                info->has_brief = 1;
            } else if (clen == 5 && memcmp(p + cs, "param", 5) == 0) {
                if (alen > 0 && alen < NAMELEN && info->nparams < MAXPARAM) {
                    memcpy(info->params[info->nparams], p + as, alen);
                    info->params[info->nparams][alen] = 0;
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
}

/*! @brief Checks whether the return type is plain "void".
 *  @param start    Index of the first token of the return type.
 *  @param name_idx Index of the function-name token.
 *  @return Non-zero if the type is void without pointers or qualifiers.
 *  @retval 0 Return type is not plain void.
 *  @retval 1 Return type is plain void.
 */
static int returns_void(int start, int name_idx)
{
    int j;
    int saw_void = 0;
    int saw_other = 0;

    for (j = start; j < name_idx; j++) {
        if (tok_punct(j, "*")) {
            if (saw_void) return 0;
            continue;
        }
        if (toks[j].type != T_IDENT) continue;
        if (is_storage_or_qualifier(toks[j].p, toks[j].len)) continue;
        if (tok_is(j, "void")) { saw_void = 1; continue; }
        saw_other = 1;
    }
    return saw_void && !saw_other;
}

/*! @brief Extracts the name of one parameter and appends it to names.
 *  @param start  Index of the first token of the parameter.
 *  @param end    Index just past the last token of the parameter.
 *  @param names  Array receiving the parameter names.
 *  @param nnames In/out counter of names already stored.
 */
static void extract_one_param(int start, int end,
                              char names[][NAMELEN], int *nnames)
{
    int j;
    int name_idx = -1;
    int in_tag = 0;
    int bdepth = 0;

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
        if (toks[j].type != T_IDENT) continue;
        if (tok_is(j, "struct") || tok_is(j, "union") || tok_is(j, "enum") ||
            tok_is(j, "class") || tok_is(j, "namespace")) {
            in_tag = 1;
            continue;
        }
        if (in_tag) { in_tag = 0; continue; }
        if (!is_keyword(toks[j].p, toks[j].len)) name_idx = j;
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
 *  @param open_idx  Index of the '(' token, or -1.
 *  @param close_idx Index of the matching ')'.
 *  @param names     Array receiving the parameter names.
 *  @param nnames    In/out counter of names already stored.
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

/*! @brief Collects integer literals returned from a function body.
 *  @param body_start Index of the opening '{' of the body.
 *  @param body_end   Index of the matching '}'.
 *  @param exprs      Array receiving the literal strings.
 *  @param nexprs     In/out counter of literals already stored.
 */
static void collect_returns(int body_start, int body_end,
                            char exprs[][EXPRLEN], int *nexprs)
{
    int j;

    for (j = body_start + 1; j < body_end; j++) {
        int k, bdepth, es, ee, m;
        char buf[EXPRLEN];

        if (!tok_is(j, "return")) continue;

        k = j + 1;
        bdepth = 0;
        es = k;
        while (k < body_end) {
            if (tok_punct(k, "{")) bdepth++;
            else if (tok_punct(k, "}")) {
                if (bdepth == 0) break;
                bdepth--;
            } else if (tok_punct(k, ";") && bdepth == 0) break;
            k++;
        }
        ee = k;
        if (ee <= es) continue;

        if (ee - es == 1 && toks[es].type == T_NUMBER) {
            int len = toks[es].len;
            if (len >= EXPRLEN) len = EXPRLEN - 1;
            memcpy(buf, toks[es].p, len);
            buf[len] = 0;
        } else if (ee - es == 2 &&
                   toks[es].type == T_PUNCT && toks[es].len == 1 &&
                   (toks[es].p[0] == '-' || toks[es].p[0] == '+') &&
                   toks[es + 1].type == T_NUMBER) {
            int len = toks[es + 1].len;
            if (len + 1 >= EXPRLEN) len = EXPRLEN - 2;
            buf[0] = toks[es].p[0];
            memcpy(buf + 1, toks[es + 1].p, len);
            buf[len + 1] = 0;
        } else {
            continue;
        }

        {
            int dup = 0;
            for (m = 0; m < *nexprs; m++) {
                if (strcmp(exprs[m], buf) == 0) { dup = 1; break; }
            }
            if (!dup && *nexprs < MAXRET) {
                strcpy(exprs[*nexprs], buf);
                (*nexprs)++;
            }
        }
    }
}

/*! @brief Reads the whole file fname into a heap buffer.
 *  @param fname   Path to the file.
 *  @param out_len Receives the number of bytes read.
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

/*! @brief Runs all checks on a single source file.
 *  @param fname Path to the source file.
 *  @return Number of warnings emitted.
 *  @retval 1 The file could not be opened.
 */
static int check_file(const char *fname)
{
    int errors = 0;
    int i;
    int pending_start;
    int pending_end;
    int pending_bad_style;
    int saw_file_doc;

    src = slurp(fname, &srclen);
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
                parse_doc_into(toks[i].p, toks[i].len, &di);
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
            int rs = i;
            DocInfo di;
            memset(&di, 0, sizeof(di));
            while (toks[i].type == T_DOC) {
                parse_doc_into(toks[i].p, toks[i].len, &di);
                i++;
            }
            if (!di.has_file) {
                pending_start = rs;
                pending_end = i;
            }
            continue;
        }

        if (toks[i].type == T_DOC_TRAILING) {
            i++;
            continue;
        }

        if (toks[i].type == T_COMMENT || toks[i].type == T_PREPROC) {
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
                        parse_doc_into(toks[k].p, toks[k].len, &di);
                    }
                }
                pending_start = -1;
                pending_end = -1;
                pending_bad_style = 0;

                if (toks[i].type == T_DOC_TRAILING) {
                    while (toks[i].type == T_DOC_TRAILING) {
                        parse_doc_into(toks[i].p, toks[i].len, &di);
                        i++;
                    }
                    has_doc = 1;
                }

                if (fn_name_idx >= 0 && !saw_eq) {
                    char fname_buf[NAMELEN];
                    int nl = toks[fn_name_idx].len;

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

                        if (!returns_void(dstart, fn_name_idx)) {
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

/*! @brief Case-insensitive test: does name end with ext?
 *  @param name File name to test.
 *  @param ext  Extension including the leading dot.
 *  @return Non-zero if name ends with ext.
 *  @retval 0 No match.
 *  @retval 1 Match.
 */
static int ext_matches(const char *name, const char *ext)
{
    size_t nlen = strlen(name);
    size_t elen = strlen(ext);
    size_t i;

    if (nlen < elen) return 0;
    for (i = 0; i < elen; i++) {
        char a = name[nlen - elen + i];
        char b = ext[i];
        if (tolower((unsigned char)a) != tolower((unsigned char)b)) return 0;
    }
    return 1;
}

/*! @brief Extensions of files that are checked. */
static const char *const suffixes[] = {
    ".c", ".h",
    ".cpp", ".hpp",
    ".cc", ".hh",
    ".cxx", ".hxx",
    ".inl",
    NULL
};

/*! @brief Invokes check_file on every matching file in dirpath.
 *  @param dirpath Directory to scan (non-recursive).
 *  @return Total number of warnings across all processed files.
 *  @retval 0 Directory cannot be opened, or no warnings were emitted.
 */
static int scan_dir(const char *dirpath)
{
    DIR *dp;
    struct dirent *de;
    int total = 0;
    char path[PATHBUF];
    size_t dlen;
    int need_sep;

    dp = opendir(dirpath);
    if (dp == NULL) {
        printf("%s: cannot open directory\n", dirpath);
        return 0;
    }

    dlen = strlen(dirpath);
    need_sep = 1;
    if (dlen > 0 &&
        (dirpath[dlen - 1] == '/' || dirpath[dlen - 1] == '\\')) {
        need_sep = 0;
    }

    while ((de = readdir(dp)) != NULL) {
        const char *name = de->d_name;
        size_t nlen = strlen(name);
        int s;

        for (s = 0; suffixes[s] != NULL; s++) {
            if (ext_matches(name, suffixes[s])) {
                size_t need = dlen + (need_sep ? 1 : 0) + nlen + 1;
                if (need > sizeof(path)) {
                    printf("%s%c%s: path too long, skipped\n",
                           dirpath, need_sep ? DIRSEP : ' ', name);
                    break;
                }
                memcpy(path, dirpath, dlen);
                if (need_sep) {
                    path[dlen] = DIRSEP;
                    memcpy(path + dlen + 1, name, nlen + 1);
                } else {
                    memcpy(path + dlen, name, nlen + 1);
                }
                total += check_file(path);
                break;
            }
        }
    }

    closedir(dp);
    return total;
}

/*! @brief Program entry point.
 *  @param argc Argument count.
 *  @param argv Argument vector; argv[1] is the directory to scan.
 *  @return Always 0: the tool never fails the build.
 *  @retval 0 Always.
 */
int main(int argc, char *argv[])
{
    const char *prog = (argc > 0 && argv[0] != NULL) ? argv[0] : "doxy-lint";

    if (argc != 2) {
        printf("Usage: %s <directory>\n", prog);
        return 0;
    }

    scan_dir(argv[1]);
    return 0;
}
