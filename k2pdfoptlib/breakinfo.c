/*
** breakinfo.c    Functions to find breaks in pages and rows of text.
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

static void breakinfo_compute_row_gaps(BREAKINFO *breakinfo,int r2);
static void breakinfo_compute_col_gaps(BREAKINFO *breakinfo,int c2);
static void breakinfo_remove_small_col_gaps(BREAKINFO *breakinfo,int lcheight,double mingap,
                                            double word_spacing);
static void textrow_assign_bmpregion(TEXTROW *textrow,BMPREGION *region);

#if (WILLUSDEBUGX & 6)
void breakinfo_echo(BREAKINFO *breakinfo)

    {
    int i;
    printf("@breakinfo_echo...\n");
    for (i=0;i<breakinfo->n;i++)
        printf("    %2d.  r1=%4d, rowbase=%4d, r2=%4d, c1=%4d, c2=%4d\n",
             i+1,breakinfo->textrow[i].r1,
             breakinfo->textrow[i].rowbase,
             breakinfo->textrow[i].r2,
             breakinfo->textrow[i].c1,
             breakinfo->textrow[i].c2);
    }
#endif


static void breakinfo_compute_row_gaps(BREAKINFO *breakinfo,int r2)

    {
    int i,n;

    n=breakinfo->n;
    if (n<=0)
        return;
    breakinfo->textrow[0].rowheight = breakinfo->textrow[0].r2 - breakinfo->textrow[0].r1;
    for (i=0;i<n-1;i++)
        breakinfo->textrow[i].gap = breakinfo->textrow[i+1].r1 - breakinfo->textrow[i].rowbase - 1;
        /*
        breakinfo->textrow[i].rowheight = breakinfo->textrow[i+1].r1 - breakinfo->textrow[i].r1;
        */
    for (i=1;i<n;i++)
        breakinfo->textrow[i].rowheight = breakinfo->textrow[i].rowbase 
                                            - breakinfo->textrow[i-1].rowbase;
    breakinfo->textrow[n-1].gap = r2 - breakinfo->textrow[n-1].rowbase;
    }


static void breakinfo_compute_col_gaps(BREAKINFO *breakinfo,int c2)

    {
    int i,n;

    n=breakinfo->n;
    if (n<=0)
        return;
    for (i=0;i<n-1;i++)
        {
        breakinfo->textrow[i].gap = breakinfo->textrow[i+1].c1 - breakinfo->textrow[i].c2 - 1;
        breakinfo->textrow[i].rowheight = breakinfo->textrow[i+1].c1 - breakinfo->textrow[i].c1;
        }
    breakinfo->textrow[n-1].gap = c2 - breakinfo->textrow[n-1].c2;
    breakinfo->textrow[n-1].rowheight = breakinfo->textrow[n-1].c2 - breakinfo->textrow[n-1].c1;
    }


static void breakinfo_remove_small_col_gaps(BREAKINFO *breakinfo,int lcheight,double mingap,
                                            double word_spacing)

    {
    int i,j;

    if (mingap < word_spacing)
        mingap = word_spacing;
    for (i=0;i<breakinfo->n-1;i++)
        {
        double gap;

        gap=(double)breakinfo->textrow[i].gap / lcheight;
        if (gap >= mingap)
            continue;
        breakinfo->textrow[i].c2 = breakinfo->textrow[i+1].c2;
        breakinfo->textrow[i].gap = breakinfo->textrow[i+1].gap;
        if (breakinfo->textrow[i+1].r1 < breakinfo->textrow[i].r1)
            breakinfo->textrow[i].r1 = breakinfo->textrow[i+1].r1;
        if (breakinfo->textrow[i+1].r2 > breakinfo->textrow[i].r2)
            breakinfo->textrow[i].r2 = breakinfo->textrow[i+1].r2;
        for (j=i+1;j<breakinfo->n-1;j++)
            breakinfo->textrow[j] = breakinfo->textrow[j+1];
        breakinfo->n--;
        i--;
        }
    }


