/*!
 * @file tests_state.cpp
 * @brief Drawing state tests (T21-T33).
 *
 * All these functions are already implemented on the server
 * (NGpiSet/QueryDrawingMode, Set/QueryDrawControl, Set/QueryStopDraw,
 * Set/QueryEditMode, Set/QueryInitialSegmentAttrs).
 */
#include "tests_common.h"

void RunSuite_State(void)
{
    HPS  hps;
    LONG rc;

    SUITE("Drawing state (T21-T33)");

    hps = TestNewPS();
    if (!hps) { CHECK(0, "TestNewPS failed"); return; }

    /* ============================================================ */
    /* T21-T24 — Drawing mode                                       */
    /* ============================================================ */
    TEST(21, "T21: SetDrawingMode(DM_DRAW) + Query = DM_DRAW");
    GpiSetDrawingMode(hps, DM_DRAW);
    CHECK_EQ_LONG(GpiQueryDrawingMode(hps), DM_DRAW, "mode = DM_DRAW");

    TEST(22, "T22: SetDrawingMode(DM_RETAIN) + Query = DM_RETAIN");
    GpiSetDrawingMode(hps, DM_RETAIN);
    CHECK_EQ_LONG(GpiQueryDrawingMode(hps), DM_RETAIN, "mode = DM_RETAIN");

    TEST(23, "T23: SetDrawingMode(DM_DRAWANDRETAIN) + Query = DM_DRAWANDRETAIN");
    GpiSetDrawingMode(hps, DM_DRAWANDRETAIN);
    CHECK_EQ_LONG(GpiQueryDrawingMode(hps), DM_DRAWANDRETAIN, "mode = DM_DRAWANDRETAIN");

    TEST(24, "T24: SetDrawingMode(999) fails, mode unchanged");
    GpiSetDrawingMode(hps, DM_DRAW);
    rc = GpiSetDrawingMode(hps, 999);
    CHECK_EQ_LONG(rc, GPI_ERROR, "SetDrawingMode(999) -> error");
    CHECK_EQ_LONG(GpiQueryDrawingMode(hps), DM_DRAW, "mode still DM_DRAW");

    /* ============================================================ */
    /* T25-T28 — Draw control                                       */
    /* ============================================================ */
    TEST(25, "T25: SetDrawControl(DCTL_ERASE, OFF) + Query = OFF");
    GpiSetDrawControl(hps, DCTL_ERASE, DCTL_ON);   /* ensure ON first */
    GpiSetDrawControl(hps, DCTL_ERASE, DCTL_OFF);
    CHECK_EQ_LONG(GpiQueryDrawControl(hps, DCTL_ERASE), DCTL_OFF, "ERASE = OFF");

    TEST(26, "T26: SetDrawControl(DCTL_DISPLAY, ON) + Query = ON");
    GpiSetDrawControl(hps, DCTL_DISPLAY, DCTL_ON);
    CHECK_EQ_LONG(GpiQueryDrawControl(hps, DCTL_DISPLAY), DCTL_ON, "DISPLAY = ON");

    TEST(27, "T27: SetDrawControl(0, ON) fails (invalid control)");
    rc = GpiSetDrawControl(hps, 0, DCTL_ON);
    CHECK_EQ_LONG(rc, GPI_ERROR, "invalid control -> error");

    TEST(28, "T28: SetDrawControl(DCTL_ERASE, 2) fails (invalid value)");
    rc = GpiSetDrawControl(hps, DCTL_ERASE, 2);
    CHECK_EQ_LONG(rc, GPI_ERROR, "invalid value -> error");

    /* Restore defaults for subsequent tests */
    GpiSetDrawControl(hps, DCTL_ERASE, DCTL_ON);

    /* ============================================================ */
    /* T29 — Stop draw                                              */
    /* ============================================================ */
    TEST(29, "T29: SetStopDraw(SDW_ON) + Query = SDW_ON");
    GpiSetStopDraw(hps, SDW_ON);
    CHECK_EQ_LONG(GpiQueryStopDraw(hps), SDW_ON, "stop draw = ON");
    GpiSetStopDraw(hps, SDW_OFF);
    CHECK_EQ_LONG(GpiQueryStopDraw(hps), SDW_OFF, "stop draw = OFF");

    /* ============================================================ */
    /* T30 — Edit mode                                              */
    /* ============================================================ */
    TEST(30, "T30: SetEditMode(SEGEM_REPLACE) + Query = SEGEM_REPLACE");
    GpiSetEditMode(hps, SEGEM_REPLACE);
    CHECK_EQ_LONG(GpiQueryEditMode(hps), SEGEM_REPLACE, "edit = REPLACE");
    GpiSetEditMode(hps, SEGEM_INSERT);
    CHECK_EQ_LONG(GpiQueryEditMode(hps), SEGEM_INSERT, "edit = INSERT");

    /* ============================================================ */
    /* T31-T33 — Initial segment attrs                              */
    /* ============================================================ */
    TEST(31, "T31: SetInitialSegmentAttrs(ATTR_VISIBLE, OFF) + Query = OFF");
    GpiSetInitialSegmentAttrs(hps, ATTR_VISIBLE, ATTR_OFF);
    CHECK_EQ_LONG(GpiQueryInitialSegmentAttrs(hps, ATTR_VISIBLE),
                  ATTR_OFF, "VISIBLE = OFF");
    GpiSetInitialSegmentAttrs(hps, ATTR_VISIBLE, ATTR_ON);
    CHECK_EQ_LONG(GpiQueryInitialSegmentAttrs(hps, ATTR_VISIBLE),
                  ATTR_ON, "VISIBLE = ON (restored)");

    TEST(32, "T32: SetInitialSegmentAttrs(ATTR_DYNAMIC, ON) + Query = ON");
    GpiSetInitialSegmentAttrs(hps, ATTR_DYNAMIC, ATTR_ON);
    CHECK_EQ_LONG(GpiQueryInitialSegmentAttrs(hps, ATTR_DYNAMIC),
                  ATTR_ON, "DYNAMIC = ON");
    GpiSetInitialSegmentAttrs(hps, ATTR_DYNAMIC, ATTR_OFF);
    CHECK_EQ_LONG(GpiQueryInitialSegmentAttrs(hps, ATTR_DYNAMIC),
                  ATTR_OFF, "DYNAMIC = OFF (restored)");

    TEST(33, "T33: SetInitialSegmentAttrs(999, ON) fails (invalid attr)");
    rc = GpiSetInitialSegmentAttrs(hps, 999, ATTR_ON);
    CHECK_EQ_LONG(rc, GPI_ERROR, "invalid attr -> error");

    TestFreePS(hps);
}
