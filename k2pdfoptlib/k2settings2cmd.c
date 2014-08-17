/*
** k2settings2cmd.c    Convert changes in settings structure to equivalent
**                     command-line arguments.
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

#ifdef HAVE_K2GUI
static void k2settings_to_cmd(STRBUF *cmdline,K2PDFOPT_SETTINGS *dst,
                              K2PDFOPT_SETTINGS *src,STRBUF *nongui);
static void minus_check(STRBUF *cmdline,STRBUF *nongui,char *optname,int *srcval,int dstval);
static void minus_inverse(STRBUF *cmdline,STRBUF *nongui,char *optname,int *srcval,int dstval);
static void plus_minus_check(STRBUF *cmdline,STRBUF *nongui,char *optname,int *srcval,int dstval);
static void integer_check(STRBUF *cmdline,STRBUF *nongui,char *optname,int *srcval,int dstval);
static void double_check(STRBUF *cmdline,STRBUF *nongui,char *optname,double *srcval,double dstval);
static void string_check(STRBUF *cmdline,STRBUF *nongui,char *optname,char *srcval,char *dstval);
static char *unit_string(int units);
static void cropbox_check(STRBUF *cmdline,STRBUF *nongui,char *opt,K2CROPBOX *src,K2CROPBOX *dst);
static void notesets_check(STRBUF *cmdline,STRBUF *nongui,K2NOTESET *src,K2NOTESET *dst);
static void cropboxes_check(STRBUF *cmdline,STRBUF *nongui,K2CROPBOXES *src,K2CROPBOXES *dst);
static int notesets_are_different(K2NOTESET *src,K2NOTESET *dst);
static int cropboxes_are_different(K2CROPBOXES *src,K2CROPBOXES *dst);
static int cropbox_differ(K2CROPBOX *src,K2CROPBOX *dst);
static int notes_differ(K2NOTES *src,K2NOTES *dst);
/*
static void margins_doublecheck(STRBUF *cmdline,char *opt,double *sleft,double dleft,
                                double *stop,double dtop,
                                double *sright,double dright,
                                double *sbottom,double dbottom);
*/
static void margins_integercheck(STRBUF *cmdline,STRBUF *nongui,char *opt,int *sleft,int dleft,
                                int *stop,int dtop,
                                int *sright,int dright,
                                int *sbottom,int dbottom);

