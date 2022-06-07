
/*
 *@@sourcefile bs_config.cpp:
 *      this has the code for the classes which manipulate CONFIG.SYS,
 *      the WPS class list, create WPS objects, or do other
 *      WarpIN system configuration.
 *
 *      These classes are designed to work independently of WarpIN
 *      and could be used in any program.
 *      I have attempted to design these classes for easy, yet flexible use.
 *
 *      All system config classes are derived from BSConfigBase.
 *      This allows for maintaining a list of all system changes
 *      with each WarpIN package, which have a somewhat uniform
 *      interface. Each config class has in turn a special method
 *      which will actually perform the system configuration change.
 *
 *      This special method will usually take a logger instance
 *      as a parameter, which logs the changes that were actually
 *      made to the system. This logger can then be used later to
 *      undo the change.
 *      See BSCfgSysManip::BSCfgSysManip for an example usage.
 *
 *      The loggers are all direct, unmodified descendants of the
 *      BSMemLoggerBase class (bs_logger.cpp) and only defined to
 *      use C++ type checking.
 *
 *      Some config methods throw instances of the BSConfigExcpt
 *      class def'd in bs_config.h, which you should handle.
 *
 *      These are the classes implemented here:
 *
 *      --  BSCfgSysManip:
 *          a "manipulator" object of this class holds information
 *          for a CONFIG.SYS change, which is passed to BSConfigSys::Manipulate.
 *          See BSCfgSysManip::BSCfgSysManip for a description.
 *
 *      --  BSConfigSys:
 *          the actual class which holds the CONFIG.SYS text file
 *          and allows changes to be made.
 *          This class does not have a corresponding "Undo" class.
 *          Instead, use BSCfgSysManip::AddToUndoList.
 *
 *      --  BSRegisterClass:
 *          this allows registering classes with the WPS.
 *          See BSRegisterClass::Register for an example usage.
 *
 *      --  BSDeregisterClass:
 *          the "Undo" class to the previous.
 *
 *      --  BSReplaceClass:
 *          this allows WPS classes to be replaced.
 *          See BSReplaceClass::Replace for an example usage.
 *
 *      --  BSUnreplaceClass:
 *          the "Undo" class to the previous.
 *
 *      --  BSCreateWPSObject:
 *          for creating a WPS object.
 *
 *      --  BSDeleteWPSObject:
 *          the "Undo" class for the previous, to delete a WPS object again.
 *
 *      See bs_config.h for the declarations of these classes.
 *
 *@@header "cppbase\bs_config.h"
 *@@header "cppbase\bs_config_impl.h"
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

#define INCL_DOSSESMGR
#define INCL_DOSSEMAPHORES
#define INCL_DOSERRORS
#define INCL_WINWINDOWMGR
#define INCL_WINMESSAGEMGR
#define INCL_WINDIALOGS
#define INCL_WINSTDCNR
#define INCL_WININPUT
#define INCL_WINSYS
#define INCL_WINSHELLDATA
#define INCL_WINWORKPLACE
#include <os2.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "setup.h"

// include's from helpers
#include "helpers\dosh.h"
#include "helpers\winh.h"
#include "helpers\prfh.h"
#include "helpers\stringh.h"
#include "helpers\xprf.h"
#include "helpers\xstring.h"

#include "helpers\configsys.h"

// front-end includes
#include "cppbase\bs_base.h"
#include "cppbase\bs_list.h"
#include "cppbase\bs_string.h"
#include "cppbase\bs_errors.h"

#include "cppbase\bs_logger.h"
#include "cppbase\bs_config.h"
#include "cppbase\bs_config_impl.h"

#pragma hdrstop

DEFINE_CLASS(BSConfigBase, BSRoot);

DEFINE_CLASS(BSCfgSysManip, BSConfigBase);
DEFINE_CLASS(BSConfigSys, BSRoot);
DEFINE_CLASS(BSRegisterClass, BSConfigBase);
DEFINE_CLASS(BSDeregisterClass, BSConfigBase);
DEFINE_CLASS(BSReplaceClassBase, BSConfigBase);
    DEFINE_CLASS(BSReplaceClass, BSReplaceClassBase);
    DEFINE_CLASS(BSUnreplaceClass, BSReplaceClassBase);
DEFINE_CLASS(BSCreateWPSObject, BSConfigBase);
DEFINE_CLASS(BSDeleteWPSObject, BSConfigBase);
DEFINE_CLASS(BSProfileBase, BSConfigBase);
    DEFINE_CLASS(BSClearProfile, BSProfileBase);
    DEFINE_CLASS(BSWriteProfile, BSProfileBase);
DEFINE_CLASS(BSExecute, BSConfigBase);
    DEFINE_CLASS(BSDeExecute, BSExecute);
DEFINE_CLASS(BSKillProcess, BSConfigBase);

    #define CONVERT(c, s) string str ## s(c, _ustr ## s); \
            PCSZ pcsz ## s = str ## s.c_str()

/* ******************************************************************
 *
 *  BSCfgSysManip class
 *
 ********************************************************************/

/*
 *@@ BSCfgSysManip(char* pszConfigSys):
 *      this constructor translates a CONFIGSYS attribute (as used
 *      in the PCK tag and stored in the database) into a CONFIGSYSITEM
 *      structure.
 *
 *      The BSCfgSysManip class is designed for use with the
 *      BSConfigSys class and describes manipulations to be done upon
 *      the CONFIG.SYS file (represented by the BSConfigSys class).
 *
 *      Manipulating CONFIG.SYS works just as follows:
 *
 *      1)  Create an instance of BSConfigSys:
 *
 +              BSConfigSys *pConfigSys = new BSConfigSys;
 *
 *          This will load your CONFIG.SYS file into the instance's memory.
 *
 *      2)  Create an instance of BSCfgSysManip and specify the manipulation
 *          in the constructor:
 *
 +              BSCfgSysManip *pManip = new BSCfgSysManip("SET TEST=YES | UNIQUE");
 *
 *          The BSCfgSysManip class has a single constructor which takes a PSZ
 *          (char*) as input. This input string has exactly the format like
 *          with the CONFIGSYS attribute to the PCK tag in WarpIN installation
 *          scripts, like this (see the WarpIN Programmer's Reference for details):
 *
 +              statement [| modifiers]
 *
 *          "modifiers" can be one of the following:
 +              [UNIQUE[(xxx)]] [vertical]
 +              ADDRIGHT [vertical]
 +              ADDLEFT [vertical]
 +              REMOVELINE
 +              REMOVEPART
 *          with "xxx" being a search string.
 *
 *          "vertical" can be one of the following:
 +              ADDTOP
 +              ADDAFTER(xxx)
 +              ADDBEFORE(xxx)
 *          with "xxx" being a search string.
 *
 *          Examples:
 +              "BASEDEV=NEWDRIVR.SYS /WHATEVER | UNIQUE ADDAFTER(IBM1S506.ADD)"
 +              "SET PATH=C:\BLAH | ADDRIGHT"
 *
 *          After this constructor has successfully converted pszConfigSys,
 *          all the instance data is valid (see cfgsys.h).
 *
 *          However, this does _not_ handle macro resultion like in WarpIn
 *          scripts (which is done in warpin.cpp before calling the Manipulate
 *          method), because in this file scope we know nothing about the
 *          PackageInfo instances.
 *
 *      3)  Create a CfgSysDone logger instance:
 +              BSCfgSysDoneLogger logger;
 *
 *      4)  Invoke the BSConfigSys::Manipulate method with the BSCfgSysManip
 *          instance (this will add data to the logger):
 +              pConfigSys->Manipulate(logger, pManip);
 *
 *      5)  Write CONFIG.SYS back to disk and clean up.
 +              pConfigSys->Flush(TRUE);
 +              delete pBSCfgSysManip;
 +              delete pConfigSys;
 *
 *      Now, if you want to undo the changes later, call the static
 *      BSCfgSysManip::AddToUndoList method with the logger passed to
 *      to the various constructor calls (above), which will create
 *      a list of BSCfgSysManip instance which will be able to undo
 *      the changes again:
 *
 +          list<BSCfgSysManip*> UndoList;
 +          BSCfgSysManip::AddToUndoList(UndoList, logger);
 *
 *      and iterate over the list and call BSConfigSys::Manipulate with
 *      the objects on that list to undo the changes.
 *
 *      Throws:
 *      -- BSConfigExcpt.
 *
 *@@changed V0.9.1 (2000-01-06) [umoeller]: added UNIQUE(xxx) support
 *@@changed V0.9.5 (2000-08-26) [umoeller]: fixed UNIQUE(xxx) and REMOVELINE, which deleted wrong lines
 */

