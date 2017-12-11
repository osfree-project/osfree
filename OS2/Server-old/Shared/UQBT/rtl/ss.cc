/*
 * Copyright (C) 1998-2001, The University of Queensland
 * Copyright (C) 1999, David Ung
 * Copyright (C) 2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:       ss.cc
 * OVERVIEW:   Implementation of the SemStr and related classes.
 *
 * Copyright (C) 1998-2001, The University of Queensland, BT group
 * Copyright (C) 1999, David Ung
 * Copyright (C) 2001, Sun Microsystems, Inc
 *============================================================================*/

/*
 * $Revision: 1.107 $
 * 25 Jun 98 - Mike: Instantiate()
 * 12 Jul 98 - Mike: idRparam
 * 17 Jul 98 - Mike: Instantiate returns a bool now
 * 16 Sep 98 - Mike: IsArrayEqual() now handles wildcards; added GetIndex()
 * 06 Oct 98 - Mike: Added SmartCompare() (not complete) and Simplify()
 * 08 Oct 98 - Mike: Added FindEndSubExpr()
 * 09 Oct 98 - Mike: Added IsArraySimilar() and FindSubExpr()
 * 15 Oct 98 - Mike: Added GetLastIdx(); pushSS takes a SemStr* now
 * 15 Oct 98 - Mike:
 *      The various print() functions cope with incomplete strings now
 * 16 Oct 98 - Mike: Rewrote SubstReg() properly
 * 24 Nov 98 - Mike: Added RemoveSize()
 * 09 Dec 98 - Mike: Fixed Simplify of <exp> | 0
 * 10 Dec 98 - Mike: Added WriteSemStr()
 * 14 Dec 98 - Shawn: Added floating operators.
 * 15 Dec 98 - Mike: Fixed Simplify()
 * 21 Jan 99 - Mike: Added SubstSpcl()
 * 27 Jan 99 - Mike: Fixed simplify of (k + 2) + 0
 * 11 Feb 99 - Mike: Added less()
 * 17 Feb 99 - Doug: Added comments
 *                   Merged FindOpIndex and FindRegIndex into FindItem
 *                   Modified SemTable::AddItem
 * 09 Mar 99 - Mike: DoExpand() changed to not depend on specific categories now
 * 09 Mar 99 - Mike: More transcendental functions (e.g. sqrt, arctan)
 * 10 Mar 99 - David: Added GetIndices()
 * 11 Mar 99 - Mike: DoIsRegCont takes two registers (lower and upper) now
 * 17 Mar 99 - Mike: Fixed print of a[] (added right bracket); flt const has dot
 * 24 Mar 99 - Mike: Merged FindOpIndex() and FindRegIndex() into FindItem()
 * 30 Mar 99 - David: Added idIndex(operator) and idDef(constant)
 * 14 Apr 99 - Doug: added SimplifyToInt
 *                   Changed SimplifyToInt to SolveForParam and added
 *                   PartitionTerms
 *                   Added FindSubExpr (new version for exact matches)
 * 22 Apr 99 - Doug: Added generalised search and replace methods * (search, 
 *                   searchReplace, searchReplaceAll, doSearch)
 * 05 Apr 99 - Doug: Added Search that returns a list of found subexpressions
 * 15 Apr 99 - Doug: Replaced all iNumIntArgs + iNumFixedArgs with
 *                   totalFixedArgs()
 * 19 May 99 - Mike: Quelched some warnings with const correctness
 * 31 May 99 - Doug: Added code for the new ids (idVar, idSP, idFP, idASP)
 * 02 Jun 99 - Mike: Removed leading upper case on function names
 * 23 Jun 99 - Mike: Fixed pushArr (was pushing n**2 items)
 * 30 Jun 99 - Mike: getSubExpr() wasn't skipping fixed and var args properly
 * 06 Jul 99 - Mike: idSignExt prints after the expression now (sim to SSL)
 * 07 Jul 99 - Doug: Removed idSP and idFP
 * 03 Sep 99 - Mike: Replaced the buggy old version of substReg with one that
 *              uses Doug's searchReplaceAll()
 * 07 Sep 99 - Mike: Fixed Doug's searchReplaceAll() to cope with replacing "a"
 *              with "abc"
 * 22 Feb 00 - Mike: Simplify a[ size{ m[ any ]}]
 * 17 Mar 00 - Cristina: items[idAGP] is now initialized 
 * 20 Mar 00 - Mike: SemStr have size now
 * 21 Mar 00 - Mike: Added idCodeAddr to represent addresses in the code segment
 * 22 Mar 00 - Mike: Sort SemStr by type and size now
 * 30 Mar 00 - Mike: Search (but not replace) respects type now (both in the
 *              comparison, and when returning a search result)
 * 12 Apr 00 - Mike: Split SemStr::operator< into fully type sensitive and sign
 *              insensitive parts (latter in new function lessSGI)
 * 28 Apr 00 - Mike: Changed searchReplaceAll() yet again to fix a problem with
 *              replacing a in (a*b)+(a*c)
 * 31 May 00 - Mike: Minor changes to make it easier to port this code to Win32
 * 04 Jul 00 - Mike: Changed the getSubType() logic, to include the current
 *              index and the current type. Also doSearchAll stores the type
 *              of the subexpressions found
 * 19 Jul 00 - Mike: Fixed optimisation of a << k where k > 32
 * 19 Jul 00 - Mike: Fixed getSubType (now 3 functions because of a const
 *              problem); was not getting the right type for other than the
 *              first subexpression
 * 29 Aug 00 - Mike: Added SemStr::operator%() (type insensitive less than)
 * 28 Sep 00 - Mike: Added bumpRegisterAll()
 * 29 Nov 00 - Mike: Implemented searchMachSpec; now have "the line"
 * 30 Nov 00 - Mike: idTrunc -> idTruncu, idTruncs
 * 01 Dec 00 - Mike: Fixed problem with debugging code in doSimplify()
 * 07 Dec 00 - Mike: simplifyArith works on multisets now, to fix a=b+b bug
 * 31 Jan 01 - Mike: Removed idDef (two of them!) and idIndex; not used except
 *              for UQDBT on another CVS tree
 * 13 Feb 01 - Mike: Added idCastIntStar (needed for solution to the "second
 *              half of 64 bit var" problem)
 * 15 Feb 01 - Nathan: Added idExecute (used in fetch-exec loop)
 * 05 Mar 01 - Simon: Removed expand(), doExpand()
 * 06 Mar 01 - Mike: Added printt (prints with type, e.g. r[8]<32i>); using
 *              this in operator<< is now an official debugging aid
 * 20 Mar 01 - Mike: SemStr constructor taking two int*s is no longer unused
 * 29 Mar 01 - Mike: Added SemStr::printAsC
 * 05 Apr 01 - Mike: Fixed the logic of Accumulate; was essentially prepending
 *              where he (Doug) should have been appending. Fixes a problem
 *              with simplification of some RTLs, particularly subx r1,r2,r3
 * 18 Apr 01 - Mike: Renamed SemStr::operator<< to operator&&; operator<< now
 *              does a push() operation.
 * 04 May 01 - Mike: Added operator<< which takes a SemStr& (like pushSS(*arg))
 * 09 May 01 - Mike: Added operator>>(int) (does a prep)
 * 10 May 01 - Nathan: Added idCTI and idPostVar to the sem table.
 * 26 Jul 01 - Mike: Added idLNot for logical not
 * 08 Aug 01 - Mike: doSimplify handles many more (k1 op k2) cases
 * 08 Aug 01 - Mike: doSimplify handles some more expr op k cases, esp AND
 * 20 Aug 01 - Mike: Suppressed the a[m[x]] == x simplification for now
 * 28 Aug 01 - Mike: Oops, doSimplifyArith returns void
 * 29 Aug 01 - Mike: Was not skipping integer and fixed args in doSimplifyAddr
 * 31 Aug 01 - Mike: getIndex() does not assert if out of bounds (returns -1)
 * 19 Oct 01 - Mike: Implemented exp1 ^ exp1 simplification
 * 28 Nov 01 - Mike: Removed doInstantiate and instantiate obsolete methods
 */


/*==============================================================================
 * Dependencies.
 *============================================================================*/

#include <stdarg.h>
#include "global.h"
#include "ss.h"
#include "rtl.h"

/*==============================================================================
 * Global declarations.
 *============================================================================*/
// The global table of SemItems, which is what the semantic string
// indexes into. Note: declared in driver.cc now, before the definition
// of prog, which contains a CSR object
// SemTable theSemTable;


/*==============================================================================
 * FUNCTION:        SemItem::SemItem
 * OVERVIEW:        Constructor.
 * PARAMETERS:      c - category of the item
 *                  ni - number of integer arguments
 *                  nf - number of fixed arguments
 *                  nv - number of variable arguments
 *                  s - string representation
 * RETURNS:         <nothing>
 *============================================================================*/
SemItem::SemItem(CAT c, int ni, int nf, int nv, const string& s)
    : cat(c), iNumIntArgs(ni), iNumFixedArgs(nf), iNumVarArgs(nv), sName(s)
{}

/*==============================================================================
 * FUNCTION:        SemItem::SemItem
 * OVERVIEW:        Constructor.
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
SemItem::SemItem()
{ }

/*==============================================================================
 * FUNCTION:        SemItem::print
 * OVERVIEW:        Output SemItem to ostream.
 * PARAMETERS:      os - ostream to use
 * RETURNS:         <nothing>
 *============================================================================*/
void SemItem::print(ostream& os)
{
    os << sName << ": category = " << (int)cat << ", int args = " <<
        iNumIntArgs;
    os << ", fixed args = " << iNumFixedArgs << ", var args = " << iNumVarArgs;
}

#if 0
// I'd prefer to avoid this... suggestions welcome
SemItem& SemItem::operator=(const SemItem& other)
{
    cat = other.cat;
    iNumIntArgs = other.iNumIntArgs;
    iNumFixedArgs = other.iNumFixedArgs;
    iNumVarArgs = other.iNumVarArgs;
    sName = other.sName;
    return *this;
}
#endif

/*==============================================================================
 * FUNCTION:        ints2dbl
 * OVERVIEW:        Converts two integers to a double.
 *                  Note: This may have endianess problems.
 * PARAMETERS:      i1 - the high word in the double
 *                  i2 - the low word in the double
 * RETURNS:         the double built from the two integers
 *============================================================================*/
double ints2dbl(int i1, int i2)
{
    int arr[2];
    double* pDbl = (double*) arr;
    arr[0] = i1;
    arr[1] = i2;
    return *pDbl;   
}

/*==============================================================================
 * FUNCTION:        dbl2ints
 * OVERVIEW:        Converts a double to two integers.
 *                  Note: This may have endianess problems.
 * PARAMETERS:      f - the double to be converted
 *                  i1 - the high word of the double
 *                  i2 - the low word of the double
 * RETURNS:         <nothing>
 *============================================================================*/
void dbl2ints(double f, int& i1, int& i2)
{
    int arr[2];
    double* pDbl = (double*) arr;
    *pDbl = f;                      // Copy to the array
    i1 = arr[0];
    i2 = arr[1];
}

