/*!
 * @file sys.c
 * @brief |SYSTEM implementation.
 */
#include "sys.h"
#include "vector.h"
#include <stdlib.h>
#include <string.h>

#pragma pack(1)
/*!
 * @struct SYSTEMHEADER
 * @brief On-disk |SYSTEM header (per helpdeco).
 */
typedef struct {
    USHORT usMagic, usMinor, usMajor;
    ULONG  ulGenDate;
    USHORT usFlags;
} SYSTEMHEADER;
#pragma pack()

/*!
 * @struct SysRecHdr
 * @brief Common record header for every |SYSTEM record.
 */
typedef struct { USHORT usRecType, usDataSize; } SysRecHdr;
/*!
 * @struct SysConfigRec
 * @brief One [CONFIG] macro line.
 */
typedef struct { PSZ pszText; } SysConfigRec;
/*!
 * @struct SysWindowRec
 * @brief One opaque [WINDOWS] window record.
 */
typedef struct { PBYTE pbData; ULONG ulSize; } SysWindowRec;

/*!
 * @struct SysRec
 * @brief |SYSTEM generator state.
 */
typedef struct {
    char   szTitle[33];
    char*  pszCopyright;
    char*  pszCitation;
    char*  pszCnt;
    ULONG  ulContents;
    BOOL   fHasContents;
    BOOL   fHasLcid;
    USHORT usLcidLang, usLcidSub, usLcidCP;
    HVECTOR vConfigs;
    HVECTOR vWindows;
} SysRec;

/*!
 * @def SYS_FROM_HANDLE
 * @brief Convert an HSYS handle to its SysRec pointer.
 * @param[in] h Handle.
 */
#define SYS_FROM_HANDLE(h) ((SysRec*)(h))
/*!
 * @def SYS_HANDLE_FROM
 * @brief Convert a SysRec pointer to an HSYS handle.
 * @param[in] d Record pointer.
 */
#define SYS_HANDLE_FROM(d) ((HANDLE)(d))

/*!
 * @brief Create an empty |SYSTEM generator.
 *
 * @param[out] phSys Receiver. Not NULL. Set to NULLHANDLE on error.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a phSys is NULL.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY SysCreate(PHSYS phSys)
{
    SysRec* p; APIRET rc;
    if (!phSys) return ERROR_INVALID_PARAMETER;
    *phSys = NULLHANDLE;
    p = (SysRec*)malloc(sizeof(SysRec));
    if (!p) return ERROR_NOT_ENOUGH_MEMORY;
    memset(p, 0, sizeof(*p));
    rc = VectorCreate(sizeof(SysConfigRec), 0, NULL, &p->vConfigs);
    if (rc != NO_ERROR) { free(p); return rc; }
    rc = VectorCreate(sizeof(SysWindowRec), 0, NULL, &p->vWindows);
    if (rc != NO_ERROR) { VectorDestroy(p->vConfigs); free(p); return rc; }
    *phSys = SYS_HANDLE_FROM(p);
    return NO_ERROR;
}

/*!
 * @brief Destroy a |SYSTEM generator and release all its data.
 *
 * @param[in] hSys Handle. May be NULLHANDLE.
 *
 * @return APIRET
 * @retval NO_ERROR  Success. Also for NULLHANDLE.
 */
APIRET APIENTRY SysDestroy(HSYS hSys)
{
    SysRec* p; ULONG n, i;
    if (hSys == NULLHANDLE) return NO_ERROR;
    p = SYS_FROM_HANDLE(hSys);
    if (p->pszCopyright) free(p->pszCopyright);
    if (p->pszCitation) free(p->pszCitation);
    if (p->pszCnt) free(p->pszCnt);
    if (p->vConfigs) {
        VectorGetCount(p->vConfigs, &n);
        for (i = 0; i < n; i++) {
            SysConfigRec r;
            VectorGetItem(p->vConfigs, i, &r, sizeof(r), NULL);
            if (r.pszText) free(r.pszText);
        }
        VectorDestroy(p->vConfigs);
    }
    if (p->vWindows) {
        VectorGetCount(p->vWindows, &n);
        for (i = 0; i < n; i++) {
            SysWindowRec r;
            VectorGetItem(p->vWindows, i, &r, sizeof(r), NULL);
            if (r.pbData) free(r.pbData);
        }
        VectorDestroy(p->vWindows);
    }
    free(p);
    return NO_ERROR;
}

/*!
 * @brief Set the help file title.
 *
 * @param[in] hSys     Handle. Not NULLHANDLE.
 * @param[in] pszTitle Title string. May be NULL to clear.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hSys is NULL.
 */
