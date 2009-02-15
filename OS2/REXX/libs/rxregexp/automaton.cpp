/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2006 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.oorexx.org/license.html                          */
/*                                                                            */
/* Redistribution and use in source and binary forms, with or                 */
/* without modification, are permitted provided that the following            */
/* conditions are met:                                                        */
/*                                                                            */
/* Redistributions of source code must retain the above copyright             */
/* notice, this list of conditions and the following disclaimer.              */
/* Redistributions in binary form must reproduce the above copyright          */
/* notice, this list of conditions and the following disclaimer in            */
/* the documentation and/or other materials provided with the distribution.   */
/*                                                                            */
/* Neither the name of Rexx Language Association nor the names                */
/* of its contributors may be used to endorse or promote products             */
/* derived from this software without specific prior written permission.      */
/*                                                                            */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS        */
/* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT          */
/* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS          */
/* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT   */
/* OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,      */
/* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,        */
/* OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY     */
/* OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING    */
/* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS         */
/* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.               */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/******************************************************************************/
/* Object REXX Support                                         automaton.cpp  */
/*                                                                            */
/* Regular Expression Utility functions                                       */
/*                                                                            */
/******************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "automaton.hpp"
#include "regexp.hpp"

#ifdef OPSYS_AIX41
#include <rexx.h>

#define bool   BOOL
#define false  FALSE
#define true   TRUE
#endif

// constructor: initialize automaton
automaton::automaton() : ch(NULL), next1(NULL), next2(NULL), final(-1), regexp(NULL),
                         setArray(NULL), setSize(0), size(16), freeState(1), currentPos(0), minimal(false)
{
  int bytes = sizeof(int)*size;

  ch    = (int*) malloc(bytes);
  next1 = (int*) malloc(bytes);
  next2 = (int*) malloc(bytes);
}

// destructor: free memory
automaton::~automaton()
{
  if (size) {
    free(ch);
    free(next1);
    free(next2);
  }
  if (setSize) {
    for (int i=0;i<setSize;i++)
      free(setArray[i]);
    free(setArray);
  }

}


/*************************************************************/
/* automaton::setMinimal                                     */
/*                                                           */
/* make the automaton match minimal or maximal               */
/* this influences the state machine.                        */
/*************************************************************/
void automaton::setMinimal(bool f)
{
  if (f != this->minimal) {
    if (this->final != -1) {
      if (f == false) {
        setState(this->final, 0x00, this->final+1, this->final+1);
      } else {
        setState(this->final, EPSILON, EOP, EOP);
      }
    }
    this->minimal = f;
  }
}


/*************************************************************/
/* automaton::parse                                          */
/*                                                           */
/* creates the automaton from the regular expression. if an  */
/* error occurs during parsing, the error number is returned */
/* and the automaton will match any pattern.                 */
/* parsing is recursive.                                     */
/*************************************************************/
int automaton::parse(char *regexp)
{
  int temp;
  this->regexp = regexp;
  currentPos = 0;
  freeState  = 1;
  memset(ch,    0x00, sizeof(int)*size);
  memset(next1, 0x00, sizeof(int)*size);
  memset(next2, 0x00, sizeof(int)*size);
  if (setSize) {
    for (int i=0;i<setSize;i++)
      free(setArray[i]);
    free(setArray);
    setSize = 0;
    setArray = NULL;
  }

  try {
    // parse expression recursively by splitting it up
    temp = expression();
    // using a temporary place to store the start state
    // is needed since next1 may change during reallocation
    next1[0] = temp;
  } catch (RE_ERROR err) {
    this->regexp = NULL;
    // an error occured!
    setState(0, EPSILON, 0, 0);  // make automaton match anything
    return (int) err;
  }
  // set start state
  setState(0, EPSILON, next1[0], next1[0]);
  this->final = freeState;
  if (minimal == false) {
    // zero-terminate the expression...
    setState(freeState, 0x00, freeState+1, freeState+1);
    freeState++;
  } else {
    // this ends the pattern, but we need a dummy
    // show the regexp be switched between min and max matching
    setState(freeState, EPSILON, EOP, EOP);
    freeState++;
  }
  // ...and set epsilon transition to end state
  setState(freeState, EPSILON, EOP, EOP);

// in debug mode, print out the automaton
#ifdef MYDEBUG
  printf("S\tT\t1\t2\n");
  for (int i=0;i<size && next1[i] != EOP;i++) {
    if ((unsigned int) next1[i] < 32000) {
    printf("%d\t",i);
    switch (ch[i] & SCAN) {
    case ANY:
      printf("any\t");
      break;
    case SET:
      printf("set %d\t",(ch[i] & 0x0fff0000) >> 16);
      break;
    case SET|NOT:
      printf("nset %d\t",(ch[i] & 0x0fff0000) >> 16);
      break;
    case EPSILON:
      printf("eps\t");
      break;
    default:
      printf("%c\t",ch[i]);
      break;
    }
    if ( (ch[i] & SCAN) == EPSILON )
      if (next1[i] != next2[i])
        printf("%d\t%d\n",next1[i],next2[i]);
      else
        printf("%d\n",next1[i]);
    else
      printf("%d\n",next1[i]);
    }
  }
#endif

  this->regexp = NULL;  // contents only guaranteed during
                        // runtime of this method
  return 0;
}

