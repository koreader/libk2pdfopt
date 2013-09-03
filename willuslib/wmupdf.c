/*
** wmupdf.c    Routines to interface w/mupdf lib (except for bmp functions, which
**             are in bmpmupdf.c).
**
** Part of willus.com general purpose C code library.
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
#include <stdio.h>
#include "willus.h"

#ifdef HAVE_Z_LIB
#include <zlib.h>
#endif

#ifdef HAVE_MUPDF_LIB
#include <mupdf.h>

static void wpdfbox_determine_original_source_position(WPDFBOX *box);
static void wpdfbox_unrotate(WPDFBOX *box,double deg);
static int wpdfbox_compare(WPDFBOX *b1,WPDFBOX *b2);
static void info_update(pdf_document *xref,char *producer);
static void dict_put_string(pdf_document *doc,pdf_obj *dict,char *key,char *string);
static void wmupdf_page_bbox(pdf_obj *srcpage,double *bbox_array);
static int wmupdf_pdfdoc_newpages(pdf_document *xref,fz_context *ctx,WPDFPAGEINFO *pageinfo,
                                  int use_forms,FILE *out);
static void set_clip_array(double *xclip,double *yclip,double rot_deg,double width,double height);
static void cat_pdf_double(char *buf,double x);
static void wmupdf_convert_pages_to_forms(pdf_document *xref,fz_context *ctx,int *srcpageused);
static void wmupdf_convert_single_page_to_form(pdf_document *xref,fz_context *ctx,
                                               pdf_obj *srcpageref,int pageno);
static int stream_deflate(pdf_document *xref,fz_context *ctx,int pageref,int pagegen,
                          int *length);
static int add_to_srcpage_stream(pdf_document *xref,fz_context *ctx,int pageref,int pagegen,
                                 pdf_obj *dict);
static char *xobject_name(int pageno);
static pdf_obj *start_new_destpage(pdf_document *doc,double width_pts,double height_pts);
static void wmupdf_preserve_old_dests(pdf_obj *olddests,fz_context *ctx,pdf_document *xref,
                                      pdf_obj *pages);
static int new_stream_object(pdf_document *xref,fz_context *ctx,char *buf);
static void wmupdf_dict_merge(fz_context *ctx,char *distname,pdf_obj *dstdict,pdf_obj *srcdict);
static void wmupdf_dict_merge_keyval(fz_context *ctx,pdf_obj *dstdict,pdf_obj *key,pdf_obj *value);
static void wmupdf_array_merge(fz_context *ctx,char *arrayname,pdf_obj *dstarray,pdf_obj *srcarray);
static void matrix_zero_round(double m[][3]);
static void matrix_unity(double m[][3],double val);
static void matrix_set_all(double m[][3],double val);
static void matrix_translate(double m[][3],double x,double y);
static void matrix_mul(double dst[][3],double src[][3]);
static void matrix_rotate(double m[][3],double deg);
static void matrix_xymul(double m[][3],double *x,double *y);

/* Character positions */
static void wtextchars_add_fz_chars(WTEXTCHARS *wtc,fz_context *ctx,fz_text_page *page);
static void wtextchar_rotate_clockwise(WTEXTCHAR *wch,int rot,double page_width_pts,
                                       double page_height_pts);
static void point_sort(double *x1,double *x2);
static void point_rotate(double *x,double *y,int rot,double page_width_pts,double page_height_pts);
static void wtextchars_get_chars_inside(WTEXTCHARS *src,WTEXTCHARS *dst,double x1,double y1,
                                        double x2,double y2);
/*
static int  wtextchars_index_by_yp(WTEXTCHARS *wtc,double yp,int type);
static void wtextchars_sort_vertically_by_position(WTEXTCHARS *wtc,int type);
static int  wtextchar_compare_vert(WTEXTCHAR *c1,WTEXTCHAR *c2,int type);
*/
static void wtextchars_sort_horizontally_by_position(WTEXTCHARS *wtc);
static int  wtextchar_compare_horiz(WTEXTCHAR *c1,WTEXTCHAR *c2);


int wmupdf_numpages(char *filename)

    {
    fz_context *ctx;
    fz_document *doc;
    int np;

    doc=NULL;
    ctx = fz_new_context(NULL,NULL,FZ_STORE_DEFAULT);
    if (!ctx)
        return(-1);
    fz_try(ctx) { doc=fz_open_document(ctx,filename); }
    fz_catch(ctx)
        {
        fz_free_context(ctx);
        return(-2);
        }
    np=fz_count_pages(doc);
    fz_close_document(doc);
    fz_flush_warnings(ctx);
    fz_free_context(ctx);
    return(np);
    }


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
static void wpdfbox_unrotate(WPDFBOX *box,double deg)

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


static void wpdfbox_determine_original_source_position(WPDFBOX *box)

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


/*
** Get /Info strings, e.g. wmupdf_info_field("myfile.pdf","Author",buf,255);
** Info labels:
**     Title         Document title
**     Author        Name of the person who created the doc.
**     Subject       Subject of the doc
**     Keywords      Keywords associated with the document.
**     Creator       If doc was converted to PDF from another format, the name
**                   of the product that created the original document.
**     Producer      If doc was converted to PDF from another format, the name
**                   of the product that converted it to PDF.
**     CreationDate  Date/Time document was created.
**     ModDate       Date/Time of most recent mod.
*/
int wmupdf_info_field(char *infile,char *label,char *buf,int maxlen)

    {
    pdf_document *xref;
    fz_context *ctx;
    pdf_obj *info,*obj;
    char *password="";

    xref=NULL;
    buf[0]='\0';
    ctx = fz_new_context(NULL,NULL,FZ_STORE_UNLIMITED);
    if (!ctx)
        return(-1);
    fz_try(ctx)
        {
        xref=pdf_open_document_no_run(ctx,infile);
        if (!xref)
            {
            fz_free_context(ctx);
            return(-2);
            }
        if (pdf_needs_password(xref) && !pdf_authenticate_password(xref,password))
            {
            pdf_close_document(xref);
            fz_free_context(ctx);
            return(-3);
            }
        if (pdf_trailer(xref)!=NULL
            && (info=pdf_dict_gets(pdf_trailer(xref),"Info"))!=NULL
            && (obj=pdf_dict_gets(info,label))!=NULL
            && pdf_is_string(obj))
            {
            strncpy(buf,pdf_to_str_buf(obj),maxlen-1);
            buf[maxlen-1]='\0';
            }
        }
    fz_always(ctx)
        {
        pdf_close_document(xref);
        }
    fz_catch(ctx)
        {
        }
    fz_free_context(ctx);
    return(0);
    }


