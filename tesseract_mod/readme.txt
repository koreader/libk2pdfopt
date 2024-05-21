I compiled Tesseract v4.0.0 with gcc using these additional options:
-march=nehalem -DGRAPHICS_DISABLED -Wno-sign-compare

I compiled these three files with -march=sandybridge
simddetect.cpp
dotproductavx.cpp
intsimdmatrixavx2.cpp
