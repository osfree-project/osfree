/*
 * Copyright (C) 1999-2001, The University of Queensland
 * Copyright (C) 1999-2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:       backend.cc
 * OVERVIEW:   This file contains general backend functions. Writes data files,
 *              readData.j, and link script
 *
 * Copyright (C) 1999-2001, The University of Queensland, BT group
 * Copyright (C) 1999-2001, Sun Microsystems, Inc
 *============================================================================*/

/*
 * 03 May 99 - Mike: Initial version
 * 07 May 99 - Mike: Moved FinalOutput() here from driver.cc; don;t write zero
 *              length data files
 * 27 May 99 - Mike: Data files are produced as .s files that are assembled
 * 11 Aug 99 - Mike: 1 or 2 data file "plans" implemented for constructing the
 *              target binary files
 * 12 Aug 99 - Mike: Always use 2 data files. bOneDataPlan -> bBlockMoveReqd
 * 17 Aug 99 - Mike: Oops! Would not compile for Pentium target
 *  8 Dec 99 - CC: moved program startup to jvm.c (printasmstart) to setup
 *              name of program correctly; readData.j is now a skeleton of 
 *              the data sections only.
 * 13 Mar 00 - Mike: Removed redundant "Error: " from error message strings
 * 21 Mar 00 - Mike: finalOutput() creates file _globals.c with global array
 * 06 Jul 00 - Mike: Allow for gaps in the rodata and rwdata files
 * 15 Aug 00 - Mike: Save %g1 in the generated uqbt_start function
 * 19 Sep 00 - Mike: proc_backend -> addSourceFile; removed last vpo code
 * 12 Nov 00 - Brian: Add call to JVM backend. Fixed bug where the total number
 *              of bytes written did not include space (sections) skipped over.
 * 14 Feb 01 - Brian: Corrected file used when creating the _globals.c file.
 * 05 Mar 01 - Brian: Changed finalOutput() to not return prematurely when
 *             creating JVM class files from source Palm executables: it must
 *             generate the data, link files, block move code, etc.
 * 14 Mar 01 - Mike: Commented out "will need the block move plan" message
 * 24 Apr 01 - Brian: Add addVPOSourceFile() method for use by VPO back end.
 * 10 May 01 - Mike: Copy code section if requested (with -C)
 * 16 May 01 - Mike: Shameful hack for PA/Risc to Linux
 * 21 May 01 - Nathan: Paths in Makefile now obtained from configure variables,
 *               changed #ifdef LINUX blocks to #if TARGET_GNU_LD == 1
 * 25 Jul 01 - Mike: Minor changes to quell warnings (when compiling at -O4)
 * 30 Jul 01 - Mike: Only do the early call to of.link() if Palm
 * 01 Aug 01 - Mike: No longer generate _globals.c; emit declaration for
 *              _globals when come across it in emitData()
 * 13 Aug 01 - Mike: One of the #ifndef LUNUX lines was not changed correctly
 *              Caused some pentium programs needing the block move plan to have
 *              wrong data
 * 23 Aug 01 - Brian: generate a Linux map file if the target is ARM.
 * 23 Aug 01 - Cristina: Made call to createReadData() conditional on 
 *		generation of JVM code (this call generates readData.j). 
 * 23 Oct 01 - Mike: Changes for seg.code (must have only one segment for
 *              the source program's .text and .rodata sections)
 */

#include "global.h"
#include "outfile.h"            // For the OutputFile class
#include "prog.h"               // For getArgv0() etc
#include "proc.h"               // For getName() etc
#include "ElfBinaryFile.h"      // For getNumSections() etc
#include "PalmBinaryFile.h"     // For GenerateBinFiles() etc
#include "options.h"            // For class options (progOptions object)

// Prototypes
void emitData(ofstream& df, SectionInfo* pSect, int skip = 0, ADDRESS agp = 0);

