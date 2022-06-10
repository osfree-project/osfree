
/*
 * bs_config.h:
 *      header file for bs_config.cpp. See remarks there.
 *
 *@@include #define INCL_WINWORKPLACE
 *@@include #include <os2.h>
 *@@include #include "base\bs_errors.h"
 *@@include #include "base\bs_logger.h"
 *@@include #include "base\bs_config.h"
 */

/*
 *      This file Copyright (C) 1999-2002 Ulrich M”ller.
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation, in version 2 as it comes in the COPYING
 *      file of this distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 */

#ifndef WARPIN_CONFIG_HEADER_INCLUDED
    #define WARPIN_CONFIG_HEADER_INCLUDED

    /* ******************************************************************
     *
     *   BSConfigExcpt declaration
     *
     ********************************************************************/

    /*
     *@@ BSConfigExcpt:
     *      exception thrown by many of the configuration
     *      methods.
     *
     *      Note that the various error codes are declared in
     *      bs_config_impl.h.
     *
     *@@changed V0.9.0 (99-10-31) [umoeller]: made this a child of BSExcptBase
     */

    class BSConfigExcpt : public BSExcptBase
    {
        public:
            int     _iErrorCode;
            int     _iData;          // with CFGSYS_OPEN, this has the APIRET

            /*
             *@@ BSConfigExcpt:
             *      constructor 1, for integer data only
             */

            BSConfigExcpt(int iErrorCode_, int iData_)
            {
                _iErrorCode = iErrorCode_;
                _iData = iData_;
            }

            /*
             *@@ BSConfigExcpt:
             *      constructor 2, report a string.
             */

            BSConfigExcpt(int iErrorCode_, const ustring &ustrSubstring_)
            {
                _iErrorCode = iErrorCode_;
                _iData = 0;
                _ustrDescription = ustrSubstring_;
            }

            /*
             *@@ BSConfigExcpt:
             *      constructor 3, both integer and string.
             */

            BSConfigExcpt(int iErrorCode_, int iData_, const ustring &ustrSubstring_)
            {
                _iErrorCode = iErrorCode_;
                _iData = iData_;
                _ustrDescription = ustrSubstring_;
            }
    };

    /* ******************************************************************
     *
     *  Derived logger classes
     *
     ********************************************************************/

    /*
     *@@ BSAttrLoggerBase:
     *      class derived from BSMemLoggerBase for storing string attributes,
     *      as specified in a PCK attribute.
     *      This is identical to BSMemLoggerBase and not used either, only
     *      for deriving more logger classes.
     */

    class BSAttrLoggerBase : public BSMemLoggerBase { };

    /*
     *@@ BSDoneLoggerBase:
     *      class derived from BSMemLoggerBase for storing the actual system
     *      changes made according to a PCK attribute. These loggers
     *      are filled during system configuration (after all files
     *      have been unpacked) while the system is being modified.
     *      This data is stored in the database so that configuration
     *      can be undone upon de-installation.
     *
     *      This is identical to BSMemLoggerBase and not used either, only
     *      for deriving more logger classes.
     */

    class BSDoneLoggerBase : public BSMemLoggerBase { };

    /*
     *@@ BSCfgSysAttrsLogger:
     *      logger for storing CONFIG.SYS manipulation attributes.
     *
     *      This stores strings just as in the CONFIGSYS
     *      attribute of the PCK tag.
     *
     *@@added V0.9.1 (2000-01-05) [umoeller]
     */

    class BSCfgSysAttrsLogger : public BSAttrLoggerBase { };

    /*
     *@@ BSCfgSysDoneLogger:
     *      this logs changes made to CONFIG.SYS.
     *      This is derived from BSDoneLoggerBase and only
     *      declared to make sure that it's a CONFIG.SYS
     *      logger passed to the BSConfigSys methods.
     *
     *      Format of the entries:
     *          slightly complicated, see BSCfgSysManip::AddToUndoList.
     */

    class BSCfgSysDoneLogger : public BSDoneLoggerBase { };

    /*
     *@@ BSRegisterAttrsLogger:
     *      logger for storing register-class attributes.
     *
     *      This stores strings just as in the REGISTERCLASS
     *      attribute of the PCK tag.
     *
     *@@added V0.9.1 (2000-01-05) [umoeller]
     */

    class BSRegisterAttrsLogger : public BSAttrLoggerBase { };

    /*
     *@@ BSReplaceAttrsLogger:
     *      logger for storing replace-class attributes.
     *
     *      This stores strings just as in the REPLACECLASS
     *      attribute of the PCK tag.
     *
     *@@added V0.9.1 (2000-01-05) [umoeller]
     */

    class BSReplaceAttrsLogger : public BSAttrLoggerBase { };

    /*
     *@@ BSRegisterDoneLogger:
     *      this logs registered WPS classes.
     *      This is derived from BSDoneLoggerBase and only
     *      declared to make sure that only a suitable
     *      logger is passed to the BSRegisterClass methods.
     *
     *      Format of the entries:
     *      This logger stores plain WPS class names only
     *      so that these can be de-registered again with
     *      BSDeregisterClass.
     */

    class BSRegisterDoneLogger : public BSDoneLoggerBase { };

    /*
     *@@ BSReplaceDoneLogger:
     *      this logs replaced WPS classes.
     *      This is derived from BSDoneLoggerBase and only
     *      declared to make sure that only a suitable
     *      logger is passed to the BSReplaceClass methods.
     *
     *      Format of the entries:
     *          "oldclass|newclass"
     *
     *      That's the same as with BSReplaceAttrsLogger.
     */

    class BSReplaceDoneLogger : public BSDoneLoggerBase { };

    /*
     *@@ BSWPSObjectAttrsLogger:
     *      logger for storing create-WPS-objects attributes.
     *
     *      This stores strings just as in the CREATEOBJECT
     *      attribute of the PCK tag.
     *
     *@@added V0.9.1 (2000-01-05) [umoeller]
     */

    class BSWPSObjectAttrsLogger : public BSAttrLoggerBase { };

    /*
     *@@ BSWPSObjectsDoneLogger:
     *      this logs created WPS objects.
     *      This is derived from BSDoneLoggerBase and only
     *      declared to make sure that only a suitable
     *      logger is passed to the BSCreateWPSObject methods.
     *
     *      Format of the entries:
     *      This stores the WPS object ID (a la "<WP_BLAH>"
     *      only as specified in the setup string in the
     *      BSWPSObjectAttrsLogger.
     */

    class BSWPSObjectsDoneLogger : public BSDoneLoggerBase { };

    /*
     *@@ BSClearPrfAttrsLogger:
     *      logger for storing CLEARPROFILE attributes of PCK tag.
     *
     *      This stores strings just as in the CLEARPROFILE
     *      attribute of the PCK tag.
     *
     *      Note that there is no corresponding "done" logger
     *      for the CLEARPROFILE tag because this is used during
     *      de-installation only.
     *
     *@@added V0.9.1 (2000-02-07) [umoeller]
     */

    class BSClearPrfAttrsLogger : public BSAttrLoggerBase { };

    /*
     *@@ BSWritePrfAttrsLogger:
     *      logger for storing WRITEPROFILE attributes of PCK tag.
     *
     *      This stores strings just as in the WRITEPROFILE
     *      attribute of the PCK tag.
     *
     *@@added V0.9.1 (2000-02-07) [umoeller]
     */

    class BSWritePrfAttrsLogger : public BSAttrLoggerBase { };

    /*
     *@@ BSWritePrfsDoneLogger:
     *      this logs "write profile" entries.
     *      This is derived from BSDoneLoggerBase and only
     *      declared to make sure that only a suitable
     *      logger is passed to ### methods.
     *
     *@@added V0.9.1 (2000-02-07) [umoeller]
     */

    // class BSWritePrfsDoneLogger : public BSDoneLoggerBase { };

    /*
     *@@ BSExecuteAttrsLogger:
     *      logger for storing EXECUTE attributes of PCK tag.
     *
     *      This stores strings just as in the EXECUTE
     *      attribute of the PCK tag.
     *
     *@@added V0.9.1 (2000-02-01) [umoeller]
     */

    class BSExecuteAttrsLogger : public BSAttrLoggerBase { };

    /*
     *@@ BSExecuteDoneLogger:
     *      this logs executed user programs.
     *      This is derived from BSDoneLoggerBase and only
     *      declared to make sure that only a suitable
     *      logger is passed to ### methods.
     *
     *@@added V0.9.1 (2000-02-01) [umoeller]
     */

    class BSExecuteDoneLogger : public BSDoneLoggerBase { };

    /*
     *@@ BSDeExecuteAttrsLogger:
     *      logger for storing DEEXECUTE attributes of PCK tag.
     *
     *      This stores strings just as in the DEEXECUTE
     *      attribute of the PCK tag.
     *
     *@@added V0.9.1 (2000-02-01) [umoeller]
     */

    class BSDeExecuteAttrsLogger : public BSAttrLoggerBase { };

    /*
     *@@ BSExecuteResolvedLogger:
     *      logger for storing _resolved_ DEEXECUTE attributes
     *      of a PCK tag. This takes the function of a "Done"
     *      logger, but since there is really nothing "done"
     *      during install except resolving macros, this
     *      is called a "ResolvedLogger".
     *
     *      Executable and parameter are tightly separated by "|".
     *
     *@@added V0.9.9 (2001-03-27) [umoeller]
     */

    class BSDeExecuteResolvedLogger : public BSDoneLoggerBase { };

    /*
     *@@ BSKillProcessAttrsLogger:
     *      logger for storing KILLPROCESS attributes of PCK tag.
     *
     *@@added V0.9.1 (2000-02-12) [umoeller]
     */

    class BSKillProcessAttrsLogger : public BSAttrLoggerBase { };

    /* ******************************************************************
     *
     *  BSConfigBase class
     *
     ********************************************************************/

    // declare all BSConfigBase descendant classes
    // so we can return pointers of them in BSConfigBase
    class BSCfgSysManip;
    class BSRegisterClass;
    class BSDeregisterClass;
    class BSReplaceClass;
    class BSUnreplaceClass;
    class BSCreateWPSObject;
    class BSDeleteWPSObject;
    class BSClearProfile;
    class BSWriteProfile;
    class BSExecute;
    class BSDeExecute;      // V0.9.9 (2001-03-27) [umoeller]
    class BSKillProcess;

    // flags for BSConfigBase._ulConfigClassType;
    // in that context, only one of these is used,
    // however, we also use these flags in
    // BSExecute._ulExecType, where these may be
    // ORed
    #define CFGT_CFGSYS             0x0001
    #define CFGT_REGISTERCLASS      0x0002
    #define CFGT_DEREGISTERCLASS    0x0004
    #define CFGT_REPLACECLASS       0x0008
    #define CFGT_UNREPLACECLASS     0x0010
    #define CFGT_CREATEOBJECT       0x0020
    #define CFGT_DELETEOBJECT       0x0040
    #define CFGT_CLEARPROFILE       0x0080
    #define CFGT_WRITEPROFILE       0x0100
    #define CFGT_EXECUTE            0x0200
    #define CFGT_KILLPROCESS        0x0400
    #define CFGT_DEEXECUTE          0x0800

    /*
     *@@ BSConfigBase:
     *      abstract parent class from which all other config
     *      classes are derived. This cannot be instantiated
     *      because the constructor is protected.
     *
     *      We use an abstract base class for the various
     *      subclasses of this class so we can always use
     *      this abstract base class in STL lists. Everything
     *      else leads to terrible code bloat.
     *
     *      Besides, this gives us a common interface for
     *      selecting configuration items during deinstallation.
     *
     *      See bs_config.cpp for more on the various config
     *      classes.
     *
     *@@added V0.9.1 (2000-02-07) [umoeller]
     */

    class BSConfigBase : public BSRoot
    {
        public:
            DECLARE_CLASS(BSConfigBase);

        protected:
            ULONG       _ulConfigClassType;
                        // CFGT_* flag specifying the actual BSConfigBase
                        // subclass of which this is an instance; use the
                        // Is* methods to check this

            BOOL        _fSelected;
                        // if TRUE, the config instance has been selected
                        // for installation or de-installation

            /*
             *@@ BSConfigBase:
             *      the constructor.
             */

            BSConfigBase(ULONG ulConfigType_,
                         BSClassID &Class)
                : BSRoot(Class)
            {
                _ulConfigClassType = ulConfigType_;
                _fSelected = TRUE;
            }

        public:

            /*
             *@@ DescribeType:
             *      describes the config object to the GUI...
             *      this is visible in various config dialogs.
             *
             *      Abstract method, must be overridden by
             *      subclasses.
             */

            virtual const char* DescribeType() = 0;

            /*
             *@@ DescribeData:
             *      must return a description of the object's
             *      data, which is displayed in GUI dialogs.
             *
             *      Abstract method, must be overridden by
             *      subclasses.
             */

            virtual ustring DescribeData()
            {
                ustring ustr;
                ustr.assignUtf8("unknown");
                return (ustr);
            }

            /*
             *@@ IsSelected:
             *      returns the _fSelected member status.
             *
             *@@added V0.9.4 (2000-07-01) [umoeller]
             */

            BOOL IsSelected(VOID)
            {
                return (_fSelected);
            }

            /*
             *@@ Select:
             *      sets the _fSelected member.
             *
             *@@added V0.9.4 (2000-07-01) [umoeller]
             */

            VOID Select(BOOL fSelect)
            {
                _fSelected = fSelect;
            }
    };

#endif