/*
** Reconstruct PDF file per the information in pageinfo.
** use_forms==0:  Old-style reconstruction where the pages are not turned into XObject Forms.
** use_forms==1:  New-style where pages are turned into XObject forms.
*/
int wmupdf_remake_pdf(char *infile,char *outfile,WPDFPAGEINFO *pageinfo,int use_forms,FILE *out)

    {
    pdf_document *xref;
    fz_context *ctx;
    fz_write_options fzopts;
    char *password="";
    int status;
    int write_failed;

    memset(&fzopts,0,sizeof(fz_write_options));
    fzopts.do_incremental=0;
    fzopts.do_ascii=0;
    fzopts.do_expand=0;
    fzopts.do_linear=0;
    fzopts.do_garbage=1; /* 2 and 3 don't work for this. */
    fzopts.continue_on_error=0;
    fzopts.errors=NULL;
    write_failed=0;
    wpdfpageinfo_sort(pageinfo);
    xref=NULL;
    /* New context */
    ctx = fz_new_context(NULL,NULL,FZ_STORE_UNLIMITED);
    if (!ctx)
        {
        nprintf(out,"wmupdf_remake_pdf:  Cannot initialize context.\n");
        return(-1);
        }
    fz_try(ctx)
        {
        xref=pdf_open_document_no_run(ctx,infile);
        if (!xref)
            {
            fz_free_context(ctx);
            nprintf(out,"wmupdf_remake_pdf:  Cannot open PDF file %s.\n",infile);
            return(-2);
            }
        if (pdf_needs_password(xref) && !pdf_authenticate_password(xref,password))
            {
            pdf_close_document(xref);
            fz_free_context(ctx);
            nprintf(out,"wmupdf_remake_pdf:  Cannot authenticate PDF file %s.\n",infile);
            return(-3);
            }
        status=wmupdf_pdfdoc_newpages(xref,ctx,pageinfo,use_forms,out);
        if (status<0)
            {
            pdf_close_document(xref);
            fz_free_context(ctx);
            nprintf(out,"wmupdf_remake_pdf:  Error re-paginating PDF file %s.\n",infile);
            return(status);
            }
        info_update(xref,pageinfo->producer);
        /* Write output */
        pdf_write_document(xref,outfile,&fzopts);
        }
    fz_always(ctx)
        {
        pdf_close_document(xref);
        }
    fz_catch(ctx)
        {
        write_failed=1;
        }
    fz_free_context(ctx);
    if (write_failed)
        {
        nprintf(out,"wmupdf_remake_pdf:  Error writing output PDF file %s.\n",outfile);
        return(-10);
        }
    return(0);
    }


static void info_update(pdf_document *xref,char *producer)

    {
    char moddate[64];
    time_t now;
    struct tm date;
    pdf_obj *info;
    int newinfo;

    if (pdf_trailer(xref)==NULL)
        return;
    time(&now);
    date=(*localtime(&now));
    sprintf(moddate,"D:%04d%02d%02d%02d%02d%02d%s",
           date.tm_year+1900,date.tm_mon+1,date.tm_mday,
           date.tm_hour,date.tm_min,date.tm_sec,
           wsys_utc_string());
    info=pdf_dict_gets(pdf_trailer(xref),"Info");
    if (info==NULL)
        {
        newinfo=1;
        info=pdf_new_dict(xref,2);
        }
    else
        newinfo=0;
    dict_put_string(xref,info,"Producer",producer);
    dict_put_string(xref,info,"ModDate",moddate);
    if (newinfo)
        {
        pdf_dict_puts(pdf_trailer(xref),"Info",info);
        pdf_drop_obj(info);
        }
    }


static void dict_put_string(pdf_document *doc,pdf_obj *dict,char *key,char *string)

    {
    pdf_obj *value;

    value=pdf_new_string(doc,string,strlen(string));
    pdf_dict_puts(dict,key,value);
    pdf_drop_obj(value);
    }


/*
** Look at CropBox and MediaBox entries to determine visible page origin.
*/
static void wmupdf_page_bbox(pdf_obj *srcpage,double *bbox_array)

    {
    int i;

    bbox_array[0] = bbox_array[1] = -1e10;
    bbox_array[2] = bbox_array[3] = 1e10;
    for (i=0;i<2;i++)
        {
        static char *boxname[] = {"MediaBox","CropBox"};
        pdf_obj *box;

        box=pdf_dict_gets(srcpage,boxname[i]);
        if (box!=NULL)
            {
            int j;
            for (j=0;j<4;j++)
                {
                pdf_obj *obj;

                obj=pdf_array_get(box,j);
                if (obj!=NULL)
                    {
                    double x;
                    x=pdf_to_real(obj);
                    if ((j<2 && x>bbox_array[j]) || (j>=2 && x<bbox_array[j]))
                        bbox_array[j]=x;
                    }
                }
            }
        }
    if (bbox_array[0] < -9e9)
        bbox_array[0] = 0.;
    if (bbox_array[1] < -9e9)
        bbox_array[1] = 0.;
    if (bbox_array[2] > 9e9)
        bbox_array[2] = 612.;
    if (bbox_array[3] > 9e9)
        bbox_array[3] = 792.;
    }


