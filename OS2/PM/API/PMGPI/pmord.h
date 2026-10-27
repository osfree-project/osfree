/*!
   @file PMORD.H

   @brief OS/2 Presentation Manager GPI — GOCA order structures and constants.

   Defines the binary layouts, order codes, and flag constants used to build
   and interpret Graphics Object Content Architecture (GOCA) drawing orders
   in the OS/2 Presentation Manager GPI environment.

   The order encoding follows the GOCA architecture described in:

   @see "Graphics Object Content Architecture for Advanced Function
         Presentation Reference", AFPC-0008-03, Fourth Edition, April 2017.
         Chapter 7, "Commands and Drawing Orders", page 73.
   @see AFP GOCA Reference, "Order Formats", page 77 (Fixed 1-byte,
         Fixed 2-byte, Long, and Extended formats).
   @see AFP GOCA Reference, Chapter 3, "AFP GOCA Overview", page 9.
   @sa  https://www.afpcinc.org
*/

#pragma pack(1)

/*!
   @brief Fixed-point number with implicit binary point between the 1st and
          2nd hexadecimal digits (8.8 format).

   The high byte holds the integer part, the low byte holds the fractional
   part.  For example, 0x0180 represents 1.5.

   @see AFP GOCA Reference, "Line Width", page 32 (fractional line width
        encoding is analogous).
*/
typedef  USHORT FIXED88;

/*!
   @brief Fixed-point signed number with implicit binary point between bits
          14 and 13.

   Bit 15 is the sign bit.  Thus 1.0 is represented by 16384 (0x4000) and
          -1.0 is represented by -16384 (0xC000).

   @see AFP GOCA Reference, "Partial Arc (GPARC, GCPARC) Orders", page 119
        (multiplier MH/MFR encoding follows this convention).
*/
typedef  USHORT FIXED114;

/*!
   @brief Construct a FIXED value from integer and fractional parts.

   @param intpart  Integer part.
   @param fractpart Fractional part.

   @see AFP GOCA Reference, "Set Fractional Line Width (GSFLW) Order", page 147.
*/
#define MAKEFIXED(intpart,fractpart) MAKELONG(fractpart,intpart)

/*!
   @brief Extract the fractional part of a FIXED quantity.

   @see AFP GOCA Reference, "Set Fractional Line Width (GSFLW) Order", page 147.
*/
#define FIXEDFRAC(fx)                (LOUSHORT(fx))

/*!
   @brief Extract the integer part of a FIXED quantity.

   @see AFP GOCA Reference, "Set Fractional Line Width (GSFLW) Order", page 147.
*/
#define FIXEDINT(fx)                 ((SHORT)HIUSHORT(fx))


/***************************************************************************\
*
* Miscellaneous structures used in this file
*
\***************************************************************************/

/*!
   @brief Rectangle with SHORT coordinates (GOCA 16-bit signed format).

   Mirrors the coordinate data format of AFP GOCA orders, where each point
   is specified by a pair of 16-bit two's-complement signed binary integers
   (SBIN), high byte first.

   @see AFP GOCA Reference, "Coordinate Data", page 79.
*/
typedef struct _RECT1S
{
   SHORT xLeft;
   SHORT yBottom;
   SHORT xRight;
   SHORT yTop;
} RECT1S;

/*!
   @brief Point with 1-byte offsets (GOCA offset data format).

   Used by relative-line orders where the endpoint of each line is an 8-bit
   two's-complement signed offset from the previous endpoint.

   @see AFP GOCA Reference, "Offset Data", page 79.
   @see AFP GOCA Reference, "Relative Line (GRLINE, GCRLINE) Orders", page 127.
*/
typedef struct _ODPOINT
{
   CHAR   dx;
   CHAR   dy;
} ODPOINT;

/*!
   @brief Size with SHORT components.

   @see AFP GOCA Reference, "Coordinate Data", page 79.
*/
typedef struct _SIZES
{
   SHORT  cx;
   SHORT  cy;
} SIZES;

/*!
   @brief Unsigned two-byte swapped integer (high byte first).

   AFP GOCA multi-byte fields are stored high-byte-first; this structure
   provides explicit access to the two bytes.

   @see AFP GOCA Reference, "How to Read the Syntax Diagrams", page v
        (multi-byte fields and bit numbering).
*/
typedef struct _SWPUSHORT
{
   UCHAR  HiByte;
   UCHAR  LoByte;
} SWPUSHORT;

/***************************************************************************\
*
* 1-byte orders
*
\***************************************************************************/

/*!
   @brief Test whether an order code denotes a 1-byte order.

   @see AFP GOCA Reference, "Fixed 1-Byte Format", page 78.
*/
#define BYTE_ORDER(oc)   ((oc)==OCODE_GNOP1 || (oc)==OCODE_GESD)

/* 1-byte order codes */

/*!
   @brief No-Operation order (GNOP1).

   A null operation; has no effect on the GPS or any current attribute.
   @see AFP GOCA Reference, "No-Operation (GNOP1) Order", page 118.
*/
#define OCODE_GNOP1    0x00            /* No-operation                */

/*!
   @brief End Symbol Definition (PM GPI extension).

   This order is a Presentation Manager GPI extension and is not part of
   the AFP GOCA architecture.
*/
#define OCODE_GESD     0xFF            /* End symbol definition       */

/***************************************************************************\
*
* 2-byte orders
*
\***************************************************************************/

/*!
   @brief Bit masks used to identify 2-byte order codes.

   A 2-byte order has bit 0 = 0 and bit 4 = 1 in its order code; i.e. the
   first hex digit is less than 8 and the second is greater than or equal
   to 8.

   @see AFP GOCA Reference, "Fixed 2-Byte Format", page 78.
*/
#define OCODE2_1       0x80
#define OCODE2_2       0x88

/*!
   @brief Test whether an order code denotes a 2-byte order.

   @see AFP GOCA Reference, "Fixed 2-Byte Format", page 78.
*/
#define SHORT_ORDER(oc)  ((((oc)^OCODE2_1)&OCODE2_2)==OCODE2_2)

/*!
   @brief General 2-byte order structure.

   Layout: 1-byte order code followed by 1 byte of operand data.

   @see AFP GOCA Reference, "Fixed 2-Byte Format", page 78.
*/
typedef struct _ORDER
{
   UCHAR  idCode;
   UCHAR  uchData;
} ORDER;

/* 2-byte order codes */

/*!
   @brief Begin Area order (GBAR).

   Starts the definition of a filled area; must be terminated by an End Area
   order (GEAR).  The BOUNDARY bit controls whether the boundary is drawn;
   the INSIDE bit selects Alternate or Nonzero Winding mode.

   @see AFP GOCA Reference, "Begin Area (GBAR) Order", page 82.
   @see AFP GOCA Reference, "Areas", page 36.
   @see AFP GOCA Reference, "Determining the Interior of an Area", Figure 18,
        page 37.
*/
#define OCODE_GBAR     0x68            /* Begin area                  */

/*!
   @brief Close Figure order (PM GPI extension).

   Not part of AFP GOCA; area figures are closed implicitly by End Area or
   by a Move Type order.
   @see AFP GOCA Reference, "Areas", page 36.
*/
#define OCODE_GCFIG    0x7D            /* Close figure                */

/*!
   @brief End Element order (PM GPI extension).

   Not part of AFP GOCA.
*/
#define OCODE_GEEL     0x49            /* End element                 */

/*!
   @brief End Path order (PM GPI extension).

   Not part of AFP GOCA.
*/
#define OCODE_GEPTH    0x7F            /* End path                    */

/*!
   @brief End Prolog order (GEPROL).

   Indicates the end of the prolog section of a segment.  Valid only within
   a prolog; otherwise raises EC-3E00.

   @see AFP GOCA Reference, "End Prolog (GEPROL) Order", page 104.
   @see AFP GOCA Reference, "Segment Prolog", page 62.
*/
#define OCODE_GEPROL   0x3E            /* End prologue                */

/*!
   @brief Pop order (PM GPI extension).

   Not part of AFP GOCA.
*/
#define OCODE_GPOP     0x3F            /* Pop                         */

/*!
   @brief Set Background Mix order (GSBMX).

   Shorthand for setting all four background-mix attributes (character,
   image, marker, pattern) to the same value.  In AFP GOCA only
   X'00' (drawing default) and X'05' (Leave Alone) are supported.

   @see AFP GOCA Reference, "Set Background Mix (GSBMX) Order", page 132.
   @see AFP GOCA Reference, "Mix", page 16.
*/
#define OCODE_GSBMX    0x0D            /* Set background mix          */

/*!
   @brief Push and Set Background Mix order (PM GPI extension).

   Not part of AFP GOCA.
*/
#define OCODE_GPSBMX   0x4D            /* Push & set b/g mix          */

/*!
   @brief Set Character Direction order (GSCD).

   Sets the direction in which characters are drawn: left-to-right (X'01'),
   top-to-bottom (X'02'), right-to-left (X'03'), or bottom-to-top (X'04').

   @see AFP GOCA Reference, "Set Character Direction (GSCD) Order", page 136.
   @see AFP GOCA Reference, "Character Strings", page 51.
*/
#define OCODE_GSCD     0x3A            /* Set char direction          */

