
/*
 * bs_logger.h:
 *      header file for bs_logger.cpp. See remarks there.
 *
 *@@include #define INCL_WINWORKPLACE
 *@@include #include <os2.h>
 *@@include #include <stdarg.h>
 *@@include #include "base\bs_string.h"
 *@@include #include "base\bs_logger.h"
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

#ifndef WARPIN_LOGGER_HEADER_INCLUDED
    #define WARPIN_LOGGER_HEADER_INCLUDED

    /* ******************************************************************
     *
     *  Logger exceptions
     *
     ********************************************************************/

    /*
     *@@ BSLoggerExcpt:
     *
     *@@added V0.9.9 (2001-03-30) [umoeller]
     */

    class BSLoggerExcpt : public BSExcptBase
    {
        public:
            BSLoggerExcpt(const char *pcsz)
            {
                _ustrDescription.assignUtf8(pcsz);
            }
    };

    /* ******************************************************************
     *
     *  Logger base class
     *
     ********************************************************************/

    /*
     *@@ BSMemLoggerBase:
     *      generic memory logger class which supports
     *      putting any kind of data into a single
     *      memory block. Useful for storing logs
     *      in an INI file.
     *
     *      The logger classes allow for storing log entries,
     *      which are simply blocks of memory in any format.
     *
     *      The format of data stored in the logger depends
     *      solely on the caller. The logger itself has no
     *      idea what the data it stores means. It is the
     *      exclusive responsibility of the caller to write
     *      and parse that data. This could be a sequence of
     *      null-terminated strings, but need not be.
     *
     *      There are several Append() methods which allow
     *      the caller to append a chunk of memory to the
     *      logger.
     *
     *      A mem logger can quickly store all its data in an
     *      OS/2 INI file thru the BSMemLoggerBase::Store method.
     *      The logger can then be reconstructed later from the
     *      INI entry though the BSMemLoggerBase::Load method.
     *
     *      Unmodified subclasses of this are declared in bs_config.h
     *      and used all over the place to log changes made to the system,
     *      which can then be stored in the database and passed to the
     *      "Undo" classes/methods to undo the changes again. All
     *      those subclasses are simply declared so that the different
     *      logger formats can be watched by the compiler.
     *
     *@@changed V0.9.0 (99-11-01) [umoeller]: renamed from LoggerBase
     *@@changed V0.9.0 (99-11-01) [umoeller]: moved this here from config.*
     *@@changed V0.9.9 (2001-03-30) [umoeller]: added BSLoggerRoot abstract base class
     *@@changed V0.9.9 (2001-03-30) [umoeller]: renamed from BSLoggerBase
     *@@changed V0.9.20 (2002-07-22) [umoeller]: added copy constructor to avoid flat copy
     *@@changed WarpIN V1.0.10 (2006-04-05) [pr]: added Clear method
     *@@changed WarpIN V1.0.18 (2008-10-06) [pr]: added Store(PXINI...) and Load(PXINI...)
     */

    class BSMemLoggerBase
    {
        public:
            char            *_pabLogString;
            unsigned long   _cbLogString;

            BSMemLoggerBase();
            ~BSMemLoggerBase();

            BSMemLoggerBase(const BSMemLoggerBase &l);

            BSMemLoggerBase& operator=(const BSMemLoggerBase &l);

            // override virtual method
            void StoreData(const char *pabData, unsigned long cbData);

            void Append(const char *pabData, unsigned long cbData);
            // void Append(const char *pcsz);
            void Append(const ustring &ustr);

            void Clear(void);

            BOOL Store(HINI hini, const char *pszApp, const char *pszKey) const;
            BOOL Store(PXINI pXIni, const char *pszApp, const char *pszKey) const;
            BOOL Load(HINI hini, const char *pszApp, const char *pszKey);
            BOOL Load(PXINI pXIni, const char *pszApp, const char *pszKey);
    };

    /*
     *@@ BSFileLogger:
     *      logger which writes to a file instead of
     *      storing the log entries in memory.
     *
     *      This needs a file name in the constructor.
     *      The logger still doesn't know what data the
     *      log contains but simply writes it out to
     *      disk.
     *
     *@@added V0.9.9 (2001-03-30) [umoeller]
     *@@changed WarpIN V1.0.14 (2006-11-30) [pr]: added WriteRaw
     */

    class BSFileLogger
    {
        string        _strFileName;
        int             _indent;
        FILE            *_File;

        public:
            BSFileLogger(ULONG ulDummy,
                         const char *pcszFilename);
            virtual ~BSFileLogger();

            void IncIndent(int i);

            void WriteV(const char *pcszFormat,
                        va_list arg_ptr);

            void Write(const char *pcszFormat,
                       ...);

            // WarpIN V1.0.14 (2006-11-30) [pr]
            void WriteRawV(const char *pcszFormat,
                           va_list arg_ptr);

            void WriteRaw(const char *pcszFormat,
                            ...);
    };

    /* ******************************************************************
     *
     *  Package loggers
     *
     ********************************************************************/

    // we define these here because they are used both in fe_script.h
    // and fe_package.h

    /*
     *@@ BSRequiresLogger:
     *      logger used in FEPackageBase to
     *      store five- or six-part package IDs
     *      which are required by a package.
     *
     *@@added V0.9.1 (2000-01-07) [umoeller]
     */

    class BSRequiresIDsLogger : public BSMemLoggerBase { };

    /*
     *@@ BSRequiresStringsLogger:
     *      logger used in FEArcPackagePck to store
     *      the REQUIRES attributes exactly as they
     *      are found in the archive. This is translated
     *      later after all packages have been parsed.
     *
     *@@added V0.9.1 (2000-01-07) [umoeller]
     */

    class BSRequiresStringsLogger : public BSMemLoggerBase { };

    /*
     *@@ BSCreatedDirsLogger:
     *      logger used in FEArcPackagePck to store
     *      the directories that were actually created
     *      during the install. This is appended to
     *      while files are being unpacked.
     *
     *@@added V0.9.20 (2002-07-22) [umoeller]
     */

    class BSCreatedDirsLogger : public BSMemLoggerBase { };

#endif


