MAJVER=  1
MINVER=  6
RELVER=  2

MOD_INC = include_mod
WILLUSLIB_DIR = willuslib
K2PDFOPTLIB_DIR = k2pdfoptlib
WILLUSLIB_SRC = $(filter-out $(WILLUSLIB_DIR)/ocrtess.c, $(wildcard $(WILLUSLIB_DIR)/*.c))
K2PDFOPTLIB_SRC = $(wildcard $(K2PDFOPTLIB_DIR)/*.c)
KOPTREFLOW_SRC = setting.c koptreflow.c koptcrop.c

OBJS:=$(KOPTREFLOW_SRC:%.c=%.o) $(K2PDFOPTLIB_SRC:%.c=%.o) $(WILLUSLIB_SRC:%.c=%.o)
K2PDFOPT_O= $(OBJS)
K2PDFOPT_DYNO= $(OBJS:.o=_dyn.o)

STATIC_CC = $(CC)
DYNAMIC_CC = $(CC) -fPIC
TARGET_CC= $(STATIC_CC)
TARGET_DYNCC= $(DYNAMIC_CC)
TARGET_STRIP=$(STRIP)
TARGET = $(K2PDFOPT_SO)
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
$(K2PDFOPT_A): $(K2PDFOPT_O)
	$(AR) rcs $@ $(K2PDFOPT_O)

$(K2PDFOPT_SO): $(K2PDFOPT_O)
	$(CC) $(TARGET_ASHLDFLAGS) -o $@ $(K2PDFOPT_DYNO) $(TARGET_ALIBS)
	ln -s $(K2PDFOPT_SO) libk2pdfopt.so
	
all: $(TARGET)

clean:
	cd $(WILLUSLIB_DIR) && rm -rf *.o
	cd $(K2PDFOPTLIB_DIR) && rm -rf *.o
	rm -rf *.o
	rm -rf *.a
	rm -rf *.so*

.PHONY: clean