/*!
   @brief Push and Set Character Direction order (PM GPI extension).

   Not part of AFP GOCA.
*/
#define OCODE_GPSCD    0x7A            /* Push & set char direction   */

/*!
   @brief Set Character Precision order (GSCR).

   Sets the precision attribute: X'01' (string precision), X'02' (character
   precision).  X'03' (stroke precision) is not supported in AFP GOCA.

   @see AFP GOCA Reference, "Set Character Precision (GSCR) Order", page 138.
   @see AFP GOCA Reference, "Character Strings", page 51.
*/
#define OCODE_GSCR     0x39            /* Set char precision          */

/*!
   @brief Push and Set Character Precision order (PM GPI extension).

   Not part of AFP GOCA.
*/
#define OCODE_GPSCR    0x79            /* Push & set char precision   */

/*!
   @brief Set Character Set order (GSCS).

   Sets the current character set attribute (LCID).  X'00' selects the
   drawing default; X'01'-X'FE' select environment-mapped character sets;
   X'FF' selects the special character set.

   @see AFP GOCA Reference, "Set Character Set (GSCS) Order", page 140.
*/
#define OCODE_GSCS     0x38            /* Set char set                */

/*!
   @brief Push and Set Character Set order (PM GPI extension).

   Not part of AFP GOCA.
*/
#define OCODE_GPSCS    0x78            /* Push & set char set         */

/*!
   @brief Set Color order (GSCOL).

   Shorthand for setting all five foreground-color attributes (character,
   image, line, marker, pattern) to the same value.  The value is prefixed
   with X'FF' to form a two-byte index into the Standard Color Table.

   @see AFP GOCA Reference, "Set Color (GSCOL) Order", page 142.
   @see AFP GOCA Reference, "Standard Color Table", page 15.
*/
#define OCODE_GSCOL    0x0A            /* Set color                   */

/*!
   @brief Push and Set Color order (PM GPI extension).

   Not part of AFP GOCA.
*/
#define OCODE_GPSCOL   0x4A            /* Push & set color            */

/*!
   @brief Set Line End order (GSLE).

   Sets the line-end shape: X'00' drawing default, X'01' flat, X'02' square,
   X'03' round.  Applies to the extreme ends of contiguous stroked lines and
   to the internal ends of dots/dashes.

   @see AFP GOCA Reference, "Set Line End (GSLE) Order", page 148.
   @see AFP GOCA Reference, "Line End and Line Join", page 32.
*/
#define OCODE_GSLE     0x1A            /* Set line end                */

/*!
   @brief Push and Set Line End order (PM GPI extension).

   Not part of AFP GOCA.
*/
#define OCODE_GPSLE    0x5A            /* Push & set line end         */

/*!
   @brief Set Line Join order (GSLJ).

   Sets the line-join shape: X'00' drawing default, X'01' bevel, X'02' round,
   X'03' miter.

   @see AFP GOCA Reference, "Set Line Join (GSLJ) Order", page 149.
   @see AFP GOCA Reference, "Line End and Line Join", page 32.
*/
#define OCODE_GSLJ     0x1B            /* Set line join               */

/*!
   @brief Push and Set Line Join order (PM GPI extension).

   Not part of AFP GOCA.
*/
#define OCODE_GPSLJ    0x5B            /* Push & set line join        */

/*!
   @brief Set Line Type order (GSLT).

   Sets a standard line type: X'01' dotted, X'02' short-dashed, X'03'
   dash-dot, X'04' double-dotted, X'05' long-dashed, X'06' dash-double-dot,
   X'07' solid, X'08' invisible.

   @see AFP GOCA Reference, "Set Line Type (GSLT) Order", page 150.
   @see AFP GOCA Reference, "Line Type", page 29.
*/
#define OCODE_GSLT     0x18            /* Set line type               */

/*!
   @brief Push and Set Line Type order (PM GPI extension).

   Not part of AFP GOCA.
*/
#define OCODE_GPSLT    0x58            /* Push & set line type        */

/*!
   @brief Set Line Width order (GSLW).

   Sets the integral multiplier of the normal line width.  Resets the
   fractional part of the line-width attribute to zero.

   @see AFP GOCA Reference, "Set Line Width (GSLW) Order", page 151.
   @see AFP GOCA Reference, "Line Width", page 32.
*/
#define OCODE_GSLW     0x19            /* Set line width              */

/*!
   @brief Push and Set Line Width order (PM GPI extension).

   Not part of AFP GOCA.
*/
#define OCODE_GPSLW    0x59            /* Push & set line width       */

/*!
   @brief Set Marker Precision order (GSMP) — obsolete.

   In AFP GOCA this order is obsolete; the marker precision attribute is
   obsolete.  Implementations that do not treat Set Marker Cell as a No-Op
   must draw markers taking into account all marker attributes.

   @see AFP GOCA Reference, Appendix C, "AFP GOCA Migration Functions",
        page 195.
   @see AFP GOCA Reference, "Set Marker Precision (GSMP) Order", page 196.
*/
#define OCODE_GSMP     0x3B            /* Set marker precision        */

/*!
   @brief Push and Set Marker Precision order (PM GPI extension).

   Not part of AFP GOCA.
*/
#define OCODE_GPSMP    0x7B            /* Push & set marker precision */

/*!
   @brief Set Marker Set order (GSMS).

   Sets the current marker-set attribute.  In AFP GOCA only X'00' (default
   marker set) is supported.

   @see AFP GOCA Reference, "Set Marker Set (GSMS) Order", page 154.
   @see AFP GOCA Reference, "Default Marker Set", Figure 32, page 57.
*/
#define OCODE_GSMS     0x3C            /* Set marker set              */

/*!
   @brief Push and Set Marker Set order (PM GPI extension).

   Not part of AFP GOCA.
*/
#define OCODE_GPSMS    0x7C            /* Push & set marker set       */

/*!
   @brief Set Marker Symbol order (GSMT).

   Sets the current marker-symbol attribute.  In the default marker set:
   X'01' cross, X'02' plus, X'03' diamond, X'04' square, X'05' 6-point star,
   X'06' 8-point star, X'07' filled diamond, X'08' filled square, X'09' dot,
   X'0A' small circle, X'40' blank.

   @see AFP GOCA Reference, "Set Marker Symbol (GSMT) Order", page 155.
   @see AFP GOCA Reference, "Markers", page 57.
*/
#define OCODE_GSMT     0x29            /* Set marker symbol           */

/*!
   @brief Push and Set Marker Symbol order (PM GPI extension).

   Not part of AFP GOCA.
*/
#define OCODE_GPSMT    0x69            /* Push & set marker symbol    */

/*!
   @brief Set Mix order (GSMX).

   Shorthand for setting all five foreground-mix attributes to the same
   value.  In AFP GOCA only X'00' (drawing default) and X'02' (Overpaint)
   are supported.

   @see AFP GOCA Reference, "Set Mix (GSMX) Order", page 156.
   @see AFP GOCA Reference, "Mix", page 16.
*/
#define OCODE_GSMX     0x0C            /* Set mix                     */

/*!
   @brief Push and Set Mix order (PM GPI extension).

   Not part of AFP GOCA.
*/
#define OCODE_GPSMX    0x4C            /* Push & set mix              */

/*!
   @brief Set Pattern Set order (GSPS).

   Sets the current pattern-set attribute.  X'00' selects the default pattern
   set; X'01'-X'FD' select custom-pattern/gradient sets; X'FE' and X'FF' are
   not supported in AFP GOCA.

   @see AFP GOCA Reference, "Set Pattern Set (GSPS) Order", page 158.
   @see AFP GOCA Reference, "Patterns", page 39.
*/
#define OCODE_GSPS     0x08            /* Set pattern set             */

/*!
   @brief Push and Set Pattern Set order (PM GPI extension).

   Not part of AFP GOCA.
*/
#define OCODE_GPSPS    0x48            /* Push & set pattern set      */

/*!
   @brief Set Pattern Symbol order (GSPT).

   Sets the current pattern-symbol attribute.  In the default pattern set:
   X'01'-X'08' predefined patterns, X'09' vertical lines, X'0A' horizontal
   lines, X'0B'-X'0E' diagonal lines, X'0F' no fill, X'10' solid fill,
   X'40' blank (same as no fill).

   @see AFP GOCA Reference, "Set Pattern Symbol (GSPT) Order", page 159.
   @see AFP GOCA Reference, "Patterns in the Default Pattern Set", page 39.
*/
#define OCODE_GSPT     0x28            /* Set pattern symbol          */

/*!
   @brief Push and Set Pattern Symbol order (PM GPI extension).

   Not part of AFP GOCA.
*/
#define OCODE_GPSPT    0x09            /* Push & set pattern symbol   */

/* constants for 2-byte orders */

/*!
   @brief Begin Area order flag bits.

   RES1 (bit 0) must be B'1' for migration compatibility; BOUNDARY (bit 1)
   controls whether the boundary is drawn; INSIDE (bit 2) selects Alternate
   mode (B'0') or Nonzero Winding mode (B'1').

   @see AFP GOCA Reference, "Begin Area (GBAR) Order — Syntax", page 82.
*/
#define GBAR_RESERVED   0x80
#define GBAR_BOUNDARY   0xC0
#define GBAR_NOBOUNDARY 0x80
#define GBAR_WINDING    0xA0
#define GBAR_ALTERNATE  0x80

