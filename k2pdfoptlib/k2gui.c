/*
** k2gui2.c      K2pdfopt Generic WILLUSGUI functions.
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
**
** VERSION HISTORY
**
**     See k2version.c.
**
*/

#include <k2pdfopt.h>

#ifdef HAVE_K2GUI

static char *unitname[]={"px","in","cm","s","t","x",""};
static K2GUI *k2gui,_k2gui;
static int force_repaint=0;

char *k2gui_short_name(char *filename);
static int  k2gui_winposition_get(WILLUSGUIRECT *rect);
static void k2gui_winposition_save(void);
static void k2gui_get_settings(int index,K2PDFOPT_SETTINGS *k2settings,STRBUF *extra);
static void k2gui_save_settings(int index,K2PDFOPT_SETTINGS *k2settings,STRBUF *extra,char *name);
static void k2gui_set_button_defaults(void);
/*
static void k2gui_cmdline_to_settings(K2PDFOPT_SETTINGS *k2settings,STRBUF *cmdline);
*/
static void k2gui_init(void);
static void k2gui_close(void);
static void k2gui_background_bitmap_fill(void);
static void bmp_gradient_fill(WILLUSBITMAP *bmp);
/*
static int  k2gui_window_size_changed(WILLUSGUIWINDOW *window);
*/
static void k2gui_main_window_init(int normal_size);
static void k2gui_window_menus_init(WILLUSGUIWINDOW *win);
static int  k2gui_set_device_from_listbox(WILLUSGUICONTROL *control);
static void k2gui_display_info(void);
static WILLUSGUICONTROL *k2gui_control_with_focus(int *index);
static void k2gui_update_controls(void);
static void k2gui_error_out(char *message);
static void k2gui_clear_envvars(void);
static void k2gui_add_files(void);
static void k2gui_add_folder(void);
static void k2gui_save_settings_to_file(void);
static void k2gui_restore_settings_from_file(void);
static int  k2gui_determine_fontsize(void);
static void k2gui_add_children(int already_drawn);
static void k2gui_cropbox_eval(char *buf,int cmindex,int fieldindex);
static void filebox_populate(void);
static void k2gui_destroy_mainwin(void);
static void k2gui_get_selection(void);
static void k2gui_apply_selection(void);
static void k2gui_destroy_children(void);
static WILLUSGUICONTROL *k2gui_control_by_name(char *name);
/* Preview functions */
static void k2gui_preview_start(void);
static void k2gui_preview_make_bitmap(char *data);
static void k2gui_preview_cleanup(int statuscode);
static int k2gui_preview_done(void);
static void k2gui_preview_terminate(void);
static void k2gui_preview_fail(int statuscode);
static void k2gui_preview_bitmap_message(WILLUSBITMAP *bmp,int width,int height,
                                         double fs,char *message);
static void next_bmp_line(char *d,char *s,int *index,double mpw,int maxpix);
static void k2gui_contextmenu_by_control(WILLUSGUICONTROL *control);
static void k2gui_contextmenu(char *title,char *content);


int k2gui_main(K2PDFOPT_CONVERSION  *k2conv0,void *hInstance,void *hPrevInstance,
               STRBUF *env,STRBUF *cmdline,int ascii)

    {
    int status;
    /* K2PDFOPT_CONVERSION *k2conv; */
    /* static char *funcname="k2gui_main"; */

#if (WILLUSDEBUGX & 0x2000)
printf("@k2gui_main, k2conv=%p, k2settings=%p\n",k2conv0,&k2conv0->k2settings);
#endif
    willusgui_init();
    willusgui_set_instance(hInstance);
    k2gui=&_k2gui;
    k2gui_init();
    k2gui_osdep_init(k2gui);
    /*
    k2gui->osdep.hinst = hInstance;
    k2gui->osdep.hprevinst = hPrevInstance;
    */
    k2gui->k2conv=k2conv0;
    k2gui->env=env;

    /* Put command-line settings into k2gui->k2conv->k2settings */
    /*
    willus_mem_alloc_warn((void **)&k2conv,sizeof(K2PDFOPT_CONVERSION),funcname,10);
    k2pdfopt_settings_init(&k2conv->k2settings);
    */
    if (k2gui->k2conv->k2settings.restore_last_settings==1
          || (k2gui->k2conv->k2settings.restore_last_settings==-1
                && parse_cmd_args(k2gui->k2conv,env,cmdline,NULL,3,1)==1))
        {
        k2gui_get_settings(0,&k2gui->k2conv->k2settings,&k2gui->cmdxtra);
        parse_cmd_args(k2gui->k2conv,&k2gui->cmdxtra,NULL,NULL,1,1);
        }
    parse_cmd_args(k2gui->k2conv,env,cmdline,NULL,1,1);
    /*
    ** v2.20
    ** Capture non-gui part of command-line options to k2gui->cmdxtra and 
    ** change k2gui->k2conv->k2settings so that only the GUI cmd-line params
    ** take effect... a bit kludgey.
    */
    {
    STRBUF guicmds;
    K2PDFOPT_CONVERSION k2c;
    K2PDFOPT_SETTINGS _k2inited,*k2inited;
    
    k2inited=&_k2inited;
    k2pdfopt_settings_init(k2inited);
    strbuf_clear(&k2gui->cmdxtra);
    strbuf_init(&guicmds);
/*
printf("k2inited->vertical_break_threshold=%g\n",k2inited->vertical_break_threshold);
printf("k2gui->k2conv->k2settings.vertical_break_threshold=%g\n",k2gui->k2conv->k2settings.vertical_break_threshold);
printf("k2inited->src_trim=%d, k2gui->srctrim=%d\n",k2inited->src_trim,k2gui->k2conv->k2settings.src_trim);
*/
    k2pdfopt_settings_get_cmdline(&guicmds,&k2gui->k2conv->k2settings,k2inited,&k2gui->cmdxtra);
/*
printf("guicmds='%s', k2gui->cmdxtra='%s'\n",guicmds.s,k2gui->cmdxtra.s);
*/
    k2pdfopt_settings_init(&k2gui->k2conv->k2settings);
    k2pdfopt_conversion_init(&k2c);
    parse_cmd_args(&k2c,&guicmds,NULL,NULL,1,1);
    k2pdfopt_settings_copy(&k2gui->k2conv->k2settings,&k2c.k2settings);
    }
    k2gui->opfontsize = k2gui->k2conv->k2settings.dst_fontsize_pts;
    if (fabs(k2gui->opfontsize)<1e-8)
        k2gui->opfontsize=12.;
    /*
    k2pdfopt_settings_copy(&k2gui->k2conv->k2settings,&k2conv->k2settings);
    willus_mem_free((double **)&k2conv,funcname);
    */
    /* k2gui_main_window_init(k2conv->k2settings.guimin ? SW_MINIMIZE : SW_SHOWNORMAL,hInstance); */
    k2gui_main_window_init(!k2gui->k2conv->k2settings.guimin);
    /* k2wingui_paint(k2wingui->hwnd); */
    k2gui->started=1;
    /*
    ** Process messages
    */
    k2gui->active=1;
    status=k2gui_osdep_window_proc_messages(&k2gui->mainwin,NULL,0,NULL);
    k2gui->active=0;
    strbuf_free(&k2gui->cmdxtra);
    k2gui_close();
    willusgui_close();
    return(status);
    }


/*
** Put up a dialog box with a message in it.
*/
int k2gui_alertbox(int retval,char *title,char *message)

    {
#ifdef HAVE_WIN32_API
    return(k2gui_messagebox(retval,title,message));
#else
    return(retval);
#endif
    }


void k2gui_get_custom_name(int index,char *name,int maxlen)

    {
    char envvar[32];
    int i;

    sprintf(envvar,"K2PDFOPT_CUSTOM%d",index);
    if (wsys_get_envvar_ex(envvar,name,maxlen)!=0)
        {
        sprintf(envvar,"Custom &%d",index);
        strncpy(name,envvar,maxlen-1);
        name[maxlen-1]='\0';
        return;
        }
    for (i=0;name[i]!='\0' && name[i]!=';';i++);
    name[i]='\0';
    }


static int k2gui_winposition_get(WILLUSGUIRECT *rect)

    {
    char buf[64];

    rect->left=rect->right=rect->top=rect->bottom=0;
    if (wsys_get_envvar_ex("K2PDFOPT_WINPOS",buf,63))
        return(0);
    if (sscanf(buf,"%d %d %d %d",&rect->left,&rect->top,&rect->right,&rect->bottom)==4)
        {
        if (rect->left<0 || rect->right-rect->left<100 || rect->top<0
                         || rect->bottom-rect->top<100)
            return(0);
        return(1);
        }
    return(0);
    }


static void k2gui_winposition_save(void)

    {
    WILLUSGUIRECT rect;
    char buf[64];

    willusgui_window_get_rect(&k2gui->mainwin,&rect);
    sprintf(buf,"%d %d %d %d",rect.left,rect.top,rect.right,rect.bottom);
    wsys_set_envvar("K2PDFOPT_WINPOS",buf,0);
    }
   
 
/*
** Get custom options from button.
*/
/*
void k2gui_get_custom_options(int index,K2PDFOPT_SETTINGS *k2settings,
                              STRBUF *extra)
    
    {
    static char *funcname="k2gui_get_custom_options";
    char *p;
    char envvar[32];
    int i;
    STRBUF *settings,_settings;

    settings=&_settings;
    strbuf_init(settings);
    strbuf_clear(settings);
    strbuf_clear(extra);
    sprintf(envvar,"K2PDFOPT_CUSTOM%d",index);
    willus_mem_alloc_warn((void **)&p,2048,funcname,10);
    if (wsys_get_envvar_ex(envvar,p,2047)==0)
        {
        for (i=0;p[i]!='\0' && p[i]!=';';i++);
        if (p[i]!='\0')
            {
            for (i++;p[i]!='\0' && p[i]!=';';i++)
                {
                char buf[2];
                buf[0]=p[i];
                buf[1]='\0';
                strbuf_cat(settings,buf);
                }
            if (p[i]!='\0')
                {
                for (i++;p[i]!='\0';i++)
                    {
                    char buf[2];
                    buf[0]=p[i];
                    buf[1]='\0';
                    strbuf_cat(extra,buf);
                    }
                }
            }
        }
    willus_mem_free((double **)&p,funcname);
    k2gui_cmdline_to_settings(k2settings,settings);
    strbuf_free(settings);
    }
*/


static void k2gui_set_button_defaults(void)

    {
    static char *defbuttons[] =
        {
        "2-column paper;-mode 2col;",
        "Trim Margins;-mode fw;",
        ""
        };
    int i;

    for (i=0;defbuttons[i][0]!='\0';i++)
        {
        char envname[32],buf[512];

        sprintf(envname,"K2PDFOPT_CUSTOM%d",i+1);
        if (wsys_get_envvar_ex(envname,buf,511) < 0)
            wsys_set_envvar(envname,defbuttons[i],0);
        }
    }


static void k2gui_get_settings(int index,K2PDFOPT_SETTINGS *k2settings,STRBUF *extra)

    {
    char envname[32];
    K2PDFOPT_CONVERSION *k2conv;
    char *pcmd,*pxtra;
    char *cmdbuf;
    STRBUF cmdline;
    static char *funcname="k2gui_get_settings";
    int i;

    willus_mem_alloc_warn((void **)&k2conv,sizeof(K2PDFOPT_CONVERSION),funcname,10);
    k2pdfopt_conversion_init(k2conv);
    sprintf(envname,"K2PDFOPT_CUSTOM%d",index);
    willus_mem_alloc_warn((void **)&cmdbuf,4096,funcname,10);
    wsys_get_envvar_ex(envname,cmdbuf,4095);
#if (WILLUSDEBUGX & 0x2000)
printf("Getenv: %s=%s\n",envname,cmdbuf);
#endif
    for (i=0;cmdbuf[i]!='\0' && cmdbuf[i]!=';';i++);
    if (cmdbuf[i]=='\0')
        pcmd=pxtra=&cmdbuf[i];
    else
        {
        pcmd=&cmdbuf[i+1];
        for (i=0;pcmd[i]!='\0' && pcmd[i]!=';';i++);
        if (pcmd[i]=='\0')
            pxtra=&pcmd[i];
        else
            {
            pcmd[i]='\0';
            pxtra=&pcmd[i+1];
            }
        }
    strbuf_clear(extra);
    strbuf_cpy(extra,pxtra);
    strbuf_init(&cmdline);
    strbuf_cpy(&cmdline,pcmd);
#if (WILLUSDEBUGX & 0x2000)
printf("   parsing '%s'\n",cmdline.s);
#endif
    parse_cmd_args(k2conv,&cmdline,NULL,NULL,1,1);
#if (WILLUSDEBUGX & 0x2000)
printf("    k2conv->k2settings.gui=%d\n",k2conv->k2settings.gui);
#endif
    k2pdfopt_settings_copy(k2settings,&k2conv->k2settings);
#if (WILLUSDEBUGX & 0x2000)
printf("    k2settings->gui=%d\n",k2settings->gui);
#endif
    strbuf_free(&cmdline);
    willus_mem_free((double **)&cmdbuf,funcname);
    k2pdfopt_conversion_close(k2conv);
    willus_mem_free((double **)&k2conv,funcname);
    }


static void k2gui_save_settings(int index,K2PDFOPT_SETTINGS *k2settings,STRBUF *extra,char *name)

    {
    STRBUF *settings,_settings;
    STRBUF *envvar,_envvar;
    char customname[32];
    char envname[32];
    int status;

    if (name==NULL || name[0]=='\0')
        {
        k2gui_get_custom_name(index,customname,31);
        status=k2gui_get_text("Save Custom Settings",
                          "Enter name for custom settings:",
                          "*Save","Cancel",customname,31);
        if (status!=1)
            return;
        }
    else
        {
        strncpy(customname,name,31);
        customname[31]='\0';
        }
    settings=&_settings;
    envvar=&_envvar;
    strbuf_init(settings);
    strbuf_init(envvar);
    k2gui_settings_to_cmdline(settings,k2settings);
    strbuf_sprintf(envvar,"%s;%s;%s",customname,
                       settings->s==NULL?"":settings->s,
                       extra->s==NULL?"":extra->s);
    sprintf(envname,"K2PDFOPT_CUSTOM%d",index);
#if (WILLUSDEBUGX & 0x2000)
printf("Setenv %s=%s\n",envname,envvar->s);
#endif
    wsys_set_envvar(envname,envvar->s,0);
    }


void k2gui_settings_to_cmdline(STRBUF *cmdline,K2PDFOPT_SETTINGS *k2settings)

    {
    K2PDFOPT_SETTINGS _k2inited,*k2inited;
    k2inited=&_k2inited;
    k2pdfopt_settings_init(k2inited);
    k2pdfopt_settings_get_cmdline(cmdline,k2settings,k2inited,NULL);
    }


/*
static void k2gui_cmdline_to_settings(K2PDFOPT_SETTINGS *k2settings,STRBUF *cmdline)

    {
    K2PDFOPT_CONVERSION *k2conv;
    static char *funcname="k2gui_cmdline_to_settings";

    willus_mem_alloc_warn((void **)&k2conv,sizeof(K2PDFOPT_CONVERSION),funcname,10);
    k2pdfopt_conversion_init(k2conv);
    parse_cmd_args(k2conv,NULL,cmdline,NULL,1,1);
    k2pdfopt_settings_copy(k2settings,&k2conv->k2settings);
    k2pdfopt_conversion_close(k2conv);
    willus_mem_free((double **)&k2conv,funcname);
    }
*/


static void k2gui_init(void)

    {
    int i;

    k2gui->mainwin.handle=NULL;
    willusgui_font_init(&k2gui->font);
    k2gui->preview_processing=0;
    k2gui->started=0;
    k2gui->active=0;
    /*
    k2gui->osdep.hinst = NULL;
    k2gui->osdep.hprevinst = NULL;
    */
    k2gui->ncontrols=0;
    k2gui->k2conv=NULL;
    k2gui->env=NULL;
    /* k2gui->cmdline=NULL; */
    strbuf_init(&k2gui->cmdxtra);
    for (i=0;i<8;i++)
        k2gui->prevthread[i]=NULL;
    bmp_init(&k2gui->pbitmap);
    bmp_init(&k2gui->pworking);
    k2gui_preview_bitmap_message(&k2gui->pworking,728,840,0.1,"WORKING...");
    bmp_init(&k2gui->pviewbitmap);
    k2gui_preview_bitmap_message(&k2gui->pviewbitmap,728,840,0.1,"PREVIEW\n\nWINDOW");
    k2gui->pbitmap.width=k2gui->pbitmap.height=0;
    k2gui->pbitmap.bpp=24;
    }


static void k2gui_close(void)

    {
    bmp_free(&k2gui->pviewbitmap);
    bmp_free(&k2gui->pworking);
    bmp_free(&k2gui->pbitmap);
    }


int k2gui_active(void)

    {
    if (k2gui==NULL)
        return(0);
    return(k2gui->active);
    }


static void k2gui_background_bitmap_fill(void)

    {
    WILLUSGUIRECT rect;

    willusgui_window_get_rect(&k2gui->mainwin,&rect);
    bmp_init(&k2gui->bgbmp);
    k2gui->bgbmp.type=WILLUSBITMAP_TYPE_WIN32;
    k2gui->bgbmp.width=rect.right-rect.left;
    k2gui->bgbmp.height=rect.bottom-rect.top;
    k2gui->bgbmp.bpp=24;
    bmp_alloc(&k2gui->bgbmp);
    bmp_gradient_fill(&k2gui->bgbmp);
    }


static void bmp_gradient_fill(WILLUSBITMAP *bmp)

    {
    int row;

    for (row=0;row<bmp->height;row++)
        {
        unsigned char *p;
        int col;
        p=bmp_rowptr_from_top(bmp,row);
        for (col=0;col<bmp->width;col++,p+=3)
            {
            double x,y,r0,r,g,b;

            x=(double)row/bmp->height;
            y=(double)col/bmp->width;
            r0=sqrt((x*x+y*y)/2.);
            b=g=255.9-70.*r0;
            r=224.9-200.*r0;
            if (b<0)
                b=0;
            if (g<0)
                g=0;
            if (r<0)
                r=0;
            p[0]=(int)b;
            p[1]=(int)g;
            p[2]=(int)r;
            }
        }

    /* Test pattern */
    /*
    bmp_fill(bmp,255,255,255);
    for (row=0;row<bmp->height;row+=10)
        {
        unsigned char *p;
        p=bmp_rowptr_from_top(bmp,row);
        for (col=0;col<bmp->width;col++,p+=3)
            {
            p[0]=(255*5-row*3)%255;
            p[1]=0;
            p[2]=0;
            }
        }
    for (col=0;col<bmp->width;col+=10)
        {
        unsigned char *p;
        p=bmp_rowptr_from_top(bmp,row);
        for (row=0;row<bmp->height;row++)
            {
            p=bmp_rowptr_from_top(bmp,row)+col*3;
            p[0]=0;
            p[1]=0;
            p[2]=(255*5-col*3)%255;
            }
        }
    */
    }

/*
static int k2gui_window_size_changed(WILLUSGUIWINDOW *window)

    {
    static int winsize_width=0;
    static int winsize_height=0;
    WILLUSGUIRECT *rect,_rect;

    rect=&_rect;
    if (!willusgui_window_get_rect(window,rect))
        return(0);
    if (rect->right-rect->left != winsize_width || rect->bottom-rect->top != winsize_height)
        {
        winsize_width = rect->right-rect->left;
        winsize_height = rect->bottom-rect->top;
        return(1);
        }
    return(0);
    }
*/


