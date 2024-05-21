/*
** wrapbmp.c    Functions to store individual word bitmaps into a collecting
**              bitmap for text re-flow.
**
** Copyright (C) 2019  http://willus.com
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

static void wrapbmp_reset(WRAPBMP *wrapbmp);
static void wrapbmp_hyphen_erase(WRAPBMP *wrapbmp,K2PDFOPT_SETTINGS *k2settings);
static double wrectmap_hcompare(WRECTMAP *x1,WRECTMAP *x2);


void wrapbmp_init(WRAPBMP *wrapbmp,int color)

    {
    int i;

    bmp_init(&wrapbmp->bmp);
    for (i=0;i<256;i++)
        wrapbmp->bmp.red[i]=wrapbmp->bmp.blue[i]=wrapbmp->bmp.green[i]=i;
    wrapbmp_set_color(wrapbmp,color);
    wrectmaps_init(&wrapbmp->wrectmaps);
    wrapbmp->bgcolor=-1;
    wrapbmp->just=0x8f;
    wrapbmp_reset(wrapbmp);
    wrapbmp->just_flushed_internal=0;
    }


static void wrapbmp_reset(WRAPBMP *wrapbmp)

    {
    wrapbmp->bmp.width=0;
    wrapbmp->bmp.height=0;
    wrapbmp->base=0;
    wrapbmp->maxgap=2;
    wrapbmp->rhmax=-1;
    wrapbmp->thmax=-1;
    wrapbmp->hyphen.ch=-1;
    /* wrapbmp->height_extended=0; *//* Not used anymore as of v2.00 */
    wrapbmp->mandatory_region_gap=-1;
    wrapbmp->page_region_gap_in=-1.;
    wrapbmp->textrow.rowheight=-1;
    wrapbmp->textrow.gap=-1;
    wrapbmp->textrow.gapblank=0;
    wrapbmp->just_flushed_internal=1;
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
    wrectmaps_free(&wrapbmp->wrectmaps);
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
**          Region must have had full bbox calculated already.
** colgap = horizontal pixel gap between existing region and region being added
*/
// static int bcount=0;
void wrapbmp_add(WRAPBMP *wrapbmp,BMPREGION *region,K2PDFOPT_SETTINGS *k2settings,
                 MASTERINFO *masterinfo,int colgap,int just_flags)

    {
    WILLUSBITMAP *tmp,_tmp;
    int i,rh,th,bw,new_base,h2,bpp,width0;
// static char filename[MAXFILENAMELEN];

#if (WILLUSDEBUGX & 205)
k2printf("@wrapbmp->add %d x %d (w=%d).\n",region->c2-region->c1+1,region->r2-region->r1+1,wrapbmp->bmp.width);
printf("    line_spacing=%d\n",region->bbox.rowheight);
#endif
    /* Figure out if what we're adding ends in a hyphen */
    bmpregion_hyphen_detect(region,k2settings->hyphen_detect,k2settings->src_left_to_right);
    if (wrapbmp_ends_in_hyphen(wrapbmp))
        colgap=0;
    wrapbmp_hyphen_erase(wrapbmp,k2settings);
    wrapbmp->just_flushed_internal=0;  // Reset "just flushed" flag
    if (wrapbmp->bmp.width==0)
        wrapbmp->textrow=region->bbox;
    else
        {
        if (region->bbox.rowheight > wrapbmp->textrow.rowheight)
            wrapbmp->textrow.rowheight = region->bbox.rowheight;
        if (region->bbox.gap > wrapbmp->textrow.gap)
            wrapbmp->textrow.gap = region->bbox.gap;
        if (region->bbox.gapblank > wrapbmp->textrow.gapblank)
            wrapbmp->textrow.gapblank = region->bbox.gapblank;
        }
    wrapbmp->bgcolor=region->bgcolor;
    wrapbmp->just=just_flags;
    if (wrapbmp->mandatory_region_gap<0)
        {
        wrapbmp->mandatory_region_gap=masterinfo->mandatory_region_gap;
        wrapbmp->page_region_gap_in=masterinfo->page_region_gap_in;
        masterinfo->mandatory_region_gap=0;
        masterinfo->page_region_gap_in=-1.;
#if (WILLUSDEBUGX & 0x200)
aprintf(ANSI_RED "mi->mandatory_region_gap change to %d by wrap_add." ANSI_NORMAL "\n",masterinfo->mandatory_region_gap);
#endif
        }
#if (WILLUSDEBUGX & 4)
k2printf("    c1=%d, c2=%d, r1=%d, r2=%d\n",region->c1,region->c2,region->r1,region->r2);
k2printf("    colgap=%d, line_spacing=%d, rowbase=%d, row gap=%d\n",colgap,region->bbox.rowheight,region->bbox.rowbase,region->bbox.gap);
#endif
    bpp=k2settings->dst_color?region->bmp->bpp/8:region->bmp8->bpp/8;
    rh=region->bbox.rowbase-region->r1+1;
    if (rh > wrapbmp->rhmax)
        wrapbmp->rhmax=rh;
    th = rh + (region->r2-region->bbox.rowbase);
    if (th > wrapbmp->thmax)
        wrapbmp->thmax=th;
#if (WILLUSDEBUGX & 4)
{
static int bcount=0;
char filename[MAXFILENAMELEN];
sprintf(filename,"out%05d.png",bcount++);
bmpregion_write(region,filename);
}
#endif
    if (wrapbmp->bmp.width==0)
        {
        /* Put appropriate gap in */
/*
        if (k2settings->last_rowbase_internal>=0 && rh < wrapbmp->textrow.rowheight-k2settings->last_rowbase_internal)
            {
            rh = wrapbmp->textrow.rowheight - k2settings->last_rowbase_internal;
            if (rh<2)
                rh=2;
            th = rh + (region->r2-region->bbox.rowbase);
            wrapbmp->height_extended=0;
            }
        else
            wrapbmp->height_extended=(k2settings->last_rowbase_internal>=0);
*/
        wrapbmp->base = rh-1;
        wrapbmp->bmp.height = th;
#if (WILLUSDEBUGX & 4)
k2printf("    bmpheight set to %d (line spacing=%d)\n",wrapbmp->bmp.height,wrapbmp->textrow.rowheight);
#endif
        wrapbmp->bmp.width=region->c2-region->c1+1;
        bmp_alloc(&wrapbmp->bmp);
        bw=bmp_bytewidth(&wrapbmp->bmp);
        memset(bmp_rowptr_from_top(&wrapbmp->bmp,0),255,bw*wrapbmp->bmp.height);
        for (i=region->r1;i<=region->r2;i++)
            {
            unsigned char *d,*s;
            d=bmp_rowptr_from_top(&wrapbmp->bmp,wrapbmp->base+(i-region->bbox.rowbase));
            s=bmp_rowptr_from_top(k2settings->dst_color?region->bmp:region->bmp8,i)+bpp*region->c1;
            memcpy(d,s,bw);
            }
#ifdef WILLUSDEBUG
if (wrapbmp->bmp.height<=wrapbmp->base)
{
k2printf("1. SCREEECH!\n");
k2printf("wrapbmp = %d x %d, base=%d\n",wrapbmp->bmp.width,wrapbmp->bmp.height,wrapbmp->base);
exit(10);
}
#endif
        /* Copy hyphen info from added region */
        wrapbmp->hyphen = region->bbox.hyphen;
        if (wrapbmp_ends_in_hyphen(wrapbmp))
            {
            wrapbmp->hyphen.r1 += (wrapbmp->base-region->bbox.rowbase);
            wrapbmp->hyphen.r2 += (wrapbmp->base-region->bbox.rowbase);
            wrapbmp->hyphen.ch -= region->c1;
            wrapbmp->hyphen.c2 -= region->c1;
            }
        /* Map to source page */
        {
        WRECTMAP _wrmap,*wrmap;

        wrmap=&_wrmap;
        wrmap->srcpageno = region->pageno;
        wrmap->srcwidth = region->bmp8->width;
        wrmap->srcheight = region->bmp8->height;
        wrmap->srcdpiw = wrmap->srcdpih = region->dpi;
        wrmap->srcrot = region->rotdeg;
        wrmap->coords[0].x = region->c1;
        wrmap->coords[0].y = region->r1;
        wrmap->coords[1].x = 0;
        wrmap->coords[1].y = wrapbmp->base+(region->r1-region->bbox.rowbase);
        wrmap->coords[2].x = region->c2-region->c1+1;
        wrmap->coords[2].y = region->r2-region->r1+1;
#if (WILLUSDEBUGX & 0x400)
    printf("@wrapbmp:  (x0,y0) = (%5.1f,%5.1f)\n",region->c1*72./region->dpi,region->r1*72./region->dpi);
printf("      %5.1f x %5.1f\n",(region->c2-region->c1+1)*72./region->dpi,(region->r2-region->r1+1)*72./region->dpi);
printf("      New bitmap = %d x %d\n",wrapbmp->bmp.width,wrapbmp->bmp.height);
#endif
        wrectmaps_add_wrectmap(&wrapbmp->wrectmaps,wrmap);
        }
        return;
        }
    width0=wrapbmp->bmp.width; /* Starting wrapbmp width */
    tmp=&_tmp;
    bmp_init(tmp);
    bmp_copy(tmp,&wrapbmp->bmp);
    tmp->width += colgap+region->c2-region->c1+1;
    if (rh > wrapbmp->base)
        {
        /* wrapbmp->height_extended=1; */
        new_base = rh-1;
        }
    else
        new_base = wrapbmp->base;
    if (region->r2-region->bbox.rowbase > wrapbmp->bmp.height-1-wrapbmp->base)
        h2=region->r2-region->bbox.rowbase;
    else
        h2=wrapbmp->bmp.height-1-wrapbmp->base;
    tmp->height = new_base + h2 + 1;
    bmp_alloc(tmp);
    bw=bmp_bytewidth(tmp);
    memset(bmp_rowptr_from_top(tmp,0),255,bw*tmp->height);
    bw=bmp_bytewidth(&wrapbmp->bmp);
#if (WILLUSDEBUGX & 4)
k2printf("3.  wbh=%d x %d, tmp=%d x %d x %d, new_base=%d, wbbase=%d\n",wrapbmp->bmp.width,wrapbmp->bmp.height,tmp->width,tmp->height,tmp->bpp,new_base,wrapbmp->base);
#endif

    /* Adjust previous mappings to source pages since WRAPBMP rectangle has been re-sized */
    if (new_base!=wrapbmp->base)
        for (i=0;i<wrapbmp->wrectmaps.n;i++)
            {
            wrapbmp->wrectmaps.wrectmap[i].coords[1].y += (new_base-wrapbmp->base);
            if (k2settings->src_left_to_right==0)
                wrapbmp->wrectmaps.wrectmap[i].coords[1].x += tmp->width-1-wrapbmp->bmp.width;
            }
    for (i=0;i<wrapbmp->bmp.height;i++)
        {
        unsigned char *d,*s;
        d=bmp_rowptr_from_top(tmp,i+new_base-wrapbmp->base)
                 + (k2settings->src_left_to_right ? 0 : tmp->width-1-wrapbmp->bmp.width)*bpp;
        s=bmp_rowptr_from_top(&wrapbmp->bmp,i);
        memcpy(d,s,bw);
        }
    bw=bpp*(region->c2-region->c1+1);
    if (region->r1+new_base-region->bbox.rowbase<0 || region->r2+new_base-region->bbox.rowbase>tmp->height-1)
        {
        k2printf(ANSI_YELLOW "INTERNAL ERROR--TMP NOT DIMENSIONED PROPERLY.\n");
        k2printf("(%d-%d), tmp->height=%d\n" ANSI_NORMAL,
            region->r1+new_base-region->bbox.rowbase,
            region->r2+new_base-region->bbox.rowbase,tmp->height);
        exit(10);
        }
    for (i=region->r1;i<=region->r2;i++)
        {
        unsigned char *d,*s;

        d=bmp_rowptr_from_top(tmp,i+new_base-region->bbox.rowbase)
                 + (k2settings->src_left_to_right ? wrapbmp->bmp.width+colgap : 0)*bpp;
        s=bmp_rowptr_from_top(k2settings->dst_color?region->bmp:region->bmp8,i)+bpp*region->c1;
        memcpy(d,s,bw);
        }
    {
    WRECTMAP _wrmap,*wrmap;

    wrmap=&_wrmap;
    wrmap->srcpageno = region->pageno;
    wrmap->srcwidth = region->bmp8->width;
    wrmap->srcheight = region->bmp8->height;
    wrmap->srcdpiw = wrmap->srcdpih = region->dpi;
    wrmap->srcrot = region->rotdeg;
    wrmap->coords[0].x = region->c1;
    wrmap->coords[0].y = region->r1;
    wrmap->coords[1].x = k2settings->src_left_to_right ? wrapbmp->bmp.width+colgap : 0;
    wrmap->coords[1].y = region->r1+new_base-region->bbox.rowbase;
    wrmap->coords[2].x = region->c2-region->c1+1;
    wrmap->coords[2].y = region->r2-region->r1+1;
#if (WILLUSDEBUGX & 0x400)
printf("@wrapbmp:  (x0,y0) = (%5.1f,%5.1f)\n",region->c1*72./region->dpi,region->r1*72./region->dpi);
printf("      org bmp = %d x %d\n",wrapbmp->bmp.width,wrapbmp->bmp.height);
printf("      new_base=%d, r_base=%d\n",new_base,region->bbox.rowbase);
printf("      (x1,y1) = (%g,%g)\n",wrmap->coords[1].x,wrmap->coords[1].y);
printf("      %5.1f x %5.1f\n",(region->c2-region->c1+1)*72./region->dpi,(region->r2-region->r1+1)*72./region->dpi);
printf("      New bitmap = %d x %d\n",tmp->width,tmp->height);
#endif
    wrectmaps_add_wrectmap(&wrapbmp->wrectmaps,wrmap);
    }
    bmp_copy(&wrapbmp->bmp,tmp);
#if (WILLUSDEBUGX & 4)
{
static int rcount=0;
char filename[MAXFILENAMELEN];
sprintf(filename,"result%03d.png",rcount++);
bmp_write(tmp,filename,stdout,100);
}
#endif
    bmp_free(tmp);
    /* Copy region's hyphen info */
    wrapbmp->hyphen = region->bbox.hyphen;
    if (wrapbmp_ends_in_hyphen(wrapbmp))
        {
        wrapbmp->hyphen.r1 += (new_base-region->bbox.rowbase);
        wrapbmp->hyphen.r2 += (new_base-region->bbox.rowbase);
        if (k2settings->src_left_to_right)
            {
            wrapbmp->hyphen.ch += width0+colgap-region->c1;
            wrapbmp->hyphen.c2 += width0+colgap-region->c1;
            }
        else
            {
            wrapbmp->hyphen.ch -= region->c1;
            wrapbmp->hyphen.c2 -= region->c1;
            }
        }
    wrapbmp->base=new_base;
    /* Doesn't seem to work?  v2.00 */
    /*
    wrapbmp->textrow.gap = (wrapbmp->bmp.height-1)-new_base;
    wrapbmp->textrow.gapblank = 0;
    */
#ifdef WILLUSDEBUG
if (wrapbmp->bmp.height<=wrapbmp->base)
{
k2printf("2. SCREEECH!\n");
k2printf("wrapbmp = %d x %d, base=%d\n",wrapbmp->bmp.width,wrapbmp->bmp.height,wrapbmp->base);
exit(10);
}
#endif
    }


