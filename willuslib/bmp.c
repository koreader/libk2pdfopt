/*
** BMP.C        For WILLUSLIB, set of routines to deal with 8-bit and 24-bit
**              bitmap structures, including functions that read and
**              write BMP files, PNG files, and JPEG files.
**
** Part of willus.com general purpose C code library.
**
** Copyright (C) 2012  http://willus.com
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
#include "willus.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>


#ifdef HAVE_PNG_LIB
#include <zlib.h>
#include <png.h>
#endif
#ifdef HAVE_JPEG_LIB
#include <jpeglib.h>
#endif
#ifdef HAVE_JASPER_LIB
#include <jasper.h>
#endif

#define BOUND(x,xmin,xmax)  if ((x)<(xmin)) (x)=(xmin); else { if ((x)>(xmax)) (x)=(xmax); }

#define RGBSET24(bmp,ptr,r,g,b) \
    if (bmp->type==WILLUSBITMAP_TYPE_NATIVE) \
        { \
        ptr[0]=r; \
        ptr[1]=g; \
        ptr[2]=b; \
        } \
    else \
        { \
        ptr[2]=r; \
        ptr[1]=g; \
        ptr[0]=b; \
        }

#define RGBGET(bmp,ptr,r,g,b) \
    if (bmp->bpp==8) \
        { \
        r=bmp->red[ptr[0]]; \
        g=bmp->green[ptr[0]]; \
        b=bmp->blue[ptr[0]]; \
        } \
    else if (bmp->type==WILLUSBITMAP_TYPE_NATIVE) \
        { \
        r=ptr[0]; \
        g=ptr[1]; \
        b=ptr[2]; \
        } \
    else \
        { \
        r=ptr[2]; \
        g=ptr[1]; \
        b=ptr[0]; \
        }

#define RGBGETINCPTR(bmp,ptr,r,g,b) \
    if (bmp->bpp==8) \
        { \
        r=bmp->red[ptr[0]]; \
        g=bmp->green[ptr[0]]; \
        b=bmp->blue[ptr[0]]; \
        ptr++; \
        } \
    else if (bmp->type==WILLUSBITMAP_TYPE_NATIVE) \
        { \
        r=ptr[0]; \
        g=ptr[1]; \
        b=ptr[2]; \
        ptr+=3; \
        } \
    else \
        { \
        r=ptr[2]; \
        g=ptr[1]; \
        b=ptr[0]; \
        ptr+=3; \
        }

static double willusbmp_dpi=150.;
static int    willusbmp_pageno=-1;


static char *cnames[]={"red","green","blue","magenta","cyan","yellow",
                           "grey","black","white",""};
static char *acodes[]={ANSI_RED,ANSI_GREEN,ANSI_BLUE,ANSI_MAGENTA,
                           ANSI_CYAN,ANSI_YELLOW,ANSI_NORMAL,ANSI_NORMAL,
                           ANSI_WHITE};

static double bmp_dpi=-1.;
#ifdef HAVE_JPEG_LIB
static int bmp_std_huffman_tables=0;

static void my_error_exit(j_common_ptr cinfo);
#endif
static int  bmp8_write(WILLUSBITMAP *bmap,char *filename,FILE *out);
static int  bmp24_write(WILLUSBITMAP *bmap,char *filename,FILE *out);
static void insert_int32lsbmsb(char *a,int x);
static int  retrieve_int32lsbmsb(char *a);
static void get_file_ext(char *fileext,char *filename);
static void bmp_resample_1(double *tempbmp,WILLUSBITMAP *src,double x1,double y1,
                           double x2,double y2,int newwidth,int newheight,
                           double *temprow,int color);
static void resample_1d(double *dst,double *src,double x1,double x2,
                        int n);
static double resample_single(double *y,double x1,double x2);
#ifdef HAVE_PNG_LIB
static void bmp_read_png_from_memory(png_structp png_ptr,void *buf,int nbytes);
#endif
static void new_rgb(int *dpc,int *spc,int *dbgc,int *dfgc,int *sbgc,int *sfgc);
static int jpeg_write_comments(FILE *out,char *buf);
static int jpeg_read2(FILE *f,int *x);
static int jpeg_write2(FILE *f,int x);
static void find_most_common_color(double ***hist,int n,int *r,int *g,int *b,
                                   double *percent);
static int bmp_uniform_row(WILLUSBITMAP *bmp,int row);
static int bmp_uniform_col(WILLUSBITMAP *bmp,int col);
static void bmp_color_xform8(WILLUSBITMAP *dest,WILLUSBITMAP *src,unsigned char *newval);
static void bmp_apply_filter_gray(WILLUSBITMAP *dest,WILLUSBITMAP *src,
                                  double **filter,int ncols,int nrows);
static double bmp_row_by_row_stdev(WILLUSBITMAP *bmp,int ccount,int whitethresh,
                                   double theta_radians);
static int pixval_dither(int pv,int n,int maxsrc,int maxdst,int x0,int y0);
static int dither_rec(int bits,int x0,int y0);


double bmp_last_read_dpi(void)

    {
    return(bmp_dpi);
    }


void bmp_set_pdf_dpi(double dpi)

    {
    willusbmp_dpi=dpi;
    }


double bmp_get_pdf_dpi(void)

    {
    return(willusbmp_dpi);
    }


void bmp_set_pdf_pageno(int pageno)

    {
    willusbmp_pageno=pageno;
    }


int bmp_get_pdf_pageno(void)

    {
    return(willusbmp_pageno);
    }

/*
** Quality is ignored if not JPEG.
*/
int bmp_write(WILLUSBITMAP *bmap,char *filename,FILE *out,int quality)

    {
    char    fileext[16];

    get_file_ext(fileext,filename);
    if (!stricmp(fileext,"ico"))
        return(bmp_write_ico(bmap,filename,out));
#ifdef HAVE_PNG_LIB
    if (!stricmp(fileext,"png"))
        return(bmp_write_png(bmap,filename,out));
#endif
    if (!stricmp(fileext,"pdf"))
        {
        PDFFILE _pdf,*pdf;
        pdf=&_pdf;
        if (pdffile_init(pdf,filename,1)!=NULL)
            {
            pdffile_add_bitmap(pdf,bmap,willusbmp_dpi,quality,0);
            pdffile_finish(pdf,NULL,NULL,NULL,NULL);
            pdffile_close(pdf);
            return(0);
            }
        else
            {
            fprintf(out,"pdffile_init(%s) failed.\n",filename);
            return(-10);
            }
        }
#ifdef HAVE_JPEG_LIB
    if (!stricmp(fileext,"jpg") || !stricmp(fileext,"jpeg"))
        {
        if (bmap->bpp!=24)
            {
            if (out!=NULL)
                fprintf(out,"Can only write JPEG output for 24-bit bitmaps.\n");
            return(-10);
            }
        return(bmp_write_jpeg(bmap,filename,quality,out));
        }
#endif
    if (stricmp(fileext,"bmp") && out!=NULL)
        fprintf(out,"Warning:  file %s has no extension.  Treating as BMP file.\n",
               filename);
    if (bmap->bpp==24)
        return(bmp24_write(bmap,filename,out));
    return(bmp8_write(bmap,filename,out));
    }


int bmp_write_ico(WILLUSBITMAP *bmp,char *filename,FILE *out)

    {
    FILE *f;
    char a[4];
    int i,j,nb;

    nb=(bmp->width+31)/8;
    while (nb&3)
        nb--;
    nb*=bmp->height;
    f=fopen(filename,"wb");
    a[0]=a[1]=a[2]=a[3]=0;
    fwrite(a,1,2,f);
    a[0]=1;
    fwrite(a,1,2,f);
    fwrite(a,1,2,f);
    a[0]=bmp->width;
    a[1]=bmp->height;
    a[2]=0; // True color
    a[3]=0;
    fwrite(a,4,1,f);
    a[0]=1;  // N planes
    a[1]=0;
    a[2]=24; // Bits-per-pixel
    a[3]=0;
    fwrite(a,4,1,f);
    i=40+bmp->width*bmp->height*3+nb;
    fwrite(&i,4,1,f);
    i=22;
    fwrite(&i,4,1,f);
    i=40;
    fwrite(&i,4,1,f);
    i=bmp->width;
    fwrite(&i,4,1,f);
    i=bmp->height*2;
    fwrite(&i,4,1,f);
    a[0]=1;
    a[1]=0;
    a[2]=24;
    a[3]=0;
    fwrite(a,4,1,f);
    i=0;
    fwrite(&i,4,1,f);
    fwrite(&i,4,1,f);
    i=2834; // Pixels per meter (72 dpi)
    fwrite(&i,4,1,f);
    fwrite(&i,4,1,f);
    i=0;
    fwrite(&i,4,1,f);
    fwrite(&i,4,1,f);
    for (i=bmp->height-1;i>=0;i--)
        {
        unsigned char *p;
        p=bmp_rowptr_from_top(bmp,i);
        for (j=0;j<bmp->width;j++,p+=bmp->bpp/8)
            {
            int r,g,b;
            RGBGET(bmp,p,r,g,b);
            a[0]=b;
            a[1]=g;
            a[2]=r;
            fwrite(a,3,1,f);
            }
        }
    a[0]=0;
    for (i=0;i<nb;i++)
        fwrite(a,1,1,f);
    fclose(f);
    return(0);
    }
    


/*
** If 8-bit, the bitmap is filled with <r>.
** If 24-bit, it gets <r>, <g>, <b> values.
*/
void bmp_fill(WILLUSBITMAP *bmp,int r,int g,int b)

    {
    int     y,n;

    if (bmp->bpp==8 || (r==g && r==b))
        {
        memset(bmp->data,r,bmp->size_allocated);
        return;
        }
    if (bmp->type==WILLUSBITMAP_TYPE_WIN32 && bmp->bpp==24)
        {
        y=r;
        r=b;
        b=y;
        }
    for (y=bmp->height-1;y>=0;y--)
        {
        unsigned char *p;

        p=bmp_rowptr_from_top(bmp,y);
        for (n=bmp->width-1;n>=0;n--)
            {
            (*p)=r;
            p++;
            (*p)=g;
            p++;
            (*p)=b;
            p++;
            }
        }
    }


/*
** Should be called right after bmp_init() to set the bitmap type.
** Type defaults to WILLUSBITMAP_TYPE_NATIVE if this is not called.
*/
void bmp_set_type(WILLUSBITMAP *bmap,int type)

    {
    bmap->type = type;
    }


int bmp_get_type(WILLUSBITMAP *bmp)

    {
    return(bmp->type);
    }


/*
** Write out to an 8-bit BMP file, OUT.BMP
*/
static int bmp8_write(WILLUSBITMAP *bmap,char *filename,FILE *out)



    {
    FILE *f;
    char    a[54];
    long    bytewidth,y,i,extra;
    long    height;

    height=bmap->height;
    bytewidth=bmp_bytewidth_win32(bmap);
    for (i=0;i<54;i++)
        a[i]=0;
    /* Fill in header */
    a[0]='B';
    a[1]='M';
    insert_int32lsbmsb(&a[2],bytewidth*bmap->height+54+256*4);
    insert_int32lsbmsb(&a[10],0x436);  /* Where the bitmap data starts */
    insert_int32lsbmsb(&a[14],0x28);   /* Size of BITMAPINFOHEADER */
    insert_int32lsbmsb(&a[18],bmap->width);
    insert_int32lsbmsb(&a[22],bmap->height);
    a[26]=1;
    a[28]=8;
    insert_int32lsbmsb(&a[34],bytewidth*bmap->height);
    insert_int32lsbmsb(&a[38],0x1274); /* pixels per meter */
    insert_int32lsbmsb(&a[42],0x1274);
    f=fopen(filename,"wb");
    if (f==NULL)
        {
        if (out!=NULL)
            fprintf(out,"Cannot open file OUT.BMP for writing.\n");
        return(-2);
        }
    fwrite(a,sizeof(char),54,f);
    for (i=0;i<256;i++)
        {
        fputc(bmap->blue[i],f);
        fputc(bmap->green[i],f);
        fputc(bmap->red[i],f);
        fputc(0,f);
        }
    if (bmap->type==WILLUSBITMAP_TYPE_WIN32)
        fwrite(bmap->data,sizeof(char),bytewidth*bmap->height,f);
    else
        {
        extra=bytewidth-bmap->width;
        a[0]=a[1]=a[2]=a[3]=0;
        for (y=0;y<height;y++)
            {
            fwrite(&bmap->data[(bmap->height-1-y)*bmap->width],sizeof(char),bmap->width,f);
            if (extra)
                fwrite(a,sizeof(char),extra,f);
            }
        }
    fclose(f);
    return(0);
    }


/*
** Write out to a 24-bit BMP file.
*/
static int bmp24_write(WILLUSBITMAP *bmap,char *filename,FILE *out)



    {
    FILE *f;
    char    a[54];
    int     n;
    long    bytewidth,y,i;

    n=0;
    bytewidth = bmp_bytewidth_win32(bmap);
    for (i=0;i<54;i++)
        a[i]=0;
    /* Fill in header */
    a[0]='B';
    a[1]='M';
    insert_int32lsbmsb(&a[2],bytewidth*bmap->height+54);
    insert_int32lsbmsb(&a[10],0x36);  /* Where the bitmap data starts */
    insert_int32lsbmsb(&a[14],0x28);  /* size of BITMAPINFOHEADER */
    insert_int32lsbmsb(&a[18],bmap->width);
    insert_int32lsbmsb(&a[22],bmap->height);
    a[26]=1;
    a[28]=24;
    insert_int32lsbmsb(&a[34],bytewidth*bmap->height);
    insert_int32lsbmsb(&a[38],0x1274); /* pixels per meter */
    insert_int32lsbmsb(&a[42],0x1274);
    f=fopen(filename,"wb");
    if (f==NULL)
        {
        if (out!=NULL)
            fprintf(out,"Cannot open file %s for writing.\n",filename);
        return(-2);
        }
    fwrite(a,sizeof(char),54,f);
    if (bmap->type == WILLUSBITMAP_TYPE_WIN32)
        fwrite(bmap->data,sizeof(char),bytewidth*bmap->height,f);
    else
        {
        a[0]=a[1]=a[2]=a[3]=0;
        n=bytewidth - (bmap->width*3);
        bmp24_flip_rgb(bmap);
        for (y=bmap->height-1;y>=0;y--)
            {
            fwrite(&bmap->data[y*bmap->width*3],sizeof(char),bmap->width*3,f);
            if (n)
                fwrite(a,sizeof(char),n,f);
            }
        bmp24_flip_rgb(bmap);
        }
    fclose(f);
    return(0);
    }


#ifdef HAVE_PNG_LIB
int bmp_png_info(char *filename,int *width,int *height,int *bpp,FILE *out)

    {
    FILE    *f;
    char    header[8];
    png_structp png_ptr;
    png_infop info_ptr,end_info;
    int     color_type,bppel;
    png_uint_32 wid,hght;

    f=fopen(filename,"rb");
    if (f==NULL)
        {
        if (out!=NULL)
            fprintf(out,"Cannot open file %s for PNG input.\n",filename);
        return(-1);
        }
    if (fread(header,1,8,f)<8 || png_sig_cmp((unsigned char *)header,0,8))
        {
        fclose(f);
        if (out!=NULL)
            fprintf(out,"File %s doesn't appear to be PNG.\n",filename);
        return(-2);
        }
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,NULL,NULL,NULL);
    if (png_ptr==NULL)
        {
        fclose(f);
        if (out!=NULL)
            fprintf(out,"Cannot create PNG structure.\n");
        return(-3);
        }
    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr==NULL)
        {
        fclose(f);
        png_destroy_read_struct(&png_ptr,NULL,NULL);
        if (out!=NULL)
            fprintf(out,"Cannot create PNG info structure.\n");
        return(-4);
        }
    end_info = png_create_info_struct(png_ptr);
    if (end_info==NULL)
        {
        fclose(f);
        png_destroy_read_struct(&png_ptr,&info_ptr,NULL);
        if (out!=NULL)
            fprintf(out,"Cannot create PNG end info structure.\n");
        return(-5);
        }
    /* Error handler */
    if (setjmp(png_jmpbuf(png_ptr)))
        {
        png_destroy_read_struct(&png_ptr,&info_ptr,&end_info);
        fclose(f);
        if (out!=NULL)
            fprintf(out,"PNG Read Error!\n");
        return(-6);
        }
    png_init_io(png_ptr,f);
    png_set_sig_bytes(png_ptr,8);
    png_read_info(png_ptr,info_ptr);
    png_get_IHDR(png_ptr,info_ptr,&wid,&hght,&bppel,
                  &color_type,NULL,NULL,NULL);
    if (width!=NULL)
        (*width)=(int)wid;
    if (height!=NULL)
        (*height)=(int)hght;
    if (color_type==PNG_COLOR_TYPE_GRAY)
        bppel=8;
    else if (color_type&PNG_COLOR_MASK_PALETTE)
        bppel=8;
    else
        bppel=24;
    if (bpp!=NULL)
        (*bpp)=bppel;
    png_destroy_read_struct(&png_ptr,&info_ptr,&end_info);
    fclose(f);
    return(0);
    }


