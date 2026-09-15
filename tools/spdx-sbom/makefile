DESCRIPTION = SDPX SBOM generator
SOURCES = spdx-sbom spdx_sbom_doc spdx_sbom_extracted spdx_sbom_opts spdx_sbom_out spdx_sbom_out_json spdx_sbom_out_tag spdx_sbom_scan spdx_sbom_utils
TARGET_API = HOST
TARGET_BITS = 32

ADD_COPT = -i=..$(SEP)include
ADD_LINKOPT = lib reuse_parser.lib, spdx.lib, sha1.lib, omf.lib, res.lib, json.lib, git.lib

!include $(%ROOT)tools/mk/build.mk
