%{
/*! @file rc2c_parse.y
 *  @brief wYacc grammar for the RC dialog subset. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rc2c_ast.h"

/*! @brief Parse the token stream produced by yylex.
 *
 *  @return 0 on success, non-zero on error.
 */
extern int  yyparse(void);

/*! @brief Read one token from the lexer.
 *
 *  @return The token code.
 */
extern int  yylex(void);

/*! @brief Current source line, maintained by the lexer. */
extern int  yy_line;

/*! @brief Report a parse error.
 *
 *  @param[in] s  Message from yacc.
 */
void yyerror(const char *s);

/*! @def MKBTN
 *  @brief Create one button item and append it to the current dialog.
 *
 *  Default flags: WS_CHILD | WS_VISIBLE | WS_TABSTOP.
 *
 *  @param[in] kind  ITEM_* kind.
 *  @param[in] txt   NUL string; freed after RcItemNew copies it.
 *  @param[in] idv   Control id.
 *  @param[in] cls   Class name string.
 *  @param[in] stl   Style bits.
 *  @param[in] xv    Left coordinate.
 *  @param[in] yv    Top coordinate.
 *  @param[in] cxv   Width.
 *  @param[in] cyv   Height.
 */
#define MKBTN(kind, txt, idv, cls, stl, xv, yv, cxv, cyv) do { \
    RCITEM *it = RcItemNew((kind), (txt), (long)(idv), (cls), \
        (stl) | WS_CHILD | WS_VISIBLE | WS_TABSTOP, \
        (int)(xv), (int)(yv), (int)(cxv), (int)(cyv)); \
    if (it) VectorAdd(g_hItems, &it); \
    RcStrFree(txt); \
} while (0)

/*! @def MKST
 *  @brief Create one non-button item and append it to the current
 *         dialog.
 *
 *  Default flags: WS_CHILD | WS_VISIBLE (no WS_TABSTOP).
 *
 *  @param[in] kind  ITEM_* kind.
 *  @param[in] txt   NUL string; freed after RcItemNew copies it.
 *  @param[in] idv   Control id.
 *  @param[in] cls   Class name string.
 *  @param[in] stl   Style bits.
 *  @param[in] xv    Left coordinate.
 *  @param[in] yv    Top coordinate.
 *  @param[in] cxv   Width.
 *  @param[in] cyv   Height.
 */
#define MKST(kind, txt, idv, cls, stl, xv, yv, cxv, cyv) do { \
    RCITEM *it = RcItemNew((kind), (txt), (long)(idv), (cls), \
        (stl) | WS_CHILD | WS_VISIBLE, \
        (int)(xv), (int)(yv), (int)(cxv), (int)(cyv)); \
    if (it) VectorAdd(g_hItems, &it); \
    RcStrFree(txt); \
} while (0)
%}

%union {
    long          lNum;
    char         *pszStr;
    unsigned long dwStyle;
}

%token <lNum> NUM
%token <pszStr> IDENT STRING
%token DIALOG DIALOGEX CAPTION STYLE FONT
%token BEGIN_ END_
%token DEFPUSHBUTTON PUSHBUTTON LTEXT CTEXT RTEXT
%token ICON EDITTEXT CONTROL GROUPBOX CHECKBOX RADIOBUTTON
%token COMMA PIPE PLUS

%type <lNum>    number
%type <pszStr>  class_id
%type <dwStyle> style_list
%type <dwStyle> opt_style_tail

%%

program
    : dialog_def
    ;

dialog_def
    : IDENT DIALOG
        number COMMA number COMMA number COMMA number opt_style_tail
        { g_pCurDlg = RcDialogNew($1, 0, NULLHANDLE);
          if (g_pCurDlg) {
              g_pCurDlg->x  = (int)$3;
              g_pCurDlg->y  = (int)$5;
              g_pCurDlg->cx = (int)$7;
              g_pCurDlg->cy = (int)$9;
              g_pCurDlg->dwStyle = $10;
          }
          RcStrFree($1);
        }
        directives BEGIN_ item_list END_
        { if (g_pCurDlg) {
              g_pCurDlg->hItems = g_hItems;
              g_hItems = NULLHANDLE;
          }
          g_pResult = g_pCurDlg;
        }
    | IDENT DIALOGEX
        number COMMA number COMMA number COMMA number opt_style_tail
        { g_pCurDlg = RcDialogNew($1, 1, NULLHANDLE);
          if (g_pCurDlg) {
              g_pCurDlg->x  = (int)$3;
              g_pCurDlg->y  = (int)$5;
              g_pCurDlg->cx = (int)$7;
              g_pCurDlg->cy = (int)$9;
              g_pCurDlg->dwStyle = $10;
          }
          RcStrFree($1);
        }
        directives BEGIN_ item_list END_
        { if (g_pCurDlg) {
              g_pCurDlg->hItems = g_hItems;
              g_hItems = NULLHANDLE;
          }
          g_pResult = g_pCurDlg;
        }
    ;

directives
    : /* empty */
    | directives directive
    ;

