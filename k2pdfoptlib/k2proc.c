/*
** k2proc.c    These functions do the "heavy lifting" in k2pdfopt.  They
**             examine the source bitmap for contiguous regions, locating
**             columns, rows of text, and individual words, and laying out the
**             output pages.
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

static void bmpregion_source_box_process(BMPREGION *region,K2PDFOPT_SETTINGS *k2settings,
                                         MASTERINFO *masterinfo,int level,int pages_done);
static void pageregions_grid(PAGEREGIONS *pageregions,BMPREGION *region,
                             K2PDFOPT_SETTINGS *k2settings,int level);
static void pageregions_from_cropboxes(PAGEREGIONS *pageregions,BMPREGION *region,
                                       K2PDFOPT_SETTINGS *k2settings,MASTERINFO *masterinfo);
static int k2cropboxes_skip_box(K2CROPBOXES *cropboxes,int index,int srcpage,int npages,int iboxes);
static void bmpregion_whiteout_cropboxes(BMPREGION *srcregion,K2PDFOPT_SETTINGS *k2settings,
                                         MASTERINFO *masterinfo);
static void bmpregion_set_cropbox_pixels(BMPREGION *dstregion,K2CROPBOX *cropbox,
                                         BMPREGION *srcregion,MASTERINFO *masterinfo);
static void pageregions_find_next_level(PAGEREGIONS *pageregions_sorted,BMPREGION *srcregion,
                                        K2PDFOPT_SETTINGS *k2settings,int level,K2NOTES *notes);
static double median_val(double *x,int n);
#ifdef HAVE_MUPDF_LIB
static int add_crop_boxes(BMPREGION *region,K2PDFOPT_SETTINGS *k2settings,MASTERINFO *masterinfo,
                          int bmp_rotation_deg);
static int add_crop_box(BMPREGION *region,K2PDFOPT_SETTINGS *k2settings,
                        MASTERINFO *masterinfo,int bmp_rotation_deg);
#endif
static int bmpregion_find_multicolumn_divider(BMPREGION *region,K2PDFOPT_SETTINGS *k2settings,
                                              int *row_black_count,PAGEREGIONS *pageregions,
                                              K2NOTES *notes);
static void bmpregion_vertically_break(BMPREGION *region,K2PDFOPT_SETTINGS *k2settings,
                          MASTERINFO *masterinfo,double force_scale,int source_page,int ncols,
                          BMPREGION *notes);
static void textrows_group(TEXTROWS *textrows,int biggap_pixels,int *firstrow,int *lastrow,
                           int *ngroups,int text_only);
static void bmpregion_add_textrows(ADDED_REGION_INFO *params,K2PDFOPT_SETTINGS *k2settings,
                                   MASTERINFO *masterinfo);
static int different_widths(double width1,double width2);
/*
static int different_row_heights(double height1,BMPREGION *region);
*/
static void bmpregion_analyze_justification_and_line_spacing(ADDED_REGION_INFO *added_region,
                                       K2PDFOPT_SETTINGS *k2settings,MASTERINFO *masterinfo);
static void multiline_params_calculate(MULTILINE_PARAMS *mlp,BMPREGION *region,
                                       MASTERINFO *masterinfo,K2PDFOPT_SETTINGS *k2settings,
                                       int allow_text_wrapping,int region_is_centered);
static double calc_median_indent(double *indent,int n,double median_h5050);
static void multiline_params_init(MULTILINE_PARAMS *mlp);
static void multiline_params_alloc(MULTILINE_PARAMS *mlp);
static void multiline_params_free(MULTILINE_PARAMS *mlp);
static int get_line_spacing_pixels(TEXTROW *tr1,TEXTROW *tr2,MULTILINE_PARAMS *mlp,
                                   K2PDFOPT_SETTINGS *k2settings,int allow_text_wrapping);
static void textrow_mark_source(TEXTROW *textrow,BMPREGION *region,MASTERINFO *masterinfo,
                                K2PDFOPT_SETTINGS *k2settings,int marking_flags);
static void bmpregion_one_row_wrap_and_add(BMPREGION *region,K2PDFOPT_SETTINGS *k2settings,
                                    MASTERINFO *masterinfo,int justflags,int line_spacing,
                                    int mean_row_gap,int marking_flags,int pi);


/*
** Call once per document
*/
void k2proc_init_one_document(void)

    {
    /* Init vert break routine */
    bmpregion_vertically_break(NULL,NULL,NULL,0.,0,0,NULL);
    }


/*
** Determine median font size (based on row heights) in a region.
** Graphically determine median font size in a rectangular region.
** Break into columns if necessary.
*/
void k2proc_get_fontsize_histogram(BMPREGION *srcregion,MASTERINFO *masterinfo,
                                   K2PDFOPT_SETTINGS *k2settings,FONTSIZE_HISTOGRAM *fsh)

    {
    PAGEREGIONS *pageregions,_pageregions;
    BMPREGION *region,_region;
    int i,maxlevels;

#if (WILLUSDEBUGX & 0x10000)
bmpregion_write(srcregion,"fontsize_bmpregion_major.png");
#endif
/*
printf("@k2proc_determine_median_font_size\n");
*/

    /* Parse region into columns */
    region=&_region;
    bmpregion_init(region);
    bmpregion_copy(region,srcregion,0);
    pageregions=&_pageregions;
    pageregions_init(pageregions);
    if (k2settings->max_columns>2)
        maxlevels = 3;
    else if (k2settings->max_columns>1)
        maxlevels = 2;
    else
        maxlevels = 1;
    pageregions_find_columns(pageregions,region,k2settings,masterinfo,maxlevels);
    for (i=0;i<pageregions->n;i++)
        {
        BMPREGION *region1;
        int j;

        region1=&pageregions->pageregion[i].bmpregion;
        bmpregion_find_textrows(region1,k2settings,0,1,
                                k2settings->join_figure_captions==2 ? 1 : 0);
        for (j=0;j<region1->textrows.n;j++)
            if (region1->textrows.textrow[j].type==REGION_TYPE_TEXTLINE)
                fontsize_histogram_add_fontsize(fsh,
                              2.*72.*region1->textrows.textrow[j].h5050/region1->dpi);
        }
    pageregions_free(pageregions);
    bmpregion_free(region);
    }


/*
** First break the source page into cropboxes and/or gridded rectangles based
** on the -cbox or -grid arguments.  Pass each box into bmpregion_source_box_process().
**
** If there are not crop boxes or gridded areas, then the entire source page is
** passed to bmperegion_source_box_process().
**
** Functionality changed in v2.13.
**
*/
void bmpregion_source_page_add(BMPREGION *region,K2PDFOPT_SETTINGS *k2settings,
                               MASTERINFO *masterinfo,int level,int pages_done)

    {
    PAGEREGIONS *pageregions,_pageregions;
    int i,gridded;

#if (!(WILLUSDEBUGX & 0x200))
    if (k2settings->debug)
#endif
        k2printf("@bmpregion_source_page_add (%d,%d) - (%d,%d) dpi=%d, lev=%d, pagesdone=%d\n",
               region->c1,region->r1,region->c2,region->r2,region->dpi,level,pages_done);

    /* White-out all crop boxes with K2CROPBOX_FLAGS_IGNOREBOXEDAREA set */
    bmpregion_whiteout_cropboxes(region,k2settings,masterinfo);


    gridded = (k2settings->src_grid_cols > 0 && k2settings->src_grid_rows > 0);
    if (!k2settings_has_cropboxes(k2settings) && !gridded)
        {
        bmpregion_source_box_process(region,k2settings,masterinfo,level,pages_done);
        return;
        }

    /* Find page regions */     
    pageregions=&_pageregions;
    pageregions_init(pageregions);

    /* Get regions from cropboxes or grid areas */
    if (k2settings_has_cropboxes(k2settings))
        pageregions_from_cropboxes(pageregions,region,k2settings,masterinfo);
    else
        pageregions_grid(pageregions,region,k2settings,0);

    /* Pass each region along to next function */
    for (i=0;i<pageregions->n;i++)
        bmpregion_source_box_process(&pageregions->pageregion[i].bmpregion,
                                     k2settings,masterinfo,level,pages_done); 
    pageregions_free(pageregions);
    }


void bmpregion_add_cover_image(BMPREGION *coverimage,K2PDFOPT_SETTINGS *k2settings,
                               MASTERINFO *masterinfo)

    {
    ADDED_REGION_INFO _added_region,*added_region;
    BMPREGION *newregion,_newregion;
    int i,dst_ocr,dst_fit_to_page,dst_figure_rotate,fit_columns;

    newregion=&_newregion;
    bmpregion_init(newregion);
    bmpregion_copy(newregion,coverimage,0);
    added_region=&_added_region;
    added_region->region=newregion;
    added_region->firstrow=0;
    added_region->lastrow=0;
    added_region->allow_text_wrapping=0;
    added_region->trim_flags=0;
    added_region->allow_vertical_breaks=0;
    added_region->force_scale=-2;
    added_region->justification_flags=0;
    added_region->caller_id=0;
    added_region->rowbase_delta=0;
    added_region->region_is_centered=1;
    added_region->notes=0;
    added_region->count=0;
    added_region->maps_to_source=0;

    /* White-out all crop boxes with K2CROPBOX_FLAGS_IGNOREBOXEDAREA set */
    bmpregion_whiteout_cropboxes(coverimage,k2settings,masterinfo);

    /* Apply crop box if found */
    for (i=0;i<k2settings->cropboxes.n;i++)
        {
        K2CROPBOX *cropbox;

        if (k2cropboxes_skip_box(&k2settings->cropboxes,i,masterinfo->pageinfo.srcpage,
                                 masterinfo->srcpages,0))
            continue;
        cropbox=&k2settings->cropboxes.cropbox[i];
        bmpregion_set_cropbox_pixels(newregion,cropbox,coverimage,masterinfo);
        break;
        }

    /* Store k2settings parameters */
#ifdef HAVE_OCR_LIB
    dst_ocr=k2settings->dst_ocr;
#endif
    dst_fit_to_page=k2settings->dst_fit_to_page;
    dst_figure_rotate=k2settings->dst_figure_rotate;
    fit_columns=k2settings->fit_columns;

    /* Modify for cover image */
    /* Turn off OCR if type 'm' */
#ifdef HAVE_OCR_LIB
    if (tolower(dst_ocr)=='m')
        k2settings->dst_ocr=0;
#endif
    k2settings->dst_fit_to_page=-2;
    k2settings->dst_figure_rotate=0;
    k2settings->fit_columns=1;
    bmpregion_add(added_region,k2settings,masterinfo);

    /* Restore k2settings */
    k2settings->fit_columns=fit_columns;
    k2settings->dst_figure_rotate=dst_figure_rotate;
    k2settings->dst_fit_to_page=dst_fit_to_page;
#ifdef HAVE_OCR_LIB
    k2settings->dst_ocr=dst_ocr;
#endif
    bmpregion_free(newregion);
    }


/*
** Process a rectangular source page bitmap determined from either a cropbox or
** a grid area (or the entire source page if no cropboxes or gridded areas).
**
** New function in v2.13.
**
*/
static void bmpregion_source_box_process(BMPREGION *region,K2PDFOPT_SETTINGS *k2settings,
                                         MASTERINFO *masterinfo,int level,int pages_done)

    {
    PAGEREGIONS *pageregions,_pageregions;
    int ipr,gridded,maxlevels,trim_regions,trim_mode;

#if (!(WILLUSDEBUGX & 0x200))
    if (k2settings->debug)
#endif
        k2printf("@bmpregion_source_box_process (%d,%d) - (%d,%d) dpi=%d, lev=%d, pagesdone=%d\n",
               region->c1,region->r1,region->c2,region->r2,region->dpi,level,pages_done);


    trim_mode=k2settings_trim_mode(k2settings);

    /* Find page regions */     
    pageregions=&_pageregions;
    pageregions_init(pageregions);

    gridded = (k2settings->src_grid_cols > 0 && k2settings->src_grid_rows > 0);

    /* Search for columns */
    if (k2settings->max_columns<=1)
        maxlevels=1;
    else if (k2settings->max_columns<=2)
        maxlevels=2;
    else
        maxlevels=3;
    pageregions_find_columns(pageregions,region,k2settings,masterinfo,maxlevels);
    trim_regions = ((k2settings->vertical_break_threshold<-1.5
                       || k2settings->dst_fit_to_page==-2
                       || gridded
                       || k2settings_has_cropboxes(k2settings))
                       && (k2settings->dst_userwidth_units==UNITS_TRIMMED
                           || k2settings->dst_userheight_units==UNITS_TRIMMED));


    /* Process page regions */
/*
#if (!(WILLUSDEBUGX & 0x200))
    if (k2settings->debug)
#endif
        {
        int i;
        k2printf("Page regions:  %d\n",pageregions->n);
        for (i=0;i<pageregions->n;i++)
            k2printf("    %d. (%d,%d) - (%d,%d)\n",i+1,pageregions->pageregion[i].bmpregion.c1,
                pageregions->pageregion[i].bmpregion.r1,
                pageregions->pageregion[i].bmpregion.c2,
                pageregions->pageregion[i].bmpregion.r2);
        }
*/
#if (WILLUSDEBUGX & 0x200)
printf("Found %d page regions.\n",pageregions->n);
{
int i;
printf("Before trimming:\n");
for (i=0;i<pageregions->n;i++)
printf("    %d. (%d,%d) - (%d,%d)\n",i+1,
pageregions->pageregion[i].bmpregion.c1,
pageregions->pageregion[i].bmpregion.r1,
pageregions->pageregion[i].bmpregion.c2,
pageregions->pageregion[i].bmpregion.r2);
}
#endif

    /* v2.33:  Do all trimming up front */
    /* Trim regions */
    if (trim_regions && k2settings->src_trim)
        for (ipr=0;ipr<pageregions->n;ipr++)
            {
            /* If blank page and in trim mode with page breaks, don't trim */
            if (trim_mode && bmpregion_is_blank(&pageregions->pageregion[ipr].bmpregion,k2settings))
{
#if (WILLUSDEBUGX & 0x200)
aprintf(ANSI_YELLOW "Skipping trim." ANSI_NORMAL "\n");
#endif
                continue;
}
            bmpregion_trim_margins(&pageregions->pageregion[ipr].bmpregion,k2settings,0xf);
            }

#if (WILLUSDEBUGX & 0x200)
{
int i;
printf("After trimming:\n");
for (i=0;i<pageregions->n;i++)
printf("    %d. (%d,%d) - (%d,%d)\n",i+1,
pageregions->pageregion[i].bmpregion.c1,
pageregions->pageregion[i].bmpregion.r1,
pageregions->pageregion[i].bmpregion.c2,
pageregions->pageregion[i].bmpregion.r2);
}
#endif

    /* v2.33 */
    /* Special sorting if more than 1 column and no notes--apply ragged column edge logic */
    if (maxlevels>=2)
        {
        int has_notes;

        has_notes=0;
        for (ipr=0;ipr<pageregions->n;ipr++)
            if (pageregions->pageregion[ipr].notes)
                {
                has_notes=1;
                break;
                }
        if (!has_notes)
            {
            /* Find text rows in each region--required for good sorting. */
            for (ipr=0;ipr<pageregions->n;ipr++)
                bmpregion_find_textrows(&pageregions->pageregion[ipr].bmpregion,k2settings,1,1,
                                        k2settings->join_figure_captions);
            pageregions_sort(pageregions,k2settings->src_dpi,k2settings->src_left_to_right,
                                         k2settings->column_offset_max,
                                         k2settings->column_row_gap_height_in,
                                         k2settings->max_column_gap_inches);
            }
        }

    for (ipr=0;ipr<pageregions->n;ipr++)
        {
        int level,fitcols;
        BMPREGION *notes_region,*main_text_region;

        if (ipr<pageregions->n-1 && pageregions->pageregion[ipr].notes)
            {
            notes_region=&pageregions->pageregion[ipr].bmpregion;
            ipr++;
            main_text_region=&pageregions->pageregion[ipr].bmpregion;
            }
        else
            {
            notes_region=NULL;
            main_text_region=&pageregions->pageregion[ipr].bmpregion;
            }

        /*
        ** v2.20
        ** For notes, really need to check to see if fitting the region
        ** to the output page is turned on.
        */

        
        /* Check for dynamic adjustment of output page to trimmed source region */
        /* Set device width/height to trimmed size if requested */
        /* v2.35--only call if region has finite size */
        if (trim_regions && main_text_region->r1<main_text_region->r2
                         && main_text_region->c1<main_text_region->c2)
            k2pdfopt_settings_set_margins_and_devsize(k2settings,main_text_region,
                                                      masterinfo,-1.0,1);

        /* Process this region */
        level = pageregions->pageregion[ipr].level;
        if (k2settings_has_cropboxes(k2settings) || gridded || !pageregions->pageregion[ipr].fullspan)
            {
            level *= 2;
            fitcols = k2settings->fit_columns;
            }
        else
            fitcols = (k2settings->fit_columns && (level>1));
/*
printf("vert break region[%d], level=%d\n",ipr,level);
*/
#if (WILLUSDEBUGX & 0x200)
aprintf(ANSI_RED "mi->mandatory_region_gap changed to %d by source_page_add." ANSI_NORMAL "\n",masterinfo->mandatory_region_gap);
#endif
#if (WILLUSDEBUGX & 0x40000)
{
if (notes_region)
{
static int count=1;
char buf[MAXFILENAMELEN];
sprintf(buf,"notes%04d.png",count);
bmpregion_write(notes_region,buf);
sprintf(buf,"main%04d.png",count);
bmpregion_write(main_text_region,buf);
count++;
}
}
#endif
        /*
        ** If this region is a "notes" region, then the next region should be
        ** the main text that goes with the notes.  Pass both to the function.
        */
#if (WILLUSDEBUGX & 1)
printf("k2settings->fit_columns=%d\n",k2settings->fit_columns);
printf("fitcols=%d\n",fitcols);
#endif
        bmpregion_vertically_break(main_text_region,k2settings,masterinfo,
                                   fitcols?-2.0:-1.0,pages_done,level,notes_region);

        /* Flush output if required */
        if (masterinfo->fit_to_page==-2)
            masterinfo_flush(masterinfo,k2settings);
        }
    /* Restore default DPI if it was altered */
/*
    if (size_reset)
{
#if (WILLUSDEBUGX & 0x200)
printf("Restoring DPI?\n");
#endif
        k2settings->dst_width = k2settings->dst_userwidth;
        k2pdfopt_settings_set_margins_and_devsize(k2settings,NULL,NULL,-99.,0);
}
*/
    pageregions_free(pageregions);
    }


