/*
** k2master.c    Functions to handle the main (master) k2pdfopt output bitmap.
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

#include "k2pdfopt.h"

static int masterinfo_detecting_orientation(MASTERINFO *masterinfo,K2PDFOPT_SETTINGS *k2settings,
                                 WILLUSBITMAP *src,WILLUSBITMAP *srcgrey,
                                 char *rotstr,double rot_deg,double *bormean,int pageno);
static int calculate_line_gap(BMPREGION *region,MASTERINFO *masterinfo,
                              K2PDFOPT_SETTINGS *k2settings,int single_passed_textline);
static int masterinfo_pageheight_pixels(MASTERINFO *masterinfo,K2PDFOPT_SETTINGS *k2settings);
#ifdef HAVE_MUPDF_LIB
static void masterinfo_add_cropbox(MASTERINFO *masterinfo,K2PDFOPT_SETTINGS *k2settings,
                                   WILLUSBITMAP *bmp1,double bmpdpi,int rows);
#endif
static void bmp_pad_and_mark(WILLUSBITMAP *dst,WILLUSBITMAP *src,K2PDFOPT_SETTINGS *k2settings,
                             int ltotheight,double bmpdpi,void *ocrwords,int landscape);
static void bmp_fully_justify(WILLUSBITMAP *jbmp,WILLUSBITMAP *src,
                              K2PDFOPT_SETTINGS *k2settings,int jbmpwidth,
                              int whitethresh,int just,int dpi,WRECTMAPS *wrectmaps);
static void wrectmaps_add_gap(WRECTMAPS *wrectmaps,int x0,int dx);
/*
static void find_word_gaps_using_wrectmaps(WRECTMAPS *wrectmaps,int **pgappos,
                                           int **pgapsize,int *png,int left_to_right);
*/
static void find_word_gaps_using_textrow(WILLUSBITMAP *src,K2PDFOPT_SETTINGS *k2settings,
                                         int **pgappos,int **pgapsize,int *png,int whitethresh,
                                         int dpi);
static int masterinfo_break_point(MASTERINFO *masterinfo,K2PDFOPT_SETTINGS *k2settings,int maxsize);
static int masterinfo_break_point_1(MASTERINFO *masterinfo,K2PDFOPT_SETTINGS *k2settings,int maxsize);
static int ocrlayer_bounding_box_inches(MASTERINFO *masterinfo,LINE2D *rect);
static void k2master_rows_color(WILLUSBITMAP *bmp,MASTERINFO *masterinfo,
                                K2PDFOPT_SETTINGS *k2settings);


void masterinfo_init(MASTERINFO *masterinfo,K2PDFOPT_SETTINGS *k2settings)

    {
    extern char *k2pdfopt_version;
    int i;

    /* Init outline / bookmarks */
    masterinfo->outline=NULL;
    masterinfo->output_page_count=0;
    masterinfo->outline_srcpage_completed=-1;
    masterinfo->preview_bitmap=NULL;
    masterinfo->preview_captured=0;
    masterinfo->published_pages=0;
    masterinfo->srcpages = -1;
    masterinfo->nextpage = -1;
    masterinfo->landscape = -1;
    masterinfo->landscape_next = -1;
    masterinfo->wordcount=0;
    masterinfo->rcindex=0;
    masterinfo->debugfolder[0]='\0';
    bmp_init(&masterinfo->bmp);
    masterinfo->bmp.height=masterinfo->bmp.width=0;
    masterinfo->bmp.bpp=k2settings->dst_color ? 24 : 8;
    for (i=0;i<256;i++)
        masterinfo->bmp.red[i]=masterinfo->bmp.blue[i]=masterinfo->bmp.green[i]=i;
#ifdef K2PDFOPT_KINDLEPDFVIEWER
    wrectmaps_init(&masterinfo->rectmaps);
#endif
    wrapbmp_init(&masterinfo->wrapbmp,k2settings->dst_color);
#ifdef HAVE_MUPDF_LIB
    if (k2settings->use_crop_boxes)
        wpdfboxes_init(&masterinfo->pageinfo.boxes);
#endif
#ifndef K2PDFOPT_KINDLEPDFVIEWER
    if (k2settings->debug)
        {
        strcpy(masterinfo->debugfolder,"k2_dst_dir");
        wfile_remove_dir(masterinfo->debugfolder,1);
        wfile_makedir(masterinfo->debugfolder);
        }
    else
#endif
        masterinfo->debugfolder[0]='\0';
    masterinfo->rows=0;
    masterinfo->lastrow.lcheight = -1;
    masterinfo->lastrow.capheight = -1;
    masterinfo->lastrow.h5050 = -1;
    masterinfo->lastrow.rowheight = -1;
    masterinfo->lastrow.gap=0;
    masterinfo->lastrow.gapblank=0;
    masterinfo->lastrow.type=REGION_TYPE_UNDETERMINED;
    masterinfo->nocr=0;
    masterinfo->mandatory_region_gap=0;
    masterinfo->page_region_gap_in=-1.;
    masterinfo->k2pagebreakmarks.n=0;
    sprintf(masterinfo->pageinfo.producer,"K2pdfopt %s",k2pdfopt_version);
    }


void masterinfo_free(MASTERINFO *masterinfo,K2PDFOPT_SETTINGS *k2settings)

    {
#ifdef HAVE_MUPDF_LIB
    if (k2settings->use_crop_boxes)
        wpdfboxes_free(&masterinfo->pageinfo.boxes);
#endif
    wrapbmp_free(&masterinfo->wrapbmp);
    bmp_free(&masterinfo->bmp);
#ifdef K2PDFOPT_KINDLEPDFVIEWER
    wrectmaps_free(&masterinfo->rectmaps);
#endif
    wpdfoutline_free(masterinfo->outline);
    }


void masterinfo_flush(MASTERINFO *masterinfo,K2PDFOPT_SETTINGS *k2settings)

    {
#ifndef K2PDFOPT_KINDLEPDFVIEWER
    masterinfo_publish(masterinfo,k2settings,1);
#endif
    }


/*
**
** Start new source page
**
** Input = "src" bitmap (color or grayscale)
**
** Initializes greyscale bitmap "srcgrey" and BMPREGION "region".
** Sets up vars in masterinfo for new page.
**
*/
int masterinfo_new_source_page_init(MASTERINFO *masterinfo,K2PDFOPT_SETTINGS *k2settings,
                         WILLUSBITMAP *src,WILLUSBITMAP *srcgrey,WILLUSBITMAP *marked,
                         BMPREGION *region,double rot_deg,double *bormean,
                         char *rotstr,int pageno,int nextpage,FILE *out)

    {
    int white;

#if (WILLUSDEBUGX & 1)
printf("@masterinfo_new_source_page(pageno=%d,nextpage=%d,maxpages=%d)\n",pageno,nextpage,masterinfo->srcpages);
#endif
    white=k2settings->src_whitethresh;
    if (pageno==masterinfo->nextpage && masterinfo->landscape_next!=-1)
        masterinfo->landscape = masterinfo->landscape_next;
    else
        masterinfo->landscape = k2pdfopt_settings_landscape(k2settings,pageno,masterinfo->srcpages);
#if (WILLUSDEBUGX & 1)
printf("masterinfo->landscape=%d\n",masterinfo->landscape);
#endif
    if (nextpage>=0)
        masterinfo->landscape_next = k2pdfopt_settings_landscape(k2settings,nextpage,masterinfo->srcpages);
    else
        masterinfo->landscape_next = -1;
    masterinfo->nextpage=nextpage;
    masterinfo->pageinfo.srcpage = pageno;
    /* v2.32 */
    
    /* v2.20:  rotation now used to find text in OCR layer, so always assign srcpage_rot_deg. */
    masterinfo->pageinfo.srcpage_rot_deg=0.;
    masterinfo->pageinfo.srcpage_fine_rot_deg = 0.;
    region->rotdeg=0;
    if (!OR_DETECT(rot_deg) && !OREP_DETECT(k2settings) && rot_deg!=0)
        {
        bmp_rotate_right_angle(src,rot_deg);
        region->rotdeg=rot_deg;
        /* v2.20: always assign */
        masterinfo->pageinfo.srcpage_rot_deg=rot_deg;
        }
    if (bmp_is_grayscale(src))
        bmp_copy(srcgrey,src);
    else
        bmp_convert_to_greyscale_ex(srcgrey,src);
    if (!OR_DETECT(rot_deg) && k2settings_need_color_permanently(k2settings))
        bmp_promote_to_24(src);
    if (k2settings->src_erosion<0)
        k2bmp_erode(src,srcgrey,k2settings);
    bmp_adjust_contrast(src,srcgrey,k2settings,&white);
    if (k2settings->src_erosion>0)
        k2bmp_erode(src,srcgrey,k2settings);
    /* v2.20 -- paint pixels above white threshold white if requested */
    if (k2settings->src_paintwhite)
        bmp_paint_white(srcgrey,src,white);

    /*
    if (k2settings->src_whitethresh>0)
        white=k2settings->src_whitethresh;
    */
    if (masterinfo_detecting_orientation(masterinfo,k2settings,src,srcgrey,rotstr,
                                         rot_deg,bormean,pageno))
        return(0);
    if (k2settings->erase_vertical_lines>0)
        bmp_detect_vertical_lines(srcgrey,src,(double)k2settings->src_dpi,/*0.005,*/0.25,
                        k2settings->min_column_height_inches,k2settings->src_autostraighten,white,
                        k2settings->erase_vertical_lines,
                        k2settings->debug,k2settings->verbose);
    if (k2settings->erase_horizontal_lines>0)
        bmp_detect_horizontal_lines(srcgrey,src,(double)k2settings->src_dpi,/*0.005,*/0.25,
                        k2settings->min_column_height_inches,k2settings->src_autostraighten,white,
                        k2settings->erase_horizontal_lines,
                        k2settings->debug,k2settings->verbose);
    if (k2settings->src_autostraighten > 0.)
        {
        double rot;
        rot=bmp_autostraighten(src,srcgrey,white,k2settings->src_autostraighten,0.1,
                               k2settings->debug,out);
#ifdef HAVE_K2GUI
        if (k2gui_active() && fabs(rot)>1e-4)
            k2printf("\n(Page straightened--rotated cc by %.2f deg.)\n",rot);
#endif
        if (k2settings->use_crop_boxes)
            masterinfo->pageinfo.srcpage_fine_rot_deg = rot;
        }
    if (k2settings->autocrop)
        bmp_autocrop2(srcgrey,masterinfo->autocrop_margins,(double)k2settings->autocrop/1000.);
#ifdef HAVE_LEPTONICA_LIB
    if (k2settings->dewarp && !k2settings->use_crop_boxes)
        {
        WILLUSBITMAP _dwbmp,*dwbmp;

        dwbmp=&_dwbmp;
        bmp_init(dwbmp);
        bmp_copy(dwbmp,srcgrey);
        if (k2settings->debug)
            {
            bmp_write(dwbmp,"dewarp_pre_prep.png",NULL,100);
            aprintf("\n" TTEXT_BOLD);
            wfile_written_info("dewarp_pre_prep.png",stdout);
            aprintf(TTEXT_NORMAL);
            }
        k2bmp_prep_for_dewarp(dwbmp,srcgrey,k2settings->src_dpi/45,white);
        if (k2settings->autocrop)
            k2bmp_apply_autocrop(dwbmp,masterinfo->autocrop_margins);
        if (k2settings->debug)
            {
            bmp_write(dwbmp,"dewarp_image.png",NULL,100);
            aprintf(TTEXT_BOLD);
            wfile_written_info("dewarp_image.png",stdout);
            aprintf(TTEXT_NORMAL);
            }
        wlept_bmp_dewarp(dwbmp,src,srcgrey,white,k2settings->dewarp,
                         k2settings->debug?"k2opt_dewarp_model.pdf":NULL);
        if (k2settings->debug)
            {
            aprintf(TTEXT_BOLD);
            wfile_written_info("k2opt_dewarp_model.pdf",stdout);
            aprintf(TTEXT_NORMAL);
            }
        bmp_free(dwbmp);
        /* Re-do autocrop after de-warp */
        if (k2settings->autocrop)
            bmp_autocrop2(srcgrey,masterinfo->autocrop_margins,(double)k2settings->autocrop/1000.);
        }
#endif /* HAVE_LEPTONICA_LIB */
#if (WILLUSDEBUGX & 0x20000)
printf("k2settings->srccropmargins->units[0]=%d\n",k2settings->srccropmargins.units[0]);
#endif
#if (WILLUSDEBUGX & 0x800000)
printf("00\n");
printf("src=%p (%dx%dx%d)\n",src,src->width,src->height,src->bpp);
printf("srcgrey=%p (%dx%dx%d)\n",srcgrey,srcgrey->width,srcgrey->height,srcgrey->bpp);
#endif
    bmp_clear_outside_crop_border(masterinfo,src,srcgrey,k2settings);
#if (WILLUSDEBUGX & 0x800000)
printf("11\n");
#endif
    /* Find page break marks -- src bitmap must be color if there are page-break marks */
    k2file_look_for_pagebreakmarks(region->k2pagebreakmarks,k2settings,src,srcgrey,k2settings->src_dpi);
#if (WILLUSDEBUGX & 0x800000)
printf("22\n");
#endif
    /* Convert source back to gray scale if not using color output */
    if (!k2settings_need_color_permanently(k2settings))
        bmp_convert_to_greyscale(src);
    region->dpi = k2settings->src_dpi;
    region->r1 = 0;
    region->r2 = srcgrey->height-1;
    region->c1 = 0;
    region->c2 = srcgrey->width-1;
    region->bgcolor = white;
    region->bmp = src;
    region->bmp8 = srcgrey;
    region->pageno = pageno;
    /* Not parsed for rows of text yet */
    textrows_clear(&region->textrows);
    region->bbox.type = REGION_TYPE_UNDETERMINED;
    region->bbox.c1 = region->c1;
    region->bbox.c2 = region->c2;
    region->bbox.r1 = region->r1;
    region->bbox.r2 = region->r2;
    if (k2settings->show_marked_source)
        {
        if (k2settings->dst_color && marked!=NULL)
            {
            bmp_copy(marked,src);
            region->marked=marked;
            }
        else
            region->marked=region->bmp;
        }
    masterinfo->bgcolor=white;
    /* dst_fit_to_page == -2 if gridding */
    masterinfo->fit_to_page = k2settings->dst_fit_to_page;
    /* v2.34: moved k2pdfopt_settings_set_margins_and_devsize() to calling function */
    /*
    k2pdfopt_settings_set_margins_and_devsize(k2settings,region,masterinfo,0);
    */
    return(1);
    }


