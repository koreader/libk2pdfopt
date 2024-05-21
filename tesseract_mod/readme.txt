I compiled Tesseract v4.1.1 with gcc using these additional options:
-march=nehalem -DGRAPHICS_DISABLED -Wno-sign-compare

I compiled these four files with -march=haswell for the Windows/Linux builds
and -march=nehalem for the Mac OSX builds:
simddetect.cpp
dotproductavx.cpp
dotproductfma.cpp
intsimdmatrixavx2.cpp