/*
** Fills cmdline with the appropriate command-line options that will
** change the settings in "src" to the settings in "dst".
**
** "src" will not be modified.
**
** If nongui is not NULL, only command-line options which cannot be set via
** the GUI controls are put into the nongui buffer.
*/
void k2pdfopt_settings_get_cmdline(STRBUF *cmdline,K2PDFOPT_SETTINGS *dst,
                                   K2PDFOPT_SETTINGS *src,STRBUF *nongui)

    {
    STRBUF *shortest,_shortest;
    STRBUF *shortestng,_shortestng;
    K2PDFOPT_SETTINGS _src0,*src0;
    static char *modelabel[]={"def","fw","fp","crop","2col","tm","copy",""};
    int i,j,nd;
#if (WILLUSDEBUGX & 0x80000)
{
FILE *xx;
static int count=0;
xx=fopen("slog.txt",count==0?"w":"a");
count++;
fprintf(xx,"@kdpfopt_settings_get_cmdline()\n");
fprintf(xx,"    src->dst_ocr=%d, dst->dst_ocr=%d\n",src->dst_ocr,dst->dst_ocr);
fprintf(xx,"    dst->srccropmargins->box[0]=%g\n",dst->srccropmargins.box[0]);
#endif
    /*
    ** Try all "-mode" options and choose the shortest result
    */
    shortest=&_shortest;
    shortestng=&_shortestng;
    strbuf_init(shortest);
    strbuf_init(shortestng);
    src0=&_src0;

    /* Try with no mode and no device specified */
    k2pdfopt_settings_copy(src0,src);
    strbuf_clear(cmdline);
    if (nongui!=NULL)
        strbuf_clear(nongui);
    k2settings_to_cmd(cmdline,dst,src0,nongui);
#if (WILLUSDEBUGX & 0x80000)
fprintf(xx,"TRY 1.  cmdline='%s'\n",cmdline->s);
if (nongui!=NULL)
fprintf(xx,"        nongui='%s'\n",nongui->s);
#endif
    if ((cmdline->s==NULL || cmdline->s[0]=='\0') && (nongui==NULL || nongui->s==NULL || nongui->s[0]=='\0'))
        return;
    strbuf_cpy(shortest,cmdline->s);
    if (nongui==NULL)
        strbuf_clear(shortestng);
    else
        strbuf_cpy(shortestng,nongui->s);
    /* Try different modes and devices and pick the shortest command-line length */
    nd=devprofiles_count();
    for (j=-1;j<nd;j++)
        {
        for (i=-1;i<0 || modelabel[i][0]!='\0';i++)
            {
            int lenc,lens,leng,lensg;

            k2pdfopt_settings_copy(src0,src);
            strbuf_clear(cmdline);
            if (nongui!=NULL)
                strbuf_clear(nongui);
            if (j>=0)
                k2settings_sprintf(cmdline,src0,"-dev %s",devprofile_alias(j));
            if (i>=0)
                k2settings_sprintf(cmdline,src0,"-mode %s",modelabel[i]);
            k2settings_to_cmd(cmdline,dst,src0,nongui);
#if (WILLUSDEBUGX & 0x80000)
fprintf(xx,"TRY j=%d, i=%d.  cmdline='%s', nongui='%s'\n",i,j,cmdline->s,nongui==NULL?"":nongui->s);
#endif
            if ((cmdline->s==NULL || cmdline->s[0]=='\0') && (nongui==NULL || nongui->s==NULL || nongui->s[0]=='\0'))
                return;
            lenc=cmdline->s==NULL ? 0 : strlen(cmdline->s);
            lens=shortest->s==NULL ? 0 : strlen(shortest->s);
            if (nongui==NULL)
                {
                if (lenc < lens)
                    strbuf_cpy(shortest,cmdline->s);
                continue;
                }
            leng=nongui->s==NULL ? 0 : strlen(nongui->s);
            lensg=shortestng->s==NULL ? 0 : strlen(shortestng->s);
            /* first, pick shortest extra commands, then gui commands */
            if (leng < lensg || (leng==lensg && lenc < lens))
                {
#if (WILLUSDEBUGX & 0x80000)
fprintf(xx,"\n\n ** IMPROVED **\n\n");
#endif
                strbuf_cpy(shortestng,nongui->s);
                strbuf_cpy(shortest,cmdline->s);
                continue;
                }
            }
        }
#if (WILLUSDEBUGX & 0x80000)
fprintf(xx,"Done. shortestng->s='%s'\n",shortestng->s);
fclose(xx);
}
#endif
    if (shortest->s==NULL)
        strbuf_clear(cmdline);
    else
        strbuf_cpy(cmdline,shortest->s);
    if (nongui!=NULL)
        {
        if (shortestng->s==NULL)
            strbuf_clear(nongui);
        else
            strbuf_cpy(nongui,shortestng->s);
        }
    }
    

/*
** Apply printf-style result to k2settings as if it were on the
** command-line.
*/
int k2settings_sprintf(STRBUF *cmdline,K2PDFOPT_SETTINGS *k2settings,char *fmt,...)

    {
    static char *funcname="k2settings_sprintf";
    va_list args;
    int status;
    char *buf;
    STRBUF _cmd1,*cmd1;
    K2PDFOPT_CONVERSION *k2conv;

    cmd1=&_cmd1;
    strbuf_init(cmd1);
    va_start(args,fmt);
    willus_dmem_alloc_warn(32,(void **)&buf,2048,funcname,10);
    willus_dmem_alloc_warn(33,(void **)&k2conv,sizeof(K2PDFOPT_CONVERSION),funcname,10);
    k2pdfopt_conversion_init(k2conv);
    status=vsprintf(buf,fmt,args);
    va_end(args);
    strbuf_cat(cmd1,buf);
    if (cmdline!=NULL)
        strbuf_cat(cmdline,buf);
    willus_dmem_free(32,(double **)&buf,funcname);
    k2pdfopt_settings_copy(&k2conv->k2settings,k2settings);
/*
printf("    @k2settings_sprintf, cmd1='%s'\n",cmd1->s);
printf("    k2settings->text_wrap=%d\n",k2settings->text_wrap);
printf("    k2settings->native=%d\n",k2settings->use_crop_boxes);
printf("    k2settings->vbthresh=%g\n",(double)k2settings->vertical_break_threshold);
*/
    parse_cmd_args(k2conv,cmd1,NULL,NULL,1,1);
    k2pdfopt_settings_copy(k2settings,&k2conv->k2settings);
/*
printf("    AFTER:\n");
printf("        k2settings->text_wrap=%d\n",k2settings->text_wrap);
printf("        k2settings->native=%d\n",k2settings->use_crop_boxes);
printf("        k2settings->vbthresh=%g\n",(double)k2settings->vertical_break_threshold);
*/
    k2pdfopt_conversion_close(k2conv);
    willus_dmem_free(33,(double **)&k2conv,funcname);
    strbuf_free(cmd1);
    return(status);
    }