void wrapbmp_flush(MASTERINFO *masterinfo,K2PDFOPT_SETTINGS *k2settings,
                   int allow_full_justification)

    {
    BMPREGION region;
    WILLUSBITMAP *bmp8,_bmp8;
    int just;
    WRAPBMP *wrapbmp;
    /*
    int gap,nomss,dh;
    */
// char filename[MAXFILENAMELEN];

#if (WILLUSDEBUGX & 4)
k2printf("@wrapbmp_flush()\n");
#endif
    /* Ignore if not re-flowing text */
    if (!k2settings->text_wrap)
        return;
    wrapbmp=&masterinfo->wrapbmp;
    if (wrapbmp==NULL || wrapbmp->just_flushed_internal)
        return;
#if (WILLUSDEBUGX & 4)
k2printf("    wrapbmp=%p\n",wrapbmp);
k2printf("    wrapbmp->bmp.width=%d\n",wrapbmp->bmp.width);
#endif
    if (wrapbmp->bmp.width<=0)
        {
        wrapbmp->just_flushed_internal=1;
        return;
        }
#if (WILLUSDEBUGX & 4)
k2printf("    Past width check\n");
#endif
/*
{
char filename[MAXFILENAMELEN];
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
    bmpregion_init(&region);
    region.c1=0;
    region.c2=wrapbmp->bmp.width-1;
    region.r1=0;
    region.r2=wrapbmp->bmp.height-1;
    region.bbox.rowbase=wrapbmp->base;
    region.bmp=&wrapbmp->bmp;
    region.bgcolor=wrapbmp->bgcolor;
    region.dpi=k2settings->src_dpi;
#if (WILLUSDEBUGX & 4)
k2printf("Bitmap is %d x %d (baseline=%d)\n",wrapbmp->bmp.width,wrapbmp->bmp.height,wrapbmp->base);
#endif

    if (wrapbmp->bmp.bpp==24)
        {
        bmp8=&_bmp8;
        bmp_init(bmp8);
        bmp_convert_to_greyscale_ex(bmp8,&wrapbmp->bmp);
        region.bmp8=bmp8;
        }
    else
        region.bmp8=&wrapbmp->bmp;
    /* Calculate lcheight and capheight */
    bmpregion_calc_bbox(&region,k2settings,1);
    region.bbox.rowbase=wrapbmp->base;
    region.bbox.rowheight=wrapbmp->textrow.rowheight;
    region.bbox.gap=wrapbmp->textrow.rowheight-(region.r2-region.r1+1);
    region.bbox.gapblank=wrapbmp->textrow.gapblank;
    region.bbox.type=REGION_TYPE_TEXTLINE;
    region.wrectmaps=&wrapbmp->wrectmaps;
    if (!allow_full_justification)
        just = (wrapbmp->just & 0xcf) | 0x20;
    else
        just = wrapbmp->just;


    /*
    ** Axet on GitLab thinks this code should be added:
    **
    ** Problem happens sometime, when here is only one element recognized on text
    ** line (happens for titles and right aligned epigraphs) then 'wrmap' not
    ** aligned properly. As result text detected correctly, and page formed normally,
    ** but when I ask for back coordinates (original coordinates on source image)
    ** I got wrong results. It happens because 'wrmap' malformed during parsing.
    */
#if (defined(__ANDROID__) && defined(K2PDFOPT_KINDLEPDFVIEWER))
    {
    int dstmar_pixels[4];
    int i,w;

    get_dest_margins(dstmar_pixels,k2settings,(double)k2settings->dst_dpi,
                     masterinfo->bmp.width,k2settings->dst_height);
    w = masterinfo->bmp.width-dstmar_pixels[0]-dstmar_pixels[2];
    w = w*region.dpi/k2settings->dst_dpi;
    w = w-wrapbmp->bmp.width;
#if (WILLUSDEBUGX & 0x4000000)
{
static int count=0;
char filename[256];
sprintf(filename,"wrapbmp_region_%04d.png",++count);
printf("@wrapbmp_flush:  region %d. region.wrectmaps->n=%d\n",count,region.wrectmaps->n);
bmpregion_write(&region,filename);
}
#endif
    for(i=0;i<region.wrectmaps->n;i++)
        { /* adjust center/right for all recently added regions */
        WRECTMAP *m;

        m = &region.wrectmaps->wrectmap[i];
        if (m->coords[1].y >= wrapbmp->base+region.r1-region.bbox.rowbase)
{
#if (WILLUSDEBUGX & 0x4000000)
printf("@wrapbmp_flush:  %d. Adding %d to m->coords[1].x=%g\n",i,
            ((just&0xc)==4) ? w/2 : ((just&0xc)==8) ? w : 0,
             m->coords[1].x);
#endif
            m->coords[1].x += ((just&0xc)==4) ? w/2 : (((just&0xc)==8) ? w : 0);
}
        }
    }
#endif /* (defined(__ANDROID__) && defined(K2PDFOPT_KINDLEPDFVIEWER)) */
/* End Axet code mod */

    /*
    ** For now, set pageinfo=NULL in calls to bmpregion_add because the
    ** pageinfo processing assumes that the BMPREGION structure it is working
    ** with is using the original source bitmap, not the wrapbmp bitmap.
    ** This means that word wrapping can't use the pageinfo structure for now.
    */
    {
    ADDED_REGION_INFO added_region;
    int npr;
    double gap;

    npr=masterinfo->mandatory_region_gap;
    gap=masterinfo->page_region_gap_in;
    /*
    ** Temporarily set masterinfo->mandatory_region_gap to what it was when
    ** wrapbmp_add() was called.
    */
    masterinfo->mandatory_region_gap=wrapbmp->mandatory_region_gap;
    masterinfo->page_region_gap_in=wrapbmp->page_region_gap_in;
#if (WILLUSDEBUGX & 0x200)
aprintf(ANSI_RED "mi->mandatory_region_gap temp change to %d by wrap_flush." ANSI_NORMAL "\n",masterinfo->mandatory_region_gap);
#endif
#if (WILLUSDEBUGX & 1)
printf("wrapbmp_flush calling bmpregion_add() w/bbox.rowheight=%d\n",region.bbox.rowheight);
#endif
    added_region.region=&region;
    added_region.firstrow=0;
    added_region.lastrow=region.textrows.n-1;
    added_region.allow_text_wrapping=0;
    added_region.trim_flags=0;
    added_region.allow_vertical_breaks=0;
    added_region.force_scale=-1.0;
    added_region.justification_flags=just;
    added_region.caller_id=2;
    /* added_region.mark_flags=0xf; */
    added_region.rowbase_delta = wrapbmp->bmp.height-1-wrapbmp->base;
    added_region.region_is_centered = -1;
    added_region.notes=0;
    added_region.count=0;
    added_region.maps_to_source=0;
    bmpregion_add(&added_region,k2settings,masterinfo);
    /*
    ** Restore masterinfo->mandatory_region_gap
    */
    masterinfo->mandatory_region_gap=npr;
    masterinfo->page_region_gap_in=gap;
#if (WILLUSDEBUGX & 0x200)
aprintf(ANSI_RED "mi->mandatory_region_gap temp change back to %d by wrap_flush." ANSI_NORMAL "\n",masterinfo->mandatory_region_gap);
#endif
    }
    wrectmaps_clear(&wrapbmp->wrectmaps);
    bmpregion_free(&region);
    if (wrapbmp->bmp.bpp==24)
        bmp_free(bmp8);
    wrapbmp_reset(wrapbmp);
    }


