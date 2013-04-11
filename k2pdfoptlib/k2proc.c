/*
** k2proc.c    These functions do the "heavy lifting" in k2pdfopt.  They
**             examine the source bitmap for contiguous regions, locating
**             columns, rows of text, and individual words, and laying out the
**             output pages.
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

static void pageregions_grid(PAGEREGIONS *pageregions,BMPREGION *region,
                      int *rowcount,int *colcount,
                      K2PDFOPT_SETTINGS *k2settings,int level);
static void pageregions_find_next_level(PAGEREGIONS *pageregions_sorted,BMPREGION *srcregion,
                      int *rowcount,int *colcount,
                      K2PDFOPT_SETTINGS *k2settings,int level);
static double median_val(double *x,int n);
static int bmpregion_find_multicolumn_divider(BMPREGION *region,
                                       K2PDFOPT_SETTINGS *k2settings,
                                       int *row_black_count,
                                       PAGEREGIONS *pageregions,
                                       int *colcount,int *rowcount);
static void bmpregion_vertically_break(BMPREGION *region,
                          K2PDFOPT_SETTINGS *k2settings,
                          MASTERINFO *masterinfo,
                          double force_scale,int *colcount,int *rowcount,
                          int colgap_pixels,int ncols);
static int different_widths(double width1,double width2);
static int different_row_heights(double height1,BREAKINFO *breakinfo,BMPREGION *region);
void bmpregion_analyze_justification_and_line_spacing(BMPREGION *region,
                            K2PDFOPT_SETTINGS *k2settings,
                            BREAKINFO *breakinfo,MASTERINFO *masterinfo,
                            int *colcount,int *rowcount,
                            int allow_text_wrapping,double force_scale);
void bmpregion_one_row_wrap_and_add(BMPREGION *region,
                                    K2PDFOPT_SETTINGS *k2settings,
                                    BREAKINFO *rowbreakinfo,
                                    int index,int i1,int i2,
                                    MASTERINFO *masterinfo,int justflags,
                                    int *colcount,int *rowcount,
                                    int line_spacing,int mean_row_gap,int rowbase,
                                    int marking_flags,int pi);


/*
** Call once per document
*/
void k2proc_init_one_document(void)

    {
    /* Init vert break routine */
    bmpregion_vertically_break(NULL,NULL,NULL,0.,NULL,NULL,0,0);
    }


/*
** Process full source page bitmap into rectangular regions and add
** to the destination bitmap.
**
** This function is no longer recursive as of v1.65.
**
*/
void bmpregion_source_page_add(BMPREGION *region,K2PDFOPT_SETTINGS *k2settings,
                               MASTERINFO *masterinfo,int level,int pages_done,
                               int colgap0_pixels)

    {
    static char *funcname="bmpregion_source_page_add";
    BMPREGION *srcregion,_srcregion;
    PAGEREGIONS *pageregions,_pageregions;
    int ipr,gridded,trim_regions;
    int *colcount,*rowcount;

    if (k2settings->debug)
        k2printf("@bmpregion_source_page_add (%d,%d) - (%d,%d) lev=%d\n",
               region->c1,region->r1,region->c2,region->r2,level);

    /* Set up rowcount, colcount arrays */
    willus_dmem_alloc_warn(1,(void **)&colcount,sizeof(int)*(region->c2+1),funcname,10);
    willus_dmem_alloc_warn(2,(void **)&rowcount,sizeof(int)*(region->r2+1),funcname,10);
    srcregion=&_srcregion;
    (*srcregion)=(*region);
    bmpregion_trim_margins(srcregion,k2settings,colcount,rowcount,k2settings->src_trim ? 0xf : 0);
    (*srcregion)=(*region);

    /* Find page regions */     
    pageregions=&_pageregions;
    pageregions_init(pageregions);
    gridded = (k2settings->src_grid_cols > 0 && k2settings->src_grid_rows > 0);
    if (gridded)
        pageregions_grid(pageregions,srcregion,rowcount,colcount,k2settings,0);
    else
        {
        int maxlevels;
        if (k2settings->max_columns<=1)
            maxlevels=1;
        else if (k2settings->max_columns<=2)
            maxlevels=2;
        else
            maxlevels=3;
        pageregions_find(pageregions,srcregion,rowcount,colcount,k2settings,maxlevels);
        }

    trim_regions = ((k2settings->vertical_break_threshold<-1.5
                       || k2settings->dst_fit_to_page==-2
                       || gridded)
                       && (k2settings->dst_userwidth_units==UNITS_TRIMMED
                           || k2settings->dst_userheight_units==UNITS_TRIMMED));


    /* Process page regions */
    if (k2settings->debug)
        k2printf("Page regions:  %d\n",pageregions->n);
    for (ipr=0;ipr<pageregions->n;ipr++)
        {
        int colgap_pixels,level,fitcols;
        BMPREGION *newregion;

        newregion=&pageregions->pageregion[ipr].bmpregion;

        /* Determine gap between regions */
        if (ipr>0)
            {
            BMPREGION *lastregion;
            lastregion=&pageregions->pageregion[ipr-1].bmpregion;
            colgap_pixels = newregion->r1-lastregion->r2;
            if (colgap_pixels < 0)
                colgap_pixels = colgap0_pixels;
            }
        else
            colgap_pixels = pages_done==0 ? 0 : colgap0_pixels;

        /* Check for dynamic adjustment of output page to trimmed source region */
        if (trim_regions)
            {
            /* Set device width/height to trimmed size if requested */
            if (k2settings->src_trim)
                bmpregion_trim_margins(newregion,k2settings,colcount,rowcount,0xf);
            k2pdfopt_settings_set_margins_and_devsize(k2settings,newregion,masterinfo,1);
            }

        /* Process this region */
        level = pageregions->pageregion[ipr].level;
        if (gridded || !pageregions->pageregion[ipr].fullspan)
            {
            level *= 2;
            fitcols = k2settings->fit_columns;
            }
        else
            fitcols = (k2settings->fit_columns && (level>1));
/*
printf("vert break region[%d], level=%d\n",ipr,level);
*/
        bmpregion_vertically_break(newregion,k2settings,masterinfo,fitcols?-2.0:-1.0,
                                   colcount,rowcount,colgap_pixels,level);

        /* Flush output if required */
        if (masterinfo->fit_to_page==-2)
            masterinfo_flush(masterinfo,k2settings);
        }
    pageregions_free(pageregions);
    willus_dmem_free(2,(double **)&rowcount,funcname);
    willus_dmem_free(1,(double **)&colcount,funcname);
    }


/*
** Set up gridded pageregion array
** (Blind Grid Output--no attempt to find breaks between rows or columns)
*/
static void pageregions_grid(PAGEREGIONS *pageregions,BMPREGION *region,
                      int *rowcount,int *colcount,
                      K2PDFOPT_SETTINGS *k2settings,int level)

    {
    int i,nr;
    BMPREGION *srcregion,_srcregion;

    srcregion=&_srcregion;
    (*srcregion)=(*region);
    nr=k2settings->src_grid_cols*k2settings->src_grid_rows;
    for (i=0;i<nr;i++)
        {
        int r,c,gw,gh,gwo,gho;

        gwo=(k2settings->src_grid_overlap_percentage*region->bmp8->width+region->bmp8->width/2)/100;
        gho=(k2settings->src_grid_overlap_percentage*region->bmp8->height+region->bmp8->height/2)/100;
        gw=region->bmp8->width/k2settings->src_grid_cols+gwo;
        gh=region->bmp8->height/k2settings->src_grid_rows+gho;
        if (k2settings->src_grid_order==0)
            {
            r=i%k2settings->src_grid_rows;
            c=i/k2settings->src_grid_rows;
            }
        else
            {
            r=i/k2settings->src_grid_cols;
            c=i%k2settings->src_grid_cols;
            }
        srcregion->c1=c*region->bmp8->width/k2settings->src_grid_cols-gwo/2;
        if (srcregion->c1<0)
            srcregion->c1=0;
        srcregion->c2=srcregion->c1+gw-1;
        if (srcregion->c2>region->bmp8->width-1)
            {
            srcregion->c2=region->bmp8->width-1;
            srcregion->c1=srcregion->c2-gw+1;
            if (srcregion->c1<0)
                srcregion->c1=0;
            }
        srcregion->r1=r*region->bmp8->height/k2settings->src_grid_rows-gho/2;
        if (srcregion->r1<0)
            srcregion->r1=0;
        srcregion->r2=srcregion->r1+gh-1;
        if (srcregion->r2>region->bmp8->height-1)
            {
            srcregion->r2=region->bmp8->height-1;
            srcregion->r1=srcregion->r2-gh+1;
            if (srcregion->r1<0)
                srcregion->r1=0;
            }
        pageregions_add_pageregion(pageregions,srcregion,level,0);
        }
    }


/*
** Return sorted list (by display order) of page regions up to appropriate level
** of recursion.
** maxlevels = 1:  One region
**             2:  Two columns
**             3:  Four columns
**             ...
**
*/
void pageregions_find(PAGEREGIONS *pageregions_sorted,BMPREGION *srcregion,
                      int *rowcount,int *colcount,K2PDFOPT_SETTINGS *k2settings,
                      int  maxlevels)

    {
    int ilevel;

    if (k2settings->debug)
        k2printf("@pageregions_find (%d,%d) - (%d,%d) maxlevels=%d\n",
               srcregion->c1,srcregion->r1,srcregion->c2,srcregion->r2,maxlevels);
    if (maxlevels==1)
        {
        pageregions_add_pageregion(pageregions_sorted,srcregion,1,1);
        return;
        }
    pageregions_find_next_level(pageregions_sorted,srcregion,rowcount,colcount,
                                k2settings,1);
    for (ilevel=2;ilevel<maxlevels;ilevel++)
        {
        int j;

        for (j=0;j<pageregions_sorted->n;j++)
            {
            /* If region is a candidate for sub-dividing, then do it */
            if (pageregions_sorted->pageregion[j].level==ilevel-1
                     && pageregions_sorted->pageregion[j].fullspan==0)
                {
                PAGEREGIONS *pageregions,_pageregions;

                pageregions=&_pageregions;
                pageregions_init(pageregions);
                /* Sub-divide region */
                pageregions_find_next_level(pageregions,
                                      &pageregions_sorted->pageregion[j].bmpregion,
                                      rowcount,colcount,k2settings,ilevel);
                /* Insert sub-divided regions into sorted array */
                pageregions_delete_one(pageregions_sorted,j);
                pageregions_insert(pageregions_sorted,j,pageregions);
                j--;
                }
            }
        }
    }
        

