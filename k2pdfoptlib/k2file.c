/*
** k2file.c      K2pdfopt file handling and main file processing
**               function (k2pdfopt_proc_one()).
**
** Copyright (C) 2014  http://willus.com
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

static int k2files_overwrite=0;

static void   k2pdfopt_proc_arg(K2PDFOPT_SETTINGS *k2settings,char *arg,int process,
                                K2PDFOPT_OUTPUT *k2out);
static double k2pdfopt_proc_one(K2PDFOPT_SETTINGS *k2settings,char *filename,double rot_deg,
                                K2PDFOPT_OUTPUT *k2out);
static int k2_handle_preview(K2PDFOPT_SETTINGS *k2settings,MASTERINFO *masterinfo,
                             int k2mark_page_count,WILLUSBITMAP *markedbmp,
                             K2PDFOPT_OUTPUT *k2out);
static int    filename_comp(char *name1,char *name2);
static void   filename_substitute(char *dst,char *fmt,char *src,int count,char *defext0);
static int    overwrite_fail(char *outname,double overwrite_minsize_mb);
static int toclist_valid(char *s,FILE *out);
static WPDFOUTLINE *wpdfoutline_from_pagelist(char *pagelist,int maxpages);
static int tocwrites=0;


/*
** If arg is a file wildcard specification, then figure out all matches and pass
** each match, one by one, to k2_proc_arg.
*/
void k2pdfopt_proc_wildarg(K2PDFOPT_SETTINGS *k2settings,char *arg,int process,
                           K2PDFOPT_OUTPUT *k2out)

    {
    int i;

    /* Init width to -1 */
    if (k2settings->preview_page!=0 && k2out->bmp!=NULL)
        k2out->bmp->width = -1;
    if (wfile_status(arg)==0)
        {
        FILELIST *fl,_fl;

        fl=&_fl;
        filelist_init(fl);
        filelist_fill_from_disk_1(fl,arg,0,0);
        if (fl->n==0)
            {
#ifdef HAVE_K2GUI
            if ((!k2gui_active() && process) || (k2gui_active() && !process))
#endif
            k2printf(TTEXT_WARN "\n** File or folder %s could not be opened. **\n\n" TTEXT_NORMAL,arg);
#ifdef HAVE_K2GUI
            if (process && k2gui_active())
                {
                char buf[512];
                sprintf(buf,"File or folder %s cannot be opened.",arg);
                k2gui_cbox_increment_error_count();
                k2gui_cbox_set_pages_completed(0,buf);
                }
#endif
            return;
            }
        for (i=0;i<fl->n;i++)
            {
            char fullname[512];
            wfile_fullname(fullname,fl->dir,fl->entry[i].name);
            k2pdfopt_proc_arg(k2settings,fullname,process,k2out);
            }
        }
    else
        k2pdfopt_proc_arg(k2settings,arg,process,k2out);
    }


/*
** If arg is a folder, look for files inside of it for PDFs / DJVU files and process
** one by one, otherwise just process the passed argument.
**
** Processing file is two steps:
**     1. If auto-rotation is requested, determine the proper rotation of the file.
**     2. Process the file with the determined rotation.
**
*/
static void k2pdfopt_proc_arg(K2PDFOPT_SETTINGS *k2settings,char *arg,int process,
                              K2PDFOPT_OUTPUT *k2out)

    {
    char filename[256];
    int i;
    double rot;
    int autorot;

    strcpy(filename,arg);
    if (wfile_status(filename)==0)
        {
#ifdef HAVE_K2GUI
        if ((!k2gui_active() && process) || (k2gui_active() && !process))
#endif
        k2printf(TTEXT_WARN "\n** File or folder %s could not be opened. **\n\n" TTEXT_NORMAL,filename);
#ifdef HAVE_K2GUI
        if (process && k2gui_active())
            {
            char buf[512];
            k2gui_cbox_increment_error_count();
            sprintf(buf,"File %s cannot be opened.",filename);
            k2gui_cbox_set_pages_completed(0,buf);
            }
#endif
        return;
        }
    if (k2settings->preview_page!=0)
        autorot = (fabs(k2settings->src_rot - SRCROT_AUTOPREV)<.5);
    else
        autorot = (fabs(k2settings->src_rot - SRCROT_AUTOPREV)<.5
                      || fabs(k2settings->src_rot - SRCROT_AUTO)<.5
                      || fabs(k2settings->src_rot - SRCROT_AUTOEP)<.5);
    /* If folder, first process all PDF/DJVU/PS files in the folder */
    if (wfile_status(filename)==2)
        {
        static char *eolist[]={""};
        static char *pdflist[]={"*.pdf","*.djvu","*.djv","*.ps","*.eps",""};
        FILELIST *fl,_fl;

        fl=&_fl;
        filelist_init(fl);
        filelist_fill_from_disk(fl,filename,pdflist,eolist,0,0);
        if (fl->n>0)
            {
            for (i=0;i<fl->n;i++)
                {
                char fullname[512];

                wfile_fullname(fullname,filename,fl->entry[i].name);
                if (autorot)
                    {
                    if (process)
                        rot=k2pdfopt_proc_one(k2settings,fullname,SRCROT_AUTO,k2out);
                    else
                        rot=0.;
                    }
                else
                    rot=k2settings->src_rot < -990. ? 0. : k2settings->src_rot;
                if (process)
                    k2pdfopt_proc_one(k2settings,fullname,rot,k2out);
                if (!process || k2out->status==0)
                    k2out->filecount++;
#ifdef HAVE_K2GUI
                if (process && k2gui_active())
                    {
                    if (k2out->status!=0)
                        k2gui_cbox_error(filename,k2out->status);
                    else
                        k2gui_cbox_set_files_completed(k2out->filecount,NULL);
                    }
#endif
                }
            }
        filelist_free(fl);
        }
    if (autorot)
        {
        if (process)
            rot=k2pdfopt_proc_one(k2settings,filename,SRCROT_AUTO,k2out);
        else
            rot=0.;
        }
    else
        rot=k2settings->src_rot < -990. ? 0. : k2settings->src_rot;
    if (process)
        k2pdfopt_proc_one(k2settings,filename,rot,k2out);
    if (!process || k2out->status==0)
        k2out->filecount++;
#ifdef HAVE_K2GUI
    if (process && k2gui_active())
        {
        if (k2out->status!=0)
            k2gui_cbox_error(filename,k2out->status);
        else
            k2gui_cbox_set_files_completed(k2out->filecount,NULL);
        }
#endif
    }


