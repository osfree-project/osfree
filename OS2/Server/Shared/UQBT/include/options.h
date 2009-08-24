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
 * FILE:        options.h
 * OVERVIEW:    Declares the class used to handle command line options.
 *
 * Copyright (C) 1996-2001, The University of Queensland, BT group
 * Copyright (C) 2000-2001, Sun Microsystems, Inc
 *============================================================================*/

/*
 * $Revision: 1.28 $
 * Cristina Cifuentes
 * 21 Feb 97 - Mike: added nodis to the options struct
 * 26 Feb 97 - Cristina
 *      the Elf32_Dyn definition is in <sys/link.h> as per
 *              linker & libraries guide, Nov 1995.
 * 6 Mar 97 - Mike
 *      Added entries such as strtScnAdr 
 * 18 Mar 97 - Mike
 *      Removed needed char* (replaced by inLibNames StringSet)
 * 21 Mar 98 - MVE
 *      Added options.follow
 * 13 Jan 98 - Cristina
 *      added word and dword type definition (used by 80x86 decoder)
 * 30 Jan 98 - Cristina
 *      merged definitions for TRUE and FALSE.
 * 2 Feb 98 - Cristina
 *      added veryVerbose option in struct options.
 * 25 Feb 98 - Cristina
 *      added BOOL type.
 *      added -d option to generate .dot file from the program's CFGs. 
 * 3 Mar 98 - Cristina
 *      added ADDRESS type (dependent on UNIX define).
 * 11 Mar 98 - Cristina 
 *  replaced BOOL type for bool (defined in C++), similar for TRUE and FALSE.
 * 24 Mar 98 - Cristina
 *  removed program header progHdr and SHdrTbl.  This is obsolete code, 
 *      the Loader object supports these data structures.
 *  added program (Prog) object.
 *  this file was split into two: driver.h and global.h.  This file will
 *      include global variables and data types other than basic ones.
 * 4 Jun 98 - Mike
 *  Removed options.follow
 * 16 Dec 98 - Mike: Added sDotFile
 * 05 Feb 99 - Mike: Implemented -R option (HL "rtl" display)
 * 18 Feb 99 - Mike: Added -i option (interpreter)
 * 23 Mar 99 - Mike: Added proctrace prog option and -q
 * 6  May 99 - Doug: Renamed to options.h
 * 31 May 99 - Mike: added endian option
 * 17 Jun 99 - Mike: removed -u switch; added -o and -m switches
 * 13 Jul 99 - Mike: endian -> noendian
 * 07 Nov 00 - Cristina: Added fields for names for all generated files 
 *                       (i.e. options -d, -D, -g, -G, -r and -R)
 * 09 Nov 00 - Cristina: Removed -g option (call graph w/o library functions)
 * 29 Nov 00 - Mike: Added fastInstr (for -f)
 * 14 Dec 00 - Brian: Added translateToJava option (for -j)
 * 10 Apr 01 - Manel: Added support for Expander back end: useExp, whExp.
 * 15 Apr 01 - Brian: Added translateToVPO option (for -O)
 * 19 Apr 01 - Mike: Removed -a; added -l for library string; untabbed
 * 18 Jun 01 - Brian: Added whVPO to control which VPO backend to use
 * 24 Jul 01 - Brian: Removed whVPO. Now using --with-target={sparc,arm} to
 *              control which VPO backend to use.
 * 13 Aug 01 - Bernard: Added support for type Analysis - added bool typeAnalysis 
                        and string typeFile to option class
*/

#ifndef __OPTIONS_H__
#define __OPTIONS_H__

/*==============================================================================
 * Command line options.
 *============================================================================*/
class options {
public:
    bool    translateToVPO;     /* true if to use VPO backend */
    bool    translateToJava;    /* true if to call Java backend */
    bool    dot;                /* generate dot file for main procedure */
    string  sMainDotFile;       /* name of .main.dot file (-d option) */
    bool    allProcs;           /* generate dot file for ALL procs */
    string  sDotFile;           /* name of .dot file (-D option) */
    bool    verbose;            /* dump ELF information */
    bool    veryVerbose;        /* dump SSL parsing */
    bool    noback;             /* Don't use the VPO back end */
    bool    useExp;             /* Use the expander back end */
    char    whExp;          /* Which instance of the expander should I use? */
    bool    rtl;            /* display Rtls */
    string  sRtlFile;       /* name of .rtl file (-r option) */
    bool    highrtl;        /* display HL Rtls */
    string  sHrtlFile;      /* name of .hrtl file (-R option) */
    bool    trace;          /* display trace of BBs and procs visited */
    bool    start;          /* use sStart instead of main as entry point */
    bool    cover;          /* do a coverage analysis */
    bool    interp;         /* call the interpreter */
    string  sStart;         /* string for the above */
    string  sCfgFile;       /* string for the above */
    bool    proctrace;      /* true if display procedure name and ret */
    bool    noendian;       /* true to NOT generate endianness swap code */
    bool    make;           /* true if make output file at end of translate */
    bool    singleProc;     /* true if only decode single procedure */
    bool    callGraph;      /* generate call graph */
    string  sCGFile;        /* name of .cg.dot file (-G option) */
    bool    highLevelC;     /* true if using structuring code to emit HL C */
    bool    dynamicGlobal1; /* true if to process dynamic globals for only 1 */
    bool    noLibInMap;     /* true if don't want library funcs in addr map */
    bool    fastInstr;      /* true if using fast but not as exact instr map */
    bool    copyCode;       /* true if need to copy code section to target */
    char    bff;            /* a binary file format char, or zero for default */
    string  outDir;         /* directory to put output files */
    string  libString;      /* Dollar separated string of library names */
    bool    typeAnalysis;   /* true if we want to perform type analysis */
    string  typeFile;	    /* String that holds the type analysis output file */

    options();          // Constructor
};

#endif
