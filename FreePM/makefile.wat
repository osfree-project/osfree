

#  Handle makefiles in subdirs?
#
!include watcom.mif

SUBDIRS=lib$(sep)Zlib lib$(sep)JpegLib lib$(sep)lpng lib$(sep)GDLib src_c src_s 
FRPM=freepm
#  $[@  $@ $<
#  $<  Lists all subdirs
#  $[@ Lists first subdir
#  $@  Lists target


# @for %a in ($(CLEANEXTS)) do -rm -f $(ODIR)\*.%a

all: .SYMBOLIC $(SUBDIRS) 
	#@for %a in ($(SUBDIRS)) do @echo wmake: Entering '%a' 
	#@for %a in ($(SUBDIRS)) do cd %a 
	#& wmake -f makefile.wat all
	#@echo wmake: Entering '$<' 
	#-cd $< 
	#-wmake -f makefile.wat $@ 
	#@echo wmake: Leaving '$<'
	
	@echo wmake: Entering 'lib$(sep)Zlib' 
	-cd lib$(sep)Zlib 
	-wmake -e -f makefile.wat $@  
	-cd ..$(sep)..
	@echo wmake: Leaving 'lib$(sep)Zlib'
	
	@echo wmake: Entering 'lib$(sep)JpegLib' 
	-cd lib$(sep)JpegLib 
	-wmake -e -f makefile.wat $@  
	-cd ..$(sep)..
	@echo wmake: Leaving 'lib$(sep)JpegLib'
	
	@echo wmake: Entering 'lib$(sep)lpng' 
	-cd lib$(sep)lpng 
	-wmake -e -f makefile.wat $@  
	-cd ..$(sep)..
	@echo wmake: Leaving 'lib$(sep)lpng'
	
	@echo wmake: Entering 'lib$(sep)GDLib' 
	-cd lib$(sep)GDLib 
	-wmake -e -f makefile.wat $@  
	-cd ..$(sep)..
	@echo wmake: Leaving 'lib$(sep)GDLib'
	
	@echo wmake: Entering 'src_c' 
	-cd  src_c
	-wmake -e -f makefile.wat $@  
	-cd ..
	@echo wmake: Leaving 'src_c'
	
	@echo wmake: Entering 'src_s' 
	-cd  src_s
	-wmake -e -f makefile.wat $@ 
	-cd ..
	@echo wmake: Leaving 'src_s'
	
clean: .SYMBOLIC $(SUBDIRS) 
	#@for %a in ($(SUBDIRS)) do @echo wmake: Entering '%a' 
	#@for %a in ($(SUBDIRS)) do cd %a 
	#& wmake -f makefile.wat all
	#@echo wmake: Entering '$<' 
	#-cd $< 
	#-wmake -f makefile.wat $@ 
	#@echo wmake: Leaving '$<'
	
	@echo wmake: Entering 'lib$(sep)Zlib' 
	-cd lib$(sep)Zlib 
	-wmake -f makefile.wat $@  
	-cd ..$(sep)..
	@echo wmake: Leaving 'lib$(sep)Zlib'
	
	@echo wmake: Entering 'lib$(sep)JpegLib' 
	-cd lib$(sep)JpegLib 
	-wmake -f makefile.wat $@  
	-cd ..$(sep)..
	@echo wmake: Leaving 'lib$(sep)JpegLib'
	
	@echo wmake: Entering 'lib$(sep)lpng' 
	-cd lib$(sep)lpng 
	-wmake -f makefile.wat $@  
	-cd ..$(sep)..
	@echo wmake: Leaving 'lib$(sep)lpng'
	
	@echo wmake: Entering 'lib$(sep)GDLib' 
	-cd lib$(sep)GDLib 
	-wmake -f makefile.wat $@  
	-cd ..$(sep)..
	@echo wmake: Leaving 'lib$(sep)GDLib'
	
	@echo wmake: Entering 'src_c' 
	-cd  src_c
	-wmake -f makefile.wat $@  
	-cd ..
	@echo wmake: Leaving 'src_c'
	
	@echo wmake: Entering 'src_s' 
	-cd  src_s
	-wmake -f makefile.wat $@ 
	-cd ..
	@echo wmake: Leaving 'src_s'	

dist: .SYMBOLIC
	 @echo Creating zip file...
	 cd ..
	 zip -ru9 FreePM_3july2007.zip FreePM

# zip's switches: -ru0
# -0   store only
# -1   compress faster              
# -9   compress better
# -r   recurse into directories
# -u   update: only changed or new files

#  A part from a makefile of  FLTK (a FAST GUI toolkit)
# Internal target to build the individual modules. The following macros must
# have been set: D (value D or R), O (D,O,W,L)

#BuildOne : .SYMBOLIC
#
#    @echo "=== making 95 jpeg library ==="
#    cd $(ROOT)/jpeg
#    wmake -h -f makefile.wat $(%OPTIONS) $(%CLEAN)
#    
#    @echo "=== making zlib library ==="
#    @cd $(ROOT)/zlib
#    @wmake -h -f makefile.wat $(%OPTIONS) $(%CLEAN)
#    @echo "=== making png library ==="
#    @cd $(ROOT)/png
#    @wmake -h -f makefile.wat $(%OPTIONS) $(%CLEAN)
#    @echo "=== making fltk libraries ==="
#    @cd $(ROOT)/src
#    @wmake -h -f makefile.wat $(%OPTIONS) $(%CLEAN)
#    @echo "=== making fluid ==="
#    @cd $(ROOT)/fluid
#    @wmake -h -f makefile.wat $(%OPTIONS) $(%CLEAN)
#    @echo "=== making test ==="
#    @cd $(ROOT)/test
#    @wmake -h -f makefile.wat $(%OPTIONS) $(%CLEAN)
#    
#    