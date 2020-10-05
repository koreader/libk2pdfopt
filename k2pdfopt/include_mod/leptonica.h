/*====================================================================*
 -  Copyright (C) 2001 Leptonica.  All rights reserved.
 -
 -  Redistribution and use in source and binary forms, with or without
 -  modification, are permitted provided that the following conditions
 -  are met:
 -  1. Redistributions of source code must retain the above copyright
 -     notice, this list of conditions and the following disclaimer.
 -  2. Redistributions in binary form must reproduce the above
 -     copyright notice, this list of conditions and the following
 -     disclaimer in the documentation and/or other materials
 -     provided with the distribution.
 -
 -  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 -  ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 -  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 -  A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL ANY
 -  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 -  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 -  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 -  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 -  OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 -  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 -  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *====================================================================*/

#ifndef  LEPTONICA_ALLHEADERS_H
#define  LEPTONICA_ALLHEADERS_H


#define LIBLEPT_MAJOR_VERSION   1
#define LIBLEPT_MINOR_VERSION   79
#define LIBLEPT_PATCH_VERSION   0

/*====================================================================*
 -  Copyright (C) 2001 Leptonica.  All rights reserved.
 -
 -  Redistribution and use in source and binary forms, with or without
 -  modification, are permitted provided that the following conditions
 -  are met:
 -  1. Redistributions of source code must retain the above copyright
 -     notice, this list of conditions and the following disclaimer.
 -  2. Redistributions in binary form must reproduce the above
 -     copyright notice, this list of conditions and the following
 -     disclaimer in the documentation and/or other materials
 -     provided with the distribution.
 -
 -  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 -  ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 -  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 -  A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL ANY
 -  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 -  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 -  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 -  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 -  OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 -  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 -  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *====================================================================*/

#ifndef  LEPTONICA_ALLTYPES_H
#define  LEPTONICA_ALLTYPES_H

    /* Standard */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

    /* General and configuration defs */
/* willus mod */
#if !defined (L_BIG_ENDIAN) && !defined (L_LITTLE_ENDIAN)
#define L_LITTLE_ENDIAN
#endif
/*====================================================================*
 -  Copyright (C) 2001 Leptonica.  All rights reserved.
 -
 -  Redistribution and use in source and binary forms, with or without
 -  modification, are permitted provided that the following conditions
 -  are met:
 -  1. Redistributions of source code must retain the above copyright
 -     notice, this list of conditions and the following disclaimer.
 -  2. Redistributions in binary form must reproduce the above
 -     copyright notice, this list of conditions and the following
 -     disclaimer in the documentation and/or other materials
 -     provided with the distribution.
 -
 -  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 -  ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 -  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 -  A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL ANY
 -  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 -  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 -  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 -  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 -  OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 -  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 -  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *====================================================================*/

#ifndef  LEPTONICA_ENVIRON_H
#define  LEPTONICA_ENVIRON_H

/*------------------------------------------------------------------------*
 *  Defines and includes differ for Unix and Windows.  Also for Windows,  *
 *  differentiate between conditionals based on platform and compiler.    *
 *      For platforms:                                                    *
 *          _WIN32       =>     Windows, 32- or 64-bit                    *
 *          _WIN64       =>     Windows, 64-bit only                      *
 *          __CYGWIN__   =>     Cygwin                                    *
 *      For compilers:                                                    *
 *          __GNUC__     =>     gcc                                       *
 *          _MSC_VER     =>     msvc                                      *
 *------------------------------------------------------------------------*/

/* MS VC++ does not provide stdint.h, so define the missing types here */


#ifndef _MSC_VER
#include <stdint.h>

#else
/* Note that _WIN32 is defined for both 32 and 64 bit applications,
   whereas _WIN64 is defined only for the latter */

#ifdef _WIN64
typedef __int64 intptr_t;
typedef unsigned __int64 uintptr_t;
#else
typedef int intptr_t;
typedef unsigned int uintptr_t;
#endif

/* VC++6 doesn't seem to have powf, expf. */
#if (_MSC_VER < 1400)
#define powf(x, y) (float)pow((double)(x), (double)(y))
#define expf(x) (float)exp((double)(x))
#endif

#endif /* _MSC_VER */

/* Windows specifics */
#ifdef _WIN32
  /* DLL EXPORTS and IMPORTS */
  #if defined(LIBLEPT_EXPORTS)
    #define LEPT_DLL __declspec(dllexport)
  #elif defined(LIBLEPT_IMPORTS)
    #define LEPT_DLL __declspec(dllimport)
  #else
    #define LEPT_DLL
  #endif
#else  /* non-Windows specifics */
  #include <stdint.h>
  #define LEPT_DLL
#endif  /* _WIN32 */

typedef intptr_t l_intptr_t;
typedef uintptr_t l_uintptr_t;


/*--------------------------------------------------------------------*
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*
 *                          USER CONFIGURABLE                         *
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*
 *               Environment variables with I/O libraries             *
 *               Manual Configuration Only: NOT AUTO_CONF             *
 *--------------------------------------------------------------------*/
/*
 *  Leptonica provides interfaces to link to several external image
 *  I/O libraries, plus zlib.  Setting any of these to 0 here causes
 *  non-functioning stubs to be linked.
 */
#if !defined(HAVE_CONFIG_H) && !defined(ANDROID_BUILD) && !defined(OS_IOS)

  #if !defined(HAVE_LIBJPEG)
  #define  HAVE_LIBJPEG       1
  #endif
  #if !defined(HAVE_LIBTIFF)
  #define  HAVE_LIBTIFF       0
  #endif
  #if !defined(HAVE_LIBPNG)
  #define  HAVE_LIBPNG        1
  #endif
  #if !defined(HAVE_LIBZ)
  #define  HAVE_LIBZ          1
  #endif
  #if !defined(HAVE_LIBGIF)
  #define  HAVE_LIBGIF        0
  #endif
  #if !defined(HAVE_LIBUNGIF)
  #define  HAVE_LIBUNGIF      0
  #endif
  #if !defined(HAVE_LIBWEBP)
  #define  HAVE_LIBWEBP       0
  #endif
  #if !defined(HAVE_LIBWEBP_ANIM)
  #define  HAVE_LIBWEBP_ANIM  0
  #endif
  #if !defined(HAVE_LIBJP2K)
  #define  HAVE_LIBJP2K       0
  #endif


  /*-----------------------------------------------------------------------*
   * Leptonica supports OpenJPEG 2.0+.  If you have a version of openjpeg  *
   * (HAVE_LIBJP2K == 1) that is >= 2.0, set the path to the openjpeg.h    *
   * header in angle brackets here.                                        *
   *-----------------------------------------------------------------------*/
  #define  LIBJP2K_HEADER   <openjpeg-2.3/openjpeg.h>

#endif  /* ! HAVE_CONFIG_H etc. */

/*--------------------------------------------------------------------*
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*
 *                          USER CONFIGURABLE                         *
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*
 *     Environ variables for image I/O without external libraries     *
 *--------------------------------------------------------------------*/
/*
 *  Leptonica supplies I/O support without using external libraries for:
 *     * image read/write for bmp, pnm
 *     * header read for jp2k
 *     * image wrapping write for pdf and ps.
 *  Setting any of these to 0 causes non-functioning stubs to be linked.
 */
#define  USE_BMPIO        1
#define  USE_PNMIO        1
#define  USE_JP2KHEADER   1
#define  USE_PDFIO        1
#define  USE_PSIO         1


/*-------------------------------------------------------------------------*
 * On linux systems, you can do I/O between Pix and memory.  Specifically,
 * you can compress (write compressed data to memory from a Pix) and
 * uncompress (read from compressed data in memory to a Pix).
 * For jpeg, png, jp2k, gif, pnm and bmp, these use the non-posix GNU
 * functions fmemopen() and open_memstream().  These functions are not
 * available on other systems.
 * To use these functions in linux, you must define HAVE_FMEMOPEN to 1.
 * To use them on MacOS, which does not support these functions, set it to 0.
 *-------------------------------------------------------------------------*/
#if !defined(HAVE_CONFIG_H) && !defined(ANDROID_BUILD) && !defined(OS_IOS) && \
    !defined(_WIN32)
#define  HAVE_FMEMOPEN    1
#endif  /* ! HAVE_CONFIG_H etc. */
/* willus mod */
#ifdef HAVE_FMEMOPEN
#undef HAVE_FMEMOPEN
#endif
#define HAVE_FMEMOPEN 0

/*-------------------------------------------------------------------------*
 * fstatat() is defined by POSIX, but some systems do not support it.      *
 * One example is older macOS systems (pre-10.10).                         *
 * Play it safe and set the default value to 0.                            *
 *-------------------------------------------------------------------------*/
#if !defined(HAVE_CONFIG_H)
#define  HAVE_FSTATAT     0
#endif /* ! HAVE_CONFIG_H */

/*--------------------------------------------------------------------*
 * It is desirable on Windows to have all temp files written to the same
 * subdirectory of the Windows <Temp> directory, because files under <Temp>
 * persist after reboot, and the regression tests write a lot of files.
 * We write all test files to /tmp/lept or subdirectories of /tmp/lept.
 * Windows temp files are specified as in unix, but have the translation
 *        /tmp/lept/xxx  -->   <Temp>/lept/xxx
 *--------------------------------------------------------------------*/


/*--------------------------------------------------------------------*
 *                          Built-in types                            *
 *--------------------------------------------------------------------*/
typedef int                     l_ok;    /*!< return type 0 if OK, 1 on error */
typedef signed char             l_int8;     /*!< signed 8-bit value */
typedef unsigned char           l_uint8;    /*!< unsigned 8-bit value */
typedef short                   l_int16;    /*!< signed 16-bit value */
typedef unsigned short          l_uint16;   /*!< unsigned 16-bit value */
typedef int                     l_int32;    /*!< signed 32-bit value */
typedef unsigned int            l_uint32;   /*!< unsigned 32-bit value */
typedef float                   l_float32;  /*!< 32-bit floating point value */
typedef double                  l_float64;  /*!< 64-bit floating point value */
#ifdef COMPILER_MSVC
typedef __int64                 l_int64;    /*!< signed 64-bit value */
typedef unsigned __int64        l_uint64;   /*!< unsigned 64-bit value */
#else
typedef long long               l_int64;    /*!< signed 64-bit value */
typedef unsigned long long      l_uint64;   /*!< unsigned 64-bit value */
#endif  /* COMPILER_MSVC */


/*-------------------------------------------------------------------------*
 * For security, the library is distributed in a configuration that does   *
 * not permit (1) forking with 'system', which is used for displaying      *
 * images and generating gnuplots, and (2) writing files with specified    *
 * compiled-in file names.  All such writes are with functions such as     *
 * pixWriteDebug() where the "Debug" is appended to the usual name.        *
 * Whether the "Debug" version defaults to the standard version or is a    *
 * no-op depends on the value of this global variable.  The default value  *
 * of LeptDebugOK is 0, and it is set in writefile.c.  This value can be   *
 * over-ridden, for development and debugging, by setLeptDebugOK().        *
 *-------------------------------------------------------------------------*/
LEPT_DLL extern l_int32  LeptDebugOK;  /* default is 0 */


/*------------------------------------------------------------------------*
 *                            Standard macros                             *
 *------------------------------------------------------------------------*/
#ifndef L_MIN
/*! Minimum of %x and %y */
#define L_MIN(x, y)   (((x) < (y)) ? (x) : (y))
#endif

#ifndef L_MAX
/*! Maximum of %x and %y */
#define L_MAX(x, y)   (((x) > (y)) ? (x) : (y))
#endif

#ifndef L_ABS
/*! Absolute value of %x */
#define L_ABS(x)     (((x) < 0) ? (-1 * (x)) : (x))
#endif

#ifndef L_SIGN
/*! Sign of %x */
#define L_SIGN(x)    (((x) < 0) ? -1 : 1)
#endif

#ifndef UNDEF
/*! Undefined value */
#define UNDEF        -1
#endif

#ifndef NULL
/*! NULL value */
#define NULL          0
#endif

#ifndef TRUE
/*! True value */
#define TRUE          1
#endif

#ifndef FALSE
/*! False value */
#define FALSE         0
#endif


/*--------------------------------------------------------------------*
 *            Environment variables for endian dependence             *
 *--------------------------------------------------------------------*/
/*
 *  To control conditional compilation, one of two variables
 *
 *       L_LITTLE_ENDIAN  (e.g., for Intel X86)
 *       L_BIG_ENDIAN     (e.g., for Sun SPARC, Mac Power PC)
 *
 *  is defined when the GCC compiler is invoked.
 *  All code should compile properly for both hardware architectures.
 */


/*------------------------------------------------------------------------*
 *                    Simple search state variables                       *
 *------------------------------------------------------------------------*/
/*! Search State */
enum {
    L_NOT_FOUND = 0,
    L_FOUND = 1
};


/*------------------------------------------------------------------------*
 *                     Path separator conversion                          *
 *------------------------------------------------------------------------*/
/*! Path Separators */
enum {
    UNIX_PATH_SEPCHAR = 0,
    WIN_PATH_SEPCHAR = 1
};


/*------------------------------------------------------------------------*
 *                          Timing structs                                *
 *------------------------------------------------------------------------*/
typedef void *L_TIMER;

/*! Timing struct */
struct L_WallTimer {
    l_int32  start_sec;
    l_int32  start_usec;
    l_int32  stop_sec;
    l_int32  stop_usec;
};
typedef struct L_WallTimer  L_WALLTIMER;


/*------------------------------------------------------------------------*
 *                      Standard memory allocation                        *
 *                                                                        *
 *  These specify the memory management functions that are used           *
 *  on all heap data except for Pix.  Memory management for Pix           *
 *  also defaults to malloc and free.  See pix1.c for details.            *
 *------------------------------------------------------------------------*/
#define LEPT_MALLOC(blocksize)           malloc(blocksize)
#define LEPT_CALLOC(numelem, elemsize)   calloc(numelem, elemsize)
#define LEPT_REALLOC(ptr, blocksize)     realloc(ptr, blocksize)
#define LEPT_FREE(ptr)                   free(ptr)


/*------------------------------------------------------------------------*
 *         Control printing of error, warning, and info messages          *
 *                                                                        *
 *  Leptonica never sends output to stdout.  By default, all messages     *
 *  go to stderr.  However, we provide a mechanism for runtime            *
 *  redirection of output, using a custom stderr handler defined          *
 *  by the user.  See utils1.c for details and examples.                  *
 *                                                                        *
 *  To omit all messages to stderr, simply define NO_CONSOLE_IO on the    *
 *  command line.  For finer grained control, we have a mechanism         *
 *  based on the message severity level.  The following assumes that      *
 *  NO_CONSOLE_IO is not defined.                                         *
 *                                                                        *
 *  Messages are printed if the message severity is greater than or equal *
 *  to the current severity threshold.  The current severity threshold    *
 *  is the greater of the compile-time severity, which is the minimum     *
 *  severity that can be reported, and the run-time severity, which is    *
 *  the severity threshold at the moment.                                 *
 *                                                                        *
 *  The compile-time threshold determines which messages are compiled     *
 *  into the library for potential printing.  Messages below the          *
 *  compile-time threshold are omitted and can never be printed.  The     *
 *  default compile-time threshold is L_SEVERITY_INFO, but this may be    *
 *  overridden by defining MINIMUM_SEVERITY to the desired enumeration    *
 *  identifier on the compiler command line.  Defining NO_CONSOLE_IO on   *
 *  the command line is the same as setting MINIMUM_SEVERITY to           *
 *  L_SEVERITY_NONE.                                                      *
 *                                                                        *
 *  The run-time threshold determines which messages are printed during   *
 *  library execution.  It defaults to the compile-time threshold but     *
 *  may be changed either statically by defining DEFAULT_SEVERITY to      *
 *  the desired enumeration identifier on the compiler command line, or   *
 *  dynamically by calling setMsgSeverity() to specify a new threshold.   *
 *  The run-time threshold may also be set from the value of the          *
 *  environment variable LEPT_MSG_SEVERITY by calling setMsgSeverity()   *
 *  and specifying L_SEVERITY_EXTERNAL.                                   *
 *                                                                        *
 *  In effect, the compile-time threshold setting says, "Generate code    *
 *  to permit messages of equal or greater severity than this to be       *
 *  printed, if desired," whereas the run-time threshold setting says,    *
 *  "Print messages that have an equal or greater severity than this."    *
 *------------------------------------------------------------------------*/

    /*! Control printing of error, warning and info messages */
/*! Message Control */
enum {
    L_SEVERITY_EXTERNAL = 0,   /* Get the severity from the environment   */
    L_SEVERITY_ALL      = 1,   /* Lowest severity: print all messages     */
    L_SEVERITY_DEBUG    = 2,   /* Print debugging and higher messages     */
    L_SEVERITY_INFO     = 3,   /* Print informational and higher messages */
    L_SEVERITY_WARNING  = 4,   /* Print warning and higher messages       */
    L_SEVERITY_ERROR    = 5,   /* Print error and higher messages         */
    L_SEVERITY_NONE     = 6    /* Highest severity: print no messages     */
};

/*  No message less than the compile-time threshold will ever be
 *  reported, regardless of the current run-time threshold.  This allows
 *  selection of the set of messages to include in the library.  For
 *  example, setting the threshold to L_SEVERITY_WARNING eliminates all
 *  informational messages from the library.  With that setting, both
 *  warning and error messages would be printed unless setMsgSeverity()
 *  was called, or DEFAULT_SEVERITY was redefined, to set the run-time
 *  severity to L_SEVERITY_ERROR.  In that case, only error messages
 *  would be printed.
 *
 *  This mechanism makes the library smaller and faster, by eliminating
 *  undesired message reporting and the associated run-time overhead for
 *  message threshold checking, because code for messages whose severity
 *  is lower than MINIMUM_SEVERITY won't be generated.
 *
 *  A production library might typically permit ERROR messages to be
 *  generated, and a development library might permit DEBUG and higher.
 *  The actual messages printed (as opposed to generated) would depend
 *  on the current run-time severity threshold.
 *
 *  This is a complex mechanism and a few examples may help.
 *  (1) No output permitted under any circumstances.
 *      Use:  -DNO_CONSOLE_IO  or  -DMINIMUM_SEVERITY=6
 *  (2) Suppose you want to only allow error messages, and you don't
 *      want to permit info or warning messages at runtime.
 *      Use:  -DMINIMUM_SEVERITY=5
 *  (3) Suppose you want to only allow error messages by default,
 *      but you will permit this to be over-ridden at runtime.
 *      Use:  -DDEFAULT_SEVERITY=5
 *            and to allow info and warning override:
 *                 setMsgSeverity(L_SEVERITY_INFO);
 */

#ifdef  NO_CONSOLE_IO
  #undef MINIMUM_SEVERITY
  #undef DEFAULT_SEVERITY

  #define MINIMUM_SEVERITY      L_SEVERITY_NONE    /*!< Compile-time default */
  #define DEFAULT_SEVERITY      L_SEVERITY_NONE    /*!< Run-time default */

#else
  #ifndef MINIMUM_SEVERITY
    #define MINIMUM_SEVERITY    L_SEVERITY_INFO    /*!< Compile-time default */
  #endif

  #ifndef DEFAULT_SEVERITY
    #define DEFAULT_SEVERITY    MINIMUM_SEVERITY   /*!< Run-time default */
  #endif
#endif


/*!  The run-time message severity threshold is defined in utils1.c.  */
LEPT_DLL extern l_int32  LeptMsgSeverity;

/*
 * <pre>
 *  Usage
 *  =====
 *  Messages are of two types.
 *
 *  (1) The messages
 *      ERROR_INT(a,b,c)       : returns l_int32
 *      ERROR_FLOAT(a,b,c)     : returns l_float32
 *      ERROR_PTR(a,b,c)       : returns void*
 *  are used to return from functions and take a fixed set of parameters:
 *      a : <message string>
 *      b : procName
 *      c : <return value from function>
 *  where procName is the name of the local variable naming the function.
 *
 *  (2) The purely informational L_* messages
 *      L_ERROR(a,...)
 *      L_WARNING(a,...)
 *      L_INFO(a,...)
 *  do not take a return value, but they take at least two parameters:
 *      a  :  <message string> with optional format conversions
 *      v1 : procName    (this must be included as the first vararg)
 *      v2, ... :  optional varargs to match format converters in the message
 *
 *  To return an error from a function that returns void, use:
 *      L_ERROR(<message string>, procName, [...])
 *      return;
 *
 *  Implementation details
 *  ======================
 *  Messages are defined with the IF_SEV macro.  The first parameter is
 *  the message severity, the second is the function to call if the
 *  message is to be printed, and the third is the return value if the
 *  message is to be suppressed.  For example, we might have an
 *  informational message defined as:
 *
 *    IF_SEV(L_SEVERITY_INFO, fprintf(.......), 0)
 *
 *  The macro expands into a conditional.  Because the first comparison
 *  is between two constants, an optimizing compiler will remove either
 *  the comparison (if it's true) or the entire macro expansion (if it
 *  is false).  This means that there is no run-time overhead for
 *  messages whose severity falls below the minimum specified at compile
 *  time, and for others the overhead is one (not two) comparisons.
 *
 *  The L_nnn() macros below do not return a value, but because the
 *  conditional operator requires one for the false condition, we
 *  specify a void expression.
 * </pre>
 */

#ifdef  NO_CONSOLE_IO

  #define PROCNAME(name)
  #define ERROR_INT(a, b, c)            ((l_int32)(c))
  #define ERROR_FLOAT(a, b, c)          ((l_float32)(c))
  #define ERROR_PTR(a, b, c)            ((void *)(c))
  #define L_ERROR(a, ...)
  #define L_WARNING(a, ...)
  #define L_INFO(a, ...)

#else

  #define PROCNAME(name)              static const char procName[] = name
  #define IF_SEV(l, t, f) \
      ((l) >= MINIMUM_SEVERITY && (l) >= LeptMsgSeverity ? (t) : (f))

  #define ERROR_INT(a, b, c) \
      IF_SEV(L_SEVERITY_ERROR, returnErrorInt((a), (b), (c)), (l_int32)(c))
  #define ERROR_FLOAT(a, b, c) \
      IF_SEV(L_SEVERITY_ERROR, returnErrorFloat((a), (b), (c)), (l_float32)(c))
  #define ERROR_PTR(a, b, c) \
      IF_SEV(L_SEVERITY_ERROR, returnErrorPtr((a), (b), (c)), (void *)(c))

  #define L_ERROR(a, ...) \
      IF_SEV(L_SEVERITY_ERROR, \
             (void)lept_stderr("Error in %s: " a, __VA_ARGS__), \
             (void)0)
  #define L_WARNING(a, ...) \
      IF_SEV(L_SEVERITY_WARNING, \
             (void)lept_stderr("Warning in %s: " a, __VA_ARGS__), \
             (void)0)
  #define L_INFO(a, ...) \
      IF_SEV(L_SEVERITY_INFO, \
             (void)lept_stderr("Info in %s: " a, __VA_ARGS__), \
             (void)0)

#if 0  /* Alternative method for controlling L_* message output */
  #define L_ERROR(a, ...) \
    { if (L_SEVERITY_ERROR >= MINIMUM_SEVERITY && \
          L_SEVERITY_ERROR >= LeptMsgSeverity) \
          lept_stderr("Error in %s: " a, __VA_ARGS__) \
    }
  #define L_WARNING(a, ...) \
    { if (L_SEVERITY_WARNING >= MINIMUM_SEVERITY && \
          L_SEVERITY_WARNING >= LeptMsgSeverity) \
          lept_stderr("Warning in %s: " a, __VA_ARGS__) \
    }
  #define L_INFO(a, ...) \
    { if (L_SEVERITY_INFO >= MINIMUM_SEVERITY && \
          L_SEVERITY_INFO >= LeptMsgSeverity) \
          lept_stderr("Info in %s: " a, __VA_ARGS__) \
    }
#endif

#endif  /* NO_CONSOLE_IO */


/*------------------------------------------------------------------------*
 *              snprintf() renamed in MSVC (pre-VS2015)                   *
 *------------------------------------------------------------------------*/
#if defined _MSC_VER && _MSC_VER < 1900
#define snprintf(buf, size, ...)  _snprintf_s(buf, size, _TRUNCATE, __VA_ARGS__)
#endif


#endif /* LEPTONICA_ENVIRON_H */

    /* Generic and non-image-specific containers */
/*====================================================================*
 -  Copyright (C) 2001 Leptonica.  All rights reserved.
 -
 -  Redistribution and use in source and binary forms, with or without
 -  modification, are permitted provided that the following conditions
 -  are met:
 -  1. Redistributions of source code must retain the above copyright
 -     notice, this list of conditions and the following disclaimer.
 -  2. Redistributions in binary form must reproduce the above
 -     copyright notice, this list of conditions and the following
 -     disclaimer in the documentation and/or other materials
 -     provided with the distribution.
 -
 -  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 -  ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 -  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 -  A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL ANY
 -  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 -  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 -  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 -  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 -  OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 -  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 -  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *====================================================================*/

#ifndef  LEPTONICA_ARRAY_H
#define  LEPTONICA_ARRAY_H

/*!
 * \file array.h
 *
 * <pre>
 *  Contains the following structs:
 *      struct Numa
 *      struct Numaa
 *      struct L_Dna
 *      struct L_Dnaa
 *      struct L_DnaHash
 *      struct Sarray
 *      struct L_Bytea
 *
 *  Contains definitions for:
 *      Numa interpolation flags
 *      Numa and FPix border flags
 *      Numa data type conversion to string
 * </pre>
 */


/*------------------------------------------------------------------------*
 *                             Array Structs                              *
 *------------------------------------------------------------------------*/

/*! Numa version for serialization */
#define  NUMA_VERSION_NUMBER     1

    /*! Number array: an array of floats */
struct Numa
{
    l_int32          nalloc;    /*!< size of allocated number array      */
    l_int32          n;         /*!< number of numbers saved             */
    l_int32          refcount;  /*!< reference count (1 if no clones)    */
    l_float32        startx;    /*!< x value assigned to array[0]        */
    l_float32        delx;      /*!< change in x value as i --> i + 1    */
    l_float32       *array;     /*!< number array                        */
};
typedef struct Numa  NUMA;

    /*! Array of number arrays */
struct Numaa
{
    l_int32          nalloc;    /*!< size of allocated ptr array          */
    l_int32          n;         /*!< number of Numa saved                 */
    struct Numa    **numa;      /*!< array of Numa                        */
};
typedef struct Numaa  NUMAA;

/*! Dna version for serialization */
#define  DNA_VERSION_NUMBER     1

    /*! Double number array: an array of doubles */
struct L_Dna
{
    l_int32          nalloc;    /*!< size of allocated number array      */
    l_int32          n;         /*!< number of numbers saved             */
    l_int32          refcount;  /*!< reference count (1 if no clones)    */
    l_float64        startx;    /*!< x value assigned to array[0]        */
    l_float64        delx;      /*!< change in x value as i --> i + 1    */
    l_float64       *array;     /*!< number array                        */
};
typedef struct L_Dna  L_DNA;

    /*! Array of double number arrays */
struct L_Dnaa
{
    l_int32          nalloc;    /*!< size of allocated ptr array          */
    l_int32          n;         /*!< number of L_Dna saved                */
    struct L_Dna   **dna;       /*!< array of L_Dna                       */
};
typedef struct L_Dnaa  L_DNAA;

    /*! A hash table of Dnas */
struct L_DnaHash
{
    l_int32          nbuckets;
    l_int32          initsize;   /*!< initial size of each dna that is made  */
    struct L_Dna   **dna;        /*!< array of L_Dna                       */
};
typedef struct L_DnaHash L_DNAHASH;

/*! Sarray version for serialization */
#define  SARRAY_VERSION_NUMBER     1

    /*! String array: an array of C strings */
struct Sarray
{
    l_int32          nalloc;    /*!< size of allocated ptr array         */
    l_int32          n;         /*!< number of strings allocated         */
    l_int32          refcount;  /*!< reference count (1 if no clones)    */
    char           **array;     /*!< string array                        */
};
typedef struct Sarray SARRAY;

    /*! Byte array (analogous to C++ "string") */
struct L_Bytea
{
    size_t           nalloc;    /*!< number of bytes allocated in data array  */
    size_t           size;      /*!< number of bytes presently used           */
    l_int32          refcount;  /*!< reference count (1 if no clones)         */
    l_uint8         *data;      /*!< data array                               */
};
typedef struct L_Bytea L_BYTEA;


/*------------------------------------------------------------------------*
 *                              Array flags                               *
 *------------------------------------------------------------------------*/
/*! Numa Interpolation */
enum {
    L_LINEAR_INTERP = 1,        /*!< linear     */
    L_QUADRATIC_INTERP = 2      /*!< quadratic  */
};

/*! Border Adding */
enum {
    L_CONTINUED_BORDER = 1,    /*!< extended with same value                  */
    L_SLOPE_BORDER = 2,        /*!< extended with constant normal derivative  */
    L_MIRRORED_BORDER = 3      /*!< mirrored                                  */
};

/*! Numa Data Conversion */
enum {
    L_INTEGER_VALUE = 1,        /*!< convert to integer  */
    L_FLOAT_VALUE = 2           /*!< convert to float    */
};

#endif  /* LEPTONICA_ARRAY_H */
/*====================================================================*
 -  Copyright (C) 2001 Leptonica.  All rights reserved.
 -
 -  Redistribution and use in source and binary forms, with or without
 -  modification, are permitted provided that the following conditions
 -  are met:
 -  1. Redistributions of source code must retain the above copyright
 -     notice, this list of conditions and the following disclaimer.
 -  2. Redistributions in binary form must reproduce the above
 -     copyright notice, this list of conditions and the following
 -     disclaimer in the documentation and/or other materials
 -     provided with the distribution.
 -
 -  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 -  ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 -  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 -  A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL ANY
 -  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 -  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 -  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 -  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 -  OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 -  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 -  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *====================================================================*/

#ifndef  LEPTONICA_BBUFFER_H
#define  LEPTONICA_BBUFFER_H

/*!
 * \file bbuffer.h
 *
 * <pre>
 *      Expandable byte buffer for reading data in from memory and
 *      writing data out to other memory.
 *
 *      This implements a queue of bytes, so data read in is put
 *      on the "back" of the queue (i.e., the end of the byte array)
 *      and data written out is taken from the "front" of the queue
 *      (i.e., from an index marker "nwritten" that is initially set at
 *      the beginning of the array.)  As usual with expandable
 *      arrays, we keep the size of the allocated array and the
 *      number of bytes that have been read into the array.
 *
 *      For implementation details, see bbuffer.c.
 * </pre>
 */

/*! Expandable byte buffer for memory read/write operations */
struct L_ByteBuffer
{
    l_int32      nalloc;       /*!< size of allocated byte array            */
    l_int32      n;            /*!< number of bytes read into to the array  */
    l_int32      nwritten;     /*!< number of bytes written from the array  */
    l_uint8     *array;        /*!< byte array                              */
};
typedef struct L_ByteBuffer L_BBUFFER;


#endif  /* LEPTONICA_BBUFFER_H */
/*====================================================================*
 -  Copyright (C) 2001 Leptonica.  All rights reserved.
 -
 -  Redistribution and use in source and binary forms, with or without
 -  modification, are permitted provided that the following conditions
 -  are met:
 -  1. Redistributions of source code must retain the above copyright
 -     notice, this list of conditions and the following disclaimer.
 -  2. Redistributions in binary form must reproduce the above
 -     copyright notice, this list of conditions and the following
 -     disclaimer in the documentation and/or other materials
 -     provided with the distribution.
 -
 -  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 -  ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 -  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 -  A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL ANY
 -  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 -  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 -  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 -  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 -  OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 -  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 -  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *====================================================================*/

#ifndef  LEPTONICA_HEAP_H
#define  LEPTONICA_HEAP_H

/*!
 * \file heap.h
 *
 * <pre>
 *      Expandable priority queue configured as a heap for arbitrary void* data
 *
 *      The L_Heap is used to implement a priority queue.  The elements
 *      in the heap are ordered in either increasing or decreasing key value.
 *      The key is a float field 'keyval' that is required to be
 *      contained in the elements of the queue.
 *
 *      The heap is a simple binary tree with the following constraints:
 *         - the key of each node is >= the keys of the two children
 *         - the tree is complete, meaning that each level (1, 2, 4, ...)
 *           is filled and the last level is filled from left to right
 *
 *      The tree structure is implicit in the queue array, with the
 *      array elements numbered as a breadth-first search of the tree
 *      from left to right.  It is thus guaranteed that the largest
 *      (or smallest) key belongs to the first element in the array.
 *
 *      Heap sort is used to sort the array.  Once an array has been
 *      sorted as a heap, it is convenient to use it as a priority queue,
 *      because the min (or max) elements are always at the root of
 *      the tree (element 0), and once removed, the heap can be
 *      resorted in not more than log[n] steps, where n is the number
 *      of elements on the heap.  Likewise, if an arbitrary element is
 *      added to the end of the array A, the sorted heap can be restored
 *      in not more than log[n] steps.
 *
 *      A L_Heap differs from a L_Queue in that the elements in the former
 *      are sorted by a key.  Internally, the array is maintained
 *      as a queue, with a pointer to the end of the array.  The
 *      head of the array always remains at array[0].  The array is
 *      maintained (sorted) as a heap.  When an item is removed from
 *      the head, the last item takes its place (thus reducing the
 *      array length by 1), and this is followed by array element
 *      swaps to restore the heap property.   When an item is added,
 *      it goes at the end of the array, and is swapped up to restore
 *      the heap.  If the ptr array is full, adding another item causes
 *      the ptr array size to double.
 *
 *      For further implementation details, see heap.c.
 * </pre>
 */

/*! Heap of arbitrary void* data */
struct L_Heap
{
    l_int32      nalloc;      /*!< size of allocated ptr array               */
    l_int32      n;           /*!< number of elements stored in the heap     */
    void       **array;       /*!< ptr array                                 */
    l_int32      direction;   /*!< L_SORT_INCREASING or L_SORT_DECREASING    */
};
typedef struct L_Heap  L_HEAP;


#endif  /* LEPTONICA_HEAP_H */
/*====================================================================*
 -  Copyright (C) 2001 Leptonica.  All rights reserved.
 -
 -  Redistribution and use in source and binary forms, with or without
 -  modification, are permitted provided that the following conditions
 -  are met:
 -  1. Redistributions of source code must retain the above copyright
 -     notice, this list of conditions and the following disclaimer.
 -  2. Redistributions in binary form must reproduce the above
 -     copyright notice, this list of conditions and the following
 -     disclaimer in the documentation and/or other materials
 -     provided with the distribution.
 -
 -  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 -  ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 -  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 -  A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL ANY
 -  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 -  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 -  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 -  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 -  OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 -  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 -  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *====================================================================*/


#ifndef  LEPTONICA_LIST_H
#define  LEPTONICA_LIST_H

/*!
 * \file list.h
 *
 * <pre>
 *       Cell for double-linked lists
 *
 *       This allows composition of a list of cells with
 *           prev, next and data pointers.  Generic data
 *           structures hang on the list cell data pointers.
 *
 *       The list is not circular because that would add much
 *           complexity in traversing the list under general
 *           conditions where list cells can be added and removed.
 *           The only disadvantage of not having the head point to
 *           the last cell is that the list must be traversed to
 *           find its tail.  However, this traversal is fast, and
 *           the listRemoveFromTail() function updates the tail
 *           so there is no searching overhead with repeated use.
 *
 *       The list macros are used to run through a list, and their
 *       use is encouraged.  They are invoked, e.g., as
 *
 *             DLLIST  *head, *elem;
 *             ...
 *             L_BEGIN_LIST_FORWARD(head, elem)
 *                 <do something with elem and/or elem->data >
 *             L_END_LIST
 * </pre>
 */

struct DoubleLinkedList
{
    struct DoubleLinkedList    *prev;
    struct DoubleLinkedList    *next;
    void                       *data;
};
typedef struct DoubleLinkedList    DLLIST;


    /*!  Simple list traverse macro - forward */
#define L_BEGIN_LIST_FORWARD(head, element) \
        { \
        DLLIST   *_leptvar_nextelem_; \
        for ((element) = (head); (element); (element) = _leptvar_nextelem_) { \
            _leptvar_nextelem_ = (element)->next;


    /*!  Simple list traverse macro - reverse */
#define L_BEGIN_LIST_REVERSE(tail, element) \
        { \
        DLLIST   *_leptvar_prevelem_; \
        for ((element) = (tail); (element); (element) = _leptvar_prevelem_) { \
            _leptvar_prevelem_ = (element)->prev;


    /*!  Simple list traverse macro - end of a list traverse */
#define L_END_LIST    }}


#endif  /* LEPTONICA_LIST_H */
/*====================================================================*
 -  Copyright (C) 2001 Leptonica.  All rights reserved.
 -
 -  Redistribution and use in source and binary forms, with or without
 -  modification, are permitted provided that the following conditions
 -  are met:
 -  1. Redistributions of source code must retain the above copyright
 -     notice, this list of conditions and the following disclaimer.
 -  2. Redistributions in binary form must reproduce the above
 -     copyright notice, this list of conditions and the following
 -     disclaimer in the documentation and/or other materials
 -     provided with the distribution.
 -
 -  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 -  ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 -  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 -  A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL ANY
 -  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 -  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 -  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 -  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 -  OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 -  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 -  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *====================================================================*/

#ifndef  LEPTONICA_PTRA_H
#define  LEPTONICA_PTRA_H

/*!
 * \file ptra.h
 *
 * <pre>
 *  Contains the following structs:
 *      struct L_Ptra
 *      struct L_Ptraa
 *
 *  Contains definitions for:
 *      L_Ptra compaction flags for removal
 *      L_Ptra shifting flags for insert
 *      L_Ptraa accessor flags
 * </pre>
 */


/*------------------------------------------------------------------------*
 *                     Generic Ptr Array Structs                          *
 *------------------------------------------------------------------------*/

    /*! Generic pointer array */
struct L_Ptra
{
    l_int32          nalloc;    /*!< size of allocated ptr array         */
    l_int32          imax;      /*!< greatest valid index                */
    l_int32          nactual;   /*!< actual number of stored elements    */
    void           **array;     /*!< ptr array                           */
};
typedef struct L_Ptra  L_PTRA;


    /*! Array of generic pointer arrays */
struct L_Ptraa
{
    l_int32          nalloc;    /*!< size of allocated ptr array         */
    struct L_Ptra  **ptra;      /*!< array of ptra                       */
};
typedef struct L_Ptraa  L_PTRAA;



/*------------------------------------------------------------------------*
 *          Accessor and modifier flags for L_Ptra and L_Ptraa            *
 *------------------------------------------------------------------------*/

/*! Ptra Removal */
enum {
    L_NO_COMPACTION = 1,        /*!< null the pointer only                */
    L_COMPACTION = 2            /*!< compact the array                    */
};

/*! Ptra Insertion */
enum {
    L_AUTO_DOWNSHIFT = 0,     /*!< choose based on number of holes        */
    L_MIN_DOWNSHIFT = 1,      /*!< downshifts min # of ptrs below insert  */
    L_FULL_DOWNSHIFT = 2      /*!< downshifts all ptrs below insert       */
};

/*! Ptraa Accessor */
enum {
    L_HANDLE_ONLY = 0,     /*!< ptr to L_Ptra; caller can inspect only    */
    L_REMOVE = 1           /*!< caller owns; destroy or save in L_Ptraa   */
};


#endif  /* LEPTONICA_PTRA_H */
/*====================================================================*
 -  Copyright (C) 2001 Leptonica.  All rights reserved.
 -
 -  Redistribution and use in source and binary forms, with or without
 -  modification, are permitted provided that the following conditions
 -  are met:
 -  1. Redistributions of source code must retain the above copyright
 -     notice, this list of conditions and the following disclaimer.
 -  2. Redistributions in binary form must reproduce the above
 -     copyright notice, this list of conditions and the following
 -     disclaimer in the documentation and/or other materials
 -     provided with the distribution.
 -
 -  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 -  ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 -  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 -  A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL ANY
 -  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 -  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 -  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 -  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 -  OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 -  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 -  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *====================================================================*/

#ifndef  LEPTONICA_QUEUE_H
#define  LEPTONICA_QUEUE_H

/*!
 * \file queue.h
 *
 * <pre>
 *      Expandable pointer queue for arbitrary void* data.
 *
 *      The L_Queue is a fifo that implements a queue of void* pointers.
 *      It can be used to hold a queue of any type of struct.
 *
 *      Internally, it maintains two counters:
 *          nhead:  location of head (in ptrs) from the beginning
 *                  of the array.
 *          nelem:  number of ptr elements stored in the queue.
 *
 *      The element at the head of the queue, which is the next to
 *      be removed, is array[nhead].  The location at the tail of the
 *      queue to which the next element will be added is
 *      array[nhead + nelem].
 *
 *      As items are added to the queue, nelem increases.
 *      As items are removed, nhead increases and nelem decreases.
 *      Any time the tail reaches the end of the allocated array,
 *      all the pointers are shifted to the left, so that the head
 *      is at the beginning of the array.
 *      If the array becomes more than 3/4 full, it doubles in size.
 *
 *      The auxiliary stack can be used in a wrapper for re-using
 *      items popped from the queue.  It is not made by default.
 *
 *      For further implementation details, see queue.c.
 * </pre>
 */

/*! Expandable pointer queue for arbitrary void* data */
struct L_Queue
{
    l_int32          nalloc;   /*!< size of allocated ptr array            */
    l_int32          nhead;    /*!< location of head (in ptrs) from the    */
                               /*!< beginning of the array                 */
    l_int32          nelem;    /*!< number of elements stored in the queue */
    void           **array;    /*!< ptr array                              */
    struct L_Stack  *stack;    /*!< auxiliary stack                        */

};
typedef struct L_Queue L_QUEUE;


#endif  /* LEPTONICA_QUEUE_H */
/*====================================================================*
 -  Copyright (C) 2001 Leptonica.  All rights reserved.
 -
 -  Redistribution and use in source and binary forms, with or without
 -  modification, are permitted provided that the following conditions
 -  are met:
 -  1. Redistributions of source code must retain the above copyright
 -     notice, this list of conditions and the following disclaimer.
 -  2. Redistributions in binary form must reproduce the above
 -     copyright notice, this list of conditions and the following
 -     disclaimer in the documentation and/or other materials
 -     provided with the distribution.
 -
 -  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 -  ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 -  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 -  A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL ANY
 -  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 -  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 -  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 -  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 -  OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 -  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 -  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *====================================================================*/

/*
 * Modified from the excellent code here:
 *     http://en.literateprograms.org/Red-black_tree_(C)?oldid=19567
 * which has been placed in the public domain under the Creative Commons
 * CC0 1.0 waiver (http://creativecommons.org/publicdomain/zero/1.0/).
 *
 * When the key is generated from a hash (e.g., string --> uint64),
 * there is always the possibility of having collisions, but to make
 * the collision probability very low requires using a large hash.
 * For that reason, the key types are 64 bit quantities, which will result
 * in a negligible probabililty of collisions for millions of hashed values.
 * Using 8 byte keys instead of 4 byte keys requires a little more
 * storage, but the simplification in being able to ignore collisions
 * with the red-black trees for most applications is worth it.
 */

#ifndef  LEPTONICA_RBTREE_H
#define  LEPTONICA_RBTREE_H

    /*! The three valid key types for red-black trees, maps and sets. */
/*! RBTree Key Type */
enum {
    L_INT_TYPE = 1,
    L_UINT_TYPE = 2,
    L_FLOAT_TYPE = 3
};

    /*!
     * Storage for keys and values for red-black trees, maps and sets.
     * <pre>
     * Note:
     *   (1) Keys and values of the valid key types are all 64-bit
     *   (2) (void *) can be used for values but not for keys.
     * </pre>
     */
union Rb_Type {
    l_int64    itype;
    l_uint64   utype;
    l_float64  ftype;
    void      *ptype;
};
typedef union Rb_Type RB_TYPE;

struct L_Rbtree {
    struct L_Rbtree_Node  *root;
    l_int32                keytype;
};
typedef struct L_Rbtree L_RBTREE;
typedef struct L_Rbtree L_AMAP;  /* hide underlying implementation for map */
typedef struct L_Rbtree L_ASET;  /* hide underlying implementation for set */

struct L_Rbtree_Node {
    union Rb_Type          key;
    union Rb_Type          value;
    struct L_Rbtree_Node  *left;
    struct L_Rbtree_Node  *right;
    struct L_Rbtree_Node  *parent;
    l_int32                color;
};
typedef struct L_Rbtree_Node L_RBTREE_NODE;
typedef struct L_Rbtree_Node L_AMAP_NODE;  /* hide tree implementation */
typedef struct L_Rbtree_Node L_ASET_NODE;  /* hide tree implementation */


#endif  /* LEPTONICA_RBTREE_H */
/*====================================================================*
 -  Copyright (C) 2001 Leptonica.  All rights reserved.
 -
 -  Redistribution and use in source and binary forms, with or without
 -  modification, are permitted provided that the following conditions
 -  are met:
 -  1. Redistributions of source code must retain the above copyright
 -     notice, this list of conditions and the following disclaimer.
 -  2. Redistributions in binary form must reproduce the above
 -     copyright notice, this list of conditions and the following
 -     disclaimer in the documentation and/or other materials
 -     provided with the distribution.
 -
 -  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 -  ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 -  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 -  A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL ANY
 -  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 -  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 -  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 -  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 -  OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 -  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 -  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *====================================================================*/

#ifndef  LEPTONICA_STACK_H
#define  LEPTONICA_STACK_H

/*!
 * \file stack.h
 *
 * <pre>
 *       Expandable pointer stack for arbitrary void* data.
 *
 *       The L_Stack is an array of void * ptrs, onto which arbitrary
 *       objects can be stored.  At any time, the number of
 *       stored objects is stack->n.  The object at the bottom
 *       of the stack is at array[0]; the object at the top of
 *       the stack is at array[n-1].  New objects are added
 *       to the top of the stack, at the first available location,
 *       which is array[n].  Objects are removed from the top of the
 *       stack.  When an attempt is made to remove an object from an
 *       empty stack, the result is null.   When the stack becomes
 *       filled, so that n = nalloc, the size is doubled.
 *
 *       The auxiliary stack can be used to store and remove
 *       objects for re-use.  It must be created by a separate
 *       call to pstackCreate().  [Just imagine the chaos if
 *       pstackCreate() created the auxiliary stack!]
 *       pstackDestroy() checks for the auxiliary stack and removes it.
 * </pre>
 */


    /*! Expandable pointer stack for arbitrary void* data.
     * Note that array[n] is the first null ptr in the array
     */
struct L_Stack
{
    l_int32          nalloc;     /*!< size of ptr array              */
    l_int32          n;          /*!< number of stored elements      */
    void           **array;      /*!< ptr array                      */
    struct L_Stack  *auxstack;   /*!< auxiliary stack                */
};
typedef struct L_Stack  L_STACK;


#endif /*  LEPTONICA_STACK_H */


    /* Imaging */
/*====================================================================*
 -  Copyright (C) 2001 Leptonica.  All rights reserved.
 -
 -  Redistribution and use in source and binary forms, with or without
 -  modification, are permitted provided that the following conditions
 -  are met:
 -  1. Redistributions of source code must retain the above copyright
 -     notice, this list of conditions and the following disclaimer.
 -  2. Redistributions in binary form must reproduce the above
 -     copyright notice, this list of conditions and the following
 -     disclaimer in the documentation and/or other materials
 -     provided with the distribution.
 -
 -  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 -  ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 -  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 -  A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL ANY
 -  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 -  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 -  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 -  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 -  OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 -  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 -  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *====================================================================*/

#ifndef  LEPTONICA_ARRAY_ACCESS_H
#define  LEPTONICA_ARRAY_ACCESS_H

/*!
 * \file arrayaccess.h
 *
 * <pre>
 *  1, 2, 4, 8, 16 and 32 bit data access within an array of 32-bit words
 *
 *  This is used primarily to access 1, 2, 4, 8, 16 and 32 bit pixels
 *  in a line of image data, represented as an array of 32-bit words.
 *
 *     pdata:  pointer to first 32-bit word in the array
 *     n:      index of the pixel in the array
 *
 *  Function calls for these accessors are defined in arrayaccess.c.
 *
 *  However, for efficiency we use the inline macros for all accesses.
 *  Even though the 2 and 4 bit set* accessors are more complicated,
 *  they are about 10% faster than the function calls.
 *
 *  The 32 bit access is just a cast and ptr arithmetic.  We include
 *  it so that the input ptr can be void*.
 *
 *  At the end of this file is code for invoking the function calls
 *  instead of inlining.
 *
 *  The macro SET_DATA_BIT_VAL(pdata, n, val) is a bit slower than
 *      if (val == 0)
 *          CLEAR_DATA_BIT(pdata, n);
 *      else
 *          SET_DATA_BIT(pdata, n);
 *
 *  Some compilers complain when the SET macros are surrounded by
 *  parentheses, because parens require an evaluation and it is not
 *  defined for SET macros.  If SET_DATA_QBIT were defined as a
 *  compound macro, in analogy to l_setDataQbit(), it requires
 *  surrounding braces:
 * \code
 *     #define  SET_DATA_QBIT(pdata, n, val) \
 *        {l_uint32 *_TEMP_WORD_PTR_; \
 *         _TEMP_WORD_PTR_ = (l_uint32 *)(pdata) + ((n) >> 3); \
 *         *_TEMP_WORD_PTR_ &= ~(0xf0000000 >> (4 * ((n) & 7))); \
 *         *_TEMP_WORD_PTR_ |= (((val) & 15) << (28 - 4 * ((n) & 7)));}
 * \endcode
 *  but if used in an if/else
 * \code
 *      if (x)
 *         SET_DATA_QBIT(...);
 *      else
 *         ...
 * \endcode
 *  the compiler sees
 * \code
 *      if (x)
 *         {......};
 *      else
 *         ...
 * \endcode
 *  The semicolon comes after the brace and will not compile.
 *  This can be fixed in the call by either omitting the semicolon
 *  or requiring another set of braces around SET_DATA_QBIT(), but
 *  both these options break compatibility with current code, and
 *  require special attention by anyone using the macros.
 *
 *  There are (at least) two ways to fix this in the macro definitions,
 *  suggested by Dave Bryan.
 *  (1) Surround the braces in the macro above with
 *         do {....} while(0)
 *      Then the semicolon just terminates the expression.
 *  (2) Reduce the blocks to a single expression; e.g,
 *         *((l_uint32 *)(pdata) + ((n) >> 3)) = \
 *           *((l_uint32 *)(pdata) + ((n) >> 3)) \
 *           & ~(0xf0000000 >> (4 * ((n) & 7))) \
 *           | (((val) & 15) << (28 - 4 * ((n) & 7)))
 *      This appears to cause redundant computation, but the compiler
 *      should evaluate the common subexpression only once.
 *  All these methods have the same performance, giving about 300M
 *  SET_DATA_QBIT operations per second on a fast 64 bit system.
 *  Using the function calls instead of the macros results in about 250M
 *  SET_DATA_QBIT operations per second, a performance hit of nearly 20%.
 * </pre>
 */

#define  USE_INLINE_ACCESSORS    1

#if USE_INLINE_ACCESSORS

    /*=============================================================*/
    /*                Faster: use in line accessors                */
    /*=============================================================*/

    /*--------------------------------------------------*
     *                     1 bit access                 *
     *--------------------------------------------------*/
/*! 1 bit access - get */
#define  GET_DATA_BIT(pdata, n) \
    ((*((const l_uint32 *)(pdata) + ((n) >> 5)) >> (31 - ((n) & 31))) & 1)

/*! 1 bit access - set */
#define  SET_DATA_BIT(pdata, n) \
    *((l_uint32 *)(pdata) + ((n) >> 5)) |= (0x80000000 >> ((n) & 31))

/*! 1 bit access - clear */
#define  CLEAR_DATA_BIT(pdata, n) \
    *((l_uint32 *)(pdata) + ((n) >> 5)) &= ~(0x80000000 >> ((n) & 31))

/*! 1 bit access - set value (0 or 1) */
#define  SET_DATA_BIT_VAL(pdata, n, val) \
     *((l_uint32 *)(pdata) + ((n) >> 5)) = \
        ((*((l_uint32 *)(pdata) + ((n) >> 5)) \
        & (~(0x80000000 >> ((n) & 31)))) \
        | ((l_uint32)(val) << (31 - ((n) & 31))))

    /*--------------------------------------------------*
     *                     2 bit access                 *
     *--------------------------------------------------*/
/*! 2 bit access - get */
#define  GET_DATA_DIBIT(pdata, n) \
    ((*((const l_uint32 *)(pdata) + ((n) >> 4)) >> (2 * (15 - ((n) & 15)))) & 3)

/*! 2 bit access - set value (0 ... 3) */
#define  SET_DATA_DIBIT(pdata, n, val) \
     *((l_uint32 *)(pdata) + ((n) >> 4)) = \
        ((*((l_uint32 *)(pdata) + ((n) >> 4)) \
        & (~(0xc0000000 >> (2 * ((n) & 15))))) \
        | ((l_uint32)((val) & 3) << (30 - 2 * ((n) & 15))))

/*! 2 bit access - clear */
#define  CLEAR_DATA_DIBIT(pdata, n) \
    *((l_uint32 *)(pdata) + ((n) >> 4)) &= ~(0xc0000000 >> (2 * ((n) & 15)))


    /*--------------------------------------------------*
     *                     4 bit access                 *
     *--------------------------------------------------*/
/*! 4 bit access - get */
#define  GET_DATA_QBIT(pdata, n) \
     ((*((const l_uint32 *)(pdata) + ((n) >> 3)) >> (4 * (7 - ((n) & 7)))) & 0xf)

/*! 4 bit access - set value (0 ... 15) */
#define  SET_DATA_QBIT(pdata, n, val) \
     *((l_uint32 *)(pdata) + ((n) >> 3)) = \
        ((*((l_uint32 *)(pdata) + ((n) >> 3)) \
        & (~(0xf0000000 >> (4 * ((n) & 7))))) \
        | ((l_uint32)((val) & 15) << (28 - 4 * ((n) & 7))))

/*! 4 bit access - clear */
#define  CLEAR_DATA_QBIT(pdata, n) \
    *((l_uint32 *)(pdata) + ((n) >> 3)) &= ~(0xf0000000 >> (4 * ((n) & 7)))


    /*--------------------------------------------------*
     *                     8 bit access                 *
     *--------------------------------------------------*/
#ifdef  L_BIG_ENDIAN
/*! 8 bit access - get */
#define  GET_DATA_BYTE(pdata, n) \
             (*((const l_uint8 *)(pdata) + (n)))
#else  /* L_LITTLE_ENDIAN */
/*! 8 bit access - get */
#define  GET_DATA_BYTE(pdata, n) \
             (*(l_uint8 *)((l_uintptr_t)((const l_uint8 *)(pdata) + (n)) ^ 3))
#endif  /* L_BIG_ENDIAN */

#ifdef  L_BIG_ENDIAN
/*! 8 bit access - set value (0 ... 255) */
#define  SET_DATA_BYTE(pdata, n, val) \
             *((l_uint8 *)(pdata) + (n)) = (val)
#else  /* L_LITTLE_ENDIAN */
/*! 8 bit access - set value (0 ... 255) */
#define  SET_DATA_BYTE(pdata, n, val) \
             *(l_uint8 *)((l_uintptr_t)((l_uint8 *)(pdata) + (n)) ^ 3) = (val)
#endif  /* L_BIG_ENDIAN */


    /*--------------------------------------------------*
     *                    16 bit access                 *
     *--------------------------------------------------*/
#ifdef  L_BIG_ENDIAN
/*! 16 bit access - get */
#define  GET_DATA_TWO_BYTES(pdata, n) \
             (*((const l_uint16 *)(pdata) + (n)))
#else  /* L_LITTLE_ENDIAN */
/*! 16 bit access - get */
#define  GET_DATA_TWO_BYTES(pdata, n) \
             (*(l_uint16 *)((l_uintptr_t)((const l_uint16 *)(pdata) + (n)) ^ 2))
#endif  /* L_BIG_ENDIAN */

#ifdef  L_BIG_ENDIAN
/*! 16 bit access - set value (0 ... 65535) */
#define  SET_DATA_TWO_BYTES(pdata, n, val) \
             *((l_uint16 *)(pdata) + (n)) = (val)
#else  /* L_LITTLE_ENDIAN */
/*! 16 bit access - set value (0 ... 65535) */
#define  SET_DATA_TWO_BYTES(pdata, n, val) \
             *(l_uint16 *)((l_uintptr_t)((l_uint16 *)(pdata) + (n)) ^ 2) = (val)
#endif  /* L_BIG_ENDIAN */


    /*--------------------------------------------------*
     *                    32 bit access                 *
     *--------------------------------------------------*/
/*! 32 bit access - get */
#define  GET_DATA_FOUR_BYTES(pdata, n) \
             (*((const l_uint32 *)(pdata) + (n)))

/*! 32 bit access - set (0 ... 4294967295) */
#define  SET_DATA_FOUR_BYTES(pdata, n, val) \
             *((l_uint32 *)(pdata) + (n)) = (val)


#else

    /*=============================================================*/
    /*         Slower: use function calls for all accessors        */
    /*=============================================================*/

#define  GET_DATA_BIT(pdata, n)               l_getDataBit(pdata, n)
#define  SET_DATA_BIT(pdata, n)               l_setDataBit(pdata, n)
#define  CLEAR_DATA_BIT(pdata, n)             l_clearDataBit(pdata, n)
#define  SET_DATA_BIT_VAL(pdata, n, val)      l_setDataBitVal(pdata, n, val)

#define  GET_DATA_DIBIT(pdata, n)             l_getDataDibit(pdata, n)
#define  SET_DATA_DIBIT(pdata, n, val)        l_setDataDibit(pdata, n, val)
#define  CLEAR_DATA_DIBIT(pdata, n)           l_clearDataDibit(pdata, n)

#define  GET_DATA_QBIT(pdata, n)              l_getDataQbit(pdata, n)
#define  SET_DATA_QBIT(pdata, n, val)         l_setDataQbit(pdata, n, val)
#define  CLEAR_DATA_QBIT(pdata, n)            l_clearDataQbit(pdata, n)

#define  GET_DATA_BYTE(pdata, n)              l_getDataByte(pdata, n)
#define  SET_DATA_BYTE(pdata, n, val)         l_setDataByte(pdata, n, val)

#define  GET_DATA_TWO_BYTES(pdata, n)         l_getDataTwoBytes(pdata, n)
#define  SET_DATA_TWO_BYTES(pdata, n, val)    l_setDataTwoBytes(pdata, n, val)

#define  GET_DATA_FOUR_BYTES(pdata, n)         l_getDataFourBytes(pdata, n)
#define  SET_DATA_FOUR_BYTES(pdata, n, val)    l_setDataFourBytes(pdata, n, val)

#endif  /* USE_INLINE_ACCESSORS */


#endif /* LEPTONICA_ARRAY_ACCESS_H */
/*====================================================================*
 -  Copyright (C) 2001 Leptonica.  All rights reserved.
 -
 -  Redistribution and use in source and binary forms, with or without
 -  modification, are permitted provided that the following conditions
 -  are met:
 -  1. Redistributions of source code must retain the above copyright
 -     notice, this list of conditions and the following disclaimer.
 -  2. Redistributions in binary form must reproduce the above
 -     copyright notice, this list of conditions and the following
 -     disclaimer in the documentation and/or other materials
 -     provided with the distribution.
 -
 -  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 -  ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 -  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 -  A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL ANY
 -  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 -  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 -  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 -  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 -  OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 -  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 -  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *====================================================================*/

#ifndef  LEPTONICA_BMF_H
#define  LEPTONICA_BMF_H

/*!
 * \file bmf.h
 *
 *     Simple data structure to hold bitmap fonts and related data
 */

    /*! Constants for deciding when text block is divided into paragraphs */
/*! Split Text */
enum {
    SPLIT_ON_LEADING_WHITE = 1,    /*!< tab or space at beginning of line   */
    SPLIT_ON_BLANK_LINE    = 2,    /*!< newline with optional white space   */
    SPLIT_ON_BOTH          = 3     /*!< leading white space or newline      */
};


/*! Data structure to hold bitmap fonts and related data */
struct L_Bmf
{
    struct Pixa  *pixa;        /*!< pixa of bitmaps for 93 characters        */
    l_int32       size;        /*!< font size (in points at 300 ppi)         */
    char         *directory;   /*!< directory containing font bitmaps        */
    l_int32       baseline1;   /*!< baseline offset for ascii 33 - 57        */
    l_int32       baseline2;   /*!< baseline offset for ascii 58 - 91        */
    l_int32       baseline3;   /*!< baseline offset for ascii 93 - 126       */
    l_int32       lineheight;  /*!< max height of line of chars              */
    l_int32       kernwidth;   /*!< pixel dist between char bitmaps          */
    l_int32       spacewidth;  /*!< pixel dist between word bitmaps          */
    l_int32       vertlinesep; /*!< extra vertical space between text lines  */
    l_int32      *fonttab;     /*!< table mapping ascii --> font index       */
    l_int32      *baselinetab; /*!< table mapping ascii --> baseline offset  */
    l_int32      *widthtab;    /*!< table mapping ascii --> char width       */
};
typedef struct L_Bmf L_BMF;

#endif  /* LEPTONICA_BMF_H */
/*====================================================================*
 -  Copyright (C) 2001 Leptonica.  All rights reserved.
 -
 -  Redistribution and use in source and binary forms, with or without
 -  modification, are permitted provided that the following conditions
 -  are met:
 -  1. Redistributions of source code must retain the above copyright
 -     notice, this list of conditions and the following disclaimer.
 -  2. Redistributions in binary form must reproduce the above
 -     copyright notice, this list of conditions and the following
 -     disclaimer in the documentation and/or other materials
 -     provided with the distribution.
 -
 -  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 -  ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 -  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 -  A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL ANY
 -  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 -  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 -  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 -  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 -  OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 -  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 -  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *====================================================================*/

#ifndef  LEPTONICA_CCBORD_H
#define  LEPTONICA_CCBORD_H

/*!
 * \file ccbord.h
 *
 * <pre>
 *           CCBord:   represents a single connected component
 *           CCBorda:  an array of CCBord
 * </pre>
 */

    /*! Use in ccbaStepChainsToPixCoords() */
/*! CCB Coords */
enum {
      CCB_LOCAL_COORDS = 1,
      CCB_GLOBAL_COORDS = 2
};

    /*! Use in ccbaGenerateSPGlobalLocs() */
/*! CCB Points */
enum {
      CCB_SAVE_ALL_PTS = 1,
      CCB_SAVE_TURNING_PTS = 2
};


    /*!
     * <pre>
     * CCBord contains:
     *
     *    (1) a minimally-clipped bitmap of the component (pix),
     *    (2) a boxa consisting of:
     *          for the primary component:
     *                (xul, yul) pixel location in global coords
     *                (w, h) of the bitmap
     *          for the hole components:
     *                (x, y) in relative coordinates in primary component
     *                (w, h) of the hole border (which is 2 pixels
     *                       larger in each direction than the hole itself)
     *    (3) a pta ('start') of the initial border pixel location for each
     *        closed curve, all in relative coordinates of the primary
     *        component.  This is given for the primary component,
     *        followed by the hole components, if any.
     *    (4) a refcount of the ccbord; used internally when a ccbord
     *        is accessed from a ccborda (array of ccbord)
     *    (5) a ptaa for the chain code for the border in relative
     *        coordinates, where the first pta is the exterior border
     *        and all other pta are for interior borders (holes)
     *    (6) a ptaa for the global pixel loc rendition of the border,
     *        where the first pta is the exterior border and all other
     *        pta are for interior borders (holes).
     *        This is derived from the local or step chain code.
     *    (7) a numaa for the chain code for the border as orientation
     *        directions between successive border pixels, where
     *        the first numa is the exterior border and all other
     *        numa are for interior borders (holes).  This is derived
     *        from the local chain code.  The 8 directions are 0 - 7.
     *    (8) a pta for a single chain for each c.c., comprised of outer
     *        and hole borders, plus cut paths between them, all in
     *        local coords.
     *    (9) a pta for a single chain for each c.c., comprised of outer
     *        and hole borders, plus cut paths between them, all in
     *        global coords.
     * </pre>
     */
struct CCBord
{
    struct Pix          *pix;          /*!< component bitmap (min size)      */
    struct Boxa         *boxa;         /*!< regions of each closed curve     */
    struct Pta          *start;        /*!< initial border pixel locations   */
    l_int32              refcount;     /*!< number of handles; start at 1    */
    struct Ptaa         *local;        /*!< ptaa of chain pixels (local)     */
    struct Ptaa         *global;       /*!< ptaa of chain pixels (global)    */
    struct Numaa        *step;         /*!< numaa of chain code (step dir)   */
    struct Pta          *splocal;      /*!< pta of single chain (local)      */
    struct Pta          *spglobal;     /*!< pta of single chain (global)     */
};
typedef struct CCBord CCBORD;

/*! Array of CCBord */
struct CCBorda
{
    struct Pix          *pix;          /*!< input pix (may be null)          */
    l_int32              w;            /*!< width of pix                     */
    l_int32              h;            /*!< height of pix                    */
    l_int32              n;            /*!< number of ccbord in ptr array    */
    l_int32              nalloc;       /*!< number of ccbord ptrs allocated  */
    struct CCBord      **ccb;          /*!< ccb ptr array                    */
};
typedef struct CCBorda CCBORDA;


#endif  /* LEPTONICA_CCBORD_H */

/*====================================================================*
 -  Copyright (C) 2001 Leptonica.  All rights reserved.
 -
 -  Redistribution and use in source and binary forms, with or without
 -  modification, are permitted provided that the following conditions
 -  are met:
 -  1. Redistributions of source code must retain the above copyright
 -     notice, this list of conditions and the following disclaimer.
 -  2. Redistributions in binary form must reproduce the above
 -     copyright notice, this list of conditions and the following
 -     disclaimer in the documentation and/or other materials
 -     provided with the distribution.
 -
 -  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 -  ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 -  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 -  A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL ANY
 -  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 -  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 -  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 -  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 -  OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 -  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 -  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *====================================================================*/

#ifndef  LEPTONICA_DEWARP_H
#define  LEPTONICA_DEWARP_H

/*!
 * \file dewarp.h
 *
 * <pre>
 *     Data structure to hold arrays and results for generating
 *     horizontal and vertical disparity arrays based on textlines.
 *     Each disparity array is two-dimensional.  The vertical disparity
 *     array gives a vertical displacement, relative to the lowest point
 *     in the textlines.  The horizontal disparty array gives a horizontal
 *     displacement, relative to the minimum values (for even pages)
 *     or maximum values (for odd pages) of the left and right ends of
 *     full textlines.  Horizontal alignment always involves translations
 *     away from the book gutter.
 *
 *     We have intentionally separated the process of building models
 *     from the rendering process that uses the models.  For any page,
 *     the building operation either creates an actual model (that is,
 *     a model with at least the vertical disparity being computed, and
 *     for which the 'success' flag is set) or fails to create a model.
 *     However, at rendering time, a page can have one of two different
 *     types of models.
 *     (1) A valid model is an actual model that meets the rendering
 *         constraints, which are limits on model curvature parameters.
 *         See dewarpaTestForValidModel() for details.
 *         Valid models are identified by dewarpaInsertRefModels(),
 *         which sets the 'vvalid' and 'hvalid' fields.  Only valid
 *         models are used for rendering.
 *     (2) A reference model is used by a page that doesn't have
 *         a valid model, but has a nearby valid model of the same
 *         parity (even/odd page) that it can use.  The range in pages
 *         to search for a valid model is given by the 'maxdist' field.
 *
 *     At the rendering stage, vertical and horizontal disparities are
 *     treated differently.  It is somewhat more robust to generate
 *     vertical disparity models (VDM) than horizontal disparity
 *     models (HDM). A valid VDM is required for any correction to
 *     be made; if a valid VDM is not available, just use the input
 *     image.  Otherwise, assuming it is available, the use of the
 *     HDM is controlled by two fields: 'useboth' and 'check_columns'.
 *       (a) With useboth == 0, we use only the VDM.
 *       (b) With useboth == 1, we require using the VDM and, if a valid
 *           horizontal disparity model (HDM) is available, we also use it.
 *       (c) With check_columns == 1, check for multiple columns and if
 *           true, only use the VDM, even if a valid HDM is available.
 *           Note that 'check_columns' takes precedence over 'useboth'
 *           when there is more than 1 column of text.  By default,
 *           check_columns == 0.
 *
 *     The 'maxdist' parameter is input when the dewarpa is created.
 *     The other rendering parameters have default values given in dewarp1.c.
 *     All parameters used by rendering can be set (or reset) using accessors.
 *
 *     After dewarping, use of the VDM will cause all points on each
 *     altered curve to have a y-value equal to the minimum.  Use of
 *     the HDA will cause the left and right edges of the textlines
 *     to be vertically aligned if they had been typeset flush-left
 *     and flush-right, respectively.
 *
 *     The sampled disparity arrays are expanded to full resolution,
 *     using linear interpolation, and this is further expanded
 *     by slope continuation to the right and below if the image
 *     is larger than the full resolution disparity arrays.  Then
 *     the disparity correction can be applied to the input image.
 *     If the input pix are 2x reduced, the expansion from sampled
 *     to full res uses the product of (sampling) * (redfactor).
 *
 *     The most accurate results are produced at full resolution, and
 *     this is generally recommended.
 * </pre>
 */

    /*! Dewarp version for serialization
     * <pre>
     * Note on versioning of the serialization of this data structure:
     * The dewarping utility and the stored data can be expected to change.
     * In most situations, the serialized version is ephemeral -- it is
     * not needed after being used.  No functions will be provided to
     * convert between different versions.
     * </pre>
     */
#define  DEWARP_VERSION_NUMBER      4

/*! Data structure to hold a number of Dewarp */
struct L_Dewarpa
{
    l_int32            nalloc;        /*!< size of dewarp ptr array          */
    l_int32            maxpage;       /*!< maximum page number in array      */
    struct L_Dewarp  **dewarp;        /*!< array of ptrs to page dewarp      */
    struct L_Dewarp  **dewarpcache;   /*!< array of ptrs to cached dewarps   */
    struct Numa       *namodels;      /*!< list of page numbers for pages    */
                                      /*!< with page models                  */
    struct Numa       *napages;       /*!< list of page numbers with either  */
                                      /*!< page models or ref page models    */
    l_int32            redfactor;     /*!< reduction factor of input: 1 or 2 */
    l_int32            sampling;      /*!< disparity arrays sampling factor  */
    l_int32            minlines;      /*!< min number of long lines required */
    l_int32            maxdist;       /*!< max distance for getting ref page */
    l_int32            max_linecurv;  /*!< maximum abs line curvature,       */
                                      /*!< in micro-units                    */
    l_int32            min_diff_linecurv; /*!< minimum abs diff line         */
                                          /*!< curvature in micro-units      */
    l_int32            max_diff_linecurv; /*!< maximum abs diff line         */
                                          /*!< curvature in micro-units      */
    l_int32            max_edgeslope; /*!< maximum abs left or right edge    */
                                      /*!< slope, in milli-units             */
    l_int32            max_edgecurv;  /*!< maximum abs left or right edge    */
                                      /*!< curvature, in micro-units         */
    l_int32            max_diff_edgecurv; /*!< maximum abs diff left-right   */
                                      /*!< edge curvature, in micro-units    */
    l_int32            useboth;       /*!< use both disparity arrays if      */
                                      /*!< available; only vertical otherwise */
    l_int32            check_columns; /*!< if there are multiple columns,    */
                                      /*!< only use the vertical disparity   */
                                      /*!< array                             */
    l_int32            modelsready;   /*!< invalid models have been removed  */
                                      /*!< and refs built against valid set  */
};
typedef struct L_Dewarpa L_DEWARPA;


/*! Data structure for a single dewarp */
struct L_Dewarp
{
    struct L_Dewarpa  *dewa;         /*!< ptr to parent (not owned)          */
    struct Pix        *pixs;         /*!< source pix, 1 bpp                  */
    struct FPix       *sampvdispar;  /*!< sampled vert disparity array       */
    struct FPix       *samphdispar;  /*!< sampled horiz disparity array      */
    struct FPix       *sampydispar;  /*!< sampled slope h-disparity array    */
    struct FPix       *fullvdispar;  /*!< full vert disparity array          */
    struct FPix       *fullhdispar;  /*!< full horiz disparity array         */
    struct FPix       *fullydispar;  /*!< full slope h-disparity array       */
    struct Numa       *namidys;      /*!< sorted y val of midpoint each line */
    struct Numa       *nacurves;     /*!< sorted curvature of each line      */
    l_int32            w;            /*!< width of source image              */
    l_int32            h;            /*!< height of source image             */
    l_int32            pageno;       /*!< page number; important for reuse   */
    l_int32            sampling;     /*!< sampling factor of disparity arrays */
    l_int32            redfactor;    /*!< reduction factor of pixs: 1 or 2   */
    l_int32            minlines;     /*!< min number of long lines required  */
    l_int32            nlines;       /*!< number of long lines found         */
    l_int32            mincurv;      /*!< min line curvature in micro-units  */
    l_int32            maxcurv;      /*!< max line curvature in micro-units  */
    l_int32            leftslope;    /*!< left edge slope in milli-units     */
    l_int32            rightslope;   /*!< right edge slope in milli-units    */
    l_int32            leftcurv;     /*!< left edge curvature in micro-units */
    l_int32            rightcurv;    /*!< right edge curvature in micro-units*/
    l_int32            nx;           /*!< number of sampling pts in x-dir    */
    l_int32            ny;           /*!< number of sampling pts in y-dir    */
    l_int32            hasref;       /*!< 0 if normal; 1 if has a refpage    */
    l_int32            refpage;      /*!< page with disparity model to use   */
    l_int32            vsuccess;     /*!< sets to 1 if vert disparity builds */
    l_int32            hsuccess;     /*!< sets to 1 if horiz disparity builds */
    l_int32            ysuccess;     /*!< sets to 1 if slope disparity builds */
    l_int32            vvalid;       /*!< sets to 1 if valid vert disparity  */
    l_int32            hvalid;       /*!< sets to 1 if valid horiz disparity */
    l_int32            skip_horiz;   /*!< if 1, skip horiz disparity         */
                                     /*!< correction                         */
    l_int32            debug;        /*!< set to 1 if debug output requested */
};
typedef struct L_Dewarp L_DEWARP;

#endif  /* LEPTONICA_DEWARP_H */
/*====================================================================*
 -  Copyright (C) 2001 Leptonica.  All rights reserved.
 -
 -  Redistribution and use in source and binary forms, with or without
 -  modification, are permitted provided that the following conditions
 -  are met:
 -  1. Redistributions of source code must retain the above copyright
 -     notice, this list of conditions and the following disclaimer.
 -  2. Redistributions in binary form must reproduce the above
 -     copyright notice, this list of conditions and the following
 -     disclaimer in the documentation and/or other materials
 -     provided with the distribution.
 -
 -  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 -  ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 -  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 -  A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL ANY
 -  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 -  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 -  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 -  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 -  OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 -  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 -  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *====================================================================*/

#ifndef  LEPTONICA_GPLOT_H
#define  LEPTONICA_GPLOT_H

/*!
 * \file gplot.h
 *
 * <pre>
 *   Data structures and parameters for generating gnuplot files
 *
 *   We used to support X11 output, but recent versions of gnuplot do not
 *   support the X11 terminal.  To get display to your screen, use
 *   GPLOT_PNG output; e.g.,
 *       gplotSimple1(na, GPLOT_PNG, "/tmp/someroot", ...);
 *       l_fileDisplay("/tmp/someroot.png", ...);
 * </pre>
 */

#define  GPLOT_VERSION_NUMBER    1

#define  NUM_GPLOT_STYLES  5
enum GPLOT_STYLE {
    GPLOT_LINES       = 0,
    GPLOT_POINTS      = 1,
    GPLOT_IMPULSES    = 2,
    GPLOT_LINESPOINTS = 3,
    GPLOT_DOTS        = 4
};

#define  NUM_GPLOT_OUTPUTS  6
enum GPLOT_OUTPUT {
    GPLOT_NONE  = 0,
    GPLOT_PNG   = 1,
    GPLOT_PS    = 2,
    GPLOT_EPS   = 3,
    GPLOT_LATEX = 4,
    GPLOT_PNM   = 5,
};

enum GPLOT_SCALING {
    GPLOT_LINEAR_SCALE  = 0,   /*!< default */
    GPLOT_LOG_SCALE_X   = 1,
    GPLOT_LOG_SCALE_Y   = 2,
    GPLOT_LOG_SCALE_X_Y = 3
};

extern const char  *gplotstylenames[];  /*!< used in gnuplot cmd file */
extern const char  *gplotfileoutputs[]; /*!< used in simple file input */

/*! Data structure for generating gnuplot files */
struct GPlot
{
    char          *rootname;   /*!< for cmd, data, output            */
    char          *cmdname;    /*!< command file name                */
    struct Sarray *cmddata;    /*!< command file contents            */
    struct Sarray *datanames;  /*!< data file names                  */
    struct Sarray *plotdata;   /*!< plot data (1 string/file)        */
    struct Sarray *plotlabels; /*!< label for each individual plot   */
    struct Numa   *plotstyles; /*!< plot style for individual plots  */
    l_int32        nplots;     /*!< current number of plots          */
    char          *outname;    /*!< output file name                 */
    l_int32        outformat;  /*!< GPLOT_OUTPUT values              */
    l_int32        scaling;    /*!< GPLOT_SCALING values             */
    char          *title;      /*!< optional                         */
    char          *xlabel;     /*!< optional x axis label            */
    char          *ylabel;     /*!< optional y axis label            */
};
typedef struct GPlot  GPLOT;


#endif /* LEPTONICA_GPLOT_H */
/*====================================================================*
 -  Copyright (C) 2001 Leptonica.  All rights reserved.
 -
 -  Redistribution and use in source and binary forms, with or without
 -  modification, are permitted provided that the following conditions
 -  are met:
 -  1. Redistributions of source code must retain the above copyright
 -     notice, this list of conditions and the following disclaimer.
 -  2. Redistributions in binary form must reproduce the above
 -     copyright notice, this list of conditions and the following
 -     disclaimer in the documentation and/or other materials
 -     provided with the distribution.
 -
 -  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 -  ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 -  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 -  A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL ANY
 -  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 -  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 -  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 -  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 -  OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 -  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 -  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *====================================================================*/

/*!
 * \file imageio.h
 *
 * <pre>
 *  General features of image I/O in leptonica
 *
 *  At present, there are 9 file formats for images that can be read
 *  and written:
 *      png (requires libpng, libz)
 *      jpeg (requires libjpeg)
 *      tiff (requires libtiff, libz)
 *      gif (requires libgif)
 *      webp (requires libwebp)
 *      jp2 (requires libopenjp2)
 *      bmp (no library required)
 *      pnm (no library required)
 *      spix (no library required)
 *  Additionally, there are two file formats for writing (only) images:
 *      PostScript (requires libpng, libz, libjpeg, libtiff)
 *      pdf (requires libpng, libz, libjpeg, libtiff)
 *
 *  For all 9 read/write formats, leptonica provides interconversion
 *  between pix (with raster data) and formatted image data:
 *      Conversion from pix (typically compression):
 *          pixWrite():        pix --> file
 *          pixWriteStream():  pix --> filestream (aka FILE*)
 *          pixWriteMem():     pix --> memory buffer
 *      Conversion to pix (typically decompression):
 *          pixRead():         file --> pix
 *          pixReadStream():   filestream --> pix
 *          pixReadMem():      memory buffer --> pix
 *
 *  Conversions for which the image data is not compressed are:
 *     * uncompressed tiff   (IFF_TIFF)
 *     * bmp
 *     * pnm
 *     * spix (fast serialization that copies the pix raster data)
 *
 *  The image header (metadata) information can be read from either
 *  the compressed file or a memory buffer, for all 9 formats.
 * </pre>
 */

#ifndef  LEPTONICA_IMAGEIO_H
#define  LEPTONICA_IMAGEIO_H

/* --------------------------------------------------------------- *
 *                    Image file format types                      *
 * --------------------------------------------------------------- */
/*
 *  The IFF_DEFAULT flag is used to write the file out in the
 *  same (input) file format that the pix was read from.  If the pix
 *  was not read from file, the input format field will be
 *  IFF_UNKNOWN and the output file format will be chosen to
 *  be compressed and lossless; namely, IFF_TIFF_G4 for d = 1
 *  and IFF_PNG for everything else.
 *
 *  In the future, new format types that have defined extensions
 *  will be added before IFF_DEFAULT, and will be kept in sync with
 *  the file format extensions in writefile.c.  The positions of
 *  file formats before IFF_DEFAULT will remain invariant.
 */

/*! Image Formats */
enum {
    IFF_UNKNOWN        = 0,
    IFF_BMP            = 1,
    IFF_JFIF_JPEG      = 2,
    IFF_PNG            = 3,
    IFF_TIFF           = 4,
    IFF_TIFF_PACKBITS  = 5,
    IFF_TIFF_RLE       = 6,
    IFF_TIFF_G3        = 7,
    IFF_TIFF_G4        = 8,
    IFF_TIFF_LZW       = 9,
    IFF_TIFF_ZIP       = 10,
    IFF_PNM            = 11,
    IFF_PS             = 12,
    IFF_GIF            = 13,
    IFF_JP2            = 14,
    IFF_WEBP           = 15,
    IFF_LPDF           = 16,
    IFF_TIFF_JPEG      = 17,
    IFF_DEFAULT        = 18,
    IFF_SPIX           = 19
};

/* Convenient macro for checking requested tiff output */
#define  L_FORMAT_IS_TIFF(f)  ((f) == IFF_TIFF || (f) == IFF_TIFF_PACKBITS || \
                               (f) == IFF_TIFF_RLE || (f) == IFF_TIFF_G3 || \
                               (f) == IFF_TIFF_G4 || (f) == IFF_TIFF_LZW || \
                               (f) == IFF_TIFF_ZIP || (f) == IFF_TIFF_JPEG)


/* --------------------------------------------------------------- *
 *                         Format header ids                       *
 * --------------------------------------------------------------- */

/*! Header Ids */
enum {
    BMP_ID             = 0x4d42,     /*!< BM - for bitmaps    */
    TIFF_BIGEND_ID     = 0x4d4d,     /*!< MM - for 'motorola' */
    TIFF_LITTLEEND_ID  = 0x4949      /*!< II - for 'intel'    */
};


/* --------------------------------------------------------------- *
 *                Hinting bit flags in jpeg reader                 *
 * --------------------------------------------------------------- */

/*! Jpeg Hints */
enum {
    L_JPEG_READ_LUMINANCE = 1,   /*!< only want luminance data; no chroma */
    L_JPEG_FAIL_ON_BAD_DATA = 2  /*!< don't return possibly damaged pix */
};


/* --------------------------------------------------------------- *
 *                    Pdf formatted encoding types                 *
 * --------------------------------------------------------------- */

/*! Pdf Encoding */
enum {
    L_DEFAULT_ENCODE  = 0,  /*!< use default encoding based on image        */
    L_JPEG_ENCODE     = 1,  /*!< use dct encoding: 8 and 32 bpp, no cmap    */
    L_G4_ENCODE       = 2,  /*!< use ccitt g4 fax encoding: 1 bpp           */
    L_FLATE_ENCODE    = 3,  /*!< use flate encoding: any depth, cmap ok     */
    L_JP2K_ENCODE     = 4   /*!< use jp2k encoding: 8 and 32 bpp, no cmap   */
};


/* --------------------------------------------------------------- *
 *                    Compressed image data                        *
 * --------------------------------------------------------------- */
/*
 *  In use, either datacomp or data85 will be produced, depending
 *  on whether the data needs to be ascii85 encoded.  PostScript
 *  requires ascii85 encoding; pdf does not.
 *
 *  For the colormap (flate compression only), PostScript uses ascii85
 *  encoding and pdf uses a bracketed array of space-separated
 *  hex-encoded rgb triples.  Only tiff g4 (type == L_G4_ENCODE) uses
 *  the minisblack field.
 */

/*! Compressed image data */
struct L_Compressed_Data
{
    l_int32            type;         /*!< encoding type: L_JPEG_ENCODE, etc   */
    l_uint8           *datacomp;     /*!< gzipped raster data                 */
    size_t             nbytescomp;   /*!< number of compressed bytes          */
    char              *data85;       /*!< ascii85-encoded gzipped raster data */
    size_t             nbytes85;     /*!< number of ascii85 encoded bytes     */
    char              *cmapdata85;   /*!< ascii85-encoded uncompressed cmap   */
    char              *cmapdatahex;  /*!< hex pdf array for the cmap          */
    l_int32            ncolors;      /*!< number of colors in cmap            */
    l_int32            w;            /*!< image width                         */
    l_int32            h;            /*!< image height                        */
    l_int32            bps;          /*!< bits/sample; typ. 1, 2, 4 or 8      */
    l_int32            spp;          /*!< samples/pixel; typ. 1 or 3          */
    l_int32            minisblack;   /*!< tiff g4 photometry                  */
    l_int32            predictor;    /*!< flate data has PNG predictors       */
    size_t             nbytes;       /*!< number of uncompressed raster bytes */
    l_int32            res;          /*!< resolution (ppi)                    */
};
typedef struct L_Compressed_Data  L_COMP_DATA;


/* ------------------------------------------------------------------------- *
 *                           Pdf multi image flags                           *
 * ------------------------------------------------------------------------- */

/*! Pdf MultiImage */
enum {
    L_FIRST_IMAGE   = 1,    /*!< first image to be used                      */
    L_NEXT_IMAGE    = 2,    /*!< intermediate image; not first or last       */
    L_LAST_IMAGE    = 3     /*!< last image to be used                       */
};


/* ------------------------------------------------------------------------- *
 *                     Intermediate pdf generation data                      *
 * ------------------------------------------------------------------------- */
/*
 *  This accumulates data for generating a pdf of a single page consisting
 *  of an arbitrary number of images.
 *
 *  None of the strings have a trailing newline.
 */

/*! Intermediate pdf generation data */
struct L_Pdf_Data
{
    char              *title;        /*!< optional title for pdf              */
    l_int32            n;            /*!< number of images                    */
    l_int32            ncmap;        /*!< number of colormaps                 */
    struct L_Ptra     *cida;         /*!< array of compressed image data      */
    char              *id;           /*!< %PDF-1.2 id string                  */
    char              *obj1;         /*!< catalog string                      */
    char              *obj2;         /*!< metadata string                     */
    char              *obj3;         /*!< pages string                        */
    char              *obj4;         /*!< page string (variable data)         */
    char              *obj5;         /*!< content string (variable data)      */
    char              *poststream;   /*!< post-binary-stream string           */
    char              *trailer;      /*!< trailer string (variable data)      */
    struct Pta        *xy;           /*!< store (xpt, ypt) array              */
    struct Pta        *wh;           /*!< store (wpt, hpt) array              */
    struct Box        *mediabox;     /*!< bounding region for all images      */
    struct Sarray     *saprex;       /*!< pre-binary-stream xobject strings   */
    struct Sarray     *sacmap;       /*!< colormap pdf object strings         */
    struct L_Dna      *objsize;      /*!< sizes of each pdf string object     */
    struct L_Dna      *objloc;       /*!< location of each pdf string object  */
    l_int32            xrefloc;      /*!< location of xref                    */
};
typedef struct L_Pdf_Data  L_PDF_DATA;


#endif  /* LEPTONICA_IMAGEIO_H */
/*====================================================================*
 -  Copyright (C) 2001 Leptonica.  All rights reserved.
 -
 -  Redistribution and use in source and binary forms, with or without
 -  modification, are permitted provided that the following conditions
 -  are met:
 -  1. Redistributions of source code must retain the above copyright
 -     notice, this list of conditions and the following disclaimer.
 -  2. Redistributions in binary form must reproduce the above
 -     copyright notice, this list of conditions and the following
 -     disclaimer in the documentation and/or other materials
 -     provided with the distribution.
 -
 -  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 -  ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 -  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 -  A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL ANY
 -  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 -  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 -  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 -  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 -  OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 -  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 -  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *====================================================================*/

#ifndef  LEPTONICA_JBCLASS_H
#define  LEPTONICA_JBCLASS_H

/*!
 * \file jbclass.h
 *
 *       JbClasser
 *       JbData
 */


    /*!
     * <pre>
     * The JbClasser struct holds all the data accumulated during the
     * classification process that can be used for a compressed
     * jbig2-type representation of a set of images.  This is created
     * in an initialization process and added to as the selected components
     * on each successive page are analyzed.
     * </pre>
     */
struct JbClasser
{
    struct Sarray   *safiles;      /*!< input page image file names          */
    l_int32          method;       /*!< JB_RANKHAUS, JB_CORRELATION          */
    l_int32          components;   /*!< JB_CONN_COMPS, JB_CHARACTERS or      */
                                   /*!< JB_WORDS                             */
    l_int32          maxwidth;     /*!< max component width allowed          */
    l_int32          maxheight;    /*!< max component height allowed         */
    l_int32          npages;       /*!< number of pages already processed    */
    l_int32          baseindex;    /*!< number components already processed  */
                                   /*!< on fully processed pages             */
    struct Numa     *nacomps;      /*!< number of components on each page    */
    l_int32          sizehaus;     /*!< size of square struct elem for haus  */
    l_float32        rankhaus;     /*!< rank val of haus match, each way     */
    l_float32        thresh;       /*!< thresh value for correlation score   */
    l_float32        weightfactor; /*!< corrects thresh value for heaver     */
                                   /*!< components; use 0 for no correction  */
    struct Numa     *naarea;       /*!< w * h of each template, without      */
                                   /*!< extra border pixels                  */
    l_int32          w;            /*!< max width of original src images     */
    l_int32          h;            /*!< max height of original src images    */
    l_int32          nclass;       /*!< current number of classes            */
    l_int32          keep_pixaa;   /*!< If zero, pixaa isn't filled          */
    struct Pixaa    *pixaa;        /*!< instances for each class; unbordered */
    struct Pixa     *pixat;        /*!< templates for each class; bordered   */
                                   /*!< and not dilated                      */
    struct Pixa     *pixatd;       /*!< templates for each class; bordered   */
                                   /*!< and dilated                          */
    struct L_DnaHash *dahash;      /*!< Hash table to find templates by size */
    struct Numa     *nafgt;        /*!< fg areas of undilated templates;     */
                                   /*!< only used for rank < 1.0             */
    struct Pta      *ptac;         /*!< centroids of all bordered cc         */
    struct Pta      *ptact;        /*!< centroids of all bordered template cc */
    struct Numa     *naclass;      /*!< array of class ids for each component */
    struct Numa     *napage;       /*!< array of page nums for each component */
    struct Pta      *ptaul;        /*!< array of UL corners at which the     */
                                   /*!< template is to be placed for each    */
                                   /*!< component                            */
    struct Pta      *ptall;        /*!< similar to ptaul, but for LL corners */
};
typedef struct JbClasser  JBCLASSER;


    /*!
     * <pre>
     * The JbData struct holds all the data required for
     * the compressed jbig-type representation of a set of images.
     * The data can be written to file, read back, and used
     * to regenerate an approximate version of the original,
     * which differs in two ways from the original:
     *   (1) It uses a template image for each c.c. instead of the
     *       original instance, for each occurrence on each page.
     *   (2) It discards components with either a height or width larger
     *       than the maximuma, given here by the lattice dimensions
     *       used for storing the templates.
     * </pre>
     */
struct JbData
{
    struct Pix      *pix;        /*!< template composite for all classes    */
    l_int32          npages;     /*!< number of pages                       */
    l_int32          w;          /*!< max width of original page images     */
    l_int32          h;          /*!< max height of original page images    */
    l_int32          nclass;     /*!< number of classes                     */
    l_int32          latticew;   /*!< lattice width for template composite  */
    l_int32          latticeh;   /*!< lattice height for template composite */
    struct Numa     *naclass;    /*!< array of class ids for each component */
    struct Numa     *napage;     /*!< array of page nums for each component */
    struct Pta      *ptaul;      /*!< array of UL corners at which the      */
                                 /*!< template is to be placed for each     */
                                 /*!< component                             */
};
typedef struct JbData  JBDATA;


/*! JB Classifier */
enum {
    JB_RANKHAUS = 0,
    JB_CORRELATION = 1
};

    /*! For jbGetComponents(): type of component to extract from images */
/*! JB Component */
enum {
    JB_CONN_COMPS = 0,
    JB_CHARACTERS = 1,
    JB_WORDS = 2
};

    /*! These parameters are used for naming the two files
     * in which the jbig2-like compressed data is stored.  */
#define   JB_TEMPLATE_EXT      ".templates.png"
#define   JB_DATA_EXT          ".data"


#endif  /* LEPTONICA_JBCLASS_H */
/*====================================================================*
 -  Copyright (C) 2001 Leptonica.  All rights reserved.
 -
 -  Redistribution and use in source and binary forms, with or without
 -  modification, are permitted provided that the following conditions
 -  are met:
 -  1. Redistributions of source code must retain the above copyright
 -     notice, this list of conditions and the following disclaimer.
 -  2. Redistributions in binary form must reproduce the above
 -     copyright notice, this list of conditions and the following
 -     disclaimer in the documentation and/or other materials
 -     provided with the distribution.
 -
 -  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 -  ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 -  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 -  A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL ANY
 -  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 -  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 -  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 -  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 -  OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 -  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 -  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *====================================================================*/

#ifndef  LEPTONICA_MORPH_H
#define  LEPTONICA_MORPH_H

/*!
 * \file morph.h
 *
 * <pre>
 *  Contains the following structs:
 *      struct Sel
 *      struct Sela
 *      struct Kernel
 *
 *  Contains definitions for:
 *      morphological b.c. flags
 *      structuring element types
 *      runlength flags for granulometry
 *      direction flags for grayscale morphology
 *      morphological operation flags
 *      standard border size
 *      grayscale intensity scaling flags
 *      morphological tophat flags
 *      arithmetic and logical operator flags
 *      grayscale morphology selection flags
 *      distance function b.c. flags
 *      image comparison flags
 *      color content flags
 * </pre>
 */

/*-------------------------------------------------------------------------*
 *                             Sel and Sel array                           *
 *-------------------------------------------------------------------------*/
#define  SEL_VERSION_NUMBER    1

/*! Selection */
struct Sel
{
    l_int32       sy;        /*!< sel height                               */
    l_int32       sx;        /*!< sel width                                */
    l_int32       cy;        /*!< y location of sel origin                 */
    l_int32       cx;        /*!< x location of sel origin                 */
    l_int32     **data;      /*!< {0,1,2}; data[i][j] in [row][col] order  */
    char         *name;      /*!< used to find sel by name                 */
};
typedef struct Sel SEL;

/*! Array of Sel */
struct Sela
{
    l_int32          n;       /*!< number of sel actually stored           */
    l_int32          nalloc;  /*!< size of allocated ptr array             */
    struct Sel     **sel;     /*!< sel ptr array                           */
};
typedef struct Sela SELA;


/*-------------------------------------------------------------------------*
 *                                 Kernel                                  *
 *-------------------------------------------------------------------------*/
#define  KERNEL_VERSION_NUMBER    2

/*! Kernel */
struct L_Kernel
{
    l_int32       sy;        /*!< kernel height                            */
    l_int32       sx;        /*!< kernel width                             */
    l_int32       cy;        /*!< y location of kernel origin              */
    l_int32       cx;        /*!< x location of kernel origin              */
    l_float32   **data;      /*!< data[i][j] in [row][col] order           */
};
typedef struct L_Kernel  L_KERNEL;


/*-------------------------------------------------------------------------*
 *                 Morphological boundary condition flags                  *
 *                                                                         *
 *  Two types of boundary condition for erosion.                           *
 *  The global variable MORPH_BC takes on one of these two values.         *
 *  See notes in morph.c for usage.                                        *
 *-------------------------------------------------------------------------*/

/*! Morph Boundary */
enum {
    SYMMETRIC_MORPH_BC = 0,
    ASYMMETRIC_MORPH_BC = 1
};

/*-------------------------------------------------------------------------*
 *                        Structuring element vals                         *
 *-------------------------------------------------------------------------*/

/*! SEL Vals */
enum {
    SEL_DONT_CARE  = 0,
    SEL_HIT        = 1,
    SEL_MISS       = 2
};

/*-------------------------------------------------------------------------*
 *                  Runlength flags for granulometry                       *
 *-------------------------------------------------------------------------*/

/*! Runlength Polarity */
enum {
    L_RUN_OFF = 0,
    L_RUN_ON  = 1
};

/*-------------------------------------------------------------------------*
 *         Direction flags for grayscale morphology, granulometry,         *
 *                 composable Sels, convolution, etc.                      *
 *-------------------------------------------------------------------------*/

/*! Direction Flags */
enum {
    L_HORIZ            = 1,
    L_VERT             = 2,
    L_BOTH_DIRECTIONS  = 3
};

/*-------------------------------------------------------------------------*
 *                   Morphological operation flags                         *
 *-------------------------------------------------------------------------*/

/*! Morph Operator */
enum {
    L_MORPH_DILATE    = 1,
    L_MORPH_ERODE     = 2,
    L_MORPH_OPEN      = 3,
    L_MORPH_CLOSE     = 4,
    L_MORPH_HMT       = 5
};

/*-------------------------------------------------------------------------*
 *                    Grayscale intensity scaling flags                    *
 *-------------------------------------------------------------------------*/

/*! Pixel Value Scaling */
enum {
    L_LINEAR_SCALE  = 1,
    L_LOG_SCALE     = 2
};

/*-------------------------------------------------------------------------*
 *                      Morphological tophat flags                         *
 *-------------------------------------------------------------------------*/

/*! Morph Tophat */
enum {
    L_TOPHAT_WHITE = 0,
    L_TOPHAT_BLACK = 1
};

/*-------------------------------------------------------------------------*
 *                Arithmetic and logical operator flags                    *
 *                 (use on grayscale images and Numas)                     *
 *-------------------------------------------------------------------------*/

/*! ArithLogical Ops */
enum {
    L_ARITH_ADD       = 1,
    L_ARITH_SUBTRACT  = 2,
    L_ARITH_MULTIPLY  = 3,   /* on numas only */
    L_ARITH_DIVIDE    = 4,   /* on numas only */
    L_UNION           = 5,   /* on numas only */
    L_INTERSECTION    = 6,   /* on numas only */
    L_SUBTRACTION     = 7,   /* on numas only */
    L_EXCLUSIVE_OR    = 8    /* on numas only */
};

/*-------------------------------------------------------------------------*
 *                        Min/max selection flags                          *
 *-------------------------------------------------------------------------*/

/*! MinMax Selection */
enum {
    L_CHOOSE_MIN = 1,         /* useful in a downscaling "erosion"       */
    L_CHOOSE_MAX = 2,         /* useful in a downscaling "dilation"      */
    L_CHOOSE_MAXDIFF = 3,     /* useful in a downscaling contrast        */
    L_CHOOSE_MIN_BOOST = 4,   /* use a modification of the min value     */
    L_CHOOSE_MAX_BOOST = 5    /* use a modification of the max value     */
};

/*-------------------------------------------------------------------------*
 *            Exterior value b.c. for distance function flags              *
 *-------------------------------------------------------------------------*/

/*! Exterior Value */
enum {
    L_BOUNDARY_BG = 1,  /* assume bg outside image */
    L_BOUNDARY_FG = 2   /* assume fg outside image */
};

/*-------------------------------------------------------------------------*
 *                         Image comparison flags                          *
 *-------------------------------------------------------------------------*/

/*! Image Comparison */
enum {
    L_COMPARE_XOR = 1,
    L_COMPARE_SUBTRACT = 2,
    L_COMPARE_ABS_DIFF = 3
};

/*-------------------------------------------------------------------------*
 *                          Color content flags                            *
 *-------------------------------------------------------------------------*/

/*! Color Content */
enum {
    L_MAX_DIFF_FROM_AVERAGE_2 = 1,
    L_MAX_MIN_DIFF_FROM_2 = 2,
    L_MAX_DIFF = 3
};

/*-------------------------------------------------------------------------*
 *    Standard size of border added around images for special processing   *
 *-------------------------------------------------------------------------*/
static const l_int32  ADDED_BORDER = 32;   /*!< pixels, not bits */


#endif  /* LEPTONICA_MORPH_H */
/*====================================================================*
 -  Copyright (C) 2001 Leptonica.  All rights reserved.
 -
 -  Redistribution and use in source and binary forms, with or without
 -  modification, are permitted provided that the following conditions
 -  are met:
 -  1. Redistributions of source code must retain the above copyright
 -     notice, this list of conditions and the following disclaimer.
 -  2. Redistributions in binary form must reproduce the above
 -     copyright notice, this list of conditions and the following
 -     disclaimer in the documentation and/or other materials
 -     provided with the distribution.
 -
 -  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 -  ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 -  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 -  A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL ANY
 -  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 -  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 -  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 -  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 -  OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 -  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 -  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *====================================================================*/

#ifndef  LEPTONICA_PIX_H
#define  LEPTONICA_PIX_H

/*!
 * \file pix.h
 *
 * <pre>
 *   Valid image types in leptonica:
 *       Pix: 1 bpp, with and without colormap
 *       Pix: 2 bpp, with and without colormap
 *       Pix: 4 bpp, with and without colormap
 *       Pix: 8 bpp, with and without colormap
 *       Pix: 16 bpp (1 spp)
 *       Pix: 32 bpp (rgb, 3 spp)
 *       Pix: 32 bpp (rgba, 4 spp)
 *       FPix: 32 bpp float
 *       DPix: 64 bpp double
 *       Notes:
 *          (1) The only valid Pix image type with alpha is rgba.
 *              In particular, the alpha component is not used in
 *              cmapped images.
 *          (2) PixComp can hold any Pix with IFF_PNG encoding.
 *
 *   Contents:
 *
 *   (1) This file defines most of the image-related structs used in leptonica:
 *         struct Pix
 *         struct PixColormap
 *         struct RGBA_Quad
 *         struct Pixa
 *         struct Pixaa
 *         struct Box
 *         struct Boxa
 *         struct Boxaa
 *         struct Pta
 *         struct Ptaa
 *         struct Pixacc
 *         struct PixTiling
 *         struct FPix
 *         struct FPixa
 *         struct DPix
 *         struct PixComp
 *         struct PixaComp
 *
 *   (2) This file has definitions for:
 *         Colors for RGBA
 *         Colors for drawing boxes
 *         Perceptual color weights
 *         Colormap conversion flags
 *         Rasterop bit flags
 *         Structure access flags (for insert, copy, clone, copy-clone)
 *         Sorting flags (by type and direction)
 *         Blending flags
 *         Graphics pixel setting flags
 *         Size and location filter flags
 *         Color component selection flags
 *         16-bit conversion flags
 *         Rotation and shear flags
 *         Affine transform order flags
 *         Grayscale filling flags
 *         Flags for setting to white or black
 *         Flags for getting white or black pixel value
 *         Flags for 8 and 16 bit pixel sums
 *         Dithering flags
 *         Distance flags
 *         Value flags
 *         Statistical measures
 *         Set selection flags
 *         Text orientation flags
 *         Edge orientation flags
 *         Line orientation flags
 *         Image orientation flags
 *         Scan direction flags
 *         Box size adjustment flags
 *         Flags for modifying box boundaries using a second box
 *         Handling overlapping bounding boxes in boxa
 *         Selecting or making a box from two (intersecting) boxes
 *         Flags for replacing invalid boxes
 *         Flags for box corners and center
 *         Horizontal warp
 *         Pixel selection for resampling
 *         Thinning flags
 *         Runlength flags
 *         Edge filter flags
 *         Subpixel color component ordering in LCD display
 *         HSV histogram flags
 *         Region flags (inclusion, exclusion)
 *         Flags for adding text to a pix
 *         Flags for plotting on a pix
 *         Flags for making simple masks
 *         Flags for selecting display program
 *         Flags in the 'special' pix field for non-default operations
 *         Handling negative values in conversion to unsigned int
 *         Relative to zero flags
 *         Flags for adding or removing trailing slash from string
 *
 *   (3) This file has typedefs for the pix allocator and deallocator functions
 *         alloc_fn()
 *         dealloc_fn().
 * </pre>
 */


/*-------------------------------------------------------------------------*
 *                              Basic Pix                                  *
 *-------------------------------------------------------------------------*/
    /* The 'special' field is by default 0, but it can hold integers
     * that direct non-default actions, e.g., in png and jpeg I/O. */

/*! Basic Pix */
struct Pix
{
    l_uint32             w;         /*!< width in pixels                   */
    l_uint32             h;         /*!< height in pixels                  */
    l_uint32             d;         /*!< depth in bits (bpp)               */
    l_uint32             spp;       /*!< number of samples per pixel       */
    l_uint32             wpl;       /*!< 32-bit words/line                 */
    l_uint32             refcount;  /*!< reference count (1 if no clones)  */
    l_int32              xres;      /*!< image res (ppi) in x direction    */
                                    /*!< (use 0 if unknown)                */
    l_int32              yres;      /*!< image res (ppi) in y direction    */
                                    /*!< (use 0 if unknown)                */
    l_int32              informat;  /*!< input file format, IFF_*          */
    l_int32              special;   /*!< special instructions for I/O, etc */
    char                *text;      /*!< text string associated with pix   */
    struct PixColormap  *colormap;  /*!< colormap (may be null)            */
    l_uint32            *data;      /*!< the image data                    */
};
typedef struct Pix PIX;

/*! Colormap of a Pix */
struct PixColormap
{
    void            *array;   /*!< colormap table (array of RGBA_QUAD)     */
    l_int32          depth;   /*!< of pix (1, 2, 4 or 8 bpp)               */
    l_int32          nalloc;  /*!< number of color entries allocated       */
    l_int32          n;       /*!< number of color entries used            */
};
typedef struct PixColormap  PIXCMAP;


    /*! Colormap table entry (after the BMP version).
     * Note that the BMP format stores the colormap table exactly
     * as it appears here, with color samples being stored sequentially,
     * in the order (b,g,r,a). */
struct RGBA_Quad
{
    l_uint8     blue;         /*!< blue value */
    l_uint8     green;        /*!< green value */
    l_uint8     red;          /*!< red value */
    l_uint8     alpha;        /*!< alpha value */
};
typedef struct RGBA_Quad  RGBA_QUAD;


/*-------------------------------------------------------------------------*
 *                           Colors for 32 RGBA                            *
 *-------------------------------------------------------------------------*/
/* <pre>
 *  Notes:
 *      (1) These are the byte indices for colors in 32 bpp images.
 *          They are used through the GET/SET_DATA_BYTE accessors.
 *          The 4th byte, typically known as the "alpha channel" and used
 *          for blending, is used to a small extent in leptonica.
 *      (2) Do not change these values!  If you redefine them, functions
 *          that have the shifts hardcoded for efficiency and conciseness
 *          (instead of using the constants below) will break.  These
 *          functions are labelled with "***"  next to their names at
 *          the top of the files in which they are defined.
 *      (3) The shifts to extract the red, green, blue and alpha components
 *          from a 32 bit pixel are defined here.
 * </pre>
 */

/*! RGBA Color */
enum {
    COLOR_RED = 0,        /*!< red color index in RGBA_QUAD    */
    COLOR_GREEN = 1,      /*!< green color index in RGBA_QUAD  */
    COLOR_BLUE = 2,       /*!< blue color index in RGBA_QUAD   */
    L_ALPHA_CHANNEL = 3   /*!< alpha value index in RGBA_QUAD  */
};

static const l_int32  L_RED_SHIFT =
       8 * (sizeof(l_uint32) - 1 - COLOR_RED);           /* 24 */
static const l_int32  L_GREEN_SHIFT =
       8 * (sizeof(l_uint32) - 1 - COLOR_GREEN);         /* 16 */
static const l_int32  L_BLUE_SHIFT =
       8 * (sizeof(l_uint32) - 1 - COLOR_BLUE);          /*  8 */
static const l_int32  L_ALPHA_SHIFT =
       8 * (sizeof(l_uint32) - 1 - L_ALPHA_CHANNEL);     /*  0 */


/*-------------------------------------------------------------------------*
 *                       Colors for drawing boxes                          *
 *-------------------------------------------------------------------------*/
/*! Box Color */
enum {
    L_DRAW_RED = 0,         /*!< draw in red                   */
    L_DRAW_GREEN = 1,       /*!< draw in green                 */
    L_DRAW_BLUE = 2,        /*!< draw in blue                  */
    L_DRAW_SPECIFIED = 3,   /*!< draw specified color          */
    L_DRAW_RGB = 4,         /*!< draw as sequence of r,g,b     */
    L_DRAW_RANDOM = 5       /*!< draw randomly chosen colors   */
};


/*-------------------------------------------------------------------------*
 *                       Perceptual color weights                          *
 *-------------------------------------------------------------------------*/
/* <pre>
 *  Notes:
 *      (1) These perceptual weighting factors are ad-hoc, but they do
 *          add up to 1.  Unlike, for example, the weighting factors for
 *          converting RGB to luminance, or more specifically to Y in the
 *          YUV colorspace.  Those numbers come from the
 *          International Telecommunications Union, via ITU-R.
 * </pre>
 */
static const l_float32 L_RED_WEIGHT =   0.3f; /*!< Percept. weight for red   */
static const l_float32 L_GREEN_WEIGHT = 0.5f; /*!< Percept. weight for green */
static const l_float32 L_BLUE_WEIGHT =  0.2f; /*!< Percept. weight for blue  */


/*-------------------------------------------------------------------------*
 *                        Flags for colormap conversion                    *
 *-------------------------------------------------------------------------*/
/*! Cmap Conversion */
enum {
    REMOVE_CMAP_TO_BINARY = 0,     /*!< remove colormap for conv to 1 bpp  */
    REMOVE_CMAP_TO_GRAYSCALE = 1,  /*!< remove colormap for conv to 8 bpp  */
    REMOVE_CMAP_TO_FULL_COLOR = 2, /*!< remove colormap for conv to 32 bpp */
    REMOVE_CMAP_WITH_ALPHA = 3,    /*!< remove colormap and alpha          */
    REMOVE_CMAP_BASED_ON_SRC = 4   /*!< remove depending on src format     */
};


/*------------------------------------------------------------------------*
 *!
 * <pre>
 * The following operation bit flags have been modified from
 * Sun's pixrect.h.
 *
 * The 'op' in 'rasterop' is represented by an integer
 * composed with Boolean functions using the set of five integers
 * given below.  The integers, and the op codes resulting from
 * boolean expressions on them, need only be in the range from 0 to 15.
 * The function is applied on a per-pixel basis.
 *
 * Examples: the op code representing ORing the src and dest
 * is computed using the bit OR, as PIX_SRC | PIX_DST;  the op
 * code representing XORing src and dest is found from
 * PIX_SRC ^ PIX_DST;  the op code representing ANDing src and dest
 * is found from PIX_SRC & PIX_DST.  Note that
 * PIX_NOT(PIX_CLR) = PIX_SET, and v.v., as they must be.
 *
 * We use the following set of definitions:
 *
 *      #define   PIX_SRC      0xc
 *      #define   PIX_DST      0xa
 *      #define   PIX_NOT(op)  (op) ^ 0xf
 *      #define   PIX_CLR      0x0
 *      #define   PIX_SET      0xf
 *
 * These definitions differ from Sun's, in that Sun left-shifted
 * each value by 1 pixel, and used the least significant bit as a
 * flag for the "pseudo-operation" of clipping.  We don't need
 * this bit, because it is both efficient and safe ALWAYS to clip
 * the rectangles to the src and dest images, which is what we do.
 * See the notes in rop.h on the general choice of these bit flags.
 *
 * [If for some reason you need compatibility with Sun's xview package,
 * you can adopt the original Sun definitions to avoid redefinition conflicts:
 *
 *      #define   PIX_SRC      (0xc << 1)
 *      #define   PIX_DST      (0xa << 1)
 *      #define   PIX_NOT(op)  ((op) ^ 0x1e)
 *      #define   PIX_CLR      (0x0 << 1)
 *      #define   PIX_SET      (0xf << 1)
 * ]
 *
 * We have, for reference, the following 16 unique op flags:
 *
 *      PIX_CLR                           0000             0x0
 *      PIX_SET                           1111             0xf
 *      PIX_SRC                           1100             0xc
 *      PIX_DST                           1010             0xa
 *      PIX_NOT(PIX_SRC)                  0011             0x3
 *      PIX_NOT(PIX_DST)                  0101             0x5
 *      PIX_SRC | PIX_DST                 1110             0xe
 *      PIX_SRC & PIX_DST                 1000             0x8
 *      PIX_SRC ^ PIX_DST                 0110             0x6
 *      PIX_NOT(PIX_SRC) | PIX_DST        1011             0xb
 *      PIX_NOT(PIX_SRC) & PIX_DST        0010             0x2
 *      PIX_SRC | PIX_NOT(PIX_DST)        1101             0xd
 *      PIX_SRC & PIX_NOT(PIX_DST)        0100             0x4
 *      PIX_NOT(PIX_SRC | PIX_DST)        0001             0x1
 *      PIX_NOT(PIX_SRC & PIX_DST)        0111             0x7
 *      PIX_NOT(PIX_SRC ^ PIX_DST)        1001             0x9
 *
 * </pre>
 *-------------------------------------------------------------------------*/

#define   PIX_SRC      (0xc)                      /*!< use source pixels      */
#define   PIX_DST      (0xa)                      /*!< use destination pixels */
#define   PIX_NOT(op)  ((op) ^ 0x0f)              /*!< invert operation %op   */
#define   PIX_CLR      (0x0)                      /*!< clear pixels           */
#define   PIX_SET      (0xf)                      /*!< set pixels             */

#define   PIX_PAINT    (PIX_SRC | PIX_DST)        /*!< paint = src | dst      */
#define   PIX_MASK     (PIX_SRC & PIX_DST)        /*!< mask = src & dst       */
#define   PIX_SUBTRACT (PIX_DST & PIX_NOT(PIX_SRC)) /*!< subtract =           */
                                                    /*!<    src & !dst        */
#define   PIX_XOR      (PIX_SRC ^ PIX_DST)        /*!< xor = src ^ dst        */


/*-------------------------------------------------------------------------*
 * <pre>
 *   Important Notes:
 *
 *       (1) The image data is stored in a single contiguous
 *           array of l_uint32, into which the pixels are packed.
 *           By "packed" we mean that there are no unused bits
 *           between pixels, except for end-of-line padding to
 *           satisfy item (2) below.
 *
 *       (2) Every image raster line begins on a 32-bit word
 *           boundary within this array.
 *
 *       (3) Pix image data is stored in 32-bit units, with the
 *           pixels ordered from left to right in the image being
 *           stored in order from the MSB to LSB within the word,
 *           for both big-endian and little-endian machines.
 *           This is the natural ordering for big-endian machines,
 *           as successive bytes are stored and fetched progressively
 *           to the right.  However, for little-endians, when storing
 *           we re-order the bytes from this byte stream order, and
 *           reshuffle again for byte access on 32-bit entities.
 *           So if the bytes come in sequence from left to right, we
 *           store them on little-endians in byte order:
 *                3 2 1 0 7 6 5 4 ...
 *           This MSB to LSB ordering allows left and right shift
 *           operations on 32 bit words to move the pixels properly.
 *
 *       (4) We use 32 bit pixels for both RGB and RGBA color images.
 *           The A (alpha) byte is ignored in most leptonica functions
 *           operating on color images.  Within each 4 byte pixel, the
 *           color samples are ordered from MSB to LSB, as follows:
 *
 *                |  MSB  |  2nd MSB  |  3rd MSB  |  LSB  |
 *                   red      green       blue      alpha
 *                    0         1           2         3   (big-endian)
 *                    3         2           1         0   (little-endian)
 *
 *           Because we use MSB to LSB ordering within the 32-bit word,
 *           the individual 8-bit samples can be accessed with
 *           GET_DATA_BYTE and SET_DATA_BYTE macros, using the
 *           (implicitly big-ending) ordering
 *                 red:    byte 0  (MSB)
 *                 green:  byte 1  (2nd MSB)
 *                 blue:   byte 2  (3rd MSB)
 *                 alpha:  byte 3  (LSB)
 *
 *           The specific color assignment is made in this file,
 *           through the definitions of COLOR_RED, etc.  Then the R, G
 *           B and A sample values can be retrieved using
 *                 redval = GET_DATA_BYTE(&pixel, COLOR_RED);
 *                 greenval = GET_DATA_BYTE(&pixel, COLOR_GREEN);
 *                 blueval = GET_DATA_BYTE(&pixel, COLOR_BLUE);
 *                 alphaval = GET_DATA_BYTE(&pixel, L_ALPHA_CHANNEL);
 *           and they can be set with
 *                 SET_DATA_BYTE(&pixel, COLOR_RED, redval);
 *                 SET_DATA_BYTE(&pixel, COLOR_GREEN, greenval);
 *                 SET_DATA_BYTE(&pixel, COLOR_BLUE, blueval);
 *                 SET_DATA_BYTE(&pixel, L_ALPHA_CHANNEL, alphaval);
 *
 *           More efficiently, these components can be extracted directly
 *           by shifting and masking, explicitly using the values in
 *           L_RED_SHIFT, etc.:
 *                 (pixel32 >> L_RED_SHIFT) & 0xff;         (red)
 *                 (pixel32 >> L_GREEN_SHIFT) & 0xff;       (green)
 *                 (pixel32 >> L_BLUE_SHIFT) & 0xff;        (blue)
 *                 (pixel32 >> L_ALPHA_SHIFT) & 0xff;       (alpha)
 *           The functions extractRGBValues() and extractRGBAValues() are
 *           provided to do this.  Likewise, the pixels can be set
 *           directly by shifting, using composeRGBPixel() and
 *           composeRGBAPixel().
 *
 *           All these operations work properly on both big- and little-endians.
 *
 *       (5) A reference count is held within each pix, giving the
 *           number of ptrs to the pix.  When a pixClone() call
 *           is made, the ref count is increased by 1, and
 *           when a pixDestroy() call is made, the reference count
 *           of the pix is decremented.  The pix is only destroyed
 *           when the reference count goes to zero.
 *
 *       (6) The version numbers (below) are used in the serialization
 *           of these data structures.  They are placed in the files,
 *           and rarely (if ever) change.  Provision is currently made for
 *           backward compatibility in reading from boxaa version 2.
 *
 *       (7) The serialization dependencies are as follows:
 *               pixaa  :  pixa  :  boxa
 *               boxaa  :  boxa
 *           So, for example, pixaa and boxaa can be changed without
 *           forcing a change in pixa or boxa.  However, if pixa is
 *           changed, it forces a change in pixaa, and if boxa is
 *           changed, if forces a change in the other three.
 *           We define four version numbers:
 *               PIXAA_VERSION_NUMBER
 *               PIXA_VERSION_NUMBER
 *               BOXAA_VERSION_NUMBER
 *               BOXA_VERSION_NUMBER
 * </pre>
 *-------------------------------------------------------------------------*/



/*-------------------------------------------------------------------------*
 *                              Array of pix                               *
 *-------------------------------------------------------------------------*/

    /*  Serialization for primary data structures */
#define  PIXAA_VERSION_NUMBER      2  /*!< Version for Pixaa serialization */
#define  PIXA_VERSION_NUMBER       2  /*!< Version for Pixa serialization  */
#define  BOXA_VERSION_NUMBER       2  /*!< Version for Boxa serialization  */
#define  BOXAA_VERSION_NUMBER      3  /*!< Version for Boxaa serialization */

/*! Array of pix */
struct Pixa
{
    l_int32             n;          /*!< number of Pix in ptr array        */
    l_int32             nalloc;     /*!< number of Pix ptrs allocated      */
    l_uint32            refcount;   /*!< reference count (1 if no clones)  */
    struct Pix        **pix;        /*!< the array of ptrs to pix          */
    struct Boxa        *boxa;       /*!< array of boxes                    */
};
typedef struct Pixa PIXA;

/*! Array of arrays of pix */
struct Pixaa
{
    l_int32             n;          /*!< number of Pixa in ptr array       */
    l_int32             nalloc;     /*!< number of Pixa ptrs allocated     */
    struct Pixa       **pixa;       /*!< array of ptrs to pixa             */
    struct Boxa        *boxa;       /*!< array of boxes                    */
};
typedef struct Pixaa PIXAA;


/*-------------------------------------------------------------------------*
 *                    Basic rectangle and rectangle arrays                 *
 *-------------------------------------------------------------------------*/

/*! Basic rectangle */
struct Box
{
    l_int32            x;           /*!< left coordinate                   */
    l_int32            y;           /*!< top coordinate                    */
    l_int32            w;           /*!< box width                         */
    l_int32            h;           /*!< box height                        */
    l_uint32           refcount;    /*!< reference count (1 if no clones)  */
};
typedef struct Box    BOX;

/*! Array of Box */
struct Boxa
{
    l_int32            n;           /*!< number of box in ptr array        */
    l_int32            nalloc;      /*!< number of box ptrs allocated      */
    l_uint32           refcount;    /*!< reference count (1 if no clones)  */
    struct Box       **box;         /*!< box ptr array                     */
};
typedef struct Boxa  BOXA;

/*! Array of Boxa */
struct Boxaa
{
    l_int32            n;           /*!< number of boxa in ptr array       */
    l_int32            nalloc;      /*!< number of boxa ptrs allocated     */
    struct Boxa      **boxa;        /*!< boxa ptr array                    */
};
typedef struct Boxaa  BOXAA;


/*-------------------------------------------------------------------------*
 *                               Array of points                           *
 *-------------------------------------------------------------------------*/
#define  PTA_VERSION_NUMBER      1  /*!< Version for Pta serialization     */

/*! Array of points */
struct Pta
{
    l_int32            n;           /*!< actual number of pts              */
    l_int32            nalloc;      /*!< size of allocated arrays          */
    l_uint32           refcount;    /*!< reference count (1 if no clones)  */
    l_float32         *x, *y;       /*!< arrays of floats                  */
};
typedef struct Pta PTA;


/*-------------------------------------------------------------------------*
 *                              Array of Pta                               *
 *-------------------------------------------------------------------------*/

/*! Array of Pta */
struct Ptaa
{
    l_int32              n;         /*!< number of pta in ptr array        */
    l_int32              nalloc;    /*!< number of pta ptrs allocated      */
    struct Pta         **pta;       /*!< pta ptr array                     */
};
typedef struct Ptaa PTAA;


/*-------------------------------------------------------------------------*
 *                       Pix accumulator container                         *
 *-------------------------------------------------------------------------*/

/*! Pix accumulator container */
struct Pixacc
{
    l_int32             w;          /*!< array width                       */
    l_int32             h;          /*!< array height                      */
    l_int32             offset;     /*!< used to allow negative            */
                                    /*!< intermediate results              */
    struct Pix         *pix;        /*!< the 32 bit accumulator pix        */
};
typedef struct Pixacc PIXACC;


/*-------------------------------------------------------------------------*
 *                              Pix tiling                                 *
 *-------------------------------------------------------------------------*/

/*! Pix tiling */
struct PixTiling
{
    struct Pix          *pix;       /*!< input pix (a clone)               */
    l_int32              nx;        /*!< number of tiles horizontally      */
    l_int32              ny;        /*!< number of tiles vertically        */
    l_int32              w;         /*!< tile width                        */
    l_int32              h;         /*!< tile height                       */
    l_int32              xoverlap;  /*!< overlap on left and right         */
    l_int32              yoverlap;  /*!< overlap on top and bottom         */
    l_int32              strip;     /*!< strip for paint; default is TRUE  */
};
typedef struct PixTiling PIXTILING;


/*-------------------------------------------------------------------------*
 *                       FPix: pix with float array                        *
 *-------------------------------------------------------------------------*/
#define  FPIX_VERSION_NUMBER      2 /*!< Version for FPix serialization    */

/*! Pix with float array */
struct FPix
{
    l_int32              w;         /*!< width in pixels                   */
    l_int32              h;         /*!< height in pixels                  */
    l_int32              wpl;       /*!< 32-bit words/line                 */
    l_uint32             refcount;  /*!< reference count (1 if no clones)  */
    l_int32              xres;      /*!< image res (ppi) in x direction    */
                                    /*!< (use 0 if unknown)                */
    l_int32              yres;      /*!< image res (ppi) in y direction    */
                                    /*!< (use 0 if unknown)                */
    l_float32           *data;      /*!< the float image data              */
};
typedef struct FPix FPIX;

/*! Array of FPix */
struct FPixa
{
    l_int32             n;          /*!< number of fpix in ptr array       */
    l_int32             nalloc;     /*!< number of fpix ptrs allocated     */
    l_uint32            refcount;   /*!< reference count (1 if no clones)  */
    struct FPix       **fpix;       /*!< the array of ptrs to fpix         */
};
typedef struct FPixa FPIXA;


/*-------------------------------------------------------------------------*
 *                       DPix: pix with double array                       *
 *-------------------------------------------------------------------------*/
#define  DPIX_VERSION_NUMBER      2 /*!< Version for DPix serialization    */

/*! Pix with double array */
struct DPix
{
    l_int32              w;         /*!< width in pixels                   */
    l_int32              h;         /*!< height in pixels                  */
    l_int32              wpl;       /*!< 32-bit words/line                 */
    l_uint32             refcount;  /*!< reference count (1 if no clones)  */
    l_int32              xres;      /*!< image res (ppi) in x direction    */
                                    /*!< (use 0 if unknown)                */
    l_int32              yres;      /*!< image res (ppi) in y direction    */
                                    /*!< (use 0 if unknown)                */
    l_float64           *data;      /*!< the double image data             */
};
typedef struct DPix DPIX;


/*-------------------------------------------------------------------------*
 *                        PixComp: compressed pix                          *
 *-------------------------------------------------------------------------*/

/*! Compressed Pix */
struct PixComp
{
    l_int32              w;         /*!< width in pixels                   */
    l_int32              h;         /*!< height in pixels                  */
    l_int32              d;         /*!< depth in bits                     */
    l_int32              xres;      /*!< image res (ppi) in x direction    */
                                    /*!<   (use 0 if unknown)              */
    l_int32              yres;      /*!< image res (ppi) in y direction    */
                                    /*!<   (use 0 if unknown)              */
    l_int32              comptype;  /*!< compressed format (IFF_TIFF_G4,   */
                                    /*!<   IFF_PNG, IFF_JFIF_JPEG)         */
    char                *text;      /*!< text string associated with pix   */
    l_int32              cmapflag;  /*!< flag (1 for cmap, 0 otherwise)    */
    l_uint8             *data;      /*!< the compressed image data         */
    size_t               size;      /*!< size of the data array            */
};
typedef struct PixComp PIXC;


/*-------------------------------------------------------------------------*
 *                     PixaComp: array of compressed pix                   *
 *-------------------------------------------------------------------------*/
#define  PIXACOMP_VERSION_NUMBER 2  /*!< Version for PixaComp serialization */

/*! Array of compressed pix */
struct PixaComp
{
    l_int32              n;         /*!< number of PixComp in ptr array    */
    l_int32              nalloc;    /*!< number of PixComp ptrs allocated  */
    l_int32              offset;    /*!< indexing offset into ptr array    */
    struct PixComp     **pixc;      /*!< the array of ptrs to PixComp      */
    struct Boxa         *boxa;      /*!< array of boxes                    */
};
typedef struct PixaComp PIXAC;


/*-------------------------------------------------------------------------*
 *                         Access and storage flags                        *
 *-------------------------------------------------------------------------*/
/*
 * <pre>
 *  For Pix, Box, Pta and Numa, there are 3 standard methods for handling
 *  the retrieval or insertion of a struct:
 *     (1) direct insertion (Don't do this if there is another handle
 *                           somewhere to this same struct!)
 *     (2) copy (Always safe, sets up a refcount of 1 on the new object.
 *               Can be undesirable if very large, such as an image or
 *               an array of images.)
 *     (3) clone (Makes another handle to the same struct, and bumps the
 *                refcount up by 1.  OK to use except in two situations:
 *                (a) You change data through one of the handles but don't
 *                    want those changes to be seen by the other handle.
 *                (b) The application is multi-threaded.  Because the clone
 *                    operation is not atomic (e.g., locked with a mutex),
 *                    it is possible to end up with an incorrect ref count,
 *                    causing either a memory leak or a crash.
 *
 *  For Pixa and Boxa, which are structs that hold an array of clonable
 *  structs, there is an additional method:
 *     (4) copy-clone (Makes a new higher-level struct with a refcount
 *                     of 1, but clones all the structs in the array.)
 *
 *  Unlike the other structs, when retrieving a string from an Sarray,
 *  you are allowed to get a handle without a copy or clone (i.e., the
 *  string is not owned by the handle).  You must not either free the string
 *  or insert it in some other struct that would own it.  Specifically,
 *  for an Sarray, the copyflag for retrieval is either:
 *         L_COPY or L_NOCOPY
 *  and for insertion, the copyflag is either:
 *         L_COPY or one of {L_INSERT , L_NOCOPY} (the latter are equivalent
 *                                                 for insertion))
 *  Typical patterns are:
 *  (1) Reference a string in an Sarray with L_NOCOPY and insert a copy
 *      of it in another Sarray with L_COPY.
 *  (2) Copy a string from an Sarray with L_COPY and insert it in
 *      another Sarray with L_INSERT (or L_NOCOPY).
 *  In both cases, a copy is made and both Sarrays own their instance
 *  of that string.
 * </pre>
 */

/*! Object Access */
enum {
    L_NOCOPY = 0,     /*!< do not copy the object; do not delete the ptr  */
    L_INSERT = L_NOCOPY,    /*!< stuff it in; do not copy or clone        */
    L_COPY = 1,       /*!< make/use a copy of the object                  */
    L_CLONE = 2,      /*!< make/use clone (ref count) of the object       */
    L_COPY_CLONE = 3  /*!< make a new array object (e.g., pixa) and fill  */
                      /*!< the array with clones (e.g., pix)              */
};


/*----------------------------------------------------------------------------*
 *                              Sort flags                                    *
 *----------------------------------------------------------------------------*/
/*! Sort Mode */
enum {
    L_SHELL_SORT = 1,            /*!< use shell sort                        */
    L_BIN_SORT = 2               /*!< use bin sort                          */
};

/*! Sort Order */
enum {
    L_SORT_INCREASING = 1,       /*!< sort in increasing order              */
    L_SORT_DECREASING = 2        /*!< sort in decreasing order              */
};

/*! Sort Type */
enum {
    L_SORT_BY_X = 1,             /*!< sort box or c.c. by left edge location  */
    L_SORT_BY_Y = 2,             /*!< sort box or c.c. by top edge location   */
    L_SORT_BY_RIGHT = 3,         /*!< sort box or c.c. by right edge location */
    L_SORT_BY_BOT = 4,           /*!< sort box or c.c. by bot edge location   */
    L_SORT_BY_WIDTH = 5,         /*!< sort box or c.c. by width               */
    L_SORT_BY_HEIGHT = 6,        /*!< sort box or c.c. by height              */
    L_SORT_BY_MIN_DIMENSION = 7, /*!< sort box or c.c. by min dimension       */
    L_SORT_BY_MAX_DIMENSION = 8, /*!< sort box or c.c. by max dimension       */
    L_SORT_BY_PERIMETER = 9,     /*!< sort box or c.c. by perimeter           */
    L_SORT_BY_AREA = 10,         /*!< sort box or c.c. by area                */
    L_SORT_BY_ASPECT_RATIO = 11  /*!< sort box or c.c. by width/height ratio  */
};


/*---------------------------------------------------------------------------*
 *                             Blend flags                                   *
 *---------------------------------------------------------------------------*/
/*! Blend Types */
enum {
    L_BLEND_WITH_INVERSE = 1,     /*!< add some of src inverse to itself     */
    L_BLEND_TO_WHITE = 2,         /*!< shift src colors towards white        */
    L_BLEND_TO_BLACK = 3,         /*!< shift src colors towards black        */
    L_BLEND_GRAY = 4,             /*!< blend src directly with blender       */
    L_BLEND_GRAY_WITH_INVERSE = 5 /*!< add amount of src inverse to itself,  */
                                  /*!< based on blender pix value            */
};

/*! Paint Selection */
enum {
    L_PAINT_LIGHT = 1,            /*!< colorize non-black pixels             */
    L_PAINT_DARK = 2              /*!< colorize non-white pixels             */
};


/*-------------------------------------------------------------------------*
 *                        Graphics pixel setting                           *
 *-------------------------------------------------------------------------*/
/*! Pixel Setting */
enum {
    L_SET_PIXELS = 1,           /*!< set all bits in each pixel to 1       */
    L_CLEAR_PIXELS = 2,         /*!< set all bits in each pixel to 0       */
    L_FLIP_PIXELS = 3           /*!< flip all bits in each pixel           */
};


/*-------------------------------------------------------------------------*
 *                     Size and location filter flags                      *
 *-------------------------------------------------------------------------*/
/*! Size Comparison */
enum {
    L_SELECT_IF_LT = 1,         /*!< save if value is less than threshold  */
    L_SELECT_IF_GT = 2,         /*!< save if value is more than threshold  */
    L_SELECT_IF_LTE = 3,        /*!< save if value is <= to the threshold  */
    L_SELECT_IF_GTE = 4         /*!< save if value is >= to the threshold  */
};

/*! Size Selection */
enum {
    L_SELECT_BY_WIDTH = 1,          /*!< select by width; 1 bpp            */
    L_SELECT_BY_HEIGHT = 2,         /*!< select by height; 1 bpp           */
    L_SELECT_BY_MAX_DIMENSION = 3,  /*!< select by max of width and        */
                                    /*!< height; 1 bpp                     */
    L_SELECT_BY_AREA = 4,           /*!< select by foreground area; 1 bpp  */
    L_SELECT_BY_PERIMETER = 5       /*!< select by perimeter; 1 bpp        */
};

/*! Location Filter */
enum {
    L_SELECT_WIDTH = 1,         /*!< width must satisfy constraint         */
    L_SELECT_HEIGHT = 2,        /*!< height must satisfy constraint        */
    L_SELECT_XVAL = 3,          /*!< x value must satisfy constraint       */
    L_SELECT_YVAL = 4,          /*!< y value must satisfy constraint       */
    L_SELECT_IF_EITHER = 5,     /*!< either width or height (or xval       */
                                /*!< or yval) can satisfy constraint       */
    L_SELECT_IF_BOTH = 6        /*!< both width and height (or xval        */
                                /*!< and yval must satisfy constraint      */
};

/*! Boxa Check */
enum {
    L_CHECK_WIDTH = 1,          /*!< check and possibly modify width       */
    L_CHECK_HEIGHT = 2,         /*!< check and possibly modify height      */
    L_CHECK_BOTH = 3            /*!< check and possibly modify both        */
};


/*-------------------------------------------------------------------------*
 *                    Color component selection flags                      *
 *-------------------------------------------------------------------------*/
/*! Color Selection */
enum {
    L_SELECT_RED = 1,           /*!< use red component                     */
    L_SELECT_GREEN = 2,         /*!< use green component                   */
    L_SELECT_BLUE = 3,          /*!< use blue component                    */
    L_SELECT_MIN = 4,           /*!< use min color component               */
    L_SELECT_MAX = 5,           /*!< use max color component               */
    L_SELECT_AVERAGE = 6,       /*!< use average of color components       */
    L_SELECT_HUE = 7,           /*!< use hue value (in HSV color space)    */
    L_SELECT_SATURATION = 8     /*!< use saturation value (in HSV space)   */
};


/*-------------------------------------------------------------------------*
 *                         16-bit conversion flags                         *
 *-------------------------------------------------------------------------*/
/*! 16-bit Conversion */
enum {
    L_LS_BYTE = 1,              /*!< use LSB                               */
    L_MS_BYTE = 2,              /*!< use MSB                               */
    L_AUTO_BYTE = 3,            /*!< use LSB if max(val) < 256; else MSB   */
    L_CLIP_TO_FF = 4,           /*!< use max(val, 255)                     */
    L_LS_TWO_BYTES = 5,         /*!< use two LSB                           */
    L_MS_TWO_BYTES = 6,         /*!< use two MSB                           */
    L_CLIP_TO_FFFF = 7          /*!< use max(val, 65535)                   */
};


/*-------------------------------------------------------------------------*
 *                        Rotate and shear flags                           *
 *-------------------------------------------------------------------------*/
/*! Rotation Type */
enum {
    L_ROTATE_AREA_MAP = 1,     /*!< use area map rotation, if possible     */
    L_ROTATE_SHEAR = 2,        /*!< use shear rotation                     */
    L_ROTATE_SAMPLING = 3      /*!< use sampling                           */
};

/*! Background Color */
enum {
    L_BRING_IN_WHITE = 1,      /*!< bring in white pixels from the outside */
    L_BRING_IN_BLACK = 2       /*!< bring in black pixels from the outside */
};

/*! Shear Point */
enum {
    L_SHEAR_ABOUT_CORNER = 1,  /*!< shear image about UL corner            */
    L_SHEAR_ABOUT_CENTER = 2   /*!< shear image about center               */
};


/*-------------------------------------------------------------------------*
 *                     Affine transform order flags                        *
 *-------------------------------------------------------------------------*/
/*! Affine Transform Order */
enum {
    L_TR_SC_RO = 1,            /*!< translate, scale, rotate               */
    L_SC_RO_TR = 2,            /*!< scale, rotate, translate               */
    L_RO_TR_SC = 3,            /*!< rotate, translate, scale               */
    L_TR_RO_SC = 4,            /*!< translate, rotate, scale               */
    L_RO_SC_TR = 5,            /*!< rotate, scale, translate               */
    L_SC_TR_RO = 6             /*!< scale, translate, rotate               */
};


/*-------------------------------------------------------------------------*
 *                       Grayscale filling flags                           *
 *-------------------------------------------------------------------------*/
/*! Grayscale Fill */
enum {
    L_FILL_WHITE = 1,         /*!< fill white pixels (e.g, in fg map)      */
    L_FILL_BLACK = 2          /*!< fill black pixels (e.g., in bg map)     */
};


/*-------------------------------------------------------------------------*
 *                   Flags for setting to white or black                   *
 *-------------------------------------------------------------------------*/
/*! BlackWhite Set */
enum {
    L_SET_WHITE = 1,         /*!< set pixels to white                      */
    L_SET_BLACK = 2          /*!< set pixels to black                      */
};


/*-------------------------------------------------------------------------*
 *                  Flags for getting white or black value                 *
 *-------------------------------------------------------------------------*/
/*! BlackWhite Get */
enum {
    L_GET_WHITE_VAL = 1,     /*!< get white pixel value                    */
    L_GET_BLACK_VAL = 2      /*!< get black pixel value                    */
};


/*-------------------------------------------------------------------------*
 *                  Flags for 8 bit and 16 bit pixel sums                  *
 *-------------------------------------------------------------------------*/
/*! BlackWhite Sum */
enum {
    L_WHITE_IS_MAX = 1, /*!< white pixels are 0xff or 0xffff; black are 0  */
    L_BLACK_IS_MAX = 2  /*!< black pixels are 0xff or 0xffff; white are 0  */
};


/*-------------------------------------------------------------------------*
 *                           Dither parameters                             *
 *         If within this grayscale distance from black or white,          *
 *         do not propagate excess or deficit to neighboring pixels.       *
 *-------------------------------------------------------------------------*/
/*! Dither Distance */
enum {
    DEFAULT_CLIP_LOWER_1 = 10, /*!< dist to black with no prop; 1 bpp      */
    DEFAULT_CLIP_UPPER_1 = 10, /*!< dist to black with no prop; 1 bpp      */
    DEFAULT_CLIP_LOWER_2 = 5,  /*!< dist to black with no prop; 2 bpp      */
    DEFAULT_CLIP_UPPER_2 = 5   /*!< dist to black with no prop; 2 bpp      */
};


/*-------------------------------------------------------------------------*
 *                          Distance type flags                            *
 *-------------------------------------------------------------------------*/
/*! Distance Type */
enum {
    L_MANHATTAN_DISTANCE = 1,  /*!< L1 distance (e.g., in color space)     */
    L_EUCLIDEAN_DISTANCE = 2   /*!< L2 distance                            */
};


/*-------------------------------------------------------------------------*
 *                         Distance Value flags                            *
 *-------------------------------------------------------------------------*/
/*! Distance Value */
enum {
    L_NEGATIVE = 1,      /*!< values < 0                                   */
    L_NON_NEGATIVE = 2,  /*!< values >= 0                                  */
    L_POSITIVE = 3,      /*!< values > 0                                   */
    L_NON_POSITIVE = 4,  /*!< values <= 0                                  */
    L_ZERO = 5,          /*!< values = 0                                   */
    L_ALL = 6            /*!< all values                                   */
};


/*-------------------------------------------------------------------------*
 *                         Statistical measures                            *
 *-------------------------------------------------------------------------*/
/*! Stats Type */
enum {
    L_MEAN_ABSVAL = 1,         /*!< average of abs values                  */
    L_MEDIAN_VAL = 2,          /*!< median value of set                    */
    L_MODE_VAL = 3,            /*!< mode value of set                      */
    L_MODE_COUNT = 4,          /*!< mode count of set                      */
    L_ROOT_MEAN_SQUARE = 5,    /*!< rms of values                          */
    L_STANDARD_DEVIATION = 6,  /*!< standard deviation from mean           */
    L_VARIANCE = 7             /*!< variance of values                     */
};


/*-------------------------------------------------------------------------*
 *                       Set index selection flags                         *
 *-------------------------------------------------------------------------*/
/*! Index Selection */
enum {
    L_CHOOSE_CONSECUTIVE = 1,  /*!< select 'n' consecutive                 */
    L_CHOOSE_SKIP_BY = 2       /*!< select at intervals of 'n'             */
};


/*-------------------------------------------------------------------------*
 *                         Text orientation flags                          *
 *-------------------------------------------------------------------------*/
/*! Text Orientation */
enum {
    L_TEXT_ORIENT_UNKNOWN = 0, /*!< low confidence on text orientation     */
    L_TEXT_ORIENT_UP = 1,      /*!< portrait, text rightside-up            */
    L_TEXT_ORIENT_LEFT = 2,    /*!< landscape, text up to left             */
    L_TEXT_ORIENT_DOWN = 3,    /*!< portrait, text upside-down             */
    L_TEXT_ORIENT_RIGHT = 4    /*!< landscape, text up to right            */
};


/*-------------------------------------------------------------------------*
 *                         Edge orientation flags                          *
 *-------------------------------------------------------------------------*/
/*! Edge Orientation */
enum {
    L_HORIZONTAL_EDGES = 0,   /*!< filters for horizontal edges            */
    L_VERTICAL_EDGES = 1,     /*!< filters for vertical edges              */
    L_ALL_EDGES = 2           /*!< filters for all edges                   */
};


/*-------------------------------------------------------------------------*
 *                         Line orientation flags                          *
 *-------------------------------------------------------------------------*/
/*! Line Orientation */
enum {
    L_HORIZONTAL_LINE = 0,   /*!< horizontal line                          */
    L_POS_SLOPE_LINE = 1,    /*!< 45 degree line with positive slope       */
    L_VERTICAL_LINE = 2,     /*!< vertical line                            */
    L_NEG_SLOPE_LINE = 3,    /*!< 45 degree line with negative slope       */
    L_OBLIQUE_LINE = 4       /*!< neither horizontal nor vertical */
};


/*-------------------------------------------------------------------------*
 *                         Image orientation flags                         *
 *-------------------------------------------------------------------------*/
/*! Image Orientation */
enum {
    L_PORTRAIT_MODE = 0,   /*!< typical: page is viewed with height > width  */
    L_LANDSCAPE_MODE = 1   /*!< page is viewed at 90 deg to portrait mode    */
};


/*-------------------------------------------------------------------------*
 *                           Scan direction flags                          *
 *-------------------------------------------------------------------------*/
/*! Scan Direction */
enum {
    L_FROM_LEFT = 0,         /*!< scan from left                           */
    L_FROM_RIGHT = 1,        /*!< scan from right                          */
    L_FROM_TOP = 2,          /*!< scan from top                            */
    L_FROM_BOT = 3,          /*!< scan from bottom                         */
    L_SCAN_NEGATIVE = 4,     /*!< scan in negative direction               */
    L_SCAN_POSITIVE = 5,     /*!< scan in positive direction               */
    L_SCAN_BOTH = 6,         /*!< scan in both directions                  */
    L_SCAN_HORIZONTAL = 7,   /*!< horizontal scan (direction unimportant)  */
    L_SCAN_VERTICAL = 8      /*!< vertical scan (direction unimportant)    */
};


/*-------------------------------------------------------------------------*
 *                Box size adjustment and location flags                   *
 *-------------------------------------------------------------------------*/
/*! Box Adjustment */
enum {
    L_ADJUST_SKIP = 0,           /*!< do not adjust                        */
    L_ADJUST_LEFT = 1,           /*!< adjust left edge                     */
    L_ADJUST_RIGHT = 2,          /*!< adjust right edge                    */
    L_ADJUST_LEFT_AND_RIGHT = 3, /*!< adjust both left and right edges     */
    L_ADJUST_TOP = 4,            /*!< adjust top edge                      */
    L_ADJUST_BOT = 5,            /*!< adjust bottom edge                   */
    L_ADJUST_TOP_AND_BOT = 6,    /*!< adjust both top and bottom edges     */
    L_ADJUST_CHOOSE_MIN = 7,     /*!< choose the min median value          */
    L_ADJUST_CHOOSE_MAX = 8,     /*!< choose the max median value          */
    L_SET_LEFT = 9,              /*!< set left side to a given value       */
    L_SET_RIGHT = 10,            /*!< set right side to a given value      */
    L_SET_TOP = 11,              /*!< set top side to a given value        */
    L_SET_BOT = 12,              /*!< set bottom side to a given value     */
    L_GET_LEFT = 13,             /*!< get left side location               */
    L_GET_RIGHT = 14,            /*!< get right side location              */
    L_GET_TOP = 15,              /*!< get top side location                */
    L_GET_BOT = 16               /*!< get bottom side location             */
};


/*-------------------------------------------------------------------------*
 *          Flags for modifying box boundaries using a second box          *
 *-------------------------------------------------------------------------*/
/*! Box Boundary Mod */
enum {
    L_USE_MINSIZE = 1,           /*!< use boundaries giving min size       */
    L_USE_MAXSIZE = 2,           /*!< use boundaries giving max size       */
    L_SUB_ON_LOC_DIFF = 3,       /*!< modify boundary if big location diff */
    L_SUB_ON_SIZE_DIFF = 4,      /*!< modify boundary if big size diff     */
    L_USE_CAPPED_MIN = 5,        /*!< modify boundary with capped min      */
    L_USE_CAPPED_MAX = 6         /*!< modify boundary with capped max      */
};


/*-------------------------------------------------------------------------*
 *              Handling overlapping bounding boxes in boxa                *
 *-------------------------------------------------------------------------*/
/*! Box Overlap Mod */
enum {
    L_COMBINE = 1,         /*!< resize to bounding region; remove smaller  */
    L_REMOVE_SMALL = 2     /*!< only remove smaller                        */
};


/*-------------------------------------------------------------------------*
 *        Selecting or making a box from two (intersecting) boxes          *
 *-------------------------------------------------------------------------*/
/*! Box Combine or Select */
enum {
    L_GEOMETRIC_UNION = 1,         /*!< use union of two boxes             */
    L_GEOMETRIC_INTERSECTION = 2,  /*!< use intersection of two boxes      */
    L_LARGEST_AREA = 3,            /*!< use box with largest area          */
    L_SMALLEST_AREA = 4            /*!< use box with smallest area         */
};


/*-------------------------------------------------------------------------*
 *                    Flags for replacing invalid boxes                    *
 *-------------------------------------------------------------------------*/
/*! Box Replacement */
enum {
    L_USE_ALL_BOXES = 1,         /*!< consider all boxes in the sequence   */
    L_USE_SAME_PARITY_BOXES = 2  /*!< consider boxes with the same parity  */
};


/*-------------------------------------------------------------------------*
 *                    Flags for box corners and center                     *
 *-------------------------------------------------------------------------*/
/*! Box Corners and Center */
enum {
    L_UPPER_LEFT = 1,         /*!< UL corner                               */
    L_UPPER_RIGHT = 2,        /*!< UR corner                               */
    L_LOWER_LEFT = 3,         /*!< LL corner                               */
    L_LOWER_RIGHT = 4,        /*!< LR corner                               */
    L_BOX_CENTER = 5          /*!< center                                  */
};


/*-------------------------------------------------------------------------*
 *                            Horizontal warp                              *
 *-------------------------------------------------------------------------*/
/*! Horiz Warp Stretch */
enum {
    L_WARP_TO_LEFT = 1,    /*!< increasing stretch or contraction to left  */
    L_WARP_TO_RIGHT = 2    /*!< increasing stretch or contraction to right */
};

/*! Horiz Warp Mode */
enum {
    L_LINEAR_WARP = 1,     /*!< stretch or contraction grows linearly      */
    L_QUADRATIC_WARP = 2   /*!< stretch or contraction grows quadratically */
};


/*-------------------------------------------------------------------------*
 *                      Pixel selection for resampling                     *
 *-------------------------------------------------------------------------*/
/*! Pixel Selection */
enum {
    L_INTERPOLATED = 1,    /*!< linear interpolation from src pixels       */
    L_SAMPLED = 2          /*!< nearest src pixel sampling only            */
};


/*-------------------------------------------------------------------------*
 *                             Thinning flags                              *
 *-------------------------------------------------------------------------*/
/*! Thinning Polarity */
enum {
    L_THIN_FG = 1,             /*!< thin foreground of 1 bpp image         */
    L_THIN_BG = 2              /*!< thin background of 1 bpp image         */
};


/*-------------------------------------------------------------------------*
 *                            Runlength flags                              *
 *-------------------------------------------------------------------------*/
/*! Runlength Direction */
enum {
    L_HORIZONTAL_RUNS = 0,   /*!< determine runlengths of horizontal runs  */
    L_VERTICAL_RUNS = 1      /*!< determine runlengths of vertical runs    */
};


/*-------------------------------------------------------------------------*
 *                          Edge filter flags                              *
 *-------------------------------------------------------------------------*/
/*! Edge Filter */
enum {
    L_SOBEL_EDGE = 1,        /*!< Sobel edge filter                        */
    L_TWO_SIDED_EDGE = 2     /*!< Two-sided edge filter                    */
};


/*-------------------------------------------------------------------------*
 *             Subpixel color component ordering in LCD display            *
 *-------------------------------------------------------------------------*/
/*! Subpixel Color Order */
enum {
    L_SUBPIXEL_ORDER_RGB = 1,   /*!< sensor order left-to-right RGB        */
    L_SUBPIXEL_ORDER_BGR = 2,   /*!< sensor order left-to-right BGR        */
    L_SUBPIXEL_ORDER_VRGB = 3,  /*!< sensor order top-to-bottom RGB        */
    L_SUBPIXEL_ORDER_VBGR = 4   /*!< sensor order top-to-bottom BGR        */
};


/*-------------------------------------------------------------------------*
 *                          HSV histogram flags                            *
 *-------------------------------------------------------------------------*/
/*! HSV Histogram */
enum {
    L_HS_HISTO = 1,            /*!< Use hue-saturation histogram           */
    L_HV_HISTO = 2,            /*!< Use hue-value histogram                */
    L_SV_HISTO = 3             /*!< Use saturation-value histogram         */
};


/*-------------------------------------------------------------------------*
 *                HSV Region flags (inclusion, exclusion)                  *
 *-------------------------------------------------------------------------*/
/*! HSV Region */
enum {
    L_INCLUDE_REGION = 1,      /*!< Use pixels with specified HSV region   */
    L_EXCLUDE_REGION = 2       /*!< Use pixels outside HSV region          */
};


/*-------------------------------------------------------------------------*
 *                Location flags for adding text to a pix                  *
 *-------------------------------------------------------------------------*/
/*! Add Text Location */
enum {
    L_ADD_ABOVE = 1,           /*!< Add text above the image               */
    L_ADD_BELOW = 2,           /*!< Add text below the image               */
    L_ADD_LEFT = 3,            /*!< Add text to the left of the image      */
    L_ADD_RIGHT = 4,           /*!< Add text to the right of the image     */
    L_ADD_AT_TOP = 5,          /*!< Add text over the top of the image     */
    L_ADD_AT_BOT = 6,          /*!< Add text over the bottom of the image  */
    L_ADD_AT_LEFT = 7,         /*!< Add text over left side of the image   */
    L_ADD_AT_RIGHT = 8         /*!< Add text over right side of the image  */
};


/*-------------------------------------------------------------------------*
 *                       Flags for plotting on a pix                       *
 *-------------------------------------------------------------------------*/
/*! Pix Plot */
enum {
    L_PLOT_AT_TOP = 1,         /*!< Plot horizontally at top               */
    L_PLOT_AT_MID_HORIZ = 2,   /*!< Plot horizontally at middle            */
    L_PLOT_AT_BOT = 3,         /*!< Plot horizontally at bottom            */
    L_PLOT_AT_LEFT = 4,        /*!< Plot vertically at left                */
    L_PLOT_AT_MID_VERT = 5,    /*!< Plot vertically at middle              */
    L_PLOT_AT_RIGHT = 6        /*!< Plot vertically at right               */
};


/*-------------------------------------------------------------------------*
 *                    Flags for making simple masks                        *
 *-------------------------------------------------------------------------*/
/*! Mask Generation */
enum {
    L_USE_INNER = 1,           /*!< Select the interior part               */
    L_USE_OUTER = 2            /*!< Select the outer part (e.g., a frame)  */
};


/*-------------------------------------------------------------------------*
 *                   Flags for selecting display program                   *
 *-------------------------------------------------------------------------*/
/*! Display Program */
enum {
    L_DISPLAY_WITH_XZGV = 1,  /*!< Use xzgv with pixDisplay()              */
    L_DISPLAY_WITH_XLI = 2,   /*!< Use xli with pixDisplay()               */
    L_DISPLAY_WITH_XV = 3,    /*!< Use xv with pixDisplay()                */
    L_DISPLAY_WITH_IV = 4,    /*!< Use irfvanview (win) with pixDisplay()  */
    L_DISPLAY_WITH_OPEN = 5   /*!< Use open (apple) with pixDisplay()      */
};

/*-------------------------------------------------------------------------*
 *    Flag(s) used in the 'special' pix field for non-default operations   *
 *      - 0 is default for chroma sampling in jpeg                         *
 *      - 10-19 are used for zlib compression in png write                 *
 *      - 4 and 8 are used for specifying connectivity in labelling        *
 *-------------------------------------------------------------------------*/
/*! Flags used in Pix::special */
enum {
    L_NO_CHROMA_SAMPLING_JPEG = 1   /*!< Write full resolution chroma      */
};


/*-------------------------------------------------------------------------*
 *          Handling negative values in conversion to unsigned int         *
 *-------------------------------------------------------------------------*/
/*! Negative Value */
enum {
    L_CLIP_TO_ZERO = 1,      /*!< Clip negative values to 0                */
    L_TAKE_ABSVAL = 2        /*!< Convert to positive using L_ABS()        */
};


/*-------------------------------------------------------------------------*
 *                        Relative to zero flags                           *
 *-------------------------------------------------------------------------*/
/*! Relative To Zero */
enum {
    L_LESS_THAN_ZERO = 1,    /*!< Choose values less than zero             */
    L_EQUAL_TO_ZERO = 2,     /*!< Choose values equal to zero              */
    L_GREATER_THAN_ZERO = 3  /*!< Choose values greater than zero          */
};


/*-------------------------------------------------------------------------*
 *         Flags for adding or removing trailing slash from string         *
 *-------------------------------------------------------------------------*/
/*! Trailing Slash */
enum {
    L_ADD_TRAIL_SLASH = 1,     /*!< Add trailing slash to string           */
    L_REMOVE_TRAIL_SLASH = 2   /*!< Remove trailing slash from string      */
};


/*-------------------------------------------------------------------------*
 *               Pix allocator and deallocator function types              *
 *-------------------------------------------------------------------------*/
/*! Allocator function type */
typedef void *(*alloc_fn)(size_t);

/*! Deallocator function type */
typedef void (*dealloc_fn)(void *);


#endif  /* LEPTONICA_PIX_H */
/*====================================================================*
 -  Copyright (C) 2001 Leptonica.  All rights reserved.
 -
 -  Redistribution and use in source and binary forms, with or without
 -  modification, are permitted provided that the following conditions
 -  are met:
 -  1. Redistributions of source code must retain the above copyright
 -     notice, this list of conditions and the following disclaimer.
 -  2. Redistributions in binary form must reproduce the above
 -     copyright notice, this list of conditions and the following
 -     disclaimer in the documentation and/or other materials
 -     provided with the distribution.
 -
 -  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 -  ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 -  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 -  A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL ANY
 -  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 -  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 -  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 -  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 -  OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 -  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 -  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *====================================================================*/

#ifndef  LEPTONICA_RECOG_H
#define  LEPTONICA_RECOG_H

/*!
 * \file recog.h
 *
 * <pre>
 *     This is a simple utility for training and recognizing individual
 *     machine-printed text characters.  It is designed to be adapted
 *     to a particular set of character images; e.g., from a book.
 *
 *     There are two methods of training the recognizer.  In the most
 *     simple, a set of bitmaps has been labeled by some means, such
 *     a generic OCR program.  This is input either one template at a time
 *     or as a pixa of templates, to a function that creates a recog.
 *     If in a pixa, the text string label must be embedded in the
 *     text field of each pix.
 *
 *     If labeled data is not available, we start with a bootstrap
 *     recognizer (BSR) that has labeled data from a variety of sources.
 *     These images are scaled, typically to a fixed height, and then
 *     fed similarly scaled unlabeled images from the source (e.g., book),
 *     and the BSR attempts to identify them.  All images that have
 *     a high enough correlation score with one of the templates in the
 *     BSR are emitted in a pixa, which now holds unscaled and labeled
 *     templates from the source.  This is the generator for a book adapted
 *     recognizer (BAR).
 *
 *     The pixa should always be thought of as the primary structure.
 *     It is the generator for the recog, because a recog is built
 *     from a pixa of unscaled images.
 *
 *     New image templates can be added to a recog as long as it is
 *     in training mode.  Once training is finished, to add templates
 *     it is necessary to extract the generating pixa, add templates
 *     to that pixa, and make a new recog.  Similarly, we do not
 *     join two recog; instead, we simply join their generating pixa,
 *     and make a recog from that.
 *
 *     To remove outliers from a pixa of labeled pix, make a recog,
 *     determine the outliers, and generate a new pixa with the
 *     outliers removed.  The outliers are determined by building
 *     special templates for each character set that are scaled averages
 *     of the individual templates.  Then a correlation score is found
 *     between each template and the averaged templates.  There are
 *     two implementations; outliers are determined as either:
 *      (1) a template having a correlation score with its class average
 *          that is below a threshold, or
 *      (2) a template having a correlation score with its class average
 *          that is smaller than the correlation score with the average
 *          of another class.
 *     Outliers are removed from the generating pixa.  Scaled averaging
 *     is only performed for determining outliers and for splitting
 *     characters; it is never used in a trained recognizer for identifying
 *     unlabeled samples.
 *
 *     Two methods using averaged templates are provided for splitting
 *     touching characters:
 *      (1) greedy matching
 *      (2) document image decoding (DID)
 *     The DID method is the default.  It is about 5x faster and
 *     possibly more accurate.
 *
 *     Once a BAR has been made, unlabeled sample images are identified
 *     by finding the individual template in the BAR with highest
 *     correlation.  The input images and images in the BAR can be
 *     represented in two ways:
 *      (1) as scanned, binarized to 1 bpp
 *      (2) as a width-normalized outline formed by thinning to a
 *          skeleton and then dilating by a fixed amount.
 *
 *     The recog can be serialized to file and read back.  The serialized
 *     version holds the templates used for correlation (which may have
 *     been modified by scaling and turning into lines from the unscaled
 *     templates), plus, for arbitrary character sets, the UTF8
 *     representation and the lookup table mapping from the character
 *     representation to index.
 *
 *     Why do we not use averaged templates for recognition?
 *     Letterforms can take on significantly different shapes (eg.,
 *     the letters 'a' and 'g'), and it makes no sense to average these.
 *     The previous version of this utility allowed multiple recognizers
 *     to exist, but this is an unnecessary complication if recognition
 *     is done on all samples instead of on averages.
 * </pre>
 */

#define  RECOG_VERSION_NUMBER      2

struct L_Recog {
    l_int32        scalew;       /*!< scale all examples to this width;      */
                                 /*!< use 0 prevent horizontal scaling       */
    l_int32        scaleh;       /*!< scale all examples to this height;     */
                                 /*!< use 0 prevent vertical scaling         */
    l_int32        linew;        /*!< use a value > 0 to convert the bitmap  */
                                 /*!< to lines of fixed width; 0 to skip     */
    l_int32        templ_use;    /*!< template use: use either the average   */
                                 /*!< or all temmplates (L_USE_AVERAGE or    */
                                 /*!< L_USE_ALL)                             */
    l_int32        maxarraysize; /*!< initialize container arrays to this    */
    l_int32        setsize;      /*!< size of character set                  */
    l_int32        threshold;    /*!< for binarizing if depth > 1            */
    l_int32        maxyshift;    /*!< vertical jiggle on nominal centroid    */
                                 /*!< alignment; typically 0 or 1            */
    l_int32        charset_type; /*!< one of L_ARABIC_NUMERALS, etc.         */
    l_int32        charset_size; /*!< expected number of classes in charset  */
    l_int32        min_nopad;    /*!< min number of samples without padding  */
    l_int32        num_samples;  /*!< number of training samples             */
    l_int32        minwidth_u;   /*!< min width averaged unscaled templates  */
    l_int32        maxwidth_u;   /*!< max width averaged unscaled templates  */
    l_int32        minheight_u;  /*!< min height averaged unscaled templates */
    l_int32        maxheight_u;  /*!< max height averaged unscaled templates */
    l_int32        minwidth;     /*!< min width averaged scaled templates    */
    l_int32        maxwidth;     /*!< max width averaged scaled templates    */
    l_int32        ave_done;     /*!< set to 1 when averaged bitmaps are made */
    l_int32        train_done;   /*!< set to 1 when training is complete or  */
                                 /*!< identification has started             */
    l_float32      max_wh_ratio; /*!< max width/height ratio to split        */
    l_float32      max_ht_ratio; /*!< max of max/min template height ratio   */
    l_int32        min_splitw;   /*!< min component width kept in splitting  */
    l_int32        max_splith;   /*!< max component height kept in splitting */
    struct Sarray *sa_text;      /*!< text array for arbitrary char set      */
    struct L_Dna  *dna_tochar;   /*!< index-to-char lut for arbitrary charset */
    l_int32       *centtab;      /*!< table for finding centroids            */
    l_int32       *sumtab;       /*!< table for finding pixel sums           */
    struct Pixaa  *pixaa_u;      /*!< all unscaled templates for each class  */
    struct Ptaa   *ptaa_u;       /*!< centroids of all unscaled templates    */
    struct Numaa  *naasum_u;     /*!< area of all unscaled templates         */
    struct Pixaa  *pixaa;        /*!< all (scaled) templates for each class  */
    struct Ptaa   *ptaa;         /*!< centroids of all (scaledl) templates   */
    struct Numaa  *naasum;       /*!< area of all (scaled) templates         */
    struct Pixa   *pixa_u;       /*!< averaged unscaled templates per class  */
    struct Pta    *pta_u;        /*!< centroids of unscaled ave. templates   */
    struct Numa   *nasum_u;      /*!< area of unscaled averaged templates    */
    struct Pixa   *pixa;         /*!< averaged (scaled) templates per class  */
    struct Pta    *pta;          /*!< centroids of (scaled) ave. templates   */
    struct Numa   *nasum;        /*!< area of (scaled) averaged templates    */
    struct Pixa   *pixa_tr;      /*!< all input training images              */
    struct Pixa   *pixadb_ave;   /*!< unscaled and scaled averaged bitmaps   */
    struct Pixa   *pixa_id;      /*!< input images for identifying           */
    struct Pix    *pixdb_ave;    /*!< debug: best match of input against ave. */
    struct Pix    *pixdb_range;  /*!< debug: best matches within range       */
    struct Pixa   *pixadb_boot;  /*!< debug: bootstrap training results      */
    struct Pixa   *pixadb_split; /*!< debug: splitting results               */
    struct L_Bmf  *bmf;          /*!< bmf fonts                              */
    l_int32        bmf_size;     /*!< font size of bmf; default is 6 pt      */
    struct L_Rdid *did;          /*!< temp data used for image decoding      */
    struct L_Rch  *rch;          /*!< temp data used for holding best char   */
    struct L_Rcha *rcha;         /*!< temp data used for array of best chars */
};
typedef struct L_Recog L_RECOG;

/*!
 *  Data returned from correlation matching on a single character
 */
struct L_Rch {
    l_int32        index;      /*!< index of best template                   */
    l_float32      score;      /*!< correlation score of best template       */
    char          *text;       /*!< character string of best template        */
    l_int32        sample;     /*!< index of best sample (within the best    */
                               /*!< template class, if all samples are used) */
    l_int32        xloc;       /*!< x-location of template (delx + shiftx)   */
    l_int32        yloc;       /*!< y-location of template (dely + shifty)   */
    l_int32        width;      /*!< width of best template                   */
};
typedef struct L_Rch L_RCH;

/*!
 *  Data returned from correlation matching on an array of characters
 */
struct L_Rcha {
    struct Numa   *naindex;    /*!< indices of best templates                */
    struct Numa   *nascore;    /*!< correlation scores of best templates     */
    struct Sarray *satext;     /*!< character strings of best templates      */
    struct Numa   *nasample;   /*!< indices of best samples                  */
    struct Numa   *naxloc;     /*!< x-locations of templates (delx + shiftx) */
    struct Numa   *nayloc;     /*!< y-locations of templates (dely + shifty) */
    struct Numa   *nawidth;    /*!< widths of best templates                 */
};
typedef struct L_Rcha L_RCHA;

/*!
 *  Data used for decoding a line of characters.
 */
struct L_Rdid {
    struct Pix    *pixs;         /*!< clone of pix to be decoded             */
    l_int32      **counta;       /*!< count array for each averaged template */
    l_int32      **delya;        /*!< best y-shift array per average template */
    l_int32        narray;       /*!< number of averaged templates           */
    l_int32        size;         /*!< size of count array (width of pixs)    */
    l_int32       *setwidth;     /*!< setwidths for each template            */
    struct Numa   *nasum;        /*!< pixel count in pixs by column          */
    struct Numa   *namoment;     /*!< first moment of pixels in pixs by cols */
    l_int32        fullarrays;   /*!< 1 if full arrays are made; 0 otherwise */
    l_float32     *beta;         /*!< channel coeffs for template fg term    */
    l_float32     *gamma;        /*!< channel coeffs for bit-and term        */
    l_float32     *trellisscore; /*!< score on trellis                       */
    l_int32       *trellistempl; /*!< template on trellis (for backtrack)    */
    struct Numa   *natempl;      /*!< indices of best path templates         */
    struct Numa   *naxloc;       /*!< x locations of best path templates     */
    struct Numa   *nadely;       /*!< y locations of best path templates     */
    struct Numa   *nawidth;      /*!< widths of best path templates          */
    struct Boxa   *boxa;         /*!< Viterbi result for splitting input pixs */
    struct Numa   *nascore;      /*!< correlation scores: best path templates */
    struct Numa   *natempl_r;    /*!< indices of best rescored templates     */
    struct Numa   *nasample_r;   /*!< samples of best scored templates       */
    struct Numa   *naxloc_r;     /*!< x locations of best rescoredtemplates  */
    struct Numa   *nadely_r;     /*!< y locations of best rescoredtemplates  */
    struct Numa   *nawidth_r;    /*!< widths of best rescoredtemplates       */
    struct Numa   *nascore_r;    /*!< correlation scores: rescored templates */
};
typedef struct L_Rdid L_RDID;


/*-------------------------------------------------------------------------*
 *             Flags for describing limited character sets                 *
 *-------------------------------------------------------------------------*/
/*! Character Set */
enum {
    L_UNKNOWN = 0,           /*!< character set type is not specified      */
    L_ARABIC_NUMERALS = 1,   /*!< 10 digits                                */
    L_LC_ROMAN_NUMERALS = 2, /*!< 7 lower-case letters (i,v,x,l,c,d,m)     */
    L_UC_ROMAN_NUMERALS = 3, /*!< 7 upper-case letters (I,V,X,L,C,D,M)     */
    L_LC_ALPHA = 4,          /*!< 26 lower-case letters                    */
    L_UC_ALPHA = 5           /*!< 26 upper-case letters                    */
};

/*-------------------------------------------------------------------------*
 *      Flags for selecting between using average and all templates:       *
 *                           recog->templ_use                              *
 *-------------------------------------------------------------------------*/
/*! Template Select */
enum {
    L_USE_ALL_TEMPLATES = 0,     /*!< use all templates; default            */
    L_USE_AVERAGE_TEMPLATES = 1  /*!< use average templates; special cases  */
};

#endif  /* LEPTONICA_RECOG_H */
/*====================================================================*
 -  Copyright (C) 2001 Leptonica.  All rights reserved.
 -
 -  Redistribution and use in source and binary forms, with or without
 -  modification, are permitted provided that the following conditions
 -  are met:
 -  1. Redistributions of source code must retain the above copyright
 -     notice, this list of conditions and the following disclaimer.
 -  2. Redistributions in binary form must reproduce the above
 -     copyright notice, this list of conditions and the following
 -     disclaimer in the documentation and/or other materials
 -     provided with the distribution.
 -
 -  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 -  ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 -  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 -  A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL ANY
 -  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 -  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 -  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 -  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 -  OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 -  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 -  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *====================================================================*/

#ifndef  LEPTONICA_REGUTILS_H
#define  LEPTONICA_REGUTILS_H

/*!
 * \file regutils.h
 *
 * <pre>
 *   Contains this regression test parameter packaging struct
 *       struct L_RegParams
 *
 *   The regression test utility allows you to write regression tests
 *   that compare results with existing "golden files" and with
 *   compiled in data.
 *
 *   Regression tests can be called in three ways.
 *   For example, for distance_reg:
 *
 *       Case 1: distance_reg [compare]
 *           This runs the test against the set of golden files.  It
 *           appends to 'outfile.txt' either "SUCCESS" or "FAILURE",
 *           as well as the details of any parts of the test that failed.
 *           It writes to a temporary file stream (fp).
 *           Using 'compare' on the command line is optional.
 *
 *       Case 2: distance_reg generate
 *           This generates golden files in /tmp for the reg test.
 *
 *       Case 3: distance_reg display
 *           This runs the test but makes no comparison of the output
 *           against the set of golden files.  In addition, this displays
 *           images and plots that are specified in the test under
 *           control of the display variable.  Display is enabled only
 *           for this case.
 *
 *   Regression tests follow the pattern given below.  Tests are
 *   automatically numbered sequentially, and it is convenient to
 *   comment each with a number to keep track (for comparison tests
 *   and for debugging).  In an actual case, comparisons of pix and
 *   of files can occur in any order.  We give a specific order here
 *   for clarity.
 *
 *       L_REGPARAMS  *rp;  // holds data required by the test functions
 *
 *       // Setup variables; optionally open stream
 *       if (regTestSetup(argc, argv, &rp))
 *           return 1;
 *
 *       // Test pairs of generated pix for identity.  This compares
 *       // two pix; no golden file is generated.
 *       regTestComparePix(rp, pix1, pix2);  // 0
 *
 *       // Test pairs of generated pix for similarity.  This compares
 *       // two pix; no golden file is generated.  The last arg determines
 *       // if stats are to be written to stderr.
 *       regTestCompareSimilarPix(rp, pix1, pix2, 15, 0.001, 0);  // 1
 *
 *       // Generation of <newfile*> outputs and testing for identity
 *       // These files can be anything, of course.
 *       regTestCheckFile(rp, <newfile0>);  // 2
 *       regTestCheckFile(rp, <newfile1>);  // 3
 *
 *       // Test pairs of output golden files for identity.  Here we
 *       // are comparing golden files 2 and 3.
 *       regTestCompareFiles(rp, 2, 3);  // 4
 *
 *       // "Write and check".  This writes a pix using a canonical
 *       // formulation for the local filename and either:
 *       //     case 1: generates a golden file
 *       //     case 2: compares the local file with a golden file
 *       //     case 3: generates local files and displays
 *       // Here we write the pix compressed with png and jpeg, respectively;
 *       // Then check against the golden file.  The internal %index
 *       // is incremented; it is embedded in the local filename and,
 *       // if generating, in the golden file as well.
 *       regTestWritePixAndCheck(rp, pix1, IFF_PNG);  // 5
 *       regTestWritePixAndCheck(rp, pix2, IFF_JFIF_JPEG);  // 6
 *
 *       // Display if reg test was called in 'display' mode
 *       pixDisplayWithTitle(pix1, 100, 100, NULL, rp->display);
 *
 *       // Clean up and output result
 *       regTestCleanup(rp);
 * </pre>
 */

/*----------------------------------------------------------------------------*
 *                      Regression test parameter packer                      *
 *----------------------------------------------------------------------------*/

/*! Regression test parameter packer */
struct L_RegParams
{
    FILE    *fp;        /*!< stream to temporary output file for compare mode */
    char    *testname;  /*!< name of test, without '_reg'                     */
    char    *tempfile;  /*!< name of temp file for compare mode output        */
    l_int32  mode;      /*!< generate, compare or display                     */
    l_int32  index;     /*!< index into saved files for this test; 0-based    */
    l_int32  success;   /*!< overall result of the test                       */
    l_int32  display;   /*!< 1 if in display mode; 0 otherwise                */
    L_TIMER  tstart;    /*!< marks beginning of the reg test                  */
};
typedef struct L_RegParams  L_REGPARAMS;


    /*! Running modes for the test */
/*! Regtest Mode */
enum {
    L_REG_GENERATE = 0,
    L_REG_COMPARE = 1,
    L_REG_DISPLAY = 2
};


#endif  /* LEPTONICA_REGUTILS_H */

/*====================================================================*
 -  Copyright (C) 2001 Leptonica.  All rights reserved.
 -
 -  Redistribution and use in source and binary forms, with or without
 -  modification, are permitted provided that the following conditions
 -  are met:
 -  1. Redistributions of source code must retain the above copyright
 -     notice, this list of conditions and the following disclaimer.
 -  2. Redistributions in binary form must reproduce the above
 -     copyright notice, this list of conditions and the following
 -     disclaimer in the documentation and/or other materials
 -     provided with the distribution.
 -
 -  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 -  ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 -  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 -  A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL ANY
 -  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 -  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 -  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 -  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 -  OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 -  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 -  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *====================================================================*/

#ifndef  LEPTONICA_STRINGCODE_H
#define  LEPTONICA_STRINGCODE_H

/*!
 * \file stringcode.h
 *
 *     Data structure to hold accumulating generated code for storing
 *     and extracting serializable leptonica objects (e.g., pixa, recog).
 *
 *     Also a flag for selecting a string from the L_GenAssoc struct
 *     in stringcode.
 */

struct L_StrCode
{
    l_int32       fileno;    /*!< index for function and output file names   */
    l_int32       ifunc;     /*!< index into struct currently being stored   */
    SARRAY       *function;  /*!< store case code for extraction             */
    SARRAY       *data;      /*!< store base64 encoded data as strings       */
    SARRAY       *descr;     /*!< store line in description table            */
    l_int32       n;         /*!< number of data strings                     */
};
typedef struct L_StrCode  L_STRCODE;


    /*! Select string in stringcode for a specific serializable data type */
/*! Stringcode Select */
enum {
    L_STR_TYPE = 0,      /*!< typedef for the data type                      */
    L_STR_NAME = 1,      /*!< name of the data type                          */
    L_STR_READER = 2,    /*!< reader to get the data type from file          */
    L_STR_MEMREADER = 3  /*!< reader to get the compressed string in memory  */
};

#endif  /* LEPTONICA_STRINGCODE_H */
/*====================================================================*
 -  Copyright (C) 2001 Leptonica.  All rights reserved.
 -
 -  Redistribution and use in source and binary forms, with or without
 -  modification, are permitted provided that the following conditions
 -  are met:
 -  1. Redistributions of source code must retain the above copyright
 -     notice, this list of conditions and the following disclaimer.
 -  2. Redistributions in binary form must reproduce the above
 -     copyright notice, this list of conditions and the following
 -     disclaimer in the documentation and/or other materials
 -     provided with the distribution.
 -
 -  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 -  ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 -  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 -  A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL ANY
 -  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 -  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 -  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 -  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 -  OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 -  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 -  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *====================================================================*/

#ifndef SUDOKU_H_INCLUDED
#define SUDOKU_H_INCLUDED

/*!
 * \file sudoku.h
 *
 * <pre>
 *    The L_Sudoku holds all the information of the current state.
 *
 *    The input to sudokuCreate() is a file with any number of lines
 *    starting with '#', followed by 9 lines consisting of 9 numbers
 *    in each line.  These have the known values and use 0 for the unknowns.
 *    Blank lines are ignored.
 *
 *    The %locs array holds the indices of the unknowns, numbered
 *    left-to-right and top-to-bottom from 0 to 80.  The array size
 *    is initialized to %num.  %current is the index into the %locs
 *    array of the current guess: locs[current].
 *
 *    The %state array is used to determine the validity of each guess.
 *    It is of size 81, and is initialized by setting the unknowns to 0
 *    and the knowns to their input values.
 * </pre>
 */

struct L_Sudoku
{
    l_int32        num;       /*!< number of unknowns                     */
    l_int32       *locs;      /*!< location of unknowns                   */
    l_int32        current;   /*!< index into %locs of current location   */
    l_int32       *init;      /*!< initial state, with 0 representing     */
                              /*!< the unknowns                           */
    l_int32       *state;     /*!< present state, including inits and     */
                              /*!< guesses of unknowns up to %current     */
    l_int32        nguess;    /*!< shows current number of guesses        */
    l_int32        finished;  /*!< set to 1 when solved                   */
    l_int32        failure;   /*!< set to 1 if no solution is possible    */
};
typedef struct L_Sudoku  L_SUDOKU;


    /*! For printing out array data */
/*! Sudoku Output */
enum {
    L_SUDOKU_INIT = 0,
    L_SUDOKU_STATE = 1
};

#endif /* SUDOKU_H_INCLUDED */


/*====================================================================*
 -  Copyright (C) 2001 Leptonica.  All rights reserved.
 -
 -  Redistribution and use in source and binary forms, with or without
 -  modification, are permitted provided that the following conditions
 -  are met:
 -  1. Redistributions of source code must retain the above copyright
 -     notice, this list of conditions and the following disclaimer.
 -  2. Redistributions in binary form must reproduce the above
 -     copyright notice, this list of conditions and the following
 -     disclaimer in the documentation and/or other materials
 -     provided with the distribution.
 -
 -  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 -  ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 -  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 -  A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL ANY
 -  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 -  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 -  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 -  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 -  OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 -  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 -  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *====================================================================*/

#ifndef  LEPTONICA_WATERSHED_H
#define  LEPTONICA_WATERSHED_H

/*!
 * \file watershed.h
 *
 *     Simple data structure to hold watershed data.
 *     All data here is owned by the L_WShed and must be freed.
 */

/*! Simple data structure to hold watershed data. */
struct L_WShed
{
    struct Pix    *pixs;      /*!< clone of input 8 bpp pixs                */
    struct Pix    *pixm;      /*!< clone of input 1 bpp seed (marker) pixm  */
    l_int32        mindepth;  /*!< minimum depth allowed for a watershed    */
    struct Pix    *pixlab;    /*!< 16 bpp label pix                         */
    struct Pix    *pixt;      /*!< scratch pix for computing wshed regions  */
    void         **lines8;    /*!< line ptrs for pixs                       */
    void         **linem1;    /*!< line ptrs for pixm                       */
    void         **linelab32; /*!< line ptrs for pixlab                     */
    void         **linet1;    /*!< line ptrs for pixt                       */
    struct Pixa   *pixad;     /*!< result: 1 bpp pixa of watersheds         */
    struct Pta    *ptas;      /*!< pta of initial seed pixels               */
    struct Numa   *nasi;      /*!< numa of seed indicators; 0 if completed  */
    struct Numa   *nash;      /*!< numa of initial seed heights             */
    struct Numa   *namh;      /*!< numa of initial minima heights           */
    struct Numa   *nalevels;  /*!< result: numa of watershed levels         */
    l_int32        nseeds;    /*!< number of seeds (markers)                */
    l_int32        nother;    /*!< number of minima different from seeds    */
    l_int32       *lut;       /*!< lut for pixel indices                    */
    struct Numa  **links;     /*!< back-links into lut, for updates         */
    l_int32        arraysize; /*!< size of links array                      */
    l_int32        debug;     /*!< set to 1 for debug output                */
};
typedef struct L_WShed L_WSHED;

#endif  /* LEPTONICA_WATERSHED_H */


#endif /* LEPTONICA_ALLTYPES_H */

#ifndef NO_PROTOS
/*
 *  These prototypes were autogen'd by xtractprotos, v. 1.5
 */
#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

LEPT_DLL extern PIX * pixCleanBackgroundToWhite ( PIX *pixs, PIX *pixim, PIX *pixg, l_float32 gamma, l_int32 blackval, l_int32 whiteval );
LEPT_DLL extern PIX * pixBackgroundNormSimple ( PIX *pixs, PIX *pixim, PIX *pixg );
LEPT_DLL extern PIX * pixBackgroundNorm ( PIX *pixs, PIX *pixim, PIX *pixg, l_int32 sx, l_int32 sy, l_int32 thresh, l_int32 mincount, l_int32 bgval, l_int32 smoothx, l_int32 smoothy );
LEPT_DLL extern PIX * pixBackgroundNormMorph ( PIX *pixs, PIX *pixim, l_int32 reduction, l_int32 size, l_int32 bgval );
LEPT_DLL extern l_ok pixBackgroundNormGrayArray ( PIX *pixs, PIX *pixim, l_int32 sx, l_int32 sy, l_int32 thresh, l_int32 mincount, l_int32 bgval, l_int32 smoothx, l_int32 smoothy, PIX **ppixd );
LEPT_DLL extern l_ok pixBackgroundNormRGBArrays ( PIX *pixs, PIX *pixim, PIX *pixg, l_int32 sx, l_int32 sy, l_int32 thresh, l_int32 mincount, l_int32 bgval, l_int32 smoothx, l_int32 smoothy, PIX **ppixr, PIX **ppixg, PIX **ppixb );
LEPT_DLL extern l_ok pixBackgroundNormGrayArrayMorph ( PIX *pixs, PIX *pixim, l_int32 reduction, l_int32 size, l_int32 bgval, PIX **ppixd );
LEPT_DLL extern l_ok pixBackgroundNormRGBArraysMorph ( PIX *pixs, PIX *pixim, l_int32 reduction, l_int32 size, l_int32 bgval, PIX **ppixr, PIX **ppixg, PIX **ppixb );
LEPT_DLL extern l_ok pixGetBackgroundGrayMap ( PIX *pixs, PIX *pixim, l_int32 sx, l_int32 sy, l_int32 thresh, l_int32 mincount, PIX **ppixd );
LEPT_DLL extern l_ok pixGetBackgroundRGBMap ( PIX *pixs, PIX *pixim, PIX *pixg, l_int32 sx, l_int32 sy, l_int32 thresh, l_int32 mincount, PIX **ppixmr, PIX **ppixmg, PIX **ppixmb );
LEPT_DLL extern l_ok pixGetBackgroundGrayMapMorph ( PIX *pixs, PIX *pixim, l_int32 reduction, l_int32 size, PIX **ppixm );
LEPT_DLL extern l_ok pixGetBackgroundRGBMapMorph ( PIX *pixs, PIX *pixim, l_int32 reduction, l_int32 size, PIX **ppixmr, PIX **ppixmg, PIX **ppixmb );
LEPT_DLL extern l_ok pixFillMapHoles ( PIX *pix, l_int32 nx, l_int32 ny, l_int32 filltype );
LEPT_DLL extern PIX * pixExtendByReplication ( PIX *pixs, l_int32 addw, l_int32 addh );
LEPT_DLL extern l_ok pixSmoothConnectedRegions ( PIX *pixs, PIX *pixm, l_int32 factor );
LEPT_DLL extern PIX * pixGetInvBackgroundMap ( PIX *pixs, l_int32 bgval, l_int32 smoothx, l_int32 smoothy );
LEPT_DLL extern PIX * pixApplyInvBackgroundGrayMap ( PIX *pixs, PIX *pixm, l_int32 sx, l_int32 sy );
LEPT_DLL extern PIX * pixApplyInvBackgroundRGBMap ( PIX *pixs, PIX *pixmr, PIX *pixmg, PIX *pixmb, l_int32 sx, l_int32 sy );
LEPT_DLL extern PIX * pixApplyVariableGrayMap ( PIX *pixs, PIX *pixg, l_int32 target );
LEPT_DLL extern PIX * pixGlobalNormRGB ( PIX *pixd, PIX *pixs, l_int32 rval, l_int32 gval, l_int32 bval, l_int32 mapval );
LEPT_DLL extern PIX * pixGlobalNormNoSatRGB ( PIX *pixd, PIX *pixs, l_int32 rval, l_int32 gval, l_int32 bval, l_int32 factor, l_float32 rank );
LEPT_DLL extern l_ok pixThresholdSpreadNorm ( PIX *pixs, l_int32 filtertype, l_int32 edgethresh, l_int32 smoothx, l_int32 smoothy, l_float32 gamma, l_int32 minval, l_int32 maxval, l_int32 targetthresh, PIX **ppixth, PIX **ppixb, PIX **ppixd );
LEPT_DLL extern PIX * pixBackgroundNormFlex ( PIX *pixs, l_int32 sx, l_int32 sy, l_int32 smoothx, l_int32 smoothy, l_int32 delta );
LEPT_DLL extern PIX * pixContrastNorm ( PIX *pixd, PIX *pixs, l_int32 sx, l_int32 sy, l_int32 mindiff, l_int32 smoothx, l_int32 smoothy );
LEPT_DLL extern l_ok pixMinMaxTiles ( PIX *pixs, l_int32 sx, l_int32 sy, l_int32 mindiff, l_int32 smoothx, l_int32 smoothy, PIX **ppixmin, PIX **ppixmax );
LEPT_DLL extern l_ok pixSetLowContrast ( PIX *pixs1, PIX *pixs2, l_int32 mindiff );
LEPT_DLL extern PIX * pixLinearTRCTiled ( PIX *pixd, PIX *pixs, l_int32 sx, l_int32 sy, PIX *pixmin, PIX *pixmax );
LEPT_DLL extern PIX * pixAffineSampledPta ( PIX *pixs, PTA *ptad, PTA *ptas, l_int32 incolor );
LEPT_DLL extern PIX * pixAffineSampled ( PIX *pixs, l_float32 *vc, l_int32 incolor );
LEPT_DLL extern PIX * pixAffinePta ( PIX *pixs, PTA *ptad, PTA *ptas, l_int32 incolor );
LEPT_DLL extern PIX * pixAffine ( PIX *pixs, l_float32 *vc, l_int32 incolor );
LEPT_DLL extern PIX * pixAffinePtaColor ( PIX *pixs, PTA *ptad, PTA *ptas, l_uint32 colorval );
LEPT_DLL extern PIX * pixAffineColor ( PIX *pixs, l_float32 *vc, l_uint32 colorval );
LEPT_DLL extern PIX * pixAffinePtaGray ( PIX *pixs, PTA *ptad, PTA *ptas, l_uint8 grayval );
LEPT_DLL extern PIX * pixAffineGray ( PIX *pixs, l_float32 *vc, l_uint8 grayval );
LEPT_DLL extern PIX * pixAffinePtaWithAlpha ( PIX *pixs, PTA *ptad, PTA *ptas, PIX *pixg, l_float32 fract, l_int32 border );
LEPT_DLL extern l_ok getAffineXformCoeffs ( PTA *ptas, PTA *ptad, l_float32 **pvc );
LEPT_DLL extern l_ok affineInvertXform ( l_float32 *vc, l_float32 **pvci );
LEPT_DLL extern l_ok affineXformSampledPt ( l_float32 *vc, l_int32 x, l_int32 y, l_int32 *pxp, l_int32 *pyp );
LEPT_DLL extern l_ok affineXformPt ( l_float32 *vc, l_int32 x, l_int32 y, l_float32 *pxp, l_float32 *pyp );
LEPT_DLL extern l_ok linearInterpolatePixelColor ( l_uint32 *datas, l_int32 wpls, l_int32 w, l_int32 h, l_float32 x, l_float32 y, l_uint32 colorval, l_uint32 *pval );
LEPT_DLL extern l_ok linearInterpolatePixelGray ( l_uint32 *datas, l_int32 wpls, l_int32 w, l_int32 h, l_float32 x, l_float32 y, l_int32 grayval, l_int32 *pval );
LEPT_DLL extern l_int32 gaussjordan ( l_float32 **a, l_float32 *b, l_int32 n );
LEPT_DLL extern PIX * pixAffineSequential ( PIX *pixs, PTA *ptad, PTA *ptas, l_int32 bw, l_int32 bh );
LEPT_DLL extern l_float32 * createMatrix2dTranslate ( l_float32 transx, l_float32 transy );
LEPT_DLL extern l_float32 * createMatrix2dScale ( l_float32 scalex, l_float32 scaley );
LEPT_DLL extern l_float32 * createMatrix2dRotate ( l_float32 xc, l_float32 yc, l_float32 angle );
LEPT_DLL extern PTA * ptaTranslate ( PTA *ptas, l_float32 transx, l_float32 transy );
LEPT_DLL extern PTA * ptaScale ( PTA *ptas, l_float32 scalex, l_float32 scaley );
LEPT_DLL extern PTA * ptaRotate ( PTA *ptas, l_float32 xc, l_float32 yc, l_float32 angle );
LEPT_DLL extern BOXA * boxaTranslate ( BOXA *boxas, l_float32 transx, l_float32 transy );
LEPT_DLL extern BOXA * boxaScale ( BOXA *boxas, l_float32 scalex, l_float32 scaley );
LEPT_DLL extern BOXA * boxaRotate ( BOXA *boxas, l_float32 xc, l_float32 yc, l_float32 angle );
LEPT_DLL extern PTA * ptaAffineTransform ( PTA *ptas, l_float32 *mat );
LEPT_DLL extern BOXA * boxaAffineTransform ( BOXA *boxas, l_float32 *mat );
LEPT_DLL extern l_ok l_productMatVec ( l_float32 *mat, l_float32 *vecs, l_float32 *vecd, l_int32 size );
LEPT_DLL extern l_ok l_productMat2 ( l_float32 *mat1, l_float32 *mat2, l_float32 *matd, l_int32 size );
LEPT_DLL extern l_ok l_productMat3 ( l_float32 *mat1, l_float32 *mat2, l_float32 *mat3, l_float32 *matd, l_int32 size );
LEPT_DLL extern l_ok l_productMat4 ( l_float32 *mat1, l_float32 *mat2, l_float32 *mat3, l_float32 *mat4, l_float32 *matd, l_int32 size );
LEPT_DLL extern l_int32 l_getDataBit ( const void *line, l_int32 n );
LEPT_DLL extern void l_setDataBit ( void *line, l_int32 n );
LEPT_DLL extern void l_clearDataBit ( void *line, l_int32 n );
LEPT_DLL extern void l_setDataBitVal ( void *line, l_int32 n, l_int32 val );
LEPT_DLL extern l_int32 l_getDataDibit ( const void *line, l_int32 n );
LEPT_DLL extern void l_setDataDibit ( void *line, l_int32 n, l_int32 val );
LEPT_DLL extern void l_clearDataDibit ( void *line, l_int32 n );
LEPT_DLL extern l_int32 l_getDataQbit ( const void *line, l_int32 n );
LEPT_DLL extern void l_setDataQbit ( void *line, l_int32 n, l_int32 val );
LEPT_DLL extern void l_clearDataQbit ( void *line, l_int32 n );
LEPT_DLL extern l_int32 l_getDataByte ( const void *line, l_int32 n );
LEPT_DLL extern void l_setDataByte ( void *line, l_int32 n, l_int32 val );
LEPT_DLL extern l_int32 l_getDataTwoBytes ( const void *line, l_int32 n );
LEPT_DLL extern void l_setDataTwoBytes ( void *line, l_int32 n, l_int32 val );
LEPT_DLL extern l_int32 l_getDataFourBytes ( const void *line, l_int32 n );
LEPT_DLL extern void l_setDataFourBytes ( void *line, l_int32 n, l_int32 val );
LEPT_DLL extern char * barcodeDispatchDecoder ( char *barstr, l_int32 format, l_int32 debugflag );
LEPT_DLL extern l_int32 barcodeFormatIsSupported ( l_int32 format );
LEPT_DLL extern NUMA * pixFindBaselines ( PIX *pixs, PTA **ppta, PIXA *pixadb );
LEPT_DLL extern PIX * pixDeskewLocal ( PIX *pixs, l_int32 nslices, l_int32 redsweep, l_int32 redsearch, l_float32 sweeprange, l_float32 sweepdelta, l_float32 minbsdelta );
LEPT_DLL extern l_ok pixGetLocalSkewTransform ( PIX *pixs, l_int32 nslices, l_int32 redsweep, l_int32 redsearch, l_float32 sweeprange, l_float32 sweepdelta, l_float32 minbsdelta, PTA **pptas, PTA **pptad );
LEPT_DLL extern NUMA * pixGetLocalSkewAngles ( PIX *pixs, l_int32 nslices, l_int32 redsweep, l_int32 redsearch, l_float32 sweeprange, l_float32 sweepdelta, l_float32 minbsdelta, l_float32 *pa, l_float32 *pb, l_int32 debug );
LEPT_DLL extern L_BBUFFER * bbufferCreate ( const l_uint8 *indata, l_int32 nalloc );
LEPT_DLL extern void bbufferDestroy ( L_BBUFFER **pbb );
LEPT_DLL extern l_uint8 * bbufferDestroyAndSaveData ( L_BBUFFER **pbb, size_t *pnbytes );
LEPT_DLL extern l_ok bbufferRead ( L_BBUFFER *bb, l_uint8 *src, l_int32 nbytes );
LEPT_DLL extern l_ok bbufferReadStream ( L_BBUFFER *bb, FILE *fp, l_int32 nbytes );
LEPT_DLL extern l_ok bbufferExtendArray ( L_BBUFFER *bb, l_int32 nbytes );
LEPT_DLL extern l_ok bbufferWrite ( L_BBUFFER *bb, l_uint8 *dest, size_t nbytes, size_t *pnout );
LEPT_DLL extern l_ok bbufferWriteStream ( L_BBUFFER *bb, FILE *fp, size_t nbytes, size_t *pnout );
LEPT_DLL extern PIX * pixBilateral ( PIX *pixs, l_float32 spatial_stdev, l_float32 range_stdev, l_int32 ncomps, l_int32 reduction );
LEPT_DLL extern PIX * pixBilateralGray ( PIX *pixs, l_float32 spatial_stdev, l_float32 range_stdev, l_int32 ncomps, l_int32 reduction );
LEPT_DLL extern PIX * pixBilateralExact ( PIX *pixs, L_KERNEL *spatial_kel, L_KERNEL *range_kel );
LEPT_DLL extern PIX * pixBilateralGrayExact ( PIX *pixs, L_KERNEL *spatial_kel, L_KERNEL *range_kel );
LEPT_DLL extern PIX* pixBlockBilateralExact ( PIX *pixs, l_float32 spatial_stdev, l_float32 range_stdev );
LEPT_DLL extern L_KERNEL * makeRangeKernel ( l_float32 range_stdev );
LEPT_DLL extern PIX * pixBilinearSampledPta ( PIX *pixs, PTA *ptad, PTA *ptas, l_int32 incolor );
LEPT_DLL extern PIX * pixBilinearSampled ( PIX *pixs, l_float32 *vc, l_int32 incolor );
LEPT_DLL extern PIX * pixBilinearPta ( PIX *pixs, PTA *ptad, PTA *ptas, l_int32 incolor );
LEPT_DLL extern PIX * pixBilinear ( PIX *pixs, l_float32 *vc, l_int32 incolor );
LEPT_DLL extern PIX * pixBilinearPtaColor ( PIX *pixs, PTA *ptad, PTA *ptas, l_uint32 colorval );
LEPT_DLL extern PIX * pixBilinearColor ( PIX *pixs, l_float32 *vc, l_uint32 colorval );
LEPT_DLL extern PIX * pixBilinearPtaGray ( PIX *pixs, PTA *ptad, PTA *ptas, l_uint8 grayval );
LEPT_DLL extern PIX * pixBilinearGray ( PIX *pixs, l_float32 *vc, l_uint8 grayval );
LEPT_DLL extern PIX * pixBilinearPtaWithAlpha ( PIX *pixs, PTA *ptad, PTA *ptas, PIX *pixg, l_float32 fract, l_int32 border );
LEPT_DLL extern l_ok getBilinearXformCoeffs ( PTA *ptas, PTA *ptad, l_float32 **pvc );
LEPT_DLL extern l_ok bilinearXformSampledPt ( l_float32 *vc, l_int32 x, l_int32 y, l_int32 *pxp, l_int32 *pyp );
LEPT_DLL extern l_ok bilinearXformPt ( l_float32 *vc, l_int32 x, l_int32 y, l_float32 *pxp, l_float32 *pyp );
LEPT_DLL extern l_ok pixOtsuAdaptiveThreshold ( PIX *pixs, l_int32 sx, l_int32 sy, l_int32 smoothx, l_int32 smoothy, l_float32 scorefract, PIX **ppixth, PIX **ppixd );
LEPT_DLL extern PIX * pixOtsuThreshOnBackgroundNorm ( PIX *pixs, PIX *pixim, l_int32 sx, l_int32 sy, l_int32 thresh, l_int32 mincount, l_int32 bgval, l_int32 smoothx, l_int32 smoothy, l_float32 scorefract, l_int32 *pthresh );
LEPT_DLL extern PIX * pixMaskedThreshOnBackgroundNorm ( PIX *pixs, PIX *pixim, l_int32 sx, l_int32 sy, l_int32 thresh, l_int32 mincount, l_int32 smoothx, l_int32 smoothy, l_float32 scorefract, l_int32 *pthresh );
LEPT_DLL extern l_ok pixSauvolaBinarizeTiled ( PIX *pixs, l_int32 whsize, l_float32 factor, l_int32 nx, l_int32 ny, PIX **ppixth, PIX **ppixd );
LEPT_DLL extern l_ok pixSauvolaBinarize ( PIX *pixs, l_int32 whsize, l_float32 factor, l_int32 addborder, PIX **ppixm, PIX **ppixsd, PIX **ppixth, PIX **ppixd );
LEPT_DLL extern l_ok pixThresholdByConnComp ( PIX *pixs, PIX *pixm, l_int32 start, l_int32 end, l_int32 incr, l_float32 thresh48, l_float32 threshdiff, l_int32 *pglobthresh, PIX **ppixd, l_int32 debugflag );
LEPT_DLL extern l_ok pixThresholdByHisto ( PIX *pixs, l_int32 factor, l_int32 halfw, l_float32 delta, l_int32 *pthresh, PIX **ppixd, PIX **ppixhisto );
LEPT_DLL extern PIX * pixExpandBinaryReplicate ( PIX *pixs, l_int32 xfact, l_int32 yfact );
LEPT_DLL extern PIX * pixExpandBinaryPower2 ( PIX *pixs, l_int32 factor );
LEPT_DLL extern PIX * pixReduceBinary2 ( PIX *pixs, l_uint8 *intab );
LEPT_DLL extern PIX * pixReduceRankBinaryCascade ( PIX *pixs, l_int32 level1, l_int32 level2, l_int32 level3, l_int32 level4 );
LEPT_DLL extern PIX * pixReduceRankBinary2 ( PIX *pixs, l_int32 level, l_uint8 *intab );
LEPT_DLL extern l_uint8 * makeSubsampleTab2x ( void );
LEPT_DLL extern PIX * pixBlend ( PIX *pixs1, PIX *pixs2, l_int32 x, l_int32 y, l_float32 fract );
LEPT_DLL extern PIX * pixBlendMask ( PIX *pixd, PIX *pixs1, PIX *pixs2, l_int32 x, l_int32 y, l_float32 fract, l_int32 type );
LEPT_DLL extern PIX * pixBlendGray ( PIX *pixd, PIX *pixs1, PIX *pixs2, l_int32 x, l_int32 y, l_float32 fract, l_int32 type, l_int32 transparent, l_uint32 transpix );
LEPT_DLL extern PIX * pixBlendGrayInverse ( PIX *pixd, PIX *pixs1, PIX *pixs2, l_int32 x, l_int32 y, l_float32 fract );
LEPT_DLL extern PIX * pixBlendColor ( PIX *pixd, PIX *pixs1, PIX *pixs2, l_int32 x, l_int32 y, l_float32 fract, l_int32 transparent, l_uint32 transpix );
LEPT_DLL extern PIX * pixBlendColorByChannel ( PIX *pixd, PIX *pixs1, PIX *pixs2, l_int32 x, l_int32 y, l_float32 rfract, l_float32 gfract, l_float32 bfract, l_int32 transparent, l_uint32 transpix );
LEPT_DLL extern PIX * pixBlendGrayAdapt ( PIX *pixd, PIX *pixs1, PIX *pixs2, l_int32 x, l_int32 y, l_float32 fract, l_int32 shift );
LEPT_DLL extern PIX * pixFadeWithGray ( PIX *pixs, PIX *pixb, l_float32 factor, l_int32 type );
LEPT_DLL extern PIX * pixBlendHardLight ( PIX *pixd, PIX *pixs1, PIX *pixs2, l_int32 x, l_int32 y, l_float32 fract );
LEPT_DLL extern l_ok pixBlendCmap ( PIX *pixs, PIX *pixb, l_int32 x, l_int32 y, l_int32 sindex );
LEPT_DLL extern PIX * pixBlendWithGrayMask ( PIX *pixs1, PIX *pixs2, PIX *pixg, l_int32 x, l_int32 y );
LEPT_DLL extern PIX * pixBlendBackgroundToColor ( PIX *pixd, PIX *pixs, BOX *box, l_uint32 color, l_float32 gamma, l_int32 minval, l_int32 maxval );
LEPT_DLL extern PIX * pixMultiplyByColor ( PIX *pixd, PIX *pixs, BOX *box, l_uint32 color );
LEPT_DLL extern PIX * pixAlphaBlendUniform ( PIX *pixs, l_uint32 color );
LEPT_DLL extern PIX * pixAddAlphaToBlend ( PIX *pixs, l_float32 fract, l_int32 invert );
LEPT_DLL extern PIX * pixSetAlphaOverWhite ( PIX *pixs );
LEPT_DLL extern l_ok pixLinearEdgeFade ( PIX *pixs, l_int32 dir, l_int32 fadeto, l_float32 distfract, l_float32 maxfade );
LEPT_DLL extern L_BMF * bmfCreate ( const char *dir, l_int32 fontsize );
LEPT_DLL extern void bmfDestroy ( L_BMF **pbmf );
LEPT_DLL extern PIX * bmfGetPix ( L_BMF *bmf, char chr );
LEPT_DLL extern l_ok bmfGetWidth ( L_BMF *bmf, char chr, l_int32 *pw );
LEPT_DLL extern l_ok bmfGetBaseline ( L_BMF *bmf, char chr, l_int32 *pbaseline );
LEPT_DLL extern PIXA * pixaGetFont ( const char *dir, l_int32 fontsize, l_int32 *pbl0, l_int32 *pbl1, l_int32 *pbl2 );
LEPT_DLL extern l_ok pixaSaveFont ( const char *indir, const char *outdir, l_int32 fontsize );
LEPT_DLL extern PIX * pixReadStreamBmp ( FILE *fp );
LEPT_DLL extern PIX * pixReadMemBmp ( const l_uint8 *cdata, size_t size );
LEPT_DLL extern l_ok pixWriteStreamBmp ( FILE *fp, PIX *pix );
LEPT_DLL extern l_ok pixWriteMemBmp ( l_uint8 **pfdata, size_t *pfsize, PIX *pixs );
LEPT_DLL extern PIXA * l_bootnum_gen1 ( void );
LEPT_DLL extern PIXA * l_bootnum_gen2 ( void );
LEPT_DLL extern PIXA * l_bootnum_gen3 ( void );
LEPT_DLL extern PIXA * l_bootnum_gen4 ( l_int32 nsamp );
LEPT_DLL extern BOX * boxCreate ( l_int32 x, l_int32 y, l_int32 w, l_int32 h );
LEPT_DLL extern BOX * boxCreateValid ( l_int32 x, l_int32 y, l_int32 w, l_int32 h );
LEPT_DLL extern BOX * boxCopy ( BOX *box );
LEPT_DLL extern BOX * boxClone ( BOX *box );
LEPT_DLL extern void boxDestroy ( BOX **pbox );
LEPT_DLL extern l_ok boxGetGeometry ( BOX *box, l_int32 *px, l_int32 *py, l_int32 *pw, l_int32 *ph );
LEPT_DLL extern l_ok boxSetGeometry ( BOX *box, l_int32 x, l_int32 y, l_int32 w, l_int32 h );
LEPT_DLL extern l_ok boxGetSideLocations ( BOX *box, l_int32 *pl, l_int32 *pr, l_int32 *pt, l_int32 *pb );
LEPT_DLL extern l_ok boxSetSideLocations ( BOX *box, l_int32 l, l_int32 r, l_int32 t, l_int32 b );
LEPT_DLL extern l_int32 boxGetRefcount ( BOX *box );
LEPT_DLL extern l_ok boxChangeRefcount ( BOX *box, l_int32 delta );
LEPT_DLL extern l_ok boxIsValid ( BOX *box, l_int32 *pvalid );
LEPT_DLL extern BOXA * boxaCreate ( l_int32 n );
LEPT_DLL extern BOXA * boxaCopy ( BOXA *boxa, l_int32 copyflag );
LEPT_DLL extern void boxaDestroy ( BOXA **pboxa );
LEPT_DLL extern l_ok boxaAddBox ( BOXA *boxa, BOX *box, l_int32 copyflag );
LEPT_DLL extern l_ok boxaExtendArray ( BOXA *boxa );
LEPT_DLL extern l_ok boxaExtendArrayToSize ( BOXA *boxa, l_int32 size );
LEPT_DLL extern l_int32 boxaGetCount ( BOXA *boxa );
LEPT_DLL extern l_int32 boxaGetValidCount ( BOXA *boxa );
LEPT_DLL extern BOX * boxaGetBox ( BOXA *boxa, l_int32 index, l_int32 accessflag );
LEPT_DLL extern BOX * boxaGetValidBox ( BOXA *boxa, l_int32 index, l_int32 accessflag );
LEPT_DLL extern NUMA * boxaFindInvalidBoxes ( BOXA *boxa );
LEPT_DLL extern l_ok boxaGetBoxGeometry ( BOXA *boxa, l_int32 index, l_int32 *px, l_int32 *py, l_int32 *pw, l_int32 *ph );
LEPT_DLL extern l_ok boxaIsFull ( BOXA *boxa, l_int32 *pfull );
LEPT_DLL extern l_ok boxaReplaceBox ( BOXA *boxa, l_int32 index, BOX *box );
LEPT_DLL extern l_ok boxaInsertBox ( BOXA *boxa, l_int32 index, BOX *box );
LEPT_DLL extern l_ok boxaRemoveBox ( BOXA *boxa, l_int32 index );
LEPT_DLL extern l_ok boxaRemoveBoxAndSave ( BOXA *boxa, l_int32 index, BOX **pbox );
LEPT_DLL extern BOXA * boxaSaveValid ( BOXA *boxas, l_int32 copyflag );
LEPT_DLL extern l_ok boxaInitFull ( BOXA *boxa, BOX *box );
LEPT_DLL extern l_ok boxaClear ( BOXA *boxa );
LEPT_DLL extern BOXAA * boxaaCreate ( l_int32 n );
LEPT_DLL extern BOXAA * boxaaCopy ( BOXAA *baas, l_int32 copyflag );
LEPT_DLL extern void boxaaDestroy ( BOXAA **pbaa );
LEPT_DLL extern l_ok boxaaAddBoxa ( BOXAA *baa, BOXA *ba, l_int32 copyflag );
LEPT_DLL extern l_ok boxaaExtendArray ( BOXAA *baa );
LEPT_DLL extern l_ok boxaaExtendArrayToSize ( BOXAA *baa, l_int32 size );
LEPT_DLL extern l_int32 boxaaGetCount ( BOXAA *baa );
LEPT_DLL extern l_int32 boxaaGetBoxCount ( BOXAA *baa );
LEPT_DLL extern BOXA * boxaaGetBoxa ( BOXAA *baa, l_int32 index, l_int32 accessflag );
LEPT_DLL extern BOX * boxaaGetBox ( BOXAA *baa, l_int32 iboxa, l_int32 ibox, l_int32 accessflag );
LEPT_DLL extern l_ok boxaaInitFull ( BOXAA *baa, BOXA *boxa );
LEPT_DLL extern l_ok boxaaExtendWithInit ( BOXAA *baa, l_int32 maxindex, BOXA *boxa );
LEPT_DLL extern l_ok boxaaReplaceBoxa ( BOXAA *baa, l_int32 index, BOXA *boxa );
LEPT_DLL extern l_ok boxaaInsertBoxa ( BOXAA *baa, l_int32 index, BOXA *boxa );
LEPT_DLL extern l_ok boxaaRemoveBoxa ( BOXAA *baa, l_int32 index );
LEPT_DLL extern l_ok boxaaAddBox ( BOXAA *baa, l_int32 index, BOX *box, l_int32 accessflag );
LEPT_DLL extern BOXAA * boxaaReadFromFiles ( const char *dirname, const char *substr, l_int32 first, l_int32 nfiles );
LEPT_DLL extern BOXAA * boxaaRead ( const char *filename );
LEPT_DLL extern BOXAA * boxaaReadStream ( FILE *fp );
LEPT_DLL extern BOXAA * boxaaReadMem ( const l_uint8 *data, size_t size );
LEPT_DLL extern l_ok boxaaWrite ( const char *filename, BOXAA *baa );
LEPT_DLL extern l_ok boxaaWriteStream ( FILE *fp, BOXAA *baa );
LEPT_DLL extern l_ok boxaaWriteMem ( l_uint8 **pdata, size_t *psize, BOXAA *baa );
LEPT_DLL extern BOXA * boxaRead ( const char *filename );
LEPT_DLL extern BOXA * boxaReadStream ( FILE *fp );
LEPT_DLL extern BOXA * boxaReadMem ( const l_uint8 *data, size_t size );
LEPT_DLL extern l_ok boxaWriteDebug ( const char *filename, BOXA *boxa );
LEPT_DLL extern l_ok boxaWrite ( const char *filename, BOXA *boxa );
LEPT_DLL extern l_ok boxaWriteStream ( FILE *fp, BOXA *boxa );
LEPT_DLL extern l_ok boxaWriteStderr ( BOXA *boxa );
LEPT_DLL extern l_ok boxaWriteMem ( l_uint8 **pdata, size_t *psize, BOXA *boxa );
LEPT_DLL extern l_ok boxPrintStreamInfo ( FILE *fp, BOX *box );
LEPT_DLL extern l_ok boxContains ( BOX *box1, BOX *box2, l_int32 *presult );
LEPT_DLL extern l_ok boxIntersects ( BOX *box1, BOX *box2, l_int32 *presult );
LEPT_DLL extern BOXA * boxaContainedInBox ( BOXA *boxas, BOX *box );
LEPT_DLL extern l_ok boxaContainedInBoxCount ( BOXA *boxa, BOX *box, l_int32 *pcount );
LEPT_DLL extern l_ok boxaContainedInBoxa ( BOXA *boxa1, BOXA *boxa2, l_int32 *pcontained );
LEPT_DLL extern BOXA * boxaIntersectsBox ( BOXA *boxas, BOX *box );
LEPT_DLL extern l_ok boxaIntersectsBoxCount ( BOXA *boxa, BOX *box, l_int32 *pcount );
LEPT_DLL extern BOXA * boxaClipToBox ( BOXA *boxas, BOX *box );
LEPT_DLL extern BOXA * boxaCombineOverlaps ( BOXA *boxas, PIXA *pixadb );
LEPT_DLL extern l_ok boxaCombineOverlapsInPair ( BOXA *boxas1, BOXA *boxas2, BOXA **pboxad1, BOXA **pboxad2, PIXA *pixadb );
LEPT_DLL extern BOX * boxOverlapRegion ( BOX *box1, BOX *box2 );
LEPT_DLL extern BOX * boxBoundingRegion ( BOX *box1, BOX *box2 );
LEPT_DLL extern l_ok boxOverlapFraction ( BOX *box1, BOX *box2, l_float32 *pfract );
LEPT_DLL extern l_ok boxOverlapArea ( BOX *box1, BOX *box2, l_int32 *parea );
LEPT_DLL extern BOXA * boxaHandleOverlaps ( BOXA *boxas, l_int32 op, l_int32 range, l_float32 min_overlap, l_float32 max_ratio, NUMA **pnamap );
LEPT_DLL extern l_ok boxOverlapDistance ( BOX *box1, BOX *box2, l_int32 *ph_ovl, l_int32 *pv_ovl );
LEPT_DLL extern l_ok boxSeparationDistance ( BOX *box1, BOX *box2, l_int32 *ph_sep, l_int32 *pv_sep );
LEPT_DLL extern l_ok boxCompareSize ( BOX *box1, BOX *box2, l_int32 type, l_int32 *prel );
LEPT_DLL extern l_ok boxContainsPt ( BOX *box, l_float32 x, l_float32 y, l_int32 *pcontains );
LEPT_DLL extern BOX * boxaGetNearestToPt ( BOXA *boxa, l_int32 x, l_int32 y );
LEPT_DLL extern BOX * boxaGetNearestToLine ( BOXA *boxa, l_int32 x, l_int32 y );
LEPT_DLL extern l_ok boxaFindNearestBoxes ( BOXA *boxa, l_int32 dist_select, l_int32 range, NUMAA **pnaaindex, NUMAA **pnaadist );
LEPT_DLL extern l_ok boxaGetNearestByDirection ( BOXA *boxa, l_int32 i, l_int32 dir, l_int32 dist_select, l_int32 range, l_int32 *pindex, l_int32 *pdist );
LEPT_DLL extern l_ok boxGetCenter ( BOX *box, l_float32 *pcx, l_float32 *pcy );
LEPT_DLL extern l_ok boxIntersectByLine ( BOX *box, l_int32 x, l_int32 y, l_float32 slope, l_int32 *px1, l_int32 *py1, l_int32 *px2, l_int32 *py2, l_int32 *pn );
LEPT_DLL extern BOX * boxClipToRectangle ( BOX *box, l_int32 wi, l_int32 hi );
LEPT_DLL extern l_ok boxClipToRectangleParams ( BOX *box, l_int32 w, l_int32 h, l_int32 *pxstart, l_int32 *pystart, l_int32 *pxend, l_int32 *pyend, l_int32 *pbw, l_int32 *pbh );
LEPT_DLL extern BOX * boxRelocateOneSide ( BOX *boxd, BOX *boxs, l_int32 loc, l_int32 sideflag );
LEPT_DLL extern BOXA * boxaAdjustSides ( BOXA *boxas, l_int32 delleft, l_int32 delright, l_int32 deltop, l_int32 delbot );
LEPT_DLL extern l_ok boxaAdjustBoxSides ( BOXA *boxa, l_int32 index, l_int32 delleft, l_int32 delright, l_int32 deltop, l_int32 delbot );
LEPT_DLL extern BOX * boxAdjustSides ( BOX *boxd, BOX *boxs, l_int32 delleft, l_int32 delright, l_int32 deltop, l_int32 delbot );
LEPT_DLL extern BOXA * boxaSetSide ( BOXA *boxad, BOXA *boxas, l_int32 side, l_int32 val, l_int32 thresh );
LEPT_DLL extern l_ok boxSetSide ( BOX *boxs, l_int32 side, l_int32 val, l_int32 thresh );
LEPT_DLL extern BOXA * boxaAdjustWidthToTarget ( BOXA *boxad, BOXA *boxas, l_int32 sides, l_int32 target, l_int32 thresh );
LEPT_DLL extern BOXA * boxaAdjustHeightToTarget ( BOXA *boxad, BOXA *boxas, l_int32 sides, l_int32 target, l_int32 thresh );
LEPT_DLL extern l_ok boxEqual ( BOX *box1, BOX *box2, l_int32 *psame );
LEPT_DLL extern l_ok boxaEqual ( BOXA *boxa1, BOXA *boxa2, l_int32 maxdist, NUMA **pnaindex, l_int32 *psame );
LEPT_DLL extern l_ok boxSimilar ( BOX *box1, BOX *box2, l_int32 leftdiff, l_int32 rightdiff, l_int32 topdiff, l_int32 botdiff, l_int32 *psimilar );
LEPT_DLL extern l_ok boxaSimilar ( BOXA *boxa1, BOXA *boxa2, l_int32 leftdiff, l_int32 rightdiff, l_int32 topdiff, l_int32 botdiff, l_int32 debug, l_int32 *psimilar, NUMA **pnasim );
LEPT_DLL extern l_ok boxaJoin ( BOXA *boxad, BOXA *boxas, l_int32 istart, l_int32 iend );
LEPT_DLL extern l_ok boxaaJoin ( BOXAA *baad, BOXAA *baas, l_int32 istart, l_int32 iend );
LEPT_DLL extern l_ok boxaSplitEvenOdd ( BOXA *boxa, l_int32 fillflag, BOXA **pboxae, BOXA **pboxao );
LEPT_DLL extern BOXA * boxaMergeEvenOdd ( BOXA *boxae, BOXA *boxao, l_int32 fillflag );
LEPT_DLL extern BOXA * boxaTransform ( BOXA *boxas, l_int32 shiftx, l_int32 shifty, l_float32 scalex, l_float32 scaley );
LEPT_DLL extern BOX * boxTransform ( BOX *box, l_int32 shiftx, l_int32 shifty, l_float32 scalex, l_float32 scaley );
LEPT_DLL extern BOXA * boxaTransformOrdered ( BOXA *boxas, l_int32 shiftx, l_int32 shifty, l_float32 scalex, l_float32 scaley, l_int32 xcen, l_int32 ycen, l_float32 angle, l_int32 order );
LEPT_DLL extern BOX * boxTransformOrdered ( BOX *boxs, l_int32 shiftx, l_int32 shifty, l_float32 scalex, l_float32 scaley, l_int32 xcen, l_int32 ycen, l_float32 angle, l_int32 order );
LEPT_DLL extern BOXA * boxaRotateOrth ( BOXA *boxas, l_int32 w, l_int32 h, l_int32 rotation );
LEPT_DLL extern BOX * boxRotateOrth ( BOX *box, l_int32 w, l_int32 h, l_int32 rotation );
LEPT_DLL extern BOXA * boxaShiftWithPta ( BOXA *boxas, PTA *pta, l_int32 dir );
LEPT_DLL extern BOXA * boxaSort ( BOXA *boxas, l_int32 sorttype, l_int32 sortorder, NUMA **pnaindex );
LEPT_DLL extern BOXA * boxaBinSort ( BOXA *boxas, l_int32 sorttype, l_int32 sortorder, NUMA **pnaindex );
LEPT_DLL extern BOXA * boxaSortByIndex ( BOXA *boxas, NUMA *naindex );
LEPT_DLL extern BOXAA * boxaSort2d ( BOXA *boxas, NUMAA **pnaad, l_int32 delta1, l_int32 delta2, l_int32 minh1 );
LEPT_DLL extern BOXAA * boxaSort2dByIndex ( BOXA *boxas, NUMAA *naa );
LEPT_DLL extern l_ok boxaExtractAsNuma ( BOXA *boxa, NUMA **pnal, NUMA **pnat, NUMA **pnar, NUMA **pnab, NUMA **pnaw, NUMA **pnah, l_int32 keepinvalid );
LEPT_DLL extern l_ok boxaExtractAsPta ( BOXA *boxa, PTA **pptal, PTA **pptat, PTA **pptar, PTA **pptab, PTA **pptaw, PTA **pptah, l_int32 keepinvalid );
LEPT_DLL extern PTA * boxaExtractCorners ( BOXA *boxa, l_int32 loc );
LEPT_DLL extern l_ok boxaGetRankVals ( BOXA *boxa, l_float32 fract, l_int32 *px, l_int32 *py, l_int32 *pr, l_int32 *pb, l_int32 *pw, l_int32 *ph );
LEPT_DLL extern l_ok boxaGetMedianVals ( BOXA *boxa, l_int32 *px, l_int32 *py, l_int32 *pr, l_int32 *pb, l_int32 *pw, l_int32 *ph );
LEPT_DLL extern l_ok boxaGetAverageSize ( BOXA *boxa, l_float32 *pw, l_float32 *ph );
LEPT_DLL extern l_ok boxaaGetExtent ( BOXAA *baa, l_int32 *pw, l_int32 *ph, BOX **pbox, BOXA **pboxa );
LEPT_DLL extern BOXA * boxaaFlattenToBoxa ( BOXAA *baa, NUMA **pnaindex, l_int32 copyflag );
LEPT_DLL extern BOXA * boxaaFlattenAligned ( BOXAA *baa, l_int32 num, BOX *fillerbox, l_int32 copyflag );
LEPT_DLL extern BOXAA * boxaEncapsulateAligned ( BOXA *boxa, l_int32 num, l_int32 copyflag );
LEPT_DLL extern BOXAA * boxaaTranspose ( BOXAA *baas );
LEPT_DLL extern l_ok boxaaAlignBox ( BOXAA *baa, BOX *box, l_int32 delta, l_int32 *pindex );
LEPT_DLL extern PIX * pixMaskConnComp ( PIX *pixs, l_int32 connectivity, BOXA **pboxa );
LEPT_DLL extern PIX * pixMaskBoxa ( PIX *pixd, PIX *pixs, BOXA *boxa, l_int32 op );
LEPT_DLL extern PIX * pixPaintBoxa ( PIX *pixs, BOXA *boxa, l_uint32 val );
LEPT_DLL extern PIX * pixSetBlackOrWhiteBoxa ( PIX *pixs, BOXA *boxa, l_int32 op );
LEPT_DLL extern PIX * pixPaintBoxaRandom ( PIX *pixs, BOXA *boxa );
LEPT_DLL extern PIX * pixBlendBoxaRandom ( PIX *pixs, BOXA *boxa, l_float32 fract );
LEPT_DLL extern PIX * pixDrawBoxa ( PIX *pixs, BOXA *boxa, l_int32 width, l_uint32 val );
LEPT_DLL extern PIX * pixDrawBoxaRandom ( PIX *pixs, BOXA *boxa, l_int32 width );
LEPT_DLL extern PIX * boxaaDisplay ( PIX *pixs, BOXAA *baa, l_int32 linewba, l_int32 linewb, l_uint32 colorba, l_uint32 colorb, l_int32 w, l_int32 h );
LEPT_DLL extern PIXA * pixaDisplayBoxaa ( PIXA *pixas, BOXAA *baa, l_int32 colorflag, l_int32 width );
LEPT_DLL extern BOXA * pixSplitIntoBoxa ( PIX *pixs, l_int32 minsum, l_int32 skipdist, l_int32 delta, l_int32 maxbg, l_int32 maxcomps, l_int32 remainder );
LEPT_DLL extern BOXA * pixSplitComponentIntoBoxa ( PIX *pix, BOX *box, l_int32 minsum, l_int32 skipdist, l_int32 delta, l_int32 maxbg, l_int32 maxcomps, l_int32 remainder );
LEPT_DLL extern BOXA * makeMosaicStrips ( l_int32 w, l_int32 h, l_int32 direction, l_int32 size );
LEPT_DLL extern l_ok boxaCompareRegions ( BOXA *boxa1, BOXA *boxa2, l_int32 areathresh, l_int32 *pnsame, l_float32 *pdiffarea, l_float32 *pdiffxor, PIX **ppixdb );
LEPT_DLL extern BOX * pixSelectLargeULComp ( PIX *pixs, l_float32 areaslop, l_int32 yslop, l_int32 connectivity );
LEPT_DLL extern BOX * boxaSelectLargeULBox ( BOXA *boxas, l_float32 areaslop, l_int32 yslop );
LEPT_DLL extern BOXA * boxaSelectRange ( BOXA *boxas, l_int32 first, l_int32 last, l_int32 copyflag );
LEPT_DLL extern BOXAA * boxaaSelectRange ( BOXAA *baas, l_int32 first, l_int32 last, l_int32 copyflag );
LEPT_DLL extern BOXA * boxaSelectBySize ( BOXA *boxas, l_int32 width, l_int32 height, l_int32 type, l_int32 relation, l_int32 *pchanged );
LEPT_DLL extern NUMA * boxaMakeSizeIndicator ( BOXA *boxa, l_int32 width, l_int32 height, l_int32 type, l_int32 relation );
LEPT_DLL extern BOXA * boxaSelectByArea ( BOXA *boxas, l_int32 area, l_int32 relation, l_int32 *pchanged );
LEPT_DLL extern NUMA * boxaMakeAreaIndicator ( BOXA *boxa, l_int32 area, l_int32 relation );
LEPT_DLL extern BOXA * boxaSelectByWHRatio ( BOXA *boxas, l_float32 ratio, l_int32 relation, l_int32 *pchanged );
LEPT_DLL extern NUMA * boxaMakeWHRatioIndicator ( BOXA *boxa, l_float32 ratio, l_int32 relation );
LEPT_DLL extern BOXA * boxaSelectWithIndicator ( BOXA *boxas, NUMA *na, l_int32 *pchanged );
LEPT_DLL extern BOXA * boxaPermutePseudorandom ( BOXA *boxas );
LEPT_DLL extern BOXA * boxaPermuteRandom ( BOXA *boxad, BOXA *boxas );
LEPT_DLL extern l_ok boxaSwapBoxes ( BOXA *boxa, l_int32 i, l_int32 j );
LEPT_DLL extern PTA * boxaConvertToPta ( BOXA *boxa, l_int32 ncorners );
LEPT_DLL extern BOXA * ptaConvertToBoxa ( PTA *pta, l_int32 ncorners );
LEPT_DLL extern PTA * boxConvertToPta ( BOX *box, l_int32 ncorners );
LEPT_DLL extern BOX * ptaConvertToBox ( PTA *pta );
LEPT_DLL extern l_ok boxaGetExtent ( BOXA *boxa, l_int32 *pw, l_int32 *ph, BOX **pbox );
LEPT_DLL extern l_ok boxaGetCoverage ( BOXA *boxa, l_int32 wc, l_int32 hc, l_int32 exactflag, l_float32 *pfract );
LEPT_DLL extern l_ok boxaaSizeRange ( BOXAA *baa, l_int32 *pminw, l_int32 *pminh, l_int32 *pmaxw, l_int32 *pmaxh );
LEPT_DLL extern l_ok boxaSizeRange ( BOXA *boxa, l_int32 *pminw, l_int32 *pminh, l_int32 *pmaxw, l_int32 *pmaxh );
LEPT_DLL extern l_ok boxaLocationRange ( BOXA *boxa, l_int32 *pminx, l_int32 *pminy, l_int32 *pmaxx, l_int32 *pmaxy );
LEPT_DLL extern l_ok boxaGetSizes ( BOXA *boxa, NUMA **pnaw, NUMA **pnah );
LEPT_DLL extern l_ok boxaGetArea ( BOXA *boxa, l_int32 *parea );
LEPT_DLL extern PIX * boxaDisplayTiled ( BOXA *boxas, PIXA *pixa, l_int32 first, l_int32 last, l_int32 maxwidth, l_int32 linewidth, l_float32 scalefactor, l_int32 background, l_int32 spacing, l_int32 border );
LEPT_DLL extern BOXA * boxaSmoothSequenceMedian ( BOXA *boxas, l_int32 halfwin, l_int32 subflag, l_int32 maxdiff, l_int32 extrapixels, l_int32 debug );
LEPT_DLL extern BOXA * boxaWindowedMedian ( BOXA *boxas, l_int32 halfwin, l_int32 debug );
LEPT_DLL extern BOXA * boxaModifyWithBoxa ( BOXA *boxas, BOXA *boxam, l_int32 subflag, l_int32 maxdiff, l_int32 extrapixels );
LEPT_DLL extern BOXA * boxaConstrainSize ( BOXA *boxas, l_int32 width, l_int32 widthflag, l_int32 height, l_int32 heightflag );
LEPT_DLL extern BOXA * boxaReconcileEvenOddHeight ( BOXA *boxas, l_int32 sides, l_int32 delh, l_int32 op, l_float32 factor, l_int32 start );
LEPT_DLL extern BOXA * boxaReconcilePairWidth ( BOXA *boxas, l_int32 delw, l_int32 op, l_float32 factor, NUMA *na );
LEPT_DLL extern l_ok boxaSizeConsistency1 ( BOXA *boxas, l_int32 type, l_float32 threshp, l_float32 threshm, l_float32 *pfvarp, l_float32 *pfvarm, l_int32 *psame );
LEPT_DLL extern l_ok boxaSizeConsistency2 ( BOXA *boxas, l_float32 *pfdevw, l_float32 *pfdevh, l_int32 debug );
LEPT_DLL extern BOXA * boxaReconcileAllByMedian ( BOXA *boxas, l_int32 select1, l_int32 select2, l_int32 thresh, l_int32 extra, PIXA *pixadb );
LEPT_DLL extern BOXA * boxaReconcileSidesByMedian ( BOXA *boxas, l_int32 select, l_int32 thresh, l_int32 extra, PIXA *pixadb );
LEPT_DLL extern BOXA * boxaReconcileSizeByMedian ( BOXA *boxas, l_int32 type, l_float32 dfract, l_float32 sfract, l_float32 factor, NUMA **pnadelw, NUMA **pnadelh, l_float32 *pratiowh );
LEPT_DLL extern l_ok boxaPlotSides ( BOXA *boxa, const char *plotname, NUMA **pnal, NUMA **pnat, NUMA **pnar, NUMA **pnab, PIX **ppixd );
LEPT_DLL extern l_ok boxaPlotSizes ( BOXA *boxa, const char *plotname, NUMA **pnaw, NUMA **pnah, PIX **ppixd );
LEPT_DLL extern BOXA * boxaFillSequence ( BOXA *boxas, l_int32 useflag, l_int32 debug );
LEPT_DLL extern l_ok boxaSizeVariation ( BOXA *boxa, l_int32 type, l_float32 *pdel_evenodd, l_float32 *prms_even, l_float32 *prms_odd, l_float32 *prms_all );
LEPT_DLL extern l_ok boxaMedianDimensions ( BOXA *boxas, l_int32 *pmedw, l_int32 *pmedh, l_int32 *pmedwe, l_int32 *pmedwo, l_int32 *pmedhe, l_int32 *pmedho, NUMA **pnadelw, NUMA **pnadelh );
LEPT_DLL extern L_BYTEA * l_byteaCreate ( size_t nbytes );
LEPT_DLL extern L_BYTEA * l_byteaInitFromMem ( const l_uint8 *data, size_t size );
LEPT_DLL extern L_BYTEA * l_byteaInitFromFile ( const char *fname );
LEPT_DLL extern L_BYTEA * l_byteaInitFromStream ( FILE *fp );
LEPT_DLL extern L_BYTEA * l_byteaCopy ( L_BYTEA *bas, l_int32 copyflag );
LEPT_DLL extern void l_byteaDestroy ( L_BYTEA **pba );
LEPT_DLL extern size_t l_byteaGetSize ( L_BYTEA *ba );
LEPT_DLL extern l_uint8 * l_byteaGetData ( L_BYTEA *ba, size_t *psize );
LEPT_DLL extern l_uint8 * l_byteaCopyData ( L_BYTEA *ba, size_t *psize );
LEPT_DLL extern l_ok l_byteaAppendData ( L_BYTEA *ba, const l_uint8 *newdata, size_t newbytes );
LEPT_DLL extern l_ok l_byteaAppendString ( L_BYTEA *ba, const char *str );
LEPT_DLL extern l_ok l_byteaJoin ( L_BYTEA *ba1, L_BYTEA **pba2 );
LEPT_DLL extern l_ok l_byteaSplit ( L_BYTEA *ba1, size_t splitloc, L_BYTEA **pba2 );
LEPT_DLL extern l_ok l_byteaFindEachSequence ( L_BYTEA *ba, const l_uint8 *sequence, size_t seqlen, L_DNA **pda );
LEPT_DLL extern l_ok l_byteaWrite ( const char *fname, L_BYTEA *ba, size_t startloc, size_t nbytes );
LEPT_DLL extern l_ok l_byteaWriteStream ( FILE *fp, L_BYTEA *ba, size_t startloc, size_t nbytes );
LEPT_DLL extern CCBORDA * ccbaCreate ( PIX *pixs, l_int32 n );
LEPT_DLL extern void ccbaDestroy ( CCBORDA **pccba );
LEPT_DLL extern CCBORD * ccbCreate ( PIX *pixs );
LEPT_DLL extern void ccbDestroy ( CCBORD **pccb );
LEPT_DLL extern l_ok ccbaAddCcb ( CCBORDA *ccba, CCBORD *ccb );
LEPT_DLL extern l_int32 ccbaGetCount ( CCBORDA *ccba );
LEPT_DLL extern CCBORD * ccbaGetCcb ( CCBORDA *ccba, l_int32 index );
LEPT_DLL extern CCBORDA * pixGetAllCCBorders ( PIX *pixs );
LEPT_DLL extern PTAA * pixGetOuterBordersPtaa ( PIX *pixs );
LEPT_DLL extern l_ok pixGetOuterBorder ( CCBORD *ccb, PIX *pixs, BOX *box );
LEPT_DLL extern l_ok ccbaGenerateGlobalLocs ( CCBORDA *ccba );
LEPT_DLL extern l_ok ccbaGenerateStepChains ( CCBORDA *ccba );
LEPT_DLL extern l_ok ccbaStepChainsToPixCoords ( CCBORDA *ccba, l_int32 coordtype );
LEPT_DLL extern l_ok ccbaGenerateSPGlobalLocs ( CCBORDA *ccba, l_int32 ptsflag );
LEPT_DLL extern l_ok ccbaGenerateSinglePath ( CCBORDA *ccba );
LEPT_DLL extern PTA * getCutPathForHole ( PIX *pix, PTA *pta, BOX *boxinner, l_int32 *pdir, l_int32 *plen );
LEPT_DLL extern PIX * ccbaDisplayBorder ( CCBORDA *ccba );
LEPT_DLL extern PIX * ccbaDisplaySPBorder ( CCBORDA *ccba );
LEPT_DLL extern PIX * ccbaDisplayImage1 ( CCBORDA *ccba );
LEPT_DLL extern PIX * ccbaDisplayImage2 ( CCBORDA *ccba );
LEPT_DLL extern l_ok ccbaWrite ( const char *filename, CCBORDA *ccba );
LEPT_DLL extern l_ok ccbaWriteStream ( FILE *fp, CCBORDA *ccba );
LEPT_DLL extern CCBORDA * ccbaRead ( const char *filename );
LEPT_DLL extern CCBORDA * ccbaReadStream ( FILE *fp );
LEPT_DLL extern l_ok ccbaWriteSVG ( const char *filename, CCBORDA *ccba );
LEPT_DLL extern char * ccbaWriteSVGString ( const char *filename, CCBORDA *ccba );
LEPT_DLL extern PIXA * pixaThinConnected ( PIXA *pixas, l_int32 type, l_int32 connectivity, l_int32 maxiters );
LEPT_DLL extern PIX * pixThinConnected ( PIX *pixs, l_int32 type, l_int32 connectivity, l_int32 maxiters );
LEPT_DLL extern PIX * pixThinConnectedBySet ( PIX *pixs, l_int32 type, SELA *sela, l_int32 maxiters );
LEPT_DLL extern SELA * selaMakeThinSets ( l_int32 index, l_int32 debug );
LEPT_DLL extern l_ok pixFindCheckerboardCorners ( PIX *pixs, l_int32 size, l_int32 dilation, l_int32 nsels, PIX **ppix_corners, PTA **ppta_corners, PIXA *pixadb );
LEPT_DLL extern SELA * makeCheckerboardCornerSela ( l_int32 size, l_int32 dilation, l_int32 nsels, PIXA *pixadb );
LEPT_DLL extern l_ok jbCorrelation ( const char *dirin, l_float32 thresh, l_float32 weight, l_int32 components, const char *rootname, l_int32 firstpage, l_int32 npages, l_int32 renderflag );
LEPT_DLL extern l_ok jbRankHaus ( const char *dirin, l_int32 size, l_float32 rank, l_int32 components, const char *rootname, l_int32 firstpage, l_int32 npages, l_int32 renderflag );
LEPT_DLL extern JBCLASSER * jbWordsInTextlines ( const char *dirin, l_int32 reduction, l_int32 maxwidth, l_int32 maxheight, l_float32 thresh, l_float32 weight, NUMA **pnatl, l_int32 firstpage, l_int32 npages );
LEPT_DLL extern l_ok pixGetWordsInTextlines ( PIX *pixs, l_int32 minwidth, l_int32 minheight, l_int32 maxwidth, l_int32 maxheight, BOXA **pboxad, PIXA **ppixad, NUMA **pnai );
LEPT_DLL extern l_ok pixGetWordBoxesInTextlines ( PIX *pixs, l_int32 minwidth, l_int32 minheight, l_int32 maxwidth, l_int32 maxheight, BOXA **pboxad, NUMA **pnai );
LEPT_DLL extern l_ok pixFindWordAndCharacterBoxes ( PIX *pixs, BOX *boxs, l_int32 thresh, BOXA **pboxaw, BOXAA **pboxaac, const char *debugdir );
LEPT_DLL extern NUMAA * boxaExtractSortedPattern ( BOXA *boxa, NUMA *na );
LEPT_DLL extern l_ok numaaCompareImagesByBoxes ( NUMAA *naa1, NUMAA *naa2, l_int32 nperline, l_int32 nreq, l_int32 maxshiftx, l_int32 maxshifty, l_int32 delx, l_int32 dely, l_int32 *psame, l_int32 debugflag );
LEPT_DLL extern l_ok pixColorContent ( PIX *pixs, l_int32 rref, l_int32 gref, l_int32 bref, l_int32 mingray, PIX **ppixr, PIX **ppixg, PIX **ppixb );
LEPT_DLL extern PIX * pixColorMagnitude ( PIX *pixs, l_int32 rref, l_int32 gref, l_int32 bref, l_int32 type );
LEPT_DLL extern PIX * pixMaskOverColorPixels ( PIX *pixs, l_int32 threshdiff, l_int32 mindist );
LEPT_DLL extern PIX * pixMaskOverGrayPixels ( PIX *pixs, l_int32 maxlimit, l_int32 satlimit );
LEPT_DLL extern PIX * pixMaskOverColorRange ( PIX *pixs, l_int32 rmin, l_int32 rmax, l_int32 gmin, l_int32 gmax, l_int32 bmin, l_int32 bmax );
LEPT_DLL extern l_ok pixColorFraction ( PIX *pixs, l_int32 darkthresh, l_int32 lightthresh, l_int32 diffthresh, l_int32 factor, l_float32 *ppixfract, l_float32 *pcolorfract );
LEPT_DLL extern l_ok pixFindColorRegions ( PIX *pixs, PIX *pixm, l_int32 factor, l_int32 lightthresh, l_int32 darkthresh, l_int32 mindiff, l_int32 colordiff, l_float32 edgefract, l_float32 *pcolorfract, PIX **pcolormask1, PIX **pcolormask2, PIXA *pixadb );
LEPT_DLL extern l_ok pixNumSignificantGrayColors ( PIX *pixs, l_int32 darkthresh, l_int32 lightthresh, l_float32 minfract, l_int32 factor, l_int32 *pncolors );
LEPT_DLL extern l_ok pixColorsForQuantization ( PIX *pixs, l_int32 thresh, l_int32 *pncolors, l_int32 *piscolor, l_int32 debug );
LEPT_DLL extern l_ok pixNumColors ( PIX *pixs, l_int32 factor, l_int32 *pncolors );
LEPT_DLL extern PIX * pixConvertRGBToCmap ( PIX *pixs );
LEPT_DLL extern l_ok pixGetMostPopulatedColors ( PIX *pixs, l_int32 sigbits, l_int32 factor, l_int32 ncolors, l_uint32 **parray, PIXCMAP **pcmap );
LEPT_DLL extern PIX * pixSimpleColorQuantize ( PIX *pixs, l_int32 sigbits, l_int32 factor, l_int32 ncolors );
LEPT_DLL extern NUMA * pixGetRGBHistogram ( PIX *pixs, l_int32 sigbits, l_int32 factor );
LEPT_DLL extern l_ok makeRGBIndexTables ( l_uint32 **prtab, l_uint32 **pgtab, l_uint32 **pbtab, l_int32 sigbits );
LEPT_DLL extern l_ok getRGBFromIndex ( l_uint32 index, l_int32 sigbits, l_int32 *prval, l_int32 *pgval, l_int32 *pbval );
LEPT_DLL extern l_ok pixHasHighlightRed ( PIX *pixs, l_int32 factor, l_float32 fract, l_float32 fthresh, l_int32 *phasred, l_float32 *pratio, PIX **ppixdb );
LEPT_DLL extern PIX * pixColorGrayRegions ( PIX *pixs, BOXA *boxa, l_int32 type, l_int32 thresh, l_int32 rval, l_int32 gval, l_int32 bval );
LEPT_DLL extern l_ok pixColorGray ( PIX *pixs, BOX *box, l_int32 type, l_int32 thresh, l_int32 rval, l_int32 gval, l_int32 bval );
LEPT_DLL extern PIX * pixColorGrayMasked ( PIX *pixs, PIX *pixm, l_int32 type, l_int32 thresh, l_int32 rval, l_int32 gval, l_int32 bval );
LEPT_DLL extern PIX * pixSnapColor ( PIX *pixd, PIX *pixs, l_uint32 srcval, l_uint32 dstval, l_int32 diff );
LEPT_DLL extern PIX * pixSnapColorCmap ( PIX *pixd, PIX *pixs, l_uint32 srcval, l_uint32 dstval, l_int32 diff );
LEPT_DLL extern PIX * pixLinearMapToTargetColor ( PIX *pixd, PIX *pixs, l_uint32 srcval, l_uint32 dstval );
LEPT_DLL extern l_ok pixelLinearMapToTargetColor ( l_uint32 scolor, l_uint32 srcmap, l_uint32 dstmap, l_uint32 *pdcolor );
LEPT_DLL extern PIX * pixShiftByComponent ( PIX *pixd, PIX *pixs, l_uint32 srcval, l_uint32 dstval );
LEPT_DLL extern l_ok pixelShiftByComponent ( l_int32 rval, l_int32 gval, l_int32 bval, l_uint32 srcval, l_uint32 dstval, l_uint32 *ppixel );
LEPT_DLL extern l_ok pixelFractionalShift ( l_int32 rval, l_int32 gval, l_int32 bval, l_float32 fraction, l_uint32 *ppixel );
LEPT_DLL extern PIXCMAP * pixcmapCreate ( l_int32 depth );
LEPT_DLL extern PIXCMAP * pixcmapCreateRandom ( l_int32 depth, l_int32 hasblack, l_int32 haswhite );
LEPT_DLL extern PIXCMAP * pixcmapCreateLinear ( l_int32 d, l_int32 nlevels );
LEPT_DLL extern PIXCMAP * pixcmapCopy ( const PIXCMAP *cmaps );
LEPT_DLL extern void pixcmapDestroy ( PIXCMAP **pcmap );
LEPT_DLL extern l_ok pixcmapIsValid ( const PIXCMAP *cmap, l_int32 *pvalid );
LEPT_DLL extern l_ok pixcmapAddColor ( PIXCMAP *cmap, l_int32 rval, l_int32 gval, l_int32 bval );
LEPT_DLL extern l_ok pixcmapAddRGBA ( PIXCMAP *cmap, l_int32 rval, l_int32 gval, l_int32 bval, l_int32 aval );
LEPT_DLL extern l_ok pixcmapAddNewColor ( PIXCMAP *cmap, l_int32 rval, l_int32 gval, l_int32 bval, l_int32 *pindex );
LEPT_DLL extern l_ok pixcmapAddNearestColor ( PIXCMAP *cmap, l_int32 rval, l_int32 gval, l_int32 bval, l_int32 *pindex );
LEPT_DLL extern l_ok pixcmapUsableColor ( PIXCMAP *cmap, l_int32 rval, l_int32 gval, l_int32 bval, l_int32 *pusable );
LEPT_DLL extern l_ok pixcmapAddBlackOrWhite ( PIXCMAP *cmap, l_int32 color, l_int32 *pindex );
LEPT_DLL extern l_ok pixcmapSetBlackAndWhite ( PIXCMAP *cmap, l_int32 setblack, l_int32 setwhite );
LEPT_DLL extern l_int32 pixcmapGetCount ( const PIXCMAP *cmap );
LEPT_DLL extern l_int32 pixcmapGetFreeCount ( PIXCMAP *cmap );
LEPT_DLL extern l_int32 pixcmapGetDepth ( PIXCMAP *cmap );
LEPT_DLL extern l_ok pixcmapGetMinDepth ( PIXCMAP *cmap, l_int32 *pmindepth );
LEPT_DLL extern l_ok pixcmapClear ( PIXCMAP *cmap );
LEPT_DLL extern l_ok pixcmapGetColor ( PIXCMAP *cmap, l_int32 index, l_int32 *prval, l_int32 *pgval, l_int32 *pbval );
LEPT_DLL extern l_ok pixcmapGetColor32 ( PIXCMAP *cmap, l_int32 index, l_uint32 *pval32 );
LEPT_DLL extern l_ok pixcmapGetRGBA ( PIXCMAP *cmap, l_int32 index, l_int32 *prval, l_int32 *pgval, l_int32 *pbval, l_int32 *paval );
LEPT_DLL extern l_ok pixcmapGetRGBA32 ( PIXCMAP *cmap, l_int32 index, l_uint32 *pval32 );
LEPT_DLL extern l_ok pixcmapResetColor ( PIXCMAP *cmap, l_int32 index, l_int32 rval, l_int32 gval, l_int32 bval );
LEPT_DLL extern l_ok pixcmapSetAlpha ( PIXCMAP *cmap, l_int32 index, l_int32 aval );
LEPT_DLL extern l_int32 pixcmapGetIndex ( PIXCMAP *cmap, l_int32 rval, l_int32 gval, l_int32 bval, l_int32 *pindex );
LEPT_DLL extern l_ok pixcmapHasColor ( PIXCMAP *cmap, l_int32 *pcolor );
LEPT_DLL extern l_ok pixcmapIsOpaque ( PIXCMAP *cmap, l_int32 *popaque );
LEPT_DLL extern l_ok pixcmapIsBlackAndWhite ( PIXCMAP *cmap, l_int32 *pblackwhite );
LEPT_DLL extern l_ok pixcmapCountGrayColors ( PIXCMAP *cmap, l_int32 *pngray );
LEPT_DLL extern l_ok pixcmapGetRankIntensity ( PIXCMAP *cmap, l_float32 rankval, l_int32 *pindex );
LEPT_DLL extern l_ok pixcmapGetNearestIndex ( PIXCMAP *cmap, l_int32 rval, l_int32 gval, l_int32 bval, l_int32 *pindex );
LEPT_DLL extern l_ok pixcmapGetNearestGrayIndex ( PIXCMAP *cmap, l_int32 val, l_int32 *pindex );
LEPT_DLL extern l_ok pixcmapGetDistanceToColor ( PIXCMAP *cmap, l_int32 index, l_int32 rval, l_int32 gval, l_int32 bval, l_int32 *pdist );
LEPT_DLL extern l_ok pixcmapGetRangeValues ( PIXCMAP *cmap, l_int32 select, l_int32 *pminval, l_int32 *pmaxval, l_int32 *pminindex, l_int32 *pmaxindex );
LEPT_DLL extern PIXCMAP * pixcmapGrayToColor ( l_uint32 color );
LEPT_DLL extern PIXCMAP * pixcmapColorToGray ( PIXCMAP *cmaps, l_float32 rwt, l_float32 gwt, l_float32 bwt );
LEPT_DLL extern PIXCMAP * pixcmapConvertTo4 ( PIXCMAP *cmaps );
LEPT_DLL extern PIXCMAP * pixcmapConvertTo8 ( PIXCMAP *cmaps );
LEPT_DLL extern PIXCMAP * pixcmapRead ( const char *filename );
LEPT_DLL extern PIXCMAP * pixcmapReadStream ( FILE *fp );
LEPT_DLL extern PIXCMAP * pixcmapReadMem ( const l_uint8 *data, size_t size );
LEPT_DLL extern l_ok pixcmapWrite ( const char *filename, const PIXCMAP *cmap );
LEPT_DLL extern l_ok pixcmapWriteStream ( FILE *fp, const PIXCMAP *cmap );
LEPT_DLL extern l_ok pixcmapWriteMem ( l_uint8 **pdata, size_t *psize, const PIXCMAP *cmap );
LEPT_DLL extern l_ok pixcmapToArrays ( const PIXCMAP *cmap, l_int32 **prmap, l_int32 **pgmap, l_int32 **pbmap, l_int32 **pamap );
LEPT_DLL extern l_ok pixcmapToRGBTable ( PIXCMAP *cmap, l_uint32 **ptab, l_int32 *pncolors );
LEPT_DLL extern l_ok pixcmapSerializeToMemory ( PIXCMAP *cmap, l_int32 cpc, l_int32 *pncolors, l_uint8 **pdata );
LEPT_DLL extern PIXCMAP * pixcmapDeserializeFromMemory ( l_uint8 *data, l_int32 cpc, l_int32 ncolors );
LEPT_DLL extern char * pixcmapConvertToHex ( l_uint8 *data, l_int32 ncolors );
LEPT_DLL extern l_ok pixcmapGammaTRC ( PIXCMAP *cmap, l_float32 gamma, l_int32 minval, l_int32 maxval );
LEPT_DLL extern l_ok pixcmapContrastTRC ( PIXCMAP *cmap, l_float32 factor );
LEPT_DLL extern l_ok pixcmapShiftIntensity ( PIXCMAP *cmap, l_float32 fraction );
LEPT_DLL extern l_ok pixcmapShiftByComponent ( PIXCMAP *cmap, l_uint32 srcval, l_uint32 dstval );
LEPT_DLL extern PIX * pixColorMorph ( PIX *pixs, l_int32 type, l_int32 hsize, l_int32 vsize );
LEPT_DLL extern PIX * pixOctreeColorQuant ( PIX *pixs, l_int32 colors, l_int32 ditherflag );
LEPT_DLL extern PIX * pixOctreeColorQuantGeneral ( PIX *pixs, l_int32 colors, l_int32 ditherflag, l_float32 validthresh, l_float32 colorthresh );
LEPT_DLL extern l_ok makeRGBToIndexTables ( l_int32 cqlevels, l_uint32 **prtab, l_uint32 **pgtab, l_uint32 **pbtab );
LEPT_DLL extern void getOctcubeIndexFromRGB ( l_int32 rval, l_int32 gval, l_int32 bval, l_uint32 *rtab, l_uint32 *gtab, l_uint32 *btab, l_uint32 *pindex );
LEPT_DLL extern PIX * pixOctreeQuantByPopulation ( PIX *pixs, l_int32 level, l_int32 ditherflag );
LEPT_DLL extern PIX * pixOctreeQuantNumColors ( PIX *pixs, l_int32 maxcolors, l_int32 subsample );
LEPT_DLL extern PIX * pixOctcubeQuantMixedWithGray ( PIX *pixs, l_int32 depth, l_int32 graylevels, l_int32 delta );
LEPT_DLL extern PIX * pixFixedOctcubeQuant256 ( PIX *pixs, l_int32 ditherflag );
LEPT_DLL extern PIX * pixFewColorsOctcubeQuant1 ( PIX *pixs, l_int32 level );
LEPT_DLL extern PIX * pixFewColorsOctcubeQuant2 ( PIX *pixs, l_int32 level, NUMA *na, l_int32 ncolors, l_int32 *pnerrors );
LEPT_DLL extern PIX * pixFewColorsOctcubeQuantMixed ( PIX *pixs, l_int32 level, l_int32 darkthresh, l_int32 lightthresh, l_int32 diffthresh, l_float32 minfract, l_int32 maxspan );
LEPT_DLL extern PIX * pixFixedOctcubeQuantGenRGB ( PIX *pixs, l_int32 level );
LEPT_DLL extern PIX * pixQuantFromCmap ( PIX *pixs, PIXCMAP *cmap, l_int32 mindepth, l_int32 level, l_int32 metric );
LEPT_DLL extern PIX * pixOctcubeQuantFromCmap ( PIX *pixs, PIXCMAP *cmap, l_int32 mindepth, l_int32 level, l_int32 metric );
LEPT_DLL extern NUMA * pixOctcubeHistogram ( PIX *pixs, l_int32 level, l_int32 *pncolors );
LEPT_DLL extern l_int32 * pixcmapToOctcubeLUT ( PIXCMAP *cmap, l_int32 level, l_int32 metric );
LEPT_DLL extern l_ok pixRemoveUnusedColors ( PIX *pixs );
LEPT_DLL extern l_ok pixNumberOccupiedOctcubes ( PIX *pix, l_int32 level, l_int32 mincount, l_float32 minfract, l_int32 *pncolors );
LEPT_DLL extern PIX * pixMedianCutQuant ( PIX *pixs, l_int32 ditherflag );
LEPT_DLL extern PIX * pixMedianCutQuantGeneral ( PIX *pixs, l_int32 ditherflag, l_int32 outdepth, l_int32 maxcolors, l_int32 sigbits, l_int32 maxsub, l_int32 checkbw );
LEPT_DLL extern PIX * pixMedianCutQuantMixed ( PIX *pixs, l_int32 ncolor, l_int32 ngray, l_int32 darkthresh, l_int32 lightthresh, l_int32 diffthresh );
LEPT_DLL extern PIX * pixFewColorsMedianCutQuantMixed ( PIX *pixs, l_int32 ncolor, l_int32 ngray, l_int32 maxncolors, l_int32 darkthresh, l_int32 lightthresh, l_int32 diffthresh );
LEPT_DLL extern l_int32 * pixMedianCutHisto ( PIX *pixs, l_int32 sigbits, l_int32 subsample );
LEPT_DLL extern PIX * pixColorSegment ( PIX *pixs, l_int32 maxdist, l_int32 maxcolors, l_int32 selsize, l_int32 finalcolors, l_int32 debugflag );
LEPT_DLL extern PIX * pixColorSegmentCluster ( PIX *pixs, l_int32 maxdist, l_int32 maxcolors, l_int32 debugflag );
LEPT_DLL extern l_ok pixAssignToNearestColor ( PIX *pixd, PIX *pixs, PIX *pixm, l_int32 level, l_int32 *countarray );
LEPT_DLL extern l_ok pixColorSegmentClean ( PIX *pixs, l_int32 selsize, l_int32 *countarray );
LEPT_DLL extern l_ok pixColorSegmentRemoveColors ( PIX *pixd, PIX *pixs, l_int32 finalcolors );
LEPT_DLL extern PIX * pixConvertRGBToHSV ( PIX *pixd, PIX *pixs );
LEPT_DLL extern PIX * pixConvertHSVToRGB ( PIX *pixd, PIX *pixs );
LEPT_DLL extern l_ok convertRGBToHSV ( l_int32 rval, l_int32 gval, l_int32 bval, l_int32 *phval, l_int32 *psval, l_int32 *pvval );
LEPT_DLL extern l_ok convertHSVToRGB ( l_int32 hval, l_int32 sval, l_int32 vval, l_int32 *prval, l_int32 *pgval, l_int32 *pbval );
LEPT_DLL extern l_ok pixcmapConvertRGBToHSV ( PIXCMAP *cmap );
LEPT_DLL extern l_ok pixcmapConvertHSVToRGB ( PIXCMAP *cmap );
LEPT_DLL extern PIX * pixConvertRGBToHue ( PIX *pixs );
LEPT_DLL extern PIX * pixConvertRGBToSaturation ( PIX *pixs );
LEPT_DLL extern PIX * pixConvertRGBToValue ( PIX *pixs );
LEPT_DLL extern PIX * pixMakeRangeMaskHS ( PIX *pixs, l_int32 huecenter, l_int32 huehw, l_int32 satcenter, l_int32 sathw, l_int32 regionflag );
LEPT_DLL extern PIX * pixMakeRangeMaskHV ( PIX *pixs, l_int32 huecenter, l_int32 huehw, l_int32 valcenter, l_int32 valhw, l_int32 regionflag );
LEPT_DLL extern PIX * pixMakeRangeMaskSV ( PIX *pixs, l_int32 satcenter, l_int32 sathw, l_int32 valcenter, l_int32 valhw, l_int32 regionflag );
LEPT_DLL extern PIX * pixMakeHistoHS ( PIX *pixs, l_int32 factor, NUMA **pnahue, NUMA **pnasat );
LEPT_DLL extern PIX * pixMakeHistoHV ( PIX *pixs, l_int32 factor, NUMA **pnahue, NUMA **pnaval );
LEPT_DLL extern PIX * pixMakeHistoSV ( PIX *pixs, l_int32 factor, NUMA **pnasat, NUMA **pnaval );
LEPT_DLL extern l_ok pixFindHistoPeaksHSV ( PIX *pixs, l_int32 type, l_int32 width, l_int32 height, l_int32 npeaks, l_float32 erasefactor, PTA **ppta, NUMA **pnatot, PIXA **ppixa );
LEPT_DLL extern PIX * displayHSVColorRange ( l_int32 hval, l_int32 sval, l_int32 vval, l_int32 huehw, l_int32 sathw, l_int32 nsamp, l_int32 factor );
LEPT_DLL extern PIX * pixConvertRGBToYUV ( PIX *pixd, PIX *pixs );
LEPT_DLL extern PIX * pixConvertYUVToRGB ( PIX *pixd, PIX *pixs );
LEPT_DLL extern l_ok convertRGBToYUV ( l_int32 rval, l_int32 gval, l_int32 bval, l_int32 *pyval, l_int32 *puval, l_int32 *pvval );
LEPT_DLL extern l_ok convertYUVToRGB ( l_int32 yval, l_int32 uval, l_int32 vval, l_int32 *prval, l_int32 *pgval, l_int32 *pbval );
LEPT_DLL extern l_ok pixcmapConvertRGBToYUV ( PIXCMAP *cmap );
LEPT_DLL extern l_ok pixcmapConvertYUVToRGB ( PIXCMAP *cmap );
LEPT_DLL extern FPIXA * pixConvertRGBToXYZ ( PIX *pixs );
LEPT_DLL extern PIX * fpixaConvertXYZToRGB ( FPIXA *fpixa );
LEPT_DLL extern l_ok convertRGBToXYZ ( l_int32 rval, l_int32 gval, l_int32 bval, l_float32 *pfxval, l_float32 *pfyval, l_float32 *pfzval );
LEPT_DLL extern l_ok convertXYZToRGB ( l_float32 fxval, l_float32 fyval, l_float32 fzval, l_int32 blackout, l_int32 *prval, l_int32 *pgval, l_int32 *pbval );
LEPT_DLL extern FPIXA * fpixaConvertXYZToLAB ( FPIXA *fpixas );
LEPT_DLL extern FPIXA * fpixaConvertLABToXYZ ( FPIXA *fpixas );
LEPT_DLL extern l_ok convertXYZToLAB ( l_float32 xval, l_float32 yval, l_float32 zval, l_float32 *plval, l_float32 *paval, l_float32 *pbval );
LEPT_DLL extern l_ok convertLABToXYZ ( l_float32 lval, l_float32 aval, l_float32 bval, l_float32 *pxval, l_float32 *pyval, l_float32 *pzval );
LEPT_DLL extern FPIXA * pixConvertRGBToLAB ( PIX *pixs );
LEPT_DLL extern PIX * fpixaConvertLABToRGB ( FPIXA *fpixa );
LEPT_DLL extern l_ok convertRGBToLAB ( l_int32 rval, l_int32 gval, l_int32 bval, l_float32 *pflval, l_float32 *pfaval, l_float32 *pfbval );
LEPT_DLL extern l_ok convertLABToRGB ( l_float32 flval, l_float32 faval, l_float32 fbval, l_int32 *prval, l_int32 *pgval, l_int32 *pbval );
LEPT_DLL extern l_ok pixEqual ( PIX *pix1, PIX *pix2, l_int32 *psame );
LEPT_DLL extern l_ok pixEqualWithAlpha ( PIX *pix1, PIX *pix2, l_int32 use_alpha, l_int32 *psame );
LEPT_DLL extern l_ok pixEqualWithCmap ( PIX *pix1, PIX *pix2, l_int32 *psame );
LEPT_DLL extern l_ok cmapEqual ( PIXCMAP *cmap1, PIXCMAP *cmap2, l_int32 ncomps, l_int32 *psame );
LEPT_DLL extern l_ok pixUsesCmapColor ( PIX *pixs, l_int32 *pcolor );
LEPT_DLL extern l_ok pixCorrelationBinary ( PIX *pix1, PIX *pix2, l_float32 *pval );
LEPT_DLL extern PIX * pixDisplayDiffBinary ( PIX *pix1, PIX *pix2 );
LEPT_DLL extern l_ok pixCompareBinary ( PIX *pix1, PIX *pix2, l_int32 comptype, l_float32 *pfract, PIX **ppixdiff );
LEPT_DLL extern l_ok pixCompareGrayOrRGB ( PIX *pix1, PIX *pix2, l_int32 comptype, l_int32 plottype, l_int32 *psame, l_float32 *pdiff, l_float32 *prmsdiff, PIX **ppixdiff );
LEPT_DLL extern l_ok pixCompareGray ( PIX *pix1, PIX *pix2, l_int32 comptype, l_int32 plottype, l_int32 *psame, l_float32 *pdiff, l_float32 *prmsdiff, PIX **ppixdiff );
LEPT_DLL extern l_ok pixCompareRGB ( PIX *pix1, PIX *pix2, l_int32 comptype, l_int32 plottype, l_int32 *psame, l_float32 *pdiff, l_float32 *prmsdiff, PIX **ppixdiff );
LEPT_DLL extern l_ok pixCompareTiled ( PIX *pix1, PIX *pix2, l_int32 sx, l_int32 sy, l_int32 type, PIX **ppixdiff );
LEPT_DLL extern NUMA * pixCompareRankDifference ( PIX *pix1, PIX *pix2, l_int32 factor );
LEPT_DLL extern l_ok pixTestForSimilarity ( PIX *pix1, PIX *pix2, l_int32 factor, l_int32 mindiff, l_float32 maxfract, l_float32 maxave, l_int32 *psimilar, l_int32 details );
LEPT_DLL extern l_ok pixGetDifferenceStats ( PIX *pix1, PIX *pix2, l_int32 factor, l_int32 mindiff, l_float32 *pfractdiff, l_float32 *pavediff, l_int32 details );
LEPT_DLL extern NUMA * pixGetDifferenceHistogram ( PIX *pix1, PIX *pix2, l_int32 factor );
LEPT_DLL extern l_ok pixGetPerceptualDiff ( PIX *pixs1, PIX *pixs2, l_int32 sampling, l_int32 dilation, l_int32 mindiff, l_float32 *pfract, PIX **ppixdiff1, PIX **ppixdiff2 );
LEPT_DLL extern l_ok pixGetPSNR ( PIX *pix1, PIX *pix2, l_int32 factor, l_float32 *ppsnr );
LEPT_DLL extern l_ok pixaComparePhotoRegionsByHisto ( PIXA *pixa, l_float32 minratio, l_float32 textthresh, l_int32 factor, l_int32 n, l_float32 simthresh, NUMA **pnai, l_float32 **pscores, PIX **ppixd, l_int32 debug );
LEPT_DLL extern l_ok pixComparePhotoRegionsByHisto ( PIX *pix1, PIX *pix2, BOX *box1, BOX *box2, l_float32 minratio, l_int32 factor, l_int32 n, l_float32 *pscore, l_int32 debugflag );
LEPT_DLL extern l_ok pixGenPhotoHistos ( PIX *pixs, BOX *box, l_int32 factor, l_float32 thresh, l_int32 n, NUMAA **pnaa, l_int32 *pw, l_int32 *ph, l_int32 debugindex );
LEPT_DLL extern PIX * pixPadToCenterCentroid ( PIX *pixs, l_int32 factor );
LEPT_DLL extern l_ok pixCentroid8 ( PIX *pixs, l_int32 factor, l_float32 *pcx, l_float32 *pcy );
LEPT_DLL extern l_ok pixDecideIfPhotoImage ( PIX *pix, l_int32 factor, l_float32 thresh, l_int32 n, NUMAA **pnaa, PIXA *pixadebug );
LEPT_DLL extern l_ok compareTilesByHisto ( NUMAA *naa1, NUMAA *naa2, l_float32 minratio, l_int32 w1, l_int32 h1, l_int32 w2, l_int32 h2, l_float32 *pscore, PIXA *pixadebug );
LEPT_DLL extern l_ok pixCompareGrayByHisto ( PIX *pix1, PIX *pix2, BOX *box1, BOX *box2, l_float32 minratio, l_int32 maxgray, l_int32 factor, l_int32 n, l_float32 *pscore, l_int32 debugflag );
LEPT_DLL extern l_ok pixCropAlignedToCentroid ( PIX *pix1, PIX *pix2, l_int32 factor, BOX **pbox1, BOX **pbox2 );
LEPT_DLL extern l_uint8 * l_compressGrayHistograms ( NUMAA *naa, l_int32 w, l_int32 h, size_t *psize );
LEPT_DLL extern NUMAA * l_uncompressGrayHistograms ( l_uint8 *bytea, size_t size, l_int32 *pw, l_int32 *ph );
LEPT_DLL extern l_ok pixCompareWithTranslation ( PIX *pix1, PIX *pix2, l_int32 thresh, l_int32 *pdelx, l_int32 *pdely, l_float32 *pscore, l_int32 debugflag );
LEPT_DLL extern l_ok pixBestCorrelation ( PIX *pix1, PIX *pix2, l_int32 area1, l_int32 area2, l_int32 etransx, l_int32 etransy, l_int32 maxshift, l_int32 *tab8, l_int32 *pdelx, l_int32 *pdely, l_float32 *pscore, l_int32 debugflag );
LEPT_DLL extern BOXA * pixConnComp ( PIX *pixs, PIXA **ppixa, l_int32 connectivity );
LEPT_DLL extern BOXA * pixConnCompPixa ( PIX *pixs, PIXA **ppixa, l_int32 connectivity );
LEPT_DLL extern BOXA * pixConnCompBB ( PIX *pixs, l_int32 connectivity );
LEPT_DLL extern l_ok pixCountConnComp ( PIX *pixs, l_int32 connectivity, l_int32 *pcount );
LEPT_DLL extern l_int32 nextOnPixelInRaster ( PIX *pixs, l_int32 xstart, l_int32 ystart, l_int32 *px, l_int32 *py );
LEPT_DLL extern BOX * pixSeedfillBB ( PIX *pixs, L_STACK *stack, l_int32 x, l_int32 y, l_int32 connectivity );
LEPT_DLL extern BOX * pixSeedfill4BB ( PIX *pixs, L_STACK *stack, l_int32 x, l_int32 y );
LEPT_DLL extern BOX * pixSeedfill8BB ( PIX *pixs, L_STACK *stack, l_int32 x, l_int32 y );
LEPT_DLL extern l_ok pixSeedfill ( PIX *pixs, L_STACK *stack, l_int32 x, l_int32 y, l_int32 connectivity );
LEPT_DLL extern l_ok pixSeedfill4 ( PIX *pixs, L_STACK *stack, l_int32 x, l_int32 y );
LEPT_DLL extern l_ok pixSeedfill8 ( PIX *pixs, L_STACK *stack, l_int32 x, l_int32 y );
LEPT_DLL extern l_ok convertFilesTo1bpp ( const char *dirin, const char *substr, l_int32 upscaling, l_int32 thresh, l_int32 firstpage, l_int32 npages, const char *dirout, l_int32 outformat );
LEPT_DLL extern PIX * pixBlockconv ( PIX *pix, l_int32 wc, l_int32 hc );
LEPT_DLL extern PIX * pixBlockconvGray ( PIX *pixs, PIX *pixacc, l_int32 wc, l_int32 hc );
LEPT_DLL extern PIX * pixBlockconvAccum ( PIX *pixs );
LEPT_DLL extern PIX * pixBlockconvGrayUnnormalized ( PIX *pixs, l_int32 wc, l_int32 hc );
LEPT_DLL extern PIX * pixBlockconvTiled ( PIX *pix, l_int32 wc, l_int32 hc, l_int32 nx, l_int32 ny );
LEPT_DLL extern PIX * pixBlockconvGrayTile ( PIX *pixs, PIX *pixacc, l_int32 wc, l_int32 hc );
LEPT_DLL extern l_ok pixWindowedStats ( PIX *pixs, l_int32 wc, l_int32 hc, l_int32 hasborder, PIX **ppixm, PIX **ppixms, FPIX **pfpixv, FPIX **pfpixrv );
LEPT_DLL extern PIX * pixWindowedMean ( PIX *pixs, l_int32 wc, l_int32 hc, l_int32 hasborder, l_int32 normflag );
LEPT_DLL extern PIX * pixWindowedMeanSquare ( PIX *pixs, l_int32 wc, l_int32 hc, l_int32 hasborder );
LEPT_DLL extern l_ok pixWindowedVariance ( PIX *pixm, PIX *pixms, FPIX **pfpixv, FPIX **pfpixrv );
LEPT_DLL extern DPIX * pixMeanSquareAccum ( PIX *pixs );
LEPT_DLL extern PIX * pixBlockrank ( PIX *pixs, PIX *pixacc, l_int32 wc, l_int32 hc, l_float32 rank );
LEPT_DLL extern PIX * pixBlocksum ( PIX *pixs, PIX *pixacc, l_int32 wc, l_int32 hc );
LEPT_DLL extern PIX * pixCensusTransform ( PIX *pixs, l_int32 halfsize, PIX *pixacc );
LEPT_DLL extern PIX * pixConvolve ( PIX *pixs, L_KERNEL *kel, l_int32 outdepth, l_int32 normflag );
LEPT_DLL extern PIX * pixConvolveSep ( PIX *pixs, L_KERNEL *kelx, L_KERNEL *kely, l_int32 outdepth, l_int32 normflag );
LEPT_DLL extern PIX * pixConvolveRGB ( PIX *pixs, L_KERNEL *kel );
LEPT_DLL extern PIX * pixConvolveRGBSep ( PIX *pixs, L_KERNEL *kelx, L_KERNEL *kely );
LEPT_DLL extern FPIX * fpixConvolve ( FPIX *fpixs, L_KERNEL *kel, l_int32 normflag );
LEPT_DLL extern FPIX * fpixConvolveSep ( FPIX *fpixs, L_KERNEL *kelx, L_KERNEL *kely, l_int32 normflag );
LEPT_DLL extern PIX * pixConvolveWithBias ( PIX *pixs, L_KERNEL *kel1, L_KERNEL *kel2, l_int32 force8, l_int32 *pbias );
LEPT_DLL extern void l_setConvolveSampling ( l_int32 xfact, l_int32 yfact );
LEPT_DLL extern PIX * pixAddGaussianNoise ( PIX *pixs, l_float32 stdev );
LEPT_DLL extern l_float32 gaussDistribSampling ( void );
LEPT_DLL extern l_ok pixCorrelationScore ( PIX *pix1, PIX *pix2, l_int32 area1, l_int32 area2, l_float32 delx, l_float32 dely, l_int32 maxdiffw, l_int32 maxdiffh, l_int32 *tab, l_float32 *pscore );
LEPT_DLL extern l_int32 pixCorrelationScoreThresholded ( PIX *pix1, PIX *pix2, l_int32 area1, l_int32 area2, l_float32 delx, l_float32 dely, l_int32 maxdiffw, l_int32 maxdiffh, l_int32 *tab, l_int32 *downcount, l_float32 score_threshold );
LEPT_DLL extern l_ok pixCorrelationScoreSimple ( PIX *pix1, PIX *pix2, l_int32 area1, l_int32 area2, l_float32 delx, l_float32 dely, l_int32 maxdiffw, l_int32 maxdiffh, l_int32 *tab, l_float32 *pscore );
LEPT_DLL extern l_ok pixCorrelationScoreShifted ( PIX *pix1, PIX *pix2, l_int32 area1, l_int32 area2, l_int32 delx, l_int32 dely, l_int32 *tab, l_float32 *pscore );
LEPT_DLL extern L_DEWARP * dewarpCreate ( PIX *pixs, l_int32 pageno );
LEPT_DLL extern L_DEWARP * dewarpCreateRef ( l_int32 pageno, l_int32 refpage );
LEPT_DLL extern void dewarpDestroy ( L_DEWARP **pdew );
LEPT_DLL extern L_DEWARPA * dewarpaCreate ( l_int32 nptrs, l_int32 sampling, l_int32 redfactor, l_int32 minlines, l_int32 maxdist );
LEPT_DLL extern L_DEWARPA * dewarpaCreateFromPixacomp ( PIXAC *pixac, l_int32 useboth, l_int32 sampling, l_int32 minlines, l_int32 maxdist );
LEPT_DLL extern void dewarpaDestroy ( L_DEWARPA **pdewa );
LEPT_DLL extern l_ok dewarpaDestroyDewarp ( L_DEWARPA *dewa, l_int32 pageno );
LEPT_DLL extern l_ok dewarpaInsertDewarp ( L_DEWARPA *dewa, L_DEWARP *dew );
LEPT_DLL extern L_DEWARP * dewarpaGetDewarp ( L_DEWARPA *dewa, l_int32 index );
LEPT_DLL extern l_ok dewarpaSetCurvatures ( L_DEWARPA *dewa, l_int32 max_linecurv, l_int32 min_diff_linecurv, l_int32 max_diff_linecurv, l_int32 max_edgecurv, l_int32 max_diff_edgecurv, l_int32 max_edgeslope );
LEPT_DLL extern l_ok dewarpaUseBothArrays ( L_DEWARPA *dewa, l_int32 useboth );
LEPT_DLL extern l_ok dewarpaSetCheckColumns ( L_DEWARPA *dewa, l_int32 check_columns );
LEPT_DLL extern l_ok dewarpaSetMaxDistance ( L_DEWARPA *dewa, l_int32 maxdist );
LEPT_DLL extern L_DEWARP * dewarpRead ( const char *filename );
LEPT_DLL extern L_DEWARP * dewarpReadStream ( FILE *fp );
LEPT_DLL extern L_DEWARP * dewarpReadMem ( const l_uint8 *data, size_t size );
LEPT_DLL extern l_ok dewarpWrite ( const char *filename, L_DEWARP *dew );
LEPT_DLL extern l_ok dewarpWriteStream ( FILE *fp, L_DEWARP *dew );
LEPT_DLL extern l_ok dewarpWriteMem ( l_uint8 **pdata, size_t *psize, L_DEWARP *dew );
LEPT_DLL extern L_DEWARPA * dewarpaRead ( const char *filename );
LEPT_DLL extern L_DEWARPA * dewarpaReadStream ( FILE *fp );
LEPT_DLL extern L_DEWARPA * dewarpaReadMem ( const l_uint8 *data, size_t size );
LEPT_DLL extern l_ok dewarpaWrite ( const char *filename, L_DEWARPA *dewa );
LEPT_DLL extern l_ok dewarpaWriteStream ( FILE *fp, L_DEWARPA *dewa );
LEPT_DLL extern l_ok dewarpaWriteMem ( l_uint8 **pdata, size_t *psize, L_DEWARPA *dewa );
/* WILLUS MOD */
    LEPT_DLL extern l_ok dewarpBuildPageModel_ex ( L_DEWARP *dew, const char *debugfile,l_int32 fit_order );
    LEPT_DLL extern l_ok dewarpFindVertDisparity_ex ( L_DEWARP *dew, PTAA *ptaa, l_int32 rotflag,l_int32 fit_order );
    LEPT_DLL extern l_ok dewarpBuildLineModel_ex ( L_DEWARP *dew, l_int32 opensize, const char *debugfile,l_int32 fit_order );
LEPT_DLL extern l_ok dewarpBuildPageModel ( L_DEWARP *dew, const char *debugfile );
LEPT_DLL extern l_ok dewarpFindVertDisparity ( L_DEWARP *dew, PTAA *ptaa, l_int32 rotflag );
LEPT_DLL extern l_ok dewarpFindHorizDisparity ( L_DEWARP *dew, PTAA *ptaa );
LEPT_DLL extern PTAA * dewarpGetTextlineCenters ( PIX *pixs, l_int32 debugflag );
LEPT_DLL extern PTAA * dewarpRemoveShortLines ( PIX *pixs, PTAA *ptaas, l_float32 fract, l_int32 debugflag );
LEPT_DLL extern l_ok dewarpFindHorizSlopeDisparity ( L_DEWARP *dew, PIX *pixb, l_float32 fractthresh, l_int32 parity );
LEPT_DLL extern l_ok dewarpBuildLineModel ( L_DEWARP *dew, l_int32 opensize, const char *debugfile );
LEPT_DLL extern l_ok dewarpaModelStatus ( L_DEWARPA *dewa, l_int32 pageno, l_int32 *pvsuccess, l_int32 *phsuccess );
LEPT_DLL extern l_ok dewarpaApplyDisparity ( L_DEWARPA *dewa, l_int32 pageno, PIX *pixs, l_int32 grayin, l_int32 x, l_int32 y, PIX **ppixd, const char *debugfile );
LEPT_DLL extern l_ok dewarpaApplyDisparityBoxa ( L_DEWARPA *dewa, l_int32 pageno, PIX *pixs, BOXA *boxas, l_int32 mapdir, l_int32 x, l_int32 y, BOXA **pboxad, const char *debugfile );
LEPT_DLL extern l_ok dewarpMinimize ( L_DEWARP *dew );
LEPT_DLL extern l_ok dewarpPopulateFullRes ( L_DEWARP *dew, PIX *pix, l_int32 x, l_int32 y );
LEPT_DLL extern l_ok dewarpSinglePage ( PIX *pixs, l_int32 thresh, l_int32 adaptive, l_int32 useboth, l_int32 check_columns, PIX **ppixd, L_DEWARPA **pdewa, l_int32 debug );
LEPT_DLL extern l_ok dewarpSinglePageInit ( PIX *pixs, l_int32 thresh, l_int32 adaptive, l_int32 useboth, l_int32 check_columns, PIX **ppixb, L_DEWARPA **pdewa );
LEPT_DLL extern l_ok dewarpSinglePageRun ( PIX *pixs, PIX *pixb, L_DEWARPA *dewa, PIX **ppixd, l_int32 debug );
LEPT_DLL extern l_ok dewarpaListPages ( L_DEWARPA *dewa );
LEPT_DLL extern l_ok dewarpaSetValidModels ( L_DEWARPA *dewa, l_int32 notests, l_int32 debug );
LEPT_DLL extern l_ok dewarpaInsertRefModels ( L_DEWARPA *dewa, l_int32 notests, l_int32 debug );
LEPT_DLL extern l_ok dewarpaStripRefModels ( L_DEWARPA *dewa );
LEPT_DLL extern l_ok dewarpaRestoreModels ( L_DEWARPA *dewa );
LEPT_DLL extern l_ok dewarpaInfo ( FILE *fp, L_DEWARPA *dewa );
LEPT_DLL extern l_ok dewarpaModelStats ( L_DEWARPA *dewa, l_int32 *pnnone, l_int32 *pnvsuccess, l_int32 *pnvvalid, l_int32 *pnhsuccess, l_int32 *pnhvalid, l_int32 *pnref );
LEPT_DLL extern l_ok dewarpaShowArrays ( L_DEWARPA *dewa, l_float32 scalefact, l_int32 first, l_int32 last );
LEPT_DLL extern l_ok dewarpDebug ( L_DEWARP *dew, const char *subdirs, l_int32 index );
LEPT_DLL extern l_ok dewarpShowResults ( L_DEWARPA *dewa, SARRAY *sa, BOXA *boxa, l_int32 firstpage, l_int32 lastpage, const char *pdfout );
LEPT_DLL extern L_DNA * l_dnaCreate ( l_int32 n );
LEPT_DLL extern L_DNA * l_dnaCreateFromIArray ( l_int32 *iarray, l_int32 size );
LEPT_DLL extern L_DNA * l_dnaCreateFromDArray ( l_float64 *darray, l_int32 size, l_int32 copyflag );
LEPT_DLL extern L_DNA * l_dnaMakeSequence ( l_float64 startval, l_float64 increment, l_int32 size );
LEPT_DLL extern void l_dnaDestroy ( L_DNA **pda );
LEPT_DLL extern L_DNA * l_dnaCopy ( L_DNA *da );
LEPT_DLL extern L_DNA * l_dnaClone ( L_DNA *da );
LEPT_DLL extern l_ok l_dnaEmpty ( L_DNA *da );
LEPT_DLL extern l_ok l_dnaAddNumber ( L_DNA *da, l_float64 val );
LEPT_DLL extern l_ok l_dnaInsertNumber ( L_DNA *da, l_int32 index, l_float64 val );
LEPT_DLL extern l_ok l_dnaRemoveNumber ( L_DNA *da, l_int32 index );
LEPT_DLL extern l_ok l_dnaReplaceNumber ( L_DNA *da, l_int32 index, l_float64 val );
LEPT_DLL extern l_int32 l_dnaGetCount ( L_DNA *da );
LEPT_DLL extern l_ok l_dnaSetCount ( L_DNA *da, l_int32 newcount );
LEPT_DLL extern l_ok l_dnaGetDValue ( L_DNA *da, l_int32 index, l_float64 *pval );
LEPT_DLL extern l_ok l_dnaGetIValue ( L_DNA *da, l_int32 index, l_int32 *pival );
LEPT_DLL extern l_ok l_dnaSetValue ( L_DNA *da, l_int32 index, l_float64 val );
LEPT_DLL extern l_ok l_dnaShiftValue ( L_DNA *da, l_int32 index, l_float64 diff );
LEPT_DLL extern l_int32 * l_dnaGetIArray ( L_DNA *da );
LEPT_DLL extern l_float64 * l_dnaGetDArray ( L_DNA *da, l_int32 copyflag );
LEPT_DLL extern l_int32 l_dnaGetRefcount ( L_DNA *da );
LEPT_DLL extern l_ok l_dnaChangeRefcount ( L_DNA *da, l_int32 delta );
LEPT_DLL extern l_ok l_dnaGetParameters ( L_DNA *da, l_float64 *pstartx, l_float64 *pdelx );
LEPT_DLL extern l_ok l_dnaSetParameters ( L_DNA *da, l_float64 startx, l_float64 delx );
LEPT_DLL extern l_ok l_dnaCopyParameters ( L_DNA *dad, L_DNA *das );
LEPT_DLL extern L_DNA * l_dnaRead ( const char *filename );
LEPT_DLL extern L_DNA * l_dnaReadStream ( FILE *fp );
LEPT_DLL extern l_ok l_dnaWrite ( const char *filename, L_DNA *da );
LEPT_DLL extern l_ok l_dnaWriteStream ( FILE *fp, L_DNA *da );
LEPT_DLL extern L_DNAA * l_dnaaCreate ( l_int32 n );
LEPT_DLL extern L_DNAA * l_dnaaCreateFull ( l_int32 nptr, l_int32 n );
LEPT_DLL extern l_ok l_dnaaTruncate ( L_DNAA *daa );
LEPT_DLL extern void l_dnaaDestroy ( L_DNAA **pdaa );
LEPT_DLL extern l_ok l_dnaaAddDna ( L_DNAA *daa, L_DNA *da, l_int32 copyflag );
LEPT_DLL extern l_int32 l_dnaaGetCount ( L_DNAA *daa );
LEPT_DLL extern l_int32 l_dnaaGetDnaCount ( L_DNAA *daa, l_int32 index );
LEPT_DLL extern l_int32 l_dnaaGetNumberCount ( L_DNAA *daa );
LEPT_DLL extern L_DNA * l_dnaaGetDna ( L_DNAA *daa, l_int32 index, l_int32 accessflag );
LEPT_DLL extern l_ok l_dnaaReplaceDna ( L_DNAA *daa, l_int32 index, L_DNA *da );
LEPT_DLL extern l_ok l_dnaaGetValue ( L_DNAA *daa, l_int32 i, l_int32 j, l_float64 *pval );
LEPT_DLL extern l_ok l_dnaaAddNumber ( L_DNAA *daa, l_int32 index, l_float64 val );
LEPT_DLL extern L_DNAA * l_dnaaRead ( const char *filename );
LEPT_DLL extern L_DNAA * l_dnaaReadStream ( FILE *fp );
LEPT_DLL extern l_ok l_dnaaWrite ( const char *filename, L_DNAA *daa );
LEPT_DLL extern l_ok l_dnaaWriteStream ( FILE *fp, L_DNAA *daa );
LEPT_DLL extern l_ok l_dnaJoin ( L_DNA *dad, L_DNA *das, l_int32 istart, l_int32 iend );
LEPT_DLL extern L_DNA * l_dnaaFlattenToDna ( L_DNAA *daa );
LEPT_DLL extern NUMA * l_dnaConvertToNuma ( L_DNA *da );
LEPT_DLL extern L_DNA * numaConvertToDna ( NUMA *na );
LEPT_DLL extern L_DNA * l_dnaUnionByAset ( L_DNA *da1, L_DNA *da2 );
LEPT_DLL extern L_DNA * l_dnaRemoveDupsByAset ( L_DNA *das );
LEPT_DLL extern L_DNA * l_dnaIntersectionByAset ( L_DNA *da1, L_DNA *da2 );
LEPT_DLL extern L_ASET * l_asetCreateFromDna ( L_DNA *da );
LEPT_DLL extern L_DNA * l_dnaDiffAdjValues ( L_DNA *das );
LEPT_DLL extern L_DNAHASH * l_dnaHashCreate ( l_int32 nbuckets, l_int32 initsize );
LEPT_DLL extern void l_dnaHashDestroy ( L_DNAHASH **pdahash );
LEPT_DLL extern l_int32 l_dnaHashGetCount ( L_DNAHASH *dahash );
LEPT_DLL extern l_int32 l_dnaHashGetTotalCount ( L_DNAHASH *dahash );
LEPT_DLL extern L_DNA * l_dnaHashGetDna ( L_DNAHASH *dahash, l_uint64 key, l_int32 copyflag );
LEPT_DLL extern l_ok l_dnaHashAdd ( L_DNAHASH *dahash, l_uint64 key, l_float64 value );
LEPT_DLL extern L_DNAHASH * l_dnaHashCreateFromDna ( L_DNA *da );
LEPT_DLL extern l_ok l_dnaRemoveDupsByHash ( L_DNA *das, L_DNA **pdad, L_DNAHASH **pdahash );
LEPT_DLL extern l_ok l_dnaMakeHistoByHash ( L_DNA *das, L_DNAHASH **pdahash, L_DNA **pdav, L_DNA **pdac );
LEPT_DLL extern L_DNA * l_dnaIntersectionByHash ( L_DNA *da1, L_DNA *da2 );
LEPT_DLL extern l_ok l_dnaFindValByHash ( L_DNA *da, L_DNAHASH *dahash, l_float64 val, l_int32 *pindex );
LEPT_DLL extern PIX * pixMorphDwa_2 ( PIX *pixd, PIX *pixs, l_int32 operation, char *selname );
LEPT_DLL extern PIX * pixFMorphopGen_2 ( PIX *pixd, PIX *pixs, l_int32 operation, char *selname );
LEPT_DLL extern l_int32 fmorphopgen_low_2 ( l_uint32 *datad, l_int32 w, l_int32 h, l_int32 wpld, l_uint32 *datas, l_int32 wpls, l_int32 index );
LEPT_DLL extern PIX * pixSobelEdgeFilter ( PIX *pixs, l_int32 orientflag );
LEPT_DLL extern PIX * pixTwoSidedEdgeFilter ( PIX *pixs, l_int32 orientflag );
LEPT_DLL extern l_ok pixMeasureEdgeSmoothness ( PIX *pixs, l_int32 side, l_int32 minjump, l_int32 minreversal, l_float32 *pjpl, l_float32 *pjspl, l_float32 *prpl, const char *debugfile );
LEPT_DLL extern NUMA * pixGetEdgeProfile ( PIX *pixs, l_int32 side, const char *debugfile );
LEPT_DLL extern l_ok pixGetLastOffPixelInRun ( PIX *pixs, l_int32 x, l_int32 y, l_int32 direction, l_int32 *ploc );
LEPT_DLL extern l_int32 pixGetLastOnPixelInRun ( PIX *pixs, l_int32 x, l_int32 y, l_int32 direction, l_int32 *ploc );
LEPT_DLL extern char * encodeBase64 ( const l_uint8 *inarray, l_int32 insize, l_int32 *poutsize );
LEPT_DLL extern l_uint8 * decodeBase64 ( const char *inarray, l_int32 insize, l_int32 *poutsize );
LEPT_DLL extern char * encodeAscii85 ( const l_uint8 *inarray, l_int32 insize, l_int32 *poutsize );
LEPT_DLL extern l_uint8 * decodeAscii85 ( const char *inarray, l_int32 insize, l_int32 *poutsize );
LEPT_DLL extern char * reformatPacked64 ( const char *inarray, l_int32 insize, l_int32 leadspace, l_int32 linechars, l_int32 addquotes, l_int32 *poutsize );
LEPT_DLL extern PIX * pixGammaTRC ( PIX *pixd, PIX *pixs, l_float32 gamma, l_int32 minval, l_int32 maxval );
LEPT_DLL extern PIX * pixGammaTRCMasked ( PIX *pixd, PIX *pixs, PIX *pixm, l_float32 gamma, l_int32 minval, l_int32 maxval );
LEPT_DLL extern PIX * pixGammaTRCWithAlpha ( PIX *pixd, PIX *pixs, l_float32 gamma, l_int32 minval, l_int32 maxval );
LEPT_DLL extern NUMA * numaGammaTRC ( l_float32 gamma, l_int32 minval, l_int32 maxval );
LEPT_DLL extern PIX * pixContrastTRC ( PIX *pixd, PIX *pixs, l_float32 factor );
LEPT_DLL extern PIX * pixContrastTRCMasked ( PIX *pixd, PIX *pixs, PIX *pixm, l_float32 factor );
LEPT_DLL extern NUMA * numaContrastTRC ( l_float32 factor );
LEPT_DLL extern PIX * pixEqualizeTRC ( PIX *pixd, PIX *pixs, l_float32 fract, l_int32 factor );
LEPT_DLL extern NUMA * numaEqualizeTRC ( PIX *pix, l_float32 fract, l_int32 factor );
LEPT_DLL extern l_int32 pixTRCMap ( PIX *pixs, PIX *pixm, NUMA *na );
LEPT_DLL extern l_int32 pixTRCMapGeneral ( PIX *pixs, PIX *pixm, NUMA *nar, NUMA *nag, NUMA *nab );
LEPT_DLL extern PIX * pixUnsharpMasking ( PIX *pixs, l_int32 halfwidth, l_float32 fract );
LEPT_DLL extern PIX * pixUnsharpMaskingGray ( PIX *pixs, l_int32 halfwidth, l_float32 fract );
LEPT_DLL extern PIX * pixUnsharpMaskingFast ( PIX *pixs, l_int32 halfwidth, l_float32 fract, l_int32 direction );
LEPT_DLL extern PIX * pixUnsharpMaskingGrayFast ( PIX *pixs, l_int32 halfwidth, l_float32 fract, l_int32 direction );
LEPT_DLL extern PIX * pixUnsharpMaskingGray1D ( PIX *pixs, l_int32 halfwidth, l_float32 fract, l_int32 direction );
LEPT_DLL extern PIX * pixUnsharpMaskingGray2D ( PIX *pixs, l_int32 halfwidth, l_float32 fract );
LEPT_DLL extern PIX * pixModifyHue ( PIX *pixd, PIX *pixs, l_float32 fract );
LEPT_DLL extern PIX * pixModifySaturation ( PIX *pixd, PIX *pixs, l_float32 fract );
LEPT_DLL extern l_int32 pixMeasureSaturation ( PIX *pixs, l_int32 factor, l_float32 *psat );
LEPT_DLL extern PIX * pixModifyBrightness ( PIX *pixd, PIX *pixs, l_float32 fract );
LEPT_DLL extern PIX * pixMosaicColorShiftRGB ( PIX *pixs, l_float32 roff, l_float32 goff, l_float32 boff, l_float32 delta, l_int32 nincr );
LEPT_DLL extern PIX * pixColorShiftRGB ( PIX *pixs, l_float32 rfract, l_float32 gfract, l_float32 bfract );
LEPT_DLL extern PIX * pixDarkenGray ( PIX *pixd, PIX *pixs, l_int32 thresh, l_int32 satlimit );
LEPT_DLL extern PIX * pixMultConstantColor ( PIX *pixs, l_float32 rfact, l_float32 gfact, l_float32 bfact );
LEPT_DLL extern PIX * pixMultMatrixColor ( PIX *pixs, L_KERNEL *kel );
LEPT_DLL extern PIX * pixHalfEdgeByBandpass ( PIX *pixs, l_int32 sm1h, l_int32 sm1v, l_int32 sm2h, l_int32 sm2v );
LEPT_DLL extern l_ok fhmtautogen ( SELA *sela, l_int32 fileindex, const char *filename );
LEPT_DLL extern l_ok fhmtautogen1 ( SELA *sela, l_int32 fileindex, const char *filename );
LEPT_DLL extern l_ok fhmtautogen2 ( SELA *sela, l_int32 fileindex, const char *filename );
LEPT_DLL extern PIX * pixHMTDwa_1 ( PIX *pixd, PIX *pixs, const char *selname );
LEPT_DLL extern PIX * pixFHMTGen_1 ( PIX *pixd, PIX *pixs, const char *selname );
LEPT_DLL extern l_int32 fhmtgen_low_1 ( l_uint32 *datad, l_int32 w, l_int32 h, l_int32 wpld, l_uint32 *datas, l_int32 wpls, l_int32 index );
LEPT_DLL extern l_ok pixItalicWords ( PIX *pixs, BOXA *boxaw, PIX *pixw, BOXA **pboxa, l_int32 debugflag );
LEPT_DLL extern PIX * pixOrientCorrect ( PIX *pixs, l_float32 minupconf, l_float32 minratio, l_float32 *pupconf, l_float32 *pleftconf, l_int32 *protation, l_int32 debug );
LEPT_DLL extern l_ok pixOrientDetect ( PIX *pixs, l_float32 *pupconf, l_float32 *pleftconf, l_int32 mincount, l_int32 debug );
LEPT_DLL extern l_ok makeOrientDecision ( l_float32 upconf, l_float32 leftconf, l_float32 minupconf, l_float32 minratio, l_int32 *porient, l_int32 debug );
LEPT_DLL extern l_ok pixUpDownDetect ( PIX *pixs, l_float32 *pconf, l_int32 mincount, l_int32 debug );
LEPT_DLL extern l_ok pixUpDownDetectGeneral ( PIX *pixs, l_float32 *pconf, l_int32 mincount, l_int32 npixels, l_int32 debug );
LEPT_DLL extern l_ok pixOrientDetectDwa ( PIX *pixs, l_float32 *pupconf, l_float32 *pleftconf, l_int32 mincount, l_int32 debug );
LEPT_DLL extern l_ok pixUpDownDetectDwa ( PIX *pixs, l_float32 *pconf, l_int32 mincount, l_int32 debug );
LEPT_DLL extern l_ok pixUpDownDetectGeneralDwa ( PIX *pixs, l_float32 *pconf, l_int32 mincount, l_int32 npixels, l_int32 debug );
LEPT_DLL extern l_ok pixMirrorDetect ( PIX *pixs, l_float32 *pconf, l_int32 mincount, l_int32 debug );
LEPT_DLL extern l_ok pixMirrorDetectDwa ( PIX *pixs, l_float32 *pconf, l_int32 mincount, l_int32 debug );
LEPT_DLL extern PIX * pixFlipFHMTGen ( PIX *pixd, PIX *pixs, const char *selname );
LEPT_DLL extern l_ok fmorphautogen ( SELA *sela, l_int32 fileindex, const char *filename );
LEPT_DLL extern l_ok fmorphautogen1 ( SELA *sela, l_int32 fileindex, const char *filename );
LEPT_DLL extern l_int32 fmorphautogen2 ( SELA *sela, l_int32 fileindex, const char *filename );
LEPT_DLL extern PIX * pixMorphDwa_1 ( PIX *pixd, PIX *pixs, l_int32 operation, char *selname );
LEPT_DLL extern PIX * pixFMorphopGen_1 ( PIX *pixd, PIX *pixs, l_int32 operation, char *selname );
LEPT_DLL extern l_int32 fmorphopgen_low_1 ( l_uint32 *datad, l_int32 w, l_int32 h, l_int32 wpld, l_uint32 *datas, l_int32 wpls, l_int32 index );
LEPT_DLL extern FPIX * fpixCreate ( l_int32 width, l_int32 height );
LEPT_DLL extern FPIX * fpixCreateTemplate ( FPIX *fpixs );
LEPT_DLL extern FPIX * fpixClone ( FPIX *fpix );
LEPT_DLL extern FPIX * fpixCopy ( FPIX *fpixd, FPIX *fpixs );
LEPT_DLL extern l_ok fpixResizeImageData ( FPIX *fpixd, FPIX *fpixs );
LEPT_DLL extern void fpixDestroy ( FPIX **pfpix );
LEPT_DLL extern l_ok fpixGetDimensions ( FPIX *fpix, l_int32 *pw, l_int32 *ph );
LEPT_DLL extern l_ok fpixSetDimensions ( FPIX *fpix, l_int32 w, l_int32 h );
LEPT_DLL extern l_int32 fpixGetWpl ( FPIX *fpix );
LEPT_DLL extern l_ok fpixSetWpl ( FPIX *fpix, l_int32 wpl );
LEPT_DLL extern l_int32 fpixGetRefcount ( FPIX *fpix );
LEPT_DLL extern l_ok fpixChangeRefcount ( FPIX *fpix, l_int32 delta );
LEPT_DLL extern l_ok fpixGetResolution ( FPIX *fpix, l_int32 *pxres, l_int32 *pyres );
LEPT_DLL extern l_ok fpixSetResolution ( FPIX *fpix, l_int32 xres, l_int32 yres );
LEPT_DLL extern l_ok fpixCopyResolution ( FPIX *fpixd, FPIX *fpixs );
LEPT_DLL extern l_float32 * fpixGetData ( FPIX *fpix );
LEPT_DLL extern l_ok fpixSetData ( FPIX *fpix, l_float32 *data );
LEPT_DLL extern l_ok fpixGetPixel ( FPIX *fpix, l_int32 x, l_int32 y, l_float32 *pval );
LEPT_DLL extern l_ok fpixSetPixel ( FPIX *fpix, l_int32 x, l_int32 y, l_float32 val );
LEPT_DLL extern FPIXA * fpixaCreate ( l_int32 n );
LEPT_DLL extern FPIXA * fpixaCopy ( FPIXA *fpixa, l_int32 copyflag );
LEPT_DLL extern void fpixaDestroy ( FPIXA **pfpixa );
LEPT_DLL extern l_ok fpixaAddFPix ( FPIXA *fpixa, FPIX *fpix, l_int32 copyflag );
LEPT_DLL extern l_int32 fpixaGetCount ( FPIXA *fpixa );
LEPT_DLL extern l_ok fpixaChangeRefcount ( FPIXA *fpixa, l_int32 delta );
LEPT_DLL extern FPIX * fpixaGetFPix ( FPIXA *fpixa, l_int32 index, l_int32 accesstype );
LEPT_DLL extern l_ok fpixaGetFPixDimensions ( FPIXA *fpixa, l_int32 index, l_int32 *pw, l_int32 *ph );
LEPT_DLL extern l_float32 * fpixaGetData ( FPIXA *fpixa, l_int32 index );
LEPT_DLL extern l_ok fpixaGetPixel ( FPIXA *fpixa, l_int32 index, l_int32 x, l_int32 y, l_float32 *pval );
LEPT_DLL extern l_ok fpixaSetPixel ( FPIXA *fpixa, l_int32 index, l_int32 x, l_int32 y, l_float32 val );
LEPT_DLL extern DPIX * dpixCreate ( l_int32 width, l_int32 height );
LEPT_DLL extern DPIX * dpixCreateTemplate ( DPIX *dpixs );
LEPT_DLL extern DPIX * dpixClone ( DPIX *dpix );
LEPT_DLL extern DPIX * dpixCopy ( DPIX *dpixd, DPIX *dpixs );
LEPT_DLL extern l_ok dpixResizeImageData ( DPIX *dpixd, DPIX *dpixs );
LEPT_DLL extern void dpixDestroy ( DPIX **pdpix );
LEPT_DLL extern l_ok dpixGetDimensions ( DPIX *dpix, l_int32 *pw, l_int32 *ph );
LEPT_DLL extern l_ok dpixSetDimensions ( DPIX *dpix, l_int32 w, l_int32 h );
LEPT_DLL extern l_int32 dpixGetWpl ( DPIX *dpix );
LEPT_DLL extern l_ok dpixSetWpl ( DPIX *dpix, l_int32 wpl );
LEPT_DLL extern l_int32 dpixGetRefcount ( DPIX *dpix );
LEPT_DLL extern l_ok dpixChangeRefcount ( DPIX *dpix, l_int32 delta );
LEPT_DLL extern l_ok dpixGetResolution ( DPIX *dpix, l_int32 *pxres, l_int32 *pyres );
LEPT_DLL extern l_ok dpixSetResolution ( DPIX *dpix, l_int32 xres, l_int32 yres );
LEPT_DLL extern l_ok dpixCopyResolution ( DPIX *dpixd, DPIX *dpixs );
LEPT_DLL extern l_float64 * dpixGetData ( DPIX *dpix );
LEPT_DLL extern l_ok dpixSetData ( DPIX *dpix, l_float64 *data );
LEPT_DLL extern l_ok dpixGetPixel ( DPIX *dpix, l_int32 x, l_int32 y, l_float64 *pval );
LEPT_DLL extern l_ok dpixSetPixel ( DPIX *dpix, l_int32 x, l_int32 y, l_float64 val );
LEPT_DLL extern FPIX * fpixRead ( const char *filename );
LEPT_DLL extern FPIX * fpixReadStream ( FILE *fp );
LEPT_DLL extern FPIX * fpixReadMem ( const l_uint8 *data, size_t size );
LEPT_DLL extern l_ok fpixWrite ( const char *filename, FPIX *fpix );
LEPT_DLL extern l_ok fpixWriteStream ( FILE *fp, FPIX *fpix );
LEPT_DLL extern l_ok fpixWriteMem ( l_uint8 **pdata, size_t *psize, FPIX *fpix );
LEPT_DLL extern FPIX * fpixEndianByteSwap ( FPIX *fpixd, FPIX *fpixs );
LEPT_DLL extern DPIX * dpixRead ( const char *filename );
LEPT_DLL extern DPIX * dpixReadStream ( FILE *fp );
LEPT_DLL extern DPIX * dpixReadMem ( const l_uint8 *data, size_t size );
LEPT_DLL extern l_ok dpixWrite ( const char *filename, DPIX *dpix );
LEPT_DLL extern l_ok dpixWriteStream ( FILE *fp, DPIX *dpix );
LEPT_DLL extern l_ok dpixWriteMem ( l_uint8 **pdata, size_t *psize, DPIX *dpix );
LEPT_DLL extern DPIX * dpixEndianByteSwap ( DPIX *dpixd, DPIX *dpixs );
LEPT_DLL extern l_ok fpixPrintStream ( FILE *fp, FPIX *fpix, l_int32 factor );
LEPT_DLL extern FPIX * pixConvertToFPix ( PIX *pixs, l_int32 ncomps );
LEPT_DLL extern DPIX * pixConvertToDPix ( PIX *pixs, l_int32 ncomps );
LEPT_DLL extern PIX * fpixConvertToPix ( FPIX *fpixs, l_int32 outdepth, l_int32 negvals, l_int32 errorflag );
LEPT_DLL extern PIX * fpixDisplayMaxDynamicRange ( FPIX *fpixs );
LEPT_DLL extern DPIX * fpixConvertToDPix ( FPIX *fpix );
LEPT_DLL extern PIX * dpixConvertToPix ( DPIX *dpixs, l_int32 outdepth, l_int32 negvals, l_int32 errorflag );
LEPT_DLL extern FPIX * dpixConvertToFPix ( DPIX *dpix );
LEPT_DLL extern l_ok fpixGetMin ( FPIX *fpix, l_float32 *pminval, l_int32 *pxminloc, l_int32 *pyminloc );
LEPT_DLL extern l_ok fpixGetMax ( FPIX *fpix, l_float32 *pmaxval, l_int32 *pxmaxloc, l_int32 *pymaxloc );
LEPT_DLL extern l_ok dpixGetMin ( DPIX *dpix, l_float64 *pminval, l_int32 *pxminloc, l_int32 *pyminloc );
LEPT_DLL extern l_ok dpixGetMax ( DPIX *dpix, l_float64 *pmaxval, l_int32 *pxmaxloc, l_int32 *pymaxloc );
LEPT_DLL extern FPIX * fpixScaleByInteger ( FPIX *fpixs, l_int32 factor );
LEPT_DLL extern DPIX * dpixScaleByInteger ( DPIX *dpixs, l_int32 factor );
LEPT_DLL extern FPIX * fpixLinearCombination ( FPIX *fpixd, FPIX *fpixs1, FPIX *fpixs2, l_float32 a, l_float32 b );
LEPT_DLL extern l_ok fpixAddMultConstant ( FPIX *fpix, l_float32 addc, l_float32 multc );
LEPT_DLL extern DPIX * dpixLinearCombination ( DPIX *dpixd, DPIX *dpixs1, DPIX *dpixs2, l_float32 a, l_float32 b );
LEPT_DLL extern l_ok dpixAddMultConstant ( DPIX *dpix, l_float64 addc, l_float64 multc );
LEPT_DLL extern l_ok fpixSetAllArbitrary ( FPIX *fpix, l_float32 inval );
LEPT_DLL extern l_ok dpixSetAllArbitrary ( DPIX *dpix, l_float64 inval );
LEPT_DLL extern FPIX * fpixAddBorder ( FPIX *fpixs, l_int32 left, l_int32 right, l_int32 top, l_int32 bot );
LEPT_DLL extern FPIX * fpixRemoveBorder ( FPIX *fpixs, l_int32 left, l_int32 right, l_int32 top, l_int32 bot );
LEPT_DLL extern FPIX * fpixAddMirroredBorder ( FPIX *fpixs, l_int32 left, l_int32 right, l_int32 top, l_int32 bot );
LEPT_DLL extern FPIX * fpixAddContinuedBorder ( FPIX *fpixs, l_int32 left, l_int32 right, l_int32 top, l_int32 bot );
LEPT_DLL extern FPIX * fpixAddSlopeBorder ( FPIX *fpixs, l_int32 left, l_int32 right, l_int32 top, l_int32 bot );
LEPT_DLL extern l_ok fpixRasterop ( FPIX *fpixd, l_int32 dx, l_int32 dy, l_int32 dw, l_int32 dh, FPIX *fpixs, l_int32 sx, l_int32 sy );
LEPT_DLL extern FPIX * fpixRotateOrth ( FPIX *fpixs, l_int32 quads );
LEPT_DLL extern FPIX * fpixRotate180 ( FPIX *fpixd, FPIX *fpixs );
LEPT_DLL extern FPIX * fpixRotate90 ( FPIX *fpixs, l_int32 direction );
LEPT_DLL extern FPIX * fpixFlipLR ( FPIX *fpixd, FPIX *fpixs );
LEPT_DLL extern FPIX * fpixFlipTB ( FPIX *fpixd, FPIX *fpixs );
LEPT_DLL extern FPIX * fpixAffinePta ( FPIX *fpixs, PTA *ptad, PTA *ptas, l_int32 border, l_float32 inval );
LEPT_DLL extern FPIX * fpixAffine ( FPIX *fpixs, l_float32 *vc, l_float32 inval );
LEPT_DLL extern FPIX * fpixProjectivePta ( FPIX *fpixs, PTA *ptad, PTA *ptas, l_int32 border, l_float32 inval );
LEPT_DLL extern FPIX * fpixProjective ( FPIX *fpixs, l_float32 *vc, l_float32 inval );
LEPT_DLL extern l_ok linearInterpolatePixelFloat ( l_float32 *datas, l_int32 w, l_int32 h, l_float32 x, l_float32 y, l_float32 inval, l_float32 *pval );
LEPT_DLL extern PIX * fpixThresholdToPix ( FPIX *fpix, l_float32 thresh );
LEPT_DLL extern FPIX * pixComponentFunction ( PIX *pix, l_float32 rnum, l_float32 gnum, l_float32 bnum, l_float32 rdenom, l_float32 gdenom, l_float32 bdenom );
LEPT_DLL extern PIX * pixReadStreamGif ( FILE *fp );
LEPT_DLL extern PIX * pixReadMemGif ( const l_uint8 *cdata, size_t size );
LEPT_DLL extern l_ok pixWriteStreamGif ( FILE *fp, PIX *pix );
LEPT_DLL extern l_ok pixWriteMemGif ( l_uint8 **pdata, size_t *psize, PIX *pix );
LEPT_DLL extern GPLOT * gplotCreate ( const char *rootname, l_int32 outformat, const char *title, const char *xlabel, const char *ylabel );
LEPT_DLL extern void gplotDestroy ( GPLOT **pgplot );
LEPT_DLL extern l_ok gplotAddPlot ( GPLOT *gplot, NUMA *nax, NUMA *nay, l_int32 plotstyle, const char *plotlabel );
LEPT_DLL extern l_ok gplotSetScaling ( GPLOT *gplot, l_int32 scaling );
LEPT_DLL extern PIX * gplotMakeOutputPix ( GPLOT *gplot );
LEPT_DLL extern l_ok gplotMakeOutput ( GPLOT *gplot );
LEPT_DLL extern l_ok gplotGenCommandFile ( GPLOT *gplot );
LEPT_DLL extern l_ok gplotGenDataFiles ( GPLOT *gplot );
LEPT_DLL extern l_ok gplotSimple1 ( NUMA *na, l_int32 outformat, const char *outroot, const char *title );
LEPT_DLL extern l_ok gplotSimple2 ( NUMA *na1, NUMA *na2, l_int32 outformat, const char *outroot, const char *title );
LEPT_DLL extern l_ok gplotSimpleN ( NUMAA *naa, l_int32 outformat, const char *outroot, const char *title );
LEPT_DLL extern PIX * gplotSimplePix1 ( NUMA *na, const char *title );
LEPT_DLL extern PIX * gplotSimplePix2 ( NUMA *na1, NUMA *na2, const char *title );
LEPT_DLL extern PIX * gplotSimplePixN ( NUMAA *naa, const char *title );
LEPT_DLL extern GPLOT * gplotSimpleXY1 ( NUMA *nax, NUMA *nay, l_int32 plotstyle, l_int32 outformat, const char *outroot, const char *title );
LEPT_DLL extern GPLOT * gplotSimpleXY2 ( NUMA *nax, NUMA *nay1, NUMA *nay2, l_int32 plotstyle, l_int32 outformat, const char *outroot, const char *title );
LEPT_DLL extern GPLOT * gplotSimpleXYN ( NUMA *nax, NUMAA *naay, l_int32 plotstyle, l_int32 outformat, const char *outroot, const char *title );
LEPT_DLL extern PIX * gplotGeneralPix1 ( NUMA *na, l_int32 plotstyle, const char *rootname, const char *title, const char *xlabel, const char *ylabel );
LEPT_DLL extern PIX * gplotGeneralPix2 ( NUMA *na1, NUMA *na2, l_int32 plotstyle, const char *rootname, const char *title, const char *xlabel, const char *ylabel );
LEPT_DLL extern PIX * gplotGeneralPixN ( NUMA *nax, NUMAA *naay, l_int32 plotstyle, const char *rootname, const char *title, const char *xlabel, const char *ylabel );
LEPT_DLL extern GPLOT * gplotRead ( const char *filename );
LEPT_DLL extern l_ok gplotWrite ( const char *filename, GPLOT *gplot );
LEPT_DLL extern PTA * generatePtaLine ( l_int32 x1, l_int32 y1, l_int32 x2, l_int32 y2 );
LEPT_DLL extern PTA * generatePtaWideLine ( l_int32 x1, l_int32 y1, l_int32 x2, l_int32 y2, l_int32 width );
LEPT_DLL extern PTA * generatePtaBox ( BOX *box, l_int32 width );
LEPT_DLL extern PTA * generatePtaBoxa ( BOXA *boxa, l_int32 width, l_int32 removedups );
LEPT_DLL extern PTA * generatePtaHashBox ( BOX *box, l_int32 spacing, l_int32 width, l_int32 orient, l_int32 outline );
LEPT_DLL extern PTA * generatePtaHashBoxa ( BOXA *boxa, l_int32 spacing, l_int32 width, l_int32 orient, l_int32 outline, l_int32 removedups );
LEPT_DLL extern PTAA * generatePtaaBoxa ( BOXA *boxa );
LEPT_DLL extern PTAA * generatePtaaHashBoxa ( BOXA *boxa, l_int32 spacing, l_int32 width, l_int32 orient, l_int32 outline );
LEPT_DLL extern PTA * generatePtaPolyline ( PTA *ptas, l_int32 width, l_int32 closeflag, l_int32 removedups );
LEPT_DLL extern PTA * generatePtaGrid ( l_int32 w, l_int32 h, l_int32 nx, l_int32 ny, l_int32 width );
LEPT_DLL extern PTA * convertPtaLineTo4cc ( PTA *ptas );
LEPT_DLL extern PTA * generatePtaFilledCircle ( l_int32 radius );
LEPT_DLL extern PTA * generatePtaFilledSquare ( l_int32 side );
LEPT_DLL extern PTA * generatePtaLineFromPt ( l_int32 x, l_int32 y, l_float64 length, l_float64 radang );
LEPT_DLL extern l_ok locatePtRadially ( l_int32 xr, l_int32 yr, l_float64 dist, l_float64 radang, l_float64 *px, l_float64 *py );
LEPT_DLL extern l_ok pixRenderPlotFromNuma ( PIX **ppix, NUMA *na, l_int32 plotloc, l_int32 linewidth, l_int32 max, l_uint32 color );
LEPT_DLL extern PTA * makePlotPtaFromNuma ( NUMA *na, l_int32 size, l_int32 plotloc, l_int32 linewidth, l_int32 max );
LEPT_DLL extern l_ok pixRenderPlotFromNumaGen ( PIX **ppix, NUMA *na, l_int32 orient, l_int32 linewidth, l_int32 refpos, l_int32 max, l_int32 drawref, l_uint32 color );
LEPT_DLL extern PTA * makePlotPtaFromNumaGen ( NUMA *na, l_int32 orient, l_int32 linewidth, l_int32 refpos, l_int32 max, l_int32 drawref );
LEPT_DLL extern l_ok pixRenderPta ( PIX *pix, PTA *pta, l_int32 op );
LEPT_DLL extern l_ok pixRenderPtaArb ( PIX *pix, PTA *pta, l_uint8 rval, l_uint8 gval, l_uint8 bval );
LEPT_DLL extern l_ok pixRenderPtaBlend ( PIX *pix, PTA *pta, l_uint8 rval, l_uint8 gval, l_uint8 bval, l_float32 fract );
LEPT_DLL extern l_ok pixRenderLine ( PIX *pix, l_int32 x1, l_int32 y1, l_int32 x2, l_int32 y2, l_int32 width, l_int32 op );
LEPT_DLL extern l_ok pixRenderLineArb ( PIX *pix, l_int32 x1, l_int32 y1, l_int32 x2, l_int32 y2, l_int32 width, l_uint8 rval, l_uint8 gval, l_uint8 bval );
LEPT_DLL extern l_ok pixRenderLineBlend ( PIX *pix, l_int32 x1, l_int32 y1, l_int32 x2, l_int32 y2, l_int32 width, l_uint8 rval, l_uint8 gval, l_uint8 bval, l_float32 fract );
LEPT_DLL extern l_ok pixRenderBox ( PIX *pix, BOX *box, l_int32 width, l_int32 op );
LEPT_DLL extern l_ok pixRenderBoxArb ( PIX *pix, BOX *box, l_int32 width, l_uint8 rval, l_uint8 gval, l_uint8 bval );
LEPT_DLL extern l_ok pixRenderBoxBlend ( PIX *pix, BOX *box, l_int32 width, l_uint8 rval, l_uint8 gval, l_uint8 bval, l_float32 fract );
LEPT_DLL extern l_ok pixRenderBoxa ( PIX *pix, BOXA *boxa, l_int32 width, l_int32 op );
LEPT_DLL extern l_ok pixRenderBoxaArb ( PIX *pix, BOXA *boxa, l_int32 width, l_uint8 rval, l_uint8 gval, l_uint8 bval );
LEPT_DLL extern l_ok pixRenderBoxaBlend ( PIX *pix, BOXA *boxa, l_int32 width, l_uint8 rval, l_uint8 gval, l_uint8 bval, l_float32 fract, l_int32 removedups );
LEPT_DLL extern l_ok pixRenderHashBox ( PIX *pix, BOX *box, l_int32 spacing, l_int32 width, l_int32 orient, l_int32 outline, l_int32 op );
LEPT_DLL extern l_ok pixRenderHashBoxArb ( PIX *pix, BOX *box, l_int32 spacing, l_int32 width, l_int32 orient, l_int32 outline, l_int32 rval, l_int32 gval, l_int32 bval );
LEPT_DLL extern l_ok pixRenderHashBoxBlend ( PIX *pix, BOX *box, l_int32 spacing, l_int32 width, l_int32 orient, l_int32 outline, l_int32 rval, l_int32 gval, l_int32 bval, l_float32 fract );
LEPT_DLL extern l_ok pixRenderHashMaskArb ( PIX *pix, PIX *pixm, l_int32 x, l_int32 y, l_int32 spacing, l_int32 width, l_int32 orient, l_int32 outline, l_int32 rval, l_int32 gval, l_int32 bval );
LEPT_DLL extern l_ok pixRenderHashBoxa ( PIX *pix, BOXA *boxa, l_int32 spacing, l_int32 width, l_int32 orient, l_int32 outline, l_int32 op );
LEPT_DLL extern l_ok pixRenderHashBoxaArb ( PIX *pix, BOXA *boxa, l_int32 spacing, l_int32 width, l_int32 orient, l_int32 outline, l_int32 rval, l_int32 gval, l_int32 bval );
LEPT_DLL extern l_ok pixRenderHashBoxaBlend ( PIX *pix, BOXA *boxa, l_int32 spacing, l_int32 width, l_int32 orient, l_int32 outline, l_int32 rval, l_int32 gval, l_int32 bval, l_float32 fract );
LEPT_DLL extern l_ok pixRenderPolyline ( PIX *pix, PTA *ptas, l_int32 width, l_int32 op, l_int32 closeflag );
LEPT_DLL extern l_ok pixRenderPolylineArb ( PIX *pix, PTA *ptas, l_int32 width, l_uint8 rval, l_uint8 gval, l_uint8 bval, l_int32 closeflag );
LEPT_DLL extern l_ok pixRenderPolylineBlend ( PIX *pix, PTA *ptas, l_int32 width, l_uint8 rval, l_uint8 gval, l_uint8 bval, l_float32 fract, l_int32 closeflag, l_int32 removedups );
LEPT_DLL extern l_ok pixRenderGridArb ( PIX *pix, l_int32 nx, l_int32 ny, l_int32 width, l_uint8 rval, l_uint8 gval, l_uint8 bval );
LEPT_DLL extern PIX * pixRenderRandomCmapPtaa ( PIX *pix, PTAA *ptaa, l_int32 polyflag, l_int32 width, l_int32 closeflag );
LEPT_DLL extern PIX * pixRenderPolygon ( PTA *ptas, l_int32 width, l_int32 *pxmin, l_int32 *pymin );
LEPT_DLL extern PIX * pixFillPolygon ( PIX *pixs, PTA *pta, l_int32 xmin, l_int32 ymin );
LEPT_DLL extern PIX * pixRenderContours ( PIX *pixs, l_int32 startval, l_int32 incr, l_int32 outdepth );
LEPT_DLL extern PIX * fpixAutoRenderContours ( FPIX *fpix, l_int32 ncontours );
LEPT_DLL extern PIX * fpixRenderContours ( FPIX *fpixs, l_float32 incr, l_float32 proxim );
LEPT_DLL extern PTA * pixGeneratePtaBoundary ( PIX *pixs, l_int32 width );
LEPT_DLL extern PIX * pixErodeGray ( PIX *pixs, l_int32 hsize, l_int32 vsize );
LEPT_DLL extern PIX * pixDilateGray ( PIX *pixs, l_int32 hsize, l_int32 vsize );
LEPT_DLL extern PIX * pixOpenGray ( PIX *pixs, l_int32 hsize, l_int32 vsize );
LEPT_DLL extern PIX * pixCloseGray ( PIX *pixs, l_int32 hsize, l_int32 vsize );
LEPT_DLL extern PIX * pixErodeGray3 ( PIX *pixs, l_int32 hsize, l_int32 vsize );
LEPT_DLL extern PIX * pixDilateGray3 ( PIX *pixs, l_int32 hsize, l_int32 vsize );
LEPT_DLL extern PIX * pixOpenGray3 ( PIX *pixs, l_int32 hsize, l_int32 vsize );
LEPT_DLL extern PIX * pixCloseGray3 ( PIX *pixs, l_int32 hsize, l_int32 vsize );
LEPT_DLL extern PIX * pixDitherToBinary ( PIX *pixs );
LEPT_DLL extern PIX * pixDitherToBinarySpec ( PIX *pixs, l_int32 lowerclip, l_int32 upperclip );
LEPT_DLL extern void ditherToBinaryLineLow ( l_uint32 *lined, l_int32 w, l_uint32 *bufs1, l_uint32 *bufs2, l_int32 lowerclip, l_int32 upperclip, l_int32 lastlineflag );
LEPT_DLL extern PIX * pixThresholdToBinary ( PIX *pixs, l_int32 thresh );
LEPT_DLL extern void thresholdToBinaryLineLow ( l_uint32 *lined, l_int32 w, l_uint32 *lines, l_int32 d, l_int32 thresh );
LEPT_DLL extern PIX * pixVarThresholdToBinary ( PIX *pixs, PIX *pixg );
LEPT_DLL extern PIX * pixAdaptThresholdToBinary ( PIX *pixs, PIX *pixm, l_float32 gamma );
LEPT_DLL extern PIX * pixAdaptThresholdToBinaryGen ( PIX *pixs, PIX *pixm, l_float32 gamma, l_int32 blackval, l_int32 whiteval, l_int32 thresh );
LEPT_DLL extern PIX * pixGenerateMaskByValue ( PIX *pixs, l_int32 val, l_int32 usecmap );
LEPT_DLL extern PIX * pixGenerateMaskByBand ( PIX *pixs, l_int32 lower, l_int32 upper, l_int32 inband, l_int32 usecmap );
LEPT_DLL extern PIX * pixDitherTo2bpp ( PIX *pixs, l_int32 cmapflag );
LEPT_DLL extern PIX * pixDitherTo2bppSpec ( PIX *pixs, l_int32 lowerclip, l_int32 upperclip, l_int32 cmapflag );
LEPT_DLL extern PIX * pixThresholdTo2bpp ( PIX *pixs, l_int32 nlevels, l_int32 cmapflag );
LEPT_DLL extern PIX * pixThresholdTo4bpp ( PIX *pixs, l_int32 nlevels, l_int32 cmapflag );
LEPT_DLL extern PIX * pixThresholdOn8bpp ( PIX *pixs, l_int32 nlevels, l_int32 cmapflag );
LEPT_DLL extern PIX * pixThresholdGrayArb ( PIX *pixs, const char *edgevals, l_int32 outdepth, l_int32 use_average, l_int32 setblack, l_int32 setwhite );
LEPT_DLL extern l_int32 * makeGrayQuantIndexTable ( l_int32 nlevels );
LEPT_DLL extern l_ok makeGrayQuantTableArb ( NUMA *na, l_int32 outdepth, l_int32 **ptab, PIXCMAP **pcmap );
LEPT_DLL extern PIX * pixGenerateMaskByBand32 ( PIX *pixs, l_uint32 refval, l_int32 delm, l_int32 delp, l_float32 fractm, l_float32 fractp );
LEPT_DLL extern PIX * pixGenerateMaskByDiscr32 ( PIX *pixs, l_uint32 refval1, l_uint32 refval2, l_int32 distflag );
LEPT_DLL extern PIX * pixGrayQuantFromHisto ( PIX *pixd, PIX *pixs, PIX *pixm, l_float32 minfract, l_int32 maxsize );
LEPT_DLL extern PIX * pixGrayQuantFromCmap ( PIX *pixs, PIXCMAP *cmap, l_int32 mindepth );
LEPT_DLL extern L_HEAP * lheapCreate ( l_int32 n, l_int32 direction );
LEPT_DLL extern void lheapDestroy ( L_HEAP **plh, l_int32 freeflag );
LEPT_DLL extern l_ok lheapAdd ( L_HEAP *lh, void *item );
LEPT_DLL extern void * lheapRemove ( L_HEAP *lh );
LEPT_DLL extern l_int32 lheapGetCount ( L_HEAP *lh );
LEPT_DLL extern void * lheapGetElement ( L_HEAP *lh, l_int32 index );
LEPT_DLL extern l_ok lheapSort ( L_HEAP *lh );
LEPT_DLL extern l_ok lheapSortStrictOrder ( L_HEAP *lh );
LEPT_DLL extern l_ok lheapPrint ( FILE *fp, L_HEAP *lh );
LEPT_DLL extern JBCLASSER * jbRankHausInit ( l_int32 components, l_int32 maxwidth, l_int32 maxheight, l_int32 size, l_float32 rank );
LEPT_DLL extern JBCLASSER * jbCorrelationInit ( l_int32 components, l_int32 maxwidth, l_int32 maxheight, l_float32 thresh, l_float32 weightfactor );
LEPT_DLL extern JBCLASSER * jbCorrelationInitWithoutComponents ( l_int32 components, l_int32 maxwidth, l_int32 maxheight, l_float32 thresh, l_float32 weightfactor );
LEPT_DLL extern l_ok jbAddPages ( JBCLASSER *classer, SARRAY *safiles );
LEPT_DLL extern l_ok jbAddPage ( JBCLASSER *classer, PIX *pixs );
LEPT_DLL extern l_ok jbAddPageComponents ( JBCLASSER *classer, PIX *pixs, BOXA *boxas, PIXA *pixas );
LEPT_DLL extern l_ok jbClassifyRankHaus ( JBCLASSER *classer, BOXA *boxa, PIXA *pixas );
LEPT_DLL extern l_int32 pixHaustest ( PIX *pix1, PIX *pix2, PIX *pix3, PIX *pix4, l_float32 delx, l_float32 dely, l_int32 maxdiffw, l_int32 maxdiffh );
LEPT_DLL extern l_int32 pixRankHaustest ( PIX *pix1, PIX *pix2, PIX *pix3, PIX *pix4, l_float32 delx, l_float32 dely, l_int32 maxdiffw, l_int32 maxdiffh, l_int32 area1, l_int32 area3, l_float32 rank, l_int32 *tab8 );
LEPT_DLL extern l_ok jbClassifyCorrelation ( JBCLASSER *classer, BOXA *boxa, PIXA *pixas );
LEPT_DLL extern l_ok jbGetComponents ( PIX *pixs, l_int32 components, l_int32 maxwidth, l_int32 maxheight, BOXA **pboxad, PIXA **ppixad );
LEPT_DLL extern l_ok pixWordMaskByDilation ( PIX *pixs, PIX **ppixm, l_int32 *psize, PIXA *pixadb );
LEPT_DLL extern l_ok pixWordBoxesByDilation ( PIX *pixs, l_int32 minwidth, l_int32 minheight, l_int32 maxwidth, l_int32 maxheight, BOXA **pboxa, l_int32 *psize, PIXA *pixadb );
LEPT_DLL extern PIXA * jbAccumulateComposites ( PIXAA *pixaa, NUMA **pna, PTA **pptat );
LEPT_DLL extern PIXA * jbTemplatesFromComposites ( PIXA *pixac, NUMA *na );
LEPT_DLL extern JBCLASSER * jbClasserCreate ( l_int32 method, l_int32 components );
LEPT_DLL extern void jbClasserDestroy ( JBCLASSER **pclasser );
LEPT_DLL extern JBDATA * jbDataSave ( JBCLASSER *classer );
LEPT_DLL extern void jbDataDestroy ( JBDATA **pdata );
LEPT_DLL extern l_ok jbDataWrite ( const char *rootout, JBDATA *jbdata );
LEPT_DLL extern JBDATA * jbDataRead ( const char *rootname );
LEPT_DLL extern PIXA * jbDataRender ( JBDATA *data, l_int32 debugflag );
LEPT_DLL extern l_ok jbGetULCorners ( JBCLASSER *classer, PIX *pixs, BOXA *boxa );
LEPT_DLL extern l_ok jbGetLLCorners ( JBCLASSER *classer );
LEPT_DLL extern l_ok readHeaderJp2k ( const char *filename, l_int32 *pw, l_int32 *ph, l_int32 *pbps, l_int32 *pspp );
LEPT_DLL extern l_ok freadHeaderJp2k ( FILE *fp, l_int32 *pw, l_int32 *ph, l_int32 *pbps, l_int32 *pspp );
LEPT_DLL extern l_ok readHeaderMemJp2k ( const l_uint8 *data, size_t size, l_int32 *pw, l_int32 *ph, l_int32 *pbps, l_int32 *pspp );
LEPT_DLL extern l_int32 fgetJp2kResolution ( FILE *fp, l_int32 *pxres, l_int32 *pyres );
LEPT_DLL extern PIX * pixReadJp2k ( const char *filename, l_uint32 reduction, BOX *box, l_int32 hint, l_int32 debug );
LEPT_DLL extern PIX * pixReadStreamJp2k ( FILE *fp, l_uint32 reduction, BOX *box, l_int32 hint, l_int32 debug );
LEPT_DLL extern l_ok pixWriteJp2k ( const char *filename, PIX *pix, l_int32 quality, l_int32 nlevels, l_int32 hint, l_int32 debug );
LEPT_DLL extern l_ok pixWriteStreamJp2k ( FILE *fp, PIX *pix, l_int32 quality, l_int32 nlevels, l_int32 hint, l_int32 debug );
LEPT_DLL extern PIX * pixReadMemJp2k ( const l_uint8 *data, size_t size, l_uint32 reduction, BOX *box, l_int32 hint, l_int32 debug );
LEPT_DLL extern l_ok pixWriteMemJp2k ( l_uint8 **pdata, size_t *psize, PIX *pix, l_int32 quality, l_int32 nlevels, l_int32 hint, l_int32 debug );
LEPT_DLL extern PIX * pixReadJpeg ( const char *filename, l_int32 cmapflag, l_int32 reduction, l_int32 *pnwarn, l_int32 hint );
LEPT_DLL extern PIX * pixReadStreamJpeg ( FILE *fp, l_int32 cmapflag, l_int32 reduction, l_int32 *pnwarn, l_int32 hint );
LEPT_DLL extern l_ok readHeaderJpeg ( const char *filename, l_int32 *pw, l_int32 *ph, l_int32 *pspp, l_int32 *pycck, l_int32 *pcmyk );
LEPT_DLL extern l_ok freadHeaderJpeg ( FILE *fp, l_int32 *pw, l_int32 *ph, l_int32 *pspp, l_int32 *pycck, l_int32 *pcmyk );
LEPT_DLL extern l_int32 fgetJpegResolution ( FILE *fp, l_int32 *pxres, l_int32 *pyres );
LEPT_DLL extern l_int32 fgetJpegComment ( FILE *fp, l_uint8 **pcomment );
LEPT_DLL extern l_ok pixWriteJpeg ( const char *filename, PIX *pix, l_int32 quality, l_int32 progressive );
LEPT_DLL extern l_ok pixWriteStreamJpeg ( FILE *fp, PIX *pixs, l_int32 quality, l_int32 progressive );
LEPT_DLL extern PIX * pixReadMemJpeg ( const l_uint8 *data, size_t size, l_int32 cmflag, l_int32 reduction, l_int32 *pnwarn, l_int32 hint );
LEPT_DLL extern l_ok readHeaderMemJpeg ( const l_uint8 *data, size_t size, l_int32 *pw, l_int32 *ph, l_int32 *pspp, l_int32 *pycck, l_int32 *pcmyk );
LEPT_DLL extern l_ok readResolutionMemJpeg ( const l_uint8 *data, size_t size, l_int32 *pxres, l_int32 *pyres );
LEPT_DLL extern l_ok pixWriteMemJpeg ( l_uint8 **pdata, size_t *psize, PIX *pix, l_int32 quality, l_int32 progressive );
LEPT_DLL extern l_ok pixSetChromaSampling ( PIX *pix, l_int32 sampling );
LEPT_DLL extern L_KERNEL * kernelCreate ( l_int32 height, l_int32 width );
LEPT_DLL extern void kernelDestroy ( L_KERNEL **pkel );
LEPT_DLL extern L_KERNEL * kernelCopy ( L_KERNEL *kels );
LEPT_DLL extern l_ok kernelGetElement ( L_KERNEL *kel, l_int32 row, l_int32 col, l_float32 *pval );
LEPT_DLL extern l_ok kernelSetElement ( L_KERNEL *kel, l_int32 row, l_int32 col, l_float32 val );
LEPT_DLL extern l_ok kernelGetParameters ( L_KERNEL *kel, l_int32 *psy, l_int32 *psx, l_int32 *pcy, l_int32 *pcx );
LEPT_DLL extern l_ok kernelSetOrigin ( L_KERNEL *kel, l_int32 cy, l_int32 cx );
LEPT_DLL extern l_ok kernelGetSum ( L_KERNEL *kel, l_float32 *psum );
LEPT_DLL extern l_ok kernelGetMinMax ( L_KERNEL *kel, l_float32 *pmin, l_float32 *pmax );
LEPT_DLL extern L_KERNEL * kernelNormalize ( L_KERNEL *kels, l_float32 normsum );
LEPT_DLL extern L_KERNEL * kernelInvert ( L_KERNEL *kels );
LEPT_DLL extern l_float32 ** create2dFloatArray ( l_int32 sy, l_int32 sx );
LEPT_DLL extern L_KERNEL * kernelRead ( const char *fname );
LEPT_DLL extern L_KERNEL * kernelReadStream ( FILE *fp );
LEPT_DLL extern l_ok kernelWrite ( const char *fname, L_KERNEL *kel );
LEPT_DLL extern l_ok kernelWriteStream ( FILE *fp, L_KERNEL *kel );
LEPT_DLL extern L_KERNEL * kernelCreateFromString ( l_int32 h, l_int32 w, l_int32 cy, l_int32 cx, const char *kdata );
LEPT_DLL extern L_KERNEL * kernelCreateFromFile ( const char *filename );
LEPT_DLL extern L_KERNEL * kernelCreateFromPix ( PIX *pix, l_int32 cy, l_int32 cx );
LEPT_DLL extern PIX * kernelDisplayInPix ( L_KERNEL *kel, l_int32 size, l_int32 gthick );
LEPT_DLL extern NUMA * parseStringForNumbers ( const char *str, const char *seps );
LEPT_DLL extern L_KERNEL * makeFlatKernel ( l_int32 height, l_int32 width, l_int32 cy, l_int32 cx );
LEPT_DLL extern L_KERNEL * makeGaussianKernel ( l_int32 halfh, l_int32 halfw, l_float32 stdev, l_float32 max );
LEPT_DLL extern l_ok makeGaussianKernelSep ( l_int32 halfh, l_int32 halfw, l_float32 stdev, l_float32 max, L_KERNEL **pkelx, L_KERNEL **pkely );
LEPT_DLL extern L_KERNEL * makeDoGKernel ( l_int32 halfh, l_int32 halfw, l_float32 stdev, l_float32 ratio );
LEPT_DLL extern char * getImagelibVersions ( void );
LEPT_DLL extern void listDestroy ( DLLIST **phead );
LEPT_DLL extern l_ok listAddToHead ( DLLIST **phead, void *data );
LEPT_DLL extern l_ok listAddToTail ( DLLIST **phead, DLLIST **ptail, void *data );
LEPT_DLL extern l_ok listInsertBefore ( DLLIST **phead, DLLIST *elem, void *data );
LEPT_DLL extern l_ok listInsertAfter ( DLLIST **phead, DLLIST *elem, void *data );
LEPT_DLL extern void * listRemoveElement ( DLLIST **phead, DLLIST *elem );
LEPT_DLL extern void * listRemoveFromHead ( DLLIST **phead );
LEPT_DLL extern void * listRemoveFromTail ( DLLIST **phead, DLLIST **ptail );
LEPT_DLL extern DLLIST * listFindElement ( DLLIST *head, void *data );
LEPT_DLL extern DLLIST * listFindTail ( DLLIST *head );
LEPT_DLL extern l_int32 listGetCount ( DLLIST *head );
LEPT_DLL extern l_ok listReverse ( DLLIST **phead );
LEPT_DLL extern l_ok listJoin ( DLLIST **phead1, DLLIST **phead2 );
LEPT_DLL extern L_AMAP * l_amapCreate ( l_int32 keytype );
LEPT_DLL extern RB_TYPE * l_amapFind ( L_AMAP *m, RB_TYPE key );
LEPT_DLL extern void l_amapInsert ( L_AMAP *m, RB_TYPE key, RB_TYPE value );
LEPT_DLL extern void l_amapDelete ( L_AMAP *m, RB_TYPE key );
LEPT_DLL extern void l_amapDestroy ( L_AMAP **pm );
LEPT_DLL extern L_AMAP_NODE * l_amapGetFirst ( L_AMAP *m );
LEPT_DLL extern L_AMAP_NODE * l_amapGetNext ( L_AMAP_NODE *n );
LEPT_DLL extern L_AMAP_NODE * l_amapGetLast ( L_AMAP *m );
LEPT_DLL extern L_AMAP_NODE * l_amapGetPrev ( L_AMAP_NODE *n );
LEPT_DLL extern l_int32 l_amapSize ( L_AMAP *m );
LEPT_DLL extern L_ASET * l_asetCreate ( l_int32 keytype );
LEPT_DLL extern RB_TYPE * l_asetFind ( L_ASET *s, RB_TYPE key );
LEPT_DLL extern void l_asetInsert ( L_ASET *s, RB_TYPE key );
LEPT_DLL extern void l_asetDelete ( L_ASET *s, RB_TYPE key );
LEPT_DLL extern void l_asetDestroy ( L_ASET **ps );
LEPT_DLL extern L_ASET_NODE * l_asetGetFirst ( L_ASET *s );
LEPT_DLL extern L_ASET_NODE * l_asetGetNext ( L_ASET_NODE *n );
LEPT_DLL extern L_ASET_NODE * l_asetGetLast ( L_ASET *s );
LEPT_DLL extern L_ASET_NODE * l_asetGetPrev ( L_ASET_NODE *n );
LEPT_DLL extern l_int32 l_asetSize ( L_ASET *s );
LEPT_DLL extern PIX * generateBinaryMaze ( l_int32 w, l_int32 h, l_int32 xi, l_int32 yi, l_float32 wallps, l_float32 ranis );
LEPT_DLL extern PTA * pixSearchBinaryMaze ( PIX *pixs, l_int32 xi, l_int32 yi, l_int32 xf, l_int32 yf, PIX **ppixd );
LEPT_DLL extern PTA * pixSearchGrayMaze ( PIX *pixs, l_int32 xi, l_int32 yi, l_int32 xf, l_int32 yf, PIX **ppixd );
LEPT_DLL extern PIX * pixDilate ( PIX *pixd, PIX *pixs, SEL *sel );
LEPT_DLL extern PIX * pixErode ( PIX *pixd, PIX *pixs, SEL *sel );
LEPT_DLL extern PIX * pixHMT ( PIX *pixd, PIX *pixs, SEL *sel );
LEPT_DLL extern PIX * pixOpen ( PIX *pixd, PIX *pixs, SEL *sel );
LEPT_DLL extern PIX * pixClose ( PIX *pixd, PIX *pixs, SEL *sel );
LEPT_DLL extern PIX * pixCloseSafe ( PIX *pixd, PIX *pixs, SEL *sel );
LEPT_DLL extern PIX * pixOpenGeneralized ( PIX *pixd, PIX *pixs, SEL *sel );
LEPT_DLL extern PIX * pixCloseGeneralized ( PIX *pixd, PIX *pixs, SEL *sel );
LEPT_DLL extern PIX * pixDilateBrick ( PIX *pixd, PIX *pixs, l_int32 hsize, l_int32 vsize );
LEPT_DLL extern PIX * pixErodeBrick ( PIX *pixd, PIX *pixs, l_int32 hsize, l_int32 vsize );
LEPT_DLL extern PIX * pixOpenBrick ( PIX *pixd, PIX *pixs, l_int32 hsize, l_int32 vsize );
LEPT_DLL extern PIX * pixCloseBrick ( PIX *pixd, PIX *pixs, l_int32 hsize, l_int32 vsize );
LEPT_DLL extern PIX * pixCloseSafeBrick ( PIX *pixd, PIX *pixs, l_int32 hsize, l_int32 vsize );
LEPT_DLL extern l_int32 selectComposableSels ( l_int32 size, l_int32 direction, SEL **psel1, SEL **psel2 );
LEPT_DLL extern l_ok selectComposableSizes ( l_int32 size, l_int32 *pfactor1, l_int32 *pfactor2 );
LEPT_DLL extern PIX * pixDilateCompBrick ( PIX *pixd, PIX *pixs, l_int32 hsize, l_int32 vsize );
LEPT_DLL extern PIX * pixErodeCompBrick ( PIX *pixd, PIX *pixs, l_int32 hsize, l_int32 vsize );
LEPT_DLL extern PIX * pixOpenCompBrick ( PIX *pixd, PIX *pixs, l_int32 hsize, l_int32 vsize );
LEPT_DLL extern PIX * pixCloseCompBrick ( PIX *pixd, PIX *pixs, l_int32 hsize, l_int32 vsize );
LEPT_DLL extern PIX * pixCloseSafeCompBrick ( PIX *pixd, PIX *pixs, l_int32 hsize, l_int32 vsize );
LEPT_DLL extern void resetMorphBoundaryCondition ( l_int32 bc );
LEPT_DLL extern l_uint32 getMorphBorderPixelColor ( l_int32 type, l_int32 depth );
LEPT_DLL extern PIX * pixExtractBoundary ( PIX *pixs, l_int32 type );
LEPT_DLL extern PIX * pixMorphSequenceMasked ( PIX *pixs, PIX *pixm, const char *sequence, l_int32 dispsep );
LEPT_DLL extern PIX * pixMorphSequenceByComponent ( PIX *pixs, const char *sequence, l_int32 connectivity, l_int32 minw, l_int32 minh, BOXA **pboxa );
LEPT_DLL extern PIXA * pixaMorphSequenceByComponent ( PIXA *pixas, const char *sequence, l_int32 minw, l_int32 minh );
LEPT_DLL extern PIX * pixMorphSequenceByRegion ( PIX *pixs, PIX *pixm, const char *sequence, l_int32 connectivity, l_int32 minw, l_int32 minh, BOXA **pboxa );
LEPT_DLL extern PIXA * pixaMorphSequenceByRegion ( PIX *pixs, PIXA *pixam, const char *sequence, l_int32 minw, l_int32 minh );
LEPT_DLL extern PIX * pixUnionOfMorphOps ( PIX *pixs, SELA *sela, l_int32 type );
LEPT_DLL extern PIX * pixIntersectionOfMorphOps ( PIX *pixs, SELA *sela, l_int32 type );
LEPT_DLL extern PIX * pixSelectiveConnCompFill ( PIX *pixs, l_int32 connectivity, l_int32 minw, l_int32 minh );
LEPT_DLL extern l_ok pixRemoveMatchedPattern ( PIX *pixs, PIX *pixp, PIX *pixe, l_int32 x0, l_int32 y0, l_int32 dsize );
LEPT_DLL extern PIX * pixDisplayMatchedPattern ( PIX *pixs, PIX *pixp, PIX *pixe, l_int32 x0, l_int32 y0, l_uint32 color, l_float32 scale, l_int32 nlevels );
LEPT_DLL extern PIXA * pixaExtendByMorph ( PIXA *pixas, l_int32 type, l_int32 niters, SEL *sel, l_int32 include );
LEPT_DLL extern PIXA * pixaExtendByScaling ( PIXA *pixas, NUMA *nasc, l_int32 type, l_int32 include );
LEPT_DLL extern PIX * pixSeedfillMorph ( PIX *pixs, PIX *pixm, l_int32 maxiters, l_int32 connectivity );
LEPT_DLL extern NUMA * pixRunHistogramMorph ( PIX *pixs, l_int32 runtype, l_int32 direction, l_int32 maxsize );
LEPT_DLL extern PIX * pixTophat ( PIX *pixs, l_int32 hsize, l_int32 vsize, l_int32 type );
LEPT_DLL extern PIX * pixHDome ( PIX *pixs, l_int32 height, l_int32 connectivity );
LEPT_DLL extern PIX * pixFastTophat ( PIX *pixs, l_int32 xsize, l_int32 ysize, l_int32 type );
LEPT_DLL extern PIX * pixMorphGradient ( PIX *pixs, l_int32 hsize, l_int32 vsize, l_int32 smoothing );
LEPT_DLL extern PTA * pixaCentroids ( PIXA *pixa );
LEPT_DLL extern l_ok pixCentroid ( PIX *pix, l_int32 *centtab, l_int32 *sumtab, l_float32 *pxave, l_float32 *pyave );
LEPT_DLL extern PIX * pixDilateBrickDwa ( PIX *pixd, PIX *pixs, l_int32 hsize, l_int32 vsize );
LEPT_DLL extern PIX * pixErodeBrickDwa ( PIX *pixd, PIX *pixs, l_int32 hsize, l_int32 vsize );
LEPT_DLL extern PIX * pixOpenBrickDwa ( PIX *pixd, PIX *pixs, l_int32 hsize, l_int32 vsize );
LEPT_DLL extern PIX * pixCloseBrickDwa ( PIX *pixd, PIX *pixs, l_int32 hsize, l_int32 vsize );
LEPT_DLL extern PIX * pixDilateCompBrickDwa ( PIX *pixd, PIX *pixs, l_int32 hsize, l_int32 vsize );
LEPT_DLL extern PIX * pixErodeCompBrickDwa ( PIX *pixd, PIX *pixs, l_int32 hsize, l_int32 vsize );
LEPT_DLL extern PIX * pixOpenCompBrickDwa ( PIX *pixd, PIX *pixs, l_int32 hsize, l_int32 vsize );
LEPT_DLL extern PIX * pixCloseCompBrickDwa ( PIX *pixd, PIX *pixs, l_int32 hsize, l_int32 vsize );
LEPT_DLL extern PIX * pixDilateCompBrickExtendDwa ( PIX *pixd, PIX *pixs, l_int32 hsize, l_int32 vsize );
LEPT_DLL extern PIX * pixErodeCompBrickExtendDwa ( PIX *pixd, PIX *pixs, l_int32 hsize, l_int32 vsize );
LEPT_DLL extern PIX * pixOpenCompBrickExtendDwa ( PIX *pixd, PIX *pixs, l_int32 hsize, l_int32 vsize );
LEPT_DLL extern PIX * pixCloseCompBrickExtendDwa ( PIX *pixd, PIX *pixs, l_int32 hsize, l_int32 vsize );
LEPT_DLL extern l_ok getExtendedCompositeParameters ( l_int32 size, l_int32 *pn, l_int32 *pextra, l_int32 *pactualsize );
LEPT_DLL extern PIX * pixMorphSequence ( PIX *pixs, const char *sequence, l_int32 dispsep );
LEPT_DLL extern PIX * pixMorphCompSequence ( PIX *pixs, const char *sequence, l_int32 dispsep );
LEPT_DLL extern PIX * pixMorphSequenceDwa ( PIX *pixs, const char *sequence, l_int32 dispsep );
LEPT_DLL extern PIX * pixMorphCompSequenceDwa ( PIX *pixs, const char *sequence, l_int32 dispsep );
LEPT_DLL extern l_int32 morphSequenceVerify ( SARRAY *sa );
LEPT_DLL extern PIX * pixGrayMorphSequence ( PIX *pixs, const char *sequence, l_int32 dispsep, l_int32 dispy );
LEPT_DLL extern PIX * pixColorMorphSequence ( PIX *pixs, const char *sequence, l_int32 dispsep, l_int32 dispy );
LEPT_DLL extern NUMA * numaCreate ( l_int32 n );
LEPT_DLL extern NUMA * numaCreateFromIArray ( l_int32 *iarray, l_int32 size );
LEPT_DLL extern NUMA * numaCreateFromFArray ( l_float32 *farray, l_int32 size, l_int32 copyflag );
LEPT_DLL extern NUMA * numaCreateFromString ( const char *str );
LEPT_DLL extern void numaDestroy ( NUMA **pna );
LEPT_DLL extern NUMA * numaCopy ( NUMA *na );
LEPT_DLL extern NUMA * numaClone ( NUMA *na );
LEPT_DLL extern l_ok numaEmpty ( NUMA *na );
LEPT_DLL extern l_ok numaAddNumber ( NUMA *na, l_float32 val );
LEPT_DLL extern l_ok numaInsertNumber ( NUMA *na, l_int32 index, l_float32 val );
LEPT_DLL extern l_ok numaRemoveNumber ( NUMA *na, l_int32 index );
LEPT_DLL extern l_ok numaReplaceNumber ( NUMA *na, l_int32 index, l_float32 val );
LEPT_DLL extern l_int32 numaGetCount ( NUMA *na );
LEPT_DLL extern l_ok numaSetCount ( NUMA *na, l_int32 newcount );
LEPT_DLL extern l_ok numaGetFValue ( NUMA *na, l_int32 index, l_float32 *pval );
LEPT_DLL extern l_ok numaGetIValue ( NUMA *na, l_int32 index, l_int32 *pival );
LEPT_DLL extern l_ok numaSetValue ( NUMA *na, l_int32 index, l_float32 val );
LEPT_DLL extern l_ok numaShiftValue ( NUMA *na, l_int32 index, l_float32 diff );
LEPT_DLL extern l_int32 * numaGetIArray ( NUMA *na );
LEPT_DLL extern l_float32 * numaGetFArray ( NUMA *na, l_int32 copyflag );
LEPT_DLL extern l_int32 numaGetRefcount ( NUMA *na );
LEPT_DLL extern l_ok numaChangeRefcount ( NUMA *na, l_int32 delta );
LEPT_DLL extern l_ok numaGetParameters ( NUMA *na, l_float32 *pstartx, l_float32 *pdelx );
LEPT_DLL extern l_ok numaSetParameters ( NUMA *na, l_float32 startx, l_float32 delx );
LEPT_DLL extern l_ok numaCopyParameters ( NUMA *nad, NUMA *nas );
LEPT_DLL extern SARRAY * numaConvertToSarray ( NUMA *na, l_int32 size1, l_int32 size2, l_int32 addzeros, l_int32 type );
LEPT_DLL extern NUMA * numaRead ( const char *filename );
LEPT_DLL extern NUMA * numaReadStream ( FILE *fp );
LEPT_DLL extern NUMA * numaReadMem ( const l_uint8 *data, size_t size );
LEPT_DLL extern l_ok numaWriteDebug ( const char *filename, NUMA *na );
LEPT_DLL extern l_ok numaWrite ( const char *filename, NUMA *na );
LEPT_DLL extern l_ok numaWriteStream ( FILE *fp, NUMA *na );
LEPT_DLL extern l_ok numaWriteStderr ( NUMA *na );
LEPT_DLL extern l_ok numaWriteMem ( l_uint8 **pdata, size_t *psize, NUMA *na );
LEPT_DLL extern NUMAA * numaaCreate ( l_int32 n );
LEPT_DLL extern NUMAA * numaaCreateFull ( l_int32 nptr, l_int32 n );
LEPT_DLL extern l_ok numaaTruncate ( NUMAA *naa );
LEPT_DLL extern void numaaDestroy ( NUMAA **pnaa );
LEPT_DLL extern l_ok numaaAddNuma ( NUMAA *naa, NUMA *na, l_int32 copyflag );
LEPT_DLL extern l_int32 numaaGetCount ( NUMAA *naa );
LEPT_DLL extern l_int32 numaaGetNumaCount ( NUMAA *naa, l_int32 index );
LEPT_DLL extern l_int32 numaaGetNumberCount ( NUMAA *naa );
LEPT_DLL extern NUMA ** numaaGetPtrArray ( NUMAA *naa );
LEPT_DLL extern NUMA * numaaGetNuma ( NUMAA *naa, l_int32 index, l_int32 accessflag );
LEPT_DLL extern l_ok numaaReplaceNuma ( NUMAA *naa, l_int32 index, NUMA *na );
LEPT_DLL extern l_ok numaaGetValue ( NUMAA *naa, l_int32 i, l_int32 j, l_float32 *pfval, l_int32 *pival );
LEPT_DLL extern l_ok numaaAddNumber ( NUMAA *naa, l_int32 index, l_float32 val );
LEPT_DLL extern NUMAA * numaaRead ( const char *filename );
LEPT_DLL extern NUMAA * numaaReadStream ( FILE *fp );
LEPT_DLL extern NUMAA * numaaReadMem ( const l_uint8 *data, size_t size );
LEPT_DLL extern l_ok numaaWrite ( const char *filename, NUMAA *naa );
LEPT_DLL extern l_ok numaaWriteStream ( FILE *fp, NUMAA *naa );
LEPT_DLL extern l_ok numaaWriteMem ( l_uint8 **pdata, size_t *psize, NUMAA *naa );
LEPT_DLL extern NUMA * numaArithOp ( NUMA *nad, NUMA *na1, NUMA *na2, l_int32 op );
LEPT_DLL extern NUMA * numaLogicalOp ( NUMA *nad, NUMA *na1, NUMA *na2, l_int32 op );
LEPT_DLL extern NUMA * numaInvert ( NUMA *nad, NUMA *nas );
LEPT_DLL extern l_int32 numaSimilar ( NUMA *na1, NUMA *na2, l_float32 maxdiff, l_int32 *psimilar );
LEPT_DLL extern l_ok numaAddToNumber ( NUMA *na, l_int32 index, l_float32 val );
LEPT_DLL extern l_ok numaGetMin ( NUMA *na, l_float32 *pminval, l_int32 *piminloc );
LEPT_DLL extern l_ok numaGetMax ( NUMA *na, l_float32 *pmaxval, l_int32 *pimaxloc );
LEPT_DLL extern l_ok numaGetSum ( NUMA *na, l_float32 *psum );
LEPT_DLL extern NUMA * numaGetPartialSums ( NUMA *na );
LEPT_DLL extern l_ok numaGetSumOnInterval ( NUMA *na, l_int32 first, l_int32 last, l_float32 *psum );
LEPT_DLL extern l_ok numaHasOnlyIntegers ( NUMA *na, l_int32 maxsamples, l_int32 *pallints );
LEPT_DLL extern NUMA * numaSubsample ( NUMA *nas, l_int32 subfactor );
LEPT_DLL extern NUMA * numaMakeDelta ( NUMA *nas );
LEPT_DLL extern NUMA * numaMakeSequence ( l_float32 startval, l_float32 increment, l_int32 size );
LEPT_DLL extern NUMA * numaMakeConstant ( l_float32 val, l_int32 size );
LEPT_DLL extern NUMA * numaMakeAbsValue ( NUMA *nad, NUMA *nas );
LEPT_DLL extern NUMA * numaAddBorder ( NUMA *nas, l_int32 left, l_int32 right, l_float32 val );
LEPT_DLL extern NUMA * numaAddSpecifiedBorder ( NUMA *nas, l_int32 left, l_int32 right, l_int32 type );
LEPT_DLL extern NUMA * numaRemoveBorder ( NUMA *nas, l_int32 left, l_int32 right );
LEPT_DLL extern l_ok numaCountNonzeroRuns ( NUMA *na, l_int32 *pcount );
LEPT_DLL extern l_ok numaGetNonzeroRange ( NUMA *na, l_float32 eps, l_int32 *pfirst, l_int32 *plast );
LEPT_DLL extern l_ok numaGetCountRelativeToZero ( NUMA *na, l_int32 type, l_int32 *pcount );
LEPT_DLL extern NUMA * numaClipToInterval ( NUMA *nas, l_int32 first, l_int32 last );
LEPT_DLL extern NUMA * numaMakeThresholdIndicator ( NUMA *nas, l_float32 thresh, l_int32 type );
LEPT_DLL extern NUMA * numaUniformSampling ( NUMA *nas, l_int32 nsamp );
LEPT_DLL extern NUMA * numaReverse ( NUMA *nad, NUMA *nas );
LEPT_DLL extern NUMA * numaLowPassIntervals ( NUMA *nas, l_float32 thresh, l_float32 maxn );
LEPT_DLL extern NUMA * numaThresholdEdges ( NUMA *nas, l_float32 thresh1, l_float32 thresh2, l_float32 maxn );
LEPT_DLL extern l_int32 numaGetSpanValues ( NUMA *na, l_int32 span, l_int32 *pstart, l_int32 *pend );
LEPT_DLL extern l_int32 numaGetEdgeValues ( NUMA *na, l_int32 edge, l_int32 *pstart, l_int32 *pend, l_int32 *psign );
LEPT_DLL extern l_ok numaInterpolateEqxVal ( l_float32 startx, l_float32 deltax, NUMA *nay, l_int32 type, l_float32 xval, l_float32 *pyval );
LEPT_DLL extern l_ok numaInterpolateArbxVal ( NUMA *nax, NUMA *nay, l_int32 type, l_float32 xval, l_float32 *pyval );
LEPT_DLL extern l_ok numaInterpolateEqxInterval ( l_float32 startx, l_float32 deltax, NUMA *nasy, l_int32 type, l_float32 x0, l_float32 x1, l_int32 npts, NUMA **pnax, NUMA **pnay );
LEPT_DLL extern l_ok numaInterpolateArbxInterval ( NUMA *nax, NUMA *nay, l_int32 type, l_float32 x0, l_float32 x1, l_int32 npts, NUMA **pnadx, NUMA **pnady );
LEPT_DLL extern l_ok numaFitMax ( NUMA *na, l_float32 *pmaxval, NUMA *naloc, l_float32 *pmaxloc );
LEPT_DLL extern l_ok numaDifferentiateInterval ( NUMA *nax, NUMA *nay, l_float32 x0, l_float32 x1, l_int32 npts, NUMA **pnadx, NUMA **pnady );
LEPT_DLL extern l_ok numaIntegrateInterval ( NUMA *nax, NUMA *nay, l_float32 x0, l_float32 x1, l_int32 npts, l_float32 *psum );
LEPT_DLL extern l_ok numaSortGeneral ( NUMA *na, NUMA **pnasort, NUMA **pnaindex, NUMA **pnainvert, l_int32 sortorder, l_int32 sorttype );
LEPT_DLL extern NUMA * numaSortAutoSelect ( NUMA *nas, l_int32 sortorder );
LEPT_DLL extern NUMA * numaSortIndexAutoSelect ( NUMA *nas, l_int32 sortorder );
LEPT_DLL extern l_int32 numaChooseSortType ( NUMA *nas );
LEPT_DLL extern NUMA * numaSort ( NUMA *naout, NUMA *nain, l_int32 sortorder );
LEPT_DLL extern NUMA * numaBinSort ( NUMA *nas, l_int32 sortorder );
LEPT_DLL extern NUMA * numaGetSortIndex ( NUMA *na, l_int32 sortorder );
LEPT_DLL extern NUMA * numaGetBinSortIndex ( NUMA *nas, l_int32 sortorder );
LEPT_DLL extern NUMA * numaSortByIndex ( NUMA *nas, NUMA *naindex );
LEPT_DLL extern l_int32 numaIsSorted ( NUMA *nas, l_int32 sortorder, l_int32 *psorted );
LEPT_DLL extern l_ok numaSortPair ( NUMA *nax, NUMA *nay, l_int32 sortorder, NUMA **pnasx, NUMA **pnasy );
LEPT_DLL extern NUMA * numaInvertMap ( NUMA *nas );
LEPT_DLL extern NUMA * numaPseudorandomSequence ( l_int32 size, l_int32 seed );
LEPT_DLL extern NUMA * numaRandomPermutation ( NUMA *nas, l_int32 seed );
LEPT_DLL extern l_ok numaGetRankValue ( NUMA *na, l_float32 fract, NUMA *nasort, l_int32 usebins, l_float32 *pval );
LEPT_DLL extern l_ok numaGetMedian ( NUMA *na, l_float32 *pval );
LEPT_DLL extern l_ok numaGetBinnedMedian ( NUMA *na, l_int32 *pval );
LEPT_DLL extern l_ok numaGetMeanDevFromMedian ( NUMA *na, l_float32 med, l_float32 *pdev );
LEPT_DLL extern l_ok numaGetMedianDevFromMedian ( NUMA *na, l_float32 *pmed, l_float32 *pdev );
LEPT_DLL extern l_ok numaGetMode ( NUMA *na, l_float32 *pval, l_int32 *pcount );
LEPT_DLL extern l_ok numaJoin ( NUMA *nad, NUMA *nas, l_int32 istart, l_int32 iend );
LEPT_DLL extern l_ok numaaJoin ( NUMAA *naad, NUMAA *naas, l_int32 istart, l_int32 iend );
LEPT_DLL extern NUMA * numaaFlattenToNuma ( NUMAA *naa );
LEPT_DLL extern NUMA * numaErode ( NUMA *nas, l_int32 size );
LEPT_DLL extern NUMA * numaDilate ( NUMA *nas, l_int32 size );
LEPT_DLL extern NUMA * numaOpen ( NUMA *nas, l_int32 size );
LEPT_DLL extern NUMA * numaClose ( NUMA *nas, l_int32 size );
LEPT_DLL extern NUMA * numaTransform ( NUMA *nas, l_float32 shift, l_float32 scale );
LEPT_DLL extern l_ok numaSimpleStats ( NUMA *na, l_int32 first, l_int32 last, l_float32 *pmean, l_float32 *pvar, l_float32 *prvar );
LEPT_DLL extern l_ok numaWindowedStats ( NUMA *nas, l_int32 wc, NUMA **pnam, NUMA **pnams, NUMA **pnav, NUMA **pnarv );
LEPT_DLL extern NUMA * numaWindowedMean ( NUMA *nas, l_int32 wc );
LEPT_DLL extern NUMA * numaWindowedMeanSquare ( NUMA *nas, l_int32 wc );
LEPT_DLL extern l_ok numaWindowedVariance ( NUMA *nam, NUMA *nams, NUMA **pnav, NUMA **pnarv );
LEPT_DLL extern NUMA * numaWindowedMedian ( NUMA *nas, l_int32 halfwin );
LEPT_DLL extern NUMA * numaConvertToInt ( NUMA *nas );
LEPT_DLL extern NUMA * numaMakeHistogram ( NUMA *na, l_int32 maxbins, l_int32 *pbinsize, l_int32 *pbinstart );
LEPT_DLL extern NUMA * numaMakeHistogramAuto ( NUMA *na, l_int32 maxbins );
LEPT_DLL extern NUMA * numaMakeHistogramClipped ( NUMA *na, l_float32 binsize, l_float32 maxsize );
LEPT_DLL extern NUMA * numaRebinHistogram ( NUMA *nas, l_int32 newsize );
LEPT_DLL extern NUMA * numaNormalizeHistogram ( NUMA *nas, l_float32 tsum );
LEPT_DLL extern l_ok numaGetStatsUsingHistogram ( NUMA *na, l_int32 maxbins, l_float32 *pmin, l_float32 *pmax, l_float32 *pmean, l_float32 *pvariance, l_float32 *pmedian, l_float32 rank, l_float32 *prval, NUMA **phisto );
LEPT_DLL extern l_ok numaGetHistogramStats ( NUMA *nahisto, l_float32 startx, l_float32 deltax, l_float32 *pxmean, l_float32 *pxmedian, l_float32 *pxmode, l_float32 *pxvariance );
LEPT_DLL extern l_ok numaGetHistogramStatsOnInterval ( NUMA *nahisto, l_float32 startx, l_float32 deltax, l_int32 ifirst, l_int32 ilast, l_float32 *pxmean, l_float32 *pxmedian, l_float32 *pxmode, l_float32 *pxvariance );
LEPT_DLL extern l_ok numaMakeRankFromHistogram ( l_float32 startx, l_float32 deltax, NUMA *nasy, l_int32 npts, NUMA **pnax, NUMA **pnay );
LEPT_DLL extern l_ok numaHistogramGetRankFromVal ( NUMA *na, l_float32 rval, l_float32 *prank );
LEPT_DLL extern l_ok numaHistogramGetValFromRank ( NUMA *na, l_float32 rank, l_float32 *prval );
LEPT_DLL extern l_ok numaDiscretizeRankAndIntensity ( NUMA *na, l_int32 nbins, NUMA **pnarbin, NUMA **pnam, NUMA **pnar, NUMA **pnabb );
LEPT_DLL extern l_ok numaGetRankBinValues ( NUMA *na, l_int32 nbins, NUMA **pnarbin, NUMA **pnam );
LEPT_DLL extern l_ok numaSplitDistribution ( NUMA *na, l_float32 scorefract, l_int32 *psplitindex, l_float32 *pave1, l_float32 *pave2, l_float32 *pnum1, l_float32 *pnum2, NUMA **pnascore );
LEPT_DLL extern l_ok grayHistogramsToEMD ( NUMAA *naa1, NUMAA *naa2, NUMA **pnad );
LEPT_DLL extern l_ok numaEarthMoverDistance ( NUMA *na1, NUMA *na2, l_float32 *pdist );
LEPT_DLL extern l_ok grayInterHistogramStats ( NUMAA *naa, l_int32 wc, NUMA **pnam, NUMA **pnams, NUMA **pnav, NUMA **pnarv );
LEPT_DLL extern NUMA * numaFindPeaks ( NUMA *nas, l_int32 nmax, l_float32 fract1, l_float32 fract2 );
LEPT_DLL extern NUMA * numaFindExtrema ( NUMA *nas, l_float32 delta, NUMA **pnav );
LEPT_DLL extern l_ok numaFindLocForThreshold ( NUMA *na, l_int32 skip, l_int32 *pthresh, l_float32 *pfract );
LEPT_DLL extern l_ok numaCountReversals ( NUMA *nas, l_float32 minreversal, l_int32 *pnr, l_float32 *prd );
LEPT_DLL extern l_ok numaSelectCrossingThreshold ( NUMA *nax, NUMA *nay, l_float32 estthresh, l_float32 *pbestthresh );
LEPT_DLL extern NUMA * numaCrossingsByThreshold ( NUMA *nax, NUMA *nay, l_float32 thresh );
LEPT_DLL extern NUMA * numaCrossingsByPeaks ( NUMA *nax, NUMA *nay, l_float32 delta );
LEPT_DLL extern l_ok numaEvalBestHaarParameters ( NUMA *nas, l_float32 relweight, l_int32 nwidth, l_int32 nshift, l_float32 minwidth, l_float32 maxwidth, l_float32 *pbestwidth, l_float32 *pbestshift, l_float32 *pbestscore );
LEPT_DLL extern l_ok numaEvalHaarSum ( NUMA *nas, l_float32 width, l_float32 shift, l_float32 relweight, l_float32 *pscore );
LEPT_DLL extern NUMA * genConstrainedNumaInRange ( l_int32 first, l_int32 last, l_int32 nmax, l_int32 use_pairs );
LEPT_DLL extern l_ok pixGetRegionsBinary ( PIX *pixs, PIX **ppixhm, PIX **ppixtm, PIX **ppixtb, PIXA *pixadb );
LEPT_DLL extern PIX * pixGenHalftoneMask ( PIX *pixs, PIX **ppixtext, l_int32 *phtfound, l_int32 debug );
LEPT_DLL extern PIX * pixGenerateHalftoneMask ( PIX *pixs, PIX **ppixtext, l_int32 *phtfound, PIXA *pixadb );
LEPT_DLL extern PIX * pixGenTextlineMask ( PIX *pixs, PIX **ppixvws, l_int32 *ptlfound, PIXA *pixadb );
LEPT_DLL extern PIX * pixGenTextblockMask ( PIX *pixs, PIX *pixvws, PIXA *pixadb );
LEPT_DLL extern BOX * pixFindPageForeground ( PIX *pixs, l_int32 threshold, l_int32 mindist, l_int32 erasedist, l_int32 showmorph, PIXAC *pixac );
LEPT_DLL extern l_ok pixSplitIntoCharacters ( PIX *pixs, l_int32 minw, l_int32 minh, BOXA **pboxa, PIXA **ppixa, PIX **ppixdebug );
LEPT_DLL extern BOXA * pixSplitComponentWithProfile ( PIX *pixs, l_int32 delta, l_int32 mindel, PIX **ppixdebug );
LEPT_DLL extern PIXA * pixExtractTextlines ( PIX *pixs, l_int32 maxw, l_int32 maxh, l_int32 minw, l_int32 minh, l_int32 adjw, l_int32 adjh, PIXA *pixadb );
LEPT_DLL extern PIXA * pixExtractRawTextlines ( PIX *pixs, l_int32 maxw, l_int32 maxh, l_int32 adjw, l_int32 adjh, PIXA *pixadb );
LEPT_DLL extern l_ok pixCountTextColumns ( PIX *pixs, l_float32 deltafract, l_float32 peakfract, l_float32 clipfract, l_int32 *pncols, PIXA *pixadb );
LEPT_DLL extern l_ok pixDecideIfText ( PIX *pixs, BOX *box, l_int32 *pistext, PIXA *pixadb );
LEPT_DLL extern l_ok pixFindThreshFgExtent ( PIX *pixs, l_int32 thresh, l_int32 *ptop, l_int32 *pbot );
LEPT_DLL extern l_ok pixDecideIfTable ( PIX *pixs, BOX *box, l_int32 orient, l_int32 *pscore, PIXA *pixadb );
LEPT_DLL extern PIX * pixPrepare1bpp ( PIX *pixs, BOX *box, l_float32 cropfract, l_int32 outres );
LEPT_DLL extern l_ok pixEstimateBackground ( PIX *pixs, l_int32 darkthresh, l_float32 edgecrop, l_int32 *pbg );
LEPT_DLL extern l_ok pixFindLargeRectangles ( PIX *pixs, l_int32 polarity, l_int32 nrect, BOXA **pboxa, PIX **ppixdb );
LEPT_DLL extern l_ok pixFindLargestRectangle ( PIX *pixs, l_int32 polarity, BOX **pbox, PIX **ppixdb );
LEPT_DLL extern BOX * pixFindRectangleInCC ( PIX *pixs, BOX *boxs, l_float32 fract, l_int32 dir, l_int32 select, l_int32 debug );
LEPT_DLL extern PIX * pixAutoPhotoinvert ( PIX *pixs, l_int32 thresh, PIX **ppixm, PIXA *pixadb );
LEPT_DLL extern l_ok pixSetSelectCmap ( PIX *pixs, BOX *box, l_int32 sindex, l_int32 rval, l_int32 gval, l_int32 bval );
LEPT_DLL extern l_ok pixColorGrayRegionsCmap ( PIX *pixs, BOXA *boxa, l_int32 type, l_int32 rval, l_int32 gval, l_int32 bval );
LEPT_DLL extern l_ok pixColorGrayCmap ( PIX *pixs, BOX *box, l_int32 type, l_int32 rval, l_int32 gval, l_int32 bval );
LEPT_DLL extern l_ok pixColorGrayMaskedCmap ( PIX *pixs, PIX *pixm, l_int32 type, l_int32 rval, l_int32 gval, l_int32 bval );
LEPT_DLL extern l_ok addColorizedGrayToCmap ( PIXCMAP *cmap, l_int32 type, l_int32 rval, l_int32 gval, l_int32 bval, NUMA **pna );
LEPT_DLL extern l_ok pixSetSelectMaskedCmap ( PIX *pixs, PIX *pixm, l_int32 x, l_int32 y, l_int32 sindex, l_int32 rval, l_int32 gval, l_int32 bval );
LEPT_DLL extern l_ok pixSetMaskedCmap ( PIX *pixs, PIX *pixm, l_int32 x, l_int32 y, l_int32 rval, l_int32 gval, l_int32 bval );
LEPT_DLL extern char * parseForProtos ( const char *filein, const char *prestring );
LEPT_DLL extern l_ok partifyFiles ( const char *dirname, const char *substr, l_int32 nparts, const char *outroot, const char *debugfile );
LEPT_DLL extern l_ok partifyPixac ( PIXAC *pixac, l_int32 nparts, const char *outroot, PIXA *pixadb );
LEPT_DLL extern BOXA * boxaGetWhiteblocks ( BOXA *boxas, BOX *box, l_int32 sortflag, l_int32 maxboxes, l_float32 maxoverlap, l_int32 maxperim, l_float32 fract, l_int32 maxpops );
LEPT_DLL extern BOXA * boxaPruneSortedOnOverlap ( BOXA *boxas, l_float32 maxoverlap );
LEPT_DLL extern l_ok convertFilesToPdf ( const char *dirname, const char *substr, l_int32 res, l_float32 scalefactor, l_int32 type, l_int32 quality, const char *title, const char *fileout );
LEPT_DLL extern l_ok saConvertFilesToPdf ( SARRAY *sa, l_int32 res, l_float32 scalefactor, l_int32 type, l_int32 quality, const char *title, const char *fileout );
LEPT_DLL extern l_ok saConvertFilesToPdfData ( SARRAY *sa, l_int32 res, l_float32 scalefactor, l_int32 type, l_int32 quality, const char *title, l_uint8 **pdata, size_t *pnbytes );
LEPT_DLL extern l_ok selectDefaultPdfEncoding ( PIX *pix, l_int32 *ptype );
LEPT_DLL extern l_ok convertUnscaledFilesToPdf ( const char *dirname, const char *substr, const char *title, const char *fileout );
LEPT_DLL extern l_ok saConvertUnscaledFilesToPdf ( SARRAY *sa, const char *title, const char *fileout );
LEPT_DLL extern l_ok saConvertUnscaledFilesToPdfData ( SARRAY *sa, const char *title, l_uint8 **pdata, size_t *pnbytes );
LEPT_DLL extern l_ok convertUnscaledToPdfData ( const char *fname, const char *title, l_uint8 **pdata, size_t *pnbytes );
LEPT_DLL extern l_ok pixaConvertToPdf ( PIXA *pixa, l_int32 res, l_float32 scalefactor, l_int32 type, l_int32 quality, const char *title, const char *fileout );
LEPT_DLL extern l_ok pixaConvertToPdfData ( PIXA *pixa, l_int32 res, l_float32 scalefactor, l_int32 type, l_int32 quality, const char *title, l_uint8 **pdata, size_t *pnbytes );
LEPT_DLL extern l_ok convertToPdf ( const char *filein, l_int32 type, l_int32 quality, const char *fileout, l_int32 x, l_int32 y, l_int32 res, const char *title, L_PDF_DATA **plpd, l_int32 position );
LEPT_DLL extern l_ok convertImageDataToPdf ( l_uint8 *imdata, size_t size, l_int32 type, l_int32 quality, const char *fileout, l_int32 x, l_int32 y, l_int32 res, const char *title, L_PDF_DATA **plpd, l_int32 position );
LEPT_DLL extern l_ok convertToPdfData ( const char *filein, l_int32 type, l_int32 quality, l_uint8 **pdata, size_t *pnbytes, l_int32 x, l_int32 y, l_int32 res, const char *title, L_PDF_DATA **plpd, l_int32 position );
LEPT_DLL extern l_ok convertImageDataToPdfData ( l_uint8 *imdata, size_t size, l_int32 type, l_int32 quality, l_uint8 **pdata, size_t *pnbytes, l_int32 x, l_int32 y, l_int32 res, const char *title, L_PDF_DATA **plpd, l_int32 position );
LEPT_DLL extern l_ok pixConvertToPdf ( PIX *pix, l_int32 type, l_int32 quality, const char *fileout, l_int32 x, l_int32 y, l_int32 res, const char *title, L_PDF_DATA **plpd, l_int32 position );
LEPT_DLL extern l_ok pixWriteStreamPdf ( FILE *fp, PIX *pix, l_int32 res, const char *title );
LEPT_DLL extern l_ok pixWriteMemPdf ( l_uint8 **pdata, size_t *pnbytes, PIX *pix, l_int32 res, const char *title );
LEPT_DLL extern l_ok convertSegmentedFilesToPdf ( const char *dirname, const char *substr, l_int32 res, l_int32 type, l_int32 thresh, BOXAA *baa, l_int32 quality, l_float32 scalefactor, const char *title, const char *fileout );
LEPT_DLL extern BOXAA * convertNumberedMasksToBoxaa ( const char *dirname, const char *substr, l_int32 numpre, l_int32 numpost );
LEPT_DLL extern l_ok convertToPdfSegmented ( const char *filein, l_int32 res, l_int32 type, l_int32 thresh, BOXA *boxa, l_int32 quality, l_float32 scalefactor, const char *title, const char *fileout );
LEPT_DLL extern l_ok pixConvertToPdfSegmented ( PIX *pixs, l_int32 res, l_int32 type, l_int32 thresh, BOXA *boxa, l_int32 quality, l_float32 scalefactor, const char *title, const char *fileout );
LEPT_DLL extern l_ok convertToPdfDataSegmented ( const char *filein, l_int32 res, l_int32 type, l_int32 thresh, BOXA *boxa, l_int32 quality, l_float32 scalefactor, const char *title, l_uint8 **pdata, size_t *pnbytes );
LEPT_DLL extern l_ok pixConvertToPdfDataSegmented ( PIX *pixs, l_int32 res, l_int32 type, l_int32 thresh, BOXA *boxa, l_int32 quality, l_float32 scalefactor, const char *title, l_uint8 **pdata, size_t *pnbytes );
LEPT_DLL extern l_ok concatenatePdf ( const char *dirname, const char *substr, const char *fileout );
LEPT_DLL extern l_ok saConcatenatePdf ( SARRAY *sa, const char *fileout );
LEPT_DLL extern l_ok ptraConcatenatePdf ( L_PTRA *pa, const char *fileout );
LEPT_DLL extern l_ok concatenatePdfToData ( const char *dirname, const char *substr, l_uint8 **pdata, size_t *pnbytes );
LEPT_DLL extern l_ok saConcatenatePdfToData ( SARRAY *sa, l_uint8 **pdata, size_t *pnbytes );
LEPT_DLL extern l_ok pixConvertToPdfData ( PIX *pix, l_int32 type, l_int32 quality, l_uint8 **pdata, size_t *pnbytes, l_int32 x, l_int32 y, l_int32 res, const char *title, L_PDF_DATA **plpd, l_int32 position );
LEPT_DLL extern l_ok ptraConcatenatePdfToData ( L_PTRA *pa_data, SARRAY *sa, l_uint8 **pdata, size_t *pnbytes );
LEPT_DLL extern l_ok convertTiffMultipageToPdf ( const char *filein, const char *fileout );
LEPT_DLL extern l_ok l_generateCIDataForPdf ( const char *fname, PIX *pix, l_int32 quality, L_COMP_DATA **pcid );
LEPT_DLL extern L_COMP_DATA * l_generateFlateDataPdf ( const char *fname, PIX *pixs );
LEPT_DLL extern L_COMP_DATA * l_generateJpegData ( const char *fname, l_int32 ascii85flag );
LEPT_DLL extern L_COMP_DATA * l_generateJpegDataMem ( l_uint8 *data, size_t nbytes, l_int32 ascii85flag );
LEPT_DLL extern l_ok l_generateCIData ( const char *fname, l_int32 type, l_int32 quality, l_int32 ascii85, L_COMP_DATA **pcid );
LEPT_DLL extern l_ok pixGenerateCIData ( PIX *pixs, l_int32 type, l_int32 quality, l_int32 ascii85, L_COMP_DATA **pcid );
LEPT_DLL extern L_COMP_DATA * l_generateFlateData ( const char *fname, l_int32 ascii85flag );
LEPT_DLL extern L_COMP_DATA * l_generateG4Data ( const char *fname, l_int32 ascii85flag );
LEPT_DLL extern l_ok cidConvertToPdfData ( L_COMP_DATA *cid, const char *title, l_uint8 **pdata, size_t *pnbytes );
LEPT_DLL extern void l_CIDataDestroy ( L_COMP_DATA **pcid );
LEPT_DLL extern void l_pdfSetG4ImageMask ( l_int32 flag );
LEPT_DLL extern void l_pdfSetDateAndVersion ( l_int32 flag );
LEPT_DLL extern void setPixMemoryManager ( alloc_fn allocator, dealloc_fn deallocator );
LEPT_DLL extern PIX * pixCreate ( l_int32 width, l_int32 height, l_int32 depth );
LEPT_DLL extern PIX * pixCreateNoInit ( l_int32 width, l_int32 height, l_int32 depth );
LEPT_DLL extern PIX * pixCreateTemplate ( const PIX *pixs );
LEPT_DLL extern PIX * pixCreateTemplateNoInit ( const PIX *pixs );
LEPT_DLL extern PIX * pixCreateWithCmap ( l_int32 width, l_int32 height, l_int32 depth, l_int32 initcolor );
LEPT_DLL extern PIX * pixCreateHeader ( l_int32 width, l_int32 height, l_int32 depth );
LEPT_DLL extern PIX * pixClone ( PIX *pixs );
LEPT_DLL extern void pixDestroy ( PIX **ppix );
LEPT_DLL extern PIX * pixCopy ( PIX *pixd, const PIX *pixs );
LEPT_DLL extern l_ok pixResizeImageData ( PIX *pixd, const PIX *pixs );
LEPT_DLL extern l_ok pixCopyColormap ( PIX *pixd, const PIX *pixs );
LEPT_DLL extern l_int32 pixSizesEqual ( const PIX *pix1, const PIX *pix2 );
LEPT_DLL extern l_ok pixTransferAllData ( PIX *pixd, PIX **ppixs, l_int32 copytext, l_int32 copyformat );
LEPT_DLL extern l_ok pixSwapAndDestroy ( PIX **ppixd, PIX **ppixs );
LEPT_DLL extern l_int32 pixGetWidth ( const PIX *pix );
LEPT_DLL extern l_int32 pixSetWidth ( PIX *pix, l_int32 width );
LEPT_DLL extern l_int32 pixGetHeight ( const PIX *pix );
LEPT_DLL extern l_int32 pixSetHeight ( PIX *pix, l_int32 height );
LEPT_DLL extern l_int32 pixGetDepth ( const PIX *pix );
LEPT_DLL extern l_int32 pixSetDepth ( PIX *pix, l_int32 depth );
LEPT_DLL extern l_ok pixGetDimensions ( const PIX *pix, l_int32 *pw, l_int32 *ph, l_int32 *pd );
LEPT_DLL extern l_ok pixSetDimensions ( PIX *pix, l_int32 w, l_int32 h, l_int32 d );
LEPT_DLL extern l_ok pixCopyDimensions ( PIX *pixd, const PIX *pixs );
LEPT_DLL extern l_int32 pixGetSpp ( const PIX *pix );
LEPT_DLL extern l_int32 pixSetSpp ( PIX *pix, l_int32 spp );
LEPT_DLL extern l_ok pixCopySpp ( PIX *pixd, const PIX *pixs );
LEPT_DLL extern l_int32 pixGetWpl ( const PIX *pix );
LEPT_DLL extern l_int32 pixSetWpl ( PIX *pix, l_int32 wpl );
LEPT_DLL extern l_int32 pixGetRefcount ( const PIX *pix );
LEPT_DLL extern l_int32 pixChangeRefcount ( PIX *pix, l_int32 delta );
LEPT_DLL extern l_int32 pixGetXRes ( const PIX *pix );
LEPT_DLL extern l_int32 pixSetXRes ( PIX *pix, l_int32 res );
LEPT_DLL extern l_int32 pixGetYRes ( const PIX *pix );
LEPT_DLL extern l_int32 pixSetYRes ( PIX *pix, l_int32 res );
LEPT_DLL extern l_ok pixGetResolution ( const PIX *pix, l_int32 *pxres, l_int32 *pyres );
LEPT_DLL extern l_ok pixSetResolution ( PIX *pix, l_int32 xres, l_int32 yres );
LEPT_DLL extern l_int32 pixCopyResolution ( PIX *pixd, const PIX *pixs );
LEPT_DLL extern l_int32 pixScaleResolution ( PIX *pix, l_float32 xscale, l_float32 yscale );
LEPT_DLL extern l_int32 pixGetInputFormat ( const PIX *pix );
LEPT_DLL extern l_int32 pixSetInputFormat ( PIX *pix, l_int32 informat );
LEPT_DLL extern l_int32 pixCopyInputFormat ( PIX *pixd, const PIX *pixs );
LEPT_DLL extern l_int32 pixSetSpecial ( PIX *pix, l_int32 special );
LEPT_DLL extern char * pixGetText ( PIX *pix );
LEPT_DLL extern l_ok pixSetText ( PIX *pix, const char *textstring );
LEPT_DLL extern l_ok pixAddText ( PIX *pix, const char *textstring );
LEPT_DLL extern l_int32 pixCopyText ( PIX *pixd, const PIX *pixs );
LEPT_DLL extern PIXCMAP * pixGetColormap ( PIX *pix );
LEPT_DLL extern l_ok pixSetColormap ( PIX *pix, PIXCMAP *colormap );
LEPT_DLL extern l_ok pixDestroyColormap ( PIX *pix );
LEPT_DLL extern l_uint32 * pixGetData ( PIX *pix );
LEPT_DLL extern l_int32 pixSetData ( PIX *pix, l_uint32 *data );
LEPT_DLL extern l_uint32 * pixExtractData ( PIX *pixs );
LEPT_DLL extern l_int32 pixFreeData ( PIX *pix );
LEPT_DLL extern void ** pixGetLinePtrs ( PIX *pix, l_int32 *psize );
LEPT_DLL extern l_ok pixPrintStreamInfo ( FILE *fp, const PIX *pix, const char *text );
LEPT_DLL extern l_ok pixGetPixel ( PIX *pix, l_int32 x, l_int32 y, l_uint32 *pval );
LEPT_DLL extern l_ok pixSetPixel ( PIX *pix, l_int32 x, l_int32 y, l_uint32 val );
LEPT_DLL extern l_ok pixGetRGBPixel ( PIX *pix, l_int32 x, l_int32 y, l_int32 *prval, l_int32 *pgval, l_int32 *pbval );
LEPT_DLL extern l_ok pixSetRGBPixel ( PIX *pix, l_int32 x, l_int32 y, l_int32 rval, l_int32 gval, l_int32 bval );
LEPT_DLL extern l_ok pixSetCmapPixel ( PIX *pix, l_int32 x, l_int32 y, l_int32 rval, l_int32 gval, l_int32 bval );
LEPT_DLL extern l_ok pixGetRandomPixel ( PIX *pix, l_uint32 *pval, l_int32 *px, l_int32 *py );
LEPT_DLL extern l_ok pixClearPixel ( PIX *pix, l_int32 x, l_int32 y );
LEPT_DLL extern l_ok pixFlipPixel ( PIX *pix, l_int32 x, l_int32 y );
LEPT_DLL extern void setPixelLow ( l_uint32 *line, l_int32 x, l_int32 depth, l_uint32 val );
LEPT_DLL extern l_ok pixGetBlackOrWhiteVal ( PIX *pixs, l_int32 op, l_uint32 *pval );
LEPT_DLL extern l_ok pixClearAll ( PIX *pix );
LEPT_DLL extern l_ok pixSetAll ( PIX *pix );
LEPT_DLL extern l_ok pixSetAllGray ( PIX *pix, l_int32 grayval );
LEPT_DLL extern l_ok pixSetAllArbitrary ( PIX *pix, l_uint32 val );
LEPT_DLL extern l_ok pixSetBlackOrWhite ( PIX *pixs, l_int32 op );
LEPT_DLL extern l_ok pixSetComponentArbitrary ( PIX *pix, l_int32 comp, l_int32 val );
LEPT_DLL extern l_ok pixClearInRect ( PIX *pix, BOX *box );
LEPT_DLL extern l_ok pixSetInRect ( PIX *pix, BOX *box );
LEPT_DLL extern l_ok pixSetInRectArbitrary ( PIX *pix, BOX *box, l_uint32 val );
LEPT_DLL extern l_ok pixBlendInRect ( PIX *pixs, BOX *box, l_uint32 val, l_float32 fract );
LEPT_DLL extern l_ok pixSetPadBits ( PIX *pix, l_int32 val );
LEPT_DLL extern l_ok pixSetPadBitsBand ( PIX *pix, l_int32 by, l_int32 bh, l_int32 val );
LEPT_DLL extern l_ok pixSetOrClearBorder ( PIX *pixs, l_int32 left, l_int32 right, l_int32 top, l_int32 bot, l_int32 op );
LEPT_DLL extern l_ok pixSetBorderVal ( PIX *pixs, l_int32 left, l_int32 right, l_int32 top, l_int32 bot, l_uint32 val );
LEPT_DLL extern l_ok pixSetBorderRingVal ( PIX *pixs, l_int32 dist, l_uint32 val );
LEPT_DLL extern l_ok pixSetMirroredBorder ( PIX *pixs, l_int32 left, l_int32 right, l_int32 top, l_int32 bot );
LEPT_DLL extern PIX * pixCopyBorder ( PIX *pixd, PIX *pixs, l_int32 left, l_int32 right, l_int32 top, l_int32 bot );
LEPT_DLL extern PIX * pixAddBorder ( PIX *pixs, l_int32 npix, l_uint32 val );
LEPT_DLL extern PIX * pixAddBlackOrWhiteBorder ( PIX *pixs, l_int32 left, l_int32 right, l_int32 top, l_int32 bot, l_int32 op );
LEPT_DLL extern PIX * pixAddBorderGeneral ( PIX *pixs, l_int32 left, l_int32 right, l_int32 top, l_int32 bot, l_uint32 val );
LEPT_DLL extern PIX * pixRemoveBorder ( PIX *pixs, l_int32 npix );
LEPT_DLL extern PIX * pixRemoveBorderGeneral ( PIX *pixs, l_int32 left, l_int32 right, l_int32 top, l_int32 bot );
LEPT_DLL extern PIX * pixRemoveBorderToSize ( PIX *pixs, l_int32 wd, l_int32 hd );
LEPT_DLL extern PIX * pixAddMirroredBorder ( PIX *pixs, l_int32 left, l_int32 right, l_int32 top, l_int32 bot );
LEPT_DLL extern PIX * pixAddRepeatedBorder ( PIX *pixs, l_int32 left, l_int32 right, l_int32 top, l_int32 bot );
LEPT_DLL extern PIX * pixAddMixedBorder ( PIX *pixs, l_int32 left, l_int32 right, l_int32 top, l_int32 bot );
LEPT_DLL extern PIX * pixAddContinuedBorder ( PIX *pixs, l_int32 left, l_int32 right, l_int32 top, l_int32 bot );
LEPT_DLL extern l_ok pixShiftAndTransferAlpha ( PIX *pixd, PIX *pixs, l_float32 shiftx, l_float32 shifty );
LEPT_DLL extern PIX * pixDisplayLayersRGBA ( PIX *pixs, l_uint32 val, l_int32 maxw );
LEPT_DLL extern PIX * pixCreateRGBImage ( PIX *pixr, PIX *pixg, PIX *pixb );
LEPT_DLL extern PIX * pixGetRGBComponent ( PIX *pixs, l_int32 comp );
LEPT_DLL extern l_ok pixSetRGBComponent ( PIX *pixd, PIX *pixs, l_int32 comp );
LEPT_DLL extern PIX * pixGetRGBComponentCmap ( PIX *pixs, l_int32 comp );
LEPT_DLL extern l_ok pixCopyRGBComponent ( PIX *pixd, PIX *pixs, l_int32 comp );
LEPT_DLL extern l_ok composeRGBPixel ( l_int32 rval, l_int32 gval, l_int32 bval, l_uint32 *ppixel );
LEPT_DLL extern l_ok composeRGBAPixel ( l_int32 rval, l_int32 gval, l_int32 bval, l_int32 aval, l_uint32 *ppixel );
LEPT_DLL extern void extractRGBValues ( l_uint32 pixel, l_int32 *prval, l_int32 *pgval, l_int32 *pbval );
LEPT_DLL extern void extractRGBAValues ( l_uint32 pixel, l_int32 *prval, l_int32 *pgval, l_int32 *pbval, l_int32 *paval );
LEPT_DLL extern l_int32 extractMinMaxComponent ( l_uint32 pixel, l_int32 type );
LEPT_DLL extern l_ok pixGetRGBLine ( PIX *pixs, l_int32 row, l_uint8 *bufr, l_uint8 *bufg, l_uint8 *bufb );
LEPT_DLL extern l_ok setLineDataVal ( l_uint32 *line, l_int32 j, l_int32 d, l_uint32 val );
LEPT_DLL extern PIX * pixEndianByteSwapNew ( PIX *pixs );
LEPT_DLL extern l_ok pixEndianByteSwap ( PIX *pixs );
LEPT_DLL extern l_int32 lineEndianByteSwap ( l_uint32 *datad, l_uint32 *datas, l_int32 wpl );
LEPT_DLL extern PIX * pixEndianTwoByteSwapNew ( PIX *pixs );
LEPT_DLL extern l_ok pixEndianTwoByteSwap ( PIX *pixs );
LEPT_DLL extern l_ok pixGetRasterData ( PIX *pixs, l_uint8 **pdata, size_t *pnbytes );
LEPT_DLL extern l_ok pixAlphaIsOpaque ( PIX *pix, l_int32 *popaque );
LEPT_DLL extern l_uint8 ** pixSetupByteProcessing ( PIX *pix, l_int32 *pw, l_int32 *ph );
LEPT_DLL extern l_ok pixCleanupByteProcessing ( PIX *pix, l_uint8 **lineptrs );
LEPT_DLL extern void l_setAlphaMaskBorder ( l_float32 val1, l_float32 val2 );
LEPT_DLL extern l_ok pixSetMasked ( PIX *pixd, PIX *pixm, l_uint32 val );
LEPT_DLL extern l_ok pixSetMaskedGeneral ( PIX *pixd, PIX *pixm, l_uint32 val, l_int32 x, l_int32 y );
LEPT_DLL extern l_ok pixCombineMasked ( PIX *pixd, PIX *pixs, PIX *pixm );
LEPT_DLL extern l_ok pixCombineMaskedGeneral ( PIX *pixd, PIX *pixs, PIX *pixm, l_int32 x, l_int32 y );
LEPT_DLL extern l_ok pixPaintThroughMask ( PIX *pixd, PIX *pixm, l_int32 x, l_int32 y, l_uint32 val );
LEPT_DLL extern PIX * pixCopyWithBoxa ( PIX *pixs, BOXA *boxa, l_int32 background );
LEPT_DLL extern l_ok pixPaintSelfThroughMask ( PIX *pixd, PIX *pixm, l_int32 x, l_int32 y, l_int32 searchdir, l_int32 mindist, l_int32 tilesize, l_int32 ntiles, l_int32 distblend );
LEPT_DLL extern PIX * pixMakeMaskFromVal ( PIX *pixs, l_int32 val );
LEPT_DLL extern PIX * pixMakeMaskFromLUT ( PIX *pixs, l_int32 *tab );
LEPT_DLL extern PIX * pixMakeArbMaskFromRGB ( PIX *pixs, l_float32 rc, l_float32 gc, l_float32 bc, l_float32 thresh );
LEPT_DLL extern PIX * pixSetUnderTransparency ( PIX *pixs, l_uint32 val, l_int32 debug );
LEPT_DLL extern PIX * pixMakeAlphaFromMask ( PIX *pixs, l_int32 dist, BOX **pbox );
LEPT_DLL extern l_ok pixGetColorNearMaskBoundary ( PIX *pixs, PIX *pixm, BOX *box, l_int32 dist, l_uint32 *pval, l_int32 debug );
LEPT_DLL extern PIX * pixDisplaySelectedPixels ( PIX *pixs, PIX *pixm, SEL *sel, l_uint32 val );
LEPT_DLL extern PIX * pixInvert ( PIX *pixd, PIX *pixs );
LEPT_DLL extern PIX * pixOr ( PIX *pixd, PIX *pixs1, PIX *pixs2 );
LEPT_DLL extern PIX * pixAnd ( PIX *pixd, PIX *pixs1, PIX *pixs2 );
LEPT_DLL extern PIX * pixXor ( PIX *pixd, PIX *pixs1, PIX *pixs2 );
LEPT_DLL extern PIX * pixSubtract ( PIX *pixd, PIX *pixs1, PIX *pixs2 );
LEPT_DLL extern l_ok pixZero ( PIX *pix, l_int32 *pempty );
LEPT_DLL extern l_ok pixForegroundFraction ( PIX *pix, l_float32 *pfract );
LEPT_DLL extern NUMA * pixaCountPixels ( PIXA *pixa );
LEPT_DLL extern l_ok pixCountPixels ( PIX *pixs, l_int32 *pcount, l_int32 *tab8 );
LEPT_DLL extern l_ok pixCountPixelsInRect ( PIX *pixs, BOX *box, l_int32 *pcount, l_int32 *tab8 );
LEPT_DLL extern NUMA * pixCountByRow ( PIX *pix, BOX *box );
LEPT_DLL extern NUMA * pixCountByColumn ( PIX *pix, BOX *box );
LEPT_DLL extern NUMA * pixCountPixelsByRow ( PIX *pix, l_int32 *tab8 );
LEPT_DLL extern NUMA * pixCountPixelsByColumn ( PIX *pix );
LEPT_DLL extern l_ok pixCountPixelsInRow ( PIX *pix, l_int32 row, l_int32 *pcount, l_int32 *tab8 );
LEPT_DLL extern NUMA * pixGetMomentByColumn ( PIX *pix, l_int32 order );
LEPT_DLL extern l_ok pixThresholdPixelSum ( PIX *pix, l_int32 thresh, l_int32 *pabove, l_int32 *tab8 );
LEPT_DLL extern l_int32 * makePixelSumTab8 ( void );
LEPT_DLL extern l_int32 * makePixelCentroidTab8 ( void );
LEPT_DLL extern NUMA * pixAverageByRow ( PIX *pix, BOX *box, l_int32 type );
LEPT_DLL extern NUMA * pixAverageByColumn ( PIX *pix, BOX *box, l_int32 type );
LEPT_DLL extern l_ok pixAverageInRect ( PIX *pixs, PIX *pixm, BOX *box, l_int32 minval, l_int32 maxval, l_int32 subsamp, l_float32 *pave );
LEPT_DLL extern l_ok pixAverageInRectRGB ( PIX *pixs, PIX *pixm, BOX *box, l_int32 subsamp, l_uint32 *pave );
LEPT_DLL extern NUMA * pixVarianceByRow ( PIX *pix, BOX *box );
LEPT_DLL extern NUMA * pixVarianceByColumn ( PIX *pix, BOX *box );
LEPT_DLL extern l_ok pixVarianceInRect ( PIX *pix, BOX *box, l_float32 *prootvar );
LEPT_DLL extern NUMA * pixAbsDiffByRow ( PIX *pix, BOX *box );
LEPT_DLL extern NUMA * pixAbsDiffByColumn ( PIX *pix, BOX *box );
LEPT_DLL extern l_ok pixAbsDiffInRect ( PIX *pix, BOX *box, l_int32 dir, l_float32 *pabsdiff );
LEPT_DLL extern l_ok pixAbsDiffOnLine ( PIX *pix, l_int32 x1, l_int32 y1, l_int32 x2, l_int32 y2, l_float32 *pabsdiff );
LEPT_DLL extern l_int32 pixCountArbInRect ( PIX *pixs, BOX *box, l_int32 val, l_int32 factor, l_int32 *pcount );
LEPT_DLL extern PIX * pixMirroredTiling ( PIX *pixs, l_int32 w, l_int32 h );
LEPT_DLL extern l_ok pixFindRepCloseTile ( PIX *pixs, BOX *box, l_int32 searchdir, l_int32 mindist, l_int32 tsize, l_int32 ntiles, BOX **pboxtile, l_int32 debug );
LEPT_DLL extern NUMA * pixGetGrayHistogram ( PIX *pixs, l_int32 factor );
LEPT_DLL extern NUMA * pixGetGrayHistogramMasked ( PIX *pixs, PIX *pixm, l_int32 x, l_int32 y, l_int32 factor );
LEPT_DLL extern NUMA * pixGetGrayHistogramInRect ( PIX *pixs, BOX *box, l_int32 factor );
LEPT_DLL extern NUMAA * pixGetGrayHistogramTiled ( PIX *pixs, l_int32 factor, l_int32 nx, l_int32 ny );
LEPT_DLL extern l_ok pixGetColorHistogram ( PIX *pixs, l_int32 factor, NUMA **pnar, NUMA **pnag, NUMA **pnab );
LEPT_DLL extern l_ok pixGetColorHistogramMasked ( PIX *pixs, PIX *pixm, l_int32 x, l_int32 y, l_int32 factor, NUMA **pnar, NUMA **pnag, NUMA **pnab );
LEPT_DLL extern NUMA * pixGetCmapHistogram ( PIX *pixs, l_int32 factor );
LEPT_DLL extern NUMA * pixGetCmapHistogramMasked ( PIX *pixs, PIX *pixm, l_int32 x, l_int32 y, l_int32 factor );
LEPT_DLL extern NUMA * pixGetCmapHistogramInRect ( PIX *pixs, BOX *box, l_int32 factor );
LEPT_DLL extern l_ok pixCountRGBColors ( PIX *pixs, l_int32 factor, l_int32 *pncolors );
LEPT_DLL extern L_AMAP * pixGetColorAmapHistogram ( PIX *pixs, l_int32 factor );
LEPT_DLL extern l_int32 amapGetCountForColor ( L_AMAP *amap, l_uint32 val );
LEPT_DLL extern l_ok pixGetRankValue ( PIX *pixs, l_int32 factor, l_float32 rank, l_uint32 *pvalue );
LEPT_DLL extern l_ok pixGetRankValueMaskedRGB ( PIX *pixs, PIX *pixm, l_int32 x, l_int32 y, l_int32 factor, l_float32 rank, l_float32 *prval, l_float32 *pgval, l_float32 *pbval );
LEPT_DLL extern l_ok pixGetRankValueMasked ( PIX *pixs, PIX *pixm, l_int32 x, l_int32 y, l_int32 factor, l_float32 rank, l_float32 *pval, NUMA **pna );
LEPT_DLL extern l_ok pixGetPixelAverage ( PIX *pixs, PIX *pixm, l_int32 x, l_int32 y, l_int32 factor, l_uint32 *pval );
LEPT_DLL extern l_ok pixGetPixelStats ( PIX *pixs, l_int32 factor, l_int32 type, l_uint32 *pvalue );
LEPT_DLL extern l_ok pixGetAverageMaskedRGB ( PIX *pixs, PIX *pixm, l_int32 x, l_int32 y, l_int32 factor, l_int32 type, l_float32 *prval, l_float32 *pgval, l_float32 *pbval );
LEPT_DLL extern l_ok pixGetAverageMasked ( PIX *pixs, PIX *pixm, l_int32 x, l_int32 y, l_int32 factor, l_int32 type, l_float32 *pval );
LEPT_DLL extern l_ok pixGetAverageTiledRGB ( PIX *pixs, l_int32 sx, l_int32 sy, l_int32 type, PIX **ppixr, PIX **ppixg, PIX **ppixb );
LEPT_DLL extern PIX * pixGetAverageTiled ( PIX *pixs, l_int32 sx, l_int32 sy, l_int32 type );
LEPT_DLL extern l_int32 pixRowStats ( PIX *pixs, BOX *box, NUMA **pnamean, NUMA **pnamedian, NUMA **pnamode, NUMA **pnamodecount, NUMA **pnavar, NUMA **pnarootvar );
LEPT_DLL extern l_int32 pixColumnStats ( PIX *pixs, BOX *box, NUMA **pnamean, NUMA **pnamedian, NUMA **pnamode, NUMA **pnamodecount, NUMA **pnavar, NUMA **pnarootvar );
LEPT_DLL extern l_ok pixGetRangeValues ( PIX *pixs, l_int32 factor, l_int32 color, l_int32 *pminval, l_int32 *pmaxval );
LEPT_DLL extern l_ok pixGetExtremeValue ( PIX *pixs, l_int32 factor, l_int32 type, l_int32 *prval, l_int32 *pgval, l_int32 *pbval, l_int32 *pgrayval );
LEPT_DLL extern l_ok pixGetMaxValueInRect ( PIX *pixs, BOX *box, l_uint32 *pmaxval, l_int32 *pxmax, l_int32 *pymax );
LEPT_DLL extern l_ok pixGetBinnedComponentRange ( PIX *pixs, l_int32 nbins, l_int32 factor, l_int32 color, l_int32 *pminval, l_int32 *pmaxval, l_uint32 **pcarray, l_int32 fontsize );
LEPT_DLL extern l_ok pixGetRankColorArray ( PIX *pixs, l_int32 nbins, l_int32 type, l_int32 factor, l_uint32 **pcarray, PIXA *pixadb, l_int32 fontsize );
LEPT_DLL extern l_ok pixGetBinnedColor ( PIX *pixs, PIX *pixg, l_int32 factor, l_int32 nbins, NUMA *nalut, l_uint32 **pcarray, PIXA *pixadb );
LEPT_DLL extern PIX * pixDisplayColorArray ( l_uint32 *carray, l_int32 ncolors, l_int32 side, l_int32 ncols, l_int32 fontsize );
LEPT_DLL extern PIX * pixRankBinByStrip ( PIX *pixs, l_int32 direction, l_int32 size, l_int32 nbins, l_int32 type );
LEPT_DLL extern PIX * pixaGetAlignedStats ( PIXA *pixa, l_int32 type, l_int32 nbins, l_int32 thresh );
LEPT_DLL extern l_ok pixaExtractColumnFromEachPix ( PIXA *pixa, l_int32 col, PIX *pixd );
LEPT_DLL extern l_ok pixGetRowStats ( PIX *pixs, l_int32 type, l_int32 nbins, l_int32 thresh, l_float32 *colvect );
LEPT_DLL extern l_ok pixGetColumnStats ( PIX *pixs, l_int32 type, l_int32 nbins, l_int32 thresh, l_float32 *rowvect );
LEPT_DLL extern l_ok pixSetPixelColumn ( PIX *pix, l_int32 col, l_float32 *colvect );
LEPT_DLL extern l_ok pixThresholdForFgBg ( PIX *pixs, l_int32 factor, l_int32 thresh, l_int32 *pfgval, l_int32 *pbgval );
LEPT_DLL extern l_ok pixSplitDistributionFgBg ( PIX *pixs, l_float32 scorefract, l_int32 factor, l_int32 *pthresh, l_int32 *pfgval, l_int32 *pbgval, PIX **ppixdb );
LEPT_DLL extern l_ok pixaFindDimensions ( PIXA *pixa, NUMA **pnaw, NUMA **pnah );
LEPT_DLL extern l_ok pixFindAreaPerimRatio ( PIX *pixs, l_int32 *tab, l_float32 *pfract );
LEPT_DLL extern NUMA * pixaFindPerimToAreaRatio ( PIXA *pixa );
LEPT_DLL extern l_ok pixFindPerimToAreaRatio ( PIX *pixs, l_int32 *tab, l_float32 *pfract );
LEPT_DLL extern NUMA * pixaFindPerimSizeRatio ( PIXA *pixa );
LEPT_DLL extern l_ok pixFindPerimSizeRatio ( PIX *pixs, l_int32 *tab, l_float32 *pratio );
LEPT_DLL extern NUMA * pixaFindAreaFraction ( PIXA *pixa );
LEPT_DLL extern l_ok pixFindAreaFraction ( PIX *pixs, l_int32 *tab, l_float32 *pfract );
LEPT_DLL extern NUMA * pixaFindAreaFractionMasked ( PIXA *pixa, PIX *pixm, l_int32 debug );
LEPT_DLL extern l_ok pixFindAreaFractionMasked ( PIX *pixs, BOX *box, PIX *pixm, l_int32 *tab, l_float32 *pfract );
LEPT_DLL extern NUMA * pixaFindWidthHeightRatio ( PIXA *pixa );
LEPT_DLL extern NUMA * pixaFindWidthHeightProduct ( PIXA *pixa );
LEPT_DLL extern l_ok pixFindOverlapFraction ( PIX *pixs1, PIX *pixs2, l_int32 x2, l_int32 y2, l_int32 *tab, l_float32 *pratio, l_int32 *pnoverlap );
LEPT_DLL extern BOXA * pixFindRectangleComps ( PIX *pixs, l_int32 dist, l_int32 minw, l_int32 minh );
LEPT_DLL extern l_ok pixConformsToRectangle ( PIX *pixs, BOX *box, l_int32 dist, l_int32 *pconforms );
LEPT_DLL extern PIXA * pixClipRectangles ( PIX *pixs, BOXA *boxa );
LEPT_DLL extern PIX * pixClipRectangle ( PIX *pixs, BOX *box, BOX **pboxc );
LEPT_DLL extern PIX * pixClipMasked ( PIX *pixs, PIX *pixm, l_int32 x, l_int32 y, l_uint32 outval );
LEPT_DLL extern l_ok pixCropToMatch ( PIX *pixs1, PIX *pixs2, PIX **ppixd1, PIX **ppixd2 );
LEPT_DLL extern PIX * pixCropToSize ( PIX *pixs, l_int32 w, l_int32 h );
LEPT_DLL extern PIX * pixResizeToMatch ( PIX *pixs, PIX *pixt, l_int32 w, l_int32 h );
LEPT_DLL extern PIX * pixSelectComponentBySize ( PIX *pixs, l_int32 rankorder, l_int32 type, l_int32 connectivity, BOX **pbox );
LEPT_DLL extern PIX * pixFilterComponentBySize ( PIX *pixs, l_int32 rankorder, l_int32 type, l_int32 connectivity, BOX **pbox );
LEPT_DLL extern PIX * pixMakeSymmetricMask ( l_int32 w, l_int32 h, l_float32 hf, l_float32 vf, l_int32 type );
LEPT_DLL extern PIX * pixMakeFrameMask ( l_int32 w, l_int32 h, l_float32 hf1, l_float32 hf2, l_float32 vf1, l_float32 vf2 );
LEPT_DLL extern PIX * pixMakeCoveringOfRectangles ( PIX *pixs, l_int32 maxiters );
LEPT_DLL extern l_ok pixFractionFgInMask ( PIX *pix1, PIX *pix2, l_float32 *pfract );
LEPT_DLL extern l_ok pixClipToForeground ( PIX *pixs, PIX **ppixd, BOX **pbox );
LEPT_DLL extern l_ok pixTestClipToForeground ( PIX *pixs, l_int32 *pcanclip );
LEPT_DLL extern l_ok pixClipBoxToForeground ( PIX *pixs, BOX *boxs, PIX **ppixd, BOX **pboxd );
LEPT_DLL extern l_ok pixScanForForeground ( PIX *pixs, BOX *box, l_int32 scanflag, l_int32 *ploc );
LEPT_DLL extern l_ok pixClipBoxToEdges ( PIX *pixs, BOX *boxs, l_int32 lowthresh, l_int32 highthresh, l_int32 maxwidth, l_int32 factor, PIX **ppixd, BOX **pboxd );
LEPT_DLL extern l_ok pixScanForEdge ( PIX *pixs, BOX *box, l_int32 lowthresh, l_int32 highthresh, l_int32 maxwidth, l_int32 factor, l_int32 scanflag, l_int32 *ploc );
LEPT_DLL extern NUMA * pixExtractOnLine ( PIX *pixs, l_int32 x1, l_int32 y1, l_int32 x2, l_int32 y2, l_int32 factor );
LEPT_DLL extern l_float32 pixAverageOnLine ( PIX *pixs, l_int32 x1, l_int32 y1, l_int32 x2, l_int32 y2, l_int32 factor );
LEPT_DLL extern NUMA * pixAverageIntensityProfile ( PIX *pixs, l_float32 fract, l_int32 dir, l_int32 first, l_int32 last, l_int32 factor1, l_int32 factor2 );
LEPT_DLL extern NUMA * pixReversalProfile ( PIX *pixs, l_float32 fract, l_int32 dir, l_int32 first, l_int32 last, l_int32 minreversal, l_int32 factor1, l_int32 factor2 );
LEPT_DLL extern l_ok pixWindowedVarianceOnLine ( PIX *pixs, l_int32 dir, l_int32 loc, l_int32 c1, l_int32 c2, l_int32 size, NUMA **pnad );
LEPT_DLL extern l_ok pixMinMaxNearLine ( PIX *pixs, l_int32 x1, l_int32 y1, l_int32 x2, l_int32 y2, l_int32 dist, l_int32 direction, NUMA **pnamin, NUMA **pnamax, l_float32 *pminave, l_float32 *pmaxave );
LEPT_DLL extern PIX * pixRankRowTransform ( PIX *pixs );
LEPT_DLL extern PIX * pixRankColumnTransform ( PIX *pixs );
LEPT_DLL extern PIXA * pixaCreate ( l_int32 n );
LEPT_DLL extern PIXA * pixaCreateFromPix ( PIX *pixs, l_int32 n, l_int32 cellw, l_int32 cellh );
LEPT_DLL extern PIXA * pixaCreateFromBoxa ( PIX *pixs, BOXA *boxa, l_int32 start, l_int32 num, l_int32 *pcropwarn );
LEPT_DLL extern PIXA * pixaSplitPix ( PIX *pixs, l_int32 nx, l_int32 ny, l_int32 borderwidth, l_uint32 bordercolor );
LEPT_DLL extern void pixaDestroy ( PIXA **ppixa );
LEPT_DLL extern PIXA * pixaCopy ( PIXA *pixa, l_int32 copyflag );
LEPT_DLL extern l_ok pixaAddPix ( PIXA *pixa, PIX *pix, l_int32 copyflag );
LEPT_DLL extern l_ok pixaAddBox ( PIXA *pixa, BOX *box, l_int32 copyflag );
LEPT_DLL extern l_ok pixaExtendArrayToSize ( PIXA *pixa, l_int32 size );
LEPT_DLL extern l_int32 pixaGetCount ( PIXA *pixa );
LEPT_DLL extern l_ok pixaChangeRefcount ( PIXA *pixa, l_int32 delta );
LEPT_DLL extern PIX * pixaGetPix ( PIXA *pixa, l_int32 index, l_int32 accesstype );
LEPT_DLL extern l_ok pixaGetPixDimensions ( PIXA *pixa, l_int32 index, l_int32 *pw, l_int32 *ph, l_int32 *pd );
LEPT_DLL extern BOXA * pixaGetBoxa ( PIXA *pixa, l_int32 accesstype );
LEPT_DLL extern l_int32 pixaGetBoxaCount ( PIXA *pixa );
LEPT_DLL extern BOX * pixaGetBox ( PIXA *pixa, l_int32 index, l_int32 accesstype );
LEPT_DLL extern l_ok pixaGetBoxGeometry ( PIXA *pixa, l_int32 index, l_int32 *px, l_int32 *py, l_int32 *pw, l_int32 *ph );
LEPT_DLL extern l_ok pixaSetBoxa ( PIXA *pixa, BOXA *boxa, l_int32 accesstype );
LEPT_DLL extern PIX ** pixaGetPixArray ( PIXA *pixa );
LEPT_DLL extern l_ok pixaVerifyDepth ( PIXA *pixa, l_int32 *psame, l_int32 *pmaxd );
LEPT_DLL extern l_ok pixaVerifyDimensions ( PIXA *pixa, l_int32 *psame, l_int32 *pmaxw, l_int32 *pmaxh );
LEPT_DLL extern l_ok pixaIsFull ( PIXA *pixa, l_int32 *pfullpa, l_int32 *pfullba );
LEPT_DLL extern l_ok pixaCountText ( PIXA *pixa, l_int32 *pntext );
LEPT_DLL extern l_ok pixaSetText ( PIXA *pixa, const char *text, SARRAY *sa );
LEPT_DLL extern void *** pixaGetLinePtrs ( PIXA *pixa, l_int32 *psize );
LEPT_DLL extern l_ok pixaWriteStreamInfo ( FILE *fp, PIXA *pixa );
LEPT_DLL extern l_ok pixaReplacePix ( PIXA *pixa, l_int32 index, PIX *pix, BOX *box );
LEPT_DLL extern l_ok pixaInsertPix ( PIXA *pixa, l_int32 index, PIX *pixs, BOX *box );
LEPT_DLL extern l_ok pixaRemovePix ( PIXA *pixa, l_int32 index );
LEPT_DLL extern l_ok pixaRemovePixAndSave ( PIXA *pixa, l_int32 index, PIX **ppix, BOX **pbox );
LEPT_DLL extern l_ok pixaRemoveSelected ( PIXA *pixa, NUMA *naindex );
LEPT_DLL extern l_ok pixaInitFull ( PIXA *pixa, PIX *pix, BOX *box );
LEPT_DLL extern l_ok pixaClear ( PIXA *pixa );
LEPT_DLL extern l_ok pixaJoin ( PIXA *pixad, PIXA *pixas, l_int32 istart, l_int32 iend );
LEPT_DLL extern PIXA * pixaInterleave ( PIXA *pixa1, PIXA *pixa2, l_int32 copyflag );
LEPT_DLL extern l_ok pixaaJoin ( PIXAA *paad, PIXAA *paas, l_int32 istart, l_int32 iend );
LEPT_DLL extern PIXAA * pixaaCreate ( l_int32 n );
LEPT_DLL extern PIXAA * pixaaCreateFromPixa ( PIXA *pixa, l_int32 n, l_int32 type, l_int32 copyflag );
LEPT_DLL extern void pixaaDestroy ( PIXAA **ppaa );
LEPT_DLL extern l_ok pixaaAddPixa ( PIXAA *paa, PIXA *pixa, l_int32 copyflag );
LEPT_DLL extern l_ok pixaaExtendArray ( PIXAA *paa );
LEPT_DLL extern l_ok pixaaAddPix ( PIXAA *paa, l_int32 index, PIX *pix, BOX *box, l_int32 copyflag );
LEPT_DLL extern l_ok pixaaAddBox ( PIXAA *paa, BOX *box, l_int32 copyflag );
LEPT_DLL extern l_int32 pixaaGetCount ( PIXAA *paa, NUMA **pna );
LEPT_DLL extern PIXA * pixaaGetPixa ( PIXAA *paa, l_int32 index, l_int32 accesstype );
LEPT_DLL extern BOXA * pixaaGetBoxa ( PIXAA *paa, l_int32 accesstype );
LEPT_DLL extern PIX * pixaaGetPix ( PIXAA *paa, l_int32 index, l_int32 ipix, l_int32 accessflag );
LEPT_DLL extern l_ok pixaaVerifyDepth ( PIXAA *paa, l_int32 *psame, l_int32 *pmaxd );
LEPT_DLL extern l_ok pixaaVerifyDimensions ( PIXAA *paa, l_int32 *psame, l_int32 *pmaxw, l_int32 *pmaxh );
LEPT_DLL extern l_int32 pixaaIsFull ( PIXAA *paa, l_int32 *pfull );
LEPT_DLL extern l_ok pixaaInitFull ( PIXAA *paa, PIXA *pixa );
LEPT_DLL extern l_ok pixaaReplacePixa ( PIXAA *paa, l_int32 index, PIXA *pixa );
LEPT_DLL extern l_ok pixaaClear ( PIXAA *paa );
LEPT_DLL extern l_ok pixaaTruncate ( PIXAA *paa );
LEPT_DLL extern PIXA * pixaRead ( const char *filename );
LEPT_DLL extern PIXA * pixaReadStream ( FILE *fp );
LEPT_DLL extern PIXA * pixaReadMem ( const l_uint8 *data, size_t size );
LEPT_DLL extern l_ok pixaWriteDebug ( const char *fname, PIXA *pixa );
LEPT_DLL extern l_ok pixaWrite ( const char *filename, PIXA *pixa );
LEPT_DLL extern l_ok pixaWriteStream ( FILE *fp, PIXA *pixa );
LEPT_DLL extern l_ok pixaWriteMem ( l_uint8 **pdata, size_t *psize, PIXA *pixa );
LEPT_DLL extern PIXA * pixaReadBoth ( const char *filename );
LEPT_DLL extern PIXAA * pixaaReadFromFiles ( const char *dirname, const char *substr, l_int32 first, l_int32 nfiles );
LEPT_DLL extern PIXAA * pixaaRead ( const char *filename );
LEPT_DLL extern PIXAA * pixaaReadStream ( FILE *fp );
LEPT_DLL extern PIXAA * pixaaReadMem ( const l_uint8 *data, size_t size );
LEPT_DLL extern l_ok pixaaWrite ( const char *filename, PIXAA *paa );
LEPT_DLL extern l_ok pixaaWriteStream ( FILE *fp, PIXAA *paa );
LEPT_DLL extern l_ok pixaaWriteMem ( l_uint8 **pdata, size_t *psize, PIXAA *paa );
LEPT_DLL extern PIXACC * pixaccCreate ( l_int32 w, l_int32 h, l_int32 negflag );
LEPT_DLL extern PIXACC * pixaccCreateFromPix ( PIX *pix, l_int32 negflag );
LEPT_DLL extern void pixaccDestroy ( PIXACC **ppixacc );
LEPT_DLL extern PIX * pixaccFinal ( PIXACC *pixacc, l_int32 outdepth );
LEPT_DLL extern PIX * pixaccGetPix ( PIXACC *pixacc );
LEPT_DLL extern l_int32 pixaccGetOffset ( PIXACC *pixacc );
LEPT_DLL extern l_ok pixaccAdd ( PIXACC *pixacc, PIX *pix );
LEPT_DLL extern l_ok pixaccSubtract ( PIXACC *pixacc, PIX *pix );
LEPT_DLL extern l_ok pixaccMultConst ( PIXACC *pixacc, l_float32 factor );
LEPT_DLL extern l_ok pixaccMultConstAccumulate ( PIXACC *pixacc, PIX *pix, l_float32 factor );
LEPT_DLL extern PIX * pixSelectBySize ( PIX *pixs, l_int32 width, l_int32 height, l_int32 connectivity, l_int32 type, l_int32 relation, l_int32 *pchanged );
LEPT_DLL extern PIXA * pixaSelectBySize ( PIXA *pixas, l_int32 width, l_int32 height, l_int32 type, l_int32 relation, l_int32 *pchanged );
LEPT_DLL extern NUMA * pixaMakeSizeIndicator ( PIXA *pixa, l_int32 width, l_int32 height, l_int32 type, l_int32 relation );
LEPT_DLL extern PIX * pixSelectByPerimToAreaRatio ( PIX *pixs, l_float32 thresh, l_int32 connectivity, l_int32 type, l_int32 *pchanged );
LEPT_DLL extern PIXA * pixaSelectByPerimToAreaRatio ( PIXA *pixas, l_float32 thresh, l_int32 type, l_int32 *pchanged );
LEPT_DLL extern PIX * pixSelectByPerimSizeRatio ( PIX *pixs, l_float32 thresh, l_int32 connectivity, l_int32 type, l_int32 *pchanged );
LEPT_DLL extern PIXA * pixaSelectByPerimSizeRatio ( PIXA *pixas, l_float32 thresh, l_int32 type, l_int32 *pchanged );
LEPT_DLL extern PIX * pixSelectByAreaFraction ( PIX *pixs, l_float32 thresh, l_int32 connectivity, l_int32 type, l_int32 *pchanged );
LEPT_DLL extern PIXA * pixaSelectByAreaFraction ( PIXA *pixas, l_float32 thresh, l_int32 type, l_int32 *pchanged );
LEPT_DLL extern PIX * pixSelectByWidthHeightRatio ( PIX *pixs, l_float32 thresh, l_int32 connectivity, l_int32 type, l_int32 *pchanged );
LEPT_DLL extern PIXA * pixaSelectByWidthHeightRatio ( PIXA *pixas, l_float32 thresh, l_int32 type, l_int32 *pchanged );
LEPT_DLL extern PIXA * pixaSelectByNumConnComp ( PIXA *pixas, l_int32 nmin, l_int32 nmax, l_int32 connectivity, l_int32 *pchanged );
LEPT_DLL extern PIXA * pixaSelectWithIndicator ( PIXA *pixas, NUMA *na, l_int32 *pchanged );
LEPT_DLL extern l_ok pixRemoveWithIndicator ( PIX *pixs, PIXA *pixa, NUMA *na );
LEPT_DLL extern l_ok pixAddWithIndicator ( PIX *pixs, PIXA *pixa, NUMA *na );
LEPT_DLL extern PIXA * pixaSelectWithString ( PIXA *pixas, const char *str, l_int32 *perror );
LEPT_DLL extern PIX * pixaRenderComponent ( PIX *pixs, PIXA *pixa, l_int32 index );
LEPT_DLL extern PIXA * pixaSort ( PIXA *pixas, l_int32 sorttype, l_int32 sortorder, NUMA **pnaindex, l_int32 copyflag );
LEPT_DLL extern PIXA * pixaBinSort ( PIXA *pixas, l_int32 sorttype, l_int32 sortorder, NUMA **pnaindex, l_int32 copyflag );
LEPT_DLL extern PIXA * pixaSortByIndex ( PIXA *pixas, NUMA *naindex, l_int32 copyflag );
LEPT_DLL extern PIXAA * pixaSort2dByIndex ( PIXA *pixas, NUMAA *naa, l_int32 copyflag );
LEPT_DLL extern PIXA * pixaSelectRange ( PIXA *pixas, l_int32 first, l_int32 last, l_int32 copyflag );
LEPT_DLL extern PIXAA * pixaaSelectRange ( PIXAA *paas, l_int32 first, l_int32 last, l_int32 copyflag );
LEPT_DLL extern PIXAA * pixaaScaleToSize ( PIXAA *paas, l_int32 wd, l_int32 hd );
LEPT_DLL extern PIXAA * pixaaScaleToSizeVar ( PIXAA *paas, NUMA *nawd, NUMA *nahd );
LEPT_DLL extern PIXA * pixaScaleToSize ( PIXA *pixas, l_int32 wd, l_int32 hd );
LEPT_DLL extern PIXA * pixaScaleToSizeRel ( PIXA *pixas, l_int32 delw, l_int32 delh );
LEPT_DLL extern PIXA * pixaScale ( PIXA *pixas, l_float32 scalex, l_float32 scaley );
LEPT_DLL extern PIXA * pixaScaleBySampling ( PIXA *pixas, l_float32 scalex, l_float32 scaley );
LEPT_DLL extern PIXA * pixaRotate ( PIXA *pixas, l_float32 angle, l_int32 type, l_int32 incolor, l_int32 width, l_int32 height );
LEPT_DLL extern PIXA * pixaRotateOrth ( PIXA *pixas, l_int32 rotation );
LEPT_DLL extern PIXA * pixaTranslate ( PIXA *pixas, l_int32 hshift, l_int32 vshift, l_int32 incolor );
LEPT_DLL extern PIXA * pixaAddBorderGeneral ( PIXA *pixad, PIXA *pixas, l_int32 left, l_int32 right, l_int32 top, l_int32 bot, l_uint32 val );
LEPT_DLL extern PIXA * pixaaFlattenToPixa ( PIXAA *paa, NUMA **pnaindex, l_int32 copyflag );
LEPT_DLL extern l_ok pixaaSizeRange ( PIXAA *paa, l_int32 *pminw, l_int32 *pminh, l_int32 *pmaxw, l_int32 *pmaxh );
LEPT_DLL extern l_ok pixaSizeRange ( PIXA *pixa, l_int32 *pminw, l_int32 *pminh, l_int32 *pmaxw, l_int32 *pmaxh );
LEPT_DLL extern PIXA * pixaClipToPix ( PIXA *pixas, PIX *pixs );
LEPT_DLL extern l_ok pixaClipToForeground ( PIXA *pixas, PIXA **ppixad, BOXA **pboxa );
LEPT_DLL extern l_ok pixaGetRenderingDepth ( PIXA *pixa, l_int32 *pdepth );
LEPT_DLL extern l_ok pixaHasColor ( PIXA *pixa, l_int32 *phascolor );
LEPT_DLL extern l_ok pixaAnyColormaps ( PIXA *pixa, l_int32 *phascmap );
LEPT_DLL extern l_ok pixaGetDepthInfo ( PIXA *pixa, l_int32 *pmaxdepth, l_int32 *psame );
LEPT_DLL extern PIXA * pixaConvertToSameDepth ( PIXA *pixas );
LEPT_DLL extern l_ok pixaEqual ( PIXA *pixa1, PIXA *pixa2, l_int32 maxdist, NUMA **pnaindex, l_int32 *psame );
LEPT_DLL extern l_ok pixaSetFullSizeBoxa ( PIXA *pixa );
LEPT_DLL extern PIX * pixaDisplay ( PIXA *pixa, l_int32 w, l_int32 h );
LEPT_DLL extern PIX * pixaDisplayRandomCmap ( PIXA *pixa, l_int32 w, l_int32 h );
LEPT_DLL extern PIX * pixaDisplayLinearly ( PIXA *pixas, l_int32 direction, l_float32 scalefactor, l_int32 background, l_int32 spacing, l_int32 border, BOXA **pboxa );
LEPT_DLL extern PIX * pixaDisplayOnLattice ( PIXA *pixa, l_int32 cellw, l_int32 cellh, l_int32 *pncols, BOXA **pboxa );
LEPT_DLL extern PIX * pixaDisplayUnsplit ( PIXA *pixa, l_int32 nx, l_int32 ny, l_int32 borderwidth, l_uint32 bordercolor );
LEPT_DLL extern PIX * pixaDisplayTiled ( PIXA *pixa, l_int32 maxwidth, l_int32 background, l_int32 spacing );
LEPT_DLL extern PIX * pixaDisplayTiledInRows ( PIXA *pixa, l_int32 outdepth, l_int32 maxwidth, l_float32 scalefactor, l_int32 background, l_int32 spacing, l_int32 border );
LEPT_DLL extern PIX * pixaDisplayTiledInColumns ( PIXA *pixas, l_int32 nx, l_float32 scalefactor, l_int32 spacing, l_int32 border );
LEPT_DLL extern PIX * pixaDisplayTiledAndScaled ( PIXA *pixa, l_int32 outdepth, l_int32 tilewidth, l_int32 ncols, l_int32 background, l_int32 spacing, l_int32 border );
LEPT_DLL extern PIX * pixaDisplayTiledWithText ( PIXA *pixa, l_int32 maxwidth, l_float32 scalefactor, l_int32 spacing, l_int32 border, l_int32 fontsize, l_uint32 textcolor );
LEPT_DLL extern PIX * pixaDisplayTiledByIndex ( PIXA *pixa, NUMA *na, l_int32 width, l_int32 spacing, l_int32 border, l_int32 fontsize, l_uint32 textcolor );
LEPT_DLL extern PIX * pixaaDisplay ( PIXAA *paa, l_int32 w, l_int32 h );
LEPT_DLL extern PIX * pixaaDisplayByPixa ( PIXAA *paa, l_int32 maxnx, l_float32 scalefactor, l_int32 hspacing, l_int32 vspacing, l_int32 border );
LEPT_DLL extern PIXA * pixaaDisplayTiledAndScaled ( PIXAA *paa, l_int32 outdepth, l_int32 tilewidth, l_int32 ncols, l_int32 background, l_int32 spacing, l_int32 border );
LEPT_DLL extern PIXA * pixaConvertTo1 ( PIXA *pixas, l_int32 thresh );
LEPT_DLL extern PIXA * pixaConvertTo8 ( PIXA *pixas, l_int32 cmapflag );
LEPT_DLL extern PIXA * pixaConvertTo8Colormap ( PIXA *pixas, l_int32 dither );
LEPT_DLL extern PIXA * pixaConvertTo32 ( PIXA *pixas );
LEPT_DLL extern PIXA * pixaConstrainedSelect ( PIXA *pixas, l_int32 first, l_int32 last, l_int32 nmax, l_int32 use_pairs, l_int32 copyflag );
LEPT_DLL extern l_ok pixaSelectToPdf ( PIXA *pixas, l_int32 first, l_int32 last, l_int32 res, l_float32 scalefactor, l_int32 type, l_int32 quality, l_uint32 color, l_int32 fontsize, const char *fileout );
LEPT_DLL extern PIXA * pixaMakeFromTiledPixa ( PIXA *pixas, l_int32 w, l_int32 h, l_int32 nsamp );
LEPT_DLL extern PIXA * pixaMakeFromTiledPix ( PIX *pixs, l_int32 w, l_int32 h, l_int32 start, l_int32 num, BOXA *boxa );
LEPT_DLL extern l_ok pixGetTileCount ( PIX *pix, l_int32 *pn );
LEPT_DLL extern PIXA * pixaDisplayMultiTiled ( PIXA *pixas, l_int32 nx, l_int32 ny, l_int32 maxw, l_int32 maxh, l_float32 scalefactor, l_int32 spacing, l_int32 border );
LEPT_DLL extern l_ok pixaSplitIntoFiles ( PIXA *pixas, l_int32 nsplit, l_float32 scale, l_int32 outwidth, l_int32 write_pixa, l_int32 write_pix, l_int32 write_pdf );
LEPT_DLL extern l_ok convertToNUpFiles ( const char *dir, const char *substr, l_int32 nx, l_int32 ny, l_int32 tw, l_int32 spacing, l_int32 border, l_int32 fontsize, const char *outdir );
LEPT_DLL extern PIXA * convertToNUpPixa ( const char *dir, const char *substr, l_int32 nx, l_int32 ny, l_int32 tw, l_int32 spacing, l_int32 border, l_int32 fontsize );
LEPT_DLL extern PIXA * pixaConvertToNUpPixa ( PIXA *pixas, SARRAY *sa, l_int32 nx, l_int32 ny, l_int32 tw, l_int32 spacing, l_int32 border, l_int32 fontsize );
LEPT_DLL extern l_ok pixaCompareInPdf ( PIXA *pixa1, PIXA *pixa2, l_int32 nx, l_int32 ny, l_int32 tw, l_int32 spacing, l_int32 border, l_int32 fontsize, const char *fileout );
LEPT_DLL extern l_ok pmsCreate ( size_t minsize, size_t smallest, NUMA *numalloc, const char *logfile );
LEPT_DLL extern void pmsDestroy ( void );
LEPT_DLL extern void * pmsCustomAlloc ( size_t nbytes );
LEPT_DLL extern void pmsCustomDealloc ( void *data );
LEPT_DLL extern void * pmsGetAlloc ( size_t nbytes );
LEPT_DLL extern l_ok pmsGetLevelForAlloc ( size_t nbytes, l_int32 *plevel );
LEPT_DLL extern l_ok pmsGetLevelForDealloc ( void *data, l_int32 *plevel );
LEPT_DLL extern void pmsLogInfo ( void );
LEPT_DLL extern l_ok pixAddConstantGray ( PIX *pixs, l_int32 val );
LEPT_DLL extern l_ok pixMultConstantGray ( PIX *pixs, l_float32 val );
LEPT_DLL extern PIX * pixAddGray ( PIX *pixd, PIX *pixs1, PIX *pixs2 );
LEPT_DLL extern PIX * pixSubtractGray ( PIX *pixd, PIX *pixs1, PIX *pixs2 );
LEPT_DLL extern PIX * pixMultiplyGray ( PIX *pixs, PIX *pixg, l_float32 norm );
LEPT_DLL extern PIX * pixThresholdToValue ( PIX *pixd, PIX *pixs, l_int32 threshval, l_int32 setval );
LEPT_DLL extern PIX * pixInitAccumulate ( l_int32 w, l_int32 h, l_uint32 offset );
LEPT_DLL extern PIX * pixFinalAccumulate ( PIX *pixs, l_uint32 offset, l_int32 depth );
LEPT_DLL extern PIX * pixFinalAccumulateThreshold ( PIX *pixs, l_uint32 offset, l_uint32 threshold );
LEPT_DLL extern l_ok pixAccumulate ( PIX *pixd, PIX *pixs, l_int32 op );
LEPT_DLL extern l_ok pixMultConstAccumulate ( PIX *pixs, l_float32 factor, l_uint32 offset );
LEPT_DLL extern PIX * pixAbsDifference ( PIX *pixs1, PIX *pixs2 );
LEPT_DLL extern PIX * pixAddRGB ( PIX *pixs1, PIX *pixs2 );
LEPT_DLL extern PIX * pixMinOrMax ( PIX *pixd, PIX *pixs1, PIX *pixs2, l_int32 type );
LEPT_DLL extern PIX * pixMaxDynamicRange ( PIX *pixs, l_int32 type );
LEPT_DLL extern PIX * pixMaxDynamicRangeRGB ( PIX *pixs, l_int32 type );
LEPT_DLL extern l_uint32 linearScaleRGBVal ( l_uint32 sval, l_float32 factor );
LEPT_DLL extern l_uint32 logScaleRGBVal ( l_uint32 sval, l_float32 *tab, l_float32 factor );
LEPT_DLL extern l_float32 * makeLogBase2Tab ( void );
LEPT_DLL extern l_float32 getLogBase2 ( l_int32 val, l_float32 *logtab );
LEPT_DLL extern PIXC * pixcompCreateFromPix ( PIX *pix, l_int32 comptype );
LEPT_DLL extern PIXC * pixcompCreateFromString ( l_uint8 *data, size_t size, l_int32 copyflag );
LEPT_DLL extern PIXC * pixcompCreateFromFile ( const char *filename, l_int32 comptype );
LEPT_DLL extern void pixcompDestroy ( PIXC **ppixc );
LEPT_DLL extern PIXC * pixcompCopy ( PIXC *pixcs );
LEPT_DLL extern l_ok pixcompGetDimensions ( PIXC *pixc, l_int32 *pw, l_int32 *ph, l_int32 *pd );
LEPT_DLL extern l_ok pixcompGetParameters ( PIXC *pixc, l_int32 *pxres, l_int32 *pyres, l_int32 *pcomptype, l_int32 *pcmapflag );
LEPT_DLL extern l_ok pixcompDetermineFormat ( l_int32 comptype, l_int32 d, l_int32 cmapflag, l_int32 *pformat );
LEPT_DLL extern PIX * pixCreateFromPixcomp ( PIXC *pixc );
LEPT_DLL extern PIXAC * pixacompCreate ( l_int32 n );
LEPT_DLL extern PIXAC * pixacompCreateWithInit ( l_int32 n, l_int32 offset, PIX *pix, l_int32 comptype );
LEPT_DLL extern PIXAC * pixacompCreateFromPixa ( PIXA *pixa, l_int32 comptype, l_int32 accesstype );
LEPT_DLL extern PIXAC * pixacompCreateFromFiles ( const char *dirname, const char *substr, l_int32 comptype );
LEPT_DLL extern PIXAC * pixacompCreateFromSA ( SARRAY *sa, l_int32 comptype );
LEPT_DLL extern void pixacompDestroy ( PIXAC **ppixac );
LEPT_DLL extern l_ok pixacompAddPix ( PIXAC *pixac, PIX *pix, l_int32 comptype );
LEPT_DLL extern l_ok pixacompAddPixcomp ( PIXAC *pixac, PIXC *pixc, l_int32 copyflag );
LEPT_DLL extern l_ok pixacompReplacePix ( PIXAC *pixac, l_int32 index, PIX *pix, l_int32 comptype );
LEPT_DLL extern l_ok pixacompReplacePixcomp ( PIXAC *pixac, l_int32 index, PIXC *pixc );
LEPT_DLL extern l_ok pixacompAddBox ( PIXAC *pixac, BOX *box, l_int32 copyflag );
LEPT_DLL extern l_int32 pixacompGetCount ( PIXAC *pixac );
LEPT_DLL extern PIXC * pixacompGetPixcomp ( PIXAC *pixac, l_int32 index, l_int32 copyflag );
LEPT_DLL extern PIX * pixacompGetPix ( PIXAC *pixac, l_int32 index );
LEPT_DLL extern l_ok pixacompGetPixDimensions ( PIXAC *pixac, l_int32 index, l_int32 *pw, l_int32 *ph, l_int32 *pd );
LEPT_DLL extern BOXA * pixacompGetBoxa ( PIXAC *pixac, l_int32 accesstype );
LEPT_DLL extern l_int32 pixacompGetBoxaCount ( PIXAC *pixac );
LEPT_DLL extern BOX * pixacompGetBox ( PIXAC *pixac, l_int32 index, l_int32 accesstype );
LEPT_DLL extern l_ok pixacompGetBoxGeometry ( PIXAC *pixac, l_int32 index, l_int32 *px, l_int32 *py, l_int32 *pw, l_int32 *ph );
LEPT_DLL extern l_int32 pixacompGetOffset ( PIXAC *pixac );
LEPT_DLL extern l_ok pixacompSetOffset ( PIXAC *pixac, l_int32 offset );
LEPT_DLL extern PIXA * pixaCreateFromPixacomp ( PIXAC *pixac, l_int32 accesstype );
LEPT_DLL extern l_ok pixacompJoin ( PIXAC *pixacd, PIXAC *pixacs, l_int32 istart, l_int32 iend );
LEPT_DLL extern PIXAC * pixacompInterleave ( PIXAC *pixac1, PIXAC *pixac2 );
LEPT_DLL extern PIXAC * pixacompRead ( const char *filename );
LEPT_DLL extern PIXAC * pixacompReadStream ( FILE *fp );
LEPT_DLL extern PIXAC * pixacompReadMem ( const l_uint8 *data, size_t size );
LEPT_DLL extern l_ok pixacompWrite ( const char *filename, PIXAC *pixac );
LEPT_DLL extern l_ok pixacompWriteStream ( FILE *fp, PIXAC *pixac );
LEPT_DLL extern l_ok pixacompWriteMem ( l_uint8 **pdata, size_t *psize, PIXAC *pixac );
LEPT_DLL extern l_ok pixacompConvertToPdf ( PIXAC *pixac, l_int32 res, l_float32 scalefactor, l_int32 type, l_int32 quality, const char *title, const char *fileout );
LEPT_DLL extern l_ok pixacompConvertToPdfData ( PIXAC *pixac, l_int32 res, l_float32 scalefactor, l_int32 type, l_int32 quality, const char *title, l_uint8 **pdata, size_t *pnbytes );
LEPT_DLL extern l_ok pixacompFastConvertToPdfData ( PIXAC *pixac, const char *title, l_uint8 **pdata, size_t *pnbytes );
LEPT_DLL extern l_ok pixacompWriteStreamInfo ( FILE *fp, PIXAC *pixac, const char *text );
LEPT_DLL extern l_ok pixcompWriteStreamInfo ( FILE *fp, PIXC *pixc, const char *text );
LEPT_DLL extern PIX * pixacompDisplayTiledAndScaled ( PIXAC *pixac, l_int32 outdepth, l_int32 tilewidth, l_int32 ncols, l_int32 background, l_int32 spacing, l_int32 border );
LEPT_DLL extern l_ok pixacompWriteFiles ( PIXAC *pixac, const char *subdir );
LEPT_DLL extern l_ok pixcompWriteFile ( const char *rootname, PIXC *pixc );
LEPT_DLL extern PIX * pixThreshold8 ( PIX *pixs, l_int32 d, l_int32 nlevels, l_int32 cmapflag );
LEPT_DLL extern PIX * pixRemoveColormapGeneral ( PIX *pixs, l_int32 type, l_int32 ifnocmap );
LEPT_DLL extern PIX * pixRemoveColormap ( PIX *pixs, l_int32 type );
LEPT_DLL extern l_ok pixAddGrayColormap8 ( PIX *pixs );
LEPT_DLL extern PIX * pixAddMinimalGrayColormap8 ( PIX *pixs );
LEPT_DLL extern PIX * pixConvertRGBToLuminance ( PIX *pixs );
LEPT_DLL extern PIX * pixConvertRGBToGray ( PIX *pixs, l_float32 rwt, l_float32 gwt, l_float32 bwt );
LEPT_DLL extern PIX * pixConvertRGBToGrayFast ( PIX *pixs );
LEPT_DLL extern PIX * pixConvertRGBToGrayMinMax ( PIX *pixs, l_int32 type );
LEPT_DLL extern PIX * pixConvertRGBToGraySatBoost ( PIX *pixs, l_int32 refval );
LEPT_DLL extern PIX * pixConvertRGBToGrayArb ( PIX *pixs, l_float32 rc, l_float32 gc, l_float32 bc );
LEPT_DLL extern PIX * pixConvertRGBToBinaryArb ( PIX *pixs, l_float32 rc, l_float32 gc, l_float32 bc, l_int32 thresh, l_int32 relation );
LEPT_DLL extern PIX * pixConvertGrayToColormap ( PIX *pixs );
LEPT_DLL extern PIX * pixConvertGrayToColormap8 ( PIX *pixs, l_int32 mindepth );
LEPT_DLL extern PIX * pixColorizeGray ( PIX *pixs, l_uint32 color, l_int32 cmapflag );
LEPT_DLL extern PIX * pixConvertRGBToColormap ( PIX *pixs, l_int32 ditherflag );
LEPT_DLL extern PIX * pixConvertCmapTo1 ( PIX *pixs );
LEPT_DLL extern l_ok pixQuantizeIfFewColors ( PIX *pixs, l_int32 maxcolors, l_int32 mingraycolors, l_int32 octlevel, PIX **ppixd );
LEPT_DLL extern PIX * pixConvert16To8 ( PIX *pixs, l_int32 type );
LEPT_DLL extern PIX * pixConvertGrayToFalseColor ( PIX *pixs, l_float32 gamma );
LEPT_DLL extern PIX * pixUnpackBinary ( PIX *pixs, l_int32 depth, l_int32 invert );
LEPT_DLL extern PIX * pixConvert1To16 ( PIX *pixd, PIX *pixs, l_uint16 val0, l_uint16 val1 );
LEPT_DLL extern PIX * pixConvert1To32 ( PIX *pixd, PIX *pixs, l_uint32 val0, l_uint32 val1 );
LEPT_DLL extern PIX * pixConvert1To2Cmap ( PIX *pixs );
LEPT_DLL extern PIX * pixConvert1To2 ( PIX *pixd, PIX *pixs, l_int32 val0, l_int32 val1 );
LEPT_DLL extern PIX * pixConvert1To4Cmap ( PIX *pixs );
LEPT_DLL extern PIX * pixConvert1To4 ( PIX *pixd, PIX *pixs, l_int32 val0, l_int32 val1 );
LEPT_DLL extern PIX * pixConvert1To8Cmap ( PIX *pixs );
LEPT_DLL extern PIX * pixConvert1To8 ( PIX *pixd, PIX *pixs, l_uint8 val0, l_uint8 val1 );
LEPT_DLL extern PIX * pixConvert2To8 ( PIX *pixs, l_uint8 val0, l_uint8 val1, l_uint8 val2, l_uint8 val3, l_int32 cmapflag );
LEPT_DLL extern PIX * pixConvert4To8 ( PIX *pixs, l_int32 cmapflag );
LEPT_DLL extern PIX * pixConvert8To16 ( PIX *pixs, l_int32 leftshift );
LEPT_DLL extern PIX * pixConvertTo2 ( PIX *pixs );
LEPT_DLL extern PIX * pixConvert8To2 ( PIX *pix );
LEPT_DLL extern PIX * pixConvertTo4 ( PIX *pixs );
LEPT_DLL extern PIX * pixConvert8To4 ( PIX *pix );
LEPT_DLL extern PIX * pixConvertTo1Adaptive ( PIX *pixs );
LEPT_DLL extern PIX * pixConvertTo1 ( PIX *pixs, l_int32 threshold );
LEPT_DLL extern PIX * pixConvertTo1BySampling ( PIX *pixs, l_int32 factor, l_int32 threshold );
LEPT_DLL extern PIX * pixConvertTo8 ( PIX *pixs, l_int32 cmapflag );
LEPT_DLL extern PIX * pixConvertTo8BySampling ( PIX *pixs, l_int32 factor, l_int32 cmapflag );
LEPT_DLL extern PIX * pixConvertTo8Colormap ( PIX *pixs, l_int32 dither );
LEPT_DLL extern PIX * pixConvertTo16 ( PIX *pixs );
LEPT_DLL extern PIX * pixConvertTo32 ( PIX *pixs );
LEPT_DLL extern PIX * pixConvertTo32BySampling ( PIX *pixs, l_int32 factor );
LEPT_DLL extern PIX * pixConvert8To32 ( PIX *pixs );
LEPT_DLL extern PIX * pixConvertTo8Or32 ( PIX *pixs, l_int32 copyflag, l_int32 warnflag );
LEPT_DLL extern PIX * pixConvert24To32 ( PIX *pixs );
LEPT_DLL extern PIX * pixConvert32To24 ( PIX *pixs );
LEPT_DLL extern PIX * pixConvert32To16 ( PIX *pixs, l_int32 type );
LEPT_DLL extern PIX * pixConvert32To8 ( PIX *pixs, l_int32 type16, l_int32 type8 );
LEPT_DLL extern PIX * pixRemoveAlpha ( PIX *pixs );
LEPT_DLL extern PIX * pixAddAlphaTo1bpp ( PIX *pixd, PIX *pixs );
LEPT_DLL extern PIX * pixConvertLossless ( PIX *pixs, l_int32 d );
LEPT_DLL extern PIX * pixConvertForPSWrap ( PIX *pixs );
LEPT_DLL extern PIX * pixConvertToSubpixelRGB ( PIX *pixs, l_float32 scalex, l_float32 scaley, l_int32 order );
LEPT_DLL extern PIX * pixConvertGrayToSubpixelRGB ( PIX *pixs, l_float32 scalex, l_float32 scaley, l_int32 order );
LEPT_DLL extern PIX * pixConvertColorToSubpixelRGB ( PIX *pixs, l_float32 scalex, l_float32 scaley, l_int32 order );
LEPT_DLL extern void l_setNeutralBoostVal ( l_int32 val );
LEPT_DLL extern PIX * pixConnCompTransform ( PIX *pixs, l_int32 connect, l_int32 depth );
LEPT_DLL extern PIX * pixConnCompAreaTransform ( PIX *pixs, l_int32 connect );
LEPT_DLL extern l_ok pixConnCompIncrInit ( PIX *pixs, l_int32 conn, PIX **ppixd, PTAA **pptaa, l_int32 *pncc );
LEPT_DLL extern l_int32 pixConnCompIncrAdd ( PIX *pixs, PTAA *ptaa, l_int32 *pncc, l_float32 x, l_float32 y, l_int32 debug );
LEPT_DLL extern l_ok pixGetSortedNeighborValues ( PIX *pixs, l_int32 x, l_int32 y, l_int32 conn, l_int32 **pneigh, l_int32 *pnvals );
LEPT_DLL extern PIX * pixLocToColorTransform ( PIX *pixs );
LEPT_DLL extern PIXTILING * pixTilingCreate ( PIX *pixs, l_int32 nx, l_int32 ny, l_int32 w, l_int32 h, l_int32 xoverlap, l_int32 yoverlap );
LEPT_DLL extern void pixTilingDestroy ( PIXTILING **ppt );
LEPT_DLL extern l_ok pixTilingGetCount ( PIXTILING *pt, l_int32 *pnx, l_int32 *pny );
LEPT_DLL extern l_ok pixTilingGetSize ( PIXTILING *pt, l_int32 *pw, l_int32 *ph );
LEPT_DLL extern PIX * pixTilingGetTile ( PIXTILING *pt, l_int32 i, l_int32 j );
LEPT_DLL extern l_ok pixTilingNoStripOnPaint ( PIXTILING *pt );
LEPT_DLL extern l_ok pixTilingPaintTile ( PIX *pixd, l_int32 i, l_int32 j, PIX *pixs, PIXTILING *pt );
LEPT_DLL extern PIX * pixReadStreamPng ( FILE *fp );
LEPT_DLL extern l_ok readHeaderPng ( const char *filename, l_int32 *pw, l_int32 *ph, l_int32 *pbps, l_int32 *pspp, l_int32 *piscmap );
LEPT_DLL extern l_ok freadHeaderPng ( FILE *fp, l_int32 *pw, l_int32 *ph, l_int32 *pbps, l_int32 *pspp, l_int32 *piscmap );
LEPT_DLL extern l_ok readHeaderMemPng ( const l_uint8 *data, size_t size, l_int32 *pw, l_int32 *ph, l_int32 *pbps, l_int32 *pspp, l_int32 *piscmap );
LEPT_DLL extern l_int32 fgetPngResolution ( FILE *fp, l_int32 *pxres, l_int32 *pyres );
LEPT_DLL extern l_ok isPngInterlaced ( const char *filename, l_int32 *pinterlaced );
LEPT_DLL extern l_ok fgetPngColormapInfo ( FILE *fp, PIXCMAP **pcmap, l_int32 *ptransparency );
LEPT_DLL extern l_ok pixWritePng ( const char *filename, PIX *pix, l_float32 gamma );
LEPT_DLL extern l_ok pixWriteStreamPng ( FILE *fp, PIX *pix, l_float32 gamma );
LEPT_DLL extern l_ok pixSetZlibCompression ( PIX *pix, l_int32 compval );
LEPT_DLL extern void l_pngSetReadStrip16To8 ( l_int32 flag );
LEPT_DLL extern PIX * pixReadMemPng ( const l_uint8 *filedata, size_t filesize );
LEPT_DLL extern l_ok pixWriteMemPng ( l_uint8 **pfiledata, size_t *pfilesize, PIX *pix, l_float32 gamma );
LEPT_DLL extern PIX * pixReadStreamPnm ( FILE *fp );
LEPT_DLL extern l_ok readHeaderPnm ( const char *filename, l_int32 *pw, l_int32 *ph, l_int32 *pd, l_int32 *ptype, l_int32 *pbps, l_int32 *pspp );
LEPT_DLL extern l_ok freadHeaderPnm ( FILE *fp, l_int32 *pw, l_int32 *ph, l_int32 *pd, l_int32 *ptype, l_int32 *pbps, l_int32 *pspp );
LEPT_DLL extern l_ok pixWriteStreamPnm ( FILE *fp, PIX *pix );
LEPT_DLL extern l_ok pixWriteStreamAsciiPnm ( FILE *fp, PIX *pix );
LEPT_DLL extern l_ok pixWriteStreamPam ( FILE *fp, PIX *pix );
LEPT_DLL extern PIX * pixReadMemPnm ( const l_uint8 *data, size_t size );
LEPT_DLL extern l_ok readHeaderMemPnm ( const l_uint8 *data, size_t size, l_int32 *pw, l_int32 *ph, l_int32 *pd, l_int32 *ptype, l_int32 *pbps, l_int32 *pspp );
LEPT_DLL extern l_ok pixWriteMemPnm ( l_uint8 **pdata, size_t *psize, PIX *pix );
LEPT_DLL extern l_ok pixWriteMemPam ( l_uint8 **pdata, size_t *psize, PIX *pix );
LEPT_DLL extern PIX * pixProjectiveSampledPta ( PIX *pixs, PTA *ptad, PTA *ptas, l_int32 incolor );
LEPT_DLL extern PIX * pixProjectiveSampled ( PIX *pixs, l_float32 *vc, l_int32 incolor );
LEPT_DLL extern PIX * pixProjectivePta ( PIX *pixs, PTA *ptad, PTA *ptas, l_int32 incolor );
LEPT_DLL extern PIX * pixProjective ( PIX *pixs, l_float32 *vc, l_int32 incolor );
LEPT_DLL extern PIX * pixProjectivePtaColor ( PIX *pixs, PTA *ptad, PTA *ptas, l_uint32 colorval );
LEPT_DLL extern PIX * pixProjectiveColor ( PIX *pixs, l_float32 *vc, l_uint32 colorval );
LEPT_DLL extern PIX * pixProjectivePtaGray ( PIX *pixs, PTA *ptad, PTA *ptas, l_uint8 grayval );
LEPT_DLL extern PIX * pixProjectiveGray ( PIX *pixs, l_float32 *vc, l_uint8 grayval );
LEPT_DLL extern PIX * pixProjectivePtaWithAlpha ( PIX *pixs, PTA *ptad, PTA *ptas, PIX *pixg, l_float32 fract, l_int32 border );
LEPT_DLL extern l_ok getProjectiveXformCoeffs ( PTA *ptas, PTA *ptad, l_float32 **pvc );
LEPT_DLL extern l_ok projectiveXformSampledPt ( l_float32 *vc, l_int32 x, l_int32 y, l_int32 *pxp, l_int32 *pyp );
LEPT_DLL extern l_ok projectiveXformPt ( l_float32 *vc, l_int32 x, l_int32 y, l_float32 *pxp, l_float32 *pyp );
LEPT_DLL extern l_ok convertFilesToPS ( const char *dirin, const char *substr, l_int32 res, const char *fileout );
LEPT_DLL extern l_ok sarrayConvertFilesToPS ( SARRAY *sa, l_int32 res, const char *fileout );
LEPT_DLL extern l_ok convertFilesFittedToPS ( const char *dirin, const char *substr, l_float32 xpts, l_float32 ypts, const char *fileout );
LEPT_DLL extern l_ok sarrayConvertFilesFittedToPS ( SARRAY *sa, l_float32 xpts, l_float32 ypts, const char *fileout );
LEPT_DLL extern l_ok writeImageCompressedToPSFile ( const char *filein, const char *fileout, l_int32 res, l_int32 *pindex );
LEPT_DLL extern l_ok convertSegmentedPagesToPS ( const char *pagedir, const char *pagestr, l_int32 page_numpre, const char *maskdir, const char *maskstr, l_int32 mask_numpre, l_int32 numpost, l_int32 maxnum, l_float32 textscale, l_float32 imagescale, l_int32 threshold, const char *fileout );
LEPT_DLL extern l_ok pixWriteSegmentedPageToPS ( PIX *pixs, PIX *pixm, l_float32 textscale, l_float32 imagescale, l_int32 threshold, l_int32 pageno, const char *fileout );
LEPT_DLL extern l_ok pixWriteMixedToPS ( PIX *pixb, PIX *pixc, l_float32 scale, l_int32 pageno, const char *fileout );
LEPT_DLL extern l_ok convertToPSEmbed ( const char *filein, const char *fileout, l_int32 level );
LEPT_DLL extern l_ok pixaWriteCompressedToPS ( PIXA *pixa, const char *fileout, l_int32 res, l_int32 level );
LEPT_DLL extern l_ok pixWriteCompressedToPS ( PIX *pix, const char *fileout, l_int32 res, l_int32 level, l_int32 *pindex );
LEPT_DLL extern l_ok pixWritePSEmbed ( const char *filein, const char *fileout );
LEPT_DLL extern l_ok pixWriteStreamPS ( FILE *fp, PIX *pix, BOX *box, l_int32 res, l_float32 scale );
LEPT_DLL extern char * pixWriteStringPS ( PIX *pixs, BOX *box, l_int32 res, l_float32 scale );
LEPT_DLL extern char * generateUncompressedPS ( char *hexdata, l_int32 w, l_int32 h, l_int32 d, l_int32 psbpl, l_int32 bps, l_float32 xpt, l_float32 ypt, l_float32 wpt, l_float32 hpt, l_int32 boxflag );
LEPT_DLL extern l_ok convertJpegToPSEmbed ( const char *filein, const char *fileout );
LEPT_DLL extern l_ok convertJpegToPS ( const char *filein, const char *fileout, const char *operation, l_int32 x, l_int32 y, l_int32 res, l_float32 scale, l_int32 pageno, l_int32 endpage );
LEPT_DLL extern l_ok convertG4ToPSEmbed ( const char *filein, const char *fileout );
LEPT_DLL extern l_ok convertG4ToPS ( const char *filein, const char *fileout, const char *operation, l_int32 x, l_int32 y, l_int32 res, l_float32 scale, l_int32 pageno, l_int32 maskflag, l_int32 endpage );
LEPT_DLL extern l_ok convertTiffMultipageToPS ( const char *filein, const char *fileout, l_float32 fillfract );
LEPT_DLL extern l_ok convertFlateToPSEmbed ( const char *filein, const char *fileout );
LEPT_DLL extern l_ok convertFlateToPS ( const char *filein, const char *fileout, const char *operation, l_int32 x, l_int32 y, l_int32 res, l_float32 scale, l_int32 pageno, l_int32 endpage );
LEPT_DLL extern l_ok pixWriteMemPS ( l_uint8 **pdata, size_t *psize, PIX *pix, BOX *box, l_int32 res, l_float32 scale );
LEPT_DLL extern l_int32 getResLetterPage ( l_int32 w, l_int32 h, l_float32 fillfract );
LEPT_DLL extern l_int32 getResA4Page ( l_int32 w, l_int32 h, l_float32 fillfract );
LEPT_DLL extern void l_psWriteBoundingBox ( l_int32 flag );
LEPT_DLL extern PTA * ptaCreate ( l_int32 n );
LEPT_DLL extern PTA * ptaCreateFromNuma ( NUMA *nax, NUMA *nay );
LEPT_DLL extern void ptaDestroy ( PTA **ppta );
LEPT_DLL extern PTA * ptaCopy ( PTA *pta );
LEPT_DLL extern PTA * ptaCopyRange ( PTA *ptas, l_int32 istart, l_int32 iend );
LEPT_DLL extern PTA * ptaClone ( PTA *pta );
LEPT_DLL extern l_ok ptaEmpty ( PTA *pta );
LEPT_DLL extern l_ok ptaAddPt ( PTA *pta, l_float32 x, l_float32 y );
LEPT_DLL extern l_ok ptaInsertPt ( PTA *pta, l_int32 index, l_int32 x, l_int32 y );
LEPT_DLL extern l_ok ptaRemovePt ( PTA *pta, l_int32 index );
LEPT_DLL extern l_int32 ptaGetRefcount ( PTA *pta );
LEPT_DLL extern l_int32 ptaChangeRefcount ( PTA *pta, l_int32 delta );
LEPT_DLL extern l_int32 ptaGetCount ( PTA *pta );
LEPT_DLL extern l_ok ptaGetPt ( PTA *pta, l_int32 index, l_float32 *px, l_float32 *py );
LEPT_DLL extern l_ok ptaGetIPt ( PTA *pta, l_int32 index, l_int32 *px, l_int32 *py );
LEPT_DLL extern l_ok ptaSetPt ( PTA *pta, l_int32 index, l_float32 x, l_float32 y );
LEPT_DLL extern l_ok ptaGetArrays ( PTA *pta, NUMA **pnax, NUMA **pnay );
LEPT_DLL extern PTA * ptaRead ( const char *filename );
LEPT_DLL extern PTA * ptaReadStream ( FILE *fp );
LEPT_DLL extern PTA * ptaReadMem ( const l_uint8 *data, size_t size );
LEPT_DLL extern l_ok ptaWriteDebug ( const char *filename, PTA *pta, l_int32 type );
LEPT_DLL extern l_ok ptaWrite ( const char *filename, PTA *pta, l_int32 type );
LEPT_DLL extern l_ok ptaWriteStream ( FILE *fp, PTA *pta, l_int32 type );
LEPT_DLL extern l_ok ptaWriteMem ( l_uint8 **pdata, size_t *psize, PTA *pta, l_int32 type );
LEPT_DLL extern PTAA * ptaaCreate ( l_int32 n );
LEPT_DLL extern void ptaaDestroy ( PTAA **pptaa );
LEPT_DLL extern l_ok ptaaAddPta ( PTAA *ptaa, PTA *pta, l_int32 copyflag );
LEPT_DLL extern l_int32 ptaaGetCount ( PTAA *ptaa );
LEPT_DLL extern PTA * ptaaGetPta ( PTAA *ptaa, l_int32 index, l_int32 accessflag );
LEPT_DLL extern l_ok ptaaGetPt ( PTAA *ptaa, l_int32 ipta, l_int32 jpt, l_float32 *px, l_float32 *py );
LEPT_DLL extern l_ok ptaaInitFull ( PTAA *ptaa, PTA *pta );
LEPT_DLL extern l_ok ptaaReplacePta ( PTAA *ptaa, l_int32 index, PTA *pta );
LEPT_DLL extern l_ok ptaaAddPt ( PTAA *ptaa, l_int32 ipta, l_float32 x, l_float32 y );
LEPT_DLL extern l_ok ptaaTruncate ( PTAA *ptaa );
LEPT_DLL extern PTAA * ptaaRead ( const char *filename );
LEPT_DLL extern PTAA * ptaaReadStream ( FILE *fp );
LEPT_DLL extern PTAA * ptaaReadMem ( const l_uint8 *data, size_t size );
LEPT_DLL extern l_ok ptaaWriteDebug ( const char *filename, PTAA *ptaa, l_int32 type );
LEPT_DLL extern l_ok ptaaWrite ( const char *filename, PTAA *ptaa, l_int32 type );
LEPT_DLL extern l_ok ptaaWriteStream ( FILE *fp, PTAA *ptaa, l_int32 type );
LEPT_DLL extern l_ok ptaaWriteMem ( l_uint8 **pdata, size_t *psize, PTAA *ptaa, l_int32 type );
LEPT_DLL extern PTA * ptaSubsample ( PTA *ptas, l_int32 subfactor );
LEPT_DLL extern l_ok ptaJoin ( PTA *ptad, PTA *ptas, l_int32 istart, l_int32 iend );
LEPT_DLL extern l_ok ptaaJoin ( PTAA *ptaad, PTAA *ptaas, l_int32 istart, l_int32 iend );
LEPT_DLL extern PTA * ptaReverse ( PTA *ptas, l_int32 type );
LEPT_DLL extern PTA * ptaTranspose ( PTA *ptas );
LEPT_DLL extern PTA * ptaCyclicPerm ( PTA *ptas, l_int32 xs, l_int32 ys );
LEPT_DLL extern PTA * ptaSelectRange ( PTA *ptas, l_int32 first, l_int32 last );
LEPT_DLL extern BOX * ptaGetBoundingRegion ( PTA *pta );
LEPT_DLL extern l_ok ptaGetRange ( PTA *pta, l_float32 *pminx, l_float32 *pmaxx, l_float32 *pminy, l_float32 *pmaxy );
LEPT_DLL extern PTA * ptaGetInsideBox ( PTA *ptas, BOX *box );
LEPT_DLL extern PTA * pixFindCornerPixels ( PIX *pixs );
LEPT_DLL extern l_int32 ptaContainsPt ( PTA *pta, l_int32 x, l_int32 y );
LEPT_DLL extern l_int32 ptaTestIntersection ( PTA *pta1, PTA *pta2 );
LEPT_DLL extern PTA * ptaTransform ( PTA *ptas, l_int32 shiftx, l_int32 shifty, l_float32 scalex, l_float32 scaley );
LEPT_DLL extern l_int32 ptaPtInsidePolygon ( PTA *pta, l_float32 x, l_float32 y, l_int32 *pinside );
LEPT_DLL extern l_float32 l_angleBetweenVectors ( l_float32 x1, l_float32 y1, l_float32 x2, l_float32 y2 );
LEPT_DLL extern l_ok ptaGetMinMax ( PTA *pta, l_float32 *pxmin, l_float32 *pymin, l_float32 *pxmax, l_float32 *pymax );
LEPT_DLL extern PTA * ptaSelectByValue ( PTA *ptas, l_float32 xth, l_float32 yth, l_int32 type, l_int32 relation );
LEPT_DLL extern PTA * ptaCropToMask ( PTA *ptas, PIX *pixm );
LEPT_DLL extern l_ok ptaGetLinearLSF ( PTA *pta, l_float32 *pa, l_float32 *pb, NUMA **pnafit );
LEPT_DLL extern l_ok ptaGetQuadraticLSF ( PTA *pta, l_float32 *pa, l_float32 *pb, l_float32 *pc, NUMA **pnafit );
LEPT_DLL extern l_ok ptaGetCubicLSF ( PTA *pta, l_float32 *pa, l_float32 *pb, l_float32 *pc, l_float32 *pd, NUMA **pnafit );
LEPT_DLL extern l_ok ptaGetQuarticLSF ( PTA *pta, l_float32 *pa, l_float32 *pb, l_float32 *pc, l_float32 *pd, l_float32 *pe, NUMA **pnafit );
LEPT_DLL extern l_ok ptaNoisyLinearLSF ( PTA *pta, l_float32 factor, PTA **pptad, l_float32 *pa, l_float32 *pb, l_float32 *pmederr, NUMA **pnafit );
LEPT_DLL extern l_ok ptaNoisyQuadraticLSF ( PTA *pta, l_float32 factor, PTA **pptad, l_float32 *pa, l_float32 *pb, l_float32 *pc, l_float32 *pmederr, NUMA **pnafit );
LEPT_DLL extern l_ok applyLinearFit ( l_float32 a, l_float32 b, l_float32 x, l_float32 *py );
LEPT_DLL extern l_ok applyQuadraticFit ( l_float32 a, l_float32 b, l_float32 c, l_float32 x, l_float32 *py );
LEPT_DLL extern l_ok applyCubicFit ( l_float32 a, l_float32 b, l_float32 c, l_float32 d, l_float32 x, l_float32 *py );
LEPT_DLL extern l_ok applyQuarticFit ( l_float32 a, l_float32 b, l_float32 c, l_float32 d, l_float32 e, l_float32 x, l_float32 *py );
LEPT_DLL extern l_ok pixPlotAlongPta ( PIX *pixs, PTA *pta, l_int32 outformat, const char *title );
LEPT_DLL extern PTA * ptaGetPixelsFromPix ( PIX *pixs, BOX *box );
LEPT_DLL extern PIX * pixGenerateFromPta ( PTA *pta, l_int32 w, l_int32 h );
LEPT_DLL extern PTA * ptaGetBoundaryPixels ( PIX *pixs, l_int32 type );
LEPT_DLL extern PTAA * ptaaGetBoundaryPixels ( PIX *pixs, l_int32 type, l_int32 connectivity, BOXA **pboxa, PIXA **ppixa );
LEPT_DLL extern PTAA * ptaaIndexLabeledPixels ( PIX *pixs, l_int32 *pncc );
LEPT_DLL extern PTA * ptaGetNeighborPixLocs ( PIX *pixs, l_int32 x, l_int32 y, l_int32 conn );
LEPT_DLL extern PTA * numaConvertToPta1 ( NUMA *na );
LEPT_DLL extern PTA * numaConvertToPta2 ( NUMA *nax, NUMA *nay );
LEPT_DLL extern l_ok ptaConvertToNuma ( PTA *pta, NUMA **pnax, NUMA **pnay );
LEPT_DLL extern PIX * pixDisplayPta ( PIX *pixd, PIX *pixs, PTA *pta );
LEPT_DLL extern PIX * pixDisplayPtaaPattern ( PIX *pixd, PIX *pixs, PTAA *ptaa, PIX *pixp, l_int32 cx, l_int32 cy );
LEPT_DLL extern PIX * pixDisplayPtaPattern ( PIX *pixd, PIX *pixs, PTA *pta, PIX *pixp, l_int32 cx, l_int32 cy, l_uint32 color );
LEPT_DLL extern PTA * ptaReplicatePattern ( PTA *ptas, PIX *pixp, PTA *ptap, l_int32 cx, l_int32 cy, l_int32 w, l_int32 h );
LEPT_DLL extern PIX * pixDisplayPtaa ( PIX *pixs, PTAA *ptaa );
LEPT_DLL extern PTA * ptaSort ( PTA *ptas, l_int32 sorttype, l_int32 sortorder, NUMA **pnaindex );
LEPT_DLL extern l_ok ptaGetSortIndex ( PTA *ptas, l_int32 sorttype, l_int32 sortorder, NUMA **pnaindex );
LEPT_DLL extern PTA * ptaSortByIndex ( PTA *ptas, NUMA *naindex );
LEPT_DLL extern PTAA * ptaaSortByIndex ( PTAA *ptaas, NUMA *naindex );
LEPT_DLL extern l_ok ptaGetRankValue ( PTA *pta, l_float32 fract, PTA *ptasort, l_int32 sorttype, l_float32 *pval );
LEPT_DLL extern PTA * ptaSort2d ( PTA *pta );
LEPT_DLL extern l_ok ptaEqual ( PTA *pta1, PTA *pta2, l_int32 *psame );
LEPT_DLL extern PTA * ptaUnionByAset ( PTA *pta1, PTA *pta2 );
LEPT_DLL extern PTA * ptaRemoveDupsByAset ( PTA *ptas );
LEPT_DLL extern PTA * ptaIntersectionByAset ( PTA *pta1, PTA *pta2 );
LEPT_DLL extern L_ASET * l_asetCreateFromPta ( PTA *pta );
LEPT_DLL extern PTA * ptaUnionByHash ( PTA *pta1, PTA *pta2 );
LEPT_DLL extern l_ok ptaRemoveDupsByHash ( PTA *ptas, PTA **pptad, L_DNAHASH **pdahash );
LEPT_DLL extern PTA * ptaIntersectionByHash ( PTA *pta1, PTA *pta2 );
LEPT_DLL extern l_ok ptaFindPtByHash ( PTA *pta, L_DNAHASH *dahash, l_int32 x, l_int32 y, l_int32 *pindex );
LEPT_DLL extern L_DNAHASH * l_dnaHashCreateFromPta ( PTA *pta );
LEPT_DLL extern L_PTRA * ptraCreate ( l_int32 n );
LEPT_DLL extern void ptraDestroy ( L_PTRA **ppa, l_int32 freeflag, l_int32 warnflag );
LEPT_DLL extern l_ok ptraAdd ( L_PTRA *pa, void *item );
LEPT_DLL extern l_ok ptraInsert ( L_PTRA *pa, l_int32 index, void *item, l_int32 shiftflag );
LEPT_DLL extern void * ptraRemove ( L_PTRA *pa, l_int32 index, l_int32 flag );
LEPT_DLL extern void * ptraRemoveLast ( L_PTRA *pa );
LEPT_DLL extern void * ptraReplace ( L_PTRA *pa, l_int32 index, void *item, l_int32 freeflag );
LEPT_DLL extern l_ok ptraSwap ( L_PTRA *pa, l_int32 index1, l_int32 index2 );
LEPT_DLL extern l_ok ptraCompactArray ( L_PTRA *pa );
LEPT_DLL extern l_ok ptraReverse ( L_PTRA *pa );
LEPT_DLL extern l_ok ptraJoin ( L_PTRA *pa1, L_PTRA *pa2 );
LEPT_DLL extern l_ok ptraGetMaxIndex ( L_PTRA *pa, l_int32 *pmaxindex );
LEPT_DLL extern l_ok ptraGetActualCount ( L_PTRA *pa, l_int32 *pcount );
LEPT_DLL extern void * ptraGetPtrToItem ( L_PTRA *pa, l_int32 index );
LEPT_DLL extern L_PTRAA * ptraaCreate ( l_int32 n );
LEPT_DLL extern void ptraaDestroy ( L_PTRAA **ppaa, l_int32 freeflag, l_int32 warnflag );
LEPT_DLL extern l_ok ptraaGetSize ( L_PTRAA *paa, l_int32 *psize );
LEPT_DLL extern l_ok ptraaInsertPtra ( L_PTRAA *paa, l_int32 index, L_PTRA *pa );
LEPT_DLL extern L_PTRA * ptraaGetPtra ( L_PTRAA *paa, l_int32 index, l_int32 accessflag );
LEPT_DLL extern L_PTRA * ptraaFlattenToPtra ( L_PTRAA *paa );
LEPT_DLL extern l_ok pixQuadtreeMean ( PIX *pixs, l_int32 nlevels, PIX *pix_ma, FPIXA **pfpixa );
LEPT_DLL extern l_ok pixQuadtreeVariance ( PIX *pixs, l_int32 nlevels, PIX *pix_ma, DPIX *dpix_msa, FPIXA **pfpixa_v, FPIXA **pfpixa_rv );
LEPT_DLL extern l_ok pixMeanInRectangle ( PIX *pixs, BOX *box, PIX *pixma, l_float32 *pval );
LEPT_DLL extern l_ok pixVarianceInRectangle ( PIX *pixs, BOX *box, PIX *pix_ma, DPIX *dpix_msa, l_float32 *pvar, l_float32 *prvar );
LEPT_DLL extern BOXAA * boxaaQuadtreeRegions ( l_int32 w, l_int32 h, l_int32 nlevels );
LEPT_DLL extern l_ok quadtreeGetParent ( FPIXA *fpixa, l_int32 level, l_int32 x, l_int32 y, l_float32 *pval );
LEPT_DLL extern l_ok quadtreeGetChildren ( FPIXA *fpixa, l_int32 level, l_int32 x, l_int32 y, l_float32 *pval00, l_float32 *pval10, l_float32 *pval01, l_float32 *pval11 );
LEPT_DLL extern l_int32 quadtreeMaxLevels ( l_int32 w, l_int32 h );
LEPT_DLL extern PIX * fpixaDisplayQuadtree ( FPIXA *fpixa, l_int32 factor, l_int32 fontsize );
LEPT_DLL extern L_QUEUE * lqueueCreate ( l_int32 nalloc );
LEPT_DLL extern void lqueueDestroy ( L_QUEUE **plq, l_int32 freeflag );
LEPT_DLL extern l_ok lqueueAdd ( L_QUEUE *lq, void *item );
LEPT_DLL extern void * lqueueRemove ( L_QUEUE *lq );
LEPT_DLL extern l_int32 lqueueGetCount ( L_QUEUE *lq );
LEPT_DLL extern l_ok lqueuePrint ( FILE *fp, L_QUEUE *lq );
LEPT_DLL extern PIX * pixRankFilter ( PIX *pixs, l_int32 wf, l_int32 hf, l_float32 rank );
LEPT_DLL extern PIX * pixRankFilterRGB ( PIX *pixs, l_int32 wf, l_int32 hf, l_float32 rank );
LEPT_DLL extern PIX * pixRankFilterGray ( PIX *pixs, l_int32 wf, l_int32 hf, l_float32 rank );
LEPT_DLL extern PIX * pixMedianFilter ( PIX *pixs, l_int32 wf, l_int32 hf );
LEPT_DLL extern PIX * pixRankFilterWithScaling ( PIX *pixs, l_int32 wf, l_int32 hf, l_float32 rank, l_float32 scalefactor );
LEPT_DLL extern L_RBTREE * l_rbtreeCreate ( l_int32 keytype );
LEPT_DLL extern RB_TYPE * l_rbtreeLookup ( L_RBTREE *t, RB_TYPE key );
LEPT_DLL extern void l_rbtreeInsert ( L_RBTREE *t, RB_TYPE key, RB_TYPE value );
LEPT_DLL extern void l_rbtreeDelete ( L_RBTREE *t, RB_TYPE key );
LEPT_DLL extern void l_rbtreeDestroy ( L_RBTREE **pt );
LEPT_DLL extern L_RBTREE_NODE * l_rbtreeGetFirst ( L_RBTREE *t );
LEPT_DLL extern L_RBTREE_NODE * l_rbtreeGetNext ( L_RBTREE_NODE *n );
LEPT_DLL extern L_RBTREE_NODE * l_rbtreeGetLast ( L_RBTREE *t );
LEPT_DLL extern L_RBTREE_NODE * l_rbtreeGetPrev ( L_RBTREE_NODE *n );
LEPT_DLL extern l_int32 l_rbtreeGetCount ( L_RBTREE *t );
LEPT_DLL extern void l_rbtreePrint ( FILE *fp, L_RBTREE *t );
LEPT_DLL extern SARRAY * pixProcessBarcodes ( PIX *pixs, l_int32 format, l_int32 method, SARRAY **psaw, l_int32 debugflag );
LEPT_DLL extern PIXA * pixExtractBarcodes ( PIX *pixs, l_int32 debugflag );
LEPT_DLL extern SARRAY * pixReadBarcodes ( PIXA *pixa, l_int32 format, l_int32 method, SARRAY **psaw, l_int32 debugflag );
LEPT_DLL extern NUMA * pixReadBarcodeWidths ( PIX *pixs, l_int32 method, l_int32 debugflag );
LEPT_DLL extern BOXA * pixLocateBarcodes ( PIX *pixs, l_int32 thresh, PIX **ppixb, PIX **ppixm );
LEPT_DLL extern PIX * pixDeskewBarcode ( PIX *pixs, PIX *pixb, BOX *box, l_int32 margin, l_int32 threshold, l_float32 *pangle, l_float32 *pconf );
LEPT_DLL extern NUMA * pixExtractBarcodeWidths1 ( PIX *pixs, l_float32 thresh, l_float32 binfract, NUMA **pnaehist, NUMA **pnaohist, l_int32 debugflag );
LEPT_DLL extern NUMA * pixExtractBarcodeWidths2 ( PIX *pixs, l_float32 thresh, l_float32 *pwidth, NUMA **pnac, l_int32 debugflag );
LEPT_DLL extern NUMA * pixExtractBarcodeCrossings ( PIX *pixs, l_float32 thresh, l_int32 debugflag );
LEPT_DLL extern NUMA * numaQuantizeCrossingsByWidth ( NUMA *nas, l_float32 binfract, NUMA **pnaehist, NUMA **pnaohist, l_int32 debugflag );
LEPT_DLL extern NUMA * numaQuantizeCrossingsByWindow ( NUMA *nas, l_float32 ratio, l_float32 *pwidth, l_float32 *pfirstloc, NUMA **pnac, l_int32 debugflag );
LEPT_DLL extern PIXA * pixaReadFiles ( const char *dirname, const char *substr );
LEPT_DLL extern PIXA * pixaReadFilesSA ( SARRAY *sa );
LEPT_DLL extern PIX * pixRead ( const char *filename );
LEPT_DLL extern PIX * pixReadWithHint ( const char *filename, l_int32 hint );
LEPT_DLL extern PIX * pixReadIndexed ( SARRAY *sa, l_int32 index );
LEPT_DLL extern PIX * pixReadStream ( FILE *fp, l_int32 hint );
LEPT_DLL extern l_ok pixReadHeader ( const char *filename, l_int32 *pformat, l_int32 *pw, l_int32 *ph, l_int32 *pbps, l_int32 *pspp, l_int32 *piscmap );
LEPT_DLL extern l_ok findFileFormat ( const char *filename, l_int32 *pformat );
LEPT_DLL extern l_ok findFileFormatStream ( FILE *fp, l_int32 *pformat );
LEPT_DLL extern l_ok findFileFormatBuffer ( const l_uint8 *buf, l_int32 *pformat );
LEPT_DLL extern l_int32 fileFormatIsTiff ( FILE *fp );
LEPT_DLL extern PIX * pixReadMem ( const l_uint8 *data, size_t size );
LEPT_DLL extern l_ok pixReadHeaderMem ( const l_uint8 *data, size_t size, l_int32 *pformat, l_int32 *pw, l_int32 *ph, l_int32 *pbps, l_int32 *pspp, l_int32 *piscmap );
LEPT_DLL extern l_ok writeImageFileInfo ( const char *filename, FILE *fpout, l_int32 headeronly );
LEPT_DLL extern l_ok ioFormatTest ( const char *filename );
LEPT_DLL extern L_RECOG * recogCreateFromRecog ( L_RECOG *recs, l_int32 scalew, l_int32 scaleh, l_int32 linew, l_int32 threshold, l_int32 maxyshift );
LEPT_DLL extern L_RECOG * recogCreateFromPixa ( PIXA *pixa, l_int32 scalew, l_int32 scaleh, l_int32 linew, l_int32 threshold, l_int32 maxyshift );
LEPT_DLL extern L_RECOG * recogCreateFromPixaNoFinish ( PIXA *pixa, l_int32 scalew, l_int32 scaleh, l_int32 linew, l_int32 threshold, l_int32 maxyshift );
LEPT_DLL extern L_RECOG * recogCreate ( l_int32 scalew, l_int32 scaleh, l_int32 linew, l_int32 threshold, l_int32 maxyshift );
LEPT_DLL extern void recogDestroy ( L_RECOG **precog );
LEPT_DLL extern l_int32 recogGetCount ( L_RECOG *recog );
LEPT_DLL extern l_ok recogSetParams ( L_RECOG *recog, l_int32 type, l_int32 min_nopad, l_float32 max_wh_ratio, l_float32 max_ht_ratio );
LEPT_DLL extern l_int32 recogGetClassIndex ( L_RECOG *recog, l_int32 val, char *text, l_int32 *pindex );
LEPT_DLL extern l_ok recogStringToIndex ( L_RECOG *recog, char *text, l_int32 *pindex );
LEPT_DLL extern l_int32 recogGetClassString ( L_RECOG *recog, l_int32 index, char **pcharstr );
LEPT_DLL extern l_ok l_convertCharstrToInt ( const char *str, l_int32 *pval );
LEPT_DLL extern L_RECOG * recogRead ( const char *filename );
LEPT_DLL extern L_RECOG * recogReadStream ( FILE *fp );
LEPT_DLL extern L_RECOG * recogReadMem ( const l_uint8 *data, size_t size );
LEPT_DLL extern l_ok recogWrite ( const char *filename, L_RECOG *recog );
LEPT_DLL extern l_ok recogWriteStream ( FILE *fp, L_RECOG *recog );
LEPT_DLL extern l_ok recogWriteMem ( l_uint8 **pdata, size_t *psize, L_RECOG *recog );
LEPT_DLL extern PIXA * recogExtractPixa ( L_RECOG *recog );
LEPT_DLL extern BOXA * recogDecode ( L_RECOG *recog, PIX *pixs, l_int32 nlevels, PIX **ppixdb );
LEPT_DLL extern l_ok recogCreateDid ( L_RECOG *recog, PIX *pixs );
LEPT_DLL extern l_ok recogDestroyDid ( L_RECOG *recog );
LEPT_DLL extern l_int32 recogDidExists ( L_RECOG *recog );
LEPT_DLL extern L_RDID * recogGetDid ( L_RECOG *recog );
LEPT_DLL extern l_ok recogSetChannelParams ( L_RECOG *recog, l_int32 nlevels );
LEPT_DLL extern l_ok recogIdentifyMultiple ( L_RECOG *recog, PIX *pixs, l_int32 minh, l_int32 skipsplit, BOXA **pboxa, PIXA **ppixa, PIX **ppixdb, l_int32 debugsplit );
LEPT_DLL extern l_ok recogSplitIntoCharacters ( L_RECOG *recog, PIX *pixs, l_int32 minh, l_int32 skipsplit, BOXA **pboxa, PIXA **ppixa, l_int32 debug );
LEPT_DLL extern l_ok recogCorrelationBestRow ( L_RECOG *recog, PIX *pixs, BOXA **pboxa, NUMA **pnascore, NUMA **pnaindex, SARRAY **psachar, l_int32 debug );
LEPT_DLL extern l_ok recogCorrelationBestChar ( L_RECOG *recog, PIX *pixs, BOX **pbox, l_float32 *pscore, l_int32 *pindex, char **pcharstr, PIX **ppixdb );
LEPT_DLL extern l_ok recogIdentifyPixa ( L_RECOG *recog, PIXA *pixa, PIX **ppixdb );
LEPT_DLL extern l_ok recogIdentifyPix ( L_RECOG *recog, PIX *pixs, PIX **ppixdb );
LEPT_DLL extern l_ok recogSkipIdentify ( L_RECOG *recog );
LEPT_DLL extern void rchaDestroy ( L_RCHA **prcha );
LEPT_DLL extern void rchDestroy ( L_RCH **prch );
LEPT_DLL extern l_ok rchaExtract ( L_RCHA *rcha, NUMA **pnaindex, NUMA **pnascore, SARRAY **psatext, NUMA **pnasample, NUMA **pnaxloc, NUMA **pnayloc, NUMA **pnawidth );
LEPT_DLL extern l_ok rchExtract ( L_RCH *rch, l_int32 *pindex, l_float32 *pscore, char **ptext, l_int32 *psample, l_int32 *pxloc, l_int32 *pyloc, l_int32 *pwidth );
LEPT_DLL extern PIX * recogProcessToIdentify ( L_RECOG *recog, PIX *pixs, l_int32 pad );
LEPT_DLL extern SARRAY * recogExtractNumbers ( L_RECOG *recog, BOXA *boxas, l_float32 scorethresh, l_int32 spacethresh, BOXAA **pbaa, NUMAA **pnaa );
LEPT_DLL extern PIXA * showExtractNumbers ( PIX *pixs, SARRAY *sa, BOXAA *baa, NUMAA *naa, PIX **ppixdb );
LEPT_DLL extern l_ok recogTrainLabeled ( L_RECOG *recog, PIX *pixs, BOX *box, char *text, l_int32 debug );
LEPT_DLL extern l_ok recogProcessLabeled ( L_RECOG *recog, PIX *pixs, BOX *box, char *text, PIX **ppix );
LEPT_DLL extern l_ok recogAddSample ( L_RECOG *recog, PIX *pix, l_int32 debug );
LEPT_DLL extern PIX * recogModifyTemplate ( L_RECOG *recog, PIX *pixs );
LEPT_DLL extern l_int32 recogAverageSamples ( L_RECOG **precog, l_int32 debug );
LEPT_DLL extern l_int32 pixaAccumulateSamples ( PIXA *pixa, PTA *pta, PIX **ppixd, l_float32 *px, l_float32 *py );
LEPT_DLL extern l_ok recogTrainingFinished ( L_RECOG **precog, l_int32 modifyflag, l_int32 minsize, l_float32 minfract );
LEPT_DLL extern PIXA * recogFilterPixaBySize ( PIXA *pixas, l_int32 setsize, l_int32 maxkeep, l_float32 max_ht_ratio, NUMA **pna );
LEPT_DLL extern PIXAA * recogSortPixaByClass ( PIXA *pixa, l_int32 setsize );
LEPT_DLL extern l_ok recogRemoveOutliers1 ( L_RECOG **precog, l_float32 minscore, l_int32 mintarget, l_int32 minsize, PIX **ppixsave, PIX **ppixrem );
LEPT_DLL extern PIXA * pixaRemoveOutliers1 ( PIXA *pixas, l_float32 minscore, l_int32 mintarget, l_int32 minsize, PIX **ppixsave, PIX **ppixrem );
LEPT_DLL extern l_ok recogRemoveOutliers2 ( L_RECOG **precog, l_float32 minscore, l_int32 minsize, PIX **ppixsave, PIX **ppixrem );
LEPT_DLL extern PIXA * pixaRemoveOutliers2 ( PIXA *pixas, l_float32 minscore, l_int32 minsize, PIX **ppixsave, PIX **ppixrem );
LEPT_DLL extern PIXA * recogTrainFromBoot ( L_RECOG *recogboot, PIXA *pixas, l_float32 minscore, l_int32 threshold, l_int32 debug );
LEPT_DLL extern l_ok recogPadDigitTrainingSet ( L_RECOG **precog, l_int32 scaleh, l_int32 linew );
LEPT_DLL extern l_int32 recogIsPaddingNeeded ( L_RECOG *recog, SARRAY **psa );
LEPT_DLL extern PIXA * recogAddDigitPadTemplates ( L_RECOG *recog, SARRAY *sa );
LEPT_DLL extern L_RECOG * recogMakeBootDigitRecog ( l_int32 nsamp, l_int32 scaleh, l_int32 linew, l_int32 maxyshift, l_int32 debug );
LEPT_DLL extern PIXA * recogMakeBootDigitTemplates ( l_int32 nsamp, l_int32 debug );
LEPT_DLL extern l_ok recogShowContent ( FILE *fp, L_RECOG *recog, l_int32 index, l_int32 display );
LEPT_DLL extern l_ok recogDebugAverages ( L_RECOG **precog, l_int32 debug );
LEPT_DLL extern l_int32 recogShowAverageTemplates ( L_RECOG *recog );
LEPT_DLL extern l_ok recogShowMatchesInRange ( L_RECOG *recog, PIXA *pixa, l_float32 minscore, l_float32 maxscore, l_int32 display );
LEPT_DLL extern PIX * recogShowMatch ( L_RECOG *recog, PIX *pix1, PIX *pix2, BOX *box, l_int32 index, l_float32 score );
LEPT_DLL extern l_ok regTestSetup ( l_int32 argc, char **argv, L_REGPARAMS **prp );
LEPT_DLL extern l_ok regTestCleanup ( L_REGPARAMS *rp );
LEPT_DLL extern l_ok regTestCompareValues ( L_REGPARAMS *rp, l_float32 val1, l_float32 val2, l_float32 delta );
LEPT_DLL extern l_ok regTestCompareStrings ( L_REGPARAMS *rp, l_uint8 *string1, size_t bytes1, l_uint8 *string2, size_t bytes2 );
LEPT_DLL extern l_ok regTestComparePix ( L_REGPARAMS *rp, PIX *pix1, PIX *pix2 );
LEPT_DLL extern l_ok regTestCompareSimilarPix ( L_REGPARAMS *rp, PIX *pix1, PIX *pix2, l_int32 mindiff, l_float32 maxfract, l_int32 printstats );
LEPT_DLL extern l_ok regTestCheckFile ( L_REGPARAMS *rp, const char *localname );
LEPT_DLL extern l_ok regTestCompareFiles ( L_REGPARAMS *rp, l_int32 index1, l_int32 index2 );
LEPT_DLL extern l_ok regTestWritePixAndCheck ( L_REGPARAMS *rp, PIX *pix, l_int32 format );
LEPT_DLL extern l_ok regTestWriteDataAndCheck ( L_REGPARAMS *rp, void *data, size_t nbytes, const char *ext );
LEPT_DLL extern char * regTestGenLocalFilename ( L_REGPARAMS *rp, l_int32 index, l_int32 format );
LEPT_DLL extern l_ok pixRasterop ( PIX *pixd, l_int32 dx, l_int32 dy, l_int32 dw, l_int32 dh, l_int32 op, PIX *pixs, l_int32 sx, l_int32 sy );
LEPT_DLL extern l_ok pixRasteropVip ( PIX *pixd, l_int32 bx, l_int32 bw, l_int32 vshift, l_int32 incolor );
LEPT_DLL extern l_ok pixRasteropHip ( PIX *pixd, l_int32 by, l_int32 bh, l_int32 hshift, l_int32 incolor );
LEPT_DLL extern PIX * pixTranslate ( PIX *pixd, PIX *pixs, l_int32 hshift, l_int32 vshift, l_int32 incolor );
LEPT_DLL extern l_ok pixRasteropIP ( PIX *pixd, l_int32 hshift, l_int32 vshift, l_int32 incolor );
LEPT_DLL extern l_ok pixRasteropFullImage ( PIX *pixd, PIX *pixs, l_int32 op );
LEPT_DLL extern void rasteropUniLow ( l_uint32 *datad, l_int32 dpixw, l_int32 dpixh, l_int32 depth, l_int32 dwpl, l_int32 dx, l_int32 dy, l_int32 dw, l_int32 dh, l_int32 op );
LEPT_DLL extern void rasteropLow ( l_uint32 *datad, l_int32 dpixw, l_int32 dpixh, l_int32 depth, l_int32 dwpl, l_int32 dx, l_int32 dy, l_int32 dw, l_int32 dh, l_int32 op, l_uint32 *datas, l_int32 spixw, l_int32 spixh, l_int32 swpl, l_int32 sx, l_int32 sy );
LEPT_DLL extern void rasteropVipLow ( l_uint32 *data, l_int32 pixw, l_int32 pixh, l_int32 depth, l_int32 wpl, l_int32 x, l_int32 w, l_int32 shift );
LEPT_DLL extern void rasteropHipLow ( l_uint32 *data, l_int32 pixh, l_int32 depth, l_int32 wpl, l_int32 y, l_int32 h, l_int32 shift );
LEPT_DLL extern PIX * pixRotate ( PIX *pixs, l_float32 angle, l_int32 type, l_int32 incolor, l_int32 width, l_int32 height );
LEPT_DLL extern PIX * pixEmbedForRotation ( PIX *pixs, l_float32 angle, l_int32 incolor, l_int32 width, l_int32 height );
LEPT_DLL extern PIX * pixRotateBySampling ( PIX *pixs, l_int32 xcen, l_int32 ycen, l_float32 angle, l_int32 incolor );
LEPT_DLL extern PIX * pixRotateBinaryNice ( PIX *pixs, l_float32 angle, l_int32 incolor );
LEPT_DLL extern PIX * pixRotateWithAlpha ( PIX *pixs, l_float32 angle, PIX *pixg, l_float32 fract );
LEPT_DLL extern PIX * pixRotateAM ( PIX *pixs, l_float32 angle, l_int32 incolor );
LEPT_DLL extern PIX * pixRotateAMColor ( PIX *pixs, l_float32 angle, l_uint32 colorval );
LEPT_DLL extern PIX * pixRotateAMGray ( PIX *pixs, l_float32 angle, l_uint8 grayval );
LEPT_DLL extern PIX * pixRotateAMCorner ( PIX *pixs, l_float32 angle, l_int32 incolor );
LEPT_DLL extern PIX * pixRotateAMColorCorner ( PIX *pixs, l_float32 angle, l_uint32 fillval );
LEPT_DLL extern PIX * pixRotateAMGrayCorner ( PIX *pixs, l_float32 angle, l_uint8 grayval );
LEPT_DLL extern PIX * pixRotateAMColorFast ( PIX *pixs, l_float32 angle, l_uint32 colorval );
LEPT_DLL extern PIX * pixRotateOrth ( PIX *pixs, l_int32 quads );
LEPT_DLL extern PIX * pixRotate180 ( PIX *pixd, PIX *pixs );
LEPT_DLL extern PIX * pixRotate90 ( PIX *pixs, l_int32 direction );
LEPT_DLL extern PIX * pixFlipLR ( PIX *pixd, PIX *pixs );
LEPT_DLL extern PIX * pixFlipTB ( PIX *pixd, PIX *pixs );
LEPT_DLL extern PIX * pixRotateShear ( PIX *pixs, l_int32 xcen, l_int32 ycen, l_float32 angle, l_int32 incolor );
LEPT_DLL extern PIX * pixRotate2Shear ( PIX *pixs, l_int32 xcen, l_int32 ycen, l_float32 angle, l_int32 incolor );
LEPT_DLL extern PIX * pixRotate3Shear ( PIX *pixs, l_int32 xcen, l_int32 ycen, l_float32 angle, l_int32 incolor );
LEPT_DLL extern l_ok pixRotateShearIP ( PIX *pixs, l_int32 xcen, l_int32 ycen, l_float32 angle, l_int32 incolor );
LEPT_DLL extern PIX * pixRotateShearCenter ( PIX *pixs, l_float32 angle, l_int32 incolor );
LEPT_DLL extern l_ok pixRotateShearCenterIP ( PIX *pixs, l_float32 angle, l_int32 incolor );
LEPT_DLL extern PIX * pixStrokeWidthTransform ( PIX *pixs, l_int32 color, l_int32 depth, l_int32 nangles );
LEPT_DLL extern PIX * pixRunlengthTransform ( PIX *pixs, l_int32 color, l_int32 direction, l_int32 depth );
LEPT_DLL extern l_ok pixFindHorizontalRuns ( PIX *pix, l_int32 y, l_int32 *xstart, l_int32 *xend, l_int32 *pn );
LEPT_DLL extern l_ok pixFindVerticalRuns ( PIX *pix, l_int32 x, l_int32 *ystart, l_int32 *yend, l_int32 *pn );
LEPT_DLL extern NUMA * pixFindMaxRuns ( PIX *pix, l_int32 direction, NUMA **pnastart );
LEPT_DLL extern l_ok pixFindMaxHorizontalRunOnLine ( PIX *pix, l_int32 y, l_int32 *pxstart, l_int32 *psize );
LEPT_DLL extern l_ok pixFindMaxVerticalRunOnLine ( PIX *pix, l_int32 x, l_int32 *pystart, l_int32 *psize );
LEPT_DLL extern l_ok runlengthMembershipOnLine ( l_int32 *buffer, l_int32 size, l_int32 depth, l_int32 *start, l_int32 *end, l_int32 n );
LEPT_DLL extern l_int32 * makeMSBitLocTab ( l_int32 bitval );
LEPT_DLL extern SARRAY * sarrayCreate ( l_int32 n );
LEPT_DLL extern SARRAY * sarrayCreateInitialized ( l_int32 n, const char *initstr );
LEPT_DLL extern SARRAY * sarrayCreateWordsFromString ( const char *string );
LEPT_DLL extern SARRAY * sarrayCreateLinesFromString ( const char *string, l_int32 blankflag );
LEPT_DLL extern void sarrayDestroy ( SARRAY **psa );
LEPT_DLL extern SARRAY * sarrayCopy ( SARRAY *sa );
LEPT_DLL extern SARRAY * sarrayClone ( SARRAY *sa );
LEPT_DLL extern l_ok sarrayAddString ( SARRAY *sa, const char *string, l_int32 copyflag );
LEPT_DLL extern char * sarrayRemoveString ( SARRAY *sa, l_int32 index );
LEPT_DLL extern l_ok sarrayReplaceString ( SARRAY *sa, l_int32 index, char *newstr, l_int32 copyflag );
LEPT_DLL extern l_ok sarrayClear ( SARRAY *sa );
LEPT_DLL extern l_int32 sarrayGetCount ( SARRAY *sa );
LEPT_DLL extern char ** sarrayGetArray ( SARRAY *sa, l_int32 *pnalloc, l_int32 *pn );
LEPT_DLL extern char * sarrayGetString ( SARRAY *sa, l_int32 index, l_int32 copyflag );
LEPT_DLL extern l_int32 sarrayGetRefcount ( SARRAY *sa );
LEPT_DLL extern l_ok sarrayChangeRefcount ( SARRAY *sa, l_int32 delta );
LEPT_DLL extern char * sarrayToString ( SARRAY *sa, l_int32 addnlflag );
LEPT_DLL extern char * sarrayToStringRange ( SARRAY *sa, l_int32 first, l_int32 nstrings, l_int32 addnlflag );
LEPT_DLL extern l_ok sarrayJoin ( SARRAY *sa1, SARRAY *sa2 );
LEPT_DLL extern l_ok sarrayAppendRange ( SARRAY *sa1, SARRAY *sa2, l_int32 start, l_int32 end );
LEPT_DLL extern l_ok sarrayPadToSameSize ( SARRAY *sa1, SARRAY *sa2, const char *padstring );
LEPT_DLL extern SARRAY * sarrayConvertWordsToLines ( SARRAY *sa, l_int32 linesize );
LEPT_DLL extern l_int32 sarraySplitString ( SARRAY *sa, const char *str, const char *separators );
LEPT_DLL extern SARRAY * sarraySelectBySubstring ( SARRAY *sain, const char *substr );
LEPT_DLL extern SARRAY * sarraySelectByRange ( SARRAY *sain, l_int32 first, l_int32 last );
LEPT_DLL extern l_int32 sarrayParseRange ( SARRAY *sa, l_int32 start, l_int32 *pactualstart, l_int32 *pend, l_int32 *pnewstart, const char *substr, l_int32 loc );
LEPT_DLL extern SARRAY * sarrayRead ( const char *filename );
LEPT_DLL extern SARRAY * sarrayReadStream ( FILE *fp );
LEPT_DLL extern SARRAY * sarrayReadMem ( const l_uint8 *data, size_t size );
LEPT_DLL extern l_ok sarrayWrite ( const char *filename, SARRAY *sa );
LEPT_DLL extern l_ok sarrayWriteStream ( FILE *fp, SARRAY *sa );
LEPT_DLL extern l_ok sarrayWriteMem ( l_uint8 **pdata, size_t *psize, SARRAY *sa );
LEPT_DLL extern l_ok sarrayAppend ( const char *filename, SARRAY *sa );
LEPT_DLL extern SARRAY * getNumberedPathnamesInDirectory ( const char *dirname, const char *substr, l_int32 numpre, l_int32 numpost, l_int32 maxnum );
LEPT_DLL extern SARRAY * getSortedPathnamesInDirectory ( const char *dirname, const char *substr, l_int32 first, l_int32 nfiles );
LEPT_DLL extern SARRAY * convertSortedToNumberedPathnames ( SARRAY *sa, l_int32 numpre, l_int32 numpost, l_int32 maxnum );
LEPT_DLL extern SARRAY * getFilenamesInDirectory ( const char *dirname );
LEPT_DLL extern SARRAY * sarraySort ( SARRAY *saout, SARRAY *sain, l_int32 sortorder );
LEPT_DLL extern SARRAY * sarraySortByIndex ( SARRAY *sain, NUMA *naindex );
LEPT_DLL extern l_int32 stringCompareLexical ( const char *str1, const char *str2 );
LEPT_DLL extern SARRAY * sarrayUnionByAset ( SARRAY *sa1, SARRAY *sa2 );
LEPT_DLL extern SARRAY * sarrayRemoveDupsByAset ( SARRAY *sas );
LEPT_DLL extern SARRAY * sarrayIntersectionByAset ( SARRAY *sa1, SARRAY *sa2 );
LEPT_DLL extern L_ASET * l_asetCreateFromSarray ( SARRAY *sa );
LEPT_DLL extern l_ok sarrayRemoveDupsByHash ( SARRAY *sas, SARRAY **psad, L_DNAHASH **pdahash );
LEPT_DLL extern SARRAY * sarrayIntersectionByHash ( SARRAY *sa1, SARRAY *sa2 );
LEPT_DLL extern l_ok sarrayFindStringByHash ( SARRAY *sa, L_DNAHASH *dahash, const char *str, l_int32 *pindex );
LEPT_DLL extern L_DNAHASH * l_dnaHashCreateFromSarray ( SARRAY *sa );
LEPT_DLL extern SARRAY * sarrayGenerateIntegers ( l_int32 n );
LEPT_DLL extern l_ok sarrayLookupCSKV ( SARRAY *sa, const char *keystring, char **pvalstring );
LEPT_DLL extern PIX * pixScale ( PIX *pixs, l_float32 scalex, l_float32 scaley );
LEPT_DLL extern PIX * pixScaleToSizeRel ( PIX *pixs, l_int32 delw, l_int32 delh );
LEPT_DLL extern PIX * pixScaleToSize ( PIX *pixs, l_int32 wd, l_int32 hd );
LEPT_DLL extern PIX * pixScaleToResolution ( PIX *pixs, l_float32 target, l_float32 assumed, l_float32 *pscalefact );
LEPT_DLL extern PIX * pixScaleGeneral ( PIX *pixs, l_float32 scalex, l_float32 scaley, l_float32 sharpfract, l_int32 sharpwidth );
LEPT_DLL extern PIX * pixScaleLI ( PIX *pixs, l_float32 scalex, l_float32 scaley );
LEPT_DLL extern PIX * pixScaleColorLI ( PIX *pixs, l_float32 scalex, l_float32 scaley );
LEPT_DLL extern PIX * pixScaleColor2xLI ( PIX *pixs );
LEPT_DLL extern PIX * pixScaleColor4xLI ( PIX *pixs );
LEPT_DLL extern PIX * pixScaleGrayLI ( PIX *pixs, l_float32 scalex, l_float32 scaley );
LEPT_DLL extern PIX * pixScaleGray2xLI ( PIX *pixs );
LEPT_DLL extern PIX * pixScaleGray4xLI ( PIX *pixs );
LEPT_DLL extern PIX * pixScaleGray2xLIThresh ( PIX *pixs, l_int32 thresh );
LEPT_DLL extern PIX * pixScaleGray2xLIDither ( PIX *pixs );
LEPT_DLL extern PIX * pixScaleGray4xLIThresh ( PIX *pixs, l_int32 thresh );
LEPT_DLL extern PIX * pixScaleGray4xLIDither ( PIX *pixs );
LEPT_DLL extern PIX * pixScaleBySampling ( PIX *pixs, l_float32 scalex, l_float32 scaley );
LEPT_DLL extern PIX * pixScaleBySamplingToSize ( PIX *pixs, l_int32 wd, l_int32 hd );
LEPT_DLL extern PIX * pixScaleByIntSampling ( PIX *pixs, l_int32 factor );
LEPT_DLL extern PIX * pixScaleRGBToGrayFast ( PIX *pixs, l_int32 factor, l_int32 color );
LEPT_DLL extern PIX * pixScaleRGBToBinaryFast ( PIX *pixs, l_int32 factor, l_int32 thresh );
LEPT_DLL extern PIX * pixScaleGrayToBinaryFast ( PIX *pixs, l_int32 factor, l_int32 thresh );
LEPT_DLL extern PIX * pixScaleSmooth ( PIX *pix, l_float32 scalex, l_float32 scaley );
LEPT_DLL extern PIX * pixScaleSmoothToSize ( PIX *pixs, l_int32 wd, l_int32 hd );
LEPT_DLL extern PIX * pixScaleRGBToGray2 ( PIX *pixs, l_float32 rwt, l_float32 gwt, l_float32 bwt );
LEPT_DLL extern PIX * pixScaleAreaMap ( PIX *pix, l_float32 scalex, l_float32 scaley );
LEPT_DLL extern PIX * pixScaleAreaMap2 ( PIX *pix );
LEPT_DLL extern PIX * pixScaleAreaMapToSize ( PIX *pixs, l_int32 wd, l_int32 hd );
LEPT_DLL extern PIX * pixScaleBinary ( PIX *pixs, l_float32 scalex, l_float32 scaley );
LEPT_DLL extern PIX * pixScaleToGray ( PIX *pixs, l_float32 scalefactor );
LEPT_DLL extern PIX * pixScaleToGrayFast ( PIX *pixs, l_float32 scalefactor );
LEPT_DLL extern PIX * pixScaleToGray2 ( PIX *pixs );
LEPT_DLL extern PIX * pixScaleToGray3 ( PIX *pixs );
LEPT_DLL extern PIX * pixScaleToGray4 ( PIX *pixs );
LEPT_DLL extern PIX * pixScaleToGray6 ( PIX *pixs );
LEPT_DLL extern PIX * pixScaleToGray8 ( PIX *pixs );
LEPT_DLL extern PIX * pixScaleToGray16 ( PIX *pixs );
LEPT_DLL extern PIX * pixScaleToGrayMipmap ( PIX *pixs, l_float32 scalefactor );
LEPT_DLL extern PIX * pixScaleMipmap ( PIX *pixs1, PIX *pixs2, l_float32 scale );
LEPT_DLL extern PIX * pixExpandReplicate ( PIX *pixs, l_int32 factor );
LEPT_DLL extern PIX * pixScaleGrayMinMax ( PIX *pixs, l_int32 xfact, l_int32 yfact, l_int32 type );
LEPT_DLL extern PIX * pixScaleGrayMinMax2 ( PIX *pixs, l_int32 type );
LEPT_DLL extern PIX * pixScaleGrayRankCascade ( PIX *pixs, l_int32 level1, l_int32 level2, l_int32 level3, l_int32 level4 );
LEPT_DLL extern PIX * pixScaleGrayRank2 ( PIX *pixs, l_int32 rank );
LEPT_DLL extern l_ok pixScaleAndTransferAlpha ( PIX *pixd, PIX *pixs, l_float32 scalex, l_float32 scaley );
LEPT_DLL extern PIX * pixScaleWithAlpha ( PIX *pixs, l_float32 scalex, l_float32 scaley, PIX *pixg, l_float32 fract );
LEPT_DLL extern PIX * pixSeedfillBinary ( PIX *pixd, PIX *pixs, PIX *pixm, l_int32 connectivity );
LEPT_DLL extern PIX * pixSeedfillBinaryRestricted ( PIX *pixd, PIX *pixs, PIX *pixm, l_int32 connectivity, l_int32 xmax, l_int32 ymax );
LEPT_DLL extern PIX * pixHolesByFilling ( PIX *pixs, l_int32 connectivity );
LEPT_DLL extern PIX * pixFillClosedBorders ( PIX *pixs, l_int32 connectivity );
LEPT_DLL extern PIX * pixExtractBorderConnComps ( PIX *pixs, l_int32 connectivity );
LEPT_DLL extern PIX * pixRemoveBorderConnComps ( PIX *pixs, l_int32 connectivity );
LEPT_DLL extern PIX * pixFillBgFromBorder ( PIX *pixs, l_int32 connectivity );
LEPT_DLL extern PIX * pixFillHolesToBoundingRect ( PIX *pixs, l_int32 minsize, l_float32 maxhfract, l_float32 minfgfract );
LEPT_DLL extern l_ok pixSeedfillGray ( PIX *pixs, PIX *pixm, l_int32 connectivity );
LEPT_DLL extern l_ok pixSeedfillGrayInv ( PIX *pixs, PIX *pixm, l_int32 connectivity );
LEPT_DLL extern l_ok pixSeedfillGraySimple ( PIX *pixs, PIX *pixm, l_int32 connectivity );
LEPT_DLL extern l_ok pixSeedfillGrayInvSimple ( PIX *pixs, PIX *pixm, l_int32 connectivity );
LEPT_DLL extern PIX * pixSeedfillGrayBasin ( PIX *pixb, PIX *pixm, l_int32 delta, l_int32 connectivity );
LEPT_DLL extern PIX * pixDistanceFunction ( PIX *pixs, l_int32 connectivity, l_int32 outdepth, l_int32 boundcond );
LEPT_DLL extern PIX * pixSeedspread ( PIX *pixs, l_int32 connectivity );
LEPT_DLL extern l_ok pixLocalExtrema ( PIX *pixs, l_int32 maxmin, l_int32 minmax, PIX **ppixmin, PIX **ppixmax );
LEPT_DLL extern l_ok pixSelectedLocalExtrema ( PIX *pixs, l_int32 mindist, PIX **ppixmin, PIX **ppixmax );
LEPT_DLL extern PIX * pixFindEqualValues ( PIX *pixs1, PIX *pixs2 );
LEPT_DLL extern l_ok pixSelectMinInConnComp ( PIX *pixs, PIX *pixm, PTA **ppta, NUMA **pnav );
LEPT_DLL extern PIX * pixRemoveSeededComponents ( PIX *pixd, PIX *pixs, PIX *pixm, l_int32 connectivity, l_int32 bordersize );
LEPT_DLL extern SELA * selaCreate ( l_int32 n );
LEPT_DLL extern void selaDestroy ( SELA **psela );
LEPT_DLL extern SEL * selCreate ( l_int32 height, l_int32 width, const char *name );
LEPT_DLL extern void selDestroy ( SEL **psel );
LEPT_DLL extern SEL * selCopy ( SEL *sel );
LEPT_DLL extern SEL * selCreateBrick ( l_int32 h, l_int32 w, l_int32 cy, l_int32 cx, l_int32 type );
LEPT_DLL extern SEL * selCreateComb ( l_int32 factor1, l_int32 factor2, l_int32 direction );
LEPT_DLL extern l_int32 ** create2dIntArray ( l_int32 sy, l_int32 sx );
LEPT_DLL extern l_ok selaAddSel ( SELA *sela, SEL *sel, const char *selname, l_int32 copyflag );
LEPT_DLL extern l_int32 selaGetCount ( SELA *sela );
LEPT_DLL extern SEL * selaGetSel ( SELA *sela, l_int32 i );
LEPT_DLL extern char * selGetName ( SEL *sel );
LEPT_DLL extern l_ok selSetName ( SEL *sel, const char *name );
LEPT_DLL extern l_ok selaFindSelByName ( SELA *sela, const char *name, l_int32 *pindex, SEL **psel );
LEPT_DLL extern l_ok selGetElement ( SEL *sel, l_int32 row, l_int32 col, l_int32 *ptype );
LEPT_DLL extern l_ok selSetElement ( SEL *sel, l_int32 row, l_int32 col, l_int32 type );
LEPT_DLL extern l_ok selGetParameters ( SEL *sel, l_int32 *psy, l_int32 *psx, l_int32 *pcy, l_int32 *pcx );
LEPT_DLL extern l_ok selSetOrigin ( SEL *sel, l_int32 cy, l_int32 cx );
LEPT_DLL extern l_ok selGetTypeAtOrigin ( SEL *sel, l_int32 *ptype );
LEPT_DLL extern char * selaGetBrickName ( SELA *sela, l_int32 hsize, l_int32 vsize );
LEPT_DLL extern char * selaGetCombName ( SELA *sela, l_int32 size, l_int32 direction );
LEPT_DLL extern l_ok getCompositeParameters ( l_int32 size, l_int32 *psize1, l_int32 *psize2, char **pnameh1, char **pnameh2, char **pnamev1, char **pnamev2 );
LEPT_DLL extern SARRAY * selaGetSelnames ( SELA *sela );
LEPT_DLL extern l_ok selFindMaxTranslations ( SEL *sel, l_int32 *pxp, l_int32 *pyp, l_int32 *pxn, l_int32 *pyn );
LEPT_DLL extern SEL * selRotateOrth ( SEL *sel, l_int32 quads );
LEPT_DLL extern SELA * selaRead ( const char *fname );
LEPT_DLL extern SELA * selaReadStream ( FILE *fp );
LEPT_DLL extern SEL * selRead ( const char *fname );
LEPT_DLL extern SEL * selReadStream ( FILE *fp );
LEPT_DLL extern l_ok selaWrite ( const char *fname, SELA *sela );
LEPT_DLL extern l_ok selaWriteStream ( FILE *fp, SELA *sela );
LEPT_DLL extern l_ok selWrite ( const char *fname, SEL *sel );
LEPT_DLL extern l_ok selWriteStream ( FILE *fp, SEL *sel );
LEPT_DLL extern SEL * selCreateFromString ( const char *text, l_int32 h, l_int32 w, const char *name );
LEPT_DLL extern char * selPrintToString ( SEL *sel );
LEPT_DLL extern SELA * selaCreateFromFile ( const char *filename );
LEPT_DLL extern SEL * selCreateFromPta ( PTA *pta, l_int32 cy, l_int32 cx, const char *name );
LEPT_DLL extern SEL * selCreateFromPix ( PIX *pix, l_int32 cy, l_int32 cx, const char *name );
LEPT_DLL extern SEL * selReadFromColorImage ( const char *pathname );
LEPT_DLL extern SEL * selCreateFromColorPix ( PIX *pixs, const char *selname );
LEPT_DLL extern SELA * selaCreateFromColorPixa ( PIXA *pixa, SARRAY *sa );
LEPT_DLL extern PIX * selDisplayInPix ( SEL *sel, l_int32 size, l_int32 gthick );
LEPT_DLL extern PIX * selaDisplayInPix ( SELA *sela, l_int32 size, l_int32 gthick, l_int32 spacing, l_int32 ncols );
LEPT_DLL extern SELA * selaAddBasic ( SELA *sela );
LEPT_DLL extern SELA * selaAddHitMiss ( SELA *sela );
LEPT_DLL extern SELA * selaAddDwaLinear ( SELA *sela );
LEPT_DLL extern SELA * selaAddDwaCombs ( SELA *sela );
LEPT_DLL extern SELA * selaAddCrossJunctions ( SELA *sela, l_float32 hlsize, l_float32 mdist, l_int32 norient, l_int32 debugflag );
LEPT_DLL extern SELA * selaAddTJunctions ( SELA *sela, l_float32 hlsize, l_float32 mdist, l_int32 norient, l_int32 debugflag );
LEPT_DLL extern SELA * sela4ccThin ( SELA *sela );
LEPT_DLL extern SELA * sela8ccThin ( SELA *sela );
LEPT_DLL extern SELA * sela4and8ccThin ( SELA *sela );
LEPT_DLL extern SEL * selMakePlusSign ( l_int32 size, l_int32 linewidth );
LEPT_DLL extern SEL * pixGenerateSelWithRuns ( PIX *pixs, l_int32 nhlines, l_int32 nvlines, l_int32 distance, l_int32 minlength, l_int32 toppix, l_int32 botpix, l_int32 leftpix, l_int32 rightpix, PIX **ppixe );
LEPT_DLL extern SEL * pixGenerateSelRandom ( PIX *pixs, l_float32 hitfract, l_float32 missfract, l_int32 distance, l_int32 toppix, l_int32 botpix, l_int32 leftpix, l_int32 rightpix, PIX **ppixe );
LEPT_DLL extern SEL * pixGenerateSelBoundary ( PIX *pixs, l_int32 hitdist, l_int32 missdist, l_int32 hitskip, l_int32 missskip, l_int32 topflag, l_int32 botflag, l_int32 leftflag, l_int32 rightflag, PIX **ppixe );
LEPT_DLL extern NUMA * pixGetRunCentersOnLine ( PIX *pixs, l_int32 x, l_int32 y, l_int32 minlength );
LEPT_DLL extern NUMA * pixGetRunsOnLine ( PIX *pixs, l_int32 x1, l_int32 y1, l_int32 x2, l_int32 y2 );
LEPT_DLL extern PTA * pixSubsampleBoundaryPixels ( PIX *pixs, l_int32 skip );
LEPT_DLL extern l_int32 adjacentOnPixelInRaster ( PIX *pixs, l_int32 x, l_int32 y, l_int32 *pxa, l_int32 *pya );
LEPT_DLL extern PIX * pixDisplayHitMissSel ( PIX *pixs, SEL *sel, l_int32 scalefactor, l_uint32 hitcolor, l_uint32 misscolor );
LEPT_DLL extern PIX * pixHShear ( PIX *pixd, PIX *pixs, l_int32 yloc, l_float32 radang, l_int32 incolor );
LEPT_DLL extern PIX * pixVShear ( PIX *pixd, PIX *pixs, l_int32 xloc, l_float32 radang, l_int32 incolor );
LEPT_DLL extern PIX * pixHShearCorner ( PIX *pixd, PIX *pixs, l_float32 radang, l_int32 incolor );
LEPT_DLL extern PIX * pixVShearCorner ( PIX *pixd, PIX *pixs, l_float32 radang, l_int32 incolor );
LEPT_DLL extern PIX * pixHShearCenter ( PIX *pixd, PIX *pixs, l_float32 radang, l_int32 incolor );
LEPT_DLL extern PIX * pixVShearCenter ( PIX *pixd, PIX *pixs, l_float32 radang, l_int32 incolor );
LEPT_DLL extern l_ok pixHShearIP ( PIX *pixs, l_int32 yloc, l_float32 radang, l_int32 incolor );
LEPT_DLL extern l_ok pixVShearIP ( PIX *pixs, l_int32 xloc, l_float32 radang, l_int32 incolor );
LEPT_DLL extern PIX * pixHShearLI ( PIX *pixs, l_int32 yloc, l_float32 radang, l_int32 incolor );
LEPT_DLL extern PIX * pixVShearLI ( PIX *pixs, l_int32 xloc, l_float32 radang, l_int32 incolor );
LEPT_DLL extern PIX * pixDeskewBoth ( PIX *pixs, l_int32 redsearch );
LEPT_DLL extern PIX * pixDeskew ( PIX *pixs, l_int32 redsearch );
LEPT_DLL extern PIX * pixFindSkewAndDeskew ( PIX *pixs, l_int32 redsearch, l_float32 *pangle, l_float32 *pconf );
LEPT_DLL extern PIX * pixDeskewGeneral ( PIX *pixs, l_int32 redsweep, l_float32 sweeprange, l_float32 sweepdelta, l_int32 redsearch, l_int32 thresh, l_float32 *pangle, l_float32 *pconf );
LEPT_DLL extern l_ok pixFindSkew ( PIX *pixs, l_float32 *pangle, l_float32 *pconf );
LEPT_DLL extern l_ok pixFindSkewSweep ( PIX *pixs, l_float32 *pangle, l_int32 reduction, l_float32 sweeprange, l_float32 sweepdelta );
LEPT_DLL extern l_ok pixFindSkewSweepAndSearch ( PIX *pixs, l_float32 *pangle, l_float32 *pconf, l_int32 redsweep, l_int32 redsearch, l_float32 sweeprange, l_float32 sweepdelta, l_float32 minbsdelta );
LEPT_DLL extern l_ok pixFindSkewSweepAndSearchScore ( PIX *pixs, l_float32 *pangle, l_float32 *pconf, l_float32 *pendscore, l_int32 redsweep, l_int32 redsearch, l_float32 sweepcenter, l_float32 sweeprange, l_float32 sweepdelta, l_float32 minbsdelta );
LEPT_DLL extern l_ok pixFindSkewSweepAndSearchScorePivot ( PIX *pixs, l_float32 *pangle, l_float32 *pconf, l_float32 *pendscore, l_int32 redsweep, l_int32 redsearch, l_float32 sweepcenter, l_float32 sweeprange, l_float32 sweepdelta, l_float32 minbsdelta, l_int32 pivot );
LEPT_DLL extern l_int32 pixFindSkewOrthogonalRange ( PIX *pixs, l_float32 *pangle, l_float32 *pconf, l_int32 redsweep, l_int32 redsearch, l_float32 sweeprange, l_float32 sweepdelta, l_float32 minbsdelta, l_float32 confprior );
LEPT_DLL extern l_ok pixFindDifferentialSquareSum ( PIX *pixs, l_float32 *psum );
LEPT_DLL extern l_ok pixFindNormalizedSquareSum ( PIX *pixs, l_float32 *phratio, l_float32 *pvratio, l_float32 *pfract );
LEPT_DLL extern PIX * pixReadStreamSpix ( FILE *fp );
LEPT_DLL extern l_ok readHeaderSpix ( const char *filename, l_int32 *pwidth, l_int32 *pheight, l_int32 *pbps, l_int32 *pspp, l_int32 *piscmap );
LEPT_DLL extern l_ok freadHeaderSpix ( FILE *fp, l_int32 *pwidth, l_int32 *pheight, l_int32 *pbps, l_int32 *pspp, l_int32 *piscmap );
LEPT_DLL extern l_ok sreadHeaderSpix ( const l_uint32 *data, l_int32 *pwidth, l_int32 *pheight, l_int32 *pbps, l_int32 *pspp, l_int32 *piscmap );
LEPT_DLL extern l_ok pixWriteStreamSpix ( FILE *fp, PIX *pix );
LEPT_DLL extern PIX * pixReadMemSpix ( const l_uint8 *data, size_t size );
LEPT_DLL extern l_ok pixWriteMemSpix ( l_uint8 **pdata, size_t *psize, PIX *pix );
LEPT_DLL extern l_ok pixSerializeToMemory ( PIX *pixs, l_uint32 **pdata, size_t *pnbytes );
LEPT_DLL extern PIX * pixDeserializeFromMemory ( const l_uint32 *data, size_t nbytes );
LEPT_DLL extern L_STACK * lstackCreate ( l_int32 n );
LEPT_DLL extern void lstackDestroy ( L_STACK **plstack, l_int32 freeflag );
LEPT_DLL extern l_ok lstackAdd ( L_STACK *lstack, void *item );
LEPT_DLL extern void * lstackRemove ( L_STACK *lstack );
LEPT_DLL extern l_int32 lstackGetCount ( L_STACK *lstack );
LEPT_DLL extern l_ok lstackPrint ( FILE *fp, L_STACK *lstack );
LEPT_DLL extern L_STRCODE * strcodeCreate ( l_int32 fileno );
LEPT_DLL extern l_ok strcodeCreateFromFile ( const char *filein, l_int32 fileno, const char *outdir );
LEPT_DLL extern l_ok strcodeGenerate ( L_STRCODE *strcode, const char *filein, const char *type );
LEPT_DLL extern l_int32 strcodeFinalize ( L_STRCODE **pstrcode, const char *outdir );
LEPT_DLL extern l_int32 l_getStructStrFromFile ( const char *filename, l_int32 field, char **pstr );
LEPT_DLL extern l_ok pixFindStrokeLength ( PIX *pixs, l_int32 *tab8, l_int32 *plength );
LEPT_DLL extern l_ok pixFindStrokeWidth ( PIX *pixs, l_float32 thresh, l_int32 *tab8, l_float32 *pwidth, NUMA **pnahisto );
LEPT_DLL extern NUMA * pixaFindStrokeWidth ( PIXA *pixa, l_float32 thresh, l_int32 *tab8, l_int32 debug );
LEPT_DLL extern PIXA * pixaModifyStrokeWidth ( PIXA *pixas, l_float32 targetw );
LEPT_DLL extern PIX * pixModifyStrokeWidth ( PIX *pixs, l_float32 width, l_float32 targetw );
LEPT_DLL extern PIXA * pixaSetStrokeWidth ( PIXA *pixas, l_int32 width, l_int32 thinfirst, l_int32 connectivity );
LEPT_DLL extern PIX * pixSetStrokeWidth ( PIX *pixs, l_int32 width, l_int32 thinfirst, l_int32 connectivity );
LEPT_DLL extern l_int32 * sudokuReadFile ( const char *filename );
LEPT_DLL extern l_int32 * sudokuReadString ( const char *str );
LEPT_DLL extern L_SUDOKU * sudokuCreate ( l_int32 *array );
LEPT_DLL extern void sudokuDestroy ( L_SUDOKU **psud );
LEPT_DLL extern l_int32 sudokuSolve ( L_SUDOKU *sud );
LEPT_DLL extern l_ok sudokuTestUniqueness ( l_int32 *array, l_int32 *punique );
LEPT_DLL extern L_SUDOKU * sudokuGenerate ( l_int32 *array, l_int32 seed, l_int32 minelems, l_int32 maxtries );
LEPT_DLL extern l_int32 sudokuOutput ( L_SUDOKU *sud, l_int32 arraytype );
LEPT_DLL extern PIX * pixAddSingleTextblock ( PIX *pixs, L_BMF *bmf, const char *textstr, l_uint32 val, l_int32 location, l_int32 *poverflow );
LEPT_DLL extern PIX * pixAddTextlines ( PIX *pixs, L_BMF *bmf, const char *textstr, l_uint32 val, l_int32 location );
LEPT_DLL extern l_ok pixSetTextblock ( PIX *pixs, L_BMF *bmf, const char *textstr, l_uint32 val, l_int32 x0, l_int32 y0, l_int32 wtext, l_int32 firstindent, l_int32 *poverflow );
LEPT_DLL extern l_ok pixSetTextline ( PIX *pixs, L_BMF *bmf, const char *textstr, l_uint32 val, l_int32 x0, l_int32 y0, l_int32 *pwidth, l_int32 *poverflow );
LEPT_DLL extern PIXA * pixaAddTextNumber ( PIXA *pixas, L_BMF *bmf, NUMA *na, l_uint32 val, l_int32 location );
LEPT_DLL extern PIXA * pixaAddTextlines ( PIXA *pixas, L_BMF *bmf, SARRAY *sa, l_uint32 val, l_int32 location );
LEPT_DLL extern l_ok pixaAddPixWithText ( PIXA *pixa, PIX *pixs, l_int32 reduction, L_BMF *bmf, const char *textstr, l_uint32 val, l_int32 location );
LEPT_DLL extern SARRAY * bmfGetLineStrings ( L_BMF *bmf, const char *textstr, l_int32 maxw, l_int32 firstindent, l_int32 *ph );
LEPT_DLL extern NUMA * bmfGetWordWidths ( L_BMF *bmf, const char *textstr, SARRAY *sa );
LEPT_DLL extern l_ok bmfGetStringWidth ( L_BMF *bmf, const char *textstr, l_int32 *pw );
LEPT_DLL extern SARRAY * splitStringToParagraphs ( char *textstr, l_int32 splitflag );
LEPT_DLL extern PIX * pixReadTiff ( const char *filename, l_int32 n );
LEPT_DLL extern PIX * pixReadStreamTiff ( FILE *fp, l_int32 n );
LEPT_DLL extern l_ok pixWriteTiff ( const char *filename, PIX *pix, l_int32 comptype, const char *modestr );
LEPT_DLL extern l_ok pixWriteTiffCustom ( const char *filename, PIX *pix, l_int32 comptype, const char *modestr, NUMA *natags, SARRAY *savals, SARRAY *satypes, NUMA *nasizes );
LEPT_DLL extern l_ok pixWriteStreamTiff ( FILE *fp, PIX *pix, l_int32 comptype );
LEPT_DLL extern l_ok pixWriteStreamTiffWA ( FILE *fp, PIX *pix, l_int32 comptype, const char *modestr );
LEPT_DLL extern PIX * pixReadFromMultipageTiff ( const char *fname, size_t *poffset );
LEPT_DLL extern PIXA * pixaReadMultipageTiff ( const char *filename );
LEPT_DLL extern l_ok pixaWriteMultipageTiff ( const char *fname, PIXA *pixa );
LEPT_DLL extern l_ok writeMultipageTiff ( const char *dirin, const char *substr, const char *fileout );
LEPT_DLL extern l_ok writeMultipageTiffSA ( SARRAY *sa, const char *fileout );
LEPT_DLL extern l_ok fprintTiffInfo ( FILE *fpout, const char *tiffile );
LEPT_DLL extern l_ok tiffGetCount ( FILE *fp, l_int32 *pn );
LEPT_DLL extern l_ok getTiffResolution ( FILE *fp, l_int32 *pxres, l_int32 *pyres );
LEPT_DLL extern l_ok readHeaderTiff ( const char *filename, l_int32 n, l_int32 *pw, l_int32 *ph, l_int32 *pbps, l_int32 *pspp, l_int32 *pres, l_int32 *pcmap, l_int32 *pformat );
LEPT_DLL extern l_ok freadHeaderTiff ( FILE *fp, l_int32 n, l_int32 *pw, l_int32 *ph, l_int32 *pbps, l_int32 *pspp, l_int32 *pres, l_int32 *pcmap, l_int32 *pformat );
LEPT_DLL extern l_ok readHeaderMemTiff ( const l_uint8 *cdata, size_t size, l_int32 n, l_int32 *pw, l_int32 *ph, l_int32 *pbps, l_int32 *pspp, l_int32 *pres, l_int32 *pcmap, l_int32 *pformat );
LEPT_DLL extern l_ok findTiffCompression ( FILE *fp, l_int32 *pcomptype );
LEPT_DLL extern l_ok extractG4DataFromFile ( const char *filein, l_uint8 **pdata, size_t *pnbytes, l_int32 *pw, l_int32 *ph, l_int32 *pminisblack );
LEPT_DLL extern PIX * pixReadMemTiff ( const l_uint8 *cdata, size_t size, l_int32 n );
LEPT_DLL extern PIX * pixReadMemFromMultipageTiff ( const l_uint8 *cdata, size_t size, size_t *poffset );
LEPT_DLL extern PIXA * pixaReadMemMultipageTiff ( const l_uint8 *data, size_t size );
LEPT_DLL extern l_ok pixaWriteMemMultipageTiff ( l_uint8 **pdata, size_t *psize, PIXA *pixa );
LEPT_DLL extern l_ok pixWriteMemTiff ( l_uint8 **pdata, size_t *psize, PIX *pix, l_int32 comptype );
LEPT_DLL extern l_ok pixWriteMemTiffCustom ( l_uint8 **pdata, size_t *psize, PIX *pix, l_int32 comptype, NUMA *natags, SARRAY *savals, SARRAY *satypes, NUMA *nasizes );
LEPT_DLL extern l_int32 setMsgSeverity ( l_int32 newsev );
LEPT_DLL extern l_int32 returnErrorInt ( const char *msg, const char *procname, l_int32 ival );
LEPT_DLL extern l_float32 returnErrorFloat ( const char *msg, const char *procname, l_float32 fval );
LEPT_DLL extern void * returnErrorPtr ( const char *msg, const char *procname, void *pval );
LEPT_DLL extern void leptSetStderrHandler ( void  ( *handler ) ( const char * ) );
LEPT_DLL extern void lept_stderr ( const char *fmt, ... );
LEPT_DLL extern l_ok filesAreIdentical ( const char *fname1, const char *fname2, l_int32 *psame );
LEPT_DLL extern l_uint16 convertOnLittleEnd16 ( l_uint16 shortin );
LEPT_DLL extern l_uint16 convertOnBigEnd16 ( l_uint16 shortin );
LEPT_DLL extern l_uint32 convertOnLittleEnd32 ( l_uint32 wordin );
LEPT_DLL extern l_uint32 convertOnBigEnd32 ( l_uint32 wordin );
LEPT_DLL extern l_ok fileCorruptByDeletion ( const char *filein, l_float32 loc, l_float32 size, const char *fileout );
LEPT_DLL extern l_ok fileCorruptByMutation ( const char *filein, l_float32 loc, l_float32 size, const char *fileout );
LEPT_DLL extern l_ok fileReplaceBytes ( const char *filein, l_int32 start, l_int32 nbytes, l_uint8 *newdata, size_t newsize, const char *fileout );
LEPT_DLL extern l_ok genRandomIntegerInRange ( l_int32 range, l_int32 seed, l_int32 *pval );
LEPT_DLL extern l_int32 lept_roundftoi ( l_float32 fval );
LEPT_DLL extern l_ok l_hashStringToUint64 ( const char *str, l_uint64 *phash );
LEPT_DLL extern l_ok l_hashPtToUint64 ( l_int32 x, l_int32 y, l_uint64 *phash );
LEPT_DLL extern l_ok l_hashFloat64ToUint64 ( l_int32 nbuckets, l_float64 val, l_uint64 *phash );
LEPT_DLL extern l_ok findNextLargerPrime ( l_int32 start, l_uint32 *pprime );
LEPT_DLL extern l_ok lept_isPrime ( l_uint64 n, l_int32 *pis_prime, l_uint32 *pfactor );
LEPT_DLL extern l_uint32 convertIntToGrayCode ( l_uint32 val );
LEPT_DLL extern l_uint32 convertGrayCodeToInt ( l_uint32 val );
LEPT_DLL extern char * getLeptonicaVersion ( void );
LEPT_DLL extern void startTimer ( void );
LEPT_DLL extern l_float32 stopTimer ( void );
LEPT_DLL extern L_TIMER startTimerNested ( void );
LEPT_DLL extern l_float32 stopTimerNested ( L_TIMER rusage_start );
LEPT_DLL extern void l_getCurrentTime ( l_int32 *sec, l_int32 *usec );
LEPT_DLL extern L_WALLTIMER * startWallTimer ( void );
LEPT_DLL extern l_float32 stopWallTimer ( L_WALLTIMER **ptimer );
LEPT_DLL extern char * l_getFormattedDate ( void );
LEPT_DLL extern char * stringNew ( const char *src );
LEPT_DLL extern l_ok stringCopy ( char *dest, const char *src, l_int32 n );
LEPT_DLL extern char * stringCopySegment ( const char *src, l_int32 start, l_int32 nbytes );
LEPT_DLL extern l_ok stringReplace ( char **pdest, const char *src );
LEPT_DLL extern l_int32 stringLength ( const char *src, size_t size );
LEPT_DLL extern l_int32 stringCat ( char *dest, size_t size, const char *src );
LEPT_DLL extern char * stringConcatNew ( const char *first, ... );
LEPT_DLL extern char * stringJoin ( const char *src1, const char *src2 );
LEPT_DLL extern l_ok stringJoinIP ( char **psrc1, const char *src2 );
LEPT_DLL extern char * stringReverse ( const char *src );
LEPT_DLL extern char * strtokSafe ( char *cstr, const char *seps, char **psaveptr );
LEPT_DLL extern l_ok stringSplitOnToken ( char *cstr, const char *seps, char **phead, char **ptail );
LEPT_DLL extern l_ok stringCheckForChars ( const char *src, const char *chars, l_int32 *pfound );
LEPT_DLL extern char * stringRemoveChars ( const char *src, const char *remchars );
LEPT_DLL extern char * stringReplaceEachSubstr ( const char *src, const char *sub1, const char *sub2, l_int32 *pcount );
LEPT_DLL extern char * stringReplaceSubstr ( const char *src, const char *sub1, const char *sub2, l_int32 *ploc, l_int32 *pfound );
LEPT_DLL extern L_DNA * stringFindEachSubstr ( const char *src, const char *sub );
LEPT_DLL extern l_int32 stringFindSubstr ( const char *src, const char *sub, l_int32 *ploc );
LEPT_DLL extern l_uint8 * arrayReplaceEachSequence ( const l_uint8 *datas, size_t dataslen, const l_uint8 *seq, size_t seqlen, const l_uint8 *newseq, size_t newseqlen, size_t *pdatadlen, l_int32 *pcount );
LEPT_DLL extern L_DNA * arrayFindEachSequence ( const l_uint8 *data, size_t datalen, const l_uint8 *sequence, size_t seqlen );
LEPT_DLL extern l_ok arrayFindSequence ( const l_uint8 *data, size_t datalen, const l_uint8 *sequence, size_t seqlen, l_int32 *poffset, l_int32 *pfound );
LEPT_DLL extern void * reallocNew ( void **pindata, l_int32 oldsize, l_int32 newsize );
LEPT_DLL extern l_uint8 * l_binaryRead ( const char *filename, size_t *pnbytes );
LEPT_DLL extern l_uint8 * l_binaryReadStream ( FILE *fp, size_t *pnbytes );
LEPT_DLL extern l_uint8 * l_binaryReadSelect ( const char *filename, size_t start, size_t nbytes, size_t *pnread );
LEPT_DLL extern l_uint8 * l_binaryReadSelectStream ( FILE *fp, size_t start, size_t nbytes, size_t *pnread );
LEPT_DLL extern l_ok l_binaryWrite ( const char *filename, const char *operation, const void *data, size_t nbytes );
LEPT_DLL extern size_t nbytesInFile ( const char *filename );
LEPT_DLL extern size_t fnbytesInFile ( FILE *fp );
LEPT_DLL extern l_uint8 * l_binaryCopy ( const l_uint8 *datas, size_t size );
LEPT_DLL extern l_ok l_binaryCompare ( const l_uint8 *data1, size_t size1, const l_uint8 *data2, size_t size2, l_int32 *psame );
LEPT_DLL extern l_ok fileCopy ( const char *srcfile, const char *newfile );
LEPT_DLL extern l_ok fileConcatenate ( const char *srcfile, const char *destfile );
LEPT_DLL extern l_ok fileAppendString ( const char *filename, const char *str );
LEPT_DLL extern FILE * fopenReadStream ( const char *filename );
LEPT_DLL extern FILE * fopenWriteStream ( const char *filename, const char *modestring );
LEPT_DLL extern FILE * fopenReadFromMemory ( const l_uint8 *data, size_t size );
LEPT_DLL extern FILE * fopenWriteWinTempfile ( void );
LEPT_DLL extern FILE * lept_fopen ( const char *filename, const char *mode );
LEPT_DLL extern l_ok lept_fclose ( FILE *fp );
LEPT_DLL extern void * lept_calloc ( size_t nmemb, size_t size );
LEPT_DLL extern void lept_free ( void *ptr );
LEPT_DLL extern l_int32 lept_mkdir ( const char *subdir );
LEPT_DLL extern l_int32 lept_rmdir ( const char *subdir );
LEPT_DLL extern void lept_direxists ( const char *dir, l_int32 *pexists );
LEPT_DLL extern l_int32 lept_rm_match ( const char *subdir, const char *substr );
LEPT_DLL extern l_int32 lept_rm ( const char *subdir, const char *tail );
LEPT_DLL extern l_int32 lept_rmfile ( const char *filepath );
LEPT_DLL extern l_int32 lept_mv ( const char *srcfile, const char *newdir, const char *newtail, char **pnewpath );
LEPT_DLL extern l_int32 lept_cp ( const char *srcfile, const char *newdir, const char *newtail, char **pnewpath );
LEPT_DLL extern void callSystemDebug ( const char *cmd );
LEPT_DLL extern l_ok splitPathAtDirectory ( const char *pathname, char **pdir, char **ptail );
LEPT_DLL extern l_ok splitPathAtExtension ( const char *pathname, char **pbasename, char **pextension );
LEPT_DLL extern char * pathJoin ( const char *dir, const char *fname );
LEPT_DLL extern char * appendSubdirs ( const char *basedir, const char *subdirs );
LEPT_DLL extern l_ok convertSepCharsInPath ( char *path, l_int32 type );
LEPT_DLL extern char * genPathname ( const char *dir, const char *fname );
LEPT_DLL extern l_ok makeTempDirname ( char *result, size_t nbytes, const char *subdir );
LEPT_DLL extern l_ok modifyTrailingSlash ( char *path, size_t nbytes, l_int32 flag );
LEPT_DLL extern char * l_makeTempFilename ( void );
LEPT_DLL extern l_int32 extractNumberFromFilename ( const char *fname, l_int32 numpre, l_int32 numpost );
LEPT_DLL extern PIX * pixSimpleCaptcha ( PIX *pixs, l_int32 border, l_int32 nterms, l_uint32 seed, l_uint32 color, l_int32 cmapflag );
LEPT_DLL extern PIX * pixRandomHarmonicWarp ( PIX *pixs, l_float32 xmag, l_float32 ymag, l_float32 xfreq, l_float32 yfreq, l_int32 nx, l_int32 ny, l_uint32 seed, l_int32 grayval );
LEPT_DLL extern PIX * pixWarpStereoscopic ( PIX *pixs, l_int32 zbend, l_int32 zshiftt, l_int32 zshiftb, l_int32 ybendt, l_int32 ybendb, l_int32 redleft );
LEPT_DLL extern PIX * pixStretchHorizontal ( PIX *pixs, l_int32 dir, l_int32 type, l_int32 hmax, l_int32 operation, l_int32 incolor );
LEPT_DLL extern PIX * pixStretchHorizontalSampled ( PIX *pixs, l_int32 dir, l_int32 type, l_int32 hmax, l_int32 incolor );
LEPT_DLL extern PIX * pixStretchHorizontalLI ( PIX *pixs, l_int32 dir, l_int32 type, l_int32 hmax, l_int32 incolor );
LEPT_DLL extern PIX * pixQuadraticVShear ( PIX *pixs, l_int32 dir, l_int32 vmaxt, l_int32 vmaxb, l_int32 operation, l_int32 incolor );
LEPT_DLL extern PIX * pixQuadraticVShearSampled ( PIX *pixs, l_int32 dir, l_int32 vmaxt, l_int32 vmaxb, l_int32 incolor );
LEPT_DLL extern PIX * pixQuadraticVShearLI ( PIX *pixs, l_int32 dir, l_int32 vmaxt, l_int32 vmaxb, l_int32 incolor );
LEPT_DLL extern PIX * pixStereoFromPair ( PIX *pix1, PIX *pix2, l_float32 rwt, l_float32 gwt, l_float32 bwt );
LEPT_DLL extern L_WSHED * wshedCreate ( PIX *pixs, PIX *pixm, l_int32 mindepth, l_int32 debugflag );
LEPT_DLL extern void wshedDestroy ( L_WSHED **pwshed );
LEPT_DLL extern l_ok wshedApply ( L_WSHED *wshed );
LEPT_DLL extern l_ok wshedBasins ( L_WSHED *wshed, PIXA **ppixa, NUMA **pnalevels );
LEPT_DLL extern PIX * wshedRenderFill ( L_WSHED *wshed );
LEPT_DLL extern PIX * wshedRenderColors ( L_WSHED *wshed );
LEPT_DLL extern l_ok pixaWriteWebPAnim ( const char *filename, PIXA *pixa, l_int32 loopcount, l_int32 duration, l_int32 quality, l_int32 lossless );
LEPT_DLL extern l_ok pixaWriteStreamWebPAnim ( FILE *fp, PIXA *pixa, l_int32 loopcount, l_int32 duration, l_int32 quality, l_int32 lossless );
LEPT_DLL extern l_ok pixaWriteMemWebPAnim ( l_uint8 **pencdata, size_t *pencsize, PIXA *pixa, l_int32 loopcount, l_int32 duration, l_int32 quality, l_int32 lossless );
LEPT_DLL extern PIX * pixReadStreamWebP ( FILE *fp );
LEPT_DLL extern PIX * pixReadMemWebP ( const l_uint8 *filedata, size_t filesize );
LEPT_DLL extern l_ok readHeaderWebP ( const char *filename, l_int32 *pw, l_int32 *ph, l_int32 *pspp );
LEPT_DLL extern l_ok readHeaderMemWebP ( const l_uint8 *data, size_t size, l_int32 *pw, l_int32 *ph, l_int32 *pspp );
LEPT_DLL extern l_ok pixWriteWebP ( const char *filename, PIX *pixs, l_int32 quality, l_int32 lossless );
LEPT_DLL extern l_ok pixWriteStreamWebP ( FILE *fp, PIX *pixs, l_int32 quality, l_int32 lossless );
LEPT_DLL extern l_ok pixWriteMemWebP ( l_uint8 **pencdata, size_t *pencsize, PIX *pixs, l_int32 quality, l_int32 lossless );
LEPT_DLL extern l_int32 l_jpegSetQuality ( l_int32 new_quality );
LEPT_DLL extern void setLeptDebugOK ( l_int32 allow );
LEPT_DLL extern l_ok pixaWriteFiles ( const char *rootname, PIXA *pixa, l_int32 format );
LEPT_DLL extern l_ok pixWriteDebug ( const char *fname, PIX *pix, l_int32 format );
LEPT_DLL extern l_ok pixWrite ( const char *fname, PIX *pix, l_int32 format );
LEPT_DLL extern l_ok pixWriteAutoFormat ( const char *filename, PIX *pix );
LEPT_DLL extern l_ok pixWriteStream ( FILE *fp, PIX *pix, l_int32 format );
LEPT_DLL extern l_ok pixWriteImpliedFormat ( const char *filename, PIX *pix, l_int32 quality, l_int32 progressive );
LEPT_DLL extern l_int32 pixChooseOutputFormat ( PIX *pix );
LEPT_DLL extern l_int32 getImpliedFileFormat ( const char *filename );
LEPT_DLL extern l_ok pixGetAutoFormat ( PIX *pix, l_int32 *pformat );
LEPT_DLL extern const char * getFormatExtension ( l_int32 format );
LEPT_DLL extern l_ok pixWriteMem ( l_uint8 **pdata, size_t *psize, PIX *pix, l_int32 format );
LEPT_DLL extern l_ok l_fileDisplay ( const char *fname, l_int32 x, l_int32 y, l_float32 scale );
LEPT_DLL extern l_ok pixDisplay ( PIX *pixs, l_int32 x, l_int32 y );
LEPT_DLL extern l_ok pixDisplayWithTitle ( PIX *pixs, l_int32 x, l_int32 y, const char *title, l_int32 dispflag );
LEPT_DLL extern PIX * pixMakeColorSquare ( l_uint32 color, l_int32 size, l_int32 addlabel, l_int32 location, l_uint32 textcolor );
LEPT_DLL extern void l_chooseDisplayProg ( l_int32 selection );
LEPT_DLL extern void changeFormatForMissingLib ( l_int32 *pformat );
LEPT_DLL extern l_ok pixDisplayWrite ( PIX *pixs, l_int32 reduction );
LEPT_DLL extern l_ok pixSaveTiled ( PIX *pixs, PIXA *pixa, l_float32 scalefactor, l_int32 newrow, l_int32 space, l_int32 dp );
LEPT_DLL extern l_ok pixSaveTiledOutline ( PIX *pixs, PIXA *pixa, l_float32 scalefactor, l_int32 newrow, l_int32 space, l_int32 linewidth, l_int32 dp );
LEPT_DLL extern l_ok pixSaveTiledWithText ( PIX *pixs, PIXA *pixa, l_int32 outwidth, l_int32 newrow, l_int32 space, l_int32 linewidth, L_BMF *bmf, const char *textstr, l_uint32 val, l_int32 location );
LEPT_DLL extern l_uint8 * zlibCompress ( l_uint8 *datain, size_t nin, size_t *pnout );
LEPT_DLL extern l_uint8 * zlibUncompress ( l_uint8 *datain, size_t nin, size_t *pnout );

#ifdef __cplusplus
}
#endif  /* __cplusplus */
#endif /* NO_PROTOS */


#endif /* LEPTONICA_ALLHEADERS_H */