static int wmupdf_pdfdoc_newpages(pdf_document *xref,fz_context *ctx,WPDFPAGEINFO *pageinfo,
                                  int use_forms,FILE *out)

    {
    static char *funcname="wmupdf_pdfdoc_newpages";
    pdf_obj *root,*oldroot,*pages,*kids,*countobj,*parent,*olddests;
    pdf_obj *srcpageobj,*srcpagecontents;
    pdf_obj *destpageobj,*destpagecontents,*destpageresources;
    double srcx0,srcy0;
    int qref,i,i0,pagecount,srccount,destpageref,nbb;
    int *srcpageused;
    char *bigbuf;
    double srcpagerot;

    /* Avoid compiler warning */
    destpageref = 0;
    destpageobj = NULL;
    srcx0=srcy0=0.;
    /* Keep only pages/type and (reduced) dest entries to avoid references to unretained pages */
    pagecount = pdf_count_pages(xref);
    if (use_forms)
        {
        willus_mem_alloc_warn((void **)&srcpageused,sizeof(int)*(pagecount+1),funcname,10);
        /* Mark all source pages as "not done" */
        for (i=0;i<=pagecount;i++)
            srcpageused[i]=0;
        nbb=4096;
        willus_mem_alloc_warn((void **)&bigbuf,nbb,funcname,10);
        bigbuf[0]='\0';
        }
    oldroot = pdf_dict_gets(pdf_trailer(xref),"Root");
    /*
    ** pages points to /Pages object in PDF file.
    ** Has:  /Type /Pages, /Count <numpages>, /Kids [ obj obj obj obj ]
    */
    pages = pdf_dict_gets(oldroot,"Pages");
    olddests = pdf_load_name_tree(xref,"Dests");

    /*
    ** Create new root object with only /Pages and /Type (and reduced dest entries)
    ** to avoid references to unretained pages.
    */
    root = pdf_new_dict(xref,4);
    pdf_dict_puts(root,"Type",pdf_dict_gets(oldroot,"Type"));
    pdf_dict_puts(root,"Pages",pages);
    pdf_update_object(xref,pdf_to_num(oldroot),root);
    pdf_drop_obj(root);

    /* Parent indirectly references the /Pages object in the file */
    /* (Each new page we create has to point to this.)            */
    parent = pdf_new_indirect(xref, pdf_to_num(pages), pdf_to_gen(pages));
    /* Create a new kids array with only the pages we want to keep */
    kids = pdf_new_array(xref, 1);


    qref=0;
    /* Avoid compiler warnings */
    destpageresources=NULL;
    destpagecontents=NULL;
    srcpagecontents=NULL;
    srcpagerot=0.;
    for (i=0;i<=pageinfo->boxes.n;i++)
        if (pageinfo->boxes.box[i].dstpage>0)
            break;
    if (i>0)
        {
        if (i<pageinfo->boxes.n)
            memmove(&pageinfo->boxes.box[0],&pageinfo->boxes.box[i],sizeof(WPDFBOX)*pageinfo->boxes.n-i);
        pageinfo->boxes.n -= i;
        }
    /* Walk through PFDBOXES array */
    for (i=srccount=i0=0;i<=pageinfo->boxes.n;i++)
        {
        WPDFBOX *box;
        int j,k,newsrc;
        static char buf[512];
        pdf_obj *s1indirect,*qindirect,*rotobj;
        static double cpm[3][3],m[3][3],m1[3][3];
        static double xclip[4],yclip[4];

/*
printf("box[%d/%d], srccount=%d\n",i,pageinfo->boxes.n,srccount);
if (i<pageinfo->boxes.n)
{
box=&pageinfo->boxes.box[i];
printf("    srcpage=%d, dstpage=%d\n",box->srcbox.pageno,box->dstpage);
printf("    x0=%g, y0=%g\n",box->x0,box->y0);
printf("    w=%g, h=%g\n",box->w,box->h);
printf("    x1=%g, y1=%g\n",box->x1,box->y1);
printf("    sr=%g, dr=%g\n",box->srcrot_deg,box->dstrot_deg);
printf("    scale=%g\n",box->scale);
}
*/
        /* Check to see if we are done with an output page */
        if (srccount>0 && (i==pageinfo->boxes.n
               || (i>0 && pageinfo->boxes.box[i].dstpage!=pageinfo->boxes.box[i-1].dstpage)))
            {
            pdf_obj *newpageref;
            /*
            ** Store destination page into document structure
            */
/*
printf("    ADDING NEW PAGE. (srccount=%d)\n",srccount);
*/
            if (use_forms)
                {
                pdf_obj *dest_stream;

                /* Create new object in document for destination page stream */
                dest_stream = pdf_new_indirect(xref,new_stream_object(xref,ctx,bigbuf),0);
                /* Store this into the destination page contents array */
                pdf_array_push(destpagecontents,dest_stream);
                pdf_drop_obj(dest_stream);
                }
            newpageref=pdf_new_indirect(xref,destpageref,0);
            /* Reference parent list of pages */
            pdf_dict_puts(destpageobj,"Parent",parent);
            pdf_dict_puts(destpageobj,"Contents",destpagecontents);
            pdf_dict_puts(destpageobj,"Resources",destpageresources);
            /* Store page object in document's kids array */
            pdf_array_push(kids,newpageref);
            /* Update document with new page */
            pdf_update_object(xref,destpageref,destpageobj);
            /* Clean up */
            pdf_drop_obj(newpageref);
            pdf_drop_obj(destpageresources);
            pdf_drop_obj(destpagecontents);
            pdf_drop_obj(destpageobj);
            /* Reset source page and index to start of new destination page */
            i0=i;
            srccount=0;
            }
        /* Quit loop if beyond last box */
        if (i>=pageinfo->boxes.n)
            break;
        box=&pageinfo->boxes.box[i];
        if (box->srcbox.pageno<1 || box->srcbox.pageno>pagecount)
            continue;
        /* Is this a source page we haven't processed yet (for this destination page)? */
        for (newsrc=1,j=i0;j<i;j++)
            {
            if (pageinfo->boxes.box[j].srcbox.pageno==box->srcbox.pageno)
                {
                newsrc=0;
                break;
                }
            }
        if (newsrc)
            {
            double v[4];

            srccount++;
            if (use_forms)
                srcpageused[box->srcbox.pageno]=1;
/*
printf("    NEW SOURCE PAGE (srccount=%d)\n",srccount);
*/
            if (srccount==1)
                {
                /*
                ** Start a new destination page.
                **
                ** Each new page object is a dict type with:
                ** /Type /Page
                ** /Contents (array of objects)
                ** /Resources (dict)
                ** /MediaBox [0 0 612 792]
                ** /Parent <PagesObj>
                ** [Can have /Rotate 90, for example.]
                **
                */
/*
printf("        (STARTING NEW DEST. PAGE)\n");
*/
                destpageobj=start_new_destpage(xref,box->dst_width_pts,box->dst_height_pts);
                destpageresources=pdf_new_dict(xref,1);
                if (use_forms)
                    pdf_dict_puts(destpageresources,"XObject",pdf_new_dict(xref,1));
                destpageref=pdf_create_object(xref);
                destpagecontents=pdf_new_array(xref,1);
                /* Init the destination page stream for forms */
                if (use_forms)
                    bigbuf[0]='\0';
                }
            /* New source page, so get the source page objects */
            /* srcpageobj = xref->page_objs[box->srcbox.pageno-1]; */
            /* pageno, or pageno-1?? */
            srcpageobj = pdf_resolve_indirect(pdf_lookup_page_obj(xref,box->srcbox.pageno-1));
            wmupdf_page_bbox(srcpageobj,v);
            srcx0=v[0];
            srcy0=v[1];
/*
printf("SRCX0=%g, SRCY0=%g\n",srcx0,srcy0);
*/
            rotobj=pdf_dict_gets(srcpageobj,"Rotate");
            srcpagerot = rotobj!=NULL ? pdf_to_real(rotobj) : 0.;
/*
printf("Page rotation = %g\n",srcpagerot);
*/
            srcpagecontents=pdf_dict_gets(srcpageobj,"Contents");
/*
if (pdf_is_array(srcpagecontents))
{
int k;
printf("    source page contents = array.\n");
for (k=0;k<pdf_array_len(srcpagecontents);k++)
{
pdf_obj *obj;
obj=pdf_array_get(srcpagecontents,k);
if (pdf_is_indirect(obj))
{
printf("    contents[%d] = indirect (%d)\n",k,pdf_to_num(obj));
pdf_resolve_indirect(obj);
}
}
}
*/
            if (use_forms)
                {
                pdf_obj *xobjdict,*pageref;
                int pageno;

                xobjdict=pdf_dict_gets(destpageresources,"XObject");
                pageno=box->srcbox.pageno;
                pageref=pdf_lookup_page_obj(xref,pageno-1);
                pdf_dict_puts(xobjdict,xobject_name(pageno),pageref);
                pdf_dict_puts(destpageresources,"XObject",xobjdict);
                }
            else
                {
                pdf_obj *srcpageresources;

                /* Merge source page resources into destination page resources */
                srcpageresources=pdf_dict_gets(srcpageobj,"Resources");
/*
printf("box->dstpage=%d, srcpage=%d (ind.#=%d)\n",box->dstpage,box->srcbox.pageno,pdf_to_num(xref->page_refs[box->srcbox.pageno-1]));
*/
                wmupdf_dict_merge(ctx,"Resources",destpageresources,srcpageresources);
                }
            }
        /*
        ** Process this source box:
        **
        ** Create a tranformation matrix and clipping path to only show the
        ** desired part of the source page at the appropriate place on the
        ** destination page.
        **
        ** How the tranformation matrix works:
        ** - Translations shall be specified as [ 1 0 0 1 tx ty ], where tx and ty
        **   shall be the distances to translate the origin of the coordinate system
        **   in the horizontal and vertical dimensions, respectively.
        **
        ** - Scaling shall be obtained by [ sx 0 0 sy 0 0 ]. This scales the coordinates
        **   so that 1 unit in the horizontal and vertical dimensions of the new coordinate
        **   system is the same size as sx and sy units, respectively, in the previous
        **   coordinate system.
        **
        ** - Rotations shall be produced by [ cos q sin q -sin q cos q 0 0 ], which has the
        **   effect of rotating the coordinate system axes by an angle q counter-clockwise.
        **
        ** - Skew shall be specified by [ 1 tan a tan b 1 0 0 ], which skews the x axis by
        **   an angle a and the y axis by an angle b.
        **
        */
        wpdfbox_determine_original_source_position(box);
/*
printf("Before unrotate.\n");
printf("box->srcrot=%g\n",box->srcrot_deg);
printf("box->x0=%g, box->y0=%g\n",box->x0,box->y0);
printf("box->w=%g, box->h=%g\n",box->w,box->h);
printf("box->pw=%g, box->ph=%g\n",box->src_width_pts,box->src_height_pts);
*/
        if (fabs(srcpagerot) > 1.0e-4)
            wpdfbox_unrotate(box,srcpagerot);
/*
printf("box->srcrot=%g\n",box->srcrot_deg);
printf("box->x0=%g, box->y0=%g\n",box->x0,box->y0);
printf("box->w=%g, box->h=%g\n",box->w,box->h);
printf("box->pw=%g, box->ph=%g\n",box->src_width_pts,box->src_height_pts);
*/
        matrix_unity(m,1.);
/*
printf("xfmatrix = [  %9.6f   %9.6f   %9.6f  ]\n"
       "           [  %9.6f   %9.6f   %9.6f  ]\n"
       "           [  %9.6f   %9.6f   %9.6f  ]\n",
        m[0][0],m[0][1],m[0][2],
        m[1][0],m[1][1],m[1][2],
        m[2][0],m[2][1],m[2][2]);
*/
        matrix_translate(m1,-box->x0-srcx0,-box->y0-srcy0);
        matrix_mul(m,m1);
        matrix_rotate(m1,-box->srcrot_deg+box->dstrot_deg);
        matrix_mul(m,m1);
        matrix_unity(m1,box->scale);
        matrix_mul(m,m1);
        matrix_translate(m1,box->x1,box->y1);
        matrix_mul(m,m1);
        matrix_zero_round(m);
        matrix_rotate(cpm,box->srcrot_deg);
        matrix_translate(m1,box->x0+srcx0,box->y0+srcy0);
        matrix_mul(cpm,m1);
/*
printf("Clip matrix:\n");
printf("xfmatrix = [  %9.6f   %9.6f   %9.6f  ]\n"
       "           [  %9.6f   %9.6f   %9.6f  ]\n"
       "           [  %9.6f   %9.6f   %9.6f  ]\n",
        cpm[0][0],cpm[0][1],cpm[0][2],
        cpm[1][0],cpm[1][1],cpm[1][2],
        cpm[2][0],cpm[2][1],cpm[2][2]);
*/


        set_clip_array(xclip,yclip,box->srcrot_deg,box->w,box->h);
        for (k=0;k<4;k++)
            matrix_xymul(cpm,&xclip[k],&yclip[k]);
/*
printf("Clip path:\n    %7.2f %7.2f\n    %7.2f,%7.2f\n    %7.2f,%7.2f\n"
                   "    %7.2f %7.2f\n    %7.2f,%7.2f\n",
                xclip[0],yclip[0],xclip[1],yclip[1],xclip[2],yclip[2],
                xclip[3],yclip[3],xclip[0],yclip[0]);
*/
        strcpy(buf,"q");
        for (k=0;k<=2;k++)
            {
            cat_pdf_double(buf,m[k][0]);
            cat_pdf_double(buf,m[k][1]);
            }
        strcat(buf," cm");
        for (k=0;k<=4;k++)
            {
            cat_pdf_double(buf,xclip[k&3]);
            cat_pdf_double(buf,yclip[k&3]);
            strcat(buf,k==0 ? " m" : " l");
            }
        strcat(buf," W n");
        if (use_forms)
            {
            /* FORM METHOD */
            sprintf(&buf[strlen(buf)]," /%s Do Q\n",xobject_name(box->srcbox.pageno));
            if (strlen(bigbuf)+strlen(buf) > nbb)
                {
                int newsize;
                newsize=nbb*2;
                willus_mem_realloc_robust_warn((void **)&bigbuf,newsize,nbb,funcname,10);
                nbb=newsize;
                }
            strcat(bigbuf,buf);
            }
        else
            {
            /* NO-FORMS METHOD */
            strcat(buf,"\n");
            /* Create new objects in document for tx matrix and restore matrix */
            s1indirect = pdf_new_indirect(xref,new_stream_object(xref,ctx,buf),0);
            if (qref==0)
                qref=new_stream_object(xref,ctx,"Q\n");
            qindirect = pdf_new_indirect(xref,qref,0);
            /* Store this region into the destination page contents array */
            pdf_array_push(destpagecontents,s1indirect);
            if (pdf_is_array(srcpagecontents))
                {
                int k;
                for (k=0;k<pdf_array_len(srcpagecontents);k++)
                    pdf_array_push(destpagecontents,pdf_array_get(srcpagecontents,k));
                }
            else
                pdf_array_push(destpagecontents,srcpagecontents);
            pdf_array_push(destpagecontents,qindirect);
            pdf_drop_obj(s1indirect);
            pdf_drop_obj(qindirect);
            }
        }
    pdf_drop_obj(parent);

    /* For forms, convert all original source pages to XObject Forms */
    if (use_forms)
        wmupdf_convert_pages_to_forms(xref,ctx,srcpageused);

    /* Update page count and kids array */
    countobj = pdf_new_int(xref, pdf_array_len(kids));
    pdf_dict_puts(pages, "Count", countobj);
    pdf_drop_obj(countobj);
    pdf_dict_puts(pages, "Kids", kids);
    pdf_drop_obj(kids);

    /* Also preserve the (partial) Dests name tree */
    if (olddests)
        wmupdf_preserve_old_dests(olddests,ctx,xref,pages);
    if (use_forms)
        {
        /* Free memory */
        willus_mem_free((double **)&bigbuf,funcname);
        willus_mem_free((double **)&srcpageused,funcname);
        }
    return(0);
    }