/*
** Find one level of page regions (splitting into multiple columns if found).
** Comes back sorted by display order.
**
*/
static void pageregions_find_next_level(PAGEREGIONS *pageregions_sorted,BMPREGION *srcregion,
                      int *rowcount,int *colcount,
                      K2PDFOPT_SETTINGS *k2settings,int level)

    {
    static char *funcname="pageregions_find_next_level";
    int *row_black_count;
    int rh,r0,cgr;
    PAGEREGIONS *pageregions,_pageregions;
    int ipr;

    if (k2settings->debug)
        k2printf("@pageregions_find_next_level (%d,%d) - (%d,%d) lev=%d\n",
               srcregion->c1,srcregion->r1,srcregion->c2,srcregion->r2,level);

    /*
    ** Store information about which rows are mostly clear for future
    ** processing (saves processing time).
    */
    willus_dmem_alloc_warn(4,(void **)&row_black_count,srcregion->bmp8->height*sizeof(int),
                           funcname,10);
    for (cgr=0,r0=0;r0<srcregion->bmp8->height;r0++)
        {
        row_black_count[r0]=bmpregion_row_black_count(srcregion,r0);
        if (row_black_count[r0]==0)
            cgr++;
        }
    if (k2settings->verbose)
        k2printf("%d clear rows.\n",cgr);
    if (k2settings->debug)
        bmpregion_row_histogram(srcregion);

    /* Unsorted array */
    pageregions=&_pageregions;
    pageregions_init(pageregions);

    /* Find all column dividers in source region and store sequentially in pageregion[] array */
    for (rh=0;srcregion->r1<=srcregion->r2;srcregion->r1+=rh)
        {
        rh=bmpregion_find_multicolumn_divider(srcregion,k2settings,row_black_count,
                                              pageregions,colcount,rowcount);
        if (k2settings->verbose)
            k2printf("rh=%d/%d\n",rh,srcregion->r2-srcregion->r1+1);
        }

    /* Sort pageregions array into pageregions_sorted array (by order of display) */
    if (k2settings->debug)
        k2printf("Page regions:  %d\n",pageregions->n);
    /* Clear sorted array */
    pageregions_sorted->n=0;
    for (ipr=0;ipr<pageregions->n;)
        {
        int jpr,colnum;

        for (colnum=1;colnum<=2;colnum++)
            {
            if (k2settings->debug)
                {
                k2printf("ipr = %d of %d...\n",ipr,pageregions->n);
                k2printf("COLUMN %d...\n",colnum);
                }
            for (jpr=ipr;jpr<pageregions->n;jpr+=2)
                {
                int index;
                PAGEREGION *pageregion;

                /* If we get to a page region that spans the entire source, stop */
                if (pageregions->pageregion[jpr].fullspan)
                    break;
                /* See if we should suspend this column and start displaying the next one */
                if (jpr>ipr)
                    {
                    double cpdiff,cdiv1,cdiv2,rowgap1_in,rowgap2_in;

                    if (k2settings->column_offset_max < 0.)
                        break;
                    /* Did column divider move too much? */
                    cdiv1=(pageregions->pageregion[jpr].bmpregion.c2
                             + pageregions->pageregion[jpr+1].bmpregion.c1) / 2.;
                    cdiv2=(pageregions->pageregion[jpr-2].bmpregion.c2
                             + pageregions->pageregion[jpr-1].bmpregion.c1) / 2.;
                    cpdiff=fabs((double)(cdiv1-cdiv2) / (srcregion->c2-srcregion->c1+1));
                    if (cpdiff>k2settings->column_offset_max)
                        break;
                    /* Is gap between this column region and next column region too big? */
                    rowgap1_in=(double)(pageregions->pageregion[jpr].bmpregion.r1
                                        - pageregions->pageregion[jpr-2].bmpregion.r2)
                                        / srcregion->dpi;
                    rowgap2_in=(double)(pageregions->pageregion[jpr+1].bmpregion.r1
                                        - pageregions->pageregion[jpr-1].bmpregion.r2)
                                        / srcregion->dpi;
                    if (rowgap1_in > 0.28 && rowgap2_in > 0.28)
                        break;
                    }
                index = k2settings->src_left_to_right ? jpr+colnum-1 : jpr+(2-colnum);
                pageregion=&pageregions->pageregion[index];
                /* 0 as last arg indicates it is one of two columns */
                pageregions_add_pageregion(pageregions_sorted,&pageregion->bmpregion,level,0);
                }
            if (jpr==ipr)
                break;
            }
        if (jpr<pageregions->n && pageregions->pageregion[jpr].fullspan)
            {
            if (k2settings->debug)
                k2printf("SINGLE COLUMN REGION...\n");
            pageregions_add_pageregion(pageregions_sorted,&pageregions->pageregion[jpr].bmpregion,
                                       level,1);
            jpr++;
            }
        ipr=jpr;
        }
    pageregions_free(pageregions);
    willus_dmem_free(4,(double **)&row_black_count,funcname);
    }



