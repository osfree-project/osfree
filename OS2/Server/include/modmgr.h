/*!

  @file modmgr.h

  @brief OS/2 Server Module Manager

  This interface heavely based on OS/2 Module Manager interface
  and implements module format neutral access to module list.

*/

/*! @todo subject to move to process manager */
int modmgr_execute_module(char * filename);

unsigned long ModLoadModule(const char *    pszName,
                            unsigned long   cbName,
                            const char *    pszModname,
                            unsigned long * phmod);

unsigned long ModFreeModule(unsigned long hmod);

unsigned long ModQueryProcAddr(unsigned long hmod,
                               unsigned long ordinal,
                               const char *  pszName,
                               void **       ppfn);

unsigned long ModQueryModuleHandle(const char *    pszModname,
                                   unsigned long * phmod);

unsigned long ModQueryModuleName(unsigned long hmod,
                                 unsigned long cbName,
                                 char * pch);

#define PT_16BIT        0
#define PT_32BIT        1

unsigned long ModQueryProcType(unsigned long   hmod,
                               unsigned long   ordinal,
                               const char *    pszName,
                               unsigned long * pulproctype);

unsigned long ModReplaceModule(const char * pszOldModule,
                               const char * pszNewModule,
                               const char * pszBackupModule);

/*! @todo detect is this really required? */

#if 0
APIRET APIENTRY  DosQueryModFromEIP(HMODULE *phMod,
                                        ULONG *pObjNum,
                                        ULONG BuffLen,
                                        PCHAR pBuff,
                                        ULONG *pOffset,
                                        ULONG Address);

   #define HW_CFG_MCA              0x00000001
   #define HW_CFG_EISA             0x00000002
   #define HW_CFG_ABIOS_SUPPORTED  0x00000004
   #define HW_CFG_ABIOS_PRESENT    0x00000008
   #define HW_CFG_PCI              0x00000010
   #define HW_CFG_IBM_ABIOS        0x00000000  /* OEM flag is OFF, ABIOS is IBM */
   #define HW_CFG_OEM_ABIOS        0x00000020
   #define HW_CFG_PENTIUM_CPU      0x00000040

APIRET  APIENTRY DosQueryABIOSSupport(ULONG reserved);


   /* structure returned by DosQueryModFromCS */

   typedef struct _QMRESULT { /* qmres */
      USHORT seg;
      USHORT hmte;
      CHAR   name[CCHMAXPATH];
   } QMRESULT;
   typedef QMRESULT * PQMRESULT;


APIRET16 APIENTRY16 Dos16QueryModFromCS(SEL, PQMRESULT);

#endif
