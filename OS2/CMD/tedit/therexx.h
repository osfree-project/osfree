/*
 * THE - The Hessling Editor. A text editor similar to VM/CMS xedit.
 * Copyright (C) 1991-2001 Mark Hessling
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to:
 *
 *    The Free Software Foundation, Inc.
 *    675 Mass Ave,
 *    Cambridge, MA 02139 USA.
 *
 *
 * If you make modifications to this software that you feel increases
 * it usefulness for the rest of the community, please email the
 * changes, enhancements, bug fixes as well as any and all ideas to me.
 * This software is going to be maintained and enhanced as deemed
 * necessary by the community.
 *
 * Mark Hessling,  M.Hessling@qut.edu.au  http://www.lightlink.com/hessling/
 */

/*
$Id: therexx.h,v 1.4 2001/12/18 08:23:27 mark Exp $
*/

#if defined(OS2)
# if defined(USE_REGINA)
#  include <rexxsaa.h>
# endif
# if defined(USE_OS2REXX) && !defined(__EMX__)
#  include <rexxsaa.h>
# endif
#endif

#if defined(UNIX)
# if defined(USE_REGINA)
#  include <rexxsaa.h>
# endif
# if defined(USE_REXXIMC)
#  include <rexxsaa.h>
# endif
# if defined(USE_REXX6000)
#  include <rexxsaa.h>
# endif
# if defined(USE_OREXX)
#  include <rexx.h>
# endif
# if defined(USE_REXXTRANS)
#  include <rexxtrans.h>
# endif
# if defined(USE_UNIREXX)
#  include <rexxsaa.h>
#  if defined(HAVE_PROTO) && !defined(ORXXPrototype)
#   define ORXXPrototype
#  endif
# endif
#endif

#if defined(DOS)
# if defined(USE_REGINA)
#  include <rexxsaa.h>
# endif
#endif

#if defined(MSWIN)
# if !defined(NOREXX)
typedef signed short int SHORT;
typedef char CHAR;
typedef CHAR *PCH;
#  include <windows.h>
#  include <wrexx.h>
#  define RXTHE_PSZ        LPCSTR
#  define ULONG            DWORD
#  define PUSHORT          LPWORD
#  define RXTHE_PFN        FARPROC
#  define RXTHE_PUCHAR     LPBYTE
#  define RXTHE_PRXSUBCOM  FARPROC
#  define RXTHE_PRXEXIT    FARPROC
# endif
#endif

#if defined(WIN32)
# if defined(USE_REGINA)
#  include <rexxsaa.h>
# endif
# if defined(USE_OREXX)
#  include <rexx.h>
# endif
# if defined(USE_QUERCUS)
#  include <windows.h>
#  include <wrexx32.h>
# endif
# if defined(USE_WINREXX)
#  include <windows.h>
#  include <wrexx32.h>
# endif
# if defined(USE_REXXTRANS)
#  include <rexxtrans.h>
# endif
# if defined(USE_UNIREXX)
#  include <windows.h>
#  include <rexxsaa.h>
#  if defined(HAVE_PROTO) && !defined(ORXXPrototype)
#   define ORXXPrototype
#  endif
# endif
#endif

#if defined(AMIGA)
# if defined(USE_REGINA)
#  include <rexxsaa.h>
# endif
#endif

/*
 * The following abbreviations are used as prefixes for the following
 * #defines:
 *
 * RFH   - RexxFunctionHandler
 * RRFE  - RexxRegisterFuntionExe
 * RDF   - RexxDeregisterFuntion
 *
 * REH   - RexxExitHandler
 * RREE  - RexxRegisterExitExe
 * RDE   - RexxDeregisterExit
 *
 * RSH   - RexxSubcomHandler
 * RRSE  - RexxRegisterSubcomExe
 * RDS   - RexxDeregisterSubcom
 *
 * RS    - RexxStart
 */
