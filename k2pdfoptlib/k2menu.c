/*
** k2menu.c      Interactive user menu for k2pdfopt.c.
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

static void menu_help(void);
static void allopts_echo(STRBUF *o1,int o1clear,STRBUF *o2,int o2clear,STRBUF *o3);
static void one_opt_echo(STRBUF *opts,char *coloring,int *column);
static int  wildcount(char *wildspec);


int k2pdfopt_menu(K2PDFOPT_SETTINGS *k2settings,int filecount,
                  STRBUF *env,STRBUF *cmdline,STRBUF *usermenu)

    {
    int status,cmdclear,envclear;
    static char *ansyesno[]={"yes","no",""};
    static char *jpegpng[]={"png","jpeg",""};
    static char *ansjust[]={"left","center","right",""};
    double defmar;
    // char specfile[512];
    static char *options[] =
        {
        "a. Autostraighten (-as)",
        "b. Bitmap type (-jpg,-png,-bpc)",
        "bp. Break pages (-bp,-f2p)",
        "c. Color/Negative output (-c, -neg)",
        "co. Column detection (-col,-ch,...)",
        "cs. Contrast/Sharpen (-cmax,-g,-s,-wt)",
        "d. Device selection (-dev,-h,-w,-dpi)",
        "ds. Document scale factor (-ds)",
        "f. Fit to single column (-fc)",
        "gt. Gap thresholds (-gt...)",
        "j. Justification (-j)",
        "l. Landscape mode (-ls)",
        "m. Margin to ignore (-m)",
        "mo. Mode (-mode)",
#ifdef HAVE_MUPDF_LIB
        "n. Native PDF output (-n)",
#endif
        "o. Output name (-o)",
#ifdef HAVE_OCR_LIB
        "oc. OCR (-ocr,-ocrvis,...)",
#endif
        "om. Output margins (-om)",
        "p. Page range (-p)",
        "pd. Padding/Marking (-p[lrbt],-mc)",
        "r. Right-to-left page scans (-r)",
        "rt. Rotate source page (-sr)",
        "s. Special (-de,-evl,-gs)",
        "sm. Show marked source (-sm)",
        "u. Usage (command line opts)",
        "v. Vertical spacing (-vb,-vs)",
        "w. Wrap/Reflow text (-wrap,-ws)",
        "x. Exit on completion (-x)",
        ""};

    status=0; /* Avoid compiler warning */
    strbuf_clear(usermenu);
    cmdclear=0;
    envclear=0;
    /*
    if (filecount>0)
        strcpy(specfile,firstfile);
    */
    if (!k2settings->query_user)
        return(0);
    while (1)
        {
        int i,no,newmenu;
        char buf[512];
        for (i=0;options[i][0]!='\0';i++);
        no=i;
        for (i=0;i<(no+1)/2;i++)
            {
            char opt[8];
            int j,k;
            for (j=0;options[i][j]!='.';j++)
                opt[j]=options[i][j];
            opt[j]='\0';
            aprintf(TTEXT_BOLD "%2s" TTEXT_NORMAL "%-38s",opt,&options[i][j]);
            k=i+(no+1)/2;
            if (k < no)
                {
                for (j=0;options[k][j]!='.';j++)
                    opt[j]=options[k][j];
                opt[j]='\0';
                aprintf(TTEXT_BOLD "%2s" TTEXT_NORMAL "%s",opt,&options[k][j]);
                }
            aprintf("\n");
            }
        aprintf("\n");
        allopts_echo(env,envclear,cmdline,cmdclear,usermenu);
        newmenu=0;
        while (1)
            {
            int goodspec;

            /*
            if (filecount>0)
                {
                if (filecount==1)
                    aprintf("\nSource file: " TTEXT_MAGENTA "%s" TTEXT_NORMAL "\n",specfile);
                else 
                    aprintf("\nSource file: (multiple files specified)\n");
                aprintf(TTEXT_BOLD2 "Enter option above" TTEXT_NORMAL
                 " or " TTEXT_BOLD2 "?" TTEXT_NORMAL " for help"
                 " or " TTEXT_BOLD2 "page range" TTEXT_NORMAL " (e.g. 2,4,8-10) to convert\n"
                 "or " TTEXT_BOLD2 "q" TTEXT_NORMAL " to quit or just "
                 TTEXT_BOLD2 "<Enter>" TTEXT_NORMAL " to convert all pages: "
                 TTEXT_INPUT);
                }
            else
                aprintf("\n(No source file specified.)\n" 
                        TTEXT_BOLD2 "Enter option above" TTEXT_NORMAL
                 " or " TTEXT_BOLD2 "?" TTEXT_NORMAL " for help"
                 " or " TTEXT_BOLD2 "q" TTEXT_NORMAL " to quit\n"
                 "or type in a file name to convert: "
                 TTEXT_INPUT);
            aprintf(TTEXT_BOLD2 "Enter option above" TTEXT_NORMAL
                " or " TTEXT_BOLD2 "?" TTEXT_NORMAL " for help"
                " or " TTEXT_BOLD2 "cmd line opts" TTEXT_NORMAL " (e.g. -dpi 200)\n"
                " or " TTEXT_BOLD2 "q" TTEXT_NORMAL " to quit or just "
                TTEXT_BOLD2 "<Enter>" TTEXT_NORMAL " to start conversion: "
                TTEXT_INPUT);
            */
            aprintf("\n" TTEXT_BOLD2 "Enter option above (h=help, q=quit): " TTEXT_NORMAL);
            fgets(buf,511,stdin);
            aprintf(TTEXT_NORMAL "\n");
            clean_line(buf);
            if (!stricmp(buf,"?") || !stricmp(buf,"h"))
                {
                menu_help();
                break;
                }
            if (buf[0]=='\0')
                {
                if (cmdclear)
                    strbuf_clear(cmdline);
                if (envclear)
                    strbuf_clear(env);
                return(0);
                }
            if (tolower(buf[0])=='q')
                return(-1);
            if (!strcmp(buf,"-"))
                {
                strbuf_clear(usermenu);
                break;
                }
            if (!strcmp(buf,"--"))
                {
                cmdclear = !cmdclear;
                break;
                }
            if (!strcmp(buf,"---"))
                {
                envclear = !envclear;
                break;
                }
            if (buf[0]=='-')
                {
                strbuf_cat(usermenu,buf);
                break;
                }
            for (i=0;options[i][0]!='\0';i++)
                {
                if (options[i][1]=='.' && buf[1]=='\0' && tolower(buf[0])==tolower(options[i][0]))
                    break;
                if (options[i][2]=='.' && buf[2]=='\0' && !strnicmp(buf,options[i],2))
                    break;
                }
            if (options[i][0]!='\0')
                break;
            if (filecount>0 && pagelist_valid_page_range(buf))
                {
                strbuf_cat(usermenu,"-p");
                strbuf_cat_no_spaces(usermenu,buf);
                break;
                }
#if (!defined(WIN32) && !defined(WIN64))
            /* On Mac, backslashes are inserted before each space, */
            /* so try getting rid of them if the file isn't found. */
            if (wildcount(buf)==0)
				{
				int i,j;
				for (i=j=0;buf[i]!='\0';i++)
					{
					if (buf[i]=='\\')
						i++;
					buf[j]=buf[i];
					j++;
					}
				buf[j]='\0';
				}
#endif
            if (wildcount(buf)==0)
                {
                char buf2[512];
                strncpy(buf2,buf,505);
                buf2[505]='\0';
                strcat(buf2,".pdf");
                if (wildcount(buf2)>0)
                    strcpy(buf,buf2);
                }
            goodspec = (wildcount(buf)>0);
            /*
            if (filecount==0 && goodspec)
                {
                strcpy(specfile,buf);
                strcpy(uifile,buf);
                filecount=1;
                newmenu=1;
                break;
                }
            */
            if (goodspec)
                {
                // aprintf(TTEXT_WARN "\a** Invalid entry. (File%s already specified.) **" TTEXT_NORMAL "\n",filecount>1?"s":"");
                strbuf_cat_with_quotes(usermenu,buf);
                break;
                }
            else
                aprintf(TTEXT_WARN "\a** Unrecognized option: %s. **" TTEXT_NORMAL "\n",buf);
            }
        if (newmenu)
            continue;
        if (!stricmp(buf,"a"))
            {
            status=userinput_string("Auto-straighten the pages",ansyesno,k2settings->src_autostraighten?"y":"n");
            if (status<0)
                return(status);
            strbuf_sprintf(usermenu,"-as%s",status==0?"":"-");
            k2settings->src_autostraighten=(status==0) ? 4.0 : -1.0;
            }
        else if (!stricmp(buf,"b"))
            {
            status=userinput_string("Bitmap encoding (png=lossless)",jpegpng,"png");
            if (status<0)
                return(status);
            if (status==0)
                {
                strbuf_sprintf(usermenu,"-png");
                // k2settings->jpeg_quality=-1;
                status=userinput_integer("Bits per color plane (1, 2, 4, or 8)",4,&k2settings->dst_bpc,1,8);
                if (status<0)
                    return(status);
                if (k2settings->dst_bpc>=6)
                    k2settings->dst_bpc=8;
                else if (k2settings->dst_bpc>=3)
                    k2settings->dst_bpc=4;
                strbuf_sprintf(usermenu,"-bpc %d",k2settings->dst_bpc);
                if (k2settings->dst_bpc<8)
                    {
                    status=userinput_string("Apply dithering",ansyesno,k2settings->dst_dither?"y":"n");
                    if (status<0)
                        return(status);
                    k2settings->dst_dither=(status==0) ? 1 : 0;
                    strbuf_sprintf(usermenu,"-d%s",k2settings->dst_dither?"":"-");
                    }
                }
            else
                {
                status=userinput_integer("JPEG quality (1-99, lower=smaller size file)",
                                     90,&k2settings->jpeg_quality,1,99);
                if (status<0)
                    return(status);
                strbuf_sprintf(usermenu,"-jpg %d",k2settings->jpeg_quality);
                }
            }
        else if (!stricmp(buf,"bp"))
            {
            status=userinput_string("Break output pages at end of each input page",ansyesno,k2settings->dst_break_pages?"y":"n");
            if (status<0)
                return(status);
            k2settings->dst_break_pages=(status==0) ? 1 : 0;
            if (!k2settings->dst_break_pages)
                {
                double x;
                x=0.;
                status=userinput_float("Gap between source pages (inches)",x,&x,1,0.0,100.,NULL);
                if (status<0)
                    return(status);
                if (x>0.)
                    {
                    k2settings->dst_break_pages=-1-(int)(1000.*x+.5);
                    strbuf_sprintf(usermenu,"-bp %g",x);
                    }
                else
                    strbuf_sprintf(usermenu,"-bp-");
                }
            else
                strbuf_sprintf(usermenu,"-bp");
            status=userinput_integer("Fit-to-page value",k2settings->dst_fit_to_page,&k2settings->dst_fit_to_page,
                                 -2,999);
            if (status<0)
                return(status);
            if (k2settings->dst_fit_to_page==-2)
                k2settings->vertical_break_threshold = -2.;
            strbuf_sprintf(usermenu,"-f2p %d",k2settings->dst_fit_to_page);
            }
        else if (!stricmp(buf,"c"))
            {
            status=userinput_string("Full color output",ansyesno,k2settings->dst_color?"y":"n");
            if (status<0)
                return(status);
            k2settings->dst_color=!status;
            strbuf_sprintf(usermenu,"-c%s",k2settings->dst_color?"":"-");
            status=userinput_string("Negative (inverted) output",ansyesno,
                                    k2settings->dst_negative?"y":"n");
            if (status<0)
                return(status);
            k2settings->dst_negative=!status;
            strbuf_sprintf(usermenu,"-neg%s",k2settings->dst_negative?"":"-");
            }
        else if (!stricmp(buf,"co"))
            {
            status=userinput_integer("Max number of columns (1, 2, or 4)",4,&k2settings->max_columns,1,4);
            if (status<0)
                return(status);
            if (k2settings->max_columns==3)
                k2settings->max_columns=4;
            strbuf_sprintf(usermenu,"-col %d",k2settings->max_columns);
            if (k2settings->max_columns>1)
                {
                status=userinput_float("Min gap between columns (inches)",k2settings->min_column_gap_inches,
                            &k2settings->min_column_gap_inches,1,0.0,20.,NULL);
                if (status<0)
                    return(status);
                strbuf_sprintf(usermenu,"-cg %g",k2settings->min_column_gap_inches);
                status=userinput_float("Max gap between columns (inches)",k2settings->max_column_gap_inches,
                            &k2settings->max_column_gap_inches,1,0.,99.,NULL);
                if (status<0)
                    return(status);
                strbuf_sprintf(usermenu,"-cgmax %g",k2settings->max_column_gap_inches);
                status=userinput_float("Min column height (inches)",k2settings->min_column_height_inches,
                            &k2settings->min_column_height_inches,1,0.05,20.,NULL);
                if (status<0)
                    return(status);
                strbuf_sprintf(usermenu,"-ch %g",k2settings->min_column_height_inches);
                status=userinput_float("Column gap range (0 - 1)",k2settings->column_gap_range,
                            &k2settings->column_gap_range,1,0.0,1.,NULL);
                if (status<0)
                    return(status);
                strbuf_sprintf(usermenu,"-cgr %g",k2settings->column_gap_range);
                status=userinput_float("Column row gap height (inches)",k2settings->column_row_gap_height_in,
                            &k2settings->column_row_gap_height_in,1,0.001,5.0,NULL);
                if (status<0)
                    return(status);
                strbuf_sprintf(usermenu,"-crgh %g",k2settings->column_row_gap_height_in);
                status=userinput_float("Column offset maximum (0 to 1 or -1 to disable)",k2settings->column_offset_max,
                            &k2settings->column_offset_max,1,-1.5,1.,NULL);
                if (status<0)
                    return(status);
                strbuf_sprintf(usermenu,"-comax %g",k2settings->column_offset_max);
                }
            }
        else if (!stricmp(buf,"cs"))
            {
            status=userinput_float("Max contrast adjust (1.0=no adjust)",k2settings->contrast_max,&k2settings->contrast_max,1,
                             -200.,200.,NULL);
            if (status<0)
                return(status);
            strbuf_sprintf(usermenu,"-cmax %g",k2settings->contrast_max);
            status=userinput_string("Sharpen the output images",ansyesno,k2settings->dst_sharpen?"y":"n");
            if (status<0)
                return(status);
            k2settings->dst_sharpen=!status;
            strbuf_sprintf(usermenu,"-s%s",k2settings->dst_sharpen?"":"-");
            status=userinput_float("Gamma value (1.0=no adjustment)",k2settings->dst_gamma,&k2settings->dst_gamma,1,
                             0.01,100.,NULL);
            if (status<0)
                return(status);
            strbuf_sprintf(usermenu,"-g %g",k2settings->dst_gamma);
            status=userinput_integer("White threshold (-1=autocalc)",k2settings->src_whitethresh,&k2settings->src_whitethresh,-1,255);
            if (status<0)
                return(status);
            strbuf_sprintf(usermenu,"-wt %d",k2settings->src_whitethresh);
            }
        else if (!stricmp(buf,"d"))
            {
            char *p;

            p=devprofile_select();
            if (p!=NULL && !strcmp(p,"q"))
                return(status);
            if (p!=NULL)
                {
                strbuf_sprintf(usermenu,"-dev %s",p);
                continue;
                }   
            status=userinput_float("Destination pixel width",DEFAULT_WIDTH,&k2settings->dst_userwidth,1,-100.,20000.,NULL);
            if (status<0)
                return(status);
            strbuf_sprintf(usermenu,"-w %g",k2settings->dst_userwidth);
            k2settings->dst_userwidth_units=UNITS_PIXELS;
            status=userinput_float("Destination pixel height",DEFAULT_HEIGHT,&k2settings->dst_userheight,1,-100.,20000.,NULL);
            if (status<0)
                return(status);
            strbuf_sprintf(usermenu,"-h %g",k2settings->dst_userheight);
            k2settings->dst_userheight_units=UNITS_PIXELS;
            status=userinput_integer("Output/Destination pixels per inch",167,&k2settings->dst_dpi,20,1200);
            if (status<0)
                return(status);
            strbuf_sprintf(usermenu,"-dpi %d",k2settings->dst_dpi);
            status=userinput_float("Input/Source pixels per inch",k2settings->user_src_dpi,&k2settings->user_src_dpi,1,-10.,1200.,NULL);
            if (status<0)
                return(status);
            while (k2settings->user_src_dpi > -.25 &&  k2settings->user_src_dpi < 50.)
                {
                aprintf(TTEXT_WARN "\n\a** Invalid response.  Dpi must be <= -.25 or >= 50. **" TTEXT_NORMAL "\n\n");
                status=userinput_float("Input/Source pixels per inch",k2settings->user_src_dpi,&k2settings->user_src_dpi,1,-10.,1200.,NULL);
                if (status<0)
                    return(status);
                }
            strbuf_sprintf(usermenu,"-idpi %g",k2settings->user_src_dpi);
            }
        else if (!stricmp(buf,"ds"))
            {
            status=userinput_float("Document scale factor (1.0=no change)",k2settings->document_scale_factor,&k2settings->document_scale_factor,1,0.01,100.,NULL);
            if (status<0)
                return(status);
            strbuf_sprintf(usermenu,"-ds %g",k2settings->document_scale_factor);
            }
        else if (!stricmp(buf,"f"))
            {
            status=userinput_string("Fit single column to reader",ansyesno,k2settings->fit_columns?"y":"n");
            if (status<0)
                return(status);
            k2settings->fit_columns=!status;
            strbuf_sprintf(usermenu,"-fc%s",k2settings->fit_columns?"":"-");
            }
        else if (!stricmp(buf,"gt"))
            {
            status=userinput_float("Gap threshold (-gtc) for columns (inches)",k2settings->gtc_in,&k2settings->gtc_in,1,0.,20.,NULL);
            if (status<0)
                return(status);
            strbuf_sprintf(usermenu,"-gtc %g",k2settings->gtc_in);
            /*
            status=userinput_float("Gap threshold for margins (inches)",gtm_in,&gtm_in,1,0.,20.,NULL);
            if (status<0)
                return(status);
            */
            status=userinput_float("Gap threshold for rows (inches)",k2settings->gtr_in,&k2settings->gtr_in,1,0.,20.,NULL);
            if (status<0)
                return(status);
            strbuf_sprintf(usermenu,"-gtr %g",k2settings->gtr_in);
            status=userinput_float("Gap threshold for words (inches)",k2settings->gtw_in,&k2settings->gtw_in,1,0.,20.,NULL);
            if (status<0)
                return(status);
            strbuf_sprintf(usermenu,"-gtw %g",k2settings->gtw_in);
            }
        else if (!stricmp(buf,"j"))
            {
            status=userinput_string("Use default document justification",ansyesno,k2settings->dst_justify<0?"y":"n");
            if (status<0)
                return(status);
            if (status==0)
                k2settings->dst_justify=-1;
            else
                {
                status=userinput_string("Justification",ansjust,"center");
                if (status<0)
                    return(status);
                k2settings->dst_justify=status;
                }
            status=userinput_string("Use default full justification (same as document)",ansyesno,k2settings->dst_fulljustify<0?"y":"n");
            if (status<0)
                return(status);
            if (status==0)
                k2settings->dst_fulljustify=-1;
            else
                {
                status=userinput_string("Attempt full justification",ansyesno,k2settings->dst_fulljustify?"y":"n");
                if (status<0)
                    return(status);
                k2settings->dst_fulljustify=!status;
                }
            strbuf_sprintf(usermenu,"-j %d%s",k2settings->dst_justify,
                              k2settings->dst_fulljustify<0?"":(k2settings->dst_fulljustify?"+":"-"));
            status=userinput_string("Apply special justification for figures (tall regions)",
                               ansyesno,k2settings->dst_figure_justify>=0?"y":"n");
            if (status<0)
                return(status);
            if (status==1)
                k2settings->dst_figure_justify = -1;
            else
                {
                status=userinput_string("Figure (tall region) justification",ansjust,"center");
                if (status<0)
                    return(status);
                k2settings->dst_figure_justify=status;
                status=userinput_float("Figure height threshold (inches)",
                                  k2settings->dst_min_figure_height_in,
                                  &k2settings->dst_min_figure_height_in,1,0.,100.,NULL);
                if (status<0)
                    return(status);
                }
            strbuf_sprintf(usermenu,"-jf %d %g",k2settings->dst_figure_justify,k2settings->dst_min_figure_height_in);
            }
        else if (!stricmp(buf,"l"))
            {
            status=userinput_string("Landscape mode",ansyesno,k2settings->dst_landscape?"y":"n");
            if (status<0)
                return(status);
            k2settings->dst_landscape=!status;
            strbuf_sprintf(usermenu,"-ls%s",k2settings->dst_landscape?"":"-");
            }
#ifdef HAVE_MUPDF_LIB
        else if (!stricmp(buf,"n"))
            {
            status=userinput_string("Use native PDF output",ansyesno,k2settings->use_crop_boxes?"y":"n");
            if (status<0)
                return(status);
            k2settings->use_crop_boxes=!status;
            strbuf_sprintf(usermenu,"-n%s",k2settings->use_crop_boxes?"":"-");
            }
#endif
        else if (!stricmp(buf,"m"))
            {
            double v[4];
            int i,na;

            defmar=-1.0;
            if (defmar<0. && k2settings->mar_left>=0.)
                defmar=k2settings->mar_left;
            if (defmar<0. && k2settings->mar_top>=0.)
                defmar=k2settings->mar_top;
            if (defmar<0. && k2settings->mar_right>=0.)
                defmar=k2settings->mar_right;
            if (defmar<0. && k2settings->mar_bot>=0.)
                defmar=k2settings->mar_bot;
            if (defmar<0.)
                defmar=0.25;
            na=userinput_float("Inches of source border to ignore",defmar,v,4,0.,10.,
                          "Enter one value or left,top,right,bottom values comma-separated.");
            if (na<0)
                return(na);
            i=0;
            k2settings->mar_left=v[i];
            if (i<na-1)
                i++;
            k2settings->mar_top=v[i];
            if (i<na-1)
                i++;
            k2settings->mar_right=v[i];
            if (i<na-1)
                i++;
            k2settings->mar_bot=v[i];
            strbuf_sprintf(usermenu,"-m %g,%g,%g,%g",k2settings->mar_left,k2settings->mar_top,k2settings->mar_right,k2settings->mar_bot);
            }
        else if (!stricmp(buf,"mo"))
            {
            static char *modename[]={"default","copy","fitwidth","2-column","grid",""};
            static char *shortname[]={"def","copy","fw","2col","grid"};
            double v[3];

            status=userinput_string("Operating mode",modename,"default");
            if (status<0)
                return(status);
            if (status<4)
                {
                strbuf_sprintf(usermenu,"-mode %s",shortname[status]);
                continue;
                }   
            status=userinput_float("Grid cols,rows,overlap (comma-separated)",-1e10,v,3,1.,10.,NULL);
            if (status<0)
                return(status);
            strbuf_sprintf(usermenu,"-grid %dx%dx%d",(int)(v[0]+.5),(int)(v[1]+.5),(int)(v[2]+.5));
            }
        else if (!stricmp(buf,"om"))
            {
            double v[4];
            int i,na;
            na=userinput_float("Output device margin",k2settings->dst_mar,v,4,0.,10.,
                          "Enter one value or left,top,right,bottom values comma-separated.");
            if (na<0)
                return(na);
            i=0;
            k2settings->dst_marleft=v[i];
            if (i<na-1)
                i++;
            k2settings->dst_martop=v[i];
            if (i<na-1)
                i++;
            k2settings->dst_marright=v[i];
            if (i<na-1)
                i++;
            k2settings->dst_marbot=v[i];
            strbuf_sprintf(usermenu,"-om %g,%g,%g,%g",
                             k2settings->dst_marleft,k2settings->dst_martop,k2settings->dst_marright,k2settings->dst_marbot);
            }
        else if (!stricmp(buf,"o"))
            {
            int prompt;
            status=userinput_any_string("Output name format (e.g. out%02d)",k2settings->dst_opname_format,127,"%s_k2opt");
            if (status<0)
                return(status);
            strbuf_cat(usermenu,"-o");
            strbuf_cat_with_quotes(usermenu,k2settings->dst_opname_format);
            prompt=(k2settings->overwrite_minsize_mb >= 0.);
            status=userinput_string("Prompt to overwrite a file",ansyesno,prompt?"y":"n");
            if (status<0)
                return(status);
            if (status)
                k2settings->overwrite_minsize_mb=-1.0;
            else
                {
                status=userinput_float("Max file size before prompting (MB)",k2settings->overwrite_minsize_mb,
                          &k2settings->overwrite_minsize_mb,1,0.,1000000.,"(Enter 0 to always prompt.)");
                if (status<0)
                    return(status);
                }
            strbuf_sprintf(usermenu,"-ow %g",k2settings->overwrite_minsize_mb);
            }
#ifdef HAVE_OCR_LIB
        else if (!stricmp(buf,"oc"))
            {
            static char *ocropts[]={
#ifdef HAVE_TESSERACT_LIB
                         "Tesseract",
#endif
                         "Gocr","None",""};

            status=userinput_string("OCR choice",ocropts,k2settings->dst_ocr=='t'?"t":(k2settings->dst_ocr=='g')?"g":"n");
            if (status<0)
                return(status);
            k2settings->dst_ocr=tolower(ocropts[status][0]);
            if (k2settings->dst_ocr=='n')
                {
                k2settings->dst_ocr=0;
                strbuf_cat(usermenu,"-ocr-");
                }
            else
                strbuf_sprintf(usermenu,"-ocr %c",k2settings->dst_ocr);
#ifdef HAVE_TESSERACT_LIB
            if (k2settings->dst_ocr=='t' && getenv("TESSDATA_PREFIX")!=NULL)
                {
                FILELIST *fl,_fl;
                char tdir1[512];
                char tdir[512];
                
                fl=&_fl;
                wfile_fullname(tdir1,getenv("TESSDATA_PREFIX"),"tessdata");
                wfile_fullname(tdir,tdir1,"*.traineddata");
                filelist_init(fl);
                filelist_fill_from_disk_1(fl,tdir,0,0);
                if (fl->n>1)
                    {
                    int i;
                    char base1[512];
                    char base[512];

                    filelist_sort_by_date(fl);
                    for (i=1;i<=fl->n;i++)
                        {
                        wfile_basespec(base1,fl->entry[fl->n-i].name);
                        wfile_newext(base,base1,"");
                        aprintf(TTEXT_BOLD "%2d" TTEXT_NORMAL ". %s\n",i,base);
                        }
                    while (1)
                        {
                        char buf[16];
                        aprintf(TTEXT_BOLD2 "Enter language selection (def=1): " TTEXT_NORMAL);
                        fgets(buf,15,stdin);
                        clean_line(buf);
                        if (buf[0]=='\0')
                            {
                            i=1;
                            break;
                            }
                        i=atoi(buf);
                        if (!is_an_integer(buf) || i<1 || i>fl->n)
                            {
                            aprintf("\n" TTEXT_WARN " ** Please enter a number in the range 1 - %d. **" TTEXT_NORMAL "\n\n",fl->n);
                            continue;
                            }
                        break;
                        }
                    wfile_basespec(base1,fl->entry[fl->n-i].name);
                    wfile_newext(base,base1,"");
                    strncpy(k2settings->dst_ocr_lang,base,15);
                    k2settings->dst_ocr_lang[15]='\0';
                    strbuf_sprintf(usermenu,"-ocrlang %s",k2settings->dst_ocr_lang);
                    }
                }
#endif
            if (k2settings->dst_ocr)
                {
                status=userinput_float("Max OCR word height (in)",k2settings->ocr_max_height_inches,
                                  &k2settings->ocr_max_height_inches,1,0.,999.,"");
                if (status<0)
                    return(status);
                strbuf_sprintf(usermenu,"-ocrhmax %g",k2settings->ocr_max_height_inches);
                status=userinput_string("Show OCR text",ansyesno,"n");
                if (status<0)
                    return(status);
                if (!status)
                    k2settings->dst_ocr_visibility_flags |= 2;
                else
                    k2settings->dst_ocr_visibility_flags &= (~2);
                status=userinput_string("Show source file",ansyesno,"y");
                if (status<0)
                    return(status);
                if (!status)
                    k2settings->dst_ocr_visibility_flags |= 1;
                else
                    k2settings->dst_ocr_visibility_flags &= (~1);
                strbuf_sprintf(usermenu,"-ocrvis %s%s%s",
                              k2settings->dst_ocr_visibility_flags&1 ? "s" : "",
                              k2settings->dst_ocr_visibility_flags&2 ? "t" : "",
                              k2settings->dst_ocr_visibility_flags&4 ? "b" : "");
                }
            }
#endif
        else if (!stricmp(buf,"p"))
            {
            status=userinput_any_string("Pages to convert (e.g. 1-5,6,9-)",k2settings->pagelist,1023,"all");
            if (status<0)
                return(status);
            strbuf_cat(usermenu,"-p");
            strbuf_cat_no_spaces(usermenu,k2settings->pagelist);
            }
        else if (!stricmp(buf,"pd"))
            {
            int defpad=0;
            status=userinput_integer("Output bitmap padding",defpad,&defpad,0,6000);
            if (status>=0.)
                k2settings->pad_left=k2settings->pad_right=k2settings->pad_bottom=k2settings->pad_top=defpad;
            else
                return(status);
            strbuf_sprintf(usermenu,"-pl %d -pr %d -pt %d -pb %d",defpad,defpad,defpad,defpad);
            status=userinput_string("Mark corners of bitmap with a dot",ansyesno,k2settings->mark_corners?"y":"n");
            if (status<0)
                return(status);
            k2settings->mark_corners=!status;
            strbuf_sprintf(usermenu,"-mc%s",k2settings->mark_corners?"":"-");
            }
        else if (!stricmp(buf,"r"))
            {
            status=userinput_string("Scan right to left",ansyesno,k2settings->src_left_to_right?"n":"y");
            if (status<0)
                return(status);
            k2settings->src_left_to_right=status;
            strbuf_sprintf(usermenu,"-r%s",k2settings->src_left_to_right?"-":"");
            }
        else if (!stricmp(buf,"rt"))
            {
            status=userinput_string("Auto-detect entire doc rotation",ansyesno,
                               fabs(k2settings->src_rot-SRCROT_AUTO)<.5?"y":"n");
            if (status<0)
                return(status);
            if (!status)
                k2settings->src_rot=SRCROT_AUTO;
            else
                {
                status=userinput_string("Auto-detect rotation of each page",ansyesno,
                                   fabs(k2settings->src_rot-SRCROT_AUTOEP)<.5?"y":"n");
                if (status<0)
                    return(status);
                if (!status)
                    k2settings->src_rot=SRCROT_AUTOEP;
                else
                    {
                    double defval;
                    defval = (k2settings->src_rot < -900.) ? 0. : k2settings->src_rot;
                    status=userinput_integer("Source rotation (degrees c.c.)",defval,&k2settings->src_rot,-360,360);
                    if (status<0)
                        return(status);
                    }
                }
            if (k2settings->src_rot==SRCROT_AUTO)
                strbuf_sprintf(usermenu,"-rt auto");
            else if (k2settings->src_rot==SRCROT_AUTOEP)
                strbuf_sprintf(usermenu,"-rt aep");
            else
                strbuf_sprintf(usermenu,"-rt %d",k2settings->src_rot);
            }
        else if (!stricmp(buf,"s"))
            {
            status=userinput_float("Defect size in points",k2settings->defect_size_pts,&k2settings->defect_size_pts,1,0.0,100.,NULL);
            if (status<0)
                return(status);
            strbuf_sprintf(usermenu,"-de %g",k2settings->defect_size_pts);
            printf("\n0. Don't erase vertical lines.\n"
                     "1. Detect and erase only free-standing vertical lines.\n"
                     "2. Detect and erase all vertical lines.\n\n");
            status=userinput_integer("Enter option above (0, 1, or 2)",
                                 k2settings->erase_vertical_lines,&k2settings->erase_vertical_lines,0,2);
            strbuf_sprintf(usermenu,"-evl %d",k2settings->erase_vertical_lines);
            if (status<0)
                return(status);
#ifdef HAVE_MUPDF_LIB
            status=userinput_string("Use Ghostscript interpreter",ansyesno,k2settings->user_usegs?"y":"n");
            if (status<0)
                return(status);
            k2settings->user_usegs=(status==0 ? 1 : 0);
            strbuf_sprintf(usermenu,"-gs%s",k2settings->user_usegs?"":"--");
#endif
            }
        else if (!stricmp(buf,"sm"))
            {
            status=userinput_string("Show marked source",ansyesno,k2settings->show_marked_source==1?"y":"n");
            if (status<0)
                return(status);
            k2settings->show_marked_source=!status;
            strbuf_sprintf(usermenu,"-sm%s",k2settings->show_marked_source?"":"-");
            }
        else if (!stricmp(buf,"u"))
            {
            int i,tty_rows;

            k2sys_header();
            if (!k2pdfopt_usage())
                return(-1);
            tty_rows = get_ttyrows();
            for (i=0;i<tty_rows-16;i++)
                aprintf("\n");
            }
        else if (!stricmp(buf,"v"))
            {
            status=userinput_float("Vertical break threshold (-1 = don't allow)",
                   k2settings->vertical_break_threshold,&k2settings->vertical_break_threshold,1,-2.,100.,NULL);
            if (status<0)
                return(status);
            strbuf_sprintf(usermenu,"-vb %g",k2settings->vertical_break_threshold);
            /*
            status=userinput_float("Vertical Multiplier",vertical_multiplier,
                               &vertical_multiplier,1,0.1,10.,NULL);
            */
            status=userinput_float("Vertical line spacing",k2settings->vertical_line_spacing,
                               &k2settings->vertical_line_spacing,1,-10.,10.,NULL);
            if (status<0)
                return(status);
            strbuf_sprintf(usermenu,"-vls %g",k2settings->vertical_line_spacing);
            status=userinput_float("Max Vertical Gap (inches)",k2settings->max_vertical_gap_inches,
                               &k2settings->max_vertical_gap_inches,1,0.0,100.,NULL);
            if (status<0)
                return(status);
            strbuf_sprintf(usermenu,"-vs %g",k2settings->max_vertical_gap_inches);
            }
        else if (!stricmp(buf,"w"))
            {
            status=userinput_string("Wrap text",ansyesno,k2settings->text_wrap?"y":"n");
            if (status<0)
                return(status);
            k2settings->text_wrap=!status ? 1 : 0;
            if (k2settings->text_wrap)
                {
                int reflow_short=0;
                status=userinput_string("Re-flow short lines",ansyesno,reflow_short?"y":"n");
                if (status<0)
                    return(status);
                if (!status)
                    k2settings->text_wrap=2;
                strbuf_sprintf(usermenu,"-wrap%s",k2settings->text_wrap==2?"+":"");
                status=userinput_string("Preserve indentation",ansyesno,k2settings->preserve_indentation?"y":"n");
                if (status<0)
                    return(status);
                k2settings->preserve_indentation=!status;
                strbuf_sprintf(usermenu,"-pi%s",k2settings->preserve_indentation?"":"-");
                status=userinput_string("Detect/eliminate hyphens",ansyesno,k2settings->hyphen_detect?"y":"n");
                if (status<0)
                    return(status);
                k2settings->hyphen_detect=!status;
                strbuf_sprintf(usermenu,"-hy%s",k2settings->hyphen_detect?"":"-");
                status=userinput_float("Word spacing threshold (as fraction of lowercase 'o' height)",
                               k2settings->word_spacing,&k2settings->word_spacing,1,0.01,10.,NULL);
                if (status<0)
                    return(status);
                strbuf_sprintf(usermenu,"-ws %g",k2settings->word_spacing);
                }
            else
                strbuf_cat(usermenu,"-wrap-");
            }
        else if (!stricmp(buf,"x"))
            {
            status=userinput_string("Exit on completion",ansyesno,k2settings->exit_on_complete==1?"y":"n");
            if (status<0)
                return(status);
            k2settings->exit_on_complete=!status;
            strbuf_sprintf(usermenu,"-x%s",k2settings->exit_on_complete?"":"-");
            }
        aprintf("\n");
        }
    }


