
/*
 *@@sourcefile syssound.c:
 *      this has code for querying and manipulating the
 *      system sounds, which are stored in MMPM.INI in
 *      the MMOS2 directory. Also, we have support for
 *      manipulating Warp 4 sound schemes in here too.
 *      This code works on Warp 3 also.
 *
 *      Usage: All OS/2 programs.
 *
 *      Some of this code used to be in XWorkplace's main\common.c,
 *      as far as the regular system sounds were concerned.
 *      However, all this code has been greatly reworked
 *      and extended with V0.9.0.
 *
 *      <B>About system sound configuration data</B>
 *
 *      The current system sounds are stored in ?:\MMOS2\MMPM.INI.
 *
 *      These are the general flags in the "MMPM2_AlarmSoundsData"
 *      application:
 *
 *      --  If "EnableSounds" is FALSE, all system sounds are disabled.
 *          This defaults to TRUE (tested).
 *
 *      --  If "ApplyVolumeToAll" is TRUE, the same volume is used for
 *          all sounds. This defaults to FALSE (tested).
 *
 *      --  If ApplyVolumeToAll is TRUE, "Volume" is used for the
 *          global volume. Otherwise, the individual sound volumes
 *          (below) will be used.
 *
 *      "MMPM2_AlarmSounds" then has all the system sounds. The keys
 *      in that application are numerical indices, which are listed
 *      in syssound.h.
 *
 *      Each sound data block in there consists of three elements:
 *
 +          soundfile#description#volume
 *
 *      where "description" is what is listed in the "Sound" object.
 *      "volume" is only used when "ApplyVolumeToAll" (above) is FALSE.
 *
 *      We have functions in this code file for decoding/setting this
 *      (sndParseSoundData, sndQuerySystemSound, sndWriteSoundData,
 *      sndSetSystemSound).
 *
 *      By contrast (and for no real reason), Warp 4 stores the
 *      "sound schemes" in OS2SYS.INI. I stuck with that for compatibility,
 *      although this prevents several users from having different
 *      sound schemes. What the heck.
 *
 *      Anyways, the "PM_SOUND_SCHEMES_LIST" application is a directory of
 *      schemes, which in turn point to other applications in OS2SYS.INI
 *      which have the actual sound scheme data. All these applications
 *      appear to start with "PM_SOUND_xxx" by convention.
 *
 *      Note that as opposed to the sound data in MMPM.INI, only the
 *      sound file name is stored here (not the three elements as
 *      described above). We slightly extend that mechanism to add an
 *      additional volume data field after the first null byte. This
 *      allows the default WPSound object to still be able to read
 *      the sound scheme data (it apparently uses PrfQueryProfileString)
 *      while the XWorkplace sound object replacement (XWPSound) can
 *      still store volume data as well.
 *
 *      I have created more functions in this code file to easily load and
 *      store sound schemes (sndDoesSchemeExist, sndCreateSoundScheme,
 *      sndLoadSoundScheme, sndDestroySoundScheme).
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 *
 *@@header "helpers\syssound.h"
 *@@added V0.9.0 [umoeller]
 */

#define OS2EMX_PLAIN_CHAR
    // this is needed for "os2emx.h"; if this is defined,
    // emx will define PSZ as _signed_ char, otherwise
    // as unsigned char

#define INCL_DOSPROFILE
#define INCL_DOSERRORS
#define INCL_WINSHELLDATA
#include <os2.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "setup.h"                      // code generation and debugging options

#include "helpers\dosh.h"
#include "helpers\prfh.h"

#include "helpers\syssound.h"

#pragma hdrstop

/*
 *@@category: Helpers\Profile (INI) helpers\System sounds
 *      see syssound.c.
 */

/*
 *@@ sndParseSoundData:
 *      this helper func splits the system sound data
 *      passed to it into three buffers.
 *      Each key data in there has the following format:
 +          soundfile#description#volume.
 *
 *      This is copied into the three specified buffers.
 *      You can set any buffer pointer to NULL if you're
 *      not interested in that data.
 *
 *      Returns the number of items successfully parsed,
 *      which should be 3.
 *
 *@@added V0.9.0 [umoeller]
 *@@changed V0.9.20 (2002-07-03) [umoeller]: optimized
 */

