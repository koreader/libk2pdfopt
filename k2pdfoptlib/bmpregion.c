/*
** bmpregion.c    Functions to handle BMPREGION structure.  These
**                are more-or-less generic functions that don't depend
**                heavily on k2pdfopt settings.
**
** Copyright (C) 2016  http://willus.com
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

static void trim_to(int *count,int *i1,int i2,double gaplen,int dpi,double defect_size_pts);
static int height2_calc(int *rc,int n);
static void bmpregion_count_text_row_pixels(BMPREGION *region,int *gw,int *copt,int *ngaps,
                                            K2PDFOPT_SETTINGS *k2settings);
static void bmpregion_find_gaps(BMPREGION *region,int *bp,int *gw,int *copt,int *ngaps);
static int get_word_gap_threshold(int *copt0,int *gw0,int ngaps,int dr,int row_width,
                                  BMPREGION *region,K2PDFOPT_SETTINGS *k2settings);
/*
static int word_longer_than(int gap_thresh,int *gw,int *copt,int ngaps,int display_width,
                            BMPREGION *region);
*/


int bmpregion_row_black_count(BMPREGION *region,int r0)

    {
    unsigned char *p;
    int i,nc,c;

    p=bmp_rowptr_from_top(region->bmp8,r0)+region->c1;
    nc=region->c2-region->c1+1;
    for (c=i=0;i<nc;i++,p++)
        if (p[0]<region->bgcolor)
            c++;
    return(c);
    }


int bmpregion_col_black_count(BMPREGION *region,int c0)

    {
    unsigned char *p;
    int i,nr,c,bw;

    bw=bmp_bytewidth(region->bmp8);
    p=bmp_rowptr_from_top(region->bmp8,region->r1)+c0;
    nr=region->r2-region->r1+1;
    for (c=i=0;i<nr;i++,p+=bw)
        if (p[0]<region->bgcolor)
            c++;
    return(c);
    }


// #if (defined(WILLUSDEBUGX) || defined(WILLUSDEBUG))
void bmpregion_write(BMPREGION *region,char *filename)

    {
    int i,bpp;
    WILLUSBITMAP *bmp,_bmp;

    bmp=&_bmp;
    bmp_init(bmp);
    bmp->width=region->c2-region->c1+1;
    bmp->height=region->r2-region->r1+1;
    bmp->bpp=region->bmp->bpp;
    bpp=bmp->bpp==8?1:3;
    bmp_alloc(bmp);
    for (i=0;i<256;i++)
        bmp->red[i]=bmp->green[i]=bmp->blue[i]=i;
    for (i=0;i<bmp->height;i++)
        {
        unsigned char *s,*d;
        s=bmp_rowptr_from_top(region->bmp,region->r1+i)+region->c1*bpp;
        d=bmp_rowptr_from_top(bmp,i);
        memcpy(d,s,bmp->width*bpp);
        }
    bmp_write(bmp,filename,stdout,97);
    bmp_free(bmp);
    }
// #endif

void bmpregion_row_histogram(BMPREGION *region)

    {
    static char *funcname="bmpregion_row_histogram";
    WILLUSBITMAP *src;
    FILE *out;
    int *rowcount;
    int *hist;
    int i,j,nn;

    src=region->bmp8;
    if (src==NULL)
        return;
    willus_dmem_alloc_warn(6,(void **)&rowcount,(region->r2-region->r1+1)*sizeof(int),funcname,10);
    willus_dmem_alloc_warn(7,(void **)&hist,(region->c2-region->c1+2)*sizeof(int),funcname,10);
    for (j=region->r1;j<=region->r2;j++)
        {
        unsigned char *p;
        p=bmp_rowptr_from_top(src,j)+region->c1;
        rowcount[j-region->r1]=0;
        for (i=region->c1;i<=region->c2;i++,p++)
            if (p[0]<region->bgcolor)
                rowcount[j-region->r1]++;
        }
    for (i=region->c1;i<=region->c2;i++)
        hist[i-region->c1]=0;
    for (i=region->r1;i<=region->r2;i++)
        hist[rowcount[i-region->r1]]++;
    for (i=region->c2-region->c1+1;i>=0;i--)
        if (hist[i]>0)
            break;
    nn=i;
    out=fopen("hist.ep","w");
    if (out!=NULL)
        {
        for (i=0;i<=nn;i++)
            fprintf(out,"%5d %5d\n",i,hist[i]);
        fclose(out);
        }
    out=fopen("rowcount.ep","w");
    if (out!=NULL)
        {
        for (i=0;i<region->r2-region->r1+1;i++)
            fprintf(out,"%5d %5d\n",i,rowcount[i]);
        fclose(out);
        }
    willus_dmem_free(7,(double **)&hist,funcname);
    willus_dmem_free(6,(double **)&rowcount,funcname);
    }


/*
** Return 0 if there are dark pixels in the region.  NZ otherwise.
*/
int bmpregion_is_clear(BMPREGION *region,int *row_black_count,int *col_black_count,
                       int *col_pix_count,int rpc,double gt_in)

    {
    int nr,nc,r,c,pt,mindim;

#if (WILLUSDEBUGX & 128)
printf("@bmpregion_is_clear(rpc=%d, gt_in=%g), region->dpi=%d\n",rpc,gt_in,region->dpi);
#endif
    pt=(int)(gt_in*region->dpi*(region->c2-region->c1+1)+.5);
    if (pt<0)
        pt=0;
    /*
    ** Fast way to count dark pixels, but requires big array
    */
    if (col_pix_count!=NULL && rpc>0)
        {
        int i;
        if (region->r1>0)
            for (c=0,i=region->c1;i<=region->c2;i++)
                {
                c += col_pix_count[i*rpc+region->r2] - col_pix_count[i*rpc+(region->r1-1)];
                if (c>pt)
                    return(0);
                }
        else
            for (c=0,i=region->c1;i<=region->c2;i++)
                {
                c += col_pix_count[i*rpc+region->r2];
                if (c>pt)
                    return(0);
                }
        return(pt<=0 ? 1 : 1+(int)10*c/pt);
        }
        
    /*
    ** row_black_count[] doesn't necessarily match up to this particular region's columns.
    ** So if row_black_count[] == 0, the row is clear, otherwise it has to be counted.
    ** because the columns are a subset.
    */
    nr=region->r2-region->r1+1;
    nc=region->c2-region->c1+1;
    mindim = nr>nc ? nc : nr;
    if (mindim > 5)
        {
        int i,bcc,brc;

        /*
        ** Determine most efficient way to see if the shaft is clear
        */
        for (bcc=0,i=region->c1;i<=region->c2;i++)
            if (col_black_count[i]==0)
                bcc++;
        for (brc=0,i=region->r1;i<=region->r2;i++)
            if (row_black_count[i]==0)
                brc++;

        /*
        ** Count dark pixels by columns
        */
        if (bcc*(region->r2-region->r1+1) > 2*brc*(region->c2-region->c1+1))
            {
            int col;

            for (c=0,col=region->c1;col<=region->c2;col++)
                {
                if (col<0 || col>=region->bmp8->width)
                    continue;
                if (col_black_count[col]==0)
                    continue;
                c+=bmpregion_col_black_count(region,col);
                if (c>pt)
                    return(0);
                }
            return(pt<=0 ? 1 : 1+(int)10*c/pt);
            }
        }

    /*
    ** Count dark pixels by rows
    */
    for (c=0,r=region->r1;r<=region->r2;r++)
        {
        if (r<0 || r>=region->bmp8->height)
            continue;
        if (row_black_count[r]==0)
            continue;
        c+=bmpregion_row_black_count(region,r);
        if (c>pt)
            return(0);
        }
/*
k2printf("(%d,%d)-(%d,%d):  c=%d, pt=%d (gt_in=%g)\n",
region->c1,region->r1,region->c2,region->r2,c,pt,gt_in);
*/
    return(pt<=0 ? 1 : 1+(int)10*c/pt);
    }


/*
** Sets region->c1,c2,r1,r2 directly.
** Must have region initialized and dpi, bmp, and bmp8 set.
** Changes bbox.type to UNDETERMINED.
*/
void bmpregion_trim_to_crop_margins(BMPREGION *region,MASTERINFO *masterinfo,
                                    K2PDFOPT_SETTINGS *k2settings)

    {
    int i,n;
    double margins_inches[4];

    region->c1=0;
    region->c2=region->bmp->width-1;
    region->r1=0;
    region->r2=region->bmp->height-1;
    for (i=0;i<4;i++)
        if (k2settings->srccropmargins.units[i]==UNITS_TRIMMED)
            break;
    if (i<4)
        bmpregion_trim_margins(region,k2settings,0xf);
    masterinfo_get_margins(k2settings,margins_inches,&k2settings->srccropmargins,
                           masterinfo,region);
    n=(int)(0.5+margins_inches[0]*region->dpi);
    if (n>region->bmp->width)
        n=region->bmp->width;
    region->c1=n;
    n=(int)(0.5+margins_inches[2]*region->dpi);
    if (n>region->bmp->width)
        n=region->bmp->width;
    region->c2=region->bmp->width-1-n;
    n=(int)(0.5+margins_inches[1]*region->dpi);
    if (n>region->bmp->height)
        n=region->bmp->height;
    region->r1=n;
    n=(int)(0.5+margins_inches[3]*region->dpi);
    if (n>region->bmp->height)
        n=region->bmp->height;
    region->r2=region->bmp->height-1-n;
    region->bbox.type=REGION_TYPE_UNDETERMINED;
    }

/*
** 1 = column 1 too short
** 2 = column 2 too short
** 3 = both too short
** 0 = both okay
** Both columns must pass height requirement.
**
** Also, if gap between columns > max_column_gap_inches, fails test. (8-31-12)
**
*/
int bmpregion_column_height_and_gap_test(BMPREGION *column,BMPREGION *region,
                                        K2PDFOPT_SETTINGS *k2settings,int r1,int r2,int cmid)

    {
    int min_height_pixels,status;

    status=0;
    min_height_pixels=k2settings->min_column_height_inches*region->dpi;
    bmpregion_copy(&column[0],region,0);
    column[0].r1=r1;
    column[0].r2=r2;
    column[0].c2=cmid-1;
    column[0].bbox.type=REGION_TYPE_UNDETERMINED;
    bmpregion_trim_margins(&column[0],k2settings,0xf);
/*
k2printf("    COL1:  pix=%d (%d - %d)\n",newregion->r2-newregion->r1+1,newregion->r1,newregion->r2);
*/
    if (column[0].r2-column[0].r1+1 < min_height_pixels)
        status |= 1;
    bmpregion_copy(&column[1],region,0);
    column[1].r1=r1;
    column[1].r2=r2;
    column[1].c1=cmid;
    column[1].c2=region->c2;
    column[1].bbox.type=REGION_TYPE_UNDETERMINED;
    bmpregion_trim_margins(&column[1],k2settings,0xf);
/*
k2printf("    COL2:  pix=%d (%d - %d)\n",newregion->r2-newregion->r1+1,newregion->r1,newregion->r2);
*/
    if (column[1].r2-column[1].r1+1 < min_height_pixels)
        status |= 2;
    /* Make sure gap between columns is not too large */
    if (k2settings->max_column_gap_inches>=0. && column[1].c1-column[0].c2-1 > k2settings->max_column_gap_inches*region->dpi)
        status |= 4;
    return(status);
    }


