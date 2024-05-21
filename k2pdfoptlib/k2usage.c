/*
** k2usage.c    K2pdfopt usage text and handling functions.
**
** Copyright (C) 2018  http://willus.com
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
*/

#include "k2pdfopt.h"

static char *usageintro=
"usage:  k2pdfopt [opts] <input pdf/djvu | folder>\n\n"
"(Or just drag a PDF or DJVU (.djvu) file to this icon.)\n\n"
"Attempts to optimize PDF (or DJVU) files (especially two-column ones) for\n"
"display on the Kindle (or other mobile readers/smartphones) by looking for\n"
"rectangular regions in the file and re-paginating them without margins and\n"
"excess white space.  Works on any PDF or DJVU (.djvu) file, but assumes it\n"
"has a mostly-white background.  Native PDF files (not scanned) work best.\n\n"
"If given a folder, k2pdfopt first looks for bitmaps in the folder and if\n"
"any are found, converts those bitmaps to a PDF as if they were pages of a\n"
"PDF file.  If there are no bitmaps in the folder and if PDF files are in\n"
"the folder, then each PDF file will be converted in sequence.\n\n"
"Output files are always .pdf and have _k2opt added to the source name by\n"
"default (see -o option to specify alternate output name.)\n\n";

static char *usageenv=
"K2PDFOPT environment variable\n"
"-----------------------------\n"
"You can supply command-line options via the environment variable K2PDFOPT,\n"
"for example,\n\n"
"     set K2PDFOPT=-ui- -x -j 0 -m 0.25\n\n"
"Command line options from the command line take precedence over the ones in\n"
"the environment variable K2PDFOPT.\n\n";

