#ifndef _ixfmgr_h_
#define _ixfmgr_h_


// @todo replace visible structure and pointers with handles

typedef unsigned long fnIdentify(void * addr, unsigned long size);
typedef unsigned long fnLoad(void * addr, unsigned long size, void ** FormatStruct);
typedef unsigned long fnFixup(void * FormatStruct);

typedef
  struct
  {
    char * FunctionName;   // Name of function (NULL if none)
    unsigned long Ordinal; // Ordinal of function (always >0)
    char * ModuleName;     // Module name
    void * Address;        // Address of function
  } IXFMODULEENTRY;


typedef
  struct
  {
    fnLoad * Load;            // Fill format structure
    fnFixup * Fixup;          // Fixup module address
    void * FormatStruct;      // Format specific structure (subject to remove)
    unsigned long cbEntries;  // Number of items in entries array
    IXFMODULEENTRY Entries[]; // Array of module entries
    unsigned long cbModules;  // Number of items in modules array
    char * Modules[];         // Array of modules names
  } IXFModule;

unsigned long IXFIdentifyModule(void * addr, unsigned long size, IXFModule * ixfModule);
unsigned long IXFLoadModule(void * addr, unsigned long size, IXFModule * ixfModule);
unsigned long IXFFixupModule(IXFModule * ixfModule);

#endif