#if defined(USE_REXX6000)
# define RXSTRING_STRPTR_TYPE PUCHAR  /* PRXFUNC - RexxFunctionHandler */
# define RFH_RETURN_TYPE      USHORT
# define RFH_ARG0_TYPE        PSZ
# define RFH_ARG1_TYPE        ULONG
# define RFH_ARG2_TYPE        PRXSTRING
# define RFH_ARG3_TYPE        PSZ
# define RFH_ARG4_TYPE        PRXSTRING
# define RRFE_ARG0_TYPE       PSZ
# define RRFE_ARG1_TYPE       PRXFUNC
# define RRFE_ARG2_TYPE       PUCHAR
# define RDF_ARG0_TYPE        PSZ
# define REH_RETURN_TYPE      LONG
# define REH_ARG0_TYPE        LONG
# define REH_ARG1_TYPE        LONG
# define REH_ARG2_TYPE        PEXIT
# define RSH_RETURN_TYPE      LONG /* PRXSUBCOM - RexxSubcomHandler */
# define RSH_ARG0_TYPE        PRXSTRING
# define RSH_ARG1_TYPE        PUSHORT
# define RSH_ARG2_TYPE        PRXSTRING
# define RRSE_ARG0_TYPE       PSZ
# define RRSE_ARG1_TYPE       PRXSUBCOM
# define RRSE_ARG2_TYPE       PUCHAR
# define RDS_ARG0_TYPE        PSZ
# define RS_ARG0_TYPE         LONG
# define RS_ARG1_TYPE         PRXSTRING
# define RS_ARG2_TYPE         PSZ
# define RS_ARG3_TYPE         PRXSTRING
# define RS_ARG4_TYPE         PSZ
# define RS_ARG5_TYPE         LONG
# define RS_ARG6_TYPE         PRXSYSEXIT
# define RS_ARG7_TYPE         PLONG
# define RS_ARG8_TYPE         PRXSTRING

