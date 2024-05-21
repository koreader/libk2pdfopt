/*
** k2bmp.c      Bitmap manipulation functions for k2pdfopt.  These routines
**              are mostly generic bitmap functions, but there are some
**              k2pdfopt-specific settings for some.
**
** Copyright (C) 2018  http://willus.com
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

static int inflection_count(double *x,int n,int delta,int *wthresh);
static int vert_line_erase(WILLUSBITMAP *bmp,WILLUSBITMAP *cbmp,WILLUSBITMAP *tmp,
                    int row0,int col0,double tanth,double minheight_in,
                    /*double minwidth_in,*/ double maxwidth_in,int white_thresh,
                    double dpi,int erase_vertical_lines);
static int gscale(unsigned char *p);
static int not_close(int c1,int c2);
static int bmp_autocrop2_ex(WILLUSBITMAP *bmp,int pixwidth,int pixstep,int whitethresh,
                            double blackweight,double minarea,double threshold,int *cx);
static void bmp_autocrop_refine(WILLUSBITMAP *bmp,int whitethresh,double threshold,int *cx,
                                int pixwidth);
static double find_threshold(double *x,double *y,int n,double threshold);
static void xsmooth(double *y,int n,int cwin);
static double frame_area(double area,int *cx);
static void bmp_convert_to_monochrome(WILLUSBITMAP *bmp,int whitethresh);
static double frame_stdev_norm(WILLUSBITMAP *bmp,int *cx,int flags);
static double frame_black_percentage(WILLUSBITMAP *bmp,int *cx,int flags);
static void k2pagebreakmarks_add_mark(K2PAGEBREAKMARKS *k2pagebreakmarks,int markcol,int markrow,
                                      int marktype,int dpi);
static int k2pagebreakmarks_too_close_to_others(K2PAGEBREAKMARKS *k2pagebreakmarks,int markcol,
                                                int markrow,int dpi);


int bmp_get_one_document_page(WILLUSBITMAP *src,K2PDFOPT_SETTINGS *k2settings,
                              int src_type,char *filename,
                              int pageno,double dpi,int bpp,FILE *out)

    {
    int status;

    /* v2.34--read PS file correctly */
    if (src_type==SRC_TYPE_PDF || src_type==SRC_TYPE_PS)
        {
#ifdef HAVE_MUPDF_LIB
        static char *mupdferr_trygs=TTEXT_WARN "\a\n ** ERROR reading from " TTEXT_BOLD2 "%s" TTEXT_WARN "using MuPDF.  Trying Ghostscript...\n\n" TTEXT_NORMAL;

        status=0;
        if (src_type==SRC_TYPE_PDF && k2settings->usegs<=0)
            {
#if (WILLUSDEBUGX & 0x80000000)
printf("\a\a\n\n\n\n\n\n\n\n\n   ****** FAKING MUPDF--IGNORING SOURCE DOCUMENT!!  *****\n\n\n\n\n\n\n");
status=bmp_read(src,"out.png",NULL);
if (status==0 && bpp!=24)
bmp_convert_to_grayscale(src);
return(status);
#else
            status=bmpmupdf_pdffile_to_bmp(src,filename,pageno,dpi*k2settings->document_scale_factor,bpp);
            if (!status || k2settings->usegs<0)
                return(status);
#endif
            }
        /* Switch to Postscript since MuPDF failed */
        if (src_type!=SRC_TYPE_PS && k2settings->usegs==0)
            {
            k2printf(mupdferr_trygs,filename);
            k2settings->usegs=1;
            }
#endif
#ifdef HAVE_GHOSTSCRIPT
        if (willusgs_init(stdout) < 0)
            {
            k2sys_enter_to_exit(k2settings);
            exit(20);
            }
#endif
        bmp_set_pdf_pageno(pageno);
        bmp_set_pdf_dpi(dpi);
        /*
        k2printf("Converting " TTEXT_BOLD2 "%s" TTEXT_NORMAL 
            " page %2d to %d dpi bitmap ... ",filename,i,dpi);
        fflush(stdout);
        */
        status=bmp_read(src,filename,NULL);
        if (!status && bpp==8)
            bmp_convert_to_greyscale(src);
        return(status);
        }
    else
#ifdef HAVE_DJVU_LIB
    if (src_type==SRC_TYPE_DJVU)
        return(bmpdjvu_djvufile_to_bmp(src,filename,pageno,dpi*k2settings->document_scale_factor,bpp,out));
    else
#endif
    /* v2.34--read bitmap correctly */
    status=bmp_read(src,filename,NULL);
    if (!status && bpp==8)
        bmp_convert_to_greyscale(src);
    return(status);
    }


void k2bmp_erode(WILLUSBITMAP *src,WILLUSBITMAP *srcgrey,
                 K2PDFOPT_SETTINGS *k2settings)
    {
    int i,n;

    n=abs(k2settings->src_erosion);
    for (i=0;i<n;i++)
        bmp_erode(srcgrey,srcgrey);
    if (src!=srcgrey && src!=NULL && src->bpp>8)
        for (i=0;i<n;i++)
            bmp_erode(src,src);
    }


void bmp_adjust_contrast(WILLUSBITMAP *src,WILLUSBITMAP *srcgrey,
                         K2PDFOPT_SETTINGS *k2settings,int *white)

    {
    int i,j,tries,wc,tc,hist[256];
    double contrast,rat0;
    WILLUSBITMAP *dst,_dst;

    if (k2settings->debug && k2settings->verbose)
        k2printf("\nAt adjust_contrast.\n");
    if ((*white) <= 0)
        (*white)=192;
    /* If contrast_max negative, use it as fixed contrast adjustment. */
    if (k2settings->contrast_max < 0.)
        {
        bmp_contrast_adjust(srcgrey,srcgrey,-k2settings->contrast_max);
        if (k2settings->dst_color && src!=srcgrey && src!=NULL && src->bpp>8
                                  && fabs(k2settings->contrast_max+1.0)>1e-4)
            bmp_contrast_adjust(src,src,-k2settings->contrast_max);
        return;
        }
    dst=&_dst;
    bmp_init(dst);
    wc=0; /* Avoid compiler warning */
    tc=srcgrey->width*srcgrey->height;
    rat0=0.5; /* Avoid compiler warning */
    for (contrast=1.0,tries=0;contrast<k2settings->contrast_max+.01;tries++)
        {
        if (fabs(contrast-1.0)>1e-4)
            bmp_contrast_adjust(dst,srcgrey,contrast);
        else
            bmp_copy(dst,srcgrey);
        /*Get bitmap histogram */
        for (i=0;i<256;i++)
            hist[i]=0;
        for (j=0;j<dst->height;j++)
            {
            unsigned char *p;
            p=bmp_rowptr_from_top(dst,j);
            for (i=0;i<dst->width;i++,p++)
                hist[p[0]]++;
            }
        if (tries==0)
            {
            int h1;
            for (h1=0,j=(*white);j<256;j++)
                h1+=hist[j];
            rat0=(double)h1/tc;
            if (k2settings->debug && k2settings->verbose)
                k2printf("    rat0 = rat[%d-255]=%.4f\n",(*white),rat0);
            }
        
        /* Find white ratio */
        /*
        for (wc=hist[254],j=253;j>=252;j--)
            if (hist[j]>wc1)
                wc1=hist[j];
        */
        for (wc=0,j=252;j<=255;j++)
            wc += hist[j];
        /*
        if ((double)wc/tc >= rat0*0.7 && (double)hist[255]/wc > 0.995)
            break;
        */
        if (k2settings->debug && k2settings->verbose)
            k2printf("    %2d. Contrast=%7.2f, rat[252-255]/rat0=%.4f\n",
                        tries+1,contrast,(double)wc/tc/rat0);
        if ((double)wc/tc >= rat0*0.94)
            break;
        contrast *= 1.05;
        }
    if (k2settings->debug)
        k2printf("Contrast=%7.2f, rat[252-255]/rat0=%.4f\n",
                       contrast,(double)wc/tc/rat0);
/*
bmp_write(dst,"outc.png",stdout,100);
wfile_written_info("outc.png",stdout);
exit(10);
*/
    bmp_copy(srcgrey,dst);
    /* Maybe don't adjust the contrast for the color bitmap? */
    if (k2settings->dst_color && src!=srcgrey && src!=NULL && src->bpp>8
                              && fabs(contrast-1.0)>1e-4)
        bmp_contrast_adjust(src,src,contrast);
    bmp_free(dst);
    }



