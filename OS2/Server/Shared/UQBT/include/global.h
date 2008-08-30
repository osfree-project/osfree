/*
 * Copyright (C) 1999-2001, The University of Queensland
 * Copyright (C) 2000-2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * File:       global.h
 * OVERVIEW:   This file contains the declarations of all the global types,
 *             classes, macros and variables used in UQBT. It also forward
 *             declares all classes used, displaying the class hierarchy.
 *             It also used to include all the header files to simplify
 *             the dependencies of .cc files, but this is disabled for now.
 *
 * Copyright (C) 1999-2001, The University of Queensland, BT group
 * Copyright (C) 2000-2001, Sun Microsystems, Inc
 *============================================================================*/

/*
 * $Revision: 1.65 $
 * 06 May 99 - Doug: initial version
 * 31 May 99 - Doug: added idLocn
 * 13 Jul 99 - Mike: PENT inst of I386, etc
 *  9 Dec 99 - CC: Linux places libelf.h in <libelf/libelf.h>
 * 14 Feb 00 - Mike: Added MC68K source and target
 * 14 Mar 00 - Cristina: added idAGP (abstract global pointer id)
 * 15 Mar 00 - Cristina: added enumerated type AXP (abstract x pointer) 
 * 27 Sep 00 - Mike: Added SRCINTPROMSZ
 * 29 Nov 00 - Mike: Added "the line" to enum INDEX; defined idMachSpec
 * 30 Nov 00 - Mike: idTrunc -> idTruncu, idTruncs
 * 31 Jan 01 - Mike: Removed idDef and idIndex; not used except
 *              for UQDBT on another CVS tree
 * 13 Feb 01 - Mike: Added idCastIntStar
 * 15 Feb 01 - Nathan: Added idExecute for fetch-exec cycle
 * 05 Mar 01 - Simon: Removed eOPTABLE, eCONDTABLE (obsolete now)
 * 24 Mar 01 - Mike: Added X286 #defines
 * 31 Mar 01 - Mike: Moved NO_ADDRESS here (from BinaryFile.h). Its value is now
 *              -1, because many 286 programs have main at the start of the
 *              image, which is an address of zero.
 * 16 Apr 01 - Brian: added hasExt() procedure.
 * 23 Apr 01 - Nathan: Added searchAndReplace (string function)
 * 30 Apr 01 - Nathan: Added BOOLEAN type
 * 02 May 01 - Nathan: Autoconf changes
 * 10 May 01 - Nathan: Added idCTI, idPostVar. Reformat
 * 11 May 01 - Nathan: Elf bits split off to syself.h
 * 18 Jun 01 - Nathan: Added idForceInt, idForceFlt to do a forced
 *              type conversion (NB: No syntax for this as yet)
 * 31 Jul 01 - Brian: Added new HRTL_KIND to replace old HL_KIND.
 *              New class HRTL replaces RTlist. Renamed LRTL to HRTLList,
 *              getLrtls to getHRTLs, and RTL_IT to HRTLList_IT.
 * 13 Aug 01 - Bernard: Added support for type analysis
 * 14 Aug 01 - Mike: "using namespace std;" is for all environments now
 * 28 Nov 01 - Mike: Removed X286 references
 */

#ifndef __GLOBAL_H_
#define __GLOBAL_H_

#include "config.h"

/*==============================================================================
 * System header files
 *============================================================================*/

// STL
#include <algorithm>
#include <functional>
#include <list>
#include <map>
#include <numeric>
#include <utility>
#include <queue>
#include <set>
#include <stack>
#include <vector>
#include <string>
#include <iterator>

// Standard C++
#include <fstream>
#include <iostream>
#include <iomanip>
#include <strstream>

// C
#include <assert.h>
#include <stdarg.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#ifdef UNIX
#include <unistd.h>           /* lseek(), read() */
#include <sys/types.h>        /* lseek(), read() */
#else
#include <io.h>
#endif

using namespace std ;


/*==============================================================================
 * System-wide class declarations
 *============================================================================*/

// Misc classes
class options;
#ifdef WIN32
namespace NameSpaceBitSet {
#endif
    class BitSet;
#ifdef WIN32
}
using namespace NameSpaceBitSet ;
#endif

// Operate on binary files
class ArchiveFile;
class ElfArchiveFile;
class AutoDetectBF;
class BinaryFile;
class ElfBinaryFile;
class ExeBinaryFile;
class SymTab;

