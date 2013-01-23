/*
** k2master.c    Functions to handle the main (master) k2pdfopt output bitmap.
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

static int masterinfo_detecting_orientation(MASTERINFO *masterinfo,K2PDFOPT_SETTINGS *k2settings,
                                 WILLUSBITMAP *src,WILLUSBITMAP *srcgrey,
                                 char *rotstr,double rot_deg,double *bormean,int pageno);
#ifdef HAVE_MUPDF_LIB
static void masterinfo_add_cropbox(MASTERINFO *masterinfo,K2PDFOPT_SETTINGS *k2settings,
                                   WILLUSBITMAP *bmp1,double bmpdpi,int rows);
#endif
static void bmp_pad_and_mark(WILLUSBITMAP *dst,WILLUSBITMAP *src,K2PDFOPT_SETTINGS *k2settings,
                             int ltotheight,double bmpdpi,void *ocrwords);
static void bmp_fully_justify(WILLUSBITMAP *jbmp,WILLUSBITMAP *src,
                              K2PDFOPT_SETTINGS *k2settings,int jbmpwidth,
                              int whitethresh,int just,int dpi);
static int masterinfo_break_point(MASTERINFO *masterinfo,int maxsize);


void masterinfo_init(MASTERINFO *masterinfo,K2PDFOPT_SETTINGS *k2settings)

    {
    extern char *k2pdfopt_version;
    int i;

    masterinfo->published_pages=0;
    masterinfo->wordcount=0;
    masterinfo->debugfolder[0]='\0';
    bmp_init(&masterinfo->bmp);
    masterinfo->bmp.bpp=k2settings->dst_color ? 24 : 8;
    for (i=0;i<256;i++)
        masterinfo->bmp.red[i]=masterinfo->bmp.blue[i]=masterinfo->bmp.green[i]=i;
    wrapbmp_init(&masterinfo->wrapbmp,k2settings->dst_color);
#ifdef HAVE_MUPDF_LIB
    if (k2settings->use_crop_boxes)
        wpdfboxes_init(&masterinfo->pageinfo.boxes);
#endif
#ifndef K2PDFOPT_KINDLEPDFVIEWER
    if (k2settings->debug)
        {
        strcpy(masterinfo->debugfolder,"k2_dst_dir");
        wfile_remove_dir(masterinfo->debugfolder,1);
        wfile_makedir(masterinfo->debugfolder);
        }
    else
#endif
        masterinfo->debugfolder[0]='\0';
    masterinfo->rows=0;
    sprintf(masterinfo->pageinfo.producer,"K2pdfopt %s",k2pdfopt_version);
    }


void masterinfo_free(MASTERINFO *masterinfo,K2PDFOPT_SETTINGS *k2settings)

    {
#ifdef HAVE_MUPDF_LIB
    if (k2settings->use_crop_boxes)
        wpdfboxes_free(&masterinfo->pageinfo.boxes);
#endif
    wrapbmp_free(&masterinfo->wrapbmp);
    bmp_free(&masterinfo->bmp);
    }


void masterinfo_flush(MASTERINFO *masterinfo,K2PDFOPT_SETTINGS *k2settings)

    {
#ifndef K2PDFOPT_KINDLEPDFVIEWER
    masterinfo_publish(masterinfo,k2settings,1);
#endif
    }


/*
**
** Start new source page
**
** Input = "src" bitmap (color or grayscale)
**
** Initializes greyscale bitmap "srcgrey" and BMPREGION "region".
** Sets up vars in masterinfo for new page.
**
*/
int masterinfo_new_source_page_init(MASTERINFO *masterinfo,K2PDFOPT_SETTINGS *k2settings,
                         WILLUSBITMAP *src,WILLUSBITMAP *srcgrey,WILLUSBITMAP *marked,
                         BMPREGION *region,double rot_deg,double *bormean,
                         char *rotstr,int pageno,FILE *out)

    {
    int white;

    white=k2settings->src_whitethresh;
    if (k2settings->use_crop_boxes)
        {
        masterinfo->pageinfo.srcpage = pageno;
        masterinfo->pageinfo.srcpage_rot_deg=0.;
        masterinfo->pageinfo.srcpage_fine_rot_deg = 0.;
        }
    if (!OR_DETECT(rot_deg) && !OREP_DETECT(k2settings) && rot_deg!=0)
        {
        bmp_rotate_right_angle(src,rot_deg);
        if (k2settings->use_crop_boxes)
            masterinfo->pageinfo.srcpage_rot_deg=rot_deg;
        }
    if (bmp_is_grayscale(src))
        bmp_copy(srcgrey,src);
    else
        bmp_convert_to_greyscale_ex(srcgrey,src);
    if (!OR_DETECT(rot_deg) && (k2settings->dst_color || k2settings->show_marked_source))
        bmp_promote_to_24(src);
    bmp_adjust_contrast(src,srcgrey,k2settings,&white);
    /*
    if (k2settings->src_whitethresh>0)
        white=k2settings->src_whitethresh;
    */
    if (masterinfo_detecting_orientation(masterinfo,k2settings,src,srcgrey,rotstr,
                                         rot_deg,bormean,pageno))
        return(0);
    if (k2settings->erase_vertical_lines>0)
        bmp_detect_vertical_lines(srcgrey,src,(double)k2settings->src_dpi,0.005,0.25,
                        k2settings->min_column_height_inches,k2settings->src_autostraighten,white,
                        k2settings->erase_vertical_lines,
                        k2settings->debug,k2settings->verbose);
    if (k2settings->src_autostraighten > 0.)
        {
        double rot;
        rot=bmp_autostraighten(src,srcgrey,white,k2settings->src_autostraighten,0.1,
                               k2settings->debug,out);
        if (k2settings->use_crop_boxes)
            masterinfo->pageinfo.srcpage_fine_rot_deg = rot;
        }
    bmp_clear_outside_crop_border(src,srcgrey,k2settings);
    region->dpi = k2settings->src_dpi;
    region->r1 = 0;
    region->r2 = srcgrey->height-1;
    region->c1 = 0;
    region->c2 = srcgrey->width-1;
    region->bgcolor = white;
    region->bmp = src;
    region->bmp8 = srcgrey;
    if (k2settings->show_marked_source)
        {
        if (k2settings->dst_color && marked!=NULL)
            {
            bmp_copy(marked,src);
            region->marked=marked;
            }
        else
            region->marked=region->bmp;
        }
    masterinfo->bgcolor=white;
    masterinfo->fit_to_page = k2settings->dst_fit_to_page;
    /* Set destination size (flush output bitmap if it changes) */
    k2pdfopt_settings_set_margins_and_devsize(k2settings,region,masterinfo,0);
    return(1);
    }


