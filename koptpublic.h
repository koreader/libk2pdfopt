#ifndef _K2PDFOPT_PUBLIC
#define _K2PDFOPT_PUBLIC

#include "k2pdfopt.h"
#include "leptonica.h"
#include "tesseract.h"
#include "context.h"

// Only functions in here will be marked for library export

#define PUBLIC __attribute__((externally_visible)) __attribute__((visibility ("default")))

PUBLIC void bmp_init(WILLUSBITMAP *);
PUBLIC void bmp_free(WILLUSBITMAP *);
PUBLIC int bmp_alloc(WILLUSBITMAP *);
PUBLIC int bmp_copy(WILLUSBITMAP *, WILLUSBITMAP *);
PUBLIC unsigned char *bmp_rowptr_from_top(WILLUSBITMAP *, int);
PUBLIC void wrectmaps_init(WRECTMAPS *);
PUBLIC void wrectmaps_free(WRECTMAPS *);
PUBLIC int wrectmap_inside(WRECTMAP *, int, int);
PUBLIC void k2pdfopt_get_reflowed_word_boxes(KOPTContext *, WILLUSBITMAP *, int, int, int, int);
PUBLIC void k2pdfopt_get_native_word_boxes(KOPTContext *, WILLUSBITMAP *, int, int, int, int);
PUBLIC void k2pdfopt_tocr_single_word(WILLUSBITMAP *, int, int, int, int, char *, int, char *, char *, int, int, int);
PUBLIC void k2pdfopt_reflow_bmp(KOPTContext *);
PUBLIC void k2pdfopt_tocr_end();
PUBLIC void pageregions_init(PAGEREGIONS *);
PUBLIC void pageregions_free(PAGEREGIONS *);
PUBLIC void k2pdfopt_crop_bmp(KOPTContext *);
PUBLIC void k2pdfopt_optimize_bmp(KOPTContext *);
PUBLIC void pixmap_to_bmp(WILLUSBITMAP *, unsigned char *, int);
PUBLIC PIX* bitmap2pix(WILLUSBITMAP *, int, int, int, int);

void k2pdfopt_settings_init_from_koptcontext(K2PDFOPT_SETTINGS *k2settings, KOPTContext *kctx);
void k2pdfopt_get_native_word_boxes(KOPTContext *kctx, WILLUSBITMAP *src, int x, int y, int w, int h);
int k2pdfopt_get_word_boxes_from_tesseract(PIX *pixs, int is_cjk, BOXA **pboxad, NUMA **pnai);

#endif
