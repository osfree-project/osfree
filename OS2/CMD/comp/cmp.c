/*
 *
 * Stupid compare tool.
 *
 * Copyright (c) 2001 knut st. osmundsen (knut.stange.osmundsen@mynd.no)
 *
 * GPL.
 *
 */


/*******************************************************************************
*   Defined Constants And Macros                                               *
*******************************************************************************/
#define INCL_BASE


/*******************************************************************************
*   Header Files                                                               *
*******************************************************************************/
#include <osfree.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>


/*******************************************************************************
*   Global Variables                                                           *
*******************************************************************************/
int     cErrors = 0;
int     cSuccess = 0;
int     cTotalFiles = 0;


/*
 * Options.
 */
static struct _OptionsCmp
{
    BOOL        fQuiet;
} options =
{
    FALSE
};


/*******************************************************************************
*   Internal Functions                                                         *
*******************************************************************************/
static void syntax(void);
static int  ProcessFiles(const char *pszFilePattern1, const char *pszFilePattern2);
static int  ProcessFile(const char *pszFile1, unsigned long cbFile1,  const char *pszFilePattern);


int main(int argc, char **argv)
{
    char *  psz;
    int     argi;
    int     argi1st = -1;
    int     i;
    APIRET  rc;


    /*
     * Parse commandline
     */
    if (argc == 1)
    {
        syntax();
        return -1;
    }

    for (argi = 1; argi < argc; argi++)
    {
        if (argv[argi][0] == '-')
        {
            switch (argv[argi][1])
            {
                case 'h':
                case 'H':
                case '?':
                case '-':
                    syntax();
                    return 0;


                case 's':
                case 'S':
                case 'q':
                case 'Q':
                    options.fQuiet = argv[argi][2] != '-';
                    break;

                default:
                    fprintf(stderr, "Unknown options '%s'.\n", argv[argi]);
                    return -1;
            }
        }
        else if (argi1st == -1)
        {
            argi1st = argi;
        }
        else
        {
            /*
             * Bind files.
             */
            rc = ProcessFiles(argv[argi1st], argv[argi]);
            if (rc)
            {
                fprintf(stderr, "Failed to process files '%s' / '%s'. SYS%04d\n",
                        argv[argi1st], argv[argi], rc);
                return -4;
            }
            argi1st = -1;
        }
    }


    /*
     * Check if no second filepattern.
     */
    if (argi1st != -1)
    {
        fprintf(stderr, "Must specify an even number of filepatterns.");
        return -4;
    }

    return cErrors;
}



void syntax(void)
{
    fputs(
         "Cmp! quick and dirty\n"
         "--------------------------------\n"
         "Syntax: cmp.exe <options> <files..> [[options] <files> ...]\n"
         "  options:\n"
         "   -s<[+]|->      Silent execution.       default: -s-\n"
         "   -q<[+]|->      Same as -s.\n"
         "   -h|?           Help screen (this).\n"
         "\n"
         "\n"
         "This program is published according to the GNU GENERAL PUBLIC LICENSE V2.\n"
         "Copyright (c) 2002 knut st. osmundsen\n",
         stdout);
}


/**
 * Process a file pattern.
 * @returns 0 on success.
 *          error code on fatal failure.
 * @param   pszFilePattern  File pattern to search.
 * @author  knut st. osmundsen (knut.stange.osmundsen@mynd.no)
 */
int ProcessFiles(const char *pszFilePattern1, const char *pszFilePattern2)
{
    int             cPrevTotalFiles = cTotalFiles;
    char            szFilename[CCHMAXPATH];
    char *          pszFilename;
    const char *    pszPath;
    const char *    pszPath1;
    const char *    pszPath2;
    APIRET          rc;
    ULONG           cFiles = 500;
    HDIR            hDir = HDIR_CREATE;
    char            achfindbuf[1024*16];
    PFILEFINDBUF3   pfindbuf = (PFILEFINDBUF3)&achfindbuf[0];


    /*
     * Extract path from pattern.
     */
    pszPath1 = strrchr(pszFilePattern1, '\\');
    pszPath2 = strrchr(pszFilePattern1, '/');
    if (pszPath1 != NULL && pszPath1 > pszPath2)
        pszPath = pszPath1+1;
    else if (pszPath2 != NULL && pszPath2 > pszPath1)
        pszPath = pszPath2+1;
    else if (pszFilePattern1[1] == ':')
        pszPath = &pszFilePattern1[2];
    else
        pszPath = &pszFilePattern1[0];
    memcpy(szFilename, pszFilePattern1, pszPath - pszFilePattern1);
    pszFilename = &szFilename[pszPath - pszFilePattern1];
    *pszFilename = '\0';

    /*
     * First search.
     */
    rc = DosFindFirst((PSZ)pszFilePattern1,
                           &hDir,
                           0,
                           pfindbuf,
                           sizeof(achfindbuf),
                           &cFiles,
                           FIL_STANDARD);
    while (rc == NO_ERROR && cFiles > 0)
    {
        PFILEFINDBUF3   pCur = pfindbuf;

        /*
         * Loop thru the result.
         */
        while (pCur && cFiles--)
        {
            /*
             * Make filename.
             * Process the file.
             * Maintain statistics.
             */
            strcpy(pszFilename, pCur->achName);
            rc = ProcessFile(szFilename, pCur->cbFile,  pszFilePattern2);
            if (rc == 0)
                cSuccess++;
            else
                cErrors++;
            cTotalFiles++;

            /*
             * Next file.
             */
            pCur = pCur->oNextEntryOffset == 0
                    ? NULL
                    : (PFILEFINDBUF3)((char*)pCur + pCur->oNextEntryOffset);
        }

        /*
         * Next chunk.
         */
        cFiles = 500;
        rc = DosFindNext(hDir, pfindbuf, sizeof(achfindbuf), &cFiles);
    }

    /*
     * Terminate find.
     */
    DosFindClose(hDir);

    /*
     * Warning about no files and return.
     */
    if (cTotalFiles == cPrevTotalFiles)
        fprintf(stderr, "No files found matching the pattern: %s\n", pszFilePattern1);
    return rc == ERROR_NO_MORE_FILES ? NO_ERROR : rc;
}