/*
** k2pdfopt_proc_one() is the main source file processing function in k2pdfopt.
** 
** Depending on the value of rot_deg, it either determines the correct rotation of
** the passed file, or it processes it and converts it.
**
** The basic idea is to parse the source document into rectangular regions
** (held in the BMPREGION structures) and then to place these regions into
** the master destination bitmap (kept track of in MASTERINFO structure).
** You can think of this bitmap as a sort of "infinitely scrolling" output
** bitmap which is then cut into output pages.
**
** The bmpregion_source_page_add() function parses the source file.
**
** The masterinfo_publish() cuts the output bitmap into destination pages.
**
** If rot_deg == SRCROT_AUTO, then the rotation correction of the source
** file is computed and returned, but no other processing is done.
**
** Otherwise, the source file is processed.
*/
static double k2pdfopt_proc_one(K2PDFOPT_SETTINGS *k2settings0,char *filename,double rot_deg,
                                K2PDFOPT_OUTPUT *k2out)

    {
    static K2PDFOPT_SETTINGS _k2settings,*k2settings;
    static MASTERINFO _masterinfo,*masterinfo;
    static PDFFILE _mpdf,*mpdf;
    char dstfile[256];
    char markedfile[256];
    char rotstr[128];
    WILLUSBITMAP _src,*src;
    WILLUSBITMAP _srcgrey,*srcgrey;
    WILLUSBITMAP _marked,*marked;
    WILLUSBITMAP preview_internal;
    int i,status,pw,np,src_type,second_time_through,or_detect,orep_detect,preview;
    int pagecount,pagestep,pages_done,local_tocwrites;
    int errcnt,pixwarn;
    FILELIST *fl,_fl;
    int folder,dpi;
    double size,bormean;
    char *mupdffilename;
    extern int k2mark_page_count;
    static char *funcname="k2pdfopt_proc_one";
    static char *readerr=TTEXT_WARN "\a\n ** ERROR reading page %d from " TTEXT_BOLD2 "%s" TTEXT_WARN ".\n\n" TTEXT_NORMAL;
    static char *readlimit=TTEXT_WARN "\a\n ** (No more read errors will be echoed for file %s.)\n\n" TTEXT_NORMAL;
#ifdef HAVE_MUPDF_LIB
    static char *mupdferr_trygs=TTEXT_WARN "\a\n ** ERROR reading from " TTEXT_BOLD2 "%s" TTEXT_WARN "using MuPDF.  Trying Ghostscript...\n\n" TTEXT_NORMAL;
#endif
/*
extern void willus_mem_debug_update(char *);
*/

/*
printf("@k2pdfopt_proc_one(filename='%s', rot_deg=%g, preview_bitmap=%p)\n",filename,rot_deg,k2out->bmp);
*/
    local_tocwrites=0;
    k2out->status = 1;
    k2settings=&_k2settings;
    k2pdfopt_settings_copy(k2settings,k2settings0);
#ifdef HAVE_K2GUI
    if (k2gui_active())
        k2gui_cbox_set_filename(filename);
#endif
    mpdf=&_mpdf;
    /* Must be called once per conversion to init margins / devsize / output size */
    k2pdfopt_settings_sanity_check(k2settings);
    k2pdfopt_settings_new_source_document_init(k2settings);
    errcnt=0;
    pixwarn=0;
    mupdffilename=_masterinfo.srcfilename;
    strncpy(mupdffilename,filename,MAXFILENAMELEN-1);
    mupdffilename[MAXFILENAMELEN-1]='\0';
    or_detect=OR_DETECT(rot_deg);
    orep_detect=OREP_DETECT(k2settings);
    if ((fabs(k2settings->src_rot-SRCROT_AUTO)<.5 || orep_detect) && !or_detect)
        second_time_through=1;
    else
        second_time_through=0;
    /* Don't care about rotation if just echoing page count */
    if (k2settings->echo_source_page_count && second_time_through==0)
        return(0.);
    if (or_detect && k2settings->src_dpi>300)
        dpi=300;
    else
        dpi=k2settings->src_dpi;
    folder=(wfile_status(filename)==2);
    /*
    if (folder && !second_time_through)
        k2printf("Processing " TTEXT_INPUT "BITMAP FOLDER %s" TTEXT_NORMAL "...\n",
               filename);
    */
    /*
    else
        k2printf("Processing " TTEXT_BOLD2 "PDF FILE %s" TTEXT_NORMAL "...\n",
               filename);
    */
    fl=&_fl;
    filelist_init(fl);
    if (folder)
        {
        char basename[256];
        static char *iolist[]={"*.png","*.jpg",""};
        static char *eolist[]={""};

        wfile_basespec(basename,filename);
        if (!second_time_through)
            k2printf("Searching folder " TTEXT_BOLD2 "%s" TTEXT_NORMAL " ... ",basename);
        fflush(stdout);
        filelist_fill_from_disk(fl,filename,iolist,eolist,0,0);
        if (fl->n<=0)
            {
            if (!second_time_through)
                k2printf(TTEXT_WARN "\n** No bitmaps found in folder %s.\n\n" 
                        TTEXT_NORMAL,filename);
            k2out->status=2;
            return(0.);
            }
        if (!second_time_through)
            k2printf("%d bitmaps found in %s.\n",(int)fl->n,filename);
        filelist_sort_by_name(fl);
        }
    src=&_src;
    srcgrey=&_srcgrey;
    marked=&_marked;
    bmp_init(src);
    bmp_init(srcgrey);
    bmp_init(marked);
    pw=0;
    /*
    ** Determine source type
    */
    if (folder)
        src_type = SRC_TYPE_BITMAPFOLDER;
    else if (!stricmp(wfile_ext(filename),"pdf"))
        src_type = SRC_TYPE_PDF;
    else if (!stricmp(wfile_ext(filename),"djvu"))
        src_type = SRC_TYPE_DJVU;
    else if (!stricmp(wfile_ext(filename),"djv"))
        src_type = SRC_TYPE_DJVU;
    else if (!stricmp(wfile_ext(filename),"ps"))
        src_type = SRC_TYPE_PS;
    else if (!stricmp(wfile_ext(filename),"eps"))
        src_type = SRC_TYPE_PS;
    else
        src_type = SRC_TYPE_OTHER;
#ifndef HAVE_DJVU_LIB
    if (src_type==SRC_TYPE_DJVU)
        {
        if (!or_detect)
            k2printf(TTEXT_WARN
                    "\a\n\n** DjVuLibre not compiled into this version of k2pdfopt. **\n\n"
                          "** Cannot process file %s. **\n\n" TTEXT_NORMAL,filename);
        k2out->status=3;
        return(0.);
        }
#endif
    if (src_type==SRC_TYPE_PS)
        k2settings->usegs=1;
    /*
    ** Turn off native PDF output if source is not PDF
    */
    if (src_type!=SRC_TYPE_PDF)
        {
        if (k2settings->use_crop_boxes && !or_detect)
            k2printf(TTEXT_WARN
                     "\n** Native PDF output mode turned off on file %s. **\n"
                     "** (It is not a PDF file.) **\n\n",filename);
        k2settings->use_crop_boxes=0;
#ifdef HAVE_OCR_LIB
        if (k2settings->dst_ocr=='m')
            k2settings->dst_ocr=0;
#endif
        }
    masterinfo=&_masterinfo;
    masterinfo_init(masterinfo,k2settings);
    if (k2settings->preview_page!=0 && !or_detect)
        {
        preview=1;
        if (k2out->bmp!=NULL)
            masterinfo->preview_bitmap=k2out->bmp;
        else
            {
            masterinfo->preview_bitmap=&preview_internal;
            bmp_init(masterinfo->preview_bitmap);
            }
        }
    else
        preview=0;
    if (!or_detect && !preview)
        {
        static int dstfilecount=0;

        wfile_newext(dstfile,filename,"");
        dstfilecount++;
        filename_substitute(dstfile,k2settings->dst_opname_format,filename,dstfilecount,"pdf");
#ifdef HAVE_OCR_LIB
        if (k2settings->ocrout[0]!='\0' && k2settings->dst_ocr)
            filename_substitute(masterinfo->ocrfilename,k2settings->ocrout,filename,dstfilecount,"txt");
        else
#endif
            masterinfo->ocrfilename[0]='\0';
        if (!filename_comp(dstfile,filename))
            {
            k2printf(TTEXT_WARN "\n\aSource file and ouput file have the same name!" TTEXT_NORMAL "\n\n");
            k2printf("    Source file = '%s'\n",filename);
            k2printf("    Output file = '%s'\n",dstfile);
            k2printf("    Output file name format string = '%s'\n",k2settings->dst_opname_format);
            k2printf("\nOperation aborted.\n");
            k2sys_exit(k2settings,50);
            }
        if ((status=overwrite_fail(dstfile,k2settings->overwrite_minsize_mb))!=0)
            {
            masterinfo_free(masterinfo,k2settings);
            if (folder)
                filelist_free(fl);
            if (status<0)
                k2sys_exit(k2settings,20);
            k2out->status=4;
            return(0.);
            }
        if (pdffile_init(&masterinfo->outfile,dstfile,1)==NULL)
            {
            k2printf(TTEXT_WARN "\n\aCannot open PDF file %s for output!" TTEXT_NORMAL "\n\n",dstfile);
#ifdef HAVE_K2GUI
            if (k2gui_active())
                {
                k2gui_okay("Failed to open output file",
                           "Cannot open PDF file %s for output!\n"
                           "Maybe another application has it open already?\n"
                           "Conversion failed!",dstfile);
                k2out->status=4;
                return(0.);
                }
#endif
            k2sys_exit(k2settings,30);
            }
        k2out->outname=NULL;
        /* Return output file name in k2out for GUI */
        willus_mem_alloc((double **)&k2out->outname,(long)(strlen(dstfile)+1),funcname);
        if (k2out->outname!=NULL)
            strcpy(k2out->outname,dstfile);
        if (k2settings->use_crop_boxes)
            pdffile_close(&masterinfo->outfile);
        if (k2settings->show_marked_source)
            {
            filename_substitute(markedfile,"%s_marked",filename,0,"pdf");
            if (pdffile_init(mpdf,markedfile,1)==NULL)
                {
                k2printf(TTEXT_WARN "\n\aCannot open PDF file %s for marked output!" TTEXT_NORMAL "\n\n",markedfile);
                k2sys_exit(k2settings,40);
                }
            }
        }
    if (src_type==SRC_TYPE_PDF || src_type==SRC_TYPE_DJVU)
        {
        wsys_set_decimal_period(1);
#ifdef HAVE_MUPDF_LIB
        if (src_type==SRC_TYPE_PDF)
            {
            np=wmupdf_numpages(mupdffilename);
#if (defined(WIN32) || defined(WIN64))
            if (np<0)
                {
                int ns;
                ns=wsys_filename_8dot3(mupdffilename,filename,MAXFILENAMELEN-1);
                if (ns>0 && stricmp(filename,mupdffilename))
                    np=wmupdf_numpages(mupdffilename);
                else
                    strcpy(mupdffilename,filename);
                }
#endif
            /* Get bookmarks / outline from PDF file */
            if (!or_detect && k2settings->use_toc!=0 && !toclist_valid(k2settings->toclist,NULL))
                {
                masterinfo->outline=wpdfoutline_read_from_pdf_file(mupdffilename);
                /* Save TOC if requested */
                if (k2settings->tocsavefile[0]!='\0')
                    {
                    FILE *f;
                    f=fopen(k2settings->tocsavefile,tocwrites==0?"w":"a");
                    if (f!=NULL)
                        {
                        int i;
                        fprintf(f,"%sFILE: %s\n",tocwrites==0?"":"\n\n",mupdffilename);
                        for (i=strlen(mupdffilename)+6;i>0;i--)
                            fputc('-',f);
                        fprintf(f,"\n");
                        if (masterinfo->outline!=NULL)
                            wpdfoutline_echo2(masterinfo->outline,0,f);
                        else
                            fprintf(f,"(No outline info in file.)\n");
                        fclose(f);
                        tocwrites++;
                        local_tocwrites++;
                        }
                    }
                }
            }
        else
#endif
#ifdef HAVE_DJVU_LIB
        if (src_type==SRC_TYPE_DJVU)
            np=bmpdjvu_numpages(filename);
        else
#endif
            np=-1;
        wsys_set_decimal_period(1);
#ifdef HAVE_MUPDF_LIB
        if (np==-1 && (k2settings->usegs<=0) && src_type==SRC_TYPE_PDF)
            {
            k2printf(mupdferr_trygs,filename);
            if (k2settings->usegs==0)
                k2settings->usegs=1;
            }
#endif
#ifdef HAVE_Z_LIB
        if (np<=0 && src_type==SRC_TYPE_PDF)
            np=pdf_numpages(filename);
#endif
        }
    else if (src_type==SRC_TYPE_BITMAPFOLDER)
        np=fl->n;
    else
        np=-1;
    if (k2settings->echo_source_page_count)
        {
        printf("\"%s\" page count = %d\n",mupdffilename,np);
        masterinfo_free(masterinfo,k2settings);
        if (folder)
            filelist_free(fl);
        return(0.);
        }
    masterinfo->srcpages = np;
    if (!or_detect && toclist_valid(k2settings->toclist,stdout))
        {
        if (pagelist_valid_page_range(k2settings->toclist))
            masterinfo->outline=wpdfoutline_from_pagelist(k2settings->toclist,masterinfo->srcpages);
        else
            masterinfo->outline=wpdfoutline_read_from_text_file(k2settings->toclist);
        }
    pagecount = np<0 ? -1 : pagelist_count(k2settings->pagelist,np);
#ifdef HAVE_K2GUI
    if (k2gui_active())
        {
        k2gui_cbox_set_num_pages(pagecount<0 ? 1 : pagecount);
        k2gui_cbox_set_pages_completed(0,NULL);
        }
#endif
    if (pagecount<0 || !or_detect)
        pagestep=1;
    else
        {
        pagestep=pagecount/10;
        if (pagestep<1)
            pagestep=1;
        }
    pages_done=0;
    if (np>0 && pagecount==0)
        {
        if (!second_time_through)
            k2printf("\a\n" TTEXT_WARN "No %ss to convert (-p %s)!" TTEXT_NORMAL "\n\n",
                     folder?"file":"page",k2settings->pagelist);
        masterinfo_free(masterinfo,k2settings);
        if (folder)
            filelist_free(fl);
        k2out->status=5;
        return(0.);
        }
    if (!second_time_through)
        {
        k2printf("Reading ");
        if (pagecount>0)
           {
           if (pagecount<np)
               k2printf("%d out of %d %s%s",pagecount,np,folder?"file":"page",np>1?"s":"");
           else
               k2printf("%d %s%s",np,folder?"file":"page",np>1?"s":"");
           }
        else
           k2printf("%ss",folder?"file":"page");
        k2printf(" from " TTEXT_BOLD2 "%s" TTEXT_NORMAL " ...\n",filename);
        }
    if (or_detect)
        k2printf("\nDetecting document orientation ... ");
    bormean=1.0;
    for (i=0;1;i+=pagestep)
        {
        char bmpfile[256];
        int pageno;
/*
sprintf(bmpfile,"i=%d",i);
willus_mem_debug_update(bmpfile);
*/
        pageno=0;
        if (pagecount>0 && i+1>pagecount)
            break;
        pageno = pagelist_page_by_index(k2settings->pagelist,i,np);
        if (!pagelist_page_by_index(k2settings->pagelist,pageno,np))
            continue;
        if (folder)
            {
            if (pageno-1>=fl->n)
                continue;
            wfile_fullname(bmpfile,fl->dir,fl->entry[pageno-1].name);
            status=bmp_read(src,bmpfile,stdout);
            if (status<0)
                {
                if (!second_time_through)
                    k2printf(TTEXT_WARN "\n\aCould not read file %s.\n" TTEXT_NORMAL,bmpfile);
                continue;
                }
            }
        else
            { 
            double npix;

            /* If not a PDF/DJVU/PS file, only read it once. */
            if (i>0 && src_type!=SRC_TYPE_PDF && src_type!=SRC_TYPE_DJVU
                    && src_type!=SRC_TYPE_PS)
                break;

            /* Pre-read at low dpi to check bitmap size */
            wsys_set_decimal_period(1);
            status=bmp_get_one_document_page(src,k2settings,src_type,mupdffilename,pageno,10.,8,
                                             stdout);
            wsys_set_decimal_period(1);
            if (status<0)
                {
                errcnt++;
                if (errcnt<=10)
                    {
                    k2printf(readerr,pageno,filename);
                    if (errcnt==10)
                        k2printf(readlimit,filename);
                    }
                /* Error reading PS probably means we've run out of pages. */
                if (src_type==SRC_TYPE_PS)
                    break;
                continue;
                }

            /* Sanity check the bitmap size */
            npix = (double)(dpi/10.)*(dpi/10.)*src->width*src->height;
            if (npix > 2.5e8 && !pixwarn)
                {
                int ww,hh;
                ww=(int)((double)(dpi/10.)*src->width+.5);
                hh=(int)((double)(dpi/10.)*src->height+.5);
                k2printf("\a\n" TTEXT_WARN "\n\a ** Source resolution is very high (%d x %d pixels)!\n"
                        "    You may want to reduce the -odpi or -idpi setting!\n"
                        "    k2pdfopt may crash when reading the source file..."
                        TTEXT_NORMAL "\n\n",ww,hh);
                pixwarn=1;
                }

            /* Read again at nominal source dpi */
            wsys_set_decimal_period(1);
            if (k2settings->dst_color)
                status=bmp_get_one_document_page(src,k2settings,src_type,mupdffilename,pageno,
                                                 dpi,24,stdout);
            else
                status=bmp_get_one_document_page(src,k2settings,src_type,mupdffilename,pageno,
                                                 dpi,8,stdout);
            wsys_set_decimal_period(1);
            if (status<0)
                {
                errcnt++;
                if (errcnt<=10)
                    {
                    k2printf(readerr,pageno,filename);
                    if (errcnt==10)
                        aprintf(readlimit,filename);
                    }
                /* Error reading PS probably means we've run out of pages. */
                if (src_type==SRC_TYPE_PS)
                    break;
                continue;
                }
            }
        k2mark_page_count = i+1;

        {
        BMPREGION region;

        /* Got Good Page Render */
        bmpregion_init(&region);
        if (masterinfo_new_source_page_init(masterinfo,k2settings,src,srcgrey,marked,
                                 &region,rot_deg,&bormean,rotstr,pageno,stdout)==0)
            {
            /* v2.15 -- memory leak fix */
            bmpregion_free(&region);
            pages_done++;
            continue;
            }
        k2printf("\n" TTEXT_HEADER "SOURCE PAGE %d",pageno);
        if (pagecount>0)
            {
            if (k2settings->pagelist[0]!='\0')
                k2printf(" (%d of %d)",pages_done+1,pagecount);
            else
                k2printf(" of %d",pagecount);
            }
        k2printf(TTEXT_NORMAL 
                " (%.1f x %.1f in) ... %s",(double)srcgrey->width/k2settings->src_dpi,
                  (double)srcgrey->height/k2settings->src_dpi,rotstr);
        fflush(stdout);

        /* Parse the source bitmap for viewable regions */
        bmpregion_source_page_add(&region,k2settings,masterinfo,1,pages_done++);
        /* v2.15 memory leak fix */
        bmpregion_free(&region);
        } /* End declaration of BMPREGION region */
#ifdef HAVE_K2GUI
        if (k2gui_active())
            k2gui_cbox_set_pages_completed(pages_done,NULL);
#endif
        if (k2settings->verbose)
            {
            k2printf("    master->rows=%d\n",masterinfo->rows);
            k2printf("Publishing...\n");
            }
        /* Reset the display order for this source page */
        if (k2settings->show_marked_source)
            mark_source_page(k2settings,masterinfo,NULL,0,0xf);
        /*
        ** v2.10 Call masterinfo_publish() no matter what.  If we've just kicked out a
        **       page, it doesn't matter.  It will do nothing.
        */
        masterinfo_publish(masterinfo,k2settings,
                           masterinfo_should_flush(masterinfo,k2settings));
        if (preview && k2_handle_preview(k2settings,masterinfo,k2mark_page_count,
                                         k2settings->dst_color?marked:src,k2out))
            {
            bmp_free(marked);
            bmp_free(srcgrey);
            bmp_free(src);
            masterinfo_free(masterinfo,k2settings);
            if (folder)
                filelist_free(fl);
            k2out->status=0;
            return(0.);
            }
        if (k2settings->show_marked_source && !preview)
            publish_marked_page(mpdf,k2settings->dst_color ? marked : src,k2settings->src_dpi);
        k2printf("%d new pages saved.\n",masterinfo->published_pages-pw);
        pw=masterinfo->published_pages;
        }
/*
willus_mem_debug_update("End");
*/
    /* Didn't find the preview page yet--push out final page. */
    if (preview)
        {
        masterinfo_flush(masterinfo,k2settings);
        if (!k2_handle_preview(k2settings,masterinfo,k2mark_page_count,
                               k2settings->dst_color?marked:src,k2out))
            {
            /* No preview bitmap--return zero-width bitmap */
            if (k2out->bmp==NULL)
                bmp_free(masterinfo->preview_bitmap);
            else
                k2out->bmp->width=0;
            }
        bmp_free(marked);
        bmp_free(srcgrey);
        bmp_free(src);
        masterinfo_free(masterinfo,k2settings);
        if (folder)
            filelist_free(fl);
        k2out->status=0;
        return(0.);
        }
    bmp_free(marked);
    bmp_free(srcgrey);
    bmp_free(src);
    /* Determine orientation of document */
    if (or_detect)
        {
        if (pages_done>0)
            {
            double thresh;
            /*
            ** bormean = 1.0 means neutral
            ** bormean >> 1.0 means document is likely portrait (no rotation necessary)
            ** bormean << 1.0 means document is likely landscape (need to rotate it)
            */
            bormean = pow(bormean,1./pages_done);
            thresh=10.-(double)pages_done/2.;
            if (thresh<5.)
                thresh=5.;
            if (bormean < 1./thresh)
                {
                k2printf("Rotating clockwise.\n");
                masterinfo_free(masterinfo,k2settings);
                if (folder)
                    filelist_free(fl);
                k2out->status=0;
                return(270.);
                }
            }
        k2printf("No rotation necessary.\n");
        masterinfo_free(masterinfo,k2settings);
        if (folder)
            filelist_free(fl);
        k2out->status=0;
        return(0.);
        }
    /*
    ** v2.10 -- Calling masterinfo_flush() without checking if a page has just been
    **          been flushed is fine at the end.  If there is nothing left
    **          in the master output bitmap, it won't do anything.
    */
    /*
    if (k2settings->dst_break_pages<=0 && !k2settings_gap_override(k2settings))
    */
        masterinfo_flush(masterinfo,k2settings);
    {
    char cdate[128],author[256],title[256];

#ifdef HAVE_MUPDF_LIB
    if (src_type==SRC_TYPE_PDF)
        {
        if (wmupdf_info_field(mupdffilename,"Author",author,255)<0)
            author[0]='\0';
        if (wmupdf_info_field(mupdffilename,"CreationDate",cdate,127)<0)
            cdate[0]='\0';
        if (wmupdf_info_field(mupdffilename,"Title",title,255)<0)
            title[0]='\0';
        }
    else
#endif
        author[0]=title[0]=cdate[0]='\0';
    if (!k2settings->use_crop_boxes)
        {
        if (masterinfo->outline!=NULL)
            {
            if (k2settings->debug)
                wpdfoutline_echo(masterinfo->outline,1,1,stdout);
            pdffile_add_outline(&masterinfo->outfile,masterinfo->outline);
            }
        pdffile_finish(&masterinfo->outfile,title,author,masterinfo->pageinfo.producer,cdate);
        pdffile_close(&masterinfo->outfile);
        }
    else
        {
        /* Re-write PDF file using crop boxes */
#if (WILLUSDEBUGX & 64)
wpdfboxes_echo(&masterinfo->pageinfo.boxes,stdout);
#endif
#ifdef HAVE_MUPDF_LIB
        /* v2.20 bug fix -- need to compensate for document_scale_factor if its not 1.0 */
        wmupdf_scale_source_boxes(&masterinfo->pageinfo,1./k2settings->document_scale_factor);
        wmupdf_remake_pdf(mupdffilename,dstfile,&masterinfo->pageinfo,1,masterinfo->outline,stdout);
#endif
        }
    if (k2settings->show_marked_source)
        {
        pdffile_finish(mpdf,title,author,masterinfo->pageinfo.producer,cdate);
        pdffile_close(mpdf);
        }
    } // cdate, author, title selection
    if (k2settings->debug || k2settings->verbose)
        k2printf("Cleaning up ...\n\n");
    /*
    if (folder)
        k2printf("Processing on " TTEXT_INPUT "folder %s" TTEXT_NORMAL " complete.  Total %d pages.\n\n",filename,masterinfo->published_pages);
    else
        k2printf("Processing on " TTEXT_BOLD2 "file %s" TTEXT_NORMAL " complete.  Total %d pages.\n\n",filename,masterinfo->published_pages);
    */
    size=wfile_size(dstfile);
    k2printf("\n" TTEXT_BOLD "%d pages" TTEXT_NORMAL,masterinfo->published_pages);
    if (masterinfo->wordcount>0)
        k2printf(" (%d words)",masterinfo->wordcount);
    k2printf(" written to " TTEXT_MAGENTA "%s" TTEXT_NORMAL " (%.1f MB).\n\n",
            dstfile,size/1024./1024.);
    if (k2settings->show_marked_source)
        {
        size=wfile_size(markedfile);
        k2printf(TTEXT_BOLD "%d pages" TTEXT_NORMAL " written to " TTEXT_MAGENTA "%s" TTEXT_NORMAL " (%.1f MB).\n\n",pages_done,markedfile,size/1024./1024.);
        }
#ifdef HAVE_OCR_LIB
    if (k2settings->dst_ocr && masterinfo->ocrfilename[0]!='\0' && wfile_status(masterinfo->ocrfilename)==1)
        {
        size=wfile_size(masterinfo->ocrfilename);
        k2printf(TTEXT_BOLD "%d words" TTEXT_NORMAL " written to " TTEXT_MAGENTA "%s" TTEXT_NORMAL " (%.1f MB).\n\n",masterinfo->wordcount,masterinfo->ocrfilename,size/1024./1024.);
        }
#endif
    if (local_tocwrites>0)
        k2printf(TTEXT_BOLD "%d bytes" TTEXT_NORMAL " written to " TTEXT_MAGENTA "%s" TTEXT_NORMAL ".\n\n",(int)(wfile_size(k2settings->tocsavefile)+.5),k2settings->tocsavefile);
    masterinfo_free(masterinfo,k2settings);
    if (folder)
        filelist_free(fl);
    k2out->status=0;
    return(0.);
    }


