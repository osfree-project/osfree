/* 
 $Id: F_shl.hpp,v 1.1 2002/09/09 10:36:30 evgen2 Exp $ 
*/
/*
 *  FreePM Shell constants, types, messages and  function declarations 
 */

#ifndef FREEPM_SHL
  #define FREEPM_SHL

#ifdef __cplusplus
      extern "C" {
#endif
   /*** program handle **********************/
   typedef LHANDLE HPROGRAM;       /* hprog */
   typedef HPROGRAM *PHPROGRAM;
   typedef LHANDLE HAPP;

   typedef ULONG PROGCATEGORY;       /* progc */
   typedef PROGCATEGORY *PPROGCATEGORY;

      typedef struct _PROGTYPE         /* progt */
      {
         PROGCATEGORY progc;
         ULONG        fbVisible;
      } PROGTYPE;
      typedef PROGTYPE *PPROGTYPE;
      
      
      /*** visibility flag for PROGTYPE structure **************************/
      #define SHE_VISIBLE         0x00
      #define SHE_INVISIBLE       0x01
      #define SHE_RESERVED        0xFF
      
      /*** Protected group flag for PROGTYPE structure *********************/
      #define SHE_UNPROTECTED     0x00
      #define SHE_PROTECTED       0x02
      
      
      
      /*** Structures associated with 'Prf' calls **************************/
      typedef struct _PROGDETAILS    /* progde */
      {
         ULONG     Length;         /* set this to sizeof(PROGDETAILS)  */
         PROGTYPE  progt;
         PSZ       pszTitle;       /* any of the pointers can be NULL  */
         PSZ       pszExecutable;
         PSZ       pszParameters;
         PSZ       pszStartupDir;
         PSZ       pszIcon;
         PSZ       pszEnvironment; /* this is terminated by  /0/0      */
         SWP       swpInitial;     /* this replaces XYWINSIZE          */
      } PROGDETAILS;
      typedef  PROGDETAILS *PPROGDETAILS;
      
      typedef struct _PROGTITLE             /* progti */
      {
         HPROGRAM hprog;
         PROGTYPE progt;
         PSZ      pszTitle;
      } PROGTITLE;
      typedef PROGTITLE *PPROGTITLE;


      HAPP APIENTRY WinStartApp(HWND hwndNotify,
                                PPROGDETAILS pDetails,
                                PCSZ  pszParams,
                                PVOID Reserved,
                                ULONG fbOptions);

#ifdef __cplusplus
        }
#endif


#endif
   /* FREEPM_SHL */