void masterinfo_add_pagebreakmark(MASTERINFO *masterinfo,int marktype)

    {
    int n;

    n=masterinfo->k2pagebreakmarks.n;
    if (n < MAXK2PAGEBREAKMARKS)
        {
        masterinfo->k2pagebreakmarks.k2pagebreakmark[n].row=masterinfo->rows;
        masterinfo->k2pagebreakmarks.k2pagebreakmark[n].type=marktype;
        masterinfo->k2pagebreakmarks.n++;
        }
    /*
    printf("\nPAGEBREAK MARK ADD:  rows = %d\n\n",masterinfo->rows);
    */
    }


static int masterinfo_detecting_orientation(MASTERINFO *masterinfo,K2PDFOPT_SETTINGS *k2settings,
                                 WILLUSBITMAP *src,WILLUSBITMAP *srcgrey,
                                 char *rotstr,double rot_deg,double *bormean,int pageno)

    {
#if (WILLUSDEBUGX & 0x1000000)
printf("OR_DETECT(%g)=%d\n",rot_deg,OR_DETECT(rot_deg));
#endif
    if (rotstr!=NULL)
        rotstr[0]='\0';
    if (OR_DETECT(rot_deg) || OREP_DETECT(k2settings))
        {
        double bor,rotnow;

        if (k2settings->debug)
            k2printf("Checking orientation of page %d ... ",pageno);
        bor=bmp_orientation(srcgrey);
        if (k2settings->debug)
            k2printf("orientation factor = %g\n",bor);
        if (OR_DETECT(rot_deg))
            {
            if (bormean!=NULL)
                (*bormean) *= bor;
            return(1);
            }
        rotnow=rot_deg;
        if (fabs(rot_deg-270)<.5)
            {
            if (bor>10.)
                {
                if (rotstr!=NULL)
                    sprintf(rotstr,"(custom rotation) ");
                rotnow=0.;
                }
            }
        else if (fabs(rot_deg)<.5)
            {
            if (bor<.1)
                {
                if (rotstr!=NULL)
                    sprintf(rotstr,"(custom rotation) ");
                rotnow=270.;
                }
            }
        if (rotnow!=0)
            {
            bmp_rotate_right_angle(srcgrey,rotnow);
            if (k2settings->dst_color)
                bmp_rotate_right_angle(src,rotnow);
            /* v2.20:  always assign */
            masterinfo->pageinfo.srcpage_rot_deg=rotnow;
            }
        }
    return(0);
    }




/*
**
** Add already-scaled source bmp to "masterinfo" destination bmp.
** Source bmp may be narrower than destination--if so, it may be fully justifed.
** dst = destination bitmap
** src = source bitmap
** dst and src bpp must match!
** All rows of src are applied to masterinfo->bmp starting at row masterinfo->rows
** Full justification is done if requested.
**
*/
void masterinfo_add_bitmap(MASTERINFO *masterinfo,WILLUSBITMAP *src,
                    K2PDFOPT_SETTINGS *k2settings,int npageboxes,
                    int justification_flags,int whitethresh,int nocr,int dpi,
                    WRECTMAPS *wrectmaps,TEXTROW *textrow0)

    {
    WILLUSBITMAP *src1,_src1;
    WILLUSBITMAP *tmp,_tmp;
    BMPREGION region;
    WILLUSBITMAP *gray,_gray;
#ifdef HAVE_OCR_LIB
    OCRWORDS _words,*words;
#endif
    int dw,dw2;
    int i,srcbytespp,srcbytewidth,go_full,gap_start;
    int destwidth,destx0,just,single_passed_textline;
    int dstmar_pixels[4];

    if (src->width<=0 || src->height<=0)
        return;
#if (WILLUSDEBUGX & 32)
k2printf("@masterinfo_add_bitmap.  dst->bpp=%d, src->bpp=%d, src=%d x %d\n",masterinfo->bmp.bpp,src->bpp,src->width,src->height);
#endif
/*
{
static int count=0;
static char filename[MAXFILENAMELEN];

k2printf("    @masterinfo_add_bitmap...\n");
sprintf(filename,"src%05d.png",count++);
bmp_write(src,filename,stdout,100);
}
*/
/*
if (fulljust && k2settings->dst_fulljustify)
k2printf("srcbytespp=%d, srcbytewidth=%d, destwidth=%d, destx0=%d, destbytewidth=%d\n",
srcbytespp,srcbytewidth,destwidth,destx0,dstbytewidth);
*/

    /* Determine what justification to use */
    /* Left? */
    if ((justification_flags&3)==0  /* Mandatory left just */
          || ((justification_flags&3)==3  /* Use user settings */
              && (k2settings->dst_justify==0  
                    || (k2settings->dst_justify<0 && (justification_flags&0xc)==0))))
        just=0;
    else if ((justification_flags&3)==2
          || ((justification_flags&3)==3
              && (k2settings->dst_justify==2
                    || (k2settings->dst_justify<0 && (justification_flags&0xc)==8))))
        just=2;
    else
        just=1;

    /* Full justification? */
    get_dest_margins(dstmar_pixels,k2settings,(double)k2settings->dst_dpi,masterinfo->bmp.width,
                     k2settings->dst_height);
    destwidth=masterinfo->bmp.width-dstmar_pixels[0]-dstmar_pixels[2];
    go_full = (destwidth*nocr > src->width 
               && (((justification_flags&0x30)==0x10)
                   || ((justification_flags&0x30)==0 // Use user settings
                       && (k2settings->dst_fulljustify==1
                            || (k2settings->dst_fulljustify<0 && (justification_flags&0xc0)==0x40)))));

    /* Cannot fully justify if using crop boxes */
    if (npageboxes>0 && wrectmaps==NULL)
        go_full=0;

    /* Put fully justified text into src1 bitmap */
    if (go_full)
        {
        src1=&_src1;
        bmp_init(src1);
        bmp_fully_justify(src1,src,k2settings,nocr*destwidth,whitethresh,just,dpi,wrectmaps);
        }
    else
        src1=src;

#if (WILLUSDEBUGX & 1)
k2printf("@masterinfo_add_bitmap:  jflags=0x%02X just=%d, go_full=%d\n",justification_flags,just,go_full);
k2printf("    destwidth=%d, src->width=%d\n",destwidth,src->width);
#endif

    /*
    ** Create BMPREGION out of new bitmap and parse for OCR if necessary.
    */
    if (src1->bpp==8)
        gray=src1;
    else
        {
        gray=&_gray;
        bmp_init(gray);
        bmp_convert_to_grayscale_ex(gray,src1);
        }
    bmpregion_init(&region);
    region.bgcolor=whitethresh;
    region.c1=0;
    region.c2=gray->width-1;
    region.r1=0;
    region.r2=gray->height-1;
    region.bmp8=gray;
    region.bmp=src1; /* May not be 24-bit! */
    region.dpi=dpi;
    region.wrectmaps=wrectmaps;
    region.bbox=(*textrow0);
    /* v2.50--reset bounding box */
    region.bbox.c1=region.c1;
    region.bbox.r1=region.r1;
    region.bbox.c2=region.c2;
    region.bbox.r2=region.r2;
#if (WILLUSDEBUGX & 512)
printf("Add bitmap to master, scanning region for vertical breaks...\n");
printf("    Region = (%d,%d) - (%d,%d)\n",region.c1,region.r1,region.c2,region.r2);
printf("    BBox = (%d,%d) - (%d,%d)\n",region.bbox.c1,region.bbox.r1,region.bbox.c2,region.bbox.r2);
#endif
    /* Find text rows so we can determine line spacing of region */
    if (textrow0->type==REGION_TYPE_FIGURE || textrow0->type==REGION_TYPE_TEXTLINE)
        {
#if (WILLUSDEBUGX & 512)
printf("    Region is single line or figure.\n");
#endif
        single_passed_textline=1;
        textrows_add_textrow(&region.textrows,&region.bbox);
#if (WILLUSDEBUGX & 512)
textrow_echo(&region.textrows.textrow[0],stdout);
#endif
        }
    else
        {
        /* Possible multi-line region */
        single_passed_textline=0;
        bmpregion_find_textrows(&region,k2settings,0,1,k2settings->join_figure_captions);
        }

#if (WILLUSDEBUGX & 512)
printf("    Found %d text rows.\n",region.textrows.n);
#endif
    /*
    ** Scale lastrow to this bitmap dpi
    */
    if (masterinfo->nocr>0 && masterinfo->nocr != nocr)
        {
        double scale;
        scale=(double)nocr/masterinfo->nocr;
        textrow_scale(&masterinfo->lastrow,scale,scale,region.bmp->width-1,region.bmp->height-1);
        }
    gap_start = calculate_line_gap(&region,masterinfo,k2settings,single_passed_textline);
    /*
    ** Remember settings for next line
    */
    if (region.textrows.n>0)
        {
        masterinfo->lastrow = region.textrows.textrow[region.textrows.n-1];
        masterinfo->nocr = nocr;
        }

    /* Reset new page status */
    masterinfo->mandatory_region_gap = 0;
    masterinfo->page_region_gap_in = -1.;

#if (WILLUSDEBUGX & 512)
printf("gap_start=%d\n\n",gap_start);
#endif
#ifdef HAVE_OCR_LIB
    if (k2settings->dst_ocr)
        {
        /* Run OCR on the bitmap */
        words=&_words;
        ocrwords_init(words);
        k2ocr_ocrwords_fill_in_ex(masterinfo,words,&region,k2settings);
/*
{
int i;
printf("\nwordcount=%d\n",words->n);
for (i=0;i<words->n;i++)
{
int j;
printf("word[%d]='%s'\n",i,words->word[i].text);
printf("   ");
for (j=0;words->word[i].text[j]!='\0';j++)
{
unsigned char ch;
ch=(unsigned char)words->word[i].text[j];
printf(" %02X",ch);
}
printf("\n");
}
}
*/
        }
#endif
/*
    if ((k2settings->dst_fgtype==4 || k2settings->dst_bgtype==4) && region.textrows.n>0)
        k2master_rows_color(masterinfo,&region,k2settings);
*/

    bmpregion_free(&region);
    if (src1->bpp!=8)
        bmp_free(gray);

    /*
    ** Up to this point, the bitmap may have been scaled at higher resolution
    ** so that the OCR would be more accurate.  Now that we are done with
    ** OCR, scale down to the appropriate size to fit onto the master output
    ** bitmap.
    */
    if (nocr>1)
        {
        tmp=&_tmp;
        bmp_init(tmp);
        bmp_integer_resample(tmp,src1,nocr);
        gap_start /= nocr;
#ifdef HAVE_OCR_LIB
        if (k2settings->dst_ocr)
            ocrwords_int_scale(words,nocr);
#endif
        }
    else
        tmp=src1;

/*
k2printf("writing...\n");
ocrwords_box(words,tmp);
bmp_write(tmp,"out.png",stdout,100);
exit(10);
*/
    destx0=dstmar_pixels[0];
    if (just==0)
        dw=destx0;
    else if (just==1)
        dw=destx0+(destwidth-tmp->width)/2;
    else
        dw=destx0+destwidth-tmp->width;
    if (dw<0)
        dw=0;
    /* Add OCR words to destination list */
#ifdef HAVE_OCR_LIB
    if (k2settings->dst_ocr)
        {
/*
#if (WILLUSDEBUGX & 0x10000)
int k;
printf("k2master:  add words to main list\n");
printf("Word list being added:\n");
for (k=0;k<words->n;k++)
printf("%3d. '%s'\n",k,words->word[k].text);
#endif
*/
        ocrwords_offset(words,dw,masterinfo->rows+gap_start);
/*
#if (WILLUSDEBUGX & 0x10000)
printf("After ocrwords_offset:\n");
for (k=0;k<words->n;k++)
printf("%3d. '%s'\n",k,words->word[k].text);
#endif
*/
        ocrwords_concatenate(&k2settings->dst_ocrwords,words);
/*
#if (WILLUSDEBUGX & 0x10000)
printf("After being added:\n");
for (k=0;k<words->n;k++)
printf("%3d. '%s'\n",k,words->word[k].text);
printf("Final list:\n");
for (k=0;k<k2settings->dst_ocrwords.n;k++)
printf("%3d. '%s'\n",k,k2settings->dst_ocrwords.word[k].text);
#endif
*/
        ocrwords_free(words);
        }
#endif

    /*
    ** For now:  set destination position in pageinfo structure as pixel position
    ** relative to top of master bitmap.  scale = the height in pixels on the master bitmap.
    */
#ifdef HAVE_MUPDF_LIB
    if (npageboxes>0)
        {
        WPDFBOX *box;
        int i;

        for (i=0;i<npageboxes;i++)
            {
            box=&masterinfo->pageinfo.boxes.box[masterinfo->pageinfo.boxes.n-(npageboxes-i)];
            /* These values will get adjusted in masterinfo_publish() */
            if (wrectmaps==NULL)
                {
                box->x1 = dw;
                box->y1 = masterinfo->rows+gap_start;
                box->userx = tmp->width;
                box->usery = tmp->height;
                }
            else
                {
                /*
                ** This never finished because I realized I didn't need this feature
                ** after I had mostly implemented it(!).  It should be code to assign
                ** all the x1,y1,userx,usery values in the WPDFBOXes corresponding
                ** to the WRECTMAPS structure.
                */
                box->x1 = dw;
                box->y1 = masterinfo->rows+gap_start;
                box->userx = tmp->width;
                box->usery = tmp->height;
                }
            }
        }
#endif

#ifdef K2PDFOPT_KINDLEPDFVIEWER
    /*
    ** Add for KOReader.
    ** Store wrectmaps to masterinfo structure.
    */
    if (wrectmaps!=NULL)
        {
        int j;

        for (j=0;j<wrectmaps->n;j++)
            {
            WRECTMAP *newrmap;

            newrmap = &wrectmaps->wrectmap[j];
            if (newrmap->coords[2].x > 0 && newrmap->coords[2].y > 0)
                {
                wrectmaps_add_wrectmap(&masterinfo->rectmaps,newrmap);
                masterinfo->rectmaps.wrectmap[masterinfo->rectmaps.n-1].coords[1].y
                    += masterinfo->rows + gap_start;
                }
            }
        }
#endif

    /*
    ** v2.35
    ** If user wants bitmap not to be divided across pages, check
    ** to see if it will fit on the existing output page.  If not,
    ** flush the output.
    */
#if (WILLUSDEBUGX & 1)
printf("dst_fit_to_page=%d\n",k2settings->dst_fit_to_page);
printf("gap_start = %d\n",gap_start);
printf("bmpht = %d\n",tmp->height);
printf("pageheight=%d\n",masterinfo_pageheight_pixels(masterinfo,k2settings));
#endif
    if (k2settings->dst_fit_to_page==-3
           && !masterinfo_fits_on_existing_page(masterinfo,k2settings,gap_start+tmp->height))
{
#if (WILLUSDEBUGX & 1)
printf("==> FLUSHING\n");
#endif
        masterinfo_flush(masterinfo,k2settings);
}
              
    /* Add tmp bitmap to dst */
    srcbytespp = tmp->bpp==24 ? 3 : 1;
    srcbytewidth = tmp->width*srcbytespp;
    dw2=masterinfo->bmp.width-tmp->width-dw;
    dw *= srcbytespp;
    dw2 *= srcbytespp;
    while (masterinfo->rows+tmp->height+gap_start > masterinfo->bmp.height)
        bmp_more_rows(&masterinfo->bmp,1.4,255);
#if (WILLUSDEBUGX & 512)
{
static int count=0;
/*
char filename[MAXFILENAMELEN];
int ht;
ht=masterinfo->bmp.height;
masterinfo->bmp.height=masterinfo->rows;
sprintf(filename,"master%03da.png",count);
if (masterinfo->bmp.height>0)
bmp_write(&masterinfo->bmp,filename,stdout,100);
masterinfo->bmp.height=ht;
sprintf(filename,"master%03db.png",count);
if (tmp->height>0)
bmp_write(tmp,filename,stdout,100);
*/
#endif
    /*
    ** Add gap
    */
    if (gap_start>0)
        {
        unsigned char *pdst;

        pdst=bmp_rowptr_from_top(&masterinfo->bmp,masterinfo->rows);
        memset(pdst,255,bmp_bytewidth(&masterinfo->bmp)*gap_start);
        masterinfo->rows += gap_start;
        }
#if (WILLUSDEBUGX & 32)
printf("tmp=%dx%dx%d, masterinfo->rows=%d, bmp=%dx%dx%d, dw=%d\n",tmp->width,tmp->height,tmp->bpp,masterinfo->rows,masterinfo->bmp.width,masterinfo->bmp.height,masterinfo->bmp.bpp,dw);
printf("dw=%d, srcbytewidth=%d, dw2=%d\n",dw,srcbytewidth,dw2);
#endif
    for (i=0;i<tmp->height;i++,masterinfo->rows++)
        {
        unsigned char *pdst,*psrc;

        psrc=bmp_rowptr_from_top(tmp,i);
        pdst=bmp_rowptr_from_top(&masterinfo->bmp,masterinfo->rows);
        memset(pdst,255,dw);
        pdst += dw;
        memcpy(pdst,psrc,srcbytewidth);
        pdst += srcbytewidth;
        memset(pdst,255,dw2);
        }
#if (WILLUSDEBUGX & 512)
/*
ht=masterinfo->bmp.height;
masterinfo->bmp.height=masterinfo->rows;
sprintf(filename,"master%03dc.png",count);
if (masterinfo->bmp.height>0)
bmp_write(&masterinfo->bmp,filename,stdout,100);
masterinfo->bmp.height=ht;
*/
count++;
}
#endif

    if (nocr>1)
        bmp_free(tmp);
    if (go_full)
        bmp_free(src1);
#if (WILLUSDEBUGX & 32)
printf("masterinfo_add_bitmap done.\n");
#endif
    }


