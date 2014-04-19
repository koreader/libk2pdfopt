/*
 ** koptreader.c  Using k2pdfopt library from KindlePDFViewer.
 **
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

#include "setting.h"

void k2pdfopt_settings_init_from_koptcontext(K2PDFOPT_SETTINGS *k2settings, KOPTContext *kctx)

{
    /* Generic settings init */
    k2pdfopt_settings_init(k2settings);
    k2settings->verbose = 0;
    k2settings->debug = 0;
    k2settings->src_rot = 0;
    k2settings->dst_dpi = 167;
    k2settings->dst_userwidth = 600;
    k2settings->dst_userheight = 800;
    k2settings->dst_width = k2settings->dst_userwidth;
    k2settings->dst_height = k2settings->dst_userheight;
    k2settings->dst_color = 0;
    k2settings->dst_mar = 0.06;
    k2settings->dst_martop = -1.0;
    k2settings->dst_marbot = -1.0;
    k2settings->dst_marleft = -1.0;
    k2settings->dst_marright = -1.0;
    k2settings->use_crop_boxes = 0;
    k2settings->defect_size_pts = 1.0;

    /* Apply context */
    k2settings->dst_dpi = kctx->dev_dpi;
    k2settings->render_dpi = kctx->dev_dpi;
    k2settings->dst_userwidth = kctx->dev_width;
    k2settings->dst_userheight = kctx->dev_height;
    k2settings->dst_width = k2settings->dst_userwidth;
    k2settings->dst_height = k2settings->dst_userheight;
    k2settings->vertical_line_spacing = kctx->line_spacing;
    k2settings->text_wrap = kctx->wrap;
    k2settings->src_whitethresh = kctx->white;
    k2settings->src_autostraighten = kctx->straighten;
    k2settings->preserve_indentation = kctx->indent;
    k2settings->max_columns = kctx->columns;
    k2settings->src_dpi = kctx->dev_dpi*kctx->quality;
    k2settings->user_src_dpi = kctx->dev_dpi*kctx->quality;
    k2settings->defect_size_pts = kctx->defect_size;
    k2settings->dst_gamma = kctx->contrast;

    if (kctx->writing_direction == 0)
        k2settings->src_left_to_right = 1;
    else if (kctx->writing_direction == 1)
        k2settings->src_left_to_right = 0;
    else if (kctx->writing_direction == 2) {
        k2settings->src_left_to_right = 1;
        k2settings->src_rot = 90;
    } else if (kctx->writing_direction == 3) {
        k2settings->src_left_to_right = 0;
        k2settings->src_rot = 270;
    }

    if (kctx->word_spacing < 0) {
        k2settings->auto_word_spacing = 1;
        k2settings->word_spacing = 0.15;
    } else {
        k2settings->auto_word_spacing = 0;
        k2settings->word_spacing = kctx->word_spacing;
    }

    if (kctx->trim == 0) {
        k2settings->mar_left = 0;
        k2settings->mar_top = 0;
        k2settings->mar_right = 0;
        k2settings->mar_bot = 0;
    } else {
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
    if (kctx->justification < 0) {
        k2settings->dst_justify = -1;
        k2settings->dst_fulljustify = -1;
    } else if (kctx->justification <= 2) {
        k2settings->dst_justify = kctx->justification;
        k2settings->dst_fulljustify = 0;
    } else {
        k2settings->dst_justify = -1;
        k2settings->dst_fulljustify = 1;
    }

    // eliminate hyphens for non-CJK characters
    if (kctx->cjkchar)
        k2settings->hyphen_detect = 0;
    else
        k2settings->hyphen_detect = 1;
}