static int masterinfo_detecting_orientation(MASTERINFO *masterinfo,K2PDFOPT_SETTINGS *k2settings,
                                 WILLUSBITMAP *src,WILLUSBITMAP *srcgrey,
                                 char *rotstr,double rot_deg,double *bormean,int pageno)

    {
    if (rotstr!=NULL)
        rotstr[0]='\0';
    if (OR_DETECT(rot_deg) || OREP_DETECT(k2settings))
        {
        double bor,rotnow;

        if (k2settings->debug)
            printf("Checking orientation of page %d ... ",pageno);
        bor=bmp_orientation(srcgrey);
        if (k2settings->debug)
            printf("orientation factor = %g\n",bor);
        if (OR_DETECT(rot_deg))
            {
            if (bormean!=NULL)
                (*bormean) *= bor;
            return(1);
            }
        rotnow=rot_deg;
        if (fabs(rot_deg-270)<.5)
            {
            if (bor>10.)
                {
                if (rotstr!=NULL)
                    sprintf(rotstr,"(custom rotation) ");
                rotnow=0.;
                }
            }
        else if (fabs(rot_deg)<.5)
            {
            if (bor<.1)
                {
                if (rotstr!=NULL)
                    sprintf(rotstr,"(custom rotation) ");
                rotnow=270.;
                }
            }
        if (rotnow!=0)
            {
            bmp_rotate_right_angle(srcgrey,rotnow);
            if (k2settings->dst_color)
                bmp_rotate_right_angle(src,rotnow);
            if (k2settings->use_crop_boxes)
                masterinfo->pageinfo.srcpage_rot_deg=rotnow;
            }
        }
    return(0);
    }




/*
**
** Add already-scaled source bmp to "masterinfo" destination bmp.
** Source bmp may be narrower than destination--if so, it may be fully justifed.
** dst = destination bitmap
** src = source bitmap
** dst and src bpp must match!
** All rows of src are applied to masterinfo->bmp starting at row masterinfo->rows
** Full justification is done if requested.
**
*/
void masterinfo_add_bitmap(MASTERINFO *masterinfo,WILLUSBITMAP *src,
                    K2PDFOPT_SETTINGS *k2settings,int have_pagebox,
                    int justification_flags,int whitethresh,int nocr,int dpi)

    {
    WILLUSBITMAP *src1,_src1;
    WILLUSBITMAP *tmp;
#ifdef HAVE_OCR_LIB
    WILLUSBITMAP _tmp;
    OCRWORDS _words,*words;
#endif
    int dw,dw2;
    int i,srcbytespp,srcbytewidth,go_full;
    int destwidth,destx0,just;

    if (src->width<=0 || src->height<=0)
        return;
/*
printf("@masterinfo_add_bitmap.  dst->bpp=%d, src->bpp=%d, src=%d x %d\n",masterinfo->bmp.bpp,src->bpp,src->width,src->height);
*/
/*
{
static int count=0;
static char filename[256];

printf("    @masterinfo_add_bitmap...\n");
sprintf(filename,"src%05d.png",count++);
bmp_write(src,filename,stdout,100);
}
*/
/*
if (fulljust && k2settings->dst_fulljustify)
printf("srcbytespp=%d, srcbytewidth=%d, destwidth=%d, destx0=%d, destbytewidth=%d\n",
srcbytespp,srcbytewidth,destwidth,destx0,dstbytewidth);
*/

    /* Determine what justification to use */
    /* Left? */
    if ((justification_flags&3)==0  /* Mandatory left just */
          || ((justification_flags&3)==3  /* Use user settings */
              && (k2settings->dst_justify==0  
                    || (k2settings->dst_justify<0 && (justification_flags&0xc)==0))))
        just=0;
    else if ((justification_flags&3)==2
          || ((justification_flags&3)==3
              && (k2settings->dst_justify==2
                    || (k2settings->dst_justify<0 && (justification_flags&0xc)==8))))
        just=2;
    else
        just=1;

    /* Full justification? */
    destwidth=(int)(masterinfo->bmp.width-(k2settings->dst_marleft+k2settings->dst_marright)*k2settings->dst_dpi+.5);
    go_full = (destwidth*nocr > src->width 
               && (((justification_flags&0x30)==0x10)
                   || ((justification_flags&0x30)==0 // Use user settings
                       && (k2settings->dst_fulljustify==1
                            || (k2settings->dst_fulljustify<0 && (justification_flags&0xc0)==0x40)))));

    /* Cannot fully justify if using crop boxes */
    if (have_pagebox)
        go_full=0;

    /* Put fully justified text into src1 bitmap */
    if (go_full)
        {
        src1=&_src1;
        bmp_init(src1);
        bmp_fully_justify(src1,src,k2settings,nocr*destwidth,whitethresh,just,dpi);
        }
    else
        src1=src;

#if (WILLUSDEBUGX & 1)
printf("@masterinfo_add_bitmap:  jflags=0x%02X just=%d, go_full=%d\n",justification_flags,just,go_full);
printf("    destx0=%d, destwidth=%d, src->width=%d\n",destx0,destwidth,src->width);
#endif
#ifdef HAVE_OCR_LIB
    if (k2settings->dst_ocr)
        {
        /* Run OCR on the bitmap */
        words=&_words;
        ocrwords_init(words);
        k2ocr_ocrwords_fill_in(k2settings,words,src1,whitethresh,dpi);
        /* Scale bitmap and word positions to destination size */
        if (nocr>1)
            {
            tmp=&_tmp;
            bmp_init(tmp);
            bmp_integer_resample(tmp,src1,nocr);
            ocrwords_int_scale(words,nocr);
            }
        else
            tmp=src1;
        }
    else
#endif
        tmp=src1;
/*
printf("writing...\n");
ocrwords_box(words,tmp);
bmp_write(tmp,"out.png",stdout,100);
exit(10);
*/
    destx0=(int)(k2settings->dst_marleft*k2settings->dst_dpi+.5);
    if (just==0)
        dw=destx0;
    else if (just==1)
        dw=destx0+(destwidth-tmp->width)/2;
    else
        dw=destx0+destwidth-tmp->width;
    if (dw<0)
        dw=0;
    /* Add OCR words to destination list */
#ifdef HAVE_OCR_LIB
    if (k2settings->dst_ocr)
        {
        ocrwords_offset(words,dw,masterinfo->rows);
        ocrwords_concatenate(&k2settings->dst_ocrwords,words);
        ocrwords_free(words);
        }
#endif

    /*
    ** For now:  set destination position in pageinfo structure as pixel position
    ** relative to top of master bitmap.  scale = the height in pixels on the master bitmap.
    */
#ifdef HAVE_MUPDF_LIB
    if (have_pagebox)
        {
        WPDFBOX *box;

        box=&masterinfo->pageinfo.boxes.box[masterinfo->pageinfo.boxes.n-1];
        /* These values will get adjusted in masterinfo_publish() */
        box->x1 = dw;
        box->y1 = masterinfo->rows;
        box->userx = tmp->width;
        box->usery = tmp->height;
        }
#endif

    /* Add tmp bitmap to dst */
    srcbytespp = tmp->bpp==24 ? 3 : 1;
    srcbytewidth = tmp->width*srcbytespp;
    dw2=masterinfo->bmp.width-tmp->width-dw;
    dw *= srcbytespp;
    dw2 *= srcbytespp;
    for (i=0;i<tmp->height;i++,masterinfo->rows++)
        {
        unsigned char *pdst,*psrc;

        psrc=bmp_rowptr_from_top(tmp,i);
        pdst=bmp_rowptr_from_top(&masterinfo->bmp,masterinfo->rows);
        memset(pdst,255,dw);
        pdst += dw;
        memcpy(pdst,psrc,srcbytewidth);
        pdst += srcbytewidth;
        memset(pdst,255,dw2);
        }


#ifdef HAVE_OCR_LIB
    if (k2settings->dst_ocr && nocr>1)
        bmp_free(tmp);
#endif
    if (go_full)
        bmp_free(src1);
    }


