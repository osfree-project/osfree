/* $Id: setenv.cmd,v 1.2 2004/03/21 04:22:53 bird Exp $
 *
 * This script defines and setup the build environment for building JFS.
 *
 * You'll have to edit this to set the correct paths.
 *
 */

    /*
     * Toolkit (4.5) main directory.
     */
    sTKMain    = 'f:\toolkit\v4.5';

    /*
     * Device Driver Kit (DDK) (v4.0+) base (important not main) directory.
     */
    sDDKBase    = 'f:\ddk_os2\200402\ddk\base';

    /*
     * IBM C/C++ Compiler and Tools Version 3.6.5 main directory.
     */
    sCxxMain    = 'f:\vacpp\v3.65_os2';

    /*
     * Microsoft C v6.0a main directory.
     */
    sMSCMain    = 'f:\DDK_os2\200204\toolkits\msc60';


    fRm = 0;

    /**
     * Set environment - don't change locally!
     *
     * Note: This is done in this inefficient was to allow this file to be
     *       invoked serveral times and still have the exactly same environment.
     **/
    call EnvSet      fRm, 'tkmain',  sTkMain;
    call EnvSet      fRm, 'ddkbase', sDDKBase;
    call EnvSet      fRm, 'cxxmain', sCxxMain;
    call EnvSet      fRm, 'mscmain', sMSCMain;

    call EnvSet      fRm, 'VACPP',  sCxxMain;

    call EnvSet      fRm, 'UFILEVER', '"@#netlabs.org:1.00.1#@Utils OpenJFS. Compiled by Pavel Shtemenko"'

    call EnvAddFront fRm, 'path',        sMSCMain'\binp;'
    call EnvAddFront fRm, 'path',        sDDKBase'\tools;'
    call EnvAddFront fRm, 'path',        sCxxMain'\bin;'
    call EnvAddFront fRm, 'path',        sTkMain'\bin;'

    call EnvAddFront fRm, 'dpath',       sCxxMain'\help;'
    call EnvAddFront fRm, 'dpath',       sCxxMain'\local;'
    call EnvAddFront fRm, 'dpath',       sTkMain'\book;'
    call EnvAddFront fRm, 'dpath',       sTkMain'\msg;'

    call EnvAddFront fRm, 'beginlibpath', sCxxMain'\runtime;'
    call EnvAddFront fRm, 'beginlibpath', sCxxMain'\dll;'
    call EnvAddFront fRm, 'beginlibpath', sTkMain'\dll;'

    call EnvAddFront fRm, 'help',        sTkMain'\help;'
    call EnvAddFront fRm, 'bookshelf',   sTkMain'\archived;'
    call EnvAddFront fRm, 'bookshelf',   sTkMain'\book;'

    call EnvAddFront fRm, 'nlspath',     sCxxMain'\msg\%N;'
    call EnvAddFront fRm, 'nlspath',     sTkMain'\msg\%N;'
    call EnvAddEnd   fRm, 'ulspath',     sTkMain'\language;'

    call EnvAddFront fRm, 'include',     sDDKBase'\h;'
    call EnvAddFront fRm, 'include',     sCxxMain'\include;'
    call EnvAddFront fRm, 'include',     sTkMain'\H;'
    call EnvAddFront fRm, 'include',     sTkMain'\H\GL;'
    call EnvAddFront fRm, 'include',     sTkMain'\SPEECH\H;'
    call EnvAddFront fRm, 'include',     sTkMain'\H\RPC;'
    call EnvAddFront fRm, 'include',     sTkMain'\H\NETNB;'
    call EnvAddFront fRm, 'include',     sTkMain'\H\NETINET;'
    call EnvAddFront fRm, 'include',     sTkMain'\H\NET;'
    call EnvAddFront fRm, 'include',     sTkMain'\H\ARPA;'

    call EnvAddEnd   fRm, 'lib',         sTkMain'\SAMPLES\MM\LIB;'
    call EnvAddEnd   fRm, 'lib',         sTkMain'\SPEECH\LIB;'
    call EnvAddFront fRm, 'lib',         sCxxMain'\lib;'
    call EnvAddFront fRm, 'lib',         sDDKBase'\lib;'
    call EnvAddFront fRm, 'lib',         sTkMain'\lib;'

    call EnvAddFront fRm, 'helpndx',     'EPMKWHLP.NDX;'

    call EnvAddFront fRm, 'ipfc',        sCxxMain'\ipfc;'
    call EnvAddFront fRm, 'ipfc',        sTkMain'\ipfc;'

    call EnvSet      fRm, 'LANG',        'en_us'
    call EnvSet      fRm, 'CPP_DBG_LANG', 'CPP'

    call EnvSet      fRm, 'CPREF',       'CP1.INF+CP2.INF+CP3.INF'
    call EnvSet      fRm, 'GPIREF',      'GPI1.INF+GPI2.INF+GPI3.INF+GPI4.INF'
    call EnvSet      fRm, 'MMREF',       'MMREF1.INF+MMREF2.INF+MMREF3.INF'
    call EnvSet      fRm, 'PMREF',       'PM1.INF+PM2.INF+PM3.INF+PM4.INF+PM5.INF'
    call EnvSet      fRm, 'WPSREF',      'WPS1.INF+WPS2.INF+WPS3.INF'

    /*
    call EnvSet      fRm, 'CAT_MACHINE', 'COM1:57600'
    call EnvSet      fRm, 'CAT_HOST_BIN_PATH', TKMAIN'\BIN'
    call EnvSet      fRm, 'CAT_COMMUNICATION_TYPE', 'ASYNC_SIGBRK'
    call EnvSet      fRm, 'CAT_HOST_SOURCE_PATH',TKMAIN'\BIN;'
    */

    exit(0);



