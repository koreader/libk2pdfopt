/*
** textrows.c   Functions to handle TEXTROWS structure.
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


static int  minval(int *x,int n,int dx,int *index,int index0,int indexmin,int indexmax);
static int  maxval(int *x,int n,int dx,int *index,int index0);
static void textrow_assign_bmpregion(TEXTROW *textrow,BMPREGION *region,int type);
/*
static int textrows_median_row_height(TEXTROWS *textrows);
*/


void textrows_init(TEXTROWS *textrows)

    {
    textrows->n=textrows->na=0;
    textrows->textrow=NULL;
    }


void textrows_free(TEXTROWS *textrows)

    {
    static char *funcname="textrows_free";

    willus_mem_free((double **)&textrows->textrow,funcname);
    textrows->n=textrows->na=0;
    }


void textrows_clear(TEXTROWS *textrows)

    {
    textrows->n=0;
    }


void textrows_delete_one(TEXTROWS *textrows,int index)

    {
    int i;

    for (i=index;i<textrows->n-1;i++)
        textrows->textrow[i] = textrows->textrow[i+1];
    textrows->n--;
    }


void textrows_insert(TEXTROWS *dst,int index,TEXTROWS *src)

    {
    static char *funcname="textrows_insert";
    int i;

    if (src->n<1)
        return;
    if (dst->n + src->n > dst->na)
        {
        int newsize;

        newsize = dst->na<128 ? 256 : dst->na*2;
        while (newsize < dst->n + src->n)
            newsize *= 2;
        willus_mem_realloc_robust_warn((void **)&dst->textrow,
                                     newsize*sizeof(TEXTROW),
                                     dst->na*sizeof(TEXTROW),funcname,10);
        dst->na=newsize;
        }
    for (i=dst->n+src->n-1;i-src->n>=index;i--)
        dst->textrow[i] = dst->textrow[i-src->n];
    for (i=0;i<src->n;i++)
        dst->textrow[i+index] = src->textrow[i];
    dst->n += src->n;
    }


void textrows_add_textrow(TEXTROWS *textrows,TEXTROW *textrow)

    {
    static char *funcname="textrows_add_textrow";

/*
printf("@textrows_add_textrow.\n");
printf("    textrows=%p\n",textrows);
printf("    textrows->n=%d, na=%d\n",textrows->n,textrows->na);
printf("    textrow=%p\n",textrow);
printf("    textrows->textrow=%p\n",textrows->textrow);
*/
    if (textrows->n>=textrows->na)
        {
        int newsize;

        newsize = textrows->na<128 ? 256 : textrows->na*2;
        willus_mem_realloc_robust_warn((void **)&textrows->textrow,newsize*sizeof(TEXTROW),
                                     textrows->na*sizeof(TEXTROW),funcname,10);
        textrows->na=newsize;
        }
    textrows->textrow[textrows->n]=(*textrow);
    textrows->n++;
    }


void textrows_add_bmpregion(TEXTROWS *textrows,BMPREGION *bmpregion,int type)

    {
    TEXTROW textrow;

    textrow_assign_bmpregion(&textrow,bmpregion,type);
    textrows_add_textrow(textrows,&textrow);
    }


void textrow_assign_bmpregion(TEXTROW *textrow,BMPREGION *bmpregion,int type)

    {
    (*textrow)=bmpregion->bbox;
    textrow->rat=0.;
    textrow->type=type;
    }


void textrow_init(TEXTROW *textrow)

    {
    textrow->type = 0;
    textrow->c1=textrow->c2=textrow->r1=textrow->r2=-1;
    textrow->rowbase=textrow->gap=textrow->rowheight=textrow->capheight=textrow->h5050=textrow->lcheight=-1;
    textrow->rat = 0.;
    textrow->hyphen.ch = -1;
    }


#if (WILLUSDEBUGX & 6)
void textrows_echo(TEXTROWS *textrows,char *name)

    {
    int i;

    k2printf("@text%s_echo (%d rows)...\n",name,textrows->n);
    for (i=0;i<textrows->n;i++)
        k2printf("    %2d.  r1=%4d, rowbase=%4d, r2=%4d, c1=%4d, c2=%4d\n",
             i+1,textrows->textrow[i].r1,
             textrows->textrow[i].rowbase,
             textrows->textrow[i].r2,
             textrows->textrow[i].c1,
             textrows->textrow[i].c2);
    }
#endif