int bmp_read_png(WILLUSBITMAP *bmp,char *filename,FILE *out)

    {
    FILE *f;
    int     status;

    f=fopen(filename,"rb");
    if (f==NULL)
        {
        nprintf(out,"Cannot open file %s for PNG input.\n",filename);
        return(-1);
        }
    status=bmp_read_png_stream(bmp,(void *)f,0,out);
    fclose(f);
    return(status);
    }

static unsigned char *pngdata;
static int pngindex;
static void bmp_read_png_from_memory(png_structp png_ptr,void *buf,int nbytes)

    {
    memcpy(buf,&pngdata[pngindex],nbytes);
    pngindex+=nbytes;
    }


int bmp_read_png_stream(WILLUSBITMAP *bmp,void *io,int size,FILE *out)

    {
    unsigned char header[8];
    png_structp png_ptr;
    png_infop info_ptr,end_info;
    int     color_type,gotpal,rowbytes;
    static png_colorp pngpal;
    unsigned char **rowptrs;
    double *dptr;
    int     i,num_palette;
    static char *funcname="bmp_read_png_stream";
    FILE *f;
    static char *notpng="File doesn't appear to be PNG.\n";

    f=NULL;
    rowptrs=NULL;
    if (size==0)
        {
        f=(FILE *)io;
        if (fread(header,1,8,f)<8 || png_sig_cmp(header,0,8))
            {
            nprintf(out,"%s",notpng);
            return(-2);
            }
        }
    else
        {
        pngindex=8;
        pngdata=(unsigned char *)io;
        if (png_sig_cmp(pngdata,0,8))
            {
            nprintf(out,"%s",notpng);
            return(-2);
            }
        }
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,NULL,NULL,NULL);
    if (png_ptr==NULL)
        {
        nprintf(out,"Cannot create PNG structure.\n");
        return(-3);
        }
    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr==NULL)
        {
        png_destroy_read_struct(&png_ptr,NULL,NULL);
        nprintf(out,"Cannot create PNG info structure.\n");
        return(-4);
        }
    end_info = png_create_info_struct(png_ptr);
    if (end_info==NULL)
        {
        png_destroy_read_struct(&png_ptr,&info_ptr,NULL);
        nprintf(out,"Cannot create PNG end info structure.\n");
        return(-5);
        }
    /* Error handler */
    if (setjmp(png_jmpbuf(png_ptr)))
        {
        dptr=(double *)rowptrs;
        willus_mem_free(&dptr,funcname);
        rowptrs=(unsigned char **)dptr;
        bmp_free(bmp);
        png_destroy_read_struct(&png_ptr,&info_ptr,&end_info);
        nprintf(out,"PNG Read Error!\n");
        return(-6);
        }
    if (size>0)
        png_set_read_fn(png_ptr,NULL,(png_rw_ptr)bmp_read_png_from_memory);
    else
        png_init_io(png_ptr,f);
    png_set_sig_bytes(png_ptr,8);
    png_read_info(png_ptr,info_ptr);
    {
    png_uint_32 ww,hh;
    png_get_IHDR(png_ptr,info_ptr,&ww,&hh,&bmp->bpp,
                  &color_type,NULL,NULL,NULL);
    bmp_dpi = (double)png_get_x_pixels_per_meter(png_ptr,info_ptr)*.0254;
    bmp->width=(int)ww;
    bmp->height=(int)hh;
    }
    /* Only allow up to 8-bits per channel */
    if (bmp->bpp==16)
        png_set_strip_16(png_ptr);
    gotpal=0;
    if (color_type==PNG_COLOR_TYPE_GRAY)
        {
        png_set_expand_gray_1_2_4_to_8(png_ptr);
        bmp->bpp = 8;
        for (i=0;i<256;i++)
            {
            bmp->red[i]=i;
            bmp->blue[i]=i;
            bmp->green[i]=i;
            }
        }
    else if (color_type&PNG_COLOR_MASK_PALETTE)
        {
        if (bmp->bpp<8)
            png_set_packing(png_ptr);
        bmp->bpp=8;
        if (png_get_valid(png_ptr,info_ptr,PNG_INFO_PLTE))
            {
            int     i;
            png_get_PLTE(png_ptr,info_ptr,&pngpal,&num_palette);
            for (i=0;i<num_palette;i++)
                {
                bmp->red[i]   = pngpal[i].red;
                bmp->green[i] = pngpal[i].green;
                bmp->blue[i]  = pngpal[i].blue;
                }
            gotpal=1;
            }
        }
    else
        {
        bmp->bpp=24;
        if (bmp->type == WILLUSBITMAP_TYPE_WIN32)
            png_set_bgr(png_ptr);
        }

    /* Strip alpha channel if there. */
    if (color_type&PNG_COLOR_MASK_ALPHA)
        png_set_strip_alpha(png_ptr);
    /* Allocate bitmap. */
    if (!bmp_alloc(bmp))
        {
        png_destroy_read_struct(&png_ptr,&info_ptr,&end_info);
        nprintf(out,"Cannot allocate memory for %d x %d x %d bitmap.\n",
                     bmp->width,bmp->height,bmp->bpp);
        return(-7);
        }
    if (!willus_mem_alloc(&dptr,bmp->height*sizeof(unsigned char *),funcname))
        {
        bmp_free(bmp);
        png_destroy_read_struct(&png_ptr,&info_ptr,&end_info);
        nprintf(out,"Cannot allocate memory (%d pointers) for bitmap.\n",
                     bmp->height);
        return(-8);
        }
    rowptrs=(unsigned char **)dptr;
    rowbytes = bmp_bytewidth(bmp);
    if (bmp->type==WILLUSBITMAP_TYPE_WIN32)
        for (i=0;i<bmp->height;i++)
            rowptrs[i] = &bmp->data[(bmp->height-1-i)*rowbytes];
    else
        for (i=0;i<bmp->height;i++)
            rowptrs[i] = &bmp->data[i*rowbytes];

    /* Okay, should be ready to read data. */
    png_read_image(png_ptr,(void *)rowptrs);
    dptr=(double *)rowptrs;
    willus_mem_free(&dptr,funcname);
    rowptrs=(unsigned char **)dptr;
    if ((color_type&PNG_COLOR_MASK_PALETTE) && !gotpal)
        {
        png_read_end(png_ptr,end_info);
        if (png_get_valid(png_ptr,end_info,PNG_INFO_PLTE))
            {
            int     i;
            png_get_PLTE(png_ptr,info_ptr,&pngpal,&num_palette);
            for (i=0;i<num_palette;i++)
                {
                bmp->red[i]   = pngpal[i].red;
                bmp->green[i] = pngpal[i].green;
                bmp->blue[i]  = pngpal[i].blue;
                }
            gotpal=1;
            }
        }
    png_destroy_read_struct(&png_ptr,&info_ptr,&end_info);
    return(0);
    }


int bmp_write_png(WILLUSBITMAP *bmp,char *filename,FILE *out)

    {
    FILE    *f;
    int     status;

    f=fopen(filename,"wb");
    if (f==NULL)
        {
        if (out!=NULL)
            fprintf(out,"Cannot open file %s for PNG output.\n",filename);
        return(-1);
        }
    status = bmp_write_png_stream(bmp,f,out);
    fclose(f);
    return(status);
    }


int bmp_write_png_stream(WILLUSBITMAP *bmp,FILE *f,FILE *out)

    {
    png_structp png_ptr;
    png_infop   info_ptr;
    unsigned char **rowptrs;
    double *dptr;
    int     i,rowbytes;
    static char *funcname="bmp_write_png_stream";

    rowptrs=NULL;
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,NULL,NULL,NULL);
    if (png_ptr==NULL)
        {
        if (out!=NULL)
            fprintf(out,"Could not allocate PNG structure.\n");
        return(-2);
        }
    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr==NULL)
        {
        if (out!=NULL)
            fprintf(out,"Could not allocate PNG info structure.\n");
        png_destroy_write_struct(&png_ptr,NULL);
        return(-3);
        }
    /* Error handler */
    if (setjmp(png_jmpbuf(png_ptr)))
        {
        dptr=(double *)rowptrs;
        willus_mem_free(&dptr,funcname);
        rowptrs=(unsigned char **)dptr;
        png_destroy_write_struct(&png_ptr,&info_ptr);
        if (out!=NULL)
            fprintf(out,"PNG Write Error!\n");
        return(-4);
        }
    png_init_io(png_ptr,f);
    png_set_compression_level(png_ptr,Z_BEST_COMPRESSION);
    png_set_IHDR(png_ptr,info_ptr,bmp->width,bmp->height,8,
                 bmp->bpp==24 ? PNG_COLOR_TYPE_RGB : PNG_COLOR_TYPE_PALETTE,
                 PNG_INTERLACE_NONE,PNG_COMPRESSION_TYPE_DEFAULT,
                 PNG_FILTER_TYPE_DEFAULT);
    if (bmp->bpp==8)
        {
        static png_color pngpal[256];
        int     i;

        for (i=0;i<256;i++)
            {
            pngpal[i].red   = bmp->red[i];
            pngpal[i].green = bmp->green[i];
            pngpal[i].blue  = bmp->blue[i];
            }
        png_set_PLTE(png_ptr,info_ptr,pngpal,256);
        }
    png_write_info(png_ptr,info_ptr);
    if (bmp->type==WILLUSBITMAP_TYPE_WIN32)
        png_set_bgr(png_ptr);
    if (!willus_mem_alloc(&dptr,bmp->height*sizeof(unsigned char *),funcname))
        {
        png_destroy_write_struct(&png_ptr,&info_ptr);
        if (out!=NULL)
            fprintf(out,"Cannot allocate memory (%d pointers) for bitmap.\n",
                     bmp->height);
        return(-5);
        }
    rowptrs=(unsigned char **)dptr;
    rowbytes = bmp_bytewidth(bmp);
    if (bmp->type==WILLUSBITMAP_TYPE_WIN32)
        for (i=0;i<bmp->height;i++)
            rowptrs[i] = &bmp->data[(bmp->height-1-i)*rowbytes];
    else
        for (i=0;i<bmp->height;i++)
            rowptrs[i] = &bmp->data[i*rowbytes];
    png_write_image(png_ptr,(void *)rowptrs);
    dptr=(double *)rowptrs;
    willus_mem_free(&dptr,funcname);
    rowptrs=NULL;
    png_write_end(png_ptr,info_ptr);
    png_destroy_write_struct(&png_ptr,&info_ptr);
    return(0);
    }
#endif /* HAVE_PNG_LIB */


#ifdef HAVE_JPEG_LIB
struct my_error_mgr
    {
    struct jpeg_error_mgr pub;    /* "public" fields */
    jmp_buf setjmp_buffer;        /* for return to caller */
    };
typedef struct my_error_mgr *my_error_ptr;

static void my_error_exit (j_common_ptr cinfo)
    {
    /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
    my_error_ptr myerr = (my_error_ptr) cinfo->err;

    /* Always display the message. */
    /* We could postpone this until after returning, if we chose. */
    (*cinfo->err->output_message) (cinfo);

    /* Return control to the setjmp point */
    longjmp(myerr->setjmp_buffer, 1);
    }


int bmp_write_jpeg(WILLUSBITMAP *bmp,char *filename,int quality,FILE *out)

    {
    FILE *f;
    int status;

    f=fopen(filename,"wb");
    if (f==NULL)
        {
        if (out!=NULL)
            fprintf(out,"Cannot open file %s for JPEG output.\n",filename);
        return(-1);
        }
    status=bmp_write_jpeg_stream(bmp,f,quality,out);
    fclose(f);
    return(status);
    }


/*
** If status==0, the bmp_write_jpeg_stream will write a JPEG file with
** optimized encoding (optimized huffman tables).  If status!=0, the
** JPEG file will be written with standard Huffman tables (JPEG standard
** section K.3)--see jcparam.c file in the jpeg library.
*/
void bmp_jpeg_set_std_huffman(int status)

    {
    bmp_std_huffman_tables=status;
    }


int bmp_write_jpeg_stream(WILLUSBITMAP *bmp,FILE *outfile,int quality,FILE *out)

    {
    struct jpeg_compress_struct cinfo;
    struct my_error_mgr jerr;
    JSAMPROW row_pointer[1];      /* pointer to JSAMPLE row[s] */
    int row_stride;               /* physical row width in image buffer */

    /* Error handler */
    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = my_error_exit;
    if (setjmp(jerr.setjmp_buffer))
        {
        jpeg_destroy_compress(&cinfo);
        return(-2);
        }

    /* Create the JPEG compression object. */
    jpeg_create_compress(&cinfo);

    jpeg_stdio_dest(&cinfo,outfile);

    cinfo.image_width      = bmp->width;
    cinfo.image_height     = bmp->height;
    cinfo.input_components = bmp->bpp==8 ? 1 : 3;
    cinfo.in_color_space   = bmp->bpp==8 ? JCS_GRAYSCALE : JCS_RGB;
    jpeg_set_defaults(&cinfo);
    /* See bmp_jpeg_set_std_huffman() */
    cinfo.optimize_coding  = bmp_std_huffman_tables ? 0 : 1;
    jpeg_set_quality(&cinfo,quality,TRUE);

    /* Do it! */
    jpeg_start_compress(&cinfo, TRUE);
    row_stride = bmp_bytewidth(bmp);
    if (bmp->type==WILLUSBITMAP_TYPE_WIN32)
        {
        if (bmp->bpp==24)
            bmp24_flip_rgb(bmp);
        while (cinfo.next_scanline < cinfo.image_height)
            {
            row_pointer[0] = &bmp->data[(cinfo.image_height-1-cinfo.next_scanline)*row_stride];
            jpeg_write_scanlines(&cinfo,row_pointer,1);
            }
        if (bmp->bpp==24)
            bmp24_flip_rgb(bmp);
        }
    else
        while (cinfo.next_scanline < cinfo.image_height)
            {
            row_pointer[0] = &bmp->data[cinfo.next_scanline*row_stride];
            jpeg_write_scanlines(&cinfo,row_pointer,1);
            }
    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);
    return(0);
    }


int bmp_jpeg_info(char *filename,int *width,int *height,int *bpp)

    {
    struct jpeg_decompress_struct cinfo;
    struct my_error_mgr jerr;
    FILE  *infile;

    infile=fopen(filename,"rb");
    if (infile==NULL)
        {
        fprintf(stderr,"Cannot open JPEG file %s for input.\n",filename);
        return(-1);
        }

    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = my_error_exit;
    if (setjmp(jerr.setjmp_buffer))
        {
        jpeg_destroy_decompress(&cinfo);
        fclose(infile);
        return(-2);
        }
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo,infile);
    cinfo.out_color_space = JCS_RGB;
    jpeg_read_header(&cinfo,TRUE);
    jpeg_start_decompress(&cinfo);
    if (width!=NULL)
        (*width)=cinfo.output_width;
    if (height!=NULL)
        (*height)=cinfo.output_height;
    if (bpp!=NULL)
        (*bpp) = (cinfo.out_color_space==JCS_GRAYSCALE) ? 8 : 24;
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);
    return(0);
    }


int bmp_read_jpeg(WILLUSBITMAP *bmp,char *filename,FILE *out)

    {
    FILE *infile;
    int     status;

    infile=fopen(filename,"rb");
    if (infile==NULL)
        {
        if (out!=NULL)
            fprintf(out,"Cannot open JPEG file %s for input.\n",filename);
        return(-1);
        }
    status=bmp_read_jpeg_stream(bmp,infile,0,out);
    fclose(infile);
    return(status);
    }