/*
** Set up gridded pageregion array
** (Blind Grid Output--no attempt to find breaks between rows or columns)
*/
static void pageregions_grid(PAGEREGIONS *pageregions,BMPREGION *region,
                             K2PDFOPT_SETTINGS *k2settings,int level)

    {
    int i,nr;

    nr=k2settings->src_grid_cols*k2settings->src_grid_rows;
    for (i=0;i<nr;i++)
        {
        int r,c,gw,gh,gwo,gho;
        BMPREGION *srcregion,_srcregion;

        srcregion=&_srcregion;
        bmpregion_init(srcregion);
        bmpregion_copy(srcregion,region,0);
        /* Modified in v2.35--more precise */
        gwo=k2settings->src_grid_overlap_percentage*region->bmp8->width/100.+.5;
        gho=k2settings->src_grid_overlap_percentage*region->bmp8->height/100.+.5;
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
        pageregions_add_pageregion(pageregions,srcregion,level,0,0);
        bmpregion_free(srcregion);
        }
    }


/*
** Determine page regions from user-specified crop boxes.
*/
static void pageregions_from_cropboxes(PAGEREGIONS *pageregions,BMPREGION *srcregion,
                                       K2PDFOPT_SETTINGS *k2settings,MASTERINFO *masterinfo)

    {
    int i;

#if (WILLUSDEBUGX & 0x200)
int c;
c=0;
printf("@pageregions_from_cropboxes:  n boxes=%d\n",k2cropboxes_count(&k2settings->cropboxes,K2CROPBOX_FLAGS_IGNOREBOXEDAREA|K2CROPBOX_FLAGS_NOTUSED,0));
#endif
    for (i=0;i<k2settings->cropboxes.n;i++)
        {
        K2CROPBOX *cropbox;
        BMPREGION *croppedregion,_croppedregion;
        /* int units2[4]; */

        if (k2cropboxes_skip_box(&k2settings->cropboxes,i,masterinfo->pageinfo.srcpage,
                                 masterinfo->srcpages,0))
            continue;
        cropbox=&k2settings->cropboxes.cropbox[i];
        croppedregion=&_croppedregion;
        bmpregion_init(croppedregion);
        bmpregion_copy(croppedregion,srcregion,0);
#if (WILLUSDEBUGX & 0x200)
printf("    cropbox[%d].pagelist='%s'\n",c++,cropbox->pagelist);
printf("    srcpage = %d of %d\n",masterinfo->pageinfo.srcpage,masterinfo->srcpages);
printf("    pagelist includes=%d\n",pagelist_includes_page(cropbox->pagelist,masterinfo->pageinfo.srcpage,masterinfo->srcpages));
#endif
/*
{
int status;
status=pagelist_includes_page(cropbox->pagelist,masterinfo->pageinfo.srcpage,masterinfo->srcpages);
printf("pagelist_includes_page('%s',%d,%d)=%d\n",cropbox->pagelist,masterinfo->pageinfo.srcpage,masterinfo->srcpages,status);
}
*/

/*
printf("Cropbox:\n");
{
int jj;
for (jj=0;jj<4;jj++)
printf("    %d. %g %d\n",jj,cropbox->box[jj],cropbox->units[jj]);
}
*/
/*
printf("c1,r1 c2,r2 = (%d,%d)-(%d,%d)\n",croppedregion->c1,croppedregion->r1,croppedregion->c2,croppedregion->r2);
*/
        bmpregion_set_cropbox_pixels(croppedregion,cropbox,srcregion,masterinfo);
        pageregions_add_pageregion(pageregions,croppedregion,0,0,0);
        bmpregion_free(croppedregion);
        }
    }


static int k2cropboxes_skip_box(K2CROPBOXES *cropboxes,int index,int srcpage,int npages,int iboxes)

    {
    K2CROPBOX *cropbox;

    cropbox=&cropboxes->cropbox[index];

    if (cropbox->cboxflags&K2CROPBOX_FLAGS_NOTUSED)
        return(1);
    if (!iboxes && (cropbox->cboxflags&K2CROPBOX_FLAGS_IGNOREBOXEDAREA))
        return(1);
    if (iboxes && !(cropbox->cboxflags&K2CROPBOX_FLAGS_IGNOREBOXEDAREA))
        return(1);

    /* If no page list, crop box used on every page */
    if (cropbox->pagelist[0]=='\0')
        return(0);

    /* Unspecified pages */
    if (!stricmp(cropbox->pagelist,"u"))
        {
        int i;

        for (i=0;i<cropboxes->n;i++)
            {
            if (i==index)
                continue;
            cropbox=&cropboxes->cropbox[i];
            if (cropbox->cboxflags&K2CROPBOX_FLAGS_NOTUSED)
                continue;
            if (!iboxes && (cropbox->cboxflags&K2CROPBOX_FLAGS_IGNOREBOXEDAREA))
                continue;
            if (iboxes && !(cropbox->cboxflags&K2CROPBOX_FLAGS_IGNOREBOXEDAREA))
                continue;
            if (cropbox->pagelist[0]=='\0' || !stricmp(cropbox->pagelist,"u"))
                continue;
            if (pagelist_includes_page(cropbox->pagelist,srcpage,npages))
                return(1);
            }
        return(0);
        }

    /* Don't apply cropbox if page not in pagelist */
    return(!pagelist_includes_page(cropbox->pagelist,srcpage,npages));
    }


/*
** White out source bitmap areas specified by "ignore" boxes
*/
static void bmpregion_whiteout_cropboxes(BMPREGION *srcregion,K2PDFOPT_SETTINGS *k2settings,
                                         MASTERINFO *masterinfo)

    {
    int i;

#if (WILLUSDEBUGX & 0x200)
int c;
c=0;
printf("@pageregions_whiteout_cropboxes:  n boxes=%d\n",k2cropboxes_count(&k2settings->cropboxes,K2CROPBOX_FLAGS_NOTUSED|K2CROPBOX_FLAGS_IGNOREBOXEDAREA,K2CROPBOX_FLAGS_IGNOREBOXEDAREA));
#endif
    for (i=0;i<k2settings->cropboxes.n;i++)
        {
        K2CROPBOX *cropbox;
        BMPREGION *croppedregion,_croppedregion;
        /* int units2[4]; */

        if (k2cropboxes_skip_box(&k2settings->cropboxes,i,masterinfo->pageinfo.srcpage,
                                 masterinfo->srcpages,1))
            continue;
        cropbox=&k2settings->cropboxes.cropbox[i];
        croppedregion=&_croppedregion;
        bmpregion_init(croppedregion);
        bmpregion_copy(croppedregion,srcregion,0);
#if (WILLUSDEBUGX & 0x200)
printf("    cropbox[%d].pagelist='%s'\n",c++,cropbox->pagelist);
printf("    srcpage = %d of %d\n",masterinfo->pageinfo.srcpage,masterinfo->srcpages);
printf("    pagelist includes=%d\n",pagelist_includes_page(cropbox->pagelist,masterinfo->pageinfo.srcpage,masterinfo->srcpages));
#endif
/*
{
int status;
status=pagelist_includes_page(cropbox->pagelist,masterinfo->pageinfo.srcpage,masterinfo->srcpages);
printf("pagelist_includes_page('%s',%d,%d)=%d\n",cropbox->pagelist,masterinfo->pageinfo.srcpage,masterinfo->srcpages,status);
}
*/
/*
printf("Cropbox:\n");
{
int jj;
for (jj=0;jj<4;jj++)
printf("    %d. %g %d\n",jj,cropbox->box[jj],cropbox->units[jj]);
}
*/
/*
printf("c1,r1 c2,r2 = (%d,%d)-(%d,%d)\n",croppedregion->c1,croppedregion->r1,croppedregion->c2,croppedregion->r2);
*/
        bmpregion_set_cropbox_pixels(croppedregion,cropbox,srcregion,masterinfo);
        bmpregion_whiteout(srcregion,croppedregion);
        bmpregion_free(croppedregion);
        }
    }


