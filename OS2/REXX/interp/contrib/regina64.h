#ifndef __REGINA64_H_INCLUDED
#define __REGINA64_H_INCLUDED
/*
 * Try and get a 64bit datatype...
 */
#ifndef rx_64u
# if defined(WIN32)
#  define rx_64         signed __int64
#  define RX_64MAX      _I64_MAX
#  define rx_64u        unsigned __int64
#  define RX_64UMAX     _UI64_MAX
#  ifdef __MINGW32__
#    define rx_mk64u(num) (rx_64u) num##ULL
#  else
#   define rx_mk64u(num) (rx_64u) num##ui64
#  endif
# elif defined(__WATCOMC__) && !defined(__QNX__)
   /* You need OpenWatcom 1.1 on Win32 or OS/2 */
#  define rx_64         signed __int64
#  define rx_64u        unsigned __int64
#  define rx_mk64u(num) (rx_64u) num##ui64
#  define RX_64MAX      LONGLONG_MAX
#  define RX_64UMAX     ULONGLONG_MAX
# elif defined(HAVE_STDINT_H) && defined(HAVE_INTTYPES_H)
#  define rx_64         int64_t
#  define rx_64u        uint64_t
#  define rx_mk64u(num) (rx_64u) num##u
#  define RX_64MAX      INT64_MAX
#  define RX_64UMAX     UINT64_MAX
# elif defined(__WORDSIZE) && (__WORDSIZE >= 64)
#  define rx_64         signed
#  define rx_64u        unsigned
#  define rx_mk64u(num) (rx_64u) num##u
#  ifdef INT64_MAX
#   define RX_64MAX     INT64_MAX
#  endif
#  ifdef UINT64_MAX
#   define RX_64UMAX    UINT64_MAX
#  endif
# elif defined(WORD_BIT) && (WORD_BIT >= 64)
#  define rx_64         signed
#  define rx_64u        unsigned
#  define rx_mk64u(num) (rx_64u) num##u
#  ifdef INT64_MAX
#   define RX_64MAX     INT64_MAX
#  endif
#  ifdef UINT64_MAX
#   define RX_64UMAX    UINT64_MAX
#  endif
# elif defined(LONG_BIT) && (LONG_BIT >= 64)
#  define rx_64         signed long
#  define rx_64u        unsigned long
#  define rx_mk64u(num) (rx_64u) num##ul
#  ifdef INT64_MAX
#   define RX_64MAX     INT64_MAX
#  endif
#  ifdef UINT64_MAX
#   define RX_64UMAX    UINT64_MAX
#  endif
# elif defined(ULLONG_MAX) && (ULLONG_MAX != ULONG_MAX)
#  define rx_64         signed long long
#  define rx_64u        unsigned long long
#  define rx_mk64u(num) (rx_64u) num##ull
#  define RX_64MAX      LLONG_MAX
#  define RX_64UMAX     ULLONG_MAX
# elif defined(ULONG_LONG_MAX) && (ULONG_LONG_MAX != ULONG_MAX)
#  define rx_64         signed long long
#  define rx_64u        unsigned long long
#  define rx_mk64u(num) (rx_64u) num##ull
#  define RX_64MAX      LONG_LONG_MAX
#  define RX_64UMAX     ULONG_LONG_MAX
# else
#  define rx_64         signed long
#  define rx_64u        unsigned long
#  define rx_mk64u(num) (rx_64u) num##ul
#  define RX_64MAX      LONG_MAX
#  define RX_64UMAX     ULONG_MAX
# endif
#endif
/*
 * Large file support
 * Must have #included config.h before #including this file
 */
#if defined(HAVE__STATI64)
# define rx_stat_buf  _stati64
# if defined(WIN32)
#  define rx_stat      rx_w32_stat
# else
#  define rx_stat      stati64
# endif
# define rx_fstat     _fstati64
# define rx_fseek     _fseeki64
# define rx_ftell     _ftelli64
#else
# define rx_stat_buf  stat
# if defined(WIN32)
#  define rx_stat     rx_w32_stat
#else
#  define rx_stat     stat
#endif
# define rx_fstat     fstat
# define rx_fseek     fseek
# define rx_ftell     ftell
#endif

#if defined(HAVE_FSEEKO)
# undef rx_fseek
# define rx_fseek     fseeko
#endif

#if defined(HAVE_FTELLO)
# undef rx_ftell
# define rx_ftell     ftello
#endif

#endif
