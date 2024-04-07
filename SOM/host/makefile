#
# A Makefile for somFree project
#

!ifeq %OS WIN32
WATCOM_RUNTIME = clbr19
!endif
!ifeq %OS WIN64
WATCOM_RUNTIME = clbr19
!endif

SOM_COMPILER = pdl sc somcpp somipc somenv somxh somstars somcorba somhh somkpub idl somcprep genh genxh somc spp
# SOM Run-time kernel
SOM_KERNEL = #somdtype somkprep som
# Interface Repository Framework
SOM_IR = #somiprep somref

DIRS = $(WATCOM_RUNTIME) $(SOM_COMPILER) $(SOM_KERNEL) $(SOM_IR) somtk

# Below not correct yet
# somcorba somir somd somnmf somcdr somany somu somestrm somirfmt Frameworks

PLATFORM = host$(SEP)$(%HOST)$(SEP)

!include $(%ROOT)tools/mk/all.mk

