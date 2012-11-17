/*
** wrapbmp.c    Functions to store individual word bitmaps into a collecting
**              bitmap for text re-flow.
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

static void wrapbmp_hyphen_erase(WRAPBMP *wrapbmp,K2PDFOPT_SETTINGS *k2settings);

void wrapbmp_init(WRAPBMP *wrapbmp,int color)

    {
    int i;

    bmp_init(&wrapbmp->bmp);
    wrapbmp->bmp.width=0;
    wrapbmp->bmp.height=0;
    for (i=0;i<256;i++)
        wrapbmp->bmp.red[i]=wrapbmp->bmp.blue[i]=wrapbmp->bmp.green[i]=i;
    wrapbmp_set_color(wrapbmp,color);
    wrapbmp->base=0;
    wrapbmp->line_spacing=-1;
    wrapbmp->gap=-1;
    wrapbmp->bgcolor=-1;
    wrapbmp->height_extended=0;
    wrapbmp->just=0x8f;
    wrapbmp->rhmax=-1;
    wrapbmp->thmax=-1;
    wrapbmp->hyphen.ch=-1;
    wrapbmp->maxgap=2;
    wrapbmp->just_flushed_internal=0;
    wrapbmp->beginning_gap_internal=-1;
    wrapbmp->last_h5050_internal=-1;
    }


int wrapbmp_ends_in_hyphen(WRAPBMP *wrapbmp)

    {
    return(wrapbmp->hyphen.ch>=0);
    }


void wrapbmp_set_color(WRAPBMP *wrapbmp,int is_color)

    {
    wrapbmp->bmp.bpp=is_color ? 24 : 8;
    }


void wrapbmp_free(WRAPBMP *wrapbmp)

    {
    bmp_free(&wrapbmp->bmp);
    }


void wrapbmp_set_maxgap(WRAPBMP *wrapbmp,int value)

    {
    wrapbmp->maxgap=value;
    }


int wrapbmp_width(WRAPBMP *wrapbmp)

    {
    return(wrapbmp->bmp.width);
    }


int wrapbmp_remaining(WRAPBMP *wrapbmp,K2PDFOPT_SETTINGS *k2settings)

    {
    int maxpix,w;
    maxpix=k2settings->max_region_width_inches*k2settings->src_dpi;
    /* Don't include hyphen if wrapbmp ends in a hyphen */
    if (wrapbmp->hyphen.ch<0)
        w=wrapbmp->bmp.width;
    else
        if (k2settings->src_left_to_right)
            w=wrapbmp->hyphen.c2+1;
        else
            w=wrapbmp->bmp.width - wrapbmp->hyphen.c2;
    return(maxpix-w);
    }


