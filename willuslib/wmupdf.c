/*
** wmupdf.c    Routines to interface w/mupdf lib (except for bmp functions, which
**             are in bmpmupdf.c).
**
** Part of willus.com general purpose C code library.
**
** Copyright (C) 2016  http://willus.com
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
#include <mupdf/pdf.h>
void pdf_install_load_system_font_funcs(fz_context *ctx);

static void info_update(fz_context *ctx,pdf_document *xref,char *producer,char *author,char *title);
static void dict_put_string(fz_context *ctx,pdf_document *doc,pdf_obj *dict,char *key,char *string);
static void wmupdf_object_bbox(fz_context *ctx,pdf_obj *srcpage,double *bbox_array,double *defbbox);
static int wmupdf_pdfdoc_newpages(pdf_document *xref,fz_context *ctx,WPDFPAGEINFO *pageinfo,
                                  int use_forms,WPDFOUTLINE *wpdfoutline,FILE *out);
static void set_clip_array(double *xclip,double *yclip,double rot_deg,double width,double height);
static void cat_pdf_double(char *buf,double x);
static void wmupdf_convert_pages_to_forms(pdf_document *xref,fz_context *ctx,int *srcpageused,
                                          double *defaultbbox);
static void wmupdf_convert_single_page_to_form(pdf_document *xref,fz_context *ctx,
                                               pdf_obj *srcpageref,int pageno,double *defaultbbox);
static int stream_deflate(pdf_document *xref,fz_context *ctx,int pageref,int pagegen,int *length);
static int add_to_srcpage_stream(pdf_document *xref,fz_context *ctx,int pageref,
                                 int pagegen,pdf_obj *dict);
static char *xobject_name(int pageno);
static pdf_obj *start_new_destpage(fz_context *ctx,pdf_document *doc,double width_pts,double height_pts);
static void wmupdf_preserve_old_dests(pdf_obj *olddests,fz_context *ctx,pdf_document *xref,
                                      pdf_obj *pages);
static int new_stream_object(pdf_document *xref,fz_context *ctx,char *buf);
static void wmupdf_update_stream(fz_context *ctx,pdf_document *doc,int num,fz_buffer *newbuf);
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
static void wtextchars_add_fz_chars(WTEXTCHARS *wtc,fz_context *ctx,fz_text_page *page,
                                    int boundingbox);
/*
** Outline functions
*/
static WPDFOUTLINE *wpdfoutline_convert_from_fitz_outline(fz_outline *fzoutline);
static void pdf_create_outline(fz_context *ctx,pdf_document *doc,pdf_obj *outline_root,pdf_obj *orref,WPDFOUTLINE *outline);
static void pdf_create_outline_1(fz_context *ctx,pdf_document *doc,pdf_obj *parent,pdf_obj *parentref,pdf_obj *dict,pdf_obj *dictref,int drefnum,WPDFOUTLINE *outline);
static pdf_obj *anchor_reference(fz_context *ctx,pdf_document *doc,int pageno);
static pdf_obj *pdf_new_string_utf8(fz_context *ctx,pdf_document *doc,char *string);


int wmupdf_numpages(char *filename)

    {
    fz_context *ctx;
    fz_document *doc;
    int np;

    doc=NULL;
    ctx = fz_new_context(NULL,NULL,FZ_STORE_DEFAULT);
    if (!ctx)
        return(-1);
    fz_try(ctx)
        {
        fz_register_document_handlers(ctx);
        doc=fz_open_document(ctx,filename);
        }
    fz_catch(ctx)
        {
        fz_drop_context(ctx);
        return(-2);
        }
    np=fz_count_pages(ctx,doc);
    fz_drop_document(ctx,doc);
    fz_flush_warnings(ctx);
    fz_drop_context(ctx);
    return(np);
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
        fz_register_document_handlers(ctx);
        xref=pdf_open_document(ctx,infile);
        if (!xref)
            {
            fz_drop_context(ctx);
            return(-2);
            }
        if (pdf_needs_password(ctx,xref) && !pdf_authenticate_password(ctx,xref,password))
            {
            pdf_close_document(ctx,xref);
            fz_drop_context(ctx);
            return(-3);
            }
        if (pdf_trailer(ctx,xref)!=NULL
            && (info=pdf_dict_gets(ctx,pdf_trailer(ctx,xref),"Info"))!=NULL
            && (obj=pdf_dict_gets(ctx,info,label))!=NULL
            && pdf_is_string(ctx,obj))
            {
            strncpy(buf,pdf_to_str_buf(ctx,obj),maxlen-1);
            buf[maxlen-1]='\0';
            }
        }
    fz_always(ctx)
        {
        pdf_close_document(ctx,xref);
        }
    fz_catch(ctx)
        {
        }
    fz_drop_context(ctx);
    return(0);
    }


/*
** Reconstruct PDF file per the information in pageinfo.
** use_forms==0:  Old-style reconstruction where the pages are not turned into XObject Forms.
** use_forms==1:  New-style where pages are turned into XObject forms.
*/
int wmupdf_remake_pdf(char *infile,char *outfile,WPDFPAGEINFO *pageinfo,int use_forms,
                      WPDFOUTLINE *wpdfoutline,WILLUSBITMAP *coverimage,FILE *out)

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
        fz_register_document_handlers(ctx);
        /* Sumatra version of MuPDF v1.4 -- use locally installed fonts */
        pdf_install_load_system_font_funcs(ctx);
        xref=pdf_open_document(ctx,infile);
        if (!xref)
            {
            fz_drop_context(ctx);
            nprintf(out,"wmupdf_remake_pdf:  Cannot open PDF file %s.\n",infile);
            return(-2);
            }
        if (pdf_needs_password(ctx,xref) && !pdf_authenticate_password(ctx,xref,password))
            {
            pdf_close_document(ctx,xref);
            fz_drop_context(ctx);
            nprintf(out,"wmupdf_remake_pdf:  Cannot authenticate PDF file %s.\n",infile);
            return(-3);
            }
        status=wmupdf_pdfdoc_newpages(xref,ctx,pageinfo,use_forms,wpdfoutline,out);
        if (status<0)
            {
            pdf_close_document(ctx,xref);
            fz_drop_context(ctx);
            nprintf(out,"wmupdf_remake_pdf:  Error re-paginating PDF file %s.\n",infile);
            return(status);
            }
        info_update(ctx,xref,pageinfo->producer,pageinfo->author,pageinfo->title);
        /* Write output */
        pdf_write_document(ctx,xref,outfile,&fzopts);
        }
    fz_always(ctx)
        {
        pdf_close_document(ctx,xref);
        }
    fz_catch(ctx)
        {
        write_failed=1;
        }
    fz_drop_context(ctx);
    if (write_failed)
        {
        nprintf(out,"wmupdf_remake_pdf:  Error writing output PDF file %s.\n",outfile);
        return(-10);
        }
    return(0);
    }


