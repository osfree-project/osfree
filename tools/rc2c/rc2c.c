/*! @file rc2c.c
 *  @brief Driver, symbol table, AST helpers and preprocessor.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "rc2c_ast.h"

/*! @brief Symbol table. */
HHASHTABLE g_hSyms   = NULLHANDLE;

/*! @brief Items vector for the dialog being parsed. */
HVECTOR    g_hItems  = NULLHANDLE;

/*! @brief Dialog being built. */
RCDIALOG  *g_pCurDlg = NULL;

/*! @brief Dialog produced by the last parse. */
RCDIALOG  *g_pResult = NULL;

/* ==================================================================
 * Style table
 *
 * Ordering within each context matters: multi-bit composites come
 * before their constituents, and WS_TABSTOP precedes WS_GROUP to
 * match the style string produced by the Win16 RC compiler.
 * ================================================================== */

/*! @brief Style name/value/context table. */
const STYLEDEF g_aStyles[] = {
    { "DS_ABSALIGN",   0x0001L,     STYLE_CTX_DIALOG },
    { "DS_SYSMODAL",   0x0002L,     STYLE_CTX_DIALOG },
    { "DS_LOCALEDIT",  0x0020L,     STYLE_CTX_DIALOG },
    { "DS_SETFONT",    0x0040L,     STYLE_CTX_DIALOG },
    { "DS_MODALFRAME", 0x0080L,     STYLE_CTX_DIALOG },
    { "DS_NOIDLEMSG",  0x0100L,     STYLE_CTX_DIALOG },

    { "WS_POPUPWINDOW", 0x80880000L, STYLE_CTX_ALL },
    { "WS_CAPTION",     0x00C00000L, STYLE_CTX_ALL },

    { "WS_POPUP",       0x80000000L, STYLE_CTX_ALL },
    { "WS_CHILD",       0x40000000L, STYLE_CTX_ALL },
    { "WS_VISIBLE",     0x10000000L, STYLE_CTX_ALL },
    { "WS_DISABLED",    0x08000000L, STYLE_CTX_ALL },
    { "WS_BORDER",      0x00800000L, STYLE_CTX_ALL },
    { "WS_DLGFRAME",    0x00400000L, STYLE_CTX_ALL },
    { "WS_VSCROLL",     0x00200000L, STYLE_CTX_ALL },
    { "WS_HSCROLL",     0x00100000L, STYLE_CTX_ALL },
    { "WS_SYSMENU",     0x00080000L, STYLE_CTX_ALL },
    { "WS_THICKFRAME",  0x00040000L, STYLE_CTX_ALL },
    { "WS_TABSTOP",     0x00010000L, STYLE_CTX_ALL },
    { "WS_GROUP",       0x00020000L, STYLE_CTX_ALL },

    { "BS_GROUPBOX",        0x00000007L, STYLE_CTX_BUTTON },
    { "BS_AUTORADIOBUTTON", 0x00000009L, STYLE_CTX_BUTTON },
    { "BS_AUTO3STATE",      0x00000006L, STYLE_CTX_BUTTON },
    { "BS_AUTOCHECKBOX",    0x00000003L, STYLE_CTX_BUTTON },
    { "BS_3STATE",          0x00000005L, STYLE_CTX_BUTTON },
    { "BS_DEFPUSHBUTTON",   0x00000001L, STYLE_CTX_BUTTON },
    { "BS_CHECKBOX",        0x00000002L, STYLE_CTX_BUTTON },
    { "BS_RADIOBUTTON",     0x00000004L, STYLE_CTX_BUTTON },

    { "SS_ICON",      0x00000003L, STYLE_CTX_STATIC },
    { "SS_BLACKRECT", 0x00000004L, STYLE_CTX_STATIC },
    { "SS_GRAYRECT",  0x00000005L, STYLE_CTX_STATIC },
    { "SS_WHITERECT", 0x00000006L, STYLE_CTX_STATIC },
    { "SS_CENTER",    0x00000001L, STYLE_CTX_STATIC },
    { "SS_RIGHT",     0x00000002L, STYLE_CTX_STATIC },
    { "SS_NOPREFIX",  0x00000080L, STYLE_CTX_STATIC },

    { "ES_CENTER",      0x00000001L, STYLE_CTX_EDIT },
    { "ES_RIGHT",       0x00000002L, STYLE_CTX_EDIT },
    { "ES_MULTILINE",   0x00000004L, STYLE_CTX_EDIT },
    { "ES_PASSWORD",    0x00000020L, STYLE_CTX_EDIT },
    { "ES_AUTOVSCROLL", 0x00000040L, STYLE_CTX_EDIT },
    { "ES_AUTOHSCROLL", 0x00000080L, STYLE_CTX_EDIT },
    { "ES_READONLY",    0x00000800L, STYLE_CTX_EDIT }
};

