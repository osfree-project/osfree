/*
 * Copyright (C) 1998-2001, The University of Queensland
 * Copyright (C) 2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * File:       ss.h
 * OVERVIEW:   Declaration of the SemStr and related classes.
 *============================================================================*/
/*
 * $Revision: 1.50 $
 *             Fundamental to the new organisation is the semantic string,
 *             which is used to represent expressions and locations, but
 *             not assignments.  Conceptually, it is a string of indices
 *             into a special table. The entries represent such things as
 *             operators, registers, memory, parameters (registers or memory
 *             that are not yet instantiated), or of integers, where there are
 *             arguments for certain categories of entry. For example, register
 *             ax is represented by an index into an item representing the
 *             operation "register of" (denoted by "r[]"), followed by an index
 *             representing register ax, which will often be written as "ax",
 *             but will be implemented as an integer.  Before instantiation,
 *             r[rd] will be represented as "reg" "rd" (there will be special
 *             indices for all the parameters). After instantiation, it would
 *             be "reg" "ax" as above.  The actual representation (pre/postfix,
 *             list or array) will be hidden as much as possible from the users.
 *             The index will be into theSemTable, which is the vector of
 *             SemItems.  Each SemItem will have a field indicating the number
 *             of arguments, and a string for display purposes.
 *
 *             As an example, the diagram below shows theSemTable after
 *             initialization:
 *
 *  Index| Category  | Number of | Number of | Number of | String
 *       |           | int args  | fixed args| var args  | representation
 * ----------------------------------------------------------------------------
 *     0 | cOPERATOR |  0        | 0         | 2        |  "+"
 *     1 | cOPERATOR |  0        | 0         | 2        |  "-"
 *     2 | cOPERATOR |  0        | 0         | 2        |  "*"
 *     3 | cOPERATOR |  0        | 0         | 2        |  "/"
 *     4 | cOPERATOR |  0        | 0         | 2        |  "*!"
 *     5 | cOPERATOR |  0        | 0         | 2        |  "/!"
 *     6 | cOPERATOR |  0        | 0         | 2        |  "%"
 *     7 | cOPERATOR |  0        | 0         | 2        |  "%!"
 *     8 | cOPERATOR |  0        | 0         | 1        |  "0-"
 *     9 | cOPERATOR |  0        | 0         | 2        |  "and"
 *    10 | cOPERATOR |  0        | 0         | 2        |  "or"
 *    11 | cOPERATOR |  0        | 0         | 2        |  "="
 *    12 | cOPERATOR |  0        | 0         | 2        |  "<"
 *    13 | cOPERATOR |  0        | 0         | 2        |  ">"
 *    14 | cOPERATOR |  0        | 0         | 2        |  "<="
 *    15 | cOPERATOR |  0        | 0         | 2        |  ">="
 *    16 | cOPERATOR |  0        | 0         | 2        |  "~="
 *    17 | cOPERATOR |  0        | 0         | 2        |  "+f"
 *    ...
 *
 * Copyright (C) 1998-2001, The University of Queensland, BT group
 * Copyright (C) 2001, Sun Microsystems, Inc
 *    
*/