/*
** Reads JPEG from EITHER file stream or from memory buffer.
** If file stream (determined by if size==0),
**     infile must be FILE * (already open).
** If memory buffer, infile must be (unsigned char *) and size is
**     size of buffer.
*/
int bmp_read_jpeg_stream(WILLUSBITMAP *bmp,void *infile,int size,FILE *out)

    {
    struct jpeg_decompress_struct cinfo;
    struct my_error_mgr jerr;
    void  *p[1];
    int row_stride,i;

    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = my_error_exit;
    if (setjmp(jerr.setjmp_buffer))
        {
        jpeg_destroy_decompress(&cinfo);
        return(-2);
        }
    jpeg_create_decompress(&cinfo);
    if (size>0)
        jpeg_mem_src(&cinfo,(unsigned char *)infile,size);
    else
        jpeg_stdio_src(&cinfo,(FILE *)infile);
    cinfo.out_color_space = JCS_RGB;
    jpeg_read_header(&cinfo,TRUE);
    bmp_dpi = cinfo.density_unit==2 ? cinfo.X_density*2.54 : cinfo.X_density;
    jpeg_start_decompress(&cinfo);
    bmp->width=cinfo.output_width;
    bmp->height=cinfo.output_height;
    bmp->bpp = (cinfo.out_color_space==JCS_GRAYSCALE) ? 8 : 24;
    if (bmp->bpp==8)
        for (i=0;i<256;i++)
            {
            bmp->red[i]=i;
            bmp->green[i]=i;
            bmp->blue[i]=i;
            }
    if (!bmp_alloc(bmp))
        {
        jpeg_destroy_decompress(&cinfo);
        return(-3);
        }
    row_stride = bmp_bytewidth(bmp);
    /*
    row_stride = cinfo.output_width*cinfo.output_components;
    */
    if (bmp->type==WILLUSBITMAP_TYPE_WIN32)
        {
        while (cinfo.output_scanline < cinfo.output_height)
            {
            p[0] = (void *)&bmp->data[(cinfo.output_height-1-cinfo.output_scanline)*row_stride];
            jpeg_read_scanlines(&cinfo,(JSAMPARRAY)p,1);
            }
        if (bmp->bpp==24)
            bmp24_flip_rgb(bmp);
        }
    else
        while (cinfo.output_scanline < cinfo.output_height)
            {
            p[0] = (void *)&bmp->data[cinfo.output_scanline*row_stride];
            jpeg_read_scanlines(&cinfo,(JSAMPARRAY)p,1);
            }
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    return(0);
    }
#endif /* HAVE_JPEG_LIB */


static void insert_int32lsbmsb(char *a,int x)

    {
    a[3] = (x >> 24) & 0xff;
    a[2] = (x >> 16) & 0xff;
    a[1] = (x >>  8) & 0xff;
    a[0] = x & 0xff;
    }


static int retrieve_int32lsbmsb(char *a)

    {
    int     a3,a2,a1,a0,x;

    a3=a[3];
    a2=a[2];
    a1=a[1];
    a0=a[0];
    x = ((a3&0xff)<<24) | ((a2&0xff)<<16) | ((a1&0xff)<<8) | (a0&0xff);
    return(x);
    }


void bmp24_mixbmps(WILLUSBITMAP *dest,WILLUSBITMAP *src1,WILLUSBITMAP *src2,int level)

    {
    int     i,n;

    if (dest->bpp!=24 || src1->bpp!=24 || src2->bpp!=24)
        return;
    n=bmp_bytewidth(dest) * dest->height;
    for (i=0;i<n;i++)
        {
        int     d,s1,s2;

        s1=src1->data[i];
        s2=src2->data[i];
        d=(s2*level+(256-level)*s1)>>8;
        dest->data[i]=d;
        }
    }


void bmp8_palette_info(WILLUSBITMAP *bmap,FILE *out)

    {
    long    i;
    int     counts[256];
    long    pixcount;

    pixcount=bmap->width*bmap->height;
    for (i=0;i<256;i++)
        counts[i]=0;
    for (i=0;i<pixcount;i++)
        counts[bmap->data[i]]++;
    for (i=0;i<256;i++)
        fprintf(out,"Index %3ld (%3d,%3d,%3d):  %6d\n",
            i,bmap->red[i],bmap->blue[i],bmap->green[i],counts[i]);
    }


/*
** Put palette values directly into data values for 8-bit grey-level bitmap.
*/
void bmp8_to_grey(WILLUSBITMAP *bmap)

    {
    long    pixcount;
    long    i;

    pixcount=bmp_bytewidth(bmap)*bmap->height;
    for (i=pixcount-1;i>=0;i--)
        {
        int     di,r,g,b;

        di=bmap->data[i];
        r=bmap->red[di];
        g=bmap->green[di];
        b=bmap->blue[di];
        bmap->data[i] = bmp8_greylevel_convert(r,g,b);
        }
    }


void bmp8_to_bw(WILLUSBITMAP *bmap,int thresh)

    {
    long    pixcount;
    int     r,g,b;
    long    i;

    pixcount=bmp_bytewidth(bmap)*bmap->height;
    for (i=pixcount-1;i>=0;i--)
        {
        int     di;
        di=bmap->data[i];
        r=bmap->red[di];
        g=bmap->green[di];
        b=bmap->blue[di];
        bmap->data[i] = (bmp8_greylevel_convert(r,g,b) >= thresh ? 0 : 1);
        }
    }


int bmp8_greylevel_convert(int r,int g,int b)

    {
    return((int)((r*0.3+g*0.59+b*0.11)*1.002));
    }


/*
** Should call bmp_set_type() right after this to set the bitmap type.
*/
void bmp_init(WILLUSBITMAP *bmap)

    {
    bmap->data=NULL;
    bmap->size_allocated=0;
    bmap->type=WILLUSBITMAP_TYPE_NATIVE;
    }


/*
** The width, height, and bpp parameters of the WILLUSBITMAP structure
** should be set before calling this function.
*/
int bmp_alloc(WILLUSBITMAP *bmap)

    {
    int     size;
    static char *funcname="bmp_alloc";

    if (bmap->bpp!=8 && bmap->bpp!=24)
        {
        printf("Internal error:  call to bmp_alloc has bpp!=8 and bpp!=24!\n");
        exit(10);
        }
    /* Choose the max size even if not WIN32 to avoid memory faults */
    /* and to allow the possibility of changing the "type" of the   */
    /* bitmap without reallocating memory.                          */
    size = bmp_bytewidth_win32(bmap)*bmap->height;
    if (bmap->data!=NULL && bmap->size_allocated>=size)
        return(1);
    if (bmap->data!=NULL)
        willus_mem_realloc_robust_warn((void **)&bmap->data,size,bmap->size_allocated,funcname,10);
    else
        willus_mem_alloc_warn((void **)&bmap->data,size,funcname,10);
    bmap->size_allocated=size;
    return(1);
    }


int bmp_bytewidth(WILLUSBITMAP *bmp)

    {
    if (bmp->type == WILLUSBITMAP_TYPE_WIN32)
        return(bmp_bytewidth_win32(bmp));
    return(bmp->bpp==24 ? bmp->width*3 : bmp->width);
    }


/*
** row==0             ==> top row of bitmap
** row==bmp->height-1 ==> bottom row of bitmap
** (regardless of bitmap type)
*/
unsigned char *bmp_rowptr_from_top(WILLUSBITMAP *bmp,int row)

    {
    if (bmp->type==WILLUSBITMAP_TYPE_WIN32)
        return(&bmp->data[bmp_bytewidth(bmp)*(bmp->height-1-row)]);
    else
        return(&bmp->data[bmp_bytewidth(bmp)*row]);
    }


/*
** y0 is row FROM TOP!  0=top row, bmp->height-1=bottom row.
*/
void bmp_crop(WILLUSBITMAP *bmp,int x0,int y0_from_top,int width,int height)

    {
    int     x1,y1,y0,i,dbw,sbw;
    unsigned char *psrc,*pdest;

    y0 = y0_from_top;
    y1 = y0+height-1;
    x1 = x0+width-1;
    if (x1 > bmp->width-1)
        x1 = bmp->width-1;
    if (y1 > bmp->height-1)
        y1 = bmp->height-1;
    if (x0<0)
        x0 = 0;
    if (y0<0)
        y0 = 0;
    if (x0==0 && y0==0 && x1==bmp->width-1 && y1==bmp->height-1)
        return;
    sbw  = bmp_bytewidth(bmp);
    psrc = bmp_rowptr_from_top(bmp,bmp->type==WILLUSBITMAP_TYPE_WIN32?y1:y0)
            + ((bmp->bpp+7)>>3)*x0;
    bmp->width=width;
    bmp->height=height;
    dbw   = bmp_bytewidth(bmp);
    pdest = bmp->data;
    for (i=height;i>0;i--,psrc+=sbw,pdest+=dbw)
        memmove(pdest,psrc,dbw);
    }


void bmp_rotate_fast(WILLUSBITMAP *bmp,double degrees,int expand)

    {
    WILLUSBITMAP _dst,*dst;
    double th,sth,cth;
    int i,r,g,b,w,h,row,col;

    dst=&_dst;
    th=degrees*PI/180.;
    sth=sin(th);
    cth=cos(th);
    if (expand)
        {
        w=(int)(fabs(bmp->width*cth)+fabs(bmp->height*sth)+.5);
        h=(int)(fabs(bmp->height*cth)+fabs(bmp->width*sth)+.5);
        }
    else
        {
        w=bmp->width;
        h=bmp->height;
        }
    dst=&_dst;
    bmp_init(dst);
    dst->width=w;
    dst->height=h;
    dst->bpp=bmp->bpp;
    if (dst->bpp==8)
        for (i=0;i<=255;i++)
            dst->red[i] = dst->green[i] = dst->blue[i]=i;
    bmp_alloc(dst);
    bmp_pix_vali(bmp,0,0,&r,&g,&b);
    bmp_fill(dst,r,g,b);
    if (dst->bpp==8)
        for (row=0;row<dst->height;row++)
            {
            unsigned char *p;
            double x1,y1,x2,y2;

            y2=dst->height/2.-row;
            p=bmp_rowptr_from_top(dst,row);  
            for (x2=-dst->width/2.,col=0;col<dst->width;col++,p++,x2+=1.0)
                {
                double g;
                x1 = -.5 + bmp->width/2. + x2*cth + y2*sth;
                y1 = -.5 + bmp->height/2. + y2*cth - x2*sth;
                if (x1<0. || x1>=bmp->width || y1<0. || y1>=bmp->height)
                    continue;
                g=bmp_grey_pix_vald(bmp,x1,y1);
                if (g>=0.)
                    p[0]=g;
                }
            }
    else
        for (row=0;row<dst->height;row++)
            {
            unsigned char *p;
            double x1,y1,x2,y2;

            y2=dst->height/2.-row;
            p=bmp_rowptr_from_top(dst,row);  
            for (x2=-dst->width/2.,col=0;col<dst->width;col++,p+=3,x2+=1.0)
                {
                double rr,gg,bb;
                x1 = -.5 + bmp->width/2. + x2*cth + y2*sth;
                y1 = -.5 + bmp->height/2. + y2*cth - x2*sth;
                if (x1<0. || x1>=bmp->width || y1<0. || y1>=bmp->height)
                    continue;
                bmp_pix_vald(bmp,x1,y1,&rr,&gg,&bb);
                if (rr<0.)
                    continue;
                p[0]=rr;
                p[1]=gg;
                p[2]=bb;
                }
            }
    bmp_copy(bmp,dst);
    bmp_free(dst);
    }


     
/*
** 1 = okay, 0 = fail
*/
int bmp_rotate_right_angle(WILLUSBITMAP *bmp,int degrees)

    {
    int d;

    d=degrees%360;
    if (d<0)
        d+=360;
    d=(d+45)/90;
    if (d==1)
        return(bmp_rotate_90(bmp));
    if (d==2)
        {
        bmp_flip_horizontal(bmp);
        bmp_flip_vertical(bmp);
        return(1);
        }
    if (d==3)
        return(bmp_rotate_270(bmp));
    return(1);
    }


int bmp_rotate_90(WILLUSBITMAP *bmp)

    {
    WILLUSBITMAP   *sbmp,_sbmp;
    int     bpp,dbw,sr;

    sbmp=&_sbmp;
    bmp_init(sbmp);
    if (!bmp_copy(sbmp,bmp))
        return(0);
    bmp->width = sbmp->height;
    bmp->height = sbmp->width;
    bpp = bmp->bpp/8;
    if (!bmp_alloc(bmp))
        {
        bmp_free(sbmp);
        return(0);
        }
    dbw = (int)(bmp_rowptr_from_top(bmp,1) - bmp_rowptr_from_top(bmp,0));
    for (sr=0;sr<sbmp->height;sr++)
        {
        unsigned char *sp,*dp;
        int     j,sc;

        sp = bmp_rowptr_from_top(sbmp,sr);
        dp = bmp_rowptr_from_top(bmp,bmp->height-1) + bpp*sr;
        for (sc=sbmp->width;sc>0;sc--,dp-=dbw)
            for (j=0;j<bpp;j++,sp++)
                dp[j] = sp[0];
        }
    bmp_free(sbmp);
    return(1);
    }


int bmp_rotate_270(WILLUSBITMAP *bmp)

    {
    WILLUSBITMAP   *sbmp,_sbmp;
    int     bpp,dbw,sr;

    sbmp=&_sbmp;
    bmp_init(sbmp);
    if (!bmp_copy(sbmp,bmp))
        return(0);
    bmp->width = sbmp->height;
    bmp->height = sbmp->width;
    bpp = bmp->bpp/8;
    if (!bmp_alloc(bmp))
        {
        bmp_free(sbmp);
        return(0);
        }
    dbw = (int)(bmp_rowptr_from_top(bmp,1) - bmp_rowptr_from_top(bmp,0));
    for (sr=0;sr<sbmp->height;sr++)
        {
        unsigned char *sp,*dp;
        int     j,sc;

        sp = bmp_rowptr_from_top(sbmp,sr);
        dp = bmp_rowptr_from_top(bmp,0) + bpp*(sbmp->height-1-sr);
        for (sc=sbmp->width;sc>0;sc--,dp+=dbw)
            for (j=0;j<bpp;j++,sp++)
                dp[j] = sp[0];
        }
    bmp_free(sbmp);
    return(1);
    }


int bmp_copy(WILLUSBITMAP *dest,WILLUSBITMAP *src)

    {
    dest->width  = src->width;
    dest->height = src->height;
    dest->bpp    = src->bpp;
    dest->type   = src->type;
    if (!bmp_alloc(dest))
        return(0);
    memcpy(dest->data,src->data,src->height*bmp_bytewidth(src));
    memcpy(dest->red,src->red,sizeof(int)*256);
    memcpy(dest->green,src->green,sizeof(int)*256);
    memcpy(dest->blue,src->blue,sizeof(int)*256);
    return(1);
    }


/*
** I figured this out empirically on a sample of four bitmaps.
** It reports approximately the number of bytes per pixel that a JPEG
** file will be based on the quality.
*/
double bmp_jpeg_bytes_per_pixel(int quality)

   {
   double x,x0,sum;
   static double coeffs[9]={0.003269,0.4074,-3.465,23.66,-68.83,82.94,
                            -16.82,-40.34,22.78};
   int i;
   x0=quality/100.;
   for (x=1,i=0,sum=0.;i<9;sum+=x*coeffs[i],i++,x*=x0);
   return(sum/.75);
   }


void bmp_flip_horizontal(WILLUSBITMAP *bmp)

    {
    int     i,j,bpp;

    bpp = bmp->bpp/8;
    for (i=0;i<bmp->height;i++)
        {
        unsigned char *p,*p2;

        p=bmp_rowptr_from_top(bmp,i);
        p2=&p[(bmp->width-1)*bpp];
        for (;p<p2;p+=bpp,p2-=bpp)
            for (j=0;j<bpp;j++)
                {
                unsigned char t;
                t=p[j];
                p[j]=p2[j];
                p2[j]=t;
                }
        }
    }


