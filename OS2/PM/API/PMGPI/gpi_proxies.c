/*!
 * @file gpi_proxies.c
 * @brief Auto-generated client-side PMGPI proxies.
 */
#define INCL_GPI
#include "gpi_platform.h"
#include "gpi_wire.h"
#include "gpi_entry.h"


/*!
 * @brief Client proxy for GpiDestroyPS.
 * @param [in] hps HPS.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiDestroyPS(HPS hps)
{
    ULONG result;
    int   rc;

    rc = GpiDispatch(hps, NGpiDestroyPS,
                      NULL, 0, &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiAssociate.
 * @param [in] hps HPS.
 * @param [in] hdc HDC.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiAssociate(HPS hps, HDC hdc)
{
    struct {
        HDC hdc;
    } args;
    ULONG result;
    int   rc;

    args.hdc = hdc;
    rc = GpiDispatch(hps, NGpiAssociate,
                      &args, sizeof(args), &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiRestorePS.
 * @param [in] hps HPS.
 * @param [in] lPSid LONG.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiRestorePS(HPS hps, LONG lPSid)
{
    struct {
        LONG lPSid;
    } args;
    ULONG result;
    int   rc;

    args.lPSid = lPSid;
    rc = GpiDispatch(hps, NGpiRestorePS,
                      &args, sizeof(args), &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiSavePS.
 * @param [in] hps HPS.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiSavePS(HPS hps)
{
    ULONG result;
    int   rc;

    rc = GpiDispatch(hps, NGpiSavePS,
                      NULL, 0, &result);
    if (rc != 0) return (LONG)GPI_ERROR;
    return (LONG)result;
}

/*!
 * @brief Client proxy for GpiErase.
 * @param [in] hps HPS.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiErase(HPS hps)
{
    ULONG result;
    int   rc;

    rc = GpiDispatch(hps, NGpiErase,
                      NULL, 0, &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiQueryDevice.
 * @param [in] hps HPS.
 * @return Handle, or GPI_ERROR.
 */
HDC APIENTRY GpiQueryDevice(HPS hps)
{
    ULONG result;
    int   rc;

    rc = GpiDispatch(hps, NGpiQueryDevice,
                      NULL, 0, &result);
    if (rc != 0) return (HDC)GPI_ERROR;
    return (HDC)result;
}

