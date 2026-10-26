/*! @file rc2c_emit.c
 *  @brief Win16 DLGTEMPLATE code emission.
 */

#include <stdio.h>
#include <string.h>
#include "rc2c_ast.h"

/* ==================================================================
 * String literal emission
 * ================================================================== */

/*! @brief Emit one C string literal.
 *
 *  Bytes below 0x20 are escaped; everything else, including bytes
 *  above 0x7F, is copied verbatim so the generated .c preserves the
 *  source bytes of the RC field exactly.
 *
 *  @param[in] fp  Output stream.
 *  @param[in] s   String; NULL is treated as empty.
 */
static void emit_str(FILE *fp, PCSZ s)
{
    PCSZ p = s ? s : "";
    fputc('"', fp);
    for (; *p; p++) {
        unsigned char c = (unsigned char)*p;
        switch (c) {
        case '\\': fputs("\\\\", fp); break;
        case '"':  fputs("\\\"", fp); break;
        case '\n': fputs("\\n",  fp); break;
        case '\r': fputs("\\r",  fp); break;
        case '\t': fputs("\\t",  fp); break;
        default:
            if (c < 0x20) fprintf(fp, "\\x%02X", c);
            else fputc(c, fp);
        }
    }
    fputc('"', fp);
}

/* ==================================================================
 * Size computation
 * ================================================================== */

/*! @brief Size of one DLGITEMTEMPLATE in the emitted template.
 *
 *  @param[in] it  Control.
 *
 *  @return Size in bytes, including the trailing dtilInfo byte.
 */
static ULONG item_size(RCITEM *it)
{
    ULONG s = 0;
    s += 2 * 5;
    s += 4;
    s += (ULONG)strlen(it->pszClass ? it->pszClass : "") + 1;
    s += (ULONG)strlen(it->pszText  ? it->pszText  : "") + 1;
    s += 1;
    return s;
}

/*! @brief Total size of the emitted template.
 *
 *  @param[in] d       Dialog.
 *  @param[in] nItems  Number of controls.
 *
 *  @return Size in bytes.
 */
static ULONG total_size(RCDIALOG *d, ULONG nItems)
{
    ULONG cb = 14;
    ULONG i;
    RCITEM *it;
    ULONG used;
    cb += 1;
    cb += 1;
    cb += (ULONG)strlen(d->pszCaption ? d->pszCaption : "") + 1;
    if ((d->dwStyle & DS_SETFONT) && d->pszFontFace && d->iFontSize > 0) {
        cb += 2;
        cb += (ULONG)strlen(d->pszFontFace) + 1;
    }
    for (i = 0; i < nItems; i++) {
        VectorGetItem(d->hItems, i, &it, sizeof(it), &used);
        cb += item_size(it);
    }
    return cb;
}

/* ==================================================================
 * Helper emission
 * ================================================================== */

/*! @brief Emit the static DlgTplPut* helper set.
 *
 *  @param[in] fp  Output stream.
 */