/*
**
** MAIN BITMAP REGION ADDING FUNCTION
**
** NOTE:  This function calls itself recursively!
**
** Input:  A generic rectangular region from the source file.  It will not
**         be checked for multiple columns, but the text may be wrapped
**         (controlled by allow_text_wrapping input).
**
** First, excess margins are trimmed off of the region.
**
** Then, if the resulting trimmed region is wider than the max desirable width
** and allow_text_wrapping is non-zero, then the
** bmpregion_analyze_justification_and_line_spacing() function is called.
** Otherwise the region is scaled to fit and added to the master set of pages.
**
** justification_flags
**     Bits 6-7:  0 = document is not fully justified
**                1 = document is fully justified
**                2 = don't know document justification yet
**     Bits 4-5:  0 = Use user settings
**                1 = fully justify
**                2 = do not fully justify
**     Bits 2-3:  0 = document is left justified
**                1 = document is centered
**                2 = document is right justified
**                3 = don't know document justification yet
**     Bits 0-1:  0 = left justify document
**                1 = center document
**                2 = right justify document
**                3 = Use user settings
**
** force_scale = -2.0 : Fit column width to display width
** force_scale = -1.0 : Use output dpi unless the region doesn't fit.
**                      In that case, scale it down until it fits.
** force_scale > 0.0  : Scale region by force_scale.
**
** mark_flags & 1 :  Mark top
** mark_flags & 2 :  Mark bottom
** mark_flags & 4 :  Mark left
** mark_flags & 8 :  Mark right
**
** trim_flags & 0x80 :  Do NOT re-trim no matter what.
**
*/
void bmpregion_add(BMPREGION *region,K2PDFOPT_SETTINGS *k2settings,
                   BREAKINFO *breakinfo,MASTERINFO *masterinfo,
                   int allow_text_wrapping,int trim_flags,
                   int allow_vertical_breaks,double force_scale,
                   int justification_flags,int caller_id,
                   int *colcount,int *rowcount,
                   int mark_flags,int rowbase_delta)

    {
    int w,wmax,i,nc,nr,h,bpp,tall_region;
    double region_width_inches;
    WILLUSBITMAP *bmp,_bmp;
    BMPREGION *newregion,_newregion;

    newregion=&_newregion;
    (*newregion)=(*region);
#if (WILLUSDEBUGX & 1)
k2printf("@bmpregion_add (%d,%d) - (%d,%d)\n",region->c1,region->r1,region->c2,region->r2);
k2printf("    trimflags = %X\n",trim_flags);
k2printf("    allow_text_wrapping = %d\n",allow_text_wrapping);
k2printf("    allow_vert_breaks = %d\n",allow_vertical_breaks);
#endif
    if (k2settings->debug)
        {
        if (!allow_text_wrapping)
            k2printf("@bmpregion_add (no break) (%d,%d) - (%d,%d) (scale=%g)\n",
                region->c1,region->r1,region->c2,region->r2,force_scale);
        else
            k2printf("@bmpregion_add (allow break) (%d,%d) - (%d,%d) (scale=%g)\n",
                region->c1,region->r1,region->c2,region->r2,force_scale);
        }
    /*
    ** Tag blank rows and columns and trim the blank margins off
    ** trimflags = 0xf for all margin trim.
    ** trimflags = 0xc for just top and bottom margins.
    */
    bmpregion_trim_margins(newregion,k2settings,colcount,rowcount,trim_flags);
#if (WILLUSDEBUGX & 1)
k2printf("    After trim:  (%d,%d) - (%d,%d)\n",newregion->c1,newregion->r1,newregion->c2,newregion->r2);
#endif
    nc=newregion->c2-newregion->c1+1;
    nr=newregion->r2-newregion->r1+1;
// k2printf("nc=%d, nr=%d\n",nc,nr);
    if (k2settings->verbose)
        {
        k2printf("    row range adjusted to %d - %d\n",newregion->r1,newregion->r2);
        k2printf("    col range adjusted to %d - %d\n",newregion->c1,newregion->c2);
        }
    if (nc<=5 || nr<=1)
        return;
    region_width_inches = (double)nc/newregion->dpi;
// k2printf("bmpregion_add:  rwidth_in = %.2f = %d / %d\n",region_width_inches,nc,newregion->dpi);
    /* Use untrimmed region left/right if possible */
    if (caller_id==1 && region_width_inches <= k2settings->max_region_width_inches)
        {
        int trimleft,trimright;
        int maxpix,dpix;

        maxpix = (int)(k2settings->max_region_width_inches*newregion->dpi+.5);
#if (WILLUSDEBUGX & 1)
k2printf("    Trimming.  C's = %4d %4d %4d %4d\n",region->c1,newregion->c1,newregion->c2,region->c2);
k2printf("    maxpix = %d, regwidth = %d\n",maxpix,region->c2-region->c1+1);
#endif
        if (maxpix > (region->c2-region->c1+1))
            maxpix = region->c2-region->c1+1;
// k2printf("    maxpix = %d\n",maxpix);
        dpix = (region->c2-region->c1+1 - maxpix)/2;
// k2printf("    dpix = %d\n",dpix);
        trimright = region->c2-newregion->c2;
        trimleft = newregion->c1-region->c1;
        if (trimleft<trimright)
            {
            if (trimleft > dpix)
                newregion->c1 = region->c1+dpix;
            newregion->c2 = newregion->c1+maxpix-1;
            }
        else
            {
            if (trimright > dpix)
                newregion->c2 = region->c2-dpix;
            newregion->c1 = newregion->c2-maxpix+1;
            }
        if (newregion->c1 < region->c1)
            newregion->c1 = region->c1;
        if (newregion->c2 > region->c2)
            newregion->c2 = region->c2;
        nc=newregion->c2-newregion->c1+1;
#if (WILLUSDEBUGX & 1)
k2printf("    Post Trim.  C's = %4d %4d %4d %4d\n",region->c1,newregion->c1,newregion->c2,region->c2);
#endif
        region_width_inches = (double)nc/newregion->dpi;
        }
        
    /*
    ** Try breaking the region into smaller horizontal pieces (wrap text lines)
    */
/*
k2printf("allow_text_wrapping=%d, region_width_inches=%g, max_region_width_inches=%g\n",
allow_text_wrapping,region_width_inches,k2settings->max_region_width_inches);
*/
    /* New in v1.50, if allow_text_wrapping==2, unwrap short lines. */
/*
k2printf("tw=%d, region_width_inches=%g, max_region_width_inches=%g\n",allow_text_wrapping,region_width_inches,k2settings->max_region_width_inches);
*/
    if (allow_text_wrapping==2 
         || (allow_text_wrapping==1 && region_width_inches > k2settings->max_region_width_inches))
        {
        bmpregion_analyze_justification_and_line_spacing(newregion,k2settings,breakinfo,
                                     masterinfo,colcount,rowcount,1,force_scale);
        return;
        }

    /*
    ** If allowed, re-submit each vertical region individually
    */
    if (allow_vertical_breaks)
        {
        bmpregion_analyze_justification_and_line_spacing(newregion,k2settings,breakinfo,
                                      masterinfo,colcount,rowcount,0,force_scale);
        return;
        }

    /* AT THIS POINT, BITMAP IS NOT TO BE BROKEN UP HORIZONTALLY OR VERTICALLY */
    /* (IT CAN STILL BE FULLY JUSTIFIED IF ALLOWED.) */

    /*
    ** Scale region to fit the destination device width and add to the master bitmap.
    **
    **
    ** Start by copying source region to new bitmap 
    **
    */
// k2printf("c1=%d\n",newregion->c1);
    /* Is it a figure? */
    tall_region = (double)(newregion->r2-newregion->r1+1)/newregion->dpi >= k2settings->dst_min_figure_height_in;
    /* Re-trim left and right? */
    if ((trim_flags&0x80)==0)
        {
        /* If tall region and figure justification turned on ... */
        if ((tall_region && k2settings->dst_figure_justify>=0)
                /* ... or if centered region ... */
                || ((trim_flags&3)!=3 && ((justification_flags&3)==1
                     || ((justification_flags&3)==3
                     && (k2settings->dst_justify==1
                         || (k2settings->dst_justify<0 && (justification_flags&0xc)==4))))))
            {
            bmpregion_trim_margins(newregion,k2settings,colcount,rowcount,0x3);
            nc=newregion->c2-newregion->c1+1;
            region_width_inches = (double)nc/newregion->dpi;
            }
        }
#if (WILLUSDEBUGX & 1)
    k2printf("atomic region:  " ANSI_CYAN "%.2f x %.2f in" ANSI_NORMAL " c1=%d, (%d x %d) (rbdel=%d) just=0x%02X\n",
                   (double)(newregion->c2-newregion->c1+1)/newregion->dpi,
                   (double)(newregion->r2-newregion->r1+1)/newregion->dpi,
                   newregion->c1,
                   (newregion->c2-newregion->c1+1),
                   (newregion->r2-newregion->r1+1),
                   rowbase_delta,justification_flags);
#endif
    /* Copy atomic region into bmp */
    bmp=&_bmp;
    bmp_init(bmp);
    bmp->width=nc;
    bmp->height=nr;
    if (k2settings->dst_color)
        bmp->bpp=24;
    else
        {
        bmp->bpp=8;
        for (i=0;i<256;i++)
            bmp->red[i]=bmp->blue[i]=bmp->green[i]=i;
        }
    bmp_alloc(bmp);
    bpp = k2settings->dst_color ? 3 : 1;
// k2printf("r1=%d, r2=%d\n",newregion->r1,newregion->r2);
    for (i=newregion->r1;i<=newregion->r2;i++)
        {
        unsigned char *psrc,*pdst;

        pdst=bmp_rowptr_from_top(bmp,i-newregion->r1);
        psrc=bmp_rowptr_from_top(k2settings->dst_color ? newregion->bmp : newregion->bmp8,i)+bpp*newregion->c1;
        memcpy(pdst,psrc,nc*bpp);
        }
    /*
    ** Now scale to appropriate destination size.
    **
    ** force_scale is used to maintain uniform scaling so that
    ** most of the regions are scaled at the same value.
    **
    ** force_scale = -2.0 : Fit column width to display width
    ** force_scale = -1.0 : Use output dpi unless the region doesn't fit.
    **                      In that case, scale it down until it fits.
    ** force_scale > 0.0  : Scale region by force_scale.
    **
    */
    /* Max viewable pixel width on device screen */
    wmax=(int)(masterinfo->bmp.width-(k2settings->dst_marleft+k2settings->dst_marright)*k2settings->dst_dpi+0.5);
    if (force_scale > 0.)
        w = (int)(force_scale*bmp->width+0.5);
    else
        {
        if (region_width_inches < k2settings->max_region_width_inches)
            w=(int)(region_width_inches*k2settings->dst_dpi+.5);
        else
            w=wmax;
        }
    /* Special processing for tall regions (likely figures) */
    if (tall_region && w < wmax && k2settings->dst_fit_to_page!=0)
        {
        if (k2settings->dst_fit_to_page<0)
            w = wmax;
        else
            {
            w = (int)(w * (1.+(double)k2settings->dst_fit_to_page/100.) + 0.5);
            if (w > wmax)
                w = wmax;
            }
        }
    h=(int)(((double)w/bmp->width)*bmp->height+.5);

    /*
    ** If scaled dimensions are finite, add to master bitmap.
    */
    if (w>0 && h>0)
        {
        WILLUSBITMAP *tmp,_tmp;
        int nocr,have_pagebox;

        have_pagebox=0;
        k2settings->last_scale_factor_internal=(double)w/bmp->width;
        /* Leave bitmap close to full resolution to scan for line spacing */
        /* (New in v1.65 -- used to just be for OCR) */
        if (k2settings->vertical_break_threshold < 0
#ifdef HAVE_OCR_LIB
              || k2settings->dst_ocr
#endif
                                     )
            {
            nocr=(int)((double)bmp->width/w+0.5);
            if (nocr < 1)
                nocr=1;
            if (nocr > 10)
                nocr=10;
            w *= nocr;
            h *= nocr;
            }
        else
            nocr=1;
        tmp=&_tmp;
        bmp_init(tmp);
        bmp_resample(tmp,bmp,(double)0.,(double)0.,(double)bmp->width,(double)bmp->height,w,h);
        bmp_free(bmp);
/*
{
static int nn=0;
char filename[256];
sprintf(filename,"xxx%02d.png",nn++);
bmp_write(tmp,filename,stdout,100);
}
*/
        /*
        ** Add scaled bitmap to destination.
        */
        /* Allocate more rows if necessary */
        while (masterinfo->rows+tmp->height/nocr > masterinfo->bmp.height)
            bmp_more_rows(&masterinfo->bmp,1.4,255);
        /* Check special justification for tall regions */
        if (tall_region && k2settings->dst_figure_justify>=0)
            justification_flags = k2settings->dst_figure_justify;
#ifdef HAVE_MUPDF_LIB
        /* Add source region corresponding to "tmp" bitmap to pageinfo structure */
        if (k2settings->use_crop_boxes)
            {
            WPDFBOX _wpdfbox,*wpdfbox;
            WPDFSRCBOX *srcbox;
            WPDFPAGEINFO *pageinfo;

            pageinfo=&masterinfo->pageinfo;
            wpdfbox=&_wpdfbox;
            srcbox=&wpdfbox->srcbox;
            wpdfbox->dstpage = -1; /* -1 while still on master bitmap */
            wpdfbox->dst_width_pts = pageinfo->width_pts;
            wpdfbox->dst_height_pts = pageinfo->height_pts;
            srcbox->pageno = pageinfo->srcpage;
            srcbox->finerot_deg = pageinfo->srcpage_fine_rot_deg;
            srcbox->rot_deg = pageinfo->srcpage_rot_deg;
            srcbox->page_width_pts = 72.*newregion->bmp8->width/newregion->dpi;
            srcbox->page_height_pts = 72.*newregion->bmp8->height/newregion->dpi;
            /* Clip the source crop box with the page crop margins */
            {
            BMPREGION *region,_region;
            double x0,y0,w,h,mar;
 
            region=&_region;
            region->bmp = newregion->bmp;
            region->dpi = newregion->dpi;
            bmpregion_trim_to_crop_margins(region,k2settings);
            x0 = 72.*newregion->c1/newregion->dpi;
            y0 = 72.*(newregion->bmp8->height-1-newregion->r2)/newregion->dpi;
            w = 72.*(newregion->c2-newregion->c1+1)/newregion->dpi;
            h = 72.*(newregion->r2-newregion->r1+1)/newregion->dpi;
            mar=region->c1*srcbox->page_width_pts/newregion->bmp->width;
            if (mar>x0)
                {
                w -= (mar-x0);
                x0=mar;
                }
            mar=(newregion->bmp->width-1-region->c2)*srcbox->page_width_pts/newregion->bmp->width;
            if (w > srcbox->page_width_pts-mar-x0)
                w = srcbox->page_width_pts-mar-x0;
            mar=(newregion->bmp->height-1-region->r2)*srcbox->page_height_pts/newregion->bmp->height;
            if (mar>y0)
                {
                h -= (mar-y0);
                y0=mar;
                }
            mar=region->r1*srcbox->page_height_pts/newregion->bmp->height;
            if (h > srcbox->page_height_pts-mar-y0)
                h = srcbox->page_height_pts-mar-y0;
            srcbox->x0_pts = x0;
            srcbox->y0_pts = y0;
            srcbox->crop_width_pts = w;
            srcbox->crop_height_pts = h;
            }
            if (srcbox->crop_width_pts > 0. && srcbox->crop_height_pts > 0.)
                {
                wpdfboxes_add_box(&pageinfo->boxes,wpdfbox);
                have_pagebox=1;
                }
            }
#endif /* HAVE_MUPDF_LIB */
        masterinfo_add_bitmap(masterinfo,tmp,k2settings,have_pagebox,justification_flags,
                       region->bgcolor,nocr,(int)((double)region->dpi*tmp->width/bmp->width+.5));
        bmp_free(tmp);
        }

    /* Store delta to base of text row (used by wrapbmp_flush()) */
    k2settings->last_rowbase_internal = rowbase_delta;
    }


