/*
** k2settings.c     Handles k2pdfopt settings (K2PDFOPT_SETTINGS structure)
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

static void k2settings_warn(char *message);
static void k2settings_apply_odpi_magnification(K2PDFOPT_SETTINGS *k2settings,double dispres,
                                                double mag);


void k2pdfopt_settings_init(K2PDFOPT_SETTINGS *k2settings)

    {
    int i;

    k2settings->verbose=0;
    k2settings->debug=0;
    k2settings->cdthresh=.01;
#ifdef HAVE_K2GUI
    k2settings->gui=-1;
    k2settings->guimin=0;
#endif
/*
** Blank Area Threshold Widths--average black pixel width, in inches, that
** prevents a region from being determined as "blank" or clear.
*/
    k2settings->src_rot=SRCROT_AUTO;
    k2settings->gtc_in=.005; // detecting gap between columns
    k2settings->gtr_in=.006; // detecting gap between rows
    k2settings->gtw_in=.0015; // detecting gap between words
    k2settings->show_usage[0]='\0';
    k2settings->src_left_to_right=1;
    k2settings->src_whitethresh=-1;
    k2settings->src_paintwhite=0;
#ifdef HAVE_OCR_LIB
    k2settings->ocrout[0]='\0';
    k2settings->ocr_detection_type='l';
    /* v2.51 */
    /* Tesseract v4.0.0 English "Tessbest" seems to do best with 300 dpi for ~8 - 15 pt fonts */
    k2settings->ocr_dpi=300;
#ifdef HAVE_TESSERACT_LIB
    k2settings->dst_ocr_lang[0]='\0';
#endif
    k2settings->ocr_max_columns=-1;  /* -1 = use value of max_columns */
#ifdef HAVE_MUPDF_LIB
    k2settings->dst_ocr='m';
#else
    k2settings->dst_ocr=0;
#endif
    k2settings->dst_ocr_visibility_flags=1;
    k2settings->ocr_max_height_inches=1.5;
    ocrwords_init(&k2settings->dst_ocrwords);
    k2settings->sort_ocr_text=0;
#endif
    k2settings->dst_dither=1;
    k2settings->dst_break_pages=1;
    k2settings->render_dpi=167;
    k2settings->fit_columns=1;
    k2settings->user_src_dpi=-2.0;
    k2settings->document_scale_factor=1.0;
    k2settings->src_dpi=300;
#ifdef HAVE_MUPDF_LIB
    k2settings->user_usegs=0;
#else
    k2settings->user_usegs=1;
#endif
    k2settings->usegs=k2settings->user_usegs;
    k2settings->query_user=-1;
    k2settings->query_user_explicit=0;
    k2settings->jpeg_quality=-1;
    k2settings->dst_magnification=1.0;
    k2settings->dst_display_resolution=1.0;
    k2settings->dst_justify=-1; // 0 = left, 1 = center
    k2settings->dst_figure_justify=-1; // -1 = same as dst_justify.  0=left 1=center 2=right
    k2settings->dst_min_figure_height_in=0.75;
    k2settings->dst_fulljustify=-1; // 0 = no, 1 = yes
    k2settings->dst_sharpen=1;
    k2settings->dst_bpc=4;
    k2settings->dst_landscape=0;
    k2settings->dst_landscape_pages[0]='\0'; /* v2.32 */
    strcpy(k2settings->dst_opname_format,"%s_k2opt");
    k2settings->src_autostraighten=0;
    k2settings->dstmargins.pagelist[0]='\0';
    k2settings->dstmargins.cboxflags=0;
    for (i=0;i<4;i++)
        {
        k2settings->dstmargins.box[i]=.02;
        k2settings->dstmargins.units[i]=UNITS_INCHES;
        }
    k2settings->min_column_gap_inches=0.1;
    k2settings->max_column_gap_inches=1.5; // max gap between columns
    k2settings->min_column_height_inches=1.5;
    k2settings->row_split_fom=20.; /* Higher make it hard to split rows */
    k2settings->srccropmargins.pagelist[0]='\0';
    k2settings->srccropmargins.cboxflags=0;
    for (i=0;i<4;i++)
        {
        k2settings->srccropmargins.box[i]=0.;
        k2settings->srccropmargins.units[i]=UNITS_INCHES;
        }
    k2settings->max_region_width_inches = 3.6; /* Max viewable width (device width minus margins) */
    k2settings->max_columns=2;
    k2settings->column_gap_range=0.33;
    k2settings->column_offset_max=0.3;
    k2settings->column_row_gap_height_in=1./72.;
    k2settings->text_wrap=1;
    k2settings->word_spacing=-0.20;
    k2settings->display_width_inches = 3.6; /* Device width = dst_width / dst_dpi */
    k2settings->pagelist[0]='\0';
    k2settings->column_fitted=0;
    k2settings->contrast_max = 2.0;
    k2settings->dst_gamma=0.5;
    k2settings->dst_negative=0;
    k2settings->exit_on_complete=-1;
    k2settings->show_marked_source=0;
    k2settings->use_crop_boxes=0;
    k2settings->preserve_indentation=1;
    k2settings->defect_size_pts=0.75;
    k2settings->max_vertical_gap_inches=0.25;
    k2settings->vertical_multiplier=1.0;
    k2settings->vertical_line_spacing=-1.2;
    k2settings->vertical_break_threshold=1.75;
    k2settings->src_trim=1;
    k2settings->erase_vertical_lines=0;
    k2settings->hyphen_detect=1;
    k2settings->overwrite_minsize_mb=10;
    k2settings->dst_fit_to_page=0;
    k2settings->src_grid_rows=-1;
    k2settings->src_grid_cols=-1;
    k2settings->src_grid_overlap_percentage=2.;
    k2settings->src_grid_order=0; /* 0=down then across, 1=across then down */
    k2settings->preview_page=0; /* 0 = no preview */
    k2settings->echo_source_page_count=0;
