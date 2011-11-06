/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkConfigure.h.in

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#ifndef __vtkConfigure_h
#define __vtkConfigure_h

/* This header is configured by VTK's build process.  */

/*--------------------------------------------------------------------------*/
/* Platform Features                                                        */

/* Byte order.  */
/* All compilers that support Mac OS X define either __BIG_ENDIAN__ or
   __LITTLE_ENDIAN__ to match the endianness of the architecture being
   compiled for. This is not necessarily the same as the architecture of the
   machine doing the building. In order to support Universal Binaries on
   Mac OS X, we prefer those defines to decide the endianness.
   On other platforms we use the result of the TRY_RUN. */
#if !defined(__APPLE__)
/* #undef VTK_WORDS_BIGENDIAN */
#elif defined(__BIG_ENDIAN__)
# define VTK_WORDS_BIGENDIAN
#endif

/* Threading system.  */
#define VTK_USE_PTHREADS
/* #undef VTK_USE_SPROC */
/* #undef VTK_HP_PTHREADS */
/* #undef VTK_USE_WIN32_THREADS */
# define VTK_MAX_THREADS 64

/* Size of fundamental data types.  */
/* Mac OS X uses two data models, ILP32 (in which integers, long integers,
   and pointers are 32-bit quantities) and LP64 (in which integers are 32-bit
   quantities and long integers and pointers are 64-bit quantities). In order
   to support Universal Binaries on Mac OS X, we rely on this knowledge
   instead of testing the sizes of the building machine.
   On other platforms we use the result of the TRY_RUN. */
#if !defined(__APPLE__)
# define VTK_SIZEOF_CHAR   1
# define VTK_SIZEOF_SHORT  2
# define VTK_SIZEOF_INT    4
# define VTK_SIZEOF_LONG   8
# define VTK_SIZEOF_FLOAT  4
# define VTK_SIZEOF_DOUBLE 8
# define VTK_SIZEOF_VOID_P 8
#else
# define VTK_SIZEOF_CHAR   1
# define VTK_SIZEOF_SHORT  2
# define VTK_SIZEOF_INT    4
# if defined(__LP64__) && __LP64__
#  define VTK_SIZEOF_LONG  8
#  define VTK_SIZEOF_VOID_P 8
# else
#  define VTK_SIZEOF_LONG  4
#  define VTK_SIZEOF_VOID_P 4
# endif
# define VTK_SIZEOF_FLOAT  4
# define VTK_SIZEOF_DOUBLE 8
#endif

/* Define size of long long and/or __int64 bit integer type only if the type
   exists.  */
#if !defined(__APPLE__)
 #define VTK_SIZEOF_LONG_LONG 8
#else
 #define VTK_SIZEOF_LONG_LONG 8
#endif
/* #undef VTK_SIZEOF___INT64 */

/* Whether types "long long" and "__int64" are enabled.  If a type is
   enabled then it is a unique fundamental type.  */
#define VTK_TYPE_USE_LONG_LONG
/* #undef VTK_TYPE_USE___INT64 */

/* Some properties of the available types.  */
/* #undef VTK_TYPE_SAME_LONG_AND___INT64 */
/* #undef VTK_TYPE_SAME_LONG_LONG_AND___INT64 */
/* #undef VTK_TYPE_CONVERT_UI64_TO_DOUBLE */

/* Whether type "char" is signed (it may be signed or unsigned).  */
#define VTK_TYPE_CHAR_IS_SIGNED 1

/* Compiler features.  */
/* #undef VTK_NO_EXPLICIT_TEMPLATE_INSTANTIATION */
/* #undef VTK_NO_FULL_TEMPLATE_SPECIALIZATION */
/* #undef VTK_NO_ANSI_STRING_STREAM */
/* #undef VTK_NO_STD_NAMESPACE */
/* #undef VTK_NO_FOR_SCOPE */
#define VTK_COMPILER_HAS_BOOL
#define VTK_ISTREAM_SUPPORTS_LONG_LONG
#define VTK_OSTREAM_SUPPORTS_LONG_LONG
#define VTK_STREAM_EOF_SEVERITY 0
#define VTK_HAVE_GETSOCKNAME_WITH_SOCKLEN_T
#define VTK_HAVE_SO_REUSEADDR

/* Whether we require large files support.  */
#define VTK_REQUIRE_LARGE_FILE_SUPPORT

/* Whether reverse const iterator's have comparison operators. */
#define VTK_CONST_REVERSE_ITERATOR_COMPARISON

/*--------------------------------------------------------------------------*/
/* VTK Platform Configuration                                               */

/* Whether the target platform supports shared libraries.  */
#define VTK_TARGET_SUPPORTS_SHARED_LIBS

/* Whether we are building shared libraries.  */
/* #undef VTK_BUILD_SHARED_LIBS */