void bmp_flip_vertical(WILLUSBITMAP *bmp)

    {
    int     i,bw,n;

    bw = bmp_bytewidth(bmp);
    n=bmp->height/2;
    for (i=0;i<n;i++)
        {
        unsigned char *p,*p2;
        int     j;

        p=bmp_rowptr_from_top(bmp,i);
        p2=bmp_rowptr_from_top(bmp,bmp->height-i-1);
        for (j=bw;j>0;j--,p++,p2++)
            {
            unsigned char t;
            t=p[0];
            p[0]=p2[0];
            p2[0]=t;
            }
        }
    }


int bmp_bytewidth_win32(WILLUSBITMAP *bmp)

    {
    return(((bmp->bpp==24 ? bmp->width*3 : bmp->width)+3)&(~0x3));
    }


void bmp_free(WILLUSBITMAP *bmap)

    {
    if (bmap->data!=NULL)
        {
        willus_mem_free((double **)&bmap->data,"bmp_free");
        bmap->data=NULL;
        bmap->size_allocated=0;
        }
    }


int bmp_read(WILLUSBITMAP *bmap,char *filename,FILE *out)

    {
    FILE   *f;
    int     bpp;
    char    fileext[16];

    get_file_ext(fileext,filename);
#ifdef HAVE_GHOSTSCRIPT
    if (!stricmp(fileext,"ps") || !stricmp(fileext,"eps") || !stricmp(fileext,"pdf"))
        return(willusgs_read_pdf_or_ps_bmp(bmap,filename,willusbmp_pageno,willusbmp_dpi,out));
#endif
#ifdef HAVE_PNG_LIB
    if (!stricmp(fileext,"png"))
        return(bmp_read_png(bmap,filename,out));
#endif
#ifdef HAVE_JPEG_LIB
    if (!stricmp(fileext,"jpg") || !stricmp(fileext,"jpeg"))
        return(bmp_read_jpeg(bmap,filename,out));
#endif
    if (stricmp(fileext,"bmp"))
#ifdef HAVE_JASPER_LIB
        {
        int fmt;
        fmt=bmp_jasper_read(NULL,filename,NULL);
        if (fmt<0)
#endif
        fprintf(out,"Warning:  file %s has no extension.  Treating as BMP file.\n",filename);
#ifdef HAVE_JASPER_LIB
        else
            return(bmp_jasper_read(bmap,filename,out));
        }
#endif
    f=fopen(filename,"rb");
    if (f==NULL)
        {
        if (out!=NULL)
            fprintf(out,"Cannot open BMP file %s for input.\n",filename);
        return(-1);
        }
    fseek(f,28L,0);
    bpp=fgetc(f);
    if (bpp!=8 && bpp!=24)
        {
        if (out!=NULL)
            fprintf(out,"BMP file %s is not 8-bit or 24-bit.\n",filename);
        fclose(f);
        return(-9);
        }
    fclose(f);
    return(bpp==24 ? bmp_read_bmp24(bmap,filename,out)
                   : bmp_read_bmp8(bmap,filename,out));
    }


int bmp_info(char *filename,int *width,int *height,int *bpp,FILE *out)

    {
    char    fileext[16];

    get_file_ext(fileext,filename);
#ifdef HAVE_PNG_LIB
    if (!stricmp(fileext,"png"))
        return(bmp_png_info(filename,width,height,bpp,out));
#endif
#ifdef HAVE_JPEG_LIB
    if (!stricmp(fileext,"jpg") || !stricmp(fileext,"jpeg"))
        return(bmp_jpeg_info(filename,width,height,bpp));
#endif
    if (stricmp(fileext,"bmp") && out!=NULL)
        fprintf(out,"Warning:  file %s has no extension.  Treating as BMP file.\n",
               filename);
    return(bmp_bmp_info(filename,width,height,bpp,out));
    }


static void get_file_ext(char *dest,char *src)

    {
    int     i;

    for (i=strlen(src)-1;i>=0 && src[i]!='.';i--);
    strncpy(dest,&src[i+1],15);
    }


int bmp_bmp_info(char *filename,int *width,int *height,int *bpp,FILE *out)

    {
    FILE   *f;
    char    a[20];
    long    filelen;


    f=fopen(filename,"rb");
    if (f==NULL)
        {
        if (out!=NULL)
            fprintf(out,"Can't open file %s for input.\n",filename);
        return(-1);
        }
    fseek(f,0L,2);
    filelen=ftell(f);
    if (filelen<54)
        {
        fclose(f);
        if (out!=NULL)
            fprintf(out,"Input file %s is too small to be a BMP file.\n",filename);
        return(-2);
        }
    fseek(f,18L,0);
    if (fread(a,sizeof(char),20,f)<20)
        {
        fclose(f);
        if (out!=NULL)
            fprintf(out,"Error reading BMP file %s.\n",filename);
        return(-3);
        }
    fclose(f);
    if (width!=NULL)
        (*width)=retrieve_int32lsbmsb(a);
    if (height!=NULL)
        (*height)=retrieve_int32lsbmsb(&a[4]);
    if (bpp!=NULL)
        (*bpp) = a[10];
    return(0);
    }


int bmp_read_bmp8(WILLUSBITMAP *bmap,char *filename,FILE *out)

    {
    FILE   *f;
    char    a[20];
    int     i,k,pixwidth,pixheight,bytewidth;
    long    filelen,bytesize;
    static char palette[1024];


    f=fopen(filename,"rb");
    if (f==NULL)
        {
        if (out!=NULL)
            fprintf(out,"Can't open file %s for input.\n",filename);
        return(-1);
        }
    fseek(f,0L,2);
    filelen=ftell(f);
    if (filelen<1078)
        {
        fclose(f);
        if (out!=NULL)
            fprintf(out,"Input file %s is too small to be an 8-bit BMP file.\n",filename);
        return(-2);
        }
    fseek(f,18L,0);
    if (fread(a,sizeof(char),20,f)<20)
        {
        fclose(f);
        if (out!=NULL)
            fprintf(out,"Error reading BMP file %s.\n",filename);
        return(-3);
        }
    bmap->width=pixwidth=retrieve_int32lsbmsb(a);
    bmap->height=pixheight=retrieve_int32lsbmsb(&a[4]);
    if (a[10]!=8)
        {
        fclose(f);
        if (out!=NULL)
            fprintf(out,"BMP file %s is not 8-bit.\n",filename);
        return(-9);
        }
    bmap->bpp=8;
    if (out!=NULL)
        fprintf(out,"Image %s is %d x %d pixels.\n",filename,pixwidth,pixheight);
    bytewidth=bmp_bytewidth_win32(bmap);
    bytesize=bytewidth*pixheight;
    if (filelen < bytesize+1078)
        {
        if (out!=NULL)
            fprintf(out,"File %s is too short.  Perhaps it is not a BMP file.\n",filename);
        fclose(f);
        return(-4);
        }
    if (!bmp_alloc(bmap))
        {
        fclose(f);
        if (out!=NULL)
            fprintf(out,"Cannot allocate memory for bitmap.\n");
        return(-5);
        }

    /* Get DPI */
    {
    int dpm1,dpm2,dpm;
    fseek(f,38L,0);
    dpm1=fgetc(f);
    dpm2=fgetc(f);
    dpm=dpm1|(dpm2<<8);
    bmp_dpi = dpm*.0254;
    }
    /* Read palette */
    fseek(f,54L,0);
    if (fread(palette,sizeof(char),1024,f)<1024)
        {
        if (out!=NULL)
            fprintf(out,"Cannot read color palette from file %s.\n",filename);
        fclose(f);
        return(-6);
        }
    for (i=0;i<256;i++)
        {
        bmap->blue[i]=(unsigned char)palette[(i<<2)+0];
        bmap->green[i]=(unsigned char)palette[(i<<2)+1];
        bmap->red[i]=(unsigned char)palette[(i<<2)+2];
        }

    /* Read bitmap data */
    fseek(f,1078L,0);
    if (bmap->type == WILLUSBITMAP_TYPE_WIN32)
        {
        int     n;

        n=fread(bmap->data,sizeof(char),bytewidth*bmap->height,f);
        if (n<bytewidth*bmap->height)
            {
            if (out!=NULL)
                fprintf(out,"Premature EOF reading BMP8 file %s.\n",filename);
            bmp_free(bmap);
            fclose(f);
            return(-7);
            }
        }
    else
        for (k=0;k<pixheight;k++)
            {
            int     n;

            fseek(f,1078L+bytewidth*k,0);
            n=fread(&bmap->data[bmap->width*(bmap->height-1-k)],sizeof(char),bmap->width,f);
            if (n<bmap->width)
                {
                if (out!=NULL)
                    fprintf(out,"Premature EOF reading BMP8 file %s.\n",filename);
                bmp_free(bmap);
                fclose(f);
                return(-7);
                }
            }
    fclose(f);
    return(0);
    }


int bmp_read_bmp24(WILLUSBITMAP *bmap,char *filename,FILE *out)

    {
    FILE   *f;
    char    a[20];
    int     k,pixwidth,pixheight,extra;
    long    filelen,bytewidth,bw32,totalbytes;


    f=fopen(filename,"rb");
    if (f==NULL)
        {
        if (out!=NULL)
            fprintf(out,"Can't open file %s for input.\n",filename);
        return(-1);
        }
    fseek(f,0L,2);
    filelen=ftell(f);
    if (filelen<54)
        {
        fclose(f);
        if (out!=NULL)
            fprintf(out,"Input file %s is too small to be an 24-bit BMP file.\n",filename);
        return(-2);
        }
    fseek(f,18L,0);
    if (fread(a,sizeof(char),20,f)<20)
        {
        fclose(f);
        if (out!=NULL)
            fprintf(out,"Error reading BMP file %s.\n",filename);
        return(-3);
        }
    if (a[10]!=24)
        {
        fclose(f);
        if (out!=NULL)
            fprintf(out,"BMP file %s is not 24-bit.\n",filename);
        return(-9);
        }
    bmap->bpp=24;
    bmap->width=pixwidth=retrieve_int32lsbmsb(a);
    bmap->height=pixheight=retrieve_int32lsbmsb(&a[4]);
    if (out!=NULL)
        fprintf(out,"Image %s is %d x %d pixels.\n",filename,pixwidth,pixheight);
    bytewidth=pixwidth*3;
    bw32 = bmp_bytewidth_win32(bmap);
    extra = bw32 - bytewidth;
    totalbytes = bw32*pixheight;
    if (filelen < totalbytes+54)
        {
        if (out!=NULL)
            fprintf(out,"File %s is too short.  Perhaps it is not a BMP file.\n",filename);
        fclose(f);
        return(-4);
        }
    if (!bmp_alloc(bmap))
        {
        fclose(f);
        if (out!=NULL)
            fprintf(out,"Cannot allocate memory for bitmap.\n");
        return(-5);
        }
    {
    int dpm1,dpm2,dpm;
    fseek(f,38L,0);
    dpm1=fgetc(f);
    dpm2=fgetc(f);
    dpm=dpm1|(dpm2<<8);
    bmp_dpi = dpm*.0254;
    }
    /* Read bitmap data */
    fseek(f,54L,0);
    if (bmap->type == WILLUSBITMAP_TYPE_WIN32)
        {
        int n;

        n=fread(bmap->data,sizeof(char),totalbytes,f);
        if (n<totalbytes)
            {
            if (out!=NULL)
                fprintf(out,"Premature EOF reading BMP file %s.\n",filename);
            bmp_free(bmap);
            fclose(f);
            return(-7);
            }
        }
    else
        {
        for (k=0;k<bmap->height;k++)
            {
            int     n;
            char   *p;

            p=(char *)&bmap->data[bytewidth*(bmap->height-1-k)];
            n=fread(p,sizeof(char),bytewidth,f);
            if (n<bytewidth)
                {
                if (out!=NULL)
                    fprintf(out,"Premature EOF reading BMP file %s.\n",filename);
                bmp_free(bmap);
                fclose(f);
                return(-7);
                }
            if (extra)
                fseek(f,extra,1);
            }
        bmp24_flip_rgb(bmap);
        }
    fclose(f);
    return(0);
    }


void bmp24_reduce_size(WILLUSBITMAP *bmp,int mx,int my)

    {
    int     i,j,nw,nh,bw,nbw,c;

    if (bmp->bpp!=24)
        return;
    c = mx*my;
    if (c<=0)
        return;
    nw = bmp->width/mx;
    nh = bmp->height/my;
    bw=bmp_bytewidth(bmp);
    nbw = bmp->type==WILLUSBITMAP_TYPE_WIN32 ? (nw*3+3)&(~0x3) : nw*3;
    for (j=0;j<nh;j++)
        {
        int     iy;

        iy = j*my;
        for (i=0;i<nw;i++)
            {
            int     sum0,sum1,sum2,dx,dy,ix;
            unsigned char *p;

            ix = i*mx;
            sum0=sum1=sum2=0;
            for (dx=0;dx<mx;dx++)
                for (dy=0;dy<my;dy++)
                    {
                    p = &bmp->data[(iy+dy)*bw + (ix+dx)*3];
                    sum0 += p[0];
                    sum1 += p[1];
                    sum2 += p[2];
                    }
            p = &bmp->data[j*nbw + i*3];
            p[0] = (sum0+c/2)/c;
            p[1] = (sum1+c/2)/c;
            p[2] = (sum2+c/2)/c;
            }
        }
    bmp->width = nw;
    bmp->height = nh;
    }


void bmp24_flip_rgb(WILLUSBITMAP *bmp)

    {
    int     n,bw,i;
    unsigned char *p;
    unsigned char t;

    if (bmp->bpp!=24)
        return;
    bw=bmp_bytewidth(bmp);
    for (i=0;i<bmp->height;i++)
        for (p=&bmp->data[i*bw],n=bmp->width;n>0;n--,p+=3)
            {
            t=p[0];
            p[0]=p[2];
            p[2]=t;
            }
    }


/*
** Returns -1 if enough bytes allocated to promote 8-bit to 24-bit
** (and does the promotion)
**
** Returns 0 if not enough bytes or if bitmap is not 8-bits.
**
*/
int bmp_promote_to_24(WILLUSBITMAP *bmp)

    {
    int     oldbpr,newbpr,rownum,colnum;
    /* static char *funcname="bmp_promote_to_24"; */

    if (bmp->bpp!=8)
        return(0);
    oldbpr = bmp_bytewidth(bmp);
    bmp->bpp=24;
    newbpr = bmp_bytewidth(bmp);
    if (!bmp_alloc(bmp))
        {
        bmp->bpp=8;
        return(0);
        }
    for (rownum = bmp->height-1;rownum>=0;rownum--)
        {
        unsigned char *oldp,*newp;
        oldp = &bmp->data[oldbpr*rownum];
        newp = &bmp->data[newbpr*rownum + bmp->width*3-1];
        for (colnum = bmp->width-1;colnum>=0;colnum--)
            {
            (*newp) = bmp->red[oldp[colnum]];
            newp--;
            (*newp) = bmp->green[oldp[colnum]];
            newp--;
            (*newp) = bmp->blue[oldp[colnum]];
            newp--;
            }
        }
    if (bmp->type != WILLUSBITMAP_TYPE_WIN32)
        bmp24_flip_rgb(bmp);
    return(-1);
    }


void bmp_convert_to_greyscale(WILLUSBITMAP *bmp)

    {
    bmp_convert_to_greyscale_ex(bmp,bmp);
    }


/*
** Convert bitmap to grey-scale in-situ
*/
void bmp_convert_to_greyscale_ex(WILLUSBITMAP *dst,WILLUSBITMAP *src)

    {
    int oldbpr,newbpr,bpp,dp,rownum,colnum,i;

    oldbpr=bmp_bytewidth(src);
    dp = src->bpp==8 ? 1 : 3;
    bpp=src->bpp;
    dst->bpp=8; 
    for (i=0;i<256;i++)
        dst->red[i]=dst->green[i]=dst->blue[i]=i;
    if (dst!=src)
        {
        dst->width=src->width;
        dst->height=src->height;
        bmp_alloc(dst);
        }
    newbpr=bmp_bytewidth(dst);
    /* Possibly restore src->bpp to 24 so RGBGET works right (src & dst may be the same) */
    src->bpp=bpp; 
    for (rownum=0;rownum<src->height;rownum++)
        {
        unsigned char *oldp,*newp;
        oldp = &src->data[oldbpr*rownum];
        newp = &dst->data[newbpr*rownum];
        for (colnum=0;colnum<src->width;colnum++,oldp+=dp,newp++)
            {
            int r,g,b;
            RGBGET(src,oldp,r,g,b);
            (*newp)=bmp8_greylevel_convert(r,g,b);
            }
        }
    dst->bpp=8; /* Possibly restore dst->bpp to 8 since src & dst may be the same. */
    }


