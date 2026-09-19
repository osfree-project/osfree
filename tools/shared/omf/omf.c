/* omf.c - extract source file names from OMF object files (C89)
 *
 * OMF (Relocatable Object Module Format) records scanned for source
 * names:
 *   - THEADR (0x80) — module name, usually the original source file.
 *   - COMENT (0x88) — comment records with vendor-specific classes
 *     carrying dependency file lists.
 *
 * Vendor-specific COMENT classes recognized:
 *   - 0xE9 — Borland auto-dependency: timestamp (4 bytes) followed
 *     by repeated [len][name] entries.
 *   - 0xFB — OpenWatcom auto-dependency: same layout as 0xE9.
 *   - 0x88 — additional dependency class found in OpenWatcom output.
 *   - 0xE8 — Borland source file name: single [len][name] entry.
 *
 * References:
 *   - TIS Portable Formats Specification, Version 1.1 (Relocatable
 *     Object Module Format), Linux Foundation.
 *     https://refspecs.linuxfoundation.org/elf/elfspec.pdf
 *   - JWasm / OpenWatcom WASM sources (COMENT class usage).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "omf.h"

/**
 * @file omf.c
 * @brief Implementation of the OMF source extractor.
 *
 * See the file header for the list of recognized record types and
 * vendor-specific COMENT classes, and for the references.
 *
 */

/** @brief Maximum length of a single extracted name. */
#define OMF_NAME_MAX 512

/**
 * @brief Read a 2-byte little-endian unsigned integer.
 *
 * @param[in] p  Pointer to two bytes. Not NULL.
 *
 * @return Value in [0, 65535].
 */
static unsigned int read_u16(const unsigned char *p) {
    return (unsigned int)p[0] | ((unsigned int)p[1] << 8);
}

/**
 * @brief Append a name of length @p len to the destination set.
 *
 * The name is copied into a temporary NUL-terminated buffer before
 * being handed to StrSetAdd.
 *
 * @param[in] hOut  Destination set. Not NULLHANDLE.
 * @param[in] str   Source bytes. Not NULL.
 * @param[in] len   Number of bytes.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success (including len == 0).
 * @retval ERROR_INVALID_PARAMETER  len exceeds the internal buffer.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET add_name(HSTRSET hOut, const unsigned char *str, size_t len) {
    char buf[OMF_NAME_MAX];
    if (!str || len == 0) return NO_ERROR;
    if (len >= sizeof(buf)) return ERROR_INVALID_PARAMETER;
    memcpy(buf, str, len);
    buf[len] = '\0';
    return StrSetAdd(hOut, buf);
}

/**
 * @brief Process a THEADR record (0x80).
 *
 * Layout: [name_len:1][name:name_len].
 *
 * @param[in] data  Record payload. Not NULL.
 * @param[in] len   Payload length.
 * @param[in] hOut  Destination set. Not NULLHANDLE.
 *
 * @return APIRET
 */
static APIRET process_theadr(const unsigned char *data, size_t len,
                             HSTRSET hOut) {
    size_t pos = 0;
    size_t name_len;

    if (len < 1) return NO_ERROR;
    name_len = data[pos];
    pos++;
    if (pos + name_len > len) return NO_ERROR;
    if (name_len > 0) {
        return add_name(hOut, &data[pos], name_len);
    }
    return NO_ERROR;
}

/**
 * @brief Process a COMENT record (0x88).
 *
 * Layout: [comment_type:1][comment_class:1][payload]. The payload
 * layout depends on the comment class; see the file header for the
 * list of recognized classes.
 *
 * @param[in] data  Record payload. Not NULL.
 * @param[in] len   Payload length.
 * @param[in] hOut  Destination set. Not NULLHANDLE.
 *
 * @return APIRET
 */