/*==============================================================================
 * FUNCTION:        SemTable::SemTable
 * OVERVIEW:        Constructor.
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
SemTable::SemTable()
    : items(idNumOf)            // This is very important. Without it,
                                // the items vector won't get initialised
                                // properly. Really would prefer resize()
                                // but gcc doesn't seem to support it
{
    init();                     // Initialise the table. Note that this
                                // could fail due to lack of memory or
                                // other problem, in which case it won't
                                // be handled very elegantly

    // Now build the map for these initial entries.
    for (int i = 0; i < idNumOf; i++)
        itemIndexes[items[i].sName] = i;
}

/*==============================================================================
 * FUNCTION:        SemTable::init
 * OVERVIEW:        Fills in the standard items (i.e. non-processor specific)
 *                  of the semantic table.
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
void SemTable::init()
{
    //   index                          int,fixd,var,text
    items[idPlus    ] = SemItem(cOPERATOR, 0, 0, 2, "+");
    items[idMinus   ] = SemItem(cOPERATOR, 0, 0, 2, "-");
    items[idMult    ] = SemItem(cOPERATOR, 0, 0, 2, "*");
    items[idDiv     ] = SemItem(cOPERATOR, 0, 0, 2, "/");
    items[idMults   ] = SemItem(cOPERATOR, 0, 0, 2, "*!");
    items[idDivs    ] = SemItem(cOPERATOR, 0, 0, 2, "/!");
    items[idMod     ] = SemItem(cOPERATOR, 0, 0, 2, "%");
    items[idMods    ] = SemItem(cOPERATOR, 0, 0, 2, "%!");
    items[idNeg     ] = SemItem(cOPERATOR, 0, 0, 1, "0-");

    items[idAnd     ] = SemItem(cOPERATOR, 0, 0, 2, "and");
    items[idOr      ] = SemItem(cOPERATOR, 0, 0, 2, "or");
    items[idEquals  ] = SemItem(cOPERATOR, 0, 0, 2, "=");
    items[idNotEqual] = SemItem(cOPERATOR, 0, 0, 2, "~=");
    items[idLess    ] = SemItem(cOPERATOR, 0, 0, 2, "<");
    items[idGtr     ] = SemItem(cOPERATOR, 0, 0, 2, ">");
    items[idLessEq  ] = SemItem(cOPERATOR, 0, 0, 2, "<=");
    items[idGtrEq   ] = SemItem(cOPERATOR, 0, 0, 2, ">=");
    items[idLessUns ] = SemItem(cOPERATOR, 0, 0, 2, "<u");
    items[idGtrUns  ] = SemItem(cOPERATOR, 0, 0, 2, ">u");
    items[idLessEqUns]= SemItem(cOPERATOR, 0, 0, 2, "<=u");
    items[idGtrEqUns] = SemItem(cOPERATOR, 0, 0, 2, ">=u");

    items[idFPlus   ] = SemItem(cOPERATOR, 0, 0, 2, "+f");
    items[idFMinus  ] = SemItem(cOPERATOR, 0, 0, 2, "-f");
    items[idFMult   ] = SemItem(cOPERATOR, 0, 0, 2, "*f");
    items[idFDiv    ] = SemItem(cOPERATOR, 0, 0, 2, "/f");
    items[idFPlusd  ] = SemItem(cOPERATOR, 0, 0, 2, "+fd");
    items[idFMinusd ] = SemItem(cOPERATOR, 0, 0, 2, "-fd");
    items[idFMultd  ] = SemItem(cOPERATOR, 0, 0, 2, "*fd");
    items[idFDivd   ] = SemItem(cOPERATOR, 0, 0, 2, "/fd");
    items[idFPlusq  ] = SemItem(cOPERATOR, 0, 0, 2, "+fq");
    items[idFMinusq ] = SemItem(cOPERATOR, 0, 0, 2, "-fq");
    items[idFMultq  ] = SemItem(cOPERATOR, 0, 0, 2, "*fq");
    items[idFDivq   ] = SemItem(cOPERATOR, 0, 0, 2, "/fq");
    items[idFMultsd ] = SemItem(cOPERATOR, 0, 0, 2, "*fsd");
    items[idFMultdq ] = SemItem(cOPERATOR, 0, 0, 2, "*fdq");

    items[idSQRTs   ] = SemItem(cOPERATOR, 0, 0, 1, "SQRTs");
    items[idSQRTd   ] = SemItem(cOPERATOR, 0, 0, 1, "SQRTd");
    items[idSQRTq   ] = SemItem(cOPERATOR, 0, 0, 1, "SQRTq");
    items[idNot     ] = SemItem(cOPERATOR, 0, 0, 1, "~");   // Bit invert
    items[idLNot    ] = SemItem(cOPERATOR, 0, 0, 1, "L!");  // Logical not
    items[idSignExt ] = SemItem(cOPERATOR, 0, 0, 1, "!");   // Sign extend
    items[idBitAnd  ] = SemItem(cOPERATOR, 0, 0, 2, "&");
    items[idBitOr   ] = SemItem(cOPERATOR, 0, 0, 2, "|");
    items[idBitXor  ] = SemItem(cOPERATOR, 0, 0, 2, "^");
    items[idShiftL  ] = SemItem(cOPERATOR, 0, 0, 2, "<<");
    items[idShiftR  ] = SemItem(cOPERATOR, 0, 0, 2, ">>");
    items[idShiftRA ] = SemItem(cOPERATOR, 0, 0, 2, ">>A");
    items[idRotateL ] = SemItem(cOPERATOR, 0, 0, 2, "rl");
    items[idRotateR ] = SemItem(cOPERATOR, 0, 0, 2, "rr");
    items[idRotateLC] = SemItem(cOPERATOR, 0, 0, 2, "rlc");
    items[idRotateRC] = SemItem(cOPERATOR, 0, 0, 2, "rrc");

    items[idTern    ] = SemItem(cOPERATOR, 0, 0, 3, "?:");
    items[idAt      ] = SemItem(cOPERATOR, 0, 0, 3, "@");

    items[idMemOf   ] = SemItem(cOPERATION, 0, 0, 1, "m[");
    items[idRegOf   ] = SemItem(cOPERATION, 0, 0, 1, "r[");
    items[idAddrOf  ] = SemItem(cOPERATION, 0, 0, 1, "a[");
    items[idVar     ] = SemItem(cOPERATION, 1, 0, 0, "v");
    items[idParam   ] = SemItem(cOPERATION, 0, 1, 0, "param `");
    items[idRparam  ] = SemItem(cOPERATION, 0, 1, 0, "rparam `");
    items[idExpand  ] = SemItem(cOPERATION, 0, 1, 0, "expand `");
    items[idTemp    ] = SemItem(cOPERATION, 0, 1, 0, "temp `");
    items[idSize    ] = SemItem(cOPERATION, 1, 0, 1, "size ");
    items[idCastIntStar] = SemItem(cOPERATION,0,0,1, "(int*) ");
    items[idPostVar ] = SemItem(cOPERATION, 0, 0, 1, "'");

    items[idTruncu  ] = SemItem(cFUNCTION, 2, 0, 1, "truncu(");
    items[idTruncs  ] = SemItem(cFUNCTION, 2, 0, 1, "truncs(");
    items[idZfill   ] = SemItem(cFUNCTION, 2, 0, 1, "zfill(");
    items[idSgnEx   ] = SemItem(cFUNCTION, 2, 0, 1, "sgnex(");

    items[idFsize   ] = SemItem(cFUNCTION, 2, 0, 1, "fsize(");
    items[idItof    ] = SemItem(cFUNCTION, 2, 0, 1, "itof(");
    items[idFtoi    ] = SemItem(cFUNCTION, 2, 0, 1, "ftoi(");
    items[idFround  ] = SemItem(cFUNCTION, 2, 0, 1, "fround(");
    items[idForceInt] = SemItem(cFUNCTION, 0, 0, 1, "toint(");
    items[idForceFlt] = SemItem(cFUNCTION, 0, 0, 1, "toflt(");

    items[idFpush   ] = SemItem(cOPERATION, 0, 0, 0, "FPUSH");
    items[idFpop    ] = SemItem(cOPERATION, 0, 0, 0, "FPOP");

    items[idSin     ] = SemItem(cFUNCTION, 0, 0, 1, "sin(");
    items[idCos     ] = SemItem(cFUNCTION, 0, 0, 1, "cos(");
    items[idTan     ] = SemItem(cFUNCTION, 0, 0, 1, "tan(");
    items[idArcTan  ] = SemItem(cFUNCTION, 0, 0, 1, "arctan(");
    items[idLog2    ] = SemItem(cFUNCTION, 0, 0, 1, "log2(");
    items[idLog10   ] = SemItem(cFUNCTION, 0, 0, 1, "log10(");
    items[idLoge    ] = SemItem(cFUNCTION, 0, 0, 1, "loge(");
    items[idSqrt    ] = SemItem(cFUNCTION, 0, 0, 1, "sqrt(");
    items[idExecute ] = SemItem(cFUNCTION, 0, 0, 1, "execute(" );

    items[idIntConst] = SemItem(cCONSTANT, 1, 0, 0, "int");
    items[idFltConst] = SemItem(cCONSTANT, 2, 0, 0, "flt");
    items[idCodeAddr] = SemItem(cCONSTANT, 1, 0, 0, "code");

    items[idPC]       = SemItem(cOPERATION, 0, 0, 0, "%pc");
    items[idAFP]      = SemItem(cOPERATION, 0, 0, 0, "%afp");
    items[idAGP]      = SemItem(cOPERATION, 0, 0, 0, "%agp");
    items[idTmpNul]   = SemItem(cSPECIAL,   0, 0, 0, "tmpNul");

    // ------------------------- "The Line" -------------------------//
    // Items below this line (idMachSpec and above) are source machine specific
    // and generally should not appear in the SemStrs for the back end to
    // process. Most should be transformed away; any that are not (e.g. %Y,
    // %CF) need to have local variables declared for them in the back end.
    // NJMCDecoder::decodeInstruction() checks for these

    items[idZF]       = SemItem(cOPERATION, 0, 0, 0, "%ZF");
    items[idCF]       = SemItem(cOPERATION, 0, 0, 0, "%CF");
    items[idNF]       = SemItem(cOPERATION, 0, 0, 0, "%NF");
    items[idOF]       = SemItem(cOPERATION, 0, 0, 0, "%OF");
    items[idFZF]      = SemItem(cOPERATION, 0, 0, 0, "%FZF");
    items[idFLF]      = SemItem(cOPERATION, 0, 0, 0, "%FLF");
    items[idFGF]      = SemItem(cOPERATION, 0, 0, 0, "%FGF");
    items[idCTI]      = SemItem(cOPERATION, 0, 0, 0, "%CTI"); // magical
    items[idNEXT]     = SemItem(cOPERATION, 0, 0, 0, "%NEXT");
    // so magical I haven't even explained it to uqbt yet...
}

/*==============================================================================
 * FUNCTION:        SemTable::operator[]
 * OVERVIEW:        Return the SemItem at a given index in this table. Performs
 *                  bounds checking. 
 * PARAMETERS:      i - an index
 * RETURN:          the SemItem at the given index
 *============================================================================*/
const SemItem& SemTable::operator[](int i) const
{
    assert(i >= 0 && i < (int)items.size());
    if (i >= (int)items.size()) {
        ostrstream ost;
        ost << "Out of bounds index (" << i << ") into the semantic table";
        error(str(ost));
    }

    return items[i];
}

/*==============================================================================
 * FUNCTION:        SemTable::addItem
 * OVERVIEW:        Adds a new item to the table.
 * PARAMETERS:      c - category of the item
 *                  ni - number of integer arguments
 *                  nf - number of fixed arguments
 *                  nv - number of variable arguments
 *                  s - string representation of the item
 * RETURNS:         the index at which the item was added
 *============================================================================*/
int SemTable::addItem(CAT c, int ni, int nf, int nv, const char* s)
{
    SemItem item(c, ni, nf, nv, s);

    // Check if the item is already in the table and return
    // the index at which it exists
    int index = this->findItem(s, false);
    if (index != -1)
            return index;
    else {
        items.push_back(item);
        itemIndexes[s] = items.size() - 1;
        return items.size() - 1;
    }
}

/*==============================================================================
 * FUNCTION:        SemTable::addItem
 * OVERVIEW:        A short hand version of AddItem for adding items in the
 *                  cSPECIAL category that has no arguments.
 * PARAMETERS:      s - string representation of the item
 * RETURNS:         the index at which the item was added
 *============================================================================*/
int SemTable::addItem(const char* s)
{
    return this->addItem(cSPECIAL,0,0,0,s);
}

/*==============================================================================
 * FUNCTION:        SemTable::findItem
 * OVERVIEW:        Find the index of an item in the table using its
 *                  string representation as the search key.
 * PARAMETERS:      s - string representation of item to search for
 *                  mustFind - if true, display an error if not found
 * RETURNS:         the index of the item or -1 if it wasn't found
 *============================================================================*/
int SemTable::findItem(const string& s, bool mustFind /*= true*/)
{
    
    map<string,int>::iterator it = itemIndexes.find(s);
    if (it != itemIndexes.end())
        return (*it).second;
    else {
        // Only display error message if the item should be in the table.
        if (mustFind) {
            error("Unknown item " + s);
        }
        return -1;          // Error return
    }
}

/*==============================================================================
 * FUNCTION:        SemTable::setItem
 * OVERVIEW:        Modify an existing item in the table.
 * PARAMETERS:      index - the item to modify
 *                  c - category of the item
 *                  ni - number of integer arguments
 *                  nf - number of fixed arguments
 *                  nv - number of variable arguments
 *                  s - string representation of the item
 * RETURNS:         the index
 *============================================================================*/
int SemTable::setItem(int index, CAT c, int ni, int nf, int nv, const char* s)
{
    SemItem item(c, ni, nf, nv, s);

    if( index < 0 || (unsigned)index >= items.size() )
        return -1;
    
    items[index] = item;
    return index;
}

/*==============================================================================
 * FUNCTION:        SemTable::size
 * OVERVIEW:        Get the size of the table.
 * PARAMETERS:      <none>
 * RETURNS:         the size of the table
 *============================================================================*/
unsigned SemTable::size() const
{
    return items.size();
}

/*==============================================================================
 * FUNCTION:        SemTable::print
 * OVERVIEW:        Output the entries of the sem table.
 * PARAMETERS:      os - the output stream to use
 * RETURNS:         <nothing>
 *============================================================================*/
void SemTable::print(ostream& os)
{
    int idx = 0;
    for(vector<SemItem>::iterator it = items.begin(); it != items.end(); it++){
        os << idx++ <<": ";
        it->print(os);
        os << endl;
    }
}

/////////////////////////////
//                         //
//  Semantic String class  //
//                         //
/////////////////////////////

/*==============================================================================
 * FUNCTION:        SemStr::SemStr
 * OVERVIEW:        Default constructor.
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
SemStr::SemStr()
 : kind(eORDINARY)
{
}


/*==============================================================================
 * FUNCTION:        SemStr::SemStr
 * OVERVIEW:        Constructor with kind (e.g. when constructing OpTableExpr
 *                  etc).
 * PARAMETERS:      k - an enum e.g. eOPTABLE
 * RETURNS:         <nothing>
 *============================================================================*/
SemStr::SemStr(EKIND k)
 : kind(k)
{
}

/*==============================================================================
 * FUNCTION:        SemStr::SemStr
 * OVERVIEW:        Constructor with size (e.g. 16)
 * PARAMETERS:      s - size of the expression in bits
 * RETURNS:         <nothing>
 *============================================================================*/
SemStr::SemStr(int s)
 : kind(eORDINARY)
{
    type.setSize(s);
}

/*==============================================================================
 * FUNCTION:        SemStr::SemStr
 * OVERVIEW:        Constructor with Type
 * PARAMETERS:      ty: Type to initualise the type to
 * RETURNS:         <nothing>
 *============================================================================*/
SemStr::SemStr(const Type& ty)
 : kind(eORDINARY), type(ty)
{
}

/*==============================================================================
 * FUNCTION:        SemStr::SemStr
 * OVERVIEW:        Copy constructor
 * PARAMETERS:      other - reference to SemStr to copy
 * RETURNS:         <nothing>
 *============================================================================*/
SemStr::SemStr(const SemStr& other)
 : indices(other.indices), kind(other.kind), type(other.type)
{
}

/*==============================================================================
 * FUNCTION:        SemStr::operator=
 * OVERVIEW:        Assignment operator
 * NOTE:            Since SemStr's don't have pointers, the default bit copy
 *                    implementation is OK, but it's sometimes useful to be
 *                    able to breakpoint this function, or add debugging
 * PARAMETERS:      other - reference to SemStr being assigned to this
 * RETURNS:         this SemStr object
 *============================================================================*/
SemStr& SemStr::operator=(const SemStr& other)
{
    if (this != &other) {
        indices = other.indices;
        kind = other.kind;
        type = other.type;
    }
    return *this;
}

/*==============================================================================
 * FUNCTION:        SemStr::SemStr
 * OVERVIEW:        Construct from a pair of integer pointers
 * PARAMETERS:      it - pointer to first integer
 *                  end - pointer to past-end integer
 * RETURNS:         <nothing>
 *============================================================================*/
SemStr::SemStr(int* it, int* end, const Type& t /* = Type() */)
 : indices(it, end), kind(eORDINARY), type(t)
{
}

/*==============================================================================
 * FUNCTION:        SemStr::SemStr
 * OVERVIEW:        Construct from a size and variable-length list of integers
 * PARAMETERS:      s - size of array
 *                  first - first element (to avoid ambiguity with SemStr(int))
 *                  ... - remaining elements
 * RETURNS:         <nothing>
 *============================================================================*/
SemStr::SemStr(int s, int first, ... )
 : kind(eORDINARY), type(Type())
{
    indices.push_back( first );
    va_list ap;
    va_start( ap, first );
    while( s > 1 ) {
        indices.push_back( va_arg(ap,int) );
        s--;
    }
    va_end(ap);
}

/*==============================================================================
 * FUNCTION:        SemStr::SemStr
 * OVERVIEW:        Construct from two const list iterators.
 * PARAMETERS:      it - first iterator
 *                  end - last iterator (excluded)
 * RETURNS:         <nothing>
 *============================================================================*/
