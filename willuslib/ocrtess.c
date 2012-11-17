/*
** ocrtess.c   Get OCR of WILLUS bitmap using Tesseract
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <leptonica.h>
#include <tesseract.h>
#include "willus.h"


static void endian_flip(char *x,int n);

/*
** Returns 0 for success, NZ for failure.
*/
int ocrtess_init(char *datadir,char *lang,int ocr_type,FILE *out)

    {
    return(tess_capi_init(datadir,lang,ocr_type,out));
    }


void ocrtess_end(void)

    {
    tess_capi_end();
    }

/*
** bmp8 must be grayscale
** (x1,y1) and (x2,y2) from top left of bitmap
*/
void ocrtess_single_word_from_bmp8(char *text,int maxlen,WILLUSBITMAP *bmp8,
                                int x1,int y1,int x2,int y2,
                                int ocr_type,int allow_spaces,
                                int std_proc,FILE *out)

    {
    PIX *pix;
    int i,w,h,dw,dh,bw,status;
    unsigned char *src,*dst;

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
    dw=(dw+3)&(~3);
    if (y1>y2)
        {
        h=y1;
        y1=y2;
        y2=h;
        }
    h=y2-y1+1;
    dh=h+bw*2;
    pix=pixCreate(dw,dh,8);
    src=bmp_rowptr_from_top(bmp8,y1)+x1;
    dst=(unsigned char *)pixGetData(pix);
    memset(dst,255,dw*dh);
    dst=(unsigned char *)pixGetData(pix);
    dst += bw + dw*bw;
    for (i=y1;i<=y2;i++,dst+=dw,src+=bmp8->width) 
        memcpy(dst,src,w);
    endian_flip((char *)pixGetData(pix),pixGetWpl(pix)*pixGetHeight(pix));
    status=tess_capi_get_ocr(pix,text,maxlen,out);
    pixDestroy(&pix);
    if (status<0)
        text[0]='\0';
    clean_line(text);
    if (std_proc)
        ocr_text_proc(text,allow_spaces);
    }


static void endian_flip(char *x,int n)

    {
    int i;

    for (i=0;i<n;i++,x+=4)
        {
        int c;
        c=x[0];
        x[0]=x[3];
        x[3]=c;
        c=x[1];
        x[1]=x[2];
        x[2]=c;
        }
    }