#elif defined(USE_OS2REXX)
# if defined(__EMX__)
#  define RXSTRING_STRPTR_TYPE PCH
#  define RFH_RETURN_TYPE      ULONG
#  define RFH_ARG0_TYPE        PCSZ
#  define RFH_ARG1_TYPE        ULONG
#  define RFH_ARG2_TYPE        PRXSTRING
#  define RFH_ARG3_TYPE        PCSZ
#  define RFH_ARG4_TYPE        PRXSTRING
#  define RRFE_ARG0_TYPE       PSZ
#  define RRFE_ARG1_TYPE       RexxFunctionHandler *
#  define RDF_ARG0_TYPE        PSZ
#  define REH_RETURN_TYPE      LONG
#  define REH_ARG0_TYPE        LONG
#  define REH_ARG1_TYPE        LONG
#  define REH_ARG2_TYPE        PEXIT
#  define RREE_ARG0_TYPE       PSZ
#  define RREE_ARG1_TYPE       PFN
#  define RREE_ARG2_TYPE       PUCHAR
#  define RSH_RETURN_TYPE      ULONG
#  define RSH_ARG0_TYPE        PRXSTRING
#  define RSH_ARG1_TYPE        PUSHORT
#  define RSH_ARG2_TYPE        PRXSTRING
#  define RRSE_ARG0_TYPE       PSZ
#  define RRSE_ARG1_TYPE       PFN
#  define RRSE_ARG2_TYPE       PUCHAR
#  define RDE_ARG0_TYPE        PSZ
#  define RDE_ARG1_TYPE        PSZ
#  define RDS_ARG0_TYPE        PSZ
#  define RDS_ARG1_TYPE        PSZ
#  define RS_ARG0_TYPE         LONG
#  define RS_ARG1_TYPE         PRXSTRING
#  define RS_ARG2_TYPE         PSZ
#  define RS_ARG3_TYPE         PRXSTRING
#  define RS_ARG4_TYPE         PSZ
#  define RS_ARG5_TYPE         LONG
#  define RS_ARG6_TYPE         PRXSYSEXIT
#  define RS_ARG7_TYPE         PSHORT
#  define RS_ARG8_TYPE         PRXSTRING
# else
#  define RXSTRING_STRPTR_TYPE PCH
#  define RFH_RETURN_TYPE      ULONG
#  define RFH_ARG0_TYPE        PUCHAR
#  define RFH_ARG1_TYPE        ULONG
#  define RFH_ARG2_TYPE        PRXSTRING
#  define RFH_ARG3_TYPE        PSZ
#  define RFH_ARG4_TYPE        PRXSTRING
#  define RRFE_ARG0_TYPE       PSZ
#  define RRFE_ARG1_TYPE       PFN
#  define RDF_ARG0_TYPE        PSZ
#  define REH_RETURN_TYPE      LONG
#  define REH_ARG0_TYPE        LONG
#  define REH_ARG1_TYPE        LONG
#  define REH_ARG2_TYPE        PEXIT
#  define RREE_ARG0_TYPE       PSZ
#  define RREE_ARG1_TYPE       PFN
#  define RREE_ARG2_TYPE       PUCHAR
#  define RSH_RETURN_TYPE      ULONG
#  define RSH_ARG0_TYPE        PRXSTRING
#  define RSH_ARG1_TYPE        PUSHORT
#  define RSH_ARG2_TYPE        PRXSTRING
#  define RRSE_ARG0_TYPE       PSZ
#  define RRSE_ARG1_TYPE       PFN
#  define RRSE_ARG2_TYPE       PUCHAR
#  define RDE_ARG0_TYPE        PSZ
#  define RDE_ARG1_TYPE        PSZ
#  define RDS_ARG0_TYPE        PSZ
#  define RDS_ARG1_TYPE        PSZ
#  define RS_ARG0_TYPE         LONG
#  define RS_ARG1_TYPE         PRXSTRING
#  define RS_ARG2_TYPE         PSZ
#  define RS_ARG3_TYPE         PRXSTRING
#  define RS_ARG4_TYPE         PSZ
#  define RS_ARG5_TYPE         LONG
#  define RS_ARG6_TYPE         PRXSYSEXIT
#  define RS_ARG7_TYPE         PSHORT
#  define RS_ARG8_TYPE         PRXSTRING
# endif

#elif defined(USE_REGINA)
# if defined(CONST)
#  define C_PSZ  PCSZ
# else
#  define C_PSZ  PSZ
# endif
# define RXSTRING_STRPTR_TYPE char *
# define RFH_RETURN_TYPE      APIRET APIENTRY
# define RFH_ARG0_TYPE        C_PSZ
# define RFH_ARG1_TYPE        ULONG
# define RFH_ARG2_TYPE        PRXSTRING
# define RFH_ARG3_TYPE        C_PSZ
# define RFH_ARG4_TYPE        PRXSTRING
# define RRFE_ARG0_TYPE       C_PSZ
# define RRFE_ARG1_TYPE       PFN
# define RDF_ARG0_TYPE        C_PSZ
# define REH_RETURN_TYPE      LONG APIENTRY
# define REH_ARG0_TYPE        LONG
# define REH_ARG1_TYPE        LONG
# define REH_ARG2_TYPE        PEXIT
# define RREE_ARG0_TYPE       C_PSZ
# define RREE_ARG1_TYPE       PFN
# define RREE_ARG2_TYPE       PUCHAR
# define RSH_RETURN_TYPE      APIRET APIENTRY
# define RSH_ARG0_TYPE        PRXSTRING
# define RSH_ARG1_TYPE        PUSHORT
# define RSH_ARG2_TYPE        PRXSTRING
# define RRSE_ARG0_TYPE       C_PSZ
# define RRSE_ARG1_TYPE       PFN
# define RRSE_ARG2_TYPE       PUCHAR
# define RDE_ARG0_TYPE        C_PSZ
# define RDE_ARG1_TYPE        C_PSZ
# define RDS_ARG0_TYPE        C_PSZ
# define RDS_ARG1_TYPE        C_PSZ
# define RS_ARG0_TYPE         LONG
# define RS_ARG1_TYPE         PRXSTRING
# define RS_ARG2_TYPE         C_PSZ
# define RS_ARG3_TYPE         PRXSTRING
# define RS_ARG4_TYPE         C_PSZ
# define RS_ARG5_TYPE         LONG
# define RS_ARG6_TYPE         PRXSYSEXIT
# define RS_ARG7_TYPE         PSHORT
# define RS_ARG8_TYPE         PRXSTRING

