/**************************************************************************************
  Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
***************************************************************************************
  $HeadURL: https://subversion01/svn/HilscherDefinitions/netXFirmware/Headers/tags/20230403-00/includes/Hil_Compiler.h $: *//*!

  \file Hil_Compiler.h

  Definitions of compiler settings.

**************************************************************************************/
#ifndef HIL_COMPILER_H_
#define HIL_COMPILER_H_

#if !defined(USER_COMPILER)

/*****************************************************************************/
/* Set byte alignment for structure members.                                 */
/*****************************************************************************/
#if defined(__GNUC__) || defined(__clang__)
/* support for GNU and clang compiler. Note the clang compiler may also set
 * _MSC_VER, so __clang__ must be checked before _MSC_VER. */

  #define __HIL_PACKED_PRE
  #define __HIL_PACKED_POST    __attribute__((__packed__))

  /* macro for setting DWORD alignment of a tag's beginning */
  #define __HIL_ALIGNED_DWORD__   __attribute__ ((aligned (4)))

#elif defined(_MSC_VER)
/* support for MS Visual C++ compiler */

  #if _MSC_VER >= 1000
    #define __HIL_PACKED_PRE
    #define __HIL_PACKED_POST
    #define __HIL_PRAGMA_PACK_ENABLE
    #define __HIL_PRAGMA_PACK_1(label) pack(push, label, 1)
    #define __HIL_PRAGMA_UNPACK_1(label) pack(pop, label)
    #ifndef STRICT
      #define STRICT
    #endif
  #endif

#elif defined (__ADS__) || defined (__REALVIEW__)
/* support for REALVIEW ARM compiler */

  #define __HIL_PACKED_PRE   __packed
  #define __HIL_PACKED_POST

#endif

/*****************************************************************************/
/* Define for variable length arrays at end of structures                    */
/*****************************************************************************/
#if defined(__GNUC__) || defined(__clang__) || defined (__ADS__) || defined (__REALVIEW__)
/* support for GNU, clang, ADS and REALVIEW ARM compiler. */
    #define __HIL_VARIABLE_LENGTH_ARRAY

#elif defined(_MSC_VER)
/* support for MS Visual C++ compiler */

  #if _MSC_VER >= 1000
    #define __HIL_VARIABLE_LENGTH_ARRAY   1
  #endif

#endif

/*****************************************************************************/
/* Common used macros                                                        */
/*****************************************************************************/
/*! For counting the number of array elements */
#define HIL_CNT_ELEMENT(array)        (sizeof(array) / sizeof((array)[0]))

/*! Compute the element count of an array to hold a given number of bytes. */
#define HIL_CALC_ELEMENT_CNT(type, size) (((size) + sizeof(type) - 1)/ sizeof(type))

/*! Returns the smaller number of two values */
#define HIL_MIN(a, b)                 (((a)<(b))?(a):(b))

/*! Returns the bigger number of two values */
#define HIL_MAX(a, b)                 (((a)>(b))?(a):(b))

/*! Calculates the offset of the given member in a structure, in bytes */
#define HIL_OFFSETOF(type, member)    (((unsigned long)&(((type*)(1))->member)) - 1)

/*! Returns the size of the given member of a structure */
#define HIL_MEMBERSIZE(type, member)  (sizeof(((type *)1)->member))

/*! Returns the address of the object containing the given member */
#if defined(__GNUC__) && !defined(_lint)
  /* GNU has the facility to make type check */
  #define HIL_CONTAINEROF(ptr, type, member) \
                                  ({const typeof( ((type *)0)->member ) *__mptr = (ptr);\
                                    (type *)( (char *)__mptr - HIL_OFFSETOF(type,member));})
#else
  #define HIL_CONTAINEROF(ptr, type, member) \
                                  ((type *)((char*)(ptr) - HIL_OFFSETOF(type,member)))
#endif

/*****************************************************************************/
#else
  #include "User_Compiler.h"
#endif

#endif /* HIL_COMPILER_H_ */
