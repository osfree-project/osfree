
/*
 * bs_config_impl.h:
 *      this defines the classes which were only
 *      declared in bs_config.h.
 *
 *@@include #define INCL_WINWORKPLACE
 *@@include #include <os2.h>
 *@@include #include <list>
 *@@include #include "helpers\configsys.h"
 *@@include #include "base\bs_errors.h"
 *@@include #include "base\bs_config.h"
 */

/*
 *      This file Copyright (C) 1999-2006 Ulrich M”ller.
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation, in version 2 as it comes in the COPYING
 *      file of this distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 */

#ifndef WARPIN_CONFIG_IMPL_HEADER_INCLUDED
    #define WARPIN_CONFIG_IMPL_HEADER_INCLUDED

    /* ******************************************************************
     *
     *  BSConfigExcpt error codes
     *
     ********************************************************************/

    // error codes for BSConfigExcpt thrown by BSConfigSys class
    const int CFGEXCPT_OPEN                 = 1;
                    // error opening CONFIG.SYS (BSConfigSys);
                    // BSConfigExcpt.iData has the APIRET
    const int CFGEXCPT_DOUBLEREPLACEMODE    = 2;
                    // error parsing CONFIGSYS attribute (BSCfgSysManip):
                    // replace mode specified twice,
                    // BSConfigExcpt.pszSubstring has the failing part
    const int CFGEXCPT_INVALIDVERTICAL      = 3;
                    // error parsing CONFIGSYS attribute (BSCfgSysManip):
                    // vertical modifier not allowed here
                    // BSConfigExcpt.pszSubstring has the failing part
    const int CFGEXCPT_INVALIDSEARCH        = 4;
                    // invalid search string (BSCfgSysManip)
                    // BSConfigExcpt.pszSubstring has the failing part
    const int CFGEXCPT_MANIPULATING         = 5;
                    // error in BSConfigSys::Manipulate()
                    // (should not happen)
    const int CFGEXCPT_WRITE                = 6;
                    // error in BSConfigSys::Flush();
                    // BSConfigExcpt.iData has the APIRET
    const int CFGEXCPT_NOTIMPLEMENTED       = 7;
                    // not implemented yet
    const int CFGEXCPT_NOSEPARATOR          = 8;
                    // this pops up while modifying CONFIG.SYS
                    // and ADDLEFT or ADDRIGHT have been specified,
                    // but the line in CONFIG.SYS has no "=" character
    const int CFGEXCPT_SYNTAX               = 9;
                    // invalid attribute passed to BSCfgSysManip::BSCfgSysManip
    const int CFGEXCPT_PARSELOG             = 10;
                    // syntax error parsing log entry in BSCfgSysManip::AddToUndoList

    // error codes for BSConfigExcpt thrown by the BSRegisterClass, BSDeregisterClass,
    // BSReplaceClass classes
    const int REGEXCPT_SYNTAX               = 60;
                    // syntax error in constructor (invalid pszRegisterClass);
                    // BSConfigExcpt.pszSubstring has the whole failing pszRegisterClass.
                    // This is also used by BSReplaceClass::BSReplaceClass.
    const int REGEXCPT_QUERYCLASSLIST       = 61;
                    // error querying WPS class list
    const int REGEXCPT_ALREADYREGISTERED    = 62;
                    // BSRegisterClass::Register(FALSE) has determined that the
                    // class is already registered;
                    // BSConfigExcpt.pszSubstring has the currently registered DLL
    const int REGEXCPT_REGISTER             = 63;
                    // BSRegisterClass::Register failed;
                    // BSConfigExcpt.iData has the APIRET of winhRegisterClass
    const int REGEXCPT_DEREGISTER           = 64;
                    // BSDeregisterClass::Deregister failed
    const int REGEXCPT_REPLACE              = 65;
                    // BSReplaceClass::Replace failed (WinReplaceObjectClass returned FALSE)
    const int REGEXCPT_UNREPLACE            = 66;
                    // BSUnreplaceClass::Unreplace failed (WinReplaceObjectClass returned FALSE)

    // error codes for BSConfigExcpt thrown by BSCreateWPSObject class
    const int WPOEXCPT_NOLOCATION           = 100;
                    // no location given
    const int WPOEXCPT_NOTITLE              = 101;
                    // no title given
    const int WPOEXCPT_NOCLASS              = 102;
                    // no class given
    const int WPOEXCPT_INVALIDLOCATIONSTRING = 103;
                    // invalid location string (not in angle brackets)
                    // V0.9.15 (2001-08-26) [umoeller]
    const int WPOEXCPT_CREATE               = 104;
                    // error in BSCreateWPSObject::CreateObject
    const int WPOEXCPT_CREATE_BADLOCATION   = 105;
                    // error in BSCreateWPSObject::CreateObject:
                    // specified location does not exist
                    // (probably invalid object ID for target folder)
                    // V0.9.19 (2002-06-15) [umoeller]
    const int WPOEXCPT_DELETEOBJECT         = 106;
                    // error deleting object (BSDeleteWPSObject::Delete)

    // error codes for BSConfigExcpt thrown by BSExecute class
    const int EXEEXCPT_SYNTAX               = 200;
                    // syntax error parsing EXECUTE tag
    const int EXEEXCPT_DOSERROR             = 201;
                    // DOS error starting executable;
                    // BSConfigExcpt.iData has the APIRET

    const int PRFEXCPT_SYNTAX               = 300;
                    // syntax error in CLEARPROFILE or WRITEPROFILE tag
    const int PRFEXCPT_PRFERROR             = 301;
                    // error clearing/writing to specified profile
    const int PRFEXCPT_PRFOPENPROFILE       = 302;
                    // error opening profile V1.0.0 (2002-11-23) [umoeller]
                    // excpt data is profile name

    // error codes for BSConfigExcpt thrown by BSKillProcess class
    const int KILLEXCPT_SYNTAX              = 400;
                    // syntax error parsing KILLPROCESS tag

    /* ******************************************************************
     *
     *  CONFIG.SYS class declarations
     *
     ********************************************************************/

    // flags for BSCfgSysManip (below)

    /*
     *@@ BSCfgSysManip:
     *      the "CONFIG.SYS manipulator" class. Each
     *      instance of this represents one change to
     *      be made to CONFIG.SYS, which itself is
     *      represented by an instance of BSConfigSys.
     *
     *      An instance of this must be created with
     *      a suitable attribute string and passed to
     *      BSConfigSys::Manipulate.
     *
     *      See BSCfgSysManip::BSCfgSysManip for details.
     *
     *@@changed V0.9.0 (99-11-06) [umoeller]: renamed from CfgSysManip
     *@@changed V0.9.1 (2000-01-06) [umoeller]: added UNIQUE(xxx) support
     *@@changed V0.9.18 (2002-03-08) [umoeller]: adjusted for Unicode
     */

    class BSCfgSysManip : public BSConfigBase
    {
        public:
            DECLARE_CLASS(BSCfgSysManip);

        // allow BSConfigSys access to private members
        friend class BSConfigSys;

        public:
            // CONFIGMANIP _ConfigManip;
            // replaced CONFIGMANIP with the below fields for Unicode support

            int         _iReplaceMode;   // this is an int, really
                        // one of the following:
                        // -- CFGRPL_ADD (0): just add line (default; iVertical applies).
                        // -- CFGRPL_UNIQUE mode (1): existing line is replaced;
                        //    if not found, iVertical applies.
                        // -- CFGRPL_ADDLEFT mode (2): line is updated to the left;
                        //    if not found, iVertical applies.
                        // -- CFGRPL_ADDRIGHT mode (3): line is updated to the right;
                        //    if not found, iVertical applies.
                        // -- CFGRPL_REMOVELINE mode (4)
                        // -- CFGRPL_REMOVEPART mode (5)

            ustring     _ustrUniqueSearchString2;
                        // for UNIQUE(statement2):
                        // the "statement2" to search for
                        // V0.9.1 (2000-01-06) [umoeller]

            int         _iVertical;      // this is an int, really
                        // 0: add to bottom (default)
                        // 1: add to top
                        // 2: add before pszSearchString
                        // 3: add after pszSearchString
            ustring     _ustrVerticalSearchString;
                        // for iVertical == 2 or 3

            ustring     _ustrNewLine;
                        // this is a copy of stuff before the "|" char
                        // in pszConfigSys given to the constructor

            BSCfgSysManip(const ustring &ustrConfigSys);
            virtual ~BSCfgSysManip() {};

            virtual const char* DescribeType();
            virtual ustring DescribeData();

            static int AddToUndoList(list<BSConfigBase*> &List,
                                     BSCfgSysDoneLogger &logger);
    };

    /*
     *@@ BSConfigSys:
     *      represents the CONFIG.SYS file for manipulation.
     *      When an instance of this is created, CONFIG.SYS is
     *      read in by the constructor, so only create an
     *      instance of this if you're actually to modify the
     *      file.
     *
     *      Note that this is not a subclass of BSConfigBase since
     *      this does not contain any configuration data. Instead,
     *      this represents a CONFIG.SYS file on which BSCfgSysManip
     *      instances are to be invoked to actually modify CONFIG.SYS.
     *
     *      The destructor does _not_ write the file. Explicitly call
     *      BSConfigSys::Flush() for that.
     *
     *@@changed V0.9.0 (99-11-06) [umoeller]: renamed from ConfigSys
     *@@changed V0.9.6 (2000-10-27) [umoeller]: added _fDirty
     *@@changed WarpIN V1.0.9 (2006-02-16) [pr]: added _pszContentOld
     */

    class BSConfigSys : public BSRoot
    {
        public:
            DECLARE_CLASS(BSConfigSys);

        protected:
            char        *_pszContent;
            char        *_pszContentOld;
            BOOL        _fDirty;                 // TRUE if _pszContent has changed

        public:
            BSConfigSys();
            virtual ~BSConfigSys();

            int Manipulate(BSUniCodec &codecProcess,
                           BSCfgSysManip& Manip,
                           BSCfgSysDoneLogger *pLogger,
                           BSFileLogger *pLogFile);
            int Flush(string *pstrBackup,
                      BSFileLogger *pLogFile)
                const;
    };

    /* ******************************************************************
     *
     *  Register-class class declarations
     *
     ********************************************************************/

    /*
     *@@ BSRegisterClass:
     *      represents one WPS class to be registered with the
     *      system.
     *
     *      Call BSRegisterClass::Register to actually register
     *      the class.
     *
     *@@changed V0.9.0 (99-11-06) [umoeller]: renamed from RegisterClass
     */

    class BSRegisterClass : public BSConfigBase
    {
        public:
            DECLARE_CLASS(BSRegisterClass);

        public:
            // all these must be public because otherwise
            // WarpIN cannot resolve macros
            ustring     _ustrClassName;
            ustring     _ustrModule;

            virtual const char* DescribeType();
            virtual ustring DescribeData();

            BSRegisterClass(const ustring &ustrRegisterClass);
            virtual ~BSRegisterClass() {};

            bool IsRegistered(BSUniCodec &codecProcess,
                              ustring &ustrModule)
                 const;

            int Register(BSUniCodec &codecProcess,
                         bool fReplace,
                         BSRegisterDoneLogger *pLogger,
                         BSFileLogger *pLogFile)
                         const;
    };

    /*
     *@@ BSDeregisterClass:
     *      represents one WPS class to be deregistered from the
     *      system.
     *
     *      Call BSDeregisterClass::Deregister to actually deregister
     *      the class.
     *
     *      This is used to deregister classes that were previously
     *      registered using the BSRegisterClass class.
     *
     *@@changed V0.9.0 (99-11-06) [umoeller]: renamed from BSDeregisterClass
     */

    class BSDeregisterClass : public BSConfigBase
    {
        public:
            DECLARE_CLASS(BSDeregisterClass);

        public:
            ustring     _ustrClassName;

            virtual const char* DescribeType();
            virtual ustring DescribeData();

            BSDeregisterClass(const ustring &ustrDeregisterClass);
            virtual ~BSDeregisterClass() {};

            int Deregister(BSUniCodec &codecProcess,
                           BSFileLogger *pLogFile)
                const;

            static int AddToUndoList(list<BSConfigBase*> &List,
                                     BSRegisterDoneLogger &logger);
    };

    /*
     *@@ BSReplaceClassBase:
     *      abstract base class for both BSReplaceClass
     *      and BSUnReplaceClass so we can share the
     *      instance vars and the parser in the constructor.
     *
     *@@added V0.9.19 (2002-05-07) [umoeller]
     */

    class BSReplaceClassBase : public BSConfigBase
    {
        public:
            DECLARE_CLASS(BSReplaceClassBase);

        public:
            // all these must be public because otherwise
            // WarpIN cannot resolve macros
            ustring     _ustrOldClassName;
            ustring     _ustrNewClassName;

        protected:
            BSReplaceClassBase(const ustring &ustrReplaceClass,
                               ULONG cfgt,
                               BSClassID &Class);
            virtual ~BSReplaceClassBase() {};
    };

    /*
     *@@ BSReplaceClass:
     *      represents one WPS class to be replaced with another
     *      WPS class on the system.
     *
     *      Call BSReplace::Replace to actually register
     *      the class.
     *
     *@@changed V0.9.0 (99-11-06) [umoeller]: renamed from ReplaceClass
     *@@changed V0.9.19 (2002-05-07) [umoeller]: changed inheritance to get DYNAMIC_CAST right
     *@@changed V0.9.20 (2002-07-22) [umoeller]: removed duplicate instance variables to fix total REPLACECLASS breakage
     */

    class BSReplaceClass : public BSReplaceClassBase
    {
        public:
            DECLARE_CLASS(BSReplaceClass);

        public:
            /* removed these instance variables duplicating the same ones
               in the parent, this kept the constructor from working at all
                V0.9.20 (2002-07-22) [umoeller]
            ustring     _ustrOldClassName;
            ustring     _ustrNewClassName;
            */

            BSReplaceClass(const ustring &ustrReplaceClass)
                : BSReplaceClassBase(ustrReplaceClass,
                                     CFGT_REPLACECLASS,
                                     tBSReplaceClass)
            {
            }

            virtual ~BSReplaceClass() {};

            virtual const char* DescribeType();
            virtual ustring DescribeData();

            int Replace(BSUniCodec &codecProcess,
                        BSReplaceDoneLogger *pLogger,
                        BSFileLogger *pLogFile)
                const;
    };

    /*
     *@@ BSUnreplaceClass:
     *      the reverse to BSReplaceClass. This is used to un-replace
     *      classes that were previously replaced using the
     *      BSRegisterClass class.
     *
     *      Since the BSReplaceDoneLogger class stores items just
     *      like the input to BSReplaceClass was, we inherit from
     *      that class to use the same parsing routines.
     *
     *@@changed V0.9.0 (99-11-06) [umoeller]: renamed from UnreplaceClass
     *@@changed V0.9.19 (2002-05-07) [umoeller]: changed inheritance to get DYNAMIC_CAST right
     */

    class BSUnreplaceClass : public BSReplaceClassBase
    {
        public:
            DECLARE_CLASS(BSUnreplaceClass);

        public:
            // inherit constructor from BSReplaceClass
            BSUnreplaceClass(const ustring &ustrUnreplaceClass)
                    : BSReplaceClassBase(ustrUnreplaceClass,
                                         CFGT_UNREPLACECLASS,
                                         tBSUnreplaceClass)
            {};

            virtual const char* DescribeType();
            virtual ustring DescribeData();

            int Unreplace(BSUniCodec &codecProcess,
                          BSFileLogger *pLogFile)
                const;

            static int AddToUndoList(list<BSConfigBase*> &List,
                                     BSReplaceDoneLogger &logger);
    };

    /* ******************************************************************
     *
     *  Create-object class declarations
     *
     ********************************************************************/

    /*
     *@@ BSCreateWPSObject:
     *      represents one WPS object to be created on the
     *      system.
     *
     *      Call BSCreateWPSObject::CreateObject to actually
     *      create the object.
     *
     *@@changed V0.9.0 (99-11-06) [umoeller]: renamed from CreateWPSObject
     */

    class BSCreateWPSObject : public BSConfigBase
    {
        public:
            DECLARE_CLASS(BSCreateWPSObject);

        public:
            // the following should always be set
            ustring     _ustrClassName;
            ustring     _ustrTitle;
            ustring     _ustrSetupString;     // this one might be NULL
            ustring     _ustrLocation;
            ULONG       _ulFlags;  // V1.0.14 (2006-12-03) [pr]: renamed from _fReplace @@fixes 893

            // the following is set by BSConfigExcpt::CreateObject
            // if an object has been created successfully
            HOBJECT     _hobj;

            BSCreateWPSObject(const ustring &ustrCreateObject);
            virtual ~BSCreateWPSObject() {};

            virtual const char* DescribeType();
            virtual ustring DescribeData();

            void CreateObject(BSUniCodec &codecProcess,
                              BSWPSObjectsDoneLogger *pLogger,
                              BSFileLogger *pLogFile);
            bool DoesObjectExist();
            void DestroyObject(BSFileLogger *pLogFile);
    };

    /*
     *@@ BSDeleteWPSObject:
     *      the reverse to BSCreateWPSObject. This is used to delete
     *      objects that were previously created using the
     *      BSCreateWPSObject class.
     */

    class BSDeleteWPSObject : public BSConfigBase
    {
        public:
            DECLARE_CLASS(BSDeleteWPSObject);

        public:
            ustring     _ustrObjectID;

            BSDeleteWPSObject(const ustring &ustrID2Delete);
            virtual ~BSDeleteWPSObject() {};

            virtual const char* DescribeType();
            virtual ustring DescribeData();

            int Delete(BSUniCodec &codecProcess,
                       BSFileLogger *pLogFile)
                const;

            static int AddToUndoList(list<BSConfigBase*> &List,
                                     BSWPSObjectsDoneLogger &logger);
    };

    /* ******************************************************************
     *
     *  Profile classes
     *
     ********************************************************************/

    /*
     *@@ BSProfileBase:
     *      abstract base class for both BSClearProfile
     *      and BSWriteProfile so that we can share the
     *      member variables and the description routines.
     *
     *      Note that BSWriteProfile still adds the
     *      _ustrWriteString member.
     *
     *@@added V0.9.19 (2002-05-07) [umoeller]
     */

    class BSProfileBase : public BSConfigBase
    {
        public:
            DECLARE_CLASS(BSProfileBase);

        public:
            ustring     _ustrProfile;
            ustring     _ustrApplication;
            ustring     _ustrKey;

            /*
             *@@ BSProfileBase:
             *
             *@@added V0.9.19 (2002-05-07) [umoeller]
             */

            BSProfileBase(ULONG cfgt,
                          BSClassID &Class)
                : BSConfigBase(cfgt, Class)
            {
            }

            virtual ~BSProfileBase() {};

            virtual const char* DescribeType();
            virtual ustring DescribeData();

            ustring DescribePrfKey() const;
            string DescribePrfKey(BSUniCodec &codecProcess) const;
    };

    /*
     *@@ BSClearProfile:
     *      represents a chunk of data to be removed from
     *      a profile (INI file) on the system.
     *
     *@@added V0.9.1 (2000-02-07) [umoeller]
     *@@changed V0.9.19 (2002-05-07) [umoeller]: changed inheritance to get DYNAMIC_CAST right
     */

    class BSClearProfile : public BSProfileBase
    {
        public:
            DECLARE_CLASS(BSClearProfile);

        public:
            BSClearProfile(const ustring &ustrClearProfile);

            virtual ~BSClearProfile() {};

            int Clear(BSUniCodec &codecProcess,
                      HAB hab,
                      BSFileLogger *pLogFile)
                const;

            static int AddToUndoList(list<BSConfigBase*> &List,
                                     BSClearPrfAttrsLogger &logger);
    };

    /*
     *@@ BSWriteProfile:
     *      represents a string to be written to
     *      a profile (INI file) on the system.
     *
     *@@added V0.9.1 (2000-02-07) [umoeller]
     *@@changed V0.9.19 (2002-05-07) [umoeller]: changed inheritance to get DYNAMIC_CAST right
     */

    class BSWriteProfile : public BSProfileBase
    {
        public:
            DECLARE_CLASS(BSWriteProfile);

        public:
            ustring     _ustrWriteString;

            BSWriteProfile(const ustring &ustrWriteProfile);

            virtual ~BSWriteProfile() {};

            int Write(BSUniCodec &codecProcess,
                      HAB hab,
                      BSClearPrfAttrsLogger *pLogger,
                      BSFileLogger *pLogFile)
                const;
    };

    /* ******************************************************************
     *
     *  Execute application program class
     *
     ********************************************************************/

    /*
     *@@ BSExecute:
     *      represents a user executable (EXECUTE tag) to
     *      be executed when installation is done.
     *
     *      Call BSExecute::Execute to actually run the
     *      program.
     *
     *@@added V0.9.1 (2000-02-07) [umoeller]
     *@@changed V0.9.9 (2001-03-27) [umoeller]: separated executable and parameters
     */

    class BSExecute : public BSConfigBase
    {
        public:
            DECLARE_CLASS(BSExecute);

        public:
            ULONG       _ulExecType;
                            // ORed CFGT_* flags as specified with BSConfigBase;
                            // these are the flags on the "Configure" page this
                            // EXECUTE thing is dependent on. If this is 0,
                            // the execute will always be executed.

            ustring     _ustrExecutable;     // V0.9.9 (2001-03-27) [umoeller]
            ustring     _ustrParams;

            ULONG       _arc;   // error code of Execute()

        public:
            BSExecute(const ustring &ustrExecute,
                      BSClassID &Class = tBSExecute);
            virtual ~BSExecute() {};

            virtual const char* DescribeType();
            virtual ustring DescribeData();

            int Execute(BSUniCodec &codecProcess,
                        BSFileLogger *pLogFile);
    };

    /*
     *@@ BSDeExecute:
     *      represents a user executable (DEEXECUTE tag) to
     *      be executed during de-installation.
     *
     *      This is derived from BSExecute, so run
     *      the parent's Execute() on this.
     *
     *@@added V0.9.9 (2001-03-27) [umoeller]
     */

    class BSDeExecute : public BSExecute
    {
        public:
            DECLARE_CLASS(BSDeExecute);

        public:
            BSDeExecute(const ustring &ustrDeExecute);

            virtual const char* DescribeType();
            // virtual ustring DescribeData();
                    // inherit this from BSExecute, it's the same code
                    // V0.9.18 (2002-03-08) [umoeller]

            static int AddToUndoList(list <BSConfigBase*> &List,
                                     BSDeExecuteResolvedLogger &logger);
    };

    /* ******************************************************************
     *
     *  Kill process class
     *
     ********************************************************************/

    /*
     *@@ BSKillProcess:
     *      represents a process to be killed before
     *      installation can start (KILLPROCESS tag).
     *
     *@@added V0.9.1 (2000-02-07) [umoeller]
     */

    class BSKillProcess : public BSConfigBase
    {
        public:
            DECLARE_CLASS(BSKillProcess);

        public:
            ustring     _ustrKillProcess;

            BSKillProcess(const ustring &ustrKillProcess);
            virtual ~BSKillProcess() {};

            virtual const char* DescribeType();
            virtual ustring DescribeData();

            int KillProcess(BSFileLogger *pLogFile)
                const;

            static int AddToUndoList(list<BSConfigBase*> &List,
                                     BSKillProcessAttrsLogger &logger);
    };

#endif

