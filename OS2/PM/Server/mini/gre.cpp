/*!
 * @file gre.cpp
 * @brief Minimal PMGRE stubs for the mini server.
 *
 * The GOCA parser (goca_parser.cpp) reaches PMGRE via the Gre* macros
 * declared in <pmddim.h>, which expand to Gre32EntryN(...) calls. The
 * mini server does not link against PMGRE, so every Gre32EntryN that
 * the parser actually uses must be defined here to satisfy the linker.
 *
 * Each stub logs the NGre* function code (last argument of the macro
 * expansion) and returns GPI_ERROR. This keeps the mini server building
 * and makes GRE calls visible in the log without wiring the real GRE
 * dispatcher.
 *
 * Arity actually used by goca_parser.cpp:
 *   Gre32Entry4  — GetCurrentPosition, SetCurrentPosition, FullArcBoth,
 *                  BoxBoth/Boundary/Interior, BeginArea, EndArea,
 *                  SetArcParameters
 *   Gre32Entry5  — PolyLine, PolyMarker, PolySpline, PolyFillet,
 *                  CharString
 *   Gre32Entry6  — GetAttributes
 *   Gre32Entry7  — SetAttributes, PartialArc
 *   Gre32Entry10 — CharStringPos
 *
 * Gre32Entry2, 3, 8, 9 are provided for completeness; goca_parser.cpp
 * does not reference them today. If another object file in the build
 * already defines any of these, delete that stub here to avoid a
 * duplicate-symbol error at link time.
 */

#include "F_platform.h"

/* Same logger as dispatch.cpp. */
extern void log(const char *fmt, ...);

