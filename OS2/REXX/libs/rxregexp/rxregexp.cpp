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
/* Regular Expression Utility functions                                       */
/*                                                                            */
/******************************************************************************/
#include "dblqueue.hpp"
#include "automaton.hpp"
#include "regexp.hpp"

#include "RexxCore.h"

#define INCL_REXXSAA
#include "rexx.h"
// #include "ObjectClass.hpp"
// #include "StringClass.hpp"
#include "RexxNativeAPI.h"                      // REXX native interface

RexxMethod3(REXXOBJECT,                // Return type
            RegExp_Init,               // Object_method name
            OSELF, self,               // Pointer to self
            REXXOBJECT, expression,    // optional regular expression
            REXXOBJECT, matchtype)     // optional match type (MAXIMAL (def.) or MINIMAL)
{
  char        szBuffer[16];
  int         iResult = 0;
  automaton  *pAutomaton;
  RexxString *pArgString = NULL;

  pAutomaton = new automaton();

  // optional matchtype given?
  if (matchtype) {
    pArgString = (RexxString *) RexxSend0(matchtype, "STRING");
    // it must be a string
    if ( !_isstring(pArgString) )
      send_exception1(Error_Incorrect_method_string, RexxArray1(RexxString("2")));
    if ( strcmp(string_data(pArgString), "MINIMAL") == 0) {
      pAutomaton->setMinimal(true);
    }
  }

  // optional expression given?
  if (expression) {
//  pArgString = (RexxString *) RexxSend1(expression, "REQUEST", RexxString("STRING"));
    pArgString = (RexxString *) RexxSend0(expression, "STRING");
    // it must be a string
    if ( !_isstring(pArgString) )
      send_exception1(Error_Incorrect_method_string, RexxArray1(RexxString("1")));
    iResult = pAutomaton->parse( string_data(pArgString) );
  }

  sprintf(szBuffer, "%p", pAutomaton);
  REXX_SETVAR("!AUTOMATON", RexxString(szBuffer));

  if (iResult)
    send_exception(Error_Invalid_template);

  return RexxNil;
}

RexxMethod1(REXXOBJECT,                // Return type
            RegExp_Uninit,             // Object_method name
            OSELF, self)               // Pointer to self
{
  automaton  *pAutomaton = NULL;
  char       *pszString;

  pszString = string_data( (RexxString*) REXX_GETVAR("!AUTOMATON") );
  if (*pszString != '!')
    sscanf(pszString, "%p", &pAutomaton);
  if (pAutomaton) delete pAutomaton;

  return RexxNil;
}

RexxMethod3(REXXOBJECT,                // Return type
            RegExp_Parse,              // Object_method name
            OSELF, self,               // Pointer to self
            REXXOBJECT, expression,    // regular expression to parse
            REXXOBJECT, matchtype)     // optional match type (MAXIMAL (def.) or MINIMAL)
{
  automaton  *pAutomaton = NULL;
  char       *pszString;
  char        szBuffer[32];
  RexxString *result;

  RexxString *pArgString = NULL;

  if (!expression)
    send_exception1(Error_Incorrect_method_noarg, RexxArray1(RexxString("1")));

//  pArgString = (RexxString *) RexxSend1(expression, "REQUEST", RexxString("STRING"));
  pArgString = (RexxString *) RexxSend0(expression, "STRING");
  // it must be a string
  if ( !_isstring(pArgString) )
    send_exception1(Error_Incorrect_method_string, RexxArray1(RexxString("1")));

  pszString = string_data( (RexxString*) REXX_GETVAR("!AUTOMATON") );
  if (*pszString != '!')
    sscanf(pszString, "%p", &pAutomaton);

  if (pAutomaton) {
    pszString = string_data(pArgString);
    // moved some ptrs to re-use variables
    // optional matchtype given?
    if (matchtype) {
      pArgString = (RexxString *) RexxSend0(matchtype, "STRING");
      // it must be a string
      if ( !_isstring(pArgString) )
        send_exception1(Error_Incorrect_method_string, RexxArray1(RexxString("2")));
      if ( strcmp(string_data(pArgString), "MINIMAL") == 0) {
        pAutomaton->setMinimal(true); // set minimal matching
      } else if (strcmp(string_data(pArgString), "CURRENT") != 0) {
        pAutomaton->setMinimal(false); // set maximal matching
      }
    }
    int i = pAutomaton->parse( pszString );
    sprintf(szBuffer, "%d", pAutomaton->getCurrentPos());
    REXX_SETVAR("!POS", RexxString(szBuffer));
    sprintf(szBuffer,"%d",i);
    result = (RexxString*) RexxString(szBuffer);
  } else {
    result = (RexxString*) RexxString("-1");
  }

  return result;
}

