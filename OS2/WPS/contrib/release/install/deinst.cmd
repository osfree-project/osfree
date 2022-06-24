/*
    Delete XWorkplace installation objects.
    (C) 2001 Ulrich M”ller.

    NOTE: This file is the exact reverse to crobjXXX.cmd
    and instlXXX.cmd, with XXX being the user's country code.

    While we need an NLS script for install to get the object
    titles right, the deinstall script really needs no language
    support, since we can just kill the objects by their
    object IDs.
 */

/* DO NOT CHANGE the following */
call RxFuncAdd 'SysLoadFuncs', 'REXXUTIL', 'SysLoadFuncs'
call SysLoadFuncs

/*********************************************
 *
 * 1) UNDO CROBJxxx.CMD (CONFIG FOLDER)
 *
 *********************************************/

/* items in config folder */
rc = Destroy("<XWP_PACKTREE>");
rc = Destroy("<XWP_TREESIZE>");
rc = Destroy("<XWP_NETSCAPE>");

rc = Destroy("<XWP_SEP1>");

/* config subfolders */
    rc = Destroy("<XWP_CONFIGCFGSHADOW>");
    rc = Destroy("<XWORKPLACE_SETUPCFGSHADOW>");
    rc = Destroy("<XWP_WPSCFGSHADOW>");
    rc = Destroy("<XWP_WPMOUSECFGSHADOW>");
    rc = Destroy("<XWP_WPKEYBCFGSHADOW>");
    rc = Destroy("<XWP_SCREENCFGSHADOW>");
    rc = Destroy("<XWP_KERNELCFGSHADOW>");
    rc = Destroy("<XWP_SEP41>");
    rc = Destroy("<XWP_REFCFGSHADOW>");
    rc = Destroy("<XWP_SEP42>");
rc = Destroy("<XWP_CFG4>");

    rc = Destroy("<XWP_ICOSMALL>");
    rc = Destroy("<XWP_ICONORM>");
    rc = Destroy("<XWP_SHOWALL>");
        rc = Destroy("<XWP_DEFICON>");
        rc = Destroy("<XWP_DEFTREE>");
        rc = Destroy("<XWP_DEFDETLS>");
    rc = Destroy("<XWP_CFG3_1>");
rc = Destroy("<XWP_CFG3>");

    rc = Destroy("<XWP_FOLDERTEMPLATE>");
    rc = Destroy("<XWP_PROGRAMTEMPLATE>");
rc = Destroy("<XWP_CFG2>");

    rc = Destroy("<XWP_CMDSHL>");
    rc = Destroy("<XWP_BASH>");
    rc = Destroy("<XWP_KSH>");
    rc = Destroy("<XWP_OS2WIN>");
    rc = Destroy("<XWP_OS2FULL>");
    rc = Destroy("<XWP_DOSWIN>");
    rc = Destroy("<XWP_DOSFULL>");
rc = Destroy("<XWP_CFG1>");

/* kill config folder itself */
rc = Destroy("<XWP_CONFIG>");

/*********************************************
 *
 * 2) UNDO INSTLxxx.CMD (INSTALL FOLDER)
 *
 *********************************************/

/* the following three added with V0.9.19 */
rc = Destroy("<XWP_LOCKUPSTR>");
rc = Destroy("<XWP_FINDSTR>");
rc = Destroy("<XWP_SHUTDOWNSTR>");

/* XShutdown... changed with V0.9.7 */
rc = Destroy("<XWP_XSHUTDOWN>");

/*  "Setup String" template (added V0.9.5) */
rc = Destroy("<XWP_STRINGTPL>");

/*  "WPS Class List" (added V0.9.2) */
rc = Destroy("<XWP_CLASSLISTMAINSHADOW>");
rc = Destroy("<XWP_CLASSLIST>");

/* "Mouse" shadow (added V0.9.2) */
rc = Destroy("<XWP_MOUSEMAINSHADOW>");
/* "Keyboard" shadow (added V0.9.2) */
rc = Destroy("<XWP_KEYBMAINSHADOW>");
/* "Sound" shadow (added V0.9.2) */
rc = Destroy("<XWP_SOUNDMAINSHADOW>");

/*  "Media" (added V0.9.5) */
rc = Destroy("<XWP_MEDIAMAINSHADOW>");
rc = Destroy("<XWP_MEDIA>");

	/*  "Screen" (added V0.9.2) */
rc = Destroy("<XWP_SCREENMAINSHADOW>");
rc = Destroy("<XWP_SCREEN>");

/*  "OS/2 Kernel" */
rc = Destroy("<XWP_KERNELMAINSHADOW>");
rc = Destroy("<XWP_KERNEL>");

/*  "Workplace Shell" */
rc = Destroy("<XWP_WPSMAINSHADOW>");
rc = Destroy("<XWP_WPS>");