/*
 * 25 Jun 98 - Mike: Instantiate()
 * 17 Jul 98 - Mike: Instantiate returns a bool now
 * 28 Jul 98 - Mike: Changed getSubExpr() to return a pointer rather than
 *              a reference, to make it easier to convert old code
 * 14 Dec 98 - Shawn: added floating point operators
 * 21 Jan 99 - Mike: Added substSpcl()
 * 09 Mar 99 - Mike: More transcendental functions (e.g. sqrt, arctan)
 * 11 Mar 99 - Mike: Added substitution function for FPUSH/FPOP removal
 * 10 Mar 99 - David: Added getIndices()
 * 24 Mar 99 - Mike: Added standard register ids such as idPC, idZF
 * 30 Mar 99 - David: Added idIndex and idDef
 * 16 Apr 99 - Doug: Added print methods to SemTable and SemItem
 *                   SemTable now has a hash table
 * 22 Apr 99 - Doug: Added generalised search and replace methods * (search, 
 *                   searchReplace, searchReplaceAll, doSearch)
 * 05 Apr 99 - Doug: Added search that returns a list of found subexpressions
 * 16 May 99 - Doug: Added new version of getSubExpr the takes a reference
 *                   argument to a SemStr in which to return the subexpression
 * 19 May 99 - Mike: Quelched some warnings with const correctness
 * 02 Jun 99 - Mike: Removed leading upper case on function names
 * 28 Mar 00 - Mike: Added SemStr::isFloat()
 * 12 Apr 00 - Mike: Added lessSGI (sign insensitive less than function) for
 *                    SemStrs, and a typedef for sets of these
 * 25 Apr 00 - Mike: Added lessTI for type insensitive SemStr maps
 * 28 Sep 00 - Mike: Added bumpRegisterAll()
 * 27 Nov 00 - Mike: Added searchMachSpec() to search for Machine Specific IDs
 * 29 Nov 00 - Mike: Added doSearchMachSpec() to impelement the above
 * 01 Dec 00 - Mike: machineSimplify() and doMachSimplify return bools now
 * 07 Dec 00 - Mike: partitionTerms and Accumulate work on multisets to fix
 *              the a=b+b bug; untabbed
 * 02 Mar 01 - Mike: Fixed bug in "lessTI" which made it basic type sensitive;
 *              Added printt() for more convenient debugging
 * 05 Mar 01 - Simon: removed SemStr::expand()
 * 29 Mar 01 - Mike: Added SemStr::printAsC
 * 18 Apr 01 - Mike: Renamed SemStr::operator<< to operator&&; operator<< now
 *              does a push() operation.
 * 04 May 01 - Mike: Added operator<<(const SemStr&)
 * 09 May 01 - Mike: Added operator>>(int) (does a prep)
 * 18 May 01 - Nathan: Added Semstr(int,...) constructor
 * 26 Jun 01 - Nathan: Added SemItem::setItem
 * 13 Aug 01 - Bernard: Added support for type analysis
 * 28 Aug 01 - Mike: Oops, doSimplifyArith returns void
 * 16 Oct 01 - Mike: Added setTiSemStr typedef
 * 28 Nov 01 - Mike: Removed instantiate and doInstantiate methods (obsolete)
*/

#ifndef __SS_H__
#define __SS_H__

#include "type.h"               // Class Type
// #include "typeLex.h"            // A convenient place to include the typeLex.h
                                // which includes many other headers. 
                                // Probably not the best place for it though				

/*==============================================================================
 * A SemItem contains all the information for a single entry in the semantic
 * string table.
 *============================================================================*/
struct SemItem
{
    /**
     * The category.
     */
    CAT cat;

    /**
     * Number of integer args (e.g. idIntConst has 1).
     */
    int iNumIntArgs;

    /**
     * Number of index args (e.g. param has 1).
     */
    int iNumFixedArgs;

    /**
     * Number of variable length args (e.g. idPlus has 2).
     */
    int iNumVarArgs;

    /**
     * Name for printing.
     */
    string sName;


    /**
     * Constructor.
     */
    SemItem();

    /**
     * Constructor.
     */
    SemItem(CAT c, int ni, int nf, int nv, const string& s);

    /**
     * Return the total number of fixed args of this item (int args + non-int
     * fixed args).
     */
    int totalFixedArgs() const
    {
        return iNumIntArgs + iNumFixedArgs;
    }

    /**
     * Display to ostream.
     */
    void print(ostream& os);
};

/*==============================================================================
 * The SemTable implements the semantic string table.
 *============================================================================*/
class SemTable
{
public:

    /**
     * Constructor
     */
    SemTable();

    /**
     * Adds array indexing capability.
     */
    const SemItem& operator[](int i) const;

    /**
     * Add new (processor dependant) item.
     */
    int addItem(CAT c, int ni, int nf, int nv, const char* s);

    /**
     * Shortcut of the above, for cSPECIAL and 0 args.
     */
    int addItem(const char* s);

    /**
     * Find the index for an item, given its string represenatation.
     */
    int findItem(const string& s, bool mustFind = true);

    /**
     * Modify an existing item
     */
    int setItem(int idx, CAT c, int ni, int nf, int nv, const char* s);

    /**
     * Output the entries of the sem table.
     */
    void print(ostream& os);

    /**
     * Get the number of items.
     */
    unsigned size() const;

private:

    /**
     * Initialise the table. Now only called from the constructor.
     */
    void init();

private:

    /**
     * The entries in the table.
     */
    vector<SemItem> items;

    /**
     * A map from the entries to their indexes. Allows fast check for whether or
     * not a entry already exists in a table.
     */
    map<string,int> itemIndexes;

};