SemStr::SemStr(SSCIT it, SSCIT& end, const Type& t /*=Type()*/)
 : indices(it, end), kind(eORDINARY), type(t)
{
}

/*==============================================================================
 * FUNCTION:        SemStr::SemStr
 * OVERVIEW:        Construct from two nonconst list iterators.
 * PARAMETERS:      it - first iterator
 *                  end - last iterator (excluded)
 * RETURNS:         <nothing>
 *============================================================================*/
SemStr::SemStr(SSIT it, SSIT& end, const Type& t /*=Type()*/)
 : indices(it, end), kind(eORDINARY), type(t)
{
}

/*==============================================================================
 * FUNCTION:        SemStr::~SemStr
 * OVERVIEW:        The destructor. Nothing special required.
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
SemStr::~SemStr()
{
}

/*==============================================================================
 * FUNCTION:        SemStr::operator==
 * OVERVIEW:        Equality comparsion.
 * PARAMETERS:      other - Semantic string being compared with
 * RETURNS:         this == other
 *============================================================================*/
bool SemStr::operator==(const SemStr& other) const
{
    return (indices == other.indices) && (kind == other.kind) &&
        (type == other.type);
}

/*==============================================================================
 * FUNCTION:        SemStr::operator%=
 * OVERVIEW:        "Approximately equal" comparsion. Compares without regard
 *                  to ANY type
 * PARAMETERS:      other - Semantic string being compared with
 * RETURNS:         this == other
 *============================================================================*/
bool SemStr::operator%=(const SemStr& other) const
{
    return (indices == other.indices) && (kind == other.kind);
}

/*==============================================================================
 * FUNCTION:        SemStr::operator-=
 * OVERVIEW:        Equality operator, but disregards sign
 * NOTE:            Not "minus equals"
 * PARAMETERS:      other - Semantic string being compared with
 * RETURNS:         this == other
 *============================================================================*/
bool SemStr::operator-=(const SemStr& other) const
{
    return (indices == other.indices) && (kind == other.kind) &&
        (type -= other.type);       // This is a compare, not "minus equals"
}

/*==============================================================================
 * FUNCTION:        SemStr::operator<
 * OVERVIEW:        Defines an ordering between SemStr's. This operator is
 *                    a "less than" for SemStrs, which respects the type of the
 *                    SemStrs being compared
 * NOTE:            The size and type of SemStr ARE considered, so for example
 *                    two SemStrs both with "m[%afp-6]" but different sizes or
 *                    types or "signedness" are considered different
 * PARAMETERS:      other - semantic string being compared to
 * RETURNS:         this is less than other (considering type)
 *============================================================================*/
bool SemStr::operator<(const SemStr& other) const
{
    if (type < other.type) return true;     // Type is "most significant" wrt
    if (other.type < type) return false;    // the ordering
    // Note: gcc will let you say type > other.type, and it will create
    // an operator> for you (calls operator< with the arguments swapped).
    // Lesser C++ compilers will just barf
    return (*this && other);                // Do sign insensitive compare
}

/*==============================================================================
 * FUNCTION:        SemStr::operator&&
 * OVERVIEW:        Defines an ordering between SemStr's. This operator is
 *                    a "less than" which disregards the "signedness" of the
 *                    SemStrs being compared
 * NOTE:            The sign of SemStr are NOT considered, so for
 *                    example two SemStrs both with "m[%afp-6]" and 32 bits
 *                    but one is signed and the other unsigned are considered
 *                    the same (neither is less than the other)
 * PARAMETERS:      other - semantic string being compared to
 * RETURNS:         this is less than other (disregarding type)
 *============================================================================*/
bool SemStr::operator&&(const SemStr& other) const
{
    // First compare types, excluding sign
    // This is the "most significant part" of the ordering
    if (type << other.type) return true;
    if (other.type << type) return false;
    return *this % other;

}

/*==============================================================================
 * FUNCTION:        SemStr::operator%
 * OVERVIEW:        Defines an ordering between SemStr's. This operator is
 *                    a "less than" for SemStrs which disregards the type of
 *                    the SemStrs being compared altogether
 * NOTE:            The type of SemStrs are NOT considered, so for
 *                    example two SemStrs both with "m[%afp-6]", one 32 bits
 *                    and signed and the other 8 bits and unsigned are con-
 *                    sidered the same (neither is less than the other)
 * PARAMETERS:      other - semantic string being compared to
 * RETURNS:         this is less than other (disregarding type)
 *============================================================================*/
bool SemStr::operator%(const SemStr& other) const
{
    // Compare by number of items
    if (indices.size() < other.indices.size()) return true;
    if (other.indices.size() < indices.size()) return false;
    // Number of items is equal. Use the indices themselves to order
    SSCIT it1, it2;
    for (it1 = indices.begin(), it2 = other.indices.begin();
         it1 != indices.end(); it1++, it2++)
    {
        if (*it1 < *it2) return true;
        if (*it2 < *it1) return false;
    }
    return false;
}

/*==============================================================================
 * FUNCTION:        SemStr::operator<<
 * OVERVIEW:        Push an integer to the end of this semantic string.
 *                  Allows for the quick building up of a string, e.g.
 *                    str << idRegOf << idIntConst << 2
 * NOTE:            There was a signless "operator<" which was called this
 * PARAMETERS:      i - integer to append to this string
 * RETURNS:         Reference to *this, so can chain << in the usual way
 *============================================================================*/
SemStr& SemStr::operator<<(int i)
{
    indices.push_back(i);
    return *this;
}

/*==============================================================================
 * FUNCTION:        SemStr::operator<<
 * OVERVIEW:        Push a semantic string to the end of this semantic string.
 *                  Allows for the quick building up of a string, e.g.
 *                    str << idPlus << makeReg(rs1) << makeReg(rs2)
 * NOTE:            There was a signless "operator<" which was called this
 * PARAMETERS:      pss - pointer to the SemStr to add
 * RETURNS:         Reference to *this, so can chain << in the usual way
 *============================================================================*/
SemStr& SemStr::operator<<(const SemStr& ss)
{
    indices.insert(indices.end(), ss.indices.begin(), ss.indices.end());
    return *this;
}

/*==============================================================================
 * FUNCTION:        SemStr::operator>>
 * OVERVIEW:        Prepend an integer to the front of this semantic string.
 *                  Allows for the quick building up of a string, e.g.
 *                    str >> idRegOf
 * PARAMETERS:      i - integer to prepend to this string
 * RETURNS:         Reference to *this, so can chain >> in the usual way
 *============================================================================*/
SemStr& SemStr::operator>>(int i)
{
    indices.push_front(i);
    return *this;
}

/*==============================================================================
 * FUNCTION:        doSprintPrefix
 * OVERVIEW:        Do the main work for string print (prefix version)
 * PARAMETERS:      it -
 *                  res -
 *                  end -
 * RETURNS:         <nothing>
 *============================================================================*/
void doSprintPrefix(SSCIT& it, string& res, SSCIT& end)
{
    char buf[40];

    if (it == end)
    {
        // An empty semantic string. Don't attempt recursion
        return;
    }
    int idx = *it++;
    CAT cat = theSemTable[idx].cat;

    switch (cat)
    {
        case cOPERATOR:
        case cSPECIAL:
        {
            // The string for the operator
            res += theSemTable[idx].sName + " ";
            // Print the integer args (if any)
            int ii;
            for (ii=0; ii < theSemTable[idx].iNumIntArgs; ii++)
            {
                char* tmp = new char[15];
                sprintf(tmp, "%d " , *it++);
                res += tmp;
            }

            // Now the fixed args
            for (ii=0; ii < theSemTable[idx].iNumFixedArgs; ii++)
            {
                int id = *it++;             // The index to the item
                res += theSemTable[id].sName;
            }
            
            // Now print the variable arguments using recursion
            for (int i = 0; i < theSemTable[idx].iNumVarArgs; i++)
            {
                doSprintPrefix(it, res, end);
            }
            break;
        }

        case cCONSTANT:
        {
            res += theSemTable[idx].sName + " ";
            if (idx == idFltConst)
            {
                // There are two ints that represent the double
                int i1 = *it++;
                int i2 = *it++;
                double f = ints2dbl(i1, i2);
                // print it
                sprintf(buf, "%g ", f);
                res += buf;
                break;
            }
            // Else assume an integer constant
            // Get the integer
            int iConst = *it++;
            // print it
            sprintf(buf, "%d ", iConst);
            res += buf;
            break;
        }

        case cOPERATION:        // Register, memory, parameter etc
        case cFUNCTION:         // fsize(), sin(), etc
        {
            // The operation's (or function's) name
            res += theSemTable[idx].sName + " ";
            // Print the integer args (if any)
            int ii;
            for (ii=0; ii < theSemTable[idx].iNumIntArgs; ii++)
            {
                char* tmp = new char[15];
                sprintf(tmp, "%d " , *it++);
                res += tmp;
            }

            // Now the fixed args
            for (ii=0; ii < theSemTable[idx].iNumFixedArgs; ii++)
            {
                int id = *it++;             // The index to the item
                res += theSemTable[id].sName;
            }
            
            // Recurse to print the variable args
            for (ii=0; ii < theSemTable[idx].iNumVarArgs; ii++)
                doSprintPrefix(it, res, end);

            // Any trailing characters
            if (cat == cFUNCTION)
                res += ") ";
            else switch(idx)
            {
                case idParam:
                case idRparam:
                case idExpand:
                case idTemp:
                    res += "' "; break;
                case idMemOf:
                case idRegOf:
                case idAddrOf:
                    res += "] "; break;
                case idSize:
                    break;
            }
            break;
        }

        default:
        {
            ostrstream ost;
            ost << "Error! unknown category " << (int)cat;
            ost << " in ss::doSprintPrefix()!\n",
            error(str(ost));
        }
    }
}

/*==============================================================================
 * FUNCTION:        SemStr::sprintPrefix
 * OVERVIEW:        Print (prefix mode) to a string
 * PARAMETERS:      <none>
 * RETURNS:         A c++ string representing this SemStr
 *============================================================================*/
string SemStr::sprintPrefix() const
{
    SSCIT it = indices.begin();
    SSCIT end = indices.end();
    string res;

    doSprintPrefix(it, res, end);
    return res;                 // We have to copy the result
}

/*==============================================================================
 * FUNCTION:        SemStr::printPrefix
 * OVERVIEW:        Print (prefix mode) to a stream
 * PARAMETERS:      Stream to send to
 * RETURNS:         <nothing>
 *============================================================================*/
// Print a prefix representation of the object to the given file stream
void SemStr::printPrefix(ostream& os /*= cout*/) const
{
    string s = sprintPrefix();      // Print to a string
    os << s;                        // Print to the output stream
}

/*==============================================================================
 * FUNCTION:        SemStr::dump
 * OVERVIEW:        Debug function: dump string to given stream as just a
 *                    list of integers
 * PARAMETERS:      Stream to send to
 * RETURNS:         <nothing>
 *============================================================================*/
// Print a prefix representation of the object to the given file stream
void SemStr::dump(ostream& os /*= cout*/) const
{
    list<int>::const_iterator it;
    for (it=indices.begin(); it != indices.end(); it++) {
        os << *it << " ";
    }
    os << "\n";
}


//
//  Various functions to push pieces to the back of the semantic string
//

/*==============================================================================
 * FUNCTION:        SemStr::push
 * OVERVIEW:        Push an index to the back of this SemStr
 * PARAMETERS:      integer to push
 * RETURNS:         <nothing>
 *============================================================================*/
void SemStr::push(int i)
{
    indices.push_back(i);
}

/*==============================================================================
 * FUNCTION:        SemStr::prep
 * OVERVIEW:        Prepend given integer to the front of this SemStr
 * PARAMETERS:      The integer to prepend
 * RETURNS:         <nothing>
 *============================================================================*/
void SemStr::prep(int i)
{
    indices.push_front(i);
}

/*==============================================================================
 * FUNCTION:        SemStr::pushSS
 * OVERVIEW:        Makes a copy of the given semantic string, and pushes this
 *                  at the end of this semantic string.
 * PARAMETERS:      pss: pointer to SemStr to copy and append to this SemStr
 * RETURNS:         <nothing>
 *============================================================================*/
void SemStr::pushSS(const SemStr* pss)
{
    indices.insert(indices.end(), pss->indices.begin(), pss->indices.end());
}

/*==============================================================================
 * FUNCTION:        SemStr::pushArr
 * OVERVIEW:        Push n items of array a
 * PARAMETERS:      n: number of items
 *                  a: array of items
 * RETURNS:         <nothing>
 *============================================================================*/
void SemStr::pushArr(int n, const int a[])
{
    indices.insert(indices.end(), a, a+n);
}

/*==============================================================================
 * FUNCTION:        SemStr::pop
 * OVERVIEW:        Remove the last index and return it.
 * PARAMETERS:      <none>
 * RETURNS:         the last index
 *============================================================================*/
int SemStr::pop()
{
    int last = indices.back();
    indices.pop_back();
    return last;
}

/*==============================================================================
 * FUNCTION:        SemStr::popFirst
 * OVERVIEW:        Remove the first index and return it.
 * PARAMETERS:      <none>
 * RETURNS:         the first index
 *============================================================================*/
int SemStr::popFirst()
{
    int first = indices.front();
    indices.pop_front();
    return first;
}

/*==============================================================================
 * FUNCTION:        SemStr::len
 * OVERVIEW:        Get the number of indices this semantic string
 * PARAMETERS:      <none>
 * RETURNS:         The size, e.g. 2 for "int 99"
 *============================================================================*/
int SemStr::len() const
{ return indices.size(); }

/*==============================================================================
 * FUNCTION:        SemStr::clear
 * OVERVIEW:        Set this to be the empty expression.
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
void SemStr::clear()
{
    indices.clear();
}

/*==============================================================================
 * FUNCTION:        SemStr::isSpRegEqual
 * NOTE:            Not used any more
 * OVERVIEW:        
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
// Return true if matches this special register
bool SemStr::isSpRegEqual(int idReg) const
{
    if ((indices.size() == 1) && (indices.front() == idReg))
        return true;
    return false;
}

/*==============================================================================
 * FUNCTION:        SemStr::isNumRegEqual
 * OVERVIEW:        Test whether this semantic string is exactly r[reg] where
 *                  reg is given as a parameter
 * PARAMETERS:      idReg: number of the register of interest, e.g. 9 for r[9]
 * RETURNS:         True if equal, false otherwise
 *============================================================================*/
// Return true if matches this numbered register
bool SemStr::isNumRegEqual(int idReg) const
{
    SSCIT it = indices.begin();
    if ((indices.size() == 3) && (indices.front() == idRegOf) &&
        (*++it == idIntConst) && 
        (indices.back() == idReg))
            return true;
    return false;
}