/*
** src is only allocated if dst_color != 0
*/
void bmp_clear_outside_crop_border(MASTERINFO *masterinfo,WILLUSBITMAP *src,
                                   WILLUSBITMAP *srcgrey,K2PDFOPT_SETTINGS *k2settings)

    {
    int i,n,bytes_per_pix;
    BMPREGION *region,_region;

    region=&_region;
    bmpregion_init(region);
    bytes_per_pix = src==NULL ? 0 : src->bpp>>8;
    region->bmp = (src!=NULL && src->bpp>8) ? src : srcgrey;
    region->bmp8 = srcgrey;
    region->dpi = k2settings->src_dpi;
    bmpregion_trim_to_crop_margins(region,masterinfo,k2settings);
    n=region->c1;
    for (i=0;i<srcgrey->height;i++)
        {
        unsigned char *p;
        if (src!=NULL && src != srcgrey)
            {
            p=bmp_rowptr_from_top(src,i);
            memset(p,255,n*bytes_per_pix);
            }
        p=bmp_rowptr_from_top(srcgrey,i);
        memset(p,255,n);
        }
    n=srcgrey->width-1-region->c2;
    for (i=0;i<srcgrey->height;i++)
        {
        unsigned char *p;
        if (src!=NULL && src != srcgrey)
            {
            p=bmp_rowptr_from_top(src,i)+bytes_per_pix*(src->width-n);
            memset(p,255,n*bytes_per_pix);
            }
        p=bmp_rowptr_from_top(srcgrey,i)+srcgrey->width-n;
        memset(p,255,n);
        }
    n=region->r1;
    for (i=0;i<n;i++)
        {
        unsigned char *p;
        if (src!=NULL && src != srcgrey)
            {
            p=bmp_rowptr_from_top(src,i);
            memset(p,255,src->width*bytes_per_pix);
            }
        p=bmp_rowptr_from_top(srcgrey,i);
        memset(p,255,srcgrey->width);
        }
    n=srcgrey->height-1-region->r2;
    for (i=srcgrey->height-n;i<srcgrey->height;i++)
        {
        unsigned char *p;
        if (src!=NULL && src != srcgrey)
            {
            p=bmp_rowptr_from_top(src,i);
            memset(p,255,src->width*bytes_per_pix);
            }
        p=bmp_rowptr_from_top(srcgrey,i);
        memset(p,255,srcgrey->width);
        }
    /* Free region--v2.15 memory leak fix, 2-1-14 */
    bmpregion_free(region);
    }


/*
** bmp_orientation()
**
** Looks for rows of text to determine document orientation.
** (Rows of alternating darker and lighter regions.)
**
** 1.0 means neutral
**
** >> 1.0 means document is likely portrait (no rotation necessary)
**    (max is 100.)
**
** << 1.0 means document is likely landscape (need to rotate it)
**    (min is 0.01)
**
*/
double bmp_orientation(WILLUSBITMAP *bmp)

    {
    int i,ic,wtcalc;
    double hsum,vsum,rat;

    wtcalc=-1;
    for (vsum=0.,hsum=0.,ic=0,i=20;i<=85;i+=5,ic++)
        {
        double nv,nh;
        int wth,wtv;

#ifdef DEBUG
k2printf("h %d:\n",i);
#endif
        if (ic==0)
            wth=-1;
        else
            wth=wtcalc;
wth=-1;
        nh=bmp_inflections_horizontal(bmp,8,i,&wth);
#ifdef DEBUG
{
FILE *f;
f=fopen("inf.ep","a");
fprintf(f,"/ag\n");
fclose(f);
}
k2printf("v %d:\n",i);
#endif
        if (ic==0)
            wtv=-1;
        else
            wtv=wtcalc;
wtv=-1;
        nv=bmp_inflections_vertical(bmp,8,i,&wtv);
        if (ic==0)
            {
            if (wtv > wth)
                wtcalc=wtv;
            else
                wtcalc=wth;
            continue;
            }
// exit(10);
        hsum += nh*i*i*i;
        vsum += nv*i*i*i;
        }
    if (vsum==0. && hsum==0.)
        rat=1.0;
    else if (hsum<vsum && hsum/vsum<.01)
        rat=100.;
    else
        rat=vsum/hsum;
    if (rat < .01)
        rat = .01;
    // k2printf("    page %2d:  %8.4f\n",pagenum,rat);
    // fprintf(out,"\t%8.4f",vsum/hsum);
    // fprintf(out,"\n");
    return(rat);
    }


double bmp_inflections_vertical(WILLUSBITMAP *srcgrey,int ndivisions,int delta,int *wthresh)

    {
    int y0,y1,ny,i,nw,nisum,ni,wt,wtmax;
    double *g;
    char *funcname="bmp_inflections_vertical";

    nw=srcgrey->width/ndivisions;
    y0=srcgrey->height/6;
    y1=srcgrey->height-y0;
    ny=y1-y0;
    willus_dmem_alloc_warn(21,(void **)&g,ny*sizeof(double),funcname,10);
    wtmax=-1;
    for (nisum=0,i=0;i<10;i++)
        {
        int x0,x1,nx,j;

        x0=(srcgrey->width-nw)*(i+2)/13;
        x1=x0+nw;
        if (x1>srcgrey->width)
            x1=srcgrey->width;
        nx=x1-x0; 
        for (j=y0;j<y1;j++)
            {
            int k,rsum;
            unsigned char *p;

            p=bmp_rowptr_from_top(srcgrey,j)+x0;
            for (rsum=k=0;k<nx;k++,p++)
                rsum+=p[0];
            g[j-y0]=(double)rsum/nx;
            }
        wt=(*wthresh);
        ni=inflection_count(g,ny,delta,&wt);
        if ((*wthresh)<0 && ni>=3 && wt>wtmax)
            wtmax=wt;
        if (ni>nisum)
            nisum=ni;
        }
    willus_dmem_free(21,&g,funcname);
    if ((*wthresh)<0)
        (*wthresh)=wtmax;
    return(nisum);
    }


double bmp_inflections_horizontal(WILLUSBITMAP *srcgrey,int ndivisions,int delta,int *wthresh)

    {
    int x0,x1,nx,bw,i,nh,nisum,ni,wt,wtmax;
    double *g;
    char *funcname="bmp_inflections_vertical";

    nh=srcgrey->height/ndivisions;
    x0=srcgrey->width/6;
    x1=srcgrey->width-x0;
    nx=x1-x0;
    bw=bmp_bytewidth(srcgrey);
    willus_dmem_alloc_warn(22,(void **)&g,nx*sizeof(double),funcname,10);
    wtmax=-1;
    for (nisum=0,i=0;i<10;i++)
        {
        int y0,y1,ny,j;

        y0=(srcgrey->height-nh)*(i+2)/13;
        y1=y0+nh;
        if (y1>srcgrey->height)
            y1=srcgrey->height;
        ny=y1-y0; 
        for (j=x0;j<x1;j++)
            {
            int k,rsum;
            unsigned char *p;

            p=bmp_rowptr_from_top(srcgrey,y0)+j;
            for (rsum=k=0;k<ny;k++,p+=bw)
                rsum+=p[0];
            g[j-x0]=(double)rsum/ny;
            }
        wt=(*wthresh);
        ni=inflection_count(g,nx,delta,&wt);
        if ((*wthresh)<0 && ni>=3 && wt>wtmax)
            wtmax=wt;
        if (ni>nisum)
            nisum=ni;
        }
    willus_dmem_free(22,&g,funcname);
    if ((*wthresh)<0)
        (*wthresh)=wtmax;
    return(nisum);
    }


