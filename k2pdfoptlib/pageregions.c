/*
** pageregions.c   Functions to handle PAGEREGIONS structure.
**
** Copyright (C) 2014  http://willus.com
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

/*
** Values are in inches
** y1 < y2, x1 < x2
** Lower y values are higher on the page (like r1, r2 in bmpregion)
** fs[0..1] = font size of first two rows
** yb[0..1] = baseline pos. of first two rows
** fs[2..3] = font size of last two rows
** yb[2..3] = baseline pos. of last two rows
** dylr = Distance between baselines of last two rows
*/
typedef struct
    {
    double x1,y1;
    double x2,y2;
    double fs[4];
    double yb[4];
    int ignore;
    int next;
    int prev;
    } PBOX;

#if (WILLUSDEBUGX & 0x400000)
static void pbox_echo(PBOX *box);
#endif
static int pbox_position_compare(PBOX *box1,PBOX *box2);
static int trapped_box(int ibox,PBOX *box,int n,double comax);
static int pbox_closest(PBOX *lastbox,PBOX *box,int n,int hoverlap,
                        double *alignbest,double *gapbest);
static void pbox_determine_closeness(PBOX *box0,PBOX *box1,double *gap_inches,
                                     double *colalign,double *rowalign);
static double alignment(double x0,double x1,double x2,double x3);


void pageregions_init(PAGEREGIONS *regions)

    {
    regions->n=regions->na=0;
    regions->pageregion=NULL;
    }


void pageregions_free(PAGEREGIONS *regions)

    {
    static char *funcname="pageregions_free";
    int i;

    for (i=regions->n-1;i>=0;i--)
        pageregion_free(&regions->pageregion[i]);
    willus_mem_free((double **)&regions->pageregion,funcname);
    }


void pageregions_delete_one(PAGEREGIONS *regions,int index)

    {
    int i;

    if (index<0 || index>=regions->n)
        return;
    for (i=index;i<regions->n-1;i++)
        pageregion_copy(&regions->pageregion[i],&regions->pageregion[i+1]);
    pageregion_free(&regions->pageregion[regions->n-1]);
    regions->n--;
    }


void pageregion_free(PAGEREGION *region)

    {
    bmpregion_free(&region->bmpregion);
    }


void pageregion_init(PAGEREGION *region)

    {
    bmpregion_init(&region->bmpregion);
    }


void pageregion_copy(PAGEREGION *dst,PAGEREGION *src)

    {
    pageregion_free(dst);
    bmpregion_copy(&dst->bmpregion,&src->bmpregion,1);
    dst->fullspan = src->fullspan;
    dst->level = src->level;
    dst->notes = src->notes;
    }


void pageregions_insert(PAGEREGIONS *dst,int index,PAGEREGIONS *src)

    {
    static char *funcname="pageregions_insert";
    int i;

    if (src->n<1)
        return;
    if (dst->n + src->n > dst->na)
        {
        int newsize;

        newsize = dst->na<16 ? 32 : dst->na*2;
        while (newsize < dst->n + src->n)
            newsize *= 2;
        willus_mem_realloc_robust_warn((void **)&dst->pageregion,newsize*sizeof(PAGEREGION),
                                     dst->na*sizeof(PAGEREGION),funcname,10);
        dst->na=newsize;
        }
    /* Must initialize the new array elements that will be used */
    for (i=dst->n;i<dst->n+src->n;i++)
        pageregion_init(&dst->pageregion[i]);
    for (i=dst->n+src->n-1;i-src->n>=index;i--)
        pageregion_copy(&dst->pageregion[i],&dst->pageregion[i-src->n]);
    for (i=0;i<src->n;i++)
        pageregion_copy(&dst->pageregion[i+index],&src->pageregion[i]);
    dst->n += src->n;
    }