static void emit_helpers(FILE *fp)
{
    fputs(
"/*! @brief Write a WORD into the template.\n"
" *\n"
" *  @param[in,out] p  Current write pointer; may be NULL for a\n"
" *                    size-only pass, in which case no store is\n"
" *                    performed.\n"
" *  @param[in]     w  Value to write.\n"
" *\n"
" *  @return The advanced write pointer.\n"
" */\n"
"static LPSTR DlgTplPutWord(LPSTR p, WORD w)\n"
"{\n"
"    if (p) *(WORD FAR *)p = w;\n"
"    return p + sizeof(WORD);\n"
"}\n"
"\n"
"/*! @brief Write a DWORD into the template.\n"
" *\n"
" *  @param[in,out] p  Current write pointer; may be NULL for a\n"
" *                    size-only pass.\n"
" *  @param[in]     d  Value to write.\n"
" *\n"
" *  @return The advanced write pointer.\n"
" */\n"
"static LPSTR DlgTplPutDword(LPSTR p, DWORD d)\n"
"{\n"
"    if (p) *(DWORD FAR *)p = d;\n"
"    return p + sizeof(DWORD);\n"
"}\n"
"\n"
"/*! @brief Write a NUL-terminated string into the template.\n"
" *\n"
" *  @param[in,out] p  Current write pointer; may be NULL for a\n"
" *                    size-only pass.\n"
" *  @param[in]     s  String to write; not NULL.\n"
" *\n"
" *  @return The advanced write pointer.\n"
" */\n"
"static LPSTR DlgTplPutStr(LPSTR p, LPCSTR s)\n"
"{\n"
"    if (p) { while (*s) *p++ = *s++; *p++ = 0; }\n"
"    else   { while (*s) { p++; s++; } p++; }\n"
"    return p;\n"
"}\n"
"\n"
"/*! @brief Write one DLGITEMTEMPLATE.\n"
" *\n"
" *  @param[in,out] p      Current write pointer; may be NULL for a\n"
" *                        size-only pass.\n"
" *  @param[in]     x      Left coordinate.\n"
" *  @param[in]     y      Top coordinate.\n"
" *  @param[in]     cx     Width.\n"
" *  @param[in]     cy     Height.\n"
" *  @param[in]     id     Control id.\n"
" *  @param[in]     style  Combined style flags.\n"
" *  @param[in]     cls    Class name (NUL-terminated).\n"
" *  @param[in]     text   Control text (NUL-terminated).\n"
" *\n"
" *  @return The advanced write pointer.\n"
" */\n"
"static LPSTR DlgTplAddItem(LPSTR p,\n"
"                           short x, short y, short cx, short cy,\n"
"                           WORD id, DWORD style,\n"
"                           LPCSTR cls, LPCSTR text)\n"
"{\n"
"    p = DlgTplPutWord(p, (WORD)x);\n"
"    p = DlgTplPutWord(p, (WORD)y);\n"
"    p = DlgTplPutWord(p, (WORD)cx);\n"
"    p = DlgTplPutWord(p, (WORD)cy);\n"
"    p = DlgTplPutWord(p, id);\n"
"    p = DlgTplPutDword(p, style);\n"
"    p = DlgTplPutStr(p, cls);\n"
"    p = DlgTplPutStr(p, text);\n"
"    if (p) *p = 0;\n"
"    p += 1;\n"
"    return p;\n"
"}\n"
"\n", fp);
}

/* ==================================================================
 * Header emission
 * ================================================================== */

/*! @brief Emit the DLGTEMPLATE header.
 *
 *  @param[in] fp      Output stream.
 *  @param[in] d       Dialog.
 *  @param[in] nItems  Number of controls.
 */
static void emit_header(FILE *fp, RCDIALOG *d, int nItems)
{
    char styleExpr[256];
    RcStyleToExpr(d->dwStyle, STYLE_CTX_DIALOG,
                  styleExpr, sizeof(styleExpr));

    fputs("    /* DLGTEMPLATE header */\n", fp);
    fprintf(fp, "    p = DlgTplPutDword(p, %s);\n", styleExpr);
    fprintf(fp, "    p = DlgTplPutWord(p, %d);  /* dtItemCount */\n",
            nItems);
    fprintf(fp, "    p = DlgTplPutWord(p, %d);  /* dtX */\n", d->x);
    fprintf(fp, "    p = DlgTplPutWord(p, %d);  /* dtY */\n", d->y);
    fprintf(fp, "    p = DlgTplPutWord(p, %d);  /* dtCX */\n", d->cx);
    fprintf(fp, "    p = DlgTplPutWord(p, %d);  /* dtCY */\n", d->cy);
    fprintf(fp, "    p = DlgTplPutStr(p, \"\"); /* dtMenuName: none */\n");
    fprintf(fp, "    p = DlgTplPutStr(p, \"\"); /* dtClassName: default */\n");
    fprintf(fp, "    p = DlgTplPutStr(p, ");
    emit_str(fp, d->pszCaption ? d->pszCaption : "");
    fputs(");\n", fp);

    if ((d->dwStyle & DS_SETFONT) && d->pszFontFace && d->iFontSize > 0) {
        fprintf(fp, "    p = DlgTplPutWord(p, %d);  /* font point size */\n",
                d->iFontSize);
        fprintf(fp, "    p = DlgTplPutStr(p, ");
        emit_str(fp, d->pszFontFace);
        fputs(");  /* font face */\n", fp);
    }
    fputs("\n", fp);
}

/* ==================================================================
 * Item emission
 * ================================================================== */

/*! @brief Emit one DlgTplAddItem call.
 *
 *  @param[in] fp  Output stream.
 *  @param[in] it  Control.
 */
