#pragma once

#ifdef BUILDING_LIBK2PDFOPT
# include "../include_mod/tesseract.h"
#endif

#include <allheaders.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

const char* tess_capi_get_init_language(void *api);
int tess_capi_get_word_boxes(void *api, PIX *pix, BOXA **out_boxa, int is_cjk, FILE *out);

#ifdef __cplusplus
}
#endif
