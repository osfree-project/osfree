/*!

  @file modmgr.h

  @brief OS/2 Server Module Manager

  This interface heavely based on OS/2 Module Manager interface
  and implements module format neutral access to module list.

*/

#ifndef _modmgr_H_
#define _modmgr_H_

#include <exe.h>
#include "gcc_os2def.h"
#include <exe386.h>

#define LOADING 1
#define DONE_LOADING 0

struct module_rec {
        char * mod_name;      /* Name of module. */
        void * module_struct; /* Pointer to module. */
        void *next;           /* Next element or NULL at end. */
        int load_status;      /* Status variable to check for recursion in loading state.
                                 A one means it is being loaded and zero it is done. */
};

void print_detailed_module_table(void);
void print_module_table(void);

struct module_rec * ModRegister(const char * name, void * mod_struct);

unsigned long ModInitialize(void);

unsigned long ModLoadModule(char *    pszName,
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



#endif

#endif
