/*!
 * @file gpi_entry.c
 * @brief Client-side dispatcher.
 */
#define INCL_GPI
#include "gpi_platform.h"
#include "gpi_entry.h"
#include "gpi_wire.h"
#include "pmclient.h"

/* Код команды для GPI диспетчера (должен совпадать с сервером) */
#ifndef F_CMD_GPI_DISPATCH
#define F_CMD_GPI_DISPATCH 0x0100
#endif

/* Структура запроса (из вашей спеки) */
struct FPM_GPI_CALL {
    ULONG   ulFnCode;    /* N_GpiLine, N_GpiSetColor, ... */
    ULONG   cbArgs;      /* размер упакованных аргументов */
    ULONG   ulFlags;     /* зарезервировано */
};

/* Структура ответа (из вашей спеки) */
struct FPM_RESP_HEADER {
    ULONG   ulStatus;    /* 0 = ok, иначе PMERR_* или APIRET */
    ULONG   ulResult;    /* возвращаемое значение (LONG/BOOL/HANDLE) */
    ULONG   cbPayload;   /* размер payload (например, для Query*) */
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

    /* 1. Формируем заголовок вызова */
    call.ulFnCode = ulFnCode;
    call.cbArgs = cbArgs;
    call.ulFlags = 0;

    /* 2. Отправляем команду F_CMD_GPI_DISPATCH с HPS */
    rc = PmcPostCommand(F_CMD_GPI_DISPATCH, (ULONG)hps);
    if (rc) return rc;

    /* 3. Отправляем FPM_GPI_CALL */
    rc = PmcSendData(&call, sizeof(call));
    if (rc) return rc;

    /* 4. Отправляем аргументы (содержимое, а не указатель!) */
    if (cbArgs > 0 && pArgs != NULL) {
        rc = PmcSendData((PVOID)pArgs, cbArgs);
        if (rc) return rc;
    }

    /* 5. Читаем FPM_RESP_HEADER */
    len = 0;
    rc = PmcGetData(&resp, &len, sizeof(resp));
    if (rc || len != sizeof(resp)) {
        return (rc != 0) ? rc : 1;
    }

    /* 6. Читаем payload ответа (для функций Query, возвращающих данные) */
    if (resp.cbPayload > 0 && pArgs != NULL && cbArgs >= resp.cbPayload) {
        len = 0;
        rc = PmcGetData((PVOID)pArgs, &len, resp.cbPayload);
        if (rc) return rc;
    }

    *pulResult = resp.ulResult;
    return (resp.ulStatus != 0) ? resp.ulStatus : 0;
}