/*! @brief Number of entries in g_aStyles. */
const ULONG g_cStyles = sizeof(g_aStyles) / sizeof(g_aStyles[0]);

/* ==================================================================
 * wlex / wyacc imports
 * ================================================================== */

/*! @brief Opaque wlex buffer state. */
struct yy_buffer_state;

/*! @brief Parse the token stream produced by yylex.
 *
 *  @return 0 on success, non-zero on error.
 */
extern int   yyparse(void);

/*! @brief Read one token.
 *
 *  @return The token code.
 */
extern int   yylex(void);

/*! @brief Scan a NUL-terminated string as lexer input.
 *
 *  @param[in] s  Input string.
 *
 *  @return New scanner buffer, or NULL on failure.
 *
 *  @retval NULL  Allocation failed.
 */
extern struct yy_buffer_state *yy_scan_string(const char *s);

/*! @brief Release a buffer created by yy_scan_string.
 *
 *  @param[in] b  Buffer.
 */
extern void  yy_delete_buffer(struct yy_buffer_state *b);

/*! @brief Current source line, maintained by the lexer. */
extern int   yy_line;

/* ==================================================================
 * String helpers
 * ================================================================== */

/*! @brief Duplicate a string with the CCL allocator.
 *
 *  @param[in] s  Source string; may be NULL.
 *
 *  @return A new NUL-terminated string, or NULL.
 *
 *  @retval NULL  s is NULL or allocation failed.
 */
char *RcStrDup(PCSZ s)
{
    size_t n;
    char *p;
    if (!s) return NULL;
    n = strlen(s) + 1;
    p = (char *)CclMalloc(n);
    if (p) memcpy(p, s, n);
    return p;
}

/*! @brief Free a string allocated by RcStrDup.
 *
 *  @param[in] s  String to free; may be NULL.
 */
void RcStrFree(char *s)
{
    if (s) CclFree(s);
}

/*! @brief Emit a diagnostic to stderr.
 *
 *  @param[in] line  Source line number.
 *  @param[in] msg   Diagnostic text; may be NULL.
 */
void RcDiagError(int line, PCSZ msg)
{
    fprintf(stderr, "rc2c: error (line %d): %s\n", line, msg ? msg : "");
}

/* ==================================================================
 * Style formatting
 * ================================================================== */

/*! @brief Format a style mask as a C expression.
 *
 *  @param[in]  dwStyle  Style mask.
 *  @param[in]  ulCtx    STYLE_CTX_* mask.
 *  @param[out] pszBuf   Output buffer.
 *  @param[in]  ulSize   Size of @p pszBuf.
 */
void RcStyleToExpr(DWORD dwStyle, ULONG ulCtx,
                   PSZ pszBuf, ULONG ulSize)
{
    DWORD remaining;
    ULONG i;
    ULONG used = 0;
    int   first = 1;

    if (!pszBuf || ulSize == 0) return;
    pszBuf[0] = 0;
    remaining = dwStyle;

    for (i = 0; i < g_cStyles; i++) {
        const STYLEDEF *pStyle = &g_aStyles[i];
        if ((pStyle->ulCtx & ulCtx) == 0) continue;
        if (pStyle->dwValue == 0) continue;
        if ((remaining & pStyle->dwValue) != pStyle->dwValue) continue;
        if (used + strlen(pStyle->pszName) + 4 >= ulSize) break;
        if (!first) { strcat(pszBuf, " | "); used += 3; }
        strcat(pszBuf, pStyle->pszName);
        used += strlen(pStyle->pszName);
        remaining &= ~pStyle->dwValue;
        first = 0;
    }

    if (remaining != 0) {
        char tmp[32];
        sprintf(tmp, "0x%08lXL", (unsigned long)remaining);
        if (used + strlen(tmp) + 4 < ulSize) {
            if (!first) strcat(pszBuf, " | ");
            strcat(pszBuf, tmp);
        }
    }

    if ((ulCtx & STYLE_CTX_STATIC) && ((dwStyle & 0xFFUL) == 0UL)) {
        if (used + 10 < ulSize) {
            if (!first) { strcat(pszBuf, " | "); used += 3; }
            strcat(pszBuf, "SS_LEFT");
            used += 7;
            first = 0;
        }
    }
    if ((ulCtx & STYLE_CTX_EDIT) && ((dwStyle & 0xFFUL) == 0UL)) {
        if (used + 10 < ulSize) {
            if (!first) { strcat(pszBuf, " | "); used += 3; }
            strcat(pszBuf, "ES_LEFT");
            used += 7;
            first = 0;
        }
    }
    if (pszBuf[0] == 0) {
        strcpy(pszBuf, "0");
    }
}

