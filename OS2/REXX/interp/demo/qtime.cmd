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
/*  qtime.rex           Open Object Rexx Samples                              */
/*                                                                            */
/* Mike Cowlishaw 1979                                                        */
/*                                                                            */
/* -------------------------------------------------------------------------- */
/*                                                                            */
/* Description:                                                               */
/* Displays or stacks time in real English, also chimes.                      */
/*                                                                            */
/* Arguments:  PUSH           pushes single result line onto stack.           */
/*             TEST hh:mm:ss  runs with specified test time.                  */
/******************************************************************************/


   Parse source  . . program .       /* get my real name */
   Parse upper arg  arg rest         /* get arguments    */
   if arg='?' then do
     call tell                       /* say what we do */
     parse var rest arg rest         /* remove "?" */
     end

   stack=0    /* flag to say whether pushing or saying */
   c8=time()  /* Normally use the time Now */

   /* Detect and handle PUSH option */
   if arg='PUSH' then do
     stack=1                         /* set flag to indicate request */
     parse var rest arg rest         /*  .. and remove the word */
     end

   /* Detect and handle TEST option (must follow PUSH) */
   if arg='TEST' then parse var rest c8 arg rest  /* set c8 and remove */

   /* Now check for extraneous (or unrecognised) args */
   if arg\='' then do
     if rest='' then do; plural='';  own='has';  end  /* grammar.. */
                else do; plural='s'; own='have'; end
     say 'The extra argument'plural '"'space(arg rest)'"',
         'that you gave to' program own 'been ignored.'
     end

   /************** OK, off we go... ***************/
   ot="It's"

   hr=substr(c8,1,2)+0        /* extract hours, mins, secs.. */
   mn=substr(c8,4,2)
   sc=substr(c8,7,2)

   h.1 = 'one' ;  h.2 = 'two';  h.3 = 'three';   h.4 = 'four'
   h.5 = 'five';  h.6 = 'six';  h.7 = 'seven';   h.8 = 'eight'
   h.9 = 'nine';  h.10= 'ten';  h.11= 'eleven';  h.12= 'twelve'

   if sc>29 then mn=mn+1      /* round up mins */
   if mn>32 then hr=hr+1      /* something to.. */

   mod=mn//5                  /* find 5-minute bracket */
   select
     when mod=0 then nop      /* exact */
     when mod=1 then ot=ot 'just gone'
     when mod=2 then ot=ot 'just after'
     when mod=3 then ot=ot 'nearly'
     when mod=4 then ot=ot 'almost'
     end                      /* select */

   mn=mn+2                    /* round up */
   if hr//12=0 & mn//60<=4
    then signal midnoon       /* special case noon and midnight */
   mn=mn-(mn//5)              /* to nearest 5 mins */
   if hr>12
    then hr=hr-12             /* get rid of 24-hour clock */
    else
     if hr=0 then hr=12       /* cater for midnight */

   select
     when mn=0  then nop      /* add O'clock later */
     when mn=60 then mn=0
     when mn= 5 then ot=ot 'five past'
     when mn=10 then ot=ot 'ten past'
     when mn=15 then ot=ot 'a quarter past'
     when mn=20 then ot=ot 'twenty past'
     when mn=25 then ot=ot 'twenty-five past'
     when mn=30 then ot=ot 'half past'
     when mn=35 then ot=ot 'twenty-five to'
     when mn=40 then ot=ot 'twenty to'
     when mn=45 then ot=ot 'a quarter to'
     when mn=50 then ot=ot 'ten to'
     when mn=55 then ot=ot 'five to'
     end
   ot=ot h.hr                    /* add the hour number */
   if mn=0 then ot=ot "o'clock"  /* and O'clock if exact */
   ot=ot'.'                      /* and the correct punctuation */

   /* Now stack or display the result */
   if \stack then do
     if mod=0 & mn//15=0 then call chime /* only if displaying */
     say; say ot; say
     end
    else push ot
   exit

/* Special-case Midnight and Noon */
MIDNOON:
   if hr=12 then ot=ot 'Noon.'
            else ot=ot 'Midnight.'
   if \stack then do
     hr=12
     if mn//60=2 then do
       mn=0
       call chime
       end
     say; say ot; say
     end
    else push ot
   exit

/* "Chime" the hours or quarters */
CHIME:
   /* Give chimes */
   if mn//60=0 then /* hourly chime */ do
     chime='Bong'
     num=hr
     end
    else do         /* quarterly tinkles */
     chime='Ding-Dong'
     num=mn%15
     end
   say; say ot
   ot='('chime      /* Parenthesis and first chime */
   do num-1         /* Add the remainder of chiming sounds ... */
     ot=ot||',' chime
     end
   ot=ot||'!)'      /* ... and final punctuation and parenthesis */
   return /* from chime */

TELL:
 say
 say program 'will query the time and display or return it in English.'
 say 'Call without any arguments to display the time, or with "PUSH"'
 say '  to push the time-string onto the Stack (without blank lines).'
 say program 'will "chime" at the quarter-hours and on the hours, but'
 say '  the chimes are also not placed on the stack.'
 say
 say 'English (British) idioms are used in this program.'
 return

/* Mike Cowlishaw,  December 1979 - July 1983                        */
