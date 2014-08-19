/*
 ** koptpart.c   page partition api for koreader.
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
#include "koptpart.h"

void k2pdfopt_part_bmp(KOPTContext *kctx) {
	static char *funcname="k2pdfopt_part_bmp";
	K2PDFOPT_SETTINGS _k2settings, *k2settings;
	MASTERINFO _masterinfo, *masterinfo;
	WILLUSBITMAP _srcgrey, *srcgrey;
	WILLUSBITMAP *src;
	BMPREGION _region, *region;
	int i,j;
	float margin;

	src = &kctx->src;
	srcgrey = &_srcgrey;
	bmp_init(srcgrey);

	k2settings = &_k2settings;
	masterinfo = &_masterinfo;
	/* Initialize settings */
	k2pdfopt_settings_init_from_koptcontext(k2settings, kctx);
	for (i=0;i<4;i++)
		k2settings->dstmargins.box[i]=.0;
	k2settings->text_wrap=0;
	k2settings->src_trim=0;
	k2pdfopt_settings_sanity_check(k2settings);
	/* Init master output structure */
	masterinfo_init(masterinfo, k2settings);
	bmp_init(&masterinfo->bmp);
	masterinfo->bmp.width = 0;
	masterinfo->bmp.height = 0;
	/* Init new source bitmap */
	region = &_region;
	bmpregion_init(region);
	masterinfo_new_source_page_init(masterinfo, k2settings, src, srcgrey, NULL,
			region, k2settings->src_rot, NULL, NULL, 1, NULL);
	int maxlevels;
	if (k2settings->max_columns<=1)
		maxlevels=1;
	else if (k2settings->max_columns<=2)
		maxlevels=2;
	else
		maxlevels=3;
	pageregions_init(&kctx->pageregions);
	pageregions_find_columns(&kctx->pageregions,region,k2settings,masterinfo,maxlevels);

	bmp_free(src);
	bmp_free(srcgrey);
	bmpregion_free(region);
	masterinfo_free(masterinfo,k2settings);
}