static void set_clip_array(double *xclip,double *yclip,double rot_deg,double width,double height)

    {
    double w,h;
    double drot;
    int nrot;

    drot=fmod(rot_deg,360.);
    if (drot < 0.)
        drot += 360.;
    nrot=(int)((drot+45.)/90.);
    if (nrot&1)
        {
        w=height;
        h=width;
        }
    else
        {
        w=width;
        h=height;
        }
    xclip[0]=0.;
    yclip[0]=0.;
    xclip[1]=w;
    yclip[1]=0.;
    xclip[2]=w;
    yclip[2]=h;
    xclip[3]=0.;
    yclip[3]=h;
    }

/*
** Try to print the shortest possible version of the number, but don't use
** scientific notation (not allowed in PDF).
*/
static void cat_pdf_double(char *buf,double x)

    {
    char fmt[8];
    int j,ix,neg;
    double m;

    if (x<0)
        {
        neg=1;
        x = -x;
        }
    else
        neg=0;
    if (x > 999999.)
        x = 999999.;
    for (j=0,m=1.;j<5;j++,m*=10.)
        {
        ix=(int)(m*x+.5);
        if (fabs(x-ix/m) < 1e-6)
            break;
        }
    if (j==0)
        {
        sprintf(&buf[strlen(buf)]," %d",neg && ix>0 ? -ix : ix);
        return;
        }
    sprintf(fmt," %%.%df",j);
    sprintf(&buf[strlen(buf)],fmt,neg ? -x : x);
    }