void bmpregion_init(BMPREGION *region)

    {
    region->bmp=region->bmp8=region->marked=NULL;
    region->dpi=0;
    region->pageno=0;
    region->rotdeg=0;
    region->c1=region->c2=0;
    region->r1=region->r2=0;
    region->colcount=NULL;
    region->rowcount=NULL;
    textrows_init(&region->textrows);
    textrow_init(&region->bbox);
    region->wrectmaps=NULL;
    region->k2pagebreakmarks=NULL;
    region->k2pagebreakmarks_allocated=0;
    }


void bmpregion_k2pagebreakmarks_allocate(BMPREGION *region)

    {
    static char *funcname="bmpregion_k2pagebreakmarks_allocate";

    bmpregion_k2pagebreakmarks_free(region);
    willus_dmem_alloc_warn(44,(void **)&region->k2pagebreakmarks,sizeof(K2PAGEBREAKMARKS),
                               funcname,10);
    region->k2pagebreakmarks_allocated=1;
    region->k2pagebreakmarks->n=0;
    }


void bmpregion_k2pagebreakmarks_free(BMPREGION *region)

    {
    static char *funcname="bmpregion_k2pagebreakmarks_free";

    if (region->k2pagebreakmarks!=NULL && region->k2pagebreakmarks_allocated)
        {
        willus_dmem_free(44,(double **)&region->k2pagebreakmarks,funcname);
        region->k2pagebreakmarks_allocated=0;
        }
    else
        region->k2pagebreakmarks=NULL;
    }


void bmpregion_free(BMPREGION *region)

    {
    static char *funcname="bmpregion_free";

    bmpregion_k2pagebreakmarks_free(region);
    willus_dmem_free(11,(double **)&region->rowcount,funcname);
    willus_dmem_free(10,(double **)&region->colcount,funcname);
    textrows_free(&region->textrows);
    }


/*
** New function in v2.36 to assess whether a region is blank
** Somewhat heuristic.
*/
int bmpregion_is_blank(BMPREGION *srcregion,K2PDFOPT_SETTINGS *k2settings)

    {
    BMPREGION region;
    double a1,a2;

    bmpregion_init(&region);
    bmpregion_copy(&region,srcregion,0);
    bmpregion_trim_margins(&region,k2settings,0xf);
    bmpregion_free(&region);
    a1=(double)srcregion->bmp->width*srcregion->bmp->height;
    a2=(double)(region.c2-region.c1+1)*(region.r2-region.r1+1);
#if (WILLUSDEBUGX & 0x200)
printf("a1=%g, a2=%g, a2/a1=%g\n",a1,a2,a2/a1);
#endif
    return(region.c2-region.c1<=5 || region.r2-region.r1<=5 || a2/a1<1e-4);
    }


/*
** Doesn't copy the colcount / rowcount pointers--those get NULLed.
*/
void bmpregion_copy(BMPREGION *dst,BMPREGION *src,int copy_text_rows)

    {
    TEXTROWS dtr;

    bmpregion_free(dst);
    dtr=dst->textrows;
    (*dst)=(*src);
    dst->k2pagebreakmarks_allocated=0;
    dst->textrows=dtr;
    textrows_clear(&dst->textrows);
    if (copy_text_rows)
        {
        int i;

        for (i=0;i<src->textrows.n;i++)
            textrows_add_textrow(&dst->textrows,&src->textrows.textrow[i]);
        }
    dst->colcount=dst->rowcount=NULL;
    }



/*
** Row base is where row dist crosses 50% on r2 side.
** Font size is where row dist crosses 5% on other side (r1 side).
** Lowercase font size is where row dist crosses 50% on r1 side.
**
** For 12 pt font:
**     Single spacing is 14.66 pts (Calibri), 13.82 pts (Times), 13.81 pts (Arial)
**     Size of cap letter is 7.7 pts (Calibri), 8.1 pts (Times), 8.7 pts (Arial)
**     Size of small letter is 5.7 pts (Calibri), 5.6 pts (Times), 6.5 pts (Arial)
** Mean line spacing = 1.15 - 1.22 (~1.16)
** Mean cap height = 0.68
** Mean small letter height = 0.49
** 
*/
void bmpregion_calc_bbox(BMPREGION *region,K2PDFOPT_SETTINGS *k2settings,int calc_text_params)

    {
    int i,j,n; /* ,r1,r2,dr1,dr2,dr,vtrim,vspace; */
    int maxcount,mc2,h2;
    double f;
    int *colcount,*rowcount;
    static char *funcname="bmpregion_calc_bbox";
    TEXTROW *bbox;

#if (WILLUSDEBUGX & 2)
{
printf("@bmpregion_calc_bbox(%d,%d)-(%d,%d)\n",region->c1,region->r1,region->c2,region->r2);
printf("    bmp8 = %d x %d\n",region->bmp8->width,region->bmp8->height);
printf("    region->colcount=%p\n",region->colcount);
printf("    region->rowcount=%p\n",region->rowcount);
}
#endif
    if (region->c2 > region->bmp8->width-1 || region->c2 > region->bmp->width-1
         || region->r2 > region->bmp8->height-1 || region->r2 > region->bmp->height-1)
        {
        printf("Internal error:  Bad c1/r1/c2/r2 vals @ bmpregion_calc_bbox!\n");
        printf("    region = (%d,%d)-(%d,%d)\n",region->c1,region->r1,region->c2,region->r2);
        printf("    bmp8 = %d x %d\n",region->bmp8->width,region->bmp8->height);
        printf("    bmp = %d x %d\n",region->bmp->width,region->bmp->height);
        printf("\nPlease contact author.\n");
        printf("\nProgram aborted.\n");
        exit(10);
        }
    bbox=&region->bbox;
    bbox->c1=region->c1;
    bbox->c2=region->c2;
    bbox->r1=region->r1;
    bbox->r2=region->r2;
    /*
    if ((bbox->type & BBOX_CALCED) && (!calc_text_params || (bbox->type & BBOX_TEXT_PARAMS)))
        return;
    */
    if (region->colcount==NULL)
        willus_dmem_alloc_warn(10,(void **)&region->colcount,sizeof(int)*region->bmp8->width,
                               funcname,10);
    colcount=region->colcount;
    if (region->rowcount==NULL)
        willus_dmem_alloc_warn(11,(void **)&region->rowcount,sizeof(int)*region->bmp8->height,
                               funcname,10);
    rowcount=region->rowcount;
    n=bbox->c2-bbox->c1+1;
#if (WILLUSDEBUGX & 2)
k2printf("Trim:  reg=(%d,%d) - (%d,%d)\n",bbox->c1,bbox->r1,bbox->c2,bbox->r2);
/*
if (bbox->c2+1 > cca || bbox->r2+1 > rca)
{
k2printf("A ha 0!\n");
exit(10);
}
*/
#endif
    memset(colcount,0,(bbox->c2+1)*sizeof(int));
    memset(rowcount,0,(bbox->r2+1)*sizeof(int));
    for (j=bbox->r1;j<=bbox->r2;j++)
        {
        unsigned char *p;
        p=bmp_rowptr_from_top(region->bmp8,j)+bbox->c1;
        for (i=0;i<n;i++,p++)
            if (p[0]<region->bgcolor)
                {
                rowcount[j]++;
                colcount[i+bbox->c1]++;
                }
        }
#if (WILLUSDEBUGX & 2)
{
if (region->rowcount!=NULL)
{
int i;
for (i=region->r1;i<=region->r2;i++)
printf("        rowcount[%d]=%d\n",i,region->rowcount[i]);
}
}
#endif
    /*
    ** Trim excess margins
    */
    trim_to(colcount,&bbox->c1,bbox->c2,k2settings->src_left_to_right ? 2.0 : 4.0,
            region->dpi,k2settings->defect_size_pts);
    trim_to(colcount,&bbox->c2,bbox->c1,k2settings->src_left_to_right ? 4.0 : 2.0,
            region->dpi,k2settings->defect_size_pts);
    /*
    if (colcount0==NULL)
        willus_dmem_free(10,(double **)&colcount,funcname);
    */
    trim_to(rowcount,&bbox->r1,bbox->r2,4.0,region->dpi,k2settings->defect_size_pts);
    trim_to(rowcount,&bbox->r2,bbox->r1,4.0,region->dpi,k2settings->defect_size_pts);
    if (calc_text_params)
        {
        /* Text row statistics */
        maxcount=0;
        for (i=bbox->r1;i<=bbox->r2;i++)
            if (rowcount[i] > maxcount)
                maxcount = rowcount[i];
        mc2 = maxcount / 2;
        for (i=bbox->r2;i>=bbox->r1;i--)
            if (rowcount[i] > mc2)
                break;
        bbox->rowbase = i;
        for (i=bbox->r1;i<=bbox->r2;i++)
            if (rowcount[i] > mc2)
                break;
        bbox->h5050 = bbox->lcheight = bbox->rowbase-i+1;
        mc2 = maxcount / 20;
        for (i=bbox->r1;i<=bbox->r2;i++)
            if (rowcount[i] > mc2)
                break;
        bbox->capheight = bbox->rowbase-i+1;
        /*
        ** Sanity check capheight and lcheight
        ** height2_calc() changed in v2.33.
        */
        h2=height2_calc(&rowcount[bbox->r1],bbox->r2-bbox->r1+1);
#if (WILLUSDEBUGX & 8)
if (bbox->c2-bbox->c1 > 1500)
k2printf("reg %d x %d (%d,%d) - (%d,%d) h2=%d ch/h2=%g\n",bbox->c2-bbox->c1+1,bbox->r2-bbox->r1+1,bbox->c1,bbox->r1,bbox->c2,bbox->r2,h2,(double)bbox->capheight/h2);
#endif
#if (WILLUSDEBUGX & 16)
printf("capheight=%d, h2=%d\n",bbox->capheight,h2);
#endif
        if (bbox->capheight < h2*0.75)
            bbox->capheight = h2;
        f=(double)bbox->lcheight/bbox->capheight;
        if (f<0.55)
            bbox->lcheight = (int)(0.72*bbox->capheight+.5);
        else if (f>0.85)
            bbox->lcheight = (int)(0.72*bbox->capheight+.5);
#if (WILLUSDEBUGX & 8)
if (bbox->c2-bbox->c1 > 1500)
k2printf("    lcheight final = %d\n",bbox->lcheight);
#endif
#if (WILLUSDEBUGX & 16)
/*
if (bbox->c2-bbox->c1 > 1500 && bbox->r2-bbox->r1 < 100)
*/
if (bbox->lcheight==42)
{
static int append=0;
FILE *f;
int i;
printf("lcheight=%d, h2=%d, capheight=%d, h5050=%d\n",bbox->lcheight,h2,bbox->capheight,bbox->h5050);
f=fopen("textrows.ep",append==0?"w":"a");
append=1;
for (i=bbox->r1;i<=bbox->r2;i++)
fprintf(f,"%d %g\n",bbox->rowbase-i,(double)rowcount[i]/maxcount);
fprintf(f,"//nc\n");
fclose(f);
}
#endif
        }
/*
    else
        {
        bbox->h5050 = bbox->r2-bbox->r1+1;
        bbox->capheight = 0.68*(bbox->r2-bbox->r1+1);
        bbox->lcheight = 0.5*(bbox->r2-bbox->r1+1);
        bbox->rowbase = bbox->r2;
        }
*/
#if (WILLUSDEBUGX & 2)
k2printf("trim:\n    reg->c1=%d, reg->c2=%d\n",bbox->c1,bbox->c2);
k2printf("    reg->r1=%d, reg->r2=%d, reg->rowbase=%d\n\n",bbox->r1,bbox->r2,bbox->rowbase);
if (bbox->r1==2135)
{
bmp_write(region->bmp8,"out.png",NULL,100);
wfile_written_info("out.png",stdout);
exit(10);
}
#endif
    /*
    if (rowcount0==NULL)
        willus_dmem_free(11,(double **)&rowcount,funcname);
    */
    }


