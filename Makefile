MAJVER=  1

MOD_INC = include_mod
LEPTONICA_DIR = leptonica-1.69
TESSERACT_DIR = tesseract-ocr
TESSERACT_MOD = tesseract_mod
WILLUSLIB_DIR = willuslib
K2PDFOPTLIB_DIR = k2pdfoptlib
WILLUSLIB_SRC = $(wildcard $(WILLUSLIB_DIR)/*.c)
K2PDFOPTLIB_SRC = $(wildcard $(K2PDFOPTLIB_DIR)/*.c)
KOPT_SRC = setting.c koptreflow.c koptcrop.c koptocr.c

TESSCAPI_CFLAGS = -I$(MOD_INC) -I$(LEPTONICA_DIR)/src \
	-I$(TESSERACT_DIR) -I$(TESSERACT_DIR)/api \
	-I$(TESSERACT_DIR)/ccutil -I$(TESSERACT_DIR)/ccstruct \
	-I$(TESSERACT_DIR)/image -I$(TESSERACT_DIR)/viewer \
    -I$(TESSERACT_DIR)/textord -I$(TESSERACT_DIR)/dict \
    -I$(TESSERACT_DIR)/classify -I$(TESSERACT_DIR)/ccmain \
    -I$(TESSERACT_DIR)/wordrec -I$(TESSERACT_DIR)/cutil

OBJS:=$(KOPT_SRC:%.c=%.o) \
	$(K2PDFOPTLIB_SRC:%.c=%.o) \
	$(WILLUSLIB_SRC:%.c=%.o) \
	$(TESSERACT_API:%.cpp=%.o)
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
##############################################################################
# Object file rules.
##############################################################################
%.o: %.c
	$(TARGET_CC) $(CFLAGS) -c -I$(MOD_INC) -I$(WILLUSLIB_DIR) -I$(K2PDFOPTLIB_DIR) -o $@ $<
	$(TARGET_DYNCC) $(CFLAGS) -c -I$(MOD_INC) -I$(WILLUSLIB_DIR) -I$(K2PDFOPTLIB_DIR) -o $(@:.o=_dyn.o) $<
	
##############################################################################
# Target file rules.
##############################################################################
leptonica:
ifdef EMULATE_READER
	cd $(LEPTONICA_DIR) && ./configure \
		CC='$(strip $(CCACHE) $(CC))' CFLAGS='$(CFLAGS)' \
		--disable-static --enable-shared \
		&& make
else
	cd $(LEPTONICA_DIR) && ./configure --host $(HOST) \
		CC='$(strip $(CCACHE) $(CC))' CFLAGS='$(CFLAGS)' \
		--disable-static --enable-shared \
		&& make
endif
	cp -a $(LEPTONICA_DIR)/src/.libs/liblept.so* ./
	
tesseract: leptonica
	cp $(TESSERACT_MOD)/tessdatamanager.cpp $(TESSERACT_DIR)/ccutil/
ifdef EMULATE_READER
	cd $(TESSERACT_DIR) && ./autogen.sh && ./configure \
		CXX='$(strip $(CCACHE) $(CXX))' CXXFLAGS='$(CXXFLAGS) -I$(CURDIR)/$(MOD_INC)' \
		LIBLEPT_HEADERSDIR=$(CURDIR)/$(LEPTONICA_DIR)/src \
		--with-extra-libraries=$(CURDIR) \
		--disable-static --enable-shared \
		&& make
else
	cd $(TESSERACT_DIR) && ./autogen.sh && ./configure --host $(HOST) \
		CXX='$(strip $(CCACHE) $(CXX))' CXXFLAGS='$(CXXFLAGS) -I$(CURDIR)/$(MOD_INC)' \
		LIBLEPT_HEADERSDIR=$(CURDIR)/$(LEPTONICA_DIR)/src \
		--with-extra-libraries=$(CURDIR) \
		--disable-static --enable-shared \
		&& make
endif
	cp -a $(TESSERACT_DIR)/api/.libs/libtesseract.so* ./
	
tesseract_capi: $(TESSERACT_MOD)/tesscapi.cpp tesseract
	$(TARGET_CXX) $(CXXFLAGS) -c $(TESSCAPI_CFLAGS) -o $(TESSERACT_API_O) $<
	$(TARGET_DYNCXX) $(CXXFLAGS) -c $(TESSCAPI_CFLAGS) -o $(TESSERACT_API_DYNO) $<

$(K2PDFOPT_A): $(K2PDFOPT_O) tesseract_capi
	$(AR) rcs $@ $(K2PDFOPT_O) $(TESSERACT_API_O)

$(K2PDFOPT_SO): $(K2PDFOPT_O) tesseract_capi
	$(CC) $(TARGET_ASHLDFLAGS) -o $@ $(K2PDFOPT_DYNO) $(TESSERACT_API_DYNO) $(TARGET_ALIBS)
	ln -s $(K2PDFOPT_SO) libk2pdfopt.so
	
all: tesseract $(TARGET)

clean:
	rm -rf *.o
	rm -rf *.a
	rm -rf *.so*
	cd $(WILLUSLIB_DIR) && rm -rf *.o
	cd $(K2PDFOPTLIB_DIR) && rm -rf *.o
	cd $(LEPTONICA_DIR) && make clean
	cd $(TESSERACT_DIR) && make clean

.PHONY: clean
