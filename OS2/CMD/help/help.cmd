@echo off
if -%1- == -- goto msg
if -%1- == -on- goto on
if -%1- == -ON- goto on
if -%1- == -On- goto on
if -%1- == -oN- goto on
if -%1- == -OFF- goto off
if -%1- == -OFf- goto off
if -%1- == -OfF- goto off
if -%1- == -Off- goto off
if -%1- == -off- goto off
if -%1- == -oFF- goto off
if -%1- == -ofF- goto off
if -%1- == -oFf- goto off
helpmsg %1 %2
goto exit

:msg
helpmsg
goto exit

:on
prompt $i[$p]
goto exit

:off
cls
prompt

:exit
