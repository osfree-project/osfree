/*
 * Copyright (C) 1996-2001, The University of Queensland
 * Copyright (C) 2000-2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:       driver.cc
 * OVERVIEW:   This file contains the command line driver for the UQBT
 *             binary translator framework.
 *
 * Copyright (C) 1996-2001, The University of Queensland, BT group
 * Copyright (C) 2000-2001, Sun Microsystems, Inc
 *============================================================================*/

/*
 * $Revision: 1.147 $
 * 2 Feb 98 - Cristina
 *  added routines to parse the relevant SSL file (based on -D definition of the
 *      architecture in use) and create the RTL dictionary.  This code came
 *      from Doug's parse_test.cc file.
 *  added veryVerbose option and test to print RTL dictionary.
 * 19 Feb 98 - Cristina
 *  loadElf() now takes a list<HRTL> to return the RTLs of the program.
 * 25 Feb 98 - Cristina
 *  added -d option to generate .dot file from the program's CFGs.
 * 11 Mar 98 - Cristina
 *  replaced BOOL for bool type (C++'s), same for TRUE and FALSE.
 * 18 Mar 98 - Cristina 
 *  removed call to buildCFG() as the graphs are being built while 
 *      decoding machine instructions, as part of followControl().
 *  added procedure list as actual parameter to loadElf().
 * 24 Mar 98 - Cristina
 *  removed program header variable -- obsolete.
 *  added program (Prog) object variable.
 *  replaced driver include to global.h. 
 * 28 Mar 98 - Cristina
 *  decoupled loading of the file and decoding of the machine instructions.
 *  changed loadElf() to return the loader object.
 * 2 Jun 98 - Mike
 *  BinaryFile->BinaryFile
 * 4 Jun 98 - Mike
 *  Removed the -f (follow) option
 * 6 Aug 98 - Mike
 *  Replaced parser() with decode() (requires frontsparc.cc)
 * 20 Oct 09 - Mike: Added -s switch (start at given symbol inst of main)
 * 23 Oct 98 - Mike: Use RTLInstDict::readSSLFile() to read the ssl file
 * 02 Nov 98 - Mike: Removed LoadRTL() (not used now)
 * 10 Dec 98 - Mike: Added -f (CFG file) capability
 * 15 Dec 98 - Mike: Moved decode() here from frontsparc.cc; added decodeProc()
 *      to remove CFG when not needed
 * 19 Jan 99 - Mike: Outputs the number of procedures to the -f file now
 * 05 Feb 99 - Mike: Implemented -R option (HL "rtl" display)
 * 08 Feb 99 - Mike: Moved library processing from processProc() to decodeProc()
 * 18 Feb 99 - Mike: Added -i option (interpreter)
 * 05 Mar 99 - Mike: Write data section to cfg file
 * 09 Mar 99 - Mike: Removed -i option (interpreter) ugh!
 * 23 Mar 99 - Mike: Added proctrace prog option and -q
 * 24 Mar 99 - Mike: Sort the BBs before -R printout
 * 08 Apr 99 - Mike: removed loadElf()
 * 09 Apr 99 - Mike: writeDotFile() takes the entry address now
 * 20 Apr 99 - Mike: Only one entry point now; added coverage code including
 *                  findCoverage()
 * 28 Apr 99 - Doug: Moved sortBBs from wfCFG to decodeProc and renamed it to
 *                   sortByAddress
 * 28 Apr 99 - Mike: Mods for the depth first procedure decoding model
 * 29 Apr 99 - Mike: Moved visit() from frontXXX()
 * 03 May 99 - Mike: changed back to noback (-b has opposite sense now).
 *              Changes for new procedure at a time back end
 * 05 May 99 - Mike: moved several blocks of code from main() to functions
 * 07 May 99 - Mike: implemented -u (untidy: don't delete temp files)
 * 18 May 99 - Mike: moved visit() to frontend.cc
 * 28 May 99 - Mike: (started) source and dest CSR files (not complete)
 * 31 May 99 - Mike: added -e switch (Endianness swaps)
 * 17 Jun 99 - Mike: removed u switch; added o and m switches
 * 27 Jun 99 - Doug: moved str() to util.cc
 * 13 Jul 99 - Mike: -e means NO endianness (when needed) code
 * 16 Sep 99 - Mike: Moved the title line, so see it if just enter "uqbtxx"
 * 29 Sep 99 - Mike: Added -a (source addresses each label) option
 * 18 Nov 99 - Mike: -a means NO addresses now (too useful!)
 * 25 Nov 99 - Mike: Added code to generate call graph
 * 13 Mar 00 - Mike: Moved SSL files to proper machine dependent directories
 * 22 Mar 00 - Mike: Moved RTLDict to Prog object
 * 02 Jun 00 - Mike: Support for Win32 .exe files
 * 16 Jun 00 - Mike: Finally removed the cfg file option
 * 19 Jul 00 - Trent: Call the back end after all analysis now
 * 25 Jul 00 - Mike: Removed writeDataSections, added decodeRange(); use the
 *                  latter to properly do speculative decoding. Also call
 *                  prog.getTextLimits and use the results
 * 17 Aug 00 - Mike: Added -L switch
 * 11 Sep 00 - Mike: Don't call back end for non decoded functions (e.g. -S)
 * 19 Sep 00 - Mike: proc_backend -> addSourceFile; don't translate a proc
 *                  twice
 *  8 Nov 00 - Cristina: Added support for generation of output files by use 
 *                       of the options -d, -D, -g, -G, -r and -R, which
 *                       generate the following files: .one.dot, .dot,
 *                       .nolib.cg.dot, .cg.dot, .rtl and .hrtl
 * 09 Nov 00 - Cristina: Removed -g option
 *                       Added support for storing rtl code in .rtl file
 * 10 Nov 00 - Cristina: Added support for storing hrtl code in .hrtl file
 *                       Separated help options into user-level and advanced
 * 13 Nov 00 - Mike: Reversed sense of -y switch
 * 27 Nov 00 - Mike: Added -f switch (fast but not as exact instr mapping)
 *  6 Dec 00 - Brian: Merged Cristina and Mike's recent changes for big
 *                    cvs.experimentalstuff.com merge.
 * 13 Dec 00 - Mike: Replaced a few changes that were lost in the merge
 * 14 Dec 00 - Brian: Added translateToJava option (for -j) to run the Java
 *                    back end. This is false by default.
 * 10 Jan 01 - Brian: Fixed a bug with "-o" writing the wrong files.
 * 11 Feb 01 - Nathan: Replaced various BinaryFile invocations with magic.
 * 22 Feb 01 - Nathan: Moved changeExt to util.cc
 * 22 Feb 01 - Mike: Moved Loading of the binary file before the parsing of
 *                    the SSL and PAL files (for GLOBALOFFSET)
 * 26 Feb 01 - Mike: Corrected some comments confusing analyse() and analysis()
 *  5 Mar 01 - Brian: Added description of "-j" flag to helpAdvanced() output.
 * 20 Mar 01 - Mike: Two trivial changes
 * 30 Mar 01 - Mike: Small change to name 286 SSL and PAL files
 * 31 Mar 01 - Mike: getFixedDest() returns NO_ADDRESS for non fixed addresses
 *  9 Apr 01 - Brian: main() now calls the JVM backend for procs that are
 *              speculatively decoded.
 * 10 Apr 01 - Manel: Added support for Expander back end.
 * 15 Apr 01 - Brian: Added support for VPO back end.
 * 19 Apr 01 - Mike: Removed -a; added -l for library string
 * 24 Apr 01 - Brian: Disable analysis if using VPO optimizer.
 * 29 Apr 01 - Brian: Made decodeProc() recurse into children of a procedure
 *              if using the VPO backend.
 * 06 May 01 - Mike: Don't speculatively decode functions starting with "__",
 *              or _fini and its children
 * 09 May 01 - Mike: Flush the .hrtl stream
 * 17 Jun 01 - Brian: Added support for ARM VPO back end.
 * 20 Jun 01 - Jens: encapsulated main() funtionality into several functions
 *              in order to call it from the HRTL-Interpreter (booked in 6/Aug)
 * 31 Jul 01 - Brian: New class HRTL replaces RTlist. Renamed LRTL to HRTLList.
 * 01 Aug 01 - Mike: Don't create reference to VPO back end unless TGT == SPARC
 * 05 Aug 01 - Brian: Corrected test for IRTL to VPO backend.
 * 06 Aug 01 - Mike: Simplify conditions in HLJconds (so compare to r[0] works)
 * 13 Aug 01 - Bernard: Added support for type analysis
 * 21 Aug 01 - Mike: slight changes to #if for non sparc, non ARM targets
 * 22 Aug 01 - Cristina: made SparcIRTLToVPOBackend references specific to 
 *               SPARC target machine
 * 30 Aug 01 - Mike: libraryParamPropagation: parameters are list (were vector)
 * 24 Oct 01 - Mike: machine/pent -> machine/pentium
 * 12 Dec 01 - Cristina: commented out includes and code related to VPO backends
*/

/*==============================================================================
 * Dependencies.
 *============================================================================*/
