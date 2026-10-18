/*!
 *
 * @file test_util.h
 *
 * @brief Minimal C89 test harness for the ccl Phase 0 tests.
 *
 * (c) osFree Project 2026, <http://www.osFree.org>
 * for licence see licence.txt in root directory, or project website
 *
 * Usage:
 *
 *     TEST_RUN(test_function);
 *
 * where test_function is a void(void) function that uses
 * TEST_ASSERT. A failed assertion aborts the current test function
 * (via return), prints file:line and the failed expression, and
 * increments the failure counter. test_summary() returns 0 if all
 * assertions passed, non-zero otherwise.
 */

#ifndef CCL_TEST_UTIL_H
#define CCL_TEST_UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * @brief Print the name of the test about to run.
 */
void test_start(const char *pszName);

/*!
 * @brief Record a successful assertion.
 */
void test_pass(void);

/*!
 * @brief Record a failed assertion and print its location.
 */
void test_fail(const char *pszFile, int iLine, const char *pszExpr);

/*!
 * @brief Print the summary line.
 *
 * @return 0 if all assertions passed, 1 otherwise.
 */
int test_summary(void);

#define TEST_RUN(fn)                                                     \
    do {                                                                 \
        test_start(#fn);                                                 \
        fn();                                                            \
    } while (0)

#define TEST_ASSERT(cond)                                                \
    do {                                                                 \
        if (cond) {                                                      \
            test_pass();                                                 \
        } else {                                                         \
            test_fail(__FILE__, __LINE__, #cond);                        \
            return;                                                      \
        }                                                                \
    } while (0)

#define TEST_ASSERT_MSG(cond, msg)                                       \
    do {                                                                 \
        if (cond) {                                                      \
            test_pass();                                                 \
        } else {                                                         \
            test_fail(__FILE__, __LINE__, (msg));                        \
            return;                                                      \
        }                                                                \
    } while (0)

#ifdef __cplusplus
}
#endif

#endif /* CCL_TEST_UTIL_H */