/*
** flags&1  : trim c1
** flags&2  : trim c2
** flags&4  : trim r1
** flags&8  : trim r2
** flags&16 : Find rowbase, font size, etc.
**
** Row base is where row dist crosses 50% on r2 side.
** Font size is where row dist crosses 5% on other side (r1 side).
** Lowercase font size is where row dist crosses 50% on r1 side.
**
** For 12 pt font:
**     Single spacing is 14.66 pts (Calibri), 13.82 pts (Times), 13.81 pts (Arial)
**     Size of cap letter is 7.7 pts (Calibri), 8.1 pts (Times), 8.7 pts (Arial)
**     Size of small letter is 5.7 pts (Calibri), 5.6 pts (Times), 6.5 pts (Arial)
** Mean line spacing = 1.15 - 1.22 (~1.16)
** Mean cap height = 0.68
** Mean small letter height = 0.49
** 
*/
void bmpregion_trim_margins(BMPREGION *region,K2PDFOPT_SETTINGS *k2settings,int flags)

    {
    bmpregion_calc_bbox(region,k2settings,flags&0x10);
    /* To detect a hyphen, we need to trim and calc text base row */
    /*
    (unnecessary as of v1.70--always done)
    if (flags&32)
        flags |= 0x1f;
    */
    if (flags&1)
        region->c1 = region->bbox.c1;
    if (flags&2)
        region->c2 = region->bbox.c2;
    if (flags&4)
        region->r1 = region->bbox.r1;
    if (flags&8)
        region->r2 = region->bbox.r2;
    }


static void trim_to(int *count,int *i1,int i2,double gaplen,int dpi,double defect_size_pts)

    {
    int del,dcount,igaplen,clevel,dlevel,defect_start,last_defect;

    igaplen=(int)(gaplen*dpi/72.);
    if (igaplen<1)
        igaplen=1;
    /* clevel=(int)(defect_size_pts*dpi/72./3.); */
    clevel=0;
    dlevel=(int)(pow(defect_size_pts*dpi/72.,2.)*PI/4.+.5);
    del=i2>(*i1) ? 1 : -1;
    defect_start=-1;
    last_defect=-1;
    dcount=0;
    for (;(*i1)!=i2;(*i1)=(*i1)+del)
        {
        if (count[(*i1)]<=clevel)
            {
            dcount=0;  /* Reset defect size */
            continue;
            }
        /* Mark found */
        if (dcount==0)
            {
            if (defect_start>=0)
                last_defect=defect_start;
            defect_start=(*i1);
            }
        dcount += count[(*i1)];
        if (dcount >= dlevel)
            {
            if (last_defect>=0 && abs(defect_start-last_defect)<=igaplen)
                (*i1)=last_defect;
            else
                (*i1)=defect_start;
            return;
            }
        }
    if (defect_start<0)
        return;
    if (last_defect<0)
        {
        (*i1)=defect_start;
        return;
        }
    if (abs(defect_start-last_defect)<=igaplen)
        (*i1)=last_defect;
    else
        (*i1)=defect_start;
    }

/*
** Calculate weighted height of a rectangular region.
** This weighted height is intended to be close to the height of
** a capital letter, or the height of the majority of the region.
**
*/
static int height2_calc(int *rc,int n)

    {
    int i,thresh,i1,h2;
    int *c;
    static char *funcname="height2_calc";
#if (WILLUSDEBUGX & 8)
    int cmax;
#endif

    if (n<=0)
        return(1);
    willus_dmem_alloc_warn(12,(void **)&c,sizeof(int)*n,funcname,10);
    memcpy(c,rc,n*sizeof(int));
    sorti(c,n);
#if (WILLUSDEBUGX & 8)
    cmax=c[n-1];
#endif
#if (WILLUSDEBUGX & 16)
{
static int append=0;
FILE *f;
f=fopen("tr2.ep",append?"a":"w");
append=1;
for (i=0;i<n;i++)
fprintf(f,"%g %g\n",(double)i/n,(double)c[i]/c[n-1]);
fprintf(f,"//nc\n");
fclose(f);
}
#endif
    /* for (i=0;i<n-1 && c[i]==0;i++); */
    /* v2.33:  change from c[(i+n)/3] to c[9*n/10]/2 */
    thresh=c[9*n/10]/2;
    willus_dmem_free(12,(double **)&c,funcname);
    for (i=0;i<n-1;i++)
        if (rc[i]>=thresh)
            break;
    i1=i;
    for (i=n-1;i>i1;i--)
        if (rc[i]>=thresh)
            break;
#if (WILLUSDEBUGX & 8)
// k2printf("thresh = %g, i1=%d, i2=%d\n",(double)thresh/cmax,i1,i);
#endif
    h2=i-i1+1; /* Guaranteed to be >=1 */
    return(h2);
    }


