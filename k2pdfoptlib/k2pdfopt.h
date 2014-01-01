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
** 0x0001 = Generic
** 0x0002 = breakinfo row analysis
** 0x0004 = word wrapping
** 0x0008 = word wrapping II
** 0x0010 = hyphens
** 0x0020 = OCR
** 0x0040 = crop boxes
** 0x0080 = column divider
** 0x0100 = breakinfo_find_doubles
** 0x0200 = 512 = gaps between big regions
** 0x0400 = 1024 = MuPDF "virtual" OCR
** 0x0800 = Put back "internal_gap" tracking (obsolete)
** 0x1000 = find text words
** 0x2000 = GUI
** 0x4000 = Keep console for GUI debugging
** 0x8000 = vertical line detection
**
*/

/* #define WILLUSDEBUGX 0x4000 */
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
/* #define K2PDFOPT_KINDLEPDFVIEWER  */

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
#ifdef HAVE_TESSERACT_LIB
#undef HAVE_TESSERACT_LIB
#endif
#ifdef HAVE_DJVU_LIB
#undef HAVE_DJVU_LIB
#endif
#ifdef HAVE_MUPDF_LIB
#undef HAVE_MUPDF_LIB
#endif
*/

#if (defined(HAVE_MUPDF) || defined(HAVE_GOCR_LIB) || defined(HAVE_TESSERACT_LIB))
#if (!defined(HAVE_OCR_LIB))
#define HAVE_OCR_LIB
#endif
#else
#if (defined(HAVE_OCR_LIB))
#undef HAVE_OCR_LIB
#endif
#endif

/* Compile w/Windows GUI? */
#if (defined(WIN32) || defined(WIN64))
#define HAVE_K2GUI
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
#define SRCROT_AUTO     -999.
#define SRCROT_AUTOEP   -998.
#define SRCROT_AUTOPREV -997.

#define OR_DETECT(x)     (fabs((x)-SRCROT_AUTO)<.5)
#define OREP_DETECT(x)   (fabs(x->src_rot-SRCROT_AUTOEP)<.5)
#define int_swap(x,y) {int t; t=(x); (x)=(y); (y)=t;}
#define double_swap(x,y) {double t; t=(x); (x)=(y); (y)=t;}
/*
** Determine if x1 and x2 agree to within m percent
*/
#define AGREE_WITHIN_MARGIN(x1,x2,m)  ((x1)>0. && (x2)>0. \
                            && ( ((x2)<(x1) && (double)(x1)/(x2)-1.<(double)(m)/100.) \
                                  || ((x1)<=(x2) && (double)(x2)/(x1)-1.<(double)(m)/100.)))

/*
** DATA STRUCTURES
*/
typedef struct
    {
    char pagelist[256];
    double box[4]; /* index 0..3 = left,top,right,bottom */
    int units[4];
    } K2CROPBOX;

#define MAXK2CROPBOXES 32

typedef struct
    {
    K2CROPBOX cropbox[MAXK2CROPBOXES];
    int n;
    } K2CROPBOXES;

/*
** K2PDFOPT_SETTINGS stores user settings that affect the document processing.
*/
typedef struct
    {
    /* debugging */
    int verbose;
    int debug;

#ifdef HAVE_K2GUI
    /* GUI */
    int gui;
    int guimin;
#endif

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
    char ocrout[128];
    int dst_ocr;
#ifdef HAVE_TESSERACT_LIB
    char dst_ocr_lang[64];
#endif
    int dst_ocr_visibility_flags;
    int ocr_max_columns;
    double ocr_max_height_inches;
    OCRWORDS dst_ocrwords;
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
    int auto_word_spacing; /* Used by KOReader */
    double display_width_inches; /* Device width = dst_width / dst_dpi */
    char pagelist[1024];
    char bpl[2048];  /* Page break list--see -bpl option */
    int use_toc;
    char toclist[2048];
    char tocsavefile[MAXFILENAMELEN];
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
    K2CROPBOXES cropboxes; /* Crop boxes */
    /*
    ** Preview options
    */
    int preview_page;  /* 0=no preview */
    int echo_source_page_count;
    /*
    ** Undocumented cmd-line args
    */
    double no_wrap_ar_limit; /* -arlim */
    double no_wrap_height_limit_inches; /* -whmax */
    double little_piece_threshold_inches; /* -rwmin */
    /*
    ** Flag for setting device size--see k2pdfoptsettings_set_margins_and_devsize().
    */
    int devsize_set; /* 0 = device size not set yet */
    } K2PDFOPT_SETTINGS;