void masterinfo_add_gap_src_pixels(MASTERINFO *masterinfo,K2PDFOPT_SETTINGS *k2settings,
                                   int pixels,char *caller)

    {
    double gap_inches;

/*
aprintf("%s " ANSI_GREEN "masterinfo_add" ANSI_NORMAL " %.3f in (%d pix)\n",caller,(double)pixels/k2settings->src_dpi,pixels);
*/
    if (k2settings->last_scale_factor_internal < 0.)
        gap_inches=(double)pixels/k2settings->src_dpi;
    else
        gap_inches=(double)pixels*k2settings->last_scale_factor_internal/k2settings->dst_dpi;
    gap_inches *= k2settings->vertical_multiplier;
    if (gap_inches > k2settings->max_vertical_gap_inches)
        gap_inches=k2settings->max_vertical_gap_inches;
    masterinfo_add_gap(masterinfo,k2settings,gap_inches);
    }


void masterinfo_remove_top_rows(MASTERINFO *masterinfo,K2PDFOPT_SETTINGS *k2settings,int rows)

    {
    int bw,i;

    /* Clear the published page:  move everything "up" by "rows" pixels */
    bw=bmp_bytewidth(&masterinfo->bmp);
    for (i=rows;i<masterinfo->rows;i++)
        {
        unsigned char *psrc,*pdst;
        pdst=bmp_rowptr_from_top(&masterinfo->bmp,i-rows);
        psrc=bmp_rowptr_from_top(&masterinfo->bmp,i);
        memcpy(pdst,psrc,bw);
        }
    masterinfo->rows -= rows;
    /* Move unused crop box positions by -rows so they track the master bitmap */
#ifdef HAVE_MUPDF_LIB
    if (k2settings->use_crop_boxes)
        for (i=0;i<masterinfo->pageinfo.boxes.n;i++)
            {
            WPDFBOX *box;
            box=&masterinfo->pageinfo.boxes.box[i];
            if (box->dstpage<0)
                box->y1 -= rows;
            }
#endif
#ifdef HAVE_OCR_LIB
    /* Move unused OCR words by -rows so they track the master bitmap */
    if (k2settings->dst_ocr)
        ocrwords_offset(&k2settings->dst_ocrwords,0,-rows);
#endif
    }