ULONG sndParseSoundData(PCSZ pszSoundData,  // in: INI data from MMPM.INI
                        PSZ pszDescr,    // out: sound description, as displayed
                               // in the "Sound" object (ptr may be NULL)
                        PSZ pszFile,     // out: sound file to (ptr may be NULL)
                        PULONG pulVolume)  // out: sound volume (0-100). Note:
                               // this always returns the individual sound volume,
                               // even if "Global volume" is set in MMPM.INI.
{
    PCSZ        p1 = pszSoundData, p2;
    ULONG       ulrc = 0;
    // get sound file
    if (p2 = strchr(p1, '#'))
    {
        ulrc++;
        if (pszFile)
        {
            strncpy(pszFile, p1, p2 - p1);
            pszFile[p2-p1] = '\0';
        }
        p1 = p2+1;

        // get sound description
        if (p2 = strchr(p1, '#'))
        {
            ulrc++;
            if (pszDescr)
            {
                strncpy(pszDescr, p1, p2-p1);
                pszDescr[p2-p1] = '\0';
            }
            p1 = p2+1;

            // get volume (0-100)
            if (pulVolume)
            {
                // individual volume settings per sound
                *pulVolume = atoi(p1);      // V0.9.20 (2002-07-03) [umoeller]
                // sscanf(p1, "%lu", pulVolume);
                ulrc++;
            }
        }
    }

    return ulrc;
}

/*
 *@@ sndQueryMmpmIniPath:
 *      writes the full path of MMPM.INI into
 *      the specified buffer (e.g. C:\MMOS2\MMPM.INI).
 *
 *@@added V0.9.10 (2001-04-16) [umoeller]
 */

VOID sndQueryMmpmIniPath(PSZ pszMMPM)       // out: fully q'fied MMPM.INI
{
    PSZ pszMMPMPath;
    if (pszMMPMPath = getenv("MMBASE"))  // V0.9.6 (2000-10-16) [umoeller]
    {
        // variable set:
        PSZ p;

        strcpy(pszMMPM, pszMMPMPath); // V0.9.7 (2000-12-17) [umoeller]

        // kill semicolon if present
        if (p = strchr(pszMMPM, ';'))
           *p = 0;

        strcat(pszMMPM, "\\MMPM.INI");
    }
    else
        // variable not set (shouldn't happen): try boot drive
        sprintf(pszMMPM, "%c:\\MMOS2\\MMPM.INI", doshQueryBootDrive());
}

/*
 *@@ sndOpenMmpmIni:
 *      this opens \MMOS2\MMPM.INI on the
 *      boot drive and returns the profile
 *      handle (or NULLHANDLE upon errors).
 *      Use PrfCloseProfile to close the
 *      profile again.
 *
 *@@added V0.9.1 (99-12-19) [umoeller]
 *@@changed V0.9.6 (2000-10-16) [umoeller]: now using MMBASE environment variable
 *@@changed V0.9.6 (2000-10-16) [umoeller]: added proper HAB param
 *@@changed V0.9.7 (2000-12-17) [umoeller]: fixed broken MMBASE handling
 */

HINI sndOpenMmpmIni(HAB hab)
{
    HAB     habDesktop = WinQueryAnchorBlock(HWND_DESKTOP);
    CHAR    szMMPM[CCHMAXPATH];

    sndQueryMmpmIniPath(szMMPM);

    return PrfOpenProfile(habDesktop, szMMPM);
}

