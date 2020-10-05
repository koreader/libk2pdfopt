/*
** k2publish.c   Convert and write the master output bitmap to PDF output pages.
**
** Copyright (C) 2020  http://willus.com
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
                                    int srcpageno,int plus_one);


/*
** flushall==1:  Flush the entire master bitmap but wait for OCR if not enough
**               OCR has been queued up.
** flushall==2:  Flush the entire master and do NOT wait for OCR (clear the bitmap)
*/
void masterinfo_publish(MASTERINFO *masterinfo,K2PDFOPT_SETTINGS *k2settings,int flushall)

    {
#ifdef HAVE_MUPDF_LIB
    static WTEXTCHARS *wtcs=NULL;
    static WTEXTCHARS _wtcs;
    /* static int pageno=0; */
#endif
    WILLUSBITMAP _bmp,*bmp;
    double bmpdpi;
    int local_output_page_count,size_reduction,nocr,queue_pages_only;
#ifdef HAVE_OCR_LIB
    OCRWORDS *ocrwords,_ocrwords;
#else
    void *ocrwords;
#endif
    int bitmap;
    int srcpageno;

    /* Queue up all output pages generated from this source page */ 
    /* This will flush the text re-flow buffer if flushall is NZ */
    while (masterinfo_queue_next_output_page(masterinfo,k2settings,flushall)>0);
#if (WILLUSDEBUGX2==3)
printf("Queued output pages = %d, rowcount=%d\n",masterinfo->queued_page_info.n,masterinfo->rows);
ocrwords_echo(&masterinfo->mi_ocrwords,stdout,2,1);
#endif

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
    ocrwords=&_ocrwords;
    ocrwords_init(ocrwords);
    /*
    ** If using native PDF OCR layer, nocr will be zero.
    */
    nocr=ocrwords_num_queued(&masterinfo->mi_ocrwords);
    queue_pages_only = (flushall<2 && nocr>0
                                   && k2ocr_max_threads()>1 
                                   && nocr < 3*k2ocr_max_threads());
#if (WILLUSDEBUGX2==3)
if (!queue_pages_only && flushall<2)
{
printf("Changing queue_pages_only to 1.\n");
queue_pages_only=1;
}
#endif
    if (nocr>0 && !queue_pages_only)
        {
        if (!k2settings->preview_page)
            k2printf("OCRing %d images ... ",nocr);
#if (WILLUSDEBUGX2==3)
{
static int count=1;
printf("Calling multithreaded ocr. Count=%d, nwords=%d\n",count,masterinfo->mi_ocrwords.n);
ocrwords_echo(&masterinfo->mi_ocrwords,stdout,count++,1);
}
#endif
        k2ocr_multithreaded_ocr(&masterinfo->mi_ocrwords,k2settings);
#if (WILLUSDEBUGX2==3)
ocrwords_echo(&masterinfo->mi_ocrwords,stdout,1,1);
#endif
        nocr=0;
        }
#else
    ocrwords=NULL;
    nocr=0;
    queue_page=0;
#endif
#if (WILLUSDEBUGX2==3)
aprintf(ANSI_GREEN "\n   SRC PAGE %d, nocr=%d, queue=%d, threads=%d\n\n" ANSI_NORMAL,masterinfo->pageinfo.srcpage,nocr,queue_pages_only,k2ocr_max_threads());
#endif
    bmp=&_bmp;
    bmp_init(bmp);
    local_output_page_count=0;
   
    /* Process queued pages if ready */
    while (!queue_pages_only && masterinfo_pop_next_queued_page(masterinfo,k2settings,
                                         bmp,&bmpdpi,&size_reduction,ocrwords,&srcpageno)>0)
        {
#if (WILLUSDEBUGX2==3)
printf("Starting loop, rowcount=%d\n",masterinfo->rows);
ocrwords_echo(ocrwords,stdout,1,1);
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
        k2publish_outline_check(masterinfo,k2settings,srcpageno,0);
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
            if (masterinfo->ocrfilename[0]!='\0')
                ocrwords_to_textfile(ocrwords,masterinfo->ocrfilename,
                                     masterinfo->published_pages>1);
            if (!bitmap && !k2settings->use_crop_boxes)
                {
                int flags;

                flags = k2settings->dst_ocr_visibility_flags
                         | ((k2settings->dst_ocr=='m') ? 0x20 : 0);
#if (WILLUSDEBUGX & 0x400)
printf("Calling pdffile_add_bitmap_with_ocrwords.\n");
#endif
#if ((WILLUSDEBUGX & 0x10000) || (WILLUSDEBUGX2==3))
if (ocrwords!=NULL)
ocrwords_echo(ocrwords,stdout,1,1);
#endif
#if (WILLUSDEBUGX2==3)
{
static int count=1;
char filename[256];
printf("Calling pdffile_add_bitmap_with_ocrwords... (%d x %d, %d dpi)\n",bmp->width,bmp->height,(int)bmpdpi);
printf("    pdffile=%s\n",masterinfo->outfile.filename);
printf("    ptr=%p\n",masterinfo->outfile.f);
printf("    nobjs=%d\n",masterinfo->outfile.n);
printf("    na=%d\n",masterinfo->outfile.na);
printf("    dpi=%d\n",(int)bmpdpi);
printf("    size_red=%g\n",(double)size_reduction);
printf("    words=%d\n",ocrwords->n);
sprintf(filename,"out%02d.png",count++);
bmp_write(bmp,filename,stdout,100);
wfile_written_info(filename,stdout);
}
#endif
                pdffile_add_bitmap_with_ocrwords(&masterinfo->outfile,bmp,bmpdpi,
                                             k2settings->jpeg_quality,size_reduction,
                                             ocrwords,flags);
                }
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
    /*
    ** v2.16 bug fix:  If no destination output generated, we still have to call outline_check().
    */
    if (!queue_pages_only && local_output_page_count==0)
        k2publish_outline_check(masterinfo,k2settings,masterinfo->pageinfo.srcpage,1);
    bmp_free(bmp);
    }


static void k2publish_outline_check(MASTERINFO *masterinfo,K2PDFOPT_SETTINGS *k2settings,
                                    int srcpageno,int plus_one)

    {
    if (k2settings->use_toc!=0 && masterinfo->outline!=NULL
         && masterinfo->outline_srcpage_completed!=srcpageno)
         {
/*
aprintf(ANSI_MAGENTA "\n    --> DEST PAGE %d\n\n" ANSI_NORMAL,masterinfo->published_pages);
*/
         wpdfoutline_set_dstpage(masterinfo->outline,srcpageno,
                                 masterinfo->published_pages+(plus_one?1:0));
         masterinfo->outline_srcpage_completed = srcpageno;
         }
    }
