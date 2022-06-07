
/*
 *@@sourcefile bs_logger.cpp:
 *      this implements the various logger classes.
 *
 *      See BSLoggerRoot and subclasses for details.
 *
 *      The base logger class was renamed from INILogger and moved
 *      to this separate file (99-11-01) [umoeller].
 *      More logger classes were added with V0.9.9 (2001-03-30) [umoeller].
 *
 *@@header "cppbase\bs_logger.h"
 *@@added V0.9.0 (99-11-01) [umoeller]
 */

/*
 *      This file Copyright (C) 1999-2008 Ulrich M”ller.
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation, in version 2 as it comes in the COPYING
 *      file of this distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 */

#define OS2EMX_PLAIN_CHAR
    // this is needed for "os2emx.h"; if this is defined,
    // emx will define PSZ as _signed_ char, otherwise
    // as unsigned char

#define INCL_DOSSEMAPHORES
#define INCL_WINSHELLDATA
#include <os2.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <io.h>
#include <fcntl.h>
#include <sys\stat.h>

#include "setup.h"
#include "bldlevel.h"

// include's from helpers
#include "helpers\prfh.h"
#include "helpers\xprf.h"
#include "helpers\xstring.h"

// base includes
#include "cppbase\bs_base.h"
#include "cppbase\bs_list.h"
#include "cppbase\bs_string.h"
#include "cppbase\bs_errors.h"

#include "cppbase\bs_logger.h"
#include "cppbase\bs_config.h"

#pragma hdrstop

/* ******************************************************************
 *
 *  BSLoggerRoot implementation
 *
 ********************************************************************/

/* ******************************************************************
 *
 *  BSMemLoggerBase implementation
 *
 ********************************************************************/

/*
 *@@ BSMemLoggerBase:
 *      default constructor for creating an empty log string.
 *
 *      See BSMemLoggerBase::Append for example usage.
 *
 *      BSMemLoggerBase::Store and BSMemLoggerBase::Load can be used for
 *      storing a logger into or retrieving one from an INI file.
 */

BSMemLoggerBase::BSMemLoggerBase()
{
    _pabLogString = 0;
    _cbLogString = 0;
}

/*
 *@@ ~BSMemLoggerBase:
 *      destructor
 */

BSMemLoggerBase::~BSMemLoggerBase()
{
    if (_pabLogString)
        free(_pabLogString);
}

/*
 *@@ BSMemLoggerBase:
 *      copy constructor to avoid a flat copy of the heap data.
 *
 *@@added V0.9.20 (2002-07-22) [umoeller]
 */

BSMemLoggerBase::BSMemLoggerBase(const BSMemLoggerBase &l)
{
    if (_cbLogString = l._cbLogString)
    {
        _pabLogString = (PSZ)malloc(_cbLogString);
        memcpy(_pabLogString, l._pabLogString, _cbLogString);
    }
    else
        _pabLogString = 0;
}

/*
 *@@ operator=:
 *      assignment operator to avoid a flat copy of the heap data.
 *
 *@@added V0.9.20 (2002-07-22) [umoeller]
 */

BSMemLoggerBase& BSMemLoggerBase::operator=(const BSMemLoggerBase &l)
{
    if (_cbLogString = l._cbLogString)
    {
        _pabLogString = (PSZ)malloc(_cbLogString);
        memcpy(_pabLogString, l._pabLogString, _cbLogString);
    }
    else
        _pabLogString = 0;

    return *this;
}

/*
 *@@ StoreData:
 *      implementation for the pure virtual method
 *      in BSLoggerRoot to store a chunk of data.
 *
 *      The implementation in BSMemLoggerBase stores
 *      this in the logger's memory block.
 *
 *@@added V0.9.9 (2001-03-30) [umoeller]
 */

