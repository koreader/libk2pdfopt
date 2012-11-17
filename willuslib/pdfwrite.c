/*
** pdfwrite.c   Routines to help write a PDF file.
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

/*
** IMPORTANT!!
**
** NEEDS SPECIAL VERSION OF ZLIB WITH CUSTOM MODES--SEE gzflags BELOW!
** SEE gzwrite.c and gzlib.c in zlib_mod FOLDER.
** (SEARCH FOR "WILLUS MOD" IN FILES.)
**
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "willus.h"

#ifdef HAVE_Z_LIB
#include <zlib.h>
#endif

#define MAXPDFPAGES 10000

typedef struct
    {
    double abovebase;
    double belowbase;
    double x0;
    double width;
    double nextchar;
    } WILLUSCHARINFO;

static WILLUSCHARINFO Helvetica[96] =
    {
    /*   */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27811},
    /* ! */ { 0.72967,-0.00040, 0.12381, 0.08446, 0.27811},
    /* " */ { 0.71010,-0.46495, 0.05170, 0.25339, 0.35536},
    /* # */ { 0.69774, 0.01917, 0.01359, 0.52841, 0.55622},
    /* $ */ { 0.77087, 0.12526, 0.03213, 0.48618, 0.55622},
    /* % */ { 0.71010, 0.01917, 0.02801, 0.83124, 0.88893},
    /* & */ { 0.71010, 0.02226, 0.05170, 0.58609, 0.66747},
    /* ' */ { 0.72967,-0.49791, 0.06406, 0.09476, 0.22249},
    /* ( */ { 0.72967, 0.21179, 0.07230, 0.21940, 0.33373},
    /* ) */ { 0.72967, 0.21179, 0.03728, 0.21940, 0.33373},
    /* * */ { 0.72967,-0.44126, 0.03934, 0.30386, 0.38936},
    /* + */ { 0.47422, 0.00887, 0.04964, 0.48515, 0.58403},
    /* , */ { 0.10443, 0.14587, 0.08672, 0.10609, 0.27811},
    /* - */ { 0.31250,-0.24040, 0.04552, 0.23897, 0.33373},
    /* . */ { 0.10443,-0.00040, 0.08672, 0.10506, 0.27811},
    /* / */ { 0.72967, 0.01917,-0.00804, 0.29253, 0.27811},
    /* 0 */ { 0.71010, 0.02226, 0.04243, 0.46558, 0.55622},
    /* 1 */ { 0.71010,-0.00040, 0.10114, 0.24618, 0.55622},
    /* 2 */ { 0.71010,-0.00040, 0.03316, 0.47794, 0.55622},
    /* 3 */ { 0.71010, 0.02226, 0.03110, 0.47588, 0.55622},
    /* 4 */ { 0.71010,-0.00040, 0.02698, 0.49339, 0.55622},
    /* 5 */ { 0.71010, 0.02226, 0.03419, 0.47897, 0.55622},
    /* 6 */ { 0.71010, 0.02226, 0.04243, 0.47073, 0.55622},
    /* 7 */ { 0.71010,-0.00040, 0.04552, 0.47485, 0.55622},
    /* 8 */ { 0.71010, 0.02226, 0.03625, 0.47691, 0.55622},
    /* 9 */ { 0.71010, 0.02226, 0.03728, 0.47176, 0.55622},
    /* : */ { 0.52469,-0.00040, 0.10938, 0.10506, 0.27811},
    /* ; */ { 0.52469, 0.14587, 0.10938, 0.10609, 0.27811},
    /* < */ { 0.47422, 0.00784, 0.04449, 0.49030, 0.58403},
    /* = */ { 0.35371,-0.11165, 0.04964, 0.48515, 0.58403},
    /* > */ { 0.47422, 0.00784, 0.04964, 0.49030, 0.58403},
    /* ? */ { 0.74203,-0.00040, 0.07642, 0.43262, 0.55622},
    /* @ */ { 0.74203, 0.14175, 0.03316, 0.91880, 1.01562},
    /* A */ { 0.72967,-0.00040, 0.01668, 0.63657, 0.66747},
    /* B */ { 0.72967,-0.00040, 0.07848, 0.54489, 0.66747},
    /* C */ { 0.74203, 0.02226, 0.04758, 0.63039, 0.72206},
    /* D */ { 0.72967,-0.00040, 0.08878, 0.57888, 0.72206},
    /* E */ { 0.72967,-0.00040, 0.08981, 0.52326, 0.66747},
    /* F */ { 0.72967,-0.00040, 0.08981, 0.48927, 0.61082},
    /* G */ { 0.74203, 0.02226, 0.04346, 0.66644, 0.77871},
    /* H */ { 0.72967,-0.00040, 0.08260, 0.56137, 0.72206},
    /* I */ { 0.72967,-0.00040, 0.09908, 0.09579, 0.27811},
    /* J */ { 0.72967, 0.02226, 0.01668, 0.40996, 0.50060},
    /* K */ { 0.72967,-0.00040, 0.07848, 0.57991, 0.66747},
    /* L */ { 0.72967,-0.00040, 0.07951, 0.45425, 0.55622},
    /* M */ { 0.72967,-0.00040, 0.07436, 0.68704, 0.83330},
    /* N */ { 0.72967,-0.00040, 0.07539, 0.57064, 0.72206},
    /* O */ { 0.74203, 0.02226, 0.03728, 0.70558, 0.77871},
    /* P */ { 0.72967,-0.00040, 0.09084, 0.52635, 0.66747},
    /* Q */ { 0.74203, 0.05831, 0.03728, 0.70558, 0.77871},
    /* R */ { 0.72967,-0.00040, 0.09290, 0.58609, 0.72206},
    /* S */ { 0.74203, 0.02226, 0.04758, 0.57373, 0.66747},
    /* T */ { 0.72967,-0.00040, 0.02080, 0.57270, 0.61082},
    /* U */ { 0.72967, 0.02226, 0.08466, 0.56034, 0.72206},
    /* V */ { 0.72967,-0.00040, 0.02904, 0.61597, 0.66747},
    /* W */ { 0.72967,-0.00040, 0.02183, 0.90747, 0.94455},
    /* X */ { 0.72967,-0.00040, 0.02183, 0.62730, 0.66747},
    /* Y */ { 0.72967,-0.00040, 0.01256, 0.64893, 0.66747},
    /* Z */ { 0.72967,-0.00040, 0.02698, 0.55622, 0.61082},
    /* [ */ { 0.72967, 0.21179, 0.06303, 0.18747, 0.27811},
    /* \ */ { 0.72967, 0.01917,-0.00804, 0.29253, 0.27811},
    /* ] */ { 0.72967, 0.21179, 0.02286, 0.18644, 0.27811},
    /* ^ */ { 0.71010,-0.33001, 0.04346, 0.38215, 0.46970},
    /* _ */ {-0.12526, 0.17574,-0.02246, 0.60052, 0.55622},
    /* ` */ { 0.71010,-0.47731, 0.06406, 0.09476, 0.22249},
    /* a */ { 0.54014, 0.02226, 0.04140, 0.49442, 0.55622},
    /* b */ { 0.72967, 0.02226, 0.05376, 0.46970, 0.55622},
    /* c */ { 0.54014, 0.02226, 0.03007, 0.44704, 0.50060},
    /* d */ { 0.72967, 0.02226, 0.02595, 0.46970, 0.55622},
    /* e */ { 0.54014, 0.02226, 0.03934, 0.47382, 0.55622},
    /* f */ { 0.73276,-0.00040, 0.01771, 0.24103, 0.27811},
    /* g */ { 0.54014, 0.21694, 0.02801, 0.46146, 0.55622},
    /* h */ { 0.72967,-0.00040, 0.06921, 0.41717, 0.55622},
    /* i */ { 0.72967,-0.00040, 0.06509, 0.08549, 0.22249},
    /* j */ { 0.72967, 0.21694,-0.01834, 0.17202, 0.22249},
    /* k */ { 0.72967,-0.00040, 0.05788, 0.44498, 0.50060},
    /* l */ { 0.72967,-0.00040, 0.06715, 0.08549, 0.22249},
    /* m */ { 0.54014,-0.00040, 0.06921, 0.69322, 0.83330},
    /* n */ { 0.54014,-0.00040, 0.06921, 0.41820, 0.55622},
    /* o */ { 0.54014, 0.02226, 0.03522, 0.47485, 0.55622},
    /* p */ { 0.54014, 0.21694, 0.05376, 0.46970, 0.55622},
    /* q */ { 0.54014, 0.21694, 0.02595, 0.46970, 0.55622},
    /* r */ { 0.54014,-0.00040, 0.06818, 0.25339, 0.33373},
    /* s */ { 0.54014, 0.02226, 0.03316, 0.42644, 0.50060},
    /* t */ { 0.66890, 0.02226, 0.01359, 0.24103, 0.27811},
    /* u */ { 0.52469, 0.02226, 0.06406, 0.41820, 0.55622},
    /* v */ { 0.52469,-0.00040, 0.00947, 0.47691, 0.50060},
    /* w */ { 0.52469,-0.00040, 0.00535, 0.70352, 0.72206},
    /* x */ { 0.52469,-0.00040, 0.01668, 0.45631, 0.50060},
    /* y */ { 0.52469, 0.21694, 0.01977, 0.45837, 0.50060},
    /* z */ { 0.52469,-0.00040, 0.03007, 0.42747, 0.50060},
    /* { */ { 0.72967, 0.21179, 0.04243, 0.23382, 0.33476},
    /* | */ { 0.72967, 0.21179, 0.09908, 0.06180, 0.26060},
    /* } */ { 0.72967, 0.21179, 0.02801, 0.23485, 0.33476},
    /* ~ */ { 0.43920,-0.26924, 0.07436, 0.43365, 0.58403},
    /*  */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27811}
    };