APIRET APIENTRY SysSetTitle(HSYS hSys, PCSZ pszTitle)
{
    SysRec* p;
    if (hSys == NULLHANDLE) return ERROR_INVALID_PARAMETER;
    p = SYS_FROM_HANDLE(hSys);
    if (pszTitle) {
        strncpy(p->szTitle, pszTitle, 32);
        p->szTitle[32] = '\0';
    } else {
        p->szTitle[0] = '\0';
    }
    return NO_ERROR;
}

/*!
 * @brief Set the copyright string.
 *
 * @param[in] hSys         Handle. Not NULLHANDLE.
 * @param[in] pszCopyright Copyright string. May be NULL to clear.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hSys is NULL.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY SysSetCopyright(HSYS hSys, PCSZ pszCopyright)
{
    SysRec* p;
    if (hSys == NULLHANDLE) return ERROR_INVALID_PARAMETER;
    p = SYS_FROM_HANDLE(hSys);
    if (p->pszCopyright) { free(p->pszCopyright); p->pszCopyright = NULL; }
    if (pszCopyright) {
        p->pszCopyright = strdup(pszCopyright);
        if (!p->pszCopyright) return ERROR_NOT_ENOUGH_MEMORY;
    }
    return NO_ERROR;
}

/*!
 * @brief Set the contents topic offset.
 *
 * @param[in] hSys          Handle. Not NULLHANDLE.
 * @param[in] ulTopicOffset Offset of the contents topic in |TOPIC.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hSys is NULL.
 */
APIRET APIENTRY SysSetContents(HSYS hSys, ULONG ulTopicOffset)
{
    SysRec* p;
    if (hSys == NULLHANDLE) return ERROR_INVALID_PARAMETER;
    p = SYS_FROM_HANDLE(hSys);
    p->ulContents = ulTopicOffset;
    p->fHasContents = TRUE;
    return NO_ERROR;
}

/*!
 * @brief Set the citation string.
 *
 * @param[in] hSys        Handle. Not NULLHANDLE.
 * @param[in] pszCitation Citation string. May be NULL to clear.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hSys is NULL.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY SysSetCitation(HSYS hSys, PCSZ pszCitation)
{
    SysRec* p;
    if (hSys == NULLHANDLE) return ERROR_INVALID_PARAMETER;
    p = SYS_FROM_HANDLE(hSys);
    if (p->pszCitation) { free(p->pszCitation); p->pszCitation = NULL; }
    if (pszCitation) {
        p->pszCitation = strdup(pszCitation);
        if (!p->pszCitation) return ERROR_NOT_ENOUGH_MEMORY;
    }
    return NO_ERROR;
}

/*!
 * @brief Set the language, sub-language, and code page.
 *
 * @param[in] hSys       Handle. Not NULLHANDLE.
 * @param[in] usLang     Primary language identifier.
 * @param[in] usSubLang  Sub-language identifier.
 * @param[in] usCodePage Code page identifier.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hSys is NULL.
 */
APIRET APIENTRY SysSetLcid(HSYS hSys, USHORT usLang, USHORT usSubLang, USHORT usCodePage)
{
    SysRec* p;
    if (hSys == NULLHANDLE) return ERROR_INVALID_PARAMETER;
    p = SYS_FROM_HANDLE(hSys);
    p->fHasLcid = TRUE;
    p->usLcidLang = usLang;
    p->usLcidSub = usSubLang;
    p->usLcidCP = usCodePage;
    return NO_ERROR;
}