/*==============================================================================
 * The SemStr class represents expression and locations in a linearised form.
 *============================================================================*/

/*
 * A struct used in doSearchAll to hold information about a subexpression.
 */
struct ResultInfo {
    SSIT    first;          // Iterator to start of subexpression
    SSIT    second;         // Iterator to end+1 of subexpression
    Type    type;           // Type of the subexpression

    ResultInfo(SSIT f, SSIT s, Type& t);    // Constructor
};

/*
 * A typedefs for a list of ResultInfo's, and an iterator
 */
typedef list<ResultInfo> ResultInfoList;
typedef ResultInfoList::iterator ResultInfoIt;

class SemStr
{
    // Can I just say friend RT, friend HRTL and friend
    // Proc since everything else is derived from these two?
    friend class RT;
    friend class HRTL;
    friend class RTAssgn;
    friend class RTFlagDef;
    friend class RTFlagCall;
    friend class HLJump;
    friend class HLJcond;
    friend class HLNwayJump;
    friend class HLCall;
    friend class HLScond;
    friend class Proc;

protected:

    /*
     * The list of indicies into the semantic table.
     */
    list<int> indices;

    /*
     * Kind of semantic string: ordinary, optable, or condtable.
     */
    EKIND kind;

    /*
     * Type of the expression (broad type, size, and signedness)
     */
    Type type;

public:

    /*
     * Default constructor.
     */
    SemStr();

    /*
     * Constructor with kind.
     */
    SemStr(EKIND k);

    /*
     * Constructor with size
     */
    SemStr(int s);

    
    /*
     * Constructor with Type
     */
    SemStr(const Type& ty);

    /*
     * Copy constructor.
     */
    SemStr(const SemStr& other);

    /*
     * Assignment operator
     */
    SemStr& operator=(const SemStr& other);

    /*
     * Construct from an array of integers. Can also be passed two iterators
     * into another SemStr.
     */
    SemStr(int* it, int* end, const Type& ty = Type());

    /**
     * VA_ARG constructor (to avoid ambiguity, size must be at least 1)
     */
    SemStr(int size, int first, ...);

    /**
     * Construct from two const iterators into another SemStr.
     */
    SemStr(SSCIT it, SSCIT& end, const Type& ty = Type());

    /*
     * Construct from two nonconst iterators into another SemStr.
     */
    SemStr(SSIT it, SSIT& end, const Type& ty = Type());

    /**
     * Virtual destructor.
     */
    virtual ~SemStr();

    /**
     * Get the kind of expr (eORDINARY or eOPTABLE or eCONDTABLE).
     */
    EKIND getKind() const   { return kind; }

    /*
     * Get the type of the expression represented by this semantic string.
     * Not to be confused with getKind() (formerly called getType())
     */
    const Type& getType() const  { return type; }

    /*
     * Non const version of the above (needed to set size, for example)
     */
    Type& getType() { return type; }

    /*
     * Return true if the type of this SemStr is a float
     * HACK: about to be obsolete
     */
    bool isFloat() const { return (type.getType() == FLOATP);}

    /*
     * Set the type of the expression represented by this semantic string.
     */
    void setType(const Type& t) {type = t; }

    /*
     * Set the types of the expression (size, sign, and type) to that of the
     * given Semantic String
     */
    void setTypes(const SemStr& o) {type = o.type; }

    /*
     * True if equal (including type).
     */
    bool operator==(const SemStr& o) const;

    /*
     * True if equal (disregarting type).
     */
    bool operator%=(const SemStr& o) const;

    /*
     * True if equal (disregarting only sign).
     */
    bool operator-=(const SemStr& o) const;

    /*
     * True if this < o (considering type)
     * This is the default ordering
     */
    bool operator<(const SemStr& o) const;

    /*
     * True if this < o (disregarding sign)
     */
    bool operator&&(const SemStr& o) const;

    /*
     * True if this < o (disregarding type altogether)
     */
    bool operator%(const SemStr& o) const;

    /*
     * Push an integer to the end of a semantic string, e.g.
     *   str << idRegOf << idConst << 2
     */
    SemStr& operator<<(int i);

    /*
     * Push a SemStr to the end of a semantic string, e.g.
     *   str << idPlus << makeReg(rs1) << makeReg(rs2)
     */
    SemStr& operator<<(const SemStr& pss);

    /*
     * Prepend an integer to the front of a semantic string, e.g.
     *   str >> idRegOf
     */
    SemStr& operator>>(int i);

