/*
** willus.h  Master include file for willus.com general purpose library.
**
** Part of willus.com general purpose C code library.
**
** Copyright (C) 2014  http://willus.com
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU Affero General Public License as
** published by the Free Software Foundation, either version 3 of the
** License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU Affero General Public License for more details.
**
** You should have received a copy of the GNU Affero General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
**
*/
#ifndef __INCLUDED_WILLUS_H__
#define __INCLUDED_WILLUS_H__

/*
** Turbo C:         __TURBOC__
** MS-DOS GNU C:    __GO32__
** Microsoft C:     _MSC_VER
** Win32:           _WIN32
**
** My predefs:
** UNIX:      Unix includes available
** UNIXPURE:  Unix and not DJGPP
** DJGPP:     DJ's 32-bit C++ for MS-DOS/Win32
** CYGWIN32:  Cygnus environment
** EMX:       Rainer Schnitker's 32-bit C++ for MS-DOS/Win32
** RSXNT:     Rainer Schnitker's NT/EMX extensions
** DJEMX:     DJGPP || EMX
** WIN32:     Win32 (MinGW or EMX/RSXNT or DJGPP/RSXNT or MS Visual C++)
**            (also defined for WIN64)
** WIN64:     MinGW 64-bit
** DJEMXWIN32: Win32 and (EMX || DJGPP)
** MSWIN32:   Microsoft Win32
** MSDOS:     Turbo C or GNU C on PC (but NOT Visual C++)
** MSDOS16:   Turbo C only
** NEEDSTRICMP:  No stricmp type functions provided
** MINGW:     If MinGW
** WILLUS_X86:   Any Intel/AMD processor
** WILLUS_X863264:  Any 32-bit or 64-bit Intel or AMD processor
** BIGENDIAN: If ints/floats are stored big-endian (essentially non-Intel/AMD)
** HAVE_WIN32_API:  If Win32 API calls are available
**
** Digital Mars:  __DM__ or __DMC__ in newer versions.
** Intel:  __INTEL_COMPILER
** Tiny CC:  __TINYC__
**
** OS/X cc predfines:
**     #define __MACH__ 1
**     #define __NATURAL_ALIGNMENT__ 1
**     #define __APPLE__ 1
**     #define __GNUC_MINOR__ 95
**     #define __ppc__ 1
**     #define __GNUC__ 2
**     #define __STDC__ 1
**     #define __DYNAMIC__ 1
**     #define __APPLE_CC__ 926
**     #define __BIG_ENDIAN__ 1
**
** OS/X 10.6.8 gcc 4.2.1 predefines (emulator)
**     #define __MACH__ 1
**
** Linux gcc 4.7.2 predefines: lots, including:
**     #define __linux 1
**     #define __linux__ 1
**     #define __gnu_linux__ 1
**     #define linux 1
**     #define __unix__ 1
**     #define __unix 1
**     #define unix 1
**
*/


typedef double  real;

#if (!defined(CYGWIN32) && defined(__CYGWIN32__))
#define CYGWIN32
#endif

#if (!defined(DJGPP) && (defined(__DJGPP__) || defined(__DJGPP)))
#define DJGPP
#endif

#if (!defined(RSXNT) && (defined(__RSXNT__) || defined(__RSXNT)))
#define RSXNT
#endif

#if (!defined(EMX) && (defined(__EMX__) || defined(__EMX)))
#define EMX
#endif

#if (!defined(DJEMX) && (defined(EMX) || defined(DJGPP)))
#define DJEMX
#endif

#if (!defined(MINGW) && (defined(__MINGW32__) || defined(__MINGW64__)))
#define MINGW
#endif

#if (!defined(UNIX) && (defined(DJEMX) || defined(MINGW) || defined(CYGWIN32) || defined(unix) || defined(__unix) || defined(__MACH__)))
#define UNIX
#endif

#if (!defined(UNIXPURE) && defined(UNIX) && !defined(MINGW) && !defined(DJEMX) && !defined(CYGWIN32))
#define UNIXPURE
#endif

#if (!defined(WIN32) && (defined(__WIN32__) || defined(__WIN32) || defined(_WIN32) || defined(MINGW) || defined(CYGWIN32) || defined(__TINYC__) || defined(__DMC__)))
#define WIN32
#endif

#if (!defined(WIN64) && (defined(__WIN64__) || defined(__WIN64) || defined(_WIN64)))
#define WIN64
#endif

#if (!defined(DJEMXWIN32) && defined(WIN32) && defined(DJEMX))
#define DJEMXWIN32
#endif

#if (!defined(MSWIN32) && (defined(WIN32) && defined(_MSC_VER)))
#define MSWIN32
#endif

#if (!defined(MSDOS) && (defined(DJEMX) || defined(__TURBOC__)))
#define MSDOS
#endif

#if (!defined(MSDOS32) && defined(MSDOS) && defined(DJEMX))
#define MSDOS32
#endif

#if (!defined(MSDOS16) && defined(__TURBOC__))
#define MSDOS16
#endif

#if (!defined(__LABWINCVI) && defined(LABWINCVI))
#define __LABWINCVI__
#endif

#undef NEEDSTRICMP
#if (!defined(MSDOS) && !defined(DJEMX) && !defined(_MSC_VER) && !defined(CYGWIN32) && !defined(__DMC__) && !defined(__LCC__) && !defined(__WATCOMC__) && !defined(__BORLANDC__) && !defined(__MINGW32__) && !defined(__TINYC__))
#define NEEDSTRICMP
#endif

#if (!defined(WILLUS_X863264) && (defined(__i386__) || defined(i386) || defined(__amd64) || defined(__x86_64) || defined(__x86_64__) || defined(__amd64__) || defined(WIN32) || defined(MSDOS32)))
#define WILLUS_X863264
#endif

#if (!defined(WILLUS_X86) && (defined(WILLUS_X863264) || defined(MSDOS) || defined(MSDOS16)))
#define WILLUS_X86
#endif

#if (!defined(WILLUS_BIGENDIAN) && !defined(WILLUS_X86))
#define WILLUS_BIGENDIAN
#endif

#if (defined(__GNUC__) && __GNUC__ >= 4 && __GNUC_MINOR__ > 4)
/* For my code */
#ifndef WILLUS_HAVE_FILE64
#define WILLUS_HAVE_FILE64
#endif
/* ?? */
#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE
#endif
/* gcc on CentOS--see stdio.h */
#ifndef __USE_LARGEFILE64
#define __USE_LARGEFILE64
#endif
#else
#if (defined(WILLUS_HAVE_FILE64))
#undef WILLUS_HAVE_FILE64
#endif
#endif


#if (defined(__linux) || defined(linux) || defined(__linux__))
#define LINUX
#if (defined(WILLUS_HAVE_FILE64) && !defined(_off64_t))
#define _off64_t __off64_t
#endif
#endif

#if (defined(NO_WIN32_API) && defined(HAVE_WIN32_API))
#undef HAVE_WIN32_API
#endif

#if (!defined(NO_WIN32_API) && !defined(HAVE_WIN32_API) && (defined(WIN32) || defined(WIN64)) && defined(WILLUS_X863264))
#define HAVE_WIN32_API
#endif


/*
** THIRD-PARTY LIBRARY OPTIONS:
**     HAVE_Z_LIB
**     HAVE_PNG_LIB
**     HAVE_JPEG_LIB
**     HAVE_JASPER_LIB
**     HAVE_GSL_LIB
**     HAVE_MUPDF_LIB (includes jbig2dec, openjpeg, freetype)
**     HAVE_GHOSTSCRIPT (enables usage of ghostscript dll/sys cmd)
**                      (Ghostscript needs PNG_LIB and Z_LIB.)
**     HAVE_DJVU_LIB
**     HAVE_GOCR_LIB
**     HAVE_LEPTONICA_LIB
**     HAVE_TESSERACT_LIB
**
** COMMENT OUT DEFINE STATEMENTS BELOW AS DESIRED.
**
*/

/*
** CMAKE handles the defines, not this source
** (Mod from Dirk Thierbach, 31-Dec-2013)
*/
#ifdef USE_CMAKE
#include "config.h"
#else /* USE_CMAKE */

#ifndef HAVE_Z_LIB
#define HAVE_Z_LIB
#endif
#ifndef HAVE_PNG_LIB
#define HAVE_PNG_LIB
#endif
#ifndef HAVE_JPEG_LIB
#define HAVE_JPEG_LIB
#endif
#ifndef HAVE_MUPDF_LIB
#define HAVE_MUPDF_LIB
#endif
#ifndef HAVE_GHOSTSCRIPT
#define HAVE_GHOSTSCRIPT
#endif
#ifndef HAVE_DJVU_LIB
#define HAVE_DJVU_LIB
#endif
#ifndef HAVE_GOCR_LIB
#define HAVE_GOCR_LIB
#endif
#ifndef HAVE_LEPTONICA_LIB
#define HAVE_LEPTONICA_LIB
#endif
#ifndef HAVE_TESSERACT_LIB
#define HAVE_TESSERACT_LIB
#endif
/*
** Defines for presence of Jasper and GSL (Gnu Scientific Library).
** Define these if you have these libs.  Default is not to define them.
**
** The Jasper library enables reading of JPEG 2000 files with the
** bmp_read() function in bmp.c.
**
** GSL enables a polynomial fit routine (gslpolyfit.c) which is used by
** various spline-interpolation routines.  If calls to these are made
** without GSL support, the program will abort with an error message.
**
** (K2pdfopt does not need Jasper or GSL.)
**
*/
#ifdef HAVE_JASPER_LIB
/* Don't have Jasper */
#undef HAVE_JASPER_LIB
#endif
#ifdef HAVE_GSL_LIB
/* Don't have GSL */
#undef HAVE_GSL_LIB
#endif

#endif /* USE_CMAKE */
/*
** Consistency check
*/
#if (!defined(HAVE_Z_LIB) && defined(HAVE_PNG_LIB))
#undef HAVE_PNG_LIB
#endif
#if (!defined(HAVE_LEPTONICA_LIB) && defined(HAVE_TESSERACT_LIB))
#undef HAVE_TESSERACT_LIB
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>

#define MAXFILENAMELEN 512
/*
** My somewhat arbitrary limits
*/
#define MAXUTF8PATHLEN  4096
#define MAXUTF16PATHLEN 4096

/* ansi.c */
#ifndef __ANSI_H__
#define ANSI_RED            "\x1b[1m\x1b[31m"
#define ANSI_GREEN          "\x1b[1m\x1b[32m"
#define ANSI_YELLOW         "\x1b[1m\x1b[33m"
#define ANSI_BROWN          "\x1b[0m\x1b[33m"
#define ANSI_BLUE           "\x1b[1m\x1b[34m"
#define ANSI_MAGENTA        "\x1b[1m\x1b[35m"
#define ANSI_CYAN           "\x1b[1m\x1b[36m"
#define ANSI_DARKCYAN       "\x1b[0m\x1b[36m"
#define ANSI_WHITE          "\x1b[1m\x1b[37m"
#define ANSI_NORMAL         "\x1b[0m\x1b[37m"
#define ANSI_SAVE_CURSOR    "\x1b[s"
#define ANSI_RESTORE_CURSOR "\x1b[u"
#define ANSI_CLEAR_TO_END   "\x1b[K"
#define ANSI_BEGIN_LINE     "\x1b[80D"
#define ANSI_UP_ONE_LINE    "\x1b[1A"
#define ANSI_HOME           "\x1b[2J\x1b[0;0;H"
#define __ANSI_H__
#endif

