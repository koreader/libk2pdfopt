/*
** k2ocr.c       k2pdfopt OCR functions
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

#include "k2pdfopt.h"

#ifdef HAVE_OCR_LIB
static int k2ocr_inited=0;
static void k2ocr_ocrwords_fill_in(K2PDFOPT_SETTINGS *k2settings,BMPREGION *region,
                            BREAKINFO *rowbreaks,int *colcount,int *rowcount,OCRWORDS *words);
#endif

void k2ocr_init(K2PDFOPT_SETTINGS *k2settings)

    {
#ifdef HAVE_OCR_LIB
    if (!k2settings->dst_ocr || k2ocr_inited)
        return;
    k2ocr_inited=1;
    ocrwords_init(&k2settings->dst_ocrwords);
#if (!defined(HAVE_TESSERACT_LIB) && defined(HAVE_GOCR_LIB))
    if (k2settings->dst_ocr=='t')
        {
        k2printf(TTEXT_WARN "\a** Tesseract not compiled into this version.  Using GOCR. **"
                TTEXT_NORMAL "\n\n");
        k2settings->dst_ocr='g';
        }
#endif
#if (defined(HAVE_TESSERACT_LIB) && !defined(HAVE_GOCR_LIB))
    if (k2settings->dst_ocr=='g')
        {
        k2printf(TTEXT_WARN "\a** GOCR not compiled into this version.  Using Tesseract. **"
                TTEXT_NORMAL "\n\n");
        k2settings->dst_ocr='t';
        }
#endif
#ifdef HAVE_TESSERACT_LIB
#ifdef HAVE_GOCR_LIB
    if (k2settings->dst_ocr=='t')
        {
#endif
        k2printf(TTEXT_BOLD);
        k2settings->ocrtess_status=ocrtess_init(NULL,
                          k2settings->dst_ocr_lang[0]=='\0'?NULL:k2settings->dst_ocr_lang,
                          3,stdout);
        k2printf(TTEXT_NORMAL);
        if (k2settings->ocrtess_status)
            k2printf(TTEXT_WARN "Could not find Tesseract data" TTEXT_NORMAL " (env var TESSDATA_PREFIX = %s).\nUsing GOCR v0.49.\n\n",getenv("TESSDATA_PREFIX")==NULL?"(not assigned)":getenv("TESSDATA_PREFIX"));
        else
            k2printf("\n");
#ifdef HAVE_GOCR_LIB
        }
    else
#endif
#endif
#ifdef HAVE_GOCR_LIB
        k2printf(TTEXT_BOLD "GOCR v0.49 OCR Engine" TTEXT_NORMAL "\n\n");
#endif
#endif /* HAVE_OCR_LIB */
    }


void k2ocr_end(K2PDFOPT_SETTINGS *k2settings)

    {
#ifdef HAVE_OCR_LIB
    if (k2settings->dst_ocr && k2ocr_inited)
        {
#ifdef HAVE_TESSERACT_LIB
        if (k2settings->dst_ocr=='t')
            ocrtess_end();
#endif
        ocrwords_free(&k2settings->dst_ocrwords);
        }
#endif /* HAVE_OCR_LIB */
    }


#ifdef HAVE_OCR_LIB
/*
** Find words in src bitmap and put into words structure.
** If ocrcolumns > maxcols, looks for multiple columns
**
*/
void k2ocr_ocrwords_fill_in_ex(K2PDFOPT_SETTINGS *k2settings,BMPREGION *region,BREAKINFO *rowbreaks,
                            int *colcount,int *rowcount,OCRWORDS *words)

    {
    PAGEREGIONS *pageregions,_pageregions;
    int i,maxlevels;

    if (k2settings->ocr_max_columns<0 || k2settings->ocr_max_columns <= k2settings->max_columns)
        {
        k2ocr_ocrwords_fill_in(k2settings,region,rowbreaks,colcount,rowcount,words);
        return;
        }
    /* Parse region into columns for OCR */
    pageregions=&_pageregions;
    pageregions_init(pageregions);
    if (k2settings->ocr_max_columns==2 || k2settings->max_columns>1)
        maxlevels = 2;
    else
        maxlevels = 3;
    pageregions_find(pageregions,region,rowcount,colcount,k2settings,maxlevels);
    for (i=0;i<pageregions->n;i++)
        {
        BREAKINFO *rbreaks,_rbreaks;
        BMPREGION *reg;

        reg=&pageregions->pageregion[i].bmpregion;
        rbreaks=&_rbreaks;
        rbreaks->textrow=NULL;
        breakinfo_alloc(104,rbreaks,reg->bmp8->height);
        bmpregion_find_vertical_breaks(reg,rbreaks,k2settings,colcount,rowcount,
                                       k2settings->column_row_gap_height_in);
        k2ocr_ocrwords_fill_in(k2settings,reg,rbreaks,colcount,rowcount,words);
        breakinfo_free(104,rbreaks);
        }
    pageregions_free(pageregions);
    }

    
