#ifndef _CYGWIN_INTTYPES_H
#define _CYGWIN_INTTYPES_H
/* /usr/include/inttypes.h for CYGWIN
 * Copyleft 2001-2002 by Felix Buenemann
 * <atmosfear at users.sourceforge.net>
 */
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef short int int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned int uint32_t;
#ifndef _WIN64
typedef signed int intptr_t;
typedef unsigned int uintptr_t;
#endif

#ifdef _MSC_VER
 typedef __int64 int64_t;
 typedef unsigned __int64 uint64_t;
#else
 typedef long long int64_t;
 typedef unsigned long long uint64_t;
#endif

#ifndef WIN64
 #define __WORDSIZE 32
#else
 #define __WORDSIZE 64
#endif
/*
typedef unsigned short UINT16;
typedef signed short INT16;
typedef unsigned char UINT8;
typedef unsigned int UINT32;
typedef uint64_t UINT64;
typedef signed char INT8;
typedef signed int INT32;
typedef int64_t INT64;

typedef long _ssize_t;
typedef _ssize_t ssize_t;
*/

#ifndef _I64_MIN
#define _I64_MIN (-9223372036854775807LL-1)
#endif

#ifndef _I64_MAX
#define _I64_MAX 9223372036854775807LL
#endif

#ifndef _UI64_MAX
#define _UI64_MAX 0xffffffffffffffffULL
#endif

#ifndef _UI32_MAX
#define _UI32_MAX 0xffffffffL
#endif

#ifndef INT32_MIN
#define INT32_MIN (-2147483647 - 1)
#endif

#ifndef INT32_MAX
#define INT32_MAX 2147483647
#endif

#ifndef INT_MIN
#define INT_MIN	INT32_MIN
#endif

#ifndef INT_MAX
#define INT_MAX	INT32_MAX
#endif

#ifndef INT16_MIN
#define INT16_MIN       (-0x7fff-1)
#endif

#ifndef INT16_MAX
#define INT16_MAX       0x7fff
#endif

#if !defined(__cplusplus) || defined(__STDC_FORMAT_MACROS)

/* 7.8.1 Macros for format specifiers
 *
 * MS runtime does not yet understand C9x standard "ll"
 * length specifier. It appears to treat "ll" as "l".
 * The non-standard I64 length specifier causes warning in GCC,
 * but understood by MS runtime functions.
 */

/* fprintf macros for signed types */
#define PRId8 "d"
#define PRId16 "d"
#define PRId32 "d"
#define PRId64 "I64d"

#define PRIdLEAST8 "d"
#define PRIdLEAST16 "d"
#define PRIdLEAST32 "d"
#define PRIdLEAST64 "I64d"

#define PRIdFAST8 "d"
#define PRIdFAST16 "d"
#define PRIdFAST32 "d"
#define PRIdFAST64 "I64d"

#define PRIdMAX "I64d"
#define PRIdPTR "d"

#define PRIi8 "i"
#define PRIi16 "i"
#define PRIi32 "i"
#define PRIi64 "I64i"

#define PRIiLEAST8 "i"
#define PRIiLEAST16 "i"
#define PRIiLEAST32 "i"
#define PRIiLEAST64 "I64i"

#define PRIiFAST8 "i"
#define PRIiFAST16 "i"
#define PRIiFAST32 "i"
#define PRIiFAST64 "I64i"

#define PRIiMAX "I64i"
#define PRIiPTR "i"

#define PRIo8 "o"
#define PRIo16 "o"
#define PRIo32 "o"
#define PRIo64 "I64o"

#define PRIoLEAST8 "o"
#define PRIoLEAST16 "o"
#define PRIoLEAST32 "o"
#define PRIoLEAST64 "I64o"

#define PRIoFAST8 "o"
#define PRIoFAST16 "o"
#define PRIoFAST32 "o"
#define PRIoFAST64 "I64o"

#define PRIoMAX "I64o"

#define PRIoPTR "o"

/* fprintf macros for unsigned types */
#define PRIu8 "u"
#define PRIu16 "u"
#define PRIu32 "u"
#define PRIu64 "I64u"


#define PRIuLEAST8 "u"
#define PRIuLEAST16 "u"
#define PRIuLEAST32 "u"
#define PRIuLEAST64 "I64u"

#define PRIuFAST8 "u"
#define PRIuFAST16 "u"
#define PRIuFAST32 "u"
#define PRIuFAST64 "I64u"

