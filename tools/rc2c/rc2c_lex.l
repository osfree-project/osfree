%{
/*! @file rc2c_lex.l
 *  @brief wlex scanner for the RC dialog subset. */

#include <stdlib.h>
#include <string.h>
#include "rc2c_ast.h"
#include "rc2c_parse.h"

/*! @brief Current source line. */
int yy_line = 1;

/*! @brief Case-insensitive ASCII compare of two NUL-terminated strings.
 *
 *  @param[in] a  First string.
 *  @param[in] b  Second string.
 *
 *  @return 1 if equal, 0 otherwise.
 */
static int rc2c_ci_eq(const char *a, const char *b)
{
    int ca, cb;
    if (!a || !b) return 0;
    while (*a && *b) {
        ca = (unsigned char)*a; cb = (unsigned char)*b;
        if (ca >= 'A' && ca <= 'Z') ca += 32;
        if (cb >= 'A' && cb <= 'Z') cb += 32;
        if (ca != cb) return 0;
        a++; b++;
    }
    return *a == 0 && *b == 0;
}

/*! @brief Unescape the body of a "..." token.
 *
 *  @param[in] s    Token body without surrounding quotes.
 *  @param[in] len  Length of s.
 *
 *  @return A new NUL-terminated string, or NULL on allocation
 *          failure.
 *
 *  @retval NULL  Allocation failed.
 */
static char *unquote(const char *s, int len)
{
    char *b = (char *)CclMalloc((size_t)len + 1);
    int i, j;
    if (!b) return NULL;
    for (i = 0, j = 0; i < len; i++) {
        char c = s[i];
        if (c == '\\' && i + 1 < len) {
            i++;
            switch (s[i]) {
            case 'n': c = '\n'; break;
            case 'r': c = '\r'; break;
            case 't': c = '\t'; break;
            case '\\': c = '\\'; break;
            case '"':  c = '"';  break;
            default:   c = s[i]; break;
            }
        }
        b[j++] = c;
    }
    b[j] = 0;
    return b;
}
%}

%option noyywrap

%%

[ \t\r]+                 ;
\n                       { yy_line++; }
"//"[^\n]*               ;
"/*"([^*]|\*+[^*/])*\*+"/" {
    int i;
    for (i = 0; i < yyleng; i++)
        if (yytext[i] == '\n') yy_line++;
}
[0-9]+                   { yylval.lNum = atol(yytext); return NUM; }
0[xX][0-9A-Fa-f]+        { yylval.lNum = strtol(yytext, NULL, 16); return NUM; }
-[0-9]+                  { yylval.lNum = atol(yytext); return NUM; }
\"([^"\\]|\\.)*\"        { yylval.pszStr = unquote(yytext + 1, yyleng - 2); return STRING; }
[A-Za-z_][A-Za-z0-9_]*   {
    if (rc2c_ci_eq(yytext, "DIALOG"))       return DIALOG;
    if (rc2c_ci_eq(yytext, "DIALOGEX"))     return DIALOGEX;
    if (rc2c_ci_eq(yytext, "CAPTION"))      return CAPTION;
    if (rc2c_ci_eq(yytext, "STYLE"))        return STYLE;
    if (rc2c_ci_eq(yytext, "FONT"))         return FONT;
    if (rc2c_ci_eq(yytext, "BEGIN"))        return BEGIN_;
    if (rc2c_ci_eq(yytext, "END"))          return END_;
    if (rc2c_ci_eq(yytext, "DEFPUSHBUTTON"))return DEFPUSHBUTTON;
    if (rc2c_ci_eq(yytext, "PUSHBUTTON"))   return PUSHBUTTON;
    if (rc2c_ci_eq(yytext, "LTEXT"))        return LTEXT;
    if (rc2c_ci_eq(yytext, "CTEXT"))        return CTEXT;
    if (rc2c_ci_eq(yytext, "RTEXT"))        return RTEXT;
    if (rc2c_ci_eq(yytext, "ICON"))         return ICON;
    if (rc2c_ci_eq(yytext, "EDITTEXT"))     return EDITTEXT;
    if (rc2c_ci_eq(yytext, "CONTROL"))      return CONTROL;
    if (rc2c_ci_eq(yytext, "GROUPBOX"))     return GROUPBOX;
    if (rc2c_ci_eq(yytext, "CHECKBOX"))     return CHECKBOX;
    if (rc2c_ci_eq(yytext, "RADIOBUTTON"))  return RADIOBUTTON;
    yylval.pszStr = RcStrDup(yytext);
    return IDENT;
}
","                      { return COMMA; }
"|"                      { return PIPE; }
"+"                      { return PLUS; }
.                        { RcDiagError(yy_line, "unexpected char"); }

%%