static void info_update(fz_context *ctx,pdf_document *xref,char *producer,char *author,char *title)

    {
    char moddate[64];
    time_t now;
    struct tm date;
    pdf_obj *info;
    int newinfo;

    if (pdf_trailer(ctx,xref)==NULL)
        return;
    time(&now);
    date=(*localtime(&now));
    sprintf(moddate,"D:%04d%02d%02d%02d%02d%02d%s",
           date.tm_year+1900,date.tm_mon+1,date.tm_mday,
           date.tm_hour,date.tm_min,date.tm_sec,
           wsys_utc_string());
    info=pdf_dict_gets(ctx,pdf_trailer(ctx,xref),"Info");
    /* v2.33:  if pdf_resolve_indirect is NULL, it means Info has a bad dictionary. */
    if (info==NULL || pdf_resolve_indirect(ctx,info)==NULL)
        {
        newinfo=1;
        info=pdf_new_dict(ctx,xref,2);
        }
    else
        newinfo=0;
    if (producer!=NULL && producer[0]!='\0')
        dict_put_string(ctx,xref,info,"Producer",producer);
    if (author!=NULL && author[0]!='\0')
        dict_put_string(ctx,xref,info,"Author",author);
    if (title!=NULL && title[0]!='\0')
        dict_put_string(ctx,xref,info,"Title",title);
    dict_put_string(ctx,xref,info,"ModDate",moddate);
    if (newinfo)
        {
        pdf_dict_puts(ctx,pdf_trailer(ctx,xref),"Info",info);
        pdf_drop_obj(ctx,info);
        }
    }


static void dict_put_string(fz_context *ctx,pdf_document *doc,pdf_obj *dict,char *key,char *string)

    {
    pdf_obj *value;

    value=pdf_new_string(ctx,doc,string,strlen(string));
    pdf_dict_puts(ctx,dict,key,value);
    pdf_drop_obj(ctx,value);
    }


/*
** Look at CropBox and MediaBox entries to determine visible page origin.
** Use bbox_def[] if no bbox found (and if not NULL)
*/
static void wmupdf_object_bbox(fz_context *ctx,pdf_obj *srcpage,double *bbox_array,double *bbox_def)

    {
    int i;

    bbox_array[0] = bbox_array[1] = -1e10;
    bbox_array[2] = bbox_array[3] = 1e10;
    for (i=0;i<2;i++)
        {
        static char *boxname[] = {"MediaBox","CropBox"};
        pdf_obj *box;

        box=pdf_dict_gets(ctx,srcpage,boxname[i]);
        if (box!=NULL)
            {
            int j;
            for (j=0;j<4;j++)
                {
                pdf_obj *obj;

                obj=pdf_array_get(ctx,box,j);
                if (obj!=NULL)
                    {
                    double x;
                    x=pdf_to_real(ctx,obj);
                    if ((j<2 && x>bbox_array[j]) || (j>=2 && x<bbox_array[j]))
                        bbox_array[j]=x;
                    }
                }
            }
        }
    if (bbox_array[0] < -9e9)
        bbox_array[0] = ((bbox_def!=NULL && bbox_def[0]>-9e9) ? bbox_def[0] : 0.);
    if (bbox_array[1] < -9e9)
        bbox_array[1] = ((bbox_def!=NULL && bbox_def[1]>-9e9) ? bbox_def[1] : 0.);
    if (bbox_array[2] > 9e9)
        bbox_array[2] = ((bbox_def!=NULL && bbox_def[2]<9e9) ? bbox_def[2] : 612.);
    if (bbox_array[3] > 9e9)
        bbox_array[3] = ((bbox_def!=NULL && bbox_def[3]<9e9) ? bbox_def[3] : 792.);
    }


static int wmupdf_pdfdoc_newpages(pdf_document *xref,fz_context *ctx,WPDFPAGEINFO *pageinfo,
                                  int use_forms,WPDFOUTLINE *wpdfoutline,FILE *out)

    {
    static char *funcname="wmupdf_pdfdoc_newpages";
    pdf_obj *root,*oldroot,*pages,*kids,*countobj,*parent,*olddests;
    pdf_obj *srcpageobj,*srcpagecontents;
    pdf_obj *destpageobj,*destpagecontents,*destpageresources;
    double srcx0,srcy0,defaultbbox[4];
    int qref,i,i0,pagecount,srccount,destpageref,nbb,numpages;
    int *srcpageused;
    char *bigbuf;
    double srcpagerot;

    /* Avoid compiler warning */
    destpageref = 0;
    destpageobj = NULL;
    srcx0=srcy0=0.;
    /* Keep only pages/type and (reduced) dest entries to avoid references to unretained pages */
    pagecount = pdf_count_pages(ctx,xref);
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
    oldroot = pdf_dict_gets(ctx,pdf_trailer(ctx,xref),"Root");
    /*
    ** pages points to /Pages object in PDF file.
    ** Has:  /Type /Pages, /Count <numpages>, /Kids [ obj obj obj obj ]
    */
    pages = pdf_dict_gets(ctx,oldroot,"Pages");
    wmupdf_object_bbox(ctx,pages,defaultbbox,NULL);
    olddests = pdf_load_name_tree(ctx,xref,pdf_dict_gets(ctx,pdf_dict_gets(ctx,oldroot,"Names"),"Dests"));

    /*
    ** Create new root object with only /Pages and /Type (and reduced dest entries)
    ** to avoid references to unretained pages.
    */
    root = pdf_new_dict(ctx,xref,4);
    pdf_dict_puts(ctx,root,"Type",pdf_dict_gets(ctx,oldroot,"Type"));
    pdf_dict_puts(ctx,root,"Pages",pages);
    pdf_update_object(ctx,xref,pdf_to_num(ctx,oldroot),root);
/*
    pdf_drop_obj(root);
*/

    /* Parent indirectly references the /Pages object in the file */
    /* (Each new page we create has to point to this.)            */
    parent = pdf_new_indirect(ctx, xref, pdf_to_num(ctx,pages), pdf_to_gen(ctx,pages));
    /* Create a new kids array with only the pages we want to keep */
    kids = pdf_new_array(ctx, xref, 1);


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
printf("    ADDING NEW PAGE. (srccount=%d, use_forms=%d)\n",srccount,use_forms);
*/
            if (use_forms)
                {
                pdf_obj *dest_stream;

                /* Create new object in document for destination page stream */
                dest_stream = pdf_new_indirect(ctx,xref,new_stream_object(xref,ctx,bigbuf),0);
                /* Store this into the destination page contents array */
                pdf_array_push(ctx,destpagecontents,dest_stream);
                pdf_drop_obj(ctx,dest_stream);
                }
            newpageref=pdf_new_indirect(ctx,xref,destpageref,0);
            /* Reference parent list of pages */
            pdf_dict_puts(ctx,destpageobj,"Parent",parent);
            pdf_dict_puts(ctx,destpageobj,"Contents",destpagecontents);
            pdf_dict_puts(ctx,destpageobj,"Resources",destpageresources);
            /* Store page object in document's kids array */
            pdf_array_push(ctx,kids,newpageref);
            /* Update document with new page */
            pdf_update_object(ctx,xref,destpageref,destpageobj);
            /* Clean up */
            pdf_drop_obj(ctx,newpageref);
            pdf_drop_obj(ctx,destpageresources);
            pdf_drop_obj(ctx,destpagecontents);
            pdf_drop_obj(ctx,destpageobj);
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
                destpageobj=start_new_destpage(ctx,xref,box->dst_width_pts,box->dst_height_pts);
                destpageresources=pdf_new_dict(ctx,xref,1);
                if (use_forms)
                    pdf_dict_puts(ctx,destpageresources,"XObject",pdf_new_dict(ctx,xref,1));
                destpageref=pdf_create_object(ctx,xref);
                destpagecontents=pdf_new_array(ctx,xref,1);
                /* Init the destination page stream for forms */
                if (use_forms)
                    bigbuf[0]='\0';
                }
            /* New source page, so get the source page objects */
            /* srcpageobj = xref->page_objs[box->srcbox.pageno-1]; */
            /* pageno, or pageno-1?? */
            srcpageobj = pdf_resolve_indirect(ctx,pdf_lookup_page_obj(ctx,xref,box->srcbox.pageno-1));
            wmupdf_object_bbox(ctx,srcpageobj,v,defaultbbox);
            srcx0=v[0];
            srcy0=v[1];
