/*
** ocrgocr.c   Routine to interface with GOCR v0.50.
**
** Part of willus.com general purpose C code library.
**
** Copyright (C) 2018  http://willus.com
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
#include "willus.h"

#ifdef HAVE_GOCR_LIB
#include <gocr.h>

/*
** bmp8 must be grayscale
** (x1,y1) and (x2,y2) from top left of bitmap
*/
void gocr_single_word_from_bmp8(char *text,int maxlen,WILLUSBITMAP *bmp8,
                                int x1,int y1,int x2,int y2,int allow_spaces,
                                int std_proc)

    {
    OCRWORDS *ocrwords,_ocrwords;

    ocrwords=&_ocrwords;
    ocrwords_init(ocrwords);
    gocr_ocrwords_from_bmp8(ocrwords,bmp8,x1,y1,x2,y2,allow_spaces,std_proc);
    if (ocrwords->n>0)
        {
        strncpy(text,ocrwords->word[0].text,maxlen-1);
        text[maxlen-1]='\0';
        }
    else
        text[0]='\0';
    ocrwords_free(ocrwords);
    }


void gocr_ocrwords_from_bmp8(OCRWORDS *ocrwords,WILLUSBITMAP *bmp8,
                             int x1,int y1,int x2,int y2,int allow_spaces,
                             int std_proc)

    {
    job_t *job,_job;
    int i,w,h,dw,dh,bw;
    unsigned char *src,*dst;
    char *buf,*buf2;
    static char *funcname="gocr_ocrwords_from_bmp8";

    if (x1>x2)
        {
        w=x1;
        x1=x2;
        x2=w;
        }
    w=x2-x1+1;
    bw=w/40;
    if (bw<6)
        bw=6;
    dw=w+bw*2;
    if (y1>y2)
        {
        h=y1;
        y1=y2;
        y2=h;
        }
    h=y2-y1+1;
    dh=h+bw*2;
    job=&_job;
    job_init(job);
    job_init_image(job);
    // willus_mem_alloc_warn((void **)&job->src.p.p,w*h,funcname,10);
    /* Must use malloc since job_free_image counts on this. */
    job->src.p.p=malloc(dw*dh);
    job->src.p.x=dw;
    job->src.p.y=dh;
    job->src.p.bpp=1;
    src=bmp_rowptr_from_top(bmp8,y1)+x1;
    memset(job->src.p.p,255,dw*dh);
    dst=(unsigned char *)job->src.p.p + dw*bw + bw;
    for (i=y1;i<=y2;i++,dst+=dw,src+=bmp8->width)
        memcpy(dst,src,w);
    pgm2asc(job);
    buf=getTextLine(&(job->res.linelist),0);
    ocrwords_clear(ocrwords);
    {
    OCRWORD word;
    ocrword_init(&word);
    word.c=bw;
    word.r=y2;
    word.maxheight=y2-y1;
    word.w=x2-x1+1;
    word.h=y2-y1+1;
    word.lcheight=word.h;
    word.rot=0;
    willus_mem_alloc_warn((void **)&buf2,2*(strlen(buf)+1),funcname,10);
    strcpy(buf2,buf);
    if (std_proc)
        ocr_text_proc(buf2,allow_spaces);
    word.text=buf2;
    ocrwords_add_word(ocrwords,&word);
    willus_mem_free((double **)&buf2,funcname);
    }
    // willus_mem_free((double **)&job->src.p.p,funcname);
    job_free_image(job);
    }
#endif /* HAVE_GOCR_LIB */
