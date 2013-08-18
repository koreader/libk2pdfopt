/*
** k2pdfopt.h   Main include file for k2pdfopt source modules.
**
** Copyright (C) 2013  http://willus.com
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

#ifndef __K2PDFOPT_H__
#define __K2PDFOPT_H__
/*
** WILLUSDEBUGX flags:
** 1 = Generic
** 2 = breakinfo row analysis
** 4 = word wrapping
** 8 = word wrapping II
** 16 = hyphens
** 32 = OCR
** 64 = crop boxes
** 128 = column divider
** 256 = breakinfo_find_doubles
** 512 = gaps between big regions
**
*/

/*
#define WILLUSDEBUGX 0xfff
#define WILLUSDEBUG
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <math.h>
#include <willus.h>

/* Uncomment below if compiling for Kindle PDF Viewer */
#define K2PDFOPT_KINDLEPDFVIEWER

/*
** The HAVE_..._LIB defines should now be carried over from willus.h,
** but you can still undefine them to disable the use by k2pdfopt lib.
**
** Undefine HAVE_DJVU_LIB if no DjVuLibre library
**
** Undefine HAVE_MUPDF_LIB if no MuPDF (must have Ghostscript in this case)
*/
/*
#ifdef HAVE_GOCR_LIB
#undef HAVE_GOCR_LIB
#endif
*/
#ifdef HAVE_TESSERACT_LIB
#undef HAVE_TESSERACT_LIB
#endif
/*
#ifdef HAVE_DJVU_LIB
#undef HAVE_DJVU_LIB
#endif
#ifdef HAVE_MUPDF_LIB
#undef HAVE_MUPDF_LIB
#endif
*/

#if (defined(HAVE_GOCR_LIB) || defined(HAVE_TESSERACT_LIB))
#if (!defined(HAVE_OCR_LIB))
#define HAVE_OCR_LIB
#endif
#else
#if (defined(HAVE_OCR_LIB))
#undef HAVE_OCR_LIB
#endif
#endif

#define GRAYLEVEL(r,g,b) ((int)(((r)*0.3+(g)*0.59+(b)*0.11)*1.002))
#if (defined(WIN32) || defined(WIN64))
#define TTEXT_BOLD    ANSI_WHITE
#define TTEXT_NORMAL  ANSI_NORMAL
#define TTEXT_BOLD2   ANSI_YELLOW
#define TTEXT_INPUT   ANSI_GREEN
#define TTEXT_WARN    ANSI_RED
#define TTEXT_HEADER  ANSI_CYAN
#define TTEXT_MAGENTA ANSI_MAGENTA
#else
#define TTEXT_BOLD    "\x1b[0m\x1b[34m"
#define TTEXT_NORMAL  "\x1b[0m"
#define TTEXT_BOLD2   "\x1b[0m\x1b[33m"
#define TTEXT_INPUT   "\x1b[0m\x1b[32m"
#define TTEXT_WARN    "\x1b[0m\x1b[31m"
#define TTEXT_HEADER  "\x1b[0m\x1b[36m"
#define TTEXT_MAGENTA "\x1b[0m\x1b[35m"
#endif

#define SRC_TYPE_PDF     1
#define SRC_TYPE_DJVU    2
#define SRC_TYPE_PS      3
#define SRC_TYPE_BITMAPFOLDER 4
#define SRC_TYPE_OTHER   5

#define UNITS_PIXELS      0
#define UNITS_INCHES      1
#define UNITS_CM          2
#define UNITS_SOURCE      3
#define UNITS_TRIMMED     4

#define DEFAULT_WIDTH 560
#define DEFAULT_HEIGHT 735
#define MIN_REGION_WIDTH_INCHES 1.0
#define SRCROT_AUTO   -999.
#define SRCROT_AUTOEP -998.

#define OR_DETECT(x)     (fabs((x)-SRCROT_AUTO)<.5)
#define OREP_DETECT(x)   (fabs(x->src_rot-SRCROT_AUTOEP)<.5)
#define int_swap(x,y) {int t; t=(x); (x)=(y); (y)=t;}
#define double_swap(x,y) {double t; t=(x); (x)=(y); (y)=t;}

/*
** DATA STRUCTURES
*/

