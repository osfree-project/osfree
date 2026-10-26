/*! @file rc2c_ast.h
 *  @brief Public interface of the RC-to-DLGTEMPLATE converter.
 */

#ifndef RC2C_AST_H
#define RC2C_AST_H

#include <stdio.h>
#include "os2types.h"
#include "os2err.h"
#include "ccl.h"
#include "hashtable.h"
#include "vector.h"

/* ==================================================================
 * Item kinds
 * ================================================================== */

/*! @brief DEFPUSHBUTTON. Value: 1. */
#define ITEM_DEFPUSHBUTTON 1
/*! @brief PUSHBUTTON. Value: 2. */
#define ITEM_PUSHBUTTON    2
/*! @brief LTEXT. Value: 3. */
#define ITEM_LTEXT         3
/*! @brief CTEXT. Value: 4. */
#define ITEM_CTEXT         4
/*! @brief RTEXT. Value: 5. */
#define ITEM_RTEXT         5
/*! @brief ICON. Value: 6. */
#define ITEM_ICON          6
/*! @brief EDITTEXT. Value: 7. */
#define ITEM_EDITTEXT      7
/*! @brief GROUPBOX. Value: 8. */
#define ITEM_GROUPBOX      8
/*! @brief CONTROL. Value: 9. */
#define ITEM_CONTROL       9
/*! @brief CHECKBOX. Value: 10. */
#define ITEM_CHECKBOX     10
/*! @brief RADIOBUTTON. Value: 11. */
#define ITEM_RADIOBUTTON  11

/* ==================================================================
 * Style contexts
 * ================================================================== */

/*! @brief Dialog style context. Value: 0x01. */
#define STYLE_CTX_DIALOG  0x01
/*! @brief Button style context. Value: 0x02. */
#define STYLE_CTX_BUTTON  0x02
/*! @brief Static style context. Value: 0x04. */
#define STYLE_CTX_STATIC  0x04
/*! @brief Edit style context. Value: 0x08. */
#define STYLE_CTX_EDIT    0x08
/*! @brief All contexts. */
#define STYLE_CTX_ALL     (STYLE_CTX_DIALOG | STYLE_CTX_BUTTON | \
                           STYLE_CTX_STATIC | STYLE_CTX_EDIT)

/* ==================================================================
 * Win16 window styles
 * ================================================================== */

/*! @brief Popup window. Value: 0x80000000. */
#define WS_POPUP        0x80000000L
/*! @brief Child window. Value: 0x40000000. */
#define WS_CHILD        0x40000000L
/*! @brief Initially visible. Value: 0x10000000. */
#define WS_VISIBLE      0x10000000L
/*! @brief Thin border. Value: 0x00800000. */
#define WS_BORDER       0x00800000L
/*! @brief Double border. Value: 0x00400000. */
#define WS_DLGFRAME     0x00400000L
/*! @brief System menu. Value: 0x00080000. */
#define WS_SYSMENU      0x00080000L
/*! @brief Tab stop. Value: 0x00010000. */
#define WS_TABSTOP      0x00010000L
/*! @brief Group start. Value: 0x00020000. */
#define WS_GROUP        0x00020000L
/*! @brief Standard popup-window combination. */
#define WS_POPUPWINDOW  (WS_POPUP | WS_BORDER | WS_SYSMENU)
/*! @brief Modal dialog frame. Value: 0x0080. */
#define DS_MODALFRAME   0x0080L
/*! @brief Dialog uses FONTINFO after the caption. Value: 0x0040. */
#define DS_SETFONT      0x0040L

/* ==================================================================
 * Win16 button styles
 * ================================================================== */

/*! @brief Default push button. Value: 0x00000001. */
#define BS_DEFPUSHBUTTON 0x00000001L
/*! @brief Regular push button. Value: 0x00000000. */
#define BS_PUSHBUTTON    0x00000000L
/*! @brief Group box. Value: 0x00000007. */
#define BS_GROUPBOX      0x00000007L
/*! @brief Check box. Value: 0x00000002. */
#define BS_CHECKBOX      0x00000002L
/*! @brief Radio button. Value: 0x00000004. */
#define BS_RADIOBUTTON   0x00000004L

/* ==================================================================
 * Win16 static styles
 * ================================================================== */

