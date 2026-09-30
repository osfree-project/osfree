/*!
 * @file hfs.c
 * @brief HLP file system container implementation.
 */
#include "hfs.h"
#include "btr.h"
#include "vector.h"
#include <stdlib.h>
#include <string.h>

#pragma pack(1)

/*!
 * @brief Fixed 16-byte HLP file header.
 */
typedef struct {
    ULONG ulMagic;           /*!< Magic 0x00035F3F. */
    ULONG ulDirectoryStart;  /*!< Offset of the directory FILEHEADER. */
    ULONG ulFreeChainStart;  /*!< Offset of the free-list head, or -1. */
    ULONG ulEntireFileSize;  /*!< Logical size of the whole HLP file. */
} HLPHEADER;

/*!
 * @brief Per-internal-file header.
 *
 * ReservedSpace includes the 9-byte FILEHEADER itself, so a valid
 * file always satisfies ReservedSpace >= UsedSpace + 9.
 */
typedef struct {
    ULONG ulReservedSpace;   /*!< Bytes reserved for this internal file. */
    ULONG ulUsedSpace;       /*!< Bytes actually used by the payload. */
    UCHAR bFileFlags;        /*!< Legacy flags byte, normally 0. */
} FILEHEADER;

#pragma pack()

/*!
 * @brief One internal file held in memory before serialization.
 */
typedef struct {
    PSZ   pszName;           /*!< Internal name such as "|SYSTEM". */
    PBYTE pbData;            /*!< Payload bytes. */
    ULONG ulSize;            /*!< Payload size in bytes. */
} HfsFileRec;

/*!
 * @brief HLP file system state.
 */
typedef struct {
    USHORT  usDirPageSize;   /*!< Page size for the directory B-tree. */
    HVECTOR vFiles;          /*!< Vector of HfsFileRec. */
} HfsRec;

/*! @def HFS_DIR_PAGE_SIZE
 *  @brief Page size used by the directory B-tree. */
#define HFS_DIR_PAGE_SIZE 0x0400

/*! @def HFS_FROM_HANDLE
 *  @brief Convert an HHFS handle to its record pointer.
 *  @param[in] h Handle. */
#define HFS_FROM_HANDLE(h) ((HfsRec*)(h))

/*! @def HFS_HANDLE_FROM
 *  @brief Convert an HfsRec pointer to an HHFS handle.
 *  @param[in] d Record pointer. */
#define HFS_HANDLE_FROM(d) ((HANDLE)(d))

/*!
 * @brief Create an empty HLP file system.
 *
 * @param[out] phHfs Receiver. Not NULL. Set to NULLHANDLE on error.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a phHfs is NULL.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY HfsCreate(PHHFS phHfs)
{
    HfsRec* p; APIRET rc;
    if (!phHfs) return ERROR_INVALID_PARAMETER;
    *phHfs = NULLHANDLE;
    p = (HfsRec*)malloc(sizeof(HfsRec));
    if (!p) return ERROR_NOT_ENOUGH_MEMORY;
    memset(p, 0, sizeof(*p));
    p->usDirPageSize = HFS_DIR_PAGE_SIZE;
    rc = VectorCreate(sizeof(HfsFileRec), &p->vFiles);
    if (rc != NO_ERROR) { free(p); return rc; }
    *phHfs = HFS_HANDLE_FROM(p);
    return NO_ERROR;
}

/*!
 * @brief Destroy an HLP file system and release all its files.
 *
 * @param[in] hHfs Handle. May be NULLHANDLE.
 *
 * @return APIRET
 * @retval NO_ERROR  Success. Also for NULLHANDLE.
 */
APIRET APIENTRY HfsDestroy(HHFS hHfs)
{
    HfsRec* p; ULONG n, i;
    if (hHfs == NULLHANDLE) return NO_ERROR;
    p = HFS_FROM_HANDLE(hHfs);
    if (p->vFiles) {
        VectorGetCount(p->vFiles, &n);
        for (i = 0; i < n; i++) {
            HfsFileRec f;
            VectorGetItem(p->vFiles, i, &f, sizeof(f), NULL);
            if (f.pszName) free(f.pszName);
            if (f.pbData) free(f.pbData);
        }
        VectorDestroy(p->vFiles);
    }
    free(p);
    return NO_ERROR;
}