void   ansi_set      (int on);
int    aprintf       (char *fmt,...);
void   wlp_save_status    (void);
void   wlp_restore_status (void);
void   wlp_set_stdout     (int sout,int serr,char *filename,int close_after,
                           int append,int newstream,FILE *str);
int    wlprintf      (char *fmt,...);
int    nprintf       (FILE *f,char *fmt,...);
int    nprintf2      (FILE *f1,FILE *f2,char *fmt,...);
int    afprintf      (FILE *f,char *fmt,...);
int    avprintf      (FILE *f,char *fmt,va_list args);
int    ansi_rows_cols(FILE *f,int *rows,int *cols);

/* array.c */
double array_mean(double *a,int n,double *mean,double *stddev);
double arrayi_mean(int *a,int n,double *mean,double *stddev);
double array_weighted_mean(double *a,double *w,int n);
double array_weighted_stddev(double *a,double *w,int n);
void   array_force_weighted_stddev(double *a,double *w,int n,double newstddev);
void   array_copy(double *dst,double *src,int n);
void   array_fabs(double *a,int n);
void   array_set(double *a,int n,double value);
void   array_scale(double *a,int n,double scale_factor,double offset);
void   array_center(double *a,int n);
double array_rms(double *a,int n);
double array_max(double *a,int n);
double array_min(double *a,int n);
void array_load(double *array,int n,char *loadtype);
void array_sort(double *a,int n);
void array_flipi(int *x,int n);
int  array_sliding_window(double *a,int n,int sw);
double array_findminfitd(double *x,double *y,int n,double dxmax,double maxerr,
                         double *err,int *npf,double *ymin);
int array_findminindexd(double *y,int n);
int array_fft(double *xr,double *xi,int n);
int array_ifft(double *xr,double *xi,int n);

int arrayf_find_max_point(float *x0,float *y0,float *x,float *y,int n);
void arrayf_get_lsq_slope(float *m,float *b,float *x,float *y,int n);
int arrayf_is_linear(float *x,int n);
int arrayf_linearize_xyz(float *x,float *y,float *z,int n,int newn);
int arrayf_max_index(float *x,int n);
float arrayf_maxdev(float *x,int n);
void arrayf_mean(float *a,int n,double *mean,double *stddev);
double arrayf_rms(float *a,int n);
float arrayf_max(float *a,int n);
float arrayf_min(float *a,int n);
void arrayf_sort(float *a,int n);
int arrayf_fft(float *xr,float *xi,int n);
int arrayf_ifft(float *xr,float *xi,int n);

/* bmp.c */
#define WILLUSBITMAP_TYPE_NATIVE       0
#define WILLUSBITMAP_TYPE_WIN32        1
typedef struct
    {
    int     red[256];
    int     green[256];
    int     blue[256];
    unsigned char *data;  /* Top to bottom in native type, bottom to */
                          /* top in Win32 type.                      */
    int     width;      /* Width of image in pixels */
    int     height;     /* Height of image in pixels */
    int     bpp;        /* Bits per pixel (only 8 or 24 allowed) */
    int     size_allocated;
    int     type;  /* See defines above for WILLUSBITMAP_TYPE_... */
    } WILLUSBITMAP;
double bmp_last_read_dpi(void);
void bmp_set_pdf_dpi(double dpi);
double bmp_get_pdf_dpi(void);
void bmp_set_pdf_pageno(int pageno);
int  bmp_get_pdf_pageno(void);
int  bmp_info(char *filename,int *width,int *height,int *bpp,FILE *out);
#ifdef HAVE_PNG_LIB
int  bmp_png_info(char *filename,int *width,int *height,int *bpp,FILE *out);
#endif
int  bmp_bmp_info(char *filename,int *width,int *height,int *bpp,FILE *out);
int  bmp_jpeg_info(char *filename,int *width,int *height,int *bpp);
int  bmp_promote_to_24(WILLUSBITMAP *bmp);
void bmp_convert_to_greyscale(WILLUSBITMAP *bmp);
#define bmp_convert_to_grayscale(bmp) bmp_convert_to_greyscale(bmp)
void bmp_convert_to_greyscale_ex(WILLUSBITMAP *dst,WILLUSBITMAP *src);
#define bmp_convert_to_grayscale_ex(dst,src) bmp_convert_to_greyscale_ex(dst,src)
int  bmp_write(WILLUSBITMAP *bmp,char *filename,FILE *out,int quality);
int  bmp_write_ico(WILLUSBITMAP *bmp,char *filename,FILE *out);
void bmp_fill(WILLUSBITMAP *bmp,int r,int g,int b);
void bmp_set_type(WILLUSBITMAP *bmap,int type);
int  bmp_get_type(WILLUSBITMAP *bmp);
int  bmp_read_bmp8(WILLUSBITMAP *bmap,char *filename,FILE *out);
int  bmp_read_bmp24(WILLUSBITMAP *bmap,char *filename,FILE *out);
#ifdef HAVE_PNG_LIB
int  bmp_read_png(WILLUSBITMAP *bmp,char *filename,FILE *out);
int  bmp_read_png_stream(WILLUSBITMAP *bmp,void *io,int size,FILE *out);
int  bmp_write_png(WILLUSBITMAP *bmp,char *filename,FILE *out);
int  bmp_write_png_stream(WILLUSBITMAP *bmp,FILE *dest,FILE *out);
#endif
#ifdef HAVE_JPEG_LIB
int  bmp_write_jpeg(WILLUSBITMAP *bmp,char *filename,int quality,FILE *out);
void bmp_jpeg_set_std_huffman(int status);
int  bmp_write_jpeg_stream(WILLUSBITMAP *bmp,FILE *dest,int quality,FILE *out);
int  bmp_read_jpeg(WILLUSBITMAP *bmp,char *filename,FILE *out);
int  bmp_read_jpeg_stream(WILLUSBITMAP *bmp,void *infile,int size,FILE *out);
#endif
void bmp8_palette_info(WILLUSBITMAP *bmap,FILE *out);
void bmp8_to_grey(WILLUSBITMAP *bmap);
#define bmp8_to_gray(bmp) bmp8_to_grey(bmp)
void bmp8_to_bw(WILLUSBITMAP *bmap,int thresh);
int  bmp8_greylevel_convert(int r,int g,int b);
#define bmp8_graylevel_convert(r,g,b) bmp8_greylevel_convert(r,g,b)
void bmp_init(WILLUSBITMAP *bmap);
int  bmp_alloc(WILLUSBITMAP *bmap);
int  bmp_bytewidth(WILLUSBITMAP *bmp);
unsigned char *bmp_rowptr_from_top(WILLUSBITMAP *bmp,int row);
void bmp_crop(WILLUSBITMAP *bmp,int x0,int y0_from_top,int width,int height);
void bmp_crop_ex(WILLUSBITMAP *dst,WILLUSBITMAP *src,int x0,int y0_from_top,int width,int height);
void bmp_rotate_fast(WILLUSBITMAP *dst,double degrees,int expand);
int  bmp_rotate_right_angle(WILLUSBITMAP *bmp,int degrees);
int  bmp_rotate_90(WILLUSBITMAP *bmp);
int  bmp_rotate_270(WILLUSBITMAP *bmp);
int  bmp_copy(WILLUSBITMAP *dest,WILLUSBITMAP *src);
double bmp_jpeg_bytes_per_pixel(int quality);
void bmp_flip_horizontal(WILLUSBITMAP *bmp);
void bmp_flip_vertical(WILLUSBITMAP *bmp);
int  bmp_bytewidth_win32(WILLUSBITMAP *bmp);
void bmp_free(WILLUSBITMAP *bmap);
int  bmp_read(WILLUSBITMAP *bmap,char *filename,FILE *out);
void bmp24_reduce_size(WILLUSBITMAP *bmp,int mx,int my);
void bmp24_mixbmps(WILLUSBITMAP *dest,WILLUSBITMAP *src1,WILLUSBITMAP *src2,int level);
void bmp24_flip_rgb(WILLUSBITMAP *bmp);
/* The bmp_..._pix routines assume y=0 as at the BOTTOM of the bitmap! */
void bmp_pix_vald(WILLUSBITMAP *bmp,double x0,double y0,
                  double *r,double *g,double *b);
void bmp_pix_vali(WILLUSBITMAP *bmp,int x0,int y0,int *r,int *g,int *b);
double bmp_grey_pix_vald(WILLUSBITMAP *bmp,double x0,double y0);
#define bmp_gray_pix_vald(bmp,x0,y0) bmp_grey_pix_vald(bmp,x0,y0)
int  bmp_grey_pix_vali(WILLUSBITMAP *bmp,int x0,int y0);
#define bmp_gray_pix_vali(bmp,x0,y0) bmp_grey_pix_vali(bmp,x0,y0)
void bmp_grey_pixel_setd(WILLUSBITMAP *bmp,double x0,double y0,int grey);
#define bmp_gray_pixel_setd(bmp,x0,y0,grey) bmp_grey_pixel_setd(bmp,x0,y0,grey)
void bmp_rgb_pixel_setd(WILLUSBITMAP *bmp,double x0,double y0,int r,int g,int b);
void bmp_grey_pixel_setf(WILLUSBITMAP *bmp,int x,int y,int grey,double f);
#define bmp_gray_pixel_setf(bmp,x,y,grey,f) bmp_grey_pixel_setf(bmp,x,y,grey,f)
void bmp_rgb_pixel_setf(WILLUSBITMAP *bmp,int x,int y,int r,int g,int b,double f);
void bmp_resize(WILLUSBITMAP *bmp,double scalefactor);
void bmp_integer_resample(WILLUSBITMAP *dest,WILLUSBITMAP *src,int n);
/*
** As of Sept. 2013, the floating-point bmp_resample() is only faster than
** the fixed-point version on 64-bit compiles.  For 32-bit Intel (and ARM),
** the fixed-point version is considerably faster.
** (__x86_64 is automatically pre-defined for 64-bit gcc compiles on Intel CPUs.)
*/
#ifdef __x86_64
#define bmp_resample_optimum_performance bmp_resample
#else
#define bmp_resample_optimum_performance bmp_resample_fixed_point
#endif
int  bmp_resample(WILLUSBITMAP *dest,WILLUSBITMAP *src,double x1,double y1,
                  double x2,double y2,int newwidth,int newheight);
int  bmp_resample_fixed_point(WILLUSBITMAP *dest,WILLUSBITMAP *src,double fx1,double fy1,
                              double fx2,double fy2,int newwidth,int newheight);
void bmp_crop_edge(WILLUSBITMAP *bmp);
void bmp_invert(WILLUSBITMAP *bmp);
void bmp_overlay(WILLUSBITMAP *dest,WILLUSBITMAP *src,int x0,int y0_from_top,
                 int *dbgc,int *dfgc,int *sbgc,int *sfgc);