static int inflection_count(double *x,int n,int delta,int *wthresh)

    {
    int i,i0,ni,ww,c,ct,wt,mode;
    double meandi,meandisq,f1,f2,stdev;
    double *xs;
    static int *hist;
    static char *funcname="inflection_count";

    /* Allocate memory for hist[] array rather than using static array */
    /* v2.13 fix */
    willus_dmem_alloc_warn(34,(void **)&hist,sizeof(int)*256,funcname,10);
    /* Find threshold white value that peaks must exceed */
    if ((*wthresh)<0)
        {
        for (i=0;i<256;i++)
            hist[i]=0;
        for (i=0;i<n;i++)
            {
            i0=floor(x[i]);
            if (i0>255)
                i0=255;
            hist[i0]++;
            }
        ct=n*.15;
        for (c=0,i=255;i>=0;i--)
            {
            c+=hist[i];
            if (c>ct)
                break;
            }
        wt=i-10;
        if (wt<192)
            wt=192;
/*
#ifdef DEBUG
k2printf("wt=%d\n",wt);
#endif
*/
        (*wthresh)=wt;
        }
    else
        wt=(*wthresh);
    willus_dmem_free(34,(double **)&hist,funcname);
    ww=n/150;
    if (ww<1)
        ww=1;
    willus_dmem_alloc_warn(23,(void **)&xs,sizeof(double)*n,funcname,10);
    for (i=0;i<n-ww;i++)
        {
        int j;
        for (xs[i]=0.,j=0;j<ww;j++,xs[i]+=x[i+j]);
        xs[i] /= ww;
        }
    meandi=meandisq=0.;
    if (xs[0]<=wt-delta)
        mode=1;
    else if (xs[0]>=wt)
        mode=-1;
    else
        mode=0;
    for (i0=0,ni=0,i=1;i<n-ww;i++)
        {
        if (mode==1 && xs[i]>=wt)
            {
            if (i0>0)
                {
                meandi+=i-i0;
                meandisq+=(i-i0)*(i-i0);
                ni++;
                }
            i0=i;
            mode=-1;
            continue;
            }
        if (xs[i]<=wt-delta)
            mode=1;
        }
    stdev = 1.0; /* Avoid compiler warning */
    if (ni>0)
        {
        meandi /= ni;
        meandisq /= ni;
        stdev = sqrt(fabs(meandi*meandi-meandisq));
        }
    f1=meandi/n;
    if (f1>.15)
        f1=.15;
    if (ni>2)
        {
        if (stdev/meandi < .05)
            f2=20.;
        else
            f2=meandi/stdev;
        }
    else
        f2=1.;
#ifdef DEBUG
k2printf("    ni=%3d, f1=%8.4f, f2=%8.4f, f1*f2*ni=%8.4f\n",ni,f1,f2,f1*f2*ni);
{
static int count=0;
FILE *f;
int i;
f=fopen("inf.ep",count==0?"w":"a");
count++;
fprintf(f,"/sa l \"%d\" 1\n",ni);
for (i=0;i<n-ww;i++)
fprintf(f,"%g\n",xs[i]);
fprintf(f,"//nc\n");
fclose(f);
}
#endif /* DEBUG */
    willus_dmem_free(23,&xs,funcname);
    return(f1*f2*ni);
    }

/*
** Detect horizontal lines by making them vertical first.
*/
void bmp_detect_horizontal_lines(WILLUSBITMAP *bmp,WILLUSBITMAP *cbmp,
                                 double dpi,/* double minwidth_in, */
                                 double maxthick_in,double minwidth_in,double anglemax_deg,
                                 int white_thresh,int erase_horizontal_lines,int debug,int verbose)

    {
    bmp_rotate_right_angle(bmp,90);
    if (cbmp!=NULL && cbmp!=bmp)
        bmp_rotate_right_angle(cbmp,90);
    bmp_detect_vertical_lines(bmp,cbmp,dpi,maxthick_in,minwidth_in,anglemax_deg,
                              white_thresh,erase_horizontal_lines,debug,verbose);
    if (cbmp!=NULL && cbmp!=bmp)
        bmp_rotate_right_angle(cbmp,-90);
    bmp_rotate_right_angle(bmp,-90);
    }


/*
** bmp must be grayscale! (cbmp might be color, might be grayscale, can be null)
** Handles cbmp either 8-bit or 24-bit in v2.10.
*/
void bmp_detect_vertical_lines(WILLUSBITMAP *bmp,WILLUSBITMAP *cbmp,
                               double dpi,/* double minwidth_in, */
                               double maxwidth_in,double minheight_in,double anglemax_deg,
                               int white_thresh,int erase_vertical_lines,int debug,int verbose)

    {
    int tc,iangle,irow,icol;
    int rowstep,na,angle_sign,ccthresh;
    int halfwidth,bytewidth;
    int bs1,nrsteps;
    double anglestep;
    WILLUSBITMAP *tmp,_tmp;
    unsigned char *p0;
    unsigned char *t0;

    if (debug)
        k2printf("At bmp_detect_vertical_lines...\n");
    if (!bmp_is_grayscale(bmp))
        {
        k2printf("Internal error.  bmp_detect_vertical_lines passed a non-grayscale bitmap.\n");
        exit(10);
        }
    tmp=&_tmp;
    bmp_init(tmp);
    bmp_copy(tmp,bmp);
    p0=bmp_rowptr_from_top(bmp,0);
    t0=bmp_rowptr_from_top(tmp,0);
    bytewidth=bmp_bytewidth(bmp);
    /*
    pixmin = (int)(minwidth_in*dpi+.5);
    if (pixmin<1)
        pixmin=1;
    halfwidth=pixmin/4;
    if (halfwidth<1)
        halfwidth=1; 
    */
    halfwidth=1;
    anglestep=atan2((double)halfwidth/dpi,minheight_in);
    na=(int)((anglemax_deg*PI/180.)/anglestep+.5);
    if (na<1)
        na=1;
    rowstep=(int)(dpi/40.+.5);
    if (rowstep<2)
        rowstep=2;
    nrsteps=bmp->height/rowstep;
    bs1=bytewidth*rowstep;
    ccthresh=(int)(minheight_in*dpi/rowstep+.5);
    if (ccthresh<2)
        ccthresh=2;
    if (debug && verbose)
        k2printf("    na = %d, rowstep = %d, ccthresh = %d, white_thresh = %d, nrsteps=%d\n",na,rowstep,ccthresh,white_thresh,nrsteps);
/*
bmp_write(bmp,"out.png",stdout,97);
wfile_written_info("out.png",stdout);
*/
    for (tc=0;tc<100;tc++)
        {
        int ccmax,ic0max,ir0max;
        double tanthmax;

        ccmax=-1;
        ic0max=ir0max=0;
        tanthmax=0.;
        for (iangle=0;iangle<=na;iangle++)
            {
            for (angle_sign=1;angle_sign>=-1;angle_sign-=2)
                {
                double th,tanth,tanthx;
                int ic1,ic2;

                if (iangle==0 && angle_sign==-1)
                    continue;
                th=(PI/180.)*iangle*angle_sign*fabs(anglemax_deg)/na;
                tanth=tan(th);
                tanthx=tanth*rowstep;
                if (angle_sign==1)
                    {
                    ic1=-(int)(bmp->height*tanth+1.);
                    ic2=bmp->width-1;
                    }
                else
                    {
                    ic1=(int)(-bmp->height*tanth+1.);
                    ic2=bmp->width-1+(int)(-bmp->height*tanth+1.);
                    }
// k2printf("iangle=%2d, angle_sign=%2d, ic1=%4d, ic2=%4d\n",iangle,angle_sign,ic1,ic2);
                for (icol=ic1;icol<=ic2;icol++)
                    {
                    unsigned char *p,*t;
                    int cc,ic0,ir0;
                    p=p0;
                    t=t0;
                    if (icol<0 || icol>bmp->width-1)
                        for (irow=0;irow<nrsteps;irow++,p+=bs1,t+=bs1)
                            {
                            int ic;
                            ic=icol+irow*tanthx;
                            if (ic>=0 && ic<bmp->width)
                                break;
                            }
                    else
                        irow=0;
                    for (ir0=ic0=cc=0;irow<nrsteps;irow++,p+=bs1,t+=bs1)
                        {
                        int ic;
                        ic=icol+irow*tanthx;
                        if (ic<0 || ic>=bmp->width)
                            break;
                        if ((p[ic]<white_thresh || p[ic+bytewidth]<white_thresh)
                            && (t[ic]<white_thresh || t[ic+bytewidth]<white_thresh))
                            {
                            if (cc==0)
                                {
                                ic0=ic;
                                ir0=irow*rowstep;
                                }
                            cc++;
                            if (cc>ccmax)
                                {
                                ccmax=cc;
                                tanthmax=tanth;
                                ic0max=ic0;
                                ir0max=ir0;
                                }
                            }
                        else
                            cc=0;
                        }
                    }
                }
            }
        if (ccmax<ccthresh)
            break;
        if (debug)
            k2printf("    Vert line detected:  ccmax=%d (pix=%d), tanthmax=%g, ic0max=%d, ir0max=%d\n",ccmax,ccmax*rowstep,tanthmax,ic0max,ir0max);
        if (!vert_line_erase(bmp,cbmp,tmp,ir0max,ic0max,tanthmax,minheight_in,
                             /*minwidth_in,*/ maxwidth_in,white_thresh,dpi,erase_vertical_lines))
            break;
        }
/*
bmp_write(tmp,"outt.png",stdout,95);
wfile_written_info("outt.png",stdout);
bmp_write(bmp,"out2.png",stdout,95);
wfile_written_info("out2.png",stdout);
exit(10);
*/
    /* v2.20--fix memory leak here */
    bmp_free(tmp);
    }


