/*
** wpdf.c      PDF support routines to process WPDFBOX, WPDFPAGEINFO, and WTEXTCHAR
**             structures.  The functions in this file are divorced from MuPDF.
**             Any functions that work with these structures and also
**             depend on MuPDF are in wmupdf.c.
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
#include <math.h>
#include "willus.h"

static int  wpdfbox_compare(WPDFBOX *b1,WPDFBOX *b2);
static void wtextchar_scale(WTEXTCHAR *wtc,double scale_factor);
static void wtextchar_rotate_clockwise(WTEXTCHAR *wch,int rot,double page_width_pts,
                                       double page_height_pts);
static void point_sort(double *x1,double *x2);
static void point_rotate(double *x,double *y,int rot,double page_width_pts,double page_height_pts);
static int wtextchar_compare_vert_ex(WTEXTCHAR *c1,WTEXTCHAR *c2,int index);
static int  wtextchar_compare_vert(WTEXTCHAR *c1,WTEXTCHAR *c2,int index);
static int  wtextchar_compare_horiz(WTEXTCHAR *c1,WTEXTCHAR *c2);
/*
** Extracting text to STRBUF
*/
static void wtextchars_to_strbuf_one_line_1(WTEXTCHARS *wtcs,int i0,int i1,STRBUF *sbuf,
                                            double xmin,double spacesize,int nnl);
static void wtextchars_to_strbuf_one_line(WTEXTCHARS *wtcs,STRBUF *sbuf,double xmin,double spacesize);
static void strbuf_newline_with_spaces(STRBUF *sbuf,int nnl,int n);
static int  wtextchars_ligature_pattern(WTEXTCHARS *wtcs,int index);


void wpdfboxes_init(WPDFBOXES *boxes)

    {
    boxes->n=boxes->na=0;
    boxes->box=NULL;
    }


void wpdfboxes_free(WPDFBOXES *boxes)

    {
    static char *funcname="wpdfboxes_free";
    willus_mem_free((double **)&boxes->box,funcname);
    }


/*
** index should be <= boxes->n, otherwise box is just appended
*/
void wpdfboxes_insert_box(WPDFBOXES *boxes,WPDFBOX *box,int index)

    {
    wpdfboxes_add_box(boxes,box);
    if (index>=boxes->n-1)
        return;
    memmove(&boxes->box[index+1],&boxes->box[index],sizeof(WPDFBOX)*(boxes->n-1-index));
    boxes->box[index]=(*box);
    }


void wpdfboxes_add_box(WPDFBOXES *boxes,WPDFBOX *box)

    {
    static char *funcname="wpdfboxes_add_box";

    if (boxes->n>=boxes->na)
        {
        int newsize;

        newsize = boxes->na < 1024 ? 2048 : boxes->na*2;
        willus_mem_realloc_robust_warn((void **)&boxes->box,newsize*sizeof(WPDFBOX),
                                      boxes->na*sizeof(WPDFBOX),funcname,10);
        boxes->na=newsize;
        }
    boxes->box[boxes->n++]=(*box);
    }


void wpdfboxes_delete(WPDFBOXES *boxes,int n)

    {
    if (n>0 && n<boxes->n)
        {
        int i;
        for (i=0;i<boxes->n-n;i++)
            boxes->box[i]=boxes->box[i+n];
        }
    boxes->n -= n;
    if (boxes->n < 0)
        boxes->n = 0;
    }

/*
** Undo source page /Rotate
*/
void wpdfbox_unrotate(WPDFBOX *box,double deg)

    {
    double rot1;
    int i,nrot;

    /* Now do 90-degree rotations (full page) */
    rot1=fmod(-deg,360.);
    while (rot1<0.)
        rot1+=360.;
    nrot=(rot1+45.)/90.;
    for (i=0;i<nrot;i++)
        {
        double t;
        t=box->x0;
        box->x0=box->y0;
        box->y0=box->src_width_pts-t;
        t=box->h;
        box->h=box->w;
        box->w=t;
        t=box->src_height_pts;
        box->src_height_pts=box->src_width_pts;
        box->src_width_pts=t;
        }
    box->srcrot_deg -= nrot*90.;
    }


