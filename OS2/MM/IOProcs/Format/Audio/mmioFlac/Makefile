.DELETE_ON_ERROR:
CC=gcc
CFLAGS=-O2 -Zcrtdll -Zomf -DUSE_OS2_TOOLKIT_HEADERS
#CFLAGS=-Wall -g -DDEBUG -Zcrtdll -Zomf -DUSE_OS2_TOOLKIT_HEADERS
LIBS= -lm -los2me
INCLUDES= -Iinclude
COMPILE = $(CC) $(DEFS) $(INCLUDES) $(CFLAGS)

all: mmioflac.dll
                                                                 
%.obj: %.c
	$(COMPILE) -c $<

*.obj: mmioflac.h

mmioFLAC.dll: mmioflac.obj libFLAC/libFLAC.lib
	$(CC) $(CFLAGS) -Zdll $(LDFLAGS) $(INCLUDES) $^ libFLAC/libFLAC.lib mmioflac.def -o $@ $(LIBS)

mmioflac.exe: main.obj mmioflac.obj libFLAC/libFLAC.lib
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@ -Zstack 65536 $(LIBS)
	
clean:
	-rm core *.obj mmioflac.exe mmioflac.dll

