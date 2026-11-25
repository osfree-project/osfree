/*!
@file test_goca_stack.c
@brief GOCA order streams exercising the attribute stack.
All streams use GOCA_COORD_LONG / GOCA_ENDIAN_LE, the GocaInit default,
except abTestStackBE16, which requires GOCA_COORD_SHORT / GOCA_ENDIAN_BE.
*/

/*! @brief Byte sequence of a 32-bit little-endian value. */
#define LE32(v)   (BYTE)((v) & 0xFFu),          \
                  (BYTE)(((v) >> 8) & 0xFFu),   \
                  (BYTE)(((v) >> 16) & 0xFFu),  \
                  (BYTE)(((v) >> 24) & 0xFFu)

/*! @brief Byte sequence of a 16-bit big-endian value. */
#define BE16(v)   (BYTE)(((v) >> 8) & 0xFFu), (BYTE)((v) & 0xFFu)

/*! @brief Eight Push-and-Set Colour orders. */
#define PUSHCOL8  0x4A,0x04, 0x4A,0x04, 0x4A,0x04, 0x4A,0x04, \
                  0x4A,0x04, 0x4A,0x04, 0x4A,0x04, 0x4A,0x04

/*! @brief Sixty-four Push-and-Set Colour orders, the depth limit. */
#define PUSHCOL64 PUSHCOL8, PUSHCOL8, PUSHCOL8, PUSHCOL8, \
                  PUSHCOL8, PUSHCOL8, PUSHCOL8, PUSHCOL8

/*!
@brief Push and Pop of the foreground colour.
The first line is drawn green, the second red.
*/
static const BYTE abTestStackColor[] = {
    0x0A, 0x02,                                 /* GSCOL  red    */
    0x4A, 0x04,                                 /* GPSCOL green  */
    0xC1, 0x10, LE32(10), LE32(10), LE32(50), LE32(50),
    0x3F, 0x00,                                 /* GPOP          */
    0xC1, 0x10, LE32(10), LE32(20), LE32(50), LE32(60)
};

/*!
@brief Accounting unit of the stack: one Push-and-Set order, one entry.
Colour and line type are pushed in turn. The first Pop must restore the
line type only, leaving the colour green; the second Pop restores the
colour. Restoring the colour on the first Pop indicates per-attribute
accounting instead of per-order accounting.
*/
static const BYTE abTestStackPerOrder[] = {
    0x0A, 0x02,                                 /* GSCOL red     */
    0x18, 0x07,                                 /* GSLT  solid   */
    0x4A, 0x04,                                 /* GPSCOL green  */
    0x58, 0x01,                                 /* GPSLT dotted  */
    0x3F, 0x00,                                 /* GPOP: solid   */
    0xC1, 0x10, LE32(10), LE32(10), LE32(50), LE32(50),
    0x3F, 0x00,                                 /* GPOP: red     */
    0xC1, 0x10, LE32(10), LE32(20), LE32(50), LE32(60)
};

/*!
@brief Two nested pushes of the same attribute.
Colour sequence drawn: green, red, blue.
*/
static const BYTE abTestStackNested[] = {
    0x0A, 0x01,                                 /* GSCOL blue    */
    0x4A, 0x02,                                 /* GPSCOL red    */
    0x4A, 0x04,                                 /* GPSCOL green  */
    0xC1, 0x10, LE32(10), LE32(10), LE32(50), LE32(10),
    0x3F, 0x00,                                 /* GPOP: red     */
    0xC1, 0x10, LE32(10), LE32(20), LE32(50), LE32(20),
    0x3F, 0x00,                                 /* GPOP: blue    */
    0xC1, 0x10, LE32(10), LE32(30), LE32(50), LE32(30)
};

/*!
@brief Pop on an empty stack.
Both Pop orders are No-Ops; the stream completes with GOCA_OK.
*/
static const BYTE abTestStackEmpty[] = {
    0x3F, 0x00,
    0x3F, 0x00,
    0x0A, 0x02,
    0xC1, 0x10, LE32(10), LE32(10), LE32(50), LE32(50)
};

/*!
@brief Pop with a non-zero reserved byte.
GDDM SC33-0868-02 defines the second byte of GPOP as X'00', reserved.
*/
static const BYTE abTestStackReserved[] = {
    0x3F, 0x01
};

/*!
@brief Stack overflow: 65 Push-and-Set orders at a depth limit of 64.
The failing order starts at byte offset 128.
*/
static const BYTE abTestStackOverflow[] = {
    PUSHCOL64,
    0x4A, 0x04
};

/*!
@brief Push and Pop of the current position.
After the Pop the second GCLINE starts from (100,200), not from (500,600).
*/
static const BYTE abTestStackCurPos[] = {
    0x21, 0x08, LE32(100), LE32(200),           /* GSCP  100,200 */
    0x61, 0x08, LE32(300), LE32(400),           /* GPSCP 300,400 */
    0x81, 0x08, LE32(500), LE32(600),           /* GCLINE        */
    0x3F, 0x00,                                 /* GPOP          */
    0x81, 0x08, LE32(150), LE32(250)            /* GCLINE        */
};

