/*
** k2file.c      K2pdfopt file handling and main file processing
**               function (k2pdfopt_proc_one()).
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

static void   k2pdfopt_proc_arg(K2PDFOPT_SETTINGS *k2settings,char *arg);
static double k2pdfopt_proc_one(K2PDFOPT_SETTINGS *k2settings,char *filename,double rot_deg);
static int    filename_comp(char *name1,char *name2);
static void   filename_substitute(char *dst,char *fmt,char *src,int count,char *defext0);
static int    overwrite_fail(char *outname,double overwrite_minsize_mb);


void k2pdfopt_proc_wildarg(K2PDFOPT_SETTINGS *k2settings,char *arg)

    {
    int i;

    if (wfile_status(arg)==0)
        {
        FILELIST *fl,_fl;

        fl=&_fl;
        filelist_init(fl);
        filelist_fill_from_disk_1(fl,arg,0,0);
        if (fl->n==0)
            {
            aprintf(TTEXT_WARN "\n** File or folder %s could not be opened.\n\n" TTEXT_NORMAL,arg);
            return;
            }
        for (i=0;i<fl->n;i++)
            {
            char fullname[512];
            wfile_fullname(fullname,fl->dir,fl->entry[i].name);
            k2pdfopt_proc_arg(k2settings,fullname);
            }
        }
    else
        k2pdfopt_proc_arg(k2settings,arg);
    }


static void k2pdfopt_proc_arg(K2PDFOPT_SETTINGS *k2settings,char *arg)

    {
    char filename[256];
    int i;
    double rot;

    strcpy(filename,arg);
    if (wfile_status(filename)==0)
        {
        aprintf(TTEXT_WARN "\n** File or folder %s could not be opened.\n\n" TTEXT_NORMAL,filename);
        return;
        }
    /* If folder, first process all PDF/DJVU/PS files in the folder */
    if (wfile_status(filename)==2)
        {
        // static char *iolist[]={"*.png","*.jpg",""};
        static char *eolist[]={""};
        static char *pdflist[]={"*.pdf","*.djvu","*.djv","*.ps","*.eps",""};
        FILELIST *fl,_fl;

        fl=&_fl;
        filelist_init(fl);
        /*
        filelist_fill_from_disk(fl,filename,iolist,eolist,0,0);
        if (fl->n==0)
            {
        */
            filelist_fill_from_disk(fl,filename,pdflist,eolist,0,0);
            if (fl->n>0)
                {
                for (i=0;i<fl->n;i++)
                    {
                    char fullname[512];

                    wfile_fullname(fullname,filename,fl->entry[i].name);
                    if (fabs(k2settings->src_rot-SRCROT_AUTO)<.5 || fabs(k2settings->src_rot-SRCROT_AUTOEP)<.5)
                        rot=k2pdfopt_proc_one(k2settings,fullname,SRCROT_AUTO);
                    else
                        rot=k2settings->src_rot;
                    k2pdfopt_proc_one(k2settings,fullname,rot);
                    }
                }
        /*
            else
                aprintf(TTEXT_WARN "\n** No files in folder %s.\n\n" TTEXT_NORMAL,filename);
        */
            filelist_free(fl);
        /*
            return;
            }
        filelist_free(fl);
        */
        }
    if (fabs(k2settings->src_rot-SRCROT_AUTO)<.5 || fabs(k2settings->src_rot-SRCROT_AUTOEP)<.5)
        rot=k2pdfopt_proc_one(k2settings,filename,SRCROT_AUTO);
    else
        rot=k2settings->src_rot;
    k2pdfopt_proc_one(k2settings,filename,rot);
    }


