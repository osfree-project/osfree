/*!
 * @file gpi_entry.cpp
 * @brief Client-side dispatcher and per-PS state table.
 */
#define INCL_GPI
#include "F_platform.h"
#include "F_cmd.h"
#include "gpi_entry.h"
#include "pmclient.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * @brief Unified logging function (defined in unimpl.cpp).
 * @param [in] fmt  printf-style format string.
 * @param [in] ...  Arguments for the format string.
 */
extern void log(const char *fmt, ...);

/* Response header (16 bytes on the wire) */
struct FPM_RESP_HEADER {
    ULONG ulStatus;    /*!< 0 = ok, otherwise PMERR_* or APIRET   */
    ULONG ulResult;    /*!< Return value (LONG/BOOL/HANDLE)       */
    ULONG cbPayload;   /*!< Payload size (e.g., for Query*)       */
    ULONG ulLastError; /*!< Per-connection last error             */
};

/*!
 * @def GPI_MAX_PS
 * @brief Maximum number of simultaneous PS entries tracked on the client.
 */
#define GPI_MAX_PS 64

static GPI_PS g_psTable[GPI_MAX_PS];

/* --------------------------------------------------------------------- */
/*  Per-PS table                                                         */
/* --------------------------------------------------------------------- */

PGPI_PS GpiPsCreate(HPS hps, ULONG flOptions)
{
    int i;

    if (hps == NULL || (ULONG)hps == (ULONG)GPI_ERROR)
        return (PGPI_PS)NULL;

    for (i = 0; i < GPI_MAX_PS; i++) {
        if (g_psTable[i].inUse && g_psTable[i].hps == hps) {
            g_psTable[i].flOptions = flOptions;
            return &g_psTable[i];
        }
    }

    for (i = 0; i < GPI_MAX_PS; i++) {
        if (!g_psTable[i].inUse) {
            g_psTable[i].inUse     = TRUE;
            g_psTable[i].hps       = hps;
            g_psTable[i].flOptions = flOptions;
            return &g_psTable[i];
        }
    }

    log("[PMGPI] WARN: PS table full, dropping hps=%p\n", (void*)hps);
    return (PGPI_PS)NULL;
}

PGPI_PS GpiPsFind(HPS hps)
{
    int i;

    if (hps == NULL)
        return (PGPI_PS)NULL;

    for (i = 0; i < GPI_MAX_PS; i++) {
        if (g_psTable[i].inUse && g_psTable[i].hps == hps)
            return &g_psTable[i];
    }
    return (PGPI_PS)NULL;
}

void GpiPsSetOptions(HPS hps, ULONG flOptions)
{
    PGPI_PS ps = GpiPsFind(hps);
    if (ps)
        ps->flOptions = flOptions;
}

void GpiPsDestroy(HPS hps)
{
    PGPI_PS ps = GpiPsFind(hps);
    if (ps) {
        ps->inUse     = FALSE;
        ps->hps       = NULL;
        ps->flOptions = 0;
    }
}

/* --------------------------------------------------------------------- */
/*  Transport                                                            */
/* --------------------------------------------------------------------- */

ULONG GpiDispatch(HPS hps, ULONG ulFnCode,
                  const void *pArgs, ULONG cbArgs,
                  ULONG *pulResult)
{
    struct FPM_RESP_HEADER resp;
    LONG   len;
    APIRET rc;
    ULONG  cmd;

    if ((ulFnCode & 0xFFFF0000UL) != 0) {
        log("[PMGPI] ERROR: ulFnCode=0x%lx does not fit in 16-bit opcode\n",
            (unsigned long)ulFnCode);
        return 1;
    }

    cmd = F_CMD_GPI(ulFnCode);

    log("[PMGPI] Transport IN : Cmd=0x%lx (class=%lu, op=0x%lx), HPS=%p, cbArgs=%lu\n",
        (unsigned long)cmd,
        (unsigned long)F_CMD_GET_CLASS(cmd),
        (unsigned long)F_CMD_GET_OPCODE(cmd),
        (void*)hps, (unsigned long)cbArgs);

    rc = PmcPostCommand(cmd, (ULONG)hps);
    if (rc) {
        log("[PMGPI] Transport ERR: PmcPostCommand failed with %d\n", (int)rc);
        return rc;
    }

    rc = PmcSendData(&cbArgs, sizeof(cbArgs));
    if (rc) {
        log("[PMGPI] Transport ERR: PmcSendData(cbArgs) failed with %d\n", (int)rc);
        return rc;
    }

    if (cbArgs > 0 && pArgs != NULL) {
        rc = PmcSendData((PVOID)pArgs, cbArgs);
        if (rc) {
            log("[PMGPI] Transport ERR: PmcSendData(args) failed with %d\n", (int)rc);
            return rc;
        }
    }

    len = 0;
    rc = PmcGetData(&resp, &len, sizeof(resp));
    if (rc || len != (LONG)sizeof(resp)) {
        log("[PMGPI] Transport ERR: PmcGetData(header) failed rc=%d, len=%d\n",
            (int)rc, (int)len);
        return (rc != 0) ? rc : 1;
    }

    if (resp.cbPayload > 0 && pArgs != NULL && cbArgs >= resp.cbPayload) {
        len = 0;
        rc = PmcGetData((PVOID)pArgs, &len, resp.cbPayload);
        if (rc) {
            log("[PMGPI] Transport ERR: PmcGetData(payload) failed with %d\n", (int)rc);
            return rc;
        }
    }

    log("[PMGPI] Transport OUT: Status=0x%x, Result=0x%x\n",
        (unsigned)resp.ulStatus, (unsigned)resp.ulResult);

    *pulResult = resp.ulResult;
    return (resp.ulStatus != 0) ? resp.ulStatus : 0;
}

#ifdef __cplusplus
}
#endif
