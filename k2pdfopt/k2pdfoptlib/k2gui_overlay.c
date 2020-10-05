/*
** k2gui_overlay.c  K2pdfopt WILLUSGUI for the overlay dialog box.
**                  (Non-OS-specific calls.)
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

static K2CONVBOX _k2gui_overlay;
static K2CONVBOX *k2gui_overlay=NULL;
static K2GUI *k2gui=NULL;

static void k2gui_overlay_init(void);
static void k2gui_overlay_update_window(void);
static int k2gui_overlay_create_dialog_window(char *filaname,char *pagelist,
                                              WILLUSGUIWINDOW *parent,void *hinst);
static void k2gui_overlay_start_conversion(void *data);
static int k2gui_overlay_wait_for_conversion_dialog_box_messages(void);
static void k2gui_overlay_conversion_thread_cleanup(void);
static void k2gui_overlay_update_progress_bar(double progress,int color,char *label);
static void k2gui_overlay_message_box_add_children(char *buttonlabel[],int buttons_only);


/*
**
** MAIN ENTRY FUNCTION FOR OVERLAY SELECTION
**
** Goes multithreaded during the creation of the overlay bitmap.
**
** Put up dialog box to show progress and do the conversion.
** Does not return until dialog box is closed.
**
** Returned margins are in inches
**     margins[0] = left
**     margins[1] = top
**     margins[2] = width (v2.33)
**     margins[3] = height (v2.33)
**     margins[4] = right margin (v2.34)
**     margins[5] = bottom margin (v2.34)
*/
int k2gui_overlay_get_crop_margins(K2GUI *k2gui0,char *filename,char *pagelist,double *margins)

    {
    int status,retval,ii;

#if (WILLUSDEBUGX & 0x2000000)
k2dprintf("@k2gui_overlay_get_crop_margins\n");
#endif
    k2gui = k2gui0;
    k2gui_overlay=&_k2gui_overlay;
    k2gui_overlay_init();
    if (margins[0]>0. || margins[1]>0. || margins[2]>0. || margins[3]>0.)
        for (ii=0;ii<4;ii++)
            k2gui_overlay->margins[ii]=margins[ii];
    else
        for (ii=0;ii<4;ii++)
            k2gui_overlay->margins[ii]=-10.;
        
    /* Launch conversion dialog box and start the conversion thread */
#if (WILLUSDEBUGX & 0x2000000)
k2dprintf("    starting dialog window / thread\n");
#endif
    status=k2gui_overlay_create_dialog_window(filename,pagelist,&k2gui->mainwin,
                                              willusgui_instance());
    retval=0;
#if (WILLUSDEBUGX & 0x2000000)
k2dprintf("\nk2gui_overlay_create_dialog_window returns %d\n\n",status);
#endif
    if (!status)
        {
        /* Disable parent so that convert dialog is modal. */
        willusgui_control_enable(&k2gui->mainwin,0);
#if (WILLUSDEBUGX & 0x2000000)
k2dprintf("    waiting for dialog box messages\n");
#endif
        /* Process messages from conversion dialog box */
        k2gui_overlay_wait_for_conversion_dialog_box_messages();
#if (WILLUSDEBUGX & 0x2000000)
k2dprintf("    back from overlay_wait_for_conversion, status=%d\n\n",k2gui_overlay->status);
#endif
        willusgui_control_enable(&k2gui->mainwin,1);
        if (k2gui_overlay->status==1)
            {
            for (ii=0;ii<6;ii++)
                margins[ii]=k2gui_overlay->margins[ii];
            retval=1;
            }
        k2gui_overlay_destroy();
        /* Without this, the main window seems to lose focus */
        willusgui_window_set_focus(&k2gui->mainwin);
        }
    return(retval);
    }


int k2gui_overlay_converting(void)

    {
    return(k2gui_overlay!=NULL && k2gui_overlay->converting==1);
    }


static void k2gui_overlay_init(void)

    {
    k2gui_overlay->successful=0;
    k2gui_overlay->converting=0;
    k2gui_overlay->ncontrols=0;
    k2gui_overlay->openfiles=0;
    k2gui_overlay->filelist=NULL;
    k2gui_overlay->filelist_na=0;
    bmp_init(&k2gui_overlay->bmp);
    k2gui_overlay->bmp.width=0;
    strbuf_init(&k2gui_overlay->buf);
    }

