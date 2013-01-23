char *k2pdfopt_version = "v1.64a";
/*
** k2version.c  K2pdfopt version number and history.
**
** Copyright (C) 2013  http://willus.com
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
** FUTURE MODIFICATIONS
**
** VERSION HISTORY
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