/*!
@brief Push and Pop of the arc parameters.
The first full arc is an ellipse 200 by 50, the second a circle of
radius 100. The multiplier is FIXED88 1.0, low byte first.
*/
static const BYTE abTestStackArcParams[] = {
    0x22, 0x10, LE32(100), LE32(100), LE32(0), LE32(0),   /* GSAP  */
    0x62, 0x10, LE32(200), LE32(50),  LE32(0), LE32(0),   /* GPSAP */
    0x87, 0x02, 0x00, 0x01,                     /* GCFARC 1.0 */
    0x3F, 0x00,                                 /* GPOP       */
    0x87, 0x02, 0x00, 0x01                      /* GCFARC 1.0 */
};

/*!
@brief Push and Pop of the shorthand mix attributes.
Each shorthand order affects every primitive type but occupies one
stack entry, so one Pop restores all of them.
*/
static const BYTE abTestStackMix[] = {
    0x0C, 0x02,                                 /* GSMX  overpaint */
    0x4C, 0x04,                                 /* GPSMX XOR       */
    0x3F, 0x00,
    0x0D, 0x05,                                 /* GSBMX  leave alone */
    0x4D, 0x02,                                 /* GPSBMX overpaint   */
    0x3F, 0x00
};

/*!
@brief Push and Pop of the extended colour, long order format.
Colour indices 2 and 4 coincide in the GOCA and GPI tables.
*/
static const BYTE abTestStackExtColor[] = {
    0x26, 0x02, BE16(0x0002),                   /* GSECOL  red   */
    0x66, 0x02, BE16(0x0004),                   /* GPSECOL green */
    0xC1, 0x10, LE32(10), LE32(10), LE32(50), LE32(50),
    0x3F, 0x00,
    0xC1, 0x10, LE32(10), LE32(20), LE32(50), LE32(60)
};

/*!
@brief Push and Pop of the fractional line width.
The first line is four times normal width, the second normal.
The multiplier is FIXED88, low byte first.
*/
static const BYTE abTestStackFracWidth[] = {
    0x11, 0x02, 0x00, 0x01,                     /* GSFLW  1.0 */
    0x51, 0x02, 0x00, 0x04,                     /* GPSFLW 4.0 */
    0xC1, 0x10, LE32(10), LE32(10), LE32(50), LE32(10),
    0x3F, 0x00,
    0xC1, 0x10, LE32(10), LE32(20), LE32(50), LE32(20)
};

/*!
@brief Push and Pop of the marker symbol.
The first marker is a square, the second a cross.
*/
static const BYTE abTestStackMarker[] = {
    0x29, 0x01,                                 /* GSMT  cross  */
    0x69, 0x04,                                 /* GPSMT square */
    0xC2, 0x08, LE32(10), LE32(10),             /* GMRK         */
    0x3F, 0x00,
    0xC2, 0x08, LE32(20), LE32(20)
};

/*!
@brief Push and Pop of the character direction and the character cell.
Both pairs are restored independently, in reverse order of pushing.
*/
static const BYTE abTestStackChar[] = {
    0x3A, 0x01,                                 /* GSCD  left to right */
    0x7A, 0x02,                                 /* GPSCD top to bottom */
    0x3F, 0x00,
    0x33, 0x08, LE32(10), LE32(20),             /* GSCC  10 x 20 */
    0x03, 0x08, LE32(30), LE32(40),             /* GPSCC 30 x 40 */
    0x3F, 0x00
};

/*!
@brief Push and Pop of the pattern reference point.
Flags and the reserved byte are zero.
*/
static const BYTE abTestStackPatRefPt[] = {
    0xA0, 0x0A, 0x00, 0x00, LE32(10), LE32(20), /* GSPRP  */
    0xE0, 0x0A, 0x00, 0x00, LE32(30), LE32(40), /* GPSPRP */
    0x3F, 0x00
};

/*!
@brief Stack lifetime across a segment boundary.
GBSEG with append mode B'00' starts a new segment; the stack depth must
be reset, so the following Pop is a No-Op. Fails until segment
boundaries are handled.
*/
static const BYTE abTestStackSegment[] = {
    0x4A, 0x04,                                 /* GPSCOL green */
    0x70, 0x0C, 0x00,0x00,0x00,0x01,            /* name = 1     */
                0x00,                           /* flag1        */
                0x00,                           /* flag2: new   */
                0x00,0x00,                      /* segl         */
                0x00,0x00,0x00,0x00,            /* psname       */
    0x3F, 0x00
};