/*!
 * @brief Set the .cnt file name.
 *
 * @param[in] hSys   Handle. Not NULLHANDLE.
 * @param[in] pszCnt Contents file name. May be NULL to clear.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hSys is NULL.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY SysSetCnt(HSYS hSys, PCSZ pszCnt)
{
    SysRec* p;
    if (hSys == NULLHANDLE) return ERROR_INVALID_PARAMETER;
    p = SYS_FROM_HANDLE(hSys);
    if (p->pszCnt) { free(p->pszCnt); p->pszCnt = NULL; }
    if (pszCnt) {
        p->pszCnt = strdup(pszCnt);
        if (!p->pszCnt) return ERROR_NOT_ENOUGH_MEMORY;
    }
    return NO_ERROR;
}

/*!
 * @brief Append a [CONFIG] macro line.
 *
 * @param[in] hSys      Handle. Not NULLHANDLE.
 * @param[in] pszConfig Macro line. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hSys or @a pszConfig is NULL.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY SysAddConfig(HSYS hSys, PCSZ pszConfig)
{
    SysRec* p; SysConfigRec r;
    if (hSys == NULLHANDLE || !pszConfig) return ERROR_INVALID_PARAMETER;
    p = SYS_FROM_HANDLE(hSys);
    r.pszText = strdup(pszConfig);
    if (!r.pszText) return ERROR_NOT_ENOUGH_MEMORY;
    return VectorAdd(p->vConfigs, &r);
}

/*!
 * @brief Append a secondary window definition.
 *
 * @param[in] hSys     Handle. Not NULLHANDLE.
 * @param[in] pvWindow Pointer to the window record. Not NULL.
 * @param[in] ulSize   Size of the window record in bytes. Non-zero.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL or @a ulSize is 0.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY SysAddWindow(HSYS hSys, PCVOID pvWindow, ULONG ulSize)
{
    SysRec* p; SysWindowRec r;
    if (hSys == NULLHANDLE || !pvWindow || ulSize == 0)
        return ERROR_INVALID_PARAMETER;
    p = SYS_FROM_HANDLE(hSys);
    r.pbData = (PBYTE)malloc(ulSize);
    if (!r.pbData) return ERROR_NOT_ENOUGH_MEMORY;
    memcpy(r.pbData, pvWindow, ulSize);
    r.ulSize = ulSize;
    return VectorAdd(p->vWindows, &r);
}

/*!
 * @brief Emit one |SYSTEM record (header plus optional payload).
 *
 * @param[in] f    Output stream.
 * @param[in] type SYS_REC_* record type.
 * @param[in] data Payload bytes, or NULL.
 * @param[in] size Payload length in bytes.
 */
static VOID sys_emit(FILE* f, USHORT type, PCVOID data, USHORT size)
{
    SysRecHdr h;
    h.usRecType = type;
    h.usDataSize = size;
    fwrite(&h, sizeof(h), 1, f);
    if (data && size) fwrite(data, 1, size, f);
}

/*!
 * @brief Serialize the |SYSTEM file to a stream.
 *
 * @param[in] hSys Handle. Not NULLHANDLE.
 * @param[in] f    Output stream. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hSys or @a f is NULL.
 */
APIRET APIENTRY SysWrite(HSYS hSys, FILE* f)
{
    SysRec* p; SYSTEMHEADER h; ULONG n, i;
    if (hSys == NULLHANDLE || !f) return ERROR_INVALID_PARAMETER;
    p = SYS_FROM_HANDLE(hSys);
    h.usMagic = 0x036C;
    h.usMinor = 15;
    h.usMajor = 1;
    h.ulGenDate = 0;
    h.usFlags = 0;
    fwrite(&h, sizeof(h), 1, f);

    if (p->szTitle[0])
        sys_emit(f, SYS_REC_TITLE, p->szTitle,
                 (USHORT)(strlen(p->szTitle) + 1));
    if (p->pszCopyright)
        sys_emit(f, SYS_REC_COPYRIGHT, p->pszCopyright,
                 (USHORT)(strlen(p->pszCopyright) + 1));
    if (p->fHasContents)
        sys_emit(f, SYS_REC_CONTENTS, &p->ulContents, sizeof(p->ulContents));
    VectorGetCount(p->vConfigs, &n);
    for (i = 0; i < n; i++) {
        SysConfigRec r;
        VectorGetItem(p->vConfigs, i, &r, sizeof(r), NULL);
        sys_emit(f, SYS_REC_CONFIG, r.pszText,
                 (USHORT)(strlen(r.pszText) + 1));
    }
    VectorGetCount(p->vWindows, &n);
    for (i = 0; i < n; i++) {
        SysWindowRec r;
        VectorGetItem(p->vWindows, i, &r, sizeof(r), NULL);
        sys_emit(f, SYS_REC_WINDOW, r.pbData, (USHORT)r.ulSize);
    }
    if (p->pszCitation)
        sys_emit(f, SYS_REC_CITATION, p->pszCitation,
                 (USHORT)(strlen(p->pszCitation) + 1));
    if (p->fHasLcid) {
        UCHAR lcid[12]; USHORT tmp;
        memset(lcid, 0, sizeof(lcid));
        tmp = p->usLcidCP;   memcpy(lcid + 0, &tmp, 2);
        tmp = p->usLcidLang; memcpy(lcid + 2, &tmp, 2);
        tmp = p->usLcidSub;  memcpy(lcid + 4, &tmp, 2);
        sys_emit(f, SYS_REC_LCID, lcid, sizeof(lcid));
    }
    if (p->pszCnt)
        sys_emit(f, SYS_REC_CNT, p->pszCnt,
                 (USHORT)(strlen(p->pszCnt) + 1));
    return NO_ERROR;
}
