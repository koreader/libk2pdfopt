######################## config variables
LIBNAME ?= libk2pdfopt.so # can be also libk2pdofopt.a if you omit -shared from LDFLAGS
CFLAGS ?= -O2 -fPIC -s -ffunction-sections -fdata-sections -fvisibility=hidden
LDFLAGS ?= -shared -Wl,--gc-sections -fvisibility=hidden

XCFLAGS ?= \
	-DHAVE_TESSERACT_LIB \
	-DHAVE_Z_LIB \
	-DHAVE_PNG_LIB \
	-DHAVE_JPEG_LIB \
	-DHAVE_MUPDF_LIB \
	-DHAVE_DJVU_LIB \
	-DHAVE_LEPTONICA_LIB \
	-DHAVE_TESSERACT_LIB
#-DHAVE_JASPER_LIB
#-DHAVE_GSL_LIB
#-DHAVE_GHOSTSCRIPT
#-DHAVE_GOCR_LIB

# must match what XCFLAGS says
XLIBS ?= \
	-lz -lpng -ljpeg -lmupdf -ldjvulibre -llept -ltesseract -lm -lpthread
########################

all: $(LIBNAME)

clean:
	rm -rf tesseract leptonica mupdf
	rm -f *.o */*/*.o $(LIBNAME)
dist_clean: clean
	rm -f patches/*.patch

K2PDFOPT := k2pdfopt
INC_DIR := $(K2PDFOPT)/include_mod
WILLUS_DIR := $(K2PDFOPT)/willuslib
K2PDFOPT_DIR := $(K2PDFOPT)/k2pdfoptlib

# um ... we're not using CMAKE, but this tricks willuslib into respecting HAVE_* flags
XCFLAGS += -DUSE_CMAKE
XCFLAGS += -DK2PDFOPT_KINDLEPDFVIEWER
XCFLAGS += -I$(INC_DIR) -I$(WILLUS_DIR) -I$(K2PDFOPT_DIR) -I.

SRC=$(wildcard $(WILLUS_DIR)/*.c) $(wildcard $(K2PDFOPT_DIR)/*.c) \
	setting.c koptreflow.c koptcrop.c koptocr.c koptimize.c
OBJ=$(SRC:%.c=%.o)

%.o: %.c
	$(CC) $(XCFLAGS) $(CFLAGS) -c $< -o $@

$(LIBNAME): $(OBJ)
	$(CC) $(LDFLAGS) $(OBJ) -o $(LIBNAME) $(XLIBS)