static char *k2pdfopt_options=
"-?[-] [pattern]   Show [don't show] usage only (no file processing).\n"
"                  If pattern is specified, only options with text matching\n"
"                  the pattern are shown.  The pattern can use * as a wild\n"
"                  card, e.g. -? -col.  Use -?- to turn off usage.\n"
"                  Combine with -ui- to get something you can redirect\n"
"                  to a file.\n"
"-a[-]             Turn on [off] text coloring (use of ANSI color codes) on\n"
"                  the screen output.  Default is on.\n"
/*
"-arlim <ar>       Set aspect ratio limit to avoid wrapping.\n"
*/
"-ac[-] [<aggressiveness>]  Auto crop.  For books or papers that have dark edges\n"
"                  due to copying artifacts, this option will attempt to\n"
"                  automatically crop out those dark regions so that k2pdfopt\n"
"                  can correctly process the source file.  The <aggressiveness>\n"
"                  factor is from 0 to 1.  Higher is more aggressive cropping.\n"
"                  Default if not specified is 0.1.  See also -m.\n"
"                  Default value is off (-ac-).\n"
"                  Note that autocropping does not work on cropped regions\n"
"                  created with -cbox.  See -dw for a discussion about this.\n"
"-as[-] [<maxdeg>] Attempt to automatically straighten tilted source pages.\n"
"                  Will rotate up to +/-<maxdegrees> degrees if a value is\n"
"                  specified, otherwise defaults to 4 degrees max.  Use -1 to\n"
"                  turn off. Default is off (-as -1 or -as-).\n"
"                  Note that autostraighten does not work on cropped regions.\n"
"                  See -dw for a discussion about this.\n"
"-author <author>   Set the author metadata / property of the PDF output\n"
"                  file(s). Default is to use the author of the source document\n"
"                  (-author \"\").\n"
"-bmp[-] <pageno>  Generate [do not generate] a bitmap rendering of converted\n"
"                  page number <pageno> and write it to file k2pdfopt_out.png.\n"
"                  If this option is used, no other files are written, i.e. the\n"
"                  complete conversion is NOT done--ONLY the bitmap file is\n"
"                  written.  If -sm is also specified, then the bitmap is of\n"
"                  marked source page <pageno>.  If -bmp-, then <pageno> is not\n"
"                  necessary.  Default is -bmp-.\n"
"-bp[+|-|--] [m|<inches>] Break [do not break] output pages at end of each input\n"
"                  page.  Default is -bp-.  If a numeric value is put after -bp,\n"
"                  then rather than breaking the output page at the end of each\n"
"                  input page, a gap is inserted of that many inches, e.g.\n"
"                  -bp 1 will insert a 1-inch gap between contents of each\n"
"                  input page.  Special option -bp+ will break the pages at\n"
"                  the green boundaries between region as marked by the -sm\n"
"                  option (see -sm).  If bookmark information is available\n"
"                  and -toc is specified (on by default) page breaks will be\n"
"                  inserted in the converted file at each bookmark unless -bp--\n"
"                  is specified.  If \"-bp m\" is specified, then a page break\n"
"                  is inserted after each major (red-box) section.  This can\n"
"                  help prevent text selection overlap problems in native output\n"
"                  mode.  See also -toc, -bpl.\n"
"-bpc <nn>         Set the bits per color plane on the output device to <nn>.\n"
"                  The value of <nn> can be 1, 2, 4, or 8.  The default is 4\n"
"                  to match the kindle's display capability.  This is ignored\n"
"                  if the -jpg option is specified.\n"
"-bpl <srcpagelist>   Insert page break in destination file before each source\n"
"                  file page listed in <srcpagelist>.  This has the same format\n"
"                  as the -p option.  See also -p, -bp, -toc, -toclist.  Default\n"
"                  is no page list.  Example:  -bpl 10,25,50,70,93,117,143.\n"
"                  This automatically sets -bp to it's default value (-bp-).\n"
"-bpm[<type>] <color>  Set a page break mark type and color.  This option allows\n"
"                  you to put colored marks in the PDF file to specify where to\n"
"                  break pages or where to avoid page breaks.  <type> is either\n"
"                  1 to force a page break or 2 to prevent a page break until\n"
"                  next mark.  <color> is an R,G,B triplet, 0-1 for each color\n"
"                  component, no spaces.  For example, to break the page\n"
"                  wherever the source file has a green dot or short green\n"
"                  horizontal line:  -bpm1 0,1,0.  Use <color> = -1 to clear.\n"
"                  If you omit the <type>, 1 is assumed.\n"
"-c[-]             Output in color [grayscale].  Default is grayscale.\n"
/*
"-cd <threshold>   Set column detection threshold.  Default = 0.01.  Range\n"
"                  is 0 to 100.  Higher makes it easier to detect columns.\n"
"                  If PDF is scanned and speckled, might set to .02 or .03.\n"
*/
"-cbox[<pagelist>|u|-] <cropbox>  Similar to the -grid option, but allows you to\n"
"                  specify exact crop boxes from the source page which will\n"
"                  then be processed as major (red-box) regions.  These regions\n"
"                  can then become individual output pages or can be processed\n"
"                  further (searched for columns, re-flowed, etc.) depending on\n"
"                  what other options are selected.  By default, they are\n"
"                  processed further, like every other major region.\n"
"                  You may specify the -cbox option multiple times to crop out\n"
"                  different parts of each source page, each crop being treated\n"
"                  as a major region.  See the -mode command.  To have each\n"
"                  crop box become a new page in the output file, for example,\n"
"                  use -mode crop, e.g.\n"
"                      k2pdfopt myfile.pdf -mode crop -cbox 2in,3in\n"
"                  <cropbox> has the format <left>,<top>,<width>,<height>\n"
"                  where all values are specified from the upper-left corner of\n"
"                  the source page, with units, like the -w and -h options,\n"
"                  except that the default units for -cbox are inches.  If only\n"
"                  <left> and <top> are specified, then <width> and <height>\n"
"                  extend to the edge of the page.\n"
"                  Example: -cbox 1in,1in,6in,9in (same as -cbox 1,1,6,9).\n"
"                      This specifies a crop box that is 6 x 9 inches and which\n"
"                      has an upper left corner which is 1 inch from the left\n"
"                      and top of the source page.\n"
"                  Use -cbox- to clear all cropboxes, which defaults back to\n"
"                  processing every page without any crop boxes.\n"
"                  You can use a page list, <pagelist>, to specify on which\n"
"                  pages to apply the cropboxes.\n"
"                  Examples:\n"
"                      -cbox5-51o ... applies the cropbox on pages 5,7,9,...,51.\n"
"                                     ('o' = odd.  Use 'e' for even.)\n"
"                      -cbox1,2-5,13,15  ... applies the cropbox on pages 1,2,3,\n"
"                                            4,5,13, and 15.\n"
"                      -cboxc <cropbox> ... applies <cropbox> to the cover image.\n"
"                                           (see -ci option.)\n"
"                  Be sure not to put a space between -cbox and the page list.\n"
"                  Use -cboxu to set a crop box for all unspecified pages.\n"
"                  E.g. -cbox1-10 <cbox1> -cboxu <cbox2> will apply <cbox1> to\n"
"                  all pages 1 to 10 and <cbox2> to all other pages.\n"
"                  The default is no crop boxes (-cbox-).  See also -m, -ac.\n"
"                  USAGE NOTE:  Once you specify -cbox at least one time, only\n"
"                  the crop boxes you specify (and any associated page ranges)\n"
"                  are processed/converted by k2pdfopt.  No other pages or\n"
"                  regions are processed.  So if you want to specify a special\n"
"                  cropbox for the first page, for example, but then have all\n"
"                  remaining pages treated entirely, you must specify this:\n"
"                      -cbox1 ...   -cboxu 0,0\n"
"                      (-cboxu 0,0 applies a full-page cropbox to all other\n"
"                        pages.  u = unspecified.)\n"
"                  The -cbox2- 0,0 will set the cropbox for pages 2 and beyond\n"
"                  to the full page size.\n"
"                  See also:  -ibox.\n"
"-cg <inches>      Minimum column gap width in inches for detecting multiple\n"
"                  columns.  Default = 0.1 inches.  Setting this too large\n"
"                  will give very poor results for multicolumn files.  See also\n"
"                  -cgmax.\n"
"-cgmax <inches>   Max allowed gap between columns in inches.  If the gap\n"
"                  between two regions exceeds this value, they will not be\n"
"                  considered as separate columns.  Default = 1.5.  Use -1 for\n"
"                  no limit (disable).  See also -cg.\n"
"-cgr <range>      Set column-gap range, 0 - 1.  This is the horizontal range\n"
"                  over which k2pdfopt will search for a column gap, as a\n"
"                  fraction of the page width.  E.g. -cgr 0.5 will search\n"
"                  from 0.25 to 0.75 of the page width for a column gap.\n"
"                  Set this to a small value, e.g. 0.05, to only search for\n"
"                  column breaks in the middle of the page.  Default = 0.33.\n"
"-ch <inches>      Minimum column height in inches for detecting multiple\n"
"                  columns.  Default = 1.5 inches.\n"
"-ci[-] <imagefile>   Specify a cover image for the first page of the converted\n"
"                  PDF.  <imagefile> can be a bitmap file (png or jpg) or can be\n"
"                  a page from a PDF file, e.g. myfile.pdf[34] would use page 34\n"
"                  of myfile.pdf.  You can just specify an integer, e.g. -ci 50\n"
"                  to use page 50 of the source file being converted as the\n"
"                  cover page.  Default is -ci-, which is no cover image.\n"
"                  NOTE:  -ci only works with bitmapped output--it does not\n"
"                         (yet) work with native PDF output.\n"
"-cmax <max>       Set max contrast increase on source pages.  1.0 keeps\n"
"                  contrast from being adjusted.  Use a negative value to\n"
"                  specify a fixed contrast adjustment.  Def = 2.0.\n"
"                  See also -er.\n"
"-col <maxcol>     Set max number of columns.  <maxcol> can be 1, 2, or 4.\n"
"                  Default is -col 2.  -col 1 disables column searching.\n"
"-colorbg (or -colorfg) <hexcolor>|<bitmap>[,<hexcolor>|<bitmap>[,...]]\n"
"                  Map the color white (background color, for -colorbg) or the\n"
"                  color black (text color, for -colorfg) to <hexcolor>,\n"
"                  where <hexcolor> is a 6-digit hex RRGGBB representation of a\n"
"                  color, e.g. ffffff for all white, 000000 for all black,\n"
"                  ff0000 for bright red, etc.  If <hexcolor> is not a grayscale\n"
"                  color, the -c (color output) option will be turned on\n"
"                  automatically.  This option only works with bitmapped output\n"
"                  (not native--see -n).  Grayscale colors between black and\n"
"                  white will be linearly interpolated between the specified\n"
"                  -colorbg and -colorfg colors.  If the source document has\n"
"                  colors, only (mostly) grayscale pixels are affected if ! is\n"
"                  put before the color, e.g. -colorbg !ffffd0\n"
"                  A bitmap can also be specified, e.g. -colorbg myfile.jpg.\n"
"                  In this case, the bitmap gets tiled in as the background.\n"
"                  If you specify a comma delimited list of colors (or bitmaps),\n"
"                  then consecutive rows of text are colored with the\n"
"                  consecutive colors.  This is a possible way to make the\n"
"                  rows of text easier to follow, e.g. -colorfg ff0000,00 will\n"
"                  color alternate rows of text red and black.\n"
"                  Default is -colorbg \"\" and -colorfg \"\" (no mappings).\n"
"-comax <range>    Stands for Column Offset Maximum.  The <range> given is as a\n"
"                  fraction of the width of a single column, and it specifies\n"
"                  how much the column divider can move around and still have\n"
"                  the columns considered contiguous.  Set to -1 to revert back\n"
"                  to how columns were treated in k2pdfopt v1.34 and before.\n"
"                  Default = 0.3.\n"
"-crgh <inches>    Set the min height of the blank area that separates regions\n"
"                  with different numbers of columns.  Default = 1/72 inch.\n"
"-d[-]             Turn on [off] dithering for bpc values < 8.  See -bpc.\n"
"                  Default is on.\n"
"-de <size>        Defect size in points.  For scanned documents, marks\n"
"                  or defects smaller than this size are ignored when bounding\n"
"                  rectangular regions.  The period at the end of a sentence is\n"
"                  typically over 1 point in size.  The default is 1.0.\n"
"-dev <name>       Select device profile (sets width, height, dpi, and corner\n"
"                  marking for selected devices).  Currently the selection is\n"
"                  limited.  <name> just has to have enough characters to\n"
"                  uniquely pick the device.  Use -dev ? to list the devices.\n"
"                  Default is -dev kindle2.\n"
"-dpi <dpival>     Same as -odpi.\n"
"-dr <value>       Display resolution multiplier.  Default = 1.0.  Using a\n"
"                  value greater than 1 should improve the resolution of the\n"
"                  output file (but will make it larger in file size).\n"
"                  E.g. -dr 2 will double the output DPI, the device width\n"
"                  (in pixels), and the device height (in pixels).\n"
"-ds <factor>      Override the document size with a scale factor.  E.g. if\n"
"                  your PDF reader says the PDF file is 17 x 22 inches and\n"
"                  it should actually be 8.5 x 11 inches, use -ds 0.5.  Default\n"
"                  is 1.0.\n"
#ifdef HAVE_LEPTONICA_LIB
"-dw[-] [<fitorder>] De-warp [do not de-warp] pages (uses Leptonica de-warp\n"
"                  algorithms).  Default is not to de-warp.  Does not work\n"
"                  for native mode output.  Optional <fitorder> specifies the\n"
"                  fit order for the dewarping curves.  Can be 2, 3, or 4.\n"
"                  Default is 4.\n"
"                  [Advanced: You can actually make the fit order a two-digit\n"
"                   code.  E.g. -dw 24 will use 4th-order on each row of text\n"
"                   but only 2nd-order for columns of displacement (see\n"
"                   leptonica dewarpFindVertDisparity() in dewarp2.c)]\n"
"                  Note: de-warping, like auto-straighten and auto-crop, is\n"
"                  intended for entire pages. It does not work on cropped areas.\n"
"                  If you want it to work on cropped areas, you should run\n"
"                  k2pdfopt in two passes--first to create selected crop\n"
"                  areas (e.g. -mode crop), then to apply dewarping.\n"
#endif
/* "-debug [<n>]      Set debug mode to <n> (def = 1).\n" */
"-ehl <n>          Same as -evl, except erases horizontal lines instead of\n"
"                  vertical lines.  See -evl.  Default is -ehl 0.\n"
"-er <n>           Use erosion filter on source bitmaps.  Makes the text look\n"
"                  darker.  A larger value of <n> makes the text thicker/darker.\n"
"                  Try -er 1 or -er 2.  Default is 0 (no erosion filtering).\n"
"                  Use a negative value for <n> to do the erosion before the\n"
"                  constrast adjustment is applied.  Use a positive value to\n"
"                  to the erosion after the constrast adjustment is applied.\n"
"                  This option may magnify scanning defects, so you might want\n"
"                  to combine with the -de (defect removal) option.\n"
"                  Has no effect in native mode output. See also -de, -g, -cmax.\n"
"-evl <n>          Detects and erases vertical lines in the source document\n"
"                  which may be keeping k2pdfopt from correctly separating\n"
"                  columns or wrapping text, e.g. column dividers.  If <n> is\n"
"                  zero, this is turned off (the default).  If <n> is 1, only\n"
"                  free-standing vertical lines are removed.  If <n> is 2,\n"
"                  vertical lines are erased even if they are the sides of\n"
"                  an enclosed rectangle or figure, for example.\n"
"-f2p <val>        Fit-to-page option.  The quantity <val> controls fitting\n"
"                  tall or small contiguous objects (like figures or\n"
"                  photographs) to the device screen.  Normally these are fit\n"
"                  to the width of the device, but if they are too small or\n"
"                  too tall, then if <val>=10, for example, they are allowed\n"
"                  to be 10%% wider (if too small) or narrower (if too tall)\n"
"                  than the screen in order to fit better.  Use -1 to fit the\n"
"                  object no matter what.  Use -2 as a special case--all\n"
"                  \"red-boxed\" regions (see -sm option) are placed one per\n"
"                  page.\n"
"                  Use -f2p -3 to fit as many \"red-boxed\" regions as\n"
"                  possible on each page without breaking them across pages.\n"
"                  (see -mode concat).\n"
"                  Default is -f2p 0.  See also -jf, -fr.\n"
"                  Note:  -f2p -2 will automatically also set -vb -2 to\n"
"                  exactly preserve the spacing in the red-boxed region.  If\n"
"                  you want to compress the vertical spacing in the red-boxed\n"
"                  region, use -f2p -2 -vb -1.\n"
"-fc[-]            For multiple column documents, fit [don't fit] columns to\n"
"                  the width of the reader screen regardless of -odpi.\n"
"                  Default is to fit the columns to the reader.\n"
"-fr[-]            Figure rotate--rotates wide-aspect-ratio figures to landscape\n"
"                  so that they best fit on the reader page.  Default is not\n"
"                  to rotate.  See also -f2p.\n"
"-fs <points>[+]   The output document is scaled so that the median font size in\n"
"                  the converted file is <points> points.  If the <points> value\n"
"                  is followed by a '+', the scaling is adjusted for every\n"
"                  source page, otherwise the font size is only adjusted once,\n"
"                  based on the median font size for the entire source document.\n"
"                  The default is -fs 0, which turns off scaling based on font\n"
"                  size.  The use of -fs overrides the -mag setting.\n"
"-g <gamma>        Set gamma value of output bitmaps. A value less than 1.0\n"
"                  makes the page darker and may make the font more readable.\n"
"                  Default is 0.5.  Has no effect with native-mode output.\n"
"                  See also -er, -cmax.\n"
"-grid <C>x<R>[x<O>][+]  Grid the source page into <C> columns by <R> rows with\n"
"                  with <O> percent overlap.  No regard will be made for trying\n"
"                  to break the page between columns or rows of text.  If a +\n"
"                  is specified, the destination page order will go across and\n"
"                  then down, otherwise it will go down and then across.  To\n"
"                  turn off gridding, specify a zero value for the columns or\n"
"                  for the rows.  Default is no gridding.  The default overlap\n"
"                  is 2%%.  Example:  -grid 2x2x5.  By default, gridding also\n"
"                  sets the following options, which can be overridden by\n"
"                  following the grid option with other command options:\n"
"                  -n -wrap- -f2p -2 -vb -2 -col 1.  For example, if you want\n"
"                  a column search done on each grid piece, you can put this:\n"
"                  -grid 2x2 -col 2.  See also -cbox.\n"
#ifdef HAVE_MUPDF_LIB
"-gs[-][-]         Force use of Ghostscript instead of MuPDF to read PDFs.\n"
"                  K2pdfopt has built-in PDF translation (via the MuPDF\n"
"                  library) but will try to use Ghostscript if Ghostscript\n"
"                  is available and the internal (MuPDF) translation fails\n"
"                  (virtually never happens).  You can force Ghostscript to\n"
"                  be used with this -gs option.  Use -gs- to use Ghostscript\n"
"                  only if MuPDF fails.  Use -gs-- to never use Ghostscript.\n"
"                  Download ghostscript at http://www.ghostscript.com.\n"
#endif
"-gtc <inches>     Threshold value for detecting column gaps (expert mode).\n"
"                  Sets how many of the pixels in the column shaft can be\n"
"                  non-white (total height of a line crossing the shaft in\n"
"                  inches).  See also -gtr.  Default = .005.\n"
/*
"-gtm <inches>     Threshold for trimming excess margins (xpert mode).\n"
"                  See -gtr.  Default = .005.\n"
*/
"-gtr <inches>     Threshold for detecting gaps between rows (expert mode).\n"
"                  This sets the maximum total black pixels, in inches, on\n"
"                  average, that can be in each row of pixels before the gap is\n"
"                  no longer considered a gap.  A higher value makes it easier\n"
"                  to detect gaps between rows of text.  Too high of a value\n"
"                  may inadvertently split figures and other graphics.\n"
"                  Default = 0.006.  See also -rsf.\n"
"-gtw <inches>     Threshold for detecting word gaps (expert mode).\n"
"                  See -gtr.  Default = .0015.\n"
#ifdef HAVE_K2GUI
"-gui[-]           Use [don't use] graphical user interface (MS Windows only).\n"
"                  If k2pdfopt is started from a console (command-line), the\n"
"                  default is not to launch the gui unless there are no command-\n"
"                  line options given.  If k2pdfopt is launched via its icon,\n"
"                  then the default is to launch the GUI.\n"
"-guimin[-]        Start the k2pdfopt GUI minimized.  Def = not minimized.\n"
#endif
"-h <height>[in|cm|s|t|p|x] Set height of output device in pixels, inches, cm,\n"
"                  source page size (s), trimmed source region size (t),\n"
"                  pixels (p), or relative to the OCR text layer (x).\n"
"                  The default units are pixels (p), and the default value\n"
"                  is 735 (the height of the Kindle 2 screen in pixels).\n"
"                  Examples:\n"
"                      -h 6.5in   Sets the device height to 6.5 in\n"
"                                 (using the output dpi to convert to\n"
"                                  pixels--see -dpi).\n"
"                      -h 1.5s    Sets the device height to 1.5 times the\n"
"                                 source page height (same as -h -1.5).\n"
"                      -h 1t      Sets the device height to whatever the\n"
"                                 trimmed page height is (you can follow\n"
"                                 -mode copy with -h 1t to make the output\n"
"                                 page height equal to the crop box height.\n"
"                      -h 0.5x    Sets the device height to half of the\n"
"                                 height of the box exactly surrounding\n"
"                                 the OCR text layer on the source page.\n"
"                  See also -w, -dpi, -dr.\n"
/*
"-hq               Higher quality (convert source to higher res bitmaps).\n"
"                  Equivalent to -idpi 400 -odpi 333 -w 1120 -h 1470.\n"
*/
"-hy[-]            Turn on [off] hyphen detection/elimination when wrapping\n"
"                  text.  Default is on.\n"
#ifdef HAVE_MUPDF_LIB
"-i                Echo information about the source file (PDF only).\n"
"                  Disables all other processing.\n"
#endif
"-ibox[<pagelist>|-|u] <cropbox>  Same as -cbox (see -cbox), except that these\n"
"                  boxes are ignored by k2pdfopt.  This is done by whiting out\n"
"                  the boxes in the source bitmap.  For native output, the\n"
"                  area in the -ibox will not affect the parsing of the source\n"
"                  file, but it may still be visible in the output file.\n"
"                  Default is no iboxes (-ibox-).  See also -cbox.\n"
"-idpi <dpi>       Set pixels per inch for input file.  Use a negative value\n"
"                  as a multiplier on the output dpi (e.g. -2 will set the\n"
"                  input file dpi to twice the output file dpi (see -odpi).\n"
"                  Default is -2.0.\n"
"-j -1|0|1|2[+/-]  Set output text justification.  0 = left, 1 = center,\n"
"                  2 = right.  Add a + to attempt full justification or a -\n"
"                  to explicitly turn it off.  The default is -1, which tells\n"
"                  k2pdfopt to try and maintain the justification of the\n"
"                  document as it is.  See also -wrap.\n"
"-jf 0|1|2 [<inches>]  Set figure (tall region) justification.  If a figure\n"
"                  has left or right margins available, this option allows\n"
"                  you to set the justification differently than the text.\n"
"                  E.g. you can center figures with -jf 1.  If you want to\n"
"                  specify a minimum height for figures (e.g. minimum region\n"
"                  height where this justification applies), you can tack it\n"
"                  on at the end, e.g. -jf 1 1.5 to center any region taller\n"
"                  than 1.5 inches.  Default is 0.75 inches for the minimum\n"
"                  height and to use the same justification on figures as\n"
"                  the rest of the document (-jf -1).  See also -f2p to fit\n"
"                  small or tall figures to the page.\n"
"-jfc[-|+]         Attempt [do not attempt] to keep figure captions joined\n"
"                  with their figures.  If you specify -jfc+, k2pdfopt will\n"
"                  also try to detect figure captions in multi-column documents.\n"
"                  This is not done by default because k2pdfopt will sometimes\n"
"                  (more often than not, in my experience) incorrectly choose\n"
"                  the multi-column layout if it is also trying to detect what\n"
"                  is a figure caption.  See also -cg, -cgmax, -cgr, -crgh.\n"
"                  Default = -jfc.\n"
"-jpg [<quality>]  Use JPEG compression in PDF file with quality level\n"
"                  <quality> (def=90).  A lower quality value will make your\n"
"                  file smaller.  See also -png. Use of -jpg is incompatible\n"
"                  with the -bpc option.\n"
#ifdef HAVE_TESSERACT_LIB
"-l <lang>         See -ocrlang.\n"
"-lang <lang>      See -ocrlang.\n"
#endif
"-ls[-][pagelist]  Set output to be in landscape [portrait] mode.  The default\n"
"                  is -ls- (portrait).  If an optional pagelist is specified,\n"
"                  only those pages are affected--any other pages are done\n"
"                  oppositely.  E.g. -ls1,3,5-10 would make source pages 1, 3\n"
"                  and 5 through 10 landscape.\n"
"-m[l|t|r|b] <val>[<units>][,<val>[units][,...]]  Set global crop margins for\n"
"                  every page.  If more than one value is given (comma-delimited\n"
"                  with no spaces in between), the order is left, top, right,\n"
"                  bottom, e.g. -m <left>,<top>,<right>,<bottom>.  You can also\n"
"                  use the more powerful -cbox option to do this same thing.\n"
"                  The default units are inches.  For available units and their\n"
"                  descriptions, see -h.\n"
"                  Examples:\n"
"                      -m 0.5cm\n"
"                         Sets all margins to 0.5 cm.\n"
"                      -m 0.5cm,1.0cm\n"
"                         Sets the left margin to 0.5 cm and all the other\n"
"                         margins to 1.0 cm.\n"
"                      -m 0.2in,0.5in,0.2in,0.5in\n"
"                         Sets the left and right crop margins to\n"
"                         0.2 inches and the top and bottom to 0.5 inches.\n"
"                      -mt 1cm\n"
"                         Sets the top margin to 0.5 cm.\n"
"                      -m -0.1x,-0.1x,1.1x,1.1x\n"
"                         With the 'x' unit, the behavior is a little\n"
"                         different.  Rather than specifying the widths\n"
"                         of each margin, you specify the position of\n"
"                         the crop box relative to the OCR text layer\n"
"                         in the source file, where 0x,0x,1x,1x would\n"
"                         exactly bound the OCR text layer.\n"
"                  The default crop margins are 0 inches.\n"
"                  [NOTE: The default was 0.25 inches for all margins before\n"
"                         v1.65.]\n"
"                  See also -cbox and -ac to autocrop scanning artifacts.\n"
"-mag <value>      Magnify the converted document (text) size by <value>.\n"
"                  Default is -mag 1 (no magnification). See also -fs.\n"
"-mc[-]            Mark [don't mark] corners of the output bitmaps with a\n"
"                  small dot to prevent the reading device from re-scaling.\n"
"                  Default = mark.\n"
"-mode <mode>      Shortcut for setting multiple options at once which\n"
"                  determine the basic way in which k2pdfopt will behave.\n"
"                  Available modes are:\n"
"                      copy   Same as -n- -wrap- -col 1 -vb -2 -w 1s -h 1s\n"
"                             -dpi 150 -rt 0 -c -t- -f2p -2 -m 0 -om 0 -pl 0\n"
"                             -pr 0 -pt 0 -pb 0 -mc-.  Makes k2pdfopt\n"
"                             behave exactly like my pdfr program--source\n"
"                             pages are simply copied to the output file, but\n"
"                             rendered as bitmaps.  No trimming or re-sizing\n"
"                             is done.  Can also use -mode pdfr.\n"
"                             Note 1:  Use -mode copy -n if you want an exact\n"
"                                      copy (output in native mode).\n"
"                             Note 2:  The default gamma and contrast settings\n"
"                                      are not reset by -mode copy.  If you\n"
"                                      want a perfect copy, do this:\n"
"                                      -mode copy -gamma 1 -cmax 1\n"
"                      fp     Also can use fitpage.  Same as -n -wrap- -col 1\n"
"                             -vb -2 -f2p -2 -t.\n"
"                      fw     Same as -n -wrap- -col 1 -vb -2 -t -ls.  Makes\n"
"                             k2pdfopt behave like sopdf's \"fit width\"\n"
"                             option.  Can also use -mode sopdf.\n"
"                      2col   Same as -n -wrap- -col 2 -vb -2 -t.\n"
"                             Optimizes for a 2-column scientific article with\n"
"                             native PDF output.\n"
"                      tm     Trim margins--same as -mode copy, but sets the\n"
"                             output to be trimmed to the margins and the width\n"
"                             and height of the output to match the trimmed\n"
"                             source pages.  Also uses native mode.  Equivalent\n"
"                             to -n -wrap- -col 1 -vb -2 -f2p -2 -t -w 1t -h 1t\n"
"                             -rt 0 -c -m 0 -om 0 -pl 0 -pr 0 -pt 0 -pb 0 -mc-.\n"
"                             Can also use -mode trim.\n"
"                      crop   Used with -cbox option, puts each cropped area\n"
"                             on a separate page, untrimmed, and sizes the\n"
"                             page to the cropped region.  Same as -wrap-\n"
"                             -col 1 -vb -2 -w 1t -h 1t -t- -rt 0 -c -f2p -2\n"
"                             -m 0 -om 0 -pad 0 -mc- -n\n"
"                      concat Keeping the output pages the same size as the\n"
"                             source pages, fit as many crop-boxed regions on\n"
"                             the output pages as possible without breaking\n"
"                             them across pages.  Equivalent to: -n -wrap-\n"
"                             -col 1 -vb -2 -t- -f2p -3 -fc- -w 1s -h 1s -ocr-\n"
"                      def    Default k2pdfopt mode: -wrap -n- -col 2 -vb 1.75\n"
"                             -dev k2 -rt auto -c- -t -f2p 0 -m 0 -om 0.02\n"
"                             -ls-.\n"
"                  You can modify modes by overriding their options after\n"
"                  specifying the mode, e.g. -mode fw -vb -1.\n"
#ifdef HAVE_MUPDF_LIB
"-n[-]             Use \"native\" PDF output format.  NOTE: if you want native\n"
"                  PDF output, it's probably best to use a -mode option like\n"
"                  -mode fitwidth or -mode 2col, both of which automatically\n"
"                  turn on native PDF output and optimize other settings for it.\n"
"                  Native PDF output preserves the native source PDF contents,\n"
"                  i.e. the output PDF file is not rendered as a sequence of\n"
"                  bitmapped pages like in the default k2pdfopt output mode.\n"
"                  Instead, the source PDF's native content is used along with\n"
"                  additional PDF instructions to translate, scale, and crop\n"
"                  the source content.  With native PDF output, if the source\n"
"                  file has selectable text, the text remains selectable in\n"
"                  the output file.  The output file can also be zoomed\n"
"                  without loss of fidelity.  This may also result in a\n"
"                  smaller output file (but not always).  By default, native\n"
"                  PDF output format is turned off.  See also -mode.\n"
"                  NOTES:\n"
"                  1. Native PDF output cannot be used with text wrapping\n"
"                     on (see -wrap option).  Turning it on will disable\n"
"                     text wrapping.\n"
"                  2. Native PDF output is not recommended for source\n"
"                     files which are scanned (there is no benefit unless\n"
"                     the scanned document includes a layer of OCR text).\n"
"                  3. Native PDF output is incompatible with OCR (see -ocr),\n"
"                     though OCR is typically not necessary if the native PDF\n"
"                     contents are kept.  Turning on native PDF output will\n"
"                     disable OCR.\n"
"                  4. Native PDF output can only be used with PDF source\n"
"                     files (it does not work with DJVU source files).\n"
"                  5. Contrast adjust, gamma correction, and sharpening\n"
"                     are disabled with native PDF output.\n"
"                  6. It is recommended that you use -vb -2 with native PDF\n"
"                     output, particularly if you are having difficulty\n"
"                     selecting/searching text in the output PDF file.\n"
"                  7. This option works well with -mode fw, -mode 2col, or\n"
"                     with the -grid option.  It is used by default in those\n"
"                     cases.\n"
#endif
"-neg[-|+]         Inverse [don't inverse] the output images (white letters\n"
"                  on black background, or \"night mode\").  If -neg+, inverts\n"
"                  all graphics no matter what.  If just -neg, attempts to\n"
"                  invert text only and not figures.  Default = -neg-.\n"
"                  See also -colorbg and -colorfg.\n"
"-ng <gap>         Set gap between notes and main text in the output document.\n"
"                  The <gap> defaults to inches but can have other units (see\n"
"                  -h, for example).  See -nl and -nr for how to turn on notes\n"
"                  processing.  Default is -ng 0.2.\n"
"-nl[<pages>] [<leftbound>,<rightbound>]\n"
"-nr[<pages>] [<leftbound>,<rightbound>]\n"
"                  The source document has notes in the left (-nl) or right\n"
"                  (-nr) margins.  Specific pages can be specified for the\n"
"                  notes using <pages> (same format as -cbox or -p).  If\n"
"                  <leftbound>,<rightbound> are specified, they specify the\n"
"                  fraction of the page width where to look for the break\n"
"                  between the notes and the main page.  E.g.\n"
"                  -nl 0.15,0.25 will look for the boundary between the notes\n"
"                  and the text between 15%% and 25%% of the way across the\n"
"                  source page.  Use -nl- to turn off all processing of notes\n"
"                  in the margins (default).  Default values for <leftbound>\n"
"                  and <rightbound> are 0.05 to 0.35 for -nl and 0.65 to 0.95\n"
"                  for -nr.\n"
"                  Notes in the margins are treated differently than other\n"
"                  \"columns\" of text.   They will be interspersed with the\n"
"                  text in the adjacent column of main text.\n"
"                  Note that -nr... or -nl... will also set -cg to 0.05.\n"
#ifdef HAVE_OCR_LIB
"-nt <nthreads>    Use <nthreads> parallel threads when OCR-ing a document\n"
"                  with the Tesseract OCR engine (GOCR is not thread safe).\n"
"                  This may provide a significant processing speed improvement\n"
"                  when using Tesseract OCR.  Note that a higher number is not\n"
"                  always faster.  You should experiment with your system to\n"
"                  find the optimum.  A negative value is interpreted as a\n"
"                  percentage of available CPUs.  The default is -50, which\n"
"                  tells k2pdfopt to use half of the available CPU threads.\n"
"                  Some performances I measured:\n"
"                  ----------------------------------------------------------\n"
"                                                               OCR Speed\n"
"                     O/S           CPU         Nthreads       improvement\n"
"                  ----------------------------------------------------------\n"
"                  Win 10 x64     Core i5      2 (default)        1.5x\n"
"                  Win 10 x64     Core i5          3              1.6x\n"
"                  Win 10 x64     Core i5          4              1.8x\n"
"                  ----------------------------------------------------------\n"
"                  Win 10 x64     Core i7          2              1.8x\n"
"                  Win 10 x64     Core i7          3              2.4x\n"
"                  Win 10 x64     Core i7      4 (default)        2.5x\n"
"                  Win 10 x64     Core i7          5              2.8x\n"
"                  Win 10 x64     Core i7          6              2.7x\n"
"                  Win 10 x64     Core i7          7              2.7x\n"
"                  Win 10 x64     Core i7          8              2.6x\n"
"                  ----------------------------------------------------------\n"
"                  Linux x64      Core i5      2 (default)        1.9x\n"
"                  Linux x64      Core i5          3              2.6x\n"
"                  Linux x64      Core i5          4              2.7x\n"
"                  ----------------------------------------------------------\n"
"                  Linux x64   Xeon E52690v2       2              1.9x\n"
"                  Linux x64   Xeon E52690v2       4              3.5x\n"
"                  Linux x64   Xeon E52690v2       6              5.1x\n"
"                  Linux x64   Xeon E52690v2       8              6.6x\n"
"                  Linux x64   Xeon E52690v2   10 (default)       8.7x\n"
"                  Linux x64   Xeon E52690v2      14              9.5x\n"
"                  Linux x64   Xeon E52690v2      20             10.2x\n"
"                  ----------------------------------------------------------\n"
"                  Interestingly, Linux seems to have much better multithreading\n"
"                  performance than Windows.  I suspect the OS/X results are\n"
"                  similar to the Linux results.\n"
"                  NOTE:  -nt has no effect if you select -ocrd c or -ocrd p.\n"
"                         See -ocrd.\n"
#endif
"-o <namefmt>      Set the output file name using <namefmt>.  %s will be\n"
"                  replaced with the full name of the source file minus the\n"
"                  extension.  %b will be replaced by the base name of the\n"
"                  source file minus the extension.  %f will be replaced with\n"
"                  the folder name of the source file.  %d will be replaced with\n"
"                  the source file count (starting with 1).  The .pdf extension\n"
"                  will be appended if you don't specify an extension.\n"
"                  E.g. -o out%04d.pdf will result in output files out0001.pdf,\n"
"                  out0002.pdf, ... for the converted files.  Def = %s_k2opt\n"
"                  -------------------------------------------------------------\n"
"                  BITMAP OUTPUT:  For output to bitmaps, you can put -o .png\n"
"                  or -o .jpg (see -jpeg for quality setting).\n"
"                  -------------------------------------------------------------\n"
"                  MORE DETAIL:  If <namefmt> ends in .jpg or .png, the output\n"
"                  will be in the JPEG or PNG bitmap format, respectively, one\n"
"                  bitmap per page.  If your <namefmt> has no %d in it, then\n"
"                  %04d will be appended.  If <namefmt> has only one %d, it will\n"
"                  get substituted with the page number.  If it has two %d's,\n"
"                  the first will get the file count and the second will get the\n"
"                  page number.  Example: if the source PDF is myfile.pdf, then\n"
"                  -o %s%03d.png would create myfile001.png, myfile002.png,\n"
"                  etc., for each page of the PDF.\n"
#ifdef HAVE_OCR_LIB
"-ocr[-] [g|t|m]   Attempt [don't attempt] to use optical character\n"
"                  recognition (OCR) in order to embed searchable text into\n"
"                  the output PDF document.  If followed by t or g, specifies\n"
"                  the ocr engine to use (tesseract or gocr).  If followed by\n"
"                  m, and if the PDF document has text in it, then the MuPDF\n"
"                  engine is used to extract the text (sort of a virtual OCR).\n"
"                  If -ocr is specified with no argument, tesseract is used.\n"
"                  If tesseract fails (e.g. no language files found), GOCR\n"
"                  is used.  The overall default operation of k2pdfopt is\n"
"                  -ocr m.  See also -ocrvis and -ocrhmax.\n"
"                  NOTE:  Turning on OCR will disable native PDF output.\n"
"                  DISCLAIMER:  The main intent of OCR isn't to improve the\n"
"                      visual quality of the text at all--at least not the way\n"
"                      k2pdfopt does it.  OCR is most useful on scanned PDFs\n"
"                      that don't have selectable text to begin with, but using\n"
"                      OCR with k2pdfopt on such documents doesn't change the\n"
"                      look of the output PDF file at all.  The OCR text is\n"
"                      simply placed invisibly over the scanned text so that\n"
"                      you appear to be able to select the scanned text (when,\n"
"                      in fact, you are selecting the invisibly placed OCR\n"
"                      text).  So the only time you will even notice the OCR\n"
"                      errors is if you try to search for a word and can't find\n"
"                      that word because the OCR of that word is incorrect, or\n"
"                      if you copy a selection of the OCR text and paste it\n"
"                      into something else so that you can actually see it.\n"
"-ocrcol <n>       If you are simply processing a PDF to OCR it (e.g. if you\n"
"                  are using the -mode copy option) and the source document has\n"
"                  multiple columns of text, set this value to the number of\n"
"                  columns to process (up to 4).  Default is to use the same\n"
"                  value as -col.\n"
#ifdef HAVE_TESSERACT_LIB
"-ocrd w|l|c|p     Set OCR detection type for k2pdfopt and Tesseract.  <type>\n"
"                  can be word (w), line (l), columns (c), or page (p).  Default\n"
"                  is line.\n"
"                  For -ocrd w, k2pdfopt locates each word in the scanned\n"
"                  document and passes individual words to Tesseract for\n"
"                  OCR conversion.  This was the only type of detection before\n"
"                  v2.42 but is not an optimal OCR conversion method when\n"
"                  using Tesseract.\n"
"                  For -ocrd l, k2pdfopt passes each line of the converted\n"
"                  file to Tesseract for conversion.  This typically gives\n"
"                  better results than -ocrd w since Tesseract can better\n"
"                  determine the text baseline position with a full line.\n"
"                  For -ocrd c, k2pdfopt detects each column of the converted\n"
"                  file and passes that to Tesseract for conversion.\n"
"                  For -ocrd p, k2pdfopt passes the entire output page of text\n"
"                  to Tesseract and lets Tesseract parse it for word positions.\n"
"                  Tesseract has done considerable code development for\n"
"                  detecting words on pages (more than k2pdfopt), so this\n"
"                  should also be a reliable way to create the OCR layer.\n"
"                  One drawback to -ocrd c or -ocr p is that there is no benefit\n"
"                  to using the OCR multithreading option (see -nt).\n"
#endif
"-ocrhmax <in>     Set max height for an OCR'd word in inches.  Any graphic\n"
"                  exceeding this height will not be processed with the OCR\n"
"                  engine.  Default = 1.5.  See -ocr.\n"
#ifdef HAVE_TESSERACT_LIB
"-ocrlang <lang>|? Select the Tesseract OCR Engine language.  This is the\n"
"                  root name of the training data, e.g. -lang eng for English,\n"
"                  -ocrlang fra for French, -ocrlang chi_sim for simplified\n"
"                  Chinese.  You can also use -l.  The default language is\n"
"                  whatever is in your Tesseract trained data folder.  If you\n"
"                  have more than one .traineddata file in that folder, the\n"
"                  one with the most recent time stamp is used.\n"
"                  NOTE 1: Use -ocrlang ? to see the list of Tesseract language\n"
"                  files in your Tesseract data folder.\n"
"                  NOTE 2: Using the -ocrvis t option will not show the OCR text\n"
"                  correctly for any character above unicode value 255 since\n"
"                  k2pdfopt does not use any embedded fonts, but the text\n"
"                  will convert to the correct Unicode values when copy /\n"
"                  pasted.\n"
"                  NOTE 3: Tesseract allows the specification of multiple\n"
"                  language training files, e.g. -ocrlang eng+fra would\n"
"                  specify English as the primary and French as the secondary\n"
"                  OCR language.  In practice I have not found this to work\n"
"                  very well.  Try multiple languages in different orders.\n"
#endif
"-ocrdpi <dpi>     Set the desired dpi of the bitmaps passed to the OCR engine\n"
"                  OR set the desired height of a lower case letter (e.g. 'e')\n"
"                  in pixels.  If <dpi> is positive, it is interpreted as dpi.\n"
"                  If <dpi> is negative, the absolute value is interpreted as\n"
"                  a lowercase letter height in pixels.  Any bitmapped text sent\n"
"                  to the OCR engine will be downsampled (if too large) so that\n"
"                  the appropriate dpi or lowercase letter size is achieved.\n"
"                  The default is 300 because I've found this works best\n"
"                  empirically for Tesseract v4.0.0 English OCR with font sizes\n"
"                  in the range 8 - 15 pts.  Use a lower value if the font size\n"
"                  in your document is larger than 15 - 20 pts.  Or use\n"
"                  -ocrdpi -24 if you have a wide range of font sizes.\n"
"                  Use -ocrdpi 0 to disable any downsampling.\n"
"-ocrout[-] <namefmt>  Write [don't write] UTF-8 OCR text output to file\n"
"                  <namefmt>.  See the -o option for more about how\n"
"                  <namefmt> works.  Default extension is .txt.  Default is\n"
"                  no output.\n"
"-ocrsort[-]       When a PDF document has its own OCR/Text layer, this option\n"
"                  orders the OCR text layer by its position on the page.  This\n"
"                  should not be necessary unless the OCR layer was very poorly\n"
"                  generated.  Default is -ocrsort- (off).\n"
"-ocrsp[+|-]       When generating the OCR layer, do an entire row of text at\n"
"                  once, with spaces between each words.  By default (-ocrsp-),\n"
"                  each word is placed separately in the PDF document's OCR\n"
"                  layer.  This causes problems with text selection in some\n"
"                  readers (for example, individual words cannot be selected).\n"
"                  Using -ocrsp- may fix behavior like this, but will result in\n"
"                  less accurate word placement since k2pdfopt does not try to\n"
"                  exactly match the font used by the document.  Use -ocrsp+\n"
"                  to allow more than one space between each word in the row\n"
"                  of text in order to optimize the selection position.\n"
"-ocrvis <s|t|b>   Set OCR visibility flags.  Put 's' to show the source doc,\n"
"                  't' to show the OCR text, and/or 'b' to put a box around\n"
"                  each word.  Default is -ocrvis s.  To show both the source\n"
"                  document and the OCR text overlayed on top:  -ocrvis st.\n"
"                  See also -ocr."
#ifdef HAVE_TESSERACT_LIB
                   "  See also -ocrlang (the note about -ocrvis t)."