/*==============================================================================
 * FUNCTION:        SemStr::doIsRegCont
 * OVERVIEW:        Do the work for checking if a register is contained in this
 *                  semantic string
 * NOTE:            Obsolete; use search() instead
 * PARAMETERS:      
 * RETURNS:         
 *============================================================================*/
// This function is needed by the two functions checking to see if
// the register is contained in the SemStr (bSpcl is true if searching
// for a special register; false if searching for a numbered register)
bool doIsRegCont(int idRegLo, int idRegHi, bool bSpcl, SSCIT& it, SSCIT& end)
{
    if (it == end)
    {
        // An empty semantic string. Don't attempt recursion
        return false;
    }
    int idx = *it++;

    if (bSpcl && (idx >= idRegLo) && (idx <= idRegHi)) return true;
    if (!bSpcl)
    {
        // Looking for idRegOf then idIntConst then our idReg
        if (idx == idRegOf)
        {
            idx = *it++;
            if (idx == idIntConst)
            {
                idx = *it++;
                return (idx >= idRegLo) && (idx <= idRegHi);
            }
            else
                // idReg but not iConst: back up and skip args
                it--;
        }
    }

    // Skip the integer and fixed args
    int ii;
    for (ii=0; ii < theSemTable[idx].totalFixedArgs(); ii++)
        *it++;
    // Check the variable arguments using recursion
    for (ii = 0; ii < theSemTable[idx].iNumVarArgs; ii++)
        if (doIsRegCont(idRegLo, idRegHi, bSpcl, it, end)) return true;

    // Didn't find it in all the recursion so far...
    return false;
}

/*==============================================================================
 * FUNCTION:        SemStr::
 * OVERVIEW:        
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
bool SemStr::isSpRegCont(int idReg) const
{
    SSCIT it = indices.begin();
    SSCIT end = indices.end();
    return doIsRegCont(idReg, idReg, true, it, end);
}

/*==============================================================================
 * FUNCTION:        SemStr::
 * OVERVIEW:        
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
bool SemStr::isNumRegCont(int idReg) const
{
    SSCIT it = indices.begin();
    SSCIT end = indices.end();
    return doIsRegCont(idReg, idReg, false, it, end);
}

bool SemStr::isNumRegCont(int idRegLo, int idRegHi) const
{
    SSCIT it = indices.begin();
    SSCIT end = indices.end();
    return doIsRegCont(idRegLo, idRegHi, false, it, end);
}

/*==============================================================================
 * FUNCTION:        SemStr::getFirstIdx
 * OVERVIEW:        Get the first index of this semantic string; same as
 *                    getIndex(0)
 * PARAMETERS:      <none>
 * RETURNS:         The first index, e.g. idRegOf for r[9]
 *============================================================================*/
int SemStr::getFirstIdx() const
{
    if (indices.size() == 0) return -1;
    return indices.front();
}

/*==============================================================================
 * FUNCTION:        SemStr::getSecondIdx
 * OVERVIEW:        Get the second index of this semantic string; same as
 *                    getIndex(1)
 * PARAMETERS:      <none>
 * RETURNS:         The second index, e.g. idIntConst for r[9]
 *============================================================================*/
int SemStr::getSecondIdx() const
{
    if (indices.size() < 2) return -1;
    SSCIT it = indices.begin();
    return *++it;
}

/*==============================================================================
 * FUNCTION:        SemStr::getThirdIdx
 * OVERVIEW:        Get the third index of this semantic string; same as
 *                    getIndex(2)
 * PARAMETERS:      <none>
 * RETURNS:         The third index, e.g. 9 for r[9]
 *============================================================================*/
int SemStr::getThirdIdx() const
{
    if (indices.size() < 3) return -1;
    SSCIT it = indices.begin();
    return *++++it;
}

/*==============================================================================
 * FUNCTION:        SemStr::getLastIdx
 * OVERVIEW:        Get the last index of this semantic string
 * PARAMETERS:      <none>
 * RETURNS:         The last index, e.g. 9 for r[9]
 *============================================================================*/
int SemStr::getLastIdx() const
{
    if (indices.size() == 0) return -1;
    return indices.back();
}

/*==============================================================================
 * FUNCTION:        SemStr::getIndex
 * OVERVIEW:        Get the specified index; zero origin
 * PARAMETERS:      idx: index (0=first) to get
 * RETURNS:         The index, e.g. getIndex(1) == idIntConst for r[9]
 *============================================================================*/
int SemStr::getIndex(int idx) const
{
    if (idx >= (int)indices.size()) return -1;
    SSCIT it = indices.begin();
    // Skip idx items
    //for (int j=0; j < idx; j++) it++;
    advance(it, idx);
    return *it;
}

/*==============================================================================
 * FUNCTION:        SemStr::findParam
 * OVERVIEW:        find the first idParam in this semstr
 * PARAMETERS:      none
 * RETURNS:         The id number of the idParam found, else -1
 *============================================================================*/
int SemStr::findParam( )
{
    SemStr match, result;
    match.push(idParam);
    match.push(WILD);
    if( search(match, result, false) == false ) {
        /* contains no param */
        return -1;
    } else {
        return result.getSecondIdx();
    }
}


/*==============================================================================
 * FUNCTION:        SemStr::getSubExpr
 * OVERVIEW:        Get the ith subexpression (i.e. the ith variable
 *                  expression). The caller of this method is responsible for
 *                  the newly created SemStrs memory.
 * PARAMETERS:      i - the subexpression to return
 * RETURNS:         a pointer to the subexpression
 *============================================================================*/
SemStr* SemStr::getSubExpr(int i) const
{
    SSCIT it = indices.begin();
    SSCIT first(it);            // Keep a copy
    
    int idx = *it++;
    // Return an empty expression if the ith subexpression doesn't exist
    if (indices.size() == 0 || i >= theSemTable[idx].iNumVarArgs)
        return new SemStr;

    // First skip the integer and fixed args
    int j;
    for (j = 0; j < theSemTable[idx].totalFixedArgs(); j++)
        it++;

    // Skip the i-1'th variable args
    for (j=0; j < i; j++)
        it = findEndSubExpr(it);

    // Now return the subexpression
    SSCIT last = it;
    last = findEndSubExpr(last);
    SemStr* ret = new SemStr(it, last);
    ret->setType(getSubType(first, type, i));
    return ret;
}

/*==============================================================================
 * FUNCTION:        SemStr::getSubExpr
 * OVERVIEW:        Get the ith subexpression (i.e. the ith variable
 *                  expression). The reference parameter used to restore the
 *                  results ensures that the caller will automatically delete
 *                  the memory used for it.
 * PARAMETERS:      i - the subexpression to return
 *                  result - the expression used to store the subexpression
 * RETURNS:         a reference to the subexpression. This will be the empty
 *                  expression if the i'th expression doesn't exist
 *============================================================================*/
SemStr& SemStr::getSubExpr(int i, SemStr& result) const
{
    SSCIT it = indices.begin();
    SSCIT first(it);            // Keep a copy
    int idx = *it++;
    
    // Return an empty expression if the ith subexpression doesn't exist
    if (indices.size() == 0 || i >= theSemTable[idx].iNumVarArgs) {
        result.indices.clear();
        return result;
    }

    // First skip the integer fixed args
    int j;
    for (j = 0; j < theSemTable[idx].totalFixedArgs(); j++)
        it++;

    // Skip the i-1'th variable args
    for (j=0; j < i; j++)
        it = findEndSubExpr(it);

    // Now return the subexpression
    SSCIT last = it;
    last = findEndSubExpr(last);

    // We need a copy if this == result
    SemStr subExpr(it,last);
    result = subExpr;
    result.setType(getSubType(first, type, i));
    return result;
}

/*==============================================================================
 * FUNCTION:        SemStr::isArrayEqual
 * OVERVIEW:        Compare the array with this semantic string. Only compares
 *                  up to n items. -1 may be used as a wildcard index, but not
 *                  as a wildcard subexpression.
 * PARAMETERS:      n: number of integers to compare
 *                  pInts: the array of integers to compare with this SemStr
 * RETURNS:         True if match
 *============================================================================*/
bool SemStr::isArrayEqual(unsigned n, int* pInts) const
{
    if (indices.size() < n) return false;
    SSCIT it = indices.begin();
    for (unsigned i=0; i < n; i++)
    {
        if (pInts[i] < 0)           // -1 matches any
        {
            it++;                   // This version, can just step to next
            continue;
        }
        if (*it++ != pInts[i]) return false;
    }
    return true;
}

/*==============================================================================
 * FUNCTION:        SemStr::isArraySimilar
 * OVERVIEW:        Compare the given array with this semantic string. Only
 *                  compares up to n indices. Can use -1 to represent a whole
 *                  subexpression, or for one fixed or integer parameter.
 * PARAMETERS:      n: number of items to consider in pInts[]
 *                  pInts: array of integer items to compare against
 * RETURNS:         True if similar
 *============================================================================*/
bool SemStr::isArraySimilar(unsigned n, int pInts[]) const
{
    SSCIT it = indices.begin();
    int i=0;
    return doIsArraySimilar(it, i, n, pInts);
}

/*==============================================================================
 * FUNCTION:        SemStr::doIsArraySimilar
 * OVERVIEW:        Do the work for isArraySimilar
 * PARAMETERS:      it: iterator to current item
 *                  i: current array index
 *                  int n: number of itemts to consider in pInts
 *                  pInts: array of items to consider
 * RETURNS:         True if similar
 *============================================================================*/
bool SemStr::doIsArraySimilar(SSCIT& it, int& i, int n, int pInts[]) const
{
    while (i < n)
    {
        if (pInts[i] < 0)
        {
            // Wildcard subexpression
            it = findEndSubExpr(it);
            i++;
            continue;
        }
        int idx = *it++;
        if (pInts[i++] != idx) return false;
        // Must check integer and fixed args
        int j;
        for (j=0; j < theSemTable[idx].totalFixedArgs(); i++, j++)
        {
            if (i >= n) return true;
            if (pInts[i] == -1) {it++; continue;}
            if (pInts[i] != *it++) return false;
        }
        // Recurse for each sub expression
        for (j=0; j < theSemTable[idx].iNumVarArgs; i++, j++)
        {
            if (!doIsArraySimilar(it, i, n, pInts))
                return false;
        }
    }
    return true;
}

/*==============================================================================
 * FUNCTION:        SemStr::doRecurInfix
 * OVERVIEW:        This is the core of the parenthesis logic. Make some sort of
 *                  decision as to whether or not to use parens this recursion.
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
void SemStr::doRecurInfix(SSCIT& it, string& res, SSCIT& end) const
{
    if (it == end) return;
    // If this expression is complex, emit parens
//  if ((idx != idIntConst) && (cat != cSPECIAL))
    if (theSemTable[*it].iNumVarArgs >= 2)
    {
        res += "(";
        doSprintInfix(it, res, end);
        res += ")";
    }
    else
        doSprintInfix(it, res, end);
}
        
/*==============================================================================
 * FUNCTION:        SemStr::doSprintInfix
 * OVERVIEW:        
 * PARAMETERS:      
 * RETURNS:         <nothing>
 *============================================================================*/
void SemStr::doSprintInfix(SSCIT& it, string& res, SSCIT& end) const
{
    char buf[40];

    if (it == end)
    {
        // An empty semantic string. Don't attempt recursion
        return;
    }
    int idx = *it++;
    CAT cat = theSemTable[idx].cat;

    switch (cat)
    {
        case cSPECIAL:
            // These are now a special case, so to speak
            res += theSemTable[idx].sName + " ";
            break;
        case cOPERATOR:
            switch (theSemTable[idx].iNumVarArgs)
            {
                case 1:
                    // Unary ops: output the string, then the arg, except for
                    // sign extension (when operator goes last)
                    if (idx == idNeg) res += "-";
                    else if (idx != idSignExt)
                        res += theSemTable[idx].sName + " ";
                    doRecurInfix(it, res, end);
                    if (idx == idSignExt)
                        res += "! ";
                    break;

                case 2:
                    // binary ops
                    doRecurInfix(it, res, end);
                    res += " " + theSemTable[idx].sName + " ";
                    doRecurInfix(it, res, end);
                    break;

                case 3:
                    // the two ternary ops
                    if (idx == idTern)
                    {
                        doRecurInfix(it, res, end);
                        res += " ? ";
                        doRecurInfix(it, res, end);
                        res += " : ";
                        doRecurInfix(it, res, end);
                    }
                    else if (idx == idAt)
                    {
                        doRecurInfix(it, res, end);
                        res += "@";
                        doRecurInfix(it, res, end);
                        res += ":";
                        doRecurInfix(it, res, end);
                    }
                    else cerr << "Unknown ternary operator\n";
                    break;

                default:
                    cerr << "cOPERATOR with other than 1-3 var args!\n";
                    break;
            }
            break;

        case cCONSTANT:
        {
            if (idx == idFltConst)
            {
                // There are two ints that represent the double
                int i1 = *it++;
                int i2 = *it++;
                double f = ints2dbl(i1, i2);
                // print it
                sprintf(buf, "%g", f);
                res += buf;
                break;
            }
            else if (idx == idCodeAddr) {
                sprintf(buf, "0x%x", *it++);
                res += buf;
                break;
            }
            // Else assume an integer constant
            // Get the integer
            int iConst = *it++;
            // print it
            sprintf(buf, "%d", iConst);
            res += buf;
            break;
        }

        case cFUNCTION:         // tan(), fsize() etc
        case cOPERATION:        // Register, memory, parameter etc
        {
            if (idx == idSize)
            {
                int iSize = *it++;          // Get the size
                // Recurse for the main variable arg
                doRecurInfix(it, res, end);
                res += "{";
                sprintf(buf, "%d", iSize);
                res += buf;
                res += "}";
                break;
            }

            // Need special handling for a idVar
            if (idx == idVar) {
                res += theSemTable[idVar].sName;
                sprintf(buf,"%d", *it++);
                res += buf;
                break;
            }

            // The operation's name, unless it is a param/expand/temp,
            // in which case we just want to see the fixed parameter
            if (theSemTable[idx].iNumFixedArgs == 0)
                res += theSemTable[idx].sName;

            // Print the integer args (e.g. itof)
            int ii;
            for (ii=0; ii < theSemTable[idx].iNumIntArgs; ii++)
            {
                sprintf(buf, "%d," , *it++);
                res += buf;
            }

            // Now the fixed args, if any
            for (ii=0; ii < theSemTable[idx].iNumFixedArgs; ii++)
            {
                int id = *it++;             // The index to the item
                res += theSemTable[id].sName;
            }
            
            // Recurse to print the variable args, if any
            for (ii=0; ii < theSemTable[idx].iNumVarArgs; ii++)
                doSprintInfix(it, res, end);
            if (cat == cFUNCTION) res += ") ";
            else if ((idx == idMemOf) || (idx == idRegOf) ||
                (idx == idAddrOf))
                res += "]";
            break;
        }

        default:
        {
            ostrstream ost;
            ost << "Error! unknown category " << (int)cat;
            ost << " in SemStr::doSprintInfix(); indices = ";
            ostream_iterator<int> iter(ost, " ");
            copy(indices.begin(),indices.end(),iter);
            error(str(ost));
            theSemTable.print(cout);
        }
    }
}