/*
 *@@ sndQuerySystemSound:
 *      this gets a system sound from the MMPM.INI file.
 *      usIndex must be the sound to query. The following
 *      MMSOUND_* IDs are declared in syssound.h:
 *
 *      Default system sounds (syssound.h):
 *      --  MMSOUND_WARNING         0
 *      --  MMSOUND_INFORMATION     1
 *      --  MMSOUND_ERROR           2
 *      --  MMSOUND_ANIMATEOPEN     3
 *      --  MMSOUND_ANIMATECLOSE    4
 *      --  MMSOUND_DRAG            5
 *      --  MMSOUND_DROP            6
 *      --  MMSOUND_SYSTEMSTARTUP   7
 *      --  MMSOUND_SHUTDOWN        8
 *      --  MMSOUND_SHREDDER        9
 *      --  MMSOUND_LOCKUP          10
 *      --  MMSOUND_ALARMCLOCK      11
 *      --  MMSOUND_PRINTERROR      12
 *
 *      BTW, these values match those of the WinAlarm call
 *      (WA_* values), but only the first three are documented
 *      in PMREF and pmwin.h (WA_WARNING, WA_NOTE, WA_ERROR).
 *
 *      New XWorkplace system sounds:
 *      --  MMSOUND_XFLD_SHUTDOWN   555
 *      --  MMSOUND_XFLD_RESTARTWPS 556
 *      --  MMSOUND_XFLD_CTXTOPEN   558
 *      --  MMSOUND_XFLD_CTXTSELECT 559
 *      --  MMSOUND_XFLD_CNRDBLCLK  560
 *
 *      The string buffers are recommended to be at least
 *      CCHMAXPATH in size.
 *
 *@@changed V0.9.0 [umoeller]: this used to be cmnQuerySystemSound
 *@@changed V0.9.0 [umoeller]: exported stuff to sndParseSoundData
 *@@changed V0.9.6 (2000-10-16) [umoeller]: added proper HAB param
 */

BOOL sndQuerySystemSound(HAB hab,           // in: caller's anchor block
                         USHORT usIndex,    // in: sound index to query
                         PSZ pszDescr,      // out: sound description, as displayed
                                // in the "Sound" object (ptr may be NULL)
                         PSZ pszFile,       // out: sound file to (ptr may be NULL)
                         PULONG pulVolume)  // out: sound volume (0-100).
                                // If the "Global volume" flag is
                                // set in MMPM.INI, this will return the global
                                // volume instead. Ptr may be NULL also.
{
    BOOL    rc = FALSE;
    HINI    hiniMMPM = sndOpenMmpmIni(hab);

    #ifdef DEBUG_SOUNDS
        _Pmpf((__FUNCTION__ ": entering, hiniMMPM is 0x%lX", hiniMMPM));
    #endif

    if (hiniMMPM)
    {
        CHAR szData[1000];
        CHAR szData2[100];
        CHAR szKey[10];
        sprintf(szKey, "%d", usIndex);
        PrfQueryProfileString(hiniMMPM,
                              MMINIKEY_SOUNDSETTINGS, "EnableSounds",
                              "TRUE",    // default string
                              szData2, sizeof(szData2));
        #ifdef DEBUG_SOUNDS
            _Pmpf(("  sounds enabled: %s", szData2));
        #endif
        if (strcmp(szData2, "TRUE") == 0)
            // sounds enabled at all?
            if (PrfQueryProfileString(hiniMMPM,
                                      MMINIKEY_SYSSOUNDS, szKey,
                                      ".",
                                      szData, sizeof(szData)-1) > 3)
            {
                sndParseSoundData(szData,
                                  pszDescr,
                                  pszFile,
                                  pulVolume);

                // if "global volume" has been enabled,
                // we do not return the value specified
                // here, but the global value
                PrfQueryProfileString(hiniMMPM,
                                      MMINIKEY_SOUNDSETTINGS, "ApplyVolumeToAll",
                                      "FALSE",
                                      szData2, sizeof(szData2));
                if (strcmp(szData2, "FALSE") != 0)
                {
                    // global volume setting for all sounds
                    PrfQueryProfileString(hiniMMPM,
                                          MMINIKEY_SOUNDSETTINGS, "Volume",
                                          "100",
                                          szData2, sizeof(szData2));
                    sscanf(szData2, "%lu", pulVolume);
                }

                rc = TRUE;
            }

        PrfCloseProfile(hiniMMPM);
    }

    return rc;
}

/*
 *@@ sndWriteSoundData:
 *      this sets a system sound in MMPM.INI.
 *      Gets called by sndSetSystemSound. As opposed
 *      to that function, this needs the profile handle
 *      of MMPM.INI.
 *
 *      If (pszDescr == NULL), that sound entry is removed.
 *
 *      Returns the return value of PrfWriteProfileString.
 *
 *@@added V0.9.0 [umoeller]
 *@@changed V0.9.1 (99-12-30) [umoeller]: added delete support
 *@@changed V0.9.6 (2000-10-16) [umoeller]: added MMPM/2 notify
 */

