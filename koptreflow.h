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

#ifndef _KOPTREADER_H
#define _KOPTREADER_H

#include "k2pdfopt.h"

typedef unsigned char  uint8_t;

typedef struct {
	float x0, y0;
	float x1, y1;
} BBox;

typedef struct KOPTContext {
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

	int precache;
	BBox bbox;
	uint8_t *data;
	WILLUSBITMAP *src;

} KOPTContext;

void k2pdfopt_reflow_bmp(KOPTContext *kctx);

#endif

