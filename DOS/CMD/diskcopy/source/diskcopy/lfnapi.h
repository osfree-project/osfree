#ifndef LFNAPI_H_
#define LFNAPI_H_

#define MAXSFNS 3
#define INFILE_ID  0
#define OUTFILE_ID 1
#define ENVVAR_ID  2

#define MAYBE 2

struct LFNAttribute
  {
    int output;
  };

void InitLFNAPI (void);
void ConvertToSFN (char *lfn, int index);
char *GetSFN (int index);
void SetLFNAttribute (struct LFNAttribute *attrib, int index);
void SaveLFN (char *lfn, int index);
char *GetLFN (int index);
void SynchronizeLFNs (void);

/* In LFNCHK.ASM */
int CheckDriveOnLFN (char drive);

#endif