/*
** Set dstregion c1,r1,c2,r2 based on cropbox.
*/
static void bmpregion_set_cropbox_pixels(BMPREGION *dstregion,K2CROPBOX *cropbox,
                                         BMPREGION *srcregion,MASTERINFO *masterinfo)

    {
    LINE2D userrect;
    POINT2D pagedims_inches;

    /* Convert cropbox units to pixels */
    userrect.p[0].x = cropbox->box[0];
    userrect.p[0].y = cropbox->box[1];
    userrect.p[1].x = cropbox->box[0]+cropbox->box[2];
    userrect.p[1].y = cropbox->box[1]+cropbox->box[3];

    /* For calculating width/height */
    /*
    userrect2.p[0].x = 0.;
    userrect2.p[0].y = 0.;
    userrect2.p[1].x = cropbox->box[2];
    userrect2.p[1].y = cropbox->box[3];
    units2[0]=units2[2]=cropbox->units[0];
    units2[1]=units2[3]=cropbox->units[1];
    */
    /* Page dimensions in inches */
    pagedims_inches.x = (double)srcregion->bmp8->width / srcregion->dpi;
    pagedims_inches.y = (double)srcregion->bmp8->height / srcregion->dpi;
    masterinfo_convert_to_source_pixels(masterinfo,&userrect,cropbox->units,&pagedims_inches,
                                        (double)srcregion->dpi,NULL);
    /*
    masterinfo_convert_to_source_pixels(masterinfo,&userrect2,units2,&pagedims_inches,
                                        (double)region->dpi,NULL);
    */

    /* Establish cropbox */
    dstregion->c1 = (int)(userrect.p[0].x+.5);
    if (dstregion->c1<0)
        dstregion->c1=0;
    if (dstregion->c1 > srcregion->bmp8->width-1)
        dstregion->c1=srcregion->bmp8->width-1;
    if (cropbox->box[2]<=0.)
        dstregion->c2 = srcregion->bmp8->width-1;
    else
        dstregion->c2 = (int)(userrect.p[1].x+.5);
    if (dstregion->c2<dstregion->c1)
        dstregion->c2=dstregion->c1;
    if (dstregion->c2> srcregion->bmp8->width-1)
        dstregion->c2=srcregion->bmp8->width-1;
    dstregion->r1 = (int)(userrect.p[0].y+.5);
    if (dstregion->r1<0)
        dstregion->r1=0;
    if (dstregion->r1 > srcregion->bmp8->height-1)
        dstregion->r1=srcregion->bmp8->height-1;
    if (cropbox->box[3]<=0.)
        dstregion->r2 = srcregion->bmp8->height-1;
    else
        dstregion->r2 = (int)(userrect.p[1].y+.5);
    if (dstregion->r2<dstregion->r1)
        dstregion->r2=dstregion->r1;
    if (dstregion->r2 > srcregion->bmp8->height-1)
        dstregion->r2=srcregion->bmp8->height-1;
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
void pageregions_find_columns(PAGEREGIONS *pageregions_sorted,BMPREGION *srcregion,
                              K2PDFOPT_SETTINGS *k2settings,MASTERINFO *masterinfo,
                              int  maxlevels)

    {
    int ilevel;
    K2NOTES *notes;

#if (!(WILLUSDEBUGX & 0x200))
    if (k2settings->debug)
#endif
        k2printf("@pageregions_find_columns (%d,%d) - (%d,%d) maxlevels=%d\n",
               srcregion->c1,srcregion->r1,srcregion->c2,srcregion->r2,maxlevels);

    /*
    ** v2.20:  Check to see if we should be looking for a "notes" column
    */
    notes = page_has_notes_margin(k2settings,masterinfo);
    if (notes!=NULL)
        maxlevels++;
    if (maxlevels==1)
        {
        pageregions_add_pageregion(pageregions_sorted,srcregion,1,1,0);
        return;
        }
    pageregions_find_next_level(pageregions_sorted,srcregion,k2settings,1,notes);
    for (ilevel=2;ilevel<maxlevels;ilevel++)
        {
        int j;

        for (j=0;j<pageregions_sorted->n;j++)
            {
            /* If region is a candidate for sub-dividing, then do it */
            if (pageregions_sorted->pageregion[j].level==ilevel-1
                     && pageregions_sorted->pageregion[j].fullspan==0
                     && pageregions_sorted->pageregion[j].notes==0)
                {
                PAGEREGIONS *pageregions,_pageregions;

                pageregions=&_pageregions;
                pageregions_init(pageregions);
                /* Sub-divide region */
                pageregions_find_next_level(pageregions,
                                      &pageregions_sorted->pageregion[j].bmpregion,
                                      k2settings,ilevel,NULL);
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
                                        K2PDFOPT_SETTINGS *k2settings,int level,K2NOTES *notes)

    {
    static char *funcname="pageregions_find_next_level";
    int *row_black_count;
    int rh,r0,cgr;
    PAGEREGIONS *pageregions,_pageregions;
    int ipr;

#if (!(WILLUSDEBUGX & 0x200))
    if (k2settings->debug)
#endif
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
        {
        k2printf("Calculating row histogram.\n");
        bmpregion_row_histogram(srcregion);
        k2printf("Done calculating row histogram.\n");
        }

    /* Unsorted array */
    pageregions=&_pageregions;
    pageregions_init(pageregions);

    /* Find all column dividers in source region and store sequentially in pageregion[] array */
#if (WILLUSDEBUGX & 0x200)
printf("Looking for multi-column divider in source region.\n");
#endif
    for (rh=0;srcregion->r1<=srcregion->r2;srcregion->r1+=rh)
        {
        int n0;

        n0=pageregions->n;
#if (WILLUSDEBUGX & 0x200)
if (notes)
printf("Looking for multi-column divider with notes.\n");
else
printf("Looking for multi-column divider.\n");
#endif
        rh=bmpregion_find_multicolumn_divider(srcregion,k2settings,row_black_count,pageregions,notes);
#if (WILLUSDEBUGX & 0x200)
printf("    rh=%d, found %d regions.\n",rh,pageregions->n-n0);
{
int i;
for (i=n0;i<pageregions->n;i++)
printf("        Region %d:  (%d,%d)-(%d,%d)\n",i-n0+1,pageregions->pageregion[i].bmpregion.c1,
pageregions->pageregion[i].bmpregion.r1,
pageregions->pageregion[i].bmpregion.c2,
pageregions->pageregion[i].bmpregion.r2);
}
#endif
        if (notes && pageregions->n-n0<=1 && level<k2settings->max_columns)
            {
#if (WILLUSDEBUGX & 0x200)
printf("No notes found--trying multi-column.  level=%d, col=%d\n",level,k2settings->max_columns);
#endif
            pageregions->n=n0;
            rh=bmpregion_find_multicolumn_divider(srcregion,k2settings,row_black_count,pageregions,NULL);
#if (WILLUSDEBUGX & 0x200)
printf("    rh=%d, found %d regions.\n",rh,pageregions->n-n0);
#endif
            }
        if (k2settings->verbose)
            k2printf("rh=%d/%d\n",rh,srcregion->r2-srcregion->r1+1);
        }

    /* Sort pageregions array into pageregions_sorted array (by order of display) */
#if (!(WILLUSDEBUGX & 0x200))
    if (k2settings->debug)
#endif
        k2printf("Found %d page regions while looking for columns\n",pageregions->n);
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

                    if (notes)
                        break;
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
                pageregions_add_pageregion(pageregions_sorted,&pageregion->bmpregion,level,0,
                                           pageregion->notes);
                }
            if (jpr==ipr)
                break;
            }
        if (jpr<pageregions->n && pageregions->pageregion[jpr].fullspan)
            {
            if (k2settings->debug)
                k2printf("SINGLE COLUMN REGION...\n");
            pageregions_add_pageregion(pageregions_sorted,&pageregions->pageregion[jpr].bmpregion,
                                       level,1,0);
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
** NOTE:  This function is recursive.
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
** trim_flags & 0x80 :  Do NOT re-trim no matter what.
**
** If wrectmaps is not NULL, it contains a sequence of mappings to the original
** source pages that can be used for tracking the positions of words on the
** original source.
**
** If textrow!=NULL then the passed region is a single line with the given
** TEXTROW parameters.
**
*/
void bmpregion_add(ADDED_REGION_INFO *added_region,K2PDFOPT_SETTINGS *k2settings,
                   MASTERINFO *masterinfo)

    {
    int w,wmax,i,nc,nr,h,bpp,tall_region,is_figure,bmp_rotation_deg;
    double region_width_inches;
    double region_height_inches;
    WILLUSBITMAP *bmp,_bmp;
    BMPREGION *region;
    BMPREGION *newregion,_newregion;

    region=added_region->region;
    newregion=&_newregion;
    bmpregion_init(newregion);
    bmpregion_copy(newregion,added_region->region,1);
#if (WILLUSDEBUGX & 0x000201)
k2printf("@bmpregion_add (%d,%d) - (%d,%d)\n",region->c1,region->r1,region->c2,region->r2);
k2printf("    trimflags = %X\n",added_region->trim_flags);
k2printf("    allow_text_wrapping = %d\n",added_region->allow_text_wrapping);
k2printf("    allow_vert_breaks = %d\n",added_region->allow_vertical_breaks);
k2printf("    dst_dpi=%d\n",k2settings->dst_dpi);
#endif
    if (k2settings->debug)
        {
        if (!added_region->allow_text_wrapping)
            k2printf("@bmpregion_add (no break) (%d,%d) - (%d,%d) (scale=%g)\n",
                region->c1,region->r1,region->c2,region->r2,added_region->force_scale);
        else
            k2printf("@bmpregion_add (allow break) (%d,%d) - (%d,%d) (scale=%g)\n",
                region->c1,region->r1,region->c2,region->r2,added_region->force_scale);
        }
    /*
    ** Tag blank rows and columns and trim the blank margins off
    ** trimflags = 0xf for all margin trim.
    ** trimflags = 0xc for just top and bottom margins.
    */
    bmpregion_trim_margins(newregion,k2settings,added_region->trim_flags&0xf);
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
        {
        bmpregion_free(newregion);
        return;
        }
    region_width_inches = (double)nc/newregion->dpi;
    region_height_inches = (double)nr/newregion->dpi;

    /*
    ** Break region up if page-break marks are found.
    */
    if (added_region->maps_to_source && newregion->k2pagebreakmarks!=NULL 
                                     && newregion->k2pagebreakmarks->n>0)
        {
        int i,c1,c2;

        c1=newregion->c1;
        c2=newregion->c2;
        if (k2settings->src_left_to_right)
            c1 -= newregion->dpi;
        else
            c2 += newregion->dpi;
        for (i=0;i<newregion->k2pagebreakmarks->n;i++)
            {
            K2PAGEBREAKMARK *mark;

            mark=&region->k2pagebreakmarks->k2pagebreakmark[i];
            if (mark->type<0 || mark->col<c1 || mark->col>c2)
                continue;
            if (mark->row<=newregion->r1)
                {
                masterinfo_add_pagebreakmark(masterinfo,mark->type);
                mark->type=-1;
                continue;
                }
            /* Region has mark in the middle -- split it. */
            if (mark->row<newregion->r2)
                {
                ADDED_REGION_INFO new_added_region;
                int r2;

                new_added_region = (*added_region);
                new_added_region.region = newregion;
                r2=newregion->r2;
                new_added_region.region->r2=mark->row-1;
                bmpregion_add(&new_added_region,k2settings,masterinfo);
                new_added_region.region->r1=mark->row;
                new_added_region.region->r2=r2;
                bmpregion_add(&new_added_region,k2settings,masterinfo);
                bmpregion_free(newregion);
                return;
                }
            }
        }

// k2printf("bmpregion_add:  rwidth_in = %.2f = %d / %d\n",region_width_inches,nc,newregion->dpi);
    /* Use untrimmed region left/right if possible */
    /* added_region->caller_id==1 means we are called from bmpregion_vertically_break function */
    if (added_region->caller_id==1 && region_width_inches <= k2settings->max_region_width_inches)
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
        region_height_inches = (double)(region->r2-region->r1+1)/newregion->dpi;
        }
        
    /*
    ** Try breaking the region into smaller horizontal pieces (wrap text lines)
    */
/*
k2printf("allow_text_wrapping=%d, region_width_inches=%g, max_region_width_inches=%g\n",
added_region->allow_text_wrapping,region_width_inches,k2settings->max_region_width_inches);
*/
    /* New in v1.50, if added_region->allow_text_wrapping==2, unwrap short lines. */
/*
k2printf("tw=%d, region_width_inches=%g, max_region_width_inches=%g\n",added_region->allow_text_wrapping,region_width_inches,k2settings->max_region_width_inches);
*/

    /*
    **
    ** Note:
    ** added_region->allow_text_wrapping will only be non-zero when bmpregion_add() is called
    ** from bmpregion_vertically_break().
    **
    */
    if (added_region->allow_text_wrapping==2 
         || (added_region->allow_text_wrapping==1 
                 && region_width_inches > k2settings->max_region_width_inches))
        {
        ADDED_REGION_INFO new_added_region;

        new_added_region.region=newregion;
        new_added_region.maps_to_source = added_region->maps_to_source;
        new_added_region.allow_text_wrapping=1;
        new_added_region.force_scale=added_region->force_scale;
        new_added_region.region_is_centered=added_region->region_is_centered;
        bmpregion_analyze_justification_and_line_spacing(&new_added_region,k2settings,masterinfo);
        bmpregion_free(newregion);
        return;
        }

    /*
    ** If allowed, re-submit each vertical region individually
    **
    ** Note:
    ** added_region->allow_vertical_breaks will only be non-zero when bmpregion_add() is called
    ** from bmpregion_vertically_break().
    **
    */
    if (added_region->allow_vertical_breaks)
        {
        ADDED_REGION_INFO new_added_region;

        new_added_region.region=newregion;
        new_added_region.maps_to_source=added_region->maps_to_source;
        new_added_region.allow_text_wrapping=0;
        new_added_region.force_scale=added_region->force_scale;
        new_added_region.region_is_centered=added_region->region_is_centered;
        bmpregion_analyze_justification_and_line_spacing(&new_added_region,k2settings,masterinfo);
        bmpregion_free(newregion);
        return;
        }

    /*
    ** AT THIS POINT, BITMAP IS NOT TO BE BROKEN UP HORIZONTALLY OR VERTICALLY.
    ** (IT IS AN "ATOMIC" REGION.)
    ** (IT CAN STILL BE FULLY JUSTIFIED IF ALLOWED.)
    */

    /*
    ** Scale region to fit the destination device width and add to the master bitmap.
    **
    **
    ** Start by copying source region to new bitmap 
    **
    */
// k2printf("c1=%d\n",newregion->c1);
    /* Is it a "tall" region or a figure? */
    tall_region = region_height_inches >= k2settings->dst_min_figure_height_in;
#if (WILLUSDEBUGX & 1)
printf("atomic region:  nrows=%d, type=%d\n",newregion->textrows.n,newregion->textrows.n>0?newregion->textrows.textrow[0].type:-1);
printf("                region width = %g in\n",region_width_inches);
#endif
#if (WILLUSDEBUGX & 512)
if (newregion->k2pagebreakmarks!=NULL)
aprintf(ANSI_CYAN "\nATOMIC REGION HAS PAGEBREAKMARKS.\n\n" ANSI_NORMAL);
#endif
    if (newregion->textrows.n<=0)
        is_figure = region_is_figure(k2settings,region_width_inches,region_height_inches);
    else if (region->textrows.n==1)
        {
        textrow_determine_type(newregion,k2settings,0);
        is_figure = (region->textrows.textrow[0].type==REGION_TYPE_FIGURE);
        }
    else
        is_figure = 0;

    /* Did user request to remove all figures? */
    if (k2settings->text_only && is_figure)
        {
        bmpregion_free(newregion);
        /* Use -to+ for this? */
        if (k2settings->dst_break_pages==4) /* Break page if we're skipping a figure */
            masterinfo_flush(masterinfo,k2settings);
        return;
        }

    /* Re-trim left and right? */
    if ((added_region->trim_flags&0x80)==0)
        {
        /* If tall region and figure justification turned on ... */
        if ((tall_region && k2settings->dst_figure_justify>=0)
                /* ... or if centered region ... */
                || ((added_region->trim_flags&3)!=3 && ((added_region->justification_flags&3)==1
                     || ((added_region->justification_flags&3)==3
                     && (k2settings->dst_justify==1
                         || (k2settings->dst_justify<0 
                                && (added_region->justification_flags&0xc)==4))))))
            {
            bmpregion_trim_margins(newregion,k2settings,0x3);
            nc=newregion->c2-newregion->c1+1;
            region_width_inches = (double)nc/newregion->dpi;
            region_height_inches = (double)(newregion->r2-newregion->r1+1)/newregion->dpi;
            }
        }
#if (WILLUSDEBUGX & 1)
if (added_region->caller_id!=2)
{
char buf[MAXFILENAMELEN];
static int count=0;
sprintf(buf,"atomic%04d.png",count++);
bmpregion_write(newregion,buf);
    k2printf("atomic region %04d:  " ANSI_CYAN "%.2f x %.2f in" ANSI_NORMAL " c1,r1=%d,%d, (%d x %d) (rbdel=%d) just=0x%02X\n",
                   count-1,
                   (double)(newregion->c2-newregion->c1+1)/newregion->dpi,
                   (double)(newregion->r2-newregion->r1+1)/newregion->dpi,
                   newregion->c1,newregion->r1,
                   (newregion->c2-newregion->c1+1),
                   (newregion->r2-newregion->r1+1),
                   added_region->rowbase_delta,added_region->justification_flags);
}
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
/*
{
static int gotone=0;
{
double prc,pr1,pr2;
prc=(double)72.*newregion->c1/newregion->dpi;
pr1=(double)72.*newregion->r1/newregion->dpi;
pr2=(double)72.*newregion->r2/newregion->dpi;
if (prc> 568. && pr1<78. && pr2>78.)
{
printf("dpi=%d\n",newregion->dpi);
printf("atomic region, source=(%.1f,%.1f) - (%.1f,%.1f)\n",72.*newregion->c1/newregion->dpi,
72.*newregion->r1/newregion->dpi,
72.*newregion->c2/newregion->dpi,
72.*newregion->r2/newregion->dpi);
printf("               pixels=(%d,%d) - (%d,%d)\n",newregion->c1,
newregion->r1,
newregion->c2,
newregion->r2);
gotone=1;
}
else if (gotone>0)
gotone++;
}
}
*/
// k2printf("r1=%d, r2=%d\n",newregion->r1,newregion->r2);
    for (i=newregion->r1;i<=newregion->r2;i++)
        {
        unsigned char *psrc,*pdst;

        pdst=bmp_rowptr_from_top(bmp,i-newregion->r1);
        psrc=bmp_rowptr_from_top(k2settings->dst_color ? newregion->bmp : newregion->bmp8,i)+bpp*newregion->c1;
        memcpy(pdst,psrc,nc*bpp);
        }

    /* Locate relevant page break markers, if any */
/*
    bmpregion_local_pagebreakmarkers(newregion,k2settings->src_left_to_right,
                                               k2settings->src_whitethresh);
{
int i;
for (i=0;i<newregion->k2pagebreakmarks->n;i++)
{
K2PAGEBREAKMARK *mark;
mark=&newregion->k2pagebreakmarks->k2pagebreakmark[i];
if (mark->type>=0)
printf("    MARK %d at %d\n",mark->type,mark->row);
}
printf("\n");
}
*/
        
        

    /*
    ** Now scale to appropriate destination size.
    **
    ** added_region->force_scale is used to maintain uniform scaling so that
    ** most of the regions are scaled at the same value.
    **
    ** added_region->force_scale = -2.0 : Fit column width to display width
    ** added_region->force_scale = -1.0 : Use output dpi unless the region doesn't fit.
    **                                    In that case, scale it down until it fits.
    ** added_region->force_scale > 0.0  : Scale region by added_region->force_scale.
    **
    */
    /* Get max viewable pixel width on device screen */
    {
    int dstmar_pixels[4];
    get_dest_margins(dstmar_pixels,k2settings,k2settings->dst_dpi,masterinfo->bmp.width,
                     k2settings->dst_height);
    wmax=masterinfo->bmp.width-dstmar_pixels[0]-dstmar_pixels[2];
    }

#if (WILLUSDEBUGX & 1)
printf("Atomic region:  is_figure=%d, k2settings->dst_figure_rotate=%d\n",is_figure,k2settings->dst_figure_rotate);
#endif
    /* If device output is negative (white text on black background), pre-invert */
    /* the bitmap if it is a figure so that when it gets inverted again in the   */
    /* output bitmap stream, it will come out correctly.                         */
    if (is_figure && k2settings->dst_negative==1)
        bmp_invert(bmp);

    /* Rotate figure to optimum aspect ratio if requested */
    /* -fr option */
    bmp_rotation_deg=0;
    if (is_figure && k2settings->dst_figure_rotate)
        {
        double dst_vwidth_in,dst_vheight_in;

        /* width and height already take landscape mode into account. */
        /* E.g. for most readers, in landscape, vwidth > vheight */
        k2pdfopt_settings_dst_viewable(k2settings,masterinfo,&dst_vwidth_in,&dst_vheight_in);
        /* If figure fits better in a rotated orientation, rotate it */
        if ((dst_vheight_in > dst_vwidth_in 
                 && region_width_inches > region_height_inches
                 && region_width_inches > dst_vwidth_in)
             ||
             (dst_vheight_in < dst_vwidth_in 
                 && region_width_inches < region_height_inches
                 && region_height_inches > dst_vheight_in))
            {
            /* Rotate bitmap for better fit to display screen */
            bmp_rotation_deg=masterinfo->landscape?-90:90;
            bmp_rotate_right_angle(bmp,bmp_rotation_deg);
            }
        }

    if (added_region->force_scale > 0.)
        w = (int)(added_region->force_scale*bmp->width+0.5);
    else
        {
        if (region_width_inches < k2settings->max_region_width_inches)
            w=(int)(region_width_inches*k2settings->dst_dpi+.5);
        else
            w=wmax;
        }
    /* Special processing for tall regions (likely figures) */
    if (tall_region && w < wmax && k2settings->dst_fit_to_page!=0 
                                && k2settings->dst_fit_to_page!=-3)
        {
        if (k2settings->dst_fit_to_page<0)
            {
            double r1,r2;

            /* v2.20 bug fix:  max magnification = 5 x nominal, somewhat arbitrary */
            r1=(double)k2settings->dst_dpi / k2settings->src_dpi;
            r2=(double)w/wmax;
            if (r2/r1 < 0.2)
                w = 0.2*r1*wmax;
            else
                w = wmax;
            }
        else
            {
            w = (int)(w * (1.+(double)k2settings->dst_fit_to_page/100.) + 0.5);
            if (w > wmax)
                w = wmax;
            }
        }
    h=(int)(((double)w/bmp->width)*bmp->height+.5);
#if (WILLUSDEBUGX & 1)
printf("dst_dpi = %d\n",(int)k2settings->dst_dpi);
printf("max_region_width = %g in\n",k2settings->max_region_width_inches);
printf("forced_scale = %g\n",added_region->force_scale);
printf("Scaled bitmap:  w=%d, h=%d\n",w,h);
#endif

    /*
    ** If scaled dimensions are finite, add to master bitmap.
    */
    if (w>0 && h>0)
        {
        WILLUSBITMAP *tmp,_tmp;
        WRECTMAPS *wrmaps0,_wrmaps0;
        double scalew,scaleh;
        int nocr,npageboxes,justification_flags_ex;

        npageboxes=0;
        /* Not used as of v2.00 */
        /*
        k2settings->last_scale_factor_internal=(double)w/bmp->width;
        */
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
#if (WILLUSDEBUGX & 1)
printf("Resampling:  nocr=%d, %d x %d --> %d x %d\n",nocr,bmp->width,bmp->height,w,h);
#endif
        bmp_resample_optimum_performance(tmp,bmp,(double)0.,(double)0.,
                                          (double)bmp->width,(double)bmp->height,w,h);
        /*
        ** scalew and scaleh can be just different enough to cause problems
        ** if we use one value for both of them.  -- v2.00, 24-Aug-2013
        */
        scalew=(double)w/bmp->width;
        scaleh=(double)h/bmp->height;
        bmp_free(bmp);
/*
{
static int nn=0;
char filename[MAXFILENAMELEN];
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
            justification_flags_ex = k2settings->dst_figure_justify;
        else
            justification_flags_ex = added_region->justification_flags;
#ifdef HAVE_MUPDF_LIB
        /* Add source region corresponding to "tmp" bitmap to pageinfo structure */
        if (k2settings->use_crop_boxes)
            npageboxes=add_crop_boxes(newregion,k2settings,masterinfo,bmp_rotation_deg);
#endif /* HAVE_MUPDF_LIB */
        if (newregion->wrectmaps!=NULL)
            wrmaps0=newregion->wrectmaps;
        else
            {
            /* Create default WRECTMAP that spans the whole bitmap (used by dst_ocr=='m') */
            WRECTMAP wrmap;

#if (WILLUSDEBUGX & 0x400) 
printf("Creating single wrmap.\n");
#endif
            wrmaps0=&_wrmaps0;
            wrectmaps_init(wrmaps0);
            wrmap.srcpageno=newregion->pageno;
            wrmap.srcwidth=newregion->bmp->width;
            wrmap.srcheight=newregion->bmp->height;
            wrmap.srcdpiw=wrmap.srcdpih=newregion->dpi;
            wrmap.srcrot=newregion->rotdeg;
            wrmap.coords[0].x = newregion->c1;
            wrmap.coords[0].y = newregion->r1;
            wrmap.coords[1].x = 0;
            wrmap.coords[1].y = 0;
            wrmap.coords[2].x = newregion->c2-newregion->c1+1;
            wrmap.coords[2].y = newregion->r2-newregion->r1+1;
            wrectmaps_add_wrectmap(wrmaps0,&wrmap);
            }
        wrectmaps_scale_wrapbmp_coords(wrmaps0,scalew,scaleh);
        {
        TEXTROW trow;
/*
printf("textrow->rowheight=%d\n",textrow->rowheight);
*/
        trow=newregion->bbox;
        trow.r2 -= trow.r1;
        trow.rowbase -= trow.r1;
        trow.r1 = 0;
        trow.c2 -= trow.c1;
        trow.c1 = 0;
        textrow_scale(&trow,scalew,scaleh,tmp->width-1,tmp->height-1);
#if (WILLUSDEBUGX & 0x200)
printf("Calling masterinfo_add_bitmap w/textrow->rowheight=%d (scalew=%g, scaleh=%g)\n",trow.rowheight,scalew,scaleh);
#endif
        masterinfo_add_bitmap(masterinfo,tmp,k2settings,npageboxes,justification_flags_ex,
                       region->bgcolor,nocr,(int)((double)region->dpi*tmp->width/bmp->width+.5),
                       wrmaps0,&trow);
        }
        if (newregion->wrectmaps==NULL)
            wrectmaps_free(wrmaps0);
        bmp_free(tmp);
        }

    /* Store delta to base of text row (used by wrapbmp_flush()) */
    /*
    k2settings->last_rowbase_internal = added_region->rowbase_delta;
    */
    bmpregion_free(newregion);
    }


#ifdef HAVE_MUPDF_LIB
static int add_crop_boxes(BMPREGION *region,K2PDFOPT_SETTINGS *k2settings,MASTERINFO *masterinfo,
                          int bmp_rotation_deg)

    {
    WILLUSBITMAP bmp;
    WRECTMAPS *wrectmaps;
    int i,pn,status;
    double rot,finerot;

#if WILLUSDEBUGX & 64
printf("@add_crop_boxes()\n");
printf("   wrectmaps=%p\n",region->wrectmaps);
printf("   region->c1,r1=%d,%d, c2,r2=%d,%d\n",region->c1,region->r1,region->c2,region->r2);
#endif
    wrectmaps=region->wrectmaps;
    if (wrectmaps==NULL)
        return(add_crop_box(region,k2settings,masterinfo,bmp_rotation_deg));
    rot=masterinfo->pageinfo.srcpage_rot_deg;
    finerot=masterinfo->pageinfo.srcpage_fine_rot_deg;
    pn=masterinfo->pageinfo.srcpage;
    status=0;
    for (i=0;i<wrectmaps->n;i++)
        {
        WRECTMAP *wrmap;
        BMPREGION *subregion,_subregion;

        subregion=&_subregion;
        bmpregion_init(subregion);
        subregion->bmp8=subregion->bmp=&bmp;
        wrmap=&wrectmaps->wrectmap[i];
        bmp.width=wrmap->srcwidth;
        bmp.height=wrmap->srcheight;
        subregion->dpi=(int)((wrmap->srcdpiw+wrmap->srcdpih)/2.+.5);
        masterinfo->pageinfo.srcpage=wrmap->srcpageno;
        masterinfo->pageinfo.srcpage_rot_deg=wrmap->srcrot;
        masterinfo->pageinfo.srcpage_fine_rot_deg=0.;
        subregion->c1=wrmap->coords[0].x;
        subregion->r1=wrmap->coords[0].y;
        subregion->c2=subregion->c1+wrmap->coords[2].x-1;
        subregion->r2=subregion->r1+wrmap->coords[2].y-1;
        status+=add_crop_box(subregion,k2settings,masterinfo,bmp_rotation_deg);
        bmpregion_free(subregion);
        }
    masterinfo->pageinfo.srcpage_rot_deg=rot;
    masterinfo->pageinfo.srcpage_fine_rot_deg=finerot;
    masterinfo->pageinfo.srcpage=pn;
    return(status);
    }

    
static int add_crop_box(BMPREGION *region,K2PDFOPT_SETTINGS *k2settings,
                        MASTERINFO *masterinfo,int bmp_rotation_deg)

    {
    WPDFBOX _wpdfbox,*wpdfbox;
    WPDFSRCBOX *srcbox;
    WPDFPAGEINFO *pageinfo;
    BMPREGION *xregion,_xregion;
    double x0,y0,w,h,mar;

#if (WILLUSDEBUGX & 64)
k2printf("@add_crop_box (nboxes=%d)\n",masterinfo->pageinfo.boxes.n);
#endif
    pageinfo=&masterinfo->pageinfo;
    wpdfbox=&_wpdfbox;
    srcbox=&wpdfbox->srcbox;
    wpdfbox->dstpage = -1; /* -1 while still on master bitmap */
    wpdfbox->dst_width_pts = pageinfo->width_pts;
    wpdfbox->dst_height_pts = pageinfo->height_pts;
    wpdfbox->dstrot_deg = bmp_rotation_deg;
    srcbox->pageno = pageinfo->srcpage;
    srcbox->finerot_deg = pageinfo->srcpage_fine_rot_deg;
    srcbox->rot_deg = pageinfo->srcpage_rot_deg;
    srcbox->page_width_pts = 72.*region->bmp8->width/region->dpi;
    srcbox->page_height_pts = 72.*region->bmp8->height/region->dpi;

    /* Clip the source crop box with the page crop margins */
    xregion=&_xregion;
    bmpregion_init(xregion);
    xregion->bmp = region->bmp;
    xregion->bmp8 = region->bmp8;
    xregion->dpi = region->dpi;
    bmpregion_trim_to_crop_margins(xregion,masterinfo,k2settings);
    x0 = 72.*region->c1/region->dpi;
    y0 = 72.*(region->bmp8->height-1-region->r2)/region->dpi;
    w = 72.*(region->c2-region->c1+1)/region->dpi;
    h = 72.*(region->r2-region->r1+1)/region->dpi;
    mar=xregion->c1*srcbox->page_width_pts/region->bmp->width;
    if (mar>x0)
        {
        w -= (mar-x0);
        x0=mar;
        }
    mar=(region->bmp->width-1-xregion->c2)*srcbox->page_width_pts/region->bmp->width;
    if (w > srcbox->page_width_pts-mar-x0)
        w = srcbox->page_width_pts-mar-x0;
    mar=(region->bmp->height-1-xregion->r2)*srcbox->page_height_pts/region->bmp->height;
    if (mar>y0)
        {
        h -= (mar-y0);
        y0=mar;
        }
    mar=xregion->r1*srcbox->page_height_pts/region->bmp->height;
    bmpregion_free(xregion);
    if (h > srcbox->page_height_pts-mar-y0)
        h = srcbox->page_height_pts-mar-y0;
    srcbox->x0_pts = x0;
    srcbox->y0_pts = y0;
    srcbox->crop_width_pts = w;
    srcbox->crop_height_pts = h;

    if (srcbox->crop_width_pts > 0. && srcbox->crop_height_pts > 0.)
        {
        wpdfboxes_add_box(&pageinfo->boxes,wpdfbox);
        return(1);
        }
    return(0);
    }
#endif /* HAVE_MUPDF_LIB */


/*
** Returns height of region found and divider position in (*divider_column).
** (*divider_column) is absolute position on source bitmap.
**
** v2.20:  If left >= 0 then left and right specify that we are looking for
**         a column of notes in the margin.  Scan from left to right as
**         fractions of the source region width.
**
*/
static int bmpregion_find_multicolumn_divider(BMPREGION *region,K2PDFOPT_SETTINGS *k2settings,
                                              int *row_black_count,PAGEREGIONS *pageregions,
                                              K2NOTES *notes)

    {
    int itop,i,n,dm,middle,divider_column,min_height_pixels,mhp2,min_col_gap_pixels;
    BMPREGION _newregion,*newregion,column[2];
    int *rowmin,*rowmax;
    int *black_pixel_count_by_column;
    int *pixel_count_array;
    int rows_per_column;
    int notesleft;
    TEXTROWS *textrows;
    TEXTROW *textrow;
    static char *funcname="bmpregion_find_multicolumn_divider";

#if (!(WILLUSDEBUGX & 0x280))
    if (k2settings->debug)
#endif
        k2printf("@bmpregion_find_multicolumn_divider(%d,%d)-(%d,%d)\n",
                 region->c1,region->r1,region->c2,region->r2);
#if (WILLUSDEBUGX & 0x40000)
if (notes)
printf("Notes=%p, notesleft=%g, notesright=%g\n",notes,notes->left,notes->right);
#endif
    if (notes)
        notesleft = (notes->left+notes->right)/2. <= 0.5;
    else
        notesleft = 0;
    bmpregion_find_textrows(region,k2settings,0,0,k2settings->join_figure_captions==2 ? 1 : 0);
    textrows=&region->textrows;
    textrow=textrows->textrow;
    /* v2.33:  Join any rows with spacing less than column_row_gap_height_in */
    textrows_remove_small_rows(textrows,k2settings,0.25,0.5,region,k2settings->column_row_gap_height_in);
#if (WILLUSDEBUGX & 0x40000)
printf("After remove small rows:  %d rows left\n",textrows->n);
#endif
    n=textrows->n;
#if (!(WILLUSDEBUGX & 0x40000))
    if (k2settings->debug)
#endif
        {
        k2printf("region (%d,%d)-(%d,%d) has %d text rows:\n",
                region->c1,region->r1,region->c2,region->r2,region->textrows.n);
        for (i=0;i<region->textrows.n;i++)
            k2printf("    Rows %d - %d\n",region->textrows.textrow[i].r1,
                                          region->textrows.textrow[i].r2);
        }
    newregion=&_newregion;
    bmpregion_init(newregion);
    bmpregion_init(&column[0]);
    bmpregion_init(&column[1]);
    bmpregion_copy(newregion,region,0);
    min_height_pixels=k2settings->min_column_height_inches*region->dpi; /* src->height/15; */ 
    mhp2 = min_height_pixels-1;
    if (mhp2 < 0)
        mhp2=0;
    if (notes)
        {
        dm=1+(region->c2-region->c1+1)*(notes->right-notes->left)/2.;
        middle=notes->left*(region->c2-region->c1+1)+dm;
        }
    else
        {
        dm=1+(region->c2-region->c1+1)*k2settings->column_gap_range/2.;
        middle=(region->c2-region->c1+1)/2;
        }
    min_col_gap_pixels=(int)(k2settings->min_column_gap_inches*region->dpi+.5);
    if (k2settings->verbose)
        {
        k2printf("(dm=%d, width=%d, min_gap=%d)\n",dm,region->c2-region->c1+1,min_col_gap_pixels);
        k2printf("Checking regions (r1=%d, r2=%d, minrh=%d)..",region->r1,region->r2,min_height_pixels);
        fflush(stdout);
        }
    textrows_sort_by_row_position(&region->textrows);
    willus_dmem_alloc_warn(5,(void **)&rowmin,(region->c2+10)*3*sizeof(int),funcname,10);
    rowmax=&rowmin[region->c2+10];
    black_pixel_count_by_column=&rowmax[region->c2+10];
    rows_per_column=0;

    /*
    ** If enough memory cache pixel counts into large 2-D array for fast calcs
    */
    pixel_count_array=NULL;
    rows_per_column=0;
#if (WILLUSDEBUGX & 0x100000)
    willus_dmem_alloc_warn(38,(void **)&pixel_count_array,
                                  sizeof(int)*(region->c2+2)*(region->r2+2),
                                  funcname,10);
    if (1)
#else
    if (willus_mem_alloc((double **)&pixel_count_array,sizeof(int)*(region->c2+2)*(region->r2+2),
                                  funcname))
#endif
        {
        int bw,jmax;

        rows_per_column=region->r2+2;
        memset(pixel_count_array,0,sizeof(int)*(region->c2+2)+(region->r2+2));
        bw=bmp_bytewidth(region->bmp8);
        jmax = region->r2+2;
        /* Don't exceed bitmap height--v1.66 fix, 7-22-2013 */
        if (jmax > region->bmp8->height)
            jmax = region->bmp8->height;
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
            for (p+=bw,cp++,j=1;j<jmax;j++,p+=bw,cp++)
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
    if (!k2settings->src_trim && n>0)
        {
        textrow[0].r1=region->r1;
        textrow[n-1].r2=region->r2;
        }

    /* Start with top-most and bottom-most regions, look for column dividers */
    for (itop=0;itop<n && textrow[itop].r1<region->r2+1-min_height_pixels;itop++)
        {
        int ibottom;
#if (WILLUSDEBUGX & 128)
k2printf("itop=%d/%d\n",itop,n);
#endif

        for (ibottom=n-1;ibottom>=itop 
              && textrow[ibottom].r2-textrow[itop].r1 >= min_height_pixels;ibottom--)
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
k2printf("    ibot=%d/%d (dm=%d)\n",ibottom,n,dm);
printf("    region->c1=%d, middle=%d\n",region->c1,middle);
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
printf("i=%d of %d\n",i,dm);
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
                if (newregion->c2 > region->c2)
                    newregion->c2 = region->c2;
                newregion->r1=textrow[itop].r1;
                newregion->r2=textrow[ibottom].r2;
#if (WILLUSDEBUGX & 128)
printf("    Checking shaft:  (%d,%d) - (%d,%d)\n",newregion->c1,newregion->r1,newregion->c2,newregion->r2);
#endif
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
                    if (newregion->c2 > region->c2)
                        newregion->c2 = region->c2;
#if (WILLUSDEBUGX & 128)
printf("    Checking shaft:  (%d,%d) - (%d,%d)\n",newregion->c1,newregion->r1,newregion->c2,newregion->r2);
#endif
                    foundgap=bmpregion_is_clear(newregion,row_black_count,
                                           black_pixel_count_by_column,
                                           pixel_count_array,rows_per_column,
                                           k2settings->gtc_in);
                    }
                if (!foundgap)
                    continue;
#if (WILLUSDEBUGX & 128)
printf("    Shaft is clear!\n");
#endif
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
                                       textrow[itop].r1,textrow[ibottom].r2,
                                       divider_column);
                /* Modify status if we're detecting a "notes" column */
                if (notes)
                    {
                    /* Ignore gap being too large */
                    status &= (~4);
                    /* Notes column can be short--no limit */
                    if (notesleft)
                        status &= (~1);
                    else
                        status &= (~2);
                    }
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

#if (!(WILLUSDEBUGX & 128))
                    if (k2settings->verbose)
                        {
#endif
                        k2printf("\n    GOOD COLUMN DIVIDER: col gap=(%d,%d) - (%d,%d)\n"
                             "                 r1=%d, r2=%d\n",
                            newregion->c1,newregion->r1,newregion->c2,newregion->r2,
                            textrow[itop].r1,textrow[ibottom].r2);
#if (!(WILLUSDEBUGX & 128))
                        }
#endif
                    if (itop>0)
                        {
                        BMPREGION *br;
                        /* add 1-column (full span) region */
                        pageregions_add_pageregion(pageregions,region,0,1,0);
                        br=&pageregions->pageregion[pageregions->n-1].bmpregion;
                        br->r2=textrow[itop-1].r2;
                        if (br->r2 > br->bmp8->height-1)
                            br->r2 = br->bmp8->height-1;
                        bmpregion_trim_margins(br,k2settings,k2settings->src_trim?0xf:0);
                        }
                    /* Un-trim columns if requested */
                    if (!k2settings->src_trim)
                        {
                        column[0].c1=region->c1;
                        column[1].c2=region->c2;
                        }
                    /* Add two side-by-side columns */
                    {
                    int ic1,ic2;
                    /* Put "notes" column first if we have a notes column */
                    if (notes && !notesleft)
                        {
                        ic1=1;
                        ic2=0;
                        }
                    else
                        {
                        ic1=0;
                        ic2=1;
                        }
#if (WILLUSDEBUGX & 128)
printf("Notes region: (%d - %d) / %d\n",column[ic1].c1,column[ic1].c2,region->c2);
#endif
                    pageregions_add_pageregion(pageregions,&column[ic1],0,0,notes!=NULL);
                    pageregions_add_pageregion(pageregions,&column[ic2],0,0,0);
                    }
                    colheight = textrow[ibottom].r2-region->r1+1;
#if (WILLUSDEBUGX & 0x100000)
                    willus_dmem_free(38,(double **)&pixel_count_array,funcname);
#else
                    willus_mem_free((double **)&pixel_count_array,funcname);
#endif
                    willus_dmem_free(5,(double **)&rowmin,funcname);
                    bmpregion_free(&column[1]);
                    bmpregion_free(&column[0]);
                    bmpregion_free(newregion);
#if (WILLUSDEBUGX & 128)
k2printf("Returning %d divider column = %d - %d\n",region->r2-region->r1+1,newregion->c1,newregion->c2);
#endif
                    return(colheight);
                    }
                }
#if (WILLUSDEBUGX & 128)
k2printf("        cols %d - %d, qec = %d, lec = %d\n",colmin,colmax,qec,lec);
#endif
            }
        }
