# tools/mk/spdx.mk - правила интеграции SPDX/REUSE в систему сборки osFree

SPDX_LINT   = spdx-lint.exe
SPDX_ANNOTATE = spdx-annotate.exe
SPDX_SBOM   = spdx-sbom.exe


!ifdef LICENSE_CHECK

!ifndef COPYRIGHT
COPYRIGHT = Copyright (C) 2025 osFree Project
!endif

EXCLUDE_LICENSE += makefile _wcc.cmd _wcc.sh

# Сборка аргументов для утилит на основе переменных компонента
SPDX_LINT_ARGS =
!ifdef LICENSE
SPDX_LINT_ARGS = $(SPDX_LINT_ARGS) --default-license="$(LICENSE)"
!endif
!ifdef COPYRIGHT
SPDX_LINT_ARGS = $(SPDX_LINT_ARGS) --default-copyright="$(COPYRIGHT)"
!endif
!ifdef EXCLUDE_LICENSE
SPDX_LINT_ARGS = $(SPDX_LINT_ARGS) --exclude="$(EXCLUDE_LICENSE)"
!endif

SPDX_ANNOTATE_ARGS =
!ifdef LICENSE
SPDX_ANNOTATE_ARGS = $(SPDX_ANNOTATE_ARGS) --default-license="$(LICENSE)"
!endif
!ifdef COPYRIGHT
SPDX_ANNOTATE_ARGS = $(SPDX_ANNOTATE_ARGS) --default-copyright="$(COPYRIGHT)"
!endif
!ifdef EXCLUDE_LICENSE
SPDX_ANNOTATE_ARGS = $(SPDX_ANNOTATE_ARGS) --exclude="$(EXCLUDE_LICENSE)"
!endif

SPDX_SBOM_ARGS =
!ifdef DESC
SPDX_SBOM_ARGS = $(SPDX_SBOM_ARGS) --name="osFree $(DESC)"
!endif
!ifdef LICENSE
SPDX_SBOM_ARGS = $(SPDX_SBOM_ARGS) --default-license="$(LICENSE)"
!endif
!ifdef COPYRIGHT
SPDX_SBOM_ARGS = $(SPDX_SBOM_ARGS) --default-copyright="$(COPYRIGHT)"
!endif
!ifdef EXCLUDE_LICENSE
SPDX_SBOM_ARGS = $(SPDX_SBOM_ARGS) --exclude="$(EXCLUDE_LICENSE)"
!endif

spdx-lint: .SYMBOLIC
    $(verbose)$(SPDX_LINT) $(CWD) $(SPDX_LINT_ARGS)

spdx-annotate: .SYMBOLIC
    @$(SPDX_ANNOTATE) --dir=$(CWD) $(SPDX_ANNOTATE_ARGS)

spdx-sbom: .SYMBOLIC
    @$(SPDX_SBOM) --output=$(PATH)$(PROJ).spdx.json $(MYDIR) $(SPDX_SBOM_ARGS) --creator="Organization: osFree Project"   --supplier="Organization: osFree Project" --version="0.1"

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