void pageregions_add_pageregion(PAGEREGIONS *regions,BMPREGION *bmpregion,int level,
                                       int fullspan,int notes)

    {
    static char *funcname="pageregions_add_pageregion";

    if (regions->n>=regions->na)
        {
        int newsize;

        newsize = regions->na<16 ? 32 : regions->na*2;
        willus_mem_realloc_robust_warn((void **)&regions->pageregion,newsize*sizeof(PAGEREGION),
                                     regions->na*sizeof(PAGEREGION),funcname,10);
        regions->na=newsize;
        }
    pageregion_init(&regions->pageregion[regions->n]);
    bmpregion_copy(&regions->pageregion[regions->n].bmpregion,bmpregion,1);
    regions->pageregion[regions->n].level=level;
    regions->pageregion[regions->n].fullspan=fullspan;
    regions->pageregion[regions->n].notes=notes;
    regions->n++;
    }


/*
** Heuristic algorithm to sort page region boxes on a page in order of how they should be read.
**
** Expects page regions to have text rows defined.
*/
void pageregions_sort(PAGEREGIONS *pageregions,double src_dpi,int left_to_right,
                                               double comax_fraction,double rgapmin_inches,
                                               double maxcolgap_inches)

    {
    static char *funcname="pageregions_sort";
    int *sorted_index;
    PBOX *box;
    double xmin,xmax,ymin,ymax;
    int i,isr,j,itry;

#if (WILLUSDEBUGX & 0x400000)
printf("@pageregions_sort, n=%d\n",pageregions->n);
#endif
    if (pageregions->n<2)
        return;
    willus_mem_alloc_warn((void **)&box,sizeof(PBOX)*pageregions->n,funcname,10);
    willus_mem_alloc_warn((void **)&sorted_index,sizeof(int)*pageregions->n,funcname,10);
    xmin=ymin=1.0e10;
    xmax=ymax=-1.0e10;

    /* Fill in PBOX array */
    for (i=0;i<pageregions->n;i++)
        {
        BMPREGION *region;

        sorted_index[i]=i;
        region=&pageregions->pageregion[i].bmpregion;
        box[i].x1 = region->c1/src_dpi;
        box[i].y1 = region->r1/src_dpi;
        box[i].x2 = (region->c2+1)/src_dpi;
        box[i].y2 = (region->r2+1)/src_dpi;
        box[i].ignore = (box[i].y2-box[i].y1 < .05 || box[i].x2-box[i].x1 < .05);
        box[i].next = -1;
        box[i].prev = -1;
        for (j=0;j<2;j++)
            {
            if (j<region->textrows.n)
                {
                box[i].yb[j] = region->textrows.textrow[j].rowbase/src_dpi;
                box[i].fs[j] = region->textrows.textrow[j].h5050/src_dpi;
                box[i].yb[3-j] = region->textrows.textrow[region->textrows.n-1-j].rowbase/src_dpi;
                box[i].fs[3-j] = region->textrows.textrow[region->textrows.n-1-j].h5050/src_dpi;
                }
            else
                {
                box[i].yb[j] = -1.;
                box[i].fs[j] = -1.;
                box[i].yb[3-j] = -1.;
                box[i].fs[3-j] = -1.;
                }
            }
        if (box[i].x1 < xmin)
            xmin=box[i].x1;
        if (box[i].x1 > xmax)
            xmax=box[i].x1;
        if (box[i].y1 < ymin)
            ymin=box[i].y1;
        if (box[i].y1 > ymax)
            ymax=box[i].y1;
        if (box[i].x2 < xmin)
            xmin=box[i].x2;
        if (box[i].x2 > xmax)
            xmax=box[i].x2;
        if (box[i].y2 < ymin)
            ymin=box[i].y2;
        if (box[i].y2 > ymax)
            ymax=box[i].y2;
#if (WILLUSDEBUGX & 0x400000)
printf("        BOX[%d]=",i);
pbox_echo(&box[i]);
#endif
        }

    /* Horizontally flip all boxes if the order is right to left */
    if (!left_to_right)
        for (i=0;i<pageregions->n;i++)
            {
            box[i].x1 = xmin + (xmax - box[i].x1);
            box[i].x2 = xmin + (xmax - box[i].x2);
            double_swap(box[i].x1,box[i].x2);
            }

    /* Sort PBOX array by position of boxes */
    /* Find first--upper left */
    for (sorted_index[0]=0,i=1;i<pageregions->n;i++)
        {
        if (box[i].ignore)
            continue;
        if (pbox_position_compare(&box[i],&box[sorted_index[0]]) < 0)
            sorted_index[0]=i;
        }
    box[sorted_index[0]].prev = 9999;
#if (WILLUSDEBUGX & 0x400000)
printf("        FIRST BOX(%d)=",sorted_index[0]);
pbox_echo(&box[sorted_index[0]]);
#endif

    /*
    ** Be more lenient with each try about choosing neighboring boxes
    */
    for (itry=0;itry<7;itry++)
        {
        int i;
        double epsilon;
        double epsilon_spacing;
        double align_thresh;
        double gap_thresh;
        static int check_above_below[7]={1,1,0,1,0,1,0};
        static double gap_thresh_array[7]={-1.,0.,0.,0.25,0.25,0.5,0.5};
        static double align_thresh_array[7]={1.,1.,0.98,0.75,0.75,0.5,0.5};

#if (WILLUSDEBUGX & 0x400000)
printf("ITRY = %d\n\n",itry);
#endif
        epsilon = 0.1; /* inches--minimum relevant difference in gaps */
        epsilon_spacing = 0.01;
        gap_thresh = gap_thresh_array[itry];
        align_thresh = check_above_below[itry] ? align_thresh_array[itry]*(1.-comax_fraction)
                                               : align_thresh_array[itry];
        if (fabs(gap_thresh) < .0001)
            gap_thresh = check_above_below[itry] ? rgapmin_inches : maxcolgap_inches;
        for (i=0;i<pageregions->n;i++)
            {
            double halign,valign,vgap,hgap;
            int ibest,ibestbelow,ibestright,spacing_match;

            if (box[i].ignore || box[i].next >= 0)
                continue;
#if (WILLUSDEBUGX & 0x400000)
printf("\n    box[%d] = ",i);
pbox_echo(&box[i]);
#endif
            box[i].next=i;
            spacing_match=0;
            ibestbelow = ibestright = -1;
            if (check_above_below[itry])
                {
                ibestbelow = pbox_closest(&box[i],box,pageregions->n,1,&halign,&vgap);
                if (ibestbelow>=0 && box[ibestbelow].ignore)
                    ibestbelow=-1;
                /* Check for contiguous rows of text */
                if (ibestbelow>=0)
                    {
                    PBOX *box1,*box2;
                    box1 = &box[i];
                    box2 = &box[ibestbelow];
                    if (box1->fs[2]>0. && box1->fs[3]>0. && box1->yb[2]>0. && box1->yb[3]>0.
                          && box2->fs[0]>0. && box2->fs[1]>0. && box2->yb[0]>0. && box2->yb[1]>0.
                          && fabs(box1->fs[2]-box1->fs[3])<epsilon_spacing
                          && fabs(box2->fs[0]-box2->fs[1])<epsilon_spacing
                          && fabs(box1->fs[3]-box2->fs[0])<epsilon_spacing
                          && fabs((box1->yb[3]-box1->yb[2])-(box2->yb[1]-box2->yb[0]))<epsilon_spacing)
                        {
                        spacing_match=1;
#if (WILLUSDEBUGX & 0x400000)
printf("    SPACING MATCH between %d and %d.\n",i,ibestbelow);
#endif
                        }
                    }
                }
            else
                {
                ibestright = pbox_closest(&box[i],box,pageregions->n,0,&valign,&hgap);
                if (ibestright>=0 && box[ibestright].ignore)
                    ibestright=-1;
                }
#if (WILLUSDEBUGX & 0x400000)
printf("    ibestbelow=%d, ibestright=%d\n",ibestbelow,ibestright);
if (check_above_below[itry])
printf("           halign=%g (thresh=%g), vgap=%g (thresh=%g)\n",halign,align_thresh,vgap,gap_thresh);
else
printf("           valign=%g (thresh=%g), hgap=%g (thresh=%g)\n",valign,align_thresh,hgap,gap_thresh);
#endif
            box[i].next=-1;
            ibest = -1;
            /* Meets user specs for contiguous column? */
            if (ibestbelow>=0)
                {
                if ((gap_thresh<0. && halign>=align_thresh && spacing_match)
                     || (halign>0. && halign>=align_thresh && vgap<=gap_thresh)
                     || (vgap<=gap_thresh && halign>=align_thresh 
                          && (ibestright<0 || vgap<hgap+epsilon 
                                           || box[ibestbelow].y1<box[ibestright].y2)))
                    ibest=ibestbelow;
                }
            else
                {
                if (ibestright>=0 && hgap<=gap_thresh && valign>=align_thresh
                                  && (ibestbelow<0 || hgap<vgap+epsilon))
                    ibest=ibestright;
                }
            if (ibest>=0)
                {
#if (WILLUSDEBUGX & 0x400000)
printf("ADJACENT BOX:  box[%d] = ",ibest);
pbox_echo(&box[ibest]);
#endif
                box[i].next=ibest;
                box[ibest].prev=i;
                if (trapped_box(i,box,pageregions->n,comax_fraction))
                    {
                    box[i].next = -1;
                    box[ibest].prev = -1;
                    }
                }
            }
        }
            
    /* We've connected all the boxes we can.  Now just order the rest by top-leftness. */
    for (i=1;i<pageregions->n;i++)
        {
        int j,k,jbest;

        if (box[sorted_index[i-1]].next >= 0)
            {
            sorted_index[i] = box[sorted_index[i-1]].next;
#if (WILLUSDEBUGX & 0x400000)
printf("Pre-det sorted_index[%d] = %d\n",i,sorted_index[i]);
#endif
            continue;
            }
        for (k=0;k<2;k++)
            {
            for (jbest=-1,j=0;j<pageregions->n;j++)
                {
                if (box[j].ignore)
                    continue;
                if (j==sorted_index[i-1])
                    continue;
                if (box[j].prev >= 0)
                    continue;
                if (jbest<0 || pbox_position_compare(&box[j],&box[jbest]) < 0)
                    {
                    int t1,t2;
                    
                    t1=box[sorted_index[i-1]].next;
                    t2=box[j].prev;
                    box[sorted_index[i-1]].next = j;
                    box[j].prev = sorted_index[i-1];
                    if (k==1 || !trapped_box(sorted_index[i-1],box,pageregions->n,comax_fraction))
                        jbest=j;
                    box[sorted_index[i-1]].next =t1;
                    box[j].prev = t2;
                    }
                }
            if (jbest>=0)
                break;
            }
#if (WILLUSDEBUGX & 0x400000)
printf("JBEST=%d, k=%d",jbest,k);
#endif
        if (jbest>=0)
            {
            sorted_index[i]=jbest;
#if (WILLUSDEBUGX & 0x400000)
printf("Top-left sorted_index[%d] = %d\n",i,sorted_index[i]);
#endif
            box[sorted_index[i-1]].next = jbest;
            box[jbest].prev = sorted_index[i-1];
            continue;
            }
        for (jbest=-1,j=0;j<pageregions->n;j++)
            {
            if (j==sorted_index[i-1])
                continue;
            if (box[j].prev >= 0)
                continue;
            if (jbest<0 || pbox_position_compare(&box[j],&box[jbest]) < 0)
                jbest=j;
            }
        sorted_index[i]=jbest;
#if (WILLUSDEBUGX & 0x400000)
printf("Ignored top-left sorted_index[%d] = %d\n",i,sorted_index[i]);
#endif
        box[sorted_index[i-1]].next = jbest;
        box[jbest].prev = sorted_index[i-1];
        }

#if 0
    /*
    ** Types of sequence by rank:
    **     1. Below previous box, mostly aligned, with small enough gap. (optimal)
    **     2. Next to previous box with top alignment and smallest gap.
    **     3. Next to previous box with some vertical overlap and smallest gap
    **     4. Below previous box with some horizontal overlap and smallest gap
    **     5. Top-left of box is closest to top-left of page
    */
    for (isr=1;isr<pageregions->n;isr++)
        {
        int ibest0,ibesttype0,itries,ibest,igood;
        double rowgapbest,colalignbest;
        double gapbest,gapbest0;
        int ibesttype;
        PBOX *lastbox;

printf("isr=%d\n",isr);
        ibest0=ibesttype0=-1;
        gapbest0=99.;
        /* Find best next box until the best next box doesn't change (max 100 tries) */
        for (itries=0;itries<100;itries++)
            {
            lastbox=&box[sorted_index[isr-1]];         
            gapbest=99.;

            ibest = pbox_closest(box,region_done,pageregions->n,
                                 &ibesttype,&rowgapbest,&colalignbest,&gapbest);
            /* Look for best next region */
            for (ibest=-1,ibesttype=-1,rowgapbest=99.,igood=0,i=0;i<pageregions->n;i++)
                {
                double rg0,ca0;
                double dymean; /* Average height of two boxes */
                int vertical_overlap,horizontal_overlap;
                double gap; /* inches */
                double vo_amount; /* Vertical overlap amount; 1.0 = exact overlap */
                PBOX *box1,*box2;

                /* Select boxes to be compared */
                if (itries==0)
                    {
                    box1=lastbox;
                    box2=&box[i];
                    }
                else
                    {
                    box1=&box[i];
                    box2=&box[ibest0];
                    }
                if (region_done[i])
                    continue;
printf("    i=%d\n",i);
printf("        box1=");
pbox_echo(box1);
printf("        box2=");
pbox_echo(box2);
printf("        ibest=%d\n",ibest);
printf("        ibesttype=%d\n",ibesttype);
                vertical_overlap = (box2->y2 > box1->y1 && box2->y1 < box1->y2);
                if (vertical_overlap)
                    gap = box2->x1 > box1->x2 ? box2->x1-box1->x2 : box1->x1-box2->x2;
                dymean = 0.5*(box2->y2+box1->y2-box2->y1-box1->y1);
                if (vertical_overlap)
                    vo_amount = ((box2->y2 > box1->y2 ? box1->y2 : box2->y2)
                                  - (box2->y1 < box1->y1 ? box1->y1 : box2->y1)) / dymean;
                horizontal_overlap = (box2->x2 > box1->x1 && box2->x1 < box1->x2);
                if (horizontal_overlap)
                    gap = box2->y1 > box1->y2 ? box2->y1-box1->y2 : box1->y1-box2->y2;
                pbox_determine_closeness(box1,box2,&rg0,&ca0);
printf("        rowgap=%g, colalign=%g, rgmin=%g, rowgapbest=%g\n",rg0,ca0,rgapmin_inches,rowgapbest);
                /* Type 1:  look for column that matches user specs */
                if (rg0 >= 0. && rg0 <= rgapmin_inches && horizontal_overlap
                              && ca0 > 0. && 1-ca0 <= comax_fraction)
                    {
                    if (ibest<0 || ibesttype!=1 || rg0 < rowgapbest)
                        {
                        ibesttype=1;
                        ibest=i;
                        rowgapbest=rg0;
                        gapbest=gap;
                        colalignbest=ca0;
                        }
                    }
                /* Type 2:  Next to previous box with top alignment and smallest gap */
                if (ibesttype!=1)
                    {
                    if (vertical_overlap && 
                            ((box2->y2-box1->y2)/dymean < .05
                              || (box2->y1-box1->y1)/dymean < .05)
                             && gap < gapbest))
                        {
                        ibesttype=2;
                        ibest=i;
                        gapbest=gap;
                        rowgapbest=rg0;
                        colalignbest=ca0;
                        }
                    }
                /* Type 3:  Next to previous box with some vertical overlap and smallest gap */
                if (ibesttype!=1)
                    {
                    if (vertical_overlap && gap < gapbest && )
                    }
                /* Last resort--just go with the default position sorter */
                if (igood<1)
                    {
                    if (ibest<0 || pbox_position_compare(&box[i],&box[ibest])<0)
                        {
                        ibest=i;
                        rowgapbest=rg0;
                        colalignbest=ca0;
                        }
                    }
printf("       ...ibest for now=%d, igood=%d\n",ibest,igood);
                }
printf("    ibest_final = %d\n",ibest);
            /* If the best next PBOX did not change, select it. */
            if (ibest==ibest0)
                break;
            /* Otherwise, store and try again */
            ibest0=ibest;
            ibesttype0=ibesttype;
            }
        sorted_index[isr]=ibest;
        region_done[ibest]=1;
        }
