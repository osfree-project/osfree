/*!
 *  @file strtof.c
 *  @brief Portable implementation of strtof().
 *
 *  Provides a simple fallback for systems whose C library does not
 *  ship strtof(). Supports an optional sign, an integer part, a
 *  decimal fraction and a base-10 exponent.
 */

#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/*!
 *  @brief Converts a string to a float.
 *
 *  Parses an optional sign, an integer part, a fractional part and an
 *  optional decimal exponent of the form `e[+-]N`.
 *
 *  @param[in]  s      NUL-terminated input string.
 *  @param[out] endptr Receives the address of the first character after
 *                     the parsed number, or the original @p s when no
 *                     number was parsed.
 *
 *  @return The parsed value.
 *  @retval 0 Empty input, no number found, or invalid exponent.
 */
float strtof(const char* s, char** endptr) {
    register const char*  p     = s;
    register float        value = 0.;
    int                   sign  = +1;
    float                 factor;
    unsigned int          expo;

    while ( isspace(*p) )
        p++;

    switch (*p) {
    case '-': sign = -1;
    case '+': p++;
    default : break;
    }

    while ( (unsigned int)(*p - '0') < 10u )
        value = value*10 + (*p++ - '0');

    if ( *p == '.' ) {
        factor = 1.;

        p++;
        while ( (unsigned int)(*p - '0') < 10u ) {
            factor *= 0.1;
            value  += (*p++ - '0') * factor;
        }
    }

    if ( (*p | 32) == 'e' ) {
        expo   = 0;
        factor = 10.L;

        switch (*++p) {                 // ja hier wei ich nicht, was mindestens nach einem 'E' folgenden MUSS.
        case '-': factor = 0.1;
        case '+': p++;
                  break;
        case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
                  break;
        default : value = 0.L;
                  p     = s;
                  goto done;
        }

        while ( (unsigned int)(*p - '0') < 10u )
            expo = 10 * expo + (*p++ - '0');

        while ( 1 ) {
            if ( expo & 1 )
                value *= factor;
            if ( (expo >>= 1) == 0 )
                break;
            factor *= factor;
        }
    }

done:
    if ( endptr != NULL )
        *endptr = (char*)p;

    return value * sign;
}
