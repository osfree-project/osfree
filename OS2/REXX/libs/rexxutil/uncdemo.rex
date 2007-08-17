/* demonstration of rexx unicode functions
 * note that all the comments about functionality being built-in or from the system
 * apply to Unix only. It's always from the system on NT.
 * $Header: /netlabs.cvs/osfree/src/REXX/libs/rexxutil/uncdemo.rex,v 1.2 2004/08/21 14:48:44 prokushev Exp $
 * $Name:  $
 */

rcc =  rxfuncadd( 'sysloadfuncs', 'rexxutil', 'sysloadfuncs')

if rcc then do
  say rxfuncerrmsg()
  exit 1
  end

call sysloadfuncs

/* text to convert */
mytext = 'éclairs are for kids'

/* convert using either builtin or system wchar methods */
call systounicode mytext,,,'wobble'

/* this is the converted text */
itstext = wobble.!text

/* now convert it to utf8 using the built-in method */
call sysfromunicode itstext,'utf8',,,'wobble'
itsutf8 = wobble.!text

/* now convert it to utf7 using the built-in method */
call sysfromunicode itstext,'utf7',,,'wobble'
itsutf7 = wobble.!text

say length(mytext) mytext
say length(itstext) itstext
say length(itsutf8) itsutf8
say length(itsutf7) itsutf7

/* now convert back using built-in method */
call systounicode itsutf8, 'utf8',,'wobble'
say length(wobble.!text) wobble.!text

if itstext \= wobble.!text then
  say 'utf8 conversion failed'

call systounicode itsutf7, 'utf7',,'wobble'
say length(wobble.!text) wobble.!text

if itstext \= wobble.!text then
  say 'utf7 conversion failed'

/* this is either a built-in or wchar */
call sysfromunicode itstext,,,,'wobble'
say length(wobble.!text) wobble.!text

if mytext \= wobble.!text then
  say 'unicode conversion failed'

/* the final conversions are done using iconv, and will fail unless your iconv
 * allows ISO-8859-1 as a name. It will always fail on NT. */
call systounicode mytext, 'ISO-8859-1',,'wobble'
say length(wobble.!text) wobble.!text

if wobble.!text \== itstext then
  say 'ISO-8859-1 to unicode conversion failed'

call sysfromunicode itstext,'ISO-8859-1',,,'wobble'
say length(wobble.!text) wobble.!text

if wobble.!text \== mytext then
  say 'unicode to ISO-8859-1 conversion failed'