#endif
"\n"
#endif
"-odpi <dpi>       Set pixels per inch of output screen (def=167). See also\n"
"                  -dr, -w, -h, -fc.  You can also use -dpi for this.\n"
"                  See also -fs, -mag.\n"
"-om[b|l|r|t] <val>[<units>][,<val>[units][,...]]  Set the blank area margins\n"
"                  on the output device.  Works very much like the -m option.\n"
"                  See -m for more about the syntax.  Default = 0.02 inches.\n"
"                  Note that the 's', 't', and 'x' units for -om all behave\n"
"                  the same and scale to the device size.  E.g. -om 0.1s will\n"
"                  make the device screen margins 0.1 times the device width\n"
"                  (for the left and right margins) or height (for the top and\n"
"                  bottom margins) of the output device screen.\n"
"-ow[-] [<mb>]     Set the minimum file size (in MB) where overwriting the\n"
"                  file will not be done without prompting.  Set to -1 (or\n"
"                  just -ow with no value) to overwrite all files with no\n"
"                  prompting.  Set to 0 (or just -ow-) to prompt for any\n"
"                  overwritten file.  Def = -ow 10 (any existing file\n"
"                  over 10 MB will not be overwritten without prompting).\n"
"                  See also -y option.\n"
"-p <pagelist>     Specify pages to convert.  <pagelist> must not have any\n"
"                  spaces.  E.g. -p 1-3,5,9,10- would do pages 1 through 3,\n"
"                  page 5, page 9, and pages 10 through the end.  The letters\n"
"                  'e' and 'o' can be used to denote even and odd pages, e.g.\n"
"                      -p o,e        Process all odd pages, then all even ones.\n"
"                      -p 2-52e,3-33o    Process 2,4,6,...,52,3,5,7,...,33.\n"
"                  Overridden by -px option.  See -px.\n"
"-pad <padlist>    A shortcut for -pl, -pt, -pr, -pb.  E.g. -pad 15,10,13,20\n"
"                  is the same as -pl 15 -pt 10 -pr 13 -pb 20.  Also, using\n"
"                  -pad 15 will set all pads to 15, for example.\n"
"-p[b|l|r|t] <nn>  Pad [bottom|left|right|top] side of destination bitmap with\n"
"                  <nn> rows.  Defaults = 4 (bottom), 0 (left), 3 (right), and\n"
"                  0 (top).  Example:  -pb 10.  This is typically only used on\n"
"                  certain devices to get the page to come out just right.  For\n"
"                  setting margins on the output device, use -om. See also -pad.\n"
/*
"-pi[-]            Preserve [don't preserve] indentation when wrapping text,\n"
"                  e.g. if the first line of each paragraph is indented, keep\n"
"                  it that way.  The default is to ignore indentation.  Also,\n"
"                  this is only used with left justification turned on (-j 0).\n"
*/
"-png              (Default) Use PNG compression in PDF file.  See also -jpeg.\n"
#ifdef HAVE_GHOSTSCRIPT
"-ppgs[-]          Post process [do not post process] with ghostscript.  This\n"
"                  will take the final PDF output and process it using\n"
"                  ghostscript's pdfwrite device (assuming ghostscript is\n"
"                  available).  A benefit to doing this is that all \"invisible\"\n"
"                  and/or overlapping text regions (outside cropping areas) get\n"
"                  completely removed, so that text selection capability is\n"
"                  improved.  The actual ghostscript command used is:\n"
"                  gs -dSAFER -dBATCH -q -dNOPAUSE -sDEVICE=pdfwrite\n"
"                     -dPDFSETTINGS=/prepress -sOutputFile=<outfile>\n"
"                     <srcfile>\n"
"                  The default is not to post process with ghostscript.\n"
#endif
"-px <pagelist>    Exclude pages from <pagelist>.  Overrides -p option.  Default\n"
"                  is no excluded pages (-px -1).\n"
"-r[-]             Right-to-left [left-to-right] page scans.  Default is\n"
"                  left to right.\n"
#ifdef HAVE_K2GUI
"-rls[+|-]         Restore [+] or don't restore [-] the last command-line\n"
"                  settings from the environment variable K2PDFOPT_CUSTOM0.\n"
"                  The default (-rls) is to restore the settings if there are no\n"
"                  other command-line options specified when running (from.\n"
"                  either the command line or the K2PDFOPT env var.), unless\n"
"                  those options are \"-gui\" or specify a file name.\n"
#endif
"-rsf <val>        Row Split Figure of merit (expert mode).  After k2pdfopt has\n"
"                  looked for gaps between rows of text, it will check to see\n"
"                  if there appear to be missed gaps (e.g. if one row is twice\n"
"                  the height of all the others).  Increasing this value makes\n"
"                  it harder for k2pdfopt to split a row.  Lowering it makes it\n"
"                  easier.  Default value = 20.\n"
"-rt <deg>|auto[+]|aep  Rotate source page counterclockwise by <deg> degrees.\n"
"                  NOTE: If you're trying to get \"landscape\" output so that\n"
"                  you can turn your reader on its side, use -ls instead of\n"
"                  -rt.  The -rt option is intended to be used for when your\n"
"                  source PDF is incorrectly rotated--e.g. if you view it on\n"
"                  a standard PC reader and it comes up sideways.\n"
"                  <deg> can be 90, 180, 270.  Or use \"-rt auto\" to examine up\n"
"                  to 10 pages of each file to determine the orientation used\n"
"                  on the entire file (this is the default).  Or use \"-rt aep\"\n"
"                  to auto-detect the rotation of every page.  If you have\n"
"                  different pages that are rotated differently from each other\n"
"                  within one file, you can use this option to try to auto-\n"
"                  rotate each source page.  Use -rt auto+ to turn on auto-\n"
"                  detect even in preview mode (otherwise it is off).\n"
"                  See also -ls.\n"
/*
"-rwmin <min>      Set min row width before the row can be considered for\n"
"                  glueing to other rows (inches).\n"
*/
"-s[-]             Sharpen [don't sharpen] images.  Default is to sharpen.\n"
"-sm[-]            Show [don't show] marked source.  This is a debugging tool\n"
"                  where k2pdfopt will mark the source file with the regions it\n"
"                  finds on them and the order in which it processes them and\n"
"                  save it as <srcfile>_marked.pdf.  Default is not to show\n"
"                  marked source.  Red regions are found on the first pass\n"
"                  (use -f2p -2 to put each red region on a separate page).\n"
"                  Green lines mark vertical regions affected by -vb and -vs.\n"
"                  Gray lines mark individual rows of text (top, bottom, and\n"
"                  baseline).  Blue boxes show individual words (passed to OCR\n"
"                  if -ocr is specified).\n"
"-sp[-]            For each file on the command-line, just echo the number\n"
"                  of pages--don't process.  Default = off (-sp-).\n"
"-t[-]             Trim [don't trim] the white space from around the edges of\n"
"                  any output region.  Default is to trim.  Using -t- is not\n"
"                  recommended unless you want to exactly duplicate the source\n"
"                  document.\n"
"-title <title>    Set the title metadata / property of the PDF output file(s).\n"
"                  Default is to use the title of the source document\n"
"                  (-title \"\").  The <title> string will be parsed for\n"
"                  special characters that allow you to substitute the file\n"
"                  name.  See the -o option for a description of these\n"
"                  substitutions.\n"
"-to[-]            Text only output.  Remove figures from output.  Figures are\n"
"                  determined empirically as any contiguous region taller than\n"
"                  0.75 inches (or you can specify this using the -jf option).\n"
"                  Use -to- to turn off (default).\n"
"-toc[-]           Include [don't include] table of contents / outline /\n"
"                  bookmark information in the PDF output if it is available\n"
"                  in the source file (works only for PDF source files and\n"
"                  only if MuPDF is compiled in).  By default, a new destination\n"
"                  page is started at each bookmark location.  Do disable this,\n"
"                  see the -bp option.  If -toc- is specified, bookmark\n"
"                  information from the source file is ignored.  See also\n"
"                  -toclist.  Default is -toc.\n"
"-toclist <pagelist>|<file>  Override the PDF source file's outline information\n"
"                  (bookmarks / table of contents) with either a list of source\n"
"                  pages or a file describing the table of contents. If you\n"
"                  specify a list of pages, e.g. -toclist 5,10,20,40,100\n"
"                  then those pages are marked as Chapter 1, 2, etc.,\n"
"                  respectively.  If you specify a file name, the file should be\n"
"                  a text file formatted like this example:\n"
"                      1 Introduction\n"
"                      10 Chapter 1\n"
"                      +10 Chapter 1, Part A\n"
"                      +25 Chapter 1, Part B\n"
"                      ++25 Chapter 1, Part B, Subsection 1\n"
"                      ++27 Chapter 1, Part B, Subsection 2\n"
"                      +30 Chapter 1, Part C\n"
"                      50 Chapter 2\n"
"                      70 Chapter 3\n"
"                  The '+' indicates a sub-level heading (multiple +'s for\n"
"                  multiple sub-levels).  The first number on the line is the\n"
"                  source page reference number.  The rest of the text on the\n"
"                  line is the name of the chapter / subheading.\n"
"                  Note:  This option overrides -toc.  To get a template from\n"
"                  an existing PDF file, see the -tocsave option.\n"
"-tocsave <file>   If an outline exists in the PDF file (and -toc is specified)\n"
"                  write that outline to text file <file> in the format required\n"
"                  by -toclist.  See -toc, -toclist.\n"
"-ui[-]            User input query turned on [off].  Default = on for linux or\n"
"                  if not run from command line in Windows.\n"
"-v                Verbose output.\n"
"-vb <thresh>      Set gap-size vertical-break threshold between regions that\n"
"                  cause them to be treated as separate regions.  E.g. -vb 2\n"
"                  will break the document into separate regions anywhere\n"
"                  there is a vertical gap that exceeds 2 times the median\n"
"                  gap between lines of text.  These separate regions may\n"
"                  then be scaled and aligned independently.\n"
"                  Special values:  Use -vb -1 to preserve all horizontal\n"
"                  alignment and scaling across entire regions (vertical\n"
"                  spacing may still be adjusted).  Use -vb -2 to exactly\n"
"                  preserve each region (both horizontal alignment and\n"
"                  vertical spacing--this is the value used by -mode fw, for\n"
"                  example).  The default is -vb 1.75.\n"
/*
"-vm <mult>        Vertical spacing multiplier.  Reduces gaps and line spacings\n"
"                  in the document using the multiplier <mult>.\n"
"                  E.g. for -vm 0.9, text lines would be spaced at 90% of their\n"
"                  original spacing.  This is applied before -vs.\n"
"                  Default value is 1.0.  See also -vs.\n"
*/
"-vls <spacing>    Set vertical line spacing as a fraction of the text size.\n"
"                  This can be used to override the line spacing in a document.\n"
"                  If 1, then single spacing is used.  2 = double spacing.\n"
"                  If negative, then the absolute value acts as the limiting\n"
"                  case.  E.g., if you set -vls -1.5, then any the line\n"
"                  spacing of the original document is preserved unless it\n"
"                  exceeds 1.5 (times single spacing).  Default = -1.2.\n"
"                  See also -vs.\n"
"-vs <maxgap>      Preserve up to <maxgap> inches of vertical spacing between\n"
"                  regions in the document (marked in green when using -sm\n"
"                  option).  This value has no effect if you use a negative\n"
"                  value for -vb.  The default value is 0.25.\n"
"                  See also -vls, -vb.\n"
"-w <width>[in|cm|s|t|p] Set width of output device.  Default is 560.  See -h.\n"
"-wrap[-|+]        Enable [disable] text wrapping.  Default = enabled.  If\n"
"                  -wrap+, regions of text with lines shorter than the mobile\n"
"                  device screen are re-flowed to fit the screen width.  If\n"
"                  you use -wrap+, you may want to also specify -fc- so that\n"
"                  narrow columns of text are not magnified to fit your device.\n"
"                  Text wrapping disables native PDF output (see -n option).\n"
"                  See also -ws, -j, -fc, -n.\n"
/*
"-whmax <height>   Max height allowed for wrapping a row (inches).\n"
*/
"-ws <spacing>     Set minimum word spacing for line breaking as a fraction of\n"
"                  the height of a lowercase 'o'.  Use a larger value to make it\n"
"                  harder to break lines.  If negative, automatic word spacing\n"
"                  is turned on.  The automatic spacing leans toward breaking\n"
"                  long words between letters to be sure to fit text to the\n"
"                  device display.  Def = -0.20.  The absolute value of the\n"
"                  setting, if negative, is used as a minimum allowed value.\n"
"                  If you want k2pdfopt to aggressively break lines (e.g. break\n"
"                  apart long words if they don't fit on a line), use a smaller\n"
"                  absolute value, e.g. -ws -0.01.  A positive value works as\n"
"                  it did in v2.18 and before.  The default value was changed\n"
"                  from 0.375 in v2.18 to -0.20 in v2.20.  See also -wrap.\n"
"-wt[+] <thresh>   Any pixels whiter than <thresh> (0-255) are treated\n"
"                  as \"white\".  Setting this lower can help k2pdfopt better\n"
"                  process some poorly-quality scanned pages or pages with\n"
"                  watermarks.  Note that the pixels which are above <thresh>\n"
"                  threshold value and therefore are treated as white are not\n"
"                  actually changed to pure white (255) unless the '+' is also\n"
"                  included.  Otherwise, this only sets a threshold.\n"
"                  The default value for -wt is -1, which tells k2pdfopt to pick\n"
"                  the optimum value.  See also -cmax, -colorfg, -colorbg.\n"
"-x[-]             Exit [don't exit--wait for <Enter>] after completion.\n"
"-y[-]             Assume [don't assume] \"yes\" to queries, such as whether\n"
"                  to overwrite a file.  See also -ow.  Also turns off any\n"
"                  warning messages.\n";