static int calculate_line_gap(BMPREGION *region,MASTERINFO *masterinfo,
                              K2PDFOPT_SETTINGS *k2settings,int single_passed_textline)

    {
    int n,gap_pixels,maxgap_pixels,sourcegap_pixels;
    TEXTROW *textrow,*lastrow;

    n=region->textrows.n;
    if (n<=0)
        return(0);
    textrow=region->textrows.textrow;
    lastrow=&masterinfo->lastrow;

    /*
    ** Calculate line spacings and font sizes to help determine gap
    ** between regions.
    */
    /* Set first row line spacing to -1 if we're not very sure on it */
    if (n <= 1 && (!single_passed_textline || masterinfo->mandatory_region_gap))
        textrow[0].rowheight = -1.;
    if (masterinfo->page_region_gap_in >= 0.)
        sourcegap_pixels = masterinfo->page_region_gap_in*region->dpi;
    else
        sourcegap_pixels = masterinfo->lastrow.gapblank;
    if (sourcegap_pixels < 0)
        sourcegap_pixels = 0;
    maxgap_pixels = (int)(k2settings->max_vertical_gap_inches*region->dpi + 0.5);
#if (WILLUSDEBUGX & 512)
printf("\nregion: %d x %d\n",region->c2-region->c1+1,region->r2-region->r1+1);
if (region->k2pagebreakmarks!=NULL)
aprintf(ANSI_CYAN "\nREGION HAS PAGEBREAKMARKS.\n\n" ANSI_NORMAL);
if (masterinfo->k2pagebreakmarks.n>0)
{
int i;
/* printf("    nmarks = %d\n",masterinfo->k2pagebreakmarks->n); */
for (i=0;i<masterinfo->k2pagebreakmarks.n;i++)
printf("        MI->Mark[%d]: row=%d, type=%d\n",i+1,masterinfo->k2pagebreakmarks.k2pagebreakmark[i].row,masterinfo->k2pagebreakmarks.k2pagebreakmark[i].type);
}
printf("mi->rows=%d / %d\n",masterinfo->rows,masterinfo->bmp.height);
printf("dpi=%d\n",region->dpi);
printf("rowbase1=%d\n",textrow[0].rowbase);
printf("fontsize1[lch,h5050,ch]=%d,%d,%d\n",textrow[0].lcheight,textrow[0].h5050,textrow[0].capheight);
printf("linespacing1=%d\n",textrow[0].rowheight);
printf("lastrow->fontsize[lch,h5050,ch]=%d,%d,%d\n",lastrow->lcheight,lastrow->h5050,lastrow->capheight);
printf("lastrow->linespacing=%d\n",lastrow->rowheight);
printf("Same font = %s\n",textrow_font_size_is_same(lastrow,&textrow[0],20)?"YES":"NO");
printf("Same linespacing = %s\n",textrow_line_spacing_is_same(lastrow,&textrow[0],10)?"YES":"NO");
printf("regiontypen=%d\n",textrow[n-1].type);
printf("lastrow->gap=%d pixels\n",lastrow->gap);
printf("lastrow->gapblank=%d pixels\n",lastrow->gapblank);
printf("lastrow->type=%d\n",lastrow->type);
aprintf(ANSI_YELLOW "mi->mandatory_region_gap=%d" ANSI_NORMAL "\n",masterinfo->mandatory_region_gap);
printf("mi->page_region_gap_in=%.2f\n",masterinfo->page_region_gap_in);
printf("k2settings->max_vert_gap=%g in\n",k2settings->max_vertical_gap_inches);
printf("sourcegap_pixels=%d\n",sourcegap_pixels);
printf("maxgap_pixels=%d\n",maxgap_pixels);
#endif

    /* 
    ** Determine gap to add to master bitmap
    **
    ** mandatory_region_gap==1 forces a specified gap to be added before this region
    **                    unless there is a special override case.
    **
    ** mandatory_region_gap==4 means this is the first bitmap being added for the whole
    **                    document, so we don't need to add a gap in that case, either.
    */
    if (k2settings->dst_fit_to_page==-2)
        gap_pixels = 0;
    else if (masterinfo->mandatory_region_gap==1)
        gap_pixels= k2settings_gap_override(k2settings) ? 0 
                                : masterinfo->page_region_gap_in*region->dpi;
    else if (textrow[0].type==REGION_TYPE_FIGURE || lastrow->type==REGION_TYPE_FIGURE)
        {
        /*
        ** If previously added region was a figure or if the first region we are
        ** adding now is a figure, just use the same spacing that was put
        ** between the figure and the text after it from the source
        ** unless the figure begins a new page, in which case we insert max gap.
        */
        if (masterinfo->mandatory_region_gap==0)
            gap_pixels=sourcegap_pixels;
        else
            gap_pixels=maxgap_pixels;
#if (WILLUSDEBUGX & 512)
printf("One region is figure ... gap_pixels set to %d.\n",gap_pixels);
#endif
        }
    else
        {
        if (textrow_line_spacing_is_same(lastrow,&textrow[0],10)
                || textrow_font_size_is_same(lastrow,&textrow[0],20))
            {
            int ls,already_placed,to_be_placed;

            ls=textrow[0].rowheight;
            if (ls<0.)
                ls=lastrow->rowheight;
            if (ls<0.)
                ls=line_spacing_from_font_size(textrow[0].lcheight,textrow[0].h5050,
                                               textrow[0].capheight);
            /* Gap from rowbase */
            already_placed = (masterinfo->lastrow.r2-masterinfo->lastrow.rowbase+1);
            to_be_placed = textrow[0].rowbase + 1 - region->r1;
            gap_pixels = ls - already_placed - to_be_placed;
#if (WILLUSDEBUGX & 512)
printf("Matches last line ... gap_pixels set to %d - %d - %d = %d.\n",ls,already_placed,to_be_placed,gap_pixels);
#endif
            /*
            ** If the line spacing just changed significantly, don't exceed source gap.
            ** (A lot of times this is a sign that were on the first line of a new
            ** section.)  If you don't do this check, you can end up with first
            ** lines in new sections that, when wrapped, create a gap between lines
            ** that is too large--e.g. k2pdfopt nasa.pdf -p 11
            */
            if (!textrow_line_spacing_is_same(lastrow,&textrow[0],20)
                      && k2settings->vertical_line_spacing < 0. 
                      && gap_pixels > sourcegap_pixels)
                {
                gap_pixels = sourcegap_pixels;
#if (WILLUSDEBUGX & 512)
printf("    ... limited to %d (sourcegap_pixels).\n",gap_pixels);
#endif
                }
            /*
            if (gap_pixels + masterinfo->gap > maxgap_pixels)
                gap_pixels = maxgap_pixels - masterinfo->gap;
            */
            }
        /*
        ** This doesn't work well if figures cause the linespacing on either
        ** side to be very large.
        **
        else if (masterinfo->linespacing > 0. && linespacing1 > 0.)
            {
            int ls;
            ls = masterinfo->linespacing > linespacing1
                        ? masterinfo->linespacing : linespacing1;
            gap_pixels = ls*1.2 - (rowbase1 + 1 - region.r1 + masterinfo->gap);
            }
        */
        else
            {
#if (WILLUSDEBUGX & 512)
printf("Font and line spacing change... assigning source gap (%d).\n",sourcegap_pixels);
#endif
            /* Don't adjust max gap for masterinfo->gap, which is from rowbase */
            gap_pixels=sourcegap_pixels;
            }

        /*
        ** If the starting gap relative to line spacing is significantly
        ** less than zero, then we probably got rowbase wrong and should
        ** stick at least a modest gap in to separate what are probably
        ** not contiguous lines of text.  (v2.00, 24-Aug-2013)
        */
        if (gap_pixels < 0)
            {
            double gapfrac;
            int h;
            if (textrow[0].rowheight<=0)
                h=(region->r2-region->r1+1);
            else
                h=textrow[0].rowheight;
            gapfrac = (double)gap_pixels/h;
            if (gapfrac < -.15)
                gap_pixels = h*.25;
            else
                gap_pixels = 0;
            }
        }

    /*
    ** Do not exceed max user-specified gap between regions.
    */
    if (gap_pixels > maxgap_pixels)
        {
        gap_pixels = maxgap_pixels;
#if (WILLUSDEBUGX & 512)
printf("Gap limited to maxgap (%d).\n",maxgap_pixels);
#endif
        }

#if (WILLUSDEBUGX & 0x200)
aprintf(ANSI_RED "mi->mandatory_region_gap changed to %d by add_bitmap." ANSI_NORMAL "\n",masterinfo->mandatory_region_gap);
#endif
#if (WILLUSDEBUGX & 512)
printf("lastrow->gap now = %d(r2) - %d(rowbase) = %d\n",region->r2,textrow[n-1].rowbase,lastrow->gap);
#endif
    return(gap_pixels);
    }