/*
** Does region end in a hyphen?  If so, fill in HYPHENINFO structure.
*/
void bmpregion_hyphen_detect(BMPREGION *region,int hyphen_detect,int left_to_right)

    {
    int i,j; /* ,r1,r2,dr1,dr2,dr,vtrim,vspace; */
    int width;
    int *r0,*r1,*r2,*r3;
    int rmin,rmax,rowbytes,nrmid,rsum;
    int cstart,cend,cdir;
    unsigned char *p;
    static char *funcname="bmpregion_hyphen_detect";
    TEXTROW *textrow;

#if (WILLUSDEBUGX & 16)
static int count=0;
char pngfile[MAXFILENAMELEN];
FILE *out;

count++;
k2printf("@bmpregion_hyphen_detect count=%d\n",count);
sprintf(pngfile,"word%04d.png",count);
bmpregion_write(region,pngfile);
sprintf(pngfile,"word%04d.txt",count);
out=fopen(pngfile,"w");
fprintf(out,"c1=%d, c2=%d, r1=%d, r2=%d\n",region->c1,region->c2,region->r1,region->r2);
fprintf(out,"lcheight=%d\n",region->bbox.lcheight);
#endif

    textrow=&region->bbox;
    textrow->hyphen.ch = -1;
    textrow->hyphen.c2 = -1;
    /*
    ** Deleted checks for number of text rows and whether region is a figure.
    ** (Since only called from wrapbmp, region must be text.)
    */
    /* Was incorrect before v2.02--had this:  if (hyphen_detect) */
    if (!hyphen_detect)
        {
#if (WILLUSDEBUGX & 16)
printf("    hyphen_detect is off.\n");
#endif
        return;
        }
    if (textrow->c2<0 || textrow->c1<0 || textrow->r1<0 || textrow->r2<0
            || textrow->rowbase<0 || textrow->capheight<0 || textrow->lcheight<0)
        {
#if (WILLUSDEBUGX & 16)
printf("    bad c1,c2,r1,r2,rowbase.\n");
#endif
        return;
        }
    width=textrow->c2-textrow->c1+1;
    if (width<2)
        {
#if (WILLUSDEBUGX & 16)
printf("    width < 2.\n");
#endif
        return;
        }
    willus_dmem_alloc_warn(27,(void **)&r0,sizeof(int)*4*width,funcname,10);
    r1=&r0[width];
    r2=&r1[width];
    r3=&r2[width];
    for (i=0;i<width;i++)
        r0[i]=r1[i]=r2[i]=r3[i]=-1;
    rmin=textrow->rowbase-textrow->capheight-textrow->lcheight*.04;
    if (rmin < textrow->r1)
        rmin = textrow->r1;
    rmax=textrow->rowbase+textrow->lcheight*.04;
    if (rmax > textrow->r2)
        rmax = textrow->r2;
    rowbytes=bmp_bytewidth(region->bmp8);
    p=bmp_rowptr_from_top(region->bmp8,0);
    nrmid=rsum=0;
    if (left_to_right)
        {
        cstart=textrow->c2;
        cend=textrow->c1-1;
        cdir=-1;
        }
    else
        {
        cstart=textrow->c1;
        cend=textrow->c2+1;
        cdir=1;
        }
#if (WILLUSDEBUGX & 16)
fprintf(out,"   j     r0     r1     r2     r3\n");
#endif
    for (j=cstart;j!=cend;j+=cdir)
        {
        int r,rmid,dr,drmax;

// k2printf("j=%d\n",j);
        rmid=(rmin+rmax)/2;
// k2printf("   rmid=%d\n",rmid);
        drmax=textrow->r2+1-rmid > rmid-textrow->r1+1 ? textrow->r2+1-rmid : rmid-textrow->r1+1;
        /* Find dark region closest to center line */
        for (dr=0;dr<drmax;dr++)
            {
            if (rmid+dr<=textrow->r2 && p[(rmid+dr)*rowbytes+j]<region->bgcolor)
                break;
            if (rmid-dr>=textrow->r1 && p[(rmid-dr)*rowbytes+j]<region->bgcolor)
                {
                dr=-dr;
                break;
                }
            }
#if (WILLUSDEBUGX & 16)
fprintf(out,"    dr=%d/%d, rmid+dr=%d, rmin=%d, rmax=%d, nrmid=%d\n",dr,drmax,rmid+dr,rmin,rmax,nrmid);
#endif
        /* No dark detected or mark is outside hyphen region? */
        /* Termination criterion #1 */
        if (dr>=drmax || (nrmid>2 && (double)nrmid/textrow->lcheight>.1 
                               && (rmid+dr<rmin || rmid+dr>rmax)))
            {
            if (textrow->hyphen.ch>=0 && dr>=drmax)
                continue;
            if (nrmid>2 && (double)nrmid/textrow->lcheight > .35)
                {
                textrow->hyphen.ch = j-cdir;
                textrow->hyphen.r1 = rmin;
                textrow->hyphen.r2 = rmax;
                }
            if (dr<drmax)
                {
                textrow->hyphen.c2=j;
                break;
                }
            continue;
            }
        if (textrow->hyphen.ch>=0)
            {
            textrow->hyphen.c2=j;
            break;
            }
        nrmid++;
        rmid += dr;
        /* Dark spot is outside expected hyphen area */
        /*
        if (rmid<rmin || rmid>rmax)
            {
            if (nrmid>0)
                break;
            continue;
            }
        */
        for (r=rmid;r>=textrow->r1;r--)
            if (p[r*rowbytes+j]>=region->bgcolor)
                break;
        r1[j-textrow->c1]=r+1;
        r0[j-textrow->c1]=-1;
        if (r>=textrow->r1)
            {
            for (;r>=textrow->r1;r--)
                if (p[r*rowbytes+j]<region->bgcolor)
                    break;
            if (r>=textrow->r1)
                r0[j-textrow->c1]=r;
            }
        for (r=rmid;r<=textrow->r2;r++)
            if (p[r*rowbytes+j]>=region->bgcolor)
                break;
        r2[j-textrow->c1]=r-1;
        r3[j-textrow->c1]=-1;
        if (r<=textrow->r2)
            {
            for (;r<=textrow->r2;r++)
                if (p[r*rowbytes+j]<region->bgcolor)
                    break;
            if (r<=textrow->r2)
                r3[j-textrow->c1]=r;
            }
#if (WILLUSDEBUGX & 16)
fprintf(out," %4d  %4d  %4d  %4d  %4d\n",j,r0[j-textrow->c1],r1[j-textrow->c1],r2[j-textrow->c1],r3[j-textrow->c1]);
#endif
        if (textrow->hyphen.c2<0 && (r0[j-textrow->c1]>=0 || r3[j-textrow->c1]>=0))
            textrow->hyphen.c2=j;
        /* Termination criterion #2 */
        if (nrmid>2 && (double)nrmid/textrow->lcheight > .35
                && (r1[j-textrow->c1] > rmax || r2[j-textrow->c1] < rmin))
            {
            textrow->hyphen.ch = j-cdir;
            textrow->hyphen.r1 = rmin;
            textrow->hyphen.r2 = rmax;
            if (textrow->hyphen.c2<0)
                textrow->hyphen.c2=j;
#if (WILLUSDEBUGX & 16)
fprintf(out,"  Termination #2\n");
#endif
            break;
            }
        // rc=(r1[j-textrow->c1]+r2[j-textrow->c1])/2;
        /* DQ possible hyphen if r1/r2 out of range */
        if (nrmid>1)
           {
           /* Too far away from last values? */
           if ((double)(rmin-r1[j-textrow->c1])/textrow->lcheight > .1
               || (double)(r2[j-textrow->c1]-rmax)/textrow->lcheight > .1)
{
#if (WILLUSDEBUGX & 16)
fprintf(out,"  Too far from last values.\n");
#endif
               break;
}
           if ((double)nrmid/textrow->lcheight > .1 && nrmid>1)
               {
               if ((double)fabs(rmin-r1[j-textrow->c1])/textrow->lcheight > .1
                   || (double)(rmax-r2[j-textrow->c1])/textrow->lcheight > .1)
{
#if (WILLUSDEBUGX & 16)
fprintf(out,"  Too far from last values (2).\n");
#endif
                   break;
}
               }
           }
        if (nrmid==1 || r1[j-textrow->c1]<rmin)
            rmin=r1[j-textrow->c1];
        if (nrmid==1 || r2[j-textrow->c1]>rmax)
            rmax=r2[j-textrow->c1];
        if ((double)nrmid/textrow->lcheight > .1 && nrmid>1)
            {
            double rmean;

            /* Can't be too thick */
            if ((double)(rmax-rmin+1)/textrow->lcheight > .55
                    || (double)(rmax-rmin+1)/textrow->lcheight < .05)
{
#if (WILLUSDEBUGX & 16)
fprintf(out,"  Too thick or too thin:  rmax=%d, rmin=%d, lch=%d rat=%g (.05 - .55).\n",
rmax,rmin,textrow->lcheight,(double)(rmax-rmin+1)/textrow->lcheight);
#endif
                break;
}
            /* Must be reasonably well centered above baseline */
            /* v2.33 -- changed to 0.25 to 0.85 (used to be 0.35 to 0.85) */
            rmean=(double)(rmax+rmin)/2;
            if ((double)(textrow->rowbase-rmean)/textrow->lcheight < 0.25
                  || (double)(textrow->rowbase-rmean)/textrow->lcheight > 0.85)
{
#if (WILLUSDEBUGX & 16)
fprintf(out,"  Not well centered (1).\n");
fprintf(out,"      rowbase=%d\n",textrow->rowbase);
fprintf(out,"      lcheight=%d\n",textrow->lcheight);
fprintf(out,"      rmin=%d, rmax=%d, rmean=%g\n",rmin,rmax,rmean);
fprintf(out,"      (rbase-rmean)/lh=%g\n",(textrow->rowbase-rmean)/textrow->lcheight);
fprintf(out,"      (Needs to be between 0.25 and 0.85.)\n");
#endif
                break;
}
            if ((double)(textrow->rowbase-rmax)/textrow->lcheight < 0.2
                  || (double)(textrow->rowbase-rmin)/textrow->lcheight > 0.92)
{
#if (WILLUSDEBUGX & 16)
fprintf(out,"  Not well centered (2).\n");
#endif
                break;
}
            }
        }
#if (WILLUSDEBUGX & 16)
fprintf(out,"   ch=%d, c2=%d, r1=%d, r2=%d\n",textrow->hyphen.ch,textrow->hyphen.c2,textrow->hyphen.r1,textrow->hyphen.r2);
#endif
    /* More sanity checks--better to miss a hyphen than falsely detect it. */
    if (textrow->hyphen.ch>=0)
        {
        double ar;
        /* If it's only a hyphen, then it's probably actually a dash--don't detect it. */
        if (textrow->hyphen.c2<0)
{
#if (WILLUSDEBUGX & 16)
fprintf(out,"  Probably a dash (no preceding letter).\n");
#endif
            textrow->hyphen.ch = -1;
}
        /* Check aspect ratio */
        ar=(double)(textrow->hyphen.r2-textrow->hyphen.r1)/nrmid;
        if (ar<0.08 || ar > 0.75)
{
#if (WILLUSDEBUGX & 16)
fprintf(out,"  Bad aspect ratio = %g (s/b between .08 and .75).\n",ar);
#endif
            textrow->hyphen.ch = -1;
}
        }
    willus_dmem_free(27,(double **)&r0,funcname);
#if (WILLUSDEBUGX & 16)
if (textrow->hyphen.ch>=0)
{
k2printf("\n\n   GOT HYPHEN.\n\n");
fprintf(out,"  HYPHEN DETECTED.\n");
}
fclose(out);
k2printf("   Exiting bmpregion_hyphen_detect\n");
#endif
    }


/*
** Return average text height in all rows excluding rows that are too tall.
*/
int bmpregion_textheight(BMPREGION *region,K2PDFOPT_SETTINGS *k2settings,int i1,int i2)

    {
    int j,i,n1,textheight;

#if (WILLUSDEBUGX & 1)
k2printf("@bmpregion_textheight:  region=(%d,%d) - (%d,%d)\n",region->c1,region->r1,region->c2,region->r2);
k2printf("    nrows = %d\n",i2-i1+1);
#endif
    for (j=0;j<3;j++)
        {
        for (n1=textheight=0,i=i1;i<=i2;i++)
            {
            TEXTROW *textrow;
            double ar,rh;

            textrow=&region->textrows.textrow[i];
            if (textrow->c2<textrow->c1)
                ar = 100.;
            else
                ar = (double)(textrow->r2-textrow->r1+1)/(double)(textrow->c2-textrow->c1+1);
            rh = (double)(textrow->r2-textrow->r1+1)/region->dpi;
            if (j==2 || (j>=1 && rh<=k2settings->no_wrap_height_limit_inches)
                     || (j==0 && rh<=k2settings->no_wrap_height_limit_inches && ar<=k2settings->no_wrap_ar_limit))
                {
                textheight += textrow->rowbase - textrow->r1+1;
                n1++;
                }
            }
        if (n1>0)
            break;
        }
    return((int)((double)textheight/n1+.5));
    }


