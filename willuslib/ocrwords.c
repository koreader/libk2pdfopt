/*
** ocrwords.c   Routines involving OCRWORDS structure but NOT actual OCR.
**              (No calls to GOCR or Tesseract.)
**
** Part of willus.com general purpose C code library.
**
** Copyright (C) 2022  http://willus.com
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include "willus.h"


static int ocrword_compare_position(OCRWORD *w1,OCRWORD *w2);


void ocrword_init(OCRWORD *word)

    {
    word->n=0;
    word->bmpscale=1.;
    word->cpos=NULL;
    word->text=NULL;
    word->xbmp=NULL;
    }


void ocrword_free(OCRWORD *word)

    {
    static char *funcname="ocrword_free";

    ocrword_bitmap_free(word);
    willus_mem_free((double **)&word->xbmp,funcname);
    willus_mem_free((double **)&word->cpos,funcname);
    willus_mem_free((double **)&word->text,funcname);
    }


void ocrwords_init(OCRWORDS *words)

    {
    words->word=NULL;
    words->n=words->na=0;
    }


/*
** Count number of bitmaps queued for OCR
*/
int ocrwords_num_queued(OCRWORDS *words)

    {
    int i,n;

    for (i=n=0;i<words->n;i++)
        if (ocrword_bitmap_ptr(&words->word[i])!=NULL)
            n++;
    return(n);
    }

/*
** allocates new memory
*/
void ocrword_copy(OCRWORD *dst,OCRWORD *src)

    {
    static char *funcname="ocrword_copy";

    (*dst)=(*src);
    dst->text=NULL;
    dst->cpos=NULL;
    dst->xbmp=NULL;
    if (src->text!=NULL)
        {
        willus_mem_alloc_warn((void **)&dst->text,strlen(src->text)+1,funcname,10);
        strcpy(dst->text,src->text);
        dst->n=utf8_to_unicode(NULL,dst->text,-1);
        }
    if (src->cpos!=NULL)
        {
        willus_mem_alloc_warn((void **)&dst->cpos,sizeof(double)*src->n,funcname,10);
        memcpy(dst->cpos,src->cpos,sizeof(double)*src->n);
        }
    if (ocrword_bitmap_ptr(src)!=NULL)
        ocrword_bitmap_copy(dst,ocrword_bitmap_ptr(src));
    }


WILLUSBITMAP *ocrword_bitmap_ptr(OCRWORD *word)

    {
    return(word->xbmp);
    }


void ocrword_bitmap_ensure(OCRWORD *word)

    {
    static char *funcname="ocrword_bitmap_ensure";

    if (word->xbmp==NULL)
        {
        willus_mem_alloc_warn((void **)&word->xbmp,sizeof(WILLUSBITMAP),funcname,10);
        bmp_init(word->xbmp);
        }
    }


void ocrword_bitmap_copy(OCRWORD *word,WILLUSBITMAP *bmp)

    {
    ocrword_bitmap_ensure(word);
    bmp_copy(ocrword_bitmap_ptr(word),bmp);
    }


void ocrword_bitmap8_copy_cropped(OCRWORD *word,WILLUSBITMAP *bmp8,int c1,int r1,
                                  int c2,int r2)

    {
    WILLUSBITMAP *bmp;
    int i;

    ocrword_bitmap_ensure(word);
    bmp=ocrword_bitmap_ptr(word);
    bmp_free(bmp);
    bmp->width=c2-c1+1;
    bmp->height=r2-r1+1;
    bmp->bpp=8;
    bmp_alloc(bmp);
    for (i=0;i<256;i++)
        bmp->red[i]=bmp->blue[i]=bmp->green[i]=i;
    bmp->type=WILLUSBITMAP_TYPE_NATIVE;
    for (i=r1;i<=r2;i++)
        {
        unsigned char *src,*dst;

        src=bmp_rowptr_from_top(bmp8,i)+c1;
        dst=bmp_rowptr_from_top(bmp,i-r1);
        memcpy(dst,src,bmp->width);
        }
    }


void ocrword_bitmap_free(OCRWORD *word)

    {
    if (ocrword_bitmap_ptr(word)!=NULL)
        bmp_free(ocrword_bitmap_ptr(word));
    }


