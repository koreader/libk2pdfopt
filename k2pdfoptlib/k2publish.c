/*
** k2publish.c   Convert and write the master output bitmap to PDF output pages.
**
** Copyright (C) 2012  http://willus.com
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
    WILLUSBITMAP _bmp,*bmp;
    double bmpdpi;
    int size_reduction;
#ifdef HAVE_OCR_LIB
    OCRWORDS *ocrwords,_ocrwords;
#else
    void *ocrwords;
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
        /*
        ** Nothing to do inside loop if using crop boxes -- they all
        ** get written after all pages have been processed.
        */
        if (k2settings->use_crop_boxes)
            continue;
#ifdef HAVE_OCR_LIB
        if (k2settings->dst_ocr)
            {
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