/*
** If rot_deg == SRCROT_AUTO, then the rotation correction of the source
** file is computed and returned, but no other processing is done.
**
** Otherwise, the source file is processed.
*/
static double k2pdfopt_proc_one(K2PDFOPT_SETTINGS *k2settings,char *filename,double rot_deg)

    {
    static MASTERINFO _masterinfo,*masterinfo;
    static PDFFILE _mpdf,*mpdf;
    char dstfile[256];
    char markedfile[256];
    char rotstr[128];
    WILLUSBITMAP _src,*src;
    WILLUSBITMAP _srcgrey,*srcgrey;
    WILLUSBITMAP _marked,*marked;
    int i,status,pw,np,src_type,second_time_through,or_detect,orep_detect;
    int pagecount,pagestep,pages_done;
    int errcnt,pixwarn;
    FILELIST *fl,_fl;
    int folder,dpi;
    double size,bormean;
    char mupdffilename[MAXFILENAMELEN];
    static char *readerr=TTEXT_WARN "\a\n ** ERROR reading page %d from " TTEXT_BOLD2 "%s" TTEXT_WARN ".\n\n" TTEXT_NORMAL;
    static char *readlimit=TTEXT_WARN "\a\n ** (No more read errors will be echoed for file %s.)\n\n" TTEXT_NORMAL;
#ifdef HAVE_MUPDF_LIB
    static char *mupdferr_trygs=TTEXT_WARN "\a\n ** ERROR reading from " TTEXT_BOLD2 "%s" TTEXT_WARN "using MuPDF.  Trying Ghostscript...\n\n" TTEXT_NORMAL;
#endif

    mpdf=&_mpdf;
    masterinfo=&_masterinfo;
    masterinfo_init(masterinfo,k2settings);
    k2pdfopt_settings_new_source_document_init(k2settings);
    errcnt=0;
    pixwarn=0;
    strncpy(mupdffilename,filename,MAXFILENAMELEN-1);
    mupdffilename[MAXFILENAMELEN-1]='\0';
    or_detect=OR_DETECT(rot_deg);
    orep_detect=OREP_DETECT(k2settings);
    if ((fabs(k2settings->src_rot-SRCROT_AUTO)<.5 || orep_detect) && !or_detect)
        second_time_through=1;
    else
        second_time_through=0;
    if (or_detect && k2settings->src_dpi>300)
        dpi=300;
    else
        dpi=k2settings->src_dpi;
    folder=(wfile_status(filename)==2);
    /*
    if (folder && !second_time_through)
        aprintf("Processing " TTEXT_INPUT "BITMAP FOLDER %s" TTEXT_NORMAL "...\n",
               filename);
    */
    /*
    else
        aprintf("Processing " TTEXT_BOLD2 "PDF FILE %s" TTEXT_NORMAL "...\n",
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
            aprintf("Searching folder " TTEXT_BOLD2 "%s" TTEXT_NORMAL " ... ",basename);
        fflush(stdout);
        filelist_fill_from_disk(fl,filename,iolist,eolist,0,0);
        if (fl->n<=0)
            {
            if (!second_time_through)
                aprintf(TTEXT_WARN "\n** No bitmaps found in folder %s.\n\n" 
                        TTEXT_NORMAL,filename);
            masterinfo_free(masterinfo,k2settings);
            return(0.);
            }
        if (!second_time_through)
            printf("%d bitmaps found in %s.\n",(int)fl->n,filename);
        filelist_sort_by_name(fl);
        }
    src=&_src;
    srcgrey=&_srcgrey;
    marked=&_marked;
    bmp_init(src);
    bmp_init(srcgrey);
    bmp_init(marked);
    /*
    masterinfo->bmp.width=dst_width;
    area_ratio = 8.5*11.0*dst_dpi*dst_dpi / (dst_width*dst_height);
    masterinfo->bmp.height=dst_height*area_ratio*1.5;
    if (!or_detect)
        {
        bmp_alloc(&masterinfo->bmp);
        bmp_fill(&masterinfo->bmp,255,255,255);
        }
    masterinfo->rows=0;
    */
    pw=0;
    if (!or_detect)
        {
        static int dstfilecount=0;

        wfile_newext(dstfile,filename,"");
        dstfilecount++;
        filename_substitute(dstfile,k2settings->dst_opname_format,filename,dstfilecount,"pdf");
        if (!filename_comp(dstfile,filename))
            {
            aprintf(TTEXT_WARN "\n\aSource file and ouput file have the same name!" TTEXT_NORMAL "\n\n");
            printf("    Source file = '%s'\n",filename);
            printf("    Output file = '%s'\n",dstfile);
            printf("    Output file name format string = '%s'\n",k2settings->dst_opname_format);
            printf("\nOperation aborted.\n");
            k2sys_exit(k2settings,50);
            }
        if ((status=overwrite_fail(dstfile,k2settings->overwrite_minsize_mb))!=0)
            {
            masterinfo_free(masterinfo,k2settings);
            if (folder)
                filelist_free(fl);
            if (status<0)
                k2sys_exit(k2settings,20);
            return(0.);
            }
        if (pdffile_init(&masterinfo->outfile,dstfile,1)==NULL)
            {
            aprintf(TTEXT_WARN "\n\aCannot open PDF file %s for output!" TTEXT_NORMAL "\n\n",dstfile);
            k2sys_exit(k2settings,30);
            }
        if (k2settings->use_crop_boxes)
            pdffile_close(&masterinfo->outfile);
        if (k2settings->show_marked_source)
            {
            filename_substitute(markedfile,"%s_marked",filename,0,"pdf");
            if (pdffile_init(mpdf,markedfile,1)==NULL)
                {
                aprintf(TTEXT_WARN "\n\aCannot open PDF file %s for marked output!" TTEXT_NORMAL "\n\n",markedfile);
                k2sys_exit(k2settings,40);
                }
            }
        }
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
    if (src_type==SRC_TYPE_PS)
        k2settings->usegs=1;
#ifndef HAVE_DJVU_LIB
    if (src_type==SRC_TYPE_DJVU)
        {
        if (!or_detect)
            aprintf(TTEXT_WARN
                    "\a\n\n** DjVuLibre not compiled into this version of k2pdfopt. **\n\n"
                          "** Cannot process file %s. **\n\n" TTEXT_NORMAL,filename);
        masterinfo_free(masterinfo,k2settings);
        return(0.);
        }
#endif
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
            aprintf(mupdferr_trygs,filename);
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
    pagecount = np<0 ? -1 : pagelist_count(k2settings->pagelist,np);
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
            aprintf("\a\n" TTEXT_WARN "No %ss to convert (-p %s)!" TTEXT_NORMAL "\n\n",
                     folder?"file":"page",k2settings->pagelist);
        masterinfo_free(masterinfo,k2settings);
        if (folder)
            filelist_free(fl);
        return(0.);
        }
    if (!second_time_through)
        {
        aprintf("Reading ");
        if (pagecount>0)
           {
           if (pagecount<np)
               aprintf("%d out of %d %s%s",pagecount,np,folder?"file":"page",np>1?"s":"");
           else
               aprintf("%d %s%s",np,folder?"file":"page",np>1?"s":"");
           }
        else
           aprintf("%ss",folder?"file":"page");
        aprintf(" from " TTEXT_BOLD2 "%s" TTEXT_NORMAL " ...\n",filename);
        }
    if (or_detect)
        aprintf("\nDetecting document orientation ... ");
    bormean=1.0;
    for (i=0;1;i+=pagestep)
        {
        BMPREGION region;
        char bmpfile[256];
        int pageno;

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
                    aprintf(TTEXT_WARN "\n\aCould not read file %s.\n" TTEXT_NORMAL,bmpfile);
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
                    aprintf(readerr,pageno,filename);
                    if (errcnt==10)
                        aprintf(readlimit,filename);
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
                aprintf("\a\n" TTEXT_WARN "\n\a ** Source resolution is very high (%d x %d pixels)!\n"
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
                    aprintf(readerr,pageno,filename);
                    if (errcnt==10)
                        aprintf(readlimit,filename);
                    }
                /* Error reading PS probably means we've run out of pages. */
                if (src_type==SRC_TYPE_PS)
                    break;
                continue;
                }
            }

        /* Got Good Page Render */
        if (masterinfo_new_source_page_init(masterinfo,k2settings,src,srcgrey,marked,
                                 &region,rot_deg,&bormean,rotstr,pageno,stdout)==0)
            {
            pages_done++;
            continue;
            }

        aprintf("\n" TTEXT_HEADER "SOURCE PAGE %d",pageno);
        if (pagecount>0)
            {
            if (k2settings->pagelist[0]!='\0')
                aprintf(" (%d of %d)",pages_done+1,pagecount);
            else
                aprintf(" of %d",pagecount);
            }
        aprintf(TTEXT_NORMAL 
                " (%.1f x %.1f in) ... %s",(double)srcgrey->width/k2settings->src_dpi,
                  (double)srcgrey->height/k2settings->src_dpi,rotstr);
        fflush(stdout);

        /* Insert gap between source pages if requested */
        if (pages_done>0 && k2settings->dst_break_pages<-1)
            masterinfo_add_gap(masterinfo,k2settings,(-1-k2settings->dst_break_pages)/1000.);
        /* Parse the source bitmap for viewable regions */
        bmpregion_source_page_add(&region,k2settings,masterinfo,1,
                                  pages_done==0. ? 0. : (int)(0.25*k2settings->src_dpi+.5));
        pages_done++;
        if (k2settings->verbose)
            {
            printf("    master->rows=%d\n",masterinfo->rows);
            printf("Publishing...\n");
            }
        /* Reset the display order for this source page */
        if (k2settings->show_marked_source)
            mark_source_page(k2settings,NULL,0,0xf);
        if (k2settings->dst_fit_to_page!=-2)
            masterinfo_publish(masterinfo,k2settings,k2settings->dst_break_pages>0 ? 1 : 0);
        if (k2settings->show_marked_source)
            publish_marked_page(mpdf,k2settings->dst_color ? marked : src,k2settings->src_dpi);
        printf("%d new pages saved.\n",masterinfo->published_pages-pw);
        pw=masterinfo->published_pages;
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
                printf("Rotating clockwise.\n");
                masterinfo_free(masterinfo,k2settings);
                if (folder)
                    filelist_free(fl);
                return(270.);
                }
            }
        printf("No rotation necessary.\n");
        masterinfo_free(masterinfo,k2settings);
        if (folder)
            filelist_free(fl);
        return(0.);
        }
    if (k2settings->dst_break_pages<=0 && k2settings->dst_fit_to_page!=-2)
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
        pdffile_finish(&masterinfo->outfile,title,author,masterinfo->pageinfo.producer,cdate);
        pdffile_close(&masterinfo->outfile);
        }
    else
        {
        /* Re-write PDF file using crop boxes */
#if (WILLUSDEBUGX & 64)
int i;
for (i=0;i<masterinfo->pageinfo.boxes.n;i++)
{
WPDFBOX *box;
box=&masterinfo->pageinfo.boxes.box[i];
printf("Box %d:\n",i);
printf("    srcpage=%2d, dstpage=%2d\n",box->srcbox.pageno,box->dstpage);
printf("    x0=%5.1f, y0=%5.1f\n",box->x0,box->y0);
printf("    w =%5.1f, h =%5.1f\n",box->w,box->h);
printf("    x1=%5.1f, y1=%5.1f\n",box->x1,box->y1);
printf("    sr=%5.1f, dr=%5.1f\n\n",box->srcrot_deg,box->dstrot_deg);
}
#endif
#ifdef HAVE_MUPDF_LIB
        wmupdf_remake_pdf(mupdffilename,dstfile,&masterinfo->pageinfo,1,stdout);
#endif
        }
    if (k2settings->show_marked_source)
        {
        pdffile_finish(mpdf,title,author,masterinfo->pageinfo.producer,cdate);
        pdffile_close(mpdf);
        }
    } // cdate, author, title selection
    if (k2settings->debug || k2settings->verbose)
        printf("Cleaning up ...\n\n");
    /*
    if (folder)
        aprintf("Processing on " TTEXT_INPUT "folder %s" TTEXT_NORMAL " complete.  Total %d pages.\n\n",filename,masterinfo->published_pages);
    else
        aprintf("Processing on " TTEXT_BOLD2 "file %s" TTEXT_NORMAL " complete.  Total %d pages.\n\n",filename,masterinfo->published_pages);
    */
    size=wfile_size(dstfile);
    aprintf("\n" TTEXT_BOLD "%d pages" TTEXT_NORMAL,masterinfo->published_pages);
    if (masterinfo->wordcount>0)
        aprintf(" (%d words)",masterinfo->wordcount);
    aprintf(" written to " TTEXT_MAGENTA "%s" TTEXT_NORMAL " (%.1f MB).\n\n",
            dstfile,size/1024./1024.);
    if (k2settings->show_marked_source)
        {
        size=wfile_size(markedfile);
        aprintf(TTEXT_BOLD "%d pages" TTEXT_NORMAL " written to " TTEXT_MAGENTA "%s" TTEXT_NORMAL " (%.1f MB).\n\n",pages_done,markedfile,size/1024./1024.);
        }
    masterinfo_free(masterinfo,k2settings);
    if (folder)
        filelist_free(fl);
    return(0.);
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


