#include <l4/os3/io.h>
#include <l4/os3/modmgr.h>

void dump_header_mz(struct exe hdr)
{
  io_log("\t\t\tDOS 2.0 EXE Header\n");
  io_log("==============================================================================\n");
  io_log("length of load module mod 200H                       =     %04XH\n", hdr.elast);
  io_log("number of 200H pages in load module                  =     %04XH\n", hdr.epagsiz);
  io_log("number of relocation items                           =     %04XH\n", hdr.erelcnt);
  io_log("size of header in paragraphs                         =     %04XH\n", hdr.ehdrsiz);
  io_log("minimum number of paragraphs required above load mod =     %04XH\n", hdr.eminfre);
  io_log("maximum number of paragraphs required above load mod =     %04XH\n", hdr.emaxfre);
  io_log("offset of stack segment in load module               =     %04XH\n", hdr.eiSS);
  io_log("initial value of SP                                  =     %04XH\n", hdr.eiSP);
  io_log("checksum                                             =     %04XH\n", hdr.enegsum);
  io_log("initial value of IP                                  =     %04XH\n", hdr.eiIP);
  io_log("offset of code segment within load module (segment)  =     %04XH\n", hdr.eiCS);
  io_log("file offset of first relocation item                 =     %04XH\n", hdr.ereloff);
  io_log("overlay number                                       =     %04XH\n", hdr.eovlnum);
  io_log("load module length                                   = %08XH\n", hdr.elast);
  io_log("\n");
}