/*
** Create the command line that changes "src" to "dst" not using any -mode options.
*/
static void k2settings_to_cmd(STRBUF *cmdline,K2PDFOPT_SETTINGS *dst,
                              K2PDFOPT_SETTINGS *src,STRBUF *nongui)


    {
    /* Don't clear cmdline--it may be passed with some args already. */
#ifdef HAVE_K2GUI
    /* Re-launch code */
    /* v2.20--these lines commented out */
    /*
    plus_minus_check(cmdline,"-gui",&src->gui,dst->gui);
    minus_check(cmdline,"-guimin",&src->guimin,dst->guimin);
    */
#endif
    minus_check(cmdline,nongui,"-?",&src->show_usage,dst->show_usage);
    /*
    if (!stricmp(cl->cmdarg,"-a") || !stricmp(cl->cmdarg,"-a-"))
        {
        if (setvals>=1)
            ansi_set(cl->cmdarg[2]=='-' ? 0 : 1);
        continue;
        }
    */
    minus_check(cmdline,nongui,"-x",&src->exit_on_complete,dst->exit_on_complete);
    if (src->query_user_explicit!=dst->query_user_explicit
            && dst->query_user_explicit)
        {
        strbuf_dsprintf(cmdline,nongui,"-ui%s",dst->query_user?"":"-");
        src->query_user_explicit=dst->query_user_explicit;
        src->query_user=dst->query_user;
        }
    else
        minus_check(cmdline,nongui,"-ui",&src->query_user,dst->query_user);
    if (dst->erase_vertical_lines==2)
    integer_check(cmdline,dst->erase_vertical_lines==2?nongui:NULL,"-evl",
                  &src->erase_vertical_lines,dst->erase_vertical_lines);    
    double_check(cmdline,nongui,"-vls",&src->vertical_line_spacing,dst->vertical_line_spacing);
    double_check(cmdline,nongui,"-vm",&src->vertical_multiplier,dst->vertical_multiplier);
    double_check(cmdline,nongui,"-vs",&src->max_vertical_gap_inches,dst->max_vertical_gap_inches);
    if (fabs(dst->defect_size_pts-0.75)<.001 || fabs(dst->defect_size_pts-1.5)<.001)
        double_check(cmdline,NULL,"-de",&src->defect_size_pts,dst->defect_size_pts);
    else
        double_check(cmdline,nongui,"-de",&src->defect_size_pts,dst->defect_size_pts);
    /*
    if (!stricmp(cl->cmdarg,"-dev"))
        {
        if (cmdlineinput_next(cl)==NULL)
            break;
        if (setvals==2 && !strcmp(cl->cmdarg,"?"))
            {
            devprofiles_echo(stdout);
            k2sys_exit(k2settings,0);
            }
        if (setvals==1)
            {
            if (!k2pdfopt_settings_set_to_device(k2settings,devprofile_get(cl->cmdarg)))
                k2printf(TTEXT_WARN "\aDevice profile '%s' not known." TTEXT_NORMAL "\n",cl->cmdarg);
            }
        continue;
        }
    */
    /*
    minus_check(cmdline,"-pi",&src->preserve_indentation,dst->preserve_indentation);
    */
    plus_minus_check(cmdline,NULL,"-wrap",&src->text_wrap,dst->text_wrap);
#ifdef HAVE_MUPDF_LIB
    if (src->user_usegs != dst->user_usegs)
        {
        strbuf_dsprintf(cmdline,nongui,"-gs%s",dst->user_usegs==-1?"--":(dst->user_usegs==0)?"-":"");
        src->user_usegs = dst->user_usegs;
        }
#ifdef HAVE_OCR_LIB
    if (!dst->use_crop_boxes && src->use_crop_boxes && src->dst_ocr==0)
        src->dst_ocr='m';
#endif
    if (dst->use_crop_boxes && !src->use_crop_boxes)
        {
#ifdef HAVE_OCR_LIB
        src->dst_ocr=0;
#endif
        src->text_wrap=0;
        }
    minus_check(cmdline,NULL,"-n",&src->use_crop_boxes,dst->use_crop_boxes);
#endif /* HAVE_MUPDF_LIB */
    minus_check(cmdline,nongui,"-to",&src->text_only,dst->text_only);
    minus_check(cmdline,nongui,"-neg",&src->dst_negative,dst->dst_negative);
    minus_check(cmdline,nongui,"-sp",&src->echo_source_page_count,dst->echo_source_page_count);
    minus_inverse(cmdline,NULL,"-r",&src->src_left_to_right,dst->src_left_to_right);
    minus_check(cmdline,nongui,"-hy",&src->hyphen_detect,dst->hyphen_detect);
    minus_check(cmdline,NULL,"-ls",&src->dst_landscape,dst->dst_landscape);
    string_check(cmdline,nongui,"-o",src->dst_opname_format,dst->dst_opname_format);
#ifdef HAVE_OCR_LIB
    string_check(cmdline,nongui,"-ocrout",src->ocrout,dst->ocrout);
#endif
    if (dst->overwrite_minsize_mb != src->overwrite_minsize_mb)
        {
        if (dst->overwrite_minsize_mb<0.)
            strbuf_dsprintf(cmdline,nongui,"-ow");
        else if (dst->overwrite_minsize_mb==0.)
            strbuf_dsprintf(cmdline,nongui,"-ow-");
        else
            strbuf_dsprintf(cmdline,nongui,"-ow %g",dst->overwrite_minsize_mb);
        src->overwrite_minsize_mb = dst->overwrite_minsize_mb;
        }
    if ((dst->src_grid_cols!=src->src_grid_cols)
         || (dst->src_grid_rows!=src->src_grid_rows)
         || (dst->src_grid_overlap_percentage!=src->src_grid_overlap_percentage)
         || (dst->src_grid_order!=src->src_grid_order))
        {
        strbuf_dsprintf(cmdline,nongui,"-grid %dx%dx%d%s",
                       dst->src_grid_cols,dst->src_grid_rows,
                       dst->src_grid_overlap_percentage,
                       dst->src_grid_order ? "+" : "");
        src->src_grid_cols = dst->src_grid_cols;
        src->src_grid_rows = dst->src_grid_rows;
        src->src_grid_overlap_percentage = dst->src_grid_overlap_percentage;
        src->src_grid_order = dst->src_grid_order;
        }
    integer_check(cmdline,nongui,"-f2p",&src->dst_fit_to_page,dst->dst_fit_to_page);
    double_check(cmdline,nongui,"-vb",&src->vertical_break_threshold,dst->vertical_break_threshold);
    minus_check(cmdline,NULL,"-sm",&src->show_marked_source,dst->show_marked_source);
    minus_check(cmdline,nongui,"-toc",&src->use_toc,dst->use_toc);
    if (src->dst_break_pages != dst->dst_break_pages)
        {
        if (dst->dst_break_pages==0)
            strbuf_dsprintf(cmdline,nongui,"-bp--");
        else if (dst->dst_break_pages==1)
            strbuf_sprintf(cmdline,"-bp-");
        else if (dst->dst_break_pages==2)
            strbuf_dsprintf(cmdline,nongui,"-bp");
        else if (dst->dst_break_pages==3)
            strbuf_dsprintf(cmdline,nongui,"-bp+");
        else if (dst->dst_break_pages==4)
            strbuf_sprintf(cmdline,"-bp m");
        else
            strbuf_dsprintf(cmdline,nongui,"-bp %g",(-1.-dst->dst_break_pages)/1000.);
        src->dst_break_pages = dst->dst_break_pages;
        }
    minus_check(cmdline,nongui,"-fc",&src->fit_columns,dst->fit_columns);
    minus_check(cmdline,nongui,"-d",&src->dst_dither,dst->dst_dither);
    minus_check(cmdline,NULL,"-c",&src->dst_color,dst->dst_color);
    minus_check(cmdline,nongui,"-v",&src->verbose,dst->verbose);
    if (src->jpeg_quality != dst->jpeg_quality)
        {
        if (dst->jpeg_quality <= 0)
            strbuf_dsprintf(cmdline,nongui,"-png");
        else
            strbuf_dsprintf(cmdline,nongui,"-jpeg %d",dst->jpeg_quality);
        src->jpeg_quality = dst->jpeg_quality;
        }
    minus_check(cmdline,nongui,"-mc",&src->mark_corners,dst->mark_corners);
#ifdef HAVE_TESSERACT_LIB
    string_check(cmdline,nongui,"-ocrlang",src->dst_ocr_lang,dst->dst_ocr_lang);
#endif
#ifdef HAVE_OCR_LIB
    if ((src->dst_ocr_visibility_flags&7) != (dst->dst_ocr_visibility_flags&7))
        {
        strbuf_dsprintf(cmdline,nongui,"-ocrvis %s%s%s",
                       dst->dst_ocr_visibility_flags&1 ? "s" : "",
                       dst->dst_ocr_visibility_flags&2 ? "t" : "",
                       dst->dst_ocr_visibility_flags&4 ? "b" : "");
        }
    if ((src->dst_ocr_visibility_flags&24) != (dst->dst_ocr_visibility_flags&24))
        {
        strbuf_dsprintf(cmdline,nongui,"-ocrsp%s",
                       dst->dst_ocr_visibility_flags&16 ? "+"
                       : dst->dst_ocr_visibility_flags&8 ? "" : "-");
        }
    if ((src->dst_ocr_visibility_flags&32) != (dst->dst_ocr_visibility_flags&32))
        {
        strbuf_dsprintf(cmdline,nongui,"-ocrsort%s",
                       dst->dst_ocr_visibility_flags&32 ? "" : "-");
        }
    src->dst_ocr_visibility_flags = dst->dst_ocr_visibility_flags;
    double_check(cmdline,nongui,"-ocrhmax",&src->ocr_max_height_inches,dst->ocr_max_height_inches);
    if (src->dst_ocr != dst->dst_ocr)
        {
        if (dst->dst_ocr==0)
            strbuf_sprintf(cmdline,"-ocr-");
        else
            strbuf_dsprintf(cmdline,dst->dst_ocr=='t'?NULL:nongui,"-ocr %c",dst->dst_ocr);
        src->dst_ocr = dst->dst_ocr;
        }
#endif
    minus_check(cmdline,nongui,"-t",&src->src_trim,dst->src_trim);
    minus_check(cmdline,nongui,"-s",&src->dst_sharpen,dst->dst_sharpen);

    /* Autostraighten has some special cases */
    if (dst->src_autostraighten<=0)
        dst->src_autostraighten=-1;
    if (src->src_autostraighten<=0)
        src->src_autostraighten=-1;
    if (src->src_autostraighten != dst->src_autostraighten)
        {
        STRBUF *sbuf;
        if (fabs(dst->src_autostraighten-4.)<.001 || dst->src_autostraighten<0.)
            sbuf=NULL;
        else
            sbuf=nongui;
        strbuf_dsprintf(cmdline,sbuf,"-as%s",dst->src_autostraighten<0 ? "-":"");
        if (dst->src_autostraighten>0 && dst->src_autostraighten!=4)
            strbuf_dsprintf(cmdline,sbuf,"%d",dst->src_autostraighten);
        src->src_autostraighten=dst->src_autostraighten;
        }

    if (src->src_rot != dst->src_rot)
        {
        if (fabs(dst->src_rot-SRCROT_AUTOPREV)<.5)
            strbuf_dsprintf(cmdline,nongui,"-rt auto+");
        else if (fabs(dst->src_rot-SRCROT_AUTO)<.5)
            strbuf_dsprintf(cmdline,nongui,"-rt auto");
        else if (fabs(dst->src_rot-SRCROT_AUTOEP)<.5)
            strbuf_dsprintf(cmdline,nongui,"-rt aep");
        else 
            strbuf_dsprintf(cmdline,nongui,"-rt %d",dst->src_rot);
        src->src_rot=dst->src_rot;
        }
    double_check(cmdline,nongui,"-crgh",&src->column_row_gap_height_in,dst->column_row_gap_height_in);
    double_check(cmdline,nongui,"-cgr",&src->column_gap_range,dst->column_gap_range);
    double_check(cmdline,nongui,"-comax",&src->column_offset_max,dst->column_offset_max);
    integer_check(cmdline,NULL,"-col",&src->max_columns,dst->max_columns);
    string_check(cmdline,NULL,"-p",src->pagelist,dst->pagelist);
    string_check(cmdline,nongui,"-bpl",src->bpl,dst->bpl);
    string_check(cmdline,nongui,"-toclist",src->toclist,dst->toclist);
    string_check(cmdline,nongui,"-tocsave",src->tocsavefile,dst->tocsavefile);
    integer_check(cmdline,nongui,"-bpc",&src->dst_bpc,dst->dst_bpc);
    double_check(cmdline,nongui,"-g",&src->dst_gamma,dst->dst_gamma);
    double_check(cmdline,nongui,"-cg",&src->min_column_gap_inches,dst->min_column_gap_inches);
    double_check(cmdline,nongui,"-cgmax",&src->max_column_gap_inches,dst->max_column_gap_inches);
    double_check(cmdline,nongui,"-gtr",&src->gtr_in,dst->gtr_in);
    double_check(cmdline,nongui,"-gtc",&src->gtc_in,dst->gtc_in);
    double_check(cmdline,nongui,"-gtw",&src->gtw_in,dst->gtw_in);
    double_check(cmdline,nongui,"-cmax",&src->contrast_max,dst->contrast_max);
    double_check(cmdline,nongui,"-ch",&src->min_column_height_inches,dst->min_column_height_inches);
    double_check(cmdline,nongui,"-ds",&src->document_scale_factor,dst->document_scale_factor);
    double_check(cmdline,nongui,"-idpi",&src->user_src_dpi,dst->user_src_dpi);
    integer_check(cmdline,NULL,"-odpi",&src->dst_dpi,dst->dst_dpi);
    cropbox_check(cmdline,NULL,"-m",&src->srccropmargins,&dst->srccropmargins);
    cropbox_check(cmdline,nongui,"-om",&src->dstmargins,&dst->dstmargins);
    cropboxes_check(cmdline,nongui,&src->cropboxes,&dst->cropboxes);
    notesets_check(cmdline,nongui,&src->noteset,&dst->noteset);
    if (src->dst_figure_justify!=dst->dst_figure_justify
           || src->dst_min_figure_height_in != dst->dst_min_figure_height_in)
        {
        if (src->dst_min_figure_height_in!=dst->dst_min_figure_height_in)
            strbuf_dsprintf(cmdline,nongui,"-jf %d %g",dst->dst_figure_justify,
                                            dst->dst_min_figure_height_in);
        else
            strbuf_dsprintf(cmdline,nongui,"-jf %d",dst->dst_figure_justify);
        src->dst_figure_justify=dst->dst_figure_justify;
        src->dst_min_figure_height_in=dst->dst_min_figure_height_in;
        }
    if (src->dst_justify!=dst->dst_justify || src->dst_fulljustify!=dst->dst_fulljustify)
        {
        strbuf_dsprintf(cmdline,nongui,"-j %d%s",dst->dst_justify,
                     dst->dst_fulljustify==1 ? "+" : (dst->dst_fulljustify==0 ? "-" : ""));
        src->dst_justify = dst->dst_justify;
        src->dst_fulljustify = dst->dst_fulljustify;
        }
    double_check(cmdline,NULL,"-dr",&src->dst_display_resolution,dst->dst_display_resolution);
    if (src->dst_userheight!=dst->dst_userheight
            || src->dst_userheight_units!=dst->dst_userheight_units)
        {
        strbuf_sprintf(cmdline,"-h %g%s",dst->dst_userheight,unit_string(dst->dst_userheight_units));
        src->dst_userheight=dst->dst_userheight;
        src->dst_userheight_units=dst->dst_userheight_units;
        }
    if (src->dst_userwidth!=dst->dst_userwidth
            || src->dst_userwidth_units!=dst->dst_userwidth_units)
        {
        strbuf_sprintf(cmdline,"-w %g%s",dst->dst_userwidth,unit_string(dst->dst_userwidth_units));
        src->dst_userwidth=dst->dst_userwidth;
        src->dst_userwidth_units=dst->dst_userwidth_units;
        }
    double_check(cmdline,NULL,"-ws",&src->word_spacing,dst->word_spacing);
    {
    char cmdopt[8];

    strcpy(cmdopt,dst->src_paintwhite?"-wt+":"-wt");
    if (dst->src_paintwhite!=src->src_paintwhite)
        src->src_paintwhite = dst->src_paintwhite+1;
    integer_check(cmdline,nongui,cmdopt,&src->src_whitethresh,dst->src_whitethresh);
    }

    /* Shorthand the margins option if possible (v2.13) */
    margins_integercheck(cmdline,nongui,"-pad",&src->pad_left,dst->pad_left,
                                      &src->pad_top,dst->pad_top,
                                      &src->pad_right,dst->pad_right,
                                      &src->pad_bottom,dst->pad_bottom);
    integer_check(cmdline,nongui,"-debug",&src->debug,dst->debug);
    /*
    ** UNDOCUMENTED COMMAND-LINE ARGS
    */
    double_check(cmdline,nongui,"-whmax",&src->no_wrap_height_limit_inches,dst->no_wrap_height_limit_inches);
    double_check(cmdline,nongui,"-arlim",&src->no_wrap_ar_limit,dst->no_wrap_ar_limit);
    double_check(cmdline,nongui,"-rwmin",&src->little_piece_threshold_inches,dst->little_piece_threshold_inches);
    }


