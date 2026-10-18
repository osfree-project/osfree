/*!
 *
 * @file test_util.c
 *
 * @brief Implementation of the C89 test harness.
 *
 * (c) osFree Project 2026, <http://www.osFree.org>
 * for licence see licence.txt in root directory, or project website
 */

#include <stdio.h>
#include "test_util.h"

static int g_iPass = 0;
static int g_iFail = 0;

void test_start(const char *pszName) {
    printf("== %s\n", pszName ? pszName : "(unnamed)");
}

void test_pass(void) {
    g_iPass++;
}

void test_fail(const char *pszFile, int iLine, const char *pszExpr) {
    g_iFail++;
    printf("   FAIL %s:%d: %s\n",
           pszFile ? pszFile : "?",
           iLine,
           pszExpr ? pszExpr : "?");
}

int test_summary(void) {
    printf("---\n");
    printf("%d passed, %d failed\n", g_iPass, g_iFail);
    return (g_iFail == 0) ? 0 : 1;
}
