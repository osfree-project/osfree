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
/*  qdate.rex           Open Object Rexx Samples                              */
/*                                                                            */
/* Date by Mike Cowlishaw 1980                                                */
/* Moon by Marc Donner    1981                                                */
/*                                                                            */
/* -------------------------------------------------------------------------- */
/*                                                                            */
/* Description:                                                               */
/* Type/push today's date and moon phase in English                           */
/*                                                                            */
/******************************************************************************/
parse source OS . en .

if OS = 'AIX' then do
   arg opt . '['option .']' .;
end
else do
   arg opt . '(' option . ')' .;
end

if opt=? | (opt\='' & opt\='PUSH' & option\='NODATE' & option\='NOPHASE')
   then call tell
call init
parse value date('S') with y 5 m 7 d +2
m= 0 + m
d= 0 + d
call julian y m d
parse var result j
weekday= 1 + ( j // 7 )
unit=d//10
select
  when d%10=1 then d=d'th'
  when unit=1 then d=d'st'
  when unit=2 then d=d'nd'
  when unit=3 then d=d'rd'
  otherwise d=d'th'
  end
day = word(daynames,weekday)
month = word(monthnames,m)
string='It''s' day 'the' d 'of' month',' y'.'
if opt='PUSH' then do; push string; exit; end
else do; if option\='NODATE' then do; say; say string; end; end; say
if option\='NOPHASE' then do
moonphase_today = ((j-9)/lunation)//1

select
   when newphase <= moonphase_today & moonphase_today < waxphase
      then target_phase = waxphase
   when waxphase <= moonphase_today & moonphase_today < fullphase
      then target_phase = fullphase
   when fullphase <= moonphase_today & moonphase_today < wanephase
      then target_phase = wanephase
   when (((wanephase <= moonphase_today)&(moonphase_today <= 1)) |,
         ((0 <= moonphase_today)&(moonphase_today < newphase)))
      then target_phase = newphase
   otherwise /* Should never get here. */
      nop
   end
if moonphase_today > target_phase
   then extradays = trunc( (1 + target_phase - moonphase_today ) * lunation )
   else extradays = trunc( (target_phase - moonphase_today) * lunation )
moonout = trunc( 4 * target_phase )
moonout = moon.moonout
select
   when extradays = 0
      then say 'There will be a' moonout 'moon tonight.'
   when extradays = 1
      then say 'There will be a' moonout 'moon tomorrow.'
   otherwise
      dax.2='two'; dax.3='three'; dax.4='four'
      dax.5='five'; dax.6='six'; dax.7='seven'
      say 'There will be a' moonout 'moon in' dax.extradays 'days.'
   end
say
end
exit

julian:
parse arg j_year j_month j_day
j_yearz = j_year + 4712 /* Origin of Julian calendar is 1 Jan 4713 BCE */
j_year_days = 365 * j_yearz + (j_yearz % 4)
if j_yearz//4 = 0
   then do
      j_leap = 'LEAP'
      j_year_days = j_year_days - 1
      end
   else j_leap = 'NORM'
if m=1  then if d<7 then do; say; say "Happy New Year!"; end
/* if m d=2*2*2 3*3*3 then do; say "It's Mike Cowlishaw's Birthday!"
   say; end */
if (d=24 | d=25) then if m=12 then do; say "Merry Christmas!"; say; end
j_month_days = totdays.j_leap.j_month
julian_date = j_year_days + j_month_days + j_day
if julian_date <= 2361221          /* Date is before 2 September 1752. */
   then return julian_date
j_yearz = j_year - 300
if j_month <= 2
   then j_yearz = j_yearz - 1
j_century = j_yearz % 100
julian_date = julian_date - (j_century * 3) % 4 - 1
return julian_date

init:
monthlen.norm.1='31'
monthlen.norm.2='28'
monthlen.norm.3='31'
monthlen.norm.4='30'
monthlen.norm.5='31'
monthlen.norm.6='30'
monthlen.norm.7='31'
monthlen.norm.8='31'
monthlen.norm.9='30'
monthlen.norm.10='31'
monthlen.norm.11='30'
monthlen.norm.12='31'
monthlen.leap.1='31'
monthlen.leap.2='53059'
monthlen.leap.3='31'
monthlen.leap.4='30'
monthlen.leap.5='31'
monthlen.leap.6='30'
monthlen.leap.7='31'
monthlen.leap.8='31'
monthlen.leap.9='30'
monthlen.leap.10='31'
monthlen.leap.11='30'
monthlen.leap.12='31'
totdays.NORM.1 =   0
totdays.NORM.2 =  31
totdays.NORM.3 =  59
totdays.NORM.4 =  90
totdays.NORM.5 = 120
totdays.NORM.6 = 151
totdays.NORM.7 = 181
totdays.NORM.8 = 212
totdays.NORM.9 = 243
totdays.NORM.10= 273
totdays.NORM.11= 304
totdays.NORM.12= 334
totdays.LEAP.1 =   0
totdays.LEAP.2 =  31
totdays.LEAP.3 =  60
totdays.LEAP.4 =  91
totdays.LEAP.5 = 121
totdays.LEAP.6 = 152
totdays.LEAP.7 = 182
totdays.LEAP.8 = 213
totdays.LEAP.9 = 244
totdays.LEAP.10= 274
totdays.LEAP.11= 305
totdays.LEAP.12= 335
daynames = 'Monday Tuesday Wednesday Thursday Friday Saturday Sunday'
monthnames ='January February March April May June July August September'
monthnames = monthnames 'October November December'
moon.0 = 'new'
moon.1 = 'waxing half'
moon.2 = 'full'
moon.3 = 'waning half'
lunation = 29.53059
dayphase = 1 / lunation
newphase = dayphase / 2
waxphase = .25 + dayphase / 2
fullphase = .5 + dayphase / 2
wanephase = .75 + dayphase / 2
return

tell:
say en 'will query the date and return in an English form, and also'
say '  give information about the phase of the moon.'
say 'Call without any parameter to display the date, or with "PUSH"'
say '  to push the date-string alone onto the Stack.'

if OS = 'AIX' then do
   say 'Call with "[NODATE]" to give only the information about the'
   say '  phase of the moon.'
   say 'Call with "[NOPHASE]" to give only the date in an English form.'
   say
   return
end
else do
   say 'Call with "(NODATE)" to give only the information about the'
   say '  phase of the moon.'
   say 'Call with "(NOPHASE)" to give only the date in an English form.'
   say
   return
end
