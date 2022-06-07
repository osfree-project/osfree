
/*
 *@@sourcefile regexp.c:
 *      extended regular expressions, taken from "Andy's Extended
 *      Regular Expressions", as written by Andy Key
 *      (nyangau@interalpha.co.uk) and released into the public
 *      domain, plus adjustments for the XWP helpers.
 *
 *      Usage: All C programs; not OS/2-specific.
 *
 *      Function prefixes:
 *      --  rxp*       regular expression functions.
 *
 *      Regular expression matching is done in the following stages:
 *
 *      1)  Call rxpCompile to parse the expression into a recursive
 *          tree of matches.
 *
 *          This tree is converted into a finite state machine (FSM).
 *          A second FSM is built from the first but with epsilon
 *          moves removed to elimate lockups and increase speed.
 *
 *          The input string can be used to drive the FSM through all
 *          the possible routes. The largest (or smallest) amount of
 *          input string required to reach the finish state is recorded
 *          since an extended regular expression is deemed to match as
 *          much input string as possible.
 *
 *      2)  Call one of rxpMatch, rxpMatch_fwd, or rxpMatch_bwd to
 *          perform a match.
 *
 *      3)  Call rxpFree to free the compiled ERE.
 *
 *      Beware: The matching routine is highly recursive and can
 *      require around 20 to 30 bytes per character in the source string
 *      to match it. Thus you should try to limit the length of the source
 *      string and/or allow a stack size of at least 20 (to 30) * max string
 *      length. In addition, add 2KB to allow for the use of several nested
 *      sub-expressions in the matching.
 *
 *@@header "helpers\regexp.h"
 *@@added V0.9.19 (2002-04-17) [umoeller]
 */

/*
 *      Copyright (C) 2002 Ulrich M”ller.
 *      This file is part of the "XWorkplace helpers" source package.
 *      This is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published
 *      by the Free Software Foundation, in version 2 as it comes in the
 *      "COPYING" file of the XWorkplace main distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 */

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <memory.h>

#include "setup.h"                      // code generation and debugging options

#define ERE_C
#include "helpers\regexp.h"

#define isword(c) (isalnum(c)||(c)=='_')

STATIC int val_of_hex(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    return 0;                   // Shouldn't get here
}

STATIC int escaped(const char *s)
{
    if (s[0] == 'x' && isxdigit(s[1]))
        // \x followed by 1 or 2 hex digits
    {
        int n = val_of_hex(s[1]);

        if (isxdigit(s[2]))
            n = n * 16 + val_of_hex(s[2]);
        return n;
    }
    else
        switch (s[0])
        {
            case 'n':
                return '\n';
            case 't':
                return '\t';
            case 'r':
                return '\r';
            case 'b':
                return '\b';
            case 'f':
                return '\f';
            case 'e':
                return 0x1b;
            default:
                return (unsigned char)s[0];
        }
}

STATIC const char *past_escaped(const char *s)
{
    if (s[0] == 'x' && isxdigit(s[1]))
        return isxdigit(s[2]) ? s + 3 : s + 2;
    else
        return s + 1;
}

#define zero_cclass(cclass) memset(cclass, 0, 0x100 >> 3)

STATIC unsigned char bits[] =
{0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};

#define add_to_cclass(n, cclass) (cclass[(unsigned char)(n)>>3] |= bits[(unsigned char)(n) & 7])

#define remove_from_cclass(n, cclass) (cclass[(unsigned char)(n)>>3] &= ~bits[(unsigned char)(n) & 7])

STATIC void invert_cclass(unsigned char *cclass)
{
    int i;

    for (i = 0; i < (0x100 >> 3); i++)
        cclass[i] ^= 0xff;
}

#define match_cclass(n, cclass) ((cclass[(unsigned char)(n)>>3] & bits[(unsigned char)(n) & 7]) != 0)

#define CHL_EOS                 0
#define CHL_END_CCLASS        (-1)
#define CHL_COMP              (-2)
#define CHL_RANGE             (-3)
#define CHL_POSIX_COLLATING   (-4)
#define CHL_POSIX_EQUIVALENCE (-5)
#define CHL_POSIX_CCLASS_BAD  (-6)
#define CHL_POSIX_CCLASS_BASE (-30)
#define CHL_POSIX_CCLASS_END  (-30+20)

/* I use my own wrapper functions so I can take their addresses.
 * Remember, isalnum etc. can be implemented as macros... */
STATIC BOOLEAN my_isalnum(int ch)
{
    return isalnum(ch);
}
STATIC BOOLEAN my_isalpha(int ch)
{
    return isalpha(ch);
}
STATIC BOOLEAN my_isblank(int ch)
{
    return ch == ' ' || ch == '\t';
}
STATIC BOOLEAN my_iscntrl(int ch)
{
    return iscntrl(ch);
}
STATIC BOOLEAN my_isdigit(int ch)
{
    return isdigit(ch);
}
STATIC BOOLEAN my_islower(int ch)
{
    return islower(ch);
}
STATIC BOOLEAN my_isprint(int ch)
{
    return isprint(ch);
}
STATIC BOOLEAN my_ispunct(int ch)
{
    return ispunct(ch);
}
STATIC BOOLEAN my_isspace(int ch)
{
    return isspace(ch);
}
STATIC BOOLEAN my_isupper(int ch)
{
    return isupper(ch);
}
STATIC BOOLEAN my_isxdigit(int ch)
{
    return isxdigit(ch);
}

typedef struct
{
    int len_name;
    const char *name;
     BOOLEAN(*iscclass) (int ch);
}
POSIX_CCLASS;

STATIC POSIX_CCLASS posix_cclass[] =
{
    5, "alnum", my_isalnum,
    5, "alpha", my_isalpha,
    5, "blank", my_isblank,
    5, "cntrl", my_iscntrl,
    5, "digit", my_isdigit,
    5, "lower", my_islower,
    5, "print", my_isprint,
    5, "punct", my_ispunct,
    5, "space", my_isspace,
    5, "upper", my_isupper,
    6, "xdigit", my_isxdigit,
};

STATIC int find_posix(const char *str)
{
    int p;

    if (str[0] != '[')
        return str[0];
    if (str[1] == '.')
        return CHL_POSIX_COLLATING;
    if (str[1] == '=')
        return CHL_POSIX_EQUIVALENCE;
    if (str[1] != ':')
        return str[0];
    for (p = 0; p < sizeof(posix_cclass) / sizeof(posix_cclass[0]); p++)
        if (!memcmp(str + 2, posix_cclass[p].name, posix_cclass[p].len_name) &&
            str[2 + posix_cclass[p].len_name] == ':' &&
            str[2 + posix_cclass[p].len_name + 1] == ']')
            return CHL_POSIX_CCLASS_BASE + p;
    return CHL_POSIX_CCLASS_BAD;
}

STATIC int cclass_thisch(const char *str)
{
    switch (str[0])
    {
        case '\\':
            return escaped(str + 1);
        case ']':
            return CHL_END_CCLASS;
        case '^':
            return CHL_COMP;
        case '-':
            return CHL_RANGE;
        case '\0':
            return CHL_EOS;
        default:
            return find_posix(str);
    }
}

STATIC const char *cclass_nextch(const char *str)
{
    int p;

    if (str[0] == '\\')
        return past_escaped(str + 1);
    p = find_posix(str);
    if (CHL_POSIX_CCLASS_BASE <= p && p < CHL_POSIX_CCLASS_END)
        return str + 2 + posix_cclass[p - CHL_POSIX_CCLASS_BASE].len_name + 2;
    else
        return str + 1;
}

STATIC unsigned char *compile_cclass(const char *str,
                                     const char **str_after,
                                     int erecf,
                                     int *rc)
{
    unsigned char *cclass;
    BOOLEAN complement;
    int i, c, last_c = -1;

    if ((cclass = (unsigned char *)malloc(0x100 >> 3)) == NULL)
    {
        *rc = ERROR_NOT_ENOUGH_MEMORY;
        return NULL;
    }
    zero_cclass(cclass);

    complement = (cclass_thisch(str) == CHL_COMP);
    if (complement)
        str = cclass_nextch(str);

    while ((c = cclass_thisch(str)) != CHL_EOS && c != CHL_END_CCLASS)
    {
        if (CHL_POSIX_CCLASS_BASE <= c && c < CHL_POSIX_CCLASS_END)
        {
            for (i = 0; i < 0x100; i++)
                if (posix_cclass[c - CHL_POSIX_CCLASS_BASE].iscclass(i))
                    add_to_cclass(i, cclass);
            last_c = -1;
        }
        else
            switch (c)
            {
/*...sCHL_POSIX_\42\ \45\ error cases:32: */
                case CHL_POSIX_COLLATING:
                    free(cclass);
                    *rc = EREE_POSIX_COLLATING;
                    return NULL;
                case CHL_POSIX_EQUIVALENCE:
                    free(cclass);
                    *rc = EREE_POSIX_EQUIVALENCE;
                    return NULL;
                case CHL_POSIX_CCLASS_BAD:
                    free(cclass);
                    *rc = EREE_POSIX_CCLASS_BAD;
                    return NULL;
/*...sCHL_RANGE   \45\ range:32: */
                case CHL_RANGE:
                    if (last_c == -1)
                        // Unexpected at this point
                    {
                        free(cclass);
                        *rc = EREE_UNEX_RANGE;
                        return NULL;
                    }
                    str = cclass_nextch(str);
                    if ((c = cclass_thisch(str)) == CHL_EOS || c == CHL_END_CCLASS)
                        // Not followed by anything
                    {
                        free(cclass);
                        *rc = EREE_UNF_RANGE;
                        return NULL;
                    }
                    for (i = last_c + 1; i <= c; i++)
                        add_to_cclass(i, cclass);
                    last_c = c;
                    break;

#pragma info(nogen)     // do not warn here

/*...sCHL_COMP    \45\ complement:32: */
                case CHL_COMP:
                    c = '^';
                    // Fall through
/*...sdefault     \45\ individual entry:32: */
                default:
                    if (erecf & ERECF_TOLOWER)
                        c = tolower(c);
                    add_to_cclass(c, cclass);
                    last_c = c;
                    break;

#pragma info(restore)

            }
        str = cclass_nextch(str);
    }

    if (c == CHL_EOS)
    {
        free(cclass);
        *rc = EREE_UNF_CCLASS;
        return NULL;
    }

    if (complement)
        invert_cclass(cclass);

    remove_from_cclass(0, cclass);

    *str_after = cclass_nextch(str);
    return cclass;
}

