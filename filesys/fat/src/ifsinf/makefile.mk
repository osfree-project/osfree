#
# Macros for IPF/INF
#

IPFC    = wipfc
CLEANUP=*.inf

.SUFFIXES:
.SUFFIXES: .inf .ipf .bmp

all: $(TARGETS) .SYMBOLIC

.ipf.inf: .AUTODEPEND
 $(IPFC) -i $< -o $^@

clean: .symbolic
 -@del $(CLEANUP)  >nul 2>&1