#include <sys/types.h>          // For mkdir()
#include <sys/stat.h>           // For mkdir()
#include "global.h"             // global base types
#include "options.h"
#include "reg.h"
#include "ss.h"
#include "rtl.h"
#include "cfg.h"
#include "proc.h"
#include "prog.h"
#include "csr.h"
#include "frontend.h"
#include "backend.h"            // Backend functions, e.g. translate2c()
#include "ElfBinaryFile.h"
#include "PalmBinaryFile.h"
#include "Win32BinaryFile.h"
#include "HpSomBinaryFile.h"
#include "outfile.h"            // class OutputFile etc
#include "jvm.h"                // JVM bytecode translation
#include "optimise.h"           // Java optimisations
#include "expander.h"           // Code expander

/* The following include files have been commented out as all VPO-related 
 * files have been removed from the UQBT distribution.  If you are wanting
 * to build a VPO backend and have obtained the license and code for VPO, 
 * then you should uncomment this section. 

#if TGT == ARM
#include "ARMVPOBackend.h"
#elif TGT == SPARC
#include "SparcIRTLToVPOBackend.h"
#endif    

  */

/*==============================================================================
 * Name of machine specification files
 *============================================================================*/
#if SRC ==  SPARC
#define SSL_NAME "machine/sparc/sparc.ssl"
#define PAL_SRC_NAME "machine/sparc/sparc.pal"

/* The following define has been commented out as all VPO-related
 * files have been removed from the UQBT distribution.  If you are wanting
 * to build a VPO backend and have obtained the license and code for VPO,
 * then you should uncomment this section.

#define SPARCVPO
 */

#elif SRC == PENT
#define SSL_NAME "machine/pentium/80386.ssl"
#define PAL_SRC_NAME "machine/pentium/386.pal"
#elif SRC == MC68K
#define SSL_NAME "machine/mc68k/mc68k.ssl"
#define PAL_SRC_NAME "machine/mc68k/mc68k.pal"
#elif SRC == W32
#define SSL_NAME "machine/pentium/80386.ssl"
#define PAL_SRC_NAME "machine/pentium/386.pal"
#elif SRC == HPPA
#define SSL_NAME "machine/hppa/hppa.ssl"
#define PAL_SRC_NAME "machine/hppa/hppa.pal"
#else
#error Must define source processor
#endif

/*==============================================================================
 * Forward declarartions.
 *============================================================================*/
void parseCommandLine(int argc, char**& argv, char*& inputName);
FrontEndSrc& decode(NJMCDecoder& decoder);
void dispHlRtls(UserProc* pProc, Cfg* pCfg, ofstream &of);
void error(const string& Msg);
unsigned findCoverage(const char* sSection, ElfBinaryFile* pEBF,
    NJMCDecoder& decoder);
void dispCoverage(const char* inputName, NJMCDecoder& decoder);
void ensureTrailSlash(string& s);
void writeCallGraph (ofstream &ofs, string filename, BinaryFile *pBF);
bool decodeRange(ADDRESS start, ADDRESS finish, FrontEndSrc& fe, bool keep,
    bool spec);

// Jens' splitting of main for interpreter
void driver_HandleArgs(int argc, char* argv[], char* &inputName);
void driver_Load(char* &inputName);
void driver_BeforeDecode(void);
void driver_Decode(void);
void driver_AfterDecode(void);

// Bernard's additional functions for type analysis
void createUseDefineDataStruct(UserProc* pProc, Cfg* pCfg);
void interProcPropagation();
void propagateBetweenBB();
void printTypeInfo();

// The following is implemented in analysis.cc
void analysis(UserProc* pProc);

/*==============================================================================
 * Program globals.
 *============================================================================*/
options progOptions;        // Note: has a constructor below
SemTable theSemTable;       // Note: must be constructed before the Prog object,
                            // which now contains a CSR object
Prog prog;                  // Program to process

/*==============================================================================
 * File globals.
 *============================================================================*/
static FILE* fMainDot;      // .one.dot file handle
static FILE* fDot;          // .dot file handle
static ofstream ofsCGDot;   // .cg.dot handle
static ofstream ofRtl;      // .rtl handle
static ofstream ofHrtl;     // .hrtl handle 
static int iDotOffset = 0;  // Initial dotty file node number
static ofstream ofCfg;      // Stream for writing to cfg file
static OutputFile of;       // Object that takes care of assembly files

/*==============================================================================
 * FUNCTION:      options::options
 * OVERVIEW:      Constructor.
 * PARAMETERS:    <none>
 * RETURNS:       <nothing>
 *============================================================================*/
options::options()
  : translateToVPO(false), translateToJava(false), dot(false), allProcs(false),
    verbose(false), veryVerbose(false), noback(false), useExp(false),
    rtl(false), highrtl(false), trace(false), start(false), cover(false),
    proctrace(true), noendian(false), make(false), singleProc(false),
    callGraph(false), highLevelC(false), dynamicGlobal1(false),
    noLibInMap(false), fastInstr(false), copyCode(false), bff(0), typeAnalysis(false) 
{
}


/*==============================================================================
 * FUNCTION:      help
 * OVERVIEW:      Displays usage message 
 * PARAMETERS:    thisProgram - name of the program
 * RETURNS:       <nothing>
 *============================================================================*/
static void help (char *thisProgram)
{
    printf ("Usage: %s {-<option>} binFileName\n", thisProgram); 
    printf("\t-D: generate .dot file for all procedures\n");
    printf("\t-G: generate call Graph (.cg.dot) including library calls\n");
    printf("\t-h: this Help file\n");
    printf("\t-o dir: put Output files into <dir> (default is ./uqbt%c%c."
        "<binFileName>)\n", SRCLETTER, TGTLETTER);
    printf("\t-q: Quiet (no display of each procedure name)\n");
    printf("\t-r: display RTLs as decoded (.rtl)\n");
    printf("\t-R: display High Level RTLs after decoding (.hrtl)\n");
    printf("\t-T: perform type analysis and output type info to a .type file\n");
    printf ("\t-A: display Advanced options (useful for debugging of the translator)\n"); 
}


/*==============================================================================
 * FUNCTION:      helpAdvanced
 * OVERVIEW:      Displays advanced options.  These options are normally 
 *                useful for development and debugging of the translator. 
 * PARAMETERS:    thisProgram - name of the program
 * RETURNS:       <nothing>
 *============================================================================*/
static void helpAdvanced (char *thisProgram)
{
    printf ("Usage: %s {-<switch>} binFileName\n", thisProgram);
    printf("\t-b: no Backend\n");
    printf("\t-Bx: use Binary file format x (e.g. h=HP PA/Risc SOM format)\n");
    printf("\t-c: print Coverage of text section, no analysis or backend\n");
    printf("\t-C: copy the Code section to the target binary\n");
    printf("\t-d: generate .one.dot file for main or procedure selected with "
      "-S\n");
    printf("\t-e: don't generate Endianness swaps even if required\n");
    printf("\t-Ex: use the expander backend (e.g. c=C j=JVM n=NJMCTK v=VPO)\n");
    printf("\t-f: use Fast but not as exact instruction mapping\n");
    printf("\t-H: emit High level C using structuring algorithms\n");
    printf("\t-j: emit Java bytecodes (JVM classfiles)\n");
    printf("\t-lLibString: use dollar separated list of libraries, e.g. "
      "-lm$dl\n");
    printf("\t-L: no Library functions in runtime address map\n");
    printf("\t-m: Make the output file immediately after translation\n");
    printf("\t-O: use the VPO optimizer backend\n");
    printf("\t-s Symbol: use symbol instead of main as entry point\n");
    printf("\t-S Symbol: as above, but only parse Single procedure\n");
    printf("\t-t: print a Trace of basic blocks and procedures visited\n");
    printf("\t-v: Verbose\n");
    printf("\t-V: Very Verbose: detailed dump of input binary file\n");
    printf("\t-y: Suppress dYnamic global processing, if there is only 1 "
      "entry\n");
}


/*==============================================================================
 * FUNCTION:      wfCFG
 * OVERVIEW:      Given a non-well formed graph, transforms it into a well
 *                formed graph and gives the nodes their depth first orderings.
 *                Also compresses the CFG now (removes BBs that are only jumps)
 * PARAMETERS:    pCfg - the graph to be transformed
 * RETURNS:       <nothing>
 *============================================================================*/
void wfCFG (PCFG pCfg)
{ 
    // Create a wfCFG, sort and number the BBs
    if (!pCfg->wellFormCfg()) {
        error("wellFormCfg returned false"); return;
    }
/*    if (!pCfg->compressCfg()) {
	    error("compressCfg returned false"); return;
    }*/
    if (!pCfg->establishDFTOrder()) {
        error("establishDFTOrder returned false"); 
        return;
    }
}

/*==============================================================================
 * FUNCTION:      getStringOption
 * OVERVIEW:      Get an option from the command line args. Calls help (which
 *                exits the program if the index requested is out of bounds.
 * PARAMETERS:    i - index of command line option required
 *                argc - the total number of command line options
 *                argv - the command line options
 * RETURNS:       the option at the given index
 *============================================================================*/
char* getStringOption(int& i, int argc, char* argv[])
{
    if (++i >= argc)
    {
        help(argv[0]);
        exit(1);
    }
    return argv[i];
}

/*==============================================================================
 * FUNCTION:      main
 * OVERVIEW:      The entry to the program.
 * PARAMETERS:    argc - the total number of command line options
 *                argv - the command line options
 * RETURNS:       exit status
 *============================================================================*/

int main(int argc, char *argv[])  {
    char *inputName = 0;        // Input binary file name 

    driver_HandleArgs(argc, argv, inputName);
    driver_Load(inputName);
    driver_BeforeDecode();

    //  //  //  //  //
    //  D e c o d e //
    //  //  //  //  //

    // Decode the instuctions in this executable, starting at the
    // various entry points. 
    NJMCDecoder decoder(prog.RTLDict, prog.csrSrc);
    FrontEndSrc& fe = decode(decoder);
    
    //  //  //  //  //  //  //  //
    //  A f t e r   D e c o d e //
    //  //  //  //  //  //  //  //

    // Propagate type information
    if (progOptions.typeAnalysis) {
    	// The number of times to loop is currently arbitary
        for (int i = 0; i < 3; i++) {
            // Propagate type information between procedures
	    interProcPropagation();
	    // Re-propagate between BBs
            propagateBetweenBB();
        }
    }

    // Close files as required
    if (progOptions.dot) {
        // write out dot tailer
        fprintf(fMainDot, "}\n");
        fclose(fMainDot);
    }

    if (progOptions.allProcs) {
        fprintf(fDot, "}\n");
        fclose(fDot);
    }

    // Write the call graph
    if (progOptions.callGraph) {
        writeCallGraph (ofsCGDot, progOptions.sCGFile, prog.pBF);
    }

    // Create the Expander instance if using the new expander framework
    Expander *expProc;
    if (progOptions.useExp) {
        expProc = Expander::getExpInstance(progOptions.whExp);
        if (expProc == NULL) {
            ostrstream os;
            os << "Non-existing expander instance <" << progOptions.whExp
               << ">\n";
            error(str(os));
            exit(1);
        }
    }

/* The following include files have been commented out as all VPO-related
 * files have been removed from the UQBT distribution.  If you are wanting
 * to build a VPO backend and have obtained the license and code for VPO,
 * then you should uncomment this section.

    // if using the VPO backend, create the appropriate VPO backend instance
    // BTL: this should be changed to use the Expander scheme...
#if TGT == ARM
    ARMVPOBackend* vpoBackend = NULL;
#elif TGT == SPARC
    SparcIRTLToVPOBackend* vpoBackend = NULL;
#endif    
    if (progOptions.translateToVPO) {
#if TGT == ARM
        vpoBackend = new ARMVPOBackend(prog);
#elif TGT == SPARC
        vpoBackend = new SparcIRTLToVPOBackend(prog);
#endif    
    }

 */

    // Call the backend to generate everything needed
    PROGMAP::const_iterator it;
    Proc* pProc;
    set<Proc*> translated;                  // Set of procs already translated
    if (!progOptions.noback) {
        for (pProc = prog.getFirstProc(it); pProc; pProc = prog.getNextProc(it))
        {
            if (!pProc->isLib() && ((UserProc*)pProc)->isDecoded()) {
                // Machine dependent translation
                if (progOptions.useExp) {
                    // Use expander for procedure
                    expProc->expandFunction((UserProc*)pProc);
                    expProc->generateFile();
                    // Add proc's .o file to the makefile
                    addSourceFile(pProc, of);

/* The following include files have been commented out as all VPO-related
 * files have been removed from the UQBT distribution.  If you are wanting
 * to build a VPO backend and have obtained the license and code for VPO,
 * then you should uncomment this section.

#if TGT==SPARC || TGT==ARM
                } else if (progOptions.translateToVPO) {
                    // Use VPO backend
                    vpoBackend->expandFunction((UserProc*)pProc);
                    addVPOSourceFile((UserProc*)pProc, of);
#endif

 */
                } else {
                    // use low-level C and optional JVM backends
                    Translate t;    // Need new one for each proc
                    t.translate2c((UserProc*)pProc,  /*speculative*/ false);
                    if (progOptions.translateToJava) {
                        // Call the JVM backend
                        translate2j((UserProc*)pProc);
                    }
                    // Add proc's .c file to the makefile
                    addSourceFile(pProc, of);
                }
                // Mark proc as translated
                translated.insert(pProc);
            }
        }
    }

    // Now consider some speculative decodes, unless -s or -S
    // Note: these are done after the ordinary procedures are done, since
    // they are only done if the above find register calls
    if (!progOptions.start && (progOptions.cover || prog.bRegisterCall)) {
        ADDRESS start = prog.pBF->GetAddressByName("_start");
        if (start) {
            // Decode _start, and all its children, to find the startup
            // code. Set keep to false (throw away the results), and spec
            // to true (not really because we want to stop if there is an
            // illegal insruction, but so that we don't decode callees that
            // are outside the text section)
            decodeProc(start, fe, false, true);
        }
        ADDRESS fini = prog.pBF->GetAddressByName("_fini");
        if (fini) {
            // It's also important not to decode _fini, since it may call
            // C runtime functions like __do_global_dtors_aux which are not
            // decoded
            decodeProc(fini, fe, false, true);
        }
        // Hopefully, now we have all the gaps to try and decode
        // We need a copy of the range object, since the decoding will
        // disrupt the ranges in prog
        bool change;
        do {
            change = false;
            Coverage copy(prog.cover);
cout << "\nCoverage: "; copy.print(); cout << endl;
            ADDRESS a1, a2;
            COV_CIT ii;
            if (copy.getFirstGap(a1, a2, ii)) {
                do {
cout << "Spec decode at " << hex << a1 << endl;
                    change |= decodeRange(a1, a2, fe, true, true);
                } while (copy.getNextGap(a1, a2, ii));
            }
        } while (change);
        // We may have code before the start of the first range, or after
        // the last range, where code might remain undecoded.
        if (prog.cover.getStartOfFirst() != prog.limitTextLow)
            decodeRange(prog.limitTextLow, prog.cover.getStartOfFirst(),
                fe, true, true);
        if (prog.cover.getEndOfLast() != prog.limitTextHigh)
            decodeRange(prog.cover.getEndOfLast(), prog.limitTextHigh, 
                fe, true, true);
		
	// Propagate type information some more because of the 
	// newly discovered functions from the spec decode
	if (progOptions.typeAnalysis) {
    	    // The number of times to loop is currently arbitary
            for (int i = 0; i < 3; i++) {
        	// Propagate type information between procedures
		interProcPropagation();
		// Re-propagate between BBs
        	propagateBetweenBB();
            }
	}				
		
        if (!progOptions.noback) {
            for (pProc = prog.getFirstProc(it); pProc;
              pProc = prog.getNextProc(it)) {
                if (!pProc->isLib() && ((UserProc*)pProc)->isDecoded()) {
                    if (translated.count(pProc)) {
                        // Don't translate a second time
                        continue;
                    }
                    // Machine dependent translation
                    if (progOptions.useExp) {
                        // Use expander for procedure
                        expProc->expandFunction((UserProc*)pProc);
                        expProc->generateFile();
                        // Add proc's .o file to the makefile
                        addSourceFile((UserProc*)pProc, of);

/* The following include files have been commented out as all VPO-related
 * files have been removed from the UQBT distribution.  If you are wanting
 * to build a VPO backend and have obtained the license and code for VPO,
 * then you should uncomment this section.

#if TGT==SPARC || TGT==ARM
                    } else if (progOptions.translateToVPO) {
                        // Use VPO backend
                        vpoBackend->expandFunction((UserProc*)pProc);
                        addVPOSourceFile((UserProc*)pProc, of);
#endif

 */
                    } else {
                        // Low level C
                        Translate t;    // Need new one for each proc
                        t.translate2c((UserProc*)pProc,   /*speculative*/ true);
                        if (progOptions.translateToJava) {
                            // Call the JVM backend
                            translate2j((UserProc*)pProc);
                        }
                        // Add proc's .c file to the makefile
                        addSourceFile((UserProc*)pProc, of);
                   }
                }
            }
        }
    }
    
    // Print out fully propagated type data
    if (progOptions.typeAnalysis){
        printTypeInfo();	    
    }

    // Delete expander, if used
    if (progOptions.useExp) {
        delete expProc;
    }

    // (Jens) here is my hook-in for writing an interpretable file
    // ie the source for the HRTL interpreter
    void saveForHRTLI(Prog *prog);
    saveForHRTLI(&prog);    

    // Done with the front end now
    delete &fe;

    // Finish the back end
    if (!progOptions.noback)
        finalOutput(of);

    // Deallocate binary file object.
    prog.pBF->UnLoad();

    if (progOptions.cover) {
        // Write a summary of remaining gaps
        dispCoverage(inputName, decoder);
    }

    return 0;               // Exit main()
}

