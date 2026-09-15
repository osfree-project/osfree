# tools/mk/spdx.mk - правила интеграции SPDX/REUSE в систему сборки osFree

SPDX_LINT     = spdx-lint.exe
SPDX_ANNOTATE = spdx-annotate.exe
SPDX_SBOM     = spdx-sbom.exe
SPDX_MERGE    = spdx-merge.exe

SPDX_DB       = $(ROOT)licenses

!ifndef COPYRIGHT
COPYRIGHT = Copyright (C) 2025 osFree Project
!endif

# ------------------------------------------------------------
# ќбщие аргументы Ч определены всегда, независимо от LICENSE_CHECK.
# »х используют и ручные инструменты (annotate, spdx-clean),
# и автоматические (spdx-lint, spdx-sbom).
# ------------------------------------------------------------

SPDX_ARGS = --spdx-db="$(SPDX_DB)"

!ifdef BLD
SPDX_ARGS = $(SPDX_ARGS) --cache="$(BLD)spdx_db.cache"
!endif

!ifdef LICENSE
SPDX_ARGS = $(SPDX_ARGS) --default-license="$(LICENSE)"
!endif

SPDX_ARGS = $(SPDX_ARGS) --default-copyright="$(COPYRIGHT)"

# ------------------------------------------------------------
# јргументы дл€ отдельных утилит
# ------------------------------------------------------------

SPDX_LINT_ARGS = $(SPDX_ARGS)

# annotate: license/copyright задаютс€ как override, не как default
SPDX_ANNOTATE_ARGS = --spdx-db="$(SPDX_DB)"
!ifdef BLD
SPDX_ANNOTATE_ARGS = $(SPDX_ANNOTATE_ARGS) --cache="$(BLD)spdx_db.cache"
!endif
!ifdef LICENSE
SPDX_ANNOTATE_ARGS = $(SPDX_ANNOTATE_ARGS) --license="$(LICENSE)"
!endif
SPDX_ANNOTATE_ARGS = $(SPDX_ANNOTATE_ARGS) --copyright="$(COPYRIGHT)"

SPDX_SBOM_ARGS = $(SPDX_ARGS)
!ifdef DESC
SPDX_SBOM_ARGS = $(SPDX_SBOM_ARGS) --name="osFree $(DESC)"
!endif
SPDX_SBOM_ARGS = $(SPDX_SBOM_ARGS) &
    --creator="Organization: osFree Project" &
    --supplier="Organization: osFree Project" &
    --version="0.1" &
    --file="$(PATH)$(TRGT)" &
    --objects="$(OBJS)"

SPDX_MERGE_ARGS = --spdx-db="$(SPDX_DB)"
!ifdef BLD
SPDX_MERGE_ARGS = $(SPDX_MERGE_ARGS) --cache="$(BLD)spdx_db.cache"
!endif

# ------------------------------------------------------------
# –учные инструменты Ч доступны всегда
# ------------------------------------------------------------

annotate: .SYMBOLIC
    @$(SPDX_ANNOTATE) $(CWD) $(SPDX_ANNOTATE_ARGS)

annotate-write: .SYMBOLIC
    @$(SPDX_ANNOTATE) $(CWD) $(SPDX_ANNOTATE_ARGS) --write

spdx-clean: .SYMBOLIC
    @if exist $(PATH)$(PROJ).spdx.json        del $(PATH)$(PROJ).spdx.json
    @if exist $(PATH)$(PROJ)-bin.spdx.json    del $(PATH)$(PROJ)-bin.spdx.json
    @if exist $(PATH)$(PROJ)-merged.spdx.json del $(PATH)$(PROJ)-merged.spdx.json

# ------------------------------------------------------------
# јвтоматическа€ проверка Ч только при LICENSE_CHECK
# ------------------------------------------------------------

!ifdef LICENSE_CHECK

spdx-lint: .SYMBOLIC
    $(verbose)$(SPDX_LINT) $(CWD) $(SPDX_LINT_ARGS)

spdx-sbom: .SYMBOLIC
    $(verbose)$(SPDX_SBOM) --purpose="SOURCE" --output=$(PATH)$(PROJ).spdx.json $(MYDIR) $(SPDX_SBOM_ARGS)
    $(verbose)$(SPDX_SBOM) --purpose="$(TARGET_CLASS)" --output=$(PATH)$(PROJ)-bin.spdx.json --source-sbom=$(PATH)$(PROJ).spdx.json $(MYDIR) $(SPDX_SBOM_ARGS)
    $(verbose)$(SPDX_MERGE) --input=$(PATH)$(PROJ)-bin.spdx.json --output=$(PATH)$(PROJ)-merged.spdx.json $(SPDX_MERGE_ARGS)

!else

spdx-lint: .SYMBOLIC
    @%null

spdx-sbom: .SYMBOLIC
    @%null

!endif