/*
** Undocumented cmd-line args
*/
    k2settings->no_wrap_ar_limit=0.2; /* -arlim */
    k2settings->no_wrap_height_limit_inches=0.55; /* -whmax */
    k2settings->little_piece_threshold_inches=0.5; /* -rwmin */
/*
** Keeping track of vertical gaps
*/
    /* Not used as of v2.00
    k2settings->last_scale_factor_internal = -1.0;
    k2settings->line_spacing_internal=0;
    k2settings->last_rowbase_internal=0;
    k2settings->gap_override_internal=-1;
    */

    k2pdfopt_settings_set_to_device(k2settings,devprofile_get(K2PDFOPT_DEFAULT_DEVICE));
    k2settings->dst_width = k2settings->dst_userwidth;
    k2settings->dst_height = k2settings->dst_userheight;

    /* v2.10 */
    k2settings->use_toc = -1;
    k2settings->toclist[0]='\0';
    k2settings->tocsavefile[0]='\0';
    k2settings->bpl[0]='\0';
    k2cropboxes_init(&k2settings->cropboxes);

    /* v2.13 */
    k2settings->devsize_set=0;

    /* v2.20 */
    k2settings->noteset.n=0;
    k2settings->text_only=0;
#ifdef HAVE_K2GUI
    k2settings->restore_last_settings=-1;
#endif

    /* v2.22 */
    k2settings->dst_fgcolor[0]='\0';
    k2settings->dst_bgcolor[0]='\0';
    k2settings->dst_fgtype=0;
    k2settings->dst_bgtype=0;

    /* v2.31 */
#ifdef HAVE_GHOSTSCRIPT
    k2settings->ppgs=0;
#endif

    /* v2.32 */
    /* dst_landscape_pages (above) */
    k2settings->autocrop=0;

    /* v2.33 */
    k2settings->dst_figure_rotate=0;
    k2settings->info=0;
    k2settings->pagebreakmark_breakpage_color=-1;
    k2settings->pagebreakmark_nobreak_color=-1;
    k2settings->erase_horizontal_lines=0;
    k2settings->pagexlist[0]='\0';
    k2settings->dst_author[0]='\0';
    k2settings->dst_title[0]='\0';

    /* v2.34 */
    k2settings->dst_fontsize_pts=0.; /* 0 = not used */
    k2settings->assume_yes=0;
    k2settings->dst_coverimage[0]='\0'; /* empty string = not used */

    /* v2.35 */
    k2settings->user_mag=0; /* No magnification adjustment */
    k2settings->join_figure_captions=1;  /* Default: join captions with figures */

    /* v2.40 */
    k2settings->nthreads=-50; /* Use 50% of available CPUs */

    /* v2.41 */
    k2settings->src_erosion=0; /* No erosion */

    /* v2.42 */
#ifdef HAVE_LEPTONICA_LIB
    k2settings->dewarp=0;
#endif
    }


int k2settings_output_is_bitmap(K2PDFOPT_SETTINGS *k2settings)

    {
    return(filename_is_bitmap(k2settings->dst_opname_format));
    }