/*
** Returns height of region found and divider position in (*divider_column).
** (*divider_column) is absolute position on source bitmap.
**
*/
static int bmpregion_find_multicolumn_divider(BMPREGION *region,
                                       K2PDFOPT_SETTINGS *k2settings,
                                       int *row_black_count,
                                       PAGEREGIONS *pageregions,
                                       int *colcount,int *rowcount)

    {
    int itop,i,dm,middle,divider_column,min_height_pixels,mhp2,min_col_gap_pixels;
    BMPREGION _newregion,*newregion,column[2];
    BREAKINFO *breakinfo,_breakinfo;
    int *rowmin,*rowmax;
    int *black_pixel_count_by_column;
    int *pixel_count_array;
    int rows_per_column;
    static char *funcname="bmpregion_find_multicolumn_divider";

    if (k2settings->debug)
        k2printf("@bmpregion_find_multicolumn_divider(%d,%d)-(%d,%d)\n",
                 region->c1,region->r1,region->c2,region->r2);
    breakinfo=&_breakinfo;
    breakinfo->textrow=NULL;
    breakinfo_alloc(101,breakinfo,region->r2-region->r1+1);
    bmpregion_find_vertical_breaks(region,breakinfo,k2settings,colcount,rowcount,
                                   k2settings->column_row_gap_height_in);
    if (k2settings->debug)
        {
        k2printf("region (%d,%d)-(%d,%d) has %d breaks:\n",
                region->c1,region->r1,region->c2,region->r2,breakinfo->n);
        for (i=0;i<breakinfo->n;i++)
            k2printf("    Rows %d - %d\n",breakinfo->textrow[i].r1,breakinfo->textrow[i].r2);
        }
    newregion=&_newregion;
    (*newregion)=(*region);
    min_height_pixels=k2settings->min_column_height_inches*region->dpi; /* src->height/15; */ 
    mhp2 = min_height_pixels-1;
    if (mhp2 < 0)
        mhp2=0;
    dm=1+(region->c2-region->c1+1)*k2settings->column_gap_range/2.;
    middle=(region->c2-region->c1+1)/2;
    min_col_gap_pixels=(int)(k2settings->min_column_gap_inches*region->dpi+.5);
    if (k2settings->verbose)
        {
        k2printf("(dm=%d, width=%d, min_gap=%d)\n",dm,region->c2-region->c1+1,min_col_gap_pixels);
        k2printf("Checking regions (r1=%d, r2=%d, minrh=%d)..",region->r1,region->r2,min_height_pixels);
        fflush(stdout);
        }
    breakinfo_sort_by_row_position(breakinfo);
    willus_dmem_alloc_warn(5,(void **)&rowmin,(region->c2+10)*3*sizeof(int),funcname,10);
    rowmax=&rowmin[region->c2+10];
    black_pixel_count_by_column=&rowmax[region->c2+10];
    rows_per_column=0;

    /*
    ** If enough memory cache pixel counts into large 2-D array for fast calcs
    */
    pixel_count_array=NULL;
    rows_per_column=0;
    if (willus_mem_alloc((double **)&pixel_count_array,sizeof(int)*(region->c2+2)*(region->r2+2),
                          funcname))
        {
        int bw;

        rows_per_column=region->r2+2;
        memset(pixel_count_array,0,sizeof(int)*(region->c2+2)+(region->r2+2));
        bw=bmp_bytewidth(region->bmp8);
        for (i=0;i<=region->c2+1;i++)
            {
            unsigned char *p;
            int *cp;
            int j;

            if (i>=region->bmp8->width)
                continue;
            cp=&pixel_count_array[i*rows_per_column];
            p=bmp_rowptr_from_top(region->bmp8,0)+i;
            cp[0] = (p[0]<region->bgcolor) ? 1 : 0;
            for (p+=bw,cp++,j=1;j<region->r2+2;j++,p+=bw,cp++)
                if (p[0]<region->bgcolor)
                    (*cp)=cp[-1]+1;
                else
                    (*cp)=cp[-1];
            }
        }
        
    /*
    ** Populate black pixel count by column
    */
    for (i=0;i<region->c1;i++)
        black_pixel_count_by_column[i]=1;
    for (i=region->c1;i<=region->c2;i++)
        black_pixel_count_by_column[i]=bmpregion_col_black_count(region,i);
    for (i=region->c2+1;i<region->c2+2;i++)
        black_pixel_count_by_column[i]=1;
    /*
    ** Init rowmin[], rowmax[] arrays
    */
    for (i=0;i<region->c2+2;i++)
        {
        rowmin[i]=region->r2+2;
        rowmax[i]=-1;
        }
    /* Un-trim top/bottom rows if requested */
    if (!k2settings->src_trim && breakinfo->n>0)
        {
        breakinfo->textrow[0].r1=region->r1;
        breakinfo->textrow[breakinfo->n-1].r2=region->r2;
        }

    /* Start with top-most and bottom-most regions, look for column dividers */
    for (itop=0;itop<breakinfo->n 
                      && breakinfo->textrow[itop].r1<region->r2+1-min_height_pixels;itop++)
        {
        int ibottom;
#if (WILLUSDEBUGX & 128)
k2printf("itop=%d/%d\n",itop,breakinfo->n);
#endif

        for (ibottom=breakinfo->n-1;ibottom>=itop 
              && breakinfo->textrow[ibottom].r2-breakinfo->textrow[itop].r1 >= min_height_pixels;
              ibottom--)
            {
            int ileft,iright;

#if (WILLUSDEBUGX & 128)
int qec,lec,colmin,colmax;
#endif
            /*
            ** Look for vertical shaft of clear space that clearly demarcates
            ** two columns
            */
#if (WILLUSDEBUGX & 128)
qec=lec=0;
colmin=99999;
colmax=0;
k2printf("    ibot=%d/%d (dm=%d)\n",ibottom,breakinfo->n,dm);
#endif
            /*
            ** ileft and iright keep track of column shafts we've already checked
            */
            ileft=region->c1+middle;
            iright=region->c1+middle;
            for (i=0;i<dm;i++)
                {
                int foundgap,ii,c1,c2,iiopt,status;

                newregion->c1=region->c1+middle-i;
#if (WILLUSDEBUGX & 128)
if (newregion->c1<colmin)
colmin=newregion->c1;
if (newregion->c1>colmax)
colmax=newregion->c1;
#endif
// k2printf("        c1=%d (%d - %d)\n",newregion->c1,region->c1,region->c2);
                /* If we've effectively already checked this shaft, move on */
                if (newregion->c1>ileft 
                        || (itop >= rowmin[newregion->c1] && ibottom <= rowmax[newregion->c1]))
#if (WILLUSDEBUGX & 128)
{
qec++;
#endif
                    continue;
#if (WILLUSDEBUGX & 128)
}
lec++;
#endif
                ileft=newregion->c1;
                newregion->c2=newregion->c1+min_col_gap_pixels-1;
                newregion->r1=breakinfo->textrow[itop].r1;
                newregion->r2=breakinfo->textrow[ibottom].r2;
                foundgap=bmpregion_is_clear(newregion,row_black_count,black_pixel_count_by_column,
                                             pixel_count_array,rows_per_column,k2settings->gtc_in);
                if (!foundgap && i>0)
                    {
                    newregion->c1=region->c1+middle+i;
#if (WILLUSDEBUGX & 128)
if (newregion->c1<colmin)
colmin=newregion->c1;
if (newregion->c1>colmax)
colmax=newregion->c1;
#endif
                    if (newregion->c1<iright
                            || (itop >= rowmin[newregion->c1] && ibottom <= rowmax[newregion->c1]))
#if (WILLUSDEBUGX & 128)
{
qec++;
#endif
                        continue;
#if (WILLUSDEBUGX & 128)
}
lec++;
#endif
                    iright=newregion->c1;
                    newregion->c2=newregion->c1+min_col_gap_pixels-1;
                    foundgap=bmpregion_is_clear(newregion,row_black_count,
                                           black_pixel_count_by_column,
                                           pixel_count_array,rows_per_column,
                                           k2settings->gtc_in);
                    }
                if (!foundgap)
                    continue;
                /* Found a gap, but look for a better gap nearby */
                c1=newregion->c1;
                c2=newregion->c2;
                for (iiopt=0,ii=-min_col_gap_pixels;ii<=min_col_gap_pixels;ii++)
                    {
                    int newgap;

                    newregion->c1=c1+ii;
                    /* New checks in v1.65 to prevent array-out-of-bounds */
                    if (newregion->c1 < region->c1 || newregion->c1 > region->c2)
                        continue;
                    newregion->c2=c2+ii;
                    if (newregion->c2 < region->c1 || newregion->c2 > region->c2)
                        continue;
                    newgap=bmpregion_is_clear(newregion,row_black_count,
                                             black_pixel_count_by_column,
                                             pixel_count_array,rows_per_column,
                                             k2settings->gtc_in);
                    if (newgap>0 && newgap<foundgap)
                        {
                        iiopt=ii;
                        foundgap=newgap;
                        if (newgap==1)
                            break;
                        }
                    }
                newregion->c1=c1+iiopt;
                /* If we've effectively already checked this shaft, move on */
                if (itop >= rowmin[newregion->c1] && ibottom <= rowmax[newregion->c1])
                    continue;
                newregion->c2=c2+iiopt;
                divider_column=newregion->c1+min_col_gap_pixels/2;
                status=bmpregion_column_height_and_gap_test(column,region,k2settings,
                                       breakinfo->textrow[itop].r1,
                                       breakinfo->textrow[ibottom].r2,
                                       divider_column,
                                       colcount,rowcount);
                /* After trimming the two columns, adjust ileft and iright */
                if (column[0].c2+1 < ileft)
                    ileft = column[0].c2+1;
                if (column[1].c1-min_col_gap_pixels > iright)
                    iright = column[1].c1-min_col_gap_pixels;
                /* If fails column height or gap test, mark as bad */
                if (status)
                    {
                    if (itop < rowmin[newregion->c1])
                        rowmin[newregion->c1]=itop;
                    if (ibottom > rowmax[newregion->c1])
                        rowmax[newregion->c1]=ibottom;
                    }
                /* If right column too short, stop looking */
                if (status&2)
                    break;
                if (!status)
                    {
                    int colheight;

/* k2printf("    GOT COLUMN DIVIDER AT x=%d.\n",(*divider_column)); */
                    if (k2settings->verbose)
                        {
                        k2printf("\n    GOOD REGION: col gap=(%d,%d) - (%d,%d)\n"
                             "                 r1=%d, r2=%d\n",
                            newregion->c1,newregion->r1,newregion->c2,newregion->r2,
                            breakinfo->textrow[itop].r1,breakinfo->textrow[ibottom].r2);
                        }
                    if (itop>0)
                        {
                        BMPREGION *br;
                        /* add 1-column (full span) region */
                        pageregions_add_pageregion(pageregions,region,0,1);
                        br=&pageregions->pageregion[pageregions->n-1].bmpregion;
                        br->r2=breakinfo->textrow[itop-1].r2;
                        if (br->r2 > br->bmp8->height-1)
                            br->r2 = br->bmp8->height-1;
                        bmpregion_trim_margins(br,k2settings,colcount,rowcount,
                                               k2settings->src_trim?0xf:0);
                        }
                    /* Un-trim columns if requested */
                    if (!k2settings->src_trim)
                        {
                        column[0].c1=region->c1;
                        column[1].c2=region->c2;
                        }
                    /* Add two side-by-side columns */
                    pageregions_add_pageregion(pageregions,&column[0],0,0);
                    pageregions_add_pageregion(pageregions,&column[1],0,0);
                    colheight = breakinfo->textrow[ibottom].r2-region->r1+1;
                    willus_mem_free((double **)&pixel_count_array,funcname);
                    willus_dmem_free(5,(double **)&rowmin,funcname);
                    breakinfo_free(101,breakinfo);
/*
k2printf("Returning %d divider column = %d - %d\n",region->r2-region->r1+1,newregion->c1,newregion->c2);
*/
                    return(colheight);
                    }
                }
#if (WILLUSDEBUGX & 128)
k2printf("        cols %d - %d, qec = %d, lec = %d\n",colmin,colmax,qec,lec);
#endif
            }
        }
    if (k2settings->verbose)
        k2printf("NO GOOD REGION FOUND.\n");
    /* Add entire region (full span) */
    pageregions_add_pageregion(pageregions,region,0,1);
    {
    BMPREGION *br;
    br=&pageregions->pageregion[pageregions->n-1].bmpregion;
    bmpregion_trim_margins(br,k2settings,colcount,rowcount,k2settings->src_trim?0xf:0);
    }
    /* (*divider_column)=region->c2+1; */
    willus_mem_free((double **)&pixel_count_array,funcname);
    willus_dmem_free(5,(double **)&rowmin,funcname);
    breakinfo_free(101,breakinfo);
/*
k2printf("Returning %d\n",region->r2-region->r1+1);
*/
    return(region->r2-region->r1+1);
    }