static void plus_minus_check(STRBUF *cmdline,STRBUF *nongui,char *optname,int *srcval,int dstval)

    {
/*
printf("@plus_minus_check, cmdline->s='%s', optname='%s', (*srcval)=%d, dstval=%d\n",
cmdline->s,optname,(*srcval),dstval);
*/
    if ((*srcval) != dstval)
        {
        strbuf_dsprintf(cmdline,nongui,"%s%s",optname,(dstval==1)?"":(dstval==0?"-":"+"));
        (*srcval)=dstval;
        }
    }


static void minus_check(STRBUF *cmdline,STRBUF *nongui,char *optname,int *srcval,int dstval)

    {
    if ((*srcval) != dstval)
        {
        strbuf_dsprintf(cmdline,nongui,"%s%s",optname,dstval?"":"-");
        (*srcval)=dstval;
        }
    }


static void minus_inverse(STRBUF *cmdline,STRBUF *nongui,char *optname,int *srcval,int dstval)

    {
    if ((*srcval) != dstval)
        {
        strbuf_dsprintf(cmdline,nongui,"%s%s",optname,dstval?"-":"");
        (*srcval)=dstval;
        }
    }


static void integer_check(STRBUF *cmdline,STRBUF *nongui,char *optname,int *srcval,int dstval)

    {
    if ((*srcval) != dstval)
        {
        strbuf_dsprintf(cmdline,nongui,"%s %d",optname,dstval);
        (*srcval)=dstval;
        }
    }


