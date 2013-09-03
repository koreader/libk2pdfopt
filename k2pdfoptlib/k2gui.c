/*
** k2gui2.c      K2pdfopt Generic WILLUSGUI functions.
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
**
** VERSION HISTORY
**
**     See k2version.c.
**
*/

#include <k2pdfopt.h>

#ifdef HAVE_K2GUI

static char *unitname[]={"px","in","cm","s","t",""};
static K2GUI *k2gui,_k2gui;

char *k2gui_short_name(char *filename);
static int  k2gui_winposition_get(WILLUSGUIRECT *rect);
static void k2gui_winposition_save(void);
/*
static void k2gui_cmdline_to_settings(K2PDFOPT_SETTINGS *k2settings,STRBUF *cmdline);
*/
static void k2gui_init(void);
static void k2gui_background_bitmap_fill(void);
static void bmp_gradient_fill(WILLUSBITMAP *bmp);
/*
static int  k2gui_window_size_changed(WILLUSGUIWINDOW *window);
*/
static void k2gui_main_window_init(int normal_size);
static void k2gui_window_menus_init(WILLUSGUIWINDOW *win);
static int  k2gui_set_device_from_listbox(WILLUSGUICONTROL *control);
static WILLUSGUICONTROL *k2gui_control_with_focus(int *index);
static void k2gui_update_controls(void);
static void k2gui_error_out(char *message);
static void k2gui_add_files(void);
static int  k2gui_determine_fontsize(void);
static void k2gui_add_children(int already_drawn);
static double mar_eval(double x);
static void filebox_populate(void);
static void k2gui_destroy_mainwin(void);
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


int k2gui_main(K2PDFOPT_CONVERSION  *k2conv0,void *hInstance,void *hPrevInstance,
               STRBUF *env,STRBUF *cmdline)

    {
    int status;
    /* K2PDFOPT_CONVERSION *k2conv; */
    /* static char *funcname="k2gui_main"; */

#if (WILLUSDEBUG & 0x2000)
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
    parse_cmd_args(k2gui->k2conv,cmdline,NULL,NULL,1,1);
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
    status=k2gui_osdep_window_proc_messages(&k2gui->mainwin,NULL,NULL);
    k2gui->active=0;
    strbuf_free(&k2gui->cmdxtra);
    willusgui_close();
    return(status);
    }


/*
** Put up a dialog box with a message in it.
*/
int k2gui_alertbox(int retval,char *title,char *message)

    {
#if (defined(WIN32) || defined(WIN64))
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
        return(1);
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


void k2gui_get_settings(int index,K2PDFOPT_SETTINGS *k2settings,STRBUF *extra)

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
#if (WILLUSDEBUG & 0x2000)
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
#if (WILLUSDEBUG & 0x2000)
printf("   parsing '%s'\n",cmdline.s);
#endif
    parse_cmd_args(k2conv,&cmdline,NULL,NULL,1,1);
#if (WILLUSDEBUG & 0x2000)
printf("    k2conv->k2settings.gui=%d\n",k2conv->k2settings.gui);
#endif
    k2pdfopt_settings_copy(k2settings,&k2conv->k2settings);
#if (WILLUSDEBUG & 0x2000)
printf("    k2settings->gui=%d\n",k2settings->gui);
#endif
    strbuf_free(&cmdline);
    willus_mem_free((double **)&cmdbuf,funcname);
    k2pdfopt_conversion_close(k2conv);
    willus_mem_free((double **)&k2conv,funcname);
    }


void k2gui_save_settings(int index,K2PDFOPT_SETTINGS *k2settings,STRBUF *extra)

    {
    STRBUF *settings,_settings;
    STRBUF *envvar,_envvar;
    char customname[32];
    char envname[32];
    int status;

    k2gui_get_custom_name(index,customname,31);
    status=k2gui_get_text("Save Custom Settings",
                          "Enter name for custom settings:",
                          "*Save","Cancel",customname,31);
    if (status!=1)
        return;
    settings=&_settings;
    envvar=&_envvar;
    strbuf_init(settings);
    strbuf_init(envvar);
    k2gui_settings_to_cmdline(settings,k2settings);
    strbuf_sprintf(envvar,"%s;%s;%s",customname,
                       settings->s==NULL?"":settings->s,
                       extra->s==NULL?"":extra->s);
    sprintf(envname,"K2PDFOPT_CUSTOM%d",index);
#if (WILLUSDEBUG & 0x2000)
printf("Setenv %s=%s\n",envname,envvar->s);
#endif
    wsys_set_envvar(envname,envvar->s,0);
    }


