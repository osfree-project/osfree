##!dmake -f

# main goal of this Makefile:
# ASSIGN

# source in the Free-Dos initialization 
.INCLUDE .FIRST .IGNORE : "fd_init.mk" "../fd_init.mk" "../../fd_init.mk"

#
#Project related settings
#
.IF $(_COMPTYPE) == MC
PRG = assign.com
.ELSE
PRG = assign.exe
.ENDIF
SRC = assign.c
OBJ = assign.obj
HDR = asgn_asc.c yerror.h
MYCFLAGS = # TEST=YES
MSGLIB !:= msg.lib
MSGDCL !:= yerror.h
LDLIBS = $(MSGLIB) $(FDLIB)\$(_MODEL)_$(LNG).lib $(FDLIB)\Suppl_$(_MODEL).lib

#
#First target
#
all : $(CFG) $(PRG)	# copy library into Free-Dos library dir

.INIT : $(CFG) tags refs errlist # Will make the utilizing files

#
#C initialization file
#
.IF $(CFG)
# Compiler configuration file, for Borland C only
# options: no Windows, no RTTI, use pre-compiled headers, no floating point

CONFIGURATION = -W-	\
-X-	\
-H	\
-I.;$(INCDIR)	\
-L.;$(LIBDIR)	\
-H=assign.csm	\
-f-	\
-ff-	\
-m$(_MODEL)

.IF $(_COMPILER) != BC31
CONFIGURATION += -RT-
.ENDIF

$(CFG) : $(MAKEFILE:s/-f//)
	Cat $(mktmp $(CONFIGURATION:t"\n")\n) >$@

.ENDIF


$(PRG) .SWAP : $(MSGLIB) $(OBJ) asgn_asc.c
.IF $(_COMPTYPE) == MC
	$(LD) $(LDCOMFLAG) $(LDFLAGS) $(MCDIR)\Pc86rl_t @$(mktmp $(OBJ:t" ") \n), $@, NUL,@$(mktmp $(LDLIBS:t" ") $(MCDIR)\Mclib\n)
.ELSE
	$(CC) -e$@ @$(mktmp $(OBJ:t"\n") $(LDLIBS:t"\n")\n)
.ENDIF

asgn_tsr.com : asgn_tsr.obj
	$(LD) $(LDCOMFLAG) $< , $@;

asgn_asc.c : asgn_tsr.com
	$(FDBIN)\Bin2c module $< >$@

#MAKEDEP START
assign.obj : assign.c \
	asgn_asc.c \
	yerror.h
#MAKEDEP STOP

ci :: $(shell dir /b *.1 *.1g)

clean :: 
	$(RM) /s asgn_tsr.obj asgn_tsr.com asgn_tsr.map assign.com assign.exe

clobber :: 
	$(RM) /s asgn_asc.c

# source in the Free-Dos standard targets 
.INCLUDE .FIRST .IGNORE : fd_exit.mk ../fd_exit.mk ../../fd_exit.mk