static void pdffile_start(PDFFILE *pdf,int pages_at_end);
static void thumbnail_create(WILLUSBITMAP *thumb,WILLUSBITMAP *bmp);
static void pdffile_bmp_stream(PDFFILE *pdf,WILLUSBITMAP *bmp,int quality,int halfsize,int thumb);
static void bmp_flate_decode(WILLUSBITMAP *bmp,void *fptr,int halfsize);
static void bmpbytewrite(void *fptr,unsigned char *p,int n);
static void pdffile_new_object(PDFFILE *pdf,int flags);
static void pdffile_add_object(PDFFILE *pdf,PDFOBJECT *object);
#ifdef HAVE_Z_LIB
static int pdf_numpages_1(void *ptr,int bufsize);
static int decodecheck(FILE *f,int np);
static int getline(char *buf,int maxlen,FILE *f);
static int getbufline(char *buf,int maxlen,char *opbuf,int *i0,int bufsize);
#endif
static void insert_length(FILE *f,long pos,int len);
static void ocrwords_to_pdf_stream(OCRWORDS *ocrwords,FILE *f,double dpi,
                                   double page_height_pts,int text_render_mode);
static double ocrwords_median_size(OCRWORDS *ocrwords,double dpi);
static void ocrword_width_and_maxheight(OCRWORD *word,double *width,double *maxheight);
static double size_round_off(double size,double median_size,double log_size_increment);
static void ocrword_to_pdf_stream(OCRWORD *word,FILE *f,double dpi,
                                  double page_height_pts,double median_size_pts);