#define delete_cclass(cclass) free(cclass)

/* A 'match' embodies all of regular expression functionality.
 * Matches can be defined in terms of each other. */

typedef unsigned char MTYPE;

#define MTYPE_NULL  ((MTYPE)  0)
#define MTYPE_CHAR  ((MTYPE)  1)
#define MTYPE_NCHAR ((MTYPE)  2)
#define MTYPE_STRING    ((MTYPE)  3)
#define MTYPE_CCLASS    ((MTYPE)  4)
#define MTYPE_WORD  ((MTYPE)  5)
#define MTYPE_NWORD ((MTYPE)  6)
#define MTYPE_DOT   ((MTYPE)  7)
#define MTYPE_QUERY ((MTYPE)  8)
#define MTYPE_PLUS  ((MTYPE)  9)
#define MTYPE_STAR  ((MTYPE) 10)
#define MTYPE_CREP  ((MTYPE) 11)
#define MTYPE_OR    ((MTYPE) 12)
#define MTYPE_CAT   ((MTYPE) 13)
#define MTYPE_SUB   ((MTYPE) 14)
#define MTYPE_SOL       ((MTYPE) 15)
#define MTYPE_EOL       ((MTYPE) 16)
#define MTYPE_SOW   ((MTYPE) 17)
#define MTYPE_EOW   ((MTYPE) 18)
#define MTYPE_IW    ((MTYPE) 19)
#define MTYPE_EW    ((MTYPE) 20)
#define MTYPE_BACK  ((MTYPE) 21)

typedef struct match_struct MATCH;

struct match_struct
{
    MTYPE mtype;
    union
    {
        char character;
        unsigned char *cclass;
        MATCH *match;
        MATCH *matchs[2];
        int n_span;
        char *string;
        struct
        {
            unsigned m, n;
            MATCH *match;
        }
        crep;
    }
    u;
};

STATIC MATCH null_match =
{MTYPE_NULL};

#define NULL_MATCH (&null_match)

STATIC void delete_match(MATCH * match)
{
    if (match == NULL_MATCH)
        return;
    switch (match->mtype)
    {
        case MTYPE_STRING:
            free(match->u.string);
            break;
        case MTYPE_CCLASS:
            delete_cclass(match->u.cclass);
            break;
        case MTYPE_QUERY:
        case MTYPE_PLUS:
        case MTYPE_STAR:
            delete_match(match->u.match);
            break;
        case MTYPE_CREP:
            delete_match(match->u.crep.match);
            break;
        case MTYPE_OR:
        case MTYPE_CAT:
            delete_match(match->u.matchs[0]);
            delete_match(match->u.matchs[1]);
            break;
        case MTYPE_SUB:
            delete_match(match->u.match);
            break;
    }
    free(match);
}

/*
 *@@ shortest_match:
 *      determines the shortest possible match length for a given match
 *      tree. In the following example it is 3, thus allowing us to only
 *      consider 4 positions for regular expression matching in a 6
 *      character line.
 *
 +          aa*bc
 +
 +          123456
 +          ---
 +          ---
 +          ---
 +          ---
 */

STATIC unsigned shortest_match(const MATCH * match)
{
    unsigned a, b;

    switch (match->mtype)
    {
        case MTYPE_NULL:
            return 0;
        case MTYPE_CHAR:
            return 1;
        case MTYPE_NCHAR:
            return 1;
        case MTYPE_STRING:
            return (unsigned char)match->u.string[0];
        case MTYPE_CCLASS:
            return 1;
        case MTYPE_WORD:
            return 1;
        case MTYPE_NWORD:
            return 1;
        case MTYPE_DOT:
            return 1;
        case MTYPE_QUERY:
            return 0;
        case MTYPE_PLUS:
            return shortest_match(match->u.match);
        case MTYPE_STAR:
            return 0;
        case MTYPE_CREP:
            return match->u.crep.m *
                shortest_match(match->u.crep.match);
        case MTYPE_OR:
            a = shortest_match(match->u.matchs[0]);
            b = shortest_match(match->u.matchs[1]);
            return a <= b ? a : b;
        case MTYPE_CAT:
            return shortest_match(match->u.matchs[0]) +
                shortest_match(match->u.matchs[1]);
        case MTYPE_SUB:
            return shortest_match(match->u.match);
        case MTYPE_SOL:
            return 0;
        case MTYPE_EOL:
            return 0;
        case MTYPE_SOW:
            return 0;
        case MTYPE_EOW:
            return 0;
        case MTYPE_IW:
            return 0;
        case MTYPE_EW:
            return 0;
        case MTYPE_BACK:
            return 0;
    }
    return 0;                   // Should never happen
}

STATIC BOOLEAN got_backrefs(MATCH * match)
{
    switch (match->mtype)
    {
        case MTYPE_QUERY:
        case MTYPE_PLUS:
        case MTYPE_STAR:
            return got_backrefs(match->u.match);
        case MTYPE_CREP:
            return got_backrefs(match->u.crep.match);
        case MTYPE_OR:
        case MTYPE_CAT:
            return got_backrefs(match->u.matchs[0]) ||
                got_backrefs(match->u.matchs[1]);
        case MTYPE_BACK:
            return TRUE;
    }
    return FALSE;
}

STATIC MATCH *remove_subs(MATCH * match)
{
    switch (match->mtype)
    {
        case MTYPE_QUERY:
        case MTYPE_PLUS:
        case MTYPE_STAR:
            match->u.match = remove_subs(match->u.match);
            break;
        case MTYPE_CREP:
            match->u.crep.match = remove_subs(match->u.crep.match);
            break;
        case MTYPE_OR:
        case MTYPE_CAT:
            match->u.matchs[0] = remove_subs(match->u.matchs[0]);
            match->u.matchs[1] = remove_subs(match->u.matchs[1]);
            break;
        case MTYPE_SUB:
            {
                MATCH *m = match;

                match = match->u.match;
                free(m);
            }
            break;
    }
    return match;
}

STATIC int count_sub(const MATCH * match)
{
    switch (match->mtype)
    {
        case MTYPE_SUB:
            return 1 + count_sub(match->u.match);
        case MTYPE_QUERY:
        case MTYPE_PLUS:
        case MTYPE_STAR:
            return count_sub(match->u.match);
        case MTYPE_CREP:
            return count_sub(match->u.crep.match);
        case MTYPE_OR:
        case MTYPE_CAT:
            return count_sub(match->u.matchs[0]) +
                count_sub(match->u.matchs[1]);
    }
    return 0;
}

/*...scompiling matches:0: */
/*
 *
 * <term>    ::= <character> | <class> | ( <match> )
 * <factor>  ::= <term>? | <term>+ | <term>* | <term>
 * <factors> ::= <factor> { <factor> }
 * <match>   ::= <factors> { |<factors> }
 *
 */

/* We have an optimisation here in that if we can see a span of 'boring'
 * characters, we return them as a single entity. We mustn't do this for the
 * last in the span, as this may have modifiers applied to it, as in the
 * example string "abcd*", where the d has the * modifier, but abc do not.
 * We only do 255 chars at once, as the length is stored in a byte.
 * Reducing 255 levels of per-character recursive searching to 1 level with
 * a string compare has got to be a massive saving. */

#define STRING_MAX 255

#define CH_NOT_BASE 1000
#define CH_NOT_END  (1000+0x100)
#define CH_EOS      0
#define CH_DOT      (-1)
#define CH_WORD     (-2)
#define CH_NWORD    (-3)
#define CH_LSQR     (-4)
#define CH_RSQR     (-5)
#define CH_LPAR     (-6)
#define CH_RPAR     (-7)
#define CH_LCUR     (-8)
#define CH_RCUR     (-9)
#define CH_QUERY    (-10)
#define CH_PLUS     (-11)
#define CH_STAR     (-12)
#define CH_OR       (-13)
#define CH_SOL      (-14)
#define CH_EOL      (-15)
#define CH_SOW      (-16)
#define CH_EOW      (-17)
#define CH_IW       (-18)
#define CH_EW       (-19)
#define CH_BAD_TILDE    (-20)
#define CH_STRING_BASE  (-1000)
#define CH_STRING_END   (-1000+STRING_MAX)
#define CH_BACK_BASE    (-2000)
#define CH_BACK_END     (-2000+9)

STATIC int boring_string(const char *s)
{
    int n = 0;

    while (*s &&
           strchr("\\.[](){}?+*|^$<>~", *s) == NULL
           && n < STRING_MAX)
    {
        s++;
        n++;
    }
    return n;
}

STATIC int thisch(const char *str)
{
    int n;

    switch (str[0])
    {
        case '\\':
            if (str[1] >= '1' && str[1] <= '9')
                return CH_BACK_BASE + (str[1] - '1');
            switch (str[1])
            {
                case '`':
                    return CH_SOL;
                case '\'':
                    return CH_EOL;
                case '<':
                    return CH_SOW;
                case '>':
                    return CH_EOW;
                case 'w':
                    return CH_WORD;
                case 'W':
                    return CH_NWORD;
                case 'B':
                    return CH_IW;
                case 'y':
                    return CH_EW;
                default:
                    return escaped(str + 1);
            }

        case '~':
            if (str[1] == '\\')
                return CH_NOT_BASE + escaped(str + 2);
            else if (str[1] != '\0')
                return CH_NOT_BASE + (unsigned char)str[1];
            else
                return '~';
        case '.':
            return CH_DOT;
        case '[':
            return CH_LSQR;
        case ']':
            return CH_RSQR;
        case '(':
            return CH_LPAR;
        case ')':
            return CH_RPAR;
        case '{':
            return CH_LCUR;
        case '}':
            return CH_RCUR;
        case '?':
            return CH_QUERY;
        case '+':
            return CH_PLUS;
        case '*':
            return CH_STAR;
        case '|':
            return CH_OR;
        case '^':
            return CH_SOL;
        case '$':
            return CH_EOL;
    }

    n = boring_string(str);
    if (n < 3)
        return (unsigned char)str[0];
    else
        return CH_STRING_BASE + n - 1;
}

