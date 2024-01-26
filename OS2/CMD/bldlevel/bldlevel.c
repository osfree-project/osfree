#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#define INCL_DOSERRORS    /* DOS error values */
#include <os2.h>
#include "bldlevel.h"
//#include <debug.h>

#include <cmd_shared.h> /* comand line tools' shared functions and defines */

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


// ULONG _blGetDescription(ULONG cbStr, PCHAR pcStr, PBLDLEVELINFO pInfo)
//
// Read first part - vendor/revision: @#Vendor:Revision#@
// Returns number of bytes read, or 0 on error.

static ULONG _blGetVendorRevision(ULONG cbStr, PCHAR pcStr, PBLDLEVELINFO pInfo)
{
  PCHAR		pcScan;
  BOOL		fRevision = FALSE;

  if ( ( cbStr < 8 ) || ( *((PUSHORT)pcStr) != '#@' ) )
    return 0;

  pcScan = &pcStr[2];
  while( ( pcScan - pcStr ) < ( cbStr - 1 ) )
  {
    if ( *((PUSHORT)pcScan) == '@#' )
    {
      pInfo->acVendor[pInfo->cbVendor] = '\0';
      pInfo->acRevision[pInfo->cbRevision] =  '\0';
      break;
    }

    if ( ( *pcScan == '\r' ) || ( *pcScan == '\n' ) )
      return 0;
    if ( *pcScan == ':' )
      fRevision = TRUE;
    else if ( !fRevision )
    {
      if ( pInfo->cbVendor == sizeof(pInfo->acVendor) - 1 )
        return 0;
      pInfo->acVendor[pInfo->cbVendor++] = *pcScan;
    }
    else
    {
      if ( pInfo->cbRevision == sizeof(pInfo->acRevision) - 1 )
        return 0;
      if( ( ( *pcScan != '.' ) && ( (*pcScan - '0') > 9 ) ) ||
          ( *((PUSHORT)pcScan) == '..' ) ||
          ( ( pInfo->cbRevision == 0 ) && *pcScan == '.' ) )
        return 0;
      pInfo->acRevision[pInfo->cbRevision++] = *pcScan;
    }

    pcScan++;
  }

  if ( pInfo->cbVendor == 0 || pInfo->cbRevision == 0 ||
       ( pInfo->acRevision[pInfo->cbRevision - 1] == '.' ) )
    return 0;

  return ( pcScan - pcStr ) + 2;
}

// ULONG _blGetDescription(ULONG cbStr, PCHAR pcStr, PBLDLEVELINFO pInfo)
//
// Read last part - description.
// Returns number of bytes read, or 0 on error.

static ULONG _blGetDescription(ULONG cbStr, PCHAR pcStr, PBLDLEVELINFO pInfo)
{
  PCHAR		pcScan = pcStr;

  while( ( *pcScan == ' ' ) && ( ( pcScan - pcStr ) < cbStr ) )
    pcScan++;

  while( ( ( pcScan - pcStr ) < cbStr ) &&
         ( *pcScan != '\r' && *pcScan != '\n' && *pcScan != '\0' ) )
  {
    if ( pInfo->cbDescription == sizeof(pInfo->acDescription) - 1 )
      return 0;

    pInfo->acDescription[pInfo->cbDescription++] = *pcScan;
    pcScan++;
  }

  return pInfo->cbDescription;
}

// ULONG _blGetType1(ULONG cbStr, PCHAR pcStr, PBLDLEVELINFO pInfo)
//
// Read signature type 1 parts after vendor/revision and "##build":
//   <DateTime> -- on <BuildMachine>;0.1@@
// Returns number of bytes read, or 0 on error.

