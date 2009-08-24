.DELETE_ON_ERROR:

DLLS=mmioFlac.dll mmioVorb.dll mmioMP3.dll

all: install.dll uninstallIOProc.exe Control.SCR $(DLLS) doc

mmioFlac.dll: mmioFlac\mmioFlac.dll
	copy $< .

mmioVorb.dll: mmioVorbis\mmioVorb.dll
	copy $< .

mmioMP3.dll: mmioMP3\mmioMP3.dll
	copy $< .

Control.SCR: Control.cmd VERSION $(DLLS) mmioFLAC/VERSION mmioVorbis/VERSION mmioMP3/VERSION
	control > Control.SCR

CC=gcc
CFLAGS=-O2 -DUSE_OS2_TOOLKIT_HEADERS
INCLUDES= 
COMPILE = $(CC) $(DEFS) $(INCLUDES) $(CFLAGS)

install.dll: install.c 
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -Zdll -Zomf install.def -o install.dll -los2me

uninstallIOProc.exe: uninstallIOProc.c 
	$(CC) $(CFLAGS) $(LDFLAGS) $^ uninstallIOProc.def -o uninstallIOProc.exe -los2me

clean:
	-del *.dll Control.SCR *.exe *.html

CATALOG=H:\sgml\catalog
HTMLDECL=H:\sgml\15445\15445.dcl
%.html: %.preHtml
	sgmlnorm -A html -d -c $(CATALOG) $(HTMLDECL) $< > $@
	nsgmls -s -c $(CATALOG) $(HTMLDECL) $@

mmioFLAC.html: mmioFLAC/VERSION
mmioVorib.html: mmioVorbis/VERSION
mmioMP3.html: mmioMP3/VERSION
index.html:  mmioFlac/VERSION mmioVorbis/VERSION mmioMP3/VERSION VERSION

doc: mmioFLAC.html mmioVorb.html mmioMP3.html index.html stream.html