void breakinfo_remove_small_rows(BREAKINFO *breakinfo,K2PDFOPT_SETTINGS *k2settings,
                                 double fracrh,double fracgap,
                                 BMPREGION *region,int *colcount,int *rowcount)

    {
    int i,j,mg,mh,mg0,mg1;
    int c1,c2,nc;
    int *rh,*gap;
    static char *funcname="breakinfo_remove_small_rows";

#if (WILLUSDEBUGX & 2)
printf("@breakinfo_remove_small_rows(fracrh=%g,fracgap=%g)\n",fracrh,fracgap);
#endif
    if (breakinfo->n<2)
        return;
    c1=region->c1;
    c2=region->c2;
    nc=c2-c1+1;
    willus_dmem_alloc_warn(16,(void **)&rh,2*sizeof(int)*breakinfo->n,funcname,10);
    gap=&rh[breakinfo->n];
    for (i=0;i<breakinfo->n;i++)
        {
        rh[i]=breakinfo->textrow[i].r2-breakinfo->textrow[i].r1+1;
        if (i<breakinfo->n-1)
            gap[i]=breakinfo->textrow[i].gap;
        }
    sorti(rh,breakinfo->n);
    sorti(gap,breakinfo->n-1);
    mh=rh[breakinfo->n/2];
    mh *= fracrh;
    if (mh<1)
        mh=1;
    mg0=gap[(breakinfo->n-1)/2];
    mg = mg0*fracgap;
    mg1 = mg0*0.7;
    if (mg<1)
        mg=1;
#if (WILLUSDEBUGX & 2)
printf("mh = %d x %g = %d\n",rh[breakinfo->n/2],fracrh,mh);
printf("mg = %d x %g = %d\n",gap[breakinfo->n/2],fracgap,mg);
#endif
    for (i=0;i<breakinfo->n;i++)
        {
        TEXTROW *textrow;
        int trh,gs1,gs2,g1,g2,gap_is_big,row_too_small;
        double m1,m2,row_width_inches;

        textrow=&breakinfo->textrow[i];
        trh=textrow->r2-textrow->r1+1;
        if (i==0)
            {
            g1=mg0+1;
            gs1=mg+1;
            }
        else
            {
            g1=textrow->r1-breakinfo->textrow[i-1].r2-1;
            gs1=breakinfo->textrow[i-1].gap;
            }
        if (i==breakinfo->n-1)
            {
            g2=mg0+1;
            gs2=mg+1;
            }
        else
            {
            g2=breakinfo->textrow[i+1].r1-textrow->r2-1;
            gs2=breakinfo->textrow[i].gap;
            }
#if (WILLUSDEBUGX & 2)
printf("   rowheight[%d] = %d, mh=%d, gs1=%d, gs2=%d\n",i,trh,mh,gs1,gs2);
#endif
        gap_is_big = (trh >= mh || (gs1 >= mg && gs2 >= mg));
        /*
        ** Is the row width small and centered?  If so, it should probably
        ** be attached to its nearest neighbor--it's usually a fragment of
        ** an equation or a table/figure.
        */
        row_width_inches = (double)(textrow->c2-textrow->c1+1)/region->dpi;
        m1 = fabs(textrow->c1-c1)/nc;
        m2 = fabs(textrow->c2-c2)/nc;
        row_too_small = m1 > 0.1 && m2 > 0.1 
                            && row_width_inches < k2settings->little_piece_threshold_inches
                            && (g1<=mg1 || g2<=mg1);
#if (WILLUSDEBUGX & 2)
printf("       m1=%g, m2=%g, rwi=%g, g1=%d, g2=%d, mg0=%d\n",m1,m2,row_width_inches,g1,g2,mg0);
#endif
        if (gap_is_big && !row_too_small)
            continue;
#if (WILLUSDEBUGX & 2)
printf("   row[%d] to be combined w/next row.\n",i);
#endif
        if (row_too_small)
            {
            if (g1<g2)
                i--;
            }
        else
            {
            if (gs1<gs2)
                i--;
            }
/*
printf("Removing row.  nrows=%d, rh=%d, gs1=%d, gs2=%d\n",breakinfo->n,trh,gs1,gs2);
printf("    mh = %d, mg = %d\n",rh[breakinfo->n/2],gap[(breakinfo->n-1)/2]);
*/
        breakinfo->textrow[i].r2 = breakinfo->textrow[i+1].r2;
        if (breakinfo->textrow[i+1].c2 > breakinfo->textrow[i].c2)
            breakinfo->textrow[i].c2 = breakinfo->textrow[i+1].c2;
        if (breakinfo->textrow[i+1].c1 < breakinfo->textrow[i].c1)
            breakinfo->textrow[i].c1 = breakinfo->textrow[i+1].c1;
        /* Re-compute rowbase, capheight, lcheight */
        {
        BMPREGION newregion;
        newregion=(*region);
        newregion.c1=breakinfo->textrow[i].c1;
        newregion.c2=breakinfo->textrow[i].c2;
        newregion.r1=breakinfo->textrow[i].r1;
        newregion.r2=breakinfo->textrow[i].r2;
        bmpregion_trim_margins(&newregion,k2settings,colcount,rowcount,0x1f);
        newregion.c1=breakinfo->textrow[i].c1;
        newregion.c2=breakinfo->textrow[i].c2;
        newregion.r1=breakinfo->textrow[i].r1;
        newregion.r2=breakinfo->textrow[i].r2;
        textrow_assign_bmpregion(&breakinfo->textrow[i],&newregion);
        }
        for (j=i+1;j<breakinfo->n-1;j++)
            breakinfo->textrow[j] = breakinfo->textrow[j+1];
        breakinfo->n--;
        i--;
        }
    willus_dmem_free(16,(double **)&rh,funcname);
    }
            

