cmake_minimum_required(VERSION 3.15)
project(k2pdfopt LANGUAGES C CXX)

option(DISABLED_LEGACY_ENGINE "Disable the legacy OCR engine" OFF)

set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(Leptonica lept REQUIRED IMPORTED_TARGET)
pkg_check_modules(Tesseract tesseract REQUIRED IMPORTED_TARGET)

file(GLOB K2PDFOPTLIB_SRC ../k2pdfoptlib/*.c)
file(GLOB WILLUSLIB_SRC ../willuslib/*.c)
file(GLOB HDR *.h ../k2pdfoptlib/*.h ../willuslib/*.h)

add_library(k2pdfopt SHARED
    koptcrop.c koptimize.c koptocr.c koptreflow.c
    setting.c
    tesscapi.cpp
    ${K2PDFOPTLIB_SRC} ${WILLUSLIB_SRC}
)
set_target_properties(k2pdfopt PROPERTIES SOVERSION 2)
target_compile_definitions(k2pdfopt PRIVATE BUILDING_LIBK2PDFOPT)
if(DISABLED_LEGACY_ENGINE)
    target_compile_definitions(k2pdfopt PRIVATE DISABLED_LEGACY_ENGINE)
endif()
target_include_directories(k2pdfopt PRIVATE ../k2pdfoptlib ../willuslib .)
target_link_libraries(k2pdfopt PRIVATE m PkgConfig::Leptonica PkgConfig::Tesseract Threads::Threads)
if(ANDROID)
    target_link_libraries(k2pdfopt PRIVATE log)
endif()

install(TARGETS k2pdfopt)
install(FILES ${HDR} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/k2pdfopt)
