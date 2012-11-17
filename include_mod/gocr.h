/*
This is a Optical-Character-Recognition program
Copyright (C) 2000-2010 Joerg Schulenburg

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

 see README for EMAIL-address

  sometimes I have written comments in german language, sorry for that

 - look for ??? for preliminary code
*/
 
/* General headerfile with gocr-definitions */

#ifndef __GOCR_H__
#define __GOCR_H__

/* Handle PNM-files  Dez98 JS
 * 0,0 = left up
 * PAM-formats
 * PAM any  P7  
 * PNM-formats
 * PGM gray ASCII=P2 RAW=P5 dx dy col gray
 * PPM RGB  ASCII=P3 RAW=P6 dx dy col RGB
 * PBM B/W  ASCII=P1 RAW=P4 dx dy     bitmap
 */

#ifndef GOCR_PNM_H
#define GOCR_PNM_H 1

/* include/config.h.in.  Generated automatically from configure.in by autoheader.  */

/* Define to empty if the keyword does not work.  */
#undef const

/* Define if the setvbuf function takes the buffering type as its second
   argument and the buffer pointer as the third, as on System V
   before release 3.  */
#undef SETVBUF_REVERSED

/* Define if you have the ANSI C header files.  */
#undef STDC_HEADERS

/* Define if you have the gettimeofday function.  */
#undef HAVE_GETTIMEOFDAY

/* Define if you have the popen function.  */
#undef HAVE_POPEN

/* Define if you have the wcschr function.  */
#undef HAVE_WCSCHR

/* Define if you have the wcsdup function.  */
#undef HAVE_WCSDUP

/* Define if you have the <pam.h> header file.  */
#undef HAVE_PAM_H

/* Define if you have the <pnm.h> header file.  */
#undef HAVE_PNM_H

/* Define if you have the <unistd.h> header file.  */
#undef HAVE_UNISTD_H

/* Define if you have the <wchar.h> header file.  */
#ifndef HAVE_WCHAR_H
#define HAVE_WCHAR_H
#endif

struct pixmap {
   unsigned char *p;	/* pointer of image buffer (pixmap) */
   int x;		/* xsize */
   int y;		/* ysize */
   int bpp;		/* bytes per pixel:  1=gray 3=rgb */
 };
typedef struct pixmap pix;

/* return 1 on multiple images (holding file open), 0 else */
int readpgm(char *name, pix *p, int vvv);

/* write pgm-map to pnm-file */
int writepgm(char *nam, pix *p);
int writepbm(char *nam, pix *p);
int writeppm(char *nam, pix *p); /* use lowest 3 bits for farbcoding */

/* ----- count colors ------ create histogram ------- */
void makehisto(pix p, unsigned col[256], int vvv);

#endif
/*
This is a Optical-Character-Recognition program
Copyright (C) 2000-2010  Joerg Schulenburg

   The character codes in this file are Copyright (c) 1991-1999 Unicode, Inc.
   All Rights reserved.

   This file is provided as-is by Unicode, Inc. (The Unicode Consortium).
   No claims are made as to fitness for any particular purpose.  No
   warranties of any kind are expressed or implied.  The recipient
   agrees to determine applicability of information provided.  If this
   file has been provided on optical media by Unicode, Inc., the sole
   remedy for any claim will be exchange of defective media within 90
   days of receipt.

   Unicode, Inc. hereby grants the right to freely use the information
   supplied in this file in the creation of products supporting the
   Unicode Standard, and to make copies of this file in any form for
   internal or external distribution as long as this notice remains
   attached.

For the rest of the file, the following applies:

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

 see README for EMAIL-address
 */
 
#ifndef G_UNICODE_H
#define G_UNICODE_H

#include <stddef.h>

enum format {
	ISO8859_1, TeX, HTML, XML, SGML, UTF8, ASCII
};
typedef enum format FORMAT;

/*
 * Prototypes
 */
wchar_t compose(wchar_t main, wchar_t modifier);
const char *decode(wchar_t c, FORMAT type);

/*
 * Unicode codes moved to unicode_defs.h avoiding macro name conflicts
 *  JS Aug2010
 */

#endif
/*
This is a Optical-Character-Recognition program
Copyright (C) 2000-2010  Joerg Schulenburg

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

 see README for EMAIL-address
 
 */

#ifndef GOCR_LIST_H
#define GOCR_LIST_H

/* ./configure --with-debug ??? DO_DEBUG? */
#ifdef DEBUG
#define g_debug(a)	a
#else
#define g_debug(a)	
#endif

/*
 * Structures
 */
 
