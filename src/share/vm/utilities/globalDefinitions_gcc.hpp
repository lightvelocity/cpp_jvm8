/*
 * globalDefinitions_gcc.hpp
 *
 *  Created on: 2018年1月4日
 *      Author: Administrator
 */

#ifndef SHARE_VM_UTILITIES_GLOBALDEFINITIONS_GCC_HPP_
#define SHARE_VM_UTILITIES_GLOBALDEFINITIONS_GCC_HPP_

#include "prims/jni.h"

typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;

typedef int				   intptr_t;
typedef unsigned int	   uintptr_t;

typedef uint8_t	           jubyte;
typedef uint16_t 		   jushort;
typedef uint32_t 		   juint;
typedef uint64_t 		   julong;


#endif /* SHARE_VM_UTILITIES_GLOBALDEFINITIONS_GCC_HPP_ */


/*
#ifndef SHARE_VM_UTILITIES_GLOBALDEFINITIONS_GCC_HPP_
#define SHARE_VM_UTILITIES_GLOBALDEFINITIONS_GCC_HPP_

#include "prims/jni.h"

// This file holds compiler-dependent includes,
// globally used constants & types, class (forward)
// declarations and a few frequently used utility functions.

#include <ctype.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

#include <math.h>
#ifndef FP_PZERO
// Linux doesn't have positive/negative zero
#define FP_PZERO FP_ZERO
#endif
#if (!defined fpclass) && ((!defined SPARC) || (!defined SOLARIS))
#define fpclass fpclassify
#endif

#include <time.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <pthread.h>

#include <limits.h>
#include <errno.h>

# ifdef SOLARIS_MUTATOR_LIBTHREAD
# include <sys/procfs.h>
# endif

#if defined(LINUX) || defined(_ALLBSD_SOURCE)
#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif // __STDC_LIMIT_MACROS
#include <inttypes.h>
#include <signal.h>
#ifndef __OpenBSD__
#include <ucontext.h>
#endif
#ifdef __APPLE__
  #include <AvailabilityMacros.h>
  #include <mach/mach.h>
#endif
#include <sys/time.h>
#endif // LINUX || _ALLBSD_SOURCE

// NULL vs NULL_WORD:
// On Linux NULL is defined as a special type '__null'. Assigning __null to
// integer variable will cause gcc warning. Use NULL_WORD in places where a
// pointer is stored as integer value.  On some platforms, sizeof(intptr_t) >
// sizeof(void*), so here we want something which is integer type, but has the
// same size as a pointer.
#ifdef __GNUC__
  #ifdef _LP64
    #define NULL_WORD  0L
  #else
    // Cast 0 to intptr_t rather than int32_t since they are not the same type
    // on platforms such as Mac OS X.
    #define NULL_WORD  ((intptr_t)0)
  #endif
#else
  #define NULL_WORD  NULL
#endif

#if !defined(LINUX) && !defined(_ALLBSD_SOURCE)
// Compiler-specific primitive types
typedef unsigned short     uint16_t;
#ifndef _UINT32_T
#define _UINT32_T
typedef unsigned int       uint32_t;
#endif // _UINT32_T

#if !defined(_SYS_INT_TYPES_H)
#ifndef _UINT64_T
#define _UINT64_T
typedef unsigned long long uint64_t;
#endif // _UINT64_T
// %%%% how to access definition of intptr_t portably in 5.5 onward?
typedef int                     intptr_t;
typedef unsigned int            uintptr_t;
// If this gets an error, figure out a symbol XXX that implies the
// prior definition of intptr_t, and add "&& !defined(XXX)" above.
#endif // _SYS_INT_TYPES_H

#endif // !LINUX && !_ALLBSD_SOURCE

// Additional Java basic types

typedef uint8_t  jubyte;
typedef uint16_t jushort;
typedef uint32_t juint;
typedef uint64_t julong;

//----------------------------------------------------------------------------------------------------
// Special (possibly not-portable) casts
// Cast floats into same-size integers and vice-versa w/o changing bit-pattern
// %%%%%% These seem like standard C++ to me--how about factoring them out? - Ungar

inline jint    jint_cast   (jfloat  x)           { return *(jint*   )&x; }
inline jlong   jlong_cast  (jdouble x)           { return *(jlong*  )&x; }
inline julong  julong_cast (jdouble x)           { return *(julong* )&x; }

inline jfloat  jfloat_cast (jint    x)           { return *(jfloat* )&x; }
inline jdouble jdouble_cast(jlong   x)           { return *(jdouble*)&x; }

//----------------------------------------------------------------------------------------------------
// Constant for jlong (specifying an long long canstant is C++ compiler specific)

// Build a 64bit integer constant
#define CONST64(x)  (x ## LL)
#define UCONST64(x) (x ## ULL)

const jlong min_jlong = CONST64(0x8000000000000000);
const jlong max_jlong = CONST64(0x7fffffffffffffff);


//----------------------------------------------------------------------------------------------------
// Debugging

#define DEBUG_EXCEPTION ::abort();

#ifdef ARM
	#ifdef SOLARIS
		#define BREAKPOINT __asm__ volatile (".long 0xe1200070")
	#else
		#define BREAKPOINT __asm__ volatile (".long 0xe7f001f0")
	#endif
#else
	extern "C" void breakpoint();
	#define BREAKPOINT ::breakpoint()
#endif

// checking for nanness
#ifdef SOLARIS
	#ifdef SPARC
		inline int g_isnan(float  f) { return isnanf(f); }
	#else
		// isnanf() broken on Intel Solaris use isnand()
		inline int g_isnan(float  f) { return isnand(f); }
	#endif
	inline int g_isnan(double f) { return isnand(f); }
#elif defined(__APPLE__)
	inline int g_isnan(double f) { return isnan(f); }
#elif defined(LINUX) || defined(_ALLBSD_SOURCE)
	inline int g_isnan(float  f) { return isnanf(f); }
	inline int g_isnan(double f) { return isnan(f); }
#else
	#error "missing platform-specific definition here"
#endif

// GCC 4.3 does not allow 0.0/0.0 to produce a NAN value
#if (__GNUC__ == 4) && (__GNUC_MINOR__ > 2)
#define CAN_USE_NAN_DEFINE 1
#endif


// Checking for finiteness

inline int g_isfinite(jfloat  f)                 { return finite(f); }
inline int g_isfinite(jdouble f)                 { return finite(f); }


// Wide characters

inline int wcslen(const jchar* x) { return wcslen((const wchar_t*)x); }


// Portability macros
#define PRAGMA_INTERFACE             #pragma interface
#define PRAGMA_IMPLEMENTATION        #pragma implementation
#define VALUE_OBJ_CLASS_SPEC

#if (__GNUC__ == 2) && (__GNUC_MINOR__ < 95)
#define TEMPLATE_TABLE_BUG
#endif
#if (__GNUC__ == 2) && (__GNUC_MINOR__ >= 96)
#define CONST_SDM_BUG
#endif

// Formatting.
#ifdef _LP64
#define FORMAT64_MODIFIER "l"
#else // !_LP64
#define FORMAT64_MODIFIER "ll"
#endif // _LP64

// HACK: gcc warns about applying offsetof() to non-POD object or calculating
//       offset directly when base address is NULL. Use 16 to get around the
//       warning. gcc-3.4 has an option -Wno-invalid-offsetof to suppress
//       this warning.
#define offset_of(klass,field) (size_t)((intx)&(((klass*)16)->field) - 16)

#ifdef offsetof
# undef offsetof
#endif
#define offsetof(klass,field) offset_of(klass,field)

#if defined(_LP64) && defined(__APPLE__)
#define JLONG_FORMAT           "%ld"
#endif // _LP64 && __APPLE__

#endif // SHARE_VM_UTILITIES_GLOBALDEFINITIONS_GCC_HPP_
*/