/* ==================================================================
 * Case-insensitive compare
 * ================================================================== */

/*! @brief Case-insensitive ASCII compare of two NUL-terminated strings.
 *
 *  @param[in] a  First string.
 *  @param[in] b  Second string.
 *
 *  @return TRUE if equal, FALSE otherwise.
 *
 *  @retval TRUE   Strings are equal (ASCII case-insensitive).
 *  @retval FALSE  Strings differ or an argument is NULL.
 */
static BOOL ci_eq(PCSZ a, PCSZ b)
{
    int ca, cb;
    if (!a || !b) return FALSE;
    while (*a && *b) {
        ca = *a; cb = *b;
        if (ca >= 'A' && ca <= 'Z') ca += 32;
        if (cb >= 'A' && cb <= 'Z') cb += 32;
        if (ca != cb) return FALSE;
        a++; b++;
    }
    return *a == 0 && *b == 0;
}

/*! @brief Determine the style context for one control.
 *
 *  @param[in] iKind     ITEM_* kind.
 *  @param[in] pszClass  Class name (may be NULL).
 *
 *  @return STYLE_CTX_* bit for this control.
 *
 *  @retval STYLE_CTX_BUTTON  Button-class control.
 *  @retval STYLE_CTX_STATIC  Static-class control.
 *  @retval STYLE_CTX_EDIT    Edit-class control.
 *  @retval STYLE_CTX_ALL     Unclassified CONTROL.
 */
ULONG RcStyleCtxOf(int iKind, PCSZ pszClass)
{
    switch (iKind) {
    case ITEM_DEFPUSHBUTTON:
    case ITEM_PUSHBUTTON:
    case ITEM_CHECKBOX:
    case ITEM_RADIOBUTTON:
    case ITEM_GROUPBOX:
        return STYLE_CTX_BUTTON;
    case ITEM_LTEXT:
    case ITEM_CTEXT:
    case ITEM_RTEXT:
    case ITEM_ICON:
        return STYLE_CTX_STATIC;
    case ITEM_EDITTEXT:
        return STYLE_CTX_EDIT;
    case ITEM_CONTROL:
        if (pszClass) {
            if (ci_eq(pszClass, "BUTTON")) return STYLE_CTX_BUTTON;
            if (ci_eq(pszClass, "STATIC")) return STYLE_CTX_STATIC;
            if (ci_eq(pszClass, "EDIT"))   return STYLE_CTX_EDIT;
        }
        return STYLE_CTX_ALL;
    default:
        return STYLE_CTX_ALL;
    }
}

/* ==================================================================
 * Dialog block extraction
 * ================================================================== */

/*! @brief Extract a named DIALOG/DIALOGEX block from a body.
 *
 *  @param[in]  body  NUL-terminated preprocessed text.
 *  @param[in]  name  Dialog name.
 *  @param[out] plen  Receives the fragment length in bytes.
 *
 *  @return Pointer into @p body, or NULL if not found.
 *
 *  @retval NULL  Dialog not found.
 */