static ULONG _blGetType1(ULONG cbStr, PCHAR pcStr, PBLDLEVELINFO pInfo)
{
  BOOL		fHost = FALSE;
  ULONG		ulLen = cbStr;
  PCHAR		pcScan = pcStr;

  while( ulLen > 3 )
  {
    if ( *pcScan == '\r' || *pcScan == '\n' || *pcScan == '\0' ||
         ulLen <= 6 )
      return 0;

    if ( !fHost )
    {
      if ( memcmp( pcScan, "-- on ", 6 ) == 0 )
      {
        fHost = TRUE;
        pInfo->acDateTime[pInfo->cbDateTime++] = '\0';
        pcScan += 6;
        ulLen -= 6;
        continue;
      }

      if ( pInfo->cbDateTime == sizeof(pInfo->acDateTime) - 1 )
        return 0;

      pInfo->acDateTime[pInfo->cbDateTime++] = *pcScan;
    }
    else
    {
      if ( memcmp( pcScan, ";0.1@@", 6 ) == 0 )
        break;

      pInfo->acBuildMachine[pInfo->cbBuildMachine++] = *pcScan;
    }

    pcScan++;
    ulLen--;
  }

  if ( ( pInfo->cbBuildMachine == 0 ) || ( pInfo->cbDateTime == 0 ) )
    return 0;

  pInfo->acBuildMachine[pInfo->cbBuildMachine] = '\0';
  return ( pcScan - pcStr ) + 6;
}

// ULONG _isDigitDateTime(PCHAR pcStr)
//
// Trying to determine that the pcStr is date/time:
//   '01.02.2015 12:02:03 '
//   '2015.02.01 12:02:03 '
//   '01-02-2015 12:02:03 '
//   '2015-02-01 12:02:03 '
// Returns number of bytes read, or 0 if date/time was not detected.

static ULONG _isDigitDateTime(PCHAR pcStr)
{
  PCHAR		pcScan = pcStr;
  PCHAR		pcEnd;
  ULONG		ulVal;
  ULONG		ulLen;

  ulVal = strtoul( pcScan, &pcEnd, 10 );
  ulLen = pcEnd - pcScan;
  if ( ( ulLen != 1 && ulLen != 2 && ulLen != 4 ) ||
       ( strchr( ".-/", *pcEnd ) == NULL ) ||
       ( ulVal > 3000 ) )
    return 0;
  pcScan = pcEnd + 1;
  ulVal = strtoul( ++pcScan, &pcScan, 10 );
  if ( ulVal > 32 || ( strchr( ".-/", *pcScan ) == NULL ) )
    return 0;
  ulVal = strtoul( ++pcScan, &pcEnd, 10 );
  ulLen = pcEnd - pcScan;
  if ( ( ulLen != 2 && ulLen != 4 ) || *pcEnd != ' ' || ulVal > 3000 )
    return 0;
  pcScan = pcEnd + 1;
  ulVal = strtoul( pcScan, &pcScan, 10 );
  if ( ulVal > 24 || *pcScan != ':' )
    return 0;
  ulVal = strtoul( ++pcScan, &pcScan, 10 );
  if ( ulVal > 60 || *pcScan != ':' )
    return 0;
  ulVal = strtoul( ++pcScan, &pcScan, 10 );
  if ( ulVal > 60 || *pcScan != ' ' )
    return 0;

  return ( pcScan - pcStr ) + 1;
}

// ULONG _blGetType2(ULONG cbStr, PCHAR pcStr, PBLDLEVELINFO pInfo)
//
// Read signature type 2 parts after vendor/revision and "1##":
//   <DateTime> <BuildMachine>:<ASDFeatureID>:<LanguageCode>:<CountryCode>:
//   <Build>:<Unknown>:<FixPackVer>@@
// Returns number of bytes read, or 0 on error.