/*
** region = bitmap region to add to line
** gap = horizontal pixel gap between existing region and region being added
** line_spacing = desired spacing between lines of text (pixels)
** rbase = position of baseline in region
** gio = gap if over--gap above top of text if it goes over line_spacing.
*/
// static int bcount=0;
void wrapbmp_add(WRAPBMP *wrapbmp,BMPREGION *region,K2PDFOPT_SETTINGS *k2settings,
                 int gap,int line_spacing,int rbase,int gio,int just_flags)

    {
    WILLUSBITMAP *tmp,_tmp;
    int i,rh,th,bw,new_base,h2,bpp,width0;
// static char filename[256];

#ifdef WILLUSDEBUG
printf("@wrapbmp->add %d x %d (w=%d).\n",region->c2-region->c1+1,region->r2-region->r1+1,wrapbmp->bmp.width);
#endif
    /* Figure out if what we're adding ends in a hyphen */
    bmpregion_hyphen_detect(region,k2settings->hyphen_detect,k2settings->src_left_to_right);
    if (wrapbmp_ends_in_hyphen(wrapbmp))
        gap=0;
    wrapbmp_hyphen_erase(wrapbmp,k2settings);
    wrapbmp->just_flushed_internal=0;  // Reset "just flushed" flag
    wrapbmp->beginning_gap_internal = -1; // Reset top-of-page or top-of-column gap
    wrapbmp->last_h5050_internal = -1; // Reset last row font size
    if (line_spacing > wrapbmp->line_spacing)
        wrapbmp->line_spacing = line_spacing;
    if (gio > wrapbmp->gap)
        wrapbmp->gap = gio;
    wrapbmp->bgcolor=region->bgcolor;
    wrapbmp->just=just_flags;
/*
printf("    c1=%d, c2=%d, r1=%d, r2=%d\n",region->c1,region->c2,region->r1,region->r2);
printf("    gap=%d, line_spacing=%d, rbase=%d, gio=%d\n",gap,line_spacing,rbase,gio);
*/
    bpp=k2settings->dst_color?3:1;
    rh=rbase-region->r1+1;
    if (rh > wrapbmp->rhmax)
        wrapbmp->rhmax=rh;
    th = rh + (region->r2-rbase);
    if (th > wrapbmp->thmax)
        wrapbmp->thmax=th;
/*
{
WILLUSBITMAP *bmp,_bmp;

bmp=&_bmp;
bmp_init(bmp);
bmp->height=region->r2-region->r1+1;
bmp->width=region->c2-region->c1+1;
bmp->bpp=bpp*8;
if (bpp==1)
for (i=0;i<256;i++)
bmp->red[i]=bmp->blue[i]=bmp->green[i]=i;
bmp_alloc(&wrapbmp->bmp);
bw=bmp_bytewidth(&wrapbmp->bmp);
memset(bmp_rowptr_from_top(bmp,0),255,bw*bmp->height);
for (i=region->r1;i<=region->r2;i++)
{
unsigned char *d,*s;
d=bmp_rowptr_from_top(bmp,i-region->r1);
s=bmp_rowptr_from_top(k2settings->dst_color?region->bmp:region->bmp8,i)+bpp*region->c1;
if (i==rbase)
memset(d,0,bw);
else
memcpy(d,s,bw);
}
sprintf(filename,"out%05d.png",bcount++);
bmp_write(bmp,filename,stdout,100);
bmp_free(bmp);
}
*/
    if (wrapbmp->bmp.width==0)
        {
        /* Put appropriate gap in */
        if (k2settings->last_rowbase_internal>=0 && rh < wrapbmp->line_spacing-k2settings->last_rowbase_internal)
            {
            rh = wrapbmp->line_spacing - k2settings->last_rowbase_internal;
            if (rh<2)
                rh=2;
            th = rh + (region->r2-rbase);
            wrapbmp->height_extended=0;
            }
        else
            wrapbmp->height_extended=(k2settings->last_rowbase_internal>=0);
        wrapbmp->base = rh-1;
        wrapbmp->bmp.height = th;
#ifdef WILLUSDEBUG
printf("@wrapbmp->add:  bmpheight set to %d (wls=%d, lrbi=%d)\n",wrapbmp->bmp.height,wrapbmp->line_spacing,k2settings->last_rowbase_internal);
#endif
        wrapbmp->bmp.width=region->c2-region->c1+1;
        bmp_alloc(&wrapbmp->bmp);
        bw=bmp_bytewidth(&wrapbmp->bmp);
        memset(bmp_rowptr_from_top(&wrapbmp->bmp,0),255,bw*wrapbmp->bmp.height);
        for (i=region->r1;i<=region->r2;i++)
            {
            unsigned char *d,*s;
            d=bmp_rowptr_from_top(&wrapbmp->bmp,wrapbmp->base+(i-rbase));
            s=bmp_rowptr_from_top(k2settings->dst_color?region->bmp:region->bmp8,i)+bpp*region->c1;
            memcpy(d,s,bw);
            }
#ifdef WILLUSDEBUG
if (wrapbmp->bmp.height<=wrapbmp->base)
{
printf("1. SCREEECH!\n");
printf("wrapbmp = %d x %d, base=%d\n",wrapbmp->bmp.width,wrapbmp->bmp.height,wrapbmp->base);
exit(10);
}
#endif
        /* Copy hyphen info from added region */
        wrapbmp->hyphen = region->hyphen;
        if (wrapbmp_ends_in_hyphen(wrapbmp))
            {
            wrapbmp->hyphen.r1 += (wrapbmp->base-rbase);
            wrapbmp->hyphen.r2 += (wrapbmp->base-rbase);
            wrapbmp->hyphen.ch -= region->c1;
            wrapbmp->hyphen.c2 -= region->c1;
            }
        return;
        }
    width0=wrapbmp->bmp.width; /* Starting wrapbmp width */
    tmp=&_tmp;
    bmp_init(tmp);
    bmp_copy(tmp,&wrapbmp->bmp);
    tmp->width += gap+region->c2-region->c1+1;
    if (rh > wrapbmp->base)
        {
        wrapbmp->height_extended=1;
        new_base = rh-1;
        }
    else
        new_base = wrapbmp->base;
    if (region->r2-rbase > wrapbmp->bmp.height-1-wrapbmp->base)
        h2=region->r2-rbase;
    else
        h2=wrapbmp->bmp.height-1-wrapbmp->base;
    tmp->height = new_base + h2 + 1;
    bmp_alloc(tmp);
    bw=bmp_bytewidth(tmp);
    memset(bmp_rowptr_from_top(tmp,0),255,bw*tmp->height);
    bw=bmp_bytewidth(&wrapbmp->bmp);
/*
printf("3.  wbh=%d x %d, tmp=%d x %d x %d, new_base=%d, wbbase=%d\n",wrapbmp->bmp.width,wrapbmp->bmp.height,tmp->width,tmp->height,tmp->bpp,new_base,wrapbmp->base);
*/
    for (i=0;i<wrapbmp->bmp.height;i++)
        {
        unsigned char *d,*s;
        d=bmp_rowptr_from_top(tmp,i+new_base-wrapbmp->base)
                 + (k2settings->src_left_to_right ? 0 : tmp->width-1-wrapbmp->bmp.width)*bpp;
        s=bmp_rowptr_from_top(&wrapbmp->bmp,i);
        memcpy(d,s,bw);
        }
    bw=bpp*(region->c2-region->c1+1);
    if (region->r1+new_base-rbase<0 || region->r2+new_base-rbase>tmp->height-1)
        {
        aprintf(ANSI_YELLOW "INTERNAL ERROR--TMP NOT DIMENSIONED PROPERLY.\n");
        aprintf("(%d-%d), tmp->height=%d\n" ANSI_NORMAL,
            region->r1+new_base-rbase,
            region->r2+new_base-rbase,tmp->height);
        exit(10);
        }
    for (i=region->r1;i<=region->r2;i++)
        {
        unsigned char *d,*s;

        d=bmp_rowptr_from_top(tmp,i+new_base-rbase)
                 + (k2settings->src_left_to_right ? wrapbmp->bmp.width+gap : 0)*bpp;
        s=bmp_rowptr_from_top(k2settings->dst_color?region->bmp:region->bmp8,i)+bpp*region->c1;
        memcpy(d,s,bw);
        }
    bmp_copy(&wrapbmp->bmp,tmp);
    bmp_free(tmp);
    /* Copy region's hyphen info */
    wrapbmp->hyphen = region->hyphen;
    if (wrapbmp_ends_in_hyphen(wrapbmp))
        {
        wrapbmp->hyphen.r1 += (new_base-rbase);
        wrapbmp->hyphen.r2 += (new_base-rbase);
        if (k2settings->src_left_to_right)
            {
            wrapbmp->hyphen.ch += width0+gap-region->c1;
            wrapbmp->hyphen.c2 += width0+gap-region->c1;
            }
        else
            {
            wrapbmp->hyphen.ch -= region->c1;
            wrapbmp->hyphen.c2 -= region->c1;
            }
        }
    wrapbmp->base=new_base;
#ifdef WILLUSDEBUG
if (wrapbmp->bmp.height<=wrapbmp->base)
{
printf("2. SCREEECH!\n");
printf("wrapbmp = %d x %d, base=%d\n",wrapbmp->bmp.width,wrapbmp->bmp.height,wrapbmp->base);
exit(10);
}
#endif
    }