/***********************************************************/
/* automaton::letter                                       */
/*                                                         */
/* check if the given character is a letter or a operator. */
/***********************************************************/
int automaton::letter(int c)
{
  int r;

  switch (c) {
  case '(':
  case ')':
  case '{':
  case '}':
  case '|':
  case '*':
  case '+':
  case '[':
  case ']':
  //case '?':
  case 0x00:
    r=0;
    break;
  default:
    r=1;
  }
  return r;
}

/**************************************************************/
/* automaton::expression                                      */
/*                                                            */
/* dissect expression into smaller parts (expression => term) */
/* and insert fitting states into the automaton.              */
/**************************************************************/
int automaton::expression()
{
  int t1, t2;
  int r;

  t1 = term();
  r = t1;
  // is the current character an 'or' operator?
  if (regexp[currentPos] == '|') {
    currentPos++;
    freeState++;
    r = t2 = freeState;
    freeState++;
    // set epsilon transitions, evaluate right expression
    // (left one has already been processed at this point)
    setState(t2, EPSILON, expression(), t1);
    setState(t2-1, EPSILON, freeState, freeState);
    setState(t1-1, ch[t1-1], t2, next2[t1-1]);
  }
  return r;
}

/*******************/
/* automaton::term */
/*                 */
/* parse a term.   */
/*******************/
int automaton::term()
{
  int r;

  r = factor();

  // do we need to concatenate parenthesis?
  // parenthesis need an extra epsilon transition so that | works correctly
  if ( (regexp[currentPos] == '(') ) {
    int t1 = freeState, t2;
    freeState++;
    t2 = term();
    setState(t1, EPSILON, t2, t2);
  } else if ((regexp[currentPos] == '[') ||
             letter(regexp[currentPos])) {
    term(); // simple concat
  }

  return r;
}