    /*
     * Push the index i. Old way of doing the above
     */
    void push(int i);

    /*
     * Prepend index i.
     */
    void prep(int i);

    /*
     * Push copy of another SemStr.
     */
    void pushSS(const SemStr* pss);

    /*
     * Push n items of array a.
     */
    void pushArr(int n, const int a[]);

    /*
     * Remove the last index and return it.
     */
    int pop();

    /*
     * Remove the first index and return it.
     */
    int popFirst();

    /*
     * Set this to be the empty expression.
     */
    void clear();

    /*
     * True if matches this special register.
     */
    bool isSpRegEqual(int i) const;

    /*
     * True if contains this special register.
     */
    bool isSpRegCont(int i) const;

    /*
     * True if matches this numbered register.
     */
    bool isNumRegEqual(int i) const;

    /*
     * True if contains this numbered register.
     */
    bool isNumRegCont(int i) const;

    /*
     * True if contains numbered register from iLo to iHi.
     */
    bool isNumRegCont(int iLo, int iHi) const;

    /*
     * True if equals the given array of ints. Accepts wild indices, but
     * not wild subexpressions.
     */
    bool isArrayEqual(unsigned n, int pInts[]) const;

    /*
     * True if equals the given array of ints. Accepts wild indices, and
     * also wild expressions.
     */
    bool isArraySimilar(unsigned n, int pInts[]) const;

    /*
     * Get first index (usually an operator).
     */
    int getFirstIdx() const;

    /*
     * Get 2nd index (usually an argument).
     */
    int getSecondIdx() const;

    /*
     *  Get 3rd index (us. another argument).
     * This function is often used to get the number of a numbered
     * register, since we have r[ int num.
     */
    int getThirdIdx() const;

    /*
     * Get the ith subexpression (i.e. the ith variable expression).
     * Subexpressions are numbered from 0.
     * The caller of this method is responsible for the newly created SemStrs
     * memory.
     */
    SemStr* getSubExpr(int i) const;

    /*
     * Get the ith subexpression (i.e. the ith variable expression).
     * Subexpressions are numbered from 0.
     * The reference parameter used to restore the results ensures that the
     * caller will automatically delete the memory used for it.
     */
    SemStr& getSubExpr(int i, SemStr& result) const;

    /*
     * Get the ith index; often used after IsArrayEqual with wild cards.
     */
    int getIndex(int i) const;

    /*
     * Get the last index. E.g. isSwitch() for idUP.
     */
    int getLastIdx() const;

    /*
     * Searches for a given subexpression within this expression. This method
     * accepts a limited form of wild search expressions. Formally, a search
     * expression (S) will match a subexpression (E) if:
     *  length(S) <= length(E) and each position,p , in S the following holds:
     *   S[p] == WILD || S[p] == E[p]
     *
     * For example an expression with the following SemStr representation:
     *
     *    {0,{3,78},{43,6,14}}
     *
     * will be successfully matched by these expressions and the 2nd parameter
     * will be initialised with the following values:
     *
     *      Search     |  Returns
     *    ------------------------
     *    {3,78}       |  {3,78}
     *    {0,*,*,{43}} |  {0,{3,78},{43,6,14}}
     *
     * If typeSens is set, then type is considered during the match
     */
    bool search(const SemStr& search, SemStr& result, bool typeSens = false)
        const;

    /*
     * Searches for all instances of a given subexpression within this
     * expression and adds them to a given list in reverse nesting order.
     * If typeSens is set, then type is considered during the match
     */
    bool searchAll(SemStr& search, list<SemStr*>& result,
        bool typeSens = false);

    /*
     * Searches for a given subexpression within this expression
     * and replaces all instances of it with a supplied replacement
     * expression. 
     * If typeSens is set, then type is considered during the match
     */
    bool searchReplace(SemStr& search, const SemStr& replace,
        bool typeSens = false);
    bool searchReplaceAll(SemStr& search, const SemStr& replace,
        bool typeSens = false);

    /*
     * Find the first occurance of idParam and return its id
     */
    int findParam( );
        
    /*
     * Substitute all occurrences of register iReg with the given string.
     */
    bool substReg(int iReg, const SemStr* pssRepl);

    /*
     * Substitute all occurrences of `idSpec' with ssRepl.
     */
    bool substSpcl(int idSpec, const SemStr* pssRepl);