static int overwrite_fail(char *outname,double overwrite_minsize_mb)

    {
    double size_mb;
    char basepath[512];
    char buf[512];
    char newname[512];
    static int all=0;

    if (wfile_status(outname)==0)
        return(0);
    if (overwrite_minsize_mb < 0.)
        return(0);
    if (all)
        return(0);
    size_mb = wfile_size(outname)/1024./1024.;
    if (size_mb < overwrite_minsize_mb)
        return(0);
    wfile_basepath(basepath,outname);
    strcpy(newname,outname);
    printf("\n\a");
    while (1)
        {
        while (1)
            {
            aprintf("File " TTEXT_MAGENTA "%s" TTEXT_NORMAL " (%.1f MB) already exists!\n"
                      "   Overwrite it (y[es]/n[o]/a[ll]/q[uit])? " TTEXT_INPUT,
                    newname,size_mb);
            fgets(buf,16,stdin);
            aprintf(TTEXT_NORMAL);
            clean_line(buf);
            buf[0]=tolower(buf[0]);
            if (buf[0]!='y' && buf[0]!='n' && buf[0]!='a' && buf[0]!='q')
                {
                aprintf("\a\n  ** Must respond with 'y', 'n', 'a', or 'q' **\n\n");
                continue;
                }
            break;
            }
        if (buf[0]=='q')
            return(-1);
        if (buf[0]=='a' || buf[0]=='y')
            {
            if (buf[0]=='a')
                all=1;
            return(0);
            }
        aprintf("Enter a new output base name (.pdf will be appended, q=quit).\n"
                "New name: " TTEXT_INPUT);
        fgets(buf,255,stdin);
        aprintf(TTEXT_NORMAL);
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