K2NOTES *page_has_notes_margin(K2PDFOPT_SETTINGS *k2settings,MASTERINFO *masterinfo)

    {
    int i;
    K2NOTES *notes;

    for (i=k2settings->noteset.n-1;i>=0;i--)
        {
        notes=&k2settings->noteset.notes[i];
        if (notes->pagelist[0]=='\0')
            return(notes);
        if (pagelist_includes_page(notes->pagelist,masterinfo->pageinfo.srcpage,
                                   masterinfo->srcpages))
            return(notes);
        }
    return(NULL);
    }


/*
** Should the current source page be turned to landscape mode?
*/
int k2pdfopt_settings_landscape(K2PDFOPT_SETTINGS *k2settings,int pageno,int maxpages)

    {
    int retval;

#if (WILLUSDEBUGX & 1)
printf("@k2pdfopt_settings_landscape(%d,%d)\n",pageno,maxpages);
printf("    dst_landscape=%d, pages='%s'\n",k2settings->dst_landscape,k2settings->dst_landscape_pages);
#endif
    if (k2settings->dst_landscape_pages[0]=='\0')
        retval=1;
    else
        retval=pagelist_includes_page(k2settings->dst_landscape_pages,pageno,maxpages);
    if (!k2settings->dst_landscape)
        retval = !retval;
#if (WILLUSDEBUGX & 1)
printf("    retval=%d\n",retval);
#endif
    return(retval);
    }
    

void k2pdfopt_conversion_init(K2PDFOPT_CONVERSION *k2conv)

    {
    k2pdfopt_settings_init(&k2conv->k2settings);
    k2pdfopt_files_init(&k2conv->k2files);
    }


void k2pdfopt_conversion_close(K2PDFOPT_CONVERSION *k2conv)

    {
    k2pdfopt_files_free(&k2conv->k2files);
    }


void k2pdfopt_settings_copy(K2PDFOPT_SETTINGS *dst,K2PDFOPT_SETTINGS *src)

    {
    (*dst)=(*src);
    }


int k2pdfopt_settings_set_to_device(K2PDFOPT_SETTINGS *k2settings,DEVPROFILE *dp)

    {
    if (dp==NULL)
        return(0);
    k2settings->dst_userwidth=dp->width;
    k2settings->dst_userwidth_units=UNITS_PIXELS;
    k2settings->dst_userheight=dp->height;
    k2settings->dst_userheight_units=UNITS_PIXELS;
    k2settings->dst_userdpi=k2settings->dst_dpi=dp->dpi;
    k2settings->mark_corners=dp->mark_corners;
    k2settings->pad_left=dp->padding[0];
    k2settings->pad_top=dp->padding[1];
    k2settings->pad_right=dp->padding[2];
    k2settings->pad_bottom=dp->padding[3];
    k2settings->dst_color=dp->color;
#if (WILLUSDEBUGX&1)
printf("Set to device: dst_dpi=%d\n",k2settings->dst_dpi);
#endif
    return(1);
    }

/*
** Warn if some command-line options are not consistent (added in v2.35).
*/
void k2settings_check_and_warn(K2PDFOPT_SETTINGS *k2settings)

    {
    char buf[256];

    if (k2settings->assume_yes)
        return;
#ifdef HAVE_LEPTONICA_LIB
    if (k2settings->use_crop_boxes && k2settings->dewarp)
        {
        sprintf(buf,"De-warping (-dw) is disabled by native mode output.");
        k2settings_warn(buf);
        }
#endif
    if (k2settings->fit_columns && k2settings->user_mag)
        {
        sprintf(buf,"You have specified -odpi, -mag, or -fs.  This may not "
                    "work as expected unless you also turn off the \"fit-column-to-device\" "
                    "option by specifying -fc-");
#ifdef HAVE_K2GUI
        if (k2gui_active())
            strcat(buf," in the \"Additional Options\" box");
#endif
        strcat(buf,".");
        k2settings_warn(buf);
        }
    }


static void k2settings_warn(char *message)

    {
    char msg2[512];

    strcpy(msg2,message);
    strcat(msg2,"  (You can disable this message by specifying -y");
#ifdef HAVE_K2GUI
        if (k2gui_active())
            strcat(msg2," in the \"Additional Options\" box");
#endif
        strcat(msg2,".)");
#ifdef HAVE_K2GUI
    if (k2gui_active())
        k2gui_alertbox(0,"Warning",msg2);
    else
#endif
    k2printf(TTEXT_WARN "\n** %s **\n\n" TTEXT_NORMAL,msg2);
    }