/*==============================================================================
 * FUNCTION:        SemStr::sprint
 * OVERVIEW:        Print this semantic string to a string object
 * PARAMETERS:      <none>
 * RETURNS:         The resultant string
 *============================================================================*/
string SemStr::sprint() const
{
    SSCIT it = indices.begin();
    SSCIT end = indices.end();
    string res;

    doSprintInfix(it, res, end);
    return res;                 // We have to copy the result
}

/*==============================================================================
 * FUNCTION:        SemStr::print
 * OVERVIEW:        Print an infix representation of the object to the given
 *                  file stream.
 * NOTE:            Never modify this function to emit debugging info; the back
 *                    ends rely on this being clean to emit correct C
 *                    If debugging is desired, use operator<<
 * PARAMETERS:      Output stream to send the output to
 * RETURNS:         <nothing>
 *============================================================================*/
void SemStr::print(ostream& os /*= cout*/) const
{
    string s = sprint();            // Print to a string
    os << s;                        // Print to the output stream
}

/*==============================================================================
 * FUNCTION:        SemStr::printAsC
 * OVERVIEW:        Print an infix representation of the object to the given
 *                  file stream, but convert r[10] to r10
 * NOTE:            Never modify this function to emit debugging info; the back
 *                    ends rely on this being clean to emit correct C
 *                    If debugging is desired, use operator<<
 * PARAMETERS:      Output stream to send the output to
 * RETURNS:         <nothing>
 *============================================================================*/
void SemStr::printAsC(ostream& os /*= cout*/) const
{
    string s = sprint();            // Print to a string
    if ((s.length() >= 4) && (s[1] == '[')) {
        // r[nn]; change to rnn
        s.erase(1, 1);              // '['
        s.erase(s.length()-1);      // ']'
    }
    os << s;                        // Print to the output stream
}

/*==============================================================================
 * FUNCTION:        SemStr::prints
 * OVERVIEW:        Print an infix representation of the object to the given
 *                  file stream, with it's size in <angle brackets>.
 * NOTE:            printt probably obsoletes this now
 * PARAMETERS:      Output stream to send the output to
 * RETURNS:         <nothing>
 *============================================================================*/
void SemStr::prints(ostream& os /*= cout*/) const
{
    print(os);
    os << "<" << dec << getType().getSize() << ">";
}

/*==============================================================================
 * FUNCTION:        SemStr::printt
 * OVERVIEW:        Print an infix representation of the object to the given
 *                  file stream, with it's type in <angle brackets>.
 * PARAMETERS:      Output stream to send the output to
 * RETURNS:         <nothing>
 *============================================================================*/
void SemStr::printt(ostream& os /*= cout*/) const
{
    print(os);
    os << "<" << dec << type.getSize();
    switch (type.getType()) {
        case INTEGER:
            if (type.getSigned())
                        os << "i";              // Integer
            else
                        os << "u"; break;       // Unsigned
        case FLOATP:    os << "f"; break;
        case DATA_ADDRESS: os << "pd"; break;   // Pointer to Data
        case FUNC_ADDRESS: os << "pc"; break;   // Pointer to Code
        case VARARGS:   os << "v"; break;
        case BOOLEAN:   os << "b"; break;
        case UNKNOWN:   os << "?"; break;
        case TVOID:     break;
    }
    os << ">";
}

/*==============================================================================
 * FUNCTION:        operator<<
 * OVERVIEW:        Output operator for SemStr
 * PARAMETERS:      os: output stream to send to
 *                  s: string to print to the stream
 * RETURNS:         copy of os (for concatenation)
 *============================================================================*/
ostream& operator<<(ostream& os, const SemStr& s)
{
#if 1
    // Useful for debugging, but can clutter the output
    s.printt(os);
#else
    s.print(os);
#endif
    return os;
}

/*==============================================================================
 * FUNCTION:        substReg
 * OVERVIEW:        substitute the given register number (the whole r[n]) with
 *                  the given replacement semantic string
 * PARAMETERS:      iReg: number of the register, e.g. 8 for r[8]
 *                  ssRepl: ptr to replacement semantic string
 * RETURNS:         True if any replacements made
 *============================================================================*/
// Replace all occurrences of `r[ int iReg' with ssRepl
bool SemStr::substReg(int iReg, const SemStr* pssRepl)
{
    SemStr search;
    search.push(idRegOf); search.push(idIntConst);
    search.push(iReg);
    return searchReplaceAll(search, *pssRepl);
}

/*==============================================================================
 * FUNCTION:        SemStr::substIndex
 * OVERVIEW:        Replace item at index i with iRepl
 * PARAMETERS:      i: the index (0 for first)
 *                  iRepl: replacement item
 * RETURNS:         <nothing>
 *============================================================================*/
void SemStr::substIndex(unsigned i, int iRepl)
{
    assert (i < indices.size());
    SSIT it = indices.begin();
    // Obtain an interator to the index in question
    for (unsigned ii = 0; ii < i; ii++) it++;
    *it = iRepl;
}   

/*==============================================================================
 * FUNCTION:        SemStr::smartCompare
 * NOTE:            Not complete! See below
 * OVERVIEW:        
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
// NOTE: this function is not complete. There are some problems:
// . Items are added to liWild in a somewhat arbiary order
// . Items are added to liWild when the comparison fails on the other
//      term of the comparison, so that would require adding in steps
//      or copying the list at some point. Ugly.
// At this stage, custom comparison code is a better solution.
// Perhaps later these problems will be overcome in an acceptable manner.
// ---
// Compare two strings, taking account of commutative arguments
// If bWild is true, add actual wildcard values to the list in the
// order that they are encountered (recursive descent order)
bool SemStr::smartCompare(const SemStr& other, bool bWild,
    list<int>& liWilds)
{
    // If run out of the right hand side, call that a match
    // This allows comparison with incomplete templates
    if (other.indices.size() == 0) return true;
    // If run out on the left, though, call that a mismatch
    if (indices.size() == 0) return false;
    if (indices.front() != other.indices.front()) return false;
    int idx = indices.front();
    switch (idx)
    {
        // These operators take two arguments, and are commutative
        case idPlus:
        case idMult:
        case idMults:
        case idAnd:
        case idOr:
        case idNotEqual:
        case idBitAnd:
        case idBitOr:
        case idBitXor:

        {
            SemStr* pExp11 = getSubExpr(0);
            SemStr* pExp12 = getSubExpr(1);
            SemStr* pExp21 = other.getSubExpr(0);
            SemStr* pExp22 = other.getSubExpr(1);
            bool bRet;
            bRet = ((pExp11->smartCompare(*pExp21, bWild, liWilds) &&
                     pExp12->smartCompare(*pExp22, bWild, liWilds)) ||
                    (pExp11->smartCompare(*pExp22, bWild, liWilds) &&
                     pExp12->smartCompare(*pExp21, bWild, liWilds)));
            delete pExp11;
            delete pExp12;
            delete pExp21;
            delete pExp22;
            return bRet;
        }

        default:
        {
            // Compare without regard for commutibility
            int i;
            for (i=0; i < theSemTable[idx].totalFixedArgs(); i++)
            {
                if ((getIndex(i) != other.getIndex(i)) ||
                    (!bWild && (other.getIndex(i) != -1)))
                    return false;
                else
                    if (bWild) liWilds.push_back(getIndex(i));
            }
            for (i=0; i < theSemTable[idx].iNumVarArgs; i++)
            {
                if (!getSubExpr(i)->smartCompare(*other.getSubExpr(i),
                    bWild, liWilds))
                        return false;
            }
            return true;
        }
    }
}

/*==============================================================================
 * FUNCTION:        SemStr::
 * OVERVIEW:        
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
// As above, for an array of ints
bool SemStr::smartCompare(int n, const int arr[], list<int>& liWilds)
{
    SemStr* pTemp = new SemStr;
    pTemp->pushArr(n, arr);
    bool bRet = smartCompare(*pTemp, true, liWilds);
    delete pTemp;
    return bRet;
}

/*==============================================================================
 * FUNCTION:        SemStr::findEndSubExpr
 * OVERVIEW:        Find the end of the subexpression starting at iterator it;
 *                  return an iterator (possibly end()) that is just past the
 *                  end of this subexpression
 * PARAMETERS:      it: iterator to the start of a subexpression
 * RETURNS:         An iterator (possibly end()) to one past the end of this
 *                  subexpression
 *============================================================================*/
// This is the const version.
SSCIT SemStr::findEndSubExpr(SSCIT it) const
{
    if (it == indices.end()) return it;
    int idx = *it++;
    int i;
    for (i=0; i < theSemTable[idx].totalFixedArgs(); i++) it++;
    for (i=0; i < theSemTable[idx].iNumVarArgs; i++)
        it = findEndSubExpr(it);
    return it;
}

/*==============================================================================
 * FUNCTION:        SemStr::findEndSubExpr
 * OVERVIEW:        Find the end of the subexpression starting at iterator it;
 *                  return an iterator (possibly end()) that is just past the
 *                  end of this subexpression
 * PARAMETERS:      it: iterator to the start of a subexpression
 * RETURNS:         An iterator (poss end()) to one past the end of this
 *                  subexpression
 *============================================================================*/
// Ugh: looks like I need a non const version (takes non const iterators).
// This is really stupid! I can't seem to const cast an iterator.
SSIT SemStr::findEndSubExpr(SSIT it) const
{
    //if (it == indices.end()) return it;   // Why doesn's this work?
    int idx = *it++;
    int i;
    for (i=0; i < theSemTable[idx].totalFixedArgs(); i++) it++;
    for (i=0; i < theSemTable[idx].iNumVarArgs; i++)
        it = findEndSubExpr(it);
    return it;
}

/*==============================================================================
 * FUNCTION:        SemStr::Accumulate
 * OVERVIEW:        This method creates an expression that is the sum of all
 *                  expressions in a multiset. The sum is appended to this
 *                  expression and joined with a given operator if both the
 *                  original expression and the sum are not empty.
 *                  E.g. if this expression is already %sp+4 and we
 *                  are given the list <4,r[8],m[14]> and the + operator, then
 *                  the resulting expression is %sp+4+4+r[8]+m[14].
 * NOTE:            Don't rename to accumulate; clash with library function
 * PARAMETERS:      exprs - a multiset of expressions
 *                  binOper - the binary operator used to join the
 *                    accumulation with this expression if both are
 *                    non empty
 *                  unOper - the unary operator prepended to the
 *                    accumulation if this expression (the current accumulation)
 *                    is empty, or -1 if there is no operator needed
 * RETURNS:         a reference to this expression
 *============================================================================*/
SemStr& SemStr::Accumulate(multiset<SemStr>& exprs, int binOper, int unOper)
{
    // General algorithm: create a SemStr called terms with the sum of all the
    // given terms (e.g. + + d e f). Append these to the existing expression
    // (e.g. + + a b c) to form the new expression (- + + a b c + + d e f),
    // i.e. a+b+c - d+e+f => (a+b+c) - (d+e+f)

    int origSize = indices.size();
    SemStr terms;

    // Add the terms
    for (multiset<SemStr>::iterator it = exprs.begin(); it != exprs.end();
      it ++) {
        terms.prep(idPlus);
        terms.pushSS(&*it);
    }
    // Remove the extra operator that may have been prepended
    if (exprs.size() != 0)
        terms.popFirst();

    if (terms.len() == 0)
        return *this;

    if (origSize != 0) {

        // We are appending to a non-empty expression so use the
        // given binary operator
        assert (theSemTable[binOper].cat == cOPERATOR &&
            theSemTable[binOper].iNumVarArgs == 2);

        this->prep(binOper);
    }
    else if (unOper >= 0 && unOper < (int)theSemTable.size()) {
        // We are appending to an empty expression so use the
        // given unary operator if is a valid index into the semTable
        assert (theSemTable[unOper].cat == cOPERATOR &&
            theSemTable[unOper].iNumVarArgs == 1);

        this->prep(unOper);
    }

    // Add the terms now.
    this->pushSS(&terms);

    return *this;
}

/*==============================================================================
 * FUNCTION:        SemStr::partitionTerms
 * OVERVIEW:        Takes an expression consisting on only + and - operators and
 *                  partitions its terms into positive non-integer fixed terms,
 *                  negative non-integer fixed terms and integer terms. For
 *                  example, given:
 *                     %sp + 108 + n - %sp - 92
 *                  the resulting partition will be:
 *                     positives = { %sp, n }
 *                     negatives = { %sp }
 *                     integers  = { 108, -92 }
 * NOTE:            The outputs have to be multisets; consider a=b+b
 * PARAMETERS:      positives - the multiset of positive terms
 *                  negatives - the multiset of negative terms
 *                  integers - the set of integer terms
 *                  negate - determines whether or not to negate the whole
 *                    expression
 * RETURNS:         <nothing>
 *============================================================================*/
void SemStr::partitionTerms(multiset<SemStr>& positives,
    multiset<SemStr>& negatives, vector<int>& integers, bool negate) const
{

    // Use a stack to record if we are on a negative branch of an expression.
    stack<int> negation;
    negation.push(negate ? -1 : 1);
    
    SSCIT expr = indices.begin();
    while (expr != indices.end()) {
        
        switch(*expr) {
        case idPlus:
        {
            int top = negation.top();
            negation.pop();

            negation.push(top);
            negation.push(top);

            // Skip the operator
            expr++;

            break;
        }
        case idMinus:
        {
            int top = negation.top();
            negation.pop();

            negation.push(-top);
            negation.push(top);

            // Skip the operator
            expr++;

            break;
        }
        case idIntConst:

            expr++;         // Move past the idIntConst to the integer itself

            integers.push_back(*(expr) * negation.top());
            negation.pop();

            // Jump over the integer constant
            expr++;

            break;
            
        default:
        {
            bool negative = (negation.top() == -1);
            negation.pop();

            // Simplify any subexpressions
            SemStr subExpr;
            int idx = *expr;
            int i;
            for (i = 0; i < theSemTable[idx].totalFixedArgs()+1; i++) {
                subExpr.push(*expr);
                expr++;
            }
            for (i = 0; i < theSemTable[idx].iNumVarArgs; i++) {
                SSCIT endExpr = findEndSubExpr(expr);
                SemStr subSubExpr(expr, endExpr, type);
                subSubExpr.simplify();
                subExpr.pushSS(&subSubExpr);
                expr = endExpr;
            }

            if (negative)
                negatives.insert(subExpr);
            else
                positives.insert(subExpr);
        }
        }

    }
}