void wpdfboxes_echo(WPDFBOXES *boxes,FILE *out)

    {
    int i;

    k2printf("Number of boxes = %d\n",boxes->n);
    for (i=0;i<boxes->n;i++)
        {
        WPDFBOX *box;
        WPDFSRCBOX *srcbox;

        box=&boxes->box[i];
        srcbox=&box->srcbox;
        k2printf("Box %d:\n",i);
        k2printf("    Source: (Page %d)\n",srcbox->pageno);
        k2printf("        (%.1f,%.1f) %.1f x %.1f pts\n",
                  srcbox->x0_pts,srcbox->y0_pts,srcbox->crop_width_pts,srcbox->crop_height_pts);
        k2printf("    Dest: (Page %d)\n",box->dstpage);
        k2printf("        whole page = %.1f x %.1f pts\n",box->dst_width_pts,box->dst_height_pts);
        k2printf("        x1=%5.1f, y1=%5.1f\n",box->x1,box->y1);
        k2printf("        Rot=%5.1f\n\n",box->dstrot_deg);
        }
    }


static int k2_handle_preview(K2PDFOPT_SETTINGS *k2settings,MASTERINFO *masterinfo,
                             int k2mark_page_count,WILLUSBITMAP *markedbmp,
                             K2PDFOPT_OUTPUT *k2out)

    {
    int status;

    status= (masterinfo->preview_captured 
                      || (k2settings->show_marked_source
                           && abs(k2settings->preview_page)==k2mark_page_count));
    if (status)
        {
        if (k2settings->show_marked_source)
            bmp_copy(masterinfo->preview_bitmap,markedbmp);
/*
printf("Got preview bitmap:  %d x %d x %d.\n",
masterinfo->preview_bitmap->width,masterinfo->preview_bitmap->height,masterinfo->preview_bitmap->bpp);
*/
        if (k2out->bmp==NULL)
            {
            bmp_write(masterinfo->preview_bitmap,"k2pdfopt_out.png",NULL,100);
            bmp_free(masterinfo->preview_bitmap);
            }
        }
    return(status);
    }