void k2pdfopt_settings_quick_sanity_check(K2PDFOPT_SETTINGS *k2settings)

    {
/* printf("@k2pdfopt_settings_quick_sanity_check, k2settings=%p.\n",k2settings); */
    /*
    ** Check compatibility between various settings
    */
    /*
    ** -f2p -2 means text wrapping gets turned off and -vb is set to -1 or less.
    */
    if (k2settings->dst_fit_to_page==-2)
        {
        if (k2settings->vertical_break_threshold > -1.)
            k2settings->vertical_break_threshold=-1.;
        k2settings->text_wrap=0;
        }

    /*
    ** If text wrapping is on or output is bitmap, can't use crop boxes
    */
    if (k2settings->text_wrap>0 || k2settings_output_is_bitmap(k2settings))
        k2settings->use_crop_boxes=0;

    /*
    ** v2.22: If -colorfg or -colorbg not grayscale, turn color output on.
    */
    k2settings->dst_fgtype = k2settings_color_type(k2settings->dst_fgcolor);
    k2settings->dst_bgtype = k2settings_color_type(k2settings->dst_bgcolor);
    if (k2settings->dst_fgtype==2 || k2settings->dst_bgtype==2)
        k2settings->dst_color=1;
    if (k2settings->dst_fgtype==4)
        {
        int i,n;
        n=k2settings_ncolors(k2settings->dst_fgcolor);
        for (i=0;i<n;i++)
            if (k2settings_color_type(k2settings_color_by_index(k2settings->dst_fgcolor,i))==2)
                k2settings->dst_color=1;
        }
    if (k2settings->dst_bgtype==4)
        {
        int i,n;
        n=k2settings_ncolors(k2settings->dst_bgcolor);
        for (i=0;i<n;i++)
            if (k2settings_color_type(k2settings_color_by_index(k2settings->dst_bgcolor,i))==2)
                k2settings->dst_color=1;
        }

    /* v2.22: If previewing a native PDF, turn color output on. */
    if (!k2settings->dst_color && k2settings->use_crop_boxes && k2settings->preview_page!=0)
        k2settings->dst_color=1;

    /*
    ** If OCR is on, can't use crop boxes
    ** v2.42--allow this so that the -ocrout option still works in native output mode
    */
#ifdef HAVE_OCR_LIB
    /*
    if (k2settings->dst_ocr)
        k2settings->use_crop_boxes=0;
    */
    if (k2settings->ocrout[0]!='\0' && k2settings->dst_ocr==0)
        k2settings->dst_ocr='m';
#endif
    }


double k2pdfopt_settings_gamma(K2PDFOPT_SETTINGS *k2settings)

    {
    if (!k2settings->dst_color && k2settings->use_crop_boxes && k2settings->preview_page!=0)
        return(1.0);
    return(k2settings->dst_gamma);
    }
    

/*
** Check / adjust k2pdfopt user input settings.
**
** This function is called ONLY ONCE per document before beginning the conversion
** of each new document.
**
*/
void k2pdfopt_settings_new_source_document_init(K2PDFOPT_SETTINGS *k2settings)

    {
    k2pdfopt_settings_quick_sanity_check(k2settings);

    {
    double mag,dr;
    mag = fabs(k2settings->dst_fontsize_pts)>1.0e-8 ? 1. : k2settings->dst_magnification;
    dr = k2settings->dst_display_resolution;
    k2settings_apply_odpi_magnification(k2settings,dr,mag);
    }

    /*
    ** With all parameters set, adjust output DPI so viewable region
    ** width >= MIN_REGION_WIDTH_INCHES
    ** NULL = first call, before any source page dimensions are known.
    ** Otherwise, bitmap region with source page dimensions set.
    */
    k2pdfopt_settings_set_margins_and_devsize(k2settings,NULL,NULL,-1.0,0);
    /*
    ** Set source DPI
    */
    if (k2settings->dst_dpi < 20.)
        k2settings->dst_dpi = 20.;
    k2settings->src_dpi=k2settings->user_src_dpi < 0. ? (int)(fabs(k2settings->user_src_dpi)*k2settings->dst_dpi+.5) : (int)(k2settings->user_src_dpi+.5);
    if (k2settings->src_dpi < 50.)
        k2settings->src_dpi = 50.;
    /* k2cropbox_set_default_values(&k2settings->srccropmargins,0.,UNITS_INCHES); */


    /* This part used to be called k2pdfopt_settings_new_source_document_init() before v2.34 */
    /* Reset usegs for each document */
    k2settings->usegs=k2settings->user_usegs;
    /* Init document word spacing history */
    textwords_add_word_gaps(NULL,0,NULL,0.);
#ifdef HAVE_OCR_LIB
    /* Init document OCR word list */
    if (k2settings->dst_ocr)
        {
        k2ocr_init(k2settings);
        ocrwords_clear(&k2settings->dst_ocrwords);
        }
#endif
    k2proc_init_one_document();
    }