/*!
 * @brief Client proxy for GpiResetPS.
 * @param [in] hps HPS.
 * @param [in] flOptions ULONG.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiResetPS(HPS hps, ULONG flOptions)
{
    struct {
        ULONG flOptions;
    } args;
    ULONG result;
    int   rc;

    args.flOptions = flOptions;
    rc = GpiDispatch(hps, NGpiResetPS,
                      &args, sizeof(args), &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiSetPS.
 * @param [in] hps HPS.
 * @param [in] psizlsize PSIZEL.
 * @param [in] flOptions ULONG.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiSetPS(HPS hps, PSIZEL psizlsize, ULONG flOptions)
{
    struct {
        SIZEL psizlsize;
        ULONG flOptions;
    } args;
    ULONG result;
    int   rc;

    args.psizlsize = *psizlsize;
    args.flOptions = flOptions;
    rc = GpiDispatch(hps, NGpiSetPS,
                      &args, sizeof(args), &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiQueryDrawControl.
 * @param [in] hps HPS.
 * @param [in] lControl LONG.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiQueryDrawControl(HPS hps, LONG lControl)
{
    struct {
        LONG lControl;
    } args;
    ULONG result;
    int   rc;

    args.lControl = lControl;
    rc = GpiDispatch(hps, NGpiQueryDrawControl,
                      &args, sizeof(args), &result);
    if (rc != 0) return (LONG)GPI_ERROR;
    return (LONG)result;
}

/*!
 * @brief Client proxy for GpiSetDrawControl.
 * @param [in] hps HPS.
 * @param [in] lControl LONG.
 * @param [in] lValue LONG.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiSetDrawControl(HPS hps, LONG lControl, LONG lValue)
{
    struct {
        LONG lControl;
        LONG lValue;
    } args;
    ULONG result;
    int   rc;

    args.lControl = lControl;
    args.lValue = lValue;
    rc = GpiDispatch(hps, NGpiSetDrawControl,
                      &args, sizeof(args), &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiQueryDrawingMode.
 * @param [in] hps HPS.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiQueryDrawingMode(HPS hps)
{
    ULONG result;
    int   rc;

    rc = GpiDispatch(hps, NGpiQueryDrawingMode,
                      NULL, 0, &result);
    if (rc != 0) return (LONG)GPI_ERROR;
    return (LONG)result;
}

/*!
 * @brief Client proxy for GpiSetDrawingMode.
 * @param [in] hps HPS.
 * @param [in] lMode LONG.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiSetDrawingMode(HPS hps, LONG lMode)
{
    struct {
        LONG lMode;
    } args;
    ULONG result;
    int   rc;

    args.lMode = lMode;
    rc = GpiDispatch(hps, NGpiSetDrawingMode,
                      &args, sizeof(args), &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiQueryStopDraw.
 * @param [in] hps HPS.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiQueryStopDraw(HPS hps)
{
    ULONG result;
    int   rc;

    rc = GpiDispatch(hps, NGpiQueryStopDraw,
                      NULL, 0, &result);
    if (rc != 0) return (LONG)GPI_ERROR;
    return (LONG)result;
}

/*!
 * @brief Client proxy for GpiSetStopDraw.
 * @param [in] hps HPS.
 * @param [in] lValue LONG.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiSetStopDraw(HPS hps, LONG lValue)
{
    struct {
        LONG lValue;
    } args;
    ULONG result;
    int   rc;

    args.lValue = lValue;
    rc = GpiDispatch(hps, NGpiSetStopDraw,
                      &args, sizeof(args), &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiSetTag.
 * @param [in] hps HPS.
 * @param [in] lTag LONG.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiSetTag(HPS hps, LONG lTag)
{
    struct {
        LONG lTag;
    } args;
    ULONG result;
    int   rc;

    args.lTag = lTag;
    rc = GpiDispatch(hps, NGpiSetTag,
                      &args, sizeof(args), &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiSetPickApertureSize.
 * @param [in] hps HPS.
 * @param [in] lOptions LONG.
 * @param [in] psizlSize PSIZEL.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiSetPickApertureSize(HPS hps, LONG lOptions, PSIZEL psizlSize)
{
    struct {
        LONG lOptions;
        SIZEL psizlSize;
    } args;
    ULONG result;
    int   rc;

    args.lOptions = lOptions;
    args.psizlSize = *psizlSize;
    rc = GpiDispatch(hps, NGpiSetPickApertureSize,
                      &args, sizeof(args), &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiSetPickAperturePosition.
 * @param [in] hps HPS.
 * @param [in] pptlPick PPOINTL.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiSetPickAperturePosition(HPS hps, PPOINTL pptlPick)
{
    struct {
        POINTL pptlPick;
    } args;
    ULONG result;
    int   rc;

    args.pptlPick = *pptlPick;
    rc = GpiDispatch(hps, NGpiSetPickAperturePosition,
                      &args, sizeof(args), &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiResetBoundaryData.
 * @param [in] hps HPS.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiResetBoundaryData(HPS hps)
{
    ULONG result;
    int   rc;

    rc = GpiDispatch(hps, NGpiResetBoundaryData,
                      NULL, 0, &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiOpenSegment.
 * @param [in] hps HPS.
 * @param [in] lSegment LONG.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiOpenSegment(HPS hps, LONG lSegment)
{
    struct {
        LONG lSegment;
    } args;
    ULONG result;
    int   rc;

    args.lSegment = lSegment;
    rc = GpiDispatch(hps, NGpiOpenSegment,
                      &args, sizeof(args), &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiCloseSegment.
 * @param [in] hps HPS.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiCloseSegment(HPS hps)
{
    ULONG result;
    int   rc;

    rc = GpiDispatch(hps, NGpiCloseSegment,
                      NULL, 0, &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiDeleteSegment.
 * @param [in] hps HPS.
 * @param [in] lSegid LONG.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiDeleteSegment(HPS hps, LONG lSegid)
{
    struct {
        LONG lSegid;
    } args;
    ULONG result;
    int   rc;

    args.lSegid = lSegid;
    rc = GpiDispatch(hps, NGpiDeleteSegment,
                      &args, sizeof(args), &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiQueryInitialSegmentAttrs.
 * @param [in] hps HPS.
 * @param [in] lAttribute LONG.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiQueryInitialSegmentAttrs(HPS hps, LONG lAttribute)
{
    struct {
        LONG lAttribute;
    } args;
    ULONG result;
    int   rc;

    args.lAttribute = lAttribute;
    rc = GpiDispatch(hps, NGpiQueryInitialSegmentAttrs,
                      &args, sizeof(args), &result);
    if (rc != 0) return (LONG)GPI_ERROR;
    return (LONG)result;
}

/*!
 * @brief Client proxy for GpiSetInitialSegmentAttrs.
 * @param [in] hps HPS.
 * @param [in] lAttribute LONG.
 * @param [in] lValue LONG.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiSetInitialSegmentAttrs(HPS hps, LONG lAttribute, LONG lValue)
{
    struct {
        LONG lAttribute;
        LONG lValue;
    } args;
    ULONG result;
    int   rc;

    args.lAttribute = lAttribute;
    args.lValue = lValue;
    rc = GpiDispatch(hps, NGpiSetInitialSegmentAttrs,
                      &args, sizeof(args), &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiQuerySegmentAttrs.
 * @param [in] hps HPS.
 * @param [in] lSegid LONG.
 * @param [in] lAttribute LONG.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiQuerySegmentAttrs(HPS hps, LONG lSegid, LONG lAttribute)
{
    struct {
        LONG lSegid;
        LONG lAttribute;
    } args;
    ULONG result;
    int   rc;

    args.lSegid = lSegid;
    args.lAttribute = lAttribute;
    rc = GpiDispatch(hps, NGpiQuerySegmentAttrs,
                      &args, sizeof(args), &result);
    if (rc != 0) return (LONG)GPI_ERROR;
    return (LONG)result;
}

/*!
 * @brief Client proxy for GpiSetSegmentAttrs.
 * @param [in] hps HPS.
 * @param [in] lSegid LONG.
 * @param [in] lAttribute LONG.
 * @param [in] lValue LONG.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiSetSegmentAttrs(HPS hps, LONG lSegid, LONG lAttribute, LONG lValue)
{
    struct {
        LONG lSegid;
        LONG lAttribute;
        LONG lValue;
    } args;
    ULONG result;
    int   rc;

    args.lSegid = lSegid;
    args.lAttribute = lAttribute;
    args.lValue = lValue;
    rc = GpiDispatch(hps, NGpiSetSegmentAttrs,
                      &args, sizeof(args), &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiQuerySegmentPriority.
 * @param [in] hps HPS.
 * @param [in] lRefSegid LONG.
 * @param [in] lOrder LONG.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiQuerySegmentPriority(HPS hps, LONG lRefSegid, LONG lOrder)
{
    struct {
        LONG lRefSegid;
        LONG lOrder;
    } args;
    ULONG result;
    int   rc;

    args.lRefSegid = lRefSegid;
    args.lOrder = lOrder;
    rc = GpiDispatch(hps, NGpiQuerySegmentPriority,
                      &args, sizeof(args), &result);
    if (rc != 0) return (LONG)GPI_ERROR;
    return (LONG)result;
}

/*!
 * @brief Client proxy for GpiSetSegmentPriority.
 * @param [in] hps HPS.
 * @param [in] lSegid LONG.
 * @param [in] lRefSegid LONG.
 * @param [in] lOrder LONG.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiSetSegmentPriority(HPS hps, LONG lSegid, LONG lRefSegid, LONG lOrder)
{
    struct {
        LONG lSegid;
        LONG lRefSegid;
        LONG lOrder;
    } args;
    ULONG result;
    int   rc;

    args.lSegid = lSegid;
    args.lRefSegid = lRefSegid;
    args.lOrder = lOrder;
    rc = GpiDispatch(hps, NGpiSetSegmentPriority,
                      &args, sizeof(args), &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiDeleteSegments.
 * @param [in] hps HPS.
 * @param [in] lFirstSegment LONG.
 * @param [in] lLastSegment LONG.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiDeleteSegments(HPS hps, LONG lFirstSegment, LONG lLastSegment)
{
    struct {
        LONG lFirstSegment;
        LONG lLastSegment;
    } args;
    ULONG result;
    int   rc;

    args.lFirstSegment = lFirstSegment;
    args.lLastSegment = lLastSegment;
    rc = GpiDispatch(hps, NGpiDeleteSegments,
                      &args, sizeof(args), &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiDrawChain.
 * @param [in] hps HPS.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiDrawChain(HPS hps)
{
    ULONG result;
    int   rc;

    rc = GpiDispatch(hps, NGpiDrawChain,
                      NULL, 0, &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiDrawFrom.
 * @param [in] hps HPS.
 * @param [in] lFirstSegment LONG.
 * @param [in] lLastSegment LONG.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiDrawFrom(HPS hps, LONG lFirstSegment, LONG lLastSegment)
{
    struct {
        LONG lFirstSegment;
        LONG lLastSegment;
    } args;
    ULONG result;
    int   rc;

    args.lFirstSegment = lFirstSegment;
    args.lLastSegment = lLastSegment;
    rc = GpiDispatch(hps, NGpiDrawFrom,
                      &args, sizeof(args), &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiDrawSegment.
 * @param [in] hps HPS.
 * @param [in] lSegment LONG.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiDrawSegment(HPS hps, LONG lSegment)
{
    struct {
        LONG lSegment;
    } args;
    ULONG result;
    int   rc;

    args.lSegment = lSegment;
    rc = GpiDispatch(hps, NGpiDrawSegment,
                      &args, sizeof(args), &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiDrawDynamics.
 * @param [in] hps HPS.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiDrawDynamics(HPS hps)
{
    ULONG result;
    int   rc;

    rc = GpiDispatch(hps, NGpiDrawDynamics,
                      NULL, 0, &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiRemoveDynamics.
 * @param [in] hps HPS.
 * @param [in] lFirstSegid LONG.
 * @param [in] lLastSegid LONG.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiRemoveDynamics(HPS hps, LONG lFirstSegid, LONG lLastSegid)
{
    struct {
        LONG lFirstSegid;
        LONG lLastSegid;
    } args;
    ULONG result;
    int   rc;

    args.lFirstSegid = lFirstSegid;
    args.lLastSegid = lLastSegid;
    rc = GpiDispatch(hps, NGpiRemoveDynamics,
                      &args, sizeof(args), &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiEndElement.
 * @param [in] hps HPS.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiEndElement(HPS hps)
{
    ULONG result;
    int   rc;

    rc = GpiDispatch(hps, NGpiEndElement,
                      NULL, 0, &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiLabel.
 * @param [in] hps HPS.
 * @param [in] lLabel LONG.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiLabel(HPS hps, LONG lLabel)
{
    struct {
        LONG lLabel;
    } args;
    ULONG result;
    int   rc;

    args.lLabel = lLabel;
    rc = GpiDispatch(hps, NGpiLabel,
                      &args, sizeof(args), &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiDeleteElement.
 * @param [in] hps HPS.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiDeleteElement(HPS hps)
{
    ULONG result;
    int   rc;

    rc = GpiDispatch(hps, NGpiDeleteElement,
                      NULL, 0, &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiDeleteElementRange.
 * @param [in] hps HPS.
 * @param [in] lFirstElement LONG.
 * @param [in] lLastElement LONG.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiDeleteElementRange(HPS hps, LONG lFirstElement, LONG lLastElement)
{
    struct {
        LONG lFirstElement;
        LONG lLastElement;
    } args;
    ULONG result;
    int   rc;

    args.lFirstElement = lFirstElement;
    args.lLastElement = lLastElement;
    rc = GpiDispatch(hps, NGpiDeleteElementRange,
                      &args, sizeof(args), &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiDeleteElementsBetweenLabels.
 * @param [in] hps HPS.
 * @param [in] lFirstLabel LONG.
 * @param [in] lLastLabel LONG.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiDeleteElementsBetweenLabels(HPS hps, LONG lFirstLabel, LONG lLastLabel)
{
    struct {
        LONG lFirstLabel;
        LONG lLastLabel;
    } args;
    ULONG result;
    int   rc;

    args.lFirstLabel = lFirstLabel;
    args.lLastLabel = lLastLabel;
    rc = GpiDispatch(hps, NGpiDeleteElementsBetweenLabels,
                      &args, sizeof(args), &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiQueryEditMode.
 * @param [in] hps HPS.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiQueryEditMode(HPS hps)
{
    ULONG result;
    int   rc;

    rc = GpiDispatch(hps, NGpiQueryEditMode,
                      NULL, 0, &result);
    if (rc != 0) return (LONG)GPI_ERROR;
    return (LONG)result;
}

/*!
 * @brief Client proxy for GpiSetEditMode.
 * @param [in] hps HPS.
 * @param [in] lMode LONG.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiSetEditMode(HPS hps, LONG lMode)
{
    struct {
        LONG lMode;
    } args;
    ULONG result;
    int   rc;

    args.lMode = lMode;
    rc = GpiDispatch(hps, NGpiSetEditMode,
                      &args, sizeof(args), &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiQueryElementPointer.
 * @param [in] hps HPS.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiQueryElementPointer(HPS hps)
{
    ULONG result;
    int   rc;

    rc = GpiDispatch(hps, NGpiQueryElementPointer,
                      NULL, 0, &result);
    if (rc != 0) return (LONG)GPI_ERROR;
    return (LONG)result;
}

/*!
 * @brief Client proxy for GpiSetElementPointer.
 * @param [in] hps HPS.
 * @param [in] lElement LONG.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiSetElementPointer(HPS hps, LONG lElement)
{
    struct {
        LONG lElement;
    } args;
    ULONG result;
    int   rc;

    args.lElement = lElement;
    rc = GpiDispatch(hps, NGpiSetElementPointer,
                      &args, sizeof(args), &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiOffsetElementPointer.
 * @param [in] hps HPS.
 * @param [in] loffset LONG.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiOffsetElementPointer(HPS hps, LONG loffset)
{
    struct {
        LONG loffset;
    } args;
    ULONG result;
    int   rc;

    args.loffset = loffset;
    rc = GpiDispatch(hps, NGpiOffsetElementPointer,
                      &args, sizeof(args), &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiSetElementPointerAtLabel.
 * @param [in] hps HPS.
 * @param [in] lLabel LONG.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiSetElementPointerAtLabel(HPS hps, LONG lLabel)
{
    struct {
        LONG lLabel;
    } args;
    ULONG result;
    int   rc;

    args.lLabel = lLabel;
    rc = GpiDispatch(hps, NGpiSetElementPointerAtLabel,
                      &args, sizeof(args), &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiSetPageViewport.
 * @param [in] hps HPS.
 * @param [in] prclViewport PRECTL.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiSetPageViewport(HPS hps, PRECTL prclViewport)
{
    struct {
        RECTL prclViewport;
    } args;
    ULONG result;
    int   rc;

    args.prclViewport = *prclViewport;
    rc = GpiDispatch(hps, NGpiSetPageViewport,
                      &args, sizeof(args), &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiSetGraphicsField.
 * @param [in] hps HPS.
 * @param [in] prclField PRECTL.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiSetGraphicsField(HPS hps, PRECTL prclField)
{
    struct {
        RECTL prclField;
    } args;
    ULONG result;
    int   rc;

    args.prclField = *prclField;
    rc = GpiDispatch(hps, NGpiSetGraphicsField,
                      &args, sizeof(args), &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiSetViewingLimits.
 * @param [in] hps HPS.
 * @param [in] prclLimits PRECTL.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiSetViewingLimits(HPS hps, PRECTL prclLimits)
{
    struct {
        RECTL prclLimits;
    } args;
    ULONG result;
    int   rc;

    args.prclLimits = *prclLimits;
    rc = GpiDispatch(hps, NGpiSetViewingLimits,
                      &args, sizeof(args), &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiBeginPath.
 * @param [in] hps HPS.
 * @param [in] lPath LONG.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiBeginPath(HPS hps, LONG lPath)
{
    struct {
        LONG lPath;
    } args;
    ULONG result;
    int   rc;

    args.lPath = lPath;
    rc = GpiDispatch(hps, NGpiBeginPath,
                      &args, sizeof(args), &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiEndPath.
 * @param [in] hps HPS.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiEndPath(HPS hps)
{
    ULONG result;
    int   rc;

    rc = GpiDispatch(hps, NGpiEndPath,
                      NULL, 0, &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiCloseFigure.
 * @param [in] hps HPS.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiCloseFigure(HPS hps)
{
    ULONG result;
    int   rc;

    rc = GpiDispatch(hps, NGpiCloseFigure,
                      NULL, 0, &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiModifyPath.
 * @param [in] hps HPS.
 * @param [in] lPath LONG.
 * @param [in] lMode LONG.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiModifyPath(HPS hps, LONG lPath, LONG lMode)
{
    struct {
        LONG lPath;
        LONG lMode;
    } args;
    ULONG result;
    int   rc;

    args.lPath = lPath;
    args.lMode = lMode;
    rc = GpiDispatch(hps, NGpiModifyPath,
                      &args, sizeof(args), &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiFillPath.
 * @param [in] hps HPS.
 * @param [in] lPath LONG.
 * @param [in] lOptions LONG.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiFillPath(HPS hps, LONG lPath, LONG lOptions)
{
    struct {
        LONG lPath;
        LONG lOptions;
    } args;
    ULONG result;
    int   rc;

    args.lPath = lPath;
    args.lOptions = lOptions;
    rc = GpiDispatch(hps, NGpiFillPath,
                      &args, sizeof(args), &result);
    if (rc != 0) return (LONG)GPI_ERROR;
    return (LONG)result;
}

/*!
 * @brief Client proxy for GpiSetClipPath.
 * @param [in] hps HPS.
 * @param [in] lPath LONG.
 * @param [in] lOptions LONG.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiSetClipPath(HPS hps, LONG lPath, LONG lOptions)
{
    struct {
        LONG lPath;
        LONG lOptions;
    } args;
    ULONG result;
    int   rc;

    args.lPath = lPath;
    args.lOptions = lOptions;
    rc = GpiDispatch(hps, NGpiSetClipPath,
                      &args, sizeof(args), &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiOutlinePath.
 * @param [in] hps HPS.
 * @param [in] lPath LONG.
 * @param [in] lOptions LONG.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiOutlinePath(HPS hps, LONG lPath, LONG lOptions)
{
    struct {
        LONG lPath;
        LONG lOptions;
    } args;
    ULONG result;
    int   rc;

    args.lPath = lPath;
    args.lOptions = lOptions;
    rc = GpiDispatch(hps, NGpiOutlinePath,
                      &args, sizeof(args), &result);
    if (rc != 0) return (LONG)GPI_ERROR;
    return (LONG)result;
}

/*!
 * @brief Client proxy for GpiPathToRegion.
 * @param [in] GpiH HPS.
 * @param [in] lPath LONG.
 * @param [in] lOptions LONG.
 * @return Handle, or GPI_ERROR.
 */
