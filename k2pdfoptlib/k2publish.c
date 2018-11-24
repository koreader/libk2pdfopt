/*
** k2publish.c   Convert and write the master output bitmap to PDF output pages.
**
** Copyright (C) 2016  http://willus.com
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

static void k2publish_outline_check(MASTERINFO *masterinfo,K2PDFOPT_SETTINGS *k2settings,
                                    int plus_one);


void masterinfo_publish(MASTERINFO *masterinfo,K2PDFOPT_SETTINGS *k2settings,int flushall)

    {
#ifdef HAVE_MUPDF_LIB
    static WTEXTCHARS *wtcs=NULL;
    static WTEXTCHARS _wtcs;
    /* static int pageno=0; */
#endif
    WILLUSBITMAP _bmp,*bmp;
    double bmpdpi;
    int local_output_page_count,size_reduction;
#ifdef HAVE_OCR_LIB
    OCRWORDS *ocrwords,_ocrwords;
#else
    void *ocrwords;
#endif
    int bitmap;

    bitmap=k2settings_output_is_bitmap(k2settings);
/*
aprintf(ANSI_GREEN "\n   @masterinfo_publish(flushall=%d)....\n\n" ANSI_NORMAL,flushall);
*/
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
#endif
        ocrwords=NULL;
    bmp=&_bmp;
    bmp_init(bmp);
    local_output_page_count=0;
    while (masterinfo_get_next_output_page(masterinfo,k2settings,flushall,bmp,
                                           &bmpdpi,&size_reduction,ocrwords)>0)
        {
#if (WILLUSDEBUGX & 1)
aprintf(ANSI_GREEN "\n   SRC PAGE %d\n\n" ANSI_NORMAL,masterinfo->pageinfo.srcpage);
#endif
        local_output_page_count++;
        masterinfo->output_page_count++;
        if (masterinfo->preview_bitmap!=NULL)
            {
            if (ocrwords!=NULL)
                ocrwords_free(ocrwords); /* Don't really need this, but just for insurance */
            if (!k2settings->show_marked_source
                        && abs(k2settings->preview_page)==masterinfo->published_pages)
                {
/*
printf("At preview page:  bmp = %d x %d x %d, preview(dst) = %d x %d x %d\n",
bmp->width,bmp->height,bmp->bpp,
masterinfo->preview_bitmap->width,masterinfo->preview_bitmap->height,masterinfo->preview_bitmap->bpp);
*/              bmp_copy(masterinfo->preview_bitmap,bmp);
                masterinfo->preview_captured=1;
                break;
                }
            continue;
            }

        /* v2.16, outline / bookmark check done in separate function. */
#if (WILLUSDEBUGX & 1)
printf("k2publish_outline_check\n");
#endif
        k2publish_outline_check(masterinfo,k2settings,0);
#if (WILLUSDEBUGX & 1)
printf("Done k2publish_outline_check, usecrop=%d, dst_ocr=%c\n",k2settings->use_crop_boxes,k2settings->dst_ocr);
#endif
/*
printf("use_toc=%d, outline=%p, spc=%d, srcpage=%d\n",k2settings->use_toc,masterinfo->outline,masterinfo->outline_srcpage_completed,masterinfo->pageinfo.srcpage);
*/

        if (bitmap)
            {
            char filename[MAXFILENAMELEN];

            filename_substitute(filename,k2settings->dst_opname_format,masterinfo->srcfilename,
                                masterinfo->filecount,masterinfo->output_page_count,"");
            bmp_set_dpi(bmpdpi);
            if (!stricmp(wfile_ext(filename),"jpg") 
                 || !stricmp(wfile_ext(filename),"jpeg"))
                bmp_promote_to_24(bmp);
            bmp_write(bmp,filename,NULL,k2settings->jpeg_quality<1?93:k2settings->jpeg_quality);
            bitmap_file_echo_status(filename);
            }
        /*
        ** Nothing to do inside loop if using crop boxes -- they all
        ** get written after all pages have been processed.
        */
#ifdef HAVE_OCR_LIB
        if (k2settings->dst_ocr)
            {
            int flags_extra;

            flags_extra=0;
            if (k2settings->dst_ocr=='m')
                {
                flags_extra=0x20;
                /* Don't re-sort--messes up the copy/paste flow */
                /*
                if (masterinfo->ocrfilename[0]=='\0')
                    ocrwords_sort_by_pageno(ocrwords);
                */
/*
** This section no longer needed in v2.20.  The text in the ocrword boxes
** has already been determined by k2ocr_ocrwords_get_from_ocrlayer() in k2ocr.c
*/
/*
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
*/
                
                }
            if (masterinfo->ocrfilename[0]!='\0')
                ocrwords_to_textfile(ocrwords,masterinfo->ocrfilename,
                                     masterinfo->published_pages>1);

            if (!bitmap && !k2settings->use_crop_boxes)
                {
#if (WILLUSDEBUGX & 0x400)
printf("Calling pdffile_add_bitmap_with_ocrwords.\n");
#endif
#if (WILLUSDEBUGX & 0x10000)
if (ocrwords!=NULL)
{
int k;
printf("flags_extra= %d\n",flags_extra);
printf("PAGE OF WORDS\n");
for (k=0;k<ocrwords->n;k++)
printf("%3d. '%s'\n",k,ocrwords->word[k].text);
}
#endif
#if (WILLUSDEBUGX & 1)
printf("Calling pdffile_add_bitmap_with_ocrwords... (%d x %d, %d dpi)\n",bmp->width,bmp->height,(int)bmpdpi);
printf("    pdffile=%s\n",masterinfo->outfile.filename);
printf("    ptr=%p\n",masterinfo->outfile.f);
printf("    nobjs=%d\n",masterinfo->outfile.n);
printf("    na=%d\n",masterinfo->outfile.na);
#endif
                pdffile_add_bitmap_with_ocrwords(&masterinfo->outfile,bmp,bmpdpi,
                                             k2settings->jpeg_quality,size_reduction,
                                             ocrwords,k2settings->dst_ocr_visibility_flags
                                                        | flags_extra);
                }
#if (WILLUSDEBUGX & 0x400)
printf("Back from pdffile_add_bitmap_with_ocrwords.\n");
#endif
/*
{
static int count=1;
char filename[MAXFILENAMELEN];
sprintf(filename,"page%04d.png",count++);
bmp_write(bmp,filename,stdout,100);
}
*/
            masterinfo->wordcount += ocrwords->n;
            ocrwords_free(ocrwords);
            }
        else if (!bitmap && !k2settings->use_crop_boxes)
#endif
            {
#if (WILLUSDEBUGX & 1)
printf("Calling pdffile_add_bitmap... (%d x %d, %d dpi)\n",bmp->width,bmp->height,(int)bmpdpi);
#endif
            pdffile_add_bitmap(&masterinfo->outfile,bmp,bmpdpi,
                               k2settings->jpeg_quality,size_reduction);
            }
        }
#if (WILLUSDEBUGX & 1)
printf("local_output_page_count=%d\n",local_output_page_count);
#endif
    /*
    ** v2.16 bug fix:  If no destination output generated, we still have to call outline_check().
    */
    if (local_output_page_count==0)
        k2publish_outline_check(masterinfo,k2settings,1);
    bmp_free(bmp);
    }


static void k2publish_outline_check(MASTERINFO *masterinfo,K2PDFOPT_SETTINGS *k2settings,
                                    int plus_one)

    {
    if (k2settings->use_toc!=0 && masterinfo->outline!=NULL
         && masterinfo->outline_srcpage_completed!=masterinfo->pageinfo.srcpage)
         {
/*
aprintf(ANSI_MAGENTA "\n    --> DEST PAGE %d\n\n" ANSI_NORMAL,masterinfo->published_pages);
*/
         wpdfoutline_set_dstpage(masterinfo->outline,masterinfo->pageinfo.srcpage,
                                 masterinfo->published_pages+(plus_one?1:0));
         masterinfo->outline_srcpage_completed = masterinfo->pageinfo.srcpage;
         }
    }