/*
** margin = % with which spacings have to agree.  E.g. 10 means within 10%.
*/
int textrow_line_spacing_is_same(TEXTROW *tr1,TEXTROW *tr2,int margin)

    {
    return(AGREE_WITHIN_MARGIN(tr1->rowheight,tr2->rowheight,margin));
    }


/*
** If any of the three determinations of font size are the same, it's a match.
*/
int textrow_font_size_is_same(TEXTROW *tr1,TEXTROW *tr2,int margin)

    {
    if (tr1->type!=REGION_TYPE_TEXTLINE || tr2->type!=REGION_TYPE_TEXTLINE)
        return(0);
    return(AGREE_WITHIN_MARGIN(tr1->lcheight,tr2->lcheight,margin)
           || AGREE_WITHIN_MARGIN(tr1->h5050,tr2->h5050,margin)
           || AGREE_WITHIN_MARGIN(tr1->capheight,tr2->capheight,margin));
    }


void textrows_compute_row_gaps(TEXTROWS *textrows,int r2)

    {
    int i,n;

    n=textrows->n;
    if (n<=0)
        return;
    /* New in v1.65 -- use [1].r1 - [0].r1 for first rowheight. */
    if (textrows->n>1)
        textrows->textrow[0].rowheight = textrows->textrow[1].r1 - textrows->textrow[0].r1;
    else
        textrows->textrow[0].rowheight = textrows->textrow[0].r2 - textrows->textrow[0].r1;
    for (i=0;i<n-1;i++)
        {
        int r1;

        r1 = (textrows->textrow[i].type==REGION_TYPE_FIGURE) 
                   ? textrows->textrow[i].r2 : textrows->textrow[i].rowbase;
        textrows->textrow[i].gap = textrows->textrow[i+1].r1 - r1 - 1;
        textrows->textrow[i].gapblank = textrows->textrow[i+1].r1 - textrows->textrow[i].r2 - 1;
        }
        /*
        textrows->textrow[i].rowheight = textrows->textrow[i+1].r1 - textrows->textrow[i].r1;
        */
    for (i=1;i<n;i++)
        textrows->textrow[i].rowheight = textrows->textrow[i].rowbase 
                                            - textrows->textrow[i-1].rowbase;
    if (textrows->textrow[n-1].type==REGION_TYPE_FIGURE)
        textrows->textrow[n-1].gap = 0;
    else
        textrows->textrow[n-1].gap = r2 - textrows->textrow[n-1].rowbase;
    textrows->textrow[n-1].gapblank = 0;
    }


