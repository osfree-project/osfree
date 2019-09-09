# Makefile for building win16 tests
# --- BUILD ---
# To buile a single test, do "make test_name.exe"
# To build all tests, do "make all"
# To build tests with large memory module, add "MM=ml" to the argument, e.g.
# 	"make all MM=ml"
# --- TESTS ---
# To run a single test, do "make test_name.ok"
# To run all tests, do "make tests"
# --- CLEANUP ---
# To clean up, do "make clean"
# --- OUTPUT OPTION ---
# Default is to output tests result to "log_test_nameOfTest.txt"
# Add "LOG=disable" to output result on console window without logging to file. 


# Macros Definitions

LOGFLAGS = 
MM = 
CFLAGS =-D_X86_ -DSTANDALONE '-i=C:\WATCOM\h\win;C:\WATCOM\h'-zW -e25 -zq -od -d2 -w4 -wcd=126 -wcd=102 -wcd=112 -bw-bt=windows 
ifeq ($(MM), ml) 
CFLAGS += -ml
endif

ifneq ($(LOG), disable)
LOGFLAGS = --log
endif

WINE = wine
CC = $(WINE) wcc
CL = $(WINE) wlink
RUNTEST = WINETEST_PLATFORM=wine $(WINE)

CTESTS = alloc.c \
atom.c \
bitmap.c \
class.c \
clipboard.c \
combo.c \
comm16.c \
dce.c \
dialog.c \
dosmem.c \
file.c \
filedlg.c \
infosize.c \
listbox.c \
mapping.c \
msg.c \
printdlg.c \
profile.c \
socket16.c \
str.c \
sys.c \
sysdir.c \
text.c \
timer.c \
window.c 

TESTRESULTS = $(CTESTS:.c=.ok)
OBJS = $(CTESTS:.c=.obj)
BINARIES = $(CTESTS:.c=.exe)
BINARIES += dos_file.exe

# Explicit Rules

all: $(BINARIES)
clean: 
	rm -f $(BINARIES) $(TESTRESULTS) $(OBJS) *.log
tests: $(TESTRESULTS) 

.PHONY: all clean tests

.SUFFIXES: .obj .ok .exe

# special rule for dos_file.exe
dos_file.exe : dos_func.obj dos_file.obj
	$(CL) @dos_file.lnk

# special rule for socket16.exe
socket16.exe : socket16.obj
	$(CL) lib winsock.lib file $<

# special rule for infosize.exe
infosize.exe : infosize.obj
	$(CL) lib ver.lib file $<

# MM=ml for combo.obj, listbox.obj, and msg.obj
combo.obj : combo.c
	$(CC) $(CFLAGS) -ml combo.c
listbox.obj : listbox.c
	$(CC) $(CFLAGS) -ml listbox.c
msg.obj : msg.c
	$(CC) $(CFLAGS) -ml msg.c

# General Suffix Rules
.obj.exe :
	$(CL) file $<

.c.obj :
	$(CC) $(CFLAGS) $<

.c.ok :
	$(RUNTEST) $*.exe $(LOGFLAGS) && touch $@

# dependencies
alloc.ok : alloc.exe
atom.ok : atom.exe
bitmap.ok : bitmap.exe
class.ok : class.exe
clipboard.ok : clipboard.exe
combo.ok : combo.exe
comm16.ok : comm16.exe
dce.ok : dce.exe
dialog.ok : dialog.exe
dosmem.ok : dosmem.exe
file.ok : file.exe
filedlg.ok : filedlg.exe
infosize.ok : infosize.exe
listbox.ok : listbox.exe
mapping.ok : mapping.exe
msg.ok : msg.exe
profile.ok : profile.exe
printdlg.ok : printdlg.exe
socket16.ok : socket16.exe
str.ok : str.exe
sys.ok : sys.exe
sysdir.ok : sysdir.exe
text.ok : text.exe
timer.ok : timer.exe
window.ok : window.exe