/*
** K2PDFOPT_SETTINGS stores user settings that affect the document processing.
*/
typedef struct
    {
    /* debugging */
    int verbose;
    int debug;

    double cdthresh;
    /*
    ** Blank Area Threshold Widths--average black pixel width, in inches, that
    ** prevents a region from being determined as "blank" or clear.
    */
    int src_rot;
    double gtc_in; // detecting gap between columns
    double gtr_in; // detecting gap between rows
    double gtw_in; // detecting gap between words
    int show_usage;
    int src_left_to_right;
    int src_whitethresh;

    /* OCR */
#ifdef HAVE_OCR_LIB
    int dst_ocr;
#ifdef HAVE_TESSERACT_LIB
    char dst_ocr_lang[16];
#endif
    int dst_ocr_visibility_flags;
    int ocr_max_columns;
    double ocr_max_height_inches;
    OCRWORDS dst_ocrwords;
#ifdef HAVE_TESSERACT_LIB
    int ocrtess_status;
#endif
#endif

    int dst_dpi;
    int dst_dither;
    int dst_break_pages;
    int render_dpi;
    int fit_columns;
    double user_src_dpi;
    double document_scale_factor;
    int src_dpi;
    int user_usegs;
    int usegs;
    int query_user;
    int query_user_explicit;
    int jpeg_quality;
    int dst_width; /* Full device width in pixels */
    int dst_height; /* pixels */
    double dst_userwidth; /* pixels */
    double dst_userheight; /* pixels */
    double dst_display_resolution;
    int dst_userwidth_units;
    int dst_userheight_units;
    int dst_justify; // 0 = left, 1 = center
    int dst_figure_justify; // -1 = same as dst_justify.  0=left 1=center 2=right
    double dst_min_figure_height_in;
    int dst_fulljustify; // 0 = no, 1 = yes
    int dst_sharpen;
    int dst_color;
    int dst_bpc;
    int dst_landscape;
    char dst_opname_format[128];
    int src_autostraighten;
    double dst_mar;
    double dst_martop;
    double dst_marbot;
    double dst_marleft;
    double dst_marright;
    int pad_left;
    int pad_right;
    int pad_bottom;
    int pad_top;
    int mark_corners;
    double min_column_gap_inches;
    double max_column_gap_inches; // max gap between columns
    double min_column_height_inches;
    double mar_top;
    double mar_bot;
    double mar_left;
    double mar_right;
    double max_region_width_inches; /* Max viewable width (device width minus margins) */
    int max_columns;
    double column_gap_range;
    double column_offset_max;
    double column_row_gap_height_in;
    double row_split_fom;  /* Used by breakinfo_find_doubles() */
    int text_wrap;
    double word_spacing;
    double display_width_inches; /* Device width = dst_width / dst_dpi */
    char pagelist[1024];
    int column_fitted;
    double lm_org,bm_org,tm_org,rm_org,dpi_org;
    double contrast_max;
    double dst_gamma;
    int dst_negative;
    int exit_on_complete;
    int show_marked_source;
    int use_crop_boxes;
    int preserve_indentation;
    double defect_size_pts;
    double max_vertical_gap_inches;
    double vertical_multiplier;
    double vertical_line_spacing;
    double vertical_break_threshold;
    int src_trim;
    int erase_vertical_lines;
    int hyphen_detect;
    double overwrite_minsize_mb;
    int dst_fit_to_page;
    int src_grid_rows;
    int src_grid_cols;
    int src_grid_overlap_percentage;
    int src_grid_order; /* 0=down then across, 1=across then down */
    /*
    ** Undocumented cmd-line args
    */
    double no_wrap_ar_limit; /* -arlim */
    double no_wrap_height_limit_inches; /* -whmax */
    double little_piece_threshold_inches; /* -rwmin */
    /*
    ** Keeping track of vertical gaps
    */
    double last_scale_factor_internal;
    int line_spacing_internal; /* If > 0, try to maintain regular line spacing.  If < 0,   */
                             /* indicates desired vert. gap before next region is added. */
    int last_rowbase_internal; /* Pixels between last text row baseline and current end */
                             /* of destination bitmap. */
    int gap_override_internal; /* If > 0, apply this gap in wrapbmp_flush() and then reset. */
    } K2PDFOPT_SETTINGS;