FILE *pdffile_init(PDFFILE *pdf,char *filename,int pages_at_end)

    {
    pdf->n=pdf->na=0;
    pdf->object=NULL;
    pdf->pae=0;
    pdf->imc=0;
    strncpy(pdf->filename,filename,511);
    pdf->filename[511]='\0';
    pdf->f = fopen(filename,"wb");
    if (pdf->f!=NULL)
        fclose(pdf->f);
    pdf->f = fopen(filename,"rb+");
    if (pdf->f!=NULL)
        pdffile_start(pdf,pages_at_end);
    return(pdf->f);
    }

void pdffile_close(PDFFILE *pdf)

    {
    if (pdf->f!=NULL)
        {
        fclose(pdf->f);
        pdf->f=NULL;
        }
    willus_mem_free((double **)&pdf->object,"pdffile_close");
    pdf->n=pdf->na=pdf->imc=0;
    }


static void pdffile_start(PDFFILE *pdf,int pages_at_end)

    {
    fprintf(pdf->f,"%%PDF-1.3 \n");
    pdffile_new_object(pdf,2);
    fprintf(pdf->f,"<<\n"
                   "/Pages ");
    fflush(pdf->f);
    fseek(pdf->f,0L,1);
    pdf->object[pdf->n-1].ptr[1]=ftell(pdf->f);
    if (pages_at_end)
        fprintf(pdf->f,"      ");
    else
        fprintf(pdf->f,"2");
    fprintf(pdf->f," 0 R\n"
                   "/Type /Catalog\n"
                   ">>\n"
                   "endobj\n");
    if (!pages_at_end)
        {
        int i;
        char cline[73];
        pdffile_new_object(pdf,4);
        fprintf(pdf->f,"<<\n"
                       "/Type /Pages\n"
                       "/Kids [");
        fflush(pdf->f);
        fseek(pdf->f,0L,1);
        pdf->pae=ftell(pdf->f);
        cline[0]='%';
        cline[1]='%';
        for (i=2;i<71;i++)
            cline[i]=' ';
        cline[71]='\n';
        cline[72]='\0';
        for (i=0;i<120;i++)
            fprintf(pdf->f,"%s",cline);
        }
    else
        pdf->pae=0;
    }


void pdffile_add_bitmap(PDFFILE *pdf,WILLUSBITMAP *bmp,double dpi,int quality,int halfsize)

    {
    pdffile_add_bitmap_with_ocrwords(pdf,bmp,dpi,quality,halfsize,NULL,1);
    }