/*
** Return pix value (0.0 - 255.0) in double precision given
** a double precision position.  Bitmap is assumed to be 8-bit greyscale.
**
** x0,y0 are from bottom corner.
** x0=0.5, y0=0.5 would give exactly the value of the pixel
**                in the lower left corner of the bitmap.
*/
double bmp_grey_pix_vald(WILLUSBITMAP *bmp,double x0,double y0)

    {
    int     ix0,iy0,ix1,iy1;
    double  fx0,fx1,fy0,fy1;

    ix0 = (int)(x0-.5);
    ix1 = ix0+1;
    iy0 = (int)(y0-.5);
    iy1 = iy0+1;
    BOUND(ix0,0,bmp->width-1);
    BOUND(ix1,0,bmp->width-1);
    BOUND(iy0,0,bmp->height-1);
    BOUND(iy1,0,bmp->height-1);
    fx0 = 1.-fabs(ix0+0.5-x0);
    if (fx0<0.)
        fx0=0.;
    fx1 = 1.-fabs(ix1+0.5-x0);
    if (fx1<0.)
        fx1=0.;
    fy0 = 1.-fabs(iy0+0.5-y0);
    if (fy0<0.)
        fy0=0.;
    fy1 = 1.-fabs(iy1+0.5-y0);
    if (fy1<0.)
        fy1=0.;
    if ((fx0==0. && fx1==0.) || (fy0==0. && fy1==0.))
        return(-1.);
    return( (   fy0 * (   fx0*bmp_grey_pix_vali(bmp,ix0,iy0)
                        + fx1*bmp_grey_pix_vali(bmp,ix1,iy0) )
              + fy1 * (   fx0*bmp_grey_pix_vali(bmp,ix0,iy1)
                        + fx1*bmp_grey_pix_vali(bmp,ix1,iy1) ) )
              / ( (fx0+fx1) * (fy0+fy1) ) );
    }


/*
** Return pix values (0.0 - 255.0) in double precision given
** a double precision position.
**
** x0,y0 are from BOTTOM CORNER.
** x0=0.5, y0=0.5 would give exactly the value of the pixel
**                in the lower left corner of the bitmap.
*/
void bmp_pix_vald(WILLUSBITMAP *bmp,double x0,double y0,
                  double *r,double *g,double *b)

    {
    int     ix0,iy0,ix1,iy1;
    double  fx0,fx1,fy0,fy1;
    int     r00,r10,r01,r11;
    int     g00,g10,g01,g11;
    int     b00,b10,b01,b11;

    ix0 = (int)(x0-.5);
    ix1 = ix0+1;
    iy0 = (int)(y0-.5);
    iy1 = iy0+1;
    BOUND(ix0,0,bmp->width-1);
    BOUND(ix1,0,bmp->width-1);
    BOUND(iy0,0,bmp->height-1);
    BOUND(iy1,0,bmp->height-1);
    fx0 = 1.-fabs(ix0+0.5-x0);
    if (fx0<0.)
        fx0=0.;
    fx1 = 1.-fabs(ix1+0.5-x0);
    if (fx1<0.)
        fx1=0.;
    fy0 = 1.-fabs(iy0+0.5-y0);
    if (fy0<0.)
        fy0=0.;
    fy1 = 1.-fabs(iy1+0.5-y0);
    if (fy1<0.)
        fy1=0.;
    if ((fx0==0. && fx1==0.) || (fy0==0. && fy1==0.))
        {
        (*r) = (*g) = (*b) = -1.;
        return;
        }
    bmp_pix_vali(bmp,ix0,iy0,&r00,&g00,&b00);
    bmp_pix_vali(bmp,ix1,iy0,&r10,&g10,&b10);
    bmp_pix_vali(bmp,ix0,iy1,&r01,&g01,&b01);
    bmp_pix_vali(bmp,ix1,iy1,&r11,&g11,&b11);
    (*r)=((fy0*(fx0*r00+fx1*r10)+fy1*(fx0*r01+fx1*r11))/((fx0+fx1)*(fy0+fy1)));
    (*g)=((fy0*(fx0*g00+fx1*g10)+fy1*(fx0*g01+fx1*g11))/((fx0+fx1)*(fy0+fy1)));
    (*b)=((fy0*(fx0*b00+fx1*b10)+fy1*(fx0*b01+fx1*b11))/((fx0+fx1)*(fy0+fy1)));
    }


/*
** y0 = 0 ==> bottom row!
*/
int bmp_grey_pix_vali(WILLUSBITMAP *bmp,int x0,int y0)

    {
    unsigned char *p;
    int r,g,b;

    p = bmp_rowptr_from_top(bmp,bmp->height-1-y0);
    p = &p[x0*(bmp->bpp>>3)];
    RGBGET(bmp,p,r,g,b);
    return(bmp8_greylevel_convert(r,g,b));
    }


/*
** y0 = 0 ==> bottom row!
*/
void bmp_pix_vali(WILLUSBITMAP *bmp,int x0,int y0,int *r,int *g,int *b)

    {
    unsigned char *p;
    int rr,gg,bb;

    p = bmp_rowptr_from_top(bmp,bmp->height-1-y0);
    p = &p[x0*(bmp->bpp>>3)];
    RGBGET(bmp,p,rr,gg,bb);
    (*r)=rr;
    (*g)=gg;
    (*b)=bb;
    }


void bmp_grey_pixel_setd(WILLUSBITMAP *bmp,double x0,double y0,int grey)

    {
    bmp_rgb_pixel_setd(bmp,x0,y0,grey,grey,grey);
    }


void bmp_rgb_pixel_setd(WILLUSBITMAP *bmp,double x0,double y0,int r,int g,int b)

    {
    int     ix0,iy0,ix1,iy1;
    double  fx0,fx1,fy0,fy1;

    ix0 = (int)(x0-.5);
    ix1 = ix0+1;
    iy0 = (int)(y0-.5);
    iy1 = iy0+1;
    BOUND(ix0,0,bmp->width-1);
    BOUND(ix1,0,bmp->width-1);
    BOUND(iy0,0,bmp->height-1);
    BOUND(iy1,0,bmp->height-1);
    fx0 = 1.-fabs(ix0+0.5-x0);
    if (fx0<0.)
        fx0=0.;
    fx1 = 1.-fabs(ix1+0.5-x0);
    if (fx1<0.)
        fx1=0.;
    fy0 = 1.-fabs(iy0+0.5-y0);
    if (fy0<0.)
        fy0=0.;
    fy1 = 1.-fabs(iy1+0.5-y0);
    if (fy1<0.)
        fy1=0.;
    if ((fx0==0. && fx1==0.) || (fy0==0. && fy1==0.))
        return;
    bmp_rgb_pixel_setf(bmp,ix0,iy0,r,g,b,fx0*fy0);
    bmp_rgb_pixel_setf(bmp,ix1,iy0,r,g,b,fx1*fy0);
    bmp_rgb_pixel_setf(bmp,ix0,iy1,r,g,b,fx0*fy1);
    bmp_rgb_pixel_setf(bmp,ix1,iy1,r,g,b,fx1*fy1);
    }


void bmp_grey_pixel_setf(WILLUSBITMAP *bmp,int x,int y,int grey,double f)

    {
    bmp_rgb_pixel_setf(bmp,x,y,grey,grey,grey,f);
    }


void bmp_rgb_pixel_setf(WILLUSBITMAP *bmp,int x,int y,int r,int g,int b,double f)

    {
    unsigned char *p;
    int      pv;

    p = bmp_rowptr_from_top(bmp,bmp->height-1-y);
    p = &p[x*(bmp->bpp>>3)];
    if (bmp->bpp==24)
        {
        int     ir,ib;

        ir = bmp->type==WILLUSBITMAP_TYPE_WIN32 ? 2 : 0;
        ib = 2-ir;
        pv = f*r + (1.-f)*p[ir];
        BOUND(pv,0,255)
        p[ir] = pv;
        pv = f*g + (1.-f)*p[1];
        BOUND(pv,0,255)
        p[1] = pv;
        pv = f*b + (1.-f)*p[ib];
        BOUND(pv,0,255)
        p[ib] = pv;
        }
    else
        {
        pv = f*((r+g+b)/3) + (1.-f)*(*p);
        BOUND(pv,0,255)
        (*p)=pv;
        }
    }


void bmp_resize(WILLUSBITMAP *bmp,double scalefactor)

    {
    WILLUSBITMAP *copy,_copy;

    copy=&_copy;
    bmp_init(copy);
    bmp_copy(copy,bmp);
    bmp->width *= scalefactor;
    bmp->height *= scalefactor;
    bmp_resample(bmp,copy,0.,0.,(double)copy->width,(double)copy->height,
                             bmp->width,bmp->height);
    bmp_free(copy);
    }


/*
** Fast all-integer resample.
**
** Resample bitmap to be an integer size smaller, e.g.
** every n x n pixels in the source bitmap map to one pixel in the
** destination bitmap.
**
** This can be done with bmp_resample(), but this function is faster.
**
** The destination bitmap will be 8-bit grayscale if the source bitmap
** passes the bmp_is_grayscale() function.  Otherwise it will be 24-bit.
**
*/
void bmp_integer_resample(WILLUSBITMAP *dest,WILLUSBITMAP *src,int n)

    {
    int gray,np,n2,colorplanes,sbw;
    int color,row,col;

    dest->width = (src->width+(n-1))/n;
    dest->height = (src->height+(n-1))/n;
    if ((gray=bmp_is_grayscale(src))!=0)
        {
        int i;
        dest->bpp=8;
        for (i=0;i<256;i++)
            dest->red[i]=dest->blue[i]=dest->green[i]=i;
        }
    else
        dest->bpp=24;
    dest->type=WILLUSBITMAP_TYPE_NATIVE;
    bmp_alloc(dest);
    np=n*n;
    n2=np/2;
    colorplanes = gray ? 1 : 3;
    sbw=bmp_bytewidth(src);
    for (color=0;color<colorplanes;color++)
        {
        int drow;

        for (drow=0;drow<dest->height;drow++)
            {
            unsigned char *dp;
            unsigned char *sp1;
            int r1,r2,dr,dcol;

            r1=drow*n;
            r2=r1+n;
            if (r2>src->height)
                r2=src->height;
            sp1=bmp_rowptr_from_top(src,r1)+color;
            dp=bmp_rowptr_from_top(dest,drow)+color;
            dr=r2-r1;
            for (dcol=0;dcol<dest->width;dcol++,dp+=colorplanes)
                {
                int pixsum,c1,c2,c1x,c2x,dc;
                unsigned char *sp;

                c1=dcol*n;
                c2=c1+n;
                if (c2>src->width)
                    c2=src->width;
                dc=c2-c1;
                np=dc*dr;
                n2=np/2;
                c1x=c1*colorplanes;
                c2x=c2*colorplanes;
                for (pixsum=n2,row=r1,sp=sp1;row<r2;row++,sp+=sbw)
                    for (col=c1x;col<c2x;col+=colorplanes)
                        pixsum += sp[col];
                pixsum /= np;
                (*dp)=pixsum;
                }
            }
        }
    }


/*
** Resample (re-size) bitmap.  The pixel positions left to right go from
** 0.0 to src->width (x-coord), and top to bottom go from
** 0.0 to src->height (y-coord).
** The cropped rectangle (x1,y1) to (x2,y2) is placed into
** the destination bitmap, which need not be allocated yet.
**
** The destination bitmap will be 8-bit grayscale if the source bitmap
** passes the bmp_is_grayscale() function.  Otherwise it will be 24-bit.
**
** Returns 0 for okay.
**         -1 for not enough memory.
**         -2 for bad cropping area or destination bitmap size
*/
int bmp_resample(WILLUSBITMAP *dest,WILLUSBITMAP *src,double x1,double y1,
                 double x2,double y2,int newwidth,int newheight)

    {
    int gray,maxlen,colorplanes;
    double t;
    double *tempbmp;
    double *temprow;
    int color,hmax,row,col,dy;
    static char *funcname="bmp_resample";

    /* Clip and sort x1,y1 and x2,y2 */
    if (x1>src->width)
        x1=src->width;
    else if (x1<0.)
        x1=0.;
    if (x2>src->width)
        x2=src->width;
    else if (x2<0.)
        x2=0.;
    if (y1>src->height)
        y1=src->height;
    else if (y1<0.)
        y1=0.;
    if (y2>src->height)
        y2=src->height;
    else if (y2<0.)
        y2=0.;
    if (x2<x1)
        {
        t=x2;
        x2=x1;
        x1=t;
        }
    if (y2<y1)
        {
        t=y2;
        y2=y1;
        y1=t;
        }
    dy=y2-y1;
    dy += 2;
    if (x2-x1==0. || y2-y1==0.)
        return(-2);

    /* Allocate temp storage */
    maxlen = x2-x1 > dy+newheight ? (int)(x2-x1) : dy+newheight;
    maxlen += 16;
    hmax = newheight > dy ? newheight : dy;
    if (!willus_mem_alloc(&temprow,maxlen*sizeof(double),funcname))
        return(-1);
    if (!willus_mem_alloc(&tempbmp,hmax*newwidth*sizeof(double),funcname))
        {
        willus_mem_free(&temprow,funcname);
        return(-1);
        }
    if ((gray=bmp_is_grayscale(src))!=0)
        {
        int i;
        dest->bpp=8;
        for (i=0;i<256;i++)
            dest->red[i]=dest->blue[i]=dest->green[i]=i;
        }
    else
        dest->bpp=24;
    dest->width=newwidth;
    dest->height=newheight;
    dest->type=WILLUSBITMAP_TYPE_NATIVE;
    if (!bmp_alloc(dest))
        {
        willus_mem_free(&tempbmp,funcname);
        willus_mem_free(&temprow,funcname);
        return(-1);
        }
    colorplanes = gray ? 1 : 3;
    for (color=0;color<colorplanes;color++)
        {
        bmp_resample_1(tempbmp,src,x1,y1,x2,y2,newwidth,newheight,
                       temprow,gray ? -1 : color);
        for (row=0;row<newheight;row++)
            {
            unsigned char *p;
            double *s;
            p=bmp_rowptr_from_top(dest,row)+color;
            s=&tempbmp[row*newwidth];
            if (colorplanes==1)
                for (col=0;col<newwidth;p[0]=(int)(s[0]+.5),col++,s++,p++);
            else
                for (col=0;col<newwidth;p[0]=(int)(s[0]+.5),col++,s++,p+=colorplanes);
            }
        }
    willus_mem_free(&tempbmp,funcname);
    willus_mem_free(&temprow,funcname);
    return(0);
    }


static void bmp_resample_1(double *tempbmp,WILLUSBITMAP *src,double x1,double y1,
                           double x2,double y2,int newwidth,int newheight,
                           double *temprow,int color)

    {
    int row,col,x0,dx,y0,dy;

    x0=floor(x1);
    dx=ceil(x2)-x0;
    x1-=x0;
    x2-=x0;
    y0=floor(y1);
    dy=ceil(y2)-y0;
    y1-=y0;
    y2-=y0;
    if (src->type==WILLUSBITMAP_TYPE_WIN32 && color>=0)
        color=2-color;
    for (row=0;row<dy;row++)
        {
        unsigned char *p;
        p=bmp_rowptr_from_top(src,row+y0);
        if (src->bpp==8)
            {
            switch (color)
                {
                case -1:
                    for (col=0,p+=x0;col<dx;col++,p++)
                        temprow[col]=p[0];
                    break;
                case 0:
                    for (col=0,p+=x0;col<dx;col++,p++)
                        temprow[col]=src->red[p[0]];
                    break;
                case 1:
                    for (col=0,p+=x0;col<dx;col++,p++)
                        temprow[col]=src->green[p[0]];
                    break;
                case 2:
                    for (col=0,p+=x0;col<dx;col++,p++)
                        temprow[col]=src->blue[p[0]];
                    break;
                }
            }
        else
            {
            p+=color;
            for (col=0,p+=3*x0;col<dx;temprow[col]=p[0],col++,p+=3);
            }
        resample_1d(&tempbmp[row*newwidth],temprow,x1,x2,newwidth);
        }
    for (col=0;col<newwidth;col++)
        {
        double *p,*s;
        p=&tempbmp[col];
        s=&temprow[dy];
        for (row=0;row<dy;row++,p+=newwidth)
            temprow[row]=p[0];
        resample_1d(s,temprow,y1,y2,newheight);
        p=&tempbmp[col];
        for (row=0;row<newheight;row++,p+=newwidth,s++)
            p[0]=s[0];
        }
    }


