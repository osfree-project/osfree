/*
 * Copyright (C) 2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * File:       analyzeBlocks.h
 * OVERVIEW:   This file contains the definations of all the data structure 
 *             classes used for type analysis.
 *
 * Copyright (C) 2001, Sun Microsystems, Inc
 *============================================================================*/
 
 /*
 * $Revision: 1.7 $
 * 27 Aug 01 - Bernard: initial version
 */

#include <strstream>
#include <sys/types.h>
#include <list>
#include <map>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <iostream.h>
#include "global.h"

/* Forward declaration */
class Proc;

/*==============================================================================
 * Public defination of enum types varType and defUse. These are used throughout
 * the type analysis procedures
 *============================================================================*/
enum varType {INT_TYPE, POINTER_D, POINTER_I, FLOAT_TYPE};
enum defUse {DEFINE_TYPE, USE_TYPE};

/*==============================================================================
 * class: UDElement
 *
 * UDElement holds the memory address of the source machine where the register
 * was used or define. It also holds where instruction it was for that
 * particular address. Finally and most importantly, it holds the information
 * of whether the register was used or defined at that address.
 *============================================================================*/
class UDElement {
    /* 
     * Needs to be a friend class to BBBlock and UDChain for 
     * implementation simplification reasons 
     */  
     friend class BBBlock;
     friend class UDChain;

private:
    /*
     * Constructor
     */
    UDElement(ADDRESS inRtlListAddress,
    	      Byte instructNumber,
              defUse inDefUse);    

    /*
     * Destructor
     */
    ~UDElement();
    
    /*
     * Writes out the current state information of this class in a 
     * formatted form to the strstream
     */    
    void outputToStringStream(strstream& inStream);	

    /*
     * Source address of instruction where register is defined or used
     */
    ADDRESS rtlListAddress;
    
    /* 
     * Since one source address can have multiple RTLs, we want
     * to also store which RTL it is at the already specified 
     * memory address
     */       
    Byte instructNumber;
    
    /*
     * Stores whether this register is used or defined
     * Defaults to define
     */
    defUse elementUD; 
};


/*==============================================================================
 * class: UDChain
 *
 * UDChain holds a chain of UDElements. Each chain will only contain one define
 * element and multiple use elements. This class takes care of storing the 
 * UDElements in the right place.
 *
 * This class also holds the information of weakListOfChains, a chain that shows
 * any simple register type aliasing that can be used for propagation.
 *============================================================================*/
class UDChain {	

    /* 
     * Needs to be a friend class to the following classes for 
     * implementation simplification reasons 
     */  
     friend class BBBlock;
     friend class typeAnalysis;
     friend class analyzeBlocks;
     friend class BasicBlock;
private:

    /*
     * Constructor
     */
    UDChain(ADDRESS rtlListAddress,
    	    Byte instructNumber,
            defUse inDefUse);

    /*
     * Destructor
     */       
    ~UDChain();
        
    /*
     * Add an element to the chain (this will create a new
     * chain if the element is a defination)
     */
    void addElement(ADDRESS rtlListAddress,
    		    Byte instructNumber,
                    defUse inDefUse);

    /*
     * Writes out the current state information of this class in a 
     * formatted form to the strstream
     */     
    void outputToStringStream(strstream& inStream);		    	 	        
     
    /*
     * Used to hold the list of elements
     */
    list<UDElement*> listOfElements;           

    /*
     * List of simple register type aliases which we can propagate across
     */
    list<UDChain*> weakListOfChains;    

public:    
    /*
     * Holds the type information for the whole chain.
     * 
     * Note: This really should be a private field. It will 
     * probably be moved to the private field in the future
     * with member functions reading and modifying this variable
     * added.
     */
    varType chainType;  // Default to INT
    
    /*
     * Returns the number of elements in the current chain
     */         
    int numOfElements();    

	/*
	 * Print the type of the chain 
	 */
	void printType (strstream &ostream); 
    
};

/*==============================================================================
 * class: CallRegInformation
 *
 * CallRegInformation holds the information that can be inferred for propagation
 * purposes at a call instruction. This class stores the pointer to the Proc 
 * data structure of the callee function. It also stores the which registers are
 * used to store return values and which registers as used as parameters.
 *============================================================================*/
class CallRegInformation {
    
public:
    /*
     * Default constructor
     */
    CallRegInformation();
    
    /*
     * Default destructor
     */    
    ~CallRegInformation();
    
    /*
     * Used to store the address of a Proc structure
     * Note: Perhaps this should be named to saveProc
     */
    void addProc(Proc * inProc);
    
    /*
     * Add register regNum to the map of registers that are 
     * used to receive return values (there only should be one)
     */    
    void addToReturnRegMap(Byte regNum, UDChain* inUDChain);

    /*
     * Add register regNum to the map of registers that are 
     * used as parameters
     */            
    void addToParamRegMap(Byte regNum, UDChain* inUDChain);
    
    /*
     * Writes out the current state information of this class in a 
     * formatted form to a C string.
     */ 
    const char* outputToString();

/* 
 * Note that the following fields should really be private
 * however, the current implementation requires them to be
 * public. This will be changed in the future
 */

    /*
     * Holds calleeProc address
     */  
    Proc * calleeProc;
    
    /*
     * Map of return registers
     */  
    map<Byte, UDChain*> returnReg;    
    
    /*
     * Map of parameter registers
     */      
    map<Byte, UDChain*> paramReg;    	    
    
private:    
    /* 
     * Used by outputToStream
     */
    strstream returnStream;
};



/*==============================================================================
 * class: BBBlock
 *
 * BBBlock is the top level data structure and holds all the other type analysis 
 * data structures. This class has most of the member functions that are needed
 * to manipulate all the type analysis data structures.
 *============================================================================*/