/*
printf("SRCX0=%g, SRCY0=%g\n",srcx0,srcy0);
*/
            rotobj=pdf_dict_gets(ctx,srcpageobj,"Rotate");
            srcpagerot = rotobj!=NULL ? pdf_to_real(ctx,rotobj) : 0.;
/*
printf("Page rotation = %g\n",srcpagerot);
*/
            srcpagecontents=pdf_dict_gets(ctx,srcpageobj,"Contents");
/*
if (pdf_is_array(ctx,srcpagecontents))
{
int k;
printf("    source page contents = array.\n");
for (k=0;k<pdf_array_len(ctx,srcpagecontents);k++)
{
pdf_obj *obj;
obj=pdf_array_get(ctx,srcpagecontents,k);
if (pdf_is_indirect(ctx,obj))
{
printf("    contents[%d] = indirect (%d)\n",k,pdf_to_num(ctx,obj));
pdf_resolve_indirect(ctx,obj);
}
}
}
*/
            if (use_forms)
                {
                pdf_obj *xobjdict,*pageref;
                int pageno;

                xobjdict=pdf_dict_gets(ctx,destpageresources,"XObject");
                pageno=box->srcbox.pageno;
                pageref=pdf_lookup_page_obj(ctx,xref,pageno-1);
                pdf_dict_puts(ctx,xobjdict,xobject_name(pageno),pageref);
                pdf_dict_puts(ctx,destpageresources,"XObject",xobjdict);
                }
            else
                {
                pdf_obj *srcpageresources;

                /* Merge source page resources into destination page resources */
                srcpageresources=pdf_dict_gets(ctx,srcpageobj,"Resources");
/*
printf("box->dstpage=%d, srcpage=%d (ind.#=%d)\n",box->dstpage,box->srcbox.pageno,pdf_to_num(ctx,xref->page_refs[box->srcbox.pageno-1]));
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
            s1indirect = pdf_new_indirect(ctx,xref,new_stream_object(xref,ctx,buf),0);
            if (qref==0)
                qref=new_stream_object(xref,ctx,"Q\n");
            qindirect = pdf_new_indirect(ctx,xref,qref,0);
            /* Store this region into the destination page contents array */
            pdf_array_push(ctx,destpagecontents,s1indirect);
            if (pdf_is_array(ctx,srcpagecontents))
                {
                int k;
                for (k=0;k<pdf_array_len(ctx,srcpagecontents);k++)
                    pdf_array_push(ctx,destpagecontents,pdf_array_get(ctx,srcpagecontents,k));
                }
            else
                pdf_array_push(ctx,destpagecontents,srcpagecontents);
            pdf_array_push(ctx,destpagecontents,qindirect);
            pdf_drop_obj(ctx,s1indirect);
            pdf_drop_obj(ctx,qindirect);
            }
        }
    pdf_drop_obj(ctx,parent);

    /* For forms, convert all original source pages to XObject Forms */
    if (use_forms)
        wmupdf_convert_pages_to_forms(xref,ctx,srcpageused,defaultbbox);

    /* Update page count and kids array */
    numpages = pdf_array_len(ctx,kids);
    countobj = pdf_new_int(ctx,xref, numpages);
    pdf_dict_puts(ctx,pages, "Count", countobj);
    pdf_drop_obj(ctx,countobj);
    pdf_dict_puts(ctx,pages, "Kids", kids);
    pdf_drop_obj(ctx,kids);

    /* Also preserve the (partial) Dests name tree */
    if (olddests)
        wmupdf_preserve_old_dests(olddests,ctx,xref,pages);
    if (use_forms)
        {
        /* Free memory */
        willus_mem_free((double **)&bigbuf,funcname);
        willus_mem_free((double **)&srcpageused,funcname);
        }

    /* Outline */
    if (wpdfoutline!=NULL)
        {
        pdf_obj *outline_root,*ori;
        int ref;

        wpdfoutline_fill_in_blank_dstpages(wpdfoutline,numpages);
/* wpdfoutline_echo(wpdfoutline,0,1,stdout); */
        ref = pdf_create_object(ctx,xref);
        outline_root = pdf_new_dict(ctx,xref,4);
        ori = pdf_new_indirect(ctx,xref,ref,0);
        pdf_create_outline(ctx,xref,outline_root,ori,wpdfoutline);
        pdf_update_object(ctx,xref,ref,outline_root);
        pdf_drop_obj(ctx,outline_root);
        pdf_dict_puts(ctx,root,"Outlines",ori);
        pdf_drop_obj(ctx,ori);
        }
    pdf_drop_obj(ctx,root);
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