static void wrapbmp_hyphen_erase(WRAPBMP *wrapbmp,K2PDFOPT_SETTINGS *k2settings)

    {
    WILLUSBITMAP *bmp,_bmp;
    int bw,bpp,c0,c1,c2,i;

    if (wrapbmp->hyphen.ch<0)
        return;
#if (WILLUSDEBUGX & 16)
k2printf("@hyphen_erase, bmp=%d x %d x %d\n",wrapbmp->bmp.width,wrapbmp->bmp.height,wrapbmp->bmp.bpp);
k2printf("    ch=%d, c2=%d, r1=%d, r2=%d\n",wrapbmp->hyphen.ch,wrapbmp->hyphen.c2,wrapbmp->hyphen.r1,wrapbmp->hyphen.r2);
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
    /*
    ** Adjust word rectangle mappings to source pages
    */
    for (i=0;i<wrapbmp->wrectmaps.n;i++)
        {
        if (k2settings->src_left_to_right==0)
            wrapbmp->wrectmaps.wrectmap[i].coords[1].x -= c0;
        if (i==wrapbmp->wrectmaps.n-1)
            {
            wrapbmp->wrectmaps.wrectmap[i].coords[2].x -= c0;
            if (k2settings->src_left_to_right==0)
                wrapbmp->wrectmaps.wrectmap[i].coords[0].x += c0;
            }
        }
    for (i=0;i<bmp->height;i++)
        memcpy(bmp_rowptr_from_top(bmp,i),bmp_rowptr_from_top(&wrapbmp->bmp,i)+bpp*c0,bw);
    bw=(c2-c1+1)*bpp;
    if (bw>0)
        for (i=wrapbmp->hyphen.r1;i<=wrapbmp->hyphen.r2;i++)
            memset(bmp_rowptr_from_top(bmp,i)+bpp*c1,255,bw);
#if (WILLUSDEBUGX & 16)
{
static int count=1;
char filename[MAXFILENAMELEN];
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


void wrectmaps_init(WRECTMAPS *wrectmaps)

    {
    wrectmaps->n=wrectmaps->na=0;
    wrectmaps->wrectmap=NULL;
    }


void wrectmaps_free(WRECTMAPS *wrectmaps)

    {
    static char *funcname="wrectmaps_free";

    willus_mem_free((double **)&wrectmaps->wrectmap,funcname);
    wrectmaps->na=wrectmaps->n=0;
    }


void wrectmaps_clear(WRECTMAPS *wrectmaps)

    {
    wrectmaps->n=0;
    }


void wrectmaps_add_wrectmap(WRECTMAPS *wrectmaps,WRECTMAP *wrectmap)

    {
    static char *funcname="wrectmaps_add_wrectmap";

    if (wrectmaps->n>=wrectmaps->na)
        {
        int newsize;
        newsize = wrectmaps->na < 128 ? 256 : wrectmaps->na*2;
        willus_mem_realloc_robust_warn((void **)&wrectmaps->wrectmap,newsize*sizeof(WRECTMAP),
                                    wrectmaps->na*sizeof(WRECTMAP),funcname,10);
        wrectmaps->na=newsize;
        }
    wrectmaps->wrectmap[wrectmaps->n++]=(*wrectmap);
    }


/*
** When wrapbmp bitmap is re-sampled by bmpregion_add(), this scales the mapped
** coordinates.
*/
void wrectmaps_scale_wrapbmp_coords(WRECTMAPS *wrectmaps,double scalew,double scaleh)

    {
    int i;

#if (WILLUSDEBUGX & 0x400)
printf("@wrectmaps_scale:  n=%d, scalew=%g, scaleh=%g\n",wrectmaps->n,scalew,scaleh);
#endif
    for (i=0;i<wrectmaps->n;i++)
        {
        int w,h;

        w=wrectmaps->wrectmap[i].srcwidth;
        wrectmaps->wrectmap[i].srcwidth = (int)(w*scalew+.5);
        scalew = (double)wrectmaps->wrectmap[i].srcwidth/w;
        h=wrectmaps->wrectmap[i].srcheight;
        wrectmaps->wrectmap[i].srcheight = (int)(h*scaleh+.5);
        scaleh = (double)wrectmaps->wrectmap[i].srcheight/h;
        wrectmaps->wrectmap[i].srcdpiw *= scalew;
        wrectmaps->wrectmap[i].srcdpih *= scaleh;
        wrectmaps->wrectmap[i].coords[0].x *= scalew;
        wrectmaps->wrectmap[i].coords[0].y *= scaleh;
        wrectmaps->wrectmap[i].coords[1].x *= scalew;
        wrectmaps->wrectmap[i].coords[1].y *= scaleh;
        wrectmaps->wrectmap[i].coords[2].x *= scalew;
        wrectmaps->wrectmap[i].coords[2].y *= scaleh;
        }
    }


int wrectmap_inside(WRECTMAP *wrmap,int xc,int yc)

    {
    return(wrmap->coords[1].x <= xc && wrmap->coords[1].x+wrmap->coords[2].x >= xc
            && wrmap->coords[1].y <= yc && wrmap->coords[1].y+wrmap->coords[2].y >= yc);
    }


void wrectmaps_sort_horizontally(WRECTMAPS *wrectmaps)

    {
    int top,n,n1;
    WRECTMAP  x0;
    WRECTMAP *x;

    x=wrectmaps->wrectmap;
    n=wrectmaps->n;
    if (n<2)
        return;
    top=n/2;
    n1=n-1;
    while (1)
        {
        if (top>0)
            {
            top--;
            x0=x[top];
            }
        else
            {
            x0=x[n1];
            x[n1]=x[0];
            n1--;
            if (!n1)
                {
                x[0]=x0;
                return;
                }
            }
        {
        int parent,child;

        parent=top;
        child=top*2+1;
        while (child<=n1)
            {
            if (child<n1 && wrectmap_hcompare(&x[child],&x[child+1])<0)
                child++;
            if (wrectmap_hcompare(&x0,&x[child])<0)
                {
                x[parent]=x[child];
                parent=child;
                child+=(parent+1);
                }
            else
                break;
            }
        x[parent]=x0;
        }
        }
    }


static double wrectmap_hcompare(WRECTMAP *x1,WRECTMAP *x2)

    {
    return(x1->coords[1].x - x2->coords[1].x);
    }
