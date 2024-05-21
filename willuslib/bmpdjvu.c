/*
** bmpdjvu.c    Routines to interface w/djvu lib.
**
** Part of willus.com general purpose C code library.
**
** Copyright (C) 2020  http://willus.com
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
static void djvu_add_page_info(char *buf,ddjvu_document_t *doc,int page,int npages);
static int wpdfoutline_fill_from_miniexp(WPDFOUTLINE *outline,miniexp_t bmarks);
static int wpdfoutline_next_from_miniexp(WPDFOUTLINE *outline,miniexp_t bmark);
static void wtextchars_from_miniexp(WTEXTCHARS *wtcs,miniexp_t dtext,int dpi,
                                    double pageheight_pts,int boundingbox);
static void wtextchars_add_single_miniexp(WTEXTCHARS *wtcs,miniexp_t r,int dpi,
                                          double pageheight_pts,int boundingbox);
static void wtextchars_add_one_djvu_char(WTEXTCHARS *wtcs,double *pos,int ucs,int index,
                                         int n,int boundingbox);
static int miniint(miniexp_t p);
static int minitype(miniexp_t p);
extern int miniexp_length(miniexp_t p);
extern miniexp_t miniexp_nth(int index,miniexp_t p);
extern char *miniexp_to_name(miniexp_t p);
extern char *miniexp_to_str(miniexp_t p);
extern int miniexp_stringp(miniexp_t p);


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
    doc=ddjvu_document_create_by_filename_utf8(ctx,infile,1);
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
    /* Seems to return 0 for blank/empty page */
    if (!status) 
        bmp_fill(bmp,255,255,255);
    ddjvu_format_release(fmt);
    ddjvu_page_release(page);
    ddjvu_document_release(doc);
    ddjvu_context_release(ctx);
    /*
    if (!status)
        {
        nprintf(out,"Error rendering page %d of djvu file %s.\n",pageno,infile);
        return(-7);
        }
    */
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
    doc=ddjvu_document_create_by_filename_utf8(ctx,infile,1);
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


WPDFOUTLINE *wpdfoutline_read_from_djvu_file(char *filename)

    {
    static char *funcname="wpdfoutline_read_from_djvu_file";
    ddjvu_context_t *ctx;
    ddjvu_document_t *doc;
    miniexp_t bmarks;
    WPDFOUTLINE *outline;

    ctx=ddjvu_context_create("wtextchars_fill_from_djvu_page");
    if (ctx==NULL)
        return(-1);
    doc=ddjvu_document_create_by_filename_utf8(ctx,filename,1);
    if (doc==NULL)
        {
        ddjvu_context_release(ctx);
        return(-2);
        }
    /*
    npages=ddjvu_document_get_pagenum(doc);
    if (pageno<1 || pageno>npages)
        {
        ddjvu_document_release(doc);
        ddjvu_context_release(ctx);
        return(-3);
        }
    */
    bmarks=ddjvu_document_get_outline(doc);
    outline=NULL;
    if (bmarks!=NULL)
        {
        WPDFOUTLINE oline;
        wpdfoutline_init(&oline);
        if (wpdfoutline_fill_from_miniexp(&oline,bmarks)>0)
            {
            willus_mem_alloc_warn((void **)&outline,sizeof(WPDFOUTLINE),funcname,10);
            (*outline)=oline;
            }
        }
    ddjvu_document_release(doc);
    ddjvu_context_release(ctx);
    return(outline);
    }


static int wpdfoutline_fill_from_miniexp(WPDFOUTLINE *outline,miniexp_t bmarks)

    {
    int i,c,n;
    static char *funcname="wpdfoutline_fill_from_miniexp";

    if (minitype(bmarks)!=0 || (n=miniexp_length(bmarks))<=0)
        return(-1);
    for (i=c=0;i<n;i++)
        {
        WPDFOUTLINE oline;

        wpdfoutline_init(&oline);
        if (wpdfoutline_next_from_miniexp(&oline,miniexp_nth(i,bmarks))>0)
            {
            if (c==0)
                (*outline)=oline;
            else
                {
                willus_mem_alloc_warn((void **)&outline->next,sizeof(WPDFOUTLINE),funcname,10);
                outline=outline->next;
                (*outline)=oline;
                }
            c++;
            }
        }
    return(c);
    }