/*==============================================================================
 * FUNCTION:     libraryReturnPropagation 
 * OVERVIEW:     Reads the return type info from the library signatures
 * PARAMETERS:   CallRegInformation* inCallRegInfo 
 * RETURNS:      none    
 *============================================================================*/
void libraryReturnPropagation(CallRegInformation* inCallRegInfo){
                
    varType paramType;
    map<Byte, UDChain*>::iterator mapIt;
    
    // Should really only be one return value
    for (mapIt = inCallRegInfo->returnReg.begin();
         mapIt != inCallRegInfo->returnReg.end();
         mapIt++){

        paramType = INT_TYPE;
        Type tempType = inCallRegInfo->calleeProc->getReturnType();

        LOC_TYPE typeInfo = tempType.getType();

        // Should really integrate this with libraryParamPropagation
        switch(typeInfo){
            case TVOID:         break; 
            case INTEGER:       paramType = INT_TYPE;
                                break;
            case FLOATP:        paramType = FLOAT_TYPE;
                                break;
            case DATA_ADDRESS:  paramType = POINTER_D;
                                break;
            case FUNC_ADDRESS:  paramType = POINTER_I;
                                break;
            case VARARGS:       return; // Don't know what to do with this yet
            case BOOLEAN:       break;  // Treated as a integer
            case UNKNOWN:       break;  // Assume integer
                        
            default:            break;  // Shouldn't get here
        }
        
        if (paramType > mapIt->second->chainType){
            mapIt->second->chainType = paramType;
        }
    }
}


/*==============================================================================
 * FUNCTION:     libraryParamPropagation 
 * OVERVIEW:     Reads the param type info from the library signatures
 * PARAMETERS:   CallRegInformation* inCallRegInfo 
 * RETURNS:      none    
 *============================================================================*/
void libraryParamPropagation(CallRegInformation* inCallRegInfo){

    const list<SemStr>& typeList = inCallRegInfo->calleeProc->getParams();
    list<SemStr>::const_iterator listIt = typeList.begin(); 

    varType paramType;
    map<Byte, UDChain*>::iterator mapIt;
    for (mapIt = inCallRegInfo->paramReg.begin();
         mapIt != inCallRegInfo->paramReg.end();
         mapIt++){

        paramType = INT_TYPE;

        // This will not work 100% of the time because it assumes all params are
        // are registers, which is not always true. This will be fixed...
        if (listIt == typeList.end()){
            printf("Formal and actual parameters don't \
	            match up for library function %s\n", 
		    inCallRegInfo->calleeProc->getName());
            return;  // parameter values doesn't really match up with
                     // what libraries says.
        }    
        Type tempType = listIt->getType();

        LOC_TYPE typeInfo = tempType.getType();
        switch(typeInfo){
            case TVOID:         break;   // Should never be here
            case INTEGER:       paramType = INT_TYPE;
                    	        break;
            case FLOATP:        paramType = FLOAT_TYPE;
                                break;
            case DATA_ADDRESS:  paramType = POINTER_D;
                                break;
            case FUNC_ADDRESS:  paramType = POINTER_I;
                                break;
            case VARARGS:       return;  // Don't know what to do with this yet
            case BOOLEAN:       break;   // Treated as a integer
            case UNKNOWN:       break;   // Assume integer
            
            default:            break;   // Shouldn't get here
        }
        if (paramType > mapIt->second->chainType){
            mapIt->second->chainType = paramType;
        }
        listIt++;    
    }
}

/*==============================================================================
 * FUNCTION:     paramValuePropagation 
 * OVERVIEW:     propagates param values between procedures
 * PARAMETERS:   CallRegInformation* inCallRegInfo 
 * RETURNS:      none    
 *============================================================================*/
void paramValuePropagation(CallRegInformation* inCallRegInfo){

    BB_CIT tempIt;
    PBB tempCalleeBB = 
      (((UserProc*)(inCallRegInfo->calleeProc))->getCFG())->getFirstBB(tempIt);

    list<int>::iterator paramListIt = 
                 inCallRegInfo->calleeProc->regParams.begin();
    map<Byte, UDChain*>::iterator mapIt;
    
    for (mapIt = inCallRegInfo->paramReg.begin();
         mapIt != inCallRegInfo->paramReg.end();
         mapIt++){

        if (paramListIt == inCallRegInfo->calleeProc->regParams.end()) {
            printf("Actual and formal parameter list size doesn't match\n");
            break;
        }
        // Get the next register based parameter
        int calleeParam = 0;
        do {
            calleeParam = *paramListIt++;
        }
        while (calleeParam == -1);

        list<UDChain*>* tempUDChain = 
            tempCalleeBB->usedDefineStruct->returnRegChainListAt(calleeParam);
        if (tempUDChain == NULL){
            // This would be a strange case
            continue;
        }

        UDChain * calleeChain = tempUDChain->front();
        if (calleeChain == NULL) {
            // This would be a strange case        
            continue;
        }    

        if (calleeChain->chainType > mapIt->second->chainType){
            mapIt->second->chainType = calleeChain->chainType;
        }
        else {
            calleeChain->chainType = mapIt->second->chainType;
        }    
    }
}

/*==============================================================================
 * FUNCTION:     returnValuePropagation
 * OVERVIEW:     propagates return values between procedures
 * PARAMETERS:   CallRegInformation* inCallRegInfo 
 * RETURNS:      none    
 *============================================================================*/
void returnValuePropagation(CallRegInformation* inCallRegInfo){

    // For each register in returnReg, do a returnRegChainListAt
    // on the list of BBBlocks in each proc. Then take only the LAST
    // chain from the list of chains.
    //
    // Propagate between the types in the chain. 

    map<Byte, UDChain*>::iterator mapIt;

    for (mapIt = inCallRegInfo->returnReg.begin();
         mapIt != inCallRegInfo->returnReg.end();
         mapIt++){
                    
          assert (inCallRegInfo->calleeProc != NULL);
                
         list<BBBlock*>::iterator listIt;
                
         // Do this three times to attempt to propagate completely    
         for (int i = 0; i < 3; i++) {
             for (listIt = 
	            inCallRegInfo->calleeProc->basicBlocksEndingWithRet.begin();
                  listIt != 
		    inCallRegInfo->calleeProc->basicBlocksEndingWithRet.end();
                 ++listIt){
                    
                list<UDChain*>* tempUDChain = 
                        (*listIt)->returnRegChainListAt(mapIt->first);			
                if (tempUDChain == NULL){
                        // This would be a strange case
                    continue;
                }

                UDChain * calleeChain = tempUDChain->back();
                if (calleeChain == NULL) {
                    // This would be a strange case        
                    continue;
                }    
                if (calleeChain->chainType > mapIt->second->chainType){
                    mapIt->second->chainType = calleeChain->chainType;
                }
                else {
                    calleeChain->chainType = mapIt->second->chainType;
                }    
                            
            }
        }

    }
}


/*==============================================================================
 * FUNCTION:      interProcPropagation
 * OVERVIEW:      Propagate type information across procedures
 * PARAMETERS:    none
 * RETURNS:       none
 *============================================================================*/
void interProcPropagation() {

    PROGMAP::const_iterator progIt;
    Proc * procTraverse;

    // Iterate through all the Procs in prog
    for (procTraverse = prog.getFirstProc(progIt); 
         procTraverse; 
         procTraverse = prog.getNextProc(progIt)){
        
        if (procTraverse->isLib()){
            // Skip libraries
            continue;
        }	
	// Call getCFG to get the CFG and iterate through the
	// CFG for each Proc to find all the call sites
        Cfg* procCfg = ((UserProc*)procTraverse)->getCFG();

        BB_CIT bbIt;
        PBB pBB = procCfg->getFirstBB(bbIt);
        
        while (pBB) {

            CallRegInformation* tempCallRegInfo = NULL;
            if ((tempCallRegInfo = 
                 pBB->usedDefineStruct->returnCallRegInfo()) != NULL){
		 		               
                if (tempCallRegInfo->calleeProc == NULL){
                    pBB = procCfg->getNextBB(bbIt);
                    continue;
                }
                        
                if (tempCallRegInfo->calleeProc->isLib()){
                    libraryParamPropagation(tempCallRegInfo);
                    libraryReturnPropagation(tempCallRegInfo);
                    pBB = procCfg->getNextBB(bbIt);
                    continue;

                }
                paramValuePropagation(tempCallRegInfo);
                returnValuePropagation(tempCallRegInfo);
            }
            pBB = procCfg->getNextBB(bbIt);
        }
    }
}

/*==============================================================================
 * FUNCTION:     printTypeInfo
 * Overview:     outputs the type info in every basic block
 * Parameters:   none 
 * Return:       none
 *============================================================================*/