/*
** Truncate word to chars from text[i1..i2]
*/
void ocrword_truncate(OCRWORD *word,int i1,int i2)

    {
    int i,n;
    int *u;
    double w0,scale;
    static char *funcname="ocrword_truncate";

    n=word->n;
    if (i1<0)
        i1=0;
    if (i2>n-1)
        i2=n-1;
    if (i2<i1)
        {
        word->n=0;
        word->w0=0.;
        word->w=0;
        return;
        }
    w0=word->w0;
    /* Fix text */
    willus_mem_alloc_warn((void **)&u,n*sizeof(int),funcname,10);
    utf8_to_unicode(u,word->text,n);
    unicode_to_utf8(word->text,&u[i1],i2-i1+1); 
    willus_mem_free((double **)&u,funcname);
    /* Fix length */
    word->n=i2-i1+1;
    if (word->cpos==NULL)
        return;
    /* Fix width */
    word->w0 = word->cpos[i2]-(i1==0?0.:word->cpos[i1-1]);
    scale = word->w/w0;
    word->w = (int)(scale*word->w0+.5);
    /* Fix left position and subsequent positions */
    if (i1>0)
        {
        double x0;

        x0=word->cpos[i1-1];
        word->x0 += x0;
        word->c += (int)(x0*scale+.5);
        for (i=i1;i<=i2;i++)
            word->cpos[i-i1]=word->cpos[i]-x0;
        }
    }


int ocrwords_to_textfile(OCRWORDS *ocrwords,char *filename,int append)

    {
    FILE *out;
    int i,newline;

    out=fopen(filename,append ? "a":"w");
    if (out==NULL)
        return(-1);
    for (i=0,newline=1;i<ocrwords->n;i++)
        {
        OCRWORD *word0;
        OCRWORD *word;
        int c,c0,r,r0;

        if (i>0)
            word0=&ocrwords->word[i-1];
        word=&ocrwords->word[i];
        if (word->text==NULL || word->text[0]=='\0')
            continue;
        if (word->rot==0)
            {
            c=word->c;
            r=word->r;
            if (i>0)
                {
                c0=word0->c;
                r0=word0->r;
                }
            }
        else
            {
            c=-word->r;
            r=word->c;
            if (i>0)
                {
                c0=-word0->r;
                r0=word0->c;
                }
            }
/*
if (!strnicmp(word->text,"II.",3))
printf("II. c=%d, r=%d, c0=%d, r0=%d, h=%d\n",c,r,c0,r0,word->h);
*/
        if (i>0 && (c<=c0 || r > r0+word->h*.75))
            {
            fprintf(out,"\n");
            if (i>0 && r > r0 + word->h*2)
                fprintf(out,"\n");
            newline=1;
            }
        if (!newline)
            fprintf(out," ");
        fprintf(out,"%s",word->text);
        newline=0;
        }
    if (!newline)
        fprintf(out,"\n");
    fclose(out);
    return(0);
    }


/*
** Allocates new space for text buffer.
*/
void ocrwords_add_word(OCRWORDS *words,OCRWORD *word)

    {
    static char *funcname="ocrwords_add_word";
    int i;

    if (words->n>=words->na)
        {
        int newsize;
      
        newsize = words->na<512 ? 1024 : words->na*2;
        willus_mem_realloc_robust_warn((void **)&words->word,newsize*sizeof(OCRWORD),
                                    words->na*sizeof(OCRWORD),funcname,10);
        for (i=words->na;i<newsize;i++)
            ocrword_init(&words->word[i]);
        words->na=newsize;
        }
    ocrword_copy(&words->word[words->n++],word);
    }


/*
** Remove words from index i1 through i2
*/
void ocrwords_remove_words(OCRWORDS *words,int i1,int i2)

    {
    int i,dn;

    if (i2>words->n-1)
        i2=words->n-1;
    if (i2<0)
        i2=0;
    if (i1>words->n-1)
        i1=words->n-1;
    if (i1<0)
        i1=0;
    if (i1>i2)
        {
        int t;
        t=i1;
        i1=i2;
        i2=t;
        }
    dn=i2-i1+1;
    for (i=i2;i>=i1;i--)
        ocrword_free(&words->word[i]);
    for (i=i1;i<words->na-dn;i++)
        words->word[i]=words->word[i+dn];
    for (i=words->na-dn;i<words->na;i++)
        ocrword_init(&words->word[i]);
    words->n -= dn;
    }


void ocrwords_clear(OCRWORDS *words)

    {
    int i;

    for (i=words->na-1;i>=0;i--)
        ocrword_free(&words->word[i]);
    words->n=0;
    }


