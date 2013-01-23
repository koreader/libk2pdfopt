/*
** k2settings.c     Handles k2pdfopt settings (K2PDFOPT_SETTINGS structure)
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

static void k2pdfopt_settings_set_device_margins(K2PDFOPT_SETTINGS *k2settings);
static void k2pdfopt_settings_set_crop_margins(K2PDFOPT_SETTINGS *k2settings);
static int devsize_pixels(double user_size,int user_units,double source_size_in,
                          double trimmed_size_in,double dst_dpi);


void k2pdfopt_settings_init(K2PDFOPT_SETTINGS *k2settings)

    {
    k2settings->verbose=0;
    k2settings->debug=0;
    k2settings->cdthresh=.01;
/*
** Blank Area Threshold Widths--average black pixel width, in inches, that
** prevents a region from being determined as "blank" or clear.
*/
    k2settings->src_rot=SRCROT_AUTO;
    k2settings->gtc_in=.005; // detecting gap between columns
    k2settings->gtr_in=.006; // detecting gap between rows
    k2settings->gtw_in=.0015; // detecting gap between words
    k2settings->show_usage=0;
    k2settings->src_left_to_right=1;
    k2settings->src_whitethresh=-1;
#ifdef HAVE_OCR_LIB
#ifdef HAVE_TESSERACT_LIB
    k2settings->dst_ocr_lang[0]='\0';
#endif
    k2settings->dst_ocr=0;
    k2settings->dst_ocr_visibility_flags=1;
    k2settings->ocr_max_height_inches=1.5;
#ifdef HAVE_TESSERACT_LIB
    k2settings->ocrtess_status=0;
#endif
    ocrwords_init(&k2settings->dst_ocrwords);
#endif
    k2settings->dst_dither=1;
    k2settings->dst_break_pages=0;
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
    k2settings->dst_display_resolution=1.0;
    k2settings->dst_justify=-1; // 0 = left, 1 = center
    k2settings->dst_figure_justify=-1; // -1 = same as dst_justify.  0=left 1=center 2=right
    k2settings->dst_min_figure_height_in=0.75;
    k2settings->dst_fulljustify=-1; // 0 = no, 1 = yes
    k2settings->dst_sharpen=1;
    k2settings->dst_bpc=4;
    k2settings->dst_landscape=0;
    strcpy(k2settings->dst_opname_format,"%s_k2opt");
    k2settings->src_autostraighten=0;
    k2settings->dst_mar=0.02;
    k2settings->dst_martop=-1.0;
    k2settings->dst_marbot=-1.0;
    k2settings->dst_marleft=-1.0;
    k2settings->dst_marright=-1.0;
    k2settings->min_column_gap_inches=0.1;
    k2settings->max_column_gap_inches=1.5; // max gap between columns
    k2settings->min_column_height_inches=1.5;
    k2settings->mar_top=-1.0;
    k2settings->mar_bot=-1.0;
    k2settings->mar_left=-1.0;
    k2settings->mar_right=-1.0;
    k2settings->max_region_width_inches = 3.6; /* Max viewable width (device width minus margins) */
    k2settings->max_columns=2;
    k2settings->column_gap_range=0.33;
    k2settings->column_offset_max=0.2;
    k2settings->column_row_gap_height_in=1./72.;
    k2settings->text_wrap=1;
    k2settings->word_spacing=0.375;
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
    k2settings->src_grid_overlap_percentage=2;
    k2settings->src_grid_order=0; /* 0=down then across, 1=across then down */
/*
** Undocumented cmd-line args
*/
    k2settings->no_wrap_ar_limit=0.2; /* -arlim */
    k2settings->no_wrap_height_limit_inches=0.55; /* -whmax */
    k2settings->little_piece_threshold_inches=0.5; /* -rwmin */
