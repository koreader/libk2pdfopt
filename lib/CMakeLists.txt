cmake_minimum_required(VERSION 3.15)
project(k2pdfopt LANGUAGES C CXX)

option(DISABLED_LEGACY_ENGINE "Disable the legacy OCR engine" OFF)

set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(Leptonica lept REQUIRED IMPORTED_TARGET)
pkg_check_modules(Tesseract tesseract REQUIRED IMPORTED_TARGET)

add_library(k2pdfopt
    koptcrop.c
    koptimize.c
    koptocr.c
    koptreflow.c
    setting.c
    tesscapi.cpp
    ../k2pdfoptlib/bmpregion.c
    ../k2pdfoptlib/devprofile.c
    ../k2pdfoptlib/k2bmp.c
    ../k2pdfoptlib/k2file.c
    ../k2pdfoptlib/k2master.c
    ../k2pdfoptlib/k2mem.c
    ../k2pdfoptlib/k2ocr.c
    ../k2pdfoptlib/k2proc.c
    ../k2pdfoptlib/k2settings.c
    ../k2pdfoptlib/k2sys.c
    ../k2pdfoptlib/k2version.c
    ../k2pdfoptlib/pagelist.c
    ../k2pdfoptlib/pageregions.c
    ../k2pdfoptlib/textrows.c
    ../k2pdfoptlib/textwords.c
    ../k2pdfoptlib/wrapbmp.c
    ../willuslib/ansi.c
    ../willuslib/array.c
    ../willuslib/bmp.c
    ../willuslib/filelist.c
    ../willuslib/gslpolyfit.c
    ../willuslib/linux.c
    ../willuslib/math.c
    ../willuslib/mem.c
    ../willuslib/ocr.c
    ../willuslib/ocrgocr.c
    ../willuslib/ocrtess.c
    ../willuslib/ocrwords.c
    ../willuslib/strbuf.c
    ../willuslib/string.c
    ../willuslib/token.c
    ../willuslib/wfile.c
    ../willuslib/wgui.c
    ../willuslib/willusversion.c
    ../willuslib/wleptonica.c
    ../willuslib/wsys.c
)
set_target_properties(k2pdfopt PROPERTIES SOVERSION 2)
set_target_properties(k2pdfopt PROPERTIES
    C_VISIBILITY_PRESET hidden
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN TRUE
)
target_compile_definitions(k2pdfopt PRIVATE BUILDING_LIBK2PDFOPT NO_FILELIST)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(k2pdfopt PRIVATE K2PDFOPT_DEBUG)
    target_sources(k2pdfopt PRIVATE
        ../k2pdfoptlib/k2mark.c
        ../willuslib/dtcompress.c
        ../willuslib/pdffonts.c
        ../willuslib/pdfwrite.c
        ../willuslib/wpdfoutline.c
        ../willuslib/wpdfutil.c
    )
endif()
if(DISABLED_LEGACY_ENGINE)
    target_compile_definitions(k2pdfopt PRIVATE DISABLED_LEGACY_ENGINE)
endif()
target_include_directories(k2pdfopt PRIVATE ../k2pdfoptlib ../willuslib .)
target_link_libraries(k2pdfopt PRIVATE m PkgConfig::Leptonica PkgConfig::Tesseract Threads::Threads)
if(ANDROID)
    target_link_libraries(k2pdfopt PRIVATE log)
endif()

install(TARGETS k2pdfopt)
install(FILES
    context.h
    koptcrop.h
    koptimize.h
    koptocr.h
    koptreflow.h
    leptonica.h
    setting.h
    tesseract.h
    ../k2pdfoptlib/k2pdfopt.h
    ../willuslib/willus.h
    DESTINATION ${CMAKE_INSTALL_PREFIX}/include/k2pdfopt
)
