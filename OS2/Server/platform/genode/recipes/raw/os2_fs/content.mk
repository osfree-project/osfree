REPOSITORIES = $(wildcard $(GENODE_DIR)/repos/*)

select_from_repositories = $(firstword $(foreach REP,$(REPOSITORIES),$(wildcard $(REP)/$(1))))

FILES_DIR := $(call select_from_repositories,shared/filesys/os2)

FILE_LIST = doscalls.dll sub32.dll kbdcalls.dll viocalls.dll moucalls.dll moncalls.dll \
 msg.dll nls.dll quecalls.dll sesmgr.dll utlapi.dll pmmerge.dll pmwin.dll oso001.msg \
 mini33.exe minicmd.exe makeini.exe argenv.exe scanenv.exe sed.exe awk.exe zip.exe \
 sort.exe ver.exe wasm.exe wmake.exe mem.exe

content: config.sys

config.sys:
	cp $(FILES_DIR)/$@ $@

content: os2.tar

os2.tar:
	mkdir -p file/system
	$(foreach f,$(FILE_LIST),ln -sf $(FILES_DIR)/$(f) file/system/$(f);)
#	ln -sf ../../kal.map file/system/kal.map
	tar cfh os2.tar file/system
	rm -rf file
