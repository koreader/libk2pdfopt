######################## config variables
LIBNAME ?= libk2pdfopt.so # can be also libk2pdofopt.a if you omit -shared from LDFLAGS
CFLAGS ?= -O2 -fPIC -s -ffunction-sections -fdata-sections
# -fvisibility=hidden
LDFLAGS ?= -shared -Wl,--gc-sections -lpthread -Wl,--version-script=export.map

# must match what config.h says
XLIBS ?= \
	-lz -lpng -ljpeg -lmupdf -ldjvulibre -llept -ltesseract -lm
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
	setting.c koptreflow.c koptcrop.c koptocr.c koptimize.c koptcb.c
OBJ=$(SRC:%.c=%.o)

%.o: %.c
	$(CC) $(XCFLAGS) $(CFLAGS) -c $< -o $@

$(LIBNAME): $(OBJ)
	$(CC) $(LDFLAGS) $(OBJ) -o $(LIBNAME) $(XLIBS)