/*
** Input:  A generic rectangular region from the source file.  It will not
**         be checked for multiple columns, but the text may be wrapped
**         (controlled by allow_text_wrapping input).
**
** force_scale == -2 :  Use same scale for entire region/column--fit to device
**
** This function looks for vertical gaps in the region and breaks it at
** the largest vertical gaps (if there are significantly larger gaps than the
** typical gap--indicating section breaks in the document).
**
*/
static void bmpregion_vertically_break(BMPREGION *region,
                          K2PDFOPT_SETTINGS *k2settings,
                          MASTERINFO *masterinfo,
                          double force_scale,int *colcount,int *rowcount,
                          int colgap_pixels,int ncols)

    {
    static int ncols_last=-1;
    static double last_region_width_inches=-1.;
    static double last_region_last_row_height_inches=-1.;
    int regcount,i,i1,biggap,revert,trim_flags,allow_vertical_breaks;
    int justification_flags,caller_id,marking_flags,rbdelta,allow_text_wrapping;
    // int trim_left_and_right;
    BMPREGION *bregion,_bregion;
    BREAKINFO *breakinfo,_breakinfo;
    WRAPBMP *wrapbmp;
    double region_width_inches,region_height_inches;

    if (region==NULL)
        {
        ncols_last = -1;
        last_region_width_inches = -1.;
        last_region_last_row_height_inches = -1.;
        return;
        }
    wrapbmp=&masterinfo->wrapbmp;
#if (WILLUSDEBUGX & 1)
k2printf("\n\n@bmpregion_vertically_break.  colgap_pixels=%d\n\n",colgap_pixels);
k2printf("    region = (%d,%d) - (%d,%d)\n",region->c1,region->r1,region->c2,region->r2);
k2printf("    vertical_break_threshold=%g\n",k2settings->vertical_break_threshold);
#endif
    /*
    ** text_wrap==0 no wrapping
    ** text_wrap==1 re-flow
    ** text_wrap==2 to re-flow short lines.
    */
    allow_text_wrapping=k2settings->text_wrap;
    allow_vertical_breaks=(k2settings->vertical_break_threshold > -1.5);
    justification_flags=0x8f; /* Don't know region justification status yet.  Use user settings. */
    rbdelta=-1;
    breakinfo=&_breakinfo;
    breakinfo->textrow=NULL;
    breakinfo_alloc(102,breakinfo,region->r2-region->r1+1);
    bmpregion_find_vertical_breaks(region,breakinfo,k2settings,colcount,rowcount,-1.0);
    /* Should there be a check for breakinfo->n==0 here? */
    /* Don't think it breaks anything to let it go.  -- 6-11-12 */
#if (WILLUSDEBUGX & 2)
breakinfo_echo(breakinfo);
#endif
    breakinfo_remove_small_rows(breakinfo,k2settings,0.25,0.5,region,colcount,rowcount);
#if (WILLUSDEBUGX & 2)
breakinfo_echo(breakinfo);
#endif
    breakinfo->centered=bmpregion_is_centered(region,k2settings,breakinfo,0,breakinfo->n-1,NULL);
#if (WILLUSDEBUGX & 2)
breakinfo_echo(breakinfo);
#endif

    /* Red, numbered region */
    mark_source_page(k2settings,region,1,0xf);
    bregion=&_bregion;
    if (k2settings->debug)
        {
        if (!allow_text_wrapping)
            k2printf("@bmpregion_vertically_break (no break) (%d,%d) - (%d,%d) (scale=%g)\n",
                region->c1,region->r1,region->c2,region->r2,force_scale);
        else
            k2printf("@bmpregion_vertically_break (allow break) (%d,%d) - (%d,%d) (scale=%g)\n",
                region->c1,region->r1,region->c2,region->r2,force_scale);
        }
    /*
    ** Tag blank rows and columns
    */
    if (k2settings->vertical_break_threshold<0. || breakinfo->n < 6)
        biggap = -1.;
    else
        {
        int gap_median;
/*
        int rowheight_median;

        breakinfo_sort_by_rowheight(breakinfo);
        rowheight_median = breakinfo->textrow[breakinfo->n/2].rowheight;
*/
#ifdef WILLUSDEBUG
for (i=0;i<breakinfo->n;i++)
k2printf("    gap[%d]=%d\n",i,breakinfo->textrow[i].gap);
#endif
        breakinfo_sort_by_gap(breakinfo);
        gap_median = breakinfo->textrow[breakinfo->n/2].gap;
#ifdef WILLUSDEBUG
k2printf("    median=%d\n",gap_median);
#endif
        biggap = gap_median*k2settings->vertical_break_threshold;
        breakinfo_sort_by_row_position(breakinfo);
        }
#ifdef WILLUSDEBUG
k2printf("    biggap=%d\n",biggap);
#endif
    region_width_inches = (double)(region->c2-region->c1+1)/region->dpi;
    region_height_inches = (double)(region->r2-region->r1+1)/region->dpi;
    /*
    trim_left_and_right = 1;
    if (region_width_inches <= k2settings->max_region_width_inches)
        trim_left_and_right = 0;
    */
/*
k2printf("force_scale=%g, rwi = %g, rwi/mrwi = %g, rhi = %g\n",
force_scale,
region_width_inches,
region_width_inches / k2settings->max_region_width_inches,
region_height_inches);
*/
    if (force_scale < -1.5 && region_width_inches > MIN_REGION_WIDTH_INCHES
                         && region_width_inches/k2settings->max_region_width_inches < 1.25
                         && region_height_inches > 0.5)
        {
        revert=1;
        force_scale = -1.0;
        k2pdfopt_settings_fit_column_to_screen(k2settings,region_width_inches);
        // trim_left_and_right = 0;
        allow_text_wrapping = 0;
        }
    else
        revert=0;
#if (WILLUSDEBUGX & 1)
k2printf("Entering vert region loop, %d regions.\n",breakinfo->n);
k2printf("    region 1:  r1=%d, r2=%d\n",breakinfo->textrow[0].r1,breakinfo->textrow[0].r2);
k2printf("    region %d:  r1=%d, r2=%d\n",breakinfo->n,breakinfo->textrow[breakinfo->n-1].r1,breakinfo->textrow[breakinfo->n-1].r2);
#endif
    /* Un-trim top and bottom region if necessary */
    if (!k2settings->src_trim && breakinfo->n>0)
        {
        breakinfo->textrow[0].r1=region->r1;
        breakinfo->textrow[breakinfo->n-1].r2=region->r2;
        }

    /* Add the regions (broken vertically) */
    caller_id=1;
    trim_flags=k2settings->src_trim ? 0xf : 0x80;
    for (regcount=i1=i=0;i1<breakinfo->n;i++)
        {
        int i2;
 
        i2 = i<breakinfo->n ? i : breakinfo->n-1;
        if (i>=breakinfo->n || (biggap>0. && breakinfo->textrow[i2].gap>=biggap))
            {
            int j,c1,c2,nc,nowrap;
            double regwidth,ar1,rh1;

// k2printf("CALLER 1:  i1=%d, i2=%d (breakinfo->n=%d)\n",i1,i2,breakinfo->n);
            (*bregion)=(*region);
            bregion->r1=breakinfo->textrow[i1].r1;
            bregion->r2=breakinfo->textrow[i2].r2;
            c1=breakinfo->textrow[i1].c1;
            c2=breakinfo->textrow[i1].c2;
            nc=c2-c1+1;
            if (nc<=0)
                nc=1;
            rh1=(double)(breakinfo->textrow[i1].r2-breakinfo->textrow[i1].r1+1)/region->dpi;
            ar1=(double)(breakinfo->textrow[i1].r2-breakinfo->textrow[i1].r1+1)/nc;
            for (j=i1+1;j<=i2;j++)
                {
                if (c1>breakinfo->textrow[j].c1)
                    c1=breakinfo->textrow[j].c1;
                if (c2<breakinfo->textrow[j].c2)
                    c2=breakinfo->textrow[j].c2;
                }
            regwidth=(double)(c2-c1+1)/region->dpi;
            marking_flags=(i1==0?0:1)|(i2==breakinfo->n-1?0:2);
            /* Green */
            mark_source_page(k2settings,bregion,3,marking_flags);
            /*
            ** Is this section not going to be re-flowed?
            */
            nowrap = (allow_text_wrapping==0
                      || (regwidth <= k2settings->max_region_width_inches && allow_text_wrapping<2)
                      || (ar1 > k2settings->no_wrap_ar_limit 
                           && rh1 > k2settings->no_wrap_height_limit_inches));
            /*
            ** If 1. between regions, or 2. if the next region isn't going to be
            ** wrapped (but normally we are wrapping), or 3. if the next region
            ** starts a different number of columns than before, then "flush and gap."
            */
#if (WILLUSDEBUGX & 512)
printf("\nregcount=%d\n",regcount);
printf("reg=%d x %d\n",region->c2-region->c1+1,region->r2-region->r1+1);
printf("k2s->text_wrap=%d\n",k2settings->text_wrap);
printf("nowrap=%d\n",nowrap);
printf("ncols_last=%d\n",ncols_last);
printf("ncols=%d\n",ncols);
printf("diffwidths=%d\n",different_widths(last_region_width_inches,region_width_inches));
printf("diffheights=%d\n",different_row_heights(last_region_last_row_height_inches,
                                                    breakinfo,region));
#endif
            if (regcount>0 || (k2settings->text_wrap && wrapbmp->just_flushed_internal) 
                           || (nowrap && k2settings->text_wrap)
                           || (ncols_last>0 && ncols_last != ncols)
                           /* New in v1.65 */
                           || different_widths(last_region_width_inches,region_width_inches)
                           || different_row_heights(last_region_last_row_height_inches,
                                                    breakinfo,region))
                {
                int gap;
#ifdef WILLUSDEBUG
k2printf("wrapflush1\n");
#endif
                if (k2settings->text_wrap && !wrapbmp->just_flushed_internal)
                    wrapbmp_flush(masterinfo,k2settings,0,0);
                gap = regcount==0 ? colgap_pixels : breakinfo->textrow[i1-1].gap;
                if (regcount==0 && k2settings->text_wrap && wrapbmp->beginning_gap_internal>0)
                    {
                    if (wrapbmp->last_h5050_internal > 0)
                        {
                        if (fabs(1.-(double)breakinfo->textrow[i1].h5050/wrapbmp->last_h5050_internal)>.1)
                            masterinfo_add_gap_src_pixels(masterinfo,k2settings,colgap_pixels,"Col/Page break");
                        wrapbmp->last_h5050_internal=-1;
                        }
                    gap=wrapbmp->beginning_gap_internal;
                    wrapbmp->beginning_gap_internal = -1;
                    }
                masterinfo_add_gap_src_pixels(masterinfo,k2settings,gap,"Vert break");
                }
            else
                {
                if (regcount==0 && wrapbmp->beginning_gap_internal < 0)
                    wrapbmp->beginning_gap_internal = colgap_pixels;
                }
            bmpregion_add(bregion,k2settings,breakinfo,masterinfo,allow_text_wrapping,trim_flags,
                          allow_vertical_breaks,force_scale,justification_flags,caller_id,
                          colcount,rowcount,marking_flags,rbdelta);
            regcount++;
            i1=i2+1;
            }
        }
    if (breakinfo->n>0)
        last_region_last_row_height_inches = (double)breakinfo->textrow[breakinfo->n-1].rowheight/region->dpi;
    else
        last_region_last_row_height_inches = (double)(region->r2-region->r1+1)/region->dpi;
    last_region_width_inches=region_width_inches;
    ncols_last=ncols;
    if (revert)
        k2pdfopt_settings_restore_output_dpi(k2settings);
    breakinfo_free(102,breakinfo);
    }


