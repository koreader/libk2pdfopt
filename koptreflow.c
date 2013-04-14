/*
 ** koptreflow.c  page reflow api for koreader.
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
#include "koptreflow.h"

void k2pdfopt_reflow_bmp(KOPTContext *kctx) {
	static K2PDFOPT_SETTINGS _k2settings, *k2settings;
	static MASTERINFO _masterinfo, *masterinfo;
	static int master_bmp_inited = 0;
	WILLUSBITMAP _srcgrey, *srcgrey;
	WILLUSBITMAP *src;
	BMPREGION region;
	int initgap;

	src = kctx->src;
	srcgrey = &_srcgrey;
	bmp_init(srcgrey);

	k2settings = &_k2settings;
	masterinfo = &_masterinfo;
	/* Initialize settings */
	k2pdfopt_settings_init_from_koptcontext(k2settings, kctx);
	k2pdfopt_settings_sanity_check(k2settings);
	/* Init master output structure */
	if (master_bmp_inited == 0) {
		masterinfo_init(masterinfo, k2settings);
		master_bmp_inited = 1;
	}
	bmp_free(&masterinfo->bmp);
	bmp_init(&masterinfo->bmp);
	masterinfo->bmp.width = 0;
	masterinfo->bmp.height = 0;
	wrapbmp_free(&masterinfo->wrapbmp);
	wrapbmp_init(&masterinfo->wrapbmp, k2settings->dst_color);
	/* Init new source bitmap */
	masterinfo_new_source_page_init(masterinfo, k2settings, src, srcgrey, NULL,
			&region, k2settings->src_rot, NULL, NULL, 1, NULL);
	/* Process single source page */
	bmpregion_source_page_add(&region, k2settings, masterinfo, 1, 0, (int)(0.25*k2settings->src_dpi+.5));
	wrapbmp_flush(masterinfo,k2settings,0,0);

	bmp_free(src);
	bmp_free(srcgrey);

	if (fabs(k2settings->dst_gamma - 1.0) > .001)
		bmp_gamma_correct(&masterinfo->bmp, &masterinfo->bmp,
				k2settings->dst_gamma);

	kctx->page_width = masterinfo->bmp.width;
	kctx->page_height = masterinfo->rows;
	kctx->data = masterinfo->bmp.data;
	kctx->precache = 0;
}
