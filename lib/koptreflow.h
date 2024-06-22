/*
 ** koptreflow.h  page reflow api for koreader.
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

#ifndef _KOPTREFLOW_H
#define _KOPTREFLOW_H

#include "k2pdfopt.h"
#include "context.h"

K2PDFOPT_EXPORT
void k2pdfopt_reflow_bmp(KOPTContext *kctx);
K2PDFOPT_EXPORT
void pixmap_to_bmp(WILLUSBITMAP *bmp, unsigned char *pix_data, int ncomp);

#endif

