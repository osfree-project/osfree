//
// Copyright (C) 2001, Sun Microsystems, Inc
//
// See the file "LICENSE.TERMS" for information on usage and
// redistribution of this file, and for a DISCLAIMER OF ALL
// WARRANTIES.
//

%name typeAnalysis     // the parser class name will be typeAnalysis

// stuff to go in typeAnalysis.h
%header{
#include "global.h"
// #include "analyzeBlocks.h"
// #include "typeLex.h"
class typeLex;
class tokenType;
%}

%{
#include "typeLex.h"
%}

//============================================================================
// Declaration of semantic value type. 
//============================================================================
%union {
tokenType *term;     /* For returning terminals                  */
varType nonterm;   /* For returning symbol-table pointers      */
}

%define DEBUG 1 

/* %define PARSE_PARAM \
    RTLInstDict& Dict
*/    

%define CONSTRUCTOR_PARAM \
    typeLex * paramLex,  \
    BBBlock * paramBlock, \
    varType inVarType    

/* %define CONSTRUCTOR_INIT : \
   sslFile(sslFile), bFloat(false)
*/   

%define CONSTRUCTOR_CODE \
    inLex = paramLex; \
    inBlock = paramBlock; \
    globalUse = DEFINE_TYPE; \
    globalType = INT_TYPE; \
    binOpType = NULL; \
    aliasBoolean = false; \
    inTypeHint = inVarType; \
    inTypeHintOn = false;

%define MEMBERS \
public: \
    virtual ~typeAnalysis();            \
private: \
    /* Note that globalType values have only valid		\
       value to the immediate next token grouping		\
   								\
       After the next immediate token grouping, the 		\
       value of these global variables will be unknown		\
       until they are explicitly assigned again			\
    */   							\
    defUse globalUse;     \
    varType globalType;   \
    varType prevType;     \
    Byte prevRegNum;      \
    tokenType* binOpType; \
    bool aliasBoolean;    \
    Byte aliasReg;        \
    varType inTypeHint;   \
    bool inTypeHintOn;    \
    void savePrevType(Byte inRegNum, varType inType); \
    void rollbackToPrevType(varType inType);          \
protected: \
\
    typeLex * inLex; \
    BBBlock * inBlock;


%token <term> EOL ASSIGN MEM_OF BIN_OP REG_OF ADDR_OF VAR_OF THREE_OP
%token <term> TMP INT_CONSTANT FLOAT_CONSTANT ADDR_CONSTANT SIZE 
%token <term> CAST_INT_STAR SINGLE_OP ZERO_OP FIXED_OP
%token <term> TRUNCU TRUNCS ZFILL SGNEX FSIZE
%token <term> ITOF FTOI FROUND TOINT TOFLT 
%token <term> SIN COS TAN ARCTAN LOG2 LOG10 LOGE SQRT EXECUTE 
%token <term> HLCTI DEFINE

%type <nonterm> assignment, highleveljump, operation, function, constants
%type <nonterm> variable, binsecondparamvariable, binsecondparamconstant, binfirstparam
%type <nonterm> assignrightside, assignleftsidealias
%%

/* program:
        EOP {
	    $$ = INT_TYPE;
	}
    |   program {globalUse = DEFINE_TYPE; globalType = INT_TYPE;} assignment 
    ;
*/    

assignment:
        {globalUse = USE_TYPE;} operation EOL 
    |	DEFINE {globalUse = DEFINE_TYPE; globalType = INT_TYPE} operation EOL {
        /* Used for param defination. The operation is for the parameters
	   and it is always a define */ 
	}
    |   ASSIGN assignrightside{
        }
    |   {globalUse = USE_TYPE;} highleveljump EOL    
    ;
    
assignleftsidealias:    
        REG_OF INT_CONSTANT EOL{
	    aliasBoolean = true;
	    aliasReg = $2->valueOne;
            inBlock->addUDElementReg($2->valueOne, 
	                             $2->memAddress, 
		   	             $2->instructNumber, 
				     DEFINE_TYPE);	     
				      
            if (globalType > ((inBlock->returnRegChainListAt($2->valueOne))->back())->chainType) {
	        inBlock->setTypeOfLastChainForReg($2->valueOne, globalType);	     	     
            }		    
	    
	    if (inTypeHintOn){
	        if (inTypeHint > ((inBlock->returnRegChainListAt($2->valueOne))->back())->chainType){
                    inBlock->setTypeOfLastChainForReg($2->valueOne, inTypeHint);				
		}	
	    }
	
	}
    |   operation EOL{
        }
    ;
           