/*
** Resample src[] into dst[].
** Examples:  resample_1d(dst,src,0.,5.,5) would simply copy the
**            first five elements of src[] to dst[].
**
**            resample_1d(dst,src,0.,5.,10) would work as follows:
**                dst[0] and dst[1] would get src[0].
**                dst[2] and dst[3] would get src[1].
**                and so on.
**
*/
static void resample_1d(double *dst,double *src,double x1,double x2,
                        int n)

    {
    int i;
    double new,last;

    last=x1;
    for (i=0;i<n;i++)
        {
        new=x1+(x2-x1)*(i+1)/n;
        dst[i] = resample_single(src,last,new);
        last=new;
        }
    }


static double resample_single(double *y,double x1,double x2)

    {
    int i,i1,i2;
    double dx,dx1,dx2,sum;

    i1=floor(x1);
    i2=floor(x2);
    if (i1==i2)
        return(y[i1]);
    dx=x2-x1;
    if (dx>1.)
        dx=1.;
    dx1= 1.-(x1-i1);
    dx2= x2-i2;
    sum=0.;
    if (dx1 > 1e-8*dx)
        sum += dx1*y[i1];
    if (dx2 > 1e-8*dx)
        sum += dx2*y[i2];
    for (i=i1+1;i<=i2-1;sum+=y[i],i++);
    return(sum/(x2-x1));
    }


/*
** Crop all edge pixels of bitmap that match the color of the upper
** left corner pixel
*/
void bmp_crop_edge(WILLUSBITMAP *bmp)

    {
    unsigned char *p0,*p;
    int i,j,pbw,imin,imax,jmin,jmax;

    p0=bmp_rowptr_from_top(bmp,0);
    pbw=bmp->bpp>>3;
    imin=bmp->height+1;
    imax=-1;
    jmax=-1;
    jmin=bmp->width+1;
    for (i=0;i<bmp->height;i++)
        {
        p=bmp_rowptr_from_top(bmp,i);
        for (j=0;j<bmp->width;j++,p+=pbw)
            if (memcmp(p,p0,pbw))
                {
                if (i<imin)
                    imin=i;
                if (i>imax)
                    imax=i;
                if (j<jmin)
                    jmin=j;
                if (j>jmax)
                    jmax=j;
                }
        }
    if (imax>=0)
        bmp_crop(bmp,imin,jmin,(imax-imin+1),(jmax-jmin+1));
    }
        

/*
** dest bitmap MUST BE 24-bit
*/     
void bmp_overlay(WILLUSBITMAP *dest,WILLUSBITMAP *src,int x0,int y0_from_top,
                 int *dbgc,int *dfgc,int *sbgc,int *sfgc)

    {
    unsigned char *dp,*sp;
    int i,j,sbw;

// printf("@bmp_overlay, x0=%d, y0_from_top=%d\n",x0,y0_from_top);
    sbw=src->bpp>>3;
    if (dest->bpp!=24)
        {
        printf("bmp_overlay:  destination bitmap must be 24-bit!\n");
        return;
        }
    for (i=0;i<src->height;i++)
        {
        sp=bmp_rowptr_from_top(src,i);
        if (i+y0_from_top < 0 || i+y0_from_top >= dest->height)
            continue;
        dp=bmp_rowptr_from_top(dest,i+y0_from_top)+3*x0;
        for (j=0;j<src->width;j++,dp+=3,sp+=sbw)
            {
            int spc[3],dpc[3];
            if (j+x0<0 || j+x0>=dest->width)
                continue;
            RGBGET(src,sp,spc[0],spc[1],spc[2]);
            RGBGET(dest,dp,dpc[0],dpc[1],dpc[2]);
            new_rgb(dpc,spc,dbgc,dfgc,sbgc,sfgc);
            RGBSET24(dest,dp,dpc[0],dpc[1],dpc[2]);
            }
        }
    }


void bmp_invert(WILLUSBITMAP *bmp)

    {
    int i;

    if (bmp->bpp==24 || bmp_is_grayscale(bmp))
        {
        unsigned char *p;
        int nb;
        p=bmp_rowptr_from_top(bmp,0);
        nb=bmp_bytewidth(bmp)*bmp->height;
        for (i=0;i<nb;i++,p++)
            (*p) = 255-(*p);
        }
    else
        for (i=0;i<256;i++)
            {
            bmp->red[i]=255-bmp->red[i];
            bmp->green[i]=255-bmp->green[i];
            bmp->blue[i]=255-bmp->blue[i];
            }
    }
             


/*
** Figure out what color to make a destination pixel based on these
** inputs:
** dpc[0..2]=r,g,b original destination pixel color.
** spc[0..2]=r,g,b source pixel color.
** dbgc[0..2]=r,g,b destination bitmap "background" color.
** dfgc[0..2]=r,g,b destination bitmap "foreground" color.
** sbgc[0..2]=r,g,b source background color.
** sfgc[0..2]=r,g,b source foreground color.
**
** Examples:
**     new_rgb(dpc,spc,NULL,NULL,NULL,NULL)
**         This will simply copy the source pixel color the destination.
**
**     new_rgb(dpc,spc,dbgc,dfgc,sbgc,sfgc)
**         This will determine how far the source pixel color is from
**         its background color (relative to the foreground) and make
**         the destination pixel an equivalent distance between its
**         background and foreground color.
**
**     new_rgb(dpc,spc,NULL,dfgc,sbgc,NULL)
**         This is like an OR.  Every source pixel that does not exactly
**         match the source background color overwrites the destination
**         with dfgc (or if dfgc==NULL, spc).
*/
static void new_rgb(int *dpc,int *spc,int *dbgc,int *dfgc,int *sbgc,int *sfgc)

    {
    double f;
    int bgc,i;

    if (sbgc==NULL && sfgc==NULL)
        {
        dpc[0]=spc[0];
        dpc[1]=spc[1];
        dpc[2]=spc[2];
        return;
        }
    if (sbgc!=NULL && sfgc!=NULL)
        {
        f=((double)(spc[0]-sbgc[0])/(sfgc[0]-sbgc[0])
         +(double)(spc[1]-sbgc[1])/(sfgc[1]-sbgc[1])
         +(double)(spc[2]-sbgc[2])/(sfgc[2]-sbgc[2]))/3.;
        if (dbgc!=NULL && dfgc!=NULL)
            {
            for (i=0;i<3;i++)
                {
                dpc[i]=dbgc[i]+f*(dfgc[i]-dbgc[i]);
                BOUND(dpc[i],0,255);
                }
            return;
            }
        if (dfgc==NULL)
            {
            for (i=0;i<3;i++)
                {
                dpc[i]*=f;
                BOUND(dpc[i],0,255);
                }
            return;
            }
        for (i=0;i<3;i++)
            {
            dpc[i]=dfgc[i]*f;
            BOUND(dpc[i],0,255);
            }
        return;
        }
    if (sbgc!=NULL && sfgc==NULL)
        bgc=(spc[0]==sbgc[0] && spc[1]==sbgc[1] && spc[2]==sbgc[2]);
    else
        bgc=(spc[0]!=sfgc[0] || spc[1]!=sfgc[1] || spc[2]!=sfgc[2]);
    if (bgc)
        {
        if (dbgc!=NULL)
            for (i=0;i<3;i++)
                dpc[i]=dbgc[i];
        return;
        }
    if (dfgc==NULL)
        for (i=0;i<3;i++)
            dpc[i]=spc[i];
    else
        for (i=0;i<3;i++)
            dpc[i]=dfgc[i];
    }
    

/*
** One of dest or src can be NULL, which is the
** same as setting them equal to each other, but
** in this case, the bitmap must be 24-bit!
** Note: contrast > 1 will increase the contrast.
**       contrast < 1 will decrease the contrast.
**       contrast of 0 will make all pixels the same value.
**       contrast of 1 will not change the image.
*/
void bmp_contrast_adjust(WILLUSBITMAP *dest,WILLUSBITMAP *src,double contrast)

    {
    int i;
    static unsigned char newval[256];

    for (i=0;i<256;i++)
        {
        double x,y;
        int sgn,v;
        x=(i-127.5)/127.5;
        sgn = x<0 ? -1 : 1;
        if (contrast<0)
            sgn = -sgn;
        x=fabs(x);
        if (fabs(contrast)>1.5)
            y=x<.99999 ? 1-exp(fabs(contrast)*x/(x-1)) : 1.;
        else
            {
            y=fabs(contrast)*x;
            if (y>1.)
                y=1.;
            }
        y = 127.5+y*sgn*127.5;
        v = (int)(y+.5);
        if (v<0)
            v=0;
        if (v>255)
            v=255;
        newval[i] = v;
        }
    bmp_color_xform(dest,src,newval);
    }


/*
** One of dest or src can be NULL, which is the
** same as setting them equal to each other, but
** in this case, the bitmap must be 24-bit!
**
** Note:  Used ...pow(i/256.,gc)... before 9-7-11.
*/
void bmp_gamma_correct(WILLUSBITMAP *dest,WILLUSBITMAP *src,double gamma)

    {
    double gc;
    int i;
    static unsigned char newval[256];

    if (gamma<0.001)
        gamma=0.001;
    gc=1./gamma;
    for (i=0;i<256;i++)
        newval[i] = 255.*pow(i/255.,gc)+.5;
    bmp_color_xform(dest,src,newval);
    }


/*
** One of dest or src can be NULL, which is the
** same as setting them equal to each other, but
** in this case, the bitmap must be 24-bit!
*/
void bmp_color_xform(WILLUSBITMAP *dest,WILLUSBITMAP *src,unsigned char *newval)

    {
    int ir,ic;

    if (src==NULL)
        src=dest;
    if (dest==NULL)
        dest=src;
    if (bmp_is_grayscale(src))
        {
        bmp_color_xform8(dest,src,newval);
        return;
        }
    if (dest!=src)
        {
        dest->width = src->width;
        dest->height = src->height;
        dest->bpp = 24;
        bmp_alloc(dest);
        }
    for (ir=0;ir<src->height;ir++)
        {
        unsigned char *sp,*dp;
        sp=bmp_rowptr_from_top(src,ir);
        dp=bmp_rowptr_from_top(dest,ir);
        for (ic=0;ic<src->width;ic++,dp+=3)
            {
            int r,g,b;

            RGBGETINCPTR(src,sp,r,g,b);
            r=newval[r];
            g=newval[g];
            b=newval[b];
            RGBSET24(dest,dp,r,g,b);
            }
        }
    }



/*
** One of dest or src can be NULL, which is the
** same as setting them equal to each other, but
** in this case, the bitmap must be 24-bit!
*/
int bmp_is_grayscale(WILLUSBITMAP *bmp)

    {
    int i;
    if (bmp->bpp!=8)
        return(0);
    for (i=0;i<256;i++)
        if (bmp->red[i]!=i || bmp->green[i]!=i || bmp->blue[i]!=i)
            return(0);
    return(1);
    }


static void bmp_color_xform8(WILLUSBITMAP *dest,WILLUSBITMAP *src,unsigned char *newval)

    {
    int i,ir;

    if (src==NULL)
        src=dest;
    if (dest==NULL)
        dest=src;
    if (dest!=src)
        {
        dest->width = src->width;
        dest->height = src->height;
        dest->bpp = 8;
        for (i=0;i<256;i++)
            dest->red[i]=dest->green[i]=dest->blue[i]=i;
        bmp_alloc(dest);
        }
    for (ir=0;ir<src->height;ir++)
        {
        unsigned char *sp,*dp;
        sp=bmp_rowptr_from_top(src,ir);
        dp=bmp_rowptr_from_top(dest,ir);
        for (i=0;i<src->width;i++)
            dp[i]=newval[sp[i]];
        }
    }



/*
** Sharpen a bitmap.
*/
void bmp_sharpen(WILLUSBITMAP *dest,WILLUSBITMAP *src)

    {
    void *ptr;
    double **filter;
    int i,j;

    vector_2d_alloc(&ptr,sizeof(double),3,3);
    filter=(double **)ptr;
    for (i=0;i<3;i++)
        for (j=0;j<3;j++)
            filter[i][j]=-0.1;
    filter[1][1]=1.8;
    bmp_apply_filter(dest,src,filter,3,3);
    ptr=(void *)filter;
    vector_2d_free(&ptr,3,3);
    }


/*
** Apply filter[0..ncols-1][0..nrows-1] to the src bitmap to create the
** dest bitmap.
**
** The src bitmap can be 8-bit or 24-bit.
** The dest bitmap need not be allocated yet.  It will be allocated
** as a 24-bit bitmap unless src is grayscale.
*/
void bmp_apply_filter(WILLUSBITMAP *dest,WILLUSBITMAP *src,double **filter,
                      int ncols,int nrows)

    {
    int ir,ic,rc,cc;
    int rf,rf1,rf2;
    int cf,cf1,cf2;
    int delspr,delspc;

    if (bmp_is_grayscale(src))
        {
        bmp_apply_filter_gray(dest,src,filter,ncols,nrows);
        return;
        }
    dest->width = src->width;
    dest->height = src->height;
    dest->bpp = 24;
    bmp_alloc(dest);
    rc=nrows/2;
    cc=ncols/2;
    delspr = bmp_rowptr_from_top(src,1)-bmp_rowptr_from_top(src,0);
    delspc = src->bpp==24 ? 3 : 1;
    for (ir=0;ir<src->height;ir++)
        {
        unsigned char *sp,*dp;
        sp=bmp_rowptr_from_top(src,ir);
        dp=bmp_rowptr_from_top(dest,ir);
        rf1=ir-rc<0 ? -ir : -rc;
        rf2=(ir+(nrows-rc-1)>src->height-1) ? src->height-1-ir : nrows-rc-1;
        for (ic=0;ic<src->width;ic++,dp+=3,sp+=delspc)
            {
            double weight,sr,sg,sb;
            int mr,mg,mb;

            cf1=ic-cc<0 ? -ic : -cc;
            cf2=(ic+(ncols-cc-1)>src->width-1) ? src->width-1-ic : ncols-cc-1;
            weight=sr=sg=sb=0.;
            for (rf=rf1;rf<=rf2;rf++)
                {
                unsigned char *sp1;
                sp1=sp+rf*delspr+cf1*delspc;
                for (cf=cf1;cf<=cf2;cf++)
                    {
                    int r,g,b;
                    double fw;
                    RGBGETINCPTR(src,sp1,r,g,b);
                    fw=filter[cf+cc][rf+rc];
                    weight+=fw;
                    sr += r*fw;
                    sg += g*fw;
                    sb += b*fw;
                    }
                }
            if (weight==0.)
                continue;
            mr = (sr/weight+.5);
            mg = (sg/weight+.5);
            mb = (sb/weight+.5);
            BOUND(mr,0,255);
            BOUND(mg,0,255);
            BOUND(mb,0,255);
            if (dest->type==WILLUSBITMAP_TYPE_NATIVE)
                {
                dp[0]=mr;
                dp[1]=mg;
                dp[2]=mb;
                }
            else
                {
                dp[2]=mr;
                dp[1]=mg;
                dp[0]=mb;
                }
            }
        }
    }

