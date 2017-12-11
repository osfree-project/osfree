#include <string.h>
#include <l4/os3/token.h>

static char *nxtToken = 0;              /* pointer to previous scanned string */
static char ch;                                         /* previous token delimiter */

char *StrTokenize(char *str, const char * const token)
{

        if(!str) {
                if((str = nxtToken) == 0                /* nothing to do */
                 || (*str++ = ch) == 0)               /* end of string reached */
                        return( nxtToken = 0);
        }

        if(!token || !*token)                   /* assume all the string */
                nxtToken = 0;
        else {
                nxtToken = str - 1;
                while(!strchr(token, *++nxtToken));
                ch = *nxtToken;
                *nxtToken = 0;
        }

        return( str);
}

void StrTokSave(STR_SAVED_TOKENS *st)
{

        if(st) {
                st->str_next_saved_tokens = nxtToken;
                st->str_ch_saved_tokens = ch;
        }

}

void StrTokRestore(STR_SAVED_TOKENS *st)
{

        if(st) {
                nxtToken = st->str_next_saved_tokens;
                ch = st->str_ch_saved_tokens;
        }

}