/*
** Put up dialog box and begin conversion
**
**     0 okay
**    -1 dialog box already up
**    -2 could not create window
**    -3 could not create conversion thread
**
*/
static int k2gui_overlay_create_dialog_window(char *filename,char *pagelist,
                                              WILLUSGUIWINDOW *parent,void *hinst)

    {
    WILLUSGUIRECT rect;
    int w,h;
    static void *data[3];
    static char *blabel[]={"Abort","",""};

#if (WILLUSDEBUGX & 0x2000000)
k2dprintf("@k2gui_overlay_create_dialog_window...\n");
#endif
    if (k2gui_overlay->converting)
        return(-1);
    k2gui_overlay->converting=1;
    k2gui_overlay->hinst=hinst; /* (HINSTANCE)GetModuleHandle(0); */
    willusgui_window_get_rect(parent,&rect);
    w=(rect.right-rect.left)*0.7;
    h=(rect.bottom-rect.top)*0.5;
    k2gui_overlay->mainwin.rect.left = rect.left + ((rect.right-rect.left)-w)/2;
    k2gui_overlay->mainwin.rect.top = rect.top + ((rect.bottom-rect.top)-h)/2;
    k2gui_overlay->mainwin.rect.right = k2gui_overlay->mainwin.rect.left + w - 1;
    k2gui_overlay->mainwin.rect.bottom = k2gui_overlay->mainwin.rect.top + h - 1;
    k2gui_overlay->maxwidth = rect.right-rect.left;
    k2gui_overlay->mf.size = k2gui_overlay->maxwidth/65;
    willusgui_font_get(&k2gui_overlay->mf);
    k2gui_overlay->bf.size = k2gui_overlay->maxwidth/50;
    willusgui_font_get(&k2gui_overlay->bf);
    strbuf_init(&k2gui_overlay->buf);
    k2gui_overlay->rgbcolor=0xb0b0ff;
    k2gui_overlay->hinst=hinst;
    k2gui_osdep_overlay_init(k2gui_overlay,&k2gui_overlay->mainwin,parent,hinst,k2gui_overlay->rgbcolor);
    if (k2gui_overlay->mainwin.handle==NULL)
        {
        k2gui_overlay->converting=0;
        return(-2);
        }
    /*
    ShowWindow(k2gui_overlay->hwnd,SW_SHOW);
    UpdateWindow(k2gui_overlay->hwnd);
    */
    k2gui_overlay_message_box_add_children(blabel,0);
    /* UpdateWindow(k2gui_overlay->hwnd); */
    willusgui_window_set_focus(&k2gui_overlay->control[1]);

    /*
    ** Start new thread to do the conversions
    */
    k2gui_overlay->semaphore = willusgui_semaphore_create_ex("k2pdfopt_overlay",1,1);
    if (k2gui_overlay->semaphore==NULL)
        {
        willusgui_control_enable(&k2gui->mainwin,1);
        k2gui_overlay_destroy();
        k2gui_alertbox(0,"Convert","Cannot create semaphore!");
        k2gui_overlay->converting=0;
        return(-3);
        }
    /* Signal the semaphore */
    willusgui_semaphore_status_wait(k2gui_overlay->semaphore);
/*
printf("k2conv=%p\n",k2conv);
printf("k2conv->k2settings=%p\n",&k2conv->k2settings);
*/
    data[0] = (void *)filename;
    data[1] = (void *)pagelist;

    /*
    ** Fork the new thread to k2gui_overlay_start_conversion().
    */
#if (WILLUSDEBUGX & 0x2000000)
k2dprintf("   starting separate thread...\n");
#endif
    k2gui_overlay->pid=willusgui_thread_create(k2gui_overlay_start_conversion,(void *)data);
    if (k2gui_overlay->pid==NULL)
        {
        willusgui_semaphore_close(k2gui_overlay->semaphore);
        k2gui_alertbox(0,"Overlay","Cannot start overlay thread!");
        willusgui_control_enable(&k2gui->mainwin,1);
        k2gui_overlay_destroy();
        k2gui_overlay->converting=0;
        return(-4);
        }
/*
** Test for Franco Vivona (Ittiandro).  Put in delay to see if it helps the
** conversion not crash.
*/
#if (WILLUSDEBUGX & 0x2000000)
k2dprintf("   continuing from main thread...\n");
#endif
#ifdef HAVE_WIN32_API
win_sleep(500);
#endif
    return(0);
    }