void k2gui_settings_to_cmdline(STRBUF *cmdline,K2PDFOPT_SETTINGS *k2settings)

    {
    K2PDFOPT_SETTINGS _k2inited,*k2inited;
    k2inited=&_k2inited;
    k2pdfopt_settings_init(k2inited);
    k2pdfopt_settings_get_cmdline(cmdline,k2settings,k2inited);
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
    k2gui->pbitmap.width=k2gui->pbitmap.height=0;
    k2gui->pbitmap.bpp=24;
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

    if (!k2gui_winposition_get(&k2gui->mainwin.rect))
        {
        /* Get desktop rectangle */
        willusgui_window_get_rect(NULL,&dtrect);
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
    static char *menus[] = {"_File","&Add Source File...","E&xit",
                            "_Help","&Website help page...","&Command-line Options...",
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
                k2gui_save_settings(preset,&k2gui->k2conv->k2settings,&k2gui->cmdxtra);
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
#if (WILLUSDEBUG & 0x2000)
printf("Got WILLUSGUIACTION_SETFOCUS.\n");
#endif
        if (control!=NULL
              && (control->type == WILLUSGUICONTROL_TYPE_EDITBOX
                   || control->type == WILLUSGUICONTROL_TYPE_UPDOWN
                   || control->type == WILLUSGUICONTROL_TYPE_UPDOWN2))
{
#if (WILLUSDEBUG & 0x2000)
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
    if (control->type==WILLUSGUICONTROL_TYPE_UPDOWN && action==WILLUSGUIACTION_CONTROL_PRESS)
        action=WILLUSGUIACTION_UPDOWN_EDIT;
    switch (action)
        {
        case WILLUSGUIACTION_CREATE:
            k2gui_window_menus_init(&k2gui->mainwin);
            willusgui_window_timer_init(&k2gui->mainwin,200); /* Send timer event every 200 ms */
            willusgui_window_accept_draggable_files(&k2gui->mainwin);
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
                    dpi=k2settings->dst_dpi;
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
                        if (ii==UNITS_SOURCE || ii==UNITS_TRIMMED)
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
                            if ((*x)==UNITS_SOURCE || (*x)==UNITS_TRIMMED)
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
                            if ((*x)==UNITS_SOURCE || (*x)==UNITS_TRIMMED)
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
#if (WILLUSDEBUG & 0x2000)
printf("k2gui_process_message:  -mode %s\n",buf);
#endif
                    k2settings_sprintf(NULL,k2settings,"-mode def");
                    if (devcontrol!=NULL)
                        k2gui_set_device_from_listbox(devcontrol);
                    if (stricmp(buf,"default"))
                        k2settings_sprintf(NULL,k2settings,"-mode %s",buf);
#if (WILLUSDEBUG & 0x2000)
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
                    willusgui_control_gettext(control,buf,1023);
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
                    k2settings->dst_break_pages= checked ? 1 : 0;
                else if (!strcmp(control->name,"color"))
                    k2settings->dst_color= checked ? 1 : 0;
                else if (!strcmp(control->name,"landscape"))
                    k2settings->dst_landscape = checked ? 1 : 0;
                else if (!strcmp(control->name,"native"))
                    k2settings->use_crop_boxes = checked ? 1 : 0;
                else if (!strcmp(control->name,"r2l"))
                    k2settings->src_left_to_right = checked ? 0 : 1;
                else if (!strcmp(control->name,"markup"))
                    k2settings->show_marked_source = checked ? 1 : 0;
                else if (!strcmp(control->name,"wrap"))
                    k2settings->text_wrap = checked ? 2 : 0;
#ifdef HAVE_OCR_LIB
                else if (!strcmp(control->name,"ocr"))
                    k2settings->dst_ocr = checked ? 't' : 'm';
#endif
                else if (!strcmp(control->name,"evl"))
                    k2settings->erase_vertical_lines = checked ? 1 : 0;
                else if (!strcmp(control->name,"autorot"))
                    k2settings->src_rot = checked ? SRCROT_AUTO : SRCROT_AUTOPREV;
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
                else if (in_string(control->name,"add file")>=0)
                    k2gui_add_files();
                else if (in_string(control->name,"remove file")>=0)
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
                        if (k2gui_cbox_conversion_successful())
                            k2pdfopt_files_clear(&k2gui->k2conv->k2files);
                        filebox_populate();
                        flcontrol=k2gui_control_by_name("file list");
                        willusgui_control_close(flcontrol);
                        k2gui_update_controls();
                        }
                    }
                else if (!stricmp(control->name,"pre&view"))
                    {
                    if (k2gui->k2conv->k2files.n<=0)
                        k2gui_messagebox(0,"Convert","No files selected for conversion.");
                    k2gui_preview_start();
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
                xi=&k2settings->dst_dpi;
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
            else if (!strcmp(control->name,"drf"))
                {
                strcpy(fmt,"%.1f");
                del=0.1;
                xf=&k2settings->dst_display_resolution;
                min=0.1;
                max=10.;
                vtype=1;
                }
            else if (!strcmp(control->name,"cropleft"))
                {
                strcpy(fmt,"%.2f");
                del=0.05;
                xf=&k2settings->mar_left;
                min=0.;
                max=-1.;
                vtype=1;
                }
            else if (!strcmp(control->name,"croptop"))
                {
                strcpy(fmt,"%.2f");
                del=0.05;
                xf=&k2settings->mar_top;
                min=0.;
                max=-1.;
                vtype=1;
                }
            else if (!strcmp(control->name,"cropright"))
                {
                strcpy(fmt,"%.2f");
                del=0.05;
                xf=&k2settings->mar_right;
                min=0.;
                max=-1.;
                vtype=1;
                }
            else if (!strcmp(control->name,"cropbottom"))
                {
                strcpy(fmt,"%.2f");
                del=0.05;
                xf=&k2settings->mar_bot;
                min=0.;
                max=-1.;
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
                        if (!strncmp(control->name,"crop",4) && fnew<=1e-8)
                            fnew = -1.;
                        (*xf)=fnew;
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
                    {
                    k2gui_add_files();
                    break;
                    }
                case 701:
                    k2gui_quit();
                    break;
                case 710:
                    willusgui_start_browser("http://willus.com/k2pdfopt/help/");
                    break;
                case 711:  /* Command-line options */
                    {
                    char *buf;
                    int status;
                    WILLUSGUIRECT wrect,rect;

                    willusgui_window_get_rect(&k2gui->mainwin,&wrect);
                    willus_mem_alloc_warn((void **)&buf,k2usage_len(),funcname,10);
                    k2usage_to_string(buf);
                    rect=wrect;
                    rect.left += 40;
                    rect.right -= 40;
                    rect.top += 40;
                    rect.bottom -= 40;
                    status=willusgui_message_box(&k2gui->mainwin,
                           "K2pdfopt Command-line Options",
                           "K2pdfopt Command-line Options",
                           "*&DISMISS","&GO TO WEBSITE",NULL,
                           buf,strlen(buf),
                           (wrect.right-wrect.left)*.029,
                           (wrect.right-wrect.left),
                           0xffb080,NULL,&rect,1);
                    if (status==2)
                        willusgui_start_browser("http://willus.com/k2pdfopt/");
                    willus_mem_free((double **)&buf,funcname);
                    break;
                    }
                case 712:  /* About box */
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
            WILLUSGUIRECT rect;
            int ww,new_width,new_height;

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
            ptr=willusgui_get_dropped_files(message->ptr[0]);
            if (ptr!=NULL)
                {
                int i;
                for (i=0;ptr[i]!=NULL;i++)
                    k2gui_add_file(ptr[i]);
                willusgui_release_dropped_files(ptr);
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
            k2gui_winposition_save();
            k2gui_destroy_mainwin();
            break;
        case WILLUSGUIACTION_DESTROY:
            k2gui_quit();
            break;
        }
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
    if (needs_redraw!=2)
        needs_redraw=1;
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


static void k2gui_add_files(void)

    {
    static char *funcname="k2gui_add_files";
    char *filename;
    static char *allowed_files="PDF files\0*.pdf\0DJVU files\0*.djvu\0"
                               "All files\0*\0\0\0";
    int size,status;

/*
printf("Calling wincomdlg...\n");
*/
    size=16384;
    willus_mem_alloc_warn((void **)&filename,size,funcname,10);
    status=willusgui_file_select_dialog(filename,size-1,allowed_files,"Select source file","pdf");
/* printf("status=%d\n",status); */
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


static int k2gui_determine_fontsize(void)

    {
    WILLUSGUIRECT rect;
    int fontsize;

    willusgui_window_get_rect(&k2gui->mainwin,&rect);
    fontsize=(rect.right-rect.left)/47;
    if (fontsize > 24)
        fontsize = 24;
    return(fontsize);
    }


/*
** Add children windows / controls to main window
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
    WILLUSGUIRECT _crect,*crect;
    WILLUSGUICONTROL *control,*flcontrol;
    WILLUSGUICONTROL *focus_control;
    STRBUF *settings,_settings;
    K2PDFOPT_SETTINGS *k2settings;

    
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
#if (WILLUSDEBUG & 0x2000)
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
    nr=(crect->bottom*.3-crect->top)/k2gui->font.size-3;
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
        double xl;
        int recreate;

        xl = 1.00;
        x1 = x0;
        w = wmax;
        y1 = y0;
        h = (k2gui->font.size+2)*nr+4;
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
            control->font.size = k2gui->font.size*xl;
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
            h=k2gui->font.size*nr+2;
            control->rect.right++;
            control->rect.bottom=control->rect.top+h;
            }
        else
            willusgui_control_redraw(control,0);
        /*
        willusgui_window_text_render(&k2gui->mainwin,&k2gui->font,"File list",
                                        x1,y1,0x000000,-1,6);
        */
        flcontrol=control;
        }

    /* Buttons by file list */
    x0 = k2gui->control[k2gui->ncontrols-1].rect.right;
    y0 = k2gui->control[k2gui->ncontrols-1].rect.top - linesize;
    for (x1=x0,j=0,i=1;i>=0;i--,j++,x1-=(w+fs))
        {
        double xl;
        static char *button_label[2]={"&ADD FILE","&REMOVE FILE"};
        WILLUSGUIRECT trect;

        xl = 1.00;
        willusgui_window_text_extents(&k2gui->mainwin,&k2gui->font,button_label[i],&trect);
        w = trect.right*1.2;
        y1 = y0;
        h = k2gui->font.size*(xl+.4);
        control=&k2gui->control[k2gui->ncontrols+i];
        if (!already_drawn)
            {
            willusgui_control_init(control);
            strcpy(control->name,button_label[i]);
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
            }
        if (already_drawn)
            willusgui_control_redraw(control,0);
        else
            willusgui_control_create(control);
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
                              (xmax+xmin)/2-r1.right/2,ymin+fs/2,0,-1,6);
    willusgui_font_init(&font);
    font.size=(int)(fs*.7);
    willusgui_font_get(&font);
    for (y1=ymin+fs/2,i=0;i<1;i++)
        {
        WILLUSGUIRECT r2;
        willusgui_window_text_extents(&k2gui->mainwin,&font,cah[i],&r2);
        willusgui_window_text_render(&k2gui->mainwin,&font,cah[i],
                              (xmax+xmin)/2-r2.right/2,y1+r2.bottom,0,-1,6);
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
        willusgui_control_listbox_select_item(control,devprofile_name(0));
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
#if (WILLUSDEBUG & 0x2000)
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
            sprintf(buf,"%d",k2settings->dst_dpi);
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
    static char *modes[]={"default","copy","fitwidth","2-column",""};
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
    /* Determine selected device */
/*
printf("settings->s='%s'\n",settings->s);
*/
    if (settings->s!=NULL && (i=in_string(settings->s,"-mode "))>=0)
        {
        int j;

        for (j=0;j<nmodes;j++)
            if (tolower(settings->s[i+6])==tolower(modes[j][0]))
                break;
        if (j>=nmodes)
            j=0;
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
        if (k2gui->pbitmap.width==0)
            k2gui_preview_bitmap_message(&k2gui->pbitmap,728,840,0.1,"PREVIEW\n\nWINDOW");
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
        ppbb=control->rect.bottom = y1+h-1;
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
            control->color=0xffd0ff;
            strcpy(control->name,"Pre&view");
            control->label[0]='\0';
            control->parent=&k2gui->mainwin;
            }
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
            control->label[0]='\0';
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
    for (y1=y0-eheight*0.9,i=0;i<3;i++)
        {
        static char *label[]={"Env. var:","Additional options:","Command-line version of options:"};
        static char *name[]={"env","extra","cmdline args"};
        WILLUSGUIRECT r1;

        x1=x0;
        willusgui_window_text_render(&k2gui->mainwin,&k2gui->font,label[i],x1,
                                         y1+(i<2?eheight*.1:0),0,-1,0);
        willusgui_window_text_extents(&k2gui->mainwin,&k2gui->font,label[i],&r1);
        control=&k2gui->control[k2gui->ncontrols];
        if (!already_drawn)
            {
            willusgui_control_init(control);
            if (i==2)
                control->attrib |= (WILLUSGUICONTROL_ATTRIB_MULTILINE | WILLUSGUICONTROL_ATTRIB_READONLY);
            control->rect.left= i<2 ? x1+r1.right+f4 : x1;
            /* control->rect.top = i<2 ? y1-eheight*.9 : y1-eheight+linesize; */
            control->rect.top = i<2 ? y1 : y1+eheight*0.1+linesize*0.6;
            if (i==0)
                control->rect.right = control->rect.left + w-(r1.right+f4)-1-(bw+f4)*2;
            else if (i==1)
                control->rect.right = control->rect.left + w-(r1.right+f4)-1;
            else
                control->rect.right = control->rect.left + w-1;
            control->rect.bottom = control->rect.top + (i<2 ? eheight-1 : eheight*2-1);
            control->index=100+k2gui->ncontrols;
            control->font.size=k2gui->font.size;
            control->type=WILLUSGUICONTROL_TYPE_EDITBOX;
            willusgui_font_get(&control->font);
            strcpy(control->name,name[i]);
            control->parent=&k2gui->mainwin;
            }
        if (already_drawn)
            willusgui_control_redraw(control,0);
        else
            willusgui_control_create(control);
        k2gui->ncontrols++;
        if (i==0)
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
        else if (i==1)
{

#if (WILLUSDEBUG & 0x2000)
printf("cmdxtra.s='%s'\n",k2gui->cmdxtra.s);
#endif

            willusgui_control_set_text(control,k2gui->cmdxtra.s==NULL?"":k2gui->cmdxtra.s);
}
        else if (i==2)
            {
            willusgui_control_set_text(control,settings->s==NULL?"":settings->s);
            /* y1 += eheight+linesize; */
            }
 
        /* Env. var set button */
        if (i==0)
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
                    control->label[0]='\0';
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
    fm = 1.0;
    h=eheight*fm;
    if (h&1)
        h++;
    x0=crect->left+xmar;
    w= (k2gui->control[0].rect.right - x0)/2;
    y1 += eheight*0.1;
    for (i=0;i<6;i++)
        {
        char buf[32];
        static char *names[]={"maxcols","drf","cropleft","croptop","cropright","cropbottom"};
        static char *labels[]={"Max columns:","Document Resolution Factor:",
                              "Crop Margins (in): Left","Top","Right","Bottom"};

        control=&k2gui->control[k2gui->ncontrols];
        control->index=100+k2gui->ncontrols;
        k2gui->ncontrols++;
        if (!already_drawn)
            {
            WILLUSGUIRECT te;

            willusgui_control_init(control);
            strcpy(control->name,names[i]);
            control->flags=0;
            control->color=0xffb060;
            control->font.size=k2gui->font.size*fm;
            willusgui_font_get(&control->font);
            willusgui_window_text_extents(&k2gui->mainwin,&control->font,labels[i],&te);
            control->rect.left=x0+te.right+eheight*.15;
            control->rect.top=y1;
            control->rect.right=control->rect.left+eheight*(i<2 ? 1.8 : 2.1);
            control->rect.bottom=control->rect.top+eheight-1;
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
            case 2:
                sprintf(buf,"%.2f",mar_eval(k2settings->mar_left));
                break;
            case 3:
                sprintf(buf,"%.2f",mar_eval(k2settings->mar_top));
                break;
            case 4:
                sprintf(buf,"%.2f",mar_eval(k2settings->mar_right));
                break;
            case 5:
                sprintf(buf,"%.2f",mar_eval(k2settings->mar_bot));
                break;
            }
        willusgui_control_set_text(control,buf);
        if (i==1 || i==5)
            {
            y1 += eheight*1.15;
            x0=crect->left+xmar;
            }
        else
            x0=control->rect.right + eheight*(i==0 ? 1.5 : 0.4);
        }
    }

    /* Page range edit box */
    {
    static char *label[]={"Pages to convert:"};
    static char *name[]={"pages"};
    WILLUSGUIRECT r1;
    
    i=0;
    x1=x0;
    willusgui_window_text_render(&k2gui->mainwin,&k2gui->font,label[i],x1,y1+eheight*.1,0,-1,0);
    willusgui_window_text_extents(&k2gui->mainwin,&k2gui->font,label[i],&r1);
    control=&k2gui->control[k2gui->ncontrols];
    control->index=100+k2gui->ncontrols;
    k2gui->ncontrols++;
    if (!already_drawn)
        {
        willusgui_control_init(control);
        control->rect.left= x0+r1.right+f4;
        control->rect.top = y1;
        control->rect.right = control->rect.left + eheight*8;
        control->rect.bottom = control->rect.top + eheight-1;
        control->font.size=k2gui->font.size;
        control->type=WILLUSGUICONTROL_TYPE_EDITBOX;
        willusgui_font_get(&control->font);
        strcpy(control->name,name[i]);
        control->parent=&k2gui->mainwin;
        }
    if (already_drawn)
        willusgui_control_redraw(control,0);
    else
        willusgui_control_create(control);
    willusgui_control_set_text(control,k2settings->pagelist[0]=='\0'
                             ? "(all)" : k2settings->pagelist);
    y1 = control->rect.bottom + ((int)(linesize*0.1)<2 ? 2 : (int)(linesize*0.1));
    }


    /* Checkboxes */
    {
    static char *checkboxlabel[] = {"Auto&straighten","&Break after each source page",
                               "Color o&utput","Rotate output to &landscape",
                               "&Native PDF output","Right-&to-left text",
                               "Generate &marked-up source","Re-flow te&xt",
                               "Erase vertical l&ines","Fast Previe&w",
#ifdef HAVE_OCR_LIB
                               "&OCR (Tesseract)",
#endif
                               ""};
    static char *checkboxname[] = {"straighten","break","color","landscape","native",
                                   "r2l","markup","wrap","evl","autorot"
#ifdef HAVE_OCR_LIB
                                   ,"ocr"
#endif
                                   };
    int n,n2,vspacing;

    for (n=0;checkboxlabel[n][0]!='\0';n++);
    y1 += eheight/2;
    n2=(n+1)/2;
    x0=crect->left+xmar;
    w= (k2gui->control[0].rect.right - x0 - pmbw)/2;
    vspacing = k2gui->font.size*1.35;
    ybmax = 0;
    for (i=0;i<n;i++)
        {
        int c,r,checked;

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
            control->rect.bottom=control->rect.top+vspacing-1;
            if (control->rect.bottom > ybmax)
                ybmax = control->rect.bottom;
            control->font.size=k2gui->font.size;
            willusgui_font_get(&control->font);
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
        switch (i)
            {
            case 0:
                checked=k2settings->src_autostraighten>=0.;
                break;
            case 1:
                checked=k2settings->dst_break_pages;
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
                checked=k2settings->show_marked_source;
                break;
            case 7:
                checked=k2settings->text_wrap;
                break;
            case 8:
                checked=k2settings->erase_vertical_lines;
                break;
            case 9:
                checked=fabs(k2settings->src_rot-SRCROT_AUTO)<.5;
                break;
#ifdef HAVE_OCR_LIB
            case 10:
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
        }
    }

    /* Convert button */
    for (i=0;i<1;i++)
        {
        double xl;

        xl = 1.2;
        w = xl*fs*10;
        x1 = crect->left + ((xmin-crect->left)-w)/2;
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
            control->color=0xd0ffd0;
            strcpy(control->name,"*&Convert All Files");
            control->parent=&k2gui->mainwin;
            }
        if (already_drawn)
            willusgui_control_redraw(control,0);
        else
            willusgui_control_create(control);
        k2gui->ncontrols++;
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
    }


static double mar_eval(double x)

    {
    return(x<0. ? 0. : x);
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

#if (WILLUSDEBUG & 0x2000)
printf("@k2gui_main_repaint(changing=%d, needs_redraw=%d)\n",changing,needs_redraw);
#endif
    willusgui_window_get_useable_rect(&k2gui->mainwin,&rect);
    /* Has the window changed size since last being drawn? */
    newsize = ((width != rect.right-rect.left+1)
                   || (height != rect.bottom-rect.top+1));
    width = rect.right-rect.left+1;
    height = rect.bottom-rect.top+1;
    if (newsize)
        {
        /* If child controls exist, delete them so we can draw new re-sized ones */
        if (children_created)
            {
            /* k2gui_destroy_children(); */
            children_created=0;
            }
        needs_redraw=2;
        }

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
        k2gui_add_children(children_created);
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


static void k2gui_preview_start(void)

    {
    if (!k2gui_preview_done())
        k2gui_preview_terminate();
    if (k2gui->k2conv->k2files.n<1)
        {
        k2gui_preview_fail(1);
        return;
        }
    k2gui->prevthread[0]=willusgui_semaphore_create("k2pdfopt_preview");
    if (k2gui->prevthread[0]==NULL)
        {
        k2gui_preview_fail(2);
        return;
        }
    k2gui->prevthread[1]=willusgui_thread_create(k2gui_preview_make_bitmap,
                                                   (void *)k2gui->prevthread);
    if (k2gui->prevthread[1]==NULL)
        {
        willusgui_semaphore_release(k2gui->prevthread[0]);
        willusgui_semaphore_close(k2gui->prevthread[0]);
        k2gui->prevthread[0]=NULL;
        k2gui_preview_fail(3);
        }
    }


static void k2gui_preview_make_bitmap(char *data)

    {
    int n,index;
    WILLUSGUICONTROL *filelistbox,*ppage,*preview;
    K2PDFOPT_CONVERSION *k2conv;
    K2PDFOPT_OUTPUT k2out;
    STRBUF *cmdline,_cmdline;
    char *buf;
    static char *funcname="k2gui_preview_make_bitmap";

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

    k2out.bmp = &k2gui->pbitmap;
    k2out.filecount = 0;
    k2out.outname=NULL;
    k2out.bmp->width=-1;

    /* Convert it, baby! */
    k2pdfopt_proc_wildarg(&k2conv->k2settings,k2conv->k2files.file[0],1,&k2out);
    willus_mem_free((double **)&k2out.outname,funcname);

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
    }


static void k2gui_preview_cleanup(int statuscode)

    {
    if (statuscode>0)
        k2gui_preview_fail(statuscode);
    else
        k2gui_preview_refresh();
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

#endif /* HAVE_K2GUI */