void BSMemLoggerBase::StoreData(const char *pabData, unsigned long cbData)
{
    if (pabData)
    {
        if (_pabLogString == NULL)
        {
            // first run:
            _pabLogString = (char*)malloc(cbData);
            memcpy(_pabLogString, pabData, cbData);
            _cbLogString = cbData;
        }
        else
        {
            // subsequent runs: append after the existing data
            // V0.9.9 (2001-03-30) [umoeller]: now using realloc

            _pabLogString = (char*)realloc(_pabLogString, _cbLogString + cbData);
            // copy new attribs behind existing data
            memcpy(_pabLogString + _cbLogString,
                   pabData,
                   cbData);
            _cbLogString += cbData;

            /* char*   pszTemp = (char*)malloc(_cbLogString + cbData);
            // copy old buffer
            memcpy(pszTemp, _pabLogString, _cbLogString);
            // copy new attribs behind last null byte
            memcpy(pszTemp + _cbLogString,
                   pszData,
                   cbData);
            // set new buffer
            free(_pabLogString);
            _pabLogString = pszTemp;
            _cbLogString += cbData; */
        }
    }
}

/*
 *@@ Append:
 *      this appends binary data to the log string.
 *      This can be any data, but you should be able
 *      to decode it again so if each data is variable
 *      in length, you should add some markers.
 *
 *      This makes a copy of the binary data and appends
 *      it to the logger, if data already exists.
 *
 *      Example 1: you could add plain text strings.
 *                 Don't forget to store the null terminator then too.
 *
 +          const char *pcsz = "Hello";
 +          Logger.Append(pcsz, strlen(pcsz) + 1);
 *
 *      Example 2: you could add binary data, with
 *                 each item having a predefined length.
 *
 +          char    *pabData = (char*)malloc(10);
 +          memset(pabData, 0, 10);
 +          Logger.Append(pabData, 10);
 +          free(pabData);
 *
 *@@changed V0.9.1 (2000-01-05) [umoeller]: fixed memory allocation problems
 *@@changed V0.9.9 (2001-03-30) [umoeller]: now using realloc
 */

void BSMemLoggerBase::Append(const char *pabData,     // in: data block to append (must be terminated with null)
                             unsigned long cbData)    // in: sizeof(*pszData), including the null byte
{
    StoreData(pabData, cbData);
}

/*
 *@@ Append:
 *      overloaded Append, which takes a BSUString as input.
 *      this takes a BSString as input. This is faster than
 *      the const char* method because BSString maintains the
 *      length of the string automatically.
 *
 *      BSMemLoggerBase now only accepts ustrings, no longer
 *      codepage strings V0.9.18 (2002-03-08) [umoeller].
 *
 *      Example:
 +          BSUString str("Hello");
 +          Logger.Append(str);
 *      is equal to
 +          Logger.Append("Hello", strlen("Hello") + 1);
 *
 *@@added V0.9.9 (2001-02-28) [umoeller]
 *@@changed V0.9.12 (2001-05-22) [umoeller]: fixed missing null terminator (CONFIG.SYS garbage in database)
 *@@changed V0.9.18 (2002-03-08) [umoeller]: now using ustrings
 */

void BSMemLoggerBase::Append(const ustring &ustr)
{
    StoreData(ustr.GetBuffer(),
              ustr.size() + 1);      // null terminator was missing,
                                    // fixed V0.9.12 (2001-05-22) [umoeller]
}

/*
 *@@ Clear:
 *      this clears the whole Logger string.
 *
 *@@added WarpIN V1.0.10 (2006-04-05) [pr]
 */

void BSMemLoggerBase::Clear(void)
{
    if (_pabLogString)
    {
        free(_pabLogString);
        _pabLogString = 0;
        _cbLogString = 0;
    }
}

/*
 *@@ Store:
 *      this stores the whole BSMemLoggerBase in the given profile key.
 *      Returns TRUE if successfully written.
 *
 *      Note: if the logger is currently empty, this will delete
 *      the given INI key.
 *
 *@@changed WarpIN V1.0.18 (2008-10-06) [pr]: added Store(PXINI...)
 */