static ULONG _blGetType2(ULONG cbStr, PCHAR pcStr, PBLDLEVELINFO pInfo)
{
  ULONG		ulLen = cbStr;
  PCHAR		pcScan = pcStr;
  CHAR		acBuf[7][128];
  ULONG		cbBuf = 0;
  ULONG		ulPart = 0;
  PCHAR		pcSpace;

  memset( &acBuf, '\0', sizeof(acBuf) );

  while( ( *pcScan == ' ' ) && ( ulLen > 0 ) )
  {
    pcScan++;
    ulLen--;
  }

  cbBuf = _isDigitDateTime( pcScan );
  if ( ( cbBuf == 0 ) && ( ulLen > 24 ) && ( pcScan[24] == ' ') )
    cbBuf = 24;

  memcpy( &acBuf[0][0], pcScan, cbBuf );
  pcScan += cbBuf;
  ulLen -= cbBuf;

  while( ulLen > 3 )
  {
    if ( cbBuf == 128 )
      return 0;

    if ( *pcScan == ':' )
    {
      if ( ulPart == 6 )
        return 0;

      while( ( cbBuf > 0 ) && ( acBuf[ulPart][cbBuf - 1] == ' ' ) )
        cbBuf--;
      acBuf[ulPart][cbBuf] = '\0';

      ulPart++;
      cbBuf = 0;
      pcScan++;
      ulLen--;
      continue;
    }

    if ( *((PUSHORT)pcScan) == '@@' )
      break;

    if ( ( cbBuf != 0 ) || ( *pcScan != ' ' ) )
      acBuf[ulPart][cbBuf++] = *pcScan;

    pcScan++;
    ulLen--;
  }

  pcSpace = strrchr( &acBuf[0][0], ' ' );
  if ( pcSpace == 0 )
    // No date/time - only BuildMachine in first section.
    pInfo->cbBuildMachine = strlcpy( &pInfo->acBuildMachine, &acBuf[0][0], sizeof(pInfo->acBuildMachine) );
  else
  {
    // Cut first section on date/time and BuildMachine.
    pInfo->cbBuildMachine = strlcpy( &pInfo->acBuildMachine,
                                     pcSpace + 1,
                                     sizeof(pInfo->acBuildMachine) );
    while( ( pcSpace > &acBuf[0][0] ) && ( *(pcSpace - 1) == ' ' ) )
      pcSpace--;
    *pcSpace = '\0';
    pInfo->cbDateTime = strlcpy( &pInfo->acDateTime, &acBuf[0][0],
                                 sizeof(pInfo->acDateTime) );
  }

  pInfo->cbASDFeatureID = strlcpy( &pInfo->acASDFeatureID, &acBuf[1][0], sizeof(pInfo->acASDFeatureID) );
  pInfo->cbLanguageCode = strlcpy( &pInfo->acLanguageCode, &acBuf[2][0], sizeof(pInfo->acLanguageCode) );
  pInfo->cbCountryCode = strlcpy( &pInfo->acCountryCode, &acBuf[3][0], sizeof(pInfo->acCountryCode) );
  pInfo->cbBuild = strlcpy( &pInfo->acBuild, &acBuf[4][0], sizeof(pInfo->cbBuild) );
  pInfo->cbUnknown = strlcpy( &pInfo->acUnknown, &acBuf[5][0], sizeof(pInfo->acUnknown) );
  pInfo->cbFixPackVer = strlcpy( &pInfo->acFixPackVer, &acBuf[6][0], sizeof(pInfo->acFixPackVer) );
  return ( pcScan - pcStr ) + 2;
}



// BOOL blParse(ULONG cbScan, PCHAR pcScan, PBLDLEVELINFO pInfo)
//
// Fills structure PBLDLEVELINFO pointed to by pInfo from string pointed to by
// pcScan and length cbScan.
// Returns TRUE if the format is successfully recognized.