HRGN APIENTRY GpiPathToRegion(HPS GpiH, LONG lPath, LONG lOptions)
{
    struct {
        LONG lPath;
        LONG lOptions;
    } args;
    ULONG result;
    int   rc;

    args.lPath = lPath;
    args.lOptions = lOptions;
    rc = GpiDispatch(GpiH, NGpiPathToRegion,
                      &args, sizeof(args), &result);
    if (rc != 0) return (HRGN)GPI_ERROR;
    return (HRGN)result;
}

/*!
 * @brief Client proxy for GpiStrokePath.
 * @param [in] hps HPS.
 * @param [in] lPath LONG.
 * @param [in] flOptions ULONG.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiStrokePath(HPS hps, LONG lPath, ULONG flOptions)
{
    struct {
        LONG lPath;
        ULONG flOptions;
    } args;
    ULONG result;
    int   rc;

    args.lPath = lPath;
    args.flOptions = flOptions;
    rc = GpiDispatch(hps, NGpiStrokePath,
                      &args, sizeof(args), &result);
    if (rc != 0) return (LONG)GPI_ERROR;
    return (LONG)result;
}

/*!
 * @brief Client proxy for GpiQueryNearestColor.
 * @param [in] hps HPS.
 * @param [in] flOptions ULONG.
 * @param [in] lRgbIn LONG.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiQueryNearestColor(HPS hps, ULONG flOptions, LONG lRgbIn)
{
    struct {
        ULONG flOptions;
        LONG lRgbIn;
    } args;
    ULONG result;
    int   rc;

    args.flOptions = flOptions;
    args.lRgbIn = lRgbIn;
    rc = GpiDispatch(hps, NGpiQueryNearestColor,
                      &args, sizeof(args), &result);
    if (rc != 0) return (LONG)GPI_ERROR;
    return (LONG)result;
}

/*!
 * @brief Client proxy for GpiQueryColorIndex.
 * @param [in] hps HPS.
 * @param [in] flOptions ULONG.
 * @param [in] lRgbColor LONG.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiQueryColorIndex(HPS hps, ULONG flOptions, LONG lRgbColor)
{
    struct {
        ULONG flOptions;
        LONG lRgbColor;
    } args;
    ULONG result;
    int   rc;

    args.flOptions = flOptions;
    args.lRgbColor = lRgbColor;
    rc = GpiDispatch(hps, NGpiQueryColorIndex,
                      &args, sizeof(args), &result);
    if (rc != 0) return (LONG)GPI_ERROR;
    return (LONG)result;
}

/*!
 * @brief Client proxy for GpiQueryRGBColor.
 * @param [in] hps HPS.
 * @param [in] flOptions ULONG.
 * @param [in] lColorIndex LONG.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiQueryRGBColor(HPS hps, ULONG flOptions, LONG lColorIndex)
{
    struct {
        ULONG flOptions;
        LONG lColorIndex;
    } args;
    ULONG result;
    int   rc;

    args.flOptions = flOptions;
    args.lColorIndex = lColorIndex;
    rc = GpiDispatch(hps, NGpiQueryRGBColor,
                      &args, sizeof(args), &result);
    if (rc != 0) return (LONG)GPI_ERROR;
    return (LONG)result;
}

/*!
 * @brief Client proxy for GpiSelectPalette.
 * @param [in] hps HPS.
 * @param [in] hpal HPAL.
 * @return Handle, or GPI_ERROR.
 */
