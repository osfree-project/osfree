/*!
 * @file dispatch_win_stubs.cpp
 * @brief Empty placeholders for Win* domains not yet implemented.
 *
 * Each stub is a private method of Fs_Server with the uniform
 * dispatcher signature. As a domain reaches its phase (4, 5, 6, 7),
 * the stub moves out of this file into dispatch_win_<domain>.cpp and
 * gains its own body.
 *
 * All stubs ignore pRecvobj / fnCode / data / pArgs / cbArgs and
 * return 0 with *pcbPayload = 0.
 */
#include "Fs_server.hpp"

/*!
 * @def WIN_DOMAIN_STUB
 * @brief Define one empty Fs_Server::DispatchWin<Domain> stub.
 * @param [in] cls   Owning class (always Fs_Server).
 * @param [in] name  Method name.
 */
#define WIN_DOMAIN_STUB(cls, name) \
    ULONG Fs_Server::name(void *pRecvobj, ULONG fnCode, ULONG data, \
                          PBYTE pArgs, ULONG cbArgs, PULONG pcbPayload) \
    { \
        (void)pRecvobj; (void)fnCode; (void)data; \
        (void)pArgs; (void)cbArgs; \
        if (pcbPayload) *pcbPayload = 0; \
        return 0; \
    }

/*!
 * @brief DispatchWinMq stub.
 * @param [in]  pRecvobj   Unused.
 * @param [in]  fnCode     Unused.
 * @param [in]  data       Unused.
 * @param [in]  pArgs      Unused.
 * @param [in]  cbArgs     Unused.
 * @param [out] pcbPayload Always 0.
 * @return Always 0.
 * @retval 0 Domain not implemented yet.
 */
WIN_DOMAIN_STUB(Fs_Server, DispatchWinMq)

/*!
 * @brief DispatchWinWnd stub.
 * @param [in]  pRecvobj   Unused.
 * @param [in]  fnCode     Unused.
 * @param [in]  data       Unused.
 * @param [in]  pArgs      Unused.
 * @param [in]  cbArgs     Unused.
 * @param [out] pcbPayload Always 0.
 * @return Always 0.
 * @retval 0 Domain not implemented yet.
 */
WIN_DOMAIN_STUB(Fs_Server, DispatchWinWnd)

/*!
 * @brief DispatchWinPs stub.
 * @param [in]  pRecvobj   Unused.
 * @param [in]  fnCode     Unused.
 * @param [in]  data       Unused.
 * @param [in]  pArgs      Unused.
 * @param [in]  cbArgs     Unused.
 * @param [out] pcbPayload Always 0.
 * @return Always 0.
 * @retval 0 Domain not implemented yet.
 */
WIN_DOMAIN_STUB(Fs_Server, DispatchWinPs)

/*!
 * @brief DispatchWinCls stub.
 * @param [in]  pRecvobj   Unused.
 * @param [in]  fnCode     Unused.
 * @param [in]  data       Unused.
 * @param [in]  pArgs      Unused.
 * @param [in]  cbArgs     Unused.
 * @param [out] pcbPayload Always 0.
 * @return Always 0.
 * @retval 0 Domain not implemented yet.
 */
WIN_DOMAIN_STUB(Fs_Server, DispatchWinCls)

/*!
 * @brief DispatchWinDlg stub.
 * @param [in]  pRecvobj   Unused.
 * @param [in]  fnCode     Unused.
 * @param [in]  data       Unused.
 * @param [in]  pArgs      Unused.
 * @param [in]  cbArgs     Unused.
 * @param [out] pcbPayload Always 0.
 * @return Always 0.
 * @retval 0 Domain not implemented yet.
 */
WIN_DOMAIN_STUB(Fs_Server, DispatchWinDlg)

/*!
 * @brief DispatchWinPtr stub.
 * @param [in]  pRecvobj   Unused.
 * @param [in]  fnCode     Unused.
 * @param [in]  data       Unused.
 * @param [in]  pArgs      Unused.
 * @param [in]  cbArgs     Unused.
 * @param [out] pcbPayload Always 0.
 * @return Always 0.
 * @retval 0 Domain not implemented yet.
 */
WIN_DOMAIN_STUB(Fs_Server, DispatchWinPtr)

/*!
 * @brief DispatchWinClip stub.
 * @param [in]  pRecvobj   Unused.
 * @param [in]  fnCode     Unused.
 * @param [in]  data       Unused.
 * @param [in]  pArgs      Unused.
 * @param [in]  cbArgs     Unused.
 * @param [out] pcbPayload Always 0.
 * @return Always 0.
 * @retval 0 Domain not implemented yet.
 */
WIN_DOMAIN_STUB(Fs_Server, DispatchWinClip)

/*!
 * @brief DispatchWinAtom stub.
 * @param [in]  pRecvobj   Unused.
 * @param [in]  fnCode     Unused.
 * @param [in]  data       Unused.
 * @param [in]  pArgs      Unused.
 * @param [in]  cbArgs     Unused.
 * @param [out] pcbPayload Always 0.
 * @return Always 0.
 * @retval 0 Domain not implemented yet.
 */
WIN_DOMAIN_STUB(Fs_Server, DispatchWinAtom)

