project(k2pdfopt C)
cmake_minimum_required(VERSION 2.6)

# ---- Test for packages

include(FindPkgConfig)

set(K2PDFOPT_LIB "-lm")

# cmake -DCMAKE_BUILD_TYPE=Debug <path>

# TODO: configure flavour with opions
## 
## option (USE_MUPDF
## 	"Use muPDF libraries" OFF)
## option (USE_GHOSTSCRIPT
## 	"Use Ghostscript executable" ON)
## option (USE_DJVU
## 	"" OFF)
## eoption (USE_GOCR
## 	"" OFF)
## option (USE_LEPTONICA
## 	"" OFF)
## option (USE_TESSERACT
## 	"" OFF)
## option (USE_JASPER
## 	"" OFF)
## option (USE_GSL
## 	"" OFF)

# HAVE_K2GUI

include(FindZLIB)
if(ZLIB_FOUND)
  set(HAVE_Z_LIB 1)
# include_directories(${ZLIB_INCLUDE_DIRS})
  set(K2PDFOPT_LIB ${K2PDFOPT_LIB} ${ZLIB_LIBRARY})
endif(ZLIB_FOUND)

include(FindPNG)
if(PNG_FOUND)
  set(HAVE_PNG_LIB 1)
  set(K2PDFOPT_LIB ${K2PDFOPT_LIB} ${PNG_LIBRARY_RELEASE})
endif(PNG_FOUND)

include(FindJPEG)
if(JPEG_FOUND)
  set(HAVE_JPEG_LIB 1)
  set(K2PDFOPT_LIB ${K2PDFOPT_LIB} ${JPEG_LIBRARY})
endif(JPEG_FOUND)

include(FindJasper)
if(JASPER_FOUND)
  set(HAVE_JASPER_LIB 1)
endif(JASPER_FOUND)

# paths from willuslib/wgs.c
find_program(GHOSTSCRIPT_EXECUTABLE gs
#  PATHS /usr/bin /usr/share/gs /usr/local/gs /opt/gs 
#    /usr/share/gs* /usr/local/gs* /opt/gs*
)
if(GHOSTSCRIPT_EXECUTABLE)
  set(HAVE_GHOSTSCRIPT 1)
  message(STATUS "Found ghostscript executable")
else()
  message(STATUS "Could NOT find ghostscript executable")
endif(GHOSTSCRIPT_EXECUTABLE)

# willus.h
# HAVE_GSL_LIB


# libfreetype6 (>= 2.3.9), libjbig2dec0, libjpeg8 (>= 8c), libx11-6, libxext6, zlib1g (>= 1:1.2.0)
# MUPDF_STATIC_LDFLAGS misses mupdf-js-none, and doubles libs ...

pkg_check_modules(MUPDF mupdf)
if(MUPDF_FOUND)
  set(HAVE_MUPDF_LIB 1) 
  # (includes jbig2dec, openjpeg, freetype)
  include_directories(SYSTEM ${MUPDF_INCLUDEDIR})
  message(STATUS "mupdf libraries: ${MUPDF_LDFLAGS}")
  set(K2PDFOPT_LIB ${K2PDFOPT_LIB} ${MUPDF_LDFLAGS} 
    -lmupdf-js-none -lopenjpeg -ljbig2dec -ljpeg -lfreetype
  )
endif(MUPDF_FOUND)

pkg_check_modules(DJVU ddjvuapi)
if(DJVU_FOUND)
  set(HAVE_DJVU_LIB 1)
  include_directories(SYSTEM ${DJVU_INCLUDEDIR})
  set(K2PDFOPT_LIB ${K2PDFOPT_LIB} ${DJVU_LDFLAGS})
endif(DJVU_FOUND)

# HAVE_GOCR_LIB
# HAVE_LEPTONICA_LIB
# HAVE_TESSERACT_LIB

# ---- Describe project

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DUSE_CMAKE -Wall")

include_directories(. willuslib k2pdfoptlib)

configure_file (
  "${PROJECT_SOURCE_DIR}/config.h.in"
  "${PROJECT_BINARY_DIR}/config.h"
)

add_subdirectory(willuslib)
add_subdirectory(k2pdfoptlib)

# ms
add_executable(k2pdfopt k2pdfopt.c)
target_link_libraries (k2pdfopt k2pdfoptlib willuslib ${K2PDFOPT_LIB})


message("")
message("-- Summary --")
message("")
message("General:")
message("	C-flags:	'${CMAKE_C_FLAGS}' (-DCMAKE_C_FLAGS=\"...\")")
message("	Build type:	${CMAKE_BUILD_TYPE} (-DCMAKE_BUILD_TYPE=\"...\")")
message("")
message("Install:")
message("	Prefix:		'${CMAKE_INSTALL_PREFIX}' (-DCMAKE_INSTALL_PREFIX=...)")
message("	Doc path:	'${DOC_PATH}' (-DDOC_PATH=...)")
message("")