static int wpdfoutline_next_from_miniexp(WPDFOUTLINE *outline,miniexp_t bmark)

    {
    int n,slen;
    miniexp_t p,p2;
    static char *funcname="wpdfoutline_next_from_miniexp";

    if (minitype(bmark)!=0 || (n=miniexp_length(bmark))<2)
        return(-1);
    p=miniexp_nth(0,bmark);
    p2=miniexp_nth(1,bmark);
    if (!miniexp_stringp(p) || !miniexp_stringp(p2))
        return(-2);
    slen=strlen(miniexp_to_str(p));
    willus_mem_alloc_warn((void **)&outline->title,slen+1,funcname,10);
    strcpy(outline->title,miniexp_to_str(p));
    clean_line_utf8(outline->title);
    outline->srcpage=outline->dstpage=atoi(miniexp_to_str(p2)+1)-1;
    if (n>2)
        {
        p=miniexp_nth(2,bmark);
        if (minitype(p)==0)
            {
            WPDFOUTLINE oline;

            wpdfoutline_init(&oline);
            if (wpdfoutline_fill_from_miniexp(&oline,p)>0)
                {
                willus_mem_alloc_warn((void **)&outline->down,sizeof(WPDFOUTLINE),funcname,10);
                (*outline->down)=oline;
                }
            }
        }
    return(1);
    }

/*
** DJVU file info
**
** pageno starts at 1
**
** if boundingbox==1, only one character is returned, and its upper-left and lower-right
** corner are the bounding box of all text on the page.
**
*/
void bmpdjvu_info_get(char *filename,int *pagelist,char **buf0)

    {
    char *buf;
    int i,npages;
    ddjvu_context_t *ctx;
    ddjvu_document_t *doc;
    ddjvu_page_t *page;
    miniexp_t dtext;

    (*buf0)=NULL;
    ctx=ddjvu_context_create("bmpdjvu_info_get");
    if (ctx==NULL)
        return;
    doc=ddjvu_document_create_by_filename_utf8(ctx,filename,1);
    if (doc==NULL)
        {
        ddjvu_context_release(ctx);
        return;
        }
    npages=ddjvu_document_get_pagenum(doc);
    buf=malloc(1024+100*npages);
    sprintf(buf,"DJVU File: %s\r\n\r\n%d total pages\r\n\r\n",filename,npages);
    if (pagelist!=NULL)
        {
        for (i=0;pagelist[i]>0;i++)
            djvu_add_page_info(buf,doc,pagelist[i]-1,npages);
        }
    if (pagelist==NULL || pagelist[i]==-2)
        for (i=(pagelist!=NULL && i>0?pagelist[i-1]+1:1);i<=npages;i++)
            djvu_add_page_info(buf,doc,i-1,npages);
    ddjvu_document_release(doc);
    ddjvu_context_release(ctx);
    (*buf0)=buf;
    }


static void djvu_add_page_info(char *buf,ddjvu_document_t *doc,int pageno,int npages)

    {
    ddjvu_page_t *page;
    int dpi;
    double width_in,height_in;

    sprintf(&buf[strlen(buf)],"Page %3d of %3d: ",pageno+1,npages);
    page=ddjvu_page_create_by_pageno(doc,pageno);
    if (page==NULL)
        {
        sprintf(&buf[strlen(buf)],"No info available.\r\n");
        return;
        }
    dpi=ddjvu_page_get_resolution(page);
    height_in = ddjvu_page_get_height(page)/dpi;
    width_in = ddjvu_page_get_width(page)/dpi;
    sprintf(&buf[strlen(buf)],"%.2f x %.2f in, %d dpi\r\n",width_in,height_in,dpi);
    ddjvu_page_release(page);
    }


/*
** CHARACTER POSITION MAPS
**
** pageno starts at 1
**
** if boundingbox==1, only one character is returned, and its upper-left and lower-right
** corner are the bounding box of all text on the page.
**
*/
int wtextchars_fill_from_djvu_page(WTEXTCHARS *wtcs,char *filename,int pageno,int boundingbox)

    {
    int dpi,npages;
    double height_pts;
    ddjvu_context_t *ctx;
    ddjvu_document_t *doc;
    ddjvu_page_t *page;
    miniexp_t dtext;
 
    wtcs->n=0; 
    ctx=ddjvu_context_create("wtextchars_fill_from_djvu_page");
    if (ctx==NULL)
        return(-1);
    doc=ddjvu_document_create_by_filename_utf8(ctx,filename,1);
    if (doc==NULL)
        {
        ddjvu_context_release(ctx);
        return(-2);
        }
    npages=ddjvu_document_get_pagenum(doc);
    if (pageno<1 || pageno>npages)
        {
        ddjvu_document_release(doc);
        ddjvu_context_release(ctx);
        return(-3);
        }
    page=ddjvu_page_create_by_pageno(doc,pageno-1);
    if (page==NULL)
        {
        ddjvu_document_release(doc);
        ddjvu_context_release(ctx);
        return(-4);
        }
    dpi=ddjvu_page_get_resolution(page);
    height_pts = 72.*ddjvu_page_get_height(page)/dpi;
    dtext=ddjvu_document_get_pagetext(doc,pageno-1,NULL);
    if (dtext!=NULL)
        wtextchars_from_miniexp(wtcs,dtext,dpi,height_pts,boundingbox);
    ddjvu_page_release(page);
    ddjvu_document_release(doc);
    ddjvu_context_release(ctx);
    return(0);
    }