int bmpregion_is_centered(BMPREGION *region,K2PDFOPT_SETTINGS *k2settings,int i1,int i2)

    {
    int i,cc;
    int textheight;

#if (WILLUSDEBUGX & 1)
k2printf("@bmpregion_is_centered:  region=(%d,%d) - (%d,%d)\n",region->c1,region->r1,region->c2,region->r2);
k2printf("    nrows = %d\n",i2-i1+1);
#endif
    textheight = bmpregion_textheight(region,k2settings,i1,i2);

    /*
    ** Does region appear to be centered?
    */
    for (cc=0,i=i1;i<=i2;i++)
        {
        double indent1,indent2;

#if (WILLUSDEBUGX & 1)
k2printf("    tr[%d].c1,c2 = %d, %d\n",i,region->textrows.textrow[i].c1,region->textrows.textrow[i].c2);
#endif
        indent1 = (double)(region->textrows.textrow[i].c1-region->c1) / textheight;
        indent2 = (double)(region->c2 - region->textrows.textrow[i].c2) / textheight;
#if (WILLUSDEBUGX & 1)
k2printf("    tr[%d].indent1,2 = %g, %g\n",i,indent1,indent2);
#endif
        /* If only one line and it spans the entire region, call it centered */
        /* Sometimes this won't be the right thing to to. */
        if (i1==i2 && indent1<.5 && indent2<.5)
{
#if (WILLUSDEBUGX & 1)
/*
k2printf("    One line default to bigger region (%s).\n",region->textrows.centered?"not centered":"centered");
*/
#endif
            return(1);
}
        if (fabs(indent1-indent2) > 1.5)
{
#if (WILLUSDEBUGX & 1)
k2printf("    Region not centered.\n");
#endif
            return(0);
}
        if (indent1 > 1.0)
            cc++;
        }
#if (WILLUSDEBUGX & 1)
k2printf("Region centering:  i=%d, i2=%d, cc=%d, ntr=%d\n",i,i2,cc,i2-i1+1);
#endif
    if (cc > (i2-i1+1)/2)
{
#if (WILLUSDEBUGX & 1)
k2printf("    Region is centered (enough obviously centered lines).\n");
#endif
        return(1);
}
#if (WILLUSDEBUGX & 1)
k2printf("    Not centered (not enough obviously centered lines).\n");
#endif
    return(0);
    }


/*
**
** Searches the region for vertical break points and stores them into
** the TEXTROWS structure.
**
*/
void bmpregion_find_textrows(BMPREGION *region,K2PDFOPT_SETTINGS *k2settings,
                             int dynamic_aperture,int remove_small_rows,
                             int join_figure_captions)

    {
    static char *funcname="bmpregion_find_textrows";
    int nr,i,brc,brcmin,dtrc,trc,figrow,labelrow;
    int rhmin_pix,rhmean_pixels;
    BMPREGION *newregion,_newregion;
    TEXTROWS *textrows;
    int *rowthresh;
    double min_fig_height,max_fig_gap,max_label_height;

#if (WILLUSDEBUGX & 0x2)
k2printf("At bmpregion_find_textrows(dyn_aprt=%d, remove_small=%d, joinfigcaps=%d)\n",
dynamic_aperture,remove_small_rows,join_figure_captions);
#endif
    min_fig_height=k2settings->dst_min_figure_height_in;
    max_fig_gap=0.16;
    max_label_height=0.5;
    /* Trim region (calculate bounding box) */
    bmpregion_trim_margins(region,k2settings,k2settings->src_trim ? 0xf : 0);
    newregion=&_newregion;
    bmpregion_init(newregion);
    bmpregion_copy(newregion,region,0);
    textrows=&region->textrows;
    if (k2settings->debug)
        k2printf("@bmpregion_find_textrows:  (%d,%d) - (%d,%d)\n",
                region->c1,region->r1,region->c2,region->r2);
    nr=region->r2-region->r1+1;
    willus_dmem_alloc_warn(15,(void **)&rowthresh,sizeof(int)*nr,funcname,10);
    brcmin = k2settings->max_vertical_gap_inches*region->dpi;
    bmpregion_fill_row_threshold_array(region,k2settings,dynamic_aperture,rowthresh,&rhmean_pixels);
#if (WILLUSDEBUGX & 0x2)
{
static int count=0;
if (!count)
{
bmp_write(region->bmp,"bigbmp.png",stdout,100);
count++;
}
}
k2printf("rhmean=%d\n",rhmean_pixels);
{
FILE *f;
static int count=0;
f=fopen("rthresh.ep",count==0?"w":"a");
count++;
k2printf("rowcount=%p\n",newregion->rowcount);
for (i=newregion->r1;i<=newregion->r2;i++)
{
nprintf(f,"%d\n",rowthresh[i-newregion->r1]);
k2printf("rowthresh[%4d]=%4d\n",i,rowthresh[i-newregion->r1]);
if (newregion->rowcount!=NULL)
k2printf("rowcount [%4d]=%4d\n",i,newregion->rowcount[i]);
}
nprintf(f,"//nc\n");
fclose(f);
}
#endif
    /* Minimum text row height required (pixels) */
    rhmin_pix = rhmean_pixels/3;
    if (rhmin_pix < .04*region->dpi)
        rhmin_pix = .04*region->dpi;
    if (rhmin_pix > .13*region->dpi)
        rhmin_pix = .13*region->dpi;
    if (rhmin_pix < 1)
        rhmin_pix = 1;
#if (WILLUSDEBUGX & 0x2)
printf("rhmin_pix = %d\n",rhmin_pix);
printf("brcmin = %d\n",brcmin);
#endif
    /*
    for (rmax=region->r2;rmax>region->r1;rmax--)
        if (rowthresh[rmax-region->r1]>10)
            break;
    */
    /*
    ** Look for gaps between rows in the region so that it can be broken into
    ** multiple "rows".
    **
    ** brc = consecutive blank pixel rows
    ** trc = consecutive non-blank pixel rows
    ** dtrc = number of non blank pixel rows since last dump
    */
    textrows_clear(textrows);
    for (labelrow=figrow=-1,dtrc=trc=brc=0,i=region->r1;i<=region->r2+1;i++)
        {
#if (WILLUSDEBUGX & 0x2)
printf("i=%d, dtrc=%d (nonblank since last dump), trc=%d (cons nb), brc=%d (cons blnk)\n",
i,dtrc,trc,brc);
printf("    rowthresh[i]=%d\n",rowthresh[i-region->r1]);
#endif
        /* Does row have few enough black pixels to be considered blank? */
        if (i>region->r2 || rowthresh[i-region->r1]<=10) 
            {
#if (WILLUSDEBUGX & 0x2)
printf("    (Blank row.)\n");
#endif
            trc=0;
            brc++;
            /*
            ** Max allowed white space between rows = max_vertical_gap_inches
            */
            if (dtrc==0 && i<=region->r2)
                {
                if (brc > brcmin)
                    newregion->r1++;
                continue;
                }
            /*
            ** Big enough blank gap, so add one row / line
            */
            if (dtrc+brc >= rhmin_pix || i>region->r2)
                {
                int i0,iopt;
                double region_height_inches;
                double gap_inches;

                if (dtrc<region->dpi*0.02)
                    dtrc=region->dpi*0.02;
                if (dtrc<2)
                    dtrc=2;
                /* Look for more optimum point */
                if (i<=region->r2)
                    {
                    for (i0=iopt=i;i<=region->r2 && i-i0<dtrc;i++)
                        {
                        if (rowthresh[i-region->r1]<rowthresh[iopt-region->r1])
                            {
                            iopt=i;
                            if (rowthresh[i-region->r1]==0)
                                break;
                            }
                        if (rowthresh[i-region->r1]>100)
                            break;
                        }
                    /* If at end of region and haven't found perfect break, stay at end */
                    if (i>region->r2 && rowthresh[iopt-region->r1]>0)
                        i=region->r2;
                    else
                        i=iopt;
                    }
                newregion->r2=i-1;
                region_height_inches = (double)(newregion->r2-newregion->r1+1)/region->dpi;
#if (WILLUSDEBUGX & 0x2)
printf("    Optimum point = %d\n",i);
printf("    Region_height = %g in.\n",region_height_inches);
#endif

                /* Could this region be a figure? */
                if (join_figure_captions && i<=region->r2 
                        && figrow < 0 && region_height_inches >= min_fig_height)
                    {
#if (WILLUSDEBUGX & 0x2)
printf("    Region could be figure.\n");
#endif
                    /* If so, set figrow and don't process it yet. */
                    figrow = newregion->r1;
                    labelrow = -1;
                    newregion->r1=i;
                    dtrc=trc=0;
                    brc=1;
                    continue;
                    }
                /* Are we processing a figure? */
                if (figrow >= 0)
                    {
#if (WILLUSDEBUGX & 0x2)
printf("    Processing a figure (figrow=%d).\n",figrow);
#endif
                    /* Compute most recent gap */
                    if (labelrow>=0)
                        gap_inches = (double)(labelrow-newregion->r1)/region->dpi;
                    else
                        gap_inches = -1.;
                    /* If gap and region height are small enough, tack them on to the figure. */
                    if (region_height_inches < max_label_height && gap_inches>0. 
                                  && gap_inches<max_fig_gap)
                        newregion->r1=figrow;
                    else
                        {
                        /* Not small enough--dump the previous figure. */
#if (WILLUSDEBUGX & 0x2)
printf("    Dumping previous figure.\n");
#endif
                        newregion->r2=newregion->r1-1;
                        newregion->r1=figrow;
                        newregion->c1=region->c1;
                        newregion->c2=region->c2;
                        newregion->bbox.type=0;
                        bmpregion_calc_bbox(newregion,k2settings,1);
                        if (newregion->r2>newregion->r1)
{
                            textrows_add_bmpregion(textrows,newregion,REGION_TYPE_FIGURE);
/*
printf("1. textrow[%d] = figure.\n",textrows->n-1);
*/
}
                        if (i<=region->r2 && gap_inches>0. && gap_inches<max_fig_gap)
                            {
                            /* This new region might be a figure--set it as the new figure */
                            /* and don't dump it yet.                                      */
                            figrow = newregion->r2+1;
                            labelrow = -1;
                            newregion->r1=i;
                            dtrc=trc=0;
                            brc=1;
                            continue;
                            }
                        else
                            {
                            newregion->r1=newregion->r2+1;
                            newregion->r2=i-1;
                            }
                        }
                    /* Cancel figure processing */
                    figrow=-1;
                    labelrow=-1;
                    }
                /*
                if (newregion->r2 >= rmax)
                    i=newregion->r2=region->r2;
                */
                newregion->c1=region->c1;
                newregion->c2=region->c2;
                newregion->bbox.type=0;
                bmpregion_calc_bbox(newregion,k2settings,1);
#if (WILLUSDEBUGX & 0x2)
printf("    Adding bmpregion: (%d,%d)-(%d,%d).\n",newregion->c1,newregion->r1,newregion->c2,newregion->r2);
#endif
                if (newregion->r2>newregion->r1)
                    textrows_add_bmpregion(textrows,newregion,REGION_TYPE_TEXTLINE);
#if (WILLUSDEBUGX & 0x2)
printf("        Done adding bmpregion: (%d,%d)-(%d,%d).\n",newregion->c1,newregion->r1,newregion->c2,newregion->r2);
#endif
                newregion->r1=i;
                dtrc=trc=0;
                brc=1;
                }
            }
        else
            {
#if (WILLUSDEBUGX & 0x2)
printf("    (Non-blank row.)\n");
#endif
            if (figrow>=0 && labelrow<0)
                labelrow=i;
            dtrc++;
            trc++;
            brc=0;
            }
        }

    /* Set rat=0 for all entries */
    for (i=0;i<textrows->n;i++)
        textrows->textrow[i].rat=0.;

    /* Compute gaps between rows and row heights */
    textrows_compute_row_gaps(textrows,region->r2);

#if (WILLUSDEBUGX & 0x2)
    {
    k2printf("FIRST PASS IN FIND TEXT ROWS:\n");
    int i;
    for (i=0;i<textrows->n;i++)
        {
        TEXTROW *textrow;
        textrow=&textrows->textrow[i];
        k2printf("   rowheight[%03d] = (%04d,%04d)-(%04d,%04d)\n",i,textrow->c1,textrow->r1,textrow->c2,textrow->r2);
        }
    }
#endif

#if (WILLUSDEBUGX & 0x2)
printf("CC\n");
#endif
    /* Look for double-height and triple-height rows and break them up */
    /* if conditions seem right.                                       */
    textrows_find_doubles(textrows,rowthresh,region,k2settings,3,dynamic_aperture);

#if (WILLUSDEBUGX & 0x2)
printf("DD\n");
#endif
    /* Compute gaps between rows and row heights again */
    textrows_compute_row_gaps(textrows,region->r2);

    /* Remove rows with text height that seems to be too small */
    if (remove_small_rows)
        {
        /* textrows_remove_small_rows needs types determined */
        for (i=0;i<textrows->n;i++)
            textrow_determine_type(region,k2settings,i);
        textrows_remove_small_rows(textrows,k2settings,0.25,0.5,region,-1.0);
        }

    /* Compute gaps between rows and row heights again */
    textrows_compute_row_gaps(textrows,region->r2);

    if (textrows->n>1)
        region->bbox.type = REGION_TYPE_MULTILINE;
    else
        region->bbox.type = REGION_TYPE_UNDETERMINED;

    /* Classify as text rows or figures (could be smarter at some point) */
    for (i=0;i<textrows->n;i++)
        textrow_determine_type(region,k2settings,i);

#if (WILLUSDEBUGX & 0x2)
    {
    k2printf("FINAL PASS IN FIND TEXT ROWS:\n");
    int i;
    for (i=0;i<textrows->n;i++)
        {
        TEXTROW *textrow;
        textrow=&textrows->textrow[i];
        k2printf("   rowheight[%03d] = (%04d,%04d)-(%04d,%04d)\n",i,textrow->c1,textrow->r1,textrow->c2,textrow->r2);
        }
    }
#endif
#if (WILLUSDEBUGX & 0x2)
{
int i;
k2printf("rowcount=%p\n",region->rowcount);
for (i=region->r1;i<=region->r2;i++)
{
if (region->rowcount!=NULL)
k2printf("rowcount [%4d]=%4d\n",i,region->rowcount[i]);
}
}
#endif

    willus_dmem_free(15,(double **)&rowthresh,funcname);
    bmpregion_free(newregion);
    }