/* "Font folder" ... added with V0.9.7 */
rc = Destroy("<XWP_FONTFOLDERSHADOW>");
rc = Destroy("<XWP_FONTFOLDER>");

/*  XCenter (V0.9.7) */
rc = Destroy("<XWP_XCENTER>");

/*  "XWorkplace Setup" (added V0.9.0) */
rc = Destroy("<XWP_SETUPMAINSHADOW>");
rc = Destroy("<XWORKPLACE_SETUP>");

/* "Readme" shadow (added V0.9.2) */
rc = Destroy("<XWP_READMEMAINSHADOW>");

/* User Guide */
rc = Destroy("<XWP_REF>");

/* "SmartGuide" introduction: this was removed with V0.9.7, but still kill it */
rc = Destroy("<XWP_INTRO>");

/* IN ADDITION, DESTROY TRASH CAN... THIS IS NOT IN THE INSTALL SCRIPT */
rc = Destroy("<XWP_TRASHCAN>");

/* finally, destroy install folder */
rc = Destroy("<XWP_MAINFLDR>");

/*********************************************
 *
 * 3) UNREPLACE/DEREGISTER WPS CLASSES
 *
 *********************************************/

rc = DeRegisterClass("XFldSystem");
rc = DeRegisterClass("XFldWPS");
rc = DeRegisterClass("XWPScreen");
rc = DeRegisterClass("XFldStartup");
rc = DeRegisterClass("XFldShutdown");
rc = DeRegisterClass("XWPSetup");
rc = DeRegisterClass("XWPTrashCan");
rc = DeRegisterClass("XWPTrashObject");
rc = DeRegisterClass("XWPClassList");
rc = DeRegisterClass("XWPMedia");
rc = DeRegisterClass("XCenter");
rc = DeRegisterClass("XWPFontFolder");
rc = DeRegisterClass("XWPFontFile");
rc = DeRegisterClass("XWPFontObject");
rc = DeRegisterClass("XWPString");

rc = UnReplaceClass("WPFileSystem", "XWPFileSystem"); /* V0.9.16 */
rc = UnReplaceClass("WPProgram", "XWPProgram");       /* V0.9.9 */
rc = UnReplaceClass("WPFolder", "XFolder");
rc = UnReplaceClass("WPObject", "XFldObject");
rc = UnReplaceClass("WPDataFile", "XFldDataFile");
rc = UnReplaceClass("WPDisk", "XFldDisk");
rc = UnReplaceClass("WPDesktop", "XFldDesktop");
/* V0.9.16: unreplace XFldProgramFile, which is the old name of XWPProgramFile */
rc = UnReplaceClass("WPProgramFile", "XFldProgramFile");
rc = UnReplaceClass("WPProgramFile", "XWPProgramFile");
rc = UnReplaceClass("WPSound", "XWPSound");
rc = UnReplaceClass("WPMouse", "XWPMouse");
rc = UnReplaceClass("WPKeyboard", "XWPKeyboard");

rc = DeRegisterClass("XWPFileSystem");        /* V0.9.16 */
rc = DeRegisterClass("XWPProgram");           /* V0.9.9 */
rc = DeRegisterClass("XFolder");
rc = DeRegisterClass("XFldObject");
rc = DeRegisterClass("XFldDataFile");
rc = DeRegisterClass("XFldDisk");
rc = DeRegisterClass("XFldDesktop");
/* V0.9.16: unreplace XFldProgramFile, which is the old name of XWPProgramFile */
rc = DeRegisterClass("XFldProgramFile");
rc = DeRegisterClass("XWPProgramFile");
rc = DeRegisterClass("XWPSound");
rc = DeRegisterClass("XWPMouse");
rc = DeRegisterClass("XWPKeyboard");

exit;

/*
 * Destroy:
 *      sneaky little subproc which first sets the NODELETE=NO style
 *      to make sure we can really delete the object and then does a
 *      SysDestroyObject() on it.
 */

Destroy:
parse arg objid

call charout , 'Killing object ID "'objid'"...'

rc = SysSetObjectData(objid, "NODELETE=NO;");
if (rc \= 0) then do
    /* got that: */
    rc = SysDestroyObject(objid);
end

if (rc \= 0) then say " OK"; else say " failed.";

return rc;

/*
 * DeRegisterClass:
 *      little helper to deregister a class.
 */

DeRegisterClass:
parse arg classname

say "Deregistering "classname

rc = SysDeregisterObjectClass(classname);

return rc;

/*
 * UnReplaceClass:
 *      little helper to un-replace a class with another.
 */

UnReplaceClass:
parse arg oldclass, newclass

say "Undoing replacement of "oldclass" with "newclass;

"..\bin\repclass" oldclass newclass

return rc;