/*!
   @brief Set Character Precision order mask.

   Selects the precision field within the 2-byte operand.

   @see AFP GOCA Reference, "Set Character Precision (GSCR) Order", page 138.
*/
/* Set Character Precision */
#define GSCR_PRECISION  0x0F

/***************************************************************************\
*
* Long orders
*
\***************************************************************************/

/*!
   @brief Order code introducing an extended (very long) order.

   @see AFP GOCA Reference, "Extended Format", page 78.
*/
#define OCODE_VLONG    0xFE

/*!
   @brief Test whether an order code denotes a long-format order.

   @see AFP GOCA Reference, "Long Format", page 78.
*/
#define LONG_ORDER(oc) (!((oc)==OCODE_VLONG||BYTE_ORDER(oc)||SHORT_ORDER(oc)))

/*!
   @brief Long-format order structure.

   Layout: 1-byte order code, 1-byte length field, then up to 255 bytes of
   operand data.  The length field gives the operand length in bytes
   (excluding the code and length fields).

   @see AFP GOCA Reference, "Long Format", page 78.
*/
#define LORDER_ML 253
typedef struct _LORDER
{
   UCHAR  idCode;
   UCHAR  uchLength;
   UCHAR  uchData[LORDER_ML];
} LORDER;

/* Long orders for which the length of data is normally zero */

/*!
   @brief End Area order (GEAR).

   Indicates the end of an area definition.  The data bytes must be X'00'.

   @see AFP GOCA Reference, "End Area (GEAR) Order", page 101.
*/
#define OCODE_GEAR     0x60            /* End Area                    */

/*!
   @brief End Image order (GEIMG).

   Identifies the end of an image definition.  The data bytes must be X'00'.

   @see AFP GOCA Reference, "End Image (GEIMG) Order", page 103.
*/
#define OCODE_GEIMG    0x93            /* End Image                   */

/* Long orders for which the data is contained in a type already defined */

/* Character String */

/*!
   @brief Character String at Current Position order (GCCHST).

   Draws a character string starting at the current position.  Current
   position is unchanged.

   @see AFP GOCA Reference, "Character String (GCHST, GCCHST) Orders",
        page 92.
*/
#define OCODE_GCCHST   0x83            /* char string at curr posn    */
#define GCCHST_MC      255             /* Max len of string in bytes  */

/*!
   @brief Character String at Given Position order (GCHST).

   Draws a character string starting at a given (X,Y) position.  Current
   position is set to (X,Y).

   @see AFP GOCA Reference, "Character String (GCHST, GCCHST) Orders",
        page 92.
*/
#define OCODE_GCHST    0xC3            /* char string at given pos    */
#define GCHST_SMC      251             /* Max len of string (S)       */
#define GCHST_LMC      247             /* Max len of string (L)       */

/* Character String Move */

/*!
   @brief Character String Move at Current Position order (PM GPI extension).

   Not part of AFP GOCA.
*/
#define OCODE_GCCHSTM  0xB1            /* char string move at c.p.    */
#define GCCHSTM_MC     255             /* Max len of string in byte   */

/*!
   @brief Character String Move at Given Position order (PM GPI extension).

   Not part of AFP GOCA.
*/
#define OCODE_GCHSTM   0xF1            /* char string move at g.p.    */
#define GCHSTM_SMC     251             /* Max len of string (S)       */
#define GCHSTM_LMC     247             /* Max len of string (L)       */

/* Comment */

/*!
   @brief Comment order (GCOMT).

   Treated as a No-Op; the data is stored within a segment and ignored.

   @see AFP GOCA Reference, "Comment (GCOMT) Order", page 95.
*/
#define OCODE_GCOMT    0x01            /* Comment                     */
#define GCOMT_ML       255             /* Maximum len of comment data */

/* Image */

/*!
   @brief Image Data order (GIMD).

   Specifies the raster data for one scan line/row of an image.  Each order
   carries 0–255 bytes of data.

   @see AFP GOCA Reference, "Image Data (GIMD) Order", page 109.
   @see AFP GOCA Reference, "Begin Image (GBIMG, GCBIMG) Orders", page 87.
*/
#define OCODE_GIMD     0x92            /* Image data                  */
#define GIMD_ML        255             /* Maximum len of image data   */

/* Full Arc */

/*!
   @brief Full Arc at Current Position order (GCFARC).

   Constructs a full circle or ellipse with its center at the current
   position.  Shape and orientation are determined by the current arc
   parameters (Set Arc Parameters order).

   @see AFP GOCA Reference, "Full Arc (GFARC, GCFARC) Orders", page 107.
   @see AFP GOCA Reference, "Full Arc", page 24.
*/
#define OCODE_GCFARC   0x87            /* full arc at current posn    */

/*!
   @brief Full Arc at Given Position order (GFARC).

   Constructs a full circle or ellipse with its center at a given (X,Y)
   position.

   @see AFP GOCA Reference, "Full Arc (GFARC, GCFARC) Orders", page 107.
*/
#define OCODE_GFARC    0xC7            /* full arc at given posn      */

/* Label */

/*!
   @brief Label order (PM GPI extension).

   Not part of AFP GOCA.
*/
#define OCODE_GLABL    0xD3            /* Label                       */

/* Set Current Position */

/*!
   @brief Set Current Position order (GSCP).

   Sets the current position to the specified (Xg, Yg) coordinate.

   @see AFP GOCA Reference, "Set Current Position (GSCP) Order", page 143.
   @see AFP GOCA Reference, "Current Position", page 19.
*/
#define OCODE_GSCP     0x21            /* Set current position        */

/*!
   @brief Push and Set Current Position order (PM GPI extension).

   Not part of AFP GOCA.
*/
#define OCODE_GPSCP    0x61            /* Push and set curr posn      */

/* Bezier spline */

/*!
   @brief Cubic Bezier Curve at Current Position order (GCCBEZ).

   Draws a third-order Bezier curve starting at the current position.
   The number of points (including the current position) must be 3m+1.

   @see AFP GOCA Reference, "Cubic Bezier Curve (GCBEZ, GCCBEZ) Orders",
        page 96.
   @see AFP GOCA Reference, "Cubic Bezier Curve", page 27.
*/
#define OCODE_GCBEZ    0xA5            /* Bezier spline at curr pos   */
#define GCBEZ_SMB      21              /* Max number of splines (S)   */
#define GCBEZ_LMB      10              /* Max number of splines (L)   */

/*!
   @brief Cubic Bezier Curve at Given Position order (GCBEZ in AFP GOCA
          naming; GBÅÇ in PMORD.H).

   Draws a third-order Bezier curve starting at a given position.

   @see AFP GOCA Reference, "Cubic Bezier Curve (GCBEZ, GCCBEZ) Orders",
        page 96.
*/
#define OCODE_GBEZ     0xE5            /* Bezier spline at given pos  */
#define GBEZ_SMB       20              /* Max number of splines (S)   */
#define GBEZ_LMB       10              /* Max number of splines (L)   */

/* Fillet */

/*!
   @brief Fillet at Current Position order (GCFLT).

   Draws a curve tangential to a set of straight lines, starting at the
   current position.

   @see AFP GOCA Reference, "Fillet (GFLT, GCFLT) Orders", page 105.
   @see AFP GOCA Reference, "Fillet", page 26.
*/
#define OCODE_GCFLT    0x85            /* fillet at current posn      */
#define GCFLT_SMP      63              /* Max number of points (S)    */
#define GCFLT_LMP      31              /* Max number of points (L)    */

/*!
   @brief Fillet at Given Position order (GFLT).

   Draws a curve tangential to a set of straight lines, starting at a given
   position.

   @see AFP GOCA Reference, "Fillet (GFLT, GCFLT) Orders", page 105.
*/
#define OCODE_GFLT     0xC5            /* fillet at given position    */
#define GFLT_SMP       62              /* Max number of points (S)    */
#define GFLT_LMP       30              /* Max number of points (L)    */

/* Polyline */

/*!
   @brief Line at Current Position order (GCLINE).

   Draws one or more connected straight lines starting at the current
   position.  Current position is set to the last point specified.

   @see AFP GOCA Reference, "Line (GLINE, GCLINE) Orders", page 110.
   @see AFP GOCA Reference, "Straight Lines", page 22.
*/
#define OCODE_GCLINE   0x81            /* polyline at current posn    */
#define GCLINE_SMP     63              /* Max number of points (S)    */
#define GCLINE_LMP     31              /* Max number of points (L)    */

/*!
   @brief Line at Given Position order (GLINE).

   Draws one or more connected straight lines starting at a given position.

   @see AFP GOCA Reference, "Line (GLINE, GCLINE) Orders", page 110.
*/
#define OCODE_GLINE    0xC1            /* polyline at given posn      */
#define GLINE_SMP      62              /* Max number of points (S)    */
#define GLINE_LMP      30              /* Max number of points (L)    */

/* Polymarker */