    /*
     * Replace all expressions of the form r[ int x] with r[ int y] when
     * min <= x <= max, and y = min + (x - min + delta & mask)
     */
    bool bumpRegisterAll(int min, int max, int delta, int mask);

    /*
     * Replace the item at index i with iRepl.
     */
    void substIndex(unsigned i, int iRepl);
    /*
     * NOTE: the function below is not complete. See implementation.
     * Compare one string with another string, or with an array of ints
     * with possible wildcards if bWild is true. If bWild is true, return
     * a list of actual matching values in liValues.
     */
    bool smartCompare(const SemStr& other, bool bWild,
        list<int>& liValues);

    /*
     * Smart compare, with wildcards, for arrays of ints (n ints in array).
     */
    bool smartCompare(int n, const int arr[], list<int>& liWilds);

    /*
     * Find the end of the subexpression starting at index it.
     */
    SSCIT findEndSubExpr(SSCIT it) const;

    /*
     * Find the end of the subexpression starting at index it.
     */
    SSIT findEndSubExpr(SSIT  it) const;

    /*
     * Simplify by using constant folding, and also commuting if possible
     * to always place integer constants on the right. The adding of
     * negative constants is also changed to subtracting positive conts
     * Also changes << k to a multiply
     */
    void simplify();

    /*
     * Simplify by removing idAddrOf - idMemOf pairs (with possibly intervening
     * idSize) where possible
     */
    void simplifyAddr();

    /*
     * Find the given (in array form) subexpression within this semantic
     * string. Return the last wildcard value matched in iValue
     */
    bool findSubExpr(int n, int arr[], int& iValue) const;

    /*
     * Print to a string, infix.
     */
    string sprint() const;

    /*
     * Print to a stream.
     */
    virtual void print(ostream& os = cout) const;

    /*
     * Print to a stream, as C (so r[10] -> r10)
     */
    virtual void printAsC(ostream& os = cout) const;

    /*
     * Print to a stream, with <size>.
     */
    virtual void prints(ostream& os = cout) const;

    /*
     * Print to a stream, with <type>.
     */
    virtual void printt(ostream& os = cout) const;

    /*
     * Output operator for a SemStr.
     */
    friend ostream& operator<<(ostream& os, const SemStr& s);

    /*
     * Print to a string, prefix.
     */
    string sprintPrefix() const;

    /*
     * Print to a stream.
     */
    virtual void printPrefix(ostream& os = cout) const;

    /*
     * Debug dump: just print the indices as decimal
     */
    void dump(ostream& os = cout) const;

    /*
     * Write string representation to file.
     */
    void writeSemStr(ofstream& of) const;

    /*
     * Get the number of indices. Like a string length.
     */
    int len() const;

    /*
     * Print to a string.
     */
    string sprintinfix() const;

    /*
     * Remove size and sign extend operations.
     */
    void removeSize();

    /*
     * Machine dep simplification of SS. Return true if a change made
     */
    bool machineSimplify();
    
    /*
     * Get the list of indices
     */
    list<int> * getIndices();

    /*
     * Search the string for machine specific indices, i.e. those indices that
     * are >= idMachSpec. If so, add these to the passed set
     */
    void searchMachSpec(set<INDEX>& stIdx);

    /*
     * Do the main work of searching the string for machine specific indices
     */
    void doSearchMachSpec(SSCIT& it, set<INDEX>& stIdx);

private:

    /*
     * Do the work of simplify()
     */
    bool doSimplify(SSIT& it, SSIT end);

    /*
     * Do the work of simplifyAddr()
     */
    void doSimplifyAddr(SSIT& it, SSIT end);

    /*
     * This method simplifies an expression consisting of + and - a
     * the top level. For example, (%sp + 100) - (%sp + 92) will be
     * simplified to 8.
     */
    SemStr& simplifyArith(SemStr& result);

    /*
     *
     */
    bool doIsArraySimilar(SSCIT& it, int& i, int n, int arr[]) const;

    /*
     * Attempts to find an match of a subexpression within this expression. If
     * found, the start and end reference parameters are set the start and end
     * of points respectively of the search expression within this expression.
     */
    bool doSearch(const SemStr& search, SSCIT& start, SSCIT& end,
        bool typeSens, Type currType) const;
    /*
     * Non const version of the above, for SearchReplace()
     */
    bool doSearch(SemStr& search, SSIT& start, SSIT& end, bool typeSens,
        Type currType);