/*
** rowthresh must be dimensioned to region->r2-region->r1+1
*/
void bmpregion_fill_row_threshold_array(BMPREGION *region,K2PDFOPT_SETTINGS *k2settings,
                                        int dynamic_aperture,int *rowthresh,int *rhmean_pixels)

    {
    int aperturemax,aperture,dtrc,ntr,i;

    aperturemax = (int)(region->dpi/72.+.5);
    if (aperturemax < 2)
        aperturemax = 2;
    aperture=aperturemax;
    /* v2.33 -- don't use column_row_gap_height_in for aperture. */
    /* aperture=(int)(region->dpi*k2settings->column_row_gap_height_in+.5); */
/*
for (i=region->r1;i<=region->r2;i++)
k2printf("rowcount[%d]=%d\n",i,region->rowcount[i]);
*/
    (*rhmean_pixels)=0; // Mean text row height
    for (ntr=dtrc=0,i=region->r1;i<=region->r2;i++)
        {
        int ii,i1,i2,sum,pt;

        if (dynamic_aperture)
            {
            aperture=(int)(dtrc/13.7+.5);
            if (aperture > aperturemax)
                aperture=aperturemax;
            if (aperture < 2)
                aperture=2;
            }
        i1=i-aperture/2;
        i2=i1+aperture-1;
        if (i1<region->r1)
            i1=region->r1;
        if (i2>region->r2)
            i2=region->r2;
        pt=(int)((i2-i1+1)*k2settings->gtr_in*region->dpi+.5); /* pixel count threshold */
        if (pt<1)
            pt=1;
        /* Sum over row aperture */
        for (sum=0,ii=i1;ii<=i2;sum+=region->rowcount[ii],ii++);
        /* Does row have few enough black pixels to be considered blank? */
        if ((rowthresh[i-region->r1]=10*sum/pt)<=40)
            {
            if (dtrc>0)
                {
                (*rhmean_pixels) = (*rhmean_pixels) + dtrc;
                ntr++;
                }
            dtrc=0;
            }
        else
            dtrc++;
        }
    if (dtrc>0)
        {
        (*rhmean_pixels) = (*rhmean_pixels) + dtrc;
        ntr++;
        }
    if (ntr>0)
        (*rhmean_pixels) = (*rhmean_pixels) / ntr;
    }


#if (WILLUSDEBUGX & 0x1000)
static int rn=0;
#endif
/*
**
** Break row of text into words.
**
** Input:  region, expected to be one row of text.
**
** Output: region->textrows (treated as textwords) structure filled in with
**         individual word regions.
**
*/
void bmpregion_one_row_find_textwords(BMPREGION *region,K2PDFOPT_SETTINGS *k2settings,
                                      int add_to_dbase)

    {
    int i,i0,dr,lcheight,gap_thresh,display_width,mgt;
    BMPREGION *newregion,_newregion;
    TEXTWORDS _textwords,*textwords;
    int *gw,*copt,ngaps;
    int width;
    double multiplier;
    static char *funcname="bmpregion_one_row_find_textwords";

#if (WILLUSDEBUGX & 0x1000)
printf("@bmpregion_one_row_find_textwords\n");
printf("    (%d,%d)-(%d,%d)\n",region->c1,region->r1,region->c2,region->r2);
printf("    wordspacing=%g\n",k2settings->word_spacing);
#endif
#if (WILLUSDEBUGX & 0x1000)
{
char filename[MAXFILENAMELEN];
rn++;
/*
if (rn==3)
exit(10);
*/
sprintf(filename,"reg%03d.png",rn);
bmpregion_write(region,filename);
printf("Region #%d\n",rn);
}
#endif
    if (k2settings->debug)
        k2printf("@bmpregion_one_row_find_textwords(%d,%d)-(%d,%d)\n",
               region->c1,region->r1,region->c2,region->r2);
    if (region->bbox.type==REGION_TYPE_MULTIWORD)
        {
        k2printf(ANSI_RED "Internal error at bmpregion_one_row_find_textwords.  Already parsed.\n"
                 "Please report error.\n");
        exit(20);
        }
    if (region->textrows.n!=0)
        {
        k2printf(ANSI_RED "Internal error at bmpregion_one_row_find_textwords.  nrows=%d (s/b 0).\n"
                 "Please report error.\n",region->textrows.n);
        exit(20);
        }
    textwords=&_textwords;
    textwords_init(textwords);
    newregion=&_newregion;
    bmpregion_init(newregion);
    bmpregion_copy(newregion,region,0);
    /*
    bmpregion_trim_margins(newregion,k2settings,0x1f);
    region->lcheight=newregion->lcheight;
    region->capheight=newregion->capheight;
    region->rowbase=newregion->rowbase;
    region->h5050=newregion->h5050;
    */
    /* Default is one word--the bounding box */
    region->bbox.type=REGION_TYPE_MULTIWORD;
    textrows_clear(&region->textrows);
    textrows_add_textrow(&region->textrows,&region->bbox);

    /* Trim columns to text row */
    bmpregion_trim_margins(newregion,k2settings,0x13);
    if (newregion->c2-newregion->c1+1<6)
        {
        bmpregion_free(newregion);
        textwords_free(textwords);
        return;
        }
    /*
    ** Look for "space-sized" gaps, i.e. gaps that would occur between words.
    ** Use this as pixel counting aperture.
    */
    dr=newregion->bbox.lcheight;
    if (dr<1)
        dr=1;

#if (WILLUSDEBUGX & 0x1000)
    printf("dr=%d\n",dr);
    {
    static int count=0;
    char fname[MAXFILENAMELEN];
    sprintf(fname,"row%04d.png",count+1);
    bmpregion_write(newregion,fname);
#endif

    /*
    ** Find sizes of gaps and look for bi-modal distribution
    ** (e.g. small gaps between characters and large gaps between words)
    ** gw[] gets gaps in pixels.
    ** copt[] gets the center position of the gap in pixels.
    */
    ngaps=0;
    width=newregion->c2-newregion->c1+1;
    willus_dmem_alloc_warn(31,(void **)&gw,sizeof(int)*width*2,funcname,10);
    copt=&gw[width];
    bmpregion_count_text_row_pixels(newregion,gw,copt,&ngaps,k2settings);

    /* Sort by gap size */
    sortxyi(gw,copt,ngaps);
    array_flipi(gw,ngaps);
    array_flipi(copt,ngaps);
    gap_thresh = get_word_gap_threshold(copt,gw,ngaps,dr,newregion->c2-newregion->c1+1,
                                        newregion,k2settings);
    mgt = (int)(fabs(k2settings->word_spacing)*dr+.5);
    /* Minimum word gap = fabs(word_spacing) */
    if (k2settings->word_spacing<0 && gap_thresh<mgt)
        gap_thresh = mgt;
#if (WILLUSDEBUGX & 0x1000)
aprintf("thresh = %5.3f" ANSI_NORMAL "\n",(double)gap_thresh/dr);
#endif
    for (i=0;i<ngaps;i++)
        if (gw[i]<gap_thresh)
            break;
    /*
    ngaps=i;
    */
#if (WILLUSDEBUGX & 0x1000)
    printf("ngaps = %d\n",i);
    }
#endif
    /* Re-sort by position */
    sortxyi(copt,gw,ngaps);
#if (WILLUSDEBUGX & 0x1000)
for (i=0;i<ngaps-1;i++)
printf("    gw[%2d]=(%4d,%2d)\n",i,copt[i],gw[i]);
printf("Gap threshold = %d\n",gap_thresh);
#endif
    display_width = k2settings->max_region_width_inches*k2settings->src_dpi;
    for (i0=-1,i=0,multiplier=1.0;i<=ngaps;i++)
        {
        int c1,c2;
        BMPREGION xregion;

        if (i<ngaps && gw[i]<gap_thresh*multiplier)
            continue;
        c1=(i0<0) ? newregion->c1 : copt[i0]+1;
        c2=(i==ngaps) ? newregion->c2 : copt[i];
        if (c2-c1<2)
            continue;
        /* Is word too long for display? (Only checked if in automatic spacing mode.) */
        if (k2settings->word_spacing<0 && (c2-c1+1 > display_width))
            {
            if (i-i0>1)
                {
#if (WILLUSDEBUGX & 0x1000)
if (multiplier > .95)
printf("Subdividing gap:  %d - %d (del=%d)\n",c1,c2,c2-c1+1);
#endif
                multiplier *= 0.9;
                if (multiplier>.05 && gap_thresh*multiplier >= mgt)
                    {
                    i=i0;
                    continue;
                    }
                }
            }
#if (WILLUSDEBUGX & 0x1000)
if (multiplier<0.95)
printf("    Subdivided:  New len = %d pixels, multiplier=%g.\n",c2-c1+1,multiplier);
#endif
        bmpregion_init(&xregion);
        bmpregion_copy(&xregion,newregion,0);
        xregion.c1=c1;
        xregion.c2=c2;
        xregion.bbox.type=0;
        bmpregion_calc_bbox(&xregion,k2settings,1);
        textwords_add_bmpregion(textwords,&xregion,REGION_TYPE_WORD);
        bmpregion_free(&xregion);
        /* Mark as done up to this gap. */
        i0=i;
        /* Reset to nominal gap threshold */
        multiplier=1.0;
        }
    /* End of scope which includes gw[] and copt[] arrays */
    willus_dmem_free(31,(double **)&gw,funcname);

    textwords_compute_col_gaps(textwords,newregion->c2);
    lcheight = newregion->bbox.lcheight;
    bmpregion_free(newregion);

    /* Remove small gaps */
    if (k2settings->word_spacing>=0.)
        {
        double median_gap;
        textwords_add_word_gaps(add_to_dbase ? textwords : NULL,lcheight,&median_gap,
                                (double)gap_thresh/dr);
        textwords_remove_small_col_gaps(textwords,lcheight,median_gap/1.9,(double)gap_thresh/dr);
        }

    /* If we found words, copy them to BMPREGION structure */
    if (textwords->n > 0)
        {
        textwords_clear(&region->textrows);
        for (i=0;i<textwords->n;i++)
            textwords_add_textword(&region->textrows,&textwords->textrow[i]);
        }
    textwords_free(textwords);
#if (WILLUSDEBUGX & 0x1000)
printf("End bmpregion_one_row_find_textwords.\n");
#endif
    }


