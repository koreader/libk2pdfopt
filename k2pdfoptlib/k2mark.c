/*
** k2mark.c    Functions to mark the regions on the source page.
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

int k2mark_page_count=0;

/*
** src guaranteed to be 24-bit color
*/
void publish_marked_page(PDFFILE *mpdf,WILLUSBITMAP *src,int src_dpi,
                         char *srcname,char *fmtname,int filecount,int pagecount,
                         int jpeg_quality)

    {
    int newdpi;
    WILLUSBITMAP *bmp,_bmp;

#if (WILLUSDEBUGX & 9)
static int count=1;
char filename[MAXFILENAMELEN];
sprintf(filename,"outsrc%02d.png",count++);
bmp_write(src,filename,stdout,100);
#endif
    bmp=&_bmp;
    bmp_init(bmp);
    newdpi = src_dpi/2;
    bmp->width=src->width/2;
    bmp->height=src->height/2;
    bmp->bpp=24;
    bmp_alloc(bmp);
    bmp_resample_optimum_performance(bmp,src,(double)0.,(double)0.,
                 (double)src->width,(double)src->height,
                 bmp->width,bmp->height);
    if (filename_is_bitmap(fmtname))
        {
        char filename[MAXFILENAMELEN];

        filename_get_marked_pdf_name(filename,fmtname,srcname,filecount,pagecount);
        bmp_set_dpi(src_dpi);
        if (!stricmp(wfile_ext(filename),"jpg") 
             || !stricmp(wfile_ext(filename),"jpeg"))
            bmp_promote_to_24(bmp);
        bmp_write(bmp,filename,NULL,jpeg_quality<1?93:jpeg_quality);
        bitmap_file_echo_status(filename);
        }
    else
        pdffile_add_bitmap(mpdf,bmp,newdpi,-1,1);
    bmp_free(bmp);
    }