/* Mostly for GUI */
typedef struct
    {
    int filecount;
    WILLUSBITMAP *bmp;
    char *outname;
    int status; /* 0 = success, otherwise, status code */
    } K2PDFOPT_OUTPUT;


/* List of files to be processed by k2pdfopt */
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
** TEXTROW describes a row of text
*/
#define REGION_TYPE_UNDETERMINED 0
#define REGION_TYPE_TEXTLINE     1
#define REGION_TYPE_MULTILINE    2
#define REGION_TYPE_WORD         3
#define REGION_TYPE_FIGURE       4
#define REGION_TYPE_MULTIWORD    5
typedef struct
    {
    int c1,c2;   /* Left and right columns */
    int r1,r2;   /* Top and bottom of region in pixels */
    int rowbase; /* Baseline of row */
    int gap;     /* Gap between next region and this region's rowbase. */
    int gapblank;  /* Actual blank area between next region and this region. */
    int rowheight; /* text + gap (delta between rowbases) */
    int capheight;
    int h5050;
    int lcheight;
    int type;    /* See region type #defines above */
    double rat;  /* If found with find_doubles, this is > 0 (the figure of merit) */
    HYPHENINFO hyphen;
    } TEXTROW;


/*
** Collection of text rows
*/
typedef struct
    {
    TEXTROW *textrow;
    int n,na;
    } TEXTROWS;

typedef TEXTROW  TEXTWORD;
typedef TEXTROWS TEXTWORDS;

/*
** Helps to determine line spacing in multi-line text region
*/
typedef struct
    {
    int i1,i2;
    int maxlines;
    int nlines;
    double median_line_spacing; /* Pixels */
    double median_capheight; /* Pixels */
    double median_lcheight; /* Pixels */
    double median_h5050; /* Pixels */
    double mean_row_gap; /* Pixels */
    int mingap; /* Pixels */
    int maxgap; /* Pixels */
    int raggedright;
    int *just;           /* Justification flags for line */
    int *indented;       /* Whether line is indented or not */
    int *short_line;     /* Whether line is a shorter than average line */
    } MULTILINE_PARAMS;

/*
** WRECTMAP and WRECTMAPS are used to map word rectangles to the
** original PDF source page.
*/
typedef struct
    {
    int srcpageno;
    int srcwidth;  /* pixels */
    int srcheight; /* pixels */
    double srcdpiw;
    double srcdpih;
    int srcrot;    /* deg */
    POINT2D coords[3];     /* [0]=top left corner of source page bitmap (pixels) */
                           /* [1]=top left corner of wrapbmp bitmap (pixels) */
                           /* [2]=width,height of region (pixels) */
    } WRECTMAP;

typedef struct
    {
    WRECTMAP *wrectmap;
    int n,na;
    } WRECTMAPS;
    
/*
** BMPREGION is a rectangular region within a bitmap.  This is the main
** data structure used by k2pdfopt to break up the source page.
*/
typedef struct
    {
    int r1,r2;      /* row position from top of bmp, inclusive */
    int c1,c2;      /* column positions, inclusive */
    TEXTROWS textrows; /* If nrows>0, top and bottom (if nrows>11) text row of region */
    TEXTROW bbox;   /* Bounding box of region.  type==REGION_TYPE_UNDETERMINED if not calced yet */
    WRECTMAPS *wrectmaps; /* If region consists of multiple, separated source rectangles
                          ** (like from wrapbmp structure), then this is non-null and maps
                          ** the bitmap region to the source page.
                          */
    int bgcolor;    /* Background color of region, 0 - 255 */
    int dpi;        /* dpi of bitmap */
    int pageno;     /* Source page number, -1 if unknown */
    int rotdeg;     /* Source rotation, degrees, counterclockwise */
    int *colcount;  /* Always check for NULL before using */
    int *rowcount;  /* Always check for NULL before using */
    WILLUSBITMAP *bmp;
    WILLUSBITMAP *bmp8;
    WILLUSBITMAP *marked;
    } BMPREGION;


/*
** BREAKINFO contains information on contiguous regions within a BMPREGION.
** These may be rows of text or words within a row of text.
*/
#ifdef COMMENT
typedef struct
    {
    TEXTROW *textrow;
    int rhmean_pixels;  /* Mean row height (text) */
    int centered;       /* Is this set of rows centered? */
    int n,na;
    } BREAKINFO;