assignrightside:
        REG_OF INT_CONSTANT {
	    aliasBoolean = false;
	    inBlock->addUDElementReg($2->valueOne, 
	                             $2->memAddress, 
				     $2->instructNumber, 
				     USE_TYPE);

	    /* Make sure there is some priority of assignment
	       i.e. FLOAT_TYPE > POINTER_I > POINTER_D > INT

	       We don't want to overwrite a higher priority
	       type over a lower prority type (because 
	       we are then losing information on the chain 
	    */
	    if (globalType > ((inBlock->returnRegChainListAt($2->valueOne))->back())->chainType) {
	       inBlock->setTypeOfLastChainForReg($2->valueOne, globalType);	     	     
	    }	           
	    else {
	       globalType = ((inBlock->returnRegChainListAt($2->valueOne))->back())->chainType;						  
            }
            globalUse = DEFINE_TYPE;
	    inTypeHintOn = true;	    	    
	} assignleftsidealias{
	    if (aliasBoolean){
	        inBlock->aliasRegLink((Byte)($2->valueOne), aliasReg);
	    }
	    inTypeHintOn = false;
	}
    |   {globalUse = USE_TYPE;}operation {globalUse = DEFINE_TYPE; inTypeHintOn = true;
                                          globalType = $2; } operation EOL {
            inTypeHintOn = false;
	}
    |   {globalUse = USE_TYPE;}highleveljump {globalUse = DEFINE_TYPE; inTypeHintOn = true;
                                              globalType = $2; } operation EOL {	
            inTypeHintOn = false;
        }
    ;
 
highleveljump:
        HLCTI {globalType = POINTER_I} operation {
            /* LATER WE NEED TO GET TYPE INFO FROM HIGHLEVELJUMP */
	    /* First operation is for the address we are jumping to.*/
	    $$ = INT_TYPE;
        }
    ;
    
/*paramlist:
        empty  FIX THIS LATER
    |   paramlist operation
    ;
*/

variable:
        MEM_OF {globalUse = USE_TYPE; globalType = POINTER_D;} operation {
	     $$ = INT_TYPE;	    
	}
    |   REG_OF INT_CONSTANT {
        
	     list<UDChain*>* listChainPtr = inBlock->returnRegChainListAt($2->valueOne);
	     if (listChainPtr == NULL) {
	         if (inTypeHintOn){
	             savePrevType($2->valueOne, inTypeHint);		 
		 }
		 else {
	             savePrevType($2->valueOne, INT_TYPE);
		 }
             }
	     else {
	         UDChain* chainPtr = listChainPtr->back();
		 if (chainPtr == NULL){
	             if (inTypeHintOn){
	        	 savePrevType($2->valueOne, inTypeHint);
                     }		 
		     else {
	        	 savePrevType($2->valueOne, INT_TYPE);
		     }	
		 }
		 else{
    		     if (inTypeHintOn){
	        	 if (inTypeHint > chainPtr->chainType){
                	     inBlock->setTypeOfLastChainForReg($2->valueOne, inTypeHint);				
			 }	
		     } 		 
                     savePrevType($2->valueOne, chainPtr->chainType);
		 }	 
	     }    
     	     
    	     /* globalUse should hold the info that's needed to 
	        determine if the register is a Use or Define
	      */
    	     	     
             inBlock->addUDElementReg($2->valueOne, 
	                              $2->memAddress, 
				      $2->instructNumber, 
				      globalUse);
             
	     /* Make sure there is some priority of assignment
	        i.e. FLOAT_TYPE > POINTER_I > POINTER_D > INT
		
		We don't want to overwrite a higher priority
		type over a lower prority type (because 
		we are then losing information on the chain 
	     */
	     if (globalType > ((inBlock->returnRegChainListAt($2->valueOne))->back())->chainType) {
		inBlock->setTypeOfLastChainForReg($2->valueOne, globalType);	     	     
	     }	      
	     
	     $$ = ((inBlock->returnRegChainListAt($2->valueOne))->back())->chainType;
	     
    	}
    |   REG_OF TMP {
    	    /* We don't currently handle this */
	    $$ = globalType;	   
    	}
    
    |   ADDR_OF {globalUse = USE_TYPE} operation {
	    $$ = POINTER_D;
    	}
    |   VAR_OF  {    
            /* Don't know what to do with this yet */ 
	    $$ = globalType;
	}
    ;		
    
/* ADD STUFF HERE TOMMORROW!!!!*/    
binsecondparamconstant:
        constants {
	    $$ = INT_TYPE;  /* Just make it INT	*/
	}
    |   variable {
            $$ = INT_TYPE;  /* Just make it INT */
        }
    ;
binsecondparamvariable:    
        constants {
	    $$ = INT_TYPE;  /* Just make it INT	*/
	}
    |   {rollbackToPrevType(binOpType->thisBinOpToken->left)} variable {
            $$ = INT_TYPE;  /* Just make it INT */
        }
    ;
        