// Control flow graph classes
class BasicBlock;
class Cfg;

// Call signature recovery classes
#ifdef WIN32
namespace NameSpaceCSR {
#endif
    class Logue;
    class CalleePrologue;
    class CalleeEpilogue;
    class LogueDict;
    class Parameters;
    class ReturnLocations;
    class CallerStack;
    class LocationMap;
    class LocationFilter;
    class CSRLocationFilter;
    class CSR;
#ifdef WIN32
}
using namespace NameSpaceCSR ;
#endif
class CSRParser;
class CSRScanner;

// Expression classes (representation for expressions and locations)
class SemStr;
class OpTableExpr;
class CondTableExpr;
class SemItem;
class SemTable;

// Type class
class Type;

// Procedure classes
class Proc;
class UserProc;
class LibProc;

// Program classes
class Prog;

// Register transfer list (RTL) classes
class Register;
class RT;
class RTAssgn;
class RTCond;
class RTFlagDef;
class RTFlagCall;
class HRTL;
class RTL;
class HLJump;
class HLJcond;
class HLCall;
class HLReturn;
class RTLInstDict;
class SSLInst;
class TableEntry;
class Table;
class OpTable;
class ExprTable;
class SSLParser;
class SSLScanner;

// Front end classes
class FrontEnd;
class FrontEndSrc;

// Backend classes
class Translate;
class OutputFile;
class Overlap;

// Type Analysis Classes
class typeAnalysis;
class typeLex;
class analysisBlocks;

/*==============================================================================
 * Global data type declarations
 *============================================================================*/

// Machine types
typedef unsigned char       Byte;      /* 8 bits */
typedef unsigned short      SWord;     /* 16 bits */
typedef unsigned int        DWord;     /* 32 bits */
typedef unsigned int        dword;     /* 32 bits */
#ifdef UNIX
typedef unsigned int        Word;      /* 32 bits */
typedef unsigned int        ADDRESS;   /* 32-bit unsigned */
#elif defined MSDOS
typedef unsigned short      Word;      /* 16 bits */
typedef unsigned long       ADDRESS;   /* 32-bit unsigned */ 
#elif defined WIN32
typedef unsigned int        Word;      /* 32 bits */
typedef unsigned int        ADDRESS;   /* 32-bit unsigned */
#else
#error ADDRESS type needs to be defined.
#endif

// Kinds of semantic strings
enum EKIND {
    eORDINARY = 0            // This whole thing can be removed now
};

// The types of items in the semantic table
enum CAT
{
    cOPERATOR = 1000,        // Operators
    cOPERATION,              // Other operation
    cCONSTANT,               // Constant
    cFUNCTION,               // Function (needs close paren)
    cSPECIAL                 // Special (processor specific)
};

// The index (and integer representation) of predefined semantic items
// (expression types)
enum INDEX
{
    // Operators
    WILD = -1,
    idPlus,                  // Binary addition
    idMinus,                 // Binary subtraction
    idMult,                  // Multiplication
    idDiv,                   // Integer division
    idFPlus,                 // Binary addition(single floats)
    idFMinus,                // Binary subtraction(single floats)
    idFMult,                 // Multiplication(single floats)
    idFDiv,                  // (single floats)
    idFPlusd,                // addition(double floats)
    idFMinusd,               // subtraction(double floats)
    idFMultd,                // Multiplication(double floats)
    idFDivd,                 // Integer division(double floats)
    idFPlusq,                // addition(quad floats)
    idFMinusq,               // subtraction(quad floats)
    idFMultq,                // Multiplication(quad floats)
    idFDivq,                 // division(quad floats)
    idFMultsd,               // Multiplication(single floats--> double floats)
    idFMultdq,               // Multiplication(single floats--> double floats)
    idSQRTs,                 // sqrt of a single
    idSQRTd,                 // sqrt of a double
    idSQRTq,                 // sqrt of a quad

    idMults,                 // Multiply signed
    idDivs,                  // Divide signed
    idMod,                   // Remainder of integer division
    idMods,                  // Remainder of signed integer division
    idNeg,                   // Unary minus