static APIRET process_coment(const unsigned char *data, size_t len,
                             HSTRSET hOut) {
    unsigned char comment_type;
    unsigned char comment_class;
    size_t pos;
    size_t payload_len;

    if (len < 2) return NO_ERROR;

    /* First byte is the comment type, second byte is the class. */
    comment_type = data[0];
    comment_class = data[1];
    pos = 2;
    payload_len = len - 2;

    (void)comment_type;

    switch (comment_class) {
        case 0xE9:    /* Borland auto-dependency */
        case 0xFB: {  /* OpenWatcom dependency */
            APIRET rc;
            if (payload_len < 4) return NO_ERROR;
            pos += 4;   /* Skip the 4-byte timestamp. */

            while (pos + 1 <= len) {
                size_t str_len = data[pos];
                pos++;
                if (str_len == 0) continue;
                if (pos + str_len > len) break;
                rc = add_name(hOut, &data[pos], str_len);
                if (rc != NO_ERROR) return rc;
                pos += str_len;
            }
            break;
        }
        case 0x88: {  /* Dependency file: timestamp + [len][name] */
            if (payload_len < 5) return NO_ERROR;
            pos += 4; /* Skip timestamp. */
            if (pos < len) {
                size_t name_len = data[pos];
                pos++;
                if (pos + name_len <= len && name_len > 0) {
                    return add_name(hOut, &data[pos], name_len);
                }
            }
            break;
        }
        case 0xE8: {  /* Borland source file: [len][name] */
            if (payload_len < 1) return NO_ERROR;
            if (pos < len) {
                size_t name_len = data[pos];
                pos++;
                if (pos + name_len <= len && name_len > 0) {
                    return add_name(hOut, &data[pos], name_len);
                }
            }
            break;
        }
        default:
            break;
    }
    return NO_ERROR;
}

/**
 * @brief Extract source file names from an OMF object file.
 *
 * @param[in] pszPath  Path to the OMF object file. Not NULL.
 * @param[in] hOut     Destination string set. Not NULLHANDLE.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszPath or hOut is invalid.
 * @retval ERROR_OPEN_FAILED        File cannot be opened.
 * @retval ERROR_READ_FAULT         Read error.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 * @retval ERROR_FILE_NOT_FOUND     No source names found.
 */
APIRET APIENTRY OmfExtractSources(PCSZ pszPath, HSTRSET hOut) {
    FILE *fp;
    unsigned char *buf;
    long file_size;
    size_t read_size;
    size_t pos = 0;
    ULONG ulBefore = 0;
    ULONG ulAfter = 0;
    APIRET rc;

    if (!pszPath || hOut == NULLHANDLE) return ERROR_INVALID_PARAMETER;

    StrSetGetCount(hOut, &ulBefore);

    fp = fopen(pszPath, "rb");
    if (!fp) return ERROR_OPEN_FAILED;

    if (fseek(fp, 0, SEEK_END) != 0) { fclose(fp); return ERROR_READ_FAULT; }
    file_size = ftell(fp);
    if (file_size <= 0) { fclose(fp); return ERROR_READ_FAULT; }
    if (fseek(fp, 0, SEEK_SET) != 0) { fclose(fp); return ERROR_READ_FAULT; }

    buf = (unsigned char*)malloc((size_t)file_size);
    if (!buf) {
        fclose(fp);
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    read_size = fread(buf, 1, (size_t)file_size, fp);
    fclose(fp);
    if (read_size != (size_t)file_size) {
        free(buf);
        return ERROR_READ_FAULT;
    }

    /* Sequential parse of OMF records without forced alignment. */
    while (pos + 3 <= (size_t)file_size) {
        unsigned char record_type = buf[pos];
        size_t record_len;
        size_t data_start;
        size_t data_len;

        pos++;
        record_len = read_u16(&buf[pos]);
        pos += 2;

        if (pos + record_len > (size_t)file_size) {
            break;
        }
        data_start = pos;
        data_len = record_len;
        pos += record_len;

        switch (record_type) {
            case 0x80: /* THEADR */
                rc = process_theadr(&buf[data_start], data_len, hOut);
                if (rc != NO_ERROR) { free(buf); return rc; }
                break;
            case 0x88: /* COMENT */
                rc = process_coment(&buf[data_start], data_len, hOut);
                if (rc != NO_ERROR) { free(buf); return rc; }
                break;
            default:
                break;
        }
    }

    free(buf);

    StrSetGetCount(hOut, &ulAfter);
    if (ulAfter == ulBefore) return ERROR_FILE_NOT_FOUND;
    return NO_ERROR;
}