void masterinfo_remove_top_rows(MASTERINFO *masterinfo,K2PDFOPT_SETTINGS *k2settings,int rows)

    {
    int bw,i,j;

    /* Clear the published page:  move everything "up" by "rows" pixels */
    bw=bmp_bytewidth(&masterinfo->bmp);
    for (i=rows;i<masterinfo->rows;i++)
        {
        unsigned char *psrc,*pdst;
        pdst=bmp_rowptr_from_top(&masterinfo->bmp,i-rows);
        psrc=bmp_rowptr_from_top(&masterinfo->bmp,i);
        memcpy(pdst,psrc,bw);
        }
    masterinfo->rows -= rows;

    /* Adjust page break markers and remove if they are out of range */
    for (i=j=0;i<masterinfo->k2pagebreakmarks.n;i++)
        {
        K2PAGEBREAKMARK *mark;

        mark=&masterinfo->k2pagebreakmarks.k2pagebreakmark[i];
        mark->row -= rows;
        if (mark->row<0 || (mark->row<=0 && mark->type==K2PAGEBREAKMARK_TYPE_BREAKPAGE)
                        || mark->type<0)
            continue;
        if (i!=j)
            masterinfo->k2pagebreakmarks.k2pagebreakmark[j]
               = masterinfo->k2pagebreakmarks.k2pagebreakmark[i];
        j++;
        }
    masterinfo->k2pagebreakmarks.n=j;

    /* Move unused crop box positions by -rows so they track the master bitmap */
#ifdef HAVE_MUPDF_LIB
    if (k2settings->use_crop_boxes)
        for (i=0;i<masterinfo->pageinfo.boxes.n;i++)
            {
            WPDFBOX *box;
            box=&masterinfo->pageinfo.boxes.box[i];
            if (box->dstpage<0)
                box->y1 -= rows;
            }
#endif
#ifdef HAVE_OCR_LIB
    /* Move unused OCR words by -rows so they track the master bitmap */
    if (k2settings->dst_ocr)
        ocrwords_offset(&k2settings->dst_ocrwords,0,-rows);
#endif
    }


int masterinfo_fits_on_existing_page(MASTERINFO *masterinfo,K2PDFOPT_SETTINGS *k2settings,
                                     int bmpheight_pixels)

    {
    int pageheight_pixels;

    pageheight_pixels = masterinfo_pageheight_pixels(masterinfo,k2settings);
    return(masterinfo->rows==0 || (masterinfo->rows + bmpheight_pixels < pageheight_pixels));
    }
    

static int masterinfo_pageheight_pixels(MASTERINFO *masterinfo,K2PDFOPT_SETTINGS *k2settings)

    {
    int dstmar_pixels[4];
    int pheight;

    get_dest_margins(dstmar_pixels,k2settings,(double)k2settings->dst_dpi,masterinfo->bmp.width,
                     k2settings->dst_height);
    pheight=k2settings->dst_height-dstmar_pixels[1]-dstmar_pixels[3];
    if (pheight>k2settings->dst_height)
        pheight=k2settings->dst_height;
    return(pheight);
    }


/*
** Publish pages from the master bitmap by finding break points that will
** fit the device page.
**
** k2settings->dst_width = masterinfo->bmp.width
**
*/
int masterinfo_get_next_output_page(MASTERINFO *masterinfo,K2PDFOPT_SETTINGS *k2settings,
                                    int flushall,WILLUSBITMAP *bmp,double *bmpdpi,
                                    int *size_reduction,void *ocrwords)

    {
    /* bmp1 = viewable contents bitmap (smaller than bmp) */
    WILLUSBITMAP *bmp1,_bmp1;
    int rr,maxsize,r0;
    /* Local DPI, width, height */
    double ldpi;
    int lwidth,lheight,ltotheight;
    int bp,i,skippage,preview;
    int dstmar_pixels[4];

    /*
    ** Set skippage if we are previewing and this is not the preview page.
    ** In this case, we don't need to do excess processing on the page
    ** (sharpening, gamma correct, etc.) since it will never be seen.
    */
    preview = (masterinfo->preview_bitmap!=NULL && k2settings->preview_page!=0);
    skippage = (preview && (k2settings->show_marked_source
                         || abs(k2settings->preview_page)!=masterinfo->published_pages+1));
/*
printf("k2pp=%d, published_pages=%d ==> skippage=%d\n",k2settings->preview_page,masterinfo->published_pages,skippage);
*/

    if (k2settings->debug)
        k2printf("@masterinfo_get_next_output_page(page %d)\n",masterinfo->published_pages);
    if (masterinfo->bmp.width != k2settings->dst_width)
        {
        k2printf("\n\n\a" TTEXT_WARN "!! Internal error, masterinfo->bmp.width=%d != dst_width=%d.\n"
               "Contact author." TTEXT_NORMAL "\n\n",masterinfo->bmp.width,k2settings->dst_width);
        wsys_enter_to_exit(NULL);
        exit(10);
        }
    /* v1.52: Make sure text wrapping is flushed if we are to publish everything. */
    if (flushall)
        wrapbmp_flush(masterinfo,k2settings,0);
    bmp1=&_bmp1;
    bmp_init(bmp1);
    /* dh = viewable height in pixels */
    
    get_dest_margins(dstmar_pixels,k2settings,(double)k2settings->dst_dpi,masterinfo->bmp.width,
                     k2settings->dst_height);
    maxsize=masterinfo_pageheight_pixels(masterinfo,k2settings);
    r0=dstmar_pixels[1];
    if (r0+maxsize > k2settings->dst_height)
        r0=k2settings->dst_height-maxsize;
    rr= flushall ? 0 : maxsize;
    if (k2settings->verbose)
        k2printf("rows=%d, maxsize=%d, rr=%d\n",masterinfo->rows,maxsize,rr);
#if (WILLUSDEBUGX & 0x200)
printf("@next_output_page: rows=%d, maxsize=%d, rr=%d\n",masterinfo->rows,maxsize,rr);
#endif
#if (WILLUSDEBUGX & 64)
k2printf("start:  mi->rows=%d, rr=%d\n",masterinfo->rows,rr);
#endif
    /* Enough pixel rows in dest bitmap?  If so, create an output page */
    if (masterinfo->rows<=0 || masterinfo->rows<rr)
        return(0);

    /* Get a suitable breaking point for the next page */
    bp=masterinfo_break_point(masterinfo,k2settings,maxsize);
    if (k2settings->verbose)
        k2printf("bp: maxsize=%d, bp=%d, r0=%d\n",maxsize,bp,r0);
#if (WILLUSDEBUGX & (64|0x200))
k2printf("bp: maxsize=%d, bp=%d, r0=%d\n",maxsize,bp,r0);
#endif
    bmp1->bpp=masterinfo->bmp.bpp;
    for (i=0;i<256;i++)
        bmp1->red[i]=bmp1->green[i]=bmp1->blue[i]=i;
    // h=bp*k2settings->dst_width/masterinfo->bmp.width;
    /*
    ** k2settings->dst_width = full device width in pixels
    ** k2settings->dst_height = full device height in pixels
    ** maxsize = viewable height in pixels (k2settings->dst_height minus margins)
    ** (swapped if -ls)
    */
    /* If too tall, shrink to fit */
#if (WILLUSDEBUGX & (64|0x200))
k2printf("masterinfo->rows=%d\n",masterinfo->rows);
k2printf("bp=%d, maxsize=%d\n",bp,maxsize);
k2printf("dst_width=%g\n",(double)k2settings->dst_width);
k2printf("dst_height=%g\n",(double)k2settings->dst_height);
#endif
    if (bp>maxsize)
        {
        double devht_in;
        lheight=bp;
        ltotheight=(int)((double)k2settings->dst_height*lheight/maxsize+.5);
        // lwidth=(int)((double)masterinfo->bmp.width*lheight/maxsize+.5);
        lwidth=(int)((double)ltotheight*k2settings->dst_width/k2settings->dst_height+.5);
        devht_in=(double)k2settings->dst_height/k2settings->dst_dpi;
        ldpi=(double)ltotheight/devht_in;
        }
    else
        {
        lheight=maxsize; /* useable height */
        ltotheight=k2settings->dst_height;
        lwidth=masterinfo->bmp.width;
        ldpi=k2settings->dst_dpi;
        }
    (*bmpdpi)=ldpi;
    masterinfo->published_pages++;
    /* We're previewing and this isn't the preview page, so don't process further */
    if (skippage)
        {
        masterinfo_remove_top_rows(masterinfo,k2settings,bp);
        return(bp);
        }
    get_dest_margins(dstmar_pixels,k2settings,(double)ldpi,lwidth,lheight);
    r0=dstmar_pixels[1];
    bmp1->width=lwidth;
    bmp1->height=lheight;
#if (WILLUSDEBUGX & (64|0x200))
k2printf("bmp1 wxh = %d x %d\n",bmp1->width,bmp1->height);
k2printf("mi->published_pages=%d\n",masterinfo->published_pages);
#endif

#ifdef HAVE_MUPDF_LIB
    /* Ignore native PDF output if on preview page */
    if (k2settings->use_crop_boxes && !preview)
        {
        masterinfo_add_cropbox(masterinfo,k2settings,bmp1,ldpi,bp);
        masterinfo_remove_top_rows(masterinfo,k2settings,bp);
        bmp_free(bmp1);
        }     
#endif /* HAVE_MUPDF_LIB */
    /* Create list of OCR'd words on this page and move */
    /* up positions of remaining OCR'd words.           */
#ifdef HAVE_OCR_LIB
    if (k2settings->dst_ocr)
        {
/*
        OCRWORDS *ow;

        ow=(OCRWORDS *)ocrwords;
{
FILE *f;
f=fopen("words2.ep","w");
for (i=0;i<k2settings->dst_ocrwords.n;i++)
{
OCRWORD *w;
w=&k2settings->dst_ocrwords.word[i];
fprintf(f,"%g %g\n",w->x0,w->y0);
fprintf(f,"%g %g\n",w->x0+w->w0,w->y0);
fprintf(f,"%g %g\n",w->x0+w->w0,w->y0+w->h0);
fprintf(f,"%g %g\n",w->x0,w->y0+w->h0);
fprintf(f,"%g %g\n",w->x0,w->y0);
fprintf(f,"//nc\n");
}
fclose(f);
}
printf("Creating ocrwords list (n=%d, o->n=%d)...\n",k2settings->dst_ocrwords.n,ow->n);
exit(10);
*/
        for (i=0;i<k2settings->dst_ocrwords.n;i++)
            if (k2settings->dst_ocrwords.word[i].r 
                        - k2settings->dst_ocrwords.word[i].maxheight 
                        + k2settings->dst_ocrwords.word[i].h/2 < bp)
                {
                if (ocrwords!=NULL)
                    ocrwords_add_word((OCRWORDS *)ocrwords,&k2settings->dst_ocrwords.word[i]);
                ocrwords_remove_words(&k2settings->dst_ocrwords,i,i);
                i--;
                }
        }
#endif
    if (k2settings->use_crop_boxes && !preview)
        return(bp);

    /*
    ** Not using crop boxes from here on, so process output bitmap
    */
    /* Center masterinfo->bmp into bmp1 (horizontally) */
    bmp_alloc(bmp1);
    bmp_fill(bmp1,255,255,255);
    {
    int bpp,w1,bw,bw1;
    bpp=bmp1->bpp==24?3:1;
    w1=(bmp1->width-masterinfo->bmp.width)/2;
    bw=bmp_bytewidth(&masterinfo->bmp);
    bw1=w1*bpp;
    for (i=0;i<bp;i++)
        memcpy(bmp_rowptr_from_top(bmp1,i)+bw1,bmp_rowptr_from_top(&masterinfo->bmp,i),bw);
#ifdef HAVE_OCR_LIB
    if (k2settings->dst_ocr && ocrwords!=NULL)
        ocrwords_offset((OCRWORDS *)ocrwords,w1,0);
#endif
    }

    /* Gamma correct */
    {
    double gamma;
    if (fabs((gamma=k2pdfopt_settings_gamma(k2settings))-1.0)>.001)
        bmp_gamma_correct(bmp1,bmp1,gamma);
    }

    /* Sharpen */
    if (k2settings->dst_sharpen)
        {
        WILLUSBITMAP tmp;
        bmp_init(&tmp);
        bmp_copy(&tmp,bmp1);
        bmp_sharpen(bmp1,&tmp);
        bmp_free(&tmp);
        }

    /* Pad and mark bmp1 -> bmp */
    bmp_pad_and_mark(bmp,bmp1,k2settings,ltotheight,ldpi,ocrwords,masterinfo->landscape);
    bmp_free(bmp1);

    /* Adjust for landscape output if necessary */
    if (masterinfo->landscape)
        {
#ifdef HAVE_OCR_LIB
        /* Rotate OCR'd words list */
        if (k2settings->dst_ocr && ocrwords!=NULL)
            {
            OCRWORDS *ocw;

            ocw=(OCRWORDS *)ocrwords;
            for (i=0;i<ocw->n;i++)
                {
                int cnew,rnew;
                ocw->word[i].rot=90;
                cnew = ocw->word[i].r;
                rnew = bmp->width-1 - ocw->word[i].c;
                ocw->word[i].c = cnew;
                ocw->word[i].r = rnew;
                }
            }
#endif
        bmp_rotate_right_angle(bmp,90);
        }


    /* Inverse -- moved to after pad and mark (was before), v2.22 */
    if (k2settings->dst_negative)
        bmp_invert(bmp);

    /* Fix colors */
/*
    if ((k2settings->dst_fgtype!=4 && k2settings->dst_bgtype!=4)
          && (k2settings->dst_fgtype!=0 || k2settings->dst_bgtype!=0))
*/
    if (k2settings->dst_fgtype!=0 || k2settings->dst_bgtype!=0)
        {
        if (k2settings->dst_fgtype==4 || k2settings->dst_bgtype==4)
            k2master_rows_color(bmp,masterinfo,k2settings);
        else
            bmp_change_colors(bmp,bmp,
                              k2settings->dst_fgtype==4?"":k2settings->dst_fgcolor,
                              k2settings->dst_fgtype,
                              k2settings->dst_bgtype==4?"":k2settings->dst_bgcolor,
                              k2settings->dst_bgtype,
                              0,0,bmp->width-1,bmp->height-1);
        }


    /* Write sequence of OCR'd words if debugging */
#ifndef K2PDFOPT_KINDLEPDFVIEWER
    if (k2settings->debug)
        {
        char basename[32];
        char opbmpfile[512];
        static int filecount=0;

        sprintf(basename,"outpage%05d.%s",filecount+1,k2settings->jpeg_quality>0?"jpg":"png");
        wfile_fullname(opbmpfile,masterinfo->debugfolder,basename);
        bmp_write(bmp,opbmpfile,stdout,k2settings->jpeg_quality<0?100:k2settings->jpeg_quality);
#ifdef HAVE_OCR_LIB
        if (k2settings->dst_ocr && ocrwords!=NULL)
            {
            FILE *f;
            sprintf(basename,"wordlist%05d.txt",filecount+1);
            f=fopen(basename,"w");
            if (f!=NULL)
                {
                OCRWORDS *ocw;

                ocw=(OCRWORDS *)ocrwords;
                for (i=0;i<ocw->n;i++)
                    fprintf(f,"%s\n",ocw->word[i].text);
                fclose(f);
                }
            }
#endif
        filecount++;
        }
#endif

    /* Reduce bitmap depth */
    if (k2settings->dst_bpc==8 || k2settings->jpeg_quality>=0)
        (*size_reduction)=0;
    else if (k2settings->dst_bpc==4)
        (*size_reduction)=1;
    else if (k2settings->dst_bpc==2)
        (*size_reduction)=2;
    else
        (*size_reduction)=3;
    if (k2settings->dst_dither && k2settings->dst_bpc<8 && k2settings->jpeg_quality<0)
        bmp_dither_to_bpc(bmp,k2settings->dst_bpc);
#if (WILLUSDEBUGX & 0x200)
printf("Removing top rows.  Before, rows=%d\n",masterinfo->rows);
#endif
    masterinfo_remove_top_rows(masterinfo,k2settings,bp);
#if (WILLUSDEBUGX & 0x200)
printf("  After, rows=%d\n",masterinfo->rows);
#endif
    return(bp);
    }