/*
** Apply DPI magnification
*/
static void k2settings_apply_odpi_magnification(K2PDFOPT_SETTINGS *k2settings,double dispres,
                                                double mag)

    {
    k2settings->dst_dpi = k2settings->dst_userdpi*mag*dispres;
    if (k2settings->dst_userwidth_units==UNITS_PIXELS && k2settings->dst_userwidth>0)
        k2settings->dst_userwidth *= dispres;
    if (k2settings->dst_userheight_units==UNITS_PIXELS && k2settings->dst_userheight>0)
        k2settings->dst_userheight *= dispres;
#if (WILLUSDEBUGX & 1)
printf("odpi mag:  dst_dpi=%d\n",k2settings->dst_dpi);
#endif
    }


void k2pdfopt_settings_set_region_widths(K2PDFOPT_SETTINGS *k2settings)

    {
    int dstmar_pixels[4];

    k2settings->max_region_width_inches=k2settings->display_width_inches=(double)k2settings->dst_width/k2settings->dst_dpi;
    get_dest_margins(dstmar_pixels,k2settings,(double)k2settings->dst_dpi,
                     k2settings->dst_width,k2settings->dst_height);
    k2settings->max_region_width_inches -= (double)(dstmar_pixels[0]+dstmar_pixels[2])/k2settings->dst_dpi;
    }


void k2pdfopt_settings_dst_viewable(K2PDFOPT_SETTINGS *k2settings,MASTERINFO *masterinfo,
                                    double *width_in,double *height_in)

    {
    int dstmar_pixels[4];
    get_dest_margins(dstmar_pixels,k2settings,k2settings->dst_dpi,masterinfo->bmp.width,
                     k2settings->dst_height);
    (*width_in) = (double)k2settings->dst_width/k2settings->dst_dpi
                    - (double)(dstmar_pixels[0]+dstmar_pixels[2])/k2settings->dst_dpi;
    (*height_in) = (double)k2settings->dst_height/k2settings->dst_dpi
                    - (double)(dstmar_pixels[1]+dstmar_pixels[3])/k2settings->dst_dpi;
    }


void k2pdfopt_settings_restore_output_dpi(K2PDFOPT_SETTINGS *k2settings)

    {
    if (k2settings->column_fitted)
        {
        k2settings->dst_dpi=k2settings->dpi_org;
        k2settings->dstmargins=k2settings->dstmargins_org;
        k2pdfopt_settings_set_region_widths(k2settings);
        }
    k2settings->column_fitted=0;
#if (WILLUSDEBUGX & 1)
printf("restore:  dst_dpi=%d\n",k2settings->dst_dpi);
#endif
    }


void k2pdfopt_settings_fit_column_to_screen(K2PDFOPT_SETTINGS *k2settings,
                                            double column_width_inches)

    {
    double text_width_pixels;
    int i,new_dpi;

    if (!k2settings->column_fitted)
        {
        k2settings->dpi_org=k2settings->dst_dpi;
        k2settings->dstmargins_org=k2settings->dstmargins;
        }
    text_width_pixels = k2settings->max_region_width_inches*k2settings->dst_dpi;
    new_dpi = text_width_pixels / column_width_inches;
    for (i=0;i<4;i++)
        if (k2settings->dstmargins.units[i]==UNITS_INCHES
             || k2settings->dstmargins.units[i]==UNITS_CM)
            k2settings->dstmargins.box[i] *= (double)k2settings->dst_dpi/new_dpi;
    k2settings->dst_dpi=new_dpi;
#if (WILLUSDEBUGX&1)
printf("fit_column_to_screen: dst_dpi=%d\n",k2settings->dst_dpi);
#endif
    k2pdfopt_settings_set_region_widths(k2settings);
    k2settings->column_fitted=1;
    }


