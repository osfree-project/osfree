#
# Makefile for the cats library by Jim Hall (jhall@freedos.org).
#

compiler = tcc -c
linker   = tcc
lib      = tlib

options = -w -Z -O -N

objects = get_line.obj db.obj catgets.obj

all: $(objects)
	@..\delete catsdb.lib
	@$(lib) catsdb.lib+ get_line.obj
	@$(lib) catsdb.lib+ db.obj
	@$(lib) catsdb.lib+ catgets.obj

get_line.obj: get_line.c
	@$(compiler) $(options) get_line.c

db.obj: db.c
	@$(compiler) $(options) db.c

catgets.obj: catgets.c
	@$(compiler) $(options) catgets.c

clean:
	@..\delete get_line.obj
	@..\delete db.obj
	@..\delete catgets.obj