/*
** Should only be called once per source page.
** Return 0 if master bitmap should not be flushed.
**        NZ if it should be flushed.
** Based on user settings for page breaks.
** v2.10
**
*/
int masterinfo_should_flush(MASTERINFO *masterinfo,K2PDFOPT_SETTINGS *k2settings)

    {
#if (WILLUSDEBUGX & 0x200)
printf("@masterinfo_should_flush, breakpages=%d\n",k2settings->dst_break_pages);
#endif
    if (k2settings->dst_landscape_pages[0]!='\0' 
           && masterinfo->landscape != masterinfo->landscape_next)
        return(1);
    if (k2settings_gap_override(k2settings))
        return(0);
    if (k2settings->dst_break_pages==0)
        return(0);
    if (k2settings->dst_break_pages>1)
        return(1);
    /* Check list of pages where user has requested a page break */
    if (k2settings->bpl[0]!='\0' && pagelist_includes_page(k2settings->bpl,masterinfo->nextpage,masterinfo->srcpages))
        return(1);
    /* Check outline / bookmarks if available */
    return(wpdfoutline_includes_srcpage(masterinfo->outline,masterinfo->nextpage,1)>0 ? 1 : 0);
    }


/*
** Correctly complete crop boxes that are associated with this output page
**
** box->x1,y1,userx,usery start out as:
**     box->x1,y1 = position of upper-left crop box corner in master
**                  bitmap, in pixels.
**     box->userx,usery = width,height of crop box in pixels
**     Original assignments:
**         box->x1 = dw;
**         box->y1 = masterinfo->rows;
**         box->userx = tmp->width;
**         box->usery = tmp->height;
**
** ... but are changed so that...
**
** box->x1 gets the position of the lower-left point of the crop box
**         (when looking at the contents of the crop box right-side up)
**         on the destination page (when holding the device right-side up).
**         The position is the distance from the left side of the device
**         in points (again, when holding the device right-side up).
**
** box->y1 is the same, but it is the distance from the bottom of the
**         device screen, in points, when holding the device right-side up.
**
** box->scale is the scale factor between source and destination sizes.
**
*/
#ifdef HAVE_MUPDF_LIB
static void masterinfo_add_cropbox(MASTERINFO *masterinfo,K2PDFOPT_SETTINGS *k2settings,
                                   WILLUSBITMAP *bmp1,double bmpdpi,int rows)

    {
    int i,dstpageno,r0,r1,height;
    int dstmar_pixels[4];

    get_dest_margins(dstmar_pixels,k2settings,(double)bmpdpi,bmp1->width,bmp1->height);
    r0=dstmar_pixels[1];
    r1=dstmar_pixels[3];
    /* Width already has user-specified device margins factored in, but height doesn't */
    height=bmp1->height+r0+r1;
    dstpageno = masterinfo->published_pages;
#if (WILLUSDEBUGX & 64)
k2printf("@masterinfo_add_cropbox, nboxes=%d rows=%d\n",masterinfo->pageinfo.boxes.n,rows);
#endif
    for (i=0;i<masterinfo->pageinfo.boxes.n;i++)
        {
        WPDFBOX *box;
        int w1,xd,yd;

        box=&masterinfo->pageinfo.boxes.box[i];
#if (WILLUSDEBUGX & 64)
k2printf("    box[%2d]:  x0pts=%g, y0pts=%g, %g x %g\n",i,box->srcbox.x0_pts,box->srcbox.y0_pts,
box->srcbox.crop_width_pts,box->srcbox.crop_height_pts);
k2printf("              Top = %d, height = %d\n",(int)box->y1,(int)box->usery);
k2printf("              box->dstpage = %d\n",box->dstpage);
k2printf("              box->y1 = %g, rows-.1 = %g\n",box->y1,rows-.1);
#endif
        if (box->dstpage>0)
            continue;
        /* If crop box not on this page, skip it */
        if (box->y1 > rows-.1)
            continue;
#if (WILLUSDEBUGX & 64)
k2printf("    box[%d]:  rows(bp)=%d, ix0=%d, iy0=%d, y1=%g, usery=%g, bmp1->ht=%d\n",i,rows,
(int)(box->srcbox.x0_pts*10.+.5),
(int)((box->srcbox.page_height_pts-box->srcbox.y0_pts)*10.+.5),
box->y1,box->usery,bmp1->height);
#endif
        /* If crop-box is cut off by this page, split it */
        if (box->y1+box->usery > bmp1->height+.1)
            {
            double newheight,dy;
            newheight = box->usery-(rows-box->y1);
#if (WILLUSDEBUGX & 64)
k2printf("    Splitting box.  New height=%d.\n",(int)newheight);
#endif
            if (newheight>0.5)
                {
                wpdfboxes_insert_box(&masterinfo->pageinfo.boxes,box,i);
                /* Next page: adjust source crop box so we only get the part on */
                /* the next page */
                box=&masterinfo->pageinfo.boxes.box[i];
                box->srcbox.crop_height_pts *= (double)newheight/box->usery;
                box->y1 += (box->usery - newheight);
                box->usery = newheight;
                i++;
                /* This page: adjust source crop box so we only get the part on this page */
                box=&masterinfo->pageinfo.boxes.box[i];
                dy = box->srcbox.crop_height_pts;
                box->srcbox.crop_height_pts *= (double)(rows-box->y1)/box->usery;
                dy -= box->srcbox.crop_height_pts;
                box->srcbox.y0_pts += dy;
                box->usery = rows-box->y1;
                }
            }
#if (WILLUSDEBUGX & 64)
k2printf("    Adding box %d (dstpageno=%d).\n",i,dstpageno);
#endif
        box->dstpage = dstpageno;
        w1=(bmp1->width-masterinfo->bmp.width)/2;
/*
printf("w1=%d\n",w1);
printf("bmp1->widthxheight=%dx%d\n",bmp1->width,bmp1->height);
printf("masterinfo->bmp.widthxheight=%dx%d\n",masterinfo->bmp.width,masterinfo->bmp.height);
printf("box->x1,y1=%g,%g\n",box->x1,box->y1);
printf("box->userxxusery=%gx%g\n",box->userx,box->usery);
*/
        // sr=(int)((box->srcrot_deg+765.)/90.);
        /* box->dstrot_deg will be +/-90 if this is a rotated figure */
        /* See k2proc.c -- call to ...add_crop_boxes() */
        {
        int bmpheight_pix,bmpwidth_pix;
        int scalewidth_pix,scaleheight_pix;
        double buserx,busery,devheight_pts,devwidth_pts,scalex,scaley;

        xd=w1+box->x1;
        yd=height-(box->y1+box->usery+r0);
        scaleheight_pix = bmpheight_pix = height;
        scalewidth_pix = bmpwidth_pix = bmp1->width;
        devheight_pts = box->dst_height_pts;
        devwidth_pts = box->dst_width_pts;
        buserx = box->userx;
        busery = box->usery;
        if ((masterinfo->landscape && fabs(box->dstrot_deg)<.1)
             || (!masterinfo->landscape && box->dstrot_deg>89.))
            {
            if (masterinfo->landscape)
                int_swap(bmpheight_pix,bmpwidth_pix);
            if (!masterinfo->landscape)
                {
                int_swap(scalewidth_pix,scaleheight_pix);
                int_swap(buserx,busery);
                }
            double_swap(devheight_pts,devwidth_pts);
            if (masterinfo->landscape)
                {
                int_swap(xd,yd);
                xd = height-xd;
                box->dstrot_deg=90.;
                }
            else
                {
                xd = bmp1->width - xd;
                }
            }
        else if (box->dstrot_deg < -89.) /* rotated figure in landscape */
            {
            box->dstrot_deg=0.;
            int_swap(buserx,busery);
            int_swap(scalewidth_pix,scaleheight_pix);
            int_swap(xd,yd);
            xd=box->y1+r0;
            }
        else
            box->dstrot_deg=0.;
        box->x1 = box->dst_width_pts*xd/bmpwidth_pix;
        box->y1 = box->dst_height_pts*yd/bmpheight_pix;
        scalex = buserx*devwidth_pts/scalewidth_pix/box->srcbox.crop_width_pts;
        scaley = busery*devheight_pts/scaleheight_pix/box->srcbox.crop_height_pts;
        box->scale = scalex < scaley ? scalex : scaley;
/*
printf("AFTER TRANSFORM:\n");
printf("    xd=%d, yd=%d\n",xd,yd);
printf("    userx=%g, usery=%g\n",buserx,busery);
printf("    bmp w,h = %d, %d\n",bmpwidth_pix,bmpheight_pix);
printf("    dev w,h = %g, %g\n",devwidth_pts,devheight_pts);
printf("    dstrot_deg = %g\n",box->dstrot_deg);
printf("    box->x1,y1 = %g,%g\n",box->x1,box->y1);
printf("    box->srcbox=%gx%g\n",box->srcbox.crop_width_pts,box->srcbox.crop_height_pts);
printf("    scalex=buserx*devwidpts/scalewidpix/srcwidpts=%g*%g/%d/%g\n",
buserx,devwidth_pts,scalewidth_pix,box->srcbox.crop_width_pts);
printf("    scaley=busery*devhtpts/scalehtpix/srchtpts=%g*%g/%d/%g\n",
busery,devheight_pts,scaleheight_pix,box->srcbox.crop_height_pts);
printf("    scalex,y = %g,%g\n",scalex,scaley);
printf("    box->scale = %g\n\n",box->scale);
*/
        }
        }
    }
#endif /* HAVE_MUPDF_LIB */


