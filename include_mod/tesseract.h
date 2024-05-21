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

void *tess_capi_init(char *datapath,char *language,int ocr_type,FILE *out,
                    char *initstr,int maxlen,int *status);
int tess_capi_get_ocr(void *api,PIX *pix,char *outstr,int maxlen,int segmode,FILE *out);
int tess_capi_get_ocr_multiword(void *vapi,PIX *pix,int segmode,
                                int **left,int **top,int **right,int **bottom,
                                int **ybase,char **text,int *nw,
                                FILE *out);
void tess_capi_end(void *api);

#ifdef __cplusplus
}
#endif

#endif