    idAnd,                   // Logical and
    idOr,                    // Logical or
    idEquals,                // Equality (logical)
    idNotEqual,              // Logical !=
    idLess,                  // Logical less than (signed)
    idGtr,                   // Logical greater than (signed)
    idLessEq,                // Logical <= (signed)
    idGtrEq,                 // Logical >= (signed)
    idLessUns,               // Logical less than (unsigned)
    idGtrUns,                // Logical greater than (unsigned)
    idLessEqUns,             // Logical <= (unsigned)
    idGtrEqUns,              // Logical >= (unsigned)

    idNot,                   // Bitwise inversion
    idLNot,                  // Logical not
    idSignExt,               // Sign extend
    idBitAnd,                // Bitwise and
    idBitOr,                 // Bitwise or
    idBitXor,                // Bitwise xor
    idShiftL,                // Left shift
    idShiftR,                // Right shift
    idShiftRA,               // Right shift arithmetic
    idRotateL,               // Rotate left
    idRotateR,               // Rotate right
    idRotateLC,              // Rotate left through carry
    idRotateRC,              // Rotate right through carry

    idTern,                  // Ternary (i.e. ? : )
    idAt,                    // Bit extraction (expr@first:last)

    idMemOf,                 // Represents m[]
    idRegOf,                 // Represents r[]
    idAddrOf,                // Represents a[]
    idVar,                   // Represents l[] (recovered locations)
    idParam,                 // Parameter param`'
    idRparam,                // Parameter rparam`'
    idExpand,                // Expandable expression
    idTemp,                  // Temp register name
    idSize,                  // Size override
    idCastIntStar,           // Cast to int*
    idPostVar,               // Post-instruction variable marker

    idTruncu,                // Integer truncate (unsigned)
    idTruncs,                // Integer truncate (signed)
    idZfill,                 // Integer zero fill
    idSgnEx,                 // Integer sign extend

    idFsize,                 // Floating point size conversion
    idItof,                  // Integer to floating point (and size) conversion
    idFtoi,                  // Floating point to integer (and size) conversion
    idFround,                // Floating point to nearest float conversion
    idForceInt,              // Forcibly change current type to int/flt,
    idForceFlt,              //  without changing any of the bits
    idFpush,                 // Floating point stack push
    idFpop,                  // Floating point stack pop

    idSin,                   // sine
    idCos,                   // cosine
    idTan,                   // tangent
    idArcTan,                // inverse tangent
    idLog2,                  // logarithm to base 2
    idLog10,                 // logarithm to base 10
    idLoge,                  // logarithm to base e
    idSqrt,                  // square root
    idExecute,               // Execute instruction at(addr)

    idIntConst,              // integer constant
    idFltConst,              // floating point constant
    idCodeAddr,              // idIntConst for addresses in code segment

    idTmpNul,                // A temporary used for anulling following instr

    // All machines are assumed to have these following registers:
    idPC,                    // program counter
    // This is the abstract frame pointer register (CSR/PAL analysis). 
    idAFP,                   // abstract frame pointer
    // This is the abstract global pointer register (CSR/PAL analysis)
    idAGP,                   // abstract global pointer


    // Added for type analysis
    idHLCTI,		     // High level Control transfer instruction
    idDEFINE,		     // Define Type of use with lexer

    //---------------------- "The line" --------------------------//
    // All id's greater or equal to idMachSpec are assumed to be source machine
    // specific. If any of these are left by the time the back end is called,
    // then these need to have local variables assigned for them
    idZF,                    // zero flag
    idCF,                    // carry flag
    idNF,                    // negative flag
    idOF,                    // overflow flag
    idFZF,                   // floating point zero flag
    idFLF,                   // floating point less flag
    idFGF,                   // floating point greater flag
    idCTI,                   // Control transfer instruction (boolean)
    idNEXT,                  // Next PC pseudo-register
    
    // ALWAYS LAST!
    idNumOf                  // Special index: MUST BE LAST!
};

#define idMachSpec idZF      // See "The Line" above

// Kinds of flag macros
enum FTYPE {
    fDEFINE,                 // macro defines some flags
    fUNDEFINE                // macro undefines some flags
};

// Kinds of actions represented by flag macros
enum FLAGOP_TYPE {
    DEFINE,
    UNDEFINE
};

// Kinds of abstract pointer abstractions
enum AXP {
    AFP,                     // abstract frame pointer (ptr to the stack frame)
    AGP                      // abstract global pointer (ptr to global data) 
}; 