BSCfgSysManip::BSCfgSysManip(const ustring &ustrConfigSys)
               : BSConfigBase(CFGT_CFGSYS, tBSCfgSysManip)
{
    PCSZ pcszConfigSys;

    if (!(pcszConfigSys = ustrConfigSys.GetBuffer()))
        throw BSConfigExcpt(CFGEXCPT_SYNTAX, 0);

    // initialize all fields to zero
    // memset(&_ConfigManip, 0, sizeof(_ConfigManip));
            // V0.9.12 (2001-05-22) [umoeller]

    _iReplaceMode = CFGRPL_ADD; // V1.0.4 (2004-12-18) [pr]
    _iVertical = CFGVRT_BOTTOM;
    // Moved above statements here from the else{} block below. If there wasn't a modifier,
    // this was uninitialised garbage and CONFIG.SYS never got modified.

    // now check if we have modifiers
    PSZ     pSep;
    if (!(pSep = strchr(pcszConfigSys, '|')))
        // no modifiers: just copy
        _ustrNewLine = ustrConfigSys;
    else
    {
        // we do have modifiers:
        BOOL    fVerticalsAllowed = TRUE;

        // get rid of spaces before '|'
        PSZ     pSep2 = pSep-1;
        while (*pSep2 == ' ')
            pSep2--;

        // get the "statement" part
        _ustrNewLine.assignUtf8(pcszConfigSys, pSep2 + 1);

        // remember the modifiers position
        PSZ     pModifiers = pSep + 1;
        BOOL    fReplaceModeFound = FALSE;
        // now check for the replace mode;
        // the default is 0 (CFGRPL_ADD)
        fVerticalsAllowed = TRUE;

        PSZ pszUnique;
        if ((pszUnique = strhistr(pModifiers, "UNIQUE")))
        {
            _iReplaceMode = CFGRPL_UNIQUE;
            fVerticalsAllowed = TRUE;
            fReplaceModeFound = TRUE;   // block future replacement modifiers

            // check if we have a "UNIQUE(xxx)" syntax
            PSZ psz;
            if (psz = strhExtract(pszUnique, '(', ')', NULL))
            {
                // if found, this extracts the stuff between ( and );
                // if not, this returns NULL
                _ustrUniqueSearchString2.assignUtf8(psz);
                free(psz);
            }
        }
        if (strhistr(pModifiers, "ADDRIGHT"))
        {
            if (!fReplaceModeFound)
            {
                _iReplaceMode = CFGRPL_ADDRIGHT;
                fVerticalsAllowed = TRUE;
                fReplaceModeFound = TRUE; // block future replacement modifiers
            }
            else
            {
                // double replace mode found:
                throw BSConfigExcpt(CFGEXCPT_DOUBLEREPLACEMODE,
                                    ustrConfigSys);
            }
        }
        if (strhistr(pModifiers, "ADDLEFT"))
        {
            if (!fReplaceModeFound)
            {
                _iReplaceMode = CFGRPL_ADDLEFT;
                fVerticalsAllowed = TRUE;
                fReplaceModeFound = TRUE; // block future replacement modifiers
            }
            else
                // double replace mode found:
                throw BSConfigExcpt(CFGEXCPT_DOUBLEREPLACEMODE,
                                    ustrConfigSys);
        }
        if (strhistr(pModifiers, "REMOVELINE"))
        {
            if (!fReplaceModeFound)
            {
                _iReplaceMode = CFGRPL_REMOVELINE;
                fVerticalsAllowed = FALSE;
                fReplaceModeFound = TRUE; // block future replacement modifiers

                // check also for stuff after "="
                PSZ p;
                if (p = strchr(_ustrNewLine.GetBuffer(), '='))
                    _ustrUniqueSearchString2.assignUtf8(p + 1);
            }
            else
                // double replace mode found:
                throw BSConfigExcpt(CFGEXCPT_DOUBLEREPLACEMODE,
                                    ustrConfigSys);
        }
        if (strhistr(pModifiers, "REMOVEPART"))
        {
            if (!fReplaceModeFound)
            {
                _iReplaceMode = CFGRPL_REMOVEPART;
                fVerticalsAllowed = FALSE;
                fReplaceModeFound = TRUE;
            }
            else
                // double replace mode found:
                throw BSConfigExcpt(CFGEXCPT_DOUBLEREPLACEMODE,
                                    ustrConfigSys);
        }

        // now parse vertical modifiers
        BOOL fVerticalFound = FALSE;
        if (strhistr(pModifiers, "ADDTOP"))
        {
            _iVertical = CFGVRT_TOP;
            fVerticalFound = TRUE;
        }
        PSZ p2;
        if ((p2 = strhistr(pModifiers, "ADDAFTER(")))
        {
            if (!fVerticalFound)
            {
                PSZ pEndOfSearch;
                if (!(pEndOfSearch = strchr(p2, ')')))
                {
                    ustring ustr;
                    ustr.assignUtf8(p2);
                    throw BSConfigExcpt(CFGEXCPT_INVALIDSEARCH, ustr);
                }

                _ustrVerticalSearchString.assignUtf8(
                                     p2 + 9,        // strlen("ADDAFTER(")
                                     pEndOfSearch); // excluding that char

                _iVertical = CFGVRT_AFTER;
                fVerticalFound = TRUE;
            }
            else
            {
                ustring ustr;
                ustr.assignUtf8(p2);
                throw BSConfigExcpt(CFGEXCPT_INVALIDVERTICAL, ustr);
            }
        }
        if ((p2 = strhistr(pModifiers, "ADDBEFORE(")))
        {
            if (!fVerticalFound)
            {
                PSZ pEndOfSearch;
                if (!(pEndOfSearch = strchr(p2, ')')))
                {
                    ustring ustr;
                    ustr.assignUtf8(p2);
                    throw BSConfigExcpt(CFGEXCPT_INVALIDSEARCH, ustr);
                }

                _ustrVerticalSearchString.assignUtf8(p2 + 10,       // strlen("ADDBEFORE(")
                                                     pEndOfSearch); // excluding that char

                _iVertical = CFGVRT_BEFORE;
                fVerticalFound = TRUE;
            }
            else
            {
                ustring ustr;
                ustr.assignUtf8(p2);
                throw BSConfigExcpt(CFGEXCPT_INVALIDVERTICAL, ustr);
            }
        }

        // finally check if vertical modifier is allowed at all
        if ( (fVerticalFound) && (!fVerticalsAllowed) )
            throw BSConfigExcpt(CFGEXCPT_INVALIDVERTICAL, ustrConfigSys);

    } // end elseif (!pSep)
}

/*
 *@@ DescribeType:
 *      describes the current manipulator type to the GUI.
 *
 *@@added V0.9.2 (2000-02-19) [umoeller]
 */

const char* BSCfgSysManip::DescribeType()
{
    return ("CONFIG.SYS manipulation");
}

/*
 *@@ DescribeData:
 *      describes the current manipulator data to the GUI.
 *
 *@@added V0.9.2 (2000-02-19) [umoeller]
 *@@changed V0.9.5 (2000-08-26) [umoeller]: UNIQUE wasn't reported right
 *@@changed V0.9.18 (2002-03-08) [umoeller]: now returning ustring
 */

ustring BSCfgSysManip::DescribeData()
{
    ustring     str;
    BOOL        fAddSpace = TRUE;

    switch (_iReplaceMode)
    {
        case CFGRPL_UNIQUE:
            str.appendUtf8("UNIQUE");
            if (_ustrUniqueSearchString2())
            {
                str.appendUtf8("(");
                str.append(_ustrUniqueSearchString2);
                str.appendUtf8(")");
            }
        break;

        case CFGRPL_ADDLEFT: str.appendUtf8("ADDLEFT"); break;
        case CFGRPL_ADDRIGHT: str.appendUtf8("ADDRIGHT"); break;
        case CFGRPL_REMOVELINE: str.appendUtf8("REMOVELINE"); break;
        case CFGRPL_REMOVEPART: str.appendUtf8("REMOVEPART"); break;

        default:
            fAddSpace = FALSE;
        break;
    }

    if (fAddSpace)
        str.appendUtf8(" ");

    switch (_iVertical)
    {
        case CFGVRT_TOP: str.appendUtf8("ADDTOP "); break;

        case CFGVRT_BEFORE:
            str.appendUtf8("BEFORE(");
            str.append(_ustrVerticalSearchString);
            str.appendUtf8(") ");
        break;

        case CFGVRT_AFTER:
            str.appendUtf8("AFTER ");
            str.append(_ustrVerticalSearchString);
            str.appendUtf8(") ");
        break;
    }

    str.append(_ustrNewLine);

    return (str);
}

/*
 *@@ AddToUndoList:
 *      static helper method to create BSCfgSysManip
 *      instances from the logger instance that was
 *      previously used with BSConfigSys::Manipulate.
 *
 *      The new BSCfgSysManip objects are appended to
 *      the specified list and are exact opposites
 *      to the BSCfgSysManip objects that were stored
 *      in the logger. That is, if the logger registered
 *      that something was added to CONFIG.SYS, we
 *      create an object which removes that text again,
 *      and vice versa.
 *
 *      Use this method to undo CONFIG.SYS changes
 *      and pass the objects on the list to BSConfigSys::Manipulate,
 *      since there is no corresponding "undo" class
 *      for BSConfigSys.
 *
 *      See BSCfgSysManip::BSCfgSysManip for an example usage.
 *
 *      This returns the number of items created.
 *
 *      Throws:
 *      -- BSConfigExcpt.
 */

int BSCfgSysManip::AddToUndoList(list<BSConfigBase*> &List,
                                 BSCfgSysDoneLogger &logger)
{
    // the logger in this case has a list of special PSZs,
    // where the first three characters of each entry
    // signify to us what was changed in CONFIG.SYS

    PSZ     pszLogStart = logger._pabLogString,
            pszLogThis = pszLogStart;
    int     iCount = 0;

    while (pszLogThis < pszLogStart + logger._cbLogString)
    {
        ULONG   cbLogThis = strlen(pszLogThis);
        string strNewModifiers;

        // now check what we have in the log; the
        // first three characters (followed by a
        // space) signify what happened:
        //  --  "DLL":      deleted line
        //  --  "DLP":      deleted part of line
        //  --  "NWL":      added an all new line
        //  --  "NWP":      added a new part to an existing line

        if (memicmp(pszLogThis, "DLL ", 4) == 0)
        {
            // line was deleted: re-insert that line (UNIQUE mode)
            // strNewModifiers = "UNIQUE";
        }
        else if (memicmp(pszLogThis, "DLP ", 4) == 0)
        {
            // part of line was deleted: re-insert that line (ADDRIGHT mode)
            strNewModifiers = "ADDRIGHT";
        }
        else if (memicmp(pszLogThis, "NWL ", 4) == 0)
        {
            // line was added: remove that whole line
            strNewModifiers = "REMOVELINE";
                    // #### no, no, no!!! this removes an entire line...
        }
        else if (memicmp(pszLogThis, "NWP ", 4) == 0)
            // part of line was added: remove that part
            strNewModifiers = "REMOVEPART";
        else
        {
            // none of the above: error
            ustring ustr;
            ustr.assignUtf8(pszLogThis);
            throw BSConfigExcpt(CFGEXCPT_PARSELOG, ustr);
        }

        // something found: compose attribute string for manipulator
        PSZ pszNewAttrs = (PSZ)malloc(strlen(pszLogThis)
                                      + strNewModifiers.size()
                                      + 30);
        sprintf(pszNewAttrs, "%s | %s",
                pszLogThis + 4,    // stuff after "DLL " or whatever
                strNewModifiers.c_str());

        // add the undo manipulator to the _front_ of the
        // list so that items are undone in reverse order
        // (because if a line was replaced originally,
        // we first have a "delete line" and then an
        // "add line" entry in the log)
        ustring ustr;
        ustr.assignUtf8(pszNewAttrs);
        List.push_front(new BSCfgSysManip(ustr));
        free(pszNewAttrs);

        pszLogThis += cbLogThis + 1;    // go beyond null byte
        iCount++;
    }

    return (iCount);
}

/* ******************************************************************
 *
 *  BSConfigSys class
 *
 ********************************************************************/