RexxMethod2(REXXOBJECT,                // Return type
            RegExp_Match,              // Object_method name
            OSELF, self,               // Pointer to self
            REXXOBJECT, string)        // string to match
{
  automaton  *pAutomaton = NULL;
  char       *pszString;
  char        szBuffer[32];
  RexxString *result;

  RexxString *pArgString = NULL;

  if (!string)
    send_exception1(Error_Incorrect_method_noarg, RexxArray1(RexxString("1")));

//  pArgString = (RexxString *) RexxSend1(string, "REQUEST", RexxString("STRING"));
  pArgString = (RexxString *) RexxSend0(string, "STRING");
  // it must be a string
  if ( !_isstring(pArgString) )
    send_exception1(Error_Incorrect_method_string, RexxArray1(RexxString("1")));

  pszString = string_data( (RexxString*) REXX_GETVAR("!AUTOMATON") );
  if (*pszString != '!')
    sscanf(pszString, "%p", &pAutomaton);
  if (pAutomaton) {
    int i = pAutomaton->match( string_data(pArgString), string_length(pArgString) );
    sprintf(szBuffer, "%d", pAutomaton->getCurrentPos());
    REXX_SETVAR("!POS", RexxString(szBuffer));
    sprintf(szBuffer,"%d",i);
    result = (RexxString*) RexxString(szBuffer);
  } else {
    result = (RexxString*) RexxString("0");
  }

  return result;
}

RexxMethod2(REXXOBJECT,                // Return type
            RegExp_Pos,                // Object_method name
            OSELF, self,               // Pointer to self
            REXXOBJECT, string)        // string to match
{
  automaton  *pAutomaton = NULL;
  bool        fOldState;
  char       *pszString;
  size_t      strlength;
  char        szBuffer[32];
  RexxString *result;
  RexxString *pArgString = NULL;
  int         i;

  if (!string)
    send_exception1(Error_Incorrect_method_noarg, RexxArray1(RexxString("1")));

  pArgString = (RexxString *) RexxSend0(string, "STRING");
  // it must be a string
  if ( !_isstring(pArgString) )
    send_exception1(Error_Incorrect_method_string, RexxArray1(RexxString("1")));

  pszString = string_data( (RexxString*) REXX_GETVAR("!AUTOMATON") );
  if (*pszString != '!')
    sscanf(pszString, "%p", &pAutomaton);

  pszString = string_data(pArgString);
  strlength = string_length(pArgString);

  if (pAutomaton && strlength > 0) {  /* only check when input > 0 */
    fOldState = pAutomaton->getMinimal();

    // we start out matching minimal
    pAutomaton->setMinimal(true);
    do {
      i = pAutomaton->match(pszString, strlength);
      strlength--;
      pszString++;
    } while (i == 0 && strlength != 0);
    // can we match at all?
    if (i != 0) {
      i = (int) (pszString - string_data(pArgString));
      // want a maximal match within string?
      if (fOldState == false) {
        pAutomaton->setMinimal(false);
        pszString--; // correct starting pos
        strlength++; // correct starting len
        while (strlength != 0) {
          if (pAutomaton->match(pszString, strlength) != 0) {
            break;
          }
          strlength--;
        }
      }
      sprintf(szBuffer, "%d", i + pAutomaton->getCurrentPos() - 1); // store end-of-match position
    } else {
      sprintf(szBuffer, "0");
    }

    REXX_SETVAR("!POS", RexxString(szBuffer));
    sprintf(szBuffer,"%d",i);
    result = (RexxString*) RexxString(szBuffer);
    pAutomaton->setMinimal(fOldState);  // restore to state at POS invocation time
  } else {
    result = (RexxString*) RexxString("0");
  }

  return result;
}
