rem
rem The willus lib needs to be compiled without any optional third party libs -- see
rem the section in willus.h with HAVE_XXX_LIB defines.
rem
rem The k2pdfopt lib needs to be compiled without HAVE_GUI defined.
rem This will minimize the size of k2view.exe.
rem
gcc -Ofast -m32 -Wall k2view.c -o k2view.exe -static-libgcc -static-libstdc++ d:\mingw\x32\lib\crt_noglob.o -Ld:\customlibs -lk2pdfopt -lwillus -lgdi32 -luuid -lole32 -lcomdlg32
k2view out.bmp