void ocrwords_free(OCRWORDS *words)

    {
    int i;
    static char *funcname="ocrwords_free";

    for (i=words->na-1;i>=0;i--)
        ocrword_free(&words->word[i]);
    willus_mem_free((double **)&words->word,funcname);
    words->n=0;
    words->na=0;
    }


void ocrwords_sort_by_pageno(OCRWORDS *ocrwords)

    {
    int top,n1,n;
    OCRWORD x0,*x;
 
    x=ocrwords->word;
    n=ocrwords->n;
    if (n<2)
        return;
    top=n/2;
    n1=n-1;
    while (1)
        {
        if (top>0)
            {
            top--;
            x0=x[top];
            }
        else
            {
            x0=x[n1];
            x[n1]=x[0];
            n1--;
            if (!n1)
                {
                x[0]=x0;
                return;
                }
            }
        {
        int parent,child;

        parent=top;
        child=top*2+1;
        while (child<=n1)
            {
            if (child<n1 && x[child].pageno<x[child+1].pageno)
                child++;
            if (x0.pageno<x[child].pageno)
                {
                x[parent]=x[child];
                parent=child;
                child+=(parent+1);
                }
            else
                break;
            }
        x[parent]=x0;
        }
        }
    }


void ocrwords_offset(OCRWORDS *words,int dx,int dy)

    {
    int i;

    for (i=0;i<words->n;i++)
        {
        words->word[i].c += dx;
        words->word[i].r += dy;
        }
    }


/*
** Don't want this affecting the MuPDF vars.
*/
void ocrwords_scale(OCRWORDS *words,double srat)

    {
    int i;

    for (i=0;i<words->n;i++)
        {
        int c2,r2;
        c2 = (words->word[i].c+words->word[i].w-1)*srat;
        r2 = (words->word[i].r+words->word[i].h-1)*srat;
        words->word[i].c = words->word[i].c*srat;
        words->word[i].r = words->word[i].r*srat;
        words->word[i].maxheight = words->word[i].maxheight*srat;
        words->word[i].lcheight = words->word[i].lcheight*srat;
        words->word[i].w = c2-words->word[i].c+1;
        words->word[i].h = r2-words->word[i].r+1;
        if (ocrword_bitmap_ptr(&words->word[i])!=NULL)
            words->word[i].bmpscale *= srat;
        }
    }


void ocrwords_rot90(OCRWORDS *words,int bmpwidth_pixels)

    {
    int i;

    for (i=0;i<words->n;i++)
        {
        int cnew,rnew;
        words->word[i].rot=90;
        cnew = words->word[i].r;
        rnew = bmpwidth_pixels-1 - words->word[i].c;
        words->word[i].c = cnew;
        words->word[i].r = rnew;
        }
    }


/*
** Don't want this affecting the MuPDF vars.
*/
void ocrwords_int_scale(OCRWORDS *words,int ndiv)

    {
    int i;

    for (i=0;i<words->n;i++)
        {
        int c2,r2;
        c2 = (words->word[i].c+words->word[i].w-1)/ndiv;
        r2 = (words->word[i].r+words->word[i].h-1)/ndiv;
        words->word[i].c = words->word[i].c/ndiv;
        words->word[i].r = words->word[i].r/ndiv;
        words->word[i].maxheight = words->word[i].maxheight/ndiv;
        words->word[i].lcheight = words->word[i].lcheight/ndiv;
        words->word[i].w = c2-words->word[i].c+1;
        words->word[i].h = r2-words->word[i].r+1;
        if (ocrword_bitmap_ptr(&words->word[i])!=NULL)
            words->word[i].bmpscale /= ndiv;
        }
    }


void ocrwords_concatenate(OCRWORDS *dst,OCRWORDS *src)

    {
    int i;

    for (i=0;i<src->n;i++)
        ocrwords_add_word(dst,&src->word[i]);
    }


void ocrwords_sort_by_position(OCRWORDS *ocrwords)

    {
    int top,n1,n;
    OCRWORD x0,*x;

    x=ocrwords->word;
    n=ocrwords->n;
    if (n<2)
        return;
    top=n/2;
    n1=n-1;
    while (1)
        {
        if (top>0)
            {
            top--;
            x0=x[top];
            }
        else
            {
            x0=x[n1];
            x[n1]=x[0];
            n1--;
            if (!n1)
                {
                x[0]=x0;
                return;
                }
            }
        {
        int parent,child;

        parent=top;
        child=top*2+1;
        while (child<=n1)
            {
            if (child<n1 && ocrword_compare_position(&x[child],&x[child+1])<0)
                child++;
            if (ocrword_compare_position(&x0,&x[child])<0)
                {
                x[parent]=x[child];
                parent=child;
                child+=(parent+1);
                }
            else
                break;
            }
        x[parent]=x0;
        }
        }
    }

