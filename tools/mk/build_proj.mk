!include $(%ROOT)tools/mk/dirs.mk

all install: .SYMBOLIC gen_proj_name
prepall:     .SYMBOLIC prep
depsall:     .SYMBOLIC deps

MAKEOPT = -h

dir2=$+ $(CWD) $-
dir3=$(dir2:$(CWD)=)
dir4=$(dir3:$(SEP)=)

deps prep clean: .SYMBOLIC
 @cd ..
 @cd $(dir4) && $(MAKE) $(MAKEOPT) $^@ PROJ=$(dir4)

gen_proj_name: .SYMBOLIC
 #generate project name
 @$(REXX) mdhier.cmd $(PATH)
 #change dir to set correct value of dir4. Magic!
 @cd ..
 #Save project name for future usage (will not start project detection from build dir)
 @if not exist $(PATH)$(dir4)$(SEP)_proj.mk @%append $(PATH)$(dir4)$(SEP)_proj.mk PROJ=$(dir4)
 #Don't split line to be correct dir4. Magic!
 @cd $(dir4) && $(MAKE) $(MAKEOPT) PROJ=$(dir4)