/*
** Set device width and height in pixels.
*/
void k2pdfopt_settings_set_margins_and_devsize(K2PDFOPT_SETTINGS *k2settings,
                                       BMPREGION *region,MASTERINFO *masterinfo,
                                       double src_fontsize_pts,int trimmed)

    {
    int new_width,new_height,zeroarea,pageno,maxpages;
    WPDFPAGEINFO *pageinfo;
    LINE2D trimrect_inches;
    POINT2D pagedims_inches;
    LINE2D userrect;
    int units[4];

#if (WILLUSDEBUGX & 0x200)
aprintf(ANSI_CYAN "@k2pdfopt_settings_set_margins_and_devsize(region=%p,masterinfo=%p,trimmed=%d)" ANSI_NORMAL "\n",region,masterinfo,trimmed);
#endif
    if (src_fontsize_pts>0. && fabs(k2settings->dst_fontsize_pts)>1.0e-8)
        {
        double mag,dres;
        dres = k2settings->dst_display_resolution;
        mag = fabs(k2settings->dst_fontsize_pts) / src_fontsize_pts;
        k2settings_apply_odpi_magnification(k2settings,dres,mag);
        }
    /* Special reset code */
    if (src_fontsize_pts<-90.)
        k2settings_apply_odpi_magnification(k2settings,k2settings->dst_display_resolution,
                                                       k2settings->dst_magnification);
    zeroarea=0;
    pageinfo=masterinfo!=NULL ? &masterinfo->pageinfo : NULL;
    if (region==NULL)
        {
        k2settings->devsize_set=0;
        /* k2cropbox_set_default_values(&k2settings->dstmargins,.02,UNITS_INCHES); */
        trimrect_inches.p[0].x=0.;
        trimrect_inches.p[0].y=0.;
        pagedims_inches.x=trimrect_inches.p[1].x=8.5;
        pagedims_inches.y=trimrect_inches.p[1].y=11.0;
        }
    else
        {
        k2settings->devsize_set++;
        trimrect_inches.p[0].x = (double)region->c1/region->dpi;
        trimrect_inches.p[0].y = (double)region->r1/region->dpi;
        trimrect_inches.p[1].x = (double)(region->c2+1)/region->dpi;
        trimrect_inches.p[1].y = (double)(region->r2+1)/region->dpi;
        if (region->c2-region->c1<=0 || region->r2-region->r1<=0)
            zeroarea=1;
        pagedims_inches.x = (double)region->bmp->width / region->dpi;
        pagedims_inches.y = (double)region->bmp->height / region->dpi;
        }

/*    
    if (trimmed)
        {
    if (wu<=0.)
        wu=twidth_in;
    if (hu<=0.)
        hu=theight_in;
    if (wu<=0.)
        wu=swidth_in;
    if (hu<=0.)
        hu=sheight_in;
        }
    else
        {
        wu=swidth_in;
        hu=sheight_in;
        }
printf("wu=%g, hu=%g\n",wu,hu);
*/
    userrect.p[0].x=0.;
    userrect.p[0].y=0.;
    userrect.p[1].x=k2settings->dst_userwidth;
    userrect.p[1].y=k2settings->dst_userheight;
    units[0]=units[2]=k2settings->dst_userwidth_units;
    units[1]=units[3]=k2settings->dst_userheight_units;
    if (userrect.p[1].x < 0.)
        {
        userrect.p[1].x=fabs(userrect.p[1].x);
        units[0]=units[2]=UNITS_SOURCE;
        }
    if (userrect.p[1].y < 0.)
        {
        userrect.p[1].y=fabs(userrect.p[1].y);
        units[1]=units[3]=UNITS_SOURCE;
        }
#if (WILLUSDEBUGX & 0x20000)
printf("@setmargins\n");
printf("    dpi = %g\n",(double)k2settings->dst_dpi);
printf("    dst_fontsize = %g pts\n",k2settings->dst_fontsize_pts);
printf("    userrect = %g,%g - %g,%g\n",userrect.p[0].x,userrect.p[0].y,userrect.p[1].x,userrect.p[1].y);
printf("    units = %d,%d,%d,%d\n",units[0],units[1],units[2],units[3]);
printf("    page = %g x %g in\n",pagedims_inches.x,pagedims_inches.y);
printf("    trim = %g,%g - %g,%g in\n",trimrect_inches.p[0].x,trimrect_inches.p[0].y,
                                       trimrect_inches.p[1].x,trimrect_inches.p[1].y);
#endif
    masterinfo_convert_to_source_pixels(masterinfo,&userrect,units,&pagedims_inches,
                                        (double)k2settings->dst_dpi,&trimrect_inches);
#if (WILLUSDEBUGX & 0x20000)
printf("    userrect (out) = %g,%g - %g,%g\n",userrect.p[0].x,userrect.p[0].y,userrect.p[1].x,userrect.p[1].y);
#endif
    new_width=(int)(userrect.p[1].x-userrect.p[0].x+.5);
    new_height=(int)(userrect.p[1].y-userrect.p[0].y+.5);
    /*
    new_width=devsize_pixels(k2settings->dst_userwidth,k2settings->dst_userwidth_units,
                             swidth_in,twidth_in,k2settings->dst_dpi,0);
    new_height=devsize_pixels(k2settings->dst_userheight,k2settings->dst_userheight_units,
                              sheight_in,theight_in,k2settings->dst_dpi,0);
    */
    pageno = masterinfo==NULL ? 1 : masterinfo->pageinfo.srcpage;
    maxpages = masterinfo==NULL ? 10 : masterinfo->srcpages;
    if (k2pdfopt_settings_landscape(k2settings,pageno,maxpages))
        int_swap(new_width,new_height)
    if (k2settings->devsize_set==1 || (k2settings->devsize_set>1 && (new_width!=k2settings->dst_width || new_height!=k2settings->dst_height)))
        {
        int width_change;

        width_change = (k2settings->devsize_set==1 || new_width != k2settings->dst_width);
        /* Flush master bitmap before changing it */
        if (width_change)
            {
            if (k2settings->devsize_set>1)
                masterinfo_flush(masterinfo,k2settings);
            }
        k2settings->dst_width=new_width;
        k2settings->dst_height=new_height;
        if (width_change)
            {
            masterinfo->bmp.width=k2settings->dst_width;
            /* dst_height*1.5*area_ratio */
            masterinfo->bmp.height=1.5*pagedims_inches.x*pagedims_inches.y*k2settings->dst_dpi*k2settings->dst_dpi/k2settings->dst_width;
            bmp_alloc(&masterinfo->bmp);
            bmp_fill(&masterinfo->bmp,255,255,255);
            masterinfo->rows=0;
            }
        if (pageinfo!=NULL && k2settings->use_crop_boxes)
            {
            pageinfo->width_pts = 72. * k2settings->dst_width / k2settings->dst_dpi;
            pageinfo->height_pts = 72. * k2settings->dst_height / k2settings->dst_dpi;
            if (k2pdfopt_settings_landscape(k2settings,pageno,maxpages))
                double_swap(pageinfo->width_pts,pageinfo->height_pts)
            }
        }
#if (WILLUSDEBUGX & 0x20000)
printf("dst_width = %d\n",k2settings->dst_width);
#endif
    {
    int dstmar_pixels[4];
    int dx_pixels;
    get_dest_margins(dstmar_pixels,k2settings,(double)k2settings->dst_dpi,
                     k2settings->dst_width,k2settings->dst_height);
    dx_pixels=dstmar_pixels[0]+dstmar_pixels[2];
    if ((double)(k2settings->dst_width-dx_pixels)/k2settings->dst_dpi < MIN_REGION_WIDTH_INCHES)
        {
        int olddpi;
        double dx_inches;
        olddpi = k2settings->dst_dpi;
        dx_inches = (double)dx_pixels/olddpi;
/*
printf("dstmargins (pixels)=%d,%d,%d,%d\n",dstmar_pixels[0],dstmar_pixels[1],
dstmar_pixels[2],dstmar_pixels[3]);
*/
        k2settings->dst_dpi = (int)(k2settings->dst_width/(MIN_REGION_WIDTH_INCHES+dx_inches));
        /* v2.36, 24 Nov 2016 -- don't allow < 1 */
        if (k2settings->dst_dpi < 1)
            k2settings->dst_dpi = 1;
#if (WILLUSDEBUGX & 0x20000)
printf("dst_width = %d\n",k2settings->dst_width);
printf("dst_dpi set to %d\n",k2settings->dst_dpi);
#endif
        if (!zeroarea)
            k2printf(TTEXT_BOLD2 "Output DPI reduced from %d to %d ... " TTEXT_NORMAL,
                olddpi,k2settings->dst_dpi);
        }
    }
    k2pdfopt_settings_set_region_widths(k2settings);
#if (WILLUSDEBUGX & 0x20000)
printf("After set_margins_and_devsize: dst_dpi=%d\n",k2settings->dst_dpi);
#endif
    }  