static int strlencrlf(char *s);
static void strcatcrlf(char *d,char *s);
static int prcmdopts(char *s,int nl,char *pattern,int prompt);
static int opts_match(char *pattern,char *usage);
static int cmdoplines(char *s);
static char *pr1cmdopt(char *s,int maxlines,int display);
static void prlines(char *s,int nlines);
static int wait_enter(void);


void k2usage_show_all(FILE *out)

    {
    fprintf(out,"%s%s"
                "Command Line Options\n"
                "--------------------\n"
                "%s\n",usageintro,usageenv,k2pdfopt_options);
    }


void k2usage_to_string(char *s)

    {
    s[0]='\0';
    strcatcrlf(s,usageintro);
    strcatcrlf(s,usageenv);
    strcatcrlf(s,"Command Line Options\n"
                 "--------------------\n");
    strcatcrlf(s,k2pdfopt_options);
    }


int k2usage_len(void)

    {
    return(strlencrlf(usageintro)+strlencrlf(usageenv)+strlencrlf(k2pdfopt_options)+128);
    }


static int strlencrlf(char *s)

    {
    int i,c;

    for (c=i=0;s[i]!='\0';i++)
        c += (s[i]=='\n') ? 2 : 1;
    return(c);
    }


static void strcatcrlf(char *d,char *s)

    {
    int i,j;

    for (j=strlen(d),i=0;s[i]!='\0';i++)
        {
        if (s[i]=='\n')
            d[j++]='\r';
        d[j++]=s[i];
        }
    d[j]='\0';
    }