// The declaration for the read-only and read-write data sections
// Note: these two strings must be exactly the same length, so pad the shorter
// with spaces, if needed (not inside the double quotes, though)
#define RODATA_DCL ".section .uqbt.rodata,\"a\"\n\n"
#define RWDATA_DCL ".section .uqbt.data,\"aw\" \n\n"

#define ROCODE_DCL ".section .uqbt.code,\"a\"\n\n"

#define BINARY_ALIGN 4          // Elf32 sections are 4 byte aligned

/*==============================================================================
 * FUNCTION:      addSourceFile
 * OVERVIEW:      Add the source file for this proc to the Makefile
 * PARAMETERS:    proc: Proc object for this procedure
 *                of: reference to the OutputFile object
 * RETURNS:       <nothing>
 *============================================================================*/
void addSourceFile(Proc* proc, OutputFile& of)
{
    const char* szName = proc->getName();
    string Name = string(szName);
    // Add the source file to the OutputFile object, which manages all those
    // temporary files
    of.addSource(Name + ".c");
}

/*==============================================================================
 * FUNCTION:      addVPOSourceFile
 * OVERVIEW:      Add the VPO file for this proc to the Makefile
 * PARAMETERS:    proc: Proc object for this procedure
 *                of: reference to the OutputFile object
 * RETURNS:       <nothing>
 *============================================================================*/
void addVPOSourceFile(Proc* proc, OutputFile& of)
{
    const char* szName = proc->getName();
    string Name = string(szName);
    // Add the VPO source file to the OutputFile object, which manages all
    // make-related files
    of.addSource(Name + ".cex");
}

/*==============================================================================
 * FUNCTION:    emitData
 * OVERVIEW:    Emit data statements for a data section. They are of the form
 *              .byte nnn,mmm,...,zzz
 *              Emits a size rounded up to the next BINARY_ALIGN, so sections
 *              can be more easily concatenated
 * PARAMETERS:  df: reference to the ofstream object representing the data file
 *              pSect: pointer to a SectionInfo struct that has section
 *              addresses and section sizes
 *              skip: Number of bytes to skip (defaults to 0)
 *              agp: Address of the abstract global pointer; emit a definition
 *                for _globals here (if found)
 * RETURNS:     <nothing>
 *============================================================================*/
void declareGlobal(bool& doneGlobal, bool& lineOpen, ofstream& df)
{
    // "Local" function to close the existing line, if any, and declare the
    // symbol "_globals"
    if (doneGlobal) return;
    if (lineOpen)
        df << "\n";
    df << ".global _globals\n";     // It's a global symbol
    df << "_globals:\n";
    if (lineOpen)
        df << ".byte ";

    doneGlobal = true;
}

void emitData(ofstream& df, SectionInfo* pSect, int skip /* = 0 */,
    ADDRESS agp)
{
    // Emit .byte statements for each byte of the data file.
    unsigned size = (pSect->uSectionSize + BINARY_ALIGN-1) & ~(BINARY_ALIGN-1);
    ADDRESS addr = pSect->uNativeAddr + skip;
    bool doneGlobal = false, lineOpen = false;
    // Pack them 16 to a line, in decimal.
    for (unsigned chars=skip; chars < size;) {
        if (addr == agp)
            declareGlobal(doneGlobal, lineOpen, df);
        df << ".byte ";
        lineOpen = true;
        for (int i=0; i<16 && chars < size; i++) {
            df << (unsigned)((Byte*)pSect->uHostAddr)[chars];
            if ((i != 16-1) && (chars != size-1) &&
              (doneGlobal || (addr+1 != agp)))
                df << ",";
            chars++; addr++;
            if (addr == agp)
                declareGlobal(doneGlobal, lineOpen, df);
        }
        df << "\n";
        lineOpen = false;
    }
}