static PCSZ extract_dialog(PCSZ body, PCSZ name, size_t *plen)
{
    size_t namelen = strlen(name);
    PCSZ lineStart = body;

    while (*lineStart) {
        PCSZ lineEnd = lineStart;
        PCSZ p;
        while (*lineEnd && *lineEnd != '\n') lineEnd++;

        p = lineStart;
        while (*p == ' ' || *p == '\t') p++;

        if (strncmp(p, name, namelen) == 0 &&
            (p[namelen] == ' ' || p[namelen] == '\t')) {
            PCSZ q = p + namelen;
            size_t kwlen = 0;

            while (*q == ' ' || *q == '\t') q++;
            if (strncmp(q, "DIALOGEX", 8) == 0) kwlen = 8;
            else if (strncmp(q, "DIALOG", 6) == 0) kwlen = 6;

            if (kwlen > 0) {
                PCSZ beginP = NULL;
                q = lineStart;
                while (*q) {
                    if (q[0]=='B' && q[1]=='E' && q[2]=='G' &&
                        q[3]=='I' && q[4]=='N' &&
                        (q[5]==0 || q[5]==' ' || q[5]=='\t' ||
                         q[5]=='\r' || q[5]=='\n')) {
                        beginP = q;
                        break;
                    }
                    q++;
                }
                if (beginP) {
                    int depth = 1;
                    q = beginP + 5;
                    while (*q && depth > 0) {
                        if (q[0]=='B' && q[1]=='E' && q[2]=='G' &&
                            q[3]=='I' && q[4]=='N' &&
                            (q[5]==0 || q[5]==' ' || q[5]=='\t' ||
                             q[5]=='\r' || q[5]=='\n')) {
                            depth++; q += 5;
                        } else if (q[0]=='E' && q[1]=='N' && q[2]=='D' &&
                                   (q[3]==0 || q[3]==' ' || q[3]=='\t' ||
                                    q[3]=='\r' || q[3]=='\n')) {
                            depth--; q += 3;
                            if (depth == 0) {
                                *plen = (size_t)(q - p);
                                return p;
                            }
                        } else {
                            q++;
                        }
                    }
                }
                return NULL;
            }
        }

        lineStart = (*lineEnd) ? lineEnd + 1 : lineEnd;
    }
    return NULL;
}

/* ==================================================================
 * Symbol table
 * ================================================================== */

/*! @brief Fixed-size key for the symbol table. */
typedef struct _SYMKEY { char sz[64]; } SYMKEY;

/*! @brief FNV-1a hash over a SYMKEY.
 *
 *  @param[in] p  Pointer to a SYMKEY.
 *  @param[in] c  Caller context (unused).
 *
 *  @return Hash value.
 */
static ULONG APIENTRY sym_hash(PCVOID p, PVOID c)
{
    const unsigned char *k = (const unsigned char *)p;
    ULONG h = 2166136261UL;
    int i;
    (void)c;
    for (i = 0; i < 64; i++) { h ^= k[i]; h *= 16777619UL; }
    return h;
}

/*! @brief Byte-wise comparison of two SYMKEYs.
 *
 *  @param[in] a  First key.
 *  @param[in] b  Second key.
 *  @param[in] c  Caller context (unused).
 *
 *  @return Negative, zero or positive per memcmp.
 */
static int APIENTRY sym_cmp(PCVOID a, PCVOID b, PVOID c)
{
    (void)c;
    return memcmp(a, b, 64);
}

/*! @brief Add a symbol to the table.
 *
 *  @param[in] h     Symbol table.
 *  @param[in] name  Symbol name.
 *  @param[in] v     Numeric value.
 *
 *  @return APIRET.
 *
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  h or name is NULL, or name is too
 *                                   long.
 *  @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET RcSymAdd(HHASHTABLE h, PCSZ name, long v)
{
    SYMKEY k;
    size_t n;
    if (!h || !name) return ERROR_INVALID_PARAMETER;
    n = strlen(name);
    if (n >= sizeof(k.sz)) return ERROR_INVALID_PARAMETER;
    memset(&k, 0, sizeof(k));
    memcpy(k.sz, name, n);
    return HashtablePut(h, &k, &v, NULL);
}

/*! @brief Look up a symbol.
 *
 *  @param[in]  h     Symbol table.
 *  @param[in]  name  Symbol name.
 *  @param[out] pv    Receiver for the numeric value.
 *
 *  @return APIRET.
 *
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  Any argument is NULL.
 *  @retval ERROR_FILE_NOT_FOUND     Symbol is not present.
 */
