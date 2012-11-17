/*
** bmpdjvu.c    Routines to interface w/djvu lib.
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

#ifdef HAVE_DJVU_LIB
#include <djvu.h>

static void handle(int wait,ddjvu_context_t *ctx);


/*
** Returns 0 for success, negative number for error code.
** bpp can be 24 or 8.
**
*/
int bmpdjvu_djvufile_to_bmp(WILLUSBITMAP *bmp,char *infile,int pageno,
                            int dpi,int bpp,FILE *out)

    {
    ddjvu_context_t *ctx;
    ddjvu_document_t *doc;
    ddjvu_page_t *page;
    /* ddjvu_page_type_t type; */
    ddjvu_rect_t prect;
    ddjvu_rect_t rrect;
    ddjvu_format_style_t style;
    ddjvu_render_mode_t mode;
    ddjvu_format_t *fmt;
    int i,iw,ih,idpi,status;

    ctx=ddjvu_context_create("bmpdjvu_djvufile_to_bmp");
    if (ctx==NULL)
        {
        nprintf(out,"Cannot create djvu context.\n");
        return(-1);
        }
    doc=ddjvu_document_create_by_filename(ctx,infile,1);
    if (doc==NULL)
        {
        ddjvu_context_release(ctx);
        nprintf(out,"Cannot create djvu document context from djvu file %s.\n",
                infile);
        return(-2);
        }
    i=ddjvu_document_get_pagenum(doc);
    if (pageno<0 || pageno>i)
        {
        ddjvu_document_release(doc);
        ddjvu_context_release(ctx);
        nprintf(out,"Page number %d is out of range for djvu file %s.\n",pageno,infile);
        return(-3);
        }
    page=ddjvu_page_create_by_pageno(doc,pageno-1);
    if (page==NULL)
        {
        ddjvu_document_release(doc);
        ddjvu_context_release(ctx);
        nprintf(out,"Cannot parse page %d of djvu file %s.\n",pageno,infile);
        return(-4);
        }
    while (!ddjvu_page_decoding_done(page))
        handle(1,ctx);
    if (ddjvu_page_decoding_error(page))
        {
        ddjvu_page_release(page);
        ddjvu_document_release(doc);
        ddjvu_context_release(ctx);
        nprintf(out,"Error decoding page %d of djvu file %s.\n",pageno,infile);
        return(-5);
        }
    /* type= */ ddjvu_page_get_type(page);
    /*
    printf("type=%d\n",type);
    description=ddjvu_page_get_long_description(page);
    printf("Description='%s'\n",description);
    */
    iw = ddjvu_page_get_width(page);
    ih = ddjvu_page_get_height(page);
    idpi = ddjvu_page_get_resolution(page);
    prect.x=prect.y=0;
    bmp->width=prect.w=iw*dpi/idpi;
    bmp->height=prect.h=ih*dpi/idpi;
    bmp->bpp=(bpp==8) ? 8 : 24;
    rrect=prect;
    bmp_alloc(bmp);
    if (bmp->bpp==8)
        {
        int ii;
        for (ii=0;ii<256;ii++)
            bmp->red[ii]=bmp->blue[ii]=bmp->green[ii]=ii;
        }
    mode=DDJVU_RENDER_COLOR;
    style=bpp==8 ? DDJVU_FORMAT_GREY8 : DDJVU_FORMAT_RGB24;
    fmt=ddjvu_format_create(style,0,0);
    if (fmt==NULL)
        {
        ddjvu_page_release(page);
        ddjvu_document_release(doc);
        ddjvu_context_release(ctx);
        nprintf(out,"Error setting DJVU format for djvu file %s (page %d).\n",infile,pageno);
        return(-6);
        }
    ddjvu_format_set_row_order(fmt,1);
    status=ddjvu_page_render(page,mode,&prect,&rrect,fmt,bmp_bytewidth(bmp),(char *)bmp->data);
    ddjvu_format_release(fmt);
    ddjvu_page_release(page);
    ddjvu_document_release(doc);
    ddjvu_context_release(ctx);
    if (!status)
        {
        nprintf(out,"Error rendering page %d of djvu file %s.\n",pageno,infile);
        return(-7);
        }
    return(0);
    }


/*
** Returns >0  for success, negative number for error code.
*/
int bmpdjvu_numpages(char *infile)

    {
    ddjvu_context_t *ctx;
    ddjvu_document_t *doc;
    int i;

    ctx=ddjvu_context_create("bmpdjvu_numpages");
    if (ctx==NULL)
        return(-1);
    doc=ddjvu_document_create_by_filename(ctx,infile,1);
    if (doc==NULL)
        {
        ddjvu_context_release(ctx);
        return(-2);
        }
    i=ddjvu_document_get_pagenum(doc);
    ddjvu_document_release(doc);
    ddjvu_context_release(ctx);
    return(i);
    }


static void handle(int wait,ddjvu_context_t *ctx)
    {
    const ddjvu_message_t *msg;

    if (!ctx)
        return;
    if (wait)
        msg = ddjvu_message_wait(ctx);
    while ((msg = ddjvu_message_peek(ctx)))
        {
        switch(msg->m_any.tag)
            {
            case DDJVU_ERROR:
                fprintf(stderr,"ddjvu: %s\n", msg->m_error.message);
                if (msg->m_error.filename)
                    fprintf(stderr,"ddjvu: '%s:%d'\n", 
                      msg->m_error.filename, msg->m_error.lineno);
            exit(10);
            default:
            break;
            }
        }
    ddjvu_message_pop(ctx);
    }
#endif /* HAVE_DJVU_LIB */
