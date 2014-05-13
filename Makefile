MAJVER=  2

MOD_INC = include_mod
LEPTONICA_DIR = leptonica-1.69
TESSERACT_DIR = tesseract-ocr
TESSERACT_MOD = tesseract_mod
WILLUSLIB_DIR = willuslib
K2PDFOPTLIB_DIR = k2pdfoptlib
WILLUSLIB_SRC = $(wildcard $(WILLUSLIB_DIR)/*.c)
K2PDFOPTLIB_SRC = $(wildcard $(K2PDFOPTLIB_DIR)/*.c)
KOPT_SRC = setting.c koptreflow.c koptcrop.c koptocr.c koptpart.c koptimize.c

TESSCAPI_CFLAGS = -I$(MOD_INC) -I$(LEPTONICA_DIR)/src \
	-I$(TESSERACT_DIR) -I$(TESSERACT_DIR)/api \
	-I$(TESSERACT_DIR)/ccutil -I$(TESSERACT_DIR)/ccstruct \
	-I$(TESSERACT_DIR)/image -I$(TESSERACT_DIR)/viewer \
    -I$(TESSERACT_DIR)/textord -I$(TESSERACT_DIR)/dict \
    -I$(TESSERACT_DIR)/classify -I$(TESSERACT_DIR)/ccmain \
    -I$(TESSERACT_DIR)/wordrec -I$(TESSERACT_DIR)/cutil

OBJS:=$(KOPT_SRC:%.c=%.o) \
	$(K2PDFOPTLIB_SRC:%.c=%.o) \
	$(WILLUSLIB_SRC:%.c=%.o)

K2PDFOPT_O= $(OBJS)
K2PDFOPT_DYNO= $(OBJS:.o=_dyn.o)
TESSERACT_API_O= $(TESSERACT_MOD)/tesscapi.o
TESSERACT_API_DYNO= $(TESSERACT_MOD)/tesscapi_dyn.o

ifeq ($(USE_NO_CCACHE), 1)
	CCACHE:=
else
	CCACHE?=$(shell which ccache)
endif

STATIC_CC= $(CC)
DYNAMIC_CC= $(CC) -fPIC
STATIC_CXX= $(CXX)
DYNAMIC_CXX= $(CXX) -fPIC
TARGET_CC= $(STATIC_CC)
TARGET_DYNCC= $(DYNAMIC_CC)
TARGET_CXX= $(STATIC_CXX)
TARGET_DYNCXX= $(DYNAMIC_CXX)
TARGET_STRIP=$(STRIP)
TARGET= $(K2PDFOPT_SO)
ifeq (static,$(BUILDMODE))
   TARGET = $(K2PDFOPT_A)
endif
TARGET_SONAME= libk2pdfopt.so.$(MAJVER)
TARGET_DYLIBPATH=
TARGET_XSHLDFLAGS= -shared -fPIC -Wl,-soname,$(TARGET_SONAME)
TARGET_ASHLDFLAGS= $(TARGET_XSHLDFLAGS) $(TARGET_DYLIBPATH) $(TARGET_FLAGS) $(TARGET_SHLDFLAGS)
TARGET_XLIBS= -lm
TARGET_ALIBS= $(TARGET_XLIBS) $(LIBS) $(TARGET_LIBS)

K2PDFOPT_A= libk2pdfopt.a
K2PDFOPT_SO= $(TARGET_SONAME)

LEPTONICA_LIB= liblept.so
TESSERACT_LIB= libtesseract.so
K2PDFOPT_LIB= libk2pdfopt.so.$(MAJVER)

##############################################################################
# Object file rules.
##############################################################################
%.o: %.c
	@echo "BUILD    $@"
	@$(TARGET_CC) $(CFLAGS) -c -I$(MOD_INC) -I$(WILLUSLIB_DIR) \
		-I$(K2PDFOPTLIB_DIR) -o $@ $<
	@$(TARGET_DYNCC) $(CFLAGS) -c -I$(MOD_INC) -I$(WILLUSLIB_DIR) \
		-I$(K2PDFOPTLIB_DIR) -o $(@:.o=_dyn.o) $<

##############################################################################
# Target file rules.
##############################################################################
$(LEPTONICA_LIB):
ifdef EMULATE_READER
	cd $(LEPTONICA_DIR) && ./configure -q \
		CC='$(strip $(CCACHE) $(CC))' CFLAGS='$(LEPT_CFLAGS)' \
		LDFLAGS='-Wl,-rpath-link,$(LEPT_PNG_DIR) -Wl,-rpath,'libs' $(LEPT_LDFLAGS)' \
		LIBS='-lz -lm' \
		--disable-static --enable-shared \
		--with-zlib --with-libpng --without-jpeg --without-giflib --without-libtiff \
		&& $(MAKE) --silent CFLAGS='$(LEPT_CFLAGS)'
else
	cd $(LEPTONICA_DIR) && ./configure -q --host $(HOST) \
		CC='$(strip $(CCACHE) $(CC))' CFLAGS='$(LEPT_CFLAGS)' \
		LDFLAGS='-Wl,-rpath-link,$(LEPT_PNG_DIR) -Wl,-rpath,'libs' $(LEPT_LDFLAGS)' \
		LIBS='-lz -lm' \
		--disable-static --enable-shared \
		--with-zlib --with-libpng --without-jpeg --without-giflib --without-libtiff \
		&& $(MAKE) --silent CFLAGS='$(LEPT_CFLAGS)'
endif
	cp -a $(LEPTONICA_DIR)/src/.libs/liblept.so* ./

$(TESSERACT_LIB): $(LEPTONICA_LIB)
	cp $(TESSERACT_MOD)/tessdatamanager.cpp $(TESSERACT_DIR)/ccutil/
ifdef EMULATE_READER
	cd $(TESSERACT_DIR) && ./autogen.sh && ./configure -q \
		CXX='$(strip $(CCACHE) $(CXX))' CXXFLAGS='$(CXXFLAGS) -I$(CURDIR)/$(MOD_INC)' \
		LIBLEPT_HEADERSDIR=$(CURDIR)/$(LEPTONICA_DIR)/src \
		LDFLAGS='-Wl,-rpath-link,$(LEPT_PNG_DIR) -Wl,-rpath,\$$$$ORIGIN $(LEPT_LDFLAGS)' \
		LIBS='-lz -lm' \
		--with-extra-libraries=$(CURDIR) \
		--disable-static --enable-shared \
		&& $(MAKE) --silent >/dev/null 2>&1
else
	cd $(TESSERACT_DIR) && ./autogen.sh && ./configure -q --host $(HOST) \
		CXX='$(strip $(CCACHE) $(CXX))' CXXFLAGS='$(CXXFLAGS) -I$(CURDIR)/$(MOD_INC)' \
		LIBLEPT_HEADERSDIR=$(CURDIR)/$(LEPTONICA_DIR)/src \
		LDFLAGS='-Wl,-rpath-link,$(LEPT_PNG_DIR) -Wl,-rpath,\$$$$ORIGIN $(LEPT_LDFLAGS)' \
		LIBS='-lz -lm' \
		--with-extra-libraries=$(CURDIR) \
		--disable-static --enable-shared \
		&& $(MAKE) --silent >/dev/null 2>&1
endif
	cp -a $(TESSERACT_DIR)/api/.libs/libtesseract.so* ./

tesseract_capi: $(TESSERACT_MOD)/tesscapi.cpp $(TESSERACT_LIB)
	$(TARGET_CXX) $(CXXFLAGS) -c $(TESSCAPI_CFLAGS) -o $(TESSERACT_API_O) $<
	$(TARGET_DYNCXX) $(CXXFLAGS) -c $(TESSCAPI_CFLAGS) -o $(TESSERACT_API_DYNO) $<

$(K2PDFOPT_A): $(K2PDFOPT_O) tesseract_capi
	$(AR) rcs $@ $(K2PDFOPT_O) $(TESSERACT_API_O)

$(K2PDFOPT_LIB): $(K2PDFOPT_O) tesseract_capi
	$(CC) $(TARGET_ASHLDFLAGS) -Wl,-rpath,'libs' -o $@ \
		$(K2PDFOPT_DYNO) $(TESSERACT_API_DYNO) $(TARGET_ALIBS) \
		$(MUPDF_LIB) $(TESSERACT_LIB) $(LEPTONICA_LIB)
	ln -sf $(K2PDFOPT_LIB) libk2pdfopt.so

all: $(TESSERACT_LIB) $(LEPTONICA_LIB) $(K2PDFOPT_LIB)

clean:
	rm -rf *.o
	rm -rf *.a
	rm -rf *.so*
	cd $(WILLUSLIB_DIR) && rm -rf *.o
	cd $(K2PDFOPTLIB_DIR) && rm -rf *.o
	cd $(TESSERACT_MOD) && rm -rf *.o
	cd $(LEPTONICA_DIR) && make clean
	cd $(TESSERACT_DIR) && make clean

.PHONY: clean