#if (defined(WIN32) || defined(WIN64))
#define fstrcmp stricmp
#else
#define fstrcmp strcmp
#endif
static int filename_comp(char *name1,char *name2)

    {
    char abs1[512],abs2[512];

    /* First do a straight compare */
    if (!fstrcmp(name1,name2))
        return(0);
    /* Convert to absolute path and compare */
    strcpy(abs1,name1);
    wfile_make_absolute(abs1);
    strcpy(abs2,name2);
    wfile_make_absolute(abs2);
    return(fstrcmp(abs1,abs2));
    }
    

static void filename_substitute(char *dst,char *fmt,char *src,int count,char *defext0)

    {
    char *defext;
    int i,j,k;
    char basespec[512];
    char xfmt[128];

    wfile_newext(basespec,src,"");
    defext=(defext0[0]=='.' ? &defext0[1] : defext0);
    for (i=j=0;fmt[i]!='\0';i++)
        {
        if (fmt[i]!='%')
            {
            dst[j++]=fmt[i];
            continue;
            }
        xfmt[0]='%';
        for (k=1;k<120 && (fmt[i+k]=='-' || (fmt[i+k]>='0' && fmt[i+k]<='9'));k++)
            xfmt[k]=fmt[i+k];
        if (fmt[i+k]=='s' || fmt[i+k]=='d')
            {
            int c;
            c=xfmt[k]=fmt[i+k];
            xfmt[k+1]='\0';
            i=i+k;
            dst[j]='\0';
            if (c=='s')
                sprintf(&dst[strlen(dst)],xfmt,basespec);
            else
                sprintf(&dst[strlen(dst)],xfmt,count);
            j=strlen(dst);
            continue;
            }
        dst[j++]=fmt[i];
        }
    dst[j]='\0';
    if (stricmp(wfile_ext(dst),defext))
        {
        strcat(dst,".");
        strcat(dst,defext);
        }
    }


