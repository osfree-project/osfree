#ifndef EA_H
#define EA_H

BOOL cwSaveStringToEA(void *wpFSObject,  PSZ pKey, PSZ pValue);
BOOL cwGetStringFromEA(void *wpFSObject,  PSZ pKey, char *chrBuffer, ULONG ulBuffSize);

#endif