/*! @brief Left-aligned text. Value: 0x00000000. */
#define SS_LEFT          0x00000000L
/*! @brief Centered text. Value: 0x00000001. */
#define SS_CENTER        0x00000001L
/*! @brief Right-aligned text. Value: 0x00000002. */
#define SS_RIGHT         0x00000002L
/*! @brief Icon. Value: 0x00000003. */
#define SS_ICON          0x00000003L

/* ==================================================================
 * Win16 edit styles
 * ================================================================== */

/*! @brief Left-aligned edit. Value: 0x00000000. */
#define ES_LEFT          0x00000000L

/* ==================================================================
 * Types
 * ================================================================== */

/*! @brief One style constant, its value, and the contexts where it
 *         is preferred. */
typedef struct _STYLEDEF {
    PCSZ  pszName;   /*!< Name as it appears in RC / windows.h.       */
    DWORD dwValue;   /*!< Numeric value.                              */
    ULONG ulCtx;     /*!< STYLE_CTX_* mask where this name applies.   */
} STYLEDEF;

/*! @brief Number of entries in g_aStyles. */
extern const ULONG g_cStyles;

/*! @brief Style name/value/context table. */
extern const STYLEDEF g_aStyles[];

/*! @brief One dialog control. */
typedef struct _RCITEM {
    int   iKind;        /*!< ITEM_* kind.                  */
    char *pszText;      /*!< NUL string, owned.            */
    long  lId;          /*!< Control id.                   */
    char *pszClass;     /*!< "BUTTON"/"STATIC"/..., owned. */
    DWORD dwStyle;      /*!< Combined style.               */
    int   x;            /*!< Left coordinate.              */
    int   y;            /*!< Top coordinate.               */
    int   cx;           /*!< Width.                        */
    int   cy;           /*!< Height.                       */
} RCITEM;

/*! @brief One dialog. */
typedef struct _RCDIALOG {
    int      fDialogEx;    /*!< DIALOGEX flag.                    */
    char    *pszName;      /*!< Symbolic name, owned.             */
    int      x;            /*!< Left coordinate.                  */
    int      y;            /*!< Top coordinate.                   */
    int      cx;           /*!< Width.                            */
    int      cy;           /*!< Height.                           */
    DWORD    dwStyle;      /*!< Combined style.                   */
    char    *pszCaption;   /*!< Caption, owned.                   */
    int      iFontSize;    /*!< FONT point size, 0 if no FONT.    */
    char    *pszFontFace;  /*!< FONT typeface, owned; NULL if none. */
    HVECTOR  hItems;       /*!< Vector of RCITEM*.                */
} RCDIALOG;

/* ==================================================================
 * String helpers
 * ================================================================== */

/*! @brief Duplicate a string with the CCL allocator.
 *
 *  @param[in] s  Source string; may be NULL.
 *
 *  @return Newly allocated NUL-terminated copy, or NULL.
 *
 *  @retval NULL  s is NULL or allocation failed.
 */
char  *RcStrDup(PCSZ s);

/*! @brief Free a string allocated by RcStrDup.
 *
 *  @param[in] s  String to free; may be NULL.
 */
void   RcStrFree(char *s);

/*! @brief Emit a diagnostic to stderr.
 *
 *  @param[in] line  Source line number.
 *  @param[in] msg   Diagnostic text; may be NULL.
 */
void   RcDiagError(int line, PCSZ msg);

/*! @brief Format a style mask as a C expression.
 *
 *  Only constants whose STYLE_CTX_* bit is present in @p ulCtx are
 *  considered. Zero-valued aliases (SS_LEFT, ES_LEFT) are emitted
 *  explicitly when no style bit from that group is set. Any bits
 *  left unmatched are emitted as "0xNNNNNNNNL".
 *
 *  @param[in]  dwStyle  Style mask.
 *  @param[in]  ulCtx    STYLE_CTX_* mask (one or more bits).
 *  @param[out] pszBuf   Output buffer.
 *  @param[in]  ulSize   Size of @p pszBuf.
 */
void   RcStyleToExpr(DWORD dwStyle, ULONG ulCtx,
                     PSZ pszBuf, ULONG ulSize);

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
ULONG  RcStyleCtxOf(int iKind, PCSZ pszClass);

/* ==================================================================
 * Symbol table
 * ================================================================== */