/*
** Show normal size if normal_size is not zero.  Otherwise, show minimized.
*/
static void k2gui_main_window_init(int normal_size)

    {
    WILLUSGUIRECT dtrect;
    int k2w,k2h,dtw,dth,i;

    /* Fill in a couple settings buttons if they are not already used */
    k2gui_set_button_defaults();
    /* Get desktop rectangle */
    willusgui_get_desktop_workarea(&dtrect);
    if (k2gui_winposition_get(&k2gui->mainwin.rect))
        {
        int w,h;

        /* Sanity check environment variable settings -- new in v2.13 */
        w=k2gui->mainwin.rect.right - k2gui->mainwin.rect.left + 1;
        h=k2gui->mainwin.rect.bottom - k2gui->mainwin.rect.top + 1;
        if (w < K2WIN_MINWIDTH)
            w = K2WIN_MINWIDTH;
        if (w > dtrect.right - dtrect.left + 1)
            w = dtrect.right - dtrect.left + 1;
        if (h < K2WIN_MINHEIGHT)
            h = K2WIN_MINHEIGHT;
        if (h > dtrect.bottom - dtrect.top + 1)
            h = dtrect.bottom - dtrect.top + 1;
        if (k2gui->mainwin.rect.left < 0)
            k2gui->mainwin.rect.left = 0;
        if (k2gui->mainwin.rect.top < 0)
            k2gui->mainwin.rect.top = 0;
        if (k2gui->mainwin.rect.left+w > dtrect.right)
            k2gui->mainwin.rect.left = dtrect.right-w;
        /* v2.20 bug fix -- change w to h */
        if (k2gui->mainwin.rect.top+h > dtrect.bottom)
            k2gui->mainwin.rect.top = dtrect.bottom-h;
        k2gui->mainwin.rect.right = k2gui->mainwin.rect.left + w - 1;
        k2gui->mainwin.rect.bottom = k2gui->mainwin.rect.top + h - 1;
        }
    else
        {
        dtw=dtrect.right-dtrect.left;
        dth=dtrect.bottom-dtrect.top;
        k2h = dth*0.6;
        if (k2h < K2WIN_MINHEIGHT)
            k2h = K2WIN_MINHEIGHT;
        if (k2h > dth)
            k2h = dth;
        k2w = k2h*K2WIN_MINWIDTH/K2WIN_MINHEIGHT;
        k2gui->mainwin.rect.left=(dtw-k2w)/2;
        k2gui->mainwin.rect.top=(dth-k2h)/2;
        k2gui->mainwin.rect.right=k2gui->mainwin.rect.left+k2w-1;
        k2gui->mainwin.rect.bottom=k2gui->mainwin.rect.top+k2h-1;
        }
    /* Done earlier now... */
    /* k2gui_get_settings(0,&k2gui->k2conv->k2settings,&k2gui->cmdxtra); */
    k2gui_osdep_main_window_init(&k2gui->mainwin,normal_size);
    willusgui_window_set_focus(&k2gui->mainwin);
    for (i=0;i<k2gui->ncontrols;i++)
        {
        if (in_string(k2gui->control[i].name,"convert all")>=0)
            {
            willusgui_window_set_focus(&k2gui->control[i]);
            break;
            }
        }
    }


static void k2gui_window_menus_init(WILLUSGUIWINDOW *win)

    {
    static char *menus[] = {"_File","&Add Source File...","Add &Folder...","&Save Settings...",
                                    "&Restore Settings...","E&xit",
                            "_Help","&Website help page...","&Command-line Options...",
                                    "&PDF File Info",
#ifdef HAVE_TESSERACT_LIB
                                    "&Tesseract Training File Info",
#endif
                                    "&About k2pdfopt...",
                            ""};

    willusgui_window_menus_init(win,menus);
    }


/*
** Button being held down?
*/
void k2gui_control_select(int index,int selected)

    {
/*
printf("@k2gui_control_select, index=%d, selected=%d\n",index,selected);
*/
    if (selected)
        {
        k2gui->control_selected=index;
        k2gui->time_selected_ms=0;
        }
    else
        {
        /* Make sure the selected button is being de-selected, otherwise ignore the deselection */
        if (k2gui->control_selected==index)
            {
            k2gui->control_selected=-1;
            k2gui->time_selected_ms=0;
            }
        }
    }


void k2gui_timer_event(void)

    {
/*
printf("@k2gui_timer_event, selected=%d, ms=%d...\n",k2gui->control_selected,k2gui->time_selected_ms);
*/
    if (k2gui->control_selected>=0)
        {
        k2gui->time_selected_ms += 200;
        /* Button held down for > 1.5 seconds */
        if (k2gui->time_selected_ms > 1500)
            {
            int index;

            index=k2gui->control_selected;
            if ((k2gui->control[index].flags&7) > 0)
                {
                int preset;

                preset=k2gui->control[index].flags&7;
                /* Store local copy because it can change during processing */
                k2gui_save_settings(preset,&k2gui->k2conv->k2settings,&k2gui->cmdxtra,NULL);
                k2gui_get_custom_name(preset,k2gui->control[index].name,31);
                /* Refresh button label */
                willusgui_control_redraw(&k2gui->control[index],0);
                k2gui->control_selected = -1;
                k2gui->time_selected_ms = 0;
                }
            }
        }
    }


