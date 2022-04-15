/**************************************************************************
*
*             eCS RexxUtil Library Replacement Project
*
*  This list of functions derived by running exehdr against the
*  rexxutil.dll distributed with eCS 2.0
*
*  Michael Greene, January 2008
*
**************************************************************************/


/*********************************************************************/
/* Library name                                                      */
/*********************************************************************/
#define RXLIBNAME  "REXXUTIL"


/*********************************************************************/
/* RxFncTable                                                        */
/*   Array of names of the REXXUTIL functions.                       */
/*   This list is used for registration and deregistration.          */
/*********************************************************************/

static char *RxFncTable[ ] = {
    RXLIBNAME,
    "SysCls",                      // sysmisc.c
    "SysCurPos",                   // sysmisc.c
    "SysCurState",                 // sysmisc.c
    "SysDriveInfo",                // sysdrive.c
    "SysDriveMap",                 // sysdrive.c
    "SysDropFuncs",                // rexxfuncs.c
    "SysFileDelete",               // sysfile.c
    "SysFileSearch",               // sysfile.c
    "SysFileTree",                 // sysfile.c
    "SysGetKey",                   // sysmisc.c
    "SysGetMessage",               // sysmisc.c
    "SysIni",                      // sysmisc.c
    "SysLoadFuncs",                // rexxfuncs.c
    "SysMkDir",                    // sysmiscfile.c
    "SysOS2Ver",                   // sysversion.c
    "SysRmDir",                    // sysmiscfile.c
    "SysSearchPath",               // sysfile.c
    "SysSleep",                    // sysmisc.c
    "SysTempFileName",             // sysmiscfile.c
    "SysTextScreenRead",           // systextscreen.c
    "SysTextScreenSize",           // systextscreen.c
    "SysGetEA",                    // syseautil.c
    "SysPutEA",                    // syseautil.c
    "SysWaitNamedPipe",            // sysmisc.c
    "SysSetIcon",                  // sysobjects.c
    "SysRegisterObjectClass",      // sysobjects.c
    "SysDeregisterObjectClass",    // sysobjects.c
    "SysCreateObject",             // sysobjects.c
    "SysQueryClassList",           // sysobjects.c
    "SysDestroyObject",            // sysobjects.c
    "SysSetObjectData",            // sysobjects.c
    "SysSaveObject",               // sysobjects.c
    "SysOpenObject",               // sysobjects.c
    "SysMoveObject",               // sysobjects.c
    "SysCopyObject",               // sysobjects.c
    "SysCreateShadow",             // sysobjects.c
    "SysBootDrive",                // sysdrive.c
    "SysQueryEAList",              // syseautil.c
    "SysWildCard",                 // sysmiscfile.c
    "SysFileSystemType",           // sysfile.c
    "SysAddFileHandle",            // sysmiscfile.c
    "SysSetFileHandle",            // sysmiscfile.c
    "SysCreateMutexSem",           // sysmutexsem.c
    "SysOpenMutexSem",             // sysmutexsem.c
    "SysCloseMutexSem",            // sysmutexsem.c
    "SysRequestMutexSem",          // sysmutexsem.c
    "SysReleaseMutexSem",          // sysmutexsem.c
    "SysCreateEventSem",           // syseventsem.c
    "SysOpenEventSem",             // syseventsem.c
    "SysCloseEventSem",            // syseventsem.c
    "SysResetEventSem",            // syseventsem.c
    "SysPostEventSem",             // syseventsem.c
    "SysWaitEventSem",             // syseventsem.c
    "SysSetPriority",              // sysprocess.c
    "SysProcessType",              // sysprocess.c
    "SysGetCollate",               // syslang.c
    "SysNationalLanguageCompare",  // syslang.c
    "SysMapCase",                  // syslang.c
    "SysSetProcessCodePage",       // syslang.c
    "SysQueryProcessCodePage",     // syslang.c
    "SysAddRexxMacro",             // sysmacro.c
    "SysDropRexxMacro",            // sysmacro.c
    "SysReorderRexxMacro",         // sysmacro.c
    "SysQueryRexxMacro",           // sysmacro.c
    "SysClearRexxMacroSpace",      // sysmacro.c
    "SysLoadRexxMacroSpace",       // sysmacro.c
    "SysSaveRexxMacroSpace",       // sysmacro.c
    "SysShutDownSystem",           // sysmisc.c
    "SysSwitchSession",            // sysmisc.c
    "SysLoadLibrary",              // sysprocess.c - Note below
    "SysDropLibrary",              // sysprocess.c - Note below
    "SysElapsedTime",              // sysmisc.c
    "SysWaitForShell",             // sysmisc.c
    "SysQuerySwitchList",          // sysmisc.c
    "SysQueryExtLIBPATH",          // sysprocess.c
    "SysSetExtLIBPATH",            // sysprocess.c
    "SysDumpVariables",            // sysprocess.c
    "SysSetFileDateTime",          // sysfile.c
    "SysGetFileDateTime",          // sysfile.c
    "SysStemSort",                 // sysstem.c    ** part from regutil
    "SysStemDelete",               // sysstem.c
    "SysStemInsert",               // sysstem.c
    "SysStemCopy",                 // sysstem.c
    "SysVersion",                  // sysversion.c
    "SysUtilVersion",              // sysversion.c

// Addition functions
//    "RxMessageBox",
    "SysReplaceObject",            // sysobjects.c  -- Rich Walsh suggestion

    "SysPi",                       // sysmath.c     -- math functions from oorexx
    "SysSqrt",                     // sysmath.c
    "SysExp",                      // sysmath.c
    "SysLog",                      // sysmath.c
    "SysLog10",                    // sysmath.c
    "SysSinh",                     // sysmath.c
    "SysCosh",                     // sysmath.c
    "SysTanh",                     // sysmath.c
    "SysPower",                    // sysmath.c
    "SysSin",                      // sysmath.c
    "SysCos",                      // sysmath.c
    "SysTan",                      // sysmath.c
    "SysCotan",                    // sysmath.c
    "SysArcSin",                   // sysmath.c
    "SysArcCos",                   // sysmath.c
    "SysArcTan"                    // sysmath.c
};
// Note: Undocumented returns INVALID_ROUTINE

