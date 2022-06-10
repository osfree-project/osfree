/*---------------------------------------------------------------------------+
|
|     (C)opyright Dennis Bareis (developed at home, in own time) 1996 - ALL RIGHTS RESERVED
|     ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
|
|     MODULE NAME:   PMPRINTF.H
|
|
|         $Author$
|       $Revision$
|           $Date$
|        $Logfile:   V:/SUEPVCS/SUPPORT/TEMPLATE.H_V  $
|
|     DESCRIPTION:
|
|         CONTACT:   If you have any problems to report or better yet
|                    suggestions, you may email me
|                    at "dbareis@ozemail.com.au"...
+---------------------------------------------------------------------------*/
#ifndef  HEADER_PMPRINTF_H
#define  HEADER_PMPRINTF_H

/*--- We need access to "stdarg.h" ------------------------------------------*/
#ifndef __stdarg_h
        #include <stdarg.h>
#endif


/*--- If C++ declare header as 'C' ------------------------------------------*/
#ifdef __cplusplus
    extern "C"
    {
#endif

/*--- Ensure we have the packing we require ---------------------------------*/
#pragma pack(1)

/*--- General definitions ---------------------------------------------------*/
#define PMPRINTF_VERSION              "96.174"
#define PMPRINTF_QUEUE_PREFIX         "\\QUEUES\\"
#define PMPRINTF_DEFAULT_QUEUE_NAME   PMPRINTF_QUEUE_PREFIX  "PRINTF32"
typedef ULONG   PMPRINTF_MODE;
        /*--- For each line send ... ----------------------------------------*/
        #define PMPRINTF_SEND_TIME          0x00000001
        #define PMPRINTF_SEND_PROC_INFO     0x00000002

        /*--- Other flags which don't effect the line sent to PMPRINTF.EXE ---*/
        #define PMPRINTF_NOISE_ON_OK_AGAIN  0x40000000
        #define PMPRINTF_NO_NOISE_ON_ERROR  0x80000000

        /*--- Combine flags for ease of use ---------------------------------*/
        #define PMPRINTF_ALL_HEADER_FIELDS_EXCEPT_TIME  (PMPRINTF_SEND_PROC_INFO)
        #define PMPRINTF_ALL_HEADER_FIELDS              (PMPRINTF_ALL_HEADER_FIELDS_EXCEPT_TIME | PMPRINTF_SEND_TIME)
        #define PMPRINTF_DEFAULT_MODE                   PMPRINTF_ALL_HEADER_FIELDS_EXCEPT_TIME

/*--- Define some per thread data (for efficiency) --------------------------*/
#define PMPRINTF_MAX_DATA_CHARS   2000    //The output from sprintf() should not produce output whose strlen() is greater than this
#define PMPRINTF_MAX_LINE_CHARS   500     //This is the maximum line length that is sent to PMPRINTF.EXE
#define PMPRINTF_MAX_HEADER_CHARS 100     //This is the maximum length of any possible header.
#define MAX_THREADS               255     //Per Process! (can't find real limit documented or in C/Toolkit headers)
typedef struct PER_THREAD
{
    /*--- The following value indicates that PMPRINTF.EXE should beep -------*/
    BOOL  Beep;

    /*--- The following is used to sprintf() data into ----------------------*/
    char  FormattedMessage[PMPRINTF_MAX_DATA_CHARS+1];

    /*--- The following is used as a line buffer, when full sent to PMPRINTF.EXE ---*/
    char  Line[PMPRINTF_MAX_LINE_CHARS+1];

    /*--- If this value is non-zero then "Line" contains "header" information ---*/
    ULONG MacroHeaderLength;

    /*--- Define a header buffer where "standard" header info is formatted ---*/
    char  LineHeader[PMPRINTF_MAX_HEADER_CHARS+1];

    /*--- Some other interesting information --------------------------------*/
    void  * Pib;
    void  * Tib;
}      PER_THREAD;


/*--- Definitions for "PMPRINTF.C" ------------------------------------------*/
PMPRINTF_MODE _System   PmPrintfModeSet(PMPRINTF_MODE NewMode);
char *        _System   PmPrintfQueueNameThisProcess(PSZ StaticPmprintfQueueName);
ULONG         _System   PmPrintf(PSZ Format, ...);
ULONG         _System   PmPrintfVa(PSZ Format, va_list args);
ULONG         _System   PmPrintfString(PSZ String);
ULONG         _System   PmPrintfDisplayInterfaceVersionInfo(void);
ULONG         _System I_PmpfSetMacroHeader(PSZ SourceModule, int LineNumber, PSZ Function);


/*--- Make sure the "__FUNCTION__" macro exists -----------------------------*/
#ifndef __FUNCTION__
        #define __FUNCTION__ "FunctionUnknown(?)"   //IBM CSET 2.01 and onwards support this
#endif


/*--- Useful Macros ---------------------------------------------------------*/
#define Pmpf(FmtAndVariableParmsInBrackets)                                  \
        do                                                                   \
        {                                                                    \
            /*--- Output a header ---------------------------------------*/  \
            I_PmpfSetMacroHeader(__FILE__, __LINE__, NULL);                  \
                                                                             \
            /*--- Output the message ------------------------------------*/  \
            PmPrintf FmtAndVariableParmsInBrackets;                          \
        }                                                                    \
        while   (0)
#define PmpfF(FmtAndVariableParmsInBrackets)                                 \
        do                                                                   \
        {                                                                    \
            /*--- Output a header ---------------------------------------*/  \
            I_PmpfSetMacroHeader(__FILE__, __LINE__, __FUNCTION__);          \
                                                                             \
            /*--- Output the message ------------------------------------*/  \
            PmPrintf FmtAndVariableParmsInBrackets;                          \
        }                                                                    \
        while   (0)


/*--- Simple to use "Here I am" macros --------------------------------------*/
#ifndef  PMPRINTF_HERE_I_AM_MSG
         #define PMPRINTF_HERE_I_AM_MSG  " DebugHereIAm()"
#endif
#define DebugHereIAm()  PmpfF((PMPRINTF_HERE_I_AM_MSG))


/*--- Macros you can conditionally include ----------------------------------*/
#ifdef  _PMPRINTF_
        /*--- The user wants to include these debug lines -------------------*/
        #define _Pmpf(FmtAndVariableParmsInBrackets)  Pmpf(FmtAndVariableParmsInBrackets)
        #define _PmpfF(FmtAndVariableParmsInBrackets) PmpfF(FmtAndVariableParmsInBrackets)
        #define _DebugHereIAm()                       DebugHereIAm()
#else
        /*--- The user does not wish to include the debug stuff -------------*/
        #define _Pmpf(FmtAndVariableParmsInBrackets)
        #define _PmpfF(FmtAndVariableParmsInBrackets)
        #define _DebugHereIAm()
#endif


/*--- Restore original pacing level -----------------------------------------*/
#pragma pack()

/*--- If C++ end declaration of header as 'C' -------------------------------*/
#ifdef __cplusplus
    }
#endif
#endif