static int needs_redraw=0;  /* Keep track of whether main window needs to be redrawn */
void k2gui_process_message(WILLUSGUIMESSAGE *message)

    {
    static int changing = 0;
    static int change_type = -1;
    int i,action,control_index;
    WILLUSGUICONTROL *control;
    K2PDFOPT_SETTINGS *k2settings;
    static char *funcname="k2gui_process_message";

#if (WILLUSDEBUGX & 0x2000000)
k2dprintf("@k2gui_process_message: action=%d\n",message->guiaction);
#endif
    k2settings=&k2gui->k2conv->k2settings;
    control=message->control;
    action=message->guiaction;
    control_index=-1;
    for (i=0;i<k2gui->ncontrols;i++)
        if (k2gui->control[i].handle == control->handle)
            {
            control_index = i;
            break;
            }
    if (control_index>=0)
        control=&k2gui->control[control_index];
    if (action==WILLUSGUIACTION_SETFOCUS)
        {
#if (WILLUSDEBUGX & 0x2000)
printf("Got WILLUSGUIACTION_SETFOCUS.\n");
#endif
        if (control!=NULL
              && (control->type == WILLUSGUICONTROL_TYPE_EDITBOX
                   || control->type == WILLUSGUICONTROL_TYPE_UPDOWN
                   || control->type == WILLUSGUICONTROL_TYPE_UPDOWN2))
{
#if (WILLUSDEBUGX & 0x2000)
printf("Selecting all in control %d.\n",control_index);
#endif
            willusgui_control_text_select_all(control);
}
        return;
        }
    /*
    ** If Enter pressed, find the associated control.
    */
    if (action==WILLUSGUIACTION_ENTER_PRESS)
        {
        control=k2gui_control_with_focus(&control_index);
        if (control_index>=0)
            action=WILLUSGUIACTION_CONTROL_PRESS;
        else
            control=NULL;
        }
    if (action==WILLUSGUIACTION_RBUTTONDOWN)
        {
        /* See if right-click on any of the control labels */
/*
printf("Mouse at %d,%d\n",message->param[0],message->param[1]);
*/
        for (i=0;i<k2gui->ncontrols;i++)
            {
            WILLUSGUIRECT rect;

            if (k2gui->control[i].type==WILLUSGUICONTROL_TYPE_CHECKBOX)
                continue;
            willusgui_control_draw_label(&k2gui->control[i],&rect);
/*
printf("label[%d]='%s' (%d,%d) - (%d,%d)\n",i,k2gui->control[i].name,rect.left,rect.top,rect.right,rect.bottom);
*/
            if (message->param[0]<rect.left || message->param[0]>rect.right)
                continue;
            if (message->param[1]<rect.top || message->param[1]>rect.bottom)
                continue;
/*
printf("fits!\n");
*/
            k2gui_contextmenu_by_control(&k2gui->control[i]);
            return;
            }
        return;
        }
    if (control->type==WILLUSGUICONTROL_TYPE_UPDOWN && action==WILLUSGUIACTION_CONTROL_PRESS)
        action=WILLUSGUIACTION_UPDOWN_EDIT;
    switch (action)
        {
        case WILLUSGUIACTION_CREATE:
            k2gui_window_menus_init(&k2gui->mainwin);
            willusgui_window_timer_init(&k2gui->mainwin,200); /* Send timer event every 200 ms */
            willusgui_window_accept_draggable_files(&k2gui->mainwin);
            break;
        case WILLUSGUIACTION_CONTEXTMENU:
            k2gui_contextmenu_by_control(control);
            break;
        case WILLUSGUIACTION_CONTROL_PRESS:
            {
            if (control_index<0)
                return;
            if (control->type==WILLUSGUICONTROL_TYPE_DROPDOWNLIST)
                {
                char buf[64];

                willusgui_control_dropdownlist_get_selected_item(control,buf);
                if (in_string(control->name,"units")>0)
                    {
                    int *x,ii,dpi;
                    double *xf;

                    if (control->name[0]=='w')
                        {
                        x=&k2settings->dst_userwidth_units;
                        xf=&k2settings->dst_userwidth;
                        }
                    else
                        {
                        x=&k2settings->dst_userheight_units;
                        xf=&k2settings->dst_userheight;
                        }
                    dpi=k2settings->dst_userdpi;
                    for (ii=0;unitname[ii][0]!='\0';ii++)
                        if (!strcmp(buf,unitname[ii]))
                            break;
                    if (unitname[ii][0]!='\0' && (*x)!=ii)
                        {
                        double srcin;
                        /*
                        int jj;
                        DEVPROFILE *dp;
                        char buf[32];

                        for (jj=0;jj<k2gui->ncontrols;jj++)
                            if (!strcmp(k2gui->control[jj].name,"Device"))
                                break;
                        willusgui_control_dropdownlist_get_selected_item(&k2gui->control[jj],buf);
                        dp=devprofile_get(buf);
                        */
                        srcin = (control->name[0]=='w') ? 8.5 : 11;
                        if (ii==UNITS_SOURCE || ii==UNITS_TRIMMED || ii==UNITS_OCRLAYER)
                            {
                            if ((*x)==UNITS_CM)
                                (*xf) /= srcin*2.54;
                            else if ((*x)==UNITS_INCHES)
                                (*xf) /= srcin;
                            else if ((*x)==UNITS_PIXELS)
                                (*xf) /= dpi*srcin;
                            }
                        else if (ii==UNITS_PIXELS)
                            {
                            if ((*x)==UNITS_SOURCE || (*x)==UNITS_TRIMMED || (*x)==UNITS_OCRLAYER)
                                (*xf) *= srcin*dpi;
                            else if ((*x)==UNITS_CM)
                                (*xf) *= dpi/2.54;
                            else if ((*x)==UNITS_INCHES)
                                (*xf) *= dpi;
                            }
                        else /* user selected inches or cm */
                            {
                            double scale;
                            scale = (ii==UNITS_CM ? 2.54 : 1.0);
                            if ((*x)==UNITS_SOURCE || (*x)==UNITS_TRIMMED || (*x)==UNITS_OCRLAYER)
                                (*xf) *= srcin*scale;
                            else if ((*x)==UNITS_INCHES)
                                (*xf) *= 2.54;
                            else if ((*x)==UNITS_CM)
                                (*xf) /= 2.54;
                            else
                                (*xf) /= (dpi/scale);
                            }
                        (*x)=ii;
                        k2gui_update_controls();
                        }
                    return;
                    }
                }

            /* User selected a new device? */
            if (!strcmp(control->name,"Device"))
                {
                if (k2gui_set_device_from_listbox(control))
                    k2gui_update_controls();
                return;
                }

            /* Preview page number */
            if (!strcmp(control->name,"previewpage"))
                {;
                char buf[32];

                willusgui_control_get_text(control,buf,31);
                k2settings->preview_page=atoi(buf);
                if (k2settings->preview_page<1)
                    k2settings->preview_page=1;
                sprintf(buf,"%d",k2settings->preview_page);
                willusgui_control_set_text(control,buf);
                return;
                }

            /* User selected a new mode? */
            if (!strcmp(control->name,"Mode"))
                {
                char buf[32];
                WILLUSGUICONTROL *devcontrol;

                devcontrol=k2gui_control_by_name("device");
                willusgui_control_dropdownlist_get_selected_item(control,buf);
                if (buf[0]!='\0')
                    {
#if (WILLUSDEBUGX & 0x2000)
printf("k2gui_process_message:  -mode %s\n",buf);
#endif
                    k2settings_sprintf(NULL,k2settings,"-mode def");
                    if (devcontrol!=NULL)
                        k2gui_set_device_from_listbox(devcontrol);
                    if (stricmp(buf,"default"))
                        k2settings_sprintf(NULL,k2settings,"-mode %s",buf);
#if (WILLUSDEBUGX & 0x2000)
printf("k2gpm:  settings->textwrap=%d\n",k2settings->text_wrap);
printf("settings->native=%d\n",k2settings->use_crop_boxes);
printf("settings->src_trim=%d\n",k2settings->src_trim);
#endif
                    k2gui_update_controls();
                    }
                return;
                }

            if (control->type==WILLUSGUICONTROL_TYPE_EDITBOX)
                {
                /* Copy page range to pagelist[] with no spaces */
                if (!strcmp(control->name,"pages"))
                    {
                    char *buf;
                    int i,j;
                    willus_mem_alloc_warn((void **)&buf,1024,funcname,10);
                    willusgui_control_get_text(control,buf,1023);
                    /* v2.13 new check */
                    if (!stricmp(buf,"(all)") || !stricmp(buf,"all"))
                        buf[0]='\0';
                    if (strcmp(buf,k2settings->pagelist))
                        {
                        for (i=j=0;buf[i]!='\0';i++)
                            if (buf[i]!=' ' && buf[i]!='\t')
                                k2settings->pagelist[j++]=buf[i];
                        k2settings->pagelist[j]='\0';
                        k2gui_update_controls();
                        }
                    willus_mem_free((double **)&buf,funcname);
                    }
                else if (!strnicmp(control->name,"cbox",4))
                    {
                    int cbindex;
                    K2CROPBOX *box;
                    char *buf;
                    int ib;

                    willus_mem_alloc_warn((void **)&buf,1024,funcname,10);
                    willusgui_control_get_text(control,buf,1023);
                    cbindex=atoi(&control->name[strlen(control->name)-1])-1;
                    box=&k2settings->cropboxes.cropbox[cbindex];
                    ib=-1;
                    if (in_string(control->name,"left")>=0)
                        ib=0;
                    else if (in_string(control->name,"top")>=0)
                        ib=1;
                    else if (in_string(control->name,"width")>=0)
                        ib=2;
                    else if (in_string(control->name,"height")>=0)
                        ib=3;
                    if (ib>=0)
                        k2parsecmd_set_value_with_units(buf,&box->box[ib],&box->units[ib],UNITS_INCHES);
                    else
                        strcpy(box->pagelist,buf);
                    willus_mem_free((double **)&buf,funcname);
                    }
                else if (!strcmp(control->name,"env"))
                    {
                    /* Nothing to do--must press buttons */
                    }
                else if (!strcmp(control->name,"extra"))
                    {
                    char *buf;
                    willus_mem_alloc_warn((void **)&buf,1024,funcname,10);
                    willusgui_control_get_text(control,buf,1023);
                    strbuf_cpy(&k2gui->cmdxtra,buf);
                    willus_mem_free((double **)&buf,funcname);
                    }
                else if (!strcmp(control->name,"linebreakval"))
                    {
                    char buf[32];
                    willusgui_control_get_text(control,buf,31);
                    if (k2settings->word_spacing<0)
                        k2settings->word_spacing = -fabs(atof(buf));
                    else
                        k2settings->word_spacing = fabs(atof(buf));
                    k2gui_update_controls();
                    }
                else if (!strcmp(control->name,"nthreads"))
                    {
                    char buf[32];
                    willusgui_control_get_text(control,buf,31);
                    if (buf[0]!='\0' && buf[strlen(buf)-1]=='%')
                        {
                        buf[strlen(buf)-1]='\0';
                        k2settings->nthreads = -atoi(buf);
                        }
                    else if (buf[0]=='\0')
                        k2settings->nthreads = -50;
                    else
                        k2settings->nthreads = atoi(buf);
                    k2gui_update_controls();
                    }
                else if (!strcmp(control->name,"landscapepages"))
                    {
                    willusgui_control_get_text(control,k2settings->dst_landscape_pages,1023);
                    clean_line(k2settings->dst_landscape_pages);
                    k2gui_update_controls();
                    }
                return;
                }

            if (control->type==WILLUSGUICONTROL_TYPE_CHECKBOX)
                {
                int checked;

                control->attrib ^= WILLUSGUICONTROL_ATTRIB_CHECKED;
                checked = (control->attrib & WILLUSGUICONTROL_ATTRIB_CHECKED);
                if (!strcmp(control->name,"straighten"))
                    k2settings->src_autostraighten = checked ? 4. : -1.;
                else if (!strcmp(control->name,"break"))
                    k2settings->dst_break_pages= checked ? 2 : 1;
#ifdef HAVE_GHOSTSCRIPT
                else if (!strcmp(control->name,"ppgs"))
                    k2settings->ppgs= checked ? 1 : 0;
#endif
                else if (!strcmp(control->name,"autocrop"))
                    k2settings->autocrop = checked ? 1 : 0;
                else if (!strcmp(control->name,"color"))
                    {
                    k2settings->dst_color= checked ? 1 : 0;
                    if (k2settings->use_crop_boxes)
                        k2gui_alertbox(0,"Cannot turn off color output",
                                   "Color output is always on for native PDF output.");
                    }
                else if (!strcmp(control->name,"landscape"))
                    k2settings->dst_landscape = checked ? 1 : 0;
                else if (!strcmp(control->name,"opfontsize"))
                    k2settings->dst_fontsize_pts= checked ? k2gui->opfontsize : 0.;
                else if (!strcmp(control->name,"native"))
                    {
                    k2settings->use_crop_boxes = checked ? 1 : 0;
                    if (k2settings->use_crop_boxes)
                        {
#ifdef HAVE_OCR_LIB
                        k2settings->dst_ocr=0;
#endif
                        k2settings->text_wrap=0;
                        }
#ifdef HAVE_OCR_LIB
                    else
                        {
                        if (k2settings->dst_ocr==0)
                            k2settings->dst_ocr='m';
                        }
#endif
                    }
                else if (!strcmp(control->name,"r2l"))
                    k2settings->src_left_to_right = checked ? 0 : 1;
                else if (!strcmp(control->name,"linebreak"))
                    k2settings->word_spacing = checked ? -fabs(k2settings->word_spacing) 
                                                       : fabs(k2settings->word_spacing);
                else if (!strcmp(control->name,"markup"))
                    k2settings->show_marked_source = checked ? 1 : 0;
                else if (!strcmp(control->name,"wrap"))
                    {
                    k2settings->text_wrap = checked ? 2 : 0;
                    if (k2settings->text_wrap)
                        k2settings->use_crop_boxes=0;
                    }
#ifdef HAVE_OCR_LIB
                else if (!strcmp(control->name,"ocr"))
                    {
                    k2settings->dst_ocr = checked ? 't' : 'm';
                    if (k2settings->dst_ocr)
                        k2settings->use_crop_boxes=0;
                    }
#endif
                else if (!strcmp(control->name,"evl"))
                    k2settings->erase_vertical_lines = checked ? 1 : 0;
                else if (!strcmp(control->name,"ehl"))
                    k2settings->erase_horizontal_lines = checked ? 1 : 0;
                else if (!strcmp(control->name,"bpm"))
                    k2settings->dst_break_pages = checked ? 4 : 1;
                else if (!strcmp(control->name,"autorot"))
                    k2settings->src_rot = checked ? SRCROT_AUTO : SRCROT_AUTOPREV;
                else if (!strcmp(control->name,"defects"))
                    k2settings->defect_size_pts = checked ? 1.5 : 0.75;
                else if (!strnicmp(control->name,"cboxactive",10))
                    {
                    K2CROPBOX *box;
                    int cbindex;

                    cbindex=control->name[10]-'1';
                    box=&k2settings->cropboxes.cropbox[cbindex];
                    box->cboxflags ^= K2CROPBOX_FLAGS_NOTUSED;
                    }
                else if (!strnicmp(control->name,"cboxignore",10))
                    {
                    K2CROPBOX *box;
                    int cbindex;

                    cbindex=control->name[10]-'1';
                    box=&k2settings->cropboxes.cropbox[cbindex];
                    box->cboxflags ^= K2CROPBOX_FLAGS_IGNOREBOXEDAREA;
                    }
                k2gui_update_controls();
                }
                 
            /*
            ** Process a button press
            ** (See k2gui_timer_event for processing of presets.)
            */
            if (control->type==WILLUSGUICONTROL_TYPE_BUTTON)
                {
                int preset;

                preset=control->flags&7;
                /* Is it a preset button? */
                if (preset > 0)
                    {
                    k2gui_get_settings(preset,k2settings,&k2gui->cmdxtra);
                    k2gui_update_controls();
                    }
                else if (!stricmp(control->name,"&info"))
                    k2gui_display_info();
                else if (in_string(control->name,"add folder")>=0)
                    k2gui_add_folder();
                else if (in_string(control->name,"add file")>=0)
                    k2gui_add_files();
                else if (in_string(control->name,"remove item")>=0)
                    {
                    int maxsel;

                    maxsel=k2gui->k2conv->k2files.n;
                    if (maxsel>0)
                        {
                        int *selected;
                        int i,n;
                        WILLUSGUICONTROL *filelistbox;

                        willus_mem_alloc_warn((void **)&selected,sizeof(int)*maxsel,funcname,10);
                        for (i=0;i<k2gui->ncontrols;i++)
                            if (!stricmp(k2gui->control[i].name,"file list"))
                                break;
                        if (i>=k2gui->ncontrols)
                            k2gui_error_out("Can't find file list control!");
                        filelistbox = &k2gui->control[i];
                        n=willusgui_control_listbox_get_selected_items_count(filelistbox,
                                                                            selected,maxsel);
                        for (i=0;i<n;i++)
                            {
                            char buf[512];
                            willusgui_control_listbox_get_item_text(filelistbox,selected[i],buf);
                            k2pdfopt_files_remove_file(&k2gui->k2conv->k2files,buf);
                            }
                        willus_mem_free((double **)&selected,funcname);
                        filebox_populate();
                        }
                    }
                else if (in_string(control->name,"restore defaults")>=0)
                    {
                    int status;
                    status=willusgui_message_box(&k2gui->mainwin,
                           "Restore Default Settings",
                           "This will restore all GUI settings to the defaults.",
                           "*&OK","&CANCEL",NULL,
                           NULL,0,24.,320,0xffc0c0,NULL,NULL,1);
                    if (status==1)
                        {
                        /* Restore Defaults */
                        k2pdfopt_settings_init(&k2gui->k2conv->k2settings);
                        strbuf_clear(&k2gui->cmdxtra);
                        k2gui_update_controls();
                        }
                    }
                else if (in_string(control->name,"cboxselect")>=0)
                    {
                    int maxsel;
                    int cbindex;
                    K2CROPBOX *box;

                    cbindex=control->name[10]-'1';
                    box=&k2settings->cropboxes.cropbox[cbindex];
                    maxsel=k2gui->k2conv->k2files.n;
                    if (maxsel>0)
                        {
                        int *selected;
                        int i,n,index;
                        char filename[MAXFILENAMELEN];
                        double margins[6];
                        WILLUSGUICONTROL *filelistbox;

                        willus_mem_alloc_warn((void **)&selected,sizeof(int)*maxsel,funcname,10);
                        for (i=0;i<k2gui->ncontrols;i++)
                            if (!stricmp(k2gui->control[i].name,"file list"))
                                break;
                        if (i>=k2gui->ncontrols)
                            k2gui_error_out("Can't find file list control!");
                        filelistbox = &k2gui->control[i];
                        n=willusgui_control_listbox_get_selected_items_count(filelistbox,
                                                                            selected,maxsel);
                        if (n==0)
                            index=0;
                        else
                            index=selected[0];
                        willus_mem_free((double **)&selected,funcname);
                        willusgui_control_listbox_get_item_text(filelistbox,index,filename);
                        /*
                        for (i=0;i<4;i++)
                            margins[i]=k2settings->srccropmargins.box[i];
                        */
                        for (i=0;i<4;i++)
                            margins[i]=box->box[i];
                        {
                        char optpagelist[256];
                        static int bcolors[3]={0x6060b0,0xf0f0f0,0xf0f0f0};
                        int status,np;

                        strncpy(optpagelist,box->pagelist,255);
                        optpagelist[255]='\0';                    
                        np=k2file_get_num_pages(filename);
                        if (np!=1)
                            status=willusgui_message_box(&k2gui->mainwin,"Optional page range",
                                        "Specify the page range to display:",
                                        "*&OK","&Cancel","",optpagelist,255,18,600,
                                         0xe0e0e0,bcolors,NULL,1);
                        else
                            status=1;
                        if (status==1)
                            {
                            n=k2gui_overlay_get_crop_margins(k2gui,filename,optpagelist,margins);
                            if (n)
                                {
                                WILLUSGUIRECT wrect,rect;

                                willusgui_window_get_rect(&k2gui->mainwin,&wrect);
                                for (i=0;i<4;i++)
                                    box->box[i]=margins[i];
                                if (margins[4]<0.)
                                    margins[4]=0.;
                                if (margins[5]<0.)
                                    margins[5]=0.;
                                sprintf(optpagelist,
                                        "\r\nCrop-box:\r\n-cbox %gin,%gin,%gin,%gin\r\n\r\n"
                                        "Margins:\r\n-m %gin,%gin,%gin,%gin",
                                        margins[0],margins[1],margins[2],margins[3],
                                        margins[0],margins[1],margins[4],margins[5]);
                                rect=wrect;
                                rect.left = (wrect.left+wrect.right)/2-300;
                                rect.right = rect.left+600;
                                rect.top = (wrect.top+wrect.bottom)/2-90;
                                rect.bottom = rect.top + 180;
                                willusgui_message_box(&k2gui->mainwin,"Selected crop box",
                                        "Command-line options:",
                                        "*&OK","","",optpagelist,255,18,rect.right-rect.left,
                                         0xe0e0e0,bcolors,&rect,1);
                                k2gui_update_controls();
                                }
                            }
                        }
                        }
                    else
                        k2gui_messagebox(0,"Overlay","No source files selected.");
                    }
                else if (in_string(control->name,"convert all")>=0)
                    {
                    if (k2gui->k2conv->k2files.n<=0)
                        k2gui_messagebox(0,"Convert","No files selected for conversion.");
                    else
                        {
                        WILLUSGUICONTROL *flcontrol;
                        /* Launch conversion dialog box and do the conversion */
                        k2gui_cbox_do_conversion(k2gui);
                        k2gui_cbox_freelist();
                        /* Clear file list if everything converted okay. */
                        /*
                        ** v2.02--don't clear the file list anymore in case user
                        **        wants to try some different cmd-line options for
                        **        the conversion.
                        */
                        /*
                        if (k2gui_cbox_conversion_successful())
                            k2pdfopt_files_clear(&k2gui->k2conv->k2files);
                        */
                        filebox_populate();
                        flcontrol=k2gui_control_by_name("file list");
                        willusgui_control_close(flcontrol);
                        k2gui_update_controls();
                        }
                    }
                else if (!stricmp(control->name,"pre&view") || !stricmp(control->name,"Cancel"))
                    {
                    if (!k2gui->preview_processing && k2gui->k2conv->k2files.n<=0)
                        k2gui_messagebox(0,"Convert","No files selected for conversion.");
                    k2gui_preview_start();
                    k2gui_update_controls();
                    }
                else if (!stricmp(control->name,"opclear"))
                    {
                    control=k2gui_control_by_name("opfolder");
                    if (control!=NULL)
                        {
                        strcpy(k2settings->dst_opname_format,"%s_k2opt");
                        k2gui_update_controls();
                        }
                    }
                else if (!stricmp(control->name,"opselect"))
                    {
                    control=k2gui_control_by_name("opfolder");
                    if (control!=NULL)
                        {
                        char foldername[MAXFILENAMELEN];
                        int status;

                        status=willusgui_folder_select(foldername,MAXFILENAMELEN-1);
                        if (status)
                            {
                            char basename[MAXFILENAMELEN];
                            strcpy(basename,"%b_k2opt");
                            wfile_fullname(k2settings->dst_opname_format,foldername,basename);
                            k2gui_update_controls();
                            }
                        }
                    }
                else if (!stricmp(control->name,"restore"))
                    {
                    char *buf;
                    control=k2gui_control_by_name("env");
                    if (control!=NULL)
                        {
                        willus_mem_alloc_warn((void **)&buf,1024,funcname,10);
                        wsys_get_envvar_ex("K2PDFOPT",buf,1023);
/*
printf("K2PDFOPT ('%s') --> editcontrol\n",buf);
*/
                        willusgui_control_set_text(control,buf);
                        willus_mem_free((double **)&buf,funcname);
                        }
                    }
                else if (!stricmp(control->name,"save"))
                    {
                    char *buf;
                    control=k2gui_control_by_name("env");
                    if (control!=NULL)
                        {
                        willus_mem_alloc_warn((void **)&buf,1024,funcname,10);
                        willusgui_control_get_text(control,buf,1023);
/*
printf("K2PDFOPT <-- '%s'\n",buf);
*/
                        wsys_set_envvar("K2PDFOPT",buf,0);
                        willus_mem_free((double **)&buf,funcname);
                        }
                    }
                else if (!stricmp(control->name,"_magplus_"))
                    {
                    control=k2gui_control_by_name("previewwin");
                    willusgui_sbitmap_change_size(control,1);
                    }
                else if (!stricmp(control->name,"_magminus_"))
                    {
                    control=k2gui_control_by_name("previewwin");
                    willusgui_sbitmap_change_size(control,-1);
                    }
                else if (!stricmp(control->name,"_fitpage_"))
                    {
                    control=k2gui_control_by_name("previewwin");
                    willusgui_sbitmap_change_size(control,0);
                    }
                }
            break;
            }
        case WILLUSGUIACTION_UPDOWN2_LEFT:
        case WILLUSGUIACTION_UPDOWN2_DLEFT:
        case WILLUSGUIACTION_UPDOWN2_RIGHT:
        case WILLUSGUIACTION_UPDOWN2_DRIGHT:
            {
            int del;
            char buf[16];

            del=1;
            if (action==WILLUSGUIACTION_UPDOWN2_DLEFT || action==WILLUSGUIACTION_UPDOWN2_DRIGHT)
                del*=10;
            if (action==WILLUSGUIACTION_UPDOWN2_LEFT || action==WILLUSGUIACTION_UPDOWN2_DLEFT)
                del=-del;
            k2settings->preview_page += del;
            if (k2settings->preview_page<1)
                k2settings->preview_page=1;
            sprintf(buf,"%d",k2settings->preview_page);
            willusgui_control_set_text(control,buf);
            willusgui_control_redraw(control,0);
            break;
            }
        case WILLUSGUIACTION_UPDOWN_UP:
        case WILLUSGUIACTION_UPDOWN_DOWN:
        case WILLUSGUIACTION_UPDOWN_EDIT:
            {
            int sn,vtype,*xi;
            double del,min,max,*xf;
            char fmt[16];
            char buf[32];

            if (control_index<0)
                return;
            sn = action==WILLUSGUIACTION_UPDOWN_UP ? 1 : -1;
            vtype=-1;
            del=1.;
            xf=NULL;
            xi=NULL;
            min=0.;
            max=-1.;
            if (!strcmp(control->name,"devwidth"))
                {
                strcpy(fmt,"%d");
                del = (k2settings->dst_userwidth_units==0) ? 10. : 0.1;
                xf=&k2settings->dst_userwidth;
                min = del;
                max = -1.;
                vtype=1;
                }
            else if (!strcmp(control->name,"devheight"))
                {
                strcpy(fmt,"%d");
                del = (k2settings->dst_userheight_units==0) ? 10. : 0.1;
                xf=&k2settings->dst_userheight;
                min = del;
                max = -1.;
                vtype=1;
                }
            else if (!strcmp(control->name,"devdpi"))
                {
                strcpy(fmt,"%d");
                del = 10.;
                xi=&k2settings->dst_userdpi;
                min = del;
                max = -1.;
                vtype=0;
                }
            else if (!strcmp(control->name,"maxcols"))
                {
                strcpy(fmt,"%d");
                del=1.;
                xi=&k2settings->max_columns;
                min=1;
                max=4;
                vtype=0;
                }
            else if (!strcmp(control->name,"ddr"))
                {
                strcpy(fmt,"%.1f");
                del=0.1;
                xf=&k2settings->dst_display_resolution;
                min=0.1;
                max=10.;
                vtype=1;
                }
            else if (!strcmp(control->name,"opfontsizeval"))
                {
                strcpy(fmt,"%.1f");
                del=0.5;
                xf=&k2gui->opfontsize;
                min=1.;
                max=99.;
                vtype=1;
                }
            if (action==WILLUSGUIACTION_UPDOWN_EDIT)
                willusgui_control_get_text(control,buf,31);
            if (vtype>=0)
                {
                /* For crop margins, -1 = default, so change to zero */
                if (!strncmp(control->name,"crop",4) && (*xf)<0.)
                    (*xf) = 0.;
                if (vtype==0)
                    {
                    int inew;

                    if (action==WILLUSGUIACTION_UPDOWN_EDIT)
                        inew = atoi(buf);
                    else
                        inew = (*xi)+sn*del;
                    if (inew < min)
                        inew = min;
                    if (max > 0. && inew>max)
                        inew = max;
                    if (inew!=(*xi))
                        {
                        (*xi)=inew;
                        k2gui_update_controls();
                        }
                    break;
                    }
                else
                    {
                    double fnew;

                    if (action==WILLUSGUIACTION_UPDOWN_EDIT)
                        fnew = atof(buf);
                    else
                        fnew = (*xf)+sn*del;
                    if (fnew < min)
                        fnew = min;
                    if (max > 0. && fnew>max)
                        fnew = max;
                    if (fabs(fnew-(*xf))>1e-4)
                        {
                        /* For crop margins, zero = default, so change to -1 */
                        /*
                        if (!strncmp(control->name,"crop",4) && fnew<=1e-8)
                            fnew = -1.;
                        */
                        (*xf)=fnew;
                        if (!strcmp(control->name,"opfontsizeval") && fabs(k2settings->dst_fontsize_pts)>1e-8)
                            k2settings->dst_fontsize_pts=fnew;
                        k2gui_update_controls();
                        }
                    break;
                    }
                }
            break;
            }
        case WILLUSGUIACTION_LOSTFOCUS:
            /* If moved off "extra args" text, store it. */
            if (!strcmp(k2gui->control[control_index].name,"extra args"))
                {
                int len;
                len=willusgui_control_get_textlen(&k2gui->control[control_index]);
                if (len>0)
                    {
                    strbuf_ensure(&k2gui->cmdxtra,len+2);
                    willusgui_control_get_text(&k2gui->control[control_index],k2gui->cmdxtra.s,
                                                 len+1);
                    }
                else
                    strbuf_clear(&k2gui->cmdxtra);
                }
            break;
        case WILLUSGUIACTION_MENU_SELECTION:
            switch (message->param[0])
                {
                case 700:  /* Add source file */
                    k2gui_add_files();
                    break;
                case 701:  /* Add folder */
                    k2gui_add_folder();
                    break;
                case 702:
                    k2gui_save_settings_to_file();
                    break;
                case 703:
                    k2gui_restore_settings_from_file();
                    break;
                case 704:
                    k2gui_quit();
                    break;
                case 710:
                    willusgui_start_browser("http://willus.com/k2pdfopt/help/");
                    break;
                case 711:  /* Command-line options */
                    {
                    char *buf;
                    int status;
                    double fontsize;
                    WILLUSGUIRECT wrect,rect;

                    willusgui_window_get_rect(&k2gui->mainwin,&wrect);
                    willus_mem_alloc_warn((void **)&buf,k2usage_len(),funcname,10);
                    k2usage_to_string(buf);
                    rect=wrect;
                    rect.left += 40;
                    rect.right -= 40;
                    rect.top += 40;
                    rect.bottom -= 40;
                    fontsize=(wrect.right-wrect.left)*.020;
                    if (fontsize > 16.)
                        fontsize = 16.;
                    winmbox_set_font("Courier New");
                    status=willusgui_message_box(&k2gui->mainwin,
                           "K2pdfopt Command-line Options",
                           "K2pdfopt Command-line Options",
                           "*&DISMISS","&GO TO WEBSITE",NULL,
                           buf,strlen(buf),
                           fontsize,
                           (wrect.right-wrect.left),
                           0xffb080,NULL,&rect,1);
                    winmbox_set_font("");
                    if (status==2)
                        willusgui_start_browser("http://willus.com/k2pdfopt/");
                    willus_mem_free((double **)&buf,funcname);
                    break;
                    }
                case 712:  /* PDF file info */
                    k2gui_display_info();
                    break;
#ifdef HAVE_TESSERACT_LIB
                /* Tesseract Training Files */
                case 713:
                    {
                    char *buf;
                    int status;
                    WILLUSGUIRECT rect;

                    winmbox_wait_end();
                    winmbox_wait(k2gui->mainwin.handle,"Checking files.  Please wait...",0);
                    ocrtess_debug_info(&buf,0);
                    winmbox_wait_end();
                    willusgui_window_get_rect(&k2gui->mainwin,&rect);
                    winmbox_set_font("Courier New");
                    status=willusgui_message_box(&k2gui->mainwin,
                           "Tesseract Training Files",
                           buf,"*&DISMISS","&TESSERACT FILES SITE",NULL,
                           NULL,0,
                           20.,(rect.right-rect.left),
                           0xb0ffb0,NULL,NULL,1);
                    willus_mem_free((double **)&buf,funcname);
                    if (status==2)
                        willusgui_start_browser("https://github.com/tesseract-ocr/tessdata/");
                    break;
                    }
                case 714:
#else
                case 713:
#endif
                /* About box */
                    {
                    char buf[256];
                    int status;
                    WILLUSGUIRECT rect;
                    k2sys_header(buf);
                    willusgui_window_get_rect(&k2gui->mainwin,&rect);
                    status=willusgui_message_box(&k2gui->mainwin,
                           "About K2pdfopt",
                           buf,"*&DISMISS","&GO TO WEBSITE",NULL,
                           NULL,0,
                           24.,(rect.right-rect.left),
                           0xb0ffb0,NULL,NULL,1);
                    if (status==2)
                        willusgui_start_browser("http://willus.com/k2pdfopt/");
                    break;
                    }
                }
            break;
        case WILLUSGUIACTION_GETMINSIZE:
            k2gui_window_minsize(&message->param[0],&message->param[1]);
            break;
        case WILLUSGUIACTION_WINDOWSIZECHANGE:
            {
            WILLUSGUIRECT rect,dtrect;
            int ww,new_width,new_height,dtw,dth;

            /* Get desktop rectangle -- v2.40 */
            willusgui_get_desktop_workarea(&dtrect);
            dtw=dtrect.right-dtrect.left;
            dth=dtrect.bottom-dtrect.top;
            new_width  = message->param[0];
            new_height = message->param[1];
            willusgui_window_get_rect(&k2gui->mainwin,&rect);
            ww=rect.right-rect.left;
            if (change_type < 0)
                change_type = (new_width != ww) ? 0 : 1;
            if (change_type==0)
                new_height = new_width*K2WIN_MINHEIGHT/K2WIN_MINWIDTH;
            else
                new_width = new_height*K2WIN_MINWIDTH/K2WIN_MINHEIGHT;
            /* v2.40--limit window size to fit screen */
            if (new_height > dth)
                {
                new_height = dth;
                new_width = new_height*K2WIN_MINWIDTH/K2WIN_MINHEIGHT;
                }
            if (new_width > dtw)
                {
                new_width = dtw;
                new_height = new_width*K2WIN_MINHEIGHT/K2WIN_MINWIDTH;
                }
            message->param[0]=new_width;
            message->param[1]=new_height;
            break;
            }
        case WILLUSGUIACTION_STARTING_RESIZE:
            /* Don't allow re-draw while window is changing size */
            /* willusgui_window_set_redraw(&k2gui->mainwin,0); */
            changing = 1;
            change_type = -1;
            break;
        case WILLUSGUIACTION_ENDING_RESIZE:
            /*
            willusgui_window_set_redraw(&k2gui->mainwin,1);
            if (k2gui_window_size_changed(&k2gui->mainwin))
                willusgui_control_redraw(&k2gui->mainwin,1);
            */
            changing = 0;
            if (needs_redraw)
                willusgui_control_redraw(&k2gui->mainwin,1);
            break;
        case WILLUSGUIACTION_DROPFILES:
            {
            char **ptr;
            /* int ca; */

            ptr=willusgui_get_dropped_files(message->ptr[0]);
            if (ptr!=NULL)
                {
                int i;
                for (i=0;ptr[i]!=NULL;i++)
                    k2gui_add_file(ptr[i]);
                willusgui_release_dropped_files(ptr);
                /*
                if (ca)
                    k2gui_alertbox(0,"Non-ASCII characters!",
                                   "Some file names had non-ASCII characters.\n\n"
                                   "These file names were not added.");
                */
                }
            break;
            }
        case WILLUSGUIACTION_REPAINT:
            if (control->handle==k2gui->mainwin.handle)
                k2gui_osdep_main_repaint(changing);  /* Calls k2gui_main_repaint() */
            break;
        /*
        ** Destroy main window last, which will send the WILLUSGUIACTION_DESTROY message.
        */
        case WILLUSGUIACTION_CLOSE:
        case WILLUSGUIACTION_ESC_PRESS:
            k2gui_save_settings(0,&k2gui->k2conv->k2settings,&k2gui->cmdxtra,"Last Settings");
            k2gui_winposition_save();
            k2gui_destroy_mainwin();
            break;
        case WILLUSGUIACTION_DESTROY:
            k2gui_quit();
            break;
        }
    }


