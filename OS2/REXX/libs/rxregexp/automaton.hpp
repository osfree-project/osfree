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
/* Object REXX Support                                          rxregexp.cpp  */
/*                                                                            */
/* Regular Expression Utility functions - automaton header file               */
/*                                                                            */
/******************************************************************************/

#ifndef AUTOMATON
#define AUTOMATON

#include "dblqueue.hpp"

class automaton {
  public:
    automaton();              // CTOR
    ~automaton();             // DTOR
    int parse(char*);         // parse regular expression
    int match(char*, int);    // match a string

    // in case of a parsing error, this can be used
    // to detect the position at which the error
    // occured. on successful parsing it tells the
    // length of the regular expression.
    int getCurrentPos() { return currentPos; }

    void setMinimal(bool);
    bool getMinimal() { return minimal; }

  private:
    // methods to parse a regular expression
    int expression();
    int term();
    int factor();
    int set();
    int letter(int);

    // insert a state into the automaton
    void setState(int, int, int, int);
    // insert a set into the list of sets
    int insertSet(char*);
    // helper function for set building
    int checkRange(char*, int, char);

    int *ch;        // characters to match
    int *next1;     // first transition possibility
    int *next2;     // second transition possibility

    int  final;     // the final state position

    char *regexp;   // pointer to regular expression

    int **setArray; // pointer to array of sets
    int setSize;    // number of sets

    int  size;      // number of states
    int  freeState; // number of next free state
    int  currentPos;// current position in parsing
    bool minimal;   // minimal matching?
};

#endif