BOOL BSMemLoggerBase::Store(HINI hini,            // in: INI handle
                            const char *pszApp,   // in: INI application
                            const char *pszKey)   // in: INI key
                   const
{
    return (PrfWriteProfileData(hini, pszApp, pszKey, _pabLogString, _cbLogString));
}

BOOL BSMemLoggerBase::Store(PXINI pXIni,            // in: INI handle
                            const char *pszApp,   // in: INI application
                            const char *pszKey)   // in: INI key
                   const
{
    return (xprfWriteProfileData(pXIni, pszApp, pszKey, _pabLogString, _cbLogString));
}

/*
 *@@ Load:
 *      reverse to BSMemLoggerBase::Store, this loads the data from INI.
 *      This will overwrite the current contents of the logger.
 *
 *      Returns TRUE if data was found.
 *
 *@@changed WarpIN V1.0.18 (2008-10-06) [pr]: added Load(PXINI...)
 */

BOOL BSMemLoggerBase::Load(HINI hini,          // in: INI handle
                        const char *pszApp,    // in: INI application
                        const char *pszKey)    // in: INI key
{
    if (_pabLogString)
        free(_pabLogString);
    _pabLogString = prfhQueryProfileData(hini,
                                         (PSZ)pszApp,
                                         (PSZ)pszKey,
                                         &_cbLogString);
    return (_pabLogString != 0);
}

BOOL BSMemLoggerBase::Load(PXINI pXIni,        // in: INI handle
                        const char *pszApp,    // in: INI application
                        const char *pszKey)    // in: INI key
{
    if (_pabLogString)
        free(_pabLogString);
    _pabLogString = xprfhQueryProfileData(pXIni,
                                          (PSZ)pszApp,
                                          (PSZ)pszKey,
                                          &_cbLogString);
    return (_pabLogString != 0);
}

/* ******************************************************************
 *
 *  BSFileLogger implementation
 *
 ********************************************************************/

BSMutex G_mtxFileLoggers;       // V0.9.20 (2002-07-06) [umoeller]

#if 0

HMTX G_hmtxFileLoggers = NULLHANDLE;

/*
 *@@ LockFileLoggers:
 *
 *@@added V0.9.12 (2001-05-31) [umoeller]
 */

BOOL LockFileLoggers(VOID)
{
    if (!G_hmtxFileLoggers)
        return (!DosCreateMutexSem(NULL,
                                   &G_hmtxFileLoggers,
                                   0,
                                   TRUE));

    return (!DosRequestMutexSem(G_hmtxFileLoggers, SEM_INDEFINITE_WAIT));
}

/*
 *@@ UnlockFileLoggers:
 *
 *@@added V0.9.12 (2001-05-31) [umoeller]
 */

VOID UnlockFileLoggers(VOID)
{
    DosReleaseMutexSem(G_hmtxFileLoggers);
}

#endif

/*
 *@@ BSFileLogger:
 *      constructor to open a file logger with the
 *      specified file name.
 *
 *@@added V0.9.9 (2001-03-30) [umoeller]
 *@@changed V0.9.12 (2001-05-31) [umoeller]: added mutex
 *@@changed V0.9.19 (2002-07-01) [umoeller]: added bldlevel to log
 *@@changed WarpIN V1.0.14 (2006-11-30) [pr]: removed WarpIN specific header message
 */

BSFileLogger::BSFileLogger(ULONG ulDummy,
                           const char *pcszFilename)
{
    BSLock lock(G_mtxFileLoggers); // V0.9.20 (2002-07-06) [umoeller]

    _Pmpf(("BSFileLogger: constructor"));

    _File = NULL;
    _strFileName = pcszFilename;

    _indent = 0;

    if (pcszFilename)
    {
        _File = fopen(pcszFilename,
                      "a");
        if (!_File)
        {
            CHAR szError[400];
            sprintf(szError,
                    "Cannot open log file \"%s\" for writing.",
                    pcszFilename);
            throw BSLoggerExcpt(szError);
        }
    }
    else
        throw BSLoggerExcpt("pcszFilename is NULL.");
}