static void wmupdf_convert_pages_to_forms(pdf_document *xref,fz_context *ctx,int *srcpageused,
                                          double *defaultbbox)

    {
    int i,pagecount;
    pdf_obj **srcpage;
    static char *funcname="wmupdf_convert_pages_to_forms";

    pagecount = pdf_count_pages(ctx,xref);
    willus_mem_alloc_warn((void **)&srcpage,sizeof(pdf_obj *)*pagecount,funcname,10);
    /*
    ** Lookup all page references before we change them to XObjects, because
    ** after they are changed to XObjects, pdf_lookup_page_obj() fails.
    */
    for (i=1;i<=pagecount;i++)
        if (srcpageused[i])
            srcpage[i-1] = pdf_lookup_page_obj(ctx,xref,i-1);
    for (i=1;i<=pagecount;i++)
        if (srcpageused[i])
            wmupdf_convert_single_page_to_form(xref,ctx,srcpage[i-1],i,defaultbbox);
    willus_mem_free((double **)&srcpage,funcname);
    }


static void wmupdf_convert_single_page_to_form(pdf_document *xref,fz_context *ctx,
                                               pdf_obj *srcpageref,int pageno,double *defaultbbox)

    {
    pdf_obj *array,*srcpageobj,*srcpagecontents;
    int i,len,streamlen,pageref,pagegen,compressed;
    double bbox_array[4];
    double matrix[6];

    srcpageobj = pdf_resolve_indirect(ctx,srcpageref);
    pageref=pdf_to_num(ctx,srcpageref);
    pagegen=pdf_to_gen(ctx,srcpageref);
    wmupdf_object_bbox(ctx,srcpageobj,bbox_array,defaultbbox);
    for (i=0;i<6;i++)
        matrix[i]=0.;
    matrix[0]=matrix[3]=1.;
    srcpagecontents=pdf_dict_gets(ctx,srcpageobj,"Contents");
    /* Concatenate all indirect streams from source page directly into it. */
/* printf("Adding streams to source page %d (pageref=%d, pagegen=%d)...\n",pageno,pageref,pagegen); */
    streamlen=0;
    /* k2pdfopt v2.10:  check if NULL--can be NULL on empty page */
    if (srcpagecontents!=NULL)
        {
        if (pdf_is_array(ctx,srcpagecontents))
            {
            int k;
            for (k=0;k<pdf_array_len(ctx,srcpagecontents);k++)
                {
                pdf_obj *obj;
                obj=pdf_array_get(ctx,srcpagecontents,k);
                if (pdf_is_indirect(ctx,obj))
                    pdf_resolve_indirect(ctx,obj);
                streamlen=add_to_srcpage_stream(xref,ctx,pageref,pagegen,obj);
                }
            }
        else
            {
            if (pdf_is_indirect(ctx,srcpagecontents))
                pdf_resolve_indirect(ctx,srcpagecontents);
            streamlen=add_to_srcpage_stream(xref,ctx,pageref,pagegen,srcpagecontents);
            }
        compressed=stream_deflate(xref,ctx,pageref,pagegen,&streamlen);
        }
    else
        compressed=0;
    len=pdf_dict_len(ctx,srcpageobj);
    for (i=0;i<len;i++)
        {
        pdf_obj *key; /* *value */

        key=pdf_dict_get_key(ctx,srcpageobj,i);
        /* value=pdf_dict_get_val(srcpageobj,i); */
        /* Keep same resources */
        if (!pdf_is_name(ctx,key))
            continue;
        if (pdf_is_name(ctx,key) && !stricmp("Resources",pdf_to_name(ctx,key)))
            continue;
        /* Drop dictionary entry otherwise */
        pdf_dict_del(ctx,srcpageobj,key);
        i=-1;
        len=pdf_dict_len(ctx,srcpageobj);
        }
    /*
    ** Once we turn the object into an XObject type (and not a Page type)
    ** it can no longer be looked up using pdf_lookup_page_obj() as of MuPDF v1.3
    */
    pdf_dict_puts(ctx,srcpageobj,"Type",pdf_new_name(ctx,xref,"XObject"));
    pdf_dict_puts(ctx,srcpageobj,"Subtype",pdf_new_name(ctx,xref,"Form"));
    pdf_dict_puts(ctx,srcpageobj,"FormType",pdf_new_int(ctx,xref,1));
    if (compressed)
        pdf_dict_puts(ctx,srcpageobj,"Filter",pdf_new_name(ctx,xref,"FlateDecode"));
    pdf_dict_puts(ctx,srcpageobj,"Length",pdf_new_int(ctx,xref,streamlen));
    array=pdf_new_array(ctx,xref,4);
    for (i=0;i<4;i++)
        pdf_array_push(ctx,array,pdf_new_real(ctx,xref,bbox_array[i]));
    pdf_dict_puts(ctx,srcpageobj,"BBox",array);
    array=pdf_new_array(ctx,xref,6);
    for (i=0;i<6;i++)
        pdf_array_push(ctx,array,pdf_new_real(ctx,xref,matrix[i]));
    pdf_dict_puts(ctx,srcpageobj,"Matrix",array);
    }


