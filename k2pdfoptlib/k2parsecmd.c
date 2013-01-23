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

static void set_value_with_units(char *s,double *val,int *units);
static int valid_numerical_char(int c);

/*
** Return file count
** setvals==1 to set all values based on options
**        ==2 to set only ansi, user interface, exit on complete
**        ==0 to not set any values
** procfiles == 1 to process files
**           == 0 to count files only
*/
int parse_cmd_args(K2PDFOPT_SETTINGS *k2settings,STRBUF *env,STRBUF *cmdline,
                   STRBUF *usermenu,int setvals,int procfiles)

    {
    CMDLINEINPUT _cl,*cl;
    STRBUF *allopts,_allopts;
    int filecount,readnext;

    allopts=&_allopts;
    strbuf_init(allopts);
    strbuf_cpy(allopts,env->s);
    strbuf_cat(allopts,cmdline->s);
    strbuf_cat(allopts,usermenu->s);
    cl=&_cl;
    filecount=0;
    cmdlineinput_init(cl,0,NULL,allopts->s);
    readnext=1;
    while (1)
        {
        if (readnext && cmdlineinput_next(cl)==NULL)
            break;
        readnext=1;
        if (!stricmp(cl->cmdarg,"-?") || !stricmp(cl->cmdarg,"-?-"))
            {
            if (setvals==2)
                k2settings->show_usage = cl->cmdarg[2]=='-' ? 0 : 1;
            continue;
            }
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
        if (!stricmp(cl->cmdarg,"-evl"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                k2settings->erase_vertical_lines=atoi(cl->cmdarg);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-vls"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                k2settings->vertical_line_spacing=atof(cl->cmdarg);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-vm"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                {
                k2settings->vertical_multiplier=fabs(atof(cl->cmdarg));
                if (k2settings->vertical_multiplier < 0.1)
                    k2settings->vertical_multiplier = 0.1;
                }
            continue;
            }
        if (!stricmp(cl->cmdarg,"-vs"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                k2settings->max_vertical_gap_inches=atof(cl->cmdarg);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-de"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                k2settings->defect_size_pts=atof(cl->cmdarg);
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
                    aprintf(TTEXT_WARN "\aDevice profile '%s' not known." TTEXT_NORMAL "\n",cl->cmdarg);
                }
            continue;
            }
        if (!stricmp(cl->cmdarg,"-pi") || !stricmp(cl->cmdarg,"-pi-"))
            {
            if (setvals==1)
                k2settings->preserve_indentation=(cl->cmdarg[3]=='-') ? 0 : 1;
            continue;
            }
        if (!strnicmp(cl->cmdarg,"-wrap",5))
            {
            if (setvals==1)
                {
                k2settings->text_wrap=(cl->cmdarg[5]=='-') ? 0 : (cl->cmdarg[5]=='+' ? 2 : 1);
                if (k2settings->text_wrap)
                    k2settings->use_crop_boxes=0;
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
                }
            continue;
            }
#endif
        if (!stricmp(cl->cmdarg,"-neg") || !stricmp(cl->cmdarg,"-neg-"))
            {
            if (setvals==1)
                k2settings->dst_negative=(cl->cmdarg[4]=='-') ? 0 : 1;
            continue;
            }
        if (!stricmp(cl->cmdarg,"-r") || !stricmp(cl->cmdarg,"-r-"))
            {
            if (setvals==1)
                k2settings->src_left_to_right=(cl->cmdarg[2]=='-') ? 1 : 0;
            continue;
            }
        if (!strnicmp(cl->cmdarg,"-hy",3))
            {
            if (setvals==1)
                k2settings->hyphen_detect=(cl->cmdarg[3]=='-') ? 0 : 1;
            continue;
            }
        if (!strnicmp(cl->cmdarg,"-ls",3))
            {
            if (setvals==1)
                k2settings->dst_landscape=(cl->cmdarg[3]=='-') ? 0 : 1;
            continue;
            }
        if (!stricmp(cl->cmdarg,"-mode"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                {
                if (!stricmp(cl->cmdarg,"pdfr") 
                          || !stricmp(cl->cmdarg,"copy"))
                    {
                    /* -n- -wrap- -col 1 -vb -2 -w -1 -h -1 -dpi 150 -rt 0 -c -t- -f2p -2 */
                    /* -m 0 -om 0 -pl 0 -pr 0 -pt 0 -pb 0 -mc- */
                    k2settings->use_crop_boxes=0;
                    k2settings->text_wrap=0;
                    k2settings->max_columns=1;
                    k2settings->vertical_break_threshold=-2;
                    k2settings->dst_userwidth=-1.0;
                    k2settings->dst_userwidth_units=UNITS_PIXELS;
                    k2settings->dst_userheight=-1.0;
                    k2settings->dst_userheight_units=UNITS_PIXELS;
                    k2settings->dst_dpi=150;
                    k2settings->src_rot=0.;
                    k2settings->dst_color=1;
                    k2settings->src_trim=0;
                    k2settings->dst_fit_to_page=-2;
                    k2settings->mar_left=k2settings->mar_top=k2settings->mar_right=k2settings->mar_bot=0.;
                    k2settings->dst_mar=k2settings->dst_marleft=k2settings->dst_martop=k2settings->dst_marright=k2settings->dst_marbot=0.;
                    k2settings->pad_left=k2settings->pad_top=k2settings->pad_bottom=k2settings->pad_right=0;
                    k2settings->mark_corners=0;
                    }
                else if (!stricmp(cl->cmdarg,"fw") 
                          || !stricmp(cl->cmdarg,"sopdf")
                          || !stricmp(cl->cmdarg,"fitwidth"))
                    {
                    /* -wrap- -col 1 -vb -2 -t -ls */
                    k2settings->use_crop_boxes=1;
                    k2settings->text_wrap=0;
                    k2settings->max_columns=1;
                    k2settings->vertical_break_threshold=-2;
                    k2settings->src_trim=1;
                    k2settings->dst_landscape=1;
                    }
                else if (!stricmp(cl->cmdarg,"2col")
                          || !stricmp(cl->cmdarg,"col2"))
                    {
                    k2settings->use_crop_boxes=1;
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
                    k2settings->use_crop_boxes=1;
                    k2settings->text_wrap=1;
                    k2settings->max_columns=2;
                    k2settings->vertical_break_threshold=1.75;
                    k2settings->src_rot=SRCROT_AUTO;
                    k2settings->src_trim=1;
                    k2settings->dst_fit_to_page=0;
                    k2settings->mar_left=k2settings->mar_top=k2settings->mar_right=k2settings->mar_bot=0.25;
                    k2settings->dst_mar=k2settings->dst_marleft=k2settings->dst_martop=k2settings->dst_marright=k2settings->dst_marbot=0.02;
                    }
                else
                    aprintf(TTEXT_WARN "\a\n** Unknown mode:  %s **\n\n" TTEXT_NORMAL,
                             cl->cmdarg);
                }
            continue;
            }
        if (!stricmp(cl->cmdarg,"-o"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                {
                strncpy(k2settings->dst_opname_format,cl->cmdarg,127);
                k2settings->dst_opname_format[127]='\0';
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
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                {
                k2settings->dst_fit_to_page=atoi(cl->cmdarg);
                if (k2settings->dst_fit_to_page == -2)
                    k2settings->vertical_break_threshold=-2.;
                }
            continue;
            }
        if (!stricmp(cl->cmdarg,"-vb"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                k2settings->vertical_break_threshold=atof(cl->cmdarg);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-sm") || !stricmp(cl->cmdarg,"-sm-"))
            {
            if (setvals==1)
                k2settings->show_marked_source=(cl->cmdarg[3]=='-' ? 0 : 1);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-bp") || !stricmp(cl->cmdarg,"-bp-"))
            {
            if (cl->cmdarg[3]=='-')
                {
                if (setvals==1)
                    k2settings->dst_break_pages=0;
                continue;
                }
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (is_a_number(cl->cmdarg))
                {
                if (setvals==1)
                    k2settings->dst_break_pages= -1 - (int)(atof(cl->cmdarg)*1000.+.5);
                }
            else
                {
                if (setvals==1)
                    k2settings->dst_break_pages=1;
                readnext=0;
                }
            continue;
            }
        if (!strnicmp(cl->cmdarg,"-fc",3))
            {
            if (setvals==1)
                k2settings->fit_columns=(cl->cmdarg[3]=='-') ? 0 : 1;
            continue;
            }
        if (!stricmp(cl->cmdarg,"-d") || !stricmp(cl->cmdarg,"-d-"))
            {
            if (setvals==1)
                k2settings->dst_dither=(cl->cmdarg[2]=='-') ? 0 : 1;
            continue;
            }
        if (!stricmp(cl->cmdarg,"-c") || !stricmp(cl->cmdarg,"-c-"))
            {
            if (setvals==1)
                {
                k2settings->dst_color=(cl->cmdarg[2]=='-') ? 0 : 1;
                /* wrapbmp_set_color(k2settings->dst_color); */
                }
            continue;
            }
        if (!strnicmp(cl->cmdarg,"-v",2))
            {
            if (setvals==1)
                k2settings->verbose=(cl->cmdarg[2]=='-') ? 0 : 1;
            continue;
            }
        if (!strnicmp(cl->cmdarg,"-png",4))
            {
            if (setvals==1)
                k2settings->jpeg_quality=(cl->cmdarg[4]=='-') ? 90 : -1;
            continue;
            }
        if (!strnicmp(cl->cmdarg,"-mc",3))
            {
            if (setvals==1)
                k2settings->mark_corners=(cl->cmdarg[3]=='-') ? 0 : 1;
            continue;
            }
        if (!stricmp(cl->cmdarg,"-ocrlang") || !stricmp(cl->cmdarg,"-l"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
#ifdef HAVE_TESSERACT_LIB
            strncpy(k2settings->dst_ocr_lang,cl->cmdarg,15);
            k2settings->dst_ocr_lang[15]='\0';
#endif
            continue;
            }
        if (!stricmp(cl->cmdarg,"-ocrvis"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
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
        if (!stricmp(cl->cmdarg,"-ocrhmax"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
#ifdef HAVE_OCR_LIB
            if (setvals==1)
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
                if (!warned)
                aprintf(TTEXT_WARN "\a\n** No OCR capability in this compile of k2pdfopt! **\n\n"
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
        if (!stricmp(cl->cmdarg,"-t") || !stricmp(cl->cmdarg,"-t-"))
            {
            if (setvals==1)
                k2settings->src_trim=(cl->cmdarg[2]=='-') ? 0 : 1;
            continue;
            }
        if (!stricmp(cl->cmdarg,"-s") || !stricmp(cl->cmdarg,"-s-"))
            {
            if (setvals==1)
                k2settings->dst_sharpen=(cl->cmdarg[2]=='-') ? 0 : 1;
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
                if (!stricmp(cl->cmdarg,"auto"))
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
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                {
                k2settings->column_row_gap_height_in=atof(cl->cmdarg);
                if (k2settings->column_row_gap_height_in < 0.001)
                    k2settings->column_row_gap_height_in = 0.001;
                }
            continue;
            }
        if (!stricmp(cl->cmdarg,"-cgr"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                {
                k2settings->column_gap_range=atof(cl->cmdarg);
                if (k2settings->column_gap_range < 0.)
                    k2settings->column_gap_range = 0.;
                if (k2settings->column_gap_range > 1.0)
                    k2settings->column_gap_range = 1.0;
                }
            continue;
            }
        if (!stricmp(cl->cmdarg,"-comax"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                {
                k2settings->column_offset_max=atof(cl->cmdarg);
                if (k2settings->column_offset_max > 1.0)
                    k2settings->column_offset_max = 1.0;
                }
            continue;
            }
        if (!stricmp(cl->cmdarg,"-col"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                {
                k2settings->max_columns=atoi(cl->cmdarg);
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
        if (!stricmp(cl->cmdarg,"-col"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                {
                k2settings->max_columns=atoi(cl->cmdarg);
                if (k2settings->max_columns<1)
                    k2settings->max_columns=1;
                if (k2settings->max_columns>2)
                    k2settings->max_columns=4;
                }
            continue;
            }
        if (!stricmp(cl->cmdarg,"-p"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                {
                strncpy(k2settings->pagelist,cl->cmdarg,1023);
                k2settings->pagelist[1023]='\0';
                }
            continue;
            }
        if (!stricmp(cl->cmdarg,"-bpc"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                {
                k2settings->dst_bpc=atoi(cl->cmdarg);
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
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                {
                k2settings->dst_gamma=atof(cl->cmdarg);
                if (k2settings->dst_gamma<.01)
                    k2settings->dst_gamma=.01;
                if (k2settings->dst_gamma>100.)
                    k2settings->dst_gamma=100.;
                }
            continue;
            }
        if (!stricmp(cl->cmdarg,"-cg"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                k2settings->min_column_gap_inches=atof(cl->cmdarg);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-cgmax"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                k2settings->max_column_gap_inches=atof(cl->cmdarg);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-gtr"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                {
                k2settings->gtr_in=atof(cl->cmdarg);
                if (k2settings->gtr_in<0.)
                    k2settings->gtr_in=0.;
                }
            continue;
            }
        if (!stricmp(cl->cmdarg,"-gtc"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                {
                k2settings->gtc_in=atof(cl->cmdarg);
                if (k2settings->gtc_in<0.)
                    k2settings->gtc_in=0.;
                }
            continue;
            }
        if (!stricmp(cl->cmdarg,"-gtw"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                {
                k2settings->gtw_in=atof(cl->cmdarg);
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
        if (!stricmp(cl->cmdarg,"-cmax"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                k2settings->contrast_max=atof(cl->cmdarg);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-ch"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                k2settings->min_column_height_inches=atof(cl->cmdarg);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-ds"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1 && atof(cl->cmdarg)>0.)
                k2settings->document_scale_factor=atof(cl->cmdarg);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-idpi"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1 && atof(cl->cmdarg)!=0.)
                k2settings->user_src_dpi=atof(cl->cmdarg);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-odpi") || !stricmp(cl->cmdarg,"-dpi"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                k2settings->dst_dpi=atoi(cl->cmdarg);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-jf"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                k2settings->dst_figure_justify=atoi(cl->cmdarg);
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (!is_a_number(cl->cmdarg))
                {
                readnext=0;
                continue;
                }
            if (setvals==1)
                k2settings->dst_min_figure_height_in=atof(cl->cmdarg);
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
        if (!stricmp(cl->cmdarg,"-dr"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                k2settings->dst_display_resolution=atof(cl->cmdarg);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-h"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                set_value_with_units(cl->cmdarg,&k2settings->dst_userheight,&k2settings->dst_userheight_units);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-ws"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                k2settings->word_spacing=atof(cl->cmdarg);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-wt"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                {
                k2settings->src_whitethresh=atoi(cl->cmdarg);
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
                set_value_with_units(cl->cmdarg,&k2settings->dst_userwidth,&k2settings->dst_userwidth_units);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-omb"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                k2settings->dst_marbot=atof(cl->cmdarg);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-omt"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                k2settings->dst_martop=atof(cl->cmdarg);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-omr"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                k2settings->dst_marright=atof(cl->cmdarg);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-oml"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                k2settings->dst_marleft=atof(cl->cmdarg);
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
        if (!stricmp(cl->cmdarg,"-mb"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                k2settings->mar_bot=atof(cl->cmdarg);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-mt"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                k2settings->mar_top=atof(cl->cmdarg);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-mr"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                k2settings->mar_right=atof(cl->cmdarg);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-ml"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                k2settings->mar_left=atof(cl->cmdarg);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-pb"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                k2settings->pad_bottom=atoi(cl->cmdarg);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-pt"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                k2settings->pad_top=atoi(cl->cmdarg);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-pr"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                k2settings->pad_right=atoi(cl->cmdarg);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-pl"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                k2settings->pad_left=atoi(cl->cmdarg);
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
        /*
        ** UNDOCUMENTED COMMAND-LINE ARGS
        */
        if (!stricmp(cl->cmdarg,"-whmax"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                k2settings->no_wrap_height_limit_inches=atof(cl->cmdarg);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-arlim"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                k2settings->no_wrap_ar_limit=atof(cl->cmdarg);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-rwmin"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                k2settings->little_piece_threshold_inches=atof(cl->cmdarg);
            continue;
            }
        filecount++;
        /*
        if (filecount==1 && firstfile!=NULL)
            {
            strncpy(firstfile,cl->cmdarg,255);
            firstfile[255]='\0';
            }
        */
        if (procfiles)
            k2pdfopt_proc_wildarg(k2settings,cl->cmdarg);
        }
    strbuf_free(allopts);
    return(filecount);
    }


static void set_value_with_units(char *s,double *val,int *units)

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
    else
        (*units)=UNITS_PIXELS;
    }


static int valid_numerical_char(int c)

    {
    return((c>='0' && c<='9') || c=='+' || c=='-' || c=='.' || tolower(c)=='e' || tolower(c)=='d');
    }