/*!
   @brief Marker at Current Position order (GCMRK).

   Draws the current marker symbol at the current position and at one or
   more further points.  Current position is set to the last coordinate
   specified.

   @see AFP GOCA Reference, "Marker (GMRK, GCMRK) Orders", page 116.
   @see AFP GOCA Reference, "Markers", page 57.
*/
#define OCODE_GCMRK    0x82            /* marker at current posn      */
#define GCMRK_SMP      63              /* Max number of points (S)    */
#define GCMRK_LMP      31              /* Max number of points (L)    */

/*!
   @brief Marker at Given Position order (GMRK).

   Draws the current marker symbol at a given position and at one or more
   further points.

   @see AFP GOCA Reference, "Marker (GMRK, GCMRK) Orders", page 116.
*/
#define OCODE_GMRK     0xC2            /* marker at given posn        */
#define GMRK_SMP       62              /* Max number of points (S)    */
#define GMRK_LMP       30              /* Max number of points (L)    */

/* Relative Line */

/*!
   @brief Relative Line at Current Position order (GCRLINE).

   Draws one or more connected straight lines using 1-byte signed offsets
   from the current position.  Current position is set to the final
   computed endpoint.

   @see AFP GOCA Reference, "Relative Line (GRLINE, GCRLINE) Orders",
        page 127.
*/
#define OCODE_GCRLINE  0xA1            /* Relative line at curr pos   */
#define GCRLINE_MP     127             /* Max number of points        */

/*!
   @brief Relative Line at Given Position order (GRLINE).

   Draws one or more connected straight lines using 1-byte signed offsets
   from a given starting position.

   @see AFP GOCA Reference, "Relative Line (GRLINE, GCRLINE) Orders",
        page 127.
*/
#define OCODE_GRLINE   0xE1            /* Relative line at givn pos   */
#define GRLINE_SMP     125             /* Max number of points (S)    */
#define GRLINE_LMP     123             /* Max number of points (L)    */

/* Set Background Color */

/*!
   @brief Set Background Color order (PM GPI extension).

   Not part of AFP GOCA.  AFP GOCA uses only the background-mix attribute
   (see Set Background Mix, GSBMX).
*/
#define OCODE_GSBCOL   0x25            /* Set background color        */

/*!
   @brief Push and Set Background Color order (PM GPI extension).

   Not part of AFP GOCA.
*/
#define OCODE_GPSBCOL  0x65            /* Push and set b/g color      */

/* Set Extended Color */

/*!
   @brief Set Extended Color order (GSECOL).

   Shorthand for setting all five foreground-color attributes to the same
   two-byte color index (Standard Color Table).

   @see AFP GOCA Reference, "Set Extended Color (GSECOL) Order", page 146.
   @see AFP GOCA Reference, "Standard Color Table", page 15.
*/
#define OCODE_GSECOL   0x26            /* Set extended color          */

/*!
   @brief Push and Set Extended Color order (PM GPI extension).

   Not part of AFP GOCA.
*/
#define OCODE_GPSECOL  0x66            /* Push and set ext color      */

/* Extended Color values */

/*!
   @brief Color index values for the Standard Color Table.

   SECOL_DEFAULT0 (X'0000') and SECOL_DEFAULT1 (X'FF00') select the drawing
   default; SECOL_NEUTRAL (X'FF07') selects the presentation-process default
   ("neutral white"); SECOL_RESET (X'FF08') selects the color of medium.

   @see AFP GOCA Reference, "Standard Color Table", page 15.
*/
#define SECOL_DEFAULT0  0x0000
#define SECOL_DEFAULT1  0xFF00
#define SECOL_NEUTRAL   0xFF07
#define SECOL_RESET     0xFF08

/* Set Character Angle */

/*!
   @brief Set Character Angle order (GSCA).

   Sets the angle of the character baseline with respect to the GPS Xg axis.
   In AFP GOCA only 0°, 90°, 180°, and 270° are supported.

   @see AFP GOCA Reference, "Set Character Angle (GSCA) Order", page 133.
*/
#define OCODE_GSCA     0x34            /* Set character angle         */

/*!
   @brief Push and Set Character Angle order (PM GPI extension).

   Not part of AFP GOCA.
*/
#define OCODE_GPSCA    0x74            /* Push and set char angle     */

/* Set Character Shear */

/*!
   @brief Set Character Shear order (GSCH).

   In AFP GOCA this order is processed as a No-Op (character shear is not
   supported).

   @see AFP GOCA Reference, "Set Character Shear (GSCH) Order", page 141.
*/
#define OCODE_GSCH     0x35            /* Set character shear         */

/*!
   @brief Push and Set Character Shear order (PM GPI extension).

   Not part of AFP GOCA.
*/
#define OCODE_GPSCH    0x75            /* Push and set char shear     */

/* Set Fractional Line Width */

/*!
   @brief Set Fractional Line Width order (GSFLW).

   Sets the line-width attribute including a fractional multiplier of the
   normal line width (MH.MFR).

   @see AFP GOCA Reference, "Set Fractional Line Width (GSFLW) Order",
        page 147.
   @see AFP GOCA Reference, "Line Width", page 32.
*/
#define OCODE_GSFLW    0x11            /* Set fractional line width   */

/*!
   @brief Push and Set Fractional Line Width order (PM GPI extension).

   Not part of AFP GOCA.
*/
#define OCODE_GPSFLW   0x51            /* Push and set frac l width   */

/* Set Pick Identifier */

/*!
   @brief Set Pick Identifier order (PM GPI extension; tolerated by AFP GOCA).

   Some AFP printers accept this long-format drawing order and process it
   as a No-Op.  It is not formally part of AFP GOCA.

   @see AFP GOCA Reference, "Summary List of Orders", page 80.
*/
#define OCODE_GSPIK    0x43            /* Set pick identifier         */

/*!
   @brief Push and Set Pick Identifier order (PM GPI extension).

   Not part of AFP GOCA.
*/
#define OCODE_GPSPIK   0x23            /* Push and set pick id        */


/* Long Orders for which a structure can be defined for the data */

/* Arc */

/*!
   @brief Arc at Current Position order (PM GPI extension; superseded in
          AFP GOCA by Partial Arc / Full Arc orders).

   Not part of AFP GOCA.  Use GCPARC/GPARC (Partial Arc) or
   GCFARC/GFARC (Full Arc) instead.

   @see AFP GOCA Reference, "Full Arc (GFARC, GCFARC) Orders", page 107.
   @see AFP GOCA Reference, "Partial Arc (GPARC, GCPARC) Orders", page 119.
*/
#define OCODE_GCARC    0x86            /* Arc at Current Position     */

/*!
   @brief Arc at Given Position order (PM GPI extension).

   Not part of AFP GOCA.
*/
#define OCODE_GARC     0xC6            /* Arc at Given Position       */

/*!
   @brief Structure for Arc order parameters (PM GPI extension).

   @see AFP GOCA Reference, "Partial Arc (GPARC, GCPARC) Orders", page 119
        (for the AFP GOCA equivalent).
*/
typedef struct _ORDERS_GCARC
{
   POINTS ptInter;
   POINTS ptEnd;
} ORDERS_GCARC;

/*!
   @brief 32-bit structure for Arc order parameters (PM GPI extension).
*/
typedef struct _ORDERL_GCARC     /* olgcarc */
{
   POINTL ptInter;
   POINTL ptEnd;
} ORDERL_GCARC;

/* Begin Element */

/*!
   @brief Begin Element order (PM GPI extension).

   Not part of AFP GOCA.
*/
#define OCODE_GBEL     0xD2            /* Begin Element               */

#define GBEL_DL        251

/*!
   @brief Structure for Begin Element order (PM GPI extension).
*/
typedef struct _ORDER_GBEL
{
   LONG   lElementType;
   CHAR   achDesc[GBEL_DL];
} ORDER_GBEL;

/* Begin Image */

/*!
   @brief Begin Image at Current Position order (GCBIMG).

   Starts an image definition at the current position.  Must be followed by
   one or more Image Data orders (GIMD) and terminated by an End Image order
   (GEIMG).

   @see AFP GOCA Reference, "Begin Image (GBIMG, GCBIMG) Orders", page 87.
*/
#define OCODE_GCBIMG   0x91            /* Begin Image at curr posn    */

/*!
   @brief Begin Image at Given Position order (GBIMG).

   Starts an image definition at a given (X,Y) origin.

   @see AFP GOCA Reference, "Begin Image (GBIMG, GCBIMG) Orders", page 87.
*/
#define OCODE_GBIMG    0xD1            /* Begin Image at given posn   */

/*!
   @brief Structure for Begin Image at Current Position order (GCBIMG).

   Layout: FORMAT (X'00'), RES (X'00'), WIDTH, HEIGHT.

   @see AFP GOCA Reference, "Begin Image at Current Position (GCBIMG) Order
        — Syntax", page 87.
*/
typedef struct _ORDER_GCBIMG     /* ogbimg */
{
   UCHAR     uchFormat;
   UCHAR     uchReserved;
   SWPUSHORT cx;
   SWPUSHORT cy;
} ORDER_GCBIMG;

/* Begin Path */

/*!
   @brief Begin Path order (PM GPI extension).

   Not part of AFP GOCA.
*/
#define OCODE_GBPTH    0xD0            /* Begin Path                  */