void wrapbmp_flush(MASTERINFO *masterinfo,K2PDFOPT_SETTINGS *k2settings,
                   int allow_full_justification,int use_bgi)

    {
    BMPREGION region;
    WILLUSBITMAP *bmp8,_bmp8;
    int gap,just,nomss,dh;
    int *colcount,*rowcount;
    static char *funcname="wrapbmp->flush";
    WRAPBMP *wrapbmp;
// char filename[256];

    wrapbmp=&masterinfo->wrapbmp;
    if (wrapbmp->bmp.width<=0)
        {
        if (use_bgi==1 && wrapbmp->beginning_gap_internal > 0)
            masterinfo_add_gap_src_pixels(masterinfo,k2settings,
                                   wrapbmp->beginning_gap_internal,"wrapbmp->bgi0");
        wrapbmp->beginning_gap_internal=-1;
        wrapbmp->last_h5050_internal=-1;
        if (use_bgi)
            wrapbmp->just_flushed_internal=1;
        return;
        }
#ifdef WILLUSDEBUG
printf("@wrapbmp->flush()\n");
#endif
/*
{
char filename[256];
int i;
static int bcount=0;
for (i=0;i<wrapbmp->bmp.height;i++)
{
unsigned char *p;
int j;
p=bmp_rowptr_from_top(&wrapbmp->bmp,i);
for (j=0;j<wrapbmp->bmp.width;j++)
if (p[j]>240)
    p[j]=192;
}
sprintf(filename,"out%05d.png",bcount++);
bmp_write(wrapbmp,filename,stdout,100);
}
*/
    colcount=rowcount=NULL;
    willus_dmem_alloc_warn(19,(void **)&colcount,(wrapbmp->bmp.width+16)*sizeof(int),funcname,10);
    willus_dmem_alloc_warn(20,(void **)&rowcount,(wrapbmp->bmp.height+16)*sizeof(int),funcname,10);
    region.c1=0;
    region.c2=wrapbmp->bmp.width-1;
    region.r1=0;
    region.r2=wrapbmp->bmp.height-1;
    region.rowbase=wrapbmp->base;
    region.bmp=&wrapbmp->bmp;
    region.bgcolor=wrapbmp->bgcolor;
    region.dpi=k2settings->src_dpi;
#ifdef WILLUSDEBUG
printf("Bitmap is %d x %d (baseline=%d)\n",wrapbmp->bmp.width,wrapbmp->bmp.height,wrapbmp->base);
#endif

    /* Sanity check on row spacing -- don't let it be too large. */
    nomss = wrapbmp->rhmax*1.7; /* Nominal single-spaced height for this row */
    if (k2settings->last_rowbase_internal<0)
        dh = 0;
    else
        {
        dh=(int)(wrapbmp->line_spacing-k2settings->last_rowbase_internal 
                           - 1.2*fabs(k2settings->vertical_line_spacing)*nomss +.5);
        if (k2settings->vertical_line_spacing < 0.)
            {
            int dh1;
            if (wrapbmp->maxgap > 0)
                dh1 = region.rowbase+1-wrapbmp->rhmax-wrapbmp->maxgap;
            else
                dh1=(int)(wrapbmp->line_spacing-k2settings->last_rowbase_internal- 1.2*nomss+.5);
            if (dh1 > dh)
                dh =dh1;
            }
        }
    if (dh>0)
{
#ifdef WILLUSDEBUG
aprintf(ANSI_YELLOW "dh > 0 = %d" ANSI_NORMAL "\n",dh);
printf("    wrapbmp->line_spacing=%d\n",wrapbmp->line_spacing);
printf("    nomss = %d\n",nomss);
printf("    vls = %g\n",k2settings->vertical_line_spacing);
printf("    lrbi=%d\n",k2settings->last_rowbase_internal);
printf("    wrapbmp->maxgap=%d\n",wrapbmp->maxgap);
printf("    wrapbmp->rhmax=%d\n",wrapbmp->rhmax);
#endif
        region.r1 = dh;
/*
if (dh>200)
{
bmp_write(wrapbmp,"out.png",stdout,100);
exit(10);
}
*/
}
    if (wrapbmp->bmp.bpp==24)
        {
        bmp8=&_bmp8;
        bmp_init(bmp8);
        bmp_convert_to_greyscale_ex(bmp8,&wrapbmp->bmp);
        region.bmp8=bmp8;
        }
    else
        region.bmp8=&wrapbmp->bmp;
    if (k2settings->gap_override_internal > 0)
        {
        region.r1=wrapbmp->base-wrapbmp->rhmax+1;
        if (region.r1<0)
            region.r1=0;
        if (region.r1>wrapbmp->base)
            region.r1=wrapbmp->base;
        gap=k2settings->gap_override_internal;
        k2settings->gap_override_internal = -1;
        }
    else
        {
        if (wrapbmp->height_extended)
            gap = wrapbmp->gap;
        else
            gap = 0;
        }
#ifdef WILLUSDEBUG
printf("wf:  gap=%d\n",gap);
#endif
    if (gap>0)
        masterinfo_add_gap_src_pixels(masterinfo,k2settings,gap,"wrapbmp");
    if (!allow_full_justification)
        just = (wrapbmp->just & 0xcf) | 0x20;
    else
        just = wrapbmp->just;
    /*
    ** For now, set pageinfo=NULL in calls to bmpregion_add because the
    ** pageinfo processing assumes that the BMPREGION structure it is working
    ** with is using the original source bitmap, not the wrapbmp bitmap.
    ** This means that word wrapping can't use the pageinfo structure for now.
    */
    bmpregion_add(&region,k2settings,NULL,masterinfo,0,0,0,-1.0,just,2,
                  colcount,rowcount,0xf,wrapbmp->bmp.height-1-wrapbmp->base);
    if (wrapbmp->bmp.bpp==24)
        bmp_free(bmp8);
    willus_dmem_free(20,(double **)&rowcount,funcname);
    willus_dmem_free(19,(double **)&colcount,funcname);
    wrapbmp->bmp.width=0;
    wrapbmp->bmp.height=0;
    wrapbmp->line_spacing=-1;
    wrapbmp->gap=-1;
    wrapbmp->rhmax=-1;
    wrapbmp->thmax=-1;
    wrapbmp->hyphen.ch=-1;
    if (use_bgi==1 && wrapbmp->beginning_gap_internal > 0)
        masterinfo_add_gap_src_pixels(masterinfo,k2settings,
                               wrapbmp->beginning_gap_internal,"wrapbmp->bgi1");
    wrapbmp->beginning_gap_internal = -1;
    wrapbmp->last_h5050_internal = -1;
    if (use_bgi)
        wrapbmp->just_flushed_internal=1;
    }