/*
** Keeping track of vertical gaps
*/
    k2settings->last_scale_factor_internal = -1.0;
    k2settings->line_spacing_internal=0;
    k2settings->last_rowbase_internal=0;
    k2settings->gap_override_internal=-1;


    k2pdfopt_settings_set_to_device(k2settings,devprofile_get("k2"));
    k2settings->dst_width = k2settings->dst_userwidth;
    k2settings->dst_height = k2settings->dst_userheight;
    }


int k2pdfopt_settings_set_to_device(K2PDFOPT_SETTINGS *k2settings,DEVPROFILE *dp)

    {
    if (dp==NULL)
        return(0);
    k2settings->dst_userwidth=dp->width;
    k2settings->dst_userwidth_units=UNITS_PIXELS;
    k2settings->dst_userheight=dp->height;
    k2settings->dst_userheight_units=UNITS_PIXELS;
    k2settings->dst_dpi=dp->dpi;
    k2settings->mark_corners=dp->mark_corners;
    k2settings->pad_left=dp->padding[0];
    k2settings->pad_top=dp->padding[1];
    k2settings->pad_right=dp->padding[2];
    k2settings->pad_bottom=dp->padding[3];
    k2settings->dst_color=dp->color;
    return(1);
    }


/*
** Check / adjust k2pdfopt user input settings.
**
** This function is called just once, right before doing any conversions.
**
*/
void k2pdfopt_settings_sanity_check(K2PDFOPT_SETTINGS *k2settings)

    {
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
    ** If text wrapping is on, can't use crop boxes
    */
    if (k2settings->text_wrap>0)
        k2settings->use_crop_boxes=0;

    /*
    ** If OCR is on, can't use crop boxes
    */
#ifdef HAVE_OCR_LIB
    if (k2settings->dst_ocr)
        k2settings->use_crop_boxes=0;
#endif

    /*
    ** Apply display resolution
    */
    k2settings->dst_dpi *= k2settings->dst_display_resolution;
    if (k2settings->dst_userwidth_units==UNITS_PIXELS && k2settings->dst_userwidth>0)
        k2settings->dst_userwidth *= k2settings->dst_display_resolution;
    if (k2settings->dst_userheight_units==UNITS_PIXELS && k2settings->dst_userheight>0)
        k2settings->dst_userheight *= k2settings->dst_display_resolution;

    /*
    ** With all parameters set, adjust output DPI so viewable region
    ** width >= MIN_REGION_WIDTH_INCHES
    ** NULL = first call, before any source page dimensions are known.
    ** Otherwise, bitmap region with source page dimensions set.
    */
    k2pdfopt_settings_set_margins_and_devsize(k2settings,NULL,NULL,0);
    /*
    ** Set source DPI
    */
    if (k2settings->dst_dpi < 20.)
        k2settings->dst_dpi = 20.;
    k2settings->src_dpi=k2settings->user_src_dpi < 0. ? (int)(fabs(k2settings->user_src_dpi)*k2settings->dst_dpi+.5) : (int)(k2settings->user_src_dpi+.5);
    if (k2settings->src_dpi < 50.)
        k2settings->src_dpi = 50.;

    k2pdfopt_settings_set_crop_margins(k2settings);
    }


/*
** Call this before each new document is processed.
*/
void k2pdfopt_settings_new_source_document_init(K2PDFOPT_SETTINGS *k2settings)

    {
    /* Reset usegs for each document */
    k2settings->usegs=k2settings->user_usegs;
    /* Init document word spacing history */
    word_gaps_add(NULL,0,NULL,0.);
#ifdef HAVE_OCR_LIB
    /* Init document OCR word list */
    if (k2settings->dst_ocr)
        {
        k2ocr_init(k2settings);
        ocrwords_clear(&k2settings->dst_ocrwords);
        }
#endif
    }


void k2pdfopt_settings_set_region_widths(K2PDFOPT_SETTINGS *k2settings)

    {
    k2settings->max_region_width_inches=k2settings->display_width_inches=(double)k2settings->dst_width/k2settings->dst_dpi;
    k2settings->max_region_width_inches -= (k2settings->dst_marleft+k2settings->dst_marright);
    }


