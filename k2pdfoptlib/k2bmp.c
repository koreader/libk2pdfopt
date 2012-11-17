/*
** k2bmp.c      Bitmap manipulation functions for k2pdfopt.  These routines
**              are mostly generic bitmap functions, but there are some
**              k2pdfopt-specific settings for some.
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


static int inflection_count(double *x,int n,int delta,int *wthresh);
static int vert_line_erase(WILLUSBITMAP *bmp,WILLUSBITMAP *cbmp,WILLUSBITMAP *tmp,
                    int row0,int col0,double tanth,double minheight_in,
                    double minwidth_in,double maxwidth_in,int white_thresh,
                    double dpi,int erase_vertical_lines);


int bmp_get_one_document_page(WILLUSBITMAP *src,K2PDFOPT_SETTINGS *k2settings,
                              int src_type,char *filename,
                              int pageno,double dpi,int bpp,FILE *out)

    {
    if (src_type==SRC_TYPE_PDF)
        {
        int status;
#ifdef HAVE_MUPDF_LIB
        static char *mupdferr_trygs=TTEXT_WARN "\a\n ** ERROR reading from " TTEXT_BOLD2 "%s" TTEXT_WARN "using MuPDF.  Trying Ghostscript...\n\n" TTEXT_NORMAL;

        status=0;
        if (k2settings->usegs<=0)
            {
            status=bmpmupdf_pdffile_to_bmp(src,filename,pageno,dpi*k2settings->document_scale_factor,bpp);
            if (!status || k2settings->usegs<0)
                return(status);
            }
        /* Switch to Postscript since MuPDF failed */
        if (k2settings->usegs==0)
            {
            aprintf(mupdferr_trygs,filename);
            k2settings->usegs=1;
            }
#endif
#ifdef HAVE_GHOSTSCRIPT_LIB
        if (willusgs_init(stdout) < 0)
            {
            k2sys_enter_to_exit(k2settings);
            exit(20);
            }
#endif
        bmp_set_pdf_pageno(pageno);
        bmp_set_pdf_dpi(dpi);
        /*
        aprintf("Converting " TTEXT_BOLD2 "%s" TTEXT_NORMAL 
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
    return(-1);
    }


void bmp_adjust_contrast(WILLUSBITMAP *src,WILLUSBITMAP *srcgrey,
                         K2PDFOPT_SETTINGS *k2settings,int *white)

    {
    int i,j,tries,wc,tc,hist[256];
    double contrast,rat0;
    WILLUSBITMAP *dst,_dst;

    if (k2settings->debug && k2settings->verbose)
        printf("\nAt adjust_contrast.\n");
    if ((*white) <= 0)
        (*white)=192;
    /* If contrast_max negative, use it as fixed contrast adjustment. */
    if (k2settings->contrast_max < 0.)
        {
        bmp_contrast_adjust(srcgrey,srcgrey,-k2settings->contrast_max);
        if (k2settings->dst_color && fabs(k2settings->contrast_max+1.0)>1e-4)
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
                printf("    rat0 = rat[%d-255]=%.4f\n",(*white),rat0);
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
            printf("    %2d. Contrast=%7.2f, rat[252-255]/rat0=%.4f\n",
                        tries+1,contrast,(double)wc/tc/rat0);
        if ((double)wc/tc >= rat0*0.94)
            break;
        contrast *= 1.05;
        }
    if (k2settings->debug)
        printf("Contrast=%7.2f, rat[252-255]/rat0=%.4f\n",
                       contrast,(double)wc/tc/rat0);
/*
bmp_write(dst,"outc.png",stdout,100);
wfile_written_info("outc.png",stdout);
exit(10);
*/
    bmp_copy(srcgrey,dst);
    /* Maybe don't adjust the contrast for the color bitmap? */
    if (k2settings->dst_color && fabs(contrast-1.0)>1e-4)
        bmp_contrast_adjust(src,src,contrast);
    bmp_free(dst);
    }



