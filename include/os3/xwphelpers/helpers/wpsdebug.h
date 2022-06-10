/*---------------------------------------------------------------------------+
|
|     (C)opyright Dennis Bareis (developed at home, in own time) 1996 - ALL RIGHTS RESERVED
|     ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
|
|     MODULE NAME:   WPSDEBUG.H
|
|
|         $Author$
|       $Revision$
|           $Date$
|        $Logfile:   V:/SUEPVCS/SUPPORT/TEMPLATE.H_V  $
|
|     DESCRIPTION:   Note that any items that begin with an "I_" are for
|                    internal use, not for general debugging.
|
|                    This header file is for SOM/WPS debugging and uses
|                    the PMPRINTF.EXE program to display the output.  Use
|                    of this API instead of my PMPRINTF API ensures that
|                    SOM/WPS messages other than those you put out (for
|                    example base OS/2 errors) should and will be mingled
|                    with your messages and hopefully being of some use!
|
|         CONTACT:   If you have any problems to report or better yet
|                    suggestions, you may email me
|                    at "dbareis@ozemail.com.au"...
+---------------------------------------------------------------------------*/
#ifndef  HEADER_WPSDEBUG_H
#define  HEADER_WPSDEBUG_H

/*--- Need access to "PMPRINTF" ---------------------------------------------*/
#ifndef  HEADER_PMPRINTF_H
         #include "PMPRINTF.H"
#endif

/*--- If C++ declare header as 'C' ------------------------------------------*/
#ifdef __cplusplus
    extern "C"
    {
#endif

/*--- Define version number -------------------------------------------------*/
#define WPSDEBUG_VERSION  "96.175"

/*--- Make sure the "__FUNCTION__" macro exists -----------------------------*/
#ifndef __FUNCTION__
        #define __FUNCTION__ "FunctionUnknown(?)"   //IBM CSET 2.01 and onwards support this
#endif

/*--- Define the maximum line length ----------------------------------------*/
#define WPSDEBUG_MAX_LINE_CHARS   PMPRINTF_MAX_LINE_CHARS


/*--- Use this macro to turn on/off tracing ---------------------------------*/
#define  WpsDebugHigh()                                \
         WpsDebugInstall(WPSDEBUG_TLEVEL_CORE_AND_USER, PMPRINTF_ALL_HEADER_FIELDS_EXCEPT_TIME)
#define  WpsDebugInstall(TraceLevel, PmprintfMode)     \
       I_WpsDebugInstall(TraceLevel, PmprintfMode, (void *)&SOMOutCharRoutine, &SOM_TraceLevel)


/*--- Better debug macro ----------------------------------------------------*/
#define WpsDebug(FmtAndVariableParmsInBrackets)                                  \
        do                                                                       \
        {                                                                        \
            /*--- Don't do anything if debugging is not turned on -----------*/  \
            if  (SOM_TraceLevel != 0)                                            \
            {                                                                    \
                /*--- Output a header ---------------------------------------*/  \
                I_WpsDebugOutputModuleIdHeader(__FILE__, __LINE__, NULL);        \
                                                                                 \
                /*--- Output the message ------------------------------------*/  \
                somPrintf FmtAndVariableParmsInBrackets;                         \
            }                                                                    \
        }                                                                        \
        while   (0)
#define WpsDebugF(FmtAndVariableParmsInBrackets)                                 \
        do                                                                       \
        {                                                                        \
            /*--- Don't do anything if debugging is not turned on -----------*/  \
            if  (SOM_TraceLevel != 0)                                            \
            {                                                                    \
                /*--- Output a header ---------------------------------------*/  \
                I_WpsDebugOutputModuleIdHeader(__FILE__, __LINE__, __FUNCTION__);\
                                                                                 \
                /*--- Output the message ------------------------------------*/  \
                somPrintf FmtAndVariableParmsInBrackets;                         \
            }                                                                    \
        }                                                                        \
        while   (0)

/*--- Simple to use "Here I am" macros --------------------------------------*/
#ifndef  WPSDEBUG_HERE_I_AM_MSG
         #define WPSDEBUG_HERE_I_AM_MSG  " Here I am"
#endif
#define WpsDebugHereIAm()       WpsDebugF((WPSDEBUG_HERE_I_AM_MSG "\n"))


/*--- Method starting & stopping macros -------------------------------------*/
#define WpsDebugMethodStart()   WpsDebugF(("Method Starting" "\n"))
#define WpsDebugMethodEnd()     WpsDebugF(("Method ending"   "\n"))


/*--- The following macros allow you to conditionly include debug code ------*/
#ifdef  _WPSDEBUG_
        /*--- The user wants to include these debug lines -------------------*/
        #define _WpsDebugHigh()                            WpsDebugHigh()
        #define _WpsDebugInstall(TraceLevel, PmprintfMode) WpsDebugInstall(TraceLevel, PmprintfMode)
        #define _WpsDebug(FmtAndVariableParmsInBrackets)   WpsDebug(FmtAndVariableParmsInBrackets)
        #define _WpsDebugF(FmtAndVariableParmsInBrackets)  WpsDebugF(FmtAndVariableParmsInBrackets)
        #define _WpsDebugHereIAm()                         WpsDebugHereIAm()
        #define _WpsDebugMethodStart()                     WpsDebugMethodStart()
        #define _WpsDebugMethodEnd()                       WpsDebugMethodEnd()
#else
        /*--- The user does not wish to include the debug stuff -------------*/
        #define _WpsDebugHigh()
        #define _WpsDebugInstall(TraceLevel, PmprintfMode)
        #define _WpsDebug(FmtAndVariableParmsInBrackets)
        #define _WpsDebugF(FmtAndVariableParmsInBrackets)
        #define _WpsDebugHereIAm()
        #define _WpsDebugMethodStart()
        #define _WpsDebugMethodEnd()
#endif


/*--- Definitions for "WPSDEBUG.C" ------------------------------------------*/
void _Optlink I_WpsDebugOutputModuleIdHeader(UCHAR *SourceModule, int LineNumber, UCHAR *Function);
void _Optlink I_WpsDebugInstall( int           TraceLevel,
                                               #define   WPSDEBUG_TLEVEL_NONE          0
                                               #define   WPSDEBUG_TLEVEL_USER          1
                                               #define   WPSDEBUG_TLEVEL_CORE_AND_USER 2
                                 PMPRINTF_MODE PmprintfMode,
                                 void       * *AddressOfSOMOutCharRoutine,
                                 int        *  AddressOfSOM_TraceLevel
                               );
void _Optlink WpsDebugPmMsgBox( UCHAR * Title,
                                UCHAR * Format,
                                ...
                              );




/*--- Override the default "SOMMethodDebug()" macro -------------------------*/
#define  WPSDEBUG_SOMMETHODDEBUG(ClassName, MethodName)          \
        _WpsDebugF(("Class = \"%s\"\n", ClassName))      /* Default action */
#ifdef   SOMMethodDebug
         #undef  SOMMethodDebug
#endif
#define  SOMMethodDebug(ClassName, MethodName)                   \
         WPSDEBUG_SOMMETHODDEBUG(ClassName, MethodName);


/*--- If C++ end declaration of header as 'C' -------------------------------*/
#ifdef __cplusplus
    }
#endif
#endif


