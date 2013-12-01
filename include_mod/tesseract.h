/*
** Willus.com's Tesseract C Wrappers
**
** 6-8-12
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