STATIC const char *nextch(const char *str)
{
    int n;

    switch (str[0])
    {
        case '\\':
            if (str[1] >= '1' && str[1] <= '9')
                return str + 2;
            switch (str[1])
            {
                case '`':
                case '\'':
                case '<':
                case '>':
                case 'w':
                case 'W':
                case 'B':
                case 'y':
                    return str + 2;
                default:
                    return past_escaped(str + 1);
            }

        case '~':
            if (str[1] == '\\')
                return past_escaped(str + 2);
            else if (str[1] != '\0')
                return str + 2;
            else
                return str + 1;
        case '.':
        case '[':
        case '(':
        case ')':
        case '{':
        case '}':
        case '?':
        case '+':
        case '*':
        case '|':
        case '^':
        case '$':
            return str + 1;
    }

    n = boring_string(str);
    if (n < 3)
        return str + 1;
    else
        return str + n - 1;
}

STATIC MATCH *compile_match(const char *str, const char **str_after, int erecf, int *rc);

STATIC const char *scan_number(const char *str, unsigned *num)
{
    if (!isdigit(*str))
        return NULL;
    *num = 0;
    do
        (*num) = (*num) * 10 + (*str++ - '0');
    while (isdigit(*str));
    return str;
}

STATIC MATCH *create_match(int *rc)
{
    MATCH *match;

    if ((match = (MATCH *) malloc(sizeof(MATCH))) == NULL)
    {
        *rc = ERROR_NOT_ENOUGH_MEMORY;
        return NULL;
    }
    return match;
}

STATIC MATCH *compile_term(const char *str, const char **str_after, int erecf, int *rc)
{
    MATCH *match;
    int c;

    c = thisch(str);
    switch (c)
    {
        case CH_RSQR:
            *rc = EREE_UNEX_RSQR;
            return NULL;
        case CH_QUERY:
            *rc = EREE_UNEX_QUERY;
            return NULL;
        case CH_PLUS:
            *rc = EREE_UNEX_PLUS;
            return NULL;
        case CH_STAR:
            *rc = EREE_UNEX_STAR;
            return NULL;
        case CH_LCUR:
            *rc = EREE_UNEX_LCUR;
            return NULL;
        case CH_RCUR:
            *rc = EREE_UNEX_RCUR;
            return NULL;
    }

    if (c == CH_EOS || c == CH_OR || c == CH_RPAR)
    {
        *str_after = str;
        return NULL_MATCH;
    }

    if ((match = create_match(rc)) == NULL)
        return NULL;

    if (CH_NOT_BASE <= c && c < CH_NOT_END)
/*...snot a specific character:16: */
    {
        char ch = (char)(c - CH_NOT_BASE);

        if (erecf & ERECF_TOLOWER)
            ch = (char)tolower(ch);
        match->mtype = MTYPE_NCHAR;
        match->u.character = ch;
        str = nextch(str);
    }
    else if (CH_STRING_BASE <= c && c < CH_STRING_END)
/*...sa string of non\45\special characters:16: */
    {
        unsigned len = c - CH_STRING_BASE;

        match->mtype = MTYPE_STRING;
        match->u.string = (char *)malloc(1 + len);
        if (match->u.string == NULL)
        {
            free(match);
            *rc = ERROR_NOT_ENOUGH_MEMORY;
            return NULL;
        }
        match->u.string[0] = (char)len;
        memcpy(match->u.string + 1, str, len);
        if (erecf & ERECF_TOLOWER)
        {
            unsigned i;

            for (i = 1; i <= len; i++)
                match->u.string[i] = (char)tolower(match->u.string[i]);
        }
        str = nextch(str);
    }
    else if (CH_BACK_BASE <= c && c < CH_BACK_END)
/*...sa backreference:16: */
    {
        match->mtype = MTYPE_BACK;
        match->u.n_span = c - CH_BACK_BASE;
        str = nextch(str);
    }
    else
        switch (c)
        {
/*...sCH_LSQR   \45\ character class:24: */
            case CH_LSQR:
                match->mtype = MTYPE_CCLASS;
                str = nextch(str);
                if ((match->u.cclass = compile_cclass(str, &str, erecf, rc)) == NULL)
                {
                    free(match);
                    return NULL;
                }
                break;
/*...sCH_DOT    \45\ any character:24: */
            case CH_DOT:
                match->mtype = MTYPE_DOT;
                str = nextch(str);
                break;
/*...sCH_WORD   \45\ \92\w:24: */
            case CH_WORD:
                match->mtype = MTYPE_WORD;
                str = nextch(str);
                break;
/*...sCH_NWORD  \45\ \92\W:24: */
            case CH_NWORD:
                match->mtype = MTYPE_NWORD;
                str = nextch(str);
                break;
/*...sCH_LPAR   \45\ nested regular expression:24: */
            case CH_LPAR:
                {
                    MATCH *sub_match;

                    str = nextch(str);
                    if ((sub_match = compile_match(str, &str, erecf, rc)) == NULL)
                    {
                        free(match);
                        return NULL;
                    }
                    if (!got_backrefs(sub_match))
                        sub_match = remove_subs(sub_match);
                    if (thisch(str) != CH_RPAR)
                    {
                        *rc = EREE_UNF_SUB;
                        return NULL;
                    }
                    str = nextch(str);
                    match->mtype = MTYPE_SUB;
                    match->u.match = sub_match;
                }
                break;
/*...sCH_SOL    \45\ \94\:24: */
            case CH_SOL:
                match->mtype = MTYPE_SOL;
                str = nextch(str);
                break;
/*...sCH_EOL    \45\ \36\:24: */
            case CH_EOL:
                match->mtype = MTYPE_EOL;
                str = nextch(str);
                break;
/*...sCH_SOW    \45\ \92\\60\:24: */
            case CH_SOW:
                match->mtype = MTYPE_SOW;
                str = nextch(str);
                break;
/*...sCH_EOW    \45\ \92\\62\:24: */
            case CH_EOW:
                match->mtype = MTYPE_EOW;
                str = nextch(str);
                break;
/*...sCH_IW     \45\ \92\B:24: */
            case CH_IW:
                match->mtype = MTYPE_IW;
                str = nextch(str);
                break;
/*...sCH_EW     \45\ \92\y:24: */
            case CH_EW:
                match->mtype = MTYPE_EW;
                str = nextch(str);
                break;
/*...sdefault   \45\ any old character:24: */
            default:
                {
                    char ch = (char)c;

                    if (erecf & ERECF_TOLOWER)
                        ch = (char)tolower(ch);
                    match->mtype = MTYPE_CHAR;
                    match->u.character = ch;
                    str = nextch(str);
                }
                break;
        }

    *str_after = str;
    return match;
}

STATIC MTYPE repeat_type_of(int c)
{
    switch (c)
    {
        case CH_QUERY:
            return MTYPE_QUERY;
        case CH_PLUS:
            return MTYPE_PLUS;
        case CH_STAR:
            return MTYPE_STAR;
        case CH_LCUR:
            return MTYPE_CREP;
        default:
            return (MTYPE)-1;
    }
}

STATIC MATCH *compile_factor(const char *str, const char **str_after, int erecf, int *rc)
{
    MATCH *match, *parent;
    MTYPE repeat_mtype;

    if ((match = compile_term(str, &str, erecf, rc)) == NULL)
        return NULL;

    while ((repeat_mtype = repeat_type_of(thisch(str))) != (MTYPE) - 1)
/*...smatch is to be repeated:16: */
    {
        if ((parent = create_match(rc)) == NULL)
        {
            delete_match(match);
            return NULL;
        }

        parent->mtype = repeat_mtype;
        str = nextch(str);
        if (repeat_mtype == MTYPE_CREP)
        {
            parent->u.crep.match = match;
            if ((str = scan_number(str, &(parent->u.crep.m))) == NULL)
            {
                delete_match(match);
                free(parent);
                *rc = EREE_BAD_CREP_M;
                return NULL;
            }
            parent->u.crep.n = parent->u.crep.m;
            if (*str == ',')
            {
                ++str;
                if (*str != '}')
                {
                    if ((str = scan_number(str, &(parent->u.crep.n))) == NULL)
                    {
                        delete_match(match);
                        free(parent);
                        *rc = EREE_BAD_CREP_N;
                        return NULL;
                    }
                }
                else
                    parent->u.crep.n = (unsigned)~0;
            }
            if (*str != '}')
            {
                delete_match(match);
                free(parent);
                *rc = EREE_UNF_CREP;
                return NULL;
            }
            ++str;
            if (parent->u.crep.m > parent->u.crep.n)
            {
                delete_match(match);
                free(parent);
                *rc = EREE_BAD_CREP;
                return NULL;
            }
        }
        else
            parent->u.match = match;
        match = parent;
    }

    *str_after = str;

    return match;
}

STATIC MATCH *compile_factors(const char *str, const char **str_after, int erecf, int *rc)
{
    MATCH *match;
    int c;

    if ((match = compile_factor(str, &str, erecf, rc)) == NULL)
        return NULL;

    while ((c = thisch(str)) != CH_EOS && c != CH_RPAR && c != CH_OR)
/*...sconsider catenation of more factors:16: */
    {
        MATCH *sibling, *parent;

        if ((sibling = compile_factor(str, &str, erecf, rc)) == NULL)
        {
            delete_match(match);
            return NULL;
        }

        if ((parent = create_match(rc)) == NULL)
        {
            delete_match(sibling);
            delete_match(match);
            return NULL;
        }
        parent->mtype = MTYPE_CAT;
        parent->u.matchs[0] = match;
        parent->u.matchs[1] = sibling;
        match = parent;
    }

    *str_after = str;
    return match;
}