static int stream_deflate(pdf_document *xref,fz_context *ctx,int pageref,int pagegen,int *length)

    {
    fz_buffer *strbuf;
    int n;
    unsigned char *p;
    static char *errmsg = ANSI_RED "** wmupdf: Error writing compressed stream to PDF file! **\n" ANSI_NORMAL;
#ifdef HAVE_Z_LIB
    char tempfile[512];
    compress_handle h;
    FILE *f;
    int nw;
#endif

    strbuf=pdf_load_stream(ctx,xref,pageref,pagegen);
    n=fz_buffer_storage(ctx,strbuf,&p);
#ifdef HAVE_Z_LIB
    /*
    ** To do:  write directly to fz_buffer, or don't compress the buffer but
    **         instead use compression options for fz_write_document.
    */
    wfile_abstmpnam(tempfile);
    f=wfile_fopen_utf8(tempfile,"wb");
    if (f==NULL)
        aprintf("%s",errmsg);
    else
        {
        h=compress_start(f,7);
        compress_write(f,h,p,n);
        compress_done(f,&h);
        fclose(f);
        }
    nw=wfile_size(tempfile);
    fz_resize_buffer(ctx,strbuf,nw+1);
    fz_buffer_storage(ctx,strbuf,&p);
    f=wfile_fopen_utf8(tempfile,"rb");
    if (f==NULL || fread(p,1,nw,f)<nw)
        aprintf("%s",errmsg);
    if (f!=NULL)
        fclose(f);
    wfile_remove_utf8(tempfile);
    p[nw]='\n';
    strbuf->len=nw+1;
    wmupdf_update_stream(ctx,xref,pageref,strbuf);
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

/*
** To do:  Can we use fz_buffer_cat for this?
*/
static int add_to_srcpage_stream(pdf_document *xref,fz_context *ctx,int pageref,
                                 int pagegen,pdf_obj *srcdict)

    {
    fz_buffer *srcbuf;
    fz_buffer *dstbuf;
    int dstlen;

/*
printf("@add_to_srcpage_stream()...pageref=%d\n",pageref);
printf("srcdict=%p\n",srcdict);
printf("pdf_to_num(ctx,srcdict)=%d\n",pdf_to_num(ctx,srcdict));
*/
    srcbuf=pdf_load_stream(ctx,xref,pdf_to_num(ctx,srcdict),pdf_to_gen(ctx,srcdict));
    if (srcbuf==NULL)
        {
        dstbuf=pdf_load_stream(ctx,xref,pageref,pagegen);
        if (dstbuf==NULL)
            return(0);
        dstlen=fz_buffer_storage(ctx,dstbuf,NULL);
        fz_drop_buffer(ctx,dstbuf);
        return(dstlen);
        }
    if (!pdf_is_stream(ctx,xref,pageref,pagegen))
        dstbuf=fz_new_buffer(ctx,16);
    else
        {
        dstbuf=pdf_load_stream(ctx,xref,pageref,pagegen);
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
    /*
    ** v2.04 fix:  Insert white space between consecutive streams.
    **             Bug found by agelos100 on mobileread.
    */
    if (dstlen>0)
        {
        char whitespace[2];

        whitespace[0]=' ';
        whitespace[1]='\0';
        fz_write_buffer(ctx,dstbuf,whitespace,1);
        }
    fz_write_buffer(ctx,dstbuf,srcbuf->data,fz_buffer_storage(ctx,srcbuf,NULL));
    dstlen=fz_buffer_storage(ctx,dstbuf,NULL);
/*
printf("    dstlen after = %d\n",dstlen);
*/
    fz_drop_buffer(ctx,srcbuf);
    wmupdf_update_stream(ctx,xref,pageref,dstbuf);
    fz_drop_buffer(ctx,dstbuf);
    return(dstlen);
    }


static char *xobject_name(int pageno)

    {
    static char buf[32];

    sprintf(buf,"Xfk2p%d",pageno);
    return(buf);
    }


static pdf_obj *start_new_destpage(fz_context *ctx,pdf_document *doc,double width_pts,double height_pts)

    {
    pdf_obj *pageobj;
    pdf_obj *mbox;

    pageobj=pdf_new_dict(ctx,doc,2);
    pdf_dict_puts(ctx,pageobj,"Type",pdf_new_name(ctx,doc,"Page"));
    mbox=pdf_new_array(ctx,doc,4);
    pdf_array_push(ctx,mbox,pdf_new_real(ctx,doc,0.));
    pdf_array_push(ctx,mbox,pdf_new_real(ctx,doc,0.));
    pdf_array_push(ctx,mbox,pdf_new_real(ctx,doc,width_pts));
    pdf_array_push(ctx,mbox,pdf_new_real(ctx,doc,height_pts));
    pdf_dict_puts(ctx,pageobj,"MediaBox",mbox);
    return(pageobj);
    }

/*
** From MuPDF pdfclean.c
*/
static void wmupdf_preserve_old_dests(pdf_obj *olddests,fz_context *ctx,pdf_document *xref,
                                      pdf_obj *pages)

    {
    int i;
    pdf_obj *names = pdf_new_dict(ctx,xref,1);
    pdf_obj *dests = pdf_new_dict(ctx,xref,1);
    pdf_obj *names_list = pdf_new_array(ctx,xref,32);
    int len = pdf_dict_len(ctx,olddests);
    pdf_obj *root;

    for (i=0;i<len;i++)
        {
        pdf_obj *key = pdf_dict_get_key(ctx,olddests,i);
        pdf_obj *val = pdf_dict_get_val(ctx,olddests,i);
        pdf_obj *key_str = pdf_new_string(ctx,xref,pdf_to_name(ctx,key),strlen(pdf_to_name(ctx,key)));
        pdf_obj *dest = pdf_dict_gets(ctx,val,"D");

        dest = pdf_array_get(ctx,dest ? dest : val, 0);
        if (pdf_array_contains(ctx,pdf_dict_gets(ctx,pages,"Kids"),dest))
            {
            pdf_array_push(ctx,names_list, key_str);
            pdf_array_push(ctx,names_list, val);
            }
        pdf_drop_obj(ctx,key_str);
        }

    root = pdf_dict_gets(ctx,pdf_trailer(ctx,xref),"Root");
    pdf_dict_puts(ctx,dests,"Names",names_list);
    pdf_dict_puts(ctx,names,"Dests",dests);
    pdf_dict_puts(ctx,root,"Names",names);

    pdf_drop_obj(ctx,names);
    pdf_drop_obj(ctx,dests);
    pdf_drop_obj(ctx,names_list);
    pdf_drop_obj(ctx,olddests);
    }


static int new_stream_object(pdf_document *xref,fz_context *ctx,char *buf)

    {
    int ref;
    pdf_obj *obj,*len;
    fz_buffer *fzbuf;

    ref = pdf_create_object(ctx,xref);
    obj = pdf_new_dict(ctx,xref,1);
    len=pdf_new_int(ctx,xref,strlen(buf));
    pdf_dict_puts(ctx,obj,"Length",len);
    pdf_drop_obj(ctx,len);
    pdf_update_object(ctx,xref,ref,obj);
    pdf_drop_obj(ctx,obj);
    fzbuf=fz_new_buffer(ctx,strlen(buf));
    fz_write_buffer(ctx,fzbuf,(unsigned char *)buf,strlen(buf));
    wmupdf_update_stream(ctx,xref,ref,fzbuf);
    fz_drop_buffer(ctx,fzbuf);
    return(ref);
    }


static void wmupdf_update_stream(fz_context *ctx,pdf_document *doc,int num,fz_buffer *newbuf)

    {
    pdf_xref_entry *x;
    pdf_obj *obj;

    if (num<=0 || num>=pdf_xref_len(ctx,doc))
        {
        fz_warn(ctx,"object out of range (%d 0 R); xref size %d",num,pdf_xref_len(ctx,doc));
        return;
        }
    x=pdf_get_xref_entry(ctx,doc,num);
    fz_drop_buffer(ctx,x->stm_buf);
    x->stm_buf = fz_keep_buffer(ctx,newbuf);
    obj = pdf_load_object(ctx,doc,num,0);
    if (obj!=NULL)
        {
        pdf_dict_puts_drop(ctx,obj,"Length",pdf_new_int(ctx,doc,newbuf->len));
        /*
        if (!compressed)
            {
            pdf_dict_dels(ctx,obj,"Filter");
            pdf_dict_dels(ctx,obj,"DecodeParms");
            }
        */
        pdf_drop_obj(ctx,obj);
        }
    }


/*
** Merge srcdict into dstdict.
*/
static void wmupdf_dict_merge(fz_context *ctx,char *dictname,pdf_obj *dstdict,pdf_obj *srcdict)

    {
    int i,len;

/*
printf("    Merging %s dictionaries (%d <-- %d)\n",dictname,pdf_to_num(ctx,dstdict),pdf_to_num(ctx,srcdict));
*/
    len=pdf_dict_len(ctx,srcdict);
    for (i=0;i<len;i++)
        {
        pdf_obj *key,*value;

        key=pdf_dict_get_key(ctx,srcdict,i);
        value=pdf_dict_get_val(ctx,srcdict,i);
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

    dstval=pdf_dict_get(ctx,dstdict,key);
    if (!dstval)
        {
        pdf_dict_put(ctx,dstdict,key,value);
        return;
        }
    /* Values are same--no action required */
    if (!pdf_objcmp(ctx,dstval,value))
        return;
    if (pdf_is_dict(ctx,dstval) && pdf_is_dict(ctx,value))
        {
        static char *okay_to_merge[] = {"Resources","XObject",""};
        int i;

        for (i=0;okay_to_merge[i][0]!='\0';i++)
            if (!stricmp(okay_to_merge[i],pdf_to_name(ctx,key)))
                break;
        if (okay_to_merge[i][0]!='\0')
            {
            /* Merge source dict into dest dict */
            wmupdf_dict_merge(ctx,pdf_to_name(ctx,key),dstval,value);
            pdf_dict_put(ctx,dstdict,key,dstval);
            }
        else
            /* Just overwrite dest dict with source dict */
            pdf_dict_put(ctx,dstdict,key,value);
        return;
        }
    /* This works for ProcSet array, but maybe not for any array (e.g. rectangle) */
    if (pdf_is_array(ctx,dstval) && pdf_is_array(ctx,value))
        {
        wmupdf_array_merge(ctx,pdf_to_name(ctx,key),dstval,value);
        return;
        }
    /* Last resort:  overwrite with new value */
    pdf_dict_put(ctx,dstdict,key,value);

    /* This does NOT work--you can't just convert the value into an array of values */
    /* PDF will become non-conformant. */
    /*
    array=pdf_new_array(ctx,2);
    pdf_array_push(ctx,array,dstval);
    pdf_array_push(ctx,array,value);
    pdf_dict_put(ctx,dstdict,key,array);
    pdf_drop_obj(ctx,array);
    */
    }


/*
** Merge items in src array into dst array (do not duplicate items).
*/
static void wmupdf_array_merge(fz_context *ctx,char *arrayname,pdf_obj *dstarray,pdf_obj *srcarray)

    {
    int i,len;

/*
printf("    Merging %s arrays:  %d <-- %d\n",arrayname,pdf_to_num(ctx,dstarray),pdf_to_num(ctx,srcarray));
*/
    len=pdf_array_len(ctx,srcarray);
    for (i=0;i<len;i++)
        if (!pdf_array_contains(ctx,dstarray,pdf_array_get(ctx,srcarray,i)))
            pdf_array_push(ctx,dstarray,pdf_array_get(ctx,srcarray,i));
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


int wtextchars_fill_from_page(WTEXTCHARS *wtc,char *filename,int pageno,char *password)

    {
    return(wtextchars_fill_from_page_ex(wtc,filename,pageno,password,0));
    }


/*
** CHARACTER POSITION MAPS
**
** if boundingbox==1, only one character is returned, and its upper-left and lower-right
** corner are the bounding box of all text on the page.
**
*/
int wtextchars_fill_from_page_ex(WTEXTCHARS *wtc,char *filename,int pageno,char *password,
                                 int boundingbox)

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
    fz_try(ctx)
        {
        fz_register_document_handlers(ctx);
        fz_set_aa_level(ctx,8);
        /* Sumatra version of MuPDF v1.4 -- use locally installed fonts */
        pdf_install_load_system_font_funcs(ctx);
        doc=fz_open_document(ctx,filename);
        if (doc==NULL)
            {
            fz_drop_context(ctx);
            return(-2);
            }
        if (fz_needs_password(ctx,doc) && !fz_authenticate_password(ctx,doc,password))
            {
            fz_drop_document(ctx,doc);
            fz_drop_context(ctx);
            return(-3);
            }
        page=fz_load_page(ctx,doc,pageno-1);
        if (page==NULL)
            {
            fz_drop_page(ctx,page);
            fz_drop_document(ctx,doc);
            fz_drop_context(ctx);
            return(-3);
            }
        fz_try(ctx)
            {
            list=fz_new_display_list(ctx);
            dev=fz_new_list_device(ctx,list);
            fz_run_page(ctx,page,dev,&fz_identity,NULL);
            }
        fz_always(ctx)
            {
            fz_drop_device(ctx,dev);
            dev=NULL;
            }
        fz_catch(ctx)
            {
            fz_drop_display_list(ctx,list);
            fz_drop_page(ctx,page);
            fz_drop_document(ctx,doc);
            fz_drop_context(ctx);
            return(-4);
            }
        fz_var(text);
        fz_bound_page(ctx,page,&bounds);
        wtc->width=fabs(bounds.x1-bounds.x0);
        wtc->height=fabs(bounds.y1-bounds.y0);
        textsheet=fz_new_text_sheet(ctx);
        fz_try(ctx)
            {
            text=fz_new_text_page(ctx);
            dev=fz_new_text_device(ctx,textsheet,text);
            if (list)
                fz_run_display_list(ctx,list,dev,&fz_identity,&fz_infinite_rect,NULL);
            else
                fz_run_page(ctx,page,dev,&fz_identity,NULL);
            fz_drop_device(ctx,dev);
            dev=NULL;
            wtextchars_add_fz_chars(wtc,ctx,text,boundingbox);
            }
        fz_always(ctx)
            {
            fz_drop_device(ctx,dev);
            dev=NULL;
            fz_drop_text_page(ctx,text);
            fz_drop_text_sheet(ctx,textsheet);
            fz_drop_display_list(ctx,list);
            fz_drop_page(ctx,page);
            fz_drop_document(ctx,doc);
            }
        fz_catch(ctx)
            {
            fz_drop_context(ctx);
            return(-5);
            }
        }
    fz_catch(ctx)
        {
        fz_drop_context(ctx);
        return(-20);
        }
    fz_drop_context(ctx);
    return(0);
    }


static void wtextchars_add_fz_chars(WTEXTCHARS *wtc,fz_context *ctx,fz_text_page *page,
                                    int boundingbox)

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
/*
printf("Span:\n");
printf("    len=%d, cap=%d\n",span->len,span->cap);
printf("    min=(%d,%d)\n",(int)span->min.x,(int)span->min.y);
printf("    max=(%d,%d)\n",(int)span->max.x,(int)span->max.y);
printf("    wmode=%d\n",span->wmode);
printf("    asmax=%g, dsmin=%g\n",span->ascender_max,span->descender_min);
printf("    bbox=(%g,%g) - (%g,%g)\n",span->bbox.x0,span->bbox.y0,span->bbox.x1,span->bbox.y1);
printf("    baseoff=%g\n",span->base_offset);
printf("    spacing=%g\n",span->spacing);
printf("    column=%d\n",span->column);
printf("    colwidth=%g\n",span->column_width);
printf("    align=%d\n",span->align);
printf("    indent=%g\n",span->indent);
*/
                for (char_num=0;char_num<span->len;char_num++)
                    {
                    fz_text_char *ch;
                    fz_rect rect;
                    double dx,dy;
                    WTEXTCHAR textchar;

                    ch=&span->text[char_num];
                    if (ch->style != style)
                        {
                        /* style change if style!=NULL */
                        style=ch->style;
                        s=strchr(style->font->name,'+');
                        s= s ? s+1 : style->font->name;
                        }
                    fz_text_char_bbox(ctx,&rect,span,char_num);
                    textchar.x1=rect.x0;
                    textchar.y1=rect.y0;
                    textchar.x2=rect.x1;
                    textchar.y2=rect.y1;
                    textchar.xp=ch->p.x;
                    textchar.yp=ch->p.y;
                    textchar.ucs=ch->c;
                    /*
                    ** Strange behavior in one particular PDF (modul1.pdf) file lead to this...
                    ** MuPDF bugzilla #695362:
                    ** "Incorrect structured-text character bounding boxes and character values"
                    ** Filed 13 July 2014
                    */
                    dx=textchar.x2-textchar.x1;
                    if (fabs(dx)>3000.)
                        {
                        if (fabs(textchar.x1-textchar.xp) < fabs(textchar.x2-textchar.xp))
                            textchar.x2 = textchar.x1 + dx/1000.;
                        else
                            textchar.x1 = textchar.x2 - dx/1000.;
                        }
                    dy=textchar.y2-textchar.y1;
                    if (fabs(dy)>3000.)
                        {
                        if (fabs(textchar.y1-textchar.yp) < fabs(textchar.y2-textchar.yp))
                            textchar.y2 = textchar.y1 + dy/1000.;
                        else
                            textchar.y1 = textchar.y2 - dy/1000.;
                        }
/*
printf("Char %4d: (%7.1f,%7.1f) - (%7.1f,%7.1f) (%7.1f,%7.1f)\n",
ch->c,textchar.x1,textchar.y1,textchar.x2,textchar.y2,textchar.xp,textchar.yp);
*/
                    if (boundingbox==0 || wtc->n<=0)
                        wtextchars_add_wtextchar(wtc,&textchar);
                    else
                        {
                        WTEXTCHAR *tc0;
                        tc0 = &wtc->wtextchar[0];
                        if (textchar.x1 < tc0->x1)
                            tc0->x1 = textchar.x1;
                        if (textchar.x2 > tc0->x2)
                            tc0->x2 = textchar.x2;
                        if (textchar.y1 < tc0->y1)
                            tc0->y1 = textchar.y1;
                        if (textchar.y2 > tc0->y2)
                            tc0->y2 = textchar.y2;
                        }
                    }
                }
            }
        }
    }