/*!
@brief Pop of the colour inside an area definition.
Colour is valid inside an area, so the Pop succeeds.
GBAR flags: boundary set, alternate fill, inclusive.
*/
static const BYTE abTestStackAreaValid[] = {
    0x0A, 0x02,                                 /* GSCOL red */
    0x68, 0x80,                                 /* GBAR      */
    0x4A, 0x04,                                 /* GPSCOL green */
    0xC1, 0x18, LE32(10), LE32(10), LE32(50), LE32(10),
                LE32(30), LE32(40),
    0x3F, 0x00,
    0x60, 0x00                                  /* GEAR */
};

/*!
@brief Pop of the character set inside an area definition.
The character set is pushed outside the area and popped inside it,
where it is not a valid attribute. Expected result EC-6802. Fails until
the area validity check on Pop is implemented.
*/
static const BYTE abTestStackAreaInvalid[] = {
    0x78, 0x01,                                 /* GPSCS lcid 1 */
    0x68, 0x80,                                 /* GBAR         */
    0x3F, 0x00,                                 /* GPOP         */
    0x60, 0x00                                  /* GEAR         */
};

/*!
@brief Push and Pop in the 16-bit big-endian format.
Requires GOCA_COORD_SHORT / GOCA_ENDIAN_BE before parsing.
Behaviour matches abTestStackColor.
*/
static const BYTE abTestStackBE16[] = {
    0x21, 0x04, BE16(100), BE16(200),           /* GSCP 100,200 */
    0x0A, 0x02,                                 /* GSCOL red    */
    0x4A, 0x04,                                 /* GPSCOL green */
    0xC1, 0x08, BE16(10), BE16(10), BE16(50), BE16(50),
    0x3F, 0x00,
    0xC1, 0x08, BE16(10), BE16(20), BE16(50), BE16(60)
};

/*!
@brief One attribute-stack test case.
*/
typedef struct _STACKTEST {
    const char *pszName;      /*!< Case name for the trace. */
    const BYTE *pbStream;     /*!< Order stream.            */
    ULONG       cbStream;     /*!< Stream length in bytes.  */
    LONG        rcExpected;   /*!< Expected return code.    */
} STACKTEST;

/*!
@brief Attribute-stack cases with a settled expected return code.
The segment-boundary and area-validity cases are run separately: their
expectations depend on work not yet implemented.
*/
static const STACKTEST aStackTests[] = {
    { "Color",     abTestStackColor,     sizeof(abTestStackColor),     GOCA_OK },
    { "PerOrder",  abTestStackPerOrder,  sizeof(abTestStackPerOrder),  GOCA_OK },
    { "Nested",    abTestStackNested,    sizeof(abTestStackNested),    GOCA_OK },
    { "Empty",     abTestStackEmpty,     sizeof(abTestStackEmpty),     GOCA_OK },
    { "Reserved",  abTestStackReserved,  sizeof(abTestStackReserved),  GOCA_EC_RESERVED_NOT_ZERO },
    { "Overflow",  abTestStackOverflow,  sizeof(abTestStackOverflow),  GOCA_EC_STACK_OVERFLOW },
    { "CurPos",    abTestStackCurPos,    sizeof(abTestStackCurPos),    GOCA_OK },
    { "ArcParams", abTestStackArcParams, sizeof(abTestStackArcParams), GOCA_OK },
    { "Mix",       abTestStackMix,       sizeof(abTestStackMix),       GOCA_OK },
    { "ExtColor",  abTestStackExtColor,  sizeof(abTestStackExtColor),  GOCA_OK },
    { "FracWidth", abTestStackFracWidth, sizeof(abTestStackFracWidth), GOCA_OK },
    { "Marker",    abTestStackMarker,    sizeof(abTestStackMarker),    GOCA_OK },
    { "Char",      abTestStackChar,      sizeof(abTestStackChar),      GOCA_OK },
    { "PatRefPt",  abTestStackPatRefPt,  sizeof(abTestStackPatRefPt),  GOCA_OK },
    { "AreaValid", abTestStackAreaValid, sizeof(abTestStackAreaValid), GOCA_OK }
};

/*!
@brief Parse one test stream on a fresh context.
@param [in] hdc       Device context to draw into.
@param [in] pbStream  Order stream.
@param [in] cbStream  Stream length in bytes.
@return The parser return code.
*/
static LONG RunStackTest(HDC hdc, const BYTE *pbStream, ULONG cbStream)
{
    GOCA_CTX c;
    GocaInit(&c, hdc, (PBYTE)pbStream, cbStream);
    return GocaParseStream(&c);
}

/*!
@brief Run every attribute-stack case with a settled expectation.
@param [in] hdc  Device context to draw into.
@return Number of cases whose return code differs from the expectation.
*/
static ULONG RunStackTests(HDC hdc)
{
    ULONG i, cFailed = 0;

    for (i = 0; i < sizeof(aStackTests) / sizeof(aStackTests[0]); i++) {
        LONG rc = RunStackTest(hdc, aStackTests[i].pbStream,
                               aStackTests[i].cbStream);
        if (rc != aStackTests[i].rcExpected) {
            cFailed++;
        }
    }
    return cFailed;
}