int k2pdfopt_usage(char *pattern,int prompt)

    {
    int nl;

    if (!prompt)
        ansi_set(0);
    if (prompt)
        {
        nl=get_ttyrows();
        if (nl < 20)
            nl=20;
        }
    else
        nl=-1;
    if (!strcmp(pattern,"*"))
        {
        prlines(usageintro,nl-4);
        if (prompt && wait_enter()<0)
            return(0);
        prlines(usageenv,nl-1);
        if (prompt && wait_enter()<0)
            return(0);
        }
    if (!prcmdopts(k2pdfopt_options,nl,pattern,prompt))
        return(0);
    return(1);
    }


static int prcmdopts(char *s,int nl,char *pattern,int prompt)

    {
    int i,ll,c,all;
    char pat2[64];

    all=!strcmp(pattern,"*");
    sprintf(pat2,"*%s*",pattern);   
    for (i=0;1;i++)
        { 
        if (i==0)
            k2printf(TTEXT_BOLD "Command Line Options\n"
                               "--------------------\n" TTEXT_NORMAL);
        else if (prompt)
            k2printf(TTEXT_BOLD "Command Line Options (cont'd)\n"
                               "-----------------------------\n" TTEXT_NORMAL);
        ll=!i ? nl-3 : nl-2;
        c=0;
        while (1)
            {
            int nlo;
            nlo=cmdoplines(s);
            if (!all && !opts_match(pat2,s))
                {
                nlo=0;
                if (s[0]=='\0')
                    break;
                s=pr1cmdopt(s,-1,0);
                }
            else
                {
                if (ll-2-nlo<0 && c==0)
                    nlo=ll-2;
                c++;
                if (s[0]=='\0' || ll-2-nlo<0)
                    break;
                s=pr1cmdopt(s,ll-2,1);
                ll-=nlo;
                }
            }
        while (ll>1)
            {
            if (prompt)
                k2printf("\n");
            ll--;
            }
        if (!i && prompt)
            k2printf("\n");
        if (prompt && wait_enter()<0)
            return(0);
        if (s[0]=='\0')
            break;
        }
    return(1);
    }