/*
** src must be 8-bit grayscale
*/
static void bmp_apply_filter_gray(WILLUSBITMAP *dest,WILLUSBITMAP *src,
                                  double **filter,int ncols,int nrows)

    {
    int i,ir,ic,rc,cc;
    int rf,rf1,rf2;
    int cf,cf1,cf2;
    int delspr;

    dest->width = src->width;
    dest->height = src->height;
    dest->bpp = 8;
    for (i=0;i<256;i++)
        dest->red[i]=dest->blue[i]=dest->green[i]=i;
    bmp_alloc(dest);
    rc=nrows/2;
    cc=ncols/2;
    delspr = bmp_rowptr_from_top(src,1)-bmp_rowptr_from_top(src,0);
    for (ir=0;ir<src->height;ir++)
        {
        unsigned char *sp,*dp;
        sp=bmp_rowptr_from_top(src,ir);
        dp=bmp_rowptr_from_top(dest,ir);
        rf1=ir-rc<0 ? -ir : -rc;
        rf2=(ir+(nrows-rc-1)>src->height-1) ? src->height-1-ir : nrows-rc-1;
        for (ic=0;ic<src->width;ic++,dp++,sp++)
            {
            double weight,sr;
            int mr;

            cf1=ic-cc<0 ? -ic : -cc;
            cf2=(ic+(ncols-cc-1)>src->width-1) ? src->width-1-ic : ncols-cc-1;
            weight=sr=0.;
            for (rf=rf1;rf<=rf2;rf++)
                {
                unsigned char *sp1;
                sp1=sp+rf*delspr+cf1;
                for (cf=cf1;cf<=cf2;cf++,sp1++)
                    {
                    double fw;
                    fw=filter[cf+cc][rf+rc];
                    weight+=fw;
                    sr += sp1[0]*fw;
                    }
                }
            if (weight==0.)
                continue;
            mr = (sr/weight+.5);
            BOUND(mr,0,255);
            dp[0]=mr;
            }
        }
    }




int bmp_jpeg_get_comments(char *filename,char **memptr,FILE *out)

    {
    FILE   *f;
    int     i,len;
    char   *buf;
    static char *funcname="bmp_jpeg_get_comments";

    f=fopen(filename,"rb");
    if (f==NULL)
        {
        nprintf(out,"Cannot open jpeg file %s for getting comments.\n",
                filename);
        return(-1);
        }
    if (!jpeg_read2(f,&i))
        {
        nprintf(out,"File %s is < 2 bytes.\n",filename);
        fclose(f);
        return(-2);
        }
    if (i!=0xffd8)
        {
        nprintf(out,"First two bytes of file %s aren't JPEG-like, = %04X\n",
                filename,i);
        fclose(f);
        return(-3);
        }
    while (1)
        {
        int     key;

        if (!jpeg_read2(f,&key) || !jpeg_read2(f,&len) || key==0xffda)
            {
            nprintf(out,"No comments in JPEG file %s.\n",filename);
            fclose(f);
            return(-4);
            }
        if (key==0xfffe)
            break;
        fseek(f,len-2,1);
        }
    willus_mem_alloc_warn((void **)memptr,len,funcname,10);
    buf=(*memptr);
    i=fread(buf,1,len-2,f);
    buf[len-2]='\0';
    fclose(f);
    if (i<len-2)
        {
        nprintf(out,"Comments from JPEG file %s may not be complete!\n",
                filename);
        return(-5);
        }
    return(0);
    }


int bmp_jpeg_set_comments(char *filename,char *buf,FILE *out)

    {
    FILE   *t,*f;
    int     i,c,status;
    char    tempfile[MAXFILENAMELEN];
    static char *werr="Error writing to temporary JPEG file %s!\n"
                      "File NOT deleted.\n";
    static char *peof="Premature EOF in JPEG file %s!\n";

    wfile_abstmpnam(tempfile);
    f=fopen(filename,"rb+");
    if (f==NULL)
        {
        nprintf(out,"Cannot open jpeg file %s for putting comments.\n",
                filename);
        return(-1);
        }
    t=fopen(tempfile,"wb");
    if (t==NULL)
        {
        fclose(f);
        nprintf(out,"Cannot open temporary jpeg file %s for writing.\n",
                tempfile);
        return(-2);
        }
    if (!jpeg_read2(f,&i))
        {
        nprintf(out,"File %s is < 2 bytes.\n",filename);
        fclose(t);
        remove(tempfile);
        fclose(f);
        return(-3);
        }
    if (i!=0xffd8)
        {
        nprintf(out,"First two bytes of file %s aren't JPEG-like, = %04X\n",
                filename,i);
        fclose(t);
        remove(tempfile);
        fclose(f);
        return(-4);
        }
    if (!jpeg_write2(t,i))
        {
        nprintf(out,werr,tempfile);
        fclose(t);
        fclose(f);
        return(-5);
        }
    while (1)
        {
        int     key,len;

        if (!jpeg_read2(f,&key) || !jpeg_read2(f,&len))
            {
            nprintf(out,"Ending key not found in JPEG file %s.\n",filename);
            fclose(t);
            remove(tempfile);
            fclose(f);
            return(-6);
            }
        if (key==0xfffe || key==0xffda)
            {
            if (!jpeg_write_comments(t,buf))
                {
                nprintf(out,werr,tempfile);
                fclose(t);
                fclose(f);
                return(-7);
                }
            if (key==0xfffe)
                status=fseek(f,len-2,1);
            else
                status=fseek(f,-4,1);
            if (status)
                {
                nprintf(out,peof,filename);
                fclose(t);
                remove(tempfile);
                fclose(f);
                return(-8);
                }
            break;
            }
        else
            {
            /* Copy this section to file */
            if (!jpeg_write2(t,key) || !jpeg_write2(t,len))
                {
                nprintf(out,werr,tempfile);
                fclose(t);
                fclose(f);
                return(-9);
                }
            for (i=0;i<len-2;i++)
                {
                if ((c=fgetc(f))==EOF)
                    {
                    nprintf(out,peof,filename);
                    fclose(t);
                    remove(tempfile);
                    fclose(f);
                    return(-10);
                    }
                if (fputc(c,t)<0)
                    {
                    nprintf(out,werr,tempfile);
                    fclose(t);
                    fclose(f);
                    return(-11);
                    }
                }
            }
        }
    /* Copy remaining part of file */
    while ((c=fgetc(f))!=EOF)
        if (fputc(c,t)<0)
            {
            nprintf(out,werr,tempfile);
            fclose(t);
            fclose(f);
            return(-12);
            }
    if (fclose(f))
        {
        nprintf(out,peof,filename);
        fclose(t);
        remove(tempfile);
        return(-13);
        }
    if (fclose(t))
        {
        nprintf(out,werr,tempfile);
        remove(tempfile);
        return(-14);
        }
    if (remove(filename))
        {
        nprintf(out,"Error removing file %s, which is to be replaced by file %s.\n"
                    "File %s not removed!\n",filename,tempfile);
        return(-15);
        }
    if (rename(tempfile,filename))
        {
        nprintf(out,"Error renaming file %s to %s!\n"
                    "Temporary file %s not deleted!\n",
                     tempfile,filename,tempfile);
        return(-16);
        }
    return(0);
    }


static int jpeg_write_comments(FILE *out,char *buf)

    {
    int len;

    len=strlen(buf)+3;
    if (!jpeg_write2(out,0xfffe) || !jpeg_write2(out,len)
           || fwrite(buf,1,len-2,out)<len-2)
        return(0);
    return(1);
    }
    

static int jpeg_read2(FILE *f,int *x)

    {
    int     b1,b2;

    b1=fgetc(f);
    if (b1==EOF)
        return(0);
    b2=fgetc(f);
    if (b2==EOF)
        return(0);
    (*x)=(b1<<8)|b2;
    return(1);
    }


static int jpeg_write2(FILE *f,int x)

    {
    int     b1,b2;

    b1=(x>>8)&0xff;
    if (fputc(b1,f)<0)
        return(0);
    b2=x&0xff;
    if (fputc(b2,f)<0)
        return(0);
    return(1);
    }


/*
** Tries to statistically find the most common colors used in an image.
**
** Puts them in order (most used first) in the rr[], gg[], and bb[] arrays
** (0-255) with percent[] indicating the percent of the bitmap that
** uses a color close to that color.  nmax is the max number to find.
** res tells how to break up the colorspace.  E.g. res=8 breaks it 
** into an 8 x 8 x 8 cube.
**
*/
void bmp_find_most_used_colors(WILLUSBITMAP *bmp,int *rr,int *gg,int *bb,
                               double *percent,int nmax,int res)

    {
    double ***hist;
    double npix;
    int n,i,j,k;
    void *ptr;

    n=res;
    vector_3d_alloc(&ptr,sizeof(double),n,n,n);
    hist = (double ***)ptr;
    for (i=0;i<n;i++)
        for (j=0;j<n;j++)
            for (k=0;k<n;k++)
                hist[i][j][k]=0.;
    for (i=0;i<bmp->height;i++)
        {
        unsigned char *p;

        p=bmp_rowptr_from_top(bmp,i);
        for (j=0;j<bmp->width;j++,p+=3)
            {
            int r,g,b;

            r=p[0];
            g=p[1];
            b=p[2];
            r=r*n/256;
            g=g*n/256;
            b=b*n/256;
            hist[r][g][b] += 1.0;
            }
        }
    npix = 100./(bmp->width*bmp->height);
    for (i=0;i<n;i++)
        for (j=0;j<n;j++)
            for (k=0;k<n;k++)
                hist[i][j][k] *= npix;
    for (i=0;i<nmax;i++)
        {
        find_most_common_color(hist,n,&rr[i],&gg[i],&bb[i],&percent[i]);
        /*
        if (i>1 && close_to_grey(rr,gg,bb))
            continue;
        printf("%2d. (r=%3d, g=%3d, b=%3d):  %7.4f %%\n",i,rr,gg,bb,percent);
        */
        }
    ptr=(void *)hist;
    vector_3d_free(&ptr,n,n,n);
    hist = (double ***)ptr;
    }


int bmp_close_to_grey(int r,int g,int b,double threshold)

    {
    double a,r0,g0,b0,max,min;

    a=(r+g+b)/3.;
    r0=r/a;
    g0=g/a;
    b0=b/a;
    max=min=r0;
    if (max<g0)
        max=g0;
    if (max<b0)
        max=b0;
    if (min>g0)
        min=g0;
    if (min>b0)
        min=b0;
    // printf("    %3d %3d %3d --------> %5.3f\n",r,g,b,max/min);
    if (max/min < threshold)
        return(1);
    return(0);
    }


static void find_most_common_color(double ***hist,int n,int *r,int *g,int *b,
                                   double *percent)

    {
    int rmax,gmax,bmax;
    int ir,ig,ib,dr,dg,db;
    double rwsum,gwsum,bwsum,count,wmax;

    rmax=gmax=bmax=wmax=0;
    for (ir=0;ir<n;ir++)
        for (ig=0;ig<n;ig++)
            for (ib=0;ib<n;ib++)
                {
                double w;
                w=0.;
                for (dr=-1;dr<=1;dr++)
                    {
                    if (ir+dr < 0 || ir+dr >= n)
                        continue;
                    for (dg=-1;dg<=1;dg++)
                        {
                        if (ig+dg < 0 || ig+dg >= n)
                            continue;
                        for (db=-1;db<=1;db++)
                            {
                            if (ib+db < 0 || ib+db >= n)
                                continue;
                            w += hist[ir+dr][ig+dg][ib+db];
                            }
                        }
                    }
                if (w > wmax)
                    {
                    wmax=w;
                    rmax=ir;
                    gmax=ig;
                    bmax=ib;
                    }
                }
    rwsum=gwsum=bwsum=count=0.;
    for (dr=-1;dr<=1;dr++)
        {
        if (rmax+dr < 0 || rmax+dr >= n)
            continue;
        for (dg=-1;dg<=1;dg++)
            {
            if (gmax+dg < 0 || gmax+dg >= n)
                continue;
            for (db=-1;db<=1;db++)
                {
                double weight;
                if (bmax+db < 0 || bmax+db >= n)
                    continue;
                weight = hist[rmax+dr][gmax+dg][bmax+db];
                rwsum += (rmax+dr)*weight;
                gwsum += (gmax+dg)*weight;
                bwsum += (bmax+db)*weight;
                count += weight;
                hist[rmax+dr][gmax+dg][bmax+db] = 0.;
                }
            }
        }
    if (count==0.)
        {
        (*r) = 0;
        (*g) = 0;
        (*b) = 0;
        }
    else
        {
        (*r) = (int)(255.9*((rwsum/count)+0.5)/n);
        (*g) = (int)(255.9*((gwsum/count)+0.5)/n);
        (*b) = (int)(255.9*((bwsum/count)+0.5)/n);
        }
    (*percent) = count;
    }




char *bmp_ansi_code(int r,int g,int b)

    {
    int i;
    static char ansicode[32];

    strcpy(ansicode,bmp_color_name(r,g,b));
    for (i=0;cnames[i][0]!='\0';i++)
        if (!stricmp(cnames[i],ansicode))
             {
             strcpy(ansicode,acodes[i]);
             return(ansicode);
             }
    strcpy(ansicode,ANSI_NORMAL);
    return(ansicode);
    }


/*
** Primitive color naming function
*/
char *bmp_color_name(int r,int g,int b)

    {
    if (r > g*1.2 && r > b*1.2)
        return(cnames[0]);
    if (g > b*1.2 && g > r*1.2)
        return(cnames[1]);
    if (b > g*1.2 && b > r*1.2)
        return(cnames[2]);
    if (r > g*1.2 && b > g*1.2)
        return(cnames[3]);
    if (b > r*1.2 && g > r*1.2)
        return(cnames[4]);
    if (r > b*1.2 && g > b*1.2)
        return(cnames[5]);
    if ((r+g+b) > 225*3)
        return(cnames[8]);
    if ((r+g+b) < 30*3)
        return(cnames[7]);
    return(cnames[6]);
    }


void bmp_autocrop(WILLUSBITMAP *bmp,int pad)

    {
    int r1,r2,c1,c2;
    WILLUSBITMAP *new,_new;

    new=&_new;
    bmp_init(new);
    for (r1=0;r1<bmp->height;r1++)
        if (!bmp_uniform_row(bmp,r1))
             break;
    if (r1>=bmp->height)
        return;
    r1-=pad;
    if (r1<0)
        r1=0;
    for (r2=bmp->height-1;r2>=0;r2--)
        if (!bmp_uniform_row(bmp,r2))
             break;
    r2+=pad;
    if (r2>bmp->height-1)
        r2=bmp->height-1;
    for (c1=0;c1<bmp->width;c1++)
        if (!bmp_uniform_col(bmp,c1))
             break;
    if (c1>=bmp->width)
        return;
    c1-=pad;
    if (c1<0)
        c1=0;
    for (c2=bmp->width-1;c2>=0;c2--)
        if (!bmp_uniform_col(bmp,c2))
             break;
    c2+=pad;
    if (c2>bmp->width-1)
        c2=bmp->width-1;
    bmp_crop(bmp,c1,r1,(c2-c1)+1,(r2-r1)+1);
    }


static int bmp_uniform_row(WILLUSBITMAP *bmp,int row)

    {
    unsigned char *p;
    int i,j;

    p=bmp_rowptr_from_top(bmp,row);
    if (bmp->bpp==8)
        {
        for (i=1;i<bmp->width;i++)
            if (p[i]!=p[0])
                break;
        }
    else
        {
        for (j=3,i=1;i<bmp->width;i++,j+=3)
            if (p[j]!=p[0] || p[j+1]!=p[1] || p[j+2]!=p[2])
                break;
        }
    return(i>=bmp->width);
    }


static int bmp_uniform_col(WILLUSBITMAP *bmp,int col)

    {
    unsigned char *p,*p0;
    int i;

    if (bmp->bpp==8)
        {
        p0=bmp_rowptr_from_top(bmp,0)+col;
        for (i=1;i<bmp->height;i++)
            {
            p=bmp_rowptr_from_top(bmp,i)+col;
            if (p[0]!=p0[0])
                break;
            }
        }
    else
        {
        p0=bmp_rowptr_from_top(bmp,0)+col*3;
        for (i=1;i<bmp->height;i++)
            {
            p=bmp_rowptr_from_top(bmp,i)+col*3;
            if (p[0]!=p0[0] || p[1]!=p0[1] || p[2]!=p0[2])
                break;
            }
        }
    return(i>=bmp->height);
    }