#elif defined(USE_REXXIMC)
# define RXSTRING_STRPTR_TYPE char *
# define RFH_RETURN_TYPE      ULONG
# define RFH_ARG0_TYPE        PSZ
# define RFH_ARG1_TYPE        ULONG
# define RFH_ARG2_TYPE        PRXSTRING
# define RFH_ARG3_TYPE        PSZ
# define RFH_ARG4_TYPE        PRXSTRING
# define RRFE_ARG0_TYPE       PSZ
# define RRFE_ARG1_TYPE       RexxFunctionHandler*
# define RDF_ARG0_TYPE        PSZ
# define REH_RETURN_TYPE      LONG
# define REH_ARG0_TYPE        LONG
# define REH_ARG1_TYPE        LONG
# define REH_ARG2_TYPE        PEXIT
# define RREE_ARG0_TYPE       PSZ
# define RREE_ARG1_TYPE       RexxExitHandler*
# define RREE_ARG2_TYPE       PUCHAR
# define RSH_RETURN_TYPE      ULONG
# define RSH_ARG0_TYPE        PRXSTRING
# define RSH_ARG1_TYPE        PUSHORT
# define RSH_ARG2_TYPE        PRXSTRING
# define RRSE_ARG0_TYPE       PSZ
# define RRSE_ARG1_TYPE       RexxSubcomHandler *
# define RRSE_ARG2_TYPE       PUCHAR
# define RDE_ARG0_TYPE        PSZ
# define RDE_ARG1_TYPE        PSZ
# define RDS_ARG0_TYPE        PSZ
# define RDS_ARG1_TYPE        PSZ
# define RS_ARG0_TYPE         long
# define RS_ARG1_TYPE         PRXSTRING
# define RS_ARG2_TYPE         char *
# define RS_ARG3_TYPE         PRXSTRING
# define RS_ARG4_TYPE         PSZ
# define RS_ARG5_TYPE         long
# define RS_ARG6_TYPE         PRXSYSEXIT
# define RS_ARG7_TYPE         short *
# define RS_ARG8_TYPE         PRXSTRING

#elif defined(USE_OREXX)
# define RXSTRING_STRPTR_TYPE PCH
# define RFH_RETURN_TYPE      ULONG APIENTRY
# define RFH_ARG0_TYPE        PUCHAR
# define RFH_ARG1_TYPE        ULONG
# define RFH_ARG2_TYPE        PRXSTRING
# define RFH_ARG3_TYPE        PSZ
# define RFH_ARG4_TYPE        PRXSTRING
# define RRFE_ARG0_TYPE       PSZ
# define RRFE_ARG1_TYPE       PFN
# define RDF_ARG0_TYPE        PSZ
# define REH_RETURN_TYPE      LONG APIENTRY
# define REH_ARG0_TYPE        LONG
# define REH_ARG1_TYPE        LONG
# define REH_ARG2_TYPE        PEXIT
# define RREE_ARG0_TYPE       PSZ
# define RREE_ARG1_TYPE       PFN
# define RREE_ARG2_TYPE       PUCHAR
# define RSH_RETURN_TYPE      APIRET APIENTRY
# define RSH_ARG0_TYPE        PRXSTRING
# define RSH_ARG1_TYPE        PUSHORT
# define RSH_ARG2_TYPE        PRXSTRING
# define RRSE_ARG0_TYPE       PSZ
# define RRSE_ARG1_TYPE       PFN
# define RRSE_ARG2_TYPE       PUCHAR
# define RDE_ARG0_TYPE        PSZ
# define RDE_ARG1_TYPE        PSZ
# define RDS_ARG0_TYPE        PSZ
# define RDS_ARG1_TYPE        PSZ
# define RS_ARG0_TYPE         LONG
# define RS_ARG1_TYPE         PRXSTRING
# define RS_ARG2_TYPE         PSZ
# define RS_ARG3_TYPE         PRXSTRING
# define RS_ARG4_TYPE         PSZ
# define RS_ARG5_TYPE         LONG
# define RS_ARG6_TYPE         PRXSYSEXIT
# define RS_ARG7_TYPE         PSHORT
# define RS_ARG8_TYPE         PRXSTRING