/*
** src is only allocated if dst_color != 0
*/
void bmp_clear_outside_crop_border(WILLUSBITMAP *src,WILLUSBITMAP *srcgrey,
                                   K2PDFOPT_SETTINGS *k2settings)

    {
    int i,n;
    BMPREGION *region,_region;

    region=&_region;
    region->bmp = srcgrey;
    region->dpi = k2settings->src_dpi;
    bmpregion_trim_to_crop_margins(region,k2settings);
    n=region->c1;
    for (i=0;i<srcgrey->height;i++)
        {
        unsigned char *p;
        if (k2settings->dst_color)
            {
            p=bmp_rowptr_from_top(src,i);
            memset(p,255,n*3);
            }
        p=bmp_rowptr_from_top(srcgrey,i);
        memset(p,255,n);
        }
    n=srcgrey->width-1-region->c2;
    for (i=0;i<srcgrey->height;i++)
        {
        unsigned char *p;
        if (k2settings->dst_color)
            {
            p=bmp_rowptr_from_top(src,i)+3*(src->width-n);
            memset(p,255,n*3);
            }
        p=bmp_rowptr_from_top(srcgrey,i)+srcgrey->width-n;
        memset(p,255,n);
        }
    n=region->r1;
    for (i=0;i<n;i++)
        {
        unsigned char *p;
        if (k2settings->dst_color)
            {
            p=bmp_rowptr_from_top(src,i);
            memset(p,255,src->width*3);
            }
        p=bmp_rowptr_from_top(srcgrey,i);
        memset(p,255,srcgrey->width);
        }
    n=srcgrey->height-1-region->r2;
    for (i=srcgrey->height-n;i<srcgrey->height;i++)
        {
        unsigned char *p;
        if (k2settings->dst_color)
            {
            p=bmp_rowptr_from_top(src,i);
            memset(p,255,src->width*3);
            }
        p=bmp_rowptr_from_top(srcgrey,i);
        memset(p,255,srcgrey->width);
        }
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
printf("h %d:\n",i);
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
printf("v %d:\n",i);
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
    // printf("    page %2d:  %8.4f\n",pagenum,rat);
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
    static int hist[256];
    static char *funcname="inflection_count";

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
#ifdef DEBUG
printf("wt=%d\n",wt);
#endif
        (*wthresh)=wt;
        }
    else
        wt=(*wthresh);
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
printf("    ni=%3d, f1=%8.4f, f2=%8.4f, f1*f2*ni=%8.4f\n",ni,f1,f2,f1*f2*ni);
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
** bmp must be grayscale! (cbmp = color, can be null)
*/
void bmp_detect_vertical_lines(WILLUSBITMAP *bmp,WILLUSBITMAP *cbmp,
                               double dpi,double minwidth_in,
                               double maxwidth_in,double minheight_in,double anglemax_deg,
                               int white_thresh,int erase_vertical_lines,int debug,int verbose)

    {
    int tc,iangle,irow,icol;
    int rowstep,na,angle_sign,ccthresh;
    int pixmin,halfwidth,bytewidth;
    int bs1,nrsteps;
    double anglestep;
    WILLUSBITMAP *tmp,_tmp;
    unsigned char *p0;
    unsigned char *t0;

    if (debug)
        printf("At bmp_detect_vertical_lines...\n");
    if (!bmp_is_grayscale(bmp))
        {
        printf("Internal error.  bmp_detect_vertical_lines passed a non-grayscale bitmap.\n");
        exit(10);
        }
    tmp=&_tmp;
    bmp_init(tmp);
    bmp_copy(tmp,bmp);
    p0=bmp_rowptr_from_top(bmp,0);
    t0=bmp_rowptr_from_top(tmp,0);
    bytewidth=bmp_bytewidth(bmp);
    pixmin = (int)(minwidth_in*dpi+.5);
    if (pixmin<1)
        pixmin=1;
    halfwidth=pixmin/4;
    if (halfwidth<1)
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
        printf("    na = %d, rowstep = %d, ccthresh = %d, white_thresh = %d, nrsteps=%d\n",na,rowstep,ccthresh,white_thresh,nrsteps);
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
// printf("iangle=%2d, angle_sign=%2d, ic1=%4d, ic2=%4d\n",iangle,angle_sign,ic1,ic2);
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
            printf("    Vert line detected:  ccmax=%d (pix=%d), tanthmax=%g, ic0max=%d, ir0max=%d\n",ccmax,ccmax*rowstep,tanthmax,ic0max,ir0max);
        if (!vert_line_erase(bmp,cbmp,tmp,ir0max,ic0max,tanthmax,minheight_in,
                             minwidth_in,maxwidth_in,white_thresh,dpi,erase_vertical_lines))
            break;
        }
/*
bmp_write(tmp,"outt.png",stdout,95);
wfile_written_info("outt.png",stdout);
bmp_write(bmp,"out2.png",stdout,95);
wfile_written_info("out2.png",stdout);
exit(10);
*/
    }


/*
** Calculate max vert line length.  Line is terminated by nw consecutive white pixels
** on either side.
*/
static int vert_line_erase(WILLUSBITMAP *bmp,WILLUSBITMAP *cbmp,WILLUSBITMAP *tmp,
                    int row0,int col0,double tanth,double minheight_in,
                    double minwidth_in,double maxwidth_in,int white_thresh,
                    double dpi,int erase_vertical_lines)

    {
    int lw,cc,maxdev,nw,dir,i,n;
    int *c1,*c2,*w;
    static char *funcname="vert_line_erase";

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

        brc = 0;
        for (del=(dir==-1)?0:1;1;del++)
            {
            int r,c;
            unsigned char *p;

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
    if (n>1)
        sorti(w,n);
    if (n < 10 || n < minheight_in*dpi
               || w[n/4] < minwidth_in*dpi 
               || w[3*n/4] > maxwidth_in*dpi
               || (erase_vertical_lines==1 && w[n-1] > maxwidth_in*dpi))
        {
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
        /* Erase line width in source bitmap */
        lw=w[3*n/4]+nw*2;
        if (lw > maxwidth_in*dpi/2)
            lw=maxwidth_in*dpi/2; 
        for (i=0;i<bmp->height;i++)
            {
            unsigned char *p;
            int c0,cmin,cmax,count,white;

            if (c1[i]<0 || c2[i]<0)
                continue;
            c0=col0+(i-row0)*tanth;
            cmin=c0-lw-1;
            if (cmin<c1[i])
                cmin=c1[i];
            cmax=c0+lw+1;
            if (cmax>c2[i])
                cmax=c2[i];
            p=bmp_rowptr_from_top(bmp,i);
            c0 = (p[cmin] > p[cmax]) ? cmin : cmax;
            white=p[c0];
            if (white <= white_thresh)
                white = white_thresh+1;
            if (white>255)
                white=255;
            count=(cmax-cmin)+1;
            p=&p[cmin];
            for (;count>0;count--,p++)
                (*p)=white;
            if (cbmp!=NULL)
                {
                unsigned char *p0;
                p=bmp_rowptr_from_top(cbmp,i);
                p0=p+c0*3;
                p=p+cmin*3;
                count=(cmax-cmin)+1;
                for (;count>0;count--,p+=3)
                    {
                    p[0]=p0[0];
                    p[1]=p0[1];
                    p[2]=p0[2];
                    }
                }
            }
        }
    willus_dmem_free(26,(double **)&c1,funcname);
    return(1);
    }