/*
** Publish pages from the master bitmap by finding break points that will
** fit the device page.
**
** k2settings->dst_width = masterinfo->bmp.width
**
*/
int masterinfo_get_next_output_page(MASTERINFO *masterinfo,K2PDFOPT_SETTINGS *k2settings,
                                    int flushall,WILLUSBITMAP *bmp,double *bmpdpi,
                                    int *size_reduction,void *ocrwords)

    {
    /* bmp1 = viewable contents bitmap (smaller than bmp) */
    WILLUSBITMAP *bmp1,_bmp1;
    int rr,maxsize,r0;
    /* Local DPI, width, height */
    double ldpi;
    int lwidth,lheight,ltotheight;
    int bp,i;

    if (k2settings->debug)
        printf("@masterinfo_get_next_output_page(page %d)\n",masterinfo->published_pages);
    if (masterinfo->bmp.width != k2settings->dst_width)
        {
        aprintf("\n\n\a" TTEXT_WARN "!! Internal error, masterinfo->bmp.width=%d != dst_width=%d.\n"
               "Contact author." TTEXT_NORMAL "\n\n",masterinfo->bmp.width,k2settings->dst_width);
        wsys_enter_to_exit(NULL);
        exit(10);
        }
    /* v1.52: Make sure text wrapping is flushed if we are to publish everything. */
    if (flushall)
        wrapbmp_flush(masterinfo,k2settings,0,0);
    bmp1=&_bmp1;
    bmp_init(bmp1);
    /* dh = viewable height in pixels */
    maxsize=k2settings->dst_height-(int)(k2settings->dst_dpi*(k2settings->dst_marbot+k2settings->dst_martop)+.5);
    if (maxsize>k2settings->dst_height)
        maxsize=k2settings->dst_height;
    r0=(int)(k2settings->dst_dpi*k2settings->dst_martop+.5);
    if (r0+maxsize > k2settings->dst_height)
        r0=k2settings->dst_height-maxsize;
    rr= flushall ? 0 : maxsize;
    if (k2settings->verbose)
        printf("rows=%d, maxsize=%d, rr=%d\n",masterinfo->rows,maxsize,rr);
#if (WILLUSDEBUGX & 64)
printf("start:  mi->rows=%d, rr=%d\n",masterinfo->rows,rr);
#endif
    /* Enough pixel rows in dest bitmap?  IF so, create an output page */
    if (masterinfo->rows<=0 || masterinfo->rows<rr)
        return(0);

    /* Get a suitable breaking point for the next page */
    bp=masterinfo_break_point(masterinfo,maxsize);
    if (k2settings->verbose)
        printf("bp: maxsize=%d, bp=%d, r0=%d\n",maxsize,bp,r0);
#if (WILLUSDEBUGX & 64)
printf("bp: maxsize=%d, bp=%d, r0=%d\n",maxsize,bp,r0);
#endif
    bmp1->bpp=masterinfo->bmp.bpp;
    for (i=0;i<256;i++)
        bmp1->red[i]=bmp1->green[i]=bmp1->blue[i]=i;
    // h=bp*k2settings->dst_width/masterinfo->bmp.width;
    /*
    ** k2settings->dst_width = full device width in pixels
    ** k2settings->dst_height = full device height in pixels
    ** maxsize = viewable height in pixels (k2settings->dst_height minus margins)
    ** (swapped if -ls)
    */
    /* If too tall, shrink to fit */
#if (WILLUSDEBUGX & 64)
printf("masterinfo->rows=%d\n",masterinfo->rows);
printf("bp=%d, maxsize=%d\n",bp,maxsize);
printf("dst_width=%g\n",(double)k2settings->dst_width);
printf("dst_height=%g\n",(double)k2settings->dst_height);
#endif
    if (bp>maxsize)
        {
        double devht_in;
        lheight=bp;
        ltotheight=(int)((double)k2settings->dst_height*lheight/maxsize+.5);
        // lwidth=(int)((double)masterinfo->bmp.width*lheight/maxsize+.5);
        lwidth=(int)((double)ltotheight*k2settings->dst_width/k2settings->dst_height+.5);
        devht_in=(double)k2settings->dst_height/k2settings->dst_dpi;
        ldpi=(double)ltotheight/devht_in;
        }
    else
        {
        lheight=maxsize; /* useable height */
        ltotheight=k2settings->dst_height;
        lwidth=masterinfo->bmp.width;
        ldpi=k2settings->dst_dpi;
        }
    r0=(int)(ldpi*k2settings->dst_martop+.5);
    bmp1->width=lwidth;
    bmp1->height=lheight;
#if (WILLUSDEBUGX & 64)
printf("bmp1 wxh = %d x %d\n",bmp1->width,bmp1->height);
#endif
    masterinfo->published_pages++;
#if (WILLUSDEBUGX & 64)
printf("mi->published_pages=%d\n",masterinfo->published_pages);
#endif

#ifdef HAVE_MUPDF_LIB
    if (k2settings->use_crop_boxes)
        {
        masterinfo_add_cropbox(masterinfo,k2settings,bmp1,ldpi,bp);
        masterinfo_remove_top_rows(masterinfo,k2settings,bp);
        bmp_free(bmp1);
        return(bp);
        }
#endif /* HAVE_MUPDF_LIB */
    /*
    ** Not using crop boxes, so process output bitmap
    */
    /* Create list of OCR'd words on this page and move */
    /* up positions of remaining OCR'd words.           */
#ifdef HAVE_OCR_LIB
    if (k2settings->dst_ocr)
        {
        for (i=0;i<k2settings->dst_ocrwords.n;i++)
            if (k2settings->dst_ocrwords.word[i].r 
                        - k2settings->dst_ocrwords.word[i].maxheight 
                        + k2settings->dst_ocrwords.word[i].h/2 < bp)
                {
                if (ocrwords!=NULL)
                    ocrwords_add_word((OCRWORDS *)ocrwords,&k2settings->dst_ocrwords.word[i]);
                ocrwords_remove_words(&k2settings->dst_ocrwords,i,i);
                i--;
                }
        }
#endif
        
    /* Center masterinfo->bmp into bmp1 (horizontally) */
    bmp_alloc(bmp1);
    bmp_fill(bmp1,255,255,255);
    {
    int bpp,w1,bw,bw1;
    bpp=bmp1->bpp==24?3:1;
    w1=(bmp1->width-masterinfo->bmp.width)/2;
    bw=bmp_bytewidth(&masterinfo->bmp);
    bw1=w1*bpp;
    for (i=0;i<bp;i++)
        memcpy(bmp_rowptr_from_top(bmp1,i)+bw1,bmp_rowptr_from_top(&masterinfo->bmp,i),bw);
#ifdef HAVE_OCR_LIB
    if (k2settings->dst_ocr && ocrwords!=NULL)
        ocrwords_offset((OCRWORDS *)ocrwords,w1,0);
#endif
    }

    /* Gamma correct */
    if (fabs(k2settings->dst_gamma-1.0)>.001)
        bmp_gamma_correct(bmp1,bmp1,k2settings->dst_gamma);

    /* Sharpen */
    if (k2settings->dst_sharpen)
        {
        WILLUSBITMAP tmp;
        bmp_init(&tmp);
        bmp_copy(&tmp,bmp1);
        bmp_sharpen(bmp1,&tmp);
        bmp_free(&tmp);
        }

    /* Inverse */
    if (k2settings->dst_negative)
        bmp_invert(bmp1);

    /* Pad and mark bmp1 -> bmp */
    bmp_pad_and_mark(bmp,bmp1,k2settings,ltotheight,ldpi,ocrwords);
    bmp_free(bmp1);

    /* Adjust for landscape output if necessary */
    if (k2settings->dst_landscape)
        {
#ifdef HAVE_OCR_LIB
        /* Rotate OCR'd words list */
        if (k2settings->dst_ocr && ocrwords!=NULL)
            {
            OCRWORDS *ocw;

            ocw=(OCRWORDS *)ocrwords;
            for (i=0;i<ocw->n;i++)
                {
                int cnew,rnew;
                ocw->word[i].rot=90;
                cnew = ocw->word[i].r;
                rnew = bmp->width-1 - ocw->word[i].c;
                ocw->word[i].c = cnew;
                ocw->word[i].r = rnew;
                }
            }
#endif
        bmp_rotate_right_angle(bmp,90);
        }

    /* Write sequence of OCR'd words if debugging */
#ifndef K2PDFOPT_KINDLEPDFVIEWER
    if (k2settings->debug)
        {
        char basename[32];
        char opbmpfile[512];
        static int filecount=0;

        sprintf(basename,"outpage%05d.%s",filecount+1,k2settings->jpeg_quality>0?"jpg":"png");
        wfile_fullname(opbmpfile,masterinfo->debugfolder,basename);
        bmp_write(bmp,opbmpfile,stdout,k2settings->jpeg_quality<0?100:k2settings->jpeg_quality);
#ifdef HAVE_OCR_LIB
        if (k2settings->dst_ocr && ocrwords!=NULL)
            {
            FILE *f;
            sprintf(basename,"wordlist%05d.txt",filecount+1);
            f=fopen(basename,"w");
            if (f!=NULL)
                {
                OCRWORDS *ocw;

                ocw=(OCRWORDS *)ocrwords;
                for (i=0;i<ocw->n;i++)
                    fprintf(f,"%s\n",ocw->word[i].text);
                fclose(f);
                }
            }
#endif
        filecount++;
        }
#endif

    /* Reduce bitmap depth */
    if (k2settings->dst_bpc==8 || k2settings->jpeg_quality>=0)
        (*size_reduction)=0;
    else if (k2settings->dst_bpc==4)
        (*size_reduction)=1;
    else if (k2settings->dst_bpc==2)
        (*size_reduction)=2;
    else
        (*size_reduction)=3;
    if (k2settings->dst_dither && k2settings->dst_bpc<8 && k2settings->jpeg_quality<0)
        bmp_dither_to_bpc(bmp,k2settings->dst_bpc);
    (*bmpdpi)=ldpi;
    masterinfo_remove_top_rows(masterinfo,k2settings,bp);
    return(bp);
    }


