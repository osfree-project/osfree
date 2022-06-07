
/*
 *@@sourcefile exeh.h:
 *      header file for exeh.c. See remarks there.
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 *
 *@@include #define INCL_WINPROGRAMLIST         // for some funcs only
 *@@include #include <os2.h>
 *@@include #include "helpers\dosh.h"
 *@@include #include "helpers\exeh.h"
 */

/*      This file Copyright (C) 2000-2002 Ulrich M”ller,
 *                                        Martin Lafaix.
 *      This file is part of the "XWorkplace helpers" source package.
 *      This is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published
 *      by the Free Software Foundation, in version 2 as it comes in the
 *      "COPYING" file of the XWorkplace main distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 */

#if __cplusplus
extern "C" {
#endif

#ifndef EXEH_HEADER_INCLUDED
    #define EXEH_HEADER_INCLUDED

    #pragma pack(1)

    /********************************************************************
     *
     *   DOS exe header
     *
     ********************************************************************/

    /*
     *@@ DOSEXEHEADER:
     *      old DOS EXE header at offset 0
     *      in any EXE file.
     *
     *@@changed V0.9.7 (2000-12-20) [umoeller]: fixed NE offset
     *@@changed V0.9.9 (2001-04-06) [lafaix]: additional fields defined
     */

    typedef struct _DOSEXEHEADER
    {
         USHORT usDosExeID;             // 00: DOS exeid (0x5a4d)
         USHORT usFileLenMod512;        // 02: filelen mod 512
         USHORT usFileLenDiv512;        // 04: filelen div 512
         USHORT usSegFix;               // 06: count of segment adds to fix
         USHORT usHdrPargCnt;           // 08: size of header in paragrphs
         USHORT usMinAddPargCnt;        // 0a: minimum addtl paragraphs count
         USHORT usMaxAddPargCnt;        // 0c: max addtl paragraphs count
         USHORT usSSStartup;            // 0e: SS at startup
         USHORT usSPStartup;            // 10: SP at startup
         USHORT usHdrChecksum;          // 12: header checksum
         USHORT usIPStartup;            // 14: IP at startup
         USHORT usCodeSegOfs;           // 16: code segment offset from EXE start
         USHORT usRelocTableOfs;        // 18: reloc table ofs.header (Win: >= 0x40 || 0)
         USHORT usOverlayNo;            // 1a: overlay no.
         USHORT usLinkerVersion;        // 1c: linker version (if 0x18 > 0x28)
         USHORT usUnused1;              // 1e: unused
         USHORT usBehaviorBits;         // 20: exe.h says this field contains
                                        //     'behavior bits'
         USHORT usUnused2;              // 22: unused
         USHORT usOEMIdentifier;        // 24: OEM identifier
         USHORT usOEMInformation;       // 26: OEM information
         ULONG  ulUnused3;              // 28:
         ULONG  ulUnused4;              // 2c:
         ULONG  ulUnused5;              // 30:
         ULONG  ulUnused6;              // 34:
         ULONG  ulUnused7;              // 38:
         ULONG  ulNewHeaderOfs;         // 3c: new header ofs (if 0x18 >= 0x40 || 0)
                    // fixed this from USHORT, thanks Martin Lafaix
                    // V0.9.7 (2000-12-20) [umoeller]
    } DOSEXEHEADER, *PDOSEXEHEADER;

    /********************************************************************
     *
     *   New Executable (NE)
     *
     ********************************************************************/

    // NE and LX OS types
    #define NEOS_UNKNOWN        0
    #define NEOS_OS2            1   // Win16 SDK says: "reserved"...
    #define NEOS_WIN16          2
    #define NEOS_DOS4           3   // Win16 SDK says: "reserved"...
    #define NEOS_WIN386         4   // Win16 SDK says: "reserved"...

    /*
     *@@ NEHEADER:
     *      linear executable (LX) header format,
     *      which comes after the DOS header in the
     *      EXE file (at DOSEXEHEADER.ulNewHeaderOfs).
     *
     *@@changed V0.9.9 (2001-04-06) [lafaix]: fixed typo in usMoveableEntries
     */

    typedef struct _NEHEADER
    {
        CHAR      achNE[2];             // 00: "NE" magic                       ne_magic
        BYTE      bLinkerVersion;       // 02: linker version                   ne_ver
        BYTE      bLinkerRevision;      // 03: linker revision                  ne_rev
        USHORT    usEntryTblOfs;        // 04: ofs from this to entrytable      ne_enttab
        USHORT    usEntryTblLen;        // 06: length of entrytable             ne_cbenttab
        ULONG     ulChecksum;           // 08: MS: reserved, OS/2: checksum     ne_crc
        USHORT    usFlags;              // 0c: flags                            ne_flags
                #ifndef NENOTP // do not conflict with toolkit exe defs
                    #define NENOTP          0x8000          // Not a process == library
                    #define NENOTMPSAFE     0x4000          // Process is not multi-processor safe
                                                            // (Win3.1 SDK: "reserved")
                    #define NEIERR          0x2000          // Errors in image
                    #define NEBOUND         0x0800          // Bound Family/API
                                                            // (Win3.1 SDK: "first segment contains code
                                                            // that loads the application")
                    #define NEAPPTYP        0x0700          // Application type mask
                                                            // (Win3.1 SDK: "reserved")
                    #define NENOTWINCOMPAT  0x0100          // Not compatible with P.M. Windowing
                                                            // (Win3.1 SDK: "reserved")
                    #define NEWINCOMPAT     0x0200          // Compatible with P.M. Windowing
                                                            // (Win3.1 SDK: "reserved")
                    #define NEWINAPI        0x0300          // Uses P.M. Windowing API
                                                            // (Win3.1 SDK: "reserved")
                    #define NEFLTP          0x0080          // Floating-point instructions
                    #define NEI386          0x0040          // 386 instructions
                    #define NEI286          0x0020          // 286 instructions
                    #define NEI086          0x0010          // 8086 instructions
                    #define NEPROT          0x0008          // Runs in protected mode only
                                                            // (Win3.1 SDK: "reserved")
                    #define NEPPLI          0x0004          // Per-Process Library Initialization
                                                            // (Win3.1 SDK: "reserved")
                    #define NEINST          0x0002          // Instance data
                    #define NESOLO          0x0001          // Solo data (single data)
                #endif
        USHORT    usAutoDataSegNo;      // 0e: auto-data seg no.                ne_autodata
                                        // (Win3.1 SDK: "0 if both NEINST and NESOLO are cleared")
        USHORT    usInitlHeapSize;      // 10: initl. heap size                 ne_heap
                                        // (Win3.1 SDK: "0 if no local allocation")
        USHORT    usInitlStackSize;     // 12: initl. stack size                ne_stack
                                        // (Win3.1 SDK: "0 if SS != DS")
        ULONG     ulCSIP;               // 14: CS:IP                            ne_csip
        ULONG     ulSSSP;               // 18: SS:SP                            ne_sssp
        USHORT    usSegTblEntries;      // 1c: segment tbl entry count          ne_cseg
        USHORT    usModuleTblEntries;   // 1e: module ref. table entry count    ne_cmod
        USHORT    usNonResdTblLen;      // 20: non-resd. name tbl length        ne_cbnrestab
        USHORT    usSegTblOfs;          // 22: segment tbl ofs                  ne_segtab
                                        // (from start of NEHEADER)
        USHORT    usResTblOfs;          // 24: resource tbl ofs                 ne_rsrctab
                                        // (from start of NEHEADER)
        USHORT    usResdNameTblOfs;     // 26: resd. name tbl ofs               ne_restab
                                        // (from start of NEHEADER)
        USHORT    usModRefTblOfs;       // 28: module ref. table ofs            ne_modtab
                                        // (from start of NEHEADER)
        USHORT    usImportTblOfs;       // 2a: import name tbl ofs              ne_imptab
                                        // (from start of NEHEADER)
        ULONG     ulNonResdTblOfs;      // 2c: non-resd. name tbl ofs           ne_nrestab
                                        // (from start of EXE!)
        USHORT    usMoveableEntries;    // 30: moveable entry points count      ne_cmovent
        USHORT    usLogicalSectShift;   // 32: logcl. sector shift              ne_align
                                        // (Win3.1 SDK: "typically 4, but default is 9")
        USHORT    usResSegmCount;       // 34: resource segm. count             ne_cres
        BYTE      bTargetOS;            // 36: target OS (NEOS_* flags)         ne_exetyp
        BYTE      bFlags2;              // 37: addtl. flags                     ne_flagsothers
                                        // Win3.1 SDK:
                                        //      bit 1 --> Win2.x, but runs in Win3.x protected mode
                                        //      bit 2 --> Win2.x that supports prop. fonts
                                        //      bit 3 --> exec contains fastload area
                                /*
                                #define NELONGNAMES     0x01
                                #define NEWINISPROT     0x02
                                #define NEWINGETPROPFON 0x04
                                #define NEWLOAPPL       0x80
                                */
        // the following are not listed in newexe.h, but are documented for Win3.x
        USHORT    usFastLoadOfs;        // 38: fast-load area ofs
        USHORT    usFastLoadLen;        // 3a: fast-load area length
        USHORT    usReserved;           // 3c: MS: 'reserved'
        USHORT    usReqWinVersion;      // 3e: Win-only: min. Win version
    } NEHEADER, *PNEHEADER;

    /*
     *@@ OS2NERESTBLENTRY:
     *      OS/2 NE resource table entry.
     *
     *@@added V0.9.16 (2001-12-08) [umoeller]
     */

    typedef struct _OS2NERESTBLENTRY
    {
        USHORT      usType;
        USHORT      usID;
    } OS2NERESTBLENTRY, *POS2NERESTBLENTRY;

    /*
     *@@ OS2NESEGMENT:
     *      OS/2 NE segment definition.
     *
     *@@added V0.9.16 (2001-12-08) [umoeller]
     */

    typedef struct _OS2NESEGMENT       // New .EXE segment table entry
    {
        USHORT      ns_sector;      // File sector of start of segment
        USHORT      ns_cbseg;       // Number of bytes in file
        USHORT      ns_flags;       // Attribute flags
        USHORT      ns_minalloc;    // Minimum allocation in bytes
    } OS2NESEGMENT, *POS2NESEGMENT;

    /********************************************************************
     *
     *   Linear Executable (LX)
     *
     ********************************************************************/

    /*
     *@@ LXHEADER:
     *      linear executable (LX) header format,
     *      which comes after the DOS header in the
     *      EXE file (at DOSEXEHEADER.ulNewHeaderOfs).
     *
     *@@changed V0.9.9 (2001-04-06) [lafaix]: fixed auto data object and ulinstanceDemdCnt
     */

    typedef struct _LXHEADER
    {
        CHAR      achLX[2];             // 00: e32_magic  "LX" or "LE" magic
            // this is "LX" for 32-bit OS/2 programs, but
            // "LE" for MS-DOS progs which use this format
            // (e.g. WINDOWS\SMARTDRV.EXE). IBM says the
            // LE format was never released and superceded
            // by LX... I am unsure what the differences are.
        BYTE      fByteBigEndian;       // 02: e32_border byte ordering (1 = big endian)
        BYTE      fWordBigEndian;       // 03: e32_worder word ordering (1 = big endian)
        ULONG     ulFormatLevel;        // 04: e32_level exe format level (0)
        USHORT    usCPU;                // 08: e32_cpu CPU type
        USHORT    usTargetOS;           // 0a: e32_os OS type (NEOS_* flags)
        ULONG     ulModuleVersion;      // 0c: e32_ver module version
        ULONG     ulFlags;              // 10: e32_mflags module flags
                #ifndef E32NOTP // do not conflict with toolkit exe defs
                    #define E32NOTP          0x00008000L    // Library Module - used as NENOTP
                    #define E32NOLOAD        0x00002000L    // Module not Loadable
                    #define E32PMAPI         0x00000300L    // Uses PM Windowing API
                    #define E32PMW           0x00000200L    // Compatible with PM Windowing
                    #define E32NOPMW         0x00000100L    // Incompatible with PM Windowing
                    #define E32NOEXTFIX      0x00000020L    // NO External Fixups in .EXE
                    #define E32NOINTFIX      0x00000010L    // NO Internal Fixups in .EXE
                    #define E32SYSDLL        0x00000008L    // System DLL, Internal Fixups discarded
                    #define E32LIBINIT       0x00000004L    // Per-Process Library Initialization
                    #define E32LIBTERM       0x40000000L    // Per-Process Library Termination

                    #define E32APPMASK       0x00000300L    // Application Type Mask

                    // hiword defs added V1.0.1 (2003-01-17) [umoeller]
                    #define E32PROTDLL       0x00010000L    // Protected memory library module *
                    #define E32DEVICE        0x00020000L    // Device driver                   *
                    #define E32MODEXE        0x00000000L    // .EXE module                     *
                    #define E32MODDLL        0x00008000L    // .DLL module                     *
                    #define E32MODPROTDLL    0x00018000L    // Protected memory library module *
                    #define E32MODPDEV       0x00020000L    // Physical device driver          *
                    #define E32MODVDEV       0x00028000L    // Virtual device driver           *
                    #define E32MODMASK       0x00038000L    // Module type mask                *
                    #define E32NOTMPSAFE     0x00080000L    // Process is multi-processor unsafe *
                #endif
        ULONG     ulPageCount;          // 14: e32_mpages no. of pages in module
        ULONG     ulEIPRelObj;          // 18: e32_startobj obj # for IP
                                        // object to which EIP is relative
        ULONG     ulEIPEntryAddr;       // 1c: e32_eip EIP entry addr
        ULONG     ulESPObj;             // 20: e32_stackobj ESP object
        ULONG     ulESP;                // 24: e32_esp ESP
        ULONG     ulPageSize;           // 28: e32_pagesize page size (4K)
        ULONG     ulPageLeftShift;      // 2c: e32_pageshift page left-shift
        ULONG     ulFixupTblLen;        // 30: e32_fixupsize fixup section total size
        ULONG     ulFixupTblChecksum;   // 34: e32_fixupsum fixup section checksum
        ULONG     ulLoaderLen;          // 38: e32_ldrsize size req. for loader section
        ULONG     ulLoaderChecksum;     // 3c: e32_ldrsum loader section checksum
        ULONG     ulObjTblOfs;          // 40: e32_objtab object table offset
        ULONG     ulObjCount;           // 44: e32_objcnt object count
        ULONG     ulObjPageTblOfs;      // 48: e32_objmap object page table ofs
        ULONG     ulObjIterPagesOfs;    // 4c: e32_itermap object iter pages ofs
        ULONG     ulResTblOfs;          // 50: e32_rsrctab resource table ofs
        ULONG     ulResTblCnt;          // 54: e32_rsrccnt resource entry count
        ULONG     ulResdNameTblOfs;     // 58: e32_restab resident name tbl ofs
        ULONG     ulEntryTblOfs;        // 5c: e32_enttab entry tbl ofs
        ULONG     ulModDirectivesOfs;   // 60: e32_dirtab module directives ofs
        ULONG     ulModDirectivesCnt;   // 64: e32_dircnt module directives count
        ULONG     ulFixupPagePageTblOfs;// 68: e32_fpagetab fixup page tbl ofs
        ULONG     ulFixupRecTblOfs;     // 6c: e32_frectab fixup record tbl ofs
        ULONG     ulImportModTblOfs;    // 70: e32_impmod import modl tbl ofs
        ULONG     ulImportModTblCnt;    // 74: e32_impmodcnt import modl tbl count
        ULONG     ulImportProcTblOfs;   // 78: e32_impproc import proc tbl ofs
        ULONG     ulPerPageCSOfs;       // 7c: e32_pagesum per page checksum ofs
        ULONG     ulDataPagesOfs;       // 80: e32_datapage data pages ofs
        ULONG     ulPreloadPagesCnt;    // 84: e32_preload preload pages count
        ULONG     ulNonResdNameTblOfs;  // 88: e32_nrestab non-resdnt name tbl ofs
        ULONG     ulNonResdNameTblLen;  // 8c: e32_cbnrestab non-resdnt name tbl length
        ULONG     ulNonResdNameTblCS;   // 90: e32_nressum non-res name tbl checksum
        ULONG     ulAutoDataSegObj;     // 94: e32_autodata auto dataseg object
        ULONG     ulDebugOfs;           // 98: e32_debuginfo debug info ofs
        ULONG     ulDebugLen;           // 9c: e32_debuglen debug info length
        ULONG     ulInstancePrelCnt;    // a0: e32_instpreload instance preload count
        ULONG     ulInstanceDemdCnt;    // a0: e32_instdemand instance demand count
        ULONG     ulHeapSize16;         // a8: e32_heapsize heap size (16-bit)
        ULONG     ulStackSize;          // ac: e32_stacksize stack size
        BYTE      e32_res3[20];
                                        // Pad structure to 196 bytes
    } LXHEADER, *PLXHEADER;

    // additional LX structures

    /*
     *@@ RESOURCETABLEENTRY:
     *     LX resource table entry.
     *
     *@@added V0.9.16 (2001-12-08) [umoeller]
     */

    typedef struct _RESOURCETABLEENTRY     // rsrc32
    {
        USHORT  type;   // Resource type
        USHORT  name;   // Resource name
        ULONG   cb;     // Resource size
        USHORT  obj;    // Object number
        ULONG   offset; // Offset within object
    } RESOURCETABLEENTRY;

    /*
     *@@ OBJECTTABLEENTRY:
     *     LX object table entry.
     *
     *@@added V0.9.16 (2001-12-08) [umoeller]
     */

    typedef struct _OBJECTTABLEENTRY       // o32_obj
    {
        ULONG   o32_size;     // Object virtual size
        ULONG   o32_base;     // Object base virtual address
        ULONG   o32_flags;    // Attribute flags
        ULONG   o32_pagemap;  // Object page map index
        ULONG   o32_mapsize;  // Number of entries in object page map
        ULONG   o32_reserved; // Reserved
    } OBJECTTABLEENTRY;

    /*
     *@@ OBJECTPAGETABLEENTRY:
     *     LX object _page_ table entry, sometimes
     *     referred to as map entry.
     *
     *@@added V0.9.16 (2001-12-08) [umoeller]
     */

    typedef struct _OBJECTPAGETABLEENTRY   // o32_map
    {
        ULONG   o32_pagedataoffset;     // file offset of page
        USHORT  o32_pagesize;           // # of real bytes of page data
        USHORT  o32_pageflags;          // Per-Page attributes
    } OBJECTPAGETABLEENTRY;

    /*
     *@@ LXITER:
     *      iteration Record format for 'EXEPACK'ed pages.
     *
     *@@added V0.9.16 (2001-12-08) [umoeller]
     */

    typedef struct _LXITER
    {
        USHORT          LX_nIter;            // number of iterations
        USHORT          LX_nBytes;           // number of bytes
        unsigned char   LX_Iterdata;         // iterated data byte(s)
    } LXITER, *PLXITER;

    // object/segment flags (in NE and LX)
    #define OBJWRITE         0x0002L    // Writeable Object
    #define OBJDISCARD       0x0010L    // Object is Discardable
    #define OBJSHARED        0x0020L    // Object is Shared
    #define OBJPRELOAD       0x0040L    // Object has preload pages

    // resource flags
    #define RNMOVE           0x0010     // Moveable resource
    #define RNPURE           0x0020     // Pure (read-only) resource
    #define RNPRELOAD        0x0040     // Preloaded resource
    #define RNDISCARD        0xF000     // Discard priority level for resource

    /********************************************************************
     *
     *   Portable Executable (PE)
     *
     ********************************************************************/

    /*
     *@@ IMAGE_FILE_HEADER:
     *      first part of PE header (standard header).
     *
     *@@added V0.9.16 (2002-01-09) [umoeller]
     */

    typedef struct _IMAGE_FILE_HEADER
    {
        USHORT      usMachine;          // CPU type
                        #define IMAGE_FILE_MACHINE_UNKNOWN          0
                        #define IMAGE_FILE_MACHINE_I860         0x14d
                        #define IMAGE_FILE_MACHINE_I386         0x14c
                        #define IMAGE_FILE_MACHINE_R3000        0x162
                        #define IMAGE_FILE_MACHINE_R4000        0x166
                        #define IMAGE_FILE_MACHINE_R10000       0x168
                        #define IMAGE_FILE_MACHINE_ALPHA        0x184
                        #define IMAGE_FILE_MACHINE_POWERPC      0x1F0
        USHORT      usNumberOfSections;
        ULONG       ulTimeDateStamp;
        ULONG       ulPointerToSymbolTable;
        ULONG       ulNumberOfSymbols;
        USHORT      usSizeOfOptionalHeader;
        USHORT      fsCharacteristics;
                        #define IMAGE_FILE_RELOCS_STRIPPED          1 // No relocation info
                        #define IMAGE_FILE_EXECUTABLE_IMAGE         2
                        #define IMAGE_FILE_LINE_NUMS_STRIPPED       4
                        #define IMAGE_FILE_LOCAL_SYMS_STRIPPED      8
                        #define IMAGE_FILE_16BIT_MACHINE         0x40
                        #define IMAGE_FILE_BYTES_REVERSED_LO     0x80
                        #define IMAGE_FILE_32BIT_MACHINE        0x100
                        #define IMAGE_FILE_DEBUG_STRIPPED       0x200
                        #define IMAGE_FILE_SYSTEM              0x1000
                        #define IMAGE_FILE_DLL                 0x2000
                        #define IMAGE_FILE_BYTES_REVERSED_HI   0x8000
    } IMAGE_FILE_HEADER, *PIMAGE_FILE_HEADER;

    typedef struct _IMAGE_DATA_DIRECTORY
    {
        ULONG   VirtualAddress;
        ULONG   Size;
    } IMAGE_DATA_DIRECTORY,*PIMAGE_DATA_DIRECTORY;

    #define IMAGE_NUMBEROF_DIRECTORY_ENTRIES    16

    /*
     *@@ IMAGE_OPTIONAL_HEADER:
     *      second part of PE header (optional header).
     *
     *@@added V0.9.16 (2002-01-09) [umoeller]
     */

    typedef struct _IMAGE_OPTIONAL_HEADER
    {
        // Standard fields
        USHORT  Magic;              // 00
                    #define IMAGE_NT_OPTIONAL_HDR_MAGIC        0x10b
                    #define IMAGE_ROM_OPTIONAL_HDR_MAGIC       0x107
        BYTE    MajorLinkerVersion;     // 02
        BYTE    MinorLinkerVersion;     // 03
        ULONG   SizeOfCode;         // 04
        ULONG   SizeOfInitializedData;      // 08
        ULONG   SizeOfUninitializedData;    // 0C
        ULONG   AddressOfEntryPoint;        // 10
        ULONG   BaseOfCode;         // 14
        ULONG   BaseOfData;         // 18

        // NT additional fields
        ULONG   ImageBase;          // 1C
        ULONG   SectionAlignment;       // 20
        ULONG   FileAlignment;          // 24
        USHORT  MajorOperatingSystemVersion;    // 28
        USHORT  MinorOperatingSystemVersion;    // 2A
        USHORT  MajorImageVersion;      // 2C
        USHORT  MinorImageVersion;      // 2E
        USHORT  MajorSubsystemVersion;      // 30
        USHORT  MinorSubsystemVersion;      // 32
        ULONG   Reserved1;          // 34
        ULONG   SizeOfImage;            // 38
        ULONG   SizeOfHeaders;          // 3C
        ULONG   CheckSum;           // 40
        USHORT  usSubsystem;          // 44
                    #define IMAGE_SUBSYSTEM_UNKNOWN                 0
                    #define IMAGE_SUBSYSTEM_NATIVE                  1
                    #define IMAGE_SUBSYSTEM_WINDOWS_GUI             2   // Windows GUI subsystem
                    #define IMAGE_SUBSYSTEM_WINDOWS_CUI             3   // Windows character subsystem
                    #define IMAGE_SUBSYSTEM_OS2_CUI                 5
                    #define IMAGE_SUBSYSTEM_POSIX_CUI               7
        USHORT  DllCharacteristics;     // 46
        ULONG   SizeOfStackReserve;     // 48
        ULONG   SizeOfStackCommit;      // 4C
        ULONG   SizeOfHeapReserve;      // 50
        ULONG   SizeOfHeapCommit;       // 54
        ULONG   LoaderFlags;            // 58
        ULONG   NumberOfRvaAndSizes;        // 5C
                            // 60:
        IMAGE_DATA_DIRECTORY DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
    } IMAGE_OPTIONAL_HEADER, *PIMAGE_OPTIONAL_HEADER;

    // indices into the DataDirectory array
    #define IMAGE_FILE_EXPORT_DIRECTORY             0
    #define IMAGE_FILE_IMPORT_DIRECTORY             1
    #define IMAGE_FILE_RESOURCE_DIRECTORY           2
    #define IMAGE_FILE_EXCEPTION_DIRECTORY          3
    #define IMAGE_FILE_SECURITY_DIRECTORY           4
    #define IMAGE_FILE_BASE_RELOCATION_TABLE        5
    #define IMAGE_FILE_DEBUG_DIRECTORY              6
    #define IMAGE_FILE_DESCRIPTION_STRING           7
    #define IMAGE_FILE_MACHINE_VALUE                8  // Mips
    #define IMAGE_FILE_THREAD_LOCAL_STORAGE         9
    #define IMAGE_FILE_CALLBACK_DIRECTORY          10

    // directory entries, indices into the DataDirectory array
    #define IMAGE_DIRECTORY_ENTRY_EXPORT            0
    #define IMAGE_DIRECTORY_ENTRY_IMPORT            1
    #define IMAGE_DIRECTORY_ENTRY_RESOURCE          2
    #define IMAGE_DIRECTORY_ENTRY_EXCEPTION         3
    #define IMAGE_DIRECTORY_ENTRY_SECURITY          4
    #define IMAGE_DIRECTORY_ENTRY_BASERELOC         5
    #define IMAGE_DIRECTORY_ENTRY_DEBUG             6
    #define IMAGE_DIRECTORY_ENTRY_COPYRIGHT         7
    #define IMAGE_DIRECTORY_ENTRY_GLOBALPTR         8   // (MIPS GP)
    #define IMAGE_DIRECTORY_ENTRY_TLS               9
    #define IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG      10
    #define IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT     11
    #define IMAGE_DIRECTORY_ENTRY_IAT              12  // Import Address Table
    #define IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT     13
    #define IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR   14

    /*
     *@@ PEHEADER:
     *      portable executable (PE) header format,
     *      which comes after the DOS header in the
     *      EXE file (at DOSEXEHEADER.ulNewHeaderOfs).
     *
     *@@added V0.9.16 (2002-01-09) [umoeller]
     */

    typedef struct _PEHEADER
    {
        // standard header
        ULONG       ulSignature;        // 00: 'P', 'E', 0, 0
        IMAGE_FILE_HEADER               // 04
                    FileHeader;
        IMAGE_OPTIONAL_HEADER           // 18
                    OptionalHeader;

    } PEHEADER, *PPEHEADER;

    /*
    #define PE_HEADER(module) \
        ((IMAGE_NT_HEADERS*)((LPBYTE)(module) + \
                             (((IMAGE_DOS_HEADER*)(module))->e_lfanew)))

    #define PE_SECTIONS(module) \
        ((IMAGE_SECTION_HEADER*)((LPBYTE)&PE_HEADER(module)->OptionalHeader + \
                               PE_HEADER(module)->FileHeader.SizeOfOptionalHeader))

    #define RVA_PTR(module,field) ((LPBYTE)(module) + PE_HEADER(module)->field)
    */

    // Section header format

    #define IMAGE_SIZEOF_SHORT_NAME 8

    /*
     *@@ IMAGE_SECTION_HEADER:
     *      comes right after PEHEADER in executable.
     *
     *@@added V0.9.16 (2002-01-09) [umoeller]
     */

    typedef struct _IMAGE_SECTION_HEADER
    {
        BYTE    Name[IMAGE_SIZEOF_SHORT_NAME];  // 00:
        union
        {
            ULONG   PhysicalAddress;    // 04:
            ULONG   VirtualSize;        // 04:
        } Misc;
        ULONG       VirtualAddress;         // 08:
        ULONG       ulSizeOfRawData;          // 0C:
        ULONG       ulPointerToRawData;       // 10:
        ULONG       PointerToRelocations;       // 14:
        ULONG       PointerToLinenumbers;       // 18:
        USHORT      NumberOfRelocations;        // 1C:
        USHORT      NumberOfLinenumbers;        // 1E:
        ULONG       flCharacteristics;        // 20:
                // #define IMAGE_SCN_TYPE_REG           0x00000000 - Reserved
                // #define IMAGE_SCN_TYPE_DSECT         0x00000001 - Reserved
                // #define IMAGE_SCN_TYPE_NOLOAD        0x00000002 - Reserved
                // #define IMAGE_SCN_TYPE_GROUP         0x00000004 - Reserved
                // #define IMAGE_SCN_TYPE_NO_PAD        0x00000008 - Reserved
                // #define IMAGE_SCN_TYPE_COPY          0x00000010 - Reserved
                #define IMAGE_SCN_CNT_CODE                  0x00000020
                #define IMAGE_SCN_CNT_INITIALIZED_DATA      0x00000040
                #define IMAGE_SCN_CNT_UNINITIALIZED_DATA    0x00000080
                #define IMAGE_SCN_LNK_OTHER                 0x00000100
                #define IMAGE_SCN_LNK_INFO                  0x00000200
                // #define  IMAGE_SCN_TYPE_OVER             0x00000400 - Reserved
                #define IMAGE_SCN_LNK_REMOVE                0x00000800
                #define IMAGE_SCN_LNK_COMDAT                0x00001000
                //                      0x00002000 - Reserved
                // #define IMAGE_SCN_MEM_PROTECTED          0x00004000 - Obsolete
                #define IMAGE_SCN_MEM_FARDATA               0x00008000
                // #define IMAGE_SCN_MEM_SYSHEAP            0x00010000 - Obsolete
                #define IMAGE_SCN_MEM_PURGEABLE             0x00020000
                #define IMAGE_SCN_MEM_16BIT                 0x00020000
                #define IMAGE_SCN_MEM_LOCKED                0x00040000
                #define IMAGE_SCN_MEM_PRELOAD               0x00080000
                #define IMAGE_SCN_ALIGN_1BYTES              0x00100000
                #define IMAGE_SCN_ALIGN_2BYTES              0x00200000
                #define IMAGE_SCN_ALIGN_4BYTES              0x00300000
                #define IMAGE_SCN_ALIGN_8BYTES              0x00400000
                #define IMAGE_SCN_ALIGN_16BYTES             0x00500000  // Default
                #define IMAGE_SCN_ALIGN_32BYTES             0x00600000
                #define IMAGE_SCN_ALIGN_64BYTES             0x00700000
                //                      0x00800000 - Unused
                #define IMAGE_SCN_LNK_NRELOC_OVFL           0x01000000
                #define IMAGE_SCN_MEM_DISCARDABLE           0x02000000
                #define IMAGE_SCN_MEM_NOT_CACHED            0x04000000
                #define IMAGE_SCN_MEM_NOT_PAGED             0x08000000
                #define IMAGE_SCN_MEM_SHARED                0x10000000
                #define IMAGE_SCN_MEM_EXECUTE               0x20000000
                #define IMAGE_SCN_MEM_READ                  0x40000000
                #define IMAGE_SCN_MEM_WRITE                 0x80000000
                            // 24:
    } IMAGE_SECTION_HEADER,*PIMAGE_SECTION_HEADER;

    #define IMAGE_SIZEOF_SECTION_HEADER 40

    /*
     *@@ IMAGE_IMPORT_BY_NAME:
     *      import name entry.
     *
     *@@added V0.9.16 (2002-01-09) [umoeller]
     */

    typedef struct _IMAGE_IMPORT_BY_NAME
    {
        USHORT  Hint;
        BYTE    Name[1];
    } IMAGE_IMPORT_BY_NAME,*PIMAGE_IMPORT_BY_NAME;

    /*
     *@@ IMAGE_THUNK_DATA:
     *      import thunk.
     *
     *@@added V0.9.16 (2002-01-09) [umoeller]
     */

    typedef struct _IMAGE_THUNK_DATA
    {
        union
        {
            PBYTE       ForwarderString;
            PFN         Function;
            ULONG       Ordinal;
            PIMAGE_IMPORT_BY_NAME
                        AddressOfData;
        } u1;
    } IMAGE_THUNK_DATA,*PIMAGE_THUNK_DATA;

    /*
     *@@ IMAGE_IMPORT_DESCRIPTOR:
     *      import module directory.
     *
     *@@added V0.9.16 (2002-01-09) [umoeller]
     */

    typedef struct _IMAGE_IMPORT_DESCRIPTOR
    {
        union
        {
            ULONG       Characteristics; // 0 for terminating null import descriptor
            PIMAGE_THUNK_DATA
                        OriginalFirstThunk;   // RVA to original unbound IAT
        } u;
        ULONG           TimeDateStamp;  // 0 if not bound,
                                        // -1 if bound, and real date\time stamp
                                        //    in IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT
                                        // (new BIND)
                                        // otherwise date/time stamp of DLL bound to
                                        // (Old BIND)
        ULONG           ForwarderChain; // -1 if no forwarders
        ULONG           Name;
        // RVA to IAT (if bound this IAT has actual addresses)
        PIMAGE_THUNK_DATA
                        FirstThunk;
    } IMAGE_IMPORT_DESCRIPTOR,*PIMAGE_IMPORT_DESCRIPTOR;

    #define IMAGE_ORDINAL_FLAG      0x80000000
    #define IMAGE_SNAP_BY_ORDINAL(Ordinal)  ((Ordinal & IMAGE_ORDINAL_FLAG) != 0)
    #define IMAGE_ORDINAL(Ordinal)      (Ordinal & 0xffff)

    /*
     *@@ IMAGE_EXPORT_DIRECTORY:
     *      export module directory
     *
     *@@added V0.9.16 (2002-01-09) [umoeller]
     */

    typedef struct _IMAGE_EXPORT_DIRECTORY
    {
        ULONG   Characteristics;
        ULONG   TimeDateStamp;
        USHORT  MajorVersion;
        USHORT  MinorVersion;
        ULONG   Name;
        ULONG   Base;
        ULONG   NumberOfFunctions;
        ULONG   NumberOfNames;
        PULONG  *AddressOfFunctions;
        PULONG  *AddressOfNames;
        PUSHORT *AddressOfNameOrdinals;
        //  u_char ModuleName[1];
    } IMAGE_EXPORT_DIRECTORY,*PIMAGE_EXPORT_DIRECTORY;

    /*
     *@@ IMAGE_RESOURCE_DIRECTORY:
     *
     *@@added V0.9.16 (2002-01-09) [umoeller]
     */

    typedef struct _IMAGE_RESOURCE_DIRECTORY
    {
        ULONG       Characteristics;
        ULONG       TimeDateStamp;
        USHORT      MajorVersion;
        USHORT      MinorVersion;
        USHORT      NumberOfNamedEntries;
        USHORT      NumberOfIdEntries;
        //  IMAGE_RESOURCE_DIRECTORY_ENTRY DirectoryEntries[];
    } IMAGE_RESOURCE_DIRECTORY,*PIMAGE_RESOURCE_DIRECTORY;

    #define IMAGE_RESOURCE_NAME_IS_STRING       0x80000000
    #define IMAGE_RESOURCE_DATA_IS_DIRECTORY    0x80000000

    /*
     *@@ IMAGE_RESOURCE_DIRECTORY_ENTRY:
     *
     *@@added V0.9.16 (2002-01-09) [umoeller]
     */

    typedef struct _IMAGE_RESOURCE_DIRECTORY_ENTRY
    {
        union
        {
            struct
            {
                unsigned NameOffset:31;
                unsigned NameIsString:1;
            } s;
            ULONG   Name;
            USHORT  Id;
        } u1;
        union
        {
            ULONG   OffsetToData;
            struct
            {
                unsigned OffsetToDirectory:31;
                unsigned DataIsDirectory:1;
            } s;
        } u2;
    } IMAGE_RESOURCE_DIRECTORY_ENTRY, *PIMAGE_RESOURCE_DIRECTORY_ENTRY;

    /*
     *@@ IMAGE_RESOURCE_DIRECTORY_STRING:
     *
     *@@added V0.9.16 (2002-01-09) [umoeller]
     */

    typedef struct _IMAGE_RESOURCE_DIRECTORY_STRING
    {
        USHORT  Length;
        CHAR    NameString[1];
    } IMAGE_RESOURCE_DIRECTORY_STRING, *PIMAGE_RESOURCE_DIRECTORY_STRING;

    typedef USHORT WCHAR;

    /*
     *@@ IMAGE_RESOURCE_DIR_STRING_U:
     *
     *@@added V0.9.16 (2002-01-09) [umoeller]
     */

    typedef struct _IMAGE_RESOURCE_DIR_STRING_U
    {
        USHORT  Length;
        WCHAR   NameString[ 1 ];
    } IMAGE_RESOURCE_DIR_STRING_U,*PIMAGE_RESOURCE_DIR_STRING_U;

    /*
     *@@ IMAGE_RESOURCE_DATA_ENTRY:
     *
     *@@added V0.9.16 (2002-01-09) [umoeller]
     */

    typedef struct _IMAGE_RESOURCE_DATA_ENTRY
    {
        ULONG   OffsetToData;
        ULONG   Size;
        ULONG   CodePage;
        ULONG   ResourceHandle;
    } IMAGE_RESOURCE_DATA_ENTRY,*PIMAGE_RESOURCE_DATA_ENTRY;

    /*
     *@@ IMAGE_BASE_RELOCATION:
     *
     *@@added V0.9.16 (2002-01-09) [umoeller]
     */

    typedef struct _IMAGE_BASE_RELOCATION
    {
        ULONG   VirtualAddress;
        ULONG   SizeOfBlock;
        USHORT  TypeOffset[1];
    } IMAGE_BASE_RELOCATION,*PIMAGE_BASE_RELOCATION;

    /*
     *@@ IMAGE_LOAD_CONFIG_DIRECTORY:
     *
     *@@added V0.9.16 (2002-01-09) [umoeller]
     */

    typedef struct _IMAGE_LOAD_CONFIG_DIRECTORY
    {
        ULONG   Characteristics;
        ULONG   TimeDateStamp;
        USHORT  MajorVersion;
        USHORT  MinorVersion;
        ULONG   GlobalFlagsClear;
        ULONG   GlobalFlagsSet;
        ULONG   CriticalSectionDefaultTimeout;
        ULONG   DeCommitFreeBlockThreshold;
        ULONG   DeCommitTotalFreeThreshold;
        PVOID   LockPrefixTable;
        ULONG   MaximumAllocationSize;
        ULONG   VirtualMemoryThreshold;
        ULONG   ProcessHeapFlags;
        ULONG   Reserved[ 4 ];
    } IMAGE_LOAD_CONFIG_DIRECTORY,*PIMAGE_LOAD_CONFIG_DIRECTORY;

    typedef VOID (* /* CALLBACK */ PIMAGE_TLS_CALLBACK)(PVOID  DllHandle,
                                                 ULONG Reason,
                                                 PVOID  Reserved);

    /*
     *@@ IMAGE_TLS_DIRECTORY:
     *
     *@@added V0.9.16 (2002-01-09) [umoeller]
     */

    typedef struct _IMAGE_TLS_DIRECTORY
    {
        ULONG   StartAddressOfRawData;
        ULONG   EndAddressOfRawData;
        PULONG  AddressOfIndex;
        PIMAGE_TLS_CALLBACK *AddressOfCallBacks;
        ULONG   SizeOfZeroFill;
        ULONG   Characteristics;
    } IMAGE_TLS_DIRECTORY,*PIMAGE_TLS_DIRECTORY;

    /*
     *@@ IMAGE_DEBUG_DIRECTORY:
     *      the IMAGE_DEBUG_DIRECTORY data directory points
     *      to an array of these structures.
     *
     *@@added V0.9.16 (2002-01-09) [umoeller]
     */

    typedef struct _IMAGE_DEBUG_DIRECTORY
    {
        ULONG   Characteristics;
        ULONG   TimeDateStamp;
        USHORT  MajorVersion;
        USHORT  MinorVersion;
        ULONG   Type;
        ULONG   SizeOfData;
        ULONG   AddressOfRawData;
        ULONG   ulPointerToRawData;
    } IMAGE_DEBUG_DIRECTORY,*PIMAGE_DEBUG_DIRECTORY;

    /*
     * The type field above can take these (plus a few other
     * irrelevant) values.
     */
    #define IMAGE_DEBUG_TYPE_UNKNOWN    0
    #define IMAGE_DEBUG_TYPE_COFF       1
    #define IMAGE_DEBUG_TYPE_CODEVIEW   2
    #define IMAGE_DEBUG_TYPE_FPO        3
    #define IMAGE_DEBUG_TYPE_MISC       4
    #define IMAGE_DEBUG_TYPE_EXCEPTION  5
    #define IMAGE_DEBUG_TYPE_FIXUP      6
    #define IMAGE_DEBUG_TYPE_OMAP_TO_SRC    7
    #define IMAGE_DEBUG_TYPE_OMAP_FROM_SRC  8

    // generic relocation types
    #define IMAGE_REL_BASED_ABSOLUTE        0
    #define IMAGE_REL_BASED_HIGH            1
    #define IMAGE_REL_BASED_LOW         2
    #define IMAGE_REL_BASED_HIGHLOW         3
    #define IMAGE_REL_BASED_HIGHADJ         4
    #define IMAGE_REL_BASED_MIPS_JMPADDR        5
    #define IMAGE_REL_BASED_SECTION         6
    #define IMAGE_REL_BASED_REL         7
    #define IMAGE_REL_BASED_MIPS_JMPADDR16      9
    #define IMAGE_REL_BASED_IA64_IMM64      9 // yes, 9 too
    #define IMAGE_REL_BASED_DIR64           10
    #define IMAGE_REL_BASED_HIGH3ADJ        11

    // I386 relocation types
    #define IMAGE_REL_I386_ABSOLUTE         0
    #define IMAGE_REL_I386_DIR16            1
    #define IMAGE_REL_I386_REL16            2
    #define IMAGE_REL_I386_DIR              6
    #define IMAGE_REL_I386_DIR32NB          7
    #define IMAGE_REL_I386_SEG12            9
    #define IMAGE_REL_I386_SECTION          10
    #define IMAGE_REL_I386_SECREL           11
    #define IMAGE_REL_I386_REL              20

    // MIPS relocation types
    #define IMAGE_REL_MIPS_ABSOLUTE     0x0000
    #define IMAGE_REL_MIPS_REFHALF      0x0001
    #define IMAGE_REL_MIPS_REFWORD      0x0002
    #define IMAGE_REL_MIPS_JMPADDR      0x0003
    #define IMAGE_REL_MIPS_REFHI        0x0004
    #define IMAGE_REL_MIPS_REFLO        0x0005
    #define IMAGE_REL_MIPS_GPREL        0x0006
    #define IMAGE_REL_MIPS_LITERAL      0x0007
    #define IMAGE_REL_MIPS_SECTION      0x000A
    #define IMAGE_REL_MIPS_SECREL       0x000B
    #define IMAGE_REL_MIPS_SECRELLO     0x000C
    #define IMAGE_REL_MIPS_SECRELHI     0x000D
    #define IMAGE_REL_MIPS_JMPADDR16    0x0010
    #define IMAGE_REL_MIPS_REFWORDNB    0x0022
    #define IMAGE_REL_MIPS_PAIR         0x0025

    // ALPHA relocation types
    #define IMAGE_REL_ALPHA_ABSOLUTE    0x0000
    #define IMAGE_REL_ALPHA_REFLONG     0x0001
    #define IMAGE_REL_ALPHA_REFQUAD     0x0002
    #define IMAGE_REL_ALPHA_GPREL       0x0003
    #define IMAGE_REL_ALPHA_LITERAL     0x0004
    #define IMAGE_REL_ALPHA_LITUSE      0x0005
    #define IMAGE_REL_ALPHA_GPDISP      0x0006
    #define IMAGE_REL_ALPHA_BRADDR      0x0007
    #define IMAGE_REL_ALPHA_HINT        0x0008
    #define IMAGE_REL_ALPHA_INLINE_REFLONG  0x0009
    #define IMAGE_REL_ALPHA_REFHI       0x000A
    #define IMAGE_REL_ALPHA_REFLO       0x000B
    #define IMAGE_REL_ALPHA_PAIR        0x000C
    #define IMAGE_REL_ALPHA_MATCH       0x000D
    #define IMAGE_REL_ALPHA_SECTION     0x000E
    #define IMAGE_REL_ALPHA_SECREL      0x000F
    #define IMAGE_REL_ALPHA_REFLONGNB   0x0010
    #define IMAGE_REL_ALPHA_SECRELLO    0x0011
    #define IMAGE_REL_ALPHA_SECRELHI    0x0012
    #define IMAGE_REL_ALPHA_REFQ3       0x0013
    #define IMAGE_REL_ALPHA_REFQ2       0x0014
    #define IMAGE_REL_ALPHA_REFQ1       0x0015
    #define IMAGE_REL_ALPHA_GPRELLO     0x0016
    #define IMAGE_REL_ALPHA_GPRELHI     0x0017

    // PowerPC relocation types
    #define IMAGE_REL_PPC_ABSOLUTE          0x0000
    #define IMAGE_REL_PPC_ADDR64            0x0001
    #define IMAGE_REL_PPC_ADDR              0x0002
    #define IMAGE_REL_PPC_ADDR24            0x0003
    #define IMAGE_REL_PPC_ADDR16            0x0004
    #define IMAGE_REL_PPC_ADDR14            0x0005
    #define IMAGE_REL_PPC_REL24             0x0006
    #define IMAGE_REL_PPC_REL14             0x0007
    #define IMAGE_REL_PPC_TOCREL16          0x0008
    #define IMAGE_REL_PPC_TOCREL14          0x0009
    #define IMAGE_REL_PPC_ADDR32NB          0x000A
    #define IMAGE_REL_PPC_SECREL            0x000B
    #define IMAGE_REL_PPC_SECTION           0x000C
    #define IMAGE_REL_PPC_IFGLUE            0x000D
    #define IMAGE_REL_PPC_IMGLUE            0x000E
    #define IMAGE_REL_PPC_SECREL16          0x000F
    #define IMAGE_REL_PPC_REFHI             0x0010
    #define IMAGE_REL_PPC_REFLO             0x0011
    #define IMAGE_REL_PPC_PAIR              0x0012
    #define IMAGE_REL_PPC_SECRELLO          0x0013
    #define IMAGE_REL_PPC_SECRELHI          0x0014
    #define IMAGE_REL_PPC_GPREL             0x0015
    #define IMAGE_REL_PPC_TYPEMASK          0x00FF
    // modifier bits
    #define IMAGE_REL_PPC_NEG               0x0100
    #define IMAGE_REL_PPC_BRTAKEN           0x0200
    #define IMAGE_REL_PPC_BRNTAKEN          0x0400
    #define IMAGE_REL_PPC_TOCDEFN           0x0800

    // SH3 ? relocation type
    #define IMAGE_REL_SH3_ABSOLUTE          0x0000
    #define IMAGE_REL_SH3_DIRECT16          0x0001
    #define IMAGE_REL_SH3_DIRECT            0x0002
    #define IMAGE_REL_SH3_DIRECT8           0x0003
    #define IMAGE_REL_SH3_DIRECT8_WORD      0x0004
    #define IMAGE_REL_SH3_DIRECT8_LONG      0x0005
    #define IMAGE_REL_SH3_DIRECT4           0x0006
    #define IMAGE_REL_SH3_DIRECT4_WORD      0x0007
    #define IMAGE_REL_SH3_DIRECT4_LONG      0x0008
    #define IMAGE_REL_SH3_PCREL8_WORD       0x0009
    #define IMAGE_REL_SH3_PCREL8_LONG       0x000A
    #define IMAGE_REL_SH3_PCREL12_WORD      0x000B
    #define IMAGE_REL_SH3_STARTOF_SECTION   0x000C
    #define IMAGE_REL_SH3_SIZEOF_SECTION    0x000D
    #define IMAGE_REL_SH3_SECTION           0x000E
    #define IMAGE_REL_SH3_SECREL            0x000F
    #define IMAGE_REL_SH3_DIRECT32_NB       0x0010

    // ARM (Archimedes?) relocation types
    #define IMAGE_REL_ARM_ABSOLUTE      0x0000
    #define IMAGE_REL_ARM_ADDR          0x0001
    #define IMAGE_REL_ARM_ADDR32NB      0x0002
    #define IMAGE_REL_ARM_BRANCH24      0x0003
    #define IMAGE_REL_ARM_BRANCH11      0x0004
    #define IMAGE_REL_ARM_SECTION       0x000E
    #define IMAGE_REL_ARM_SECREL        0x000F

    // IA64 relocation types
    #define IMAGE_REL_IA64_ABSOLUTE     0x0000
    #define IMAGE_REL_IA64_IMM14        0x0001
    #define IMAGE_REL_IA64_IMM22        0x0002
    #define IMAGE_REL_IA64_IMM64        0x0003
    #define IMAGE_REL_IA64_DIR          0x0004
    #define IMAGE_REL_IA64_DIR64        0x0005
    #define IMAGE_REL_IA64_PCREL21B     0x0006
    #define IMAGE_REL_IA64_PCREL21M     0x0007
    #define IMAGE_REL_IA64_PCREL21F     0x0008
    #define IMAGE_REL_IA64_GPREL22      0x0009
    #define IMAGE_REL_IA64_LTOFF22      0x000A
    #define IMAGE_REL_IA64_SECTION      0x000B
    #define IMAGE_REL_IA64_SECREL22     0x000C
    #define IMAGE_REL_IA64_SECREL64I    0x000D
    #define IMAGE_REL_IA64_SECREL       0x000E
    #define IMAGE_REL_IA64_LTOFF64      0x000F
    #define IMAGE_REL_IA64_DIR32NB      0x0010
    #define IMAGE_REL_IA64_RESERVED_11  0x0011
    #define IMAGE_REL_IA64_RESERVED_12  0x0012
    #define IMAGE_REL_IA64_RESERVED_13  0x0013
    #define IMAGE_REL_IA64_RESERVED_14  0x0014
    #define IMAGE_REL_IA64_RESERVED_15  0x0015
    #define IMAGE_REL_IA64_RESERVED_16  0x0016
    #define IMAGE_REL_IA64_ADDEND       0x001F

    /*
     *@@ IMAGE_SEPARATE_DEBUG_HEADER:
     *      structure that appears at the very start of a .DBG file.
     *
     *@@added V0.9.16 (2002-01-09) [umoeller]
     */

    typedef struct _IMAGE_SEPARATE_DEBUG_HEADER
    {
        USHORT  Signature;
        USHORT  Flags;
        USHORT  Machine;
        USHORT  Characteristics;
        ULONG   TimeDateStamp;
        ULONG   CheckSum;
        ULONG   ImageBase;
        ULONG   SizeOfImage;
        ULONG   NumberOfSections;
        ULONG   ExportedNamesSize;
        ULONG   DebugDirectorySize;
        ULONG   Reserved[ 3 ];
    } IMAGE_SEPARATE_DEBUG_HEADER,*PIMAGE_SEPARATE_DEBUG_HEADER;

    #define IMAGE_SEPARATE_DEBUG_SIGNATURE 0x4944

    /*
     *@@ MESSAGE_RESOURCE_ENTRY:
     *
     *@@added V0.9.16 (2002-01-09) [umoeller]
     */

    typedef struct tagMESSAGE_RESOURCE_ENTRY
    {
        USHORT  Length;
        USHORT  Flags;
        BYTE    Text[1];
    } MESSAGE_RESOURCE_ENTRY,*PMESSAGE_RESOURCE_ENTRY;
    #define MESSAGE_RESOURCE_UNICODE    0x0001

    /*
     *@@ MESSAGE_RESOURCE_BLOCK:
     *
     *@@added V0.9.16 (2002-01-09) [umoeller]
     */

    typedef struct tagMESSAGE_RESOURCE_BLOCK
    {
        ULONG   LowId;
        ULONG   HighId;
        ULONG   OffsetToEntries;
    } MESSAGE_RESOURCE_BLOCK, *PMESSAGE_RESOURCE_BLOCK;

    /*
     *@@ MESSAGE_RESOURCE_DATA:
     *
     *@@added V0.9.16 (2002-01-09) [umoeller]
     */

    typedef struct tagMESSAGE_RESOURCE_DATA
    {
        ULONG           NumberOfBlocks;
        MESSAGE_RESOURCE_BLOCK  Blocks[ 1 ];
    } MESSAGE_RESOURCE_DATA,*PMESSAGE_RESOURCE_DATA;

    /*
     *@@ IMAGE_IMPORT_MODULE_DIRECTORY:
     *
     *@@added V0.9.16 (2002-01-09) [umoeller]
     */

    typedef struct _IMAGE_IMPORT_MODULE_DIRECTORY // tagImportDirectory
    {
        ULONG    dwRVAFunctionNameList;
        ULONG    dwUseless1;
        ULONG    dwUseless2;
        ULONG    dwRVAModuleName;
        ULONG    dwRVAFunctionAddressList;
    } IMAGE_IMPORT_MODULE_DIRECTORY, *PIMAGE_IMPORT_MODULE_DIRECTORY;

    #pragma pack()

    /********************************************************************
     *
     *   Declarations and APIs
     *
     ********************************************************************/

#ifndef __STRIP_DOWN_EXECUTABLE__
// for mini stubs in warpin, which has its own
// implementation of this

    /*
     *@@ EXECUTABLE:
     *      structure used with all the exeh*
     *      functions.
     */

    typedef struct _EXECUTABLE
    {
        // executable opened by doshOpen;
        // note, NULL for .COM, .BAT, .CMD files (V0.9.16)
        PXFILE          pFile;

        // Most of the following fields are set by
        // exehOpen if NO_ERROR is returned:

        // old DOS EXE header;
        // warning, this is NULL if
        // --   the executable has a new header only
        //      (NOSTUB, V0.9.12);
        // --   for .COM, .BAT, .CMD files (V0.9.16)
        DOSEXEHEADER    DosExeHeader;       // no longer a ptr, but inline struct V1.0.2 (2003-11-13) [umoeller]
        ULONG           cbDosExeHeader;     // if != 0, DosExeHeader is present

        // New Executable (NE) header, if ulExeFormat == EXEFORMAT_NE
        PNEHEADER       pNEHeader;
        ULONG           cbNEHeader;

        // Linear Executable (LX) header, if ulExeFormat == EXEFORMAT_LX
        PLXHEADER       pLXHeader;
        ULONG           cbLXHeader;

        // Portable Executable (PE) header, if ulExeFormat == EXEFORMAT_PE
        PPEHEADER       pPEHeader;
        ULONG           cbPEHeader;
                    // note, this one may vary; however, all Win32 programs
                    // will have sizeof(PEHEADER) in here since they all
                    // use both the standard and the optional PE header

        // module analysis (always set, otherwise error would be
        // returned by exehOpen):
        ULONG           ulExeFormat;
#endif
                    #define EXEFORMAT_OLDDOS        1
                    #define EXEFORMAT_NE            2
                    #define EXEFORMAT_PE            3
                    #define EXEFORMAT_LX            4
                    #define EXEFORMAT_TEXT_BATCH    5
                    #define EXEFORMAT_TEXT_CMD      6       // REXX or plain OS/2 batch
                    #define EXEFORMAT_COM           7       // added V0.9.16 (2002-01-04) [umoeller]

#ifndef __STRIP_DOWN_EXECUTABLE__
        BOOL            fLibrary,           // TRUE if this is a DLL
                                            // (works for NE, LX, PE)
                        f32Bits;            // TRUE if this a 32-bits module
                                            // (TRUE always for PE)

        ULONG           ulOS;
#endif
                // target operating system as flagged in one of
                // the EXE headers; one of:
                    #define EXEOS_UNKNOWN           0
                                // set for non-standard PE files too
                    #define EXEOS_DOS3              1
                    #define EXEOS_DOS4              2
                                // there is a flag for this in NE
                    #define EXEOS_OS2               3
                    #define EXEOS_WIN16             4
                    #define EXEOS_WIN386            5
                                // according to IBM, there are flags
                                // for this both in NE and LX
                    #define EXEOS_WIN32_GUI         6
                                // Win32 GUI (see IMAGE_OPTIONAL_HEADER.usSubsystem),
                                // if PE optional header was successfully read
                    #define EXEOS_WIN32_CLI         7
                                // Win32 command line (see IMAGE_OPTIONAL_HEADER.usSubsystem),
                                // if PE optional header was successfully read

#ifndef __STRIP_DOWN_EXECUTABLE__
        // The following fields are only set after
        // an extra call to exehQueryBldLevel (NE and LX only):

        PSZ             pszDescription;
                // whole string (first non-res name tbl entry)
        PSZ             pszVendor;
                // vendor substring (if IBM BLDLEVEL format)
        PSZ             pszVersion;
                // version substring (if IBM BLDLEVEL format)

        PSZ             pszInfo;
                // module info substring (if IBM BLDLEVEL format)

        // if pszInfo is extended DESCRIPTION field, the following
        // are set as well:
        PSZ             pszBuildDateTime,
                        pszBuildMachine,
                        pszASD,
                        pszLanguage,
                        pszCountry,
                        pszRevision,
                        pszUnknown,
                        pszFixpak;

        // the following fields are set after exehLoadLXMaps (LX only)
        BOOL                    fLXMapsLoaded;  // TRUE after good exehLoadLXMaps
        RESOURCETABLEENTRY      *pRsTbl;        // pLXHeader->ulResTblCnt
        OBJECTTABLEENTRY        *pObjTbl;       // pLXHeader->ulObjCount
        OBJECTPAGETABLEENTRY    *pObjPageTbl;   // pLXHeader->ulPageCount

        // the following fields are set after exehLoadOS2NEMaps (OS/2 NE only)
        BOOL                    fOS2NEMapsLoaded;   // TRUE after good exehLoadOS2NEMaps
        POS2NERESTBLENTRY       paOS2NEResTblEntry;
        POS2NESEGMENT           paOS2NESegments;
    } EXECUTABLE, *PEXECUTABLE;

    APIRET exehOpen(const char* pcszExecutable,
                    PEXECUTABLE* ppExec);

    APIRET exehQueryBldLevel(PEXECUTABLE pExec);

    #ifdef INCL_WINPROGRAMLIST

        // additional PROG_* flags for exehQueryProgType; moved these here
        // from app.h V1.0.1 (2003-01-17) [umoeller]

        // #define PROG_XWP_DLL            998      // dynamic link library
                    // removed, PROG_DLL exists already
                    // V0.9.16 (2001-10-06)

        #define PROG_WIN32              990     // added V0.9.16 (2001-12-08) [umoeller]

        APIRET exehQueryProgType(const EXECUTABLE *pExec,
                                 PROGCATEGORY *pulProgType);

        PCSZ exehDescribeProgType(PROGCATEGORY progc);

    #endif

    /*
     *@@ FSYSMODULE:
     *
     *@@added V0.9.9 (2001-03-11) [lafaix]
     */

    typedef struct _FSYSMODULE
    {
        CHAR achModuleName[256];
    } FSYSMODULE, *PFSYSMODULE;

    APIRET exehQueryImportedModules(PEXECUTABLE pExec,
                                    PFSYSMODULE *ppaModules,
                                    PULONG pcModules);

    APIRET exehFreeImportedModules(PFSYSMODULE paModules);

    /*
     *@@ FSYSFUNCTION:
     *
     *@@added V0.9.9 (2001-03-11) [lafaix]
     */

    typedef struct _FSYSFUNCTION
    {
        ULONG ulOrdinal;
        ULONG ulType;
        CHAR achFunctionName[256];
    } FSYSFUNCTION, *PFSYSFUNCTION;

    APIRET exehQueryExportedFunctions(PEXECUTABLE pExec,
                                      PFSYSFUNCTION *ppaFunctions,
                                      PULONG pcFunctions);

    APIRET exehFreeExportedFunctions(PFSYSFUNCTION paFunctions);

    #define WINRT_CURSOR                1
    #define WINRT_BITMAP                2
    #define WINRT_ICON                  3
    #define WINRT_MENU                  4
    #define WINRT_DIALOG                5
    #define WINRT_STRING                6
    #define WINRT_FONTDIR               7
    #define WINRT_FONT                  8
    #define WINRT_ACCELERATOR           9
    #define WINRT_RCDATA               10
    #define WINRT_MESSAGELIST          11       // Win32
    #define WINRT_GROUP_CURSOR         12       // Win32
    #define WINRT_GROUP_ICON           13       // Win32

    /*
     *@@ FSYSRESOURCE:
     *
     *@@added V0.9.7 (2000-12-18) [lafaix]
     */

    typedef struct _FSYSRESOURCE
    {
        ULONG ulID;                     // resource ID
        ULONG ulType;                   // resource type
        ULONG ulSize;                   // resource size in bytes
        ULONG ulFlag;                   // resource flags

    } FSYSRESOURCE, *PFSYSRESOURCE;

    APIRET exehQueryResources(PEXECUTABLE pExec,
                              PFSYSRESOURCE *ppaResources,
                              PULONG pcResources);

    APIRET exehFreeResources(PFSYSRESOURCE paResources);

    APIRET exehLoadLXMaps(PEXECUTABLE pExec);

    VOID exehFreeLXMaps(PEXECUTABLE pExec);

    APIRET exehLoadLXResource(PEXECUTABLE pExec,
                              ULONG ulType,
                              ULONG idResource,
                              PBYTE *ppbResData,
                              PULONG pulOffset,
                              PULONG pcbResData);

    APIRET exehLoadOS2NEMaps(PEXECUTABLE pExec);

    VOID exehFreeNEMaps(PEXECUTABLE pExec);

    APIRET exehLoadOS2NEResource(PEXECUTABLE pExec,
                                 ULONG ulType,
                                 ULONG idResource,
                                 PBYTE *ppbResData,
                                 PULONG pcbResData);

    APIRET exehClose(PEXECUTABLE *ppExec);

#endif

#endif

#if __cplusplus
}
#endif