/*
** Calculate max vert line length.  Line is terminated by nw consecutive white pixels
** on either side.
**
** v2.10--handle cbmp 8-bit correctly.
*/
static int vert_line_erase(WILLUSBITMAP *bmp,WILLUSBITMAP *cbmp,WILLUSBITMAP *tmp,
                    int row0,int col0,double tanth,double minheight_in,
                    /* double minwidth_in,*/ double maxwidth_in,int white_thresh,
                    double dpi,int erase_vertical_lines)

    {
    int lw,cc,maxdev,nw,dir,i,n,cbpp;
    int *c1,*c2,*w;
    static char *funcname="vert_line_erase";

#if (WILLUSDEBUGX & 0x8000)
printf("@vert_line_erase(row0=%d,col0=%d,tanth=%g,minheight_in=%g\n"
       "                 maxwidth_in=%g,white_thresh=%d,dpi=%g,evl=%d\n",
row0,col0,tanth,minheight_in,
maxwidth_in,white_thresh,dpi,erase_vertical_lines);
printf("     bmp = %d x %d x %d\n",bmp->width,bmp->height,bmp->bpp);
if (cbmp!=NULL)
printf("     cbmp = %d x %d x %d\n",cbmp->width,cbmp->height,cbmp->bpp);
if (tmp!=NULL)
printf("     tmp = %d x %d x %d\n",tmp->width,tmp->height,tmp->bpp);
#endif
    cbpp = (cbmp!=NULL && cbmp->bpp==24) ? 3 : 1;
    willus_dmem_alloc_warn(26,(void **)&c1,sizeof(int)*3*bmp->height,funcname,10);
    c2=&c1[bmp->height];
    w=&c2[bmp->height];
    /*
    maxdev = (int)((double)bmp->height / minheight_in +.5);
    if (maxdev < 3)
        maxdev=3;
    */
    nw = (int)(dpi/100.+.5);
    if (nw<2)
        nw=2;
    maxdev=nw;
    for (i=0;i<bmp->height;i++)
        c1[i]=c2[i]=-1;
    n=0;
    for (dir=-1;dir<=1;dir+=2)
        {
        int del,brc;

#if (WILLUSDEBUGX & 0x8000)
printf("dir=%d\n",dir);
#endif
        brc = 0;
        for (del=(dir==-1)?0:1;1;del++)
            {
            int r,c;
            unsigned char *p;

#if (WILLUSDEBUGX & 0x8000)
printf("del=%d\n",del);
#endif
            r=row0+dir*del;
            if (r<0 || r>bmp->height-1)
                break;
            c=col0+(r-row0)*tanth;
            if (c<0 || c>bmp->width-1)
                break;
            p=bmp_rowptr_from_top(bmp,r);
            for (i=c;i<=c+maxdev && i<bmp->width;i++)
                if (p[i]<white_thresh)
                    break;
            if (i>c+maxdev || i>=bmp->width)
                {
                for (i=c-1;i>=c-maxdev && i>=0;i--)
                    if (p[i]<white_thresh)
                        break;
                if (i<c-maxdev || i<0)
                    {
                    brc++;
                    if (brc>=nw)
                        break;
                    continue;
                    }
                }
            brc=0;
            for (c=i,cc=0;i<bmp->width;i++)
                if (p[i]<white_thresh)
                    cc=0;
                else
                    {
                    cc++;
                    if (cc>=nw)
                        break;
                    }
            c2[r]=i-cc;
            if (c2[r]>bmp->width-1)
                c2[r]=bmp->width-1;
            for (cc=0,i=c;i>=0;i--)
                if (p[i]<white_thresh)
                    cc=0;
                else
                    {
                    cc++;
                    if (cc>=nw)
                        break;
                    }
            c1[r]=i+cc;
            if (c1[r]<0)
                c1[r]=0;
            w[n++]=c2[r]-c1[r]+1;
            c1[r]-=cc;
            if (c1[r]<0)
                c1[r]=0;
            c2[r]+=cc;
            if (c2[r]>bmp->width-1)
                c2[r]=bmp->width-1;
            }
        }
#if (WILLUSDEBUGX & 0x8000)
printf("n=%d\n",n);
#endif
    if (n>1)
        sorti(w,n);
/*
printf("n=%d, w[%d]=%d, w[%d]=%d (mw=%g)\n",n,n/4,w[n/4],3*n/4,w[3*n/4],maxwidth_in*dpi);
*/
    if (n < 10 || n < minheight_in*dpi
               || w[n/4] < 1 /* (int)(minwidth_in*dpi + .5) */
               || w[3*n/4] > (int)(maxwidth_in*dpi)
               || (erase_vertical_lines==1 && w[n-1] > maxwidth_in*dpi))
        {
#if (WILLUSDEBUGX & 0x8000)
printf("Erasing area in temp bitmap.\n");
#endif
        /* Erase area in temp bitmap */
        for (i=0;i<bmp->height;i++)
            {
            unsigned char *p;
            int cmax;

            if (c1[i]<0 || c2[i]<0)
                continue;
            cmax=(c2[i]-c1[i])+1;
            p=bmp_rowptr_from_top(tmp,i)+c1[i];
            for (;cmax>0;cmax--,p++)
                (*p)=255;
            }
        }
    else
        {
#if (WILLUSDEBUGX & 0x8000)
printf("Erasing line width in source\n");
#endif
        /* Erase line width in source bitmap */
        lw=w[3*n/4]+nw*2;
#if (WILLUSDEBUGX & 0x8000)
printf("1. lw=%d\n",lw);
#endif
        if (lw > maxwidth_in*dpi/2)
            lw=maxwidth_in*dpi/2;
#if (WILLUSDEBUGX & 0x8000)
printf("2. lw=%d\n",lw);
#endif
        for (i=0;i<bmp->height;i++)
            {
            unsigned char *p;
            int c0,cmin,cmax,count,white;

#if (WILLUSDEBUGX & 0x8000)
printf("i=%d\n",i);
#endif
            if (c1[i]<0 || c2[i]<0)
                continue;
            c0=col0+(i-row0)*tanth;
            cmin=c0-lw-1;
            if (cmin<c1[i])
                cmin=c1[i];
            cmax=c0+lw+1;
            if (cmax>c2[i])
                cmax=c2[i];
#if (WILLUSDEBUGX & 0x8000)
printf("A\n");
#endif
            p=bmp_rowptr_from_top(bmp,i);
            c0 = (p[cmin] > p[cmax]) ? cmin : cmax;
            white=p[c0];
#if (WILLUSDEBUGX & 0x8000)
printf("B\n");
#endif
            if (white <= white_thresh)
                white = white_thresh+1;
            if (white>255)
                white=255;
#if (WILLUSDEBUGX & 0x8000)
printf("C\n");
#endif
            count=(cmax-cmin)+1;
            p=&p[cmin];
#if (WILLUSDEBUGX & 0x8000)
printf("D\n");
#endif
            for (;count>0;count--,p++)
                (*p)=white;
#if (WILLUSDEBUGX & 0x8000)
printf("E\n");
#endif
            if (cbmp!=NULL)
                {
                unsigned char *p0;

                p=bmp_rowptr_from_top(cbmp,i);
                p0=p+c0*cbpp;
                p=p+cmin*cbpp;
                count=(cmax-cmin)+1;
#if (WILLUSDEBUGX & 0x8000)
printf("F width=%d, ht=%d, bpp=%d, c0=%d, cmin=%d, i=%d, count=%d\n",cbmp->width,cbmp->height,cbmp->bpp,c0,cmin,i,count);
#endif
                if (cbpp==3)
                    for (;count>0;count--,p+=3)
                        {
                        p[0]=p0[0];
                        p[1]=p0[1];
                        p[2]=p0[2];
                        }
                else
                    memset(p,p0[0],count);
#if (WILLUSDEBUGX & 0x8000)
printf("G\n");
#endif
                }
            }
#if (WILLUSDEBUGX & 0x8000)
printf("   done.\n");
#endif
        }
    willus_dmem_free(26,(double **)&c1,funcname);
    return(1);
    }