HPAL APIENTRY GpiSelectPalette(HPS hps, HPAL hpal)
{
    struct {
        HPAL hpal;
    } args;
    ULONG result;
    int   rc;

    args.hpal = hpal;
    rc = GpiDispatch(hps, NGpiSelectPalette,
                      &args, sizeof(args), &result);
    if (rc != 0) return (HPAL)GPI_ERROR;
    return (HPAL)result;
}

/*!
 * @brief Client proxy for GpiQueryPalette.
 * @param [in] hps HPS.
 * @return Handle, or GPI_ERROR.
 */
HPAL APIENTRY GpiQueryPalette(HPS hps)
{
    ULONG result;
    int   rc;

    rc = GpiDispatch(hps, NGpiQueryPalette,
                      NULL, 0, &result);
    if (rc != 0) return (HPAL)GPI_ERROR;
    return (HPAL)result;
}

/*!
 * @brief Client proxy for GpiSetColor.
 * @param [in] hps HPS.
 * @param [in] lColor LONG.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiSetColor(HPS hps, LONG lColor)
{
    struct {
        LONG lColor;
    } args;
    ULONG result;
    int   rc;

    args.lColor = lColor;
    rc = GpiDispatch(hps, NGpiSetColor,
                      &args, sizeof(args), &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiQueryColor.
 * @param [in] hps HPS.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiQueryColor(HPS hps)
{
    ULONG result;
    int   rc;

    rc = GpiDispatch(hps, NGpiQueryColor,
                      NULL, 0, &result);
    if (rc != 0) return (LONG)GPI_ERROR;
    return (LONG)result;
}

/*!
 * @brief Client proxy for GpiBox.
 * @param [in] hps HPS.
 * @param [in] lControl LONG.
 * @param [in] pptlPoint PPOINTL.
 * @param [in] lHRound LONG.
 * @param [in] lVRound LONG.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiBox(HPS hps, LONG lControl, PPOINTL pptlPoint, LONG lHRound, LONG lVRound)
{
    struct {
        LONG lControl;
        POINTL pptlPoint;
        LONG lHRound;
        LONG lVRound;
    } args;
    ULONG result;
    int   rc;

    args.lControl = lControl;
    args.pptlPoint = *pptlPoint;
    args.lHRound = lHRound;
    args.lVRound = lVRound;
    rc = GpiDispatch(hps, NGpiBox,
                      &args, sizeof(args), &result);
    if (rc != 0) return (LONG)GPI_ERROR;
    return (LONG)result;
}

/*!
 * @brief Client proxy for GpiMove.
 * @param [in] hps HPS.
 * @param [in] pptlPoint PPOINTL.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiMove(HPS hps, PPOINTL pptlPoint)
{
    struct {
        POINTL pptlPoint;
    } args;
    ULONG result;
    int   rc;

    args.pptlPoint = *pptlPoint;
    rc = GpiDispatch(hps, NGpiMove,
                      &args, sizeof(args), &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiLine.
 * @param [in] hps HPS.
 * @param [in] pptlEndPoint PPOINTL.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiLine(HPS hps, PPOINTL pptlEndPoint)
{
    struct {
        POINTL pptlEndPoint;
    } args;
    ULONG result;
    int   rc;

    args.pptlEndPoint = *pptlEndPoint;
    rc = GpiDispatch(hps, NGpiLine,
                      &args, sizeof(args), &result);
    if (rc != 0) return (LONG)GPI_ERROR;
    return (LONG)result;
}

/*!
 * @brief Client proxy for GpiSetPattern.
 * @param [in] hps HPS.
 * @param [in] lPatternSymbol LONG.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiSetPattern(HPS hps, LONG lPatternSymbol)
{
    struct {
        LONG lPatternSymbol;
    } args;
    ULONG result;
    int   rc;

    args.lPatternSymbol = lPatternSymbol;
    rc = GpiDispatch(hps, NGpiSetPattern,
                      &args, sizeof(args), &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiQueryPattern.
 * @param [in] hps HPS.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiQueryPattern(HPS hps)
{
    ULONG result;
    int   rc;

    rc = GpiDispatch(hps, NGpiQueryPattern,
                      NULL, 0, &result);
    if (rc != 0) return (LONG)GPI_ERROR;
    return (LONG)result;
}

/*!
 * @brief Client proxy for GpiBeginArea.
 * @param [in] hps HPS.
 * @param [in] flOptions ULONG.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiBeginArea(HPS hps, ULONG flOptions)
{
    struct {
        ULONG flOptions;
    } args;
    ULONG result;
    int   rc;

    args.flOptions = flOptions;
    rc = GpiDispatch(hps, NGpiBeginArea,
                      &args, sizeof(args), &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiEndArea.
 * @param [in] hps HPS.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiEndArea(HPS hps)
{
    ULONG result;
    int   rc;

    rc = GpiDispatch(hps, NGpiEndArea,
                      NULL, 0, &result);
    if (rc != 0) return (LONG)GPI_ERROR;
    return (LONG)result;
}

/*!
 * @brief Client proxy for GpiSetAttrMode.
 * @param [in] hps HPS.
 * @param [in] lMode LONG.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiSetAttrMode(HPS hps, LONG lMode)
{
    struct {
        LONG lMode;
    } args;
    ULONG result;
    int   rc;

    args.lMode = lMode;
    rc = GpiDispatch(hps, NGpiSetAttrMode,
                      &args, sizeof(args), &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiQueryAttrMode.
 * @param [in] hps HPS.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiQueryAttrMode(HPS hps)
{
    ULONG result;
    int   rc;

    rc = GpiDispatch(hps, NGpiQueryAttrMode,
                      NULL, 0, &result);
    if (rc != 0) return (LONG)GPI_ERROR;
    return (LONG)result;
}

/*!
 * @brief Client proxy for GpiSetBackColor.
 * @param [in] hps HPS.
 * @param [in] lColor LONG.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiSetBackColor(HPS hps, LONG lColor)
{
    struct {
        LONG lColor;
    } args;
    ULONG result;
    int   rc;

    args.lColor = lColor;
    rc = GpiDispatch(hps, NGpiSetBackColor,
                      &args, sizeof(args), &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiQueryBackColor.
 * @param [in] hps HPS.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiQueryBackColor(HPS hps)
{
    ULONG result;
    int   rc;

    rc = GpiDispatch(hps, NGpiQueryBackColor,
                      NULL, 0, &result);
    if (rc != 0) return (LONG)GPI_ERROR;
    return (LONG)result;
}

/*!
 * @brief Client proxy for GpiSetMix.
 * @param [in] hps HPS.
 * @param [in] lMixMode LONG.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiSetMix(HPS hps, LONG lMixMode)
{
    struct {
        LONG lMixMode;
    } args;
    ULONG result;
    int   rc;

    args.lMixMode = lMixMode;
    rc = GpiDispatch(hps, NGpiSetMix,
                      &args, sizeof(args), &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiQueryMix.
 * @param [in] hps HPS.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiQueryMix(HPS hps)
{
    ULONG result;
    int   rc;

    rc = GpiDispatch(hps, NGpiQueryMix,
                      NULL, 0, &result);
    if (rc != 0) return (LONG)GPI_ERROR;
    return (LONG)result;
}

/*!
 * @brief Client proxy for GpiSetBackMix.
 * @param [in] hps HPS.
 * @param [in] lMixMode LONG.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiSetBackMix(HPS hps, LONG lMixMode)
{
    struct {
        LONG lMixMode;
    } args;
    ULONG result;
    int   rc;

    args.lMixMode = lMixMode;
    rc = GpiDispatch(hps, NGpiSetBackMix,
                      &args, sizeof(args), &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiQueryBackMix.
 * @param [in] hps HPS.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiQueryBackMix(HPS hps)
{
    ULONG result;
    int   rc;

    rc = GpiDispatch(hps, NGpiQueryBackMix,
                      NULL, 0, &result);
    if (rc != 0) return (LONG)GPI_ERROR;
    return (LONG)result;
}

/*!
 * @brief Client proxy for GpiSetLineType.
 * @param [in] hps HPS.
 * @param [in] lLineType LONG.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiSetLineType(HPS hps, LONG lLineType)
{
    struct {
        LONG lLineType;
    } args;
    ULONG result;
    int   rc;

    args.lLineType = lLineType;
    rc = GpiDispatch(hps, NGpiSetLineType,
                      &args, sizeof(args), &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiQueryLineType.
 * @param [in] hps HPS.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiQueryLineType(HPS hps)
{
    ULONG result;
    int   rc;

    rc = GpiDispatch(hps, NGpiQueryLineType,
                      NULL, 0, &result);
    if (rc != 0) return (LONG)GPI_ERROR;
    return (LONG)result;
}

/*!
 * @brief Client proxy for GpiSetLineWidth.
 * @param [in] hps HPS.
 * @param [in] fxLineWidth FIXED.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiSetLineWidth(HPS hps, FIXED fxLineWidth)
{
    struct {
        FIXED fxLineWidth;
    } args;
    ULONG result;
    int   rc;

    args.fxLineWidth = fxLineWidth;
    rc = GpiDispatch(hps, NGpiSetLineWidth,
                      &args, sizeof(args), &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiQueryLineWidth.
 * @param [in] hps HPS.
 * @return Fixed value, or GPI_ERROR.
 */
