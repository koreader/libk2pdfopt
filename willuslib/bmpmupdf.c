/*
** bmppdf.c    Routines to interface w/mupdf lib.
**
** Part of willus.com general purpose C code library.
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
#include <stdio.h>
#include "willus.h"

#ifdef HAVE_MUPDF_LIB
#include <mupdf.h>

static int bmpmupdf_pixmap_to_bmp(WILLUSBITMAP *bmp,fz_context *ctx,fz_pixmap *pixmap);

int bmpmupdf_pdffile_to_bmp(WILLUSBITMAP *bmp,char *filename,int pageno,double dpi,
                            int bpp)

    {
    fz_context *ctx;
    fz_colorspace *colorspace;
    fz_document *doc;
    fz_page *page;
    fz_display_list *list;
    fz_device *dev;
    fz_pixmap *pix;
    double dpp;
    fz_rect bounds,bounds2;
    fz_matrix ctm;
    fz_bbox bbox;
//    fz_glyph_cache *glyphcache;
//    fz_error error;
    int np,status;

    dev=NULL;
    list=NULL;
    page=NULL;
    doc=NULL;
    status=0;
    if (pageno<1)
        return(-99);
    ctx = fz_new_context(NULL,NULL,FZ_STORE_DEFAULT);
    if (!ctx)
        return(-1);
    fz_set_aa_level(ctx,8);
//    fz_accelerate();
//    glyphcache=fz_new_glyph_cache();
    colorspace=(bpp==8 ? fz_device_gray : fz_device_rgb);
    fz_try(ctx) { doc=fz_open_document(ctx,filename); }
    fz_catch(ctx) 
        { 
        fz_free_context(ctx);
        return(-1);
        }
    /*
    if (fz_needs_password(doc) && !fz_authenticate_password(doc,password))
        return(-2);
    */
//    error=pdf_load_page_tree(xref);
//    if (error)
//        {
//        pdf_free_xref(xref);
//        return(-2);
//        }

    np=fz_count_pages(doc);
    if (pageno>np)
        return(-99);
    fz_try(ctx) { page = fz_load_page(doc,pageno-1); }
    fz_catch(ctx) 
        {
        fz_close_document(doc);
        fz_free_context(ctx);
        return(-3);
        }
    fz_try(ctx) { list=fz_new_display_list(ctx);
                  dev=fz_new_list_device(ctx,list);
                  fz_run_page(doc,page,dev,fz_identity,NULL);
                }
    fz_catch(ctx)
        {
        fz_free_device(dev);
        fz_free_display_list(ctx,list);
        fz_free_page(doc,page);
        fz_close_document(doc);
        fz_free_context(ctx);
        return(-4);
        }
    fz_free_device(dev);
    dev=NULL;
    dpp=dpi/72.;
    pix=NULL;
    fz_var(pix);
    bounds=fz_bound_page(doc,page);
    ctm=fz_scale(dpp,dpp);
//    ctm=fz_concat(ctm,fz_rotate(rotation)); 
    bounds2=fz_transform_rect(ctm,bounds);
    bbox=fz_round_rect(bounds2);
//    ctm=fz_translate(0,-page->mediabox.y1);
//    ctm=fz_concat(ctm,fz_scale(dpp,-dpp));
//    ctm=fz_concat(ctm,fz_rotate(page->rotate));
//    ctm=fz_concat(ctm,fz_rotate(0));
//    bbox=fz_round_rect(fz_transform_rect(ctm,page->mediabox));
//    pix=fz_new_pixmap_with_rect(colorspace,bbox);
    fz_try(ctx)
        {
        pix=fz_new_pixmap_with_bbox(ctx,colorspace,bbox);
        fz_clear_pixmap_with_value(ctx,pix,255);
        dev=fz_new_draw_device(ctx,pix);
        if (list)
            fz_run_display_list(list,dev,ctm,bbox,NULL);
        else
            fz_run_page(doc,page,dev,ctm,NULL);
        fz_free_device(dev);
        dev=NULL;
        status=bmpmupdf_pixmap_to_bmp(bmp,ctx,pix);
        fz_drop_pixmap(ctx,pix);
        }
    fz_catch(ctx)
        {
        fz_free_device(dev);
        fz_drop_pixmap(ctx,pix);
        fz_free_display_list(ctx,list);
        fz_free_page(doc,page);
        fz_close_document(doc);
        fz_free_context(ctx);
        return(-5);
        }
    if (list)
        fz_free_display_list(ctx,list);
    fz_free_page(doc,page);
//    pdf_free_xref(xref);
    fz_close_document(doc);
    fz_flush_warnings(ctx);
    fz_free_context(ctx);
//    fz_free_glyph_cache(glyphcache);
//    fz_flush_warnings();
    if (status<0)
        return(status-10);
    return(0);
    }


static int bmpmupdf_pixmap_to_bmp(WILLUSBITMAP *bmp,fz_context *ctx,fz_pixmap *pixmap)

    {
	unsigned char *p;
	int ncomp,i,row,col;

    bmp->width=fz_pixmap_width(ctx,pixmap);
    bmp->height=fz_pixmap_height(ctx,pixmap);
    ncomp=fz_pixmap_components(ctx,pixmap);
    /* Has to be 8-bit or RGB */
	if (ncomp != 2 && ncomp != 4)
		return(-1);
    bmp->bpp=(ncomp==2) ? 8 : 24;
    bmp_alloc(bmp);
    if (ncomp==2)
        for (i=0;i<256;i++)
            bmp->red[i]=bmp->green[i]=bmp->blue[i]=i;
	p = fz_pixmap_samples(ctx,pixmap);
    if (ncomp==1)
        for (row=0;row<bmp->height;row++)
            {
            unsigned char *dest;
            dest=bmp_rowptr_from_top(bmp,row);
            memcpy(dest,p,bmp->width);
            p+=bmp->width;
            }
    else if (ncomp==2)
        for (row=0;row<bmp->height;row++)
            {
            unsigned char *dest;
            dest=bmp_rowptr_from_top(bmp,row);
            for (col=0;col<bmp->width;col++,dest++,p+=2)
                dest[0]=p[0];
            }
    else
        for (row=0;row<bmp->height;row++)
            {
            unsigned char *dest;
            dest=bmp_rowptr_from_top(bmp,row);
            for (col=0;col<bmp->width;col++,dest+=ncomp-1,p+=ncomp)
                memcpy(dest,p,ncomp-1);
            }
	return(0);
    }
#endif /* HAVE_MUPDF_LIB */