void wpdfbox_determine_original_source_position(WPDFBOX *box)

    {
    double rot1,sw,sh;
    int i,nrot;
    WPDFSRCBOX *srcbox;

    srcbox=&box->srcbox;
    /* First undo fine rotation about center */
    if (fabs(srcbox->finerot_deg)<1e-5)
        {
        box->srcrot_deg=0.;
        box->x0=srcbox->x0_pts;
        box->y0=srcbox->y0_pts;
        }
    else
        {
        double xc,yc,dx,dy,th,costh,sinth;

        box->srcrot_deg=-srcbox->finerot_deg;
        xc=srcbox->page_width_pts/2.;
        yc=srcbox->page_height_pts/2.;
        dx=srcbox->x0_pts-xc;
        dy=srcbox->y0_pts-yc;
        th=box->srcrot_deg*PI/180.;
        costh=cos(th);
        sinth=sin(th);
        box->x0=xc + dx*costh - dy*sinth;
        box->y0=yc + dy*costh + dx*sinth;
        }

    /* Now do 90-degree rotations (full page) */
    rot1=fmod(-srcbox->rot_deg,360.);
    while (rot1<0.)
        rot1+=360.;
    nrot=(rot1+45.)/90.;
    sw=srcbox->page_width_pts;
    sh=srcbox->page_height_pts;
    box->w=srcbox->crop_width_pts;
    box->h=srcbox->crop_height_pts;
    for (i=0;i<nrot;i++)
        {
        double t;
        t=box->y0;
        box->y0=box->x0;
        box->x0=sh-t;
        t=box->h;
        box->h=box->w;
        box->w=t;
        t=sh;
        sh=sw;
        sw=t;
        box->srcrot_deg += 90.;
        }
    box->src_width_pts=sw;
    box->src_height_pts=sh;
    }