APIRET RcSymLookup(HHASHTABLE h, PCSZ name, long *pv)
{
    SYMKEY k;
    size_t n;
    ULONG used;
    APIRET rc;
    if (!h || !name || !pv) return ERROR_INVALID_PARAMETER;
    n = strlen(name);
    if (n >= sizeof(k.sz)) return ERROR_FILE_NOT_FOUND;
    memset(&k, 0, sizeof(k));
    memcpy(k.sz, name, n);
    rc = HashtableGet(h, &k, pv, sizeof(*pv), &used);
    return (rc == ERROR_NO_MORE_ITEMS) ? ERROR_FILE_NOT_FOUND : rc;
}

/*! @brief Create and populate the symbol table.
 *
 *  @param[out] ph  Receiver for the table handle.
 *
 *  @return APIRET.
 *
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  ph is NULL.
 *  @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET RcSymInit(HHASHTABLE *ph)
{
    CCLOPTS o;
    APIRET rc;
    ULONG i;
    CclOptsInit(&o);
    o.pHash = sym_hash;
    o.pCompare = sym_cmp;
    rc = HashtableCreate(sizeof(SYMKEY), sizeof(long), 0, &o, ph);
    if (rc != NO_ERROR) return rc;

    for (i = 0; i < g_cStyles; i++)
        RcSymAdd(*ph, g_aStyles[i].pszName, (long)g_aStyles[i].dwValue);

    RcSymAdd(*ph, "IDOK",       1L);
    RcSymAdd(*ph, "IDCANCEL",   2L);
    RcSymAdd(*ph, "IDABORT",    3L);
    RcSymAdd(*ph, "IDRETRY",    4L);
    RcSymAdd(*ph, "IDIGNORE",   5L);
    RcSymAdd(*ph, "IDYES",      6L);
    RcSymAdd(*ph, "IDNO",       7L);
    RcSymAdd(*ph, "IDC_STATIC", -1L);
    return NO_ERROR;
}

/* ==================================================================
 * AST helpers
 * ================================================================== */

/*! @brief Vector destructor for one RCITEM*.
 *
 *  @param[in] pe  Element in the vector (a RCITEM*).
 */
static void APIENTRY destroy_item(PVOID pe)
{
    RCITEM *it = *(RCITEM **)pe;
    if (!it) return;
    if (it->pszText)  CclFree(it->pszText);
    if (it->pszClass) CclFree(it->pszClass);
    CclFree(it);
}

/*! @brief Create a vector of RCITEM* with a destroy callback.
 *
 *  @return New vector handle, or NULLHANDLE on failure.
 *
 *  @retval NULLHANDLE  Allocation failed.
 */
HVECTOR RcItemsVectorNew(void)
{
    HVECTOR h;
    CCLOPTS o;
    CclOptsInit(&o);
    o.pDestroyValue = destroy_item;
    if (VectorCreate(sizeof(RCITEM *), 0, &o, &h) != NO_ERROR)
        return NULLHANDLE;
    return h;
}

/*! @brief Allocate a dialog.
 *
 *  @param[in] name   Symbolic name; may be NULL.
 *  @param[in] ex     Non-zero for DIALOGEX.
 *  @param[in] items  Items vector; may be NULLHANDLE.
 *
 *  @return A new dialog, or NULL on failure.
 *
 *  @retval NULL  Allocation failed.
 */
RCDIALOG *RcDialogNew(PCSZ name, int ex, HVECTOR items)
{
    RCDIALOG *d = (RCDIALOG *)CclCalloc(1, sizeof(*d));
    if (!d) return NULL;
    d->pszName     = RcStrDup(name ? name : "");
    d->fDialogEx   = ex;
    d->hItems      = items;
    d->iFontSize   = 0;
    d->pszFontFace = NULL;
    return d;
}

/*! @brief Allocate a control.
 *
 *  @param[in] kind   ITEM_* kind.
 *  @param[in] text   Control text.
 *  @param[in] id     Control id.
 *  @param[in] cls    Class name.
 *  @param[in] style  Combined style.
 *  @param[in] x      Left coordinate.
 *  @param[in] y      Top coordinate.
 *  @param[in] cx     Width.
 *  @param[in] cy     Height.
 *
 *  @return A new control, or NULL on failure.
 *
 *  @retval NULL  Allocation failed.
 */