/*
** Give output bitmap appropriate padding and margins (bmp1 -> bmp).
**
** pl, pr, pt, pb are padding specified by user to get correct pixel mapping
**                on a specific device.
**
** r0 = top margin on output device specified by user (dstmargins.box[1])
**
** Left margin on output device (dstmargins.box[0]) was already taken into account
**     in bmp_src_to_dst().
**
*/
static void bmp_pad_and_mark(WILLUSBITMAP *dst,WILLUSBITMAP *src,K2PDFOPT_SETTINGS *k2settings,
                             int ltotheight,double bmpdpi,void *ocrwords,int landscape)

    {
    int i,r,r0,bw,bytespp,pl,pr,pt,pb;
    int dstmar_pixels[4];
/*
printf("Pad:  %d,%d,%d,%d\n",k2settings->pad_left,
k2settings->pad_top,
k2settings->pad_right,
k2settings->pad_bottom);
printf("usecropboxes=%d\n",k2settings->use_crop_boxes);
*/
    get_dest_margins(dstmar_pixels,k2settings,(double)bmpdpi,src->width,src->height);
/*
printf("OM (pixels):  %g,%g,%g,%g\n",
dstmar_pixels[0],
dstmar_pixels[1],
dstmar_pixels[2],
dstmar_pixels[3]);
*/
    r0=dstmar_pixels[1];
    if (landscape)
        {
        pl=k2settings->pad_bottom;
        pr=k2settings->pad_top;
        pt=k2settings->pad_left;
        pb=k2settings->pad_right;
        }
    else
        {
        pb=k2settings->pad_bottom;
        pt=k2settings->pad_top;
        pl=k2settings->pad_left;
        pr=k2settings->pad_right;
        }
    dst->bpp=src->bpp;
    for (i=0;i<256;i++)
        dst->red[i]=dst->green[i]=dst->blue[i]=i;
    dst->width=src->width+pl+pr;
    dst->height=ltotheight+pt+pb;
    bmp_alloc(dst);
    bmp_fill(dst,255,255,255);
    bw=bmp_bytewidth(src);
    bytespp=dst->bpp==8?1:3;
    for (r=0;r<src->height && r+r0+pt<dst->height;r++)
        {
        unsigned char *psrc,*pdst;
        psrc=bmp_rowptr_from_top(src,r);
        pdst=bmp_rowptr_from_top(dst,r+r0+pt)+pl*bytespp;
        memcpy(pdst,psrc,bw);
        }
#ifdef HAVE_OCR_LIB
    if (k2settings->dst_ocr && ocrwords!=NULL)
        ocrwords_offset((OCRWORDS *)ocrwords,pl,r0+pt);
#endif
    /* Can't save grayscale as JPEG yet. */
    if (dst->bpp==8 && k2settings->jpeg_quality>=0)
        bmp_promote_to_24(dst);
    bytespp=dst->bpp==8?1:3;
    /* v2.22:  Fix marking of corners for 24-bit bitmap */
    if (k2settings->mark_corners)
        {
        unsigned char *p;

        if (pt<dst->height)
            {
            p=bmp_rowptr_from_top(dst,pt);
            if (pl<dst->width)
                {
                int k;
                for (k=0;k<bytespp;k++)
                    p[pl*bytespp+k]=0;
                }
            if (pr<dst->width)
                {
                int k;
                for (k=0;k<bytespp;k++)
                    p[(dst->width-1-pr)*bytespp+k]=0;
                }
            }
        if (pb<dst->height)
            {
            p=bmp_rowptr_from_top(dst,dst->height-1-pb);
            if (pl<dst->width)
                {
                int k;
                for (k=0;k<bytespp;k++)
                    p[pl*bytespp+k]=0;
                }
            if (pr<dst->width)
                {
                int k;
                for (k=0;k<bytespp;k++)
                    p[(dst->width-1-pr)*bytespp+k]=0;
                }
            }
        }
    }


/*
** Spread words out in src and put into jbmp at scaling nocr
** In case the text can't be expanded enough,
**     just=0 (left justify), 1 (center), 2 (right justify)
*/
static void bmp_fully_justify(WILLUSBITMAP *jbmp,WILLUSBITMAP *src,
                              K2PDFOPT_SETTINGS *k2settings,int jbmpwidth,
                              int whitethresh,int just,int dpi,WRECTMAPS *wrectmaps)

    {
    int *gappos,*gapsize,*sx0,*dx0;
    int i,srcbytespp,srcbytewidth,jbmpbytewidth,newwidth,destx0,ng;
    static char *funcname="bmp_fully_justify";

/*
{
char filename[MAXFILENAMELEN];
count++;
sprintf(filename,"out%03d.png",count);
bmp_write(src,filename,stdout,100);
}
*/
    srcbytespp=src->bpp==24 ? 3 : 1;

    /* Init/allocate destination bitmap */
    jbmp->width = jbmpwidth;
    jbmp->height = src->height;
    jbmp->bpp = src->bpp;
    if (jbmp->bpp==8)
        for (i=0;i<256;i++)
            jbmp->red[i]=jbmp->green[i]=jbmp->blue[i]=i;
    bmp_alloc(jbmp);

    gappos=NULL;
/*
    if (wrectmaps!=NULL)
        find_word_gaps_using_wrectmaps(wrectmaps,&gappos,&gapsize,&ng,
                                       k2settings->src_left_to_right);
    else
*/
    find_word_gaps_using_textrow(src,k2settings,&gappos,&gapsize,&ng,whitethresh,dpi);

    if (ng>0)
        {
        /* Figure out total pixel expansion */
        newwidth = src->width*1.25;
        if (newwidth > jbmp->width)
            newwidth=jbmp->width;
        }
    else
        newwidth=src->width;

    /* Starting column in destination bitmap */
    if (just==1)
        destx0 = (jbmp->width-newwidth)/2;
    else if (just==2)
        destx0 = (jbmp->width-newwidth);
    else
        destx0 = 0;

    jbmpbytewidth = bmp_bytewidth(jbmp);
    srcbytewidth = bmp_bytewidth(src);

    /* Clear entire fully justified bitmap */
    memset(bmp_rowptr_from_top(jbmp,0),255,jbmpbytewidth*jbmp->height);

    if (wrectmaps!=NULL)
        wrectmaps_sort_horizontally(wrectmaps);

    /* Spread out source pieces to fully justify them */
    willus_dmem_alloc_warn(30,(void *)&dx0,2*sizeof(int)*(ng+1),funcname,10);
    sx0=&dx0[ng+1];
    for (i=0;i<=ng;i++)
        {
        int j,dx;
        unsigned char *pdst,*psrc;

        dx = i<ng ? (i>0 ? gappos[i]-gappos[i-1] : gappos[i]+1)
                  : (i>0 ? src->width-(gappos[i-1]+1) : src->width);
        dx *= srcbytespp;
        sx0[i]= i==0 ? 0 : (gappos[i-1]+1);
        dx0[i]= destx0 + sx0[i] + (i==0 ? 0 : (newwidth-src->width)*i/ng);
        psrc=bmp_rowptr_from_top(src,0)+sx0[i]*srcbytespp;
        pdst=bmp_rowptr_from_top(jbmp,0)+dx0[i]*srcbytespp;
        for (j=0;j<src->height;j++,pdst+=jbmpbytewidth,psrc+=srcbytewidth)
            memcpy(pdst,psrc,dx);
        }
    /* Adjust rectangle mappings to reflect new positions */
    if (wrectmaps!=NULL && wrectmaps->n>0)
        for (i=0;i<=ng;i++)
            {
            int x0;
            x0 = (i==0) ? 0 : dx0[i-1]+(sx0[i]-sx0[i-1]);
            wrectmaps_add_gap(wrectmaps,x0,dx0[i]-x0);
            }
    willus_dmem_free(30,(double **)&dx0,funcname);
    if (gappos!=NULL)
        willus_dmem_free(9,(double **)&gappos,funcname);
    }


/*
** Starting at bitmap column x0, insert a gap of dx pixels into the set of WRECTMAPS.
*/
static void wrectmaps_add_gap(WRECTMAPS *wrectmaps,int x0,int dx)

    {
    int i;
    WRECTMAP wrmap2;

    if (wrectmaps==NULL || wrectmaps->n<=0)
        return;
    wrmap2.coords[2].x = -1.0; /* Indicates if touched by loop below */
    for (i=0;i<wrectmaps->n;i++)
        {
        WRECTMAP *wrmap;

        wrmap=&wrectmaps->wrectmap[i];
        /* wrmap is before gap--not affected */
        if (wrmap->coords[1].x + wrmap->coords[2].x < x0)
            continue;
        /* gap is inside wrmap--split into two */
        if (wrmap->coords[1].x < x0+.5)
            {
            double len1,len2;

            /* WRECTMAP is split into lengths len1 and len2 with the gap dx inbetween */
            len1 = x0-wrmap->coords[1].x;
            len2 = wrmap->coords[2].x - len1;
            /* Copy wrmap to second WRECTMAP */
            wrmap2 = (*wrmap);
            /* Adjust appropriate params */
            wrmap->coords[2].x = len1;
            wrmap2.coords[0].x = wrmap->coords[0].x + len1;
            wrmap2.coords[1].x = wrmap->coords[1].x + len1 + dx;
            /* Positive value for wrmap2.coords[2].x signals it to be added after loop. */
            wrmap2.coords[2].x = len2;
            }
        else
            {
            /* gap is before wrmap--move it over by dx */
            wrmap->coords[1].x += dx;
            }
        }
    if (wrmap2.coords[2].x > 0.)
        {
        wrectmaps_add_wrectmap(wrectmaps,&wrmap2);
        wrectmaps_sort_horizontally(wrectmaps);
        }
    }

/*
static void find_word_gaps_using_wrectmaps(WRECTMAPS *wrectmaps,int **pgappos,
                                           int **pgapsize,int *png,int left_to_right)

    {
    int *gappos,*gapsize,ng;
    static char *funcname="find_word_gaps_using_wrectmaps";

    ng=wrectmaps->n-1;
    if (ng<0)
        ng=0;
    if (ng>0)
        {
        int i;

        willus_dmem_alloc_warn(9,(void **)pgappos,(2*sizeof(int))*ng,funcname,10);
        gappos=(*pgappos);
        gapsize=&gappos[ng];
        for (i=0;i<ng;i++)
            {
            double x0,x1;

            if (left_to_right)
                {
                x0=wrectmaps->wrectmap[i].coords[1].x+wrectmaps->wrectmap[i].coords[2].x;
                x1=wrectmaps->wrectmap[i+1].coords[1].x;
                }
            else
                {
                x0=wrectmaps->wrectmap[ng-i].coords[1].x+wrectmaps->wrectmap[ng-i].coords[2].x;
                x1=wrectmaps->wrectmap[ng-i-1].coords[1].x;
                }
            gappos[i]=(int)(x0+.5);
            gapsize[i]=(int)(x1-x0+.5);
            }
        }
    else
        (*pgappos)=(*pgapsize)=NULL;
    (*png)=ng;
    }
*/


static void find_word_gaps_using_textrow(WILLUSBITMAP *src,K2PDFOPT_SETTINGS *k2settings,
                                         int **pgappos,int **pgapsize,int *png,int whitethresh,
                                         int dpi)

    {
    int *gappos,*gapsize,ng,srcbytespp;
    BMPREGION srcregion;
    WILLUSBITMAP gray;
    TEXTWORDS *textwords;
    static char *funcname="find_word_gaps_using_textrow";

    /* Find breaks in the text row */
    bmpregion_init(&srcregion);
    srcregion.bgcolor=whitethresh;
    srcregion.c1=0;
    srcregion.c2=src->width-1;
    srcregion.r1=0;
    srcregion.r2=src->height-1;
    srcregion.dpi=dpi;
    srcbytespp=src->bpp==24 ? 3 : 1;
    if (srcbytespp==3)
        {
        srcregion.bmp=src;
        srcregion.bmp8=&gray;
        bmp_init(srcregion.bmp8);
        bmp_convert_to_greyscale_ex(srcregion.bmp8,src);
        }
    else
        {
        srcregion.bmp=src;
        srcregion.bmp8=src;
        }
    /* Find the words within the region */
    bmpregion_trim_margins(&srcregion,k2settings,0x1f);
    bmpregion_one_row_find_textwords(&srcregion,k2settings,1);
    textwords=&srcregion.textrows;
    if (srcbytespp==3)
        bmp_free(srcregion.bmp8);
    ng=textwords->n-1;
    if (ng>0)
        {
        int maxsize,mingap,i,j;

        willus_dmem_alloc_warn(9,(void **)pgappos,(2*sizeof(int))*ng,funcname,10);
        gappos=(*pgappos);
        gapsize=&gappos[ng];
        (*pgapsize)=gapsize;
        for (i=0;i<ng;i++)
            {
            gappos[i]=textwords->textrow[i].c2+1;
            gapsize[i]=textwords->textrow[i].gap;
            }
        
        /* Take only the largest group of gaps */
        for (maxsize=i=0;i<ng;i++)
            if (maxsize<gapsize[i])
                maxsize=gapsize[i];
        /* Could this be classified as a figure already?? */
        /* v2.20 -- only use word_spacing */
        if (srcregion.bbox.type!=REGION_TYPE_FIGURE)
            mingap = srcregion.bbox.lcheight*fabs(k2settings->word_spacing);
        else
            mingap = -1;
        /*
        if (mingap < 2)
            mingap = 2; 
        if (maxsize > mingap)
            maxsize = mingap;
        ms2 = maxsize/2;
        */
        for (i=j=0;i<ng;i++)
            if (mingap>0 && gapsize[i] > mingap)
                {
                if (j!=i)
                    {
                    gapsize[j]=gapsize[i];
                    gappos[j]=gappos[i];
                    }
                j++;
                }
        ng=j;
        }
    else
        (*pgappos)=(*pgapsize)=NULL;
    bmpregion_free(&srcregion);
    (*png)=ng;
    }