BOOL sndWriteSoundData(HINI hiniMMPM,       // in: MMPM.INI handle (from sndOpenMmpmIni)
                       USHORT usIndex,      // in: sound index
                       PCSZ pszDescr,       // in: sound name or NULL for removal
                       PCSZ pszFile,        // in: sound file
                       ULONG ulVolume)      // in: sound volume
{
    BOOL    brc = FALSE;
    CHAR    szKey[10];

    sprintf(szKey, "%d", usIndex);
    if (pszDescr)
    {
        CHAR szData[1000];
        // format: soundfile#description#volume
        sprintf(szData, "%s#%s#%lu", pszFile, pszDescr, ulVolume);
        brc = PrfWriteProfileString(hiniMMPM,
                                    MMINIKEY_SYSSOUNDS,
                                    szKey,
                                    szData);
    }
    else
        // pszDescr == NULL:
        // delete entry
        brc = PrfWriteProfileString(hiniMMPM,
                                    MMINIKEY_SYSSOUNDS,
                                    szKey,
                                    NULL);

    if (brc)
        // success:
        if (usIndex < 100)
            // one of the default OS/2 sounds has changed:
            // we then need to notify MMPM/2...
            // this is done by calling WinAlarm with 1000+index!
            // sick stuff..
            WinAlarm(HWND_DESKTOP, WA_FILECHANGED + usIndex); // V0.9.6 (2000-10-16) [umoeller]

    return brc;
}

/*
 *@@ sndSetSystemSound:
 *      this sets a system sound in MMPM.INI by
 *      calling sndWriteSoundData.
 *      Returns FALSE if an error occurred.
 *
 *      See sndQuerySystemSound for the parameters.
 *
 *@@changed V0.9.0 [umoeller]: this used to be cmnSetSystemSound
 *@@changed V0.9.0 [umoeller]: exported stuff to sndWriteSoundData
 *@@changed V0.9.6 (2000-10-16) [umoeller]: added proper HAB param
 */

BOOL sndSetSystemSound(HAB hab,
                       USHORT usIndex,
                       PCSZ pszDescr,
                       PCSZ pszFile,
                       ULONG ulVolume)
{
    BOOL    brc = FALSE;
    HINI    hiniMMPM;
    if (hiniMMPM = sndOpenMmpmIni(hab))
    {
        brc = sndWriteSoundData(hiniMMPM, usIndex, pszDescr, pszFile, ulVolume);
        PrfCloseProfile(hiniMMPM);
    }

    return brc;
}

/*
 *@@ sndDoesSchemeExist:
 *      returns TRUE if pszScheme already exists
 *      in OS2SYS.INI.
 *
 *      If so, and *ppszRealScheme is != NULL, it
 *      is set to the key name found. Since the
 *      scheme names are case-insensitive, this
 *      check is necessary to delete the original
 *      scheme for overwrites. The caller is
 *      responsible for free()ing *ppszRealScheme
 *      then.
 *
 *@@added V0.9.0 [umoeller]
 *@@changed V0.9.20 (2002-07-03) [umoeller]: check has to be case-insensitive, fixed; changed prototype
 */

BOOL sndDoesSchemeExist(PCSZ pcszScheme,
                        PSZ *ppszRealScheme)    // out: actual key name (ptr can be NULL)
{
    BOOL fExists = FALSE;
    PSZ pszKeysList;
    if (!prfhQueryKeysForApp(HINI_SYSTEM,
                             MMINIKEY_SOUNDSCHEMES,  // "PM_SOUND_SCHEMES_LIST"
                             &pszKeysList))
    {
        PSZ pKey2 = pszKeysList;
        while (*pKey2)
        {
            if (!stricmp(pKey2, pcszScheme))
            {
                fExists = TRUE;

                if (ppszRealScheme)
                    *ppszRealScheme = strdup(pKey2);

                break;
            }

            pKey2 += strlen(pKey2)+1; // next key
        }

        free(pszKeysList);
    }

    return fExists;

    /* old code V0.9.20 (2002-07-03) [umoeller]
    // check in OS2SYS.INI's scheme list whether that
    // scheme exists already
    PSZ pszExisting = prfhQueryProfileData(HINI_SYSTEM,
                                           MMINIKEY_SOUNDSCHEMES,  // "PM_SOUND_SCHEMES_LIST"
                                           pszScheme,
                                           NULL);
    if (pszExisting)
    {
        free(pszExisting);
        return TRUE;
    }

    return FALSE;
    */

}

