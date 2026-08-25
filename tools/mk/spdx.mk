# tools/mk/spdx.mk - правила интеграции SPDX/REUSE в систему сборки osFree

SPDX_LINT   = spdx-lint.exe
SPDX_ANNOTATE = spdx-annotate.exe
SPDX_SBOM   = spdx-sbom.exe

!ifdef LICENSE_CHECK

# Сборка аргументов для утилит на основе переменных компонента
SPDX_LINT_ARGS =
!ifdef LICENSE
SPDX_LINT_ARGS = $(SPDX_LINT_ARGS) --default-license=$(LICENSE)
!endif
!ifdef COPYRIGHT
SPDX_LINT_ARGS = $(SPDX_LINT_ARGS) --default-copyright=$(COPYRIGHT)
!endif
!ifdef EXCLUDE_LICENSE
SPDX_LINT_ARGS = $(SPDX_LINT_ARGS) --exclude=$(EXCLUDE_LICENSE)
!endif

SPDX_ANNOTATE_ARGS =
!ifdef LICENSE
SPDX_ANNOTATE_ARGS = $(SPDX_ANNOTATE_ARGS) --default-license=$(LICENSE)
!endif
!ifdef COPYRIGHT
SPDX_ANNOTATE_ARGS = $(SPDX_ANNOTATE_ARGS) --default-copyright=$(COPYRIGHT)
!endif
!ifdef EXCLUDE_LICENSE
SPDX_ANNOTATE_ARGS = $(SPDX_ANNOTATE_ARGS) --exclude=$(EXCLUDE_LICENSE)
!endif

SPDX_SBOM_ARGS =
!ifdef LICENSE
SPDX_SBOM_ARGS = $(SPDX_SBOM_ARGS) --default-license=$(LICENSE)
!endif
!ifdef COPYRIGHT
SPDX_SBOM_ARGS = $(SPDX_SBOM_ARGS) --default-copyright=$(COPYRIGHT)
!endif

spdx-lint: .SYMBOLIC
    @$(SPDX_LINT) $(CWD) $(SPDX_LINT_ARGS)

spdx-annotate: .SYMBOLIC
    @$(SPDX_ANNOTATE) --dir=$(CWD) $(SPDX_ANNOTATE_ARGS)

spdx-sbom: .SYMBOLIC
    @$(SPDX_SBOM) --output=$(PROJECT).spdx.json $(CURDIR) $(SPDX_SBOM_ARGS)

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
