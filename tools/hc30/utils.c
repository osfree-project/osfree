/*!
 * @file utils.c
 * @brief Implementation of the shared utilities.
 */
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "os2types.h"

/*!
 * @brief Translation table used by the name hashing function.
 *
 * Maps each byte value 0x00..0xFF to its hashing weight. Values that
 * appear in WinHelp topic names are mapped through this table so the
 * resulting hash matches the one computed by the HC30 compiler.
 */
static const unsigned char table[256] = {
    '\x00','\xD1','\xD2','\xD3','\xD4','\xD5','\xD6','\xD7','\xD8','\xD9','\xDA','\xDB','\xDC','\xDD','\xDE','\xDF',
    '\xE0','\xE1','\xE2','\xE3','\xE4','\xE5','\xE6','\xE7','\xE8','\xE9','\xEA','\xEB','\xEC','\xED','\xEE','\xEF',
    '\xF0','\x0B','\xF2','\xF3','\xF4','\xF5','\xF6','\xF7','\xF8','\xF9','\xFA','\xFB','\xFC','\xFD','\x0C','\xFF',
    '\x0A','\x01','\x02','\x03','\x04','\x05','\x06','\x07','\x08','\x09','\x0A','\x0B','\x0C','\x0D','\x0E','\x0F',
    '\x10','\x11','\x12','\x13','\x14','\x15','\x16','\x17','\x18','\x19','\x1A','\x1B','\x1C','\x1D','\x1E','\x1F',
    '\x20','\x21','\x22','\x23','\x24','\x25','\x26','\x27','\x28','\x29','\x2A','\x0B','\x0C','\x0D','\x0E','\x0D',
    '\x10','\x11','\x12','\x13','\x14','\x15','\x16','\x17','\x18','\x19','\x1A','\x1B','\x1C','\x1D','\x1E','\x1F',
    '\x20','\x21','\x22','\x23','\x24','\x25','\x26','\x27','\x28','\x29','\x2A','\x2B','\x2C','\x2D','\x2E','\x2F',
    '\x50','\x51','\x52','\x53','\x54','\x55','\x56','\x57','\x58','\x59','\x5A','\x5B','\x5C','\x5D','\x5E','\x5F',
    '\x60','\x61','\x62','\x63','\x64','\x65','\x66','\x67','\x68','\x69','\x6A','\x6B','\x6C','\x6D','\x6E','\x6F',
    '\x70','\x71','\x72','\x73','\x74','\x75','\x76','\x77','\x78','\x79','\x7A','\x7B','\x7C','\x7D','\x7E','\x7F',
    '\x80','\x81','\x82','\x83','\x0B','\x85','\x86','\x87','\x88','\x89','\x8A','\x8B','\x8C','\x8D','\x8E','\x8F',
    '\x90','\x91','\x92','\x93','\x94','\x95','\x96','\x97','\x98','\x99','\x9A','\x9B','\x9C','\x9D','\x9E','\x9F',
    '\xA0','\xA1','\xA2','\xA3','\xA4','\xA5','\xA6','\xA7','\xA8','\xA9','\xAA','\xAB','\xAC','\xAD','\xAE','\xAF',
    '\xB0','\xB1','\xB2','\xB3','\xB4','\xB5','\xB6','\xB7','\xB8','\xB9','\xBA','\xBB','\xBC','\xBD','\xBE','\xBF',
    '\xC0','\xC1','\xC2','\xC3','\xC4','\xC5','\xC6','\xC7','\xC8','\xC9','\xCA','\xCB','\xCC','\xCD','\xCE','\xCF'
};

/*!
 * @brief Compute the HC30 hash of a name.
 *
 * @param[in] name NUL-terminated name to hash. Not NULL.
 *
 * @return 32-bit hash value.
 */
uint32_t hash_name(const char* name)
{
    uint32_t h = 0;
    const unsigned char* p = (const unsigned char*)name;
    while (*p) h = h * 43L + table[*p++];
    return h;
}

/*!
 * @brief Print an error message and terminate the process.
 *
 * @param[in] fmt printf-style format string. Not NULL.
 * @param[in] ... Format arguments.
 */
void error(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    exit(1);
}

/*!
 * @brief Allocate memory, terminating on failure.
 *
 * @param[in] size Number of bytes to allocate.
 *
 * @return Pointer to the newly allocated block.
 */
void* xmalloc(size_t size)
{
    void* p = malloc(size);
    if (!p) error("Out of memory\n");
    return p;
}

/*!
 * @brief Reallocate memory, terminating on failure.
 *
 * @param[in] ptr  Previously allocated block, or NULL.
 * @param[in] size New size in bytes.
 *
 * @return Pointer to the reallocated block.
 */
void* xrealloc(void* ptr, size_t size)
{
    void* p = realloc(ptr, size);
    if (!p) error("Out of memory\n");
    return p;
}

/*!
 * @brief Duplicate a string, terminating on failure.
 *
 * @param[in] str NUL-terminated source string. Not NULL.
 *
 * @return Pointer to the newly allocated copy.
 */
char* xstrdup(const char* str)
{
    size_t len = strlen(str) + 1;
    char* p = (char*)malloc(len);
    if (!p) error("Out of memory\n");
    memcpy(p, str, len);
    return p;
}