/*!
   @brief Structure for Begin Path order (PM GPI extension).
*/
typedef struct _ORDER_GBPTH      /* ogbpth */
{
   USHORT usReserved;
   LONG   idPath;
} ORDER_GBPTH;

/* Box */

/*!
   @brief Box at Current Position order (GCBOX).

   Draws a rectangular box with square or rounded corners; the first corner
   is at the current position, the diagonally opposite corner is specified.

   @see AFP GOCA Reference, "Box (GBOX, GCBOX) Orders", page 90.
   @see AFP GOCA Reference, "Boxes", page 28.
*/
#define OCODE_GCBOX    0x80            /* Box at current position     */

/*!
   @brief Box at Given Position order (GBOX).

   Draws a rectangular box with square or rounded corners; both corners are
   specified.

   @see AFP GOCA Reference, "Box (GBOX, GCBOX) Orders", page 90.
*/
#define OCODE_GBOX     0xC0            /* Box at given position       */

/*!
   @brief Structure for Box at Current Position order (GCBOX).

   Layout: flags, reserved, diagonal corner (POINTS), optional HAXIS and
   VAXIS.

   @see AFP GOCA Reference, "Box at Current Position (GCBOX) Order —
        Syntax", page 90.
*/
typedef struct _ORDERS_GCBOX
{
   UCHAR  fbFlags;
   UCHAR  uchReserved;
   POINTS ptCorner;
   SHORT  hAxis;
   SHORT  vAxis;
} ORDERS_GCBOX;

/*!
   @brief 32-bit structure for Box at Current Position order (GCBOX).
*/
typedef struct _ORDERL_GCBOX
{
   UCHAR  fbFlags;
   UCHAR  uchReserved;
   POINTL ptCorner;
   LONG   hAxis;
   LONG   vAxis;
} ORDERL_GCBOX;

/*!
   @brief Box order flag: fill the box interior (PM GPI extension).

   @see AFP GOCA Reference, "Box (GBOX, GCBOX) Orders", page 90.
*/
#define GCBOX_FILL     0x40

/*!
   @brief Box order flag: draw the box boundary.
*/
#define GCBOX_BOUNDARY 0x20

/* Call Segment */

/*!
   @brief Call Segment order (PM GPI extension).

   AFP GOCA does not support the calling of segments; all segments are
   processed in immediate mode and chained.

   @see AFP GOCA Reference, "Segment Types", page 61.
*/
#define OCODE_GCALLS   0x07            /* call segment                */

/*!
   @brief Structure for Call Segment order (PM GPI extension).
*/
typedef struct _ORDER_GCALLS
{
   USHORT sReserved;
   LONG   idSegment;
} ORDER_GCALLS;

/* Fill Path */

/*!
   @brief Fill Path order (PM GPI extension).

   Not part of AFP GOCA.
*/
#define OCODE_GFPTH    0xD7            /* Fill path                   */

/*!
   @brief Structure for Fill Path order (PM GPI extension).
*/
typedef struct _ORDER_GFPTH
{
   UCHAR  fbFlags;
   UCHAR  uchReserved;
   LONG   idPath;
} ORDER_GFPTH;

/*!
   @brief Fill Path flag: Alternate mode.
*/
#define GFPTH_ALTERNATE 0x00

/*!
   @brief Fill Path flag: Nonzero Winding mode.

   @see AFP GOCA Reference, "Nonzero Winding mode", page 37.
*/
#define GFPTH_WINDING   0x40

/*!
   @brief Fill Path flag: modify path (PM GPI extension).
*/
#define GFPTH_MODIFY    0x20

/* Outline Path */

/*!
   @brief Outline Path order (PM GPI extension).

   Not part of AFP GOCA.
*/
#define OCODE_GOPTH    0xD4            /* Outline Path                */

/*!
   @brief Structure for Outline Path order (PM GPI extension).
*/
typedef struct _ORDER_GOPTH
{
   UCHAR  fbFlags;
   UCHAR  uchReserved;
   LONG   idPath;
} ORDER_GOPTH;

/* Modify Path */

/*!
   @brief Modify Path order (PM GPI extension).

   Not part of AFP GOCA.
*/
#define OCODE_GMPTH 0xD8               /* modify path                 */

/*!
   @brief Structure for Modify Path order (PM GPI extension).
*/
typedef struct _ORDER_GMPTH
{
   UCHAR  uchMode;
   UCHAR  uchReserved;
   LONG   idPath;
} ORDER_GMPTH;

/*!
   @brief Modify Path mode: stroke (PM GPI extension).
*/
#define GMPTH_STROKE    0x06

/* Partial Arc */

/*!
   @brief Partial Arc at Current Position order (GCPARC).

   Draws a line from the current position to the start of an arc, then draws
   the arc.  Start and sweep angles are specified in unit-circle space.

   @see AFP GOCA Reference, "Partial Arc (GPARC, GCPARC) Orders", page 119.
   @see AFP GOCA Reference, "Partial Arc", page 25.
*/
#define OCODE_GCPARC   0xA3            /* Partial arc at curr posn    */

/*!
   @brief Partial Arc at Given Position order (GPARC).

   Draws a line from a given position to the start of an arc, then draws the
   arc.

   @see AFP GOCA Reference, "Partial Arc (GPARC, GCPARC) Orders", page 119.
*/
#define OCODE_GPARC    0xE3            /* Partial arc at given posn   */

/*!
   @brief Structure for Partial Arc at Current Position order (GCPARC).

   Layout: center (POINTS), multiplier (FIXED88), start angle, sweep angle.

   @see AFP GOCA Reference, "Partial Arc at Current Position (GCPARC) Order
        — Syntax", page 119.
*/
typedef struct _ORDERS_GCPARC
{
   POINTS   ptCenter;
   FIXED88  ufx88Multiplier;
   LONG     usStartAngle;
   LONG     usSweepAngle;
} ORDERS_GCPARC;

/*!
   @brief 32-bit structure for Partial Arc at Current Position order
          (GCPARC).
*/
typedef struct _ORDERL_GCPARC
{
   POINTL   ptCenter;
   FIXED    ufxMultiplier;
   LONG     usStartAngle;
   LONG     usSweepAngle;
} ORDERL_GCPARC;

/* Set Clip Path */

/*!
   @brief Set Clip Path order (PM GPI extension).

   Not part of AFP GOCA.
*/
#define OCODE_GSCPTH   0xB4            /* Set clip path               */

/*!
   @brief Structure for Set Clip Path order (PM GPI extension).
*/
typedef struct _ORDER_GSCPTH
{
   UCHAR  fbFlags;
   UCHAR  uchReserved;
   LONG   idPath;
} ORDER_GSCPTH;

/*!
   @brief Set Clip Path flag: Alternate mode.
*/
#define GSCPTH_ALTERNATE 0x00

/*!
   @brief Set Clip Path flag: Nonzero Winding mode.

   @see AFP GOCA Reference, "Nonzero Winding mode", page 37.
*/
#define GSCPTH_WINDING   0x40

/*!
   @brief Set Clip Path flag: reset (PM GPI extension).
*/
#define GSCPTH_RESET     0x00

/*!
   @brief Set Clip Path flag: intersect (PM GPI extension).
*/
#define GSCPTH_INTERSECT 0x20

/* Set Arc Parameters */

/*!
   @brief Set Arc Parameters order (GSAP).

   Sets the current arc parameters P, Q, R, S.  These define a transforma-
   tion that maps the unit circle to the required ellipse.

   @see AFP GOCA Reference, "Set Arc Parameters (GSAP) Order", page 130.
   @see AFP GOCA Reference, "Full Arc", page 24.
*/
#define OCODE_GSAP     0x22            /* Set arc parameters          */

/*!
   @brief Push and Set Arc Parameters order (PM GPI extension).

   Not part of AFP GOCA.
*/
#define OCODE_GPSAP    0x62            /* Push and set arc params     */

/*!
   @brief Structure for Set Arc Parameters order (GSAP).

   Layout: P, Q, R, S (16-bit signed integers).

   @see AFP GOCA Reference, "Set Arc Parameters (GSAP) Order — Syntax",
        page 130.
*/
typedef struct _ORDERS_GSAP
{
   SHORT  p;
   SHORT  q;
   SHORT  r;
   SHORT  s;
} ORDERS_GSAP;

/*!
   @brief 32-bit structure for Set Arc Parameters order (GSAP).
*/
typedef struct _ORDERL_GSAP
{
   LONG   p;
   LONG   q;
   LONG   r;
   LONG   s;
} ORDERL_GSAP;

/* Set Background Indexed Color */

/*!
   @brief Set Background Indexed Color order (PM GPI extension).

   Not part of AFP GOCA.
*/
#define OCODE_GSBICOL  0xA7            /* Set b/g indexed color       */

/*!
   @brief Push and Set Background Indexed Color order (PM GPI extension).

   Not part of AFP GOCA.
*/
#define OCODE_GPSBICOL 0xE7            /* Push and set b/g ind color  */

/*!
   @brief Set Indexed Color order (PM GPI extension).

   Not part of AFP GOCA.
*/
#define OCODE_GSICOL   0xA6            /* Set indexed color           */

