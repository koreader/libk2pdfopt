/*
 ** koptimize.c  page optimize api for koreader.
 **
 **
 ** Copyright (C) 2014  http://willus.com
 ** Copyright (C) 2014  chrox
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

#include "setting.h"
#include "koptimize.h"

void k2pdfopt_optimize_bmp(KOPTContext *kctx) {
    K2PDFOPT_SETTINGS _k2settings, *k2settings;
    MASTERINFO _masterinfo, *masterinfo;
    WILLUSBITMAP _srcgrey, *srcgrey;
    WILLUSBITMAP *src, *dst;
    BMPREGION region;
    int i, bw;
    char initstr[256];

    src = &kctx->src;
    srcgrey = &_srcgrey;
    bmp_init(srcgrey);

    k2settings = &_k2settings;
    masterinfo = &_masterinfo;
    /* Initialize settings */
    k2pdfopt_settings_init_from_koptcontext(k2settings, kctx);
    /* Init for new source doc */
    k2pdfopt_settings_new_source_document_init(k2settings, initstr);
    /* Additional settings for optimization */
    k2settings->text_wrap=0;
    k2settings->max_columns=1;
    k2settings->vertical_break_threshold=-2;
    k2settings->src_dpi=kctx->dev_dpi;
    k2settings->dst_userwidth=1.0;
    k2settings->dst_userwidth_units=UNITS_SOURCE;
    k2settings->dst_userheight=1.0;
    k2settings->dst_userheight_units=UNITS_SOURCE;
    k2settings->dst_fit_to_page=-2;
    k2settings->src_trim=0;
    for (i=0;i<4;i++)
        k2settings->dstmargins.box[i]=.0;
    k2settings->pad_left=k2settings->pad_top=k2settings->pad_bottom=k2settings->pad_right=0;
    k2settings->mark_corners=0;
    k2pdfopt_settings_quick_sanity_check(k2settings);
    /* Init master output structure */
    masterinfo_init(masterinfo, k2settings);
    wrapbmp_init(&masterinfo->wrapbmp, k2settings->dst_color);
    /* Init new source bitmap */
    bmpregion_init(&region);
    masterinfo_new_source_page_init(masterinfo, k2settings, src, srcgrey, NULL,
            &region, k2settings->src_rot, NULL, NULL, 1, -1, NULL );
    /* Set output size */
    k2pdfopt_settings_set_margins_and_devsize(k2settings,&region,masterinfo,-1.,0);
    /* Process single source page */
    bmpregion_source_page_add(&region, k2settings, masterinfo, 1, 0);
    wrapbmp_flush(masterinfo, k2settings, 0);

    if (fabs(k2settings->dst_gamma - 1.0) > .001)
        bmp_gamma_correct(&masterinfo->bmp, &masterinfo->bmp,
                k2settings->dst_gamma);

    /* copy master bitmap to context dst bitmap */
    dst = &kctx->dst;
    dst->bpp = masterinfo->bmp.bpp;
    dst->width = masterinfo->bmp.width;
    dst->height = masterinfo->rows;
    bmp_alloc(dst);
    bmp_fill(dst, 255, 255, 255);
    bw = bmp_bytewidth(&masterinfo->bmp);
    for (i = 0; i < masterinfo->rows; i++)
        memcpy(bmp_rowptr_from_top(dst, i),
                bmp_rowptr_from_top(&masterinfo->bmp, i), bw);

    kctx->page_width = kctx->dst.width;
    kctx->page_height = kctx->dst.height;

    bmp_free(src);
    bmp_free(srcgrey);
    bmpregion_free(&region);
    masterinfo_free(masterinfo, k2settings);
}
