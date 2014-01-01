project(k2pdfopt C)
cmake_minimum_required(VERSION 2.6)

include_directories(..)

add_library(k2pdfoptlib
	bmpregion.c  k2gui_cbox.c  k2ocr.c   k2sys.c textwords.c
	devprofile.c k2gui_osdep.c k2parsecmd.c  k2usage.c  userinput.c
	k2bmp.c  k2mark.c  k2proc.c  k2version.c wrapbmp.c
	k2file.c  k2master.c k2publish.c  pagelist.c
	k2files.c k2mem.c k2settings.c  pageregions.c
	k2menu.c  k2settings2cmd.c textrows.c
	
)

# k2gui.c  


