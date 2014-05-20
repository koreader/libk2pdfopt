/*
 ** koptreflow.c  page OCR api for koreader.
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

#include <assert.h>
#include "setting.h"
#include "koptocr.h"

PIX* bitmap2pix(WILLUSBITMAP *src, int x, int y, int w, int h);
l_int32 k2pdfopt_pixGetWordBoxesInTextlines(PIX *pixs, l_int32 maxsize,
		l_int32 reduction, l_int32 minwidth, l_int32 minheight,
		l_int32 maxwidth, l_int32 maxheight, BOXA **pboxad, NUMA **pnai);

static int k2pdfopt_tocr_inited = 0;

void k2pdfopt_tocr_init(char *datadir, char *lang) {
	if (strncmp(lang, k2pdfopt_tocr_get_language(), 32)) {
		k2pdfopt_tocr_end();
	}
	if (!k2pdfopt_tocr_inited) {
		if (ocrtess_init(datadir, lang, NULL) == 0 ) {
				k2pdfopt_tocr_inited = 1;
		} else {
			printf("fail to start tesseract OCR engine\n");
		}
	}
}

void k2pdfopt_tocr_single_word(WILLUSBITMAP *src,
		int x, int y, int w, int h,
		char *word, int max_length,
		char *datadir, char *lang, int ocr_type,
		int allow_spaces, int std_proc) {
	k2pdfopt_tocr_init(datadir, lang);
	if (k2pdfopt_tocr_inited) {
		ocrtess_single_word_from_bmp8(
				word, max_length, src,
				x, y, x + w, y + h,
				ocr_type, allow_spaces, std_proc, stderr);
	}
}

const char* k2pdfopt_tocr_get_language() {
	return tess_capi_get_init_language();
}

void k2pdfopt_tocr_end() {
	if (k2pdfopt_tocr_inited) {
		ocrtess_end();
		k2pdfopt_tocr_inited = 0;
	}
}

void k2pdfopt_get_word_boxes(KOPTContext *kctx, WILLUSBITMAP *src,
		int x, int y, int w, int h, int box_type) {
	static K2PDFOPT_SETTINGS _k2settings, *k2settings;
	PIX *pixs, *pixt, *pixb;
	int words;
	BOXA **pboxa;
	NUMA **pnai;

	/* Initialize settings */
	k2settings = &_k2settings;
	k2pdfopt_settings_init_from_koptcontext(k2settings, kctx);
	k2pdfopt_settings_sanity_check(k2settings);

	if (box_type == 0) {
        pboxa = &kctx->rboxa;
        pnai = &kctx->rnai;
    } else if (box_type == 1) {
        pboxa = &kctx->nboxa;
        pnai = &kctx->nnai;
    }

	if (*pboxa == NULL && *pnai == NULL && src->bpp == 8) {
		assert(x + w <= src->width);
		assert(y + h <= src->height);
		pixs = bitmap2pix(src, x, y, w, h);
		if (kctx->cjkchar) {
			k2pdfopt_get_word_boxes_from_tesseract(pixs, kctx->cjkchar,
					pboxa, pnai);
		} else {
			pixb = pixConvertTo1(pixs, 128);
			k2pdfopt_pixGetWordBoxesInTextlines(pixb,
					7*kctx->dev_dpi/150, 1, 10, 10, 300, 100,
					pboxa, pnai);
			pixDestroy(&pixb);
		}

		if (kctx->debug == 1) {
			//pixt = pixDrawBoxaRandom(pixs, kctx->boxa, 2);
			//pixWrite("junkpixt", pixt, IFF_PNG);
			//pixDestroy(&pixt);
		}
		pixDestroy(&pixs);
	}
}

void k2pdfopt_get_reflowed_word_boxes(KOPTContext *kctx, WILLUSBITMAP *src,
        int x, int y, int w, int h) {
    k2pdfopt_get_word_boxes(kctx, src, x, y, w, h, 0);
}

void k2pdfopt_get_native_word_boxes(KOPTContext *kctx, WILLUSBITMAP *src,
        int x, int y, int w, int h) {
    k2pdfopt_get_word_boxes(kctx, src, x, y, w, h, 1);
}

