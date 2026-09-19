/* res.c - extract source file names from .res files (C89)
 *
 * Resource file format (Microsoft Resource Compiler, also used by
 * OpenWatcom and Borland):
 *
 *   RESOURCEHEADER:
 *     DWORD DataSize
 *     DWORD HeaderSize
 *     ResourceType
 *     ResourceName
 *     [padding to 4 bytes]
 *
 *   ResourceType / ResourceName:
 *     - If the first WORD is 0xFFFF: ordinal (next WORD is the value).
 *     - Otherwise: length (WORD) followed by a string. Usually UTF-16,
 *       but ASCII is also possible.
 *
 *   After the header come DataSize bytes of resource data, followed
 *   by padding to a 4-byte boundary.
 *
 * The dependency resource we look for:
 *   type = 0x79 (DEP_LIST_TYPE)
 *   name = "EBWF_XFMMTUPPE" (DEP_LIST_NAME)
 *
 * The payload of that resource is a sequence of DepInfo records:
 *   DWORD time (Unix timestamp)
 *   WORD  len  (name length including the terminating '\0')
 *   char  name[len]
 *
 * Terminated by a record with time = 0 and len = 0.
 *
 * References:
 *   - Microsoft Resource File Formats.
 *     https://learn.microsoft.com/en-us/windows/win32/menurc/resource-file-formats
 *   - RESOURCEHEADER structure.
 *     https://learn.microsoft.com/en-us/windows/win32/menurc/resourceheader
 *
 * The dependency resource itself is Open Watcom specific and is not
 * documented by Microsoft.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "res.h"

/**
 * @file res.c
 * @brief Implementation of the .res source extractor.
 *
 * See the file header for the resource format description and
 * references.
 *
 */

/** @brief Resource type carrying the dependency list. */
#define DEP_LIST_TYPE 0x0079

/** @brief Resource name carrying the dependency list. */
static const char DEP_LIST_NAME[] = "EBWF_XFMMTUPPE";

/** @brief Maximum length of a single extracted name. */
#define RES_NAME_MAX 512

/* ------------------------------------------------------------------ */
/* Little-endian readers                                               */
/* ------------------------------------------------------------------ */

/**
 * @brief Read a 2-byte little-endian unsigned integer.
 *
 * @param[in] p  Pointer to two bytes. Not NULL.
 *
 * @return Value in [0, 65535].
 */
static unsigned int read_u16le(const unsigned char *p) {
    return (unsigned int)p[0] | ((unsigned int)p[1] << 8);
}

/**
 * @brief Read a 4-byte little-endian unsigned integer.
 *
 * @param[in] p  Pointer to four bytes. Not NULL.
 *
 * @return Value in [0, 2^32 - 1].
 */
static unsigned long read_u32le(const unsigned char *p) {
    return (unsigned long)p[0] |
           ((unsigned long)p[1] << 8) |
           ((unsigned long)p[2] << 16) |
           ((unsigned long)p[3] << 24);
}

/* ------------------------------------------------------------------ */
/* Resource header parsing                                             */
/* ------------------------------------------------------------------ */

/**
 * @brief Read a resource header field (type or name).
 *
 * A field is either an ordinal (first WORD == 0xFFFF) or a
 * length-prefixed string. Strings are usually UTF-16 but may be
 * ASCII; the function detects this heuristically.
 *
 * @param[in]  buf           Whole file buffer. Not NULL.
 * @param[in]  buf_size      Size of @p buf.
 * @param[in]  pos           Start offset of the field.
 * @param[out] is_ordinal    Receiver: 1 if the field is an ordinal.
 *                           May be NULL.
 * @param[out] ordinal_value Receiver: ordinal value. May be NULL.
 * @param[in]  expect_name   If not NULL, the string is compared
 *                           case-sensitively to this value.
 * @param[out] matched       Receiver: 1 if the string equals
 *                           @p expect_name. May be NULL.
 *
 * @return Offset just past the field, or 0 on parse error.
 */
