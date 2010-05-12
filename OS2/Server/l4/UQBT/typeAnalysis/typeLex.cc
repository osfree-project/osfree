/*
 * Copyright (C) 2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * File:       typeLex.cc
 * OVERVIEW:   This file contains the implementation of the lexical analyzer's
 *             class member functions used for type analysis.
 *
 * Copyright (C) 2001, Sun Microsystems, Inc
 *============================================================================*/
 
 /*
 * $Revision: 1.8 $
 * 28 Aug 01 - Bernard: initial version
 */

#include "typeLex.h"
#include <string>
#include <list>
#include <iostream.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>


/**********************************************************************
  Current issues: Might get out of bounds of array if I'm not careful
		  with the it++
**********************************************************************/

/*
 *  Constructor with both a right hand side
 *  and a left hand side
 */
typeLex::typeLex(list<int>* inLHS, 
		 list<int>* inRHS, 
		 ADDRESS inMemAddress,
		 Byte inInstructNumber)  {		 
    LHS = inLHS;
    RHS = inRHS;
    memAddress = inMemAddress;
    instructNumber = inInstructNumber;
    lexBegin();  // Start tokenizing process
}

/*
 *  Constructor with just a right hand side
 */
typeLex::typeLex(list<int>* inRHS, 
		 ADDRESS inMemAddress, 
		 Byte inInstructNumber) {
    LHS = NULL;
    RHS = inRHS;
    memAddress = inMemAddress;    
    instructNumber = inInstructNumber;    
    lexBegin();  // Start tokenizing process
}

/*
 *  Destructor
 */
typeLex::~typeLex() {   
}


/*
 *  lexBegin starts off the tokenizing process of 
 *  each number in the SemStr list. 
 */
void typeLex::lexBegin() {
  
   /* This is what the following code achieves
    * 
    * LHS == NULL , RHS == NULL
    * Weird case
    *  
    * LHS == NULL, RHS != NULL
    * Single operation
    *  
    * LHS != NULL, RHS == NULL
    * Define
    *  
    * LHS != NULL, RHS != NULL
    * Normal assign
    */
   
   if (LHS != NULL && RHS !=NULL)   	
       addAssignment();

   if (RHS != NULL) {
        // We create a copy of the list since we
	// will be making temporary changes to it
   	list<int> thisRHS = *RHS;
   	tokenizeSSL(&thisRHS);
   }
   else {
        addDefine();
   }
   
   if (LHS != NULL){
        // We create a copy of the list since we
	// will be making temporary changes to it   
       list<int> thisLHS = *LHS;   
       tokenizeSSL(&thisLHS);  
   }
}

/*
 *  addAssignment adds an ASSIGN token to
 *  the tokenList
 */
void typeLex::addAssignment(){
   tokenType currentToken;    

   currentToken.tokenValue = typeAnalysis::ASSIGN;
   tokenList.push_back(currentToken);    
}

/*
 *  addAssignment adds an DEFINE token to
 *  the tokenList
 */
void typeLex::addDefine(){
   tokenType currentToken;    

   currentToken.tokenValue = typeAnalysis::DEFINE;
   tokenList.push_back(currentToken);    
}


/*
 *  Returns the next Token type from the tokenList. 
 *  The yylval holds the pointer to the semantic
 *  data of the token (as it is needed by the 
 *  bison++ generated parser)
 */
int typeLex::getNext(){

    if (tokenIt == tokenList.end())
        return typeAnalysis::EOL;
	
    
/*    typeAnalysis::yylval.term = &(*tokenIt++);
    typeAnalysis::yylval.term->memAddress = memAddress;
    
    return typeAnalysis::yylval.term->tokenValue;
*/    
    return (*tokenIt++).tokenValue;
}


/*
 *  tokenizeSSL is the member function that takes a list
 *  of ints, perform the lexical analysis and matches it 
 *  up to the correct tokens
 */ 