void bmp_contrast_adjust(WILLUSBITMAP *dest,WILLUSBITMAP *src,double contrast);
void bmp_gamma_correct(WILLUSBITMAP *dest,WILLUSBITMAP *src,double gamma);
void bmp_color_xform(WILLUSBITMAP *dest,WILLUSBITMAP *src,unsigned char *newval);
int  bmp_is_grayscale(WILLUSBITMAP *bmp);
#define bmp_is_greyscale(bmp) bmp_is_grayscale(bmp)
void bmp_sharpen(WILLUSBITMAP *dest,WILLUSBITMAP *src);
void bmp_apply_filter(WILLUSBITMAP *dest,WILLUSBITMAP *src,double **filter,
                      int ncols,int nrows);
int bmp_jpeg_get_comments(char *filename,char **memptr,FILE *out);
int bmp_jpeg_set_comments(char *filename,char *buf,FILE *out);
void bmp_find_most_used_colors(WILLUSBITMAP *bmp,int *rr,int *gg,int *bb,
                               double *percent,int nmax,int res);
int bmp_close_to_grey(int r,int g,int b,double threshold);
#define bmp_close_to_gray(r,g,b,threshold) bmp_close_to_grey(r,g,b,threshold)
char *bmp_ansi_code(int r,int g,int b);
char *bmp_color_name(int r,int g,int b);
#ifdef HAVE_JASPER_LIB
int bmp_jasper_read(WILLUSBITMAP *bmp,char *filename,FILE *out);
#endif
void bmp_more_rows(WILLUSBITMAP *bmp,double ratio,int pixval);
double bmp_autostraighten(WILLUSBITMAP *src,WILLUSBITMAP *srcgrey,int white,double maxdegrees,
                        double mindegrees,int debug,FILE *out);
void bmp_apply_whitethresh(WILLUSBITMAP *bmp,int whitethresh);
void bmp_dither_to_bpc(WILLUSBITMAP *bmp,int newbpc);
void bmp_extract(WILLUSBITMAP *dst,WILLUSBITMAP *src,int x0,int y0_from_top,int width,int height);

/* fontrender.c */
void fontrender_set_or(int status);
void fontrender_set_typeface(char *name);
void fontrender_set_fgcolor(int r,int g,int b);
void fontrender_set_bgcolor(int r,int g,int b);
void fontrender_set_pixel_size_ex(double size,double points);
void fontrender_set_pixel_size(double size);
void fontrender_set_justification(int just);
void fontrender_caption(WILLUSBITMAP *bmp,char *s,int justify,FILE *out);
void fontrender_close(void);
void fontrender_render(WILLUSBITMAP *bmp,double x,double y_from_bottom,
                       char *string,int rot,FILE *out);
void fontrender_render_ex(WILLUSBITMAP *bmp,double x,double y_from_bottom,
                          char *string,int rot,
                          int ep_translate,int justify_per_line,
                          int clear_behind_letters,FILE *out);
void fontrender_tweak_string(char *s,int maxwidth,int *nlines);
int fontrender_pixwidth(char *string);

/* math.c */
void   freex         (double **x);
void   freexy        (double **x);
void   freexyz       (double **x);
long   indexx        (float x0,float *x,long n);
long   indexxd       (double x0,double *x,long n);
float  interpxy      (float x0,float *x,float *y,long n);
double interpxyd     (double x0,double *x,double *y,long n);
int    readx         (char *filename,double **x,int *n,FILE *err);
int    readxy        (char *filename,double **x,double **y,int *n,FILE *err);
int    readxy_ex     (char *filename,double **x,double **y,int *n,FILE *err,
                      int ignore_after_semicolon);
int    readxyz       (char *filename,double **x,double **y,double **z,
                      int *n,FILE *err);
int    readxyz_ex    (char *filename,double **x,double **y,double **z,
                      int *n,FILE *err,int ignore_after_semicolon);
void   sort          (float *x,int n);
void   sortd         (double *x,int n);
void   sorti         (int *x,int n);
void   sortxy        (float *x,float *y,int n);
void   sortxyi       (int *x,int *y,int n);
void   sortxyzi      (int *x,int *y,int *z,int n);
void   sortxyd       (double *x,double *y,int n);
void   sortxyz       (float *x,float *y,float *z,int n);
void   wsortxyzd     (double *x,double *y,double *z,int n);
void   sortxyzd      (double *x,double *y,double *z,int n);
void   sortxyzwd     (double *x,double *y,double *z,double *w,int n);
void   sortxyzwvd    (double *x,double *y,double *z,double *w,double *v,int n);
void   sort8d        (double *x,double *y,double *z,double *a,double *b,
                      double *c,double *e,double *f,int n);
void   sort9d        (double *x,double *y,double *z,double *a,double *b,
                      double *c,double *e,double *f,double *g,int n);
void   compressxy    (double *x,double *y,int *n);
void   compressxyz   (double *x,double *y,double *z,int *n);
void   compressx     (double *x,int *n);
double line_segment_dist_3d(double x1,double y1,double z1,
                            double x2,double y2,double z2,
                            double x,double y,double z);
double line_segment_dist_2d(double x1,double y1,double x2,double y2,
                            double x,double y);
double line_segment_dist_2dx(double z0,double r0,double z1,double r1,
                             double z,double r,double *f);
double point_distance_3d(double x1,double y1,double z1,
                         double x2,double y2,double z2);
double point_distance_2d(double x1,double y1,double x2,double y2);
double interp_high_order(double x0,double *x,double *y,int n,int order,
                         int nlocal);
double interp_high_order_dx(double x0,double *x,double *y,int n,int order,
                            double dx);
double interp_high_order_dx_ex(double x0,double *x,double *y,int n,int order,
                               double dx);
double interp_high_order_dxf(double x0,float *x,float *y,int n,int order,
                             double dx);
double interp_high_order_slope(double x0,double *x,double *y,int n,int order,
                               double dx);
double interp_high_order_slopef(double x0,float *x,float *y,int n,int order,
                                double dx);
int willusmath_is_inf(double x);
int willusmath_is_nan(double x);
double hammersley(int index,int base);
void vector_alloc(void **ptr,int element_size,int d1);
void vector_free(void **ptr,int d1);
void vector_2d_alloc(void **ptr,int element_size,int d1,int d2);
void vector_2d_free(void **ptr,int d1,int d2);
void vector_3d_alloc(void **ptr,int element_size,int d1,int d2,int d3);
void vector_3d_free(void **ptr,int d1,int d2,int d3);
void vector_nd_free(void **ptr,int *dims,int n);
void vector_nd_alloc(void **ptr,int element_size,int *dims,int n);
double afmod(double x,real modval);

/* mem.c */
void willus_mem_init(void);
void willus_mem_close(void);
int willus_mem_alloc_warn(void **ptr,int size,char *name,int exitcode);
int willus_mem_realloc_warn(void **ptr,int newsize,char *name,int exitcode);
int willus_mem_realloc_robust_warn(void **ptr,int newsize,int oldsize,char *name,
                                int exitcode);
int  willus_mem_alloc(double **ptr,long size,char *name);
int  willus_mem_realloc(double **ptr,long newsize,char *name);
int  willus_mem_realloc_robust(double **ptr,long newsize,long oldsize,char *name);
void willus_mem_free(double **ptr,char *name);

/* string.c */
void   clean_line    (char *buf);
void   clean_line_end(char *buf);
char  *get_line      (char *buf,int max,FILE *f);
char  *get_line_cf   (char *buf,int max,FILE *f);
int   mem_get_line_cf(char *buf,int maxlen,char *cptr,long *cindex,long csize);
int    in_string     (char *buffer,char *pattern);
int    is_a_number   (char *s);
int    is_an_integer (char *s);
int    get_pos_range (char *s,int *n1,int *n2);
#ifdef NEEDSTRICMP
int    stricmp       (const char *s1,const char *s2);
void   strlwr        (char *s);
int    strnicmp      (const char *s1,const char *s2,int n);
void   strupr        (char *s);
#endif
void pr_short_usage(char *progname,char *usage);
void decimal_string(char *s,double x,int ndigits,int signspace);
void comma_print(char *s,long size);
void comma_dprint(char *s,double size);
int string_wild_match_ignore_case(char *pattern,char *name);
int string_read_doubles(char *buf,double *a,int nmax);
double string_atof(char *s);
int string_read_integers(char *buf,int *a,int nmax);
void clean_quotes(char *s);
int range_to_darray(double *a,int maxsize,char *s);
int range_to_iarray(int *a,int maxsize,char *s);
void envvar_subst(char *dest,char *src);
void double_quote_if_has_spaces(char *s);
void exp_str(char *buf,int maxlen,int decimals,double value);
int structtm_from_datetime(struct tm *date,char *datetime);
int structtm_from_date(struct tm *date,char *datestr);
int structtm_from_time(struct tm *date,char *time);
char *wstrtok(char *s,char *t);
int wide_is_ascii(short *s);
int utf8_is_ascii(char *s);
int wide_is_legal_ascii_filename(short *s);
int wide_strlen(short *s);
void wide_strcpy(short *d,short *s);
char  *wide_to_char(char *d,short *s);
short *char_to_wide(short *d,char *s);
int utf8_to_unicode(int *d,char *s,int maxlen);
int utf8_length(int *s,int n);
char *unicode_to_utf8(char *d,int *s,int n);
int utf8_to_utf16_alloc(void **d,char *s);
int utf16_to_utf8_alloc(void **d,short *s);
int utf8_to_utf16(short *d,char *s,int maxlen);
int utf16_to_utf8(char *d,short *s,int maxlen);
int hexcolor(char *s);


#ifdef WIN32
typedef void wmetafile;
#endif

/* wfile.c */
#define DIR_STRUCT_SIZE 4096
#define WFILE_ARCHIVE   0x0001
#define WFILE_DIR       0x0002
#define WFILE_SYSTEM    0x0004
#define WFILE_HIDDEN    0x0008
#define WFILE_READONLY  0x0010
#define WFILE_SYMLINK   0x0020
typedef struct
        {
        char    fullname[MAXFILENAMELEN];
        char    basename[MAXFILENAMELEN];
        char    path[MAXFILENAMELEN];
        int     attr;
        void   *winhandle;
        void   *ftphandle; /* Used by wininet */
        void   *ihandle;   /* Used by wininet */
        void   *unixdptr;
        char    unixspec[MAXFILENAMELEN];
        char    ds[DIR_STRUCT_SIZE];
        struct tm date;
        double  size;
        } wfile;

typedef struct tag_filist
    {
    int     dirsearch;
    wfile   wf;
    struct tag_filist *next;
    } FILIST;

typedef struct
    {
    char    fullname[MAXFILENAMELEN];
    char    basename[MAXFILENAMELEN];
    char    path[MAXFILENAMELEN];
    char    directory[MAXFILENAMELEN];
    char    initpath[MAXFILENAMELEN];
    char    initspec[MAXFILENAMELEN];
    int     recursive;
    FILIST *toplist;
    } RFIND;