/*
** Correctly complete crop boxes that are associated with this output page
**
** box->x1,y1,userx,usery start out as:
**     box->x1,y1 = position of upper-left crop box corner in master
**                  bitmap, in pixels.
**     box->userx,usery = width,height of crop box in pixels
**     Original assignments:
**         box->x1 = dw;
**         box->y1 = masterinfo->rows;
**         box->userx = tmp->width;
**         box->usery = tmp->height;
**
** ... but are changed so that...
**
** box->x1 gets the position of the lower-left point of the crop box
**         (when looking at the contents of the crop box right-side up)
**         on the destination page (when holding the device right-side up).
**         The position is the distance from the left side of the device
**         in points (again, when holding the device right-side up).
**
** box->y1 is the same, but it is the distance from the bottom of the
**         device screen, in points, when holding the device right-side up.
**
** box->scale is the scale factor between source and destination sizes.
**
*/
#ifdef HAVE_MUPDF_LIB
static void masterinfo_add_cropbox(MASTERINFO *masterinfo,K2PDFOPT_SETTINGS *k2settings,
                                   WILLUSBITMAP *bmp1,double bmpdpi,int rows)

    {
    int i,dstpageno,r0,r1,height;

    r0=(int)(bmpdpi*k2settings->dst_martop+.5);
    r1=(int)(bmpdpi*k2settings->dst_marbot+.5);
    /* Width already has user-specified device margins factored in, but height doesn't */
    height=bmp1->height+r0+r1;
    dstpageno = masterinfo->published_pages;
#if (WILLUSDEBUGX & 64)
printf("@masterinfo_add_cropbox, nboxes=%d\n",masterinfo->pageinfo.boxes.n);
#endif
    for (i=0;i<masterinfo->pageinfo.boxes.n;i++)
        {
        WPDFBOX *box;
        int w1,xd,yd;

        box=&masterinfo->pageinfo.boxes.box[i];
#if (WILLUSDEBUGX & 64)
printf("    box[%2d]:  x0pts=%g, y0pts=%g, %g x %g\n",i,box->srcbox.x0_pts,box->srcbox.y0_pts,
box->srcbox.crop_width_pts,box->srcbox.crop_height_pts);
printf("              Top = %d, height = %d\n",(int)box->y1,(int)box->usery);
printf("              box->dstpage = %d\n",box->dstpage);
printf("              box->y1 = %g, rows-.1 = %g\n",box->y1,rows-.1);
#endif
        if (box->dstpage>0)
            continue;
        /* If crop box not on this page, skip it */
        if (box->y1 > rows-.1)
            continue;
        /* If crop-box is cut off by this page, split it */
#if (WILLUSDEBUGX & 64)
printf("    box[%d]:  rows(bp)=%d, ix0=%d, iy0=%d, y1=%g, usery=%g, bmp1->ht=%d\n",i,rows,
(int)(box->srcbox.x0_pts*10.+.5),
(int)((box->srcbox.page_height_pts-box->srcbox.y0_pts)*10.+.5),
box->y1,box->usery,bmp1->height);
#endif
        if (box->y1+box->usery > bmp1->height+.1)
            {
            double newheight,dy;
            newheight = box->usery-(rows-box->y1);
#if (WILLUSDEBUGX & 64)
printf("    Splitting box.  New height=%d.\n",(int)newheight);
#endif
            if (newheight>0.5)
                {
                wpdfboxes_insert_box(&masterinfo->pageinfo.boxes,box,i);
                /* Next page: adjust source crop box so we only get the part on */
                /* the next page */
                box=&masterinfo->pageinfo.boxes.box[i];
                box->srcbox.crop_height_pts *= (double)newheight/box->usery;
                box->y1 += (box->usery - newheight);
                box->usery = newheight;
                i++;
                /* This page: adjust source crop box so we only get the part on this page */
                box=&masterinfo->pageinfo.boxes.box[i];
                dy = box->srcbox.crop_height_pts;
                box->srcbox.crop_height_pts *= (double)(rows-box->y1)/box->usery;
                dy -= box->srcbox.crop_height_pts;
                box->srcbox.y0_pts += dy;
                box->usery = rows-box->y1;
                }
            }
#if (WILLUSDEBUGX & 64)
printf("    Adding box %d.\n",i);
#endif
        box->dstpage = dstpageno;
        w1=(bmp1->width-masterinfo->bmp.width)/2;
        xd=w1+box->x1;
        yd=height-(box->y1+box->usery+r0);
        // sr=(int)((box->srcrot_deg+765.)/90.);
        if (k2settings->dst_landscape)
            {
            yd=xd;
            xd=box->y1+box->usery+r0;
            box->x1 = box->dst_width_pts*xd/height;
            box->y1 = box->dst_height_pts*yd/bmp1->width;
            box->dstrot_deg=90.;
            if (box->userx > box->usery)
                box->scale = box->userx*box->dst_height_pts/bmp1->width
                             / box->srcbox.crop_width_pts;
            else
                box->scale = box->usery*box->dst_width_pts/bmp1->height
                             / box->srcbox.crop_height_pts;
            }
        else
            {
            box->x1 = box->dst_width_pts*xd/bmp1->width;
            box->y1 = box->dst_height_pts*yd/height;
            box->dstrot_deg=0.;
            if (box->userx > box->usery)
                box->scale = box->userx*box->dst_width_pts/bmp1->width
                             / box->srcbox.crop_width_pts;
            else
                box->scale = box->usery*box->dst_height_pts/height
                             / box->srcbox.crop_height_pts;
            }
        }
    }