void breakinfo_alloc(int index,BREAKINFO *breakinfo,int nrows)

    {
    static char *funcname="breakinfo_alloc";

    willus_dmem_alloc_warn(index,(void **)&breakinfo->textrow,sizeof(TEXTROW)*(nrows/2+2),
                        funcname,10);
    }


void breakinfo_free(int index,BREAKINFO *breakinfo)

    {
    static char *funcname="breakinfo_free";

    willus_dmem_free(index,(double **)&breakinfo->textrow,funcname);
    }


void breakinfo_sort_by_gap(BREAKINFO *breakinfo)

    {
    int n,top,n1;
    TEXTROW *x,x0;

    x=breakinfo->textrow;
    n=breakinfo->n;
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
            if (child<n1 && x[child].gap<x[child+1].gap)
                child++;
            if (x0.gap<x[child].gap)
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


void breakinfo_sort_by_row_position(BREAKINFO *breakinfo)

    {
    int n,top,n1;
    TEXTROW *x,x0;

    x=breakinfo->textrow;
    n=breakinfo->n;
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
            if (child<n1 && x[child].r1<x[child+1].r1)
                child++;
            if (x0.r1<x[child].r1)
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
**
** Searches the region for vertical break points and stores them into
** the BREAKINFO structure.
**
** apsize_in = averaging aperture size in inches.  Use -1 for dynamic aperture.
**
*/
void bmpregion_find_vertical_breaks(BMPREGION *region,BREAKINFO *breakinfo,
                                    K2PDFOPT_SETTINGS *k2settings,
                                    int *colcount,int *rowcount,double apsize_in)

    {
    static char *funcname="bmpregion_find_vertical_breaks";
    int nr,i,brc,brcmin,dtrc,trc,aperture,aperturemax,figrow,labelrow;
    int ntr,rhmin_pix;
    BMPREGION *newregion,_newregion;
    int *rowthresh;
    double min_fig_height,max_fig_gap,max_label_height;

    min_fig_height=k2settings->dst_min_figure_height_in;
    max_fig_gap=0.16;
    max_label_height=0.5;
    /* Trim region and populate colcount/rowcount arrays */
    bmpregion_trim_margins(region,k2settings,colcount,rowcount,
                           k2settings->src_trim ? 0xf : 0);
    newregion=&_newregion;
    (*newregion)=(*region);
    if (k2settings->debug)
        printf("@bmpregion_find_vertical_breaks:  (%d,%d) - (%d,%d)\n",
                region->c1,region->r1,region->c2,region->r2);
    /*
    ** brc = consecutive blank pixel rows
    ** trc = consecutive non-blank pixel rows
    ** dtrc = number of non blank pixel rows since last dump
    */
    nr=region->r2-region->r1+1;
    willus_dmem_alloc_warn(15,(void **)&rowthresh,sizeof(int)*nr,funcname,10);
    brcmin = k2settings->max_vertical_gap_inches*region->dpi;
    aperturemax = (int)(region->dpi/72.+.5);
    if (aperturemax < 2)
        aperturemax = 2;
    aperture=(int)(region->dpi*apsize_in+.5);
/*
for (i=region->r1;i<=region->r2;i++)
printf("rowcount[%d]=%d\n",i,rowcount[i]);
*/
    breakinfo->rhmean_pixels=0; // Mean text row height
    ntr=0; // Number of text rows
    /* Fill rowthresh[] array */
    for (dtrc=0,i=region->r1;i<=region->r2;i++)
        {
        int ii,i1,i2,sum,pt;

        if (apsize_in < 0.)
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
        for (sum=0,ii=i1;ii<=i2;sum+=rowcount[ii],ii++);
        /* Does row have few enough black pixels to be considered blank? */
        if ((rowthresh[i-region->r1]=10*sum/pt)<=40)
            {
            if (dtrc>0)
                {
                breakinfo->rhmean_pixels += dtrc;
                ntr++;
                }
            dtrc=0;
            }
        else
            dtrc++;
        }
    if (dtrc>0)
        {
        breakinfo->rhmean_pixels += dtrc;
        ntr++;
        }
    if (ntr>0)
        breakinfo->rhmean_pixels /= ntr;
/*
printf("rhmean=%d (ntr=%d)\n",breakinfo->rhmean_pixels,ntr);
{
FILE *f;
static int count=0;
f=fopen("rthresh.ep",count==0?"w":"a");
count++;
for (i=region->r1;i<=region->r2;i++)
nprintf(f,"%d\n",rowthresh[i-region->r1]);
nprintf(f,"//nc\n");
fclose(f);
}
*/
    /* Minimum text row height required (pixels) */
    rhmin_pix = breakinfo->rhmean_pixels/3;
    if (rhmin_pix < .04*region->dpi)
        rhmin_pix = .04*region->dpi;
    if (rhmin_pix > .13*region->dpi)
        rhmin_pix = .13*region->dpi;
    if (rhmin_pix < 1)
        rhmin_pix = 1;
    /*
    for (rmax=region->r2;rmax>region->r1;rmax--)
        if (rowthresh[rmax-region->r1]>10)
            break;
    */
    /* Look for "row" gaps in the region so that it can be broken into */
    /* multiple "rows".                                                */
    breakinfo->n=0;
    for (labelrow=figrow=-1,dtrc=trc=brc=0,i=region->r1;i<=region->r2+1;i++)
        {
        /* Does row have few enough black pixels to be considered blank? */
        if (i>region->r2 || rowthresh[i-region->r1]<=10) 
            {
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

                /* Could this region be a figure? */
                if (i<=region->r2 && figrow < 0 && region_height_inches >= min_fig_height)
                    {
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
                        newregion->r2=newregion->r1-1;
                        newregion->r1=figrow;
                        newregion->c1=region->c1;
                        newregion->c2=region->c2;
                        bmpregion_trim_margins(newregion,k2settings,colcount,rowcount,0x1f);
                        if (newregion->r2>newregion->r1)
                            textrow_assign_bmpregion(&breakinfo->textrow[breakinfo->n++],newregion);
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
                bmpregion_trim_margins(newregion,k2settings,colcount,rowcount,0x1f);
                if (newregion->r2>newregion->r1)
                    textrow_assign_bmpregion(&breakinfo->textrow[breakinfo->n++],newregion);
                newregion->r1=i;
                dtrc=trc=0;
                brc=1;
                }
            }
        else
            {
            if (figrow>=0 && labelrow<0)
                labelrow=i;
            dtrc++;
            trc++;
            brc=0;
            }
        }
/* Re-did logic in 1.52 so that this next part is no longer necessary */
#ifdef COMMENT
    newregion->r2=region->r2;
    if (dtrc>0 && newregion->r2-newregion->r1+1 > 0)
        {
        /* If we were processing a figure, include it. */
        if (figrow>=0)
            newregion->r1=figrow;
        newregion->c1=region->c1;
        newregion->c2=region->c2;
        bmpregion_trim_margins(newregion,k2settings,colcount,rowcount,0x1f);
printf("Final add:  %d - %d\n",newregion->r1,newregion->r2);
        if (newregion->r2>newregion->r1)
            textrow_assign_bmpregion(&breakinfo->textrow[breakinfo->n++],newregion);
        }
#endif
    /* Compute gaps between rows and row heights */
    breakinfo_compute_row_gaps(breakinfo,region->r2);
    willus_dmem_free(15,(double **)&rowthresh,funcname);
    }


static void textrow_assign_bmpregion(TEXTROW *textrow,BMPREGION *region)

    {
    textrow->r1=region->r1;
    textrow->r2=region->r2;
    textrow->c1=region->c1;
    textrow->c2=region->c2;
    textrow->rowbase=region->rowbase;
    textrow->lcheight=region->lcheight;
    textrow->capheight=region->capheight;
    textrow->h5050=region->h5050;
    }


/*
** Add a vertically-contiguous rectangular region to the destination bitmap.
** The rectangular region may be broken up horizontally (wrapped).
*/
void bmpregion_one_row_find_breaks(BMPREGION *region,BREAKINFO *breakinfo,
                                   K2PDFOPT_SETTINGS *k2settings,
                                   int *colcount,int *rowcount,int add_to_dbase)

    {
    int nc,i,mingap,col0,dr,thlow,thhigh;
    int *bp;
    BMPREGION *newregion,_newregion;
    static char *funcname="bmpregion_one_row_find_breaks";

    if (k2settings->debug)
        printf("@bmpregion_one_row_find_breaks(%d,%d)-(%d,%d)\n",
               region->c1,region->r1,region->c2,region->r2);
    newregion=&_newregion;
    (*newregion)=(*region);
    bmpregion_trim_margins(newregion,k2settings,colcount,rowcount,0x1f);
    region->lcheight=newregion->lcheight;
    region->capheight=newregion->capheight;
    region->rowbase=newregion->rowbase;
    region->h5050=newregion->h5050;
    nc=newregion->c2-newregion->c1+1;
    breakinfo->n=0;
    if (nc<6)
        return;
    /*
    ** Look for "space-sized" gaps, i.e. gaps that would occur between words.
    ** Use this as pixel counting aperture.
    */
    dr=newregion->lcheight;
    mingap = dr*k2settings->word_spacing*0.8;
    if (mingap < 2)
        mingap = 2;

    /*
    ** Find places where there are gaps (store in bp array)
    ** Could do this more intelligently--maybe calculate a histogram?
    */
    willus_dmem_alloc_warn(18,(void **)&bp,sizeof(int)*nc,funcname,10);
    for (i=0;i<nc;i++)
        bp[i]=0;
    if (k2settings->src_left_to_right)
        {
        for (i=newregion->c1;i<=newregion->c2;i++)
            {
            int i1,i2,pt,sum,ii;
            i1=i-mingap/2;
            i2=i1+mingap-1;
            if (i1<newregion->c1)
                i1=newregion->c1;
            if (i2>newregion->c2)
                i2=newregion->c2;
            pt=(int)((i2-i1+1)*k2settings->gtw_in*region->dpi+.5);
            if (pt<1)
                pt=1;
            for (sum=0,ii=i1;ii<=i2;ii++,sum+=colcount[ii]);
            bp[i-newregion->c1]=10*sum/pt;
            }
        }
    else
        {
        for (i=newregion->c2;i>=newregion->c1;i--)
            {
            int i1,i2,pt,sum,ii;
            i1=i-mingap/2;
            i2=i1+mingap-1;
            if (i1<newregion->c1)
                i1=newregion->c1;
            if (i2>newregion->c2)
                i2=newregion->c2;
            pt=(int)((i2-i1+1)*k2settings->gtw_in*region->dpi+.5);
            if (pt<1)
                pt=1;
            for (sum=0,ii=i1;ii<=i2;ii++,sum+=colcount[ii]);
            bp[i-newregion->c1]=10*sum/pt;
            }
        }
#if (WILLUSDEBUGX & 4)
if (region->r1 > 3699 && region->r1<3750)
{
static int a=0;
FILE *f;
f=fopen("outbp.ep",a==0?"w":"a");
a++;
fprintf(f,"/sa l \"(%d,%d)-(%d,%d) lch=%d\" 2\n",region->c1,region->r1,region->c2,region->r2,region->lcheight);
for (i=0;i<nc;i++)
fprintf(f,"%d\n",bp[i]);
fprintf(f,"//nc\n");
fclose(f);
}
#endif
    thlow=10;
    thhigh=50;
    /*
    ** Break into pieces
    */
    for (col0=newregion->c1;col0<=newregion->c2;col0++)
        {
        int copt,c0;
        BMPREGION xregion;

        xregion=(*newregion);
        xregion.c1=col0;
        for (;col0<=newregion->c2;col0++)
            if (bp[col0-newregion->c1]>=thhigh)
                break;
        if (col0>newregion->c2)
            break;
        for (col0++;col0<=newregion->c2;col0++)
            if (bp[col0-newregion->c1]<thlow)
                break;
        for (copt=c0=col0;col0<=newregion->c2 && col0-c0<=dr;col0++)
            {
            if (bp[col0-newregion->c1] <  bp[copt-newregion->c1])
                copt=col0;
            if (bp[col0-newregion->c1] > thhigh)
                break;
            }
        if (copt>newregion->c2)
            copt=newregion->c2;
        xregion.c2=copt;
        if (xregion.c2-xregion.c1 < 2)
            continue;
        bmpregion_trim_margins(&xregion,k2settings,colcount,rowcount,0x1f);
        textrow_assign_bmpregion(&breakinfo->textrow[breakinfo->n++],&xregion);
        col0=copt;
        if (copt==newregion->c2)
            break;
        }
    breakinfo_compute_col_gaps(breakinfo,newregion->c2);
    willus_dmem_free(18,(double **)&bp,funcname);

    /* Remove small gaps */
    {
    double median_gap;
    word_gaps_add(add_to_dbase ? breakinfo : NULL,region->lcheight,&median_gap,
                  k2settings->word_spacing);
    breakinfo_remove_small_col_gaps(breakinfo,region->lcheight,median_gap/1.9,
                                    k2settings->word_spacing);
    }
    }


/*
** Track gaps between words so that we can tell when one is out of family.
** lcheight = height of a lowercase letter.
*/
void word_gaps_add(BREAKINFO *breakinfo,int lcheight,double *median_gap,
                   double word_spacing)

    {
    static int nn=0;
    static double gap[1024];
    static char *funcname="word_gaps_add";

    if (breakinfo==NULL && median_gap==NULL)
        {
        nn=0;
        return;
        }
    if (breakinfo!=NULL && breakinfo->n>1)
        {
        int i;

        for (i=0;i<breakinfo->n-1;i++)
            {
            double g;
            g = (double)breakinfo->textrow[i].gap / lcheight;
            if (g>=word_spacing)
                {
                gap[nn&0x3ff]= g;
                nn++;
                }
            }
        }
    if (median_gap!=NULL)
        {
        if (nn>0)
            {
            int n;
            static double *gap_sorted;

            n = (nn>1024) ? 1024 : nn;
            willus_dmem_alloc_warn(28,(void **)&gap_sorted,sizeof(double)*n,funcname,10);
            memcpy(gap_sorted,gap,n*sizeof(double));
            sortd(gap_sorted,n);
            (*median_gap)=gap_sorted[n/2];
            willus_dmem_free(28,&gap_sorted,funcname);
            }
        else
            (*median_gap)=0.7;
        }
    }