void typeLex::tokenizeSSL(list<int>* thisList) {

    list<int>::iterator it;         
    
    // Iterate through the list of ints
    for (it = thisList->begin(); it != thisList->end(); ++it){
    
        tokenType currentToken;    
	
	// Call checkForBinOperator, which checks whether
	// the Token is a BinaryOperator. If not, it will
        // automatically chain to different types of tokens
	// and eventually writing to the currentToken 
	// the result		
    	checkForBinOperator(it, currentToken); 
	
	// Put this at the at the back of the tokenList
	tokenList.push_back(currentToken);
    }
    
    // Set the tokenIt iterator to the beginning of the 
    // completed tokenList
    tokenIt = tokenList.begin();			
}


/*
 *  Given an integer from the list, match whether it is a 
 *  BinaryOperator and if it is, check whether there are 
 *  any additional info we can get from the instruction
 *
 *  Note that if the integer does not match with a
 *  BinaryOperator, it will automatically check it with
 *  the other possible types
 */
void typeLex::checkForBinOperator(list<int>::iterator& it,
				  tokenType& inToken){
				        
    // Make the type an BIN_OP now. It will be change to 
    // something else if it doesn't match with BIN_OP
    inToken.tokenValue = typeAnalysis::BIN_OP;
    
    // Create a binOpToken to hold the information that 
    // we may gather in the following switch statement.
    // If it turns out that this is not an BIN_OP, then
    // the binOpToken will be deleted
    inToken.thisBinOpToken = new binOpToken();
    				  				  
    switch(*it){

	// Note that the default is INT_TYPE, INT_TYPE, INT_TYPE
	case idPlus:   break; 	    
	case idMinus:  break;
    	case idMult:   break;
        case idDiv:    break;
	
	// We know that the return values of these floating point
	// instructions must be a floating point. Even though we
	// know that at least one of the 2 parameters is a float,
	// we don't know which one for sure so we don't guess.
        case idFPlus:   
	               (inToken.thisBinOpToken)->entireToken = FLOAT_TYPE;
		       break;
        case idFMinus:
	               (inToken.thisBinOpToken)->entireToken = FLOAT_TYPE;
		       break;
        case idFMult:
	               (inToken.thisBinOpToken)->entireToken = FLOAT_TYPE;
		       break;
        case idFDiv:   
	               (inToken.thisBinOpToken)->entireToken = FLOAT_TYPE;
		       break;
        case idFPlusd:
	               (inToken.thisBinOpToken)->entireToken = FLOAT_TYPE;
		       break;
        case idFMinusd:
	               (inToken.thisBinOpToken)->entireToken = FLOAT_TYPE;
		       break;
        case idFMultd:
	               (inToken.thisBinOpToken)->entireToken = FLOAT_TYPE;
		       break;
        case idFDivd:
	               (inToken.thisBinOpToken)->entireToken = FLOAT_TYPE;
		       break;
        case idFPlusq:
	               (inToken.thisBinOpToken)->entireToken = FLOAT_TYPE;
		       break;
        case idFMinusq:
	               (inToken.thisBinOpToken)->entireToken = FLOAT_TYPE;
		       break;
        case idFMultq:
	               (inToken.thisBinOpToken)->entireToken = FLOAT_TYPE;
		       break;
        case idFDivq:
	               (inToken.thisBinOpToken)->entireToken = FLOAT_TYPE;
		       break;
        case idFMultsd:
	               (inToken.thisBinOpToken)->entireToken = FLOAT_TYPE;
		       break;
        case idFMultdq:
	               (inToken.thisBinOpToken)->entireToken = FLOAT_TYPE;
		       break;

	case idMults:  break;
    	case idDivs:   break;
	case idMod:    break;
    	case idMods:   break;

        case idAnd:       break;
        case idOr:        break;
        case idEquals:    break;
        case idNotEqual:  break;
        case idLess:      break;
        case idGtr:       break;
        case idLessEq:    break;
        case idGtrEq:     break;
        case idLessUns:   break;
        case idGtrUns:    break;
        case idLessEqUns: break;
        case idGtrEqUns:  break;

        case idBitAnd:	  break;
        case idBitOr:	  break;
        case idBitXor:	  break;
        case idShiftL:	  break;
        case idShiftR:	  break;
        case idShiftRA:	  break;
        case idRotateL:	  break;
        case idRotateR:	  break;
        case idRotateLC:  break;
        case idRotateRC:  break;

	default:
	    // It doesn't match with as a BIN_OP, delete the
	    // binOpToken we created and check whether it is
	    // a SINGLE_OP	
	    delete(inToken.thisBinOpToken);
	    checkForSingleOperator(it, inToken);

    }
}


