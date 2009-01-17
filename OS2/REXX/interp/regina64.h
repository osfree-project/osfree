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