static void wtextchars_from_miniexp(WTEXTCHARS *wtcs,miniexp_t dtext,int dpi,
                                    double pageheight_pts,int boundingbox)

    {
    int i,n;

    if (minitype(dtext)!=0)
        return;
    n=miniexp_length(dtext);
    for (i=0;i<n;i++)
        {
        miniexp_t r;

        r=miniexp_nth(i,dtext);
        if (r!=NULL)
            wtextchars_add_single_miniexp(wtcs,r,dpi,pageheight_pts,boundingbox);
        }
    }


static void wtextchars_add_single_miniexp(WTEXTCHARS *wtcs,miniexp_t r,int dpi,
                                          double pageheight_pts,int boundingbox)

    {
    int i,n;
    double pos[4];
    miniexp_t p;
    char buf[32];
    int *ucs;
    static char *funcname="wtextchars_add_single_miniexp";

    if (minitype(r)!=0)
        return;
    n=miniexp_length(r);
    if (n!=6)
        return;
    p=miniexp_nth(0,r);
    if (minitype(p)!=2)
        return;
    xstrncpy(buf,miniexp_to_name(p),31);
    if (stricmp(buf,"char") && stricmp(buf,"word"))
        return;
    for (i=0;i<4;i++)
        {
        int j;
        p=miniexp_nth(i+1,r);
        if (minitype(p)!=3)
            return;
        j= (i==1) ? 3 : (i==3 ? 1 : i);
        pos[j]=72.*miniint(p)/dpi;
        if (j==1 || j==3)
            pos[j]=pageheight_pts-pos[j];
        }
    p=miniexp_nth(5,r);
    if (!miniexp_stringp(p))
        return;
    ucs=NULL;
    n=utf8_to_unicode(NULL,miniexp_to_str(p),-1);
    willus_mem_alloc_warn((void **)&ucs,sizeof(int)*(n+1),funcname,10);
    n=utf8_to_unicode(ucs,miniexp_to_str(p),n);
    /* Eliminate lf's at the end of the word */
    for (;n>0 && ucs[n-1]=='\n';n--);
    for (i=0;i<n;i++)
        wtextchars_add_one_djvu_char(wtcs,pos,ucs[i],i,n,boundingbox);
    willus_mem_free((double **)&ucs,funcname);
    }


static void wtextchars_add_one_djvu_char(WTEXTCHARS *wtcs,double *pos,int ucs,int index,
                                         int n,int boundingbox)

    {
    WTEXTCHAR *wtc,_wtc;;

    wtc=&_wtc;
    wtc->x1=pos[0] + (pos[2]-pos[0])*index/n;
    wtc->x2=pos[0] + (pos[2]-pos[0])*(index+1.)/n;
    wtc->y1=pos[1];
    wtc->y2=pos[3];
    wtc->xp=wtc->x1;
    wtc->yp=wtc->y2;
    wtc->ucs=ucs;
    if (boundingbox && wtcs->n>0)
        {
        WTEXTCHAR *wtc1;
        wtc1=&wtcs->wtextchar[0];
        if (wtc->x1 < wtc1->x1)
            {
            wtc1->x1=wtc->x1;
            wtc1->xp=wtc->xp;
            }
        if (wtc->y1 < wtc1->y1)
            {
            wtc1->y1=wtc->y1;
            wtc1->yp=wtc->yp;
            }
        if (wtc->x2 > wtc1->x2)
            wtc1->x2=wtc->x2;
        if (wtc->y2 > wtc1->y2)
            wtc1->y2=wtc->y2;
        }
    else
        wtextchars_add_wtextchar(wtcs,wtc);
    }


static int miniint(miniexp_t p)

    {
    int size;

    size=(int)((size_t)p);
    return((size&3)==3 ? size>>2 : -1);
    }


/*
** 0 = list
** 1 = ?
** 2 = symbol
** 3 = integer
*/
static int minitype(miniexp_t p)

    {
    int size;

    size=(int)((size_t)p);
    return(size&3);
    }
#endif /* HAVE_DJVU_LIB */