FIXED APIENTRY GpiQueryLineWidth(HPS hps)
{
    ULONG result;
    int   rc;

    rc = GpiDispatch(hps, NGpiQueryLineWidth,
                      NULL, 0, &result);
    if (rc != 0) return (FIXED)GPI_ERROR;
    return (FIXED)result;
}

/*!
 * @brief Client proxy for GpiSetLineWidthGeom.
 * @param [in] hps HPS.
 * @param [in] lLineWidth LONG.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiSetLineWidthGeom(HPS hps, LONG lLineWidth)
{
    struct {
        LONG lLineWidth;
    } args;
    ULONG result;
    int   rc;

    args.lLineWidth = lLineWidth;
    rc = GpiDispatch(hps, NGpiSetLineWidthGeom,
                      &args, sizeof(args), &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiQueryLineWidthGeom.
 * @param [in] hps HPS.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiQueryLineWidthGeom(HPS hps)
{
    ULONG result;
    int   rc;

    rc = GpiDispatch(hps, NGpiQueryLineWidthGeom,
                      NULL, 0, &result);
    if (rc != 0) return (LONG)GPI_ERROR;
    return (LONG)result;
}

/*!
 * @brief Client proxy for GpiSetLineEnd.
 * @param [in] hps HPS.
 * @param [in] lLineEnd LONG.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiSetLineEnd(HPS hps, LONG lLineEnd)
{
    struct {
        LONG lLineEnd;
    } args;
    ULONG result;
    int   rc;

    args.lLineEnd = lLineEnd;
    rc = GpiDispatch(hps, NGpiSetLineEnd,
                      &args, sizeof(args), &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiQueryLineEnd.
 * @param [in] hps HPS.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiQueryLineEnd(HPS hps)
{
    ULONG result;
    int   rc;

    rc = GpiDispatch(hps, NGpiQueryLineEnd,
                      NULL, 0, &result);
    if (rc != 0) return (LONG)GPI_ERROR;
    return (LONG)result;
}

/*!
 * @brief Client proxy for GpiSetLineJoin.
 * @param [in] hps HPS.
 * @param [in] lLineJoin LONG.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiSetLineJoin(HPS hps, LONG lLineJoin)
{
    struct {
        LONG lLineJoin;
    } args;
    ULONG result;
    int   rc;

    args.lLineJoin = lLineJoin;
    rc = GpiDispatch(hps, NGpiSetLineJoin,
                      &args, sizeof(args), &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiQueryLineJoin.
 * @param [in] hps HPS.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiQueryLineJoin(HPS hps)
{
    ULONG result;
    int   rc;

    rc = GpiDispatch(hps, NGpiQueryLineJoin,
                      NULL, 0, &result);
    if (rc != 0) return (LONG)GPI_ERROR;
    return (LONG)result;
}

/*!
 * @brief Client proxy for GpiSetCurrentPosition.
 * @param [in] hps HPS.
 * @param [in] pptlPoint PPOINTL.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiSetCurrentPosition(HPS hps, PPOINTL pptlPoint)
{
    struct {
        POINTL pptlPoint;
    } args;
    ULONG result;
    int   rc;

    args.pptlPoint = *pptlPoint;
    rc = GpiDispatch(hps, NGpiSetCurrentPosition,
                      &args, sizeof(args), &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiSetArcParams.
 * @param [in] hps HPS.
 * @param [in] parcpArcParams PARCPARAMS.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiSetArcParams(HPS hps, PARCPARAMS parcpArcParams)
{
    struct {
        ARCPARAMS parcpArcParams;
    } args;
    ULONG result;
    int   rc;

    args.parcpArcParams = *parcpArcParams;
    rc = GpiDispatch(hps, NGpiSetArcParams,
                      &args, sizeof(args), &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiPointArc.
 * @param [in] hps HPS.
 * @param [in] pptl2 PPOINTL.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiPointArc(HPS hps, PPOINTL pptl2)
{
    struct {
        POINTL pptl2;
    } args;
    ULONG result;
    int   rc;

    args.pptl2 = *pptl2;
    rc = GpiDispatch(hps, NGpiPointArc,
                      &args, sizeof(args), &result);
    if (rc != 0) return (LONG)GPI_ERROR;
    return (LONG)result;
}

/*!
 * @brief Client proxy for GpiFullArc.
 * @param [in] hps HPS.
 * @param [in] lControl LONG.
 * @param [in] fxMultiplier FIXED.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiFullArc(HPS hps, LONG lControl, FIXED fxMultiplier)
{
    struct {
        LONG lControl;
        FIXED fxMultiplier;
    } args;
    ULONG result;
    int   rc;

    args.lControl = lControl;
    args.fxMultiplier = fxMultiplier;
    rc = GpiDispatch(hps, NGpiFullArc,
                      &args, sizeof(args), &result);
    if (rc != 0) return (LONG)GPI_ERROR;
    return (LONG)result;
}

/*!
 * @brief Client proxy for GpiPartialArc.
 * @param [in] hps HPS.
 * @param [in] pptlCenter PPOINTL.
 * @param [in] fxMultiplier FIXED.
 * @param [in] fxStartAngle FIXED.
 * @param [in] fxSweepAngle FIXED.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiPartialArc(HPS hps, PPOINTL pptlCenter, FIXED fxMultiplier, FIXED fxStartAngle, FIXED fxSweepAngle)
{
    struct {
        POINTL pptlCenter;
        FIXED fxMultiplier;
        FIXED fxStartAngle;
        FIXED fxSweepAngle;
    } args;
    ULONG result;
    int   rc;

    args.pptlCenter = *pptlCenter;
    args.fxMultiplier = fxMultiplier;
    args.fxStartAngle = fxStartAngle;
    args.fxSweepAngle = fxSweepAngle;
    rc = GpiDispatch(hps, NGpiPartialArc,
                      &args, sizeof(args), &result);
    if (rc != 0) return (LONG)GPI_ERROR;
    return (LONG)result;
}

/*!
 * @brief Client proxy for GpiSetPatternSet.
 * @param [in] hps HPS.
 * @param [in] lSet LONG.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiSetPatternSet(HPS hps, LONG lSet)
{
    struct {
        LONG lSet;
    } args;
    ULONG result;
    int   rc;

    args.lSet = lSet;
    rc = GpiDispatch(hps, NGpiSetPatternSet,
                      &args, sizeof(args), &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiQueryPatternSet.
 * @param [in] hps HPS.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiQueryPatternSet(HPS hps)
{
    ULONG result;
    int   rc;

    rc = GpiDispatch(hps, NGpiQueryPatternSet,
                      NULL, 0, &result);
    if (rc != 0) return (LONG)GPI_ERROR;
    return (LONG)result;
}

/*!
 * @brief Client proxy for GpiSetPatternRefPoint.
 * @param [in] hps HPS.
 * @param [in] pptlRefPoint PPOINTL.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiSetPatternRefPoint(HPS hps, PPOINTL pptlRefPoint)
{
    struct {
        POINTL pptlRefPoint;
    } args;
    ULONG result;
    int   rc;

    args.pptlRefPoint = *pptlRefPoint;
    rc = GpiDispatch(hps, NGpiSetPatternRefPoint,
                      &args, sizeof(args), &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiSetCharSet.
 * @param [in] hps HPS.
 * @param [in] llcid LONG.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiSetCharSet(HPS hps, LONG llcid)
{
    struct {
        LONG llcid;
    } args;
    ULONG result;
    int   rc;

    args.llcid = llcid;
    rc = GpiDispatch(hps, NGpiSetCharSet,
                      &args, sizeof(args), &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiQueryCharSet.
 * @param [in] hps HPS.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiQueryCharSet(HPS hps)
{
    ULONG result;
    int   rc;

    rc = GpiDispatch(hps, NGpiQueryCharSet,
                      NULL, 0, &result);
    if (rc != 0) return (LONG)GPI_ERROR;
    return (LONG)result;
}

/*!
 * @brief Client proxy for GpiSetCharBox.
 * @param [in] hps HPS.
 * @param [in] psizfxBox PSIZEF.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiSetCharBox(HPS hps, PSIZEF psizfxBox)
{
    struct {
        SIZEF psizfxBox;
    } args;
    ULONG result;
    int   rc;

    args.psizfxBox = *psizfxBox;
    rc = GpiDispatch(hps, NGpiSetCharBox,
                      &args, sizeof(args), &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiSetCharAngle.
 * @param [in] hps HPS.
 * @param [in] pgradlAngle PGRADIENTL.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiSetCharAngle(HPS hps, PGRADIENTL pgradlAngle)
{
    struct {
        GRADIENTL pgradlAngle;
    } args;
    ULONG result;
    int   rc;

    args.pgradlAngle = *pgradlAngle;
    rc = GpiDispatch(hps, NGpiSetCharAngle,
                      &args, sizeof(args), &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiSetCharShear.
 * @param [in] hps HPS.
 * @param [in] pptlAngle PPOINTL.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiSetCharShear(HPS hps, PPOINTL pptlAngle)
{
    struct {
        POINTL pptlAngle;
    } args;
    ULONG result;
    int   rc;

    args.pptlAngle = *pptlAngle;
    rc = GpiDispatch(hps, NGpiSetCharShear,
                      &args, sizeof(args), &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiSetCharDirection.
 * @param [in] hps HPS.
 * @param [in] lDirection LONG.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiSetCharDirection(HPS hps, LONG lDirection)
{
    struct {
        LONG lDirection;
    } args;
    ULONG result;
    int   rc;

    args.lDirection = lDirection;
    rc = GpiDispatch(hps, NGpiSetCharDirection,
                      &args, sizeof(args), &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiQueryCharDirection.
 * @param [in] hps HPS.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiQueryCharDirection(HPS hps)
{
    ULONG result;
    int   rc;

    rc = GpiDispatch(hps, NGpiQueryCharDirection,
                      NULL, 0, &result);
    if (rc != 0) return (LONG)GPI_ERROR;
    return (LONG)result;
}

/*!
 * @brief Client proxy for GpiSetCharMode.
 * @param [in] hps HPS.
 * @param [in] lMode LONG.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiSetCharMode(HPS hps, LONG lMode)
{
    struct {
        LONG lMode;
    } args;
    ULONG result;
    int   rc;

    args.lMode = lMode;
    rc = GpiDispatch(hps, NGpiSetCharMode,
                      &args, sizeof(args), &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiQueryCharMode.
 * @param [in] hps HPS.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiQueryCharMode(HPS hps)
{
    ULONG result;
    int   rc;

    rc = GpiDispatch(hps, NGpiQueryCharMode,
                      NULL, 0, &result);
    if (rc != 0) return (LONG)GPI_ERROR;
    return (LONG)result;
}

/*!
 * @brief Client proxy for GpiSetTextAlignment.
 * @param [in] hps HPS.
 * @param [in] lHoriz LONG.
 * @param [in] lVert LONG.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiSetTextAlignment(HPS hps, LONG lHoriz, LONG lVert)
{
    struct {
        LONG lHoriz;
        LONG lVert;
    } args;
    ULONG result;
    int   rc;

    args.lHoriz = lHoriz;
    args.lVert = lVert;
    rc = GpiDispatch(hps, NGpiSetTextAlignment,
                      &args, sizeof(args), &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiSetCharExtra.
 * @param [in] hps HPS.
 * @param [in] Extra FIXED.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiSetCharExtra(HPS hps, FIXED  Extra)
{
    struct {
        FIXED Extra;
    } args;
    ULONG result;
    int   rc;

    args.Extra = Extra;
    rc = GpiDispatch(hps, NGpiSetCharExtra,
                      &args, sizeof(args), &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiSetCharBreakExtra.
 * @param [in] hps HPS.
 * @param [in] BreakExtra FIXED.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiSetCharBreakExtra(HPS hps, FIXED  BreakExtra)
{
    struct {
        FIXED BreakExtra;
    } args;
    ULONG result;
    int   rc;

    args.BreakExtra = BreakExtra;
    rc = GpiDispatch(hps, NGpiSetCharBreakExtra,
                      &args, sizeof(args), &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiMarker.
 * @param [in] hps HPS.
 * @param [in] pptlPoint PPOINTL.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiMarker(HPS hps, PPOINTL pptlPoint)
{
    struct {
        POINTL pptlPoint;
    } args;
    ULONG result;
    int   rc;

    args.pptlPoint = *pptlPoint;
    rc = GpiDispatch(hps, NGpiMarker,
                      &args, sizeof(args), &result);
    if (rc != 0) return (LONG)GPI_ERROR;
    return (LONG)result;
}

/*!
 * @brief Client proxy for GpiSetMarker.
 * @param [in] hps HPS.
 * @param [in] lSymbol LONG.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiSetMarker(HPS hps, LONG lSymbol)
{
    struct {
        LONG lSymbol;
    } args;
    ULONG result;
    int   rc;

    args.lSymbol = lSymbol;
    rc = GpiDispatch(hps, NGpiSetMarker,
                      &args, sizeof(args), &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiSetMarkerBox.
 * @param [in] hps HPS.
 * @param [in] psizfxSize PSIZEF.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiSetMarkerBox(HPS hps, PSIZEF psizfxSize)
{
    struct {
        SIZEF psizfxSize;
    } args;
    ULONG result;
    int   rc;

    args.psizfxSize = *psizfxSize;
    rc = GpiDispatch(hps, NGpiSetMarkerBox,
                      &args, sizeof(args), &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiSetMarkerSet.
 * @param [in] hps HPS.
 * @param [in] lSet LONG.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiSetMarkerSet(HPS hps, LONG lSet)
{
    struct {
        LONG lSet;
    } args;
    ULONG result;
    int   rc;

    args.lSet = lSet;
    rc = GpiDispatch(hps, NGpiSetMarkerSet,
                      &args, sizeof(args), &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiQueryMarker.
 * @param [in] hps HPS.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiQueryMarker(HPS hps)
{
    ULONG result;
    int   rc;

    rc = GpiDispatch(hps, NGpiQueryMarker,
                      NULL, 0, &result);
    if (rc != 0) return (LONG)GPI_ERROR;
    return (LONG)result;
}

/*!
 * @brief Client proxy for GpiQueryMarkerSet.
 * @param [in] hps HPS.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiQueryMarkerSet(HPS hps)
{
    ULONG result;
    int   rc;

    rc = GpiDispatch(hps, NGpiQueryMarkerSet,
                      NULL, 0, &result);
    if (rc != 0) return (LONG)GPI_ERROR;
    return (LONG)result;
}

/*!
 * @brief Client proxy for GpiPop.
 * @param [in] hps HPS.
 * @param [in] lCount LONG.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiPop(HPS hps, LONG lCount)
{
    struct {
        LONG lCount;
    } args;
    ULONG result;
    int   rc;

    args.lCount = lCount;
    rc = GpiDispatch(hps, NGpiPop,
                      &args, sizeof(args), &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiPtVisible.
 * @param [in] hps HPS.
 * @param [in] pptlPoint PPOINTL.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiPtVisible(HPS hps, PPOINTL pptlPoint)
{
    struct {
        POINTL pptlPoint;
    } args;
    ULONG result;
    int   rc;

    args.pptlPoint = *pptlPoint;
    rc = GpiDispatch(hps, NGpiPtVisible,
                      &args, sizeof(args), &result);
    if (rc != 0) return (LONG)GPI_ERROR;
    return (LONG)result;
}

/*!
 * @brief Client proxy for GpiRectVisible.
 * @param [in] hps HPS.
 * @param [in] prclRectangle PRECTL.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiRectVisible(HPS hps, PRECTL prclRectangle)
{
    struct {
        RECTL prclRectangle;
    } args;
    ULONG result;
    int   rc;

    args.prclRectangle = *prclRectangle;
    rc = GpiDispatch(hps, NGpiRectVisible,
                      &args, sizeof(args), &result);
    if (rc != 0) return (LONG)GPI_ERROR;
    return (LONG)result;
}

/*!
 * @brief Client proxy for GpiDeleteSetId.
 * @param [in] hps HPS.
 * @param [in] lLcid LONG.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiDeleteSetId(HPS hps, LONG lLcid)
{
    struct {
        LONG lLcid;
    } args;
    ULONG result;
    int   rc;

    args.lLcid = lLcid;
    rc = GpiDispatch(hps, NGpiDeleteSetId,
                      &args, sizeof(args), &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiQueryNumberSetIds.
 * @param [in] hps HPS.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiQueryNumberSetIds(HPS hps)
{
    ULONG result;
    int   rc;

    rc = GpiDispatch(hps, NGpiQueryNumberSetIds,
                      NULL, 0, &result);
    if (rc != 0) return (LONG)GPI_ERROR;
    return (LONG)result;
}

/*!
 * @brief Client proxy for GpiSetCp.
 * @param [in] hps HPS.
 * @param [in] ulCodePage ULONG.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiSetCp(HPS hps, ULONG ulCodePage)
{
    struct {
        ULONG ulCodePage;
    } args;
    ULONG result;
    int   rc;

    args.ulCodePage = ulCodePage;
    rc = GpiDispatch(hps, NGpiSetCp,
                      &args, sizeof(args), &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiQueryCp.
 * @param [in] hps HPS.
 * @return Implementation-defined value.
 */
