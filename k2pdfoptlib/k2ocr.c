/*
** k2ocr.c       k2pdfopt OCR functions
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

#include "k2pdfopt.h"

#ifdef HAVE_OCR_LIB
static int k2ocr_inited=0;
#if (defined(HAVE_TESSERACT_LIB))
static int k2ocr_tess_status=0;
#endif
static void k2ocr_ocrwords_fill_in(OCRWORDS *words,BMPREGION *region,K2PDFOPT_SETTINGS *k2settings);
static void ocrword_fillin_mupdf_info(OCRWORD *word,BMPREGION *region);
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
        k2ocr_tess_status=ocrtess_init(NULL,
                          k2settings->dst_ocr_lang[0]=='\0'?NULL:k2settings->dst_ocr_lang,stdout);
        k2printf(TTEXT_NORMAL);
        if (k2ocr_tess_status)
            k2printf(TTEXT_WARN "Could not find Tesseract data" TTEXT_NORMAL " (env var TESSDATA_PREFIX = %s).\nUsing GOCR v0.49.\n\n",getenv("TESSDATA_PREFIX")==NULL?"(not assigned)":getenv("TESSDATA_PREFIX"));
        else
            k2printf("\n");
#ifdef HAVE_GOCR_LIB
        }
    else
#endif
#endif
#ifdef HAVE_GOCR_LIB
        {
        if (k2settings->dst_ocr=='g')
            k2printf(TTEXT_BOLD "GOCR v0.49 OCR Engine" TTEXT_NORMAL "\n\n");
        }
#endif
#ifdef HAVE_MUPDF_LIB
    /* Could announce MuPDF virtual OCR here, but I think it will just confuse people. */
    /*
    if (k2settings->dst_ocr=='m')
        k2printf(TTEXT_BOLD "Using MuPDF \"Virtual\" OCR" TTEXT_NORMAL "\n\n");
    */
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
** wrectmaps has information that maps the bitmap pixel space to the original
** source page space so that the location of the words can be used to extract
** the text from a native PDF document if possible (dst_ocr=='m').
**
*/
void k2ocr_ocrwords_fill_in_ex(OCRWORDS *words,BMPREGION *region,K2PDFOPT_SETTINGS *k2settings)

    {
    PAGEREGIONS *pageregions,_pageregions;
    int i,maxlevels;

    if (k2settings->ocr_max_columns<0 || k2settings->ocr_max_columns <= k2settings->max_columns)
        {
#if (WILLUSDEBUGX & 32)
printf("Call #1. k2ocr_ocrwords_fill_in\n");
#endif
        k2ocr_ocrwords_fill_in(words,region,k2settings);
        return;
        }
    /* Parse region into columns for OCR */
    pageregions=&_pageregions;
    pageregions_init(pageregions);
    if (k2settings->ocr_max_columns==2 || k2settings->max_columns>1)
        maxlevels = 2;
    else
        maxlevels = 3;
    pageregions_find(pageregions,region,k2settings,maxlevels);
    for (i=0;i<pageregions->n;i++)
        {
        bmpregion_find_textrows(&pageregions->pageregion[i].bmpregion,k2settings,0,1);
        pageregions->pageregion[i].bmpregion.wrectmaps = region->wrectmaps;
#if (WILLUSDEBUGX & 32)
printf("Call #2. k2ocr_ocrwords_fill_in, pr = %d of %d\n",i+1,pageregions->n);
#endif
        k2ocr_ocrwords_fill_in(words,&pageregions->pageregion[i].bmpregion,k2settings);
        }
    pageregions_free(pageregions);
    }

    
