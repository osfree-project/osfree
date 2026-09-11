# tools/mk/spdx.mk - правила интеграции SPDX/REUSE в систему сборки osFree

SPDX_LINT   = spdx-lint.exe
SPDX_ANNOTATE = spdx-annotate.exe
SPDX_SBOM   = spdx-sbom.exe


!ifdef LICENSE_CHECK

!ifndef COPYRIGHT
COPYRIGHT = Copyright (C) 2025 osFree Project
!endif


# Common arguments for all tools
SPDX_ARGS =
!ifdef LICENSE
SPDX_ARGS = $(SPDX_ARGS) --default-license="$(LICENSE)"
!endif
!ifdef COPYRIGHT
SPDX_ARGS = $(SPDX_ARGS) --default-copyright="$(COPYRIGHT)"
!endif
!ifdef EXCLUDE_LICENSE
EXCLUDE_LICENSE = $(EXCLUDE_LICENSE) makefile _wcc.cmd _wcc.sh
SPDX_ARGS = $(SPDX_ARGS)  --exclude="$(EXCLUDE_LICENSE)"
!endif

# Сборка аргументов для утилит на основе переменных компонента
SPDX_LINT_ARGS = $(SPDX_ARGS)

SPDX_ANNOTATE_ARGS = $(SPDX_ARGS)

SPDX_SBOM_ARGS = $(SPDX_ARGS)
!ifdef DESC
SPDX_SBOM_ARGS = $(SPDX_SBOM_ARGS) --name="osFree $(DESC)"
!endif

SPDX_SBOM_ARGS = $(SPDX_SBOM_ARGS) --creator="Organization: osFree Project"   --supplier="Organization: osFree Project" --version="0.1" --file="$(PATH)$(TRGT)" --objects="$(OBJS)"

spdx-lint: .SYMBOLIC
    $(verbose)$(SPDX_LINT) $(CWD) $(SPDX_LINT_ARGS) --licenses-json=$(ROOT)licenses\licenses.json --exceptions-json=$(ROOT)licenses\exceptions.json --cache=$(BLD)spdx_db.cache

spdx-annotate: .SYMBOLIC
    @$(SPDX_ANNOTATE) --dir=$(CWD) $(SPDX_ANNOTATE_ARGS)

spdx-sbom: .SYMBOLIC
    $(SPDX_SBOM) --purpose="SOURCE" --objects="$(OBJS)" --output=$(PATH)$(PROJ).spdx.json $(MYDIR) $(SPDX_SBOM_ARGS)
    $(SPDX_SBOM) --purpose="$(TARGET_CLASS)" --output=$(PATH)$(PROJ)-bin.spdx.json --source-sbom=$(PATH)$(PROJ).spdx.json $(MYDIR) $(SPDX_SBOM_ARGS)
    spdx-merge.exe --input=$(PATH)$(PROJ)-bin.spdx.json --output=$(PATH)$(PROJ)-merged.spdx.json

spdx-clean: .SYMBOLIC
    @if exist $(PROJ).spdx.json del $(PROJ).spdx.json

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
