/*!
 * @file compiler.h
 * @brief Project compilation.
 */
#ifndef COMPILER_H
#define COMPILER_H

#include "os2types.h"
#include "os2err.h"
#include "rtf.h"
#include "hpj.h"
#include "hfs.h"

APIRET APIENTRY CmpCompile(HHPJ hHpj, HRTFDOC hDoc, PCSZ pszHpjPath,
                           PHHFS phHfs);

#endif /* COMPILER_H */
