#ifndef _ixfmgr_h_
#define _ixfmgr_h_


// @todo replace visible structure and pointers with handles

typedef unsigned long fnIdentify(void * addr, unsigned long size);
typedef unsigned long fnLoad(void * addr, unsigned long size, void * ixfModule);
typedef unsigned long fnFixup(void * FormatStruct);

typedef
  struct
  {
    char * FunctionName;   // Name of function (NULL if none)
    void * Address;        // Address of function
    char * ModuleName;     // Name of module
    char * Ordinal;        // Ordinal of function
  } IXFMODULEENTRY;


typedef
  struct
  {
    void           * SrcAddress;      // Address to fix
    IXFMODULEENTRY   ImportEntry;     // Imported function
  } IXFFIXUPENTRY;

typedef
  struct
  {
    fnLoad * Load;            // Fill format structure
    fnFixup * Fixup;          // Fixup module address
    void * FormatStruct;      // Format specific structure (subject to remove)
    unsigned long cbEntries;  // Number of items in entries array
    IXFMODULEENTRY *Entries;  //[]; // Array of module entries
    unsigned long cbModules;  // Number of items in modules array
    char **Modules;           //[]; // Array of modules names
    unsigned long cbFixups;   // Number of items in fixups array
    IXFFIXUPENTRY *Fixups;    //[]; // Array of fixups entries
  } IXFModule;

unsigned long IXFIdentifyModule(void * addr, unsigned long size, IXFModule * ixfModule);
unsigned long IXFLoadModule(void * addr, unsigned long size, IXFModule * ixfModule);
unsigned long IXFFixupModule(IXFModule * ixfModule);


#endif
