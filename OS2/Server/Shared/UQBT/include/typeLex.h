/*
 * Copyright (C) 2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * File:       typeLex.h
 * OVERVIEW:   This file contains the declarations of all the classes used
 *             by the type analyzer's lexical analyzer. The purpose of theses 
 *             classes is to convert semantic strings into tokens which
 *             the type analyzer's parser can use.
 *
 * Copyright (C) 2001, Sun Microsystems, Inc
 *============================================================================*/
 
 /*
 * $Revision: 1.6 $
 * 27 Aug 01 - Bernard: initial version
 */
 
#include "analyzeBlocks.h"
#include "typeAnalysis.h"

/*==============================================================================
 * class: binOpToken
 *
 * binOpToken is the class that holds all the information the lexical analyzer 
 * can gather from the semantic string of binary operators (e.g. +,-,*,/).                  
 *============================================================================*/
class binOpToken{
    /* 
     * Needs to be a friend class to typeLex and typeAnalysis for 
     * implementation simplification reasons 
     */    
    friend class typeLex;
    friend class typeAnalysis;

public:
    /* 
     * Default constructor. Makes default type information
     * for the left-hand side, right-hand side and entireToken 
     * to be INT_TYPE
     */   
    binOpToken() {
        entireToken = INT_TYPE;
        left = INT_TYPE;
	right = INT_TYPE;
    }

private:    
    /*
     * Type information for LHS, RHS and entireToken
     */    
    varType entireToken;
    varType left; 
    varType right;
};

/*==============================================================================
 * class: singleOpToken
 *
 * singleOpToken is the class that holds all the information the lexical 
 * analyzer can gather from the semantic string of single operators (e.g. sqrt) 
 *============================================================================*/
class singleOpToken{
    /* 
     * Needs to be a friend class to typeLex and typeAnalysis for 
     * implementation simplification reasons 
     */ 
    friend class typeLex;
    friend class typeAnalysis;    
public:
    /* 
     * Default constructor. Makes default type information
     * for the parameter and entireToken to be INT_TYPE
     */  
    singleOpToken(){
        entireToken = INT_TYPE;
	parameter = INT_TYPE;    
    } 

private:
    /*
     * Type information for parameter and entireToken
     */
    varType entireToken;
    varType parameter;
};


/*==============================================================================
 * class: tokenType
 *
 * tokenType is the wrapper class that the parser actually accesses. It holds 
 * either a binOpToken or a singleOpToken or just a int value depending on the 
 * type of token this is. This class also holds additional information about
 * the token such as the source memory address, the instruction number for 
 * the particular address and the return value (which tells the parser how
 * to classify this token)
 *============================================================================*/
class tokenType {

    /* 
     * Needs to be a friend class to typeLex and typeAnalysis for 
     * implementation simplification reasons 
     */ 
    friend class typeLex;
    friend class typeAnalysis;   

public: 
    /*
     * Default constructor - does nothing
     */   
    tokenType(){
    }

private:
    /* 
     * tokenValue holds the actual token type information which is 
     * used by the parser
     */
    int tokenValue;
    
    /* 
     * Since only one of each of these are ever needed
     * needed by a token, we are using a union
     */
    union {
        binOpToken * thisBinOpToken;
        singleOpToken * thisSingleOpToken;
	int valueOne;
    };
    
    /*
     * Source address of where this token can be found
     */
    ADDRESS memAddress;
    
    /* 
     * Since one source address can have multiple RTLs, we want
     * to also store which RTL it is at the already specified 
     * memory address
     */
    Byte instructNumber;        
    /*
     * Addtional field to hold the 2nd part of a float in case
     * this token is holding information about a float
     */
    int valueTwo;
};

/*==============================================================================
 * class: typeLex
 *
 * This is the main lexical analyzer class. It contains the logic to match up 
 * the semantic strings to tokens. It also stores all the relevant information 
 * that can be inferred from the semantic string into its list of tokenType
 *
 * This class works with one RTL statement at a time. Since an RTL statment is
 * just a list of ints, the constructor to typeLex takes pointers to list of
 * ints. The class will tokenize the whole semantic string, and then will 
 * output the tokens one at a time through getNext().
 *============================================================================*/
class typeLex {
    /* 
     * Needs to be a friend class to typeAnalysis for 
     * implementation simplification reasons 
     */ 
    friend class typeAnalysis;

    public:
	/*
	 * Constructor for RTLs that have both a RHS and a LHS
	 */	        
	typeLex(list<int>* inLHS, 
		list<int>* inRHS,
		ADDRESS inMemAddress,
		Byte inInstructNumber); 
	/*
	 * Constructor for RTLs that just have a RHS
	 */	
	typeLex(list<int>* inRHS, 
		ADDRESS inMemAddress, 
		Byte inInstructNumber); // Single Op SemStr
	
	/*
	 * Default destructor
	 */
	~typeLex();

        /*
	 *
	 */ 

	/*
	 * Get the next token once the typeLex object has 
	 * been created. 
	 */
	int getNext();
		
    private:

	/*
	 * Internal function used to start the lexical analyzer
	 */    
	void lexBegin();    
               
	/*
	 * Adding an assignment token before passing the tokens 
	 * to the parser
	 */
        void addAssignment();

	/*
	 * Add a define token (used to distingish definations from
	 * other RTL instructions, as we can get additional information
	 * from definations)
	 */
        void addDefine();
	
	/*
	 * Tokenizes a list of ints to tokens
	 */	
        void tokenizeSSL(list<int>* thisList);
	
	/* 
	 * We only ever want to call the checkForBinOperator
	 * as that will automatically call the other checkFor 
	 * functions until a token type is found or an error 
	 * occurs
	 */
        void checkForBinOperator(list<int>::iterator& it,
				 tokenType& inToken);
	void checkForSingleOperator(list<int>::iterator& it,
				    tokenType& inToken);
        void checkForZeroOperation(list<int>::iterator& it,
		     		   tokenType& inToken);

        void checkForHLOperation(list<int>::iterator& it,
                                 tokenType& inToken);

        void checkForFixedOperation(list<int>::iterator& it,
		                    tokenType& inToken);
        void checkForThreeOperator(list<int>::iterator& it,
				   tokenType& inToken);
				   
        void checkForOperation(list<int>::iterator& it,
			       tokenType& inToken);
			       
        void checkForConstant(list<int>::iterator& it,
    			      tokenType& inToken);	
	void checkForFunction(list<int>::iterator& it,
    			      tokenType& inToken);
				 
        /*
	 * Internally used to hold pointer to LHS and RHS
	 */    
	list<int>* LHS;
	list<int>* RHS;
	
	/*
	 * Source address of RTL
	 */
	ADDRESS memAddress;
	
	/*
	 * Instruction number of RTL at the source address
	 */
	Byte instructNumber;
	
	/*
	 * Holds the list of tokens after the semantic string has 
	 * be lexically analyzed
	 */	
	list<tokenType> tokenList;
	
	/*
	 * Iterator for use with tokenList
	 */
	list<tokenType>::iterator tokenIt;
};