/*
 *  Given an integer from the list, match whether it is a 
 *  SINGLE_OP and if it is, check whether there are 
 *  any additional info we can get from the instruction
 */
void typeLex::checkForSingleOperator(list<int>::iterator& it,
				  tokenType& inToken){
				     
    // Make the type an SINGLE_OP now. It will be change to 
    // something else if it doesn't match with SINGLE_OP    
    inToken.tokenValue = typeAnalysis::SINGLE_OP;
    
    // Create a singleOpToken to store info in case it does match
    // as a SINGLE_OP. The token will be deleted if it actually
    // doesn't match as a SINGLE_OP
    inToken.thisSingleOpToken = new singleOpToken();
    
    switch(*it){
        case idSQRTs:  
			(inToken.thisSingleOpToken)->entireToken = FLOAT_TYPE;
			(inToken.thisSingleOpToken)->parameter = FLOAT_TYPE;
			break;			
        case idSQRTd:
			(inToken.thisSingleOpToken)->entireToken = FLOAT_TYPE;
			(inToken.thisSingleOpToken)->parameter = FLOAT_TYPE;
			break;
        case idSQRTq:  
			(inToken.thisSingleOpToken)->entireToken = FLOAT_TYPE;
			(inToken.thisSingleOpToken)->parameter = FLOAT_TYPE;
			break;
			
	case idNeg:    	break;
        case idNot:    	break;
        case idSignExt: break;
	
	default:
	
	    // It doesn't match as a SINGLE_OP, delete the
	    // binOpToken we created and check whether it is
	    // a ZERO_OP (which is actually an operation, 
	    // not an operator)	
	    delete(inToken.thisSingleOpToken);
	    checkForZeroOperation(it, inToken);	
    }	    
	
}


/*
 *  Given an integer from the list, match whether it is a 
 *  ZERO_OP and if it is, check whether there are 
 *  any additional info we can get from the instruction
 */
void typeLex::checkForZeroOperation(list<int>::iterator& it,
				        tokenType& inToken) {

    // Make the type an ZERO_OP now. It will be change to 
    // something else if it doesn't match up
    inToken.tokenValue = typeAnalysis::ZERO_OP;
    
    // Note that ZERO_OP does not have any extra semantic
    // information that we need
    
    switch(*it) {
        case idFpush: break;
	case idFpop:  break;
	case idZF:    break;
	case idCF:    break;
	case idNF:    break;
	case idOF:    break;
	case idFZF:   break;
	case idFLF:   break;
	case idFGF:   break;    
	case idCTI:   break;
	case idNEXT:  break;
	case idPC:    break;
	case idAFP:   break;
        case idAGP:   break;
    
        default:
		
	    // Check whether it is an HLCTI
	    checkForHLOperation(it, inToken);
    }
}	


/*
 *  Given an integer from the list, match whether it is a 
 *  HLCTI and if it is, check whether there are 
 *  any additional info we can get from the instruction
 */
void typeLex::checkForHLOperation(list<int>::iterator& it,
				        tokenType& inToken) {

    // Make the type an HLCTI now. It will be change to 
    // something else if it doesn't match up
    inToken.tokenValue = typeAnalysis::HLCTI;
    
    // Note that HLCTI does not have any extra semantic
    // information that we need
    
    switch(*it) {
        case idHLCTI:   
	                inToken.tokenValue = typeAnalysis::HLCTI;
	                break;		
	case idDEFINE:
	                inToken.tokenValue = typeAnalysis::DEFINE;
			break;
    
        default:		
	    // Check whether it is an FIXED_OP
	    checkForFixedOperation(it, inToken);
    }
}