/*! @brief Create and populate the symbol table.
 *
 *  @param[out] phSyms  Receiver for the handle.
 *
 *  @return APIRET.
 *
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  phSyms is NULL.
 *  @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET RcSymInit(HHASHTABLE *phSyms);

/*! @brief Add a symbol to the table.
 *
 *  @param[in] hSyms  Symbol table.
 *  @param[in] name   Symbol name.
 *  @param[in] v      Numeric value.
 *
 *  @return APIRET.
 *
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  hSyms or name is NULL, or name is
 *                                   too long.
 *  @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET RcSymAdd(HHASHTABLE hSyms, PCSZ name, long v);

/*! @brief Look up a symbol in the table.
 *
 *  @param[in]  hSyms  Symbol table.
 *  @param[in]  name   Symbol name.
 *  @param[out] pv     Receiver for the value.
 *
 *  @return APIRET.
 *
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  Any argument is NULL.
 *  @retval ERROR_FILE_NOT_FOUND     Symbol is not present.
 */
APIRET RcSymLookup(HHASHTABLE hSyms, PCSZ name, long *pv);

/* ==================================================================
 * Preprocessor
 * ================================================================== */

/*! @brief Preprocess an .rc file into a flat body.
 *
 *  @param[in]  path   Input .rc file.
 *  @param[in]  hSyms  Symbol table.
 *  @param[out] ppOut  Receiver for the NUL-terminated body.
 *  @param[out] pcb    Receiver for the body length.
 *
 *  @return APIRET.
 *
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_DATA       Include depth exceeded.
 *  @retval ERROR_OPEN_FAILED        File cannot be opened.
 *  @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET RcPreprocess(PCSZ path, HHASHTABLE hSyms, char **ppOut, ULONG *pcb);

/* ==================================================================
 * AST
 * ================================================================== */

/*! @brief Allocate a dialog.
 *
 *  @param[in] name   Symbolic name; may be NULL.
 *  @param[in] dlgEx  Non-zero for DIALOGEX.
 *  @param[in] items  Items vector; may be NULLHANDLE.
 *
 *  @return A new dialog, or NULL on failure.
 *
 *  @retval NULL  Allocation failed.
 */
RCDIALOG *RcDialogNew(PCSZ name, int dlgEx, HVECTOR items);

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
RCITEM   *RcItemNew(int kind, PCSZ text, long id, PCSZ cls,
                    DWORD style, int x, int y, int cx, int cy);

/*! @brief Free a dialog and its items.
 *
 *  @param[in] d  Dialog; may be NULL.
 */
void      RcDialogFree(RCDIALOG *d);

/*! @brief Create a vector of RCITEM* with a destroy callback.
 *
 *  @return New vector handle, or NULLHANDLE on failure.
 *
 *  @retval NULLHANDLE  Allocation failed.
 */
HVECTOR   RcItemsVectorNew(void);

/*! @brief Parse the preprocessed body into g_pResult.
 *
 *  @param[in] body  NUL-terminated preprocessed text.
 *
 *  @return 0 on success, -1 on failure.
 *
 *  @retval  0  Parse succeeded.
 *  @retval -1  body is NULL or the scanner buffer could not be
 *              created.
 */
int       RcParseBody(PCSZ body);

/* ==================================================================
 * Emitters
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
APIRET    RcEmitFunction(FILE *fp, RCDIALOG *d, PCSZ fn);

/*! @brief Emit a static byte array (not implemented).
 *
 *  @param[in] fp   Output stream.
 *  @param[in] d    Dialog.
 *  @param[in] arr  Array name.
 *
 *  @return APIRET.
 *
 *  @retval ERROR_NOT_SUPPORTED  Not implemented.
 */
APIRET    RcEmitArray(FILE *fp, RCDIALOG *d, PCSZ arr);

/* ==================================================================
 * Globals
 * ================================================================== */

/*! @brief Symbol table. */
extern HHASHTABLE g_hSyms;

/*! @brief Items vector for the dialog being parsed. */
extern HVECTOR    g_hItems;

/*! @brief Dialog being built. */
extern RCDIALOG  *g_pCurDlg;

/*! @brief Dialog produced by the last parse. */
extern RCDIALOG  *g_pResult;

#endif /* RC2C_AST_H */
