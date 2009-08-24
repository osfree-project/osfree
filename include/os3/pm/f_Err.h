#define FREEPMERR_BASE  0x5000

/* This section lists PM errors returned by WinGetLastError
   in order of their error numbers. For explanations of these errors,
   see Error Name and Explanation.
*/
/* FreePM errors */
      #define FPMERR_NULL_POINTER              (FREEPMERR_BASE + 0x0001)
      #define FPMERR_NULL_WINDPROC             (FREEPMERR_BASE + 0x0002)
      #define FPMERR_INITSERVER_CONNECTION      (FREEPMERR_BASE + 0x0101)