static int different_widths(double width1,double width2)

    {
    if (width1 < 0. || width2 < 0.)
        return(0);
    if (width2 < width1)
        double_swap(width1,width2);
    if (width1 < 1.)
        return(width2 - width1 > 0.5);
    return((width2/width1-1.) > 0.25);
    }


static int different_row_heights(double height1,BREAKINFO *breakinfo,BMPREGION *region)

    {
    double height2;

    if (height1<0.)
        return(0);
    if (breakinfo->n>0)
        height2= (double)breakinfo->textrow[0].rowheight/region->dpi;
    else
        height2 = (double)(region->r2-region->r1+1)/region->dpi;
    if (height2 < height1)
        double_swap(height1,height2);
    if (height1 < .02)
        return(height2-height1 > .05);
    return((height2/height1-1.) > 0.25);
    }


/*
** A region that needs its line spacing and justification analyzed.
**
** The region may be wider than the max desirable region width.
**
** Input:  breakinfo should be valid row-break information for the region.
**
** Calls bmpregion_one_row_wrap_and_add() for each text row from the
** breakinfo structure that is within the region.
**
*/
void bmpregion_analyze_justification_and_line_spacing(BMPREGION *region,
                            K2PDFOPT_SETTINGS *k2settings,BREAKINFO *breakinfo,
                            MASTERINFO *masterinfo,
                            int *colcount,int *rowcount,
                            int allow_text_wrapping,double force_scale)

    {
    int i,i1,i2,ntr,mean_row_gap,maxgap,line_spacing,nls,nch;
    BMPREGION *newregion,_newregion;
    double *id,*c1,*c2,*ch,*lch,*ls;
    int *just,*indented,*short_line;
    double capheight,lcheight,fontsize;
    int textheight,ragged_right,src_line_spacing,mingap;
    static char *funcname="bmpregion_analyze_justification_and_line_spacing";
    WRAPBMP *wrapbmp;

    wrapbmp=&masterinfo->wrapbmp;
#if (WILLUSDEBUGX & 1)
k2printf("@bmpregion_analyze_justification_and_line_spacing");
k2printf("    (%d,%d) - (%d,%d)\n",region->c1,region->r1,region->c2,region->r2);
k2printf("    centering = %d\n",breakinfo->centered);
#endif
#if (WILLUSDEBUGX & 2)
breakinfo_echo(breakinfo);
#endif

    /* Locate the vertical part indices in the breakinfo structure */
    newregion=&_newregion;
    breakinfo_sort_by_row_position(breakinfo);
    for (i=0;i<breakinfo->n;i++)
        {
        TEXTROW *textrow;
        textrow=&breakinfo->textrow[i];
        if ((textrow->r1+textrow->r2)/2 >= region->r1)
            break;
        }
    if (i>=breakinfo->n)
        return;
    i1=i;
    for (;i<breakinfo->n;i++)
        {
        TEXTROW *textrow;
        textrow=&breakinfo->textrow[i];
        if ((textrow->r1+textrow->r2)/2 > region->r2)
            break;
        }
    i2=i-1;
    if (i2<i1)
        return;
    ntr=i2-i1+1;
#if (WILLUSDEBUGX & 1)
k2printf("    i1=%d, i2=%d, ntr=%d\n",i1,i2,ntr);
#endif

    willus_dmem_alloc_warn(13,(void **)&c1,sizeof(double)*6*ntr,funcname,10);
    willus_dmem_alloc_warn(14,(void **)&just,sizeof(int)*3*ntr,funcname,10);
    c2=&c1[ntr];
    ch=&c2[ntr];
    lch=&ch[ntr];
    ls=&lch[ntr];
    id=&ls[ntr];
    indented=&just[ntr];
    short_line=&indented[ntr];
    for (i=0;i<ntr;i++)
        id[i]=i;

    /* Find baselines / font size */
    capheight=lcheight=0.;
    maxgap=-1;
    for (nch=nls=0,i=i1;i<=i2;i++)
        {
        TEXTROW *textrow;
        double ar,rh;
        int marking_flags;

        textrow=&breakinfo->textrow[i];
        c1[i-i1]=(double)textrow->c1;
        c2[i-i1]=(double)textrow->c2;
        if (i<i2 && maxgap < textrow->gap)
            {
            maxgap = textrow->gap;
            if (maxgap < 2)
                maxgap=2;
            }
        if (textrow->c2<textrow->c1)
            ar = 100.;
        else
            ar = (double)(textrow->r2-textrow->r1+1)/(double)(textrow->c2-textrow->c1+1);
        rh = (double)(textrow->r2-textrow->r1+1)/region->dpi;
        if (i<i2 && ar <= k2settings->no_wrap_ar_limit && rh <= k2settings->no_wrap_height_limit_inches)
            ls[nls++]=breakinfo->textrow[i+1].r1-textrow->r1;
        if (ar <= k2settings->no_wrap_ar_limit && rh <= k2settings->no_wrap_height_limit_inches)
            {
            ch[nch] = textrow->capheight;
            lch[nch] = textrow->lcheight;
            nch++;
            }

        /* Mark region w/gray, mark rowbase also */
        marking_flags=(i==i1?0:1)|(i==i2?0:2);
        if (i<i2 || textrow->r2-textrow->rowbase>1)
            marking_flags |= 0x10;
        (*newregion)=(*region);
        newregion->r1=textrow->r1;
        newregion->r2=textrow->r2;
        newregion->c1=textrow->c1;
        newregion->c2=textrow->c2;
        newregion->rowbase=textrow->rowbase;
        if (textrow->rat>0.)
            {
            int irat;
            if (textrow->rat > 99.)
                irat=99;
            else
                irat=textrow->rat+.5;
            mark_source_page(k2settings,newregion,100+irat,marking_flags);
            }
        else
            mark_source_page(k2settings,newregion,5,marking_flags);
#if (WILLUSDEBUGX & 1)
k2printf("   Row %2d: (%4d,%4d) - (%4d,%4d) rowbase=%4d, lch=%d, h5050=%d, rh=%d\n",i-i1+1,textrow->c1,textrow->r1,textrow->c2,textrow->r2,textrow->rowbase,textrow->lcheight,textrow->h5050,textrow->rowheight);
#endif
        }
    wrapbmp_set_maxgap(wrapbmp,maxgap);
    if (nch<1)
        capheight = lcheight = 2; // Err on the side of too small
    else
        {
        capheight = median_val(ch,nch);
        lcheight = median_val(lch,nch);
        }
// k2printf("capheight = %g, lcheight = %g\n",capheight,lcheight);
    bmpregion_is_centered(region,k2settings,breakinfo,i1,i2,&textheight);
    /*
    ** For 12 pt font:
    **     Single spacing is 14.66 pts (Calibri), 13.82 pts (Times), 13.81 pts (Arial)
    **     Size of cap letter is 7.7 pts (Calibri), 8.1 pts (Times), 8.7 pts (Arial)
    **     Size of small letter is 5.7 pts (Calibri), 5.6 pts (Times), 6.5 pts (Arial)
    ** Mean line spacing = 1.15 - 1.22 (~1.16)
    ** Mean cap height = 0.68
    ** Mean small letter height = 0.49
    */
    fontsize = (capheight+lcheight)/1.17;
// k2printf("font size = %g pts.\n",(fontsize/region->dpi)*72.);
    /*
    ** Set line spacing for this region
    */
    if (nls>0)
        src_line_spacing = median_val(ls,nls);
    else
        src_line_spacing = fontsize*1.2;
    if (k2settings->vertical_line_spacing < 0 
              && src_line_spacing <= fabs(k2settings->vertical_line_spacing)*fontsize*1.16)
        line_spacing = src_line_spacing;
    else
        line_spacing = fabs(k2settings->vertical_line_spacing)*fontsize*1.16;
#if (WILLUSDEBUGX & 1)
k2printf("   font size = %.2f pts = %d pixels\n",(fontsize/region->dpi)*72.,(int)(fontsize+.5));
k2printf("   src_line_spacing = %d, line_spacing = %d\n",src_line_spacing,line_spacing);
#endif
    /*
    if (ntr==1)
        rheight=  (int)((breakinfo->textrow[i1].r2 - breakinfo->textrow[i1].r1)*1.25+.5);
    else
        rheight = (int)((double)(breakinfo->textrow[i2].rowbase - breakinfo->textrow[i1].rowbase)/(ntr-1)+.5);
    */
    mean_row_gap = line_spacing - textheight;
    if (mean_row_gap <= 1)
        mean_row_gap = 1;
    mingap = mean_row_gap / 4;
    if (mingap < 1)
        mingap = 1;

    /* Try to figure out if we have a ragged right edge */
    if (ntr<3)
        ragged_right=1;
    else
        {
        int flushcount;

        if (k2settings->src_left_to_right)
            {
            for (flushcount=i=0;i<ntr;i++)
                {
#if (WILLUSDEBUGX & 1)
k2printf("    flush_factors[%d] = %g (<.5), %g in (<.1)\n",
i,(double)(region->c2-c2[i])/textheight,(double)(region->c2-c2[i])/region->dpi);
#endif
                if ((double)(region->c2-c2[i])/textheight < 0.5
                      && (double)(region->c2-c2[i])/region->dpi < 0.1)
                    flushcount++;
                }
            }
        else
            {
            for (flushcount=i=0;i<ntr;i++)
                {
#if (WILLUSDEBUGX & 1)
k2printf("    flush_factors[%d] = %g (<.5), %g in (<.1)\n",
i,(double)(c1[i]-region->c1)/textheight,(double)(c1[i]-region->c1)/region->dpi);
#endif
                if ((double)(c1[i]-region->c1)/textheight < 0.5
                      && (double)(c1[i]-region->c1)/region->dpi < 0.1)
                    flushcount++;
                }
            }
        ragged_right = (flushcount <= ntr/2);
        /*
        if (k2settings->src_left_to_right)
            {
            sortxyd(c2,id,ntr);
            del = region->c2 - c2[ntr-1-ntr/3];
            sortxyd(id,c2,ntr);
            }
        else
            {
            sortxyd(c1,id,ntr);
            del = c1[ntr/3] - region->c1;
            sortxyd(id,c1,ntr);
            }
        del /= textheight;
k2printf("del=%g\n",del);
        ragged_right = (del > 0.5);
        */
        }
#if (WILLUSDEBUGX & 1)
k2printf("ragged_right=%d\n",ragged_right);
#endif

    /* Store justification and other info line by line */
    for (i=i1;i<=i2;i++)
        {
        double indent1,del;
        double i1f,ilfi,i2f,ilf,ifmin,dif;
        int centered;

        TEXTROW *textrow;
        textrow=&breakinfo->textrow[i];
        i1f = (double)(c1[i-i1]-region->c1)/(region->c2-region->c1+1);
        i2f = (double)(region->c2-c2[i-i1])/(region->c2-region->c1+1);
        ilf = k2settings->src_left_to_right ? i1f : i2f;
        ilfi = ilf*(region->c2-region->c1+1)/region->dpi; /* Indent in inches */
        ifmin = i1f<i2f ? i1f : i2f;
        dif=fabs(i1f-i2f);
        if (ifmin < .01)
            ifmin=0.01;
        if (k2settings->src_left_to_right)
            indent1 = (double)(c1[i-i1]-region->c1) / textheight;
        else
            indent1 = (double)(region->c2 - c2[i-i1]) / textheight;
// k2printf("    row %2d:  indent1=%g\n",i-i1,indent1);
        if (!breakinfo->centered)
            {
            indented[i-i1]=(indent1 > 0.5 && ilfi < 1.2 && ilf < .25);
            centered= (!indented[i-i1] && indent1 > 1.0 && dif/ifmin<0.5);
            }
        else
            {
            centered= (dif<0.1 || dif/ifmin<0.5);
            indented[i-i1]=(indent1 > 0.5 && ilfi < 1.2 && ilf < .25 && !centered);
            }
#if (WILLUSDEBUGX & 1)
k2printf("Indent %d:  %d.  indent1=%g, ilf=%g, centered=%d\n",i-i1+1,indented[i-i1],indent1,ilf,centered);
k2printf("    indent1=%g, i1f=%g, i2f=%g\n",indent1,i1f,i2f);
#endif
        if (centered)
            just[i-i1] = 4;
        else
            {
            /*
            ** The .01 favors left justification over right justification in
            ** close cases.
            */
            if (k2settings->src_left_to_right)
                just[i-i1] = indented[i-i1] || (i1f < i2f+.01) ? 0 : 8;
            else
                just[i-i1] = indented[i-i1] || (i2f < i1f+.01) ? 8 : 0;
            }
        if (k2settings->src_left_to_right)
            del = (double)(region->c2 - textrow->c2);
        else
            del = (double)(textrow->c1 - region->c1);
        /* Should we keep wrapping after this line? */
        if (!ragged_right)
            short_line[i-i1] = (del/textheight > 0.5);
        else
            short_line[i-i1] = (del/(region->c2-region->c1) > 0.25);
        /* If this row is a bigger/smaller row (font) than the next row, don't wrap. */
        if (!short_line[i-i1] && i<i2)
            {
            TEXTROW *t1;
            t1=&breakinfo->textrow[i+1];
            if ((textrow->h5050 > t1->h5050*1.5 || textrow->h5050*1.5 < t1->h5050)
                  && (i==0 || (i>0 && (textrow->rowheight > t1->rowheight*1.5
                                        || textrow->rowheight*1.5 < t1->rowheight))))
                short_line[i-i1] = 1;
            }
        if (!ragged_right)
            just[i-i1] |= 0x40;
#if (WILLUSDEBUGX & 1)
k2printf("        just[%d]=0x%02X, shortline[%d]=%d\n",i-i1,just[i-i1],i-i1,short_line[i-i1]);
k2printf("        textrow->c2=%d, region->c2=%d, del=%g, textheight=%d\n",textrow->c2,region->c2,del,textheight);
#endif
        /* If short line, it should still be fully justified if it is wrapped. */
        /*
        if (short_line[i-i1])
            just[i-i1] = (just[i-i1]&0xf)|0x60;
        */
        }
/*        
{
double mean1,mean2,stdev1,stdev2;
array_mean(c1,ntr,&mean1,&stdev1);
array_mean(c2,ntr,&mean2,&stdev2);
k2printf("Mean c1, c2 = %g, %g; stddevs = %g, %g\n",mean1,mean2,stdev1,stdev2);
k2printf("textheight = %d, line_spacing = %d\n",textheight,line_spacing);
}
*/

#if (WILLUSDEBUGX & 1)
if (!allow_text_wrapping)
k2printf("Processing text row by row (no wrapping)...\n");
#endif
    /*
    ** Process row by row
    */
    for (i=i1;i<=i2;i++)
        {
        TEXTROW *textrow;
        int justflags,trimflags,centered,marking_flags,gap;

#if (WILLUSDEBUGX & 1)
k2printf("Row " ANSI_YELLOW "%d of %d" ANSI_NORMAL " (wrap=%d)\n",i-i1+1,i2-i1+1,allow_text_wrapping);
#endif
        textrow=&breakinfo->textrow[i];
        (*newregion)=(*region);
        newregion->r1=textrow->r1;
        newregion->r2=textrow->r2;
#if (WILLUSDEBUGX & 1)
k2printf("Row %2d:  r1=%4d, r2=%4d, linespacing=%3d\n",i,textrow->r1,textrow->r2,line_spacing);
#endif

        /* The |3 tells it to use the user settings for left/right/center */
        justflags = just[i-i1]|0x3;
        centered=((justflags&0xc)==4);
#if (WILLUSDEBUGX & 1)
k2printf("    justflags[%d]=0x%2X, centered=%d, indented=%d\n",i-i1,justflags,centered,indented[i-i1]);
#endif
        if (allow_text_wrapping)
            {
            /* If this line is indented or if the justification has changed, */
            /* then start a new line.                                        */
            if (centered || indented[i-i1] || (i>i1 && (just[i-i1]&0xc)!=(just[i-i1-1]&0xc)))
{
#ifdef WILLUSDEBUG
k2printf("wrapflush4\n");
#endif
                wrapbmp_flush(masterinfo,k2settings,0,1);
}
#ifdef WILLUSDEBUG
k2printf("    c1=%d, c2=%d\n",newregion->c1,newregion->c2);
#endif
            marking_flags=0xc|(i==i1?0:1)|(i==i2?0:2);
            bmpregion_one_row_wrap_and_add(newregion,k2settings,breakinfo,i,i1,i2,
                                       masterinfo,justflags,colcount,rowcount,
                                       line_spacing,mean_row_gap,textrow->rowbase,
                                       marking_flags,indented[i-i1]);
            if (centered || short_line[i-i1])
{
#ifdef WILLUSDEBUG
k2printf("wrapflush5\n");
#endif
                wrapbmp_flush(masterinfo,k2settings,0,2);
}
            continue;
            }
#ifdef WILLUSDEBUG
k2printf("wrapflush5a\n");
#endif

        /* No wrapping allowed:  process whole line as one region */
        wrapbmp_flush(masterinfo,k2settings,0,1);
        /* If default justifications, ignore all analysis and just center it. */
        if (k2settings->dst_justify<0 && k2settings->dst_fulljustify<0)
            {
            newregion->c1 = region->c1;
            newregion->c2 = region->c2;
            justflags=0xad; /* Force centered region, no justification */
            trimflags=0x80;
            }
        else
            trimflags=0;
        /* No wrapping:  text wrap, trim flags, vert breaks, fscale, just */
        bmpregion_add(newregion,k2settings,breakinfo,masterinfo,0,trimflags,0,force_scale,
                      justflags,5,colcount,rowcount,0,textrow->r2-textrow->rowbase);
        /* Compute line spacing between rows */
        {
        int thisgap,gap_allowed;
        double fs,ls_allowed;

        thisgap = (i<i2) ? textrow->gap 
                         : textrow->rowheight-(textrow->rowbase + k2settings->last_rowbase_internal);
#if (WILLUSDEBUGX & 1)
k2printf("    thisgap=%3d, vls = %g\n",thisgap,k2settings->vertical_line_spacing);
#endif
        fs = (textrow->capheight+textrow->lcheight)/1.17;
        if (fs < fontsize/4.) /* Probably not text?? */
            fs = fontsize;
        ls_allowed=fabs(k2settings->vertical_line_spacing)*fs*1.16;
        /* If close to median line spacing, use median line spacing */
        /* ... Good idea?? */
        if (line_spacing>.5 && fabs(ls_allowed/line_spacing-1.0)<.2)
            ls_allowed=line_spacing;
        gap_allowed=(int)(0.5+ls_allowed-(textrow->r2-textrow->r1+1));
#if (WILLUSDEBUGX & 1)
k2printf("    gap_allowed = %3d\n",gap_allowed);
#endif
        if (k2settings->vertical_line_spacing < 0)
            gap = thisgap > gap_allowed ? gap_allowed : thisgap;
        else
            gap = gap_allowed;
/*
            gap = gap1 < gap_allowed ? gap_allowed : gap1;
            if (i<i2)
                {
                if (textrow->gap > gap1)
                    {
                    int gap_allowed;
                    srcls = (textrow->r2-textrow->r1+1)+textrow->gap;
                    fs = (textrow->capheight+textrow->lcheight)/1.17;
                    ls_allowed=fabs(k2settings->vertical_line_spacing)*fs*1.16;
                    gap_allowed=ls_allowed-(textrow->r2-textrow->r1+1);
                    if (gap_allowed < textrow->gap)
                        gap_allowed = textrow->gap;
                    gap = gap1 > gap_allowed ? gap_allowed : gap1;
                    }
                else
                    gap = textrow->gap;
                }
            else
                {
                gap = textrow->rowheight - (textrow->rowbase + k2settings->last_rowbase_internal);
                if (gap < mean_row_gap/2.)
                    gap = mean_row_gap;
                }

            }
        else
            {
            gap = line_spacing - (textrow->r2-textrow->r1+1);
            if (gap < mean_row_gap/2.)
                gap = mean_row_gap;
            }
*/
        if (gap < mingap)
            gap = mingap;
#if (WILLUSDEBUGX & 1)
k2printf("    gap = %3d (mingap=%d)\n",gap,mingap);
#endif
        if (i<i2)
            masterinfo_add_gap_src_pixels(masterinfo,k2settings,gap,"No-wrap line");
        else
            {
            wrapbmp->last_h5050_internal = textrow->h5050;
            wrapbmp->beginning_gap_internal = gap;
            }
        }
        }
    willus_dmem_free(14,(double **)&just,funcname);
    willus_dmem_free(13,(double **)&c1,funcname);
#ifdef WILLUSDEBUG
k2printf("Done wrap_and_add.\n");
#endif
    }