#ifdef WILLUS_HAVE_FILE64
#define wfile_tell(f) ((long long)(ftello64(f)))
#define wfile_seek(f,pos,offtype) fseeko64(f,(_off64_t)(pos),offtype)
#else
#define wfile_tell(f) ftell(f)
#define wfile_seek(f,pos,offtype) fseek(f,pos,offtype)
#endif
int wfile_is_archive(char *filename);
double wfile_file_age_secs(char *filename);
double wfile_date_diff(struct tm *d2,struct tm *d1);
int wfile_be_read(void *ptr,int elsize,int nobj,FILE *f);
int wfile_be_write(void *ptr,int elsize,int nobj,FILE *f);
int wfile_ascii(char *filename,int maxcheck);
int wfile_is_zipfile(char *filename);
char *wfile_ext(char *filename);
void wfile_newext(char *dst,char *src,char *ext);
void wfile_stripext(char *dst,char *src);
char *wfile_getenv(char *envvar);
int wfile_findfirst(const char *spec,wfile *wptr);
int wfile_findnext(wfile *wptr);
void wfile_findclose(wfile *wptr);
int wfile_date(const char *filename,struct tm *filedate);
void wfile_set_mod_date(char *filename,struct tm *date);
void wfile_date_add_seconds(struct tm *date,double secs);
void wfile_date_add_hours(struct tm *date,int nhours);
#define wfile_date_increment_hour wfile_increment_hour
#define wfile_date_decrement_hour wfile_decrement_hour
/*
void wfile_date_increment_hour(struct tm *date);
void wfile_date_decrement_hour(struct tm *date);
*/
int wfile_is_symlink_ex(char *filename,char *src);
int wfile_is_symlink(char *filename);
int wfile_is_regular_file(char *filename);
int wfile_saved_during_daylight_savings(char *filename);
void wfile_increment_hour(struct tm *ts);
void wfile_increment_day(struct tm *ts);
void wfile_decrement_hour(struct tm *ts);
void wfile_decrement_day(struct tm *ts);
int wfile_days_in_month(struct tm *ts);
int wfile_days_since_jan_1_1900(struct tm *date);
int wfile_days_in_year(int year);
int wfile_leap_year(int year);
int wfile_set_wd(char *path);
char *wfile_get_wd(void);
void wfile_volumeinfo(char *drive,char *volume,char *sn,char *filesys,
                      int *maxnamelen);
double wfile_freespace(char *volume,double *totalspace);
int wfile_prepdir(char *filename);
int wfile_makedir(char *path);
int wfile_dir_is_empty(char *dir);
int wfile_remove_dir(char *path,int recursive);
int wfile_remove_dir_file_by_file(char *path,int recursive);
int wfile_strong_remove(char *filename);
int wfile_strong_rmdir(char *dirname);
int wfile_check_file_64bit(char *filename);
int wfile_absolute_path(char *path);
void wfile_make_absolute(char *path);
void wfile_written_info(char *filename,FILE *out);
int wfile_newer(char *filename1,char *filename2);
int wfile_datecomp(struct tm *d1,struct tm *d2);
int wfile_filename_compare(char *fn1,char *fn2);
int wfile_filename_basename_compare(char *fn1,char *fn2);
int wfile_status(char *filename);
void wfile_relative_basename(char *basename,char *fullname,char *dir);
void wfile_up_one(char *filename);
void wfile_remove_dots(char *path);
void wfile_fullname(char *fullname,char *path,char *spec);
void wfile_fullname_zipex(char *fullname,char *path,char *spec);
char *wfile_nullfile(void);
void wfile_zipex_cleanup(void);
void wfile_basepath(char *dst,const char *src);
void wfile_basespec(char *dst,char *src);
void wfile_addwild(char *dst,char *src);
void wfile_addslash(char *dst);
void wfile_goodpath(char *dst,char *src);
void wfile_expandname(char *expanded,char *filename);
void wfile_noslash(char *dst,char *src);
void wfile_unique_part(char *filename,char *path);
void wfile_reslash(char *filename);
void wfile_slash_this_way(char *filename,int slash);
char *wfile_temppath(char *path);
void wfile_temppath_from_env(char *dir);
char *wfile_tempname(char *dir,char *prefix);
void wfile_abstmpnam(char *filename);
int wfile_hushit(char *filename);
int wfile_eitherslash(int c);
int wfile_slash(int c);
double wfile_size(char *filename);
int wfile_copy_file(char *destname,char *srcname,int append);
int wfile_find_in_path(char *dest,char *src,char *path);
int wfile_shorten_ascii(char *filename,char *pattern,int maxlen,
                        int desired_len);
int wfile_unix_style_match(char *pattern,char *name);
int wfile_wild_match_ignore_case(char *pattern,char *name);
int wfile_wild_match(char *pattern,char *name);
int wfile_recfindfirst(char *path,RFIND *rf,int rec);
int wfile_recfindnext(RFIND *rf);
void wfile_recfindclose(RFIND *rf);
int wfile_findfirstdir(char *spec,wfile *wf);
int wfile_findnextdir(wfile *wf);
#ifdef WILLUS_HAVE_FILE64
long long wfile_seek_to(FILE *f,char *pattern);
#else
long wfile_seek_to(FILE *f,char *pattern);
#endif
void wfile_touch(char *filename);
int wfile_symlink_size(char *filename);
int wfile_symlink_date(const char *filename,struct tm *filedate);
FILE *wfile_open_most_recent(char *wildspec,char *mode,int recursive);
int wfile_extract_in_place(char *filename);
int wfile_find_file(char *fullname,char *basename,char *folderlist[],char *drives,
                    int checkpath,int cwd,int exedir,char *envdir);
int wfile_smartfind(char *fullname,char *basename,char *folder,int recursive);
void wfile_remove_file_plus_parent_dir(char *tempfile);
FILE *wfile_fopen_utf8(char *filename,char *mode);
int wfile_remove_utf8(char *filename);
int wfile_rename_utf8(char *filename1,char *filename2);

/* wzfile.c */
typedef struct
    {
    void *f;
    int   type;  /* 0 = regular, 1 = zlib file */
    } WZFILE;
WZFILE *wzopen_special(char *archfile,char *filename,char *tempname);
int     wfile_is_binary(char *filename,int maxlen);
WZFILE *wzopen(char *filename,char *mode);
void    wzfile_date(char *filename,struct tm *date);
int     wzclose(WZFILE *wz);
char   *wzgets(char *buf,int maxlen,WZFILE *wz);
int     wzgetc(WZFILE *wz);
int     wzputc(WZFILE *wz,int c);
int     wzread(WZFILE *wz,void *ptr,int length);
int     wzwrite(WZFILE *wz,void *ptr,int length);
#ifdef WILLUS_HAVE_FILE64
int     wzseek(WZFILE *wz,long long position);
int     wzseek2(WZFILE *wz,long long position,int type);
long long wztell(WZFILE *wz);
#else
int     wzseek(WZFILE *wz,long position);
int     wzseek2(WZFILE *wz,long position,int type);
long    wztell(WZFILE *wz);
#endif
void    wzrewind(WZFILE *wz);
int     wfile_is_gzfile(char *filename);
int     wzbe_read(WZFILE *wz,void *ptr,int elsize,int nobj);
int     wzbe_write(WZFILE *wz,void *ptr,int elsize,int nobj);
int     wzerror(WZFILE *wz);
int     wzprintf(WZFILE *wz,char *fmt,...);
int     wzcompressed(WZFILE *wz);
WZFILE *wzuncompressed(FILE *out);

/* dtcompress.c */
/* From Dirk Thierbach, 31-Dec-2013, avoids custom mod to Z-lib */
typedef void *compress_handle;
compress_handle compress_start(FILE* f, int level);
void compress_done(FILE* f, compress_handle *h);
size_t compress_write(FILE* f, compress_handle h, const void *buf, size_t size);
    
/* win.c */
#ifdef HAVE_WIN32_API
void *win_activewin(void);
void *win_hinstance(void);
char *win_full_exe_name(char *s);
char *win_lasterror(void);
int win_grant_full_file_access(char *filename);
void win_launch(char *exename,char *cmdlineopts);
void win_launch_detail(char *exename,char *cmdlineopts,
                       int showstyle,int priority);
int process_launch(char *exename,char *cmdlineopts,int inherits,
                    int detached,char *pwd,int flags);
void process_close_handles(int index);
int process_launch_ex_ii(char *command,char *cmdlineopts,int inherits,
                         int detached,char *pwd,int flags,int *pnum,
                         char *stdinfile,char *stdoutfile,char *stderrfile);
int process_launch_ex(char *command,char *cmdlineopts,int inherits,
                      int detached,char *pwd,int flags,int *pnum);
int detail_process(char *exename,char *cmdlineopts,int inherits,
                   int swflags,int dwflags,int cflags,char *pwd);
int win_createprocess_utf8(char *exename,char *cmdline,int inherits,int cflags,
                           char *pwd,void *si,void *pi);
int process_done(int *exitcode);
int process_done_ex(int procnum,int *exitcode);
int win_terminate_process(int procnum);
void *win_process_handle(void);
void win_sleep(int ms);
int win_setdir(char *directory);
wmetafile *win_emf_clipboard(void);
int win_text_file_to_clipboard(char *filename,FILE *out);
int win_buf_to_clipboard(char *lbuf,FILE *out);
char *win_clipboard_to_buf(FILE *out);
int win_clipboard_has_bitmap(void);
int win_clipboard_has_text(void);
void win_clipboard_close(void);
wmetafile *win_emf_create(double width_in,double height_in);
wmetafile *win_emf_close_created_metafile(wmetafile *wmf);
void win_emf_clipboard_close(void);
wmetafile *win_emf_from_metafile(char *metafile);
wmetafile *win_emf_from_file(char *filename);
int win_emf_write_to_file(wmetafile *wmf,char *filename);
int win_emf_write(wmetafile *wmf,FILE *f);
void win_emf_close(wmetafile *wmf,int close_clipboard);
int win_emf_write_prn(wmetafile *wmf,char *printer,char *psfile,
                      int *left,int *bottom,int *right,int *top);
int win_emf_into_emf(wmetafile *dest,wmetafile *src,
                     double dist_from_left_in,double dist_from_top_in,
                     double scale_factor);
void *win_create(char *title,int xpos,int ypos,int width,int height,
                 int scrollbars,void *callback);
int win_still_open(void *winptr);
void win_update(void *handle);
void win_set_foreground(void *handle);
void win_destroy(void *handle);
int win_get_priority(void);
int win_set_priority(int pri);
int win_copy_file(char *destfile,char *srcfile);
int win_fileattr_to_wfile(int winattr);
void win_windate_to_tm(struct tm *filedate,void *wtime);
void win_windate_to_tm_direct(struct tm *filedate,void *wtime);
void win_tm_to_windate(void *wtime,struct tm *filedate);
int win_file_is_ntfs(char *filename);
void win_ntfs_date_to_proper_date(struct tm *date);
void win_proper_date_to_ntfs_date(struct tm *date);
void win_file_windate_to_tm(struct tm *filedate,void *wtime,char *filename);
void win_file_windate_to_tm_direct(struct tm *filedate,void *wtime,char *filename);
void win_tm_to_file_windate(void *fT,struct tm *dd,char *filename);
void win_set_filetime(char *filename,struct tm *date);
void win_set_mod_filetime(char *filename,struct tm *date);
int win_which(char *exactname,char *exename);
int win_most_recent_in_path(char *exactname,char *wildcard);
void *win_thread_create(void *funcptr,void *data);
void win_thread_close(void *thread_id);
int win_thread_terminate(void *thread_id,int exitcode);
void win_thread_exit(int exitcode);
/* voids here are Win32 FILETIME ptrs */
int win_getfiletimes(void *atime,void *ctime,void *mtime,
                     char *filename);