static void wmupdf_convert_pages_to_forms(pdf_document *xref,fz_context *ctx,int *srcpageused)

    {
    int i,pagecount;
    pdf_obj **srcpage;
    static char *funcname="wmupdf_convert_pages_to_forms";

    pagecount = pdf_count_pages(xref);
    willus_mem_alloc_warn((void **)&srcpage,sizeof(pdf_obj *)*pagecount,funcname,10);
    /*
    ** Lookup all page references before we change them to XObjects, because
    ** after they are changed to XObjects, pdf_lookup_page_obj() fails.
    */
    for (i=1;i<=pagecount;i++)
        if (srcpageused[i])
            srcpage[i-1] = pdf_lookup_page_obj(xref,i-1);
    for (i=1;i<=pagecount;i++)
        if (srcpageused[i])
            wmupdf_convert_single_page_to_form(xref,ctx,srcpage[i-1],i);
    willus_mem_free((double **)&srcpage,funcname);
    }


static void wmupdf_convert_single_page_to_form(pdf_document *xref,fz_context *ctx,
                                               pdf_obj *srcpageref,int pageno)

    {
    pdf_obj *array,*srcpageobj,*srcpagecontents;
    int i,len,streamlen,pageref,pagegen,compressed;
    double bbox_array[4];
    double matrix[6];

    srcpageobj = pdf_resolve_indirect(srcpageref);
    pageref=pdf_to_num(srcpageref);
    pagegen=pdf_to_gen(srcpageref);
    wmupdf_page_bbox(srcpageobj,bbox_array);
    for (i=0;i<6;i++)
        matrix[i]=0.;
    matrix[0]=matrix[3]=1.;
    srcpagecontents=pdf_dict_gets(srcpageobj,"Contents");
    /* Concatenate all indirect streams from source page directly into it. */
/* printf("Adding streams to source page %d (pageref=%d, pagegen=%d)...\n",pageno,pageref,pagegen); */
    streamlen=0;
    if (pdf_is_array(srcpagecontents))
        {
        int k;
        for (k=0;k<pdf_array_len(srcpagecontents);k++)
            {
            pdf_obj *obj;
            obj=pdf_array_get(srcpagecontents,k);
            if (pdf_is_indirect(obj))
                pdf_resolve_indirect(obj);
            streamlen=add_to_srcpage_stream(xref,ctx,pageref,pagegen,obj);
            }
        }
    else
        {
        if (pdf_is_indirect(srcpagecontents))
            pdf_resolve_indirect(srcpagecontents);
        streamlen=add_to_srcpage_stream(xref,ctx,pageref,pagegen,srcpagecontents);
        }
    compressed=stream_deflate(xref,ctx,pageref,pagegen,&streamlen);
    len=pdf_dict_len(srcpageobj);
    for (i=0;i<len;i++)
        {
        pdf_obj *key; /* *value */

        key=pdf_dict_get_key(srcpageobj,i);
/*
if (pdf_is_name(key))
printf("key[%d] = name = %s\n",i,pdf_to_name(key));
else
printf("key[%d] = ??\n",i);
*/
        /* value=pdf_dict_get_val(srcpageobj,i); */
        /* Keep same resources */
        if (!pdf_is_name(key))
            continue;
        if (pdf_is_name(key) && !stricmp("Resources",pdf_to_name(key)))
            continue;
        /* Drop dictionary entry otherwise */
        pdf_dict_del(srcpageobj,key);
        i=-1;
        len=pdf_dict_len(srcpageobj);
        }
    /*
    ** Once we turn the object into an XObject type (and not a Page type)
    ** it can no longer be looked up using pdf_lookup_page_obj() as of MuPDF v1.3
    */
    pdf_dict_puts(srcpageobj,"Type",pdf_new_name(xref,"XObject"));
    pdf_dict_puts(srcpageobj,"Subtype",pdf_new_name(xref,"Form"));
    pdf_dict_puts(srcpageobj,"FormType",pdf_new_int(xref,1));
    if (compressed)
        pdf_dict_puts(srcpageobj,"Filter",pdf_new_name(xref,"FlateDecode"));
    pdf_dict_puts(srcpageobj,"Length",pdf_new_int(xref,streamlen));
    array=pdf_new_array(xref,4);
    for (i=0;i<4;i++)
        pdf_array_push(array,pdf_new_real(xref,bbox_array[i]));
    pdf_dict_puts(srcpageobj,"BBox",array);
    array=pdf_new_array(xref,6);
    for (i=0;i<6;i++)
        pdf_array_push(array,pdf_new_real(xref,matrix[i]));
    pdf_dict_puts(srcpageobj,"Matrix",array);
    }