static void k2gui_display_info(void)

    {
    int *selected;
    char *buf;
    char buf0[MAXFILENAMELEN+128];
    int *pagelist;
    int i,n,index,maxsel;
    WILLUSGUIRECT wrect,rect;
    WILLUSGUICONTROL *filelistbox;
    char filename[MAXFILENAMELEN];
    double fontsize;
    static char *funcname="k2gui_display_info";

    /* Get selected file name */
    maxsel=k2gui->k2conv->k2files.n;
    willus_mem_alloc_warn((void **)&selected,sizeof(int)*maxsel,funcname,10);
    for (i=0;i<k2gui->ncontrols;i++)
        if (!stricmp(k2gui->control[i].name,"file list"))
            break;
    if (i>=k2gui->ncontrols)
        k2gui_error_out("Can't find file list control!");
    filelistbox = &k2gui->control[i];
    n=willusgui_control_listbox_get_selected_items_count(filelistbox,
                                                        selected,maxsel);
    if (n==0)
        index=0;
    else
        index=selected[0];
    willus_mem_free((double **)&selected,funcname);
    willusgui_control_listbox_get_item_text(filelistbox,index,filename);
    if (filename[0]=='\0')
        {
        k2gui_messagebox(0,"Convert","No files selected for conversion.");
        return;
        }
    pagelist_get_array(&pagelist,k2gui->k2conv->k2settings.pagelist);
    wmupdfinfo_get(filename,pagelist,&buf);
    if (pagelist!=NULL)
        free(pagelist);
    if (buf==NULL)
        {
        buf=buf0;
        sprintf(buf,"FILE: %s\n\nCannot obtain information.\n",filename);
        }
    willusgui_window_get_rect(&k2gui->mainwin,&wrect);
    rect=wrect;
    rect.left += 40;
    rect.right -= 40;
    rect.top += 40;
    rect.bottom -= 40;
    fontsize=(wrect.right-wrect.left)*.020;
    if (fontsize > 16.)
        fontsize = 16.;
    winmbox_set_font("Courier New");
    willusgui_message_box(&k2gui->mainwin,
           "PDF File Info",
           filename,
           "*&DISMISS",NULL,NULL,
           buf,strlen(buf),
           fontsize,
           (wrect.right-wrect.left),
           0xffb080,NULL,&rect,1);
    winmbox_set_font("");
    if (buf!=NULL && buf!=buf0)
        free(buf);
    }


static int k2gui_set_device_from_listbox(WILLUSGUICONTROL *control)

    {
    char buf[32];

    willusgui_control_dropdownlist_get_selected_item(control,buf);
    if (buf[0]!='\0')
        {
        int dpc,j;

        dpc=devprofiles_count();
        for (j=0;j<dpc;j++)
            {
            if (!strcmp(buf,devprofile_name(j)))
                break;
            }
        if (j<dpc)
            {
            k2settings_sprintf(NULL,&k2gui->k2conv->k2settings,"-dev %s",devprofile_alias(j));
            return(1);
            }
        }
    return(0);
    }


static WILLUSGUICONTROL *k2gui_control_with_focus(int *index)

    {
    void *focuswin;
    int i;

    if (index!=NULL)
        (*index)=-999;
    focuswin = willusgui_control_handle_with_focus();
    if (focuswin==k2gui->mainwin.handle)
        {
        if (index!=NULL)
            (*index)=-1;
        return(&k2gui->mainwin);
        }
    for (i=0;i<k2gui->ncontrols;i++)
        if (focuswin==k2gui->control[i].handle)
            break;
    /* No recognizeable control has focus */
    if (i>=k2gui->ncontrols)
        return(NULL);
    if (index!=NULL)
        (*index)=i;
    return(&k2gui->control[i]);
    }


static void k2gui_update_controls(void)

    {
/*
char buf[256];
int status;
sprintf(buf,"@k2gui_update_controls, needs_redraw=%d",needs_redraw);
status=willusgui_message_box(&k2gui->mainwin,"Debug",buf,
"*&OK","","",NULL,0,24,600,0xe0e0e0,NULL,NULL,1);
printf("@k2gui_update_controls\n");
*/
    /* Make checkboxes consistent */
    if (k2gui!=NULL && k2gui->k2conv!=NULL)
        k2pdfopt_settings_quick_sanity_check(&k2gui->k2conv->k2settings);
    if (needs_redraw!=2)
        needs_redraw=1;
    willusgui_set_ime_notify(1);
    willusgui_control_redraw(&k2gui->mainwin,1);
    }


static void k2gui_error_out(char *message)

    {
    k2gui_messagebox(0,"Critical Error",message);
    exit(200);
    }


void k2gui_window_minsize(int *width_pixels,int *height_pixels)

    {
    (*width_pixels) = K2WIN_MINWIDTH;
    (*height_pixels) = K2WIN_MINHEIGHT;
    }


void k2gui_quit(void)

    {
    willusgui_send_quit_message();
    }


int k2gui_messagebox(int retval,char *title,char *fmt,...)

    {
    va_list args;
    char buf[500];
    static int bcolors[3]={0x6060b0,0xf0f0f0,0xf0f0f0};

    va_start(args,fmt);
    vsprintf(buf,fmt,args);
    willusgui_message_box(&k2gui->mainwin,title,buf,"*&OK",NULL,NULL,
                               NULL,0,24,600,0xe0e0e0,bcolors,NULL,1);
    return(retval);
    }


int k2gui_get_text(char *title,char *message,char *button1,char *button2,
                   char *textbuf,int maxlen)

    {
    static int bcolors[3] = {0x6060b0,0xf0f0f0,0xf0f0f0};
    int maxwidth,bgcolor;

    maxwidth=600;
    bgcolor=0xffffa0;
    return(willusgui_message_box(&k2gui->mainwin,title,message,button1,button2,NULL,
                                   textbuf,maxlen,k2gui->font.size,maxwidth,bgcolor,
                                   bcolors,NULL,1));
    }


void k2gui_add_file(char *filename)

    {
    k2pdfopt_files_add_file(&k2gui->k2conv->k2files,filename);
    willusgui_control_listbox_add_item(&k2gui->control[0],filename);
    }


static void k2gui_save_settings_to_file(void)

    {
    int status,size;
    char *filename;
    static char *funcname="k2gui_save_settings_to_file";
    static int bcolors[3]={0x6060b0,0xf0f0f0,0xf0f0f0};
    char buf[512];

    size=512;
    willus_mem_alloc_warn((void **)&filename,size,funcname,10);
    status=willusgui_file_select_dialog(filename,size-1,"Text files\0*.txt\0\0\0",
                                        "Select a save file","txt",1);
    if (status)
        {
        FILE *out;
        if (wfile_status(filename)!=0)
            {
            sprintf(buf,"File %s already exists?  Overwrite it?",filename);
            status=willusgui_message_box(&k2gui->mainwin,"Overwrite settings?",buf,
                                        "&YES (Overwrite)","*&NO (Cancel)","",
                                        NULL,0,24,600,0xe0e0e0,bcolors,NULL,1);
            if (status!=1)
                {
                willus_mem_free((double **)&filename,funcname);
                return;
                }
            }
        out=fopen(filename,"w");
        if (out==NULL)
            {
            sprintf(buf,"Cannot open file %s for overwriting.",filename);
            status=willusgui_message_box(&k2gui->mainwin,"Cannot write file",buf,
                                        "*&OK","","",NULL,0,24,600,0xe0e0e0,bcolors,NULL,1);
            }
        else
            {
            int i;

            for (i=-1;i<=4;i++)
                {
                char envname[128];
                char *cmdbuf;

                if (i<0)
                    strcpy(envname,"K2PDFOPT");
                else
                    sprintf(envname,"K2PDFOPT_CUSTOM%d",i);
                willus_mem_alloc_warn((void **)&cmdbuf,4096,funcname,10);
                if (!wsys_get_envvar_ex(envname,cmdbuf,4095))
                    fprintf(out,"%s=%s\n",envname,cmdbuf);
                willus_mem_free((double **)&cmdbuf,funcname);
                }
            if (fclose(out))
                {
                sprintf(buf,"Error writing settings to file %s.",filename);
                status=willusgui_message_box(&k2gui->mainwin,"Error",buf,
                                            "*&OK","","",NULL,0,24,600,0xe0e0e0,bcolors,NULL,1);
                }
            else
                {
                sprintf(buf,"Settings successfully saved to file %s.",filename);
                status=willusgui_message_box(&k2gui->mainwin,"Success",buf,
                                            "*&OK","","",NULL,0,24,600,0xe0e0e0,bcolors,NULL,1);
                }
            }
        }
    willus_mem_free((double **)&filename,funcname);
    }


static void k2gui_restore_settings_from_file(void)

    {
    int status,size;
    char *filename;
    static char *funcname="k2gui_restore_settings_from_file";
    static int bcolors[3]={0x6060b0,0xf0f0f0,0xf0f0f0};
    char buf[512];

    size=512;
    willus_mem_alloc_warn((void **)&filename,size,funcname,10);
    status=willusgui_file_select_dialog(filename,size-1,"Text files\0*.txt\0\0\0",
                                        "Select a settings file","txt",0);
    if (status)
        {
        FILE *f;
        f=fopen(filename,"r");
        if (f==NULL)
            {
            sprintf(buf,"Cannot open settings file %s for reading.",filename);
            status=willusgui_message_box(&k2gui->mainwin,"Cannot open settings file",buf,
                                        "*&OK","","",NULL,0,24,600,0xe0e0e0,bcolors,NULL,1);
            }
        else
            {
            char *cmdbuf;

            k2gui_clear_envvars();
            willus_mem_alloc_warn((void **)&cmdbuf,4096,funcname,10);
            while (fgets(cmdbuf,4095,f)!=NULL)
                {
                int i;

                if (cmdbuf[0]==';')
                    continue;
                /* Get rid of CR/LF */
                clean_line(cmdbuf);
                for (i=0;cmdbuf[i]!='\0' && cmdbuf[i]!='=';i++);
                if (cmdbuf[i]=='\0')
                    continue;
                cmdbuf[i]='\0';
                clean_line(cmdbuf);
                if (strnicmp(cmdbuf,"k2pdfopt",8))
                    continue;
                wsys_set_envvar(cmdbuf,&cmdbuf[i+1],0);
                }
            willus_mem_free((double **)&cmdbuf,funcname);
            fclose(f);
            /* Apply K2PDFOPT_CUSTOM0 */
            k2gui_get_settings(0,&k2gui->k2conv->k2settings,&k2gui->cmdxtra);
            parse_cmd_args(k2gui->k2conv,&k2gui->cmdxtra,NULL,NULL,1,1);
            /* Completely re-draw main window */
            force_repaint=1;
            k2gui_main_repaint(0);
            sprintf(buf,"Settings restored from file %s.",filename);
            status=willusgui_message_box(&k2gui->mainwin,"Success",buf,
                                        "*&OK","","",NULL,0,24,600,0xe0e0e0,bcolors,NULL,1);
            }
        }
    willus_mem_free((double **)&filename,funcname);
    }


static void k2gui_clear_envvars(void)

    {
    int i;

    wsys_set_envvar("K2PDFOPT","",0);
    for (i=0;i<=4;i++)
        {
        char buf[128];

        sprintf(buf,"K2PDFOPT_CUSTOM%d",i);
        wsys_set_envvar(buf,"",0);
        }
    }


static void k2gui_add_files(void)

    {
    static char *funcname="k2gui_add_files";
    char *filename;
    static char *allowed_files="PDF files\0*.pdf\0"
                               "DJVU files\0*.djvu\0"
                               "All files\0*\0\0\0";
    int size,status;

/*
printf("Calling wincomdlg...\n");
*/
    size=16384;
    willus_mem_alloc_warn((void **)&filename,size,funcname,10);
    status=willusgui_file_select_dialog(filename,size-1,allowed_files,"Select source file",
                                        "pdf",0);
    if (status)
        {
        char *p;
        char basepath[512];

        if (wfile_status(filename)!=2)
            k2gui_add_file(filename);
        else
            {
            strcpy(basepath,filename);
            for (p=&filename[strlen(filename)+1];p[0]!='\0';p=&p[strlen(p)+1])
                {
                char fullname[512];

                wfile_fullname(fullname,basepath,p);
/* printf("fullname = '%s'\n",fullname); */
                k2gui_add_file(fullname);
                }
            }
        }
    willus_mem_free((double **)&filename,funcname);
    }


static void k2gui_add_folder(void)

    {
    char foldername[MAXFILENAMELEN];
    int status;

    status=willusgui_folder_select(foldername,MAXFILENAMELEN-1);
    if (status)
        k2gui_add_file(foldername);
    }


static int k2gui_determine_fontsize(void)

    {
    WILLUSGUIRECT rect;
    int fontsize;

    willusgui_window_get_rect(&k2gui->mainwin,&rect);
    fontsize=(rect.right-rect.left)/47;
    if (fontsize > 28)
        fontsize = 28;
    return(fontsize);
    }