int win_setfiletimes(void *atime,void *ctime,void *mtime,
                     char *filename);
void win_filetime2tm(struct tm *filedate,void *ftime);
void win_tm2filetime(void *_ftime,struct tm *date);
void *win_start_app_get_win(char *syscmd,char *wname,double sleeptime,
                            int ntries,FILE *out);
void *win_find_window(char *wname);
int win_kill_by_name(char *title);
int win_kill_pid(int pid);
void win_get_desktop_directory(char *desktop,int maxlen);
int win_get_path(char *path,int maxlen,int syspath);
int win_set_path(char *path,int syspath);
int win_registry_search(char *data,int maxlen,char *basename,char *keyroot,int recursive);
int win_get_user_and_domain(char *szUser,int maxlen,char *szDomain,int maxlen2);
int win_has_own_window(void);
int win_symlink(char *linkname,char *target,int maxlen,int *linksize);
int win_textout(void *hdc,int x,int y,char *s);
int win_textout_utf8(void *hdc,int x,int y,char *s);
int win_gettextextentpoint_utf8(void *hdc,char *s,long *dx,long *dy);
int win_close_handle(void *handle);
void *win_shared_handle_utf8(char *filename);
#endif

/* winshell.c */
#ifdef HAVE_WIN32_API
int win_resolve_shortcut(void *shortcut,void *target,int maxlen,int wide);
int winshell_get_foldername(char *foldername,char *title);
int winshell_get_foldernamew(short *foldername,char *title);
#endif

/* winmbox.c */
#ifdef HAVE_WIN32_API
int winmbox_message_box(void *parent,char *title,char *message,char *button1,
                        char *button2,char *button3,char *inbuf,int maxlen,
                        int fontsize_pixels,int maxwidth_pixels,int rgbcolor,
                        void *winproc,void **winhandle,int *bcolors);
int winmbox_message_box_ex(void *parent,char *title,char *message,
                        char *button1,char *button2,char *button3,
                        char *inbuf,int maxlen,int fontsize_pixels,
                        int maxwidth_pixels,int rgbcolor,void *myproc,
                        void **window,int *button_colors,void *aboutbox);
int winmbox_message_box_ex2(void *parent,char *title,char *message,
                        char *button1,char *button2,char *button3,
                        char *inbuf,int maxlen,int fontsize_pixels,
                        int maxwidth_pixels,int rgbcolor,void *myproc,
                        void **window,int *button_colors,void *aboutbox,
                        int modal);
int winmbox_def_proc(void *hwnd,int iMsg,int wParam,void *lParam);
void winmbox_destroy(void);
void winmbox_message_box_display_message(char *message,int *ypos);
void winmbox_checkbox_button_draw(void *hdc0,void *rect0,int state,void *hfont0,
                                  char *text,int textcolorrgb,int checked,
                                  WILLUSBITMAP *bgbmp,int x0,int y0);
void winmbox_button_draw(void *hdc0,void *rect0,int state,int basecolorrgb,
                         void *hfont0,char *text,int textcolorrgb);
#endif

/* winbmp.c */
#ifdef HAVE_WIN32_API
int  win_clipboard_to_bmp(WILLUSBITMAP *bmp,FILE *out);
int  win_emf_dims(char *filename,double *width_in,double *height_in);
int  win_emf_to_bmp(wmetafile *wmf,int dpi,WILLUSBITMAP *bmp,FILE *out);
int  win_emf_write_gif(wmetafile *wmf,int dpi,char *giffile,int quiet);
int  bmp_show_bmp(WILLUSBITMAP *bmp,void *handle,int x0,int y0);
int  bmp_show_bmp_ex(WILLUSBITMAP *bmp,void *handle,int x0,int y0,int update);
void bmp_blit_to_hdc(WILLUSBITMAP *bmp,void *hdc,int x0,int y0);
void bmp_blit_to_hdc_ex(WILLUSBITMAP *bmp,void *hdc,int x0,int y0,int width,int height,
                        int xs,int ys);
int  bmp_get_bmp(WILLUSBITMAP *bmp,void *handle,int x0,int y0);
void bmp8_win32_palette_init(WILLUSBITMAP *bmp);
void bmp8_win32_palette_set(WILLUSBITMAP *bmp);
void bmp8_win32_palette_free(WILLUSBITMAP *bmp);
void win_icon_free(void *icon);
void win_icons_from_exe(void **iconr,void **smalliconr);
#endif

/* wincomdlg.c */
#ifdef HAVE_WIN32_API
int wincomdlg_get_filename(char *filename,int maxlen,char *filter,char *title,char *defext,
                           int multiselect,int must_exist,int for_writing);
int wincomdlg_get_filenamew(short *filename,int maxlen,char *filter,char *title,char *defext,
                            int multiselect,int must_exist,int for_writing);
#endif

/* wsys.c */
#define system_header(progname,verstring,author) \
    { char os[32],chip[32],compiler[32]; \
    system_version(NULL,os,chip,compiler); \
    aprintf(ANSI_CYAN "%s %s" ANSI_NORMAL "%s%s (compiled %s, %s)\n" \
        "    Compiled by %s for %s on %s\n\n", \
       progname,verstring,author==NULL || author[0]=='\0' ? "":" ", \
       author==NULL ? "" : author,__TIME__,__DATE__,compiler,os,chip); }
#define WPID_NO_PROCESS  0
#define WPID_RUNNING     1
#define WPID_SLEEPING    2
#define WPID_UNKNOWN     3
void   wsys_system_version(char *system,char *_os,char *_chip,char *_compiler);
int    wsys_win32_api(void);
int    wsys_wpid_status(int wpid);
void   wsys_sleep(int secs);
char  *wsys_full_exe_name(char *s);
void   wsys_append_nul_redirect(char *s);
int    wsys_which(char *exactname,char *exename);
int    wsys_most_recent_in_path(char *exename,char *wildcard);
void   wsys_computer_name(char *name,int maxlen);
int    wsys_filename_8dot3(char *dst,char *src,int maxlen);
double wsys_year_double(struct tm *date);
double wsys_utc_offset(void);
char  *wsys_utc_string(void);
void   wsys_enter_to_exit(char *mesg);
int    wsys_set_decimal_period(int setit);
int    wsys_set_envvar(char *varname,char *value,int system);
int    wsys_get_envvar_ex(char *varname,char *value,int maxlen);


/* token.c */
typedef struct
    {
    char *p;
    int index;
    int i;
    char **argv;
    int argc;
    char cmdarg[1024];
    } CMDLINEINPUT;
void cmdlineinput_init(CMDLINEINPUT *cl,int argc,char *argv[],char *opt_string);
char *cmdlineinput_next(CMDLINEINPUT *cl);
int  token_next_from_string(char *dst,char *src,int *index,int max);
void token_set_white(char *s);
void token_set_escapes(int status);
int  token_next(FILE *f,char *string,int max,int sameline,int *linenum,int *quoted);


/* filelist.c */
typedef struct
    {
    char   *name;       /* v3.18 change */
    struct tm date;     /* File date */
    double  size;       /* File size */
    int     attr;       /* File attr */
    } FLENTRY;

typedef struct
    {
    char    dir[MAXFILENAMELEN];  /* Base directory, doesn't end in a slash */
    FLENTRY *entry;
    size_t  n;
    size_t  nmax;       /* Max number of FLENTRY before realloc needed */
    int     sorted;     /* 0 = unsorted, 1=by name, 2=by date */
    size_t  bytes_allocated;  /* Total bytes allocated */
    char   *databuf;    /* Beginning of data buffer = string space followed */
                        /* by FLENTRY space.                                */
    size_t  nc;         /* Number of string chars being used. */
    size_t  ncmax;      /* Max number of strings before realloc needed */
    } FILELIST;

void filelist_convert_symlink_sizes(FILELIST *fl);
double filelist_total_bytes(FILELIST *fl);
void filelist_redir(FILELIST *fl,char *newdir);
void filelist_copy(FILELIST *dst,FILELIST *src);
void filelist_hushlist(FILELIST *fl);
void filelist_hushlist_fast(FILELIST *fl);
void filelist_diff(FILELIST *newf,FILELIST *arch,FILELIST *disk);
void filelist_diff_in_situ_fast(FILELIST *dst,FILELIST *src);
int flentry_different(FLENTRY *e1,FLENTRY *e2);
void filelist_combine(FILELIST *newf,FILELIST *src1,FILELIST *src2,int newer);
void filelist_combine_in_situ(FILELIST *dst,FILELIST *src,int newer);
int flentry_index(FILELIST *fl,FLENTRY *en);
void filelist_sort_by_date(FILELIST *fl);
void filelist_sort_by_size(FILELIST *fl);
void filelist_sort_by_name(FILELIST *fl);
void filelist_sort_by_basename(FILELIST *fl);
void filelist_sort_by_name_index1(FILELIST *fl);
void filelist_sort_by_name_index2(FILELIST *fl);
int filelist_name_index1(char *s);
int filelist_name_index2(char *s);
int filelist_span_days(FILELIST *fl);
void filelist_remove_fast(FILELIST *fl,char *pattern);
void filelist_keep_only_fast(FILELIST *fl,char *pattern);
void filelist_keep_only(FILELIST *fl,char *pattern);
void filelist_remove(FILELIST *fl,char *pattern);
int filelist_fill_from_disk(FILELIST *fl,char *dirname,char *include_only[],
                            char *exclude[],int recursive,int dirstoo);
int filelist_fill_from_disk_1(FILELIST *fl,char *filespec,
                              int recursive,int dirstoo);
int filelist_create_zipfile(FILELIST *fl,char *zipfile,FILE *out);
void filelist_date_recursively(FILELIST *fl);
void filelist_zero_seconds(FILELIST *fl);
void filelist_round_seconds(FILELIST *fl);
void filelist_adjust_archive_datestamps_for_dst(FILELIST *fl,char *zipfile);
int filelist_write_tar_list(FILELIST *fl,char *filename);
int filelist_fill_from_zip(FILELIST *fl,char *zipfile,char *wildspec);
int filelist_fill_from_archive(FILELIST *fl,WZFILE *f,int append,int dirstoo);
int filelist_fill_from_archive_ex(FILELIST *fl,WZFILE *f,int append,int dirstoo,
                                  char *prepend,char *include_only[],char *exclude[]);
