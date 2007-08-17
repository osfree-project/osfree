#       Dedicated to Jesus Christ, my Lord and savior!
#
#       Copyright (C) 2005 by Sascha Schmidt
#
#       This makefile builds os2ldr, the osFree kernel loader
#


linker = wlink
LFLAGS = name os2ldr system com option nodef option map=os2ldr.map
CC = wcc
CFLAGS = -3 -d0 -s -od -ms -D__OS2__
AS = wasm
AFLAGS = -ms
OE = obj                        # Object Extension differs from Linux to OS/2
DC = del                        # Delete command is rm on linux and del on OS/2
CP = copy                       # Copy command
RN = move                       # Rename command
!ifdef 0
OE = obj                        # Object Extension differs from Linux to OS/2
DC = rm                         # Delete command is rm on linux and del on OS/2
CP = cp                         # Copy command
RN = mv                         # Rename command
!endif

objects = freeldra.$(OE) cmodel.$(OE) segread.$(OE) strcpy.$(OE) &
                        freeldrc.$(OE) periphs.$(OE) builtins.$(OE) cstart_t.$(OE)

os2ldr: os2ldr.com
        $(RN) os2ldr.com os2ldr

#os2ldr.com: $(objects)
#        $(linker) $(LFLAGS) @$^*

os2ldr.com: $(objects)
         @%create $^&.lnk
         @%append $^&.lnk NAME $^&
         @%append $^&.lnk SYSTEM com
         @%append $^&.lnk OPTION NODEFAULTLIBS
         @%append $^&.lnk OPTION MAP=$^&.map
         @%append $^&.lnk PATH $(MYDIR)
         @%append $^&.lnk LIBPATH $(MYDIR)
         #@%append $^&.lnk LIBRARY $(LIBC)
         @for %i in ($(objects)) do @%append $^&.lnk FILE %i
         $(linker) @$^&.lnk

freeldra.$(OE): .AUTODEPEND
        $(AS) -3p $(AFLAGS) freeldra.asm

periphs.$(OE): periphs.asm
   @$(AS) -3p $(AFLAGS) $<

.c.$(OE): .AUTODEPEND
   $(CC) $(CFLAGS) $<

.asm.$(OE): .AUTODEPEND
   $(AS) $(AFLAGS) $<

clean: .SYMBOLIC
        -$(DC) *.o
        -$(DC) *.map
        -$(DC) os2ldr