/* Whether vtkIdType is a 64-bit integer type (or a 32-bit integer type).  */
#define VTK_USE_64BIT_IDS

/* Whether we are using ANSI C++ streams (or old-style streams).  */
#define VTK_USE_ANSI_STDLIB

/* Whether VTK was built to support Carbon or Cocoa on the Mac.  */
/* #undef VTK_USE_CARBON */
#define VTK_USE_COCOA

/* Whether we are linking to Tcl/Tk statically.  */
/* #undef VTK_TCL_TK_STATIC */

/* Whether Tk widgets are NOT initialized when vtkRendering loads.  */
/* #undef VTK_USE_TK */

/* Whether the Tcl/Tk support files are copied to the build dir */
/* #undef VTK_TCL_TK_COPY_SUPPORT_LIBRARY */

/* Configure internal Tk headers.  */
#ifdef VTK_USE_CARBON
# define USE_NON_CONST
# define MAC_OSX_TK
#endif

/* Whether the Boost library is being used */
/* #undef VTK_USE_BOOST */

/* Whether Gnu R can be used */
/* #undef VTK_USE_GNU_R */

/* Whether the Qt files are available */
/* #undef VTK_USE_QT */

/* Whether N-way arrays are being used */
#define VTK_USE_N_WAY_ARRAYS

/*--------------------------------------------------------------------------*/
/* VTK Versioning                                                           */

/* Version number.  */
#define VTK_MAJOR_VERSION 5
#define VTK_MINOR_VERSION 8
#define VTK_BUILD_VERSION 0
#define VTK_VERSION "5.8.0"

/* C++ compiler used.  */
#define VTK_CXX_COMPILER "/usr/bin/g++"

/* Compatibility settings.  */
/* #undef VTK_LEGACY_REMOVE */
/* #undef VTK_LEGACY_SILENT */

/*--------------------------------------------------------------------------*/
/* Setup VTK based on platform features and configuration.                  */

/* Setup vtkstd, a portable namespace for std.  */
#ifndef VTK_NO_STD_NAMESPACE
# define vtkstd std
#else
# define vtkstd
#endif

/* Define a "vtkstd_bool" type.  This should be used as the
   return type of comparison operators to keep STL happy on all
   platforms.  It should not be used elsewhere.  Only use bool
   if this file is included by a c++ file. */
#if defined(VTK_COMPILER_HAS_BOOL) && defined(__cplusplus)
typedef bool vtkstd_bool;
#else
typedef int vtkstd_bool;
#endif

/* Define a macro to help define template specializations.  Skip if
   compiling a windows resource file because the resource compiler
   warns about truncating the long symbol.  */
#if !defined(RC_INVOKED)
# if defined(VTK_NO_FULL_TEMPLATE_SPECIALIZATION)
#  define VTK_TEMPLATE_SPECIALIZE
# else
#  define VTK_TEMPLATE_SPECIALIZE template <>
# endif
#endif

/* #undef VTK_USE_FOR_SCOPE_WORKAROUND */

/* Use the common for-scope work-around when compiling a source in VTK.  */
/* External Projects can set VTK_USE_FOR_SCOPE_WORKAROUND to get this "fix" */
#if (defined(VTK_IN_VTK) && !defined(VTK_NO_WORKAROUND_FOR_SCOPE)) || defined(VTK_USE_FOR_SCOPE_WORKAROUND)
# define VTK_WORKAROUND_FOR_SCOPE
#endif
#if defined(VTK_NO_FOR_SCOPE) && defined(VTK_WORKAROUND_FOR_SCOPE)
# ifndef for
#  define for if(0) {} else for
#  if defined(_MSC_VER)
#   pragma warning (disable: 4127) /* conditional expression is constant */
#  endif
# endif
#endif

/* Provide missing streaming operators.  */
#if defined(VTK_SIZEOF_LONG_LONG)
# if !defined(VTK_OSTREAM_SUPPORTS_LONG_LONG)
#  define VTK_IOSTREAM_NEED_OPERATORS_LL
# elif !defined(VTK_ISTREAM_SUPPORTS_LONG_LONG)
#  define VTK_IOSTREAM_NEED_OPERATORS_LL
# endif
# if defined(VTK_IOSTREAM_NEED_OPERATORS_LL)
   typedef long long vtkIOStreamSLL;
   typedef unsigned long long vtkIOStreamULL;
# endif
#elif defined(VTK_SIZEOF___INT64)
# if defined(_MSC_VER) && (_MSC_VER < 1300)
#  define VTK_IOSTREAM_NEED_OPERATORS_LL
   typedef __int64 vtkIOStreamSLL;
   typedef unsigned __int64 vtkIOStreamULL;
# endif
#endif

/* Set the whether we have UINTPTR_T defined in support of the GNU R interface */
#define HAVE_VTK_UINTPTR_T
#define VTK_R_HOME ""

#endif