void filelist_write_zipstyle_list(FILELIST *fl,FILE *out);
void filelist_write_7zstyle_list(FILELIST *fl,FILE *out);
int filelist_add_entry(FILELIST *fl,FLENTRY *entry);
int filelist_write_to_file(FILELIST *fl,char *filename);
int filelist_read_from_file(FILELIST *fl,char *filename);
void filelist_new_entry_name(FILELIST *fl,int index,char *newname);
void filelist_filter(FILELIST *fl,char *include[],char *exclude[]);
void filelist_add_path_dirs(FILELIST *fl);
void filelist_add_dirs_only(FILELIST *fl,char *wildspec);
void filelist_fill_from_dirs(FILELIST *fl,FILELIST *dirlist,char *wildspec);
void filelist_delete_entry(FILELIST *fl,int index);
void filelist_copy_entry(FILELIST *fl,int index,FLENTRY *entry);
void filelist_clear(FILELIST *fl);
void filelist_init(FILELIST *fl);
void filelist_free(FILELIST *fl);
int filelist_use_file(char *fullname,char *include_only[],char *exclude[]);
int filelist_dir_excluded(char *dirname,char *include_only[],char *exclude[]);
int filelist_dir_name_match(char *pattern,char *dirname);
void filelist_remove_files_larger_than(FILELIST *fl,double bytes);
void filelist_reslash(FILELIST *fl);

/* linux.c */
int linux_which(char *exactname,char *exename);
int linux_most_recent_in_path(char *exactname,char *wildcard);


/* point2d.c */
typedef struct
    {
    double  x,y;
    } POINT2D;
typedef POINT2D VECTOR2D;
typedef struct
    {
    POINT2D  p[3];
    } TRIANGLE2D;
typedef struct
    {
    POINT2D p[2];
    } LINE2D;
typedef struct
    {
    double   rgb[3];
    } RENDER_COLOR;
typedef struct
    {
    POINT2D  p[2];
    } RENDER_CLIPBOX;
typedef struct
    {
    double  x,y,z;
    } POINT3D;
typedef struct
    {
    POINT3D p[3];
    } TRIANGLE3D;

#define p2d_same(a,b)  ((a).x==(b).x && (a).y==(b).y)
#define p2d_equal(a,b) ((a).x==(b).x && (a).y==(b).y)
#define p2d_slope(p1,p2) (((p2).y-(p1).y)/((p2).x-(p1).x))
#define p2d_dist(p1,p2) (sqrt(((p2).x-(p1).x)*((p2).x-(p1).x)  \
                          + ((p2).y-(p1).y)*((p2).y-(p1).y)))
#define p2d_x_intercept(y0,p1,p2) ((p1).x+(y0-(p1).y)*((p2).x-(p1).x)/((p2).y-(p1).y))
#define p2d_y_intercept(x0,p1,p2) ((p1).y+(x0-(p1).x)*((p2).y-(p1).y)/((p2).x-(p1).x))
void p2d_swap(POINT2D *a,POINT2D *b);
POINT2D p2d_point(double x,double y);
TRIANGLE2D tri2d_triangle(POINT2D p1,POINT2D p2,POINT2D p3);
int p2d_is_zero(POINT2D *p);
double p2d_magnitude(VECTOR2D *v);
double p2d_sine_angle_between(VECTOR2D *v1,VECTOR2D *v2);
double p2d_angle_between_deg(VECTOR2D *v1,VECTOR2D *v2);
void p2d_min_angles(POINT2D *p,int n,double *mostneg_deg,
                                     double *closest_to_zero_deg);
POINT2D p2d_vector(POINT2D *p1,POINT2D *p2);
int tri2d_point_inside(TRIANGLE2D *tri,POINT2D p);
double tri2d_point_interp(TRIANGLE2D *t,POINT2D p0,double *val);
VECTOR2D p2d_normalize(VECTOR2D v);
void tri2d_sort_ypoints(TRIANGLE2D *tri);
int tri2d_zero_area(TRIANGLE2D *tri);
double tri2d_area(TRIANGLE2D *tri);
int p2d_tri_line_intersect(TRIANGLE2D *tri,double *val,LINE2D *line,
                           POINT2D *pout,double *vout);
int p2d_line_line_intersection(LINE2D *line1,LINE2D *line2,POINT2D *point);
double p2d_point_line_distance(LINE2D *line,POINT2D *point);
void p2d_remove_duplicate_xcoords(POINT2D *p,int *n);
void p2d_sort_by_xcoord(POINT2D *x,int n);
void p2d_sort_by_theta(POINT2D *x,int n);

/* render.c */
#define RENDER_TYPE_SET             1
#define RENDER_TYPE_OR              2
#define RENDER_TYPE_XOR             3
#define RENDER_TYPE_ADD             4
#define RENDER_TYPE_ANTIALIASED     5
void render_init(void);
void render_set_clipbox(double x1,double y1,double x2,double y2);
void render_set_clipbox_pts(double x1,double y1,double x2,double y2);
void render_restore_clipbox(double x1,double y1,double x2,double y2);
void render_get_clipbox(double *x1,double *y1,double *x2,double *y2);
RENDER_COLOR render_color(double r,double g,double b);
void render_circle(WILLUSBITMAP *bmp,double xc,double yc,double radius,
                   int nsteps,RENDER_COLOR *rcolor,RENDER_COLOR *bgcolor,
                   int render_type);
void render_box(WILLUSBITMAP *bmp,double x1,double y1,
                               double x2,double y2,
                               double btx,double bty,
                               RENDER_COLOR *fgc,
                               RENDER_COLOR *bgc,int render_type);
void render_rect(WILLUSBITMAP *bmp,double x1,double y1,
                                double x2,double y2,
                                RENDER_COLOR *fgc,RENDER_COLOR *bgc,
                                int render_type);
void render_triangle(WILLUSBITMAP *bmap,TRIANGLE2D *tri,RENDER_COLOR *rcolor,
                     RENDER_COLOR *bgcolor,int render_type);
int render_clipped_triangle(POINT2D *clippath,int *nc,
                            RENDER_CLIPBOX *box,TRIANGLE2D *triangle);
void render_triangle_3d(WILLUSBITMAP *bmp,double *zbuffer,
                        TRIANGLE3D *srctri,RENDER_COLOR *color,
                        RENDER_COLOR *edge_color);
void rendercolor_rainbow_colorscale(RENDER_COLOR *rcolor,double scale);
int intcolor_from_rendercolor(RENDER_COLOR *rcolor);
RENDER_COLOR rendercolor_from_intcolor(int i);
double rendercolor_diff(RENDER_COLOR *rc1,RENDER_COLOR *rc2);
/* Functions in points */
void render_set_point_size(WILLUSBITMAP *bmp,double width,double height);
void render_set_fg_bg_rtype(RENDER_COLOR *fgc,RENDER_COLOR *bgc,
                                   int render_type);
int render_get_rtype(void);
void render_setlinewidth_pts(double lw);
void render_setlinecap(int lc); /* 0=butt, 1=circle, 2=project */
void render_setdash_pts(double *dash,double offset);
double render_dash_index(void);
void render_position_pixels(double *xpx,double *ypx);
void render_position_pts(double *xpts,double *ypts);
void render_moveto_pts(double x,double y);
void render_dash_index_and_offset(double index,int *ii,double *dashoff);
void render_lineto_pts(double x,double y);
void render_line_pts(double x1,double y1,double x2,double y2);
void render_outline_rect_pts(double x1,double y1,double x2,double y2);
void render_rect_pts(double x1,double y1,double x2,double y2,
                     double x3,double y3,double x4,double y4);
void render_tri_pts(double x1,double y1,double x2,double y2,
                    double x3,double y3);
void render_partial_circle_pts(double xc,double yc,double radius,
                               double theta0,double theta1,int nsteps);

/* willusversion.c */
char  *willuslibversion (void);

/* wgs.c */
#ifdef HAVE_GHOSTSCRIPT
int willusgs_read_pdf_or_ps_bmp(WILLUSBITMAP *bmp,char *filename,int pageno,double dpi,FILE *out);
int willusgs_ps_to_pdf(char *dstfile,char *srcfile,int firstpage,int lastpage,FILE *out);
int willusgs_init(FILE *out);
int willusgs_exec(int argc,char *argv[],FILE *out);
void willusgs_close(void);
#endif

/* ocr.c */
typedef struct
    {
    int r,c;   /* row,column position of left baseline of the word, e.g. the bottom left of */
               /* most capital letters */
    int w,h;   /* width and height of word in pixels */
    double maxheight;  /* max height of any letter from baseline in pixels */
    double lcheight;  /* height of a lowercase letter in pixels */
    int rot;   /* rotation angle of word in degrees */
    int n;     /* Number of chars in word */
    char *text;  /* UTF-8 text of word */

    /* Used by MuPDF */
    double x0,y0; /* Position of top-left of first char of word rel. to top-left of
                     rendered source page (pts) */
    double w0,h0; /* Width and height of word in points */
    double *cpos; /* Position of the right side of each character, horizontally, from the */
                  /* beginning of the word, in points.  cpos[n-1] should be = w0.         */
    double rot0_deg; /* Rotation of source document */
    int pageno; /* Source page number */
    } OCRWORD;

typedef struct
    {
    OCRWORD *word;
    int n,na;
    } OCRWORDS;

void ocrword_init(OCRWORD *word);
void ocrword_free(OCRWORD *word);
void ocrwords_init(OCRWORDS *words);
void ocrword_copy(OCRWORD *dst,OCRWORD *src);
void ocrword_truncate(OCRWORD *word,int i1,int i2);
int  ocrwords_to_textfile(OCRWORDS *words,char *filename,int append);
void ocrwords_add_word(OCRWORDS *words,OCRWORD *word);
void ocrwords_remove_words(OCRWORDS *words,int i1,int i2);
void ocrwords_clear(OCRWORDS *words);
void ocrwords_free(OCRWORDS *words);
void ocrwords_sort_by_pageno(OCRWORDS *words);
void ocrwords_offset(OCRWORDS *words,int dx,int dy);
void ocrwords_scale(OCRWORDS *words,double srat);
void ocrwords_int_scale(OCRWORDS *words,int ndiv);
void ocrwords_concatenate(OCRWORDS *dst,OCRWORDS *src);
void ocr_text_proc(char *s,int allow_spaces);

#ifdef HAVE_GOCR_LIB
/* ocrjocr.c */
void jocr_single_word_from_bmp8(char *text,int maxlen,WILLUSBITMAP *bmp8,
                                int x1,int y1,int x2,int y2,int allow_spaces,
                                int std_proc);
#endif

#ifdef HAVE_TESSERACT_LIB
/* ocrtess.c */
int ocrtess_init(char *datadir,char *lang,FILE *out);
void ocrtess_end(void);
void ocrtess_single_word_from_bmp8(char *text,int maxlen,WILLUSBITMAP *bmp8,
                                int x1,int y1,int x2,int y2,
                                int ocr_type,int allow_spaces,
                                int std_proc,FILE *out);
#endif

/* pdfwrite.c */
typedef struct
    {
    size_t ptr[3];
    int    flags;    /*
                     ** 1 = new page
                     ** 2 = needs parent reference
                     ** 4 = outline head
                     ** 8 = outline title
                     ** 16 = page anchor
                     */
    } PDFOBJECT;

typedef struct wpdfoutline_s
    {
    char *title;
    int srcpage; /* 0 = first page */
    int dstpage; /* 0 = first page */
    struct wpdfoutline_s *next;
    struct wpdfoutline_s *down;
    } WPDFOUTLINE;