/**
 * Processes a single file.
 * @returns 0 if successfully processed.
 *          -2 if wrong format.
 *          other error code on failure.
 * @param   pszFile         Name of the file to process.
 * @param   pszFilePattern  File pattern to match pszFile with.
 * @author  knut st. osmundsen (knut.stange.osmundsen@mynd.no)
 */
int ProcessFile(const char *pszFile1, unsigned long cbFile1, const char *pszFilePattern)
{
    int             rc = 0;
    FILESTATUS3     fst3;
    int             cchPattern = strlen(pszFilePattern);
    char            szFile2[CCHMAXPATH];
    char            szPattern[CCHMAXPATH];
    const char *    pszFileName;
    const char *    pszPatternName;


    /*
     * Correct the pattern.
     */
    memcpy(szPattern, pszFilePattern, cchPattern+1);
    if (szPattern[cchPattern - 1] == '\\' || szPattern[cchPattern - 1] == '/')
        szPattern[cchPattern++] = '*';
    else
    {
        if (!DosQueryPathInfo((PSZ)pszFilePattern, FIL_STANDARD, &fst3, sizeof(fst3)))
        {
            if (fst3.attrFile & FILE_DIRECTORY)
            {
                szPattern[cchPattern++] = '\\';
                szPattern[cchPattern++] = '*';
            }
        }
    }
    szPattern[cchPattern] = '\0';


    /*
     * Get the filename part of pszFile1.
     */
    pszFileName = strrchr(pszFile1, '\\');
    if (!pszFileName)
        pszFileName = strrchr(pszFile1, '/');
    if (!pszFileName)
        pszFileName = strchr(pszFile1, ':');
    if (!pszFileName)
        pszFileName = pszFile1 - 1;
    pszFileName++;


    /*
     * Get the filename portion of szPattern.
     */
    pszPatternName = strrchr(szPattern, '\\');
    if (!pszPatternName)
        pszPatternName = strrchr(szPattern, '/');
    if (!pszPatternName)
        pszPatternName = strchr(szPattern, ':');
    if (!pszPatternName)
        pszPatternName = szPattern - 1;
    pszPatternName++;


    /*
     * Edit the input name according to the pattern.
     */
    rc = DosEditName(1, (PSZ)pszFileName, (PSZ)pszPatternName, szFile2, sizeof(szFile2));
    if (!rc)
    {
        /*
         * Make fully qualified name.
         */
        if (pszPatternName == szPattern)
        {
            memmove(szFile2 + (int)pszFileName - (int)pszFile1, szFile2, strlen(szFile2) + 1);
            memcpy(szFile2, pszFile1, (int)pszFileName - (int)pszFile1);
        }
        else
        {
            memmove(szFile2 + (int)pszPatternName - (int)szPattern, szFile2, strlen(szFile2) + 1);
            memcpy(szFile2, szPattern, (int)pszPatternName - (int)szPattern);
        }

        /*
         * Check for existance of target.
         */
        rc = DosQueryPathInfo(szFile2, FIL_STANDARD, &fst3, sizeof(fst3));
        if (rc == NO_ERROR && !(fst3.attrFile & FILE_DIRECTORY))
        {
            ULONG   ulAction = 0;
            ULONG   cbFile2 = fst3.cbFile;
            HFILE   hFile1 = NULLHANDLE;
            HFILE   hFile2 = NULLHANDLE;

            rc = DosOpen(szFile2, &hFile2, &ulAction, 0, FILE_NORMAL,
                         OPEN_ACTION_FAIL_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS,
                         OPEN_FLAGS_SEQUENTIAL | OPEN_ACCESS_READONLY | OPEN_SHARE_DENYNONE,
                         NULL);
            if (!rc)
            {
                ulAction = 0;
                rc = DosOpen((PSZ)pszFile1, &hFile1, &ulAction, 0, FILE_NORMAL,
                             OPEN_ACTION_FAIL_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS,
                             OPEN_FLAGS_SEQUENTIAL | OPEN_ACCESS_READONLY | OPEN_SHARE_DENYNONE,
                             NULL);
                if (!rc)
                {
                    unsigned long   cMismatches = 0;
                    unsigned long   offRead = 0;

                    while (cbFile1 > 0 && cbFile2 > 0)
                    {
                        static char szBuffer1[0x10000];
                        static char szBuffer2[0x10000];
                        ULONG       cbRead1;
                        ULONG       cbRead2;
                        APIRET      rc1;
                        APIRET      rc2;

                        rc1 = DosRead(hFile1, &szBuffer1[0], min(sizeof(szBuffer1), cbFile1), &cbRead1);
                        cbFile1 -= cbRead1;

                        rc2 = DosRead(hFile2, &szBuffer2[0], min(sizeof(szBuffer2), cbFile2), &cbRead2);
                        cbFile2 -= cbRead2;

                        if (!rc1 && !rc2)
                        {
                            int cbCmp = min(cbRead1, cbRead2);

                            /*
                             * Check if equal using fast compare.
                             */
                            if (memcmp(&szBuffer1[0], &szBuffer2[0], cbCmp))
                            {
                                int i;
                                /*
                                 * Wasn't equal. Display mismatches using slow byte by byte compare.
                                 */
                                for (i = 0; i < cbCmp; i++)
                                {
                                    if (szBuffer1[i] != szBuffer2[i])
                                    {
                                        if (!options.fQuiet)
                                        {
                                            if (cMismatches++ == 0)
                                                printf("Mismatch comparing '%s' and '%s'.\n", pszFile1, szFile2);
                                            printf("at 0x%08x 0x%02x (%c) != 0x%02 (%c)\n",
                                                   offRead + i,
                                                   szBuffer1[i], isprint(szBuffer1[i]) ? szBuffer1[i] : ' ',
                                                   szBuffer2[i], isprint(szBuffer2[i]) ? szBuffer2[i] : ' ');
                                        }
                                        else
                                            cMismatches++;
                                    }
                                }
                            }

                            /*
                             * Check if last read and size is different.
                             */
                            if (cbRead1 != cbRead2)
                            {
                                if (!options.fQuiet)
                                {
                                    if (cMismatches == 0)
                                        printf("Mismatch comparing '%s' and '%s'.\n", pszFile1, szFile2);
                                    printf("Length differs. %d != %d\n",
                                           offRead + cbRead1 + cbFile1,
                                           offRead + cbRead2 + cbFile2);
                                    printf("%d differences so far\n", cMismatches);
                                }
                                rc = 1;
                                break;
                            }

                            /*
                             * Last read?
                             */
                            if (!cbFile1 || !cbFile2)
                            {
                                if (!options.fQuiet)
                                {
                                    if (cMismatches > 0)
                                    {
                                        if (cMismatches == 0)
                                            printf("Mismatch comparing '%s' and '%s'.\n", pszFile1, szFile2);
                                        printf("%d differences\n", cMismatches);
                                    }
                                    else
                                        printf("Successfull. '%s' matches '%s'.\n", pszFile1, szFile2);
                                }
                            }
                        }
                        else
                        {
                            /*
                             * Read Error.
                             */
                            if (rc1)
                                printf("Error: failed to read from '%s'. rc=%d\n", pszFile1, rc1);
                            if (rc2)
                                printf("Error: failed to read from '%s'. rc=%d\n", szFile2, rc2);
                            rc = rc1 ? rc1 : rc2;
                            break;
                        }
                    }

                    DosClose(hFile1);

                    /*
                     * Return number of errors.
                     */
                    if (cMismatches && !rc)
                        rc = cMismatches;
                }
                else
                    fprintf(stderr, "(SYS%04d): Could not open file '%s'.\n", rc, pszFile1);

                DosClose(hFile2);
            }
            else
                fprintf(stderr, "(SYS%04d): Could not open file '%s'.\n", rc, szFile2);
        }
        else
        {
            if (rc)
                fprintf(stderr, "(SYS%04d): File '%s' was not found.\n", rc, szFile2);
            else
                fprintf(stderr, "Error: Can't compare a file with a directory (%s).\n", szFile2);
        }
    }
    else
        fprintf(stderr, "Internal error DosEditName(1, '%s','%s',..) -> rc=%d\n", pszFileName, pszPatternName, rc);

    return rc;
}