/*...scompile_match   \45\ factors\124\factors:0: */
STATIC MATCH *compile_match(const char *str, const char **str_after, int erecf, int *rc)
{
    MATCH *match;

    if ((match = compile_factors(str, &str, erecf, rc)) == NULL)
        return NULL;

    while (thisch(str) == CH_OR)
/*...sfind sibling and or it in:16: */
    {
        MATCH *sibling, *parent;

        str = nextch(str);
        if ((sibling = compile_factors(str, &str, erecf, rc)) == NULL)
        {
            delete_match(match);
            return NULL;
        }
        if ((parent = create_match(rc)) == NULL)
        {
            delete_match(sibling);
            delete_match(match);
            return NULL;
        }
        parent->mtype = MTYPE_OR;
        parent->u.matchs[0] = match;
        parent->u.matchs[1] = sibling;
        match = parent;
    }

    *str_after = str;

    return match;
}

#ifdef DEBUG
/*...sprint_tree:0: */
/*...sdo_indent:0: */
STATIC void do_indent(int indent)
{
    while (indent--)
        putchar('\t');
}

STATIC void print_tree(const MATCH * match, int indent)
{
    do_indent(indent);
    switch (match->mtype)
    {
        case MTYPE_NULL:
            printf("null\n");
            break;
        case MTYPE_CHAR:
            printf("%c\n", match->u.character);
            break;
        case MTYPE_NCHAR:
            printf("~%c\n", match->u.character);
            break;
        case MTYPE_STRING:
            printf("%*.*s\n",
                   (unsigned char)match->u.string[0],
                   (unsigned char)match->u.string[0],
                   match->u.string + 1);
            break;
        case MTYPE_CCLASS:
            {
                int i;

                printf("[");
                for (i = 0; i < 0x100; i++)
                    if (match_cclass(i, match->u.cclass))
                        printf("%c", i);
                printf("]\n");
            }
            break;
        case MTYPE_DOT:
            printf(".\n");
            break;
        case MTYPE_WORD:
            printf("\\w\n");
            break;
        case MTYPE_NWORD:
            printf("\\W\n");
            break;
        case MTYPE_QUERY:
            printf("?\n");
            print_tree(match->u.match, indent + 1);
            break;
        case MTYPE_PLUS:
            printf("+\n");
            print_tree(match->u.match, indent + 1);
            break;
        case MTYPE_STAR:
            printf("*\n");
            print_tree(match->u.match, indent + 1);
            break;
        case MTYPE_CREP:
            printf("{%u,%u}\n",
                   match->u.crep.m,
                   match->u.crep.n);
            print_tree(match->u.crep.match, indent + 1);
            break;
        case MTYPE_OR:
            printf("|\n");
            print_tree(match->u.matchs[0], indent + 1);
            print_tree(match->u.matchs[1], indent + 1);
            break;
        case MTYPE_CAT:
            printf("CAT\n");
            print_tree(match->u.matchs[0], indent + 1);
            print_tree(match->u.matchs[1], indent + 1);
            break;
        case MTYPE_SUB:
            printf("SUB\n");
            print_tree(match->u.match, indent + 1);
            break;
        case MTYPE_SOL:
            printf("^\n");
            break;
        case MTYPE_EOL:
            printf("$\n");
            break;
        case MTYPE_SOW:
            printf("\\<\n");
            break;
        case MTYPE_EOW:
            printf("\\>\n");
            break;
        case MTYPE_IW:
            printf("\\B\n");
            break;
        case MTYPE_EW:
            printf("\\y\n");
            break;
        case MTYPE_BACK:
            printf("\\%d\n", match->u.n_span + 1);
            break;
    }
}
#endif

/*...sfinite state machine:0: */
typedef unsigned char ETYPE;    // Edge type

#define ETYPE_CHAR  ((ETYPE)  0)    // Can advance if given character
#define ETYPE_NCHAR ((ETYPE)  1)    // Can advance if not given char.
#define ETYPE_STRING    ((ETYPE)  2)    // Can advance if strings match
#define ETYPE_DOT   ((ETYPE)  3)    // Can advance if any character
#define ETYPE_CCLASS    ((ETYPE)  4)    // Can advance if any in class
#define ETYPE_WORD  ((ETYPE)  5)    // Can advance if word constituent
#define ETYPE_NWORD ((ETYPE)  6)    // Can advance if not word constit.
#define ETYPE_EPSILON   ((ETYPE)  7)    // Can advance without reading input
#define ETYPE_SOL   ((ETYPE)  8)    // Matches if at start of line
#define ETYPE_EOL   ((ETYPE)  9)    // Matches if at end of line
#define ETYPE_SOW   ((ETYPE) 10)    // Matches if at start of word
#define ETYPE_EOW   ((ETYPE) 11)    // Matches if at end of word
#define ETYPE_IW    ((ETYPE) 12)    // Matches if within word
#define ETYPE_EW    ((ETYPE) 13)    // Matches if at word start or end
#define ETYPE_SSUB  ((ETYPE) 14)    // Records passage over (
#define ETYPE_ESUB  ((ETYPE) 15)    // Records passage over )
#define ETYPE_BACK  ((ETYPE) 16)    // Back reference

typedef struct
{
    ETYPE etype;                // Edge type, (an ETYPE_ no)
    union
    {
        char character;         // Character to use if ETYPE_CHAR
        unsigned char *cclass;  // Class to use if ETYPE_CCLASS
        char *string;           // len, then chars if ETYPE_STRING
        int n_span;             // Used if ETYPE_BACK
    }
    u;
    const char *gate;           // Used if type of epsilon move
    int to_state;               // State to go to if test succeeds
    int next_edge;              // Next test to try after this
}
EDGE;

/* Under DOS, restrict FSM size to 13KB approx.. On other environments memory
 * isn't such an issue, so allow something closer to 32KB. Of course, a better
 * implementation would dynamically grow the FSM size as needed. Something for
 * the future perhaps... */

#ifdef xxDOS
#define MAX_STATES   200
#define MAX_EDGES    600
#else
#define MAX_STATES   500
#define MAX_EDGES   1500
#endif

#define FLAG_FINISH 0x01
#define FLAG_VISITED    0x02
#define FLAG_REACHABLE  0x04

typedef struct
{
    int n_states;
    int state_first_edges[MAX_STATES];
    unsigned char state_flags[MAX_STATES];
    int n_edges;
    EDGE edges[MAX_EDGES];
}
FSM;

/*...screate_fsm:0: */
STATIC FSM *create_fsm(int *rc)
{
    FSM *fsm;
    int i;

    if ((fsm = (FSM *) malloc(sizeof(FSM))) == NULL)
    {
        *rc = ERROR_NOT_ENOUGH_MEMORY;
        return NULL;
    }

    fsm->n_states = 0;
    fsm->n_edges = 0;

    for (i = 0; i < MAX_STATES; i++)
    {
        fsm->state_first_edges[i] = -1;
        fsm->state_flags[i] = 0;
    }

    return fsm;
}
/*...sdelete_fsm:0: */
#define delete_fsm(fsm) free(fsm)

/*...smalloc_state:0: */
STATIC int malloc_state(FSM * fsm)
{
    if (fsm->n_states == MAX_STATES)
        return -1;
    else
        return fsm->n_states++;
}
/*...smalloc_edge:0: */
/* Allocate space for new supplied edge and fill it in.
 * If already exists then don't bother (duplicates waste search time).
 * Return TRUE if all went ok. */