void printTypeInfo(){

    PROGMAP::const_iterator progIt;
    Proc * procTraverse;

    FILE * typeInfoFile;
    typeInfoFile = fopen(progOptions.typeFile.c_str(), "w");

    for (procTraverse = prog.getFirstProc(progIt); 
         procTraverse; 
         procTraverse = prog.getNextProc(progIt)){
        
        if (procTraverse->isLib()){
            // Skip libraries
            continue;
        }

        fprintf(typeInfoFile, "Currently In Procedure: %s\n", 
	        procTraverse->getName());
        Cfg* procCfg = ((UserProc*)procTraverse)->getCFG();

        BB_CIT bbIt;
        PBB pBB = procCfg->getFirstBB(bbIt);
        
        while (pBB) {
            fprintf(typeInfoFile, "%s\n", 
	            pBB->usedDefineStruct->outputToString());

            CallRegInformation* tempPrintCallRegInfo = NULL;
            if ((tempPrintCallRegInfo = 
                pBB->usedDefineStruct->returnCallRegInfo()) != NULL){
                fprintf(typeInfoFile, "%s\n\n", 
		        tempPrintCallRegInfo->outputToString());
            }
            pBB = procCfg->getNextBB(bbIt);
        }
    }
    fclose(typeInfoFile);
}


/*==============================================================================
 * FUNCTION:      createUseDefineDataStructure
 * OVERVIEW:      Iterates through each BB in a procedure and create the 
 *                Used/Define data structure for each one
 * PARAMETERS:    pProc - the procedure to be displayed
 *                pCfg - its CFG
 * RETURNS:       <nothing>
 *============================================================================*/
void createUseDefineDataStruct(UserProc* pProc, Cfg* pCfg)
{
    BB_CIT it;
    PBB pBB = pCfg->getFirstBB(it);

    /* First create the BBBlock*/
    if (pBB) {
        pBB->usedDefineStruct = new BBBlock();
        pProc->storeParams(*(pBB->usedDefineStruct));
    }

    while (pBB)
    {
        pBB->storeUseDefineStruct(*(pBB->usedDefineStruct));
        
        if(pBB->usedDefineStruct->returnRetInfo()){
            // if it is a Ret BB, then add it to the list
            // to be stored in the Proc
            pProc->basicBlocksEndingWithRet.push_back(pBB->usedDefineStruct);
        }
        
        pBB = pCfg->getNextBB(it);
        
        // Since it's a new BB, must create BBBlock for it again
        if (pBB) {
            pBB->usedDefineStruct = new BBBlock();
        }
    }

    // Propagate three times between basic blocks (hopefully that's enough)
    for (int i = 0; i < 3; i++){
        // Clear the traverse flags
        pCfg->unTraverse();

        // Perform interBB type propagation
        pBB = pCfg->getFirstBB(it);
        pBB->propagateType(NULL);    
    }
    // Clear them again
    pCfg->unTraverse();
}


/*==============================================================================
 * FUNCTION       propagateBetweenBB
 * OVERVIEW       Tells every procedure to propagate type information between
 *                its basic blocks
 * PARAMETERS     none
 * RETURNS        none
 *============================================================================*/
void propagateBetweenBB(){

    PROGMAP::const_iterator progIt;
    Proc * procTraverse;

    for (procTraverse = prog.getFirstProc(progIt); 
         procTraverse; 
         procTraverse = prog.getNextProc(progIt)){
        
        if (procTraverse->isLib()){
            // Skip libraries
            continue;
        }
    
        Cfg* procCfg = ((UserProc*)procTraverse)->getCFG();
        
	// Propagate 3 times, hopefully enough
        for (int i = 0; i < 3; i++) {
            procCfg->unTraverse();
            BB_CIT bbIt;
            PBB pBB = procCfg->getFirstBB(bbIt);
            if (!pBB)
                break;
            pBB->propagateType(NULL);
        }
        procCfg->unTraverse();
    }
}



/*==============================================================================
 * FUNCTION:      driver_InitArgs
 * OVERVIEW:      
 * PARAMETERS:    
 * RETURNS:       <nothing>
 * REMARK:        Jens' splitting of main for interpreter
 *============================================================================*/

void driver_HandleArgs(int argc, char* argv[], char* &inputName) {

  // Pass argv[0] to the global Prog object. Helps to locate files like
  // common.hs
  prog.setArgv0(argv[0]);

  // Parse the command line, and set the name of the input binary file
  parseCommandLine(argc, argv, inputName);

  // Read the file containing signature information for system functions
  // (common.hs)
  prog.readLibParams();
  prog.setName (inputName);
}


/*==============================================================================
 * FUNCTION:      driver_Load
 * OVERVIEW:      
 * PARAMETERS:    
 * RETURNS:       <nothing>
 * REMARK:        Jens' splitting of main for interpreter
 *============================================================================*/

void driver_Load(char* &inputName) {

  //  //  //  //
  //  L o a d //
  //  //  //  //
  
  // Decide the binary file format. Normally, we use a loader appropriate
  // to the source machine, but this can be overridden by the -Bx switch
#if 0
  switch (progOptions.bff) {
  case 'h':   // HP PA/Risc/SOM
    prog.pBF = new HpSomBinaryFile; break;
  case 'e':   // Elf32
    prog.pBF = new ElfBinaryFile; break;
  case 'p':   // Palm .prc format
    prog.pBF = new PalmBinaryFile; break;
  case 'w':   // Windows PE format
    prog.pBF = new Win32BinaryFile; break;
  default:
#if SRC==MC68K
    // Assume it's a Palm binary file
    prog.pBF = new PalmBinaryFile;
#elif SRC==WIN32
    // Assume it's a Windows .exe file
    prog.pBF = new Win32BinaryFile;
#elif SRC==HPPA
    // Assume it's a PA/RISC SOM file
    prog.pBF = new HpSomBinaryFile;
#else
    // Assume it's an elf file
    prog.pBF = new ElfBinaryFile;
#endif
  }
#endif
  
  // Load binary file
  prog.pBF = BinaryFile::Load(inputName);
  if( prog.pBF == NULL ){
    exit (-1);
  }
  
  // Set the text limits
  prog.getTextLimits();
  
  // Dump the header, if veryVerbose (-V)
  if (progOptions.veryVerbose) {
    prog.pBF->DisplayDetails(inputName);
  }
  
  // load RTL dictionary by parsing SSL file. Include the program path using
  // getProgPath()
  if (!(prog.RTLDict.readSSLFile(prog.getProgPath() + SSL_NAME, false)))
    // No point continuing
    exit(1);
  
  // Initialise the CSR analyser by parsing in the CSR files.
  if (!(prog.csrSrc.readCSRFile(PAL_SRC_NAME
#ifdef DEBUG_CSRPARSER
				,true
#endif
				)))
    exit(1);
}

/*==============================================================================
 * FUNCTION:      driver_BeforeDecode
 * OVERVIEW:      
 * PARAMETERS:    
 * RETURNS:       <nothing>
 * REMARK:        Jens' splitting of main for interpreter
 *============================================================================*/

void driver_BeforeDecode(void) {

  //  //  //  //  //  //  //  //  //
  //  B e f o r e   D e c o d e   //
  //  //  //  //  //  //  //  //  //
  
  // Create the output directory, if needed
  if (!progOptions.noback) {
    mkdir(progOptions.outDir.c_str(), 0777);
  }
  
  // Open relevant output files (dot files and ascii output)
  if (progOptions.dot) {
    fMainDot = fopen(progOptions.sMainDotFile.c_str(), "w");
    if (fMainDot == NULL) {
      ostrstream os;
      os << "Could not open dot file <" << progOptions.sMainDotFile;
      os << "> for writing\n";
      error(str(os));
      exit(1);
    } else {
      fprintf(fMainDot, "digraph todot_output {\n");
    }
  }
  
  if (progOptions.allProcs) {
    fDot = fopen(progOptions.sDotFile.c_str(), "w");
    if (fDot == NULL) {
      ostrstream os;
      os << "Could not open dot file <" << progOptions.sDotFile;
      os << "> for writing\n";
      error(str(os));
      exit(1);
    } else {
      fprintf(fDot, "digraph todot_output {\n");
    }
  }
  
  if (progOptions.rtl) {
    ofRtl.open(progOptions.sRtlFile.c_str());
    if (!ofRtl) {
      ostrstream os;
      os << "Could not open ascii file <" << progOptions.sRtlFile;
      os << "> for writing\n";
      error(str(os));
      exit(1);
    }
  }
  
  if (progOptions.highrtl) {
    ofHrtl.open(progOptions.sHrtlFile.c_str());
    if (!ofHrtl) {
      ostrstream os;
      os << "Could not open ascii file <" << progOptions.sHrtlFile;
      os << "> for writing\n";
      error(str(os));
      exit(1);
    }
  }
}

/*==============================================================================
 * FUNCTION:      driver_Decode
 * OVERVIEW:      
 * PARAMETERS:    
 * RETURNS:       <nothing>
 * REMARK:        Jens' splitting of main for interpreter
 *============================================================================*/

void driver_Decode(void) {
  ;
}

/*==============================================================================
 * FUNCTION:      driver_AfterDecode
 * OVERVIEW:      
 * PARAMETERS:    
 * RETURNS:       <nothing>
 * REMARK:        Jens' splitting of main for interpreter
 *============================================================================*/

void driver_AfterDecode(void) {
  ;
}