class BBBlock {

public:

    /*
     * Constructor
     */
    BBBlock();
    
    /*
     * Destructor
     */  
    ~BBBlock();


    /*
     * Used to add an use or define of a register to the 
     * data structure
     *
     * Just a wrapper to the template based addUDElementCommon
     */
    void addUDElementReg(Byte regNum, 
                         ADDRESS rtlListAddress,
			 Byte instructNumber, 
			 defUse inDefUse);
			 
    /*
     * Used to add an use or define of a memory address to the 
     * data structure
     *
     * Just a wrapper to the template based addUDElementCommon
     */			     
    void addUDElementMem(ADDRESS regNum, 
                         ADDRESS rtlListAddress,
			 Byte instructNumber, 			  
			 defUse inDefUse);
			 

    /*
     * This function returns a pointer to the list of UDChains
     * for the specified register number.
     */		 
    list<UDChain*>* returnRegChainListAt(Byte regNum);    
    
    /*
     * This function returns a pointer to the list of UDChains
     * for the specified memory address.
     */		     
    list<UDChain*>* returnMemChainListAt(ADDRESS regNum);
    
    /*
     * Return a map of registers in the current basic block
     * that does not have definations
     */    
    map<Byte, UDChain*> returnNoDefRegChains();

    /*
     * Return a map of memory addresses in the current basic block
     * that does not have definations
     */        
    map<ADDRESS, UDChain*> returnNoDefMemChains();
        
    /*
     * Return a map of registers to their last
     * chain which has a defination (not just uses)
     */    
    map<Byte, UDChain*> returnLastDefRegChains();
    
    /*
     * Return a map of memory addresses to their last
     * chain which has a defination (not just uses)
     */    
    map<ADDRESS, UDChain*> returnLastDefMemChains();
    
    /*
     * Writes out the current state information of this class in a 
     * formatted form to a C string.
     */     
    const char* outputToString();    
    
    /*
     * Set the type of the last chain for the register specified 
     * by regNum
     */
    void setTypeOfLastChainForReg(Byte regNum, varType inVarType);
    
    /* 
     * Create a new CallRegInformation structure (pointed to by callInfoStruct)
     * and then return a pointer to it.
     */
    CallRegInformation* createCallRegInfo();

    /* 
     * Return a pointer to the existing CallRegInformation (callInfoStruct)
     * Returns null if createCallRegInfo has not been called yet.
     */    
    CallRegInformation * returnCallRegInfo();
    
    /*
     * Returns whether or not this current basic block 
     * ends with a Ret. This is important as we need to keep
     * track of all return sites.
     */    
    bool returnRetInfo(){ return endsWithARet;}
    
    /*
     * Set the variable endsWithARet to the parameter inValue.
     */        
    void setRetInfo(bool inValue){ endsWithARet = inValue;}
              
    /*
     * Create a alias link between registers inReg1 and inReg2
     */ 
    void aliasRegLink(Byte inReg1, Byte inReg2);   
    
    /*
     * Return a list of all Chains that contains a alias link
     */    
    list<UDChain*> returnRegAliasChains();
    
    /*
     * Propagates type information within the basic block
     */    
    void propagateRegWithinBB();
    
private:

    /*
     * Used to add an use or define of a register or memory address \
     * to the data structure
     */
    template <class IntSize>
        void addUDElementCommon(IntSize regNum, 
                                ADDRESS rtlListAddress, 
				Byte instructNumber,
		                defUse inDefUse,
		                map<IntSize, list<UDChain*>*>& inMap);
				
    /*
     * Used by destructor to remove all the lists in 
     * the reg and memory maps
     */				
    template <class IntSize>
        void deleteListFromMap(map<IntSize, list<UDChain*>*>& inMap);

    /*
     * Used to return a list of memory or register chains of the specified
     * register number or memory address
     */		
    template <class IntSize>
        list<UDChain*>* returnGenericChainListAt(IntSize regNum, 
	                              map<IntSize, list<UDChain*>*>& inMap);
    /*
     * Used to return a map of registers or memory addresses that have
     * no definations, just uses.
     */		
    template <class IntSize> 
        map <IntSize, UDChain*> returnNoDefGenericChains(
	                              map<IntSize, list<UDChain*>*>& inMap);
    /*
     * Used to return a map of registers or memory addresses containing the
     * the last UDChain that contains a defination.
     */						      
    template <class IntSize> 
        map <IntSize, UDChain*> returnLastDefGenericChains(
	                              map<IntSize, list<UDChain*>*>& inMap);
    /*
     * Used to create a alias link between inVar1 and inVar2
     */	
    template <class IntSize> 
    	void aliasGenericLink(IntSize inVar1, IntSize inVar2, 
				      map<IntSize, list<UDChain*>*>& inMap);
    /*
     * Return a list of UDChains containing all the registers or memory
     * address (one or the other, not both at the same time) that contains 
     * aliases
     */					      
    template <class IntSize> 
    	list<UDChain*> returnGenericAliasChains(
	                              map<IntSize, list<UDChain*>*>& inMap);
				          
    /*
     * Holds a map of registers with its UDChains
     */    
    map<Byte, list<UDChain*>*> mapOfReg;
    
    /*
     * Holds a map of memory addresses with its UDChains
     */    
    map<ADDRESS, list<UDChain*>*> mapOfMemAddress;    

    /*
     * Holds a pointer to a CallRegInformation object
     */            
    CallRegInformation* callInfoStruct;

    /*
     * Specifies whether this basic block ends with a Ret instruction
     */    
    bool endsWithARet; 
    
    /* 
     * Used by outputToStream
     */
    strstream returnStream;
};