void textrow_echo(TEXTROW *textrow,FILE *out)

    {
    fprintf(out,"Text row info:\n"
                "    (%d,%d) - (%d,%d)\n",textrow->c1,textrow->r1,textrow->c2,textrow->r2);
    fprintf(out,"    rowbase=%d\n",textrow->rowbase);
    fprintf(out,"    gap=%d\n",textrow->gap);
    fprintf(out,"    gapblank=%d\n",textrow->gapblank);
    fprintf(out,"    rowheight=%d\n",textrow->rowheight);
    fprintf(out,"    capheight=%d\n",textrow->capheight);
    fprintf(out,"    h5050=%d\n",textrow->h5050);
    fprintf(out,"    lcheight=%d\n",textrow->lcheight);
    fprintf(out,"    type=%d\n",textrow->type);
    fprintf(out,"    rat=%g\n",textrow->rat);
    }


static void bmpregion_count_text_row_pixels(BMPREGION *region,int *gw,int *copt,int *ngaps,
                                            K2PDFOPT_SETTINGS *k2settings)

    {
    int nc,dr,mingap,*bp;
    static char *funcname="bmpregion_count_text_row_pixels";

    /*
    ** Find places where there are gaps (store in bp array)
    ** Could do this more intelligently--maybe calculate a histogram?
    */
    nc=region->c2-region->c1+1;
    willus_dmem_alloc_warn(18,(void **)&bp,sizeof(int)*nc,funcname,10);
    memset(bp,0,nc*sizeof(int));
    /*
    ** Look for "space-sized" gaps, i.e. gaps that would occur between words.
    ** Use this as pixel counting aperture.
    */
    dr=region->bbox.lcheight;
    if (dr<1)
        dr=1;
    /*
    ** v2.20:  Err on small value for mingap now.  The auto-spacing version
    **         for KO Reader used to have a special algorithm for calculating
    **         mingap, but using a very small value seems to work fine for the
    **         the latest word gap detection algorithm, regardless of whether
    **         the alphabet is Western or CJK.
    */
    mingap = dr*.02;
    if (mingap < 2)
        mingap = 2;

    if (k2settings->src_left_to_right)
        {
        int i;

#if (WILLUSDEBUGX & 0x1000)
FILE *xx;
static int count=0;
xx=fopen("rowgaps.ep",count==0?"w":"a");
count++;
nprintf(xx,"/sa l \"reg %d\" 1\n",rn);
#endif
        for (i=region->c1;i<=region->c2;i++)
            {
            int i1,i2,pt,sum,ii;

            i1=i-mingap/2;
            i2=i1+mingap-1;
            if (i1<region->c1)
                i1=region->c1;
            if (i2>region->c2)
                i2=region->c2;
            pt=(int)((i2-i1+1)*k2settings->gtw_in*region->dpi+.5);
            if (pt<1)
                pt=1;
            for (sum=0,ii=i1;ii<=i2;sum+=region->colcount[ii],ii++);
            bp[i-region->c1]=10*sum/pt;
#if (WILLUSDEBUGX & 0x1000)
nprintf(xx,"%.1f\n",(double)bp[i-region->c1]);
#endif
            }
#if (WILLUSDEBUGX & 0x1000)
nprintf(xx,"//nc\n");
if (xx!=NULL)
fclose(xx);
#endif
        }
    else
        {
        int i;

        for (i=region->c2;i>=region->c1;i--)
            {
            int i1,i2,pt,sum,ii;

            i1=i-mingap/2;
            i2=i1+mingap-1;
            if (i1<region->c1)
                i1=region->c1;
            if (i2>region->c2)
                i2=region->c2;
            pt=(int)((i2-i1+1)*k2settings->gtw_in*region->dpi+.5);
            if (pt<1)
                pt=1;
            for (sum=0,ii=i1;ii<=i2;sum+=region->colcount[ii],ii++);
            bp[i-region->c1]=10*sum/pt;
            }
        }
#if (WILLUSDEBUGX & 4)
if (region->r1 > 3699 && region->r1<3750)
{
static int a=0;
int i;
FILE *f;
f=fopen("outbp.ep",a==0?"w":"a");
a++;
fprintf(f,"/sa l \"(%d,%d)-(%d,%d) lch=%d\" 2\n",region->c1,region->r1,region->c2,region->r2,region->bbox.lcheight);
for (i=0;i<nc;i++)
fprintf(f,"%d\n",bp[i]);
fprintf(f,"//nc\n");
fclose(f);
}
#endif
    bmpregion_find_gaps(region,bp,gw,copt,ngaps);
    willus_dmem_free(18,(double **)&bp,funcname);
    }


static void bmpregion_find_gaps(BMPREGION *region,int *bp,int *gw,int *copt,int *ngaps)

    {
    int thlow,thhigh,col0,dr;
                             
    thlow=10;
    thhigh=20;
    dr=region->bbox.lcheight;
    if (dr<1)
        dr=1;
    /*
    ** Find sizes of gaps and look for bi-modal distribution
    ** (e.g. small gaps between characters and large gaps between words)
    ** gw[] gets gaps in pixels.
    ** copt[] gets the center position of the gap in pixels.
    ** dgap[i] = gapwidth[i]-gapwidth[i-1] after gapwidth[] gets sorted
    ** gapcount[i] = i (before sorting)
    */
    (*ngaps)=0;
    /* Find gaps between text (letters and words) and store in gw[] and copt[] */
    for (col0=region->c1;col0<=region->c2;col0++)
        {
        int copt0,c0;

        for (;col0<=region->c2;col0++)
            if (bp[col0-region->c1]>=thhigh)
                break;
        if (col0>region->c2)
            break;
        for (col0++;col0<=region->c2;col0++)
            if (bp[col0-region->c1]<thlow)
                break;
        if (col0 >= region->c2)
            break;
        /* 2*dr was dr before v2.20 */
        for (copt0=c0=col0;col0<=region->c2  && col0-c0<=2*dr;col0++)
            {
            if (bp[col0-region->c1] <  bp[copt0-region->c1])
                copt0=col0;
            if (bp[col0-region->c1] > thhigh)
                break;
            }
        if (col0>region->c2)
            break;
        if (copt0>region->c2)
            copt0=region->c2;
        gw[(*ngaps)]=col0-c0;
        copt[(*ngaps)]=copt0;
        (*ngaps)=(*ngaps)+1;
        col0=copt0;
        if (copt0==region->c2)
            break;
        }
    }