// Control flow types
enum INSTTYPE {
    I_UNCOND,                // unconditional branch
    I_COND,                  // conditional branch
    I_N_COND,                // case branch
    I_CALL,                  // procedure call
    I_RET,                   // return
    I_COMPJUMP,              // computed jump
    I_COMPCALL               // computed call
};

// Kinds of SSL instructions
enum INST_TYPE {
    SINGLE,                  // single machine instruction definition
    TABLE                    // multiple machine instructions definitions via a table
};

// Kinds of register transfers (RTs)
enum RT_KIND {
    RTASSGN,
    RTFLAGDEF,
    RTFLAGCALL
    // to be hidden from RTL interface
};

// Kinds of HRTLs, or high-level register transfer lists. A HRTL is either:
//    1) a higher-level RTL (instance of class HLJump, HLCall, etc.) that
//       represents information about a control transfer instruction (CTI)
//       in the source program, or
//    2) a low-level RTL (instance of class RTL) that is the result of
//       decoding a non-CTI source machine instruction.
enum HRTL_KIND {
    // higher-level RTLs
    CALL_HRTL,
    RET_HRTL,
    JCOND_HRTL,
    JUMP_HRTL,
    SCOND_HRTL,                 // For "setCC" instructions that set destination
                                // to 1 or 0 depending on the condition codes.
    NWAYJUMP_HRTL,              // Used to represent switch statements.
    // lower-level RTLs
    LOW_LEVEL_HRTL              // A low-level RTL, instance of class RTL.
};

// Kinds of basic block nodes
enum BBTYPE {
    ONEWAY,                  // unconditional branch
    TWOWAY,                  // conditional branch
    NWAY,                    // case branch
    CALL,                    // procedure call
    RET,                     // return
    FALL,                    // fall-through node
    COMPJUMP,                // computed jump
    COMPCALL,                // computed call
    INVALID                  // invalid instruction
};

// These values can be combined to indicate what kind of comparison is being
// performed. For example, HLCMP_INTCC | HLCMP_FLTCMP indicates an X86-like
// floating point comparison resulting in the setting of the integer condition
// codes.
#define HLCMP_INTCC  0
#define HLCMP_INTCMP 0
#define HLCMP_FLTCC  1
#define HLCMP_FLTCMP 2

// These values can be combined to indicate what kind of conditional jump is
// being performed.
enum JCOND_TYPE {
    HLJCOND_JE = 0,          // Jump if equals
    HLJCOND_JNE,             // Jump if not equals
    HLJCOND_JSL,             // Jump if signed less
    HLJCOND_JSLE,            // Jump if signed less or equal
    HLJCOND_JSGE,            // Jump if signed greater or equal
    HLJCOND_JSG,             // Jump if signed greater
    HLJCOND_JUL,             // Jump if unsigned less
    HLJCOND_JULE,            // Jump if unsigned less or equal
    HLJCOND_JUGE,            // Jump if unsigned greater or equal
    HLJCOND_JUG,             // Jump if unsigned greater
    HLJCOND_JMI,             // Jump if result is minus
    HLJCOND_JPOS,            // Jump if result is positive
    HLJCOND_JOF,             // Jump if overflow
    HLJCOND_JNOF,            // Jump if no overflow
    HLJCOND_JPAR             // Jump if parity even (Intel only)
};

// Kinds of SSL specification tables
enum TABLE_TYPE {
    NAMETABLE,
    OPTABLE,
    EXPRTABLE
};

// These are the types that we can recover and detect for locations in RTLs. The
// recovery may result either from using information in header files or through
// analysis
enum LOC_TYPE {
    TVOID = 0,               // void (for return type only)
    INTEGER,                 // integer (any size and signedness)
    FLOATP,                  // a floating point (any size), windows headers have FLOAT
    DATA_ADDRESS,            // a pointer to some data (e.g. char*, struct*,
                             // float* etc)
    FUNC_ADDRESS,            // a pointer to a function
    VARARGS,                 // variable arguments from here on, i.e. "..."
    BOOLEAN,                 // a true/false value
    UNKNOWN
};

// This is a map from LOC_TYPE to a string (defined in proc.cc)
extern char* LOC_TYPE_NAMES[];

// This is a map from LOC_TYPE to the size in bytes of each type (defined in
// proc.cc)
extern int LOC_TYPE_SIZE[];

/*==============================================================================
 * Abbreviations for template instantiations
 *============================================================================*/