void k2pdfopt_settings_restore_output_dpi(K2PDFOPT_SETTINGS *k2settings)

    {
    if (k2settings->column_fitted)
        {
        k2settings->dst_dpi=k2settings->dpi_org;
        k2settings->dst_marleft=k2settings->lm_org;
        k2settings->dst_marright=k2settings->rm_org;
        k2settings->dst_martop=k2settings->tm_org;
        k2settings->dst_marbot=k2settings->bm_org;
        k2pdfopt_settings_set_region_widths(k2settings);
        }
    k2settings->column_fitted=0;
    }


void k2pdfopt_settings_fit_column_to_screen(K2PDFOPT_SETTINGS *k2settings,
                                            double column_width_inches)

    {
    double text_width_pixels,lm_pixels,rm_pixels,tm_pixels,bm_pixels;

    if (!k2settings->column_fitted)
        {
        k2settings->dpi_org=k2settings->dst_dpi;
        k2settings->lm_org=k2settings->dst_marleft;
        k2settings->rm_org=k2settings->dst_marright;
        k2settings->tm_org=k2settings->dst_martop;
        k2settings->bm_org=k2settings->dst_marbot;
        }
    text_width_pixels = k2settings->max_region_width_inches*k2settings->dst_dpi;
    lm_pixels = k2settings->dst_marleft*k2settings->dst_dpi;
    rm_pixels = k2settings->dst_marright*k2settings->dst_dpi;
    tm_pixels = k2settings->dst_martop*k2settings->dst_dpi;
    bm_pixels = k2settings->dst_marbot*k2settings->dst_dpi;
    k2settings->dst_dpi = text_width_pixels / column_width_inches;
    k2settings->dst_marleft = lm_pixels / k2settings->dst_dpi;
    k2settings->dst_marright = rm_pixels / k2settings->dst_dpi;
    k2settings->dst_martop = tm_pixels / k2settings->dst_dpi;
    k2settings->dst_marbot = bm_pixels / k2settings->dst_dpi;
    k2pdfopt_settings_set_region_widths(k2settings);
    k2settings->column_fitted=1;
    }


static void k2pdfopt_settings_set_device_margins(K2PDFOPT_SETTINGS *k2settings)

    {
    if (k2settings->dst_mar<0.)
        k2settings->dst_mar=0.02;
    if (k2settings->dst_martop<0.)
        k2settings->dst_martop=k2settings->dst_mar;
    if (k2settings->dst_marbot<0.)
        k2settings->dst_marbot=k2settings->dst_mar;
    if (k2settings->dst_marleft<0.)
        k2settings->dst_marleft=k2settings->dst_mar;
    if (k2settings->dst_marright<0.)
        k2settings->dst_marright=k2settings->dst_mar;
    }