/*==============================================================================
 * FUNCTION:        SemStr::simplifyArith
 * OVERVIEW:        This method simplifies an expression consisting of + and - a
 *                  the top level. For example, (%sp + 100) - (%sp + 92) will be
 *                  simplified to 8.
 * PARAMETERS:      result - the expression for storing the result (can be equal
 *                    to *this
 * RETURNS:         a reference to result
 *============================================================================*/
SemStr& SemStr::simplifyArith(SemStr& result)
{
    // Eliminate the trivial case of no string
    if (indices.size() == 0) {
        result = *this;
        return result;
    }

//ostrstream r;
//r << "\nbefore `" << *this;
    // Partition this expression into positive non-integer terms, negative
    // non-integer terms and integer terms.
    // Note that we need multisets here, in case we have a=b+b for example
    multiset<SemStr> positives;
    multiset<SemStr> negatives;
    vector<int> integers;
    partitionTerms(positives,negatives,integers,false);

    // Now reduce these multisets by cancelling pairs
    multiset<SemStr>::iterator pos_it = positives.begin();
    multiset<SemStr>::iterator neg_it = negatives.begin();
    while (pos_it != positives.end() && neg_it != negatives.end()) {
        if (*pos_it < *neg_it)
            pos_it++;
        else if (*neg_it < *pos_it)
            neg_it++;
        else {
            // A positive and a negative that are equal; therefore they cancel
            /* Doug's code appears to be in error here; after an erase,
              iterators are invalid. But only the erased items are
              affected, it seems.
            positives.erase(pos_it);
            negatives.erase(neg_it);
            pos_it++;
            neg_it++; */
            multiset<SemStr>::iterator temp = pos_it++;
            positives.erase(temp);
            temp = neg_it++;
            negatives.erase(temp);
        }
    }

    // Summarise the set of integers to a single number.
    int sum = accumulate(integers.begin(),integers.end(),0);

    // Now put all these elements back together in result
    result.indices.clear();

    // Add the non-constant positives
    result.Accumulate(positives,idPlus,-1);

    // Add the non-constant negatives
    result.Accumulate(negatives,idMinus,idNeg);

    // Add the constant if it is non-zero
    if (sum < 0) {
        if (result.len() != 0) {
            result.prep(idMinus);
            result.push(idIntConst);
            result.push(-sum);
        }
        else {
            result.push(idIntConst);
            result.push(sum);
        }
    }
    else if (sum > 0) {
        if (result.len() != 0)
            result.prep(idPlus);
        result.push(idIntConst);
        result.push(sum);
    }
    else if (result.len() == 0) {
        result.push(idIntConst);
        result.push(0);
    }
//r << "'\n after `" << result << "'\n";
//cerr << str(r);
    return result;
}

#if 0
/*==============================================================================
 * FUNCTION:        SemStr::evaluateLogical
 * OVERVIEW:        This is a logical expression with one free variable (that
 *                  may occur more than once. E.g.:
 *                     n % 4 == 0 && n > 0
 *                  Given a value for n, this method evaluates the predicate and
 *                  returns the result.
 * PARAMETERS:      val - a value for the free variable in this predicate
 *                  begin - start of a subexpression
 *                  end - one past the end of a subexpression
 * RETURNS:         the predicate is satisfied for the given value
 *============================================================================*/
bool SemStr::evaluateLogical(int val,SSIT begin = NULL, SSIT end = NULL)
{
    // Give values to the iterators if this is the top level expression as well
    // as substituting the value for the free variable.
    if (begin == NULL) {
        begin = indices.begin();
        end = indices.end();

        // Find all idParam subexpressions and replace them with an integer
        // constant expression (assumes all idParams will refer to exactly the
        // same variable).
        int searchArr[] = {idParam};
        int replaceArr[] = {idIntConst,val};
        SemStr search(searchArr,searchArr+(sizeof(searchArr)/sizeof(int)));
        SemStr replace(replaceArr,replaceArr+(sizeof(replaceArr)/sizeof(int)));
        SearchReplaceAll(search,replace);
    }

    SSIT expr = begin;
    int exprType = *expr;
    switch (exprType) {
        case idAnd:
        case idOr:
        {
            SSIT LHS_start = ++expr;
            SSIT LHS_end   = findEndSubExpr(LHS_start);
            SSIT RHS_start = LHS_end;
            SSIT RHS_end   = end;

            if (exprType == idAnd)
                return EvaluateLogical(val,LHS_start,LHS_end) &&
                    EvaluateLogical(val,RHS_start,RHS_end);
            else
                return EvaluateLogical(val,LHS_start,LHS_end) ||
                    EvaluateLogical(val,RHS_start,RHS_end);
        }
        case idEquals:
        case idLess:
        case idGreater:
        case idLessEq:
        case idGreaterEq:
        case idNotEqual:
        {
            // Simplify the two operands to integers
            SSCIT LHS_start = ++expr;
            SSCIT LHS_end   = findEndSubExpr(LHS_start);
            SSCIT RHS_start = LHS_end;
            SSCIT RHS_end   = end;

            SemStr LHS(LHS_start,LHS_end);
            SemStr RHS(RHS_start,RHS_end);
            LHS.Simplify();
            RHS.Simplify();

            assert(LHS.getFirstIdx() == idIntConst);
            assert(RHS.getFirstIdx() == idIntConst);

            int lhs_val = LHS.getSecondIdx();
            int rhs_val = RHS.getSecondIdx();

            switch (exprType) {
            case idEquals:    return lhs_val == rhs_val;
            case idLess:      return lhs_val <  rhs_val;
            case idGreater:   return lhs_val >  rhs_val;
            case idLessEq:    return lhs_val <= rhs_val;
            case idGreaterEq: return lhs_val >= rhs_val;
            case idNotEqual:  return lhs_val != rhs_val;
            }
            
        }
        default:
            error("Can't evaulate non-boolean expression");
    }
    return false;
}
#endif

/*==============================================================================
 * FUNCTION:        SemStr::doSimplify
 * OVERVIEW:        Do the work of simplifying this SS.
 * PARAMETERS:      it: reference to the iterator to current index
 *                  end: iterator to one past the end of this SS
 * RETURNS:         True if make a change
 *============================================================================*/
#if DEBUG_SS_SIMP
static int indent = 0;
#endif
bool SemStr::doSimplify(SSIT& it, SSIT end)
{
    if (it == end) return false;
#if DEBUG_SS_SIMP
  {  
    indent++;
    int sp, num_int_param = 0;
    for (sp=0; sp < indent; sp++) cerr << "  ";
    printPrefix(cerr);
    cerr << endl;
    for (sp=0; sp < indent; sp++) cerr << "  ";
    ostrstream ost;
    for (SSIT itz=indices.begin(); itz != it; itz++) {
        // Note: the following simple logic gets out of step due to the close
        // brackets and close quotes
        if (num_int_param) {
            ost << *itz << " ";
            num_int_param--;
        } else {
            ost << theSemTable[*itz].sName << " ";
            num_int_param = theSemTable[*itz].iNumIntArgs;
        }
    }
    int n = strlen(str(ost));
    for (int z=0; z < n; z++)
        cerr << " ";
    cerr << "^\n";
  }
#endif
    bool bSimpLevel = false;    // True if do simplify this level
    bool bSimp;                 // True if do simplify this loop
    bool bMod;                  // True if simplified at this or lower level
    do
    {
        int idx = *it++;
        int i;
        bSimp = false;          // Not yet modified this loop
        for (i=0; i < theSemTable[idx].totalFixedArgs(); i++) it++;
        SSIT it2 = it;          // Points just after idx and all fixed and
                                // integer args

        // -int or not int, or int{size}
        if ((theSemTable[idx].iNumVarArgs == 1) && (it != end) &&
            (*it == idIntConst))
        {
            switch (idx)
            {
                case idNeg:
                case idNot:
                case idLNot:
                {
                    // We want to remove the idx
                    it = indices.erase(--it);
                    // it now points to the idIntConst
                    it++;           // Now to the integer const
                    switch (idx)
                    {
                        case idNeg:
                            *it = 0 - *it; break;
                        case idNot:
                            *it = ~*it; break;
                        case idLNot:
                            *it = !*it; break;
                        }
                    it++;           // Now past the integer itself
                    bSimp = true;   // Mod this level
                    break;
                }
                case idSize:
                {
                    // int{size}; this can simplify to just int
                    // Can happen in 68k (and probably others)
                    SSIT itStart = it;
                    advance (itStart, -2);      // Skip idSize, and size param
                    indices.erase(itStart, it); // Remove these
                    ++++it;                     // Point past int const
                    bSimp = true;
                    break;
                }
                default:
                        // Optimisation does not apply; leave it alone
                    ++++it;         // Point past the integer itself
            }
        }

        // int op int (i.e. binary expression between 2 constants)
        else if ((theSemTable[idx].iNumVarArgs == 2) &&
            (it2 != end) && (*it2++ == idIntConst) &&
            (++it2 != end) && (*it2 == idIntConst))
        {
            switch (idx)
            {
                case idPlus:
                case idMinus:
                case idMult:
                case idDiv:
                case idMults:
                case idDivs:
                case idMod:
                case idMods:
                case idShiftL:
                case idShiftR:
                case idShiftRA:
                case idBitOr:
                case idBitAnd:
                case idAnd:
                case idOr:
                case idEquals:
                case idNotEqual:
                case idLess:
                case idGtr:
                case idLessEq:
                case idGtrEq:
                case idLessUns:
                case idGtrUns:
                case idLessEqUns:
                case idGtrEqUns:

                {
                    // We want to read the two constants, then erase the idx
                    // and two indices after it
                    SSIT it3 = it;      // it3 points to first idIntConst
                    int i1 = *++it;     // it points to fist integer
                    int i2 = *++it2;
                    it = indices.erase(--it3, ++it);
                    it++;                   // Point to 2nd integer
                    switch (idx)
                    {
                        case idPlus:    *it = i1 + i2; break;
                        case idMinus:   *it = i1 - i2; break;
                        case idDiv:     *it = (int)
                                            ((unsigned)i1 / (unsigned)i2);
                        case idDivs:    *it = i1 / i2; break;
                        case idMod:     *it = (int)
                                            ((unsigned)i1 % (unsigned)i2);
                        case idMods:    *it = i1 % i2; break;
                        case idMult:    *it = (int)
                                            ((unsigned)i1 * (unsigned)i2);
                                        break;
                        case idMults:   *it = i1 * i2; break;
                        case idShiftL:  *it = i1 << i2; break;
                        case idShiftR:  *it = i1 >> i2; break;
                        case idShiftRA: *it = (i1 >> i2) |
                                            (((1 << i2) -1) << (32 - i2));
                                        break;
                        case idBitOr:       *it = i1 | i2; break;
                        case idBitAnd:      *it = i1 & i2; break;
                        case idAnd:         *it = i1 && i2; break;
                        case idOr:          *it = i1 || i2; break;
                        case idEquals:      *it = (i1 == i2); break;
                        case idNotEqual:    *it = (i1 != i2); break;
                        case idLess:        *it = (i1 <  i2); break;
                        case idGtr:         *it = (i1 >  i2); break;
                        case idLessEq:      *it = (i1 <= i2); break;
                        case idGtrEq:       *it = (i1 >= i2); break;
                        case idLessUns:     *it = ((unsigned)i1 < (unsigned)i2);
                                        break;
                        case idGtrUns:      *it = ((unsigned)i1 > (unsigned)i2);
                                        break;
                        case idLessEqUns:   *it = ((unsigned)i1 <=(unsigned)i2);
                                        break;
                        case idGtrEqUns:    *it = ((unsigned)i1 >=(unsigned)i2);
                                        break;
                    }
                    it++;               // Past the whole thing
                    bSimp = true;       // Modified this level
                    break;
                }
                default:
                    // Optimisation does not apply; step the iterator past this
                    // subexpression
                    // Past the two `int' `num' pairs
                    advance(it, 4);
                    //++++it;           // Past the first idIntConst and integer
                    //++++it;           // Past the 2nd idIntConst and integer
            }
        }
        else if ((theSemTable[idx].iNumVarArgs == 2) &&
          ((idx == idBitXor) || (idx == idMinus))) {
            // exp ^ exp   and  exp - exp
            // where the expressions are the same
            if (*getSubExpr(0) == *getSubExpr(1)) {
                // In each case, we can replace the whole thing with const 0
                SSIT itStart = it;
                it = findEndSubExpr(it);
                it = findEndSubExpr(it);
                // Remove the 2 subexpressions
                it = indices.erase(itStart, it);
                // Replace the operator (^ or -) with int
                *--it = idIntConst;
                // Insert 0 to complete the constant
                indices.insert(++it, 0);
            }
        }
        else
        {
            // Can't simplify one or two integer constants to a single one.
            // Might want to commute to put an integer constant on the
            // right hand side, however.
            it2 = it;
            if ((theSemTable[idx].iNumVarArgs == 2) &&
                (it2 != end) && (*it2++ == idIntConst))
            {
                switch (idx)
                {
                    case idPlus:
                    case idMult:
                    case idMults:
                    case idBitOr:
                    case idBitAnd:
                    case idAnd:
                    case idOr:
                    {
                        // Move the idIntConst and the integer to the
                        // other side of the next expression
                        // We use splice to just change a few pointers
                        // it2 is pointing to the integer const
                        SSIT itEnd = findEndSubExpr(++it2);
                        // it2 now points past the integer constant
                        indices.splice(itEnd, indices, it, it2);
                        // Note that it and it2 are swapped
                        it = it2;
                    }
                }
            }

            // Check for exp + 0  or  exp - 0  or  exp | 0 or exp OR 0
            if (idx == idPlus || idx == idMinus || idx == idBitOr ||
              (idx == idOr) )
            {
                SSIT itStart = it;
                SSIT itEnd = findEndSubExpr(it);
                SSIT itConst = itEnd;
                if (itEnd != indices.end() && *itEnd == idIntConst &&
                    *++itEnd == 0)
                {
                    // Remove the operator. Leave "it" at the start of
                    // the new subexpression
                    it = indices.erase(--itStart);
                    // Remove the idIntConst and the 0
                    indices.erase(itConst, ++itEnd);
                    bSimp = true;
                    idx = *it++;
                }
            }
                    
            // Check for exp * 0  or exp & 0  or exp AND 0
            if (idx == idMult || idx == idMults || idx == idBitAnd ||
              (idx == idAnd) )
            {
                SSIT itStart = it;
                SSIT itEnd = findEndSubExpr(it);
                SSIT itConst = itEnd;
                if (itEnd != indices.end() && *itEnd == idIntConst &&
                    *++itEnd == 0)
                {
                    // Remove the operator and the subexpression, leaving just
                    // the int 0
                    it = indices.erase(--itStart, itConst);
                    bSimp = true;
                    idx = *it++;
                }
            }
                    
            // Check for exp * 1  or exp AND 1
            if (idx == idMult || idx == idMults || (idx == idAnd) )
            {
                SSIT itStart = it;
                SSIT itEnd = findEndSubExpr(it);
                SSIT itConst = itEnd;
                if (itEnd != indices.end() && *itEnd == idIntConst &&
                    *++itEnd == 1)
                {
                    // Remove the operator. Leave "it" at the start of
                    // the new subexpression
                    it = indices.erase(--itStart);
                    // Remove the idIntConst and the 1
                    indices.erase(itConst, ++itEnd);
                    bSimp = true;
                    idx = *it++;
                }
            }
                    
            // Next change: if we have x + k where k is a negative
            // integer constant, change this to x - p where p = -k 
            if (idx == idPlus)
            {
                SSIT itEnd = findEndSubExpr(it);
                if (itEnd != indices.end() && *itEnd == idIntConst &&
                    *++itEnd < 0)
                {
                    *(--it) = idMinus;
                    it++;
                    *itEnd = -*itEnd;
                    bSimp = true;
                }
            }
                    
            // Check for [exp] << k where k is a positive integer const
            if (idx == idShiftL)
            {
                SSIT itEnd = findEndSubExpr(it);
                int k;
                if (itEnd != indices.end() && *itEnd == idIntConst &&
                  ((k = *++itEnd), ((k > 0) && (k < 32)))) {
                    *(--it) = idMult;
                    it++;
                    *itEnd = 1 << *itEnd;
                }
            }

// Doing this optimisation here prevents some %afp transformations
// e.g. pa-risc LDO -64(fp), r19; FLDWS 4(r19), fr23
#if 0
            // Check for a[ m[ any ] ]
            if ((idx == idAddrOf) && (it != end) && (*it == idMemOf))
            {
                SSIT itStart = it;
                indices.erase(--itStart, ++it);
                idx = *it++;
                bSimp = true;
            }

            // Check for a[ size{ m[ any ]}]
            // Can happen in 68k (and probably others)
            if ((idx == idAddrOf) && (it != end) && (*it == idSize))
            {
                SSIT itStart = it;
                ++it;                       // The size itself, e.g. 32
                int nxt = *++it;            // Whatever comes after the size
                if (nxt != idMemOf)
                {
                    // This is highly unusual, and is probably an error.
                    // For now, silently disregard
                    advance(it, -2);
                }
                else
                {
                    indices.erase(--itStart, ++it);
                    idx = *it++;
                    bSimp = true;
                }
            }
#endif

        }
        bMod = bSimp;           // bMod true if mod at this or lower level
        bSimpLevel |= bSimp;    // bSimpLevel if simplified this level
        // Recurse for all the variable arguments. (Integer and fixed args
        // are skipped way back at the beginning of the procedure)
        for (i=0; i < theSemTable[idx].iNumVarArgs; i++)
        {
            bMod |= doSimplify(it, end);
        }
        if (bMod) it = indices.begin();     // Restore iterator to start
    } while (bMod);             // If mod at this or lower level, redo this
#if DEBUG_SS_SIMP
    if (--indent == 0) {cerr << "Final => "; printPrefix(cerr); cerr << endl;}
#endif
    return bSimp;
}

