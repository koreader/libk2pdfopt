/*
** k2file.c      K2pdfopt file handling and main file processing
**               function (k2pdfopt_proc_one()).
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
*/

#include "k2pdfopt.h"

static int k2files_overwrite=0;

static void k2pdfopt_proc_file_or_folder(K2PDFOPT_SETTINGS *k2settings,char *arg,
                                         K2PDFOPT_FILELIST_PROCESS *k2listproc);
static void k2pdfopt_warn_file_not_found(K2PDFOPT_SETTINGS *k2settings,char *filename,
                                         K2PDFOPT_FILELIST_PROCESS *k2listproc);
static void k2pdfopt_preprocess_single_doc(K2PDFOPT_SETTINGS *k2settings,char *filename,
                                           K2PDFOPT_FILELIST_PROCESS *k2listproc);
static void k2pdfopt_echo_file_info(K2PDFOPT_SETTINGS *k2settings,char *filename);
static int k2pdfopt_proc_one(K2PDFOPT_SETTINGS *k2settings,char *filename,
                             K2PDFOPT_FILE_PROCESS *k2fileproc);
static int k2_handle_preview(K2PDFOPT_SETTINGS *k2settings,MASTERINFO *masterinfo,
                             int k2mark_page_count,WILLUSBITMAP *markedbmp,
                             K2PDFOPT_FILE_PROCESS *k2fileproc);
static char *pagename(int pageno);
static int  filename_comp(char *name1,char *name2);
static int  filename_get_temp_pdf_name(char *dst,char *fmt,char *psname);
static int  count_format_strings(char *fmt,int type);
static int  overwrite_fail(char *outname,double overwrite_minsize_mb,int assume_yes);
static int toclist_valid(char *s,FILE *out);
static WPDFOUTLINE *wpdfoutline_from_pagelist(char *pagelist,int maxpages);
static int tocwrites=0;
static int get_source_type(char *filename);
static int file_numpages(char *filename,char *mupdffilename,int src_type,int *usegs);
#ifdef HAVE_GHOSTSCRIPT
static int  gsproc_init(void);
static int  gs_convert_to_pdf(char *temppdfname,char *psfilename,K2PDFOPT_SETTINGS *k2settings);
static void gs_postprocess(char *filename);
#endif
static void gs_conv_cleanup(int psconv,char *filename,char *original_file);
static void k2pdfopt_file_process_init(K2PDFOPT_FILE_PROCESS *k2fileproc);
static void k2pdfopt_file_process_close(K2PDFOPT_FILE_PROCESS *k2fileproc);
static int  k2pdfopt_get_cover_image(WILLUSBITMAP *src,K2PDFOPT_SETTINGS *k2settings,
                                     char *filename,int dpi,int *errcnt,int *pixwarn);
static int  k2pdfopt_get_file_image(WILLUSBITMAP *src,K2PDFOPT_SETTINGS *k2settings,
                                    int src_type,char *filename,int pageno,
                                    int dpi,int *errcnt,int *pixwarn);
static int  k2file_get_bitmap_file_list(FILELIST *fl,char *filename,int first_time_through);
static int k2file_setup_output_file_names(K2PDFOPT_SETTINGS *k2settings,char *filename,
                                          K2PDFOPT_FILE_PROCESS *k2fileproc,
                                          MASTERINFO *masterinfo,
                                          char *dstfile,char *markedfile,PDFFILE *mpdf);


