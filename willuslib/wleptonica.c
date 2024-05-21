/*
** wleptonica.c  Routines to interface w/leptonica lib
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
#include "willus.h"

#ifdef HAVE_LEPTONICA_LIB
#include <leptonica.h>

static void wlept_pix_from_bmp(PIX **pixptr,WILLUSBITMAP *bmp);
static void wlept_bmp_from_pix(WILLUSBITMAP *bmp,PIX *pix);

static void wlept_pix_from_bmp(PIX **pixptr,WILLUSBITMAP *bmp)

    {
    PIX *pix;
    unsigned char *p;
    int i,pixbpl;

    (*pixptr)=pix=pixCreate(bmp->width,bmp->height,bmp->bpp<=8?8:32);
    p=(unsigned char *)pixGetData(pix);
    pixbpl=pixGetWpl(pix)*4;
    for (i=0;i<bmp->height;i++)
        {
        unsigned char *s,*d;
        int j;
        s=bmp_rowptr_from_top(bmp,i);
        d=&p[pixbpl*i];
        if (bmp->bpp==8)
            for (j=0;j<bmp->width;j++)
                {
                int j4,jmod;
                j4=j>>2;
                j4<<=2;
                jmod=j&3;
                p[pixbpl*i+j4+(3-jmod)]=s[j];
                }
        else
            for (j=0;j<bmp->width;j++,s+=3,d+=4)
                {
                d[0]=0;
                d[1]=s[2];
                d[2]=s[1];
                d[3]=s[0];
                }
        }
    }


static void wlept_bmp_from_pix(WILLUSBITMAP *bmp,PIX *pix)

    {
    unsigned char *p;
    int i,pixbpl;

    bmp->width=pix->w;
    bmp->height=pix->h;
    bmp->bpp=pix->d>8?24:8;
    if (bmp->bpp==8)
        for (i=0;i<256;i++)
            bmp->red[i]=bmp->green[i]=bmp->blue[i]=i;
    pixbpl=pixGetWpl(pix)*4;
    p=(unsigned char *)pixGetData(pix);
    for (i=0;i<bmp->height;i++)
        {
        unsigned char *s,*d;
        int j;
        d=bmp_rowptr_from_top(bmp,i);
        s=&p[pixbpl*i];
        if (bmp->bpp==8)
            for (j=0;j<bmp->width;j++)
                {
                int j4,jmod;
                j4=j>>2;
                j4<<=2;
                jmod=j&3;
                d[j]=p[pixbpl*i+j4+(3-jmod)];
                }
        else
            for (j=0;j<bmp->width;j++,s+=4,d+=3)
                {
                d[0]=s[3];
                d[1]=s[2];
                d[2]=s[1];
                }
        }
    }


/* De-warp image using Leptonica algorithm--based on dewarptest1.c in Leptonica distro */
/* src2 -> dst2 using the same warpedness of src -> dst */
/* Fit order = 2, 3, or 4 */
void wlept_bmp_dewarp(WILLUSBITMAP *src,WILLUSBITMAP *bmp1,WILLUSBITMAP *bmp2,int wthresh,
                      int fit_order,char *debugfile)

    {
    PIX *pix,*pixg,*pixb;
    L_DEWARPA *dewa;
    L_DEWARP  *dew1;
    char *debug;

    debug=(debugfile==NULL || debugfile[0]=='\0') ? NULL : debugfile;
    wlept_pix_from_bmp(&pix,src);
    if (pix->d>8)
        pixg=pixConvertRGBToGray(pix,.5,.3,.2);
    else
        pixg=pix;
    pixb=pixThresholdToBinary(pixg,wthresh);
    if (pixg!=pix)
        pixDestroy(&pixg);
/*
printf("pixb=%dx%dx%d\n",pixb->w,pixb->h,pixb->d);
pixWrite("pixb.png",pixb,IFF_PNG);
*/
    dewa=dewarpaCreate(2,50,1,10,30);
    dewarpaUseBothArrays(dewa,1);
    dew1=dewarpCreate(pixb,1);
    pixDestroy(&pixb);
    dewarpaInsertDewarp(dewa,dew1);
    dewarpBuildPageModel_ex(dew1,debug,fit_order);
    if (bmp1!=NULL)
        {
        PIX *pix2,*pix2d;
        wlept_pix_from_bmp(&pix2,bmp1);
        dewarpaApplyDisparity(dewa,1,pix2,-1,0,0,&pix2d,NULL);
        wlept_bmp_from_pix(bmp1,pix2d);
        pixDestroy(&pix2d);
        pixDestroy(&pix2);
        }
    if (bmp2!=NULL && bmp2!=bmp1)
        {
        PIX *pix2,*pix2d;
        wlept_pix_from_bmp(&pix2,bmp2);
        dewarpaApplyDisparity(dewa,1,pix2,-1,0,0,&pix2d,NULL);
        wlept_bmp_from_pix(bmp2,pix2d);
        pixDestroy(&pix2d);
        pixDestroy(&pix2);
        }
    dewarpaDestroy(&dewa); /* Includes dewarpDestroy of dew1 */
    pixDestroy(&pix);
    }
#endif /* HAVE_LEPTONICA_LIB */