#endif /* HAVE_MUPDF_LIB */


/*
** Give output bitmap appropriate padding and margins (bmp1 -> bmp).
**
** pl, pr, pt, pb are padding specified by user to get correct pixel mapping
**                on a specific device.
**
** r0 = top margin on output device specified by user (dst_martop)
**
** Left margin on output device (dst_marleft) was already taken into account
**     in bmp_src_to_dst().
**
*/
static void bmp_pad_and_mark(WILLUSBITMAP *dst,WILLUSBITMAP *src,K2PDFOPT_SETTINGS *k2settings,
                             int ltotheight,double bmpdpi,void *ocrwords)

    {
    int i,r,r0,bw,bytespp,pl,pr,pt,pb;

    r0=(int)(bmpdpi*k2settings->dst_martop+.5);
    if (k2settings->dst_landscape)
        {
        pl=k2settings->pad_bottom;
        pr=k2settings->pad_top;
        pt=k2settings->pad_left;
        pb=k2settings->pad_right;
        }
    else
        {
        pb=k2settings->pad_bottom;
        pt=k2settings->pad_top;
        pl=k2settings->pad_left;
        pr=k2settings->pad_right;
        }
    dst->bpp=src->bpp;
    for (i=0;i<256;i++)
        dst->red[i]=dst->green[i]=dst->blue[i]=i;
    dst->width=src->width+pl+pr;
    dst->height=ltotheight+pt+pb;
    bmp_alloc(dst);
    bmp_fill(dst,255,255,255);
    bw=bmp_bytewidth(src);
    bytespp=dst->bpp==8?1:3;
    for (r=0;r<src->height && r+r0+pt<dst->height;r++)
        {
        unsigned char *psrc,*pdst;
        psrc=bmp_rowptr_from_top(src,r);
        pdst=bmp_rowptr_from_top(dst,r+r0+pt)+pl*bytespp;
        memcpy(pdst,psrc,bw);
        }
#ifdef HAVE_OCR_LIB
    if (k2settings->dst_ocr && ocrwords!=NULL)
        ocrwords_offset((OCRWORDS *)ocrwords,pl,r0+pt);
#endif
    /* Can't save grayscale as JPEG yet. */
    if (dst->bpp==8 && k2settings->jpeg_quality>=0)
        bmp_promote_to_24(dst);
    if (k2settings->mark_corners)
        {
        unsigned char *p;

        if (pt<dst->height)
            {
            p=bmp_rowptr_from_top(dst,pt);
            if (pl<dst->width)
                p[pl]=0;
            if (pr<dst->width)
                p[dst->width-1-pr]=0;
            }
        if (pb<dst->height)
            {
            p=bmp_rowptr_from_top(dst,dst->height-1-pb);
            if (pl<dst->width)
                p[pl]=0;
            if (pr<dst->width)
                p[dst->width-1-pr]=0;
            }
        }
    }


