/*!
 * @file fnt.c
 * @brief |FONT implementation.
 */
#include "fnt.h"
#include "vector.h"
#include <stdlib.h>
#include <string.h>

#pragma pack(1)
typedef struct {
    USHORT usNumFacenames, usNumDescriptors, usFacenamesOffset,
           usDescriptorsOffset, usNumFormats, usFormatsOffset,
           usNumCharmaps, usCharmapsOffset;
} FONTHEADER;
typedef struct {
    UCHAR bAttributes, bHalfPoints, bFontFamily;
    USHORT usFontName;
    UCHAR abFGRGB[3], abBGRGB[3];
} OLDFONT;
#pragma pack()

#define FNT_FACENAME_LEN 20
typedef struct {
    ULONG ulFaceIndex, ulHalfPoints, ulAttributes, ulFamily;
    UCHAR abFGRGB[3], abBGRGB[3];
} FntDescRec;
typedef struct { HVECTOR vFaceNames, vDescriptors; } FntRec;

#define FNT_FROM_HANDLE(h) ((FntRec*)(h))
#define FNT_HANDLE_FROM(d) ((HANDLE)(d))

APIRET APIENTRY FntCreate(PHFNT phFnt)
{
    FntRec* p; APIRET rc;
    if (!phFnt) return ERROR_INVALID_PARAMETER;
    *phFnt = NULLHANDLE;
    p = (FntRec*)malloc(sizeof(FntRec));
    if (!p) return ERROR_NOT_ENOUGH_MEMORY;
    memset(p, 0, sizeof(*p));
    rc = VectorCreate(sizeof(PSZ), &p->vFaceNames);
    if (rc != NO_ERROR) { free(p); return rc; }
    rc = VectorCreate(sizeof(FntDescRec), &p->vDescriptors);
    if (rc != NO_ERROR) { VectorDestroy(p->vFaceNames); free(p); return rc; }
    *phFnt = FNT_HANDLE_FROM(p);
    return NO_ERROR;
}

APIRET APIENTRY FntDestroy(HFNT hFnt)
{
    FntRec* p; ULONG n, i;
    if (hFnt == NULLHANDLE) return NO_ERROR;
    p = FNT_FROM_HANDLE(hFnt);
    if (p->vFaceNames) {
        VectorGetCount(p->vFaceNames, &n);
        for (i = 0; i < n; i++) {
            PSZ s = NULL;
            VectorGetItem(p->vFaceNames, i, &s, sizeof(s), NULL);
            if (s) free(s);
        }
        VectorDestroy(p->vFaceNames);
    }
    if (p->vDescriptors) VectorDestroy(p->vDescriptors);
    free(p);
    return NO_ERROR;
}

APIRET APIENTRY FntAddFaceName(HFNT hFnt, PCSZ pszName, PULONG pulIndex)
{
    FntRec* p; PSZ c; ULONG n = 0; APIRET rc;
    if (hFnt == NULLHANDLE || !pszName) return ERROR_INVALID_PARAMETER;
    p = FNT_FROM_HANDLE(hFnt);
    c = strdup(pszName);
    if (!c) return ERROR_NOT_ENOUGH_MEMORY;
    rc = VectorAdd(p->vFaceNames, &c);
    if (rc != NO_ERROR) { free(c); return rc; }
    VectorGetCount(p->vFaceNames, &n);
    if (pulIndex) *pulIndex = n - 1;
    return NO_ERROR;
}

APIRET APIENTRY FntGetOrCreateDescriptor(HFNT hFnt, ULONG ulFaceIndex,
                                         ULONG ulHalfPoints, ULONG ulAttributes,
                                         PBYTE pbFGRGB, PBYTE pbBGRGB,
                                         ULONG ulFamily, PULONG pulIndex)
{
    FntRec* p; ULONG n = 0, i; FntDescRec d;
    UCHAR fg[3] = { 0, 0, 0 }, bg[3] = { 0xFF, 0xFF, 0xFF };
    PBYTE pfg, pbg;
    if (hFnt == NULLHANDLE) return ERROR_INVALID_PARAMETER;
    p = FNT_FROM_HANDLE(hFnt);
    pfg = pbFGRGB ? pbFGRGB : fg;
    pbg = pbBGRGB ? pbBGRGB : bg;
    VectorGetCount(p->vDescriptors, &n);
    for (i = 0; i < n; i++) {
        VectorGetItem(p->vDescriptors, i, &d, sizeof(d), NULL);
        if (d.ulFaceIndex == ulFaceIndex && d.ulHalfPoints == ulHalfPoints &&
            d.ulAttributes == ulAttributes && d.ulFamily == ulFamily &&
            memcmp(d.abFGRGB, pfg, 3) == 0 &&
            memcmp(d.abBGRGB, pbg, 3) == 0) {
            if (pulIndex) *pulIndex = i;
            return NO_ERROR;
        }
    }
    d.ulFaceIndex = ulFaceIndex;
    d.ulHalfPoints = ulHalfPoints;
    d.ulAttributes = ulAttributes;
    d.ulFamily = ulFamily;
    memcpy(d.abFGRGB, pfg, 3);
    memcpy(d.abBGRGB, pbg, 3);
    {
        APIRET rc = VectorAdd(p->vDescriptors, &d);
        if (rc != NO_ERROR) return rc;
    }
    if (pulIndex) *pulIndex = n;
    return NO_ERROR;
}

