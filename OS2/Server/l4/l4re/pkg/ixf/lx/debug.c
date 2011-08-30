#include <l4/os3/io.h>
#include <l4/os3/modmgr.h>

void dump_header_mz(struct exe hdr)
{
  io_printf("\t\t\tDOS 2.0 EXE Header\n");
  io_printf("==============================================================================\n");
  io_printf("length of load module mod 200H                       =     %04XH\n", hdr.elast);
  io_printf("number of 200H pages in load module                  =     %04XH\n", hdr.epagsiz);
  io_printf("number of relocation items                           =     %04XH\n", hdr.erelcnt);
  io_printf("size of header in paragraphs                         =     %04XH\n", hdr.ehdrsiz);
  io_printf("minimum number of paragraphs required above load mod =     %04XH\n", hdr.eminfre);
  io_printf("maximum number of paragraphs required above load mod =     %04XH\n", hdr.emaxfre);
  io_printf("offset of stack segment in load module               =     %04XH\n", hdr.eiSS);
  io_printf("initial value of SP                                  =     %04XH\n", hdr.eiSP);
  io_printf("checksum                                             =     %04XH\n", hdr.enegsum);
  io_printf("initial value of IP                                  =     %04XH\n", hdr.eiIP);
  io_printf("offset of code segment within load module (segment)  =     %04XH\n", hdr.eiCS);
  io_printf("file offset of first relocation item                 =     %04XH\n", hdr.ereloff);
  io_printf("overlay number                                       =     %04XH\n", hdr.eovlnum);
  io_printf("load module length                                   = %08XH\n", hdr.elast);
  io_printf("\n");
}

