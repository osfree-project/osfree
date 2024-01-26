#include <os2.h>

// Types of signature:
//
// Type 0:
// @#<Vendor>:<Revision>#@<Description>
//
// Type 1:
// @#<Vendor>:<Revision>#@##build <DateTime> -- on <BuildMachine>;0.1
// @@<Description>
//
// Type 2:
// @#<Vendor>:<Revision>#@##1## <DateTime> <BuildHost>:<ASDFeatureID>:
// <LanguageCode>:<CountryCode>:<Build>:<Unknown>:<FixPackVer>@@<Description>

typedef struct _BLDLEVELINFO {
  ULONG		ulType;             // Type of signature: 0, 1 or 2.
  ULONG		cbVendor;           // Length of Vendor string w/o '\0'.
  CHAR		acVendor[32];       // ASCIIZ Vendor.
  ULONG		cbRevision;
  CHAR		acRevision[32];
  ULONG		cbDateTime;
  CHAR		acDateTime[26];
  ULONG		cbBuildMachine;
  CHAR		acBuildMachine[12];
  ULONG		cbASDFeatureID;
  CHAR		acASDFeatureID[12];
  ULONG		cbLanguageCode;
  CHAR		acLanguageCode[4];
  ULONG		cbCountryCode;
  CHAR		acCountryCode[16];
  ULONG		cbBuild;
  CHAR		acBuild[16];
  ULONG		cbUnknown;
  CHAR		acUnknown[16];
  ULONG		cbFixPackVer;
  CHAR		acFixPackVer[16];
  ULONG		cbDescription;
  CHAR		acDescription[80];
  ULONG		cbPackage;
  CHAR		acPackage[80];
} BLDLEVELINFO, *PBLDLEVELINFO;


// BOOL blParse(ULONG cbScan, PCHAR pcScan, PBLDLEVELINFO pInfo)
//
// Fills structure PBLDLEVELINFO pointed to by pInfo from string pointed to by
// pcScan and length cbScan.
// Returns TRUE if the format is successfully recognized.

BOOL blParse(ULONG cbScan, PCHAR pcScan, PBLDLEVELINFO pInfo);

// ULONG blGetFromFile(PSZ pszFile, PBLDLEVELINFO pInfo)
//
// Read signature from file pointed by pszFile into pInfo.
// Returns ERROR_BAD_FORMAT if signature not found or other system error.

ULONG blGetFromFile(PSZ pszFile, PBLDLEVELINFO pInfo);