APIRET APIENTRY FntQueryFaceCount(HFNT hFnt, PULONG pulCount)
{
    if (hFnt == NULLHANDLE || !pulCount) return ERROR_INVALID_PARAMETER;
    return VectorGetCount(FNT_FROM_HANDLE(hFnt)->vFaceNames, pulCount);
}

APIRET APIENTRY FntQueryFaceName(HFNT hFnt, ULONG ulIndex,
                                 PSZ pszBuf, ULONG ulSize, PULONG pulUsed)
{
    FntRec* p; PSZ s = NULL; APIRET rc; ULONG len;
    if (hFnt == NULLHANDLE) return ERROR_INVALID_PARAMETER;
    p = FNT_FROM_HANDLE(hFnt);
    rc = VectorGetItem(p->vFaceNames, ulIndex, &s, sizeof(s), NULL);
    if (rc != NO_ERROR) return rc;
    if (!s) return ERROR_FILE_NOT_FOUND;
    len = (ULONG)strlen(s);
    if (!pszBuf && ulSize == 0) { if (pulUsed) *pulUsed = len; return NO_ERROR; }
    if (ulSize < len + 1) { if (pulUsed) *pulUsed = len + 1; return ERROR_BUFFER_OVERFLOW; }
    memcpy(pszBuf, s, len + 1);
    if (pulUsed) *pulUsed = len;
    return NO_ERROR;
}

APIRET APIENTRY FntQueryDescriptorCount(HFNT hFnt, PULONG pulCount)
{
    if (hFnt == NULLHANDLE || !pulCount) return ERROR_INVALID_PARAMETER;
    return VectorGetCount(FNT_FROM_HANDLE(hFnt)->vDescriptors, pulCount);
}

APIRET APIENTRY FntWrite(HFNT hFnt, FILE* f)
{
    FntRec* p; FONTHEADER h; ULONG nf = 0, nd = 0, i;
    if (hFnt == NULLHANDLE || !f) return ERROR_INVALID_PARAMETER;
    p = FNT_FROM_HANDLE(hFnt);
    VectorGetCount(p->vFaceNames, &nf);
    VectorGetCount(p->vDescriptors, &nd);
    if (nf == 0) {
        PSZ s = strdup("Helv");
        if (!s) return ERROR_NOT_ENOUGH_MEMORY;
        VectorAdd(p->vFaceNames, &s);
        nf = 1;
    }
    if (nd == 0) {
        FntDescRec d;
        memset(&d, 0, sizeof(d));
        d.ulHalfPoints = 20;
        d.ulFamily = 2;
        d.abBGRGB[0] = d.abBGRGB[1] = d.abBGRGB[2] = 0xFF;
        VectorAdd(p->vDescriptors, &d);
        nd = 1;
    }
    memset(&h, 0, sizeof(h));
    h.usNumFacenames = (USHORT)nf;
    h.usNumDescriptors = (USHORT)nd;
    h.usFacenamesOffset = sizeof(h);
    h.usDescriptorsOffset = h.usFacenamesOffset + (USHORT)(nf * FNT_FACENAME_LEN);
    fwrite(&h, sizeof(h), 1, f);
    for (i = 0; i < nf; i++) {
        CHAR nm[FNT_FACENAME_LEN]; PSZ s = NULL;
        VectorGetItem(p->vFaceNames, i, &s, sizeof(s), NULL);
        memset(nm, 0, sizeof(nm));
        if (s) strncpy(nm, s, FNT_FACENAME_LEN - 1);
        fwrite(nm, FNT_FACENAME_LEN, 1, f);
    }
    for (i = 0; i < nd; i++) {
        FntDescRec d; OLDFONT o;
        VectorGetItem(p->vDescriptors, i, &d, sizeof(d), NULL);
        memset(&o, 0, sizeof(o));
        o.bAttributes = (UCHAR)d.ulAttributes;
        o.bHalfPoints = (UCHAR)d.ulHalfPoints;
        o.bFontFamily = (UCHAR)d.ulFamily;
        o.usFontName = (USHORT)d.ulFaceIndex;
        memcpy(o.abFGRGB, d.abFGRGB, 3);
        memcpy(o.abBGRGB, d.abBGRGB, 3);
        fwrite(&o, sizeof(o), 1, f);
    }
    return NO_ERROR;
}