#elif defined(USE_QUERCUS)
# define RXSTRING_STRPTR_TYPE LPBYTE
# define RFH_RETURN_TYPE      DWORD
# define RFH_ARG0_TYPE        LPCSTR
# define RFH_ARG1_TYPE        DWORD
# define RFH_ARG2_TYPE        PRXSTRING
# define RFH_ARG3_TYPE        LPCSTR
# define RFH_ARG4_TYPE        PRXSTRING
# define RRFE_ARG0_TYPE       LPCSTR
# define RRFE_ARG1_TYPE       FARPROC
# define RDF_ARG0_TYPE        LPCSTR
# define REH_RETURN_TYPE      LONG
# define REH_ARG0_TYPE        LONG
# define REH_ARG1_TYPE        LONG
# define REH_ARG2_TYPE        PEXIT
# define RREE_ARG0_TYPE       LPCSTR
# define RREE_ARG1_TYPE       FARPROC
# define RREE_ARG2_TYPE       LPBYTE
# define RDE_ARG0_TYPE        LPCSTR
# define RDE_ARG1_TYPE        LPCSTR
# define RSH_RETURN_TYPE      DWORD
# define RSH_ARG0_TYPE        PRXSTRING
# define RSH_ARG1_TYPE        LPWORD
# define RSH_ARG2_TYPE        PRXSTRING
# define RRSE_ARG0_TYPE       LPCSTR
# define RRSE_ARG1_TYPE       FARPROC
# define RRSE_ARG2_TYPE       LPBYTE
# define RDS_ARG0_TYPE        LPCSTR
# define RDS_ARG1_TYPE        LPCSTR
# define RS_ARG0_TYPE         int
# define RS_ARG1_TYPE         PRXSTRING
# define RS_ARG2_TYPE         LPCSTR
# define RS_ARG3_TYPE         PRXSTRING
# define RS_ARG4_TYPE         LPCSTR
# define RS_ARG5_TYPE         int
# define RS_ARG6_TYPE         PRXSYSEXIT
# define RS_ARG7_TYPE         LPSHORT
# define RS_ARG8_TYPE         PRXSTRING

#elif defined(USE_WINREXX)
# define RXSTRING_STRPTR_TYPE LPBYTE
# define RFH_RETURN_TYPE      int APIENTRY
# define RFH_ARG0_TYPE        LPCSTR
# define RFH_ARG1_TYPE        UINT
# define RFH_ARG2_TYPE        PRXSTRING
# define RFH_ARG3_TYPE        LPCSTR
# define RFH_ARG4_TYPE        PRXSTRING
# define RRFE_ARG0_TYPE       LPCSTR
# define RRFE_ARG1_TYPE       FARPROC
# define RDF_ARG0_TYPE        LPCSTR
# define REH_RETURN_TYPE      int APIENTRY
# define REH_ARG0_TYPE        int
# define REH_ARG1_TYPE        int
# define REH_ARG2_TYPE        PEXIT
# define RREE_ARG0_TYPE       LPCSTR
# define RREE_ARG1_TYPE       FARPROC
# define RREE_ARG2_TYPE       LPBYTE
# define RDE_ARG0_TYPE        LPCSTR
# define RDE_ARG1_TYPE        LPCSTR
# define RSH_RETURN_TYPE      int APIENTRY
# define RSH_ARG0_TYPE        PRXSTRING
# define RSH_ARG1_TYPE        LPWORD
# define RSH_ARG2_TYPE        PRXSTRING
# define RRSE_ARG0_TYPE       LPCSTR
# define RRSE_ARG1_TYPE       FARPROC
# define RRSE_ARG2_TYPE       LPBYTE
# define RDS_ARG0_TYPE        LPCSTR
# define RDS_ARG1_TYPE        LPCSTR
# define RS_ARG0_TYPE         int
# define RS_ARG1_TYPE         PRXSTRING
# define RS_ARG2_TYPE         LPCSTR
# define RS_ARG3_TYPE         PRXSTRING
# define RS_ARG4_TYPE         LPCSTR
# define RS_ARG5_TYPE         int
# define RS_ARG6_TYPE         PRXSYSEXIT
# define RS_ARG7_TYPE         LPSHORT
# define RS_ARG8_TYPE         PRXSTRING

