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

void* tess_api = NULL;

void k2pdfopt_tocr_init(char *datadir, char *lang) {
	if (tess_api != NULL && strncmp(lang, k2pdfopt_tocr_get_language(tess_api), 32)) {
		k2pdfopt_tocr_end(tess_api);
	}
	if (tess_api == NULL) {
		int status;
		tess_api = ocrtess_init(datadir, lang, NULL, NULL, 0, &status);
		if (tess_api == NULL) {
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
	if (tess_api != NULL) {
		ocrtess_single_word_from_bmp8(tess_api,
				word, max_length, src,
				x, y, x + w, y + h,
				ocr_type, allow_spaces, std_proc, stderr);
	}
}

const char* k2pdfopt_tocr_get_language() {
	return tess_capi_get_init_language(tess_api);
}

void k2pdfopt_tocr_end() {
	if (tess_api != NULL) {
		ocrtess_end(tess_api);
		tess_api = NULL;
	}
}

void k2pdfopt_get_word_boxes(KOPTContext *kctx, WILLUSBITMAP *src,
		int x, int y, int w, int h, int box_type) {
	static K2PDFOPT_SETTINGS _k2settings, *k2settings;
	PIX *pixs, *pixt, *pixb;
	int words;
	BOXA **pboxa;
	NUMA **pnai;

	k2settings = &_k2settings;
	/* Initialize settings */
	k2pdfopt_settings_init_from_koptcontext(k2settings, kctx);
	k2pdfopt_settings_quick_sanity_check(k2settings);
	/* Init for new source doc */
	k2pdfopt_settings_new_source_document_init(k2settings);

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
			if (k2pdfopt_get_word_boxes_from_tesseract(pixs, kctx->cjkchar,
					pboxa, pnai) != 0) {
				printf("failed to get word boxes from tesseract\n");
			}
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

	if (tess_capi_get_word_boxes(tess_api, pixs, &boxa, is_cjk, stderr) != 0) {
		*pboxad = NULL;
		*pnai = NULL;
		return ERROR_INT("Tesseract failed to get word boxes", procName, 1);
	}
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
	BOXA *boxa1, *boxad;
	BOXAA *baa;
	NUMA *nai;
	NUMAA *naa;
	PIX *pix1;
	PIXA *pixa1, *pixad;
	PIXAA *paa;

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
		pix1 = pixClone(pixs);
	} else { /* reduction == 2 */
		pix1 = pixReduceRankBinaryCascade(pixs, 1, 0, 0, 0);
		maxsize = maxsize / 2;
	}

    /* Get the bounding boxes of the words from the word mask. */
    pixWordBoxesByDilation(pix1, maxsize, minwidth, minheight,
			maxwidth, maxheight, &boxa1, NULL);
	/* Generate a pixa of the word images */
	pixa1 = pixaCreateFromBoxa(pix1, boxa1, NULL);  /* mask over each word */
	/* Sort the bounding boxes of these words by line.  We use the
	* index mapping to allow identical sorting of the pixa. */
	baa = boxaSort2d(boxa1, &naa, -1, -1, 4);
	paa = pixaSort2dByIndex(pixa1, naa, L_CLONE);
	/* Flatten the word paa */
	pixad = pixaaFlattenToPixa(paa, &nai, L_CLONE);
	boxad = pixaGetBoxa(pixad, L_COPY);

	*pnai = nai;
	*pboxad = boxad;

	pixDestroy(&pix1);
	pixaDestroy(&pixa1);
	pixaDestroy(&pixad);
	boxaDestroy(&boxa1);
	pixaaDestroy(&paa);
	boxaaDestroy(&baa);
	numaaDestroy(&naa);
	return 0;
}