/*!
   @brief Push and Set Indexed Color order (PM GPI extension).

   Not part of AFP GOCA.
*/
#define OCODE_GPSICOL  0xE6            /* Push and set indexd color   */

/*!
   @brief Structure for Set Background Indexed Color order (PM GPI
          extension).
*/
typedef struct _ORDER_GSBICOL
{
   UCHAR  fbFlags;
   UCHAR  auchColor[3];
} ORDER_GSBICOL;

/*!
   @brief Set Indexed Color flag: specify (PM GPI extension).
*/
#define SICOL_SPECIFY  0x00

/*!
   @brief Set Indexed Color flag: special (PM GPI extension).
*/
#define SICOL_SPECIAL  0x40

/*!
   @brief Set Indexed Color flag: default (PM GPI extension).
*/
#define SICOL_DEFAULT  0x80

/*!
   @brief Set Indexed Color value: black (PM GPI extension).
*/
#define SICOL_BLACK    1

/*!
   @brief Set Indexed Color value: white (PM GPI extension).
*/
#define SICOL_WHITE    2

/*!
   @brief Set Indexed Color value: ones (PM GPI extension).
*/
#define SICOL_ONES     4

/*!
   @brief Set Indexed Color value: zeroes (PM GPI extension).
*/
#define SICOL_ZEROES   5

/* Set Character Cell */

/*!
   @brief Set Character Cell order (GSCC).

   Sets the character cell-size attribute.  Two formats exist: 4-byte
   (integer only) and 8-byte (integer + fractional).

   @see AFP GOCA Reference, "Set Character Cell (GSCC) Order", page 134.
   @see AFP GOCA Reference, "Character Strings", page 51.
*/
#define OCODE_GSCC     0x33            /* Set character cell          */

/*!
   @brief Push and Set Character Cell order (PM GPI extension).

   Not part of AFP GOCA.
*/
#define OCODE_GPSCC    0x03            /* Push and set char cell      */

/*!
   @brief Structure for Set Character Cell order (GSCC).

   Layout: integer width/height, fractional width/height, flags, reserved.

   @see AFP GOCA Reference, "Set Character Cell (GSCC) Order — Syntax",
        page 134.
*/
typedef struct _ORDERS_GSCC
{
   SHORT  cxInt;
   SHORT  cyInt;
   USHORT cxFract;
   USHORT cyFract;
   UCHAR  fbFlags;
   UCHAR  uchReserved;
} ORDERS_GSCC;

/*!
   @brief 32-bit structure for Set Character Cell order (GSCC).
*/
typedef struct _ORDERL_GSCC
{
   LONG   cxInt;
   LONG   cyInt;
   USHORT cxFract;
   USHORT cyFract;
   UCHAR  fbFlags;
   UCHAR  uchReserved;
} ORDERL_GSCC;

/*!
   @brief Character Cell flag: zero means drawing default.
*/
#define GSCC_ZERODEF   0x00

/*!
   @brief Character Cell flag: zero means zero size.
*/
#define GSCC_ZEROZERO  0x80

/* Set Marker Cell */

/*!
   @brief Set Marker Cell order (GSMC).

   Sets the marker cell-size attribute.  In AFP GOCA the order is supported
   for setting the size at which markers appear.

   @see AFP GOCA Reference, "Set Marker Cell (GSMC) Order", page 152.
   @see AFP GOCA Reference, "Markers", page 57.
*/
#define OCODE_GSMC     0x37            /* Set marker cell             */

/*!
   @brief Push and Set Marker Cell order (PM GPI extension).

   Not part of AFP GOCA.
*/
#define OCODE_GPSMC    0x77            /* Push and set marker cell    */

/*!
   @brief Structure for Set Marker Cell order (GSMC).

   Layout: width, height, flags, reserved.

   @see AFP GOCA Reference, "Set Marker Cell (GSMC) Order — Syntax", page 152.
*/
typedef struct _ORDERS_GSMC
{
   SHORT  cx;
   SHORT  cy;
   UCHAR  fbFlags;
   UCHAR  uchReserved;
} ORDERS_GSMC;

/*!
   @brief 32-bit structure for Set Marker Cell order (GSMC).
*/
typedef struct _ORDERL_GSMC
{
   LONG   cx;
   LONG   cy;
   UCHAR  fbFlags;
   UCHAR  uchReserved;
} ORDERL_GSMC;

/*!
   @brief Marker Cell flag: zero means drawing default.
*/
#define GSMC_ZERODEF   0x00

/*!
   @brief Marker Cell flag: zero means zero size.
*/
#define GSMC_ZEROZERO  0x80

/* Set Pattern Reference Point */

/*!
   @brief Set Pattern Reference Point order (GSPRP).

   Sets the origin used for placement of custom patterns when filling an
   area.  Not used for patterns from the default pattern set or for
   gradients.

   @see AFP GOCA Reference, "Set Pattern Reference Point (GSPRP) Order",
        page 157.
   @see AFP GOCA Reference, "Custom Patterns", page 41.
*/
#define OCODE_GSPRP    0xA0            /* Set pattern ref point       */

/*!
   @brief Push and Set Pattern Reference Point order (PM GPI extension).

   Not part of AFP GOCA.
*/
#define OCODE_GPSPRP   0xE0            /* Push and set patt ref pt    */

/*!
   @brief Structure for Set Pattern Reference Point order (GSPRP).

   Layout: flags, reserved, reference point (POINTS).

   @see AFP GOCA Reference, "Set Pattern Reference Point (GSPRP) Order —
        Syntax", page 157.
*/
typedef struct _ORDERS_GSPRP
{
   UCHAR fbFlags;
   UCHAR uchReserved;
   POINTS ptPos;
} ORDERS_GSPRP;

/*!
   @brief 32-bit structure for Set Pattern Reference Point order (GSPRP).
*/
typedef struct _ORDERL_GSPRP
{
   UCHAR fbFlags;
   UCHAR uchReserved;
   POINTL ptPos;
} ORDERL_GSPRP;

/*!
   @brief Pattern Reference Point flag: use drawing default.
*/
#define GSPRP_DEFAULT  0x80

/*!
   @brief Pattern Reference Point flag: set to specified value.
*/
#define GSPRP_SPECIFY  0x00


/* Set Individual Attribute */

/*!
   @brief Set Individual Attribute order (PM GPI extension).

   Not part of AFP GOCA.
*/
#define OCODE_GSIA     0x14            /* Set individual attribute    */

/*!
   @brief Push and Set Individual Attribute order (PM GPI extension).

   Not part of AFP GOCA.
*/
#define OCODE_GPSIA    0x54            /* Push and set ind attr       */

#define GSIA_VL 3

/*!
   @brief Structure for Set Individual Attribute order (PM GPI extension).
*/
typedef struct _ORDER_GSIA
{
   UCHAR  uchAttrType;
   UCHAR  uchPrimType;
   UCHAR  fbFlags;
   UCHAR  auchValue[GSIA_VL];
} ORDER_GSIA;

/*!
   @brief Set Individual Attribute type: color (PM GPI extension).
*/
#define GSIA_COLOR     0x01

/*!
   @brief Set Individual Attribute type: background color (PM GPI
          extension).
*/
#define GSIA_BCOLOR    0x02

/*!
   @brief Set Individual Attribute type: mix (PM GPI extension).
*/
#define GSIA_MIX       0x03

/*!
   @brief Set Individual Attribute type: background mix (PM GPI extension).
*/
#define GSIA_BMIX      0x04

/*!
   @brief Set Individual Attribute primitive type: line (PM GPI extension).
*/
#define GSIA_LINE      0x01

/*!
   @brief Set Individual Attribute primitive type: character (PM GPI
          extension).
*/
#define GSIA_CHAR      0x02

/*!
   @brief Set Individual Attribute primitive type: marker (PM GPI
          extension).
*/
#define GSIA_MARKER    0x03

/*!
   @brief Set Individual Attribute primitive type: pattern (PM GPI
          extension).
*/
#define GSIA_PATTERN   0x04

/*!
   @brief Set Individual Attribute primitive type: image (PM GPI extension).
*/
#define GSIA_IMAGE     0x05

/*!
   @brief Set Individual Attribute flag: specify (PM GPI extension).
*/
#define GSIA_SPECIFY   0x00

/*!
   @brief Set Individual Attribute flag: special (PM GPI extension).
*/
#define GSIA_SPECIAL   0x40

/*!
   @brief Set Individual Attribute flag: default (PM GPI extension).
*/
#define GSIA_DEFAULT   0x80

/*!
   @brief Set Individual Attribute value: black (PM GPI extension).
*/
#define GSIA_BLACK     1

/*!
   @brief Set Individual Attribute value: white (PM GPI extension).
*/
#define GSIA_WHITE     2

/*!
   @brief Set Individual Attribute value: ones (PM GPI extension).
*/
#define GSIA_ONES      4

/*!
   @brief Set Individual Attribute value: zeroes (PM GPI extension).
*/
#define GSIA_ZEROES    5


/* Set Model /Viewing Transform */

/*!
   @brief Set Model Transform order (PM GPI extension).

   Not part of AFP GOCA.
*/
#define OCODE_GSTM     0x24            /* Set model transform         */