/*
** Mark the region
** mark_flags & 1 :  Mark top
** mark_flags & 2 :  Mark bottom
** mark_flags & 4 :  Mark left
** mark_flags & 8 :  Mark right
**
*/
void mark_source_page(K2PDFOPT_SETTINGS *k2settings,MASTERINFO *masterinfo,
                      BMPREGION *region0,int caller_id,int mark_flags)

    {
    static int display_order=0;
    int i,n,nn,r,g,b;
#ifndef K2PDFOPT_KINDLEPDFVIEWER
    int shownum,nval,fontsize;
    char num[16];
#endif
    BMPREGION *region,_region;

    if (!k2settings->show_marked_source)
        return;

    /* Don't waste time marking this page if we're previewing and this isn't the preview page */
    if (k2settings->preview_page!=0 && abs(k2settings->preview_page)!=k2mark_page_count)
        return;

    if (region0==NULL)
        {
        display_order=0;
        return;
        }

    region=&_region;
    /* v2.15--use new conventions for bmpregion--init and free */
    bmpregion_init(region);
    bmpregion_copy(region,region0,0);

    /* Clip the region w/ignored margins */
    {
    BMPREGION *clip,_clip;
    clip=&_clip;
    bmpregion_init(clip);
    clip->bmp=region0->bmp;
    clip->bmp8=region0->bmp8;
    clip->dpi=region0->dpi;
    bmpregion_trim_to_crop_margins(clip,masterinfo,k2settings);
    if (region->c1 < clip->c1)
        region->c1 = clip->c1;
    if (region->c2 > clip->c2)
        region->c2 = clip->c2;
    if (region->r1 < clip->r1)
        region->r1 = clip->r1;
    if (region->r2 > clip->r2)
        region->r2 = clip->r2;
    bmpregion_free(clip);
    }
    if (region->r2 <= region->r1 || region->c2 <= region->c1)
        {
        bmpregion_free(region);
        return;
        }

    /* k2printf("@mark_source_page(display_order=%d)\n",display_order); */
#ifndef K2PDFOPT_KINDLEPDFVIEWER
    shownum=0;
    nval=0;
#endif
/* aprintf(ANSI_YELLOW "MARK CALLER ID = %d" ANSI_NORMAL "\n",caller_id); */
    if (caller_id==1)
        {
        display_order++;
#ifndef K2PDFOPT_KINDLEPDFVIEWER
        shownum=1;
        nval=display_order;
#endif
        n=(int)(region->dpi/60.+0.5);
        if (n<5)
            n=5;
        r=255;
        g=b=0;
        }
    else if (caller_id==2)
        {
        n=2;
        r=0;
        g=0;
        b=255;
        }
    else if (caller_id==3)
        {
        n=(int)(region->dpi/80.+0.5);
        if (n<4)
            n=4;
        if (caller_id==3)
            {
            r=0;
            g=255;
            b=0;
            }
        }
    else if (caller_id==4) /* Notes */
        {
        n=4;
        r=255;
        g=0;
        b=255;
        }
    else if (caller_id==5) /* rows of text */
        {
        n=2;
        r=140;
        g=140;
        b=140;
        }
    else if (caller_id>=100 && caller_id<=199) /* Double rows */
        {
#ifndef K2PDFOPT_KINDLEPDFVIEWER
        shownum=1;
        nval=caller_id-100;
#endif
        n=3;
        r=255;
        g=90;
        b=40;
        }
    else /* Not sure this is used anymore -- v2.20 */
        {
        n=2;
        r=110;
        g=110;
        b=110;
        }
    if (n<2)
        n=2;
    nn=(region->c2+1-region->c1)/2;
    if (n>nn)
        n=nn;
    nn=(region->r2+1-region->r1)/2;
    if (n>nn)
        n=nn;
    if (n<1)
        n=1;
    for (i=0;i<n;i++)
        {
        int j;
        unsigned char *p;
        if (mark_flags & 1)
            {
            p=bmp_rowptr_from_top(region->marked,region->r1+i)+region->c1*3;
            for (j=region->c1;j<=region->c2;j++,p+=3)
                {
                p[0]=r;
                p[1]=g;
                p[2]=b;
                }
            }
        if (mark_flags & 2)
            {
            p=bmp_rowptr_from_top(region->marked,region->r2-i)+region->c1*3;
            for (j=region->c1;j<=region->c2;j++,p+=3)
                {
                p[0]=r;
                p[1]=g;
                p[2]=b;
                }
            }
        /* rowbase */
        if ((mark_flags & 16) && region->bbox.type!=REGION_TYPE_FIGURE 
                              && region->bbox.rowbase>=region->r1
                              && region->bbox.rowbase<=region->r2)
            {
            p=bmp_rowptr_from_top(region->marked,region->bbox.rowbase-i)+region->c1*3;
            for (j=region->c1;j<=region->c2;j++,p+=3)
                {
                p[0]=r;
                p[1]=g;
                p[2]=b;
                }
            }
        if (mark_flags & 4)
            for (j=region->r1;j<=region->r2;j++)
                {
                p=bmp_rowptr_from_top(region->marked,j)+(region->c1+i)*3;
                p[0]=r;
                p[1]=g;
                p[2]=b;
                }
        if (mark_flags & 8)
            for (j=region->r1;j<=region->r2;j++)
                {
                p=bmp_rowptr_from_top(region->marked,j)+(region->c2-i)*3;
                p[0]=r;
                p[1]=g;
                p[2]=b;
                }
        }
/*
** Compiling out the fontrender...() functions reduces the exe size
** considerably because it eliminates the font data from being compiled in.
*/
#ifndef K2PDFOPT_KINDLEPDFVIEWER
    if (!shownum)
        {
        bmpregion_free(region);
        return;
        }
    fontsize=region->c2-region->c1+1;
    if (fontsize > region->r2-region->r1+1)
        fontsize=region->r2-region->r1+1;
    fontsize /= 2;
    if (fontsize > region->dpi)
        fontsize = region->dpi;
    if (fontsize < 5)
        {
        bmpregion_free(region);
        return;
        }
    fontrender_set_typeface("helvetica-bold");
    fontrender_set_fgcolor(r,g,b);
    fontrender_set_bgcolor(255,255,255);
    fontrender_set_pixel_size(fontsize);
    fontrender_set_justification(4);
    fontrender_set_or(1);
    sprintf(num,"%d",nval);
    fontrender_render(region->marked,(double)(region->c1+region->c2)/2.,
                      (double)(region->marked->height-((region->r1+region->r2)/2.)),num,0,NULL);    
#endif
    bmpregion_free(region);
    /* k2printf("    done mark_source_page.\n"); */
    }
