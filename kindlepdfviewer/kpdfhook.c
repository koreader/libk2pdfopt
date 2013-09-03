/*
** kpdfhook.c    Example of how to use k2pdfopt library.
**               (Can be compiled with no third-party lib dependencies.)
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

/*
** In willus.h, search for "THIRD PARTY" and then comment out all third
** party library macros, e.g. comment out HAVE_Z_LIB, HAVE_PNG_LIB, etc.
**
** In k2pdfopt.h, uncomment the #define K2PDFOPT_KINDLEPDFVIEWER statement.
**
*/
#include "k2pdfopt.h"

typedef unsigned char uint8_t;

typedef struct
    {
    float x0, y0;
    float x1, y1;
    } BBox;

typedef struct
    {
    int trim;
    int wrap;
    int indent;
    int rotate;
    int columns;
    int offset_x;
    int offset_y;
    int dev_width;
    int dev_height;
    int page_width;
    int page_height;
    int straighten;
    int justification;
    int read_max_width;
    int read_max_height;

    double zoom;
    double margin;
    double quality;
    double contrast;
    double defect_size;
    double line_spacing;
    double word_spacing;
    double shrink_factor;

    uint8_t *data;
    BBox bbox;
    } KOPTContext;


void k2pdfopt_reflow_bmp(KOPTContext *kctx,WILLUSBITMAP *src,int init);
static void k2pdfopt_settings_init_from_koptcontext(K2PDFOPT_SETTINGS *k2settings,
                                                    KOPTContext *kctx);

void k2pdfopt_reflow_bmp(KOPTContext *kctx,WILLUSBITMAP *src,int init)

    {
    static K2PDFOPT_SETTINGS _k2settings, *k2settings;
    static MASTERINFO _masterinfo, *masterinfo;
    WILLUSBITMAP _srcgrey, *srcgrey;
    BMPREGION region;
    static int pages_done=0;

    srcgrey=&_srcgrey;
    bmp_init(srcgrey);

    k2settings=&_k2settings;
    masterinfo=&_masterinfo;
    /* First call or new document call should use init =  1 */
    if (init==1)
        {
        /* Initialize settings */
        k2pdfopt_settings_init_from_koptcontext(k2settings,kctx);
        k2pdfopt_settings_sanity_check(k2settings);

        /* Init master output structure */
        masterinfo_init(masterinfo,k2settings);

        /* Init for new source doc */
        k2pdfopt_settings_new_source_document_init(k2settings);
        }

    /* Init new source bitmap */
    bmpregion_init(&region);
    masterinfo_new_source_page_init(masterinfo,k2settings,src,srcgrey,NULL,&region,0.,NULL,NULL,1,NULL);

    /* Process single source page */
    if (init)
        pages_done=0;
    bmpregion_source_page_add(&region,k2settings,masterinfo,1,pages_done++);
    bmp_free(srcgrey);

    /*
    ** Get output pages
    */ 
    {
    WILLUSBITMAP *bmp,_bmp;
    int rows,size_reduction;
    double bmpdpi;

    bmp=&_bmp;
    bmp_init(bmp);
    while ((rows=masterinfo_get_next_output_page(masterinfo,k2settings,1,bmp,
                                                 &bmpdpi,&size_reduction,NULL))>0)
        {
        /* Process output page stored in "bmp" */
        }
    bmp_free(bmp);
    }

    /* Final call should use init = -999 */
    if (init == -999)
        masterinfo_free(masterinfo,k2settings);
    }


static void k2pdfopt_settings_init_from_koptcontext(K2PDFOPT_SETTINGS *k2settings,
                                                    KOPTContext *kctx)

    {
    /* Generic settings init */
    k2pdfopt_settings_init(k2settings);
    k2settings->src_rot=0;
    k2settings->user_src_dpi=300;
    k2settings->dst_dpi=167;
    k2settings->dst_userwidth=600;
    k2settings->dst_userheight=800;
    k2settings->dst_width = k2settings->dst_userwidth;
    k2settings->dst_height = k2settings->dst_userheight;
    k2settings->dst_color=0;
    k2settings->dst_mar=0.06;
    k2settings->dst_martop=-1.0;
    k2settings->dst_marbot=-1.0;
    k2settings->dst_marleft=-1.0;
    k2settings->dst_marright=-1.0;
    k2settings->use_crop_boxes=0;
    k2settings->defect_size_pts=1.0;

    /* Apply context */
    k2settings->dst_userwidth = kctx->dev_width;
    k2settings->dst_userheight = kctx->dev_height;
    k2settings->vertical_line_spacing = kctx->line_spacing;
    k2settings->word_spacing = kctx->word_spacing;
    k2settings->text_wrap = kctx->wrap;
    k2settings->src_autostraighten = kctx->straighten;
    k2settings->preserve_indentation = kctx->indent;
    k2settings->max_columns = kctx->columns;
    k2settings->src_rot = kctx->rotate;
    k2settings->src_dpi = (int)300*kctx->quality;
    k2settings->defect_size_pts = kctx->defect_size;
    k2settings->dst_gamma = kctx->contrast;

    if (kctx->trim == 0)
        {
        k2settings->mar_left = 0;
        k2settings->mar_top = 0;
        k2settings->mar_right = 0;
        k2settings->mar_bot = 0;
        }
    else
        {
        k2settings->mar_left = -1;
        k2settings->mar_top = -1;
        k2settings->mar_right = -1;
        k2settings->mar_bot = -1;
        }

    // margin
    k2settings->dst_mar = kctx->margin;
    k2settings->dst_martop = -1.0;
    k2settings->dst_marbot = -1.0;
    k2settings->dst_marleft = -1.0;
    k2settings->dst_marright = -1.0;

    // justification
    if (kctx->justification < 0)
        {
        k2settings->dst_justify = -1;
        k2settings->dst_fulljustify = -1;
        }
    else if (kctx->justification <= 2)
        {
        k2settings->dst_justify = kctx->justification;
        k2settings->dst_fulljustify = 0;
        }
    else
        {
        k2settings->dst_justify = -1;
        k2settings->dst_fulljustify = 1;
        }
    }