static void double_check(STRBUF *cmdline,STRBUF *nongui,char *optname,double *srcval,double dstval)

    {
/*
printf("dcheck: '%s' srcval=%g, dstval=%g\n",optname,(*srcval),dstval);
*/
    if ((*srcval) != dstval)
        {
        strbuf_dsprintf(cmdline,nongui,"%s %g",optname,dstval);
        (*srcval)=dstval;
        }
    }


static void string_check(STRBUF *cmdline,STRBUF *nongui,char *optname,char *srcval,char *dstval)

    {
    if (strcmp(srcval,dstval))
        {
        if (in_string(dstval," ")>=0)
            strbuf_dsprintf(cmdline,nongui,"%s \"%s\"",optname,dstval);
        else
            strbuf_dsprintf(cmdline,nongui,"%s %s",optname,dstval);
        strcpy(srcval,dstval);
        }
    }


static char *unit_string(int units)

    {
    static char *strvals[] = {"","in","cm","s","t","x"};

    if (units==UNITS_INCHES)
        return(strvals[1]);
    else if (units==UNITS_CM)
        return(strvals[2]);
    else if (units==UNITS_SOURCE)
        return(strvals[3]);
    else if (units==UNITS_TRIMMED)
        return(strvals[4]);
    else if (units==UNITS_OCRLAYER)
        return(strvals[5]);
    else
        return(strvals[0]);
    }


