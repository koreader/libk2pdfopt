/*
** k2cmdparse.c   Parse command-line options for k2pdfopt.
**
** Copyright (C) 2018  http://willus.com
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

static int valid_numerical_char(int c);
static int next_is_number(CMDLINEINPUT *cl,int setvals,int quiet,int *good,int *readnext,double *dstval);
static int next_is_integer(CMDLINEINPUT *cl,int setvals,int quiet,int *good,int *readnext,int *dstval);


#define CBOXVAL(x,cbox,cboxindex,defunits,srcmar) if (!stricmp(cl->cmdarg,x)) { \
                                  if (cmdlineinput_next(cl)==NULL) \
                                      break; \
                                  if (srcmar && is_a_number(cl->cmdarg) && atof(cl->cmdarg)<0.) \
                                      { \
                                      cbox.box[cboxindex]=fabs(atof(cl->cmdarg)); \
                                      cbox.units[cboxindex]=UNITS_SOURCE; \
                                      } \
                                  else \
                                      k2parsecmd_set_value_with_units(cl->cmdarg, \
                                         &cbox.box[cboxindex], \
                                         &cbox.units[cboxindex], \
                                         defunits); \
                                  if (!srcmar && (cbox.units[cboxindex]==UNITS_TRIMMED \
                                                   || cbox.units[cboxindex]==UNITS_OCRLAYER)) \
                                      cbox.units[cboxindex]=UNITS_SOURCE; \
                                  continue; \
                                  }

#define NEEDS_VALUE(x,y) if (!stricmp(cl->cmdarg,x)) { \
                         if (!next_is_number(cl,setvals==1,quiet,&good,&readnext,&k2settings->y)) \
                             break; \
                         continue; }

#define NEEDS_INTEGER(x,y) if (!stricmp(cl->cmdarg,x)) { \
                        if (!next_is_integer(cl,setvals==1,quiet,&good,&readnext,&k2settings->y)) \
                              break; \
                        continue; }
/* v2.33 */
#define PLUS_MINUS_OPTION(x,y,nomval,plusval,minusval,sv) if (!stricmp(cl->cmdarg,x) || !stricmp(cl->cmdarg,x "-") || !stricmp(cl->cmdarg,x "+")) \
            { \
            if (setvals==sv) \
                { \
                if (cl->cmdarg[strlen(cl->cmdarg)-1]=='-') \
                    k2settings->y = minusval; \
                else if (cl->cmdarg[strlen(cl->cmdarg)-1]=='+') \
                    k2settings->y = plusval; \
                else \
                    k2settings->y = nomval; \
                } \
            continue; \
            }
/* Fixed to recognize + option, v2.22 */
#define PLUS_MINUS_BITOPTION(x,y,orval,plusval,sv) if (!stricmp(cl->cmdarg,x) || !stricmp(cl->cmdarg,x "-") || !stricmp(cl->cmdarg,x "+")) \
            { \
            if (setvals==sv) \
                { \
                if (cl->cmdarg[strlen(cl->cmdarg)-1]=='-') \
                    k2settings->y &= ~(orval|plusval); \
                else if (cl->cmdarg[strlen(cl->cmdarg)-1]=='+') \
                    k2settings->y |= plusval; \
                else \
                    k2settings->y |= orval; \
                } \
            continue; \
            }
#define MINUS_BITOPTION(x,y,orval,sv) if (!stricmp(cl->cmdarg,x) || !stricmp(cl->cmdarg,x "-")) \
            { \
            if (setvals==sv) \
                { \
                if (cl->cmdarg[strlen(cl->cmdarg)-1]=='-') \
                    k2settings->y &= ~orval; \
                else \
                    k2settings->y |= orval; \
                } \
            continue; \
            }
#define MINUS_OPTION(x,y,sv) if (!stricmp(cl->cmdarg,x) || !stricmp(cl->cmdarg,x "-")) \
            { \
            if (setvals==sv) \
                k2settings->y=(cl->cmdarg[strlen(cl->cmdarg)-1]=='-' ? 0 : 1); \
            continue; \
            }