/**
 * Procedure section
 **/

/**
 * Add sToAdd in front of sEnvVar.
 * Note: sToAdd now is allowed to be alist!
 *
 * Known features: Don't remove sToAdd from original value if sToAdd
 *                 is at the end and don't end with a ';'.
 */
EnvAddFront: procedure
    parse arg fRM, sEnvVar, sToAdd, sSeparator

    /* sets default separator if not specified. */
    if (sSeparator = '') then sSeparator = ';';

    /* checks that sToAdd ends with an ';'. Adds one if not. */
    if (substr(sToAdd, length(sToAdd), 1) <> sSeparator) then
        sToAdd = sToAdd || sSeparator;

    /* check and evt. remove ';' at start of sToAdd */
    if (substr(sToAdd, 1, 1) = ';') then
        sToAdd = substr(sToAdd, 2);

    /* loop thru sToAdd */
    rc = 0;
    i = length(sToAdd);
    do while i > 1 & rc = 0
        j = lastpos(sSeparator, sToAdd, i-1);
        rc = EnvAddFront2(fRM, sEnvVar, substr(sToAdd, j+1, i - j), sSeparator);
        i = j;
    end

return rc;

/**
 * Add sToAdd in front of sEnvVar.
 *
 * Known features: Don't remove sToAdd from original value if sToAdd
 *                 is at the end and don't end with a ';'.
 */
EnvAddFront2: procedure
    parse arg fRM, sEnvVar, sToAdd, sSeparator

    /* sets default separator if not specified. */
    if (sSeparator = '') then sSeparator = ';';

    /* checks that sToAdd ends with a separator. Adds one if not. */
    if (substr(sToAdd, length(sToAdd), 1) <> sSeparator) then
        sToAdd = sToAdd || sSeparator;

    /* check and evt. remove the separator at start of sToAdd */
    if (substr(sToAdd, 1, 1) = sSeparator) then
        sToAdd = substr(sToAdd, 2);

    /* Get original variable value */
    sOrgEnvVar = EnvGet(sEnvVar);

    /* Remove previously sToAdd if exists. (Changing sOrgEnvVar). */
    i = pos(translate(sToAdd), translate(sOrgEnvVar));
    if (i > 0) then
        sOrgEnvVar = substr(sOrgEnvVar, 1, i-1) || substr(sOrgEnvVar, i + length(sToAdd));

    /* set environment */
    if (fRM) then
        return EnvSet(0, sEnvVar, sOrgEnvVar);
return EnvSet(0, sEnvVar, sToAdd||sOrgEnvVar);


/**
 * Add sToAdd as the end of sEnvVar.
 * Note: sToAdd now is allowed to be alist!
 *
 * Known features: Don't remove sToAdd from original value if sToAdd
 *                 is at the end and don't end with a ';'.
 */