BOOL blParse(ULONG cbScan, PCHAR pcScan, PBLDLEVELINFO pInfo)
{
  ULONG		cBytes;

  memset( pInfo, '\0', sizeof(BLDLEVELINFO) );

  // Try to read vendor and revision parts: @#Vendor:1.2.3#@
  cBytes = _blGetVendorRevision( cbScan, pcScan, pInfo );
  if ( cBytes == 0 )
    return FALSE;

  pcScan += cBytes;
  cbScan -= cBytes;

  if ( ( cbScan > 0 ) && ( *((PUSHORT)pcScan) == '##' ) )
  {
    // '##' after vendor/revision - type 1 or 2 of signature

    pcScan += 2;
    cbScan -= 2;

    // 17 is "build"..."-- on "...";0.1@@"
    if ( ( cbScan > 17 ) && ( memcmp( pcScan, "build", 5 ) == 0 ) )
    {
      // Signature type 1 : @#Vendor:1.2.3#@##build ...

      pcScan += 5;
      cbScan -= 5;
      cBytes = _blGetType1( cbScan, pcScan, pInfo );
      if ( cBytes == 0 )
        return FALSE;
      pcScan += cBytes;
      cbScan -= cBytes;
      pInfo->ulType = 1;
    }
    // 24 is "1## DD.MM.YYYY hh:mm:ss"..."@@"
    else if ( ( cbScan > 24 ) && ( memcmp( pcScan, "1##", 3 ) == 0 ) )
    {
      // Signature type 1 : @#Vendor:1.2.3#@##1## ...

      pcScan += 3;
      cbScan -= 3;
      cBytes = _blGetType2( cbScan, pcScan, pInfo );
      if ( cBytes == 0 )
        return FALSE;
      pcScan += cBytes;
      cbScan -= cBytes;
      pInfo->ulType = 2;
    }
  }

  // Read description part : @@description text
  return _blGetDescription( cbScan, pcScan, pInfo ) != 0;
}


// ULONG blGetFromFile(PSZ pszFile, PBLDLEVELINFO pInfo)
//
// Read signature from file pointed by pszFile into pInfo.
// Returns ERROR_BAD_FORMAT if signature not found or other system error.

ULONG blGetFromFile(PSZ pszFile, PBLDLEVELINFO pInfo)
{
  CHAR       acBuf[8 * 1024];
  ULONG      ulRC;
  HFILE      hFile;
  ULONG      ulAction;
  ULONG      cbBuf;
  ULONG      ulTailPos;

  ulRC = DosOpen( pszFile, &hFile, &ulAction, 0, 0, 
                  OPEN_ACTION_OPEN_IF_EXISTS,
                  OPEN_FLAGS_FAIL_ON_ERROR | OPEN_SHARE_DENYNONE |
                  OPEN_ACCESS_READONLY, NULL );
  if ( ulRC != NO_ERROR )
  {
    //debug( "DosOpen(), rc = %lu", ulRC );
    return ulRC;
  }


  // Search at the tail of the file

  ulRC = DosSetFilePtr( hFile, -sizeof(acBuf), FILE_END, &ulTailPos );
  if ( ulRC != NO_ERROR )
  {
    if ( ulRC != ERROR_NEGATIVE_SEEK )
      //debug( "#1 DosSetFilePtr(), rc = %lu", ulRC );
    ulTailPos = 0;
  }

  ulRC = DosRead( hFile, acBuf, sizeof(acBuf), &cbBuf );
  if ( ulRC != NO_ERROR )
  {//debug( "#1 DosRead(), rc = %lu", ulRC );
  }
  else
  {
    ulRC = ERROR_BAD_FORMAT;
    if ( cbBuf > 10 )
    {
      ULONG  cbScan = 10;
      PCHAR  pcScan = &acBuf[cbBuf - cbScan];

      while( pcScan >= acBuf )
      {
        if ( ( *pcScan == '@' ) && blParse( cbScan, pcScan, pInfo ) )
        {
          //debug( "Found at the tail of the file. Position: %lu",
            //     ulTailPos + ( pcScan - acBuf ) );
          ulRC = NO_ERROR;
          break;
        }

        pcScan--;
        cbScan++;
      }
    }
  }


  // Search at the beginning of the file

  if ( ( ulRC == ERROR_BAD_FORMAT ) && ( ulTailPos > 1 ) )
  {
    /* We have already looked at the file from the ulTailPos position to the
       end. Now we will search from the beginning up to sizeof(acBuf) or
       ulTailPos (which of this will be closer).  */
    ulRC = DosSetFilePtr( hFile, 0, FILE_BEGIN, &ulAction );
    if ( ulRC != NO_ERROR )
	{//debug( "#2 DosSetFilePtr(), rc = %lu", ulRC );
	}
    else
    {
      ulRC = DosRead( hFile, acBuf, min( sizeof(acBuf), ulTailPos + 10 ), &cbBuf );
      if ( ulRC != NO_ERROR )
	  {//debug( "#2 DosRead(), rc = %lu", ulRC );
	  }
      else
      {
        PCHAR  pcScan = acBuf;

        while( cbBuf > 10 )
        {
          if ( ( *pcScan == '@' ) && blParse( cbBuf, pcScan, pInfo ) )
          {
            //debug( "Found. Position: %lu", ( pcScan - acBuf ) );
            ulRC = NO_ERROR;
            break;
          }

          pcScan++;
          cbBuf--;
        }

      }  // if ( ulRC != NO_ERROR ) else
    }  // if ( ulRC != NO_ERROR ) else
  }  // if ( ( ulRC == ERROR_BAD_FORMAT ) && ( ulTailPos > 10 ) )


  DosClose( hFile );

  return ulRC;
}