static void cropbox_check(STRBUF *cmdline,STRBUF *nongui,char *opt,K2CROPBOX *src,K2CROPBOX *dst)

    {
    if (cropbox_differ(src,dst))
        {
        int i;

        strbuf_dsprintf(cmdline,nongui,"%s",opt);
        for (i=0;i<4;i++)
            strbuf_dsprintf_no_space(cmdline,nongui,"%s%g%s",i==0?" ":",",dst->box[i],
                                    unit_string(dst->units[i]));
        (*src)=(*dst);
        }
    }


static void notesets_check(STRBUF *cmdline,STRBUF *nongui,K2NOTESET *src,K2NOTESET *dst)

    {
    if (notesets_are_different(src,dst))
        {
        int i;

        strbuf_dsprintf(cmdline,nongui,"-nl-");
        for (i=0;i<dst->n;i++)
            {
            double m;
            m=(dst->notes[i].left+dst->notes[i].right)/2.;
            strbuf_dsprintf(cmdline,nongui,"-n%s%s %g,%g",m<=0.5?"l":"r",
                           dst->notes[i].pagelist,
                           dst->notes[i].left,
                           dst->notes[i].right);
            }
        src->n=dst->n;
        }
    }

        
static void cropboxes_check(STRBUF *cmdline,STRBUF *nongui,K2CROPBOXES *src,K2CROPBOXES *dst)

    {
    if (cropboxes_are_different(src,dst))
        {
        int i;

        strbuf_dsprintf(cmdline,nongui,"-cbox-");
        for (i=0;i<dst->n;i++)
            {
            int c;
            strbuf_dsprintf(cmdline,nongui,"-cbox%s",dst->cropbox[i].pagelist);
            for (c=0;c<4;c++)
                strbuf_dsprintf_no_space(cmdline,nongui,"%s%g%s",c==0?" ":",",
                                         dst->cropbox[i].box[c],
                                         unit_string(dst->cropbox[i].units[c]));
            src->cropbox[i]=dst->cropbox[i];
            }
        src->n=dst->n;
        }
    }


