/* multifit/gsl_multifit.h
 * 
 * Copyright (C) 2000, 2007, 2010 Brian Gough
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __GSL_MULTIFIT_H__
#define __GSL_MULTIFIT_H__

#include <stdlib.h>
/* gsl_math.h
 * 
 * Copyright (C) 1996, 1997, 1998, 1999, 2000, 2004, 2007 Gerard Jungman, Brian Gough
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __GSL_MATH_H__
#define __GSL_MATH_H__
#include <math.h>
/* sys/gsl_sys.h
 * 
 * Copyright (C) 1996, 1997, 1998, 1999, 2000, 2007 Gerard Jungman, Brian Gough
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __GSL_SYS_H__
#define __GSL_SYS_H__

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif

__BEGIN_DECLS

double gsl_log1p (const double x);
double gsl_expm1 (const double x);
double gsl_hypot (const double x, const double y);
double gsl_hypot3 (const double x, const double y, const double z);
double gsl_acosh (const double x);
double gsl_asinh (const double x);
double gsl_atanh (const double x);

int gsl_isnan (const double x);
int gsl_isinf (const double x);
int gsl_finite (const double x);

double gsl_nan (void);
double gsl_posinf (void);
double gsl_neginf (void);
double gsl_fdiv (const double x, const double y);

double gsl_coerce_double (const double x);
float gsl_coerce_float (const float x);
long double gsl_coerce_long_double (const long double x);

double gsl_ldexp(const double x, const int e);
double gsl_frexp(const double x, int * e);

int gsl_fcmp (const double x1, const double x2, const double epsilon);

__END_DECLS

#endif /* __GSL_SYS_H__ */
/* gsl_inline.h
 * 
 * Copyright (C) 2008, 2009 Brian Gough
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __GSL_INLINE_H__
#define __GSL_INLINE_H__

/* In recent versiions of GCC, the inline keyword has two different
   forms: GNU and C99.

   In GNU mode we can use 'extern inline' to make inline functions
   work like macros.  The function is only inlined--it is never output
   as a definition in an object file.

   In the new C99 mode 'extern inline' has a different meaning--it
   causes the definition of the function to be output in each object
   file where it is used.  This will result in multiple-definition
   errors on linking.  The 'inline' keyword on its own (without
   extern) has the same behavior as the original GNU 'extern inline'.

   The C99 style is the default with -std=c99 in GCC 4.3.  

   This header file allows either form of inline to be used by
   redefining the macros INLINE_DECL and INLINE_FUN.  These are used
   in the public header files as

        INLINE_DECL double gsl_foo (double x);
	#ifdef HAVE_INLINE
	INLINE_FUN double gsl_foo (double x) { return x+1.0; } ;
        #endif
   
*/

#ifdef HAVE_INLINE
#  if defined(__GNUC_STDC_INLINE__) || defined(GSL_C99_INLINE) || defined(HAVE_C99_INLINE)
#    define INLINE_DECL inline  /* use C99 inline */
#    define INLINE_FUN inline
#  else
#    define INLINE_DECL         /* use GNU extern inline */
#    define INLINE_FUN extern inline
#  endif
#else
#  define INLINE_DECL /* */
#endif

/* Range checking conditions in headers do not require any run-time
   tests of the global variable gsl_check_range.  They are enabled or
   disabled in user code at compile time with GSL_RANGE_CHECK macro.
   See also build.h. */
#define GSL_RANGE_COND(x) (x)

#endif /* __GSL_INLINE_H__ */
/* Author:  B. Gough and G. Jungman */
#ifndef __GSL_MACHINE_H__
#define __GSL_MACHINE_H__

#include <limits.h>
#include <float.h>

/* magic constants; mostly for the benefit of the implementation */

/* -*-MACHINE CONSTANTS-*-
 *
 * PLATFORM: Whiz-O-Matic 9000
 * FP_PLATFORM: IEEE-Virtual
 * HOSTNAME: nnn.lanl.gov
 * DATE: Fri Nov 20 17:53:26 MST 1998
 */
#define GSL_DBL_EPSILON        2.2204460492503131e-16
#define GSL_SQRT_DBL_EPSILON   1.4901161193847656e-08
#define GSL_ROOT3_DBL_EPSILON  6.0554544523933429e-06
#define GSL_ROOT4_DBL_EPSILON  1.2207031250000000e-04
#define GSL_ROOT5_DBL_EPSILON  7.4009597974140505e-04
#define GSL_ROOT6_DBL_EPSILON  2.4607833005759251e-03
#define GSL_LOG_DBL_EPSILON   (-3.6043653389117154e+01)

#define GSL_DBL_MIN        2.2250738585072014e-308
#define GSL_SQRT_DBL_MIN   1.4916681462400413e-154
#define GSL_ROOT3_DBL_MIN  2.8126442852362996e-103
#define GSL_ROOT4_DBL_MIN  1.2213386697554620e-77
#define GSL_ROOT5_DBL_MIN  2.9476022969691763e-62
#define GSL_ROOT6_DBL_MIN  5.3034368905798218e-52
#define GSL_LOG_DBL_MIN   (-7.0839641853226408e+02)

#define GSL_DBL_MAX        1.7976931348623157e+308
#define GSL_SQRT_DBL_MAX   1.3407807929942596e+154
#define GSL_ROOT3_DBL_MAX  5.6438030941222897e+102
#define GSL_ROOT4_DBL_MAX  1.1579208923731620e+77
#define GSL_ROOT5_DBL_MAX  4.4765466227572707e+61
#define GSL_ROOT6_DBL_MAX  2.3756689782295612e+51
#define GSL_LOG_DBL_MAX    7.0978271289338397e+02

#define GSL_FLT_EPSILON        1.1920928955078125e-07
#define GSL_SQRT_FLT_EPSILON   3.4526698300124393e-04
#define GSL_ROOT3_FLT_EPSILON  4.9215666011518501e-03
#define GSL_ROOT4_FLT_EPSILON  1.8581361171917516e-02
#define GSL_ROOT5_FLT_EPSILON  4.1234622211652937e-02
#define GSL_ROOT6_FLT_EPSILON  7.0153878019335827e-02
#define GSL_LOG_FLT_EPSILON   (-1.5942385152878742e+01)

#define GSL_FLT_MIN        1.1754943508222875e-38
#define GSL_SQRT_FLT_MIN   1.0842021724855044e-19
#define GSL_ROOT3_FLT_MIN  2.2737367544323241e-13
#define GSL_ROOT4_FLT_MIN  3.2927225399135965e-10
#define GSL_ROOT5_FLT_MIN  2.5944428542140822e-08
#define GSL_ROOT6_FLT_MIN  4.7683715820312542e-07
#define GSL_LOG_FLT_MIN   (-8.7336544750553102e+01)

#define GSL_FLT_MAX        3.4028234663852886e+38
#define GSL_SQRT_FLT_MAX   1.8446743523953730e+19
#define GSL_ROOT3_FLT_MAX  6.9814635196223242e+12
#define GSL_ROOT4_FLT_MAX  4.2949672319999986e+09
#define GSL_ROOT5_FLT_MAX  5.0859007855960041e+07
#define GSL_ROOT6_FLT_MAX  2.6422459233807749e+06
#define GSL_LOG_FLT_MAX    8.8722839052068352e+01

#define GSL_SFLT_EPSILON        4.8828125000000000e-04
#define GSL_SQRT_SFLT_EPSILON   2.2097086912079612e-02
#define GSL_ROOT3_SFLT_EPSILON  7.8745065618429588e-02
#define GSL_ROOT4_SFLT_EPSILON  1.4865088937534013e-01
#define GSL_ROOT5_SFLT_EPSILON  2.1763764082403100e-01
#define GSL_ROOT6_SFLT_EPSILON  2.8061551207734325e-01
#define GSL_LOG_SFLT_EPSILON   (-7.6246189861593985e+00)

/* !MACHINE CONSTANTS! */


/* a little internal backwards compatibility */
#define GSL_MACH_EPS  GSL_DBL_EPSILON



/* Here are the constants related to or derived from
 * machine constants. These are not to be confused with
 * the constants that define various precision levels
 * for the precision/error system.
 *
 * This information is determined at configure time
 * and is platform dependent. Edit at your own risk.
 *
 * PLATFORM: WHIZ-O-MATIC
 * CONFIG-DATE: Thu Nov 19 19:27:18 MST 1998
 * CONFIG-HOST: nnn.lanl.gov
 */

/* machine precision constants */
/* #define GSL_MACH_EPS         1.0e-15 */
#define GSL_SQRT_MACH_EPS       3.2e-08
#define GSL_ROOT3_MACH_EPS      1.0e-05
#define GSL_ROOT4_MACH_EPS      0.000178
#define GSL_ROOT5_MACH_EPS      0.00100
#define GSL_ROOT6_MACH_EPS      0.00316
#define GSL_LOG_MACH_EPS       (-34.54)


#endif /* __GSL_MACHINE_H__ */
/* gsl_precision.h
 * 
 * Copyright (C) 1996, 1997, 1998, 1999, 2000 Gerard Jungman
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

/* Author:  B. Gough and G. Jungman */

#ifndef __GSL_PRECISION_H__
#define __GSL_PRECISION_H__
/* gsl_types.h
 * 
 * Copyright (C) 2001, 2007 Brian Gough
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __GSL_TYPES_H__
#define __GSL_TYPES_H__

#ifndef GSL_VAR

#ifdef WIN32
#  ifdef GSL_DLL
#    ifdef DLL_EXPORT
#      define GSL_VAR extern __declspec(dllexport)
#    else
#      define GSL_VAR extern __declspec(dllimport)
#    endif
#  else
#    define GSL_VAR extern
#  endif
#else
#  define GSL_VAR extern
#endif

#endif

#endif /* __GSL_TYPES_H__ */

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif

__BEGIN_DECLS


/* A type for the precision indicator.
 * This is mainly for pedagogy.
 */
typedef  unsigned int  gsl_prec_t;


/* The number of precision types.
 * Remember that precision-mode
 * can index an array.
 */
#define _GSL_PREC_T_NUM 3


/* Arrays containing derived
 * precision constants for the
 * different precision levels.
 */
GSL_VAR const double gsl_prec_eps[];
GSL_VAR const double gsl_prec_sqrt_eps[];
GSL_VAR const double gsl_prec_root3_eps[];
GSL_VAR const double gsl_prec_root4_eps[];
GSL_VAR const double gsl_prec_root5_eps[];
GSL_VAR const double gsl_prec_root6_eps[];


__END_DECLS

#endif /* __GSL_PRECISION_H__ */
/* gsl_nan.h
 * 
 * Copyright (C) 1996, 1997, 1998, 1999, 2000, 2007 Gerard Jungman, Brian Gough
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __GSL_NAN_H__
#define __GSL_NAN_H__

#ifdef INFINITY
# define GSL_POSINF INFINITY
# define GSL_NEGINF (-INFINITY)
#elif defined(HUGE_VAL)
# define GSL_POSINF HUGE_VAL
# define GSL_NEGINF (-HUGE_VAL)
#else
# define GSL_POSINF (gsl_posinf())
# define GSL_NEGINF (gsl_neginf())
#endif

#ifdef NAN
# define GSL_NAN NAN
#elif defined(INFINITY)
# define GSL_NAN (INFINITY/INFINITY)
#else
# define GSL_NAN (gsl_nan())
#endif

#define GSL_POSZERO (+0.0)
#define GSL_NEGZERO (-0.0)

#endif /* __GSL_NAN_H__ */
/* gsl_pow_int.h
 * 
 * Copyright (C) 1996, 1997, 1998, 1999, 2000, 2004, 2007 Gerard Jungman, Brian Gough
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __GSL_POW_INT_H__
#define __GSL_POW_INT_H__

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif

__BEGIN_DECLS

INLINE_DECL double gsl_pow_2(const double x);
INLINE_DECL double gsl_pow_3(const double x);
INLINE_DECL double gsl_pow_4(const double x);
INLINE_DECL double gsl_pow_5(const double x);
INLINE_DECL double gsl_pow_6(const double x);
INLINE_DECL double gsl_pow_7(const double x);
INLINE_DECL double gsl_pow_8(const double x);
INLINE_DECL double gsl_pow_9(const double x);

#ifdef HAVE_INLINE
INLINE_FUN double gsl_pow_2(const double x) { return x*x;   }
INLINE_FUN double gsl_pow_3(const double x) { return x*x*x; }
INLINE_FUN double gsl_pow_4(const double x) { double x2 = x*x;   return x2*x2;    }
INLINE_FUN double gsl_pow_5(const double x) { double x2 = x*x;   return x2*x2*x;  }
INLINE_FUN double gsl_pow_6(const double x) { double x2 = x*x;   return x2*x2*x2; }
INLINE_FUN double gsl_pow_7(const double x) { double x3 = x*x*x; return x3*x3*x;  }
INLINE_FUN double gsl_pow_8(const double x) { double x2 = x*x;   double x4 = x2*x2; return x4*x4; }
INLINE_FUN double gsl_pow_9(const double x) { double x3 = x*x*x; return x3*x3*x3; }
#endif

double gsl_pow_int(double x, int n);
double gsl_pow_uint(double x, unsigned int n);

__END_DECLS

#endif /* __GSL_POW_INT_H__ */
/* gsl_minmax.h
 * 
 * Copyright (C) 2008 Gerard Jungman, Brian Gough
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __GSL_MINMAX_H__
#define __GSL_MINMAX_H__

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif

__BEGIN_DECLS

/* Define MAX and MIN macros/functions if they don't exist. */

/* plain old macros for general use */
#define GSL_MAX(a,b) ((a) > (b) ? (a) : (b))
#define GSL_MIN(a,b) ((a) < (b) ? (a) : (b))

/* function versions of the above, in case they are needed */
double gsl_max (double a, double b);
double gsl_min (double a, double b);

/* inline-friendly strongly typed versions */
#ifdef HAVE_INLINE

INLINE_FUN int GSL_MAX_INT (int a, int b);
INLINE_FUN int GSL_MIN_INT (int a, int b);
INLINE_FUN double GSL_MAX_DBL (double a, double b);
INLINE_FUN double GSL_MIN_DBL (double a, double b);
INLINE_FUN long double GSL_MAX_LDBL (long double a, long double b);
INLINE_FUN long double GSL_MIN_LDBL (long double a, long double b);

INLINE_FUN int
GSL_MAX_INT (int a, int b)
{
  return GSL_MAX (a, b);
}

INLINE_FUN int
GSL_MIN_INT (int a, int b)
{
  return GSL_MIN (a, b);
}

INLINE_FUN double
GSL_MAX_DBL (double a, double b)
{
  return GSL_MAX (a, b);
}

INLINE_FUN double
GSL_MIN_DBL (double a, double b)
{
  return GSL_MIN (a, b);
}

INLINE_FUN long double
GSL_MAX_LDBL (long double a, long double b)
{
  return GSL_MAX (a, b);
}

INLINE_FUN long double
GSL_MIN_LDBL (long double a, long double b)
{
  return GSL_MIN (a, b);
}
#else
#define GSL_MAX_INT(a,b)   GSL_MAX(a,b)
#define GSL_MIN_INT(a,b)   GSL_MIN(a,b)
#define GSL_MAX_DBL(a,b)   GSL_MAX(a,b)
#define GSL_MIN_DBL(a,b)   GSL_MIN(a,b)
#define GSL_MAX_LDBL(a,b)  GSL_MAX(a,b)
#define GSL_MIN_LDBL(a,b)  GSL_MIN(a,b)
#endif /* HAVE_INLINE */

__END_DECLS

#endif /* __GSL_POW_INT_H__ */

#ifndef M_E
#define M_E        2.71828182845904523536028747135      /* e */
#endif

#ifndef M_LOG2E
#define M_LOG2E    1.44269504088896340735992468100      /* log_2 (e) */
#endif

#ifndef M_LOG10E
#define M_LOG10E   0.43429448190325182765112891892      /* log_10 (e) */
#endif

#ifndef M_SQRT2
#define M_SQRT2    1.41421356237309504880168872421      /* sqrt(2) */
#endif

#ifndef M_SQRT1_2
#define M_SQRT1_2  0.70710678118654752440084436210      /* sqrt(1/2) */
#endif


#ifndef M_SQRT3
#define M_SQRT3    1.73205080756887729352744634151      /* sqrt(3) */
#endif

#ifndef M_PI
#define M_PI       3.14159265358979323846264338328      /* pi */
#endif

#ifndef M_PI_2
#define M_PI_2     1.57079632679489661923132169164      /* pi/2 */
#endif

#ifndef M_PI_4
#define M_PI_4     0.78539816339744830961566084582     /* pi/4 */
#endif

#ifndef M_SQRTPI
#define M_SQRTPI   1.77245385090551602729816748334      /* sqrt(pi) */
#endif

#ifndef M_2_SQRTPI
#define M_2_SQRTPI 1.12837916709551257389615890312      /* 2/sqrt(pi) */
#endif

#ifndef M_1_PI
#define M_1_PI     0.31830988618379067153776752675      /* 1/pi */
#endif

#ifndef M_2_PI
#define M_2_PI     0.63661977236758134307553505349      /* 2/pi */
#endif

#ifndef M_LN10
#define M_LN10     2.30258509299404568401799145468      /* ln(10) */
#endif

#ifndef M_LN2
#define M_LN2      0.69314718055994530941723212146      /* ln(2) */
#endif

#ifndef M_LNPI
#define M_LNPI     1.14472988584940017414342735135      /* ln(pi) */
#endif

#ifndef M_EULER
#define M_EULER    0.57721566490153286060651209008      /* Euler constant */
#endif

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif

__BEGIN_DECLS

/* other needlessly compulsive abstractions */

#define GSL_IS_ODD(n)  ((n) & 1)
#define GSL_IS_EVEN(n) (!(GSL_IS_ODD(n)))
#define GSL_SIGN(x)    ((x) >= 0.0 ? 1 : -1)

/* Return nonzero if x is a real number, i.e. non NaN or infinite. */
#define GSL_IS_REAL(x) (gsl_finite(x))

/* Definition of an arbitrary function with parameters */

struct gsl_function_struct 
{
  double (* function) (double x, void * params);
  void * params;
};

typedef struct gsl_function_struct gsl_function ;

#define GSL_FN_EVAL(F,x) (*((F)->function))(x,(F)->params)

/* Definition of an arbitrary function returning two values, r1, r2 */

struct gsl_function_fdf_struct 
{
  double (* f) (double x, void * params);
  double (* df) (double x, void * params);
  void (* fdf) (double x, void * params, double * f, double * df);
  void * params;
};

typedef struct gsl_function_fdf_struct gsl_function_fdf ;

#define GSL_FN_FDF_EVAL_F(FDF,x) (*((FDF)->f))(x,(FDF)->params)
#define GSL_FN_FDF_EVAL_DF(FDF,x) (*((FDF)->df))(x,(FDF)->params)
#define GSL_FN_FDF_EVAL_F_DF(FDF,x,y,dy) (*((FDF)->fdf))(x,(FDF)->params,(y),(dy))


/* Definition of an arbitrary vector-valued function with parameters */

struct gsl_function_vec_struct 
{
  int (* function) (double x, double y[], void * params);
  void * params;
};

typedef struct gsl_function_vec_struct gsl_function_vec ;

#define GSL_FN_VEC_EVAL(F,x,y) (*((F)->function))(x,y,(F)->params)

__END_DECLS

#endif /* __GSL_MATH_H__ */
#ifndef __GSL_VECTOR_H__
#define __GSL_VECTOR_H__

/* vector/gsl_vector_complex_long_double.h
 * 
 * Copyright (C) 1996, 1997, 1998, 1999, 2000, 2007 Gerard Jungman, Brian Gough
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __GSL_VECTOR_COMPLEX_LONG_DOUBLE_H__
#define __GSL_VECTOR_COMPLEX_LONG_DOUBLE_H__

#include <stdlib.h>
/* err/gsl_errno.h
 * 
 * Copyright (C) 1996, 1997, 1998, 1999, 2000, 2007 Gerard Jungman, Brian Gough
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __GSL_ERRNO_H__
#define __GSL_ERRNO_H__

#include <stdio.h>
#include <errno.h>

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif

__BEGIN_DECLS

enum { 
  GSL_SUCCESS  = 0, 
  GSL_FAILURE  = -1,
  GSL_CONTINUE = -2,  /* iteration has not converged */
  GSL_EDOM     = 1,   /* input domain error, e.g sqrt(-1) */
  GSL_ERANGE   = 2,   /* output range error, e.g. exp(1e100) */
  GSL_EFAULT   = 3,   /* invalid pointer */
  GSL_EINVAL   = 4,   /* invalid argument supplied by user */
  GSL_EFAILED  = 5,   /* generic failure */
  GSL_EFACTOR  = 6,   /* factorization failed */
  GSL_ESANITY  = 7,   /* sanity check failed - shouldn't happen */
  GSL_ENOMEM   = 8,   /* malloc failed */
  GSL_EBADFUNC = 9,   /* problem with user-supplied function */
  GSL_ERUNAWAY = 10,  /* iterative process is out of control */
  GSL_EMAXITER = 11,  /* exceeded max number of iterations */
  GSL_EZERODIV = 12,  /* tried to divide by zero */
  GSL_EBADTOL  = 13,  /* user specified an invalid tolerance */
  GSL_ETOL     = 14,  /* failed to reach the specified tolerance */
  GSL_EUNDRFLW = 15,  /* underflow */
  GSL_EOVRFLW  = 16,  /* overflow  */
  GSL_ELOSS    = 17,  /* loss of accuracy */
  GSL_EROUND   = 18,  /* failed because of roundoff error */
  GSL_EBADLEN  = 19,  /* matrix, vector lengths are not conformant */
  GSL_ENOTSQR  = 20,  /* matrix not square */
  GSL_ESING    = 21,  /* apparent singularity detected */
  GSL_EDIVERGE = 22,  /* integral or series is divergent */
  GSL_EUNSUP   = 23,  /* requested feature is not supported by the hardware */
  GSL_EUNIMPL  = 24,  /* requested feature not (yet) implemented */
  GSL_ECACHE   = 25,  /* cache limit exceeded */
  GSL_ETABLE   = 26,  /* table limit exceeded */
  GSL_ENOPROG  = 27,  /* iteration is not making progress towards solution */
  GSL_ENOPROGJ = 28,  /* jacobian evaluations are not improving the solution */
  GSL_ETOLF    = 29,  /* cannot reach the specified tolerance in F */
  GSL_ETOLX    = 30,  /* cannot reach the specified tolerance in X */
  GSL_ETOLG    = 31,  /* cannot reach the specified tolerance in gradient */
  GSL_EOF      = 32   /* end of file */
} ;

void gsl_error (const char * reason, const char * file, int line,
                int gsl_errno);

void gsl_stream_printf (const char *label, const char *file,
                        int line, const char *reason);

const char * gsl_strerror (const int gsl_errno);

typedef void gsl_error_handler_t (const char * reason, const char * file,
                                  int line, int gsl_errno);

typedef void gsl_stream_handler_t (const char * label, const char * file,
                                   int line, const char * reason);

gsl_error_handler_t * 
gsl_set_error_handler (gsl_error_handler_t * new_handler);

gsl_error_handler_t *
gsl_set_error_handler_off (void);

gsl_stream_handler_t * 
gsl_set_stream_handler (gsl_stream_handler_t * new_handler);

FILE * gsl_set_stream (FILE * new_stream);

/* GSL_ERROR: call the error handler, and return the error code */

#define GSL_ERROR(reason, gsl_errno) \
       do { \
       gsl_error (reason, __FILE__, __LINE__, gsl_errno) ; \
       return gsl_errno ; \
       } while (0)

/* GSL_ERROR_VAL: call the error handler, and return the given value */

#define GSL_ERROR_VAL(reason, gsl_errno, value) \
       do { \
       gsl_error (reason, __FILE__, __LINE__, gsl_errno) ; \
       return value ; \
       } while (0)

/* GSL_ERROR_VOID: call the error handler, and then return
   (for void functions which still need to generate an error) */

#define GSL_ERROR_VOID(reason, gsl_errno) \
       do { \
       gsl_error (reason, __FILE__, __LINE__, gsl_errno) ; \
       return ; \
       } while (0)

/* GSL_ERROR_NULL suitable for out-of-memory conditions */

#define GSL_ERROR_NULL(reason, gsl_errno) GSL_ERROR_VAL(reason, gsl_errno, 0)

/* Sometimes you have several status results returned from
 * function calls and you want to combine them in some sensible
 * way. You cannot produce a "total" status condition, but you can
 * pick one from a set of conditions based on an implied hierarchy.
 *
 * In other words:
 *    you have: status_a, status_b, ...
 *    you want: status = (status_a if it is bad, or status_b if it is bad,...)
 *
 * In this example you consider status_a to be more important and
 * it is checked first, followed by the others in the order specified.
 *
 * Here are some dumb macros to do this.
 */
#define GSL_ERROR_SELECT_2(a,b)       ((a) != GSL_SUCCESS ? (a) : ((b) != GSL_SUCCESS ? (b) : GSL_SUCCESS))
#define GSL_ERROR_SELECT_3(a,b,c)     ((a) != GSL_SUCCESS ? (a) : GSL_ERROR_SELECT_2(b,c))
#define GSL_ERROR_SELECT_4(a,b,c,d)   ((a) != GSL_SUCCESS ? (a) : GSL_ERROR_SELECT_3(b,c,d))
#define GSL_ERROR_SELECT_5(a,b,c,d,e) ((a) != GSL_SUCCESS ? (a) : GSL_ERROR_SELECT_4(b,c,d,e))

#define GSL_STATUS_UPDATE(sp, s) do { if ((s) != GSL_SUCCESS) *(sp) = (s);} while(0)

__END_DECLS

#endif /* __GSL_ERRNO_H__ */
/* complex/gsl_complex.h
 * 
 * Copyright (C) 1996, 1997, 1998, 1999, 2000, 2007 Gerard Jungman, Brian Gough
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __GSL_COMPLEX_H__
#define __GSL_COMPLEX_H__

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif

__BEGIN_DECLS


/* two consecutive built-in types as a complex number */
typedef double *       gsl_complex_packed ;
typedef float *        gsl_complex_packed_float  ;
typedef long double *  gsl_complex_packed_long_double ;

typedef const double *       gsl_const_complex_packed ;
typedef const float *        gsl_const_complex_packed_float  ;
typedef const long double *  gsl_const_complex_packed_long_double ;


/* 2N consecutive built-in types as N complex numbers */
typedef double *       gsl_complex_packed_array ;
typedef float *        gsl_complex_packed_array_float  ;
typedef long double *  gsl_complex_packed_array_long_double ;

typedef const double *       gsl_const_complex_packed_array ;
typedef const float *        gsl_const_complex_packed_array_float  ;
typedef const long double *  gsl_const_complex_packed_array_long_double ;


/* Yes... this seems weird. Trust us. The point is just that
   sometimes you want to make it obvious that something is
   an output value. The fact that it lacks a 'const' may not
   be enough of a clue for people in some contexts.
 */
typedef double *       gsl_complex_packed_ptr ;
typedef float *        gsl_complex_packed_float_ptr  ;
typedef long double *  gsl_complex_packed_long_double_ptr ;

typedef const double *       gsl_const_complex_packed_ptr ;
typedef const float *        gsl_const_complex_packed_float_ptr  ;
typedef const long double *  gsl_const_complex_packed_long_double_ptr ;


typedef struct
  {
    long double dat[2];
  }
gsl_complex_long_double;

typedef struct
  {
    double dat[2];
  }
gsl_complex;

typedef struct
  {
    float dat[2];
  }
gsl_complex_float;

#define GSL_REAL(z)     ((z).dat[0])
#define GSL_IMAG(z)     ((z).dat[1])
#define GSL_COMPLEX_P(zp) ((zp)->dat)
#define GSL_COMPLEX_P_REAL(zp)  ((zp)->dat[0])
#define GSL_COMPLEX_P_IMAG(zp)  ((zp)->dat[1])
#define GSL_COMPLEX_EQ(z1,z2) (((z1).dat[0] == (z2).dat[0]) && ((z1).dat[1] == (z2).dat[1]))

#define GSL_SET_COMPLEX(zp,x,y) do {(zp)->dat[0]=(x); (zp)->dat[1]=(y);} while(0)
#define GSL_SET_REAL(zp,x) do {(zp)->dat[0]=(x);} while(0)
#define GSL_SET_IMAG(zp,y) do {(zp)->dat[1]=(y);} while(0)

#define GSL_SET_COMPLEX_PACKED(zp,n,x,y) do {*((zp)+2*(n))=(x); *((zp)+(2*(n)+1))=(y);} while(0)

__END_DECLS

#endif /* __GSL_COMPLEX_H__ */
/* vector/gsl_check_range.h
 * 
 * Copyright (C) 2003, 2004, 2007 Brian Gough
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __GSL_CHECK_RANGE_H__
#define __GSL_CHECK_RANGE_H__

#include <stdlib.h>

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif

__BEGIN_DECLS

GSL_VAR int gsl_check_range;

/* Turn range checking on by default, unless the user defines
   GSL_RANGE_CHECK_OFF, or defines GSL_RANGE_CHECK to 0 explicitly */

#ifdef GSL_RANGE_CHECK_OFF
# ifndef GSL_RANGE_CHECK
#  define GSL_RANGE_CHECK 0
# else
#  error "cannot set both GSL_RANGE_CHECK and GSL_RANGE_CHECK_OFF"
# endif
#else
# ifndef GSL_RANGE_CHECK
#  define GSL_RANGE_CHECK 1
# endif
#endif

__END_DECLS

#endif /* __GSL_CHECK_RANGE_H__ */
/* vector/gsl_vector_long_double.h
 * 
 * Copyright (C) 1996, 1997, 1998, 1999, 2000, 2007 Gerard Jungman, Brian Gough
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __GSL_VECTOR_LONG_DOUBLE_H__
#define __GSL_VECTOR_LONG_DOUBLE_H__

#include <stdlib.h>
/* block/gsl_block_long_double.h
 * 
 * Copyright (C) 1996, 1997, 1998, 1999, 2000, 2007 Gerard Jungman, Brian Gough
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __GSL_BLOCK_LONG_DOUBLE_H__
#define __GSL_BLOCK_LONG_DOUBLE_H__

#include <stdlib.h>

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif

__BEGIN_DECLS

struct gsl_block_long_double_struct
{
  size_t size;
  long double *data;
};

typedef struct gsl_block_long_double_struct gsl_block_long_double;

gsl_block_long_double *gsl_block_long_double_alloc (const size_t n);
gsl_block_long_double *gsl_block_long_double_calloc (const size_t n);
void gsl_block_long_double_free (gsl_block_long_double * b);

int gsl_block_long_double_fread (FILE * stream, gsl_block_long_double * b);
int gsl_block_long_double_fwrite (FILE * stream, const gsl_block_long_double * b);
int gsl_block_long_double_fscanf (FILE * stream, gsl_block_long_double * b);
int gsl_block_long_double_fprintf (FILE * stream, const gsl_block_long_double * b, const char *format);

int gsl_block_long_double_raw_fread (FILE * stream, long double * b, const size_t n, const size_t stride);
int gsl_block_long_double_raw_fwrite (FILE * stream, const long double * b, const size_t n, const size_t stride);
int gsl_block_long_double_raw_fscanf (FILE * stream, long double * b, const size_t n, const size_t stride);
int gsl_block_long_double_raw_fprintf (FILE * stream, const long double * b, const size_t n, const size_t stride, const char *format);

size_t gsl_block_long_double_size (const gsl_block_long_double * b);
long double * gsl_block_long_double_data (const gsl_block_long_double * b);

__END_DECLS

#endif /* __GSL_BLOCK_LONG_DOUBLE_H__ */

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif

__BEGIN_DECLS

typedef struct 
{
  size_t size;
  size_t stride;
  long double *data;
  gsl_block_long_double *block;
  int owner;
} 
gsl_vector_long_double;

typedef struct
{
  gsl_vector_long_double vector;
} _gsl_vector_long_double_view;

typedef _gsl_vector_long_double_view gsl_vector_long_double_view;

typedef struct
{
  gsl_vector_long_double vector;
} _gsl_vector_long_double_const_view;

typedef const _gsl_vector_long_double_const_view gsl_vector_long_double_const_view;


/* Allocation */

gsl_vector_long_double *gsl_vector_long_double_alloc (const size_t n);
gsl_vector_long_double *gsl_vector_long_double_calloc (const size_t n);

gsl_vector_long_double *gsl_vector_long_double_alloc_from_block (gsl_block_long_double * b,
                                                     const size_t offset, 
                                                     const size_t n, 
                                                     const size_t stride);

gsl_vector_long_double *gsl_vector_long_double_alloc_from_vector (gsl_vector_long_double * v,
                                                      const size_t offset, 
                                                      const size_t n, 
                                                      const size_t stride);

void gsl_vector_long_double_free (gsl_vector_long_double * v);

/* Views */

_gsl_vector_long_double_view 
gsl_vector_long_double_view_array (long double *v, size_t n);

_gsl_vector_long_double_view 
gsl_vector_long_double_view_array_with_stride (long double *base,
                                         size_t stride,
                                         size_t n);

_gsl_vector_long_double_const_view 
gsl_vector_long_double_const_view_array (const long double *v, size_t n);

_gsl_vector_long_double_const_view 
gsl_vector_long_double_const_view_array_with_stride (const long double *base,
                                               size_t stride,
                                               size_t n);

_gsl_vector_long_double_view 
gsl_vector_long_double_subvector (gsl_vector_long_double *v, 
                            size_t i, 
                            size_t n);

_gsl_vector_long_double_view 
gsl_vector_long_double_subvector_with_stride (gsl_vector_long_double *v, 
                                        size_t i,
                                        size_t stride,
                                        size_t n);

_gsl_vector_long_double_const_view 
gsl_vector_long_double_const_subvector (const gsl_vector_long_double *v, 
                                  size_t i, 
                                  size_t n);

_gsl_vector_long_double_const_view 
gsl_vector_long_double_const_subvector_with_stride (const gsl_vector_long_double *v, 
                                              size_t i, 
                                              size_t stride,
                                              size_t n);

/* Operations */

void gsl_vector_long_double_set_zero (gsl_vector_long_double * v);
void gsl_vector_long_double_set_all (gsl_vector_long_double * v, long double x);
int gsl_vector_long_double_set_basis (gsl_vector_long_double * v, size_t i);

int gsl_vector_long_double_fread (FILE * stream, gsl_vector_long_double * v);
int gsl_vector_long_double_fwrite (FILE * stream, const gsl_vector_long_double * v);
int gsl_vector_long_double_fscanf (FILE * stream, gsl_vector_long_double * v);
int gsl_vector_long_double_fprintf (FILE * stream, const gsl_vector_long_double * v,
                              const char *format);

int gsl_vector_long_double_memcpy (gsl_vector_long_double * dest, const gsl_vector_long_double * src);

int gsl_vector_long_double_reverse (gsl_vector_long_double * v);

int gsl_vector_long_double_swap (gsl_vector_long_double * v, gsl_vector_long_double * w);
int gsl_vector_long_double_swap_elements (gsl_vector_long_double * v, const size_t i, const size_t j);

long double gsl_vector_long_double_max (const gsl_vector_long_double * v);
long double gsl_vector_long_double_min (const gsl_vector_long_double * v);
void gsl_vector_long_double_minmax (const gsl_vector_long_double * v, long double * min_out, long double * max_out);

size_t gsl_vector_long_double_max_index (const gsl_vector_long_double * v);
size_t gsl_vector_long_double_min_index (const gsl_vector_long_double * v);
void gsl_vector_long_double_minmax_index (const gsl_vector_long_double * v, size_t * imin, size_t * imax);

int gsl_vector_long_double_add (gsl_vector_long_double * a, const gsl_vector_long_double * b);
int gsl_vector_long_double_sub (gsl_vector_long_double * a, const gsl_vector_long_double * b);
int gsl_vector_long_double_mul (gsl_vector_long_double * a, const gsl_vector_long_double * b);
int gsl_vector_long_double_div (gsl_vector_long_double * a, const gsl_vector_long_double * b);
int gsl_vector_long_double_scale (gsl_vector_long_double * a, const double x);
int gsl_vector_long_double_add_constant (gsl_vector_long_double * a, const double x);

int gsl_vector_long_double_equal (const gsl_vector_long_double * u, 
                            const gsl_vector_long_double * v);

int gsl_vector_long_double_isnull (const gsl_vector_long_double * v);
int gsl_vector_long_double_ispos (const gsl_vector_long_double * v);
int gsl_vector_long_double_isneg (const gsl_vector_long_double * v);
int gsl_vector_long_double_isnonneg (const gsl_vector_long_double * v);

INLINE_DECL long double gsl_vector_long_double_get (const gsl_vector_long_double * v, const size_t i);
INLINE_DECL void gsl_vector_long_double_set (gsl_vector_long_double * v, const size_t i, long double x);
INLINE_DECL long double * gsl_vector_long_double_ptr (gsl_vector_long_double * v, const size_t i);
INLINE_DECL const long double * gsl_vector_long_double_const_ptr (const gsl_vector_long_double * v, const size_t i);

#ifdef HAVE_INLINE

INLINE_FUN
long double
gsl_vector_long_double_get (const gsl_vector_long_double * v, const size_t i)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(i >= v->size))
    {
      GSL_ERROR_VAL ("index out of range", GSL_EINVAL, 0);
    }
#endif
  return v->data[i * v->stride];
}

INLINE_FUN
void
gsl_vector_long_double_set (gsl_vector_long_double * v, const size_t i, long double x)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(i >= v->size))
    {
      GSL_ERROR_VOID ("index out of range", GSL_EINVAL);
    }
#endif
  v->data[i * v->stride] = x;
}

INLINE_FUN
long double *
gsl_vector_long_double_ptr (gsl_vector_long_double * v, const size_t i)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(i >= v->size))
    {
      GSL_ERROR_NULL ("index out of range", GSL_EINVAL);
    }
#endif
  return (long double *) (v->data + i * v->stride);
}

INLINE_FUN
const long double *
gsl_vector_long_double_const_ptr (const gsl_vector_long_double * v, const size_t i)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(i >= v->size))
    {
      GSL_ERROR_NULL ("index out of range", GSL_EINVAL);
    }
#endif
  return (const long double *) (v->data + i * v->stride);
}
#endif /* HAVE_INLINE */

__END_DECLS

#endif /* __GSL_VECTOR_LONG_DOUBLE_H__ */


#ifndef __GSL_VECTOR_COMPLEX_H__
#define __GSL_VECTOR_COMPLEX_H__

#define  GSL_VECTOR_REAL(z, i)  ((z)->data[2*(i)*(z)->stride])
#define  GSL_VECTOR_IMAG(z, i)  ((z)->data[2*(i)*(z)->stride + 1])

#if GSL_RANGE_CHECK
#define GSL_VECTOR_COMPLEX(zv, i) (((i) >= (zv)->size ? (gsl_error ("index out of range", __FILE__, __LINE__, GSL_EINVAL), 0):0 , *GSL_COMPLEX_AT((zv),(i))))
#else
#define GSL_VECTOR_COMPLEX(zv, i) (*GSL_COMPLEX_AT((zv),(i)))
#endif

#define GSL_COMPLEX_AT(zv,i) ((gsl_complex*)&((zv)->data[2*(i)*(zv)->stride]))
#define GSL_COMPLEX_FLOAT_AT(zv,i) ((gsl_complex_float*)&((zv)->data[2*(i)*(zv)->stride]))
#define GSL_COMPLEX_LONG_DOUBLE_AT(zv,i) ((gsl_complex_long_double*)&((zv)->data[2*(i)*(zv)->stride]))

#endif /* __GSL_VECTOR_COMPLEX_H__ */
/* block/gsl_block_complex_long_double.h
 * 
 * Copyright (C) 1996, 1997, 1998, 1999, 2000, 2007 Gerard Jungman, Brian Gough
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __GSL_BLOCK_COMPLEX_LONG_DOUBLE_H__
#define __GSL_BLOCK_COMPLEX_LONG_DOUBLE_H__

#include <stdlib.h>

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif

__BEGIN_DECLS

struct gsl_block_complex_long_double_struct
{
  size_t size;
  long double *data;
};

typedef struct gsl_block_complex_long_double_struct gsl_block_complex_long_double;

gsl_block_complex_long_double *gsl_block_complex_long_double_alloc (const size_t n);
gsl_block_complex_long_double *gsl_block_complex_long_double_calloc (const size_t n);
void gsl_block_complex_long_double_free (gsl_block_complex_long_double * b);

int gsl_block_complex_long_double_fread (FILE * stream, gsl_block_complex_long_double * b);
int gsl_block_complex_long_double_fwrite (FILE * stream, const gsl_block_complex_long_double * b);
int gsl_block_complex_long_double_fscanf (FILE * stream, gsl_block_complex_long_double * b);
int gsl_block_complex_long_double_fprintf (FILE * stream, const gsl_block_complex_long_double * b, const char *format);

int gsl_block_complex_long_double_raw_fread (FILE * stream, long double * b, const size_t n, const size_t stride);
int gsl_block_complex_long_double_raw_fwrite (FILE * stream, const long double * b, const size_t n, const size_t stride);
int gsl_block_complex_long_double_raw_fscanf (FILE * stream, long double * b, const size_t n, const size_t stride);
int gsl_block_complex_long_double_raw_fprintf (FILE * stream, const long double * b, const size_t n, const size_t stride, const char *format);

size_t gsl_block_complex_long_double_size (const gsl_block_complex_long_double * b);
long double * gsl_block_complex_long_double_data (const gsl_block_complex_long_double * b);

__END_DECLS

#endif /* __GSL_BLOCK_COMPLEX_LONG_DOUBLE_H__ */

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif

__BEGIN_DECLS

typedef struct 
{
  size_t size;
  size_t stride;
  long double *data;
  gsl_block_complex_long_double *block;
  int owner;
} gsl_vector_complex_long_double;

typedef struct
{
  gsl_vector_complex_long_double vector;
} _gsl_vector_complex_long_double_view;

typedef _gsl_vector_complex_long_double_view gsl_vector_complex_long_double_view;

typedef struct
{
  gsl_vector_complex_long_double vector;
} _gsl_vector_complex_long_double_const_view;

typedef const _gsl_vector_complex_long_double_const_view gsl_vector_complex_long_double_const_view;

/* Allocation */

gsl_vector_complex_long_double *gsl_vector_complex_long_double_alloc (const size_t n);
gsl_vector_complex_long_double *gsl_vector_complex_long_double_calloc (const size_t n);

gsl_vector_complex_long_double *
gsl_vector_complex_long_double_alloc_from_block (gsl_block_complex_long_double * b, 
                                           const size_t offset, 
                                           const size_t n, 
                                           const size_t stride);

gsl_vector_complex_long_double *
gsl_vector_complex_long_double_alloc_from_vector (gsl_vector_complex_long_double * v, 
                                             const size_t offset, 
                                             const size_t n, 
                                             const size_t stride);

void gsl_vector_complex_long_double_free (gsl_vector_complex_long_double * v);

/* Views */

_gsl_vector_complex_long_double_view
gsl_vector_complex_long_double_view_array (long double *base,
                                     size_t n);

_gsl_vector_complex_long_double_view
gsl_vector_complex_long_double_view_array_with_stride (long double *base,
                                                 size_t stride,
                                                 size_t n);

_gsl_vector_complex_long_double_const_view
gsl_vector_complex_long_double_const_view_array (const long double *base,
                                           size_t n);

_gsl_vector_complex_long_double_const_view
gsl_vector_complex_long_double_const_view_array_with_stride (const long double *base,
                                                       size_t stride,
                                                       size_t n);

_gsl_vector_complex_long_double_view
gsl_vector_complex_long_double_subvector (gsl_vector_complex_long_double *base,
                                         size_t i, 
                                         size_t n);


_gsl_vector_complex_long_double_view 
gsl_vector_complex_long_double_subvector_with_stride (gsl_vector_complex_long_double *v, 
                                                size_t i, 
                                                size_t stride, 
                                                size_t n);

_gsl_vector_complex_long_double_const_view
gsl_vector_complex_long_double_const_subvector (const gsl_vector_complex_long_double *base,
                                               size_t i, 
                                               size_t n);


_gsl_vector_complex_long_double_const_view 
gsl_vector_complex_long_double_const_subvector_with_stride (const gsl_vector_complex_long_double *v, 
                                                      size_t i, 
                                                      size_t stride, 
                                                      size_t n);

_gsl_vector_long_double_view
gsl_vector_complex_long_double_real (gsl_vector_complex_long_double *v);

_gsl_vector_long_double_view 
gsl_vector_complex_long_double_imag (gsl_vector_complex_long_double *v);

_gsl_vector_long_double_const_view
gsl_vector_complex_long_double_const_real (const gsl_vector_complex_long_double *v);

_gsl_vector_long_double_const_view 
gsl_vector_complex_long_double_const_imag (const gsl_vector_complex_long_double *v);


/* Operations */

void gsl_vector_complex_long_double_set_zero (gsl_vector_complex_long_double * v);
void gsl_vector_complex_long_double_set_all (gsl_vector_complex_long_double * v,
                                       gsl_complex_long_double z);
int gsl_vector_complex_long_double_set_basis (gsl_vector_complex_long_double * v, size_t i);

int gsl_vector_complex_long_double_fread (FILE * stream,
                                    gsl_vector_complex_long_double * v);
int gsl_vector_complex_long_double_fwrite (FILE * stream,
                                     const gsl_vector_complex_long_double * v);
int gsl_vector_complex_long_double_fscanf (FILE * stream,
                                     gsl_vector_complex_long_double * v);
int gsl_vector_complex_long_double_fprintf (FILE * stream,
                                      const gsl_vector_complex_long_double * v,
                                      const char *format);

int gsl_vector_complex_long_double_memcpy (gsl_vector_complex_long_double * dest, const gsl_vector_complex_long_double * src);

int gsl_vector_complex_long_double_reverse (gsl_vector_complex_long_double * v);

int gsl_vector_complex_long_double_swap (gsl_vector_complex_long_double * v, gsl_vector_complex_long_double * w);
int gsl_vector_complex_long_double_swap_elements (gsl_vector_complex_long_double * v, const size_t i, const size_t j);

int gsl_vector_complex_long_double_equal (const gsl_vector_complex_long_double * u, 
                                    const gsl_vector_complex_long_double * v);

int gsl_vector_complex_long_double_isnull (const gsl_vector_complex_long_double * v);
int gsl_vector_complex_long_double_ispos (const gsl_vector_complex_long_double * v);
int gsl_vector_complex_long_double_isneg (const gsl_vector_complex_long_double * v);
int gsl_vector_complex_long_double_isnonneg (const gsl_vector_complex_long_double * v);

int gsl_vector_complex_long_double_add (gsl_vector_complex_long_double * a, const gsl_vector_complex_long_double * b);
int gsl_vector_complex_long_double_sub (gsl_vector_complex_long_double * a, const gsl_vector_complex_long_double * b);
int gsl_vector_complex_long_double_mul (gsl_vector_complex_long_double * a, const gsl_vector_complex_long_double * b);
int gsl_vector_complex_long_double_div (gsl_vector_complex_long_double * a, const gsl_vector_complex_long_double * b);
int gsl_vector_complex_long_double_scale (gsl_vector_complex_long_double * a, const gsl_complex_long_double x);
int gsl_vector_complex_long_double_add_constant (gsl_vector_complex_long_double * a, const gsl_complex_long_double x);

INLINE_DECL gsl_complex_long_double gsl_vector_complex_long_double_get (const gsl_vector_complex_long_double * v, const size_t i);
INLINE_DECL void gsl_vector_complex_long_double_set (gsl_vector_complex_long_double * v, const size_t i, gsl_complex_long_double z);
INLINE_DECL gsl_complex_long_double *gsl_vector_complex_long_double_ptr (gsl_vector_complex_long_double * v, const size_t i);
INLINE_DECL const gsl_complex_long_double *gsl_vector_complex_long_double_const_ptr (const gsl_vector_complex_long_double * v, const size_t i);

#ifdef HAVE_INLINE

INLINE_FUN
gsl_complex_long_double
gsl_vector_complex_long_double_get (const gsl_vector_complex_long_double * v,
                              const size_t i)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(i >= v->size))
    {
      gsl_complex_long_double zero = {{0, 0}};
      GSL_ERROR_VAL ("index out of range", GSL_EINVAL, zero);
    }
#endif
  return *GSL_COMPLEX_LONG_DOUBLE_AT (v, i);
}

INLINE_FUN
void
gsl_vector_complex_long_double_set (gsl_vector_complex_long_double * v,
                              const size_t i, gsl_complex_long_double z)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(i >= v->size))
    {
      GSL_ERROR_VOID ("index out of range", GSL_EINVAL);
    }
#endif
  *GSL_COMPLEX_LONG_DOUBLE_AT (v, i) = z;
}

INLINE_FUN
gsl_complex_long_double *
gsl_vector_complex_long_double_ptr (gsl_vector_complex_long_double * v,
                              const size_t i)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(i >= v->size))
    {
      GSL_ERROR_NULL ("index out of range", GSL_EINVAL);
    }
#endif
  return GSL_COMPLEX_LONG_DOUBLE_AT (v, i);
}

INLINE_FUN
const gsl_complex_long_double *
gsl_vector_complex_long_double_const_ptr (const gsl_vector_complex_long_double * v,
                                    const size_t i)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(i >= v->size))
    {
      GSL_ERROR_NULL ("index out of range", GSL_EINVAL);
    }
#endif
  return GSL_COMPLEX_LONG_DOUBLE_AT (v, i);
}


#endif /* HAVE_INLINE */

__END_DECLS

#endif /* __GSL_VECTOR_COMPLEX_LONG_DOUBLE_H__ */
/* vector/gsl_vector_complex_double.h
 * 
 * Copyright (C) 1996, 1997, 1998, 1999, 2000, 2007 Gerard Jungman, Brian Gough
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __GSL_VECTOR_COMPLEX_DOUBLE_H__
#define __GSL_VECTOR_COMPLEX_DOUBLE_H__

#include <stdlib.h>
/* vector/gsl_vector_double.h
 * 
 * Copyright (C) 1996, 1997, 1998, 1999, 2000, 2007 Gerard Jungman, Brian Gough
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __GSL_VECTOR_DOUBLE_H__
#define __GSL_VECTOR_DOUBLE_H__

#include <stdlib.h>
/* block/gsl_block_double.h
 * 
 * Copyright (C) 1996, 1997, 1998, 1999, 2000, 2007 Gerard Jungman, Brian Gough
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __GSL_BLOCK_DOUBLE_H__
#define __GSL_BLOCK_DOUBLE_H__

#include <stdlib.h>

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif

__BEGIN_DECLS

struct gsl_block_struct
{
  size_t size;
  double *data;
};

typedef struct gsl_block_struct gsl_block;

gsl_block *gsl_block_alloc (const size_t n);
gsl_block *gsl_block_calloc (const size_t n);
void gsl_block_free (gsl_block * b);

int gsl_block_fread (FILE * stream, gsl_block * b);
int gsl_block_fwrite (FILE * stream, const gsl_block * b);
int gsl_block_fscanf (FILE * stream, gsl_block * b);
int gsl_block_fprintf (FILE * stream, const gsl_block * b, const char *format);

int gsl_block_raw_fread (FILE * stream, double * b, const size_t n, const size_t stride);
int gsl_block_raw_fwrite (FILE * stream, const double * b, const size_t n, const size_t stride);
int gsl_block_raw_fscanf (FILE * stream, double * b, const size_t n, const size_t stride);
int gsl_block_raw_fprintf (FILE * stream, const double * b, const size_t n, const size_t stride, const char *format);

size_t gsl_block_size (const gsl_block * b);
double * gsl_block_data (const gsl_block * b);

__END_DECLS

#endif /* __GSL_BLOCK_DOUBLE_H__ */

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif

__BEGIN_DECLS

typedef struct 
{
  size_t size;
  size_t stride;
  double *data;
  gsl_block *block;
  int owner;
} 
gsl_vector;

typedef struct
{
  gsl_vector vector;
} _gsl_vector_view;

typedef _gsl_vector_view gsl_vector_view;

typedef struct
{
  gsl_vector vector;
} _gsl_vector_const_view;

typedef const _gsl_vector_const_view gsl_vector_const_view;


/* Allocation */

gsl_vector *gsl_vector_alloc (const size_t n);
gsl_vector *gsl_vector_calloc (const size_t n);

gsl_vector *gsl_vector_alloc_from_block (gsl_block * b,
                                                     const size_t offset, 
                                                     const size_t n, 
                                                     const size_t stride);

gsl_vector *gsl_vector_alloc_from_vector (gsl_vector * v,
                                                      const size_t offset, 
                                                      const size_t n, 
                                                      const size_t stride);

void gsl_vector_free (gsl_vector * v);

/* Views */

_gsl_vector_view 
gsl_vector_view_array (double *v, size_t n);

_gsl_vector_view 
gsl_vector_view_array_with_stride (double *base,
                                         size_t stride,
                                         size_t n);

_gsl_vector_const_view 
gsl_vector_const_view_array (const double *v, size_t n);

_gsl_vector_const_view 
gsl_vector_const_view_array_with_stride (const double *base,
                                               size_t stride,
                                               size_t n);

_gsl_vector_view 
gsl_vector_subvector (gsl_vector *v, 
                            size_t i, 
                            size_t n);

_gsl_vector_view 
gsl_vector_subvector_with_stride (gsl_vector *v, 
                                        size_t i,
                                        size_t stride,
                                        size_t n);

_gsl_vector_const_view 
gsl_vector_const_subvector (const gsl_vector *v, 
                                  size_t i, 
                                  size_t n);

_gsl_vector_const_view 
gsl_vector_const_subvector_with_stride (const gsl_vector *v, 
                                              size_t i, 
                                              size_t stride,
                                              size_t n);

/* Operations */

void gsl_vector_set_zero (gsl_vector * v);
void gsl_vector_set_all (gsl_vector * v, double x);
int gsl_vector_set_basis (gsl_vector * v, size_t i);

int gsl_vector_fread (FILE * stream, gsl_vector * v);
int gsl_vector_fwrite (FILE * stream, const gsl_vector * v);
int gsl_vector_fscanf (FILE * stream, gsl_vector * v);
int gsl_vector_fprintf (FILE * stream, const gsl_vector * v,
                              const char *format);

int gsl_vector_memcpy (gsl_vector * dest, const gsl_vector * src);

int gsl_vector_reverse (gsl_vector * v);

int gsl_vector_swap (gsl_vector * v, gsl_vector * w);
int gsl_vector_swap_elements (gsl_vector * v, const size_t i, const size_t j);

double gsl_vector_max (const gsl_vector * v);
double gsl_vector_min (const gsl_vector * v);
void gsl_vector_minmax (const gsl_vector * v, double * min_out, double * max_out);

size_t gsl_vector_max_index (const gsl_vector * v);
size_t gsl_vector_min_index (const gsl_vector * v);
void gsl_vector_minmax_index (const gsl_vector * v, size_t * imin, size_t * imax);

int gsl_vector_add (gsl_vector * a, const gsl_vector * b);
int gsl_vector_sub (gsl_vector * a, const gsl_vector * b);
int gsl_vector_mul (gsl_vector * a, const gsl_vector * b);
int gsl_vector_div (gsl_vector * a, const gsl_vector * b);
int gsl_vector_scale (gsl_vector * a, const double x);
int gsl_vector_add_constant (gsl_vector * a, const double x);

int gsl_vector_equal (const gsl_vector * u, 
                            const gsl_vector * v);

int gsl_vector_isnull (const gsl_vector * v);
int gsl_vector_ispos (const gsl_vector * v);
int gsl_vector_isneg (const gsl_vector * v);
int gsl_vector_isnonneg (const gsl_vector * v);

INLINE_DECL double gsl_vector_get (const gsl_vector * v, const size_t i);
INLINE_DECL void gsl_vector_set (gsl_vector * v, const size_t i, double x);
INLINE_DECL double * gsl_vector_ptr (gsl_vector * v, const size_t i);
INLINE_DECL const double * gsl_vector_const_ptr (const gsl_vector * v, const size_t i);

#ifdef HAVE_INLINE

INLINE_FUN
double
gsl_vector_get (const gsl_vector * v, const size_t i)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(i >= v->size))
    {
      GSL_ERROR_VAL ("index out of range", GSL_EINVAL, 0);
    }
#endif
  return v->data[i * v->stride];
}

INLINE_FUN
void
gsl_vector_set (gsl_vector * v, const size_t i, double x)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(i >= v->size))
    {
      GSL_ERROR_VOID ("index out of range", GSL_EINVAL);
    }
#endif
  v->data[i * v->stride] = x;
}

INLINE_FUN
double *
gsl_vector_ptr (gsl_vector * v, const size_t i)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(i >= v->size))
    {
      GSL_ERROR_NULL ("index out of range", GSL_EINVAL);
    }
#endif
  return (double *) (v->data + i * v->stride);
}

INLINE_FUN
const double *
gsl_vector_const_ptr (const gsl_vector * v, const size_t i)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(i >= v->size))
    {
      GSL_ERROR_NULL ("index out of range", GSL_EINVAL);
    }
#endif
  return (const double *) (v->data + i * v->stride);
}
#endif /* HAVE_INLINE */

__END_DECLS

#endif /* __GSL_VECTOR_DOUBLE_H__ */


/* block/gsl_block_complex_double.h
 * 
 * Copyright (C) 1996, 1997, 1998, 1999, 2000, 2007 Gerard Jungman, Brian Gough
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __GSL_BLOCK_COMPLEX_DOUBLE_H__
#define __GSL_BLOCK_COMPLEX_DOUBLE_H__

#include <stdlib.h>

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif

__BEGIN_DECLS

struct gsl_block_complex_struct
{
  size_t size;
  double *data;
};

typedef struct gsl_block_complex_struct gsl_block_complex;

gsl_block_complex *gsl_block_complex_alloc (const size_t n);
gsl_block_complex *gsl_block_complex_calloc (const size_t n);
void gsl_block_complex_free (gsl_block_complex * b);

int gsl_block_complex_fread (FILE * stream, gsl_block_complex * b);
int gsl_block_complex_fwrite (FILE * stream, const gsl_block_complex * b);
int gsl_block_complex_fscanf (FILE * stream, gsl_block_complex * b);
int gsl_block_complex_fprintf (FILE * stream, const gsl_block_complex * b, const char *format);

int gsl_block_complex_raw_fread (FILE * stream, double * b, const size_t n, const size_t stride);
int gsl_block_complex_raw_fwrite (FILE * stream, const double * b, const size_t n, const size_t stride);
int gsl_block_complex_raw_fscanf (FILE * stream, double * b, const size_t n, const size_t stride);
int gsl_block_complex_raw_fprintf (FILE * stream, const double * b, const size_t n, const size_t stride, const char *format);

size_t gsl_block_complex_size (const gsl_block_complex * b);
double * gsl_block_complex_data (const gsl_block_complex * b);

__END_DECLS

#endif /* __GSL_BLOCK_COMPLEX_DOUBLE_H__ */

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif

__BEGIN_DECLS

typedef struct 
{
  size_t size;
  size_t stride;
  double *data;
  gsl_block_complex *block;
  int owner;
} gsl_vector_complex;

typedef struct
{
  gsl_vector_complex vector;
} _gsl_vector_complex_view;

typedef _gsl_vector_complex_view gsl_vector_complex_view;

typedef struct
{
  gsl_vector_complex vector;
} _gsl_vector_complex_const_view;

typedef const _gsl_vector_complex_const_view gsl_vector_complex_const_view;

/* Allocation */

gsl_vector_complex *gsl_vector_complex_alloc (const size_t n);
gsl_vector_complex *gsl_vector_complex_calloc (const size_t n);

gsl_vector_complex *
gsl_vector_complex_alloc_from_block (gsl_block_complex * b, 
                                           const size_t offset, 
                                           const size_t n, 
                                           const size_t stride);

gsl_vector_complex *
gsl_vector_complex_alloc_from_vector (gsl_vector_complex * v, 
                                             const size_t offset, 
                                             const size_t n, 
                                             const size_t stride);

void gsl_vector_complex_free (gsl_vector_complex * v);

/* Views */

_gsl_vector_complex_view
gsl_vector_complex_view_array (double *base,
                                     size_t n);

_gsl_vector_complex_view
gsl_vector_complex_view_array_with_stride (double *base,
                                                 size_t stride,
                                                 size_t n);

_gsl_vector_complex_const_view
gsl_vector_complex_const_view_array (const double *base,
                                           size_t n);

_gsl_vector_complex_const_view
gsl_vector_complex_const_view_array_with_stride (const double *base,
                                                       size_t stride,
                                                       size_t n);

_gsl_vector_complex_view
gsl_vector_complex_subvector (gsl_vector_complex *base,
                                         size_t i, 
                                         size_t n);


_gsl_vector_complex_view 
gsl_vector_complex_subvector_with_stride (gsl_vector_complex *v, 
                                                size_t i, 
                                                size_t stride, 
                                                size_t n);

_gsl_vector_complex_const_view
gsl_vector_complex_const_subvector (const gsl_vector_complex *base,
                                               size_t i, 
                                               size_t n);


_gsl_vector_complex_const_view 
gsl_vector_complex_const_subvector_with_stride (const gsl_vector_complex *v, 
                                                      size_t i, 
                                                      size_t stride, 
                                                      size_t n);

_gsl_vector_view
gsl_vector_complex_real (gsl_vector_complex *v);

_gsl_vector_view 
gsl_vector_complex_imag (gsl_vector_complex *v);

_gsl_vector_const_view
gsl_vector_complex_const_real (const gsl_vector_complex *v);

_gsl_vector_const_view 
gsl_vector_complex_const_imag (const gsl_vector_complex *v);


/* Operations */

void gsl_vector_complex_set_zero (gsl_vector_complex * v);
void gsl_vector_complex_set_all (gsl_vector_complex * v,
                                       gsl_complex z);
int gsl_vector_complex_set_basis (gsl_vector_complex * v, size_t i);

int gsl_vector_complex_fread (FILE * stream,
                                    gsl_vector_complex * v);
int gsl_vector_complex_fwrite (FILE * stream,
                                     const gsl_vector_complex * v);
int gsl_vector_complex_fscanf (FILE * stream,
                                     gsl_vector_complex * v);
int gsl_vector_complex_fprintf (FILE * stream,
                                      const gsl_vector_complex * v,
                                      const char *format);

int gsl_vector_complex_memcpy (gsl_vector_complex * dest, const gsl_vector_complex * src);

int gsl_vector_complex_reverse (gsl_vector_complex * v);

int gsl_vector_complex_swap (gsl_vector_complex * v, gsl_vector_complex * w);
int gsl_vector_complex_swap_elements (gsl_vector_complex * v, const size_t i, const size_t j);

int gsl_vector_complex_equal (const gsl_vector_complex * u, 
                                    const gsl_vector_complex * v);

int gsl_vector_complex_isnull (const gsl_vector_complex * v);
int gsl_vector_complex_ispos (const gsl_vector_complex * v);
int gsl_vector_complex_isneg (const gsl_vector_complex * v);
int gsl_vector_complex_isnonneg (const gsl_vector_complex * v);

int gsl_vector_complex_add (gsl_vector_complex * a, const gsl_vector_complex * b);
int gsl_vector_complex_sub (gsl_vector_complex * a, const gsl_vector_complex * b);
int gsl_vector_complex_mul (gsl_vector_complex * a, const gsl_vector_complex * b);
int gsl_vector_complex_div (gsl_vector_complex * a, const gsl_vector_complex * b);
int gsl_vector_complex_scale (gsl_vector_complex * a, const gsl_complex x);
int gsl_vector_complex_add_constant (gsl_vector_complex * a, const gsl_complex x);

INLINE_DECL gsl_complex gsl_vector_complex_get (const gsl_vector_complex * v, const size_t i);
INLINE_DECL void gsl_vector_complex_set (gsl_vector_complex * v, const size_t i, gsl_complex z);
INLINE_DECL gsl_complex *gsl_vector_complex_ptr (gsl_vector_complex * v, const size_t i);
INLINE_DECL const gsl_complex *gsl_vector_complex_const_ptr (const gsl_vector_complex * v, const size_t i);

#ifdef HAVE_INLINE

INLINE_FUN
gsl_complex
gsl_vector_complex_get (const gsl_vector_complex * v,
                              const size_t i)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(i >= v->size))
    {
      gsl_complex zero = {{0, 0}};
      GSL_ERROR_VAL ("index out of range", GSL_EINVAL, zero);
    }
#endif
  return *GSL_COMPLEX_AT (v, i);
}

INLINE_FUN
void
gsl_vector_complex_set (gsl_vector_complex * v,
                              const size_t i, gsl_complex z)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(i >= v->size))
    {
      GSL_ERROR_VOID ("index out of range", GSL_EINVAL);
    }
#endif
  *GSL_COMPLEX_AT (v, i) = z;
}

INLINE_FUN
gsl_complex *
gsl_vector_complex_ptr (gsl_vector_complex * v,
                              const size_t i)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(i >= v->size))
    {
      GSL_ERROR_NULL ("index out of range", GSL_EINVAL);
    }
#endif
  return GSL_COMPLEX_AT (v, i);
}

INLINE_FUN
const gsl_complex *
gsl_vector_complex_const_ptr (const gsl_vector_complex * v,
                                    const size_t i)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(i >= v->size))
    {
      GSL_ERROR_NULL ("index out of range", GSL_EINVAL);
    }
#endif
  return GSL_COMPLEX_AT (v, i);
}


#endif /* HAVE_INLINE */

__END_DECLS

#endif /* __GSL_VECTOR_COMPLEX_DOUBLE_H__ */
/* vector/gsl_vector_complex_float.h
 * 
 * Copyright (C) 1996, 1997, 1998, 1999, 2000, 2007 Gerard Jungman, Brian Gough
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __GSL_VECTOR_COMPLEX_FLOAT_H__
#define __GSL_VECTOR_COMPLEX_FLOAT_H__

#include <stdlib.h>
/* vector/gsl_vector_float.h
 * 
 * Copyright (C) 1996, 1997, 1998, 1999, 2000, 2007 Gerard Jungman, Brian Gough
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __GSL_VECTOR_FLOAT_H__
#define __GSL_VECTOR_FLOAT_H__

#include <stdlib.h>
/* block/gsl_block_float.h
 * 
 * Copyright (C) 1996, 1997, 1998, 1999, 2000, 2007 Gerard Jungman, Brian Gough
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __GSL_BLOCK_FLOAT_H__
#define __GSL_BLOCK_FLOAT_H__

#include <stdlib.h>

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif

__BEGIN_DECLS

struct gsl_block_float_struct
{
  size_t size;
  float *data;
};

typedef struct gsl_block_float_struct gsl_block_float;

gsl_block_float *gsl_block_float_alloc (const size_t n);
gsl_block_float *gsl_block_float_calloc (const size_t n);
void gsl_block_float_free (gsl_block_float * b);

int gsl_block_float_fread (FILE * stream, gsl_block_float * b);
int gsl_block_float_fwrite (FILE * stream, const gsl_block_float * b);
int gsl_block_float_fscanf (FILE * stream, gsl_block_float * b);
int gsl_block_float_fprintf (FILE * stream, const gsl_block_float * b, const char *format);

int gsl_block_float_raw_fread (FILE * stream, float * b, const size_t n, const size_t stride);
int gsl_block_float_raw_fwrite (FILE * stream, const float * b, const size_t n, const size_t stride);
int gsl_block_float_raw_fscanf (FILE * stream, float * b, const size_t n, const size_t stride);
int gsl_block_float_raw_fprintf (FILE * stream, const float * b, const size_t n, const size_t stride, const char *format);

size_t gsl_block_float_size (const gsl_block_float * b);
float * gsl_block_float_data (const gsl_block_float * b);

__END_DECLS

#endif /* __GSL_BLOCK_FLOAT_H__ */

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif

__BEGIN_DECLS

typedef struct 
{
  size_t size;
  size_t stride;
  float *data;
  gsl_block_float *block;
  int owner;
} 
gsl_vector_float;

typedef struct
{
  gsl_vector_float vector;
} _gsl_vector_float_view;

typedef _gsl_vector_float_view gsl_vector_float_view;

typedef struct
{
  gsl_vector_float vector;
} _gsl_vector_float_const_view;

typedef const _gsl_vector_float_const_view gsl_vector_float_const_view;


/* Allocation */

gsl_vector_float *gsl_vector_float_alloc (const size_t n);
gsl_vector_float *gsl_vector_float_calloc (const size_t n);

gsl_vector_float *gsl_vector_float_alloc_from_block (gsl_block_float * b,
                                                     const size_t offset, 
                                                     const size_t n, 
                                                     const size_t stride);

gsl_vector_float *gsl_vector_float_alloc_from_vector (gsl_vector_float * v,
                                                      const size_t offset, 
                                                      const size_t n, 
                                                      const size_t stride);

void gsl_vector_float_free (gsl_vector_float * v);

/* Views */

_gsl_vector_float_view 
gsl_vector_float_view_array (float *v, size_t n);

_gsl_vector_float_view 
gsl_vector_float_view_array_with_stride (float *base,
                                         size_t stride,
                                         size_t n);

_gsl_vector_float_const_view 
gsl_vector_float_const_view_array (const float *v, size_t n);

_gsl_vector_float_const_view 
gsl_vector_float_const_view_array_with_stride (const float *base,
                                               size_t stride,
                                               size_t n);

_gsl_vector_float_view 
gsl_vector_float_subvector (gsl_vector_float *v, 
                            size_t i, 
                            size_t n);

_gsl_vector_float_view 
gsl_vector_float_subvector_with_stride (gsl_vector_float *v, 
                                        size_t i,
                                        size_t stride,
                                        size_t n);

_gsl_vector_float_const_view 
gsl_vector_float_const_subvector (const gsl_vector_float *v, 
                                  size_t i, 
                                  size_t n);

_gsl_vector_float_const_view 
gsl_vector_float_const_subvector_with_stride (const gsl_vector_float *v, 
                                              size_t i, 
                                              size_t stride,
                                              size_t n);

/* Operations */

void gsl_vector_float_set_zero (gsl_vector_float * v);
void gsl_vector_float_set_all (gsl_vector_float * v, float x);
int gsl_vector_float_set_basis (gsl_vector_float * v, size_t i);

int gsl_vector_float_fread (FILE * stream, gsl_vector_float * v);
int gsl_vector_float_fwrite (FILE * stream, const gsl_vector_float * v);
int gsl_vector_float_fscanf (FILE * stream, gsl_vector_float * v);
int gsl_vector_float_fprintf (FILE * stream, const gsl_vector_float * v,
                              const char *format);

int gsl_vector_float_memcpy (gsl_vector_float * dest, const gsl_vector_float * src);

int gsl_vector_float_reverse (gsl_vector_float * v);

int gsl_vector_float_swap (gsl_vector_float * v, gsl_vector_float * w);
int gsl_vector_float_swap_elements (gsl_vector_float * v, const size_t i, const size_t j);

float gsl_vector_float_max (const gsl_vector_float * v);
float gsl_vector_float_min (const gsl_vector_float * v);
void gsl_vector_float_minmax (const gsl_vector_float * v, float * min_out, float * max_out);

size_t gsl_vector_float_max_index (const gsl_vector_float * v);
size_t gsl_vector_float_min_index (const gsl_vector_float * v);
void gsl_vector_float_minmax_index (const gsl_vector_float * v, size_t * imin, size_t * imax);

int gsl_vector_float_add (gsl_vector_float * a, const gsl_vector_float * b);
int gsl_vector_float_sub (gsl_vector_float * a, const gsl_vector_float * b);
int gsl_vector_float_mul (gsl_vector_float * a, const gsl_vector_float * b);
int gsl_vector_float_div (gsl_vector_float * a, const gsl_vector_float * b);
int gsl_vector_float_scale (gsl_vector_float * a, const double x);
int gsl_vector_float_add_constant (gsl_vector_float * a, const double x);

int gsl_vector_float_equal (const gsl_vector_float * u, 
                            const gsl_vector_float * v);

int gsl_vector_float_isnull (const gsl_vector_float * v);
int gsl_vector_float_ispos (const gsl_vector_float * v);
int gsl_vector_float_isneg (const gsl_vector_float * v);
int gsl_vector_float_isnonneg (const gsl_vector_float * v);

INLINE_DECL float gsl_vector_float_get (const gsl_vector_float * v, const size_t i);
INLINE_DECL void gsl_vector_float_set (gsl_vector_float * v, const size_t i, float x);
INLINE_DECL float * gsl_vector_float_ptr (gsl_vector_float * v, const size_t i);
INLINE_DECL const float * gsl_vector_float_const_ptr (const gsl_vector_float * v, const size_t i);

#ifdef HAVE_INLINE

INLINE_FUN
float
gsl_vector_float_get (const gsl_vector_float * v, const size_t i)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(i >= v->size))
    {
      GSL_ERROR_VAL ("index out of range", GSL_EINVAL, 0);
    }
#endif
  return v->data[i * v->stride];
}

INLINE_FUN
void
gsl_vector_float_set (gsl_vector_float * v, const size_t i, float x)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(i >= v->size))
    {
      GSL_ERROR_VOID ("index out of range", GSL_EINVAL);
    }
#endif
  v->data[i * v->stride] = x;
}

INLINE_FUN
float *
gsl_vector_float_ptr (gsl_vector_float * v, const size_t i)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(i >= v->size))
    {
      GSL_ERROR_NULL ("index out of range", GSL_EINVAL);
    }
#endif
  return (float *) (v->data + i * v->stride);
}

INLINE_FUN
const float *
gsl_vector_float_const_ptr (const gsl_vector_float * v, const size_t i)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(i >= v->size))
    {
      GSL_ERROR_NULL ("index out of range", GSL_EINVAL);
    }
#endif
  return (const float *) (v->data + i * v->stride);
}
#endif /* HAVE_INLINE */

__END_DECLS

#endif /* __GSL_VECTOR_FLOAT_H__ */


/* block/gsl_block_complex_float.h
 * 
 * Copyright (C) 1996, 1997, 1998, 1999, 2000, 2007 Gerard Jungman, Brian Gough
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __GSL_BLOCK_COMPLEX_FLOAT_H__
#define __GSL_BLOCK_COMPLEX_FLOAT_H__

#include <stdlib.h>

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif

__BEGIN_DECLS

struct gsl_block_complex_float_struct
{
  size_t size;
  float *data;
};

typedef struct gsl_block_complex_float_struct gsl_block_complex_float;

gsl_block_complex_float *gsl_block_complex_float_alloc (const size_t n);
gsl_block_complex_float *gsl_block_complex_float_calloc (const size_t n);
void gsl_block_complex_float_free (gsl_block_complex_float * b);

int gsl_block_complex_float_fread (FILE * stream, gsl_block_complex_float * b);
int gsl_block_complex_float_fwrite (FILE * stream, const gsl_block_complex_float * b);
int gsl_block_complex_float_fscanf (FILE * stream, gsl_block_complex_float * b);
int gsl_block_complex_float_fprintf (FILE * stream, const gsl_block_complex_float * b, const char *format);

int gsl_block_complex_float_raw_fread (FILE * stream, float * b, const size_t n, const size_t stride);
int gsl_block_complex_float_raw_fwrite (FILE * stream, const float * b, const size_t n, const size_t stride);
int gsl_block_complex_float_raw_fscanf (FILE * stream, float * b, const size_t n, const size_t stride);
int gsl_block_complex_float_raw_fprintf (FILE * stream, const float * b, const size_t n, const size_t stride, const char *format);

size_t gsl_block_complex_float_size (const gsl_block_complex_float * b);
float * gsl_block_complex_float_data (const gsl_block_complex_float * b);

__END_DECLS

#endif /* __GSL_BLOCK_COMPLEX_FLOAT_H__ */

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif

__BEGIN_DECLS

typedef struct 
{
  size_t size;
  size_t stride;
  float *data;
  gsl_block_complex_float *block;
  int owner;
} gsl_vector_complex_float;

typedef struct
{
  gsl_vector_complex_float vector;
} _gsl_vector_complex_float_view;

typedef _gsl_vector_complex_float_view gsl_vector_complex_float_view;

typedef struct
{
  gsl_vector_complex_float vector;
} _gsl_vector_complex_float_const_view;

typedef const _gsl_vector_complex_float_const_view gsl_vector_complex_float_const_view;

/* Allocation */

gsl_vector_complex_float *gsl_vector_complex_float_alloc (const size_t n);
gsl_vector_complex_float *gsl_vector_complex_float_calloc (const size_t n);

gsl_vector_complex_float *
gsl_vector_complex_float_alloc_from_block (gsl_block_complex_float * b, 
                                           const size_t offset, 
                                           const size_t n, 
                                           const size_t stride);

gsl_vector_complex_float *
gsl_vector_complex_float_alloc_from_vector (gsl_vector_complex_float * v, 
                                             const size_t offset, 
                                             const size_t n, 
                                             const size_t stride);

void gsl_vector_complex_float_free (gsl_vector_complex_float * v);

/* Views */

_gsl_vector_complex_float_view
gsl_vector_complex_float_view_array (float *base,
                                     size_t n);

_gsl_vector_complex_float_view
gsl_vector_complex_float_view_array_with_stride (float *base,
                                                 size_t stride,
                                                 size_t n);

_gsl_vector_complex_float_const_view
gsl_vector_complex_float_const_view_array (const float *base,
                                           size_t n);

_gsl_vector_complex_float_const_view
gsl_vector_complex_float_const_view_array_with_stride (const float *base,
                                                       size_t stride,
                                                       size_t n);

_gsl_vector_complex_float_view
gsl_vector_complex_float_subvector (gsl_vector_complex_float *base,
                                         size_t i, 
                                         size_t n);


_gsl_vector_complex_float_view 
gsl_vector_complex_float_subvector_with_stride (gsl_vector_complex_float *v, 
                                                size_t i, 
                                                size_t stride, 
                                                size_t n);

_gsl_vector_complex_float_const_view
gsl_vector_complex_float_const_subvector (const gsl_vector_complex_float *base,
                                               size_t i, 
                                               size_t n);


_gsl_vector_complex_float_const_view 
gsl_vector_complex_float_const_subvector_with_stride (const gsl_vector_complex_float *v, 
                                                      size_t i, 
                                                      size_t stride, 
                                                      size_t n);

_gsl_vector_float_view
gsl_vector_complex_float_real (gsl_vector_complex_float *v);

_gsl_vector_float_view 
gsl_vector_complex_float_imag (gsl_vector_complex_float *v);

_gsl_vector_float_const_view
gsl_vector_complex_float_const_real (const gsl_vector_complex_float *v);

_gsl_vector_float_const_view 
gsl_vector_complex_float_const_imag (const gsl_vector_complex_float *v);


/* Operations */

void gsl_vector_complex_float_set_zero (gsl_vector_complex_float * v);
void gsl_vector_complex_float_set_all (gsl_vector_complex_float * v,
                                       gsl_complex_float z);
int gsl_vector_complex_float_set_basis (gsl_vector_complex_float * v, size_t i);

int gsl_vector_complex_float_fread (FILE * stream,
                                    gsl_vector_complex_float * v);
int gsl_vector_complex_float_fwrite (FILE * stream,
                                     const gsl_vector_complex_float * v);
int gsl_vector_complex_float_fscanf (FILE * stream,
                                     gsl_vector_complex_float * v);
int gsl_vector_complex_float_fprintf (FILE * stream,
                                      const gsl_vector_complex_float * v,
                                      const char *format);

int gsl_vector_complex_float_memcpy (gsl_vector_complex_float * dest, const gsl_vector_complex_float * src);

int gsl_vector_complex_float_reverse (gsl_vector_complex_float * v);

int gsl_vector_complex_float_swap (gsl_vector_complex_float * v, gsl_vector_complex_float * w);
int gsl_vector_complex_float_swap_elements (gsl_vector_complex_float * v, const size_t i, const size_t j);

int gsl_vector_complex_float_equal (const gsl_vector_complex_float * u, 
                                    const gsl_vector_complex_float * v);

int gsl_vector_complex_float_isnull (const gsl_vector_complex_float * v);
int gsl_vector_complex_float_ispos (const gsl_vector_complex_float * v);
int gsl_vector_complex_float_isneg (const gsl_vector_complex_float * v);
int gsl_vector_complex_float_isnonneg (const gsl_vector_complex_float * v);

int gsl_vector_complex_float_add (gsl_vector_complex_float * a, const gsl_vector_complex_float * b);
int gsl_vector_complex_float_sub (gsl_vector_complex_float * a, const gsl_vector_complex_float * b);
int gsl_vector_complex_float_mul (gsl_vector_complex_float * a, const gsl_vector_complex_float * b);
int gsl_vector_complex_float_div (gsl_vector_complex_float * a, const gsl_vector_complex_float * b);
int gsl_vector_complex_float_scale (gsl_vector_complex_float * a, const gsl_complex_float x);
int gsl_vector_complex_float_add_constant (gsl_vector_complex_float * a, const gsl_complex_float x);

INLINE_DECL gsl_complex_float gsl_vector_complex_float_get (const gsl_vector_complex_float * v, const size_t i);
INLINE_DECL void gsl_vector_complex_float_set (gsl_vector_complex_float * v, const size_t i, gsl_complex_float z);
INLINE_DECL gsl_complex_float *gsl_vector_complex_float_ptr (gsl_vector_complex_float * v, const size_t i);
INLINE_DECL const gsl_complex_float *gsl_vector_complex_float_const_ptr (const gsl_vector_complex_float * v, const size_t i);

#ifdef HAVE_INLINE

INLINE_FUN
gsl_complex_float
gsl_vector_complex_float_get (const gsl_vector_complex_float * v,
                              const size_t i)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(i >= v->size))
    {
      gsl_complex_float zero = {{0, 0}};
      GSL_ERROR_VAL ("index out of range", GSL_EINVAL, zero);
    }
#endif
  return *GSL_COMPLEX_FLOAT_AT (v, i);
}

INLINE_FUN
void
gsl_vector_complex_float_set (gsl_vector_complex_float * v,
                              const size_t i, gsl_complex_float z)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(i >= v->size))
    {
      GSL_ERROR_VOID ("index out of range", GSL_EINVAL);
    }
#endif
  *GSL_COMPLEX_FLOAT_AT (v, i) = z;
}

INLINE_FUN
gsl_complex_float *
gsl_vector_complex_float_ptr (gsl_vector_complex_float * v,
                              const size_t i)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(i >= v->size))
    {
      GSL_ERROR_NULL ("index out of range", GSL_EINVAL);
    }
#endif
  return GSL_COMPLEX_FLOAT_AT (v, i);
}

INLINE_FUN
const gsl_complex_float *
gsl_vector_complex_float_const_ptr (const gsl_vector_complex_float * v,
                                    const size_t i)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(i >= v->size))
    {
      GSL_ERROR_NULL ("index out of range", GSL_EINVAL);
    }
#endif
  return GSL_COMPLEX_FLOAT_AT (v, i);
}


#endif /* HAVE_INLINE */

__END_DECLS

#endif /* __GSL_VECTOR_COMPLEX_FLOAT_H__ */


/* vector/gsl_vector_ulong.h
 * 
 * Copyright (C) 1996, 1997, 1998, 1999, 2000, 2007 Gerard Jungman, Brian Gough
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __GSL_VECTOR_ULONG_H__
#define __GSL_VECTOR_ULONG_H__

#include <stdlib.h>
/* block/gsl_block_ulong.h
 * 
 * Copyright (C) 1996, 1997, 1998, 1999, 2000, 2007 Gerard Jungman, Brian Gough
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __GSL_BLOCK_ULONG_H__
#define __GSL_BLOCK_ULONG_H__

#include <stdlib.h>

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif

__BEGIN_DECLS

struct gsl_block_ulong_struct
{
  size_t size;
  unsigned long *data;
};

typedef struct gsl_block_ulong_struct gsl_block_ulong;

gsl_block_ulong *gsl_block_ulong_alloc (const size_t n);
gsl_block_ulong *gsl_block_ulong_calloc (const size_t n);
void gsl_block_ulong_free (gsl_block_ulong * b);

int gsl_block_ulong_fread (FILE * stream, gsl_block_ulong * b);
int gsl_block_ulong_fwrite (FILE * stream, const gsl_block_ulong * b);
int gsl_block_ulong_fscanf (FILE * stream, gsl_block_ulong * b);
int gsl_block_ulong_fprintf (FILE * stream, const gsl_block_ulong * b, const char *format);

int gsl_block_ulong_raw_fread (FILE * stream, unsigned long * b, const size_t n, const size_t stride);
int gsl_block_ulong_raw_fwrite (FILE * stream, const unsigned long * b, const size_t n, const size_t stride);
int gsl_block_ulong_raw_fscanf (FILE * stream, unsigned long * b, const size_t n, const size_t stride);
int gsl_block_ulong_raw_fprintf (FILE * stream, const unsigned long * b, const size_t n, const size_t stride, const char *format);

size_t gsl_block_ulong_size (const gsl_block_ulong * b);
unsigned long * gsl_block_ulong_data (const gsl_block_ulong * b);

__END_DECLS

#endif /* __GSL_BLOCK_ULONG_H__ */

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif

__BEGIN_DECLS

typedef struct 
{
  size_t size;
  size_t stride;
  unsigned long *data;
  gsl_block_ulong *block;
  int owner;
} 
gsl_vector_ulong;

typedef struct
{
  gsl_vector_ulong vector;
} _gsl_vector_ulong_view;

typedef _gsl_vector_ulong_view gsl_vector_ulong_view;

typedef struct
{
  gsl_vector_ulong vector;
} _gsl_vector_ulong_const_view;

typedef const _gsl_vector_ulong_const_view gsl_vector_ulong_const_view;


/* Allocation */

gsl_vector_ulong *gsl_vector_ulong_alloc (const size_t n);
gsl_vector_ulong *gsl_vector_ulong_calloc (const size_t n);

gsl_vector_ulong *gsl_vector_ulong_alloc_from_block (gsl_block_ulong * b,
                                                     const size_t offset, 
                                                     const size_t n, 
                                                     const size_t stride);

gsl_vector_ulong *gsl_vector_ulong_alloc_from_vector (gsl_vector_ulong * v,
                                                      const size_t offset, 
                                                      const size_t n, 
                                                      const size_t stride);

void gsl_vector_ulong_free (gsl_vector_ulong * v);

/* Views */

_gsl_vector_ulong_view 
gsl_vector_ulong_view_array (unsigned long *v, size_t n);

_gsl_vector_ulong_view 
gsl_vector_ulong_view_array_with_stride (unsigned long *base,
                                         size_t stride,
                                         size_t n);

_gsl_vector_ulong_const_view 
gsl_vector_ulong_const_view_array (const unsigned long *v, size_t n);

_gsl_vector_ulong_const_view 
gsl_vector_ulong_const_view_array_with_stride (const unsigned long *base,
                                               size_t stride,
                                               size_t n);

_gsl_vector_ulong_view 
gsl_vector_ulong_subvector (gsl_vector_ulong *v, 
                            size_t i, 
                            size_t n);

_gsl_vector_ulong_view 
gsl_vector_ulong_subvector_with_stride (gsl_vector_ulong *v, 
                                        size_t i,
                                        size_t stride,
                                        size_t n);

_gsl_vector_ulong_const_view 
gsl_vector_ulong_const_subvector (const gsl_vector_ulong *v, 
                                  size_t i, 
                                  size_t n);

_gsl_vector_ulong_const_view 
gsl_vector_ulong_const_subvector_with_stride (const gsl_vector_ulong *v, 
                                              size_t i, 
                                              size_t stride,
                                              size_t n);

/* Operations */

void gsl_vector_ulong_set_zero (gsl_vector_ulong * v);
void gsl_vector_ulong_set_all (gsl_vector_ulong * v, unsigned long x);
int gsl_vector_ulong_set_basis (gsl_vector_ulong * v, size_t i);

int gsl_vector_ulong_fread (FILE * stream, gsl_vector_ulong * v);
int gsl_vector_ulong_fwrite (FILE * stream, const gsl_vector_ulong * v);
int gsl_vector_ulong_fscanf (FILE * stream, gsl_vector_ulong * v);
int gsl_vector_ulong_fprintf (FILE * stream, const gsl_vector_ulong * v,
                              const char *format);

int gsl_vector_ulong_memcpy (gsl_vector_ulong * dest, const gsl_vector_ulong * src);

int gsl_vector_ulong_reverse (gsl_vector_ulong * v);

int gsl_vector_ulong_swap (gsl_vector_ulong * v, gsl_vector_ulong * w);
int gsl_vector_ulong_swap_elements (gsl_vector_ulong * v, const size_t i, const size_t j);

unsigned long gsl_vector_ulong_max (const gsl_vector_ulong * v);
unsigned long gsl_vector_ulong_min (const gsl_vector_ulong * v);
void gsl_vector_ulong_minmax (const gsl_vector_ulong * v, unsigned long * min_out, unsigned long * max_out);

size_t gsl_vector_ulong_max_index (const gsl_vector_ulong * v);
size_t gsl_vector_ulong_min_index (const gsl_vector_ulong * v);
void gsl_vector_ulong_minmax_index (const gsl_vector_ulong * v, size_t * imin, size_t * imax);

int gsl_vector_ulong_add (gsl_vector_ulong * a, const gsl_vector_ulong * b);
int gsl_vector_ulong_sub (gsl_vector_ulong * a, const gsl_vector_ulong * b);
int gsl_vector_ulong_mul (gsl_vector_ulong * a, const gsl_vector_ulong * b);
int gsl_vector_ulong_div (gsl_vector_ulong * a, const gsl_vector_ulong * b);
int gsl_vector_ulong_scale (gsl_vector_ulong * a, const double x);
int gsl_vector_ulong_add_constant (gsl_vector_ulong * a, const double x);

int gsl_vector_ulong_equal (const gsl_vector_ulong * u, 
                            const gsl_vector_ulong * v);

int gsl_vector_ulong_isnull (const gsl_vector_ulong * v);
int gsl_vector_ulong_ispos (const gsl_vector_ulong * v);
int gsl_vector_ulong_isneg (const gsl_vector_ulong * v);
int gsl_vector_ulong_isnonneg (const gsl_vector_ulong * v);

INLINE_DECL unsigned long gsl_vector_ulong_get (const gsl_vector_ulong * v, const size_t i);
INLINE_DECL void gsl_vector_ulong_set (gsl_vector_ulong * v, const size_t i, unsigned long x);
INLINE_DECL unsigned long * gsl_vector_ulong_ptr (gsl_vector_ulong * v, const size_t i);
INLINE_DECL const unsigned long * gsl_vector_ulong_const_ptr (const gsl_vector_ulong * v, const size_t i);

#ifdef HAVE_INLINE

INLINE_FUN
unsigned long
gsl_vector_ulong_get (const gsl_vector_ulong * v, const size_t i)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(i >= v->size))
    {
      GSL_ERROR_VAL ("index out of range", GSL_EINVAL, 0);
    }
#endif
  return v->data[i * v->stride];
}

INLINE_FUN
void
gsl_vector_ulong_set (gsl_vector_ulong * v, const size_t i, unsigned long x)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(i >= v->size))
    {
      GSL_ERROR_VOID ("index out of range", GSL_EINVAL);
    }
#endif
  v->data[i * v->stride] = x;
}

INLINE_FUN
unsigned long *
gsl_vector_ulong_ptr (gsl_vector_ulong * v, const size_t i)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(i >= v->size))
    {
      GSL_ERROR_NULL ("index out of range", GSL_EINVAL);
    }
#endif
  return (unsigned long *) (v->data + i * v->stride);
}

INLINE_FUN
const unsigned long *
gsl_vector_ulong_const_ptr (const gsl_vector_ulong * v, const size_t i)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(i >= v->size))
    {
      GSL_ERROR_NULL ("index out of range", GSL_EINVAL);
    }
#endif
  return (const unsigned long *) (v->data + i * v->stride);
}
#endif /* HAVE_INLINE */

__END_DECLS

#endif /* __GSL_VECTOR_ULONG_H__ */


/* vector/gsl_vector_long.h
 * 
 * Copyright (C) 1996, 1997, 1998, 1999, 2000, 2007 Gerard Jungman, Brian Gough
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __GSL_VECTOR_LONG_H__
#define __GSL_VECTOR_LONG_H__

#include <stdlib.h>
/* block/gsl_block_long.h
 * 
 * Copyright (C) 1996, 1997, 1998, 1999, 2000, 2007 Gerard Jungman, Brian Gough
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __GSL_BLOCK_LONG_H__
#define __GSL_BLOCK_LONG_H__

#include <stdlib.h>

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif

__BEGIN_DECLS

struct gsl_block_long_struct
{
  size_t size;
  long *data;
};

typedef struct gsl_block_long_struct gsl_block_long;

gsl_block_long *gsl_block_long_alloc (const size_t n);
gsl_block_long *gsl_block_long_calloc (const size_t n);
void gsl_block_long_free (gsl_block_long * b);

int gsl_block_long_fread (FILE * stream, gsl_block_long * b);
int gsl_block_long_fwrite (FILE * stream, const gsl_block_long * b);
int gsl_block_long_fscanf (FILE * stream, gsl_block_long * b);
int gsl_block_long_fprintf (FILE * stream, const gsl_block_long * b, const char *format);

int gsl_block_long_raw_fread (FILE * stream, long * b, const size_t n, const size_t stride);
int gsl_block_long_raw_fwrite (FILE * stream, const long * b, const size_t n, const size_t stride);
int gsl_block_long_raw_fscanf (FILE * stream, long * b, const size_t n, const size_t stride);
int gsl_block_long_raw_fprintf (FILE * stream, const long * b, const size_t n, const size_t stride, const char *format);

size_t gsl_block_long_size (const gsl_block_long * b);
long * gsl_block_long_data (const gsl_block_long * b);

__END_DECLS

#endif /* __GSL_BLOCK_LONG_H__ */

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif

__BEGIN_DECLS

typedef struct 
{
  size_t size;
  size_t stride;
  long *data;
  gsl_block_long *block;
  int owner;
} 
gsl_vector_long;

typedef struct
{
  gsl_vector_long vector;
} _gsl_vector_long_view;

typedef _gsl_vector_long_view gsl_vector_long_view;

typedef struct
{
  gsl_vector_long vector;
} _gsl_vector_long_const_view;

typedef const _gsl_vector_long_const_view gsl_vector_long_const_view;


/* Allocation */

gsl_vector_long *gsl_vector_long_alloc (const size_t n);
gsl_vector_long *gsl_vector_long_calloc (const size_t n);

gsl_vector_long *gsl_vector_long_alloc_from_block (gsl_block_long * b,
                                                     const size_t offset, 
                                                     const size_t n, 
                                                     const size_t stride);

gsl_vector_long *gsl_vector_long_alloc_from_vector (gsl_vector_long * v,
                                                      const size_t offset, 
                                                      const size_t n, 
                                                      const size_t stride);

void gsl_vector_long_free (gsl_vector_long * v);

/* Views */

_gsl_vector_long_view 
gsl_vector_long_view_array (long *v, size_t n);

_gsl_vector_long_view 
gsl_vector_long_view_array_with_stride (long *base,
                                         size_t stride,
                                         size_t n);

_gsl_vector_long_const_view 
gsl_vector_long_const_view_array (const long *v, size_t n);

_gsl_vector_long_const_view 
gsl_vector_long_const_view_array_with_stride (const long *base,
                                               size_t stride,
                                               size_t n);

_gsl_vector_long_view 
gsl_vector_long_subvector (gsl_vector_long *v, 
                            size_t i, 
                            size_t n);

_gsl_vector_long_view 
gsl_vector_long_subvector_with_stride (gsl_vector_long *v, 
                                        size_t i,
                                        size_t stride,
                                        size_t n);

_gsl_vector_long_const_view 
gsl_vector_long_const_subvector (const gsl_vector_long *v, 
                                  size_t i, 
                                  size_t n);

_gsl_vector_long_const_view 
gsl_vector_long_const_subvector_with_stride (const gsl_vector_long *v, 
                                              size_t i, 
                                              size_t stride,
                                              size_t n);

/* Operations */

void gsl_vector_long_set_zero (gsl_vector_long * v);
void gsl_vector_long_set_all (gsl_vector_long * v, long x);
int gsl_vector_long_set_basis (gsl_vector_long * v, size_t i);

int gsl_vector_long_fread (FILE * stream, gsl_vector_long * v);
int gsl_vector_long_fwrite (FILE * stream, const gsl_vector_long * v);
int gsl_vector_long_fscanf (FILE * stream, gsl_vector_long * v);
int gsl_vector_long_fprintf (FILE * stream, const gsl_vector_long * v,
                              const char *format);

int gsl_vector_long_memcpy (gsl_vector_long * dest, const gsl_vector_long * src);

int gsl_vector_long_reverse (gsl_vector_long * v);

int gsl_vector_long_swap (gsl_vector_long * v, gsl_vector_long * w);
int gsl_vector_long_swap_elements (gsl_vector_long * v, const size_t i, const size_t j);

long gsl_vector_long_max (const gsl_vector_long * v);
long gsl_vector_long_min (const gsl_vector_long * v);
void gsl_vector_long_minmax (const gsl_vector_long * v, long * min_out, long * max_out);

size_t gsl_vector_long_max_index (const gsl_vector_long * v);
size_t gsl_vector_long_min_index (const gsl_vector_long * v);
void gsl_vector_long_minmax_index (const gsl_vector_long * v, size_t * imin, size_t * imax);

int gsl_vector_long_add (gsl_vector_long * a, const gsl_vector_long * b);
int gsl_vector_long_sub (gsl_vector_long * a, const gsl_vector_long * b);
int gsl_vector_long_mul (gsl_vector_long * a, const gsl_vector_long * b);
int gsl_vector_long_div (gsl_vector_long * a, const gsl_vector_long * b);
int gsl_vector_long_scale (gsl_vector_long * a, const double x);
int gsl_vector_long_add_constant (gsl_vector_long * a, const double x);

int gsl_vector_long_equal (const gsl_vector_long * u, 
                            const gsl_vector_long * v);

int gsl_vector_long_isnull (const gsl_vector_long * v);
int gsl_vector_long_ispos (const gsl_vector_long * v);
int gsl_vector_long_isneg (const gsl_vector_long * v);
int gsl_vector_long_isnonneg (const gsl_vector_long * v);

INLINE_DECL long gsl_vector_long_get (const gsl_vector_long * v, const size_t i);
INLINE_DECL void gsl_vector_long_set (gsl_vector_long * v, const size_t i, long x);
INLINE_DECL long * gsl_vector_long_ptr (gsl_vector_long * v, const size_t i);
INLINE_DECL const long * gsl_vector_long_const_ptr (const gsl_vector_long * v, const size_t i);

#ifdef HAVE_INLINE

INLINE_FUN
long
gsl_vector_long_get (const gsl_vector_long * v, const size_t i)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(i >= v->size))
    {
      GSL_ERROR_VAL ("index out of range", GSL_EINVAL, 0);
    }
#endif
  return v->data[i * v->stride];
}

INLINE_FUN
void
gsl_vector_long_set (gsl_vector_long * v, const size_t i, long x)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(i >= v->size))
    {
      GSL_ERROR_VOID ("index out of range", GSL_EINVAL);
    }
#endif
  v->data[i * v->stride] = x;
}

INLINE_FUN
long *
gsl_vector_long_ptr (gsl_vector_long * v, const size_t i)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(i >= v->size))
    {
      GSL_ERROR_NULL ("index out of range", GSL_EINVAL);
    }
#endif
  return (long *) (v->data + i * v->stride);
}

INLINE_FUN
const long *
gsl_vector_long_const_ptr (const gsl_vector_long * v, const size_t i)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(i >= v->size))
    {
      GSL_ERROR_NULL ("index out of range", GSL_EINVAL);
    }
#endif
  return (const long *) (v->data + i * v->stride);
}
#endif /* HAVE_INLINE */

__END_DECLS

#endif /* __GSL_VECTOR_LONG_H__ */



/* vector/gsl_vector_uint.h
 * 
 * Copyright (C) 1996, 1997, 1998, 1999, 2000, 2007 Gerard Jungman, Brian Gough
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __GSL_VECTOR_UINT_H__
#define __GSL_VECTOR_UINT_H__

#include <stdlib.h>
/* block/gsl_block_uint.h
 * 
 * Copyright (C) 1996, 1997, 1998, 1999, 2000, 2007 Gerard Jungman, Brian Gough
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __GSL_BLOCK_UINT_H__
#define __GSL_BLOCK_UINT_H__

#include <stdlib.h>

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif

__BEGIN_DECLS

struct gsl_block_uint_struct
{
  size_t size;
  unsigned int *data;
};

typedef struct gsl_block_uint_struct gsl_block_uint;

gsl_block_uint *gsl_block_uint_alloc (const size_t n);
gsl_block_uint *gsl_block_uint_calloc (const size_t n);
void gsl_block_uint_free (gsl_block_uint * b);

int gsl_block_uint_fread (FILE * stream, gsl_block_uint * b);
int gsl_block_uint_fwrite (FILE * stream, const gsl_block_uint * b);
int gsl_block_uint_fscanf (FILE * stream, gsl_block_uint * b);
int gsl_block_uint_fprintf (FILE * stream, const gsl_block_uint * b, const char *format);

int gsl_block_uint_raw_fread (FILE * stream, unsigned int * b, const size_t n, const size_t stride);
int gsl_block_uint_raw_fwrite (FILE * stream, const unsigned int * b, const size_t n, const size_t stride);
int gsl_block_uint_raw_fscanf (FILE * stream, unsigned int * b, const size_t n, const size_t stride);
int gsl_block_uint_raw_fprintf (FILE * stream, const unsigned int * b, const size_t n, const size_t stride, const char *format);

size_t gsl_block_uint_size (const gsl_block_uint * b);
unsigned int * gsl_block_uint_data (const gsl_block_uint * b);

__END_DECLS

#endif /* __GSL_BLOCK_UINT_H__ */

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif

__BEGIN_DECLS

typedef struct 
{
  size_t size;
  size_t stride;
  unsigned int *data;
  gsl_block_uint *block;
  int owner;
} 
gsl_vector_uint;

typedef struct
{
  gsl_vector_uint vector;
} _gsl_vector_uint_view;

typedef _gsl_vector_uint_view gsl_vector_uint_view;

typedef struct
{
  gsl_vector_uint vector;
} _gsl_vector_uint_const_view;

typedef const _gsl_vector_uint_const_view gsl_vector_uint_const_view;


/* Allocation */

gsl_vector_uint *gsl_vector_uint_alloc (const size_t n);
gsl_vector_uint *gsl_vector_uint_calloc (const size_t n);

gsl_vector_uint *gsl_vector_uint_alloc_from_block (gsl_block_uint * b,
                                                     const size_t offset, 
                                                     const size_t n, 
                                                     const size_t stride);

gsl_vector_uint *gsl_vector_uint_alloc_from_vector (gsl_vector_uint * v,
                                                      const size_t offset, 
                                                      const size_t n, 
                                                      const size_t stride);

void gsl_vector_uint_free (gsl_vector_uint * v);

/* Views */

_gsl_vector_uint_view 
gsl_vector_uint_view_array (unsigned int *v, size_t n);

_gsl_vector_uint_view 
gsl_vector_uint_view_array_with_stride (unsigned int *base,
                                         size_t stride,
                                         size_t n);

_gsl_vector_uint_const_view 
gsl_vector_uint_const_view_array (const unsigned int *v, size_t n);

_gsl_vector_uint_const_view 
gsl_vector_uint_const_view_array_with_stride (const unsigned int *base,
                                               size_t stride,
                                               size_t n);

_gsl_vector_uint_view 
gsl_vector_uint_subvector (gsl_vector_uint *v, 
                            size_t i, 
                            size_t n);

_gsl_vector_uint_view 
gsl_vector_uint_subvector_with_stride (gsl_vector_uint *v, 
                                        size_t i,
                                        size_t stride,
                                        size_t n);

_gsl_vector_uint_const_view 
gsl_vector_uint_const_subvector (const gsl_vector_uint *v, 
                                  size_t i, 
                                  size_t n);

_gsl_vector_uint_const_view 
gsl_vector_uint_const_subvector_with_stride (const gsl_vector_uint *v, 
                                              size_t i, 
                                              size_t stride,
                                              size_t n);

/* Operations */

void gsl_vector_uint_set_zero (gsl_vector_uint * v);
void gsl_vector_uint_set_all (gsl_vector_uint * v, unsigned int x);
int gsl_vector_uint_set_basis (gsl_vector_uint * v, size_t i);

int gsl_vector_uint_fread (FILE * stream, gsl_vector_uint * v);
int gsl_vector_uint_fwrite (FILE * stream, const gsl_vector_uint * v);
int gsl_vector_uint_fscanf (FILE * stream, gsl_vector_uint * v);
int gsl_vector_uint_fprintf (FILE * stream, const gsl_vector_uint * v,
                              const char *format);

int gsl_vector_uint_memcpy (gsl_vector_uint * dest, const gsl_vector_uint * src);

int gsl_vector_uint_reverse (gsl_vector_uint * v);

int gsl_vector_uint_swap (gsl_vector_uint * v, gsl_vector_uint * w);
int gsl_vector_uint_swap_elements (gsl_vector_uint * v, const size_t i, const size_t j);

unsigned int gsl_vector_uint_max (const gsl_vector_uint * v);
unsigned int gsl_vector_uint_min (const gsl_vector_uint * v);
void gsl_vector_uint_minmax (const gsl_vector_uint * v, unsigned int * min_out, unsigned int * max_out);

size_t gsl_vector_uint_max_index (const gsl_vector_uint * v);
size_t gsl_vector_uint_min_index (const gsl_vector_uint * v);
void gsl_vector_uint_minmax_index (const gsl_vector_uint * v, size_t * imin, size_t * imax);

int gsl_vector_uint_add (gsl_vector_uint * a, const gsl_vector_uint * b);
int gsl_vector_uint_sub (gsl_vector_uint * a, const gsl_vector_uint * b);
int gsl_vector_uint_mul (gsl_vector_uint * a, const gsl_vector_uint * b);
int gsl_vector_uint_div (gsl_vector_uint * a, const gsl_vector_uint * b);
int gsl_vector_uint_scale (gsl_vector_uint * a, const double x);
int gsl_vector_uint_add_constant (gsl_vector_uint * a, const double x);

int gsl_vector_uint_equal (const gsl_vector_uint * u, 
                            const gsl_vector_uint * v);

int gsl_vector_uint_isnull (const gsl_vector_uint * v);
int gsl_vector_uint_ispos (const gsl_vector_uint * v);
int gsl_vector_uint_isneg (const gsl_vector_uint * v);
int gsl_vector_uint_isnonneg (const gsl_vector_uint * v);

INLINE_DECL unsigned int gsl_vector_uint_get (const gsl_vector_uint * v, const size_t i);
INLINE_DECL void gsl_vector_uint_set (gsl_vector_uint * v, const size_t i, unsigned int x);
INLINE_DECL unsigned int * gsl_vector_uint_ptr (gsl_vector_uint * v, const size_t i);
INLINE_DECL const unsigned int * gsl_vector_uint_const_ptr (const gsl_vector_uint * v, const size_t i);

#ifdef HAVE_INLINE

INLINE_FUN
unsigned int
gsl_vector_uint_get (const gsl_vector_uint * v, const size_t i)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(i >= v->size))
    {
      GSL_ERROR_VAL ("index out of range", GSL_EINVAL, 0);
    }
#endif
  return v->data[i * v->stride];
}

INLINE_FUN
void
gsl_vector_uint_set (gsl_vector_uint * v, const size_t i, unsigned int x)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(i >= v->size))
    {
      GSL_ERROR_VOID ("index out of range", GSL_EINVAL);
    }
#endif
  v->data[i * v->stride] = x;
}

INLINE_FUN
unsigned int *
gsl_vector_uint_ptr (gsl_vector_uint * v, const size_t i)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(i >= v->size))
    {
      GSL_ERROR_NULL ("index out of range", GSL_EINVAL);
    }
#endif
  return (unsigned int *) (v->data + i * v->stride);
}

INLINE_FUN
const unsigned int *
gsl_vector_uint_const_ptr (const gsl_vector_uint * v, const size_t i)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(i >= v->size))
    {
      GSL_ERROR_NULL ("index out of range", GSL_EINVAL);
    }
#endif
  return (const unsigned int *) (v->data + i * v->stride);
}
#endif /* HAVE_INLINE */

__END_DECLS

#endif /* __GSL_VECTOR_UINT_H__ */


/* vector/gsl_vector_int.h
 * 
 * Copyright (C) 1996, 1997, 1998, 1999, 2000, 2007 Gerard Jungman, Brian Gough
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __GSL_VECTOR_INT_H__
#define __GSL_VECTOR_INT_H__

#include <stdlib.h>
/* block/gsl_block_int.h
 * 
 * Copyright (C) 1996, 1997, 1998, 1999, 2000, 2007 Gerard Jungman, Brian Gough
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __GSL_BLOCK_INT_H__
#define __GSL_BLOCK_INT_H__

#include <stdlib.h>

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif

__BEGIN_DECLS

struct gsl_block_int_struct
{
  size_t size;
  int *data;
};

typedef struct gsl_block_int_struct gsl_block_int;

gsl_block_int *gsl_block_int_alloc (const size_t n);
gsl_block_int *gsl_block_int_calloc (const size_t n);
void gsl_block_int_free (gsl_block_int * b);

int gsl_block_int_fread (FILE * stream, gsl_block_int * b);
int gsl_block_int_fwrite (FILE * stream, const gsl_block_int * b);
int gsl_block_int_fscanf (FILE * stream, gsl_block_int * b);
int gsl_block_int_fprintf (FILE * stream, const gsl_block_int * b, const char *format);

int gsl_block_int_raw_fread (FILE * stream, int * b, const size_t n, const size_t stride);
int gsl_block_int_raw_fwrite (FILE * stream, const int * b, const size_t n, const size_t stride);
int gsl_block_int_raw_fscanf (FILE * stream, int * b, const size_t n, const size_t stride);
int gsl_block_int_raw_fprintf (FILE * stream, const int * b, const size_t n, const size_t stride, const char *format);

size_t gsl_block_int_size (const gsl_block_int * b);
int * gsl_block_int_data (const gsl_block_int * b);

__END_DECLS

#endif /* __GSL_BLOCK_INT_H__ */

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif

__BEGIN_DECLS

typedef struct 
{
  size_t size;
  size_t stride;
  int *data;
  gsl_block_int *block;
  int owner;
} 
gsl_vector_int;

typedef struct
{
  gsl_vector_int vector;
} _gsl_vector_int_view;

typedef _gsl_vector_int_view gsl_vector_int_view;

typedef struct
{
  gsl_vector_int vector;
} _gsl_vector_int_const_view;

typedef const _gsl_vector_int_const_view gsl_vector_int_const_view;


/* Allocation */

gsl_vector_int *gsl_vector_int_alloc (const size_t n);
gsl_vector_int *gsl_vector_int_calloc (const size_t n);

gsl_vector_int *gsl_vector_int_alloc_from_block (gsl_block_int * b,
                                                     const size_t offset, 
                                                     const size_t n, 
                                                     const size_t stride);

gsl_vector_int *gsl_vector_int_alloc_from_vector (gsl_vector_int * v,
                                                      const size_t offset, 
                                                      const size_t n, 
                                                      const size_t stride);

void gsl_vector_int_free (gsl_vector_int * v);

/* Views */

_gsl_vector_int_view 
gsl_vector_int_view_array (int *v, size_t n);

_gsl_vector_int_view 
gsl_vector_int_view_array_with_stride (int *base,
                                         size_t stride,
                                         size_t n);

_gsl_vector_int_const_view 
gsl_vector_int_const_view_array (const int *v, size_t n);

_gsl_vector_int_const_view 
gsl_vector_int_const_view_array_with_stride (const int *base,
                                               size_t stride,
                                               size_t n);

_gsl_vector_int_view 
gsl_vector_int_subvector (gsl_vector_int *v, 
                            size_t i, 
                            size_t n);

_gsl_vector_int_view 
gsl_vector_int_subvector_with_stride (gsl_vector_int *v, 
                                        size_t i,
                                        size_t stride,
                                        size_t n);

_gsl_vector_int_const_view 
gsl_vector_int_const_subvector (const gsl_vector_int *v, 
                                  size_t i, 
                                  size_t n);

_gsl_vector_int_const_view 
gsl_vector_int_const_subvector_with_stride (const gsl_vector_int *v, 
                                              size_t i, 
                                              size_t stride,
                                              size_t n);

/* Operations */

void gsl_vector_int_set_zero (gsl_vector_int * v);
void gsl_vector_int_set_all (gsl_vector_int * v, int x);
int gsl_vector_int_set_basis (gsl_vector_int * v, size_t i);

int gsl_vector_int_fread (FILE * stream, gsl_vector_int * v);
int gsl_vector_int_fwrite (FILE * stream, const gsl_vector_int * v);
int gsl_vector_int_fscanf (FILE * stream, gsl_vector_int * v);
int gsl_vector_int_fprintf (FILE * stream, const gsl_vector_int * v,
                              const char *format);

int gsl_vector_int_memcpy (gsl_vector_int * dest, const gsl_vector_int * src);

int gsl_vector_int_reverse (gsl_vector_int * v);

int gsl_vector_int_swap (gsl_vector_int * v, gsl_vector_int * w);
int gsl_vector_int_swap_elements (gsl_vector_int * v, const size_t i, const size_t j);

int gsl_vector_int_max (const gsl_vector_int * v);
int gsl_vector_int_min (const gsl_vector_int * v);
void gsl_vector_int_minmax (const gsl_vector_int * v, int * min_out, int * max_out);

size_t gsl_vector_int_max_index (const gsl_vector_int * v);
size_t gsl_vector_int_min_index (const gsl_vector_int * v);
void gsl_vector_int_minmax_index (const gsl_vector_int * v, size_t * imin, size_t * imax);

int gsl_vector_int_add (gsl_vector_int * a, const gsl_vector_int * b);
int gsl_vector_int_sub (gsl_vector_int * a, const gsl_vector_int * b);
int gsl_vector_int_mul (gsl_vector_int * a, const gsl_vector_int * b);
int gsl_vector_int_div (gsl_vector_int * a, const gsl_vector_int * b);
int gsl_vector_int_scale (gsl_vector_int * a, const double x);
int gsl_vector_int_add_constant (gsl_vector_int * a, const double x);

int gsl_vector_int_equal (const gsl_vector_int * u, 
                            const gsl_vector_int * v);

int gsl_vector_int_isnull (const gsl_vector_int * v);
int gsl_vector_int_ispos (const gsl_vector_int * v);
int gsl_vector_int_isneg (const gsl_vector_int * v);
int gsl_vector_int_isnonneg (const gsl_vector_int * v);

INLINE_DECL int gsl_vector_int_get (const gsl_vector_int * v, const size_t i);
INLINE_DECL void gsl_vector_int_set (gsl_vector_int * v, const size_t i, int x);
INLINE_DECL int * gsl_vector_int_ptr (gsl_vector_int * v, const size_t i);
INLINE_DECL const int * gsl_vector_int_const_ptr (const gsl_vector_int * v, const size_t i);

#ifdef HAVE_INLINE

INLINE_FUN
int
gsl_vector_int_get (const gsl_vector_int * v, const size_t i)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(i >= v->size))
    {
      GSL_ERROR_VAL ("index out of range", GSL_EINVAL, 0);
    }
#endif
  return v->data[i * v->stride];
}

INLINE_FUN
void
gsl_vector_int_set (gsl_vector_int * v, const size_t i, int x)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(i >= v->size))
    {
      GSL_ERROR_VOID ("index out of range", GSL_EINVAL);
    }
#endif
  v->data[i * v->stride] = x;
}

INLINE_FUN
int *
gsl_vector_int_ptr (gsl_vector_int * v, const size_t i)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(i >= v->size))
    {
      GSL_ERROR_NULL ("index out of range", GSL_EINVAL);
    }
#endif
  return (int *) (v->data + i * v->stride);
}

INLINE_FUN
const int *
gsl_vector_int_const_ptr (const gsl_vector_int * v, const size_t i)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(i >= v->size))
    {
      GSL_ERROR_NULL ("index out of range", GSL_EINVAL);
    }
#endif
  return (const int *) (v->data + i * v->stride);
}
#endif /* HAVE_INLINE */

__END_DECLS

#endif /* __GSL_VECTOR_INT_H__ */



/* vector/gsl_vector_ushort.h
 * 
 * Copyright (C) 1996, 1997, 1998, 1999, 2000, 2007 Gerard Jungman, Brian Gough
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __GSL_VECTOR_USHORT_H__
#define __GSL_VECTOR_USHORT_H__

#include <stdlib.h>
/* block/gsl_block_ushort.h
 * 
 * Copyright (C) 1996, 1997, 1998, 1999, 2000, 2007 Gerard Jungman, Brian Gough
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __GSL_BLOCK_USHORT_H__
#define __GSL_BLOCK_USHORT_H__

#include <stdlib.h>

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif

__BEGIN_DECLS

struct gsl_block_ushort_struct
{
  size_t size;
  unsigned short *data;
};

typedef struct gsl_block_ushort_struct gsl_block_ushort;

gsl_block_ushort *gsl_block_ushort_alloc (const size_t n);
gsl_block_ushort *gsl_block_ushort_calloc (const size_t n);
void gsl_block_ushort_free (gsl_block_ushort * b);

int gsl_block_ushort_fread (FILE * stream, gsl_block_ushort * b);
int gsl_block_ushort_fwrite (FILE * stream, const gsl_block_ushort * b);
int gsl_block_ushort_fscanf (FILE * stream, gsl_block_ushort * b);
int gsl_block_ushort_fprintf (FILE * stream, const gsl_block_ushort * b, const char *format);

int gsl_block_ushort_raw_fread (FILE * stream, unsigned short * b, const size_t n, const size_t stride);
int gsl_block_ushort_raw_fwrite (FILE * stream, const unsigned short * b, const size_t n, const size_t stride);
int gsl_block_ushort_raw_fscanf (FILE * stream, unsigned short * b, const size_t n, const size_t stride);
int gsl_block_ushort_raw_fprintf (FILE * stream, const unsigned short * b, const size_t n, const size_t stride, const char *format);

size_t gsl_block_ushort_size (const gsl_block_ushort * b);
unsigned short * gsl_block_ushort_data (const gsl_block_ushort * b);

__END_DECLS

#endif /* __GSL_BLOCK_USHORT_H__ */

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif

__BEGIN_DECLS

typedef struct 
{
  size_t size;
  size_t stride;
  unsigned short *data;
  gsl_block_ushort *block;
  int owner;
} 
gsl_vector_ushort;

typedef struct
{
  gsl_vector_ushort vector;
} _gsl_vector_ushort_view;

typedef _gsl_vector_ushort_view gsl_vector_ushort_view;

typedef struct
{
  gsl_vector_ushort vector;
} _gsl_vector_ushort_const_view;

typedef const _gsl_vector_ushort_const_view gsl_vector_ushort_const_view;


/* Allocation */

gsl_vector_ushort *gsl_vector_ushort_alloc (const size_t n);
gsl_vector_ushort *gsl_vector_ushort_calloc (const size_t n);

gsl_vector_ushort *gsl_vector_ushort_alloc_from_block (gsl_block_ushort * b,
                                                     const size_t offset, 
                                                     const size_t n, 
                                                     const size_t stride);

gsl_vector_ushort *gsl_vector_ushort_alloc_from_vector (gsl_vector_ushort * v,
                                                      const size_t offset, 
                                                      const size_t n, 
                                                      const size_t stride);

void gsl_vector_ushort_free (gsl_vector_ushort * v);

/* Views */

_gsl_vector_ushort_view 
gsl_vector_ushort_view_array (unsigned short *v, size_t n);

_gsl_vector_ushort_view 
gsl_vector_ushort_view_array_with_stride (unsigned short *base,
                                         size_t stride,
                                         size_t n);

_gsl_vector_ushort_const_view 
gsl_vector_ushort_const_view_array (const unsigned short *v, size_t n);

_gsl_vector_ushort_const_view 
gsl_vector_ushort_const_view_array_with_stride (const unsigned short *base,
                                               size_t stride,
                                               size_t n);

_gsl_vector_ushort_view 
gsl_vector_ushort_subvector (gsl_vector_ushort *v, 
                            size_t i, 
                            size_t n);

_gsl_vector_ushort_view 
gsl_vector_ushort_subvector_with_stride (gsl_vector_ushort *v, 
                                        size_t i,
                                        size_t stride,
                                        size_t n);

_gsl_vector_ushort_const_view 
gsl_vector_ushort_const_subvector (const gsl_vector_ushort *v, 
                                  size_t i, 
                                  size_t n);

_gsl_vector_ushort_const_view 
gsl_vector_ushort_const_subvector_with_stride (const gsl_vector_ushort *v, 
                                              size_t i, 
                                              size_t stride,
                                              size_t n);

/* Operations */

void gsl_vector_ushort_set_zero (gsl_vector_ushort * v);
void gsl_vector_ushort_set_all (gsl_vector_ushort * v, unsigned short x);
int gsl_vector_ushort_set_basis (gsl_vector_ushort * v, size_t i);

int gsl_vector_ushort_fread (FILE * stream, gsl_vector_ushort * v);
int gsl_vector_ushort_fwrite (FILE * stream, const gsl_vector_ushort * v);
int gsl_vector_ushort_fscanf (FILE * stream, gsl_vector_ushort * v);
int gsl_vector_ushort_fprintf (FILE * stream, const gsl_vector_ushort * v,
                              const char *format);

int gsl_vector_ushort_memcpy (gsl_vector_ushort * dest, const gsl_vector_ushort * src);

int gsl_vector_ushort_reverse (gsl_vector_ushort * v);

int gsl_vector_ushort_swap (gsl_vector_ushort * v, gsl_vector_ushort * w);
int gsl_vector_ushort_swap_elements (gsl_vector_ushort * v, const size_t i, const size_t j);

unsigned short gsl_vector_ushort_max (const gsl_vector_ushort * v);
unsigned short gsl_vector_ushort_min (const gsl_vector_ushort * v);
void gsl_vector_ushort_minmax (const gsl_vector_ushort * v, unsigned short * min_out, unsigned short * max_out);

size_t gsl_vector_ushort_max_index (const gsl_vector_ushort * v);
size_t gsl_vector_ushort_min_index (const gsl_vector_ushort * v);
void gsl_vector_ushort_minmax_index (const gsl_vector_ushort * v, size_t * imin, size_t * imax);

int gsl_vector_ushort_add (gsl_vector_ushort * a, const gsl_vector_ushort * b);
int gsl_vector_ushort_sub (gsl_vector_ushort * a, const gsl_vector_ushort * b);
int gsl_vector_ushort_mul (gsl_vector_ushort * a, const gsl_vector_ushort * b);
int gsl_vector_ushort_div (gsl_vector_ushort * a, const gsl_vector_ushort * b);
int gsl_vector_ushort_scale (gsl_vector_ushort * a, const double x);
int gsl_vector_ushort_add_constant (gsl_vector_ushort * a, const double x);

int gsl_vector_ushort_equal (const gsl_vector_ushort * u, 
                            const gsl_vector_ushort * v);

int gsl_vector_ushort_isnull (const gsl_vector_ushort * v);
int gsl_vector_ushort_ispos (const gsl_vector_ushort * v);
int gsl_vector_ushort_isneg (const gsl_vector_ushort * v);
int gsl_vector_ushort_isnonneg (const gsl_vector_ushort * v);

INLINE_DECL unsigned short gsl_vector_ushort_get (const gsl_vector_ushort * v, const size_t i);
INLINE_DECL void gsl_vector_ushort_set (gsl_vector_ushort * v, const size_t i, unsigned short x);
INLINE_DECL unsigned short * gsl_vector_ushort_ptr (gsl_vector_ushort * v, const size_t i);
INLINE_DECL const unsigned short * gsl_vector_ushort_const_ptr (const gsl_vector_ushort * v, const size_t i);

#ifdef HAVE_INLINE

INLINE_FUN
unsigned short
gsl_vector_ushort_get (const gsl_vector_ushort * v, const size_t i)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(i >= v->size))
    {
      GSL_ERROR_VAL ("index out of range", GSL_EINVAL, 0);
    }
#endif
  return v->data[i * v->stride];
}

INLINE_FUN
void
gsl_vector_ushort_set (gsl_vector_ushort * v, const size_t i, unsigned short x)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(i >= v->size))
    {
      GSL_ERROR_VOID ("index out of range", GSL_EINVAL);
    }
#endif
  v->data[i * v->stride] = x;
}

INLINE_FUN
unsigned short *
gsl_vector_ushort_ptr (gsl_vector_ushort * v, const size_t i)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(i >= v->size))
    {
      GSL_ERROR_NULL ("index out of range", GSL_EINVAL);
    }
#endif
  return (unsigned short *) (v->data + i * v->stride);
}

INLINE_FUN
const unsigned short *
gsl_vector_ushort_const_ptr (const gsl_vector_ushort * v, const size_t i)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(i >= v->size))
    {
      GSL_ERROR_NULL ("index out of range", GSL_EINVAL);
    }
#endif
  return (const unsigned short *) (v->data + i * v->stride);
}
#endif /* HAVE_INLINE */

__END_DECLS

#endif /* __GSL_VECTOR_USHORT_H__ */


/* vector/gsl_vector_short.h
 * 
 * Copyright (C) 1996, 1997, 1998, 1999, 2000, 2007 Gerard Jungman, Brian Gough
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __GSL_VECTOR_SHORT_H__
#define __GSL_VECTOR_SHORT_H__

#include <stdlib.h>
/* block/gsl_block_short.h
 * 
 * Copyright (C) 1996, 1997, 1998, 1999, 2000, 2007 Gerard Jungman, Brian Gough
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __GSL_BLOCK_SHORT_H__
#define __GSL_BLOCK_SHORT_H__

#include <stdlib.h>

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif

__BEGIN_DECLS

struct gsl_block_short_struct
{
  size_t size;
  short *data;
};

typedef struct gsl_block_short_struct gsl_block_short;

gsl_block_short *gsl_block_short_alloc (const size_t n);
gsl_block_short *gsl_block_short_calloc (const size_t n);
void gsl_block_short_free (gsl_block_short * b);

int gsl_block_short_fread (FILE * stream, gsl_block_short * b);
int gsl_block_short_fwrite (FILE * stream, const gsl_block_short * b);
int gsl_block_short_fscanf (FILE * stream, gsl_block_short * b);
int gsl_block_short_fprintf (FILE * stream, const gsl_block_short * b, const char *format);

int gsl_block_short_raw_fread (FILE * stream, short * b, const size_t n, const size_t stride);
int gsl_block_short_raw_fwrite (FILE * stream, const short * b, const size_t n, const size_t stride);
int gsl_block_short_raw_fscanf (FILE * stream, short * b, const size_t n, const size_t stride);
int gsl_block_short_raw_fprintf (FILE * stream, const short * b, const size_t n, const size_t stride, const char *format);

size_t gsl_block_short_size (const gsl_block_short * b);
short * gsl_block_short_data (const gsl_block_short * b);

__END_DECLS

#endif /* __GSL_BLOCK_SHORT_H__ */

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif

__BEGIN_DECLS

typedef struct 
{
  size_t size;
  size_t stride;
  short *data;
  gsl_block_short *block;
  int owner;
} 
gsl_vector_short;

typedef struct
{
  gsl_vector_short vector;
} _gsl_vector_short_view;

typedef _gsl_vector_short_view gsl_vector_short_view;

typedef struct
{
  gsl_vector_short vector;
} _gsl_vector_short_const_view;

typedef const _gsl_vector_short_const_view gsl_vector_short_const_view;


/* Allocation */

gsl_vector_short *gsl_vector_short_alloc (const size_t n);
gsl_vector_short *gsl_vector_short_calloc (const size_t n);

gsl_vector_short *gsl_vector_short_alloc_from_block (gsl_block_short * b,
                                                     const size_t offset, 
                                                     const size_t n, 
                                                     const size_t stride);

gsl_vector_short *gsl_vector_short_alloc_from_vector (gsl_vector_short * v,
                                                      const size_t offset, 
                                                      const size_t n, 
                                                      const size_t stride);

void gsl_vector_short_free (gsl_vector_short * v);

/* Views */

_gsl_vector_short_view 
gsl_vector_short_view_array (short *v, size_t n);

_gsl_vector_short_view 
gsl_vector_short_view_array_with_stride (short *base,
                                         size_t stride,
                                         size_t n);

_gsl_vector_short_const_view 
gsl_vector_short_const_view_array (const short *v, size_t n);

_gsl_vector_short_const_view 
gsl_vector_short_const_view_array_with_stride (const short *base,
                                               size_t stride,
                                               size_t n);

_gsl_vector_short_view 
gsl_vector_short_subvector (gsl_vector_short *v, 
                            size_t i, 
                            size_t n);

_gsl_vector_short_view 
gsl_vector_short_subvector_with_stride (gsl_vector_short *v, 
                                        size_t i,
                                        size_t stride,
                                        size_t n);

_gsl_vector_short_const_view 
gsl_vector_short_const_subvector (const gsl_vector_short *v, 
                                  size_t i, 
                                  size_t n);

_gsl_vector_short_const_view 
gsl_vector_short_const_subvector_with_stride (const gsl_vector_short *v, 
                                              size_t i, 
                                              size_t stride,
                                              size_t n);

/* Operations */

void gsl_vector_short_set_zero (gsl_vector_short * v);
void gsl_vector_short_set_all (gsl_vector_short * v, short x);
int gsl_vector_short_set_basis (gsl_vector_short * v, size_t i);

int gsl_vector_short_fread (FILE * stream, gsl_vector_short * v);
int gsl_vector_short_fwrite (FILE * stream, const gsl_vector_short * v);
int gsl_vector_short_fscanf (FILE * stream, gsl_vector_short * v);
int gsl_vector_short_fprintf (FILE * stream, const gsl_vector_short * v,
                              const char *format);

int gsl_vector_short_memcpy (gsl_vector_short * dest, const gsl_vector_short * src);

int gsl_vector_short_reverse (gsl_vector_short * v);

int gsl_vector_short_swap (gsl_vector_short * v, gsl_vector_short * w);
int gsl_vector_short_swap_elements (gsl_vector_short * v, const size_t i, const size_t j);

short gsl_vector_short_max (const gsl_vector_short * v);
short gsl_vector_short_min (const gsl_vector_short * v);
void gsl_vector_short_minmax (const gsl_vector_short * v, short * min_out, short * max_out);

size_t gsl_vector_short_max_index (const gsl_vector_short * v);
size_t gsl_vector_short_min_index (const gsl_vector_short * v);
void gsl_vector_short_minmax_index (const gsl_vector_short * v, size_t * imin, size_t * imax);

int gsl_vector_short_add (gsl_vector_short * a, const gsl_vector_short * b);
int gsl_vector_short_sub (gsl_vector_short * a, const gsl_vector_short * b);
int gsl_vector_short_mul (gsl_vector_short * a, const gsl_vector_short * b);
int gsl_vector_short_div (gsl_vector_short * a, const gsl_vector_short * b);
int gsl_vector_short_scale (gsl_vector_short * a, const double x);
int gsl_vector_short_add_constant (gsl_vector_short * a, const double x);

int gsl_vector_short_equal (const gsl_vector_short * u, 
                            const gsl_vector_short * v);

int gsl_vector_short_isnull (const gsl_vector_short * v);
int gsl_vector_short_ispos (const gsl_vector_short * v);
int gsl_vector_short_isneg (const gsl_vector_short * v);
int gsl_vector_short_isnonneg (const gsl_vector_short * v);

INLINE_DECL short gsl_vector_short_get (const gsl_vector_short * v, const size_t i);
INLINE_DECL void gsl_vector_short_set (gsl_vector_short * v, const size_t i, short x);
INLINE_DECL short * gsl_vector_short_ptr (gsl_vector_short * v, const size_t i);
INLINE_DECL const short * gsl_vector_short_const_ptr (const gsl_vector_short * v, const size_t i);

#ifdef HAVE_INLINE

INLINE_FUN
short
gsl_vector_short_get (const gsl_vector_short * v, const size_t i)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(i >= v->size))
    {
      GSL_ERROR_VAL ("index out of range", GSL_EINVAL, 0);
    }
#endif
  return v->data[i * v->stride];
}

INLINE_FUN
void
gsl_vector_short_set (gsl_vector_short * v, const size_t i, short x)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(i >= v->size))
    {
      GSL_ERROR_VOID ("index out of range", GSL_EINVAL);
    }
#endif
  v->data[i * v->stride] = x;
}

INLINE_FUN
short *
gsl_vector_short_ptr (gsl_vector_short * v, const size_t i)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(i >= v->size))
    {
      GSL_ERROR_NULL ("index out of range", GSL_EINVAL);
    }
#endif
  return (short *) (v->data + i * v->stride);
}

INLINE_FUN
const short *
gsl_vector_short_const_ptr (const gsl_vector_short * v, const size_t i)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(i >= v->size))
    {
      GSL_ERROR_NULL ("index out of range", GSL_EINVAL);
    }
#endif
  return (const short *) (v->data + i * v->stride);
}
#endif /* HAVE_INLINE */

__END_DECLS

#endif /* __GSL_VECTOR_SHORT_H__ */



/* vector/gsl_vector_uchar.h
 * 
 * Copyright (C) 1996, 1997, 1998, 1999, 2000, 2007 Gerard Jungman, Brian Gough
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __GSL_VECTOR_UCHAR_H__
#define __GSL_VECTOR_UCHAR_H__

#include <stdlib.h>
/* block/gsl_block_uchar.h
 * 
 * Copyright (C) 1996, 1997, 1998, 1999, 2000, 2007 Gerard Jungman, Brian Gough
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __GSL_BLOCK_UCHAR_H__
#define __GSL_BLOCK_UCHAR_H__

#include <stdlib.h>

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif

__BEGIN_DECLS

struct gsl_block_uchar_struct
{
  size_t size;
  unsigned char *data;
};

typedef struct gsl_block_uchar_struct gsl_block_uchar;

gsl_block_uchar *gsl_block_uchar_alloc (const size_t n);
gsl_block_uchar *gsl_block_uchar_calloc (const size_t n);
void gsl_block_uchar_free (gsl_block_uchar * b);

int gsl_block_uchar_fread (FILE * stream, gsl_block_uchar * b);
int gsl_block_uchar_fwrite (FILE * stream, const gsl_block_uchar * b);
int gsl_block_uchar_fscanf (FILE * stream, gsl_block_uchar * b);
int gsl_block_uchar_fprintf (FILE * stream, const gsl_block_uchar * b, const char *format);

int gsl_block_uchar_raw_fread (FILE * stream, unsigned char * b, const size_t n, const size_t stride);
int gsl_block_uchar_raw_fwrite (FILE * stream, const unsigned char * b, const size_t n, const size_t stride);
int gsl_block_uchar_raw_fscanf (FILE * stream, unsigned char * b, const size_t n, const size_t stride);
int gsl_block_uchar_raw_fprintf (FILE * stream, const unsigned char * b, const size_t n, const size_t stride, const char *format);

size_t gsl_block_uchar_size (const gsl_block_uchar * b);
unsigned char * gsl_block_uchar_data (const gsl_block_uchar * b);

__END_DECLS

#endif /* __GSL_BLOCK_UCHAR_H__ */

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif

__BEGIN_DECLS

typedef struct 
{
  size_t size;
  size_t stride;
  unsigned char *data;
  gsl_block_uchar *block;
  int owner;
} 
gsl_vector_uchar;

typedef struct
{
  gsl_vector_uchar vector;
} _gsl_vector_uchar_view;

typedef _gsl_vector_uchar_view gsl_vector_uchar_view;

typedef struct
{
  gsl_vector_uchar vector;
} _gsl_vector_uchar_const_view;

typedef const _gsl_vector_uchar_const_view gsl_vector_uchar_const_view;


/* Allocation */

gsl_vector_uchar *gsl_vector_uchar_alloc (const size_t n);
gsl_vector_uchar *gsl_vector_uchar_calloc (const size_t n);

gsl_vector_uchar *gsl_vector_uchar_alloc_from_block (gsl_block_uchar * b,
                                                     const size_t offset, 
                                                     const size_t n, 
                                                     const size_t stride);

gsl_vector_uchar *gsl_vector_uchar_alloc_from_vector (gsl_vector_uchar * v,
                                                      const size_t offset, 
                                                      const size_t n, 
                                                      const size_t stride);

void gsl_vector_uchar_free (gsl_vector_uchar * v);

/* Views */

_gsl_vector_uchar_view 
gsl_vector_uchar_view_array (unsigned char *v, size_t n);

_gsl_vector_uchar_view 
gsl_vector_uchar_view_array_with_stride (unsigned char *base,
                                         size_t stride,
                                         size_t n);

_gsl_vector_uchar_const_view 
gsl_vector_uchar_const_view_array (const unsigned char *v, size_t n);

_gsl_vector_uchar_const_view 
gsl_vector_uchar_const_view_array_with_stride (const unsigned char *base,
                                               size_t stride,
                                               size_t n);

_gsl_vector_uchar_view 
gsl_vector_uchar_subvector (gsl_vector_uchar *v, 
                            size_t i, 
                            size_t n);

_gsl_vector_uchar_view 
gsl_vector_uchar_subvector_with_stride (gsl_vector_uchar *v, 
                                        size_t i,
                                        size_t stride,
                                        size_t n);

_gsl_vector_uchar_const_view 
gsl_vector_uchar_const_subvector (const gsl_vector_uchar *v, 
                                  size_t i, 
                                  size_t n);

_gsl_vector_uchar_const_view 
gsl_vector_uchar_const_subvector_with_stride (const gsl_vector_uchar *v, 
                                              size_t i, 
                                              size_t stride,
                                              size_t n);

/* Operations */

void gsl_vector_uchar_set_zero (gsl_vector_uchar * v);
void gsl_vector_uchar_set_all (gsl_vector_uchar * v, unsigned char x);
int gsl_vector_uchar_set_basis (gsl_vector_uchar * v, size_t i);

int gsl_vector_uchar_fread (FILE * stream, gsl_vector_uchar * v);
int gsl_vector_uchar_fwrite (FILE * stream, const gsl_vector_uchar * v);
int gsl_vector_uchar_fscanf (FILE * stream, gsl_vector_uchar * v);
int gsl_vector_uchar_fprintf (FILE * stream, const gsl_vector_uchar * v,
                              const char *format);

int gsl_vector_uchar_memcpy (gsl_vector_uchar * dest, const gsl_vector_uchar * src);

int gsl_vector_uchar_reverse (gsl_vector_uchar * v);

int gsl_vector_uchar_swap (gsl_vector_uchar * v, gsl_vector_uchar * w);
int gsl_vector_uchar_swap_elements (gsl_vector_uchar * v, const size_t i, const size_t j);

unsigned char gsl_vector_uchar_max (const gsl_vector_uchar * v);
unsigned char gsl_vector_uchar_min (const gsl_vector_uchar * v);
void gsl_vector_uchar_minmax (const gsl_vector_uchar * v, unsigned char * min_out, unsigned char * max_out);

size_t gsl_vector_uchar_max_index (const gsl_vector_uchar * v);
size_t gsl_vector_uchar_min_index (const gsl_vector_uchar * v);
void gsl_vector_uchar_minmax_index (const gsl_vector_uchar * v, size_t * imin, size_t * imax);

int gsl_vector_uchar_add (gsl_vector_uchar * a, const gsl_vector_uchar * b);
int gsl_vector_uchar_sub (gsl_vector_uchar * a, const gsl_vector_uchar * b);
int gsl_vector_uchar_mul (gsl_vector_uchar * a, const gsl_vector_uchar * b);
int gsl_vector_uchar_div (gsl_vector_uchar * a, const gsl_vector_uchar * b);
int gsl_vector_uchar_scale (gsl_vector_uchar * a, const double x);
int gsl_vector_uchar_add_constant (gsl_vector_uchar * a, const double x);

int gsl_vector_uchar_equal (const gsl_vector_uchar * u, 
                            const gsl_vector_uchar * v);

int gsl_vector_uchar_isnull (const gsl_vector_uchar * v);
int gsl_vector_uchar_ispos (const gsl_vector_uchar * v);
int gsl_vector_uchar_isneg (const gsl_vector_uchar * v);
int gsl_vector_uchar_isnonneg (const gsl_vector_uchar * v);

INLINE_DECL unsigned char gsl_vector_uchar_get (const gsl_vector_uchar * v, const size_t i);
INLINE_DECL void gsl_vector_uchar_set (gsl_vector_uchar * v, const size_t i, unsigned char x);
INLINE_DECL unsigned char * gsl_vector_uchar_ptr (gsl_vector_uchar * v, const size_t i);
INLINE_DECL const unsigned char * gsl_vector_uchar_const_ptr (const gsl_vector_uchar * v, const size_t i);

#ifdef HAVE_INLINE

INLINE_FUN
unsigned char
gsl_vector_uchar_get (const gsl_vector_uchar * v, const size_t i)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(i >= v->size))
    {
      GSL_ERROR_VAL ("index out of range", GSL_EINVAL, 0);
    }
#endif
  return v->data[i * v->stride];
}

INLINE_FUN
void
gsl_vector_uchar_set (gsl_vector_uchar * v, const size_t i, unsigned char x)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(i >= v->size))
    {
      GSL_ERROR_VOID ("index out of range", GSL_EINVAL);
    }
#endif
  v->data[i * v->stride] = x;
}

INLINE_FUN
unsigned char *
gsl_vector_uchar_ptr (gsl_vector_uchar * v, const size_t i)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(i >= v->size))
    {
      GSL_ERROR_NULL ("index out of range", GSL_EINVAL);
    }
#endif
  return (unsigned char *) (v->data + i * v->stride);
}

INLINE_FUN
const unsigned char *
gsl_vector_uchar_const_ptr (const gsl_vector_uchar * v, const size_t i)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(i >= v->size))
    {
      GSL_ERROR_NULL ("index out of range", GSL_EINVAL);
    }
#endif
  return (const unsigned char *) (v->data + i * v->stride);
}
#endif /* HAVE_INLINE */

__END_DECLS

#endif /* __GSL_VECTOR_UCHAR_H__ */


/* vector/gsl_vector_char.h
 * 
 * Copyright (C) 1996, 1997, 1998, 1999, 2000, 2007 Gerard Jungman, Brian Gough
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __GSL_VECTOR_CHAR_H__
#define __GSL_VECTOR_CHAR_H__

#include <stdlib.h>
/* block/gsl_block_char.h
 * 
 * Copyright (C) 1996, 1997, 1998, 1999, 2000, 2007 Gerard Jungman, Brian Gough
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __GSL_BLOCK_CHAR_H__
#define __GSL_BLOCK_CHAR_H__

#include <stdlib.h>

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif

__BEGIN_DECLS

struct gsl_block_char_struct
{
  size_t size;
  char *data;
};

typedef struct gsl_block_char_struct gsl_block_char;

gsl_block_char *gsl_block_char_alloc (const size_t n);
gsl_block_char *gsl_block_char_calloc (const size_t n);
void gsl_block_char_free (gsl_block_char * b);

int gsl_block_char_fread (FILE * stream, gsl_block_char * b);
int gsl_block_char_fwrite (FILE * stream, const gsl_block_char * b);
int gsl_block_char_fscanf (FILE * stream, gsl_block_char * b);
int gsl_block_char_fprintf (FILE * stream, const gsl_block_char * b, const char *format);

int gsl_block_char_raw_fread (FILE * stream, char * b, const size_t n, const size_t stride);
int gsl_block_char_raw_fwrite (FILE * stream, const char * b, const size_t n, const size_t stride);
int gsl_block_char_raw_fscanf (FILE * stream, char * b, const size_t n, const size_t stride);
int gsl_block_char_raw_fprintf (FILE * stream, const char * b, const size_t n, const size_t stride, const char *format);

size_t gsl_block_char_size (const gsl_block_char * b);
char * gsl_block_char_data (const gsl_block_char * b);

__END_DECLS

#endif /* __GSL_BLOCK_CHAR_H__ */

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif

__BEGIN_DECLS

typedef struct 
{
  size_t size;
  size_t stride;
  char *data;
  gsl_block_char *block;
  int owner;
} 
gsl_vector_char;

typedef struct
{
  gsl_vector_char vector;
} _gsl_vector_char_view;

typedef _gsl_vector_char_view gsl_vector_char_view;

typedef struct
{
  gsl_vector_char vector;
} _gsl_vector_char_const_view;

typedef const _gsl_vector_char_const_view gsl_vector_char_const_view;


/* Allocation */

gsl_vector_char *gsl_vector_char_alloc (const size_t n);
gsl_vector_char *gsl_vector_char_calloc (const size_t n);

gsl_vector_char *gsl_vector_char_alloc_from_block (gsl_block_char * b,
                                                     const size_t offset, 
                                                     const size_t n, 
                                                     const size_t stride);

gsl_vector_char *gsl_vector_char_alloc_from_vector (gsl_vector_char * v,
                                                      const size_t offset, 
                                                      const size_t n, 
                                                      const size_t stride);

void gsl_vector_char_free (gsl_vector_char * v);

/* Views */

_gsl_vector_char_view 
gsl_vector_char_view_array (char *v, size_t n);

_gsl_vector_char_view 
gsl_vector_char_view_array_with_stride (char *base,
                                         size_t stride,
                                         size_t n);

_gsl_vector_char_const_view 
gsl_vector_char_const_view_array (const char *v, size_t n);

_gsl_vector_char_const_view 
gsl_vector_char_const_view_array_with_stride (const char *base,
                                               size_t stride,
                                               size_t n);

_gsl_vector_char_view 
gsl_vector_char_subvector (gsl_vector_char *v, 
                            size_t i, 
                            size_t n);

_gsl_vector_char_view 
gsl_vector_char_subvector_with_stride (gsl_vector_char *v, 
                                        size_t i,
                                        size_t stride,
                                        size_t n);

_gsl_vector_char_const_view 
gsl_vector_char_const_subvector (const gsl_vector_char *v, 
                                  size_t i, 
                                  size_t n);

_gsl_vector_char_const_view 
gsl_vector_char_const_subvector_with_stride (const gsl_vector_char *v, 
                                              size_t i, 
                                              size_t stride,
                                              size_t n);

/* Operations */

void gsl_vector_char_set_zero (gsl_vector_char * v);
void gsl_vector_char_set_all (gsl_vector_char * v, char x);
int gsl_vector_char_set_basis (gsl_vector_char * v, size_t i);

int gsl_vector_char_fread (FILE * stream, gsl_vector_char * v);
int gsl_vector_char_fwrite (FILE * stream, const gsl_vector_char * v);
int gsl_vector_char_fscanf (FILE * stream, gsl_vector_char * v);
int gsl_vector_char_fprintf (FILE * stream, const gsl_vector_char * v,
                              const char *format);

int gsl_vector_char_memcpy (gsl_vector_char * dest, const gsl_vector_char * src);

int gsl_vector_char_reverse (gsl_vector_char * v);

int gsl_vector_char_swap (gsl_vector_char * v, gsl_vector_char * w);
int gsl_vector_char_swap_elements (gsl_vector_char * v, const size_t i, const size_t j);

char gsl_vector_char_max (const gsl_vector_char * v);
char gsl_vector_char_min (const gsl_vector_char * v);
void gsl_vector_char_minmax (const gsl_vector_char * v, char * min_out, char * max_out);

size_t gsl_vector_char_max_index (const gsl_vector_char * v);
size_t gsl_vector_char_min_index (const gsl_vector_char * v);
void gsl_vector_char_minmax_index (const gsl_vector_char * v, size_t * imin, size_t * imax);

int gsl_vector_char_add (gsl_vector_char * a, const gsl_vector_char * b);
int gsl_vector_char_sub (gsl_vector_char * a, const gsl_vector_char * b);
int gsl_vector_char_mul (gsl_vector_char * a, const gsl_vector_char * b);
int gsl_vector_char_div (gsl_vector_char * a, const gsl_vector_char * b);
int gsl_vector_char_scale (gsl_vector_char * a, const double x);
int gsl_vector_char_add_constant (gsl_vector_char * a, const double x);

int gsl_vector_char_equal (const gsl_vector_char * u, 
                            const gsl_vector_char * v);

int gsl_vector_char_isnull (const gsl_vector_char * v);
int gsl_vector_char_ispos (const gsl_vector_char * v);
int gsl_vector_char_isneg (const gsl_vector_char * v);
int gsl_vector_char_isnonneg (const gsl_vector_char * v);

INLINE_DECL char gsl_vector_char_get (const gsl_vector_char * v, const size_t i);
INLINE_DECL void gsl_vector_char_set (gsl_vector_char * v, const size_t i, char x);
INLINE_DECL char * gsl_vector_char_ptr (gsl_vector_char * v, const size_t i);
INLINE_DECL const char * gsl_vector_char_const_ptr (const gsl_vector_char * v, const size_t i);

#ifdef HAVE_INLINE

INLINE_FUN
char
gsl_vector_char_get (const gsl_vector_char * v, const size_t i)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(i >= v->size))
    {
      GSL_ERROR_VAL ("index out of range", GSL_EINVAL, 0);
    }
#endif
  return v->data[i * v->stride];
}

INLINE_FUN
void
gsl_vector_char_set (gsl_vector_char * v, const size_t i, char x)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(i >= v->size))
    {
      GSL_ERROR_VOID ("index out of range", GSL_EINVAL);
    }
#endif
  v->data[i * v->stride] = x;
}

INLINE_FUN
char *
gsl_vector_char_ptr (gsl_vector_char * v, const size_t i)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(i >= v->size))
    {
      GSL_ERROR_NULL ("index out of range", GSL_EINVAL);
    }
#endif
  return (char *) (v->data + i * v->stride);
}

INLINE_FUN
const char *
gsl_vector_char_const_ptr (const gsl_vector_char * v, const size_t i)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(i >= v->size))
    {
      GSL_ERROR_NULL ("index out of range", GSL_EINVAL);
    }
#endif
  return (const char *) (v->data + i * v->stride);
}
#endif /* HAVE_INLINE */

__END_DECLS

#endif /* __GSL_VECTOR_CHAR_H__ */




#endif /* __GSL_VECTOR_H__ */
#ifndef __GSL_MATRIX_H__
#define __GSL_MATRIX_H__

/* matrix/gsl_matrix_complex_long_double.h
 * 
 * Copyright (C) 1996, 1997, 1998, 1999, 2000, 2007 Gerard Jungman, Brian Gough
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __GSL_MATRIX_COMPLEX_LONG_DOUBLE_H__
#define __GSL_MATRIX_COMPLEX_LONG_DOUBLE_H__

#include <stdlib.h>

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif

__BEGIN_DECLS

typedef struct 
{
  size_t size1;
  size_t size2;
  size_t tda;
  long double * data;
  gsl_block_complex_long_double * block;
  int owner;
} gsl_matrix_complex_long_double ;

typedef struct
{
  gsl_matrix_complex_long_double matrix;
} _gsl_matrix_complex_long_double_view;

typedef _gsl_matrix_complex_long_double_view gsl_matrix_complex_long_double_view;

typedef struct
{
  gsl_matrix_complex_long_double matrix;
} _gsl_matrix_complex_long_double_const_view;

typedef const _gsl_matrix_complex_long_double_const_view gsl_matrix_complex_long_double_const_view;


/* Allocation */

gsl_matrix_complex_long_double * 
gsl_matrix_complex_long_double_alloc (const size_t n1, const size_t n2);

gsl_matrix_complex_long_double * 
gsl_matrix_complex_long_double_calloc (const size_t n1, const size_t n2);

gsl_matrix_complex_long_double * 
gsl_matrix_complex_long_double_alloc_from_block (gsl_block_complex_long_double * b, 
                                           const size_t offset, 
                                           const size_t n1, const size_t n2, const size_t d2);

gsl_matrix_complex_long_double * 
gsl_matrix_complex_long_double_alloc_from_matrix (gsl_matrix_complex_long_double * b,
                                            const size_t k1, const size_t k2,
                                            const size_t n1, const size_t n2);

gsl_vector_complex_long_double * 
gsl_vector_complex_long_double_alloc_row_from_matrix (gsl_matrix_complex_long_double * m,
                                                const size_t i);

gsl_vector_complex_long_double * 
gsl_vector_complex_long_double_alloc_col_from_matrix (gsl_matrix_complex_long_double * m,
                                                const size_t j);

void gsl_matrix_complex_long_double_free (gsl_matrix_complex_long_double * m);

/* Views */

_gsl_matrix_complex_long_double_view 
gsl_matrix_complex_long_double_submatrix (gsl_matrix_complex_long_double * m, 
                            const size_t i, const size_t j, 
                            const size_t n1, const size_t n2);

_gsl_vector_complex_long_double_view 
gsl_matrix_complex_long_double_row (gsl_matrix_complex_long_double * m, const size_t i);

_gsl_vector_complex_long_double_view 
gsl_matrix_complex_long_double_column (gsl_matrix_complex_long_double * m, const size_t j);

_gsl_vector_complex_long_double_view 
gsl_matrix_complex_long_double_diagonal (gsl_matrix_complex_long_double * m);

_gsl_vector_complex_long_double_view 
gsl_matrix_complex_long_double_subdiagonal (gsl_matrix_complex_long_double * m, const size_t k);

_gsl_vector_complex_long_double_view 
gsl_matrix_complex_long_double_superdiagonal (gsl_matrix_complex_long_double * m, const size_t k);

_gsl_vector_complex_long_double_view
gsl_matrix_complex_long_double_subrow (gsl_matrix_complex_long_double * m,
                                 const size_t i, const size_t offset,
                                 const size_t n);

_gsl_vector_complex_long_double_view
gsl_matrix_complex_long_double_subcolumn (gsl_matrix_complex_long_double * m,
                                    const size_t j, const size_t offset,
                                    const size_t n);

_gsl_matrix_complex_long_double_view
gsl_matrix_complex_long_double_view_array (long double * base,
                             const size_t n1, 
                             const size_t n2);

_gsl_matrix_complex_long_double_view
gsl_matrix_complex_long_double_view_array_with_tda (long double * base, 
                                      const size_t n1, 
                                      const size_t n2,
                                      const size_t tda);

_gsl_matrix_complex_long_double_view
gsl_matrix_complex_long_double_view_vector (gsl_vector_complex_long_double * v,
                              const size_t n1, 
                              const size_t n2);

_gsl_matrix_complex_long_double_view
gsl_matrix_complex_long_double_view_vector_with_tda (gsl_vector_complex_long_double * v,
                                       const size_t n1, 
                                       const size_t n2,
                                       const size_t tda);


_gsl_matrix_complex_long_double_const_view 
gsl_matrix_complex_long_double_const_submatrix (const gsl_matrix_complex_long_double * m, 
                                  const size_t i, const size_t j, 
                                  const size_t n1, const size_t n2);

_gsl_vector_complex_long_double_const_view 
gsl_matrix_complex_long_double_const_row (const gsl_matrix_complex_long_double * m, 
                            const size_t i);

_gsl_vector_complex_long_double_const_view 
gsl_matrix_complex_long_double_const_column (const gsl_matrix_complex_long_double * m, 
                               const size_t j);

_gsl_vector_complex_long_double_const_view
gsl_matrix_complex_long_double_const_diagonal (const gsl_matrix_complex_long_double * m);

_gsl_vector_complex_long_double_const_view 
gsl_matrix_complex_long_double_const_subdiagonal (const gsl_matrix_complex_long_double * m, 
                                    const size_t k);

_gsl_vector_complex_long_double_const_view 
gsl_matrix_complex_long_double_const_superdiagonal (const gsl_matrix_complex_long_double * m, 
                                      const size_t k);

_gsl_vector_complex_long_double_const_view
gsl_matrix_complex_long_double_const_subrow (const gsl_matrix_complex_long_double * m,
                                       const size_t i, const size_t offset,
                                       const size_t n);

_gsl_vector_complex_long_double_const_view
gsl_matrix_complex_long_double_const_subcolumn (const gsl_matrix_complex_long_double * m,
                                          const size_t j, const size_t offset,
                                          const size_t n);

_gsl_matrix_complex_long_double_const_view
gsl_matrix_complex_long_double_const_view_array (const long double * base,
                                   const size_t n1, 
                                   const size_t n2);

_gsl_matrix_complex_long_double_const_view
gsl_matrix_complex_long_double_const_view_array_with_tda (const long double * base, 
                                            const size_t n1, 
                                            const size_t n2,
                                            const size_t tda);

_gsl_matrix_complex_long_double_const_view
gsl_matrix_complex_long_double_const_view_vector (const gsl_vector_complex_long_double * v,
                                    const size_t n1, 
                                    const size_t n2);

_gsl_matrix_complex_long_double_const_view
gsl_matrix_complex_long_double_const_view_vector_with_tda (const gsl_vector_complex_long_double * v,
                                             const size_t n1, 
                                             const size_t n2,
                                             const size_t tda);

/* Operations */

void gsl_matrix_complex_long_double_set_zero (gsl_matrix_complex_long_double * m);
void gsl_matrix_complex_long_double_set_identity (gsl_matrix_complex_long_double * m);
void gsl_matrix_complex_long_double_set_all (gsl_matrix_complex_long_double * m, gsl_complex_long_double x);

int gsl_matrix_complex_long_double_fread (FILE * stream, gsl_matrix_complex_long_double * m) ;
int gsl_matrix_complex_long_double_fwrite (FILE * stream, const gsl_matrix_complex_long_double * m) ;
int gsl_matrix_complex_long_double_fscanf (FILE * stream, gsl_matrix_complex_long_double * m);
int gsl_matrix_complex_long_double_fprintf (FILE * stream, const gsl_matrix_complex_long_double * m, const char * format);

int gsl_matrix_complex_long_double_memcpy(gsl_matrix_complex_long_double * dest, const gsl_matrix_complex_long_double * src);
int gsl_matrix_complex_long_double_swap(gsl_matrix_complex_long_double * m1, gsl_matrix_complex_long_double * m2);

int gsl_matrix_complex_long_double_swap_rows(gsl_matrix_complex_long_double * m, const size_t i, const size_t j);
int gsl_matrix_complex_long_double_swap_columns(gsl_matrix_complex_long_double * m, const size_t i, const size_t j);
int gsl_matrix_complex_long_double_swap_rowcol(gsl_matrix_complex_long_double * m, const size_t i, const size_t j);

int gsl_matrix_complex_long_double_transpose (gsl_matrix_complex_long_double * m);
int gsl_matrix_complex_long_double_transpose_memcpy (gsl_matrix_complex_long_double * dest, const gsl_matrix_complex_long_double * src);

int gsl_matrix_complex_long_double_equal (const gsl_matrix_complex_long_double * a, const gsl_matrix_complex_long_double * b);

int gsl_matrix_complex_long_double_isnull (const gsl_matrix_complex_long_double * m);
int gsl_matrix_complex_long_double_ispos (const gsl_matrix_complex_long_double * m);
int gsl_matrix_complex_long_double_isneg (const gsl_matrix_complex_long_double * m);
int gsl_matrix_complex_long_double_isnonneg (const gsl_matrix_complex_long_double * m);

int gsl_matrix_complex_long_double_add (gsl_matrix_complex_long_double * a, const gsl_matrix_complex_long_double * b);
int gsl_matrix_complex_long_double_sub (gsl_matrix_complex_long_double * a, const gsl_matrix_complex_long_double * b);
int gsl_matrix_complex_long_double_mul_elements (gsl_matrix_complex_long_double * a, const gsl_matrix_complex_long_double * b);
int gsl_matrix_complex_long_double_div_elements (gsl_matrix_complex_long_double * a, const gsl_matrix_complex_long_double * b);
int gsl_matrix_complex_long_double_scale (gsl_matrix_complex_long_double * a, const gsl_complex_long_double x);
int gsl_matrix_complex_long_double_add_constant (gsl_matrix_complex_long_double * a, const gsl_complex_long_double x);
int gsl_matrix_complex_long_double_add_diagonal (gsl_matrix_complex_long_double * a, const gsl_complex_long_double x);

/***********************************************************************/
/* The functions below are obsolete                                    */
/***********************************************************************/
int gsl_matrix_complex_long_double_get_row(gsl_vector_complex_long_double * v, const gsl_matrix_complex_long_double * m, const size_t i);
int gsl_matrix_complex_long_double_get_col(gsl_vector_complex_long_double * v, const gsl_matrix_complex_long_double * m, const size_t j);
int gsl_matrix_complex_long_double_set_row(gsl_matrix_complex_long_double * m, const size_t i, const gsl_vector_complex_long_double * v);
int gsl_matrix_complex_long_double_set_col(gsl_matrix_complex_long_double * m, const size_t j, const gsl_vector_complex_long_double * v);
/***********************************************************************/

/* inline functions if you are using GCC */

INLINE_DECL gsl_complex_long_double gsl_matrix_complex_long_double_get(const gsl_matrix_complex_long_double * m, const size_t i, const size_t j);
INLINE_DECL void gsl_matrix_complex_long_double_set(gsl_matrix_complex_long_double * m, const size_t i, const size_t j, const gsl_complex_long_double x);

INLINE_DECL gsl_complex_long_double * gsl_matrix_complex_long_double_ptr(gsl_matrix_complex_long_double * m, const size_t i, const size_t j);
INLINE_DECL const gsl_complex_long_double * gsl_matrix_complex_long_double_const_ptr(const gsl_matrix_complex_long_double * m, const size_t i, const size_t j);

#ifdef HAVE_INLINE

INLINE_FUN 
gsl_complex_long_double
gsl_matrix_complex_long_double_get(const gsl_matrix_complex_long_double * m, 
                     const size_t i, const size_t j)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(1)) 
    {
      gsl_complex_long_double zero = {{0,0}};

      if (i >= m->size1)
        {
          GSL_ERROR_VAL("first index out of range", GSL_EINVAL, zero) ;
        }
      else if (j >= m->size2)
        {
          GSL_ERROR_VAL("second index out of range", GSL_EINVAL, zero) ;
        }
    }
#endif
  return *(gsl_complex_long_double *)(m->data + 2*(i * m->tda + j)) ;
} 

INLINE_FUN 
void
gsl_matrix_complex_long_double_set(gsl_matrix_complex_long_double * m, 
                     const size_t i, const size_t j, const gsl_complex_long_double x)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(1)) 
    {
      if (i >= m->size1)
        {
          GSL_ERROR_VOID("first index out of range", GSL_EINVAL) ;
        }
      else if (j >= m->size2)
        {
          GSL_ERROR_VOID("second index out of range", GSL_EINVAL) ;
        }
    }
#endif
  *(gsl_complex_long_double *)(m->data + 2*(i * m->tda + j)) = x ;
}

INLINE_FUN 
gsl_complex_long_double *
gsl_matrix_complex_long_double_ptr(gsl_matrix_complex_long_double * m, 
                             const size_t i, const size_t j)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(1)) 
    {
      if (i >= m->size1)
        {
          GSL_ERROR_NULL("first index out of range", GSL_EINVAL) ;
        }
      else if (j >= m->size2)
        {
          GSL_ERROR_NULL("second index out of range", GSL_EINVAL) ;
        }
    }
#endif
  return (gsl_complex_long_double *)(m->data + 2*(i * m->tda + j)) ;
} 

INLINE_FUN 
const gsl_complex_long_double *
gsl_matrix_complex_long_double_const_ptr(const gsl_matrix_complex_long_double * m, 
                                   const size_t i, const size_t j)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(1)) 
    {
      if (i >= m->size1)
        {
          GSL_ERROR_NULL("first index out of range", GSL_EINVAL) ;
        }
      else if (j >= m->size2)
        {
          GSL_ERROR_NULL("second index out of range", GSL_EINVAL) ;
        }
    }
#endif
  return (const gsl_complex_long_double *)(m->data + 2*(i * m->tda + j)) ;
} 

#endif /* HAVE_INLINE */

__END_DECLS

#endif /* __GSL_MATRIX_COMPLEX_LONG_DOUBLE_H__ */
/* matrix/gsl_matrix_complex_double.h
 * 
 * Copyright (C) 1996, 1997, 1998, 1999, 2000, 2007 Gerard Jungman, Brian Gough
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __GSL_MATRIX_COMPLEX_DOUBLE_H__
#define __GSL_MATRIX_COMPLEX_DOUBLE_H__

#include <stdlib.h>

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif

__BEGIN_DECLS

typedef struct 
{
  size_t size1;
  size_t size2;
  size_t tda;
  double * data;
  gsl_block_complex * block;
  int owner;
} gsl_matrix_complex ;

typedef struct
{
  gsl_matrix_complex matrix;
} _gsl_matrix_complex_view;

typedef _gsl_matrix_complex_view gsl_matrix_complex_view;

typedef struct
{
  gsl_matrix_complex matrix;
} _gsl_matrix_complex_const_view;

typedef const _gsl_matrix_complex_const_view gsl_matrix_complex_const_view;


/* Allocation */

gsl_matrix_complex * 
gsl_matrix_complex_alloc (const size_t n1, const size_t n2);

gsl_matrix_complex * 
gsl_matrix_complex_calloc (const size_t n1, const size_t n2);

gsl_matrix_complex * 
gsl_matrix_complex_alloc_from_block (gsl_block_complex * b, 
                                           const size_t offset, 
                                           const size_t n1, const size_t n2, const size_t d2);

gsl_matrix_complex * 
gsl_matrix_complex_alloc_from_matrix (gsl_matrix_complex * b,
                                            const size_t k1, const size_t k2,
                                            const size_t n1, const size_t n2);

gsl_vector_complex * 
gsl_vector_complex_alloc_row_from_matrix (gsl_matrix_complex * m,
                                                const size_t i);

gsl_vector_complex * 
gsl_vector_complex_alloc_col_from_matrix (gsl_matrix_complex * m,
                                                const size_t j);

void gsl_matrix_complex_free (gsl_matrix_complex * m);

/* Views */

_gsl_matrix_complex_view 
gsl_matrix_complex_submatrix (gsl_matrix_complex * m, 
                            const size_t i, const size_t j, 
                            const size_t n1, const size_t n2);

_gsl_vector_complex_view 
gsl_matrix_complex_row (gsl_matrix_complex * m, const size_t i);

_gsl_vector_complex_view 
gsl_matrix_complex_column (gsl_matrix_complex * m, const size_t j);

_gsl_vector_complex_view 
gsl_matrix_complex_diagonal (gsl_matrix_complex * m);

_gsl_vector_complex_view 
gsl_matrix_complex_subdiagonal (gsl_matrix_complex * m, const size_t k);

_gsl_vector_complex_view 
gsl_matrix_complex_superdiagonal (gsl_matrix_complex * m, const size_t k);

_gsl_vector_complex_view
gsl_matrix_complex_subrow (gsl_matrix_complex * m,
                                 const size_t i, const size_t offset,
                                 const size_t n);

_gsl_vector_complex_view
gsl_matrix_complex_subcolumn (gsl_matrix_complex * m,
                                    const size_t j, const size_t offset,
                                    const size_t n);

_gsl_matrix_complex_view
gsl_matrix_complex_view_array (double * base,
                             const size_t n1, 
                             const size_t n2);

_gsl_matrix_complex_view
gsl_matrix_complex_view_array_with_tda (double * base, 
                                      const size_t n1, 
                                      const size_t n2,
                                      const size_t tda);

_gsl_matrix_complex_view
gsl_matrix_complex_view_vector (gsl_vector_complex * v,
                              const size_t n1, 
                              const size_t n2);

_gsl_matrix_complex_view
gsl_matrix_complex_view_vector_with_tda (gsl_vector_complex * v,
                                       const size_t n1, 
                                       const size_t n2,
                                       const size_t tda);


_gsl_matrix_complex_const_view 
gsl_matrix_complex_const_submatrix (const gsl_matrix_complex * m, 
                                  const size_t i, const size_t j, 
                                  const size_t n1, const size_t n2);

_gsl_vector_complex_const_view 
gsl_matrix_complex_const_row (const gsl_matrix_complex * m, 
                            const size_t i);

_gsl_vector_complex_const_view 
gsl_matrix_complex_const_column (const gsl_matrix_complex * m, 
                               const size_t j);

_gsl_vector_complex_const_view
gsl_matrix_complex_const_diagonal (const gsl_matrix_complex * m);

_gsl_vector_complex_const_view 
gsl_matrix_complex_const_subdiagonal (const gsl_matrix_complex * m, 
                                    const size_t k);

_gsl_vector_complex_const_view 
gsl_matrix_complex_const_superdiagonal (const gsl_matrix_complex * m, 
                                      const size_t k);

_gsl_vector_complex_const_view
gsl_matrix_complex_const_subrow (const gsl_matrix_complex * m,
                                       const size_t i, const size_t offset,
                                       const size_t n);

_gsl_vector_complex_const_view
gsl_matrix_complex_const_subcolumn (const gsl_matrix_complex * m,
                                          const size_t j, const size_t offset,
                                          const size_t n);

_gsl_matrix_complex_const_view
gsl_matrix_complex_const_view_array (const double * base,
                                   const size_t n1, 
                                   const size_t n2);

_gsl_matrix_complex_const_view
gsl_matrix_complex_const_view_array_with_tda (const double * base, 
                                            const size_t n1, 
                                            const size_t n2,
                                            const size_t tda);

_gsl_matrix_complex_const_view
gsl_matrix_complex_const_view_vector (const gsl_vector_complex * v,
                                    const size_t n1, 
                                    const size_t n2);

_gsl_matrix_complex_const_view
gsl_matrix_complex_const_view_vector_with_tda (const gsl_vector_complex * v,
                                             const size_t n1, 
                                             const size_t n2,
                                             const size_t tda);

/* Operations */

void gsl_matrix_complex_set_zero (gsl_matrix_complex * m);
void gsl_matrix_complex_set_identity (gsl_matrix_complex * m);
void gsl_matrix_complex_set_all (gsl_matrix_complex * m, gsl_complex x);

int gsl_matrix_complex_fread (FILE * stream, gsl_matrix_complex * m) ;
int gsl_matrix_complex_fwrite (FILE * stream, const gsl_matrix_complex * m) ;
int gsl_matrix_complex_fscanf (FILE * stream, gsl_matrix_complex * m);
int gsl_matrix_complex_fprintf (FILE * stream, const gsl_matrix_complex * m, const char * format);

int gsl_matrix_complex_memcpy(gsl_matrix_complex * dest, const gsl_matrix_complex * src);
int gsl_matrix_complex_swap(gsl_matrix_complex * m1, gsl_matrix_complex * m2);

int gsl_matrix_complex_swap_rows(gsl_matrix_complex * m, const size_t i, const size_t j);
int gsl_matrix_complex_swap_columns(gsl_matrix_complex * m, const size_t i, const size_t j);
int gsl_matrix_complex_swap_rowcol(gsl_matrix_complex * m, const size_t i, const size_t j);

int gsl_matrix_complex_transpose (gsl_matrix_complex * m);
int gsl_matrix_complex_transpose_memcpy (gsl_matrix_complex * dest, const gsl_matrix_complex * src);

int gsl_matrix_complex_equal (const gsl_matrix_complex * a, const gsl_matrix_complex * b);

int gsl_matrix_complex_isnull (const gsl_matrix_complex * m);
int gsl_matrix_complex_ispos (const gsl_matrix_complex * m);
int gsl_matrix_complex_isneg (const gsl_matrix_complex * m);
int gsl_matrix_complex_isnonneg (const gsl_matrix_complex * m);

int gsl_matrix_complex_add (gsl_matrix_complex * a, const gsl_matrix_complex * b);
int gsl_matrix_complex_sub (gsl_matrix_complex * a, const gsl_matrix_complex * b);
int gsl_matrix_complex_mul_elements (gsl_matrix_complex * a, const gsl_matrix_complex * b);
int gsl_matrix_complex_div_elements (gsl_matrix_complex * a, const gsl_matrix_complex * b);
int gsl_matrix_complex_scale (gsl_matrix_complex * a, const gsl_complex x);
int gsl_matrix_complex_add_constant (gsl_matrix_complex * a, const gsl_complex x);
int gsl_matrix_complex_add_diagonal (gsl_matrix_complex * a, const gsl_complex x);

/***********************************************************************/
/* The functions below are obsolete                                    */
/***********************************************************************/
int gsl_matrix_complex_get_row(gsl_vector_complex * v, const gsl_matrix_complex * m, const size_t i);
int gsl_matrix_complex_get_col(gsl_vector_complex * v, const gsl_matrix_complex * m, const size_t j);
int gsl_matrix_complex_set_row(gsl_matrix_complex * m, const size_t i, const gsl_vector_complex * v);
int gsl_matrix_complex_set_col(gsl_matrix_complex * m, const size_t j, const gsl_vector_complex * v);
/***********************************************************************/

/* inline functions if you are using GCC */

INLINE_DECL gsl_complex gsl_matrix_complex_get(const gsl_matrix_complex * m, const size_t i, const size_t j);
INLINE_DECL void gsl_matrix_complex_set(gsl_matrix_complex * m, const size_t i, const size_t j, const gsl_complex x);

INLINE_DECL gsl_complex * gsl_matrix_complex_ptr(gsl_matrix_complex * m, const size_t i, const size_t j);
INLINE_DECL const gsl_complex * gsl_matrix_complex_const_ptr(const gsl_matrix_complex * m, const size_t i, const size_t j);

#ifdef HAVE_INLINE

INLINE_FUN 
gsl_complex
gsl_matrix_complex_get(const gsl_matrix_complex * m, 
                     const size_t i, const size_t j)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(1)) 
    {
      gsl_complex zero = {{0,0}};

      if (i >= m->size1)
        {
          GSL_ERROR_VAL("first index out of range", GSL_EINVAL, zero) ;
        }
      else if (j >= m->size2)
        {
          GSL_ERROR_VAL("second index out of range", GSL_EINVAL, zero) ;
        }
    }
#endif
  return *(gsl_complex *)(m->data + 2*(i * m->tda + j)) ;
} 

INLINE_FUN 
void
gsl_matrix_complex_set(gsl_matrix_complex * m, 
                     const size_t i, const size_t j, const gsl_complex x)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(1)) 
    {
      if (i >= m->size1)
        {
          GSL_ERROR_VOID("first index out of range", GSL_EINVAL) ;
        }
      else if (j >= m->size2)
        {
          GSL_ERROR_VOID("second index out of range", GSL_EINVAL) ;
        }
    }
#endif
  *(gsl_complex *)(m->data + 2*(i * m->tda + j)) = x ;
}

INLINE_FUN 
gsl_complex *
gsl_matrix_complex_ptr(gsl_matrix_complex * m, 
                             const size_t i, const size_t j)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(1)) 
    {
      if (i >= m->size1)
        {
          GSL_ERROR_NULL("first index out of range", GSL_EINVAL) ;
        }
      else if (j >= m->size2)
        {
          GSL_ERROR_NULL("second index out of range", GSL_EINVAL) ;
        }
    }
#endif
  return (gsl_complex *)(m->data + 2*(i * m->tda + j)) ;
} 

INLINE_FUN 
const gsl_complex *
gsl_matrix_complex_const_ptr(const gsl_matrix_complex * m, 
                                   const size_t i, const size_t j)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(1)) 
    {
      if (i >= m->size1)
        {
          GSL_ERROR_NULL("first index out of range", GSL_EINVAL) ;
        }
      else if (j >= m->size2)
        {
          GSL_ERROR_NULL("second index out of range", GSL_EINVAL) ;
        }
    }
#endif
  return (const gsl_complex *)(m->data + 2*(i * m->tda + j)) ;
} 

#endif /* HAVE_INLINE */

__END_DECLS

#endif /* __GSL_MATRIX_COMPLEX_DOUBLE_H__ */
/* matrix/gsl_matrix_complex_float.h
 * 
 * Copyright (C) 1996, 1997, 1998, 1999, 2000, 2007 Gerard Jungman, Brian Gough
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __GSL_MATRIX_COMPLEX_FLOAT_H__
#define __GSL_MATRIX_COMPLEX_FLOAT_H__

#include <stdlib.h>

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif

__BEGIN_DECLS

typedef struct 
{
  size_t size1;
  size_t size2;
  size_t tda;
  float * data;
  gsl_block_complex_float * block;
  int owner;
} gsl_matrix_complex_float ;

typedef struct
{
  gsl_matrix_complex_float matrix;
} _gsl_matrix_complex_float_view;

typedef _gsl_matrix_complex_float_view gsl_matrix_complex_float_view;

typedef struct
{
  gsl_matrix_complex_float matrix;
} _gsl_matrix_complex_float_const_view;

typedef const _gsl_matrix_complex_float_const_view gsl_matrix_complex_float_const_view;


/* Allocation */

gsl_matrix_complex_float * 
gsl_matrix_complex_float_alloc (const size_t n1, const size_t n2);

gsl_matrix_complex_float * 
gsl_matrix_complex_float_calloc (const size_t n1, const size_t n2);

gsl_matrix_complex_float * 
gsl_matrix_complex_float_alloc_from_block (gsl_block_complex_float * b, 
                                           const size_t offset, 
                                           const size_t n1, const size_t n2, const size_t d2);

gsl_matrix_complex_float * 
gsl_matrix_complex_float_alloc_from_matrix (gsl_matrix_complex_float * b,
                                            const size_t k1, const size_t k2,
                                            const size_t n1, const size_t n2);

gsl_vector_complex_float * 
gsl_vector_complex_float_alloc_row_from_matrix (gsl_matrix_complex_float * m,
                                                const size_t i);

gsl_vector_complex_float * 
gsl_vector_complex_float_alloc_col_from_matrix (gsl_matrix_complex_float * m,
                                                const size_t j);

void gsl_matrix_complex_float_free (gsl_matrix_complex_float * m);

/* Views */

_gsl_matrix_complex_float_view 
gsl_matrix_complex_float_submatrix (gsl_matrix_complex_float * m, 
                            const size_t i, const size_t j, 
                            const size_t n1, const size_t n2);

_gsl_vector_complex_float_view 
gsl_matrix_complex_float_row (gsl_matrix_complex_float * m, const size_t i);

_gsl_vector_complex_float_view 
gsl_matrix_complex_float_column (gsl_matrix_complex_float * m, const size_t j);

_gsl_vector_complex_float_view 
gsl_matrix_complex_float_diagonal (gsl_matrix_complex_float * m);

_gsl_vector_complex_float_view 
gsl_matrix_complex_float_subdiagonal (gsl_matrix_complex_float * m, const size_t k);

_gsl_vector_complex_float_view 
gsl_matrix_complex_float_superdiagonal (gsl_matrix_complex_float * m, const size_t k);

_gsl_vector_complex_float_view
gsl_matrix_complex_float_subrow (gsl_matrix_complex_float * m,
                                 const size_t i, const size_t offset,
                                 const size_t n);

_gsl_vector_complex_float_view
gsl_matrix_complex_float_subcolumn (gsl_matrix_complex_float * m,
                                    const size_t j, const size_t offset,
                                    const size_t n);

_gsl_matrix_complex_float_view
gsl_matrix_complex_float_view_array (float * base,
                             const size_t n1, 
                             const size_t n2);

_gsl_matrix_complex_float_view
gsl_matrix_complex_float_view_array_with_tda (float * base, 
                                      const size_t n1, 
                                      const size_t n2,
                                      const size_t tda);

_gsl_matrix_complex_float_view
gsl_matrix_complex_float_view_vector (gsl_vector_complex_float * v,
                              const size_t n1, 
                              const size_t n2);

_gsl_matrix_complex_float_view
gsl_matrix_complex_float_view_vector_with_tda (gsl_vector_complex_float * v,
                                       const size_t n1, 
                                       const size_t n2,
                                       const size_t tda);


_gsl_matrix_complex_float_const_view 
gsl_matrix_complex_float_const_submatrix (const gsl_matrix_complex_float * m, 
                                  const size_t i, const size_t j, 
                                  const size_t n1, const size_t n2);

_gsl_vector_complex_float_const_view 
gsl_matrix_complex_float_const_row (const gsl_matrix_complex_float * m, 
                            const size_t i);

_gsl_vector_complex_float_const_view 
gsl_matrix_complex_float_const_column (const gsl_matrix_complex_float * m, 
                               const size_t j);

_gsl_vector_complex_float_const_view
gsl_matrix_complex_float_const_diagonal (const gsl_matrix_complex_float * m);

_gsl_vector_complex_float_const_view 
gsl_matrix_complex_float_const_subdiagonal (const gsl_matrix_complex_float * m, 
                                    const size_t k);

_gsl_vector_complex_float_const_view 
gsl_matrix_complex_float_const_superdiagonal (const gsl_matrix_complex_float * m, 
                                      const size_t k);

_gsl_vector_complex_float_const_view
gsl_matrix_complex_float_const_subrow (const gsl_matrix_complex_float * m,
                                       const size_t i, const size_t offset,
                                       const size_t n);

_gsl_vector_complex_float_const_view
gsl_matrix_complex_float_const_subcolumn (const gsl_matrix_complex_float * m,
                                          const size_t j, const size_t offset,
                                          const size_t n);

_gsl_matrix_complex_float_const_view
gsl_matrix_complex_float_const_view_array (const float * base,
                                   const size_t n1, 
                                   const size_t n2);

_gsl_matrix_complex_float_const_view
gsl_matrix_complex_float_const_view_array_with_tda (const float * base, 
                                            const size_t n1, 
                                            const size_t n2,
                                            const size_t tda);

_gsl_matrix_complex_float_const_view
gsl_matrix_complex_float_const_view_vector (const gsl_vector_complex_float * v,
                                    const size_t n1, 
                                    const size_t n2);

_gsl_matrix_complex_float_const_view
gsl_matrix_complex_float_const_view_vector_with_tda (const gsl_vector_complex_float * v,
                                             const size_t n1, 
                                             const size_t n2,
                                             const size_t tda);

/* Operations */

void gsl_matrix_complex_float_set_zero (gsl_matrix_complex_float * m);
void gsl_matrix_complex_float_set_identity (gsl_matrix_complex_float * m);
void gsl_matrix_complex_float_set_all (gsl_matrix_complex_float * m, gsl_complex_float x);

int gsl_matrix_complex_float_fread (FILE * stream, gsl_matrix_complex_float * m) ;
int gsl_matrix_complex_float_fwrite (FILE * stream, const gsl_matrix_complex_float * m) ;
int gsl_matrix_complex_float_fscanf (FILE * stream, gsl_matrix_complex_float * m);
int gsl_matrix_complex_float_fprintf (FILE * stream, const gsl_matrix_complex_float * m, const char * format);

int gsl_matrix_complex_float_memcpy(gsl_matrix_complex_float * dest, const gsl_matrix_complex_float * src);
int gsl_matrix_complex_float_swap(gsl_matrix_complex_float * m1, gsl_matrix_complex_float * m2);

int gsl_matrix_complex_float_swap_rows(gsl_matrix_complex_float * m, const size_t i, const size_t j);
int gsl_matrix_complex_float_swap_columns(gsl_matrix_complex_float * m, const size_t i, const size_t j);
int gsl_matrix_complex_float_swap_rowcol(gsl_matrix_complex_float * m, const size_t i, const size_t j);

int gsl_matrix_complex_float_transpose (gsl_matrix_complex_float * m);
int gsl_matrix_complex_float_transpose_memcpy (gsl_matrix_complex_float * dest, const gsl_matrix_complex_float * src);

int gsl_matrix_complex_float_equal (const gsl_matrix_complex_float * a, const gsl_matrix_complex_float * b);

int gsl_matrix_complex_float_isnull (const gsl_matrix_complex_float * m);
int gsl_matrix_complex_float_ispos (const gsl_matrix_complex_float * m);
int gsl_matrix_complex_float_isneg (const gsl_matrix_complex_float * m);
int gsl_matrix_complex_float_isnonneg (const gsl_matrix_complex_float * m);

int gsl_matrix_complex_float_add (gsl_matrix_complex_float * a, const gsl_matrix_complex_float * b);
int gsl_matrix_complex_float_sub (gsl_matrix_complex_float * a, const gsl_matrix_complex_float * b);
int gsl_matrix_complex_float_mul_elements (gsl_matrix_complex_float * a, const gsl_matrix_complex_float * b);
int gsl_matrix_complex_float_div_elements (gsl_matrix_complex_float * a, const gsl_matrix_complex_float * b);
int gsl_matrix_complex_float_scale (gsl_matrix_complex_float * a, const gsl_complex_float x);
int gsl_matrix_complex_float_add_constant (gsl_matrix_complex_float * a, const gsl_complex_float x);
int gsl_matrix_complex_float_add_diagonal (gsl_matrix_complex_float * a, const gsl_complex_float x);

/***********************************************************************/
/* The functions below are obsolete                                    */
/***********************************************************************/
int gsl_matrix_complex_float_get_row(gsl_vector_complex_float * v, const gsl_matrix_complex_float * m, const size_t i);
int gsl_matrix_complex_float_get_col(gsl_vector_complex_float * v, const gsl_matrix_complex_float * m, const size_t j);
int gsl_matrix_complex_float_set_row(gsl_matrix_complex_float * m, const size_t i, const gsl_vector_complex_float * v);
int gsl_matrix_complex_float_set_col(gsl_matrix_complex_float * m, const size_t j, const gsl_vector_complex_float * v);
/***********************************************************************/

/* inline functions if you are using GCC */

INLINE_DECL gsl_complex_float gsl_matrix_complex_float_get(const gsl_matrix_complex_float * m, const size_t i, const size_t j);
INLINE_DECL void gsl_matrix_complex_float_set(gsl_matrix_complex_float * m, const size_t i, const size_t j, const gsl_complex_float x);

INLINE_DECL gsl_complex_float * gsl_matrix_complex_float_ptr(gsl_matrix_complex_float * m, const size_t i, const size_t j);
INLINE_DECL const gsl_complex_float * gsl_matrix_complex_float_const_ptr(const gsl_matrix_complex_float * m, const size_t i, const size_t j);

#ifdef HAVE_INLINE

INLINE_FUN 
gsl_complex_float
gsl_matrix_complex_float_get(const gsl_matrix_complex_float * m, 
                     const size_t i, const size_t j)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(1)) 
    {
      gsl_complex_float zero = {{0,0}};

      if (i >= m->size1)
        {
          GSL_ERROR_VAL("first index out of range", GSL_EINVAL, zero) ;
        }
      else if (j >= m->size2)
        {
          GSL_ERROR_VAL("second index out of range", GSL_EINVAL, zero) ;
        }
    }
#endif
  return *(gsl_complex_float *)(m->data + 2*(i * m->tda + j)) ;
} 

INLINE_FUN 
void
gsl_matrix_complex_float_set(gsl_matrix_complex_float * m, 
                     const size_t i, const size_t j, const gsl_complex_float x)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(1)) 
    {
      if (i >= m->size1)
        {
          GSL_ERROR_VOID("first index out of range", GSL_EINVAL) ;
        }
      else if (j >= m->size2)
        {
          GSL_ERROR_VOID("second index out of range", GSL_EINVAL) ;
        }
    }
#endif
  *(gsl_complex_float *)(m->data + 2*(i * m->tda + j)) = x ;
}

INLINE_FUN 
gsl_complex_float *
gsl_matrix_complex_float_ptr(gsl_matrix_complex_float * m, 
                             const size_t i, const size_t j)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(1)) 
    {
      if (i >= m->size1)
        {
          GSL_ERROR_NULL("first index out of range", GSL_EINVAL) ;
        }
      else if (j >= m->size2)
        {
          GSL_ERROR_NULL("second index out of range", GSL_EINVAL) ;
        }
    }
#endif
  return (gsl_complex_float *)(m->data + 2*(i * m->tda + j)) ;
} 

INLINE_FUN 
const gsl_complex_float *
gsl_matrix_complex_float_const_ptr(const gsl_matrix_complex_float * m, 
                                   const size_t i, const size_t j)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(1)) 
    {
      if (i >= m->size1)
        {
          GSL_ERROR_NULL("first index out of range", GSL_EINVAL) ;
        }
      else if (j >= m->size2)
        {
          GSL_ERROR_NULL("second index out of range", GSL_EINVAL) ;
        }
    }
#endif
  return (const gsl_complex_float *)(m->data + 2*(i * m->tda + j)) ;
} 

#endif /* HAVE_INLINE */

__END_DECLS

#endif /* __GSL_MATRIX_COMPLEX_FLOAT_H__ */

/* matrix/gsl_matrix_long_double.h
 * 
 * Copyright (C) 1996, 1997, 1998, 1999, 2000, 2007 Gerard Jungman, Brian Gough
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __GSL_MATRIX_LONG_DOUBLE_H__
#define __GSL_MATRIX_LONG_DOUBLE_H__

#include <stdlib.h>

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif

__BEGIN_DECLS

typedef struct 
{
  size_t size1;
  size_t size2;
  size_t tda;
  long double * data;
  gsl_block_long_double * block;
  int owner;
} gsl_matrix_long_double;

typedef struct
{
  gsl_matrix_long_double matrix;
} _gsl_matrix_long_double_view;

typedef _gsl_matrix_long_double_view gsl_matrix_long_double_view;

typedef struct
{
  gsl_matrix_long_double matrix;
} _gsl_matrix_long_double_const_view;

typedef const _gsl_matrix_long_double_const_view gsl_matrix_long_double_const_view;

/* Allocation */

gsl_matrix_long_double * 
gsl_matrix_long_double_alloc (const size_t n1, const size_t n2);

gsl_matrix_long_double * 
gsl_matrix_long_double_calloc (const size_t n1, const size_t n2);

gsl_matrix_long_double * 
gsl_matrix_long_double_alloc_from_block (gsl_block_long_double * b, 
                                   const size_t offset, 
                                   const size_t n1, 
                                   const size_t n2, 
                                   const size_t d2);

gsl_matrix_long_double * 
gsl_matrix_long_double_alloc_from_matrix (gsl_matrix_long_double * m,
                                    const size_t k1, 
                                    const size_t k2,
                                    const size_t n1, 
                                    const size_t n2);

gsl_vector_long_double * 
gsl_vector_long_double_alloc_row_from_matrix (gsl_matrix_long_double * m,
                                        const size_t i);

gsl_vector_long_double * 
gsl_vector_long_double_alloc_col_from_matrix (gsl_matrix_long_double * m,
                                        const size_t j);

void gsl_matrix_long_double_free (gsl_matrix_long_double * m);

/* Views */

_gsl_matrix_long_double_view 
gsl_matrix_long_double_submatrix (gsl_matrix_long_double * m, 
                            const size_t i, const size_t j, 
                            const size_t n1, const size_t n2);

_gsl_vector_long_double_view 
gsl_matrix_long_double_row (gsl_matrix_long_double * m, const size_t i);

_gsl_vector_long_double_view 
gsl_matrix_long_double_column (gsl_matrix_long_double * m, const size_t j);

_gsl_vector_long_double_view 
gsl_matrix_long_double_diagonal (gsl_matrix_long_double * m);

_gsl_vector_long_double_view 
gsl_matrix_long_double_subdiagonal (gsl_matrix_long_double * m, const size_t k);

_gsl_vector_long_double_view 
gsl_matrix_long_double_superdiagonal (gsl_matrix_long_double * m, const size_t k);

_gsl_vector_long_double_view
gsl_matrix_long_double_subrow (gsl_matrix_long_double * m, const size_t i,
                         const size_t offset, const size_t n);

_gsl_vector_long_double_view
gsl_matrix_long_double_subcolumn (gsl_matrix_long_double * m, const size_t j,
                            const size_t offset, const size_t n);

_gsl_matrix_long_double_view
gsl_matrix_long_double_view_array (long double * base,
                             const size_t n1, 
                             const size_t n2);

_gsl_matrix_long_double_view
gsl_matrix_long_double_view_array_with_tda (long double * base, 
                                      const size_t n1, 
                                      const size_t n2,
                                      const size_t tda);


_gsl_matrix_long_double_view
gsl_matrix_long_double_view_vector (gsl_vector_long_double * v,
                              const size_t n1, 
                              const size_t n2);

_gsl_matrix_long_double_view
gsl_matrix_long_double_view_vector_with_tda (gsl_vector_long_double * v,
                                       const size_t n1, 
                                       const size_t n2,
                                       const size_t tda);


_gsl_matrix_long_double_const_view 
gsl_matrix_long_double_const_submatrix (const gsl_matrix_long_double * m, 
                                  const size_t i, const size_t j, 
                                  const size_t n1, const size_t n2);

_gsl_vector_long_double_const_view 
gsl_matrix_long_double_const_row (const gsl_matrix_long_double * m, 
                            const size_t i);

_gsl_vector_long_double_const_view 
gsl_matrix_long_double_const_column (const gsl_matrix_long_double * m, 
                               const size_t j);

_gsl_vector_long_double_const_view
gsl_matrix_long_double_const_diagonal (const gsl_matrix_long_double * m);

_gsl_vector_long_double_const_view 
gsl_matrix_long_double_const_subdiagonal (const gsl_matrix_long_double * m, 
                                    const size_t k);

_gsl_vector_long_double_const_view 
gsl_matrix_long_double_const_superdiagonal (const gsl_matrix_long_double * m, 
                                      const size_t k);

_gsl_vector_long_double_const_view
gsl_matrix_long_double_const_subrow (const gsl_matrix_long_double * m, const size_t i,
                               const size_t offset, const size_t n);

_gsl_vector_long_double_const_view
gsl_matrix_long_double_const_subcolumn (const gsl_matrix_long_double * m, const size_t j,
                                  const size_t offset, const size_t n);

_gsl_matrix_long_double_const_view
gsl_matrix_long_double_const_view_array (const long double * base,
                                   const size_t n1, 
                                   const size_t n2);

_gsl_matrix_long_double_const_view
gsl_matrix_long_double_const_view_array_with_tda (const long double * base, 
                                            const size_t n1, 
                                            const size_t n2,
                                            const size_t tda);

_gsl_matrix_long_double_const_view
gsl_matrix_long_double_const_view_vector (const gsl_vector_long_double * v,
                                    const size_t n1, 
                                    const size_t n2);

_gsl_matrix_long_double_const_view
gsl_matrix_long_double_const_view_vector_with_tda (const gsl_vector_long_double * v,
                                             const size_t n1, 
                                             const size_t n2,
                                             const size_t tda);

/* Operations */

void gsl_matrix_long_double_set_zero (gsl_matrix_long_double * m);
void gsl_matrix_long_double_set_identity (gsl_matrix_long_double * m);
void gsl_matrix_long_double_set_all (gsl_matrix_long_double * m, long double x);

int gsl_matrix_long_double_fread (FILE * stream, gsl_matrix_long_double * m) ;
int gsl_matrix_long_double_fwrite (FILE * stream, const gsl_matrix_long_double * m) ;
int gsl_matrix_long_double_fscanf (FILE * stream, gsl_matrix_long_double * m);
int gsl_matrix_long_double_fprintf (FILE * stream, const gsl_matrix_long_double * m, const char * format);
 
int gsl_matrix_long_double_memcpy(gsl_matrix_long_double * dest, const gsl_matrix_long_double * src);
int gsl_matrix_long_double_swap(gsl_matrix_long_double * m1, gsl_matrix_long_double * m2);

int gsl_matrix_long_double_swap_rows(gsl_matrix_long_double * m, const size_t i, const size_t j);
int gsl_matrix_long_double_swap_columns(gsl_matrix_long_double * m, const size_t i, const size_t j);
int gsl_matrix_long_double_swap_rowcol(gsl_matrix_long_double * m, const size_t i, const size_t j);
int gsl_matrix_long_double_transpose (gsl_matrix_long_double * m);
int gsl_matrix_long_double_transpose_memcpy (gsl_matrix_long_double * dest, const gsl_matrix_long_double * src);

long double gsl_matrix_long_double_max (const gsl_matrix_long_double * m);
long double gsl_matrix_long_double_min (const gsl_matrix_long_double * m);
void gsl_matrix_long_double_minmax (const gsl_matrix_long_double * m, long double * min_out, long double * max_out);

void gsl_matrix_long_double_max_index (const gsl_matrix_long_double * m, size_t * imax, size_t *jmax);
void gsl_matrix_long_double_min_index (const gsl_matrix_long_double * m, size_t * imin, size_t *jmin);
void gsl_matrix_long_double_minmax_index (const gsl_matrix_long_double * m, size_t * imin, size_t * jmin, size_t * imax, size_t * jmax);

int gsl_matrix_long_double_equal (const gsl_matrix_long_double * a, const gsl_matrix_long_double * b);

int gsl_matrix_long_double_isnull (const gsl_matrix_long_double * m);
int gsl_matrix_long_double_ispos (const gsl_matrix_long_double * m);
int gsl_matrix_long_double_isneg (const gsl_matrix_long_double * m);
int gsl_matrix_long_double_isnonneg (const gsl_matrix_long_double * m);

int gsl_matrix_long_double_add (gsl_matrix_long_double * a, const gsl_matrix_long_double * b);
int gsl_matrix_long_double_sub (gsl_matrix_long_double * a, const gsl_matrix_long_double * b);
int gsl_matrix_long_double_mul_elements (gsl_matrix_long_double * a, const gsl_matrix_long_double * b);
int gsl_matrix_long_double_div_elements (gsl_matrix_long_double * a, const gsl_matrix_long_double * b);
int gsl_matrix_long_double_scale (gsl_matrix_long_double * a, const double x);
int gsl_matrix_long_double_add_constant (gsl_matrix_long_double * a, const double x);
int gsl_matrix_long_double_add_diagonal (gsl_matrix_long_double * a, const double x);

/***********************************************************************/
/* The functions below are obsolete                                    */
/***********************************************************************/
int gsl_matrix_long_double_get_row(gsl_vector_long_double * v, const gsl_matrix_long_double * m, const size_t i);
int gsl_matrix_long_double_get_col(gsl_vector_long_double * v, const gsl_matrix_long_double * m, const size_t j);
int gsl_matrix_long_double_set_row(gsl_matrix_long_double * m, const size_t i, const gsl_vector_long_double * v);
int gsl_matrix_long_double_set_col(gsl_matrix_long_double * m, const size_t j, const gsl_vector_long_double * v);
/***********************************************************************/

/* inline functions if you are using GCC */

INLINE_DECL long double   gsl_matrix_long_double_get(const gsl_matrix_long_double * m, const size_t i, const size_t j);
INLINE_DECL void    gsl_matrix_long_double_set(gsl_matrix_long_double * m, const size_t i, const size_t j, const long double x);
INLINE_DECL long double * gsl_matrix_long_double_ptr(gsl_matrix_long_double * m, const size_t i, const size_t j);
INLINE_DECL const long double * gsl_matrix_long_double_const_ptr(const gsl_matrix_long_double * m, const size_t i, const size_t j);

#ifdef HAVE_INLINE
INLINE_FUN 
long double
gsl_matrix_long_double_get(const gsl_matrix_long_double * m, const size_t i, const size_t j)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(1)) 
    {
      if (i >= m->size1)
        {
          GSL_ERROR_VAL("first index out of range", GSL_EINVAL, 0) ;
        }
      else if (j >= m->size2)
        {
          GSL_ERROR_VAL("second index out of range", GSL_EINVAL, 0) ;
        }
    }
#endif
  return m->data[i * m->tda + j] ;
} 

INLINE_FUN 
void
gsl_matrix_long_double_set(gsl_matrix_long_double * m, const size_t i, const size_t j, const long double x)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(1)) 
    {
      if (i >= m->size1)
        {
          GSL_ERROR_VOID("first index out of range", GSL_EINVAL) ;
        }
      else if (j >= m->size2)
        {
          GSL_ERROR_VOID("second index out of range", GSL_EINVAL) ;
        }
    }
#endif
  m->data[i * m->tda + j] = x ;
}

INLINE_FUN 
long double *
gsl_matrix_long_double_ptr(gsl_matrix_long_double * m, const size_t i, const size_t j)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(1)) 
    {
      if (i >= m->size1)
        {
          GSL_ERROR_NULL("first index out of range", GSL_EINVAL) ;
        }
      else if (j >= m->size2)
        {
          GSL_ERROR_NULL("second index out of range", GSL_EINVAL) ;
        }
    }
#endif
  return (long double *) (m->data + (i * m->tda + j)) ;
} 

INLINE_FUN 
const long double *
gsl_matrix_long_double_const_ptr(const gsl_matrix_long_double * m, const size_t i, const size_t j)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(1)) 
    {
      if (i >= m->size1)
        {
          GSL_ERROR_NULL("first index out of range", GSL_EINVAL) ;
        }
      else if (j >= m->size2)
        {
          GSL_ERROR_NULL("second index out of range", GSL_EINVAL) ;
        }
    }
#endif
  return (const long double *) (m->data + (i * m->tda + j)) ;
} 

#endif

__END_DECLS

#endif /* __GSL_MATRIX_LONG_DOUBLE_H__ */
/* matrix/gsl_matrix_double.h
 * 
 * Copyright (C) 1996, 1997, 1998, 1999, 2000, 2007 Gerard Jungman, Brian Gough
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __GSL_MATRIX_DOUBLE_H__
#define __GSL_MATRIX_DOUBLE_H__

#include <stdlib.h>

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif

__BEGIN_DECLS

typedef struct 
{
  size_t size1;
  size_t size2;
  size_t tda;
  double * data;
  gsl_block * block;
  int owner;
} gsl_matrix;

typedef struct
{
  gsl_matrix matrix;
} _gsl_matrix_view;

typedef _gsl_matrix_view gsl_matrix_view;

typedef struct
{
  gsl_matrix matrix;
} _gsl_matrix_const_view;

typedef const _gsl_matrix_const_view gsl_matrix_const_view;

/* Allocation */

gsl_matrix * 
gsl_matrix_alloc (const size_t n1, const size_t n2);

gsl_matrix * 
gsl_matrix_calloc (const size_t n1, const size_t n2);

gsl_matrix * 
gsl_matrix_alloc_from_block (gsl_block * b, 
                                   const size_t offset, 
                                   const size_t n1, 
                                   const size_t n2, 
                                   const size_t d2);

gsl_matrix * 
gsl_matrix_alloc_from_matrix (gsl_matrix * m,
                                    const size_t k1, 
                                    const size_t k2,
                                    const size_t n1, 
                                    const size_t n2);

gsl_vector * 
gsl_vector_alloc_row_from_matrix (gsl_matrix * m,
                                        const size_t i);

gsl_vector * 
gsl_vector_alloc_col_from_matrix (gsl_matrix * m,
                                        const size_t j);

void gsl_matrix_free (gsl_matrix * m);

/* Views */

_gsl_matrix_view 
gsl_matrix_submatrix (gsl_matrix * m, 
                            const size_t i, const size_t j, 
                            const size_t n1, const size_t n2);

_gsl_vector_view 
gsl_matrix_row (gsl_matrix * m, const size_t i);

_gsl_vector_view 
gsl_matrix_column (gsl_matrix * m, const size_t j);

_gsl_vector_view 
gsl_matrix_diagonal (gsl_matrix * m);

_gsl_vector_view 
gsl_matrix_subdiagonal (gsl_matrix * m, const size_t k);

_gsl_vector_view 
gsl_matrix_superdiagonal (gsl_matrix * m, const size_t k);

_gsl_vector_view
gsl_matrix_subrow (gsl_matrix * m, const size_t i,
                         const size_t offset, const size_t n);

_gsl_vector_view
gsl_matrix_subcolumn (gsl_matrix * m, const size_t j,
                            const size_t offset, const size_t n);

_gsl_matrix_view
gsl_matrix_view_array (double * base,
                             const size_t n1, 
                             const size_t n2);

_gsl_matrix_view
gsl_matrix_view_array_with_tda (double * base, 
                                      const size_t n1, 
                                      const size_t n2,
                                      const size_t tda);


_gsl_matrix_view
gsl_matrix_view_vector (gsl_vector * v,
                              const size_t n1, 
                              const size_t n2);

_gsl_matrix_view
gsl_matrix_view_vector_with_tda (gsl_vector * v,
                                       const size_t n1, 
                                       const size_t n2,
                                       const size_t tda);


_gsl_matrix_const_view 
gsl_matrix_const_submatrix (const gsl_matrix * m, 
                                  const size_t i, const size_t j, 
                                  const size_t n1, const size_t n2);

_gsl_vector_const_view 
gsl_matrix_const_row (const gsl_matrix * m, 
                            const size_t i);

_gsl_vector_const_view 
gsl_matrix_const_column (const gsl_matrix * m, 
                               const size_t j);

_gsl_vector_const_view
gsl_matrix_const_diagonal (const gsl_matrix * m);

_gsl_vector_const_view 
gsl_matrix_const_subdiagonal (const gsl_matrix * m, 
                                    const size_t k);

_gsl_vector_const_view 
gsl_matrix_const_superdiagonal (const gsl_matrix * m, 
                                      const size_t k);

_gsl_vector_const_view
gsl_matrix_const_subrow (const gsl_matrix * m, const size_t i,
                               const size_t offset, const size_t n);

_gsl_vector_const_view
gsl_matrix_const_subcolumn (const gsl_matrix * m, const size_t j,
                                  const size_t offset, const size_t n);

_gsl_matrix_const_view
gsl_matrix_const_view_array (const double * base,
                                   const size_t n1, 
                                   const size_t n2);

_gsl_matrix_const_view
gsl_matrix_const_view_array_with_tda (const double * base, 
                                            const size_t n1, 
                                            const size_t n2,
                                            const size_t tda);

_gsl_matrix_const_view
gsl_matrix_const_view_vector (const gsl_vector * v,
                                    const size_t n1, 
                                    const size_t n2);

_gsl_matrix_const_view
gsl_matrix_const_view_vector_with_tda (const gsl_vector * v,
                                             const size_t n1, 
                                             const size_t n2,
                                             const size_t tda);

/* Operations */

void gsl_matrix_set_zero (gsl_matrix * m);
void gsl_matrix_set_identity (gsl_matrix * m);
void gsl_matrix_set_all (gsl_matrix * m, double x);

int gsl_matrix_fread (FILE * stream, gsl_matrix * m) ;
int gsl_matrix_fwrite (FILE * stream, const gsl_matrix * m) ;
int gsl_matrix_fscanf (FILE * stream, gsl_matrix * m);
int gsl_matrix_fprintf (FILE * stream, const gsl_matrix * m, const char * format);
 
int gsl_matrix_memcpy(gsl_matrix * dest, const gsl_matrix * src);
int gsl_matrix_swap(gsl_matrix * m1, gsl_matrix * m2);

int gsl_matrix_swap_rows(gsl_matrix * m, const size_t i, const size_t j);
int gsl_matrix_swap_columns(gsl_matrix * m, const size_t i, const size_t j);
int gsl_matrix_swap_rowcol(gsl_matrix * m, const size_t i, const size_t j);
int gsl_matrix_transpose (gsl_matrix * m);
int gsl_matrix_transpose_memcpy (gsl_matrix * dest, const gsl_matrix * src);

double gsl_matrix_max (const gsl_matrix * m);
double gsl_matrix_min (const gsl_matrix * m);
void gsl_matrix_minmax (const gsl_matrix * m, double * min_out, double * max_out);

void gsl_matrix_max_index (const gsl_matrix * m, size_t * imax, size_t *jmax);
void gsl_matrix_min_index (const gsl_matrix * m, size_t * imin, size_t *jmin);
void gsl_matrix_minmax_index (const gsl_matrix * m, size_t * imin, size_t * jmin, size_t * imax, size_t * jmax);

int gsl_matrix_equal (const gsl_matrix * a, const gsl_matrix * b);

int gsl_matrix_isnull (const gsl_matrix * m);
int gsl_matrix_ispos (const gsl_matrix * m);
int gsl_matrix_isneg (const gsl_matrix * m);
int gsl_matrix_isnonneg (const gsl_matrix * m);

int gsl_matrix_add (gsl_matrix * a, const gsl_matrix * b);
int gsl_matrix_sub (gsl_matrix * a, const gsl_matrix * b);
int gsl_matrix_mul_elements (gsl_matrix * a, const gsl_matrix * b);
int gsl_matrix_div_elements (gsl_matrix * a, const gsl_matrix * b);
int gsl_matrix_scale (gsl_matrix * a, const double x);
int gsl_matrix_add_constant (gsl_matrix * a, const double x);
int gsl_matrix_add_diagonal (gsl_matrix * a, const double x);

/***********************************************************************/
/* The functions below are obsolete                                    */
/***********************************************************************/
int gsl_matrix_get_row(gsl_vector * v, const gsl_matrix * m, const size_t i);
int gsl_matrix_get_col(gsl_vector * v, const gsl_matrix * m, const size_t j);
int gsl_matrix_set_row(gsl_matrix * m, const size_t i, const gsl_vector * v);
int gsl_matrix_set_col(gsl_matrix * m, const size_t j, const gsl_vector * v);
/***********************************************************************/

/* inline functions if you are using GCC */

INLINE_DECL double   gsl_matrix_get(const gsl_matrix * m, const size_t i, const size_t j);
INLINE_DECL void    gsl_matrix_set(gsl_matrix * m, const size_t i, const size_t j, const double x);
INLINE_DECL double * gsl_matrix_ptr(gsl_matrix * m, const size_t i, const size_t j);
INLINE_DECL const double * gsl_matrix_const_ptr(const gsl_matrix * m, const size_t i, const size_t j);

#ifdef HAVE_INLINE
INLINE_FUN 
double
gsl_matrix_get(const gsl_matrix * m, const size_t i, const size_t j)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(1)) 
    {
      if (i >= m->size1)
        {
          GSL_ERROR_VAL("first index out of range", GSL_EINVAL, 0) ;
        }
      else if (j >= m->size2)
        {
          GSL_ERROR_VAL("second index out of range", GSL_EINVAL, 0) ;
        }
    }
#endif
  return m->data[i * m->tda + j] ;
} 

INLINE_FUN 
void
gsl_matrix_set(gsl_matrix * m, const size_t i, const size_t j, const double x)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(1)) 
    {
      if (i >= m->size1)
        {
          GSL_ERROR_VOID("first index out of range", GSL_EINVAL) ;
        }
      else if (j >= m->size2)
        {
          GSL_ERROR_VOID("second index out of range", GSL_EINVAL) ;
        }
    }
#endif
  m->data[i * m->tda + j] = x ;
}

INLINE_FUN 
double *
gsl_matrix_ptr(gsl_matrix * m, const size_t i, const size_t j)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(1)) 
    {
      if (i >= m->size1)
        {
          GSL_ERROR_NULL("first index out of range", GSL_EINVAL) ;
        }
      else if (j >= m->size2)
        {
          GSL_ERROR_NULL("second index out of range", GSL_EINVAL) ;
        }
    }
#endif
  return (double *) (m->data + (i * m->tda + j)) ;
} 

INLINE_FUN 
const double *
gsl_matrix_const_ptr(const gsl_matrix * m, const size_t i, const size_t j)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(1)) 
    {
      if (i >= m->size1)
        {
          GSL_ERROR_NULL("first index out of range", GSL_EINVAL) ;
        }
      else if (j >= m->size2)
        {
          GSL_ERROR_NULL("second index out of range", GSL_EINVAL) ;
        }
    }
#endif
  return (const double *) (m->data + (i * m->tda + j)) ;
} 

#endif

__END_DECLS

#endif /* __GSL_MATRIX_DOUBLE_H__ */
/* matrix/gsl_matrix_float.h
 * 
 * Copyright (C) 1996, 1997, 1998, 1999, 2000, 2007 Gerard Jungman, Brian Gough
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __GSL_MATRIX_FLOAT_H__
#define __GSL_MATRIX_FLOAT_H__

#include <stdlib.h>

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif

__BEGIN_DECLS

typedef struct 
{
  size_t size1;
  size_t size2;
  size_t tda;
  float * data;
  gsl_block_float * block;
  int owner;
} gsl_matrix_float;

typedef struct
{
  gsl_matrix_float matrix;
} _gsl_matrix_float_view;

typedef _gsl_matrix_float_view gsl_matrix_float_view;

typedef struct
{
  gsl_matrix_float matrix;
} _gsl_matrix_float_const_view;

typedef const _gsl_matrix_float_const_view gsl_matrix_float_const_view;

/* Allocation */

gsl_matrix_float * 
gsl_matrix_float_alloc (const size_t n1, const size_t n2);

gsl_matrix_float * 
gsl_matrix_float_calloc (const size_t n1, const size_t n2);

gsl_matrix_float * 
gsl_matrix_float_alloc_from_block (gsl_block_float * b, 
                                   const size_t offset, 
                                   const size_t n1, 
                                   const size_t n2, 
                                   const size_t d2);

gsl_matrix_float * 
gsl_matrix_float_alloc_from_matrix (gsl_matrix_float * m,
                                    const size_t k1, 
                                    const size_t k2,
                                    const size_t n1, 
                                    const size_t n2);

gsl_vector_float * 
gsl_vector_float_alloc_row_from_matrix (gsl_matrix_float * m,
                                        const size_t i);

gsl_vector_float * 
gsl_vector_float_alloc_col_from_matrix (gsl_matrix_float * m,
                                        const size_t j);

void gsl_matrix_float_free (gsl_matrix_float * m);

/* Views */

_gsl_matrix_float_view 
gsl_matrix_float_submatrix (gsl_matrix_float * m, 
                            const size_t i, const size_t j, 
                            const size_t n1, const size_t n2);

_gsl_vector_float_view 
gsl_matrix_float_row (gsl_matrix_float * m, const size_t i);

_gsl_vector_float_view 
gsl_matrix_float_column (gsl_matrix_float * m, const size_t j);

_gsl_vector_float_view 
gsl_matrix_float_diagonal (gsl_matrix_float * m);

_gsl_vector_float_view 
gsl_matrix_float_subdiagonal (gsl_matrix_float * m, const size_t k);

_gsl_vector_float_view 
gsl_matrix_float_superdiagonal (gsl_matrix_float * m, const size_t k);

_gsl_vector_float_view
gsl_matrix_float_subrow (gsl_matrix_float * m, const size_t i,
                         const size_t offset, const size_t n);

_gsl_vector_float_view
gsl_matrix_float_subcolumn (gsl_matrix_float * m, const size_t j,
                            const size_t offset, const size_t n);

_gsl_matrix_float_view
gsl_matrix_float_view_array (float * base,
                             const size_t n1, 
                             const size_t n2);

_gsl_matrix_float_view
gsl_matrix_float_view_array_with_tda (float * base, 
                                      const size_t n1, 
                                      const size_t n2,
                                      const size_t tda);


_gsl_matrix_float_view
gsl_matrix_float_view_vector (gsl_vector_float * v,
                              const size_t n1, 
                              const size_t n2);

_gsl_matrix_float_view
gsl_matrix_float_view_vector_with_tda (gsl_vector_float * v,
                                       const size_t n1, 
                                       const size_t n2,
                                       const size_t tda);


_gsl_matrix_float_const_view 
gsl_matrix_float_const_submatrix (const gsl_matrix_float * m, 
                                  const size_t i, const size_t j, 
                                  const size_t n1, const size_t n2);

_gsl_vector_float_const_view 
gsl_matrix_float_const_row (const gsl_matrix_float * m, 
                            const size_t i);

_gsl_vector_float_const_view 
gsl_matrix_float_const_column (const gsl_matrix_float * m, 
                               const size_t j);

_gsl_vector_float_const_view
gsl_matrix_float_const_diagonal (const gsl_matrix_float * m);

_gsl_vector_float_const_view 
gsl_matrix_float_const_subdiagonal (const gsl_matrix_float * m, 
                                    const size_t k);

_gsl_vector_float_const_view 
gsl_matrix_float_const_superdiagonal (const gsl_matrix_float * m, 
                                      const size_t k);

_gsl_vector_float_const_view
gsl_matrix_float_const_subrow (const gsl_matrix_float * m, const size_t i,
                               const size_t offset, const size_t n);

_gsl_vector_float_const_view
gsl_matrix_float_const_subcolumn (const gsl_matrix_float * m, const size_t j,
                                  const size_t offset, const size_t n);

_gsl_matrix_float_const_view
gsl_matrix_float_const_view_array (const float * base,
                                   const size_t n1, 
                                   const size_t n2);

_gsl_matrix_float_const_view
gsl_matrix_float_const_view_array_with_tda (const float * base, 
                                            const size_t n1, 
                                            const size_t n2,
                                            const size_t tda);

_gsl_matrix_float_const_view
gsl_matrix_float_const_view_vector (const gsl_vector_float * v,
                                    const size_t n1, 
                                    const size_t n2);

_gsl_matrix_float_const_view
gsl_matrix_float_const_view_vector_with_tda (const gsl_vector_float * v,
                                             const size_t n1, 
                                             const size_t n2,
                                             const size_t tda);

/* Operations */

void gsl_matrix_float_set_zero (gsl_matrix_float * m);
void gsl_matrix_float_set_identity (gsl_matrix_float * m);
void gsl_matrix_float_set_all (gsl_matrix_float * m, float x);

int gsl_matrix_float_fread (FILE * stream, gsl_matrix_float * m) ;
int gsl_matrix_float_fwrite (FILE * stream, const gsl_matrix_float * m) ;
int gsl_matrix_float_fscanf (FILE * stream, gsl_matrix_float * m);
int gsl_matrix_float_fprintf (FILE * stream, const gsl_matrix_float * m, const char * format);
 
int gsl_matrix_float_memcpy(gsl_matrix_float * dest, const gsl_matrix_float * src);
int gsl_matrix_float_swap(gsl_matrix_float * m1, gsl_matrix_float * m2);

int gsl_matrix_float_swap_rows(gsl_matrix_float * m, const size_t i, const size_t j);
int gsl_matrix_float_swap_columns(gsl_matrix_float * m, const size_t i, const size_t j);
int gsl_matrix_float_swap_rowcol(gsl_matrix_float * m, const size_t i, const size_t j);
int gsl_matrix_float_transpose (gsl_matrix_float * m);
int gsl_matrix_float_transpose_memcpy (gsl_matrix_float * dest, const gsl_matrix_float * src);

float gsl_matrix_float_max (const gsl_matrix_float * m);
float gsl_matrix_float_min (const gsl_matrix_float * m);
void gsl_matrix_float_minmax (const gsl_matrix_float * m, float * min_out, float * max_out);

void gsl_matrix_float_max_index (const gsl_matrix_float * m, size_t * imax, size_t *jmax);
void gsl_matrix_float_min_index (const gsl_matrix_float * m, size_t * imin, size_t *jmin);
void gsl_matrix_float_minmax_index (const gsl_matrix_float * m, size_t * imin, size_t * jmin, size_t * imax, size_t * jmax);

int gsl_matrix_float_equal (const gsl_matrix_float * a, const gsl_matrix_float * b);

int gsl_matrix_float_isnull (const gsl_matrix_float * m);
int gsl_matrix_float_ispos (const gsl_matrix_float * m);
int gsl_matrix_float_isneg (const gsl_matrix_float * m);
int gsl_matrix_float_isnonneg (const gsl_matrix_float * m);

int gsl_matrix_float_add (gsl_matrix_float * a, const gsl_matrix_float * b);
int gsl_matrix_float_sub (gsl_matrix_float * a, const gsl_matrix_float * b);
int gsl_matrix_float_mul_elements (gsl_matrix_float * a, const gsl_matrix_float * b);
int gsl_matrix_float_div_elements (gsl_matrix_float * a, const gsl_matrix_float * b);
int gsl_matrix_float_scale (gsl_matrix_float * a, const double x);
int gsl_matrix_float_add_constant (gsl_matrix_float * a, const double x);
int gsl_matrix_float_add_diagonal (gsl_matrix_float * a, const double x);

/***********************************************************************/
/* The functions below are obsolete                                    */
/***********************************************************************/
int gsl_matrix_float_get_row(gsl_vector_float * v, const gsl_matrix_float * m, const size_t i);
int gsl_matrix_float_get_col(gsl_vector_float * v, const gsl_matrix_float * m, const size_t j);
int gsl_matrix_float_set_row(gsl_matrix_float * m, const size_t i, const gsl_vector_float * v);
int gsl_matrix_float_set_col(gsl_matrix_float * m, const size_t j, const gsl_vector_float * v);
/***********************************************************************/

/* inline functions if you are using GCC */

INLINE_DECL float   gsl_matrix_float_get(const gsl_matrix_float * m, const size_t i, const size_t j);
INLINE_DECL void    gsl_matrix_float_set(gsl_matrix_float * m, const size_t i, const size_t j, const float x);
INLINE_DECL float * gsl_matrix_float_ptr(gsl_matrix_float * m, const size_t i, const size_t j);
INLINE_DECL const float * gsl_matrix_float_const_ptr(const gsl_matrix_float * m, const size_t i, const size_t j);

#ifdef HAVE_INLINE
INLINE_FUN 
float
gsl_matrix_float_get(const gsl_matrix_float * m, const size_t i, const size_t j)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(1)) 
    {
      if (i >= m->size1)
        {
          GSL_ERROR_VAL("first index out of range", GSL_EINVAL, 0) ;
        }
      else if (j >= m->size2)
        {
          GSL_ERROR_VAL("second index out of range", GSL_EINVAL, 0) ;
        }
    }
#endif
  return m->data[i * m->tda + j] ;
} 

INLINE_FUN 
void
gsl_matrix_float_set(gsl_matrix_float * m, const size_t i, const size_t j, const float x)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(1)) 
    {
      if (i >= m->size1)
        {
          GSL_ERROR_VOID("first index out of range", GSL_EINVAL) ;
        }
      else if (j >= m->size2)
        {
          GSL_ERROR_VOID("second index out of range", GSL_EINVAL) ;
        }
    }
#endif
  m->data[i * m->tda + j] = x ;
}

INLINE_FUN 
float *
gsl_matrix_float_ptr(gsl_matrix_float * m, const size_t i, const size_t j)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(1)) 
    {
      if (i >= m->size1)
        {
          GSL_ERROR_NULL("first index out of range", GSL_EINVAL) ;
        }
      else if (j >= m->size2)
        {
          GSL_ERROR_NULL("second index out of range", GSL_EINVAL) ;
        }
    }
#endif
  return (float *) (m->data + (i * m->tda + j)) ;
} 

INLINE_FUN 
const float *
gsl_matrix_float_const_ptr(const gsl_matrix_float * m, const size_t i, const size_t j)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(1)) 
    {
      if (i >= m->size1)
        {
          GSL_ERROR_NULL("first index out of range", GSL_EINVAL) ;
        }
      else if (j >= m->size2)
        {
          GSL_ERROR_NULL("second index out of range", GSL_EINVAL) ;
        }
    }
#endif
  return (const float *) (m->data + (i * m->tda + j)) ;
} 

#endif

__END_DECLS

#endif /* __GSL_MATRIX_FLOAT_H__ */

/* matrix/gsl_matrix_ulong.h
 * 
 * Copyright (C) 1996, 1997, 1998, 1999, 2000, 2007 Gerard Jungman, Brian Gough
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __GSL_MATRIX_ULONG_H__
#define __GSL_MATRIX_ULONG_H__

#include <stdlib.h>

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif

__BEGIN_DECLS

typedef struct 
{
  size_t size1;
  size_t size2;
  size_t tda;
  unsigned long * data;
  gsl_block_ulong * block;
  int owner;
} gsl_matrix_ulong;

typedef struct
{
  gsl_matrix_ulong matrix;
} _gsl_matrix_ulong_view;

typedef _gsl_matrix_ulong_view gsl_matrix_ulong_view;

typedef struct
{
  gsl_matrix_ulong matrix;
} _gsl_matrix_ulong_const_view;

typedef const _gsl_matrix_ulong_const_view gsl_matrix_ulong_const_view;

/* Allocation */

gsl_matrix_ulong * 
gsl_matrix_ulong_alloc (const size_t n1, const size_t n2);

gsl_matrix_ulong * 
gsl_matrix_ulong_calloc (const size_t n1, const size_t n2);

gsl_matrix_ulong * 
gsl_matrix_ulong_alloc_from_block (gsl_block_ulong * b, 
                                   const size_t offset, 
                                   const size_t n1, 
                                   const size_t n2, 
                                   const size_t d2);

gsl_matrix_ulong * 
gsl_matrix_ulong_alloc_from_matrix (gsl_matrix_ulong * m,
                                    const size_t k1, 
                                    const size_t k2,
                                    const size_t n1, 
                                    const size_t n2);

gsl_vector_ulong * 
gsl_vector_ulong_alloc_row_from_matrix (gsl_matrix_ulong * m,
                                        const size_t i);

gsl_vector_ulong * 
gsl_vector_ulong_alloc_col_from_matrix (gsl_matrix_ulong * m,
                                        const size_t j);

void gsl_matrix_ulong_free (gsl_matrix_ulong * m);

/* Views */

_gsl_matrix_ulong_view 
gsl_matrix_ulong_submatrix (gsl_matrix_ulong * m, 
                            const size_t i, const size_t j, 
                            const size_t n1, const size_t n2);

_gsl_vector_ulong_view 
gsl_matrix_ulong_row (gsl_matrix_ulong * m, const size_t i);

_gsl_vector_ulong_view 
gsl_matrix_ulong_column (gsl_matrix_ulong * m, const size_t j);

_gsl_vector_ulong_view 
gsl_matrix_ulong_diagonal (gsl_matrix_ulong * m);

_gsl_vector_ulong_view 
gsl_matrix_ulong_subdiagonal (gsl_matrix_ulong * m, const size_t k);

_gsl_vector_ulong_view 
gsl_matrix_ulong_superdiagonal (gsl_matrix_ulong * m, const size_t k);

_gsl_vector_ulong_view
gsl_matrix_ulong_subrow (gsl_matrix_ulong * m, const size_t i,
                         const size_t offset, const size_t n);

_gsl_vector_ulong_view
gsl_matrix_ulong_subcolumn (gsl_matrix_ulong * m, const size_t j,
                            const size_t offset, const size_t n);

_gsl_matrix_ulong_view
gsl_matrix_ulong_view_array (unsigned long * base,
                             const size_t n1, 
                             const size_t n2);

_gsl_matrix_ulong_view
gsl_matrix_ulong_view_array_with_tda (unsigned long * base, 
                                      const size_t n1, 
                                      const size_t n2,
                                      const size_t tda);


_gsl_matrix_ulong_view
gsl_matrix_ulong_view_vector (gsl_vector_ulong * v,
                              const size_t n1, 
                              const size_t n2);

_gsl_matrix_ulong_view
gsl_matrix_ulong_view_vector_with_tda (gsl_vector_ulong * v,
                                       const size_t n1, 
                                       const size_t n2,
                                       const size_t tda);


_gsl_matrix_ulong_const_view 
gsl_matrix_ulong_const_submatrix (const gsl_matrix_ulong * m, 
                                  const size_t i, const size_t j, 
                                  const size_t n1, const size_t n2);

_gsl_vector_ulong_const_view 
gsl_matrix_ulong_const_row (const gsl_matrix_ulong * m, 
                            const size_t i);

_gsl_vector_ulong_const_view 
gsl_matrix_ulong_const_column (const gsl_matrix_ulong * m, 
                               const size_t j);

_gsl_vector_ulong_const_view
gsl_matrix_ulong_const_diagonal (const gsl_matrix_ulong * m);

_gsl_vector_ulong_const_view 
gsl_matrix_ulong_const_subdiagonal (const gsl_matrix_ulong * m, 
                                    const size_t k);

_gsl_vector_ulong_const_view 
gsl_matrix_ulong_const_superdiagonal (const gsl_matrix_ulong * m, 
                                      const size_t k);

_gsl_vector_ulong_const_view
gsl_matrix_ulong_const_subrow (const gsl_matrix_ulong * m, const size_t i,
                               const size_t offset, const size_t n);

_gsl_vector_ulong_const_view
gsl_matrix_ulong_const_subcolumn (const gsl_matrix_ulong * m, const size_t j,
                                  const size_t offset, const size_t n);

_gsl_matrix_ulong_const_view
gsl_matrix_ulong_const_view_array (const unsigned long * base,
                                   const size_t n1, 
                                   const size_t n2);

_gsl_matrix_ulong_const_view
gsl_matrix_ulong_const_view_array_with_tda (const unsigned long * base, 
                                            const size_t n1, 
                                            const size_t n2,
                                            const size_t tda);

_gsl_matrix_ulong_const_view
gsl_matrix_ulong_const_view_vector (const gsl_vector_ulong * v,
                                    const size_t n1, 
                                    const size_t n2);

_gsl_matrix_ulong_const_view
gsl_matrix_ulong_const_view_vector_with_tda (const gsl_vector_ulong * v,
                                             const size_t n1, 
                                             const size_t n2,
                                             const size_t tda);

/* Operations */

void gsl_matrix_ulong_set_zero (gsl_matrix_ulong * m);
void gsl_matrix_ulong_set_identity (gsl_matrix_ulong * m);
void gsl_matrix_ulong_set_all (gsl_matrix_ulong * m, unsigned long x);

int gsl_matrix_ulong_fread (FILE * stream, gsl_matrix_ulong * m) ;
int gsl_matrix_ulong_fwrite (FILE * stream, const gsl_matrix_ulong * m) ;
int gsl_matrix_ulong_fscanf (FILE * stream, gsl_matrix_ulong * m);
int gsl_matrix_ulong_fprintf (FILE * stream, const gsl_matrix_ulong * m, const char * format);
 
int gsl_matrix_ulong_memcpy(gsl_matrix_ulong * dest, const gsl_matrix_ulong * src);
int gsl_matrix_ulong_swap(gsl_matrix_ulong * m1, gsl_matrix_ulong * m2);

int gsl_matrix_ulong_swap_rows(gsl_matrix_ulong * m, const size_t i, const size_t j);
int gsl_matrix_ulong_swap_columns(gsl_matrix_ulong * m, const size_t i, const size_t j);
int gsl_matrix_ulong_swap_rowcol(gsl_matrix_ulong * m, const size_t i, const size_t j);
int gsl_matrix_ulong_transpose (gsl_matrix_ulong * m);
int gsl_matrix_ulong_transpose_memcpy (gsl_matrix_ulong * dest, const gsl_matrix_ulong * src);

unsigned long gsl_matrix_ulong_max (const gsl_matrix_ulong * m);
unsigned long gsl_matrix_ulong_min (const gsl_matrix_ulong * m);
void gsl_matrix_ulong_minmax (const gsl_matrix_ulong * m, unsigned long * min_out, unsigned long * max_out);

void gsl_matrix_ulong_max_index (const gsl_matrix_ulong * m, size_t * imax, size_t *jmax);
void gsl_matrix_ulong_min_index (const gsl_matrix_ulong * m, size_t * imin, size_t *jmin);
void gsl_matrix_ulong_minmax_index (const gsl_matrix_ulong * m, size_t * imin, size_t * jmin, size_t * imax, size_t * jmax);

int gsl_matrix_ulong_equal (const gsl_matrix_ulong * a, const gsl_matrix_ulong * b);

int gsl_matrix_ulong_isnull (const gsl_matrix_ulong * m);
int gsl_matrix_ulong_ispos (const gsl_matrix_ulong * m);
int gsl_matrix_ulong_isneg (const gsl_matrix_ulong * m);
int gsl_matrix_ulong_isnonneg (const gsl_matrix_ulong * m);

int gsl_matrix_ulong_add (gsl_matrix_ulong * a, const gsl_matrix_ulong * b);
int gsl_matrix_ulong_sub (gsl_matrix_ulong * a, const gsl_matrix_ulong * b);
int gsl_matrix_ulong_mul_elements (gsl_matrix_ulong * a, const gsl_matrix_ulong * b);
int gsl_matrix_ulong_div_elements (gsl_matrix_ulong * a, const gsl_matrix_ulong * b);
int gsl_matrix_ulong_scale (gsl_matrix_ulong * a, const double x);
int gsl_matrix_ulong_add_constant (gsl_matrix_ulong * a, const double x);
int gsl_matrix_ulong_add_diagonal (gsl_matrix_ulong * a, const double x);

/***********************************************************************/
/* The functions below are obsolete                                    */
/***********************************************************************/
int gsl_matrix_ulong_get_row(gsl_vector_ulong * v, const gsl_matrix_ulong * m, const size_t i);
int gsl_matrix_ulong_get_col(gsl_vector_ulong * v, const gsl_matrix_ulong * m, const size_t j);
int gsl_matrix_ulong_set_row(gsl_matrix_ulong * m, const size_t i, const gsl_vector_ulong * v);
int gsl_matrix_ulong_set_col(gsl_matrix_ulong * m, const size_t j, const gsl_vector_ulong * v);
/***********************************************************************/

/* inline functions if you are using GCC */

INLINE_DECL unsigned long   gsl_matrix_ulong_get(const gsl_matrix_ulong * m, const size_t i, const size_t j);
INLINE_DECL void    gsl_matrix_ulong_set(gsl_matrix_ulong * m, const size_t i, const size_t j, const unsigned long x);
INLINE_DECL unsigned long * gsl_matrix_ulong_ptr(gsl_matrix_ulong * m, const size_t i, const size_t j);
INLINE_DECL const unsigned long * gsl_matrix_ulong_const_ptr(const gsl_matrix_ulong * m, const size_t i, const size_t j);

#ifdef HAVE_INLINE
INLINE_FUN 
unsigned long
gsl_matrix_ulong_get(const gsl_matrix_ulong * m, const size_t i, const size_t j)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(1)) 
    {
      if (i >= m->size1)
        {
          GSL_ERROR_VAL("first index out of range", GSL_EINVAL, 0) ;
        }
      else if (j >= m->size2)
        {
          GSL_ERROR_VAL("second index out of range", GSL_EINVAL, 0) ;
        }
    }
#endif
  return m->data[i * m->tda + j] ;
} 

INLINE_FUN 
void
gsl_matrix_ulong_set(gsl_matrix_ulong * m, const size_t i, const size_t j, const unsigned long x)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(1)) 
    {
      if (i >= m->size1)
        {
          GSL_ERROR_VOID("first index out of range", GSL_EINVAL) ;
        }
      else if (j >= m->size2)
        {
          GSL_ERROR_VOID("second index out of range", GSL_EINVAL) ;
        }
    }
#endif
  m->data[i * m->tda + j] = x ;
}

INLINE_FUN 
unsigned long *
gsl_matrix_ulong_ptr(gsl_matrix_ulong * m, const size_t i, const size_t j)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(1)) 
    {
      if (i >= m->size1)
        {
          GSL_ERROR_NULL("first index out of range", GSL_EINVAL) ;
        }
      else if (j >= m->size2)
        {
          GSL_ERROR_NULL("second index out of range", GSL_EINVAL) ;
        }
    }
#endif
  return (unsigned long *) (m->data + (i * m->tda + j)) ;
} 

INLINE_FUN 
const unsigned long *
gsl_matrix_ulong_const_ptr(const gsl_matrix_ulong * m, const size_t i, const size_t j)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(1)) 
    {
      if (i >= m->size1)
        {
          GSL_ERROR_NULL("first index out of range", GSL_EINVAL) ;
        }
      else if (j >= m->size2)
        {
          GSL_ERROR_NULL("second index out of range", GSL_EINVAL) ;
        }
    }
#endif
  return (const unsigned long *) (m->data + (i * m->tda + j)) ;
} 

#endif

__END_DECLS

#endif /* __GSL_MATRIX_ULONG_H__ */
/* matrix/gsl_matrix_long.h
 * 
 * Copyright (C) 1996, 1997, 1998, 1999, 2000, 2007 Gerard Jungman, Brian Gough
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __GSL_MATRIX_LONG_H__
#define __GSL_MATRIX_LONG_H__

#include <stdlib.h>

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif

__BEGIN_DECLS

typedef struct 
{
  size_t size1;
  size_t size2;
  size_t tda;
  long * data;
  gsl_block_long * block;
  int owner;
} gsl_matrix_long;

typedef struct
{
  gsl_matrix_long matrix;
} _gsl_matrix_long_view;

typedef _gsl_matrix_long_view gsl_matrix_long_view;

typedef struct
{
  gsl_matrix_long matrix;
} _gsl_matrix_long_const_view;

typedef const _gsl_matrix_long_const_view gsl_matrix_long_const_view;

/* Allocation */

gsl_matrix_long * 
gsl_matrix_long_alloc (const size_t n1, const size_t n2);

gsl_matrix_long * 
gsl_matrix_long_calloc (const size_t n1, const size_t n2);

gsl_matrix_long * 
gsl_matrix_long_alloc_from_block (gsl_block_long * b, 
                                   const size_t offset, 
                                   const size_t n1, 
                                   const size_t n2, 
                                   const size_t d2);

gsl_matrix_long * 
gsl_matrix_long_alloc_from_matrix (gsl_matrix_long * m,
                                    const size_t k1, 
                                    const size_t k2,
                                    const size_t n1, 
                                    const size_t n2);

gsl_vector_long * 
gsl_vector_long_alloc_row_from_matrix (gsl_matrix_long * m,
                                        const size_t i);

gsl_vector_long * 
gsl_vector_long_alloc_col_from_matrix (gsl_matrix_long * m,
                                        const size_t j);

void gsl_matrix_long_free (gsl_matrix_long * m);

/* Views */

_gsl_matrix_long_view 
gsl_matrix_long_submatrix (gsl_matrix_long * m, 
                            const size_t i, const size_t j, 
                            const size_t n1, const size_t n2);

_gsl_vector_long_view 
gsl_matrix_long_row (gsl_matrix_long * m, const size_t i);

_gsl_vector_long_view 
gsl_matrix_long_column (gsl_matrix_long * m, const size_t j);

_gsl_vector_long_view 
gsl_matrix_long_diagonal (gsl_matrix_long * m);

_gsl_vector_long_view 
gsl_matrix_long_subdiagonal (gsl_matrix_long * m, const size_t k);

_gsl_vector_long_view 
gsl_matrix_long_superdiagonal (gsl_matrix_long * m, const size_t k);

_gsl_vector_long_view
gsl_matrix_long_subrow (gsl_matrix_long * m, const size_t i,
                         const size_t offset, const size_t n);

_gsl_vector_long_view
gsl_matrix_long_subcolumn (gsl_matrix_long * m, const size_t j,
                            const size_t offset, const size_t n);

_gsl_matrix_long_view
gsl_matrix_long_view_array (long * base,
                             const size_t n1, 
                             const size_t n2);

_gsl_matrix_long_view
gsl_matrix_long_view_array_with_tda (long * base, 
                                      const size_t n1, 
                                      const size_t n2,
                                      const size_t tda);


_gsl_matrix_long_view
gsl_matrix_long_view_vector (gsl_vector_long * v,
                              const size_t n1, 
                              const size_t n2);

_gsl_matrix_long_view
gsl_matrix_long_view_vector_with_tda (gsl_vector_long * v,
                                       const size_t n1, 
                                       const size_t n2,
                                       const size_t tda);


_gsl_matrix_long_const_view 
gsl_matrix_long_const_submatrix (const gsl_matrix_long * m, 
                                  const size_t i, const size_t j, 
                                  const size_t n1, const size_t n2);

_gsl_vector_long_const_view 
gsl_matrix_long_const_row (const gsl_matrix_long * m, 
                            const size_t i);

_gsl_vector_long_const_view 
gsl_matrix_long_const_column (const gsl_matrix_long * m, 
                               const size_t j);

_gsl_vector_long_const_view
gsl_matrix_long_const_diagonal (const gsl_matrix_long * m);

_gsl_vector_long_const_view 
gsl_matrix_long_const_subdiagonal (const gsl_matrix_long * m, 
                                    const size_t k);

_gsl_vector_long_const_view 
gsl_matrix_long_const_superdiagonal (const gsl_matrix_long * m, 
                                      const size_t k);

_gsl_vector_long_const_view
gsl_matrix_long_const_subrow (const gsl_matrix_long * m, const size_t i,
                               const size_t offset, const size_t n);

_gsl_vector_long_const_view
gsl_matrix_long_const_subcolumn (const gsl_matrix_long * m, const size_t j,
                                  const size_t offset, const size_t n);

_gsl_matrix_long_const_view
gsl_matrix_long_const_view_array (const long * base,
                                   const size_t n1, 
                                   const size_t n2);

_gsl_matrix_long_const_view
gsl_matrix_long_const_view_array_with_tda (const long * base, 
                                            const size_t n1, 
                                            const size_t n2,
                                            const size_t tda);

_gsl_matrix_long_const_view
gsl_matrix_long_const_view_vector (const gsl_vector_long * v,
                                    const size_t n1, 
                                    const size_t n2);

_gsl_matrix_long_const_view
gsl_matrix_long_const_view_vector_with_tda (const gsl_vector_long * v,
                                             const size_t n1, 
                                             const size_t n2,
                                             const size_t tda);

/* Operations */

void gsl_matrix_long_set_zero (gsl_matrix_long * m);
void gsl_matrix_long_set_identity (gsl_matrix_long * m);
void gsl_matrix_long_set_all (gsl_matrix_long * m, long x);

int gsl_matrix_long_fread (FILE * stream, gsl_matrix_long * m) ;
int gsl_matrix_long_fwrite (FILE * stream, const gsl_matrix_long * m) ;
int gsl_matrix_long_fscanf (FILE * stream, gsl_matrix_long * m);
int gsl_matrix_long_fprintf (FILE * stream, const gsl_matrix_long * m, const char * format);
 
int gsl_matrix_long_memcpy(gsl_matrix_long * dest, const gsl_matrix_long * src);
int gsl_matrix_long_swap(gsl_matrix_long * m1, gsl_matrix_long * m2);

int gsl_matrix_long_swap_rows(gsl_matrix_long * m, const size_t i, const size_t j);
int gsl_matrix_long_swap_columns(gsl_matrix_long * m, const size_t i, const size_t j);
int gsl_matrix_long_swap_rowcol(gsl_matrix_long * m, const size_t i, const size_t j);
int gsl_matrix_long_transpose (gsl_matrix_long * m);
int gsl_matrix_long_transpose_memcpy (gsl_matrix_long * dest, const gsl_matrix_long * src);

long gsl_matrix_long_max (const gsl_matrix_long * m);
long gsl_matrix_long_min (const gsl_matrix_long * m);
void gsl_matrix_long_minmax (const gsl_matrix_long * m, long * min_out, long * max_out);

void gsl_matrix_long_max_index (const gsl_matrix_long * m, size_t * imax, size_t *jmax);
void gsl_matrix_long_min_index (const gsl_matrix_long * m, size_t * imin, size_t *jmin);
void gsl_matrix_long_minmax_index (const gsl_matrix_long * m, size_t * imin, size_t * jmin, size_t * imax, size_t * jmax);

int gsl_matrix_long_equal (const gsl_matrix_long * a, const gsl_matrix_long * b);

int gsl_matrix_long_isnull (const gsl_matrix_long * m);
int gsl_matrix_long_ispos (const gsl_matrix_long * m);
int gsl_matrix_long_isneg (const gsl_matrix_long * m);
int gsl_matrix_long_isnonneg (const gsl_matrix_long * m);

int gsl_matrix_long_add (gsl_matrix_long * a, const gsl_matrix_long * b);
int gsl_matrix_long_sub (gsl_matrix_long * a, const gsl_matrix_long * b);
int gsl_matrix_long_mul_elements (gsl_matrix_long * a, const gsl_matrix_long * b);
int gsl_matrix_long_div_elements (gsl_matrix_long * a, const gsl_matrix_long * b);
int gsl_matrix_long_scale (gsl_matrix_long * a, const double x);
int gsl_matrix_long_add_constant (gsl_matrix_long * a, const double x);
int gsl_matrix_long_add_diagonal (gsl_matrix_long * a, const double x);

/***********************************************************************/
/* The functions below are obsolete                                    */
/***********************************************************************/
int gsl_matrix_long_get_row(gsl_vector_long * v, const gsl_matrix_long * m, const size_t i);
int gsl_matrix_long_get_col(gsl_vector_long * v, const gsl_matrix_long * m, const size_t j);
int gsl_matrix_long_set_row(gsl_matrix_long * m, const size_t i, const gsl_vector_long * v);
int gsl_matrix_long_set_col(gsl_matrix_long * m, const size_t j, const gsl_vector_long * v);
/***********************************************************************/

/* inline functions if you are using GCC */

INLINE_DECL long   gsl_matrix_long_get(const gsl_matrix_long * m, const size_t i, const size_t j);
INLINE_DECL void    gsl_matrix_long_set(gsl_matrix_long * m, const size_t i, const size_t j, const long x);
INLINE_DECL long * gsl_matrix_long_ptr(gsl_matrix_long * m, const size_t i, const size_t j);
INLINE_DECL const long * gsl_matrix_long_const_ptr(const gsl_matrix_long * m, const size_t i, const size_t j);

#ifdef HAVE_INLINE
INLINE_FUN 
long
gsl_matrix_long_get(const gsl_matrix_long * m, const size_t i, const size_t j)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(1)) 
    {
      if (i >= m->size1)
        {
          GSL_ERROR_VAL("first index out of range", GSL_EINVAL, 0) ;
        }
      else if (j >= m->size2)
        {
          GSL_ERROR_VAL("second index out of range", GSL_EINVAL, 0) ;
        }
    }
#endif
  return m->data[i * m->tda + j] ;
} 

INLINE_FUN 
void
gsl_matrix_long_set(gsl_matrix_long * m, const size_t i, const size_t j, const long x)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(1)) 
    {
      if (i >= m->size1)
        {
          GSL_ERROR_VOID("first index out of range", GSL_EINVAL) ;
        }
      else if (j >= m->size2)
        {
          GSL_ERROR_VOID("second index out of range", GSL_EINVAL) ;
        }
    }
#endif
  m->data[i * m->tda + j] = x ;
}

INLINE_FUN 
long *
gsl_matrix_long_ptr(gsl_matrix_long * m, const size_t i, const size_t j)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(1)) 
    {
      if (i >= m->size1)
        {
          GSL_ERROR_NULL("first index out of range", GSL_EINVAL) ;
        }
      else if (j >= m->size2)
        {
          GSL_ERROR_NULL("second index out of range", GSL_EINVAL) ;
        }
    }
#endif
  return (long *) (m->data + (i * m->tda + j)) ;
} 

INLINE_FUN 
const long *
gsl_matrix_long_const_ptr(const gsl_matrix_long * m, const size_t i, const size_t j)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(1)) 
    {
      if (i >= m->size1)
        {
          GSL_ERROR_NULL("first index out of range", GSL_EINVAL) ;
        }
      else if (j >= m->size2)
        {
          GSL_ERROR_NULL("second index out of range", GSL_EINVAL) ;
        }
    }
#endif
  return (const long *) (m->data + (i * m->tda + j)) ;
} 

#endif

__END_DECLS

#endif /* __GSL_MATRIX_LONG_H__ */

/* matrix/gsl_matrix_uint.h
 * 
 * Copyright (C) 1996, 1997, 1998, 1999, 2000, 2007 Gerard Jungman, Brian Gough
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __GSL_MATRIX_UINT_H__
#define __GSL_MATRIX_UINT_H__

#include <stdlib.h>

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif

__BEGIN_DECLS

typedef struct 
{
  size_t size1;
  size_t size2;
  size_t tda;
  unsigned int * data;
  gsl_block_uint * block;
  int owner;
} gsl_matrix_uint;

typedef struct
{
  gsl_matrix_uint matrix;
} _gsl_matrix_uint_view;

typedef _gsl_matrix_uint_view gsl_matrix_uint_view;

typedef struct
{
  gsl_matrix_uint matrix;
} _gsl_matrix_uint_const_view;

typedef const _gsl_matrix_uint_const_view gsl_matrix_uint_const_view;

/* Allocation */

gsl_matrix_uint * 
gsl_matrix_uint_alloc (const size_t n1, const size_t n2);

gsl_matrix_uint * 
gsl_matrix_uint_calloc (const size_t n1, const size_t n2);

gsl_matrix_uint * 
gsl_matrix_uint_alloc_from_block (gsl_block_uint * b, 
                                   const size_t offset, 
                                   const size_t n1, 
                                   const size_t n2, 
                                   const size_t d2);

gsl_matrix_uint * 
gsl_matrix_uint_alloc_from_matrix (gsl_matrix_uint * m,
                                    const size_t k1, 
                                    const size_t k2,
                                    const size_t n1, 
                                    const size_t n2);

gsl_vector_uint * 
gsl_vector_uint_alloc_row_from_matrix (gsl_matrix_uint * m,
                                        const size_t i);

gsl_vector_uint * 
gsl_vector_uint_alloc_col_from_matrix (gsl_matrix_uint * m,
                                        const size_t j);

void gsl_matrix_uint_free (gsl_matrix_uint * m);

/* Views */

_gsl_matrix_uint_view 
gsl_matrix_uint_submatrix (gsl_matrix_uint * m, 
                            const size_t i, const size_t j, 
                            const size_t n1, const size_t n2);

_gsl_vector_uint_view 
gsl_matrix_uint_row (gsl_matrix_uint * m, const size_t i);

_gsl_vector_uint_view 
gsl_matrix_uint_column (gsl_matrix_uint * m, const size_t j);

_gsl_vector_uint_view 
gsl_matrix_uint_diagonal (gsl_matrix_uint * m);

_gsl_vector_uint_view 
gsl_matrix_uint_subdiagonal (gsl_matrix_uint * m, const size_t k);

_gsl_vector_uint_view 
gsl_matrix_uint_superdiagonal (gsl_matrix_uint * m, const size_t k);

_gsl_vector_uint_view
gsl_matrix_uint_subrow (gsl_matrix_uint * m, const size_t i,
                         const size_t offset, const size_t n);

_gsl_vector_uint_view
gsl_matrix_uint_subcolumn (gsl_matrix_uint * m, const size_t j,
                            const size_t offset, const size_t n);

_gsl_matrix_uint_view
gsl_matrix_uint_view_array (unsigned int * base,
                             const size_t n1, 
                             const size_t n2);

_gsl_matrix_uint_view
gsl_matrix_uint_view_array_with_tda (unsigned int * base, 
                                      const size_t n1, 
                                      const size_t n2,
                                      const size_t tda);


_gsl_matrix_uint_view
gsl_matrix_uint_view_vector (gsl_vector_uint * v,
                              const size_t n1, 
                              const size_t n2);

_gsl_matrix_uint_view
gsl_matrix_uint_view_vector_with_tda (gsl_vector_uint * v,
                                       const size_t n1, 
                                       const size_t n2,
                                       const size_t tda);


_gsl_matrix_uint_const_view 
gsl_matrix_uint_const_submatrix (const gsl_matrix_uint * m, 
                                  const size_t i, const size_t j, 
                                  const size_t n1, const size_t n2);

_gsl_vector_uint_const_view 
gsl_matrix_uint_const_row (const gsl_matrix_uint * m, 
                            const size_t i);

_gsl_vector_uint_const_view 
gsl_matrix_uint_const_column (const gsl_matrix_uint * m, 
                               const size_t j);

_gsl_vector_uint_const_view
gsl_matrix_uint_const_diagonal (const gsl_matrix_uint * m);

_gsl_vector_uint_const_view 
gsl_matrix_uint_const_subdiagonal (const gsl_matrix_uint * m, 
                                    const size_t k);

_gsl_vector_uint_const_view 
gsl_matrix_uint_const_superdiagonal (const gsl_matrix_uint * m, 
                                      const size_t k);

_gsl_vector_uint_const_view
gsl_matrix_uint_const_subrow (const gsl_matrix_uint * m, const size_t i,
                               const size_t offset, const size_t n);

_gsl_vector_uint_const_view
gsl_matrix_uint_const_subcolumn (const gsl_matrix_uint * m, const size_t j,
                                  const size_t offset, const size_t n);

_gsl_matrix_uint_const_view
gsl_matrix_uint_const_view_array (const unsigned int * base,
                                   const size_t n1, 
                                   const size_t n2);

_gsl_matrix_uint_const_view
gsl_matrix_uint_const_view_array_with_tda (const unsigned int * base, 
                                            const size_t n1, 
                                            const size_t n2,
                                            const size_t tda);

_gsl_matrix_uint_const_view
gsl_matrix_uint_const_view_vector (const gsl_vector_uint * v,
                                    const size_t n1, 
                                    const size_t n2);

_gsl_matrix_uint_const_view
gsl_matrix_uint_const_view_vector_with_tda (const gsl_vector_uint * v,
                                             const size_t n1, 
                                             const size_t n2,
                                             const size_t tda);

/* Operations */

void gsl_matrix_uint_set_zero (gsl_matrix_uint * m);
void gsl_matrix_uint_set_identity (gsl_matrix_uint * m);
void gsl_matrix_uint_set_all (gsl_matrix_uint * m, unsigned int x);

int gsl_matrix_uint_fread (FILE * stream, gsl_matrix_uint * m) ;
int gsl_matrix_uint_fwrite (FILE * stream, const gsl_matrix_uint * m) ;
int gsl_matrix_uint_fscanf (FILE * stream, gsl_matrix_uint * m);
int gsl_matrix_uint_fprintf (FILE * stream, const gsl_matrix_uint * m, const char * format);
 
int gsl_matrix_uint_memcpy(gsl_matrix_uint * dest, const gsl_matrix_uint * src);
int gsl_matrix_uint_swap(gsl_matrix_uint * m1, gsl_matrix_uint * m2);

int gsl_matrix_uint_swap_rows(gsl_matrix_uint * m, const size_t i, const size_t j);
int gsl_matrix_uint_swap_columns(gsl_matrix_uint * m, const size_t i, const size_t j);
int gsl_matrix_uint_swap_rowcol(gsl_matrix_uint * m, const size_t i, const size_t j);
int gsl_matrix_uint_transpose (gsl_matrix_uint * m);
int gsl_matrix_uint_transpose_memcpy (gsl_matrix_uint * dest, const gsl_matrix_uint * src);

unsigned int gsl_matrix_uint_max (const gsl_matrix_uint * m);
unsigned int gsl_matrix_uint_min (const gsl_matrix_uint * m);
void gsl_matrix_uint_minmax (const gsl_matrix_uint * m, unsigned int * min_out, unsigned int * max_out);

void gsl_matrix_uint_max_index (const gsl_matrix_uint * m, size_t * imax, size_t *jmax);
void gsl_matrix_uint_min_index (const gsl_matrix_uint * m, size_t * imin, size_t *jmin);
void gsl_matrix_uint_minmax_index (const gsl_matrix_uint * m, size_t * imin, size_t * jmin, size_t * imax, size_t * jmax);

int gsl_matrix_uint_equal (const gsl_matrix_uint * a, const gsl_matrix_uint * b);

int gsl_matrix_uint_isnull (const gsl_matrix_uint * m);
int gsl_matrix_uint_ispos (const gsl_matrix_uint * m);
int gsl_matrix_uint_isneg (const gsl_matrix_uint * m);
int gsl_matrix_uint_isnonneg (const gsl_matrix_uint * m);

int gsl_matrix_uint_add (gsl_matrix_uint * a, const gsl_matrix_uint * b);
int gsl_matrix_uint_sub (gsl_matrix_uint * a, const gsl_matrix_uint * b);
int gsl_matrix_uint_mul_elements (gsl_matrix_uint * a, const gsl_matrix_uint * b);
int gsl_matrix_uint_div_elements (gsl_matrix_uint * a, const gsl_matrix_uint * b);
int gsl_matrix_uint_scale (gsl_matrix_uint * a, const double x);
int gsl_matrix_uint_add_constant (gsl_matrix_uint * a, const double x);
int gsl_matrix_uint_add_diagonal (gsl_matrix_uint * a, const double x);

/***********************************************************************/
/* The functions below are obsolete                                    */
/***********************************************************************/
int gsl_matrix_uint_get_row(gsl_vector_uint * v, const gsl_matrix_uint * m, const size_t i);
int gsl_matrix_uint_get_col(gsl_vector_uint * v, const gsl_matrix_uint * m, const size_t j);
int gsl_matrix_uint_set_row(gsl_matrix_uint * m, const size_t i, const gsl_vector_uint * v);
int gsl_matrix_uint_set_col(gsl_matrix_uint * m, const size_t j, const gsl_vector_uint * v);
/***********************************************************************/

/* inline functions if you are using GCC */

INLINE_DECL unsigned int   gsl_matrix_uint_get(const gsl_matrix_uint * m, const size_t i, const size_t j);
INLINE_DECL void    gsl_matrix_uint_set(gsl_matrix_uint * m, const size_t i, const size_t j, const unsigned int x);
INLINE_DECL unsigned int * gsl_matrix_uint_ptr(gsl_matrix_uint * m, const size_t i, const size_t j);
INLINE_DECL const unsigned int * gsl_matrix_uint_const_ptr(const gsl_matrix_uint * m, const size_t i, const size_t j);

#ifdef HAVE_INLINE
INLINE_FUN 
unsigned int
gsl_matrix_uint_get(const gsl_matrix_uint * m, const size_t i, const size_t j)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(1)) 
    {
      if (i >= m->size1)
        {
          GSL_ERROR_VAL("first index out of range", GSL_EINVAL, 0) ;
        }
      else if (j >= m->size2)
        {
          GSL_ERROR_VAL("second index out of range", GSL_EINVAL, 0) ;
        }
    }
#endif
  return m->data[i * m->tda + j] ;
} 

INLINE_FUN 
void
gsl_matrix_uint_set(gsl_matrix_uint * m, const size_t i, const size_t j, const unsigned int x)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(1)) 
    {
      if (i >= m->size1)
        {
          GSL_ERROR_VOID("first index out of range", GSL_EINVAL) ;
        }
      else if (j >= m->size2)
        {
          GSL_ERROR_VOID("second index out of range", GSL_EINVAL) ;
        }
    }
#endif
  m->data[i * m->tda + j] = x ;
}

INLINE_FUN 
unsigned int *
gsl_matrix_uint_ptr(gsl_matrix_uint * m, const size_t i, const size_t j)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(1)) 
    {
      if (i >= m->size1)
        {
          GSL_ERROR_NULL("first index out of range", GSL_EINVAL) ;
        }
      else if (j >= m->size2)
        {
          GSL_ERROR_NULL("second index out of range", GSL_EINVAL) ;
        }
    }
#endif
  return (unsigned int *) (m->data + (i * m->tda + j)) ;
} 

INLINE_FUN 
const unsigned int *
gsl_matrix_uint_const_ptr(const gsl_matrix_uint * m, const size_t i, const size_t j)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(1)) 
    {
      if (i >= m->size1)
        {
          GSL_ERROR_NULL("first index out of range", GSL_EINVAL) ;
        }
      else if (j >= m->size2)
        {
          GSL_ERROR_NULL("second index out of range", GSL_EINVAL) ;
        }
    }
#endif
  return (const unsigned int *) (m->data + (i * m->tda + j)) ;
} 

#endif

__END_DECLS

#endif /* __GSL_MATRIX_UINT_H__ */
/* matrix/gsl_matrix_int.h
 * 
 * Copyright (C) 1996, 1997, 1998, 1999, 2000, 2007 Gerard Jungman, Brian Gough
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __GSL_MATRIX_INT_H__
#define __GSL_MATRIX_INT_H__

#include <stdlib.h>

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif

__BEGIN_DECLS

typedef struct 
{
  size_t size1;
  size_t size2;
  size_t tda;
  int * data;
  gsl_block_int * block;
  int owner;
} gsl_matrix_int;

typedef struct
{
  gsl_matrix_int matrix;
} _gsl_matrix_int_view;

typedef _gsl_matrix_int_view gsl_matrix_int_view;

typedef struct
{
  gsl_matrix_int matrix;
} _gsl_matrix_int_const_view;

typedef const _gsl_matrix_int_const_view gsl_matrix_int_const_view;

/* Allocation */

gsl_matrix_int * 
gsl_matrix_int_alloc (const size_t n1, const size_t n2);

gsl_matrix_int * 
gsl_matrix_int_calloc (const size_t n1, const size_t n2);

gsl_matrix_int * 
gsl_matrix_int_alloc_from_block (gsl_block_int * b, 
                                   const size_t offset, 
                                   const size_t n1, 
                                   const size_t n2, 
                                   const size_t d2);

gsl_matrix_int * 
gsl_matrix_int_alloc_from_matrix (gsl_matrix_int * m,
                                    const size_t k1, 
                                    const size_t k2,
                                    const size_t n1, 
                                    const size_t n2);

gsl_vector_int * 
gsl_vector_int_alloc_row_from_matrix (gsl_matrix_int * m,
                                        const size_t i);

gsl_vector_int * 
gsl_vector_int_alloc_col_from_matrix (gsl_matrix_int * m,
                                        const size_t j);

void gsl_matrix_int_free (gsl_matrix_int * m);

/* Views */

_gsl_matrix_int_view 
gsl_matrix_int_submatrix (gsl_matrix_int * m, 
                            const size_t i, const size_t j, 
                            const size_t n1, const size_t n2);

_gsl_vector_int_view 
gsl_matrix_int_row (gsl_matrix_int * m, const size_t i);

_gsl_vector_int_view 
gsl_matrix_int_column (gsl_matrix_int * m, const size_t j);

_gsl_vector_int_view 
gsl_matrix_int_diagonal (gsl_matrix_int * m);

_gsl_vector_int_view 
gsl_matrix_int_subdiagonal (gsl_matrix_int * m, const size_t k);

_gsl_vector_int_view 
gsl_matrix_int_superdiagonal (gsl_matrix_int * m, const size_t k);

_gsl_vector_int_view
gsl_matrix_int_subrow (gsl_matrix_int * m, const size_t i,
                         const size_t offset, const size_t n);

_gsl_vector_int_view
gsl_matrix_int_subcolumn (gsl_matrix_int * m, const size_t j,
                            const size_t offset, const size_t n);

_gsl_matrix_int_view
gsl_matrix_int_view_array (int * base,
                             const size_t n1, 
                             const size_t n2);

_gsl_matrix_int_view
gsl_matrix_int_view_array_with_tda (int * base, 
                                      const size_t n1, 
                                      const size_t n2,
                                      const size_t tda);


_gsl_matrix_int_view
gsl_matrix_int_view_vector (gsl_vector_int * v,
                              const size_t n1, 
                              const size_t n2);

_gsl_matrix_int_view
gsl_matrix_int_view_vector_with_tda (gsl_vector_int * v,
                                       const size_t n1, 
                                       const size_t n2,
                                       const size_t tda);


_gsl_matrix_int_const_view 
gsl_matrix_int_const_submatrix (const gsl_matrix_int * m, 
                                  const size_t i, const size_t j, 
                                  const size_t n1, const size_t n2);

_gsl_vector_int_const_view 
gsl_matrix_int_const_row (const gsl_matrix_int * m, 
                            const size_t i);

_gsl_vector_int_const_view 
gsl_matrix_int_const_column (const gsl_matrix_int * m, 
                               const size_t j);

_gsl_vector_int_const_view
gsl_matrix_int_const_diagonal (const gsl_matrix_int * m);

_gsl_vector_int_const_view 
gsl_matrix_int_const_subdiagonal (const gsl_matrix_int * m, 
                                    const size_t k);

_gsl_vector_int_const_view 
gsl_matrix_int_const_superdiagonal (const gsl_matrix_int * m, 
                                      const size_t k);

_gsl_vector_int_const_view
gsl_matrix_int_const_subrow (const gsl_matrix_int * m, const size_t i,
                               const size_t offset, const size_t n);

_gsl_vector_int_const_view
gsl_matrix_int_const_subcolumn (const gsl_matrix_int * m, const size_t j,
                                  const size_t offset, const size_t n);

_gsl_matrix_int_const_view
gsl_matrix_int_const_view_array (const int * base,
                                   const size_t n1, 
                                   const size_t n2);

_gsl_matrix_int_const_view
gsl_matrix_int_const_view_array_with_tda (const int * base, 
                                            const size_t n1, 
                                            const size_t n2,
                                            const size_t tda);

_gsl_matrix_int_const_view
gsl_matrix_int_const_view_vector (const gsl_vector_int * v,
                                    const size_t n1, 
                                    const size_t n2);

_gsl_matrix_int_const_view
gsl_matrix_int_const_view_vector_with_tda (const gsl_vector_int * v,
                                             const size_t n1, 
                                             const size_t n2,
                                             const size_t tda);

/* Operations */

void gsl_matrix_int_set_zero (gsl_matrix_int * m);
void gsl_matrix_int_set_identity (gsl_matrix_int * m);
void gsl_matrix_int_set_all (gsl_matrix_int * m, int x);

int gsl_matrix_int_fread (FILE * stream, gsl_matrix_int * m) ;
int gsl_matrix_int_fwrite (FILE * stream, const gsl_matrix_int * m) ;
int gsl_matrix_int_fscanf (FILE * stream, gsl_matrix_int * m);
int gsl_matrix_int_fprintf (FILE * stream, const gsl_matrix_int * m, const char * format);
 
int gsl_matrix_int_memcpy(gsl_matrix_int * dest, const gsl_matrix_int * src);
int gsl_matrix_int_swap(gsl_matrix_int * m1, gsl_matrix_int * m2);

int gsl_matrix_int_swap_rows(gsl_matrix_int * m, const size_t i, const size_t j);
int gsl_matrix_int_swap_columns(gsl_matrix_int * m, const size_t i, const size_t j);
int gsl_matrix_int_swap_rowcol(gsl_matrix_int * m, const size_t i, const size_t j);
int gsl_matrix_int_transpose (gsl_matrix_int * m);
int gsl_matrix_int_transpose_memcpy (gsl_matrix_int * dest, const gsl_matrix_int * src);

int gsl_matrix_int_max (const gsl_matrix_int * m);
int gsl_matrix_int_min (const gsl_matrix_int * m);
void gsl_matrix_int_minmax (const gsl_matrix_int * m, int * min_out, int * max_out);

void gsl_matrix_int_max_index (const gsl_matrix_int * m, size_t * imax, size_t *jmax);
void gsl_matrix_int_min_index (const gsl_matrix_int * m, size_t * imin, size_t *jmin);
void gsl_matrix_int_minmax_index (const gsl_matrix_int * m, size_t * imin, size_t * jmin, size_t * imax, size_t * jmax);

int gsl_matrix_int_equal (const gsl_matrix_int * a, const gsl_matrix_int * b);

int gsl_matrix_int_isnull (const gsl_matrix_int * m);
int gsl_matrix_int_ispos (const gsl_matrix_int * m);
int gsl_matrix_int_isneg (const gsl_matrix_int * m);
int gsl_matrix_int_isnonneg (const gsl_matrix_int * m);

int gsl_matrix_int_add (gsl_matrix_int * a, const gsl_matrix_int * b);
int gsl_matrix_int_sub (gsl_matrix_int * a, const gsl_matrix_int * b);
int gsl_matrix_int_mul_elements (gsl_matrix_int * a, const gsl_matrix_int * b);
int gsl_matrix_int_div_elements (gsl_matrix_int * a, const gsl_matrix_int * b);
int gsl_matrix_int_scale (gsl_matrix_int * a, const double x);
int gsl_matrix_int_add_constant (gsl_matrix_int * a, const double x);
int gsl_matrix_int_add_diagonal (gsl_matrix_int * a, const double x);

/***********************************************************************/
/* The functions below are obsolete                                    */
/***********************************************************************/
int gsl_matrix_int_get_row(gsl_vector_int * v, const gsl_matrix_int * m, const size_t i);
int gsl_matrix_int_get_col(gsl_vector_int * v, const gsl_matrix_int * m, const size_t j);
int gsl_matrix_int_set_row(gsl_matrix_int * m, const size_t i, const gsl_vector_int * v);
int gsl_matrix_int_set_col(gsl_matrix_int * m, const size_t j, const gsl_vector_int * v);
/***********************************************************************/

/* inline functions if you are using GCC */

INLINE_DECL int   gsl_matrix_int_get(const gsl_matrix_int * m, const size_t i, const size_t j);
INLINE_DECL void    gsl_matrix_int_set(gsl_matrix_int * m, const size_t i, const size_t j, const int x);
INLINE_DECL int * gsl_matrix_int_ptr(gsl_matrix_int * m, const size_t i, const size_t j);
INLINE_DECL const int * gsl_matrix_int_const_ptr(const gsl_matrix_int * m, const size_t i, const size_t j);

#ifdef HAVE_INLINE
INLINE_FUN 
int
gsl_matrix_int_get(const gsl_matrix_int * m, const size_t i, const size_t j)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(1)) 
    {
      if (i >= m->size1)
        {
          GSL_ERROR_VAL("first index out of range", GSL_EINVAL, 0) ;
        }
      else if (j >= m->size2)
        {
          GSL_ERROR_VAL("second index out of range", GSL_EINVAL, 0) ;
        }
    }
#endif
  return m->data[i * m->tda + j] ;
} 

INLINE_FUN 
void
gsl_matrix_int_set(gsl_matrix_int * m, const size_t i, const size_t j, const int x)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(1)) 
    {
      if (i >= m->size1)
        {
          GSL_ERROR_VOID("first index out of range", GSL_EINVAL) ;
        }
      else if (j >= m->size2)
        {
          GSL_ERROR_VOID("second index out of range", GSL_EINVAL) ;
        }
    }
#endif
  m->data[i * m->tda + j] = x ;
}

INLINE_FUN 
int *
gsl_matrix_int_ptr(gsl_matrix_int * m, const size_t i, const size_t j)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(1)) 
    {
      if (i >= m->size1)
        {
          GSL_ERROR_NULL("first index out of range", GSL_EINVAL) ;
        }
      else if (j >= m->size2)
        {
          GSL_ERROR_NULL("second index out of range", GSL_EINVAL) ;
        }
    }
#endif
  return (int *) (m->data + (i * m->tda + j)) ;
} 

INLINE_FUN 
const int *
gsl_matrix_int_const_ptr(const gsl_matrix_int * m, const size_t i, const size_t j)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(1)) 
    {
      if (i >= m->size1)
        {
          GSL_ERROR_NULL("first index out of range", GSL_EINVAL) ;
        }
      else if (j >= m->size2)
        {
          GSL_ERROR_NULL("second index out of range", GSL_EINVAL) ;
        }
    }
#endif
  return (const int *) (m->data + (i * m->tda + j)) ;
} 

#endif

__END_DECLS

#endif /* __GSL_MATRIX_INT_H__ */

/* matrix/gsl_matrix_ushort.h
 * 
 * Copyright (C) 1996, 1997, 1998, 1999, 2000, 2007 Gerard Jungman, Brian Gough
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __GSL_MATRIX_USHORT_H__
#define __GSL_MATRIX_USHORT_H__

#include <stdlib.h>

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif

__BEGIN_DECLS

typedef struct 
{
  size_t size1;
  size_t size2;
  size_t tda;
  unsigned short * data;
  gsl_block_ushort * block;
  int owner;
} gsl_matrix_ushort;

typedef struct
{
  gsl_matrix_ushort matrix;
} _gsl_matrix_ushort_view;

typedef _gsl_matrix_ushort_view gsl_matrix_ushort_view;

typedef struct
{
  gsl_matrix_ushort matrix;
} _gsl_matrix_ushort_const_view;

typedef const _gsl_matrix_ushort_const_view gsl_matrix_ushort_const_view;

/* Allocation */

gsl_matrix_ushort * 
gsl_matrix_ushort_alloc (const size_t n1, const size_t n2);

gsl_matrix_ushort * 
gsl_matrix_ushort_calloc (const size_t n1, const size_t n2);

gsl_matrix_ushort * 
gsl_matrix_ushort_alloc_from_block (gsl_block_ushort * b, 
                                   const size_t offset, 
                                   const size_t n1, 
                                   const size_t n2, 
                                   const size_t d2);

gsl_matrix_ushort * 
gsl_matrix_ushort_alloc_from_matrix (gsl_matrix_ushort * m,
                                    const size_t k1, 
                                    const size_t k2,
                                    const size_t n1, 
                                    const size_t n2);

gsl_vector_ushort * 
gsl_vector_ushort_alloc_row_from_matrix (gsl_matrix_ushort * m,
                                        const size_t i);

gsl_vector_ushort * 
gsl_vector_ushort_alloc_col_from_matrix (gsl_matrix_ushort * m,
                                        const size_t j);

void gsl_matrix_ushort_free (gsl_matrix_ushort * m);

/* Views */

_gsl_matrix_ushort_view 
gsl_matrix_ushort_submatrix (gsl_matrix_ushort * m, 
                            const size_t i, const size_t j, 
                            const size_t n1, const size_t n2);

_gsl_vector_ushort_view 
gsl_matrix_ushort_row (gsl_matrix_ushort * m, const size_t i);

_gsl_vector_ushort_view 
gsl_matrix_ushort_column (gsl_matrix_ushort * m, const size_t j);

_gsl_vector_ushort_view 
gsl_matrix_ushort_diagonal (gsl_matrix_ushort * m);

_gsl_vector_ushort_view 
gsl_matrix_ushort_subdiagonal (gsl_matrix_ushort * m, const size_t k);

_gsl_vector_ushort_view 
gsl_matrix_ushort_superdiagonal (gsl_matrix_ushort * m, const size_t k);

_gsl_vector_ushort_view
gsl_matrix_ushort_subrow (gsl_matrix_ushort * m, const size_t i,
                         const size_t offset, const size_t n);

_gsl_vector_ushort_view
gsl_matrix_ushort_subcolumn (gsl_matrix_ushort * m, const size_t j,
                            const size_t offset, const size_t n);

_gsl_matrix_ushort_view
gsl_matrix_ushort_view_array (unsigned short * base,
                             const size_t n1, 
                             const size_t n2);

_gsl_matrix_ushort_view
gsl_matrix_ushort_view_array_with_tda (unsigned short * base, 
                                      const size_t n1, 
                                      const size_t n2,
                                      const size_t tda);


_gsl_matrix_ushort_view
gsl_matrix_ushort_view_vector (gsl_vector_ushort * v,
                              const size_t n1, 
                              const size_t n2);

_gsl_matrix_ushort_view
gsl_matrix_ushort_view_vector_with_tda (gsl_vector_ushort * v,
                                       const size_t n1, 
                                       const size_t n2,
                                       const size_t tda);


_gsl_matrix_ushort_const_view 
gsl_matrix_ushort_const_submatrix (const gsl_matrix_ushort * m, 
                                  const size_t i, const size_t j, 
                                  const size_t n1, const size_t n2);

_gsl_vector_ushort_const_view 
gsl_matrix_ushort_const_row (const gsl_matrix_ushort * m, 
                            const size_t i);

_gsl_vector_ushort_const_view 
gsl_matrix_ushort_const_column (const gsl_matrix_ushort * m, 
                               const size_t j);

_gsl_vector_ushort_const_view
gsl_matrix_ushort_const_diagonal (const gsl_matrix_ushort * m);

_gsl_vector_ushort_const_view 
gsl_matrix_ushort_const_subdiagonal (const gsl_matrix_ushort * m, 
                                    const size_t k);

_gsl_vector_ushort_const_view 
gsl_matrix_ushort_const_superdiagonal (const gsl_matrix_ushort * m, 
                                      const size_t k);

_gsl_vector_ushort_const_view
gsl_matrix_ushort_const_subrow (const gsl_matrix_ushort * m, const size_t i,
                               const size_t offset, const size_t n);

_gsl_vector_ushort_const_view
gsl_matrix_ushort_const_subcolumn (const gsl_matrix_ushort * m, const size_t j,
                                  const size_t offset, const size_t n);

_gsl_matrix_ushort_const_view
gsl_matrix_ushort_const_view_array (const unsigned short * base,
                                   const size_t n1, 
                                   const size_t n2);

_gsl_matrix_ushort_const_view
gsl_matrix_ushort_const_view_array_with_tda (const unsigned short * base, 
                                            const size_t n1, 
                                            const size_t n2,
                                            const size_t tda);

_gsl_matrix_ushort_const_view
gsl_matrix_ushort_const_view_vector (const gsl_vector_ushort * v,
                                    const size_t n1, 
                                    const size_t n2);

_gsl_matrix_ushort_const_view
gsl_matrix_ushort_const_view_vector_with_tda (const gsl_vector_ushort * v,
                                             const size_t n1, 
                                             const size_t n2,
                                             const size_t tda);

/* Operations */

void gsl_matrix_ushort_set_zero (gsl_matrix_ushort * m);
void gsl_matrix_ushort_set_identity (gsl_matrix_ushort * m);
void gsl_matrix_ushort_set_all (gsl_matrix_ushort * m, unsigned short x);

int gsl_matrix_ushort_fread (FILE * stream, gsl_matrix_ushort * m) ;
int gsl_matrix_ushort_fwrite (FILE * stream, const gsl_matrix_ushort * m) ;
int gsl_matrix_ushort_fscanf (FILE * stream, gsl_matrix_ushort * m);
int gsl_matrix_ushort_fprintf (FILE * stream, const gsl_matrix_ushort * m, const char * format);
 
int gsl_matrix_ushort_memcpy(gsl_matrix_ushort * dest, const gsl_matrix_ushort * src);
int gsl_matrix_ushort_swap(gsl_matrix_ushort * m1, gsl_matrix_ushort * m2);

int gsl_matrix_ushort_swap_rows(gsl_matrix_ushort * m, const size_t i, const size_t j);
int gsl_matrix_ushort_swap_columns(gsl_matrix_ushort * m, const size_t i, const size_t j);
int gsl_matrix_ushort_swap_rowcol(gsl_matrix_ushort * m, const size_t i, const size_t j);
int gsl_matrix_ushort_transpose (gsl_matrix_ushort * m);
int gsl_matrix_ushort_transpose_memcpy (gsl_matrix_ushort * dest, const gsl_matrix_ushort * src);

unsigned short gsl_matrix_ushort_max (const gsl_matrix_ushort * m);
unsigned short gsl_matrix_ushort_min (const gsl_matrix_ushort * m);
void gsl_matrix_ushort_minmax (const gsl_matrix_ushort * m, unsigned short * min_out, unsigned short * max_out);

void gsl_matrix_ushort_max_index (const gsl_matrix_ushort * m, size_t * imax, size_t *jmax);
void gsl_matrix_ushort_min_index (const gsl_matrix_ushort * m, size_t * imin, size_t *jmin);
void gsl_matrix_ushort_minmax_index (const gsl_matrix_ushort * m, size_t * imin, size_t * jmin, size_t * imax, size_t * jmax);

int gsl_matrix_ushort_equal (const gsl_matrix_ushort * a, const gsl_matrix_ushort * b);

int gsl_matrix_ushort_isnull (const gsl_matrix_ushort * m);
int gsl_matrix_ushort_ispos (const gsl_matrix_ushort * m);
int gsl_matrix_ushort_isneg (const gsl_matrix_ushort * m);
int gsl_matrix_ushort_isnonneg (const gsl_matrix_ushort * m);

int gsl_matrix_ushort_add (gsl_matrix_ushort * a, const gsl_matrix_ushort * b);
int gsl_matrix_ushort_sub (gsl_matrix_ushort * a, const gsl_matrix_ushort * b);
int gsl_matrix_ushort_mul_elements (gsl_matrix_ushort * a, const gsl_matrix_ushort * b);
int gsl_matrix_ushort_div_elements (gsl_matrix_ushort * a, const gsl_matrix_ushort * b);
int gsl_matrix_ushort_scale (gsl_matrix_ushort * a, const double x);
int gsl_matrix_ushort_add_constant (gsl_matrix_ushort * a, const double x);
int gsl_matrix_ushort_add_diagonal (gsl_matrix_ushort * a, const double x);

/***********************************************************************/
/* The functions below are obsolete                                    */
/***********************************************************************/
int gsl_matrix_ushort_get_row(gsl_vector_ushort * v, const gsl_matrix_ushort * m, const size_t i);
int gsl_matrix_ushort_get_col(gsl_vector_ushort * v, const gsl_matrix_ushort * m, const size_t j);
int gsl_matrix_ushort_set_row(gsl_matrix_ushort * m, const size_t i, const gsl_vector_ushort * v);
int gsl_matrix_ushort_set_col(gsl_matrix_ushort * m, const size_t j, const gsl_vector_ushort * v);
/***********************************************************************/

/* inline functions if you are using GCC */

INLINE_DECL unsigned short   gsl_matrix_ushort_get(const gsl_matrix_ushort * m, const size_t i, const size_t j);
INLINE_DECL void    gsl_matrix_ushort_set(gsl_matrix_ushort * m, const size_t i, const size_t j, const unsigned short x);
INLINE_DECL unsigned short * gsl_matrix_ushort_ptr(gsl_matrix_ushort * m, const size_t i, const size_t j);
INLINE_DECL const unsigned short * gsl_matrix_ushort_const_ptr(const gsl_matrix_ushort * m, const size_t i, const size_t j);

#ifdef HAVE_INLINE
INLINE_FUN 
unsigned short
gsl_matrix_ushort_get(const gsl_matrix_ushort * m, const size_t i, const size_t j)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(1)) 
    {
      if (i >= m->size1)
        {
          GSL_ERROR_VAL("first index out of range", GSL_EINVAL, 0) ;
        }
      else if (j >= m->size2)
        {
          GSL_ERROR_VAL("second index out of range", GSL_EINVAL, 0) ;
        }
    }
#endif
  return m->data[i * m->tda + j] ;
} 

INLINE_FUN 
void
gsl_matrix_ushort_set(gsl_matrix_ushort * m, const size_t i, const size_t j, const unsigned short x)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(1)) 
    {
      if (i >= m->size1)
        {
          GSL_ERROR_VOID("first index out of range", GSL_EINVAL) ;
        }
      else if (j >= m->size2)
        {
          GSL_ERROR_VOID("second index out of range", GSL_EINVAL) ;
        }
    }
#endif
  m->data[i * m->tda + j] = x ;
}

INLINE_FUN 
unsigned short *
gsl_matrix_ushort_ptr(gsl_matrix_ushort * m, const size_t i, const size_t j)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(1)) 
    {
      if (i >= m->size1)
        {
          GSL_ERROR_NULL("first index out of range", GSL_EINVAL) ;
        }
      else if (j >= m->size2)
        {
          GSL_ERROR_NULL("second index out of range", GSL_EINVAL) ;
        }
    }
#endif
  return (unsigned short *) (m->data + (i * m->tda + j)) ;
} 

INLINE_FUN 
const unsigned short *
gsl_matrix_ushort_const_ptr(const gsl_matrix_ushort * m, const size_t i, const size_t j)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(1)) 
    {
      if (i >= m->size1)
        {
          GSL_ERROR_NULL("first index out of range", GSL_EINVAL) ;
        }
      else if (j >= m->size2)
        {
          GSL_ERROR_NULL("second index out of range", GSL_EINVAL) ;
        }
    }
#endif
  return (const unsigned short *) (m->data + (i * m->tda + j)) ;
} 

#endif

__END_DECLS

#endif /* __GSL_MATRIX_USHORT_H__ */
/* matrix/gsl_matrix_short.h
 * 
 * Copyright (C) 1996, 1997, 1998, 1999, 2000, 2007 Gerard Jungman, Brian Gough
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __GSL_MATRIX_SHORT_H__
#define __GSL_MATRIX_SHORT_H__

#include <stdlib.h>

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif

__BEGIN_DECLS

typedef struct 
{
  size_t size1;
  size_t size2;
  size_t tda;
  short * data;
  gsl_block_short * block;
  int owner;
} gsl_matrix_short;

typedef struct
{
  gsl_matrix_short matrix;
} _gsl_matrix_short_view;

typedef _gsl_matrix_short_view gsl_matrix_short_view;

typedef struct
{
  gsl_matrix_short matrix;
} _gsl_matrix_short_const_view;

typedef const _gsl_matrix_short_const_view gsl_matrix_short_const_view;

/* Allocation */

gsl_matrix_short * 
gsl_matrix_short_alloc (const size_t n1, const size_t n2);

gsl_matrix_short * 
gsl_matrix_short_calloc (const size_t n1, const size_t n2);

gsl_matrix_short * 
gsl_matrix_short_alloc_from_block (gsl_block_short * b, 
                                   const size_t offset, 
                                   const size_t n1, 
                                   const size_t n2, 
                                   const size_t d2);

gsl_matrix_short * 
gsl_matrix_short_alloc_from_matrix (gsl_matrix_short * m,
                                    const size_t k1, 
                                    const size_t k2,
                                    const size_t n1, 
                                    const size_t n2);

gsl_vector_short * 
gsl_vector_short_alloc_row_from_matrix (gsl_matrix_short * m,
                                        const size_t i);

gsl_vector_short * 
gsl_vector_short_alloc_col_from_matrix (gsl_matrix_short * m,
                                        const size_t j);

void gsl_matrix_short_free (gsl_matrix_short * m);

/* Views */

_gsl_matrix_short_view 
gsl_matrix_short_submatrix (gsl_matrix_short * m, 
                            const size_t i, const size_t j, 
                            const size_t n1, const size_t n2);

_gsl_vector_short_view 
gsl_matrix_short_row (gsl_matrix_short * m, const size_t i);

_gsl_vector_short_view 
gsl_matrix_short_column (gsl_matrix_short * m, const size_t j);

_gsl_vector_short_view 
gsl_matrix_short_diagonal (gsl_matrix_short * m);

_gsl_vector_short_view 
gsl_matrix_short_subdiagonal (gsl_matrix_short * m, const size_t k);

_gsl_vector_short_view 
gsl_matrix_short_superdiagonal (gsl_matrix_short * m, const size_t k);

_gsl_vector_short_view
gsl_matrix_short_subrow (gsl_matrix_short * m, const size_t i,
                         const size_t offset, const size_t n);

_gsl_vector_short_view
gsl_matrix_short_subcolumn (gsl_matrix_short * m, const size_t j,
                            const size_t offset, const size_t n);

_gsl_matrix_short_view
gsl_matrix_short_view_array (short * base,
                             const size_t n1, 
                             const size_t n2);

_gsl_matrix_short_view
gsl_matrix_short_view_array_with_tda (short * base, 
                                      const size_t n1, 
                                      const size_t n2,
                                      const size_t tda);


_gsl_matrix_short_view
gsl_matrix_short_view_vector (gsl_vector_short * v,
                              const size_t n1, 
                              const size_t n2);

_gsl_matrix_short_view
gsl_matrix_short_view_vector_with_tda (gsl_vector_short * v,
                                       const size_t n1, 
                                       const size_t n2,
                                       const size_t tda);


_gsl_matrix_short_const_view 
gsl_matrix_short_const_submatrix (const gsl_matrix_short * m, 
                                  const size_t i, const size_t j, 
                                  const size_t n1, const size_t n2);

_gsl_vector_short_const_view 
gsl_matrix_short_const_row (const gsl_matrix_short * m, 
                            const size_t i);

_gsl_vector_short_const_view 
gsl_matrix_short_const_column (const gsl_matrix_short * m, 
                               const size_t j);

_gsl_vector_short_const_view
gsl_matrix_short_const_diagonal (const gsl_matrix_short * m);

_gsl_vector_short_const_view 
gsl_matrix_short_const_subdiagonal (const gsl_matrix_short * m, 
                                    const size_t k);

_gsl_vector_short_const_view 
gsl_matrix_short_const_superdiagonal (const gsl_matrix_short * m, 
                                      const size_t k);

_gsl_vector_short_const_view
gsl_matrix_short_const_subrow (const gsl_matrix_short * m, const size_t i,
                               const size_t offset, const size_t n);

_gsl_vector_short_const_view
gsl_matrix_short_const_subcolumn (const gsl_matrix_short * m, const size_t j,
                                  const size_t offset, const size_t n);

_gsl_matrix_short_const_view
gsl_matrix_short_const_view_array (const short * base,
                                   const size_t n1, 
                                   const size_t n2);

_gsl_matrix_short_const_view
gsl_matrix_short_const_view_array_with_tda (const short * base, 
                                            const size_t n1, 
                                            const size_t n2,
                                            const size_t tda);

_gsl_matrix_short_const_view
gsl_matrix_short_const_view_vector (const gsl_vector_short * v,
                                    const size_t n1, 
                                    const size_t n2);

_gsl_matrix_short_const_view
gsl_matrix_short_const_view_vector_with_tda (const gsl_vector_short * v,
                                             const size_t n1, 
                                             const size_t n2,
                                             const size_t tda);

/* Operations */

void gsl_matrix_short_set_zero (gsl_matrix_short * m);
void gsl_matrix_short_set_identity (gsl_matrix_short * m);
void gsl_matrix_short_set_all (gsl_matrix_short * m, short x);

int gsl_matrix_short_fread (FILE * stream, gsl_matrix_short * m) ;
int gsl_matrix_short_fwrite (FILE * stream, const gsl_matrix_short * m) ;
int gsl_matrix_short_fscanf (FILE * stream, gsl_matrix_short * m);
int gsl_matrix_short_fprintf (FILE * stream, const gsl_matrix_short * m, const char * format);
 
int gsl_matrix_short_memcpy(gsl_matrix_short * dest, const gsl_matrix_short * src);
int gsl_matrix_short_swap(gsl_matrix_short * m1, gsl_matrix_short * m2);

int gsl_matrix_short_swap_rows(gsl_matrix_short * m, const size_t i, const size_t j);
int gsl_matrix_short_swap_columns(gsl_matrix_short * m, const size_t i, const size_t j);
int gsl_matrix_short_swap_rowcol(gsl_matrix_short * m, const size_t i, const size_t j);
int gsl_matrix_short_transpose (gsl_matrix_short * m);
int gsl_matrix_short_transpose_memcpy (gsl_matrix_short * dest, const gsl_matrix_short * src);

short gsl_matrix_short_max (const gsl_matrix_short * m);
short gsl_matrix_short_min (const gsl_matrix_short * m);
void gsl_matrix_short_minmax (const gsl_matrix_short * m, short * min_out, short * max_out);

void gsl_matrix_short_max_index (const gsl_matrix_short * m, size_t * imax, size_t *jmax);
void gsl_matrix_short_min_index (const gsl_matrix_short * m, size_t * imin, size_t *jmin);
void gsl_matrix_short_minmax_index (const gsl_matrix_short * m, size_t * imin, size_t * jmin, size_t * imax, size_t * jmax);

int gsl_matrix_short_equal (const gsl_matrix_short * a, const gsl_matrix_short * b);

int gsl_matrix_short_isnull (const gsl_matrix_short * m);
int gsl_matrix_short_ispos (const gsl_matrix_short * m);
int gsl_matrix_short_isneg (const gsl_matrix_short * m);
int gsl_matrix_short_isnonneg (const gsl_matrix_short * m);

int gsl_matrix_short_add (gsl_matrix_short * a, const gsl_matrix_short * b);
int gsl_matrix_short_sub (gsl_matrix_short * a, const gsl_matrix_short * b);
int gsl_matrix_short_mul_elements (gsl_matrix_short * a, const gsl_matrix_short * b);
int gsl_matrix_short_div_elements (gsl_matrix_short * a, const gsl_matrix_short * b);
int gsl_matrix_short_scale (gsl_matrix_short * a, const double x);
int gsl_matrix_short_add_constant (gsl_matrix_short * a, const double x);
int gsl_matrix_short_add_diagonal (gsl_matrix_short * a, const double x);

/***********************************************************************/
/* The functions below are obsolete                                    */
/***********************************************************************/
int gsl_matrix_short_get_row(gsl_vector_short * v, const gsl_matrix_short * m, const size_t i);
int gsl_matrix_short_get_col(gsl_vector_short * v, const gsl_matrix_short * m, const size_t j);
int gsl_matrix_short_set_row(gsl_matrix_short * m, const size_t i, const gsl_vector_short * v);
int gsl_matrix_short_set_col(gsl_matrix_short * m, const size_t j, const gsl_vector_short * v);
/***********************************************************************/

/* inline functions if you are using GCC */

INLINE_DECL short   gsl_matrix_short_get(const gsl_matrix_short * m, const size_t i, const size_t j);
INLINE_DECL void    gsl_matrix_short_set(gsl_matrix_short * m, const size_t i, const size_t j, const short x);
INLINE_DECL short * gsl_matrix_short_ptr(gsl_matrix_short * m, const size_t i, const size_t j);
INLINE_DECL const short * gsl_matrix_short_const_ptr(const gsl_matrix_short * m, const size_t i, const size_t j);

#ifdef HAVE_INLINE
INLINE_FUN 
short
gsl_matrix_short_get(const gsl_matrix_short * m, const size_t i, const size_t j)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(1)) 
    {
      if (i >= m->size1)
        {
          GSL_ERROR_VAL("first index out of range", GSL_EINVAL, 0) ;
        }
      else if (j >= m->size2)
        {
          GSL_ERROR_VAL("second index out of range", GSL_EINVAL, 0) ;
        }
    }
#endif
  return m->data[i * m->tda + j] ;
} 

INLINE_FUN 
void
gsl_matrix_short_set(gsl_matrix_short * m, const size_t i, const size_t j, const short x)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(1)) 
    {
      if (i >= m->size1)
        {
          GSL_ERROR_VOID("first index out of range", GSL_EINVAL) ;
        }
      else if (j >= m->size2)
        {
          GSL_ERROR_VOID("second index out of range", GSL_EINVAL) ;
        }
    }
#endif
  m->data[i * m->tda + j] = x ;
}

INLINE_FUN 
short *
gsl_matrix_short_ptr(gsl_matrix_short * m, const size_t i, const size_t j)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(1)) 
    {
      if (i >= m->size1)
        {
          GSL_ERROR_NULL("first index out of range", GSL_EINVAL) ;
        }
      else if (j >= m->size2)
        {
          GSL_ERROR_NULL("second index out of range", GSL_EINVAL) ;
        }
    }
#endif
  return (short *) (m->data + (i * m->tda + j)) ;
} 

INLINE_FUN 
const short *
gsl_matrix_short_const_ptr(const gsl_matrix_short * m, const size_t i, const size_t j)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(1)) 
    {
      if (i >= m->size1)
        {
          GSL_ERROR_NULL("first index out of range", GSL_EINVAL) ;
        }
      else if (j >= m->size2)
        {
          GSL_ERROR_NULL("second index out of range", GSL_EINVAL) ;
        }
    }
#endif
  return (const short *) (m->data + (i * m->tda + j)) ;
} 

#endif

__END_DECLS

#endif /* __GSL_MATRIX_SHORT_H__ */

/* matrix/gsl_matrix_uchar.h
 * 
 * Copyright (C) 1996, 1997, 1998, 1999, 2000, 2007 Gerard Jungman, Brian Gough
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __GSL_MATRIX_UCHAR_H__
#define __GSL_MATRIX_UCHAR_H__

#include <stdlib.h>

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif

__BEGIN_DECLS

typedef struct 
{
  size_t size1;
  size_t size2;
  size_t tda;
  unsigned char * data;
  gsl_block_uchar * block;
  int owner;
} gsl_matrix_uchar;

typedef struct
{
  gsl_matrix_uchar matrix;
} _gsl_matrix_uchar_view;

typedef _gsl_matrix_uchar_view gsl_matrix_uchar_view;

typedef struct
{
  gsl_matrix_uchar matrix;
} _gsl_matrix_uchar_const_view;

typedef const _gsl_matrix_uchar_const_view gsl_matrix_uchar_const_view;

/* Allocation */

gsl_matrix_uchar * 
gsl_matrix_uchar_alloc (const size_t n1, const size_t n2);

gsl_matrix_uchar * 
gsl_matrix_uchar_calloc (const size_t n1, const size_t n2);

gsl_matrix_uchar * 
gsl_matrix_uchar_alloc_from_block (gsl_block_uchar * b, 
                                   const size_t offset, 
                                   const size_t n1, 
                                   const size_t n2, 
                                   const size_t d2);

gsl_matrix_uchar * 
gsl_matrix_uchar_alloc_from_matrix (gsl_matrix_uchar * m,
                                    const size_t k1, 
                                    const size_t k2,
                                    const size_t n1, 
                                    const size_t n2);

gsl_vector_uchar * 
gsl_vector_uchar_alloc_row_from_matrix (gsl_matrix_uchar * m,
                                        const size_t i);

gsl_vector_uchar * 
gsl_vector_uchar_alloc_col_from_matrix (gsl_matrix_uchar * m,
                                        const size_t j);

void gsl_matrix_uchar_free (gsl_matrix_uchar * m);

/* Views */

_gsl_matrix_uchar_view 
gsl_matrix_uchar_submatrix (gsl_matrix_uchar * m, 
                            const size_t i, const size_t j, 
                            const size_t n1, const size_t n2);

_gsl_vector_uchar_view 
gsl_matrix_uchar_row (gsl_matrix_uchar * m, const size_t i);

_gsl_vector_uchar_view 
gsl_matrix_uchar_column (gsl_matrix_uchar * m, const size_t j);

_gsl_vector_uchar_view 
gsl_matrix_uchar_diagonal (gsl_matrix_uchar * m);

_gsl_vector_uchar_view 
gsl_matrix_uchar_subdiagonal (gsl_matrix_uchar * m, const size_t k);

_gsl_vector_uchar_view 
gsl_matrix_uchar_superdiagonal (gsl_matrix_uchar * m, const size_t k);

_gsl_vector_uchar_view
gsl_matrix_uchar_subrow (gsl_matrix_uchar * m, const size_t i,
                         const size_t offset, const size_t n);

_gsl_vector_uchar_view
gsl_matrix_uchar_subcolumn (gsl_matrix_uchar * m, const size_t j,
                            const size_t offset, const size_t n);

_gsl_matrix_uchar_view
gsl_matrix_uchar_view_array (unsigned char * base,
                             const size_t n1, 
                             const size_t n2);

_gsl_matrix_uchar_view
gsl_matrix_uchar_view_array_with_tda (unsigned char * base, 
                                      const size_t n1, 
                                      const size_t n2,
                                      const size_t tda);


_gsl_matrix_uchar_view
gsl_matrix_uchar_view_vector (gsl_vector_uchar * v,
                              const size_t n1, 
                              const size_t n2);

_gsl_matrix_uchar_view
gsl_matrix_uchar_view_vector_with_tda (gsl_vector_uchar * v,
                                       const size_t n1, 
                                       const size_t n2,
                                       const size_t tda);


_gsl_matrix_uchar_const_view 
gsl_matrix_uchar_const_submatrix (const gsl_matrix_uchar * m, 
                                  const size_t i, const size_t j, 
                                  const size_t n1, const size_t n2);

_gsl_vector_uchar_const_view 
gsl_matrix_uchar_const_row (const gsl_matrix_uchar * m, 
                            const size_t i);

_gsl_vector_uchar_const_view 
gsl_matrix_uchar_const_column (const gsl_matrix_uchar * m, 
                               const size_t j);

_gsl_vector_uchar_const_view
gsl_matrix_uchar_const_diagonal (const gsl_matrix_uchar * m);

_gsl_vector_uchar_const_view 
gsl_matrix_uchar_const_subdiagonal (const gsl_matrix_uchar * m, 
                                    const size_t k);

_gsl_vector_uchar_const_view 
gsl_matrix_uchar_const_superdiagonal (const gsl_matrix_uchar * m, 
                                      const size_t k);

_gsl_vector_uchar_const_view
gsl_matrix_uchar_const_subrow (const gsl_matrix_uchar * m, const size_t i,
                               const size_t offset, const size_t n);

_gsl_vector_uchar_const_view
gsl_matrix_uchar_const_subcolumn (const gsl_matrix_uchar * m, const size_t j,
                                  const size_t offset, const size_t n);

_gsl_matrix_uchar_const_view
gsl_matrix_uchar_const_view_array (const unsigned char * base,
                                   const size_t n1, 
                                   const size_t n2);

_gsl_matrix_uchar_const_view
gsl_matrix_uchar_const_view_array_with_tda (const unsigned char * base, 
                                            const size_t n1, 
                                            const size_t n2,
                                            const size_t tda);

_gsl_matrix_uchar_const_view
gsl_matrix_uchar_const_view_vector (const gsl_vector_uchar * v,
                                    const size_t n1, 
                                    const size_t n2);

_gsl_matrix_uchar_const_view
gsl_matrix_uchar_const_view_vector_with_tda (const gsl_vector_uchar * v,
                                             const size_t n1, 
                                             const size_t n2,
                                             const size_t tda);

/* Operations */

void gsl_matrix_uchar_set_zero (gsl_matrix_uchar * m);
void gsl_matrix_uchar_set_identity (gsl_matrix_uchar * m);
void gsl_matrix_uchar_set_all (gsl_matrix_uchar * m, unsigned char x);

int gsl_matrix_uchar_fread (FILE * stream, gsl_matrix_uchar * m) ;
int gsl_matrix_uchar_fwrite (FILE * stream, const gsl_matrix_uchar * m) ;
int gsl_matrix_uchar_fscanf (FILE * stream, gsl_matrix_uchar * m);
int gsl_matrix_uchar_fprintf (FILE * stream, const gsl_matrix_uchar * m, const char * format);
 
int gsl_matrix_uchar_memcpy(gsl_matrix_uchar * dest, const gsl_matrix_uchar * src);
int gsl_matrix_uchar_swap(gsl_matrix_uchar * m1, gsl_matrix_uchar * m2);

int gsl_matrix_uchar_swap_rows(gsl_matrix_uchar * m, const size_t i, const size_t j);
int gsl_matrix_uchar_swap_columns(gsl_matrix_uchar * m, const size_t i, const size_t j);
int gsl_matrix_uchar_swap_rowcol(gsl_matrix_uchar * m, const size_t i, const size_t j);
int gsl_matrix_uchar_transpose (gsl_matrix_uchar * m);
int gsl_matrix_uchar_transpose_memcpy (gsl_matrix_uchar * dest, const gsl_matrix_uchar * src);

unsigned char gsl_matrix_uchar_max (const gsl_matrix_uchar * m);
unsigned char gsl_matrix_uchar_min (const gsl_matrix_uchar * m);
void gsl_matrix_uchar_minmax (const gsl_matrix_uchar * m, unsigned char * min_out, unsigned char * max_out);

void gsl_matrix_uchar_max_index (const gsl_matrix_uchar * m, size_t * imax, size_t *jmax);
void gsl_matrix_uchar_min_index (const gsl_matrix_uchar * m, size_t * imin, size_t *jmin);
void gsl_matrix_uchar_minmax_index (const gsl_matrix_uchar * m, size_t * imin, size_t * jmin, size_t * imax, size_t * jmax);

int gsl_matrix_uchar_equal (const gsl_matrix_uchar * a, const gsl_matrix_uchar * b);

int gsl_matrix_uchar_isnull (const gsl_matrix_uchar * m);
int gsl_matrix_uchar_ispos (const gsl_matrix_uchar * m);
int gsl_matrix_uchar_isneg (const gsl_matrix_uchar * m);
int gsl_matrix_uchar_isnonneg (const gsl_matrix_uchar * m);

int gsl_matrix_uchar_add (gsl_matrix_uchar * a, const gsl_matrix_uchar * b);
int gsl_matrix_uchar_sub (gsl_matrix_uchar * a, const gsl_matrix_uchar * b);
int gsl_matrix_uchar_mul_elements (gsl_matrix_uchar * a, const gsl_matrix_uchar * b);
int gsl_matrix_uchar_div_elements (gsl_matrix_uchar * a, const gsl_matrix_uchar * b);
int gsl_matrix_uchar_scale (gsl_matrix_uchar * a, const double x);
int gsl_matrix_uchar_add_constant (gsl_matrix_uchar * a, const double x);
int gsl_matrix_uchar_add_diagonal (gsl_matrix_uchar * a, const double x);

/***********************************************************************/
/* The functions below are obsolete                                    */
/***********************************************************************/
int gsl_matrix_uchar_get_row(gsl_vector_uchar * v, const gsl_matrix_uchar * m, const size_t i);
int gsl_matrix_uchar_get_col(gsl_vector_uchar * v, const gsl_matrix_uchar * m, const size_t j);
int gsl_matrix_uchar_set_row(gsl_matrix_uchar * m, const size_t i, const gsl_vector_uchar * v);
int gsl_matrix_uchar_set_col(gsl_matrix_uchar * m, const size_t j, const gsl_vector_uchar * v);
/***********************************************************************/

/* inline functions if you are using GCC */

INLINE_DECL unsigned char   gsl_matrix_uchar_get(const gsl_matrix_uchar * m, const size_t i, const size_t j);
INLINE_DECL void    gsl_matrix_uchar_set(gsl_matrix_uchar * m, const size_t i, const size_t j, const unsigned char x);
INLINE_DECL unsigned char * gsl_matrix_uchar_ptr(gsl_matrix_uchar * m, const size_t i, const size_t j);
INLINE_DECL const unsigned char * gsl_matrix_uchar_const_ptr(const gsl_matrix_uchar * m, const size_t i, const size_t j);

#ifdef HAVE_INLINE
INLINE_FUN 
unsigned char
gsl_matrix_uchar_get(const gsl_matrix_uchar * m, const size_t i, const size_t j)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(1)) 
    {
      if (i >= m->size1)
        {
          GSL_ERROR_VAL("first index out of range", GSL_EINVAL, 0) ;
        }
      else if (j >= m->size2)
        {
          GSL_ERROR_VAL("second index out of range", GSL_EINVAL, 0) ;
        }
    }
#endif
  return m->data[i * m->tda + j] ;
} 

INLINE_FUN 
void
gsl_matrix_uchar_set(gsl_matrix_uchar * m, const size_t i, const size_t j, const unsigned char x)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(1)) 
    {
      if (i >= m->size1)
        {
          GSL_ERROR_VOID("first index out of range", GSL_EINVAL) ;
        }
      else if (j >= m->size2)
        {
          GSL_ERROR_VOID("second index out of range", GSL_EINVAL) ;
        }
    }
#endif
  m->data[i * m->tda + j] = x ;
}

INLINE_FUN 
unsigned char *
gsl_matrix_uchar_ptr(gsl_matrix_uchar * m, const size_t i, const size_t j)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(1)) 
    {
      if (i >= m->size1)
        {
          GSL_ERROR_NULL("first index out of range", GSL_EINVAL) ;
        }
      else if (j >= m->size2)
        {
          GSL_ERROR_NULL("second index out of range", GSL_EINVAL) ;
        }
    }
#endif
  return (unsigned char *) (m->data + (i * m->tda + j)) ;
} 

INLINE_FUN 
const unsigned char *
gsl_matrix_uchar_const_ptr(const gsl_matrix_uchar * m, const size_t i, const size_t j)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(1)) 
    {
      if (i >= m->size1)
        {
          GSL_ERROR_NULL("first index out of range", GSL_EINVAL) ;
        }
      else if (j >= m->size2)
        {
          GSL_ERROR_NULL("second index out of range", GSL_EINVAL) ;
        }
    }
#endif
  return (const unsigned char *) (m->data + (i * m->tda + j)) ;
} 

#endif

__END_DECLS

#endif /* __GSL_MATRIX_UCHAR_H__ */
/* matrix/gsl_matrix_char.h
 * 
 * Copyright (C) 1996, 1997, 1998, 1999, 2000, 2007 Gerard Jungman, Brian Gough
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __GSL_MATRIX_CHAR_H__
#define __GSL_MATRIX_CHAR_H__

#include <stdlib.h>

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif

__BEGIN_DECLS

typedef struct 
{
  size_t size1;
  size_t size2;
  size_t tda;
  char * data;
  gsl_block_char * block;
  int owner;
} gsl_matrix_char;

typedef struct
{
  gsl_matrix_char matrix;
} _gsl_matrix_char_view;

typedef _gsl_matrix_char_view gsl_matrix_char_view;

typedef struct
{
  gsl_matrix_char matrix;
} _gsl_matrix_char_const_view;

typedef const _gsl_matrix_char_const_view gsl_matrix_char_const_view;

/* Allocation */

gsl_matrix_char * 
gsl_matrix_char_alloc (const size_t n1, const size_t n2);

gsl_matrix_char * 
gsl_matrix_char_calloc (const size_t n1, const size_t n2);

gsl_matrix_char * 
gsl_matrix_char_alloc_from_block (gsl_block_char * b, 
                                   const size_t offset, 
                                   const size_t n1, 
                                   const size_t n2, 
                                   const size_t d2);

gsl_matrix_char * 
gsl_matrix_char_alloc_from_matrix (gsl_matrix_char * m,
                                    const size_t k1, 
                                    const size_t k2,
                                    const size_t n1, 
                                    const size_t n2);

gsl_vector_char * 
gsl_vector_char_alloc_row_from_matrix (gsl_matrix_char * m,
                                        const size_t i);

gsl_vector_char * 
gsl_vector_char_alloc_col_from_matrix (gsl_matrix_char * m,
                                        const size_t j);

void gsl_matrix_char_free (gsl_matrix_char * m);

/* Views */

_gsl_matrix_char_view 
gsl_matrix_char_submatrix (gsl_matrix_char * m, 
                            const size_t i, const size_t j, 
                            const size_t n1, const size_t n2);

_gsl_vector_char_view 
gsl_matrix_char_row (gsl_matrix_char * m, const size_t i);

_gsl_vector_char_view 
gsl_matrix_char_column (gsl_matrix_char * m, const size_t j);

_gsl_vector_char_view 
gsl_matrix_char_diagonal (gsl_matrix_char * m);

_gsl_vector_char_view 
gsl_matrix_char_subdiagonal (gsl_matrix_char * m, const size_t k);

_gsl_vector_char_view 
gsl_matrix_char_superdiagonal (gsl_matrix_char * m, const size_t k);

_gsl_vector_char_view
gsl_matrix_char_subrow (gsl_matrix_char * m, const size_t i,
                         const size_t offset, const size_t n);

_gsl_vector_char_view
gsl_matrix_char_subcolumn (gsl_matrix_char * m, const size_t j,
                            const size_t offset, const size_t n);

_gsl_matrix_char_view
gsl_matrix_char_view_array (char * base,
                             const size_t n1, 
                             const size_t n2);

_gsl_matrix_char_view
gsl_matrix_char_view_array_with_tda (char * base, 
                                      const size_t n1, 
                                      const size_t n2,
                                      const size_t tda);


_gsl_matrix_char_view
gsl_matrix_char_view_vector (gsl_vector_char * v,
                              const size_t n1, 
                              const size_t n2);

_gsl_matrix_char_view
gsl_matrix_char_view_vector_with_tda (gsl_vector_char * v,
                                       const size_t n1, 
                                       const size_t n2,
                                       const size_t tda);


_gsl_matrix_char_const_view 
gsl_matrix_char_const_submatrix (const gsl_matrix_char * m, 
                                  const size_t i, const size_t j, 
                                  const size_t n1, const size_t n2);

_gsl_vector_char_const_view 
gsl_matrix_char_const_row (const gsl_matrix_char * m, 
                            const size_t i);

_gsl_vector_char_const_view 
gsl_matrix_char_const_column (const gsl_matrix_char * m, 
                               const size_t j);

_gsl_vector_char_const_view
gsl_matrix_char_const_diagonal (const gsl_matrix_char * m);

_gsl_vector_char_const_view 
gsl_matrix_char_const_subdiagonal (const gsl_matrix_char * m, 
                                    const size_t k);

_gsl_vector_char_const_view 
gsl_matrix_char_const_superdiagonal (const gsl_matrix_char * m, 
                                      const size_t k);

_gsl_vector_char_const_view
gsl_matrix_char_const_subrow (const gsl_matrix_char * m, const size_t i,
                               const size_t offset, const size_t n);

_gsl_vector_char_const_view
gsl_matrix_char_const_subcolumn (const gsl_matrix_char * m, const size_t j,
                                  const size_t offset, const size_t n);

_gsl_matrix_char_const_view
gsl_matrix_char_const_view_array (const char * base,
                                   const size_t n1, 
                                   const size_t n2);

_gsl_matrix_char_const_view
gsl_matrix_char_const_view_array_with_tda (const char * base, 
                                            const size_t n1, 
                                            const size_t n2,
                                            const size_t tda);

_gsl_matrix_char_const_view
gsl_matrix_char_const_view_vector (const gsl_vector_char * v,
                                    const size_t n1, 
                                    const size_t n2);

_gsl_matrix_char_const_view
gsl_matrix_char_const_view_vector_with_tda (const gsl_vector_char * v,
                                             const size_t n1, 
                                             const size_t n2,
                                             const size_t tda);

/* Operations */

void gsl_matrix_char_set_zero (gsl_matrix_char * m);
void gsl_matrix_char_set_identity (gsl_matrix_char * m);
void gsl_matrix_char_set_all (gsl_matrix_char * m, char x);

int gsl_matrix_char_fread (FILE * stream, gsl_matrix_char * m) ;
int gsl_matrix_char_fwrite (FILE * stream, const gsl_matrix_char * m) ;
int gsl_matrix_char_fscanf (FILE * stream, gsl_matrix_char * m);
int gsl_matrix_char_fprintf (FILE * stream, const gsl_matrix_char * m, const char * format);
 
int gsl_matrix_char_memcpy(gsl_matrix_char * dest, const gsl_matrix_char * src);
int gsl_matrix_char_swap(gsl_matrix_char * m1, gsl_matrix_char * m2);

int gsl_matrix_char_swap_rows(gsl_matrix_char * m, const size_t i, const size_t j);
int gsl_matrix_char_swap_columns(gsl_matrix_char * m, const size_t i, const size_t j);
int gsl_matrix_char_swap_rowcol(gsl_matrix_char * m, const size_t i, const size_t j);
int gsl_matrix_char_transpose (gsl_matrix_char * m);
int gsl_matrix_char_transpose_memcpy (gsl_matrix_char * dest, const gsl_matrix_char * src);

char gsl_matrix_char_max (const gsl_matrix_char * m);
char gsl_matrix_char_min (const gsl_matrix_char * m);
void gsl_matrix_char_minmax (const gsl_matrix_char * m, char * min_out, char * max_out);

void gsl_matrix_char_max_index (const gsl_matrix_char * m, size_t * imax, size_t *jmax);
void gsl_matrix_char_min_index (const gsl_matrix_char * m, size_t * imin, size_t *jmin);
void gsl_matrix_char_minmax_index (const gsl_matrix_char * m, size_t * imin, size_t * jmin, size_t * imax, size_t * jmax);

int gsl_matrix_char_equal (const gsl_matrix_char * a, const gsl_matrix_char * b);

int gsl_matrix_char_isnull (const gsl_matrix_char * m);
int gsl_matrix_char_ispos (const gsl_matrix_char * m);
int gsl_matrix_char_isneg (const gsl_matrix_char * m);
int gsl_matrix_char_isnonneg (const gsl_matrix_char * m);

int gsl_matrix_char_add (gsl_matrix_char * a, const gsl_matrix_char * b);
int gsl_matrix_char_sub (gsl_matrix_char * a, const gsl_matrix_char * b);
int gsl_matrix_char_mul_elements (gsl_matrix_char * a, const gsl_matrix_char * b);
int gsl_matrix_char_div_elements (gsl_matrix_char * a, const gsl_matrix_char * b);
int gsl_matrix_char_scale (gsl_matrix_char * a, const double x);
int gsl_matrix_char_add_constant (gsl_matrix_char * a, const double x);
int gsl_matrix_char_add_diagonal (gsl_matrix_char * a, const double x);

/***********************************************************************/
/* The functions below are obsolete                                    */
/***********************************************************************/
int gsl_matrix_char_get_row(gsl_vector_char * v, const gsl_matrix_char * m, const size_t i);
int gsl_matrix_char_get_col(gsl_vector_char * v, const gsl_matrix_char * m, const size_t j);
int gsl_matrix_char_set_row(gsl_matrix_char * m, const size_t i, const gsl_vector_char * v);
int gsl_matrix_char_set_col(gsl_matrix_char * m, const size_t j, const gsl_vector_char * v);
/***********************************************************************/

/* inline functions if you are using GCC */

INLINE_DECL char   gsl_matrix_char_get(const gsl_matrix_char * m, const size_t i, const size_t j);
INLINE_DECL void    gsl_matrix_char_set(gsl_matrix_char * m, const size_t i, const size_t j, const char x);
INLINE_DECL char * gsl_matrix_char_ptr(gsl_matrix_char * m, const size_t i, const size_t j);
INLINE_DECL const char * gsl_matrix_char_const_ptr(const gsl_matrix_char * m, const size_t i, const size_t j);

#ifdef HAVE_INLINE
INLINE_FUN 
char
gsl_matrix_char_get(const gsl_matrix_char * m, const size_t i, const size_t j)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(1)) 
    {
      if (i >= m->size1)
        {
          GSL_ERROR_VAL("first index out of range", GSL_EINVAL, 0) ;
        }
      else if (j >= m->size2)
        {
          GSL_ERROR_VAL("second index out of range", GSL_EINVAL, 0) ;
        }
    }
#endif
  return m->data[i * m->tda + j] ;
} 

INLINE_FUN 
void
gsl_matrix_char_set(gsl_matrix_char * m, const size_t i, const size_t j, const char x)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(1)) 
    {
      if (i >= m->size1)
        {
          GSL_ERROR_VOID("first index out of range", GSL_EINVAL) ;
        }
      else if (j >= m->size2)
        {
          GSL_ERROR_VOID("second index out of range", GSL_EINVAL) ;
        }
    }
#endif
  m->data[i * m->tda + j] = x ;
}

INLINE_FUN 
char *
gsl_matrix_char_ptr(gsl_matrix_char * m, const size_t i, const size_t j)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(1)) 
    {
      if (i >= m->size1)
        {
          GSL_ERROR_NULL("first index out of range", GSL_EINVAL) ;
        }
      else if (j >= m->size2)
        {
          GSL_ERROR_NULL("second index out of range", GSL_EINVAL) ;
        }
    }
#endif
  return (char *) (m->data + (i * m->tda + j)) ;
} 

INLINE_FUN 
const char *
gsl_matrix_char_const_ptr(const gsl_matrix_char * m, const size_t i, const size_t j)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(1)) 
    {
      if (i >= m->size1)
        {
          GSL_ERROR_NULL("first index out of range", GSL_EINVAL) ;
        }
      else if (j >= m->size2)
        {
          GSL_ERROR_NULL("second index out of range", GSL_EINVAL) ;
        }
    }
#endif
  return (const char *) (m->data + (i * m->tda + j)) ;
} 

#endif

__END_DECLS

#endif /* __GSL_MATRIX_CHAR_H__ */


#endif /* __GSL_MATRIX_H__ */

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif

__BEGIN_DECLS

typedef struct 
{
  size_t n; /* number of observations */
  size_t p; /* number of parameters */
  gsl_matrix * A;
  gsl_matrix * Q;
  gsl_matrix * QSI;
  gsl_vector * S;
  gsl_vector * t;
  gsl_vector * xt;
  gsl_vector * D;
} 
gsl_multifit_linear_workspace;

gsl_multifit_linear_workspace *
gsl_multifit_linear_alloc (size_t n, size_t p);

void
gsl_multifit_linear_free (gsl_multifit_linear_workspace * work);

int
gsl_multifit_linear (const gsl_matrix * X,
                     const gsl_vector * y,
                     gsl_vector * c,
                     gsl_matrix * cov,
                     double * chisq,
                     gsl_multifit_linear_workspace * work);

int
gsl_multifit_linear_svd (const gsl_matrix * X,
                         const gsl_vector * y,
                         double tol,
                         size_t * rank,
                         gsl_vector * c,
                         gsl_matrix * cov,
                         double *chisq, 
                         gsl_multifit_linear_workspace * work);

int
gsl_multifit_linear_usvd (const gsl_matrix * X,
                          const gsl_vector * y,
                          double tol,
                          size_t * rank,
                          gsl_vector * c,
                          gsl_matrix * cov,
                          double *chisq, 
                          gsl_multifit_linear_workspace * work);

int
gsl_multifit_wlinear (const gsl_matrix * X,
                      const gsl_vector * w,
                      const gsl_vector * y,
                      gsl_vector * c,
                      gsl_matrix * cov,
                      double * chisq,
                      gsl_multifit_linear_workspace * work);

int
gsl_multifit_wlinear_svd (const gsl_matrix * X,
                          const gsl_vector * w,
                          const gsl_vector * y,
                          double tol,
                          size_t * rank,
                          gsl_vector * c,
                          gsl_matrix * cov,
                          double *chisq, 
                          gsl_multifit_linear_workspace * work);

int
gsl_multifit_wlinear_usvd (const gsl_matrix * X,
                           const gsl_vector * w,
                           const gsl_vector * y,
                           double tol,
                           size_t * rank,
                           gsl_vector * c,
                           gsl_matrix * cov,
                           double *chisq, 
                           gsl_multifit_linear_workspace * work);

int
gsl_multifit_linear_est (const gsl_vector * x,
                         const gsl_vector * c,
                         const gsl_matrix * cov, double *y, double *y_err);

int
gsl_multifit_linear_residuals (const gsl_matrix *X, const gsl_vector *y,
                               const gsl_vector *c, gsl_vector *r);

__END_DECLS

#endif /* __GSL_MULTIFIT_H__ */