/*
** Get outline of PDF file into WPDFOUTLINE structure
*/
WPDFOUTLINE *wpdfoutline_read_from_pdf_file(char *filename)

    {
    fz_context *ctx;
    fz_document *doc;
    fz_outline *fzoutline;
    WPDFOUTLINE *wpdfoutline;

    wpdfoutline=NULL;
    doc=NULL;
    ctx = fz_new_context(NULL,NULL,FZ_STORE_DEFAULT);
    if (!ctx)
        return(NULL);
    fz_try(ctx)
        {
        fz_register_document_handlers(ctx);
        fz_set_aa_level(ctx,8);
        /* Sumatra version of MuPDF v1.4 -- use locally installed fonts */
        pdf_install_load_system_font_funcs(ctx);
        fz_try(ctx) { doc=fz_open_document(ctx,filename); }
        fz_catch(ctx) 
            { 
            fz_drop_context(ctx);
            return(NULL);
            }
        fzoutline=fz_load_outline(ctx,doc);
        wpdfoutline=wpdfoutline_convert_from_fitz_outline(fzoutline);
        if (fzoutline!=NULL)
            fz_drop_outline(ctx,fzoutline);
        fz_drop_document(ctx,doc);
        }
    fz_catch(ctx)
        {
        fz_drop_context(ctx);
        return(NULL);
        }
    fz_drop_context(ctx);
    return(wpdfoutline);
    }


