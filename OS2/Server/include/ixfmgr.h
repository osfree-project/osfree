// @todo replace visible structure and pointers with handles

typedef unsigned long fnIdentify(void * addr, unsigned long size);
typedef unsigned long fnLoad(void * addr, unsigned long size, void ** FormatStruct);
typedef unsigned long fnFixup(void * FormatStruct);

typedef
  struct
  {
    fnLoad * Load;
    fnFixup * Fixup;
    void * FormatStruct;
  } IXFModule;

unsigned long IXFIdentifyModule(void * addr, unsigned long size, IXFModule * ixfModule);
unsigned long IXFLoadModule(void * addr, unsigned long size, IXFModule * ixfModule);
unsigned long IXFFixupModule(IXFModule * ixfModule);