/*!
 * @brief Add an internal file from an in-memory buffer.
 *
 * @param[in] hHfs    Handle. Not NULLHANDLE.
 * @param[in] pszName Internal file name. Not NULL.
 * @param[in] pvData  File body. Not NULL.
 * @param[in] ulSize  Body size in bytes.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hHfs or @a pszName is NULL, or
 *                                  a file with that name already exists.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY HfsAddFile(HHFS hHfs, PCSZ pszName, PCVOID pvData, ULONG ulSize)
{
    HfsRec* p; HfsFileRec f; ULONG n, i;
    if (hHfs == NULLHANDLE || !pszName) return ERROR_INVALID_PARAMETER;
    p = HFS_FROM_HANDLE(hHfs);
    VectorGetCount(p->vFiles, &n);
    for (i = 0; i < n; i++) {
        HfsFileRec x;
        VectorGetItem(p->vFiles, i, &x, sizeof(x), NULL);
        if (x.pszName && strcmp(x.pszName, pszName) == 0)
            return ERROR_INVALID_PARAMETER;
    }
    f.pszName = strdup(pszName);
    if (!f.pszName) return ERROR_NOT_ENOUGH_MEMORY;
    f.pbData = (PBYTE)malloc(ulSize ? ulSize : 1);
    if (!f.pbData) { free(f.pszName); return ERROR_NOT_ENOUGH_MEMORY; }
    if (pvData && ulSize) memcpy(f.pbData, pvData, ulSize);
    f.ulSize = ulSize;
    return VectorAdd(p->vFiles, &f);
}

/*!
 * @brief Add an internal file by invoking a writer callback.
 *
 * @param[in] hHfs      Handle. Not NULLHANDLE.
 * @param[in] pszName   Internal file name. Not NULL.
 * @param[in] pfnWriter Writer callback. Not NULL.
 * @param[in] pArg      Opaque argument passed to the callback.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 * @retval ERROR_READ_FAULT         Temporary stream is not readable.
 */
APIRET APIENTRY HfsAddFileFromWriter(HHFS hHfs, PCSZ pszName,
                                     PFNHFSWRITER pfnWriter, PVOID pArg)
{
    FILE* tmp; PBYTE pb; LONG sz; APIRET rc;
    if (hHfs == NULLHANDLE || !pszName || !pfnWriter)
        return ERROR_INVALID_PARAMETER;
    tmp = tmpfile();
    if (!tmp) return ERROR_NOT_ENOUGH_MEMORY;
    pfnWriter(pArg, tmp);
    sz = ftell(tmp);
    if (sz < 0) { fclose(tmp); return ERROR_READ_FAULT; }
    pb = (PBYTE)malloc(sz ? (ULONG)sz : 1);
    if (!pb) { fclose(tmp); return ERROR_NOT_ENOUGH_MEMORY; }
    rewind(tmp);
    if (sz > 0 && fread(pb, 1, (ULONG)sz, tmp) != (ULONG)sz) {
        free(pb); fclose(tmp); return ERROR_READ_FAULT;
    }
    fclose(tmp);
    rc = HfsAddFile(hHfs, pszName, pb, (ULONG)sz);
    free(pb);
    return rc;
}

/*!
 * @brief Query the number of internal files.
 *
 * @param[in]  hHfs     Handle. Not NULLHANDLE.
 * @param[out] pulCount Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 */
APIRET APIENTRY HfsQueryFileCount(HHFS hHfs, PULONG pulCount)
{
    if (hHfs == NULLHANDLE || !pulCount) return ERROR_INVALID_PARAMETER;
    return VectorGetCount(HFS_FROM_HANDLE(hHfs)->vFiles, pulCount);
}

/*!
 * @brief Query an internal file name by index.
 *
 * @param[in]  hHfs    Handle. Not NULLHANDLE.
 * @param[in]  ulIndex Zero-based index.
 * @param[out] pszBuf  Output buffer. May be NULL for size query.
 * @param[in]  ulSize  Size of @a pszBuf in bytes.
 * @param[out] pulUsed Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hHfs is NULL.
 * @retval ERROR_NO_MORE_ITEMS      @a ulIndex out of range.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY HfsQueryFileName(HHFS hHfs, ULONG ulIndex,
                                 PSZ pszBuf, ULONG ulSize, PULONG pulUsed)
{
    HfsFileRec f; APIRET rc; ULONG len;
    if (hHfs == NULLHANDLE) return ERROR_INVALID_PARAMETER;
    rc = VectorGetItem(HFS_FROM_HANDLE(hHfs)->vFiles, ulIndex, &f, sizeof(f), NULL);
    if (rc != NO_ERROR) return rc;
    len = (ULONG)strlen(f.pszName);
    if (!pszBuf && ulSize == 0) { if (pulUsed) *pulUsed = len; return NO_ERROR; }
    if (ulSize < len + 1) { if (pulUsed) *pulUsed = len + 1; return ERROR_BUFFER_OVERFLOW; }
    memcpy(pszBuf, f.pszName, len + 1);
    if (pulUsed) *pulUsed = len;
    return NO_ERROR;
}

/*!
 * @brief Write the container to a file.
 *
 * Layout per helpfile.txt:
 *   [HLPHEADER 16]
 *   [FILEHEADER 9]                <-- directory_start points here
 *   [directory B-tree dirSize]    <-- UsedSpace = dirSize
 *   [FILEHEADER 9][file body]     <-- per internal file
 *   ...
 *
 * @param[in] hHfs        Handle. Not NULLHANDLE.
 * @param[in] pszFileName Output file name. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL, or the
 *                                  container has no files.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 * @retval ERROR_FILE_NOT_FOUND     Output file cannot be created.
 */