static int stream_deflate(pdf_document *xref,fz_context *ctx,int pageref,int pagegen,
                          int *length)

    {
    fz_buffer *strbuf;
    int n;
    unsigned char *p;
#ifdef HAVE_Z_LIB
    char tempfile[512];
    gzFile gz;
    FILE *f;
    int nw;
#endif

    strbuf=pdf_load_stream(xref,pageref,pagegen);
    n=fz_buffer_storage(ctx,strbuf,&p);
#ifdef HAVE_Z_LIB
    wfile_abstmpnam(tempfile);
    gz=gzopen(tempfile,"sab7");
    gzwrite(gz,p,n);
    gzclose(gz);
    nw=wfile_size(tempfile);
    fz_resize_buffer(ctx,strbuf,nw+1);
    fz_buffer_storage(ctx,strbuf,&p);
    f=fopen(tempfile,"rb");
    if (f==NULL || fread(p,1,nw,f)<nw)
        aprintf(ANSI_RED "** wmupdf: Error writing compressed stream to PDF file! **\n" ANSI_NORMAL);
    if (f!=NULL)
        fclose(f);
    remove(tempfile);
    p[nw]='\n';
    strbuf->len=nw+1;
    pdf_update_stream(xref,pageref,strbuf);
    fz_drop_buffer(ctx,strbuf);
/*
printf("    After drop, xref->table[%d].stm_buf=%p, refs=%d\n",pageref,xref->table[pageref].stm_buf,
                                                  xref->table[pageref].stm_buf->refs);
*/
    (*length)=nw;
    return(1);
#else
    fz_drop_buffer(ctx,strbuf);
    (*length)=n;
    return(0);
#endif
    }


static int add_to_srcpage_stream(pdf_document *xref,fz_context *ctx,int pageref,int pagegen,
                                 pdf_obj *srcdict)

    {
    fz_buffer *srcbuf;
    fz_buffer *dstbuf;
    int dstlen;

// printf("@add_to_srcpage_stream()...pageref=%d\n",pageref);
    srcbuf=pdf_load_stream(xref,pdf_to_num(srcdict),pdf_to_gen(srcdict));
    if (srcbuf==NULL)
        {
        dstbuf=pdf_load_stream(xref,pageref,pagegen);
        if (dstbuf==NULL)
            return(0);
        dstlen=fz_buffer_storage(ctx,dstbuf,NULL);
        fz_drop_buffer(ctx,dstbuf);
        return(dstlen);
        }
    if (!pdf_is_stream(xref,pageref,pagegen))
        dstbuf=fz_new_buffer(ctx,16);
    else
        {
        dstbuf=pdf_load_stream(xref,pageref,pagegen);
        if (dstbuf==NULL)
            dstbuf=fz_new_buffer(ctx,16);
        }
    /* Concatenate srcbuf to dstbuf:  (Will srcbuf->data be allowed?)  */
    dstlen=fz_buffer_storage(ctx,dstbuf,NULL);
/*
printf("    dstlen before = %d\n",dstlen);
printf("    srclen = %d\n",fz_buffer_storage(ctx,srcbuf,NULL));
printf("    srcptr = %p\n",srcbuf->data);
*/
    fz_write_buffer(ctx,dstbuf,srcbuf->data,fz_buffer_storage(ctx,srcbuf,NULL));
    dstlen=fz_buffer_storage(ctx,dstbuf,NULL);
// printf("    dstlen after = %d\n",dstlen);
    fz_drop_buffer(ctx,srcbuf);
    pdf_update_stream(xref,pageref,dstbuf);
    fz_drop_buffer(ctx,dstbuf);
    return(dstlen);
    }


static char *xobject_name(int pageno)

    {
    static char buf[32];

    sprintf(buf,"Xfk2p%d",pageno);
    return(buf);
    }


static pdf_obj *start_new_destpage(pdf_document *doc,double width_pts,double height_pts)

    {
    pdf_obj *pageobj;
    pdf_obj *mbox;

    pageobj=pdf_new_dict(doc,2);
    pdf_dict_puts(pageobj,"Type",pdf_new_name(doc,"Page"));
    mbox=pdf_new_array(doc,4);
    pdf_array_push(mbox,pdf_new_real(doc,0.));
    pdf_array_push(mbox,pdf_new_real(doc,0.));
    pdf_array_push(mbox,pdf_new_real(doc,width_pts));
    pdf_array_push(mbox,pdf_new_real(doc,height_pts));
    pdf_dict_puts(pageobj,"MediaBox",mbox);
    return(pageobj);
    }

