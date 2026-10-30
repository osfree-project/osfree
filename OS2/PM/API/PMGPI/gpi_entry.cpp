/*!
 * @file gpi_entry.c
 * @brief Client-side dispatcher.
 */
#define INCL_GPI
#include "gpi_platform.h"
#include "gpi_entry.h"
#include "gpi_wire.h"
#include "pmclient.h"

/* Declaration of the logging function from unimpl.cpp */
extern void log(const char *fmt, ...);

/* Command code for the GPI dispatcher (must match the server) */
#ifndef F_CMD_GPI_DISPATCH
#define F_CMD_GPI_DISPATCH 0x0100
#endif

/* Request structure (from the specification) */
struct FPM_GPI_CALL {
    ULONG   ulFnCode;    /* N_GpiLine, N_GpiSetColor, ... */
    ULONG   cbArgs;      /* size of marshaled arguments */
    ULONG   ulFlags;     /* reserved */
};

/* Response structure (from the specification) */
struct FPM_RESP_HEADER {
    ULONG   ulStatus;    /* 0 = ok, otherwise PMERR_* or APIRET */
    ULONG   ulResult;    /* return value (LONG/BOOL/HANDLE) */
    ULONG   cbPayload;   /* payload size (e.g., for Query*) */
    ULONG   ulLastError; /* per-connection last error */
};

/*!
 * @brief Client dispatcher.
 * @param [in] hps Presentation space handle.
 * @param [in] ulFnCode Function code (NGpi*).
 * @param [in] pArgs Marshaled arguments.
 * @param [in] cbArgs Size of pArgs in bytes.
 * @param [out] pulResult Receives result from server.
 * @return 0 on success, non-zero on transport error.
 */
ULONG GpiDispatch(HPS hps, ULONG ulFnCode,
                  const void *pArgs, ULONG cbArgs,
                  ULONG *pulResult)
{
    struct FPM_GPI_CALL call;
    struct FPM_RESP_HEADER resp;
    LONG len;
    APIRET rc;

    log("[PMGPI] Transport IN : FnCode=0x%x, HPS=%p, cbArgs=%lu\n", ulFnCode, (void*)hps, (unsigned long)cbArgs);

    /* 1. Form the call header */
    call.ulFnCode = ulFnCode;
    call.cbArgs = cbArgs;
    call.ulFlags = 0;

    /* 2. Send the F_CMD_GPI_DISPATCH command with HPS */
    rc = PmcPostCommand(F_CMD_GPI_DISPATCH, (ULONG)hps);
    if (rc) {
        log("[PMGPI] Transport ERR: PmcPostCommand failed with %d\n", (int)rc);
        return rc;
    }

    /* 3. Send the FPM_GPI_CALL */
    rc = PmcSendData(&call, sizeof(call));
    if (rc) {
        log("[PMGPI] Transport ERR: PmcSendData(call) failed with %d\n", (int)rc);
        return rc;
    }

    /* 4. Send the arguments (contents, not a pointer!) */
    if (cbArgs > 0 && pArgs != NULL) {
        rc = PmcSendData((PVOID)pArgs, cbArgs);
        if (rc) {
            log("[PMGPI] Transport ERR: PmcSendData(args) failed with %d\n", (int)rc);
            return rc;
        }
    }

    /* 5. Read the FPM_RESP_HEADER */
    len = 0;
    rc = PmcGetData(&resp, &len, sizeof(resp));
    if (rc || len != sizeof(resp)) {
        log("[PMGPI] Transport ERR: PmcGetData(header) failed rc=%d, len=%d\n", (int)rc, (int)len);
        return (rc != 0) ? rc : 1;
    }

    /* 6. Read the response payload (for Query functions returning data) */
    if (resp.cbPayload > 0 && pArgs != NULL && cbArgs >= resp.cbPayload) {
        len = 0;
        rc = PmcGetData((PVOID)pArgs, &len, resp.cbPayload);
        if (rc) {
            log("[PMGPI] Transport ERR: PmcGetData(payload) failed with %d\n", (int)rc);
            return rc;
        }
    }

    log("[PMGPI] Transport OUT: Status=0x%x, Result=0x%x\n", resp.ulStatus, resp.ulResult);

    *pulResult = resp.ulResult;
    return (resp.ulStatus != 0) ? resp.ulStatus : 0;
}