/*
 *@@ BSConfigSys:
 *      the constructor, which loads the current CONFIG.SYS
 *      file from OS/2 boot drive into instance memory.
 *
 *      Throws:
 *      -- BSConfigExcpt.
 *
 *@@changed V0.9.7 (2001-01-15) [umoeller]: now using csysLoadConfigSys
 *@@changed WarpIN V1.0.9 (2006-02-16) [pr]: added _pszContentOld
 */

BSConfigSys::BSConfigSys() : BSRoot(tBSConfigSys)
{
    _pszContent = _pszContentOld = NULL;

    /* sprintf(_szFilename, "%c:\\config.sys", doshQueryBootDrive());
    // now read CONFIG.SYS file to initialize the dlg items
    APIRET arc = doshLoadTextFile(_szFilename, &_pszContent); */

    APIRET arc = csysLoadConfigSys(NULL,        // default CONFIG.SYS
                                   &_pszContent);

    if (arc != NO_ERROR)
        throw BSConfigExcpt(CFGEXCPT_OPEN, arc);
    else
        _pszContentOld = strhdup(_pszContent, NULL);

    _fDirty = FALSE;
}

/*
 *@@ ~ConfigSys:
 *      the destructor.
 *
 *      This does _not_ write the file. Use BSConfigSys::Flush()
 *      before deleting an instance of this.
 *
 *@@changed WarpIN V1.0.9 (2006-02-16) [pr]: added _pszContentOld
 */

BSConfigSys::~BSConfigSys()
{
    if (_pszContent)
        free(_pszContent);

    if (_pszContentOld)
        free(_pszContentOld);
}

/*
 *@@ Manipulate:
 *      this monster method changes the data in our memory copy of
 *      CONFIG.SYS according to the BSCfgSysManip object, which you must
 *      have created before. See BSCfgSysManip::BSCfgSysManip for a
 *      description of this usage.
 *
 *      This also takes a BSCfgSysDoneLogger as input where all the
 *      changes made to the CONFIG.SYS memory copy are logged. This
 *      logger can then be passed to BSConfigSys::Undo to have the
 *      changes undone again.
 *
 *      Call BSConfigSys::Flush to have CONFIG.SYS written back to disk.
 *
 *      Returns NO_ERROR if everything was OK or an error code.
 *
 *@@changed V0.9.18 (2002-03-08) [umoeller]: adjusted for Unicode; added codec
 */

int BSConfigSys::Manipulate(BSUniCodec &codecProcess, // in: codec for process codepage
                            BSCfgSysManip &Manip,       // in: manipulator object
                            BSCfgSysDoneLogger *pLogger, // in: logger object to append items to (can be NULL)
                            BSFileLogger *pLogFile)     // in: file logger (log file), can be NULL
{
    int         irc = 0;

    XSTRING     strLogger;
    xstrInit(&strLogger, 0);

    CONFIGMANIP cm;
    memset(&cm, 0, sizeof(cm));

    cm.iReplaceMode = Manip._iReplaceMode;

    string strUniqueSearchString2(&codecProcess, Manip._ustrUniqueSearchString2);
    cm.pszUniqueSearchString2 = strUniqueSearchString2.c_str();

    cm.iVertical = Manip._iVertical;

    string strVerticalSearchString(&codecProcess, Manip._ustrVerticalSearchString);
    cm.pszVerticalSearchString = strVerticalSearchString.c_str();

    string strNewLine(&codecProcess, Manip._ustrNewLine);
    cm.pszNewLine = strNewLine.c_str();

    irc = csysManipulate(&_pszContent,
                         &cm,
                         &_fDirty,
                         &strLogger);

    // now parse the logger...
    if (strLogger.ulLength)
    {
        // something to be logged:
        PSZ p = strLogger.psz;

        while (p)
        {
            PSZ pEOL;
            if (    (pEOL = strhFindEOL(p, NULL))
                 && (*pEOL)
               )
            {
                string strTemp(p, pEOL);

                if (strTemp())
                {
                    if (pLogFile)
                        // write to log file
                        pLogFile->Write("Updated CONFIG.SYS: \"%s\"",
                                        strTemp.c_str());

                    if (pLogger)
                    {
                        // write to "done" logger:
                        ustring ustr(&codecProcess, strTemp);
                        pLogger->Append(ustr);
                    }
                }
                else
                    break;

                p = pEOL + 1;
            }
            else
                break;
        }
    }

    xstrClear(&strLogger);

    return (irc);
}

/*
 *@@ Flush:
 *      this rewrites the CONFIG.SYS file on disk with the
 *      data we have in memory.
 *
 *      This makes a backup copy in "CONFIG.003" style if
 *      (pszBackup != NULL). See doshCreateBackupFileName
 *      for details.
 *
 *      Returns:
 *
 *      --  0: contents were dirty, written back to disk.
 *
 *      --  1: no error, but contents were clean, so no write-back
 *             was needed.
 *
 *      Throws:
 *      -- BSConfigExcpt(CFGEXCPT_WRITE, APIRET).
 *
 *@@changed V0.9.3 (2000-05-03) [umoeller]: added more error checking
 *@@changed V0.9.3 (2000-05-12) [umoeller]: added pszBackup
 *@@changed V0.9.6 (2000-10-27) [umoeller]: added check if contents are dirty
 *@@changed WarpIN V1.0.9 (2006-02-16) [pr]: added _pszContentOld
 */

int BSConfigSys::Flush(string *pstrBackup,          // in/out: create backup?
                       BSFileLogger *pLogFile)      // in: file logger (log file), can be NULL
                 const
{
    int irc = 0;

    // WarpIN V1.0.9 (2006-02-16) [pr]: Check for content modification @@fixes 269
    if (   _fDirty
        && strhcmp (_pszContent, _pszContentOld)
       )
    {
        PSZ pszBackup = NULL;
        CHAR szBackup[CCHMAXPATH];
        if (pstrBackup)
            pszBackup = szBackup;

        APIRET arc = csysWriteConfigSys(NULL,       // default _szFilename,
                                        _pszContent,
                                        pszBackup);        // create backup

        if (arc)
        {
            if (pLogFile)
                pLogFile->Write("Error %d occurred writing CONFIG.SYS back to disk",
                                arc);

            throw BSConfigExcpt(CFGEXCPT_WRITE, arc);
        }

        if (pLogFile)
            pLogFile->Write("CONFIG.SYS file written back to disk, backup is \"%s\"",
                            pszBackup);

        if (pstrBackup)
            pstrBackup->assign(szBackup);
    }
    else
        irc = 1;

    return (irc);
}

/* ******************************************************************
 *
 *  BSRegisterClass class
 *
 ********************************************************************/

/*
 *@@ BSRegisterClass:
 *      this constructor translates a REGISTERCLASS attribute (as used
 *      in the PCK tag and stored in the database) into the BSRegisterClass
 *      instance data.
 *
 *      Syntax:
 *
 +          REGISTERCLASS="classname|dllpath"
 *
 *      Throws:
 *      -- BSConfigExcpt.
 */

BSRegisterClass::BSRegisterClass(const ustring &ustrRegisterClass)
                 : BSConfigBase(CFGT_REGISTERCLASS, tBSRegisterClass)
{
    // find separator
    PCSZ pcszRegisterClass = ustrRegisterClass.GetBuffer();
    PCSZ pBegin;
    if (!(pBegin = pcszRegisterClass))
        throw BSConfigExcpt(REGEXCPT_SYNTAX, ustrRegisterClass);

    PCSZ pEnd;
    if (!(pEnd = strchr(pBegin, '|')))
        throw BSConfigExcpt(REGEXCPT_SYNTAX, ustrRegisterClass);

    // strip trailing spaces
    PCSZ pEnd2 = pEnd;
    while ((*pEnd2 == ' ') && (pEnd2 > pcszRegisterClass))
        pEnd2--;

    _ustrClassName.assignUtf8(pBegin, pEnd2);
    if (!_ustrClassName())
        throw BSConfigExcpt(REGEXCPT_SYNTAX, ustrRegisterClass);

    pBegin = pEnd + 1;
    // strip leading spaces
    while ((*pBegin) && (*pBegin == ' '))
        pBegin++;
    _ustrModule.assignUtf8(pBegin);
    if (!_ustrModule())
        throw BSConfigExcpt(REGEXCPT_SYNTAX, ustrRegisterClass);
}

/*
 *@@ DescribeType:
 *      describes the current manipulator type to the GUI.
 *
 *@@added V0.9.9 (2001-03-27) [umoeller]
 */

const char* BSRegisterClass::DescribeType()
{
    return ("WPS class registration");
}

/*
 *@@ DescribeData:
 *      describes the current manipulator data to the GUI.
 *
 *@@added V0.9.9 (2001-03-27) [umoeller]
 *@@changed V0.9.18 (2002-03-08) [umoeller]: now returning ustring
 */

ustring BSRegisterClass::DescribeData()
{
    ustring    str = _ustrClassName;
    str.appendUtf8(" in ");
    str += _ustrModule;

    return (str);
}

/*
 *@@ IsRegistered:
 *      this returns TRUE if a class of the same WPS class name
 *      as this instance is already registered with the WPS.
 *      In that case, the module of the registered class is
 *      copied into strModule.
 *
 *      This throws a BSConfigExcpt with REGEXCPT_QUERYCLASSLIST
 *      if the class list could not be queried.
 *
 *@@changed V0.9.18 (2002-03-08) [umoeller]: now using string for buffer, added codec
 */

bool BSRegisterClass::IsRegistered(BSUniCodec &codecProcess, // in: codec for process codepage
                                   ustring &ustrModule)
                      const
{
    BOOL    fInstalled = FALSE;
    PBYTE   pClassList,
            pClassThis = 0;
    if (pClassList = winhQueryWPSClassList())
    {
        string strClassName(&codecProcess, _ustrClassName);
        if ((pClassThis = winhQueryWPSClass(pClassList, strClassName.c_str())))
        {
            fInstalled = TRUE;
            ustrModule.assignCP(&codecProcess, ((POBJCLASS)pClassThis)->pszModName);
        }

        free(pClassList);
    }
    else
        throw BSConfigExcpt(REGEXCPT_QUERYCLASSLIST, 0);

    return (fInstalled);
}