/*
** Use quality=-1 for PNG
**
** NEEDS SPECIAL VERSION OF ZLIB WITH CUSTOM MOD--SEE gzflags BELOW!
** SEE gzwrite.c and gzlib.c in zlib_mod FOLDER.
** (SEARCH FOR "WILLUS MOD" IN FILES.)
**
** If quality < 0, the deflate (PNG-style) method is used.
**
** halfsize==0 for 8-bits per color plane
**         ==1 for 4-bits per color plane
**         ==2 for 2-bits per color plane
**         ==3 for 1-bit  per color plane
**
** visibility_flags
**     Bit 1 (1):  1=Show source bitmap
**     Bit 2 (2):  1=Show OCR text
**     Bit 3 (4):  1=Box around text
**
*/
void pdffile_add_bitmap_with_ocrwords(PDFFILE *pdf,WILLUSBITMAP *bmp,double dpi,
                                      int quality,int halfsize,OCRWORDS *ocrwords,
                                      int visibility_flags)

    {
    double pw,ph;
    int ptr1,ptr2,ptrlen,showbitmap;

    showbitmap = (visibility_flags&1);

    pw=bmp->width*72./dpi;
    ph=bmp->height*72./dpi;

    /* New page object */
    pdffile_new_object(pdf,3);
    pdf->imc++;
    fprintf(pdf->f,"<<\n"
                   "/Type /Page\n"
                   "/Parent ");
    fflush(pdf->f);
    fseek(pdf->f,0L,1);
    pdf->object[pdf->n-1].ptr[1]=ftell(pdf->f);
    fprintf(pdf->f,"%s 0 R\n"
                   "/Resources\n    <<\n",
                   pdf->pae>0 ? "2" : "      ");
    if (ocrwords!=NULL)
        fprintf(pdf->f,"    /Font << /F3 << /Type /Font /Subtype /Type1 /BaseFont /Helvetica /Encoding /WinAnsiEncoding >> >>\n");
    if (showbitmap)
        fprintf(pdf->f,"    /XObject << /Im%d %d 0 R >>\n"
                   "    /ProcSet [ /PDF /Text /ImageC ]\n",
                   pdf->imc,pdf->n+2);
    fprintf(pdf->f,"    >>\n"
                   "/MediaBox [0 0 %.1f %.1f]\n"
                   "/CropBox [0 0 %.1f %.1f]\n"
                   "/Contents %d 0 R\n",
                   pw,ph,pw,ph,
                   pdf->n+1); /* Contents stream */
    if (showbitmap)
        fprintf(pdf->f,"/Thumb %d 0 R\n",pdf->n+3);
    fprintf(pdf->f,">>\n"
                   "endobj\n");

    /* Execution stream:  draw bitmap and OCR words */
    pdffile_new_object(pdf,0);
    fprintf(pdf->f,"<< /Length ");
    fflush(pdf->f);
    fseek(pdf->f,0L,1);
    ptrlen=ftell(pdf->f);
    fprintf(pdf->f,"         >>\n"
                   "stream\n");
    fflush(pdf->f);
    fseek(pdf->f,0L,1);
    ptr1=ftell(pdf->f);
    if (showbitmap)
        fprintf(pdf->f,"q\n%.1f 0 0 %.1f 0 0 cm\n/Im%d Do\nQ\n",pw,ph,pdf->imc);
    if (ocrwords!=NULL)
        ocrwords_to_pdf_stream(ocrwords,pdf->f,dpi,ph,(visibility_flags&2)?0:3);
    if (visibility_flags&4)
        ocrwords_box(ocrwords,bmp);
    fflush(pdf->f);
    fseek(pdf->f,0L,1);
    ptr2=ftell(pdf->f);
    fprintf(pdf->f,"endstream\n"
                   "endobj\n");
    insert_length(pdf->f,ptrlen,ptr2-ptr1);
    if (showbitmap)
        {
        /* Stream the bitmap */
        pdffile_bmp_stream(pdf,bmp,quality,halfsize,0);
        /* Stream the thumbnail */
        pdffile_bmp_stream(pdf,bmp,quality,halfsize,1);
        }
    }


static void thumbnail_create(WILLUSBITMAP *thumb,WILLUSBITMAP *bmp)

    {
    if (bmp->width > bmp->height)
        {
        thumb->width = bmp->width<106 ? bmp->width : 106;
        thumb->height = (int)(((double)bmp->height/bmp->width)*thumb->width+.5);
        if (thumb->height<1)
            thumb->height=1;
        }
    else
        {
        thumb->height = bmp->height<106 ? bmp->height : 106;
        thumb->width = (int)(((double)bmp->width/bmp->height)*thumb->height+.5);
        if (thumb->width<1)
            thumb->width=1;
        }
    bmp_resample(thumb,bmp,0.,0.,(double)bmp->width,(double)bmp->height,
                 thumb->width,thumb->height);
    if (bmp->bpp==8)
        bmp_convert_to_greyscale(thumb);
    }


static void pdffile_bmp_stream(PDFFILE *pdf,WILLUSBITMAP *src,int quality,int halfsize,int thumb)

    {
    int ptrlen,ptr1,ptr2,bpc;
    WILLUSBITMAP *bmp,_bmp;

    if (thumb)
        {
        bmp=&_bmp;
        bmp_init(bmp);
        thumbnail_create(bmp,src);
        }
    else
        bmp=src;
    if (quality<0 && halfsize>0 && halfsize<4)
        bpc=8>>halfsize;
    else
        bpc=8;
    /* The bitmap */
    pdffile_new_object(pdf,0);
    fprintf(pdf->f,"<<\n");
    if (!thumb)
        fprintf(pdf->f,"/Type /XObject\n"
                       "/Subtype /Image\n");
#ifdef HAVE_JPEG_LIB
    if (quality>0)
        fprintf(pdf->f,"/Filter %s/DCTDecode%s\n",thumb?"[ ":"",thumb?" ]":"");
#endif
#if (defined(HAVE_JPEG_LIB) && defined(HAVE_Z_LIB))
    else
#endif
#ifdef HAVE_Z_LIB
        fprintf(pdf->f,"/Filter %s/FlateDecode%s\n",thumb?"[ ":"",thumb?" ]":"");
#endif
    fprintf(pdf->f,"/Width %d\n"
                   "/Height %d\n"
                   "/ColorSpace /Device%s\n"
                   "/BitsPerComponent %d\n"
                   "/Length ",
                   bmp->width,bmp->height,
                   bmp->bpp==8?"Gray":"RGB",
                   bpc);
    fflush(pdf->f);
    fseek(pdf->f,0L,1);
    ptrlen=(int)ftell(pdf->f);
    fprintf(pdf->f,"         \n"
                   ">>\n"
                   "stream\n");
    fflush(pdf->f);
    fseek(pdf->f,0L,1);
    ptr1=(int)ftell(pdf->f);
#ifdef HAVE_JPEG_LIB
    if (quality>0)
        {
        bmp_write_jpeg_stream(bmp,pdf->f,quality,NULL);
        fprintf(pdf->f,"\n");
        }
    else
#endif
        {
#ifdef HAVE_Z_LIB
        gzFile gz;
        static char *gzflags="sab7"; /* s is special flag set up by me in zlib */
                                     /* It turns off the gzip header/trailer   */
                                     /* 1 July 2011 */
        fclose(pdf->f);
        gz=gzopen(pdf->filename,gzflags);
        bmp_flate_decode(bmp,(void *)gz,halfsize);
        gzclose(gz);
        pdf->f=fopen(pdf->filename,"rb+");
        fseek(pdf->f,(size_t)0,2);
#else
        bmp_flate_decode(bmp,(void *)pdf->f,halfsize);
#endif
        fprintf(pdf->f,"\n");
        }
    fflush(pdf->f);
    fseek(pdf->f,0L,1);
    ptr2=(int)ftell(pdf->f)-1;
    fprintf(pdf->f,"endstream\nendobj\n");
    insert_length(pdf->f,ptrlen,ptr2-ptr1);
    if (thumb)
        bmp_free(bmp);
    }