STATIC BOOLEAN malloc_edge(int s, EDGE * edge, FSM * fsm)
{
    int edge_no, n_edges = fsm->n_edges++;

    // See if edge already exists

    for (edge_no = fsm->state_first_edges[s];
         edge_no != -1;
         edge_no = fsm->edges[edge_no].next_edge)
        // Do we already have this edge
        if (fsm->edges[edge_no].to_state == edge->to_state &&
            fsm->edges[edge_no].etype == edge->etype)
            // An edge of this type already exists
            switch (edge->etype)
            {
/*...sETYPE_CHAR\47\NCHAR:32: */
                case ETYPE_CHAR:
                case ETYPE_NCHAR:
                    if (edge->u.character == fsm->edges[edge_no].u.character)
                        return TRUE;
                    break;
/*...sETYPE_STRING:32: */
                case ETYPE_STRING:
                    if ((unsigned char)fsm->edges[edge_no].u.string[0] ==
                        (unsigned char)edge->u.string[0] &&
                        !memcmp(fsm->edges[edge_no].u.string + 1,
                                edge->u.string + 1,
                                (unsigned char)edge->u.string[0]))
                        return TRUE;
                    break;
/*...sETYPE_CCLASS:32: */
                case ETYPE_CCLASS:
                    if (edge->u.cclass == fsm->edges[edge_no].u.cclass)
                        return TRUE;
                    break;
/*...sETYPE_DOT\47\EPSILON\47\SOL\47\EOL etc\46\\46\:32: */
                case ETYPE_DOT:
                case ETYPE_EPSILON:
                case ETYPE_SOL:
                case ETYPE_EOL:
                case ETYPE_SOW:
                case ETYPE_EOW:
                case ETYPE_IW:
                case ETYPE_EW:
                case ETYPE_SSUB:
                case ETYPE_ESUB:
                    return TRUE;
/*...sETYPE_BACK:32: */
                case ETYPE_BACK:
                    if (edge->u.n_span == fsm->edges[edge_no].u.n_span)
                        return TRUE;
                    break;
            }

    // Going to have to add the edge

    if (n_edges >= MAX_EDGES)
        return FALSE;

    memcpy(&(fsm->edges[n_edges]), edge, sizeof(EDGE));
    fsm->edges[n_edges].next_edge = fsm->state_first_edges[s];
    fsm->state_first_edges[s] = n_edges;
    return TRUE;
}
/*...smake_fsm_from_match:0: */
/*...sadd_edge_to_fsm_character:0: */
STATIC BOOLEAN add_edge_to_fsm_character(int s, int f, FSM * fsm, char character)
{
    EDGE edge;

    edge.etype = ETYPE_CHAR;
    edge.u.character = character;
    edge.to_state = f;
    return malloc_edge(s, &edge, fsm);
}
/*...sadd_edge_to_fsm_ncharacter:0: */
STATIC BOOLEAN add_edge_to_fsm_ncharacter(int s, int f, FSM * fsm, char character)
{
    EDGE edge;

    edge.etype = ETYPE_NCHAR;
    edge.u.character = character;
    edge.to_state = f;
    return malloc_edge(s, &edge, fsm);
}
/*...sadd_edge_to_fsm_string:0: */
STATIC BOOLEAN add_edge_to_fsm_string(int s, int f, FSM * fsm, char *string)
{
    EDGE edge;

    edge.etype = ETYPE_STRING;
    edge.u.string = string;
    edge.to_state = f;
    return malloc_edge(s, &edge, fsm);
}
/*...sadd_edge_to_fsm_cclass:0: */
STATIC BOOLEAN add_edge_to_fsm_cclass(int s, int f, FSM * fsm, unsigned char *cclass)
{
    EDGE edge;

    edge.etype = ETYPE_CCLASS;
    edge.u.cclass = cclass;
    edge.to_state = f;
    return malloc_edge(s, &edge, fsm);
}
/*...sadd_edge_to_fsm_dot:0: */
STATIC BOOLEAN add_edge_to_fsm_dot(int s, int f, FSM * fsm)
{
    EDGE edge;

    edge.etype = ETYPE_DOT;
    edge.to_state = f;
    return malloc_edge(s, &edge, fsm);
}
/*...sadd_edge_to_fsm_word:0: */
STATIC BOOLEAN add_edge_to_fsm_word(int s, int f, FSM * fsm)
{
    EDGE edge;

    edge.etype = ETYPE_WORD;
    edge.to_state = f;
    return malloc_edge(s, &edge, fsm);
}
/*...sadd_edge_to_fsm_nword:0: */
STATIC BOOLEAN add_edge_to_fsm_nword(int s, int f, FSM * fsm)
{
    EDGE edge;

    edge.etype = ETYPE_NWORD;
    edge.to_state = f;
    return malloc_edge(s, &edge, fsm);
}
/*...sadd_edge_to_fsm_epsilon:0: */
STATIC BOOLEAN add_edge_to_fsm_epsilon(int s, int f, FSM * fsm)
{
    EDGE edge;

    edge.etype = ETYPE_EPSILON;
    edge.to_state = f;
    return malloc_edge(s, &edge, fsm);
}
/*...sadd_edge_to_fsm_special:0: */
STATIC BOOLEAN add_edge_to_fsm_special(int s, int f, FSM * fsm, ETYPE etype)
{
    EDGE edge;

    edge.etype = etype;
    edge.to_state = f;
    edge.gate = NULL;
    return malloc_edge(s, &edge, fsm);
}
/*...sadd_edge_to_fsm_back:0: */
STATIC BOOLEAN add_edge_to_fsm_back(int s, int f, FSM * fsm, int n_span)
{
    EDGE edge;

    edge.etype = ETYPE_BACK;
    edge.to_state = f;
    edge.gate = NULL;
    edge.u.n_span = n_span;
    return malloc_edge(s, &edge, fsm);
}

STATIC BOOLEAN make_fsm_from_match(MATCH * match, FSM * fsm, int *s, int *f)
{
    int n1, n2, n3, n4, i;

    switch (match->mtype)
    {
/*...sMTYPE_NULL:16: */
/*
 * e
 * S ----> F
 *
 */

        case MTYPE_NULL:
            return (*s = malloc_state(fsm)) != -1 &&
                (*f = malloc_state(fsm)) != -1 &&
                add_edge_to_fsm_epsilon(*s, *f, fsm);
/*...sMTYPE_CHAR:16: */
/*
 * c
 * S ----> F
 *
 */

        case MTYPE_CHAR:
            return (*s = malloc_state(fsm)) != -1 &&
                (*f = malloc_state(fsm)) != -1 &&
                add_edge_to_fsm_character(*s, *f, fsm, match->u.character);
/*...sMTYPE_NCHAR:16: */
/*
 * ~c
 * S ----> F
 *
 */

        case MTYPE_NCHAR:
            return (*s = malloc_state(fsm)) != -1 &&
                (*f = malloc_state(fsm)) != -1 &&
                add_edge_to_fsm_ncharacter(*s, *f, fsm, match->u.character);
/*...sMTYPE_STRING:16: */
/*
 * string
 * S ----> F
 *
 */

        case MTYPE_STRING:
            return (*s = malloc_state(fsm)) != -1 &&
                (*f = malloc_state(fsm)) != -1 &&
                add_edge_to_fsm_string(*s, *f, fsm, match->u.string);

/*...sMTYPE_CCLASS:16: */
/*
 * cclass
 * S ----> F
 *
 */

        case MTYPE_CCLASS:
            return (*s = malloc_state(fsm)) != -1 &&
                (*f = malloc_state(fsm)) != -1 &&
                add_edge_to_fsm_cclass(*s, *f, fsm, match->u.cclass);

/*...sMTYPE_DOT:16: */
/*
 * any
 * S ----> F
 *
 */

        case MTYPE_DOT:
            return (*s = malloc_state(fsm)) != -1 &&
                (*f = malloc_state(fsm)) != -1 &&
                add_edge_to_fsm_dot(*s, *f, fsm);

/*...sMTYPE_WORD:16: */
/*
 * word
 * S ----> F
 *
 */

        case MTYPE_WORD:
            return (*s = malloc_state(fsm)) != -1 &&
                (*f = malloc_state(fsm)) != -1 &&
                add_edge_to_fsm_word(*s, *f, fsm);

/*...sMTYPE_NWORD:16: */
/*
 * !word
 * S ----> F
 *
 */

        case MTYPE_NWORD:
            return (*s = malloc_state(fsm)) != -1 &&
                (*f = malloc_state(fsm)) != -1 &&
                add_edge_to_fsm_nword(*s, *f, fsm);

/*...sMTYPE_QUERY:16: */
/*
 * e
 * S ----> F
 * |       ^
 * | e     | e
 * v       |
 * [n1     n2]
 *
 */

        case MTYPE_QUERY:
            if (!make_fsm_from_match(match->u.match, fsm, &n1, &n2))
                return FALSE;
            return (*s = malloc_state(fsm)) != -1 &&
                (*f = malloc_state(fsm)) != -1 &&
                add_edge_to_fsm_epsilon(*s, *f, fsm) &&
                add_edge_to_fsm_epsilon(*s, n1, fsm) &&
                add_edge_to_fsm_epsilon(n2, *f, fsm);

/*...sMTYPE_PLUS:16: */
/*
 *
 * [S   F]
 * ^ e |
 * +---+
 *
 */

        case MTYPE_PLUS:
            if (!make_fsm_from_match(match->u.match, fsm, s, f))
                return FALSE;
            return add_edge_to_fsm_epsilon(*f, *s, fsm);

/*...sMTYPE_STAR:16: */
/*
 *
 * +---- S/F
 * |      ^
 * | e    | e
 * v      |
 * [n1     n2]
 *
 */

        case MTYPE_STAR:
            if (!make_fsm_from_match(match->u.match, fsm, &n1, &n2))
                return FALSE;
            return (*s = *f = malloc_state(fsm)) != -1 &&
                add_edge_to_fsm_epsilon(*s, n1, fsm) &&
                add_edge_to_fsm_epsilon(n2, *f, fsm);

/*...sMTYPE_CREP:16: */
/*
 * e          e          e          e
 * S-->[n1  n2]-->[n1  n2]-->[n1  n2]-->[n1  n2]      <re>{2,4}
 * |          |          |
 * | e        | e        | e
 * v          |          |
 * F<---------+----------+
 *
 * e
 * +----+
 * |    |
 * e          e          e  v    |
 * S-->[n1  n2]-->[n1  n2]-->[n1  n2]         <re>{2,}
 * |          |
 * | e        | e
 * v          |
 * F<---------+
 */

        case MTYPE_CREP:
            if ((*s = malloc_state(fsm)) == -1 ||
                (*f = malloc_state(fsm)) == -1)
                return FALSE;
            n3 = *s;
            for (i = 0; i < (int)match->u.crep.m; i++)
            {
                if (!make_fsm_from_match(match->u.crep.match, fsm, &n1, &n2))
                    return FALSE;
                if (!add_edge_to_fsm_epsilon(n3, n1, fsm))
                    return FALSE;
                n3 = n2;
            }
            if (!add_edge_to_fsm_epsilon(n3, *f, fsm))
                return FALSE;
            if (match->u.crep.n != ~0)
                for (; i < (int)match->u.crep.n; i++)
                {
                    if (!make_fsm_from_match(match->u.crep.match, fsm, &n1, &n2))
                        return FALSE;
                    if (!add_edge_to_fsm_epsilon(n3, n1, fsm))
                        return FALSE;
                    if (!add_edge_to_fsm_epsilon(n2, *f, fsm))
                        return FALSE;
                    n3 = n2;
                }
            else
            {
                if (!make_fsm_from_match(match->u.crep.match, fsm, &n1, &n2))
                    return FALSE;
                if (!add_edge_to_fsm_epsilon(n3, n1, fsm))
                    return FALSE;
                if (!add_edge_to_fsm_epsilon(n2, *f, fsm))
                    return FALSE;
                if (!add_edge_to_fsm_epsilon(n2, n1, fsm))
                    return FALSE;
            }
            return TRUE;

/*...sMTYPE_OR:16: */
/*
 * e           e
 * +--->[n1 n2]----+
 * |               v
 * S               F
 * | e           e ^
 * +--->[n3 n4]----+
 *
 */

        case MTYPE_OR:
            if (!make_fsm_from_match(match->u.matchs[0], fsm, &n1, &n2) ||
                !make_fsm_from_match(match->u.matchs[1], fsm, &n3, &n4))
                return FALSE;
            return (*s = malloc_state(fsm)) != -1 &&
                (*f = malloc_state(fsm)) != -1 &&
                add_edge_to_fsm_epsilon(*s, n1, fsm) &&
                add_edge_to_fsm_epsilon(*s, n3, fsm) &&
                add_edge_to_fsm_epsilon(n2, *f, fsm) &&
                add_edge_to_fsm_epsilon(n4, *f, fsm);

/*...sMTYPE_CAT:16: */
/*
 * e
 * [S  n1]---->[n2  F]
 *
 */

        case MTYPE_CAT:
            if (!make_fsm_from_match(match->u.matchs[0], fsm, s, &n1) ||
                !make_fsm_from_match(match->u.matchs[1], fsm, &n2, f))
                return FALSE;
            return add_edge_to_fsm_epsilon(n1, n2, fsm);

/*...sMTYPE_SUB:16: */
/*
 * ssub         esub
 * S---->[n1  n2]---->F
 *
 */

        case MTYPE_SUB:
            if (!make_fsm_from_match(match->u.match, fsm, &n1, &n2))
                return FALSE;
            return (*s = malloc_state(fsm)) != -1 &&
                (*f = malloc_state(fsm)) != -1 &&
                add_edge_to_fsm_special(*s, n1, fsm, ETYPE_SSUB) &&
                add_edge_to_fsm_special(n2, *f, fsm, ETYPE_ESUB);

/*...sMTYPE_SOL\47\EOL\47\SOW\47\EOW\47\IW\47\EW:16: */
/*
 * special
 * S ----> F
 *
 */

        case MTYPE_SOL:
            return (*s = malloc_state(fsm)) != -1 &&
                (*f = malloc_state(fsm)) != -1 &&
                add_edge_to_fsm_special(*s, *f, fsm, ETYPE_SOL);
        case MTYPE_EOL:
            return (*s = malloc_state(fsm)) != -1 &&
                (*f = malloc_state(fsm)) != -1 &&
                add_edge_to_fsm_special(*s, *f, fsm, ETYPE_EOL);
        case MTYPE_SOW:
            return (*s = malloc_state(fsm)) != -1 &&
                (*f = malloc_state(fsm)) != -1 &&
                add_edge_to_fsm_special(*s, *f, fsm, ETYPE_SOW);
        case MTYPE_EOW:
            return (*s = malloc_state(fsm)) != -1 &&
                (*f = malloc_state(fsm)) != -1 &&
                add_edge_to_fsm_special(*s, *f, fsm, ETYPE_EOW);
        case MTYPE_IW:
            return (*s = malloc_state(fsm)) != -1 &&
                (*f = malloc_state(fsm)) != -1 &&
                add_edge_to_fsm_special(*s, *f, fsm, ETYPE_IW);
        case MTYPE_EW:
            return (*s = malloc_state(fsm)) != -1 &&
                (*f = malloc_state(fsm)) != -1 &&
                add_edge_to_fsm_special(*s, *f, fsm, ETYPE_EW);

/*...sMTYPE_BACK:16: */
        case MTYPE_BACK:
            return (*s = malloc_state(fsm)) != -1 &&
                (*f = malloc_state(fsm)) != -1 &&
                add_edge_to_fsm_back(*s, *f, fsm, match->u.n_span);

    }
    return FALSE;               // Should never happen
}