ULONG APIENTRY GpiQueryCp(HPS hps)
{
    ULONG result;
    int   rc;

    rc = GpiDispatch(hps, NGpiQueryCp,
                      NULL, 0, &result);
    if (rc != 0) return (ULONG)GPI_ERROR;
    return (ULONG)result;
}

/*!
 * @brief Client proxy for GpiSetFontMappingFlags.
 * @param [in] hps HPS.
 * @param [in] flags ULONG.
 * @return Implementation-defined value.
 */
ULONG APIENTRY GpiSetFontMappingFlags(HPS   hps, ULONG flags)
{
    struct {
        ULONG flags;
    } args;
    ULONG result;
    int   rc;

    args.flags = flags;
    rc = GpiDispatch(hps, NGpiSetFontMappingFlags,
                      &args, sizeof(args), &result);
    if (rc != 0) return (ULONG)GPI_ERROR;
    return (ULONG)result;
}

/*!
 * @brief Client proxy for GpiQueryFontMappingFlags.
 * @param [in] hps HPS.
 * @return Implementation-defined value.
 */
ULONG APIENTRY GpiQueryFontMappingFlags(HPS   hps)
{
    ULONG result;
    int   rc;

    rc = GpiDispatch(hps, NGpiQueryFontMappingFlags,
                      NULL, 0, &result);
    if (rc != 0) return (ULONG)GPI_ERROR;
    return (ULONG)result;
}