/*
 *@@ ~BSFileLogger:
 *      destructor. Closes the log file.
 *
 *@@added V0.9.9 (2001-03-30) [umoeller]
 *@@changed V0.9.12 (2001-05-31) [umoeller]: added mutex
 *@@changed WarpIN V1.0.14 (2006-11-30) [pr]: removed "install log closed" message
 */

BSFileLogger::~BSFileLogger()
{
    BSLock lock(G_mtxFileLoggers); // V0.9.20 (2002-07-06) [umoeller]

    _Pmpf(("BSFileLogger: destructor"));

    if (_File)
    {
        fclose(_File);
        _File = NULL;
    }
}

/*
 *@@ IncIndent:
 *      modifies indentation for the file output
 *      (spaces between date/time and the actual
 *      logger output).
 *
 *      A positive "i" increases the spacing, a
 *      negative decreases it.
 *
 *@@added V0.9.9 (2001-03-30) [umoeller]
 *@@changed V0.9.12 (2001-05-31) [umoeller]: added mutex
 */

void BSFileLogger::IncIndent(int i)
{
    BSLock lock(G_mtxFileLoggers); // V0.9.20 (2002-07-06) [umoeller]

    _indent += i;

    if (i < 0)
        i = 0;
}

/*
 *@@ WriteV:
 *
 *      NOTE: This appends \n for each string.
 *
 *@@added V0.9.9 (2001-03-30) [umoeller]
 *@@changed V0.9.12 (2001-05-31) [umoeller]: added mutex
 *@@changed V0.9.14 (2001-07-26) [umoeller]: added hundredths to log lines
 *@@changed V0.9.18 (2002-03-08) [umoeller]: renamed from Append(); now adding \n after each line
 */

void BSFileLogger::WriteV(const char *pcszFormat,  // in: format string
                          va_list arg_ptr)
{
    BSLock lock(G_mtxFileLoggers); // V0.9.20 (2002-07-06) [umoeller]

    DATETIME dt;
    DosGetDateTime(&dt);

    fprintf(_File,
            "%02d:%02d:%02d.%02d ",
            dt.hours, dt.minutes, dt.seconds, dt.hundredths);

    if (_indent)
    {
        for (int i = 0;
             i < _indent;
             i++)
            fprintf(_File, " ");
    }

    vfprintf(_File, pcszFormat, arg_ptr);
    fprintf(_File, "\n");
}

/*
 *@@ Write:
 *
 *@@added V0.9.18 (2002-03-08) [umoeller]
 */

void BSFileLogger::Write(const char *pcszFormat,  // in: format string
                         ...)                     // in: variable arguments
{
    va_list arg_ptr;
    va_start(arg_ptr, pcszFormat);
    WriteV(pcszFormat, arg_ptr);
    va_end(arg_ptr);
}

/*
 *@@ WriteRawV:
 *
 *@@added WarpIN V1.0.14 (2006-11-30) [pr]
 */

void BSFileLogger::WriteRawV(const char *pcszFormat,  // in: format string
                             va_list arg_ptr)
{
    BSLock lock(G_mtxFileLoggers);

    vfprintf(_File, pcszFormat, arg_ptr);
}

/*
 *@@ WriteRaw:
 *
 *@@added WarpIN V1.0.14 (2006-11-30) [pr]
 */

void BSFileLogger::WriteRaw(const char *pcszFormat,  // in: format string
                            ...)                     // in: variable arguments
{
    va_list arg_ptr;
    va_start(arg_ptr, pcszFormat);
    WriteRawV(pcszFormat, arg_ptr);
    va_end(arg_ptr);
}