/*
** bmpgray must be 8-bit grayscale.  bmp can be either 8-bit or 24-bit.
*/
void bmp_paint_white(WILLUSBITMAP *bmpgray,WILLUSBITMAP *bmp,int white_thresh)

    {
    int i,bpp;

    bpp=bmp->bpp==24 ? 3 : 1;
    for (i=0;i<bmpgray->height;i++)
        {
        unsigned char *pgray,*p;
        int j;

        pgray=bmp_rowptr_from_top(bmpgray,i);
        p=bmp_rowptr_from_top(bmp,i);
        for (j=0;j<bmpgray->width;j++,pgray++,p+=bpp)
            if ((*pgray) >= white_thresh)
                {
                (*pgray) = 255;
                memset(p,255,bpp);
                }
        }
    }


/* Added in v2.22 -- -colorfg and -colorbg options */
void bmp_change_colors(WILLUSBITMAP *bmp,WILLUSBITMAP *mask,
                       char *colorfg,int fgtype,char *colorbg,int bgtype,
                       int c1,int r1,int c2,int r2)

    {
    int change_only_gray;
    int fg[3],bg[3],kmax,r;
    WILLUSBITMAP *bgbmp,_bgbmp;
    WILLUSBITMAP *fgbmp,_fgbmp;
    int statbg,statfg;

    if (bgtype==3)
        {
        bgbmp=&_bgbmp;
        bmp_init(bgbmp);
        statbg=bmp_read(bgbmp,colorbg[0]=='!'?&colorbg[1]:colorbg,NULL);
        if (statbg<0)
            {
            bmp_free(bgbmp);
            bgbmp=NULL;
            }
        }
    else
        bgbmp=NULL;
    if (fgtype==3)
        {
        fgbmp=&_fgbmp;
        bmp_init(fgbmp);
        statfg=bmp_read(fgbmp,colorfg[0]=='!'?&colorfg[1]:colorfg,NULL);
        if (statfg<0)
            {
            bmp_free(fgbmp);
            fgbmp=NULL;
            }
        }
    else
        fgbmp=NULL;
    change_only_gray = (colorfg[0]=='!' || colorbg[0]=='!');
    if (fgbmp==NULL)
        {
        int fgc;
        fgc=colorfg[0]=='\0' ? 0 : hexcolor(colorfg);
        fg[0]=(fgc&0xff0000)>>16;
        fg[1]=(fgc&0xff00)>>8;
        fg[2]=(fgc&0xff);
        }
    if (bgbmp==NULL)
        {
        int bgc;
        bgc=colorbg[0]=='\0' ? 0xffffff : hexcolor(colorbg);
        bg[0]=(bgc&0xff0000)>>16;
        bg[1]=(bgc&0xff00)>>8;
        bg[2]=(bgc&0xff);
        }
    kmax = bmp->bpp>>3;
    if (kmax>3)
        kmax=3;
    if (kmax<1)
        kmax=1;
    /* Make sure tiling bitmaps match bitness / grayscale of master */
    if (kmax==1)
        {
        if (fgbmp!=NULL && !bmp_is_grayscale(fgbmp))
            bmp_convert_to_grayscale(fgbmp);
        if (bgbmp!=NULL && !bmp_is_grayscale(bgbmp))
            bmp_convert_to_grayscale(bgbmp);
        }
    else
        {
        if (fgbmp!=NULL && fgbmp->bpp!=24)
            bmp_promote_to_24(fgbmp);
        if (bgbmp!=NULL && bgbmp->bpp!=24)
            bmp_promote_to_24(bgbmp);
        }
    for (r=r1;r<bmp->height && r<=r2;r++)
        {
        unsigned char *p;
        unsigned char *pm;
        unsigned char *pfg;
        unsigned char *pbg;
        int j;
        p=bmp_rowptr_from_top(bmp,r)+c1*kmax;
        pm=bmp_rowptr_from_top(mask,r)+c1*kmax;
        pfg = fgbmp==NULL ? NULL : bmp_rowptr_from_top(fgbmp,r%fgbmp->height)+(c1%fgbmp->width)*kmax;
        pbg = bgbmp==NULL ? NULL : bmp_rowptr_from_top(bgbmp,r%bgbmp->height)+(c1%bgbmp->width)*kmax;
        for (j=c1;j<=c2 && j<bmp->width;j++)
            {
            int k;
            if (change_only_gray && kmax==3 && !gscale(pm))
                {
                p+=kmax;
                pm+=kmax;
                continue;
                }
            for (k=0;k<kmax;k++,pm++,p++)
                {
                int x,fgc,bgc;

                x = pm[0];
                fgc = pfg==NULL ? (colorfg[0]=='\0' ? p[0]:fg[k]) : pfg[(j%fgbmp->width)*kmax+k];
                bgc = pbg==NULL ? (colorbg[0]=='\0' ? p[0]:bg[k]) : pbg[(j%bgbmp->width)*kmax+k];
                x = fgc + x*(bgc-fgc)/255;
                p[0] = x;
                }
            }
        }
    if (bgbmp!=NULL)
        bmp_free(bgbmp);
    if (fgbmp!=NULL)
        bmp_free(fgbmp);
    }


static int gscale(unsigned char *p)

    {
    int c1,c2;

    c1=p[0];
    c2=p[1];
    if (not_close(c1,c2))
        return(0);
    c2=p[2];
    if (not_close(c1,c2))
        return(0);
    c1=p[1];
    if (not_close(c1,c2))
        return(0);
    return(1);
    }


static int not_close(int c1,int c2)

    {
    int cm,dc,pd;

    cm=(c1+c2)>>1;
    dc=abs(c1-c2);
    if (cm<20)
        return(dc>5);
    pd=100*dc/cm;
    return(pd>5);
    }


void bmp8_merge(WILLUSBITMAP *dst,WILLUSBITMAP *src,int count)

    {
    int row,maxcount;

    if (dst->bpp!=8 || src->bpp!=8)
        return;
    maxcount=4;
    for (row=0;row<src->height && row<dst->height;row++)
        {
        int col;
        unsigned char *s,*d;

        s=bmp_rowptr_from_top(src,row);
        d=bmp_rowptr_from_top(dst,row);
        for (col=0;col<src->width && col<dst->width;col++,d++,s++)
            {
            int si,di,ni;

            si=s[0];
            di=d[0];
            if (count<maxcount)
                ni = (di*count + si) / (count + 1);
            else
                ni = 255 - ((255-di) + (255-si)/(maxcount+1));
            if (ni<0)
                ni=0;
            if (ni>255)
                ni=255;
            d[0]=ni;
            }
        }
    }