extern "C" {

/*!
 * @brief Gre32Entry2 stub.
 * @param [in] a       First 32-bit argument.
 * @param [in] fnCode  NGre* function code (last arg of the macro).
 * @return GPI_ERROR.
 */
ULONG APIENTRY Gre32Entry2(ULONG a, ULONG fnCode)
{
    log("[GRE] IN  : Gre32Entry2(FnCode=0x%x, a=%p)\n", (unsigned)fnCode, (void*)a);
    log("[GRE] OUT : Gre32Entry2 -> 0x%x\n", (unsigned)GPI_ERROR);
    return (ULONG)GPI_ERROR;
}

/*!
 * @brief Gre32Entry3 stub.
 * @param [in] a       First argument.
 * @param [in] b       Second argument.
 * @param [in] fnCode  NGre* function code.
 * @return GPI_ERROR.
 */
ULONG APIENTRY Gre32Entry3(ULONG a, ULONG b, ULONG fnCode)
{
    log("[GRE] IN  : Gre32Entry3(FnCode=0x%x, a=%p, b=%p)\n", (unsigned)fnCode, (void*)a, (void*)b);
    log("[GRE] OUT : Gre32Entry3 -> 0x%x\n", (unsigned)GPI_ERROR);
    return (ULONG)GPI_ERROR;
}

/*!
 * @brief Gre32Entry4 stub.
 * @param [in] a       First argument.
 * @param [in] b       Second argument.
 * @param [in] c       Third argument.
 * @param [in] fnCode  NGre* function code.
 * @return GPI_ERROR.
 */
ULONG APIENTRY Gre32Entry4(ULONG a, ULONG b, ULONG c, ULONG fnCode)
{
    log("[GRE] IN  : Gre32Entry4(FnCode=0x%x, a=%p, b=%p, c=%p)\n", (unsigned)fnCode, (void*)a, (void*)b, (void*)c);
    log("[GRE] OUT : Gre32Entry4 -> 0x%x\n", (unsigned)GPI_ERROR);
    return (ULONG)GPI_ERROR;
}

/*!
 * @brief Gre32Entry5 stub.
 * @param [in] a       First argument.
 * @param [in] b       Second argument.
 * @param [in] c       Third argument.
 * @param [in] d       Fourth argument.
 * @param [in] fnCode  NGre* function code.
 * @return GPI_ERROR.
 */
ULONG APIENTRY Gre32Entry5(ULONG a, ULONG b, ULONG c, ULONG d, ULONG fnCode)
{
    log("[GRE] IN  : Gre32Entry5(FnCode=0x%x, a=%p, b=%p, c=%p, d=%p)\n", (unsigned)fnCode, (void*)a, (void*)b, (void*)c, (void*)d);
    log("[GRE] OUT : Gre32Entry5 -> 0x%x\n", (unsigned)GPI_ERROR);
    return (ULONG)GPI_ERROR;
}

/*!
 * @brief Gre32Entry6 stub.
 * @param [in] a       First argument.
 * @param [in] b       Second argument.
 * @param [in] c       Third argument.
 * @param [in] d       Fourth argument.
 * @param [in] e       Fifth argument.
 * @param [in] fnCode  NGre* function code.
 * @return GPI_ERROR.
 */
ULONG APIENTRY Gre32Entry6(ULONG a, ULONG b, ULONG c, ULONG d, ULONG e, ULONG fnCode)
{
    log("[GRE] IN  : Gre32Entry6(FnCode=0x%x, a=%p, b=%p, c=%p, d=%p, e=%p)\n", (unsigned)fnCode, (void*)a, (void*)b, (void*)c, (void*)d, (void*)e);
    log("[GRE] OUT : Gre32Entry6 -> 0x%x\n", (unsigned)GPI_ERROR);
    return (ULONG)GPI_ERROR;
}

/*!
 * @brief Gre32Entry7 stub.
 * @param [in] a       First argument.
 * @param [in] b       Second argument.
 * @param [in] c       Third argument.
 * @param [in] d       Fourth argument.
 * @param [in] e       Fifth argument.
 * @param [in] f       Sixth argument.
 * @param [in] fnCode  NGre* function code.
 * @return GPI_ERROR.
 */
ULONG APIENTRY Gre32Entry7(ULONG a, ULONG b, ULONG c, ULONG d, ULONG e, ULONG f, ULONG fnCode)
{
    log("[GRE] IN  : Gre32Entry7(FnCode=0x%x, a=%p, b=%p, c=%p, d=%p, e=%p, f=%p)\n", (unsigned)fnCode, (void*)a, (void*)b, (void*)c, (void*)d, (void*)e, (void*)f);
    log("[GRE] OUT : Gre32Entry7 -> 0x%x\n", (unsigned)GPI_ERROR);
    return (ULONG)GPI_ERROR;
}

/*!
 * @brief Gre32Entry8 stub.
 * @param [in] a       First argument.
 * @param [in] b       Second argument.
 * @param [in] c       Third argument.
 * @param [in] d       Fourth argument.
 * @param [in] e       Fifth argument.
 * @param [in] f       Sixth argument.
 * @param [in] g       Seventh argument.
 * @param [in] fnCode  NGre* function code.
 * @return GPI_ERROR.
 */
ULONG APIENTRY Gre32Entry8(ULONG a, ULONG b, ULONG c, ULONG d, ULONG e, ULONG f, ULONG g, ULONG fnCode)
{
    log("[GRE] IN  : Gre32Entry8(FnCode=0x%x, a=%p, b=%p, c=%p, d=%p, e=%p, f=%p, g=%p)\n", (unsigned)fnCode, (void*)a, (void*)b, (void*)c, (void*)d, (void*)e, (void*)f, (void*)g);
    log("[GRE] OUT : Gre32Entry8 -> 0x%x\n", (unsigned)GPI_ERROR);
    return (ULONG)GPI_ERROR;
}

/*!
 * @brief Gre32Entry9 stub.
 * @param [in] a       First argument.
 * @param [in] b       Second argument.
 * @param [in] c       Third argument.
 * @param [in] d       Fourth argument.
 * @param [in] e       Fifth argument.
 * @param [in] f       Sixth argument.
 * @param [in] g       Seventh argument.
 * @param [in] h       Eighth argument.
 * @param [in] fnCode  NGre* function code.
 * @return GPI_ERROR.
 */
ULONG APIENTRY Gre32Entry9(ULONG a, ULONG b, ULONG c, ULONG d, ULONG e, ULONG f, ULONG g, ULONG h, ULONG fnCode)
{
    log("[GRE] IN  : Gre32Entry9(FnCode=0x%x, a=%p, b=%p, c=%p, d=%p, e=%p, f=%p, g=%p, h=%p)\n", (unsigned)fnCode, (void*)a, (void*)b, (void*)c, (void*)d, (void*)e, (void*)f, (void*)g, (void*)h);
    log("[GRE] OUT : Gre32Entry9 -> 0x%x\n", (unsigned)GPI_ERROR);
    return (ULONG)GPI_ERROR;
}

/*!
 * @brief Gre32Entry10 stub.
 * @param [in] a       First argument.
 * @param [in] b       Second argument.
 * @param [in] c       Third argument.
 * @param [in] d       Fourth argument.
 * @param [in] e       Fifth argument.
 * @param [in] f       Sixth argument.
 * @param [in] g       Seventh argument.
 * @param [in] h       Eighth argument.
 * @param [in] i       Ninth argument.
 * @param [in] fnCode  NGre* function code.
 * @return GPI_ERROR.
 */
ULONG APIENTRY Gre32Entry10(ULONG a, ULONG b, ULONG c, ULONG d, ULONG e, ULONG f, ULONG g, ULONG h, ULONG i, ULONG fnCode)
{
    log("[GRE] IN  : Gre32Entry10(FnCode=0x%x, a=%p, b=%p, c=%p, d=%p, e=%p, f=%p, g=%p, h=%p, i=%p)\n", (unsigned)fnCode, (void*)a, (void*)b, (void*)c, (void*)d, (void*)e, (void*)f, (void*)g, (void*)h, (void*)i);
    log("[GRE] OUT : Gre32Entry10 -> 0x%x\n", (unsigned)GPI_ERROR);
    return (ULONG)GPI_ERROR;
}

} /* extern "C" */
