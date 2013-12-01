/*
** k2publish.c   Convert and write the master output bitmap to PDF output pages.
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
*/

#include "k2pdfopt.h"


void masterinfo_publish(MASTERINFO *masterinfo,K2PDFOPT_SETTINGS *k2settings,int flushall)

    {
#ifdef HAVE_MUPDF_LIB
    static WTEXTCHARS *wtcs=NULL;
    static WTEXTCHARS _wtcs;
    static int pageno=0;
#endif
    WILLUSBITMAP _bmp,*bmp;
    double bmpdpi;
    int size_reduction;
#ifdef HAVE_OCR_LIB
    OCRWORDS *ocrwords,_ocrwords;
#else
    void *ocrwords;
#endif

#ifdef HAVE_MUPDF_LIB
    if (wtcs==NULL)
        {
        wtcs=&_wtcs;
        wtextchars_init(wtcs);
        }
#endif
#ifdef HAVE_OCR_LIB
    if (k2settings->dst_ocr)
        {
        ocrwords=&_ocrwords;
        ocrwords_init(ocrwords);
        }
    else
#else
    ocrwords=NULL;
#endif
    bmp=&_bmp;
    bmp_init(bmp);
    while (masterinfo_get_next_output_page(masterinfo,k2settings,flushall,bmp,
                                           &bmpdpi,&size_reduction,ocrwords)>0)
        {
        if (masterinfo->preview_bitmap!=NULL)
            {
            if (!k2settings->show_marked_source
                        && abs(k2settings->preview_page)==masterinfo->published_pages)
                {
/*
printf("At preview page:  bmp = %d x %d x %d, preview(dst) = %d x %d x %d\n",
bmp->width,bmp->height,bmp->bpp,
masterinfo->preview_bitmap->width,masterinfo->preview_bitmap->height,masterinfo->preview_bitmap->bpp);
*/
                bmp_copy(masterinfo->preview_bitmap,bmp);
                masterinfo->preview_captured=1;
                break;
                }
            continue;
            }

        /* v2.10, Put destination page in outline / bookmarks */
/*
printf("use_toc=%d, outline=%p, spc=%d, srcpage=%d\n",k2settings->use_toc,masterinfo->outline,masterinfo->outline_srcpage_completed,masterinfo->pageinfo.srcpage);
*/
        if (k2settings->use_toc!=0 
             && masterinfo->outline!=NULL
             && masterinfo->outline_srcpage_completed!=masterinfo->pageinfo.srcpage)
             {
             wpdfoutline_set_dstpage(masterinfo->outline,masterinfo->pageinfo.srcpage,
                                     masterinfo->published_pages);
             masterinfo->outline_srcpage_completed = masterinfo->pageinfo.srcpage;
             }

        /*
        ** Nothing to do inside loop if using crop boxes -- they all
        ** get written after all pages have been processed.
        */
        if (k2settings->use_crop_boxes)
            continue;
#ifdef HAVE_OCR_LIB
        if (k2settings->dst_ocr)
            {
            if (k2settings->dst_ocr=='m')
                {
                int i;

                /* Don't re-sort--messes up the copy/paste flow */
                /*
                if (masterinfo->ocrfilename[0]=='\0')
                    ocrwords_sort_by_pageno(ocrwords);
                */
                for (i=0;i<ocrwords->n;i++)
                    {
                    static char *funcname="masterinfo_publish";

                    if (ocrwords->word[i].pageno != pageno)
                        {
                        wtextchars_clear(wtcs);
                        wtextchars_fill_from_page(wtcs,masterinfo->srcfilename,
                                                  ocrwords->word[i].pageno,"");
                        wtextchars_rotate_clockwise(wtcs,360-(int)ocrwords->word[i].rot0_deg);
                        pageno=ocrwords->word[i].pageno;
                        }
                    willus_mem_free((double **)&ocrwords->word[i].text,funcname);
                    wtextchars_text_inside(wtcs,&ocrwords->word[i].text,
                                           ocrwords->word[i].x0,
                                           ocrwords->word[i].y0,
                                           ocrwords->word[i].x0+ocrwords->word[i].w0,
                                           ocrwords->word[i].y0+ocrwords->word[i].h0);
#if (WILLUSDEBUGX & 0x400)
printf("MuPDF Word (%5.1f,%5.1f) - (%5.1f,%5.1f) = '%s'\n",
ocrwords->word[i].x0,
ocrwords->word[i].y0,
ocrwords->word[i].x0+ocrwords->word[i].w0,
ocrwords->word[i].y0+ocrwords->word[i].h0,
ocrwords->word[i].text);
#endif
                    if (ocrwords->word[i].text==NULL || ocrwords->word[i].text[0]=='\0')
                        {
                        ocrwords_remove_words(ocrwords,i,i);
                        i--;
                        }
                    }
                
                }
            if (masterinfo->ocrfilename[0]!='\0')
                ocrwords_to_textfile(ocrwords,masterinfo->ocrfilename,
                                     masterinfo->published_pages>1);

            pdffile_add_bitmap_with_ocrwords(&masterinfo->outfile,bmp,bmpdpi,
                                             k2settings->jpeg_quality,size_reduction,
                                             ocrwords,k2settings->dst_ocr_visibility_flags);
/*
{
static int count=1;
char filename[256];
sprintf(filename,"page%04d.png",count++);
bmp_write(bmp,filename,stdout,100);
}
*/
            masterinfo->wordcount += ocrwords->n;
            ocrwords_free(ocrwords);
            }
        else
#endif
            pdffile_add_bitmap(&masterinfo->outfile,bmp,bmpdpi,
                               k2settings->jpeg_quality,size_reduction);
        }
    bmp_free(bmp);
    }