/*
** Add child windows / controls to main window
*/
static void k2gui_add_children(int already_drawn)

    {
    int i,j,nr,xmar,linesize,eheight,w,h,ww,fs,f2,f4,wmax;
    /*
    int wh;
    */
    int xmin,xmax,ymin,ymax;
    int x1,y1,x0,y0,w1,ybmax;
    int pmbw; /* Preview magnification button width */
    double fm;
    /* static double fontscale=0.; */
    WILLUSGUIRECT _crect,*crect;
    WILLUSGUICONTROL *control,*flcontrol;
    WILLUSGUICONTROL *focus_control;
    STRBUF *settings,_settings;
    K2PDFOPT_SETTINGS *k2settings;

/*    
char buf[256];
int status;
sprintf(buf,"@k2gui_add_children, already_drawn=%d",already_drawn);
status=willusgui_message_box(&k2gui->mainwin,"Debug",buf,
"*&OK","","",NULL,0,24,600,0xe0e0e0,NULL,NULL,1);
*/
/*
    {
    HWND dummy;
    DWORD bclass;

    dummy = CreateWindow("button",NULL,WS_CHILD|BS_OWNERDRAW,
                         0,0,0,0,k2wingui->hwnd,(HMENU)0,k2wingui->hinst,NULL);
    bclass = GetClassLong(dummy,GCL_STYLE);
    SetClassLong(dummy,GCL_STYLE,bclass & ~CS_DBLCLKS);
    DestroyWindow(dummy);
    }
*/
#if (WILLUSDEBUGX & 0x2000)
printf("@k2gui_add_children(already_drawn=%d)\n",already_drawn);
#endif
    if (!already_drawn)
        {
        focus_control=k2gui_control_with_focus(NULL);
        k2gui_destroy_children();
        }
    else
        {
        k2gui->ncontrols=0;
        focus_control=NULL;
        }
    /* Get command line */
    settings=&_settings;
    strbuf_init(settings);
    k2settings=&k2gui->k2conv->k2settings;
    k2gui_settings_to_cmdline(settings,k2settings);
    crect=&_crect;
    willusgui_window_get_useable_rect(&k2gui->mainwin,crect);
    ww = crect->right-crect->left;
    /*
    wh = crect->bottom-crect->top;
    */
    /* fontscale calculation:  For Calibri = 1.1, for Arial = 1.0 */
    /*
    if (fontscale<0.1)
        {
        WILLUSGUIRECT r1;
        WILLUSGUIFONT f1;
        int i,rectsum,ssum;

        willusgui_font_init(&f1);
        for (rectsum=ssum=0,i=10;i<25;i++)
            {
            f1.size=i;
            willusgui_font_get(&f1);
            willusgui_window_text_extents(&k2gui->mainwin,&f1,"0123456789",&r1);
            rectsum+=(r1.right-r1.left);
            ssum+=i;
            }
        willusgui_font_release(&f1);
        fontscale = (double)4.63*ssum/rectsum;
        if (fontscale < 0.8 || fontscale > 1.25)
            fontscale = 1.0;
        }
    k2gui->font.size=fs;
    */
    nr=(crect->bottom*.3-crect->top)/k2gui->font.size-3.;
    /* Windows weirdness--works best if nr is even */
    nr=nr&(~1);
    fs=k2gui->font.size;
    f2=fs/2;
    f4=fs/4;
    xmar = f2;
    eheight = fs*1.31;
    linesize = fs*1.45;
    x0 = crect->left+xmar;
    y0 = crect->top+fs*1.55;
    wmax = 0.6*ww;
    /*
    ** listbox[0].text = "File list"
    */
    for (i=0;i<1;i++)
        {
        int recreate;

        x1 = x0;
        w = wmax;
        y1 = y0;
        h = k2gui->font.size*(nr+1);
        control=&k2gui->control[k2gui->ncontrols];
        control->index=100+k2gui->ncontrols;
        k2gui->ncontrols++;
        recreate = (!already_drawn || control->handle==NULL);
        if (recreate)
            {
            willusgui_control_init(control);
            control->rect.left=x1;
            control->rect.top=y1;
            control->rect.right=x1+w-1;
            control->rect.bottom=y1+h-1;
            control->font.size = k2gui->font.size;
            willusgui_font_get(&control->font);
            strcpy(control->name,"File list");
            control->type=WILLUSGUICONTROL_TYPE_LISTBOX;
            control->parent=&k2gui->mainwin;
            strcpy(control->label,"File list");
            control->labelx=x1;
            control->labely=y1;
            control->labeljust=6;
            control->attrib=WILLUSGUICONTROL_ATTRIB_MULTISELECT;
            willusgui_control_create(control);
            filebox_populate();
            /* Windows Weirdness */
            control->rect.right++;
            control->rect.bottom=control->rect.top+k2gui->font.size*nr+2;
            }
        else
            willusgui_control_redraw(control,0);
        /*
        willusgui_window_text_render(&k2gui->mainwin,&k2gui->font,"File list",
                                        x1,y1,0x000000,-1,6,NULL);
        */
        flcontrol=control;
        }

    /* Buttons by file list */
    x0 = k2gui->control[k2gui->ncontrols-1].rect.right;
    y0 = k2gui->control[k2gui->ncontrols-1].rect.top - linesize;
    for (x1=x0,j=0,i=3;i>=0;i--,j++,x1-=(w+fs))
        {
        double xl;
        static char *button_label[4]={"&INFO","&ADD FILE","ADD FOLDER","&REMOVE ITEM"};
        WILLUSGUIRECT trect;

        xl = 1.00;
        willusgui_window_text_extents(&k2gui->mainwin,&k2gui->font,button_label[i],&trect);
        w = trect.right*1.2;
        if (i==0)
            w *= 1.25;
        y1 = y0;
        h = k2gui->font.size*(xl+.4);
        control=&k2gui->control[k2gui->ncontrols+i];
        if (!already_drawn)
            {
            willusgui_control_init(control);
            strcpy(control->name,button_label[i]);
            strcpy(control->label,button_label[i]);
            control->color=0xd0d0ff;
            control->font.size = k2gui->font.size*xl;
            willusgui_font_get(&control->font);
            control->rect.left = x1-w;
            control->rect.top = y1;
            control->rect.right = x1-1;
            control->rect.bottom = y1+h-1;
            control->type=WILLUSGUICONTROL_TYPE_BUTTON;
            control->index=100+k2gui->ncontrols+i;
            control->parent=&k2gui->mainwin;
            willusgui_control_create(control);
            }
        else
            willusgui_control_redraw(control,0);
        }
    k2gui->ncontrols+=j;

    /* Custom settings buttons */
    xmin=ymin=9999;
    xmax=ymax=0;
    wmax = 0.2*ww;
    x0 = k2gui->control[0].rect.right+f4;
    y0 = crect->top + f4/2;
    for (i=0;i<4;i++)
        {
        double xl;

        xl = 1.15;
        w = wmax*0.85;
        x1 = x0 + wmax/2 - w/2;
        y1 = y0 + linesize*1.3 + i*(xl+.7)*fs;
        h = fs*(xl+.4);
        control=&k2gui->control[k2gui->ncontrols];
        control->index=100+k2gui->ncontrols;
        k2gui->ncontrols++;
        if (!already_drawn)
            {
            willusgui_control_init(control);
            k2gui_get_custom_name(i+1,control->name,31);
            strcpy(control->label,control->name);
            control->flags=i+1;
            control->color=0xffffd0;
            control->font.size=k2gui->font.size*xl;
            willusgui_font_get(&control->font);
            control->rect.left = x1;
            control->rect.top = y1;
            control->rect.right = x1+w-1;
            control->rect.bottom = y1+h-1;
            control->type=WILLUSGUICONTROL_TYPE_BUTTON;
            control->parent=&k2gui->mainwin;
            }
        if (already_drawn)
            willusgui_control_redraw(control,0);
        else
            willusgui_control_create(control);
        if (x1<xmin)
            xmin=x1;
        if (x1+w>xmax)
            xmax=x1+w;
        if (y1<ymin)
            ymin=y1;
        if (y1+h>ymax)
            ymax=y1+h;
        }
    ymin -= linesize;
    ymax += k2gui->font.size/2;
    xmin -= k2gui->font.size/2;
    xmax += k2gui->font.size/2;
    {
    static char *cps="Custom Presets";
    static char *cah[1]={"(Press and hold to save.)"};
    WILLUSGUIFONT font;
    WILLUSGUIRECT r1;

    willusgui_window_text_extents(&k2gui->mainwin,&k2gui->font,cps,&r1);
    willusgui_window_text_render(&k2gui->mainwin,&k2gui->font,cps,
                              (xmax+xmin)/2-r1.right/2,ymin+fs/2,0,-1,6,NULL);
    willusgui_font_init(&font);
    font.size=(int)(fs*.7);
    willusgui_font_get(&font);
    for (y1=ymin+fs/2,i=0;i<1;i++)
        {
        WILLUSGUIRECT r2;
        willusgui_window_text_extents(&k2gui->mainwin,&font,cah[i],&r2);
        willusgui_window_text_render(&k2gui->mainwin,&font,cah[i],
                              (xmax+xmin)/2-r2.right/2,y1+r2.bottom,0,-1,6,NULL);
        y1 += r2.bottom*1.1;
        }
    willusgui_font_release(&font);
    /* Box around group of buttons */
    willusgui_window_draw_line(&k2gui->mainwin,(xmin+xmax)/2-r1.right/2-f4,ymin,xmin,ymin,0,0);
    willusgui_window_draw_line(&k2gui->mainwin,xmin,ymin,xmin,ymax,0,0);
    willusgui_window_draw_line(&k2gui->mainwin,xmin,ymax,xmax,ymax,0,0);
    willusgui_window_draw_line(&k2gui->mainwin,xmax,ymax,xmax,ymin,0,0);
    willusgui_window_draw_line(&k2gui->mainwin,xmax,ymin,(xmin+xmax)/2+r1.right/2+f4,ymin,0,0);
    }

    /*
    ** Device select menu
    */
    {
    int dpc;

    dpc=devprofiles_count();
    x1 = xmax + f4;
    y1 = crect->top + f4/2 + k2gui->font.size - 4;
    w = ww-f4-x1;
    h = k2gui->font.size*10;
    control=&k2gui->control[k2gui->ncontrols];
    control->index=100+k2gui->ncontrols;
    k2gui->ncontrols++;
    if (!already_drawn)
        {
        willusgui_control_init(control);
        strcpy(control->name,"Device");
        control->flags=0;
        control->color=0;
        control->font.size=k2gui->font.size;
        willusgui_font_get(&control->font);
        control->rect.left = x1;
        control->rect.top = y1;
        control->rect.right = x1+w-1;
        control->rect.bottom = y1+h-1;
        control->type=WILLUSGUICONTROL_TYPE_DROPDOWNLIST;
        control->parent=&k2gui->mainwin;
        strcpy(control->label,"Device:");
        control->labeljust=6;
        control->labelx=x1;
        control->labely=y1+2;
        willusgui_control_create(control);
        willusgui_control_listbox_clear(control);
        for (i=0;i<dpc;i++)
            willusgui_control_listbox_add_item(control,devprofile_name(i));
        }
    else
        willusgui_control_redraw(control,0);
    /* Determine selected device */
    if (settings->s!=NULL && (i=in_string(settings->s,"-dev "))>=0)
        {
        int j;
        char buf[16];

        for (j=i+5;j-(i+5)<16 && settings->s[j]!=' ' && settings->s[j]!='\0';j++)
            buf[j-(i+5)]=settings->s[j];
        buf[j-(i+5)]='\0';
        for (j=0;j<dpc;j++)
            if (!strcmp(devprofile_alias(j),buf))
                break;
        if (j<dpc)
            willusgui_control_listbox_select_item(control,devprofile_name(j));
        }
    else
        {
        int j;
        for (j=0;j<dpc;j++)
            if (!stricmp(devprofile_alias(j),K2PDFOPT_DEFAULT_DEVICE))
                break;
        if (j>=dpc)
            j=0;
        willusgui_control_listbox_select_item(control,devprofile_name(j));
        }
    }

    fm = 0.9;
    h=eheight*fm;
    if (h&1)
        h++;
    y1 += h*1.60;
    w1 = h*4.25;
    /* Device width up/down control */
    for (i=0;i<3;i++)
        {
        char buf[32];
        static char *names[]={"devwidth","devheight","devdpi"};
        static char *labels[]={"Width","Height","DPI"};

        control=&k2gui->control[k2gui->ncontrols];
        control->index=100+k2gui->ncontrols;
        k2gui->ncontrols++;
        if (!already_drawn)
            {
            willusgui_control_init(control);
            strcpy(control->name,names[i]);
            control->flags=0;
            control->color=0xffb060;
            control->font.size=k2gui->font.size*fm;
            willusgui_font_get(&control->font);
            control->rect.left = x1 + h*1.75;
            control->rect.top = y1;
            control->rect.right = x1 + w1;
            control->rect.bottom = y1+h-1;
            control->type=WILLUSGUICONTROL_TYPE_UPDOWN;
            control->parent=&k2gui->mainwin;
            strcpy(control->label,labels[i]);
            control->labeljust=0;
            control->labelx=x1;
            control->labely=y1 + h*.15;
            willusgui_control_create(control);
            }
        else
            willusgui_control_redraw(control,0);
        if (i==0)
            {
#if (WILLUSDEBUGX & 0x2000)
printf("dst_userwidth = %g\n",k2settings->dst_userwidth);
printf("dst_userwidth_units = %d\n",k2settings->dst_userwidth_units);
#endif
            if (k2settings->dst_userwidth_units==UNITS_PIXELS)
                sprintf(buf,"%d",(int)(k2settings->dst_userwidth+.5));
            else
                sprintf(buf,"%.2f",k2settings->dst_userwidth);
            }
        else if (i==1)
            {
            if (k2settings->dst_userheight_units==UNITS_PIXELS)
                sprintf(buf,"%d",(int)(k2settings->dst_userheight+.5));
            else
                sprintf(buf,"%.2f",k2settings->dst_userheight);
            }
        else
            sprintf(buf,"%d",k2settings->dst_userdpi);
        willusgui_control_set_text(control,buf);
        if (i<2)
            {
            int units;
            control=&k2gui->control[k2gui->ncontrols];
            control->index=100+k2gui->ncontrols;
            k2gui->ncontrols++;
            if (!already_drawn)
                {
                int ii;
                willusgui_control_init(control);
                strcpy(control->name,i==0?"widthunits":"heightunits");
                control->flags=0;
                control->color=0;
                control->font.size=k2gui->font.size*fm;
                willusgui_font_get(&control->font);
                control->rect.left = x1+w1+h/6;
                control->rect.top = y1-2;
                control->rect.right = control->rect.left + h*2.3;
                control->rect.bottom = control->rect.top+6*h;
                control->type=WILLUSGUICONTROL_TYPE_DROPDOWNLIST;
                control->parent=&k2gui->mainwin;
                /*
                strcpy(control->label,"Device:");
                control->labeljust=6;
                control->labelx=x1;
                control->labely=y1+2;
                */
                willusgui_control_create(control);
                willusgui_control_listbox_clear(control);
                for (ii=0;unitname[ii][0]!='\0';ii++)
                    willusgui_control_listbox_add_item(control,unitname[ii]);
                }
            else
                willusgui_control_redraw(control,0);
            units = i==0 ? k2settings->dst_userwidth_units
                         : k2settings->dst_userheight_units;
            if (units>=0 && units<=4)
                willusgui_control_listbox_select_item(control,unitname[units]);
            }
        y1 += h*1.3;
        }

    /*
    ** Mode select menu
    */
    {
    static char *modes[]={"default","copy","trim","fitwidth","fitpage","2-column","crop","concat",""};
    int nmodes;

    for (nmodes=0;modes[nmodes][0]!='\0';nmodes++);
    x1 = xmax + f4;
    w = ww-f4-x1;
    h = k2gui->font.size*10;
    y1 += k2gui->font.size*0.8;
    if (k2gui->font.size == 15)
       y1--;
    if (k2gui->font.size == 13)
       y1-=2;
    control=&k2gui->control[k2gui->ncontrols];
    control->index=100+k2gui->ncontrols;
    k2gui->ncontrols++;
    if (!already_drawn)
        {
        willusgui_control_init(control);
        strcpy(control->name,"Mode");
        control->flags=0;
        control->color=0;
        control->font.size=k2gui->font.size;
        willusgui_font_get(&control->font);
        control->rect.left = x1;
        control->rect.top = y1;
        control->rect.right = x1+w-1;
        control->rect.bottom = y1+h-1;
        control->type=WILLUSGUICONTROL_TYPE_DROPDOWNLIST;
        control->parent=&k2gui->mainwin;
        strcpy(control->label,"Conversion Mode:");
        control->labeljust=6;
        control->labelx=x1;
        control->labely=y1+2;
        willusgui_control_create(control);
        willusgui_control_listbox_clear(control);
        for (i=0;i<nmodes;i++)
            willusgui_control_listbox_add_item(control,modes[i]);
        }
    else
        willusgui_control_redraw(control,0);
    /* Determine selected mode */
/*
printf("settings->s='%s'\n",settings->s);
*/
    if (settings->s!=NULL && (i=in_string(settings->s,"-mode "))>=0)
        {
        int j;

        /* Kludge to correctly select "fitpage", v2.15 */
        if (!strnicmp(&settings->s[i+6],"fp",2) || !strnicmp(&settings->s[i+6],"fitp",4))
            j=4;
        /* v2.35--correctly set mode to "crop" */
        else if (!strnicmp(&settings->s[i+6],"cr",2))
            j=6;
        else if (!strnicmp(&settings->s[i+6],"cc",2) || !strnicmp(&settings->s[i+6],"con",3))
            j=7;
        else
            {
            for (j=0;j<nmodes;j++)
                if (tolower(settings->s[i+6])==tolower(modes[j][0]))
                    break;
            if (j>=nmodes)
                j=0;
            }
        willusgui_control_listbox_select_item(control,modes[j]);
        }
    else
        willusgui_control_listbox_select_item(control,modes[0]);
    }

    /*
    ** Preview pane
    */
    {
    int yb,ppleft,ppright,ppbottom,ppbb;

    ppleft=0; /* Avoid compiler warning */
    {
    x1 = xmin;
    w = ww-f4-x1;
    y1 += k2gui->font.size*1.8;
    h = crect->bottom - f4 - k2gui->font.size*1.2 - y1;
    control=&k2gui->control[k2gui->ncontrols];
    control->index=100+k2gui->ncontrols;
    k2gui->ncontrols++;
    if (!already_drawn || control->handle==NULL)
        {
        static int firstcall=1;
        int size;

        if (!firstcall)
            size=control->sbitmap_size;
        else
            size=0;
        if (!already_drawn)
            willusgui_control_init(control);
        if (!firstcall)
            control->sbitmap_size=size;
        firstcall=0;
        strcpy(control->name,"previewwin");
        control->flags=0;
        control->color=0;
        control->font.size=k2gui->font.size*2;
        willusgui_font_get(&control->font);
        ppleft=control->rect.left = x1;
        control->rect.top = y1;
        ppright=control->rect.right = x1+w-1;
        ppbottom=control->rect.bottom = y1+h-1;
        control->type=WILLUSGUICONTROL_TYPE_SCROLLABLEBITMAP;
        control->parent=&k2gui->mainwin;
        strcpy(control->label,"");
        /* Before creating, need bmp assigned. */
        if (k2gui->preview_processing)
            control->obmp=&k2gui->pworking;
        else if (k2gui->pbitmap.width==0)
            control->obmp=&k2gui->pviewbitmap;
        else
            control->obmp=&k2gui->pbitmap;
/*
printf("Calling control_create() for previewwin.\n");
*/
        willusgui_control_create(control);
/*
printf("    Preview bitmap created w/control->index=%d, index=%d\n",control->index,k2gui->ncontrols-1);
printf("    control->handle=%p\n",control->handle);
*/
        }
    else
        {
        willusgui_control_redraw(control,0);
        ppbottom=control->rect.bottom;
        }
    }

    /* Preview page number select */
    {
    WILLUSGUIRECT extents;
    static char *ppname="Preview page: ";

    control=&k2gui->control[k2gui->ncontrols];
    control->index=100+k2gui->ncontrols;
    k2gui->ncontrols++;
    willusgui_window_text_extents(&k2gui->mainwin,&k2gui->font,ppname,&extents);
    x1 = xmin+extents.right;
    y1 += h+1;
    h = eheight;
    if (!already_drawn)
        {
        willusgui_control_init(control);
        strcpy(control->name,"previewpage");
        control->flags=0;
        control->color=0xffd0ff;
        control->font.size=k2gui->font.size;
        willusgui_font_get(&control->font);
        control->rect.left = x1;
        control->rect.top = y1;
        control->rect.right = x1 + h*5;
        control->rect.bottom = y1+h-1;
        control->type=WILLUSGUICONTROL_TYPE_UPDOWN2;
        control->parent=&k2gui->mainwin;
        strcpy(control->label,ppname);
        control->labeljust=2;
        control->labelx=x1;
        control->labely=y1 + h*.15;
        willusgui_control_create(control);
        }
    else
        willusgui_control_redraw(control,0);
    ppbb=control->rect.bottom = y1+h-1;
    {
    char buf[16];

    if (k2settings->preview_page<1)
        k2settings->preview_page=1;
    sprintf(buf,"%d",k2settings->preview_page);
    willusgui_control_set_text(control,buf);
    x1 = control->rect.right+h*0.3;
    }
    }

    /* Preview button */
    for (i=0;i<1;i++)
        {
        control=&k2gui->control[k2gui->ncontrols];
        if (!already_drawn)
            {
            willusgui_control_init(control);
            control->rect.left=x1;
            control->rect.top=y1;
            control->rect.right=ppright;
            control->rect.bottom=ppbb;
            control->index=100+k2gui->ncontrols;
            control->font.size=k2gui->font.size;
            willusgui_font_get(&control->font);
            control->type=WILLUSGUICONTROL_TYPE_BUTTON;
            control->parent=&k2gui->mainwin;
            }
        control->color=k2gui->preview_processing?0x806080:0xffd0ff;
        strcpy(control->name,k2gui->preview_processing?"Cancel":"Pre&view");
        strcpy(control->label,control->name);
        if (already_drawn)
            willusgui_control_redraw(control,0);
        else
            willusgui_control_create(control);
        k2gui->ncontrols++;
        }

    /* Preview magnification buttons */
    pmbw = ww*.03;
    if (pmbw > 64)
        pmbw=64;
    else if (pmbw < 24)
        pmbw=24;
    for (yb=ppbottom,i=0;i<3;i++)
        {
        int bh,dx;
        static char *bname[3]={"_magminus_","_magplus_","_fitpage_"};

        dx = ww*.003;
        if (dx < 3)
            dx=3;
        if (i<=1)
            bh=pmbw;
        else
            bh=pmbw*35/32;
        control=&k2gui->control[k2gui->ncontrols];
        if (!already_drawn)
            {
            willusgui_control_init(control);
            control->rect.right=ppleft-dx;
            control->rect.bottom=yb;
            control->rect.left=control->rect.right-pmbw+1;
            control->rect.top=control->rect.bottom-bh+1;
            control->index=100+k2gui->ncontrols;
            control->font.size=k2gui->font.size;
            willusgui_font_get(&control->font);
            control->type=WILLUSGUICONTROL_TYPE_BUTTON;
            control->color=0xffd0ff;
            strcpy(control->name,bname[i]);
            strcpy(control->label,bname[i]);
            control->parent=&k2gui->mainwin;
            }
        if (already_drawn)
            willusgui_control_redraw(control,0);
        else
            willusgui_control_create(control);
        yb -= bh - dx + 4;
        k2gui->ncontrols++;
        }
    } /* End preview pane & buttons */


    /* Edit boxes */
    /*
    ** cmd arg box needs name:  "cmdline args"
    */
    {
    int bw;

    bw=eheight*2.5;
    x0=crect->left+xmar;
    y0=k2gui->control[0].rect.bottom+fs*1.4;
    w= k2gui->control[0].rect.right - x0;
    for (y1=y0-eheight*0.9,i=0;i<4;i++)
        {
        static char *label[]={"Output Folder:","Env. var:","Additional options:","Command-line version of options:"};
        static char *name[]={"opfolder","env","extra","cmdline args"};
        WILLUSGUIRECT r1;

        x1=x0;
        /* v2.32: Incorporate label into control */
        /*
        willusgui_window_text_render(&k2gui->mainwin,&k2gui->font,label[i],x1,
                                         y1+(i<2?eheight*.1:0),0,-1,0,NULL);
        willusgui_window_text_extents(&k2gui->mainwin,&k2gui->font,label[i],&r1);
        */
        control=&k2gui->control[k2gui->ncontrols];
        if (!already_drawn)
            {
            willusgui_control_init(control);
            if (i==0)
                control->attrib |= WILLUSGUICONTROL_ATTRIB_READONLY;
            else if (i==3)
                control->attrib |= (WILLUSGUICONTROL_ATTRIB_MULTILINE | WILLUSGUICONTROL_ATTRIB_READONLY);
            strcpy(control->label,label[i]);
            control->labelx=x1;
            control->labely=y1+(i<2?eheight*.1:0);
            control->labeljust=0;
            control->parent=&k2gui->mainwin;
            control->index=100+k2gui->ncontrols;
            control->font.size=k2gui->font.size;
            control->type=WILLUSGUICONTROL_TYPE_EDITBOX;
            willusgui_font_get(&control->font);
            strcpy(control->name,name[i]);
            willusgui_control_draw_label(control,&r1);
            r1.right = (r1.right-r1.left+1);
            r1.bottom -= (r1.bottom-r1.top+1);
            control->rect.left= i<2 ? x1+r1.right+f4 : x1;
            /* control->rect.top = i<2 ? y1-eheight*.9 : y1-eheight+linesize; */
            control->rect.top = i<2 ? y1 : y1+eheight*0.1+linesize*0.6;
            if (i==0)
                control->rect.right = control->rect.left + w-(r1.right+f4)-1-(bw+f4)*2;
            else if (i==1)
                control->rect.right = control->rect.left + w-(r1.right+f4)-1-(bw+f4)*2;
            else if (i==2)
                control->rect.right = control->rect.left + w-(r1.right+f4)-1;
            else
                control->rect.right = control->rect.left + w-1;
            control->rect.bottom = control->rect.top + (i<2 ? eheight-1 : eheight*2-1);
            }
        if (already_drawn)
            willusgui_control_redraw(control,0);
        else
            willusgui_control_create(control);
        k2gui->ncontrols++;
        if (i==0)
            {
            char foldername[MAXFILENAMELEN];

            wfile_basepath(foldername,k2settings->dst_opname_format);
            willusgui_control_set_text(control,foldername);
            }
        else if (i==1)
            {
            char *buf;
            static char *funcname="k2gui_add_children";
            willus_mem_alloc_warn((void **)&buf,1024,funcname,10);
            wsys_get_envvar_ex("K2PDFOPT",buf,1023);
            willusgui_control_set_text(control,buf);
            strbuf_cpy(k2gui->env,buf);
            willus_mem_free((double **)&buf,funcname);
            /* willusgui_control_set_text(control,k2gui->env->s==NULL?"":k2gui->env->s); */
            }
        else if (i==2)
{

#if (WILLUSDEBUGX & 0x2000)
printf("cmdxtra.s='%s'\n",k2gui->cmdxtra.s);
#endif

            willusgui_control_set_text(control,k2gui->cmdxtra.s==NULL?"":k2gui->cmdxtra.s);
}
        else if (i==3)
            {
            willusgui_control_set_text(control,settings->s==NULL?"":settings->s);
            /* y1 += eheight+linesize; */
            }

        /* Output folder select */ 
        if (i==0)
            {
            WILLUSGUIRECT r;
            int j;

            r=control->rect;
            for (j=0;j<2;j++)
                {
                control=&k2gui->control[k2gui->ncontrols];
                control->index=100+k2gui->ncontrols;
                k2gui->ncontrols++;
                if (!already_drawn)
                    {
                    willusgui_control_init(control);
                    control->rect.left=r.right + (j+1)*f4 + j*bw;
                    control->rect.top=r.top;
                    /* control->rect.right=r.left+w-(r1.right+f4)-1; */
                    control->rect.right=r.right + (j+1)*(bw+f4);
                    control->rect.bottom=r.bottom;
                    control->font.size=k2gui->font.size;
                    willusgui_font_get(&control->font);
                    control->type=WILLUSGUICONTROL_TYPE_BUTTON;
                    control->color=0xd0d0ff;
                    if (j==0)
                        {
                        strcpy(control->name,"opclear");
                        strcpy(control->label,"Clear");
                        }
                    else
                        {
                        strcpy(control->name,"opselect");
                        strcpy(control->label,"Select");
                        }
                    control->parent=&k2gui->mainwin;
                    }
                if (already_drawn)
                    willusgui_control_redraw(control,0);
                else
                    willusgui_control_create(control);
                }
            }

        /* Env. var buttons */
        if (i==1)
            {
            WILLUSGUIRECT r;
            int j;
            static char *bname[]={"Save","Restore"};

            r=control->rect;
            for (j=0;j<2;j++)
                {
                control=&k2gui->control[k2gui->ncontrols];
                control->index=100+k2gui->ncontrols;
                k2gui->ncontrols++;
                if (!already_drawn)
                    {
                    willusgui_control_init(control);
                    control->rect.left=r.right + (j+1)*f4 + j*bw;
                    control->rect.top=r.top;
                    /* control->rect.right=r.left+w-(r1.right+f4)-1; */
                    control->rect.right=r.right + (j+1)*(bw+f4);
                    control->rect.bottom=r.bottom;
                    control->font.size=k2gui->font.size;
                    willusgui_font_get(&control->font);
                    control->type=WILLUSGUICONTROL_TYPE_BUTTON;
                    control->color=0xd0d0ff;
                    strcpy(control->name,bname[j]);
                    strcpy(control->label,bname[j]);
                    control->parent=&k2gui->mainwin;
                    }
                if (already_drawn)
                    willusgui_control_redraw(control,0);
                else
                    willusgui_control_create(control);
                }
            }

        y1 = control->rect.bottom + ((int)(linesize*0.1)<2 ? 2 : (int)(linesize*0.1));
        }
    }


    /* More up/down selectors: columns, display resolution factor, crop margins */
    {
    WILLUSGUIFONT _font,*font;
    WILLUSGUIRECT te;
    int fsize,bmar;
    int xb[3],yb[3];

    fm = 1.0;
    fsize=k2gui->font.size*fm+.5;
    if (fsize<8)
        fsize=8;
    bmar=fsize/4;
    x0=crect->left+xmar;
    w= (k2gui->control[0].rect.right - x0)/2;
    y1 += eheight*0.1;

    font=&_font;
    willusgui_font_init(font);
    font->size=fsize;
    willusgui_font_get(font);
    willusgui_window_text_extents(&k2gui->mainwin,font,"Crop areas",&te);
    xb[0]=x0;
    yb[0]=y1+eheight*0.5;
    xb[1]=x0+te.right;
    yb[1]=yb[0]+fsize;
    xb[2]=yb[2]=0; /* Avoid compiler warning */
    willusgui_window_text_render(&k2gui->mainwin,&k2gui->font,"Crop areas",x0,y1+eheight*0.5,0x000000,-1,0,NULL);
    x0 +=  te.right+eheight*1.2;
    for (i=0;i<2;i++)
        {
        char buf[32];
        static char *names[]={"maxcols","ddr"};
        static char *labels[]={"Max columns:","Document Resolution Factor:"};

        control=&k2gui->control[k2gui->ncontrols];
        control->index=100+k2gui->ncontrols;
        k2gui->ncontrols++;
        if (!already_drawn)
            {

            willusgui_control_init(control);
            strcpy(control->name,names[i]);
            control->flags=0;
            control->color=0xffb060;
            control->font.size=fsize;
            willusgui_font_get(&control->font);
            willusgui_window_text_extents(&k2gui->mainwin,&control->font,labels[i],&te);
            control->rect.left=x0+te.right+eheight*.15;
            control->rect.top=y1;
            control->rect.right=control->rect.left+eheight*(i<2 ? 1.8 : 2.1);
            control->rect.bottom=control->rect.top+fsize+3;
            control->type=WILLUSGUICONTROL_TYPE_UPDOWN;
            control->parent=&k2gui->mainwin;
            strcpy(control->label,labels[i]);
            control->labeljust=0;
            control->labelx=x0;
            control->labely=y1 + h*.15;
            willusgui_control_create(control);
            }
        else
            willusgui_control_redraw(control,0);
        switch(i)
            {
            case 0:
                sprintf(buf,"%d",(int)k2settings->max_columns);
                break;
            case 1:
                sprintf(buf,"%.1f",k2settings->dst_display_resolution);
                break;
            }
        willusgui_control_set_text(control,buf);
        /*
        if (i==1 || i==5)
            {
            y1 += eheight*1.15;
            x0=crect->left+xmar;
            }
        else
        */
        x0=control->rect.right + eheight*0.5;
        }
    y1 += eheight*1.2;

    {
    int k,boxheight,boxgapv,boxgaph,fsize;
    double ytop,xleft;

    /* Apply K2SETTINGS cropboxes to GUI storage */
    fm = 0.78;
    fsize=k2gui->font.size*fm+.5;
    if (fsize<8)
        fsize=8;
    boxheight=eheight*fm+.5;
    boxgapv = boxheight*0.07;
    boxgaph = boxheight*0.15;
    if (boxgapv<2)
        boxgapv=2;
    if (boxgaph<4)
        boxgaph=4;
    x0=crect->left+xmar;
    w= (k2gui->control[0].rect.right - x0)/2;
    for (k=-1,ytop=y1;k<3;k++,ytop+=(k==0 ? boxheight*0.8 : boxheight+boxgapv))
        {
        static char *names[]={"cboxactive%d","cboxleft%d","cboxtop%d",
                              "cboxwidth%d","cboxheight%d",
                              "cboxpages%d","cboxignore%d","cboxselect%d",""};
        static char *labels[]={"Active","Left","Top","Width","Height","Page Range",
                               "Ignore","Select"};
        static char *position[]={"XXXXXX","XXXXXXXX","XXXXXXXX","XXXXXXXX","XXXXXXXX",
                                 "XXXXXXXXXXXXXX","XXXXXX","XXXXXXXX"};
        int row_enabled;

        row_enabled=1;
        for (i=0,xleft=x0;names[i][0]!='\0';i++)
            {
            char buf[32];
            WILLUSGUIFONT _font,*font;
            WILLUSGUIRECT te;

            font=&_font;
            willusgui_font_init(font);
            font->size=fsize;
            willusgui_font_get(font);
            willusgui_window_text_extents(&k2gui->mainwin,font,position[i],&te);
            if (k<0)
                {
                willusgui_window_text_render(&k2gui->mainwin,font,labels[i],xleft,ytop,
                                             0x000000,-1,0,NULL);
                }
            else
                {
                control=&k2gui->control[k2gui->ncontrols];
                control->index=100+k2gui->ncontrols;
                k2gui->ncontrols++;
                if (!already_drawn)
                    {
                    willusgui_control_init(control);
                    sprintf(control->name,names[i],k+1);
                    if (i==6)
                        strcpy(control->label,labels[i]);
                    control->flags=0;
                    control->color=(i<6 ? 0xffb060 : 0x70a0ff);
                    control->font.size=k2gui->font.size*fm;
                    control->font = (*font);
                    control->rect.left=xleft;
                    if (i==0 || i==6)
                        control->rect.left += te.right*.13;
                    control->rect.top=ytop;
                    control->rect.right=xleft+te.right-boxgaph;
                    control->rect.bottom=control->rect.top+fsize+3;
                    control->type=(i==0 || i==6) ? WILLUSGUICONTROL_TYPE_CHECKBOX 
                                       : (i==7 ? WILLUSGUICONTROL_TYPE_BUTTON
                                               : WILLUSGUICONTROL_TYPE_EDITBOX);
                    control->parent=&k2gui->mainwin;
                    if (i==7)
                        strcpy(control->label,"Select");
                    else
                        control->label[0]='\0';
                    control->labeljust=0;
                    control->labelx=xleft;
                    control->labely=ytop + h*.15;
                    willusgui_control_create(control);
                    }
                else
                    willusgui_control_redraw(control,0);
                k2gui_cropbox_eval(buf,k,i);
                if (i==0)
                    row_enabled = (buf[0]!='\0');
                if (i>0 && i<6)
                    willusgui_control_set_text(control,buf);
                else if (i==0 || i==6)
                    {
                    if (buf[0]!='\0')
                        control->attrib |= WILLUSGUICONTROL_ATTRIB_CHECKED;
                    else
                        control->attrib &= (~WILLUSGUICONTROL_ATTRIB_CHECKED);
                    }
                willusgui_control_enable(control,i==0?1:row_enabled);
                }
            xleft += te.right;
            }
        if (k<0)
            xb[2]=xleft;
        } /* k loop */
    yb[2]=ytop;
    /* Draw little box around crop area controls */
    willusgui_window_draw_line(&k2gui->mainwin,xb[0]-bmar,yb[0]-bmar+5,xb[1]+bmar-1,yb[0]-bmar+5,0,0);
    willusgui_window_draw_line(&k2gui->mainwin,xb[1]+bmar-1,yb[0]-bmar+5,xb[1]+bmar-1,yb[1]-bmar+1,0,0);
    willusgui_window_draw_line(&k2gui->mainwin,xb[1]+bmar-1,yb[1]-bmar+1,xb[2]+bmar-3,yb[1]-bmar+1,0,0);
    willusgui_window_draw_line(&k2gui->mainwin,xb[2]+bmar-3,yb[1]-bmar+1,xb[2]+bmar-3,yb[2]+bmar-2,0,0);
    willusgui_window_draw_line(&k2gui->mainwin,xb[0]-bmar,yb[2]+bmar-2,xb[2]+bmar-3,yb[2]+bmar-2,0,0);
    willusgui_window_draw_line(&k2gui->mainwin,xb[0]-bmar,yb[0]-bmar+5,xb[0]-bmar,yb[2]+bmar-2,0,0);
    y1=ytop+eheight*.3;
    }
    }

    /* Page range edit box */
    {
    static char *label[]={"Pages to convert:"};
    static char *name[]={"pages"};
    WILLUSGUIRECT r1;
    int fsize;
    
    i=0;
    x1=x0;
    fsize=k2gui->font.size*.9+.5;
    if (fsize<8)
        fsize=8;
    /* Incorporate as label, v2.32 */
    /*
    willusgui_window_text_render(&k2gui->mainwin,&k2gui->font,label[i],x1,y1+eheight*.1,0,-1,0,NULL);
    willusgui_window_text_extents(&k2gui->mainwin,&k2gui->font,label[i],&r1);
    */
    control=&k2gui->control[k2gui->ncontrols];
    control->index=100+k2gui->ncontrols;
    k2gui->ncontrols++;
    if (!already_drawn)
        {
        willusgui_control_init(control);
        control->type=WILLUSGUICONTROL_TYPE_EDITBOX;
        control->font.size=fsize;
        willusgui_font_get(&control->font);
        strcpy(control->name,name[i]);
        control->parent=&k2gui->mainwin;
        strcpy(control->label,label[0]);
        control->labelx=x1;
        control->labely=y1+eheight*.1;
        control->labeljust=0;
        willusgui_control_draw_label(control,&r1);
        r1.right = (r1.right-r1.left+1);
        control->rect.left= x0+r1.right+f4;
        control->rect.top = y1;
        control->rect.right = control->rect.left + eheight*8;
        control->rect.bottom = control->rect.top + fsize + 3;
        }
    if (already_drawn)
        willusgui_control_redraw(control,0);
    else
        willusgui_control_create(control);
    willusgui_control_set_text(control,k2settings->pagelist[0]=='\0'
                             ? "(all)" : k2settings->pagelist);
    y1 = control->rect.bottom + ((int)(linesize*0.1)<2 ? 2 : (int)(linesize*0.1));
/*
    {
    static char *blabel="Margin Select";
    WILLUSGUIRECT trect;
    double xl;

    xl = 0.9;
    willusgui_window_text_extents(&k2gui->mainwin,&k2gui->font,blabel,&trect);
    w = trect.right*1.1;
    h = k2gui->font.size*(xl+.4);
    control=&k2gui->control[k2gui->ncontrols];
    control->index=100+k2gui->ncontrols;
    k2gui->ncontrols++;
    if (!already_drawn)
        {
        willusgui_control_init(control);
        strcpy(control->name,blabel);
        control->color=0x70a0ff;
        control->font.size = k2gui->font.size*xl;
        willusgui_font_get(&control->font);
        control->rect.left = k2gui->control[k2gui->ncontrols-2].rect.right+h/2;
        control->rect.right = control->rect.left + w;
        control->rect.top = k2gui->control[k2gui->ncontrols-2].rect.top;
        control->rect.bottom = k2gui->control[k2gui->ncontrols-2].rect.bottom;
        control->type=WILLUSGUICONTROL_TYPE_BUTTON;
        control->parent=&k2gui->mainwin;
        willusgui_control_create(control);
        }
    else
        willusgui_control_redraw(control,0);
    }
*/

    }

    /* Checkboxes */
    {
    static char *checkboxlabel[] = {"Auto&straighten","&Break after each source page",
                               "Color o&utput","Output in &landscape",
                               "&Native PDF output","Right-&to-left text",
                               "Smart line brea&ks",
#ifdef HAVE_GHOSTSCRIPT
                               "&Post-process w/Ghostscript",
#endif
                               "Generate &marked-up source","Re-flow te&xt",
                               "&Erase vertical lines","Erase hori&zontal lines","Fast Previe&w",
                               "Avoi&d Text Select Overlap","I&gnore small defects",
                               "Auto-crop","Fixed output font size",
#ifdef HAVE_OCR_LIB
                               "&OCR (Tesseract)",
#endif
                               ""};
    static char *checkboxname[] = {"straighten","break","color","landscape","native",
                                   "r2l","linebreak",
#ifdef HAVE_GHOSTSCRIPT
                                   "ppgs",
#endif
                                   "markup","wrap","evl","ehl","autorot","bpm","defects",
                                   "autocrop","opfontsize"
#ifdef HAVE_OCR_LIB
                                   ,"ocr"
#endif
                                   };
    int n,n2,vspacing,fsize;
    double fm;

    fm=0.8;
    fsize = k2gui->font.size*fm+.5;
    if (fsize<10)
        fsize = 10;
    for (n=0;checkboxlabel[n][0]!='\0';n++);
    y1 += eheight*0.2;
    n2=(n+1)/2;
    x0=crect->left+xmar;
    w= (k2gui->control[0].rect.right - x0 - pmbw)/2;
    vspacing = fsize+1;
    ybmax = 0;
    for (i=0;i<n;i++)
        {
        int c,r,checked,textbox,ii;

        if (!stricmp(checkboxname[i],"linebreak"))
            textbox=1;
        else if (!stricmp(checkboxname[i],"landscape"))
            textbox=2;
        else if (!stricmp(checkboxname[i],"opfontsize"))
            textbox=3;
        else if (!stricmp(checkboxname[i],"ocr"))
            textbox=4;
        else
            textbox=0;
        c=i/n2;
        r=i%n2;
        control=&k2gui->control[k2gui->ncontrols];
        control->index=100+k2gui->ncontrols;
        k2gui->ncontrols++;
        if (!already_drawn)
            {
            willusgui_control_init(control);
            control->rect.left=x0 + c*w;
            control->rect.top=y1+vspacing*r;
            control->rect.right=control->rect.left+w-10;
            control->rect.bottom=control->rect.top+fsize+3;
            if (control->rect.bottom > ybmax)
                ybmax = control->rect.bottom;
            control->font.size=fsize;
            willusgui_font_get(&control->font);
            if (textbox)
                {
                WILLUSGUIRECT r1;
                willusgui_window_text_extents(&k2gui->mainwin,&control->font,checkboxlabel[i],&r1);
                if (textbox==3)
                    control->rect.right=control->rect.left+(r1.right-r1.left)+eheight*1.0;
                else
                    control->rect.right=control->rect.left+(r1.right-r1.left)+eheight*0.85;
                }
            control->type=WILLUSGUICONTROL_TYPE_CHECKBOX;
            control->color=0xd0ffd0;
            strcpy(control->name,checkboxname[i]);
            strcpy(control->label,checkboxlabel[i]);
            control->labelx=control->rect.left + vspacing;
            control->labely=control->rect.top;
            control->labeljust = 0;
            control->parent=&k2gui->mainwin;
            }
        /* Set checkmarks */
        checked=0;
#ifdef HAVE_GHOSTSCRIPT
        ii = i;
#else
        ii = (i>=7 ? i+1 : i);
#endif
        switch (ii)
            {
            case 0:
                checked=k2settings->src_autostraighten>=0.;
                break;
            case 1:
                checked=(k2settings->dst_break_pages==2);
                break;
            case 2:
                checked=k2settings->dst_color;
                break;
            case 3:
                checked=k2settings->dst_landscape;
                break;
            case 4:
                checked=k2settings->use_crop_boxes;
                break;
            case 5:
                checked=!k2settings->src_left_to_right;
                break;
            case 6:
                checked=(k2settings->word_spacing<0);
                break;
#ifdef HAVE_GHOSTSCRIPT
            case 7:
                checked=(k2settings->ppgs==1);
                break;
#endif
            case 8:
                checked=k2settings->show_marked_source;
                break;
            case 9:
                checked=k2settings->text_wrap;
                break;
            case 10:
                checked=k2settings->erase_vertical_lines;
                break;
            case 11:
                checked=k2settings->erase_horizontal_lines;
                break;
            case 12:
                checked=fabs(k2settings->src_rot-SRCROT_AUTO)<.5;
                break;
            case 13:
                checked=(k2settings->dst_break_pages==4);
                break;
            case 14:
                checked=fabs(k2settings->defect_size_pts-1.5)<.001;
                break;
            case 15:
                checked=k2settings->autocrop;
                break;
            case 16:
                checked=fabs(k2settings->dst_fontsize_pts)>1e-8;
                break;
#ifdef HAVE_OCR_LIB
            case 17:
                checked=(k2settings->dst_ocr=='t');
                break;
#endif
            }
        if (checked)
            control->attrib |= WILLUSGUICONTROL_ATTRIB_CHECKED;
        else
            control->attrib &= ~WILLUSGUICONTROL_ATTRIB_CHECKED;
        if (!already_drawn)
            willusgui_control_create(control);
        else
            willusgui_control_redraw(control,0);
        if (textbox)
            {
            char xbuf[32];
            WILLUSGUICONTROL *ctrl1;

            ctrl1=control;
            control=&k2gui->control[k2gui->ncontrols];
            control->index=100+k2gui->ncontrols;
            k2gui->ncontrols++;
            if (!already_drawn)
                {
                willusgui_control_init(control);
                if (textbox==3)
                    control->rect.left= ctrl1->rect.right + eheight;
                else  if (textbox==4)
                    control->rect.left= ctrl1->rect.right + eheight/4;
                else 
                    control->rect.left= ctrl1->rect.right + eheight/2;
                control->rect.top = ctrl1->rect.top+2;
                control->rect.bottom = control->rect.top+fsize-1;
                if (textbox==1)
                    control->rect.right = control->rect.left + eheight*1.5;
                else if (textbox==2)
                    control->rect.right = control->rect.left + eheight*2.5;
                else if (textbox==3)
                    control->rect.right = control->rect.left + eheight*2.0;
                else
                    control->rect.right = control->rect.left + eheight*1.2;
                if (textbox==3)
                    control->font.size=fsize;
                else
                    control->font.size=fsize-4;
                if (textbox==3)
                    control->type=WILLUSGUICONTROL_TYPE_UPDOWN;
                else
                    control->type=WILLUSGUICONTROL_TYPE_EDITBOX;
                willusgui_font_get(&control->font);
                if (textbox==1)
                    strcpy(control->name,"linebreakval");
                else if (textbox==2)
                    strcpy(control->name,"landscapepages");
                else if (textbox==3)
                    strcpy(control->name,"opfontsizeval");
                else if (textbox==4)
                    strcpy(control->name,"nthreads");
                if (textbox==4)
                    {
                    strcpy(control->label,"CPUs");
                    control->labelx=control->rect.right+eheight*.1;
                    control->labely=control->rect.top+eheight*.1;
                    }
                else
                    {
                    control->label[0]='\0';
                    control->labelx=control->rect.left;
                    control->labely=control->rect.top;
                    }
                control->parent=&k2gui->mainwin;
                }
            if (already_drawn)
                willusgui_control_redraw(control,0);
            else
                willusgui_control_create(control);
            if (textbox==1)
                {
                sprintf(xbuf,"%.3f",fabs(k2settings->word_spacing));
                willusgui_control_set_text(control,xbuf);
                }
            else if (textbox==2)
                willusgui_control_set_text(control,k2settings->dst_landscape_pages);
            else if (textbox==3)
                {
                char buf[32];
                sprintf(buf,"%.1f",k2gui->opfontsize);
                willusgui_control_set_text(control,buf);
                }
            else
                {
                if (k2settings->nthreads<0)
                    sprintf(xbuf,"%d%%",-k2settings->nthreads);
                else if (k2settings->nthreads>0)
                    sprintf(xbuf,"%d",k2settings->nthreads);
                else
                    xbuf[0]='\0';
                willusgui_control_set_text(control,xbuf);
                }
            }
        }
    }

    /* Reset / Convert buttons */
    {
    int n;
    n=2;
    xmin -= pmbw;
    for (i=0;i<n;i++)
        {
        double xl;
        int dx;
        static char *buttonnames[]={"*&Convert All Files","Restore Defaults"};
        static int buttoncolors[2]={0xd0ffd0,0xffb0b0};

/* printf("crect->left=%d, xmin=%d\n",crect->left,xmin); */
        xl = 1.2;
        w = xl*fs*8.5;
        dx = xl*fs*1.5;
        x1 = crect->left + ((xmin-crect->left)-n*w-(n-1)*dx)/2 + i*(w+dx);
        h = fs*(xl+.4);
        /* y1 = crect->bottom - h - f4; */
        y1 = (crect->bottom - f4 + ybmax - h)/2;
        control=&k2gui->control[k2gui->ncontrols];
        if (!already_drawn)
            {
            willusgui_control_init(control);
            control->rect.left=x1;
            control->rect.top=y1;
            control->rect.right=control->rect.left+w-1;
            control->rect.bottom=control->rect.top+h-1;
            control->index=100+k2gui->ncontrols;
            control->font.size=k2gui->font.size*xl;
            willusgui_font_get(&control->font);
            control->type=WILLUSGUICONTROL_TYPE_BUTTON;
            control->color=buttoncolors[i];
            strcpy(control->label,buttonnames[i]);
            strcpy(control->name,buttonnames[i]);
            control->parent=&k2gui->mainwin;
            }
        if (already_drawn)
            willusgui_control_redraw(control,0);
        else
            willusgui_control_create(control);
        k2gui->ncontrols++;
        }
    }
    strbuf_free(settings);
/*
if (!already_drawn)
aprintf(ANSI_GREEN "\nCHILDREN ADDED.  ncontrols=%d.\n" ANSI_NORMAL,k2gui->ncontrols);
else
aprintf(ANSI_YELLOW "\nCHILDREN REDRAWN.  ncontrols=%d.\n" ANSI_NORMAL,k2gui->ncontrols);
*/
    /* Windows 7 Weirdness...have to re-draw file list */
    willusgui_window_draw_rect_filled(&k2gui->mainwin,&flcontrol->rect,0xffffff);
    willusgui_control_redraw(flcontrol,0);
    willusgui_window_draw_rect_outline(&k2gui->mainwin,&flcontrol->rect,0);

    /* If focus was on an edit control, select all of the text. */
    if (focus_control!=NULL)
        {
        willusgui_window_set_focus(focus_control);
        if (focus_control->type == WILLUSGUICONTROL_TYPE_EDITBOX
             || focus_control->type == WILLUSGUICONTROL_TYPE_UPDOWN
             || focus_control->type == WILLUSGUICONTROL_TYPE_UPDOWN2)
            willusgui_control_text_select_all(focus_control);
        }
/*
printf("Done drawing controls.  ncontrols=%d\n",k2gui->ncontrols);
{
int i;
for (i=0;i<k2gui->ncontrols;i++)
printf("index[%03d]= %-20s %03d\n",i,k2gui->control[i].name,k2gui->control[i].index);
}
*/
    }