void dump_header_lx(struct e32_exe hdr)
{
  io_printf("\t\tLinear EXE Header (OS/2 V2.x) - LX\n");
  io_printf("==============================================================================\n");
  io_printf("file offset = 00000080H\n");
  io_printf("\n");
  io_printf("byte order (0==little endian, 1==big endian)      =       %02XH\n", E32_BORDER(hdr));
  io_printf("word order       \"                \"               =       %02XH\n", E32_WORDER(hdr));
  io_printf("linear EXE format level                           = %08XH\n", E32_LEVEL(hdr));
  io_printf("cpu type                                          =     %04XH\n", E32_CPU(hdr));
  io_printf("os type (1==OS/2, 2==Windows, 3==DOS4, 4==Win386) =     %04XH\n", E32_OS(hdr));
  io_printf("module version                                    = %08XH\n", E32_VER(hdr));
  io_printf("module flags                                      = %08XH\n", E32_MFLAGS(hdr));
  io_printf("# module pages                                    = %08XH\n", E32_MPAGES(hdr));
  io_printf("object # for initial EIP                          = %08XH\n", E32_STARTOBJ(hdr));
  io_printf("initial EIP                                       = %08XH\n", E32_EIP(hdr));
  io_printf("object # for initial ESP                          = %08XH\n", E32_STACKOBJ(hdr));
  io_printf("initial ESP                                       = %08XH\n", E32_ESP(hdr));
  io_printf("page size                                         = %08XH\n", E32_PAGESIZE(hdr));
  io_printf("last page size (LE)/page shift (LX)               = %08XH\n", E32_PAGESHIFT(hdr));
  io_printf("fixup section size                                = %08XH\n", E32_FIXUPSIZE(hdr));
  io_printf("fixup section checksum                            = %08XH\n", E32_FIXUPSUM(hdr));
  io_printf("loader section size                               = %08XH\n", E32_LDRSIZE(hdr));
  io_printf("loader section checksum                           = %08XH\n", E32_LDRSUM(hdr));
  io_printf("object table offset                               = %08XH\n", E32_OBJTAB(hdr));
  io_printf("# of objects in module                            = %08XH\n", E32_OBJCNT(hdr));
  io_printf("object page map offset                            = %08XH\n", E32_OBJMAP(hdr));
  io_printf("object iterated data map offset                   = %08XH\n", E32_ITERMAP(hdr));
  io_printf("offset of resource table                          = %08XH\n", E32_RSRCTAB(hdr));
  io_printf("# of resource entries                             = %08XH\n", E32_RSRCCNT(hdr));
  io_printf("offset of resident name table                     = %08XH\n", E32_RESTAB(hdr));
  io_printf("offset of entry table                             = %08XH\n", E32_ENTTAB(hdr));
  io_printf("offset of module directive table                  = %08XH\n", E32_DIRTAB(hdr));
  io_printf("# of module directives                            = %08XH\n", E32_DIRCNT(hdr));
  io_printf("offset of fixup page table                        = %08XH\n", E32_FPAGETAB(hdr));
  io_printf("offset of fixup record table                      = %08XH\n", E32_FRECTAB(hdr));
  io_printf("offset of import module name table                = %08XH\n", E32_IMPMOD(hdr));
  io_printf("# of entries in import module name table          = %08XH\n", E32_IMPMODCNT(hdr));
  io_printf("offset of import procedure name table             = %08XH\n", E32_IMPPROC(hdr));
  io_printf("offset of per-page checksum table                 = %08XH\n", E32_PAGESUM(hdr));
  io_printf("offset of enumerated data pages                   = %08XH\n", E32_DATAPAGE(hdr));
  io_printf("# of pre-load pages                               = %08XH\n", E32_PRELOAD(hdr));
  io_printf("offset of non-resident names table (rel file)     = %08XH\n", E32_NRESTAB(hdr));
  io_printf("size of non-resident names table                  = %08XH\n", E32_CBNRESTAB(hdr));
  io_printf("non-resident names table checksum                 = %08XH\n", E32_NRESSUM(hdr));
  io_printf("object # for automatic data object                = %08XH\n", E32_AUTODATA(hdr));
  io_printf("offset of the debugging information               = %08XH\n", E32_DEBUGINFO(hdr));
  io_printf("size of the debugging information                 = %08XH\n", E32_DEBUGLEN(hdr));
  io_printf("# of instance pages in the preload section        = %08XH\n", E32_INSTPRELOAD(hdr));
  io_printf("# of instance pages in the demand load section    = %08XH\n", E32_INSTDEMAND(hdr));
  io_printf("size of heap (for 16-bit apps)                    = %08XH\n", E32_HEAPSIZE(hdr));
  io_printf("size of stack                                     = %08XH\n", E32_STACKSIZE(hdr));
  io_printf("Module Flags = ");
  if( (E32_MFLAGS(hdr) & E32MODMASK) == E32MODVDEV ) {
      io_printf( "VIRTDEVICE" );
  } else if( (E32_MFLAGS(hdr) & E32MODMASK) == E32MODPDEV ) {
      io_printf( "PHYSDEVICE" );
  } else if( (E32_MFLAGS(hdr) & E32MODMASK) == E32MODDLL ) {
      io_printf( "LIBRARY" );
  } else {
      io_printf( "PROGRAM" );
  }
  if( E32_MFLAGS(hdr) & 0x0001 ) {
      io_printf( " | SINGLEDATA" );
  }
  if( E32_MFLAGS(hdr) & E32LIBINIT ) {
      io_printf( " | INITINSTANCE" );
  }
  if( E32_MFLAGS(hdr) & E32LIBTERM ) {
      io_printf( " | TERMINSTANCE" );
  }
  if( E32_MFLAGS(hdr) & E32PROTDLL ) {
      io_printf( " | PROTDLL" );
  }
  if( E32_MFLAGS(hdr) & E32NOINTFIX ) {
      io_printf( " | NO_INT_FIXUPS" );
  }
  if( E32_MFLAGS(hdr) & E32NOEXTFIX ) {
      io_printf( " | NO_EXT_FIXUPS" );
  }
  if( E32_MFLAGS(hdr) & E32NOLOAD ) {
      io_printf( " | LINKERRORSDETECTED" );
  }
  switch( E32_MFLAGS(hdr) & E32APPMASK ) {
  case E32NOPMW:
      io_printf( " | NOTWINDOWCOMPAT" );
      break;
  case E32PMW:
      io_printf( " | WINDOWCOMPAT" );
      break;
  case E32PMAPI:
      io_printf( " | WINDOWAPI" );
      break;
  }
  if( E32_MFLAGS(hdr) & 0x80000 ) {
      io_printf( " | MPUNSAFE" );
  }
  io_printf( "\n" );
  io_printf( "\n" );
}