/*
** Convert fz_outline structure to WPDFOUTLINE structure
*/
static WPDFOUTLINE *wpdfoutline_convert_from_fitz_outline(fz_outline *fzoutline)

    {
    static char *funcname="wpdfoutline_convert_from_fitz_outline";
    WPDFOUTLINE *x;
    void *p;

    if (fzoutline==NULL)
        return(NULL);
    willus_mem_alloc_warn(&p,sizeof(WPDFOUTLINE),funcname,10);
    x=(WPDFOUTLINE *)p;
    wpdfoutline_init(x);
    if (fzoutline->title!=NULL)
        {
        willus_mem_alloc_warn(&p,strlen(fzoutline->title)+1,funcname,10);
        x->title=p;
        strcpy(x->title,fzoutline->title);
        }
    if (fzoutline->dest.kind==FZ_LINK_GOTO || fzoutline->dest.kind==FZ_LINK_GOTOR)
        x->srcpage=fzoutline->dest.ld.gotor.page;
    else
        x->srcpage=-1;
    x->dstpage=-1;
    x->next = wpdfoutline_convert_from_fitz_outline(fzoutline->next);
    x->down = wpdfoutline_convert_from_fitz_outline(fzoutline->down);
    return(x);
    }


/*
** Save outline stored in "outline" to outline_root object.
*/
static void pdf_create_outline(fz_context *ctx,pdf_document *doc,pdf_obj *outline_root,pdf_obj *orref,WPDFOUTLINE *outline)

    {
    int ref;
    pdf_obj *first,*firstref;

    ref = pdf_create_object(ctx,doc);
    first = pdf_new_dict(ctx,doc,4);
    firstref = pdf_new_indirect(ctx,doc,ref,0);
    pdf_create_outline_1(ctx,doc,outline_root,orref,first,firstref,ref,outline);
    }