void k2gui_overlay_close_buttons(void)

    {
    static char *buttonlabel[3];
    static char buf[3][32];
    int i;

    if (k2gui_overlay->bmp.width==0)
        {
        strcpy(buf[0],"Cancel");
        buf[1][0]='\0';
        buf[2][0]='\0';
        for (i=0;i<3;i++)
            buttonlabel[i]=&buf[i][0];
        }
    else if (k2gui_overlay->bmp.width<0)
        {
        strcpy(buf[0],"Close");
        buf[1][0]='\0';
        buf[2][0]='\0';
        for (i=0;i<3;i++)
            buttonlabel[i]=&buf[i][0];
        }
    else
        {
        strcpy(buf[0],"&Set Crop Region");
        strcpy(buf[1],"&Reset Region to Entire Page");
        strcpy(buf[2],"&Cancel");
        for (i=0;i<3;i++)
            buttonlabel[i]=&buf[i][0];
        }
    k2gui_overlay_message_box_add_children(buttonlabel,1);
    }


/*
** Conversion thread.  This is launched as a separate thread to do the
** conversions while the dialog box displays the progress.
*/
static void k2gui_overlay_start_conversion(void *data)

    {
    void **ptrs;
    /* static char *funcname="k2gui_overlay_start_conversion"; */
    char *filename;
    char *pagelist;
    char buf[320];
    WILLUSBITMAP *bmp;

#if (WILLUSDEBUGX & 0x2000000)
k2dprintf("@k2gui_overlay_start_conversion thread.\n");
#endif
    ptrs=(void **)data;
    filename=(char *)ptrs[0];
    pagelist=(char *)ptrs[1];
    overwrite_set(1);
    k2gui_overlay_set_error_count(0);
    /* k2gui_overlay_freelist(); */
    overwrite_set(0);
    k2gui_overlay_set_num_pages(1);
    strncpy(k2gui_overlay->filename,k2gui_short_name(filename),255);
    k2gui_overlay->filename[255]='\0';
    sprintf(buf,"Creating overlay for %s...",k2gui_overlay->filename);
    k2gui_overlay_set_pages_completed(0,buf);
    bmp=&k2gui_overlay->bmp;
#if (WILLUSDEBUGX & 0x2000000)
k2dprintf("Getting overlay bitmap...\n");
#endif
    k2file_get_overlay_bitmap(bmp,&k2gui_overlay->dpi,filename,pagelist);
#if (WILLUSDEBUGX & 0x2000000)
k2dprintf("   Done overlay bitmap (ecount=%d)...\n",k2gui_overlay->error_count);
#endif
/*
printf(" = %d x %d\n",bmp->width,bmp->height);
*/
    /* If successful, size the window to the proportions of the loaded bitmap */
    if (k2gui_overlay->error_count==0 && bmp->width>0 && bmp->height>0)
        {
        WILLUSGUIRECT rect,r1,ru;
        int dw,dh,w,h,bh,bw,delw,delh,delw2,delh2;

        k2gui_overlay->converting=2;
        k2gui_overlay->successful=1;
        willusgui_window_get_rect(&k2gui_overlay->mainwin,&r1);
        willusgui_window_get_useable_rect(&k2gui_overlay->mainwin,&ru);
        delw=(r1.right-r1.left)-(ru.right-ru.left);
        delw2=delw + 10;
        delh=(r1.bottom-r1.top)-(ru.bottom-ru.top);
        delh2=delh + (int)k2gui_overlay->bf.size*2.0 + (int)k2gui_overlay->mf.size/4;
/*
printf("delw=%d, delw2=%d, delh=%d, delh2=%d\n",delw,delw2,delh,delh2);
*/
        willusgui_get_desktop_workarea(&rect);
        dw=rect.right-rect.left+1;
        dh=rect.bottom-rect.top+1;
        h = dh*3/4;
        bh = h - delh2;
        bw = bh*bmp->width/bmp->height;
        w = bw+delw2;
        if (w > dw*3/4)
            {
            bw =  dw*3/4-delw2;
            bh =  bw*bmp->height/bmp->width;
            }
/*
printf("bw=%d, bh=%d\n",bw,bh);
*/
        w = bw+delw2;
        h = bh+delh2;
/*
printf("w=%d, h=%d\n",w,h);
*/
        rect.left=dw/2-w/2;
        rect.right=rect.left+w-1;
        rect.top=dh/2-h/2;
        rect.bottom=rect.top+h-1;
        willusgui_window_set_pos(&k2gui_overlay->mainwin,&rect);
        k2gui_overlay_update_window();
        }
    else
        bmp->width=-1;
#if (WILLUSDEBUGX & 0x2000000)
k2dprintf("   Done overlay_start_conversion..\n",k2gui_overlay->error_count);
#endif
    k2gui_overlay_conversion_thread_cleanup();
    }