RCITEM *RcItemNew(int kind, PCSZ text, long id, PCSZ cls,
                  DWORD style, int x, int y, int cx, int cy)
{
    RCITEM *it = (RCITEM *)CclCalloc(1, sizeof(*it));
    if (!it) return NULL;
    it->iKind    = kind;
    it->pszText  = RcStrDup(text);
    it->lId      = id;
    it->pszClass = RcStrDup(cls);
    it->dwStyle  = style;
    it->x = x; it->y = y; it->cx = cx; it->cy = cy;
    return it;
}

/*! @brief Free a dialog and its items.
 *
 *  @param[in] d  Dialog; may be NULL.
 */
void RcDialogFree(RCDIALOG *d)
{
    if (!d) return;
    if (d->pszName)     { CclFree(d->pszName);     d->pszName     = NULL; }
    if (d->pszCaption)  { CclFree(d->pszCaption);  d->pszCaption  = NULL; }
    if (d->pszFontFace) { CclFree(d->pszFontFace); d->pszFontFace = NULL; }
    if (d->hItems)      { VectorDestroy(d->hItems); d->hItems     = NULLHANDLE; }
    CclFree(d);
}

/* ==================================================================
 * Parser entry
 * ================================================================== */

/*! @brief Parse a preprocessed RC body into g_pResult.
 *
 *  @param[in] body  NUL-terminated preprocessed text.
 *
 *  @return 0 on success, -1 on failure.
 *
 *  @retval  0  Parse succeeded.
 *  @retval -1  body is NULL or the scanner buffer could not be
 *              created.
 */
int RcParseBody(PCSZ body)
{
    struct yy_buffer_state *buf;
    int rc;
    if (!body) return -1;
    yy_line = 1;
    buf = yy_scan_string(body);
    if (!buf) return -1;
    rc = yyparse();
    yy_delete_buffer(buf);
    return rc;
}

/* ==================================================================
 * Preprocessor
 * ================================================================== */

/*! @brief Growable byte buffer. */
typedef struct _BUF { char *p; ULONG used; ULONG cap; } BUF;

/*! @brief Append bytes to a growable buffer.
 *
 *  @param[in,out] b  Buffer.
 *  @param[in]     s  Bytes to append.
 *  @param[in]     n  Number of bytes.
 *
 *  @return 1 on success, 0 on allocation failure.
 *
 *  @retval 1  Data appended.
 *  @retval 0  Allocation failed.
 */
static int buf_app(BUF *b, const char *s, ULONG n)
{
    if (b->used + n + 1 > b->cap) {
        ULONG nc = b->cap ? b->cap * 2 : 4096;
        char *np;
        while (nc < b->used + n + 1) nc *= 2;
        np = (char *)CclRealloc(b->p, nc);
        if (!np) return 0;
        b->p = np;
        b->cap = nc;
    }
    memcpy(b->p + b->used, s, n);
    b->used += n;
    b->p[b->used] = 0;
    return 1;
}

/*! @brief Scan one identifier token from a NUL-terminated string.
 *
 *  @param[in] r  Input.
 *
 *  @return Number of bytes in the identifier, or 0.
 *
 *  @retval 0  r is empty or does not start with [A-Za-z_].
 *  @retval >0 Length of the identifier at the start of r.
 */
static size_t scan_ident(PCSZ r)
{
    size_t n = 0;
    if (!r || !*r) return 0;
    if (!(*r == '_' || (*r >= 'A' && *r <= 'Z') || (*r >= 'a' && *r <= 'z')))
        return 0;
    n = 1;
    while (r[n] &&
           (r[n] == '_' ||
            (r[n] >= 'A' && r[n] <= 'Z') ||
            (r[n] >= 'a' && r[n] <= 'z') ||
            (r[n] >= '0' && r[n] <= '9')))
        n++;
    return n;
}

/*! @brief Resolve the value of a #define replacement.
 *
 *  @param[in] hSyms  Symbol table.
 *  @param[in] r      Replacement text.
 *
 *  @return Resolved numeric value, 0 if unresolved.
 *
 *  @retval 0  The replacement is empty, non-numeric, or names an
 *             unknown symbol.
 *  @retval >0 The numeric value of the replacement.
 */