#define NEEDS_STRING(x,y,maxlen,minus_clears) if (!stricmp(cl->cmdarg,x)) { \
            if (cmdlineinput_next(cl)==NULL) \
                break; \
            if (setvals==1) \
                { \
                strncpy(k2settings->y,cl->cmdarg,maxlen); \
                k2settings->y[maxlen]='\0'; \
                } \
            continue; \
            } \
            if (minus_clears && !stricmp(cl->cmdarg,x "-")) { \
            if (setvals==1) \
                k2settings->y[0]='\0'; \
            continue; \
            }
#define NEEDS_VALUE_PLUS(x,y) if (!stricmp(cl->cmdarg,x)) { \
            if (cmdlineinput_next(cl)==NULL) \
                break; \
            if (setvals==1) \
                { \
                char buf[64]; \
                strncpy(buf,cl->cmdarg,63);\
                buf[63]='\0'; \
                if (strlen(buf)>1 && buf[strlen(buf)-1]=='+') \
                    { \
                    buf[strlen(buf)-1]='\0'; \
                    k2settings->y=-atof(buf); \
                    } \
                else \
                    k2settings->y=atof(buf); \
                } \
            continue; \
            } 

/*
** Return file count
** NEW BEHAVIOR (v1.65 and up):
** setvals==1 to set all values based on options
**        ==2 to set only ansi, user interface, exit on complete
**            (also still sets and counts files.)
**        ==3 to test if should restore last settings.
**            returns 0 if there are cmd args (do not restore last settings, GUI only)
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
#ifdef HAVE_K2GUI
    int argcheck;
#endif

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
#ifdef HAVE_K2GUI
    argcheck=0;
#endif
    while (1)
        {
        if (readnext && cmdlineinput_next(cl)==NULL)
            break;
        readnext=1;
#ifdef HAVE_K2GUI
        /* v2.20 */
        if (setvals==3 && argcheck)
            return(0);
        /* Re-launch code */
        if (!stricmp(cl->cmdarg,"-gui+"))
            {
            if (setvals==2)
                k2settings->gui = 2;
            continue;
            }
        MINUS_OPTION("-gui",gui,2)
        MINUS_OPTION("-guimin",guimin,2)
        if (!stricmp(cl->cmdarg,"-rls+"))
            {
            if (setvals==2)
                k2settings->restore_last_settings = 1;
            continue;
            }
        if (!stricmp(cl->cmdarg,"-rls-"))
            {
            if (setvals==2)
                k2settings->restore_last_settings = 0;
            continue;
            }
        if (!stricmp(cl->cmdarg,"-rls"))
            {
            if (setvals==2)
                k2settings->restore_last_settings = -1;
            continue;
            }
        /* v2.20 */
        argcheck=1;
#endif
        PLUS_MINUS_OPTION("-jfc",join_figure_captions,1,2,0,1)
        MINUS_OPTION("-i",info,1)
        MINUS_OPTION("-toc",use_toc,1)
        MINUS_OPTION("-sp",echo_source_page_count,1)
        MINUS_OPTION("-neg",dst_negative,1)
        PLUS_MINUS_OPTION("-neg",dst_negative,1,2,0,1)
        MINUS_OPTION("-hy",hyphen_detect,1)
        MINUS_OPTION("-sm",show_marked_source,1)
        MINUS_OPTION("-fc",fit_columns,1)
        MINUS_OPTION("-d",dst_dither,1)
        MINUS_OPTION("-c",dst_color,1)
        MINUS_OPTION("-v",verbose,1)
        MINUS_OPTION("-mc",mark_corners,1)
        MINUS_OPTION("-t",src_trim,1)
        MINUS_OPTION("-s",dst_sharpen,1)
        MINUS_OPTION("-to",text_only,1)
        MINUS_OPTION("-fr",dst_figure_rotate,1)
        MINUS_OPTION("-y",assume_yes,1)
#ifdef HAVE_GHOSTSCRIPT
        MINUS_OPTION("-ppgs",ppgs,1)
