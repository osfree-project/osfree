# Macros for building, deleting ########################################

CC=bcc -mt -lt -N- -Z

RM=del


# Targets ##############################################################

all: attrib.com

attrib.com: attrib.c types.h *.inc
	$(CC) attrib.c 


# Clean up #############################################################

clean:
	-$(RM) *.dsk
	-$(RM) *.swp
	-$(RM) *.bak
	-$(RM) *.obj
	-$(RM) *.com
	-$(RM) *.exe