/*
** Set device width and height in pixels.
*/
void k2pdfopt_settings_set_margins_and_devsize(K2PDFOPT_SETTINGS *k2settings,
                                       BMPREGION *region,MASTERINFO *masterinfo,int trimmed)

    {
    static int count=0;
    static double wu=0.; /* Store untrimmed width, height */
    static double hu=0.;
    double swidth_in,sheight_in;
    int new_width,new_height,zeroarea;
    WPDFPAGEINFO *pageinfo;

    zeroarea=0;
    pageinfo=masterinfo!=NULL ? &masterinfo->pageinfo : NULL;
    if (region==NULL)
        {
        count=0;
        k2pdfopt_settings_set_device_margins(k2settings);
        swidth_in = 8.5;
        sheight_in = 11.0;
        }
    else
        {
        count++;
        if (trimmed)
            {
            swidth_in = (double)(region->c2-region->c1+1) / region->dpi;
            if (swidth_in < 1.0)
                swidth_in = 1.0;
            sheight_in = (double)(region->r2-region->r1+1) / region->dpi;
            if (sheight_in < 1.0)
                sheight_in = 1.0;
            if (region->c2-region->c1<=0 || region->r2-region->r1<=0)
                zeroarea=1;
            }
        else
            {
            swidth_in = (double)region->bmp->width / region->dpi;
            sheight_in = (double)region->bmp->height / region->dpi;
            }
        }
    if (trimmed)
        {
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
    new_width=devsize_pixels(k2settings->dst_userwidth,k2settings->dst_userwidth_units,
                             wu,swidth_in,k2settings->dst_dpi);
    new_height=devsize_pixels(k2settings->dst_userheight,k2settings->dst_userheight_units,
                              hu,sheight_in,k2settings->dst_dpi);
    if (k2settings->dst_landscape)
        int_swap(new_width,new_height)
    if (count==1 || (count>1 && (new_width!=k2settings->dst_width || new_height!=k2settings->dst_height)))
        {
        /* Flush master bitmap before changing it */
        if (count>1)
            masterinfo_flush(masterinfo,k2settings);
        k2settings->dst_width=new_width;
        k2settings->dst_height=new_height;
        masterinfo->bmp.width=k2settings->dst_width;
        /* dst_height*1.5*area_ratio */
        masterinfo->bmp.height=1.5*swidth_in*sheight_in*k2settings->dst_dpi*k2settings->dst_dpi/k2settings->dst_width;
        bmp_alloc(&masterinfo->bmp);
        bmp_fill(&masterinfo->bmp,255,255,255);
        masterinfo->rows=0;
        if (pageinfo!=NULL && k2settings->use_crop_boxes)
            {
            pageinfo->width_pts = 72. * k2settings->dst_width / k2settings->dst_dpi;
            pageinfo->height_pts = 72. * k2settings->dst_height / k2settings->dst_dpi;
            if (k2settings->dst_landscape)
                double_swap(pageinfo->width_pts,pageinfo->height_pts)
            }
        }
    if ((double)k2settings->dst_width/k2settings->dst_dpi - k2settings->dst_marleft - k2settings->dst_marright < MIN_REGION_WIDTH_INCHES)
        {
        int olddpi;
        olddpi = k2settings->dst_dpi;
        k2settings->dst_dpi = (int)((double)k2settings->dst_width/(MIN_REGION_WIDTH_INCHES+k2settings->dst_marleft+k2settings->dst_marright));
        if (!zeroarea)
            aprintf(TTEXT_BOLD2 "Output DPI reduced from %d to %d ... " TTEXT_NORMAL,
                olddpi,k2settings->dst_dpi);
        }
    k2pdfopt_settings_set_region_widths(k2settings);
    }


static void k2pdfopt_settings_set_crop_margins(K2PDFOPT_SETTINGS *k2settings)

    {
    double defval;

    defval=0.25;
    if (k2settings->mar_left < 0.)
        k2settings->mar_left=defval;
    if (k2settings->mar_right < 0.)
        k2settings->mar_right=defval;
    if (k2settings->mar_top < 0.)
        k2settings->mar_top=defval;
    if (k2settings->mar_bot < 0.)
        k2settings->mar_bot=defval;
    }


static int devsize_pixels(double user_size,int user_units,double source_size_in,
                          double trimmed_size_in,double dst_dpi)

    {
    
    if (user_size==0.)
        return((int)(source_size_in*dst_dpi+.5));
    if (user_size<0. || user_units==UNITS_SOURCE)
        return((int)(source_size_in*fabs(user_size)*dst_dpi+.5));
    if (user_units==UNITS_TRIMMED)
        return((int)(trimmed_size_in*user_size*dst_dpi+.5));
    if (user_units==UNITS_CM)
        return((int)((user_size/2.54)*dst_dpi+.5));
    if (user_units==UNITS_INCHES)
        return((int)(user_size*dst_dpi+.5));
    return((int)(user_size+.5));
    }