/*
**  0 = ask each time
**  1 = overwrite all
** -1 = no overwriting (all)
*/
void overwrite_set(int status)

    {
    k2files_overwrite=status;
    }


static int overwrite_fail(char *outname,double overwrite_minsize_mb)

    {
    double size_mb;
    char basepath[512];
    char buf[512];
    char newname[512];

    if (wfile_status(outname)==0)
        return(0);
    if (overwrite_minsize_mb < 0.)
        return(0);
    if (k2files_overwrite==1)
        return(0);
    size_mb = wfile_size(outname)/1024./1024.;
    if (size_mb < overwrite_minsize_mb)
        return(0);
    if (k2files_overwrite==-1)
        return(1);
    wfile_basepath(basepath,outname);
    strcpy(newname,outname);
    k2printf("\n\a");
    while (1)
        {
        while (1)
            {
#ifdef HAVE_K2GUI
            if (k2gui_active())
                {
                int reply;
                reply=k2gui_yes_no_all("File overwrite query","File %s (%.1f MB) already exists!  "
                                       "Overwrite it?",newname,size_mb);
                if (reply==2)
                    {
                    overwrite_set(-1);
                    return(1);
                    }
                if (reply==3)
                    overwrite_set(1);
                return(0);
                }
            else
                {
#endif
            k2printf("File " TTEXT_MAGENTA "%s" TTEXT_NORMAL " (%.1f MB) already exists!\n"
                      "   Overwrite it (y[es]/n[o]/a[ll]/q[uit])? " TTEXT_INPUT,
                    newname,size_mb);
            k2gets(buf,16,"y");
            k2printf(TTEXT_NORMAL);
            clean_line(buf);
            buf[0]=tolower(buf[0]);
#ifdef HAVE_K2GUI
                }
#endif
            if (buf[0]!='y' && buf[0]!='n' && buf[0]!='a' && buf[0]!='q')
                {
                k2printf("\a\n  ** Must respond with 'y', 'n', 'a', or 'q' **\n\n");
                continue;
                }
            break;
            }
        if (buf[0]=='q')
            return(-1);
        if (buf[0]=='a' || buf[0]=='y')
            {
            if (buf[0]=='a')
                overwrite_set(1);
            return(0);
            }

        k2printf("Enter a new output base name (.pdf will be appended, q=quit).\n"
                "New name: " TTEXT_INPUT);
        k2gets(buf,255,"__out__.pdf");
        k2printf(TTEXT_NORMAL);
        clean_line(buf);
        if (!stricmp(buf,"q"))
            return(-1);
        if (buf[0]=='/' || buf[0]=='\\' || buf[1]==':')
            strcpy(newname,buf);
        else
            wfile_fullname(newname,basepath,buf);
        if (!strcmp(wfile_ext(newname),""))
            strcat(newname,".pdf");
        if (wfile_status(newname)==0)
            break;
        }
    strcpy(outname,newname);
    return(0);
    }