void wpdfpageinfo_sort(WPDFPAGEINFO *pageinfo)

    {
    WPDFBOX *x;
    int n,top,n1;
    WPDFBOX x0;

    x=pageinfo->boxes.box;
    n=pageinfo->boxes.n;
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
            if (child<n1 && wpdfbox_compare(&x[child],&x[child+1])<0)
                child++;
            if (wpdfbox_compare(&x0,&x[child])<0)
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


static int wpdfbox_compare(WPDFBOX *b1,WPDFBOX *b2)

    {
    int x;
    double d;

    x=(b1->dstpage-b2->dstpage);
    if (x)
        return(x);
    x=(b1->srcbox.pageno-b2->srcbox.pageno);
    if (x)
        return(x);
    d=(b2->y1-b1->y1);
    if (d)
        return(d<0. ? -1 : 1);
    d=(b1->x1-b2->x1);
    if (d)
        return(d<0. ? -1 : 1);
    return(0);
    }


void wpdfpageinfo_scale_source_boxes(WPDFPAGEINFO *pageinfo,double doc_scale_factor)

    {
    int i;

    for (i=0;i<pageinfo->boxes.n;i++)
        {
        WPDFBOX *box;

        box=&pageinfo->boxes.box[i];
        box->scale /= doc_scale_factor;
        box->srcbox.page_width_pts *= doc_scale_factor;
        box->srcbox.page_height_pts *= doc_scale_factor;
        box->srcbox.x0_pts *= doc_scale_factor;
        box->srcbox.y0_pts *= doc_scale_factor;
        box->srcbox.crop_width_pts *= doc_scale_factor;
        box->srcbox.crop_height_pts *= doc_scale_factor;
        }
    }


void wtextchars_init(WTEXTCHARS *wtc)

    {
    wtc->n=wtc->na=0;
    wtc->wtextchar=NULL;
    wtc->sorted=0;
    }


void wtextchars_free(WTEXTCHARS *wtc)

    {
    static char *funcname="wtextchars_free";

    willus_mem_free((double **)&wtc->wtextchar,funcname);
    wtc->n=wtc->na=0;
    wtc->sorted=0;
    }


void wtextchars_clear(WTEXTCHARS *wtc)

    {
    wtc->n=0;
    wtc->sorted=0;
    }


void wtextchars_add_wtextchar(WTEXTCHARS *wtc,WTEXTCHAR *textchar)

    {
    static char *funcname="wtextchars_add_wtextchar";

    if (wtc->n>=wtc->na)
        {
        int newsize;
        newsize = wtc->na < 512 ? 1024 : wtc->na*2;
        willus_mem_realloc_robust_warn((void **)&wtc->wtextchar,newsize*sizeof(WTEXTCHAR),
                                    wtc->na*sizeof(WTEXTCHAR),funcname,10);
        wtc->na=newsize;
        }
    wtc->wtextchar[wtc->n++]=(*textchar);
    wtc->sorted=0;
    }


void wtextchars_remove_wtextchar(WTEXTCHARS *wtc,int index)

    {
    if (index>=wtc->n)
        return;
    if (index<wtc->n-1)
        memmove(&wtc->wtextchar[index],&wtc->wtextchar[index+1],sizeof(WTEXTCHAR)*(wtc->n-index-1));
    wtc->n--;
    }


/*
** rot_deg s/b multiple of 90.
*/
void wtextchars_rotate_clockwise(WTEXTCHARS *wtc,int rot_deg)

    {
    int i;

    while (rot_deg<0)
        rot_deg += 360;
    rot_deg = rot_deg % 360;
    rot_deg = (rot_deg+45)/90;
    rot_deg = rot_deg&3;
    if (rot_deg==0)
        return;
    for (i=0;i<wtc->n;i++)
        wtextchar_rotate_clockwise(&wtc->wtextchar[i],rot_deg,wtc->width,wtc->height);
    if (rot_deg&1)
        {
        double t;
        t=wtc->width;
        wtc->width=wtc->height;
        wtc->height=t;
        }
    }


void wtextchars_scale_page(WTEXTCHARS *wtextchars,double scale_factor)

    {
    int i;

    wtextchars->width *= scale_factor;
    wtextchars->height *= scale_factor;
    for (i=0;i<wtextchars->n;i++)
        wtextchar_scale(&wtextchars->wtextchar[i],scale_factor);
    }


static void wtextchar_scale(WTEXTCHAR *wtc,double scale_factor)

    {
    wtc->xp *= scale_factor;
    wtc->yp *= scale_factor;
    wtc->x1 *= scale_factor;
    wtc->y1 *= scale_factor;
    wtc->x2 *= scale_factor;
    wtc->y2 *= scale_factor;
    }


/*
** rot = 1 for 90
**       2 for 180
**       3 for 270
*/
static void wtextchar_rotate_clockwise(WTEXTCHAR *wch,int rot,double page_width_pts,
                                       double page_height_pts)

    {
    point_rotate(&wch->xp,&wch->yp,rot,page_width_pts,page_height_pts);
    point_rotate(&wch->x1,&wch->y1,rot,page_width_pts,page_height_pts);
    point_rotate(&wch->x2,&wch->y2,rot,page_width_pts,page_height_pts);
    point_sort(&wch->x1,&wch->x2);
    point_sort(&wch->y1,&wch->y2);
    }


static void point_sort(double *x1,double *x2)

    {
    if ((*x2) < (*x1))
        {
        double t;
        t=(*x1);
        (*x1)=(*x2);
        (*x2)=t;
        }
    }


static void point_rotate(double *x,double *y,int rot,double page_width_pts,double page_height_pts)

    {
    double x0,y0;

    x0=(*x);
    y0=(*y);
    switch (rot)
        {
        case 1:
            (*y)=x0;
            (*x)=page_height_pts-y0;
            break;
        case 2:
            (*x)=page_width_pts-x0;
            (*y)=page_height_pts-y0;
            break;
        case 3:
            (*y)=page_width_pts-x0;
            (*x)=y0;
            break;
        }
    }


/*
** x1,y1 = upper left bounding box of text
** x2,y2 = lower right bounding box of text
** (*text) gets allocated and then a UTF-8 string of the text inside the bounding box.
*/
void wtextchars_text_inside(WTEXTCHARS *src,char **text,double x1,double y1,double x2,double y2)

    {
    WTEXTCHARS _dst,*dst;
    int *unicode,utf8len,i,i2,j,n;
    char *t;
    static char *funcname="wtextchars_text_inside";

    dst=&_dst;
    wtextchars_init(dst);
    wtextchars_get_chars_inside(src,dst,x1,y1,x2,y2);
    willus_mem_alloc_warn((void **)&unicode,sizeof(int)*dst->n,funcname,10);
    /* Clean off spaces/tabs from ends */
    for (i=0;i<dst->n && (dst->wtextchar[i].ucs==32 || dst->wtextchar[i].ucs==9);i++);
    for (i2=dst->n-1;i2>=i && (dst->wtextchar[i2].ucs==32 || dst->wtextchar[i2].ucs==9);i2--);
    for (j=0;i<=i2;i++)
        unicode[j++]=dst->wtextchar[i].ucs;
    n=j;
    wtextchars_free(dst);
    utf8len=n==0 ? 0 : utf8_length(unicode,n);
    willus_mem_alloc_warn((void **)text,utf8len+1,funcname,10);
    t=(*text);
    unicode_to_utf8(t,unicode,n);
    willus_mem_free((double **)&unicode,funcname);
    }


void wtextchars_get_chars_inside(WTEXTCHARS *src,WTEXTCHARS *dst,double x1,double y1,
                                 double x2,double y2)

    {
    int i,i1,i2;
    double dy,xl,xr,yt,yb,xc,yc;

    wtextchars_clear(dst);
    dy=y2-y1;
/*
    i1=wtextchars_index_by_yp(src_sort2,y1-dy*.001,2);
    if (i1>=src->n)
        return;
    i2=wtextchars_index_by_yp(src_sort1,y2+dy*.001,1);
    if (i2<=i1)
        return;
*/
    i1=0;
    i2=src->n;
    yt=y1-dy*.001;
    yb=y2+dy*.001;
    xl=x1-dy*.1;
    xr=x2;
    xc=(x1+x2)/2.;
    yc=(y1+y2)/2.;
    for (i=i1;i<i2;i++)
        {
        double cxc,cyc;

        /* If word box is completely outside char box, skip */
        if (src->wtextchar[i].x2 < x1 || src->wtextchar[i].x1 > x2
              || src->wtextchar[i].y2 < y1 || src->wtextchar[i].y1 > y2)
            continue;
        /* There is some overlap */
        cxc = (src->wtextchar[i].x1 + src->wtextchar[i].x2)/2.;
        cyc = (src->wtextchar[i].y1 + src->wtextchar[i].y2)/2.;
        /*
        ** In both directions (horizontal and vertical), determine if either
        **     A. The center of the char box is inside the word box, or
        **     B. The center of the word box is inside the char box.
        ** If this is true in both directions, keep the char.
        */
        if (((cxc >= xl && src->wtextchar[i].x1 <= xr) || (xc >= src->wtextchar[i].x1 && xc <= src->wtextchar[i].x2))
              && ((cyc>=yt && cyc<=yb) || (yc >= src->wtextchar[i].y1 && yc <= src->wtextchar[i].y2)))
            wtextchars_add_wtextchar(dst,&src->wtextchar[i]);
        }
    wtextchars_sort_horizontally_by_position(dst);
    }


#if 0
/*
** Return index of the first character that has y_type >= yp.
** If wtc->n==0, returns -1.
** If all chars are < yp, returns wtc->n.
*/
static int wtextchars_index_by_yp(WTEXTCHARS *wtc,double yp,int type)

    {
    int i1,i2,c;

    if (wtc->n<=0)
        return(-1);
    wtextchars_sort_vertically_by_position(wtc,type);
    c = (type==1) ? (wtc->wtextchar[0].y1 >= yp) : (wtc->wtextchar[0].y2 >= yp);
    if (c)
        return(0);
    c = (type==1) ? (wtc->wtextchar[wtc->n-1].y1 < yp) : (wtc->wtextchar[wtc->n-1].y2 < yp);
    if (c)
        return(wtc->n);
    i1=0;
    i2=wtc->n-1;
    while (i2-i1>1)
        {
        int imid;

        imid=(i1+i2)/2;
        c = (type==1) ? (wtc->wtextchar[imid].y1 < yp) : (wtc->wtextchar[imid].y2 < yp);
        if (c)
            i1=imid;
        else
            i2=imid;
        }
   return(i2);
   }
#endif


void wtextchars_sort_vertically_by_position(WTEXTCHARS *wtc,int type)

    {
    int top,n1,n;
    WTEXTCHAR x0,*x;

    if (wtc->sorted==10+type)
        return;
    x=wtc->wtextchar;
    n=wtc->n;
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
            if (child<n1 && wtextchar_compare_vert_ex(&x[child],&x[child+1],type)<0)
                child++;
            if (wtextchar_compare_vert_ex(&x0,&x[child],type)<0)
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
    wtc->sorted=10+type;
    }


static int wtextchar_compare_vert_ex(WTEXTCHAR *c1,WTEXTCHAR *c2,int index)

    {
    double dy;

    dy=wtextchar_compare_vert(c1,c2,index);
    if (fabs(dy)>1e-8)
        return(dy);
    return(wtextchar_compare_horiz(c1,c2));
    }
 

static int wtextchar_compare_vert(WTEXTCHAR *c1,WTEXTCHAR *c2,int index)

    {
    if (index==1)
        return(c1->y1-c2->y1);
    else if (index==2)
        return(c1->y2-c2->y2);
    else if (index==3)
        {
        double percentage_overlap;
        double h,ol;
        if (c1->y2 <= c2->y1)
            return(-1);
        if (c1->y1 >= c2->y2)
            return(1);
        h=(c2->y2-c2->y1) < (c1->y2-c1->y1) ? c2->y2-c2->y1 : c1->y2-c1->y1;
        if (h<0.1)
            h=0.1;
        ol=c1->y2<c2->y2 ? c1->y2-c2->y1 : c2->y2-c1->y1;
        percentage_overlap=ol*100./h;
        if (percentage_overlap < 7.)
            return(c1->y2 < c2->y2 ? -1 : 1);
        return(0);
        }
    else
        return(c1->yp-c2->yp);
    }


void wtextchars_sort_horizontally_by_position(WTEXTCHARS *wtc)

    {
    if (wtc->sorted==2)
        return;
    wtextchar_array_sort_horizontally_by_position(wtc->wtextchar,wtc->n);
    wtc->sorted=2;
    }


void wtextchar_array_sort_horizontally_by_position(WTEXTCHAR *x,int n)

    {
    int top,n1;
    WTEXTCHAR x0;

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
            if (child<n1 && wtextchar_compare_horiz(&x[child],&x[child+1])<0)
                child++;
            if (wtextchar_compare_horiz(&x0,&x[child])<0)
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


static int wtextchar_compare_horiz(WTEXTCHAR *c1,WTEXTCHAR *c2)

    {
    if (c1->xp==c2->xp)
        return((c1->x1+c1->x2) - (c2->x1+c2->x2));
    return(c1->xp-c2->xp);
    }


void wtextchars_to_strbuf_formatted(WTEXTCHARS *wtcs,STRBUF *sbuf)

    {
    double vert_spacing_threshold;
    static char *funcname="wtextchars_group_by_words";
    int i,i0,nnl;
    double xmin,spacesize; /* Left most character position in points and size of space in points */

    if (wtcs->n<=0)
        return;
    {
    double *ch; /* population of character heights */

    willus_mem_alloc_warn((void **)&ch,sizeof(double)*wtcs->n,funcname,10);
    /* Create population character heights to determine row spacing threshold */
    for (xmin=99999.,i=0;i<wtcs->n-1;i++)
        {
        ch[i]=wtcs->wtextchar[i].y2-wtcs->wtextchar[i].y1;
        if (wtcs->wtextchar[i].xp<xmin)
            xmin=wtcs->wtextchar[i].xp;
        }
    sortd(ch,wtcs->n);
    vert_spacing_threshold=0.67*ch[wtcs->n/10];
    if (vert_spacing_threshold<=2.0)
        vert_spacing_threshold=2.0;
    spacesize=0.5*ch[wtcs->n/5];
    if (spacesize < 2.0)
        spacesize = 2.0;
    willus_mem_free(&ch,funcname);
    }

    /* Group characters row by row and add one row at a time */
    for (nnl=0,i0=0,i=1;i<wtcs->n;i++)
        {
        double dy,h;

        h=wtcs->wtextchar[i].y2 - wtcs->wtextchar[i].y1;
        if (h<2.)
            h=2.;
        dy=wtcs->wtextchar[i].yp - wtcs->wtextchar[i-1].yp;
        if (dy >= h*.75 || dy >= vert_spacing_threshold)
            {
            wtextchars_to_strbuf_one_line_1(wtcs,i0,i-1,sbuf,xmin,spacesize,nnl);
            nnl=dy/vert_spacing_threshold/1.5;
            if (nnl>2)
                nnl=2;
            if (nnl<1)
                nnl=1;
            i0=i;
            }
        }
    wtextchars_to_strbuf_one_line_1(wtcs,i0,wtcs->n-1,sbuf,xmin,spacesize,nnl);
    }


static void wtextchars_to_strbuf_one_line_1(WTEXTCHARS *wtcs,int i0,int i1,STRBUF *sbuf,
                                            double xmin,double spacesize,int nnl)

    {
    WTEXTCHARS *line,_line;
    int i;

    if (i0>i1)
        return;
    line=&_line;
    wtextchars_init(line);
    for (i=i0;i<=i1;i++)
        wtextchars_add_wtextchar(line,&wtcs->wtextchar[i]);
    wtextchars_sort_horizontally_by_position(line);
    strbuf_newline_with_spaces(sbuf,nnl,0);
    wtextchars_to_strbuf_one_line(line,sbuf,xmin,spacesize);
    wtextchars_free(line);
    }


static void strbuf_newline_with_spaces(STRBUF *sbuf,int nnl,int n)

    {
    for (;nnl>0;nnl--)
        strbuf_cat_ex(sbuf,"\n");
    for (;n>0;n--)
        strbuf_cat_ex(sbuf," ");
    }


/*
** xmin = left margin in points
** spacesize = approx. size of space in points
*/
static void wtextchars_to_strbuf_one_line(WTEXTCHARS *wtcs,STRBUF *sbuf,double xmin,double spacesize)

    {
    int i;

    if (wtcs->n<=0)
        return;
    for (i=0;i<wtcs->n;i++)
        {
        double dx;
        int nc;
        int u16str[2];
        char utf8str[8];

        /* If letters are physically separated, add space(s) */
        dx = wtcs->wtextchar[i].x1 - (i==0 ? xmin : wtcs->wtextchar[i-1].x2);
        if (dx > spacesize/2.)
            {
            int n1;

            n1 = dx>spacesize*3. ? (int)dx/spacesize : 1;
            if (wtcs->wtextchar[i].ucs==' ')
                n1--;
            for (;n1>0;n1--)
                strbuf_cat_ex(sbuf," ");
            }

        /*
        ** Look for ligatured pattern--v2.33 with MuPDF v1.7
        ** Ligature pattern, e.g. "fi"
        ** As of MuPDF 1.7, a ligatured "fi" is like this:
        ** f has zero width, followed by space at same position w/zero width,
        ** followed by i at same position, followed by space of zero width.
        */
        nc=0;
        if (wtextchars_ligature_pattern(wtcs,i))
            {
            u16str[nc++]=wtcs->wtextchar[i].ucs;
            u16str[nc++]=wtcs->wtextchar[i+2].ucs;
            i+=3;
            }
        else
            u16str[nc++]=wtcs->wtextchar[i].ucs;
        unicode_to_utf8(utf8str,u16str,nc);
        strbuf_cat_ex(sbuf,utf8str);
        }
    }


/*
** For MuPDF v1.7 and up
*/
static int wtextchars_ligature_pattern(WTEXTCHARS *wtcs,int index)

    {
    return(index<wtcs->n-3
             && wtcs->wtextchar[index].ucs!=' ' 
             && wtcs->wtextchar[index+1].ucs==' '
             && wtcs->wtextchar[index+2].ucs!=' '
             && wtcs->wtextchar[index+3].ucs==' '
             && fabs(wtcs->wtextchar[index+1].x1-wtcs->wtextchar[index].x1)<.01
             && fabs(wtcs->wtextchar[index+2].x1 - wtcs->wtextchar[index].x1)<.01);
    }


void wtextchars_to_easyplot(WTEXTCHARS *wtcs,char *filename)

    {
    FILE *f;
    int i;

    f=fopen(filename,"w");
    if (f==NULL)
        return;
    fprintf(f,"/sc on\n/sd off\n/sm off\n");
    for (i=0;i<wtcs->n;i++)
        {
        WTEXTCHAR *wtc;
        double x1,y1,x2,y2;

        wtc=&wtcs->wtextchar[i];
        x1=wtc->x1;
        x2=wtc->x2;
        y1=792.-wtc->y2;
        y2=792.-wtc->y1;
        fprintf(f,"/sa m 1 2\n%g %g\n%g %g\n%g %g\n%g %g\n%g %g\n//nc\n",x1,y1,x1,y2,x2,y2,x2,y1,x1,y1);
        fprintf(f,"/aa %g %g \"%d.%c\"\n",x1,y1+(y2-y1)*.2,i+1,wtc->ucs);
        }
    fclose(f);
    }
