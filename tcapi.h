#ifndef _TCAPI_H
#define _TCAPI_H

#ifdef __cplusplus
extern "C" {
#endif

int tess_capi_get_word_boxes(void *vapi, PIX *pix, BOXA **out_boxa, int is_cjk, FILE *out);

#ifdef __cplusplus
}
#endif

#endif