static int toclist_valid(char *s,FILE *out)

    {
    if (s[0]=='\0')
        return(0);
    if (pagelist_valid_page_range(s))
        return(1);
    if (wfile_status(s)==1)
        return(1);
    if (out!=NULL)
        k2printf(ANSI_RED "\nTOC page list '%s' is not valid page range or file name."
                 ANSI_NORMAL "\n\n",s);
    return(0);
    }


/*
** Create outline from page list
*/
static WPDFOUTLINE *wpdfoutline_from_pagelist(char *pagelist,int maxpages)

    {
    int i;
    WPDFOUTLINE *outline,*outline0;

    outline0=outline=NULL;
    for (i=0;1;i++)
        {
        int page;
        char buf[64];
        WPDFOUTLINE *oline;

        page=pagelist_page_by_index(pagelist,i,maxpages);
        if (page<0)
            break;
        sprintf(buf,"Chapter %d",i+1);
        oline=malloc(sizeof(WPDFOUTLINE));
        wpdfoutline_init(oline);
        oline->title=malloc(strlen(buf)+1);
        strcpy(oline->title,buf);
        oline->srcpage=page-1;
        oline->dstpage=-1;
        if (i==0)
            {
            outline0=outline=oline;
            continue;
            }
        outline->next=oline;
        outline=outline->next;
        }
    return(outline0);
    }
