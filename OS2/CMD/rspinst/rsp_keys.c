/*!
 * @file rsp_keys.c
 *
 * @brief Static keyword table for the OS/2 response file parser.
 *
 * Every entry carries the normative source from which the keyword,
 * its range and its meaning are taken. Sources are:
 *
 *   - "SG24-2010-00 §7.1.2"  — IBM Redbook SG24-2010-00, section
 *     7.1.2, "Detailed Keyword Description".
 *   - "SG24-2010-00 §7.1.3"  — IBM Redbook SG24-2010-00, section
 *     7.1.3, "New Keywords in OS/2 Warp 4".
 *   - "SG24-2010-00 Fig. 56" — IBM Redbook SG24-2010-00, Figure 56,
 *     "Example of an OS/2 Response File (Part 1 of 2)".
 *   - "SG24-2010-00 Fig. 57" — IBM Redbook SG24-2010-00, Figure 57,
 *     "Example of an OS/2 Response File (Part 2 of 2)".
 *   - "SG24-2010-00 §7.1.3 (note)" — the note stating that the
 *     keyword is not supported.
 *
 * @par References
 *   - IBM Redbook SG24-2010-00, section 7.1.2.
 *   - IBM Redbook SG24-2010-00, section 7.1.3.
 *   - IBM Redbook SG24-2010-00, Figures 56 and 57.
 */

#include "rspinst.h"

#include <string.h>
#include <ctype.h>

/*!
 * @brief Case-insensitive comparison of two NUL-terminated strings.
 *
 * @param[in] pszA  First string.
 * @param[in] pszB  Second string.
 *
 * @return Ordering of the two strings.
 *
 * @retval 0   The two strings are equal (case-insensitive).
 * @retval <0  @p pszA sorts before @p pszB.
 * @retval >0  @p pszA sorts after @p pszB.
 */
static int rsp_ci_cmp(PCSZ pszA, PCSZ pszB)
{
    int cA;
    int cB;

    for (;;) {
        cA = tolower((unsigned char)*pszA);
        cB = tolower((unsigned char)*pszB);
        if (cA != cB) {
            return cA - cB;
        }
        if (cA == 0) {
            return 0;
        }
        pszA++;
        pszB++;
    }
}

/*!
 * @brief Static table of documented keywords.
 *
 * Every entry names the normative source of the keyword in
 * pszSource. Lookup by RspLookupKey is a case-insensitive linear
 * scan over this table.
 *
 * @par References
 *   - IBM Redbook SG24-2010-00, section 7.1.2.
 *   - IBM Redbook SG24-2010-00, section 7.1.3.
 *   - IBM Redbook SG24-2010-00, Figures 56 and 57.
 */