/*
** halfsize==0 for 8-bits per color plane
**         ==1 for 4-bits per color plane
**         ==2 for 2-bits per color plane
**         ==3 for 1-bit  per color plane
*/
static void bmp_flate_decode(WILLUSBITMAP *bmp,void *fptr,int halfsize)

    {
    int row;
    static char *funcname="bmp_flate_decode";

    if (halfsize==1)
        {
        int w2,nb;
        unsigned char *data;
        nb=bmp->bpp==8 ? bmp->width : bmp->width*3;
        w2=(nb+1)/2;
        willus_mem_alloc_warn((void **)&data,w2,funcname,10);
        for (row=0;row<bmp->height;row++)
            {
            int i;
            unsigned char *p;
            p=bmp_rowptr_from_top(bmp,row);
            for (i=0;i<w2-1;i++,p+=2)
                data[i]=(p[0] & 0xf0) | (p[1] >> 4);
            if (nb&1)
                data[i]=p[0]&0xf0;
            else
                data[i]=(p[0]&0xf0) | (p[1] >> 4);
            if (bmp->bpp==8)
                bmpbytewrite(fptr,data,w2);
            else
                bmpbytewrite(fptr,data,w2);
            }
        willus_mem_free((double **)&data,funcname);
        }
    else if (halfsize==2)
        {
        int w2,nb;
        unsigned char *data;
        nb=bmp->bpp==8 ? bmp->width : bmp->width*3;
        w2=(nb+3)/4;
        willus_mem_alloc_warn((void **)&data,w2,funcname,10);
        for (row=0;row<bmp->height;row++)
            {
            int i,j,k;
            unsigned char *p;
            p=bmp_rowptr_from_top(bmp,row);
            for (i=0;i<w2-1;i++,p+=4)
                data[i]=(p[0] & 0xc0) | ((p[1] >> 2)&0x30) | ((p[2]>>4)&0xc) | (p[3]>>6);
            data[i]=0;
            j=(nb&3);
            if (j==0)
                j=4;
            for (k=0;k<j;k++)
                data[i]|=((p[k]&0xc0)>>(k*2));
            if (bmp->bpp==8)
                bmpbytewrite(fptr,data,w2);
            else
                bmpbytewrite(fptr,data,w2);
            }
        willus_mem_free((double **)&data,funcname);
        }
    else if (halfsize==3)
        {
        int w2,nb;
        unsigned char *data;
        nb=bmp->bpp==8 ? bmp->width : bmp->width*3;
        w2=(nb+7)/8;
        willus_mem_alloc_warn((void **)&data,w2,funcname,10);
        for (row=0;row<bmp->height;row++)
            {
            int i,j,k;
            unsigned char *p;
            p=bmp_rowptr_from_top(bmp,row);
            for (i=0;i<w2-1;i++,p+=8)
                data[i]=(p[0] & 0x80) | ((p[1]&0x80) >> 1)
                                      | ((p[2]&0x80) >> 2)
                                      | ((p[3]&0x80) >> 3)
                                      | ((p[4]&0x80) >> 4)
                                      | ((p[5]&0x80) >> 5)
                                      | ((p[6]&0x80) >> 6)
                                      | ((p[7]&0x80) >> 7);
            data[i]=0;
            j=(nb&7);
            if (j==0)
                j=8;
            for (k=0;k<j;k++)
                data[i]|=((p[k]&0x80)>>k);
            if (bmp->bpp==8)
                bmpbytewrite(fptr,data,w2);
            else
                bmpbytewrite(fptr,data,w2);
            }
        willus_mem_free((double **)&data,funcname);
        }
    else
        for (row=0;row<bmp->height;row++)
            {
            unsigned char *p;
            p=bmp_rowptr_from_top(bmp,row);
            if (bmp->bpp==8)
                bmpbytewrite(fptr,p,bmp->width);
            else
                bmpbytewrite(fptr,p,bmp->width*3);
            }
    }


static void bmpbytewrite(void *fptr,unsigned char *p,int n)

    {
#ifdef HAVE_Z_LIB
    gzwrite((gzFile)fptr,p,n);
#else
    fwrite(p,1,n,(FILE *)fptr);
#endif
    }