static int notesets_are_different(K2NOTESET *src,K2NOTESET *dst)

    {
    int i;

    if (src->n != dst->n)
        return(1);
    for (i=0;i<src->n;i++)
        if (notes_differ(&src->notes[i],&dst->notes[i]))
            return(1);
    return(0);
    }


static int cropboxes_are_different(K2CROPBOXES *src,K2CROPBOXES *dst)

    {
    int i;

    if (src->n != dst->n)
        return(1);
    for (i=0;i<src->n;i++)
        if (cropbox_differ(&src->cropbox[i],&dst->cropbox[i]))
            return(1);
    return(0);
    }


static int cropbox_differ(K2CROPBOX *src,K2CROPBOX *dst)

    {
    int i;

    if (stricmp(src->pagelist,dst->pagelist))
        return(1);
    for (i=0;i<4;i++)
        {
        if (fabs(src->box[i]-dst->box[i])>1e-6)
            return(1);
        if (src->units[i]!=dst->units[i])
            return(1);
        }
    return(0);
    }


static int notes_differ(K2NOTES *src,K2NOTES *dst)

    {
    if (stricmp(src->pagelist,dst->pagelist))
        return(1);
    if (src->left != dst->left)
        return(1);
    if (src->right != dst->right)
        return(1);
    return(0);
    }