/*
** Crop margins, in pixels, put into cx[0..3] = left, top, right, bottom
*/
int bmp_autocrop2(WILLUSBITMAP *bmp0,int *cx,double aggressiveness)

    {
    WILLUSBITMAP *bmp,_bmp;
    int i,whitemax,wt,sum,status,pw;
    double s30;
    double hist[256];
    double blackweight;

#if (WILLUSDEBUGX & 0x8000)
printf("@bmp_autocrop2...\n");
#endif
    bmp=&_bmp;
    bmp_init(bmp);
    bmp_copy(bmp,bmp0);
    bmp_convert_to_grayscale(bmp);
    for (i=0;i<256;i++)
        hist[i]=0.;
    for (i=0;i<bmp->height;i++)
        {
        unsigned char *p;
        int j;

        p=bmp_rowptr_from_top(bmp,i);
        for (j=0;j<bmp->width;j++,p++)
            hist[(*p)]+=1.0;
        }
    blackweight = aggressiveness*50.;
#if (WILLUSDEBUGX & 0x8000)
{
FILE *f;
static int count=0;
int i,s;
count++;
if (count%2==1)
{
char filename[256];
sprintf(filename,"src%03d.png",(count+1)/2);
bmp_write(bmp,filename,NULL,100);
f=fopen("hist.ep",count==1?"w":"a");
fprintf(f,"/sa l \"%d\" 1\n",(count+1)/2);
for (i=s=0;i<256;i++)
{
s+=hist[i];
fprintf(f,"%d\n",s);
}
fprintf(f,"//nc\n");
}
}
#endif
    s30=0.3*bmp->width*bmp->height;
    for (i=255,sum=0.;sum<s30;sum+=hist[i],i--);
    whitemax=i;
/*
printf("whitemax=%d\n",whitemax);
*/
    pw = bmp->width/80;
    if (pw<1)
        pw=1;
    wt=192+(whitemax-192)*(pw-1)/pw;
#if (WILLUSDEBUGX & 0x8000)
printf("wt=%d\n",wt);
#endif
/*
printf("pw=%d, wt=%d\n",pw,wt);
*/
//    status=bmp_autocrop2_ex(bmp,pw,pw,wt,10.,.6,cx);
    status=bmp_autocrop2_ex(bmp,pw,pw,wt,blackweight,.6,0.05,cx);
/*
    printf("bmp_autocrop returns %d\n",status);
    printf("    (%d,%d) - (%d,%d)\n",cx[0],cx[1],cx[2],cx[3]);
    if (bmp->bpp!=24)
        bmp_promote_to_24(bmp);
    printf("bmp=%d x %d x %d\n",bmp->width,bmp->height,bmp->bpp);
    if (1)
        {
        int row,col;
        unsigned char *p1,*p2;
        for (row=cx[1];row<=cx[3];row++)
            {
            p1=bmp_rowptr_from_top(bmp,row)+cx[0]*3;
            p2=bmp_rowptr_from_top(bmp,row)+cx[2]*3;
            p1[0]=255;
            p1[1]=p1[2]=0;
            p2[0]=255;
            p2[1]=p2[2]=0;
            }
        p1=bmp_rowptr_from_top(bmp,cx[1])+cx[0]*3;
        p2=bmp_rowptr_from_top(bmp,cx[3])+cx[0]*3;
        for (col=cx[0];col<=cx[2];col++,p1+=3,p2+=3)
            {
            p1[0]=255;
            p1[1]=p1[2]=0;
            p2[0]=255;
            p2[1]=p2[2]=0;
            }
        bmp_write(bmp,"out.png",stdout,100);
        wfile_written_info("out.png",stdout);
        }
*/
    bmp_free(bmp);
    cx[2] = bmp->width-1-cx[2];
    cx[3] = bmp->height-1-cx[3];
#if (WILLUSDEBUGX & 0x8000)
printf("cx[0]=%d\n",cx[0]);
printf("cx[1]=%d\n",cx[1]);
printf("cx[2]=%d\n",cx[2]);
printf("cx[3]=%d\n",cx[3]);
#endif
    return(status);
    }


/*
** bmp must be grayscale
*/
void k2bmp_apply_autocrop(WILLUSBITMAP *bmp,int *cx0)

    {
    int i,j;
    int cx[4];

    for (i=0;i<4;i++)
        cx[i]=cx0[i];
    cx[2] = bmp->width-1-cx[2];
    cx[3] = bmp->height-1-cx[3];
    for (i=0;i<bmp->width;i++)
        if (i<cx[0] || i>cx[2])
            for (j=0;j<bmp->height;j++)
                {
                unsigned char *p;
                p=bmp_rowptr_from_top(bmp,j)+i;
                p[0]=255;
                }
    for (i=0;i<bmp->height;i++)
        if (i<cx[1] || i>cx[3])
            {
            unsigned char *p;
            p=bmp_rowptr_from_top(bmp,i);
            memset(p,255,bmp->width);
            }
    }

#ifdef HAVE_LEPTONICA_LIB
/*
** src must be grayscale
*/
void k2bmp_prep_for_dewarp(WILLUSBITMAP *dst,WILLUSBITMAP *src,int dx,int whitethresh)

    {
    int row;

    bmp_copy(dst,src);
    bmp_fill(dst,255,255,255);
    for (row=0;row<src->height;row++)
        {
        unsigned char *dp,*sp;
        int col;

        dp=bmp_rowptr_from_top(dst,row);
        sp=bmp_rowptr_from_top(src,row);
        for (col=0;col<src->width;col++,dp++,sp++)
            if (sp[0]<whitethresh)
                {
                int i,i1,i2;
                i1=col<dx?-col:-dx;
                i2=col+dx>=src->width?src->width-1-col:dx;
                for (i=i1;i<=i2;i++)
                    dp[i]=0;
                }
        }
    }
#endif


/*
** Passed bitmap must be grayscale
**
** pixwidth = pixel width of the search frame
** pixstep = step value for the search frame
** whitethresh = value above which pixels are considered white
** blackweight = weighting given to any black pixels in the frame
** minarea = min area encompassed by frame
** cx[0] = left frame position
** cx[1] = top frame position (from top of bitmap)
** cx[2] = right frame position
** cx[3] = bottom frame position (from top of bitmap)
*/
static int bmp_autocrop2_ex(WILLUSBITMAP *bmp,int pixwidth,int pixstep,int whitethresh,
                            double blackweight,double minarea,double threshold,int *cx)

    {
    int k,cxbest[4];
    double maxarea,bmparea;
    WILLUSBITMAP *bw,_bw;

    for (k=0;k<4;k++)
        cxbest[k]=0;
    pixstep = (pixstep+pixwidth/2)/pixwidth;
    if (pixstep<1)
        pixstep=1;
    bw=&_bw;
    bmp_init(bw);
    bmp_integer_resample(bw,bmp,pixwidth);
/*
printf("pixwidth=%d, bw->height=%d\n",pixwidth,bw->height);
*/
//    bmp_convert_to_monochrome(bw,whitethresh);
#if (WILLUSDEBUGX & 0x8000)
{
static int pageno=0;
char filename[256];
int i,n;
pageno++;
if (pageno%2==1)
{
n=bw->width*bw->height;
sprintf(filename,"page%03d.png",(pageno+1)/2);
//for (i=0;i<n;i++)
//    bw->data[i]=bw->data[i]?0:255;
bmp_write(bw,filename,NULL,100);
//for (i=0;i<n;i++)
//    bw->data[i]=bw->data[i]?0:1;
printf("%s\n",filename);
}
}
#endif
    maxarea=-999.;
    /* minblack=1.1; */
    bmparea=(double)bw->width*bw->height;
    cxbest[0]=cxbest[1]=0;
    cxbest[2]=bw->width-1;
    cxbest[3]=bw->height-1;
    for (cx[0]=0;1;cx[0]=cx[0]+pixstep)
        {
        cx[1]=0;
        cx[2]=bw->width-1;
        cx[3]=bw->height-1;
        if (frame_area(bmparea,cx)<minarea)
            break;
        for (cx[1]=0;1;cx[1]=cx[1]+pixstep)
            {
            cx[2]=bw->width-1;
            cx[3]=bw->height-1;
            if (frame_area(bmparea,cx)<minarea)
                break;
            for (cx[2]=bw->width-1;1;cx[2]=cx[2]-pixstep)
                {
                cx[3]=bw->height-1;
                if (frame_area(bmparea,cx)<minarea)
                    break;
                for (cx[3]=bw->height-1;1;cx[3]=cx[3]-pixstep)
                    {
                    double area,areaw,black,stdev;

                    area=frame_area(bmparea,cx);
                    if (area<minarea)
                        break;
                    black=frame_black_percentage(bw,cx,3);
                    stdev=frame_stdev_norm(bw,cx,3);
                    areaw=area-blackweight*(black+3.*stdev);
                    if (areaw > maxarea)
                        {
                        maxarea=areaw;
/*
printf("maxarea(%d,%d,%d,%d)=%g-10x%g=%g\n",cx[0],cx[1],cx[2],cx[3],area,black,areaw);
*/
                        for (k=0;k<4;k++)
                            cxbest[k]=cx[k];
                        break;
                        }
/*
                    if (black < minblack)
                        {
printf("minblack(%d,%d,%d,%d)=%g\n",cx[0],cx[1],cx[2],cx[3],black);
                        minblack = black;
                        for (k=0;k<4;k++)
                            cxb2[k]=cx[k];
                        }
                    if (black > maxblack)
                        continue;
                    if (area > maxarea)
                        {
                        maxarea=area;
printf("maxarea(%d,%d,%d,%d)=%g\n",cx[0],cx[1],cx[2],cx[3],area);
                        for (k=0;k<4;k++)
                            cxbest[k]=cx[k];
                        break;
                        }
*/
                    }
                }
            }
        }
    bmp_free(bw);
    cx[0]=cxbest[0]*pixwidth;
    cx[1]=cxbest[1]*pixwidth;
    cx[2]=(cxbest[2]+1)*pixwidth-1;
    cx[3]=(cxbest[3]+1)*pixwidth-1;
    if (cx[2]>bmp->width-1)
        cx[2]=bmp->width-1;
    if (cx[3]>bmp->height-1)
        cx[3]=bmp->height-1;
    bmp_autocrop_refine(bmp,whitethresh,threshold,cx,pixwidth);
    return(maxarea>=0.);
    }