typedef struct
    {
    PDFOBJECT *object; /* PDF reference number = index + 1 */
    int n;
    int na;
    int imc;    // Image count
    size_t pae; // Pointer into page type reference
    FILE *f;
    char filename[512];
    } PDFFILE;

FILE *pdffile_init(PDFFILE *pdf,char *filename,int pages_at_end);
void pdffile_close(PDFFILE *pdf);
int  pdffile_page_count(PDFFILE *pdf);
void pdffile_add_outline(PDFFILE *pdf,WPDFOUTLINE *outline);
void pdffile_add_bitmap(PDFFILE *pdf,WILLUSBITMAP *bmp,double dpi,int quality,int halfsize);
void pdffile_add_bitmap_with_ocrwords(PDFFILE *pdf,WILLUSBITMAP *bmp,double dpi,
                                      int quality,int halfsize,OCRWORDS *ocrwords,
                                      int ocr_render_flags);
void pdffile_finish(PDFFILE *pdf,char *title,char *author,char *producer,char *cdate);
int  pdf_numpages(char *filename);
void ocrwords_box(OCRWORDS *ocrwords,WILLUSBITMAP *bmp);
void wpdfoutline_init(WPDFOUTLINE *wpdfoutline);
void wpdfoutline_init(WPDFOUTLINE *wpdfoutline);
void wpdfoutline_free(WPDFOUTLINE *wpdfoutline);
void wpdfoutline_set_dstpage(WPDFOUTLINE *outline,int srcpage,int dstpage);
int  wpdfoutline_includes_srcpage(WPDFOUTLINE *outline,int pageno,int level);
void wpdfoutline_echo(WPDFOUTLINE *outline,int level,int count,FILE *out);
void wpdfoutline_echo2(WPDFOUTLINE *outline,int level,FILE *out);
int  wpdfoutline_fill_in_blank_dstpages(WPDFOUTLINE *outline,int pageno);
WPDFOUTLINE *wpdfoutline_read_from_text_file(char *filename);
int  wpdf_docenc_from_utf8(char *dst,char *src_utf8,int maxlen);

/* wpdf.c */
/* PDF file support functions--no depedence on MuPDF */
typedef struct
    {
    int pageno;
    double finerot_deg;
    double rot_deg;
    double page_width_pts;
    double page_height_pts;
    double x0_pts;
    double y0_pts;
    double crop_width_pts;
    double crop_height_pts;
    } WPDFSRCBOX;

typedef struct
    {
    int dstpage;     /* Dest page */
    /*
    ** x0,y0,w,h,srcrot_deg,src_width_pts,src_height_pts are used INTERNALLY by wmupdf.c
    */
    double x0,y0;    /* x0,y0, in points, of lower left point on transformed source page */
                     /* (Used internally by wmupdf.c) */
    double w,h;      /* width and height of transformed source rectangle in points */
                     /* (Used internally by wmupdf.c) */
    double x1,y1;    /* (x,y) position of lower left source point on destination page, in points */
    double scale;    /* Scale rectangle by this factor on destination page */
    double srcrot_deg;  /* Rotation of source selection rectangle about x0,y0 */
                        /* (Used internally by wmupdf.c) */
    double dstrot_deg;  /* Rotation of destination rectangle about x1,y1 */
    double userx,usery; /* For user use--not used by wmupdf.c */
    double src_width_pts,src_height_pts;  /* Width/height of transformed source page in points */
                                          /* (Used internally by wmupdf.c) */
    double dst_width_pts,dst_height_pts;  /* Width/height of device page in points */
    WPDFSRCBOX srcbox;
    } WPDFBOX;

typedef struct
    {
    WPDFBOX *box;
    int n;
    int na;
    } WPDFBOXES;

typedef struct
    {
    char producer[128];  /* Producer */
    double width_pts;    /* Destination page width in pts. */
    double height_pts;   /* Destination page height in pts. */
    int srcpage;                 /* Ignored by wmupdf_remake_pdf */
    double srcpage_rot_deg;      /* Ignored by wmupdf_remake_pdf */
    double srcpage_fine_rot_deg; /* Ignored by wmupdf_remake_pdf */
    WPDFBOXES boxes;
    } WPDFPAGEINFO;
/*
** Positions are from upper left corner of page
*/
typedef struct
    {
    int ucs;      /* character */
    double xp,yp; /* Left baseline position of char */
    double x1,y1; /* bounding box upper left corner in points */
    double x2,y2; /* bounding box lower right corner in points */
    } WTEXTCHAR;

typedef struct
    {
    WTEXTCHAR *wtextchar;
    double width,height; /* width/height of page in points */
                         /* (used to rotate chars boxes) */
    int n,na;
    int sorted;
    } WTEXTCHARS;

void wpdfboxes_init(WPDFBOXES *boxes);
void wpdfboxes_free(WPDFBOXES *boxes);
void wpdfboxes_insert_box(WPDFBOXES *boxes,WPDFBOX *box,int index);
void wpdfboxes_add_box(WPDFBOXES *boxes,WPDFBOX *box);
void wpdfboxes_delete(WPDFBOXES *boxes,int n);
void wpdfbox_unrotate(WPDFBOX *box,double deg);
void wpdfbox_determine_original_source_position(WPDFBOX *box);
void wpdfpageinfo_sort(WPDFPAGEINFO *pageinfo);
void wpdfpageinfo_scale_source_boxes(WPDFPAGEINFO *pageinfo,double doc_scale_factor);
void wtextchars_init(WTEXTCHARS *wtc);
void wtextchars_free(WTEXTCHARS *wtc);
void wtextchars_clear(WTEXTCHARS *wtc);
void wtextchars_add_wtextchar(WTEXTCHARS *wtc,WTEXTCHAR *textchar);
void wtextchars_remove_wtextchar(WTEXTCHARS *wtc,int index);
void wtextchars_rotate_clockwise(WTEXTCHARS *wtc,int rot_deg);
void wtextchars_text_inside(WTEXTCHARS *src,char **text,double x1,double y1,double x2,double y2);
void wtextchars_sort_vertically_by_position(WTEXTCHARS *wtc,int type);
void wtextchars_sort_horizontally_by_position(WTEXTCHARS *wtc);
void wtextchar_array_sort_horizontally_by_position(WTEXTCHAR *x,int n);


/* bmpmupdf.c */
/* Mupdf / bitmap functions */
#ifdef HAVE_MUPDF_LIB
int bmpmupdf_pdffile_to_bmp(WILLUSBITMAP *bmp,char *filename,int pageno,double dpi,int bpp);
int bmpmupdf_pdffile_width_and_height(char *filename,int pageno,double *width_in,double *height_in);
#endif /* HAVE_MUPDF_LIB */

/* wmupdf.c */
/* Mupdf support functions */
#ifdef HAVE_MUPDF_LIB
int  wmupdf_numpages(char *filename);
int  wmupdf_info_field(char *infile,char *label,char *buf,int maxlen);
int  wmupdf_remake_pdf(char *infile,char *outfile,WPDFPAGEINFO *pageinfo,int use_forms,
                       WPDFOUTLINE *wpdfoutline,FILE *out);
/* Character position map */
int  wtextchars_fill_from_page(WTEXTCHARS *wtc,char *filename,int pageno,char *password);
int  wtextchars_fill_from_page_ex(WTEXTCHARS *wtc,char *filename,int pageno,char *password,
                                 int boundingbox);
WPDFOUTLINE *wpdfoutline_read_from_pdf_file(char *filename);
#endif /* HAVE_MUPDF_LIB */

#ifdef HAVE_DJVU_LIB
/* bmpdjvu.c */
/* djvu supported functions */
int bmpdjvu_djvufile_to_bmp(WILLUSBITMAP *bmp,char *infile,int pageno,
                            int dpi,int bpp,FILE *out);
int bmpdjvu_numpages(char *infile);
#endif /* HAVE_DJVU_LIB */


/* gslpolyfit.c */
void gslpolyfit(double *x,double *y,int n,int degree,double *c);
#define wpolyfitd gslpolyfit
/* strbuf.c */
typedef struct
    {
    char *s;
    int na;
    } STRBUF;
char *strbuf_lineno(STRBUF *buf,int line_index);
void strbuf_init(STRBUF *sbuf);
void strbuf_cat_ex(STRBUF *sbuf,char *s);
void strbuf_cat(STRBUF *sbuf,char *s);
void strbuf_cat_with_quotes(STRBUF *sbuf,char *s);
void strbuf_cat_no_spaces(STRBUF *sbuf,char *s);
void strbuf_cpy(STRBUF *sbuf,char *s);
void strbuf_clear(STRBUF *sbuf);
void strbuf_ensure(STRBUF *sbuf,int n);
void strbuf_free(STRBUF *sbuf);
void strbuf_sprintf(STRBUF *sbuf,char *fmt,...);
void strbuf_dsprintf(STRBUF *sbuf,STRBUF *sbuf2,char *fmt,...);
void strbuf_sprintf_no_space(STRBUF *sbuf,char *fmt,...);
void strbuf_dsprintf_no_space(STRBUF *sbuf,STRBUF *sbuf2,char *fmt,...);

/* wgui.c */
#define WILLUSGUICONTROL_TYPE_BUTTON       1
#define WILLUSGUICONTROL_TYPE_LISTBOX      2
#define WILLUSGUICONTROL_TYPE_EDITBOX      3
#define WILLUSGUICONTROL_TYPE_DROPDOWNLIST 4
/* Text field w/up and down arrow for adjustment */
#define WILLUSGUICONTROL_TYPE_UPDOWN       5
#define WILLUSGUICONTROL_TYPE_UPDOWN2      6
#define WILLUSGUICONTROL_TYPE_CHECKBOX     7
#define WILLUSGUICONTROL_TYPE_SCROLLABLEBITMAP 8

#define WILLUSGUIACTION_DRAW_CONTROL     1
#define WILLUSGUIACTION_INIT             2
#define WILLUSGUIACTION_TIMER_TICK       3
#define WILLUSGUIACTION_ENTER_PRESS      4
#define WILLUSGUIACTION_ESC_PRESS        5
#define WILLUSGUIACTION_CONTROL_PRESS    6
#define WILLUSGUIACTION_MENU_SELECTION   7
#define WILLUSGUIACTION_REPAINT          8
#define WILLUSGUIACTION_CLOSE            9
#define WILLUSGUIACTION_GETMINSIZE       10
#define WILLUSGUIACTION_WINDOWSIZECHANGE 11
#define WILLUSGUIACTION_STARTING_RESIZE  12
#define WILLUSGUIACTION_ENDING_RESIZE    13
#define WILLUSGUIACTION_SETFOCUS         14
#define WILLUSGUIACTION_LOSTFOCUS        15
#define WILLUSGUIACTION_SELECTION_CHANGE 16
#define WILLUSGUIACTION_UPDOWN_UP        17
#define WILLUSGUIACTION_UPDOWN_DOWN      18
#define WILLUSGUIACTION_UPDOWN_EDIT      19
#define WILLUSGUIACTION_UPDOWN2_DLEFT    20
#define WILLUSGUIACTION_UPDOWN2_LEFT     21
#define WILLUSGUIACTION_UPDOWN2_RIGHT    22
#define WILLUSGUIACTION_UPDOWN2_DRIGHT   23
#define WILLUSGUIACTION_DESTROY          24
#define WILLUSGUIACTION_DROPFILES        25
#define WILLUSGUIACTION_CREATE           26
#define WILLUSGUIACTION_CONTEXTMENU      27