static void menu_help(void)

    {
    static char *mhelp=
        "\nYou may enter any of the following at the prompt:\n\n"
        TTEXT_BOLD2
        "    (menu item)  "
        TTEXT_NORMAL
        "Enter the one of the 1- or 2-letter menu options.\n"
        "                 E.g. " TTEXT_BOLD "d" TTEXT_NORMAL " or "
                                 TTEXT_BOLD "mo" TTEXT_NORMAL " (followed by <Enter>).\n\n"
        TTEXT_BOLD2
        "    <Enter>      "
        TTEXT_NORMAL
        "Start the conversion process.\n\n"
        TTEXT_BOLD2
        "    (page range) "
        TTEXT_NORMAL
        "Set the pages to be converted.  E.g. " TTEXT_BOLD "1,5,6-10" TTEXT_NORMAL ".\n\n"
        TTEXT_BOLD2
        "    (file name)  "
        TTEXT_NORMAL
        "Add a file to be converted.  May have wildcards.\n"
        "                 E.g. " TTEXT_BOLD "myfile.pdf" TTEXT_NORMAL " or "
                                 TTEXT_BOLD "romance*.pdf" TTEXT_NORMAL ".\n\n"
        TTEXT_BOLD2
        "    (cmd opt)    "
        TTEXT_NORMAL
        "Enter any command-line option(s) (must start with -).\n"
        "                 E.g. " TTEXT_BOLD "-dr 2 -wrap+" TTEXT_NORMAL ".\n\n"
        TTEXT_BOLD2
        "    -            "
        TTEXT_NORMAL
        "Clear all options that you've entered (no undo).\n"
        "                 (These are shown in " ANSI_GREEN "green" TTEXT_NORMAL ".)\n\n"
        TTEXT_BOLD2
        "    --           "
        TTEXT_NORMAL
        "Clear/unclear any options entered at the command line (" ANSI_BROWN "brown" TTEXT_NORMAL ").\n\n"
        TTEXT_BOLD2
        "    ---          "
        TTEXT_NORMAL
        "Clear/unclear options from the K2PDFOPT env. variable (" ANSI_DARKCYAN "cyan" TTEXT_NORMAL ").\n\n"
        TTEXT_BOLD2
        "    q            "
        TTEXT_NORMAL
        "Quit (abort).\n";
    char buf[16];

    aprintf("%s",mhelp);
    aprintf("Press " TTEXT_BOLD "<Enter>" TTEXT_NORMAL " to re-display the menu.");
    fgets(buf,15,stdin);
    aprintf("\n\n");
    }


