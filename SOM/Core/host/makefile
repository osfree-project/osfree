#
# A Makefile for somFree project
#

SOM_COMPILER = pdl somcpp somipc sc
# SOM Run-time kernel
SOM_KERNEL = somkpub somdtype somkprep som
# Interface Repository Framework
SOM_IR = #somiprep somref

DIRS = $(SOM_COMPILER) $(SOM_KERNEL) $(SOM_IR)

# Below not correct yet
# somref somcorba somir somd somnmf somcdr somany somu somestrm somirfmt Frameworks

PLATFORM = host$(SEP)$(%HOST)$(SEP)

!include $(%ROOT)/mk/all.mk

#prepr: .symbolic
#  $(verbose)$(MDHIER) $(ROOT)build$(SEP)include$(SEP)somidl
#  $(verbose)$(MDHIER) $(ROOT)build$(SEP)include$(SEP)somidl$(SEP)$(%OS)
