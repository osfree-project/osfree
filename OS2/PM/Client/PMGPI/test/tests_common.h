/*!
 * @file tests_common.h
 * @brief Test framework for FreePM GPI regression suite.
 */
#ifndef TESTS_COMMON_H
#define TESTS_COMMON_H

#include <stdio.h>
#include <string.h>

/* Win* (WinInitialize, WinCreateMsgQueue, WinGetScreenPS, ...) */
#ifndef INCL_WIN
#define INCL_WIN
#endif

#if defined(__OS2__) || defined(__OS2V2__)
  #include "F_platform.h"
#else
  #include <os2win.h>
#endif

/* Counters - global, updated by CHECK macros */
extern int g_tests_pass;
extern int g_tests_fail;
extern int g_tests_current_id;
extern const char *g_current_suite;

/*! @brief Begin a test suite. */
#define SUITE(name) \
    do { \
        g_current_suite = (name); \
        printf("\n=== SUITE: %s ===\n", (name)); \
    } while (0)

/*! @brief Begin a single test (prints test id and name). */
#define TEST(id, name) \
    do { \
        g_tests_current_id = (id); \
        printf("[T%d] %s\n", (id), (name)); \
    } while (0)

/*! @brief Assert a condition. */
#define CHECK(cond, msg) \
    do { \
        if (cond) { \
            g_tests_pass++; \
            printf("  PASS: %s\n", (msg)); \
        } else { \
            g_tests_fail++; \
            printf("  FAIL: %s (T%d)\n", (msg), g_tests_current_id); \
        } \
    } while (0)

/*! @brief Assert equality of two longs. */
#define CHECK_EQ_LONG(actual, expected, msg) \
    do { \
        long _a = (long)(actual); \
        long _e = (long)(expected); \
        if (_a == _e) { \
            g_tests_pass++; \
            printf("  PASS: %s (got %ld)\n", (msg), _a); \
        } else { \
            g_tests_fail++; \
            printf("  FAIL: %s (T%d, got %ld, expected %ld)\n", \
                   (msg), g_tests_current_id, _a, _e); \
        } \
    } while (0)

/*! @brief Assert equality of two ints. */
#define CHECK_EQ_INT(actual, expected, msg) \
    do { \
        int _a = (int)(actual); \
        int _e = (int)(expected); \
        if (_a == _e) { \
            g_tests_pass++; \
            printf("  PASS: %s (got %d)\n", (msg), _a); \
        } else { \
            g_tests_fail++; \
            printf("  FAIL: %s (T%d, got %d, expected %d)\n", \
                   (msg), g_tests_current_id, _a, _e); \
        } \
    } while (0)

/*! @brief Assert inequality (for error tests). */
#define CHECK_NE_LONG(actual, unexpected, msg) \
    do { \
        long _a = (long)(actual); \
        long _u = (long)(unexpected); \
        if (_a != _u) { \
            g_tests_pass++; \
            printf("  PASS: %s (got %ld)\n", (msg), _a); \
        } else { \
            g_tests_fail++; \
            printf("  FAIL: %s (T%d, got %ld, did not expect %ld)\n", \
                   (msg), g_tests_current_id, _a, _u); \
        } \
    } while (0)

/*!
 * @brief Scratch buffers for replacing C99 compound literals.
 *
 * Watcom (and pre-C++11 C++) do not support &(POINTL){x, y}.
 * Use SCRATCH_PT(x, y) / SCRATCH_SZ(cx, cy) instead:
 *     GpiMove(hps, SCRATCH_PT(10, 10));
 *
 * WARNING: only one SCRATCH_PT / SCRATCH_SZ may be live in a single
 * expression (each call overwrites the scratch). All current tests
 * use them one at a time.
 */
extern POINTL g_scratch_pt;
extern SIZEL  g_scratch_sz;

#define SCRATCH_PT(x_, y_)   (g_scratch_pt.x = (x_), g_scratch_pt.y = (y_), &g_scratch_pt)
#define SCRATCH_SZ(cx_, cy_) (g_scratch_sz.cx = (cx_), g_scratch_sz.cy = (cy_), &g_scratch_sz)

/*!
 * @brief Create a fresh PS for one test.
 *
 * Uses WinGetScreenPS (which internally goes to the server via
 * F_CMD_GET_HPS). Returns a fresh HPS in a clean state.
 * Returns NULL on failure.
 */
HPS TestNewPS(void);

/*!
 * @brief Release the PS created by TestNewPS.
 */
void TestFreePS(HPS hps);

/*!
 * @brief Get GOCA body of segment lSegId into buf.
 *
 * Wrapper around GpiGetData with offset 0.
 * Returns number of bytes read, or -1 on error.
 */
LONG TestGetSegmentBody(HPS hps, LONG lSegId, PBYTE buf, LONG cbBuf);

/*!
 * @brief Scan a GOCA byte stream and find the offset of the first
 *        occurrence of order code oc.
 *
 * Returns byte offset, or -1 if not found.
 */
LONG TestFindOrder(PBYTE buf, LONG cbBuf, BYTE oc);

/*!
 * @brief Count occurrences of order code oc in a GOCA byte stream.
 */
LONG TestCountOrder(PBYTE buf, LONG cbBuf, BYTE oc);

/*!
 * @brief Return byte length of GOCA order at buf (uses same rules
 *        as GocaOrderLength, no C++ dependency).
 */
LONG TestOrderLen(PBYTE p, LONG cbAvail);

/*! @brief Print summary and return exit code. */
int TestSummary(void);

#endif /* TESTS_COMMON_H */