/*!
   @brief Push and Set Model Transform order (PM GPI extension).

   Not part of AFP GOCA.
*/
#define OCODE_GPSTM    0x64            /* Push and set model tfm      */

/*!
   @brief Set Viewing Transform order (PM GPI extension).

   Not part of AFP GOCA.
*/
#define OCODE_GSTV     0x31            /* Set Viewing Transform       */

#define GSTM_ML        16

/*!
   @brief Structure for Set Model Transform order (PM GPI extension).
*/
typedef struct _ORDERS_GSTM
{
   UCHAR  uchReserved;
   UCHAR  fbFlags;
   USHORT fsMask;
   SHORT  asMatrix[GSTM_ML];
} ORDERS_GSTM;

/*!
   @brief 32-bit structure for Set Model Transform order (PM GPI
          extension).
*/
typedef struct _ORDERL_GSTM
{
   UCHAR  uchReserved;
   UCHAR  fbFlags;
   USHORT fsMask;
   LONG   alMatrix[GSTM_ML];
} ORDERL_GSTM;

/*!
   @brief Model Transform matrix element masks (PM GPI extension).
*/
#define GSTM_M11     0x8000
#define GSTM_M12     0x4000
#define GSTM_M13     0x2000
#define GSTM_M14     0x1000
#define GSTM_M21     0x0800
#define GSTM_M22     0x0400
#define GSTM_M23     0x0200
#define GSTM_M24     0x0100
#define GSTM_M31     0x0080
#define GSTM_M32     0x0040
#define GSTM_M33     0x0020
#define GSTM_M34     0x0010
#define GSTM_M41     0x0008
#define GSTM_M42     0x0004
#define GSTM_M43     0x0002
#define GSTM_M44     0x0001

/*!
   @brief Model Transform flag: unity (PM GPI extension).
*/
#define GSTM_UNITY     0x00

/*!
   @brief Model Transform flag: after (PM GPI extension).
*/
#define GSTM_AFTER     0x01

/*!
   @brief Model Transform flag: before (PM GPI extension).
*/
#define GSTM_BEFORE    0x02

/*!
   @brief Model Transform flag: overwrite (PM GPI extension).
*/
#define GSTM_OVERWRITE 0x03

/*!
   @brief Viewing Transform flag: overwrite (PM GPI extension).
*/
#define GSTV_OVERWRITE 0x00

/*!
   @brief Viewing Transform flag: after (PM GPI extension).
*/
#define GSTV_AFTER     0x04

/* Set Segment Boundary, Viewing Window */

/*!
   @brief Set Segment Boundary order (PM GPI extension).

   Not part of AFP GOCA.
*/
#define OCODE_GSSB     0x32            /* Set segment boundary        */

/*!
   @brief Set Viewing Window order (PM GPI extension).

   Not part of AFP GOCA.
*/
#define OCODE_GSVW     0x27            /* Set viewing window          */

/*!
   @brief Push and Set Viewing Window order (PM GPI extension).

   Not part of AFP GOCA.
*/
#define OCODE_GPSVW    0x67            /* Push and set view window    */

#define GSSB_ML        4

/*!
   @brief Structure for Set Segment Boundary order (PM GPI extension).
*/
typedef struct _ORDERS_GSSB
{
   UCHAR  fbFlags;
   UCHAR  fbMask;
   SHORT  alMatrix[GSSB_ML];
} ORDERS_GSSB;

/*!
   @brief 32-bit structure for Set Segment Boundary order (PM GPI
          extension).
*/
typedef struct _ORDERL_GSSB
{
   UCHAR  fbFLags;
   UCHAR  fbMask;
   LONG   alMatrix[GSSB_ML];
} ORDERL_GSSB;

/*!
   @brief Segment Boundary mask: X-left (PM GPI extension).
*/
#define GSSB_XLEFT     0x20

/*!
   @brief Segment Boundary mask: X-right (PM GPI extension).
*/
#define GSSB_XRIGHT    0x10

/*!
   @brief Segment Boundary mask: Y-bottom (PM GPI extension).
*/
#define GSSB_YBOTTOM   0x08

/*!
   @brief Segment Boundary mask: Y-top (PM GPI extension).
*/
#define GSSB_YTOP      0x04

/*!
   @brief Viewing Window flag: intersect (PM GPI extension).
*/
#define GSVW_INTERSECT 0x00

/*!
   @brief Viewing Window flag: replace (PM GPI extension).
*/
#define GSVW_REPLACE   0x80

/* Set Segment Characteristics */

/*!
   @brief Segment Characteristics order (GSGCH).

   In AFP GOCA this order is processed as a No-Op and is valid only in the
   prolog of a segment.

   @see AFP GOCA Reference, "Segment Characteristics (GSGCH) Order",
        page 129.
*/
#define OCODE_GSGCH    0x04            /* Set segment characteristics */

#define GSGCH_ML       254

/*!
   @brief Structure for Segment Characteristics order (GSGCH).

   @see AFP GOCA Reference, "Segment Characteristics (GSGCH) Order —
        Syntax", page 129.
*/
typedef struct _ORDER_GSGCH
{
   UCHAR  uchIdent;
   UCHAR  auchData[GSGCH_ML];
} ORDER_GSGCH;

/* Set Stroke Line Width */

/*!
   @brief Set Stroke Line Width order (PM GPI extension).

   Not part of AFP GOCA.
*/
#define OCODE_GSSLW    0x15            /* Set stroke line width       */

/*!
   @brief Push and Set Stroke Line Width order (PM GPI extension).

   Not part of AFP GOCA.
*/
#define OCODE_GPSSLW   0x55            /* Push and set strk l width   */

/*!
   @brief Structure for Set Stroke Line Width order (PM GPI extension).
*/
typedef struct _ORDERS_GSSLW
{
   UCHAR  fbFlags;
   UCHAR  uchReserved;
   SHORT  LineWidth;
} ORDERS_GSSLW;

/*!
   @brief 32-bit structure for Set Stroke Line Width order (PM GPI
          extension).
*/
typedef struct _ORDERL_GSSLW
{
   UCHAR  fbFlags;
   UCHAR  uchReserved;
   LONG   LineWidth;
} ORDERL_GSSLW;

/*!
   @brief Stroke Line Width flag: default (PM GPI extension).
*/
#define GSSLW_DEFAULT  0x80

/*!
   @brief Stroke Line Width flag: specify (PM GPI extension).
*/
#define GSSLW_SPECIFY  0x00

/* Sharp Fillet at Current Position */

/*!
   @brief Sharp Fillet at Current Position order (PM GPI extension).

   Not part of AFP GOCA.  AFP GOCA does not support a sharpness parameter
   for the Fillet order.

   @see AFP GOCA Reference, "Fillet", page 26 (Architecture Note on the
        sharpness parameter).
*/
#define OCODE_GCSFLT   0xA4            /* Sharp fillet at curr pos    */

/*!
   @brief Sharp Fillet at Given Position order (PM GPI extension).

   Not part of AFP GOCA.
*/
#define OCODE_GSFLT    0xE4            /* Sharp fillet at given pos   */

#define GCSFLT_SMF     21
#define GSFLT_SMF      20

/*!
   @brief Structure for Sharp Fillet at Current Position order (PM GPI
          extension).
*/
typedef struct _ORDERS_GCSFLT
{
   POINTS apt[2*GCSFLT_SMF];
   FIXED  afxSharpness[GCSFLT_SMF];
} ORDERS_GCSFLT;

#define GCSFLT_LMF     12
#define GSFLT_LMF      12

/*!
   @brief 32-bit structure for Sharp Fillet at Current Position order
          (PM GPI extension).
*/
typedef struct _ORDERL_GCSFLT
{
   POINTL apt[2*GCSFLT_SMF];
   FIXED  afxSharpness[GCSFLT_SMF];
} ORDERL_GCSFLT;

/* Bitblt */

/*!
   @brief Bitblt order (PM GPI extension).

   Not part of AFP GOCA.
*/
#define OCODE_GBBLT    0xD6            /* Bitblt                      */

/*!
   @brief Structure for Bitblt order (PM GPI extension).
*/
typedef struct _ORDERS_GBBLT
{
   USHORT  fsFlags;
   USHORT  usMix;
   HBITMAP hbmSrc;
   LONG    lOptions;
   RECT1S  rcsTargetRect;
   RECTL   rclSourceRect;
} ORDERS_GBBLT;

/*!
   @brief 32-bit structure for Bitblt order (PM GPI extension).
*/
typedef struct _ORDERL_GBBLT
{
   USHORT  fsFlags;
   USHORT  usMix;
   HBITMAP hbmSrc;
   LONG    lOptions;
   RECTL   rclTargetRect;
   RECTL   rclSourceRect;
} ORDERL_GBBLT;

/* Char & break extra */

/*!
   @brief Set Character Extra order (PM GPI extension).

   Not part of AFP GOCA.
*/
#define OCODE_GSCE     0x17            /* Set char extra              */

/*!
   @brief Push and Set Character Extra order (PM GPI extension).

   Not part of AFP GOCA.
*/
#define OCODE_GPSCE    0x57            /* Push and set char extra     */

