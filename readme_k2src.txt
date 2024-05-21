K2pdfopt build help.
http://willus.com
Original: 7 September 2012
Last updated:  4 Jan 2019 (v2.51)

This "read me" file describes the source code distribution for k2pdfopt.

Version Change History
----------------------
See k2pdfoptlib\k2version.c.

K2pdfopt Source Files
---------------------
1. k2pdfopt.c (main module)

2. k2pdfopt C library (28 C files + 1 header file) in k2pdfoptlib subfolder.
   Compile all C files in this subfolder and build them into libk2pdfopt.a.
   Near the top of k2pdfopt.h are #defines which control third-party
   library dependencies (HAVE_XXX_LIB) and whether or not the MS Windows
   GUI is compiled in (HAVE_K2GUI).

3. willus.com general-purpose C library (38 C files + 1 header file) in
   willuslib subfolder.
   Compile all C files in this subfolder and build them into libwillus.a.
   Near the top of willus.h are #defines which control third-party
   library dependencies, i.e. HAVE_MUPDF_LIB.  These are passed on to
   the k2pdfopt library since k2pdfopt.h includes willus.h.


Third Party Libraries
---------------------
There are also a number of required 3rd-party open-source C/C++ libraries
for k2pdfopt.  These libraries are not necessarily required when using
the k2pdfopt library in other applications (e.g. KindlePDFViewer--see
NOTE 4 below).

    REQUIRED
    --------
    1.  Z-lib 1.2.11 (zlib.net)
    2.  libpng 1.6.35 (www.libpng.org)
    3.  Turbo JPEG lib 2.0.1 (sourceforge.net/projects/libjpeg-turbo/)

    TO INCLUDE MuPDF LIBRARY (search for HAVE_MUPDF in k2pdfopt.c)
    --------------------------------------------------------------
    4.  JBIG2Dec 0.11 (jbig2dec.sourceforge.net)
    5.  OpenJPEG 2.3.0 (www.openjpeg.org)
    6.  FreeType 2.9.1 (freetype.sourceforge.net/index2.html)
    7.  Mupdf 1.14 (mupdf.com) -- SEE NOTE 1.

    TO INCLUDE DjVuLibre LIBRARY (search for HAVE_DJVU in k2pdfopt.c)
    -----------------------------------------------------------------
    8.  DJVULibre 3.5.25.3 (C++) (djvu.sourceforge.net)

    FOR OCR VERSIONS OF K2PDFOPT (search for HAVE_OCR in k2pdfopt.c)
    ----------------------------------------------------------------
    9.  GOCR 0.50 (sourceforge.net/jocr/)
    10. Leptonica 1.74.4 (leptonica.com)
    11. Tesseract 4.0.0 (C++) (code.google.com/tesseract-ocr/) -- SEE NOTE 2.
    12. POSIX threads support (pretty standard with gcc implementations)

    If you don't include MuPDF, DjVuLibre, or OCR, then k2pdfopt will
    look for an installation of Ghostscript.


Notes
-----
1. Mods to the released MuPDF library are in the mupdf_mod folder.
   Search for "willus" or "sumatra" or "bugs" in the files to find the mods.

2. Tesseract requires my small C API file plus some custom-modified source files.
   These are in the tesseract_mod folder.  Search for "willus" in the files to
   find the mods.  To use Tesseract, you'll need to download one of the data
   packages for it from the Tesseract web site and to point the TESSDATA_PREFIX
   environment variable to the root tesseract folder
   (e.g. TESSDATA_PREFIX=c:\tesseract-ocr\).

3. For a lot of the 3rd-party libraries, I combined their headers into one
   header that gets included by some of the willus library source files.
   I can't always remember which ones I did this for, so if you can't find
   any of the included files (if they didn't come standard w/the 3rd party
   library), look in my include_mod subfolder.