/*
** Given an array of pixel gap widths (gw[]) and gap positions (copt[]) between letters
** and words in a text row, determine a threshold-word-gap width in pixels.  Gaps at this
** width or higher will be considered gaps between words (eligible places to split a
** text row for text re-flow).
** 
** NOTE!  gw[] and copt[] arrays must be sorted, in descending order, by values in gw[]
**
** dr = height of lowercase 'o' in pixels
** row_width = width of text row in pixels
**
** ngaps must be >=2
**
** v2.20
**
*/
static int get_word_gap_threshold(int *copt,int *gw,int ngaps,int dr,int row_width,
                                  BMPREGION *region,K2PDFOPT_SETTINGS *k2settings)

    {
    int i,gt,ibest;
    int *dgap,*gapcount;
    double expected,bestpos;
    static char *funcname="get_word_gap_threshold";
    int display_width;

#if (WILLUSDEBUGX & 0x01000)
printf("@get_word_gap_threshold, ngaps=%d, dr=%d\n",ngaps,dr);
#endif
    if (ngaps<=0)
        return((int)(fabs(k2settings->word_spacing)*dr+.5));
    /*
    ** Compute the expected number of word gaps in the text row based
    ** on a typical word length being ~ 6 * dr.
    **
    ** where dr = height of lowercase 'o' in pixels.
    */
    expected=(double)row_width/(6*dr)-1.;
    /*
    ** Text rows longer than display width either need to be wrapped or shrunk
    */
    display_width = k2settings->max_region_width_inches*k2settings->src_dpi;
#if (WILLUSDEBUGX & 0x01000)
    if (ngaps>0)
        {
        FILE *out;
        static int c2=0;

        out=fopen("rowgaps2.ep",c2==0?"w":"a");
        c2++;
        /* nprintf(out,"/sa l \"reg %d, row %d, len=%d, lcheight=%d\" 2\n",rn,count+1,row_width,dr); */
        for (i=0;i<ngaps;i++)
            nprintf(out,"%g %g\n",(double)(i+1)/expected,(double)gw[i]/dr);
        nprintf(out,"//nc\n");
        if (out!=NULL)
            fclose(out);
        }
#endif
    if (expected<=0. || (expected<1.5 && (double)gw[0]/dr<.2))
        return(gw[ngaps-1]+.1); /* No gaps */
    /* If ngaps==1, use historicals?? */
    if (k2settings->word_spacing>=0. || ngaps<2)
        return((int)(fabs(k2settings->word_spacing)*dr+.5));
    if (expected<0.1)
        expected=0.1;
    willus_dmem_alloc_warn(36,(void **)&dgap,sizeof(int)*ngaps*2,funcname,10);
    gapcount=&dgap[ngaps];

    for (i=0;i<ngaps-1;i++)
        {
        dgap[i]=gw[i]-gw[i+1];
        gapcount[i]=i+1;
        }
    sortxyi(dgap,gapcount,ngaps-1);
    array_flipi(dgap,ngaps-1);
    array_flipi(gapcount,ngaps-1);
    /*
    ** Check the three largest changes to the gap size--we expect that there
    ** should be a natural bi-modal gap size distribution made up of gaps between
    ** letters and gaps between words.  So the distribution of gap sizes should
    ** have a natural break in it. -- This may not work for all languages, though,
    ** particularly symbol languages like Chinese.
    **
    */
    gt=-1;
#if (WILLUSDEBUGX & 0x1000)
for (i=0;i<ngaps-1;i++)
printf("    gw[%2d]=(%4d,%2d); dgap[%2d]=%2d, gapcount[%2d]=%4d\n",i,copt[i],gw[i],i,dgap[i],i,gapcount[i]);
#endif
    /* First look for best-centered large gap change */
    ibest = -1;
    bestpos = -1.;
    for (i=0;i<ngaps-1;i++)
        {
        double pos;
        
#if (WILLUSDEBUGX & 0x1000)
printf("i=%d/%d, dgap=%d, gapcount=%d\n",i,ngaps-1,dgap[i],gapcount[i]);
#endif
        if ((double)dgap[i]/dr < 0.1)
            break;
        if (i>0 && (double)gw[gapcount[i]]/gw[gapcount[i-1]] > 0.6)
            continue;
        pos = (double)gapcount[i]/expected;
        if (bestpos<0. || fabs(pos-1.0) < fabs(bestpos-1.0))
            {
            bestpos=pos;
            ibest=i;
            }
        }
#if (WILLUSDEBUGX & 0x1000)
printf("Done loop checkinf for best-centered large gap. ibest=%d\n",ibest);
#endif
    if (ibest >= 0)
        {
        gt=(gw[gapcount[ibest]]+gw[gapcount[ibest]-1])/2;
#if (WILLUSDEBUGX & 0x1000)
aprintf(ANSI_GREEN "ibest=%d, gt_init=%d, ",ibest,gt);
#endif
        }
    else
        {
        /* Look for largest gap change that's in the right ball park */
        for (i=0;i<ngaps-1;i++)
            {
            /*
            ** Change in gap sizes has to be at least 0.07 x dr
            */
#if (WILLUSDEBUGX & 0x1000)
printf("ngaps=%d, dgap[%d]/%d = %g\n",ngaps,i,dr,(double)dgap[i]/dr);
printf("   expected = %g\n",expected);
printf("   gapcount[%d]/expected = %g\n",i,gapcount[i]/expected);
#endif
            if ((double)dgap[i]/dr < 0.07)
                break;
            if (i==0 && ngaps<=2)
                {
                gt=(gw[0]+gw[1])/2;
                break;
                }
            /*
            ** If this change in gap sizes is significantly larger than any
            ** others, it's probably the right one.
            ** Or if we get about the right number of word gaps compared to
            ** what we expect.
            */
            if ((i==0 && (double)dgap[i+1]/dgap[i] < 0.6)
                 || (gapcount[i]/expected > 0.3 && gapcount[i]/expected < 3.5))
                {
                gt=(gw[gapcount[i]]+gw[gapcount[i]-1])/2;
#if (WILLUSDEBUGX & 0x1000)
aprintf(ANSI_GREEN);
#endif
                break;
                }
            }
        }
#if (WILLUSDEBUGX & 0x1000)
printf("Past first cut analysis.  gt = %d\n",gt);
#endif
    /*
    ** No obvious break point in the gap spacings found?  (gt < 0)
    */
    if (gt<0)
        {
        /* Not a very long row -- lean towards not breaking it */
        if (expected < 3.5 && row_width <= display_width)
            {
            if ((double)gw[0]/dr < 0.15)
                gt = gw[0]+.1; /* Don't allow breaks */
            else
                {
                i=(int)(2.0*expected+0.5);
                if (i>ngaps-1)
                    i=ngaps-1;
                gt = gw[i]+0.1*dr;
                }
#if (WILLUSDEBUGX & 0x1000)
aprintf(ANSI_MAGENTA "short row: ");
#endif
            }
        else
            {
            /* Long row--we should pick a gap size so we can break it up */
            i=(int)(.35*expected+0.5);
            if (i>ngaps-1)
                i=ngaps-1;
            gt=gw[i];
            if (gt > 0.4*dr)
                gt /= 2;
            else
                gt -= 0.1*dr;
            if (gt<0)
                gt=0;
#if (WILLUSDEBUGX & 0x1000)
aprintf(ANSI_YELLOW "long row (dw=%d): ",display_width);
#endif
            }
        }
/*
** This part is done later by the calling function now
*/
#ifdef COMMENT
    if (gt>0)
        {
        int gt0;

        gt0=gt;
        /* Make sure no word is longer than display width if possible */
        sortxyi(copt,gw,ngaps);
        while (word_longer_than(gt,gw,copt,ngaps,display_width,region))
            gt--;
        sortxyi(gw,copt,ngaps);
        array_flipi(gw,ngaps);
        array_flipi(copt,ngaps);
#if (WILLUSDEBUGX & 0x1000)
if (gt<gt0)
aprintf("DECREMENT FROM %d TO %d\n",gt0,gt);
#endif
        }
#endif /* COMMENT */
    willus_dmem_free(36,(double **)&dgap,funcname);
#if (WILLUSDEBUGX & 0x1000)
printf("Done get_word_gap_threshold, gt=%d.\n",gt);
#endif
    return(gt);
    }


#ifdef COMMENT
/*
** Returns NZ if longest word length is > display_length
** copt[] and gw[] arrays must be sorted by copt[] values.
*/
static int word_longer_than(int gap_thresh,int *gw,int *copt,int ngaps,int display_width,
                            BMPREGION *region)

   {
   int i,i0;

   for (i0=region->c1,i=0;i<=ngaps;i++)
       {
       int c;

       c=(i==ngaps)?region->c2:copt[i];
       if (i<ngaps && gw[i] < gap_thresh)
           continue;
/*
#if (WILLUSDEBUGX & 0x1000)
aprintf("        gt=%d, wlen=%d (dl=%d)\n",gap_thresh,c-i0,display_width);
#endif
*/
       if (c-i0 > display_width)
           return(1);
       i0=c;
       }
    return(0);
    }
#endif


void bmpregion_whiteout(BMPREGION *dstregion,BMPREGION *croppedregion)

    {
    if (dstregion->bmp!=NULL)
        bmp_draw_filled_rect(dstregion->bmp,croppedregion->c1,croppedregion->r1,
                                            croppedregion->c2,croppedregion->r2,
                                            255,255,255);
    if (dstregion->bmp8!=NULL && dstregion->bmp8!=dstregion->bmp)
        bmp_draw_filled_rect(dstregion->bmp8,croppedregion->c1,croppedregion->r1,
                                             croppedregion->c2,croppedregion->r2,
                                             255,255,255);
    }


void bmpregion_local_pagebreakmarkers(BMPREGION *region,int left_to_right,int whitethresh)

    {
    int i,c1,c2;

    if (region->k2pagebreakmarks==NULL)
        return;
    c1=region->c1;
    c2=region->c2;
    if (left_to_right)
        c1 -= region->dpi;
    else
        c2 += region->dpi;
    for (i=0;i<region->k2pagebreakmarks->n;i++)
        {
        K2PAGEBREAKMARK *mark;

        mark=&region->k2pagebreakmarks->k2pagebreakmark[i];
        if (mark->col < c1 || mark->col > c2)
            {
            mark->type = -1;
            continue;
            }
        if (mark->row < region->r2 || bmpregion_clean_to_row(region,mark->row,whitethresh))
            mark->row -= region->r1;
        else
            mark->type = -1;
        }
    }


/*
** This should be smarter--should have a generic function that does this using
** same logic as trim_to().
*/
int bmpregion_clean_to_row(BMPREGION *region,int row,int whitethresh)

    {
    int i,max,pixwidth;

    max=(int)(.01*region->dpi+.5);
    if (max<1)
        max=1;
    pixwidth=region->c2-region->c1+1;
    for (i=region->r2+1;i<row;i++)
        {
        unsigned char *p;
        int pc,j;

        pc=0;
        p=bmp_rowptr_from_top(region->bmp8,i)+region->c1;
        for (j=0;j<pixwidth;j++,p++)
            if (p[0] < whitethresh)
                pc++;
        if (pc >= max)
            return(0);
        }
    return(1);
    }