/*==============================================================================
 * FUNCTION:        SemStr::simplify
 * OVERVIEW:        Simplify by using constant folding, and also commuting if
 *                  possible to always place integer constants on the right.
 *                  The adding of negative constants is also changed to
 *                  subtracting positive constants. Also changes << k to a
 *                  multiply
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
void SemStr::simplify()
{
    SSIT it = indices.begin();
    doSimplify(it, indices.end());

    // Now simplify top level +'s and -'s
    simplifyArith(*this);
}

/*==============================================================================
 * FUNCTION:        SemStr::doSimplifyAddr
 * OVERVIEW:        Do the work of ADDR simplifying this SS.
 * PARAMETERS:      it: reference to the iterator to current index
 *                  end: iterator to one past the end of this SS
 * RETURNS:         Nothing
 *============================================================================*/
void SemStr::doSimplifyAddr(SSIT& it, SSIT end)
{
    int idx = *it++;

    // Check for a[ m[ any ] ]
    if ((idx == idAddrOf) && (it != end) && (*it == idMemOf)) {
        SSIT itStart = it;
        indices.erase(--itStart, ++it);
        idx = *it++;
    }

    // Check for a[ size{ m[ any ]}]
    // Can happen in 68k (and probably others)
    if ((idx == idAddrOf) && (it != end) && (*it == idSize)) {
        SSIT itStart = it;
        ++it;                       // The size itself, e.g. 32
        int nxt = *++it;            // Whatever comes after the size
        if (nxt != idMemOf)
        {
            // This is highly unusual, and is probably an error.
            // For now, silently disregard
            advance(it, -2);
        }
        else
        {
            indices.erase(--itStart, ++it);
            idx = *it++;
        }
    }

    // Skip the integer and fixed args
    for (int j=0; j < theSemTable[idx].totalFixedArgs(); j++) it++;
    // Recurse into subexpressions
    for (int i=0; i < theSemTable[idx].iNumVarArgs; i++) {
        doSimplifyAddr(it, end);
    }

}
/*==============================================================================
 * FUNCTION:        SemStr::simplifyAddr
 * OVERVIEW:        Simplify by combining idAddrOf with idMemOf, if possible
 * NOTE:            This used to be part of simplify(), but was separated out
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
void SemStr::simplifyAddr()
{
    SSIT it = indices.begin();
    doSimplifyAddr(it, indices.end());
}

/*==============================================================================
 * FUNCTION:        SemStr::search
 * OVERVIEW:        Finds a subexpression within this expression and puts a copy
 *                  of it in a given expression.
 *                  The given search expression can be incomplete.
 *                  For example, a search pattern of {idParam} can be given to
 *                  find the first 'parameter' expression, regardless of the
 *                  actual parameter (or variable) that it refers to. This
 *                  subexpression will be put into result.
 * PARAMETERS:      search - the expression to search for
 *                  result - the found expression
 *                  typeSens - if true, only match if search expression's type
 *                    equals the current subexpression's type
 * RETURNS:         the subexpression was found
 *============================================================================*/
bool SemStr::search(const SemStr& search, SemStr& result, bool typeSens) const
{
    if (indices.size() == 0)
        return false;

    SSCIT begin = indices.begin();
    SSCIT end;
    if (doSearch(search, begin, end, typeSens, type)) {
        result.indices.clear();
        result.indices.insert(result.indices.begin(),begin,end);
        result.setTypes(*this);     // Transfer this SemStr's types
        return true;
    }
    else
        return false;
}

/*==============================================================================
 * FUNCTION:        SemStr::searchAll
 * OVERVIEW:        Finds all instances of a given subexpression within this
 *                  expression and adds them to a given list.
 *                  The given search expression can be incomplete.
 *                  For example, a search pattern of {idParam} can be given to
 *                  find the first 'parameter' expression, regardless of the
 *                  actual parameter (or variable) that it refers to. This
 *                  subexpression will be put into result.
 *                  typeSens - if true, only search expressions with the same
 *                    type as this SemStr are considered to match
 * PARAMETERS:      search - the expression to search for
 *                  result - the list of found expressions
 *                    The list is a list of pointers to copies of the originals
 * RETURNS:         True if the subexpression was found
 *============================================================================*/
bool SemStr::searchAll(SemStr& search, list<SemStr*>& result,
    bool typeSens /* = false */)
{
    if (indices.size() == 0)
        return false;

    SSIT begin = indices.begin();
    ResultInfoList resInfo;

    if (doSearchAll(search, begin, resInfo, typeSens, type)) {

        for (ResultInfoIt it = resInfo.begin(); it != resInfo.end(); it++) {
            SemStr* pRes = new SemStr(it->first, it->second);
            pRes->setTypes(it->type);          // Transfer type to result
            result.push_back(pRes);
        }
        return true;
    }
    return false;
}

/*==============================================================================
 * FUNCTION:        SemStr::searchReplace
 * OVERVIEW:        Finds a subexpression within this expression and replaces it
 *                  with the supplied replacement expression. This method has
 *                  the same search semantics as the Search method.
 * PARAMETERS:      search - the expression to search for
 *                  replace - the expression to replace the first found instance
 *                    of the search expression with
 *                  typeSens - if true, search is sensitive to types
 * NOTES:           It is diffucult to make parameter search const, though it
 *                    should be
 * RETURNS:         true if the expression was found and successfully replaced
 *============================================================================*/
bool SemStr::searchReplace(SemStr& search, const SemStr& replace,
    bool typeSens)
{
    if (indices.size() == 0)
        return false;

    SSIT begin = indices.begin();
    SSIT end;

    if (doSearch(search, begin, end, typeSens, type)) {

        SSIT insertPos = indices.erase(begin, end);
        indices.insert(insertPos,replace.indices.begin(),replace.indices.end());
        return true;
    }
    else
        return false;
}

/*==============================================================================
 * FUNCTION:        SemStr::searchReplaceAll
 * OVERVIEW:        Searches for a given subexpression within this expression
 *                  and replaces all instances of it with a supplied replacement
 *                  expression.
 *                  Has been modified to cope with replacement strings contain-
 *                  ing the search string as a prefix, e.g. replacing a with abc
 * PARAMETERS:      search - the expression to search for
 *                  replace - the expression to replace the first found instance
 *                    of the search expression with
 *                  typeSens - if true, only search expressions with the same
 *                    type as this SemStr are considered to match
 * NOTES:           It is diffucult to make parameter search const, though it
 *                    should be
 *                  It is necessary to use doSearchAll and mess with the list
 *                    of iterators, because otherwise you won't solve these two
 *                    problems:
 *                      1) Ability to replace a with abc (and not infite loop)
 *                      2) Replace a of (a*b)+(a*c) correctly. After the first
 *                        replacement, if you call doSearch with an iterator
 *                        to after the insertion point, it never gets to (a*c)
 *                        since that's not a subexpression *starting at the
 *                        first a*. Can't just "keep going past the end of the
 *                        current subexpression" any more, since that won't
 *                        handle types of subexpressions
 * RETURNS:         true if at least one instance of the search expression was
 *                  found and replaced
 *============================================================================*/
bool SemStr::searchReplaceAll(SemStr& search, const SemStr& replace,
    bool typeSens /* = false */)
{
    if (indices.size() == 0)
        return false;

    SSIT begin = indices.begin();
    bool result = false;
    ResultInfoList resInfo;

    if (doSearchAll(search, begin, resInfo, typeSens, type)) {
        for (ResultInfoIt it = resInfo.begin(); it != resInfo.end(); it++) {
            SSIT ip = indices.erase(it->first, it->second);
            indices.insert(ip, replace.indices.begin(), replace.indices.end());
            result = true;
        }
    }

#if 0       // Did not handle changing a of (a*b)+(a*c)
    SSIT end;
    while (doSearch(search, begin, end, typeSens, type)) {
        SSIT insertPos = indices.erase(begin, end);
        indices.insert(insertPos,replace.indices.begin(),replace.indices.end());
        // Continue searching *after* the insert position
        begin = insertPos;
        result = true;
    }
#endif

    return result;
}

/*==============================================================================
 * FUNCTION:        SemStr::doSearch
 * OVERVIEW:        Finds a match of a subexpression and returns an
 *                  iterator to the start and end of that subexpression.
 *                  The search expression may contain wild values (i.e. indicies
 *                  set to WILD) but these expressions will only match if they
 *                  are of exactly the same length and all the non-wild indices
 *                  match.
 * PARAMETERS:      search - the expression to search for
 *                  begin - where to start searching from in this expression
 *                  end - set to the end of the search expression in this
 *                    expression if it is found
 *                  typeSens - if true, consider type in the match
 *                  currType - the type at this point of the string
 * RETURNS:         the search expression was found
 *============================================================================*/
bool SemStr::doSearch(const SemStr& search, SSCIT& begin, SSCIT& end,
    bool typeSens, Type currType) const
{
    assert(indices.size() != 0);

    if (begin == indices.end()) return false;      // Can happen now!
    // Test if the current subexpression matches the search expression
    // Note: we use the -= operator (sign insensitive equal: equal in type
    // and size only)
    if (!typeSens || (search.type -= currType)) {
        SSCIT searchIT = search.indices.begin();
        end = begin;
        // In this part of the search, searchIt iterates through the search
        // string, and end iterates through *this. Either the search item
        // must be wild, or the two items must be equal (for a match here)
        while (searchIT != search.indices.end() && end != indices.end() &&
            (*searchIT == WILD || *searchIT == *end)) {
            searchIT++;
            end++;
        }
        if (searchIT == search.indices.end()) {
            end = findEndSubExpr(begin);
            return true;
        }
    }

    // Recurse through the following expressions (if any). We need to be aware
    // of the different types at different points of a string
    if (begin == indices.end())
        return false;           // Only way to fail!
    // First skip past *begin, and any integer and fixed arguments
    int idx = *begin;
    SSCIT first(begin);
    advance(begin,theSemTable[idx].totalFixedArgs()+1);
    for (int j=0; j < theSemTable[idx].iNumVarArgs; j++)
        if (doSearch(search, begin, end, typeSens,
          getSubType(first, currType, j)))
            return true;
    return false;
}

/*==============================================================================
 * FUNCTION:        SemStr::doSearch
 * OVERVIEW:        Exactly the same as the above, except that it takes non
 *                  const iterators, and is not const. This seems to be the
 *                  best compromise with const correctness
 * NOTE:            Can't use the above function with const_cast, since it
 *                    seems impossible to const_cast iterators
 * PARAMETERS:      See above
 * RETURNS:         See above
 *============================================================================*/
