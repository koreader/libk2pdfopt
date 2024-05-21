Tesseract 5.3.0  Released 27 Dec 2022
-------------------------------------
!
! NOTE:  Need to use march=nehalem or higher to get SSE 4.1 code to work!
!
! As of 13 Jan, Tesseract v3.05 CUBE does a better job (on Pooh.pdf) than this version.
!
1. Copy all source (gocopy.bat).
1a. Copy the file pkg\include\tesseract folder to the system include folder
2. Copy env.txt from previous version.
       Defines from autoconfig that are used:
           GRAPHICS_DISABLED
           Pre-defined:  GIT_REV, MINGW, __MINGW32__, _OPENMP, __UNIX__
3. Create file allheaders.h and put this line in it:
       #include <leptonica.h>
4. Make empty version of these:  environ.h, pix.h, imageio.h, arrayaccess.h
     (Empty versions are okay -- maybe because I have GRAPHICS_DISABLED...?)
5. Wrote tesscapi.cpp, my version of a C API wrapper.
6. Search for "willus" in previous version files and implement mods
   (check tesseract subfolder as well)
7. Fix tess_version.h (look at previous version) and copy to include system folder as tesseract\version.h
8. Copy tesseract.h from previous version.
9. Check for new dotproduct / matrix CPU options (e.g. FMA, AVX512, etc.)--look at simddetect.h.

[Note:  If first compile fails and complains about not finding stdio.h or stdlib.h,
        check the cplus_include_path environment variable path--make sure it has the
        right version of gcc in the all of the path names.]

-Will Menninger
Last updated:  28 Dec 2022