static void emit_item(FILE *fp, RCITEM *it)
{
    char styleExpr[256];
    ULONG ctx = RcStyleCtxOf(it->iKind, it->pszClass);
    RcStyleToExpr(it->dwStyle, ctx, styleExpr, sizeof(styleExpr));

    fprintf(fp, "    /* %s id=%ld */\n",
            it->pszClass ? it->pszClass : "?", it->lId);
    fprintf(fp, "    p = DlgTplAddItem(p,\n");
    fprintf(fp, "                       %d, %d, %d, %d, %ld,\n",
            it->x, it->y, it->cx, it->cy, it->lId);
    fprintf(fp, "                       %s,\n", styleExpr);
    fprintf(fp, "                       ");
    emit_str(fp, it->pszClass ? it->pszClass : "");
    fputs(", ", fp);
    emit_str(fp, it->pszText  ? it->pszText  : "");
    fputs(");\n\n", fp);
}

/* ==================================================================
 * Function emission
 * ================================================================== */

/*! @brief Emit a Build*Template function.
 *
 *  @param[in] fp  Output stream.
 *  @param[in] d   Dialog.
 *  @param[in] fn  Function name.
 *
 *  @return APIRET.
 *
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  Any argument is NULL.
 */
APIRET RcEmitFunction(FILE *fp, RCDIALOG *d, PCSZ fn)
{
    ULONG nItems = 0;
    ULONG cb;
    ULONG i;
    RCITEM *it;
    ULONG used;

    if (!fp || !d || !fn) return ERROR_INVALID_PARAMETER;
    VectorGetCount(d->hItems, &nItems);
    cb = total_size(d, nItems);

    fprintf(fp, "/*\n");
    fprintf(fp, " * SPDX-FileCopyrightText: (C) 2026 osFree Project\n");
    fprintf(fp, " * SPDX-License-Identifier: LGPL-2.1-or-later\n");
    fprintf(fp, " */\n\n");

    fprintf(fp, "/*! @file %s.c\n", fn);
    fprintf(fp, " *  @brief Win16 DLGTEMPLATE builder (generated by rc2c).\n");
    fprintf(fp, " */\n\n");
    fprintf(fp, "#include <windows.h>\n\n");

    emit_helpers(fp);

    fprintf(fp, "/*! @brief Build the dialog template in a global block.\n");
    fprintf(fp, " *\n");
    fprintf(fp, " *  @return HGLOBAL handle, or NULL on allocation failure.\n");
    fprintf(fp, " *\n");
    fprintf(fp, " *  @retval NULL  GlobalAlloc or GlobalLock failed.\n");
    fprintf(fp, " */\n");
    fprintf(fp, "HGLOBAL %s(void)\n{\n", fn);
    fprintf(fp, "    HGLOBAL hTpl;\n");
    fprintf(fp, "    LPSTR base, p;\n\n");
    fprintf(fp,
            "    hTpl = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, "
            "%luUL);\n", (unsigned long)cb);
    fprintf(fp, "    if (!hTpl) return NULL;\n");
    fprintf(fp, "    base = (LPSTR)GlobalLock(hTpl);\n");
    fprintf(fp, "    if (!base) { GlobalFree(hTpl); return NULL; }\n");
    fprintf(fp, "    p = base;\n\n");

    emit_header(fp, d, (int)nItems);

    for (i = 0; i < nItems; i++) {
        VectorGetItem(d->hItems, i, &it, sizeof(it), &used);
        emit_item(fp, it);
    }

    fprintf(fp, "    GlobalUnlock(hTpl);\n");
    fprintf(fp, "    return hTpl;\n");
    fprintf(fp, "}\n");
    return NO_ERROR;
}

/* ==================================================================
 * Array emission (stub)
 * ================================================================== */

/*! @brief Emit a static byte array (stub).
 *
 *  @param[in] fp   Output stream.
 *  @param[in] d    Dialog.
 *  @param[in] arr  Array name.
 *
 *  @return APIRET.
 *
 *  @retval ERROR_NOT_SUPPORTED  Not implemented.
 */
APIRET RcEmitArray(FILE *fp, RCDIALOG *d, PCSZ arr)
{
    (void)fp;
    (void)d;
    (void)arr;
    return ERROR_NOT_SUPPORTED;
}
