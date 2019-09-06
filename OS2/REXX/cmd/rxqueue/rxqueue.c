#define  INCL_BASE
#include <os2.h>

#include <rexxsaa.h>

#include <string.h>
#include <stdlib.h>

#define RXQUEUE_CLEAR -2
#define RXQUEUE_UNSET -1
#define RXQUEUE_FIFO   0
#define RXQUEUE_LIFO   1

APIRET APIENTRY RexxQueryQueue( PSZ QueueName, ULONG* Count);
APIRET APIENTRY RexxAddQueue( PSZ QueueName, PRXSTRING EntryData, ULONG AddFlag);
APIRET APIENTRY RexxPullQueue( PSZ QueueName, PRXSTRING DataBuf, PDATETIME TimeStamp, ULONG WaitFlag);

char Buf[0x1000];
char Buf2[0x100];

UCHAR chRead = 0;

ULONG fFinished = 0;

LONG lAddFlag = RXQUEUE_UNSET;

void showMsg(ULONG ulMsgNum, PSZ pszTableName)
{
    char buf[256];
    ULONG cbMsg;
    APIRET rc = NO_ERROR;
    PSZ pszFile = "REX.MSG";
    ULONG cTable = 0;
    PSZ pszTable;
    ULONG cMsgNum;

    memset(buf, 0, sizeof(buf));
    
    switch (ulMsgNum)
    {
        case 0:
            pszFile = "OSO001.MSG";
            cMsgNum = 0x3eb;
            break;

        case 1:
        case 3:
        case 7:
        case 8:
            pszFile = "OSO001.MSG";
            cMsgNum = 0x57;
            break;
            
        case 2:
            cMsgNum = 0x78;
            break;
            

        case 4:
            cMsgNum = 0x79;
            break;

        case 5:
            cMsgNum = 0x7a;
            if (! pszTableName)
            {
                pszTable = "";
            }
            else
            {
                pszTable = pszTableName;
            }
            cTable = 1;
            break;

        case 6:
            cMsgNum = 0x7b;
            break;

        case 9:
            cMsgNum = 0x7c;
            if (! pszTableName)
            {
                pszTable = "";
            }
            else
            {
                pszTable = pszTableName;
            }
            cTable = 1;
            break;

        default:
            if (ulMsgNum == 0x3e8)
            {
                cMsgNum = 0x77;
            }
            else
            {
                pszFile = "OSO001.MSG";
                cMsgNum = 0x57;
            }
    }

    if ( DosGetMessage(&pszTable,
                       cTable,
                       buf,
                       0x100,
                       cMsgNum,
                       pszFile,
                       &cbMsg) )
    {
        exit(-6);
    }

    DosPutMessage(2, cbMsg, buf);
    
    exit(rc);
}

int readData(PSZ pBuf, ULONG cbBuf, ULONG *cbRead)
{
    char c;
    ULONG len;
    ULONG cbActual;

    if (fFinished)
    {
        return 1;
    }

    len = 0;

    if (chRead)
    {
        *pBuf++ = chRead;
        len = 1;
        chRead = 0;
    }

    while (! DosRead(0, &c, 1, &cbActual) )
    {
        if (! cbActual)
        {
            *cbRead = len;

            if (! len)
            {
                return 1;
            }

            fFinished = 1;
            return 0;
        }

        if (c == '\r')
        {
            *cbRead = len;

            if (! DosRead(0, &c, 1, &cbActual) )
            {
                if (cbActual)
                {
                    if (c != '\n')
                    {
                        chRead = c;
                    }
                }
            }

            return 0;
        }

        if (c == '\n')
        {
           *cbRead = len; 
           return 0;
        }

        if (c == '')
        {
            *cbRead = len;
            fFinished = 1;

            if (len)
            {
                return 1;
            }
        }

        if (cbBuf > len)
        {
            *pBuf++ = c;
            len++;
        }
    }

    if (len)
    {
        *cbRead = len;
        fFinished = 1;
        return 0;
    }

    return 1;
}

char *parseArgs(int argc, char **argv, char *pszQueName)
{
    int  ind = 1;
    char *arg;
    
    do
    {
        if (argc == 1)
        {
            break;
        }

        arg = argv[ind];
    
        if (*arg == '/' || *arg == '-')
        {
            *arg = '/';
        
            if (! stricmp(arg, "/FIFO") )
            {
                if (lAddFlag == RXQUEUE_UNSET)
                {
                    lAddFlag = RXQUEUE_FIFO;
                }
            }
            else if (! stricmp(arg, "/LIFO") )
            {
                if (lAddFlag == RXQUEUE_UNSET)
                {
                    lAddFlag = RXQUEUE_LIFO;
                }
            }
            else if (! stricmp(arg, "/CLEAR") )
            {
                if (lAddFlag == RXQUEUE_UNSET)
                {
                    lAddFlag = RXQUEUE_CLEAR;
                }
            }
            else
            {
                showMsg(0, pszQueName);
            }
        }
        else
        {
            if (pszQueName)
            {
                showMsg(0, pszQueName);
            }
            else
            {
                pszQueName = arg;
            }
        }

        ind++;
    } while (ind < argc);

    return pszQueName;
}

int main(int argc, char **argv)
{
    DATETIME timestamp;
    ULONG count; 
    RXSTRING str;
    ULONG cbLen;
    char *pszQueName = NULL;
    APIRET rc;

    memset(Buf, 0, sizeof(Buf));
    memset(Buf2, 0, sizeof(Buf2));

    pszQueName = parseArgs(argc, argv, pszQueName);

    if (lAddFlag == RXQUEUE_UNSET)
    {
        lAddFlag = RXQUEUE_FIFO;
    }

    if (! pszQueName)
    {
        if ( DosScanEnv("RXQUEUE", (PSZ *)&pszQueName) )
        {
            if (! pszQueName)
            {
                pszQueName = "SESSION";
            }
        }
    }

    if ( rc = RexxQueryQueue(pszQueName, &count) )
    {
        showMsg(rc, pszQueName);
    }

    switch (lAddFlag)
    {
        case RXQUEUE_CLEAR:
            str.strlength = 0;
            str.strptr = NULL;

            while (! RexxPullQueue(pszQueName, &str, &timestamp, 0) )
            {
                if (str.strlength)
                {
                    DosFreeMem(str.strptr);
                }

                str.strlength = 0;
                str.strptr = NULL;
            }
            break;

        case RXQUEUE_FIFO:
        case RXQUEUE_LIFO:
            while (! readData(Buf, 0x1000, &cbLen) )
            {
                str.strlength = cbLen;
                str.strptr = Buf;

                if ( ( rc = RexxAddQueue(pszQueName, &str, lAddFlag) ) )
                {
                    showMsg(rc, pszQueName);
                }
            }
    }

    return 0;
}