typedef struct
    {
    char **file;
    int na;
    int n;
    } K2PDFOPT_FILES;


typedef struct
    {
    K2PDFOPT_SETTINGS k2settings;
    K2PDFOPT_FILES k2files;
    } K2PDFOPT_CONVERSION;

/*
** hyphen-detection structure
*/
typedef struct
    {
    int ch;    /* Hyphen starting point -- < 0 for no hyphen */
    int c2;    /* End of end region if hyphen is erased */
    int r1;    /* Top of hyphen */
    int r2;    /* Bottom of hyphen */
    } HYPHENINFO;

/*
** BMPREGION is a rectangular region within a bitmap.  This is the main
** data structure used by k2pdfopt to break up the source page.
*/
typedef struct
    {
    int r1,r2;     /* row position from top of bmp, inclusive */
    int c1,c2;     /* column positions, inclusive */
    int rowbase;   /* Baseline of text row */
    int capheight; /* capital letter height */
    int h5050;
    int lcheight;  /* lower-case letter height */
    int bgcolor;   /* 0 - 255 */
    int dpi;       /* dpi of bitmap */
    HYPHENINFO hyphen;
    WILLUSBITMAP *bmp;
    WILLUSBITMAP *bmp8;
    WILLUSBITMAP *marked;
    } BMPREGION;

/*
** TEXTROW describes a row of text
*/
typedef struct
    {
    int c1,c2;   /* Left and right columns */
    int r1,r2;   /* Top and bottom of region in pixels */
    int rowbase; /* Baseline of row */
    int gap;     /* Gap to next region in pixels */
    int rowheight; /* text + gap (delta between rowbases) */
    int capheight;
    int h5050;
    int lcheight;
    double rat;  /* If found with find_doubles, this is > 0 (the figure of merit) */
    HYPHENINFO hyphen;
    } TEXTROW;

/*
** BREAKINFO contains information on contiguous regions within a BMPREGION.
** These may be rows of text or words within a row of text.
*/
typedef struct
    {
    TEXTROW *textrow;
    int rhmean_pixels;  /* Mean row height (text) */
    int centered;       /* Is this set of rows centered? */
    int n,na;
    } BREAKINFO;

/*
** WRAPBMP contains a cached bitmap where individual word bitmaps are placed
** during text re-flow.  This bitmap is eventually sent to the master output
** bitmap.
*/
typedef struct
    {
    WILLUSBITMAP bmp;
    int base;
    int line_spacing;
    int gap;
    int bgcolor;
    int just;
    int rhmax;
    int thmax;
    int maxgap;
    int height_extended;
    int just_flushed_internal;
    int beginning_gap_internal;
    int last_h5050_internal;
    HYPHENINFO hyphen;
    } WRAPBMP;

/*
** MASTERINFO handles the output to the device.
*/
typedef struct
    {
    PDFFILE outfile;  /* PDF output file data structure */
    WILLUSBITMAP bmp; /* Master output bitmap collects pages that will go to */
                      /* the output device */
    WRAPBMP wrapbmp;  /* See WRAPBMP structure */
    WPDFPAGEINFO pageinfo;  /* Holds crop boxes for native PDF output */
    int rows;             /* Rows stored within the bmp structure */
    int published_pages;  /* Count of published pages */
    int bgcolor;
    int fit_to_page;
    int wordcount;
    char debugfolder[256];
    /*
    ** Values used to get appropriate spacing on next large block added
    */
    int fontsize;    /* Font size of last row added (pixels).  < 0 = no last font */
    int linespacing; /* Line spacing of last row added (pixels) */
    int gap;         /* Gap after baseline of last row added (pixels) */
    } MASTERINFO;

/*
** DEVPROFILE contains data that describes an e-book
** (screen width, display resolution, etc.)
*/
typedef struct
    {
    char *name;
    char *alias;
    int width;
    int height;
    int dpi;
    int color;
    int mark_corners;
    int padding[4];
    } DEVPROFILE;


/*
**
** Function prototypes
**
*/

/* k2file.c */
void k2pdfopt_proc_wildarg(K2PDFOPT_SETTINGS *k2settings,char *arg,int process,int *filecount);