/*
** From MuPDF pdfclean.c
*/
static void wmupdf_preserve_old_dests(pdf_obj *olddests,fz_context *ctx,pdf_document *xref,
                                      pdf_obj *pages)

    {
    int i;
    pdf_obj *names = pdf_new_dict(xref,1);
    pdf_obj *dests = pdf_new_dict(xref,1);
    pdf_obj *names_list = pdf_new_array(xref,32);
    int len = pdf_dict_len(olddests);
    pdf_obj *root;

    for (i=0;i<len;i++)
        {
        pdf_obj *key = pdf_dict_get_key(olddests,i);
        pdf_obj *val = pdf_dict_get_val(olddests,i);
        pdf_obj *key_str = pdf_new_string(xref,pdf_to_name(key),strlen(pdf_to_name(key)));
        pdf_obj *dest = pdf_dict_gets(val,"D");

        dest = pdf_array_get(dest ? dest : val, 0);
        if (pdf_array_contains(pdf_dict_gets(pages,"Kids"),dest))
            {
            pdf_array_push(names_list, key_str);
            pdf_array_push(names_list, val);
            }
        pdf_drop_obj(key_str);
        }

    root = pdf_dict_gets(pdf_trailer(xref),"Root");
    pdf_dict_puts(dests,"Names",names_list);
    pdf_dict_puts(names,"Dests",dests);
    pdf_dict_puts(root,"Names",names);

    pdf_drop_obj(names);
    pdf_drop_obj(dests);
    pdf_drop_obj(names_list);
    pdf_drop_obj(olddests);
    }


static int new_stream_object(pdf_document *xref,fz_context *ctx,char *buf)

    {
    int ref;
    pdf_obj *obj,*len;
    fz_buffer *fzbuf;

    ref = pdf_create_object(xref);
    obj = pdf_new_dict(xref,1);
    len=pdf_new_int(xref,strlen(buf));
    pdf_dict_puts(obj,"Length",len);
    pdf_drop_obj(len);
    pdf_update_object(xref,ref,obj);
    pdf_drop_obj(obj);
    fzbuf=fz_new_buffer(ctx,strlen(buf));
    fz_write_buffer(ctx,fzbuf,(unsigned char *)buf,strlen(buf));
    pdf_update_stream(xref,ref,fzbuf);
    fz_drop_buffer(ctx,fzbuf);
    return(ref);
    }


/*
** Merge srcdict into dstdict.
*/
static void wmupdf_dict_merge(fz_context *ctx,char *dictname,pdf_obj *dstdict,pdf_obj *srcdict)

    {
    int i,len;

/*
printf("    Merging %s dictionaries (%d <-- %d)\n",dictname,pdf_to_num(dstdict),pdf_to_num(srcdict));
*/
    len=pdf_dict_len(srcdict);
    for (i=0;i<len;i++)
        {
        pdf_obj *key,*value;

        key=pdf_dict_get_key(srcdict,i);
        value=pdf_dict_get_val(srcdict,i);
        wmupdf_dict_merge_keyval(ctx,dstdict,key,value);
        }
    }


/*
** If key doesn't exist, puts key,value pair.
** If key does exist, converts by merging (if dict) or adding (if array)
*/
static void wmupdf_dict_merge_keyval(fz_context *ctx,pdf_obj *dstdict,pdf_obj *key,pdf_obj *value)

    {
    pdf_obj *dstval;

    dstval=pdf_dict_get(dstdict,key);
    if (!dstval)
        {
        pdf_dict_put(dstdict,key,value);
        return;
        }
    /* Values are same--no action required */
    if (!pdf_objcmp(dstval,value))
        return;
    if (pdf_is_dict(dstval) && pdf_is_dict(value))
        {
        static char *okay_to_merge[] = {"Resources","XObject",""};
        int i;

        for (i=0;okay_to_merge[i][0]!='\0';i++)
            if (!stricmp(okay_to_merge[i],pdf_to_name(key)))
                break;
        if (okay_to_merge[i][0]!='\0')
            {
            /* Merge source dict into dest dict */
            wmupdf_dict_merge(ctx,pdf_to_name(key),dstval,value);
            pdf_dict_put(dstdict,key,dstval);
            }
        else
            /* Just overwrite dest dict with source dict */
            pdf_dict_put(dstdict,key,value);
        return;
        }
    /* This works for ProcSet array, but maybe not for any array (e.g. rectangle) */
    if (pdf_is_array(dstval) && pdf_is_array(value))
        {
        wmupdf_array_merge(ctx,pdf_to_name(key),dstval,value);
        return;
        }
    /* Last resort:  overwrite with new value */
    pdf_dict_put(dstdict,key,value);

    /* This does NOT work--you can't just convert the value into an array of values */
    /* PDF will become non-conformant. */
    /*
    array=pdf_new_array(ctx,2);
    pdf_array_push(array,dstval);
    pdf_array_push(array,value);
    pdf_dict_put(dstdict,key,array);
    pdf_drop_obj(array);
    */
    }


/*
** Merge items in src array into dst array (do not duplicate items).
*/
static void wmupdf_array_merge(fz_context *ctx,char *arrayname,pdf_obj *dstarray,pdf_obj *srcarray)

    {
    int i,len;

/*
printf("    Merging %s arrays:  %d <-- %d\n",arrayname,pdf_to_num(dstarray),pdf_to_num(srcarray));
*/
    len=pdf_array_len(srcarray);
    for (i=0;i<len;i++)
        if (!pdf_array_contains(dstarray,pdf_array_get(srcarray,i)))
            pdf_array_push(dstarray,pdf_array_get(srcarray,i));
    }


static void matrix_zero_round(double m[][3])

    {
    int r,c;
    for (r=0;r<3;r++)
        for (c=0;c<3;c++)
            if (fabs(m[r][c])<1e-5)
                m[r][c]=0.;
    }


static void matrix_unity(double m[][3],double val)

     {
     matrix_set_all(m,0.);
     m[0][0]=val;
     m[1][1]=val;
     m[2][2]=1.;
     }


static void matrix_set_all(double m[][3],double val)

    {
    int r,c;
    for (r=0;r<3;r++)
        for (c=0;c<3;c++)
            m[r][c]=val;
    }


static void matrix_translate(double m[][3],double x,double y)

    {
    matrix_unity(m,1.);
    m[2][0]=x;
    m[2][1]=y;
    }


static void matrix_mul(double dst[][3],double src[][3])

    {
    int r,c,i;
    double newdst[3][3];

    matrix_set_all(newdst,0.);
    for (r=0;r<3;r++)
        for (c=0;c<3;c++)
            for (i=0;i<3;i++)
                newdst[r][c] += dst[r][i]*src[i][c];
    memcpy(dst,newdst,sizeof(double)*9);
    }