#endif

/*
** WRAPBMP contains a cached bitmap where bitmaps containing a word or a collection
** of words on collected onto a single line during text re-flow.  This bitmap
** (line of text) is sent to the master output bitmap (flushed) every time the
** text line fills up.
*/
typedef struct
    {
    WILLUSBITMAP bmp;
    int base;
    int bgcolor;
    int just;
    int rhmax;
    int thmax;
    int maxgap;
    int height_extended;
    int just_flushed_internal;
    int mandatory_region_gap; /* Copies from masterinfo at wrapbmp_add, reset at wrapbmp_flush */
    double page_region_gap_in;  /* Copies from masterinfo like mandatory_region_gap */
    TEXTROW textrow;  /* Keep text line statistics */
    WRECTMAPS wrectmaps;
    HYPHENINFO hyphen;
    } WRAPBMP;

/*
** MASTERINFO contains performance parameters relevant to the device output.
** (E.g. the "master" bitmap which is a running scroll of content meant to
**  be sent to the output device.)
*/
typedef struct
    {
    char srcfilename[MAXFILENAMELEN];
    char ocrfilename[MAXFILENAMELEN];
    int outline_srcpage_completed; /* Which source page was last checked in the outline */
    PDFFILE outfile;      /* PDF output file data structure */
    WPDFOUTLINE *outline; /* PDF outline / bookmarks structure--loaded by MuPDF only */
    WILLUSBITMAP bmp; /* Master output bitmap collects pages that will go to */
                      /* the output device */
    WILLUSBITMAP *preview_bitmap;
    int preview_captured;  /* = 1 if preview bitmap obtained */
    WRAPBMP wrapbmp;  /* See WRAPBMP structure */
#ifdef K2PDFOPT_KINDLEPDFVIEWER
    WRECTMAPS rectmaps;   /* KOReader add to hold WRECTMAPs of the output bitmap */
#endif
    WPDFPAGEINFO pageinfo;  /* Holds crop boxes for native PDF output */
    int srcpages;         /* Total pages in source file */
    int rows;             /* Rows stored within the bmp structure */
    int published_pages;  /* Count of published pages */
    int bgcolor;
    int fit_to_page;
    int wordcount;
    char debugfolder[256];
    /*
    ** Details on last region added--used to help determine spacing to next region.
    */
    TEXTROW lastrow;     /* lastrow->gap and gapblank are as passed.  They represent
                         ** the gaps as they were in the source doc (except at the
                         ** dpi of the destination doc).
                         */
    int nocr;            /* scaling value used on lastrow */
    int gapblank;        /* Current white-space pixel gap at bottom of master bitmap */
    int mandatory_region_gap; /* 1 = put in the page_region_gap_in below. */
                         /* OLD:
                         ** 0 if between lines in a multi-line region
                         ** 1 if adding a region from a new vertical region.
                         ** 2 if adding a new page region
                         ** 3 if adding a region which is on a new page
                         ** 4 if adding the first region on the first page
                         */
    double page_region_gap_in;  /* Gap between page regions.  If new page, gap between
                                ** top of page and new region.
                                */
#if 0
    int fontsize;    /* Font size of last row added (pixels).  < 0 = no last font */
    int linespacing; /* Line spacing of last row added (pixels) */
    double rowheight_in; /* Height of last row added */
    int gap;         /* Gap after baseline of last row added (pixels) */
    int last_row_type; /* Type of last text row added */
    int gapblank;      /* Gap between last text row added and next non-blank row of pixels */
#endif
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
void k2pdfopt_proc_wildarg(K2PDFOPT_SETTINGS *k2settings,char *arg,int process,
                           K2PDFOPT_OUTPUT *k2out);
void wpdfboxes_echo(WPDFBOXES *boxes,FILE *out);

/* k2sys.c */
void k2sys_init(void);
void k2sys_close(K2PDFOPT_SETTINGS *k2settings);
void k2sys_header(char *s);
void k2sys_exit(K2PDFOPT_SETTINGS *k2settings,int val);
void k2sys_enter_to_exit(K2PDFOPT_SETTINGS *k2settings);
int  k2printf(char *fmt,...);
void k2gets(char *buf,int maxlen,char *def);

/* k2usage.c */
void k2usage_show_all(FILE *out);
void k2usage_to_string(char *s);
int  k2usage_len(void);
int  k2pdfopt_usage(void);

/* k2parsecmd.c */
int parse_cmd_args(K2PDFOPT_CONVERSION *k2conv,STRBUF *env,STRBUF *cmdline,
                   STRBUF *userinput,int setvals,int quiet);

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
                                        int r1,int r2,int cmid);