void k2gui_overlay_reset_margins(void)

    {
    int i;

    for (i=0;i<6;i++)
        k2gui_overlay->margins[i]=(i!=2 && i!=3) ? 0. : -1.;
    }


void k2gui_overlay_store_margins(WILLUSGUICONTROL *control)

    {
    WILLUSGUIRECT *marked;
    WILLUSGUIRECT rect0;
    int ii;
    
    willusgui_window_get_useable_rect(control,&rect0);
    marked=&control->rectmarked;
    if (marked->left<-9000)
        {
        k2gui_overlay_reset_margins();
        return;
        }
    k2gui_overlay->margins[0] = (double)(marked->left-rect0.left)/control->dpi_rendered;
    k2gui_overlay->margins[1] = (double)(marked->top-rect0.top)/control->dpi_rendered;
    k2gui_overlay->margins[2] = (double)(marked->right-marked->left)/control->dpi_rendered;
    k2gui_overlay->margins[3] = (double)(marked->bottom-marked->top)/control->dpi_rendered;
    k2gui_overlay->margins[4] = (double)(rect0.right-marked->right)/control->dpi_rendered;
    k2gui_overlay->margins[5] = (double)(rect0.bottom-marked->bottom)/control->dpi_rendered;
    for (ii=0;ii<6;ii++)
        {
        if (ii==2 || ii==3)
            continue;
        if (k2gui_overlay->margins[ii]<0.)
            k2gui_overlay->margins[ii]=0.;
/*
k2printf("k2gui_overlay->margins[%d]=%g\n",ii,k2gui_overlay->margins[ii]);
*/
        }
    }


void k2gui_overlay_apply_margins(WILLUSGUICONTROL *control)

    {
    WILLUSGUIRECT *marked;
    WILLUSGUIRECT rect0;

/*
printf("dpi_rendered=%g\n",(double)control->dpi_rendered);
printf("rect = %d x %d\n",rect0.right,rect0.bottom);
{int i;
for (i=0;i<4;i++)
printf("k2margins[%d]=%g\n",i,k2gui_overlay->margins[i]);   
}
*/
    if (k2gui_overlay->margins[0]<0 || k2gui_overlay->margins[1]<0)
        return;
    willusgui_window_get_useable_rect(control,&rect0);
    marked=&control->rectmarked;
    marked->left=k2gui_overlay->margins[0]*control->dpi_rendered+rect0.left;
    if (marked->left<0)
        marked->left=0;
    marked->top=k2gui_overlay->margins[1]*control->dpi_rendered+rect0.top;
    if (marked->top<0)
        marked->top=0;
/*
    marked->right=rect0.right-k2gui_overlay->margins[2]*control->dpi_rendered;
    marked->bottom=rect0.bottom-k2gui_overlay->margins[3]*control->dpi_rendered;
*/
    if (k2gui_overlay->margins[2]<0)
        marked->right=rect0.right;
    else
        marked->right=marked->left+k2gui_overlay->margins[2]*control->dpi_rendered;
    if (k2gui_overlay->margins[3]<0)
        marked->bottom=rect0.bottom;
    else
        marked->bottom=marked->top+k2gui_overlay->margins[3]*control->dpi_rendered;
/*
printf("marked=%d,%d,%d,%d\n",marked->left,marked->top,marked->right,marked->bottom);
*/
    }


