PROJ = spdx-sbom
TRGT = $(PROJ).exe
DESC = SDPX SBOM generator
srcfiles = $(p)spdx-sbom$(e) $(p)spdx_sbom_doc$(e) $(p)spdx_sbom_extracted$(e) $(p)spdx_sbom_opts$(e) $(p)spdx_sbom_out$(e) $(p)spdx_sbom_out_json$(e) $(p)spdx_sbom_out_tag$(e) $(p)spdx_sbom_scan$(e) $(p)spdx_sbom_utils$(e)

ADD_COPT = -i=..$(SEP)include
ADD_LINKOPT = lib reuse_parser.lib, spdx.lib, sha1.lib, omf.lib, res.lib, json.lib

!include $(%ROOT)tools/mk/tools.mk