void pdffile_finish(PDFFILE *pdf,char *title,char *author,char *producer,char *cdate)

    {
    int icat,i,pagecount;
    time_t now;
    struct tm today;
    size_t ptr;
    char nbuf[10];
    char buf[128];
    char mdate[128];
    char basename[256];

    time(&now);
    today=(*localtime(&now));
    ptr=0; /* Avoid compiler warning */
    if (pdf->pae==0)
        {
        pdffile_new_object(pdf,0);
        icat=pdf->n;
        fprintf(pdf->f,"<<\n"
                   "/Type /Pages\n"
                   "/Kids [");
        }
    else
        {
        fflush(pdf->f);
        fseek(pdf->f,0L,1);
        ptr=ftell(pdf->f);
        icat=pdf->n;
        fseek(pdf->f,pdf->pae,0);
        }
    for (pagecount=i=0;i<pdf->n;i++)
        if (pdf->object[i].flags&1)
            {
            pagecount++;
            if (pagecount>MAXPDFPAGES && pdf->pae>0)
                {
                printf("WILLUS lib %s:  PDF page counts > %d not supported!\n",
                       willuslibversion(),MAXPDFPAGES);
                exit(10);
                }
            fprintf(pdf->f," %d 0 R",i+1);
            }
    fprintf(pdf->f," ]\n"
                   "/Count %d\n"
                   ">>\n"
                   "endobj\n",pagecount);
    if (pdf->pae > 0)
        {
        fseek(pdf->f,ptr,0);
        }
    pdffile_new_object(pdf,0);
    if (producer==NULL)
        sprintf(buf,"WILLUS lib %s",willuslibversion());
    else
        buf[0]='\0';
    for (i=0;buf[i]!='\0';i++)
        if (buf[i]=='(' || buf[i]==')')
            buf[i]=' ';
    sprintf(mdate,"D:%04d%02d%02d%02d%02d%02d%s",
                   today.tm_year+1900,today.tm_mon+1,today.tm_mday,
                   today.tm_hour,today.tm_min,today.tm_sec,
                   wsys_utc_string());
    fprintf(pdf->f,"<<\n");
    if (author!=NULL && author[0]!='\0')
        fprintf(pdf->f,"/Author (%s)\n",author);
    if (title==NULL || title[0]=='\0')
        wfile_basespec(basename,pdf->filename);
    fprintf(pdf->f,"/Title (%s)\n"
                   "/CreationDate (%s)\n"
                   "/ModDate (%s)\n"
                   "/Producer (%s)\n"
                   ">>\n"
                   "endobj\n",
                   title!=NULL && title[0]!='\0' ? title : basename,
                   cdate!=NULL && cdate[0]!='\0' ? cdate : mdate,
                   mdate,
                   producer==NULL ? buf : producer);
    fflush(pdf->f);
    fseek(pdf->f,0L,1);
    ptr=ftell(pdf->f);
    /* Kindles require the space after the 'f' and 'n' in the lines below. */
    fprintf(pdf->f,"xref\n"
                   "0 %d\n"
                   "0000000000 65535 f \n",pdf->n+1);
    for (i=0;i<pdf->n;i++)
        fprintf(pdf->f,"%010d 00000 n \n",(int)pdf->object[i].ptr[0]);
    fprintf(pdf->f,"trailer\n"
                   "<<\n"
                   "/Size %d\n"
                   "/Info %d 0 R\n"
                   "/Root 1 0 R\n"
                   ">>\n"
                   "startxref\n"
                   "%d\n"
                   "%%%%EOF\n",pdf->n+1,pdf->n,(int)ptr);
    /*
    ** Go back and put in catalog block references
    */
    if (pdf->pae==0)
        {
        sprintf(nbuf,"%6d",icat);
        for (i=0;i<pdf->n;i++)
            if (pdf->object[i].flags&2)
                {
                fseek(pdf->f,pdf->object[i].ptr[1],0);
                fwrite(nbuf,1,6,pdf->f);
                }
        }
    fclose(pdf->f);
    pdf->f=fopen(pdf->filename,"ab");
    }


static void pdffile_new_object(PDFFILE *pdf,int flags)

    {
    PDFOBJECT obj;

    fflush(pdf->f);
    fseek(pdf->f,0L,1);
    obj.ptr[0]=obj.ptr[1]=ftell(pdf->f);
    obj.flags=flags;
    pdffile_add_object(pdf,&obj);
    fprintf(pdf->f,"%d 0 obj\n",pdf->n);
    }


static void pdffile_add_object(PDFFILE *pdf,PDFOBJECT *object)

    {
    static char *funcname="pdffile_add_object";

    if (pdf->n>=pdf->na)
        {
        int newsize;
        newsize = pdf->na < 512 ? 1024 : pdf->na*2;
        if (pdf->na==0)
            willus_mem_alloc_warn((void **)&pdf->object,newsize*sizeof(PDFOBJECT),funcname,10);
        else
            willus_mem_realloc_robust_warn((void **)&pdf->object,newsize*sizeof(PDFOBJECT),
                                        pdf->na*sizeof(PDFOBJECT),funcname,10);
        pdf->na=newsize;
        }
    pdf->object[pdf->n++]=(*object);
    }