/*!
 * @brief DispatchWinStr stub.
 * @param [in]  pRecvobj   Unused.
 * @param [in]  fnCode     Unused.
 * @param [in]  data       Unused.
 * @param [in]  pArgs      Unused.
 * @param [in]  cbArgs     Unused.
 * @param [out] pcbPayload Always 0.
 * @return Always 0.
 * @retval 0 Domain not implemented yet.
 */
WIN_DOMAIN_STUB(Fs_Server, DispatchWinStr)

/*!
 * @brief DispatchWinErr stub.
 * @param [in]  pRecvobj   Unused.
 * @param [in]  fnCode     Unused.
 * @param [in]  data       Unused.
 * @param [in]  pArgs      Unused.
 * @param [in]  cbArgs     Unused.
 * @param [out] pcbPayload Always 0.
 * @return Always 0.
 * @retval 0 Domain not implemented yet.
 */
WIN_DOMAIN_STUB(Fs_Server, DispatchWinErr)

/*!
 * @brief DispatchWinSys stub.
 * @param [in]  pRecvobj   Unused.
 * @param [in]  fnCode     Unused.
 * @param [in]  data       Unused.
 * @param [in]  pArgs      Unused.
 * @param [in]  cbArgs     Unused.
 * @param [out] pcbPayload Always 0.
 * @return Always 0.
 * @retval 0 Domain not implemented yet.
 */
WIN_DOMAIN_STUB(Fs_Server, DispatchWinSys)

/*!
 * @brief DispatchWinRect stub.
 * @param [in]  pRecvobj   Unused.
 * @param [in]  fnCode     Unused.
 * @param [in]  data       Unused.
 * @param [in]  pArgs      Unused.
 * @param [in]  cbArgs     Unused.
 * @param [out] pcbPayload Always 0.
 * @return Always 0.
 * @retval 0 Domain not implemented yet.
 */
WIN_DOMAIN_STUB(Fs_Server, DispatchWinRect)

/*!
 * @brief DispatchWinInp stub.
 * @param [in]  pRecvobj   Unused.
 * @param [in]  fnCode     Unused.
 * @param [in]  data       Unused.
 * @param [in]  pArgs      Unused.
 * @param [in]  cbArgs     Unused.
 * @param [out] pcbPayload Always 0.
 * @return Always 0.
 * @retval 0 Domain not implemented yet.
 */
WIN_DOMAIN_STUB(Fs_Server, DispatchWinInp)

/*!
 * @brief DispatchWinDraw stub.
 * @param [in]  pRecvobj   Unused.
 * @param [in]  fnCode     Unused.
 * @param [in]  data       Unused.
 * @param [in]  pArgs      Unused.
 * @param [in]  cbArgs     Unused.
 * @param [out] pcbPayload Always 0.
 * @return Always 0.
 * @retval 0 Domain not implemented yet.
 */
WIN_DOMAIN_STUB(Fs_Server, DispatchWinDraw)

/*!
 * @brief DispatchWinFrame stub.
 * @param [in]  pRecvobj   Unused.
 * @param [in]  fnCode     Unused.
 * @param [in]  data       Unused.
 * @param [in]  pArgs      Unused.
 * @param [in]  cbArgs     Unused.
 * @param [out] pcbPayload Always 0.
 * @return Always 0.
 * @retval 0 Domain not implemented yet.
 */
WIN_DOMAIN_STUB(Fs_Server, DispatchWinFrame)

/*!
 * @brief DispatchWinSync stub.
 * @param [in]  pRecvobj   Unused.
 * @param [in]  fnCode     Unused.
 * @param [in]  data       Unused.
 * @param [in]  pArgs      Unused.
 * @param [in]  cbArgs     Unused.
 * @param [out] pcbPayload Always 0.
 * @return Always 0.
 * @retval 0 Domain not implemented yet.
 */
WIN_DOMAIN_STUB(Fs_Server, DispatchWinSync)

/*!
 * @brief DispatchWinMisc stub.
 * @param [in]  pRecvobj   Unused.
 * @param [in]  fnCode     Unused.
 * @param [in]  data       Unused.
 * @param [in]  pArgs      Unused.
 * @param [in]  cbArgs     Unused.
 * @param [out] pcbPayload Always 0.
 * @return Always 0.
 * @retval 0 Domain not implemented yet.
 */
WIN_DOMAIN_STUB(Fs_Server, DispatchWinMisc)

/*!
 * @brief DispatchWin32 stub.
 * @param [in]  pRecvobj   Unused.
 * @param [in]  fnCode     Unused.
 * @param [in]  data       Unused.
 * @param [in]  pArgs      Unused.
 * @param [in]  cbArgs     Unused.
 * @param [out] pcbPayload Always 0.
 * @return Always 0.
 * @retval 0 Domain not implemented yet.
 */
WIN_DOMAIN_STUB(Fs_Server, DispatchWin32)

/*!
 * @brief DispatchWinDax stub.
 * @param [in]  pRecvobj   Unused.
 * @param [in]  fnCode     Unused.
 * @param [in]  data       Unused.
 * @param [in]  pArgs      Unused.
 * @param [in]  cbArgs     Unused.
 * @param [out] pcbPayload Always 0.
 * @return Always 0.
 * @retval 0 Domain not implemented yet.
 */
WIN_DOMAIN_STUB(Fs_Server, DispatchWinDax)

#undef WIN_DOMAIN_STUB