/*
** If in trim mode and effectively got a blank page, add blank rows and eject them
** (if also -bp).
** New in v2.36.
*/
int k2settings_trim_mode(K2PDFOPT_SETTINGS *k2settings)

    {
    return(k2settings->vertical_break_threshold==-2
            && k2settings->src_trim==1
            && k2settings->dst_fit_to_page==-2
            && k2settings->max_columns==1
            && k2settings->dst_break_pages==2);
    }


/*
** Returns NZ if the settings are such that gaps should not be added to the
** master bitmap (e.g. -f2p -2 or gridded output).
*/
int k2settings_gap_override(K2PDFOPT_SETTINGS *k2settings)

    {
    return(k2settings->dst_fit_to_page==-2
           || (k2settings->src_grid_cols > 0 && k2settings->src_grid_rows > 0));
    }


int k2settings_color_type(char *s)

    {
    int c;

    if (s[0]=='\0')
        return(0);
    /* Is it a bitmap? */
    if (wfile_status(s)==1)
        {
        WILLUSBITMAP *bmp,_bmp;
        int status;

        bmp=&_bmp;
        bmp_init(bmp);
        status=bmp_read(bmp,s,NULL);
        bmp_free(bmp);
        if (!status)
            return(3);
        }
    /* Array of colors? */
    c=in_string(s,",");
    if (c>0)
        return(4);
    c=hexcolor(s);
    if (((c&0xff0000)>>16)==((c&0xff00)>>8) && ((c&0xff00)>>8)==(c&0xff))
        return(1);
    return(2);
    }