void masterinfo_add_gap(MASTERINFO *masterinfo,K2PDFOPT_SETTINGS *k2settings,double inches)

    {
    int n,bw;
    unsigned char *p;

    n=(int)(inches*k2settings->dst_dpi+.5);
    if (n<1)
        n=1;
    while (masterinfo->rows+n > masterinfo->bmp.height)
        bmp_more_rows(&masterinfo->bmp,1.4,255);
    bw=bmp_bytewidth(&masterinfo->bmp)*n;
    p=bmp_rowptr_from_top(&masterinfo->bmp,masterinfo->rows);
    memset(p,255,bw);
    masterinfo->rows += n;
    }


/*
** Spread words out in src and put into jbmp at scaling nocr
** In case the text can't be expanded enough,
**     just=0 (left justify), 1 (center), 2 (right justify)
*/
static void bmp_fully_justify(WILLUSBITMAP *jbmp,WILLUSBITMAP *src,
                              K2PDFOPT_SETTINGS *k2settings,int jbmpwidth,
                              int whitethresh,int just,int dpi)

    {
    BMPREGION srcregion;
    BREAKINFO *colbreaks,_colbreaks;
    WILLUSBITMAP gray;
    int *gappos,*gapsize;
    int i,srcbytespp,srcbytewidth,jbmpbytewidth,newwidth,destx0,ng;
    static char *funcname="bmp_fully_justify";

/*
{
char filename[256];
count++;
sprintf(filename,"out%03d.png",count);
bmp_write(src,filename,stdout,100);
}
*/
    /* Init/allocate destination bitmap */
    jbmp->width = jbmpwidth;
    jbmp->height = src->height;
    jbmp->bpp = src->bpp;
    if (jbmp->bpp==8)
        for (i=0;i<256;i++)
            jbmp->red[i]=jbmp->green[i]=jbmp->blue[i]=i;
    bmp_alloc(jbmp);

    /* Find breaks in the text row */
    colbreaks=&_colbreaks;
    colbreaks->textrow=NULL;
    srcregion.bgcolor=whitethresh;
    srcregion.c1=0;
    srcregion.c2=src->width-1;
    srcregion.r1=0;
    srcregion.r2=src->height-1;
    srcregion.dpi=dpi;
    srcbytespp=src->bpp==24 ? 3 : 1;
    if (srcbytespp==3)
        {
        srcregion.bmp=src;
        srcregion.bmp8=&gray;
        bmp_init(srcregion.bmp8);
        bmp_convert_to_greyscale_ex(srcregion.bmp8,src);
        }
    else
        {
        srcregion.bmp=src;
        srcregion.bmp8=src;
        }
    breakinfo_alloc(103,colbreaks,src->width);
    {
    int *colcount,*rowcount;

    colcount=rowcount=NULL;
    willus_dmem_alloc_warn(8,(void **)&colcount,sizeof(int)*(src->width+src->height),funcname,10);
    rowcount=&colcount[src->width];
    bmpregion_one_row_find_breaks(&srcregion,colbreaks,k2settings,colcount,rowcount,1);
    willus_dmem_free(8,(double **)&colcount,funcname);
    }
    if (srcbytespp==3)
        bmp_free(srcregion.bmp8);
    ng=colbreaks->n-1;
    gappos=NULL;
    if (ng>0)
        {
        int maxsize,ms2,mingap,j;

        willus_dmem_alloc_warn(9,(void **)&gappos,(2*sizeof(int))*ng,funcname,10);
        gapsize=&gappos[ng];
        for (i=0;i<ng;i++)
            {
            gappos[i]=colbreaks->textrow[i].c2+1;
            gapsize[i]=colbreaks->textrow[i].gap;
            }
        
        /* Take only the largest group of gaps */
        for (maxsize=i=0;i<ng;i++)
            if (maxsize<gapsize[i])
                maxsize=gapsize[i];
        mingap = srcregion.lcheight*k2settings->word_spacing;
        if (mingap < 2)
            mingap = 2; 
        if (maxsize > mingap)
            maxsize = mingap;
        ms2 = maxsize/2;
        for (i=j=0;i<ng;i++)
            if (gapsize[i] > ms2)
                {
                if (j!=i)
                    {
                    gapsize[j]=gapsize[i];
                    gappos[j]=gappos[i];
                    }
                j++;
                }
        ng=j;

        /* Figure out total pixel expansion */
        newwidth = src->width*1.25;
        if (newwidth > jbmp->width)
            newwidth=jbmp->width;
        }
    else
        newwidth=src->width;
    breakinfo_free(103,colbreaks);

    /* Starting column in destination bitmap */
    if (just==1)
        destx0 = (jbmp->width-newwidth)/2;
    else if (just==2)
        destx0 = (jbmp->width-newwidth);
    else
        destx0 = 0;

    jbmpbytewidth = bmp_bytewidth(jbmp);
    srcbytewidth = bmp_bytewidth(src);

    /* Clear entire fully justified bitmap */
    memset(bmp_rowptr_from_top(jbmp,0),255,jbmpbytewidth*jbmp->height);

    /* Spread out source pieces to fully justify them */
    for (i=0;i<=ng;i++)
        {
        int j,dx0,dx,sx0;
        unsigned char *pdst,*psrc;

        dx = i<ng ? (i>0 ? gappos[i]-gappos[i-1] : gappos[i]+1)
                  : (i>0 ? src->width-(gappos[i-1]+1) : src->width);
        dx *= srcbytespp;
        sx0= i==0 ? 0 : (gappos[i-1]+1);
        dx0= destx0 + sx0 + (i==0 ? 0 : (newwidth-src->width)*i/ng);
        psrc=bmp_rowptr_from_top(src,0)+sx0*srcbytespp;
        pdst=bmp_rowptr_from_top(jbmp,0)+dx0*srcbytespp;
        for (j=0;j<src->height;j++,pdst+=jbmpbytewidth,psrc+=srcbytewidth)
            memcpy(pdst,psrc,dx);
        }
    if (gappos!=NULL)
        willus_dmem_free(9,(double **)&gappos,funcname);
    }


