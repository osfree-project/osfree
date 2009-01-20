#define CONFIGSYS_DONE 1
#define CONFIGSYS_DONE_BackUp 2
#define CONFIGSYS_DONE_Changed 4

#define CONFIGSYS_ERR_IsReadOnly 8
#define CONFIGSYS_ERR_FailedBackUp 16

#define CONFIGSYSACTION_Length 1024
#define CONFIGSYSACTSTR_Length 1024
#define CONFIGSYSACTSTR_MAXLENGTH 1024

#define CONFIGSYSACTION_Flags_Merge 1
#define CONFIGSYSACTION_Flags_MatchOnFilename 2

typedef struct _CONFIGSYSACTION {
   ULONG Flags;
   char * CommandStrPtr;
   char * MatchStrPtr;
   char * MatchInLineStrPtr;
   char * ValueStrPtr;
} CONFIGSYSACTION;

typedef CONFIGSYSACTION * PCONFIGSYSACTION;

typedef struct _CONFIGSYSACTSTR {
     char CommandStr[CONFIGSYSACTSTR_Length];
     char ValueStr[CONFIGSYSACTSTR_Length];
     char MatchStr[CONFIGSYSACTSTR_Length];
     char MatchInLineStr[CONFIGSYSACTSTR_Length];
} CONFIGSYSACTSTR;

typedef CONFIGSYSACTSTR * PCONFIGSYSACTSTR;

ULONG CONFIGSYS_DelayInit(ULONG BootDrive, PSZ Locklist, PSZ DelayedDir);
ULONG CONFIGSYS_Process (ULONG x, ULONG ChangeCount, PCONFIGSYSACTION ChangeArrayPtr, PSZ str);