/*
** Find words in src bitmap and put into words structure.
*/
static void k2ocr_ocrwords_fill_in(OCRWORDS *words,BMPREGION *region,K2PDFOPT_SETTINGS *k2settings)

    {
    int i;
    /* static char *funcname="ocrwords_fill_in"; */

/*
k2printf("@ocrwords_fill_in (%d x %d)...tr=%d\n",region->bmp->width,region->bmp->height,region->textrows.n);
if (region->textrows.n==0)
{
bmpregion_write(region,"out.png");
exit(10);
}
*/
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

#if (WILLUSDEBUGX & 32)
k2printf("    %d rows of text\n",region->textrows.n);
#endif
    /* Go text row by text row */
    for (i=0;i<region->textrows.n;i++)
        {
        BMPREGION _newregion,*newregion;
        int j,rowbase,r1,r2;
        double lcheight;
        TEXTWORDS *textwords;

        newregion=&_newregion;
        bmpregion_init(newregion);
        bmpregion_copy(newregion,region,0);
        r1=newregion->r1=region->textrows.textrow[i].r1;
        r2=newregion->r2=region->textrows.textrow[i].r2;
#if (WILLUSDEBUGX & 32)
printf("    @%3d,%3d = %3d x %3d\n",
region->textrows.textrow[i].c1,
region->textrows.textrow[i].r1,
region->textrows.textrow[i].c2-region->textrows.textrow[i].c1+1,
region->textrows.textrow[i].r2-region->textrows.textrow[i].r1+1);
#endif
        newregion->bbox.type = REGION_TYPE_TEXTLINE;
        rowbase=region->textrows.textrow[i].rowbase;
        lcheight=region->textrows.textrow[i].lcheight;
        /* Sanity check on rowbase, lcheight */
        if ((double)(rowbase-r1)/(r2-r1) < .5)
            rowbase = r1+(r2-r1)*0.7;
        if (lcheight/(r2-r1) < .33)
            lcheight = 0.33;
        /* Break text row into words (also establishes lcheight) */
        bmpregion_one_row_find_textwords(newregion,k2settings,0);
        textwords = &newregion->textrows;
        /* Add each word */
        for (j=0;j<textwords->n;j++)
            {
            char wordbuf[256];

            /* Don't OCR if "word" height exceeds spec */
            if ((double)(textwords->textrow[j].r2-textwords->textrow[j].r1+1)/region->dpi
                     > k2settings->ocr_max_height_inches)
                continue;
#if (WILLUSDEBUGX & 32)
printf("dst_ocr='%c', ocrtessstatus=%d\n",k2settings->dst_ocr,k2ocr_tess_status);
k2printf("j=%d of %d\n",j,textwords->n);
{
static int counter=1;
int i;
char filename[256];
WILLUSBITMAP *bmp,_bmp;
bmp=&_bmp;
bmp_init(bmp);
bmp->width=textwords->textrow[j].c2-textwords->textrow[j].c1+1;
bmp->height=textwords->textrow[j].r2-textwords->textrow[j].r1+1;
bmp->bpp=8;
bmp_alloc(bmp);
for (i=0;i<256;i++)
bmp->red[i]=bmp->green[i]=bmp->blue[i]=i;
for (i=0;i<bmp->height;i++)
{
unsigned char *s,*d;
s=bmp_rowptr_from_top(newregion->bmp8,textwords->textrow[j].r1+i)+textwords->textrow[j].c1;
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
            if (k2settings->dst_ocr=='t' && !k2ocr_tess_status)
#else
            if (!k2ocr_tess_status)
#endif
                ocrtess_single_word_from_bmp8(wordbuf,255,newregion->bmp8,
                                          textwords->textrow[j].c1,
                                          textwords->textrow[j].r1,
                                          textwords->textrow[j].c2,
                                          textwords->textrow[j].r2,3,0,1,NULL);
#ifdef HAVE_GOCR_LIB
            else if (k2settings->dst_ocr=='g')
#endif
#endif
#ifdef HAVE_GOCR_LIB
                jocr_single_word_from_bmp8(wordbuf,255,newregion->bmp8,
                                            textwords->textrow[j].c1,
                                            textwords->textrow[j].r1,
                                            textwords->textrow[j].c2,
                                            textwords->textrow[j].r2,0,1);
#endif
#ifdef HAVE_MUPDF_LIB
            if (k2settings->dst_ocr=='m')
                {
                wordbuf[0]='m'; /* Dummy value for now */
                wordbuf[1]='\0';
                }
#endif
#if (WILLUSDEBUGX & 32)
printf("..");
fflush(stdout);
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
                word.c=textwords->textrow[j].c1;
                /* Use same rowbase for whole row */
                word.r=rowbase;
                word.maxheight=rowbase-textwords->textrow[j].r1;
                word.w=textwords->textrow[j].c2-textwords->textrow[j].c1+1;
                word.h=textwords->textrow[j].r2-textwords->textrow[j].r1+1;
                word.lcheight=lcheight;
                word.rot=0;
                word.text=wordbuf;
                ocrword_fillin_mupdf_info(&word,region);
#if (WILLUSDEBUGX & 32)
k2printf("'%s', r1=%d, rowbase=%d, h=%d\n",wordbuf,
                             textwords->textrow[j].r1,
                             textwords->textrow[j].rowbase,
                             textwords->textrow[j].r2-textwords->textrow[j].r1+1);
#endif
                ocrwords_add_word(words,&word);
                }
            }
        bmpregion_free(newregion);
        }
    }


static void ocrword_fillin_mupdf_info(OCRWORD *word,BMPREGION *region)

    {
    int xp,yp; /* Top left of word on passed bitmap */
    int xc,yc; /* Center of word */
    int i;
    double pix2ptsw,pix2ptsh;
    WRECTMAP *wrmap;
    WRECTMAPS *wrectmaps;

    wrectmaps=region->wrectmaps;
#if (WILLUSDEBUGX & 0x400)
printf("Fillin: wrectmaps->n=%d\n",wrectmaps==NULL ? 0 : wrectmaps->n);
#endif
    if (wrectmaps==NULL || wrectmaps->n==0)
        {
        word->x0=word->c;
        word->y0=word->r-word->maxheight;
        word->w0=word->w;
        word->h0=word->h;
        word->rot0_deg=region->rotdeg;
        word->pageno=0;
        return;
        }
    xp=word->c;
    yp=word->r-word->maxheight;
    xc=xp+word->w/2;
    yc=yp+word->h/2;
#if (WILLUSDEBUGX & 0x400)
printf("    word xc,yc = (%d,%d)\n",xc,yc);
#endif
    for (i=0;i<wrectmaps->n;i++)
        {
#if (WILLUSDEBUGX & 0x400)
printf("    wrectmap[%d]= (%g,%g) , %g x %g\n",i,
wrectmaps->wrectmap[i].coords[1].x,
wrectmaps->wrectmap[i].coords[1].y,
wrectmaps->wrectmap[i].coords[2].x,
wrectmaps->wrectmap[i].coords[2].y);
#endif
        if (wrectmap_inside(&wrectmaps->wrectmap[i],xc,yc))
            break;
        }
    if (i>=wrectmaps->n)
        {
#if (WILLUSDEBUGX & 0x400)
printf("Not inside.\n");
#endif
        i=0;
        }
    wrmap=&wrectmaps->wrectmap[i];
    word->pageno = wrmap->srcpageno;
    word->rot0_deg = wrmap->srcrot;
    pix2ptsw = 72./wrmap->srcdpiw;
    pix2ptsh = 72./wrmap->srcdpih;
    word->x0 = (wrmap->coords[0].x + (xp-wrmap->coords[1].x))*pix2ptsw;
    word->y0 = (wrmap->coords[0].y + (yp-wrmap->coords[1].y))*pix2ptsh;
    word->w0 = word->w*pix2ptsw;
    word->h0 = word->h*pix2ptsh;
#if (WILLUSDEBUGX & 0x400)
printf("Word: (%5.1f,%5.1f) = %5.1f x %5.1f (page %2d)\n",word->x0,word->y0,word->w0,word->h0,word->pageno);
#endif
    }
#endif /* HAVE_OCR_LIB */