/*!
 * @brief Client proxy for GpiLoadBitmap.
 * @param [in] hps HPS.
 * @param [in] Resource HMODULE.
 * @param [in] idBitmap ULONG.
 * @param [in] lWidth LONG.
 * @param [in] lHeight LONG.
 * @return Handle, or GPI_ERROR.
 */
HBITMAP APIENTRY GpiLoadBitmap(HPS hps, HMODULE Resource, ULONG idBitmap, LONG lWidth, LONG lHeight)
{
    struct {
        HMODULE Resource;
        ULONG idBitmap;
        LONG lWidth;
        LONG lHeight;
    } args;
    ULONG result;
    int   rc;

    args.Resource = Resource;
    args.idBitmap = idBitmap;
    args.lWidth = lWidth;
    args.lHeight = lHeight;
    rc = GpiDispatch(hps, NGpiLoadBitmap,
                      &args, sizeof(args), &result);
    if (rc != 0) return (HBITMAP)GPI_ERROR;
    return (HBITMAP)result;
}

/*!
 * @brief Client proxy for GpiSetBitmap.
 * @param [in] hps HPS.
 * @param [in] hbm HBITMAP.
 * @return Handle, or GPI_ERROR.
 */
HBITMAP APIENTRY GpiSetBitmap(HPS hps, HBITMAP hbm)
{
    struct {
        HBITMAP hbm;
    } args;
    ULONG result;
    int   rc;

    args.hbm = hbm;
    rc = GpiDispatch(hps, NGpiSetBitmap,
                      &args, sizeof(args), &result);
    if (rc != 0) return (HBITMAP)GPI_ERROR;
    return (HBITMAP)result;
}

/*!
 * @brief Client proxy for GpiSetBitmapId.
 * @param [in] hps HPS.
 * @param [in] hbm HBITMAP.
 * @param [in] lLcid LONG.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiSetBitmapId(HPS hps, HBITMAP hbm, LONG lLcid)
{
    struct {
        HBITMAP hbm;
        LONG lLcid;
    } args;
    ULONG result;
    int   rc;

    args.hbm = hbm;
    args.lLcid = lLcid;
    rc = GpiDispatch(hps, NGpiSetBitmapId,
                      &args, sizeof(args), &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiQueryBitmapHandle.
 * @param [in] hps HPS.
 * @param [in] lLcid LONG.
 * @return Handle, or GPI_ERROR.
 */
HBITMAP APIENTRY GpiQueryBitmapHandle(HPS hps, LONG lLcid)
{
    struct {
        LONG lLcid;
    } args;
    ULONG result;
    int   rc;

    args.lLcid = lLcid;
    rc = GpiDispatch(hps, NGpiQueryBitmapHandle,
                      &args, sizeof(args), &result);
    if (rc != 0) return (HBITMAP)GPI_ERROR;
    return (HBITMAP)result;
}

/*!
 * @brief Client proxy for GpiSetPel.
 * @param [in] hps HPS.
 * @param [in] pptlPoint PPOINTL.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiSetPel(HPS hps, PPOINTL pptlPoint)
{
    struct {
        POINTL pptlPoint;
    } args;
    ULONG result;
    int   rc;

    args.pptlPoint = *pptlPoint;
    rc = GpiDispatch(hps, NGpiSetPel,
                      &args, sizeof(args), &result);
    if (rc != 0) return (LONG)GPI_ERROR;
    return (LONG)result;
}

/*!
 * @brief Client proxy for GpiFloodFill.
 * @param [in] hps HPS.
 * @param [in] lOptions LONG.
 * @param [in] lColor LONG.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiFloodFill(HPS hps, LONG lOptions, LONG lColor)
{
    struct {
        LONG lOptions;
        LONG lColor;
    } args;
    ULONG result;
    int   rc;

    args.lOptions = lOptions;
    args.lColor = lColor;
    rc = GpiDispatch(hps, NGpiFloodFill,
                      &args, sizeof(args), &result);
    if (rc != 0) return (LONG)GPI_ERROR;
    return (LONG)result;
}

/*!
 * @brief Client proxy for GpiCombineRegion.
 * @param [in] hps HPS.
 * @param [in] hrgnDest HRGN.
 * @param [in] hrgnSrc1 HRGN.
 * @param [in] hrgnSrc2 HRGN.
 * @param [in] lMode LONG.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiCombineRegion(HPS hps, HRGN hrgnDest, HRGN hrgnSrc1, HRGN hrgnSrc2, LONG lMode)
{
    struct {
        HRGN hrgnDest;
        HRGN hrgnSrc1;
        HRGN hrgnSrc2;
        LONG lMode;
    } args;
    ULONG result;
    int   rc;

    args.hrgnDest = hrgnDest;
    args.hrgnSrc1 = hrgnSrc1;
    args.hrgnSrc2 = hrgnSrc2;
    args.lMode = lMode;
    rc = GpiDispatch(hps, NGpiCombineRegion,
                      &args, sizeof(args), &result);
    if (rc != 0) return (LONG)GPI_ERROR;
    return (LONG)result;
}

/*!
 * @brief Client proxy for GpiDestroyRegion.
 * @param [in] hps HPS.
 * @param [in] hrgn HRGN.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiDestroyRegion(HPS hps, HRGN hrgn)
{
    struct {
        HRGN hrgn;
    } args;
    ULONG result;
    int   rc;

    args.hrgn = hrgn;
    rc = GpiDispatch(hps, NGpiDestroyRegion,
                      &args, sizeof(args), &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiEqualRegion.
 * @param [in] hps HPS.
 * @param [in] hrgnSrc1 HRGN.
 * @param [in] hrgnSrc2 HRGN.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiEqualRegion(HPS hps, HRGN hrgnSrc1, HRGN hrgnSrc2)
{
    struct {
        HRGN hrgnSrc1;
        HRGN hrgnSrc2;
    } args;
    ULONG result;
    int   rc;

    args.hrgnSrc1 = hrgnSrc1;
    args.hrgnSrc2 = hrgnSrc2;
    rc = GpiDispatch(hps, NGpiEqualRegion,
                      &args, sizeof(args), &result);
    if (rc != 0) return (LONG)GPI_ERROR;
    return (LONG)result;
}

/*!
 * @brief Client proxy for GpiOffsetRegion.
 * @param [in] hps HPS.
 * @param [in] Hrgn HRGN.
 * @param [in] pptlOffset PPOINTL.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiOffsetRegion(HPS hps, HRGN Hrgn, PPOINTL pptlOffset)
{
    struct {
        HRGN Hrgn;
        POINTL pptlOffset;
    } args;
    ULONG result;
    int   rc;

    args.Hrgn = Hrgn;
    args.pptlOffset = *pptlOffset;
    rc = GpiDispatch(hps, NGpiOffsetRegion,
                      &args, sizeof(args), &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiPaintRegion.
 * @param [in] hps HPS.
 * @param [in] hrgn HRGN.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiPaintRegion(HPS hps, HRGN hrgn)
{
    struct {
        HRGN hrgn;
    } args;
    ULONG result;
    int   rc;

    args.hrgn = hrgn;
    rc = GpiDispatch(hps, NGpiPaintRegion,
                      &args, sizeof(args), &result);
    if (rc != 0) return (LONG)GPI_ERROR;
    return (LONG)result;
}

/*!
 * @brief Client proxy for GpiFrameRegion.
 * @param [in] hps HPS.
 * @param [in] hrgn HRGN.
 * @param [in] thickness PSIZEL.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiFrameRegion(HPS hps, HRGN hrgn, PSIZEL thickness)
{
    struct {
        HRGN hrgn;
        SIZEL thickness;
    } args;
    ULONG result;
    int   rc;

    args.hrgn = hrgn;
    args.thickness = *thickness;
    rc = GpiDispatch(hps, NGpiFrameRegion,
                      &args, sizeof(args), &result);
    if (rc != 0) return (LONG)GPI_ERROR;
    return (LONG)result;
}

/*!
 * @brief Client proxy for GpiPtInRegion.
 * @param [in] hps HPS.
 * @param [in] hrgn HRGN.
 * @param [in] pptlPoint PPOINTL.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiPtInRegion(HPS hps, HRGN hrgn, PPOINTL pptlPoint)
{
    struct {
        HRGN hrgn;
        POINTL pptlPoint;
    } args;
    ULONG result;
    int   rc;

    args.hrgn = hrgn;
    args.pptlPoint = *pptlPoint;
    rc = GpiDispatch(hps, NGpiPtInRegion,
                      &args, sizeof(args), &result);
    if (rc != 0) return (LONG)GPI_ERROR;
    return (LONG)result;
}

/*!
 * @brief Client proxy for GpiRectInRegion.
 * @param [in] hps HPS.
 * @param [in] hrgn HRGN.
 * @param [in] prclRect PRECTL.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiRectInRegion(HPS hps, HRGN hrgn, PRECTL prclRect)
{
    struct {
        HRGN hrgn;
        RECTL prclRect;
    } args;
    ULONG result;
    int   rc;

    args.hrgn = hrgn;
    args.prclRect = *prclRect;
    rc = GpiDispatch(hps, NGpiRectInRegion,
                      &args, sizeof(args), &result);
    if (rc != 0) return (LONG)GPI_ERROR;
    return (LONG)result;
}

/*!
 * @brief Client proxy for GpiQueryClipRegion.
 * @param [in] hps HPS.
 * @return Handle, or GPI_ERROR.
 */