/*
** pi = preserve indentation
*/
void bmpregion_one_row_wrap_and_add(BMPREGION *region,K2PDFOPT_SETTINGS *k2settings,
                                    BREAKINFO *rowbreakinfo,
                                    int index,int i1,int i2,
                                    MASTERINFO *masterinfo,int justflags,
                                    int *colcount,int *rowcount,
                                    int line_spacing,int mean_row_gap,int rowbase,
                                    int marking_flags,int pi)

    {
    int nc,nr,i,i0,gappix;
    double aspect_ratio,region_height;
    BREAKINFO *colbreaks,_colbreaks;
    BMPREGION *newregion,_newregion;
    WRAPBMP *wrapbmp;

    wrapbmp=&masterinfo->wrapbmp;
#if (WILLUSDEBUGX & 4)
k2printf("@bmpregion_one_row_wrap_and_add, index=%d, i1=%d, i2=%d\n",index,i1,i2);
#endif
    newregion=&_newregion;
    (*newregion)=(*region);
    bmpregion_trim_margins(newregion,k2settings,colcount,rowcount,0xf);
    nc=newregion->c2-newregion->c1+1;
    nr=newregion->r2-newregion->r1+1;
    if (nc<6)
        return;
    aspect_ratio = (double)nr/nc;
    region_height = (double)nr/region->dpi;
    if (aspect_ratio > k2settings->no_wrap_ar_limit && region_height > k2settings->no_wrap_height_limit_inches)
        {
        newregion->r1=region->r1;
        newregion->r2=region->r2;
#ifdef WILLUSDEBUG
k2printf("wrapflush6\n");
#endif
        wrapbmp_flush(masterinfo,k2settings,0,1);
        if (index>i1)
            masterinfo_add_gap_src_pixels(masterinfo,k2settings,
                                   rowbreakinfo->textrow[index-1].gap,"Tall region");
        bmpregion_add(newregion,k2settings,rowbreakinfo,masterinfo,0,0xf,0,-1.0,0,2,
                      colcount,rowcount,0xf,
                      rowbreakinfo->textrow[index].r2-rowbreakinfo->textrow[index].rowbase);
        if (index<i2)
            k2settings->gap_override_internal=rowbreakinfo->textrow[index].gap;
        return;
        }
    colbreaks=&_colbreaks;
    colbreaks->textrow=NULL;
    breakinfo_alloc(106,colbreaks,newregion->c2-newregion->c1+1);
    bmpregion_one_row_find_breaks(newregion,colbreaks,k2settings,colcount,rowcount,1);
    if (pi && colbreaks->n>0)
        {
        if (k2settings->src_left_to_right)
            colbreaks->textrow[0].c1=region->c1;
        else
            colbreaks->textrow[colbreaks->n-1].c2=region->c2;
        }
    /*
    hs=0.;
    for (i=0;i<colbreaks->n;i++)
        hs += (colbreaks->textrow[i].r2-colbreaks->textrow[i].r1);
    hs /= colbreaks->n;
    */
    /*
    ** Find appropriate letter height to use for word spacing
    */
    {
    double median_gap;
    word_gaps_add(NULL,newregion->lcheight,&median_gap,k2settings->word_spacing);
    gappix = (int)(median_gap*newregion->lcheight+.5);
    }
#if (WILLUSDEBUGX & 4)
k2printf("Before small gap removal, column breaks:\n");
breakinfo_echo(colbreaks);
#endif
#if (WILLUSDEBUGX & 4)
k2printf("After small gap removal, column breaks:\n");
breakinfo_echo(colbreaks);
#endif
    if (k2settings->show_marked_source)
        for (i=0;i<colbreaks->n;i++)
            {
            BMPREGION xregion;
            xregion=(*newregion);
            xregion.c1=colbreaks->textrow[i].c1;
            xregion.c2=colbreaks->textrow[i].c2;
            mark_source_page(k2settings,&xregion,2,marking_flags);
            }
#if (WILLUSDEBUGX & 4)
for (i=0;i<colbreaks->n;i++)
k2printf("    colbreak[%d] = %d - %d\n",i,colbreaks->textrow[i].c1,colbreaks->textrow[i].c2);
#endif
    /* Maybe skip gaps < 0.5*median_gap or collect gap/rowheight ratios and skip small gaps */
    /* (Could be thrown off by full-justified articles where some lines have big gaps.)     */
    /* Need do call a separate function that removes these gaps. */
    for (i0=0;i0<colbreaks->n;)
        {
        int i1,i2,toolong,rw,remaining_width_pixels;
        BMPREGION reg;

        toolong=0; /* Avoid compiler warning */
        for (i=i0;i<colbreaks->n;i++)
            {
            int wordgap;

            wordgap = wrapbmp_ends_in_hyphen(wrapbmp) ? 0 : gappix;
            i1=k2settings->src_left_to_right ? i0 : colbreaks->n-1-i;
            i2=k2settings->src_left_to_right ? i : colbreaks->n-1-i0;
            rw=(colbreaks->textrow[i2].c2-colbreaks->textrow[i1].c1+1);
            remaining_width_pixels = wrapbmp_remaining(wrapbmp,k2settings);
            toolong = (rw+wordgap > remaining_width_pixels);
#if (WILLUSDEBUGX & 4)
k2printf("    i1=%d, i2=%d, rw=%d, rw+gap=%d, remainder=%d, toolong=%d\n",i1,i2,rw,rw+wordgap,remaining_width_pixels,toolong);
#endif
            /*
            ** If we're too long with just one word and there is already
            ** stuff on the queue, then flush it and re-evaluate.
            */
            if (i==i0 && toolong && wrapbmp_width(wrapbmp)>0)
                {
#ifdef WILLUSDEBUG
k2printf("wrapflush8\n");
#endif
                wrapbmp_flush(masterinfo,k2settings,1,0);
                i--;
                continue;
                }
            /*
            ** If we're not too long and we're not done yet, add another word.
            */
            if (i < colbreaks->n-1 && !toolong)
                continue;
            /*
            ** Add the regions from i0 to i (or i0 to i-1)
            */
            break;
            }
        if (i>i0 && toolong)
            i--;
        i1=k2settings->src_left_to_right ? i0 : colbreaks->n-1-i;
        i2=k2settings->src_left_to_right ? i : colbreaks->n-1-i0;
        reg=(*newregion);
        reg.c1=colbreaks->textrow[i1].c1;
        reg.c2=colbreaks->textrow[i2].c2;
#if (WILLUSDEBUGX & 4)
k2printf("    Adding i1=%d to i2=%d\n",i1,i2);
#endif
        /* Trim the word top/bottom */
        bmpregion_trim_margins(&reg,k2settings,colcount,rowcount,0xc);
        reg.c1=colbreaks->textrow[i1].c1;
        reg.c2=colbreaks->textrow[i2].c2;
        reg.lcheight=newregion->lcheight;
        reg.capheight=newregion->capheight;
        reg.rowbase=newregion->rowbase;
        reg.h5050=newregion->h5050;
        if (reg.r1 > reg.rowbase)
            reg.r1 = reg.rowbase;
        if (reg.r2 < reg.rowbase)
            reg.r2=reg.rowbase;
        /* Add it to the existing line queue */
        wrapbmp_add(wrapbmp,&reg,k2settings,gappix,line_spacing,rowbase,mean_row_gap,justflags);
        if (toolong)
{
#ifdef WILLUSDEBUG
k2printf("wrapflush7\n");
#endif
            wrapbmp_flush(masterinfo,k2settings,1,0);
}
        i0=i+1;
        }
    breakinfo_free(106,colbreaks);
    }


/*
** CAUTION:  This function re-orders the x[] array!
*/
static double median_val(double *x,int n)

    {
    int i1,n1;

    if (n<4)
        return(array_mean(x,n,NULL,NULL));
    sortd(x,n);
    if (n==4)
        {
        n1=2;
        i1=1;
        }
    else if (n==5)
        {
        n1=3;
        i1=1;
        }
    else
        {
        n1=n/3;
        i1=(n-n1)/2;
        }
    return(array_mean(&x[i1],n1,NULL,NULL));
    }