#endif

    /* Order the page regions array according to the sorted_index[] array */
    for (isr=0;isr<pageregions->n;isr++)
        {
        PAGEREGION p0;

        if (sorted_index[isr]==isr)
            continue;
        p0=pageregions->pageregion[isr];
        pageregions->pageregion[isr] = pageregions->pageregion[sorted_index[isr]];
        pageregions->pageregion[sorted_index[isr]] = p0;
        for (j=isr+1;j<pageregions->n;j++)
            if (sorted_index[j]==isr)
                {
                sorted_index[j]=sorted_index[isr];
                break;
                }
        }
    willus_mem_free((double **)&box,funcname);
    willus_mem_free((double **)&sorted_index,funcname);
    }


/*
** Make sure there are no boxes mostly above box0
*/
static int trapped_box(int ibox,PBOX *box,int n,double comax)

    {
    int i,boxtrapped;
    int *trapped;
    static char *funcname="trapped_box";

    if (comax > .2)
        comax = .2;
    willus_mem_alloc_warn((void **)&trapped,sizeof(int)*n,funcname,10);
    for (i=0;i<n;i++)
        trapped[i]=0;
    /* Find first box in sequence */
    for (i=0;i<n && box[ibox].prev >= 0 && box[ibox].prev <= n-1;i++)
        {
        if (box[box[ibox].prev].next==ibox)
            ibox=box[ibox].prev;
        else
            break;
        }
    for (i=ibox;i>=0;i=box[i].next)
        trapped[i] = 255;
    while (ibox>=0)
        {
        double ibx1,ibx2,iby1,iby2;

        ibx1 = box[ibox].x1 + (box[ibox].x2-box[ibox].x1)*comax;
        ibx2 = box[ibox].x2 - (box[ibox].x2-box[ibox].x1)*comax;
        iby1 = box[ibox].y1 + (box[ibox].y2-box[ibox].y1)*comax;
        iby2 = box[ibox].y2 - (box[ibox].y2-box[ibox].y1)*comax;
        for (i=0;i<n;i++)
            {
            if (trapped[i]==255)
                continue;
            if (box[i].prev >= 0)
                continue;
            if (box[i].y2 > box[ibox].y1)
                continue;
            if (box[i].x2 < ibx1 || box[i].x1 > ibx2)
                continue;
            trapped[i] |= 1; /* This box is above one of the boxes */
#if (WILLUSDEBUGX & 0x400000)
printf("box %d (i) is above box %d (ibox)\n",i,ibox);
#endif
            }
        for (i=0;i<n;i++)
            {
            if (trapped[i]==255)
                continue;
            if (box[i].prev >= 0)
                continue;
            if (box[i].y1 < box[ibox].y2)
                continue;
            if (box[i].x2 < ibx1 || box[i].x1 > ibx2)
                continue;
            trapped[i] |= 8; /* This box is below one of the boxes */
#if (WILLUSDEBUGX & 0x400000)
printf("box %d (i) is below box %d (ibox)\n",i,ibox);
#endif
            }
        for (i=0;i<n;i++)
            {
            if (trapped[i]==255)
                continue;
            if (box[i].prev >= 0)
                continue;
            if (box[i].x2 > box[ibox].x1)
                continue;
            if (box[i].y1 > iby2 || box[i].y2 < iby1)
                continue;
            trapped[i] |= 2; /* This box is to the left of a box */
#if (WILLUSDEBUGX & 0x400000)
printf("box %d (i) is left of box %d (ibox)\n",i,ibox);
#endif
            }
        for (i=0;i<n;i++)
            {
            if (trapped[i]==255)
                continue;
            if (box[i].prev >= 0)
                continue;
            if (box[i].x1 < box[ibox].x2)
                continue;
            if (box[i].y1 > iby2 || box[i].y2 <iby1)
                continue;
            trapped[i] |= 4; /* This box is to the right of a box */
#if (WILLUSDEBUGX & 0x400000)
printf("box %d (i) is right of box %d (ibox)\n",i,ibox);
#endif
            }
        ibox=box[ibox].next;
        }
    for (boxtrapped=0,i=0;i<n;i++)
        if (trapped[i]!=255 && ((trapped[i]&3)==3 || (trapped[i]&5)==5 || (trapped[i]&10)==10))
            {
            boxtrapped=1;
#if (WILLUSDEBUGX & 0x400000)
            printf("BOX %d is TRAPPED (trapped=%d).\n",i,trapped[i]);
#endif
            break;
            }
    willus_mem_free((double **)&trapped,funcname);
    return(boxtrapped);
    }