struct ocr_element {
   struct ocr_element *next, *previous;
   void *data;
};
typedef struct ocr_element Element;

struct ocr_object_list {
   Element start;               /* simplifies for(each_element) { ... */
   Element stop;                /*   ... list_del() ... }  v0.41      */
   Element **current;	 	/* for(each_element) */
   int n;			/* number of elements */
   int level;			/* level of nested fors */
};
typedef struct ocr_object_list List;

/*
 * Functions
 */

void	list_init		( List *l );
int	list_app		( List *l, void *data );
int	list_ins		( List *l, void *data_after, void *data);
Element*list_element_from_data	( List *l, void *data );
int	list_del		( List *l, void *data );
void	list_free		( List *l );
int	list_and_data_free	( List *l, void (*free_data)(void *data));
int	list_higher_level	( List *l );
void	list_lower_level	( List *l );
void *	list_next		( List *l, void *data );
void *	list_prev		( List *l, void *data );
void	list_sort		( List *l, int (*compare)(const void *, const void *) );

#define list_empty(l)			((l)->start.next == &(l)->stop ? 1 : 0)
#define list_get_header(l)		((l)->start.next->data)
#define list_get_tail(l)		((l)->stop.previous->data)
#define list_get_current(l)		((l)->current[(l)->level]->data)
#define list_get_cur_prev(l)		((l)->current[(l)->level]->previous == NULL ? \
			NULL : (l)->current[(l)->level]->previous->data )
#define list_get_cur_next(l)		((l)->current[(l)->level]->next == NULL ? \
			NULL : (l)->current[(l)->level]->next->data )
#define list_total(l)			((l)->n)

#define for_each_data(l)		\
 if (list_higher_level(l) == 0) { \
   for ( ; (l)->current[(l)->level] \
        && (l)->current[(l)->level]!=&(l)->stop; (l)->current[(l)->level] = \
	(l)->current[(l)->level]->next ) {


#define end_for_each(l)			\
   } \
 list_lower_level(l); \
 }

#endif
#include <stddef.h>
#ifdef HAVE_GETTIMEOFDAY
#include <sys/time.h>
#endif

/*
 *  wchar_t should always exist (ANSI), but WCHAR.H is sometimes missing
 *  USE_UNICODE should be removed or replaced by HAVE_WCHAR_H in future
 */
#ifdef HAVE_WCHAR_H
#define USE_UNICODE 1
#endif

/* extern "C"{ */
/* ------------------------ feature extraction ----------------- */
#define AT 7	/* mark */
#define M1 1	/* mark */
enum direction {
  UP=1, DO, RI, LE
};
typedef enum direction DIRECTION;
#define ST 7    /* stop */
/* ------------------------------------------------------------- */
/* detect maximas in of line overlapps (return in %) and line koord */
#define HOR 1    /* horizontal */
#define VER 2    /* vertikal   */
#define RIS 3    /* rising=steigend */
#define FAL 4    /* falling=fallend */

#define MAXlines 1024

/* ToDo: if we have a tree instead of a list, a line could be a node object */
struct tlines {
    int num;
    int dx, dy;		/* direction of text lines (straight/skew) */
    int m1[MAXlines],   /* start of line = upper bound of 'A' */
        m2[MAXlines],   /* upper bound of 'e' */
        m3[MAXlines],	/* lower bound of 'e' = baseline */
        m4[MAXlines];	/* stop of line = lower bound of 'q' */
    /* ToDo: add sureness per m1,m2 etc? */
    int x0[MAXlines],
        x1[MAXlines];	/* left and right border */
    int wt[MAXlines];   /* weight, how sure thats correct in percent, v0.41 */
    int pitch[MAXlines]; /* word pitch (later per box?), v0.41 */
    int mono[MAXlines];  /* spacing type, 0=proportional, 1=monospaced */
};

#define NumAlt 10 /* maximal number of alternative chars (table length) */
#define MaxNumFrames 8       /* maximum number of frames per char/box */
#define MaxFrameVectors 128  /* maximum vectors per frame (*8=1KB/box) */
/* ToDo: use only malloc_box(),free_box(),copybox() for creation, destroy etc.
 *       adding reference_counter to avoid pointer pointing to freed box
 */