typedef BitSet                              BITSET;
typedef BasicBlock*                         PBB;
typedef map<string, int, less<string> >     StrIntMap;
typedef less<string>                        StrCmp;
typedef list<PBB>::iterator                 BB_IT;
typedef list<PBB>::const_iterator           BB_CIT;
typedef map<ADDRESS, PBB, less<ADDRESS> >   MAPBB;
typedef Cfg*                                PCFG;
typedef set < ADDRESS>                      SETPROC;

#ifdef WIN32
typedef list<Proc*>::iterator               PROC_IT;
#else
/* this looks like an error to me - Trent */
typedef list<Proc>::iterator                PROC_IT;
#endif

typedef map<ADDRESS, Proc*, less<ADDRESS> > PROGMAP;
typedef list<RT*>::iterator                 RT_IT;
typedef list<RT*>::const_iterator           RT_CIT;
typedef list<HRTL*>                         HRTLList;
typedef list<HRTL*>::iterator               HRTLList_IT;
typedef list<HRTL*>::reverse_iterator       HRTLList_RIT;
typedef list<HRTL*>::const_iterator         HRTLList_CIT;
typedef HRTL*                               PRTL;
typedef map<string, int, less<string> >     RMAP; 
typedef list<int>::const_iterator           SSCIT;
typedef list<int>::iterator                 SSIT;
typedef set<HLCall*>                        SET_CALLS;
typedef set<HLCall*>::iterator              SCIT;
typedef queue < ADDRESS >                   TARGETS;
typedef map<ADDRESS,string,less<ADDRESS> >  RelocMap;

/*==============================================================================
 * Local header files
 *============================================================================*/

//#include "SymTab.h"
//#include "ArchiveFile.h"
//#include "ElfArchiveFile.h"
//#include "AutoDetectBF.h"
//#include "BinaryFile.h"
//#include "ElfBinaryFile.h"
//#ifndef NODETAILS
//#include "ElfDetails.h"
//#endif
//#include "ExeBinaryFile.h"
//#include "cfg.h"
//#include "csr.h"
//#include "driver.h"
//#include "ss.h"
//#include "rtl.h"
//#include "expr_hide.h"
//#include "frontend.h"
//#include "miniparser.h"
//#include "osfcn.h"
//#include "proc.h"
//#include "prog.h"
//#include "reg.h"
//#include "rt_hide.h"
//#include "sslinst.h"
//#include "tables.h"

//#if SRC == "SPARC"
//#include "../sparcDir/decoder.h"
//#elif SRC == "PENT"
//#include "../386Dir/decoder.h"
//#else
//#error Must define source processor
//#endif


//#include "../BACKEND/arena.h"
//#include "../BACKEND/vportl.h"
//#include "../BACKEND/rtl-pri.h"
////#include "../BACKEND/vportl-pri.h"

//#define instruction SparcInst
//#include "../BACKEND/asm.h"
//#undef instruction

//#include "../BACKEND/asm-pri.h"
//#include "../BACKEND/asm-file.h"
//#include "../BACKEND/vpoi.h"
//#include "../BACKEND/vpoi-pri.h"
//#include "../BACKEND/oper.h"
//#include "../BACKEND/sparcvpoi.h"
//#include "../BACKEND/targets.h"

// NOTE: These headers can not be included here due to issues of cyclic
// dependency. As such files that depend on them must include them after
// including "global.h". Also, they must explicity include the files they depend
// on.
//#include "sslparser_base.h"
//#include "sslscanner.h"
//#include "sslparser.h"

//#include "csrparser_base.h"
//#include "csrscanner.h"
//#include "csrparser.h"

/*=========================================================================
 * Miscellaneous macros and constants
 *=======================================================================*/
#define SPARC   1
#define PENT    2
#define MC68K   3
#define W32     4
#define HPPA    5

#define LITTLEE 10
#define BIGE    11

#ifdef WIN32
/* default to win32 */
#ifndef SRC
#define SRC W32
#endif
#ifndef TGT
#define TGT W32
#endif
#endif