/* k2sys.c */
void k2sys_init(void);
void k2sys_close(K2PDFOPT_SETTINGS *k2settings);
void k2sys_header(char *s);
void k2sys_exit(K2PDFOPT_SETTINGS *k2settings,int val);
void k2sys_enter_to_exit(K2PDFOPT_SETTINGS *k2settings);
int  k2printf(char *fmt,...);

/* k2usage.c */
void k2usage_show_all(FILE *out);
void k2usage_to_string(char *s);
int  k2usage_len(void);
int  k2pdfopt_usage(void);

/* k2parsecmd.c */
int parse_cmd_args(K2PDFOPT_CONVERSION *k2conv,STRBUF *env,STRBUF *cmdline,
                   STRBUF *userinput,int setvals);

/* k2menu.c */
int k2pdfopt_menu(K2PDFOPT_CONVERSION *k2conv,STRBUF *env,STRBUF *cmdline,STRBUF *usermenu);

/* userinput.c */
int  userinput_float(char *message,double defval,double *dstval,int nmax,
                      double min,double max,char *extra_message);
int  userinput_integer(char *message,int defval,int *dstval,int min,int max);
int  userinput_string(char *message,char *selection[],char *def);
int  userinput_any_string(char *message,char *dstval,int maxlen,char *defname);
int  get_ttyrows(void);

/* bmpregion.c */
int  bmpregion_row_black_count(BMPREGION *region,int r0);
int  bmpregion_col_black_count(BMPREGION *region,int c0);
#if (defined(WILLUSDEBUGX) || defined(WILLUSDEBUG))
void bmpregion_write(BMPREGION *region,char *filename);
#endif
void bmpregion_row_histogram(BMPREGION *region);
int  bmpregion_is_clear(BMPREGION *region,int *row_black_count,int *col_black_count,
                        int *pixel_count_array,int rpc,double gt_in);
void bmpregion_trim_to_crop_margins(BMPREGION *region,K2PDFOPT_SETTINGS *k2settings);
int  bmpregion_column_height_and_gap_test(BMPREGION *column,BMPREGION *region,
                                        K2PDFOPT_SETTINGS *k2settings,
                                        int r1,int r2,int cmid,
                                        int *colcount,int *rowcount);
void bmpregion_trim_margins(BMPREGION *region,K2PDFOPT_SETTINGS *k2settings,
                            int *colcount0,int *rowcount0,int flags);
void bmpregion_hyphen_detect(BMPREGION *region,int hyphen_detect,int left_to_right);
int  bmpregion_is_centered(BMPREGION *region,K2PDFOPT_SETTINGS *k2settings,
                           BREAKINFO *breakinfo,int i1,int i2,int *textheight);
void bmpregion_get_white_margins(BMPREGION *region);


/* pageregions.c */
typedef struct
    {
    BMPREGION bmpregion;
    int fullspan;
    int level;
    } PAGEREGION;
typedef struct
    {
    PAGEREGION *pageregion;
    int n,na;
    } PAGEREGIONS;
void pageregions_init(PAGEREGIONS *regions);
void pageregions_free(PAGEREGIONS *regions);
void pageregions_delete_one(PAGEREGIONS *regions,int index);
void pageregions_insert(PAGEREGIONS *dst,int index,PAGEREGIONS *src);
void pageregions_add_pageregion(PAGEREGIONS *regions,BMPREGION *bmpregion,int level,int fullspane);


/* k2proc.c */
void k2proc_init_one_document(void);
void bmpregion_source_page_add(BMPREGION *region,K2PDFOPT_SETTINGS *k2settings,
                               MASTERINFO *masterinfo,int level,int pages_done,int colgap0_pixels);
void pageregions_find(PAGEREGIONS *pageregions_sorted,BMPREGION *srcregion,
                      int *rowcount,int *colcount,K2PDFOPT_SETTINGS *k2settings,
                      int  maxlevels);
void bmpregion_add(BMPREGION *region,K2PDFOPT_SETTINGS *k2settings,BREAKINFO *breakinfo,
                   MASTERINFO *masterinfo,int allow_text_wrapping,
                   int trim_flags,int allow_vertical_breaks,
                   double force_scale,int justify_flags,int caller_id,
                   int *colcount,int *rowcount,int mark_flags,int rowbase_delta);