/*
** Find gaps in the master bitmap so that it can be broken into regions
** which go onto separate pages.
*/
static int masterinfo_break_point(MASTERINFO *masterinfo,int maxsize)

    {
    static char *funcname="break_point";
    int *rowcount;
    int fig,fc,figend,cw,bp1,bp2,i,j,goodsize,figure,bp,scanheight,nwc;
    int bp1f,bp2f;
    int bp1e,bp2e;

/*
printf("@breakpoint, mi->rows=%d, maxsize=%d\n",masterinfo->rows,maxsize);
*/
    /* masterinfo->fit_to_page==-2 means user specified -f2p -2 which means */
    /* flush entire contents of master to single page every time.   */
    if (masterinfo->rows<maxsize || masterinfo->fit_to_page==-2)
        return(masterinfo->rows);

    /* scanheight tells how far down the master bitmap to scan */
    if (masterinfo->fit_to_page==-1)
        scanheight=masterinfo->rows;
    else if (masterinfo->fit_to_page>0)
        scanheight=(int)(((1.+masterinfo->fit_to_page/100.)*maxsize)+.5);
    else
        scanheight=maxsize;
    /* If available rows almost exactly fit page, just send the whole thing */
/*
printf("    scanheight=%d, mi->rows=%d, fabs=%g\n",scanheight,masterinfo->rows,
fabs((double)scanheight/masterinfo->rows-1.));
*/
    if (abs(scanheight-masterinfo->rows)<=1
             || fabs((double)scanheight/masterinfo->rows-1.)<.002)
        return(masterinfo->rows);
    if (scanheight > masterinfo->rows)
        scanheight=masterinfo->rows;
    goodsize=masterinfo->bmp.width/100;
    figure=masterinfo->bmp.width/10;
    willus_dmem_alloc_warn(29,(void **)&rowcount,masterinfo->rows*sizeof(int),funcname,10);
    for (j=0;j<masterinfo->rows;j++)
        {
        unsigned char *p;
        p=bmp_rowptr_from_top(&masterinfo->bmp,j);
        rowcount[j]=0;
        if (masterinfo->bmp.bpp==24)
            {
            for (i=0;i<masterinfo->bmp.width;i++,p+=3)
                if (GRAYLEVEL(p[0],p[1],p[2])<masterinfo->bgcolor)
                    rowcount[j]++;
            }
        else
            {
            for (i=0;i<masterinfo->bmp.width;i++,p++)
                if (p[0]<masterinfo->bgcolor)
                    rowcount[j]++;
            }
        }
    /* cw = consecutive all-white rows */
    /* fc = consecutive non-all-white rows */
    bp1f=bp2f=0; /* bp1f,bp2f = max break points that fit within maxsize */
    bp1e=bp2e=0; /* bp1e,bp2e = min break points that exceed maxsize */
    for (figend=fc=fig=cw=i=bp=bp1=bp2=nwc=0;i<scanheight;i++)
        {
        if (rowcount[i]==0)
            {
// if (cw==0)
// printf("%d black\n",fc);
            cw++;
            if (fc>figure)
                {
                fig=i-fc;
                figend=i;
                }
            fc=0;
            if (fig && i-figend > fc/2)
                fig=0;
            if (fig)
                continue;
            if (nwc==0)
                continue;
            bp1=i-cw/2;
            if (bp1<=maxsize)
                bp1f=bp1;
            if (bp1>maxsize && bp1e==0)
                bp1e=bp1;
            if (cw>=goodsize)
                {
                bp2=i-cw/2;
                if (bp2<=maxsize)
                    bp2f=bp2;
                if (bp2>maxsize && bp2e==0)
                    bp2e=bp2;
                }
            }
        else
            {
// if (fc==0)
// printf("%d white\n",cw);
            cw=0;
            nwc++;
            fc++;
            }
        }
/*
{
static int count=0;
FILE *out;
count++;
printf("rows=%d, gs=%d, scanheight=%d, bp1=%d, bp2=%d\n",masterinfo->rows,goodsize,scanheight,bp1,bp2);
printf("     bp1f=%d, bp2f=%d, bp1e=%d, bp2e=%d\n",bp1f,bp2f,bp1e,bp2e);
bmp_write(&masterinfo->bmp,"master.png",stdout,100);
out=fopen("rc.dat","w");
for (i=0;i<scanheight;i++)
fprintf(out,"%d\n",rowcount[i]);
fclose(out);
if (count==2)
exit(10);
}
*/
    willus_dmem_free(29,(double **)&rowcount,funcname);
    if (masterinfo->fit_to_page==0)
        {
        if (bp2 > maxsize*.8)
            return(bp2);
        if (bp1 < maxsize*.25)
            bp1=scanheight;
        return(bp1);
        }
    if (bp1f==0 && bp1e==0)
        return(scanheight);
    if (bp2f > 0)
        return((bp1f>0 && bp2f < maxsize*.8) ? bp1f : bp2f);
    if (bp1f > 0)
        return(bp1f);
    if (masterinfo->fit_to_page<0)
        return(bp1e);
    if (bp2e > 0)
        return(bp2e);
    return(bp1e);
    }
