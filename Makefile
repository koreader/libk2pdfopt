MAJVER=  2

MOD_INC = include_mod
LEPTONICA_DIR = $(CURDIR)/leptonica-1.69
TESSERACT_DIR = tesseract-ocr
TESSERACT_MOD = $(CURDIR)/tesseract_mod
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
DYNAMIC_CC= $(CC) $(if $(WIN32),,-fPIC)
STATIC_CXX= $(CXX)
DYNAMIC_CXX= $(CXX) $(if $(WIN32),,-fPIC)
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
ifdef DARWIN
	TARGET_XSHLDFLAGS= -shared -fPIC
else
	TARGET_XSHLDFLAGS= -shared $(if $(WIN32),,-fPIC) -Wl,-soname,$(TARGET_SONAME)
endif
TARGET_ASHLDFLAGS= $(TARGET_XSHLDFLAGS) $(TARGET_DYLIBPATH) $(TARGET_FLAGS) $(TARGET_SHLDFLAGS)
TARGET_XLIBS= -lm
TARGET_ALIBS= $(TARGET_XLIBS) $(LIBS) $(TARGET_LIBS)

K2PDFOPT_A= libk2pdfopt.a
K2PDFOPT_SO= $(TARGET_SONAME)

LEPTONICA_LIB= liblept$(if $(WIN32),-3.dll,$(if $(DARWIN),.dylib,.so))
TESSERACT_LIB= libtesseract$(if $(WIN32),-3.dll,$(if $(DARWIN),.dylib,.so))
K2PDFOPT_LIB= libk2pdfopt$(if $(WIN32),-$(MAJVER).dll,$(if $(DARWIN),.$(MAJVER).dylib,.so.$(MAJVER)))

##############################################################################
# Object file rules.
##############################################################################
%.o: %.c
	@echo "BUILD    $@"
	@$(TARGET_CC) $(CFLAGS) -c -DK2PDFOPT_KINDLEPDFVIEWER \
		$(if $(ANDROID),-DANDROID,) $(if $(DARWIN),-DDARWIN,) \
		-I$(MOD_INC) -I$(WILLUSLIB_DIR) -I$(K2PDFOPTLIB_DIR) -o $@ $<
	@$(TARGET_DYNCC) $(CFLAGS) -c -DK2PDFOPT_KINDLEPDFVIEWER \
		$(if $(ANDROID),-DANDROID,) $(if $(DARWIN),-DDARWIN,) \
		-I$(MOD_INC) -I$(WILLUSLIB_DIR) -I$(K2PDFOPTLIB_DIR) -o $(@:.o=_dyn.o) $<

##############################################################################
# Target file rules.
##############################################################################
$(LEPTONICA_LIB):
	cd $(LEPTONICA_DIR) && ./configure -q $(if $(EMULATE_READER),,--host $(HOST)) \
		--prefix=$(LEPTONICA_DIR) \
		CC='$(strip $(CCACHE) $(CC))' CFLAGS='$(LEPT_CFLAGS)' \
		LDFLAGS='$(LEPT_LDFLAGS) -Wl,-rpath,"libs" $(ZLIB_LDFLAGS) $(PNG_LDFLAGS)' \
		--disable-static --enable-shared \
		--with-zlib --with-libpng --without-jpeg --without-giflib --without-libtiff
	# fix cannot find library -lc on mingw-w64
	cd $(LEPTONICA_DIR) && sed -ie "s|archive_cmds_need_lc='yes'|archive_cmds_need_lc='no'|" config.status
	cd $(LEPTONICA_DIR) && $(MAKE) CFLAGS='$(LEPT_CFLAGS)' \
		install --silent >/dev/null 2>&1
ifdef WIN32
	cp -a $(LEPTONICA_DIR)/src/.libs/liblept-3.dll ./
else
	cp -a $(LEPTONICA_DIR)/src/.libs/liblept$(if $(DARWIN),*.dylib,.so*) ./
endif

$(TESSERACT_LIB): $(LEPTONICA_LIB)
	cp $(TESSERACT_MOD)/tessdatamanager.cpp $(TESSERACT_DIR)/ccutil/
	-cd $(TESSERACT_DIR) && \
		patch -N -p1 < $(TESSERACT_MOD)/baseapi.cpp.patch
	cd $(TESSERACT_DIR) && ./autogen.sh && ./configure -q \
		$(if $(EMULATE_READER),,--host=$(HOST)) \
		CXX='$(strip $(CCACHE) $(CXX))' \
		CXXFLAGS='$(CXXFLAGS) -I$(MOD_INC)' \
		$(if $(WIN32),CPPFLAGS='-D_tagBLOB_DEFINED',) \
		LIBLEPT_HEADERSDIR=$(LEPTONICA_DIR)/src \
		LDFLAGS='$(STDCPPLIB) $(LEPT_LDFLAGS) -Wl,-rpath,\$$$$ORIGIN $(ZLIB_LDFLAGS) $(PNG_LDFLAGS)' \
		--with-extra-libraries=$(LEPTONICA_DIR)/src/.libs \
		--disable-static --enable-shared --disable-graphics
	cd $(TESSERACT_DIR) && sed -ie 's|-lstdc++||g' libtool
	$(MAKE) -C $(TESSERACT_DIR)
ifdef WIN32
	cp -a $(TESSERACT_DIR)/api/.libs/libtesseract-3.dll ./
else
	cp -a $(TESSERACT_DIR)/api/.libs/libtesseract$(if $(DARWIN),*.dylib,.so*) ./
endif

tesseract_capi: $(TESSERACT_MOD)/tesscapi.cpp $(TESSERACT_LIB)
	$(TARGET_CXX) $(CXXFLAGS) -c $(TESSCAPI_CFLAGS) -o $(TESSERACT_API_O) $<
	$(TARGET_DYNCXX) $(CXXFLAGS) -c $(TESSCAPI_CFLAGS) -o $(TESSERACT_API_DYNO) $<

$(K2PDFOPT_A): $(K2PDFOPT_O) tesseract_capi
	$(AR) rcs $@ $(K2PDFOPT_O) $(TESSERACT_API_O)

$(K2PDFOPT_LIB): $(K2PDFOPT_O) tesseract_capi
	$(CXX) $(TARGET_ASHLDFLAGS) -static-libstdc++ -Wl,-rpath,'libs' -o $@ \
		$(K2PDFOPT_DYNO) $(TESSERACT_API_DYNO) $(TARGET_ALIBS) \
		$(TESSERACT_LIB) $(LEPTONICA_LIB)
ifndef WIN32
	ln -sf $(K2PDFOPT_LIB) libk2pdfopt$(if $(DARWIN),.dylib,.so)
endif

all: $(TESSERACT_LIB) $(LEPTONICA_LIB) $(K2PDFOPT_LIB)

clean:
	rm -rf *.o
	rm -rf *.a
	rm -rf *.so*
	rm -rf *.dll
	rm -rf *.dylib
	cd $(WILLUSLIB_DIR) && rm -rf *.o
	cd $(K2PDFOPTLIB_DIR) && rm -rf *.o
	cd $(TESSERACT_MOD) && rm -rf *.o
	cd $(LEPTONICA_DIR) && make distclean
	cd $(TESSERACT_DIR) && make distclean

.PHONY: clean