void bmpregion_init(BMPREGION *region);
void bmpregion_free(BMPREGION *region);
void bmpregion_copy(BMPREGION *dst,BMPREGION *src,int copy_textrows);
void bmpregion_calc_bbox(BMPREGION *region,K2PDFOPT_SETTINGS *k2settings,int calc_text_params);
void bmpregion_trim_margins(BMPREGION *region,K2PDFOPT_SETTINGS *k2settings,int flags);
void bmpregion_hyphen_detect(BMPREGION *region,int hyphen_detect,int left_to_right);
int  bmpregion_textheight(BMPREGION *region,K2PDFOPT_SETTINGS *k2settings,int i1,int i2);
int  bmpregion_is_centered(BMPREGION *region,K2PDFOPT_SETTINGS *k2settings,int i1,int i2);
void bmpregion_get_white_margins(BMPREGION *region);
void bmpregion_find_textrows(BMPREGION *region,K2PDFOPT_SETTINGS *k2settings,
                             int dynamic_aperture,int remove_small_rows);
void bmpregion_one_row_find_textwords(BMPREGION *region,K2PDFOPT_SETTINGS *k2settings,
                                      int add_to_dbase);
void textrow_echo(TEXTROW *textrow,FILE *out);


/* pageregions.c */
/*
** The PAGEREGIONS structure holds the set of regions which are determined
** during the first pass of the page parsing.  See the pageregions_find_columns()
** function in k2proc.c
*/
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
void pageregion_free(PAGEREGION *region);
void pageregion_init(PAGEREGION *region);
void pageregion_copy(PAGEREGION *dst,PAGEREGION *src);


/* textrows.c */
void textrows_init(TEXTROWS *textrows);
void textrows_free(TEXTROWS *textrows);
void textrows_clear(TEXTROWS *textrows);
void textrows_delete_one(TEXTROWS *textrows,int index);
void textrows_insert(TEXTROWS *dst,int index,TEXTROWS *src);
void textrows_add_textrow(TEXTROWS *textrows,TEXTROW *textrow);
void textrow_init(TEXTROW *textrow);
void textrows_add_bmpregion(TEXTROWS *textrows,BMPREGION *bmpregion,int type);
int  textrow_line_spacing_is_same(TEXTROW *tr1,TEXTROW *tr2,int margin);
int  textrow_font_size_is_same(TEXTROW *tr1,TEXTROW *tr2,int margin);
void textrows_compute_row_gaps(TEXTROWS *textrows,int r2);
void textrows_sort_by_gap(TEXTROWS *textrows);
void textrows_sort_by_row_position(TEXTROWS *textrows);
void textrows_find_doubles(TEXTROWS *textrows,int *rowthresh,BMPREGION *region,
                           K2PDFOPT_SETTINGS *k2settings,int maxsize);
void textrows_remove_small_rows(TEXTROWS *textrows,K2PDFOPT_SETTINGS *k2settings,
                                double fracrh,double fracgap,BMPREGION *region);
void textrow_determine_type(BMPREGION *region,K2PDFOPT_SETTINGS *k2settings,int index);
void textrow_scale(TEXTROW *textrow,double scalew,double scaleh,int c2max,int r2max);
#if (WILLUSDEBUGX & 6)
void textrows_echo(TEXTROWS *textrows,char *name);
#endif

/* textwords.c */
void textwords_compute_col_gaps(TEXTWORDS *textwords,int c2);
void textwords_remove_small_col_gaps(TEXTWORDS *textwords,int lcheight,double mingap,
                                     double word_spacing);
void textwords_add_word_gaps(TEXTWORDS *textwords,int lcheight,double *median_gap,
                             double word_spacing);
#define textwords_init(x) textrows_init(x)
#define textwords_free(x) textrows_free(x)
#define textwords_clear(x) textrows_clear(x)
#define textwords_delete_one(x,y) textrows_delete_one(x,y)
#define textwords_insert(x,y,z) textrows_insert(x,y,z)
#define textwords_add_textword(x,y) textrows_add_textrow(x,y)
#define textwords_add_bmpregion(x,y,z) textrows_add_bmpregion(x,y,z)
#define textword_init(x) textrow_init(x)