/*
** Recursive
*/
static void pdf_create_outline_1(fz_context *ctx,pdf_document *doc,pdf_obj *parent,pdf_obj *parentref,pdf_obj *dict,pdf_obj *dictref,int drefnum,WPDFOUTLINE *outline)

    {
    int count;
    pdf_obj *first,*prev;

    prev=NULL;
    first=dictref;
    count=0;
    while (outline)
        {
        pdf_obj *title,*nextdict,*nextdictref,*aref;
        int nextdictrefnum;

        title=pdf_new_string_utf8(ctx,doc,outline->title);
        pdf_dict_puts(ctx,dict,"Title",title);
        pdf_drop_obj(ctx,title);
        aref=anchor_reference(ctx,doc,outline->dstpage);
        pdf_dict_puts(ctx,dict,"A",aref);
        pdf_drop_obj(ctx,aref);
        count++;
        if (parentref!=NULL)
           pdf_dict_puts(ctx,dict,"Parent",parentref);
        if (prev!=NULL)
           pdf_dict_puts(ctx,dict,"Prev",prev);
        if (outline->down)
            {
            pdf_obj *newdict,*newdictref;
            int newdictrefnum;

            newdictrefnum = pdf_create_object(ctx,doc);
            newdict = pdf_new_dict(ctx,doc,4);
            newdictref = pdf_new_indirect(ctx,doc,newdictrefnum,0);
            pdf_create_outline_1(ctx,doc,dict,dictref,newdict,newdictref,newdictrefnum,outline->down);
            }
        pdf_update_object(ctx,doc,drefnum,dict);
        if (outline->next==NULL)
            break;
        nextdictrefnum = pdf_create_object(ctx,doc);
        nextdict = pdf_new_dict(ctx,doc,4);
        nextdictref = pdf_new_indirect(ctx,doc,nextdictrefnum,0);
        pdf_dict_puts(ctx,dict,"Next",nextdictref);
        if (dictref!=first)
            {
            pdf_drop_obj(ctx,dictref);
            pdf_drop_obj(ctx,dict);
            }
        prev=dictref;
        dictref=nextdictref;
        dict=nextdict;
        drefnum=nextdictrefnum;
        outline = outline->next;
        }
    pdf_dict_puts(ctx,parent,"First",first);
    pdf_dict_puts(ctx,parent,"Last",dictref);
    {
    pdf_obj *countobj;

    countobj=pdf_new_int(ctx,doc,count);
    pdf_dict_puts(ctx,parent,"Count",countobj);
    pdf_drop_obj(ctx,countobj);
    }
    pdf_update_object(ctx,doc,drefnum,dict);
    pdf_drop_obj(ctx,dict);
    pdf_drop_obj(ctx,dictref);
    }


static pdf_obj *anchor_reference(fz_context *ctx,pdf_document *doc,int pageno)

    {
    pdf_obj *pageref;
    pdf_obj *anchor,*anchorref;
    pdf_obj *array;
    pdf_obj *name;
    int arefnum;

    pageref=pdf_lookup_page_obj(ctx,doc,pageno);
    arefnum = pdf_create_object(ctx,doc);
    anchor = pdf_new_dict(ctx,doc,4);
    anchorref = pdf_new_indirect(ctx,doc,arefnum,0);
    array = pdf_new_array(ctx,doc,2);
    pdf_array_push(ctx,array,pageref);
    name = pdf_new_name(ctx,doc,"Fit");
    pdf_array_push(ctx,array,name);
    pdf_drop_obj(ctx,name);
    pdf_dict_puts(ctx,anchor,"D",array);
    pdf_drop_obj(ctx,array);
    name = pdf_new_name(ctx,doc,"GoTo");
    pdf_dict_puts(ctx,anchor,"S",name);
    pdf_drop_obj(ctx,name);
    pdf_update_object(ctx,doc,arefnum,anchor);
    pdf_drop_obj(ctx,anchor);
    return(anchorref);
    }


static pdf_obj *pdf_new_string_utf8(fz_context *ctx,pdf_document *doc,char *string)

    {
    int *utf16;
    char *utfbuf;
    int i,j,n;
    static char *funcname="pdf_new_string_utf8";
    pdf_obj *pdfobj;

    n=strlen(string)+2;
    willus_mem_alloc_warn((void **)&utf16,sizeof(int)*n,funcname,10);
    n=utf8_to_unicode(utf16,string,n-1);
    willus_mem_alloc_warn((void **)&utfbuf,n*2+3,funcname,10);
    j=0;
    utfbuf[j++]=0xfe;
    utfbuf[j++]=0xff;
    for (i=0;i<n;i++)
        {
        utfbuf[j++]=(utf16[i]>>8)&0xff;
        utfbuf[j++]=(utf16[i]&0xff);
        }
    utfbuf[j]='\0';
    willus_mem_free((double **)&utf16,funcname);
    pdfobj=pdf_new_string(ctx,doc,utfbuf,j);
    willus_mem_free((double **)&utfbuf,funcname);
    return(pdfobj);
    }
    
#endif /* HAVE_MUPDF_LIB */