static const RSPKEYINFO RspKnownKeys[] = {

    /* -------- section 7.1.2, base keywords -------------------- */

    { "AdditionalPrinters",       RSP_KEYTYPE_LIST,       0,    0,
      "Printer list, entries of the form port:index",
      NULL,
      "SG24-2010-00 §7.1.2" },
    { "AlternateAdapter",         RSP_KEYTYPE_INTEGER,    0,    9,
      "Secondary display adapter",
      NULL,
      "SG24-2010-00 §7.1.2" },
    { "APM",                      RSP_KEYTYPE_INTEGER,    0,    2,
      "Advanced Power Management; 0=no, 1=detect, 2=install",
      NULL,
      "SG24-2010-00 §7.1.2" },
    { "BaseFileSystem",           RSP_KEYTYPE_DEPRECATED, 1,    2,
      "Obsolete; use FormatFAT or FormatHPFS",
      "FormatFAT / FormatHPFS",
      "SG24-2010-00 §7.1.2" },
    { "CDROM",                    RSP_KEYTYPE_INTEGER,    0,   67,
      "CD-ROM adapter type",
      NULL,
      "SG24-2010-00 §7.1.2" },
    { "ConfigSysLine",            RSP_KEYTYPE_STRING,     0,    0,
      "Line appended to CONFIG.SYS; may repeat",
      NULL,
      "SG24-2010-00 §7.1.2" },
    { "Copy",                     RSP_KEYTYPE_STRING,     0,    0,
      "Copy request, \"source destination\"",
      NULL,
      "SG24-2010-00 §7.1.2" },
    { "CountryCode",              RSP_KEYTYPE_STRING,     0,    0,
      "Numeric country code",
      NULL,
      "SG24-2010-00 §7.1.2" },
    { "CountryKeyboard",          RSP_KEYTYPE_STRING,     0,    0,
      "Keyboard layout code",
      NULL,
      "SG24-2010-00 §7.1.2" },
    { "DDIDDP",                   RSP_KEYTYPE_LIST,       0,    0,
      "List of .DDP driver profile files",
      NULL,
      "SG24-2010-00 §7.1.2" },
    { "DDIDest",                  RSP_KEYTYPE_PATH,       0,    0,
      "Destination directory for device driver install",
      NULL,
      "SG24-2010-00 §7.1.2" },
    { "DDISrc",                   RSP_KEYTYPE_PATH,       0,    0,
      "Source directory for device driver install",
      NULL,
      "SG24-2010-00 §7.1.2" },
    { "DefaultPrinter",           RSP_KEYTYPE_INTEGER,    0, 9999,
      "Index into PRDESC.LST",
      NULL,
      "SG24-2010-00 §7.1.2 and §7.1.4" },
    { "DisplayAdapter",           RSP_KEYTYPE_INTEGER,    0,    7,
      "Primary display adapter type",
      NULL,
      "SG24-2010-00 §7.1.2" },
    { "Documentation",            RSP_KEYTYPE_INTEGER,    0,    4,
      "Documentation components to install",
      NULL,
      "SG24-2010-00 §7.1.2" },
    { "DOSSupport",               RSP_KEYTYPE_BOOLEAN,    0,    1,
      "Install DOS support",
      NULL,
      "SG24-2010-00 §7.1.2" },
    { "DPMI",                     RSP_KEYTYPE_INTEGER,    0,    4,
      "DOS Protected Mode Interface options",
      NULL,
      "SG24-2010-00 §7.1.2" },
    { "EarlyUserExit",            RSP_KEYTYPE_STRING,     0,    0,
      "Program run early in the install; may repeat",
      NULL,
      "SG24-2010-00 §7.1.2" },
    { "ExistingWindowsPath",      RSP_KEYTYPE_PATH,       0,    0,
      "Existing Windows directory",
      NULL,
      "SG24-2010-00 §7.1.2" },
    { "ExitOnError",              RSP_KEYTYPE_BOOLEAN,    0,    1,
      "Exit with a return code on error; CID requires 1",
      NULL,
      "SG24-2010-00 §7.1.2 (note: CID requires 1)" },
    { "ExtendedInstall",          RSP_KEYTYPE_STRING,     0,    0,
      "Program run after install",
      NULL,
      "SG24-2010-00 §7.1.2" },
    { "Fonts",                    RSP_KEYTYPE_INTEGER,    0,    8,
      "Font set to install",
      NULL,
      "SG24-2010-00 §7.1.2" },
    { "FormatFAT",                RSP_KEYTYPE_LIST,       0,    0,
      "Drives to format as FAT",
      NULL,
      "SG24-2010-00 §7.1.2" },
    { "FormatHPFS",               RSP_KEYTYPE_LIST,       0,    0,
      "Drives to format as HPFS",
      NULL,
      "SG24-2010-00 §7.1.2" },
    { "FormatPartition",          RSP_KEYTYPE_DEPRECATED, 0,    1,
      "Obsolete; use FormatFAT or FormatHPFS",
      "FormatFAT / FormatHPFS",
      "SG24-2010-00 §7.1.2" },
    { "Include",                  RSP_KEYTYPE_INCLUDE,    0,    0,
      "Deferred include file",
      NULL,
      "SG24-2010-00 §7.1.5.3" },
    { "IncludeInLine",            RSP_KEYTYPE_INCLUDE,    0,    0,
      "Immediate include file",
      NULL,
      "SG24-2010-00 §7.1.5.2" },
    { "Infrared",                 RSP_KEYTYPE_BOOLEAN,    0,    1,
      "Install Infrared Port Support",
      NULL,
      "SG24-2010-00 §7.1.2" },
    { "MigrateApplications",      RSP_KEYTYPE_STRING,     0,    0,
      "Drives and database for application migration",
      NULL,
      "SG24-2010-00 §7.1.2" },
    { "MigrateConfigFiles",       RSP_KEYTYPE_BOOLEAN,    0,    1,
      "Migrate CONFIG.SYS and AUTOEXEC.BAT",
      NULL,
      "SG24-2010-00 §7.1.2" },
    { "Mouse",                    RSP_KEYTYPE_INTEGER,    0,    9,
      "Mouse driver type",
      NULL,
      "SG24-2010-00 §7.1.2" },
    { "MousePort",                RSP_KEYTYPE_INTEGER,    0,    4,
      "Serial mouse port",
      NULL,
      "SG24-2010-00 §7.1.2" },
    { "MultimediaSupport",        RSP_KEYTYPE_BOOLEAN,    0,    1,
      "Install multimedia support",
      NULL,
      "SG24-2010-00 §7.1.2" },
    { "Optical",                  RSP_KEYTYPE_BOOLEAN,    0,    1,
      "Install SCSI-II optical support",
      NULL,
      "SG24-2010-00 §7.1.2" },
    { "OptionalSystemComponents", RSP_KEYTYPE_LIST,       0,    3,
      "Optional system components",
      NULL,
      "SG24-2010-00 §7.1.2" },
    { "OptionalSystemUtilities",  RSP_KEYTYPE_LIST,       0,   15,
      "Optional system utilities",
      NULL,
      "SG24-2010-00 §7.1.2" },
    { "OS2IniData",               RSP_KEYTYPE_STRING,     0,    0,
      "Profile string for OS2.INI, form /App/Key/Value/",
      NULL,
      "SG24-2010-00 §7.1.2" },
    { "PCMCIA",                   RSP_KEYTYPE_INTEGER,    0,   64,
      "PCMCIA machine type",
      NULL,
      "SG24-2010-00 §7.1.2" },
    { "PCMCIAOptions",            RSP_KEYTYPE_LIST,       0,    4,
      "PCMCIA options",
      NULL,
      "SG24-2010-00 §7.1.2" },
    { "PrimaryCodePage",          RSP_KEYTYPE_INTEGER,    1,    2,
      "Primary code page: 1=national, 2=multilingual",
      NULL,
      "SG24-2010-00 §7.1.2" },
    { "PrinterPort",              RSP_KEYTYPE_INTEGER,    1,    7,
      "Port for the default printer",
      NULL,
      "SG24-2010-00 §7.1.2" },
    { "ProcessEnvironment",       RSP_KEYTYPE_BOOLEAN,    0,    1,
      "Publish keywords through the environment",
      NULL,
      "SG24-2010-00 §7.1.2" },
    { "ProgressIndication",       RSP_KEYTYPE_BOOLEAN,    0,    1,
      "Show a progress indicator",
      NULL,
      "SG24-2010-00 §7.1.2" },
    { "RebootRequired",           RSP_KEYTYPE_BOOLEAN,    0,    1,
      "Auto-reboot after install; CID requires 0",
      NULL,
      "SG24-2010-00 §7.1.2 (note: CID requires 0)" },
    { "SCSI",                     RSP_KEYTYPE_INTEGER,    0,   28,
      "SCSI adapter type",
      NULL,
      "SG24-2010-00 §7.1.2" },
    { "SeedConfigSysLine",        RSP_KEYTYPE_STRING,     0,    0,
      "Line appended to CONFIG.SYS of the seed system; may repeat",
      NULL,
      "SG24-2010-00 §7.1.2" },
    { "SerialDeviceSupport",      RSP_KEYTYPE_BOOLEAN,    0,    1,
      "Install serial device driver",
      NULL,
      "SG24-2010-00 §7.1.2" },
    { "ShareDesktopConfigFiles",  RSP_KEYTYPE_BOOLEAN,    0,    1,
      "Share desktop configuration with existing Windows",
      NULL,
      "SG24-2010-00 §7.1.2" },
    { "SourcePath",               RSP_KEYTYPE_PATH,       0,    0,
      "Source directory for install",
      NULL,
      "SG24-2010-00 §7.1.2" },
    { "TargetDrive",              RSP_KEYTYPE_DRIVE,      0,    0,
      "Target drive letter",
      NULL,
      "SG24-2010-00 §7.1.2" },
    { "ToolsAndGames",            RSP_KEYTYPE_LIST,       0,    8,
      "Tools and games to install",
      NULL,
      "SG24-2010-00 §7.1.2" },
    { "UserExit",                 RSP_KEYTYPE_STRING,     0,    0,
      "Program run at end of install; may repeat",
      NULL,
      "SG24-2010-00 §7.1.2" },
    { "WindowsInstallSourcePath", RSP_KEYTYPE_PATH,       0,    0,
      "Directory with Windows diskettes",
      NULL,
      "SG24-2010-00 §7.1.2" },
    { "WIN-OS/2Desktop",          RSP_KEYTYPE_INTEGER,    0,    2,
      "WIN-OS/2 desktop handling",
      NULL,
      "SG24-2010-00 §7.1.2" },
    { "WIN-OS/2Support",          RSP_KEYTYPE_LIST,       0,    6,
      "WIN-OS/2 support: 0=none, 1=all, 2..6=specific groups",
      NULL,
      "SG24-2010-00 §7.1.2" },
    { "WIN-OS/2TargetDrive",      RSP_KEYTYPE_DRIVE,      0,    0,
      "Target drive letter for WIN-OS/2",
      NULL,
      "SG24-2010-00 §7.1.2" },

    /* -------- section 7.1.3, CLIFI and dotted keywords -------- */

    { "ART.Selection",            RSP_KEYTYPE_BOOLEAN,    0,    1,
      "Install Application Registration Tool",
      NULL,
      "SG24-2010-00 §7.1.3" },
    { "BASKPSP.Selection",        RSP_KEYTYPE_BOOLEAN,    0,    1,
      "Install BonusPak AskPSP support",
      NULL,
      "SG24-2010-00 §7.1.3" },
    { "BASKPSP.TarDrv",           RSP_KEYTYPE_DRIVE,      0,    0,
      "Target drive for AskPSP",
      NULL,
      "SG24-2010-00 §7.1.3" },
    { "BPASKPSP.Selection",       RSP_KEYTYPE_BOOLEAN,    0,    1,
      "Install BonusPak AskPSP support (alternate spelling)",
      NULL,
      "SG24-2010-00 Fig. 57" },
    { "BPASKPSP.TarDrv",          RSP_KEYTYPE_DRIVE,      0,    0,
      "Target drive for AskPSP (alternate spelling)",
      NULL,
      "SG24-2010-00 Fig. 57" },
    { "BPCIM.Selection",          RSP_KEYTYPE_BOOLEAN,    0,    1,
      "Install CompuServe Information Manager",
      NULL,
      "SG24-2010-00 §7.1.3" },
    { "BPCIM.TarDrv",             RSP_KEYTYPE_DRIVE,      0,    0,
      "Target drive for CompuServe",
      NULL,
      "SG24-2010-00 §7.1.3" },
    { "BPFAXWORKS.Selection",     RSP_KEYTYPE_BOOLEAN,    0,    1,
      "Install FaxWorks",
      NULL,
      "SG24-2010-00 §7.1.3" },
    { "BPFAXWORKS.TarDrv",        RSP_KEYTYPE_DRIVE,      0,    0,
      "Target drive for FaxWorks",
      NULL,
      "SG24-2010-00 §7.1.3" },
    { "BPHAL.Selection",          RSP_KEYTYPE_BOOLEAN,    0,    1,
      "Install HyperAccess Lite",
      NULL,
      "SG24-2010-00 §7.1.3" },
    { "BPHAL.TarDrv",             RSP_KEYTYPE_DRIVE,      0,    0,
      "Target drive for HyperAccess Lite",
      NULL,
      "SG24-2010-00 §7.1.3" },
    { "BPHPJETCLIENT.Selection",  RSP_KEYTYPE_BOOLEAN,    0,    1,
      "Install JetAdmin client",
      NULL,
      "SG24-2010-00 §7.1.3" },
    { "BPHPJETCLIENT.TarDrv",     RSP_KEYTYPE_DRIVE,      0,    0,
      "Target drive for JetAdmin client",
      NULL,
      "SG24-2010-00 §7.1.3" },
    { "BPHPJETSERVER.Selection",  RSP_KEYTYPE_BOOLEAN,    0,    1,
      "Install JetAdmin server",
      NULL,
      "SG24-2010-00 §7.1.3" },
    { "BPHPJETSERVER.TarDrv",     RSP_KEYTYPE_DRIVE,      0,    0,
      "Target drive for JetAdmin server",
      NULL,
      "SG24-2010-00 §7.1.3" },
    { "BPIBMWORKS.Selection",     RSP_KEYTYPE_BOOLEAN,    0,    1,
      "Install IBM Works",
      NULL,
      "SG24-2010-00 §7.1.3" },
    { "BPIBMWORKS.TarDrv",        RSP_KEYTYPE_DRIVE,      0,    0,
      "Target drive for IBM Works",
      NULL,
      "SG24-2010-00 §7.1.3" },
    { "BPMARKNET.Selection",      RSP_KEYTYPE_BOOLEAN,    0,    1,
      "Install MarkNet Port Driver",
      NULL,
      "SG24-2010-00 §7.1.3" },
    { "BPMARKNET.TarDrv",         RSP_KEYTYPE_DRIVE,      0,    0,
      "Target drive for MarkNet Port Driver",
      NULL,
      "SG24-2010-00 §7.1.3" },
    { "BPMARKVIS.Selection",      RSP_KEYTYPE_BOOLEAN,    0,    1,
      "Install MarkVision",
      NULL,
      "SG24-2010-00 §7.1.3" },
    { "BPMARKVIS.TarDrv",         RSP_KEYTYPE_DRIVE,      0,    0,
      "Target drive for MarkVision",
      NULL,
      "SG24-2010-00 §7.1.3" },
    { "BPRS2.Selection",          RSP_KEYTYPE_BOOLEAN,    0,    1,
      "Install Remote Support",
      NULL,
      "SG24-2010-00 §7.1.3" },
    { "BPRS2.TarDrv",             RSP_KEYTYPE_DRIVE,      0,    0,
      "Target drive for Remote Support",
      NULL,
      "SG24-2010-00 §7.1.3" },
    { "BPVIDEOIN.Selection",      RSP_KEYTYPE_BOOLEAN,    0,    1,
      "Install VideoIn",
      NULL,
      "SG24-2010-00 §7.1.3" },
    { "BPVIDEOIN.TarDrv",         RSP_KEYTYPE_DRIVE,      0,    0,
      "Target drive for VideoIn",
      NULL,
      "SG24-2010-00 §7.1.3" },
    { "COACHES.Selection",        RSP_KEYTYPE_BOOLEAN,    0,    1,
      "Install Warp Guide",
      NULL,
      "SG24-2010-00 §7.1.3" },
    { "DAXCOMP1.Selection",       RSP_KEYTYPE_BOOLEAN,    0,    1,
      "Install Developer API Extensions",
      NULL,
      "SG24-2010-00 §7.1.3" },
    { "DAXCOMP1.TarDrv",          RSP_KEYTYPE_DRIVE,      0,    0,
      "Target drive for Developer API Extensions",
      NULL,
      "SG24-2010-00 §7.1.3" },
    { "HOTPLUG.Selection",        RSP_KEYTYPE_BOOLEAN,    0,    1,
      "Install external floppy support",
      NULL,
      "SG24-2010-00 §7.1.3" },
    { "JAVARUN.Selection",        RSP_KEYTYPE_BOOLEAN,    0,    1,
      "Install Java runtime",
      NULL,
      "SG24-2010-00 Fig. 57" },
    { "JAVASMPLS.Selection",      RSP_KEYTYPE_BOOLEAN,    0,    1,
      "Install Java samples",
      NULL,
      "SG24-2010-00 §7.1.3" },
    { "JAVATLKT.Selection",       RSP_KEYTYPE_BOOLEAN,    0,    1,
      "Install Java toolkit",
      NULL,
      "SG24-2010-00 §7.1.3" },
    { "ODBASE.Selection",         RSP_KEYTYPE_BOOLEAN,    0,    1,
      "Install OpenDoc",
      NULL,
      "SG24-2010-00 §7.1.3" },
    { "ODBASE.TarDrv",            RSP_KEYTYPE_DRIVE,      0,    0,
      "Target drive for OpenDoc",
      NULL,
      "SG24-2010-00 §7.1.3" },
    { "ODSECBASE.Selection",      RSP_KEYTYPE_BOOLEAN,    0,    1,
      "Install Security Extensions",
      NULL,
      "SG24-2010-00 §7.1.3" },
    { "ODSECBASE.TarDrv",         RSP_KEYTYPE_DRIVE,      0,    0,
      "Target drive for Security Extensions",
      NULL,
      "SG24-2010-00 §7.1.3" },
    { "SRVDIAG.Selection",        RSP_KEYTYPE_BOOLEAN,    0,    1,
      "Install serviceability and diagnostic aids",
      NULL,
      "SG24-2010-00 §7.1.3" },
    { "SRVDOC.Selection",         RSP_KEYTYPE_BOOLEAN,    0,    1,
      "Install serviceability documentation",
      NULL,
      "SG24-2010-00 §7.1.3" },
    { "SYSMGT.Selection",         RSP_KEYTYPE_BOOLEAN,    0,    1,
      "Install system management",
      NULL,
      "SG24-2010-00 §7.1.3" },
    { "VT.Selection",             RSP_KEYTYPE_BOOLEAN,    0,    1,
      "Install VoiceType",
      NULL,
      "SG24-2010-00 §7.1.3" },
    { "VT.TarDrv",                RSP_KEYTYPE_DRIVE,      0,    0,
      "Target drive for VoiceType",
      NULL,
      "SG24-2010-00 §7.1.3" },
    { "WARMDOCK.Selection",       RSP_KEYTYPE_BOOLEAN,    0,    1,
      "Install Dock II support",
      NULL,
      "SG24-2010-00 §7.1.3" },
    { "WARMSWAP.Selection",       RSP_KEYTYPE_BOOLEAN,    0,    1,
      "Install UltraBay swap support",
      NULL,
      "SG24-2010-00 §7.1.3" },
    { "WARMPLUG.Selection",       RSP_KEYTYPE_DEPRECATED, 0,    1,
      "Deprecated; use WARMSWAP.Selection",
      "WARMSWAP.Selection",
      "SG24-2010-00 §7.1.3 (note: not supported)" }
};

/*!
 * @brief Number of entries in RspKnownKeys.
 *
 * Computed at compile time from the array size.
 */
static const ULONG RspKnownKeysCount =
    sizeof(RspKnownKeys) / sizeof(RspKnownKeys[0]);

/*!
 * @brief Look up the static metadata of a documented keyword.
 *
 * Case-insensitive linear scan over the table. The pszSource field
 * of the returned structure names the normative source.
 *
 * @param[in] pszName  Keyword name. Not NULL.
 *
 * @return Pointer to the metadata, or NULL when the keyword is not
 *         documented.
 *
 * @retval NULL     @p pszName is NULL or the keyword is unknown.
 * @retval other    A pointer into the static keyword table.
 */
const RSPKEYINFO * APIENTRY RspLookupKey(PCSZ pszName)
{
    ULONG i;

    if (pszName == NULL) {
        return NULL;
    }
    for (i = 0; i < RspKnownKeysCount; i++) {
        if (rsp_ci_cmp(pszName, RspKnownKeys[i].pszName) == 0) {
            return &RspKnownKeys[i];
        }
    }
    return NULL;
}