/*
** If arg is a file wildcard specification, then figure out all matches and pass
** each match, one by one, to k2_proc_arg.
*/
void k2pdfopt_proc_wildarg(K2PDFOPT_SETTINGS *k2settings,char *arg,
                           K2PDFOPT_FILELIST_PROCESS *k2listproc)

    {
    int i;

#if (WILLUSDEBUGX & 1)
printf("@k2pdfopt_proc_wildarg(%s)\n",arg);
#endif
    /* Init width to -1 */
    if (k2settings->preview_page!=0 && k2listproc->bmp!=NULL)
        k2listproc->bmp->width = -1;
    /* Converting first file in command line?  If so, check/warn about settings. */
    if (k2listproc->filecount==0 
           && k2listproc->mode==K2PDFOPT_FILELIST_PROCESS_MODE_CONVERT_FILES)
        k2settings_check_and_warn(k2settings);
    if (wfile_status(arg)==0)
        {
        FILELIST *fl,_fl;

        fl=&_fl;
        filelist_init(fl);
        filelist_fill_from_disk_1(fl,arg,0,0);
        if (fl->n==0)
            {
            k2pdfopt_warn_file_not_found(k2settings,arg,k2listproc);
            return;
            }
        for (i=0;i<fl->n;i++)
            {
            char fullname[512];
            wfile_fullname(fullname,fl->dir,fl->entry[i].name);
            k2pdfopt_proc_file_or_folder(k2settings,fullname,k2listproc);
            }
        filelist_free(fl);
        }
    else
        k2pdfopt_proc_file_or_folder(k2settings,arg,k2listproc);
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
static void k2pdfopt_proc_file_or_folder(K2PDFOPT_SETTINGS *k2settings,char *filename,
                                         K2PDFOPT_FILELIST_PROCESS *k2listproc)

    {
    int i,npdf,nbmp;
    FILELIST *pdflist,_pdflist;

#if (WILLUSDEBUGX & 1)
printf("@k2pdfopt_proc_file_or_folder(%s)\n",filename);
printf("wfile_status(%s) = %d\n",filename,wfile_status(filename));
#endif
    if (wfile_status(filename)==0)
        {
        k2pdfopt_warn_file_not_found(k2settings,filename,k2listproc);
        return;
        }
    /* If folder, count PDF/DJVU files and bitmap files */
    if (wfile_status(filename)==2)
        {
        static char *eolist[]={""};
        static char *pdfextlist[]={"*.pdf","*.djvu","*.djv","*.ps","*.eps",""};
        static char *bmpextlist[]={"*.png","*.jpg",""};
        FILELIST *bmplist,_bmplist;

        pdflist=&_pdflist;
        filelist_init(pdflist);
        filelist_fill_from_disk(pdflist,filename,pdfextlist,eolist,0,0);
        npdf=pdflist->n;
        bmplist=&_bmplist;
        filelist_init(bmplist);
        filelist_fill_from_disk(bmplist,filename,bmpextlist,eolist,0,0);
        nbmp=bmplist->n;
        filelist_free(bmplist);
        }
    else
        {
        pdflist=NULL;
        npdf=0;
        nbmp=0;
        }
    for (i=0;i<=npdf;i++)
        {
        char fullname[512];

        if (i==npdf && npdf>0 && nbmp==0)
            break;
        if (i<npdf)
            wfile_fullname(fullname,filename,pdflist->entry[i].name);
        else
            strcpy(fullname,filename);
        k2pdfopt_preprocess_single_doc(k2settings,fullname,k2listproc);
        }
    if (pdflist!=NULL)
        filelist_free(pdflist);
    }


static void k2pdfopt_warn_file_not_found(K2PDFOPT_SETTINGS *k2settings,char *filename,
                                         K2PDFOPT_FILELIST_PROCESS *k2listproc)

    {
#ifdef HAVE_K2GUI
    if ((!k2gui_active() && k2listproc->mode==K2PDFOPT_FILELIST_PROCESS_MODE_CONVERT_FILES)
         || (k2gui_active() && k2listproc->mode!=K2PDFOPT_FILELIST_PROCESS_MODE_CONVERT_FILES))
#endif
    k2printf(TTEXT_WARN "\n** File or folder %s could not be opened. **\n\n" TTEXT_NORMAL,filename);
#ifdef HAVE_K2GUI
    if (k2listproc->mode==K2PDFOPT_FILELIST_PROCESS_MODE_CONVERT_FILES && k2gui_active())
        {
        char buf[512];
        k2gui_cbox_increment_error_count();
        sprintf(buf,"File %s cannot be opened.",filename);
        if (k2settings->preview_page>0)
            k2gui_alertbox(0,"File not found",buf);
        else
            k2gui_cbox_set_pages_completed(0,buf);
        }
#endif
    }


/*
**
** "filename" is either a PDF/DJVU file or a folder of bitmap files.  It is meant
** to be converted to a single PDF document.
**
** This function determines any pre-processing that needs to be done (rotation
** checking, font size checking, etc.).
**
** Processing file is two steps:
**     1. If auto-rotation is requested, determine the proper rotation of the file.
**     2. Process the file with the determined rotation.
**
*/
static void k2pdfopt_preprocess_single_doc(K2PDFOPT_SETTINGS *k2settings,char *srcfilename,
                                           K2PDFOPT_FILELIST_PROCESS *k2listproc)

    {
    K2PDFOPT_FILE_PROCESS _k2fileproc,*k2fileproc;
    int autorot,status,src_type,psconv;
    char original_file[MAXFILENAMELEN];
    char filename[MAXFILENAMELEN];

    strncpy(filename,srcfilename,MAXFILENAMELEN-1);
    filename[MAXFILENAMELEN-1]='\0';
    k2fileproc=&_k2fileproc;
    k2pdfopt_file_process_init(k2fileproc);
    k2fileproc->filecount=k2listproc->filecount+1;
    k2fileproc->callcount=0;
    k2fileproc->bmp = k2listproc->bmp;
#if (WILLUSDEBUGX & 0x1000000)
printf("@k2pdfopt_preprocess_single_doc(%s)\n",filename);
printf("    wfile_status(%s) = %d\n",filename,wfile_status(filename));
printf("    mode=%d\n",k2listproc->mode);
printf("    info=%d\n",k2settings->info);
printf("    preview=%d\n",k2settings->preview_page);
printf("    src_rot=%d\n",k2settings->src_rot);
#endif
    if (wfile_status(filename)==0)
        {
        k2pdfopt_warn_file_not_found(k2settings,filename,k2listproc);
        k2pdfopt_file_process_close(k2fileproc);
        return;
        }
    /* File is legit and counts towards total (if counting) */
    if (k2listproc->mode==K2PDFOPT_FILELIST_PROCESS_MODE_GET_FILECOUNT)
        {
        k2listproc->filecount++;
        k2pdfopt_file_process_close(k2fileproc);
        return;
        }
    if (k2settings->info) /* Info only? */
        {
        k2pdfopt_echo_file_info(k2settings,filename);
        k2pdfopt_file_process_close(k2fileproc);
        return;
        }

    /* If Postscript file, convert it to PDF */
    src_type = get_source_type(filename);
    psconv=0;
    if (src_type==SRC_TYPE_PS)
#ifdef HAVE_GHOSTSCRIPT
        {
        int status;
        char tempname[MAXFILENAMELEN];
        status=gs_convert_to_pdf(tempname,filename,k2settings);
        if (!status)
            return;
        strncpy(original_file,filename,MAXFILENAMELEN-1);
        original_file[MAXFILENAMELEN-1]='\0';
        strcpy(filename,tempname);
        psconv=1;
        }
#else
        {
        k2printf(TTEXT_WARN
                "\a\n\n** Ghostscript support not compiled into this version of k2pdfopt. **\n\n"
                      "** Cannot process file %s. **\n\n" TTEXT_NORMAL,filename);
        k2fileproc->status=3;
        return;
        }
#endif
    
    /* Determine if we need to check the orientation of the file */      
    if (k2settings->preview_page!=0)
        autorot = (fabs(k2settings->src_rot - SRCROT_AUTOPREV)<.5);
    else
        autorot = (fabs(k2settings->src_rot - SRCROT_AUTOPREV)<.5
                      || fabs(k2settings->src_rot - SRCROT_AUTO)<.5
                      || fabs(k2settings->src_rot - SRCROT_AUTOEP)<.5);
#if (WILLUSDEBUGX & 0x1000000)
printf("    autorot=%d\n",autorot);
#endif

    /* If folder, first process all PDF/DJVU/PS files in the folder */
    if (autorot)
        {
        k2fileproc->mode=K2PDFOPT_FILE_PROCESS_MODE_GET_ROTATION;
        k2fileproc->rotation_deg = SRCROT_AUTO;
#if (WILLUSDEBUGX & 0x1000000)
printf("    k2fileproc->rotation_deg=%d\n",(int)k2fileproc->rotation_deg);
#endif
        status=k2pdfopt_proc_one(k2settings,filename,k2fileproc);
        if (status!=0)
            {
#ifdef HAVE_K2GUI
            if (k2gui_active())
                k2gui_cbox_error(filename,status);
#endif
            gs_conv_cleanup(psconv,filename,original_file);
            k2pdfopt_file_process_close(k2fileproc);
            return;
            }
        }
    else
        k2fileproc->rotation_deg = k2settings->src_rot < -990. ? 0. : k2settings->src_rot;

    /* Check for font size if needed -- set k2fileproc->fontsize_pts either way */
    if (k2settings->dst_fontsize_pts>0.)
        {
        k2fileproc->mode=K2PDFOPT_FILE_PROCESS_MODE_GET_FONTSIZE;
        status=k2pdfopt_proc_one(k2settings,filename,k2fileproc);
        if (status!=0)
            {
#ifdef HAVE_K2GUI
            if (k2gui_active())
                k2gui_cbox_error(filename,status);
#endif
            gs_conv_cleanup(psconv,filename,original_file);
            k2pdfopt_file_process_close(k2fileproc);
            return;
            }
        }

    /* Convert file to new PDF */
    k2fileproc->mode=K2PDFOPT_FILE_PROCESS_MODE_CONVERT_FILE;
    status=k2pdfopt_proc_one(k2settings,filename,k2fileproc);
    if (status==0)
        {
        k2listproc->filecount++;
        /* Pass converted name back to k2listproc structure */
        if (k2fileproc->outname!=NULL)
            {
            static char *funcname="k2pdfopt_preprocess_single_doc";

            k2listproc->outname=NULL;
            /* Return output file name in k2fileproc for GUI */
            willus_mem_alloc((double **)&k2listproc->outname,(long)(strlen(k2fileproc->outname)+1),funcname);
            if (k2listproc->outname!=NULL)
                strcpy(k2listproc->outname,k2fileproc->outname);
            }
        }
#ifdef HAVE_K2GUI
    if (k2gui_active())
        {
        if (status!=0)
            k2gui_cbox_error(filename,status);
        else
            k2gui_cbox_set_files_completed(k2listproc->filecount,NULL);
        }
#endif
    gs_conv_cleanup(psconv,filename,original_file);
    k2pdfopt_file_process_close(k2fileproc);
    }


static void k2pdfopt_echo_file_info(K2PDFOPT_SETTINGS *k2settings,char *filename)

    {
#ifdef HAVE_MUPDF_LIB
    char *buf;
    int *pagelist;

    pagelist=NULL;
    pagelist_get_array(&pagelist,k2settings->pagelist);
/*
{
int i;
for (i=0;pagelist!=NULL&&pagelist[i]>=0;i++)
printf("pagelist[%d]=%d\n",i,pagelist[i]);
printf("pagelist[%d]=%d\n",i,pagelist[i]);
}
*/
    buf=NULL;
    wmupdfinfo_get(filename,pagelist,&buf);
    printf("%s",buf);
    if (buf!=NULL)
        free(buf);
    if (pagelist!=NULL)
        free(pagelist);
#else
    printf("FILE: %s\n",filename);
    printf("Cannot print file info.  MuPDF not compiled into application.\n");
#endif
    }


/*
** k2pdfopt_proc_one() is the main source file processing function in k2pdfopt.
** 
** Depending on the value of k2fileproc->mode, it either determines the correct rotation of
** the passed file, determines the font size, or it processes it and converts it.
**
** Some other special processing flags in k2settings0:
**     k2settings0->echo_source_page_count --> Just echo the source page count.
**     k2settings0->preview_page --> Generate a preview page bitmap
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
static int k2pdfopt_proc_one(K2PDFOPT_SETTINGS *k2settings0,char *filename,
                             K2PDFOPT_FILE_PROCESS *k2fileproc)

    {
    static K2PDFOPT_SETTINGS _k2settings,*k2settings;
    static MASTERINFO _masterinfo,*masterinfo;
    static PDFFILE _mpdf,*mpdf;
    char dstfile[MAXFILENAMELEN];
    char markedfile[MAXFILENAMELEN];
    char rotstr[128];
    WILLUSBITMAP _src,*src;
    WILLUSBITMAP _srcgrey,*srcgrey;
    WILLUSBITMAP _marked,*marked;
    WILLUSBITMAP preview_internal;
    int i,status,pw,np,src_type,first_time_through,or_detect,fontsize_detect,preview;
    int pagecount,pagestep,pages_done,local_tocwrites;
    int errcnt,pixwarn;
    FILELIST *fl,_fl;
    int dpi;
    double rot_deg,size,bormean;
    char *mupdffilename;
    extern int k2mark_page_count;
/*
    static char *funcname="k2pdfopt_proc_one";
    static char *readerr=TTEXT_WARN "\a\n ** ERROR reading page %d from " TTEXT_BOLD2 "%s" TTEXT_WARN ".\n\n" TTEXT_NORMAL;
    static char *readlimit=TTEXT_WARN "\a\n ** (No more read errors will be echoed for file %s.)\n\n" TTEXT_NORMAL;
*/
/*
extern void willus_mem_debug_update(char *);
*/

#if (WILLUSDEBUGX & 0x1000001)
printf("@k2pdfopt_proc_one(%s)\n",filename);
#endif
#if (WILLUSDEBUGX & 0x1000000)
printf("    wfile_status(%s) = %d\n",filename,wfile_status(filename));
printf("    mode=%d\n",k2fileproc->mode);
printf("    callcount=%d\n",k2fileproc->callcount);
printf("    filecount=%d\n",k2fileproc->filecount);
#endif
    /* Default rotation */
    local_tocwrites=0;
    k2settings=&_k2settings;
    k2pdfopt_settings_copy(k2settings,k2settings0);
#ifdef HAVE_K2GUI
    if (k2gui_active())
        k2gui_cbox_set_filename(filename);
#endif
    mpdf=&_mpdf;
    /* Must be called once per conversion to init margins / devsize / output size */
    k2pdfopt_settings_new_source_document_init(k2settings);
    errcnt=0;
    pixwarn=0;
    mupdffilename=_masterinfo.srcfilename;
    strncpy(mupdffilename,filename,MAXFILENAMELEN-1);
    mupdffilename[MAXFILENAMELEN-1]='\0';
    or_detect=(k2fileproc->mode==K2PDFOPT_FILE_PROCESS_MODE_GET_ROTATION);
    rot_deg=k2fileproc->rotation_deg;
    if (or_detect)
        k2fileproc->rotation_deg=0.; /* Default */
    fontsize_detect=(k2fileproc->mode==K2PDFOPT_FILE_PROCESS_MODE_GET_FONTSIZE);
    k2fileproc->callcount++;
    first_time_through = (k2fileproc->callcount==1);
#if (WILLUSDEBUGX & 0x1000000)
printf("or_detect = %d\n",or_detect);
printf("rot_deg = %g\n",rot_deg);
printf("fontsize_detect = %d\n",fontsize_detect);
printf("callcount = %d\n",k2fileproc->callcount);
printf("first_time_through = %d\n",first_time_through);
#endif
    /* Don't care about rotation if just echoing page count */
    if (k2settings->echo_source_page_count && (or_detect || fontsize_detect))
        {
        k2fileproc->status=0;
        return(k2fileproc->status);
        }
    if (or_detect && k2settings->src_dpi>300)
        dpi=300;
    else
        dpi=k2settings->src_dpi;
    src_type = get_source_type(filename);
    /*
    if (folder && first_time_through)
        k2printf("Processing " TTEXT_INPUT "BITMAP FOLDER %s" TTEXT_NORMAL "...\n",
               filename);
    */
    /*
    else
        k2printf("Processing " TTEXT_BOLD2 "PDF FILE %s" TTEXT_NORMAL "...\n",
               filename);
    */
    fl=&_fl;
    if (src_type == SRC_TYPE_BITMAPFOLDER &&
         !k2file_get_bitmap_file_list(fl,filename,first_time_through))
        {
        k2fileproc->status=2;
        return(k2fileproc->status);
        }
    src=&_src;
    srcgrey=&_srcgrey;
    marked=&_marked;
    bmp_init(src);
    bmp_init(srcgrey);
    bmp_init(marked);
    pw=0;
    if (src_type==SRC_TYPE_PS)
        {
        k2printf(TTEXT_WARN
                    "\a\n\n** Internal error.  Should not be seeing type postscript here! **\n\n"
                          "** Contact author.  Cannot process file %s. **\n\n" TTEXT_NORMAL,filename);
        k2fileproc->status=3;
        return(k2fileproc->status);
        }
#ifndef HAVE_DJVU_LIB
    if (src_type==SRC_TYPE_DJVU)
        {
        if (!or_detect)
            k2printf(TTEXT_WARN
                    "\a\n\n** DjVuLibre not compiled into this version of k2pdfopt. **\n\n"
                          "** Cannot process file %s. **\n\n" TTEXT_NORMAL,filename);
        k2fileproc->status=3;
        return(k2fileproc->status);
        }
#endif
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
    masterinfo->filecount=k2fileproc->filecount;
    if (k2settings->preview_page!=0 && !or_detect && !fontsize_detect)
        {
        preview=1;
        if (k2fileproc->bmp!=NULL)
            masterinfo->preview_bitmap=k2fileproc->bmp;
        else
            {
            masterinfo->preview_bitmap=&preview_internal;
            bmp_init(masterinfo->preview_bitmap);
            }
        }
    else
        preview=0;
    if (!or_detect && !preview && !fontsize_detect
          && !k2file_setup_output_file_names(k2settings,filename,k2fileproc,masterinfo,
                                             dstfile,markedfile,mpdf))
        {
        if (src_type == SRC_TYPE_BITMAPFOLDER)
            filelist_free(fl);
        return(k2fileproc->status);
        }
    if (src_type==SRC_TYPE_PDF || src_type==SRC_TYPE_DJVU)
        {
        np=file_numpages(filename,mupdffilename,src_type,&k2settings->usegs);
#ifdef HAVE_MUPDF_LIB
        if (src_type==SRC_TYPE_PDF)
            {
            /* Get bookmarks / outline from PDF file */
            if (!or_detect && !fontsize_detect && k2settings->use_toc!=0 && !toclist_valid(k2settings->toclist,NULL))
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
        if (src_type==SRC_TYPE_BITMAPFOLDER)
            filelist_free(fl);
        return(0.);
        }
    masterinfo->srcpages = np;
    if (!or_detect && !fontsize_detect && toclist_valid(k2settings->toclist,stdout))
        {
        if (pagelist_valid_page_range(k2settings->toclist))
            masterinfo->outline=wpdfoutline_from_pagelist(k2settings->toclist,masterinfo->srcpages);
        else
            masterinfo->outline=wpdfoutline_read_from_text_file(k2settings->toclist);
        }
    pagecount = np<0 ? -1 : double_pagelist_count(k2settings->pagelist,k2settings->pagexlist,np);
#ifdef HAVE_K2GUI
    if (k2gui_active())
        {
        k2gui_cbox_set_num_pages(pagecount<0 ? 1 : pagecount);
        k2gui_cbox_set_pages_completed(0,NULL);
        }
#endif
    if (pagecount<0 || (!or_detect && !fontsize_detect))
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
        if (first_time_through)
            k2printf("\a\n" TTEXT_WARN "No %ss to convert (-p %s -px %s)!" TTEXT_NORMAL "\n\n",
                     src_type==SRC_TYPE_BITMAPFOLDER?"file":"page",
                     k2settings->pagelist,k2settings->pagexlist);
        masterinfo_free(masterinfo,k2settings);
        if (src_type==SRC_TYPE_BITMAPFOLDER)
            filelist_free(fl);
        k2fileproc->status=5;
        return(k2fileproc->status);
        }
    if (first_time_through)
        {
        if (!k2settings->preview_page)
        {
        k2printf("Reading ");
        if (pagecount>0)
           {
           if (pagecount<np)
               k2printf("%d out of %d %s%s",pagecount,np,
                        src_type==SRC_TYPE_BITMAPFOLDER?"file":"page",np>1?"s":"");
           else
               k2printf("%d %s%s",np,src_type==SRC_TYPE_BITMAPFOLDER?"file":"page",np>1?"s":"");
           }
        else
           k2printf("%ss",src_type==SRC_TYPE_BITMAPFOLDER?"file":"page");
        k2printf(" from " TTEXT_BOLD2 "%s" TTEXT_NORMAL " ...\n",filename);
        }
        }
    if (or_detect)
        {
        if (!k2settings->preview_page)
            k2printf("\nDetecting document orientation ... ");
        }
    else if (fontsize_detect)
        {
        if (!k2settings->preview_page)
            {
            k2printf("\nDetecting document median font size ... ");
            if (k2settings->verbose)
                k2printf("\n");
            }
        }
    bormean=1.0;
/*
printf("np=%d, src_type=%d\n",np,src_type);
*/
    /*
    ** LOOP THROUGH SOURCE DOCUMENT PAGES
    */
    for (i=-1;1;i+=pagestep)
        {
        char bmpfile[MAXFILENAMELEN];
        int pageno,nextpage;
/*
sprintf(bmpfile,"i=%d",i);
willus_mem_debug_update(bmpfile);
*/
        pageno=0;
        if (pagecount>0 && i+1>pagecount)
            break;
        nextpage = (i+2>pagecount) ? -1 : double_pagelist_page_by_index(k2settings->pagelist,
                                                             k2settings->pagexlist,i+1,np);
        if (i<0)
            {
            if (k2settings->dst_coverimage[0]=='\0')
                continue;
            status=k2pdfopt_get_cover_image(src,k2settings,mupdffilename,dpi,&errcnt,&pixwarn);
            if (!status)
                continue;
            if (k2settings->use_crop_boxes)
                {
                bmp_copy(&masterinfo->cover_image,src);
                continue;
                }
            pageno=-1;
            }
        else
            {
            pageno = double_pagelist_page_by_index(k2settings->pagelist,k2settings->pagexlist,i,np);
            if (pageno<0)
                break;
            /* Removed in v2.32 */
            /* This always returned non-zero */
            /*
            if (!pagelist_page_by_index(k2settings->pagelist,pageno,np))
                continue;
            */
            if (src_type==SRC_TYPE_BITMAPFOLDER)
                {
                if (pageno-1>=fl->n)
                    continue;
                wfile_fullname(bmpfile,fl->dir,fl->entry[pageno-1].name);
                status=bmp_read(src,bmpfile,stdout);
                if (status<0)
                    {
                    if (first_time_through)
                        k2printf(TTEXT_WARN "\n\aCould not read file %s.\n" TTEXT_NORMAL,bmpfile);
                    continue;
                    }
                }
            else
                { 
                /* If not a PDF/DJVU file, only read it once. */
                if (i>0 && src_type!=SRC_TYPE_PDF && src_type!=SRC_TYPE_DJVU)
                    break;
                status=k2pdfopt_get_file_image(src,k2settings,src_type,mupdffilename,
                                               pageno,dpi,&errcnt,&pixwarn);
                if (status<0)
                    break;
                if (status==0)
                    continue;
                }
            } /* closing brace for "else" from checking for cover page */
        k2mark_page_count = i+1;

        {
        BMPREGION region;
        int mstatus;

        /* Got Good Page Render */
        bmpregion_init(&region);
        bmpregion_k2pagebreakmarks_allocate(&region);
        mstatus=masterinfo_new_source_page_init(masterinfo,k2settings,src,srcgrey,marked,
                                 &region,rot_deg,&bormean,rotstr,pageno,nextpage,stdout);
        if (mstatus==0 || fontsize_detect)
            {
            if (fontsize_detect)
                {
                int si;
                si=k2fileproc->fsh.n;
                k2proc_get_fontsize_histogram(&region,masterinfo,k2settings,&k2fileproc->fsh);
                if (k2settings->verbose)
                    k2printf("    %d text rows on %s.\n",k2fileproc->fsh.n-si,pagename(pageno));
                }
            /* v2.15 -- memory leak fix */
            bmpregion_free(&region);
            pages_done++;
            continue;
            }

        /* If user has set output size by font size, determine source font size (v2.34) */
        {
        double src_fontsize_pts;

        if (k2settings->dst_fontsize_pts>0.)
            src_fontsize_pts = fontsize_histogram_median(&k2fileproc->fsh,0);
        else if (k2settings->dst_fontsize_pts<0.)
            {
            int si;

            si=k2fileproc->fsh.n;
            k2proc_get_fontsize_histogram(&region,masterinfo,k2settings,&k2fileproc->fsh);
            src_fontsize_pts = fontsize_histogram_median(&k2fileproc->fsh,si);
            if (k2settings->verbose)
                k2printf("    %d text rows on page %s\n",k2fileproc->fsh.n-si,pagename(pageno));
            }
        else
            src_fontsize_pts=-1.;

        /* v2.34:  Set destination size (flush output bitmap if it changes) */
        k2pdfopt_settings_set_margins_and_devsize(k2settings,&region,masterinfo,src_fontsize_pts,0);
        if (!k2settings->preview_page)
            {
            if (pageno<0)
                k2printf("\n" TTEXT_HEADER "COVER PAGE (%s)",k2settings->dst_coverimage);
            else
                k2printf("\n" TTEXT_HEADER "SOURCE PAGE %d",pageno);
            }
        if (pagecount>0)
            {
            if (!k2settings->preview_page && pageno>=0)
                {
                if (k2settings->pagelist[0]!='\0')
                    k2printf(" (%d of %d)",pages_done+1,pagecount);
                else
                    k2printf(" of %d",pagecount);
                }
            }
        if (!k2settings->preview_page)
            {
            k2printf(TTEXT_NORMAL 
                " (%.1f x %.1f in",(double)srcgrey->width/k2settings->src_dpi,
                                   (double)srcgrey->height/k2settings->src_dpi);
            if (k2settings->dst_fontsize_pts<0.)
                {
                if (src_fontsize_pts<0)
                    k2printf(", fs=undet.");
                else
                    k2printf(", fs=%.1fpts",src_fontsize_pts);
                }
            k2printf(") ... %s",rotstr);
            fflush(stdout);
            }
        } /* End of scope with src_fontsize_pts */

        /* Parse the source bitmap for viewable regions */
        /* v2.34:  If cover page, use special function */
        if (pageno<0)
            bmpregion_add_cover_image(&region,k2settings,masterinfo);
        else
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
        /* Reset the display order for this source page (v2.34--don't call if on cover image) */
        if (k2settings->show_marked_source && pageno>=0)
            mark_source_page(k2settings,masterinfo,NULL,0,0xf);
        /*
        ** v2.10 Call masterinfo_publish() no matter what.  If we've just kicked out a
        **       page, it doesn't matter.  It will do nothing.
        ** v2.34 Flush output if cover image
        */
        {
        int flush_output;
        if (pageno<0)
            flush_output=1;
        else
            flush_output=masterinfo_should_flush(masterinfo,k2settings);
        masterinfo_publish(masterinfo,k2settings,flush_output);
        }
        if (preview && k2_handle_preview(k2settings,masterinfo,k2mark_page_count,
                                         k2settings->dst_color?marked:src,k2fileproc))
            {
            bmp_free(marked);
            bmp_free(srcgrey);
            bmp_free(src);
            masterinfo_free(masterinfo,k2settings);
            if (src_type==SRC_TYPE_BITMAPFOLDER)
                filelist_free(fl);
            k2fileproc->status=0;
            return(k2fileproc->status);
            }
        /* v2.34--only if not cover image */
        if (k2settings->show_marked_source && pageno>=0 && !preview)
            publish_marked_page(mpdf,k2settings->dst_color ? marked : src,k2settings->src_dpi,
                                filename,k2settings->dst_opname_format,
                                k2fileproc->filecount,pages_done,k2settings->jpeg_quality);
        if (!k2settings->preview_page)
            {
            int np;
            np=masterinfo->published_pages-pw;
            if (!k2settings_output_is_bitmap(k2settings))
                k2printf("%d new page%s saved.\n",np,np==1?"":"s");
            }
        pw=masterinfo->published_pages;
        }
    /*
    **
    ** END MAIN SOURCE DOCUMENT PAGE PROCESSING LOOP
    **
    */
/*
willus_mem_debug_update("End");
*/
    /* Didn't find the preview page yet--push out final page. */
    if (preview)
        {
        masterinfo_flush(masterinfo,k2settings);
        if (!k2_handle_preview(k2settings,masterinfo,k2mark_page_count,
                               k2settings->dst_color?marked:src,k2fileproc))
            {
            /* No preview bitmap--return zero-width bitmap */
            if (k2fileproc->bmp==NULL)
                bmp_free(masterinfo->preview_bitmap);
            else
                k2fileproc->bmp->width=0;
            }
        bmp_free(marked);
        bmp_free(srcgrey);
        bmp_free(src);
        masterinfo_free(masterinfo,k2settings);
        if (src_type==SRC_TYPE_BITMAPFOLDER)
            filelist_free(fl);
        k2fileproc->status=0;
        return(k2fileproc->status);
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
                if (!k2settings->preview_page)
                    k2printf("Rotating clockwise.\n");
                masterinfo_free(masterinfo,k2settings);
                if (src_type==SRC_TYPE_BITMAPFOLDER)
                    filelist_free(fl);
                k2fileproc->status=0;
                k2fileproc->rotation_deg=270.;
                return(k2fileproc->status);
                }
            }
        if (!k2settings->preview_page)
            k2printf("No rotation necessary.\n");
        masterinfo_free(masterinfo,k2settings);
        if (src_type==SRC_TYPE_BITMAPFOLDER)
            filelist_free(fl);
        k2fileproc->status=0;
        return(k2fileproc->status);
        }
    if (fontsize_detect)
        {
        double mfs;

        mfs=fontsize_histogram_median(&k2fileproc->fsh,0);
        if (!k2settings->preview_page)
            {
            if (k2settings->verbose)
                {
                if (mfs<=0.)
                    k2printf("    Median document font size could not be determined.\n");
                else
                    k2printf("    Median document font size = %.1f pts.\n",mfs);
                }
            else
                {
                if (mfs<=0.)
                    k2printf("undetermined.\n");
                else
                    k2printf("%.1f pts.\n",mfs);
                }
            }
        k2fileproc->status=0;
        return(k2fileproc->status);
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
    if (!k2settings_output_is_bitmap(k2settings))
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
        if (k2settings->dst_author[0]!='\0')
            strcpy(author,k2settings->dst_author);
        /* v2.35--title can have file name */
        if (k2settings->dst_title[0]!='\0')
            filename_substitute(title,k2settings->dst_title,filename,k2fileproc->filecount,1,"pdf");
    /*
            strcpy(title,k2settings->dst_title);
    */
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
#ifdef HAVE_MUPDF_LIB
            if (masterinfo->pageinfo.boxes.n>0)
                {
                /* Native PDF output:  Re-write PDF file using crop boxes */
#if (WILLUSDEBUGX & 64)
wpdfboxes_echo(&masterinfo->pageinfo.boxes,stdout);
#endif
#if (WILLUSDEBUGX & 64)
printf("Calling wpdfpageinfo_scale_source_boxes()...\n");
#endif
                if (k2settings->dst_author[0]!='\0')
                    strcpy(masterinfo->pageinfo.author,k2settings->dst_author);
                if (k2settings->dst_title[0]!='\0')
                    strcpy(masterinfo->pageinfo.title,title);
                /* v2.20 bug fix -- need to compensate for document_scale_factor if its not 1.0 */
                wpdfpageinfo_scale_source_boxes(&masterinfo->pageinfo,1./k2settings->document_scale_factor);
#if (WILLUSDEBUGX & 64)
printf("Calling wmupdf_remake_pdf()...\n");
#endif
                wmupdf_remake_pdf(mupdffilename,dstfile,&masterinfo->pageinfo,1,masterinfo->outline,
                                  masterinfo->cover_image.width==0?NULL:&masterinfo->cover_image,stdout);
                }
            else
                k2printf(TTEXT_WARN "\nNo PDF output for file %s." TTEXT_NORMAL "\n",dstfile);
#endif
            }
        if (k2settings->show_marked_source)
            {
            pdffile_finish(mpdf,title,author,masterinfo->pageinfo.producer,cdate);
            pdffile_close(mpdf);
            }
    /*
    if (k2settings->debug || k2settings->verbose)
        k2printf("Cleaning up ...\n\n");
    if (src_type==SRC_TYPE_BITMAPFOLDER)
        k2printf("Processing on " TTEXT_INPUT "src_type==SRC_TYPE_BITMAPFOLDER %s" TTEXT_NORMAL " complete.  Total %d pages.\n\n",filename,masterinfo->published_pages);
    else
        k2printf("Processing on " TTEXT_BOLD2 "file %s" TTEXT_NORMAL " complete.  Total %d pages.\n\n",filename,masterinfo->published_pages);
    */
        size=wfile_size(dstfile);
        if (wfile_status(dstfile)==0)
            k2printf("\n" TTEXT_WARN "File %s not written." TTEXT_NORMAL "\n\n",dstfile);
        else if (size<.5)
            k2printf("\n" TTEXT_WARN "File %s is empty (0 bytes)." TTEXT_NORMAL "\n\n",dstfile);
        else
            {
            k2printf("\n" TTEXT_BOLD "%d pages" TTEXT_NORMAL,masterinfo->published_pages);
            if (masterinfo->wordcount>0)
                k2printf(" (%d words)",masterinfo->wordcount);
            k2printf(" written to " TTEXT_MAGENTA "%s" TTEXT_NORMAL " (%.1f MB).\n\n",
                    dstfile,fabs(size)/1024./1024.);
            }