binfirstparam:
        constants {if (globalType < POINTER_D) globalType = binOpType->thisBinOpToken->right;}
	binsecondparamconstant{
	    $$ = INT_TYPE;  /* Just make it INT */
	}
    |	{if (globalType < POINTER_D) globalType = binOpType->thisBinOpToken->left;} variable 
        {globalType = binOpType->thisBinOpToken->right;} binsecondparamvariable{
	    $$ = INT_TYPE;  /* Just make it INT	*/
        }
    ;	
      
operation:
        variable {
            $$ = $1	
	}
    |   THREE_OP operation operation operation {
            /* Don't handle this yet */
	    $$ = globalType;
	}
    |   TMP { 
            /* Don't handle this yet */
	    $$ = globalType;
	}  
    |   constants {
            $$ = $1;
	}
    |   function {
            $$ = $1;
        }
    |   SIZE operation {
    	    /* Not handling this 100% correct yet */
            $$ = $2;
        }
    |   CAST_INT_STAR {if (globalType != POINTER_I ) globalType = POINTER_D;} operation {            
            $$ = $3;
	}	     
    |   SINGLE_OP {globalType = $1->thisSingleOpToken->parameter} operation {
    	    if ($3 > $1->thisSingleOpToken->entireToken)
	        $$ = $3;
	    else 
	        $$ = $1->thisSingleOpToken->entireToken;
        }
    |   ZERO_OP {
            $$ = INT_TYPE;    
        }
    |   FIXED_OP {
            $$ = INT_TYPE;
        }
    |   BIN_OP {binOpType = $1} binfirstparam {
            $$ = $1->thisBinOpToken->entireToken
        }
    ;

function:
        TRUNCU operation {$$ = INT_TYPE; }
    |   TRUNCS operation {$$ = INT_TYPE; }
    |   ZFILL operation {$$ = INT_TYPE; }
    |   SGNEX operation {$$ = INT_TYPE; }
    |   FSIZE {globalType = FLOAT_TYPE;} operation {$$ = FLOAT_TYPE; } /* Need to look this up later */
    |   ITOF {globalType = INT_TYPE;} operation {$$ = FLOAT_TYPE; }
    |   FTOI {globalType = FLOAT_TYPE;} operation {$$ = INT_TYPE; }
    |   FROUND {globalType = FLOAT_TYPE;} operation {$$ = INT_TYPE; }
    |   TOINT operation {$$ = INT_TYPE; }
    |   TOFLT operation {$$ = FLOAT_TYPE; }
    |   SIN operation {$$ = FLOAT_TYPE; }
    |   COS operation {$$ = FLOAT_TYPE; }
    |   TAN operation {$$ = FLOAT_TYPE; }
    |   ARCTAN operation {$$ = FLOAT_TYPE; }
    |   LOG2 operation {$$ = FLOAT_TYPE; }
    |   LOG10 operation {$$ = FLOAT_TYPE; }
    |   LOGE operation {$$ = FLOAT_TYPE; }
    |   SQRT operation {$$ = FLOAT_TYPE; }
    |   EXECUTE {globalType = POINTER_I;} operation {$$ = INT_TYPE; } /* What does an execute return? */
    ;
    
constants:
        INT_CONSTANT {
	    /* We probably want to get my info from this
	       later when we want to do constant propagation
	     */	
            $$ = INT_TYPE;
        } 
    |   FLOAT_CONSTANT {

	    $$ = FLOAT_TYPE;
	}
    |   ADDR_CONSTANT {
   	    $$ = POINTER_D;
	} 
    ;
    
%%

/*==============================================================================
 * FUNCTION:        yylex
 * OVERVIEW:        The scanner driver than returns the next token.
 * PARAMETERS:      <none>
 * RETURNS:         the next token
 *============================================================================*/
int typeAnalysis::yylex()
{
    if (inLex->tokenIt == inLex->tokenList.end())
        return EOL;
 
    yylval.term = &(*(inLex->tokenIt)++);
    yylval.term->memAddress = inLex->memAddress;
    yylval.term->instructNumber = inLex->instructNumber;
    return yylval.term->tokenValue;
}

typeAnalysis::~typeAnalysis(){
}

void typeAnalysis::yyerror(char * inString){
}

void typeAnalysis::savePrevType(Byte inRegNum, varType inType){
    prevRegNum = inRegNum;
    prevType = inType;      
    // Note that prevType and prevRegNum are 
    // temporary. Only use within the same instruction
}

void typeAnalysis::rollbackToPrevType(varType inType){
    if (inType > prevType){
        inBlock->setTypeOfLastChainForReg(prevRegNum, inType);    
    }
    else{
        inBlock->setTypeOfLastChainForReg(prevRegNum, prevType);
    }
}
