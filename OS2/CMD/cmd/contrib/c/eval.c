// EVAL.C - Expression analyzer for 4xxx / TCMD batch language
//  Copyright (c) 1991 - 1997 Rex C. Conn  All rights reserved
//  This routine supports the standard arithmetic functions: +, -, *, /,
//    %, and the unary operators + and -
//  EVAL supports BCD numbers with 16 integer and 8 decimal places

#include "product.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "4all.h"


#define SIZE_INTEGER 16
#define SIZE_DECIMAL 8
#define SIZE_MANTISSA 24

#define ADD 0
#define SUBTRACT 1
#define MULTIPLY 2
#define EXPONENTIATE 3
#define DIVIDE 4
#define INTEGER_DIVIDE 5
#define MODULO 6
#define OPEN_PAREN 7
#define CLOSE_PAREN 8

#define MAX_DIGITS 25


// BCD structure
typedef struct {
    char sign;
    char integer[SIZE_INTEGER];
    char decimal[SIZE_DECIMAL+2];
} BCD;

static void level1(BCD *);
static void level2(BCD *);
static void level3(BCD *);
static void level4(BCD *);

extern int _System add_bcd(BCD *, BCD *);
extern int _System multiply_bcd(BCD *, BCD *);
extern int _System divide_bcd(BCD *, BCD *, int);

static char operators[] = "+-**/\\%()"; // valid EVAL operators
static char *line;          // pointer to expression
static char token[MAX_DIGITS+2];    // current token (number or operator)

static char tok_type;           // token type (number or operator)
static char delim_type;         // delimiter type

static jmp_buf env;


#define DELIMITER   1
#define NUMBER      2


static char is_operator(void);
static void get_token(void);

// check for a operator character:  + - * / % ( )
static char is_operator(void)
{
    int i;

    delim_type = 0;
    for ( i = 0; ( operators[i] != '\0' ); i++ ) {

        if ( *line == operators[i] ) {
            delim_type = (char)i;
            if (( delim_type == MULTIPLY ) && ( line[1] == '*' )) {
                delim_type = EXPONENTIATE;
                line++;
            }
            return ( *line );
        }
    }

    return 0;
}


static void get_token( void )
{
    int i = 0;
    int decimal_flag = 0;

    tok_type = 0;
    while (( *line == ' ' ) || ( *line == '\t' ))
        line++;

    // get the next token (number or operator)
    for ( ; (( *line != '\0' ) && ( i < MAX_DIGITS )); line++ ) {

        if ( isdigit( *line )) {
            tok_type = NUMBER;
            token[i++] = *line;

        } else if ( *line == gaCountryInfo.szDecimal[0] ) {

            // only one '.' allowed!
            if ( decimal_flag++ != 0 ) {
                tok_type = 0;
                return;
            }
            token[i++] = *line;

        } else if (( *line != '.' ) && ( *line != ',' ) && ( *line != gaCountryInfo.szThousandsSeparator[0] ))
            break;
    }

    if (( tok_type == 0 ) && (( token[i++] = is_operator()) != '\0' )) {
        tok_type = DELIMITER;
        line++;
    }

    token[i] = '\0';
}


static void arith(int op, BCD *r, BCD *h)
{
    int rval = 0;
    BCD temp;

    if ( op < MULTIPLY ) {

        // reverse the second argument's sign if subtracting
        if ( op == SUBTRACT )
            h->sign = (char)(( h->sign == '-' ) ? '+' : '-' );

add_values:
        // if first op - & second +, swap & subtract
        if ((r->sign != h->sign) && (r->sign == '-')) {
            memmove(&temp,r,sizeof(BCD));
            memmove(r,h,sizeof(BCD));
            memmove(h,&temp,sizeof(BCD));
        }

        rval = add_bcd(r,h);

    } else if (op == MULTIPLY)
        rval = multiply_bcd(r,h);

    else if (op == DIVIDE)
        rval = divide_bcd(r,h,0);

    else if (op == EXPONENTIATE) {

        // convert number from BCD format back to ASCII format
        for (op = 0; (op < SIZE_INTEGER); op++)
            h->integer[op] += '0';
        h->integer[op] = '\0';
        op = atoi(&(h->sign));

        if (op < 0) {
            // we don't allow negative exponents!
            longjmp(env,ERROR_4DOS_BAD_SYNTAX);
        } else if (op == 0) {
            // zero == 1!
            for (op = 0; (op < SIZE_MANTISSA); op++)
                r->integer[op] = '\0';
            r->decimal[-1] = 1;
        } else {
            memmove( &temp, r, sizeof(BCD) );
            while ((--op > 0) && (rval == 0))
                rval = multiply_bcd( r, &temp );
        }

    } else if (op == INTEGER_DIVIDE) {

        rval = divide_bcd(r,h,0);
        // remove the decimal portion
        memset(r->decimal,'\0',SIZE_DECIMAL);

    } else if (op == MODULO) {

        // remove the decimal portion
        memset( r->decimal, '\0', SIZE_DECIMAL );
        memset( h->decimal, '\0', SIZE_DECIMAL );

        // save the divisor for kludge below
        memmove( &temp, h, sizeof(BCD) );

        rval = divide_bcd( r, h, 1 );

        // remainder was saved in "h"
        memmove( r, h, sizeof(BCD) );

        // kludge to make the mathematicians happy!  if the signs
        //   don't match, modulo = remainder + divisor
        if ( r->sign != temp.sign ) {
            memmove( h, &temp, sizeof(BCD) );
            goto add_values;
        }
    }

    if ( rval == -1 )
        longjmp( env, ERROR_4DOS_OVERFLOW );
}