static void wrapbmp_hyphen_erase(WRAPBMP *wrapbmp,K2PDFOPT_SETTINGS *k2settings)

    {
    WILLUSBITMAP *bmp,_bmp;
    int bw,bpp,c0,c1,c2,i;

    if (wrapbmp->hyphen.ch<0)
        return;
#if (WILLUSDEBUGX & 16)
printf("@hyphen_erase, bmp=%d x %d x %d\n",wrapbmp->bmp.width,wrapbmp->bmp.height,wrapbmp->bmp.bpp);
printf("    ch=%d, c2=%d, r1=%d, r2=%d\n",wrapbmp->hyphen.ch,wrapbmp->hyphen.c2,wrapbmp->hyphen.r1,wrapbmp->hyphen.r2);
#endif
    bmp=&_bmp;
    bmp_init(bmp);
    bmp->bpp = wrapbmp->bmp.bpp;
    if (bmp->bpp==8)
        for (i=0;i<256;i++)
            bmp->red[i]=bmp->blue[i]=bmp->green[i]=i;
    bmp->height = wrapbmp->bmp.height;
    if (k2settings->src_left_to_right)
        {
        bmp->width = wrapbmp->hyphen.c2+1;
        c0=0;
        c1=wrapbmp->hyphen.ch;
        c2=bmp->width-1;
        }
    else
        {
        bmp->width = wrapbmp->bmp.width - wrapbmp->hyphen.c2;
        c0=wrapbmp->hyphen.c2;
        c1=0;
        c2=wrapbmp->hyphen.ch-wrapbmp->hyphen.c2;
        }
    bmp_alloc(bmp);
    bpp=bmp->bpp==24 ? 3 : 1;
    bw=bpp*bmp->width;
    for (i=0;i<bmp->height;i++)
        memcpy(bmp_rowptr_from_top(bmp,i),bmp_rowptr_from_top(&wrapbmp->bmp,i)+bpp*c0,bw);
    bw=(c2-c1+1)*bpp;
    if (bw>0)
        for (i=wrapbmp->hyphen.r1;i<=wrapbmp->hyphen.r2;i++)
            memset(bmp_rowptr_from_top(bmp,i)+bpp*c1,255,bw);
#if (WILLUSDEBUGX & 16)
{
static int count=1;
char filename[256];
sprintf(filename,"be%04d.png",count);
bmp_write(&wrapbmp->bmp,filename,stdout,100);
sprintf(filename,"ae%04d.png",count);
bmp_write(bmp,filename,stdout,100);
count++;
}
#endif
    bmp_copy(&wrapbmp->bmp,bmp);
    bmp_free(bmp);
    }