/***************************************************************/
/* automaton::factor                                           */
/*                                                             */
/* create new states for the automaton according to meaning of */
/* single characters.                                          */
/* known operators are '?', '\', '[', '(', '{', '*', '+'       */
/***************************************************************/
int automaton::factor()
{
  int t1, t2;
  int r;

  t1 = freeState;
  switch (regexp[currentPos]) {
  // match any single character
  case '?':
    setState(freeState, ANY, freeState+1, freeState+1);
    r = t2 = freeState;
    freeState++;
    currentPos++;
    break;
  // escape operator, use next character literally
  case '\\':
    currentPos++;
    if (regexp[currentPos] == 0x00) throw E_UNEXPECTED_EOP;
    setState(freeState, regexp[currentPos], freeState+1, freeState+1);
    t2 = freeState;
    freeState++;
    currentPos++;
    break;
  // set definition
  case '[':
    currentPos++;
    t2 = set();
    if (regexp[currentPos] == ']') currentPos++;
    else throw E_ILLEGAL_SET;
    break;
  // an expression in parenthesis
  case '(':
    currentPos++;
    t2 = expression();
    if (regexp[currentPos] == ')') currentPos++;
    else throw E_MISSING_PAREN_CLOSE;
    break;
  default:
    // only a letter is valid at this position
    if (letter(regexp[currentPos])) {
      setState(freeState, regexp[currentPos], freeState+1, freeState+1);
      t2 = freeState;
      freeState++;
      currentPos++;
    } else throw E_UNEXPECTED_SYMBOL;
  }

  // the next statements are left out of the switch-block because
  // they are all postfix operators.
  // if they are used without operands, E_UNEXPECTED_SYMBOL is
  // caused in the switch-block

  // match previous expression zero or more times
  if (regexp[currentPos] == '*') {
    setState(freeState, EPSILON, freeState+1, t2);
    r = freeState;
    next1[t1-1] = freeState;
    freeState++;
    currentPos++;
  }
  // match previous expression one or more times
  else if (regexp[currentPos] == '+') {
    setState(freeState, EPSILON, t1, freeState+1);
    r = t1;
    freeState++;
    currentPos++;
  }
  // match previous expression exactly n times
  else if (regexp[currentPos] == '{') {
    char stringBuffer[64];
    int tempPos = currentPos+1;
    int i = 0;
    int j,k, size;
    bool orExpression = t1!=t2;

    r = t2;

    // get n
    while (regexp[tempPos] && regexp[tempPos] != '}') {
      stringBuffer[i] = regexp[tempPos];
      tempPos++;
      if (i<62) i++;
    }

    if (regexp[tempPos] == 0x00) throw E_UNEXPECTED_EOP;
    stringBuffer[i] = 0x00;
    i = atoi(stringBuffer) - 1;
    if (i <= 0) throw E_ILLEGAL_NUMBER;

    // the previous expression will be copied and insert
    // n times into the automaton
    size = freeState - t1;
    if (orExpression)
      size++;
    while (i--) {
      k = freeState;
      if (orExpression) {
        t2 += size;
        setState(freeState++, EPSILON, t2, t2);
      }
      for (j=t1;j<k;j++,freeState++)
        setState(freeState, ch[j], next1[j]+size, next2[j]+size);
      t1 += size;
    }

    currentPos = tempPos + 1;
  } else r = t2;
  return r;
}