/*
** thresold = 0 to 1.  Typically 0.1, I'd think.
*/
static void bmp_autocrop_refine(WILLUSBITMAP *bmp,int whitethresh,double threshold,int *cx,
                                int pixwidth)

    {
    double *x0,*hist;
    double sum;
    int max,i,c,n;
    static char *funcname="bmp_autocrop_refine";
    int cx0[4],cnew[4];
#if (WILLUSDEBUGX & 0x8000)
static int count0=0;

if (count0&1)
{
count0++;
return;
}
{
char filename[256];
sprintf(filename,"bmp%04d.png",count0);
bmp_write(bmp,filename,NULL,100);
}
for (i=0;i<4;i++)
printf("cx[%d]=%d\n",i,cx[i]);
#endif
    max=bmp->width > bmp->height ? bmp->width : bmp->height;
    willus_dmem_alloc_warn(46,(void **)&hist,sizeof(double)*2*max,funcname,10);
    x0=&hist[max];
    for (i=0;i<4;i++)
        cx0[i]=cx[i];
    n=bmp->width;
    for (sum=0.,c=cx0[0]=0;cx0[0]<n;cx0[0]++)
        {
        double black,stdev;

        i=cx0[0];
        black=frame_black_percentage(bmp,cx0,1);
        stdev=frame_stdev_norm(bmp,cx0,1);
        x0[i]=(double)(i-cx[0])/(cx[2]-cx[0]+1);
        hist[i]=black+3.*stdev;
        if (x0[i]>=.25 && x0[i]<=.75)
            {
            c++;
            sum+=hist[i];
            }
        }
    if (c>0)
        sum/=c;
    else
        sum=1.0;
    for (i=0;i<n;i++)
        hist[i] /= sum;
    xsmooth(hist,n,pixwidth/3);
    cnew[0]=cx[0]+find_threshold(x0,hist,n,threshold)*(cx[2]-cx[0]+1);
#if (WILLUSDEBUGX & 0x8000)
    {
    FILE *f;
    static int count=0;
    f=fopen("acrosswidth.ep",count==0?"w":"a");
    fprintf(f,"/sa l \"%d\" 2\n",count0);
    for (i=0;i<n;i++)
        fprintf(f,"%g %g\n",x0[i],hist[i]);
    fprintf(f,"//nc\n");
    fclose(f);
    count++;
    }
#endif
    for (i=0;i<n;i++)
        x0[i]=1.0-x0[i];
    sortxyd(x0,hist,n);
    cnew[2]=cx[2]+1-find_threshold(x0,hist,n,threshold)*(cx[2]-cx[0]+1);
    for (i=0;i<4;i++)
        cx0[i]=cx[i];
    n=bmp->height;
    for (sum=0.,c=cx0[1]=0;cx0[1]<n;cx0[1]++)
        {
        double black,stdev;

        i=cx0[1];
        black=frame_black_percentage(bmp,cx0,2);
        stdev=frame_stdev_norm(bmp,cx0,2);
        x0[i]=(double)(i-cx[1])/(cx[3]-cx[1]+1);
        hist[i]=black+3.*stdev;
        if (x0[i]>=.25 && x0[i]<=.75)
            {
            c++;
            sum+=hist[i];
            }
        }
    if (c>0)
        sum/=c;
    else
        sum=1.0;
    for (i=0;i<n;i++)
        hist[i] /= sum;
    xsmooth(hist,n,pixwidth/3);
    cnew[1]=cx[1]+find_threshold(x0,hist,bmp->width,threshold)*(cx[3]-cx[1]+1);
#if (WILLUSDEBUGX & 0x8000)
    {
    FILE *f;
    static int count=0;
    f=fopen("acrossheight.ep",count==0?"w":"a");
    fprintf(f,"/sa l \"%d\" 2\n",count0);
    for (i=0;i<n;i++)
        fprintf(f,"%g %g\n",x0[i],hist[i]);
    fprintf(f,"//nc\n");
    fclose(f);
    count++;
    }
#endif
    for (i=0;i<n;i++)
        x0[i]=1.0-x0[i];
    sortxyd(x0,hist,n);
    cnew[3]=cx[3]+1-find_threshold(x0,hist,n,threshold)*(cx[3]-cx[1]+1);
#if (WILLUSDEBUGX & 0x8000)
count0++;
#endif
    willus_dmem_free(46,(double **)&hist,funcname);
    for (i=0;i<4;i++)
        cx[i]=cnew[i];
#if (WILLUSDEBUGX & 0x8000)
for (i=0;i<4;i++)
printf("cxnew[%d]=%d\n",i,cx[i]);
#endif
    }


static void xsmooth(double *y,int n,int cwin)

    {
    int i,dn;
    double *y2;
    static char *funcname="xsmooth";

    willus_dmem_alloc_warn(47,(void **)&y2,sizeof(double)*n,funcname,10);
    dn=(cwin-1)/2;
    for (i=dn;i<n-dn;i++)
        {
        int j;
        double sum;
        for (sum=0.,j=i-dn;j<=i+dn;j++)
            sum+=y[i];
        y2[i]=sum/cwin;
        }
    for (i=0;i<dn;i++)
        y2[i]=y2[dn];
    for (i=n-dn;i<n;i++)
        y2[i]=y2[n-dn-1];
    for (i=0;i<n;i++)
        y[i]=y2[i];
    willus_dmem_free(47,(double **)&y2,funcname);
    }


static double find_threshold(double *x,double *y,int n,double threshold) 

    {
    double max,thresh;
    int i,i0,imin;

    i0=indexxd(0.,x,n);
    if (i0<0)
        i0=0;
    if (i0>n-1)
        i0=n-1;
    for (max=0.,i=0;i<n;i++)
        {
        if (x[i]<0. || x[i]>1.)
            continue;
        if (y[i]>max)
            max=y[i];
        }
    /* Find starting point -- minimum */
    if (max<.2)
        return(0.);
    for (imin=i=i0;i<n;i++)
        {
        if (x[i]>0.35 || y[i] > 0.1*(max-y[i0]))
            break;
        if (y[i]<y[imin])
            imin=i;
        }
    for (i=i0;i>=0;i--)
        {
        if (x[i]<-0.2 || y[i] > 0.1*(max-y[i0]))
            break;
        if (y[i]<y[imin])
            imin=i;
        }
    thresh=y[imin]+(max-y[imin])*threshold;
    for (i=imin;i<n;i++)
        if (x[i]>0.35 || y[i] >= thresh)
            break;
    return(x[i-1]);
    }


static double frame_area(double area,int *cx)

    {
    return((double)(cx[2]+1-cx[0])*(double)(cx[3]+1-cx[1])/area);
    }


/* src must be grayscale */
static void bmp_convert_to_monochrome(WILLUSBITMAP *bmp,int whitethresh)

    {
    int row;

    if (!bmp_is_grayscale(bmp))
        {
        printf("Internal error (bitmap not grayscale at bmp_autocrop).\n");
        exit(100);
        }
    for (row=0;row<bmp->height;row++)
        {
        int col;
        unsigned char *p;
        p=bmp_rowptr_from_top(bmp,row);
        for (col=0;col<bmp->width;col++,p++)
            p[0]=(p[0]>=whitethresh ? 0 : 1);
        }
    }