static int opts_match(char *pattern,char *usage)

    {
    int i,status;
    char *buf;
    static char *funcname="opts_match";

    for (i=0;usage[i]!='\0';i++)
        if (usage[i]=='\n' && usage[i+1]=='-')
            break;
    if (i==0)
        return(0);
    buf=NULL;
    willus_mem_alloc_warn((void **)&buf,i+1,funcname,10);
    strncpy(buf,usage,i);
    buf[i]='\0';
    status=wfile_unix_style_match(pattern,buf);
    willus_mem_free((double **)&buf,funcname);
    return(status);
    }


static int cmdoplines(char *s)

    {
    int i,j;

    for (j=0,i=1;1;i++)
        {
        for (;s[j]!='\n' && s[j]!='\0';j++);
        if (s[j]=='\0')
            return(i);
        j++;
        if (s[j]!=' ')
            return(i);
        }
    }


static char *pr1cmdopt(char *s,int maxlines,int display)

    {
    int j,k,k0,nl;
    char buf[128];

    for (nl=j=0;1;)
        {
        for (k=0;k<18 && s[j]!=' ' && s[j]!='\n' && s[j]!='\0';j++)
            buf[k++]=s[j];
        buf[k]='\0';
        if (display)
            k2printf(TTEXT_BOLD "%s" TTEXT_NORMAL,buf);
        if (k<17 && s[j]==' ' && s[j+1]!=' ')
            {
            for (k0=0;k<18 && s[j]!='\n' && s[j]!='\0';j++,k++)
                buf[k0++]=s[j];
            buf[k0]='\0';
            if (display)
                k2printf(TTEXT_MAGENTA "%s" TTEXT_NORMAL,buf);
            }
        if (s[j]!='\0' && s[j]!='\n')
            {
            for (k=0;s[j]!='\n' && s[j]!='\0';j++)
                buf[k++]=s[j];
            buf[k]='\0';
            if (display)
                k2printf("%s\n",buf);
            }
        nl++;
        if (maxlines>0 && nl>=maxlines)
            return(&s[j]);
        if (s[j]=='\0')
            return(&s[j]);
        j++;
        if (s[j]!=' ')
            return(&s[j]);
        }
    }
    

static void prlines(char *s,int nlines)

    {
    int i,ns;

    for (i=ns=0;s[i]!='\0';i++)
        if (s[i]=='\n')
            ns++;
    k2printf("%s",s);
    for (i=ns;i<nlines;i++)
        k2printf("\n");
    }


static int wait_enter(void)

    {
    char buf[32];

    k2printf(TTEXT_BOLD2 "Press <ENTER> to continue (q to quit)." TTEXT_NORMAL);
    fflush(stdout);
    k2gets(buf,16,"");
    if (tolower(buf[0])=='q')
        return(-1);
    return(0);
    }