void textrows_remove_small_rows(TEXTROWS *textrows,K2PDFOPT_SETTINGS *k2settings,
                                double fracrh,double fracgap,BMPREGION *region)

    {
    int i,j,mg,mh,mg0,mg1;
    int c1,c2,nc;
    int *rh,*gap;
    static char *funcname="textrows_remove_small_rows";

#if (WILLUSDEBUGX & 2)
k2printf("@textrows_remove_small_rows(fracrh=%g,fracgap=%g)\n",fracrh,fracgap);
#endif
    if (textrows->n<2)
        return;
    c1=region->c1;
    c2=region->c2;
    nc=c2-c1+1;
    willus_dmem_alloc_warn(16,(void **)&rh,2*sizeof(int)*textrows->n,funcname,10);
    gap=&rh[textrows->n];
    for (i=0;i<textrows->n;i++)
        {
        rh[i]=textrows->textrow[i].r2-textrows->textrow[i].r1+1;
        if (i<textrows->n-1)
            gap[i]=textrows->textrow[i].gapblank;
        }
    sorti(rh,textrows->n);
    sorti(gap,textrows->n-1);
    mh=rh[textrows->n/2];
    mh *= fracrh;
    if (mh<1)
        mh=1;
    mg0=gap[(textrows->n-1)/2];
    mg = mg0*fracgap;
    mg1 = mg0*0.7;
    if (mg<1)
        mg=1;
#if (WILLUSDEBUGX & 2)
k2printf("mh = %d x %g = %d\n",rh[textrows->n/2],fracrh,mh);
k2printf("mg = %d x %g = %d\n",gap[textrows->n/2],fracgap,mg);
#endif
    for (i=0;i<textrows->n;i++)
        {
        TEXTROW *textrow;
        int trh,gs1,gs2,g1,g2,gap_is_big,row_too_small;
        double m1,m2,row_width_inches;

        textrow=&textrows->textrow[i];
        trh=textrow->r2-textrow->r1+1;
        if (i==0)
            {
            g1=mg0+1;
            gs1=mg+1;
            }
        else
            {
            g1=textrow->r1-textrows->textrow[i-1].r2-1;
            gs1=textrows->textrow[i-1].gap;
            }
        if (i==textrows->n-1)
            {
            g2=mg0+1;
            gs2=mg+1;
            }
        else
            {
            g2=textrows->textrow[i+1].r1-textrow->r2-1;
            gs2=textrows->textrow[i].gap;
            }
#if (WILLUSDEBUGX & 2)
k2printf("   rowheight[%d] = %d, mh=%d, gs1=%d, gs2=%d\n",i,trh,mh,gs1,gs2);
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
k2printf("       m1=%g, m2=%g, rwi=%g, g1=%d, g2=%d, mg0=%d\n",m1,m2,row_width_inches,g1,g2,mg0);
#endif
        if (gap_is_big && !row_too_small)
            continue;
#if (WILLUSDEBUGX & 2)
k2printf("   row[%d] to be combined w/next row.\n",i);
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
k2printf("Removing row.  nrows=%d, rh=%d, gs1=%d, gs2=%d\n",textrows->n,trh,gs1,gs2);
k2printf("    mh = %d, mg = %d\n",rh[textrows->n/2],gap[(textrows->n-1)/2]);
*/
        textrows->textrow[i].r2 = textrows->textrow[i+1].r2;
        if (textrows->textrow[i+1].c2 > textrows->textrow[i].c2)
            textrows->textrow[i].c2 = textrows->textrow[i+1].c2;
        if (textrows->textrow[i+1].c1 < textrows->textrow[i].c1)
            textrows->textrow[i].c1 = textrows->textrow[i+1].c1;
        /* Re-compute rowbase, capheight, lcheight */
        {
        BMPREGION newregion;

        bmpregion_init(&newregion);
        bmpregion_copy(&newregion,region,0);
        newregion.c1=textrows->textrow[i].c1;
        newregion.c2=textrows->textrow[i].c2;
        newregion.r1=textrows->textrow[i].r1;
        newregion.r2=textrows->textrow[i].r2;
        newregion.bbox.type=0;
        bmpregion_calc_bbox(&newregion,k2settings,1);
        textrow_assign_bmpregion(&textrows->textrow[i],&newregion,REGION_TYPE_TEXTLINE);
        bmpregion_free(&newregion);
        }
        for (j=i+1;j<textrows->n-1;j++)
            textrows->textrow[j] = textrows->textrow[j+1];
        textrows->n--;
        i--;
        }
    willus_dmem_free(16,(double **)&rh,funcname);
    }


