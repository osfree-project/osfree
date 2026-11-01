/*!
   @file GOCA_EXCEPTIONS.H

   @brief Exception Conditions (EC codes) of the GOCA architecture.

   Contains the complete list of exception constants defined in:
     - AFP GOCA Reference (AFPC-0008-03), Chapter 8, "Exception Conditions"
     - IBM SC31-6804, "Graphics Object Content Architecture Reference"

   Each exception belongs to one of two categories:
     - DRAWING_PROCESS_CHECK: drawing stops (fatal for the order)
     - STANDARD_ACTION:       a prescribed action is performed, drawing
                               continues

   @note  For the complete list of general exceptions not tied to a specific
          drawing order, see "General Exception Conditions"
          (AFPC-0008-03, p. 108–110).
*/

#ifndef GOCA_EXCEPTIONS_H
#define GOCA_EXCEPTIONS_H

#ifdef __cplusplus
extern "C" {
#endif

/* ======================================================================== */
/*  Exception Categories                                                    */
/* ======================================================================== */

typedef enum _GOCA_EC_CATEGORY {
    GOCA_ECCAT_DRAWING_CHECK = 0,  /* Drawing process check (stop)        */
    GOCA_ECCAT_STANDARD_ACTION = 1  /* Standard action (continue)          */
} GOCA_EC_CATEGORY;

/* ======================================================================== */
/*  General Exception Conditions                                            */
/*  Can occur on any order type.                                            */
/* ======================================================================== */

/*! @brief An unallocated (unsupported) order code has been detected, or the
           device does not support this order.
    @details The operation code is either unallocated in the architecture or
             not implemented by the receiver. This exception has priority over
             all other exceptions.
    @action  Standard action: ignore the order and advance to the next order
             using the order format information.
    @see     AFPC-0008-03, p. 109; SC31-6804, p. 109. */
#define GOCA_EC_UNALLOCATED_ORDER       0x0001  /* Std: skip order          */

/*! @brief A reserved byte or bit in the order is not set to zero.
    @details Receivers may or may not generate this exception; it is not
             universally enforced (pre-May-2012 implementations may differ).
    @action  Drawing process check: drawing stops.
    @see     AFPC-0008-03, p. 168. */
#define GOCA_EC_RESERVED_NOT_ZERO       0x0002  /* Drawing check            */

/*! @brief The order length is incorrect: it does not match the format or
           specification for this order type.
    @details For example, a fixed two-byte order whose second byte is missing,
             or a long-format order whose length count does not match the
             actual byte count.
    @action  Drawing process check.
    @see     AFPC-0008-03, p. 168; SC31-6804, p. 168. */
#define GOCA_EC_INVALID_LENGTH          0x0003  /* Drawing check            */

/*! @brief An attribute value specified in the order is not valid.
    @details The attribute value falls outside the allowed range or is
             otherwise illegal for the current attribute.
    @action  Standard action: use the standard default value of the attribute.
             For color attributes, the action may be implementation-dependent.
    @see     AFPC-0008-03, p. 168; SC31-6804, p. 196. */
#define GOCA_EC_INVALID_ATTR_VALUE      0x0004  /* Std: default value       */

/*! @brief Segment stack overflow (Push-and-Set orders only).
    @details The segment stack used by push/pop operations exceeds its
             architectural limit.
    @action  Drawing process check.
    @see     AFPC-0008-03, p. 168. */
#define GOCA_EC_STACK_OVERFLOW          0x0005  /* Drawing check            */

/*! @brief Truncated order: the order extends beyond the segment boundary,
           or an extended-order qualifier is missing.
    @details Specific conditions include: a fixed two-byte order whose second
             byte is outside the segment; a long-format order whose length
             byte is outside the segment; a long/extended order whose declared
             length exceeds the remaining segment bytes; an extended order
             missing its qualifier byte or length bytes.
             Orders may legitimately be split across an appended segment.
    @action  Drawing process check.
    @see     AFPC-0008-03, p. 168; SC31-6804, p. 169. */
#define GOCA_EC_TRUNCATED_ORDER         0x0008  /* Drawing check            */

/*! @brief Invalid Graphics Data Descriptor (GDD).
    @details The GDD passed in the invocation to the drawing processor
             contains invalid bits. This exception may optionally be generated
             in MO:DCA environments.
    @action  Drawing process check.
    @see     AFPC-0008-03, p. 169. */
#define GOCA_EC_INVALID_DESCRIPTOR      0x000A  /* Drawing check            */

/*! @brief An order that is not valid in the segment prolog has been detected,
           or the segment ends without an End Prolog (GEPROL) order.
    @details Within the prolog section of a segment, either an order that is
             illegal in the prolog appears, or the prolog is terminated
             implicitly by the end of the segment without an explicit End
             Prolog order.
    @action  Drawing process check.
    @see     AFPC-0008-03, p. 169. */
#define GOCA_EC_INVALID_IN_PROLOG       0x000C  /* Drawing check            */

/*! @brief Graphic Presentation Space (GPS) overflow: an attempt to draw
           outside the GPS.
    @details An order is executed that tries to draw something outside the
             Graphic Presentation Space.
    @action  Implementation-dependent.
    @see     AFPC-0008-03, p. 169. */
#define GOCA_EC_GPS_OVERFLOW            0x000D  /* Implementation-dependent */

/*! @brief An attribute value is not supported.
    @details The attribute value is syntactically valid but not supported by
             the receiver.
    @action  Standard action: use the standard default value. For color,
             implementation-dependent.
    @see     AFPC-0008-03, p. 168. */
#define GOCA_EC_UNSUPPORTED_ATTR_VALUE  0x000E  /* Std: default value       */

/*! @brief The current Mix mode is XOR, and a line join cannot be correctly
           represented.
    @details This applies to arc and line orders where the XOR mix mode
             produces an indeterminate or unrepresentable join.
    @action  Standard action: implementation-dependent.
    @see     AFPC-0008-03, p. 120 (Arc orders). */
#define GOCA_EC_XOR_JOIN                0x0013  /* Std: impl-dependent      */

/*! @brief The segment with the specified name was not found in the segment
           store.
    @details A reference to a non-existent segment (e.g., for segment
             operations or retrieval).
    @action  Drawing process check.
    @see     SC31-6804, p. 109. */
#define GOCA_EC_INVALID_SEGMENT_NAME    0x0009  /* Drawing check            */

/*! @brief Invalid bits in the draw control.
    @details **NOTE:** This code (0x000A) conflicts with
             GOCA_EC_INVALID_DESCRIPTOR. Both are defined as 0x000A in the
             original header. Verify against the target specification and
             resolve the collision before deployment.
    @action  Drawing process check (per SC31-6804).
    @see     SC31-6804, p. 109. */
#define GOCA_EC_INVALID_DRAW_CONTROL    0x000A  /* Drawing check            */

/*! @brief Dynamic segment sequence violation.
    @details A segment operation violates the required sequence for dynamic
             segments (e.g., begin/end nesting rules).
    @action  Drawing process check.
    @see     SC31-6804, p. 109. */
#define GOCA_EC_DYNAMIC_SEQ_CHECK       0x0046  /* Drawing check            */

/*! @brief The Segment Characteristics order was found outside the segment
           prolog.
    @details This is an optional exception. The Segment Characteristics order
             is only valid inside the prolog.
    @action  Drawing process check (if generated).
    @see     AFPC-0008-03, p. 169. */
#define GOCA_EC_SEGCHAR_OUTSIDE_PROLOG  0x0400  /* Drawing check            */

/*! @brief End Prolog (GEPROL) found outside the segment prolog.
    @details An End Prolog order appears when no prolog is active.
    @action  Drawing process check.
    @see     AFPC-0008-03, p. 169; pmord.h (comment on OCODE_GEPROL). */
#define GOCA_EC_END_PROLOG_OUTSIDE      0x3E00  /* Drawing check            */

/* ======================================================================== */
/*  Area (GBAR / GEAR)                                                      */
/* ======================================================================== */

/*! @brief End Area (GEAR) without a preceding Begin Area (GBAR).
    @details An End Area order is executed without a matching, unmatched
             Begin Area order having been executed previously.
    @action  Drawing process check.
    @see     AFPC-0008-03, p. 169. */
#define GOCA_EC_GEAR_WITHOUT_GBAR       0x6000  /* Drawing check            */

/*! @brief Begin Area after Begin Area without an intervening End Area.
    @details A Begin Area order is executed while another Begin Area is still
             active (nesting violation).
    @action  Drawing process check.
    @see     AFPC-0008-03, p. 169. */
#define GOCA_EC_GBAR_NESTED             0x6800  /* Drawing check            */

/*! @brief End of segment reached without an End Area after a Begin Area.
    @details A Begin Area has been executed in a segment, but the segment
             ends before the matching End Area is executed.
    @action  Drawing process check.
    @see     AFPC-0008-03, p. 169. */
#define GOCA_EC_GBAR_UNCLOSED           0x6801  /* Drawing check            */

/*! @brief An order that is not valid inside an area has been detected.
    @details A supported order appears inside an area (between GBAR and GEAR)
             where it is architecturally invalid.
    @action  Drawing process check.
    @see     AFPC-0008-03, p. 169. */
#define GOCA_EC_ORDER_INVALID_IN_AREA   0x6802  /* Drawing check            */

/*! @brief The current pattern set is not supported.
    @details The pattern set referenced by the current pattern-set attribute
             is unavailable or unsupported.
    @action  Standard action: use the default pattern set.
    @see     AFPC-0008-03, p. 43 (Areas). */
#define GOCA_EC_PATTSET_UNAVAILABLE     0x6803  /* Std: default pattern set */

/*! @brief The current pattern symbol is not defined in the current pattern
           set.
    @details The pattern-symbol code point does not exist in the active
             pattern set.
    @action  Standard action: use the special symbol X'FF' (or the default
             symbol if X'FF' is invalid).
    @see     AFPC-0008-03, p. 43. */
#define GOCA_EC_PATTSYM_UNDEFINED       0x6804  /* Std: special symbol X'FF'*/

/*! @brief Temporary storage overflow while drawing an area in an
           immediate segment.
    @details The area to be drawn requires more temporary storage than is
             available in the immediate-segment environment.
    @action  Implementation-dependent.
    @see     AFPC-0008-03, p. 169. */
#define GOCA_EC_AREA_STORAGE_OVERFLOW   0x6805  /* Impl-dependent           */

/*! @brief A GBAR was encountered inside a custom pattern, but the current
           pattern symbol refers to a custom pattern or gradient.
    @details Inside a custom pattern definition, a Begin Area order appears
             while the active pattern symbol points to a custom pattern or
             gradient, which is architecturally inconsistent.
    @action  Standard action: use symbol X'10'.
    @see     AFPC-0008-03, p. 169. */
#define GOCA_EC_GBAR_IN_CUSTOM_PATTERN  0x6806  /* Std: symbol X'10'        */

/* ======================================================================== */
/*  Image (GBIMG / GCBIMG / GIMD / GEIMG)                                   */
/* ======================================================================== */

/*! @brief Image Data (GIMD) without a preceding Begin Image.
    @details A GIMD order appears in a segment where no Begin Image order
             has been executed first.
    @action  Drawing process check.
    @see     AFPC-0008-03, p. 169. */
#define GOCA_EC_GIMD_WITHOUT_GBIMG      0x9200  /* Drawing check            */

/*! @brief Incorrect byte count in GIMD.
    @details The Image Data order contains either too few or too many data
             bytes for the current image row/format.
    @action  Drawing process check.
    @see     AFPC-0008-03, p. 169. */
#define GOCA_EC_GIMD_BAD_BYTE_COUNT     0x9201  /* Drawing check            */

/*! @brief End Image (GEIMG) without a preceding Begin Image.
    @details A GEIMG order is executed without an unmatched Begin Image order
             having been executed previously.
    @action  Drawing process check.
    @see     AFPC-0008-03, p. 169. */
#define GOCA_EC_GEIMG_WITHOUT_GBIMG     0x9300  /* Drawing check            */

/*! @brief The number of GIMD orders between Begin Image and End Image does
           not equal the number of rows (HEIGHT).
    @details The row count specified by the HEIGHT parameter in Begin Image
             does not match the actual number of Image Data orders supplied.
    @action  Drawing process check.
    @see     AFPC-0008-03, p. 169. */
#define GOCA_EC_IMAGE_ROW_COUNT_MISMATCH 0x9301 /* Drawing check            */

/*! @brief Begin Image in a segment, but the segment ends without End Image.
    @details A Begin Image has been executed in a segment, and the end of the
             segment is reached without an End Image order.
    @action  Drawing process check.
    @see     AFPC-0008-03, p. 169. */
#define GOCA_EC_GBIMG_UNCLOSED          0xD100  /* Drawing check            */

/*! @brief After Begin Image, an order other than GCOMT, GNOP1, GIMD, or
           GEIMG was encountered.
    @details Inside an image (after Begin Image), only Comment, No-Operation,
             Image Data, and End Image orders are permitted. Any other
             supported order triggers this exception.
    @action  Drawing process check.
    @see     AFPC-0008-03, p. 169. */
#define GOCA_EC_ORDER_INVALID_IN_IMAGE  0xD101  /* Drawing check            */

/*! @brief Unsupported FORMAT value in Begin Image.
    @details The value specified for the FORMAT parameter in a Begin Image
             order is not supported by the receiver.
    @action  Drawing process check.
    @see     AFPC-0008-03, p. 169. */
#define GOCA_EC_IMAGE_FORMAT_UNSUPPORTED 0xD102 /* Drawing check            */

/*! @brief WIDTH in Begin Image is too large to be fully displayed.
    @details The image width exceeds what can be rendered in the available
             output area.
    @action  Standard action: truncate the width.
    @see     AFPC-0008-03, p. 172. */
#define GOCA_EC_IMAGE_WIDTH_TRUNCATED   0xD103  /* Std: truncate width      */

/*! @brief HEIGHT in Begin Image is too large.
    @details The image height exceeds what can be rendered in the available
             output area.
    @action  Standard action: truncate the height.
    @see     AFPC-0008-03, p. 172. */
#define GOCA_EC_IMAGE_HEIGHT_TRUNCATED  0xD104  /* Std: truncate height     */

/* ======================================================================== */
/*  Box (GBOX / GCBOX)                                                      */
/* ======================================================================== */

/*! @brief HAXIS or VAXIS is too large for the specified corner to fit
           within the box size.
    @details One or both axis values are so large that the requested corner
             geometry cannot be accommodated.
    @action  Standard action: use the largest fit axis.
    @see     AFPC-0008-03, p. 90. */
#define GOCA_EC_BOX_AXIS_TOO_LARGE      0xC000  /* Std: largest fit axis    */

/*! @brief HAXIS or VAXIS is out of the allowed range.
    @details One or both axis values fall outside their permissible range.
             Note: a related but distinct condition, EC-C001 (negative radius
             in a Box Order), is documented separately.
    @action  Standard action: use square corners.
    @see     AFPC-0008-03, p. 90. */
#define GOCA_EC_BOX_AXIS_OUT_OF_RANGE   0xC001  /* Std: square corners      */

/* ======================================================================== */
/*  Marker (GSMS / GSMT)                                                    */
/* ======================================================================== */

/*! @brief The current marker set is unavailable.
    @details The coded font identified by the current marker-set attribute
             is not available.
    @action  Standard action: use the default marker set.
    @see     AFPC-0008-03, p. 172. */
#define GOCA_EC_MARKER_SET_UNAVAILABLE  0xC200  /* Std: default marker set  */

/*! @brief The GSMT code point is not defined in the current marker set.
    @details The marker-symbol code point does not exist in the active marker
             set.
    @action  Standard action: use the corresponding symbol from the special
             marker set (X'FF'), or the default marker symbol if X'FF' is
             invalid.
    @see     AFPC-0008-03, p. 172. */
#define GOCA_EC_MARKER_SYMBOL_UNDEFINED 0xC201  /* Std: default symbol X'01'*/

/* ======================================================================== */
/*  Character (GSCS / GCHST / GCCHST / Extended Char Strings)               */
/* ======================================================================== */

/*! @brief The font specified in GSCS is unavailable.
    @details The character-set font referenced by the current font attribute
             is not accessible.
    @action  Standard action: use the default font.
    @see     AFPC-0008-03, p. 172. */
#define GOCA_EC_FONT_UNAVAILABLE        0xC300  /* Std: default font        */

/*! @brief A code point does not correspond to a valid graphic character.
    @details The character code point is not mapped to a graphic character
             in the current character set.
    @action  Standard action: use a special or default symbol.
    @see     AFPC-0008-03, p. 172. */
#define GOCA_EC_INVALID_CODE_POINT      0xC301  /* Std: special/default sym */

/*! @brief The current character set does not support the functions required
           by the current character precision.
    @details The character set cannot provide the precision level (e.g.,
             positioning accuracy) demanded by the current character-precision
             attribute.
    @action  Standard action: use the highest precision the character set can
             support.
    @see     AFPC-0008-03, p. 172. */
#define GOCA_EC_PRECISION_UNSUPPORTED   0xC302  /* Std: highest precision   */

/*! @brief The encoding is Unicode, but the code points are not valid
           Unicode data.
    @details A Unicode character string contains code points that are not
             valid Unicode scalar values (e.g., surrogate halves, out-of-range
             values).
    @action  Standard action: skip the invalid code points.
    @see     AFPC-0008-03, p. 172. */
#define GOCA_EC_INVALID_UNICODE         0xC303  /* Std: skip code points    */

/* ======================================================================== */
/*  Arc (GPARC / GCPARC / GFARC / GCFARC / GARC / GCARC)                    */
/* ======================================================================== */

/*! @brief An exceptional situation may prevent the arc from being drawn
           with normal accuracy.
    @details The arc parameters or environment may degrade the achievable
             accuracy.
    @action  Standard action: draw the arc as accurately as possible.
    @see     AFPC-0008-03, p. 116, 172. */
#define GOCA_EC_ARC_ACCURACY            0xC601  /* Std: draw as accurately  */

/*! @brief The start and end points of the arc are identical.
    @details A degenerate arc whose endpoints coincide, so no curve is
             produced.
    @action  Standard action: draw straight lines (or equivalent degenerate
             representation).
    @see     AFPC-0008-03, p. 116. */
#define GOCA_EC_ARC_ENDPOINTS_IDENTICAL 0xC600  /* Std: straight lines      */

/*! @brief The arc parameters define a degenerate ellipse.
    @details The semi-axis values or other parameters yield a degenerate
             ellipse (e.g., zero radius on one axis).
    @action  Standard action: draw straight lines.
    @see     AFPC-0008-03, p. 116. */
#define GOCA_EC_ARC_DEGENERATE_ELLIPSE  0xC603  /* Std: straight lines      */

/* ======================================================================== */
/*  Path (GBPTH / GEPTH / PM GPI paths)                                     */
/* ======================================================================== */

/*! @brief Begin Path after Begin Path without an intervening End Path.
    @details A Begin Path order is executed while another Begin Path is still
             active.
    @action  Drawing process check.
    @see     AFPC-0008-03 (PM GPI extension); outputlinks SC31-6804, p. 229. */
#define GOCA_EC_PATH_NESTED             0xD000  /* Drawing check            */

/*! @brief End of segment reached without an End Path after a Begin Path.
    @details A Begin Path has been executed in a segment, but the segment
             ends before the matching End Path.
    @action  Drawing process check.
    @see     outputlinks SC31-6804, p. 229. */
#define GOCA_EC_PATH_UNCLOSED           0xD001  /* Drawing check            */

/*! @brief An order that is not valid inside a path has been detected.
    @details A supported order appears inside a path (between GBPTH and
             GEPTH) where it is architecturally invalid.
    @action  Drawing process check.
    @see     outputlinks SC31-6804, p. 229. */
#define GOCA_EC_ORDER_INVALID_IN_PATH   0xD002  /* Drawing check            */

/*! @brief PTHID is zero.
    @details The path identifier (PTHID) is zero, which is not a valid path
             identifier.
    @action  Drawing process check.
    @see     outputlinks SC31-6804, p. 229. */
#define GOCA_EC_PATH_ID_ZERO            0xD003  /* Drawing check            */

/* ======================================================================== */
/*  Linear Gradient                                                         */
/* ======================================================================== */

#define GOCA_EC_LGRAD_PATTSET_INVALID   0xDC00  /* Std: ignore order        */
#define GOCA_EC_LGRAD_PATTSYM_INVALID   0xDC01  /* Std: ignore order        */
#define GOCA_EC_LGRAD_PATTERN_EXISTS    0xDC02  /* Std: ignore order        */
#define GOCA_EC_LGRAD_STORAGE           0xDC03  /* Std: ignore order        */
#define GOCA_EC_LGRAD_OUTSIDE_INVALID   0xDC04  /* Std: use X'00' (None)    */
#define GOCA_EC_LGRAD_COLORSTOP_INVALID 0xDC05  /* Std: ignore color stop   */
#define GOCA_EC_LGRAD_COLORSPEC_LENGTH  0xDC06  /* Std: ignore order        */
#define GOCA_EC_LGRAD_COLOR_RULES       0xDC07  /* Std: ignore order        */

/* ======================================================================== */
/*  Radial Gradient                                                         */
/* ======================================================================== */

#define GOCA_EC_RGRAD_PATTSET_INVALID   0xDD00  /* Std: ignore order        */
#define GOCA_EC_RGRAD_PATTSYM_INVALID   0xDD01  /* Std: ignore order        */
#define GOCA_EC_RGRAD_PATTERN_EXISTS    0xDD02  /* Std: ignore order        */
#define GOCA_EC_RGRAD_STORAGE           0xDD03  /* Std: ignore order        */
#define GOCA_EC_RGRAD_OUTSIDE_INVALID   0xDD04  /* Std: use X'00' (None)    */
#define GOCA_EC_RGRAD_COLORSTOP_INVALID 0xDD05  /* Std: ignore color stop   */
#define GOCA_EC_RGRAD_COLORSPEC_LENGTH  0xDD06  /* Std: ignore order        */
#define GOCA_EC_RGRAD_COLOR_RULES       0xDD07  /* Std: ignore order        */

/* ======================================================================== */
/*  Custom Pattern (GBCP / GECP)                                            */
/* ======================================================================== */

#define GOCA_EC_CP_NESTED               0xDE00  /* Std: implicit GECP       */
#define GOCA_EC_CP_UNCLOSED             0xDE01  /* Std: implicit GECP       */
#define GOCA_EC_CP_PATTSET_INVALID      0xDE02  /* Std: skip to GECP        */
#define GOCA_EC_CP_PATTSYM_INVALID      0xDE03  /* Std: skip to GECP        */
#define GOCA_EC_CP_WINDOW_INVALID       0xDE04  /* Std: skip to GECP        */
#define GOCA_EC_CP_PATTERN_EXISTS       0xDE05  /* Std: skip to GECP        */
#define GOCA_EC_CP_STORAGE              0xDE06  /* Std: skip to GECP        */
#define GOCA_EC_CP_ORDER_INVALID        0xDE07  /* Std: ignore order        */

/* ======================================================================== */
/*  Delete Pattern (GDP)                                                    */
/* ======================================================================== */

#define GOCA_EC_GDP_NO_PATTERN          0xDF00  /* Std: ignore GDP          */
#define GOCA_EC_GDP_PATTSET_INVALID     0xDF01  /* Std: ignore GDP          */
#define GOCA_EC_GDP_PATTSYM_INVALID     0xDF02  /* Std: ignore GDP          */

/* ======================================================================== */
/*  Relative Line (GRLINE / GCRLINE)                                        */
/* ======================================================================== */

/*! @brief A relative line starts inside the GPS but then goes outside it.
    @details The start point is within the GPS, but the computed end point
             falls outside the GPS. For devices that can maintain a position
             outside the GPS, this exception is optional.
    @action  Implementation-dependent.
    @see     AFPC-0008-03, p. 169. */
#define GOCA_EC_RELLINE_OUTSIDE_GPS     0xE100  /* Impl-dependent           */

/* ======================================================================== */
/*  Partial Arc (GPARC / GCPARC)                                            */
/* ======================================================================== */

/*! @brief A partial arc starts inside the GPS but ends outside; the new
           current position is outside the GPS.
    @details The arc begins within the GPS, but its computed end point (and
             therefore the new current position) lies outside the GPS. For
             devices that can maintain a position outside the GPS, this
             exception is optional.
    @action  Implementation-dependent.
    @see     AFPC-0008-03, p. 169. */
#define GOCA_EC_PARC_ENDS_OUTSIDE_GPS   0xE300  /* Impl-dependent           */

/*! @brief Negative SWEEP value in a Partial Arc.
    @details The SWEEP angle parameter is negative, which is not permitted.
    @action  Drawing process check.
    @see     AFPC-0008-03, p. 169. */
#define GOCA_EC_PARC_NEGATIVE_SWEEP     0xE302  /* Drawing check            */

/*! @brief Negative START value in a Partial Arc.
    @details The START angle parameter is negative, which is not permitted.
    @action  Drawing process check.
    @see     AFPC-0008-03, p. 169. */
#define GOCA_EC_PARC_NEGATIVE_START     0xE303  /* Drawing check            */

/*! @brief An exceptional situation may prevent the arc from being drawn
           with normal accuracy (for Partial Arc).
    @details The arc parameters or environment may degrade the achievable
             accuracy.
    @action  Standard action: draw the arc as accurately as possible.
    @see     outputlinks SC31-6804, p. 232. */
#define GOCA_EC_PARC_ACCURACY           0xE301  /* Std: draw as accurately  */

/* ======================================================================== */
/*  Vector Symbol / End Symbol                                              */
/* ======================================================================== */

/*! @brief An invalid order inside a vector symbol definition.
    @details A supported order appears within a vector symbol definition
             where it is not permitted.
    @action  Drawing process check.
    @see     AFPC-0008-03, p. 168. */
#define GOCA_EC_INVALID_VECTOR_SYMBOL   0x000F  /* Drawing check            */

/*! @brief End of Symbol Definition executed outside a symbol definition.
    @details An End of Symbol Definition order appears when no symbol
             definition is active.
    @action  Drawing process check.
    @see     outputlinks SC31-6804, p. 229. */
#define GOCA_EC_ESD_OUTSIDE_SYMBOL      0xFF00  /* Drawing check            */

/* ======================================================================== */
/*  Bitblt (PM GPI extension)                                               */
/* ======================================================================== */

#define GOCA_EC_BITBLT_SRC_PATTERN      0xD600  /* Drawing check            */
#define GOCA_EC_BITBLT_SRC_RECT         0xD601  /* Drawing check            */
#define GOCA_EC_BITBLT_INVALID_MIX      0xD602  /* Drawing check            */
#define GOCA_EC_BITBLT_INVALID_TRANS    0xD603  /* Drawing check            */
#define GOCA_EC_BITBLT_ROTATION         0xD604  /* Std: bounding rect       */
#define GOCA_EC_BITBLT_BMID_UNAVAILABLE 0xD605  /* Drawing check            */

/* ======================================================================== */
/*  Element (GBEL / GEEL — PM GPI extension)                                */
/* ======================================================================== */

#define GOCA_EC_ELEMENT_NESTED          0xD200  /* Drawing check            */
#define GOCA_EC_ELEMENT_UNCLOSED        0xD201  /* Drawing check            */
#define GOCA_EC_ORDER_INVALID_IN_ELEMENT 0xD202 /* Drawing check            */

/* ======================================================================== */
/*  Escape / Registered Identifier                                          */
/* ======================================================================== */

/*! @brief Unregistered identifier with type X'80'.
    @details An escape order carries a type of X'80' and an identifier that
             is not registered.
    @action  Drawing process check.
    @see     outputlinks SC31-6804, p. 229. */
#define GOCA_EC_ESCAPE_UNREG_ID         0xD500  /* Drawing check            */

/* ======================================================================== */
/*  Modify Path                                                             */
/* ======================================================================== */

/*! @brief Modify Path caused storage overflow due to path complexity.
    @details The path to be modified is too complex and exhausts available
             storage.
    @action  Drawing process check.
    @see     outputlinks SC31-6804, p. 229. */
#define GOCA_EC_MODIFYPATH_OVERFLOW     0xD800  /* Drawing check            */

/* ======================================================================== */
/*  Helper Functions (inline)                                               */
/* ======================================================================== */

/*! @brief Returns TRUE if the exception belongs to the
           "Drawing Process Check" category (drawing must stop). */
static __inline int GocaEcIsDrawingCheck(USHORT usEC)
{
    switch (usEC) {
    /* General */
    case GOCA_EC_RESERVED_NOT_ZERO:
    case GOCA_EC_INVALID_LENGTH:
    case GOCA_EC_STACK_OVERFLOW:
    case GOCA_EC_TRUNCATED_ORDER:
    case GOCA_EC_INVALID_DESCRIPTOR:
    case GOCA_EC_INVALID_IN_PROLOG:
    case GOCA_EC_INVALID_SEGMENT_NAME:
    case GOCA_EC_INVALID_DRAW_CONTROL:
    case GOCA_EC_DYNAMIC_SEQ_CHECK:
    case GOCA_EC_SEGCHAR_OUTSIDE_PROLOG:
    case GOCA_EC_END_PROLOG_OUTSIDE:
    /* Area */
    case GOCA_EC_GEAR_WITHOUT_GBAR:
    case GOCA_EC_GBAR_NESTED:
    case GOCA_EC_GBAR_UNCLOSED:
    case GOCA_EC_ORDER_INVALID_IN_AREA:
    /* Image */
    case GOCA_EC_GIMD_WITHOUT_GBIMG:
    case GOCA_EC_GIMD_BAD_BYTE_COUNT:
    case GOCA_EC_GEIMG_WITHOUT_GBIMG:
    case GOCA_EC_IMAGE_ROW_COUNT_MISMATCH:
    case GOCA_EC_GBIMG_UNCLOSED:
    case GOCA_EC_ORDER_INVALID_IN_IMAGE:
    case GOCA_EC_IMAGE_FORMAT_UNSUPPORTED:
    /* Path */
    case GOCA_EC_PATH_NESTED:
    case GOCA_EC_PATH_UNCLOSED:
    case GOCA_EC_ORDER_INVALID_IN_PATH:
    case GOCA_EC_PATH_ID_ZERO:
    /* Partial Arc */
    case GOCA_EC_PARC_NEGATIVE_SWEEP:
    case GOCA_EC_PARC_NEGATIVE_START:
    /* Vector symbol */
    case GOCA_EC_INVALID_VECTOR_SYMBOL:
    case GOCA_EC_ESD_OUTSIDE_SYMBOL:
    /* Bitblt (PM) */
    case GOCA_EC_BITBLT_SRC_PATTERN:
    case GOCA_EC_BITBLT_SRC_RECT:
    case GOCA_EC_BITBLT_INVALID_MIX:
    case GOCA_EC_BITBLT_INVALID_TRANS:
    case GOCA_EC_BITBLT_BMID_UNAVAILABLE:
    /* Element (PM) */
    case GOCA_EC_ELEMENT_NESTED:
    case GOCA_EC_ELEMENT_UNCLOSED:
    case GOCA_EC_ORDER_INVALID_IN_ELEMENT:
    /* Escape / Modify Path */
    case GOCA_EC_ESCAPE_UNREG_ID:
    case GOCA_EC_MODIFYPATH_OVERFLOW:
        return 1;
    default:
        return 0;
    }
}

/*! @brief Returns TRUE if a standard action is defined for the exception
           (drawing continues). */
static __inline int GocaEcHasStandardAction(USHORT usEC)
{
    return !GocaEcIsDrawingCheck(usEC) && usEC != 0x0000;
}

/*! @brief Returns a static string with the short name of the exception.
    @note  For unknown codes, returns "EC-????". */
const char *GocaEcName(USHORT usEC);

#ifdef __cplusplus
}
#endif

#endif /* GOCA_EXCEPTIONS_H */