/*
 *@@ Register:
 *      this attempts to register the class.
 *
 *      If (fReplace == TRUE), we do not call
 *      BSRegisterClass::IsRegistered before registering
 *      this, i.e. we will always register the class,
 *      even if it's already registered.
 *
 *      If (fReplace == FALSE) and BSRegisterClass::IsRegistered
 *      returned TRUE, we throw a BSConfigExcpt with
 *      REGEXCPT_ALREADYREGISTERED and pszSubstr set to the
 *      file name of the registered DLL. Note that IsRegistered might
 *      in turn throw BSConfigExcpt with REGEXCPT_QUERYCLASSLIST.
 *
 *      If registering the class failed, this throws a
 *      BSConfigExcpt with REGEXCPT_REGISTER and iData set
 *      to the APIRET of winhRegisterClass.
 *
 *      This method can take a BSDoneLoggerBase object as input, which
 *      can later be used with the BSDeregisterClass::AddToUndoList
 *      static class method to easily create a list of BSDeregisterClass
 *      objects to undo the changes made.
 *
 *      Example usage (exception handling omitted):
 +          BSRegisterDoneLogger logger;
 +                      // create logger instance
 +          BSRegisterClass RegClass("XFolder|C:\XFOLDER\XFLDR.DLL");
 +                      // create BSRegisterClass instance:
 +          RegClass.Register(TRUE, logger);
 +          ... // register more classes with the same logger
 +
 +          // now create undo list
 +          list<BSDeregisterClass*> List;
 +          BSDeregisterClass::AddToUndoList(List, logger);
 +          list<BSDeregisterClass*>::iterator DeregStart = List.begin(),
 +                                             DeregEnd = List.end();
 +          for (; DeregStart != DeregEnd; DeregStart++)
 +              (**DeregStart).Deregister;
 *
 *@@changed V0.9.18 (2002-03-08) [umoeller]: added codec
 *@@changed V0.9.18 (2002-03-08) [umoeller]: added missing log file entry
 */

int BSRegisterClass::Register(BSUniCodec &codecProcess, // in: codec for process codepage
                              bool fReplace,
                              BSRegisterDoneLogger *pLogger, // in: logger object to append items to (can be NULL)
                              BSFileLogger *pLogFile)     // in: file logger (log file), can be NULL
                     const
{
    int irc = 0;

    if (fReplace == FALSE)
    {
        ustring ustrModule;
        if (IsRegistered(codecProcess, ustrModule))
            throw BSConfigExcpt(REGEXCPT_ALREADYREGISTERED, ustrModule);
    }

    CONVERT(&codecProcess, ClassName);
    CONVERT(&codecProcess, Module);

    CHAR   szBuf[1000];
    APIRET arc = winhRegisterClass(pcszClassName,
                                   pcszModule,
                                   szBuf,
                                   sizeof(szBuf));
    // always record in logger, even if failed,
    // because the class is in the class list anyway
    if (pLogger)
        pLogger->Append(_ustrClassName);

    if (arc != NO_ERROR)
    {
        // this was missing V0.9.18 (2002-03-08) [umoeller]
        if (pLogFile)
            pLogFile->Write("Error %d registering WPS class \"%s\"",
                            arc,
                            pcszClassName);

        throw BSConfigExcpt(REGEXCPT_REGISTER, arc);
    }

    // this was missing V0.9.18 (2002-03-08) [umoeller]
    if (pLogFile)
        pLogFile->Write("Registered WPS class \"%s\"",
                        pcszClassName);

    return (irc);
}

/* ******************************************************************
 *
 *  BSDeregisterClass class
 *
 ********************************************************************/

/*
 *@@ BSDeregisterClass:
 *      the constructor, which in this case takes a simple
 *      PSZ class name as input.
 */

BSDeregisterClass::BSDeregisterClass(const ustring &ustrDeregisterClass)
                   : BSConfigBase(CFGT_DEREGISTERCLASS, tBSDeregisterClass)
{
    _ustrClassName = ustrDeregisterClass;
}

/*
 *@@ DescribeType:
 *      describes the current manipulator type to the GUI.
 *
 *@@added V0.9.2 (2000-02-19) [umoeller]
 */

const char* BSDeregisterClass::DescribeType()
{
    return ("WPS class deregistration");
}

/*
 *@@ DescribeData:
 *      describes the current manipulator data to the GUI.
 *
 *@@added V0.9.2 (2000-02-19) [umoeller]
 *@@changed V0.9.18 (2002-03-08) [umoeller]: now returning ustring
 */

ustring BSDeregisterClass::DescribeData()
{
    return (_ustrClassName);
}

/*
 *@@ Deregister:
 *      this attempts to deregister the class.
 *
 *      Throws:
 *      -- BSConfigExcpt.
 *
 *@@changed V0.9.18 (2002-03-08) [umoeller]: added codec
 */

int BSDeregisterClass::Deregister(BSUniCodec &codecProcess, // in: codec for process codepage
                                  BSFileLogger *pLogFile)       // in: file logger (log file), can be NULL
                       const
{
    int irc = 0;

    CONVERT(&codecProcess, ClassName);      // creates local string strXXX, PCSZ pcszXXX

    if (!WinDeregisterObjectClass(pcszClassName))
    {
        if (pLogFile)
            pLogFile->Write("An error occurred deregistering WPS class \"%s\"",
                            pcszClassName);

        throw BSConfigExcpt(REGEXCPT_DEREGISTER, _ustrClassName);
    }

    if (pLogFile)
        pLogFile->Write("Deregistered WPS class \"%s\"",
                        pcszClassName);

    return (irc);
}

/*
 *@@ AddToUndoList:
 *      static helper method to create BSDeregisterClass
 *      instances from the logger instance that was
 *      previously used with BSRegisterClass::Register.
 *
 *      The new BSDeregisterClass objects are appended to
 *      the specified list.
 *
 *      See BSRegisterClass::Register for an example usage.
 *
 *      This returns the number of items created.
 */

int BSDeregisterClass::AddToUndoList(list<BSConfigBase*> &List,
                                     BSRegisterDoneLogger &logger)
{
    // the logger in this case has a simple list of PSZs with all
    // the class names that were registered, each of which is terminated
    // with a null character

    PSZ     pszLogStart = logger._pabLogString,
            pszLogThis = pszLogStart;
    int     iCount = 0;

    while (pszLogThis < pszLogStart + logger._cbLogString)
    {
        ULONG cbLogThis = strlen(pszLogThis);
        ustring ustr;
        ustr.assignUtf8(pszLogThis);
        List.push_back(new BSDeregisterClass(ustr));
        pszLogThis += cbLogThis + 1;    // go beyond null byte
        iCount++;
    }

    return (iCount);
}

/* ******************************************************************
 *
 *  BSReplaceClass class
 *
 ********************************************************************/

/*
 *@@ BSReplaceClassBase:
 *      this constructor translates a REPLACECLASS attribute (as used
 *      in the PCK tag and stored in the database) into the BSReplaceClass
 *      instance data.
 *
 *      Syntax:
 *
 +          REPLACECLASS="oldclassname|newclassname"
 *
 *@@changed V0.9.19 (2002-05-07) [umoeller]: changed inheritance hierarchy here
 */

BSReplaceClassBase::BSReplaceClassBase(const ustring &ustrReplaceClass,
                                       ULONG cfgt,
                                       BSClassID &Class)
        : BSConfigBase(cfgt, Class)
{
    PCSZ pcszReplaceClass = ustrReplaceClass.GetBuffer();

    // find separator
    PCSZ pBegin;
    if (!(pBegin = pcszReplaceClass))
        throw BSConfigExcpt(REGEXCPT_SYNTAX, ustrReplaceClass);

    PCSZ pEnd;
    if (!(pEnd = strchr(pBegin, '|')))
        throw BSConfigExcpt(REGEXCPT_SYNTAX, ustrReplaceClass);

    // strip trailing spaces
    PCSZ pEnd2 = pEnd;
    while ((*pEnd2 == ' ') && (pEnd2 > pcszReplaceClass))
        pEnd2--;

    _ustrOldClassName.assignUtf8(pBegin, pEnd2);
    if (!_ustrOldClassName())
        throw BSConfigExcpt(REGEXCPT_SYNTAX, ustrReplaceClass);

    pBegin = pEnd + 1;
    // strip leading spaces
    while ((*pBegin) && (*pBegin == ' '))
        pBegin++;
    _ustrNewClassName.assignUtf8(pBegin);
    if (!_ustrNewClassName())
        throw BSConfigExcpt(REGEXCPT_SYNTAX, ustrReplaceClass);
}

/*
 *@@ DescribeType:
 *      describes the current manipulator type to the GUI.
 *
 *@@added V0.9.2 (2000-02-19) [umoeller]
 */

const char* BSReplaceClass::DescribeType()
{
    return ("WPS class replacement");
}

/*
 *@@ DescribeData:
 *      describes the current manipulator data to the GUI.
 *
 *@@added V0.9.2 (2000-02-19) [umoeller]
 *@@changed V0.9.5 (2000-08-26) [umoeller]: UNIQUE wasn't reported right
 *@@changed V0.9.18 (2002-03-08) [umoeller]: now returning ustring
 */

ustring BSReplaceClass::DescribeData()
{
    ustring    str = _ustrOldClassName;
    str.appendUtf8(" with ");
    str += _ustrNewClassName;

    return (str);
}

/*
 *@@ Replace:
 *      this attempts to replace the class or undo
 *      an existing replacement (if fReplace == FALSE).
 *
 *      If replacing the class failed, this throws a
 *      BSConfigExcpt with REGEXCPT_REPLACE.
 *
 *      Example usage (exception handling omitted):
 +          BSReplaceDoneLogger logger;
 +                      // create logger instance
 +          BSReplaceClass ReplClass("WPFolder;XFolder");
 +                      // create BSReplaceClass instance:
 +          ReplClass.Replace(TRUE, logger);
 +          ... // replace more classes with the same logger
 +
 +          // now create undo list
 +          list<BSUnreplaceClass*> List;
 +          BSUnreplaceClass::AddToUndoList(List, logger);
 +          list<BSUnreplaceClass*>::iterator UnrplStart = List.begin(),
 +                                            UnrplEnd = List.end();
 +          for (; UnrplStart != UnrplEnd; UnrplStart++)
 +              (**UnrplStart).Unreplace;
 *
 *@@changed V0.9.1 (2000-01-05) [umoeller]: finally added logging, which was missing
 *@@changed V0.9.9 (2001-03-30) [umoeller]: fixed wrong logging if replace failed
 *@@changed V0.9.18 (2002-03-08) [umoeller]: added codec
 */