/**********************************************************/
/* automaton::set                                         */
/*                                                        */
/* process a set definition.                              */
/* this parses the the definition and creates a character */
/* array that contains all specified characters.          */
/* if this is an exclusive match, the NOT bit of the SET  */
/* transition will be set.                                */
/**********************************************************/
int automaton::set()
{
  int i=0;
  int transition = SET;
  int length = 256;                 // pre-allocation length is 256 bytes
                                    // if more are need, reallocation is used
  char *ptr = regexp+currentPos;
  char *range = (char*) malloc(sizeof(char)*length);
#ifdef WIN32
  int (__cdecl *func)(int) = NULL;     // function pointer for symbolic names
#else
  int (*func)(int) = NULL;
#endif
  // is this an exclusive definition?
  if (*ptr == '^') {
    transition |= NOT;
    ptr++;
  }

  // parse the set definition
  while (*ptr && *ptr != ']') {
    // '-' operator: go from last specified character
    // to next specified character (fill in all in-between)
    // (a '-' at the beginning is interpreted as a simple character)
    if (*ptr == '-' && i) {
      char b=ptr[-1], e=ptr[1];
      if (e == 0x00 || e == ']') throw E_ILLEGAL_SET;
      ptr+=2;
      // start character > end character? then swap them
      if (b>e) {
        char t=b;
        b=e; e=t;
      }
      // fill up the range
      for (char c=b+1;c<=e;c++) {
        if (checkRange(range,i,c)) range[i++] = c;
        if (i == length) {
          range = (char*) realloc(range,length*2);
          length *= 2;
        }
      }
    }
    // escape operator
    else if (*ptr == '\\') {
      ptr++;
      if (*ptr == 0x00) throw E_ILLEGAL_SET;
      if (checkRange(range,i,*ptr)) range[i++] = *ptr;
      if (i == length) {
        range = (char*) realloc(range,length*2);
        length *= 2;
      }
      ptr++;
    }
    // symbolic name?
    else if (*ptr == ':') {
      int len=0;
      char buffer[16];
      ptr++;
      while (len<15 && *ptr && *ptr != ':')
        buffer[len++] = *ptr++;
      buffer[len] = 0x00;
      if (*ptr != ':') throw E_ILLEGAL_SYMBOLIC_NAME;
      ptr++;
      // make symbolic name uppercase
      while (len>=0) {
        buffer[len] = toupper((int) buffer[len]);
        len--;
      }
      // keyword ALPHA
      if (!strcmp(buffer,"ALPHA"))
        func = isalpha;
      // keyword LOWER
      else if (!strcmp(buffer,"LOWER"))
        func = islower;
      // keyword UPPER
      else if (!strcmp(buffer,"UPPER"))
        func = isupper;
      // keyword ALNUM
      else if (!strcmp(buffer,"ALNUM"))
        func = isalnum;
      // keyword DIGIT
      else if (!strcmp(buffer,"DIGIT"))
        func = isdigit;
      // keyword XDIGIT
      else if (!strcmp(buffer,"XDIGIT"))
        func = isxdigit;
      // keyword BLANK
      else if (!strcmp(buffer,"BLANK")) {
        // whitespace...
        if (checkRange(range,i,' ')) range[i++] = ' ';
        if (i == length) {
          range = (char*) realloc(range,length*2);
          length *= 2;
        }
        // ...or tab
        if (checkRange(range,i,0x09)) range[i++] = 0x09;
        if (i == length) {
          range = (char*) realloc(range,length*2);
          length *= 2;
        }
      }
      // keyword SPACE
      else if (!strcmp(buffer,"SPACE"))
        func = isspace;
      // keyword CNTRL
      else if (!strcmp(buffer,"CNTRL"))
        func = iscntrl;
      // keyword PRINT
      else if (!strcmp(buffer,"PRINT"))
        func = isprint;
      // keyword PUNCT
      else if (!strcmp(buffer,"PUNCT"))
        func = ispunct;
      // keyword GRAPH
      else if (!strcmp(buffer,"GRAPH"))
        func = isgraph;
      // unknown symbolic name?
      else
        throw E_ILLEGAL_SYMBOLIC_NAME;

      if (func) {
        int j;
        // go over all ASCII characters and use the specified
        // function...
        for (j=0;j<256;j++)
          if ( (*func)(j) ) {
            if (checkRange(range,i,j)) range[i++] = j;
            if (i == length) {
              range = (char*) realloc(range,length*2);
              length *= 2;
            }
          }
      }

    }
    // simple character
    else {
      if (checkRange(range,i,*ptr)) range[i++] = *ptr;
      ptr++;
      if (i == length) {
        range = (char*) realloc(range,length*2);
        length *= 2;
      }
    }
  }
  range[i]=0x00;  // null-terminate string

  // empty sets are not allowed
  if (i == 0) throw E_ILLEGAL_SET;
  else currentPos+=(ptr-(regexp+currentPos));

  // set SET transition (bits 16 to 27 contain the set number)
  // that is created by insertSet
  setState(freeState, transition | (insertSet(range)<<16), freeState+1, freeState+1);
  i = freeState;
  freeState++;

  free(range);

  return i;
}

/******************************************************************/
/* automaton::setState                                            */
/*                                                                */
/* insert a state into the automaton.                             */
/* if the given position is too large for the currently allocated */
/* arrays, reallocation will occur.                               */
/******************************************************************/
void automaton::setState(int position, int transition, int state1, int state2)
{
  // insert an element not yet settable?
  // yes, then get enough memory for that
  while (size <= position) {
    size<<=1;

    ch    = (int*) realloc(ch,size*sizeof(int));
    next1 = (int*) realloc(next1,size*sizeof(int));
    next2 = (int*) realloc(next2,size*sizeof(int));
  }
  ch[position]    = transition;
  next1[position] = state1;
  next2[position] = state2;
}