/* k2proc.c */
void k2proc_init_one_document(void);
void bmpregion_source_page_add(BMPREGION *region,K2PDFOPT_SETTINGS *k2settings,
                               MASTERINFO *masterinfo,int level,int pages_done);
void pageregions_find_columns(PAGEREGIONS *pageregions_sorted,BMPREGION *srcregion,
                              K2PDFOPT_SETTINGS *k2settings,int  maxlevels);
void bmpregion_add(BMPREGION *region,K2PDFOPT_SETTINGS *k2settings,
                   MASTERINFO *masterinfo,int allow_text_wrapping,
                   int trim_flags,int allow_vertical_breaks,
                   double force_scale,int justify_flags,int caller_id,
                   int mark_flags,int rowbase_delta,int region_is_centered);
double line_spacing_from_font_size(double lcheight,double h5050,double capheight);


/* k2settings.c */
void k2pdfopt_settings_init(K2PDFOPT_SETTINGS *k2settings);
void k2pdfopt_conversion_init(K2PDFOPT_CONVERSION *k2conv);
void k2pdfopt_conversion_close(K2PDFOPT_CONVERSION *k2conv);
void k2pdfopt_settings_copy(K2PDFOPT_SETTINGS *dst,K2PDFOPT_SETTINGS *src);
int  k2pdfopt_settings_set_to_device(K2PDFOPT_SETTINGS *k2settings,DEVPROFILE *dp);
void k2pdfopt_settings_quick_sanity_check(K2PDFOPT_SETTINGS *k2settings);
void k2pdfopt_settings_sanity_check(K2PDFOPT_SETTINGS *k2settings);
void k2pdfopt_settings_new_source_document_init(K2PDFOPT_SETTINGS *k2settings);
void k2pdfopt_settings_restore_output_dpi(K2PDFOPT_SETTINGS *k2settings);
void k2pdfopt_settings_fit_column_to_screen(K2PDFOPT_SETTINGS *k2settings,
                                            double column_width_inches);
void k2pdfopt_settings_set_region_widths(K2PDFOPT_SETTINGS *k2settings);
int k2settings_gap_override(K2PDFOPT_SETTINGS *k2settings);
void k2pdfopt_settings_set_margins_and_devsize(K2PDFOPT_SETTINGS *k2settings,
                         BMPREGION *region,MASTERINFO *masterinfo,int trimmed);
int devsize_pixels(double user_size,int user_units,double source_size_in,
                   double trimmed_size_in,double dst_dpi,int flags);


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
                 MASTERINFO *masterinfo,int colgap,int justification_flags);
void wrapbmp_flush(MASTERINFO *masterinfo,K2PDFOPT_SETTINGS *k2settings,int allow_full_justify);
void wrectmaps_init(WRECTMAPS *wrectmaps);
void wrectmaps_free(WRECTMAPS *wrectmaps);
void wrectmaps_clear(WRECTMAPS *wrectmaps);
void wrectmaps_add_wrectmap(WRECTMAPS *wrectmaps,WRECTMAP *wrectmap);
void wrectmaps_scale_wrapbmp_coords(WRECTMAPS *wrectmaps,double scalew,double scaleh);
int  wrectmap_inside(WRECTMAP *wrmap,int xc,int yc);
void wrectmaps_sort_horizontally(WRECTMAPS *wrectmaps);

/* k2master.c */
void masterinfo_init(MASTERINFO *masterinfo,K2PDFOPT_SETTINGS *k2settings);
void masterinfo_free(MASTERINFO *masterinfo,K2PDFOPT_SETTINGS *k2settings);
void masterinfo_flush(MASTERINFO *masterinfo,K2PDFOPT_SETTINGS *k2settings);
int  masterinfo_new_source_page_init(MASTERINFO *masterinfo,K2PDFOPT_SETTINGS *k2settings,
                         WILLUSBITMAP *src,WILLUSBITMAP *srcgrey,WILLUSBITMAP *marked,
                         BMPREGION *region,double rot_deg,double *bormean,
                         char *rotstr,int pageno,FILE *out);
void masterinfo_add_bitmap(MASTERINFO *masterinfo,WILLUSBITMAP *src,
                    K2PDFOPT_SETTINGS *k2settings,int npageboxes,
                    int justification_flags,int whitethresh,int nocr,int dpi,
                    WRECTMAPS *wrectmaps,TEXTROW *textrow);
