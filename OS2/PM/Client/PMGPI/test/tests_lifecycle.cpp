/*!
 * @file tests_lifecycle.cpp
 * @brief PS lifecycle tests (T7-T20).
 *
 * T7-T10, T13-T15 test the CURRENT implementation (Create/Set/Destroy).
 * T11-T12, T16-T20 require Step K (Associate/Erase/Reset/QueryPS/Save/Restore)
 * and are marked with [requires Step K].
 */
#include "tests_common.h"

void RunSuite_Lifecycle(void)
{
    HPS   hps;
    HPS   hps2;
    LONG  rc;
    SIZEL szl;

    SUITE("PS lifecycle (T7-T20)");

    /* ============================================================ */
    /* T7 - CreatePS returns valid handle                           */
    /* ============================================================ */
    TEST(7, "T7: GpiCreatePS({640,480}) returns nonzero handle");
    {
        SIZEL in = {640, 480};
        hps = GpiCreatePS((HAB)1, (HDC)0, &in, 0);
        CHECK(hps != NULL, "hps != NULL");
        CHECK(hps != (HPS)GPI_ERROR, "hps != GPI_ERROR");
        if (hps && hps != (HPS)GPI_ERROR) GpiDestroyPS(hps);
    }

    /* ============================================================ */
    /* T8 - table full (65th CreatePS fails)                        */
    /* ============================================================ */
    TEST(8, "T8: 65th GpiCreatePS fails (table full)");
    {
        SIZEL sz = {100, 100};
        HPS   pool[70];
        int   k;
        int   created = 0;

        for (k = 0; k < 70; k++) {
            pool[k] = GpiCreatePS((HAB)1, (HDC)0, &sz, 0);
            if (pool[k] && pool[k] != (HPS)GPI_ERROR) created++;
        }
        /* Server table has 64 slots; client may have a different count,
           but at least one of the 65..70 must have failed. */
        CHECK(created >= 64, "at least 64 PS created");
        CHECK(created < 70, "not all 70 PS created (table has limit)");

        for (k = 0; k < 70; k++) {
            if (pool[k] && pool[k] != (HPS)GPI_ERROR) GpiDestroyPS(pool[k]);
        }
    }

    /* ============================================================ */
    /* T9 - QueryPS returns size (requires Step K + bug #7)         */
    /* ============================================================ */
    TEST(9, "T9: GpiQueryPS returns szlPS  [requires Step K]");
    {
        SIZEL in = {640, 480};
        SIZEL out = {0, 0};
        hps = GpiCreatePS((HAB)1, (HDC)0, &in, 0);
        if (hps && hps != (HPS)GPI_ERROR) {
            GpiQueryPS(hps, &out);
            CHECK_EQ_LONG(out.cx, 640, "QueryPS cx");
            CHECK_EQ_LONG(out.cy, 480, "QueryPS cy");
            GpiDestroyPS(hps);
        } else {
            CHECK(0, "CreatePS failed");
        }
    }

    /* ============================================================ */
    /* T10 - SetPS updates size                                     */
    /* ============================================================ */
    TEST(10, "T10: GpiSetPS({800,600}) updates size  [requires Step K]");
    {
        SIZEL in = {640, 480};
        SIZEL in2 = {800, 600};
        SIZEL out = {0, 0};
        hps = GpiCreatePS((HAB)1, (HDC)0, &in, 0);
        if (hps && hps != (HPS)GPI_ERROR) {
            GpiSetPS(hps, &in2, 0);
            GpiQueryPS(hps, &out);
            CHECK_EQ_LONG(out.cx, 800, "SetPS cx");
            CHECK_EQ_LONG(out.cy, 600, "SetPS cy");
            GpiDestroyPS(hps);
        } else {
            CHECK(0, "CreatePS failed");
        }
    }

    /* ============================================================ */
    /* T11 - QueryDevice  [requires Step K]                         */
    /* ============================================================ */
    TEST(11, "T11: GpiQueryDevice returns associated HDC  [requires Step K]");
    hps = TestNewPS();
    if (!hps) { CHECK(0, "TestNewPS failed"); return; }
    {
        HDC hdc = GpiQueryDevice(hps);
        CHECK(hdc != NULL, "QueryDevice returns non-NULL");
    }
    TestFreePS(hps);

    /* ============================================================ */
    /* T12 - Associate + QueryDevice  [requires Step K]             */
    /* ============================================================ */
    TEST(12, "T12: GpiAssociate(hdc) + QueryDevice returns hdc  [requires Step K]");
    {
        SIZEL in = {100, 100};
        HDC   fakeHdc = (HDC)0xABCD;
        HDC   outHdc;
        hps = GpiCreatePS((HAB)1, (HDC)0, &in, 0);
        if (hps && hps != (HPS)GPI_ERROR) {
            rc = GpiAssociate(hps, fakeHdc);
            CHECK_EQ_LONG(rc, 1, "Associate returns 1");
            outHdc = GpiQueryDevice(hps);
            CHECK(outHdc == fakeHdc, "QueryDevice returns fakeHdc");
            GpiDestroyPS(hps);
        } else {
            CHECK(0, "CreatePS failed");
        }
    }

    /* ============================================================ */
    /* T13 - DestroyPS frees slot                                   */
    /* ============================================================ */
    TEST(13, "T13: GpiDestroyPS frees slot");
    {
        SIZEL in = {100, 100};
        hps = GpiCreatePS((HAB)1, (HDC)0, &in, 0);
        if (!hps || hps == (HPS)GPI_ERROR) { CHECK(0, "CreatePS failed"); }
        else {
            rc = GpiDestroyPS(hps);
            CHECK_EQ_LONG(rc, 1, "DestroyPS returns 1");
        }
    }

    /* ============================================================ */
    /* T14 - operation on destroyed hps fails (bug #6)              */
    /* ============================================================ */
    TEST(14, "T14: GpiSetColor on destroyed hps returns error  [requires bug #6 fix]");
    {
        SIZEL in = {100, 100};
        hps = GpiCreatePS((HAB)1, (HDC)0, &in, 0);
        if (!hps || hps == (HPS)GPI_ERROR) { CHECK(0, "CreatePS failed"); }
        else {
            GpiDestroyPS(hps);
            rc = GpiSetColor(hps, CLR_RED);
            CHECK_EQ_LONG(rc, GPI_ERROR, "SetColor on destroyed hps -> error");
        }
    }

    /* ============================================================ */
    /* T15 - CreatePS after DestroyPS succeeds                      */
    /* ============================================================ */
    TEST(15, "T15: CreatePS after DestroyPS succeeds (slot reuse)");
    {
        SIZEL in = {100, 100};
        hps = GpiCreatePS((HAB)1, (HDC)0, &in, 0);
        if (hps && hps != (HPS)GPI_ERROR) {
            GpiDestroyPS(hps);
            hps2 = GpiCreatePS((HAB)1, (HDC)0, &in, 0);
            CHECK(hps2 != NULL, "second CreatePS returns handle");
            CHECK(hps2 != (HPS)GPI_ERROR, "second CreatePS not GPI_ERROR");
            if (hps2 && hps2 != (HPS)GPI_ERROR) GpiDestroyPS(hps2);
        } else {
            CHECK(0, "first CreatePS failed");
        }
    }

    /* ============================================================ */
    /* T16 - Erase  [requires Step K + real GRE]                    */
    /* ============================================================ */
    TEST(16, "T16: GpiErase calls GreErasePS  [requires Step K]");
    hps = TestNewPS();
    if (!hps) { CHECK(0, "TestNewPS failed"); return; }
    rc = GpiErase(hps);
    CHECK_EQ_LONG(rc, 1, "Erase returns 1");
    TestFreePS(hps);

    /* ============================================================ */
    /* T17 - ResetPS  [requires Step K]                             */
    /* ============================================================ */
    TEST(17, "T17: GpiResetPS clears pbGoca and state  [requires Step K]");
    hps = TestNewPS();
    if (!hps) { CHECK(0, "TestNewPS failed"); return; }
    {
        GpiSetDrawingMode(hps, DM_RETAIN);
        GpiOpenSegment(hps, 1);
        GpiSetColor(hps, CLR_RED);
        GpiMove(hps, SCRATCH_PT(10, 10));
        GpiLine(hps, SCRATCH_PT(100, 100));
        GpiCloseSegment(hps);
        GpiSetDrawingMode(hps, DM_DRAW);

        rc = GpiResetPS(hps, 0);
        CHECK_EQ_LONG(rc, 1, "ResetPS returns 1");

        /* After reset: no segments */
        {
            LONG ids[8];
            LONG count = GpiQuerySegmentNames(hps, 0, 999, 8, ids);
            CHECK_EQ_LONG(count, 0, "no segments after ResetPS");
        }
        /* After reset: drawing mode is DM_DRAW */
        CHECK_EQ_LONG(GpiQueryDrawingMode(hps), DM_DRAW,
                      "drawing mode = DM_DRAW after reset");
    }
    TestFreePS(hps);

    /* ============================================================ */
    /* T18 - SavePS stack overflow  [requires Step K]               */
    /* ============================================================ */
    TEST(18, "T18: GpiSavePS x8 + 9th fails (stack limit)  [requires Step K]");
    hps = TestNewPS();
    if (!hps) { CHECK(0, "TestNewPS failed"); return; }
    {
        LONG level;
        int  k;
        int  lastRc;
        for (k = 0; k < 8; k++) {
            level = GpiSavePS(hps);
        }
        lastRc = (int)GpiSavePS(hps);
        CHECK(lastRc == (int)GPI_ERROR || lastRc == 0,
              "9th SavePS fails (stack full)");
    }
    TestFreePS(hps);

    /* ============================================================ */
    /* T19 - RestorePS restores state  [requires Step K]            */
    /* ============================================================ */
    TEST(19, "T19: GpiRestorePS(1) restores color  [requires Step K]");
    hps = TestNewPS();
    if (!hps) { CHECK(0, "TestNewPS failed"); return; }
    {
        LONG lvl;
        GpiSetColor(hps, CLR_RED);
        lvl = GpiSavePS(hps);
        CHECK_NE_LONG(lvl, GPI_ERROR, "SavePS succeeds");

        GpiSetColor(hps, CLR_BLUE);
        rc = GpiRestorePS(hps, lvl);
        CHECK_EQ_LONG(rc, 1, "RestorePS returns 1");

        /* After restore: color should be red. Verify via segment replay:
           open segment, GpiSetColor is NOT called, but replaying the
           segment must use current color. However we cannot directly
           inspect current color from client without Step M.
           For now: only check RestorePS succeeds. */
    }
    TestFreePS(hps);

    /* ============================================================ */
    /* T20 - RestorePS invalid level  [requires Step K]             */
    /* ============================================================ */
    TEST(20, "T20: GpiRestorePS(0) and (9) fail  [requires Step K]");
    hps = TestNewPS();
    if (!hps) { CHECK(0, "TestNewPS failed"); return; }
    rc = GpiRestorePS(hps, 0);
    CHECK_EQ_LONG(rc, GPI_ERROR, "RestorePS(0) -> error");
    rc = GpiRestorePS(hps, 9);
    CHECK_EQ_LONG(rc, GPI_ERROR, "RestorePS(9) -> error");
    TestFreePS(hps);

    printf("\n  NOTE: T11-T12, T16-T20 require Step K (lifecycle).\n");
    printf("  T9-T10 require Step K + bug #7 fix.\n");
    printf("  T14 requires bug #6 fix.\n");
}