/************************************************************/
/* automaton::checkRange                                    */
/*                                                          */
/* small helper function to check if a character is already */
/* in the range (avoid multiple entries).                   */
/* returns 1 if character not in range, zero otherwise.     */
/************************************************************/
int automaton::checkRange(char *range, int length, char c)
{
  int rc = 1;
  for (int i=0;i<length;i++)
    if (range[i] == c) {
      rc = 0;
      break;
    }
  return rc;
}

/***************************************************************/
/* automaton::insertSet                                        */
/*                                                             */
/* create a set and return the set number.                     */
/* the set is an array of characters, the first element of the */
/* set specifies the number of elements in the array.          */
/* the setArray containing pointers to the sets will be re-    */
/* allocated on each call. it is assumed that performance is   */
/* secondary because parsing takes place only once and usually */
/* there are only a few set definitions at all.                */
/***************************************************************/
int automaton::insertSet(char *range)
{
  unsigned int i;

  setSize++;
  // enlarge setArray
  setArray = (int**) realloc(setArray,setSize*sizeof(int*));
  // get memory for set array
  setArray[setSize-1] = (int*) malloc((strlen(range)+1)*sizeof(int));

  // fill in elements
  for ( i=0; i<strlen(range); i++)
    setArray[setSize-1][i+1] = (int) range[i];

  setArray[setSize-1][0] = i;  // set length
  return setSize-1;
}

/*************************************************/
/* automaton::match                              */
/*                                               */
/* try to match a string with the automaton.     */
/* returns 1 on success and 0 on failure.        */
/* matching is non-recursively done with a queue */
/* that has a push, put and pop method.          */
/*************************************************/
int automaton::match(char *a, int N)  // string length passed in
                                      // instead of strlen
{
  int n1, n2;
  int j = 0;
  int state = next1[0];  // get start state
  doubleQueue dq(64);    // create a double queue
                         // one SCAN symbol will be put into the queue
  int i;
  int set;
  int len;
  bool found;

  // terminates when end state (==0) is reached
  while (state) {
#ifdef MYDEBUG
    printf("double queue dump:\n");
    dq.dump();
#endif
    // go to next character
    if (state == SCAN) {
      if (minimal == true && j == N) break; // for minimal match
#ifdef MYDEBUG
      printf("consume %2d %c\n",j,a[j]);
#endif
      j++;
      dq.put(SCAN);
    }
    else
    switch (ch[state] & SCAN) {
    case EPSILON:        // epsilon transition
      n1 = next1[state];
      n2 = next2[state];
      dq.push(n1);
      if (n1 != n2) dq.push(n2);
      break;
    case SET:            // inclusive set
    case SET|NOT:        // exclusive set
      set = (ch[state] & 0x0fff0000)>>16;   // get set number
      len = setArray[set][0];               // get number of elements in set
      found = (ch[state]&NOT)?true:false;   // set default value

      for (i=1; i<=len; i++) {
        if (setArray[set][i] == (int) a[j]) {
          found = !found;
          break;
        }
      }
      if (found) {
        dq.put(next1[state]);
      }
      break;
    case ANY:            // just match any character
      dq.put(next1[state]);
      break;
    default:
      if (j < N) { // freeState: we can't read past end of string
        // normal character?
        if (ch[state] == (int) a[j]) {
          dq.put(next1[state]);
        }
      } else if (j == N) {  // simulate zero-terminated string in any case
        if (ch[state] == (int) 0) {
          dq.put(next1[state]);
        }
      }
      break;
    }
#ifdef MYDEBUG
    printf("dq is %s\n",dq.isEmpty()?"empty":"NOT empty");
    dq.dump();
#endif
    // break out if we've reached the end of the string
    // or no more states are available
    if (dq.isEmpty() || (j>(N+1)) ) break;
    state = dq.pop();
  }

  currentPos = j;

  if (currentPos > N) currentPos = N;

  // return 1 if end state (EOP) has been reached
  return state==EOP?1:0;
}