int BSReplaceClass::Replace(BSUniCodec &codecProcess, // in: codec for process codepage
                            BSReplaceDoneLogger *pLogger,   // in: logger object to append items to (can be NULL)
                            BSFileLogger *pLogFile)     // in: file logger (log file), can be NULL
                    const
{
    int irc = 0;

    CONVERT(&codecProcess, OldClassName);
    CONVERT(&codecProcess, NewClassName);

    BOOL brc = WinReplaceObjectClass(pcszOldClassName,
                                     pcszNewClassName,
                                     TRUE);       // replace
    // always record in logger, even if failed,
    // because the class is in the class list anyway
    // V0.9.9 (2001-03-30) [umoeller]
    if (pLogger)
    {
        ustring ustr2Append(_ustrOldClassName);
        ustr2Append.appendUtf8("|");
        ustr2Append += _ustrNewClassName;
        pLogger->Append(ustr2Append);
    }


    if (!brc)
    {
        if (pLogFile)
            pLogFile->Write("An error occurred replacing WPS class \"%s\" with \"%s\"",
                            pcszOldClassName,
                            pcszNewClassName);

        throw BSConfigExcpt(REGEXCPT_REPLACE, 0);
    }

    // success:
    if (pLogFile)
        pLogFile->Write("Replaced WPS class \"%s\" with \"%s\"",
                         pcszOldClassName,
                         pcszNewClassName);

    return (irc);
}

/* ******************************************************************
 *
 *  BSUnreplaceClass class
 *
 ********************************************************************/

/*
 *@@ DescribeType:
 *      describes the current manipulator type to the GUI.
 *
 *@@added V0.9.2 (2000-02-19) [umoeller]
 */

const char* BSUnreplaceClass::DescribeType()
{
    return ("WPS class un-replacement");
}

/*
 *@@ DescribeData:
 *      describes the current manipulator data to the GUI.
 *
 *@@added V0.9.2 (2000-02-19) [umoeller]
 *@@changed V0.9.18 (2002-03-08) [umoeller]: now returning ustring
 */

ustring BSUnreplaceClass::DescribeData()
{
    ustring    ustr = _ustrOldClassName;
    ustr.appendUtf8(" with ");
    ustr += _ustrNewClassName;

    return (ustr);
}

/*
 *@@ Unreplace:
 *      this attempts to unreplace the class.
 *      Throws a BSConfigExcpt upon failure.
 *
 *@@changed V0.9.18 (2002-03-08) [umoeller]: added codec
 */

int BSUnreplaceClass::Unreplace(BSUniCodec &codecProcess, // in: codec for process codepage,
                                BSFileLogger *pLogFile) // in: file logger (log file), can be NULL
                      const
{
    int irc = 0;

    CONVERT(&codecProcess, OldClassName);
    CONVERT(&codecProcess, NewClassName);

    if (!WinReplaceObjectClass(pcszOldClassName,
                               pcszNewClassName,
                               FALSE))
    {
        if (pLogFile)
            pLogFile->Write("Error undoing WPS class replacement of \"%s\" with \"%s\"",
                             pcszOldClassName,
                             pcszNewClassName);

        throw BSConfigExcpt(REGEXCPT_UNREPLACE, 0);
    }

    // success:
    if (pLogFile)
        pLogFile->Write("Undid WPS class replacement of \"%s\" with \"%s\"",
                         pcszOldClassName,
                         pcszNewClassName);

    return (irc);
}

/*
 *@@ AddToUndoList:
 *      static helper method to create BSUnreplaceClass
 *      instances from the logger instance that was
 *      previously used with BSRegisterClass::Register.
 *
 *      The new BSUnreplaceClass objects are appended to
 *      the specified list.
 *
 *      See BSReplaceClass::Replace for an example usage.
 *
 *      This returns the number of items created.
 */

int BSUnreplaceClass::AddToUndoList(list<BSConfigBase*> &List,
                                    BSReplaceDoneLogger &logger)
{
    // the logger in this case has a simple list of PSZs with all
    // the class names that were registered, each of which is terminated
    // with a null character

    PSZ     pszLogStart = logger._pabLogString,
            pszLogThis = pszLogStart;
    int     iCount = 0;

    while (pszLogThis < pszLogStart + logger._cbLogString)
    {
        ULONG cbLogThis = strlen(pszLogThis);
        ustring ustr;
        ustr.assignUtf8(pszLogThis);
        List.push_back(new BSUnreplaceClass(ustr));
                                    // this calls the inherited BSReplaceClass
                                    // constructor, which will parse the thing
        pszLogThis += cbLogThis + 1;    // go beyond null byte
        iCount++;
    }

    return (iCount);
}

/* ******************************************************************
 *
 *  BSCreateWPSObject class
 *
 ********************************************************************/

/*
 *@@ BSCreateWPSObject:
 *      this constructor translates a CREATEOBJECT attribute (as used
 *      in the PCK tag and stored in the database) into the BSCreateWPSObject
 *      instance data.
 *
 *      Syntax:
 *
 +          CREATEOBJECT="[REPLACE] classname|title|location[|config]]"
 +          CREATEOBJECT="[FAIL] classname|title|location[|config]]"
 *
 *      Throws:
 *      -- BSConfigExcpt.
 *
 *@@changed V0.9.3 (2000-04-08) [umoeller]: "REPLACE" wasn't evaluated; fixed. Thanks, Cornelis Bockemhl.
 *@@changed V0.9.15 (2001-08-26) [umoeller]: added WPOEXCPT_INVALIDLOCATION checks
 *@@changed V0.9.19 (2002-04-14) [umoeller]: REPLACE was never detected, fixed
 *@@changed V1.0.14 (2006-12-03) [pr]: added FAIL mode @@fixes 893
 */

BSCreateWPSObject::BSCreateWPSObject(const ustring &ustrCreateObject)
                   : BSConfigBase(CFGT_CREATEOBJECT, tBSCreateWPSObject)
{
    PCSZ pcszCreateObject = ustrCreateObject.GetBuffer();

    // extract class name
    PCSZ pBegin;
    if (!(pBegin = pcszCreateObject))
        throw BSConfigExcpt(WPOEXCPT_NOCLASS, ustrCreateObject);

    if (!memcmp(pBegin, "REPLACE ", 8))
    {
        _ulFlags = CO_REPLACEIFEXISTS;  // V1.0.14 (2006-12-03) [pr]
        pBegin += 8;
        while (*pBegin == ' ')
            pBegin++;
    }
    else
        if (!memcmp(pBegin, "FAIL ", 5))  // V1.0.14 (2006-12-03) [pr]
        {
            _ulFlags = CO_FAILIFEXISTS;
            pBegin += 5;
            while (*pBegin == ' ')
                pBegin++;
        }
        else
            _ulFlags = CO_UPDATEIFEXISTS;  // V1.0.14 (2006-12-03) [pr]

    PSZ pEnd;
    if (pEnd = strchr(pBegin, '|'))
    {
        _ustrClassName.assignUtf8(pBegin, pEnd);

        // extract title
        pBegin = pEnd + 1;
        if (pEnd = strchr(pBegin, '|'))
        {
            _ustrTitle.assignUtf8(pBegin, pEnd);

            // extract location
            pBegin = pEnd + 1;
            if (pEnd = strchr(pBegin, '|'))
            {
                // yet another separator found: we then have a config parameter
                _ustrLocation.assignUtf8(pBegin, pEnd);

                // extract config (rest of pszCreateObject)
                pBegin = pEnd + 1;
                _ustrSetupString.assignUtf8(pBegin);
            }
            else
                // no separator after "location" found:
                // duplicate whole string
                _ustrLocation.assignUtf8(pBegin);

            if (!_ustrLocation())
                throw BSConfigExcpt(WPOEXCPT_NOLOCATION, ustrCreateObject);

            // check if the location is correctly in <> brackets
            // V0.9.15 (2001-08-26) [umoeller]
            PCSZ pcszLocation = _ustrLocation.GetBuffer();
            if (    (pcszLocation[0] == '<')
                 && (pcszLocation[_ustrLocation.length() - 1] != '>')
               )
                throw BSConfigExcpt(WPOEXCPT_INVALIDLOCATIONSTRING, ustrCreateObject);
        }
        else
            throw BSConfigExcpt(WPOEXCPT_NOTITLE, ustrCreateObject);
    }
    else
        throw BSConfigExcpt(WPOEXCPT_NOCLASS, ustrCreateObject);
}

/*
 *@@ DescribeType:
 *      describes the current manipulator type to the GUI.
 *
 *@@added V0.9.9 (2001-03-27) [umoeller]
 */

const char* BSCreateWPSObject::DescribeType()
{
    return ("WPS object");
}

/*
 *@@ DescribeData:
 *      describes the current manipulator data to the GUI.
 *
 *@@added V0.9.9 (2001-03-27) [umoeller]
 *@@changed V0.9.18 (2002-03-08) [umoeller]: now returning ustring
 */

ustring BSCreateWPSObject::DescribeData()
{
    ustring    ustr = _ustrClassName;
    ustr.appendUtf8(" ");
    ustr += _ustrTitle;
    ustr.appendUtf8(" ");
    ustr += _ustrLocation;
    ustr.appendUtf8(" ");
    ustr += _ustrSetupString;

    return (ustr);
}

/*
 *@@ CreateObject:
 *      this attempts to create the WPS object
 *      with the instance data.
 *
 *      Throws a BSConfigExcpt with WPOEXCPT_CREATE
 *      if this fails.
 *
 *@@changed V0.9.18 (2002-03-08) [umoeller]: added codec
 *@@changed V1.0.14 (2006-12-03) [pr]: added FAIL mode @@fixes 893
 */