/*
 *@@ sndCreateSoundScheme:
 *      this creates a new sound scheme and copies
 *      the current data in MMPM.INI into it.
 *      No check is made for whether that sound
 *      scheme exists already (use sndDoesSchemeExist
 *      for that). Data is overwritten without further
 *      discussion.
 *
 *      Returns:
 *      --  NO_ERROR
 *      --  ERROR_INVALID_HANDLE: hiniMMPM invalid
 *      --  ERROR_NO_DATA: scheme not found.
 *
 *@@added V0.9.0 [umoeller]
 */

APIRET sndCreateSoundScheme(HINI hiniMMPM,      // in: MMPM.INI handle (from sndOpenMmpmIni)
                            PCSZ pszNewScheme)  // in: name of new scheme
{
    APIRET  arc = NO_ERROR;
    CHAR    szNewAppName[200] = "PM_SOUNDS_";

    // create a unique new application name for
    // this scheme in OS2SYS.INI (this is how
    // Warp 4 apparently does it)
    strcat(szNewAppName, pszNewScheme);  // PM_SOUNDS_blahblah
    strupr(szNewAppName);               // PM_SOUNDS_BLAHBLAH

    if (hiniMMPM)
    {
        // get applications list for sounds list in MMPM.INI
        PSZ pszKeysList = NULL;
        if (!(arc = prfhQueryKeysForApp(hiniMMPM,
                                        MMINIKEY_SYSSOUNDS, // "MMPM2_AlarmSounds"
                                        &pszKeysList)))
        {
            PSZ     pKey2 = pszKeysList;

            CHAR    szFile[CCHMAXPATH+50];
            ULONG   ulVolume,
                    ulKeyLen;

            while (*pKey2 != 0)
            {
                // now copy this key to the new sound scheme;
                // however, we can't just copy the whole key,
                // but need to extract the file name and
                // volume first.
                // Warp 4 normally _only_ stores the file name
                // in the sound schemes. Since we want the
                // volume also, we add a null char after the
                // file name and append the volume...

                PSZ pSoundData;
                if (pSoundData = prfhQueryProfileData(hiniMMPM,
                                                      MMINIKEY_SYSSOUNDS, // "MMPM2_AlarmSounds"
                                                      pKey2,
                                                      NULL))
                {
                    sndParseSoundData(pSoundData,
                                      NULL,     // we don't need the description
                                      szFile,
                                      &ulVolume);
                    ulKeyLen = strlen(szFile)+1;    // go beyond null byte
                    ulKeyLen += sprintf(szFile+ulKeyLen,
                                "%lu",
                                ulVolume) + 1;
                    // and write to OS2SYS.INI
                    PrfWriteProfileData(HINI_SYSTEM,
                                        szNewAppName,
                                        pKey2,
                                        szFile,
                                        ulKeyLen);

                    free(pSoundData);
                } // end if (pSoundData)

                pKey2 += strlen(pKey2)+1;
            } // end while (*pKey2 != 0)

            free (pszKeysList);

            // finally, store new scheme in schemes list
            PrfWriteProfileString(HINI_SYSTEM,
                                  MMINIKEY_SOUNDSCHEMES,  // "PM_SOUND_SCHEMES_LIST"
                                  (PSZ)pszNewScheme,      // key is scheme name
                                  szNewAppName);    // data is new OS2SYS.INI application
        }
        else
            arc = ERROR_NO_DATA;
    }
    else
        arc = ERROR_INVALID_HANDLE;

    return arc;
}