struct box { /* this structure should contain all pixel infos of a letter */
    int x0,x1,y0,y1,x,y,dots; /* xmin,xmax,ymin,ymax,reference-pixel,i-dots */
    int num_boxes, /* 1 "abc", 2 "!i?", 3 "&auml;" (composed objects) 0.41 */
        num_subboxes;   /* 1 for "abdegopqADOPQR", 2 for "B"  (holes) 0.41 */
    wchar_t c;		/* detected char (same as tac[0], obsolete?) */
    wchar_t modifier;	/* default=0, see compose() in unicode.c */
    int num;		/* same number = same char */
    int line;		/* line number (points to struct tlines lines) */
    int m1,m2,m3,m4;	/* m2 = upper boundary, m3 = baseline */
    /* planed: sizeof hole_1, hole_2, certainty (run1=100%,run2=90%,etc.) */
    pix *p;		/* pointer to pixmap (v0.2.5) */
    /* tac, wac is used together with setac() to manage very similar chars */
    int num_ac;         /* length of table (alternative chars), default=0 */
    wchar_t tac[NumAlt]; /* alternative chars, only used by setac(),getac() */
    int     wac[NumAlt]; /* weight of alternative chars */
    char   *tas[NumAlt]; /* alternative UTF8-strings or XML codes if tac[]=0 */
                         /*   replacing old obj */
	                 /* ToDo: (*obj)[NumAlt] + olen[NumAlt] ??? */
	                 /* ToDo: bitmap for possible Picture|Object|Char ??? */
/*  char    *obj; */     /* pointer to text-object ... -> replaced by tas[] */
                         /*  ... (melted chars, barcode, picture coords, ...) */
                         /*  must be freed before box is freed! */
                         /*  do _not_ copy only the pointer to object */
    /* --------------------------------------------------------
     *  extension since v0.41 js05, Store frame vectors,
     *  which is a table of vectors sourrounding the char and its
     *  inner white holes. The advantage is the independence from
     *  resolution, handling of holes, overlap and rotation.
     * --------------------------------------------------------- */
    int num_frames;      /* number of frames: 1 for cfhklmnrstuvwxyz */
                         /*                   2 for abdegijopq */
    int frame_vol[MaxNumFrames]; /* volume inside frame +/- (black/white) */
    int frame_per[MaxNumFrames]; /* periphery, summed length of vectors */
    int num_frame_vectors[MaxNumFrames]; /* index to next frame */
                /* biggest frame should be stored first (outer frame) */
                /* biggest has the maximum pair distance */
                /* num vector loops */
    int frame_vector[MaxFrameVectors][2]; /* may be 16*int=fixpoint_number */
    
};
typedef struct box Box;

/* true if the coordination pair (a,b) is outside the image p */
#define outbounds(p, a, b)  (a < 0 || b < 0 || a >= (p)->x || b >= (p)->y)

/* ToDo: this structure seems to be obsolete, remove it */
typedef struct path {
  int start;	/* color at the beginning of the path, (0=white, 1=black) */
  int *x;	/* x coordinates of transitions */
  int *y;	/* y coordinates of transitions */
  int num;	/* current number of entries in x or y */
  int max;	/* maximum number of entries in x or y */
  /* (if more values need to be stored, the arrays are enlarged) */
} path_t;

/* job_t contains all information needed for an OCR task */
typedef struct job_s {
  struct {       /* source data */
    char *fname; /* input filename; default value: "-" */
    pix p;       /* source pixel data, pixelmap 8bit gray */
  } src;
  struct { /* temporary stuff, e.g. buffers */
#ifdef HAVE_GETTIMEOFDAY
    struct timeval init_time; /* starting time of this job */
#endif
    pix ppo; /* pixmap for visual debugging output, obsolete */

    /* sometimes recognition function is called again and again, if result was 0
       n_run tells the pixel function to return alternative results */
    int n_run;   /* num of run, if run_2 critical pattern get other results */
                 /* used for 2nd try, pixel uses slower filter function etc. */
    List dblist; /* list of boxes loaded from the character database */
  } tmp;
  struct {         /* results */
    List boxlist;  /* store every object in a box, which contains */
                   /* the characteristics of the object (see struct box) */
    List linelist; /* recognized text lines after recognition */
    
    struct tlines lines; /* used to access to line-data (statistics) */
                         /* here the positions (frames) of lines are */
                         /* stored for further use */
    int avX,avY;         /* average X,Y (avX=sumX/numC) */
    int sumX,sumY,numC;  /* sum of all X,Y; num chars */
  } res;
  struct {    /* configuration */
    int cs;   /* critical grey value (pixel<cs => black pixel) */
              /* range: 0..255,  0 means autodetection */
    int spc;  /* spacewidth/dots (0 = autodetect); default value: 0 */
    int mode; /* operation modes; default value: 0 */
              /* operation mode (see --help) */
    int dust_size;    /* dust size; default value: 10 */
    int only_numbers; /* numbers only; default value: 0 */
    int verbose; /* verbose mode; default value: 0 */ 
                 /* verbose option (see --help) */
    FORMAT out_format; /* output format; default value: ISO8859_1 */
    char *lc; /* debuglist of chars (_ = not recognized chars) */
              /* default value: "_" */
    char *db_path; /* pathname for database; default value: NULL */
    char *cfilter; /* char filter; default value: NULL, ex: "A-Za-z" */
        /* limit of certainty where chars are accepted as identified */
    int  certainty; /* in units of 100 (percent); 0..100; default 95 */
    char *unrec_marker; /* output this string for every unrecognized char */
  } cfg;
} job_t;

