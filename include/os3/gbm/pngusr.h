
#ifndef PNGUSR_H
#define PNGUSR_H

/* Note:
 * We build a static lib -> use compiler specific calling convention.
 * in header file pngconf.h the part that configures exports and sets
 * api calling conventions must be disabled for GBM build.
 *
 * Check for NO_GBM_BUILD ifdef in pngconf.h which marks the area
 * beginning with libpng 1.5.1
 */

/* we supply our own memory management functions */
#ifndef PNG_USER_MEM_SUPPORTED
 #define PNG_USER_MEM_SUPPORTED
#endif

/* disable warnings and console I/O in release code */
#ifdef NDEBUG
  #undef PNG_WARNINGS_SUPPORTED
  #undef PNG_CONSOLE_IO_SUPPORTED
#endif

/* Disable unused features */
#undef PNG_READ_UNKNOWN_CHUNKS_SUPPORTED
#undef PNG_WRITE_UNKNOWN_CHUNKS_SUPPORTED
#undef PNG_READ_USER_CHUNKS_SUPPORTED
#undef PNG_READ_iCCP_SUPPORTED
#undef PNG_WRITE_iCCP_SUPPORTED
#undef PNG_READ_iTXt_SUPPORTED
#undef PNG_WRITE_iTXt_SUPPORTED
#undef PNG_MNG_FEATURES_SUPPORTED

/* Disable APNG write support (only read is used with APNG patch) */
#undef PNG_WRITE_APNG_SUPPORTED

#endif