static long resolve_define_value(HHASHTABLE hSyms, PCSZ r)
{
    char *end;
    long v;
    if (!r || !*r) return 0;
    v = strtol(r, &end, 0);
    if (end != r) return v;
    {
        size_t idlen = scan_ident(r);
        if (idlen > 0 && idlen < 64) {
            char idbuf[64];
            long symval = 0;
            memcpy(idbuf, r, idlen);
            idbuf[idlen] = 0;
            if (RcSymLookup(hSyms, idbuf, &symval) == NO_ERROR)
                return symval;
        }
    }
    return 0;
}

/*! @brief Recursively preprocess one .rc file.
 *
 *  @param[in]     path   Path to the file.
 *  @param[in]     hSyms  Symbol table for #define.
 *  @param[in,out] out    Accumulating buffer.
 *  @param[in]     depth  Recursion depth for #include.
 *
 *  @return APIRET.
 *
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_DATA       Include depth exceeded.
 *  @retval ERROR_OPEN_FAILED        File cannot be opened.
 */
static APIRET pp_file(PCSZ path, HHASHTABLE hSyms, BUF *out, int depth)
{
    FILE *fp;
    char line[2048];
    if (depth > 16) return ERROR_INVALID_DATA;
    fp = fopen(path, "r");
    if (!fp) return ERROR_OPEN_FAILED;
    while (fgets(line, sizeof(line), fp)) {
        char *p = line;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '#') {
            char *c = p + 1;
            while (*c == ' ' || *c == '\t') c++;
            if (strncmp(c, "define", 6) == 0 &&
                (c[6] == ' ' || c[6] == '\t')) {
                char name[64];
                char *r;
                char *q;
                long v;
                r = c + 6;
                while (*r == ' ' || *r == '\t') r++;
                q = name;
                while (*r && *r != ' ' && *r != '\t' &&
                       *r != '\r' && *r != '\n' &&
                       (size_t)(q - name) < sizeof(name) - 1)
                    *q++ = *r++;
                *q = 0;
                while (*r == ' ' || *r == '\t') r++;
                v = resolve_define_value(hSyms, r);
                RcSymAdd(hSyms, name, v);
                continue;
            }
            if (strncmp(c, "include", 7) == 0) {
                char *r = c + 7;
                while (*r == ' ' || *r == '\t') r++;
                if (*r == '"') {
                    char path2[512];
                    char *q = path2;
                    r++;
                    while (*r && *r != '"' &&
                           (size_t)(q - path2) < sizeof(path2) - 1)
                        *q++ = *r++;
                    *q = 0;
                    pp_file(path2, hSyms, out, depth + 1);
                }
                continue;
            }
            continue;
        }
        buf_app(out, line, (ULONG)strlen(line));
    }
    fclose(fp);
    return NO_ERROR;
}

/*! @brief Preprocess an .rc file into a flat NUL-terminated body.
 *
 *  @param[in]  path   Path to the .rc file.
 *  @param[in]  hSyms  Symbol table.
 *  @param[out] pp     Receiver for the body pointer.
 *  @param[out] pcb    Receiver for the body length.
 *
 *  @return APIRET.
 *
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_DATA       Include depth exceeded.
 *  @retval ERROR_OPEN_FAILED        File cannot be opened.
 *  @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET RcPreprocess(PCSZ path, HHASHTABLE hSyms, char **pp, ULONG *pcb)
{
    BUF b;
    APIRET rc;
    memset(&b, 0, sizeof(b));
    buf_app(&b, "", 0);
    rc = pp_file(path, hSyms, &b, 0);
    if (rc != NO_ERROR) {
        if (b.p) CclFree(b.p);
        return rc;
    }
    *pp  = b.p;
    *pcb = b.used;
    return NO_ERROR;
}

/* ==================================================================
 * Driver
 * ================================================================== */

/*! @brief Convert one RC dialog into a Win16 DLGTEMPLATE builder.
 *
 *  @param[in] argc  Argument count.
 *  @param[in] argv  Argument vector.
 *
 *  @return 0 on success, 1 on failure.
 *
 *  @retval 0  Success.
 *  @retval 1  Bad arguments, allocation failure, or parse failure.
 */