static size_t read_res_header_field(const unsigned char *buf, size_t buf_size,
                                    size_t pos,
                                    int *is_ordinal,
                                    unsigned int *ordinal_value,
                                    const char *expect_name,
                                    int *matched) {
    unsigned int first;
    unsigned int wlen;
    size_t i;
    char name_buf[256];
    int is_utf16;

    if (matched) *matched = 0;
    if (is_ordinal) *is_ordinal = 0;
    if (ordinal_value) *ordinal_value = 0;

    if (pos + 2 > buf_size) return 0;
    first = read_u16le(buf + pos);

    if (first == 0xFFFF) {
        if (pos + 4 > buf_size) return 0;
        if (is_ordinal) *is_ordinal = 1;
        if (ordinal_value) *ordinal_value = read_u16le(buf + pos + 2);
        return pos + 4;
    }

    wlen = first;
    pos += 2;
    if (wlen == 0) return pos;

    if (pos + (size_t)wlen * 2 > buf_size) return 0;

    is_utf16 = 1;
    for (i = 0; i < wlen; i++) {
        if (buf[pos + i * 2 + 1] != 0) { is_utf16 = 0; break; }
    }

    if (wlen < sizeof(name_buf)) {
        if (is_utf16) {
            for (i = 0; i < wlen; i++)
                name_buf[i] = (char)buf[pos + i * 2];
            name_buf[wlen] = '\0';
            if (expect_name && matched) {
                if (strcmp(name_buf, expect_name) == 0) *matched = 1;
            }
        } else {
            if (pos + wlen > buf_size) return 0;
            memcpy(name_buf, buf + pos, wlen);
            name_buf[wlen] = '\0';
            if (expect_name && matched) {
                if (strcmp(name_buf, expect_name) == 0) *matched = 1;
            }
        }
    }

    return pos + (size_t)wlen * 2;
}

/* ------------------------------------------------------------------ */
/* DepInfo parsing                                                     */
/* ------------------------------------------------------------------ */

/**
 * @brief Parse the DepInfo records of the dependency resource.
 *
 * @param[in] data       Payload bytes. Not NULL.
 * @param[in] data_size  Payload length.
 * @param[in] hOut       Destination set. Not NULLHANDLE.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET parse_depinfo(const unsigned char *data, size_t data_size,
                            HSTRSET hOut) {
    size_t pos = 0;
    char buf[RES_NAME_MAX];

    while (pos + 6 <= data_size) {
        unsigned long time = read_u32le(data + pos);
        unsigned int  len  = read_u16le(data + pos + 4);
        pos += 6;

        if (time == 0 && len == 0) {
            return NO_ERROR;
        }
        if (len == 0) break;
        if (pos + len > data_size) break;

        if (len > 1) {
            size_t name_len = len - 1;
            APIRET rc;
            if (name_len >= sizeof(buf)) name_len = sizeof(buf) - 1;
            memcpy(buf, data + pos, name_len);
            buf[name_len] = '\0';
            rc = StrSetAdd(hOut, buf);
            if (rc != NO_ERROR) return rc;
        }
        pos += len;
    }
    return NO_ERROR;
}

/* ------------------------------------------------------------------ */
/* Public API                                                          */
/* ------------------------------------------------------------------ */

APIRET APIENTRY ResExtractSources(PCSZ pszPath, HSTRSET hOut) {
    FILE *fp;
    unsigned char *buf;
    long file_size;
    size_t read_size;
    size_t pos = 0;
    int found = 0;
    APIRET rc = NO_ERROR;

    if (!pszPath || hOut == NULLHANDLE) return ERROR_INVALID_PARAMETER;

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

    while (pos + 8 <= (size_t)file_size) {
        unsigned long data_size;
        unsigned long header_size;
        size_t header_start;
        size_t header_end;
        int is_ordinal;
        unsigned int ordinal_value;
        int matched;
        size_t type_end;
        size_t name_end;
        size_t data_start;
        size_t next_pos;

        data_size   = read_u32le(buf + pos);
        header_size = read_u32le(buf + pos + 4);
        header_start = pos + 8;
        header_end = header_start + (size_t)header_size;

        if (header_end > (size_t)file_size) break;

        /* Type field. */
        type_end = read_res_header_field(buf, (size_t)file_size, header_start,
                                         &is_ordinal, &ordinal_value,
                                         NULL, NULL);
        if (type_end == 0) break;
        while ((type_end % 4) != 0) type_end++;

        /* Name field. */
        name_end = read_res_header_field(buf, (size_t)file_size, type_end,
                                         &is_ordinal, NULL,
                                         DEP_LIST_NAME, &matched);
        if (name_end == 0) break;
        while ((name_end % 4) != 0) name_end++;

        data_start = header_end;
        while ((data_start % 4) != 0) data_start++;

        if (data_start + data_size > (size_t)file_size) break;

        if (ordinal_value == DEP_LIST_TYPE && matched) {
            rc = parse_depinfo(buf + data_start, (size_t)data_size, hOut);
            if (rc != NO_ERROR) { free(buf); return rc; }
            found = 1;
        }

        next_pos = data_start + (size_t)data_size;
        while ((next_pos % 4) != 0) next_pos++;
        if (next_pos <= pos) break;
        pos = next_pos;
    }

    free(buf);

    if (!found) return ERROR_FILE_NOT_FOUND;
    return NO_ERROR;
}