void BSCreateWPSObject::CreateObject(BSUniCodec &codecProcess, // in: codec for process codepage,
                                     BSWPSObjectsDoneLogger *pLogger, // in: logger object to append items to (can be NULL)
                                     BSFileLogger *pLogFile)    // in: file logger (log file), can be NULL
{
    CONVERT(&codecProcess, ClassName);  // creates local string strXXX, PCSZ pcszXXX
    CONVERT(&codecProcess, Title);      // creates local string strXXX, PCSZ pcszXXX
    CONVERT(&codecProcess, SetupString); // creates local string strXXX, PCSZ pcszXXX
    CONVERT(&codecProcess, Location);   // creates local string strXXX, PCSZ pcszXXX

    // check if the target folder exists, this is the most common problem
    // if WinCreateObject fails
    // V0.9.19 (2002-06-15) [umoeller]

    if (!WinQueryObject(pcszLocation))
    {
        if (pLogFile)
            pLogFile->Write("Error creating WPS object \"%s\", class \"%s\", location \"%s\", setup \"%s\": Invalid target folder",
                             pcszTitle,
                             pcszClassName,
                             pcszLocation,
                             pcszSetupString);

        throw BSConfigExcpt(WPOEXCPT_CREATE_BADLOCATION, _ustrTitle);
    }

    if (!(_hobj = WinCreateObject(pcszClassName,
                                  pcszTitle,
                                  pcszSetupString,
                                  pcszLocation,
                                  _ulFlags)))
    {
        if (_ulFlags == CO_FAILIFEXISTS)  // V1.0.14 (2006-12-03) [pr]
        {
            if (pLogFile)
                pLogFile->Write("WPS object already exists \"%s\", class \"%s\", location \"%s\", setup \"%s\"",
                                pcszTitle,
                                pcszClassName,
                                pcszLocation,
                                pcszSetupString);
        }
        else
        {
            if (pLogFile)
                pLogFile->Write("Error creating WPS object \"%s\", class \"%s\", location \"%s\", setup \"%s\"",
                                pcszTitle,
                                pcszClassName,
                                pcszLocation,
                                pcszSetupString);

            throw BSConfigExcpt(WPOEXCPT_CREATE, _ustrTitle);
        }
    }
    else
    {
        // V1.0.5 (2005-02-17) [pr]
        // We now save the object synchronously to prevent objects getting lost if we
        // restart the WPS imminently due to a class registration for example. @@fixes 634.
        WinSaveObject (_hobj, FALSE);
        if (pLogFile)
        {
            pLogFile->Write("Created WPS object \"%s\", class \"%s\", location \"%s\", setup \"%s\"",
                            pcszTitle,
                            pcszClassName,
                            pcszLocation,
                            pcszSetupString);
            pLogFile->Write("HOBJECT is 0x%lX", _hobj);
        }
    }

    // store the object ID in the logger
    PCSZ pObjectID;
    if (    // do we have a setup string at all?
            (pObjectID = _ustrSetupString.GetBuffer())
            // does it contain an object ID?
         && (pObjectID = strhistr(pObjectID,
                                  "OBJECTID=<"))
       )
    {
        PCSZ pBegin = pObjectID + 9;     // points to '<' now
        PCSZ pEnd;
        if (pEnd = strchr(pBegin, '>'))
        {
            ustring ustrObjectID;
            ustrObjectID.assignUtf8(pBegin, pEnd + 1);
            if (pLogger)
                pLogger->Append(ustrObjectID);
        }
    }
}

/* ******************************************************************
 *
 *  BSDeleteWPSObject class
 *
 ********************************************************************/

/*
 *@@ BSDeleteWPSObject:
 *      the constructor, which in this case takes a simple
 *      PSZ object ID as input.
 */

BSDeleteWPSObject::BSDeleteWPSObject(const ustring &ustrID2Delete)
                   : BSConfigBase(CFGT_DELETEOBJECT, tBSDeleteWPSObject)
{
    _ustrObjectID = ustrID2Delete;
}

/*
 *@@ DescribeType:
 *      describes the current manipulator type to the GUI.
 *
 *@@added V0.9.2 (2000-02-19) [umoeller]
 */

const char* BSDeleteWPSObject::DescribeType()
{
    return ("WPS object deletion");
}

/*
 *@@ DescribeData:
 *      describes the current manipulator data to the GUI.
 *
 *@@added V0.9.2 (2000-02-19) [umoeller]
 *@@changed V0.9.18 (2002-03-08) [umoeller]: now returning ustring
 */

ustring BSDeleteWPSObject::DescribeData()
{
    return (_ustrObjectID);
}

/*
 *@@ Delete:
 *      this attempts to delete the object.
 *      Throws a BSConfigExcpt upon failure.
 *
 *@@changed V0.9.18 (2002-03-08) [umoeller]: added codec
 *@@changed V1.0.18 (2008-09-27) [pr]: no error if object no longer exists @@fixes 1062
 */

int BSDeleteWPSObject::Delete(BSUniCodec &codecProcess,
                              BSFileLogger *pLogFile)   // in: file logger (log file), can be NULL
                       const
{
    int irc = 0;

    CONVERT(&codecProcess, ObjectID);  // creates local string strXXX, PCSZ pcszXXX

    HOBJECT     hobj;
    if (!(hobj = WinQueryObject(pcszObjectID)))
    {
        if (pLogFile)
            pLogFile->Write("Error locating WPS object \"%s\"",
                             pcszObjectID);
    }
    else
        if (!WinDestroyObject(hobj))
        {
            if (pLogFile)
                pLogFile->Write("Error deleting WPS object \"%s\", HOBJECT was 0x%lX",
                                 pcszObjectID,
                                 hobj);

            throw BSConfigExcpt(WPOEXCPT_DELETEOBJECT, 0);
        }

    return (irc);
}

/*
 *@@ AddToUndoList:
 *      static helper method to create BSDeleteWPSObject
 *      instances from the logger instance that was
 *      previously used with BSRegisterClass::Register.
 *
 *      The new BSDeleteWPSObject objects are appended to
 *      the specified list.
 *
 *      This returns the number of items created.
 */

int BSDeleteWPSObject::AddToUndoList(list<BSConfigBase*> &List,
                                     BSWPSObjectsDoneLogger &logger)
{
    // the logger in this case has a simple list of PSZs with all
    // the object IDs that were created, each of which is terminated
    // with a null character

    PSZ     pszLogStart = logger._pabLogString,
            pszLogThis = pszLogStart;
    int     iCount = 0;

    while (pszLogThis < pszLogStart + logger._cbLogString)
    {
        ULONG cbLogThis = strlen(pszLogThis);
        ustring ustr;
        ustr.assignUtf8(pszLogThis);
        List.push_back(new BSDeleteWPSObject(ustr));
        pszLogThis += cbLogThis + 1;    // go beyond null byte
        iCount++;
    }

    return (iCount);
}

/* ******************************************************************
 *
 *  BSClearProfile class
 *
 ********************************************************************/

/*
 *@@ DescribeType:
 *      describes the current manipulator type to the GUI.
 *
 *@@added V0.9.2 (2000-02-19) [umoeller]
 */

const char* BSProfileBase::DescribeType()
{
    return ("Profile data");
}

/*
 *@@ DescribeData:
 *      describes the current manipulator data to the GUI.
 *
 *@@added V0.9.2 (2000-02-19) [umoeller]
 *@@changed V0.9.18 (2002-03-08) [umoeller]: now returning ustring
 */

ustring BSProfileBase::DescribeData()
{
    return (DescribePrfKey());
}

/*
 *@@ DescribePrfKey:
 *      returns a descriptive string describing the
 *      member fields, a la "USER\app\key".
 *
 *@@added V0.9.1 (2000-02-12) [umoeller]
 */

ustring BSProfileBase::DescribePrfKey() const
{
    ustring str(_ustrProfile);
    if (_ustrApplication())
    {
        str.appendUtf8("\\");
        str += _ustrApplication;

        if (_ustrKey())
        {
            str.appendUtf8("\\");
            str += _ustrKey;
        }
    }

    return (str);
}

/*
 *@@ DescribePrfKey:
 *
 *@@added V0.9.18 (2002-03-08) [umoeller]
 */

string BSProfileBase::DescribePrfKey(BSUniCodec &codecProcess) const
{
    string str(&codecProcess, DescribePrfKey());
    return str;
}

/*
 *@@ BSClearProfile:
 *
 *      Syntax:
 *
 +          CLEARPROFILE="profile\application[\key]"
 *
 *@@added V0.9.1 (2000-02-07) [umoeller]
 *@@changed V1.0.5 (2005-03-06) [pr]: Improved parsing of profiles with path names. @@fixes 633
 */

BSClearProfile::BSClearProfile(const ustring &ustrClearProfile)
                : BSProfileBase(CFGT_CLEARPROFILE, tBSClearProfile)
{
    PCSZ pcszClearProfile = ustrClearProfile.GetBuffer();

    // get profile
    PCSZ pFirstBackslash;
    if (pFirstBackslash = strchr(pcszClearProfile, '\\'))
    {
        // extract profile
        _ustrProfile.assignUtf8(pcszClearProfile, pFirstBackslash);

        // V1.0.7 (2005-05-15) [pr]: Changed to case-insensitive compare as there are
        // a few broken packages out there
        if (   _ustrProfile.compareUtf8I("USER")
            && _ustrProfile.compareUtf8I("SYSTEM")
           )
        {
            while (pFirstBackslash)
            {
                ULONG cbProfile = pFirstBackslash - pcszClearProfile;
                if (   (cbProfile > 4)
                    && (!strnicmp(pFirstBackslash - 4, ".INI", 4))
                   )
                {
                    _ustrProfile.assignUtf8(pcszClearProfile, pFirstBackslash);
                    break;
                }

                pFirstBackslash = strchr(pFirstBackslash + 1, '\\');
            }
        }
    }

    if (pFirstBackslash)
    {
        // get application
        PCSZ pSecondBackslash;
        if (pSecondBackslash = strchr(pFirstBackslash + 2, '\\'))
        {
            // key specified:
            // extract application up to key
            _ustrApplication.assignUtf8(pFirstBackslash + 1, pSecondBackslash);

            // extract key (all the rest)
            _ustrKey.assignUtf8(pSecondBackslash + 1);
        }
        else
            // key not specified:
            _ustrApplication.assignUtf8(pFirstBackslash + 1);
    }

    if (!_ustrApplication())
        // any error:
        throw BSConfigExcpt(PRFEXCPT_SYNTAX, ustrClearProfile);
}

/*
 *@@ Clear:
 *      deletes the corresponding profile entry.
 *
 *@@added V0.9.5 (2000-08-26) [umoeller]
 *@@changed V0.9.18 (2002-03-08) [umoeller]: added codec
 *@@changed WarpIN V1.0.11 (2006-08-29) [pr]: fix entire app. delete @@fixes 833
 */