static void allopts_echo(STRBUF *o1,int o1clear,STRBUF *o2,int o2clear,STRBUF *o3)

    {
    int col;

    aprintf("Selected options: ");
    col=18;
    if ((o1->s==NULL || o1->s[0]=='\n')
         && (o2->s==NULL || o2->s[0]=='\n')
         && (o3->s==NULL || o3->s[0]=='\n'))
        aprintf("(none)");
    else
        {
        if (!o1clear)
            one_opt_echo(o1,ANSI_DARKCYAN,&col);
        if (!o2clear)
            one_opt_echo(o2,ANSI_BROWN,&col);
        one_opt_echo(o3,ANSI_GREEN,&col);
        }
    aprintf("\n");
    }


static void one_opt_echo(STRBUF *opts,char *coloring,int *column)

    {
    int i;
    char *s;

    if (opts->s==NULL || opts->s[0]=='\0')
        return;
    aprintf("%s",coloring);
    s=opts->s;
    for (i=0;s[i]!='\0';)
        {
        int i0,c;
        for (;s[i]==' ' || s[i]=='\t';i++);
        if (s[i]=='\0')
            break;
        for (i0=i;s[i]!=' ' && s[i]!='\t' && s[i]!='\0';i++);
        if (i-i0+1+(*column) > 78)
            {
            aprintf("\n   ");
            (*column)=4;
            }
        c=s[i];
        s[i]='\0';
        aprintf(" %s",&s[i0]);
        (*column) += strlen(&s[i0])+1;
        s[i]=c;
        if (s[i]=='\0')
            break;
        }
    aprintf("%s",TTEXT_NORMAL);
    }


static int wildcount(char *wildspec)

    {
    FILELIST *fl,_fl;
    int n;

    fl=&_fl;
    filelist_init(fl);
    filelist_fill_from_disk_1(fl,wildspec,0,0);
    n=fl->n;
    filelist_free(fl);
    return(n);
    }


