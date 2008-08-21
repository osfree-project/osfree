#
# A Makefile for lzexpand.dll
# (c) osFree project,
# author, date
#

PROJ     = keyboard
DESC     = Windows keyboard library
ADD_COPT = -i=$(MYDIR) -i=$(MYDIR)..$(SEP)include -i=$(MYDIR)..$(SEP)win
srcfiles = $(p)Keyboard$(e) $(p)SysKeyboard$(e) $(p)KeyboardIF$(e) &
           $(p)KeyboardBin$(e) #$(p)Keyboard.res$(e)
# linking dll file
DLL      = 1
EXPORTS  = ToAscii         .4,   &
           AnsiToOem       .5,   &
           OemToAnsi       .6,   &
           OemKeyScan      .128, &
           VkKeyScan       .129, &
           GetKeyboardType .130, &
           MapVirtualKey   .131, &
           GetKBCodePage   .132, &
           GetKeyNameText  .133, &
           AnsiToOemBuff   .134, &
           OemToAnsiBuff   .135
OPTIONS  = heapsize=1024K libfile libentry
RESOURCE = $(PATH)Keyboard.res
DEST     = os2$(SEP)mdos$(SEP)winos2$(SEP)system

!include $(%ROOT)/mk/appsw16.mk
