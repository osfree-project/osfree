// @todo replace visible structure and pointers with handles

typedef unsigned long fnIdentify(void * addr, unsigned long size);
typedef unsigned long fnLoad(void * addr, unsigned long size, void ** FormatStruct);
typedef unsigned long fnFixup(void * FormatStruct);

typedef
  struct
  {
    char * Name;           // Name of function (NULL if none)
    unsigned long Ordinal; // Ordinal of function (always >0)
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
  } IXFModule;

unsigned long IXFIdentifyModule(void * addr, unsigned long size, IXFModule * ixfModule);
unsigned long IXFLoadModule(void * addr, unsigned long size, IXFModule * ixfModule);
unsigned long IXFFixupModule(IXFModule * ixfModule);
