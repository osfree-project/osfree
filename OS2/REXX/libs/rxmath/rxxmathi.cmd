/* REXX RXXMATH v1.3 (19 Nov 1996); Copyright 1992, 1996 by John Brock

RXXMATHI supports the interpretive REXX calculator function of RXXMATH,
and should never be called directly.

Note that no REXX variables (other than "prompt") are ever set here.
This means that when this program is used to create a "REXX shell" the
interactive user cannot disrupt the function of the program by changing
a variable that shouldn't be changed.

*/

signal on halt
signal on syntax

numeric digits arg(3)

if arg(1) \= ""
  then interpret 'exit ('"RXXMATH"(arg(2) arg(3), "SCAN", arg(1))") + 0"

if arg(5, "E") then prompt = arg(5)

retry: if arg(1) \= "" then exit

do forever
  interpret "RXXMATH"(arg(2) 0, "PULLSCAN", prompt, symbol("prompt"))
  end


halt: signal on halt
say arg(4) errortext(4)
signal retry


syntax: signal on syntax
if rc \= 44 then say arg(4) errortext(rc)
signal retry