/*
 *  Given an integer from the list, match whether it is a 
 *  FIXED_OP and if it is, check whether there are 
 *  any additional info we can get from the instruction
 */
void typeLex::checkForFixedOperation(list<int>::iterator& it,
				        tokenType& inToken) {
    // Make the type an FIXED_OP now. It will be change to 
    // something else if it doesn't match up					
    inToken.tokenValue = typeAnalysis::FIXED_OP;
    
    // The it++ instruction is used as the number following 
    // the fixed operation is a constant number. Since this
    // constant number is not very useful in our analysis,
    // we'll just throw it away for now
    //
    // Note: We really should check for array bounds when
    //       we do the it++. It's not 100% safe at the moment       
    switch(*it) {
        case idParam:   // Skip next NUM
			it++;
			break;
	case idRparam:  // Skip next NUM
			it++;
			break;
	case idExpand:  // Skip next NUM
			it++;
			break;
	case idTemp:
                        /* Ugly Hack, make tmps start at 250 (and goes 
			   downwards)
			*/			
			inToken.tokenValue = typeAnalysis::INT_CONSTANT;
			inToken.valueOne = *(++it) - 135 + 255;
			
			/* inToken.tokenValue = typeAnalysis::TMP;*/
			// Skip next NUM
			// it++;			
			break;    
	case 137: 
			inToken.tokenValue = typeAnalysis::TMP;
			// This is not really a TMP but actually
			// it is a %Y register. Just use TMP for
			// now
			break;
	
        default:				
	    checkForThreeOperator(it, inToken);	
    }
}	    

/*
 *  Given an integer from the list, match whether it is a 
 *  THREE_OP and if it is, check whether there are 
 *  any additional info we can get from the instruction
 *
 *  Note, for our analysis, we don't really care about THREE_OP
 */
void typeLex::checkForThreeOperator(list<int>::iterator& it,
				        tokenType& inToken) {

    // Make the type an THREE_OP now. It will be change to 
    // something else if it doesn't match up					
    inToken.tokenValue = typeAnalysis::THREE_OP;
    
    switch(*it) {
        case idAt:    break;
	case idTern:  break;
    
        default:
	    checkForOperation(it, inToken);	
    }				  
}				 
				  
/*
 *  Given an integer from the list, match whether it is a 
 *  an operation. Since operations are probably the most 
 *  important to our analysis, it is broken up into many 
 *  different types of tokens
 */
void typeLex::checkForOperation(list<int>::iterator& it,
				  tokenType& inToken){
    
    /* Used for idVar */
    int nextValue;
    switch(*it){
    
        case idMemOf:
			inToken.tokenValue = typeAnalysis::MEM_OF;
			break;
        case idRegOf:  		
			inToken.tokenValue = typeAnalysis::REG_OF;
			break;
        case idAddrOf:
			inToken.tokenValue = typeAnalysis::ADDR_OF;
			break;
        case idVar:  
                        /* Ugly hack, should actually be typeAnalysis::VAR_OF
			*/
			
			inToken.tokenValue = typeAnalysis::REG_OF;
			
			/* First change the next num to 180 + num */
			nextValue = *(++it);
			*(it) = nextValue + 180;
			
			/* Now change the current it to a idIntConsts */
			*(--it) = idIntConst;
			
			/* Now rewind one more time so that this
			   will be done again */
			--it;			
						  
			/*inToken.tokenValue = typeAnalysis::VAR_OF;
			// Skip next NUM token
			inToken.valueOne = *(++it);
			*/
			break;
	case idSize:
			inToken.tokenValue = typeAnalysis::SIZE;
			inToken.valueOne = *(++it);
			break;
	case idCastIntStar:
			inToken.tokenValue = typeAnalysis::CAST_INT_STAR;
			break;
	default:	
	    checkForFunction(it, inToken);	
    }	    	    	
}


/*
 *  Given an integer from the list, match whether it is a 
 *  a Function. Since functions are also very important 
 *  for our analysis, it too is broken up into many  
 *  different types of tokens
 */