/*==============================================================================
 * FUNCTION:      saveForHRTLI
 * OVERVIEW:      Writes a file for the HRTL interpreter with all the
 *                neccessary information in it
 * PARAMETERS:    prog - the program to process
 * RETURNS:       <nothing>
 * REMARK:        Jens' splitting of main for interpreter
 *============================================================================*/

void saveForHRTLI(Prog *prog) {
  // HRTLSaver *s = new HRTLSaver();
  // if (s->init()) {
  //   s->save(prog);
  // }
  // delete s;
}

/*==============================================================================
 * FUNCTION:      decodeRange
 * OVERVIEW:      Attempt to decode instructions in the given range
 * PARAMETERS:    start: beginning of the range (inclusive)
 *                finish: end of the range (exclusive)
 *                fe: reference to the FrontEndSrc object
 *                keep: true if want to keep the code
 *                spec: true if speculatively decoding
 * RETURNS:       true if anything useful decoded
 *============================================================================*/
bool decodeRange(ADDRESS start, ADDRESS finish, FrontEndSrc& fe, bool keep,
    bool spec)
{
    bool change = false, changed;
    do {
        // If there is something already decoded here, skip to the end of it
        start = prog.cover.findEndRange(start);
        if (start >= finish) break;
        changed = decodeProc(start, fe, keep, spec);
        change |= changed;
        if (!changed)
            // We failed at this address. Just try the next address!
            start += SRCINSTSIZE;
    } while (start < finish);
    return change;
}

// Main front-end call graph is as follows:
//         main
//          |
//        decode        // Finds entry procedure: driver.cc
//          |
// +---->decodeProc<-+* // Machine independent: driver.cc
// |      __|________|
// |     |        |
// | processProc  |     // Machine dependent: frontxxx.cc and frontend.cc
// |           analyse  // Machine independent: csr.cc
// |______________|
// The * self loop is when doing coverage (analyse not called in that case)

/*==============================================================================
 * FUNCTION:      decode
 * OVERVIEW:      Drives the parsing of machine instructions following the call
 *                graph of the program, starting at the entry procedure
 * PARAMETERS:    decoder - reference to NJMC decoder object
 * RETURNS:       Reference to the FrontEndSrc object
 *============================================================================*/
FrontEndSrc& decode(NJMCDecoder& decoder)
{
    initFront();            // The frontxxx.cc modules need this call
    initCti();              // The ctixxx.cc module needs this call
    initSwitch();           // The switch.cc module needs this call
    list<SectionInfo*> entries;
    ADDRESS uAddr;          // Current location counter

    if (progOptions.start)
    {
        uAddr = prog.pBF->GetAddressByName(progOptions.sStart.c_str());
        if (uAddr == 0)
        {
            ostrstream ost;
            ost << "symbol " << progOptions.sStart;
            ost << " not known";
            error(str(ost));
            exit(1);
        }
    }
    else {
        uAddr = prog.getMainEntryPoint();
        if (uAddr == NO_ADDRESS)
        {
            ostrstream ost;
            ost << "program has no 'main'. Use -s option";
            error(str(ost));
            exit(1);
        }
    }

    // We use the delta and upper address from the Prog object now.
    // That means we don't need to pass these as parameters to the FrontEndSrc
    // object any more, but it stays in case we find deltas different in
    // different sections
    FrontEndSrc& fe = *new FrontEndSrc(prog.textDelta, prog.limitTextHigh,
        decoder);

    // Decode the main procedure. It will recurse via CSR::analyse()
    decodeProc(uAddr, fe);
   
    return fe; 
}

/*==============================================================================
 * FUNCTION:      decodeProc
 * OVERVIEW:      This decodes a given procedure. It performs the
 *                analysis to recover switch statements, call
 *                parameters and return types etc.
 * PARAMETERS:    uAddr - the Native (source) address of the entry point
 *                fe - reference to the FrontEndSrc object
 *                keep - true if we want to keep the decoded RTLs
 *                spec - true if we are speculatively decoding (i.e. if there
 *                  is an illegal instruction, we just bail out)
 * RETURNS:       New procedure was successfully decoded and analysed
 *============================================================================*/
bool decodeProc(ADDRESS uAddr, FrontEndSrc& fe, bool keep /* = true */,
    bool spec /* = false */)
{
    // If it's a library proc, don't decode it.
    if (prog.pBF->IsDynamicLinkedProc(uAddr)) {
        return false;                        // No further processing
    }

    // Make a Proc entry for this procedure
    UserProc* pProc = (UserProc*)prog.visitProc(uAddr);

    // If this proc has already been decoded and deleted (i.e. it can be
    // reached from _start but not main), then just exit
    if (pProc == 0) return false;

    // If we've already decoded this procedure, then don't do it again.
    // Just exit with false.
    if (pProc->isDecoded()) return false;

    // If the name starts with double underscore, assume that it's a runtime
    // helper proc like __deregister_frameinfo. These contain position
    // independent code, which we don't translate well as yet.
    if (spec && strncmp(pProc->getName(), "__", 2) == 0)
        return false;

    string sSpec;
    if (progOptions.trace)
        cout << "\n\n" << sSpec << "Proc " << pProc->getName() << " at "
            << hex << uAddr << dec << endl;
    if (progOptions.proctrace) {
        if (spec) sSpec = "speculatively ";
        if (!keep) sSpec += "(not keeping) ";
        cout << "Processing " << sSpec << pProc->getName() << endl << flush;
    }

    //  //  //  //  //  //  //  //  //  //  //  //  //  //  //  //
    //                                                          //
    //  This is the main front end function for each procedure  //
    //  Implemented in a processor specific file, e.g. front386 //
    //                                                          //
    //  //  //  //  //  //  //  //  //  //  //  //  //  //  //  //

    // CSR step 1: decode proc, building set of call sites
    if (!fe.processProc(uAddr, pProc, ofRtl, spec)) {
        // Encountered illegal instruction. If this is a speculative decode,
        // this has failed
        if (spec) {
            if (progOptions.proctrace)
                cout << "Speculative decode of " << pProc->getName() <<
                    " @ " << hex << pProc->getNativeAddress() << " failed\n";
                prog.remProc(pProc);
            return false;
        }
    }

    // Set the entry BB
    pProc->setEntryBB();

    // Ensure graph is well formed and number the BBs
    Cfg* cfg = pProc->getCFG();
    wfCFG (cfg);

    // Perform simplifications. Some of these are necessary for later, e.g.
    // so that r[8+1] => r[9] and will be changed to v[1] if needed
    BB_CIT it;
    PBB pBB = cfg->getFirstBB(it);
    while (pBB)
    {
        HRTLList* pRtls = pBB->getHRTLs();
        if (pRtls == 0) {
            pBB = cfg->getNextBB(it);
            continue;
        }
        HRTLList_IT rit;
        for (rit = pRtls->begin(); rit != pRtls->end(); rit++) {
            // Perform machine dependent, and standard simplifications.
            // This includes important semantic rules such as r[0] == 0 for
            // sparc, and standard simplifications such as 66 << 10 => 67584,
            // and 0 | 594 => 594
            (*rit)->allSimplify();
            // Even conditions need simplifying, e.g. comparison with r[0]
            if ((*rit)->getKind() == JCOND_HRTL) {
                HLJcond* pJC = (HLJcond*)(*rit);
                SemStr* cond = pJC->getCondExpr();
                if (cond) cond->machineSimplify();
            }
        }
        pBB = cfg->getNextBB(it);
    }

    pProc->setDecoded();

    // Coverage analysis
    if (progOptions.cover)
        pProc->getCoverage();
    
    // Check whether to recurse without doing the rest of the analysis.
    // This is necessary if using the SPARC (not ARM!) VPO backend, doing
    // coverage, or not keeping the results.
    bool usingSparcIRTLBackend = false;

/* The following include files have been commented out as all VPO-related
 * files have been removed from the UQBT distribution.  If you are wanting
 * to build a VPO backend and have obtained the license and code for VPO,
 * then you should uncomment this section.

#if TGT == SPARC
    if (progOptions.translateToVPO) {
        usingSparcIRTLBackend = true;
    }
#endif
  */

    if (usingSparcIRTLBackend || progOptions.cover || !keep) {
        // Recurse into the children of this procedure (usually done in
        // CSR::analyse()). Iterate through the calls in the procedure and
        // decode the corresponding callees if they are user procedures.
        // Exception is if -S used (single procedure only)
        if (!progOptions.singleProc) {
            SET_CALLS& callSites = pProc->getCFG()->getCalls();
            for (SET_CALLS::iterator it=callSites.begin();
                    it!= callSites.end();  it++) {
                HLCall* call = (HLCall*)*it;
                ADDRESS dest = call->getFixedDest();
                if (dest == NO_ADDRESS) continue;
                // Recurse with the address of the callee
                decodeProc(dest, fe, keep, spec);
            }
        }
    }

    // If using the SPARC IRTL->VPO backend, we don't want to generate HRTLs,
    // so return now.
    if (usingSparcIRTLBackend) {
        return true;
    }

    // If not keeping this procedure, delete it now and exit
    if (!keep) {
cout << "Removing proc " << pProc->getName() << endl;
        prog.remProc(pProc);
        return false;
    }

    //  //  //  //  //  //
    //   A n a l y s e  //
    //  //  //  //  //  //

    // Recover the parameters and return type of this procedure as
    // well as the outgoing paramaters and return values for each call
    // within this procedure.
    // Note that analyse will call decodeProc, so there is mutual recursion
    // from decodeProc to processProc to analyse
    prog.csrSrc.analyse(pProc, fe, spec);

    // Sort the BBs by address for the purpose of printing the graph.
    cfg->sortByAddress();

    //  //  //  //  //  //
    //  A n a l y s i s //
    //  //  //  //  //  //

    // Do analysis on this procedure, to remove the last machine
    // dependencies
    analysis(pProc);
    
    // Structure the control flow graph
    if (progOptions.highLevelC)
        cfg->structure();

    // This is where we used to call the Backend for this procedure

    // Write dot file if required. Unless allProcs is set, we only want the
    // first proc processed
    if (progOptions.allProcs || (progOptions.dot && pProc == prog.getProc(0))) {
        // Establish an ordering. This is in case any orphan BBs were added by
        // the analysis phase (those BBs will have an m_first of 0, and will all
        // appear to be the same BB to dotty)
        cfg->establishDFTOrder();
        if (progOptions.allProcs) {
            if (!cfg->writeDotFile(fDot, pProc->getName(), iDotOffset, 
                                   prog.pBF, uAddr)) {
                ostrstream os;
                os << "Could not write .dot file for ";
                os << pProc->getName();
                error(str(os));
            }
        } else {   // progOptions.dot 
            if (!cfg->writeDotFile(fMainDot, pProc->getName(), iDotOffset, 
                                   prog.pBF, uAddr)) {
                ostrstream os;
                os << "Could not write .one.dot file for ";
                os << pProc->getName();
                error(str(os));
            }
        }
        iDotOffset += 1000;
    }

    // Display HL rtls if options set
    if (progOptions.highrtl) {
        dispHlRtls(pProc, cfg, ofHrtl);
        ofHrtl << flush;        // Handy if the translator crashes
    }
       
    // Create the use define chains for this Basic Block
    if (progOptions.typeAnalysis) {
	createUseDefineDataStruct(pProc, cfg);
    }    

    // Remove the CFG and associated RTLs to save memory
    // cout << " *** About to delete CFG for proc " << pProc->getName() << endl;
    // pProc->deleteCFG();
    // cout << " *** After delete CFG for proc " << pProc->getName() << endl;

    if (progOptions.trace)
        cout << "\nFinished proc " << pProc->getName() << endl;
//  if (progOptions.proctrace)
//      cout << "Finished " << pProc->getName() << endl << flush;

    return true;
}

