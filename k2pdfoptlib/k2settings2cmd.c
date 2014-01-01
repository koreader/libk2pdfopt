/*
** k2settings2cmd.c    Convert changes in settings structure to equivalent
**                     command-line arguments.
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

#include "k2pdfopt.h"

#ifdef HAVE_K2GUI
static void k2settings_to_cmd(STRBUF *cmdline,K2PDFOPT_SETTINGS *dst,
                              K2PDFOPT_SETTINGS *src);
static void minus_check(STRBUF *cmdline,char *optname,int *srcval,int dstval);
static void minus_inverse(STRBUF *cmdline,char *optname,int *srcval,int dstval);
static void plus_minus_check(STRBUF *cmdline,char *optname,int *srcval,int dstval);
static void integer_check(STRBUF *cmdline,char *optname,int *srcval,int dstval);
static void double_check(STRBUF *cmdline,char *optname,double *srcval,double dstval);
static void string_check(STRBUF *cmdline,char *optname,char *srcval,char *dstval);
static char *unit_string(int units);
static int cropboxes_are_different(K2CROPBOXES *src,K2CROPBOXES *dst);
static int cropbox_differ(K2CROPBOX *src,K2CROPBOX *dst);
static void margins_doublecheck(STRBUF *cmdline,char *opt,double *sleft,double dleft,
                                double *stop,double dtop,
                                double *sright,double dright,
                                double *sbottom,double dbottom);
static void margins_integercheck(STRBUF *cmdline,char *opt,int *sleft,int dleft,
                                int *stop,int dtop,
                                int *sright,int dright,
                                int *sbottom,int dbottom);

/*
** Fills cmdline with the appropriate command-line options that will
** change the settings in "src" to the settings in "dst".
**
** "src" will not be modified.
*/
void k2pdfopt_settings_get_cmdline(STRBUF *cmdline,K2PDFOPT_SETTINGS *dst,
                                   K2PDFOPT_SETTINGS *src)

    {
    STRBUF *shortest,_shortest;
    K2PDFOPT_SETTINGS _src0,*src0;
    static char *modelabel[]={"def","fw","fp","crop","2col","tm","copy",""};
    int i,j,nd;

/*
printf("@kdpfopt_settings_get_cmdline()\n");
printf("    src->src_trim=%d, dst->src_trim=%d\n",src->src_trim,dst->src_trim);
printf("    src->dst_marleft=%g, dst->dst_marleft=%g\n",src->dst_marleft,dst->dst_marleft);
*/
    /*
    ** Try all "-mode" options and choose the shortest result
    */
    shortest=&_shortest;
    strbuf_init(shortest);
    src0=&_src0;

    /* Try with no mode and no device specified */
    k2pdfopt_settings_copy(src0,src);
    strbuf_clear(cmdline);
    k2settings_to_cmd(cmdline,dst,src0);
/*
printf("TRY 1.  cmdline='%s'\n",cmdline->s);
*/
    if (cmdline->s==NULL)
        return;
    strbuf_cpy(shortest,cmdline->s);
    /* Try different modes and devices and pick the shortest command-line length */
    nd=devprofiles_count();
    for (j=-1;j<nd;j++)
        {
        for (i=-1;i<0 || modelabel[i][0]!='\0';i++)
            {
            k2pdfopt_settings_copy(src0,src);
            strbuf_clear(cmdline);
            if (j>=0)
                k2settings_sprintf(cmdline,src0,"-dev %s",devprofile_alias(j));
            if (i>=0)
                k2settings_sprintf(cmdline,src0,"-mode %s",modelabel[i]);
            k2settings_to_cmd(cmdline,dst,src0);
/*
printf("TRY j=%d, i=%d.  cmdline='%s'\n",i,j,cmdline->s);
*/
            if (cmdline->s==NULL)
                return;
            if (strlen(cmdline->s) < strlen(shortest->s))
                strbuf_cpy(shortest,cmdline->s);
            }
        }
    strbuf_cpy(cmdline,shortest->s);
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
                              K2PDFOPT_SETTINGS *src)


    {
    /* Don't clear cmdline--it may be passed with some args already. */
#ifdef HAVE_K2GUI
    /* Re-launch code */
    plus_minus_check(cmdline,"-gui",&src->gui,dst->gui);
    minus_check(cmdline,"-guimin",&src->guimin,dst->guimin);
#endif
    minus_check(cmdline,"-?",&src->show_usage,dst->show_usage);
    /*
    if (!stricmp(cl->cmdarg,"-a") || !stricmp(cl->cmdarg,"-a-"))
        {
        if (setvals>=1)
            ansi_set(cl->cmdarg[2]=='-' ? 0 : 1);
        continue;
        }
    */
    minus_check(cmdline,"-x",&src->exit_on_complete,dst->exit_on_complete);
    if (src->query_user_explicit!=dst->query_user_explicit
            && dst->query_user_explicit)
        {
        strbuf_sprintf(cmdline,"-ui%s",dst->query_user?"":"-");
        src->query_user_explicit=dst->query_user_explicit;
        src->query_user=dst->query_user;
        }
    else
        minus_check(cmdline,"-ui",&src->query_user,dst->query_user);
    integer_check(cmdline,"-evl",&src->erase_vertical_lines,dst->erase_vertical_lines);    
    double_check(cmdline,"-vls",&src->vertical_line_spacing,dst->vertical_line_spacing);
    double_check(cmdline,"-vm",&src->vertical_multiplier,dst->vertical_multiplier);
    double_check(cmdline,"-vs",&src->max_vertical_gap_inches,dst->max_vertical_gap_inches);
    double_check(cmdline,"-vs",&src->defect_size_pts,dst->defect_size_pts);
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
    plus_minus_check(cmdline,"-wrap",&src->text_wrap,dst->text_wrap);
#ifdef HAVE_MUPDF_LIB
    if (src->user_usegs != dst->user_usegs)
        {
        strbuf_sprintf(cmdline,"-gs%s",dst->user_usegs==-1?"--":(dst->user_usegs==0)?"-":"");
        src->user_usegs = dst->user_usegs;
        }
    minus_check(cmdline,"-n",&src->use_crop_boxes,dst->use_crop_boxes);
/*
            if (k2settings->use_crop_boxes)
                {
                k2settings->text_wrap=0;
#ifdef HAVE_OCR_LIB
                k2settings->dst_ocr=0;
#endif
                }
*/
#endif
    minus_check(cmdline,"-neg",&src->dst_negative,dst->dst_negative);
    minus_check(cmdline,"-sp",&src->echo_source_page_count,dst->echo_source_page_count);
    minus_inverse(cmdline,"-r",&src->src_left_to_right,dst->src_left_to_right);
    minus_check(cmdline,"-hy",&src->hyphen_detect,dst->hyphen_detect);
    minus_check(cmdline,"-ls",&src->dst_landscape,dst->dst_landscape);
    string_check(cmdline,"-o",src->dst_opname_format,dst->dst_opname_format);
#ifdef HAVE_OCR_LIB
    string_check(cmdline,"-ocrout",src->ocrout,dst->ocrout);
#endif
    if (dst->overwrite_minsize_mb != src->overwrite_minsize_mb)
        {
        if (dst->overwrite_minsize_mb<0.)
            strbuf_sprintf(cmdline,"-ow");
        else if (dst->overwrite_minsize_mb==0.)
            strbuf_sprintf(cmdline,"-ow-");
        else
            strbuf_sprintf(cmdline,"-ow %g",dst->overwrite_minsize_mb);
        src->overwrite_minsize_mb = dst->overwrite_minsize_mb;
        }
    if ((dst->src_grid_cols!=src->src_grid_cols)
         || (dst->src_grid_rows!=src->src_grid_rows)
         || (dst->src_grid_overlap_percentage!=src->src_grid_overlap_percentage)
         || (dst->src_grid_order!=src->src_grid_order))
        {
        strbuf_sprintf(cmdline,"-grid %dx%dx%d%s",
                       dst->src_grid_cols,dst->src_grid_rows,
                       dst->src_grid_overlap_percentage,
                       dst->src_grid_order ? "+" : "");
        src->src_grid_cols = dst->src_grid_cols;
        src->src_grid_rows = dst->src_grid_rows;
        src->src_grid_overlap_percentage = dst->src_grid_overlap_percentage;
        src->src_grid_order = dst->src_grid_order;
        }
    integer_check(cmdline,"-f2p",&src->dst_fit_to_page,dst->dst_fit_to_page);
    double_check(cmdline,"-vb",&src->vertical_break_threshold,dst->vertical_break_threshold);
    minus_check(cmdline,"-sm",&src->show_marked_source,dst->show_marked_source);
    minus_check(cmdline,"-toc",&src->use_toc,dst->use_toc);
    if (src->dst_break_pages != dst->dst_break_pages)
        {
        if (dst->dst_break_pages==0)
            strbuf_sprintf(cmdline,"-bp--");
        else if (dst->dst_break_pages==1)
            strbuf_sprintf(cmdline,"-bp-");
        else if (dst->dst_break_pages==2)
            strbuf_sprintf(cmdline,"-bp");
        else if (dst->dst_break_pages==3)
            strbuf_sprintf(cmdline,"-bp+");
        else if (dst->dst_break_pages==4)
            strbuf_sprintf(cmdline,"-bp m");
        else
            strbuf_sprintf(cmdline,"-bp %g",(-1.-dst->dst_break_pages)/1000.);
        src->dst_break_pages = dst->dst_break_pages;
        }
    minus_check(cmdline,"-fc",&src->fit_columns,dst->fit_columns);
    minus_check(cmdline,"-d",&src->dst_dither,dst->dst_dither);
    minus_check(cmdline,"-c",&src->dst_color,dst->dst_color);
    minus_check(cmdline,"-v",&src->verbose,dst->verbose);
    if (src->jpeg_quality != dst->jpeg_quality)
        {
        if (dst->jpeg_quality <= 0)
            strbuf_sprintf(cmdline,"-png");
        else
            strbuf_sprintf(cmdline,"-jpeg %d",dst->jpeg_quality);
        src->jpeg_quality = dst->jpeg_quality;
        }
    minus_check(cmdline,"-mc",&src->mark_corners,dst->mark_corners);
#ifdef HAVE_TESSERACT_LIB
    string_check(cmdline,"-ocrlang",src->dst_ocr_lang,dst->dst_ocr_lang);
#endif
#ifdef HAVE_OCR_LIB
    if (src->dst_ocr_visibility_flags != dst->dst_ocr_visibility_flags)
        {
        strbuf_sprintf(cmdline,"-ocrvis %s%s%s",
                       dst->dst_ocr_visibility_flags&1 ? "s" : "",
                       dst->dst_ocr_visibility_flags&2 ? "t" : "",
                       dst->dst_ocr_visibility_flags&4 ? "b" : "");
        src->dst_ocr_visibility_flags = dst->dst_ocr_visibility_flags;
        }
    double_check(cmdline,"-ocrhmax",&src->ocr_max_height_inches,dst->ocr_max_height_inches);
    if (src->dst_ocr != dst->dst_ocr)
        {
        if (dst->dst_ocr==0)
            strbuf_sprintf(cmdline,"-ocr-");
        else
            strbuf_sprintf(cmdline,"-ocr %c",dst->dst_ocr);
        src->dst_ocr = dst->dst_ocr;
        }
#endif
    minus_check(cmdline,"-t",&src->src_trim,dst->src_trim);
    minus_check(cmdline,"-s",&src->dst_sharpen,dst->dst_sharpen);

    /* Autostraighten has some special cases */
    if (dst->src_autostraighten<=0)
        dst->src_autostraighten=-1;
    if (src->src_autostraighten<=0)
        src->src_autostraighten=-1;
    if (src->src_autostraighten != dst->src_autostraighten)
        {
        strbuf_sprintf(cmdline,"-as%s",dst->src_autostraighten<0 ? "-":"");
        if (dst->src_autostraighten>0 && dst->src_autostraighten!=4)
            strbuf_sprintf(cmdline,"%d",dst->src_autostraighten);
        src->src_autostraighten=dst->src_autostraighten;
        }

    if (src->src_rot != dst->src_rot)
        {
        if (fabs(dst->src_rot-SRCROT_AUTOPREV)<.5)
            strbuf_sprintf(cmdline,"-rt auto+");
        else if (fabs(dst->src_rot-SRCROT_AUTO)<.5)
            strbuf_sprintf(cmdline,"-rt auto");
        else if (fabs(dst->src_rot-SRCROT_AUTOEP)<.5)
            strbuf_sprintf(cmdline,"-rt aep");
        else 
            strbuf_sprintf(cmdline,"-rt %d",dst->src_rot);
        src->src_rot=dst->src_rot;
        }
    double_check(cmdline,"-crgh",&src->column_row_gap_height_in,dst->column_row_gap_height_in);
    double_check(cmdline,"-cgr",&src->column_gap_range,dst->column_gap_range);
    double_check(cmdline,"-comax",&src->column_offset_max,dst->column_offset_max);
    integer_check(cmdline,"-col",&src->max_columns,dst->max_columns);
    string_check(cmdline,"-p",src->pagelist,dst->pagelist);
    string_check(cmdline,"-bpl",src->bpl,dst->bpl);
    string_check(cmdline,"-toclist",src->toclist,dst->toclist);
    string_check(cmdline,"-tocsave",src->tocsavefile,dst->tocsavefile);
    integer_check(cmdline,"-bpc",&src->dst_bpc,dst->dst_bpc);
    double_check(cmdline,"-g",&src->dst_gamma,dst->dst_gamma);
    double_check(cmdline,"-cg",&src->min_column_gap_inches,dst->min_column_gap_inches);
    double_check(cmdline,"-cgmax",&src->max_column_gap_inches,dst->max_column_gap_inches);
    double_check(cmdline,"-gtr",&src->gtr_in,dst->gtr_in);
    double_check(cmdline,"-gtc",&src->gtc_in,dst->gtc_in);
    double_check(cmdline,"-gtw",&src->gtw_in,dst->gtw_in);
    double_check(cmdline,"-cmax",&src->contrast_max,dst->contrast_max);
    double_check(cmdline,"-ch",&src->min_column_height_inches,dst->min_column_height_inches);
    double_check(cmdline,"-ds",&src->document_scale_factor,dst->document_scale_factor);
    double_check(cmdline,"-idpi",&src->user_src_dpi,dst->user_src_dpi);
    integer_check(cmdline,"-odpi",&src->dst_dpi,dst->dst_dpi);
    if (cropboxes_are_different(&src->cropboxes,&dst->cropboxes))
        {
        int i;

        strbuf_sprintf(cmdline,"-cbox-");
        for (i=0;i<dst->cropboxes.n;i++)
            {
            int c;
            strbuf_sprintf(cmdline,"-cbox%s",dst->cropboxes.cropbox[i].pagelist);
            for (c=0;c<4;c++)
                strbuf_sprintf_no_space(cmdline,"%s%g%s",c==0?" ":",",
                                         dst->cropboxes.cropbox[i].box[c],
                                         unit_string(dst->cropboxes.cropbox[i].units[c]));
            }
        }
    if (src->dst_figure_justify!=dst->dst_figure_justify
           || src->dst_min_figure_height_in != dst->dst_min_figure_height_in)
        {
        if (src->dst_min_figure_height_in!=dst->dst_min_figure_height_in)
            strbuf_sprintf(cmdline,"-jf %d %g",dst->dst_figure_justify,
                                            dst->dst_min_figure_height_in);
        else
            strbuf_sprintf(cmdline,"-jf %d",dst->dst_figure_justify);
        src->dst_figure_justify=dst->dst_figure_justify;
        src->dst_min_figure_height_in=dst->dst_min_figure_height_in;
        }
    if (src->dst_justify!=dst->dst_justify || src->dst_fulljustify!=dst->dst_fulljustify)
        {
        strbuf_sprintf(cmdline,"-j %d%s",dst->dst_justify,
                     dst->dst_fulljustify==1 ? "+" : (dst->dst_fulljustify==0 ? "-" : ""));
        src->dst_justify = dst->dst_justify;
        src->dst_fulljustify = dst->dst_fulljustify;
        }
    double_check(cmdline,"-dr",&src->dst_display_resolution,dst->dst_display_resolution);
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
    {
    double sws,dws;
    sws = src->auto_word_spacing ? -fabs(src->word_spacing) : fabs(src->word_spacing);
    dws = dst->auto_word_spacing ? -fabs(dst->word_spacing) : fabs(dst->word_spacing);
    double_check(cmdline,"-ws",&sws,dws);
    src->auto_word_spacing = dst->auto_word_spacing;
    src->word_spacing = dst->word_spacing;
    }
    integer_check(cmdline,"-wt",&src->src_whitethresh,dst->src_whitethresh);
    if (src->dst_marbot<0.)
        src->dst_marbot=src->dst_mar;
    if (src->dst_martop<0.)
        src->dst_martop=src->dst_mar;
    if (src->dst_marleft<0.)
        src->dst_marleft=src->dst_mar;
    if (src->dst_marright<0.)
        src->dst_marright=src->dst_mar;
    if (dst->dst_marbot<0.)
        dst->dst_marbot=dst->dst_mar;
    if (dst->dst_martop<0.)
        dst->dst_martop=dst->dst_mar;
    if (dst->dst_marleft<0.)
        dst->dst_marleft=dst->dst_mar;
    if (dst->dst_marright<0.)
        dst->dst_marright=dst->dst_mar;

    /* Shorthand the margins option if possible (v2.13) */
    margins_doublecheck(cmdline,"-om",&src->dst_marleft,dst->dst_marleft,
                                      &src->dst_martop,dst->dst_martop,
                                      &src->dst_marright,dst->dst_marright,
                                      &src->dst_marbot,dst->dst_marbot);
    margins_doublecheck(cmdline,"-m",&src->mar_left,dst->mar_left,
                                     &src->mar_top,dst->mar_top,
                                     &src->mar_right,dst->mar_right,
                                     &src->mar_bot,dst->mar_bot);
    margins_integercheck(cmdline,"-pad",&src->pad_left,dst->pad_left,
                                      &src->pad_top,dst->pad_top,
                                      &src->pad_right,dst->pad_right,
                                      &src->pad_bottom,dst->pad_bottom);
    integer_check(cmdline,"-debug",&src->debug,dst->debug);
    /*
    ** UNDOCUMENTED COMMAND-LINE ARGS
    */
    double_check(cmdline,"-whmax",&src->no_wrap_height_limit_inches,dst->no_wrap_height_limit_inches);
    double_check(cmdline,"-arlim",&src->no_wrap_ar_limit,dst->no_wrap_ar_limit);
    double_check(cmdline,"-rwmin",&src->little_piece_threshold_inches,dst->little_piece_threshold_inches);
    }