#ifdef HAVE_GHOSTSCRIPT
        if (k2settings->ppgs)
            gs_postprocess(dstfile);
#endif
        if (k2settings->show_marked_source)
            {
            size=wfile_size(markedfile);
            k2printf(TTEXT_BOLD "%d pages" TTEXT_NORMAL " written to " TTEXT_MAGENTA "%s" TTEXT_NORMAL " (%.1f MB).\n\n",pages_done,markedfile,size/1024./1024.);
            }
        } /* PDF (non-bitmap) output */
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
    if (src_type==SRC_TYPE_BITMAPFOLDER)
        filelist_free(fl);
    k2fileproc->status=0;
    return(k2fileproc->status);
    }


void bitmap_file_echo_status(char *filename)

    {
    double size;

    size=wfile_size(filename);
    if (wfile_status(filename)==0)
        k2printf("\n" TTEXT_WARN "File %s not written." TTEXT_NORMAL "\n",filename);
    else if (size<.5)
        k2printf("\n" TTEXT_WARN "File %s is empty (0 bytes)." TTEXT_NORMAL "\n",filename);
    else
        k2printf("\n%.2f MB written to " TTEXT_MAGENTA "%s" TTEXT_NORMAL "\n",
            fabs(size)/1024./1024.,filename);
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
                             K2PDFOPT_FILE_PROCESS *k2fileproc)

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
        if (k2fileproc->bmp==NULL)
            {
            bmp_write(masterinfo->preview_bitmap,"k2pdfopt_out.png",NULL,100);
            bmp_free(masterinfo->preview_bitmap);
            }
        }
    return(status);
    }