#elif defined(USE_REXXTRANS)
# if defined(__EMX__)
#  define RXSTRING_STRPTR_TYPE PCH
#  define RFH_RETURN_TYPE      ULONG
#  define RFH_ARG0_TYPE        PCSZ
#  define RFH_ARG1_TYPE        ULONG
#  define RFH_ARG2_TYPE        PRXSTRING
#  define RFH_ARG3_TYPE        PCSZ
#  define RFH_ARG4_TYPE        PRXSTRING
#  define RRFE_ARG0_TYPE       PSZ
#  define RRFE_ARG1_TYPE       RexxFunctionHandler *
#  define RDF_ARG0_TYPE        PSZ
#  define REH_RETURN_TYPE      LONG
#  define REH_ARG0_TYPE        LONG
#  define REH_ARG1_TYPE        LONG
#  define REH_ARG2_TYPE        PEXIT
#  define RREE_ARG0_TYPE       PSZ
#  define RREE_ARG1_TYPE       PFN
#  define RREE_ARG2_TYPE       PUCHAR
#  define RSH_RETURN_TYPE      ULONG
#  define RSH_ARG0_TYPE        PRXSTRING
#  define RSH_ARG1_TYPE        PUSHORT
#  define RSH_ARG2_TYPE        PRXSTRING
#  define RRSE_ARG0_TYPE       PSZ
#  define RRSE_ARG1_TYPE       PFN
#  define RRSE_ARG2_TYPE       PUCHAR
#  define RDE_ARG0_TYPE        PSZ
#  define RDE_ARG1_TYPE        PSZ
#  define RDS_ARG0_TYPE        PSZ
#  define RDS_ARG1_TYPE        PSZ
#  define RS_ARG0_TYPE         LONG
#  define RS_ARG1_TYPE         PRXSTRING
#  define RS_ARG2_TYPE         PSZ
#  define RS_ARG3_TYPE         PRXSTRING
#  define RS_ARG4_TYPE         PSZ
#  define RS_ARG5_TYPE         LONG
#  define RS_ARG6_TYPE         PRXSYSEXIT
#  define RS_ARG7_TYPE         PSHORT
#  define RS_ARG8_TYPE         PRXSTRING
# else
#  define RXSTRING_STRPTR_TYPE char *
#  define RFH_RETURN_TYPE      APIRET APIENTRY
#  define RFH_ARG0_TYPE        PUCHAR
#  define RFH_ARG1_TYPE        ULONG
#  define RFH_ARG2_TYPE        PRXSTRING
#  define RFH_ARG3_TYPE        PSZ
#  define RFH_ARG4_TYPE        PRXSTRING
#  define RRFE_ARG0_TYPE       PSZ
#  define RRFE_ARG1_TYPE       PFN
#  define RDF_ARG0_TYPE        PSZ
#  define REH_RETURN_TYPE      LONG APIENTRY
#  define REH_ARG0_TYPE        LONG
#  define REH_ARG1_TYPE        LONG
#  define REH_ARG2_TYPE        PEXIT
#  define RREE_ARG0_TYPE       PSZ
#  define RREE_ARG1_TYPE       PFN
#  define RREE_ARG2_TYPE       PUCHAR
#  define RSH_RETURN_TYPE      APIRET APIENTRY
#  define RSH_ARG0_TYPE        PRXSTRING
#  define RSH_ARG1_TYPE        PUSHORT
#  define RSH_ARG2_TYPE        PRXSTRING
#  define RRSE_ARG0_TYPE       PSZ
#  define RRSE_ARG1_TYPE       PFN
#  define RRSE_ARG2_TYPE       PUCHAR
#  define RDE_ARG0_TYPE        PSZ
#  define RDE_ARG1_TYPE        PSZ
#  define RDS_ARG0_TYPE        PSZ
#  define RDS_ARG1_TYPE        PSZ
#  define RS_ARG0_TYPE         LONG
#  define RS_ARG1_TYPE         PRXSTRING
#  define RS_ARG2_TYPE         PSZ
#  define RS_ARG3_TYPE         PRXSTRING
#  define RS_ARG4_TYPE         PSZ
#  define RS_ARG5_TYPE         LONG
#  define RS_ARG6_TYPE         PRXSYSEXIT
#  define RS_ARG7_TYPE         PSHORT
#  define RS_ARG8_TYPE         PRXSTRING
# endif

