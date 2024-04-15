/*
 ** k2pdfopt.h   K2pdfopt optimizes PDF/DJVU files for mobile e-readers
 **              (e.g. the Kindle) and smartphones. It works well on
 **              multi-column PDF/DJVU files. K2pdfopt is freeware.
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

#ifndef _CONTEXT_H
#define _CONTEXT_H

#include "k2pdfopt.h"
#include "leptonica.h"

typedef struct {
    float x0, y0;
    float x1, y1;
} BBox;

typedef struct KOPTContext {
    int trim;
    int wrap;
    int white;
    int indent;
    int rotate;
    int columns;
    int offset_x;
    int offset_y;
    int dev_dpi;
    int dev_width;
    int dev_height;
    int page_width;
    int page_height;
    int straighten;
    int justification;
    int read_max_width;
    int read_max_height;
    int writing_direction;

    double zoom;
    double margin;
    double quality;
    double contrast;
    double defect_size;
    double line_spacing;
    double word_spacing;
    double shrink_factor;

    int precache;
    int debug;
    int cjkchar;
    BOXA *rboxa;    // word boxes in reflowed page
    NUMA *rnai;     // word boxes indices in reflowed page
    BOXA *nboxa;    // word boxes in native page
    NUMA *nnai;     // word boxes indices in native page
    WRECTMAPS rectmaps; // rect maps between reflowed and native pages
    PAGEREGIONS pageregions; // sorted region list by display order
    BBox bbox;
    char *language;
    WILLUSBITMAP dst;
    WILLUSBITMAP src;

} KOPTContext;

#endif

