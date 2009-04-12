/* $Id: Files.c,v 1.1 2000/04/21 10:57:53 ktk Exp $
 *
 * Creates a lot of files.
 *
 * Copyright (c) 2000 knut st. osmundsen (knut.stange.osmundsen@pmsc.no)
 *
 */

#define INCL_BASE
#include <os2.h>
#include <stdio.h>
#include <stdlib.h>


/*******************************************************************************
*   Internal Functions                                                         *
*******************************************************************************/
void syntax(void);



int main(int argc, char**argv)
{
    /*
     * Argument values set to defaults.
     */
    unsigned long   cFiles  = 1024;
    unsigned long   cbMin   = 0;
    unsigned long   cbMax   = 1024*1024*2;

    /*
     * Other variables.
     */
    int             argi;
    unsigned long   cb;
    int             i;
    APIRET          rc;


    /*
     * Parse arguments:
     */
    argi = 1;
    while (argi < argc)
    {
        char    *psz;
        char    *psz2;
        unsigned long ul;
        int     iBase;

        /*
         * Convert argument to number
         *      Check number base.
         *      Try convert it.
         *      Check that the entire string was a number.
         */
        if (argv[argi][0] == '0')
            iBase = argv[argi][1] == 'x' || argv[argi][1] == 'X' ? 16 : 8;
        else
            iBase = 10;
        ul = strtoul(argv[argi], &psz2, iBase);
        if (psz2 == NULL || psz2 == argv[argi] || *psz2 != '\0')
        {
            syntax();
            printf("error: argument %d is not a number\n", argi);
            return -1;
        }

        switch (argi)
        {
            case 1: cFiles = ul; break;
            case 2: cbMin = ul; break;
            case 3: cbMax = ul; break;
            default:
                syntax();
                printf("error: too many arguments\n");
                break;
        }

        argi++;
    }


    /*
     * Switch cbMin and cbMax if necessary
     */
    if (cbMin > cbMax)
    {
        cb = cbMin;
        cbMin = cbMax;
        cbMax = cb;
    }


    /*
     * Do the real work. We're gona generate <cFiles> number of files in
     * the current directory. The files are initiated at a random size
     * in the range cbMin to cbMax. The names are not random but a basename
     * with an increasing number and the size as extention.
     * (It don't apply with the 8.3 DOS format.)
     */
    printf("creating %d files\n", cFiles);
    for (i = 0; i < cFiles; i++)
    {
        char szFilename[50];
        ULONG   ulAction = 0;
        HFILE   hFile;

        /*
         * Calc filesize.
         */
        if (cbMin != cbMax)
        {
            cb = rand();
            cb = (unsigned long)(cbMin + ((double)(cbMax - cbMin) * ((double)cb / (double)RAND_MAX)));
        }
        else
            cb = cbMin;

        /*
         * Make filename
         */
        sprintf(szFilename, "test.%05d.%08d", i, cb);

        /*
         * Try create the file.
         */
        rc = DosOpen(szFilename,
                     &hFile,
                     &ulAction,
                     cb,
                     FILE_NORMAL,
                     OPEN_ACTION_CREATE_IF_NEW | OPEN_ACTION_REPLACE_IF_EXISTS,
                     OPEN_SHARE_DENYREADWRITE | OPEN_ACCESS_READWRITE,
                     NULL);

        if (rc != NO_ERROR)
        {
            printf("Failed to create file %s, rc=%d\n", szFilename, rc);
            break;
        }
        else
            DosClose(hFile);
    }

    return rc;
}


void syntax(void)
{
    printf(
        "Files.exe [number of files] [min files size] [max files size]\n"
        );
}