/*
** Allow up to 7% row overlap until two rows are considered the same row.
*/
static int ocrword_compare_position(OCRWORD *w1,OCRWORD *w2)

    {
    double percentage_overlap;
    int h,ol;

    if (w1->r <= w2->r-w2->h)
        return(-1);
    if (w1->r-w1->h >= w2->r)
        return(1);
    h=w1->h<w2->h ? w1->h : w2->h;
    if (h<1)
        h=1;
    ol=w1->r<w2->r ? w1->r-(w2->r-w2->h+1)+1 : w2->r-(w1->r-w1->h+1)+1;
    percentage_overlap=ol*100./h;
    if (percentage_overlap<7.)
        return(w1->r < w2->r ? -1 : 1);
    if (w1->c==w2->c)
        return(w1->r-w2->r);
    return(w1->c - w2->c);
    }


void ocrwords_to_easyplot(OCRWORDS *words,char *filename,int append,int *yoffset)

    {
    FILE *f;
    int i,ymin,ymax,ymin0,ymin1;
    static int count=0;

    ymin1=yoffset==NULL ? 0 : (*yoffset);
    f=fopen(filename,append?"a":"w");
    if (f==NULL)
        return;
    if (!append)
        {
        count=0;
        fprintf(f,"/sc on\n/sd off\n/n/sm off\n");
        }
    ymin=100000;
    ymax=0;
    for (i=0;i<words->n;i++)
        {
        OCRWORD *word;
        int y1,y2;
        word=&words->word[i];
        y1=word->r-word->h;
        y2=word->r;
        if (y1<ymin)
            ymin=y1;
        if (y2>ymax)
            ymax=y2;
        }
    ymin0=0;
    for (i=0;i<words->n;i++)
        {
        OCRWORD *word;
        int x1,y1,x2,y2;

        word=&words->word[i];
        x1=word->c;
        x2=word->c+word->w;
        y1=ymin1-(word->r-ymin);
        y2=ymin1-(word->r-word->h-ymin);
        if (y1<ymin0)
            ymin0=y1;
        fprintf(f,"/sa m 1 2\n%d %d\n%d %d\n%d %d\n%d %d\n%d %d\n//nc\n",x1,y1,x1,y2,x2,y2,x2,y1,x1,y1);
        fprintf(f,"/aa %d %g \"%d.%s\"\n",x1,y1+(y2-y1)*.2,count+1,word->text);
        count++;
        }
    if (ymin0<ymin1)
        ymin1=ymin0;
    ymin1-=10;
    if (yoffset!=NULL)
        (*yoffset)=ymin1;
    fclose(f);
    }


void ocrword_echo(OCRWORD *word,FILE *out,int count,int index,int writebmp)

    {
    WILLUSBITMAP *bmp;

    bmp=ocrword_bitmap_ptr(word);
    printf("%03d. @%4d,%4d, %3dx%3d, ",index,word->r,word->c,word->w,word->h);
    if (bmp!=NULL)
        {
        char filename[256];
        printf("BMP %d x %d",bmp->width,bmp->height);
        if (writebmp)
            {
            sprintf(filename,"word_%02d_%03d.png",count,index);
            bmp_write(bmp,filename,stdout,100);
            printf("..written to %s",filename);
            }
        printf("\n");
        }
    else
        {
        if (word->text!=NULL)
            {
            printf("text='%s'\n",word->text);
            printf("         Unicode text=");
            utf8_vals_to_stream(word->text,stdout);
            printf("\n");
            }
        }
    printf("        lch=%g px, mh=%g px, rot=%d deg\n",word->lcheight,word->maxheight,word->rot);
    }


void ocrwords_echo(OCRWORDS *ocrwords,FILE *out,int count,int writebmp)

    {
    int i;

    if (ocrwords==NULL)
        {
        printf("OCRWORD set is NULL.\n");
        return;
        }
    printf("OCRWORD set %d: nwords=%d\n",count,ocrwords->n);
    for (i=0;i<ocrwords->n;i++)
        {
        OCRWORD *word;
        word=&ocrwords->word[i];
        ocrword_echo(word,out,count,i+1,writebmp);
        }
    }