/*
** Find closest next or previous PBOX in the array to the last box
** (Smallest gap with some overlap)
**
** Inputs:
**     lastbox = box we're trying to find the next closest box to
**     box[] = array of boxes to choose from
**     n = number of boxes in array
**     hoverlap = only test cases with horizontal overlap (otherwise only vertical overlap)
**
** Outputs:
**     alignbest = alignment between regions (1 = perfect, 0 = no overlap)
**     gapbest = gap between regions (inches)
**
*/
static int pbox_closest(PBOX *lastbox,PBOX *box,int n,int hoverlap,
                        double *alignbest,double *gapbest)

    {
    int i,ibest;
    double epsilon;

/*
printf("@pbox_closest box(ho=%d)=",hoverlap);
pbox_echo(lastbox);
*/
    epsilon = 0.1; /* Gap change (in inches) that is irrelevant */
    (*alignbest) = -1.0;
    (*gapbest) = 99.;
    /* Look for best next/previous region */
    for (ibest=-1,i=0;i<n;i++)
        {
        PBOX *box1,*box2;
        double calign,ralign,gap_inches;

        if (box[i].prev >= 0)
            continue;
        /* Select boxes to be compared */
        box1=lastbox;
        box2=&box[i];
/*
printf("    box[%d]=",i);
pbox_echo(box2);
printf("        gap=%g, calign=%g, ralign=%g\n",gap_inches,calign,ralign);
*/
        pbox_determine_closeness(box1,box2,&gap_inches,&calign,&ralign);
        if (gap_inches < 0.)
            continue;
        if ((hoverlap && calign<=0.) || (!hoverlap && ralign<=0.))
            continue;
        if (calign>0.)
            {
            if (ibest<0 || gap_inches < (*gapbest)-epsilon
                        || (fabs(gap_inches - (*gapbest))<=epsilon && box[ibest].x1 > box[i].x1))
                {
/*
printf("        ** BEST SO FAR **\n");
*/
                ibest=i;
                (*gapbest)=gap_inches;
                (*alignbest)=calign;
                }
            }
        else /* horizontally adjacent columns */
            {
            if (ibest<0 || gap_inches < (*gapbest)-epsilon
                        || (fabs(gap_inches - (*gapbest))<=epsilon && box2->y1<box[ibest].y1))
                {
/*
printf("        ** BEST SO FAR **\n");
*/
                ibest=i;
                (*gapbest)=gap_inches;
                (*alignbest)=ralign;
                }
            }
        }
/*
printf("        ibest=%d\n",ibest);
*/
    return(ibest);
    }