#ifdef HAVE_Z_LIB
int pdf_numpages(char *filename)

    {
    FILE *f;
    int np;

    f=fopen(filename,"rb");
    if (f==NULL)
        return(-1);
    np=pdf_numpages_1((void *)f,0);
    fclose(f);
    return(np);
    }


static int pdf_numpages_1(void *ptr,int bufsize)

    {
    char buf[256];
    FILE *f;
    char *opbuf;
    int i,i0,status,np,gls;
    static char *kwords[]={"/Type","/Pages","/Kids","/Count",
                           "/Filter","/FlateDecode","/Length",
                           "/ObjStm","stream",""};

    f=NULL; /* Avoid compiler warning */
    opbuf=NULL; /* Avoid compiler warning */
    if (bufsize==0)
        f=(FILE *)ptr;
    else
        opbuf=(char *)ptr;
    status=0;
    i0=0;
    np=-1;
    while (1)
        {
        if (bufsize==0)
            gls=getline(buf,254,f);
        else
            gls=getbufline(buf,254,opbuf,&i0,bufsize);
        for (i=0;kwords[i][0]!='\0';i++)
            {
            int ip;

            ip=in_string(buf,kwords[i]);
            if (ip>=0)
                {
                status |= (1<<i);
                if (i==3 || i==6)
                    np=atoi(&buf[ip+strlen(kwords[i])]);
/*
printf("    '%s' %x np=%d\n",kwords[i],status,np);
*/
                if (status==15 && np>0)
                    break;
                if (bufsize==0 && (status&0x1f1)==0x1f1 && np>0)
                    {
                    np=decodecheck(f,np);
                    if (np>0)
                        {
                        status=15;
                        break;
                        }
                    }
                }
            }
        if (status==15 && np>0)
            break;
        if (in_string(buf,"endobj")>=0)
            {
            status=0;
            np=-1;
            }
        if (!gls)
            break;
        }
    if (np>0)
        return(np);
    return(-2);
    }


static int decodecheck(FILE *f,int np)

    {
    char *inbuf,*outbuf;
    z_stream zstrm;
    int i0,status,obsize,extra;
    static char *funcname="decodecheck";

// printf("@decodecheck(np=%d)\n",np);    
    extra=4;
    willus_mem_alloc_warn((void **)&inbuf,np+extra,funcname,10);
    obsize=np*10;
    if (obsize<1024)
        obsize=1024;
    willus_mem_alloc_warn((void **)&outbuf,obsize,funcname,10);
    fread(inbuf,1,np+extra,f);
    i0=0;
    if (inbuf[i0]=='\n' || inbuf[i0]=='\r')
        i0++;
    memset(&zstrm,0,sizeof(zstrm));
    zstrm.avail_in=np+extra-i0;
    zstrm.avail_out=obsize;
    zstrm.next_in=(Bytef*)&inbuf[i0];
    zstrm.next_out=(Bytef*)outbuf;
    status=inflateInit(&zstrm);
    if (status!=Z_OK)
        {
        willus_mem_free((double **)&outbuf,funcname);
        willus_mem_free((double **)&inbuf,funcname);
        return(0);
        }
    status=inflate(&zstrm,Z_FINISH);
/*
printf("    Total output bytes = %d, status = %d\n",(int)zstrm.total_out,status);
printf("    ");
fwrite(outbuf,1,zstrm.total_out>2048 ? 2048:zstrm.total_out,stdout);
*/
    if (zstrm.total_out>0)
        np=pdf_numpages_1(outbuf,(int)zstrm.total_out);
    else
        np=0;
    willus_mem_free((double **)&outbuf,funcname);
    willus_mem_free((double **)&inbuf,funcname);
    return(np);
    }


static int getline(char *buf,int maxlen,FILE *f)

    {
    int i,c;

    i=0;
    while ((c=fgetc(f))!=EOF)
        {
        if (c=='\n' || c=='\r')
            break;
        buf[i++]=c;
        if (i>=maxlen)
            break;
        }
    buf[i]='\0';
    return(c!=EOF);
    }


static int getbufline(char *buf,int maxlen,char *opbuf,int *i0,int bufsize)

    {
    int i,c;

    i=0;
    while ((*i0) < bufsize)
        {
        c=opbuf[(*i0)];
        (*i0)=(*i0)+1;
        if (c=='\n' || c=='\r')
            break;
        buf[i++]=c;
        if (i>=maxlen)
            break;
        }
    buf[i]='\0';
    return((*i0)<bufsize);
    }
#endif /* HAVE_Z_LIB */


static void insert_length(FILE *f,long pos,int len)

    {
    long ptr;
    int i;
    char nbuf[64];

    fflush(f);
    fseek(f,0L,1);
    ptr=ftell(f);
    fseek(f,pos,0);
    sprintf(nbuf,"%d",len);
    for (i=0;i<8 && nbuf[i]!='\0';i++)
        fputc(nbuf[i],f);
    fseek(f,ptr,0);
    }