/*
** Determine crop margin setting
*/
static void k2gui_cropbox_eval(char *buf,int cmindex,int fieldindex)

    {
    K2CROPBOX *box;
    K2PDFOPT_SETTINGS *k2settings;

    k2settings=&k2gui->k2conv->k2settings;
    buf[0]='\0';
    if (cmindex<0 || cmindex>=MAXK2CROPBOXES)
        return;
    box = &k2settings->cropboxes.cropbox[cmindex];
    if (fieldindex==0)
        {
        if (!(box->cboxflags&K2CROPBOX_FLAGS_NOTUSED))
            strcpy(buf,"x");
        return;
        }
    if (fieldindex==6)
        {
        if (box->cboxflags&K2CROPBOX_FLAGS_IGNOREBOXEDAREA)
            strcpy(buf,"x");
        return;
        }
    if (fieldindex==5)
        {
        strcpy(buf,box->pagelist);
        return;
        }
    if (fieldindex>=1 && fieldindex<=4)
        {
        char fmtstr[16];

        if (box->units[fieldindex-1]==UNITS_INCHES || box->units[fieldindex-1]==UNITS_CM)
            strcpy(fmtstr,"%.2f%s");
        else
            strcpy(fmtstr,"%.3f%s");
        sprintf(buf,fmtstr,box->box[fieldindex-1],k2pdfopt_settings_unit_string(box->units[fieldindex-1]));
        return;
        }
    }