/*
 *@@ sndLoadSoundScheme:
 *      this loads the data in pszScheme (OS2SYS.INI)
 *      into MMPM.INI. Existing sound data
 *      in that profile will be overwritten.
 *
 *      Note: Only those sounds in MMPM.INI will be
 *      overwritten for which a corresponding entry
 *      in the sound scheme exists. If it doesn't,
 *      the data in MMPM.INI for _that_ sound only
 *      will _not_ be changed. In other words, existing
 *      sound data will be merged with the scheme data.
 *
 *      If you don't like this, delete the whole
 *      "MMPM2_AlarmSounds" application in MMPM.INI
 *      before calling this function.
 *
 *      Returns:
 *      --  NO_ERROR
 *      --  ERROR_INVALID_HANDLE: hiniMMPM invalid
 *      --  ERROR_NO_DATA: scheme not found.
 *      --  ERROR_BAD_FORMAT: error in MMPM.INI data.
 *
 *@@added V0.9.0 [umoeller]
 *@@changed V0.9.1 (99-12-20) [umoeller]: fixed memory leak
 *@@changed V0.9.6 (2000-10-16) [umoeller]: added MMPM/2 notify
 */

APIRET sndLoadSoundScheme(HINI hiniMMPM,      // in: HINI of ?:\MMOS2\MMPM.INI (PrfOpenProfile)
                          PCSZ pszScheme)     // in: scheme name
{
    APIRET arc = NO_ERROR;

    #ifdef DEBUG_SOUNDS
        _Pmpf(("Entering sndLoadSoundScheme"));
    #endif

    if (hiniMMPM)
    {
        // check in OS2SYS.INI's scheme list whether that
        // scheme exists already
        PSZ pszSchemeAppName = prfhQueryProfileData(HINI_SYSTEM,
                                    MMINIKEY_SOUNDSCHEMES,  // "PM_SOUND_SCHEMES_LIST"
                                    pszScheme,
                                    NULL);
        #ifdef DEBUG_SOUNDS
            _Pmpf(("    pszSchemeAppName: %s", pszSchemeAppName));
        #endif

        if (pszSchemeAppName)
        {
            // now copy keys from OS2SYS.INI to MMPM.INI;
            // since OS2SYS.INI _only_ has the file name
            // (and _maybe_ the individual volume, if it
            // was us who created the sound scheme --
            // see sndCreateSoundScheme above), we need
            // to go thru the MMPM.INI (!) sounds lists
            // and merge the data in there with the
            // corresponding sound data for the current
            // scheme...

            // get applications list for sounds list in MMPM.INI
            PSZ pszMMPMKeysList = NULL;
            if (!(arc = prfhQueryKeysForApp(hiniMMPM,
                                            MMINIKEY_SYSSOUNDS,// "MMPM2_AlarmSounds"
                                            &pszMMPMKeysList)))
            {
                PSZ     pMMPMKey2 = pszMMPMKeysList,
                        pMMPMSoundData,
                        pSchemeSoundData;

                CHAR    szDescription[300];         // from MMPM.INI
                CHAR    szFile[CCHMAXPATH+50];      // from OS2SYS.INI
                ULONG   ulVolume;                   // from MMPM.INI _or_ OS2SYS.INI
                CHAR    szData[1000];

                // go thru keys (numbers)
                while (*pMMPMKey2 != 0)
                {
                    ULONG   cbSchemeSoundData = 0,
                            cbSchemeSoundFile = 0;

                    pMMPMSoundData = prfhQueryProfileData(hiniMMPM,
                                                          MMINIKEY_SYSSOUNDS, // "MMPM2_AlarmSounds"
                                                          pMMPMKey2,
                                                          NULL);
                    pSchemeSoundData = prfhQueryProfileData(HINI_SYSTEM,
                                                            pszSchemeAppName,
                                                            pMMPMKey2,
                                                            &cbSchemeSoundData);

                    if ((pMMPMSoundData) && (pSchemeSoundData))
                    {
                        sndParseSoundData(pMMPMSoundData,
                                          szDescription,
                                          NULL,                // we don't need the file
                                          &ulVolume);
                        // now overwrite this data with scheme data
                        strcpy(szFile,
                               pSchemeSoundData);       // up to first null byte
                        cbSchemeSoundFile = strlen(pSchemeSoundData)+1;
                        if (cbSchemeSoundData > cbSchemeSoundFile)
                            // this means we have an additional volume string
                            // after the null byte (sndCreateSoundScheme);
                            // copy it
                            sscanf(pSchemeSoundData + cbSchemeSoundFile,
                                   "%lu",
                                   &ulVolume);

                        // and write data to MMPM.INI
                        // format: soundfile#description#volume
                        sprintf(szData, "%s#%s#%lu", szFile, szDescription, ulVolume);
                        if (PrfWriteProfileString(hiniMMPM,
                                                  MMINIKEY_SYSSOUNDS, // "MMPM2_AlarmSounds"
                                                  pMMPMKey2,    // key (decimal number)
                                                  szData))
                        {
                            // success:
                            USHORT usIndex = atoi(pMMPMKey2);
                            if (usIndex < 100)
                                // this was one of the default OS/2 sounds:
                                // notify MMPM/2 of the change... see sndWriteSoundData
                                // V0.9.6 (2000-10-16) [umoeller]
                                WinAlarm(HWND_DESKTOP, WA_FILECHANGED + usIndex);
                        }
                    }

                    if (pMMPMSoundData)
                        free(pMMPMSoundData);
                    if (pSchemeSoundData)
                        free(pSchemeSoundData);

                    pMMPMKey2 += strlen(pMMPMKey2)+1;
                } // end while (*pMMPMKey2 != 0)

                free (pszMMPMKeysList);
            }
            else
                arc = ERROR_BAD_FORMAT;

            free(pszSchemeAppName);
        } // end if (pszSchemeAppName)
        else
            arc = ERROR_NO_DATA;
    }
    else
        arc = ERROR_INVALID_HANDLE;

    #ifdef DEBUG_SOUNDS
        _Pmpf(("End of sndLoadSoundScheme, arc: %d", arc));
    #endif
    return arc;
}