#if (WILLUSDEBUGX & 128)
printf("Done looking for columns.\n");
#endif
#if (!(WILLUSDEBUGX & 128))
    if (k2settings->verbose)
#endif
        k2printf("NO GOOD REGION FOUND.\n");
    /* Add entire region (full span) */
    pageregions_add_pageregion(pageregions,region,0,1,0);
    bmpregion_trim_margins(&pageregions->pageregion[pageregions->n-1].bmpregion,
                           k2settings,k2settings->src_trim?0xf:0);
    /* (*divider_column)=region->c2+1; */
#if (WILLUSDEBUGX & 0x100000)
    willus_dmem_free(38,(double **)&pixel_count_array,funcname);
#else
    willus_mem_free((double **)&pixel_count_array,funcname);
#endif
    willus_dmem_free(5,(double **)&rowmin,funcname);
    bmpregion_free(&column[1]);
    bmpregion_free(&column[0]);
    bmpregion_free(newregion);
#if (WILLUSDEBUGX & 128)
k2printf("Returning %d\n",region->r2-region->r1+1);
#endif
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
static void bmpregion_vertically_break(BMPREGION *region,K2PDFOPT_SETTINGS *k2settings,
                                       MASTERINFO *masterinfo,double force_scale,
                                       int source_page,int ncols,BMPREGION *notes)

    {
    /* Keep track of last region dimensions */
    static int    last_ncols=-1;
    static double last_region_width_inches=-1.;
    static int    last_source_page=-1;
    static int    last_region_r2=-1;
    static int    last_page_height=-1;
    int i,biggap,revert;
    int region_is_centered;
    int ni,notesgap,notes_are_centered;
    TEXTROWS *notesrows;
    TEXTROWS *textrows;
    TEXTROW *textrow;
    int n;
    double region_width_inches,region_height_inches;
    int *firstrow,*lastrow;
    int ng;
    int *nfirstrow,*nlastrow;
    int nng;
    int trim_mode,rblank;
    ADDED_REGION_INFO added_region;
    static char *funcname="bmpregion_vertically_break";

    added_region.force_scale = force_scale;
    if (region==NULL)
        {
        last_ncols = -1;
        last_region_width_inches = -1.;
        last_source_page=-1;
        last_region_r2=-1;
        last_page_height=-1;
        return;
        }
/*
{
static int vreg=1;
char filename[MAXFILENAMELEN];
sprintf(filename,"vreg%04d.png",vreg++);
k2printf("\n\n@bmpregion_vertically_break.\n\n");
k2printf("    region to analyze = (%d,%d) - (%d,%d)\n",region->c1,region->r1,region->c2,region->r2);
k2printf("    vertical_break_threshold=%g\n",k2settings->vertical_break_threshold);
bmpregion_write(region,filename);
}
*/
#if (WILLUSDEBUGX & 0x101)
k2printf("\n\n@bmpregion_vertically_break.\n\n");
k2printf("    region to analyze = (%d,%d) - (%d,%d)\n",region->c1,region->r1,region->c2,region->r2);
k2printf("    vertical_break_threshold=%g\n",k2settings->vertical_break_threshold);
#endif
#if (WILLUSDEBUGX & 0x40000)
k2printf("    notes=%p\n",notes);
#endif
    /*
    ** text_wrap==0 no wrapping
    ** text_wrap==1 re-flow
    ** text_wrap==2 to re-flow short lines.
    */
    added_region.allow_text_wrapping=k2settings->text_wrap;
    added_region.allow_vertical_breaks=(k2settings->vertical_break_threshold > -1.5);
    /* Special case to break pages at "green" gaps */
    if (k2settings->dst_break_pages==3)
        added_region.allow_vertical_breaks=0;
    /* If user specifies no figures, need to be able to parse vertically */
    /*
    if (k2settings->text_only)
        added_region.allow_vertical_breaks=1;
    */
    /* Don't know region justification status yet.  Use user settings. */
    added_region.justification_flags=0x8f; 
    added_region.rowbase_delta=-1;
    /* Use dynamic aperture and remove small rows */
    /* v2.36--don't trim if blank */
    trim_mode=k2settings_trim_mode(k2settings);
    rblank=bmpregion_is_blank(region,k2settings);
    if (trim_mode && rblank)
        k2settings->src_trim=0;
    bmpregion_find_textrows(region,k2settings,1,1,k2settings->join_figure_captions);
    if (trim_mode && rblank)
        k2settings->src_trim=1;
    textrows=&region->textrows;
    textrow=textrows->textrow;
    n=textrows->n;
#if (WILLUSDEBUGX & 0x40000)
printf("text row count=%d\n",n);
#endif
    willus_dmem_alloc_warn(35,(void **)&firstrow,n*2*sizeof(int),funcname,10);
    lastrow=&firstrow[n];
    if (notes)
        {
        bmpregion_find_textrows(notes,k2settings,1,1,k2settings->join_figure_captions);
        notesrows=&notes->textrows;
        willus_dmem_alloc_warn(43,(void **)&nfirstrow,notesrows->n*2*sizeof(int),funcname,10);
        nlastrow=&nfirstrow[notesrows->n];
#if (WILLUSDEBUGX & 0x40000)
k2printf("    notes row count=%d\n",notesrows->n);
#endif
        }
    else
        notesrows=NULL;
    /* Should there be a check for zero text rows here? */
    /* Don't think it breaks anything to let it go.  -- 6-11-12 */
#if (WILLUSDEBUGX & 1)
printf("Checking region centering...\n");
#endif
#if (WILLUSDEBUGX & 102)
// textrows_echo(&region->textrows,"rows");
#endif
    region_is_centered=bmpregion_is_centered(region,k2settings,0,n-1);
    if (notes)
        if (notesrows->n>0)
            notes_are_centered=bmpregion_is_centered(notes,k2settings,0,notesrows->n-1);
        else
            notes_are_centered=0;
    else
        notes_are_centered=0;
#if (WILLUSDEBUGX & 2)
textrows_echo(&region->textrows,"rows");
#endif

    /* Mark notes region */
    /*
    if (notes)
        mark_source_page(k2settings,masterinfo,notes,4,0xf);
    */
    /* Mark main region with red, numbered box */
#if (WILLUSDEBUGX & 1)
printf("Marking source page...\n");
#endif
    mark_source_page(k2settings,masterinfo,region,1,0xf);
    if (k2settings->debug)
        {
        if (!added_region.allow_text_wrapping)
            k2printf("@bmpregion_vertically_break (no break) (%d,%d) - (%d,%d) (scale=%g)\n",
                region->c1,region->r1,region->c2,region->r2,added_region.force_scale);
        else
            k2printf("@bmpregion_vertically_break (allow break) (%d,%d) - (%d,%d) (scale=%g)\n",
                region->c1,region->r1,region->c2,region->r2,added_region.force_scale);
        }
#if (WILLUSDEBUGX & 1)
printf("Tagging blank rows and columns...\n");
#endif
    /*
    ** Tag blank rows and columns
    */
    notesgap = -1.;
    if (k2settings->vertical_break_threshold<0. || n < 6)
        biggap = -1.;
    else
        {
        int gap_median;

#ifdef WILLUSDEBUG
for (i=0;i<n;i++)
k2printf("    gap[%d]=%d\n",i,textrow[i].gap);
#endif
        textrows_sort_by_gap(textrows);
        gap_median = textrow[n/2].gap;
#ifdef WILLUSDEBUG
k2printf("    median=%d\n",gap_median);
#endif
        biggap = gap_median*k2settings->vertical_break_threshold;
        textrows_sort_by_row_position(textrows);
        if (notes && notesrows->n>0) /* v2.32 bug fix--check that n>0 */
            {
            int maxgap;

            /* This could probably be smarter */
            textrows_sort_by_gap(notesrows);
            gap_median = notesrows->textrow[notesrows->n/2].gap;
            notesgap = gap_median*k2settings->vertical_break_threshold;
            maxgap=(int)(0.25*region->dpi+0.5);
            if (notesgap > maxgap)
                notesgap = maxgap;
            textrows_sort_by_row_position(notesrows);
            }
        }
#ifdef WILLUSDEBUG
k2printf("    biggap=%d\n",biggap);
#endif

    /*
    ** Determine gap between regions
    */
    region_width_inches = (double)(region->c2-region->c1+1)/region->dpi;
    region_height_inches = (double)(region->r2-region->r1+1)/region->dpi;
    /* If user wants a gap between pages--do that */
    if (k2settings->dst_break_pages<-1 && source_page>0 && source_page != last_source_page)
        {
        masterinfo->mandatory_region_gap=1;
        masterinfo->page_region_gap_in=(-1-k2settings->dst_break_pages)/1000.;
        }
    else
        {
        double gap_in;

        /* First region on the source page? */
        if (source_page != last_source_page)
            {
            double margins_inches[4];

            masterinfo_get_margins(k2settings,margins_inches,&k2settings->srccropmargins,
                                   masterinfo,region);
            gap_in = (double)region->r1/k2settings->src_dpi - margins_inches[1];
            if (last_source_page>=0)
                gap_in += (double)(last_page_height-last_region_r2)/k2settings->src_dpi
                            - margins_inches[3];
            }
        else
            {
            gap_in = (double)(region->r1 - last_region_r2 - 1)/k2settings->src_dpi;
            if (gap_in < 0.)
                gap_in = 0.25;
            }
        masterinfo->page_region_gap_in = gap_in;

        /* Got the gap--now determine whether it should be mandatory */
        if (different_widths(last_region_width_inches,region_width_inches)
               || ncols != last_ncols)
            masterinfo->mandatory_region_gap=1;
        else
            masterinfo->mandatory_region_gap=0;
        }
            
    /*
    ** Done determining region gap--store data about this region for next comparison
    */
    last_ncols = ncols;
    last_region_width_inches = region_width_inches;
    last_source_page=source_page;
    last_region_r2=region->r2;
    last_page_height=region->bmp->height;


/*
k2printf("force_scale=%g, rwi = %g, rwi/mrwi = %g, rhi = %g\n",
added_region.force_scale,
region_width_inches,
region_width_inches / k2settings->max_region_width_inches,
region_height_inches);
*/
#if (WILLUSDEBUGX & 1)
printf("Added_region.force_scale = %g\n",added_region.force_scale);
#endif
    if (added_region.force_scale < -1.5 && region_width_inches > MIN_REGION_WIDTH_INCHES
                         && region_width_inches/k2settings->max_region_width_inches < 1.25
                         && region_height_inches > 0.5)
        {
        revert=1;
        added_region.force_scale = -1.0;
        k2pdfopt_settings_fit_column_to_screen(k2settings,region_width_inches);
        // trim_left_and_right = 0;
        added_region.allow_text_wrapping = 0;
        }
    else
        revert=0;
#if (WILLUSDEBUGX & 0x201)
k2printf("Entering vert region loop, %d regions.\n",n);
{
int i;
for (i=0;i<n;i++)
k2printf("    Region %d:  r1=%d, r2=%d, gapblank=%d\n",i,textrow[i].r1,textrow[i].r2,textrow[i].gapblank);
}
#endif
    /* Un-trim top and bottom region if necessary */
    /* v2.36--also factor in blank region */
    if (((trim_mode && rblank) || !k2settings->src_trim) && n>0)
        {
        textrow[0].r1=region->r1;
        textrow[n-1].r2=region->r2;
        }

    textrows_group(textrows,biggap,firstrow,lastrow,&ng,k2settings->text_only);
#if (WILLUSDEBUGX & 0x40000)
for (i=0;i<ng;i++)
printf("Main row group %2d:  %3d - %3d out of %3d\n",i+1,firstrow[i],lastrow[i],textrows->n);
#endif
    nng=0; /* Avoid compiler warning */
    if (notes)
        {
        textrows_group(notesrows,notesgap,nfirstrow,nlastrow,&nng,k2settings->text_only);
#if (WILLUSDEBUGX & 0x40000)
for (i=0;i<nng;i++)
printf("Notes row group %2d:  %3d - %3d out of %3d (r1=%d)\n",i+1,nfirstrow[i],nlastrow[i],notesrows->n,notesrows->textrow[nfirstrow[i]].r1);
#endif
        }
    /* Add the regions (broken vertically) */
    added_region.caller_id=1;
    /* v2.36--adjust trim flags if region is blank and we're in trim mode with -bp */
    if (trim_mode && rblank)
        added_region.trim_flags=0x80;
    else
        added_region.trim_flags=k2settings->src_trim ? 0xf : 0x80;
    added_region.maps_to_source = 1;
    for (ni=i=0;i<ng;i++)
        {
#if (WILLUSDEBUGX & 0x200)
k2printf("    Group %d (firstrow=%d, lastrow=%d, textrows->n=%d)\n",i,firstrow[i],lastrow[i],textrows->n);
#endif
        int row0;

        row0=firstrow[i];
        /*
        ** Before adding this region, see if there are notes above it or within it
        */
        if (notes)
            {
            for (;ni<nng;ni++)
                {
                int row00;

#if (WILLUSDEBUGX & 0x40000)
printf("ni=%d\n",ni);
#endif
                /*
                ** Is the next note region within this main text region?
                ** If no, break out of loop
                */
                if (notesrows->textrow[nfirstrow[ni]].r1 >= textrow[lastrow[i]].r2)
                    break;
                /* Process this notes region--find where to insert it */
                for (row00=row0;row0<=lastrow[i];row0++)
                    {
#if (WILLUSDEBUGX & 0x40000)
printf("    tr[%d].r2=%d, nr.rw[%d]=%d\n",row0,textrow[row0].r2,ni,notesrows->textrow[nfirstrow[ni]].r1);
#endif
                    if (textrow[row0].r2 > notesrows->textrow[nfirstrow[ni]].r1)
                        break;
                    }
#if (WILLUSDEBUGX & 0x40000)
printf("row0=%d\n",row0);
#endif
                /* Add main text rows: row00 to row0-1 */
                if (row0>row00)
                    {
                    added_region.region=region;
                    added_region.firstrow=row00;
                    added_region.lastrow=row0-1;
                    added_region.notes=0;
                    added_region.count=i;
                    added_region.region_is_centered=region_is_centered;
                    bmpregion_add_textrows(&added_region,k2settings,masterinfo);
                    }
                /* Add notes region */
                added_region.region=notes;
                added_region.firstrow=nfirstrow[ni];
                added_region.lastrow=nlastrow[ni];
                added_region.notes=1;
                added_region.count=0;
                added_region.region_is_centered=notes_are_centered;
                bmpregion_add_textrows(&added_region,k2settings,masterinfo);
                }
            }
        if (row0<=lastrow[i])
            {
            added_region.region=region;
            added_region.firstrow=row0;
            added_region.lastrow=lastrow[i];
            added_region.notes=0;
            added_region.count=i;
            added_region.region_is_centered=region_is_centered;
            bmpregion_add_textrows(&added_region,k2settings,masterinfo);
            }
        if (k2settings->dst_break_pages==3)
            masterinfo_flush(masterinfo,k2settings);
        }
    /* Finish notes regions */
    if (notes)
        for (;ni<nng;ni++)
            {
            added_region.region=notes;
            added_region.firstrow=nfirstrow[ni];
            added_region.lastrow=nlastrow[ni];
            added_region.notes=1;
            added_region.count=0;
            added_region.region_is_centered=notes_are_centered;
            bmpregion_add_textrows(&added_region,k2settings,masterinfo);
            }
    if (k2settings->dst_break_pages==4)
        masterinfo_flush(masterinfo,k2settings);
    if (revert)
        k2pdfopt_settings_restore_output_dpi(k2settings);
    if (notes)
        willus_dmem_free(43,(double **)&nfirstrow,funcname);
    willus_dmem_free(35,(double **)&firstrow,funcname);
    }