void k2gui_main_repaint(int changing)

    {
    static int width=0;
    static int height=0;
    static int children_created=0;
    int i;
    int newsize;
    char buf[256];
    WILLUSGUIRECT rect;

/*
int status;
sprintf(buf,"@k2gui_main_repaint, changing=%d, needs_redraw=%d",changing,needs_redraw);
status=willusgui_message_box(&k2gui->mainwin,"Debug",buf,
"*&OK","","",NULL,0,24,600,0xe0e0e0,NULL,NULL,1);
*/
#if (WILLUSDEBUGX & 0x2000)
printf("@k2gui_main_repaint(changing=%d, needs_redraw=%d)\n",changing,needs_redraw);
#endif
    willusgui_window_get_useable_rect(&k2gui->mainwin,&rect);
    /* Has the window changed size since last being drawn? */
    newsize = ((width != rect.right-rect.left+1)
                   || (height != rect.bottom-rect.top+1));
    width = rect.right-rect.left+1;
    height = rect.bottom-rect.top+1;
    if (newsize || force_repaint)
        {
        /* If child controls exist, delete them so we can draw new re-sized ones */
        if (children_created)
            {
            /* k2gui_destroy_children(); */
            children_created=0;
            }
        needs_redraw=2;
        }
    if (force_repaint)
        force_repaint=0;

    /* Set window title (in title bar) */
    k2sys_header(buf);
    for (i=0;buf[i]!='\r';i++);
    buf[i]=',';
    buf[i+1]=' ';
    for (;buf[i]!='\r';i++);
    buf[i]='\0';
    willusgui_control_set_text(&k2gui->mainwin,buf);

    /* Paint background */
    k2gui_background_bitmap_fill();
    if (k2gui->bgbmp.width>0)
        willusgui_background_bitmap_blit(&k2gui->mainwin,&k2gui->bgbmp);
    else
        willusgui_window_draw_rect_filled(&k2gui->mainwin,&rect,0xac5711);

    /* Draw buttons/controls */
    if (needs_redraw || !changing)
        {
        k2gui->font.size = k2gui_determine_fontsize();
        willusgui_font_get(&k2gui->font);
        /* v2.32--remember selected text edit */
        k2gui_get_selection();
/*
if (k2gui->sel_index>=0)
printf("get_selection: k2gui->sel_index=%d (%d-%d)\n",k2gui->sel_index,k2gui->sel_start,k2gui->sel_end);
*/
        k2gui_add_children(children_created);
        /* v2.32--restore selected text edit */
        k2gui_apply_selection();
        children_created=1;
        needs_redraw=0;
        /* k2gui_window_size_changed(&k2gui->mainwin); */
        }
    else
        if (!needs_redraw)
            needs_redraw=1;
    }


static void filebox_populate(void)

    {
    int i;
    WILLUSGUICONTROL *control;

    control=k2gui_control_by_name("file list");
    if (control==NULL)
        return;
    willusgui_control_listbox_clear(control);
    for (i=0;i<k2gui->k2conv->k2files.n;i++)
        willusgui_control_listbox_add_item(control,k2gui->k2conv->k2files.file[i]);
    }


static void k2gui_destroy_mainwin(void)

    {
    k2gui_destroy_children();
    willusgui_font_release(&k2gui->font);
    willusgui_control_close(&k2gui->mainwin);
    }


/* If any of the text controls have selected text, remember it. */
static void k2gui_get_selection(void)

    {
    int i;
    void *fhandle;

    fhandle=willusgui_control_handle_with_focus();
    k2gui->sel_index=-1;
    for (i=0;i<k2gui->ncontrols;i++)
        {
        int seltext,start,end;

        if (k2gui->control[i].handle!=fhandle)
            continue;
        /* Preserve selected text position */
        seltext=willusgui_control_text_selected(&k2gui->control[i],&start,&end);
        if (seltext)
            {
/*
printf("Getting selection from index=%d, handle=0x%X\n",k2gui->control[i].index,k2gui->control[i].handle);
printf("        seltext=%d, start=%d, end=%d\n",seltext,start,end);
printf("        handle w/focus = 0x%X\n",willusgui_control_handle_with_focus());
*/
            k2gui->sel_index=k2gui->control[i].index;
            k2gui->sel_start=start;
            k2gui->sel_end=end;
            }
        }
    }


/* Re-apply the remembered text selection */
static void k2gui_apply_selection(void)

    {
    int i;

    if (k2gui->sel_index<0)
        return;
    for (i=0;i<k2gui->ncontrols;i++)
        if (k2gui->control[i].index==k2gui->sel_index)
            {
/*
printf("Applying selection, index=%d, %d-%d\n",k2gui->sel_index,k2gui->sel_start,k2gui->sel_end);
printf("      hwnd=0x%X\n",k2gui->control[i].handle);
*/
            willusgui_control_text_select(&k2gui->control[i],k2gui->sel_start,k2gui->sel_end);
            break;
            }
    }


static void k2gui_destroy_children(void)

    {
    int i,nclosed;
/*
int norg;
norg=k2gui->ncontrols;
*/
    for (nclosed=0,i=k2gui->ncontrols-1;i>=0;i--)
        {
        int status;

        status=willusgui_control_close(&k2gui->control[i]);
/*
printf("    status[%d]=%d\n",i,status);
if (status==0)
printf("Error closing:  %s\n",win_lasterror());
*/
        if (status==1)
            nclosed++;
        }
    k2gui->ncontrols=0;
/*
aprintf(ANSI_RED "\nSUBWINDOWS DESTROYED (%d of %d actually closed).\n" ANSI_NORMAL,nclosed,norg);
*/
    }


char *k2gui_short_name(char *filename)

    {
    return(filename);
    }


static WILLUSGUICONTROL *k2gui_control_by_name(char *name)

    {
    int i;

    for (i=0;i<k2gui->ncontrols;i++)
        if (!stricmp(k2gui->control[i].name,name))
            return(&k2gui->control[i]);
    return(NULL);
    }


int k2gui_previewing(void)

    {
    return(k2gui->preview_processing);
    }


static void k2gui_preview_start(void)

    {
#if (WILLUSDEBUGX & 1)
printf("@k2gui_preview_start...\n");
#endif
    if (!k2gui_preview_done())
        {
        k2gui_preview_terminate();
        k2gui->pbitmap.width=0;
        k2gui->preview_processing=0;
        willusgui_set_cursor(0);
        k2gui_preview_refresh();
        return;
        }
    k2gui->preview_processing=1;
    willusgui_set_cursor(1);
    k2gui_preview_refresh();
    if (k2gui->k2conv->k2files.n<1)
        {
        k2gui_preview_fail(1);
        return;
        }
    k2gui->prevthread[0]=willusgui_semaphore_create_ex("k2pdfopt_preview",1,1);
    if (k2gui->prevthread[0]==NULL)
        {
        k2gui_preview_fail(2);
        return;
        }
    willusgui_semaphore_status_wait(k2gui->prevthread[0]);
    k2gui->prevthread[1]=willusgui_thread_create(k2gui_preview_make_bitmap,
                                                   (void *)k2gui->prevthread);
    if (k2gui->prevthread[1]==NULL)
        {
        willusgui_semaphore_release(k2gui->prevthread[0]);
        willusgui_semaphore_close(k2gui->prevthread[0]);
        k2gui->prevthread[0]=NULL;
        k2gui_preview_fail(3);
        }
/*
** This small delay seems to prevent a weird bug where the preview
** occasionally fails, resulting in a fail with status=4, or a complete
** crash with some errors reported by the PNG library.  I can't seem
** to get the crash to occur with this delay.  -- 4 Sep 2013, v2.01
*/
#ifdef HAVE_WIN32_API
win_sleep(100);
#endif
#if (WILLUSDEBUGX & 1)
printf("Exiting k2gui_preview_start.\n");
#endif
    }


static void k2gui_preview_make_bitmap(char *data)

    {
    int n,index;
    WILLUSGUICONTROL *filelistbox,*ppage,*preview;
    K2PDFOPT_CONVERSION *k2conv;
    K2PDFOPT_FILELIST_PROCESS k2listproc;
    STRBUF *cmdline,_cmdline;
    char *buf;
    static char *funcname="k2gui_preview_make_bitmap";

#if (WILLUSDEBUGX & 1)
printf("@k2gui_preview_make_bitmap...\n");
#endif
    if (k2gui->k2conv->k2files.n<1)
        {
        k2gui_preview_cleanup(1);
        return;
        }
    filelistbox=k2gui_control_by_name("file list");
    preview=k2gui_control_by_name("previewwin");
    ppage=k2gui_control_by_name("previewpage");
    if (ppage==NULL || filelistbox==NULL || preview==NULL)
        {
        k2gui_preview_cleanup(4);
        return;
        }
    n=willusgui_control_listbox_get_selected_items_count(filelistbox,&index,1);
    if (n<=0)
        index=0;
    willus_mem_alloc_warn((void **)&k2conv,sizeof(K2PDFOPT_CONVERSION),funcname,10);
    willus_mem_alloc_warn((void **)&buf,1024,funcname,10);
    k2pdfopt_conversion_init(k2conv);
    /* Add env. var and cmd-line to k2settings */
    cmdline=&_cmdline;
    strbuf_init(cmdline);
    k2gui_settings_to_cmdline(cmdline,&k2gui->k2conv->k2settings);
    parse_cmd_args(k2conv,k2gui->env,cmdline,&k2gui->cmdxtra,1,1);
    /* v2.35--remove duplicate line */
    /* parse_cmd_args(k2conv,k2gui->env,cmdline,&k2gui->cmdxtra,1,1); */

    /* Clear the files and only process the first file */
    willusgui_control_listbox_get_item_text(filelistbox,index,buf);
    k2pdfopt_files_clear(&k2conv->k2files);
    k2pdfopt_files_add_file(&k2conv->k2files,buf);

    /* Set the preview page */
    willusgui_control_get_text(ppage,buf,10);
    k2conv->k2settings.preview_page = atoi(buf);

    /* Turn off OCR (no point in having it on for a preview) */
#ifdef HAVE_OCR_LIB
    k2conv->k2settings.dst_ocr=0;
#endif

    k2listproc.bmp = &k2gui->pbitmap;
    k2listproc.filecount = 0;
    k2listproc.outname=NULL;
    k2listproc.bmp->width=-1;

    /* Convert it, baby! */
    overwrite_set(1);
    k2listproc.mode = K2PDFOPT_FILELIST_PROCESS_MODE_CONVERT_FILES;
    k2pdfopt_proc_wildarg(&k2conv->k2settings,k2conv->k2files.file[0],&k2listproc);
    willus_mem_free((double **)&k2listproc.outname,funcname);

    /* Faux conversion */
    /*
    sprintf(buf,"preview_bitmap_%03d.png",k2conv->k2settings.preview_page);
printf("Preview bitmap file name = '%s'\n",buf);
    bmp_read(&k2conv->preview_bitmap,buf,NULL);
    bmp_copy(&preview->bmp,&k2conv->preview_bitmap);
    */
    
    /*
    ** Clean up
    */
    k2pdfopt_conversion_close(k2conv);
    willus_mem_free((double **)&buf,funcname);
    willus_mem_free((double **)&k2conv,funcname);
    k2gui_preview_cleanup(k2gui->pbitmap.width<0 ? 5 : (k2gui->pbitmap.width==0 ? 4 : 0));
#if (WILLUSDEBUGX & 1)
printf("Exiting k2gui_preview_make_bitmap.\n");
#endif
    }


static void k2gui_preview_cleanup(int statuscode)

    {
    if (statuscode>0)
        k2gui_preview_fail(statuscode);
    else
        {
        k2gui->preview_processing=0;
        willusgui_set_cursor(0);
        k2gui_preview_refresh();
        }
    if (k2gui->prevthread[0]!=NULL)
        {
        willusgui_semaphore_release(k2gui->prevthread[0]);
        willusgui_semaphore_close(k2gui->prevthread[0]);
        k2gui->prevthread[0]=NULL;
        }
    }


