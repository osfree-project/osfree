# tools/mk/spdx.mk - правила интеграции SPDX/REUSE в систему сборки osFree

SPDX_LINT   = spdx-lint.exe
SPDX_ANNOTATE = spdx-annotate.exe
SPDX_SBOM   = spdx-sbom.exe

!ifdef LICENSE_CHECK

spdx-lint: .SYMBOLIC
    @$(SPDX_LINT) $(CWD)

spdx-annotate: .SYMBOLIC
    @$(SPDX_ANNOTATE) --dir=$(CWD)

spdx-sbom: .SYMBOLIC
    @$(SPDX_SBOM) --output=$(PROJECT).spdx.json $(CURDIR)

spdx-clean: .SYMBOLIC
    @if exist $(PROJECT).spdx.json del $(PROJECT).spdx.json

!else

spdx-lint: .SYMBOLIC
    @%null

spdx-annotate: .SYMBOLIC
    @%null

spdx-sbom: .SYMBOLIC
    @%null

spdx-clean: .SYMBOLIC
    @%null

!endif