int BSClearProfile::Clear(BSUniCodec &codecProcess, // in: codec for process codepage,
                          HAB hab,
                          BSFileLogger *pLogFile)   // in: file logger (log file), can be NULL
    const
{
    BOOL    fOK = FALSE;
    HINI    hini = NULLHANDLE,
            hini2Close = NULLHANDLE;

    if (!_ustrProfile.compareUtf8("USER"))
        hini = HINI_USER;
    else if (!_ustrProfile.compareUtf8("SYSTEM"))
        hini = HINI_SYSTEM;
    else
    {
        CONVERT(&codecProcess, Profile);
        hini = PrfOpenProfile(hab,
                              pcszProfile);
        hini2Close = hini;
    }

    if (hini)
    {
        CONVERT(&codecProcess, Application);
        CONVERT(&codecProcess, Key);

        ULONG cb = 0;

        // WarpIN V1.0.11 (2006-08-29) [pr]: fix entire app. delete @@fixes 833
        if (strlen(pcszKey) == 0)
            pcszKey = NULL;

        // does key exist?
        if (PrfQueryProfileSize(hini,
                                pcszApplication,
                                pcszKey,                // can be NULL
                                &cb))
        {
            // key exists:
            if (PrfWriteProfileString(hini,
                                      pcszApplication,
                                      pcszKey,          // can be NULL
                                      NULL))
                fOK = TRUE;
        }
        else
            // key does not exist:
            fOK = TRUE;
    }

    if (!fOK)
    {
        string strClear = DescribePrfKey(codecProcess);
        if (pLogFile)
            pLogFile->Write("Error deleting profile key \"%s\"",
                             strClear.c_str());

        // error:
        throw BSConfigExcpt(PRFEXCPT_PRFERROR, DescribePrfKey());
    }

    if (pLogFile)
    {
        string strClear = DescribePrfKey(codecProcess);
        pLogFile->Write("Deleted profile key \"%s\"",
                         strClear.c_str());
    }

    return (1);
}

/*
 *@@ AddToUndoList:
 *      static helper method to create BSClearProfile
 *      instances from the specified logger instance.
 *      Note that this class method will be used both
 *      for "CLEARPROFILE" tags specified in the script
 *      as well as "WRITEPROFILE" tags which were actually
 *      executed (written to a profile).
 *
 *      The new BSClearProfile objects are appended to
 *      the specified list.
 *
 *      This returns the number of items created.
 */

int BSClearProfile::AddToUndoList(list<BSConfigBase*> &List,
                                  BSClearPrfAttrsLogger &logger)
{
    // the logger in this case has a simple list of PSZs with all
    // the profile keys that were created, each of which is terminated
    // with a null character

    PSZ     pszLogStart = logger._pabLogString,
            pszLogThis = pszLogStart;
    int     iCount = 0;

    while (pszLogThis < pszLogStart + logger._cbLogString)
    {
        ULONG cbLogThis = strlen(pszLogThis);
        ustring ustr;
        ustr.assignUtf8(pszLogThis);
        List.push_back(new BSClearProfile(ustr));
        pszLogThis += cbLogThis + 1;    // go beyond null byte
        iCount++;
    }

    return (iCount);
}

/* ******************************************************************
 *
 *  BSWriteProfile class
 *
 ********************************************************************/

/*
 *@@ BSWriteProfile:
 *
 *      Syntax:
 *
 +          WRITEPROFILE="profile\application\key|string"
 *
 *@@added V0.9.1 (2000-02-07) [umoeller]
 *@@changed V0.9.19 (2002-05-07) [umoeller]: added missing class init, now writeprofiles work again
 *@@changed V1.0.5 (2005-03-06) [pr]: Improved parsing of profiles with path names. @@fixes 633
 */

BSWriteProfile::BSWriteProfile(const ustring &ustrWriteProfile)
                : BSProfileBase(CFGT_WRITEPROFILE, tBSWriteProfile)
{
    PCSZ    pcszWriteProfile = ustrWriteProfile.GetBuffer();

    // override class, this was still BSClearProfile due
    // to the constructor; this is also why writeprofiles
    // never worked V0.9.19 (2002-05-07) [umoeller]
    _Class = tBSWriteProfile;

    PCSZ    pSlash;
    if (pSlash = strchr(pcszWriteProfile, '|'))
    {
        // copy write data (after '|')
        _ustrWriteString.assignUtf8(pSlash + 1);

        // get profile
        PCSZ pFirstBackslash;
        if (pFirstBackslash = strchr(pcszWriteProfile, '\\'))
        {
            // extract profile
            _ustrProfile.assignUtf8(pcszWriteProfile, pFirstBackslash);

            if (   _ustrProfile.compareUtf8("USER")
                && _ustrProfile.compareUtf8("SYSTEM")
               )
            {
                while (pFirstBackslash)
                {
                    ULONG cbProfile = pFirstBackslash - pcszWriteProfile;
                    if (   (cbProfile > 4)
                        && (!strnicmp(pFirstBackslash - 4, ".INI", 4))
                        && (pFirstBackslash < pSlash)
                       )
                    {
                        _ustrProfile.assignUtf8(pcszWriteProfile, pFirstBackslash);
                        break;
                    }

                    pFirstBackslash = strchr(pFirstBackslash + 1, '\\');
                }
            }
        }

        if (pFirstBackslash)
        {
            // get application
            PCSZ pSecondBackslash;
            if (pSecondBackslash = strchr(pFirstBackslash + 2, '\\'))
            {
                // extract application
                _ustrApplication.assignUtf8(pFirstBackslash + 1, pSecondBackslash);

                // extract key (up to '|')
                _ustrKey.assignUtf8(pSecondBackslash + 1, pSlash);
            }
        }
    }

    if (!_ustrKey())
        // any error:
        throw BSConfigExcpt(PRFEXCPT_SYNTAX, ustrWriteProfile);
}

/*
 *@@ Write:
 *      this actually writes data to a profile, all
 *      as specified by the member variables.
 *
 *      Note that this stores the "done" stuff in
 *      a BSClearPrfAttrsLogger for undoing later.
 *
 *      Throws:
 *      -- BSConfigExcpt.
 *
 *@@added V0.9.1 (2000-02-07) [umoeller]
 *@@changed V0.9.18 (2002-03-08) [umoeller]: added codec
 */

int BSWriteProfile::Write(BSUniCodec &codecProcess, // in: codec for process codepage,
                          HAB hab,      // in: anchor block (for PrfOpenProfile)
                          BSClearPrfAttrsLogger *pLogger,
                          BSFileLogger *pLogFile)   // in: file logger (log file), can be NULL
                    const
{
    BOOL    fOK = FALSE;
    HINI    hini = NULLHANDLE,
            hini2Close = NULLHANDLE;

    if (!_ustrProfile.compareUtf8("USER"))
        hini = HINI_USER;
    else if (!_ustrProfile.compareUtf8("SYSTEM"))
        hini = HINI_SYSTEM;
    else
    {
        CONVERT(&codecProcess, Profile);
        if (!(hini = PrfOpenProfile(hab,
                                    pcszProfile)))
            throw BSConfigExcpt(PRFEXCPT_PRFOPENPROFILE, _ustrProfile);

        hini2Close = hini;
    }

    if (hini)
    {
        CONVERT(&codecProcess, Application);
        CONVERT(&codecProcess, Key);
        CONVERT(&codecProcess, WriteString);
        if (PrfWriteProfileString(hini,
                                  pcszApplication,
                                  pcszKey,
                                  pcszWriteString))
        {
            fOK = TRUE;
        }
    }

    // recompose string for BSClearProfile
    ustring ustrClear = DescribePrfKey();
    string strClear = DescribePrfKey(codecProcess);

    if (fOK)
    {
        if (pLogFile)
            pLogFile->Write("Wrote profile key \"%s\"",
                             strClear.c_str());

        if (pLogger)
            pLogger->Append(ustrClear);
    }
    else
    {
        // error:
        if (pLogFile)
            pLogFile->Write("Error writing profile key \"%s\"",
                             strClear.c_str());

        throw BSConfigExcpt(PRFEXCPT_PRFERROR, DescribePrfKey());
    }

    return 1;
}

/* ******************************************************************
 *
 *  BSExecute class
 *
 ********************************************************************/

/*
 *@@ BSExecute:
 *      this constructor takes an EXECUTE attribute (of the PCK tag)
 *      as input and sets up the instance data accordingly.
 *
 *      Syntax:
 *
 +          EXECUTE="[ context ] execfile params"
 *
 *      Throws:
 *      -- BSConfigExcpt.
 *
 *@@added V0.9.1 (2000-02-07) [umoeller]
 *@@changed V0.9.3 (2000-04-08) [umoeller]: "|" was a requirement, contradicting the docs; changed. Thanks, Cornelis Bockemhl.
 *@@changed V0.9.4 (2000-07-10) [umoeller]: _ulExecType wasn't always initialized; thanks Yuri Dario
 *@@changed V0.9.4 (2000-07-26) [umoeller]: now ORing the flags
 *@@changed V0.9.9 (2001-03-27) [umoeller]: fixed EXECUTE parameters
 */