static void k2gui_overlay_update_window(void)

    {
    willusgui_control_redraw(&k2gui_overlay->mainwin,0);
    }


void k2gui_overlay_error(char *filename,int pagenum,int statuscode)

    {
    char buf[512];

    if (k2gui_overlay==NULL || k2gui_overlay->converting!=1)
        return;
    if (statuscode<1 || statuscode>5)
        statuscode=1;
    sprintf(buf,"Error %d getting page %d of file %s.",statuscode,pagenum,filename);
    k2printf("\n\n** %s **\n\n\n",buf);
    k2gui_overlay_increment_error_count();
    }


void k2gui_overlay_open_bitmap(WILLUSBITMAP *bmp)

    {
    char tempfile[MAXFILENAMELEN];

    wfile_abstmpnam(tempfile);
    wfile_newext(tempfile,NULL,"png");
    bmp_write(bmp,tempfile,NULL,100);
    willusgui_open_file_ex(tempfile);
    }



/*
** Wait for conversion dialog box messages
**
** Returns:
**      bit 0:  1 if dialog box closed from button press
**              0 if dialog box closed from ESC press
**      bit 1:  1 if conversion completed normally.
**              0 if conversion was terminated early.
**
*/
static int k2gui_overlay_wait_for_conversion_dialog_box_messages(void)

    {
    int done;

#if (WILLUSDEBUGX & 0x2000000)
k2dprintf("@k2gui_overlay_wait_for_conversion_dialog_box_messages, converting=%d\n",k2gui_overlay->converting);
#endif
    done=k2gui_osdep_window_proc_messages(&k2gui_overlay->mainwin,k2gui_overlay->semaphore,2,
                                          &k2gui_overlay->control[1]);
#if (WILLUSDEBUGX & 0x2000000)
k2dprintf("    back from osdep_window_proc_messages, converting=%d\n",k2gui_overlay->converting);
#endif
    k2gui_overlay->converting=0;
    /* If conversion aborted, terminate thread and cleanup */
    if (!done)
        {
#if (WILLUSDEBUGX & 0x2000000)
k2dprintf("    terminating thread to create overlay.\n");
#endif
        willusgui_thread_terminate(k2gui_overlay->pid,0);
        k2gui_overlay_conversion_thread_cleanup();
        }
#if (WILLUSDEBUGX & 0x2000000)
k2dprintf("    closing semaphore.\n");
#endif
    willusgui_semaphore_close(k2gui_overlay->semaphore);
#if (WILLUSDEBUGX & 0x2000000)
k2dprintf("    Done k2gui_overlay_wait_for_conversion_dialog_box_messages\n");
#endif
    return((k2gui_overlay->status==1 ? 1 : 0) | (done ? 2 : 0));
    }


static void k2gui_overlay_conversion_thread_cleanup(void)

    {
    /* Release and close semaphore */
    willusgui_semaphore_release(k2gui_overlay->semaphore);
#if (WILLUSDEBUGX & 0x2000000)
k2dprintf("thread clean-up complete.  Semaphore released.\n");
#endif
    }


void k2gui_overlay_set_pages_completed(int n,char *message)

    {
    char buf[MAXFILENAMELEN+80]; /* More room, v2.22 */
    int color;
    double progress;

    if (k2gui_overlay==NULL || k2gui_overlay->converting!=1)
        return;
    color=0x4080ff;
    if (k2gui_overlay->num_pages>0)
        progress=(double)n/k2gui_overlay->num_pages;
    else
        progress=0.;
    if (message==NULL)
        sprintf(buf,"%s: %d of %d pages completed.",k2gui_overlay->filename,n,k2gui_overlay->num_pages);
    else
        if (in_string(message,"cannot")>=0)
            {
            color=0xd0d0ff;
            progress=1.;
            }
    k2gui_overlay_update_progress_bar(progress,color,message==NULL?buf:message);
    }