static void plus_minus_check(STRBUF *cmdline,char *optname,int *srcval,int dstval)

    {
/*
printf("@plus_minus_check, cmdline->s='%s', optname='%s', (*srcval)=%d, dstval=%d\n",
cmdline->s,optname,(*srcval),dstval);
*/
    if ((*srcval) != dstval)
        {
        strbuf_sprintf(cmdline,"%s%s",optname,(dstval==1)?"":(dstval==0?"-":"+"));
        (*srcval)=dstval;
        }
    }


static void minus_check(STRBUF *cmdline,char *optname,int *srcval,int dstval)

    {
    if ((*srcval) != dstval)
        {
        strbuf_sprintf(cmdline,"%s%s",optname,dstval?"":"-");
        (*srcval)=dstval;
        }
    }


static void minus_inverse(STRBUF *cmdline,char *optname,int *srcval,int dstval)

    {
    if ((*srcval) != dstval)
        {
        strbuf_sprintf(cmdline,"%s%s",optname,dstval?"-":"");
        (*srcval)=dstval;
        }
    }


static void integer_check(STRBUF *cmdline,char *optname,int *srcval,int dstval)

    {
    if ((*srcval) != dstval)
        {
        strbuf_sprintf(cmdline,"%s %d",optname,dstval);
        (*srcval)=dstval;
        }
    }


