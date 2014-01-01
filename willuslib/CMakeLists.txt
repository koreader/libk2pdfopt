project(k2pdfopt C)
cmake_minimum_required(VERSION 2.6)

include_directories(..)

set(WILLUSLIB_SRC
        pdfwrite.c wfile.c wzfile.c wgs.c 
	math.c point2d.c array.c mem.c string.c strbuf.c ansi.c render.c 
	filelist.c fontdata.c bmp.c gslpolyfit.c token.c   
	willusversion.c fontrender.c wsys.c
	linux.c dtcompress.c 
)
# ocr.c  
# win.c  winbmp.c winmbox.c wincomdlg.c wgui.c winshell.c

if(DJVU_FOUND)
  set(WILLUSLIB_SRC ${WILLUSLIB_SRC} bmpdjvu.c)
endif(DJVU_FOUND)

if(MUPDF_FOUND)
  set(WILLUSLIB_SRC ${WILLUSLIB_SRC} bmpmupdf.c wmupdf.c)
endif(MUPDF_FOUND)

message("-- Willuslib sources: ${WILLUSLIB_SRC}")

add_library(willuslib ${WILLUSLIB_SRC})

# HAVE_GOCR_LIB: ocrjocr.c
# HAVE_TESSERACT_LIB: ocrtess.c 

