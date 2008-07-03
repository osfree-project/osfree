#ifndef LFN_API_H_
#define LFN_API_H_

int LFNConvertToSFN(char* file);
int LFNFirstFile(char* wildcard, char* file, char* longfile);
int LFNNextFile(char* file, char* longfile);
void LFNFindStop(void);

/* In LFNCHK.ASM */
int CheckDriveOnLFN (char drive);

#endif
