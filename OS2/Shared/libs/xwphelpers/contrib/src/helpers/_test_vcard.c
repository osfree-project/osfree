#define OS2EMX_PLAIN_CHAR
    // this is needed for "os2emx.h"; if this is defined,
    // emx will define PSZ as _signed_ char, otherwise
    // as unsigned char

#define INCL_DOSNLS
#define INCL_DOSERRORS
#define INCL_WINCOUNTRY
#include <os2.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "setup.h"                      // code generation and debugging options

#include "helpers\dosh.h"
#include "helpers\linklist.h"
#include "helpers\nls.h"
#include "helpers\standards.h"
#include "helpers\stringh.h"
#include "helpers\xstring.h"

#define INCLUDE_VCARD_ALL
#include "helpers\vcard.h"

#pragma hdrstop


VOID DumpList(ULONG ulLevel,
              PLINKLIST pll)
{
    PLISTNODE pNode;
    ULONG ul;

    for (pNode = lstQueryFirstNode(pll);
         pNode;
         pNode = pNode->pNext)
    {
        PVCFPROPERTY pProp = (PVCFPROPERTY)pNode->pItemData;

        {
            CHAR szIndent[100];
            ULONG x;
            if (ulLevel)
                memset(szIndent, ' ', ulLevel * 3);
            szIndent[ulLevel * 3] = '\0';

            printf("%sproperty \"%s\"\n",
                    szIndent,
                    pProp->strProperty.psz);

            if (pProp->pllSubList)
                DumpList(ulLevel + 1,
                         pProp->pllSubList);
            else
            {
                ULONG ul2, c;
                PXSTRING paStrings;

                c = pProp->cParameters;
                paStrings = pProp->pastrParameters;

                for (ul2 = 0;
                     ul2 < c;
                     ul2++)
                {
                    printf("%s  param %d: \"%s\"\n",
                            szIndent,
                            ul2,
                            paStrings[ul2].psz);

                }

                c = pProp->cValues;
                paStrings = pProp->pastrValues;

                for (ul2 = 0;
                     ul2 < c;
                     ul2++)
                {
                    printf("%s  value %d: \"%s\"\n",
                            szIndent,
                            ul2,
                            paStrings[ul2].psz);

                }
            }
        }
    }
}

int main (int argc, char *argv[])
{
    if (argc <= 1)
    {
        printf("vcard "__DATE__"\n");
        printf("Usage: vcard <filename>\n");
    }
    else
    {
        PVCARD pv;
        APIRET arc;
        ULONG ul;
        if (!(arc = vcfRead(argv[1], &pv)))
        {
            DumpList(0, pv->pll);

            printf("Formatted name: %s\n",
                    pv->pcszFormattedName);

            for (ul = 0;
                 ul < 5;
                 ul++)
                printf("Name field %d: %s\n",
                        ul,
                        pv->apcszName[ul]);

            vcfFree(&pv);
        }
        else
            printf("vcard: error %d\n", arc);

    }

    return 0;
}


