/*
** tesseract.h     Include file for willus.com Tesseract C API.
**                 Last udpated 9-1-12
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


#ifndef           _TESSERACT_H_
#define           _TESSERACT_H_

#include <leptonica.h>
#ifdef __cplusplus
extern "C" {
#endif

int tess_capi_init(char *datapath,char *language,int ocr_type,FILE *out);
int tess_capi_get_ocr(PIX *pix,char *outstr,int maxlen,FILE *out);
void tess_capi_end(void);

#ifdef __cplusplus
}
#endif

#endif