/*!
   @brief Set Character Break Extra order (PM GPI extension).

   Not part of AFP GOCA.
*/
#define OCODE_GSCBE    0x05            /* Set char break extra        */

/*!
   @brief Push and Set Character Break Extra order (PM GPI extension).

   Not part of AFP GOCA.
*/
#define OCODE_GPSCBE   0x45            /* Push and set char break extra */

/*!
   @brief Structure for Set Character Extra / Break Extra orders (PM GPI
          extension).
*/
typedef struct _ORDER_GSCBE
{
   UCHAR   fbFlags;
   UCHAR   uchReserved;
   FIXED   ufxextra;
} ORDER_GSCE, ORDER_GPSCE, ORDER_GSCBE, ORDER_GPSCBE;

/* Escape */

/*!
   @brief Escape order (PM GPI extension).

   Not part of AFP GOCA.
*/
#define OCODE_GESCP    0xD5            /* Escape                      */

/*
* type describes type of escape order, identifier gives the escape
* order if the type is registered
*/
#define GESCP_ML       253

/*!
   @brief Structure for Escape order (PM GPI extension).
*/
typedef struct _ORDER_GESCP
{
   UCHAR  uchType;
   UCHAR  uchIdent;
   UCHAR  auchData[GESCP_ML];           /* Escape data                 */
} ORDER_GESCP;

/*!
   @brief Escape order flag: identifier is registered (PM GPI extension).
*/
#define GESCP_REG      0x80            /* identifier is registered    */

/* Escape (Bitblt) */

/*!
   @brief Escape order identifier: Bitblt (PM GPI extension).
*/
#define GEBB_REGID     0x02            /* uchIdent - Bitblt           */

#define ETYPE_GEBB          0x800200D5L

#define GEBB_LMP       29

/*!
   @brief Structure for Escape (Bitblt) order (PM GPI extension).
*/
typedef struct _ORDERL_GEBB
{
   UCHAR   fbFlags;
   USHORT  usMix;
   UCHAR   cPoints;
   HBITMAP hbmSrc;
   LONG    lReserved;
   LONG    lOptions;
   POINTL  aptPoints[GEBB_LMP];
} ORDERL_GEBB;

/* Escape (Set Pel) */

/*!
   @brief Escape order identifier: Set Pel (PM GPI extension).
*/
#define GEPEL_REGID    0x01            /* uchIdent - Set Pel          */

#define ETYPE_GEPEL         0x800100D5L

/* Escape (DrawBits) */

/*!
   @brief Escape order identifier: DrawBits (PM GPI extension).
*/
#define GEDB_REGID     0x04          /* uchIdent - DrawBits */

#define ETYPE_GEDB          0x800400D5L

/*!
   @brief Structure for Escape (DrawBits) order (PM GPI extension).
*/
typedef struct _ORDERL_GEDB
{
   USHORT      fsFlags;
   USHORT      usMix;
   PVOID       pBits;
   PBITMAPINFO2 pbmi;
   LONG        lOptions;
   RECTL       rclTargetRect;
   RECTL       rclSourceRect;
} ORDERL_GEDB;


/* Escape (FloodFill) */

/*!
   @brief Escape order identifier: FloodFill (PM GPI extension).
*/
#define GEFF_REGID     0x03          /* uchIdent - FloodFill */

#define ETYPE_GEFF          0x800300D5L

/*!
   @brief Structure for Escape (FloodFill) order (PM GPI extension).
*/
typedef struct _ORDERL_GEFF
{
   UCHAR       fsFlags;
   UCHAR       auchColor[3];
} ORDERL_GEFF;


/* Element Types for attribute bundles */

/*!
   @brief Element type: line bundle (PM GPI extension).
*/
#define ETYPE_LINEBUNDLE    0x0000FD01L

/*!
   @brief Element type: character bundle (PM GPI extension).
*/
#define ETYPE_CHARBUNDLE    0x0000FD02L

/*!
   @brief Element type: marker bundle (PM GPI extension).
*/
#define ETYPE_MARKERBUNDLE  0x0000FD03L

/*!
   @brief Element type: area bundle (PM GPI extension).
*/
#define ETYPE_AREABUNDLE    0x0000FD04L

/*!
   @brief Element type: image bundle (PM GPI extension).
*/
#define ETYPE_IMAGEBUNDLE   0x0000FD05L


/***************************************************************************\
*
* Very long orders
*
\***************************************************************************/

/*!
   @brief Test whether an order code denotes an extended (very long) order.

   @see AFP GOCA Reference, "Extended Format", page 78.
*/
#define VLONG_ORDER(oc)  ((oc)==OCODE_VLONG)

/*!
   @brief Extended (very long) order structure.

   Layout: 1-byte order code (X'FE'), 1-byte qualifier, 2-byte length,
   then up to 65,535 bytes of operand data.

   @see AFP GOCA Reference, "Extended Format", page 78.
*/
#define VORDER_ML 65531
typedef struct _VORDER
{
   UCHAR     idCode;
   UCHAR     uchQualifier;
   SWPUSHORT uchLength;
   UCHAR     uchData[VORDER_ML];
} VORDER;

/* Character String Extended */

/*!
   @brief Extended Character String order qualifier: current position.
*/
#define OCODEQ_GCCHSTE  0xB0           /* Qualifier - current posn    */

/*!
   @brief Extended Character String order qualifier: given position.
*/
#define OCODEQ_GCHSTE   0xF0           /* Qualifier - given position  */

/*!
   @brief Extended Tabbed Character String At order qualifier.
*/
#define OCODEQ_GTCHSPA  0xF4           /* Tabbed Char String At       */

/*!
   @brief Extended Character String order element type: current position.
*/
#define ETYPE_GCCHSTE       0x0000FEB0L

/*!
   @brief Extended Character String order element type: given position.
*/
#define ETYPE_GCHSTE        0x0000FEF0L

/*!
   @brief Structure for Extended Character String order at current
          position.
*/
typedef struct _ORDERS_GCCHSTE
{
   UCHAR     fbFlags;
   UCHAR     uchReserved;
   POINTS    ptRect[2];
   SWPUSHORT cchString;
   CHAR      achString[1];
   SHORT     adx[1];
} ORDERS_GCCHSTE;

/*!
   @brief 32-bit structure for Extended Character String order at current
          position.
*/
typedef struct _ORDERL_GCCHSTE
{
   UCHAR     fbFlags;
   UCHAR     uchReserved;
   POINTL    ptRect[2];
   SWPUSHORT cchString;
   CHAR      achString[1];
   LONG      adx[1];
} ORDERL_GCCHSTE;

/*!
   @brief 32-bit structure for Extended Tabbed Character String At order.
*/
typedef struct _ORDERL_GTCHSPA
{
   UCHAR     fbFlags;
   UCHAR     uchReserved;
   POINTL    ptRect[2];
   SWPUSHORT cchString;
   CHAR      achString[1];
   LONG      adx[2];
   LONG      tabs[1];
} ORDERL_GTCHSPA;

/*!
   @brief Extended Character String flag: draw rectangle.
*/
#define GCCHSTE_DRAWRECT      0x80

/*!
   @brief Extended Character String flag: no rectangle.
*/
#define GCCHSTE_NORECT        0x00

/*!
   @brief Extended Character String flag: clip.
*/
#define GCCHSTE_CLIP          0x40

/*!
   @brief Extended Character String flag: no clip.
*/
#define GCCHSTE_NOCLIP        0x00

/*!
   @brief Extended Character String flag: deemphasize (reserved).
*/
#define GCCHSTE_DEEMPHASIZE   0x20            /* Reserved */

/*!
   @brief Extended Character String flag: no deemphasize.
*/
#define GCCHSTE_NODEEMPHASIZE 0x00

/*!
   @brief Extended Character String flag: leave position.
*/
#define GCCHSTE_LEAVEPOS      0x10

/*!
   @brief Extended Character String flag: move position.
*/
#define GCCHSTE_MOVEPOS       0x00

/*!
   @brief Extended Character String flag: underscore.
*/
#define GCCHSTE_UNDERSCORE    0x08

/*!
   @brief Extended Character String flag: no underscore.
*/
#define GCCHSTE_NOUNDERSCORE  0x00

/*!
   @brief Extended Character String flag: strikeout.
*/
#define GCCHSTE_STRIKEOUT     0x04

/*!
   @brief Extended Character String flag: no strikeout.
*/
#define GCCHSTE_NOSTRIKEOUT   0x00

/*!
   @brief Extended Tabbed Character String flag: start position.
*/
#define GTCHSPA_STARTPOS      0x02

/*!
   @brief Extended Tabbed Character String flag: no start position.
*/
#define GTCHSPA_NOSTARTPOS    0x00

/* Extended Escape */

/*!
   @brief Extended Escape order qualifier.
*/
#define OCODEQ_GEESCP   0xD5           /* Qualifier - extended escape */

#define GEESCP_ML      65533

/*!
   @brief Structure for Extended Escape order (PM GPI extension).
*/
typedef struct _ORDER_GEESCP
{
   UCHAR  uchType;
   UCHAR  uchIdent;
   UCHAR  auchData[GEESCP_ML];
} ORDER_GEESCP;

#pragma pack()