    /*
     * Attempts to find all matches of a given search expression within this
     * expression. A list of iterator pairs is built that contains the start and
     * end respectively of each match. The subexpressions are added to the list
     * in reverse nesting order.
     */
    bool doSearchAll(SemStr& search, SSIT& begin, 
        ResultInfoList& result, bool typeSens, Type currType);

    /*
     * Do the work of finding a subexpression
     */
    bool doFindSubExpr(SSCIT& it, int n, int arr[], int& iValue)
                    const;

    /*
     *
     */
    typedef const SemStr* (*PREPL)(int);
    
    /*
     *
     */
    bool doSubstReg(int iRegLo, int iRegHi, PREPL pRepl, int idxType,
        SSIT& it);

    /*
     *
     */
    bool doSubstReg(int iReg, const SemStr* pssRepl,
                    int idxType, SSIT& it);

    /*
     *
     */
    void doRemoveSize(SSIT& it);

    /*
     * Next is in machine dependent files like ctisparc.cc.
     */
    bool doMachSimplify(SSIT& it);

    /*
     *
     */
    void doRecurInfix(SSCIT& it, string& res, SSCIT& end) const;

    /*
     *
     */
    void doSprintInfix(SSCIT& it, string& res, SSCIT& end) const;

    /*
     * Partitions this expression's terms into
     * positive non-integer fixed terms, negative non-integer fixed terms,
     * integer terms and variable terms.
     */
    void partitionTerms(multiset<SemStr>& positives,multiset<SemStr>& negatives,
        vector<int>& integers, bool negate) const;

    /*
     * Create an expression that is the sum of all
     * expressions in a set. This sum is appended to this
     * expression and joined with binOper if both this
     * expression and the sum are not empty. If this expression is
     * empty and the sum isn't, then unOper is prepended to the front
     * of the result (e.g. idNeg).
     * E.g. if this expression is already %sp+4 and we
     * are given the list <4,r[8],m[14]> and the + operator, then
     * the resulting expression is %sp+4+4+r[8]+m[14].
     * Not to be confused with library function "accumulate"
     */
    SemStr& Accumulate(multiset<SemStr>& exprs, int binOper, int unOper);

    /*
     * Check whether a given token is one that converts the "current type"
     * of the SemStr
     */
    static bool isTypeConvert(int tok);

    /*
     * Get the Type of the ith subexpression, given the current index and
     * the current type. Note the three overloaded versions, because of a
     * problem with const iterators
     */
    Type getSubType(SSCIT it,            const Type& currType, int i) const;
    Type getSubType(SSIT  it,            const Type& currType, int i);
    Type getSubType(int idx, int fromSz, const Type& currType, int i) const;
};


/*
 * SemStr helper function. Uses the '==' operator between two SemStrs
 * given pointers to them.
 */
bool cmpSemPtrEqual(const SemStr* ss1, const SemStr* ss2);

/*
 * SemStr helper function. Uses the '<' operator between two SemStrs
 * given pointers to them.
 */
bool cmpSemPtrLess(const SemStr* ss1, const SemStr* ss2);

/*
 * A class for comparing SemStrs in a SIGN insensitive way
 * An object of this type can have the operator() function called to compare
 * two SemStrs. If we have an object called cmp of this type, then the template
 * functions can call cmp(x, y) to compare SemStrs x and y.
 */
class lessSGI : public binary_function<SemStr, SemStr, bool> {
public:
    bool operator()(const SemStr& x, const SemStr& y) const
        {
            return (x && y);            // This is compare, not "logical and"
        }
};

/*
 * A class for comparing SemStrs in a TYPE insensitive way
 * An object of this type can have the operator() function called to compare
 * two SemStrs. If we have an object called cmp of this type, then the template
 * functions can call cmp(x, y) to compare SemStrs x and y.
 */
class lessTI : public binary_function<SemStr, SemStr, bool> {
public:
    bool operator()(const SemStr& x, const SemStr& y) const
        {
            return (x % y);            // This is compare, not "modulus"
        }
};



/*
 * A typedef for a set of Semantic Strings, where membership of this set is
 * insensitive to sign. Saves some wordiness when passing such things as
 * parameters
 */
typedef set<SemStr, lessSGI> setSgiSemStr;

/*
 * Similarly, a set of completely type insensitive Semantic Strings.
 */
typedef set<SemStr, lessTI> setTiSemStr;

#endif          // __SS_H__