// do addition & subtraction
static void level1(BCD *result)
{
    int op;
    BCD hold;

    level2(result);

    while ((tok_type == DELIMITER) && ((delim_type == ADD) || (delim_type == SUBTRACT))) {

        op = delim_type;

        // initialize the BCD structure
        memset(&hold,'\0',SIZE_MANTISSA+2);
        get_token();

        level2(&hold);
        arith(op,result,&hold);
    }
}


// do multiplication & division & modulo
static void level2(BCD *result)
{
    int op;
    BCD hold;

    level3(result);

    while ((tok_type == DELIMITER) && ((delim_type == MULTIPLY) || (delim_type == EXPONENTIATE) || (delim_type == DIVIDE) || (delim_type == INTEGER_DIVIDE) || (delim_type == MODULO))) {

        op = delim_type;

        // initialize the BCD structure
        memset(&hold,'\0',SIZE_MANTISSA+2);
        get_token();

        level3(&hold);
        arith(op,result,&hold);
    }
}


// process unary + & -
static void level3( BCD *result )
{
    int is_unary = -1;

    if (( tok_type == DELIMITER ) && (( delim_type == ADD ) || ( delim_type == SUBTRACT ))) {
        is_unary = delim_type;
        get_token();
    }

    level4(result);

    if ( is_unary == ADD )
        result->sign = '+';
    else if ( is_unary == SUBTRACT )
        result->sign = (char)((result->sign == '-') ? '+' : '-');
}


// process parentheses
static void level4( BCD *result )
{
    int i;
    char *ptr;

    // is it a parenthesis?
    if (( tok_type == DELIMITER ) && ( delim_type == OPEN_PAREN )) {

        get_token();
        level1(result);
        if ( delim_type != CLOSE_PAREN )
            longjmp( env, ERROR_4DOS_UNBALANCED_PARENS );

        get_token();

    } else if ( tok_type == NUMBER ) {

        // initialize the BCD structure
        memset( result, '\0', SIZE_MANTISSA+2 );

        result->sign = '+';

        if (( ptr = strchr( token, gaCountryInfo.szDecimal[0] )) != NULL )
            strcpy( ptr, ptr + 1 );
        else
            ptr = strend( token );

        // stuff the token into the BCD structure
        if (( i = (int)( SIZE_INTEGER - ( ptr - token ))) < 0 )
            longjmp(env,ERROR_4DOS_OVERFLOW);

        for ( ptr = token; (( *ptr != '\0' ) && ( i < SIZE_MANTISSA )); ptr++, i++ )
            result->integer[i] = (char)( *ptr - '0' );

        get_token();
    }

    if (( tok_type != DELIMITER ) && ( tok_type != NUMBER ) && ( *line ))
        longjmp( env, ERROR_4DOS_BAD_SYNTAX );
}


// set the default EVAL precision
void SetEvalPrecision( char *ptr, unsigned int *uMin, unsigned int *uMax )
{
    if ( isdigit( *ptr ))
        sscanf( ptr, "%u%*c%u", uMin, uMax );
    else if (( *ptr == '.' ) || ( *ptr == ',' ))
        *uMax = atoi( ptr+1 );

    if ( *uMax > 8 )
        *uMax = 8;
    if ( *uMin > 8 )
        *uMin = 8;
}


// evaluate the algebraic expression
int evaluate( char *expr )
{
    int n;
    unsigned int uMax, uMin;
    int rval = 0;
    char *ptr;
    BCD x;

    uMin = gpIniptr->EvalMin;
    uMax = gpIniptr->EvalMax;

    // %@eval[...=n] sets the default minimum decimal precision
    // %@eval[...=.n] sets the default maximum decimal precision
    if (( ptr = strchr( expr, '=' )) != NULL ) {

        *ptr++ = '\0';
        SetEvalPrecision( skipspace( ptr ), &uMin, &uMax );
        if ( *expr == '\0' )
            return 0;
    }

    if (( n = setjmp( env )) >= OFFSET_4DOS_MSG )
        rval = error( n, expr );

    else {

        memset( &x, '\0', SIZE_MANTISSA+2 );
        line = expr;

        get_token();
        if ( token[0] == '\0' )
            longjmp( env, ERROR_4DOS_NO_EXPRESSION );

        level1( &x );

        // round up
        for ( n = SIZE_MANTISSA; ( n > 0 ); n-- ) {

            if ( n >= (int)( SIZE_INTEGER + uMax )) {
                if (x.integer[n] > 5)
                    x.integer[n-1] += 1;
            } else if ( x.integer[n] > 9 ) {
                x.integer[n] = 0;
                x.integer[n-1] += 1;
            }
        }

        // convert number from BCD format back to ASCII format
        for ( n = 0; ( n < SIZE_MANTISSA ); n++ )
            x.integer[n] += '0';
        x.integer[n] = '\0';

        // truncate to maximum precision
        x.decimal[uMax] = '\0';

        // strip trailing 0's
        for ( n = uMax - 1; ( n >= (int)uMin ); n-- ) {
            if ( x.decimal[n] == '0' )
                x.decimal[n] = '\0';
            else
                break;
        }

        if ( x.decimal[0] )
            strins( x.decimal, gaCountryInfo.szDecimal );

        // check for a leading '-' and strip leading 0's
        sscanf( x.integer, "%*15[0]%s", expr );

        if (( x.sign == '-' ) && ( _stricmp( expr, "0" ) != 0 ))
            strins( expr, "-" );
    }

    return rval;
}