static char *pagename(int pageno)

    {
    static char pname[32];

    if (pageno<0)
        strcpy(pname,"cover page");
    else
        sprintf(pname,"page %d",pageno);
    return(pname);
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


void filename_get_marked_pdf_name(char *dst,char *fmt,char *pdfname,int filecount,
                                  int pagecount)

    {
    char basepath[MAXFILENAMELEN];
    char basename[MAXFILENAMELEN];
    char mfmt[MAXFILENAMELEN];
    int i;

    if (!stricmp(fmt,".png"))
        strcpy(basename,"%s%04d_k2opt.png");
    else if (!stricmp(fmt,".jpg"))
        strcpy(basename,"%s%04d_k2opt.jpg");
    else if (!stricmp(fmt,".jpeg"))
        strcpy(basename,"%s%04d_k2opt.jpeg");
    else
        wfile_basespec(basename,fmt);
    wfile_basepath(basepath,fmt);
    if ((i=in_string(basename,"_k2opt"))>=0)
        {
        char tempstr[MAXFILENAMELEN];
        basename[i]='\0';
        strcpy(tempstr,basename);
        strcat(tempstr,"_marked");
        strcat(tempstr,&basename[i+6]);
        strcpy(basename,tempstr);
        }
    else
        {
        char fmt_noext[MAXFILENAMELEN];
        char *ext;

        ext=wfile_ext(basename);
        wfile_newext(fmt_noext,basename,"");
        strcat(fmt_noext,"_marked.");
        strcat(fmt_noext,ext);
        strcpy(basename,fmt_noext);
        }
    if (basepath[0]=='\0')
        strcpy(mfmt,basename);
    else
        wfile_fullname(mfmt,basepath,basename);
    filename_substitute(dst,mfmt,pdfname,filecount,pagecount,filename_is_bitmap(fmt)?"":"pdf");
    }
        

static int filename_get_temp_pdf_name(char *dst,char *fmt,char *psname)

    {
    char basepath[MAXFILENAMELEN];
    int i;

    wfile_basepath(basepath,fmt);
    if (basepath[0]=='\0')
        wfile_newext(dst,psname,"pdf");
    else
        {
        char basename[MAXFILENAMELEN];
        wfile_basespec(basename,psname);
        wfile_fullname(dst,basepath,basename);
        wfile_newext(dst,NULL,"pdf");
        }
    if (wfile_status(dst)==0)
        return(1);
    wfile_newext(dst,NULL,"");
    for (i=1;i<10000;i++)
        {
        char newname[MAXFILENAMELEN];
        sprintf(newname,"%s%04d.pdf",dst,i);
        if (wfile_status(newname)==0)
            {
            strcpy(dst,newname);
            break;
            }
        }
    return(i<10000);
    }
    
        
/*
** %s = full filename without extension
** %b = basename without extension
** %f = base path
** If one %d:
**     If pagecount <= 0, gets file count
**     If pagecount >  0 and ext is bitmap, gets page count
** If two %d's:
**     First gets filecount
**     Second gets page count
**
*/
void filename_substitute(char *dst,char *fmt0,char *src,int filecount,int pagecount,
                                char *defext0)

    {
    char *defext;
    int i,j,k;
    char fmt[MAXFILENAMELEN];
    char basespec[MAXFILENAMELEN];
    char basebasespec[MAXFILENAMELEN];
    char basepath[MAXFILENAMELEN];
    char xfmt[128];
    int npd,cpd,bitmap;

    strcpy(fmt,fmt0);
    bitmap=filename_is_bitmap(fmt);
    if (bitmap)
        {
        if (!stricmp(fmt,".png"))
            strcpy(fmt,"%s%04d.png");
        else if (!stricmp(fmt,".jpg"))
            strcpy(fmt,"%s%04d.jpg");
        else if (!stricmp(fmt,".jpeg"))
            strcpy(fmt,"%s%04d.jpeg");
        }
    npd=count_format_strings(fmt,'d');
    if (bitmap && !npd)
        {
        char *ext;
        ext=wfile_ext(fmt); /* ext points into fmt */
        strcpy(basespec,ext);
        if (ext[-1]=='.')
            {
            strcpy(&ext[-1],"%04d.");
            strcat(fmt,basespec);
            }
        else
            strcat(fmt,"%04d");
        npd=1;
        }
    wfile_newext(basespec,src,"");
    wfile_basespec(basebasespec,basespec);
    wfile_basepath(basepath,basespec);
    if (defext0==NULL)
        defext=NULL;
    else
        defext=(defext0[0]=='.' ? &defext0[1] : defext0);
    for (cpd=i=j=0;fmt[i]!='\0';i++)
        {
        if (fmt[i]!='%')
            {
            dst[j++]=fmt[i];
            continue;
            }
        xfmt[0]='%';
        for (k=1;k<120 && (fmt[i+k]=='-' || (fmt[i+k]>='0' && fmt[i+k]<='9'));k++)
            xfmt[k]=fmt[i+k];
        if (fmt[i+k]=='s' || fmt[i+k]=='d' || fmt[i+k]=='b' || fmt[i+k]=='f')
            {
            int c;
            c=xfmt[k]=fmt[i+k];
            xfmt[k+1]='\0';
            i=i+k;
            dst[j]='\0';
            if (c=='s')
                sprintf(&dst[strlen(dst)],"%s",basespec);
            else if (c=='b')
                sprintf(&dst[strlen(dst)],"%s",basebasespec);
            else if (c=='f')
                sprintf(&dst[strlen(dst)],"%s",basepath);
            else
                {
                cpd++;
                if (!bitmap || (npd>1 && cpd==1))
                    sprintf(&dst[strlen(dst)],xfmt,filecount);
                else
                    sprintf(&dst[strlen(dst)],xfmt,pagecount);
                }
            j=strlen(dst);
            continue;
            }
        dst[j++]=fmt[i];
        }
    dst[j]='\0';
    if (defext!=NULL && defext[0]!='\0' && stricmp(wfile_ext(dst),defext))
        {
        strcat(dst,".");
        strcat(dst,defext);
        }
    }


int filename_is_bitmap(char *filename)

    {
    char *ext;

    ext=wfile_ext(filename);
    return(!stricmp(ext,"png") || !stricmp(ext,"jpg") || !stricmp(ext,"jpeg"));
    }


static int count_format_strings(char *fmt,int type)

    {
    int i,c;

    for (i=c=0;fmt[i]!='\0';i++)
        {
        if (fmt[i]!='%')
            continue;
        for (i++;fmt[i]!='\0' && (fmt[i]=='-' || (fmt[i]>='0' && fmt[i]<='9'));i++);
        if (fmt[i]==type)
            c++;
        }
    return(c);
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


static int overwrite_fail(char *outname,double overwrite_minsize_mb,int assume_yes)

    {
    double size_mb;
    char basepath[512];
    char buf[512];
    char newname[512];

    if (assume_yes)
        return(0);
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
                if (reply<0)
                    return(1);
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


static int get_source_type(char *filename)

    {
    /*
    ** Determine source type
    */
    if (wfile_status(filename)==2)
        return(SRC_TYPE_BITMAPFOLDER);
    else if (!stricmp(wfile_ext(filename),"pdf"))
        return(SRC_TYPE_PDF);
    else if (!stricmp(wfile_ext(filename),"djvu"))
        return(SRC_TYPE_DJVU);
    else if (!stricmp(wfile_ext(filename),"djv"))
        return(SRC_TYPE_DJVU);
    else if (!stricmp(wfile_ext(filename),"ps"))
        return(SRC_TYPE_PS);
    else if (!stricmp(wfile_ext(filename),"eps"))
        return(SRC_TYPE_PS);
    else
        return(SRC_TYPE_OTHER);
    }


static int file_numpages(char *filename,char *mupdffilename,int src_type,int *usegs)

    {
    int np;

    wsys_set_decimal_period(1);
#ifdef HAVE_MUPDF_LIB
    if (src_type==SRC_TYPE_PDF)
        {
        np=wmupdf_numpages(mupdffilename);
#ifdef HAVE_WIN32_API
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
    if (usegs!=NULL && src_type==SRC_TYPE_PDF && np==-1 && ((*usegs)<=0))
        {
        static char *mupdferr_trygs=TTEXT_WARN "\a\n ** ERROR reading from " TTEXT_BOLD2 "%s" TTEXT_WARN "using MuPDF.  Trying Ghostscript...\n\n" TTEXT_NORMAL;

        k2printf(mupdferr_trygs,filename);
        if ((*usegs)==0)
            (*usegs)=1;
        }
#endif
#ifdef HAVE_Z_LIB
    if (np<=0 && src_type==SRC_TYPE_PDF)
        np=pdf_numpages(filename);
#endif
    return(np);
    }


int k2file_get_num_pages(char *filename)

    {
    int src_type,usegs;
    char mupdffilename[MAXFILENAMELEN];

    src_type = get_source_type(filename);
    if (src_type!=SRC_TYPE_PDF && src_type!=SRC_TYPE_DJVU)
        return(-1);
    strncpy(mupdffilename,filename,MAXFILENAMELEN-1);
    mupdffilename[MAXFILENAMELEN-1]='\0';
    usegs=-1;
    return(file_numpages(filename,mupdffilename,src_type,&usegs));
    }


void k2file_get_overlay_bitmap(WILLUSBITMAP *bmp,double *dpi,char *filename,char *pagelist)

    {
    int i,c,c2,src_type,np;
    char mupdffilename[MAXFILENAMELEN];
    static K2PDFOPT_SETTINGS _k2settings;
    K2PDFOPT_SETTINGS *k2settings;
    WILLUSBITMAP *tmp,_tmp;

    (*dpi)=100.;
    src_type = get_source_type(filename);
    if (src_type!=SRC_TYPE_PDF && src_type!=SRC_TYPE_DJVU)
        return;
    strncpy(mupdffilename,filename,MAXFILENAMELEN-1);
    mupdffilename[MAXFILENAMELEN-1]='\0';
    k2settings=&_k2settings;
    k2pdfopt_settings_init(k2settings);
    k2settings->document_scale_factor=1.0;
    k2settings->usegs=-1;
    np=file_numpages(filename,mupdffilename,src_type,&k2settings->usegs);
    for (c=0,i=1;i<=np;i++)
        if (pagelist_includes_page(pagelist,i,np))
            c++;
#ifdef HAVE_K2GUI
    if (k2gui_active())
        {
        k2gui_overlay_set_num_pages(c);
        k2gui_overlay_set_pages_completed(0,NULL);
        }
#endif
    tmp=&_tmp;
    bmp_init(tmp);
    for (c=c2=0,i=1;i<=np;i++)
        {
        int status;

        if (!pagelist_includes_page(pagelist,i,np))
            continue;
        status=bmp_get_one_document_page(tmp,k2settings,src_type,mupdffilename,i,100.,8,NULL);
        c2++;
#ifdef HAVE_K2GUI
        if (k2gui_active())
            k2gui_overlay_set_pages_completed(c2,NULL);
#endif
        if (status)
            {
#ifdef HAVE_K2GUI
            if (k2gui_active())
                k2gui_overlay_error(filename,i,status);
#endif
            continue;
            }
        c++;
        if (c==1)
            bmp_copy(bmp,tmp);
        else
            bmp8_merge(bmp,tmp,c);
        }
    bmp_free(tmp);
    }


void k2file_look_for_pagebreakmarks(K2PAGEBREAKMARKS *k2pagebreakmarks,
                                    K2PDFOPT_SETTINGS *k2settings,WILLUSBITMAP *src,
                                    WILLUSBITMAP *srcgrey,int dpi)

    {
    int color[2];
    int type[2];
    int n;

#if (WILLUSDEBUGX & 0x800000)
printf("@k2file_look_for_pagebreakmarks.\n");
printf("    k2pagebreakmarks = %p\n",k2pagebreakmarks);
printf("    n=%d\n",k2pagebreakmarks->n);
#endif
    if (k2pagebreakmarks==NULL)
        return;
    k2pagebreakmarks->n=n=0;
    if (k2settings->pagebreakmark_breakpage_color>0)
        {
        color[n]=k2settings->pagebreakmark_breakpage_color;
        type[n]=K2PAGEBREAKMARK_TYPE_BREAKPAGE;
        n++;
        }
#if (WILLUSDEBUGX & 0x800000)
printf("AA\n");
#endif
    if (k2settings->pagebreakmark_nobreak_color>0)
        {
        color[n]=k2settings->pagebreakmark_nobreak_color;
        type[n]=K2PAGEBREAKMARK_TYPE_NOBREAK;
        n++;
        }
#if (WILLUSDEBUGX & 0x800000)
printf("BB\n");
#endif
    if (n==0)
        return;
#if (WILLUSDEBUGX & 0x800000)
printf("CC\n");
#endif
    k2pagebreakmarks_find_pagebreak_marks(k2pagebreakmarks,src,srcgrey,dpi,color,type,n);
#if (WILLUSDEBUGX & 0x800000)
printf("\n%d PAGE BREAK MARKS FOUND.\n",k2pagebreakmarks->n);
for (n=0;n<k2pagebreakmarks->n;n++)
printf("    Mark %2d / %2d at %.2f, %.2f in from top left, type %d\n",n+1,k2pagebreakmarks->n,(double)k2pagebreakmarks->k2pagebreakmark[n].col/dpi,(double)k2pagebreakmarks->k2pagebreakmark[n].row/dpi,k2pagebreakmarks->k2pagebreakmark[n].type);
#endif
    }


#ifdef HAVE_GHOSTSCRIPT

static int gsproc_init(void)

    {
    int status;

    if ((status=willusgs_init(stdout))<0)
        {
        static int warn=0;
        if (warn==0)
            {
            k2printf("\a");
            warn=1;
            }
        k2printf("\n" TTEXT_WARN "** Error %d initializing Ghostscript. **"
                 TTEXT_NORMAL "\n\n",status);
        return(0);
        }
    return(1);
    }


static int gs_convert_to_pdf(char *temppdfname,char *psfilename,K2PDFOPT_SETTINGS *k2settings)

    {
    int status;

    if (!gsproc_init())
        {
        k2printf("\n" TTEXT_WARN "** Cannot process postscript file %s. **"
                 TTEXT_NORMAL "\n\n",psfilename);
        return(0);
        }
    status=filename_get_temp_pdf_name(temppdfname,k2settings->dst_opname_format,psfilename);
    if (!status)
        {
        k2printf("\n" TTEXT_WARN "** Error converting %s to PDF.  Could not get temp name. **"
                 TTEXT_NORMAL "\n\n",psfilename);
        return(0);
        }
    k2printf("Converting " TTEXT_MAGENTA "%s" TTEXT_NORMAL " to PDF...\n",
              psfilename);
    status=willusgs_ps_to_pdf(temppdfname,psfilename,NULL);
    if (status<0)
        {
        static int warn=0;
        if (warn==0)
            {
            k2printf("\a");
            warn=1;
            }
        k2printf("\n" TTEXT_WARN "** Error %d running Ghostscript.  Conversion to PDF aborted. **"
                 TTEXT_NORMAL "\n\n",status);
        remove(temppdfname);
        return(0);
        }
    if (wfile_status(temppdfname)!=1)
        {
        static int warn=0;
        if (warn==0)
            {
            k2printf("\a");
            warn=1;
            }
        k2printf("\n" TTEXT_WARN "** Error converting postscript file %s to PDF. **"
                 TTEXT_NORMAL "\n\n",psfilename);
        remove(temppdfname);
        return(0);
        }
    return(1);
    /*
    size=wfile_size(temppdfname);
    k2printf(TTEXT_BOLD "    ... %d bytes" TTEXT_NORMAL " written to " TTEXT_MAGENTA "%s" TTEXT_NORMAL " (%.1f MB).\n",(int)size,temppdfname,size/1024./1024.);
    */
    }


static void gs_postprocess(char *filename)

    {
    char tempname[MAXFILENAMELEN];
    int status;
    double size;

    if (!gsproc_init())
        {
        k2printf("\n" TTEXT_WARN "** Post-process step aborted. **" TTEXT_NORMAL "\n\n");
        return;
        }
    wfile_abstmpnam(tempname);
    k2printf("Post processing " TTEXT_MAGENTA "%s" TTEXT_NORMAL " with Ghostscript...\n",
              filename);
    status=willusgs_ps_to_pdf(tempname,filename,NULL);
    if (status<0)
        {
        static int warn=0;
        if (warn==0)
            {
            k2printf("\a");
            warn=1;
            }
        k2printf("\n" TTEXT_WARN "** Error %d running Ghostscript.  Post-process step aborted. **"
                 TTEXT_NORMAL "\n\n",status);
        remove(tempname);
        return;
        }
    status=wfile_copy_file(filename,tempname,0);
    if (status==0)
        {
        static int warn=0;
        if (warn==0)
            {
            k2printf("\a");
            warn=1;
            }
        k2printf("\n" TTEXT_WARN "** Error copying temp file %s to %s.  Post-process error. **"
                 TTEXT_NORMAL "\n\n",tempname,filename);
        return;
        }
    remove(tempname);
    size=wfile_size(filename);
    k2printf(TTEXT_BOLD "    ... %d bytes" TTEXT_NORMAL " written to " TTEXT_MAGENTA "%s" TTEXT_NORMAL " (%.1f MB).\n",(int)size,filename,size/1024./1024.);
    }
#endif

static void gs_conv_cleanup(int psconv,char *filename,char *original_file)

    {
    if (psconv)
        {
        remove(filename);
        strcpy(filename,original_file);
        }
    }


static void k2pdfopt_file_process_init(K2PDFOPT_FILE_PROCESS *k2fileproc)

    {
    k2fileproc->callcount=0;
    k2fileproc->filecount=0;
    k2fileproc->bmp = NULL;
    k2fileproc->outname = NULL;
    fontsize_histogram_init(&k2fileproc->fsh);
    }


static void k2pdfopt_file_process_close(K2PDFOPT_FILE_PROCESS *k2fileproc)

    {
    static char *funcname="k2pdfopt_file_process_close";

    fontsize_histogram_free(&k2fileproc->fsh);
    willus_mem_free((double **)&k2fileproc->outname,funcname);
    }

/*
** Returns:
**     1 = success
**     0 = fail
*/
static int k2pdfopt_get_cover_image(WILLUSBITMAP *src,K2PDFOPT_SETTINGS *k2settings,
                                    char *filename,int dpi,int *errcnt,int *pixwarn)

    {
    int status,ib,pageno,src_type;
    char covfile[MAXFILENAMELEN];

    if (k2settings->dst_coverimage[0]=='\0')
        return(0);

    strncpy(covfile,k2settings->dst_coverimage,MAXFILENAMELEN-1);
    covfile[MAXFILENAMELEN-1]='\0';
    ib=in_string(covfile,"[");
    if (ib>=0)
        {
        pageno=atoi(&covfile[ib+1]);
        if (ib>0)
            covfile[ib]='\0';
        else
            {
            strncpy(covfile,filename,MAXFILENAMELEN-1);
            covfile[MAXFILENAMELEN-1]='\0';
            }
        }
    else if (is_an_integer(k2settings->dst_coverimage))
        pageno=atoi(k2settings->dst_coverimage);
    else
        pageno=0;
    src_type = get_source_type(covfile);
    /* If integer, interpret as page number of PDF source file */
    if ((src_type==SRC_TYPE_PDF || src_type==SRC_TYPE_DJVU) && pageno<=0)
        pageno=1;
    status=k2pdfopt_get_file_image(src,k2settings,src_type,covfile,pageno,dpi,errcnt,pixwarn);
    return(status==1 ? 1 : 0);
    }
    

/*
** Gets bitmap from file.
** Returns:
** -1 = Error--don't try to continue reading more pages (stop loop)
**  0 = Error
**  1 = Success
*/
static int k2pdfopt_get_file_image(WILLUSBITMAP *src,K2PDFOPT_SETTINGS *k2settings,
                                   int src_type,char *filename,int pageno,
                                   int dpi,int *errcnt,int *pixwarn)

    {
    static char *readerr=TTEXT_WARN "\a\n ** ERROR reading page %d from " TTEXT_BOLD2 "%s" TTEXT_WARN ".\n\n" TTEXT_NORMAL;
    static char *readlimit=TTEXT_WARN "\a\n ** (No more read errors will be echoed for file %s.)\n\n" TTEXT_NORMAL;
    int source_is_bitmap,bpp,status;
    double npix;

/*
printf("@k2pdfopt_get_file_image, fn=%s, src_type=%d, pageno=%d, dpi=%d\n",filename,src_type,pageno,dpi);
*/
    /* Pre-read at low dpi to check bitmap size */

    source_is_bitmap = (src_type!=SRC_TYPE_PS && src_type!=SRC_TYPE_PDF && src_type!=SRC_TYPE_DJVU);
    if (source_is_bitmap && k2settings_need_color_initially(k2settings))
        bpp=24;
    else
        bpp=8;
    wsys_set_decimal_period(1);
    status=bmp_get_one_document_page(src,k2settings,src_type,filename,pageno,10.,bpp,stdout);
    wsys_set_decimal_period(1);
    if (status<0)
        {
        (*errcnt)=(*errcnt)+1;
        if ((*errcnt)<=10)
            {
            k2printf(readerr,pageno,filename);
            if ((*errcnt)==10)
                k2printf(readlimit,filename);
            }
        /* Error reading PS probably means we've run out of pages. */
        if (src_type==SRC_TYPE_PS)
            return(-1);
        return(0);
        }
    /* If bitmap, no need to re-read */
    if (source_is_bitmap)
        return(1);

    /* Sanity check the bitmap size */
    npix = (double)(dpi/10.)*(dpi/10.)*src->width*src->height;
    if (npix > 2.5e8 && !(*pixwarn))
        {
        int ww,hh;
        ww=(int)((double)(dpi/10.)*src->width+.5);
        hh=(int)((double)(dpi/10.)*src->height+.5);
        k2printf("\a\n" TTEXT_WARN "\n\a ** Source resolution is very high (%d x %d pixels)!\n"
                "    You may want to reduce the -odpi or -idpi setting!\n"
                "    k2pdfopt may crash when reading the source file..."
                TTEXT_NORMAL "\n\n",ww,hh);
        (*pixwarn)=1;
        }

    /* Read again at nominal source dpi */
    wsys_set_decimal_period(1);
    if (k2settings_need_color_initially(k2settings))
        status=bmp_get_one_document_page(src,k2settings,src_type,filename,pageno,
                                         dpi,24,stdout);
    else
        status=bmp_get_one_document_page(src,k2settings,src_type,filename,pageno,
                                         dpi,8,stdout);
    wsys_set_decimal_period(1);
    if (status<0)
        {
        (*errcnt)=(*errcnt)+1;
        if ((*errcnt)<=10)
            {
            k2printf(readerr,pageno,filename);
            if ((*errcnt)==10)
                aprintf(readlimit,filename);
            }
        /* Error reading PS probably means we've run out of pages. */
        if (src_type==SRC_TYPE_PS)
            return(-1);
        return(0);
        }
    return(1);
    }


static int k2file_get_bitmap_file_list(FILELIST *fl,char *filename,int first_time_through)

    {
    char basename[MAXFILENAMELEN];
    static char *iolist[]={"*.png","*.jpg",""};
    static char *eolist[]={""};

    filelist_init(fl);
    wfile_basespec(basename,filename);
    if (first_time_through)
        k2printf("Searching folder " TTEXT_BOLD2 "%s" TTEXT_NORMAL " ... ",basename);
    fflush(stdout);
    filelist_fill_from_disk(fl,filename,iolist,eolist,0,0);
    if (fl->n<=0)
        {
        if (first_time_through)
            k2printf(TTEXT_WARN "\n** No bitmaps found in folder %s.\n\n" 
                    TTEXT_NORMAL,filename);
        return(0);
        }
    if (first_time_through)
        k2printf("%d bitmaps found in %s.\n",(int)fl->n,filename);
    filelist_sort_by_name(fl);
    return(1);
    }

/*
**
** Set up output file names.  Check for overwriting files.
**
** Fills in / initializes:
**     dstfile[] with converted file name.
**     masterinfo->ocrfilename[] with OCR text file name.
**     markedfile[] with marked PDF file name.
**     mpdf with PDFFILE info for marked file.
**
*/
static int k2file_setup_output_file_names(K2PDFOPT_SETTINGS *k2settings,char *filename,
                                          K2PDFOPT_FILE_PROCESS *k2fileproc,
                                          MASTERINFO *masterinfo,
                                          char *dstfile,char *markedfile,PDFFILE *mpdf)

    {
    int bitmap,can_write,status;
    static char *funcname="k2file_setup_output_file_names";

    wfile_newext(dstfile,filename,"");
    bitmap=k2settings_output_is_bitmap(k2settings);
    if (bitmap)
        filename_substitute(dstfile,k2settings->dst_opname_format,filename,k2fileproc->filecount,1,"");
    else
        filename_substitute(dstfile,k2settings->dst_opname_format,filename,k2fileproc->filecount,1,"pdf");
#ifdef HAVE_OCR_LIB
    if (k2settings->ocrout[0]!='\0' && k2settings->dst_ocr)
        filename_substitute(masterinfo->ocrfilename,k2settings->ocrout,filename,k2fileproc->filecount,1,"txt");
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
    wfile_prepdir(dstfile);
    if ((status=overwrite_fail(dstfile,k2settings->overwrite_minsize_mb,
                                       k2settings->assume_yes))!=0)
        {
        masterinfo_free(masterinfo,k2settings);
        if (status<0)
            k2sys_exit(k2settings,20);
        k2fileproc->status=4;
        return(0);
        }
    if (!bitmap && !k2settings->use_crop_boxes)
        can_write = (pdffile_init(&masterinfo->outfile,dstfile,1)!=NULL);
    else
        {
        FILE *f1;
        f1 = wfile_fopen_utf8(dstfile,"w");
        can_write = (f1!=NULL);
        if (f1!=NULL)
            {
            fclose(f1);
            wfile_remove_utf8(dstfile);
            }
        }
    if (!can_write)
        {
        k2printf(TTEXT_WARN "\n\aCannot open %sfile %s for output!" TTEXT_NORMAL "\n\n",
                  bitmap?"":"PDF ",dstfile);
#ifdef HAVE_K2GUI
        if (k2gui_active())
            {
            k2gui_okay("Failed to open output file",
                       "Cannot open %sfile %s for output!\n"
                       "Maybe another application has it open already?\n"
                       "Or does the output folder have write permission?\n"
                       "Conversion failed!",bitmap?"":"PDF ",dstfile);
            k2fileproc->status=4;
            return(0);
            }
#endif
        k2sys_exit(k2settings,30);
        }
    /* Return output file name in k2fileproc for GUI */
    willus_mem_alloc((double **)&k2fileproc->outname,(long)(strlen(dstfile)+1),funcname);
    if (k2fileproc->outname!=NULL)
        strcpy(k2fileproc->outname,dstfile);
    if (k2settings->use_crop_boxes)
        pdffile_close(&masterinfo->outfile);
    if (!bitmap && k2settings->show_marked_source)
        {
        filename_get_marked_pdf_name(markedfile,k2settings->dst_opname_format,filename,
                                     k2fileproc->filecount,1);
        if (pdffile_init(mpdf,markedfile,1)==NULL)
            {
            k2printf(TTEXT_WARN "\n\aCannot open PDF file %s for marked output!" TTEXT_NORMAL "\n\n",markedfile);
            k2sys_exit(k2settings,40);
            }
        }
    return(1);
    }