BSExecute::BSExecute(const ustring &ustrExecute,
                     BSClassID &Class)
           : BSConfigBase(CFGT_EXECUTE, Class)
{
    BOOL    fContextFound = FALSE,
            fContinue = TRUE;
    PCSZ    pcszExecute = ustrExecute.GetBuffer();
    PCSZ    pSearch = (PSZ)pcszExecute;

    _ulExecType = 0;
    _arc = 0;       // NO_ERROR

    while (fContinue)
    {
        // skip leading spaces
        while (    (*pSearch)
                && (*pSearch == ' ')
              )
            pSearch++;

        if (strncmp(pSearch, "CONFIGSYS", 9) == 0)
        {
            _ulExecType |= CFGT_CFGSYS;
            fContextFound = TRUE;
            pSearch += 9; // strlen("CONFIGSYS");
        }
        else if (strncmp(pSearch, "REGISTERCLASS", 13) == 0)
        {
            _ulExecType |= CFGT_REGISTERCLASS;
            fContextFound = TRUE;
            pSearch += 13; // strlen("REGISTERCLASS");
        }
        else if (strncmp(pSearch, "CREATEOBJECT", 12) == 0)
        {
            _ulExecType |= CFGT_CREATEOBJECT;
            fContextFound = TRUE;
            pSearch += 12; // strlen("CREATEOBJECT");
        }
        else
            // other: stop looping
            fContinue = FALSE;
    } // end while (fContinue)

    if (!(*pSearch))
        throw BSConfigExcpt(EXEEXCPT_SYNTAX, ustrExecute);

    // pSearch now points to the first non-space
    // character which does not introduces a keyword;
    // check if this is a "|" separator

    if (*pSearch == '|')
        // skip '|'
        pSearch++;

    // skip following spaces
    while (    (*pSearch)
            && (*pSearch == ' ')
          )
        pSearch++;

    // NOW we finally point to the executable

    if (!(*pSearch))
        throw BSConfigExcpt(EXEEXCPT_SYNTAX, ustrExecute);

    // separate executable and parameters:
    // This can be separated by another '|',
    // or a space.

    // NOTE: The '|' separator is used by the "resolved"
    // logger, from which BSDeExecute instances are
    // created on de-install. But the documentation states
    // spaces should be used... as long as a user doesn't
    // use spaces in the (unresolved) filename, we're fine.
    // For example we get:
    // 1)  on install: $(1)\install.cmd PARAMETER
    //      works nicely, and $(1) may contain spaces.
    // 2)  Logger stores C:\PATH1 WITH SPACES\install.cmd|PARAMETER
    //     in database.
    // 3)  on deinstall, BSDeExecutes get created, and
    //     we now have nice separation and get PARAMETER
    //     properly separated.
    PCSZ p;
    if (!(p = strchr(pSearch, '|')))
        p = strchr(pSearch, ' ');
    if (p)
    {
        const char *pSep = p++; // V1.0.5 (2005-01-20) [pr]: @@fixes 624
        // we have a space --> parameters:
        while (    (*p)
                && (*p == ' ')
              )
            p++;

        if (*p)
        {
            _ustrParams.assignUtf8(p);
            _ustrExecutable.assignUtf8(pSearch, pSep);
        }
        else
            _ustrExecutable.assignUtf8(pSearch);
    }
    else
        _ustrExecutable.assignUtf8(pSearch);
}

/*
 *@@ DescribeType:
 *      describes the current manipulator type to the GUI.
 *
 *@@added V0.9.9 (2001-03-27) [umoeller]
 */

const char* BSExecute::DescribeType()
{
    return ("Execute program");
}

/*
 *@@ DescribeData:
 *      describes the current manipulator data to the GUI.
 *
 *@@added V0.9.9 (2001-03-27) [umoeller]
 *@@changed V0.9.18 (2002-03-08) [umoeller]: now returning ustring
 */

ustring BSExecute::DescribeData()
{
    ustring    ustr;
    ustr._printf("\"%s\"", _ustrExecutable.GetBuffer());

    if (_ustrParams())
    {
        ustr.appendUtf8(" \"");
        ustr += _ustrParams;
        ustr.appendUtf8("\"");
    }

    if (_ulExecType == 0)
        ustr.appendUtf8(" (no flags)");
    else
    {
        if (_ulExecType & CFGT_CFGSYS)
            ustr.appendUtf8(" CONFIGSYS");
        if (_ulExecType & CFGT_REGISTERCLASS)
            ustr.appendUtf8(" REGISTERCLASS");
        if (_ulExecType & CFGT_CREATEOBJECT)
            ustr.appendUtf8(" CREATEOBJECT");
    }

    return (ustr);
}

/*
 *@@ Execute:
 *      executes the member program. This does not return
 *      until the program terminates. As a result, do not
 *      call this on a PM thread, because this gets blocked.
 *
 *      Throws:
 *      -- BSConfigExcpt.
 *
 *@@added V0.9.1 (2000-02-07) [umoeller]
 *@@changed V0.9.18 (2002-03-08) [umoeller]: added codec
 *@@changed V1.0.5 (2005-01-25) [pr]: Change directory to that of executable
 *@@changed WarpIN V1.0.10 (2006-04-01) [pr]: Set title in doshStartSession call
 */

int BSExecute::Execute(BSUniCodec &codecProcess, // in: codec for process codepage,
                       BSFileLogger *pLogFile)  // in: file logger (log file), can be NULL
{
    string    strTitle;
    strTitle.assignUtf8(&codecProcess, _ustrExecutable);
    string    strParams = "/c ";
    strParams.appendUtf8(&codecProcess, _ustrExecutable);
    if (_ustrParams())
    {
        strParams += " ";
        strParams.appendUtf8(&codecProcess, _ustrParams);
    }

    // V1.0.7 (2005-05-30) [pr]: This doesn't work. Some packages rely on the current
    // directory not being changed, so you cannot win whatever you do. In any case, the
    // code below assumes it is always a path'd executable, which is not always the case.
    // Perhaps we should just set BEGINLIBPATH here instead which will allow newly
    // installed EXEs to find their DLLs.
    //
    // V1.0.5 (2005-01-26) [pr]: Change directory to that of executable. @@fixes 626.
    // We remove any leading quotes and just leave the directory name, excluding a
    // trailing slash, unless it is a root directory.
#if 0
    string    strDir;
    strDir.assignUtf8(&codecProcess, _ustrExecutable);
    strDir = strDir.substr(strDir.find_first_not_of("\"'"));
    size_type pos = strDir.rfind('\\', 0);
    if (pos != string::npos)
    {
        strDir.erase(pos);
        if (strDir.size() == 2 && strDir[1] == ':')
            strDir += '\\';

        doshSetCurrentDir(strDir.c_str());
    }
#endif

    ULONG       sid = 0;
    PID         pid = 0;
    if (_arc = doshQuickStartSession("cmd.exe",
                                     (PSZ)strParams.c_str(),
                                     (PSZ)strTitle.c_str(),
                                     SSF_TYPE_DEFAULT,            // session type
                                     FALSE,   // foreground
                                     0,       // auto-close, visible
                                     TRUE,    // wait
                                     &sid,
                                     &pid,
                                     NULL))
    {
        if (pLogFile)
            pLogFile->Write("Error %d executing \"CMD.EXE %s\"",
                             _arc,
                             strParams.c_str());

        throw BSConfigExcpt(EXEEXCPT_DOSERROR, _arc);
    }

    if (pLogFile)
        pLogFile->Write("Executed \"CMD.EXE %s\"",
                         strParams.c_str());

    return (0);
}

/* ******************************************************************
 *
 *  BSDeExecute class
 *
 ********************************************************************/

/*
 *@@ BSDeExecute:
 *      constructor.
 *
 *@@added V0.9.16 (2001-12-08) [umoeller]
 */

BSDeExecute::BSDeExecute(const ustring &ustrDeExecute)
    : BSExecute(ustrDeExecute, tBSDeExecute)
{
};

/*
 *@@ DescribeType:
 *      describes the current manipulator type to the GUI.
 *
 *@@added V0.9.2 (2000-02-19) [umoeller]
 */

const char* BSDeExecute::DescribeType()
{
    return ("External program for deinstall");
}

/*
 *@@ AddToUndoList:
 *      static helper method to create BSDeExecute instances from
 *      the specified logger instance.
 *
 *      The new BSKillProcess objects are appended to
 *      the specified list.
 *
 *      This returns the number of items created.
 *
 *      This isn't really an "Undo" method like the other
 *      static AddToUndoList methods (for the other classes),
 *      but has been named like this for conformity. This
 *      is during package de-install to create the same
 *      BSDeExecute list which was initially created from
 *      the script so that processes can be killed during
 *      de-install as well.
 *
 *@@added V0.9.9 (2001-03-27) [umoeller]
 */

int BSDeExecute::AddToUndoList(list <BSConfigBase*> &List,
                               BSDeExecuteResolvedLogger &logger)
{
    PSZ     pszLogStart = logger._pabLogString,
            pszLogThis = pszLogStart;
    int     iCount = 0;

    while (pszLogThis < pszLogStart + logger._cbLogString)
    {
        ULONG cbLogThis = strlen(pszLogThis);
        ustring ustr;
        ustr.assignUtf8(pszLogThis);
        List.push_back(new BSDeExecute(ustr));
        pszLogThis += cbLogThis + 1;    // go beyond null byte
        iCount++;
    }

    return (iCount);
}

/* ******************************************************************
 *
 *  BSKillProcess class
 *
 ********************************************************************/

/*
 *@@ BSKillProcess:
 *
 *      Syntax:
 *
 +          KILLPROCESS="filename"
 *
 *      Throws:
 *      -- BSConfigExcpt.
 *
 *@@added V0.9.1 (2000-02-12) [umoeller]
 */

BSKillProcess::BSKillProcess(const ustring &ustrKillProcess)
               : BSConfigBase(CFGT_KILLPROCESS, tBSKillProcess)
{
    if (!ustrKillProcess())
        throw BSConfigExcpt(KILLEXCPT_SYNTAX, ustrKillProcess);

    _ustrKillProcess = ustrKillProcess;
}

/*
 *@@ DescribeType:
 *      describes the current manipulator type to the GUI.
 *
 *@@added V0.9.4 (2000-07-01) [umoeller]
 */

const char* BSKillProcess::DescribeType()
{
    return ("Kill process");
}

/*
 *@@ DescribeData:
 *      describes the current manipulator data to the GUI.
 *
 *@@added V0.9.4 (2000-07-01) [umoeller]
 *@@changed V0.9.18 (2002-03-08) [umoeller]: now returning ustring
 */

ustring BSKillProcess::DescribeData()
{
    return _ustrKillProcess;
}

/*
 *@@ AddToUndoList:
 *      static helper method to create BSKillProcess
 *      instances from the specified logger instance.
 *
 *      The new BSKillProcess objects are appended to
 *      the specified list.
 *
 *      This returns the number of items created.
 *
 *      This isn't really an "Undo" method like the other
 *      static AddToUndoList methods (for the other classes),
 *      but has been named like this for conformity. This
 *      is during package de-install to create the same
 *      BSKillProcess list which was initially created from
 *      the script so that processes can be killed during
 *      de-install as well.
 *
 *@@added V0.9.4 (2000-07-01) [umoeller]
 */

int BSKillProcess::AddToUndoList(list<BSConfigBase*> &List,
                                 BSKillProcessAttrsLogger &logger)
{
    // the logger in this case has a simple list of PSZs with all
    // the executable names that were created, each of which is
    // terminated with a null character

    PSZ     pszLogStart = logger._pabLogString,
            pszLogThis = pszLogStart;
    int     iCount = 0;

    while (pszLogThis < pszLogStart + logger._cbLogString)
    {
        ULONG cbLogThis = strlen(pszLogThis);
        ustring ustr;
        ustr.assignUtf8(pszLogThis);
        List.push_back(new BSKillProcess(ustr));
        pszLogThis += cbLogThis + 1;    // go beyond null byte
        iCount++;
    }

    return (iCount);
}


