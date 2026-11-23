/*!
 * @file tests_main.cpp
 * @brief Test runner for FreePM GPI regression suite.
 *
 * Test suite organisation:
 *   Suite P0        - bugs #1-#8, N4, N5, Extra (regression tests)
 *   Suite LegacyPort - G1-G7 (ported from testgpi.cpp)
 *   Suite T1-T6     - Transport
 *   Suite T7-T20    - PS lifecycle
 *   Suite T21-T33   - Drawing state
 *   Suite T34-T56   - Primitives
 *   Suite T57-T77   - Segments basic
 *   Suite T78-T101  - Segments advanced
 *   Suite T102-T126 - Element editing
 *   Suite T127-T139 - Paths
 *   Suite T140-T150 - Attribute queries (requires Step M)
 *   Suite T163-T177 - Edge + Stress
 *   Suite T178-T179 - Multi-PS
 *   Suite T200-T204 - GORDER (P8, placeholder)
 *
 * Total tests: ~191 + 5 GORDER placeholders.
 */
#include "tests_common.h"

/* Forward declarations */
void RunSuite_P0(void);
void RunSuite_LegacyPort(void);
void RunSuite_Transport(void);
void RunSuite_Lifecycle(void);
void RunSuite_State(void);
void RunSuite_Primitives(void);
void RunSuite_SegmentsBasic(void);
void RunSuite_SegmentsAdvanced(void);
void RunSuite_Elements(void);
void RunSuite_Paths(void);
void RunSuite_Queries(void);
void RunSuite_EdgeStress(void);
void RunSuite_MultiMicro(void);
void RunSuite_Gorder(void);

int main(void)
{
    HAB hab;
    HMQ hmq;
    int rc;

    printf("========================================\n");
    printf(" FreePM GPI Regression Test Suite\n");
    printf("========================================\n");
    printf(" Build: %s %s\n", __DATE__, __TIME__);
    printf(" Server: PMSERVER.EXE (must be running)\n");
    printf("========================================\n");

    hab = WinInitialize(0);
    if (!hab) {
        printf("FATAL: WinInitialize failed\n");
        return 1;
    }

    hmq = WinCreateMsgQueue(hab, 0);
    if (!hmq) {
        printf("FATAL: WinCreateMsgQueue failed\n");
        WinTerminate(hab);
        return 1;
    }

    /* Core suites - all runnable against current server (some will FAIL
       pending P0/P1/P2 fixes; this is expected TDD behaviour). */
    RunSuite_P0();
    RunSuite_LegacyPort();
    RunSuite_Transport();
    RunSuite_Lifecycle();
    RunSuite_State();
    RunSuite_Primitives();
    RunSuite_SegmentsBasic();
    RunSuite_SegmentsAdvanced();
    RunSuite_Elements();
    RunSuite_Paths();
    RunSuite_Queries();
    RunSuite_EdgeStress();
    RunSuite_MultiMicro();
    RunSuite_Gorder();

    WinDestroyMsgQueue(hmq);
    WinTerminate(hab);

    rc = TestSummary();
    return rc;
}