void textrows_sort_by_gap(TEXTROWS *textrows)

    {
    int n,top,n1;
    TEXTROW *x,x0;

    x=textrows->textrow;
    n=textrows->n;
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


void textrows_sort_by_row_position(TEXTROWS *textrows)

    {
    int n,top,n1;
    TEXTROW *x,x0;

    x=textrows->textrow;
    n=textrows->n;
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
** Find double-height rows and split them into two if there is a reasonable gap.
*/
void textrows_find_doubles(TEXTROWS *textrows,int *rowthresh,BMPREGION *region,
                           K2PDFOPT_SETTINGS *k2settings,int maxsize)

    {
    int i,r1,r2;
    int rb[4];

    r1=region->r1;
    r2=region->r2;
    if (maxsize > 5)
        maxsize = 5;
#if (WILLUSDEBUGX & 256)
printf("@textrows_find_doubles, textrows->n=%d\n",textrows->n);
for (i=0;i<textrows->n;i++)
printf("    row[%2d].lc/h5050/cap = %3d %3d %3d\n",i,textrows->textrow[i].lcheight,textrows->textrow[i].h5050,textrows->textrow[i].capheight);
#endif
    for (i=0;i<textrows->n;i++)
        {
        int lch,rh;

        lch=0;
        if (i>0 && textrows->textrow[i].capheight > textrows->textrow[i-1].capheight*1.8)
            lch=textrows->textrow[i-1].lcheight;
        if (lch==0 && i<textrows->n-1
                  && textrows->textrow[i].capheight > textrows->textrow[i+1].capheight*1.8)
            lch=textrows->textrow[i+1].lcheight;
        /* Get "nominal" rowheight */
        rh=-1;
        if (i-1>0)
            rh = textrows->textrow[i-1].rowheight;
        if (i>0 && (rh<0 || rh > textrows->textrow[i].rowheight))
            rh = textrows->textrow[i].rowheight;
        if (rh<0 || rh > textrows->textrow[i+1].rowheight)
            rh = textrows->textrow[i+1].rowheight;

        /* Make sure it's not too big */
        if (rh > 7*lch)
            rh = 7*lch;
        
        /* v1.66 fix -- r2 - r1 must be > 5 */   
        if (lch>0 && textrows->textrow[i].r2-textrows->textrow[i].r1>5)
           {
           int jbest,itry;
           double maxrat;

           maxrat = -1.0;
           jbest=-1;
           for (itry=0;itry<=1;itry++)

           {
           int j;

#if (WILLUSDEBUGX & 256)
if (itry==0)
printf("Large gap:  row[%d] = rows %d - %d, capheight = %d, lch=%d, rh=%d\n",i,textrows->textrow[i].r1,textrows->textrow[i].r2,textrows->textrow[i].capheight,lch,rh);
#endif
           for (j=2;j<=maxsize;j++)
               {
               int ip,c1,c2;
               double rat;

               if (itry==1 && j!=jbest)
                   continue;
               /* Sanity check the row height -- should be approx. j * the nominal height */
               if (i>0 && (textrows->textrow[i].rowheight < j*rh*0.7
                             || textrows->textrow[i].rowheight > j*rh*1.5))
                   continue;
               if (i==0 && (textrows->textrow[i].capheight < j*rh*0.7
                             || textrows->textrow[i].capheight > j*rh*1.5))
                   continue;
               c1=c2=-1;
               for (ip=0;ip<j-1;ip++)
                   {
                   int rmin,c;
                   /* v1.66 fix -- bound the search */
                   c=minval(rowthresh,r2-r1+1,lch,&rmin,
                        textrows->textrow[i].rowbase-(ip+1)*textrows->textrow[i].capheight/j-r1,
                        textrows->textrow[i].r1+2-r1,textrows->textrow[i].r2-2-r1);
                   rb[ip]=rmin+r1;
/* printf("        nadir point[%d of %d] = row %4d = %d\n",ip+1,j-1,rmin+r1,rowthresh[rmin]); */
                   if (c1<0 || c>c1)
                       c1=c;
                   } 
               for (ip=0;ip<j;ip++)
                   {
                   int rmax,c;
                   if (ip==0)
                       c=maxval(rowthresh,r2-r1+1,lch,&rmax,textrows->textrow[i].rowbase-textrows->textrow[i].capheight+lch-r1);
                   else if (ip==j-1)
                       c=maxval(rowthresh,r2-r1+1,lch,&rmax,textrows->textrow[i].rowbase-lch-r1);
                   else
                       c=maxval(rowthresh,r2-r1+1,lch,&rmax,textrows->textrow[i].rowbase - (ip+1)*textrows->textrow[i].capheight*ip/(j-1)-r1);
                   if (c2<0 || c<c2)
                       c2=c;
/* printf("        peak point[%d of %d] = row %4d = %d\n",ip+1,j,rmax+r1,rowthresh[rmax]); */
                   } 
               if (c1==0)
                   c1=1;
               rat=(double)c2/c1;
#if (WILLUSDEBUGX & 256)
if (itry==0)
printf("    rat[%d rows] = %g\n",j,rat);
#endif
               if (maxrat<0 || rat > maxrat)
                   {
                   maxrat = rat;
                   jbest = j;
                   }
               }
           }
#if (WILLUSDEBUGX & 256)
printf("MAX RATIO (%d rows) = %g\n",jbest,maxrat);
#endif
           /* If figure of merit is met, split this row into jbest rows */
           if (maxrat > k2settings->row_split_fom)
               {
               int ii;
               TEXTROWS newrows;

               /* Insert jbest-1 new rows (copies of textrow[i]) at index i */
               textrows_init(&newrows);
               for (ii=0;ii<jbest-1;ii++)
                   textrows_add_textrow(&newrows,&textrows->textrow[i]);
               textrows_insert(textrows,i,&newrows);
               textrows_free(&newrows);

               /* Modify the copies */
               for (ii=i;ii<i+jbest;ii++)
                   {
                   BMPREGION _newregion,*newregion;
                   TEXTROW *textrow;

                   textrow=&textrows->textrow[ii];
                   if (ii<i+jbest-1)
                       textrow->r2 = rb[ii-i];
                   if (ii>i)
                       textrow->r1 = rb[ii-i-1]+1;
                   newregion=&_newregion;
                   bmpregion_init(newregion);
                   bmpregion_copy(newregion,region,0);
                   newregion->c1=textrow->c1;
                   newregion->c2=textrow->c2;
                   newregion->r1=textrow->r1;
                   newregion->r2=textrow->r2;
                   newregion->bbox.type=0;
                   bmpregion_calc_bbox(newregion,k2settings,1);
                   textrow_assign_bmpregion(textrow,newregion,REGION_TYPE_TEXTLINE);
                   textrow->rat=maxrat;
                   bmpregion_free(newregion);
                   }
               }
           }
        }
    }


void textrow_determine_type(BMPREGION *region,K2PDFOPT_SETTINGS *k2settings,int index)

    {
    TEXTROW *textrow;
    int nr,nc;
    double aspect_ratio;
    double region_height_inches;

    textrow=&region->textrows.textrow[index];
    if (textrow->type != REGION_TYPE_FIGURE)
        {
        nr=textrow->r2-textrow->r1+1;
        nc=textrow->c2-textrow->c1+1;
        aspect_ratio = (double)nr/nc;
        region_height_inches = (double)nr/region->dpi;
        if (aspect_ratio > k2settings->no_wrap_ar_limit
                  && (region_height_inches > k2settings->no_wrap_height_limit_inches
                       || region_height_inches > k2settings->dst_min_figure_height_in))
            {
            textrow->type = REGION_TYPE_FIGURE;
/*
printf("2. textrow[%d] type = figure.\n",index);
*/
            }
        }
    }


void textrow_scale(TEXTROW *textrow,double scalew,double scaleh,int c2max,int r2max)

    {
    textrow->c1 = (textrow->c1*scalew+.5);
    if (textrow->c1 > c2max)
        textrow->c1 = c2max;
    textrow->r1 = (textrow->r1*scaleh+.5);
    if (textrow->r1 > r2max)
        textrow->r1 = r2max;
    textrow->c2 = (textrow->c2*scalew+.5);
    if (textrow->c2 > c2max)
        textrow->c2 = c2max;
    textrow->r2 = (textrow->r2*scaleh+.5);
    if (textrow->r2 > r2max)
        textrow->r2 = r2max;
    textrow->rowbase = (textrow->rowbase*scaleh+.5);
    if (textrow->rowbase > r2max)
        textrow->rowbase=r2max;
    textrow->gap = (textrow->gap*scaleh+.5);
    textrow->gapblank = (textrow->gapblank*scaleh+.5);
    textrow->rowheight = (textrow->rowheight*scaleh+.5);
    textrow->capheight = (textrow->capheight*scaleh+.5);
    textrow->h5050 = (textrow->h5050*scaleh+.5);
    textrow->lcheight = (textrow->lcheight*scaleh+.5);
    if (textrow->hyphen.ch>=0)
        {
        textrow->hyphen.ch = (textrow->hyphen.ch*scalew+.5);
        textrow->hyphen.c2 = (textrow->hyphen.c2*scalew+.5);
        if (textrow->hyphen.c2 > c2max)
            textrow->hyphen.c2 = c2max;
        textrow->hyphen.r1 = (textrow->hyphen.r1*scaleh+.5);
        if (textrow->hyphen.r1 > r2max)
            textrow->hyphen.r1 = r2max;
        textrow->hyphen.r2 = (textrow->hyphen.r2*scalew+.5);
        if (textrow->hyphen.r2 > r2max)
            textrow->hyphen.r2 = r2max;
        }
    }

    
/*
** v1.66 fix -- pass indexmin and indexmax to bound the search.
*/
static int minval(int *x,int n,int dx,int *index,int index0,int indexmin,int indexmax)

    {
    int i,imin,dx2,index1,index2;

    dx2=dx/4;
    if (dx2<1)
        dx2=1;
    index1=index0-dx2;
    index2=index0+dx2;
    if (index1<indexmin)
        {
        index1=indexmin;
        if (index2 <= index1)
            index2=index1+1;
        }
    if (index2>indexmax)
        {
        index2=indexmax;
        if (index1 >= index2)
            index1=index2-1;
        }
    for (imin=-1,i=index1;i<=index2;i++)
        {
        if (i<0 || i>=n)
            continue;
        if (imin<0 || x[i]<x[imin])
            imin=i;
        }
    if (index!=NULL)
        (*index)=imin;
    return(x[imin]);
    }


static int maxval(int *x,int n,int dx,int *index,int index0)

    {
    int i,imax,dx2;

    dx2=dx/4;
    if (dx2<1)
        dx2=1;
    for (imax=-1,i=index0-dx2;i<=index0+dx2;i++)
        {
        if (i<0 || i>=n)
            continue;
        if (imax<0 || x[i]>x[imax])
            imax=i;
        }
    if (index!=NULL)
        (*index)=imax;
    return(x[imax]);
    }