static void matrix_rotate(double m[][3],double deg)

    {
    double th,costh,sinth;

    th=deg*PI/180.;
    costh=cos(th);
    sinth=sin(th);
    matrix_unity(m,1.);
    m[0][0]=costh;
    m[0][1]=sinth;
    m[1][0]=-sinth;
    m[1][1]=costh;
    }


static void matrix_xymul(double m[][3],double *x,double *y)

    {
    double x0,y0;

    x0=m[0][0]*(*x)+m[1][0]*(*y)+m[2][0];
    y0=m[0][1]*(*x)+m[1][1]*(*y)+m[2][1];
    (*x)=x0;
    (*y)=y0;
    }


/*
** CHARACTER POSITION MAPS
*/
int wtextchars_fill_from_page(WTEXTCHARS *wtc,char *filename,int pageno,char *password)

    {
    fz_document *doc=NULL;
    fz_display_list *list=NULL;
    fz_context *ctx;
    fz_text_sheet *textsheet=NULL;
    fz_page *page;
    fz_text_page *text=NULL;
    fz_device *dev=NULL;
    fz_rect bounds;

    fz_var(doc);
    ctx=fz_new_context(NULL,NULL,FZ_STORE_DEFAULT);
    if (ctx==NULL)
        return(-1);
    fz_set_aa_level(ctx,8);
    doc=fz_open_document(ctx,filename);
    if (doc==NULL)
        {
        fz_free_context(ctx);
        return(-2);
        }
    if (fz_needs_password(doc) && !fz_authenticate_password(doc,password))
        {
        fz_close_document(doc);
        fz_free_context(ctx);
        return(-3);
        }
    page=fz_load_page(doc,pageno-1);
    if (page==NULL)
        {
        fz_free_page(doc,page);
        fz_close_document(doc);
        fz_free_context(ctx);
        return(-3);
        }
    fz_try(ctx)
        {
        list=fz_new_display_list(ctx);
        dev=fz_new_list_device(ctx,list);
        fz_run_page(doc,page,dev,&fz_identity,NULL);
        }
    fz_always(ctx)
        {
        fz_free_device(dev);
        dev=NULL;
        }
    fz_catch(ctx)
        {
        fz_drop_display_list(ctx,list);
        fz_free_page(doc,page);
        fz_close_document(doc);
        fz_free_context(ctx);
        return(-4);
        }
    fz_var(text);
    fz_bound_page(doc,page,&bounds);
    wtc->width=fabs(bounds.x1-bounds.x0);
    wtc->height=fabs(bounds.y1-bounds.y0);
    textsheet=fz_new_text_sheet(ctx);
    fz_try(ctx)
        {
        text=fz_new_text_page(ctx);
        dev=fz_new_text_device(ctx,textsheet,text);
        if (list)
            fz_run_display_list(list,dev,&fz_identity,&fz_infinite_rect,NULL);
        else
            fz_run_page(doc,page,dev,&fz_identity,NULL);
        fz_free_device(dev);
        dev=NULL;
        wtextchars_add_fz_chars(wtc,ctx,text);
        }
    fz_always(ctx)
        {
        fz_free_device(dev);
        dev=NULL;
        fz_free_text_page(ctx,text);
        fz_free_text_sheet(ctx,textsheet);
        fz_drop_display_list(ctx,list);
        fz_free_page(doc,page);
        fz_close_document(doc);
        }
    fz_catch(ctx)
        {
        fz_free_context(ctx);
        return(-5);
        }
    fz_free_context(ctx);
    return(0);
    }


static void wtextchars_add_fz_chars(WTEXTCHARS *wtc,fz_context *ctx,fz_text_page *page)

    {
    int iblock;

    for (iblock=0;iblock<page->len;iblock++)
        {
        fz_text_block *block;
        fz_text_line *line;
        char *s;

        if (page->blocks[iblock].type != FZ_PAGE_BLOCK_TEXT)
            continue;
        block=page->blocks[iblock].u.text;
        for (line=block->lines;line<block->lines+block->len;line++)
            {
            fz_text_span *span;

            for (span=line->first_span;span;span=span->next)
                {
                fz_text_style *style=NULL;
                int char_num;

                for (char_num=0;char_num<span->len;char_num++)
                    {
                    fz_text_char *ch;
                    fz_rect rect;
                    WTEXTCHAR textchar;

                    ch=&span->text[char_num];
                    if (ch->style != style)
                        {
                        /* style change if style!=NULL */
                        style=ch->style;
                        s=strchr(style->font->name,'+');
                        s= s ? s+1 : style->font->name;
                        }
                    fz_text_char_bbox(&rect,span,char_num);
                    textchar.x1=rect.x0;
                    textchar.y1=rect.y0;
                    textchar.x2=rect.x1;
                    textchar.y2=rect.y1;
                    textchar.xp=ch->p.x;
                    textchar.yp=ch->p.y;
                    textchar.ucs=ch->c;
                    wtextchars_add_wtextchar(wtc,&textchar);
                    }
                }
            }
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
    utf8len=n==0 ? 0 : utf8_length(unicode,n);
    willus_mem_alloc_warn((void **)text,utf8len+1,funcname,10);
    t=(*text);
    unicode_to_utf8(t,unicode,n);
    willus_mem_free((double **)&unicode,funcname);
    }


static void wtextchars_get_chars_inside(WTEXTCHARS *src,WTEXTCHARS *dst,double x1,double y1,
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
        **     B. The center of the word box is insdie the char box.
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


static void wtextchars_sort_vertically_by_position(WTEXTCHARS *wtc,int type)

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
            if (child<n1 && wtextchar_compare_vert(&x[child],&x[child+1],type)<0)
                child++;
            if (wtextchar_compare_vert(&x0,&x[child],type)<0)
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


static int wtextchar_compare_vert(WTEXTCHAR *c1,WTEXTCHAR *c2,int index)

    {
    if (index==1)
        {
        if (c1->y1!=c2->y1)
            return(c1->y1-c2->y1);
        }
    else
        {
        if (c1->y2!=c2->y2)
            return(c1->y2-c2->y2);
        }
    return(c1->xp-c2->xp);
    }
#endif


static void wtextchars_sort_horizontally_by_position(WTEXTCHARS *wtc)

    {
    int top,n1,n;
    WTEXTCHAR x0,*x;

    if (wtc->sorted==2)
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
    wtc->sorted=2;
    }


static int wtextchar_compare_horiz(WTEXTCHAR *c1,WTEXTCHAR *c2)

    {
    return(c1->xp-c2->xp);
    }
#endif /* HAVE_MUPDF_LIB */
