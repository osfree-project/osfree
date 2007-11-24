      /* extended attribute structures */


      /* flags for _FEA.fEA */
      #define FEA_NEEDEA         0x80     /* need EA bit */


      typedef PFEA2 PDENA2;



   /* Reference types for DosEnumAttribute */
   #define ENUMEA_REFTYPE_FHANDLE  0       /* file handle */
   #define ENUMEA_REFTYPE_PATH     1       /* path name */
   #define ENUMEA_REFTYPE_MAX      ENUMEA_REFTYPE_PATH


   /* defined bits in pipe mode */
   #define NP_NBLK                    0x8000 /* non-blocking read/write */
   #define NP_SERVER                  0x4000 /* set if server end       */
   #define NP_WMESG                   0x0400 /* write messages          */
   #define NP_RMESG                   0x0100 /* read as messages        */
   #define NP_ICOUNT                  0x00FF /* instance count field    */
