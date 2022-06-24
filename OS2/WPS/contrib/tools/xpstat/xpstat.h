
#pragma pack(1)

typedef struct _MQ
{
    // offset 0x00
            struct _MQ  *pNextMQ;
        // offset 0x04
            USHORT      usNumberQueued;         // always 0x0020
            USHORT      usQueueEntryLength;     // always 0
        // offset 0x08
            USHORT      _usUnknown1;            // always 0
            USHORT      usQueueDepth;           // always 0
        // offset 0x0c
            ULONG       ulTopOfQueue;           // always 0x0BB8
    // offset 0x10
            ULONG       ulBottomOfQueue;        // always 0x2FFF
        // offset 0x14
            ULONG       ulCurrentRead;
        // offset 0x18
            ULONG       ulCurrentWrite;
        // offset 0x1c
            ULONG       ulRealTID;             // _ulUnknown2; THIS is the TID
    // offset 0x20
            ULONG       _ulUnknown3;            // always 0
        // offset 0x24
            ULONG       _ulPID;                 // always 0
        // offset 0x28
            ULONG       ulRealSID;              // screen group (session) ID; not ulTID
        // offset 0x2c
            // ULONG       _ulSGID;         // wrong
            USHORT      usUnknown4;             // variable; 0x00E0 etc.
            USHORT      us0380;                 // always 0x0380
    // offset 0x30
            ULONG       hevMsg;     // message event semaphore handle
        // offset 0x34
            ULONG       _ulUnknown4[3];
    // offset 0x40
            ULONG       _ulUnknown5;
        // offset 0x44
            ULONG       ulCurrentSentSMS;       // if != 0, thread is blocked
                                                // in WinSendMsg waiting for response
        // offset 0x48
            ULONG       ulCurrentRcvdSMS;       // if != 0, thread is currently
                                                // processing message
        // offset 0x4c
            ULONG       _ulUnknown6;
    // offset 0x50
            BYTE        _abFill[0x4c];
    // offset 0x9c
            BYTE        _abSMS[0x08];
    // offset 0xa4
            USHORT      usSlotID;               // slot ID
} MQ, *PMQ;

#pragma pack()

typedef struct _PROCRECORD
{
    RECORDCORE  recc;
    ULONG       ulPID;
    CHAR        szPID[30];
    PSZ         pszPID;             // points to szPID
    CHAR        szModuleName[300];
    PSZ         pszModuleName;      // points to szModuleName
    CHAR        szTitle[500];
    PQPROCESS32 pProcess;
} PROCRECORD, *PPROCRECORD;

typedef struct _MODRECORD
{
    RECORDCORE  recc;
    CHAR        szModuleName[300];
    PSZ         pszModuleName;      // points to szModuleName
    PQMODULE32  pModule;
    BOOL        fSubModulesInserted;
} MODRECORD, *PMODRECORD;

#define ID_PROCLISTCNR  1000
#define ID_PROCINFO     1001
#define ID_PROCSPLIT    1002

#define ID_XPSM_MAIN        100

#define ID_XPSM_XPSTAT      200
#define ID_XPSMI_EXIT       201

#define ID_XPSM_VIEW        300
#define ID_XPSMI_PIDLIST    301
#define ID_XPSMI_SIDLIST    302
#define ID_XPSMI_PIDTREE    303
#define ID_XPSMI_MODTREE    304
#define ID_XPSMI_WORDWRAP   305
#define ID_XPSMI_REFRESH    306