/*==============================================================================
 * FUNCTION:    createReadData
 * OVERVIEW:    Create the readData.j file. This is a file used only by uqbtj
 *              (i.e. only if generating java .class files rather than a
 *              target binary). This file is appended to the output of the main
 *              uqbtj tool (cc1), and the combined file is passed to jas to
 *              "assemble". This produces the .class file which can be run
 *              with "java <projectname>
 * PARAMETERS:  moduleName: string with the name of the module (.e.g. "fibo")
 *              ROdataStart: native address of the start of the read only
 *              data section
 *              ROdataLen: length of the read only data section
 *              RWdataStart: native addr of start of read/write section
 *              RWdataLen: length of r/w section
 *              BSSdataStart: native addr of start of .bss section
 *              sizeBSSdata: size of .bss section
 * RETURNS:     <nothing>
 *============================================================================*/
void createReadData(const string& moduleName,
    ADDRESS ROdataStart, int ROdataLen,
    ADDRESS RWdataStart, int RWdataLen,
    ADDRESS BSSdataStart, int sizeBSSdata)
{
    ofstream sf;
    sf.open((progOptions.outDir + "readData.sed").c_str());
    if (!sf)
    {
        error("could not open readData.sed for writing");
        return;
    }

    sf << dec;                      // All numbers in decimal

    sf << "s/$dsize_ro/" << ROdataLen << "/\n";
    sf << "s/$dsize_rw/" << RWdataLen << "/\n";
    sf << "s/$dsize_bss/" << sizeBSSdata << "/\n";
    sf << "s/$dstart_ro/" << ROdataStart << "/\n";
    sf << "s/$dstart_rw/" << RWdataStart << "/\n";
    sf << "s/$dstart_bss/" << BSSdataStart << "/\n";

    sf.close();

    // Run the sed script over readData.j, and pipe the result to
    // <outdir>/readData.j
    ostrstream sed;
    sed << "sed -f " << progOptions.outDir << "readData.sed ";
    sed << prog.getProgPath() << "readData.j > " << progOptions.outDir;
    sed << "readData.j";
    int ret = system(str(sed));
    if (ret) {
        error("readData.sed failed");
        return;
    }

    // Remove readData.sed; no longer needed
    unlink((progOptions.outDir + "readData.sed").c_str());
}

/*==============================================================================
 * FUNCTION:    createStart
 * OVERVIEW:    Create a file called "start.s", which will define the label
 *              uqbt_start, and will get control before _start
 * PARAMETERS:  src: Native address of start of block move (see below)
 *              dst: Native address of dest of block move
 *              size: size of block move
 * RETURNS:     <nothing>
 *============================================================================*/
// This startup code is needed only for those cases where the source machine's
// page size is less than that of the target machine (e.g. Pentium to Sparc),
// and also the addresses are such that the end of the read only section is
// in the same page as the start of the read-write section (on the target
// machine).
void createStart(ADDRESS src, ADDRESS dst, ADDRESS size)
{
    ofstream of;
    of.open((progOptions.outDir + "start.s").c_str());
    if (!of)
    {
        error("could not open startup source file start.s for writing");
        return;
    }
    of << ".section\t\".text\"\n";
    of << "\t.align 4\n";
    of << "\t.global uqbt_start\n";
    of << "uqbt_start:\n";

#if TGT == SPARC
    // Save general registers
    of << "\tsave %sp,-96,%sp\n";
    // Must also save %g1. This holds the address of a function that will be
    // passed to atexit(), so that it will be called at the end of main (or
    // when exit(3C) is called).
    of << "\tmov %g1, %l0\n";
    // Perform the block move using memmove(dst, src, size)
    // Note that it may be necessary to save %g1 as well (contains arg to
    // atexit())
    of << "\tsethi %hi(0x" << hex << dst << "),%o0\n";
    of << "\tor %o0,%lo(0x" << dst << "),%o0\n";
    of << "\tsethi %hi(0x" << hex << src << "),%o1\n";
    of << "\tor %o1,%lo(0x" << src << "),%o1\n";
    of << "\tsethi %hi(" << dec << size << "),%o2\n";
    of << "\tcall memmove\n";
    of << "\tor %o2,%lo(" << size << "),%o2\n";
    // Restore %g1, saved above in %l0
    of << "\tmov %l0, %g1\n";
    // Now jump to start. We must use the original register window, because
    // argc and argv come from %sp+64 and %sp+68 of the original window
    // The restore can go into the delay slot of the "call" (it will never
    // return anyway)
    of << "\tcall _start\n";
    of << "\trestore\n";
#else
    // For each target except those with the equal shortest page size (at
    // present, this is only the PENT target), we need code to perform the
    // block move. This has not been done for this target.
    error("createStart called for target " TGTNAME);
#endif

    // Create space for the block move. Relies on start.o being the last object
    // file linked (though might be able to be forced in the map file, have had
    // no success so far)
    // Note: this swells the target binary by over 4K! Can't seem to get
    // a "bss" solution to work so far
    of << ".section\t\".uqbt.data\",\"aw\"\n";
    // Note: .skip doesn't work for the intel assembler!
    of << "\t.=.+" << dec << dst - src << "\n";
}

