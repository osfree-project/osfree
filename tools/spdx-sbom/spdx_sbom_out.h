/* spdx_sbom_out.h - диспетчер вывода SBOM (C89) */
#ifndef SPDX_SBOM_OUT_H
#define SPDX_SBOM_OUT_H

#include "spdx_sbom_types.h"

/* Выводит документ в формате format ("json", "tagvalue"/"tag").
 * Возвращает 0 при успехе, -1 на неизвестном формате. */
int sbom_output(const SpdxDocument *doc, const char *format);

int sbom_output_json(const SpdxDocument *doc);
int sbom_output_tagvalue(const SpdxDocument *doc);

#endif /* SPDX_SBOM_OUT_H */
