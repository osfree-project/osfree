!include "..\config.mak"


SOURCE = diskcomp.c misc.c md5.c drive.c

EXE_DEP = diskcomp.obj misc.obj md5.obj drive.obj


clobber:        clean
		del ..\bin\diskcomp.exe 
                del diskcomp.exe 
		del diskcomp.com

clean:
                del *.obj 
		del *.bak 
		del *.crf 
		del *.xrf 
		del *.map *.lst

all:    obj diskcomp.exe

obj: 	diskcomp.obj misc.obj drive.obj md5.obj
		

diskcomp.exe: $(EXE_DEP)
	$(TLINK) diskcomp.obj misc.obj drive.obj md5.obj $(LIBS)c0t.obj,diskcomp.exe,,$(LIBS)cs ;$(LIBS)maths
	copy diskcomp.exe ..\bin

#individual files
diskcomp.obj: $(SOURCE)
	$(CC) $(CFLAGS) -c diskcomp.c

misc.obj: $(SOURCE)
	$(CC) $(CFLAGS) -c misc.c

drive.obj: $(SOURCE)
	$(CC) $(CFLAGS) -c drive.c

md5.obj: $(SOURCE)
	$(CC) $(CFLAGS) -c md5.c