/* initialze job structure */
void job_init(job_t *job);        /* once for cfg and db */
void job_init_image(job_t *job);  /* for each of a multiimage */

/* free job structure */
void job_free_image(job_t *job); /* for each of a multiimage */

/*FIXME jb: remove JOB; */
extern job_t *JOB;

/* calculate the overlapp of the line (0-1) with black points 
 * by rekursiv bisection 
 * (evl. Fehlertoleranz mit pixel in Umgebung dx,dy suchen) (umschaltbar) ???
 * MidPoint Line Algorithm (Bresenham) Foley: ComputerGraphics better?
 * will be replaced by vector functions
 */

/* gerade y=dy/dx*x+b, implizit d=F(x,y)=dy*x-dx*y+b*dx=0 
 * incrementell y(i+1)=m*(x(i)+1)+b, F(x+1,y+1)=f(F(x,y))  */
int get_lineg(int x0, int y0, int x1, int y1, pix *p, int cs, int ret);
int get_line2(int x0, int y0, int x1, int y1, pix *p, int cs, int ret);

/* look for white 0x02 or black 0x01 dots (0x03 = white+black) */
char get_bw(int x0, int x1, int y0, int y1,
             pix *p, int cs,int mask);

/* look for black crossing a line x0,y0,x1,y1
 * follow line and count crossings ([white]-black-transitions)
 */
int num_cross(int x0, int x1, int y0, int y1,
               pix *p,  int cs);

/* memory allocation with error checking */
void *xrealloc(void *ptr, size_t size);

/* follow a line x0,y0,x1,y1 recording locations of transitions,
 * return count of transitions 
 */
int follow_path(int x0, int x1, int y0, int y1, pix *p,  int cs, path_t *path);

/* -------------------------------------------------------------
 * mark edge-points
 *  - first move forward until b/w-edge
 *  - more than 2 pixel?
 *  - loop around
 *    - if forward    pixel : go up, rotate right
 *    - if forward no pixel : rotate left
 *  - stop if found first 2 pixel in same order
 * mit an rechter-Wand-entlang-gehen strategie
 * --------------------------------------------------------------
 * turmite game: inp: start-x,y, regel r_black=UP,r_white=RIght until border
 *               out: last-position
 * Zaehle dabei, Schritte,Sackgassen,xmax,ymax,ro-,ru-,lo-,lu-Ecken
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * is this the right place for declaration?
 */
void turmite(pix *p, int *x, int *y, 
             int x0, int x1, int y0, int y1, int cs, int rw, int rb);

/* test if points are connected via t-pixel (rekursiv!) */
int joined(pix *p, int x0, int y0, int x1, int y1, int cs);

/* move from x,y to direction r until pixel or l steps
 * return number of steps
 */
int loop(pix *p, int x, int y, int l, int cs, int col, DIRECTION r);

#define MAX_HOLES 3
typedef struct list_holes { 
  int num;    /* numbers of holes, initialize with 0 */ 
  struct hole_s {
    int size,x,y,x0,y0,x1,y1; /*  size, start point, outer rectangle */
  } hole[MAX_HOLES];
} holes_t;
          
/* look for white holes surrounded by black points
 * at moment white point with black in all four directions
 */
int num_hole(int x0, int x1, int y0, int y1, pix *p, int cs, holes_t *holes);

/* count for black nonconnected objects --- used for i,auml,ouml,etc. */
int num_obj(int x0, int x1, int y0, int y1, pix  *p, int cs);

int distance(   pix *p1, struct box *box1,	/* box-frame */
		pix *p2, struct box *box2, int cs);

/* call the OCR engine ;) */
/* char whatletter(struct box *box1,int cs); */

/* declared in pixel.c */
/* getpixel() was pixel() but it may collide with netpnm pixel declaration */
int getpixel(pix *p, int x, int y);
int marked(pix *p, int  x, int  y);
void put(pix * p, int x, int y, int ia, int io);

/* } */ /* extern C */
#endif /* __GOCR_H__ */
