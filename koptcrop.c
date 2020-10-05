/*
 ** koptcrop.c   page crop api for koreader.
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
#include "koptcrop.h"

void k2pdfopt_crop_bmp(KOPTContext *kctx) {
	static char *funcname="k2pdfopt_crop_bmp";
	K2PDFOPT_SETTINGS _k2settings, *k2settings;
	MASTERINFO _masterinfo, *masterinfo;
	WILLUSBITMAP _srcgrey, *srcgrey;
	WILLUSBITMAP *src;
	BMPREGION _region, *region;
	int *colcount,*rowcount;
	int i,j;
	float margin;
    char initstr[256];

	src = &kctx->src;
	srcgrey = &_srcgrey;
	bmp_init(srcgrey);

	k2settings = &_k2settings;
	masterinfo = &_masterinfo;
	/* Initialize settings */
	k2pdfopt_settings_init_from_koptcontext(k2settings, kctx);
	k2pdfopt_settings_quick_sanity_check(k2settings);
	/* Init for new source doc */
	k2pdfopt_settings_new_source_document_init(k2settings,initstr);
	/* Init master output structure */
	masterinfo_init(masterinfo, k2settings);
	bmp_init(&masterinfo->bmp);
	masterinfo->bmp.width = 0;
	masterinfo->bmp.height = 0;
	//wrapbmp_free(&masterinfo->wrapbmp);
	//wrapbmp_init(&masterinfo->wrapbmp, k2settings->dst_color);
	/* Init new source bitmap */
	region = &_region;
	bmpregion_init(region);
	masterinfo_new_source_page_init(masterinfo, k2settings, src, srcgrey, NULL,
			region, k2settings->src_rot, NULL, NULL, 1, -1, NULL);
	//printf("source page (%d,%d) - (%d,%d)\n",region->c1,region->r1,region->c2,region->r2);
	//printf("source page bgcolor %d\n", region->bgcolor);
	bmpregion_trim_margins(region,k2settings,0xf);
	margin = kctx->margin*k2settings->dst_dpi;
	/*
	 * Suppose when page is zoomed to k level at fit to content width zoom mode,
	 * the content width is a, the device screen width is w,
	 * and the request margin size is m, x pixels are added as page margin.
	 * Then the following equations should be satisified:
	 * 1. k*(a + 2x) = w
	 * 2. 2*x*k = m
	 * which should be solved to x = m*a/(w-m)/2
	*/
	margin = margin*(region->c2 - region->c1)/(kctx->dev_width - margin)/2;

	kctx->bbox.x0 = (float)region->c1 - margin;
	kctx->bbox.y0 = (float)region->r1 - margin;
	kctx->bbox.x1 = (float)region->c2 + margin;
	kctx->bbox.y1 = (float)region->r2 + margin;

	bmp_free(src);
	bmp_free(srcgrey);
	bmpregion_free(region);
	masterinfo_free(masterinfo,k2settings);
}