void k2gui_overlay_set_num_pages(int npages)

    {
    if (k2gui_overlay!=NULL && k2gui_overlay->converting==1)
        k2gui_overlay->num_pages=npages;
    }


void k2gui_overlay_set_filename(char *name)

    {
    if (k2gui_overlay!=NULL && k2gui_overlay->converting==1)
        {
        strncpy(k2gui_overlay->filename,k2gui_short_name(name),255);
        k2gui_overlay->filename[255]='\0';
        }
    }


void k2gui_overlay_set_error_count(int ecount)

    {
    if (k2gui_overlay!=NULL)
        {
        k2gui_overlay->error_count=ecount;
#if (WILLUSDEBUGX & 0x2000)
printf("\n error count set to %d\n\n",k2gui_overlay->error_count);
#endif
        }
    }


void k2gui_overlay_increment_error_count(void)

    {
    if (k2gui_overlay!=NULL && k2gui_overlay->converting==1)
        {
        k2gui_overlay->error_count++;
#if (WILLUSDEBUGX & 0x2000)
printf("\n error count incremented to %d\n\n",k2gui_overlay->error_count);
#endif
        }
    }


int k2gui_overlay_vprintf(FILE *f,char *fmt,va_list args)

    {
    static char *funcname="k2gui_overlay_vprintf";
    char prbuf[1024];
    char *buf;
    int i,j,status,nlines;

    if (k2gui_overlay==NULL || k2gui_overlay->control[0].handle==NULL || k2gui_overlay->ncontrols<1)
        return(-1);
    status=vsprintf(prbuf,fmt,args);
    willus_mem_alloc_warn((void **)&buf,strlen(prbuf)*2,funcname,10);
    for (i=j=0;prbuf[i]!='\0';i++)
        {
        /* Insert CRs before LFs */
        if (prbuf[i]=='\r' && prbuf[i+1]=='\n')
            {
            buf[j++]=prbuf[i++];
            buf[j++]=prbuf[i];
            continue;
            }
        if (prbuf[i]=='\n')
            {
            buf[j++]='\r';
            buf[j++]=prbuf[i];
            continue;
            }
        /* Remove any ANSI color code */
        if (prbuf[i]=='\x1b' && prbuf[i+1]=='[')
            {
            for (i+=2;prbuf[i]!='\0' && (tolower(prbuf[i])<'a' || tolower(prbuf[i])>'z');i++);
            if (prbuf[i]=='\0')
                break;
            continue;
            }
        buf[j++]=prbuf[i];
        }
    buf[j]='\0';
    strbuf_sprintf(&k2gui_overlay->buf,"%s",buf);
    nlines=willusgui_control_nlines(&k2gui_overlay->control[0]);
    willusgui_control_set_text(&k2gui_overlay->control[0],strbuf_lineno(&k2gui_overlay->buf,-nlines));
    willusgui_control_scroll_to_bottom(&k2gui_overlay->control[0]);
    willus_mem_free((double **)&buf,funcname);
    return(status);
    }


void k2gui_overlay_final_print(void)

    {
    willusgui_control_set_text(&k2gui_overlay->control[0],k2gui_overlay->buf.s);
    willusgui_control_scroll_to_bottom(&k2gui_overlay->control[0]);
    }


/* Clean up */
void k2gui_overlay_terminate_conversion(void)

    {
k2printf("Releasing semaphore.\r\n");
    willusgui_semaphore_release(k2gui_overlay->semaphore);
    willusgui_thread_exit(0);
    }
    