#if (WILLUSDEBUGX & 0x400000)
static void pbox_echo(PBOX *box)

    {
/*
    printf("(%4.1f,%4.1f) - (%4.1f,%4.1f), y1b=%5.2f, y2b=%5.2f, dylr=%4.2f\n",
           box->x1,box->y1,box->x2,box->y2,box->y1b,box->y2b,box->dylr);
*/
    printf("(%4.1f,%4.1f) - (%4.1f,%4.1f), prev=%d, next=%d\n",
           box->x1,box->y1,box->x2,box->y2,box->prev,box->next);
    printf("    fs=%6.3f %6.3f .. %6.3f %6.3f\n",box->fs[0],box->fs[1],box->fs[2],box->fs[3]);
    printf("    yb=%6.3f .. %6.3f\n",box->yb[1]-box->yb[0],box->yb[3]-box->yb[2]);
    }
#endif


static int pbox_position_compare(PBOX *box1,PBOX *box2)

    {
    int vertical_overlap,horizontal_overlap;

    vertical_overlap = (box2->y2 > box1->y1 && box2->y1 < box1->y2);
    horizontal_overlap = (box2->x2 > box1->x1 && box2->x1 < box1->x2);
    if (vertical_overlap)
        return(box1->x1 - box2->x1);
    if (horizontal_overlap)
        return(box1->y1 - box2->y1);
    return(box1->x1+box1->y1 - box2->x1+box2->y1);
    }