int main (int argc, char* argv[], char* envp[])
{
	BLDLEVELINFO Info = {0};
	int rc=0;

	printf("Build Level Display Facility Version 1.2\n");
	printf("(C) Copyright 2003-2004, 2024 by Yuri Prokushev\n");
	printf("(C) Copyright 2017-2020 by Andrey Vasilkin\n");

	if (argc==1) 
	{
		cmd_ShowSystemMessage(8067,0L); /* Help message */
		return rc;
	}
	
	rc=blGetFromFile(argv[1], &Info);

	//cmd_ShowSystemMessage(8068,0L);
	//printf();
	//Say SysGetMessage(8068)||Signature

	if (Info.cbVendor)
	{
		cmd_ShowSystemMessage(8069,0L);
		printf("%s\n", Info.acVendor);
	}
//Say SysGetMessage(8069)||Vendor

	if (Info.cbRevision)
	{
		cmd_ShowSystemMessage(8070,0L);
		printf("%s\n", Info.acRevision);
	}
//Say SysGetMessage(8070)||Revision

	if (Info.cbDateTime)
	{
		cmd_ShowSystemMessage(8071,0L);
		printf("%s\n", Info.acDateTime);
	}
//If BuildDate<>'' then Say SysGetMessage(8071)||Space(BuildDate)

	if (Info.cbBuildMachine)
	{
		cmd_ShowSystemMessage(8072,0L);
		printf("%s\n", Info.acBuildMachine);
	}
//If HostName<>'' then Say SysGetMessage(8072)||Space(HostName)

	if (Info.cbRevision)
	{
		cmd_ShowSystemMessage(8073,0L);
		if (Info.cbBuild)
		{
			printf("%s.%s\n", Info.acBuild, Info.acRevision);
		}
		else 
		{
			printf("%s\n", Info.acRevision);
		}
	}
	
//If Package<>'' then Say SysGetMessage(8074)||Space(Package)

	if (Info.cbDescription)
	{
		cmd_ShowSystemMessage(8075,0L);
		printf("%s\n", Info.acDescription);
	}
//Say SysGetMessage(8075)||Space(Description)

	if (Info.cbASDFeatureID)
	{
		cmd_ShowSystemMessage(8076,0L);
		printf("%s\n", Info.acASDFeatureID);
	}
//If ASDFeatureID<>'' then Say SysGetMessage(8076)||ASDFeatureID

	if (Info.cbLanguageCode)
	{
		cmd_ShowSystemMessage(8077,0L);
		printf("%s\n", Info.acLanguageCode);
	}
//If LanguageCode<>'' then Say SysGetMessage(8077)||LanguageCode

	if (Info.cbCountryCode)
	{
		cmd_ShowSystemMessage(8078,0L);
		printf("%s\n", Info.acCountryCode);
	}
//If CountryCode<>'' then Say SysGetMessage(8078)||CountryCode

	if (Info.cbFixPackVer)
	{
		cmd_ShowSystemMessage(8079,0L);
		printf("%s\n", Info.acFixPackVer);
	}
//If FixPackVer<>'' then Say SysGetMessage(8079)||FixPackVer
  return rc;
}