/*==============================================================================
 * FUNCTION:      dispHlRtls
 * OVERVIEW:      Displays the high level RTLs for a procedure on the standard
 *                output.
 * PARAMETERS:    pProc - the procedure to be displayed
 *                pCfg - its CFG
 *                of - an opened output file stream for hrtl code
 * RETURNS:       <nothing>
 *============================================================================*/
void dispHlRtls(UserProc* pProc, Cfg* pCfg, ofstream &of)
{
    of << "\nHigh level RTLs for procedure " << pProc->getName();

    // Display the symbolic names of the recovered parameters
    of << "(";
    pProc->printParams(of);
    of << ")" << endl;

    BB_CIT it;
    PBB pBB = pCfg->getFirstBB(it);
    while (pBB)
    {
        pBB->print(of);
        of << endl;                       // Separate BBs
        pBB = pCfg->getNextBB(it);
    }

    // Display the return type and location (if any)
    if (pProc->getReturnType() == TVOID)
        of << "No return location\n";
    else {
        of << "Return location is " << *pProc->getReturnLoc() << " (type ";
        pProc->printReturnTypeAsC(of);
        of << ")\n";
    }
}

/*==============================================================================
 * FUNCTION:      findCoverage
 * OVERVIEW:      Find the coverage of functions called by code in this section
 *                  Typically used for sections like ".init and .fini, which are
 *                  separate to the .text section (hence, we don't want to con-
 *                  sider the functions themselves for the coverage), but we
 *                  want to find the size of all the functions they call (since
 *                  they aren'y part of the program proper)
 * NOTE:          This ASSUMES ELF BINARYFILE!! So it can't stay like this.
 *                  Perhaps we can do a sort of speculative decode of the
 *                  init and fini sections... but how to do this in a source
 *                  machine independent way?
 * PARAMETERS:    sSection - name of the section (e.g. ".init")
 *                pEBF - pointer to the ElfBinaryFile object associated with
 *                  the loaded image
 *                decoder - reference to NJMC decoder object
 * RETURNS:       Coverage in bytes (unsigned int) (not including the section
 *                  itself)
 *============================================================================*/
unsigned findCoverage(const char* sSection, ElfBinaryFile* pEBF,
    NJMCDecoder& decoder)
{
    SectionInfo* pSect = pEBF->GetSectionInfoByName(sSection);
    // Assume that the entry point to the section is the first byte
    ADDRESS address = pSect->uNativeAddr;
    ADDRESS upper = address + pSect->uSectionSize;
    int delta = pSect->uHostAddr - address;
    unsigned coverage = 0;
    HRTL* pRtl;

    while (address < upper) {
        // Process this instruction. Basically, only want to record the
        // number of bytes, or follow a function if there is one
        pRtl = decodeRtl(address, delta, decoder);
        unsigned bytes = pRtl->getNumBytes();
        // coverage += bytes;
        if (pRtl->getKind() == CALL_HRTL) {
            // This is a call RTL. Guess the size of the called function
            // and add it to the coverage
            HLCall* pCall = (HLCall*)pRtl;
            ADDRESS dest = pCall->getFixedDest();
            if (dest != NO_ADDRESS) {
                const char* pName = pEBF->SymbolByAddress(dest);
                if (pName != NULL) {
                    coverage += pEBF->GetDistanceByName(pName);
                }
            }
        }
        address += bytes;
    }
    return coverage;
}

/*==============================================================================
 * FUNCTION:    dispCoverage
 * OVERVIEW:    Calculate and output total coverage information (i.e. what
 *              percentage of the executable file was decoded).
 * PARAMETERS:  inputName: Name of the input binary file
 *              decoder: reference to NJMC decoder object
 * RETURNS:     <nothing>
 *============================================================================*/
void dispCoverage(const char* inputName, NJMCDecoder& decoder)
{
    // Coverage analysis
    int totGap = 0;
    ADDRESS a1, a2;
    COV_CIT ii;
    if (prog.cover.getFirstGap(a1, a2, ii)) {
        cout << "Gaps remaining in program coverage:\n";
        do {
            int gap = a2 - a1;
            totGap += gap;
            cout << dec << gap << ": " << hex << a1 << "-" << a2 << "\t";
        } while (prog.cover.getNextGap(a1, a2, ii));
        cout << "\nTotal gap: " << dec << totGap << endl;
    }
    else
        cout << "No gap in coverage!\n";

    // We need to re-open the BinaryFile object. Note: it was necessary
    // to close and open it, because the section info had been fiddled to
    // make it look like main was the start of .text.
    // There seem to be problems with the symbol table if the same object
    // is just re-opened. So we create a new one.
    ElfBinaryFile EBF;
    if (EBF.Load(inputName) == 0)
    {
        ostrstream ost;
        ost << "? Could not open file " << inputName;
        ost << " for coverage analysis\n";
        error(str(ost));
        return;
    }
    SectionInfo* pSect = EBF.GetSectionInfoByName(".text");
    unsigned uSections = pSect->uSectionSize;

    // Use GetDistanceByName, since we include any fillers between modules
    //unsigned uTotal = EBF.GetSizeByName("_start");
    unsigned size, uTotal = 0;
    size = EBF.GetDistanceByName("_start");
    cout << "Coverage: _start removes " << dec << size << " bytes\n";
    uSections -= size;
    
    PROGMAP::const_iterator it;
    Proc* proc;
    for (proc = prog.getFirstProc(it); proc; proc = prog.getNextProc(it)) {
        if (proc->isLib()) continue;
cout << "Coverage: " << dec << proc->getCoverage() << " from proc " << proc->getName() << endl;
        uTotal += proc->getCoverage();
    }

    // Don't want to count .init and .fini in either the top line or the bottom
    // line (they are in different sections, after all).
    // However, we should count the calls that they make into the .text section
    // (if any).
    pSect = EBF.GetSectionInfoByName(".init");
    if (pSect) {
        size = findCoverage(".init", &EBF, decoder);
        cout << "Coverage: .init section removes " << dec << size
            << endl;
        uSections -= size;
    }
    pSect = EBF.GetSectionInfoByName(".fini");
    if (pSect) {
        size = findCoverage(".fini", &EBF, decoder);
        cout << "Coverage: .fini section removes " << dec << size
            << endl;
        uSections -= size;
    }

    // Final cludge for gcc: cover the functions init_dummy and fini_dummy.
    // If they don't exist, it won't hurt anything
    uSections -= EBF.GetDistanceByName("init_dummy");
    uSections -= EBF.GetDistanceByName("fini_dummy");
    cout << "Coverage: " << dec << uTotal << " bytes = ";
    cout << 100.0 * uTotal / uSections << "%\n";
    cout << "Uncovered: " << (int) (uSections - uTotal) << " bytes\n";
cout << "That's " << uTotal << " out of " << uSections << " bytes\n";
}