/*
** (*colalign) = 1.0 for perfect column alignment.  -1.0 for no overlap at all.
** (*rowgap) = gap between columns (0 = no gap).
*/
static void pbox_determine_closeness(PBOX *box0,PBOX *box1,double *gap_inches,
                                     double *colalign,double *rowalign)

    {
    (*colalign) = alignment(box0->x1,box0->x2,box1->x1,box1->x2);
    (*rowalign) = alignment(box0->y1,box0->y2,box1->y1,box1->y2);
    if ((*colalign) > 0.)
        (*gap_inches) = box1->y1-box0->y2;
    else
        (*gap_inches) = box1->x1-box0->x2;
    /*
    if (box0->y2b > 0. && box1->y1b > 0. && box0->dylr > 0.)
        {
        (*rowgap) = (box1->y1b - box0->y2b) - box0->dylr;
        if ((*rowgap)<0. && box1->y1b >= box0->y2b)
            (*rowgap) = 0.;
        }
    else
        (*rowgap) = box1->y1 - box0->y2;
    */
    }


/*
** box0->x1 -> x0
** box0->x2 -> x1
** box1->x1 -> x2
** box1->x2 -> x3
*/
static double alignment(double x0,double x1,double x2,double x3)

    {
    double w1,w0,wmax;
    double align;

    /* Width of box0 */
    w0 = x1-x0;
    if (w0 < .01)
        w0 = .01;
    /* Width of box1 */
    w1 = x3-x2;
    if (w1 < .01)
        w1 = .01;
    wmax = w1>w0 ? w1 : w0;
    if (x2 > x1)
        align = (double)(x1 - x2) / wmax; /* No overlap--negative result */
    else if (x3 < x0)
        align = (double)(x3 - x0) / wmax; /* No overlap--negative result */
    else if (x2 < x0)
        if (x3 > x1)
            align = (x1 - x0) / wmax;
        else
            align = (x3 - x0) / wmax;
    else
        if (x3 > x1)
            align = (x1 - x2) / wmax;
        else
            align = (x3 - x2) / wmax;
    return(align);
    } 
