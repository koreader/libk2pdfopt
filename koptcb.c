#include <mupdf/pdf.h>

// called whenever k2pdfopt creates a pdf context
// usually, we don't care, but may need to expose it as a callback to Lua one day

void pdf_install_load_system_font_funcs(fz_context *ctx) {
}
