#
# A Makefile for bvh32.dll
# (c) osFree project,
# authors:
# Yuri Prokushev,   2018,
#

PROJ     = bvh32
TRGT     = $(PROJ).dll
DESC     = Base Video Handler 32-bit wrapper around 16-bit BVHSVGA
srcfiles = $(p)msg$(e) &
           $(p)dbcsdisplayinfo$(e) &
           $(p)querymode$(e) &
           $(p)setmode$(e) &
           $(p)devenable$(e) &
           $(p)queryvariableinfo$(e) &
           $(p)setvariableinfo$(e) &
           $(p)initializeenvironment$(e) &
           $(p)queryvideostate$(e) &
           $(p)setvideostate$(e) &
           $(p)queryconfiginfo$(e) &
           $(p)restoreenvironment$(e) &
           $(p)textbufferupdate$(e) &
           $(p)querycursorinfo$(e) &
           $(p)saveenvironment$(e) &
           $(p)querylvbinfo$(e) &
           $(p)setcursorinfo$(e)

ADD_COPT = -s -od
ADD_LINKOPT  = lib clib3r op nod segment   type DATA shared
DLL     = 1
DLLOPT  = initinstance terminstance
OPTIONS = manyautodata
UNI2H = 1

#IMPORTS = 

EXPORTS = Dev32Enable           .1