void ocrwords_box(OCRWORDS *ocrwords,WILLUSBITMAP *bmp)

    {
    int i,bpp;

    if (ocrwords==NULL)
        return;
    bpp=bmp->bpp==24 ? 3 : 1;
    for (i=0;i<ocrwords->n;i++)
        {
        int j;
        unsigned char *p;
        OCRWORD *word;
        word=&ocrwords->word[i];
        p=bmp_rowptr_from_top(bmp,word->r)+word->c*bpp;
        for (j=0;j<word->w;j++,p+=bpp)
            {
            (*p)=0;
            if (bpp==3)
                {
                p[1]=0;
                p[2]=255;
                }
            }
        p=bmp_rowptr_from_top(bmp,word->r-word->maxheight)+word->c*bpp;
        for (j=0;j<word->w;j++,p+=bpp)
            {
            (*p)=0;
            if (bpp==3)
                {
                p[1]=0;
                p[2]=255;
                }
            }
        for (j=0;j<word->maxheight;j++)
            {
            p=bmp_rowptr_from_top(bmp,word->r-j)+word->c*bpp;
            (*p)=0;
            if (bpp==3)
                {
                p[1]=0;
                p[2]=255;
                }
            p=bmp_rowptr_from_top(bmp,word->r-j)+(word->c+word->w-1)*bpp;
            (*p)=0;
            if (bpp==3)
                {
                p[1]=0;
                p[2]=255;
                }
            }
        }
    }


static void ocrwords_to_pdf_stream(OCRWORDS *ocrwords,FILE *f,double dpi,
                                   double page_height_pts,int text_render_mode)

    {
    int i;
    double median_size;

    fprintf(f,"BT\n%d Tr\n",text_render_mode);
    median_size=ocrwords_median_size(ocrwords,dpi);
    for (i=0;i<ocrwords->n;i++)
        ocrword_to_pdf_stream(&ocrwords->word[i],f,dpi,page_height_pts,median_size);
    fprintf(f,"ET\n");
    }


static double ocrwords_median_size(OCRWORDS *ocrwords,double dpi)

    {
    static char *funcname="ocrwords_to_histogram";
    static double *fontsize_hist;
    double msize;
    int i;

    if (ocrwords->n<=0)
        return(1.);
    willus_mem_alloc_warn((void **)&fontsize_hist,sizeof(double)*ocrwords->n,funcname,10);
    for (i=0;i<ocrwords->n;i++)
        {
        double w,h;
        ocrword_width_and_maxheight(&ocrwords->word[i],&w,&h);
        fontsize_hist[i] = (72.*ocrwords->word[i].maxheight/dpi) / h;
        }
    sortd(fontsize_hist,ocrwords->n);
    msize=fontsize_hist[ocrwords->n/2];
    if (msize < 0.5)
        msize = 0.5;
    willus_mem_free(&fontsize_hist,funcname);
    return(msize);
    }


static void ocrword_width_and_maxheight(OCRWORD *word,double *width,double *maxheight)

    {
    int i;

    (*width)=0.;
    (*maxheight)=0.;
    for (i=0;word->text[i]!='\0';i++)
        {
        int c;
        c=word->text[i]-32;
        if (c<0 || c>=96)
            c=0; 
        if (word->text[i+1]=='\0')
            (*width) += Helvetica[c].width;
        else
            (*width) += Helvetica[c].nextchar;
        if (Helvetica[c].abovebase > (*maxheight))
            (*maxheight)=Helvetica[c].abovebase;
        }
    }


static double size_round_off(double size,double median_size,double log_size_increment)

    {
    double rat,lograt;

    if (size < .5)
        size = .5;
    rat=size / median_size;
    lograt = floor(log10(rat)/log_size_increment+.5);
    return(median_size*pow(10.,lograt*log_size_increment));
    }


static void ocrword_to_pdf_stream(OCRWORD *word,FILE *f,double dpi,
                                  double page_height_pts,double median_size_pts)

    {
    int i,wordw;
    double fontsize_width,fontsize_height,ybase;
    double width_per_point,height_per_point,arat;
    char rotbuf[48];

    ocrword_width_and_maxheight(word,&width_per_point,&height_per_point);
    if (word->w/10. < word->lcheight)
        wordw = 0.9*word->w;
    else
        wordw = word->w-word->lcheight;
    fontsize_width = 72.*wordw/dpi / width_per_point;
    fontsize_height = size_round_off((72.*word->maxheight/dpi) / height_per_point,
                                       median_size_pts,.25);
    arat = fontsize_width / fontsize_height;
    ybase = page_height_pts - 72.*word->r/dpi;
    if (word->rot==0)
        sprintf(rotbuf,"%.4f 0 0 1",arat);
    else if (word->rot==90)
        sprintf(rotbuf,"0 %.4f -1 0",arat);
    else
        {
        double theta,sinth,costh;

        theta=word->rot*PI/180.;
        sinth=sin(theta);
        costh=cos(theta);
        sprintf(rotbuf,"%.3f %.3f %.3f %.3f",costh*arat,sinth*arat,-sinth,costh);
        }
    fprintf(f,"/F3 %.2f Tf\n"
              "%s %.2f %.2f Tm\n"
              "<",
              fontsize_height,rotbuf,72.*word->c/dpi,ybase);
    for (i=0;word->text[i]!='\0';i++)
        {
        int c;
        c=word->text[i]-32;
        if (c<0 || c>=96)
            c=0; 
        fprintf(f,"%02X",c+32);
        }
    fprintf(f,"> Tj\n");
    }
