/*
** k2cmdparse.c   Parse command-line options for k2pdfopt.
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

static void set_value_with_units(char *s,double *val,int *units,int defunits);
static int valid_numerical_char(int c);
static int next_is_number(CMDLINEINPUT *cl,int setvals,int quiet,int *good,int *readnext,double *dstval);
static int next_is_integer(CMDLINEINPUT *cl,int setvals,int quiet,int *good,int *readnext,int *dstval);

#define NEEDS_VALUE(x,y) if (!stricmp(cl->cmdarg,x)) { \
                         if (!next_is_number(cl,setvals==1,quiet,&good,&readnext,&k2settings->y)) \
                             break; \
                         continue; }

#define NEEDS_INTEGER(x,y) if (!stricmp(cl->cmdarg,x)) { \
                        if (!next_is_integer(cl,setvals==1,quiet,&good,&readnext,&k2settings->y)) \
                              break; \
                        continue; }

#define MINUS_OPTION(x,y,sv) if (!stricmp(cl->cmdarg,x) || !stricmp(cl->cmdarg,x "-")) \
            { \
            if (setvals==sv) \
                k2settings->y=(cl->cmdarg[strlen(cl->cmdarg)-1]=='-' ? 0 : 1); \
            continue; \
            }
#define NEEDS_STRING(x,y,maxlen) if (!stricmp(cl->cmdarg,x)) { \
            if (cmdlineinput_next(cl)==NULL) \
                break; \
            if (setvals==1) \
                { \
                strncpy(k2settings->y,cl->cmdarg,maxlen); \
                k2settings->y[maxlen]='\0'; \
                } \
            continue; \
            } 

/*
** Return file count
** NEW BEHAVIOR (v1.65 and up):
** setvals==1 to set all values based on options
**        ==2 to set only ansi, user interface, exit on complete
**            (also still sets and counts files.)
**
** OLD BEHAVIOR (PRE v1.65):
** setvals==1 to set all values based on options
**        ==2 to set only ansi, user interface, exit on complete
**        ==0 to not set any values
** procfiles == 1 to process files
**           == 0 to count files only
*/
int parse_cmd_args(K2PDFOPT_CONVERSION *k2conv,STRBUF *env,STRBUF *cmdline,
                   STRBUF *usermenu,int setvals,int quiet)

    {
    CMDLINEINPUT _cl,*cl;
    STRBUF *allopts,_allopts;
    int readnext,good;
    K2PDFOPT_SETTINGS *k2settings;

    k2settings=&k2conv->k2settings;
    k2pdfopt_files_clear(&k2conv->k2files);
    allopts=&_allopts;
    strbuf_init(allopts);
    if (env!=NULL)
        strbuf_cat(allopts,env->s);
    if (cmdline!=NULL)
        strbuf_cat(allopts,cmdline->s);
    if (usermenu!=NULL)
        strbuf_cat(allopts,usermenu->s);
    cl=&_cl;
    cmdlineinput_init(cl,0,NULL,allopts->s);
    readnext=1;
    while (1)
        {
        if (readnext && cmdlineinput_next(cl)==NULL)
            break;
        readnext=1;
#ifdef HAVE_K2GUI
        /* Re-launch code */
        if (!stricmp(cl->cmdarg,"-gui+"))
            {
            if (setvals==2)
                k2settings->gui = 2;
            continue;
            }
        MINUS_OPTION("-gui",gui,2)
        MINUS_OPTION("-guimin",guimin,2)
#endif
        MINUS_OPTION("-?",show_usage,2)
        MINUS_OPTION("-toc",use_toc,1)
        MINUS_OPTION("-sp",echo_source_page_count,1)
        MINUS_OPTION("-neg",dst_negative,1)
        MINUS_OPTION("-hy",hyphen_detect,1)
        MINUS_OPTION("-ls",dst_landscape,1)
        MINUS_OPTION("-sm",show_marked_source,1)
        MINUS_OPTION("-fc",fit_columns,1)
        MINUS_OPTION("-d",dst_dither,1)
        MINUS_OPTION("-c",dst_color,1)
        MINUS_OPTION("-v",verbose,1)
        MINUS_OPTION("-mc",mark_corners,1)
        MINUS_OPTION("-t",src_trim,1)
        MINUS_OPTION("-s",dst_sharpen,1)
        /*
        MINUS_OPTION("-pi",preserve_indentation,1)
        */

        if (!stricmp(cl->cmdarg,"-a") || !stricmp(cl->cmdarg,"-a-"))
            {
            if (setvals>=1)
                ansi_set(cl->cmdarg[2]=='-' ? 0 : 1);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-x") || !stricmp(cl->cmdarg,"-x-"))
            {
            if (setvals>=1)
                k2settings->exit_on_complete=(cl->cmdarg[2]=='-' ? 0 : 1);
            continue;
            }
        if (!strnicmp(cl->cmdarg,"-ui",3))
            {
            if (setvals>=1)
                {
                if (cl->cmdarg[3]!='-')
                   k2settings->query_user_explicit=1;
                k2settings->query_user=(cl->cmdarg[3]!='-') ? 1 : 0;
                }
            continue;
            }
        if (!stricmp(cl->cmdarg,"-bmp") || !stricmp(cl->cmdarg,"-bmp-"))
            {
            if (cl->cmdarg[4]=='-')
                 {
                 if (setvals==1)
                     k2settings->preview_page=0;
                 continue;
                 }
            k2settings->preview_page=1;
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                k2settings->preview_page=atoi(cl->cmdarg);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-vm"))
            {
            if (!next_is_number(cl,setvals==1,quiet,&good,&readnext,&k2settings->vertical_multiplier))
                break;
            if (good && setvals==1)
                {
                if (k2settings->vertical_multiplier < 0.1)
                    k2settings->vertical_multiplier = 0.1;
                }
            continue;
            }
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
                    {
                    if (!quiet)
                        k2printf(TTEXT_WARN "\aDevice profile '%s' not known." TTEXT_NORMAL "\n",cl->cmdarg);
                    }
                }
            continue;
            }
        if (!strnicmp(cl->cmdarg,"-wrap",5))
            {
            if (setvals==1)
                {
                k2settings->text_wrap=(cl->cmdarg[5]=='-') ? 0 : (cl->cmdarg[5]=='+' ? 2 : 1);
                if (k2settings->text_wrap)
                    {
                    if (k2settings->use_crop_boxes)
                        {
                        k2settings->use_crop_boxes=0;
#ifdef HAVE_OCR_LIB
                        k2settings->dst_ocr='m';
#endif
                        }
                    }
                }
            continue;
            }
#ifdef HAVE_MUPDF_LIB
        if (!stricmp(cl->cmdarg,"-gs") || !stricmp(cl->cmdarg,"-gs-")
                                       || !stricmp(cl->cmdarg,"-gs--"))
            {
            if (setvals==1)
                k2settings->user_usegs=(cl->cmdarg[3]=='-' ? (cl->cmdarg[4]=='-' ? -1 : 0) : 1);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-n") || !stricmp(cl->cmdarg,"-n-"))
            {
            if (setvals==1)
                {
                k2settings->use_crop_boxes=(cl->cmdarg[2]=='-') ? 0 : 1;
                if (k2settings->use_crop_boxes)
                    {
                    k2settings->text_wrap=0;
#ifdef HAVE_OCR_LIB
                    k2settings->dst_ocr=0;
#endif
                    }
#ifdef HAVE_OCR_LIB
                else
                    {
                    if (k2settings->dst_ocr==0)
                        k2settings->dst_ocr='m';
                    }
#endif
                }
            continue;
            }
#endif
        if (!stricmp(cl->cmdarg,"-r") || !stricmp(cl->cmdarg,"-r-"))
            {
            if (setvals==1)
                k2settings->src_left_to_right=(cl->cmdarg[2]=='-') ? 1 : 0;
            continue;
            }
        if (!stricmp(cl->cmdarg,"-mode"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                {
                if (!stricmp(cl->cmdarg,"pdfr") 
                          || !stricmp(cl->cmdarg,"copy")
                          || !stricmp(cl->cmdarg,"trim")
                          || !stricmp(cl->cmdarg,"crop")
                          || !stricmp(cl->cmdarg,"tm"))
                    {
                    int tm,crop;

                    crop=(!stricmp(cl->cmdarg,"crop"));
                    tm=(!stricmp(cl->cmdarg,"trim") || !stricmp(cl->cmdarg,"tm"));
                    /* -n- -wrap- -col 1 -vb -2 -w -1 -h -1 -dpi 150 -rt 0 -c -t- -f2p -2 */
                    /* -m 0 -om 0 -pl 0 -pr 0 -pt 0 -pb 0 -mc- */
                    k2settings->use_crop_boxes= (tm||crop) ? 1 : 0;
#ifdef HAVE_OCR_LIB
                    k2settings->dst_ocr=(tm||crop) ? 0 : 'm';
#endif
                    k2settings->text_wrap=0;
                    k2settings->max_columns=1;
                    k2settings->vertical_break_threshold=-2;
                    k2settings->dst_userwidth=1.0;
                    k2settings->dst_userwidth_units=(tm||crop) ? UNITS_TRIMMED : UNITS_SOURCE;
                    k2settings->dst_userheight=1.0;
                    k2settings->dst_userheight_units=(tm||crop) ? UNITS_TRIMMED : UNITS_SOURCE;
                    k2settings->dst_dpi=150;
                    k2settings->src_rot=0.;
                    k2settings->dst_color=1;
                    k2settings->src_trim=tm ? 1 : 0;
                    k2settings->dst_fit_to_page=-2;
                    k2settings->mar_left=k2settings->mar_top=k2settings->mar_right=k2settings->mar_bot=0.;
                    k2settings->dst_mar=k2settings->dst_marleft=k2settings->dst_martop=k2settings->dst_marright=k2settings->dst_marbot=0.;
                    k2settings->pad_left=k2settings->pad_top=k2settings->pad_bottom=k2settings->pad_right=0;
                    k2settings->mark_corners=0;
                    }
                else if (!stricmp(cl->cmdarg,"fw") 
                          || !stricmp(cl->cmdarg,"sopdf")
                          || !stricmp(cl->cmdarg,"fitwidth")
                          || !stricmp(cl->cmdarg,"fp")
                          || !stricmp(cl->cmdarg,"fitpage"))
                    {
                    int fitpage;

                    fitpage=(!stricmp(cl->cmdarg,"fp") || !stricmp(cl->cmdarg,"fitpage"));
                    /* -wrap- -col 1 -vb -2 -t -ls */
                    k2settings->use_crop_boxes=1;
#ifdef HAVE_OCR_LIB
                    k2settings->dst_ocr=0;
#endif
                    k2settings->text_wrap=0;
                    k2settings->max_columns=1;
                    k2settings->vertical_break_threshold=-2;
                    k2settings->src_trim=1;
                    if (fitpage)
                        k2settings->dst_fit_to_page=-2;
                    k2settings->dst_landscape=fitpage ? 0 : 1;
                    }
                else if (!stricmp(cl->cmdarg,"2col")
                          || !stricmp(cl->cmdarg,"2-column")
                          || !stricmp(cl->cmdarg,"col2"))
                    {
                    k2settings->use_crop_boxes=1;
#ifdef HAVE_OCR_LIB
                    k2settings->dst_ocr=0;
#endif
                    k2settings->text_wrap=0;
                    k2settings->max_columns=2;
                    k2settings->vertical_break_threshold=-2;
                    k2settings->src_trim=1;
                    }
                else if (!stricmp(cl->cmdarg,"def") 
                          || !stricmp(cl->cmdarg,"default")
                          || !stricmp(cl->cmdarg,"std")
                          || !stricmp(cl->cmdarg,"standard"))
                    {
                    k2pdfopt_settings_set_to_device(k2settings,devprofile_get("k2"));
                    k2settings->use_crop_boxes=0;
#ifdef HAVE_OCR_LIB
                    k2settings->dst_ocr='m';
#endif
                    k2settings->dst_landscape=0;
                    k2settings->text_wrap=1;
                    k2settings->max_columns=2;
                    k2settings->vertical_break_threshold=1.75;
                    k2settings->src_rot=SRCROT_AUTO;
                    k2settings->src_trim=1;
                    k2settings->dst_fit_to_page=0;
                    k2settings->mar_left=k2settings->mar_top=k2settings->mar_right=k2settings->mar_bot=-1.;
                    k2settings->dst_mar=k2settings->dst_marleft=k2settings->dst_martop=k2settings->dst_marright=k2settings->dst_marbot=0.02;
                    }
                else
                    {
                    if (!quiet)
                        k2printf(TTEXT_WARN "\a\n** Unknown mode:  %s **\n\n" TTEXT_NORMAL,
                                 cl->cmdarg);
                    }
                }
            continue;
            }
        if (!stricmp(cl->cmdarg,"-ow") || !stricmp(cl->cmdarg,"-ow-"))
            {
            int always_prompt;
            char *ptr;
            always_prompt = (cl->cmdarg[3]=='-');
            if (((ptr=cmdlineinput_next(cl))==NULL) || !is_a_number(cl->cmdarg))
                {
                readnext=0;
                if (setvals==1)
                    k2settings->overwrite_minsize_mb= always_prompt ? 0 : -1;
                if (ptr==NULL)
                    break;
                continue;
                }
            if (setvals==1)
                k2settings->overwrite_minsize_mb=atof(cl->cmdarg);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-grid"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                {
                char buf[128];
                double v[3];
                int na,i;

                strncpy(buf,cl->cmdarg,127);
                buf[127]='\0';
                k2settings->src_grid_order=0;
                for (i=0;buf[i]!='\0';i++)
                    {
                    if (tolower(buf[i])=='x')
                        buf[i]=' ';
                    if (buf[i]=='+' && buf[i+1]=='\0')
                        k2settings->src_grid_order=1;
                    }
                na=string_read_doubles(buf,v,3);
                if (na>=2)
                    {
                    k2settings->src_grid_cols=(int)(v[0]+.5);
                    k2settings->src_grid_rows=(int)(v[1]+.5);
                    if (na>2)
                        k2settings->src_grid_overlap_percentage=(int)(v[2]+.5);
                    }
                else
                    k2settings->src_grid_cols = k2settings->src_grid_rows = -1;
                if (k2settings->src_grid_cols>0 && k2settings->src_grid_rows>0)
                    {
                    k2settings->use_crop_boxes=1;
#ifdef HAVE_OCR_LIB
                    k2settings->dst_ocr=0;
#endif
                    k2settings->dst_fit_to_page=-2;
                    k2settings->vertical_break_threshold=-2;
                    k2settings->text_wrap=1;
                    k2settings->max_columns=1;
                    }
                }
            continue;
            }
        if (!stricmp(cl->cmdarg,"-f2p"))
            {
            if (!next_is_integer(cl,setvals==1,quiet,&good,&readnext,&k2settings->dst_fit_to_page))
                break;
            if (good && setvals==1)
                {
                if (k2settings->dst_fit_to_page == -2)
                    k2settings->vertical_break_threshold=-2.;
                }
            continue;
            }
        if (!stricmp(cl->cmdarg,"-bp") || !stricmp(cl->cmdarg,"-bp-")
                                       || !stricmp(cl->cmdarg,"-bp--")
                                       || !stricmp(cl->cmdarg,"-bp+"))
            {
            if (cl->cmdarg[3]=='-')
                {
                if (setvals==1)
                    k2settings->dst_break_pages = (cl->cmdarg[4]=='-' ? 0 : 1);
                continue;
                }
            if (cl->cmdarg[3]=='+')
                {
                if (setvals==1)
                    k2settings->dst_break_pages=3;
                continue;
                }
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (is_a_number(cl->cmdarg) || !stricmp(cl->cmdarg,"m"))
                {
                if (setvals==1)
                    {
                    if (!stricmp(cl->cmdarg,"m"))
                        k2settings->dst_break_pages = 4; /* Special code */
                    else
                        k2settings->dst_break_pages= -1 - (int)(atof(cl->cmdarg)*1000.+.5);
                    }
                }
            else
                {
                if (setvals==1)
                    k2settings->dst_break_pages=2;
                readnext=0;
                }
            continue;
            }
        if (!strnicmp(cl->cmdarg,"-png",4))
            {
            if (setvals==1)
                k2settings->jpeg_quality=(cl->cmdarg[4]=='-') ? 90 : -1;
            continue;
            }
        if (!stricmp(cl->cmdarg,"-ocrlang") || !stricmp(cl->cmdarg,"-l") 
                                            || !stricmp(cl->cmdarg,"-lang"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
#ifdef HAVE_TESSERACT_LIB
            strncpy(k2settings->dst_ocr_lang,cl->cmdarg,63);
            k2settings->dst_ocr_lang[63]='\0';
#endif
            continue;
            }
        if (!stricmp(cl->cmdarg,"-ocrvis"))
            {
            int i;

            if (cmdlineinput_next(cl)==NULL)
                break;
            for (i=0;cl->cmdarg[i]!='\0';i++)
                if (tolower(cl->cmdarg[i])!='s'
                     && tolower(cl->cmdarg[i])!='t'
                     && tolower(cl->cmdarg[i])!='b')
                    break;
            if (cl->cmdarg[i]!='\0')
                {
                if (!quiet)
                    k2printf(TTEXT_WARN "\a-ocrvis expects s|t|b.  Arg %s not used for -ocrvis." TTEXT_NORMAL "\n",cl->cmdarg);
                readnext=0;
                continue;
                }
#ifdef HAVE_OCR_LIB
            if (setvals==1)
                {
                k2settings->dst_ocr_visibility_flags=0;
                if (in_string(cl->cmdarg,"s")>=0)
                    k2settings->dst_ocr_visibility_flags |= 1;
                if (in_string(cl->cmdarg,"t")>=0)
                    k2settings->dst_ocr_visibility_flags |= 2;
                if (in_string(cl->cmdarg,"b")>=0)
                    k2settings->dst_ocr_visibility_flags |= 4;
                }
#endif
            continue;
            }
        if (!stricmp(cl->cmdarg,"-ocrcol"))
            {
            if (!next_is_integer(cl,setvals==1,quiet,&good,&readnext,NULL))
                break;
#ifdef HAVE_OCR_LIB
            if (good && setvals==1)
                k2settings->ocr_max_columns=atoi(cl->cmdarg);
#endif
            continue;
            }
        if (!stricmp(cl->cmdarg,"-ocrhmax"))
            {
            if (!next_is_number(cl,setvals==1,quiet,&good,&readnext,NULL))
                break;
#ifdef HAVE_OCR_LIB
            if (good && setvals==1)
                k2settings->ocr_max_height_inches=atof(cl->cmdarg);
#endif
            continue;
            }
        if (!stricmp(cl->cmdarg,"-ocr") || !stricmp(cl->cmdarg,"-ocr-"))
            {
#ifndef HAVE_OCR_LIB
            if (setvals==1)
                {
                static int warned=0;
                if (!warned && !quiet)
                k2printf(TTEXT_WARN "\a\n** No OCR capability in this compile of k2pdfopt! **\n\n"
                        TTEXT_NORMAL);
                warned=1;
                }
#endif
            if (cl->cmdarg[4]=='-')
                {
#ifdef HAVE_OCR_LIB
                if (setvals==1)
                    k2settings->dst_ocr=0;
#endif
                continue;
                }
            if (cmdlineinput_next(cl)==NULL || !stricmp(cl->cmdarg,"t"))
                { 
#ifdef HAVE_OCR_LIB
                if (setvals==1)
                    {
                    k2settings->dst_ocr='t';
                    k2settings->use_crop_boxes=0;
                    }
#endif
                continue;
                }
            if (!stricmp(cl->cmdarg,"g") || !stricmp(cl->cmdarg,"j"))
                {
#ifdef HAVE_OCR_LIB
                if (setvals==1)
                    {
                    k2settings->dst_ocr='g';
                    k2settings->use_crop_boxes=0;
                    }
#endif
                continue;
                }
            /* Internal (MuPDF) OCR */
            if (!stricmp(cl->cmdarg,"i") || !stricmp(cl->cmdarg,"m"))
                {
#if (defined(HAVE_OCR_LIB) && defined(HAVE_MUPDF_LIB))
                if (setvals==1)
                    {
                    k2settings->dst_ocr='m';
                    k2settings->use_crop_boxes=0;
                    }
#endif
                continue;
                }
#ifdef HAVE_OCR_LIB
            if (setvals==1)
                {
#ifdef HAVE_TESSERACT_LIB
                k2settings->dst_ocr='t';
#else
                k2settings->dst_ocr='g';
#endif
                k2settings->use_crop_boxes=0;
                }
#endif
            readnext=0;
            continue;
            }
        if (!stricmp(cl->cmdarg,"-as-"))
            {
            if (setvals==1)
                k2settings->src_autostraighten=-1.;
            continue;
            }
        if (!stricmp(cl->cmdarg,"-as"))
            {
            if (setvals==1)
                k2settings->src_autostraighten=4.;
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (is_a_number(cl->cmdarg))
                {
                if (setvals==1)
                    k2settings->src_autostraighten=atof(cl->cmdarg);
                }
            else
                readnext=0;
            if (k2settings->src_autostraighten > 45.)
                k2settings->src_autostraighten = 45.;
            continue;
            }
        if (!stricmp(cl->cmdarg,"-rt"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                {
                if (!stricmp(cl->cmdarg,"auto+"))
                    k2settings->src_rot=SRCROT_AUTOPREV;
                else if (!stricmp(cl->cmdarg,"auto"))
                    k2settings->src_rot=SRCROT_AUTO;
                else if (!stricmp(cl->cmdarg,"aep"))
                    k2settings->src_rot=SRCROT_AUTOEP;
                else
                    k2settings->src_rot=atoi(cl->cmdarg);
                }
            continue;
            }
        if (!stricmp(cl->cmdarg,"-crgh"))
            {
            if (!next_is_number(cl,setvals==1,quiet,&good,&readnext,&k2settings->column_row_gap_height_in))
                break;
            if (good && setvals==1)
                {
                if (k2settings->column_row_gap_height_in < 0.001)
                    k2settings->column_row_gap_height_in = 0.001;
                }
            continue;
            }
        if (!stricmp(cl->cmdarg,"-cgr"))
            {
            if (!next_is_number(cl,setvals==1,quiet,&good,&readnext,&k2settings->column_gap_range))
                break;
            if (good && setvals==1)
                {
                if (k2settings->column_gap_range < 0.)
                    k2settings->column_gap_range = 0.;
                if (k2settings->column_gap_range > 1.0)
                    k2settings->column_gap_range = 1.0;
                }
            continue;
            }
        if (!stricmp(cl->cmdarg,"-comax"))
            {
            if (!next_is_number(cl,setvals==1,quiet,&good,&readnext,&k2settings->column_offset_max))
                break;
            if (good && setvals==1)
                {
                if (k2settings->column_offset_max > 1.0)
                    k2settings->column_offset_max = 1.0;
                }
            continue;
            }
        if (!stricmp(cl->cmdarg,"-col"))
            {
            if (!next_is_integer(cl,setvals==1,quiet,&good,&readnext,&k2settings->max_columns))
                break;
            if (good && setvals==1)
                {
                if (k2settings->max_columns<1)
                    k2settings->max_columns=1;
                if (k2settings->max_columns>2)
                    k2settings->max_columns=4;
                }
            continue;
            }
        if (!strnicmp(cl->cmdarg,"-jpg",4) || !strnicmp(cl->cmdarg,"-jpeg",5))
            {
            int ic;
            ic = (tolower(cl->cmdarg[3])=='g') ? 4 : 5;
            if (cl->cmdarg[ic]=='-')
                {
                if (setvals==1)
                    k2settings->jpeg_quality=-1;
                }
            else
                {
                if (cmdlineinput_next(cl)==NULL)
                    {
                    if (setvals==1)
                        k2settings->jpeg_quality=90;
                    }
                else if (is_an_integer(cl->cmdarg))
                    {
                    if (setvals==1)
                        k2settings->jpeg_quality=atoi(cl->cmdarg);
                    }
                else
                    {
                    readnext=0;
                    if (setvals==1)
                        k2settings->jpeg_quality=90;
                    }
                }
            if (k2settings->jpeg_quality>100)
                k2settings->jpeg_quality=100;
            continue;
            }
        if (!stricmp(cl->cmdarg,"-bpc"))
            {
            if (!next_is_integer(cl,setvals==1,quiet,&good,&readnext,&k2settings->dst_bpc))
                break;
            if (good && setvals==1)
                {
                if (k2settings->dst_bpc>=6)
                    k2settings->dst_bpc=8;
                else if (k2settings->dst_bpc>=3)
                    k2settings->dst_bpc=4;
                else if (k2settings->dst_bpc<1)
                    k2settings->dst_bpc=1;
                }
            continue;
            }
        if (!stricmp(cl->cmdarg,"-g"))
            {
            if (!next_is_number(cl,setvals==1,quiet,&good,&readnext,&k2settings->dst_gamma))
                break;
            if (good && setvals==1)
                {
                if (k2settings->dst_gamma<.01)
                    k2settings->dst_gamma=.01;
                if (k2settings->dst_gamma>100.)
                    k2settings->dst_gamma=100.;
                }
            continue;
            }
        if (!stricmp(cl->cmdarg,"-gtr"))
            {
            if (!next_is_number(cl,setvals==1,quiet,&good,&readnext,&k2settings->gtr_in))
                break;
            if (good && setvals==1)
                {
                if (k2settings->gtr_in<0.)
                    k2settings->gtr_in=0.;
                }
            continue;
            }
        if (!stricmp(cl->cmdarg,"-gtc"))
            {
            if (!next_is_number(cl,setvals==1,quiet,&good,&readnext,&k2settings->gtc_in))
                break;
            if (good && setvals==1)
                {
                if (k2settings->gtc_in<0.)
                    k2settings->gtc_in=0.;
                }
            continue;
            }
        if (!stricmp(cl->cmdarg,"-gtw"))
            {
            if (!next_is_number(cl,setvals==1,quiet,&good,&readnext,&k2settings->gtw_in))
                break;
            if (good && setvals==1)
                {
                if (k2settings->gtw_in<0.)
                    k2settings->gtw_in=0.;
                }
            continue;
            }
/*
        if (i<argc-1 && !stricmp(cl->cmdarg,"-cd"))
            {
            if (setvals==1)
                {
                cdthresh=atof(argv[++i]);
                if (cdthresh<0.)
                    cdthresh=0.;
                else if (cdthresh>100.)
                    cdthresh=100.;
                }
            else
                i++;
            continue;
            }
*/
        if (!stricmp(cl->cmdarg,"-ds"))
            {
            if (!next_is_number(cl,setvals==1,quiet,&good,&readnext,NULL))
                break;
            if (good && setvals==1 && atof(cl->cmdarg)>0.)
                k2settings->document_scale_factor=atof(cl->cmdarg);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-idpi"))
            {
            if (!next_is_number(cl,setvals==1,quiet,&good,&readnext,NULL))
                break;
            if (good && setvals==1 && atof(cl->cmdarg)!=0.)
                k2settings->user_src_dpi=atof(cl->cmdarg);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-jf"))
            {
            if (!next_is_integer(cl,setvals==1,quiet,&good,&readnext,&k2settings->dst_figure_justify))
                break;
            if (!next_is_number(cl,setvals==1,1,&good,&readnext,&k2settings->dst_min_figure_height_in))
                break;
            continue;
            }
        if (!stricmp(cl->cmdarg,"-j"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                {
                k2settings->dst_justify=atoi(cl->cmdarg);
                if (in_string(cl->cmdarg,"+")>=0)
                    k2settings->dst_fulljustify=1;
                else if (in_string(&cl->cmdarg[1],"-")>=0)
                    k2settings->dst_fulljustify=0;
                else
                    k2settings->dst_fulljustify=-1;
                }
            continue;
            }
        if (!stricmp(cl->cmdarg,"-h"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                set_value_with_units(cl->cmdarg,&k2settings->dst_userheight,&k2settings->dst_userheight_units,UNITS_PIXELS);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-wt"))
            {
            if (!next_is_integer(cl,setvals==1,quiet,&good,&readnext,&k2settings->src_whitethresh))
                break;
            if (good && setvals==1)
                {
                if (k2settings->src_whitethresh>255)
                    k2settings->src_whitethresh=255;
                }
            continue;
            }
        if (!stricmp(cl->cmdarg,"-w"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                set_value_with_units(cl->cmdarg,&k2settings->dst_userwidth,&k2settings->dst_userwidth_units,UNITS_PIXELS);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-om"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                {
                double v[4];
                int na;
                na=string_read_doubles(cl->cmdarg,v,4);
                if (na>=1)
                    k2settings->dst_mar=k2settings->dst_marleft=k2settings->dst_martop=k2settings->dst_marright=k2settings->dst_marbot=v[0];
                if (na>=2)
                    k2settings->dst_martop=k2settings->dst_marright=k2settings->dst_marbot=v[1];
                if (na>=3)
                    k2settings->dst_marright=k2settings->dst_marbot=v[2];
                if (na>=4)
                    k2settings->dst_marbot=v[3];
                }
            continue;
            }
        if (!stricmp(cl->cmdarg,"-m"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                {
                double v[4];
                int na;
                na=string_read_doubles(cl->cmdarg,v,4);
                if (na>=1)
                    k2settings->mar_left=k2settings->mar_top=k2settings->mar_right=k2settings->mar_bot=v[0];
                if (na>=2)
                    k2settings->mar_top=k2settings->mar_right=k2settings->mar_bot=v[1];
                if (na>=3)
                    k2settings->mar_right=k2settings->mar_bot=v[2];
                if (na>=4)
                    k2settings->mar_bot=v[3];
                }
            continue;
            }
        if (!strnicmp(cl->cmdarg,"-cbox",5))
            {
            char buf[256];

            if (cl->cmdarg[5]=='-' && cl->cmdarg[6]=='\0')
                {
                if (setvals==1)
                    k2settings->cropboxes.n=0;
                continue;
                }
            strncpy(buf,&cl->cmdarg[5],255);
            buf[255]='\0';
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                {
                int na,index,k;

                if (k2settings->cropboxes.n>=MAXK2CROPBOXES)
                    {
                    static int warned=0;
                    if (!warned && !quiet)
                        {
                        k2printf(TTEXT_WARN "\a\n** Max crop boxes exceeded (max=%d). **\n\n",
                                 MAXK2CROPBOXES);
                        k2printf(TTEXT_WARN "\a\n** Crop box %s and subsequent ignored. **\n\n",
                                 cl->cmdarg);
                        }
                    warned=1;
                    continue;
                    }
                index=k2settings->cropboxes.n;
                strcpy(k2settings->cropboxes.cropbox[index].pagelist,buf);
                k2settings->cropboxes.cropbox[index].box[0]=0.;
                k2settings->cropboxes.cropbox[index].box[1]=0.;
                k2settings->cropboxes.cropbox[index].box[2]=-1.;
                k2settings->cropboxes.cropbox[index].box[3]=-1.;
                for (na=0,k=0;na<4;na++,k++)
                    {
                    int c,m;
                    
                    for (m=k;cl->cmdarg[k]!=',' && cl->cmdarg[k]!='\0';k++);
                    c=cl->cmdarg[k];
                    cl->cmdarg[k]='\0';
                    if (k>m)
                        set_value_with_units(&cl->cmdarg[m],
                                         &k2settings->cropboxes.cropbox[index].box[na],
                                         &k2settings->cropboxes.cropbox[index].units[na],
                                         UNITS_INCHES);
                    if (c=='\0')
                        break;
                    }
                if (na==0 || na==2)
                    {
                    if (!quiet)
                        k2printf(TTEXT_WARN "\a\n** Crop box %s is invalid and will be ignored. **\n\n"
                        TTEXT_NORMAL,cl->cmdarg);
                    }
                else
                    k2settings->cropboxes.n++;
                }
            continue;
            }
        if (!stricmp(cl->cmdarg,"-pad"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                {
                double v[4];
                int na;
                na=string_read_doubles(cl->cmdarg,v,4);
                if (na>=1)
                    k2settings->pad_left=k2settings->pad_top=k2settings->pad_right=k2settings->pad_bottom=v[0];
                if (na>=2)
                    k2settings->pad_top=k2settings->pad_right=k2settings->pad_bottom=v[1];
                if (na>=3)
                    k2settings->pad_right=k2settings->pad_bottom=v[2];
                if (na>=4)
                    k2settings->pad_bottom=v[3];
                }
            continue;
            }
        if (!strnicmp(cl->cmdarg,"-hq",3))
            {
            if (setvals==1)
                continue;
            if (cl->cmdarg[3]=='-')
                {
                k2settings->dst_dpi=167;
                k2settings->user_src_dpi = -2.0;
                k2settings->dst_userwidth=DEFAULT_WIDTH;
                k2settings->dst_userwidth_units=UNITS_PIXELS;
                k2settings->dst_userheight=DEFAULT_HEIGHT;
                k2settings->dst_userheight_units=UNITS_PIXELS;
                }
            else
                {
                k2settings->dst_dpi=333;
                k2settings->user_src_dpi = -2.0;
                k2settings->dst_userwidth=DEFAULT_WIDTH*2;
                k2settings->dst_userheight=DEFAULT_HEIGHT*2;
                k2settings->dst_userwidth_units=UNITS_PIXELS;
                k2settings->dst_userheight_units=UNITS_PIXELS;
                }
            continue;
            }
        if (!stricmp(cl->cmdarg,"-debug"))
            {
            if (setvals==1)
                k2settings->debug=1;
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (is_an_integer(cl->cmdarg))
                {
                if (setvals==1)
                    k2settings->debug=atoi(cl->cmdarg);
                }
            else
                readnext=0;
            continue;
            }
        NEEDS_STRING("-toclist",toclist,2047);
        NEEDS_STRING("-tocsave",tocsavefile,MAXFILENAMELEN-1);
        NEEDS_STRING("-bpl",bpl,2047);
        NEEDS_STRING("-p",pagelist,1023)
#ifdef HAVE_OCR_LIB
        NEEDS_STRING("-ocrout",ocrout,127)
#endif
        NEEDS_STRING("-o",dst_opname_format,127)
        NEEDS_INTEGER("-evl",erase_vertical_lines)
        NEEDS_VALUE("-vls",vertical_line_spacing)
        NEEDS_VALUE("-vs",max_vertical_gap_inches)
        NEEDS_VALUE("-de",defect_size_pts)
        NEEDS_VALUE("-vb",vertical_break_threshold)
        NEEDS_VALUE("-cg",min_column_gap_inches)
        NEEDS_VALUE("-cgmax",max_column_gap_inches)
        NEEDS_VALUE("-rsf",row_split_fom)
        NEEDS_VALUE("-cmax",contrast_max)
        NEEDS_VALUE("-ch",min_column_height_inches)
        NEEDS_VALUE("-dr",dst_display_resolution)
        NEEDS_INTEGER("-odpi",dst_dpi)
        NEEDS_INTEGER("-dpi",dst_dpi)
        NEEDS_VALUE("-ws",word_spacing)
        if (k2settings->word_spacing < 0)
            {
            k2settings->word_spacing = -k2settings->word_spacing;
            k2settings->auto_word_spacing = 1;
            }
        else
            k2settings->auto_word_spacing = 0;
        NEEDS_VALUE("-omb",dst_marbot)
        NEEDS_VALUE("-omt",dst_martop)
        NEEDS_VALUE("-omr",dst_marright)
        NEEDS_VALUE("-oml",dst_marleft)
        NEEDS_VALUE("-mb",mar_bot)
        NEEDS_VALUE("-mt",mar_top)
        NEEDS_VALUE("-mr",mar_right)
        NEEDS_VALUE("-ml",mar_left)
        NEEDS_INTEGER("-pb",pad_bottom)
        NEEDS_INTEGER("-pr",pad_right)
        NEEDS_INTEGER("-pl",pad_left)
        NEEDS_INTEGER("-pt",pad_top)
        /*
        ** UNDOCUMENTED COMMAND-LINE ARGS
        */
        NEEDS_VALUE("-whmax",no_wrap_height_limit_inches)
        NEEDS_VALUE("-arlim",no_wrap_ar_limit)
        NEEDS_VALUE("-rwmin",little_piece_threshold_inches)

        /* Add command arg to file list */
        k2pdfopt_files_add_file(&k2conv->k2files,cl->cmdarg);
        }
    strbuf_free(allopts);
    return(k2conv->k2files.n);
    }


static void set_value_with_units(char *s,double *val,int *units,int defunits)

    {
    int i;

    (*val)=atof(s);
    for (i=0;valid_numerical_char(s[i]);i++);
    if (tolower(s[i])=='i')
        (*units)=UNITS_INCHES;
    else if (tolower(s[i])=='c')
        (*units)=UNITS_CM;
    else if (tolower(s[i])=='s')
        (*units)=UNITS_SOURCE;
    else if (tolower(s[i])=='t')
        (*units)=UNITS_TRIMMED;
    else if (tolower(s[i])=='p')
        (*units)=UNITS_PIXELS;
    else
        (*units)=defunits;
    }


static int valid_numerical_char(int c)

    {
    return((c>='0' && c<='9') || c=='+' || c=='-' || c=='.' || tolower(c)=='e' || tolower(c)=='d');
    }


/*
** Return zero if no more args.
** If arg is not a number, (*readnext)=0.
** If arg is a number and setvals is nz, (*dstval) gets the number.
*/
static int next_is_number(CMDLINEINPUT *cl,int setvals,int quiet,int *good,int *readnext,double *dstval)

    {
    char buf[64];

    strncpy(buf,cl->cmdarg,63);
    buf[63]='\0';
    (*good)=0;
    if (cmdlineinput_next(cl)==NULL)
        return(0);
    if (!is_a_number(cl->cmdarg))
        {
        if (!quiet && setvals)
            k2printf(TTEXT_WARN "\a%s expects a value.  Arg '%s' not used for %s." TTEXT_NORMAL "\n",buf,cl->cmdarg,buf);
        (*readnext)=0;
        }
    else
        {
        (*good)=1;
        if (setvals && dstval!=NULL)
            (*dstval)=atof(cl->cmdarg);
        }
    return(1);
    }


/*
** Return zero if no more args.
** If arg is not an integer, (*readnext)=0.
** If arg is an integer and setvals is nz, (*dstval) gets the number.
*/
static int next_is_integer(CMDLINEINPUT *cl,int setvals,int quiet,int *good,int *readnext,int *dstval)

    {
    char buf[64];

    strncpy(buf,cl->cmdarg,63);
    buf[63]='\0';
    (*good)=0;
    if (cmdlineinput_next(cl)==NULL)
        return(0);
    if (!is_an_integer(cl->cmdarg))
        {
        if (!quiet && setvals)
            k2printf(TTEXT_WARN "\a%s expects an integer.  Arg '%s' not used for %s." TTEXT_NORMAL "\n",buf,cl->cmdarg,buf);
        (*readnext)=0;
        }
    else
        {
        (*good)=1;
        if (setvals && dstval!=NULL)
            (*dstval)=atoi(cl->cmdarg);
        }
    return(1);
    }