directive
    : CAPTION STRING
        { if (g_pCurDlg) {
              if (g_pCurDlg->pszCaption) RcStrFree(g_pCurDlg->pszCaption);
              g_pCurDlg->pszCaption = $2;
          } else {
              RcStrFree($2);
          }
        }
    | STYLE style_list
        { if (g_pCurDlg) g_pCurDlg->dwStyle |= $2; }
    | FONT number COMMA STRING
        { if (g_pCurDlg) {
              g_pCurDlg->iFontSize = (int)$2;
              if (g_pCurDlg->pszFontFace) RcStrFree(g_pCurDlg->pszFontFace);
              g_pCurDlg->pszFontFace = $4;
              g_pCurDlg->dwStyle |= DS_SETFONT;
          } else {
              RcStrFree($4);
          }
        }
    ;

item_list
    : { g_hItems = RcItemsVectorNew(); }
    | item_list item
    ;

item
    : DEFPUSHBUTTON STRING COMMA number COMMA number COMMA number COMMA number COMMA number
        { MKBTN(ITEM_DEFPUSHBUTTON, $2, $4, "BUTTON",
                BS_DEFPUSHBUTTON,
                $6, $8, $10, $12); }
    | PUSHBUTTON    STRING COMMA number COMMA number COMMA number COMMA number COMMA number
        { MKBTN(ITEM_PUSHBUTTON, $2, $4, "BUTTON",
                BS_PUSHBUTTON,
                $6, $8, $10, $12); }
    | LTEXT         STRING COMMA number COMMA number COMMA number COMMA number COMMA number
        { MKST(ITEM_LTEXT, $2, $4, "STATIC",
               SS_LEFT,
               $6, $8, $10, $12); }
    | CTEXT         STRING COMMA number COMMA number COMMA number COMMA number COMMA number
        { MKST(ITEM_CTEXT, $2, $4, "STATIC",
               SS_CENTER,
               $6, $8, $10, $12); }
    | RTEXT         STRING COMMA number COMMA number COMMA number COMMA number COMMA number
        { MKST(ITEM_RTEXT, $2, $4, "STATIC",
               SS_RIGHT,
               $6, $8, $10, $12); }
    | GROUPBOX      STRING COMMA number COMMA number COMMA number COMMA number COMMA number
        { MKST(ITEM_GROUPBOX, $2, $4, "BUTTON",
               BS_GROUPBOX,
               $6, $8, $10, $12); }
    | CHECKBOX      STRING COMMA number COMMA number COMMA number COMMA number COMMA number
        { MKBTN(ITEM_CHECKBOX, $2, $4, "BUTTON",
                BS_CHECKBOX,
                $6, $8, $10, $12); }
    | RADIOBUTTON   STRING COMMA number COMMA number COMMA number COMMA number COMMA number
        { MKBTN(ITEM_RADIOBUTTON, $2, $4, "BUTTON",
                BS_RADIOBUTTON,
                $6, $8, $10, $12); }
    | ICON          number COMMA number COMMA number COMMA number COMMA number COMMA number
        { MKST(ITEM_ICON, "", $4, "STATIC",
               SS_ICON,
               $6, $8, $10, $12); }
    | EDITTEXT      number COMMA number COMMA number COMMA number COMMA number opt_style_tail
        { MKBTN(ITEM_EDITTEXT, "", $2, "EDIT",
                ES_LEFT | WS_BORDER | $11,
                $4, $6, $8, $10); }
    | CONTROL STRING COMMA number COMMA class_id COMMA style_list
              COMMA number COMMA number COMMA number COMMA number
        { MKST(ITEM_CONTROL, $2, $4, $6, $8,
               $10, $12, $14, $16);
          RcStrFree($2);
          RcStrFree($6);
        }
    ;

opt_style_tail
    : { $$ = 0UL; }
    | COMMA style_list { $$ = $2; }
    ;

style_list
    : IDENT
        { long v = 0;
          if (RcSymLookup(g_hSyms, $1, &v) != NO_ERROR) {
              char m[160];
              sprintf(m, "unknown style: %s", $1);
              RcDiagError(yy_line, m);
              v = 0;
          }
          $$ = (unsigned long)v;
          RcStrFree($1);
        }
    | style_list PIPE IDENT
        { long v = 0;
          if (RcSymLookup(g_hSyms, $3, &v) != NO_ERROR) {
              char m[160];
              sprintf(m, "unknown style: %s", $3);
              RcDiagError(yy_line, m);
              v = 0;
          }
          $$ = $1 | (unsigned long)v;
          RcStrFree($3);
        }
    | style_list PLUS IDENT
        { long v = 0;
          if (RcSymLookup(g_hSyms, $3, &v) != NO_ERROR) {
              char m[160];
              sprintf(m, "unknown style: %s", $3);
              RcDiagError(yy_line, m);
              v = 0;
          }
          $$ = $1 | (unsigned long)v;
          RcStrFree($3);
        }
    ;

class_id
    : IDENT  { $$ = $1; }
    | STRING { $$ = $1; }
    ;

number
    : NUM { $$ = $1; }
    | IDENT
        { long v = 0;
          if (RcSymLookup(g_hSyms, $1, &v) != NO_ERROR) {
              char m[160];
              sprintf(m, "unknown symbol: %s", $1);
              RcDiagError(yy_line, m);
              v = 0;
          }
          $$ = v;
          RcStrFree($1);
        }
    ;

%%

/*! @brief Parse error handler.
 *
 *  @param[in] s  Message from yacc.
 */
void yyerror(const char *s)
{
    fprintf(stderr, "rc2c: parse error (line %d): %s\n", yy_line, s);
}
