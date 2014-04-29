#
# $Header$
#

.AUTODEPEND

# Define targets to build
all:	ext2flt.flt

debug:	ext2flt.flt

# Compiler and Assembler options
CC_OPT= -3 -O2 -v -d -ms! -wpro -weas -wpre 
AS_OPT= /MX /ZI /O
LN_OPT= /code /line /map

# Include path for files
INC= include

# Names of complier, assembler and linker.
CC = bcc -c $(CC_OPT) -I$(INC)
AS = tasm $(AS_OPT) /i$(INC)
LINK = link

# Implicit rules
.c.obj:
  $(CC) {$< }

.asm.obj:
  $(AS) $&.asm

# Define object files used to build executable from:
OBJ_1= e2header.obj e2wrap.obj e2data.obj
OBJ_2= e2router.obj e2filt.obj e2part.obj
OBJ_3= e2iocmd.obj e2virtio.obj e2init.obj 
OBJ_4= e2inutil.obj

OBJS= $(OBJ_1) $(OBJ_2) $(OBJ_3) $(OBJ_4)

# Make sure that the new debug flag is active...
debug.h: MAKEFILE

# List macros
EXE_depend = $(OBJS)

# Explicit rules
ext2flt.flt: $(EXE_depend) ext2flt.def MAKEFILE
  $(LINK) $(LN_OPT) @&&|
$(OBJ_1) +
$(OBJ_2) +
$(OBJ_3) +
$(OBJ_4)
ext2flt.flt
ext2flt.map

ext2flt.def
| 
  mapsym ext2flt.map 
  tdstrip ext2flt.flt
  copy ext2flt.flt d:\ext2flt.flt 
  copy ext2flt.sym d:\ext2flt.sym 