int k2settings_ncolors(char *s)

    {
    int i,c;
    for (i=c=0;s[i]!='\0';i++)
        if (s[i]==',')
            c++;
    return(c+1);
    }


char *k2settings_color_by_index(char *s,int index)

    {
    int i,c;
    static char x[128];

    for (i=c=0;c<index && s[i]!='\0';i++)
        if (s[i]==',')
            c++;
    for (c=0;s[i]!='\0' && s[i]!=',' && c<127;i++)
        x[c++]=s[i];
    x[c]='\0';
    return(x);
    /* return(hexcolor(x)); */
    }

    
/*
void k2cropbox_set_default_values(K2CROPBOX *cbox,double value,int units)

    {
    int i;

    for (i=0;i<4;i++)
        {
        if (cbox->box[i]<0.)
            {
            cbox->box[i]=value;
            if (units>=0)
                cbox->units[i]=units;
            }
        }
    }
*/

/*
** Clear cropboxes that have flags set to flagtype
*/
void k2pdfopt_settings_clear_cropboxes(K2PDFOPT_SETTINGS *k2settings,int flagmask,int flagtype)

    {
    int i;
    K2CROPBOXES *boxes;

    boxes=&k2settings->cropboxes;
    for (i=0;i<boxes->n;i++)
        if ((boxes->cropbox[i].cboxflags&flagmask)==flagtype)
            boxes->cropbox[i].cboxflags |= K2CROPBOX_FLAGS_NOTUSED;
    }


void k2cropboxes_init(K2CROPBOXES *cropboxes)

    {
    int i,j;

    cropboxes->n=MAXK2CROPBOXES;
    for (i=0;i<cropboxes->n;i++)
        {
        K2CROPBOX *box;

        box=&cropboxes->cropbox[i];
        box->pagelist[0]='\0';
        for (j=0;j<4;j++)
            {
            box->box[j]=j<2?0.:-1.;
            box->units[j]=UNITS_INCHES;
            }
        box->cboxflags = K2CROPBOX_FLAGS_NOTUSED;
        }
    }


int k2cropboxes_count(K2CROPBOXES *cropboxes,int flagmask,int flagtype)

    {
    int i,n;

    for (n=i=0;i<cropboxes->n;i++)
        if ((cropboxes->cropbox[i].cboxflags&flagmask)==flagtype)
            n++;
    return(n);
    }


int k2settings_has_cropboxes(K2PDFOPT_SETTINGS *k2settings)

    {
    return(k2cropboxes_count(&k2settings->cropboxes,
                      K2CROPBOX_FLAGS_NOTUSED|K2CROPBOX_FLAGS_IGNOREBOXEDAREA,0)>0);
    }


int k2settings_need_color_initially(K2PDFOPT_SETTINGS *k2settings)

    {
    return(k2settings->dst_color 
             || (k2settings->pagebreakmark_breakpage_color >= 0)
             || (k2settings->pagebreakmark_nobreak_color >= 0));
    }


int k2settings_need_color_permanently(K2PDFOPT_SETTINGS *k2settings)

    {
    return(k2settings->dst_color || k2settings->show_marked_source);
    }


char *k2pdfopt_settings_unit_string(int units)

    {
    static char *strvals[] = {"","in","cm","s","t","x"};

    if (units==UNITS_INCHES)
        return(strvals[1]);
    else if (units==UNITS_CM)
        return(strvals[2]);
    else if (units==UNITS_SOURCE)
        return(strvals[3]);
    else if (units==UNITS_TRIMMED)
        return(strvals[4]);
    else if (units==UNITS_OCRLAYER)
        return(strvals[5]);
    else
        return(strvals[0]);
    }