/*
** Calculate variation in black and white in frame.
** 0 to 1
** Higher value indicates part of the frame is probably over
** a row or rows of text.
** flags==1 does left side only
** flags==2 does top only
** flags==3 does whole frame
*/
static double frame_stdev_norm(WILLUSBITMAP *bmp,int *cx,int flags)

    {
    unsigned char *p,*p1,*p2,*p3;
    int i,w,h,dr,sum;
    double stdev0,stdev;

    w=cx[2]-cx[0]+1;
    h=cx[3]-cx[1]+1-2;
    dr=bmp_bytewidth(bmp);
    p=bmp_rowptr_from_top(bmp,cx[1])+cx[0];
    p1=p+dr;
    if (flags==1)
        {
        h+=2;
        p1-=dr;
        }
    p2=bmp_rowptr_from_top(bmp,cx[1]+1)+cx[2];
    p3=bmp_rowptr_from_top(bmp,cx[3])+cx[0];
    stdev=0.;
    if (flags!=1)
        {
        for (sum=0,i=0;i<w-1;i++,p++)
            {
            int v1,v2;
            v1=p[0];
            v2=p[1];
            sum+=abs(v2-v1);
            }
        stdev0=(double)sum/(w-1);
        if (flags==2)
            return(stdev0/255.);
        if (stdev0 > stdev)
            stdev=stdev0;
        for (sum=0,i=0;i<w-1;i++,p3++)
            {
            int v1,v2;
            v1=p3[0];
            v2=p3[1];
            sum+=abs(v2-v1);
            }
        stdev0=(double)sum/(w-1);
        if (stdev0 > stdev)
            stdev=stdev0;
        }
    for (sum=0,i=0;i<h-1;i++,p1+=dr)
        {
        int v1,v2;
        v1=p1[0];
        v2=p1[dr];
        sum+=abs(v2-v1);
        }
    stdev0=(double)sum/(h-1);
    if (flags==1)
        return(stdev0/255.);
    if (stdev0 > stdev)
        stdev=stdev0;
    for (sum=0,i=0;i<h-1;i++,p2+=dr)
        {
        int v1,v2;
        v1=p2[0];
        v2=p2[dr];
        sum+=abs(v2-v1);
        }
    stdev0=(double)sum/(h-1);
    if (stdev0 > stdev)
        stdev=stdev0;
    return(stdev/255.);
    }

/*
** flags==1 does left side only
** flags==2 does top only
** flags==3 does whole frame
*/
static double frame_black_percentage(WILLUSBITMAP *bmp,int *cx,int flags)

    {
    unsigned char *p,*p1,*p2,*p3;
    int w,h,dr,len,sum;

    w=cx[2]-cx[0]+1;
    h=cx[3]-cx[1]+1-2;
    len=2*w+2*h;
    dr=bmp_bytewidth(bmp);
    p=bmp_rowptr_from_top(bmp,cx[1])+cx[0];
    p1=p+dr;
    p2=bmp_rowptr_from_top(bmp,cx[1]+1)+cx[2];
    p3=bmp_rowptr_from_top(bmp,cx[3])+cx[0];
    if (flags==3)
        {
        for (sum=0;w>0;w--,p++,p3++)
            sum+=(*p)+(*p3);
        for (;h>0;h--,p1+=dr,p2+=dr)
            sum+=(*p1)+(*p2);
        }
    else if (flags==1)
        {
        h+=2;
        p1-=dr;
        len=h;
        for (sum=0;h>0;h--,p1+=dr)
            sum+=(*p1);
        }
    else
        {
        len=w;
        for (sum=0;w>0;w--,p++)
            sum+=(*p);
        }
       
//    return((double)sum/len);
    return(1.-(double)sum/(255.*len));
    }


void k2pagebreakmarks_find_pagebreak_marks(K2PAGEBREAKMARKS *k2pagebreakmarks,WILLUSBITMAP *bmp,
                                        WILLUSBITMAP *bmpgrey,int dpi,int *color,int *type,int n)

    {
    int j,row,width;
    double rr[8],gg[8],bb[8];

#if (WILLUSDEBUGX & 0x800000)
printf("At k2pagebreakmarks_find_pagebreak_marks.\n");
for (j=0;j<n;j++)
printf("    color[%d]=0x%0X\n",j,color[j]);
printf("    type[%d]=%d\n",j,type[j]);
#endif
    if (k2pagebreakmarks==NULL)
        return;
    if (bmp==NULL || bmp->bpp<24 || (bmpgrey!=NULL && bmpgrey->bpp!=8))
        {
        printf("Internal Error--Bit-per-pixel mismatch in k2pagebreaks_find_pagebreak_marks.  Contact Author.\n");
        exit(20);
        }
    width=bmp->width;
    for (j=0;j<n && j<8;j++)
        {
        rr[j]=((color[j]>>16)&0xff)/255.;
        gg[j]=((color[j]>>8)&0xff)/255.;
        bb[j]=(color[j]&0xff)/255.;
        }
    for (row=0;row<bmp->height;row++)
        {
        unsigned char *p,*pg;
        int jbest,jlast,lastcol,col;

        p=bmp_rowptr_from_top(bmp,row);
        pg = bmpgrey==NULL ? NULL : bmp_rowptr_from_top(bmpgrey,row);
        for (jbest=-1,jlast=-1,lastcol=-1,col=0;col<width;col++,p+=3)
            {
            double blackmatch,whitematch,graymatch,cm,cmbest;
            int r,g,b;

            /* Find best color match */
            r=p[0];
            g=p[1];
            b=p[2];
            blackmatch=(r/255.+g/255.+b/255.)/3.;
            whitematch=1.-blackmatch;
            graymatch=(abs(r-g)/255.+abs(g-b)/255.+abs(r-b)/255.)/3.;
            for (cmbest=3.,jbest=-1,j=0;j<n && j<8;j++)
                {
                cm = (fabs(r/255.-rr[j])+fabs(g/255.-gg[j])+fabs(b/255.-bb[j]))/3.;
                if (cm<cmbest && cm<blackmatch && cm<whitematch && cm<graymatch)
                    {
                    jbest=j;
                    cmbest=cm;
                    }
                }

            /* Paint over mark if it matches a color */
            if (jbest>=0)
                {
                int paintover;

                /* paintover = (type[jbest]==K2PAGEBREAKMARK_TYPE_NOBREAK ? 0 : 255); */
                paintover=255;
                p[0]=p[1]=p[2]=paintover;
                if (pg!=NULL)
                    pg[col]=paintover;
                }
/*
if (jbest>=0)
printf(" TYPE %d (%d,%d,%d) @ %4d,%4d\n",jbest,p[0],p[1],p[2],col,row);
*/
            if (jbest==jlast && col<width-1)
                continue;
            if (jbest>=0 && jbest==jlast)
                {
                int markcol,markrow;

                markcol=(col+lastcol)/2;
                markrow=row;
/*
printf("    Adding type %d at %d,%d\n",jbest,markcol,markrow);
*/
                k2pagebreakmarks_add_mark(k2pagebreakmarks,markcol,markrow,type[jbest],dpi);
                }
            else if (jbest!=jlast && jlast>=0)
                {
                int markcol,markrow;

                markcol=(col-1+lastcol)/2;
                markrow=row;
/*
printf("    Adding type %d at %d,%d\n",jlast,markcol,markrow);
*/
                k2pagebreakmarks_add_mark(k2pagebreakmarks,markcol,markrow,type[jlast],dpi);
                }
            if (jbest!=jlast)
                {
                jlast=jbest;
                if (jlast>=0)
                    lastcol=col;
                else
                    lastcol=-1;
                }
            }
        }
#if (WILLUSDEBUGX & 0x800000)
printf("...Call complete.\n");
#endif
    }


static void k2pagebreakmarks_add_mark(K2PAGEBREAKMARKS *k2pagebreakmarks,int markcol,int markrow,
                                      int marktype,int dpi)

    {
    int n;
    static int warned=0;
    K2PAGEBREAKMARK *breakmark;

    if (k2pagebreakmarks==NULL)
        return;
    if (k2pagebreakmarks_too_close_to_others(k2pagebreakmarks,markcol,markrow,dpi))
/*
{
printf("    %d,%d too close to other marks.\n",markcol,markrow);
*/
        return;
/*
}
*/
    n=k2pagebreakmarks->n;
    if (n>=MAXK2PAGEBREAKMARKS)
        {
        if (!warned)
            k2printf("\n" TTEXT_WARN "Warning: Max page break marks exceeded.  "
                         "Some will be ignored!" TTEXT_NORMAL "\n\n");
        warned=1;
        return;
        }
    warned=0;
    breakmark = &k2pagebreakmarks->k2pagebreakmark[n];
    breakmark->row = markrow;
    breakmark->col = markcol;
    breakmark->type = marktype;
    k2pagebreakmarks->n++;
    }


static int k2pagebreakmarks_too_close_to_others(K2PAGEBREAKMARKS *k2pagebreakmarks,int markcol,
                                                int markrow,int dpi)

    {
    int i;

    if (k2pagebreakmarks==NULL)
        return(0);
    for (i=0;i<k2pagebreakmarks->n;i++)
        {
        K2PAGEBREAKMARK *mark;
        double dx,dy;

        mark=&k2pagebreakmarks->k2pagebreakmark[i];
        dx = (double)abs(mark->col - markcol)/dpi;
        dy = (double)abs(mark->row - markrow)/dpi;
        if (dx < 1.0 && dy < .1)
            return(1);
        }
    return(0);
    }