/* k2settings.c */
void k2pdfopt_settings_init(K2PDFOPT_SETTINGS *k2settings);
void k2pdfopt_settings_copy(K2PDFOPT_SETTINGS *dst,K2PDFOPT_SETTINGS *src);
int  k2pdfopt_settings_set_to_device(K2PDFOPT_SETTINGS *k2settings,DEVPROFILE *dp);
void k2pdfopt_settings_sanity_check(K2PDFOPT_SETTINGS *k2settings);
void k2pdfopt_settings_new_source_document_init(K2PDFOPT_SETTINGS *k2settings);
void k2pdfopt_settings_restore_output_dpi(K2PDFOPT_SETTINGS *k2settings);
void k2pdfopt_settings_fit_column_to_screen(K2PDFOPT_SETTINGS *k2settings,
                                            double column_width_inches);
void k2pdfopt_settings_set_region_widths(K2PDFOPT_SETTINGS *k2settings);
void k2pdfopt_settings_set_margins_and_devsize(K2PDFOPT_SETTINGS *k2settings,
                         BMPREGION *region,MASTERINFO *masterinfo,int trimmed);


/* breakinfo.c */
void bmpregion_find_vertical_breaks(BMPREGION *region,BREAKINFO *breakinfo,
                                    K2PDFOPT_SETTINGS *k2settings,
                                    int *colcount,int *rowcount,double apsize_in);
void bmpregion_one_row_find_breaks(BMPREGION *region,BREAKINFO *breakinfo,
                                   K2PDFOPT_SETTINGS *k2settings,
                                   int *colcount,int *rowcount,int add_to_dbase);
void breakinfo_remove_small_rows(BREAKINFO *breakinfo,K2PDFOPT_SETTINGS *k2settings,
                                 double fracrh,double fracgap,
                                 BMPREGION *region,int *colcount,int *rowcount);
void breakinfo_alloc(int index,BREAKINFO *breakinfo,int nrows);
void breakinfo_free(int index,BREAKINFO *breakinfo);
void breakinfo_sort_by_gap(BREAKINFO *breakinfo);
void breakinfo_sort_by_row_position(BREAKINFO *breakinfo);
#if (WILLUSDEBUGX & 6)
void breakinfo_echo(BREAKINFO *bi);
#endif
void word_gaps_add(BREAKINFO *breakinfo,int lcheight,double *median_gap,double word_spacing);

/* k2mark.c */
void publish_marked_page(PDFFILE *mpdf,WILLUSBITMAP *src,int src_dpi);
void mark_source_page(K2PDFOPT_SETTINGS *k2settings,BMPREGION *region,int caller_id,int mark_flags);

/* wrapbmp.c */
void wrapbmp_init(WRAPBMP *wrapbmp,int color);
int  wrapbmp_ends_in_hyphen(WRAPBMP *wrapbmp);
void wrapbmp_set_color(WRAPBMP *wrapbmp,int is_color);
void wrapbmp_free(WRAPBMP *wrapbmp);
void wrapbmp_set_maxgap(WRAPBMP *wrapbmp,int value);
int  wrapbmp_width(WRAPBMP *wrapbmp);
int  wrapbmp_remaining(WRAPBMP *wrapbmp,K2PDFOPT_SETTINGS *k2settings);
void wrapbmp_add(WRAPBMP *wrapbmp,BMPREGION *region,K2PDFOPT_SETTINGS *k2settings,
                 int gap,int line_spacing,int rbase,int gio,int justification_flags);
void wrapbmp_flush(MASTERINFO *masterinfo,K2PDFOPT_SETTINGS *k2settings,
                   int allow_full_justify,int use_bgi);

/* k2master.c */
void masterinfo_init(MASTERINFO *masterinfo,K2PDFOPT_SETTINGS *k2settings);
void masterinfo_free(MASTERINFO *masterinfo,K2PDFOPT_SETTINGS *k2settings);
void masterinfo_flush(MASTERINFO *masterinfo,K2PDFOPT_SETTINGS *k2settings);
int  masterinfo_new_source_page_init(MASTERINFO *masterinfo,K2PDFOPT_SETTINGS *k2settings,
                         WILLUSBITMAP *src,WILLUSBITMAP *srcgrey,WILLUSBITMAP *marked,
                         BMPREGION *region,double rot_deg,double *bormean,
                         char *rotstr,int pageno,FILE *out);