/*...sremove_epsilons:0: */
/* The problems with epsilon moves are :-
 * 1) They can gang up on you in groups to form loops of states that require
 * no input to go all the way around them. Any function attempting a
 * recursive search of the FSM will recurse forever.
 * 2) They can slow the recognition process by as much as a factor of 2.
 * eg:
 * a       e       b       e       c
 * O ----> O ----> O ----> O ----> O ----> O   Is slow
 *
 * a       b       c
 * O ----> O ----> O ----> O           Is faster
 *
 */

/*...sfinish_states:0: */
/*...sis_finish_reachable:0: */
/* When we recurse we mark the current state as visited to stop infinite
 * recursion on loops of epsilon moves. */

STATIC BOOLEAN is_finish_reachable(FSM * fsm, int state_no)
{
    int edge_no;
    BOOLEAN ok;

    if (fsm->state_flags[state_no] & FLAG_VISITED)
        // Been here already
        return FALSE;

    if (fsm->state_flags[state_no] & FLAG_FINISH)
        // At finish
        return TRUE;

    for (edge_no = fsm->state_first_edges[state_no];
         edge_no != -1;
         edge_no = fsm->edges[edge_no].next_edge)
        if (fsm->edges[edge_no].etype == ETYPE_EPSILON)
        {
            fsm->state_flags[state_no] |= FLAG_VISITED;
            ok = is_finish_reachable(fsm, fsm->edges[edge_no].to_state);
            fsm->state_flags[state_no] &= ~FLAG_VISITED;
            if (ok)
                return TRUE;
        }


    return FALSE;
}


STATIC void finish_states(int f, FSM * fsm, FSM * fsm_without)
{
    int state_no;

    fsm->state_flags[f] = FLAG_FINISH;
    fsm_without->state_flags[f] = FLAG_FINISH;
    for (state_no = 0; state_no < fsm->n_states; state_no++)
        if (is_finish_reachable(fsm, state_no))
            fsm_without->state_flags[state_no] = FLAG_FINISH;
}

/*...sdetermine_reachable:0: */
STATIC void determine_reachable(int s, FSM * fsm, FSM * fsm_without)
{
    int edge_no, to_state;

    fsm_without->state_flags[s] |= FLAG_REACHABLE;
    for (edge_no = 0; edge_no < fsm->n_edges; edge_no++)
        if (fsm->edges[edge_no].etype != ETYPE_EPSILON)
        {
            to_state = fsm->edges[edge_no].to_state;
            fsm_without->state_flags[to_state] |= FLAG_REACHABLE;
        }
}

/*...scopy_non_epsilons:0: */
STATIC void copy_non_epsilons(FSM * fsm, FSM * fsm_without)
{
    int state_no, edge_no;

    for (state_no = 0; state_no < fsm->n_states; state_no++)
        if (fsm_without->state_flags[state_no] & FLAG_REACHABLE)
            for (edge_no = fsm->state_first_edges[state_no];
                 edge_no != -1;
                 edge_no = fsm->edges[edge_no].next_edge)
                if (fsm->edges[edge_no].etype != ETYPE_EPSILON)
                    malloc_edge(state_no, &(fsm->edges[edge_no]), fsm_without);
}

/*...sfollow_epsilons:0: */
/*...scopy_edges_reachable:0: */
/* What this says is :-
 * If state A can reach state B, by an epsilon move, and
 * state B can reach state C then
 * state A can reach state C *
 *
 * If the state B to state C involves an epsilon move then
 * A can reach whatever C can reach too (by recursion)
 */

STATIC BOOLEAN copy_edges_reachable(
                                       FSM * fsm,
                                       FSM * fsm_without,
                                       int state_no_to,     /* AK: Bad identifier, might better be
                                                             * described as 'original reachable state' */
                                       int state_no_from
)
{
    int edge_no;
    BOOLEAN ok;

    if (fsm->state_flags[state_no_from] & FLAG_VISITED)
        // Been here already, therefore all copied from here ok
        return TRUE;

    for (edge_no = fsm->state_first_edges[state_no_from];
         edge_no != -1;
         edge_no = fsm->edges[edge_no].next_edge)
        if (fsm->edges[edge_no].etype != ETYPE_EPSILON)
            // Had better add this edge
/*...sadd this edge to the \39\to\39\ state:24: */
        {
            if (!malloc_edge(state_no_to, &(fsm->edges[edge_no]), fsm_without))
                return (FALSE);
        }

        else
        {
            fsm->state_flags[state_no_from] |= FLAG_VISITED;
            ok = copy_edges_reachable(fsm, fsm_without, state_no_to, fsm->edges[edge_no].to_state);
            fsm->state_flags[state_no_from] &= ~FLAG_VISITED;
            if (!ok)
                return FALSE;
        }

    return TRUE;
}


STATIC BOOLEAN follow_epsilons(FSM * fsm, FSM * fsm_without)
{
    int state_no;

    for (state_no = 0; state_no < fsm->n_states; state_no++)
        if (fsm_without->state_flags[state_no] & FLAG_REACHABLE)
            if (!copy_edges_reachable(fsm, fsm_without, state_no, state_no))
                return FALSE;
    return TRUE;
}


STATIC BOOLEAN remove_epsilons(int s, int f, FSM * fsm, FSM * fsm_without)
{
    // FSM with no epsilon moves will have the same number of states

    fsm_without->n_states = fsm->n_states;

    // Mark state f as a finish state in the new FSM
    // Any state with epsilon move(s) to state f is also a finish state

    finish_states(f, fsm, fsm_without);

    // Determine which states can be reached by non-epsilon moves. Add
    // to this set, the start state. The resulting states should have
    // their edges considered, but the other will not be reachable. This
    // is because they will be bypassed by follow_epsilons().

    determine_reachable(s, fsm, fsm_without);

    // Copy across all reachable, non epsilon moves to new FSM

    copy_non_epsilons(fsm, fsm_without);

    // For all states, determine all other states that can be reached by
    // epsilon moves and add the edges leading from them to us

    return follow_epsilons(fsm, fsm_without);
}

/*...smatch_fsm:0: */
/* Stack requirements per call, (one call per character in string!).
 *
 * eg:  16 bit OS/2, large model :-
 * Return address + Stack frame + SI and DI + Arguments + Locals
 * 2              + 2           + 2+2       + 4+2+4+4   + 2      = 24
 *
 * eg:  16 bit DOS large model :-
 * Return address + Stack frame + SI and DI + Arguments + Locals
 * 2              + 2           + 2+2       + 4+2       + 2      = 16
 *
 * eg:  RS/6000 AIX :-
 * Massive stack frame per call, massive program stack size
 * net effect - room for approx 200 levels before core dump!
 *
 * In addition, we can consume additional stack for every ETYPE_SSUB
 * (and ETYPE_ESUB). Guessing this requirement to be approx 100 bytes,
 * we place an arbitrary limit of 20 (ie: 2KB) on subexpressions in an ERE.
 * This limit was enforced earlier during match tree parsing.
 *
 */

typedef struct substruct SUBS;
struct substruct
{
    int n_spans;
    ERE_SPAN spans[MAX_SPANS];
    SUBS *next;
};

#define MAX_SUBS 20