int main(int argc, char **argv)
{
    PCSZ inPath = NULL;
    PCSZ outPath = NULL;
    PCSZ dlgName = NULL;
    PCSZ mode = "function";
    char *body = NULL;
    ULONG bodyLen = 0;
    APIRET rc;
    FILE *out;
    int i;
    char funcName[128];
    PCSZ toParse = NULL;
    char *fragBuf = NULL;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--input") == 0 && i + 1 < argc)
            inPath = argv[++i];
        else if (strcmp(argv[i], "--output") == 0 && i + 1 < argc)
            outPath = argv[++i];
        else if (strcmp(argv[i], "--dialog") == 0 && i + 1 < argc)
            dlgName = argv[++i];
        else if (strcmp(argv[i], "--mode") == 0 && i + 1 < argc)
            mode = argv[++i];
        else if (!inPath && argv[i][0] != '-')
            inPath = argv[i];
    }
    if (!inPath || !outPath) {
        fprintf(stderr,
                "usage: rc2c [--input] <.rc> --output <.c> "
                "[--dialog NAME] [--mode function|array]\n");
        return 1;
    }

    rc = RcSymInit(&g_hSyms);
    if (rc != NO_ERROR) {
        fprintf(stderr, "rc2c: symtab init failed (%lu)\n",
                (unsigned long)rc);
        return 1;
    }

    rc = RcPreprocess(inPath, g_hSyms, &body, &bodyLen);
    if (rc != NO_ERROR) {
        fprintf(stderr, "rc2c: preprocess failed (%lu)\n",
                (unsigned long)rc);
        return 1;
    }

    if (dlgName) {
        size_t flen = 0;
        PCSZ frag = extract_dialog(body, dlgName, &flen);
        if (!frag) {
            fprintf(stderr, "rc2c: dialog '%s' not found\n", dlgName);
            if (body) CclFree(body);
            HashtableDestroy(g_hSyms);
            return 1;
        }
        fragBuf = (char *)CclMalloc(flen + 1);
        if (!fragBuf) {
            fprintf(stderr, "rc2c: out of memory\n");
            if (body) CclFree(body);
            HashtableDestroy(g_hSyms);
            return 1;
        }
        memcpy(fragBuf, frag, flen);
        fragBuf[flen] = 0;
        toParse = fragBuf;
    } else {
        toParse = body;
    }

    if (RcParseBody(toParse) != 0 || !g_pResult) {
        fprintf(stderr, "rc2c: parse failed\n");
        if (fragBuf) CclFree(fragBuf);
        if (body) CclFree(body);
        HashtableDestroy(g_hSyms);
        return 1;
    }

    if (dlgName && g_pResult->pszName &&
        strcmp(g_pResult->pszName, dlgName) != 0) {
        fprintf(stderr, "rc2c: dialog '%s' not found\n", dlgName);
        if (fragBuf) CclFree(fragBuf);
        if (body) CclFree(body);
        HashtableDestroy(g_hSyms);
        return 1;
    }

    {
        char *dst;
        PCSZ s = g_pResult->pszName ? g_pResult->pszName : "Dlg";
        if (strncmp(s, "IDD_", 4) == 0) s += 4;
        dst = funcName;
        strcpy(dst, "Build");
        dst += 5;
        while (*s && (size_t)(dst - funcName) < sizeof(funcName) - 16) {
            if (*s == '_') { s++; continue; }
            *dst++ = *s++;
        }
        *dst = '\0';
        strcat(funcName, "Template");
    }

    out = fopen(outPath, "w");
    if (!out) {
        fprintf(stderr, "rc2c: cannot open %s\n", outPath);
        if (fragBuf) CclFree(fragBuf);
        if (body) CclFree(body);
        HashtableDestroy(g_hSyms);
        return 1;
    }
    if (strcmp(mode, "function") == 0)
        RcEmitFunction(out, g_pResult, funcName);
    else
        RcEmitArray(out, g_pResult, funcName);
    fclose(out);

    RcDialogFree(g_pResult);
    if (fragBuf) CclFree(fragBuf);
    if (body) CclFree(body);
    HashtableDestroy(g_hSyms);
    return 0;
}
