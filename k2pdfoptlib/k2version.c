char *k2pdfopt_version = "v2.42";
/*
** k2version.c  K2pdfopt version number and history.
**
** Copyright (C) 2017  http://willus.com
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU Affero General Public License as
** published by the Free Software Foundation, either version 3 of the
** License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU Affero General Public License for more details.
**
** You should have received a copy of the GNU Affero General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
**
** VERSION HISTORY
**
** V2.42 20 MAY 2017
**           NEW FEATURES
**           - Bitmap output.  The -o <namefmt> option now takes names ending
**             in .jpg and .png to tell k2pdfopt to write out bitmaps instead of
**             a PDF file.  E.g. -o .png writes pages to srcfile0001.png,
**             srcfile0002.png, etc.
**           - Added de-warp (-dw) option to de-warp scanned pages a la
**             ScanTailor.  This is most useful for books that have been pressed
**             against the glass of a copier and the scans still have some
**             curvature (warping) in them.  Or for photographs of pages taken
**             at a slight angle.  Uses Leptonica's built-in functions.
**           ENHANCEMENTS
**           - Improved autocrop (-ac) algorithm and added an "aggressiveness" factor
**             to it.  It is now called more efficiently as well (it used to be
**             called multiple times per source page).
**           - Added Kobo Aura One to built-in device list
**             https://www.mobileread.com/forums/showthread.php?p=3513245#post3513245
**           - Improved straightening/deskew algorithm.  Faster and uses weighted
**             average / fine grain search (25 Mar 2017).  ~20% faster for 64-bit.
**           - Keyboard shortcut change:  'i' key now brings up file info.
**           - PDF information shows dates and page size more clearly now.
**           BUG FIXES
**           - The -ocrout function works even if native output mode is specified.
**           - Pressing ESC key when file overwrite is requested now results in a
**             "no" response (it used to result in a "yes" response).
**
** V2.41 25 FEB 2017
**           ENHANCEMENTS
**           - Updated libraries: MuPDF v1.10a, Tesseract 3.05.00,
**             Leptonica 1.74.1, FreeType 2.7.1, libpng v1.6.28, zlib 1.2.11.
**           - Compiled with MinGW/GNU C 6.3 (MS Windows version)
**           NEW FEATURE
**           - Added bitmap erosion filter (-er option) as an alternative to
**             gamma correction (-g) to make text thicker/darker (idea from
**             the original "Yet Another PDF to LRF Converter" program).
**           
** V2.40 7 JAN 2017
**           NEW FEATURE
**           - OCR processing is now multithreaded.  This may provide a significant
**             boost in OCR processing speed depending on how many CPU threads you
**             have available.  See the -nt option.  As part of this upgrade, the
**             OCR and total CPU time are now echoed for each run (using the
**             clock() function).  Include MS Windows GUI control for number of
**             CPUS.  Inspired by Harry Shamanski's Elucidate app.
**           BUG FIXES
**           - Fixed a couple of minor places in the usage where options were not
**             in alphabetical order.
**           - k2printf() flushes stdout.
**           - MS Windows GUI crop selection does not ask for page range if
**             document only has one page.
**           - MS Windows GUI no longer exceeds size of desktop when maximized.
**
** V2.36 26 NOV 2016
**           ENHANCEMENTS
**           - The -colorfg and -colorbg options can take an array of colors, in
**             which case each array element is used for the next row of text.
**             See usage.  Suggested in a 5 Nov 2016 e-mail.
**           BUG FIXES
**           - If using -mode trim with -bp, blank pages are passed through to
**             the output file as if -mode copy.  From 11-16-2016 e-mail.
**           - Minimum dpi allowed from k2settings_settings_set_margins_and_devsize
**             is 1.
**           - Fixed bug where -bp was not recognized if at the end of a command line.
**           - -ocrvis b did not work before without 's'.  Now it does.
**           - Fixed Tesseract to correctly parse .traineddata files when certain
**             locales are set.  It was not correctly parsing Chinese .traineddata
**             files on my Windows 10 system because of the locale.
**           - Updated GOCR library to GOCR v0.50.  Not convinced it is improved.
**           - Re-ordered some command line options in usage to keep them alphabetical.
**
** V2.35 22 OCT 2016 
**           ENHANCEMENTS
**           - Mac OSX version compiled on new machine running Sierra with GCC 6.2.0.
**             Also, the binaries are compressed with a newer version of UPX which
**             is compatible with Mac OSX 10.12 Sierra.
**           - Linux binaries compiled on CentOS 7.2 with GCC 4.8.5.
**           - Windows binaries compiled with GCC 6.2.0 (MinGW).
**           - Compiled with the latest versions of libpng (1.6.25), freetype (2.7),
**             turbo JPEG (1.5.1).
**           - I tried newer versions of MuPDF (v1.9a and v1.10 pre-release), but
**             they broke more things than I was comfortable with, so I've stayed
**             with MuPDF v1.8 for this release.
**           NEW FEATURES
**           - Added -jfc- option to prevent trying to join figure captions to the
**             figures.
**             http://www.mobileread.com/forums/showthread.php?p=3342105#post3342105
**           - Added new conversion mode, -mode concat, which keeps the output at
**             the same dimensions as the source file and concatenates crop-boxes
**             (red boxes) together--as many as can fit on each page without breaking
**             them apart.
**           - Added option -f2p -3 to support -mode concat.
**           BUG FIXES
**           - No longer crashes in native PDF output mode if there is no output.
**           - Writes more informative message to screen if output file not written.
**           - Makes sure output file can be opened for writing before proceeding
**             with the conversion.  Warns user if file cannot be opened.
**             http://www.mobileread.com/forums/showthread.php?p=3343367#post3343367
**           - Correctly processes blank/empty pages in .djvu files.
**             http://www.mobileread.com/forums/showthread.php?p=3350691#post3350691
**           - The -title option now substitutes the file name for %s or %b, like
**             the -o option.
**             http://www.mobileread.com/forums/showthread.php?p=3389292#post3389292
**           - The -grid overlap percentage is more precise now.
**           - Warning message to use -fc- with -odpi, -fs, or -mag.  Not sure this
**             is the best way--should I just turn off -fc if those are specified?
**             Reported in 22 April 2016 e-mail.
**             Also: http://www.mobileread.com/forums/showthread.php?p=3354548#post3354548
**           - Wide-char (UTF-8) DJVU file names now work.
**             http://www.mobileread.com/forums/showthread.php?p=3351085#post3351085
**           - Blank pages no longer cause an improper conversion with -mode trim.
**             E-mail from 17 Mar 2016.
**           - Clarified usage of -mode copy, explaining about gamma and contrast
**             settings (how they are not reset to 1 with -mode copy).
**           - In the function where the 2-column divider is detected, a special
**             call is made to find_textrows() so that figure caption joining is
**             disabled unless -jfc+ is specified.  This helps 2-column detection
**             work more reliably.
**             http://www.mobileread.com/forums/showthread.php?p=3351808#post3351808
**             http://www.mobileread.com/forums/showthread.php?p=3342105#post3342105
**           MS WINDOWS GUI BUG FIXES
**           - The GUI now correctly selects the "crop" conversion mode.
**
** V2.34b 21 MAR 2016
**           MS WINDOWS BUG FIXES
**           - Native conversions correctly generate unicode file names
**             http://www.mobileread.com/forums/showthread.php?p=3284264#post3284264
**           - GUI correctly provides right-click help on the output folder widgets
**
** V2.34a 19 MAR 2016
**           BUG FIX
**           - Device "Kobo H2O" can now be correctly assigned. (E-mail 3-17-16)
**           MS WINDOWS GUI BUG FIX
**           - Fixed "Additional Options" and "Command-line Options" read-only issue.
**           - Fixed file list box overlapping the output folder box.
**           - Fixed "Add File" selection of .pdf and .djvu files.
**
** V2.34 18 MAR 2016
**           ENHANCEMENTS
**           - Compiled with the latest versions of MuPDF (1.8), Tesseract (3.04.01),
**             libpng (1.6.21), freetype (2.6.2), turbo JPEG (1.4.2), and
**             leptonica (1.72).
**           NEW FEATURES
**           - Added an option to scale the output font size to a fixed value:
**             -fs <fontsize>.  E.g. -fs 12 will scale the median font size
**             in the source document to be 12 points in the converted document.
**             See details in the command-line usage help.  Supported in MS Windows
**             GUI as well (new check box with up/down control next to it).
**             http://www.mobileread.com/forums/showthread.php?p=3175758#post3175758
**           - The -o has two new format characters--%b for the base name of the
**             source file (not including the path).  %f for the folder the source
**             file is stored in.
**           - The -mag feature magnifies the text/images in the output file.  It
**             as the same effect as increasing -odpi.
**           - New -y option is same as -ow (assumes "yes" on overwriting files).
**             I just did this for compatibility with other programs that use this
**             same option the same way (e.g. ffmpeg).
**           - The -cbox and -ibox options now have a "u" appendix that applies to
**             any unspecified pages.  E.g. -cbox5-10 <cbox1> -cboxu <cbox2>
**             This will apply <cbox1> to pages 5 through 10 and <cbox2> to all
**             other pages.
**             http://www.mobileread.com/forums/showthread.php?p=3204759#post3204759
**           - The -ci option can be used to specify an image for a cover page for
**             the PDF file (doesn't work with native PDF output yet).
**             http://www.mobileread.com/forums/showthread.php?p=3196307#post3196307
**           NEW MS WINDOWS GUI FEATURES
**           - There is now a selection button for the output folder, which uses
**             the new %b formatting in the -o option.  Requested 6 Jan 2016 via e-mail.
**           - Fixed several bugs involving the crop box GUI selection.
**           - Crop box setting prompts for an overriding page range now, and then
**             reports both the -cbox and the -m dimensions when complete.
**           - Added code to detect Calibri vs. Arial font, but this didn't have
**             the effect I wanted, so I commented it back out (in k2gui.c,
**             search for "fontscale".)
**           BUG FIXES
**           - Fixed the documentation of -ocrcol.  Said it was ignored now, but it
**             is not ignored for documents that don't already have an OCR layer.
**           - If the folder for the converted file(s) does not exist, it will be
**             created (6 Jan 2016 e-mail).
**           - Gets page bounding box from "Pages" object if not available for an
**             individual page.  (E-mail bug report, 10 Oct 2015).
**           - Made -bpc / -jpg incompatibility more clear in the command-line usage.
**           - k2printf correctly uses a semaphore to prevent threads from both using
**             it at the same time.
**           - MS Windows GUI:  Fixed issues with multi-threading when starting the
**             conversion process and starting the process to generate the bitmap for
**             setting the crop boxes.  Semaphores were not being correctly closed.
**           - MS Windows GUI:  File opening works more reliably now after multiple
**             aborted conversions (new willusgui_file_open_ex function).
**           - Postscript files are correctly processed now.  Because ghostscript
**             cannot pull specific pages from a PS file, they are now converted to
**             PDF (using ghostscript) before any further processing by k2pdfopt.
**
** V2.33a 3 OCT 2015
**           BUG FIX
**           - Fixed MS Windows GUI bug where preview page controls were missing.
**
** V2.33 27 SEP 2015
**           NEW FEATURES
**           - Compiled with GCC v5.2.0 and MuPDF v1.7a (released May 7, 2015).
**             The MuPDF upgrade involved modifying a significant amount of the
**             MuPDF interface code in the willus library since Artifex changed the
**             APIs on several functions, but the bulk of the logic did not change.
**             I uncovered a bug in the pdf_dict_del() function as well (reported).
**           - The -i option displays information about the source PDF file.  Added
**             to MS Windows GUI also.
**           - Added -fr option to rotate wide-aspect-ratio figures to landscape.
**             http://www.mobileread.com/forums/showthread.php?p=3060339#post3060339
**           - Added Kindle Paperwhite 3 (2015 release) and Pocketbook Basic 2 to
**             dev list (from http://www.mobileread.com/forums/showthread.php?t=253579)
**           - Smarter sorting of red regions on a multiple-column page.  See
**             pageregion_sort() function in pageregions.c.
**           - New -ibox option has same format as -cbox, but these boxes are ignored
**             by k2pdfopt--they are "whited out" in the source file.  For native
**             output, the contents may still be visible in the output.
**           - The -neg option now attempts to only negate text passages to white
**             on black and to leave figures alone.  Use -neg+ to negate everything.
**             http://www.mobileread.com/forums/showthread.php?p=3104536#post3104536
**           - Added option -ehl to erase horizontal lines in the document.  Works
**             exactly like the -evl option.
**           - Added -author and -title options to specify the author and title of
**             the output PDF.
**             http://www.mobileread.com/forums/showthread.php?p=3112052#post3112052
**           - Added -px option to exclude a set of pages, e.g. -px 4,7,10-20.
**             http://www.mobileread.com/forums/showthread.php?p=3112052#post3112052
**           - User can use color markings to tell k2pdfopt where to apply page
**             breaks to the output file.
**             http://www.mobileread.com/forums/showthread.php?p=3152988#post3152988
**           - The -? option can now be followed by a (wildcard) matching string to
**             show the usage of a particlar option, e.g. -? -ws.
**
**           NEW MS WINDOWS GUI FEATURES
**           - Crop margins replaced by three crop-box entries.
**             These entries can act as either -cbox or -ibox.
**           - Added an "INFO" button to show information on the selected PDF file.
**           - If a wildcard is specified for a PDF source file
**             on the command line (e.g. *.pdf), and the GUI is launched, all matching
**             entries are placed into the GUI file list.
**
**           BUG FIXES
**           - With notes options turned on (-nl / -nr), k2pdfopt will still search
**             for multiple columns if no notes are found on the page.  In addition,
**             the -crgh option now more directly affects column divider finding.
**             See textrows_remove_small_rows() call in bmpregion_find_multicolumn_divider().
**             http://www.mobileread.com/forums/showthread.php?p=3148589#post3148589
**           - Fixed multiple file select (broke when I converted to wide chars in v2.30).
**           - Modified bmpregion_hyphen_detect() to be less strict about rejecting
**             hyphens that aren't exactly centered.  Also modified calculation of
**             lcheight in bmpregion_calc_bbox()--see the function.
**             http://www.mobileread.com/forums/showthread.php?p=3119501#post3119501
**           - The k2pdfopt web site and help pages work again from the help menu.
**           - Turned off some debugging text from the bmp_autocrop2 function in k2bmp.c.
**           - Not really a bug fix, but the command-line help is now shown in
**             Courier New in MS Windows (a mono-spaced font).
**           - In info_update() in wmupdf.c in the willus library, I check to see
**             if I can resolve the Info dictionary.  This checks to see if it can
**             be parsed correctly.  If not, I discard the dictionary.  This was
**             causing a bug that a user submitted to me in an e-mail on 15 April 2015.
**             The users had a PDF file with a corrupt "Info" dictionary.
**           - WPDFOUTLINE structures correctly freed.
**           - MuPDF v1.7 stores ligatured characters differently than previous versions
**             in its internal character arrays, so I had to compensate for this.
**
** v2.32 6 MAR 2015
**           NEW FEATURES
**           - A new auto-cropping feature (-ac) has been added where k2pdfopt will
**             attempt to crop out dark edges due to scanning / copying artifacts.
**             There is a checkbox for it in the MS Windows GUI.
**           - MS Windows GUI: graphical selection of crop margins has now been
**             implemented.  When you click the "Select Margins" button, k2pdfopt will
**             overlay all of the pages in the "Pages to Convert" box and allow you
**             to select a rectangular crop region with the mouse, which it will then
**             use to populate the "Crop Margins" values.
**           - The -ls option now takes an optional page range so that it can be
**             applied to specified pages.  A control was added to the MS Windows
**             GUI for this.
**             http://www.mobileread.com/forums/showthread.php?p=3016119#post3016119
**           - Improved the context sensitive help in the MS Windows GUI.
**
**           OTHER
**           - Update the list of devices and their dimension from the information
**             collected at http://www.mobileread.com/forums/showthread.php?t=253579
**           - Clarified -cbox usage.
**             http://www.mobileread.com/forums/showthread.php?p=3048458#post3048458
**           - Added source code flow description to k2pdfopt.c.  (2-18-15 e-mail).
**
**           BUG FIXES
**           - Fixed a bug with notes in the margins (-nl/-nr options), checking
**             for notesrows->n==0 (was causing a crash).  E-mailed on 13 Jan 2015.
**           - Clarified usage for -m option.
**           - Added MS Windows GUI confirmation of Tesseract initialization (in the
**             conversion dialog box).
**             http://www.mobileread.com/forums/showthread.php?p=3016119#post3016119
**           - -cbox<n>- did not work correctly if <n> was beyond the last page of
**             the source PDF.  This is fixed.
**             http://www.mobileread.com/forums/showthread.php?p=3047145#post3047145
**           - Fixed a preview error noted by a mobileread member.  Was due to
**             not correctly clearing a WTEXTCHARS structure in
**             ocrlayer_bounding_box_inches().
**             http://www.mobileread.com/forums/showthread.php?p=3027213#post3027213
**           - Fixed some issues with masterinfo_should_flush() where it wasn't
**             correctly figuring out the next page.
**           - Fixed issues selecting the text in text edit boxes as they gain focus
**             (either through tabbing or mouse clicks).
**           - The number of inserted rows added by textrows_find_doubles() is now
**             limited to a reasonable number.  This was going out of control in
**             one oddball case and (I think) causing k2pdfopt to crash.
**
** v2.31 27 DEC 2014
**           NEW FEATURES
**           - Added -ppgs option to post process output with ghostscript pdfwrite
**             device.  This can improve text selection when there are overlapping
**             cropped regions.  Recommended in 7 Dec 2014 e-mail.
**           - In MS Windows GUI, added context-sensitive help.  If you right-click
**             most controls in the main window, you will get a dialog box explaining
**             what that control does.
**           NEW DEVICES
**           - Added separate Kindle Paperwhite 2 resolution:  710x960.
**             From 12 Dec 2014 e-mail.
**           - Added kindle voyage.
**           BUG FIXES
**           - Erase vertical lines correctly works from MS Windows GUI again.
**           OTHER
**           - Compiled with MuPDF v1.6 library.
**           - Separated out functions in wmupdf.c that do not depend on MuPDF.
**             Those are now in wpdf.c.  This helps the KOReader build.
**             https://github.com/koreader/koreader-base/pull/290 (2 Dec 2014)
**             Also e-mail on 7 Dec 2014.
**           - No longer assumes that WIN32 = GUI.  To compile so as not to use
**             the WIN32 API, define NO_WIN32_API from the compile command line.
**
** v2.30 26 NOV 2014
**           NEW FEATURES
**           - MS Windows version (including GUI) now supports wide-character file
**             names (UTF16), so names with non-ASCII characters in them (e.g.
**             Chinese characters) should be correctly handled.
**           - Added -colorfg and -colorbg options to adjust the foreground (text)
**             and background colors of the output file (only works for bitmapped
**             output files--doesn't work for native PDF output). You can even use
**             a background bitmap (which will be tiled).  Suggested in mobileread.com
**             private message, 24 Nov 2014. (beta4).
**           - Windows versions compiled with gcc 4.9.2 (MinGW).
**
**           MS WINDOWS GUI UPDATES
**           - As mentioned above, the MS Windows GUI supports wide characters now.
**           - Added "Get Folder" button. (beta1)
**             http://www.mobileread.com/forums/showthread.php?p=2945278#post2945278
**
**           BUG FIXES
**           - Marking corners now works for color bitmapped output.
**             http://www.mobileread.com/forums/showpost.php?p=2962214&postcount=935
**           - Compiled with OpenJPEG v2.1.0--fixed some cases with incorrectly
**             reading PDF files. (beta2)  The typical symptom is reported as
**             "JPX stream not read coorectly."  Issue reported in 2 Nov 2014 e-mail.
**           - Removed "wrectmaps->n=..." debugging output from k2ocr.c. (beta3)
**           - Clarified intended use of -rt vs. -ls.
**             http://www.mobileread.com/forums/showthread.php?p=2938230#post2938230
**           - Fixed buffer overrun in k2gui_cbox_set_pages_completed().
**             Reported via e-mail on 10 Nov 2014.
**           - Preview mode correctly turns on color for native PDF output
**             (also turns off gamma).  If the user tries to turn off color output
**             in the MS Windows GUI while native PDF output is checked, a dialog
**             box will pop up explaining why it won't uncheck.
**             http://www.mobileread.com/forums/showthread.php?p=2930855#post2930855
**           - Preview mode correctly puts up alert box when source file is not found.
**           - File open / folder open puts up alert box if file or folder is not found.
**           - The -ocrsp+ command-line option is now correctly recognized.
**             http://www.mobileread.com/forums/showthread.php?p=2902672#post2902672
**           - GOCR is correctly used if Tesseract cannot be initialized.
**
** v2.21 25 JUL 2014
**           - Compiled with MuPDF v1.5 (a highly recommened, mostly-bug-fix
**             upgrade recommended by the MuPDF folks).
**
** v2.20 25 JUL 2014
**           NEW FEATURES
**           - Added k2ocr_ocrwords_get_from_ocrlayer() and supporting functions to
**             to more intelligently extract all of the OCR-layer text from a major
**             ("red box") region (rather than parsing for the words graphically,
**             the OCR layer is queried for any words which are within the box).
**             This should eliminate the need to use the -ocrcol option on PDF
**             files which already have their own text layer.
**             [NOTE: The -ocrcol still has impact for documents that don't
**              already have an OCR layer. 13 Feb 2016.]
**           - There is a new option optimized for PDFs that have notes in the
**             left or right margins.  This option (-nl for notes in the left
**             margin or -nr for notes in the right margin) tells k2pdfopt to
**             look for notes and intersperse them with the main text.  The notes
**             can even alternate, e.g. left margin for odd pages and right margin for
**             even pages would be -nlo -nre.  Works well, for example, with the
**             attachment from this post:
**             http://www.mobileread.com/forums/showthread.php?p=2539093#post2539093
**             E.g. k2pdfopt -p 18-23 -cg 0.05 -nl -sm -f2p -1 ebook.pdf
**           - The word spacing (-ws) option now defaults to -0.20 (the old default
**             was 0.375).  When a negative value is given, an automatic word spacing
**             detection algorithm now used to break apart words in lines.  The
**             algorithm will try to choose a natural word spacing value, with
**             the minimum allowed being the absolute value of the setting (e.g.
**             0.2 for the default).  If you want k2pdfopt to aggressively break
**             lines (e.g. break apart long words if they don't fit on a line),
**             use a smaller absolute value, e.g. -ws -0.01.
**             You can use a positive value for the older style of line breaking,
**             and the lines are only broken where a gap exceeds that fraction
**             of the height of a lower-case 'o'.  There is also a new Windows
**             GUI checkbox for this option.
**           - Entire rows of text within the OCR layer can either have the words
**             within rendered individually (the default and original behavior
**             of k2pdfopt), or the entire row can be rendered at once with
**             spaces used between each word.  This may improve the text selection
**             behavior for certain readers.  The option that controls this is
**             -ocrsp:
**                 -ocrsp   puts one space between each word in the row.
**                 -ocrsp+  puts multiple spaces between each word in the row in
**                          order to better position the words since k2pdfopt
**                          does not typically match the exact font used by
**                          the source document when doing the OCR layer (it
**                          always uses arial).
**                 -ocrsp-  reverts to the original (default) behavior.
**             To test this, you can temporarily make the OCR layer visible by using
**             this option:  -ocrvis st
**             Thanks to a 19 Jan 2014 e-mail for inspiring this change.  A user
**             was having difficulty selecting text on their Sony PRS-T1 (the
**             entire row would select whenever they pressed on a single word).
**           - The -m and -om options now can use units, e.g. pixels, inches, cm,
**             and "s" for page/screen size.  In addition, -m can use the other
**             units used by -cbox, -w, and -h ("t" and "x").
**           - The new "x" unit of measure corresponds to the OCR Layer bounding
**             box, e.g. -cbox 0x,0x,1x,1x will correspond to a crop box that
**             matches the bounding box of the OCR layer.  This can be used by
**             the -w, -h, and -m options as well.  See -h usage.  Thanks to
**             markom for suggesting this (quite a while ago!):
**             http://www.mobileread.com/forums/showthread.php?p=2212714#post2212714
**           - New option -to[-] for text-only output, removes all figures as
**             determined by a height limit (see -jf option).  Use with -bp m
**             to avoid text selection issues if using in conjunction with native
**             output.  Requested in a 9 May 2014 e-mail.
**           - If -wt+ is specified for the white threshold, all pixels >= the
**             specified value will be painted pure white (255).
**             https://github.com/koreader/koreader/pull/549
**           - While not a perfect work-around, large, stylized first letters
**             which frequently begin a book chapter (typically the height of 2
**             or 3 normal-sized text rows) are now detected when wrapping text
**             lines so that the lines adjacent to them are more-or-less correctly
**             wrapped. Still needs improvement.
**
**           MS-WINDOWS GUI ENHANCEMENTS
**           - If the custom buttons are not used, a "2-column" and a "Fit Width"
**             button are automatically assigned.
**           - New MS-Windows GUI check box for -bp m option (Avoid text select overlap).
**           - New MS-Windows GUI check box for defects (sets -de 1.5).
**           - New MS-Windows GUI check box / text box for line break setting (-ws option)
**           - The last settings (other than the custom button presets) are remembered
**             between settings (stored in the K2PDFOPT_CUSTOM0 env var).
**             (4-15-14 e-mail)
**           - There is now a "Restore Defaults" button since k2pdfopt remembers
**             its last settings.
**           - New option for GUI: -rls[+|-].  Forces/disables restoration of last
**             settings from K2PDFOPT_CUSTOM0 environment variable.
**           - Environment variables related to the MS Windows GUI are only read
**             and set through Windows calls (not through getenv() and putenv())--
**             this was causing problems in trying to clear them.
**           - Command-line options that don't impact the GUI are put into the
**             "additional options" box upon launch.
**           - For file overwriting, the user is now given a "Yes to All" option
**             and a "No to all" option.
**           - There are two new menu options--to save and restore the settings
**             (stored in environment variables) to and from a file. This is done
**             in the k2gui_save_settings_to_file() and
**             k2gui_restore_settings_from_file() functions in k2gui.c.
**             http://www.mobileread.com/forums/showthread.php?p=2865852#post2865852
**           - If the output file cannot be opened (e.g. because another application
**             already has it open), a message box is shown informing the user
**             rather than just quitting the program.
**             http://www.mobileread.com/forums/showthread.php?p=2862339#post2862339
**
**           BUG FIXES
**           - Tabbing between crop margin text fields in the MS Windows GUI keeps
**             the entire text field selected even when you change a value before
**             tabbing.
**           - Fixed bug in window positioning at startup.
**           - Fixed bug where -ds did not get properly applied in native mode.
**             http://www.mobileread.com/forums/showthread.php?p=2828182#post2828182
**           - Fixed bug where tesseract was incorrectly initializing languages
**             that do not have CUBE/COMBINED data (e.g. Russian).  If that
**             initialization fails, it now tries CUBE-only (and then no CUBE at all).
**             This required a couple minor mods to the Tesseract library itself,
**             which are included in the latest k2pdfopt source code distribution.
**             http://www.mobileread.com/forums/showthread.php?p=2859736#post2859736
**
**           SOURCE CODE MODIFICATIONS
**           - The bmpregion_add() function and some others use a new parameter
**             data structure rather than having so many arguments passed to them
**             (ADDED_REGION_INFO).
**           - Copied web text at top of main k2 page to k2pdfopt.c intro.
**
** v2.18     14 JUN 2014
**           - Fixed problem when scaling sometimes gets out of control with tall
**             regions.  Was causing excessively large bitmaps to be allocated
**             which would sometimes run the system out of memory.  Search for
**             "2.18" in k2proc.c.
**             http://www.mobileread.com/forums/showthread.php?p=2846916#post2846916
**
** v2.17a    2 JUN 2014
**           - Fixes MuPDF v1.4 problem where it was not correctly using MS Windows
**             system fonts (introduced in v2.17).
**           - Compiled w/gcc 4.8.3.
**
** v2.17     17 MAY 2014
**           ENHANCEMENTS
**           - Compiled with the latest versions of MuPDF (1.4), Turbo JPEG (1.3.1),
**             libpng (1.6.10), and freetype (2.5.3).
**           - No longer echoes "fontdesc->font=000000000..." to the screen (was a
**             debugging printf that I put in MuPDF).
**             Example: k2pdfopt Example-PDF-Fonts-1.pdf
**
** v2.16     03 MAY 2014
**           BUG FIXES
**           - Avoid zero-value return from masterinfo_break_point().
**             (5-2-14 e-mail).
**           - TOC positioning fixed when source pages aren't large enough to
**             cause a new destination page (see k2publish.c).  Also, wmupdf
**             output now correctly handles UTF-8 outline/TOC titles.
**             http://www.mobileread.com/forums/showthread.php?p=2815504#post2815504
**
** v2.15     22 MAR 2014
**           ENHANCEMENTS
**           - The -cbox option usage has been rewritten and hopefully clarified.
**             It is a very powerful and useful new option as of v2.10.
**
**           BUG FIXES
**           - Specific variable to track Tesseract initialization (it was sometimes
**             getting missed if it was turned on after a conversion in the GUI).
**           - Mode selection works in GUI (wasn't correctly selecting "fitpage")
**             and in text menu again (1-7-14 e-mail).
**           - Fixed memory leak in bmp_detect_vertical_lines() in k2bmp.c.
**             http://www.mobileread.com/forums/showthread.php?p=2737816#post2737816
**           - Fixed several memory leaks--made sure bmpregion_free() is called
**             for each declared BMPREGION, and also patched wmupdf.c to fix two
**             memory leaks.  The most significant memory leak was in the dtcompress.c
**             library function, though, which has also been fixed.
**             http://www.mobileread.com/forums/showthread.php?p=2752370#post2752370
**           - Fixed incorrectly formed #ifdef HAVE_OCR_LIB in k2publish.c.  Thanks
**             to user facut on mobileread.com, who e-mailed me about this on
**             21 March 2014.
**
** v2.14     31 DEC 2013
**           - Compiled using dtcompress.c module in willus library which avoids
**             requiring my custom modification to zlib.  Thank you to Dirk Thierbach
**             for this.
**           - willus lib modules use more standard include callouts for MuPDF and
**             DjVu.
**           - Added CMakeLists.txt files to source distribution (also from Dirk).
**           - Correctly re-compiled Win32 build (wasn't done correctly in v2.13).
**
** v2.13     30 DEC 2013
**           ENHANCEMENTS
**           - Added kobo mini to device profile:
**             http://www.mobileread.com/forums/showthread.php?p=2704451#post2704451
**           - Applied patches from KOReader team:
**             http://www.mobileread.com/forums/showthread.php?p=2701292#post2701292
**               >> rectmap.patch.txt    (only on if K2PDFOPT_KINDLEPDFVIEWER defined)
**               >> indent_calibration.patch.txt (with modifications so that the
**                     example case that resulted in the patch still works, but so
**                     that most of my regression tests don't break.)
**                     See this link: https://github.com/koreader/koreader/issues/305
**               >> word_gap_calculation.patch.txt, auto-gap not on by default per
**                     this post:
**                     http://www.mobileread.com/forums/showthread.php?p=2708795#post2708795
**               >> static.patch.txt
**               >> debug.patch.txt
**           - The -ws option can take a negative value to turn on automatic word
**             spacing from the KOReader team.
**           - <pagelist> can have even 'e' and odd 'o' specifiers, e.g.
**               -p e (process all even pages numbers)
**               -p 1-100e,1-100o (process all even pages from 1-100, then all odd ones.)
**           - Cropboxes and gridded areas can now be processed fully into separate
**             columns if desired.
**           - The -cbox option now takes an optional page list to specify a specific
**             range of pages to apply the crop box to.  e.g.
**               -cbox1-99o <cropbox> applies the crop box to pages 1,3,5,...,99.
**           - The -cbox option takes units as well, so you can specify the crop box
**             size in pixels, inches, cm, or a fraction of the source page size.  The
**             units work the same as for the -w and -h options.
**           - GUI is smarter about creating command-line versions of margins
**             when they are all the same.
**           - New "-bp m" option will force page break at end of each major (red-box)
**             region.  This can improve text selection problems in native mode by
**             avoiding the text selection overlap that can occur when multiple
**             cropped source regions are placed on to a single output page.
**             (12/13/13 e-mail).
**
**           BUG FIXES
**           - Error checking done on K2PDFOPT_WINPOS variable.
**             http://www.mobileread.com/forums/showthread.php?p=2700958#post2700958
**           - Makes sure "(all)" doesn't get put in for the -p option (GUI).
**
** v2.12     30 NOV 2013
**           BUG FIXES
**           - No longer writes k2pdfopt_out.png when previewing in the GUI.
**           - Removed DLL dependencies from 64-bit Windows compile.
**
** v2.11     28 NOV 2013
**           BUG FIXES (MW WINDOWS GUI)
**           - Several routines in k2gui_cbox.c which are called from k2file.c
**             during the conversion were not correctly working during a preview
**             and were resulting in garbage sometimes being sent to the
**             desktop screen if the preview button was clicked after a file
**             conversion.  This has been fixed.
**
** v2.10     23 NOV 2013
**           NEW FEATURES
**           - The PDF "Outlines" tree (often called "bookmarks" by PDF viewers)
**             that helps you navigate the PDF file and is usually shown in the left
**             pane of the PDF viewer is now preserved in the converted file.  Or
**             you can create your own bookmarks from a simple text file if your
**             PDF source file doesn't have one (or if you want to change it).
**             See the -toc, -toclist, and -tocsave command-line options.
**             (toc = Table of Contents.)  Destination page breaks are forced
**             at outline anchor pages by default (see -bp option).
**           - A new -cbox option allows you to specify a crop box to be applied
**             to each page.  You can specify more than one, and each separate
**             crop box will be rendered to a different output page, similar to
**             the way the -grid option works.  See -cbox in the command usage.
**             Using -mode crop with -cbox, you can crop a source PDF file to
**             a destination PDF file.  You can specify different crop boxes
**             for even and odd pages, as well.
**           - The -bpl option now allows you to specify a list of source pages
**             where destination page breaks will be forced.
**           - Three new modes:  -mode trim causes the source page to be trimmed and
**             the destination to be sized to the trimmed source.  -mode fitpage
**             is similar, but squeezes the trimmed source page into the specified
**             device output screen size.  -mode crop is a complement to the -cbox
**             option and causes each cropped box to be placed on a new page the
**             size of the cropped box.
**
**           ENHANCEMENTS
**           - Windows versions are compiled with gcc 4.8.2.
**           - The Win64 binary is now compressed with UPX 3.91w which finally is
**             able to compress the Win64/PE format.
**            
**           BUG FIXES
**           - In native output, consecutive streams now delimited by white space. 
**             http://www.mobileread.com/forums/showthread.php?p=2655550#post2655550
**           - Pages with no "/Contents" entry are correctly handled.
**           - Re-wrote masterinfo_break_point() to make use of 
**             bmpregion_find_textrows() so that decisions on where to break
**             pages in the "fitwidth" mode should be more consistent and also
**             will be affected by the -gtr option.
**             http://www.mobileread.com/forums/showthread.php?p=2686067#post2686067
**           - Removed last vestiges of -pi option (interactive menu 'w' option
**             was incorrectly still using it).
**           - The vert_line_erase() function in k2bmp.c correctly handle the
**             cbmp pointer when it is an 8-bit bitmap now.
**           - Fixed a flow problem in k2file.c (k2pdfopt_proc_one() function)
**             which was causing the GUI preview not to work with -mode copy.
**           - The textrows_remove_small_rows() function no longer includes
**             figures (REGION_TYPE_FIGURE) when doing statistics on the row
**             heights.
**
** v2.03     21 SEP 2013
**           ENHANCEMENTS
**           - MuPDF library now uses the Sumatra versions of pdf-font.c and
**             pdf-fontfile.c so that it correctly checks Windows system fonts
**             for non-embedded fonts in the PDF file.
**
**           BUG FIXES
**           - Native mode is correctly turned off as the default setting.
**           - Native mode output works correctly from the MS Windows GUI.
**           - Check boxes made consistent (native/wrap/OCR) with quick
**             sanity check call.
**
** v2.02     16 SEP 2013
**           ENHANCEMENTS
**           - The main bitmap resampling function in the willus library,
**             bmp_resample(), now uses an alternate fixed-precision version (with
**             virtually no accuracy loss) on non-64-bit compiles, where it has
**             been tested to be significantly (50% to 100%) faster (on 64-bit
**             modern Intel CPUs, the floating-point version is actually fastest).
**             This should improve k2pdfopt performance on 32-bit and ARM
**             implementations, including on the KOReader, for example.
**             The resample routine also checks to see if no size change is
**             required--if so, it does a simple bmp_copy() call instead of resampling.
**             Thanks to Xin Huang (Chrox) for this suggestion and the initial
**             implementation.
**
**           GENERIC BUG FIXES
**           - echo_source_page_count correctly initialized to zero.
**           - hyphen detection turned on (has been disabled since at least v1.66).
**             (See bmpregion_hyphen_detect() function.)
**           - gap_sorted variable in textwords_add_word_gaps() is no longer static.
**           - Changed kindle paperwhite dims to 658 x 889 based on 9-15-13 e-mail
**             feedback.
**
**           MS WINDOWS GUI BUG FIXES
**           - Minimize/Maximize/Restore/Close buttons added to GUI convert dialog
**             box so that users can get k2pdfopt out of the way while it is working.
**           - GUI Conversion box no longer stays on top of other windows.
**           - Text displayed in GUI conversion box is limited in number of lines
**             until the very end to speed up scrolling on long conversions.
**           - The GUI file list is not cleared the file list after a successful
**             conversion anymore in case the user wants to try some different
**             command-line options for the conversion.
**
** v2.01     4 SEP 2013
**           BUG FIXES
**           - Fixed significant memory leak in wmupdf.c (added wtextchars_free call in
**             wtextchars_text_inside()).  Was causing k2pdfopt to crash on conversions
**             of large PDF files.
**           - Better feedback after preview button is pressed.  Also works correctly
**             when re-sizing the window during a preview.
**           - Inserted 0.1-second sleep at end of k2gui_preview_start()--seems to
**             prevent occasional problems with the preview.
**             
** v2.00     2 SEP 2013
**           MAJOR NEW FEATURES
**           - Added a GUI for the MS Windows version.  In MS Windows, k2pdfopt now
**             has a dual-mode operation:  it runs as a GUI or a console program
**             depending on the situation and command-line arguments:  Use -gui
**             to force the GUI, -guimin to minimize the GUI, use -gui- for no GUI.
**             - Window size / position stored in K2PDFOPT_WINPOS env. var.
**             - Custom presets stored in K2PDFOPT_CUSTOM<n> env. vars. (<n>=1-4).
**             - As part of the GUI, printf() and aprintf() calls switched to k2printf().
**           - For PDF documents that contain text (not scanned), I now use
**             the MuPDF text analysis functions so that even on re-flowed documents
**             (i.e. not using native PDF output) I can include the text from
**             the source for searching and highlighting without resorting to OCR.
**             I treat this as a sort of "virtual OCR".  You explicitly turn it
**             on with -ocr m (m for MuPDF), though it is on by default since it
**             adds very little time to the PDF file processing.  It is
**             not necessary if you are using native PDF output (-n).
**             Requested at http://www.mobileread.com/forums/showthread.php?p=2513005#post2513005
**
**           OTHER NEW FEATURES
**           - Windows 64-bit version compiled with gcc 4.7.3 (on core i5-670).
**             The optimized Windows 32-bit version still uses gcc 4.6.3 (it does
**             not compile correctly in gcc 4.7.3 on my MinGW platform due to some
**             issue with with the Tesseract library).  The generic Windows 32-bit
**             version uses gcc 4.7.3.
**           - Compiled with latest versions of MuPDF (v1.3), OpenJPEG, FreeType,
**             Turbo-JPEG, PNG, and Z libraries.
**           - A new option, -bmp, will write out a preview bitmap of the specified
**             page, e.g. -bmp 2 will write out the 2nd converted page to the file
**             k2pdfopt_out.png (sorry, the bitmap file name cannot be specified).
**           - New option -sp simply echoes the page counts of the source files
**             and exits.
**           - OCR text can be written to a separate text file now with the
**             -ocrout option.  E.g. -ocrout %s.txt will write the OCR from
**             myfile.pdf to myfile.txt.
**             Requested at http://www.mobileread.com/forums/showthread.php?p=2558909
**             and via e-mail on 3 June 2013.
**
**           SOURCE CODE RE-WRITE
**           - I re-did the way a number of data structures store information
**             (BMPREGION, TEXTROWS) and eliminated the BREAKINFO structure.
**             This simplifies several function calls and makes it a bit more
**             straightforward to follow the flow of the software and to expand
**             the functionality.
**           - The way gaps are placed between regions of text has been
**             substantially re-done.  I have done extensive regression testing,
**             and overall I think things are improved, but people may find
**             minor changes in behavior from v1.66.
**
**           BUG FIXES
**           - In k2pdfopt_settings_set_margins_and_devsize(), doesn't flush the
**             master bitmap unless the width changes.
**           - Mode -f2p -2 (put every new red-box region onto a new page) no
**             longer add any gap between these regions, so that the page size
**             is exactly the region size.
**           - When expected numeric arguments on the command-line don't occur
**             (e.g. -g is not followed by a numeric value), that parameter is not
**             set, a warning is printed, and the non-numeric argument is processed
**             as if it were the next argument on the command line (pointed out
**             by Jens Wallauer, 6-12-13).
**           - Modified tessedit.cpp in Tesseract library to automatically detect
**             if .cube files are present so that it uses them even for multiple
**             language specification.  Correspondingly modified ocrtess_init()
**             in willus library.
**           - v1.65 was crashing after not finding the Tesseract language file
**             and switching to GOCR:
**             http://www.mobileread.com/forums/showpost.php?p=2483500&postcount=393
**             This is fixed (tesseract status now stored in global static variable
**             in k2ocr.c).
**
**           NOTES
**           - This version still does not have unicode / wide character file
**             name handling in MS Windows (file names are stored in 8-bit ASCII
**             strings, as before).
**
** v1.66     23 JUNE 2013
**           NEW FEATURES
**           - Option -bp+ will break pages between the green regions as marked by
**             the -sm option (feature request from 6-2-13 e-mail).
**
**           BUG FIXES
**           - Option -mode def correctly sets margins to zero instead of 0.25.  It
**             also now correctly turns off native mode and landscape mode.
**           - Prevents infinite OCR font sizes from being written to PDF file.
**             (5-26-2013 private message at mobileread.com about a problem
**              converting a DjVu file.)
**           - Fixed array-out-of-bounds issue when searching for the column
**             divider, particularly with blank pages.  See "v1.66 fix" in
**             k2proc.c.  Fixes these reported issues:
**                 1. 4-27-13 e-mail (alice.pdf)
**                 2. http://www.mobileread.com/forums/showthread.php?p=2558185
**                 3. 7-10-13 e-mail (failed on pages 1, 2, and 14).
**           - Fixed breakinfo_find_doubles() in breakinfo.c to avoid an infinite
**             loop situation.  See "v1.66 fix" notes.  Fixes this reported issue:
**                 1. 7-7-13 e-mail (pages 98 and 187 failed).
**           - Fixed bug in MuPDF library when fz_ensure_buffer() is called with
**             buf->cap==1 (results in infinite loop).  Reported in 6-18-13 e-mail
**             on a conversion that hung with -mode fw.
**
** v1.65     30 MAR 2013
**           NEW FEATURES / OPTIONS
**           - Added Kobo Glo and Kobo Touch device settings.
**             (http://www.mobileread.com/forums/showpost.php?p=2441354&postcount=336)
**           - Re-vamped the bmp_source_page_add() function so that the
**             logic that breaks the page out into displayable rectangular
**             regions can be used in other places (e.g. by the OCR fill-in
**             function).
**           - Added option -ocrcol which sets the max number of columns for
**             processing with OCR (if different from the -col value).  You would
**             use this if you want to OCR a PDF file using -mode copy, but
**             the file has multiple columns of text.
**             (http://www.mobileread.com/forums/showpost.php?p=2442523&postcount=341)
**           - Added option -rsf (row-split figure-of-merit) which controls a
**             new algorithm which goes back and looks for rows of text which
**             should be split into two (or three) separate rows.  This is meant
**             to help catch those cases where k2pdfopt should have split apart
**             two rows of text but did not because of a small amount of overlap.
**             See breakinfo_find_doubles() in breakinfo.c.
**
**           LIBRARY UPDATES
**           - Compiled with latest versions of major libraries:  MuPDF 1.2,
**             DjVu 3.5.25.3, FreeType 2.4.11, Turbo JPEG 1.2.1, PNG 1.5.14,
**             Z-lib 1.2.7.
**           - Linux version now compiled with gcc 4.7.2 in Ubuntu 12.
**
**           TWEAKS
**           - Clarified usage for -vb in k2usage.c
**           - Changed "destination" to "E-reader" in places on the k2 interactive
**             menu and device menu.
**           - Put "disclaimer" in OCR usage which clarifies the purpose.
**           - Default crop margins are now zero (was 0.25 inches).  This was
**             confusing too many people.
**             (http://www.mobileread.com/forums/showpost.php?p=2456032&postcount=352)
**           - In bmp_region_vertically_break(), different width regions and
**             regions with different ending/starting row heights cause
**             a vertical gap to be inserted in the output.
**
**           BUG FIXES
**           - Call k2pdfopt_settings_sanity_check() once per source document.
**             This fixes a crash when converting multiple files.
**             (Certain vars weren't getting correctly initialized on the
**              2nd, 3rd, etc. conversion files.)
**             (http://www.mobileread.com/forums/showpost.php?p=2409726&postcount=317)
**           - Fixed array-out-of-bounds access in k2proc.c
**             (bmpregion_find_multicolumn_divider function) which occasionally
**             caused k2pdfopt to terminate abnormally (typically when converting
**             mostly blank pages).
**             (http://www.mobileread.com/forums/showpost.php?p=2456548&postcount=356)
**           - Fixed k2pdfopt_proc_one() in k2file.c so that native PDF output
**             is turned off if the source file is not PDF (e.g. DjVu conversion).
**           - Fixed spacing between regions with -vb -2 or -vb -1 (gap between
**             pages where new chapter starts, for example--font change, etc.).
**             (http://www.mobileread.com/forums/showpost.php?p=2373550&postcount=292)
**           - Minimum width in vertical line detection is now 1 pixel.
**             (http://www.mobileread.com/forums/showpost.php?p=2452356&postcount=345)
**           - Better diagnostic output on TESSDATA_PREFIX env var.
**           - Fixed native PDF output so that scientific notation is not allowed
**             in PDF clipping commands.  This was causing native conversions
**             not to work correctly in some cases.
**             (http://www.mobileread.com/forums/showpost.php?p=2467063&postcount=371)
**
** v1.64a    5 JAN 2013
**           - Fixed bug in Native PDF output introduced in v1.64.
**             (stream_deflate function in wmupdf.c)
**
** v1.64     4 JAN 2013 
**           - Native PDF output changed so that source pages are converted
**             to XObjects (Form type).  This should be much more robust when
**             putting contents from multiple source pages onto a single
**             destination page.
**           - Added profile for Kindle paperwhite. (-dev kpw)
**           - The fontdata.c file in willus lib has been reduced to only one
**             font in order to reduce the size of the k2pdfopt binaries since
**             k2pdfopt only uses one font for the -sm option.
**           - The page width and height can now be specified in terms of
**             the trimmed source page width and height.  Use 't' for the
**             units, e.g. -w 1t -h 1t.  This would typically be used with
**             the -mode copy and/or -grid options.
**           - The -bp option can now take a numeric argument (inches) to
**             insert a gap (of that many inches) between each source page.
**           - There is now an interactive menu option for selecting the
**             OCR language training file (Tesseract OCR only).
**           - Fixed memory leak in bmpregion_find_multicolumn_divider().
**           - Fixed default value for -col in usage.
**           - Clarified -ocrlang usage.
**           - Compiled Linux versions with -static and -static-libstdc++
**             to hopefully reduce shared library incompatibilities.
**
** v1.63     20 DEC 2012
**           - Now supports OCR in multiple languages using Tesseract with
**             Unicode-16 text encoding so that the OCR text can be copy / pasted
**             into Unicode-aware applications.
**             To select the language for OCR:  -ocrlang (or -l).
**             Examples: -ocrlang eng (English)
**                       -ocrlang fra (French)
**                       -ocrlang chi_sim (Chinese simplified)
**             [Note that using the -ocrvis t option will not show the
**              OCR text correctly for any character above unicode value
**              255 since I do not use any embedded fonts, but the text
**              will convert to the correct Unicode values when copy / pasted.]
**           - Tesseract "cube" files are automatically checked so that the
**             best OCR detection mode is selected.
**           - Updated wmupdf.c in willus lib to account for both CropBox
**             and MediaBox to determine page origin (fixes user-reported
**             bug in native output mode for pages with non-zero MediaBox
**             origins).
**           - Made changes to multicolumn divider finder to improve the
**             speed.  Includes counting pixels by column rather than row,
**             making use of trimmed column boundaries, and using a
**             2-D pixel count array.  Resulting code runs ~ 5 - 10% faster
**             on average in my regression tests.
**           - Removed dprintf() and fsincos() from willus lib to prevent
**             minor compiling problems on some platforms.  Fixed some other
**             minor issues for kindlepdfviewer.
**
** v1.62     15 NOV 2012
**           CODE RE-ORGANIZATION
**           - This was largely motivated by the kindlepdfviewer app which
**             now uses k2pdfopt code:
**                    https://github.com/hwhw/kindlepdfviewer
**           - Moved the bulk of the code to a k2pdfopt library consisting
**             of 21 source modules.  The main k2pdfopt.c program is now
**             only about 150 lines.
**           - The willus and k2pdfopt libraries have options to compile
**             so that they don't access any other 3rd-party library calls.
**             Seach for "THIRD PARTY" in willus.h.
**           - There is now a K2PDFOPT_KINDLEPDFVIEWER macro which can
**             be defined in k2pdfopt.h to make the code more friendly for
**             the kindle viewer app. For example, compiling kview.c with
**             all third-party libs disabled and K2PDFOOPT_KINDLEPDFVIEWER
**             defined results an executable about 300 KiB in size in Windows.
**
**           NEW FEATURES
**           - New -neg option inverts the output to be white on black
**             ("night mode").  Note that figures and photographs are not
**             distinguished, so they will also be inverted.
**           - Native mode now defaults to off and -n turns it on no matter
**             what (disables text wrapping and OCR).
**
**           BUG FIXES
**           - Setting re-initialized before final call to k2parsecmd.
**           - Fixed minor memory leak in willus lib (vector_nd_free).
**           - Safeguard against possible infinite loop in willus lib
**             (bmp_more_rows).
**           - Word spacing history initialized before each new document.
**           - The XObjects dictionary is now merged when crop boxes from
**             multiple source pages are put onto the same output page.
**             This should improve the native PDF output in certain cases.
**
** v1.61a    5 NOV 2012
**           - Fixed an issue with strtok_r in Tesseract 3.02.02 that was
**             causing k2pdfopt.exe to crash in Windows XP 32-bit.
**           - Option "u" in the user menu to show the command-line usage
**             works again.
**
** v1.61     3 NOV 2012
**           - Some user menu options were not taking effect.  This is fixed.
**           - Compiled with tesseract 3.02.02 and Leptonica 1.69.
**           - User "help" menu fits in 25-line window now.
**
** v1.60     1 NOV 2012
**           MAJOR NEW FEATURES
**           - Option to keep native PDF contents (see full details under
**             command-line option usage under -n for "native").
**             User menu option "n".  This is only available if k2pdfopt
**             has been compiled with MuPDF.
**           - New grid option grids the source page into a fixed number
**             of rows and columns with some overlap.  E.g. -grid 2x2.
**             This option works well with the -n option above.
**             This is under user menu option "mo".
**
**           NEW FEATURES / CHANGES
**           - The new -dev option selects device profiles (there are
**             only two so far:  kindle2 and nook simple touch).
**             The user menu option for -dev is "d".
**           - The new -mode option selects a particular mode of operation
**             that is shorthand for a number of options.  There are
**             currently four modes:  default (def), copy, 2-column (2col),
**             and fw (fit width).  E.g. "-mode copy" makes k2pdfopt behave
**             just like my "pdfr" program (thus eliminating the need for me
**             to distribute pdfr separately).  And "-mode fw" makes k2pdfopt
**             behave very much like sopdf's fit width option.  For more details,
**             see the full command-line usage (k2pdfopt -?).
**             The user menu option for -mode is "mo".
**           - The user input system was revamped so that the menu options
**             build up a set of command-line arguments that are passed to
**             the main program.  This way the user can easily see the
**             command-line options that match the selections from the menu.
**           - Command line arguments can be put in directly at the user
**             input menu (anything beginning with a - is considered to be
**             a command-line option and is appended to the list).
**           - At the user input menu, '-' will clear all menu-selected
**             options, '--' will clear the command-line options actually
**             entered at the command-line, and '---' will clear the options
**             from the K2PDFOPT environment variable.
**           - The -vb option can now take -2 as an argument, which indicates
**             that the vertical spacing is to be exactly preserved from the
**             source document.
**           - New -t option to specify trimming or no trimming (-t-) of
**             excess white space.  Default is to trim white space.
**           - New -dpi option is same as -odpi.
**           - The -w and -h options can take values in inches or cm now
**             instead of pixels, and they can be negative to specify that
**             the device size should be scaled from the source page size.
**           - With -w and -h set to -1 to follow the source page sizes,
**             k2pdfopt can now handle varying destination page sizes
**             (see set_margins_and_devsize function).
**           - The -m and -om command-line arguments can now be a comma-
**             -delimited list of margins: left,top,right,bottom.
**             E.g. -m 0.5,1.0,0.5,1.0.
**           - The "Author", "Title", and "CreationDate" fields in the
**             source PDF file are now correctly passed on to the output
**             file.  If there is no Title field in the source, the base
**             output file name is used.
**           - Specifying -f2p -2 will now also set -vb -2 automatically.
**             Use -fp -2 -vb -1 to revert to v1.51 behavior.
**
**           BUG FIXES
**           - Fixed OCR word placement bug for tall (centered) bitmaps.
**           - Make sure if crop boxes are used that padding/corner marking
**             is turned off and OCR is turned off.
**           - Fixed some issues when switching to Ghostscript and when
**             processing a folder full of bitmaps.
**           - Should work with postscript (.eps or .ps) files now
**             (requires Ghostscript; output file is still PDF).
**           - Fixed bmpregion_find_vertical_breaks() bug where it wasn't
**             always correctly interpreting the last section of a region.
**           - The -vls option works correctly when combined with -vb -1
**             and no text wrapping now (vls_test.pdf).
**           - Eliminated divide by zero issue in bmpregion_is_clear() when
**             gt_in gets too small.
**           - Put call to wrapbmp_flush() in publish_master() when flushall
**             is set (fixes bug reported by hm88 on mobileread)
**           - Adjusted critierion for too thick / too thin hyphen in
**             bmpregion_hyphen_detect() to be more correct and to allow
**             for slightly thinner hyphens.
**           - If mupdf can't open the file in Windows, I try the 8.3 alternative
**             file name.  This solved a problem involving a path that had a
**             non-traditional (non-ASCII) character in it.
**           - Fixed bug in detect_vertical_lines() that caused problems
**             in some cases on 64-bit versions.
**
** v1.51     9-21-12
**           NEW FEATURES
**           - New option -jf for special figure justification (under
**             "j" in interactive menu).
**           - -f2p option applies to small figures as well as tall figures.
**           - Compiled w/MuPDF v1.1 and Freetype v2.4.10.
**           - Source can be built without DjVuLibre library (see HAVE_DJVU
**             macro) or MuPDF (see HAVE_MUPDF macro).  If no MuPDF, then
**             Ghostscript must be installed to process PDF.
**           - Option -o (used to control overwrite) now sets the output
**             name via a formatting string.
**             Example #1: -o %s_k2opt
**               This is the default and and appends _k2opt to the source name.
**               (%s is replaced by the base name of the source file.)
**             Example #2: -o out%04d
**               In this case, each subsequent output file (assuming you
**               specify more than one input file) will be out0001.pdf,
**               out0002.pdf, out0003.pdf, ...
**
**           CHANGES(!) TO COMMAND-LINE OPTIONS/INPUT MENU
**           - I generally like to avoid doing this for backwards compatibility,
**             but I felt some changes were overdue.
**             COMMAND-LINE
**             * The overwrite option is now controlled with -ow instead of
**               -o since I wanted -o to specify the output name format.
**             * The new command-line option for setting OCR visibility (was -wc)
**               is -ocrvis.  E.g. "-ocrvis st" will show the 's'ource document
**               and the OCR 't'ext.
**             * The -gtcmax option has been changed to -cgmax to be more
**               consistent.  It's also been moved under the "co" options
**               in the interactive menu.  Meaning of -gtc option has been
**               clarified somewhat.
**             INTERACTIVE MENU
**             * Revamped the user input menu a bit by grouping things under
**               certain options in order to reduce the number.
**               > "o" now sets the output name format string and you
**                 now set the output DPI (-odpi) under "d" for device resolution.
**               > "g" (gamma correction), "s" (sharpening), and "wt" (white
**                  threshold) are all now under "cs" for contrast/sharpen.
**               > "de" (defect size), "evl" (erase vertical lines), and "gs"
**                  (ghostscript) are all under "s" for Special options.
**               > "mc" (mark corners) is now set under "pd" (padding/marking).
**               > "ws" (word spacing) is now set under "w" (wrap text).
**
**           BUG FIXES
**           - OCR text now rendered at a more uniform height.
**           - Box around OCR text options fixed (-ocrvis b).
**           - Fixed bug in text re-flow when a hyphen is detected.
**           - Fixed interpretation of -ocrhmax so that it is based on the
**             source dimension.
**           - Min figure height from -jf option is applied in the
**             bmpregion_find_vertical_breaks() function.
**           - Interactive menu should be smarter about when you try to enter
**             a file name at the prompt (fixed a bug where it seemed to
**             not accept a file you would type in at the prompt even though
**             it would eventually process it).
**           - No longer needs Jasper or GSL libs for build.
**           - Tightened up the pdfwrite_bitmap function in willus lib.
**           - Cleaned up / revised source code and build instructions for
**             a simpler build (eliminated two basically unneeded libraries).
**           - OCR options are ignored in versions compiled without OCR.
**
** v1.50     9-7-12
**           MAJOR NEW FEATURE: OCR
**           - For PDFs in English, added optical character recognition
**             using two different open source libraries:  Tesseract v3.01
**             (http://code.google.com/p/tesseract-ocr/) and GOCR v0.49
**             (http://jocr.sourceforge.net).  The OCR'd text
**             is embedded into the document as invisible ASCII text
**             stored in the same location as the bitmapped words,
**             exactly like some document scanning software works
**             (e.g. Canon's Canoscan software).  This allows the resultant
**             PDF document to be searched for text, assuming the OCR
**             is successful (won't always be the case, but should
**             work reasonably well if the source text is clear enough).
**           - Tesseract works far better than GOCR, but requires that
**             you download the "trained data" file for your language
**             from http://code.google.com/p/tesseract-ocr/downloads/list
**             and point the environment var TESSDATA_PREFIX to the root
**             folder for your trained data.  If the tesseract trained
**             data files are not found, k2pdfopt falls back to GOCR.
**             E.g. if your data is in c:\tesseract-ocr\tessdata\...
**             then set TESSDATA_PREFIX= c:\tessseract-ocr\
**           - I wrestled with this, but the default for OCR (for now)
**             is for it to be turned off.  You must explicitly turn it
**             on with the -ocr command-line option or "oc" at the
**             interactive menu.  I did this because it does significantly
**             slow down processing (about 20 words/second on a fast PC
**             using Tesseract).
**           - -wc option sets the OCR word color (visibility)
**             (e.g. -wc 0 for invisible OCR text, the default).
**           - -ocrhmax option sets max height of OCR'd word in inches.
**
**           OTHER NEW FEATURES:
**           - Added -evl option (menu item "e") to erase vertical lines.
**             This allows the option, for example, to get rid of
**             column divider lines which often prevent k2pdfopt from
**             properly separating columns and/or wrapping text.
**           - Detects and eliminates hyphens when wrapping text.  Turn
**             this off with -hy- ("w" interactive menu option).
**           - New option -wrap+ option will unwrap/re-flow narrow columns
**             of text to your wider device screen (typically desired
**             on a Kindle DX, for example).  Best if combined with -fc-.
**           - There is now a max column gap threshold option so that
**             columns are not detected if the gap between them is too large.
**             (-gtcmax command-line option).  Default = 1.5.
**           - New option -o controls when files get overwritten.  E.g.
**             -o 10 tells k2pdfopt not to overwrite any existing files
**             larger than 10 MB without prompting (the default).
**           - New option -f2p ("fit to page") can be used to fit tall
**             figures or the "red-boxed" regions (when using -sm) onto
**             single pages.
**
**           BUG FIXES / MISC
**           - Fixed description of -whitethresh in usage (had wrong default).
**           - Interactive menu more obvious about what files are specifed
**             and allows wildcards for file specification.
**           - Fixed bug in word_gaps_add() where the gap array was getting
**             erroneously filled with zeros, leading to some cases where
**             words got put together with no gap between them during text
**             wrapping/re-flow.
**           - The -sm option (show marked source) now works correctly when
**             used with the -c (color) option.
**           - Removed the separate usage note about Ghostscript and put it
**             under the -gs option usage.
**           - Fixed bug where zero height bitmap was sometimes passed to
**             bmp_src_to_dst().  Also rounded off (rather than floor()-ing)
**             scaling height used in the bitmap passed to bmp_src_to_dst().
**           - Due to some minor bitmap rendering improvements, this version
**             seems to be generally a little faster (~2-4%) compared to
**             v1.41 (under same conditions--no OCR).
**               
** v1.41     6-11-2012
**           IMPROVEMENTS
**           - Compiled w/MuPDF v1.0.
**           - Tweaked the auto-straightening algorithm--hopefully more
**             accurate and robust.  Now straighten even if only tilted
**             by 0.1 degree or more.
**           - Improved auto-contrast adjust algorithm and added option
**             to force a contrast setting by suppying a negative
**             value for -cmax.  Does a better job on scans of older
**             documents with significantly yellowed or browned pages.
**           - Options -? and -ui- when specified together now correctly
**             echo the entire usage without pausing so that you can
**             redirect to a file (as claimed in the usage for -?).
**
**           BUG FIXES
**           - Fixed bug where the column finding algorithm became far
**             too slow on certain types of pages (to the point where
**             k2pdfopt appeared to have crashed).
**           - Fixed bug where k2pdfopt wasn't working correctly when
**             the -c option was specified (color output).
**           - Fixed bug where if max columns was set to 3 in the
**             interactive menu, it didn't get upgraded to 4.
**           - Fixed memory leak in bmpregion_add() (temp bitmap
**             wasn't getting freed).
**           - Fixed memory leak in bmp_src_to_dst() (temp 8-bit
**             bitmap not getting freed).
**           - Fixed memory leak in bmpregion_one_row_wrap_and_add()
**             (breakinfo_free).
**           - Check for zero regions in breakinfo_compute_row_gaps()
**             and breakinfo_compute_col_gaps().
**           - Autostraighten no longer inadvertently turned on
**             when debugging.
**
** v1.40     4-3-2012
**           - This is probably my most substantial update so far.
**             I did a re-write of many parts of the code and
**             consequently have spent many hours doing regression
**             testing.
**           - Major new features:
**             * Does true word wrap (brings words up from the
**               next line if necessary).
**             * Preserves indentation, justification, and vertical
**               spacing more faithfully.  Overall, particularly for
**               cases with text wrapping, I think the output looks
**               much better.
**             * Ignores defects in scanned documents.
**             * Compiled with all of the very latest third party
**               libraries, including mupdf 0.9.
**             * v1.40 is about 4% faster than v1.35 on average
**               (PC x64 version).
**           - New justification command-line option is:
**                 -j [-1|0|1|2][+/-]
**             Using -1 tells k2pdfopt to use the document's own
**             justification.  A + after will attempt to fully
**             justify the text.  A - will force no full justification.
**             Nothing after the number will attempt to determine
**             whether or not to use full justification based on
**             if the source document is fully justified.
**           - The default defect size to ignore in scanned documents
**             is a specified user size (default is 1 point).  The
**             command-line option is -de (user menu option "de").
**           - Command line options -vls, -vb, and -vs control
**             vertical spacing, breaks, and gaps.  They are all
**             under the interactive user menu under "v".
**           - Line spacing is controlled by -vls.
**             Example:  -vls -1.2 (the default) will preserve
**             the default document line spacing up to 1.2 x
**             single-spaced.  If line spacing exceeds 1.2 x in the
**             source document, the lines are spaced at 1.2 x.
**             The negative value (-1.2) tells k2pdfopt to use it
**             as a limit rather than forcing the spacing to be 
**             exactly 1.2 x.  A positive value, on the other hand,
**             forces the spacing.  E.g. -vls 2.0 will force line
**             spacing to be double-spaced.
**           - Regions are broken up vertically using the new -vb
**             option.  It defaults to 2 which breaks up regions
**             separated by gap 2 X larger than the median line gap.
**             For behavior more like v1.35, or to not break up the
**             document into vertical regions, use -vb -1.  Vertical
**             breaks between regions are shown with green lines when
**             using -sm.
**           - The new -vs option sets the maximum gap between regions
**             in the source document before they are truncated.
**             Default is -vs 0.25 (inches).
**           - Added menu option for -cg under "co".
**           - Reduced default min column gap from 0.125 to 0.1 inches.
**           - The -ws (word spacing threshold) value is now specified
**             as a fraction of the lowercase letter height (e.g. a
**             small 'o').  The new default is 0.375.
**
** v1.35     2-15-2012
**           - Changed how the columns in a PDF file are interpreted
**             when the column divider moves around some.  The column
**             divider is now allowed to move around on the page
**             but still have the columns be considered contiguous.
**             This is controlled by the -comax option.  Use
**             -comax -1 to revert to v1.34 and before.  The
**             default is -comax 0.2.  See example at:
**             http://willus.com/k2pdfopt/help/column_divider.shtml
**           - Added nice debugging tool with the -sm command-line
**             option ("sm" on interactive menu) which shows marked
**             source pages so you can clearly see how k2pdfopt
**             is interpreting your PDF file and what affect the
**             options are having.
**           - The last line in a paragraph, if shorter than the
**             other lines significantly, will be split differently
**             and not fully justified.
**           - Modified the column search function to better find
**             optimal gaps.
**           - The height of a multi-column region is calculated
**             more correctly now (does not include blank space,
**             and both columns must exceed the minimum height
**             requirement).
**           - Text immediately after a large rectangular block
**             (typically a figure) will not be wrapped, since it
**             is often the axis labels for the figure.
**           - Fixed array-out-of-bounds bug in 
**             bmpregion_wrap_and_add().
**           - colcount and rowcount allocated only once per page.
**
** v1.34a    12-30-2011
**           - Some build corrections after the first release of
**             v1.34 which had issues in Linux and Windows.
**           - Fixed interpretation of -jpg flag when it's the last
**             command-line option specified.
**
** v1.34     12-30-2011
**           - I've collected enough bug reports and new feature
**             requests that I decided to do an update.
**           - Added -cgr and -crgh options to give more control
**             over how k2pdfopt selects multi-column regions.
**           - Don't switch to Ghostscript on DJVU docs.
**           - Continues processing files even if has an error on
**             one page.
**           - Fixed bug in orientation detection (minimum returned
**             value is now 0.01 so as not to kill the average).
**           - Added document scale factor (-ds or "ds" in menu)
**             which allows users to correct PDF docs that are the
**             wrong size (e.g. if your PDF reader says your
**             document is 17 x 22 inches when it should be
**             8.5 x 11, use -ds 0.5).
**           - Fixed bug in break_point() where bp1 and bp2 did not
**             get initialized correctly.
**
** v1.33     11-11-2011
**           - Added autodetection of the orientation of the PDF
**             file.  This is somewhat experimental and comes with
**             several caveats, but I have made it the default
**             because I think it works pretty well.
**             Caveat #1:  It assumes the PDF/DJVU file is mostly
**             lines of text and looks for regularly spaced lines
**             of text to determine the orientation.
**             Caveat #2:  If it determines that the page is
**             sideways, it rotates it 90 degrees clockwise, so it
**             may end up upside down.
**           - The autodetection is set with the -rt command-line
**             option (or the "rt" menu option):
**             1. Set it to a number to rotate your PDF/DJVU file
**                that many degrees counter-clockwise.
**             2. Set it to "auto" and k2pdfopt will examine up
**                to 10 pages of the file to determine the
**                orientation it will use.
**             3. Set it to "aep" to auto-detect the rotation of
**                every page.  If you have different pages that
**                are rotated differently from each other within
**                one file, you can use this option to try to
**                auto-rotate each page.
**             4. To revert to v1.32 and turn off the orientation
**                detection, just put -rt 0 on the command line.
**           - Added option to attempt full justification when
**             breaking lines of text.  This is experimental and
**             will only work well if the output dpi is chosen so
**             that rows break approximately evenly.  To turn on,
**             use the "j" option in the interactive menu or the
**             -j command-line option with a + after the selection,
**             e.g.
**                 -j 0+  (left/full justification)
**                 -j 1+  (center/full justification)
**                 -j 2+  (right/full justification)
**
** v1.32     10-25-2011
**           - Make sure locale is set so that decimal marker is
**             a period for numbers.  This was causing problems
**             in locales where the decimal marker is a comma,
**             resulting in unreadable PDF output files.  This
**             was introduced by having to compile for the DJVU
**             library in v1.31.
**           - Slightly modified compile of DJVU lib (re: locale).
**           - Remove "cd" option from interactive menu (it was
**             obsoleted in v1.27).
**           - Warn user if source bitmap is excessively large.
**           - Print more info in header (compiler, O/S, chip).
**
** v1.31     10-17-2011
**           - Now able to read DJVU (.djvu) files using ddjvuapi
**             from djvulibre v3.5.24.  All output is still PDF.
**           - Now offer generic i386 versions for Win and Linux
**             which are more compatible w/older CPUs, and fixed
**             issue with MuPDF so it doesn't crash on older CPUs
**             when compiled w/my version of MinGW gcc.
**
** v1.30     10-4-2011
**           - Just after I posted v1.29, I found a bug I'd introduced
**             in v1.27 where k2pdfopt didn't quit when you typed 'q'.
**             I fixed that.
**           - Made user menu a little smarter--allows different
**             entries depending on whether a source file has already
**             been specified.
**
** v1.29     10-4-2011
**           - Input file dpi now defaults to twice the output dpi.
**             (See -idpi option.)
**           - Added option to break input pages at the end of each
**             output page.  ("Break pages" in menu or -bp option.)
**           - Set dpi minimums to 50 for input and 20 for output.
**
** v1.28     10-1-2011
**           - Fixed bug that was causing vertical stripes to show
**             up on Mac and Linux version output.
**           - OSX 64-bit version now available.
**
** v1.27     9-25-2011
**           - Changed default max columns to two.  There were
**             too many cases of false detection of sub-columns.
**             Use -col 4 to detect up to 4 columns (or select
**             the "co" option in the user menu).
**           - The environment variable K2PDFOPT now can be
**             use to supply default command-line options.  It
**             replaces all previous environment variables,
**             which are now ignored.  The options on the
**             command line override the options in K2PDFOPT.
**           - Added -rt ("rt" in menu) option to rotate the source
**             pages by 90 (or 180 or 270) degrees if desired.
**           - Default startup is now to show the user menu rather
**             than command line usage.  Type '?' for command line
**             usage or use the -? command line option to see usage.
**           - Added three new "expert-mode" options for controlling
**             detection of gaps between columns, rows, and words:
**             -gtc, -gtr, -gtw.  The -gtc option replaces
**             the -cd option from v1.26.  These can all be set
**             with the "gt" menu option.  Use the "u" option for
**             more info (to see usage).
**           - In conjunction with the new "expert-mode" options,
**             I adjusted how gaps between columns, rows, and words
**             are detected and adjusted the defaults to hopefully
**             be more robust.
**           - You can now enter all four margin settings (left,
**             top, right, bottom) from the user input menu for
**             "m" and "om".
**           - Added -x option to get k2pdfopt to exit without asking
**             you to press <Enter> first.
**
** v1.26     9-18-2011
**           - Added column detection threshold input (-cd).  Set
**             higher to make it easier to detect multiple columns.
**           - Adjusted the default column detection to make column
**             detection a bit easier on scanned docs with
**             imperfections.
**
** v1.25     9-16-2011
**           - Smarter detection of number of TTY rows.
**
** v1.24     9-12-2011
**           - Input on user menu fixed not to truncate file names
**             longer than 32 chars for Mac and Linux.
**              
** v1.23     9-11-2011
**           - Added right-to-left (-r) option for scanning pages.
**
** v1.22     9-10-2011
**           - First version compiled under Mac OS X.
**           - Made some changes to run on OS X.  Kludgey, but works.
**             You have to double-click the icon and then drag a file
**             to the display window and press <Enter>.  I've made
**             linux work similarly.
**           - Since Mac and Linux shells default to black on white,
**             I've made the the text colors more friendly to that
**             scheme for linux and Mac.  Use -a- to turn off text
**             coloring altogether, or set the env variable
**             K2PDFOPT_NO_TEXT_COLORING.
**           - Re-vamped the print out of the cmd-line options some.
**
** v1.21     9-7-2011
**           - Moved some bmp functions to standard library.
**           - JPEG images always done at 8 bpc (no dithering).
**           - Fixed dithering of 1-bit-per-colorplane images.
**
** v1.20     9-2-2011
**           - Added dithering for bpc < 8.  Use -d- to turn off.
**           - Adjusted gamma correction algorithm slightly (so that
**             pure white stays pure white).
**
** v1.19     9-2-2011
**           - Added gamma adjust.  Setting to a value lower than 1.0
**             will darken the font some and appear to thicken it up.
**             Default is 0.5. Thanks to PaperCrop for the idea.
**           - Interactive menu now uses letters for the options.
**             This should keep the option choices the same even if
**             I add new ones, and now the user can enter a page range
**             as the final entry.
**
** v1.18     8-30-2011
**           - break_point() function now uses same white threshold
**             as all other functions.
**           - Added "-wt" option to manually specify "white threshold"
**             value above which all pixels are considered white.
**           - Tweaked the contrast adjustment algorithm and changed
**             the max to 2.0 (was much higher).
**           - Added "-cmax" option to limit contrast adjustment.
**
** v1.17     8-29-2011
**           - Min region width now 1.0 inches.  Bug fixed when
**             output dpi set too large--it is now reduced so that
**             the output display has at least 1-inch of display.
**
** v1.16     8-29-2011
**           - Now queries user for options when run (just press
**             <Enter> to go ahead with the conversion).
**             Use -ui- to disable this (it is automatically disabled
**             when run from the command line in Windows).
**           - Fixed bug in MuPDF calling sequence that results in
**             more robust reading of PDF files. (Fixes the parsing
**             of the second two-column example on my web page.)
**           - Fixed bug in MuPDF library that prevented it from
**             correctly parsing encrypted sections in PDF files.
**             (This bug is not in the 0.8.165 tarball but it
**              was in the version that I got via "git".)
**             This only affected a small number of PDF files.
**           - New landscape mode (not the default) is enabled
**             with the -ls option.  This turns the output sideways
**             on the kindle, resulting in a more magnified display
**             for typical 2-column files.  Thanks to Taesoo Kwon
**             for this idea.
**           - Default PDF output is now much smaller--about half
**             the original size.  This is because the bitmaps are
**             saved with 4 bits per colorplane (same as the Kindle).
**             You can set this to 1, 2, 4, or 8 with the -bpc option.
**             Thanks to Taesoo Kwon and PaperCrop for this idea.
**           - Default -m value is now 0.25 inches (was 0.03 inches).
**             This ignores anything within 0.25 inches of the edge
**             of the source page.
**           - Now uses precise Kindle 2 (and 3?) display resolution
**             by default.  Thanks to the PaperCrop forum for pointing
**             out that Shift-ALT-G saves screenshot on Kindle.
**             The kindle is a weird beast, though--after lots of
**             testing, I figured out that I have to do the
**             following to get it to display the bitmaps with
**             a 1:1 mapping to the Kindle's 560 x 735 resolution:
**                 (a) Make the actual bitmap in the PDF file 
**                     563 x 739 and don't use the excess pixels.
**                     I.e. pad the output bitmap with 3 extra
**                     columns and 4 extra rows.
**                 (b) Put black dots in the corners at the 560x735
**                     locations, otherwise the kindle will scale
**                     the bitmap to fit its screen.
**             This is accomplished with the new -pr (pad right), -pb
**             (pad bottom), and -mc (mark corners) options.  The
**             defaults are -pr 3 -pb 4 -mc. 
**           - New -as option will attempt to automatically straighten
**             source pages.  This is not on by default since it slows
**             down the conversion and is somewhat experimental, but I've
**             found it to be pretty reliable and it is good to use on
**             scanned PDFs that are a bit tilted since the pages need
**             to be straight to accurately detect cropping regions.
**           - Reads 8-bit grayscale directly from PDF now for faster
**             processing (unless -c is specified for full color).
**           - Individual bitmaps created only in debug mode.
**             k2_src_dir and k2_dst_dir folders no longer needed.
**
** v1.15     8-3-2011
**           - Substantial code re-write, mostly to clean things up
**             internally.  Hopefully won't introduce too many bugs!
**           - Can handle up to 4 columns now (see -col option).
**           - Added -c for full color output.
**           - If column width is close too destination screen width,
**             the column is fit to the device.  Controlled with -fc
**             option.
**           - Optimized much of code for 8-bit grayscale bitmaps--
**             up to 50% faster than v1.14.
**           - Added -wrap- option to disable text wrapping.
**           - Can convert specific pages now--see -p option.
**           - Added margin ignoring options:  -m, -ml, -mr, -mt, -mb.
**           - Added options for margins on the destination device:
**                 -om, -oml, -omr, -omt, -omb.
**           - Min column gap now 0.125 inches and min column height
**             now 1.5 inches.  Options -cg and -ch added to control
**             this.
**           - Min word spacing now 0.25.  See -ws option.
**
** v1.14     7-26-2011
**           - Smarter line wrapping and text sizing based on custom options.
**             (e.g. should work better for any size destination screen
**              --not just 6-inch.)
**           - Bug fix.  -w option fixed.
**           - First page text doesn't butt right up against top of page.
**
** v1.13     7-25-2011
**           - Added more command-line options:  justification, encoding
**             type, source and destination dpi, destination width
**             and height, and source margin width to ignore.
**             Use -ui to turn on user input query.
**           - Now applies a sharpening algorithm to the output images
**             (can be turned off w/command-line option).
**
** v1.12     7-20-2011
**           - Fixed a bug in the PDF output that was ignored by some readers
**             (including the kindle itself), but not by Adobe's reader.
**             PDF files should be readable by all software now.
**
** v1.11     7-5-2011
**           - Doesn't put "Press <ENTER> to exit." if launched as a
**             command in a console window (in Windows).  No change to
**             Linux version.
**
** v1.10     7-2-2011
**           - Integrated with mupdf 0.8.165 so that Ghostscript is
**             no longer required!  Ghostscript can still be used/
**             will be tried if mupdf fails to decypher the pdf file.
**           - PDF page number count now much more reliable.
**
** v1.07     7-1-2011
**           - Fixed bugs in the pdf writing that were making the
**             pdf files incompatible with the kindle.
**           - Compiled w/gcc 4.5.2.
**           - Added smarter determination of # of PDF pages in source,
**             though it doesn't always work on newer PDF formats.
**             This can cause an issue with the win32 version because
**             calling Ghostscript on a page number beyond what is in
**             the PDF file seems to sometimes result in an exception.
**
** v1.06     6-23-2011
**           - k2pdfopt now first tries to find Ghostscript using the registry
**             (Windows only).  If not found, searches path and common folders.
**           - Compiled w/turbo jpeg lib 1.1.1, libpng 1.5.2, and zlib 1.2.5.
**           - Correctly sources single bitmap files.
**
** v1.05     6-22-2011
**           Fixed bug in routine that looks for Ghostscript.
**           Also, Win64 version now looks for gsdll64.dll/gswin64c.exe
**           before gsdll32.dll/gswin32c.exe.
**
** v1.04     6-6-2011
**           No longer requires imagemagick's convert program.
**
** v1.03     3-29-2010
**           Made some minor mods for Linux compatibility.
**
** v1.02     3-28-2010
**           Changed rules for two-column detection to hopefully avoid
**           false detection.  At least 0.1 inches must separate columns.
**
** v1.01     3-22-2010
**           Fixed some bugs with file names having spaces in them.
**           Added program icon.  Cleaned up screen output some.
**
** v1.00     3-20-2010
**           First released version.  Auto adjusts contrast, clears
**           edges.
**
*/