#if 0
/*
** Find gaps in the master bitmap so that it can be broken into regions
** which go onto separate pages.
*/
static int masterinfo_break_point(MASTERINFO *masterinfo,int maxsize)

    {
    static char *funcname="break_point";
    int *rowcount;
    int fig,fc,figend,cw,bp1,bp2,i,j,goodsize,figure,bp,scanheight,nwc;
    int bp1f,bp2f;
    int bp1e,bp2e;

k2printf("@breakpoint, mi->rows=%d, maxsize=%d\n",masterinfo->rows,maxsize);
k2printf("    fit_to_page=%d\n",(int)masterinfo->fit_to_page);
{
static int count=1;
char filename[MAXFILENAMELEN];
sprintf(filename,"page%04d.png",count++);
bmp_write(&masterinfo->bmp,filename,stdout,100);
}
    /* masterinfo->fit_to_page==-2 means user specified -f2p -2 which means */
    /* flush entire contents of master to single page every time.   */
    if (masterinfo->rows<maxsize || masterinfo->fit_to_page==-2)
        return(masterinfo->rows);

    /* scanheight tells how far down the master bitmap to scan */
    if (masterinfo->fit_to_page==-1)
        scanheight=masterinfo->rows;
    else if (masterinfo->fit_to_page>0)
        scanheight=(int)(((1.+masterinfo->fit_to_page/100.)*maxsize)+.5);
    else
        scanheight=maxsize;
    /* If available rows almost exactly fit page, just send the whole thing */
k2printf("    scanheight=%d, mi->rows=%d, fabs=%g\n",scanheight,masterinfo->rows,
fabs((double)scanheight/masterinfo->rows-1.));
    if (masterinfo->fit_to_page==0 && (abs(scanheight-masterinfo->rows)<=1
             || fabs((double)scanheight/masterinfo->rows-1.)<.002))
        return(masterinfo->rows);
    if (scanheight > masterinfo->rows)
        scanheight=masterinfo->rows;
    goodsize=masterinfo->bmp.width/100;
    figure=masterinfo->bmp.width/10;
    willus_dmem_alloc_warn(29,(void **)&rowcount,masterinfo->rows*sizeof(int),funcname,10);
    for (j=0;j<masterinfo->rows;j++)
        {
        unsigned char *p;
        p=bmp_rowptr_from_top(&masterinfo->bmp,j);
        rowcount[j]=0;
        if (masterinfo->bmp.bpp==24)
            {
            for (i=0;i<masterinfo->bmp.width;i++,p+=3)
                if (GRAYLEVEL(p[0],p[1],p[2])<masterinfo->bgcolor)
                    rowcount[j]++;
            }
        else
            {
            for (i=0;i<masterinfo->bmp.width;i++,p++)
                if (p[0]<masterinfo->bgcolor)
                    rowcount[j]++;
            }
        }
    /* cw = consecutive all-white rows */
    /* fc = consecutive non-all-white rows */
    bp1f=bp2f=0; /* bp1f,bp2f = max break points that fit within maxsize */
    bp1e=bp2e=0; /* bp1e,bp2e = min break points that exceed maxsize */
    for (figend=fc=fig=cw=i=bp=bp1=bp2=nwc=0;i<scanheight;i++)
        {
        if (rowcount[i]==0)
            {
if (cw==0)
k2printf("%d black\n",fc);
            cw++;
            if (fc>figure)
                {
                fig=i-fc;
                figend=i;
                }
            fc=0;
            if (fig && i-figend > fc/2)
                fig=0;
            if (fig)
                continue;
            if (nwc==0)
                continue;
            bp1=i-cw/2;
            if (bp1<=maxsize)
                bp1f=bp1;
            if (bp1>maxsize && bp1e==0)
                bp1e=bp1;
            if (cw>=goodsize)
                {
                bp2=i-cw/2;
                if (bp2<=maxsize)
                    bp2f=bp2;
                if (bp2>maxsize && bp2e==0)
                    bp2e=bp2;
                }
            }
        else
            {
if (fc==0)
k2printf("%d white\n",cw);
            cw=0;
            nwc++;
            fc++;
            }
        }
{
static int count=0;
FILE *out;
count++;
k2printf("rows=%d, gs=%d, scanheight=%d, bp1=%d, bp2=%d\n",masterinfo->rows,goodsize,scanheight,bp1,bp2);
k2printf("     bp1f=%d, bp2f=%d, bp1e=%d, bp2e=%d\n",bp1f,bp2f,bp1e,bp2e);
//bmp_write(&masterinfo->bmp,"master.png",stdout,100);
//out=fopen("rc.dat","w");
//for (i=0;i<scanheight;i++)
//fprintf(out,"%d\n",rowcount[i]);
//fclose(out);
// if (count==2)
// exit(10);
}
    willus_dmem_free(29,(double **)&rowcount,funcname);
    if (masterinfo->fit_to_page==0)
        {
        if (bp2 > maxsize*.8)
            return(bp2);
        if (bp1 < maxsize*.25)
            bp1=scanheight;
        return(bp1);
        }
    if (bp1f==0 && bp1e==0)
        return(scanheight);
    if (bp2f > 0)
        return((bp1f>0 && bp2f < maxsize*.8) ? bp1f : bp2f);
    if (bp1f > 0)
        return(bp1f);
    if (masterinfo->fit_to_page<0)
        return(bp1e);
    if (bp2e > 0)
        return(bp2e);
    return(bp1e);
    }
#endif /* 0 */


/*
** Find gaps in the master bitmap so that it can be broken into regions
** which go onto separate pages.
**
** v2.33:  Factor in page break markers
**
** maxsize is the ideal desired bitmap size to fit the page.
** Depending on the fit_to_page setting, the bitmap can actually go
** beyond this.
**
** Re-written to use bmpregion_find_textrows() in v2.10.
**
*/
static int masterinfo_break_point(MASTERINFO *masterinfo,K2PDFOPT_SETTINGS *k2settings,int maxsize)

    {
    int rowcount;
    int i,nobreak;

    rowcount=masterinfo_break_point_1(masterinfo,k2settings,maxsize);
#if (WILLUSDEBUGX & 0x800000)
printf("\n@masterinfo_break_point(rows=%d, recommended break=%d)\n",masterinfo->rows,rowcount);
#endif
    if (masterinfo->k2pagebreakmarks.n==0)
{
#if (WILLUSDEBUGX & 0x800000)
printf("    No page break marks.\n");
#endif
        return(rowcount);
}
    nobreak=-999;
#if (WILLUSDEBUGX & 0x800000)
for (i=0;i<masterinfo->k2pagebreakmarks.n;i++)
{
K2PAGEBREAKMARK *mark;
mark=&masterinfo->k2pagebreakmarks.k2pagebreakmark[i];
if (mark->type>=0)
printf("    Mark Type %d @ row=%d\n",mark->type,mark->row);
}
#endif
    for (i=0;i<masterinfo->k2pagebreakmarks.n;i++)
        {
        K2PAGEBREAKMARK *mark;

        mark=&masterinfo->k2pagebreakmarks.k2pagebreakmark[i];
        if (mark->type<0)
            continue;
        if (mark->row >= rowcount && nobreak < -990)
            break;
        if (mark->type==K2PAGEBREAKMARK_TYPE_BREAKPAGE)
            {
            mark->type=-1;
            rowcount=mark->row;
#if (WILLUSDEBUGX & 0x800000)
printf("    Page break mark forces rowcount = %d\n",rowcount);
#endif
            nobreak = -999;
            break;
            }
        if (mark->type==K2PAGEBREAKMARK_TYPE_NOBREAK)
            {
            if (nobreak > 1)
                {
                if (mark->row > rowcount)
                    {
                    rowcount=nobreak;
#if (WILLUSDEBUGX & 0x800000)
printf("    No-break span pre-dump forces rowcount = %d\n",nobreak);
#endif
                    break;
                    }
                }
            nobreak=nobreak > -990 ? -999 : mark->row;
            if (nobreak < -990 && mark->row > rowcount)
                {
#if (WILLUSDEBUGX & 0x800000)
printf("    No-break span forces rowcount = %d\n",mark->row);
#endif
                rowcount=mark->row;
                mark->type=-1;
                break;
                }
            }
        }
#if (WILLUSDEBUGX & 0x800000)
printf("    Final breakpoint = %d\n\n",rowcount);
#endif
    return(rowcount);
    }


/*
** Called by masterinfo_break_point -- does not factor in page break markers.
*/
static int masterinfo_break_point_1(MASTERINFO *masterinfo,K2PDFOPT_SETTINGS *k2settings,int maxsize)

    {
    int scanheight,j,r1,r2,r1a,r2a,rowcount;
    BMPREGION region;
    WILLUSBITMAP *bmp,_bmp;

/*
k2printf("@breakpoint, mi->rows=%d, maxsize=%d\n",masterinfo->rows,maxsize);
k2printf("    fit_to_page=%d\n",(int)masterinfo->fit_to_page);
{
static int count=1;
char filename[MAXFILENAMELEN];
sprintf(filename,"page%04d.png",count++);
bmp_write(&masterinfo->bmp,filename,stdout,100);
}
*/
    /* masterinfo->fit_to_page==-2 means user specified -f2p -2 which means */
    /* flush entire contents of master to single page every time.   */
    if (masterinfo->rows<maxsize || masterinfo->fit_to_page==-2)
        return(masterinfo->rows);

    /* scanheight tells how far down the master bitmap to scan */
    if (masterinfo->fit_to_page==-1)
        scanheight=masterinfo->rows;
    else if (masterinfo->fit_to_page>0)
        scanheight=(int)(((1.+masterinfo->fit_to_page/100.)*maxsize)+.5);
    else
        scanheight=maxsize;
    /* If available rows almost exactly fit page, just send the whole thing */
/*
k2printf("    scanheight=%d, mi->rows=%d, fabs=%g\n",scanheight,masterinfo->rows,
fabs((double)scanheight/masterinfo->rows-1.));
*/
    if (masterinfo->fit_to_page==0 && (abs(scanheight-masterinfo->rows)<=1
             || fabs((double)scanheight/masterinfo->rows-1.)<.002))
        return(masterinfo->rows);
    if (scanheight > masterinfo->rows)
        scanheight=masterinfo->rows;

    /*
    ** Find text rows (and gaps between)
    */
    bmp=&_bmp;
    bmp_init(bmp);
    if (bmp_is_grayscale(&masterinfo->bmp))
        bmp_copy(bmp,&masterinfo->bmp);
    else
        bmp_convert_to_grayscale_ex(bmp,&masterinfo->bmp);
    bmp->height=scanheight*1.4;
    if (bmp->height >  masterinfo->rows)
        bmp->height = masterinfo->rows;
    bmpregion_init(&region);
    region.bgcolor=masterinfo->bgcolor;
    region.c1=0;
    region.c2=bmp->width-1;
    region.r1=0;
    region.r2=bmp->height-1;
    region.bmp8=bmp;
    region.bmp=bmp;
    region.dpi=k2settings->dst_dpi;
    bmpregion_find_textrows(&region,k2settings,0,1,k2settings->join_figure_captions);
/*
{
static int count=1;
char filename[MAXFILENAMELEN];
printf("regiondpi = %d\n",region.dpi);
bmpregion_write(&region,"region.png");
sprintf(filename,"page%04d.png",count);
bmp_write(bmp,filename,stdout,100);
printf("\nmaxsize=%d, scanheight=%d, dst_dpi=%d\n",maxsize,scanheight,k2settings->dst_dpi);
printf("OUTPUT PAGE %d\n",count++);
for (j=0;j<region.textrows.n;j++)
{
printf("%d. ",j+1);
textrow_echo(&region.textrows.textrow[j],stdout);
}
}
*/
    bmp_free(bmp);
    for (r1a=r2a=r1=r2=j=0;j<region.textrows.n;j++)
        {
        TEXTROW *row;

        row=&region.textrows.textrow[j];
        r2=row->r2+1;
        if (j<region.textrows.n-1)
            r2a=(row->r2+region.textrows.textrow[j+1].r1)/2; /* Midpoint */
        else
            r2a=r2;
        if (row->r2 > maxsize)
            break;
        r1=r2;
        r1a=r2a;
        }
    bmpregion_free(&region);
    if (r1a<=maxsize)
        r1=r1a;
    if (r2a<=scanheight)
        r2=r2a;
    rowcount=(r1<maxsize*.25 ? (r2<scanheight ? r2:scanheight) : r1);
    /* v2.16:  Avoid zero rowcount! */
    if (rowcount<=2)
        rowcount=scanheight;
    return(rowcount);
    }


/*
** Get padding margins for destination device
** margins_pixels[0] = left side
** margins_pixels[1] = top side
** margins_pixels[2] = right side
** margins_pixels[3] = bottom side
**
** dpi should be physical device DPI (not including any document magnification)
*/
void get_dest_margins(int *margins_pixels,K2PDFOPT_SETTINGS *k2settings,
                      double dpi,int width_pixels,int height_pixels)

    {
    int i;

    for (i=0;i<4;i++)
        {
        double x;
        int unit;

        x=k2settings->dstmargins.box[i];
        unit=k2settings->dstmargins.units[i];
        if (unit==UNITS_PIXELS)
            margins_pixels[i]=(int)floor(x+.5);
        else if (unit==UNITS_INCHES)
            margins_pixels[i]=(int)floor(x*dpi+.5);
        else if (unit==UNITS_CM)
            margins_pixels[i]=(int)floor((x/2.54)*dpi+.5);
        else
            margins_pixels[i]=(int)floor(x*((i&1)?height_pixels:width_pixels)+.5);
        }
    }