/*==============================================================================
 * FUNCTION:    finalOutput
 * OVERVIEW:    Generate the final output file: create the data file
 *              "sources", adding lines as needed to the map file.
 *              Calls link() to generate the makefile
 * PARAMETERS:  of: reference to the OutputFile object that manages the temp-
 *                  orary files (data files, assembler files etc)
 * RETURNS:     <nothing>
 *============================================================================*/
void finalOutput(OutputFile& of)
{
#if SRC==MC68K
    // Assumption: targets that have _globals arrays (e.g. Palm) are not
    // elf-like, and so don't need all the rest of this function to
    // generate the data, link files, block move code etc etc
    // Generate the make file
    of.link(false, 0);
    // Generate the .bin files
    ((PalmBinaryFile*)prog.pBF)->GenerateBinFiles(progOptions.outDir);
        
    // BTL 3/5/01: this assumption is wrong if we are creating JVM class
    // files from source Palm executables, so do go ahead and run the
    // rest of this function!
    //    return;
#endif

    // There are two strategies for generating the final output file.
    // Most times, we use the "standard" plan, where the target binary has
    // two special data sections (.uqbt.rodata and .uqbt.data) that contain
    // the original r/o and r/w data section(s) respectively.
    // All read-only and all read-write data sections are combined into
    // two files, corresponding to two segments creating entries in the map
    // file. (Actually, when the block move plan is used, there is only one
    // map entry; see below).

    // For target binaries where the source machine has a smaller page size
    // than the target machine, and if the data sizes are unsuitable (50% of
    // larger programs), and if the target OS is Solaris, then this scheme is
    // not possible. In these cases, we need the "block move" plan.
    // Both the r/o and r/w sections are combined into one section (.uqbt.data)
    // which is r/w, space is reserved at the end of the section (to make room
    // for the block move) and a small piece of assembler code is required
    // to perform the block move of the r/w data to it's proper virtual
    // address (the r/o data will already be loaded at the correct address).
    // This code becomes the start address of the target binary, and will
    // jump to the normal _start symbol to begin the target binary after
    // the block move.

    // Note: for compatibility with uqbtj, the two sections are still placed
    // into two .s files, although there is only one entry in the map file
    // (segments have to be in different pages, it seems). Uqbtj gets the
    // addresses and the sizes of the sections via a files called "readData.j"
    // generated below

    bool bBlockMoveReqd = false;
    // For each section in the input file, check if data. If so, create a
    // "source" file for it, or append to an existing file, and add it to
    // the list of source files to be assembled.

    // Open the read-only data file.
    ofstream df;
    df.open((progOptions.outDir + "rodata.s").c_str());
    if (!df) {
        error("could not open data source file rodata.s for writing");
        return;
    }
    // Write the read-only data declaration (.section ...).
    // Note: it may have to be replaced by the read/write declaration later
    df << RODATA_DCL;

    int n = prog.pBF->GetNumSections();
    assert(n != 0);
    int i=0;
    ADDRESS nextData = 0, firstROdata = 0, firstRWdata = 0;
    ADDRESS srcAddr = 0, sizeROdata = 0, sizeRWdata = 0;
    ADDRESS firstCode = NO_ADDRESS; 
    pair<unsigned, unsigned> gi = prog.pBF->GetGlobalPointerInfo();
    ADDRESS agpAddress = gi.first;
    SectionInfo* pSect;
    while (pSect = prog.pBF->GetSectionInfo(i), pSect->bReadOnly) {
        // Don't bother with non data sections or those with zero length.
        if (!pSect->bData || (pSect->uSectionSize == 0)) {
            i++;
            assert (i < n);
            continue;
        }

        if (nextData && (nextData != pSect->uNativeAddr)) {
            int numBytes = (pSect->uNativeAddr - nextData);
            // Leave space for the section we're not interested in
            df << ".=.+" << dec << numBytes << endl;
            // Add that section's size to the total written byte count
            sizeROdata += numBytes;
        }
        if (firstROdata == 0) firstROdata = pSect->uNativeAddr;
        emitData(df, pSect, 0, agpAddress);
        sizeROdata += pSect->uSectionSize;

        nextData = (pSect->uNativeAddr + pSect->uSectionSize + BINARY_ALIGN-1) &
            ~(BINARY_ALIGN-1);
        i++;
    }

    // Skip any non data read/write sections
    while ((i < n) && (pSect = prog.pBF->GetSectionInfo(i), !pSect->bData)) {
        i++;
    }

    // We expect this to be a read/write data section
    assert((i < n) && pSect->bData && !pSect->bReadOnly);

    // Test whether a block move plan is required. We find the page number
    // (upper bits) of the end of the rodata (nextData-1), and compare it with
    // the page number of the start of the rwdata section (uNativeAddr).
    // If these are the same, then the gap between the end of the rodata and
    // the start of the rwdata is not large enough to make them different pages
    // (on the target machine)
    if (((nextData-1) & ~(TGTPAGESIZE-1)) ==
      (pSect->uNativeAddr & ~(TGTPAGESIZE-1))) {
        // Yes, we need the block move plan
        bBlockMoveReqd = true;
        // Overwrite the r/o declaration with a r/w declaration
        df.seekp(0);
        df << RWDATA_DCL;
        // Remember the last read-only data address. This will be where we
        // have to start the block move FROM
        srcAddr = nextData;

    }

    // Close the read-only data file
    df.close();

    // Now process the read/write data sections
    nextData = 0;                   // Separate check for contiguity now
    // Open the read-write data file.
    df.open((progOptions.outDir + "rwdata.s").c_str());
    if (df == NULL) {
        error("could not open data source file rwdata.s for writing");
        return;
    }
    // Write the read-write data declaration (.section ...).
    df << RWDATA_DCL;

    while ((i < n) && (pSect = prog.pBF->GetSectionInfo(i), !pSect->bReadOnly))
    {
        // Don't bother with non data sections or those with zero length.
        if (pSect->bBss) break;         // Non allocated section def ends data
        if (pSect->bCode || (pSect->uSectionSize == 0)) {
            i++;
            continue;
        }

        if (nextData && (nextData != pSect->uNativeAddr)) {
            int numBytes = (pSect->uNativeAddr - nextData);
            // Leave space for the section we're not interested in
            df << ".=.+" << dec << numBytes << endl;
            // Add that section's size to the total written byte count
            sizeRWdata += numBytes;
        }
        if (firstRWdata == 0) firstRWdata = pSect->uNativeAddr;
        emitData(df, pSect, 0, agpAddress);
        sizeRWdata += pSect->uSectionSize;

        nextData = (pSect->uNativeAddr + pSect->uSectionSize + BINARY_ALIGN-1)
            & ~(BINARY_ALIGN-1);
        i++;
    }

    // Copy the code section to the target, if needed
    // At present, do this by making .s files, as per data sections
    if (progOptions.copyCode) {
        // Open the code data file.
        ofstream cf;            // Note: df is still open
        cf.open((progOptions.outDir + "codeseg.s").c_str());
        if (cf == NULL) {
            error("could not open data source file codeseg.s for writing");
            return;
        }
        // Write the code declaration (.section ...).
        cf << ROCODE_DCL;

        
        i = 0;      // n is still the number of sections
        while ((i < n) && (pSect = prog.pBF->GetSectionInfo(i), !pSect->bCode))
            i++;            // Skip to first code section
        while ((i < n) && (pSect = prog.pBF->GetSectionInfo(i), pSect->bCode)) {
            // Don't bother with sections of zero length.
            if (pSect->uSectionSize != 0) {
#if TARGET_GNU_LD == 1
                if (firstCode == NO_ADDRESS) {
                    firstCode = pSect->uNativeAddr;
                    // If the first address is less than a page size and a bit,
                    // start the first address 0x200 late
                    // Note: this is part of a shameful HACK!
                    emitData(cf, pSect, 0x200);
                } else
                    emitData(cf, pSect);
#else
                if (firstCode == NO_ADDRESS) {
                    firstCode = pSect->uNativeAddr;
                    // If (as is likely for hppa front ends) the required
                    // address is less than one target page size, we will need
                    // a .align and possibly a .=.+xxx to get the address of
                    // the first section correct. This is because the first
                    // segment, whether we like it or not, gets the elf header
                    // bytes (often 0x110-0x150 in size).
                    if (firstCode < TGTPAGESIZE) {
                        assert(firstCode >= 0x1000);
                        cf << ".align 0x1000\n";
                        if (((firstCode + 0xFFF) & ~0xFFF) != firstCode)
                            cf << ".=.+0x" << hex <<
                              (((firstCode + 0xFFF) & ~0xFFF) - 0x1000);
                    }
                }
                emitData(cf, pSect);
#endif
            }
            i++;
        }
        cf.close();
        // Add the data source files to the set of files to be processed by make
        of.addSource("codeseg.s");
    }


    // Add the data source files to the set of files to be processed by make
    of.addSource("rodata.s");
    of.addSource("rwdata.s");

    if (bBlockMoveReqd) {
cout << "Backend: will need the block move plan, with move from ";
cout << hex << srcAddr << " to " << firstRWdata << ", size " << dec << sizeRWdata << endl;
        // We need to create a start.s file
        createStart(srcAddr, firstRWdata, sizeRWdata);
        // Add it to the make file
        of.addSource("start.s");
    }

    ofstream sf;
    sf.open((progOptions.outDir + "link.sed").c_str());

#if TARGET_GNU_LD==1
#define DO_LINUX   1
#define TARGETNAME "linux"
#elif TGT==ARM
#define DO_LINUX   1
#define TARGETNAME "arm_linux"
#endif   

#if DO_LINUX==1
    // Lines of this form: s/$dstartro/115c8/
    if (progOptions.copyCode) {
        // However, if the address is less than one target page size, we
        // need to use this trick: start the code segment 0x200 too late,
        // and pray that the target elf header occupies less than 0x200
        // bytes. This is a temporary, big fat HACK!
        if (firstCode < TGTPAGESIZE+0x200)
            sf << "s/$cstart/"   << hex << firstCode+0x200 << "/\n";
        else
            sf << "s/$cstart/"   << hex << firstCode << "/\n";
    } else
        // Delete the line with $cstart
        sf << "/$cstart/ d\n";
    if (firstROdata)
        sf << "s/$dstartro/" << hex << firstROdata << "/\n";
    else
        sf << "/$dstartro/ d\n";
    sf << "s/$dstartrw/" << hex << firstRWdata << "/\n";

#else       // It's assumed that this means Solaris
#define TARGETNAME "solaris"
    // Lines of this form: s/$dstartrw/115c8/
    if (progOptions.copyCode) {
        // However, if the address is less than one target page size, we
        // need to use the dummy segment
        if (firstCode < TGTPAGESIZE)
            // Delete the line defining the seg.code segment, and change
            // the line associated with section .uqbt.code to use segment dummy
            sf << "/$cstart/ {\nd\n}\ns/seg.code:/dummy:/\n";
        else {
            sf << "s/$cstart/"   << hex << firstCode << "/\n";
            if (bBlockMoveReqd) {
                // We need one segment for the whole lot
                sf << "s/seg.code/seg.data/\n";
                // Remove the line referencing rodata
                sf << "/uqbt.rodata/ d\n";
                // Also the line referenging $dstartrw; it's assumed to be
                // contiguous with code and rodata
                sf << "/$dstartrw/ d\n";
            }
        }
    }
    else {
        if (firstROdata) {
            if (bBlockMoveReqd) {
                // Don't need the code segment at all; delete the 3 lines
                sf << "/seg.code/ {\nd\nd\nd\n}\n";
                // Fix the start of the rodata
                sf << "s/$dstartrw/" << hex << firstROdata << "/\n";
            } else {
                // The start of seg.code will be the rodata
                sf << "s/$cstart/" << hex << firstROdata << "/\n";
                // Get rid of the line mentioning .uqbt.code (not necessary;
                // just less confusing)
                sf << "/.uqbt.code/ {\nd\n}\n";
            }
        }
        else
            // Need to get rid of the three lines defining seg.code
            sf << "/seg.code/ {\nd\nd\nd\n}\n";
    }
    // We now more or less assume that the rodata section, if
    // present, is contiguous with the end of the code section
    //if (firstROdata)
    //    sf << "s/$dstartro/" << hex << firstROdata << "/\n";
    //else
    //    // Need to get rid of those lines
    //    sf << "/$dstartro/ d\n/.uqbt.rodata/ d\n";
    sf << "s/$dstartrw/" << hex << firstRWdata << "/\n";
#endif

    sf.close();

    // Run the link.sed script over the linker script file, and pipe the result
    // to <outdir>/<project>.map
    ostrstream sed;
    sed << "sed -f " << progOptions.outDir << "link.sed ";
    sed << prog.getProgPath() << TARGETNAME;
    sed << ".link > " << progOptions.outDir << prog.getNameNoPath() << ".map";
    int ret = system(str(sed));
    if (ret) {
        error("link.sed failed");
        return;
    }

    // Remove link.sed; no longer needed
//  unlink((progOptions.outDir + "link.sed").c_str());

#undef TARGETNAME
#undef DO_LINUX

    // Find the position and size of the bss section
    ADDRESS BSSstartData = 0; int sizeBSSdata = 0;
    pSect = prog.pBF->GetSectionInfoByName(".bss");
    if (pSect) {
        BSSstartData = pSect->uNativeAddr;
        sizeBSSdata = pSect->uSectionSize;
    }

    if (sizeBSSdata) {
        // Add the appropriate entry in rwdata.s
        // Note: sometimes .bss is aligned to 8 bytes, so get a 4 byte gap
        if (nextData && ((BSSstartData - nextData) > 4)) {
            ostrstream ost;
            ost << "read write sections of source binary are not "
                "contiguous before bss at " << hex << pSect->uNativeAddr;
            ost << "; last data ends " << nextData-1;
            warning(str(ost));
        }

        // For now, this seems to be the best option
        // Note: .skip doesn't work for the intel assembler!
        // .=.+num seems to work for sparc, intel Solaris, and Linux
        df << "\n.=.+\t" << dec << ((sizeBSSdata+ BINARY_ALIGN-1)
            & ~(BINARY_ALIGN-1)) << endl;
    }

    df.close();

    // Create the readData.j file if generating JVM code
	if (progOptions.translateToJava == true) 
    	createReadData(prog.getNameNoPath(),
        	firstROdata, sizeROdata,
        	firstRWdata, sizeRWdata,
        	BSSstartData, sizeBSSdata);

    // Generate the make file; this is in outfile.cc
    of.link(bBlockMoveReqd, sizeBSSdata);
}