/*
 *@@ sndDestroySoundScheme:
 *      destroys pszScheme in OS2SYS.INI;
 *      returns TRUE is pszScheme was found
 *      and deleted.
 *
 *      See xsound.c for explanations.
 *
 *      Returns:
 *      --  NO_ERROR
 *      --  ERROR_NO_DATA: scheme not found.
 *
 *@@added V0.9.0 [umoeller]
 */

APIRET sndDestroySoundScheme(PCSZ pszScheme)
{
    APIRET arc = NO_ERROR;

    // check in OS2SYS.INI's scheme list whether that
    // scheme exists
    PSZ pszExisting;
    if (pszExisting = prfhQueryProfileData(HINI_SYSTEM,
                                           MMINIKEY_SOUNDSCHEMES,  // "PM_SOUND_SCHEMES_LIST"
                                           pszScheme,
                                           NULL))
    {
        // delete whole existing PM_SOUNDS_BLAHBLAH application
        PrfWriteProfileString(HINI_SYSTEM,
                              pszExisting,      // application
                              NULL,
                              NULL);
        // and delete entry in sound schemes list
        PrfWriteProfileString(HINI_SYSTEM,
                              MMINIKEY_SOUNDSCHEMES,  // "PM_SOUND_SCHEMES_LIST"
                              (PSZ)pszScheme,
                              NULL);
        free(pszExisting);
    }
    else
        arc = ERROR_NO_DATA;

    return arc;
}

/*
 *@@ fixMmpmIni:
 *      creates the often missing "End drag"
 *      sound item in MMPM.INI if it doesn't exist.
 *
 *@@added XWP V1.0.9 (2009-10-08) [pr]: @@fixes 691
 */

VOID fixMmpmIni(void)
{
    HINI hiniMmpm;
    CHAR szMMPM[CCHMAXPATH];

    sndQueryMmpmIniPath(szMMPM);
    if (hiniMmpm = PrfOpenProfile(WinQueryAnchorBlock(HWND_DESKTOP), szMMPM))
    {
        ULONG   cbData = 0;
        CHAR    szKey[10];

        sprintf (szKey, "%u", MMSOUND_DROP);
        if (   !PrfQueryProfileSize(hiniMmpm,
                                    MMINIKEY_SYSSOUNDS,
                                    szKey,
                                    &cbData)
            || (cbData ==0))
        {
            PSZ pszPos;

            if (pszPos = strrchr (szMMPM, '\\'))
            {
                strcpy (pszPos, "\\SOUNDS\\DESKTOP\\DSK_DROP.WAV");
                sndWriteSoundData(hiniMmpm, MMSOUND_DROP, "End drag", szMMPM, 40);
            }
        }

        PrfCloseProfile(hiniMmpm);
    }
}