void typeLex::checkForFunction(list<int>::iterator& it,
    				        tokenType& inToken){	

    switch(*it){
    
        case idTruncu:
			inToken.tokenValue = typeAnalysis::TRUNCU;
			// Skip next 2 NUMs. Keep
			// the value in valueOne and 
			// valueTwo just in case we
			// may need them
			inToken.valueOne = *(++it);
			inToken.valueTwo = *(++it);
			break;
        case idTruncs:  		
			inToken.tokenValue = typeAnalysis::TRUNCS;
			// Skip next 2 NUMs
			inToken.valueOne = *(++it);
			inToken.valueTwo = *(++it);			
			break;
        case idZfill:
			inToken.tokenValue = typeAnalysis::ZFILL;
			// Skip next 2 NUMs
			inToken.valueOne = *(++it);
			inToken.valueTwo = *(++it);			
			break;
        case idSgnEx:  
			inToken.tokenValue = typeAnalysis::SGNEX;
			// Skip next 2 NUMs
			inToken.valueOne = *(++it);
			inToken.valueTwo = *(++it);			
			break;
	case idFsize:
			inToken.tokenValue = typeAnalysis::FSIZE;
			// Skip next 2 NUMs
			inToken.valueOne = *(++it);
			inToken.valueTwo = *(++it);			
			break;
	case idItof:
			inToken.tokenValue = typeAnalysis::ITOF;
			// Skip next 2 NUMs
			inToken.valueOne = *(++it);
			inToken.valueTwo = *(++it);			
			break;
	case idFtoi:
			inToken.tokenValue = typeAnalysis::FTOI;
			// Skip next 2 NUMs
			inToken.valueOne = *(++it);
			inToken.valueTwo = *(++it);			
			break;			
	case idFround:
			inToken.tokenValue = typeAnalysis::FROUND;
			// Skip next 2 NUMs
			inToken.valueOne = *(++it);
			inToken.valueTwo = *(++it);			
			break;			
        case idForceInt:
			inToken.tokenValue = typeAnalysis::TOINT;
			break;			
	case idForceFlt:
			inToken.tokenValue = typeAnalysis::TOFLT;
			break;	
	case idSin:
			inToken.tokenValue = typeAnalysis::SIN;
			break;	
	case idCos:	
			inToken.tokenValue = typeAnalysis::COS;
			break;	
	case idTan:
			inToken.tokenValue = typeAnalysis::TAN;
			break;	
	case idArcTan:
			inToken.tokenValue = typeAnalysis::ARCTAN;
			break;	
	case idLog2:
			inToken.tokenValue = typeAnalysis::LOG2;
			break;	
	case idLog10:
			inToken.tokenValue = typeAnalysis::LOG10;
			break;	
	case idLoge:
			inToken.tokenValue = typeAnalysis::LOGE;
			break;	
	case idSqrt:
			inToken.tokenValue = typeAnalysis::SQRT;
			break;	
	case idExecute:
			inToken.tokenValue = typeAnalysis::EXECUTE;
			break;	
							
	default:
	    checkForConstant(it, inToken);
    }	    

}	


/*
 *  Constants are the last thing we check for. If it isn't
 *  a constant, than we have a mistake in the lexical analysis
 *  or the input stream is wrong.
 *
 *  Note that the float constant takes 2 numbers
 */
				
void typeLex::checkForConstant(list<int>::iterator& it,
    				        tokenType& inToken){
					
    switch(*it) {
        case idIntConst:
			inToken.tokenValue = typeAnalysis::INT_CONSTANT;
			// Skip next NUM symbol
			inToken.valueOne = *(++it);
			break;
	case idFltConst:
			inToken.tokenValue = typeAnalysis::FLOAT_CONSTANT;
			// Skip next 2 NUM symbol
			inToken.valueOne = *(++it);
			inToken.valueTwo = *(++it);
			break;
	case idCodeAddr:
			inToken.tokenValue = typeAnalysis::ADDR_CONSTANT;
			// Skip next NUM symbol
			inToken.valueOne = *(++it);
			break;
    					
	default:
	    /* lex error */
	    printf("Lexical Analyzer Error! Cannot find symbol %d\n", *it);
    }
}