bool SemStr::doSearch(SemStr& search, SSIT& begin, SSIT& end, bool typeSens,
    Type currType)
{
    assert(indices.size() != 0);

    if (begin == indices.end()) return false;      // Can happen now!
    // Test if the current subexpression matches the search expression
    // Note: we use the -= operator (sign insensitive equal: equal in type
    // and size only)
    if (!typeSens || (search.type -= currType)) {
        SSCIT searchIT = search.indices.begin();
        end = begin;
        // In this part of the search, searchIt iterates through the search
        // string, and end iterates through *this. Either the search item
        // must be wild, or the two items must be equal (for a match here)
        while (searchIT != search.indices.end() && end != indices.end() &&
            (*searchIT == WILD || *searchIT == *end)) {
            searchIT++;
            end++;
        }
        if (searchIT == search.indices.end()) {
            end = findEndSubExpr(begin);
            return true;
        }
    }

    // Recurse through the following expressions (if any). We need to be aware
    // of the different types at different points of a string
    if (begin == indices.end())
        return false;           // Only way to fail!
    // First skip past *begin, and any integer and fixed arguments
    int idx = *begin;
    SSIT first(begin);
    advance(begin,theSemTable[idx].totalFixedArgs()+1);
    for (int j=0; j < theSemTable[idx].iNumVarArgs; j++)
        if (doSearch(search, begin, end, typeSens,
          getSubType(first, currType, j)))
            return true;
    return false;
}


/*==============================================================================
 * FUNCTION:        SemStr::doSearchAll
 * OVERVIEW:        Finds all matches of a subexpression and adds a pair of
 *                  iterators denoting the location of each match to a list of
 *                  such pairs.
 *                  The search expression may contain wild values (i.e. indicies
 *                  set to WILD) but these expressions will only match if they
 *                  are of exactly the same length and all the non-wild indices
 *                  match.
 * PARAMETERS:      search - the expression to search for
 *                  begin - where to start searching from in this expression
 *                  result - list of found matching subexpressions
 *                  typeSens - true if we are type sensitive
 *                  currType - the current Type at this point of the string
 * RETURNS:         at least one instance of the search expression was found
 *============================================================================*/
bool SemStr::doSearchAll(SemStr& search, SSIT& begin,
    ResultInfoList& result, bool typeSens, Type currType)
{
    if (begin == indices.end())
        return false;         // Guard against incomplete ss

    // Try and match this top level expression
    // Note: we use the -= operator (sign insensitive equal: equal in type
    // and size only)
    if (!typeSens || (search.type -= currType)) {
        SSCIT searchIT = search.indices.begin();
        SSCIT end = begin;
        // In this part of the search, searchIt iterates through the search
        // string, and end iterates through *this. Either the search item
        // must be wild, or the two items must be equal (for a match here)
        while (searchIT != search.indices.end() && end != indices.end() &&
          (*searchIT == WILD || *searchIT == *end)) {
            searchIT++;
            end++;
        }
        if (searchIT == search.indices.end()) {
            ResultInfo ri(begin, findEndSubExpr(begin), currType);
            result.push_back(ri);
        }
    }

    // Try the subexpressions (if any)
    int idx = *begin;
    SSIT first(begin);
    advance(begin,theSemTable[idx].totalFixedArgs()+1);
    for (int j=0; j < theSemTable[idx].iNumVarArgs; j++) {
        doSearchAll(search, begin, result, typeSens,
            getSubType(first, currType, j));
    }
    return result.size() != 0;
}

/*==============================================================================
 * FUNCTION:        SemStr::bumpRegisterAll
 * OVERVIEW:        Finds a subexpression within this expression of the form
 *                    r[ int x] where min <= x <= max, and replaces it with
 *                    r[ int y] where y = min + (x - min + delta & mask)
 * PARAMETERS:      min, max: minimum and maximum register numbers before
 *                    any change is considered
 *                  delta: amount to bump up the register number by
 *                  mask: see above
 * APPLICATION:     Used to "flatten" stack floating point arithmetic (e.g.
 *                    Pentium floating point code)
 *                    If registers are not replaced "all at once" like this,
 *                    there can be subtle errors from re-replacing already
 *                    replaced registers
 * RETURNS:         true if the expression was found and successfully replaced
 *============================================================================*/
bool SemStr::bumpRegisterAll(int min, int max, int delta, int mask)
{
    if (indices.size() == 0)
        return false;

    SemStr search, replace;
    search.push(idRegOf);  search.push(idIntConst);

    SSIT begin = indices.begin();
    bool result = false;
    ResultInfoList resInfo;

    if (doSearchAll(search, begin, resInfo, false, type)) {
        for (ResultInfoIt it = resInfo.begin(); it != resInfo.end(); it++) {
            SSIT last = it->second; 
            int reg = *--last;
            if ((min <= reg) && (reg <= max)) {
                *last = min + (reg - min + delta & mask);
                result = true;
            }
        }
    }
    return result;
}

/*==============================================================================
 * FUNCTION:        SemStr::findSubExpr
 * OVERVIEW:        Find the given wildcard subexpression within this
 *                  expression. The wildcard can represent an index only (not a
 *                  complete subexpression).
 * PARAMETERS:      n - the size of the subexpression
 *                  arr - the subexpression
 *                  iValue -  the last wildcard value
 * RETURNS:         subexpression was found
 *============================================================================*/
bool SemStr::findSubExpr(int n, int arr[], int& iValue) const
{
    SSCIT it = indices.begin();
    return doFindSubExpr(it, n, arr, iValue);
}

/*==============================================================================
 * FUNCTION:        SemStr::doFindSubExpr
 * OVERVIEW:        Recursive function that is a helper for the above version of
 *                  FindSubExpr.
 * PARAMETERS:      it -
 *                  n -
 *                  arr -
 *                  iValue -
 * RETURNS:         subexpression was found
 *============================================================================*/
bool SemStr::doFindSubExpr(SSCIT& it, int n, int arr[], int& iValue) const
{
    while (it != indices.end())
    {
        if (*it == arr[0])
        {
            bool bFound = true;
            for (int i=0; i < n; i++)
            {
                if (arr[i] == -1)
                    iValue = *it++;
                else if (arr[i] != *it++)
                {
                    bFound = false;
                    break;
                }
            }
            if (bFound) return true;
            continue;
        }
        else
        {
            int idx = *it++;
            // Did not match. Skip fixed and integer args, and recurse
            // through the variable args
            int j;
            for (j=0; j < theSemTable[idx].totalFixedArgs(); j++)
                it++;
            bool bRet;
            for (j=0; j < theSemTable[idx].iNumVarArgs; j++)
            {
                bRet = doFindSubExpr(it, n, arr, iValue);
                if (bRet) return true;
            }
            return false;
        }
    }
    return false;
}

/*==============================================================================
 * FUNCTION:        SemStr::
 * OVERVIEW:        
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
// See below for details
void SemStr::doRemoveSize(SSIT& it)
{
    SSIT oldIt = it;
    int idx = *it++;
    if (idx == idSize)
    {
        // There is one integer and one variable arg
        it++;               // Skip the size integer arg
        it = indices.erase(oldIt, it);
        doRemoveSize(it);
    }
    else if (idx == idSignExt)
    {
        // There is just the variable sized arg. Remove the index for
        // idSignExt, and recurse for the variable sized arg
        it = indices.erase(oldIt);
        doRemoveSize(it);
    }
    else
    {
        // Skip fixed and integer args, and recurse
        // through the variable args
        int j;
        for (j=0; j < theSemTable[idx].totalFixedArgs(); j++)
            it++;
        for (j=0; j < theSemTable[idx].iNumVarArgs; j++)
            doRemoveSize(it);
    }
}

/*==============================================================================
 * FUNCTION:        SemStr::removeSize
 * OVERVIEW:        Remove size casts, and sign extends. Useful where pattern
 *                    matching is required, e.g. switch analysis, AXP subst
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
// Remove any size ({}) and sign extend (!) functions
void SemStr::removeSize()
{
    SSIT it = indices.begin();
    doRemoveSize(it);
}

/*==============================================================================
 * FUNCTION:        SemStr::
 * OVERVIEW:        
 * NOTE:            Obsolete
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
// Write a representation to a file stream
void SemStr::writeSemStr(ofstream& of) const
{
    // Output the number of indices
    of << dec << indices.size() << "  ";
    SSCIT it = indices.begin();
    while (it != indices.end())
    {
        of << *it++ << " ";
    }
    of << endl;
}

// Returns a pointer to the indices list
list<int> * SemStr::getIndices() {
    return &indices;
}

/*==============================================================================
 * FUNCTION:        cmpSemPtrLess
 * OVERVIEW:        Helper function. Uses the '<' operator between two SemStrs
 *                  given pointers to them.
 * PARAMETERS:      <same as for operator< >
 * RETURNS:         ss1 < ss2
 *============================================================================*/
bool cmpSemPtrLess(const SemStr* ss1, const SemStr* ss2)
{
    return *ss1 < *ss2;
}

/*==============================================================================
 * FUNCTION:        cmpSemPtrEqual
 * OVERVIEW:        Helper function. Uses the '==' operator between two SemStrs
 *                  given pointers to them.
 * PARAMETERS:      <same as for operator== >
 * RETURNS:         ss1 == ss2
 *============================================================================*/
bool cmpSemPtrEqual(const SemStr* ss1, const SemStr* ss2)
{
    return *ss1 == *ss2;
}

#if 0
/*==============================================================================
 * FUNCTION:        SemStr::isTypeConvert
 * OVERVIEW:        Checks whether the given token (index) is one that converts
 *                    the "current type" (e.g. idTrunc, idFtoi)
 *                    All these tokens will take 2 integer parameters: a from
 *                    size, and a to size (in bits)
 * PARAMETERS:      tok: the given token (index)
 * RETURNS:         True if it is a type conversion token
 *============================================================================*/
bool SemStr::isTypeConvert(int tok)
{
    switch (tok) {
        case idTruncu:
        case idTruncs:
        case idZfill:
        case idSgnEx:
        case idFsize:
        case idItof:
        case idFtoi:
        case idSize:            // Old
        case idSignExt:         // Deprecated?
        case idMemOf:
            return true;
    }
    return false;
}
#endif

/*==============================================================================
 * FUNCTION:        SemStr::getSubType
 * OVERVIEW:        Get the type for the subexpression of the current expression
 * NOTE:            The real work is done by the overloaded version that takes
 *                    two integers (see below). We need this because we can't
 *                    cast iterators to const iterators
 * PARAMETERS:      it: constant iterator to the current index
 *                  currType: reference to the current type
 *                  i - index of subexpression (0=first) (only used for idTern)
 * RETURNS:         The resultant type
 *============================================================================*/
Type SemStr::getSubType(SSCIT it, const Type& currType, int i) const
{
    int idx = *it;
    int fromSz = 0;
    if (it != indices.end())
        fromSz = *++it;
    return getSubType(idx, fromSz, currType, i);
}
    
/*==============================================================================
 * FUNCTION:        SemStr::getSubType
 * OVERVIEW:        Get the type for the subexpression of the current expression
 * PARAMETERS:      it: nonconstant iterator to the current index
 *                  currType: reference to the current type
 *                  i - index of subexpression (0=first) (only used for idTern)
 * RETURNS:         The resultant type
 *============================================================================*/
Type SemStr::getSubType(SSIT it, const Type& currType, int i)
{
    int idx = *it;
    int fromSz = 0;
    if (it != indices.end())
        fromSz = *++it;
    return getSubType(idx, fromSz, currType, i);
}
    
/*==============================================================================
 * FUNCTION:        SemStr::getSubType
 * OVERVIEW:        Get the type for the subexpression of the current expression
 * EXAMPLE:         If current expression is fsize(32, 64, r[32]), and the
 *                    current type is 64 bits float, the type for the first
 *                    subexpression (r[32]) is 32 bit float.
 * PARAMETERS:      idx: current index
 *                  fromSz: if idx is a size conversion operator, this is the
 *                    "from" size
 *                  currType: reference to the current type
 *                  i - index of subexpression (0=first) (only used for idTern)
 * RETURNS:         The resultant type
 *============================================================================*/
Type SemStr::getSubType(int idx, int fromSz, const Type& currType, int i) const
{
    Type ret(currType);                 // Initially same type as current
    switch (idx) {
        case idTruncu:
        case idTruncs:
        case idZfill:
        case idSgnEx:
        case idFsize:
        case idItof:
        case idFtoi:
        case idSize:            // Old
        {
            ret.setSize(fromSz);
            switch (idx) {
                case idItof:
                    ret.setType(INTEGER); break;
                case idFtoi:
                    ret.setType(FLOATP); break;
                case idFsize:
                    ret.setType(FLOATP); break;
                case idTruncu:
                case idZfill:
                    ret.setSigned(false); break;
                case idTruncs:
                case idSgnEx:
                    ret.setSigned(true); break;
            }
            break;
        }
        case idSignExt:         // Deprecated?
            ret.setSigned(true); break;
        case idMemOf:
            return Type(INTEGER, 32, false);    // Addresses unsigned
        case idTern:
            if (i == 0)
                // First subexpression of idTern: a bool.
                // We use type int for now
                return Type(INTEGER);
            // Other 2 sub expressions are same as the parent expression
            break;
        default:
            break;
    }
    return ret;
}

/*==============================================================================
 * FUNCTION:        ResultInfo::ResultInfo
 * OVERVIEW:        Constructor for the ResultInfo struct
 * PARAMETERS:      f: iterator to the start of a subexpression
 *                  s: iterator to the end of a subexpression
 *                  t: reference to the type of a subexpression
 * RETURNS:         <n/a>
 *============================================================================*/
ResultInfo::ResultInfo(SSIT f, SSIT s, Type& t)
  : first(f), second(s), type(t)
{
}

/*==============================================================================
 * FUNCTION:        SemStr::searchMachSpec
 * OVERVIEW:        Search the string for IDs that are Machine Specific, i.e.
 *                    those with indices that are >= idMachSpec
 *                    If found, add them to the set
 * PARAMETERS:      stIdx: a set of INDEXs (IDs)
 * RETURNS:         Nothing
 *============================================================================*/
void SemStr::searchMachSpec(set<INDEX>& stIdx)
{
    SSCIT it = indices.begin();
    doSearchMachSpec(it, stIdx);
}

/*==============================================================================
 * FUNCTION:        SemStr::doSearchMachSpec
 * OVERVIEW:        Do the main work of searching the string for IDs that are
 *                    Machine Specific, i.e. those with indices that are
 *                    >= idMachSpec. If found, add them to the set
 * PARAMETERS:      it: iterator to the current index
 *                  stIdx: a set of INDEXs (IDs)
 * RETURNS:         Nothing
 *============================================================================*/
void SemStr::doSearchMachSpec(SSCIT& it, set<INDEX>& stIdx)
{
    if (it == indices.end()) return;
    INDEX idx = (INDEX)*it++;
    if (idx >= idMachSpec)
        // Found! Insert it into the set
        stIdx.insert(idx);
    int j;
    for (j=0; j < theSemTable[idx].totalFixedArgs(); j++)
        // Skip fixed arguments
        it++;
    for (j=0; j < theSemTable[idx].iNumVarArgs; j++)
        // Recurse to subexpressions
        doSearchMachSpec(it, stIdx);
}