PIX* bitmap2pix(WILLUSBITMAP *src, int x, int y, int w, int h) {
	PIX* pix = pixCreate(w, h, 8);
	unsigned char *rpt = src->data;
	int i, j;
	for (i = y; i < y + h; ++i) {
		l_uint32 *lwpt = pixGetData(pix) + i * pixGetWpl(pix);
		for (j = x; j < x + w; ++j) {
			SET_DATA_BYTE(lwpt, j, *rpt++);
		}
	}
	return pix;
}

int k2pdfopt_get_word_boxes_from_tesseract(PIX *pixs, int is_cjk,
		BOXA **pboxad, NUMA **pnai) {
	BOXA *boxa, *boxad;
	BOXAA *baa;
	NUMA *nai;

	PROCNAME("k2pdfopt_get_word_boxes_from_tesseract");

	if (!pboxad || !pnai)
		return ERROR_INT("&boxad and &nai not both defined", procName, 1);
	*pboxad = NULL;
	*pnai = NULL;
	if (!pixs)
		return ERROR_INT("pixs not defined", procName, 1);

	tess_capi_get_word_boxes(pixs, &boxa, is_cjk, stderr);
	/* 2D sort the bounding boxes of these words. */
	baa = boxaSort2d(boxa, NULL, 3, -5, 5);

	/* Flatten the boxaa, saving the boxa index for each box */
	boxad = boxaaFlattenToBoxa(baa, &nai, L_CLONE);

	*pnai = nai;
	*pboxad = boxad;

	boxaDestroy(&boxa);
	boxaaDestroy(&baa);
	return 0;
}

// modified version of leptonica pixGetWordBoxesInTextlines
// adding maxsize parameter
l_int32 k2pdfopt_pixGetWordBoxesInTextlines(PIX *pixs,
		l_int32 maxsize, l_int32 reduction,
		l_int32 minwidth, l_int32 minheight,
		l_int32 maxwidth, l_int32 maxheight,
		BOXA **pboxad, NUMA **pnai) {
	BOXA *boxa1, *boxa2, *boxa3, *boxad;
	BOXAA *baa;
	NUMA *nai;
	PIX *pixt1, *pixt2;

	PROCNAME("k2pdfopt_pixGetWordBoxesInTextlines");

	if (!pboxad || !pnai)
		return ERROR_INT("&boxad and &nai not both defined", procName, 1);
	*pboxad = NULL;
	*pnai = NULL;
	if (!pixs)
		return ERROR_INT("pixs not defined", procName, 1);
	if (reduction != 1 && reduction != 2)
		return ERROR_INT("reduction not in {1,2}", procName, 1);

	if (reduction == 1) {
		pixt1 = pixClone(pixs);
	} else { /* reduction == 2 */
		pixt1 = pixReduceRankBinaryCascade(pixs, 1, 0, 0, 0);
		maxsize = maxsize / 2;
	}

	/* First estimate of the word masks */
	pixt2 = pixWordMaskByDilation(pixt1, maxsize, NULL);

	/* Get the bounding boxes of the words, and remove the
	 * small ones, which can be due to punctuation that was
	 * not joined to a word, and the large ones, which are
	 * also not likely to be words. */
	boxa1 = pixConnComp(pixt2, NULL, 8);
	boxa2 = boxaSelectBySize(boxa1, minwidth, minheight, L_SELECT_IF_BOTH,
			L_SELECT_IF_GTE, NULL);
	boxa3 = boxaSelectBySize(boxa2, maxwidth, maxheight, L_SELECT_IF_BOTH,
			L_SELECT_IF_LTE, NULL);

	/* 2D sort the bounding boxes of these words. */
	baa = boxaSort2d(boxa3, NULL, 3, -5, 5);

	/* Flatten the boxaa, saving the boxa index for each box */
	boxad = boxaaFlattenToBoxa(baa, &nai, L_CLONE);

	*pnai = nai;
	*pboxad = boxad;

	pixDestroy(&pixt1);
	pixDestroy(&pixt2);
	boxaDestroy(&boxa1);
	boxaDestroy(&boxa2);
	boxaDestroy(&boxa3);
	boxaaDestroy(&baa);
	return 0;
}
