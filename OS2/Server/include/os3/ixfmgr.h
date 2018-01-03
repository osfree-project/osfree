#ifndef _ixfmgr_h_
#define _ixfmgr_h_

#ifdef __cplusplus
  extern "C" {
#endif

// @todo replace visible structure and pointers with handles

typedef unsigned long fnIdentify(void * addr, unsigned long size);
typedef unsigned long fnLoad(void * addr, unsigned long size, void * ixfModule);
typedef unsigned long fnFixup(void * FormatStruct);

typedef struct slist slist_t;

#ifdef L4API_l4v2

#include <l4/env/env.h>

#else

#include <os3/dataspace.h>

typedef struct
{
  void *addr;
  unsigned long size;
  l4_os3_dataspace_t ds;
} l4exec_section_t;

#endif

struct slist
{
  struct slist     *next;
  l4exec_section_t *section;
};

//slist_t *
//lastelem (slist_t *e);

typedef
  struct
  {
    int                 secnum;
    slist_t             *seclist;
    void                *stack_low;
    void                *stack_high;
    //l4_addr_t           pageaddr;
  } IXFSYSDEP;

typedef
  struct _IXFHandler
  {
    fnIdentify * Identify;
    fnLoad * Load;
    fnFixup * Fixup;
    struct _IXFHandler *next;
  } IXFHandler;

typedef
  struct
  {
    char * FunctionName;   // Name of function (NULL if none)
    void * Address;        // Address of function
    char * ModuleName;     // Name of module
    int    Ordinal;        // Ordinal of function
  } IXFMODULEENTRY;


typedef
  struct
  {
    void           * SrcAddress;      // Address to fix (in client task address space)
    void           * SrcVmAddress;    // Address to fix (in execsrv address space)
    IXFMODULEENTRY   ImportEntry;     // Imported function
  } IXFFIXUPENTRY;

typedef
  struct
  {
    fnLoad * Load;            // Fill format structure
    fnFixup * Fixup;          // Fixup module address
    unsigned long long area;  // memory area identifier (for example, shared memory area)
    void *addr;               // Load address
    unsigned long size;       // File size
    char *name;               // Fully-qualified filename
    char exec;                // exe (=1) or dll (=0)
    void * FormatStruct;      // Format specific structure (subject to remove)
    unsigned long cbEntries;  // Number of items in entries array
    IXFMODULEENTRY *Entries;  //[]; // Array of module entries
    unsigned long cbModules;  // Number of items in modules array
    char **Modules;           //[]; // Array of modules names
    unsigned long cbFixups;   // Number of items in fixups array
    IXFFIXUPENTRY *Fixups;    //[]; // Array of fixups entries
    void * EntryPoint;        // Entry point
    void * Stack;             // Stack
    unsigned long PIC;        // Module contains internal fixup table (PIC)
    unsigned long long hdlSysDep;  // Host dependent structure (handle or pointer)
  } IXFModule;

unsigned long IXFIdentifyModule(void * addr, unsigned long size, IXFModule * ixfModule);
unsigned long IXFLoadModule(void * addr, unsigned long size, IXFModule * ixfModule);
unsigned long IXFFixupModule(IXFModule * ixfModule);
unsigned long IXFAllocModule(IXFModule **ixf);
unsigned long IXFFreeModule(IXFModule *ixf);
unsigned long IXFCopyModule(IXFModule *dst, IXFModule *src);

#ifdef __cplusplus
  }
#endif

#endif