HRGN APIENTRY GpiQueryClipRegion(HPS hps)
{
    ULONG result;
    int   rc;

    rc = GpiDispatch(hps, NGpiQueryClipRegion,
                      NULL, 0, &result);
    if (rc != 0) return (HRGN)GPI_ERROR;
    return (HRGN)result;
}

/*!
 * @brief Client proxy for GpiExcludeClipRectangle.
 * @param [in] hps HPS.
 * @param [in] prclRectangle PRECTL.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiExcludeClipRectangle(HPS hps, PRECTL prclRectangle)
{
    struct {
        RECTL prclRectangle;
    } args;
    ULONG result;
    int   rc;

    args.prclRectangle = *prclRectangle;
    rc = GpiDispatch(hps, NGpiExcludeClipRectangle,
                      &args, sizeof(args), &result);
    if (rc != 0) return (LONG)GPI_ERROR;
    return (LONG)result;
}

/*!
 * @brief Client proxy for GpiIntersectClipRectangle.
 * @param [in] hps HPS.
 * @param [in] prclRectangle PRECTL.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiIntersectClipRectangle(HPS hps, PRECTL prclRectangle)
{
    struct {
        RECTL prclRectangle;
    } args;
    ULONG result;
    int   rc;

    args.prclRectangle = *prclRectangle;
    rc = GpiDispatch(hps, NGpiIntersectClipRectangle,
                      &args, sizeof(args), &result);
    if (rc != 0) return (LONG)GPI_ERROR;
    return (LONG)result;
}

/*!
 * @brief Client proxy for GpiOffsetClipRegion.
 * @param [in] hps HPS.
 * @param [in] pptlPoint PPOINTL.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiOffsetClipRegion(HPS hps, PPOINTL pptlPoint)
{
    struct {
        POINTL pptlPoint;
    } args;
    ULONG result;
    int   rc;

    args.pptlPoint = *pptlPoint;
    rc = GpiDispatch(hps, NGpiOffsetClipRegion,
                      &args, sizeof(args), &result);
    if (rc != 0) return (LONG)GPI_ERROR;
    return (LONG)result;
}

/*!
 * @brief Client proxy for GpiCreateRoundRectRegion.
 * @param [in] hps HPS.
 * @param [in] pptlPoint PPOINTL.
 * @param [in] lHround LONG.
 * @param [in] lVRound LONG.
 * @return Handle, or GPI_ERROR.
 */
HRGN APIENTRY GpiCreateRoundRectRegion(HPS hps, PPOINTL pptlPoint, LONG lHround, LONG lVRound)
{
    struct {
        POINTL pptlPoint;
        LONG lHround;
        LONG lVRound;
    } args;
    ULONG result;
    int   rc;

    args.pptlPoint = *pptlPoint;
    args.lHround = lHround;
    args.lVRound = lVRound;
    rc = GpiDispatch(hps, NGpiCreateRoundRectRegion,
                      &args, sizeof(args), &result);
    if (rc != 0) return (HRGN)GPI_ERROR;
    return (HRGN)result;
}

/*!
 * @brief Client proxy for GpiCreateEllipticRegion.
 * @param [in] hps HPS.
 * @param [in] prclRect PRECTL.
 * @return Handle, or GPI_ERROR.
 */
HRGN APIENTRY GpiCreateEllipticRegion(HPS hps, PRECTL prclRect)
{
    struct {
        RECTL prclRect;
    } args;
    ULONG result;
    int   rc;

    args.prclRect = *prclRect;
    rc = GpiDispatch(hps, NGpiCreateEllipticRegion,
                      &args, sizeof(args), &result);
    if (rc != 0) return (HRGN)GPI_ERROR;
    return (HRGN)result;
}

/*!
 * @brief Client proxy for GpiResumePlay.
 * @param [in] hps HPS.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiResumePlay(HPS hps)
{
    ULONG result;
    int   rc;

    rc = GpiDispatch(hps, NGpiResumePlay,
                      NULL, 0, &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiSuspendPlay.
 * @param [in] hps HPS.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiSuspendPlay(HPS hps)
{
    ULONG result;
    int   rc;

    rc = GpiDispatch(hps, NGpiSuspendPlay,
                      NULL, 0, &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiSetDefArcParams.
 * @param [in] hps HPS.
 * @param [in] parcpArcParams PARCPARAMS.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiSetDefArcParams(HPS hps, PARCPARAMS parcpArcParams)
{
    struct {
        ARCPARAMS parcpArcParams;
    } args;
    ULONG result;
    int   rc;

    args.parcpArcParams = *parcpArcParams;
    rc = GpiDispatch(hps, NGpiSetDefArcParams,
                      &args, sizeof(args), &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiSetDefTag.
 * @param [in] hps HPS.
 * @param [in] lTag LONG.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiSetDefTag(HPS hps, LONG lTag)
{
    struct {
        LONG lTag;
    } args;
    ULONG result;
    int   rc;

    args.lTag = lTag;
    rc = GpiDispatch(hps, NGpiSetDefTag,
                      &args, sizeof(args), &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiSetDefViewingLimits.
 * @param [in] hps HPS.
 * @param [in] prclLimits PRECTL.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiSetDefViewingLimits(HPS hps, PRECTL prclLimits)
{
    struct {
        RECTL prclLimits;
    } args;
    ULONG result;
    int   rc;

    args.prclLimits = *prclLimits;
    rc = GpiDispatch(hps, NGpiSetDefViewingLimits,
                      &args, sizeof(args), &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiBeginInkPath.
 * @param [in] hps HPS.
 * @param [in] lPath LONG.
 * @param [in] flOptions ULONG.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiBeginInkPath(HPS hps, LONG lPath, ULONG flOptions)
{
    struct {
        LONG lPath;
        ULONG flOptions;
    } args;
    ULONG result;
    int   rc;

    args.lPath = lPath;
    args.flOptions = flOptions;
    rc = GpiDispatch(hps, NGpiBeginInkPath,
                      &args, sizeof(args), &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}

/*!
 * @brief Client proxy for GpiEndInkPath.
 * @param [in] hps HPS.
 * @param [in] flOptions ULONG.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiEndInkPath(HPS hps, ULONG flOptions)
{
    struct {
        ULONG flOptions;
    } args;
    ULONG result;
    int   rc;

    args.flOptions = flOptions;
    rc = GpiDispatch(hps, NGpiEndInkPath,
                      &args, sizeof(args), &result);
    if (rc != 0) return (BOOL)GPI_ERROR;
    return (BOOL)result;
}
