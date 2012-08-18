#include <ctype.h>

#define NULL '\0'

void DosNameConversion(char * pszName);

void DosNameConversion(char * pszName)
{
    char separator;
    char replacement;
    char * tempName;

    if (pszName == NULL) {
        return;
    }

    /* set the delimiter to convert */
    separator = '\\';

    /* set the replacement character */
    replacement = '/';

    /* now convert the characters */
    tempName = pszName;
    while (*tempName != '\0') {
        if (*tempName == separator) {
            *tempName = replacement;
        }
        tempName++;
    }

    /* covert the name to lower case if necessary */
    tempName = pszName;
    while (*tempName)
    {
            *tempName = (char)tolower(*tempName);
            tempName++;
    }
}