typedef struct
{
    FSM *fsm;
    int eremf;
    const char *str_init;
    const char *str_limit;
    const char *str_best;
    SUBS *subs;
    SUBS *subs_base;
    ERE_MATCHINFO *mi;
}
CONTEXT;

#define NR

STATIC void NR walk_fsm(const char *str, int state_no, CONTEXT * cx);

STATIC void NR walk_fsm_gated(const char *str, CONTEXT * cx, EDGE * e)
{
    if (e->gate != str)
        // Avoid looping via this edge
    {
        const char *gate = e->gate;

        e->gate = str;
        walk_fsm(str, e->to_state, cx);
        e->gate = gate;
    }
}

STATIC void NR walk_fsm_ssub(const char *str, CONTEXT * cx, EDGE * e)
{
    SUBS subs;

    if (cx->subs->n_spans < MAX_SPANS)
        cx->subs->spans[cx->subs->n_spans].pos = str - cx->str_init;

    subs.n_spans = 0;
    subs.next = cx->subs;
    cx->subs = &subs;
    walk_fsm_gated(str, cx, e);
    cx->subs = subs.next;
}

STATIC void NR walk_fsm_esub(const char *str, CONTEXT * cx, EDGE * e)
{
    SUBS *subs = cx->subs;

    cx->subs = cx->subs->next;

    if (cx->subs->n_spans < MAX_SPANS)
        cx->subs->spans[cx->subs->n_spans].len =
            (str - cx->str_init) - cx->subs->spans[cx->subs->n_spans].pos;
    ++(cx->subs->n_spans);

    walk_fsm_gated(str, cx, e);

    --(cx->subs->n_spans);
    cx->subs = subs;
}

STATIC void NR walk_fsm(const char *str, int state_no, CONTEXT * cx)
{
    int edge_no;

    if (cx->fsm->state_flags[state_no] & FLAG_FINISH)
        // Got to finishing state, may have got a better match than before
    {
        if ((cx->str_best == NULL ||
             (cx->str_best < str) == (cx->eremf & EREMF_SHORTEST) == 0) &&
            str <= cx->str_limit)
        {
            cx->str_best = str;
            if (cx->mi != NULL)
            {
                int i;

                cx->mi->n_spans = cx->subs_base->n_spans;
                for (i = 0; i < cx->mi->n_spans; i++)
                    cx->mi->spans[i] = cx->subs_base->spans[i];
            }
        }
        if (cx->eremf & EREMF_ANY)
            return;
        // Continue, as may be able to get a better match
    }

    for (edge_no = cx->fsm->state_first_edges[state_no];
         edge_no != -1;
         edge_no = cx->fsm->edges[edge_no].next_edge)
        // Consider taking a step along an edge to a new state
    {
        EDGE *e = &(cx->fsm->edges[edge_no]);

        switch (e->etype)
        {
/*...sETYPE_CHAR   \45\ if matches character\44\ we can advance:24: */
            case ETYPE_CHAR:
                if (*str == e->u.character)
                    walk_fsm(str + 1, e->to_state, cx);
                break;

/*...sETYPE_NCHAR  \45\ if not matches character\44\ we can advance:24: */
            case ETYPE_NCHAR:
                if (*str != '\0' && *str != e->u.character)
                    walk_fsm(str + 1, e->to_state, cx);
                break;

/*...sETYPE_STRING \45\ if matches string\44\ we can advance:24: */
            case ETYPE_STRING:
                {
                    unsigned len = (unsigned char)e->u.string[0];

                    if (!memcmp(str, e->u.string + 1, len))
                        walk_fsm(str + len, e->to_state, cx);
                }
                break;

/*...sETYPE_DOT    \45\ if got any character\44\ we can advance:24: */
            case ETYPE_DOT:
                if (*str != '\0')
                    walk_fsm(str + 1, e->to_state, cx);
                break;

/*...sETYPE_WORD   \45\ if got word constituent\44\ we can advance:24: */
            case ETYPE_WORD:
                if (*str != '\0' && isword(*str))
                    walk_fsm(str + 1, e->to_state, cx);
                break;

/*...sETYPE_NWORD  \45\ if got non word constituent\44\ we can advance:24: */
            case ETYPE_NWORD:
                if (*str != '\0' && !isword(*str))
                    walk_fsm(str + 1, e->to_state, cx);
                break;

/*...sETYPE_CCLASS \45\ if in the class\44\ we can advance:24: */
            case ETYPE_CCLASS:
                if (match_cclass(*str, e->u.cclass))
                    walk_fsm(str + 1, e->to_state, cx);
                break;

/*...sETYPE_SOL\47\EOL\47\SOW\47\EOW\47\IW\47\EW \45\ special epsilon moves:24: */
            case ETYPE_SOL:
                if (str == cx->str_init)
                    walk_fsm_gated(str, cx, e);
                break;
            case ETYPE_EOL:
                if (*str == '\0')
                    walk_fsm_gated(str, cx, e);
                break;
            case ETYPE_SOW:
                if (isword(str[0]) &&
                    ((cx->str_init < str && !isword(str[-1])) ||
                     (cx->str_init == str)))
                    walk_fsm_gated(str, cx, e);
                break;
            case ETYPE_EOW:
                if ((cx->str_init < str && isword(str[-1])) &&
                    (str[0] == '\0' || !isword(str[0])))
                    walk_fsm_gated(str, cx, e);
                break;
            case ETYPE_IW:
                if (cx->str_init < str && isword(str[-1]) &&
                    str[0] != '\0' && isword(str[0]))
                    walk_fsm_gated(str, cx, e);
                break;
            case ETYPE_EW:
                if (isword(str[0]) &&
                    ((cx->str_init < str && !isword(str[-1])) ||
                     (cx->str_init == str)))
                    walk_fsm_gated(str, cx, e);
                else if ((cx->str_init < str && isword(str[-1])) &&
                         (str[0] == '\0' || !isword(str[0])))
                    walk_fsm_gated(str, cx, e);
                break;

/*...sETYPE_SSUB\47\ESUB \45\ handle nested subexpression:24: */
            case ETYPE_SSUB:
                walk_fsm_ssub(str, cx, e);
                break;
            case ETYPE_ESUB:
                walk_fsm_esub(str, cx, e);
                break;

/*...sETYPE_BACK \45\ check backreference:24: */
            case ETYPE_BACK:
                if (e->u.n_span < cx->subs->n_spans)
                {
                    int len = cx->subs->spans[e->u.n_span].len;

                    if (!memcmp(str, cx->str_init + cx->subs->spans[e->u.n_span].pos, len))
                        walk_fsm_gated(str + len, cx, e);
                }
                break;

        }
    }
}

STATIC const char *match_fsm(FSM * fsm,
                             int eremf,
                             const char *str,
                             int posn,
                             int limit,
                             int state_no,
                             ERE_MATCHINFO * mi)
{
    CONTEXT cx;
    SUBS subs;

    cx.fsm = fsm;
    cx.eremf = eremf;
    cx.str_init = str;
    cx.str_limit = str + limit;
    cx.str_best = NULL;
    cx.subs = &subs;
    cx.subs_base = &subs;
    cx.mi = mi;
    subs.n_spans = 0;
    walk_fsm(str + posn, state_no, &cx);
    return cx.str_best;
}


#ifdef DEBUG
/*...sprint_fsm:0: */
STATIC void print_fsm(FSM * fsm, int s, BOOLEAN not_just_reachable)
{
    int state_no, edge_no;

    printf("Starting state %02d\n", s);
    for (state_no = 0; state_no < fsm->n_states; state_no++)
        if ((fsm->state_flags[state_no] & FLAG_REACHABLE) != 0 ||
            not_just_reachable)
        {
            printf("%02d:%c\t", state_no, ((fsm->state_flags[state_no] & FLAG_FINISH) != 0) ? 'F' : ' ');
            for (edge_no = fsm->state_first_edges[state_no];
                 edge_no != -1;
                 edge_no = fsm->edges[edge_no].next_edge)
/*...sshow edge:32: */
            {
                EDGE *e = &(fsm->edges[edge_no]);

                switch (e->etype)
                {
                    case ETYPE_CHAR:
                        printf("%c", e->u.character);
                        break;
                    case ETYPE_NCHAR:
                        printf("~%c", e->u.character);
                        break;
                    case ETYPE_STRING:
                        printf("%*.*s",
                               (unsigned char)e->u.string[0],
                               (unsigned char)e->u.string[0],
                               e->u.string + 1);
                        break;
                    case ETYPE_DOT:
                        printf(".");
                        break;
                    case ETYPE_CCLASS:
                        printf("[");
                        break;
                    case ETYPE_WORD:
                        printf("\\w");
                        break;
                    case ETYPE_NWORD:
                        printf("\\W");
                        break;
                    case ETYPE_EPSILON:
                        printf("e");
                        break;
                    case ETYPE_SOL:
                        printf("^");
                        break;
                    case ETYPE_EOL:
                        printf("$");
                        break;
                    case ETYPE_SOW:
                        printf("\\<");
                        break;
                    case ETYPE_EOW:
                        printf("\\>");
                        break;
                    case ETYPE_IW:
                        printf("\\B");
                        break;
                    case ETYPE_EW:
                        printf("\\y");
                        break;
                    case ETYPE_SSUB:
                        printf("(");
                        break;
                    case ETYPE_ESUB:
                        printf(")");
                        break;
                    case ETYPE_BACK:
                        printf("\\%d", e->u.n_span + 1);
                        break;
                }
                printf("->%02d\t", e->to_state);
            }

            printf("\n");
        }
}

#endif

/*...sextended regular expressions:0: */
/* An ERE knows its original match tree and also the FSM it is compiled into.
 * Using a (largely epsilon move free) FSM makes for faster searching. */

typedef struct
{
    MATCH *match;               // Parse tree for expression
    int shortest_match;         // Shortest match possible
    FSM *fsm;                   // Compiled FSM
    int s;                      // Start state for FSM
}
ERE;

/*
 *@@ rxpCompile:
 *      compiles the regular expression str for later matching.
 *
 *      If ERECF_TOLOWER is passed with erecf, every character
 *      (or range of characters) to be matched are stored in the
 *      compiled ERE in lower case. Therefore, if strings to be
 *      matched are passed in lower case also, the result is a
 *      case-insensitive match.
 */

