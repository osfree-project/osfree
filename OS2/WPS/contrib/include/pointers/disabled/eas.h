
#ifndef EAS_H
#define EAS_H

#define EANAME_TIMEFRAME   ".TIMEFRAME"
#define EANAME_INFONAME    ".INFONAME"
#define EANAME_INFOARTIST  ".INFOARTIST"

typedef struct _EAMVMT
   {
         USHORT         usType;
         USHORT         usCodepage;
         USHORT         usEntries;
         USHORT         usEntryType;
         USHORT         usEntryLen;
         CHAR           chEntry[1];
   } EAMVMT, *PEAMVMT;

typedef struct _EASVST
   {
         USHORT         usType;
         USHORT         usEntryLen;
         CHAR           chEntry[1];
   } EASVST, *PEASVST;

// prototypes
APIRET WriteStringEa( HFILE hfile, PSZ pszEaName, PSZ pszEaValue);
APIRET ReadStringEa( PSZ pszFileName, PSZ pszEaName, PSZ pszBuffer, PULONG ulBuflen);

#endif // EAS_H

