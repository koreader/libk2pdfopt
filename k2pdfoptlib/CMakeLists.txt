project(k2pdfopt C)
cmake_minimum_required(VERSION 2.6)

include_directories(..)

add_library(k2pdfoptlib
	bmpregion.c devprofile.c k2bmp.c k2file.c k2files.c k2gui_cbox.c
	k2gui_osdep.c k2mark.c k2master.c k2mem.c k2menu.c k2ocr.c
	k2parsecmd.c k2proc.c k2publish.c k2settings.c k2settings2cmd.c
	k2sys.c k2usage.c k2version.c pagelist.c pageregions.c textrows.c
	textwords.c userinput.c wrapbmp.c
)

# k2gui.c k2gui_overlay.c 
