/*  freshini.cmd

    Registers the eWorkplace classes (and class replacements)
    in the specified user INI file. The WPS will pick up the
    changes after the next WPS startup with that INI file.

    (C) 2002 Ulrich M”ller
 */

call RxFuncAdd 'SysLoadFuncs', 'REXXUTIL', 'SysLoadFuncs'
call SysLoadFuncs

parse arg inifile
if inifile == "" then do
    Say "freshini.cmd (C) 2001-2002 Ulrich M”ller"
    Say "Sets up the INI file for a new eWorkplace installation."

    Say "Usage: freshini <inifile>";
    Say "with <inifile> being the full path specification of an INI file."
    Say "Specify USER to manipulate the OS2.INI which is currently in use.";
    exit;
end

/* get the directory from where we're started */
parse source mydir;
parse var mydir x1 x2 mydir;

mydir = filespec("D", mydir)||filespec("P", mydir);
if (right(mydir, 1) = "\") then
    mydir = left(mydir, length(mydir)-1);

say 'freshini.cmd: install subdirectory is "'mydir'"'

/* mydir now has the install subdir of the xwp dir...
        note that this works even if we are started from
        a different directory;
   replace "install" with "bin" to get the directory
   of xfldr.dll */

p = pos("\INSTALL", translate(mydir));
basedir = left(mydir, p - 1);

/* basedir now has the base dir without trailing \ */

bindir = basedir||"\bin";

xfldrdll = bindir||"\xfldr.dll";

rc = SysINI(inifile, "XWorkplace", "JustInstalled", "1");
if (rc == 0) then do
    Say "Error writing to "inifile". Terminating.";
    exit;
end

nlscode = 001;

/* check if xfldr049.dll exists in bin */

say 'Searching for NLS DLLs in "'bindir'"...';

rc = SysFileTree(bindir||"\xfldr???.dll", nlsdlls, "FO");
do i = 1 to nlsdlls.0
    p2 = lastpos("\", nlsdlls.i);
    thisdll = substr(nlsdlls.i, p2 + 1);
    thisdll = left(thisdll, pos(".", thisdll) - 1);
    /* rule out xfldr.dll which is still found by the above mask */
    if (length(thisdll) == 8) then do
        say '  Found NLS DLL "'thisdll'"';
        nlsthis = substr(thisdll, 6, 3);
        if (nlsthis > nlscode) then do
            nlscode = nlsthis;
        end
    end
end

/* set language code */

say 'Setting language code "'nlscode'"';
rc = SysINI(inifile, "XWorkplace", "Language", nlscode || '00'x);

/* set base path */

say 'Setting base path "'basedir'"';
rc = SysINI(inifile, "XWorkplace", "XFolderPath", basedir || '00'x);

rc = RegisterClass("XWPFileSystem");        /* V0.9.16 */
rc = RegisterClass("XFolder");
rc = RegisterClass("XFldObject");
rc = RegisterClass("XFldDataFile");
rc = RegisterClass("XFldDisk");
rc = RegisterClass("XFldDesktop");
rc = RegisterClass("XWPProgramFile");       /* class renamed V0.9.16 */
rc = RegisterClass("XWPSound");
rc = RegisterClass("XWPString");
rc = RegisterClass("XWPMouse");
rc = RegisterClass("XWPKeyboard");
rc = RegisterClass("XWPProgram");           /* V0.9.9 */

rc = ReplaceClass("WPFileSystem", "XWPFileSystem"); /* V0.9.16 */
rc = ReplaceClass("WPFolder", "XFolder");
rc = ReplaceClass("WPObject", "XFldObject");
rc = ReplaceClass("WPDataFile", "XFldDataFile");
rc = ReplaceClass("WPDisk", "XFldDisk");
rc = ReplaceClass("WPDesktop", "XFldDesktop");
rc = ReplaceClass("WPProgramFile", "XWPProgramFile"); /* class renamed V0.9.16 */
rc = ReplaceClass("WPSound", "XWPSound");
rc = ReplaceClass("WPMouse", "XWPMouse");
rc = ReplaceClass("WPKeyboard", "XWPKeyboard");
rc = ReplaceClass("WPProgram", "XWPProgram");       /* V0.9.9 */

/* rc = RegisterClass("XFldSystem"); */
rc = RegisterClass("XFldWPS");
rc = RegisterClass("XWPScreen");
rc = RegisterClass("XFldStartup");
rc = RegisterClass("XFldShutdown");
/* rc = RegisterClass("XWPSetup"); */
rc = RegisterClass("XWPTrashCan");
rc = RegisterClass("XWPTrashObject");
/* rc = RegisterClass("XWPClassList"); */
/* rc = RegisterClass("XWPMedia"); */
rc = RegisterClass("XCenter");
rc = RegisterClass("XWPFontFolder");
rc = RegisterClass("XWPFontFile");
rc = RegisterClass("XWPFontObject");

exit;

/*
 * RegisterClass:
 *      little helper to register a class from xfldr.dll.
 *
 *      This adds a new key under "PM_InstallClass", with
 *      the key name being the class name and the data the
 *      DLL (which in this case is xfldr.dll, full-pathed above).
 */

RegisterClass:
parse arg classname

say "Registering "classname" in "xfldrdll;

rc = SysIni(inifile, "PM_InstallClass", classname, xfldrdll);

return rc;

/*
 * ReplaceClass:
 *      little helper to replace a class with another.
 *
 *      This adds a new key under "PM_InstallClassReplacement",
 *      with the key name being the replacee and the data the
 *      replacement class.
 */

ReplaceClass:
parse arg oldclass, newclass

say "Replacing "oldclass" with "newclass;

rc = SysIni(inifile, "PM_InstallClassReplacement", oldclass, newclass);

return rc;