/*==============================================================================
 * FUNCTION:    ensureTrailSlash
 * OVERVIEW:    Ensure that this string (a path) has a trailing slash
 * PARAMETERS:  s: reference to string
 * RETURNS:     <nothing>
 *============================================================================*/
void ensureTrailSlash(string& s)
{
    int n = s.length();
    if (s[n-1] == '/') return;
    s += '/';
}

/*==============================================================================
 * FUNCTION:    parseCommandLine
 * OVERVIEW:    Parse the command line and set the name of the input binary
 *              file
 * PARAMETERS:  argc: count of command line arguments, including interpreter's
 *                name
 *              argv: reference to array of char* representing the command line
 *                args
 *              inputName: set to the name of the input binary file, including
 *                path, if any
 * RETURNS:     <nothing>
 *============================================================================*/
void parseCommandLine(int argc, char**& argv, char*& inputName)
{
    bool bOutputSet = false;                // True if -o seen
    for (int i = 1; i < argc; i++)
    {
        if ((strlen(argv[i]) >= 2) && (argv[i][0] == '-'))
        {
            switch(argv[i][1])
            {
                case 'T':
                    progOptions.typeAnalysis = true; 
                    break;
                case 'A': 
                    helpAdvanced (argv[0]); 
                    exit(1); 
                case 'b':       // no backend
                    progOptions.noback = true;
                    break;
                case 'B':       // Binary file format
                    progOptions.bff = argv[i][2];
                    break;
                case 'c':       // Coverage analysis
                    progOptions.cover = true;
                    // Coverage implies no analysis, which means no backend
                    progOptions.noback = true;
                    break;
                case 'C':       // copy Code section
                    progOptions.copyCode = true;
                    break;
                case 'D':       // dotty file, all procs
                    progOptions.allProcs = true;
                    break;
                case 'd':       // dotty file
                    progOptions.dot = true;
                    // -d implies single procedure (i.e. -S main)
                    progOptions.singleProc = true;
                    break;
                case 'e':
                    progOptions.noendian = true;
                    break;
                case 'E':       // use the Expander backend
                    progOptions.useExp = true;
                    progOptions.whExp = argv[i][2];
                    break;
                case 'f':
                    progOptions.fastInstr = true;
                    break;
                case 'G':
                    progOptions.callGraph = true;
                    break;
                case 'h':       // Help
                default:
                    help (argv[0]);
                    exit(1);
                case 'H':
                    progOptions.highLevelC = true;
                    break;
                case 'j':
                    progOptions.translateToJava = true;
                    break;
                case 'l':
                    progOptions.libString = argv[i];
                    break;
                case 'L':
                    progOptions.noLibInMap = true;
                    break;
                case 'm':
                    progOptions.make = true;
                    break;
                case 'o':
					/* set the output directory from the next argument */
                    progOptions.outDir = getStringOption(i, argc, argv);
                    ensureTrailSlash(progOptions.outDir);
                    bOutputSet = true;
                    break;
                case 'O':       // use a VPO optimizer backend
                    progOptions.translateToVPO = true;
                    break;
                case 'q':       // quiet
                    progOptions.proctrace = false;
                    break;
                case 'r':       // display RTLs
                    progOptions.rtl = true;
                    break;
                case 'R':       // display HL RTLs
                    progOptions.highrtl = true;
                    break;
                case 'S':
                    progOptions.singleProc = true;
                case 's':       // use given symbol rather than main
                    progOptions.start = true;
                    progOptions.sStart = getStringOption(i, argc, argv);
                    break;
                case 't':       // display trace of BBs and procs
                    progOptions.trace = true;
                    break;
                case 'V':
                    progOptions.veryVerbose = true;
                    // Fall through to 'v' case
                case 'v':       // Verbose
                    progOptions.verbose = true;
                    break;
                case 'y':       // Suppress dYnamic globals for only 1 symbol
                    progOptions.dynamicGlobal1 = true;
                    break;
            }
        }
        else inputName = argv[i];   
    }

    if (progOptions.proctrace)
        cout << "The University of Queensland " << SRCNAME << " to "
            << TGTNAME << " Binary Translator\n";

    if (inputName == 0) {
        help (argv[0]);
        exit(1);
    }

    // To hold name of the program w/o including its path
    string infile(inputName);

    if (!bOutputSet) {
        // No -o was issued. Default progOptions.outDir to uqbtST.<inputName>/
        // (except remove the path, if any, from inputName)
        progOptions.outDir = argv[0];
        size_t j = progOptions.outDir.rfind("/");
        if (j != progOptions.outDir.length()) {
            // Keep from just after the last slash to the end; append a dot
            progOptions.outDir = progOptions.outDir.substr(j+1) + ".";

            // Get a string copy of input name, and remove the path if any
            unsigned n = infile.rfind("/");
            if (n != string::npos)
                infile = infile.substr(n+1);

            // Add the input file name, and a trailing slash
            progOptions.outDir += infile + "/";
        }
    } else {
        // Get a string copy of input name, and remove the path if any
        unsigned n = infile.rfind("/");
        if (n != string::npos) {
            infile = infile.substr(n+1);
        }
    }

    // Set the names for output files based on the output path 
    if (progOptions.dot)
        progOptions.sMainDotFile = progOptions.outDir + infile +".one.dot"; 
    if (progOptions.allProcs)
        progOptions.sDotFile = progOptions.outDir + infile + ".dot"; 
    if (progOptions.rtl)
        progOptions.sRtlFile = progOptions.outDir + infile + ".rtl";
    if (progOptions.highrtl)
        progOptions.sHrtlFile = progOptions.outDir + infile + ".hrtl";
    if (progOptions.callGraph)
        progOptions.sCGFile = progOptions.outDir + infile + ".cg.dot";
    if (progOptions.typeAnalysis)
        progOptions.typeFile = progOptions.outDir + infile + ".type";
}

/*==============================================================================
 * FUNCTION:    writeCGproc
 * OVERVIEW:    Append this procedure to the call graph, and all it's children
 * PARAMETERS:  ofCG: output file stream to write to
 *              proc: pointer to the current procedure to write
 *              done: set of pointers to procedures already in the graph
 *              pBF: ptr to BinaryFile object that loaded the source image
 * RETURNS:     <nothing>
 *============================================================================*/
static int node;            // Current node number

void writeCGproc(ofstream& ofCG, const Proc* proc, set<const Proc*>& done,
    BinaryFile* pBF)
{
    if (done.find(proc) != done.end())
        // Already done this proc and its children
        return;
    done.insert(proc);

    // Add a node for this proc
    ofCG << "\t\"" << proc->getName() << "\";\n";

    // Now add each child, unless already in the graph, and an
    // edge from this procedure to its parent
    ADDRESS entry = proc->getNativeAddress();
    // This should not be a library function
    assert (!pBF->IsDynamicLinkedProc(entry));
    UserProc* up = (UserProc*)proc;
    set<const Proc*>::iterator it;
    for (it= up->getCallees().begin(); it!= up->getCallees().end(); it++) {
        ADDRESS dest = (*it)->getNativeAddress();
        if (dest == 0) {
            // Not sure if this ever happens. Ignore for now
            continue;
        }
        if (pBF->IsDynamicLinkedProc(dest)) {
            // Give library functions a different shape, and make them a 
            // unique node (else all "strlen" calls point to the same box)
            ofCG << "\t" << node << " [shape = box, label = \"" <<
                (*it)->getName() << "\"];\n";
            // Add the (unique) edge
            ofCG << "\t\"" << proc->getName() << "\" -> " << node++ << ";\n";
        } else {
            // Recurse to children first
            writeCGproc(ofCG, *it, done, pBF);
            // Add the edge
            ofCG << "\t\"" << proc->getName() << "\" -> \"" <<
                (*it)->getName() << "\";\n";
        }
    }
}

/*==============================================================================
 * FUNCTION:    writeCallGraph
 * OVERVIEW:    Write a dotty file with the call graph for this program
 * PARAMETERS:  ofs: output stream
 *              filename: name of the output file
 *              pBF: pointer to the BinaryFile object with the source image
 * RETURNS:     <nothing>
 *============================================================================*/
void writeCallGraph(ofstream &ofs, string filename, BinaryFile *pBF)
{
    set<const Proc*> done;              // Set of procedures already done
    node = 1;                           // Start with node 1

    ofs.open (filename.c_str());
    if (!ofs) {
        ostrstream os;
        os << "Could not open dot file <" << filename;
        os << "> for writing\n";
        error(str(os));
        exit(1);
    }

    // Header
    ofs << "digraph todot_output {\n";

    // This procedure will recurse for each child of each proc
    writeCGproc (ofs, prog.getProc(0), done, pBF);

    // Trailer
    ofs << "}\n";
    ofs.close();
}
   