/*
** Get source margins
** Needs region->bmp, bmp8, dpi, and c1,r1,c2,r2 set.
*/
void masterinfo_get_margins(K2PDFOPT_SETTINGS *k2settings,double *margins_inches,
                            K2CROPBOX *cbox,MASTERINFO *masterinfo,BMPREGION *region)

    {
    LINE2D userrect;
    POINT2D pagedims_inches;
    LINE2D trimrect_inches;
    int i;

#if (WILLUSDEBUGX & 0x20000)
printf("@masterinfo_get_margins.\n");
printf("    box=%g,%g,%g,%g\n",
cbox->box[0],
cbox->box[1],
cbox->box[2],
cbox->box[3]);
printf("    units=%d,%d,%d,%d\n",
cbox->units[0],
cbox->units[1],
cbox->units[2],
cbox->units[3]);
#endif
    /* Autocrop page */
    if (k2settings->autocrop)
        {
        for (i=0;i<4;i++)
            margins_inches[i]=(double)masterinfo->autocrop_margins[i]/region->dpi;
        return;
        }
    userrect.p[0].x=cbox->box[0];
    userrect.p[0].y=cbox->box[1];
    userrect.p[1].x=cbox->box[2];
    userrect.p[1].y=cbox->box[3];
    pagedims_inches.x=(double)region->bmp8->width/region->dpi;
    pagedims_inches.y=(double)region->bmp8->height/region->dpi;
#if (WILLUSDEBUGX & 0x20000)
printf("    page=%gx%g in\n",pagedims_inches.x,pagedims_inches.y);
#endif
    trimrect_inches.p[0].x=(double)region->c1/region->dpi;
    trimrect_inches.p[0].y=(double)region->r1/region->dpi;
    trimrect_inches.p[1].x=(double)(region->c2+1)/region->dpi;
    trimrect_inches.p[1].y=(double)(region->r2+1)/region->dpi;
    masterinfo_convert_to_source_pixels(masterinfo,&userrect,cbox->units,&pagedims_inches,
                                        (double)region->dpi,&trimrect_inches);
#if (WILLUSDEBUGX & 0x20000)
printf("back\n");
#endif
    margins_inches[0]=userrect.p[0].x/region->dpi;
    margins_inches[1]=userrect.p[0].y/region->dpi;
    if (cbox->units[2]==UNITS_TRIMMED || cbox->units[2]==UNITS_OCRLAYER)
        margins_inches[2]=pagedims_inches.x - userrect.p[1].x/region->dpi;
    else
        margins_inches[2]=userrect.p[1].x/region->dpi;
    if (cbox->units[3]==UNITS_TRIMMED || cbox->units[3]==UNITS_OCRLAYER)
        margins_inches[3]=pagedims_inches.y - userrect.p[1].y/region->dpi;
    else
        margins_inches[3]=userrect.p[1].y/region->dpi;
    for (i=0;i<4;i++)
        if (margins_inches[i]<0.)
            margins_inches[i]=0.;
#if (WILLUSDEBUGX & 0x20000)
printf("   margins_inches=%g,%g,%g,%g\n",
margins_inches[0],
margins_inches[1],
margins_inches[2],
margins_inches[3]);
#endif
    }


/*
**
** Master function to do unit conversion.  Does entire rectangle at once.
**
** units[0] = userrect->p[0].x = x1
** units[1] = userrect->p[0].y = y1
** units[2] = userrect->p[1].x = x2
** units[3] = userrect->p[1].y = y2
** If units[i]<0, ignored.
**
** pagedims_inches supplies page dimensions in inches (pagedims_inches->x = width, y = height).
**
** If UNITS_TRIMMED used, then trimrect_in->p[0] = upper left corner of trimmed region
** relative to upper left corner of source page, in inches, and trimrect_in->p[1] =
** lower right corner of trimmed region relative to upper left corner of source page, in inches.
**
*/
void masterinfo_convert_to_source_pixels(MASTERINFO *masterinfo,LINE2D *userrect,int *units,
                                         POINT2D *pagedims_inches,double dpi,LINE2D *trimrect_in)

    {
    int i;

#if (WILLUSDEBUGX & 0x20000)
printf("userrect=%g,%g,%g,%g\n",userrect->p[0].x,userrect->p[0].y,userrect->p[1].x,userrect->p[1].y);
#endif
    for (i=0;i<4;i++)
        {
        double userval,srclen_in;
        int unit,userval_pixels;

        unit=units[i];
        if (unit<0)
            continue;
        userval = (i&1) ? userrect->p[i/2].y : userrect->p[i/2].x;
        if (unit==UNITS_CM)
            {
            userval /= 2.54;
            unit=UNITS_INCHES;
            }
        if (unit==UNITS_OCRLAYER)
            {
            LINE2D ocrbb;

            if (!ocrlayer_bounding_box_inches(masterinfo,&ocrbb))
                {
                ocrbb.p[0].x=ocrbb.p[0].y=0.;
                ocrbb.p[1].x=pagedims_inches->x;
                ocrbb.p[1].y=pagedims_inches->y;
                }
            if (i&1)
                userrect->p[i/2].y=floor(dpi*(ocrbb.p[0].y+userrect->p[i/2].y*(ocrbb.p[1].y-ocrbb.p[0].y))+.5);
            else
                userrect->p[i/2].x=floor(dpi*(ocrbb.p[0].x+userrect->p[i/2].x*(ocrbb.p[1].x-ocrbb.p[0].x))+.5);
            continue;
            }
        if (unit==UNITS_TRIMMED && trimrect_in!=NULL)
            {
            if (i&1)
                userrect->p[i/2].y = floor((trimrect_in->p[0].y+userrect->p[i/2].y*(trimrect_in->p[1].y-trimrect_in->p[0].y))*dpi+.5);
            else
                userrect->p[i/2].x = floor((trimrect_in->p[0].x+userrect->p[i/2].x*(trimrect_in->p[1].x-trimrect_in->p[0].x))*dpi+.5);
            continue;
            }
        if (i&1)
            srclen_in = pagedims_inches->y;
        else
            srclen_in = pagedims_inches->x;
        if (unit==UNITS_TRIMMED || unit==UNITS_SOURCE)
            userval_pixels = (int)(srclen_in*userval*dpi+.5);
        else if (unit==UNITS_INCHES)
            userval_pixels = (int)(userval*dpi+.5);
        else
            userval_pixels = (int)(userval+.5);
        if (i&1)
            userrect->p[i/2].y=userval_pixels;
        else
            userrect->p[i/2].x=userval_pixels;
        }
#if (WILLUSDEBUGX & 0x20000)
printf("userrect(end)=%g,%g,%g,%g\n",userrect->p[0].x,userrect->p[0].y,userrect->p[1].x,userrect->p[1].y);
#endif
    }
            
/*
** Distances are from upper-left corner of source page in inches.
** rect->p[0].x = left side of OCR layer bounding box.
** rect->p[0].y = top side of OCR layer bounding box.
** rect->p[1].x = right side of OCR layer bounding box.
** rect->p[1].y = bottom side of OCR layer bounding box.
*/
static int ocrlayer_bounding_box_inches(MASTERINFO *masterinfo,LINE2D *rect)

    {
#ifdef HAVE_MUPDF_LIB
    static WTEXTCHARS *wtcs=NULL;
    static WTEXTCHARS _wtcs;
    static int pageno=-1;
    static char pdffile[512];

#if (WILLUSDEBUGX & 0x200000)
printf("@ocrlayer_bounding_box_inches...masterinfo=%p\n",masterinfo);
if (wtcs!=NULL)
printf("    wtcs->n = %d\n",wtcs->n);
#endif
    if (masterinfo==NULL)
        return(0);
    if (wtcs==NULL)
        {
        wtcs=&_wtcs;
        wtextchars_init(wtcs);
        pdffile[0]='\0';
        }
    if (pageno!=masterinfo->pageinfo.srcpage || strcmp(pdffile,masterinfo->srcfilename))
        {
        wtextchars_clear(wtcs); /* v2.32 bug fix--clear out any previous words */
        wtextchars_fill_from_page_ex(wtcs,masterinfo->srcfilename,masterinfo->pageinfo.srcpage,"",1);
        wtextchars_rotate_clockwise(wtcs,360-(int)masterinfo->pageinfo.srcpage_rot_deg);
        pageno=masterinfo->pageinfo.srcpage;
        strncpy(pdffile,masterinfo->srcfilename,511);
        pdffile[511]='\0';
        }
#if (WILLUSDEBUGX & 0x200000)
if (wtcs!=NULL)
printf("    wtcs->n after fill_from_page = %d\n",wtcs->n);
#endif
    if (wtcs->n>0)
        {
        WTEXTCHAR *tc;

        tc=&wtcs->wtextchar[0];
        rect->p[0].x=tc->x1/72.;
        rect->p[0].y=tc->y1/72.;
        rect->p[1].x=tc->x2/72.;
        rect->p[1].y=tc->y2/72.;
#if (WILLUSDEBUGX & 0x200000)
printf("    Inches:  (%g,%g) - (%g,%g)\n",tc->x1/72.,tc->y1/72.,tc->x2/72.,tc->y2/72.);
#endif
        return(1);
        }
#endif
    return(0);
    }


static void k2master_rows_color(WILLUSBITMAP *srcbmp,MASTERINFO *masterinfo,
                                K2PDFOPT_SETTINGS *k2settings)

    {
    PAGEREGIONS *pageregions,_pageregions;
    int j,maxlevels;
    WILLUSBITMAP *bmpmask,_bmpmask;
    char fgc[128],bgc[128];
    BMPREGION *region,_region;
    WILLUSBITMAP *gray,_gray;


/*
printf("@k2master_rows_color:  %d x %d\n",srcbmp->width,srcbmp->height);
*/
    region=&_region;
    if (srcbmp->bpp==8)
        gray=srcbmp;
    else
        {
        gray=&_gray;
        bmp_init(gray);
        bmp_convert_to_grayscale_ex(gray,srcbmp);
        }
    bmpregion_init(region);
    region->bgcolor=k2settings->src_whitethresh<0 ? 192 : k2settings->src_whitethresh;
    region->c1=0;
    region->c2=gray->width-1;
    region->r1=0;
    region->r2=gray->height-1;
    region->bmp8=gray;
    region->bmp=srcbmp; /* May not be 24-bit! */
    region->dpi=k2settings->dst_dpi;
    region->wrectmaps=NULL;
    bmpregion_trim_margins(region,k2settings,0xf);
    bmpmask=&_bmpmask;
    bmp_init(bmpmask);
    bmp_copy(bmpmask,srcbmp);
    fgc[0]='\0';
    bgc[0]='\0';

    /* Parse region into columns */
    pageregions=&_pageregions;
    pageregions_init(pageregions);
#ifdef HAVE_OCR_LIB
    if (k2settings->ocr_max_columns==2 || k2settings->max_columns>1)
#else
    if (k2settings->max_columns>1)
#endif
        maxlevels = 2;
    else
        maxlevels = 3;
    pageregions_find_columns(pageregions,region,k2settings,masterinfo,maxlevels);
    for (j=0;j<pageregions->n;j++)
        {
        int i,nfg,nbg;
        BMPREGION *subregion;

        subregion=&pageregions->pageregion[j].bmpregion;
/*
printf("region %d:  (%d,%d) - (%d,%d)\n",j,subregion->c1,subregion->r1,subregion->c2,subregion->r2);
*/
        bmpregion_find_textrows(subregion,k2settings,0,1,0);
        nfg=k2settings_ncolors(k2settings->dst_fgcolor);
        nbg=k2settings_ncolors(k2settings->dst_bgcolor);
/*
printf("nc=%d, rcindex=%d\n",nc,masterinfo->rcindex);
printf("TEXTROWS.n=%d\n",region->textrows.n);
for (i=0;i<region->textrows.n;i++)
printf("TEXTROW[%2d]=%d\n",i,region->textrows.textrow[i].type);
*/
        for (i=0;i<subregion->textrows.n;i++)
            {
            TEXTROW *textrow;

            textrow=&subregion->textrows.textrow[i];
            if (textrow->type==REGION_TYPE_TEXTLINE)
                {
                if (k2settings->dst_fgtype==4)
                    strcpy(fgc,k2settings_color_by_index(k2settings->dst_fgcolor,
                                                         masterinfo->rcindex%nfg));
                if (k2settings->dst_bgtype==4)
                    strcpy(bgc,k2settings_color_by_index(k2settings->dst_bgcolor,
                                                         masterinfo->rcindex%nbg));
                masterinfo->rcindex++;
                /* For some reason if it's a single text line, need to re-calc bbox. */
                /* Should investigate why at some point... */
/*
                if (single_passed_textline)
                    {
                    bmpregion_calc_bbox(&region,k2settings,1);
                    textrow->c1=region->bbox.c1;
                    textrow->c2=region->bbox.c2;
                    textrow->r1=region->bbox.r1;
                    textrow->r2=region->bbox.r2;
                    }
*/
                bmp_change_colors(subregion->bmp,bmpmask,fgc,k2settings_color_type(fgc),
                                             bgc,k2settings_color_type(bgc),
                                             textrow->c1,textrow->r1,
                                             textrow->c2,textrow->r2);
                }
            }
        }
    if (k2settings->dst_fgtype!=4)
        strcpy(fgc,k2settings->dst_fgcolor);
    else
        fgc[0]='\0';
    if (k2settings->dst_bgtype!=4)
        strcpy(bgc,k2settings->dst_bgcolor);
    else
        bgc[0]='\0';
    if (fgc[0]!='\0' || bgc[0]!='\0')
        bmp_change_colors(region->bmp,bmpmask,fgc,k2settings_color_type(fgc),
                                              bgc,k2settings_color_type(bgc),
                                         0,0,region->bmp->width-1,region->bmp->height-1);
    pageregions_free(pageregions);
    bmp_free(bmpmask);
    if (srcbmp->bpp!=8)
        bmp_free(gray);
    }