ERE* rxpCompile(const char *str,
                int erecf,
                int *rc)            // out: error code
{
    ERE *ere;
    const char *str_after;
    FSM *fsm;
    int s, f;

    *rc = NO_ERROR;

    if ((ere = (ERE *) malloc(sizeof(ERE))) == NULL)
    {
        *rc = ERROR_NOT_ENOUGH_MEMORY;
        return NULL;
    }

    if ((ere->match = compile_match(str, &str_after, erecf, rc)) == NULL)
    {
        free(ere);
        return NULL;
    }

    if (thisch(str_after) == CH_RPAR)
    {
        delete_match(ere->match);
        free(ere);
        *rc = EREE_UNEX_RPAR;
        return NULL;
    }

    if (count_sub(ere->match) > MAX_SUBS)
    {
        delete_match(ere->match);
        free(ere);
        *rc = EREE_TOO_MANY_SUB;
        return NULL;
    }

#ifdef DEBUG
    print_tree(ere->match, 0);
#endif

    ere->shortest_match = (int)shortest_match(ere->match);

    if ((fsm = create_fsm(rc)) == NULL)
    {
        delete_match(ere->match);
        free(ere);
        return NULL;
    }

    if (!make_fsm_from_match(ere->match, fsm, &s, &f))
    {
        delete_fsm(fsm);
        delete_match(ere->match);
        free(ere);
        *rc = EREE_COMPILE_FSM;
        return NULL;
    }

#ifdef DEBUG
    print_fsm(fsm, s, TRUE);
#endif

    if ((ere->fsm = create_fsm(rc)) == NULL)
    {
        delete_fsm(fsm);
        delete_match(ere->match);
        free(ere);
        *rc = EREE_COMPILE_FSM;
        return NULL;
    }

    if (!remove_epsilons(s, f, fsm, ere->fsm))
    {
        delete_fsm(ere->fsm);
        delete_fsm(fsm);
        delete_match(ere->match);
        free(ere);
        *rc = EREE_COMPILE_FSM;
        return NULL;
    }

    delete_fsm(fsm);

    ere->s = s;

#ifdef DEBUG
    print_fsm(ere->fsm, s, FALSE);
#endif

    return ere;
}

/*
 *@@ rxpMinLen:
 *
 */

int rxpMinLen(const ERE * ere)
{
    return ere->shortest_match;
}

/*
 *@@ rxpMatch:
 *      returns the number of characters in the match, starting from
 *      pos characters into the string to be searched. Details of
 *      sub-matches can also be returend. Returns -1 if no match.
 *
 *      If EREMF_SHORTEST is passed with eremf, the code looks for
 *      the shortest match, instead of the longest match.
 *
 *      If EREMF_ANY is passed with eremf, the code doesn't try to
 *      find the longest (or shortest) match, it will return with the
 *      first match it finds (which could be of any length).
 *      This can speed up matching.
 */

int rxpMatch(const ERE * ere,
             int eremf,
             const char *str,
             int pos,
             ERE_MATCHINFO * mi)
{
    int len = pos + strlen(str + pos);
    const char *str_best;

    if ((str_best = match_fsm(ere->fsm, eremf, str, pos, len, ere->s, mi)) == NULL)
        return -1;
    return (str_best - str) - pos;
}

/*
 *@@ rxpMatch_fwd:
 *      match forwards within a string from a specified start position.
 *
 *      If a match, return TRUE, and also return pos and len of the match.
 *
 *      If EREMF_SHORTEST is passed with eremf, the code looks for
 *      the shortest match, instead of the longest match.
 *
 *      If EREMF_ANY is passed with eremf, the code doesn't try to
 *      find the longest (or shortest) match, it will return with the
 *      first match it finds (which could be of any length).
 *      This can speed up matching.
 */

BOOLEAN rxpMatch_fwd(const ERE *ere,        // in: compiled ERE (from rxpCompile)
                     int eremf,             // in: EREMF_* flags
                     const char *str,       // in: string to test
                     int pos,               // in: start position
                     int *pos_match,        // out: position of match
                     int *len_match,        // out: length of match
                     ERE_MATCHINFO *mi)     // out: match info (for rxpSubsWith)
{
    int len = pos + strlen(str + pos);
    int i;

    for (i = pos; i <= len - ere->shortest_match; i++)
    {
        const char *str_best;

        if ((str_best = match_fsm(ere->fsm, eremf, str, i, len, ere->s, mi)) != NULL)
        {
            *pos_match = i;
            *len_match = (str_best - str) - i;
            return TRUE;
        }
    }
    return FALSE;
}

/*
 *@@ rxpMatch_bwd:
 *      match backwards within a string not passing a
 *      specified end position.
 *
 *      If a match, return TRUE, and also return pos and
 *      len of the match. We need to consider matches from
 *      the beginning of the line. We want the one which
 *      ends up in the rightmost position. Of those which
 *      end up equally far right, we want the one which
 *      extends the furthest (or shortest if EREMF_SHORTEST)
 *      left. See how we get this as a side effect of the
 *      loop ordering and the '>= + delta' test.
 *
 *      This may not look as efficient as scanning the string
 *      backwards, but note that this would require a reversed
 *      ERE too, and we can't reverse EREs as they may
 *      contain backreferences.
 *
 *      If EREMF_SHORTEST is passed with eremf, the code looks for
 *      the shortest match, instead of the longest match.
 *
 *      If EREMF_ANY is passed with eremf, the code doesn't try to
 *      find the longest (or shortest) match, it will return with the
 *      first match it finds (which could be of any length).
 *      This can speed up matching.
 */

BOOLEAN rxpMatch_bwd(const ERE *ere,        // in: compiled ERE (from rxpCompile)
                     int eremf,             // in: EREMF_* flags
                     const char *str,       // in: string to test
                     int pos,               // in: start position
                     int *pos_match,        // out: position of match
                     int *len_match,        // out: length of match
                     ERE_MATCHINFO * mi)    // out: match info (for rxpSubsWith)
{
    int i;
    int delta = (eremf & EREMF_SHORTEST) ? 0 : 1;
    const char *rightmost = NULL;
    ERE_MATCHINFO mi2;

    for (i = 0; i <= pos - ere->shortest_match; i++)
    {
        const char *str_best;

        if ((str_best = match_fsm(ere->fsm, eremf, str, i, pos, ere->s, &mi2)) != NULL)
        {
            if (rightmost == NULL ||
                str_best >= rightmost + delta)
            {
                *pos_match = i;
                *len_match = (str_best - str) - i;
                rightmost = str_best;
                if (mi != NULL)
                {
                    mi->n_spans = mi2.n_spans;
                    for (i = 0; i < mi->n_spans; i++)
                        mi->spans[i] = mi2.spans[i];
                }
            }
        }
    }
    return rightmost != NULL;
}

/*
 *@@ rxpFree:
 *      frees all resources allocated by rxpCompile.
 */

void rxpFree(ERE * ere)
{
    if (ere)
    {
        delete_match(ere->match);
        delete_fsm(ere->fsm);
        free(ere);
    }
}

/*
 *@@ rxpSubsWith:
 *      perform a substitution based upon an earlier found match.
 *      This allows for implementing a "find and replace" function.
 */

BOOLEAN rxpSubsWith(const char *str,    // in: original string searched (same as str given to rxpMatch_fwd)
                    int pos,            // in: span of the entire match (pos_match from rxpMatch_fwd)
                    int len,            // in: span of the entire match (len_match from rxpMatch_fwd)
                    ERE_MATCHINFO *mi,  // in: details of match sub-spans (as from rxpMatch_fwd)
                    const char *with,   // in: replacement string with \1 etc.
                    char *out,          // out: buffer for string substitutions
                    int len_out,        // in: sizeof *out
                    int *rc)            // out: error, if FALSE returned
{
    int i = 0;
    int j;

    memcpy(out, str, pos);
    i += pos;
    while (*with != '\0')
    {
        const char *rep;
        int len_rep;

        if (*with != '\\')
        {
            rep = with++;
            len_rep = 1;
        }
        else
        {
            ++with;
            if (*with >= '1' && *with <= '9')
            {
                int span = *with - '1';

                ++with;
                if (span >= mi->n_spans)
                {
                    *rc = EREE_BAD_BACKREF;
                    return FALSE;
                }
                rep = str + mi->spans[span].pos;
                len_rep = mi->spans[span].len;
            }
            else if (*with != '\0')
            {
                rep = with++;
                len_rep = 1;
            }
            else
            {
                *rc = EREE_BAD_BACKSLASH;
                return FALSE;
            }
        }
        if (i + len_rep > len_out)
        {
            *rc = EREE_SUBS_LEN;
            return FALSE;
        }
        memcpy(out + i, rep, len_rep);
        i += len_rep;
    }
    j = pos + len + strlen(str + pos + len);
    if (i + j > len_out)
    {
        *rc = EREE_SUBS_LEN;
        return FALSE;
    }
    memcpy(out + i, str + pos + len, j);
    i += j;
    out[i] = '\0';
    return TRUE;
}

#ifdef __TESTCASE__

int main(int argc, char *argv[])
{
    ERE             *ere;
    ERE_MATCHINFO   mi;
    int rc;

    const char *pcsz, *pcszEre;

    if (argc != 3)
    {
        printf("Usage: regexp <teststring> <ere>\n");
        exit(1);
    }

    pcsz = argv[1];
    pcszEre = argv[2];

    printf("matching \"%s\" against \"%s\"\n",
           pcsz,
           pcszEre);
    fflush(stdout);

    if (!(ere = rxpCompile(pcszEre,
                           0,
                           &rc)))
    {
        printf("Error %d in rxpCompile: %s\n", rc, rxpError(rc));
        exit(rc);
    }

    {
        int pos, length;
        rc = rxpMatch_fwd(ere,
                          0,
                          pcsz,
                          0,
                          &pos,
                          &length,
                          &mi);

        if (rc == 0)
            printf("no match\n");
        else
            printf("found at pos %d, length %d\n", pos, length);
    }

    return 0;
}

#endif
