/*!
 * @file sys.c
 * @brief |SYSTEM implementation.
 */
#include "sys.h"
#include "vector.h"
#include <stdlib.h>
#include <string.h>

#pragma pack(1)
typedef struct {
    USHORT usMagic, usMinor, usMajor;
    ULONG  ulGenDate;
    USHORT usFlags;
} SYSTEMHEADER;
#pragma pack()

typedef struct { USHORT usRecType, usDataSize; } SysRecHdr;
typedef struct { PSZ pszText; } SysConfigRec;
typedef struct { PBYTE pbData; ULONG ulSize; } SysWindowRec;

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

#define SYS_FROM_HANDLE(h) ((SysRec*)(h))
#define SYS_HANDLE_FROM(d) ((HANDLE)(d))

APIRET APIENTRY SysCreate(PHSYS phSys)
{
    SysRec* p; APIRET rc;
    if (!phSys) return ERROR_INVALID_PARAMETER;
    *phSys = NULLHANDLE;
    p = (SysRec*)malloc(sizeof(SysRec));
    if (!p) return ERROR_NOT_ENOUGH_MEMORY;
    memset(p, 0, sizeof(*p));
    rc = VectorCreate(sizeof(SysConfigRec), &p->vConfigs);
    if (rc != NO_ERROR) { free(p); return rc; }
    rc = VectorCreate(sizeof(SysWindowRec), &p->vWindows);
    if (rc != NO_ERROR) { VectorDestroy(p->vConfigs); free(p); return rc; }
    *phSys = SYS_HANDLE_FROM(p);
    return NO_ERROR;
}

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

APIRET APIENTRY SysSetContents(HSYS hSys, ULONG ulTopicOffset)
{
    SysRec* p;
    if (hSys == NULLHANDLE) return ERROR_INVALID_PARAMETER;
    p = SYS_FROM_HANDLE(hSys);
    p->ulContents = ulTopicOffset;
    p->fHasContents = TRUE;
    return NO_ERROR;
}

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

APIRET APIENTRY SysAddConfig(HSYS hSys, PCSZ pszConfig)
{
    SysRec* p; SysConfigRec r;
    if (hSys == NULLHANDLE || !pszConfig) return ERROR_INVALID_PARAMETER;
    p = SYS_FROM_HANDLE(hSys);
    r.pszText = strdup(pszConfig);
    if (!r.pszText) return ERROR_NOT_ENOUGH_MEMORY;
    return VectorAdd(p->vConfigs, &r);
}

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

static VOID sys_emit(FILE* f, USHORT type, PCVOID data, USHORT size)
{
    SysRecHdr h;
    h.usRecType = type;
    h.usDataSize = size;
    fwrite(&h, sizeof(h), 1, f);
    if (data && size) fwrite(data, 1, size, f);
}

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