/*
** Find words in src bitmap and put into words structure.
*/
static void k2ocr_ocrwords_fill_in(K2PDFOPT_SETTINGS *k2settings,BMPREGION *region,
                            BREAKINFO *rowbreaks,int *colcount,int *rowcount,OCRWORDS *words)

    {
    BREAKINFO *colbreaks,_colbreaks;
    int i;
    /* static char *funcname="ocrwords_fill_in"; */

#if (WILLUSDEBUGX & 32)
k2printf("@ocrwords_fill_in...\n");
#endif
/*
{
char filename[256];
count++;
sprintf(filename,"out%03d.png",count);
bmp_write(src,filename,stdout,100);
}
*/
    if (region->bmp->width<=0 || region->bmp->height<=0)
        return;

    /* Allocate colbreaks structures */
    colbreaks=&_colbreaks;
    colbreaks->textrow=NULL;
    breakinfo_alloc(105,colbreaks,region->bmp8->width);

#if (WILLUSDEBUGX & 32)
k2printf("    %d rows of text\n",rowbreaks->n);
#endif
    /* Go text row by text row */
    for (i=0;i<rowbreaks->n;i++)
        {
        BMPREGION _newregion,*newregion;
        int j,rowbase,r1,r2;
        double lcheight;

        newregion=&_newregion;
        (*newregion)=(*region);
        r1=newregion->r1=rowbreaks->textrow[i].r1;
        r2=newregion->r2=rowbreaks->textrow[i].r2;
        rowbase=rowbreaks->textrow[i].rowbase;
        lcheight=rowbreaks->textrow[i].lcheight;
        /* Sanity check on rowbase, lcheight */
        if ((double)(rowbase-r1)/(r2-r1) < .5)
            rowbase = r1+(r2-r1)*0.7;
        if (lcheight/(r2-r1) < .33)
            lcheight = 0.33;
        /* Break text row into words (also establishes lcheight) */
        bmpregion_one_row_find_breaks(newregion,colbreaks,k2settings,colcount,rowcount,0);
        /* Add each word */
        for (j=0;j<colbreaks->n;j++)
            {
            char wordbuf[256];

            /* Don't OCR if "word" height exceeds spec */
            if ((double)(colbreaks->textrow[j].r2-colbreaks->textrow[j].r1+1)/region->dpi
                     > k2settings->ocr_max_height_inches)
                continue;
#if (WILLUSDEBUGX & 32)
k2printf("j=%d of %d\n",j,colbreaks->n);
{
static int counter=1;
int i;
char filename[256];
WILLUSBITMAP *bmp,_bmp;
bmp=&_bmp;
bmp_init(bmp);
bmp->width=colbreaks->textrow[j].c2-colbreaks->textrow[j].c1+1;
bmp->height=colbreaks->textrow[j].r2-colbreaks->textrow[j].r1+1;
bmp->bpp=8;
bmp_alloc(bmp);
for (i=0;i<256;i++)
bmp->red[i]=bmp->green[i]=bmp->blue[i]=i;
for (i=0;i<bmp->height;i++)
{
unsigned char *s,*d;
s=bmp_rowptr_from_top(newregion->bmp8,colbreaks->textrow[j].r1+i)+colbreaks->textrow[j].c1;
d=bmp_rowptr_from_top(bmp,i);
memcpy(d,s,bmp->width);
}
sprintf(filename,"word%04d.png",counter);
bmp_write(bmp,filename,stdout,100);
bmp_free(bmp);
k2printf("%5d. ",counter);
fflush(stdout);
#endif
            wordbuf[0]='\0';
#ifdef HAVE_TESSERACT_LIB
#ifdef HAVE_GOCR_LIB
            if (k2settings->dst_ocr=='t' && !k2settings->ocrtess_status)
#else
            if (!k2settings->ocrtess_status)
#endif
                ocrtess_single_word_from_bmp8(wordbuf,255,newregion->bmp8,
                                          colbreaks->textrow[j].c1,
                                          colbreaks->textrow[j].r1,
                                          colbreaks->textrow[j].c2,
                                          colbreaks->textrow[j].r2,3,0,1,NULL);
#ifdef HAVE_GOCR_LIB
            else
#endif
#endif
#ifdef HAVE_GOCR_LIB
                jocr_single_word_from_bmp8(wordbuf,255,newregion->bmp8,
                                            colbreaks->textrow[j].c1,
                                            colbreaks->textrow[j].r1,
                                            colbreaks->textrow[j].c2,
                                            colbreaks->textrow[j].r2,0,1);
#endif
#if (WILLUSDEBUGX & 32)
if (wordbuf[0]!='\0')
{
char filename[256];
FILE *f;
sprintf(filename,"word%04d.txt",counter);
f=fopen(filename,"wb");
fprintf(f,"%s\n",wordbuf);
fclose(f);
k2printf("%s\n",wordbuf);
}
else
k2printf("(OCR failed)\n");
counter++;
}
#endif
            if (wordbuf[0]!='\0')
                {
                OCRWORD word;
                word.c=colbreaks->textrow[j].c1;
                /* Use same rowbase for whole row */
                word.r=rowbase;
                word.maxheight=rowbase-colbreaks->textrow[j].r1;
                word.w=colbreaks->textrow[j].c2-colbreaks->textrow[j].c1+1;
                word.h=colbreaks->textrow[j].r2-colbreaks->textrow[j].r1+1;
                word.lcheight=lcheight;
                word.rot=0;
                word.text=wordbuf;
#if (WILLUSDEBUGX & 32)
k2printf("'%s', r1=%d, rowbase=%d, h=%d\n",wordbuf,
                             colbreaks->textrow[j].r1,
                             colbreaks->textrow[j].rowbase,
                             colbreaks->textrow[j].r2-colbreaks->textrow[j].r1+1);
#endif
                ocrwords_add_word(words,&word);
                }
            }
        }
    breakinfo_free(105,colbreaks);
    }
#endif /* HAVE_OCR_LIB */