static void k2gui_overlay_update_progress_bar(double progress,int color,char *label)

     {
     int x1,y1,w,h;
     WILLUSGUIRECT rect;
     WILLUSGUIRECT drect;

     if (progress<0.)
         progress=0.;
     if (progress>1.)
         progress=1.;
     willusgui_window_get_useable_rect(&k2gui_overlay->mainwin,&rect);
     w=(rect.right-rect.left)*.95;
     h=k2gui_overlay->bf.size*1.4;
     x1=rect.left+(rect.right-rect.left-w)/2;
     y1=k2gui_overlay->control[1].rect.top - h*.05 - h*1.15;
     drect.left=x1+1;
     drect.right=x1+w*progress+1;
     drect.top=y1;
     drect.bottom=y1+h;
     if (drect.right>drect.left)
         willusgui_window_draw_rect_filled(&k2gui_overlay->mainwin,&drect,color);
     drect.left=drect.right+1;
     drect.right=x1+w;
     if (drect.right>drect.left)
         willusgui_window_draw_rect_filled(&k2gui_overlay->mainwin,&drect,0xffffff);
     drect.left=x1;
     drect.right=x1+w+1;
     drect.bottom++;
     willusgui_window_draw_rect_outline(&k2gui_overlay->mainwin,&drect,0);
     willusgui_window_text_render(&k2gui_overlay->mainwin,&k2gui_overlay->bf,label,
                                    (int)(x1+w/2),(int)(y1+h*.2),0,-1,1,NULL);
     }
     

static void k2gui_overlay_message_box_add_children(char *buttonlabel[],int buttons_only)

    {
    int i,width,w,nb,dx,dx1,x1,y1,attrib;
    WILLUSGUIRECT rect;
    WILLUSGUIRECT brect[3];
    WILLUSGUICONTROL *control;
    /*
    static char *buttonlabel[] = {"Abort","",""};
    */

/*
    {
    HWND dummy;
    DWORD bclass;

    dummy = CreateWindow("button",NULL,WS_CHILD|BS_OWNERDRAW,
                         0,0,0,0,k2gui_overlay->hwnd,(HMENU)0,k2gui_overlay->hinstance,NULL);
    bclass = GetClassLong(dummy,GCL_STYLE);
    SetClassLong(dummy,GCL_STYLE,bclass & ~CS_DBLCLKS);
    DestroyWindow(dummy);
    }
*/
    if (k2gui_overlay->ncontrols>0)
        {
        for (i=k2gui_overlay->ncontrols-1;i>=0;i--)
            willusgui_control_close(&k2gui_overlay->control[i]);
        k2gui_overlay->ncontrols=0;
        }
    willusgui_window_get_useable_rect(&k2gui_overlay->mainwin,&rect);
    width=rect.right-rect.left;
/*
printf("Add children, useablerect = %d x %d\n",width,rect.bottom-rect.top);
*/
    y1=rect.bottom - (int)k2gui_overlay->bf.size*1.5;
    for (nb=w=0,i=1;i<=3;i++)
        {
        if (buttonlabel[i-1][0]=='\0')
            continue;
        willusgui_window_text_extents(&k2gui_overlay->mainwin,&k2gui_overlay->bf,buttonlabel[i-1],&brect[i-1]);
        brect[i-1].right += 0.5*k2gui_overlay->bf.size;
        w += brect[i-1].right;
        nb++;
        }
    dx=(width-6-w)/(nb+1);
    dx1=k2gui_overlay->bf.size;
    if (dx > dx1)
        dx=dx1;
    if (nb==0)
        x1=rect.left+width/2;
    else
        x1=rect.left+(width-6-w-dx*(nb-1))/2;

    /* Edit box */
    if (k2gui_overlay->bmp.width<=0 && k2gui_overlay->converting<=1)
        {
        attrib=0;
        if (!buttons_only)
            {
            control=&k2gui_overlay->control[0];
            willusgui_control_init(control);
            control->attrib = WILLUSGUICONTROL_ATTRIB_READONLY | WILLUSGUICONTROL_ATTRIB_MULTILINE 
                                 | WILLUSGUICONTROL_ATTRIB_SCROLLBARS;
            control->rect.left = rect.left + width*0.02;
            control->rect.right = rect.left + width*0.95;
            control->rect.top = rect.top + k2gui_overlay->mf.size/4;
            control->rect.bottom = (y1 - k2gui_overlay->bf.size*2.0);
            control->font.size = k2gui_overlay->mf.size;
            willusgui_font_get(&control->font);
            control->type=WILLUSGUICONTROL_TYPE_EDITBOX;
            control->index=10;
            strcpy(control->name,"k2pdfopt output");
            control->parent=&k2gui_overlay->mainwin;
            willusgui_control_create(control);
            }
        }
    else
        {
        control=&k2gui_overlay->control[0];
        willusgui_control_init(control);
        strcpy(control->name,"bitmapwin");
        control->flags=0;
        control->color=0;
        attrib = control->attrib = WILLUSGUICONTROL_ATTRIB_NOKEYS;
        control->font.size = k2gui_overlay->mf.size;
        willusgui_font_get(&control->font);
        control->rect.left = rect.left + 5;
        control->rect.right = rect.left + width-5;
        control->rect.top = rect.top + k2gui_overlay->mf.size/4;
        control->rect.bottom = (y1 - k2gui_overlay->bf.size*0.5);
        control->type=WILLUSGUICONTROL_TYPE_BITMAP;
        control->dpi=control->dpi_rendered=k2gui_overlay->dpi;
        control->index=10;
        control->parent=&k2gui_overlay->mainwin;
        control->label[0]='\0';
        control->obmp=&k2gui_overlay->bmp;
        willusgui_control_create(control);
        k2gui_overlay_apply_margins(&k2gui_overlay->control[0]);
        }
    k2gui_overlay->ncontrols=1;

    /* Button */
    if (buttons_only)
        k2gui_overlay_draw_defbutton_border(0);
    for (i=1;i<=3;i++)
        {
        control=&k2gui_overlay->control[i];
        if (control->handle!=NULL)
            willusgui_control_close(control);
        willusgui_control_init(control);
        strcpy(control->name,buttonlabel[i-1]);
        if (control->name[0]=='\0')
            continue;
        control->color=0xf0f0f0;
        /* defbutton=(control->text[0]=='*'); */
        control->attrib = attrib;
        control->rect.left = x1;
        control->rect.right = x1+brect[i-1].right;
        control->rect.top = y1;
        control->rect.bottom = y1+k2gui_overlay->bf.size*1.2;
        control->font.size = k2gui_overlay->bf.size;
        willusgui_font_get(&control->font);
        control->type=WILLUSGUICONTROL_TYPE_BUTTON;
        control->index=2+i;
        control->parent=&k2gui_overlay->mainwin;
        willusgui_control_create(control);
        k2gui_overlay->ncontrols++;
        x1 += (control->rect.right-control->rect.left)+dx;
        }
    k2gui_overlay_draw_defbutton_border(1);
/*
dprintf(NULL,"msg=%s, k2gui_overlay->inbuf=%p, right-left=%d\n",
k2gui_overlay->msg,k2gui_overlay->inbuf,k2gui_overlay->aboutbox.right-k2gui_overlay->aboutbox.left);
*/
    }