static void double_check(STRBUF *cmdline,char *optname,double *srcval,double dstval)

    {
/*
printf("dcheck: '%s' srcval=%g, dstval=%g\n",optname,(*srcval),dstval);
*/
    if ((*srcval) != dstval)
        {
        strbuf_sprintf(cmdline,"%s %g",optname,dstval);
        (*srcval)=dstval;
        }
    }


static void string_check(STRBUF *cmdline,char *optname,char *srcval,char *dstval)

    {
    if (strcmp(srcval,dstval))
        {
        if (in_string(dstval," ")>=0)
            strbuf_sprintf(cmdline,"%s \"%s\"",optname,dstval);
        else
            strbuf_sprintf(cmdline,"%s %s",optname,dstval);
        strcpy(srcval,dstval);
        }
    }


static char *unit_string(int units)

    {
    static char *strvals[] = {"","in","cm","s","t"};

    if (units==UNITS_INCHES)
        return(strvals[1]);
    else if (units==UNITS_CM)
        return(strvals[2]);
    else if (units==UNITS_SOURCE)
        return(strvals[3]);
    else if (units==UNITS_TRIMMED)
        return(strvals[4]);
    else
        return(strvals[0]);
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


/*
** Added in v2.13
*/        
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


/*
** Added in v2.13
*/        
static void margins_integercheck(STRBUF *cmdline,char *opt,int *sleft,int dleft,
                                int *stop,int dtop,
                                int *sright,int dright,
                                int *sbottom,int dbottom)

    {
    if (dbottom==dtop && dbottom==dleft && dbottom==dright
         && (dbottom!=(*sbottom) || dtop!=(*stop) || dleft!=(*sleft)
                || dright!=(*sright)))
        {
        strbuf_sprintf(cmdline,"%s %d",opt,dbottom);
        (*sleft)=(*sright)=(*stop)=(*sbottom)=dbottom;
        }
    else
        {
        char opt2[16];
        sprintf(opt2,"-%cb",opt[1]);
        integer_check(cmdline,opt2,sbottom,dbottom);
        sprintf(opt2,"-%ct",opt[1]);
        integer_check(cmdline,opt2,stop,dtop);
        sprintf(opt2,"-%cl",opt[1]);
        integer_check(cmdline,opt2,sleft,dleft);
        sprintf(opt2,"-%cr",opt[1]);
        integer_check(cmdline,opt2,sright,dright);
        }
    }
#endif /* HAVE_K2GUI */