#endif
#ifdef HAVE_OCR_LIB
        MINUS_BITOPTION("-ocrsort",dst_ocr_visibility_flags,32,1)
        PLUS_MINUS_BITOPTION("-ocrsp",dst_ocr_visibility_flags,8,16,1)
#endif
        /*
        MINUS_OPTION("-pi",preserve_indentation,1)
        */

        if (!stricmp(cl->cmdarg,"-?") || !stricmp(cl->cmdarg,"-?-"))
            {
            if (cl->cmdarg[2]=='\0')
                {
                if (setvals==2)
                    strcpy(k2settings->show_usage,"*");
                if (cmdlineinput_next(cl)==NULL)
                    break;
                if (setvals==2)
                    {
                    strncpy(k2settings->show_usage,cl->cmdarg,31);
                    k2settings->show_usage[31]='\0';
                    }
                }
            else if (setvals==2)
                k2settings->show_usage[0]='\0';
            continue;
            } 

        /* New in 2.32:  -ls can have a page list specified. */
        if (!strnicmp(cl->cmdarg,"-ls",3))
            {
            if (setvals==1)
                {
                int ipl;

                if (cl->cmdarg[3]=='-')
                    {
                    k2settings->dst_landscape = 0;
                    ipl=4;
                    }
                else
                    {
                    k2settings->dst_landscape = 1;
                    ipl=3;
                    }
                strncpy(k2settings->dst_landscape_pages,&cl->cmdarg[ipl],1023);
                k2settings->dst_landscape_pages[1023]='\0';
                }
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
        if (!stricmp(cl->cmdarg,"-bpm1") || !stricmp(cl->cmdarg,"-bpm2")
                                         || !stricmp(cl->cmdarg,"-bpm"))
            {
            int *bcolor;

            bcolor=(cl->cmdarg[4]=='2') ? &k2settings->pagebreakmark_nobreak_color
                                        : &k2settings->pagebreakmark_breakpage_color;
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                {
                double v[3];
                int na;
                na=string_read_doubles(cl->cmdarg,v,3);
                if (na==1 && v[0]<0.)
                    {
                    (*bcolor) = -1.;
                    }
                else if (na<3)
                    {
                    if (!quiet)
                        k2printf(TTEXT_WARN "\a\n** Invalid -bpm color:  %s **\n\n" TTEXT_NORMAL,
                                 cl->cmdarg);
                    }
                else
                    {
                    int j,cc[3];

                    for (j=0;j<3;j++)
                        cc[j] = (v[j]<0.) ? 0 : (v[j]>1. ? 255 : (int)(v[j]*255.));
                    (*bcolor) = (cc[0]<<16) | (cc[1]<<8) | cc[2];
                    }
                }
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
                    k2settings->dst_dpi=k2settings->dst_userdpi=150;
                    k2settings->dst_fontsize_pts=0.;
                    k2settings->src_rot=0.;
                    k2settings->dst_color=1;
                    k2settings->src_trim=tm ? 1 : 0;
                    k2settings->dst_fit_to_page=-2;
                    {
                    int ii;
                    for (ii=0;ii<4;ii++)
                        {
                        k2settings->srccropmargins.box[ii]=0.;
                        k2settings->srccropmargins.units[ii]=UNITS_INCHES;
                        k2settings->dstmargins.box[ii]=0.;
                        k2settings->dstmargins.units[ii]=UNITS_INCHES;
                        }
                    }
                    k2settings->pad_left=k2settings->pad_top=k2settings->pad_bottom=k2settings->pad_right=0;
                    k2settings->mark_corners=0;
                    }
                else if (!stricmp(cl->cmdarg,"cc") 
                          || !stricmp(cl->cmdarg,"concat"))
                    {
#ifdef HAVE_OCR_LIB
                    k2settings->dst_ocr=0; /* -ocr- */
#endif
                    k2settings->use_crop_boxes=1; /* -n */
                    k2settings->text_wrap=0; /* -wrap- */
                    k2settings->max_columns=1; /* -col 1 */
                    k2settings->vertical_break_threshold=-2; /* -vb -2 */
                    k2settings->src_trim=0; /* -t- */
                    k2settings->dst_fit_to_page=-3; /* -f2p -3 */
                    k2settings->dst_userwidth=1.0;
                    k2settings->dst_userwidth_units=UNITS_SOURCE; /* -w 1s */
                    k2settings->dst_userheight=1.0;
                    k2settings->dst_userheight_units=UNITS_SOURCE; /* -h 1s */
                    k2settings->fit_columns=0; /* -fc- */
                    k2settings->dst_dpi=k2settings->dst_userdpi=150;
                    /* Reset other stuff */
                    k2settings->dst_landscape=0;
                    k2settings->dst_landscape_pages[0]='\0';
                    k2settings->dst_fontsize_pts=0.;
                    k2settings->src_rot=0.;
                    k2settings->dst_color=1;
                    k2settings->pad_left=k2settings->pad_top=k2settings->pad_bottom=k2settings->pad_right=0;
                    k2settings->mark_corners=0;
                    {
                    int ii;
                    for (ii=0;ii<4;ii++)
                        {
                        k2settings->srccropmargins.box[ii]=0.;
                        k2settings->srccropmargins.units[ii]=UNITS_INCHES;
                        k2settings->dstmargins.box[ii]=0.;
                        k2settings->dstmargins.units[ii]=UNITS_INCHES;
                        }
                    }
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
                    k2settings->dst_landscape_pages[0]='\0';
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
                    k2settings->dst_landscape_pages[0]='\0';
                    k2settings->text_wrap=1;
                    k2settings->max_columns=2;
                    k2settings->vertical_break_threshold=1.75;
                    k2settings->src_rot=SRCROT_AUTO;
                    k2settings->src_trim=1;
                    k2settings->dst_fit_to_page=0;
                    {
                    int ii;
                    for (ii=0;ii<4;ii++)
                        {
                        k2settings->srccropmargins.box[ii]=0.;
                        k2settings->srccropmargins.units[ii]=UNITS_INCHES;
                        k2settings->dstmargins.box[ii]=0.02;
                        k2settings->dstmargins.units[ii]=UNITS_INCHES;
                        }
                    }
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
                    /* v2.35--no more rounding--double precision */
                    if (na>2)
                        k2settings->src_grid_overlap_percentage=v[2];
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
            /* v2.36;  Fix if -bp is at end of cmd line */
            if (cmdlineinput_next(cl)==NULL)
                {
                if (setvals==1)
                    k2settings->dst_break_pages=2;
                break;
                }
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
        if (!stricmp(cl->cmdarg,"-ocrdpi"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
#ifdef HAVE_OCR_LIB
            if (setvals==1)
                k2settings->ocr_dpi=atoi(cl->cmdarg);
#endif
            continue;
            }
        if (!stricmp(cl->cmdarg,"-ocrd"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
#ifdef HAVE_OCR_LIB
            if (setvals==1)
                {
                int dt;
                dt=tolower(cl->cmdarg[0]);
                if (dt!='l' && dt!='w' && dt!='c' && dt!='p')
                    k2printf(TTEXT_WARN "\a-ocrd expects w(ord), l(ine), c(olumn), or p(age). Arg %s ignored." TTEXT_NORMAL "\n",cl->cmdarg);
                k2settings->ocr_detection_type=dt;
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
        if (!stricmp(cl->cmdarg,"-ac-"))
            {
            if (setvals==1)
                k2settings->autocrop=0;
            continue;
            }
        if (!stricmp(cl->cmdarg,"-ac"))
            {
            if (setvals==1)
                k2settings->autocrop=100;
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (is_a_number(cl->cmdarg))
                {
                if (setvals==1)
                    k2settings->autocrop=atof(cl->cmdarg)*990+10;
                }
            else
                readnext=0;
            if (k2settings->autocrop < 10)
                k2settings->autocrop = 10;
            if (k2settings->autocrop > 1000)
                k2settings->autocrop = 1000;
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
#ifdef HAVE_LEPTONICA_LIB
        if (!stricmp(cl->cmdarg,"-dw-"))
            {
            if (setvals==1)
                k2settings->dewarp=0;
            continue;
            }
        if (!stricmp(cl->cmdarg,"-dw"))
            {
            if (setvals==1)
                k2settings->dewarp=4;
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (is_a_number(cl->cmdarg))
                {
                if (setvals==1)
                    k2settings->dewarp=atoi(cl->cmdarg);
                }
            else
                readnext=0;
            continue;
            }
#endif
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
                k2parsecmd_set_value_with_units(cl->cmdarg,&k2settings->dst_userheight,&k2settings->dst_userheight_units,UNITS_PIXELS);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-wt") || !stricmp(cl->cmdarg,"-wt+"))
            {
            int paint;

            paint=(cl->cmdarg[3]=='+');
            if (!next_is_integer(cl,setvals==1,quiet,&good,&readnext,&k2settings->src_whitethresh))
                break;
            if (good && setvals==1)
                {
                if (k2settings->src_whitethresh>255)
                    k2settings->src_whitethresh=255;
                k2settings->src_paintwhite = paint ? 1 : 0;
                }
            continue;
            }
        if (!stricmp(cl->cmdarg,"-w"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                k2parsecmd_set_value_with_units(cl->cmdarg,&k2settings->dst_userwidth,&k2settings->dst_userwidth_units,UNITS_PIXELS);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-m") || !stricmp(cl->cmdarg,"-om"))
            {
            K2CROPBOX *cbox;
            int srcmar;

            srcmar = !stricmp(cl->cmdarg,"-m");
            cbox = srcmar ? &k2settings->srccropmargins : &k2settings->dstmargins;
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                {
                int na,k;

                for (na=0,k=0;na<4;na++,k++)
                    {
                    int c,m;
                    
                    for (m=k;cl->cmdarg[k]!=',' && cl->cmdarg[k]!='\0';k++);
                    c=cl->cmdarg[k];
                    cl->cmdarg[k]='\0';
                    if (k>m)
                        {
                        int jj;
                        /* Negative value w/o units means use source page size */
                        if (srcmar && is_a_number(&cl->cmdarg[m]) && atof(&cl->cmdarg[m])<0.)
                            {
                            cbox->box[na]=fabs(atof(&cl->cmdarg[m]));
                            cbox->units[na]=UNITS_SOURCE;
                            }
                        else
                            k2parsecmd_set_value_with_units(&cl->cmdarg[m],&cbox->box[na],&cbox->units[na],
                                                 UNITS_INCHES);
                        if (!srcmar && (cbox->units[na]==UNITS_TRIMMED || cbox->units[na]==UNITS_OCRLAYER))
                            cbox->units[na]=UNITS_SOURCE;
                        for (jj=na+1;jj<4;jj++)
                            {
                            cbox->box[jj]=cbox->box[na];
                            cbox->units[jj]=cbox->units[na];
                            }
                        }
                    if (c=='\0')
                        break;
                    }
/*
{
int jj;
printf("srcmar=%d\n    box,units =",srcmar);
for (jj=0;jj<4;jj++)
printf(" %g (%d)",cbox->box[jj],cbox->units[jj]);
printf("\n");
printf("units=%d\n",k2settings->srccropmargins.units[0]);
}
*/
                }
            continue;
            }
        if (!strnicmp(cl->cmdarg,"-cbox",5) || !strnicmp(cl->cmdarg,"-ibox",5))
            {
            char buf[256];
            K2CROPBOXES *boxes;
            int ignore;

            ignore = tolower(cl->cmdarg[1])=='c' ? 0 : K2CROPBOX_FLAGS_IGNOREBOXEDAREA;
            boxes = &k2settings->cropboxes;
            if (cl->cmdarg[5]=='-' && cl->cmdarg[6]=='\0')
                {
                if (setvals==1)
                    k2pdfopt_settings_clear_cropboxes(k2settings,K2CROPBOX_FLAGS_IGNOREBOXEDAREA,ignore);
                continue;
                }
            strncpy(buf,&cl->cmdarg[5],255);
            buf[255]='\0';
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                {
                int na,index,k;
                K2CROPBOX *box;

                for (index=0;index<boxes->n;index++)
                    if (boxes->cropbox[index].cboxflags&K2CROPBOX_FLAGS_NOTUSED)
                        break;
                if (index>=MAXK2CROPBOXES)
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
                box=&boxes->cropbox[index];
                strcpy(box->pagelist,buf);
                box->box[0]=0.;
                box->box[1]=0.;
                box->box[2]=-1.;
                box->box[3]=-1.;
                box->cboxflags=ignore;
                for (na=0,k=0;na<4;na++,k++)
                    {
                    int c,m;
                    
                    for (m=k;cl->cmdarg[k]!=',' && cl->cmdarg[k]!='\0';k++);
                    c=cl->cmdarg[k];
                    cl->cmdarg[k]='\0';
                    if (k>m)
                        k2parsecmd_set_value_with_units(&cl->cmdarg[m],&box->box[na],&box->units[na],
                                             UNITS_INCHES);
                    if (c=='\0')
                        break;
                    }
                if (na==0 || na==2)
                    {
                    if (!quiet)
                        k2printf(TTEXT_WARN "\a\n** Crop box %s is invalid and will be ignored. **\n\n"
                        TTEXT_NORMAL,cl->cmdarg);
                    box->cboxflags |= K2CROPBOX_FLAGS_NOTUSED;
                    }
                }
            continue;
            }
        if (!strnicmp(cl->cmdarg,"-nl",3) || !strnicmp(cl->cmdarg,"-nr",3))
            {
            char buf[256];
            double v[2];
            double left,right;

            if (cl->cmdarg[3]=='-' && cl->cmdarg[4]=='\0')
                {
                if (setvals==1)
                    k2settings->noteset.n=0;
                continue;
                }
            left = tolower(cl->cmdarg[2])=='l' ? .05 : .65;
            right = left + .3;
            strncpy(buf,&cl->cmdarg[3],255);
            buf[255]='\0';
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (string_read_doubles(cl->cmdarg,v,2)<2)
                readnext=0;
            else
                {
                left=v[0];
                right=v[1];
                }
            if (setvals==1)
                {
                int index;

                if (k2settings->noteset.n>=MAXK2NOTES)
                    {
                    static int warned=0;
                    if (!warned && !quiet)
                        k2printf(TTEXT_WARN "\a\n** Max notes margins exceeded (max=%d). **\n\n",
                                 MAXK2NOTES);
                    warned=1;
                    continue;
                    }
                index=k2settings->noteset.n;
                strcpy(k2settings->noteset.notes[index].pagelist,buf);
                k2settings->noteset.notes[index].left=left;
                k2settings->noteset.notes[index].right=right;
                k2settings->noteset.n++;
                /*
                ** In the two examples I tried, min_column_gap_inches had to be 0.05
                ** rather than 0.1.
                */
                k2settings->min_column_gap_inches=0.05;
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
                k2settings->dst_dpi=k2settings->dst_userdpi=167;
                k2settings->user_src_dpi = -2.0;
                k2settings->dst_userwidth=DEFAULT_WIDTH;
                k2settings->dst_userwidth_units=UNITS_PIXELS;
                k2settings->dst_userheight=DEFAULT_HEIGHT;
                k2settings->dst_userheight_units=UNITS_PIXELS;
                }
            else
                {
                k2settings->dst_dpi=k2settings->dst_userdpi=333;
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
        NEEDS_STRING("-colorbg",dst_bgcolor,MAXFILENAMELEN-1,0);
        NEEDS_STRING("-colorfg",dst_fgcolor,MAXFILENAMELEN-1,0);
        NEEDS_STRING("-toclist",toclist,2047,0);
        NEEDS_STRING("-tocsave",tocsavefile,MAXFILENAMELEN-1,0);
        NEEDS_STRING("-bpl",bpl,2047,0);
        NEEDS_STRING("-p",pagelist,1023,0)
        NEEDS_STRING("-px",pagexlist,1023,0)
        NEEDS_STRING("-author",dst_author,255,0)
        NEEDS_STRING("-title",dst_title,255,0)
#ifdef HAVE_OCR_LIB
        NEEDS_STRING("-ocrout",ocrout,127,0)
        if (k2settings->ocrout[0]!='\0' && k2settings->dst_ocr==0)
            k2settings->dst_ocr='m';
#endif
        NEEDS_STRING("-o",dst_opname_format,127,0)
        NEEDS_STRING("-ci",dst_coverimage,255,1)
        NEEDS_INTEGER("-evl",erase_vertical_lines)
        NEEDS_INTEGER("-er",src_erosion)
        NEEDS_INTEGER("-ehl",erase_horizontal_lines)
        if (!stricmp(cl->cmdarg,"-fs") && setvals==1)
            k2settings->user_mag |= 2;
        NEEDS_VALUE_PLUS("-fs",dst_fontsize_pts)
        NEEDS_INTEGER("-nt",nthreads)
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
        if (!stricmp(cl->cmdarg,"-mag") && setvals==1)
            k2settings->user_mag |= 4;
        NEEDS_VALUE("-mag",dst_magnification)
        if ((!stricmp(cl->cmdarg,"-odpi") || !stricmp(cl->cmdarg,"-dpi")) && setvals==1)
            k2settings->user_mag |= 1;
        NEEDS_INTEGER("-odpi",dst_userdpi)
        NEEDS_INTEGER("-dpi",dst_userdpi)
        k2settings->dst_dpi=k2settings->dst_userdpi;
#if (WILLUSDEBUX & 1)
printf("dst_dpi = %g\n",k2settings->dst_dpi);
#endif
        NEEDS_VALUE("-ws",word_spacing)
        CBOXVAL("-oml",k2settings->dstmargins,0,UNITS_INCHES,0)
        CBOXVAL("-omt",k2settings->dstmargins,1,UNITS_INCHES,0)
        CBOXVAL("-omr",k2settings->dstmargins,2,UNITS_INCHES,0)
        CBOXVAL("-omb",k2settings->dstmargins,3,UNITS_INCHES,0)
        CBOXVAL("-ml",k2settings->srccropmargins,0,UNITS_INCHES,1)
        CBOXVAL("-mt",k2settings->srccropmargins,1,UNITS_INCHES,1)
        CBOXVAL("-mr",k2settings->srccropmargins,2,UNITS_INCHES,1)
        CBOXVAL("-mb",k2settings->srccropmargins,3,UNITS_INCHES,1)
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

#ifdef HAVE_K2GUI
        /* v2.20--indicate that this arg is a file name */
        argcheck=0;
#endif
        /* Add command arg to file list */
        /* If wildcard, add all matching files--v2.33 */
        if (in_string(cl->cmdarg,"*")>=0 || in_string(cl->cmdarg,"?")>=0)
            {
            FILELIST *fl,_fl;
            int i;
            fl=&_fl;
            filelist_init(fl);
            filelist_fill_from_disk_1(fl,cl->cmdarg,0,0);
            filelist_sort_by_name(fl);
            for (i=0;i<fl->n;i++)
                {
                char fullname[MAXFILENAMELEN];

                wfile_fullname(fullname,fl->dir,fl->entry[i].name);
                k2pdfopt_files_add_file(&k2conv->k2files,fullname);
                }
            filelist_free(fl);
            }
        else
            k2pdfopt_files_add_file(&k2conv->k2files,cl->cmdarg);
        }
    strbuf_free(allopts);
#ifdef HAVE_K2GUI
    if (setvals==3)
        return(1);
#endif
    return(k2conv->k2files.n);
    }


void k2parsecmd_set_value_with_units(char *s,double *val,int *units,int defunits)

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
    else if (tolower(s[i])=='x')
        (*units)=UNITS_OCRLAYER;
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