#if SRC == SPARC
#define SRCNAME      "Sparc"
#define SRCLNAME     "sparc"
#define SRCPAGESIZE  8192
#define SRCENDIAN    BIGE
#define SRCLETTER    's'
#define SRCINSTSIZE  4       // Smallest instruction size
#define SRCINTPROMSZ 32      // Integer promotion size
#elif SRC == PENT
#define SRCNAME      "Intel Pentium"
#define SRCLNAME     "pent"
#define SRCPAGESIZE  4096
#define SRCENDIAN    LITTLEE
#define SRCLETTER    'p'
#define SRCINSTSIZE  1
#define SRCINTPROMSZ 32      // Integer promotion size
#elif SRC == MC68K
#define SRCNAME      "Motorola 68328"
#define SRCLNAME     "mc68k"
#define SRCPAGESIZE  4096    // ? Not really applicable!
#define SRCENDIAN    BIGE
#define SRCLETTER    'm'
#define SRCINSTSIZE  2
#define SRCINTPROMSZ 16      // Integer promotion size
#elif SRC == W32
#define SRCNAME      "Intel Win32"
#define SRCLNAME     "win32"
#define SRCPAGESIZE  4096
#define SRCENDIAN    LITTLEE
#define SRCLETTER    'w'
#define SRCINSTSIZE  1
#define SRCINTPROMSZ 32      // Integer promotion size
#elif SRC == HPPA
#define SRCNAME      "HP PA/Risc"
#define SRCLNAME     "hppa"
#define SRCPAGESIZE  4096
#define SRCENDIAN    BIGE
#define SRCLETTER    'h'
#define SRCINSTSIZE  4
#define SRCINTPROMSZ 32      // Integer promotion size
#else
#error Must define source (SRC)
#endif

#if TGT == SPARC
#define TGTNAME     "Sparc"
#define TGTLNAME    "sparc"
#define TGTPAGESIZE 8192
#define TGTENDIAN   BIGE
#define TGTLETTER   's'
#define TGTINSTSIZE 4
#elif TGT == PENT
#define TGTNAME     "Intel Pentium"
#define TGTLNAME    "pent"
#define TGTPAGESIZE 4096
#define TGTENDIAN   LITTLEE
#define TGTLETTER   'p'
#define TGTINSTSIZE 1
#elif TGT == MC68K
#define TGTNAME     "Motorola 68328"
#define TGTLNAME    "mc68k"
#define TGTPAGESIZE 4096
#define TGTENDIAN   BIGE
#define TGTLETTER   'm'
#define TGTINSTSIZE 2
#elif TGT == W32
#define TGTNAME     "Intel Win32"
#define TGTLNAME    "win32"
#define TGTPAGESIZE 4096
#define TGTENDIAN   LITTLEE
#define TGTLETTER   'w'
#define TGTINSTSIZE 1
#elif TGT == HPPA
#define TGTNAME     "HP PA/Risc"
#define TGTLNAME    "hppa"
#define TGTPAGESIZE 4096
#define TGTENDIAN   BIGE
#define TGTLETTER   'h'
#define TGTINSTSIZE 4
#elif TGT == ARM
#define TGTNAME     "ARM StrongARM"
#define TGTLNAME    "arm"
#define TGTPAGESIZE 4096
#define TGTENDIAN   LITTLEE
#define TGTLETTER   'a'
#define TGTINSTSIZE 4
#else
#error Must define target (TGT)
#endif

#define NO_ADDRESS ((ADDRESS)-1)  // For invalid ADDRESSes

/*=========================================================================
 * Debugging macros
 *=======================================================================*/

#define DEBUG_CSR_PARSER 1

/*=========================================================================
 * Global variables
 *=======================================================================*/

extern SemTable theSemTable;
extern options progOptions;  // Program options
extern Prog prog;            // Processed program

/*=========================================================================
 * Global functions
 *=======================================================================*/
// The main error function; all error reporting should use this function
void error(const string& sMsg);
// The main warning function; all warning reporting should use this function
void warning(const string& sMsg);

// Return true if file name has the given extension; false otherwise.
bool hasExt(const string& s, const char* ext);

// Change extension (given a file name and new extension)
string changeExt(const string& s, const char* e);

// Return the null terminated string of an ostrstream object
char* str(ostrstream& os);

// Define operator+ to add integers to a string
string operator+(const string& s, int i);

// Capitalise the first letter of a string
string initCapital(const string& s);

// Change the extension of a given filename
string changeExt(const string& s, const char *ext);

// String-based search and replace
string searchAndReplace(const string &in, const string &match,
                        const string &rep);

// Convert a C string to upper case (and write to d)
void upperStr(const char* s, char* d); 

// additional stuff for win32
#ifdef WIN32
#define min(x,y) (x<y?x:y)
#endif

#endif // #ifndef __GLOBAL_H_

