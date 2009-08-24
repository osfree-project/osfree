#
# 	Sample sources demonstrating functionality of Willows TWIN
#	
#

#
# 	Common to win16 and win32
#

include $(TWINDIR)/Makefile.config

MAKEVARS =  TWINDIR=$(TWINDIR) TARGET=$(TARGET) 

DIRS = 	windemo cdsdemo dlgdemo mdidemo msgdemo cliptext dlldemo wmfdemo \
	hlpdemo prtdemo apidemo

#
#	win32 only
#
DIRS32 = ctrldemo listview progbar treeview

#
#	win16 only
#
BIN  = 	bindemo bindemo2 bn2demo

INSTALLDIR=$(TWINDIR)/$(TARGET)/bin

all:	demo$(TWIN_APISIZE)

demo:
	@for i in $(DIRS) $(BIN); do $(MAKE) -C $(TWINDIR)/samples/$$i $(MAKEVARS); done

demo32:
	@for i in $(DIRS) $(DIRS32); do $(MAKE) -C $(TWINDIR)/samples/$$i $(MAKEVARS); done

clean:
	@for i in $(DIRS) $(DIRS32) $(BIN); do $(MAKE) -C $(TWINDIR)/samples/$$i clean $(MAKEVARS); done

clobber: clean

install:
	@for i in $(DIRS); do cp $(TWINDIR)/samples/$$i/$$i $(INSTALLDIR) $(MAKEVARS); done

depend:
