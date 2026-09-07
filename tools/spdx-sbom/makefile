PROJ = spdx-sbom
TRGT = $(PROJ).exe
DESC = SDPX SBOM generator
srcfiles = $(p)spdx-sbom$(e)

ADD_COPT = -i=..$(SEP)include
ADD_LINKOPT = lib reuse_parser.lib, spdx.lib, sha1.lib

!include $(%ROOT)tools/mk/tools.mk