#define WILLUSGUICONTROL_ATTRIB_INACTIVE    0x0001
#define WILLUSGUICONTROL_ATTRIB_READONLY    0x0002
#define WILLUSGUICONTROL_ATTRIB_MULTILINE   0x0004
#define WILLUSGUICONTROL_ATTRIB_SCROLLBARS  0x0008
#define WILLUSGUICONTROL_ATTRIB_MULTISELECT 0x0010
#define WILLUSGUICONTROL_ATTRIB_CHECKED     0x0020

typedef struct
    {
    int left,right,top,bottom;
    } WILLUSGUIRECT;

typedef struct
    {
    void *handle;
    int size;     /* pixels */
    } WILLUSGUIFONT;

typedef struct _willusguicontrol
    {
    void *handle;
    int type;
    WILLUSGUIRECT rect;
    WILLUSGUIFONT font;
    char name[32];       /* (Unique) name of control--drawn on button if button */
    int color;
    int index;
    int attrib;          /* See WILLUSGUIACTION_ATTRIB_... defines */
    /*
    ** flags & 7 = config preset button index (starting at 1).
    */
    int flags;
    void *subhandle[4]; /* Handles to related controls */
    struct _willusguicontrol *parent;
    char label[32];      /* Drawn with or next to control */
    int labeljust;       /* label justification */
    int labelx,labely;   /* x,y position of label */
    /* Scrollable bitmap controls */
    int sbitmap_size;    /* Scrollable bitmap toggle */
    WILLUSBITMAP *obmp;     /* Original size bitmap */
    WILLUSBITMAP bmp;       /* Sized bitmap for display in Window */
    } WILLUSGUICONTROL;

typedef struct
    {
    WILLUSGUICONTROL *control; /* Who the message is from */
    int guiaction;       /* message action */
    int param[2];        /* passed params */
    void *ptr[2];        /* pointers */
    } WILLUSGUIMESSAGE;

typedef WILLUSGUICONTROL WILLUSGUIWINDOW;

void willusgui_init(void);
void willusgui_close(void);
void willusgui_set_cursor(int type);
void willusgui_open_file(char *filename);
WILLUSGUIWINDOW *willusgui_window_find(void *oshandle);
void willusgui_window_text_render(WILLUSGUIWINDOW *win,WILLUSGUIFONT *font,char *text,int x0,int y0,
                                   int fgcolor,int bgcolor,int justification);
void willusgui_window_text_extents(WILLUSGUIWINDOW *win,WILLUSGUIFONT *font,char *string,WILLUSGUIRECT *rect);
void willusgui_window_draw_line(WILLUSGUIWINDOW *win,int x0,int y0,int x1,int y1,
                                                 int pixwidth,int rgbcolor);
void willusgui_window_draw_rect_filled(WILLUSGUIWINDOW *win,WILLUSGUIRECT *rect,int rgb);
void willusgui_window_draw_path_filled(WILLUSGUIWINDOW *win,int *x,int *y,int n,int rgb);
int  willusgui_control_nlines(WILLUSGUICONTROL *control);
void willusgui_window_draw_rect_outline(WILLUSGUIWINDOW *win,WILLUSGUIRECT *rect,int rgb);
void willusgui_set_instance(void *instanceptr);
void *willusgui_instance(void);
void willusgui_window_register(WILLUSGUIWINDOW *window);
void willusgui_window_deregister(WILLUSGUIWINDOW *window);
void willusgui_control_create(WILLUSGUICONTROL *control);
void willusgui_control_enable(WILLUSGUICONTROL *control,int enabled);
void willusgui_control_set_text(WILLUSGUICONTROL *control,char *text);
void willusgui_control_get_text(WILLUSGUICONTROL *control,char *text,int maxlen);
int  willusgui_control_get_textlen(WILLUSGUICONTROL *control);
void willusgui_control_scroll_to_bottom(WILLUSGUICONTROL *control);
int  willusgui_window_get_rect(WILLUSGUIWINDOW *win,WILLUSGUIRECT *guirect);
int  willusgui_window_get_useable_rect(WILLUSGUIWINDOW *win,WILLUSGUIRECT *guirect);
void willusgui_window_accept_draggable_files(WILLUSGUIWINDOW *win);
void willusgui_window_timer_init(WILLUSGUIWINDOW *win,int ms);
void willusgui_window_menus_init(WILLUSGUIWINDOW *win,char *menus[]);
int  willusgui_message_box(WILLUSGUIWINDOW *parent,char *title,char *message,char *button1,
                            char *button2,char *button3,char *inbuf,int maxlen,
                            int fontsize_pixels,int maxwidth_pixels,int rgbcolor,
                            int *bcolors,WILLUSGUIRECT *rect,int modal);
void willusgui_send_quit_message(void);
void willusgui_control_init(WILLUSGUICONTROL *control);
int  willusgui_control_close(WILLUSGUICONTROL *control);
int  willusgui_control_close_ex(WILLUSGUICONTROL *control,int caller);
void willusgui_control_draw_label(WILLUSGUICONTROL *control);
void willusgui_control_redraw(WILLUSGUICONTROL *control,int children_too);
void willusgui_font_release(WILLUSGUIFONT *font);
void willusgui_font_get(WILLUSGUIFONT *font);
/*
int  willusgui_font_is_calibri(void);
*/
void willusgui_font_init(WILLUSGUIFONT *font);
void willusgui_start_browser(char *link);
int  willusgui_control_get_checked(WILLUSGUICONTROL *control);
void willusgui_control_set_checked(WILLUSGUICONTROL *control,int checked);
int  willusgui_control_dropdownlist_get_selected_item(WILLUSGUICONTROL *control,char *buf);
int  willusgui_control_listbox_get_selected_items_count(WILLUSGUICONTROL *control,int *selected_indices,
                                                         int maxsel);
int  willusgui_control_listbox_select_item(WILLUSGUICONTROL *control,char *string);
void willusgui_control_listbox_clear(WILLUSGUICONTROL *control);
void willusgui_control_listbox_add_item(WILLUSGUICONTROL *control,char *text);
int  willusgui_control_listbox_get_item_text(WILLUSGUICONTROL *control,int index,char *buf);
char **willusgui_get_dropped_files(void *dropptr);
void willusgui_release_dropped_files(char **ptr);
void willusgui_window_set_focus(WILLUSGUIWINDOW *win);
void willusgui_control_text_select_all(WILLUSGUICONTROL *control);
void *willusgui_control_handle_with_focus(void);
void willusgui_window_set_redraw(WILLUSGUIWINDOW *window,int status);
int  willusgui_file_select_dialog(char *buf,int maxlen,char *allowedfiles,
                                   char *prompt,char *defext,int for_writing);
int willusgui_file_select_dialogw(short *buf,int maxlen,char *allowedfiles,
                               char *prompt,char *defext,int for_writing);
void willusgui_background_bitmap_blit(WILLUSGUIWINDOW *win,WILLUSBITMAP *bmp);
void *willusgui_semaphore_create(char *name);
void willusgui_semaphore_release(void *semaphore);
void willusgui_semaphore_close(void *semaphore);
int  willusgui_semaphore_status(void *semaphore);
void *willusgui_thread_create(void *funcptr,void *data);
void willusgui_thread_terminate(void *pid,int exitcode);
void willusgui_thread_exit(int exitcode);
void willusgui_sbitmap_resample_original(WILLUSGUICONTROL *control);
void willusgui_sbitmap_change_size(WILLUSGUICONTROL *control,int delsize);
void willusgui_sbitmap_proc(void *handle,int message,int wparam,void *lparam);
void willusgui_set_ime_notify(int status);


#ifdef PI
#undef PI
#endif
/*
** Constants from the front of the CRC standard math tables
** (Accuracy = 50 digits)
*/
/* The 50 digits cause problems with MPW's Mr. C on the Macintosh, */
/* so I've truncated to 20 digits.                                 */
/*
#define PI      3.14159265358979323846264338327950288419716939937511
#define SQRT2   1.41421356237309504880168872420969807856967187537695
#define SQRT3   1.73205080756887729352744634150587236694280525381039
#define LOG10E  0.43429448190325182765112891891660508229439700580367
*/
#define PI      3.1415926535897932384
#define SQRT2   1.4142135623730950488
#define SQRT3   1.7320508075688772935
#define LOG10E  0.4342944819032518276
#define DBPERNEP    (20.*LOG10E)

/*
** Speed of light in meters per second (exact by definition)
*/
#define CMPS    299792458.
#define CIPS    (CMPS*100./2.54)

/*
** Permeability of free space (exact definition)
*/
#define MU0     (4.*PI*1e-7)

/*
** Permittivity of free space (exact definition)
*/
#define EPS0    (1/(MU0*CMPS*CMPS))

/*
** Electron charge in Coulombs
**     standard uncertainty          = 0.000 000 14e-19 Coulombs
**     relative standard uncertainty = 8.5e-8
*/
/* #define QE      1.60217646263e-19    -- 1998 */
#define QE      1.6021765314e-19

/*
** Electron mass in kg
**     standard uncertainty          = 0.000 0016e-31 kg
**     relative standard uncertainty = 1.7e-7
*/
/* #define ME      9.1093818872e-31     -- 1998 */
#define ME      9.109382616e-31

/*
** Note:  ETA is commonly defined as QE/ME in beam physics
**        (charge-to-mass ratio)
*/

/*
** Avogadro's Number [mol^-1]
**     standard uncertainty          = 0.000 0010e+23 mol^-1
**     relative standard uncertainty = 1.7e-7
*/
/* #define NAVOGADRO   6.0221419947e23  -- 1998 */
#define NAVOGADRO   6.022141510e23

/*
** Boltzmann's constant [Joule/Kelvin]
**     standard uncertainty          = 0.000 0024e-23 J/K
**     relative standard uncertainty = 1.8e-6
*/
/* #define BOLTZMANN   1.380650324e-23  -- 1998 */
#define BOLTZMANN   1.380650524e-23

/*
** Molar Gas Constant [Joule/(mol*Kelvin)]
**     standard uncertainty          = 0.000 015e-23 J/(mol*K)
**     relative standard uncertainty = 1.7e-6
*/
/* #define REYNOLDS    8.31447215     -- 1998 */
#define REYNOLDS    8.31447215

/*
** Newtonian constant of gravitation, G [meters^3 / (kg*sec^2)]
**     standard uncertainty          = 0.0010e-11 m^3/(kg*s^2)
**     relative standard uncertainty = 1.5e-4
*/
#define GRAVCONST   6.674210e-11

/*
** Planck's constant, Joule-seconds
** Standard uncertainty = 0.0000011e-34 J-s
** Rel standard uncertainty = 1.7e-7
*/
#define PLANCK      6.626069311e-34


/*
** First nonzero root of J1'(x), accuracy = 10 digits
*/
#define NU11    1.8411836157
#endif /* __INCLUDED_WILLUS_H__ */