static void textrows_group(TEXTROWS *textrows,int biggap_pixels,int *firstrow,int *lastrow,
                           int *ngroups,int text_only)

    {
    int i,i1;

    (*ngroups)=0;
/*
for (i=0;i<textrows->n;i++)
printf("  tr[%2d].type=%d\n",i,textrows->textrow[i].type);
*/
    for (i1=i=0;i1<textrows->n;i++)
        {
        int i2;
 
        i2 = i<textrows->n ? i : textrows->n-1;
        if (i>=textrows->n || (biggap_pixels>0 && textrows->textrow[i2].gapblank>=biggap_pixels)
                           || (text_only 
                                  && (textrows->textrow[i2].type==REGION_TYPE_FIGURE
                                       || (i2<textrows->n-1 && textrows->textrow[i2+1].type==REGION_TYPE_FIGURE))))
            {
            firstrow[(*ngroups)]=i1;
            lastrow[(*ngroups)]=i2;
            (*ngroups)=(*ngroups)+1;
            i1=i2+1;
            }
        }
    }


static void bmpregion_add_textrows(ADDED_REGION_INFO *added_region,
                                   K2PDFOPT_SETTINGS *k2settings,MASTERINFO *masterinfo)

    {
    BMPREGION *region,_region;
    TEXTROW *textrow;
    int n,j,c1,c2,nc,marking_flags;
    static int lasttype=-1;

#if (WILLUSDEBUGX & 0x40000)
printf("ADDING %s ROWS %d - %d OUT OF %d ...\n",added_region->notes?"NOTES":"MAIN TEXT",added_region->firstrow+1,added_region->lastrow+1,added_region->region->textrows.n);
#endif
    n=added_region->region->textrows.n;
    textrow=added_region->region->textrows.textrow;
    region=&_region;
    bmpregion_init(region);
    bmpregion_copy(region,added_region->region,0);
    region->r1=textrow[added_region->firstrow].r1;
    region->r2=textrow[added_region->lastrow].r2;
    for (j=added_region->firstrow;j<=added_region->lastrow;j++)
        textrows_add_textrow(&region->textrows,&textrow[j]);
    c1=textrow[added_region->firstrow].c1;
    c2=textrow[added_region->firstrow].c2;
    nc=c2-c1+1;
    if (nc<=0)
        nc=1;
    for (j=added_region->firstrow+1;j<=added_region->lastrow;j++)
        {
        if (c1>textrow[j].c1)
            c1=textrow[j].c1;
        if (c2<textrow[j].c2)
            c2=textrow[j].c2;
        }
    marking_flags=(added_region->firstrow==0?0:1)|(added_region->lastrow==n-1?0:2);
    /* Green or Magenta (for notes) */
    mark_source_page(k2settings,masterinfo,region,added_region->notes?4:3,added_region->notes?0xf:marking_flags);
    region->bbox.type=REGION_TYPE_MULTILINE;
    region->bbox.c1=region->c1;
    region->bbox.c2=region->c2;
    region->bbox.r1=region->r1;
    region->bbox.r2=region->r2;
    region->bbox.gap=textrow[added_region->lastrow].gap;
    region->bbox.gapblank=textrow[added_region->lastrow].gapblank;
    region->bbox.rowbase=textrow[added_region->lastrow].rowbase;
    /*
    ** Much simpler decision making about gap now (v2.00, 22 Aug 2013)
    */
    if (lasttype>=0 && added_region->notes != lasttype)
        {
        wrapbmp_flush(masterinfo,k2settings,0);
        masterinfo->mandatory_region_gap=1;
        masterinfo->page_region_gap_in=0.5;
        }
    else if (!added_region->notes && (added_region->count>0  || masterinfo->mandatory_region_gap==1))
        {
        wrapbmp_flush(masterinfo,k2settings,0);
        if (masterinfo->mandatory_region_gap==0)
            {
#if (WILLUSDEBUGX & 0x200)
aprintf(ANSI_RED "mi->mandatory_region_gap changed to 1 by vertically_break." ANSI_NORMAL "\n");
#endif
            masterinfo->mandatory_region_gap=1;
            masterinfo->page_region_gap_in=(double)textrow[added_region->firstrow-1].gapblank
                                            / k2settings->src_dpi;
            }
        }
    lasttype = added_region->notes;
/* printf("Adding %s region...\n",added_region->notes?"NOTES":"MAIN TEXT"); */
    {
    ADDED_REGION_INFO new_added_region;

    new_added_region=(*added_region);
    new_added_region.region=region;
    new_added_region.firstrow=0;
    new_added_region.lastrow=region->textrows.n-1;
#if (WILLUSDEBUGX & 0x200)
aprintf(ANSI_MAGENTA "Adding region.\n" ANSI_NORMAL);
#endif
    bmpregion_add(&new_added_region,k2settings,masterinfo);
    }
    bmpregion_free(region);
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


/*
** A region that needs its line spacing and justification analyzed.
**
** The region may be wider than the max desirable region width.
**
** Input:  region should have textrows already decyphered by 
**         bmpregion_vertically_break.
**
** Uses these parameters from ADDED_REGION_INFO:
**     region
**     allow_text_wrapping
**     force_scale
**     region_is_centered
**
** Calls bmpregion_one_row_wrap_and_add() for each text row from the
** breakinfo structure that is within the region.
**
*/
static void bmpregion_analyze_justification_and_line_spacing(ADDED_REGION_INFO *added_region,
                                     K2PDFOPT_SETTINGS *k2settings,MASTERINFO *masterinfo)

    {
    int i;
    TEXTROWS *textrows;
    MULTILINE_PARAMS *mlp,_mlp;
    BMPREGION *region;

    region=added_region->region;
    mlp=&_mlp;
    multiline_params_init(mlp);
    textrows=&region->textrows;
#if (WILLUSDEBUGX & 7)
printf("@bmpregion_analyze_justification_and_line_spacing, textrows->n=%d\n",textrows->n);
k2printf("    (%d,%d) - (%d,%d)\n",region->c1,region->r1,region->c2,region->r2);
k2printf("    centering = %d\n",added_region->region_is_centered);
#endif
#if (WILLUSDEBUGX & 6)
textrows_echo(textrows,"rows");
#endif
    if (textrows->n<=0)
        return;

    mlp->maxlines=textrows->n;
    multiline_params_alloc(mlp);
    multiline_params_calculate(mlp,region,masterinfo,k2settings,added_region->allow_text_wrapping,
                               added_region->region_is_centered);

#if (WILLUSDEBUGX & 1)
if (!added_region->allow_text_wrapping)
k2printf("Processing text row by row (no wrapping)...\n");
else
k2printf("Processing text row by row (text wrapping on)...\n");
#endif
    wrapbmp_set_maxgap(&masterinfo->wrapbmp,mlp->maxgap);
    /*
    ** Process row by row
    */
    for (i=mlp->i1;i<=mlp->i2;i++)
        {
        TEXTROW *textrow;
        int justflags,trimflags,centered,marking_flags,line_spacing;
        BMPREGION *newregion,_newregion;

        textrow=&textrows->textrow[i];
#if (WILLUSDEBUGX & 1)
k2printf("Analyze justification:  Row " ANSI_YELLOW "%d of %d" ANSI_NORMAL " (wrap=%d)\n",i-mlp->i1+1,mlp->i2-mlp->i1+1,added_region->allow_text_wrapping);
k2printf("    r1=%4d, r2=%4d\n",textrow->r1,textrow->r2);
#endif
        newregion=&_newregion;
        bmpregion_init(newregion);
        bmpregion_copy(newregion,region,0);
        newregion->r1=textrow->r1;
        newregion->r2=textrow->r2;
        newregion->bbox=(*textrow);
        if (newregion->bbox.type != REGION_TYPE_FIGURE)
            newregion->bbox.type=REGION_TYPE_TEXTLINE;
        /* newregion->bbox.type=REGION_TYPE_TEXTLINE; */

        line_spacing = get_line_spacing_pixels(textrow,i>mlp->i1?&textrow[-1]:NULL,mlp,k2settings,
                                               added_region->allow_text_wrapping);
#if (WILLUSDEBUGX & 1)
k2printf("    linespacing=%3d\n",line_spacing);
#endif

        /* The |3 tells it to use the user settings for left/right/center */
        justflags = mlp->just[i-mlp->i1]|0x3;
        centered=((justflags&0xc)==4);
#if (WILLUSDEBUGX & 1)
k2printf("    justflags[%d]=0x%2X, centered=%d, indented=%d\n",i-mlp->i1,justflags,centered,mlp->indented[i-mlp->i1]);
#endif
        if (added_region->allow_text_wrapping)
            {
            /* If this line is indented or if the justification has changed, */
            /* then start a new line.                                        */
            if (centered || mlp->indented[i-mlp->i1] || (i>mlp->i1 && (mlp->just[i-mlp->i1]&0xc)!=(mlp->just[i-mlp->i1-1]&0xc)))
{
#ifdef WILLUSDEBUG
k2printf("wrapflush4\n");
#endif
                wrapbmp_flush(masterinfo,k2settings,0);
}
#ifdef WILLUSDEBUG
k2printf("    c1=%d, c2=%d\n",newregion->c1,newregion->c2);
#endif
            marking_flags=0xc|(i==mlp->i1?0:1)|(i==mlp->i2?0:2);
            /* Figures can't be wrapped */
            if (textrow->type == REGION_TYPE_FIGURE)
                {
                ADDED_REGION_INFO new_added_region;
/*
printf("DON'T WRAP (row %d of %d) %d x %d region\n",i+1,mlp->i2+1,newregion->c2-newregion->c1+1,newregion->r2-newregion->r1+1);
*/
#ifdef WILLUSDEBUG
k2printf("wrapflush6\n");
#endif
                wrapbmp_flush(masterinfo,k2settings,0);
#if (WILLUSDEBUGX & 0x200)
printf("    Adding it atomically...\n");
#endif
                new_added_region.region=newregion;
                new_added_region.maps_to_source = added_region->maps_to_source;
                new_added_region.firstrow=0;
                new_added_region.lastrow=newregion->textrows.n-1;
                new_added_region.allow_text_wrapping=0;
                new_added_region.trim_flags=0xf;
                new_added_region.allow_vertical_breaks=0;
                new_added_region.force_scale=-1.0;
                new_added_region.justification_flags=0;
                new_added_region.caller_id=2;
                /* new_added_region.mark_flags=0xf; */
                new_added_region.rowbase_delta = textrow[i].r2-textrow[i].rowbase;
                new_added_region.region_is_centered = -1;
                new_added_region.notes=0;
                new_added_region.count=0;
                bmpregion_add(&new_added_region,k2settings,masterinfo);
                /*
                if (i<mlp->i2)
                    k2settings->gap_override_internal=textrow[i].gapblank;
                */
                }
            else
                bmpregion_one_row_wrap_and_add(newregion,k2settings,masterinfo,
                                           justflags,line_spacing,(int)(mlp->mean_row_gap+.5),
                                           marking_flags,mlp->indented[i-mlp->i1]);
            if (centered || mlp->short_line[i-mlp->i1])
{
#ifdef WILLUSDEBUG
k2printf("wrapflush5\n");
#endif
                wrapbmp_flush(masterinfo,k2settings,0);
}
            }
        else /* !allow_text_wrapping */
            {
            ADDED_REGION_INFO new_added_region;
#ifdef WILLUSDEBUG
k2printf("wrapflush5a\n");
#endif
            /* No wrapping allowed:  process whole line as one region */
            wrapbmp_flush(masterinfo,k2settings,0);
            /* If default justifications, ignore all analysis and just center it. */
            if (k2settings->dst_justify<0 && k2settings->dst_fulljustify<0)
                {
                newregion->c1 = newregion->bbox.c1 = region->c1;
                newregion->c2 = newregion->bbox.c2 = region->c2;
                justflags=0xad; /* Force centered region, no justification */
                trimflags=0x80;
                }
            else
                trimflags=0;
            /* textrow->rowheight = line_spacing; */
            newregion->bbox.rowheight = line_spacing;

            /* No wrapping:  text wrap, trim flags, vert breaks, fscale, just */
            new_added_region.region=newregion;
            new_added_region.maps_to_source=added_region->maps_to_source;
            new_added_region.firstrow=0;
            new_added_region.lastrow=newregion->textrows.n-1;
            new_added_region.allow_text_wrapping=0;
            new_added_region.trim_flags=trimflags;
            new_added_region.allow_vertical_breaks=0;
            new_added_region.force_scale=added_region->force_scale;
            new_added_region.justification_flags=justflags;
            new_added_region.caller_id=5;
            /* new_added_region.mark_flags=0xf; */
            new_added_region.rowbase_delta = textrow->r2-textrow->rowbase;
            new_added_region.region_is_centered = -1;
            new_added_region.notes=0;
            new_added_region.count=0;
            bmpregion_add(&new_added_region,k2settings,masterinfo);
            }
        bmpregion_free(newregion);
        }
    
    multiline_params_free(mlp);
#ifdef WILLUSDEBUG
k2printf("Done wrap_and_add.\n");
#endif
    }


/*
** Determine median value for a multi-line region
**
*/
static void multiline_params_calculate(MULTILINE_PARAMS *mlp,BMPREGION *region,
                                       MASTERINFO *masterinfo,K2PDFOPT_SETTINGS *k2settings,
                                       int allow_text_wrapping,int region_is_centered)

    {
    int i,nls,nch,textheight,ragged_right;
    double *id,*c1,*c2,*ch,*lch,*ls,*h5050;
    /*
    ** Chrox KOReader fix implemented from patch:
    ** https://github.com/koreader/koreader/issues/305
    ** v2.13
    */
    double *indent;
    double median_indent;

    static char *funcname="multiline_params_calculate";
    TEXTROWS *textrows;

    textrows=&region->textrows;
#if (WILLUSDEBUGX & 7)
printf("@multiline_params_calculate, number of textrows=%d\n",textrows->n);
k2printf("    (%d,%d) - (%d,%d)\n",region->c1,region->r1,region->c2,region->r2);
k2printf("    centering = %d\n",region_is_centered);
#endif
#if (WILLUSDEBUGX & 6)
// textrows_echo(textrows,"rows");
#endif

    if (textrows->n<=0)
        return;

    /* Locate the vertical part indices in the textrows structure */
    textrows_sort_by_row_position(textrows);
    for (i=0;i<textrows->n;i++)
        {
        TEXTROW *textrow;
        textrow=&textrows->textrow[i];
        if ((textrow->r1+textrow->r2)/2 >= region->r1)
            break;
        }
    if (i>=textrows->n)
        return;
    mlp->i1=i;
    for (;i<textrows->n;i++)
        {
        TEXTROW *textrow;
        textrow=&textrows->textrow[i];
        if ((textrow->r1+textrow->r2)/2 > region->r2)
            break;
        }
    mlp->i2=i-1;
    if (mlp->i2<mlp->i1)
        return;
    mlp->nlines=mlp->i2-mlp->i1+1;
#if (WILLUSDEBUGX & 1)
k2printf("    i1=%d, i2=%d, mlp->nlines=%d\n",mlp->i1,mlp->i2,mlp->nlines);
#endif

    /*
    ** Allocate arrays for determining median values
    */
    willus_dmem_alloc_warn(13,(void **)&c1,sizeof(double)*8*mlp->nlines,funcname,10);
    c2=&c1[mlp->nlines];
    ch=&c2[mlp->nlines];
    lch=&ch[mlp->nlines];
    ls=&lch[mlp->nlines];
    id=&ls[mlp->nlines];
    h5050=&id[mlp->nlines];
    indent=&h5050[mlp->nlines];
    for (i=0;i<mlp->nlines;i++)
        id[i]=i;

    /* Find baselines / font size */
    mlp->maxgap=-1;
    for (nch=nls=0,i=mlp->i1;i<=mlp->i2;i++)
        {
        TEXTROW *textrow;
        int marking_flags;

        textrow=&textrows->textrow[i];
        c1[i-mlp->i1]=(double)textrow->c1;
        c2[i-mlp->i1]=(double)textrow->c2;
        indent[i-mlp->i1]=k2settings->src_left_to_right ? c1[i-mlp->i1]-region->c1
                                                        : region->c2-c2[i-mlp->i1];
        if (i<mlp->i2 && mlp->maxgap < textrow->gap)
            {
            mlp->maxgap = textrow->gap;
            if (mlp->maxgap < 2)
                mlp->maxgap=2;
            }
        if (textrow->type!=REGION_TYPE_FIGURE)
            {
#if (WILLUSDEBUGX & 1)
printf("assigned lcheight\n");
#endif
            ls[nls++]=textrow->rowheight;
            /* v2.00 use all three */
            ch[nch] = textrow->capheight;
            lch[nch] = textrow->lcheight;
            h5050[nch] = textrow->h5050;
            nch++;
            }

        /* Mark region w/gray, mark rowbase also */
        marking_flags=(i==mlp->i1?0:1)|(i==mlp->i2?0:2);
        if (i<mlp->i2 || textrow->r2-textrow->rowbase>1)
            marking_flags |= 0x10;
        textrow_mark_source(textrow,region,masterinfo,k2settings,marking_flags);
#if (WILLUSDEBUGX & 1)
k2printf("   Row %2d: (%4d,%4d) - (%4d,%4d) rowbase=%4d, ch=%d, lch=%d, h5050=%d, rh=%d, nch=%d\n",i-mlp->i1+1,textrow->c1,textrow->r1,textrow->c2,textrow->r2,textrow->rowbase,textrow->capheight,textrow->lcheight,textrow->h5050,textrow->rowheight,nch);
#endif
        }
    if (nch<1)
        {
        mlp->median_capheight=textrows->textrow[0].capheight;
        mlp->median_lcheight=textrows->textrow[0].lcheight;
        mlp->median_h5050=textrows->textrow[0].h5050;
        }
    else
        {
        mlp->median_capheight = median_val(ch,nch);
        mlp->median_lcheight = median_val(lch,nch);
        mlp->median_h5050 = median_val(h5050,nch);
        }
    textheight=bmpregion_textheight(region,k2settings,mlp->i1,mlp->i2);

    /*
    ** Chrox KOReader patch, v2.13
    ** https://github.com/koreader/koreader/issues/305
    ** Calibrate indentation
    ** Median indent is used to determine proper wrapping so that the
    ** majority of lines get wrapped properly.
    **
    ** Modified for v2.13 by willus.com so that 60% of lines must be
    ** at median indent value to be used, otherwise returns 0.
    */
    median_indent=calc_median_indent(indent,mlp->nlines,mlp->median_h5050);

    /*
    ** Determine regular line spacing for this region
    */
    {
    int j1,j2;
    double lsmean,lsstdev;

    j1= nls>4 ? nls/4 : 0;
    j2= nls>4 ? 3*nls/4 : nls-1;
    array_mean(&ls[j1],j2-j1+1,&lsmean,&lsstdev);
#if (WILLUSDEBUGX & 1)
printf("    nls=%d, j1=%d, j2=%d, lsmean=%g, lsstdev/lsmean=%g\n",nls,j1,j2,lsmean,lsstdev/lsmean);
#endif
    /* Set median line spacing if it's regular enough */
    if (nls>1 && lsmean>0. && lsstdev/lsmean < 0.15)
        mlp->median_line_spacing = median_val(ls,nls);
    else
        mlp->median_line_spacing = -1.;
    }

#if (WILLUSDEBUGX & 1)
k2printf("   lcheight = %.2f pts = %d pixels\n",(mlp->median_lcheight/region->dpi)*72.,(int)(mlp->median_lcheight+.5));
k2printf("   median_line_spacing = %g\n",mlp->median_line_spacing);
#endif

    mlp->mean_row_gap = mlp->median_line_spacing - textheight;
    if (mlp->mean_row_gap <= 1.)
        mlp->mean_row_gap = 1.;
    mlp->mingap = mlp->mean_row_gap / 4;
    if (mlp->mingap < 1)
        mlp->mingap = 1;

    /*
    ** Determine if we have a ragged right edge
    */
    if (mlp->nlines<3)
        ragged_right=1;
    else
        {
        int flushcount;

        if (k2settings->src_left_to_right)
            {
            for (flushcount=i=0;i<mlp->nlines;i++)
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
            for (flushcount=i=0;i<mlp->nlines;i++)
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
        ragged_right = (flushcount <= mlp->nlines/2);
        }
#if (WILLUSDEBUGX & 1)
k2printf("ragged_right=%d\n",ragged_right);
#endif

    /*
    ** Determine justification flags, indentation status, and short-line status
    ** for each line of text.
    */
    for (i=mlp->i1;i<=mlp->i2;i++)
        {
        double indent1,del;
        double i1f,ilfi,i2f,ilf,ifmin,dif;
        double edge_c1,edge_c2;
        int centered;

        TEXTROW *textrow;
        textrow=&textrows->textrow[i];
        /*
        ** Chrox KOReader fix, v2.13
        ** https://github.com/koreader/koreader/issues/305
        */
        edge_c1 = k2settings->src_left_to_right ? region->c1+median_indent : region->c1;
        edge_c2 = k2settings->src_left_to_right ? region->c2 : region->c2-median_indent;
        
        i1f = (double)(c1[i-mlp->i1]-edge_c1)/(region->c2-region->c1+1);
        i2f = (double)(edge_c2-c2[i-mlp->i1])/(region->c2-region->c1+1);
        ilf = k2settings->src_left_to_right ? i1f : i2f;
        ilfi = ilf*(region->c2-region->c1+1)/region->dpi; /* Indent in inches */
        ifmin = i1f<i2f ? i1f : i2f;
        dif=fabs(i1f-i2f);
        if (ifmin < .01)
            ifmin=0.01;
        if (k2settings->src_left_to_right)
            indent1 = (double)(c1[i-mlp->i1]-edge_c1) / textheight;
        else
            indent1 = (double)(edge_c2-c2[i-mlp->i1]) / textheight;
        if (!region_is_centered)
            {
            mlp->indented[i-mlp->i1]=(indent1 > 0.5 && ilfi < 1.2 && ilf < .25);
            centered= (!mlp->indented[i-mlp->i1] && indent1 > 1.0 && dif/ifmin<0.5);
            }
        else
            {
            centered= (dif<0.1 || dif/ifmin<0.5);
            mlp->indented[i-mlp->i1]=(indent1 > 0.5 && ilfi < 1.2 && ilf < .25 && !centered);
            }
#if (WILLUSDEBUGX & 1)
k2printf("Indent %d:  %d.  indent1=%g, ilf=%g, centered=%d\n",i-mlp->i1+1,mlp->indented[i-mlp->i1],indent1,ilf,centered);
k2printf("    indent1=%g, i1f=%g, i2f=%g\n",indent1,i1f,i2f);
#endif
        if (centered)
            mlp->just[i-mlp->i1] = 4;
        else
            {
            /*
            ** The .01 favors left justification over right justification in
            ** close cases.
            */
            if (k2settings->src_left_to_right)
                mlp->just[i-mlp->i1] = mlp->indented[i-mlp->i1] || (i1f < i2f+.01) ? 0 : 8;
            else
                mlp->just[i-mlp->i1] = mlp->indented[i-mlp->i1] || (i2f < i1f+.01) ? 8 : 0;
            }
        if (k2settings->src_left_to_right)
            del = (double)(edge_c2 - textrow->c2);
        else
            del = (double)(textrow->c1 - edge_c1);
        /* Should we keep wrapping after this line? */
        if (!ragged_right)
            mlp->short_line[i-mlp->i1] = (del/textheight > 0.5);
        else
            mlp->short_line[i-mlp->i1] = (del/(region->c2-region->c1) > 0.25);
        /* If this row is a bigger/smaller row (font) than the next row, don't wrap. */
        if (!mlp->short_line[i-mlp->i1] && i<mlp->i2)
            {
            TEXTROW *t1;
            t1=&textrows->textrow[i+1];
            if ((textrow->h5050 > t1->h5050*1.5 || textrow->h5050*1.5 < t1->h5050)
                  && (i==0 || (i>0 && (textrow->rowheight > t1->rowheight*1.5
                                        || textrow->rowheight*1.5 < t1->rowheight))))
                mlp->short_line[i-mlp->i1] = 1;
            }
        if (!ragged_right)
            mlp->just[i-mlp->i1] |= 0x40;
#if (WILLUSDEBUGX & 1)
k2printf("        just[%d]=0x%02X, shortline[%d]=%d\n",i-mlp->i1,mlp->just[i-mlp->i1],i-mlp->i1,mlp->short_line[i-mlp->i1]);
k2printf("        textrow->c2=%d, region->c2=%d, del=%g, textheight=%d\n",textrow->c2,region->c2,del,textheight);
#endif
        /* If short line, it should still be fully justified if it is wrapped. */
        /*
        if (mlp->short_line[i-mlp->i1])
            mlp->just[i-mlp->i1] = (mlp->just[i-mlp->i1]&0xf)|0x60;
        */
        }
/*        
{
double mean1,mean2,stdev1,stdev2;
array_mean(c1,mlp->nlines,&mean1,&stdev1);
array_mean(c2,mlp->nlines,&mean2,&stdev2);
k2printf("Mean c1, c2 = %g, %g; stddevs = %g, %g\n",mean1,mean2,stdev1,stdev2);
k2printf("textheight = %d, line_spacing = %d\n",textheight,line_spacing);
}
*/
    willus_dmem_free(13,(double **)&c1,funcname);
    }


/*
** If enough lines and most of them are indented, then return the median
** indented value so that those lines will be wrapped.
*/
static double calc_median_indent(double *indent,int n,double median_h5050)

    {
    int n1,n2;
    double median_indent;

    if (n<4)
        return(0.);
    sortd(indent,n);
    median_indent=indent[n/2];
    /* indent value must be ~ consistent for 60% of the lines */
    n1=n/5;
    n2=4*n/5-1;
    if (fabs(median_indent-indent[n1])/median_h5050 < 0.5
         && fabs(median_indent-indent[n2])/median_h5050 < 0.5)
        return(median_indent);
    return(0.);
    }


static void multiline_params_init(MULTILINE_PARAMS *mlp)

    {
    mlp->just=NULL;
    mlp->nlines=0;
    mlp->maxlines=0;
    }


static void multiline_params_alloc(MULTILINE_PARAMS *mlp)

    {
    static char *funcname="multiline_params_alloc";

    willus_dmem_alloc_warn(14,(void **)&mlp->just,sizeof(int)*3*mlp->maxlines,funcname,10);
    mlp->indented=&mlp->just[mlp->maxlines];
    mlp->short_line=&mlp->indented[mlp->maxlines];
    }


static void multiline_params_free(MULTILINE_PARAMS *mlp)

    {
    static char *funcname="multiline_params_free";

    willus_dmem_free(14,(double **)&mlp->just,funcname);
    }


/*
**
** Determine single-spaced line height given the font letter sizes.
**
** For 12 pt font:
**     Single spacing is 14.66 pts (Calibri), 13.82 pts (Times), 13.81 pts (Arial)
**     Size of cap letter is 7.7 pts (Calibri), 8.1 pts (Times), 8.7 pts (Arial)
**     Size of small letter is 5.7 pts (Calibri), 5.6 pts (Times), 6.5 pts (Arial)
** Mean line spacing = 1.15 - 1.22 (~1.16) (x 12 pts)
** Mean cap height = 0.68 (x 12 pts)
** Mean small letter height = 0.49 (x 12 pts)
*/
double line_spacing_from_font_size(double lcheight,double h5050,double capheight)

    {
    return(1.16*1.7*((lcheight+capheight+h5050)/3.));
    }


/*
** Determine appropriate line spacing for this row of text
** tr1 = row of text
** tr2 = next row of text (if not NULL)
** (*line_spacing) gets line spacing in pixels
** median_line_spacing (if > 0) is median line spacing in pixels.
** median_font_size = h5050*2
*/
static int get_line_spacing_pixels(TEXTROW *tr1,TEXTROW *tr2,MULTILINE_PARAMS *mlp,
                                    K2PDFOPT_SETTINGS *k2settings,int allow_text_wrapping)

    {
    double row_line_spacing,row_line_spacing_pixels;
    double median_single_space_pixels;
    double row_single_space_pixels;

#if (WILLUSDEBUGX & 1)
printf("    @get_line_spacing_pixels\n");
#endif
    median_single_space_pixels = line_spacing_from_font_size(mlp->median_lcheight,
                                                    mlp->median_h5050,mlp->median_capheight);
    row_single_space_pixels = line_spacing_from_font_size(tr1->lcheight,tr1->h5050,tr1->capheight);
#if (WILLUSDEBUGX & 1)
printf("        median_single_space_pixels = %g\n",median_single_space_pixels);
printf("        row_single_space_pixels = %g\n",row_single_space_pixels);
#endif
    if (row_single_space_pixels < median_single_space_pixels/4.
            || AGREE_WITHIN_MARGIN(median_single_space_pixels,row_single_space_pixels,20))
        row_single_space_pixels = median_single_space_pixels;
#if (WILLUSDEBUGX & 1)
printf("            ... changed to %g\n",row_single_space_pixels);
printf("        tr1->rowheight = %d\n",tr1->rowheight);
#endif

    /* If wrapping, don't want to go line by line--use the median values for uniformity. */
    if (allow_text_wrapping && mlp->median_line_spacing>0.)
        if (mlp->median_line_spacing > 0.)
            row_line_spacing_pixels = mlp->median_line_spacing;
        else
            /* Default to 1.2 x single spacing as last resort */
            row_line_spacing_pixels = 1.2*median_single_space_pixels;
    else
        {
        if (tr1->type==REGION_TYPE_FIGURE)
            row_line_spacing_pixels = tr2==NULL ? tr1->r2-tr1->r1+1 : tr1->r2-tr2->r2+1;
        else
            row_line_spacing_pixels = tr1->rowheight;
        }
#if (WILLUSDEBUGX & 1)
printf("        row_line_spacing_pixels initially set to %g\n",row_line_spacing_pixels);
#endif
    if (AGREE_WITHIN_MARGIN(row_line_spacing_pixels,mlp->median_line_spacing,10))
        row_line_spacing_pixels = mlp->median_line_spacing;
#if (WILLUSDEBUGX & 1)
printf("        adjusted to %g\n",row_line_spacing_pixels);
#endif

    /*
    ** Convert pixels to normalized by dividing by height of single-spaced line
    */
    row_line_spacing = row_line_spacing_pixels / median_single_space_pixels;
#if (WILLUSDEBUGX & 1)
printf("        normalized to %g\n",row_line_spacing);
#endif

    /* If close to normalized median line spacing, make it match for extra uniformity */
    {
    double median_line_spacing; /* normalized to single-spaced line */

    median_line_spacing = mlp->median_line_spacing / median_single_space_pixels;
    if (AGREE_WITHIN_MARGIN(row_line_spacing,median_line_spacing,10))
        row_line_spacing=median_line_spacing;
    }
#if (WILLUSDEBUGX & 1)
printf("        adjusted to %g\n",row_line_spacing);
#endif

    /* Did we override with user settings? */
    if (k2settings->vertical_line_spacing>0.
            || (k2settings->vertical_line_spacing<0. 
                   && row_line_spacing>fabs(k2settings->vertical_line_spacing)))
        row_line_spacing = fabs(k2settings->vertical_line_spacing);
#if (WILLUSDEBUGX & 1)
printf("        After user settings: %g\n",row_line_spacing);
#endif

    /*
    ** Convert normalized back to pixels
    */
    if (allow_text_wrapping)
        row_line_spacing_pixels = row_line_spacing*median_single_space_pixels;
    else
        row_line_spacing_pixels = row_line_spacing*row_single_space_pixels;
#if (WILLUSDEBUGX & 1)
printf("        Converting back to pixels: %g\n",row_line_spacing_pixels);
#endif

    /* Too small for font?  Then default to 1.2 * single space */
    if (row_line_spacing_pixels/row_single_space_pixels < 0.9)
        {
        row_line_spacing_pixels = row_single_space_pixels;
#if (WILLUSDEBUGX & 1)
printf("        Too small for font.  Adjusted to %g\n",row_line_spacing_pixels);
#endif
        }

    return((int)(row_line_spacing_pixels+.5));
    }


static void textrow_mark_source(TEXTROW *textrow,BMPREGION *region,MASTERINFO *masterinfo,
                                K2PDFOPT_SETTINGS *k2settings,int marking_flags)

    {
    BMPREGION *newregion,_newregion;

    newregion=&_newregion;
    bmpregion_init(newregion);
    bmpregion_copy(newregion,region,0);
    newregion->r1=textrow->r1;
    newregion->r2=textrow->r2;
    newregion->c1=textrow->c1;
    newregion->c2=textrow->c2;
    newregion->bbox=(*textrow);
    newregion->bbox.type=REGION_TYPE_TEXTLINE;
    if (textrow->rat>0.)
        {
        int irat;
        if (textrow->rat > 99.)
            irat=99;
        else
            irat=textrow->rat+.5;
        mark_source_page(k2settings,masterinfo,newregion,100+irat,marking_flags);
        }
    else
        mark_source_page(k2settings,masterinfo,newregion,5,marking_flags);
    bmpregion_free(newregion);
    }


/*
** pi = preserve indentation
** Note:  Checking for unwrappable region must have already been done (new in v1.70).
*/
static void bmpregion_one_row_wrap_and_add(BMPREGION *region,K2PDFOPT_SETTINGS *k2settings,
                                           MASTERINFO *masterinfo,int justflags,int line_spacing,
                                           int mean_row_gap,int marking_flags,int pi)

    {
    int nc,i,i0,gappix;
    BMPREGION *newregion,_newregion;
    WRAPBMP *wrapbmp;
    TEXTWORDS *textwords;
    TEXTWORD *textword;
    int n;

    if (region->textrows.n>0)
        {
        k2printf(ANSI_RED "Internal error in one_row_wrap_and_add, nrows=%d (s/b 0).\n",
                 region->textrows.n);
        k2printf("Please report error.\n");
        exit(20);
        }
    wrapbmp=&masterinfo->wrapbmp;
    newregion=&_newregion;
    bmpregion_init(newregion);
    bmpregion_copy(newregion,region,0);
    bmpregion_trim_margins(newregion,k2settings,0x1f);
    nc=newregion->c2-newregion->c1+1;
    newregion->bbox.type=REGION_TYPE_TEXTLINE;
    if (nc<6)
        {
        bmpregion_free(newregion);
        return;
        }
    bmpregion_one_row_find_textwords(newregion,k2settings,1);
    textwords=&newregion->textrows;
    textword=textwords->textrow;
    n=textwords->n;
    if (pi && n>0)
        {
        if (k2settings->src_left_to_right)
            textword[0].c1=region->c1;
        else
            textword[n-1].c2=region->c2;
        }
    /*
    hs=0.;
    for (i=0;i<n;i++)
        hs += (textword[i].r2-textword[i].r1);
    hs /= n;
    */
    /*
    ** Find appropriate letter height to use for word spacing
    */
    {
    double median_gap;
    textwords_add_word_gaps(NULL,newregion->bbox.lcheight,&median_gap,k2settings->word_spacing);
    gappix = (int)(median_gap*newregion->bbox.lcheight+.5);
    }
#if (WILLUSDEBUGX & 4)
k2printf("Before small gap removal, column breaks:\n");
// textrows_echo(textwords,"words");
#endif
#if (WILLUSDEBUGX & 4)
k2printf("After small gap removal, column breaks:\n");
// textrows_echo(textwords,"words");
#endif
    if (k2settings->show_marked_source)
        for (i=0;i<n;i++)
            {
            BMPREGION xregion;
            bmpregion_init(&xregion);
            bmpregion_copy(&xregion,newregion,0);
            xregion.c1=textword[i].c1;
            xregion.c2=textword[i].c2;
            mark_source_page(k2settings,masterinfo,&xregion,2,marking_flags);
            bmpregion_free(&xregion);
            }
#if (WILLUSDEBUGX & 4)
for (i=0;i<n;i++)
k2printf("    textword[%d] = %d - %d\n",i,textword[i].c1,textword[i].c2);
#endif
    /* Maybe skip gaps < 0.5*median_gap or collect gap/rowheight ratios and skip small gaps */
    /* (Could be thrown off by full-justified articles where some lines have big gaps.)     */
    /* Need do call a separate function that removes these gaps. */
    for (i0=0;i0<n;)
        {
        int i1,i2,toolong,rw,remaining_width_pixels;
        BMPREGION reg;

        toolong=0; /* Avoid compiler warning */
        for (i=i0;i<n;i++)
            {
            int wordgap;

            wordgap = wrapbmp_ends_in_hyphen(wrapbmp) ? 0 : gappix;
            i1=k2settings->src_left_to_right ? i0 : n-1-i;
            i2=k2settings->src_left_to_right ? i : n-1-i0;
            rw=(textword[i2].c2-textword[i1].c1+1);
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
                wrapbmp_flush(masterinfo,k2settings,1);
                i--;
                continue;
                }
            /*
            ** If we're not too long and we're not done yet, add another word.
            */
            if (i < n-1 && !toolong)
                continue;
            /*
            ** Add the regions from i0 to i (or i0 to i-1)
            */
            break;
            }
        if (i>i0 && toolong)
            i--;
        i1=k2settings->src_left_to_right ? i0 : n-1-i;
        i2=k2settings->src_left_to_right ? i : n-1-i0;

        bmpregion_init(&reg);
        bmpregion_copy(&reg,newregion,0);
        reg.c1=textword[i1].c1;
        reg.c2=textword[i2].c2;
#if (WILLUSDEBUGX & 4)
k2printf("    Adding i1=%d to i2=%d\n",i1,i2);
#endif
        /* Trim the word top/bottom */
        bmpregion_trim_margins(&reg,k2settings,0xc);
#if (WILLUSDEBUGX & 0x10)
printf("wrapregion lcheight=%2d (source lch=%2d)\n",reg.bbox.lcheight,region->bbox.lcheight);
#endif
        reg.c1=textword[i1].c1;
        reg.c2=textword[i2].c2;
        if (reg.r1 > reg.bbox.rowbase)
            reg.r1 = reg.bbox.rowbase;
        if (reg.r2 < reg.bbox.rowbase)
            reg.r2 = reg.bbox.rowbase;
        /*
        ** Override values from trim_margins with values determined for entire line
        ** This gives better uniformity to wrapped text.
        */
        reg.bbox.rowheight = line_spacing;
        reg.bbox.rowbase = region->bbox.rowbase;
        reg.bbox.gap = mean_row_gap;
        /* Add it to the existing line queue */
        wrapbmp_add(wrapbmp,&reg,k2settings,masterinfo,gappix,justflags);
        bmpregion_free(&reg);
        if (toolong)
{
#ifdef WILLUSDEBUG
k2printf("wrapflush7\n");
#endif
            wrapbmp_flush(masterinfo,k2settings,1);
}
        i0=i+1;
        }
    bmpregion_free(newregion);
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