#define PRIuMAX "I64u"
#define PRIuPTR "u"

#define PRIx8 "x"
#define PRIx16 "x"
#define PRIx32 "x"
#define PRIx64 "I64x"

#define PRIxLEAST8 "x"
#define PRIxLEAST16 "x"
#define PRIxLEAST32 "x"
#define PRIxLEAST64 "I64x"

#define PRIxFAST8 "x"
#define PRIxFAST16 "x"
#define PRIxFAST32 "x"
#define PRIxFAST64 "I64x"

#define PRIxMAX "I64x"
#define PRIxPTR "x"

#define PRIX8 "X"
#define PRIX16 "X"
#define PRIX32 "X"
#define PRIX64 "I64X"

#define PRIXLEAST8 "X"
#define PRIXLEAST16 "X"
#define PRIXLEAST32 "X"
#define PRIXLEAST64 "I64X"

#define PRIXFAST8 "X"
#define PRIXFAST16 "X"
#define PRIXFAST32 "X"
#define PRIXFAST64 "I64X"

#define PRIXMAX "I64X"
#define PRIXPTR "X"

/*
 *   fscanf macros for signed int types
 *   NOTE: if 32-bit int is used for int_fast8_t and int_fast16_t
 *   (see stdint.h, 7.18.1.3), FAST8 and FAST16 should have
 *   no length identifiers
 */

#define SCNd16 "hd"
#define SCNd32 "d"
#define SCNd64 "I64d"

#define SCNdLEAST16 "hd"
#define SCNdLEAST32 "d"
#define SCNdLEAST64 "I64d"

#define SCNdFAST16 "hd"
#define SCNdFAST32 "d"
#define SCNdFAST64 "I64d"

#define SCNdMAX "I64d"
#define SCNdPTR "d"

#define SCNi16 "hi"
#define SCNi32 "i"
#define SCNi64 "I64i"

#define SCNiLEAST16 "hi"
#define SCNiLEAST32 "i"
#define SCNiLEAST64 "I64i"

#define SCNiFAST16 "hi"
#define SCNiFAST32 "i"
#define SCNiFAST64 "I64i"

#define SCNiMAX "I64i"
#define SCNiPTR "i"

#define SCNo16 "ho"
#define SCNo32 "o"
#define SCNo64 "I64o"

#define SCNoLEAST16 "ho"
#define SCNoLEAST32 "o"
#define SCNoLEAST64 "I64o"

#define SCNoFAST16 "ho"
#define SCNoFAST32 "o"
#define SCNoFAST64 "I64o"

#define SCNoMAX "I64o"
#define SCNoPTR "o"

#define SCNx16 "hx"
#define SCNx32 "x"
#define SCNx64 "I64x"

#define SCNxLEAST16 "hx"
#define SCNxLEAST32 "x"
#define SCNxLEAST64 "I64x"

#define SCNxFAST16 "hx"
#define SCNxFAST32 "x"
#define SCNxFAST64 "I64x"

#define SCNxMAX "I64x"
#define SCNxPTR "x"


/* fscanf macros for unsigned int types */

#define SCNu16 "hu"
#define SCNu32 "u"
#define SCNu64 "I64u"

#define SCNuLEAST16 "hu"
#define SCNuLEAST32 "u"
#define SCNuLEAST64 "I64u"

#define SCNuFAST16 "hu"
#define SCNuFAST32 "u"
#define SCNuFAST64 "I64u"

#define SCNuMAX "I64u"
#define SCNuPTR "u"

#if defined (__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
/*
 * no length modifier for char types prior to C9x
 * MS runtime  scanf appears to treat "hh" as "h"
 */

/* signed char */
#define SCNd8 "hhd"
#define SCNdLEAST8 "hhd"
#define SCNdFAST8 "hhd"

#define SCNi8 "hhi"
#define SCNiLEAST8 "hhi"
#define SCNiFAST8 "hhi"

#define SCNo8 "hho"
#define SCNoLEAST8 "hho"
#define SCNoFAST8 "hho"

#define SCNx8 "hhx"
#define SCNxLEAST8 "hhx"
#define SCNxFAST8 "hhx"

/* unsigned char */
#define SCNu8 "hhu"
#define SCNuLEAST8 "hhu"
#define SCNuFAST8 "hhu"
#endif /* __STDC_VERSION__ >= 199901 */

#endif	/* !defined(__cplusplus) || defined(__STDC_FORMAT_MACROS) */

#endif
