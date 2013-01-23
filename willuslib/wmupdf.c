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
static void info_update(fz_context *ctx,pdf_document *xref,char *producer);
static void dict_put_string(fz_context *ctx,pdf_obj *dict,char *key,char *string);
static void wmupdf_page_bbox(pdf_obj *srcpage,double *bbox_array);
static int wmupdf_pdfdoc_newpages(pdf_document *xref,fz_context *ctx,WPDFPAGEINFO *pageinfo,
                                  int use_forms,FILE *out);
static void wmupdf_convert_pages_to_forms(pdf_document *xref,fz_context *ctx,int *srcpageused);
static void wmupdf_convert_single_page_to_form(pdf_document *xref,fz_context *ctx,int pageno);
static int stream_deflate(pdf_document *xref,fz_context *ctx,int pageref,int pagegen,
                          int *length);
static int add_to_srcpage_stream(pdf_document *xref,fz_context *ctx,int pageref,int pagegen,
                                 pdf_obj *dict);
static char *xobject_name(int pageno);
static pdf_obj *start_new_destpage(fz_context *ctx,double width_pts,double height_pts);
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
        box->srcrot_deg -= 90.;
        }
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
        if (xref->trailer!=NULL
            && (info=pdf_dict_gets(xref->trailer,"Info"))!=NULL
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

    fzopts.do_garbage=1; /* 2 and 3 don't work for this. */
    fzopts.do_expand=0;
    fzopts.do_ascii=0;
    fzopts.do_linear=0;
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
        info_update(ctx,xref,pageinfo->producer);
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


static void info_update(fz_context *ctx,pdf_document *xref,char *producer)

    {
    char moddate[64];
    time_t now;
    struct tm date;
    pdf_obj *info;
    int newinfo;

    if (xref->trailer==NULL)
        return;
    time(&now);
    date=(*localtime(&now));
    sprintf(moddate,"D:%04d%02d%02d%02d%02d%02d%s",
           date.tm_year+1900,date.tm_mon+1,date.tm_mday,
           date.tm_hour,date.tm_min,date.tm_sec,
           wsys_utc_string());
    info=pdf_dict_gets(xref->trailer,"Info");
    if (info==NULL)
        {
        newinfo=1;
        info=pdf_new_dict(ctx,2);
        }
    else
        newinfo=0;
    dict_put_string(ctx,info,"Producer",producer);
    dict_put_string(ctx,info,"ModDate",moddate);
    if (newinfo)
        {
        pdf_dict_puts(xref->trailer,"Info",info);
        pdf_drop_obj(info);
        }
    }


static void dict_put_string(fz_context *ctx,pdf_obj *dict,char *key,char *string)

    {
    pdf_obj *value;

    value=pdf_new_string(ctx,string,strlen(string));
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
	oldroot = pdf_dict_gets(xref->trailer,"Root");
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
	root = pdf_new_dict(ctx,4);
	pdf_dict_puts(root,"Type",pdf_dict_gets(oldroot,"Type"));
	pdf_dict_puts(root,"Pages",pages);
	pdf_update_object(xref,pdf_to_num(oldroot),root);
  	pdf_drop_obj(root);

    /* Parent indirectly references the /Pages object in the file */
    /* (Each new page we create has to point to this.)            */
	parent = pdf_new_indirect(ctx, pdf_to_num(pages), pdf_to_gen(pages), xref);
	/* Create a new kids array with only the pages we want to keep */
	kids = pdf_new_array(ctx, 1);


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
        char buf[512];
        pdf_obj *s1indirect,*qindirect,*rotobj;
        double cpm[3][3],m[3][3],m1[3][3];
        double xclip[4],yclip[4];

/*
printf("box[%d]\n",i);
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
                dest_stream = pdf_new_indirect(ctx,new_stream_object(xref,ctx,bigbuf),
                                               0,(void *)xref);
                /* Store this into the destination page contents array */
                pdf_array_push(destpagecontents,dest_stream);
                pdf_drop_obj(dest_stream);
                }
            newpageref=pdf_new_indirect(ctx,destpageref,0,(void *)xref);
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
            if (pageinfo->boxes.box[j].srcbox.pageno==box->srcbox.pageno)
                {
                newsrc=0;
                break;
                }
        if (newsrc)
            {
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
                destpageobj=start_new_destpage(ctx,box->dst_width_pts,box->dst_height_pts);
                destpageresources=pdf_new_dict(ctx,1);
                if (use_forms)
                    pdf_dict_puts(destpageresources,"XObject",pdf_new_dict(ctx,1));
                destpageref=pdf_create_object(xref);
                destpagecontents=pdf_new_array(ctx,1);
                /* Init the destination page stream for forms */
                if (use_forms)
                    bigbuf[0]='\0';
                }
            /* New source page, so get the source page objects */
    		srcpageobj = xref->page_objs[box->srcbox.pageno-1];
            {
            double v[4];
            wmupdf_page_bbox(srcpageobj,v);
            srcx0=v[0];
            srcy0=v[1];
            }
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
                pdf_obj *xobjdict;
                int pageno;

                xobjdict=pdf_dict_gets(destpageresources,"XObject");
                pageno=box->srcbox.pageno;
	            pdf_dict_puts(xobjdict,xobject_name(pageno),xref->page_refs[pageno-1]);
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
        if (fabs(srcpagerot)>1e-4)
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
        {
        double w,h;
        double drot;
        int nrot;
        drot=fmod(box->srcrot_deg,360.);
        if (drot < 0.)
            drot += 360.;
        nrot=(int)((drot+45.)/90.);
        if (nrot&1)
            {
            w=box->h;
            h=box->w;
            }
        else
            {
            w=box->w;
            h=box->h;
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
        for (k=0;k<4;k++)
            matrix_xymul(cpm,&xclip[k],&yclip[k]);
/*
printf("Clip path:\n    %7.2f %7.2f\n    %7.2f,%7.2f\n    %7.2f,%7.2f\n"
                   "    %7.2f %7.2f\n    %7.2f,%7.2f\n",
                xclip[0],yclip[0],xclip[1],yclip[1],xclip[2],yclip[2],
                xclip[3],yclip[3],xclip[0],yclip[0]);
*/
        sprintf(buf,"q %.5g %.5g %.5g %.5g %.5g %.5g cm %.5g %.5g m %.5g %.5g l %.5g %.5g l %.5g %.5g l %.5g %.5g l W n",
                 m[0][0],m[0][1],m[1][0],m[1][1],m[2][0],m[2][1],
                 xclip[0],yclip[0],xclip[1],yclip[1],xclip[2],yclip[2],xclip[3],yclip[3],
                 xclip[0],yclip[0]);
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
            s1indirect = pdf_new_indirect(ctx,new_stream_object(xref,ctx,buf),0,(void *)xref);
            if (qref==0)
                qref=new_stream_object(xref,ctx,"Q\n");
            qindirect = pdf_new_indirect(ctx,qref,0,(void *)xref);
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
	countobj = pdf_new_int(ctx, pdf_array_len(kids));
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


static void wmupdf_convert_pages_to_forms(pdf_document *xref,fz_context *ctx,int *srcpageused)

    {
    int i,pagecount;

    pagecount = pdf_count_pages(xref);
    for (i=0;i<=pagecount;i++)
        if (srcpageused[i])
            wmupdf_convert_single_page_to_form(xref,ctx,i);
    }


static void wmupdf_convert_single_page_to_form(pdf_document *xref,fz_context *ctx,int pageno)

    {
    pdf_obj *array,*srcpageobj,*srcpagecontents;
    int i,len,streamlen,pageref,pagegen,compressed;
    double bbox_array[4];
    double matrix[6];

    /* New source page, so get the source page objects */
    srcpageobj = xref->page_objs[pageno-1];
    pageref=pdf_to_num(xref->page_refs[pageno-1]);
    pagegen=pdf_to_gen(xref->page_refs[pageno-1]);
    wmupdf_page_bbox(srcpageobj,bbox_array);
    for (i=0;i<6;i++)
        matrix[i]=0.;
    matrix[0]=matrix[3]=1.;
    srcpagecontents=pdf_dict_gets(srcpageobj,"Contents");
    /* Concatenate all indirect streams from source page directly into it. */
// printf("Adding streams to source page %d (pageref=%d, pagegen=%d)...\n",pageno,pageref,pagegen);
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
    srcpageobj = xref->page_objs[pageno-1];
    pageref=pdf_to_num(xref->page_refs[pageno-1]);
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
// printf("Deleting key %s.\n",pdf_to_name(key));
        pdf_dict_del(srcpageobj,key);
        i=-1;
        len=pdf_dict_len(srcpageobj);
        }
	pdf_dict_puts(srcpageobj,"Type",fz_new_name(ctx,"XObject"));
	pdf_dict_puts(srcpageobj,"Subtype",fz_new_name(ctx,"Form"));
	pdf_dict_puts(srcpageobj,"FormType",pdf_new_int(ctx,1));
    if (compressed)
        pdf_dict_puts(srcpageobj,"Filter",fz_new_name(ctx,"FlateDecode"));
 	pdf_dict_puts(srcpageobj,"Length",pdf_new_int(ctx,streamlen));
    array=pdf_new_array(ctx,4);
    for (i=0;i<4;i++)
        pdf_array_push(array,pdf_new_real(ctx,bbox_array[i]));
    pdf_dict_puts(srcpageobj,"BBox",array);
    array=pdf_new_array(ctx,6);
    for (i=0;i<6;i++)
        pdf_array_push(array,pdf_new_real(ctx,matrix[i]));
    pdf_dict_puts(srcpageobj,"Matrix",array);
    /* (It's no longer a "page"--it's a Form-type XObject) */
    /* I don't think this call should be made since it will call fz_drop_object on srcpageobj */
    /* pdf_update_object(xref,pageref,srcpageobj); */
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


static pdf_obj *start_new_destpage(fz_context *ctx,double width_pts,double height_pts)

    {
    pdf_obj *pageobj;
    pdf_obj *mbox;

    pageobj=pdf_new_dict(ctx,2);
	pdf_dict_puts(pageobj,"Type",fz_new_name(ctx,"Page"));
    mbox=pdf_new_array(ctx,4);
    pdf_array_push(mbox,pdf_new_real(ctx,0.));
    pdf_array_push(mbox,pdf_new_real(ctx,0.));
    pdf_array_push(mbox,pdf_new_real(ctx,width_pts));
    pdf_array_push(mbox,pdf_new_real(ctx,height_pts));
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
    pdf_obj *names = pdf_new_dict(ctx,1);
    pdf_obj *dests = pdf_new_dict(ctx,1);
    pdf_obj *names_list = pdf_new_array(ctx,32);
    int len = pdf_dict_len(olddests);
    pdf_obj *root;

    for (i=0;i<len;i++)
        {
        pdf_obj *key = pdf_dict_get_key(olddests,i);
        pdf_obj *val = pdf_dict_get_val(olddests,i);
        pdf_obj *key_str = pdf_new_string(ctx,pdf_to_name(key),strlen(pdf_to_name(key)));
        pdf_obj *dest = pdf_dict_gets(val,"D");

        dest = pdf_array_get(dest ? dest : val, 0);
        if (pdf_array_contains(pdf_dict_gets(pages,"Kids"),dest))
            {
            pdf_array_push(names_list, key_str);
            pdf_array_push(names_list, val);
            }
        pdf_drop_obj(key_str);
        }

    root = pdf_dict_gets(xref->trailer,"Root");
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
    obj = pdf_new_dict(ctx,1);
    len=pdf_new_int(ctx,strlen(buf));
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
#endif /* HAVE_MUPDF_LIB */