void dump_header_lx(struct e32_exe hdr)
{
  io_log("\t\tLinear EXE Header (OS/2 V2.x) - LX\n");
  io_log("==============================================================================\n");
  io_log("file offset = 00000080H\n");
  io_log("\n");
  io_log("byte order (0==little endian, 1==big endian)      =       %02XH\n", E32_BORDER(hdr));
  io_log("word order       \"                \"               =       %02XH\n", E32_WORDER(hdr));
  io_log("linear EXE format level                           = %08XH\n", E32_LEVEL(hdr));
  io_log("cpu type                                          =     %04XH\n", E32_CPU(hdr));
  io_log("os type (1==OS/2, 2==Windows, 3==DOS4, 4==Win386) =     %04XH\n", E32_OS(hdr));
  io_log("module version                                    = %08XH\n", E32_VER(hdr));
  io_log("module flags                                      = %08XH\n", E32_MFLAGS(hdr));
  io_log("# module pages                                    = %08XH\n", E32_MPAGES(hdr));
  io_log("object # for initial EIP                          = %08XH\n", E32_STARTOBJ(hdr));
  io_log("initial EIP                                       = %08XH\n", E32_EIP(hdr));
  io_log("object # for initial ESP                          = %08XH\n", E32_STACKOBJ(hdr));
  io_log("initial ESP                                       = %08XH\n", E32_ESP(hdr));
  io_log("page size                                         = %08XH\n", E32_PAGESIZE(hdr));
  io_log("last page size (LE)/page shift (LX)               = %08XH\n", E32_PAGESHIFT(hdr));
  io_log("fixup section size                                = %08XH\n", E32_FIXUPSIZE(hdr));
  io_log("fixup section checksum                            = %08XH\n", E32_FIXUPSUM(hdr));
  io_log("loader section size                               = %08XH\n", E32_LDRSIZE(hdr));
  io_log("loader section checksum                           = %08XH\n", E32_LDRSUM(hdr));
  io_log("object table offset                               = %08XH\n", E32_OBJTAB(hdr));
  io_log("# of objects in module                            = %08XH\n", E32_OBJCNT(hdr));
  io_log("object page map offset                            = %08XH\n", E32_OBJMAP(hdr));
  io_log("object iterated data map offset                   = %08XH\n", E32_ITERMAP(hdr));
  io_log("offset of resource table                          = %08XH\n", E32_RSRCTAB(hdr));
  io_log("# of resource entries                             = %08XH\n", E32_RSRCCNT(hdr));
  io_log("offset of resident name table                     = %08XH\n", E32_RESTAB(hdr));
  io_log("offset of entry table                             = %08XH\n", E32_ENTTAB(hdr));
  io_log("offset of module directive table                  = %08XH\n", E32_DIRTAB(hdr));
  io_log("# of module directives                            = %08XH\n", E32_DIRCNT(hdr));
  io_log("offset of fixup page table                        = %08XH\n", E32_FPAGETAB(hdr));
  io_log("offset of fixup record table                      = %08XH\n", E32_FRECTAB(hdr));
  io_log("offset of import module name table                = %08XH\n", E32_IMPMOD(hdr));
  io_log("# of entries in import module name table          = %08XH\n", E32_IMPMODCNT(hdr));
  io_log("offset of import procedure name table             = %08XH\n", E32_IMPPROC(hdr));
  io_log("offset of per-page checksum table                 = %08XH\n", E32_PAGESUM(hdr));
  io_log("offset of enumerated data pages                   = %08XH\n", E32_DATAPAGE(hdr));
  io_log("# of pre-load pages                               = %08XH\n", E32_PRELOAD(hdr));
  io_log("offset of non-resident names table (rel file)     = %08XH\n", E32_NRESTAB(hdr));
  io_log("size of non-resident names table                  = %08XH\n", E32_CBNRESTAB(hdr));
  io_log("non-resident names table checksum                 = %08XH\n", E32_NRESSUM(hdr));
  io_log("object # for automatic data object                = %08XH\n", E32_AUTODATA(hdr));
  io_log("offset of the debugging information               = %08XH\n", E32_DEBUGINFO(hdr));
  io_log("size of the debugging information                 = %08XH\n", E32_DEBUGLEN(hdr));
  io_log("# of instance pages in the preload section        = %08XH\n", E32_INSTPRELOAD(hdr));
  io_log("# of instance pages in the demand load section    = %08XH\n", E32_INSTDEMAND(hdr));
  io_log("size of heap (for 16-bit apps)                    = %08XH\n", E32_HEAPSIZE(hdr));
  io_log("size of stack                                     = %08XH\n", E32_STACKSIZE(hdr));
  io_log("Module Flags = ");
  if( (E32_MFLAGS(hdr) & E32MODMASK) == E32MODVDEV ) {
      io_log( "VIRTDEVICE" );
  } else if( (E32_MFLAGS(hdr) & E32MODMASK) == E32MODPDEV ) {
      io_log( "PHYSDEVICE" );
  } else if( (E32_MFLAGS(hdr) & E32MODMASK) == E32MODDLL ) {
      io_log( "LIBRARY" );
  } else {
      io_log( "PROGRAM" );
  }
  if( E32_MFLAGS(hdr) & 0x0001 ) {
      io_log( " | SINGLEDATA" );
  }
  if( E32_MFLAGS(hdr) & E32LIBINIT ) {
      io_log( " | INITINSTANCE" );
  }
  if( E32_MFLAGS(hdr) & E32LIBTERM ) {
      io_log( " | TERMINSTANCE" );
  }
  if( E32_MFLAGS(hdr) & E32PROTDLL ) {
      io_log( " | PROTDLL" );
  }
  if( E32_MFLAGS(hdr) & E32NOINTFIX ) {
      io_log( " | NO_INT_FIXUPS" );
  }
  if( E32_MFLAGS(hdr) & E32NOEXTFIX ) {
      io_log( " | NO_EXT_FIXUPS" );
  }
  if( E32_MFLAGS(hdr) & E32NOLOAD ) {
      io_log( " | LINKERRORSDETECTED" );
  }
  switch( E32_MFLAGS(hdr) & E32APPMASK ) {
  case E32NOPMW:
      io_log( " | NOTWINDOWCOMPAT" );
      break;
  case E32PMW:
      io_log( " | WINDOWCOMPAT" );
      break;
  case E32PMAPI:
      io_log( " | WINDOWAPI" );
      break;
  }
  if( E32_MFLAGS(hdr) & 0x80000 ) {
      io_log( " | MPUNSAFE" );
  }
  io_log( "\n" );
  io_log( "\n" );
}