/*
void masterinfo_add_gap_src_pixels(MASTERINFO *masterinfo,K2PDFOPT_SETTINGS *k2settings,
                                   int pixels,char *caller);
void masterinfo_add_gap(MASTERINFO *masterinfo,K2PDFOPT_SETTINGS *k2settings,double inches);
*/
void masterinfo_remove_top_rows(MASTERINFO *masterinfo,K2PDFOPT_SETTINGS *k2settings,int rows);
int masterinfo_get_next_output_page(MASTERINFO *masterinfo,K2PDFOPT_SETTINGS *k2settings,
                                    int flushall,WILLUSBITMAP *bmp,double *bmpdpi,
                                    int *size_reduction,void *ocrwords);
int masterinfo_should_flush(MASTERINFO *masterinfo,K2PDFOPT_SETTINGS *k2settings);

/* k2publish.c */
void masterinfo_publish(MASTERINFO *masterinfo,K2PDFOPT_SETTINGS *k2settings,int flushall);

/* k2ocr.c */
void k2ocr_init(K2PDFOPT_SETTINGS *k2settings);
void k2ocr_end(K2PDFOPT_SETTINGS *k2settings);
#ifdef HAVE_OCR_LIB
void k2ocr_ocrwords_fill_in_ex(OCRWORDS *words,BMPREGION *region,K2PDFOPT_SETTINGS *k2settings);
#endif

/* pagelist.c */
int pagelist_valid_page_range(char *pagelist);
int pagelist_includes_page(char *pagelist,int pageno,int maxpages);
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
char       *devprofile_name(int index);
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

#ifdef HAVE_K2GUI

#if ((defined(WIN32) || defined(WIN64)))
#ifndef MSWINGUI
#define MSWINGUI
#endif
#endif

#define K2WIN_MINWIDTH  600
#define K2WIN_MINHEIGHT 440
#define MAXGUICONTROLS  64

/*
** K2GUI contains the parameters related to the functioning of
** the k2pdfopt graphical user interface.
*/
typedef struct
    {
    WILLUSGUIWINDOW mainwin;
    WILLUSGUICONTROL control[MAXGUICONTROLS];
    int ncontrols;
    int control_selected; /* -1 = none */
    int time_selected_ms; /* milliseconds */
    /* WILLUSGUIOSDEP osdep; */
    /* HINSTANCE hinst; */
    /* WNDPROC eclass2proc; */
    int started;
    WILLUSGUIFONT font;
    int active;
    int preview_processing;
    WILLUSBITMAP bgbmp;
    K2PDFOPT_CONVERSION *k2conv;
    STRBUF *env;
    /* STRBUF *cmdline; */
    STRBUF  cmdxtra;
    WILLUSBITMAP pworking; /* Preview bitmap "working" */
    WILLUSBITMAP pviewbitmap; /* Preview bitmap source */
    WILLUSBITMAP pbitmap; /* Preview bitmap source */
    void *prevthread[8]; /* Preview thread controls */
    } K2GUI;

/*
** K2CONVBOX parameters affect the operation of the k2pdfopt
** conversion dialog box (GUI operation only).
*/
typedef struct
    {
    WILLUSGUIFONT mf;
    WILLUSGUIFONT bf;
    WILLUSGUIWINDOW mainwin;
    int  rgbcolor;
    WILLUSGUICONTROL control[4]; /* 0 = edit box, 1-3 = buttons */
    int  ncontrols;
    STRBUF buf;
    int  maxwidth;
    int  width;
    int  height;
    int  status;  /* Termination status.  -1 = ESC press, 1 = button press */ 
    void *hinst;     /* Used by MS Windows */
    WILLUSGUIRECT aboutbox;
    void *semaphore; /* Semaphore controlling forked thread */
    void *pid;       /* handle to forked thread */
    int converting;
    int successful;
    int openfiles;
    int num_files;
    int num_pages;
    int error_count;
    char filename[256];
    char *filelist; /* Double '\0' terminated string */
    int filelist_na;
    } K2CONVBOX;

/* k2gui.c */
void k2gui_settings_to_cmdline(STRBUF *cmdline,K2PDFOPT_SETTINGS *k2settings);
int  k2gui_main(K2PDFOPT_CONVERSION  *k2conv,void *hInstance,void *hPrevInstance,
                STRBUF *env,STRBUF *cmdline);
