/*
 ** koptreflow.h  page OCR api for koreader.
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

#ifndef _KOPTOCR_H
#define _KOPTOCR_H

#include "context.h"
#include "k2pdfopt.h"
#include "leptonica.h"
#include "tesseract.h"

void k2pdfopt_tocr_single_word(WILLUSBITMAP *src,
		int x, int y, int w, int h,
		char *word, int max_length,
		char *datadir, char *lang, char *ocr_type,
		int allow_spaces, int std_proc);

void k2pdfopt_get_word_boxes(KOPTContext *kctx, WILLUSBITMAP *src,
		int x, int y, int w, int h,
		int reduction, int min_w, int min_h, int max_w, int max_h);

#endif

