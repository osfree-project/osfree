// KPARSE.C - Keystroke parsing routines for everything except 4DOS
//   Copyright 1992 - 1997, JP Software Inc., All Rights Reserved

#include "product.h"

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "4all.h"

#define KEYPARSE 1
#include "inifile.h"

#include "message.h"

static int FindNext(char *pszOutput, char *pszInput, char *pszSkipDelims,
                    char *pszEndDelims, int *nStart);


// Parse value for key directive
int keyparse(char *keystr, int klen)
{
        int key;
        char tokstr[11], tokbuf[11];
        int knum, prefix = -1, tl, tstart;

        sprintf(tokstr, FMT_FAR_PREC_STR, ((klen > 10) ? 10 : klen), keystr);

        // Get token, quit if nothing there
        if (FindNext(tokbuf, tokstr, " -\t", "^ -;\t", &tstart) == 0)
                return -1;

        // handle "@nn" -- return nn + 256 for extended keystroke
        if (tokbuf[0] == '@')
                return (0x100 + atoi(&tokbuf[1]));

        // handle numeric without '@'
        if (isdigit(tokbuf[0]))
                return atoi(tokbuf);

        // look for Alt / Ctrl / Shift prefix (must be more than 1 char, or
        // 1 char with a '-' after it)
        if (((tl = strlen(tokbuf)) > 1) || (tokstr[tstart+1] == '-')) {

                // check prefix; complain if no match & single character; if no
                // match and multiple characters set prefix to 0
                if (toklist(tokbuf, &KeyPrefixes, &prefix) != 1) {
                        if (tl == 1)
                                return -1;
                        prefix = 0;
                }

                // if it matches look for something else, complain if not there
                else if (FindNext(tokbuf, tokstr + tstart + strlen(tokbuf), " -\t", "^ -;\t", NULL) == 0)
                        return -1;

                else
                        tl = strlen(tokbuf);
        }

        // If single character return ASCII
        if (tl == 1) {

                key = (int)_ctoupper(tokbuf[0]);

                if (isalpha(key)) {

                        switch (prefix) {
                        case 1:     // Alt
                                key = 0x100 + (int)ALT_ALPHA_KEYS[key - 'A'];
                                break;
                        case 2:     // Ctrl -- adjust
                                key -= 0x40;
                                break;
                        case 3:     // Shift -- error
                                key = -1;
                        }

                } else if (isdigit(key)) {

                        switch (prefix) {
                        case 0:     // No prefix -- numeric key value
                                key = atoi(tokbuf);
                                break;
                        case 1:     // Alt -- do table lookup
                                key = 0x100 + (int)ALT_DIGIT_KEYS[key - '0'];
                                break;
                        case 2:     // Ctrl -- error
                        case 3:     // Shift -- error
                                key = -1;
                        }
                }

                return key;
        }

        // We must have a non-printing key name, see if it's a function key
        // (no other key names start with 'F')
        if (_ctoupper(tokbuf[0]) == 'F')
                return ((((key = atoi(&tokbuf[1]) - 1) < 0) || (key > 12)) ? -1 : 256 + (((key <= 9) ? (int)KeyPrefixList[prefix].F1Pref : (int)KeyPrefixList[prefix].F11Pref) + key) );

        // Check for a valid non-printing key name
        if (toklist(tokbuf, &KeyNames, &knum) != 1)
                return -1;

        // Get standard value for this key
        key = KeyNameList[knum].NPStd;

        // handle prefix (special handling for some keys)
        if (prefix) {

                // don't allow Alt-Tab, handle Ctrl-Tab as special case
                // (Shift-Tab drops through to load NPSecond value)
                if (key == K_Tab) {
                        if (prefix == 1)
                                return -1;
         else if (prefix == 2)
            return K_CtlTab;

                // don't allow Shift-Backspace, handle Alt-Backspace as special case
                // (Ctrl-Backspace drops through to load NPSecond value)
                } else if (key == K_Bksp) {
                        if (prefix == 3)
                                return -1;
                        else if (prefix == 1)
                                return K_AltBS;

                // For all other keys the only valid prefix is Ctrl
                } else if (prefix != 2)
                        return -1;

                // It is a real prefixed key, get value with prefix
                key = KeyNameList[knum].NPSecond;
        }

        return key;
}


int toklist(char *token, TL_HEADER *tlist, int *index)
{
        int i;
        union {
                char **pptr;
                char *cptr;
        } eptrs;

        eptrs.pptr = tlist->elist;

        for (i = 0; (i < tlist->num_entries); i++, eptrs.cptr += tlist->entry_size) {

                if (_stricmp(token, *eptrs.pptr) == 0) {
                        *index = i;
                        return 1;
                }
        }

        return 0;
}


// move to next token, copy into buffer, and return its length
static int FindNext(char *pszOutput, char *pszInput, char *pszSkipDelims,
                    char *pszEndDelims, int *nStart)
{
        int nEnd;
        int nHoldStart;


        // Find beginning of token by skipping over a set of delimiters
        nHoldStart = strspn(pszInput, pszSkipDelims);
        pszInput += nHoldStart;
        if (nStart != NULL)
                *nStart = nHoldStart;

        // Find the end of token
        nEnd = strcspn(pszInput, pszEndDelims);

        // Copy token into buffer
        memcpy(pszOutput, pszInput, nEnd);
        *(pszOutput + nEnd) = '\0';

        return(nEnd);
}  // End FindNext