static int k2gui_preview_done(void)

    {
    if (k2gui==NULL || k2gui->prevthread[0]==NULL)
        return(1);
    return(willusgui_semaphore_status(k2gui->prevthread[0]));
    }


static void k2gui_preview_terminate(void)

    {
    if (k2gui==NULL || k2gui->prevthread[1]==NULL)
        return;
    willusgui_thread_terminate(k2gui->prevthread[1],0);
    k2gui->prevthread[1]=NULL;
    if (k2gui->prevthread[0]!=NULL)
        {
        willusgui_semaphore_release(k2gui->prevthread[0]);
        willusgui_semaphore_close(k2gui->prevthread[0]);
        k2gui->prevthread[0]=NULL;
        }
    }


static void k2gui_preview_fail(int statuscode)

    {
    char buf[256];
    WILLUSGUICONTROL *control;
    static char *err[] = {"Unknown error","No files to convert","Cannot create semaphore",
                          "Cannot create thread","Beyond page count of converted file",
                          "Conversion unexpectedly terminated" };

    control=k2gui_control_by_name("previewwin");
    if (control==NULL)
        return;
    if (statuscode<0 || statuscode>5)
        statuscode=0;
    sprintf(buf,"Preview failed.\n\n%s.",err[statuscode]);
    k2gui_preview_bitmap_message(&k2gui->pbitmap,700,950,0.07,buf);
    k2gui->preview_processing=0;
    willusgui_set_cursor(0);
    k2gui_preview_refresh();
    }


static void k2gui_preview_bitmap_message(WILLUSBITMAP *bmp,int width,int height,
                                         double fs,char *message)

    {
    double mpw=fs*height*.5;
    int i,nl,mh,y1;
    char buf[256];

    bmp->width=width;
    bmp->height=height;
    bmp->bpp=24;
    bmp_alloc(bmp);
    bmp_fill(bmp,192,192,192);
    for (i=0,nl=0;1;nl++)
        {
        next_bmp_line(buf,message,&i,mpw,width);
        if (message[i]=='\0')
            {
            if (buf[0]!='\0')
                nl++;
            break;
            }
        }
    mh=fs*height*nl;
    fontrender_set_typeface("helvetica-bold");
    fontrender_set_fgcolor(100,0,100);
    fontrender_set_bgcolor(192,192,192);
    fontrender_set_pixel_size(fs*height);
    fontrender_set_justification(5);
    fontrender_set_or(1);
    for (y1=height-(height-mh)/2,i=0;1;y1-=fs*height)
        {
        next_bmp_line(buf,message,&i,mpw,width);
        if (buf[0]!='\0')
            fontrender_render(bmp,bmp->width/2,y1,buf,0,NULL);
        if (message[i]=='\0')
            break;
        }
    }


static void next_bmp_line(char *d,char *s,int *index,double mpw,int maxpix)

    {
    int i,j,i0,j0;

    i=(*index);
    for (;s[i]==' ' || s[i]=='\t';i++);
    for (j=j0=0,i0=i;s[i]!='\n' && s[i]!='\0';i++)
        {
        if (s[i]==' ' || s[i]=='\t')
            {
            j0=j;
            i0=i+1;
            d[j++]=s[i];
            if ((j-1)*mpw >= maxpix)
                {
                d[j0]='\0';
                (*index)=i+1;
                return;
                }
            continue;
            }
        if (j*mpw>=maxpix)
            {
            if (j0==0)
                {
                d[j]='\0';
                (*index)=i;
                return;
                }
            d[j0]='\0';
            (*index)=i0;
            return;
            }
        d[j++]=s[i];
        }
    d[j]='\0';
    (*index)=(s[i]=='\0') ? i : i+1;
    }


void k2gui_preview_refresh(void)

    {
    WILLUSGUICONTROL *control;

    control=k2gui_control_by_name("previewwin");
    if (control==NULL)
        return;
    /*
    ** In MS Windows, the close command below will automatically initiate
    ** a WM_PAINT message to the main window, which will re-draw all the
    ** child windows, which will re-create this window since it will have
    ** a NULL handle.
    */
    willusgui_control_close(control);
    }


static void k2gui_contextmenu_by_control(WILLUSGUICONTROL *control)

    {
    static char *contextmenu_help[] =
        {
        "evl",
            "Erase vertical lines",
            "The \"Erase vertical lines\" checkbox, when checked, causes "
            "k2pdfopt to erase vertical lines that may be preventing "
            "your text from getting re-flowed.",
        "ehl",
            "Erase horizontal lines",
            "The \"Erase horizontal lines\" checkbox, when checked, causes "
            "k2pdfopt to erase horizontal lines that may be preventing "
            "your file from being processed the way you want.",
        "straighten",
            "Autostraighten",
            "The \"Autostraighten\" checkbox, when checked, causes k2pdfopt "
            "to analyze and straighten (de-skew) each source page.",
        "break",
            "Break after each source page",
            "The \"Break after each source page\" checkbox, when checked "
            "causes k2pdfopt to insert a page break into the output file after each "
            "source page has been processed.",
        "color",
            "Color output",
            "The \"Color output\" checkbox, when checked, causes k2pdfopt "
            "to generate the output PDF in color.",
        "landscape",
            "Output in landscape",
            "The \"Output in landscape\" checkbox, when checked, causes "
            "k2pdfopt to rotate the output PDF such that you will hold your "
            "e-reader sideways (in landscape mode) in order to read it.  "
            "This effectively gives you a wider screen.  The text box "
            "next to the checkbox allows you to specify a source page range "
            "to convert to landscape (e.g. 2,4,10-12).",
        "opfontsize",
            "Fixed output font size",
            "Magnify the output PDF so that the font size matches the specified value.  "
            "Use a negative value to individually set each page; otherwise the whole "
            "document is magnified by a uniform value based on the median font size.",
        "native",
            "Native PDF output",
            "The \"Native PDF Output\" checkbox, when checked, causes "
            "k2pdfopt to generate an output PDF in \"native\" mode--i.e. if the "
            "source PDF has scaleable graphics and fonts, the output "
            "PDF will also have scaleable graphics and fonts.  If native "
            "PDF output is unchecked, the output PDF will be made up of "
            "fixed resolution bitmaps (with an optional OCR text layer).  "
            "Note that on most e-readers, bitmapped PDF files often render "
            "faster than native PDF files.  Also, native PDF output is not "
            "allowed if text re-flow is checked.",
        "r2l",
            "Right-to-left text",
            "The \"Right-to-left text\" checkbox, when checked, causes k2dfopt "
            "to processed the source PDF file from right to left.  "
            "This is best for arabic and hebrew texts, for example.",
        "linebreak",
            "Smart line breaks",
            "The \"Smart Line Breaks\" checkbox, when checked, causes "
            "k2pdfopt to group letters into words using a heuristic "
            "algorithm.  The value in the box is an override in case "
            "the algorithm fails (or if the option is not checked).  "
            "It represents the fraction of the height of a lowercase "
            "'o'.  In manual mode (unchecked), if the gap between "
            "letters exceeds this distance, it is considered to be "
            "a word separator.",
    #ifdef HAVE_GHOSTSCRIPT
        "ppgs",
            "Post-process with Ghostscript",
            "The \"Post-process with Ghostscript\" checkbox, when checked, "
            "causes k2pdfopt to post-process the converted PDF file "
            "using Ghostscript's pdfwrite device.  This requires that "
            "you have Ghostscript installed on your PC.  This is a good "
            "way to solve text selection overlap problems if you are "
            "using native PDF output, particularly if processing a two-column "
            "source PDF.  See the -ppgs option in the command-line help "
            "for more information.",
    #endif
        "markup",
            "Generate marked-up source",
            "The \"Generate Marked-up Source\" checkbox, when checked, causes "
            "k2pdfopt to generate a separate \"markup\" output PDF file "
            "(with the ending _marked.pdf) that "
            "shows how k2pdfopt is processing the "
            "source PDF file.  If this option is checked, the preview "
            "window will also show the contents of this markup file.",
        "wrap",
            "Re-flow text",
            "The \"Re-flow text\" checkbox, when checked, causes k2pdfopt to "
            "try to re-flowed text to fit the output device screen.  "
            "Note that this option is not compatible with native PDF output.",
        "autorot",
            "Fast Preview",
            "The \"Fast Preview\" checkbox, when checked, prevents k2pdfopt "
            "from trying to automatically determine the "
            "source PDF orientation when generating previews since this "
            "takes extra processing time.",
        "bpm",
            "Avoid Text Select Overlap",
            "The \"Avoid Text Select Overlap\" checkbox, when checked, will "
            "cause k2pdfopt to force each cropped area from the source PDF "
            "onto a new "
            "output PDF page.  This is one way to avoid text selection "
            "overlap issues.  See also the post-processing with Ghostscript "
            "option.",
        "defects",
            "Ignore small defects",
            "The \"Ignore small defects\" checkbox, when checked, will cause "
            "k2pdfopt to ignore scanning / copying artifacts (speckles) smaller "
            "than 1.5 points "
            "in size will be ignored when processing the source PDF.",
        "autocrop",
            "Auto-crop",
            "The \"Auto-crop\" checkbox, when checked, will cause k2pdfopt "
            "to attempt to "
            "automatically crop out dark scanning artifacts from "
            "the edges of pages.  This is typically used on book scans or "
            "photocopied images.",
    #ifdef HAVE_OCR_LIB
        "ocr",
            "OCR (Tesseract)",
            "The \"OCR (Tesseract)\" checkbox, when checked, will cause "
            "k2pdfopt to analyze the source PDF using optical character "
            "recognition (OCR) using the Tesseract library.  An invisible "
            "layer of text will be added to the output PDF so that the "
            "text can be selected and/or searched.  You must have a "
            "Tesseract language library installed.  See "
            "http://willus.com/k2pdfopt/help/ocr.shtml for more details.\n\n"
            "The \"CPUs\" box next to the OCR check box specifies how many "
            "CPU threads to use for OCR processing.  It can be a percentage "
            "of the available CPU threads or just a number of threads.  The "
            "default is 50%, which typically provides a 1.5 - 2.5x speed "
            "improvement.",
    #endif
        "_fitpage_",
            "Fit to Preview Window",
            "Fits the preview page to the window.",
        "_magplus_",
            "Increase Preview Magnification",
            "Increases the magnification in the preview window.",
        "_magminus_",
            "Decrease Preview Magnification",
            "Decreases the magnification in the preview window.",
        "Restore Defaults",
            "Restore Default Settings",
            "The \"Restore Defaults\" button returns all settings "
            "(except for the custom presets) to the default k2pdfopt values.",
        "*&Convert all files",
            "Convert All Files",
            "The \"Convert All Files\" button processes all of the "
            "files in the file list.",
        "File list",
            "File list",
            "The \"File list\" shows all source PDF files which "
            "will be processed.  The "
            "selected file will be the one shown in the preview window.",
        "Save",
            "Save Environment Variable",
            "The \"Save\" button saves the contents of the text field "
            "to the K2PDFOPT environment variable.",
        "Restore",
            "Restore Environment Variable",
            "The \"Restore\" button restores the contents of the "
            "K2PDFOPT environment variable into the text field.",
        "info",
            "Info",
            "Display information about the selected PDF file.  "
            "The shortcut key is CTRL-D.  The information is specific "
            "to the page range in the 'Pages to Convert' box.",
        "&add file",
            "Add File",
            "The \"Add File\" button adds a source PDF file to the file "
            "list to be converted.",
        "add folder",
            "Add Folder",
            "The \"Add Folder\" button allows you to add an entire folder "
            "to the file list.  All files in that "
            "folder will be converted.  If the folder contains a sequence "
            "of bitmaps, those bitmaps will be converted, in alphabetic order, "
            "as if they were pages of a PDF file.",
        "ignore*",
            "Ignore Cropped Area",
            "If the \"Ignore\" box is checked, the corresponding crop region "
            "is ignored / excluded from the output PDF conversion.",
        "cboxactive*",
            "Crop-box activation",
            "Activates / de-activates the crop-box settings in the given row.  "
            "Each crop-box can be used to select a specific area of the source document "
            "(or multiple specific areas if "
            "you specify more than one crop-box on a page) for processing (or for "
            "ignoring if you check the ignore box).  The crop-box can be "
            "graphically selected using the Select button or you can manually "
            "put in the left,top corner plus the width and height in various "
            "different units.",
        "cboxleft*",
            "Crop-box left side",
            "Sets the left side position of the crop-box."
            "Default units are inches.  You can put, for example, "
            "1.0cm or 0.5s for other types of units.",
        "cboxtop*",
            "Crop-box top side",
            "Sets the top side position of the crop-box."
            "Default units are inches.  You can put, for example, "
            "1.0cm or 0.5s for other types of units.",
        "cboxwidth*",
            "Crop-box width",
            "Sets the width of the crop-box."
            "Default units are inches.  You can put, for example, "
            "1.0cm or 0.5s for other types of units.",
        "cboxheight*",
            "Crop-box height",
            "Sets the height of the crop-box."
            "Default units are inches.  You can put, for example, "
            "1.0cm or 0.5s for other types of units.",
        "cboxignore*",
            "Crop-box ignore",
            "If checked, the region within the crop-box is ignored (internally "
            "it is painted white).  Otherwise, the regions outside the crop-box "
            "are ignored.",
        "cboxpages*",
            "Crop-box page range",
            "The page range for which the crop box applies.  For example, you "
            "can put 1,10,20e-40,61o- to specify pages 1, 10, 20 to 40 even, "
            "and 61 to the end of the document odd.",
        "cboxselect*",
            "Crop-box / Margin Select",
            "The \"Select\" buttons allow you to set the crop margins "
            "graphically using an overlay of all selected "
            "source pages (using the page range in the correponding "
            "\"Pages Range\" box).",
        "remove item",
            "Remove Item",
            "The \"Remove Item\" button removes the selected item "
            "from the file list.",
        "heightunits;widthunits;devwidth;devheight",
            "Device Height and Width and Units",
            "Select the output page width and height and dimensional units.",
        "crop*",
            "Crop Margins",
            "The \"Crop Margins\" controls set the amount of margin on each "
            "side of the "
            "source document which you wish to crop out and not show in the "
            "converted document.  You can use the \"Margin Select\" button "
            "to graphically set these numbers.  The units are inches.",
        "maxcols",
            "Max Columns",
            "The \"Max Columns\" value will control the maximum number of "
            "columns that k2pdfopt attempts to detect in the source document.  "
            "If you don't want to detect multiple columns, set this to 1.",
        "ddr",
            "Document Display Resolution",
            "The \"Document Resolution Factor\" value can be used to "
            "increase or decrease the resolution of "
            "the converted document.  For example, set this to 2 to double "
            "the default output document resolution.",
        "opfolder",
            "Output Folder",
            "The \"Output Folder\" text box shows the name of the folder "
            "where the converted files will be saved.  You can choose "
            "this folder by clicking the \"Select\" button next to it.",
        "opselect",
            "Output Folder Select",
            "The \"Select\" button next to the Output Folder text box "
            "selects the output folder where the converted files will be saved.",
        "opclear",
            "Output Folder Clear",
            "The \"Clear\" button next to the Output Folder text box "
            "clears the output folder.  With no output folder selected, the "
            "converted files are saved into the same folder as their associated "
            "source files.",
        "env",
            "Environment Variable",
            "The \"Environment Variable\" text box shows the value "
            "of the K2PDFOPT environment variable. "
            "You can change the value and then click the neighboring "
            "\"SAVE\" button to store it.",
        "extra",
            "Additional Options",
            "In the \"Additional Options\" text box, you can enter "
            "command-line arguments that will be appended to the GUI "
            "command-line arguments in order to allow access to k2pdfopt "
            "features that are not supported by the GUI.",
        "cmdline args",
            "Command-line version of options",
            "The \"Command-line version of options\" text box shows the actual "
            "arguments that would be used on the command line.  "
            "If you need to write a script that does the same type of "
            "conversion as the GUI, you can copy and paste these arguments "
            "to your script.",
        "mode",
            "Conversion Mode",
            "The \"Conversion Mode\" pull-down menu can be used to select "
            "a specific conversion mode.  Each mode selects multiple options "
            "in order to best "
            "process your source document in a number of custom ways. "
            "See the command-line usage (Help menu -> Command-line), under "
            "-mode, to see what each conversion mode is best suited for.  "
            "For instance, if you have a 2-column source PDF, you "
            "may want to selet \"2-column.\"  If you want to best preserve "
            "the source PDF formatting and don't need to re-flow the text, "
            "try the \"fit width\" mode.  The default mode tries to best "
            "detect what type of source document you have.",
        "devdpi",
            "DPI",
            "The \"DPI\" setting sets your e-reader screen DPI, or "
            "dots per inch.  Increasing "
            "this will increase the magnification of the converted "
            "document assuming that the deevice width and height are set "
            "in pixels.",
        "device",
            "Device",
            "The \"Device\" drop-down menu allows you to select your device "
            "type from a drop-down menu.  "
            "If your device is not listed, you can "
            "enter your e-reader dimensions and DPI into the boxes below "
            "this menu.",
        "previewpage;Pre&view",
            "Preview Window",
            "The \"Preview\" button shows a preview of your converted "
            "file.  Select the page you wish to preview and then press the "
            "\"Preview\" button to see it in the preview window.  Also, "
            "you can check the \"Generate marked-up source\" checkbox "
            "and then the preview window will show the marked up source "
            "page instead of the converted page.",
        "pages",
            "Pages to convert",
            "The \"Pages to convert\" text box allows you to enter a list "
            "of the pages which you wish to convert.  "
            "Some examples are:\n"
            "    1,3,10-27,30-   Converts pages 1, 3, 10 through 27, "
            "and 30 and higher.\n"
            "    6e-    Converts all even pages from 6 up.",
        "*",
            "Custom Preset",
            "The \"Custom Preset\" buttons store or recall a snapshot of all of the "
            "selected settings.  To store the settings, press and hold the "
            "button for two seconds (much like your FM radio presets work "
            "in your car).  To recall settings, just click on the desired button."
        };
    int i;
    static char *unithelp =
        "px = pixels\n"
        "in = inches\n"
        "cm = centimeters\n"
        "s = fraction of the source page (e.g. 1 = same as source page)\n"
        "t = fraction of the trimmed source page\n"
        "x = fraction of the text-layer width/height on the source page";

    for (i=0;contextmenu_help[i][0]!='\0';i+=3)
        {
        char *p;
        int j;
        for (j=0,p=&contextmenu_help[i][j];1;p=&p[j+1])
            {
            int match,len;
            char buf[128];

            strncpy(buf,p,127);
            buf[127]='\0';
            j=in_string(buf,";");
            if (j>0)
                buf[j]='\0';
            len=strlen(buf);
            if (!strcmp(buf,"*"))
                match=1;
            else if (buf[len-1]=='*')
                match=!strnicmp(control->name,buf,len-1);
            else
                match=!stricmp(control->name,buf);
            if (match)
                {
                if (!strcmp(&contextmenu_help[i+2][strlen(contextmenu_help[i+2])-7]," units."))
                    {
                    char *buf2;
                    static char *funcname="k2gui_contextmenu_by_control";

                    willus_mem_alloc_warn((void **)&buf2,strlen(contextmenu_help[i+2])+strlen(unithelp)+4,funcname,10);
                    sprintf(buf2,"%s\n%s",contextmenu_help[i+2],unithelp);
                    k2gui_contextmenu(contextmenu_help[i+1],buf2);
                    willus_mem_free((double **)&buf2,funcname);
                    }
                else
                    k2gui_contextmenu(contextmenu_help[i+1],contextmenu_help[i+2]);
                return;
                }
            if (j<0)
                break;
            }
        }
    }


static void k2gui_contextmenu(char *title,char *content)

    {
    static int bcolors[3]={0x60b060,0xe0ffe0,0xe0ffe0};

    willusgui_message_box(&k2gui->mainwin,title,content,"*&OK",NULL,NULL,
                           NULL,0,20,600,0xe0ffe0,bcolors,NULL,1);
    }
#endif /* HAVE_K2GUI */