void k2gui_overlay_draw_defbutton_border(int status)

    {
    WILLUSGUIRECT rect;
    int i,color;

#if (WILLUSDEBUGX & 0x2000000)
willusgui_dprintf(ANSI_YELLOW "@k2gui_overlay_draw_defbutton_border\n");
#endif
    rect=k2gui_overlay->control[1].rect;
    color = status ? 0 : k2gui_overlay->rgbcolor;
    for (i=0;i<4;i++)
        {
        rect.top--;
        rect.bottom++;
        rect.left--;
        rect.right++;
        if (i>1)
            willusgui_window_draw_rect_outline(&k2gui_overlay->mainwin,&rect,color);
        }
    }


void k2gui_overlay_destroy(void)

    {
    int i;

    if (!k2gui_overlay->converting)
        return;
    bmp_free(&k2gui_overlay->bmp);
    for (i=k2gui_overlay->ncontrols-1;i>=0;i--)
        willusgui_control_close(&k2gui_overlay->control[i]);
    k2gui_overlay->ncontrols=0;
    willusgui_font_release(&k2gui_overlay->mf);
    willusgui_font_release(&k2gui_overlay->bf);
    willusgui_control_close(&k2gui_overlay->mainwin);
    /* UnregisterClass(k2gui_overlay->class,k2gui_overlay->hinstance); */
    k2gui_overlay->converting=0;
    }


int k2gui_overlay_conversion_successful(void)

    {
    return(k2gui_overlay->successful);
    }
#endif /* HAVE_K2GUI */