/*
** Added in v2.13
*/        
/*
static void margins_doublecheck(STRBUF *cmdline,char *opt,double *sleft,double dleft,
                                double *stop,double dtop,
                                double *sright,double dright,
                                double *sbottom,double dbottom)

    {
    if (fabs(dbottom-dtop)<1e-6 && fabs(dbottom-dleft)<1e-6 && fabs(dbottom-dright)<1e-6
         && (fabs(dbottom-(*sbottom))>1e-6 || fabs(dtop-(*stop))>1e-6
              || fabs(dleft-(*sleft))>1e-6 || fabs(dright-(*sright))>1e-6))
        {
        strbuf_sprintf(cmdline,"%s %g",opt,dbottom);
        (*sleft)=(*sright)=(*stop)=(*sbottom)=dbottom;
        }
    else
        {
        char opt2[16];
        sprintf(opt2,"%sb",opt);
        double_check(cmdline,opt2,sbottom,dbottom);
        sprintf(opt2,"%st",opt);
        double_check(cmdline,opt2,stop,dtop);
        sprintf(opt2,"%sl",opt);
        double_check(cmdline,opt2,sleft,dleft);
        sprintf(opt2,"%sr",opt);
        double_check(cmdline,opt2,sright,dright);
        }
    }
*/



/*
** Added in v2.13
*/        
static void margins_integercheck(STRBUF *cmdline,STRBUF *nongui,char *opt,int *sleft,int dleft,
                                int *stop,int dtop,
                                int *sright,int dright,
                                int *sbottom,int dbottom)

    {
    if (dbottom==dtop && dbottom==dleft && dbottom==dright
         && (dbottom!=(*sbottom) || dtop!=(*stop) || dleft!=(*sleft)
                || dright!=(*sright)))
        {
        strbuf_dsprintf(cmdline,nongui,"%s %d",opt,dbottom);
        (*sleft)=(*sright)=(*stop)=(*sbottom)=dbottom;
        }
    else
        {
        char opt2[16];
        sprintf(opt2,"-%cb",opt[1]);
        integer_check(cmdline,nongui,opt2,sbottom,dbottom);
        sprintf(opt2,"-%ct",opt[1]);
        integer_check(cmdline,nongui,opt2,stop,dtop);
        sprintf(opt2,"-%cl",opt[1]);
        integer_check(cmdline,nongui,opt2,sleft,dleft);
        sprintf(opt2,"-%cr",opt[1]);
        integer_check(cmdline,nongui,opt2,sright,dright);
        }
    }
#endif /* HAVE_K2GUI */