#ifdef HAVE_JASPER_LIB
/*
** Read image using Jasper library
** 9-3-2010
*/
int bmp_jasper_read(WILLUSBITMAP *bmp,char *filename,FILE *out)

    {
    jas_image_t *image;
    jas_stream_t *in;
    jas_matrix_t *jasdata;
    // jas_tmr_t dectmr;
    // double dectime; /* seconds? */
    int imagefmt,nc,ii,status;

    if (jas_init())
        {
        nprintf(out,"Jasper library init failure.\n");
        return(-1);
        }
    jas_setdbglevel(0);
    in=jas_stream_fopen(filename,"rb");
    if (in==NULL)
        {
        jas_image_clearfmts();
        nprintf(out,"Could not open file %s for input.\n",filename);
        return(-2);
        }

    /* Get image format */
    if ((imagefmt=jas_image_getfmt(in)) < 0)
        {
        jas_image_clearfmts();
        nprintf(out,"Format of file %s unknown by Jasper library.\n",filename);
        return(-3);
        }
    if (bmp==NULL)
        {
        jas_image_clearfmts();
        return(imagefmt);
        }

    /* Read image */
    // jas_tmr_start(&dectmr); /* If you want to time the image decode */
    image=jas_image_decode(in,imagefmt,0);
    // jas_tmr_stop(&dectmr); /* If you want to time the image decode */
    // dectime = jas_tmr_get(&dectmr);
    if (image==NULL)
        {
        jas_image_clearfmts();
        nprintf(out,"Could not read file %s with Jasper library.\n",filename);
        return(-4);
        }
    nc=jas_image_numcmpts(image);
    if (nc<=0 || nc==2)
        {
        jas_image_destroy(image);
        jas_image_clearfmts();
        nprintf(out,"File %s has %d image components--can't process.\n",filename,nc);
        return(-5);
        }
    if (nc>3)
        nc=3;
    for (ii=0;ii<nc;ii++)
        {
        int bpp,bs,bw,row;
        if (ii==0)
            {
            bmp->width = jas_image_cmptwidth(image,ii);
            bmp->height = jas_image_cmptheight(image,ii);
            if (nc==1)
                {
                int j;
                bmp->bpp=8;
                for (j=0;j<256;j++)
                    bmp->red[j]=bmp->green[j]=bmp->blue[j]=j;
                }
            else
                bmp->bpp=24;
            if (!bmp_alloc(bmp))
                {
                jas_image_destroy(image);
                jas_image_clearfmts();
                nprintf(out,"Not enough mem for %d x %d x %d Jasper image from file %s.\n",bmp->width,bmp->height,bmp->bpp,filename);
                return(-6);
                }
            }
        else
            {
            if (bmp->width != jas_image_cmptwidth(image,ii)
                 || bmp->height != jas_image_cmptheight(image,ii))
                {
                jas_image_destroy(image);
                jas_image_clearfmts();
                nprintf(out,"File %s has different-sized image components.\n",filename);
                return(-7);
                }
            }
        bpp = jas_image_cmptprec(image,ii);
        bs = bpp>8 ? bpp-8 : 8-bpp;
        jasdata = jas_matrix_create(bmp->height,bmp->width);
        if (jasdata==NULL)
            {
            jas_image_destroy(image);
            jas_image_clearfmts();
            nprintf(out,"Not enough mem for %d x %d x %d Jasper image from file %s.\n",bmp->width,bmp->height,bpp,filename);
            return(-8);
            }
        status=jas_image_readcmpt(image,ii,0,0,bmp->width,bmp->height,jasdata);
        if (status)
            {
            jas_matrix_destroy(jasdata);
            jas_image_destroy(image);
            jas_image_clearfmts();
            nprintf(out,"Error %d reading %d x %d x %d Jasper image from file %s.\n",status,bmp->width,bmp->height,bpp,filename);
            return(-9);
            }
        /* Copy component to bitmap structure */
        bw=bmp->bpp>>3;
        for (row=0;row<bmp->height;row++)
            {
            unsigned char *p;
            int col;
            p=bmp_rowptr_from_top(bmp,row)+ii;
            if (bpp>8)
                for (col=0;col<bmp->width;col++,p+=bw)
                    p[0]=(jas_matrix_get(jasdata,row,col) >> bs);
            else if (bpp<8)
                for (col=0;col<bmp->width;col++,p+=bw)
                    p[0]=(jas_matrix_get(jasdata,row,col) << bs);
            else
                for (col=0;col<bmp->width;col++,p+=bw)
                    p[0]=jas_matrix_get(jasdata,row,col);
            }
        jas_matrix_destroy(jasdata);
        }
    jas_image_destroy(image);
    jas_image_clearfmts();
    return(0);
    }
#endif // HAVE_JASPER_LIB


/*
** Allocate more bitmap rows.
** ratio typically something like 1.5 or 2.0
** If ratio not enough to increment height, 128 rows get added.
*/
void bmp_more_rows(WILLUSBITMAP *bmp,double ratio,int pixval)

    {
    int new_height,new_bytes,bw;
    static char *funcname="bmp_more_rows";

    new_height=(int)(bmp->height*ratio+.5);
    if (new_height <= bmp->height)
        new_height = bmp->height + 128;
    bw=bmp_bytewidth(bmp);
    new_bytes=bw*new_height;
    if (new_bytes > bmp->size_allocated)
        {
        willus_mem_realloc_robust_warn((void **)&bmp->data,
                  new_bytes,bmp->size_allocated,funcname,10);
        bmp->size_allocated=new_bytes;
        }
    /* Fill in */
    memset(bmp_rowptr_from_top(bmp,bmp->height),pixval,(new_height-bmp->height)*bw);
    bmp->height=new_height;
    }


/*
** Bitmap is assumed to be grayscale
*/
static double bmp_row_by_row_stdev(WILLUSBITMAP *bmp,int ccount,int whitethresh,
                                   double theta_radians)

    {
    int dc1,dc2,c1,c2;
    int r,n,nn,dw;
    double tanth,csum,csumsq,stdev;

    c1=bmp->width/15.;
    c2=bmp->width-c1;
    dw=(int)((c2-c1)/ccount+.5);
    if (dw<1)
        dw=1;
    tanth=-tan(theta_radians);
    dc1=(int)(tanth*bmp->width);
    if (dc1<0)
        {
        dc1=1-dc1;
        dc2=0;
        }
    else
        {
        dc2=-dc1-1;
        dc1=0;
        }
    dc1 += bmp->height/15.;
    dc2 -= bmp->height/15.;
    csum=csumsq=0.;
    n=0;
    for (r=dc1+1;r<bmp->height+dc2-1;r++)
        {
        int c,count,r0last;
        double dcount;
        unsigned char *p;

        r0last=0;
        p=bmp_rowptr_from_top(bmp,r0last);
        for (nn=count=0,c=c1;c<c2;c+=dw)
            {
            int r0;

            r0=r+tanth*c;
            if (r0<0 || r0>=bmp->height)
                continue;
            if (r0!=r0last)
                {
                r0last=r0;
                p=bmp_rowptr_from_top(bmp,r0last);
                }
            nn++;
            if (p[c]<whitethresh)
                count++;
            }
        dcount=100.*count/nn;
        csum+=dcount;
        csumsq+=dcount*dcount;
        n++;
        }
    stdev=sqrt(fabs((csum/n)*(csum/n)-csumsq/n));
    return(stdev);
    }


double bmp_autostraighten(WILLUSBITMAP *src,WILLUSBITMAP *srcgrey,int white,double maxdegrees,
                          double mindegrees,int debug,FILE *out)

    {
    int i,na,n,imax;
    double stepsize,sdmin,sdmax,rotdeg;
    double *sdev;
    FILE *f;
    static int rpc=0;
    static char *funcname="bmp_autostraighten";

    rpc++;
    f=NULL;
    if (debug)
        {
        f=fopen("straighten_metrics.ep",rpc==1?"w":"a");
        nprintf(f,"/sa l \"src page %d\" 2\n",rpc);
        }
    stepsize=.05;
    na = (int)(maxdegrees/stepsize+.5);
    if (na<1)
        na=1;
    n = 1+na*2;
    sdmin=999.;
    sdmax=-999.;
    imax=0;
    willus_mem_alloc_warn((void **)&sdev,n*sizeof(double),funcname,10);
    for (i=0;i<n;i++)
        {
        double theta,sdev0;

        theta = (i-na)*stepsize*PI/180.;
        sdev0=bmp_row_by_row_stdev(srcgrey,400,white,theta);
        if (sdmin > sdev0)
            sdmin = sdev0;
        if (sdmax < sdev0)
            {
            imax = i;
            sdmax = sdev0;
            }
        sdev[i]=sdev0;
        }
    if (sdmax<=0.)
        {
        willus_mem_free((double **)&sdev,funcname);
        if (debug)
            fclose(f);
        return(0.);
        }
    for (i=0;i<n;i++)
        sdev[i] /= sdmax;
    if (debug)
        {
        for (i=0;i<n;i++)
            nprintf(f,"%.3f %g\n",(i-na)*stepsize,sdev[i]);
        nprintf(f,"//nc\n");
        }
    sdmin /= sdmax;
    rotdeg = -(imax-na)*stepsize;
    if (sdmin > 0.95 || fabs(rotdeg) <= mindegrees
                     || fabs(fabs(rotdeg)-fabs(maxdegrees)) < 0.25)
        {
        willus_mem_free((double **)&sdev,funcname);
        if (debug)
            {
            nprintf(f,"/sa l \"srcpage %d, rot=%.2f deg (no rot)\" 2\n/sa m 2 3\n%g 0\n%g 1\n//nc\n",
                 rpc,-rotdeg,-rotdeg,-rotdeg);
            fclose(f);
            }
        return(0.);
        }
    if (imax>=3 && imax<=n-4)
        {
        double sd1min,sd2min,sdthresh;

        for (sd1min=sdev[imax-1],i=imax-2;i>=0;i--)
            if (sd1min > sdev[i])
                sd1min = sdev[i];
        for (sd2min=sdev[imax+1],i=imax+2;i<n;i++)
            if (sd2min > sdev[i])
                sd2min = sdev[i];
        sdthresh = sd1min > sd2min ? sd1min*1.01 : sd2min*1.01;
        if (sdthresh < 0.9)
            sdthresh = 0.9;
        if (sdthresh < 0.95)
            {
            double deg1,deg2;

            for (i=imax-1;i>=0;i--)
                if (sdev[i]<sdthresh)
                    break;
            deg1=stepsize*((i-na)+(sdthresh-sdev[i])/(sdev[i+1]-sdev[i]));
            for (i=imax+1;i<n-1;i++)
                if (sdev[i]<sdthresh)
                    break;
            deg2=stepsize*((i-na)-(sdthresh-sdev[i])/(sdev[i-1]-sdev[i]));
            if (deg2 - deg1 < 2.5)
                {
                rotdeg = -(deg1+deg2)/2.;
                if (debug)
                    nprintf(f,"/sa l \"srcpage %d, %.1f%% line\" 2\n/sa m 2 2\n"
                              "%g 0\n%g 1\n//nc\n"
                              "/sa l \"srcpage %d, %.1f%% line\" 2\n/sa m 2 2\n"
                              "%g 0\n%g 1\n//nc\n",rpc,sdthresh*100.,deg1,deg1,
                                                   rpc,sdthresh*100.,deg2,deg2);
                }
            }
        }
    if (debug)
        {
        nprintf(f,"/sa l \"srcpage %d, rot=%.2f\" 2\n/sa m 2 3\n%g 0\n%g 1\n//nc\n",
                 rpc,-rotdeg,-rotdeg,-rotdeg);
        fclose(f);
        }
    nprintf(out,"\n(Straightening page:  rotating cc by %.2f deg.)\n",rotdeg);
    /* BMP rotation fills with pixel value at (0,0) */
    if (debug)
        {
        char filename[256];
        sprintf(filename,"unrotated%05d.png",rpc);
        bmp_write(srcgrey,filename,stdout,100);
        }
    srcgrey->data[0]=255;
    bmp_rotate_fast(srcgrey,rotdeg,0);
    if (debug)
        {
        char filename[256];
        sprintf(filename,"rotated%05d_%03ddeg.png",rpc,(int)(rotdeg*100.));
        bmp_write(srcgrey,filename,stdout,100);
        }
    if (src!=NULL)
        {
        src->data[0]=src->data[1]=src->data[2]=255;
        bmp_rotate_fast(src,rotdeg,0);
        }
    willus_mem_free((double **)&sdev,funcname);
    return(rotdeg);
    }




/*
** Everything >= whitethresh gets 255.
*/
void bmp_apply_whitethresh(WILLUSBITMAP *bmp,int whitethresh)

    {
    int i,j;

    if (!bmp_is_grayscale(bmp))
        {
        for (i=0;i<bmp->height;i++)
            {
            unsigned char *p,*pcolor;

            pcolor=bmp_rowptr_from_top(bmp,i);
            p=bmp_rowptr_from_top(bmp,i);
            for (j=0;j<bmp->width;j++,p++,pcolor+=3)
                if ((*p)>=whitethresh)
                    (*p)=pcolor[0]=pcolor[1]=pcolor[2]=255;
            }
        }
    else
        {
        for (i=0;i<bmp->height;i++)
            {
            unsigned char *p;

            p=bmp_rowptr_from_top(bmp,i);
            for (j=0;j<bmp->width;j++,p++)
                if ((*p)>=whitethresh)
                    (*p)=255;
            }
        }
    }


/*
** Non-Floyd-Steinberg
** bpc = bits per color plane.
*/
void bmp_dither_to_bpc(WILLUSBITMAP *bmp,int newbpc)

    {
    int r,c,k,dbits,newmax,bshift,kmax;

    kmax=bmp->bpp==24 ? 3 : 1;
    newmax=(1<<newbpc)-1;
    bshift=8-newbpc;
    if (newbpc<1 || newbpc>7)
        return;
    if (newbpc<2)
        dbits=4;
    else if (newbpc<4)
        dbits=3;
    else if (newbpc<6)
        dbits=2;
    else
        dbits=1;
    for (r=0;r<bmp->height;r++)
        {
        unsigned char *p;
        p=bmp_rowptr_from_top(bmp,r);
        for (c=0;c<bmp->width;c++)
            for (k=0;k<kmax;k++,p++)
                p[0]=(pixval_dither(p[0],dbits,255,newmax,c,r)<<bshift);
        }
    }

/*
** pv = src pix value
** Dither using a 2^n x 2^n pattern (n from 1 to 4)
*/
static int pixval_dither(int pv,int n,int maxsrc,int maxdst,int x0,int y0)

    {
    int pv1,newval,newvalr,dl;

    if (pv==maxsrc)
        return(maxdst);
    if (pv==0)
        return(pv);
    pv1=pv*maxdst;
    newval=pv1/maxsrc;
    newvalr=pv1%maxsrc;
    dl=(newvalr<<(n*2))/maxsrc;
    return(newval+( (dl > dither_rec(n,x0,y0)) ? 1 : 0));
    }


static int dither_rec(int bits,int x0,int y0)

    {
    static int da[4]={0,3,2,1};
    int xr,yr;

    if (bits==0)
        return(0);
    xr=x0&1;
    yr=y0&1;
    return((da[(yr<<1)|xr]<<((bits-1)<<1))+dither_rec(bits-1,x0>>1,y0>>1));
    }


/*
** Extract a cropped part of src into dst.
** y0 is row FROM TOP!  0=top row, bmp->height-1=bottom row.
*/
void bmp_extract(WILLUSBITMAP *dst,WILLUSBITMAP *src,int x0,int y0_from_top,int width,int height)

    {
    int     x1,y1,y0,i,dbw,sbw;
    unsigned char *psrc,*pdest;

    dst->width=width;
    dst->height=height;
    dst->bpp=src->bpp;
    dst->type=src->type;
    for (i=0;i<256;i++)
        {
        dst->red[i]=src->red[i];
        dst->green[i]=src->green[i];
        dst->blue[i]=src->blue[i];
        }
    bmp_alloc(dst);
    y0 = y0_from_top;
    y1 = y0+height-1;
    x1 = x0+width-1;
    if (x1 > src->width-1)
        x1 = src->width-1;
    if (y1 > src->height-1)
        y1 = src->height-1;
    if (x0<0)
        x0 = 0;
    if (y0<0)
        y0 = 0;
    if (x0==0 && y0==0 && x1==src->width-1 && y1==src->height-1)
        {
        bmp_copy(dst,src);
        return;
        }
    sbw  = bmp_bytewidth(src);
    psrc = bmp_rowptr_from_top(src,src->type==WILLUSBITMAP_TYPE_WIN32?y1:y0)
            + ((src->bpp+7)>>3)*x0;
    dbw   = bmp_bytewidth(dst);
    pdest = dst->data;
    for (i=height;i>0;i--,psrc+=sbw,pdest+=dbw)
        memcpy(pdest,psrc,dbw);
    }