EnvAddEnd: procedure
    parse arg fRM, sEnvVar, sToAdd, sSeparator

    /* sets default separator if not specified. */
    if (sSeparator = '') then sSeparator = ';';

    /* checks that sToAdd ends with a separator. Adds one if not. */
    if (substr(sToAdd, length(sToAdd), 1) <> sSeparator) then
        sToAdd = sToAdd || sSeparator;

    /* check and evt. remove ';' at start of sToAdd */
    if (substr(sToAdd, 1, 1) = sSeparator) then
        sToAdd = substr(sToAdd, 2);

    /* loop thru sToAdd */
    rc = 0;
    i = length(sToAdd);
    do while i > 1 & rc = 0
        j = lastpos(sSeparator, sToAdd, i-1);
        rc = EnvAddEnd2(fRM, sEnvVar, substr(sToAdd, j+1, i - j), sSeparator);
        i = j;
    end

return rc;

/**
 * Add sToAdd as the end of sEnvVar.
 *
 * Known features: Don't remove sToAdd from original value if sToAdd
 *                 is at the end and don't end with a ';'.
 */
EnvAddEnd2: procedure
    parse arg fRM, sEnvVar, sToAdd, sSeparator

    /* sets default separator if not specified. */
    if (sSeparator = '') then sSeparator = ';';

    /* checks that sToAdd ends with a separator. Adds one if not. */
    if (substr(sToAdd, length(sToAdd), 1) <> sSeparator) then
        sToAdd = sToAdd || sSeparator;

    /* check and evt. remove separator at start of sToAdd */
    if (substr(sToAdd, 1, 1) = sSeparator) then
        sToAdd = substr(sToAdd, 2);

    /* Get original variable value */
    sOrgEnvVar = EnvGet(sEnvVar);

    if (sOrgEnvVar <> '') then
    do
        /* Remove previously sToAdd if exists. (Changing sOrgEnvVar). */
        i = pos(translate(sToAdd), translate(sOrgEnvVar));
        if (i > 0) then
            sOrgEnvVar = substr(sOrgEnvVar, 1, i-1) || substr(sOrgEnvVar, i + length(sToAdd));

        /* checks that sOrgEnvVar ends with a separator. Adds one if not. */
        if (sOrgEnvVar = '') then
            if (right(sOrgEnvVar,1) <> sSeparator) then
                sOrgEnvVar = sOrgEnvVar || sSeparator;
    end

    /* set environment */
    if (fRM) then return EnvSet(0, sEnvVar, sOrgEnvVar);
return EnvSet(0, sEnvVar, sOrgEnvVar||sToAdd);


/**
 * Sets sEnvVar to sValue.
 */
EnvSet: procedure
    parse arg fRM, sEnvVar, sValue

    /* if we're to remove this, make valuestring empty! */
    if (fRM) then
        sValue = '';
    sEnvVar = translate(sEnvVar);

    /*
     * Begin/EndLibpath fix:
     *      We'll have to set internal these using both commandline 'SET'
     *      and internal VALUE in order to export it and to be able to
     *      get it (with EnvGet) again.
     */
    if ((sEnvVar = 'BEGINLIBPATH') | (sEnvVar = 'ENDLIBPATH')) then
    do
        if (length(sValue) >= 1024) then
            say 'Warning: 'sEnvVar' is too long,' length(sValue)' char.';
        return SysSetExtLibPath(sValue, substr(sEnvVar, 1, 1));
    end

    if (length(sValue) >= 1024) then
    do
        say 'Warning: 'sEnvVar' is too long,' length(sValue)' char.';
        say '    This may make CMD.EXE unstable after a SET operation to print the environment.';
    end
    sRc = VALUE(sEnvVar, sValue, 'OS2ENVIRONMENT');
return 0;

/**
 * Gets the value of sEnvVar.
 */
EnvGet: procedure
    parse arg sEnvVar
    if ((translate(sEnvVar) = 'BEGINLIBPATH') | (translate(sEnvVar) = 'ENDLIBPATH')) then
        return SysQueryExtLibPath(substr(sEnvVar, 1, 1));
return value(sEnvVar,, 'OS2ENVIRONMENT');


/**
 *  Workaround for bug in CMD.EXE.
 *  It messes up when REXX have expanded the environment.
 */
FixCMDEnv: procedure
    /* check for 4OS2 first */
    Address CMD 'set 4os2test_env=%@eval[2 + 2]';
    if (value('4os2test_env',, 'OS2ENVIRONMENT') = '4') then
        return 0;

    /* force environment expansion by setting a lot of variables and freeing them. */
    do i = 1 to 100
        Address CMD '@set dummyenvvar'||i'=abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ';
    end
    do i = 1 to 100
        Address CMD '@set dummyenvvar'||i'=';
    end
return 0;