#elif defined(USE_UNIREXX)
# define RXSTRING_STRPTR_TYPE PCH
#ifdef WIN32
#  define RFH_RETURN_TYPE     ULONG ORXXCDecl ORXXLoadDS
# else
#  define RFH_RETURN_TYPE     ULONG
#endif
# define RFH_ARG0_TYPE        PUCHAR
# define RFH_ARG1_TYPE        ULONG
# define RFH_ARG2_TYPE        PRXSTRING
# define RFH_ARG3_TYPE        PSZ
# define RFH_ARG4_TYPE        PRXSTRING
# define RRFE_ARG0_TYPE       PSZ
# define RRFE_ARG1_TYPE       PFN
# define RDF_ARG0_TYPE        PSZ
# ifdef WIN32
#  define REH_RETURN_TYPE     LONG ORXXCDecl ORXXLoadDS
# else
#  define REH_RETURN_TYPE     LONG
#endif
# define REH_ARG0_TYPE        LONG
# define REH_ARG1_TYPE        LONG
# define REH_ARG2_TYPE        PEXIT
# define RREE_ARG0_TYPE       PSZ
# define RREE_ARG1_TYPE       PFN
# define RREE_ARG2_TYPE       PUCHAR
# ifdef WIN32
#  define RSH_RETURN_TYPE     ULONG ORXXCDecl ORXXLoadDS
# else
#  define RSH_RETURN_TYPE     ULONG
#endif
# define RSH_ARG0_TYPE        PRXSTRING
# define RSH_ARG1_TYPE        PUSHORT
# define RSH_ARG2_TYPE        PRXSTRING
# define RRSE_ARG0_TYPE       PSZ
# define RRSE_ARG1_TYPE       PFN
# define RRSE_ARG2_TYPE       PUCHAR
# define RDE_ARG0_TYPE        PSZ
# define RDE_ARG1_TYPE        PSZ
# define RDS_ARG0_TYPE        PSZ
# define RDS_ARG1_TYPE        PSZ
# define RS_ARG0_TYPE         LONG
# define RS_ARG1_TYPE         PRXSTRING
# define RS_ARG2_TYPE         PSZ
# define RS_ARG3_TYPE         PRXSTRING
# define RS_ARG4_TYPE         PSZ
# define RS_ARG5_TYPE         LONG
# define RS_ARG6_TYPE         PRXSYSEXIT
# define RS_ARG7_TYPE         PSHORT
# define RS_ARG8_TYPE         PRXSTRING

#endif