APIRET APIENTRY HfsWrite(HHFS hHfs, PCSZ pszFileName)
{
    HfsRec* p;
    FILE* f;
    ULONG n, i;
    ULONG dirStart, dirSize, cur;
    PULONG offsets;
    HBTR hBtr;
    APIRET rc;

    if (hHfs == NULLHANDLE || !pszFileName) return ERROR_INVALID_PARAMETER;
    p = HFS_FROM_HANDLE(hHfs);
    VectorGetCount(p->vFiles, &n);
    if (n == 0) return ERROR_INVALID_PARAMETER;

    /* First pass: build the directory B-tree into a temp file to learn its size. */
    rc = BtrCreate(&hBtr, p->usDirPageSize,
                   BTR_FLAG_DIRECTORY | BTR_FLAG_ALWAYS, "Lz4");
    if (rc != NO_ERROR) return rc;
    for (i = 0; i < n; i++) {
        HfsFileRec x;
        VectorGetItem(p->vFiles, i, &x, sizeof(x), NULL);
        BtrAddEntry(hBtr, x.pszName, 0);
    }
    {
        FILE* tmp = tmpfile();
        if (!tmp) { BtrDestroy(hBtr); return ERROR_NOT_ENOUGH_MEMORY; }
        BtrWrite(hBtr, tmp);
        dirSize = (ULONG)ftell(tmp);
        fclose(tmp);
    }
    BtrDestroy(hBtr);

    dirStart = sizeof(HLPHEADER);                     /* 16 */
    cur = dirStart + sizeof(FILEHEADER) + dirSize;

    offsets = (PULONG)malloc(n * sizeof(ULONG));
    if (!offsets) return ERROR_NOT_ENOUGH_MEMORY;
    for (i = 0; i < n; i++) {
        HfsFileRec x;
        VectorGetItem(p->vFiles, i, &x, sizeof(x), NULL);
        offsets[i] = cur;
        cur += (ULONG)(sizeof(FILEHEADER) + x.ulSize);
    }

    f = fopen(pszFileName, "wb");
    if (!f) { free(offsets); return ERROR_FILE_NOT_FOUND; }

    /* HLPHEADER */
    {
        HLPHEADER h;
        h.ulMagic = 0x00035F3F;
        h.ulDirectoryStart = dirStart;
        h.ulFreeChainStart = 0xFFFFFFFF;
        h.ulEntireFileSize = cur;
        fwrite(&h, sizeof(h), 1, f);
    }

    /*
     * FILEHEADER for the directory itself.  helpdeco and Rusty HLP Viewer
     * both expect a valid FILEHEADER at directory_start, followed by the
     * B-tree.  ReservedSpace includes the 9-byte FILEHEADER.
     */
    {
        FILEHEADER fh;
        fh.ulReservedSpace = dirSize + (ULONG)sizeof(FILEHEADER);
        fh.ulUsedSpace     = dirSize;
        fh.bFileFlags      = 0;
        fwrite(&fh, sizeof(fh), 1, f);
    }

    /* Second pass: rebuild the B-tree with real offsets and write it. */
    rc = BtrCreate(&hBtr, p->usDirPageSize,
                   BTR_FLAG_DIRECTORY | BTR_FLAG_ALWAYS, "Lz4");
    if (rc != NO_ERROR) { fclose(f); free(offsets); return rc; }
    for (i = 0; i < n; i++) {
        HfsFileRec x;
        VectorGetItem(p->vFiles, i, &x, sizeof(x), NULL);
        BtrAddEntry(hBtr, x.pszName, offsets[i]);
    }
    BtrWrite(hBtr, f);
    BtrDestroy(hBtr);

    /* Internal file bodies: FILEHEADER + payload. */
    for (i = 0; i < n; i++) {
        HfsFileRec x; FILEHEADER fh;
        VectorGetItem(p->vFiles, i, &x, sizeof(x), NULL);
        fh.ulReservedSpace = x.ulSize + (ULONG)sizeof(FILEHEADER);
        fh.ulUsedSpace     = x.ulSize;
        fh.bFileFlags      = 0;
        fwrite(&fh, sizeof(fh), 1, f);
        if (x.ulSize) fwrite(x.pbData, 1, x.ulSize, f);
    }
    fclose(f);
    free(offsets);
    return NO_ERROR;
}