void masterinfo_add_bitmap(MASTERINFO *masterinfo,WILLUSBITMAP *src,
                    K2PDFOPT_SETTINGS *k2settings,int have_pagebox,
                    int justification_flags,int whitethresh,int nocr,int dpi);
void masterinfo_add_gap_src_pixels(MASTERINFO *masterinfo,K2PDFOPT_SETTINGS *k2settings,
                                   int pixels,char *caller);
void masterinfo_add_gap(MASTERINFO *masterinfo,K2PDFOPT_SETTINGS *k2settings,double inches);
void masterinfo_remove_top_rows(MASTERINFO *masterinfo,K2PDFOPT_SETTINGS *k2settings,int rows);
int masterinfo_get_next_output_page(MASTERINFO *masterinfo,K2PDFOPT_SETTINGS *k2settings,
                                    int flushall,WILLUSBITMAP *bmp,double *bmpdpi,
                                    int *size_reduction,void *ocrwords);

/* k2publish.c */
void masterinfo_publish(MASTERINFO *masterinfo,K2PDFOPT_SETTINGS *k2settings,int flushall);

/* k2ocr.c */
void k2ocr_init(K2PDFOPT_SETTINGS *k2settings);
void k2ocr_end(K2PDFOPT_SETTINGS *k2settings);
#ifdef HAVE_OCR_LIB
void k2ocr_ocrwords_fill_in_ex(K2PDFOPT_SETTINGS *k2settings,BMPREGION *region,
                 BREAKINFO *rowbreaks,int *colcount,int *rowcount,OCRWORDS *words);
#endif

/* pagelist.c */
int pagelist_valid_page_range(char *pagelist);
int pagelist_page_by_index(char *pagelist,int index,int maxpages);
int pagelist_count(char *pagelist,int maxpages);

/* k2bmp.c */
int    bmp_get_one_document_page(WILLUSBITMAP *src,K2PDFOPT_SETTINGS *k2pdfopt,
                              int src_type,char *filename,
                              int pageno,double dpi,int bpp,FILE *out);
double bmp_orientation(WILLUSBITMAP *bmp);
void   bmp_clear_outside_crop_border(WILLUSBITMAP *src,WILLUSBITMAP *srcgrey,
                                   K2PDFOPT_SETTINGS *k2settings);
double bmp_inflections_vertical(WILLUSBITMAP *srcgrey,int ndivisions,int delta,int *wthresh);
double bmp_inflections_horizontal(WILLUSBITMAP *srcgrey,int ndivisions,int delta,int *wthresh);
void   bmp_detect_vertical_lines(WILLUSBITMAP *bmp,WILLUSBITMAP *cbmp,double dpi,
                                      /* double minwidth_in, */
                                      double maxwidth_in,double minheight_in,double anglemax_deg,
                                      int white_thresh,int erase_vertical_lines,
                                      int debug,int verbose);
void   bmp_adjust_contrast(WILLUSBITMAP *src,WILLUSBITMAP *srcgrey,
                           K2PDFOPT_SETTINGS *k2settings,int *white);

/* k2mem.c */
void willus_dmem_alloc_warn(int index,void **ptr,int size,char *funcname,int exitcode);
void willus_dmem_free(int index,double **ptr,char *funcname);

/* devprofiles.c */
int         devprofiles_count(void);
char       *devprofile_alias(int index);
void        devprofiles_echo(FILE *out);
DEVPROFILE *devprofile_get(char *name);
int         devprofile_set_to_device(DEVPROFILE *dp);
char       *devprofile_select(void);

/* k2files.c */
void k2pdfopt_files_init(K2PDFOPT_FILES *k2files);
void k2pdfopt_files_free(K2PDFOPT_FILES *k2files);
void k2pdfopt_files_clear(K2PDFOPT_FILES *k2files);
void k2pdfopt_files_add_file(K2PDFOPT_FILES *k2files,char *filename);
void k2pdfopt_files_remove_file(K2PDFOPT_FILES *k2files,char *filename);

#endif /* __K2PDFOPT_H__ */
