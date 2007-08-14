/* 
 $Id: F_GPI_struct.hpp,v 1.1 2003/06/18 22:51:49 evgen2 Exp $ 
*/

/* F_GPI_struct.hpp */
#ifndef FREEPM_GPI_STRUCT
#define FREEPM_GPI_STRUCT

   /* control data for GpiQueryRegionRects */
   typedef struct _RGNRECT          /* rgnrc */
   {
      ULONG  ircStart;
      ULONG  crc;
      ULONG  crcReturned;
      ULONG  ulDirection;
   } RGNRECT;
   typedef RGNRECT *PRGNRECT;
   
/* structure for size parameters e.g. for GpiCreatePS */
  typedef struct _SIZEL            /* sizl */
  {
     LONG cx;
     LONG cy;
  } SIZEL;
  typedef SIZEL *PSIZEL;


#endif /* FREEPM_GPI_STRUCT */
