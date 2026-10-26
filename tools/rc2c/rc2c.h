typedef union {
    long   lNum;
    char  *pszStr;
    DWORD  dwStyle;
} YYSTYPE;
/*!
 *  @brief Token NUM.
 *  @def NUM
 */
#define	NUM	257
/*!
 *  @brief Token IDENT.
 *  @def IDENT
 */
#define	IDENT	258
/*!
 *  @brief Token STRING.
 *  @def STRING
 */
#define	STRING	259
/*!
 *  @brief Token DIALOG.
 *  @def DIALOG
 */
#define	DIALOG	260
/*!
 *  @brief Token DIALOGEX.
 *  @def DIALOGEX
 */
#define	DIALOGEX	261
/*!
 *  @brief Token CAPTION.
 *  @def CAPTION
 */
#define	CAPTION	262
/*!
 *  @brief Token STYLE.
 *  @def STYLE
 */
#define	STYLE	263
/*!
 *  @brief Token FONT.
 *  @def FONT
 */
#define	FONT	264
/*!
 *  @brief Token BEGIN_.
 *  @def BEGIN_
 */
#define	BEGIN_	265
/*!
 *  @brief Token END_.
 *  @def END_
 */
#define	END_	266
/*!
 *  @brief Token DEFPUSHBUTTON.
 *  @def DEFPUSHBUTTON
 */
#define	DEFPUSHBUTTON	267
/*!
 *  @brief Token PUSHBUTTON.
 *  @def PUSHBUTTON
 */
#define	PUSHBUTTON	268
/*!
 *  @brief Token LTEXT.
 *  @def LTEXT
 */
#define	LTEXT	269
/*!
 *  @brief Token CTEXT.
 *  @def CTEXT
 */
#define	CTEXT	270
/*!
 *  @brief Token RTEXT.
 *  @def RTEXT
 */
#define	RTEXT	271
/*!
 *  @brief Token ICON.
 *  @def ICON
 */
#define	ICON	272
/*!
 *  @brief Token EDITTEXT.
 *  @def EDITTEXT
 */
#define	EDITTEXT	273
/*!
 *  @brief Token CONTROL.
 *  @def CONTROL
 */
#define	CONTROL	274
/*!
 *  @brief Token GROUPBOX.
 *  @def GROUPBOX
 */
#define	GROUPBOX	275
/*!
 *  @brief Token CHECKBOX.
 *  @def CHECKBOX
 */
#define	CHECKBOX	276
/*!
 *  @brief Token RADIOBUTTON.
 *  @def RADIOBUTTON
 */
#define	RADIOBUTTON	277
/*!
 *  @brief Token COMMA.
 *  @def COMMA
 */
#define	COMMA	278
/*!
 *  @brief Token PIPE.
 *  @def PIPE
 */
#define	PIPE	279
/*!
 *  @brief Token PLUS.
 *  @def PLUS
 */
#define	PLUS	280


extern YYSTYPE yylval;
