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

#ifndef _K2PDFOPT_H
#define _K2PDFOPT_H

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

	uint8_t *data;
	BBox bbox;
} KOPTContext;

typedef struct {
	int red[256];
	int green[256];
	int blue[256];
	unsigned char *data; /* Top to bottom in native type, bottom to */
	/* top in Win32 type.                      */
	int width; /* Width of image in pixels */
	int height; /* Height of image in pixels */
	int bpp; /* Bits per pixel (only 8 or 24 allowed) */
	int size_allocated;
	int type; /* See defines above for WILLUSBITMAP_TYPE_... */
} WILLUSBITMAP;

/* bmp utilities */
void bmp_init(WILLUSBITMAP *bmap);
int bmp_alloc(WILLUSBITMAP *bmap);
void bmp_free(WILLUSBITMAP *bmap);
int bmp_bytewidth(WILLUSBITMAP *bmp);
unsigned char *bmp_rowptr_from_top(WILLUSBITMAP *bmp, int row);

void k2pdfopt_reflow_bmp(KOPTContext *kctx, WILLUSBITMAP *bmp);

#endif