4. Building for KOReader or KindlePDFViewer:
   a. In willus.h, search for "THIRD PARTY" and comment out all #define's
      for third party libs, e.g. HAVE_Z_LIB, etc.
   b. In k2pdfopt.h, uncomment this:  #define K2PDFOPT_KINDLEPDFVIEWER
   c. Compile all modules in libwillus and libk2pdfopt.
   d. Look in the "kindlepdfviewer" subfolder for a couple examples of
      how to call the k2pdfopt library functions.  The k2view.c program
      is stand-alone.  If it is compiled without dependencies on third-party
      libraries, it will be quite small (~300 KiB in windows).
   e. If you are compiling with MINGW but don't have the Win32 API,
      use the predefined macro:  NO_WIN32_API (-DNO_WIN32_API).

5. I have included CMakeLists.txt files for the k2pdfopt and willus libraries
   from Dirk Thierbach to help with Linux builds.  He also contributed the
   config.h.in file and the dtcompress.c file (in willus lib).  It is possible
   to build the project without using these files (I do not use them).
   

Build Steps for k2pdfopt on Windows (gcc 7.3.0)
-----------------------------------------------
My compile steps with gcc (MinGW) are as follows (assuming all the libraries are built
to libxxx.a files in d:\3rdparty_lib and headers are in d:\3rdparty_include):

    64-bit
    ------
    1. echo k2pdfopt ICON k2pdfopt.ico > k2pdfopt.rc

    2. windres --target=pe-x86-64 -o resfile.o k2pdfopt.rc

    3. gcc -Ofast -m64 -Wall -c -Id:\3rdparty_include k2pdfopt.c

    4. g++ -Ofast -m64 -Wall -o k2pdfopt.exe k2pdfopt.o resfile.o -static-libgcc -static-libstdc++ d:\mingw\x64\lib\crt_noglob.o -Ld:\3rdparty_lib -lk2pdfopt -lwillus -lgocr -ltesseract -lleptonica -ldjvu -lmupdf -lfreetype -ljbig2 -ljpeglib -lopenjpeg -lpng -lzlib -lpthread -lgdi32 -luuid -lole32 -lcomdlg32 -lshlwapi


    32-bit
    ------
    1. echo k2pdfopt ICON k2pdfopt.ico > k2pdfopt.rc

    2. windres --target=pe-i386 -o resfile.o k2pdfopt.rc

    3. gcc -Ofast -m32 -Wall -c -Id:\3rdparty_include k2pdfopt.c

    4. g++ -Ofast -m32 -Wall -o k2pdfopt.exe k2pdfopt.o resfile.o -static-libgcc -static-libstdc++ d:\mingw\i386\lib\crt_noglob.o -Ld:\3rdparty_lib -lk2pdfopt -lwillus -lgocr -ltesseract -lleptonica -ldjvu -lmupdf -lfreetype -ljbig2 -ljpeglib -lopenjpeg -lpng -lzlib -lpthread -lgdi32 -luuid -lole32 -lcomdlg32 -lshlwapi


Build Steps on Linux (64-bit, gcc 8.2.0, compiled on Fedora 29)
---------------------------------------------------------------
1. gcc -Wall -Ofast -m64 -o k2pdfopt.o -c k2pdfopt.c

2. g++ -Ofast -m64 -o k2pdfopt k2pdfopt.o -static -static-libgcc -static-libstdc++ -lk2pdfopt -lwillus -lgocr -ltesseract -lleptonica -ldjvu -lmupdf -lfreetype -ljbig2 -ljpeglib -lopenjpeg -lpng -lzlib -lpthread -lstdc++ -lc -lm


Build Steps on OS/X (64-bit, gcc 8.2.0, compiled on OSX 10.12 Sierra)
----------------------------------------------------------------------
1. gcc -Ofast -Wall -m64 -o k2pdfopt.o -c k2pdfopt.c

2. g++ -Ofast -m64 -o k2pdfopt k2pdfopt.o -static-libgcc -static-libstdc++ -lk2pdfopt -lwillus -lgocr -ltesseract -lleptonica -ldjvu -lmupdf -lfreetype -ljbig2 -ljpeglib -lopenjpeg -lpng -lzlib -lpthread
