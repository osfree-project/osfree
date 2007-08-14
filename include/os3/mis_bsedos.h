      /* extended attribute structures */


      /* flags for _FEA.fEA */
      #define FEA_NEEDEA         0x80     /* need EA bit */


      typedef PFEA2 PDENA2;



   /* Reference types for DosEnumAttribute */
   #define ENUMEA_REFTYPE_FHANDLE  0       /* file handle */
   #define ENUMEA_REFTYPE_PATH     1       /* path name */
   #define ENUMEA_REFTYPE_MAX      ENUMEA_REFTYPE_PATH