void k2gui_start_conversion(void *data);
int  k2gui_conversion_successful(void);
void k2gui_conversion_thread_cleanup(void);
void k2gui_destroy_conversion_dialog_box(void);
int  k2gui_alertbox(int retval,char *title,char *message);
void k2gui_get_custom_name(int index,char *name,int maxlen);
void k2gui_get_custom_options(int index,K2PDFOPT_SETTINGS *k2settings,
                              STRBUF *extra);
void k2gui_get_settings(int index,K2PDFOPT_SETTINGS *k2settings,STRBUF *extra);
void k2gui_save_settings(int index,K2PDFOPT_SETTINGS *k2settings,STRBUF *extra);
int  k2gui_wait_for_conversion_dialog_box_messages(void);
int  k2gui_vprintf(FILE *f,char *fmt,va_list args);
void k2gui_set_files_completed(int nfiles,char *message);
void k2gui_set_pages_completed(int n,char *message);
void k2gui_update_progress_bar(int index,double progress,int color,char *message);
void k2gui_set_num_files(int nfiles);
void k2gui_set_num_pages(int npages);
void k2gui_set_error_count(int ecount);
void k2gui_increment_error_count(void);
void k2gui_set_filename(char *name);
int  k2gui_active(void);
void k2gui_control_select(int index,int selected);
void k2gui_timer_event(void);
void k2gui_process_message(WILLUSGUIMESSAGE *message);
void k2gui_window_minsize(int *width_pixels,int *height_pixels);
void k2gui_quit(void);
int  k2gui_messagebox(int retval,char *title,char *fmt,...);
int  k2gui_get_text(char *title,char *message,char *button1,char *button2,
                    char *textbuf,int maxlen);
void k2gui_add_file(char *filename);
void k2gui_main_repaint(int changing);
char *k2gui_short_name(char *filename);
void k2gui_preview_toggle_size(int increment);
void k2gui_preview_refresh(void);
void k2gui_preview_paint(void);
int  k2gui_previewing(void);

/* k2gui_cbox.c */
int  k2gui_cbox_converting(void);
int  k2gui_yesno(char *title,char *fmt,...);
void k2gui_cbox_do_conversion(K2GUI *k2gui0);
void k2gui_cbox_final_print(void);
void k2gui_cbox_terminate_conversion(void);
int  k2gui_cbox_conversion_successful(void);

/* Functions for opening converted files / folders */
void  k2gui_cbox_error(char *filename,int statuscode);
void  k2gui_cbox_open_files(void);
void  k2gui_cbox_open_folders(void);
void  k2gui_cbox_freelist(void);

void k2gui_cbox_set_files_completed(int nfiles,char *message);
void k2gui_cbox_set_pages_completed(int n,char *message);
void k2gui_cbox_set_num_files(int nfiles);
void k2gui_cbox_set_num_pages(int npages);
void k2gui_cbox_set_filename(char *name);
void k2gui_cbox_set_error_count(int ecount);
void k2gui_cbox_increment_error_count(void);
int  k2gui_cbox_vprintf(FILE *f,char *fmt,va_list args);
void k2gui_cbox_draw_defbutton_border(int status);
void k2gui_cbox_close_buttons(void);
void k2gui_cbox_destroy(void);

/* k2gui_osdep.c */
void k2gui_osdep_init(K2GUI *k2gui0);
int  k2gui_osdep_window_proc_messages(WILLUSGUIWINDOW *win,void *semaphore,WILLUSGUICONTROL *closebutton);
void k2gui_osdep_main_window_init(WILLUSGUIWINDOW *win,int normal_size);
void k2gui_osdep_cbox_init(K2CONVBOX *k2cb0,WILLUSGUIWINDOW *win,WILLUSGUIWINDOW *parent,
                           void *hinst,int rgbcolor);
void k2gui_osdep_mainwin_init_after_create(WILLUSGUIWINDOW *win);
void k2gui_osdep_main_repaint(int changing);

/* k2settings2cmd.c */
int  k2settings_sprintf(STRBUF *cmdline,K2PDFOPT_SETTINGS *k2settings,char *fmt,...);
void k2pdfopt_settings_get_cmdline(STRBUF *cmdline,K2PDFOPT_SETTINGS *dst,
                                   K2PDFOPT_SETTINGS *src);
#endif /* K2GUI */

#endif /* __K2PDFOPT_H__ */
