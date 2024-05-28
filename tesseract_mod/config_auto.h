#ifndef __CONFIG_AUTO_H__

/* willus mod */
#if (defined(__NEON__) || defined(__ARM_NEON__))
#ifndef __ARM_NEON
#define __ARM_NEON
#endif
#ifndef HAVE_NEON
#define HAVE_NEON
#endif
#endif

#ifdef __AVX__
#ifndef AVX
#define AVX
#endif
#ifndef HAVE_AVX
#define HAVE_AVX
#endif
#endif

#ifdef __AVX2__
#ifndef AVX2
#define AVX2
#endif
#ifndef HAVE_AVX2
#define HAVE_AVX2
#endif
#endif

#ifdef __FMA__
#ifndef FMA
#define FMA
#endif
#ifndef HAVE_FMA
#define HAVE_FMA
#endif
#endif

#ifdef __SSE4_1__
#ifndef SSE4_1
#define SSE4_1
#endif
#ifndef HAVE_SSE4_1
#define HAVE_SSE4_1
#endif
#endif

#ifndef FAST_FLOAT
#define FAST_FLOAT
#endif

#ifndef GRAPHICS_DISABLED
#define GRAPHICS_DISABLED
#endif

#ifdef OPENMP_BUILD
#undef OPENMP_BUILD
#endif

#ifdef DISABLED_LEGACY_ENGINE
#undef DISABLED_LEGACY_ENGINE
#endif

#if defined(AVX) || defined(AVX2) || defined(FMA) || defined(SSE4_1)
# define HAS_CPUID
#endif

#define __CONFIG_AUTO_H__
#endif
