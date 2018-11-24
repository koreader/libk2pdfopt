/*
** k2gui_cbox.c   K2pdfopt WILLUSGUI for the conversion dialog box.
**                (Non-OS-specific calls.)
**
** Copyright (C) 2017  http://willus.com
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

static K2CONVBOX _k2gui_cbox;
static K2CONVBOX *k2gui_cbox=NULL;
static K2GUI *k2gui=NULL;

static int str0_len(char *list);
static void str0_addstring(char **list,int *na,char *s);
static void str0_free(char **list,int *na);
static char *str0_string(char *list,int index);
static int str0_n(char *list);

static int   k2gui_cbox_nfiles(void);
static char *k2gui_cbox_converted_file(int index);
static int   k2gui_cbox_nfolders(void);
static char *k2gui_cbox_folder(int index);
static int   same_folder(char *file1,char *file2);
static char *folder_name(char *filename);

static void k2gui_cbox_init(void);
static int k2gui_cbox_create_dialog_window(K2PDFOPT_CONVERSION *k2conv,STRBUF *env,STRBUF *cmdline,
                                           WILLUSGUIWINDOW *parent,void *hinst);
static void k2gui_cbox_start_conversion(void *data);
static int k2gui_cbox_wait_for_conversion_dialog_box_messages(void);
static void k2gui_cbox_conversion_thread_cleanup(void);
static void k2gui_cbox_update_progress_bar(int index,double progress,int color,char *label);
static void k2gui_cbox_message_box_add_children(char *buttonlabel[],int buttons_only);


/*
** Put up dialog box to show progress and do the conversion.
** Does not return until dialog box is closed.
*/
void k2gui_cbox_do_conversion(K2GUI *k2gui0)

    {
    int status;
    STRBUF *cmdline,_cmdline;
    cmdline=&_cmdline;
    strbuf_init(cmdline);

    k2gui = k2gui0;
    k2gui_cbox=&_k2gui_cbox;
    k2gui_cbox_init();
    /* Launch conversion dialog box and start the conversion thread */
    /* This will fork a thread that starts k2gui_cbox_start_conversion() */
    status=k2gui_cbox_create_dialog_window(k2gui->k2conv,k2gui->env,cmdline,/* k2gui->cmdline, */
                                           &k2gui->mainwin,willusgui_instance());
    if (!status)
        {
        /* Disable parent so that convert dialog is modal. */
        willusgui_control_enable(&k2gui->mainwin,0);
        /* Process messages from conversion dialog box */
        k2gui_cbox_wait_for_conversion_dialog_box_messages();
        willusgui_control_enable(&k2gui->mainwin,1);
        k2gui_cbox_destroy();
        /* Without this, the main window seems to lose focus */
        willusgui_window_set_focus(&k2gui->mainwin);
        }

    strbuf_free(cmdline);
    }


int k2gui_cbox_converting(void)

    {
    return(k2gui_cbox!=NULL && k2gui_cbox->converting);
    }


void k2gui_okay(char *title,char *fmt,...)

    {
    va_list args;
    char buf[500];
    static int bcolors[3]={0x6060b0,0xf0f0f0,0xf0f0f0};

    va_start(args,fmt);
    vsprintf(buf,fmt,args);
    willusgui_message_box(k2gui_cbox->mainwin.handle!=NULL ? &k2gui_cbox->mainwin:&k2gui->mainwin,
                          title,buf,"*&OK",NULL,NULL,NULL,0,24,600,0xe0e0e0,bcolors,NULL,1);
    }

/*
** 1=yes
** 2=no
*/
int k2gui_yesno(char *title,char *fmt,...)

    {
    va_list args;
    char buf[500];
    static int bcolors[3]={0x6060b0,0xf0f0f0,0xf0f0f0};

    va_start(args,fmt);
    vsprintf(buf,fmt,args);
    return(willusgui_message_box(k2gui_cbox->mainwin.handle!=NULL ? &k2gui_cbox->mainwin : &k2gui->mainwin,
                          title,buf,"&YES","*&NO",NULL,NULL,0,24,600,0xe0e0e0,bcolors,NULL,1));
    }


/*
** 1=yes this once
** 2=no to all
** 3=yes to all
*/
int k2gui_yes_no_all(char *title,char *fmt,...)

    {
    va_list args;
    char buf[500];
    static int bcolors[3]={0x6060b0,0xf0f0f0,0xf0f0f0};

    va_start(args,fmt);
    vsprintf(buf,fmt,args);
    return(willusgui_message_box(k2gui_cbox->mainwin.handle!=NULL ? &k2gui_cbox->mainwin
                                                                  : &k2gui->mainwin,
                          title,buf,"*&YES (THIS ONCE)","&NO (TO ALL)","YES TO &ALL",
                          NULL,0,24,600,0xe0e0e0,bcolors,NULL,1));
    }


static void k2gui_cbox_init(void)

    {
    k2gui_cbox->successful=0;
    k2gui_cbox->converting=0;
    k2gui_cbox->ncontrols=0;
    k2gui_cbox->openfiles=0;
    k2gui_cbox->filelist=NULL;
    k2gui_cbox->filelist_na=0;
    strbuf_init(&k2gui_cbox->buf);
    }


/*
** Returns total memory used minus 1 unless list==NULL, in which case returns 0.
*/
static int str0_len(char *list)

    {
    int i;

    if (list==NULL)
        return(0);
    for (i=0;list[i]!='\0' || list[i+1]!='\0';i++);
    return(i+1);
    }


static void str0_addstring(char **list,int *na,char *s)

    {
    char *d;
    int i,needed;
    static char *funcname="str0_addstring";

    if (s==NULL || s[0]=='\0')
        return;
    i=str0_len(*list);
    needed=i+strlen(s)+2;
    if ((*list)==NULL || (needed > (*na)))
        {
        int newsize;
        newsize = (*na)<512 ? 1024 : (*na)*2;
        while (newsize < needed)
            newsize *= 2;
        willus_mem_realloc_robust_warn((void **)list,newsize,(*na),funcname,10);
        (*na)=newsize;
        }
    d=(*list);
    strcpy(&d[i],s);
    d[i+strlen(s)+1]='\0';
    }


static void str0_free(char **list,int *na)

    {
    static char *funcname="str0_free";

    willus_mem_free((double **)list,funcname);
    (*na)=0;
    }


static char *str0_string(char *list,int index)

    {
    int i,j;

    if (list==NULL)
        return(NULL);
    for (i=j=0;i<index;i++)
        {
        j += strlen(&list[j])+1;
        if (list[j]=='\0')
            return(NULL);
        }
    return(&list[j]);
    }
            
     
static int str0_n(char *list)

    {
    int i,j;

    if (list==NULL)
        return(0);
    for (i=j=0;list[j]!='\0';i++,j+=strlen(&list[j])+1);
    return(i);
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
static int k2gui_cbox_create_dialog_window(K2PDFOPT_CONVERSION *k2conv,STRBUF *env,STRBUF *cmdline,
                                           WILLUSGUIWINDOW *parent,void *hinst)

    {
    WILLUSGUIRECT rect;
    int w,h;
    static void *data[4];
    static char *blabel[]={"Abort","",""};

#if (WILLUSDEBUGX & 0x2000)
printf("@k2gui_cbox_create_dialog_window...\n");
#endif
    if (k2gui_cbox->converting)
        return(-1);
    k2gui_cbox->converting=1;
    k2gui_cbox->hinst=hinst; /* (HINSTANCE)GetModuleHandle(0); */
    willusgui_window_get_rect(parent,&rect);
    w=(rect.right-rect.left)*0.9;
    h=(rect.bottom-rect.top)*0.75;
    k2gui_cbox->mainwin.rect.left = rect.left + ((rect.right-rect.left)-w)/2;
    k2gui_cbox->mainwin.rect.top = rect.top + ((rect.bottom-rect.top)-h)/2;
    k2gui_cbox->mainwin.rect.right = k2gui_cbox->mainwin.rect.left + w - 1;
    k2gui_cbox->mainwin.rect.bottom = k2gui_cbox->mainwin.rect.top + h - 1;
    k2gui_cbox->maxwidth = rect.right-rect.left;
    k2gui_cbox->mf.size = k2gui_cbox->maxwidth/65;
    willusgui_font_get(&k2gui_cbox->mf);
    k2gui_cbox->bf.size = k2gui_cbox->maxwidth/40;
    willusgui_font_get(&k2gui_cbox->bf);
    strbuf_init(&k2gui_cbox->buf);
    k2gui_cbox->rgbcolor=0xb0b0ff;
    k2gui_cbox->hinst=hinst;
    k2gui_osdep_cbox_init(k2gui_cbox,&k2gui_cbox->mainwin,parent,hinst,k2gui_cbox->rgbcolor);
    if (k2gui_cbox->mainwin.handle==NULL)
        {
        k2gui_cbox->converting=0;
        return(-2);
        }
    /*
    ShowWindow(k2gui_cbox->hwnd,SW_SHOW);
    UpdateWindow(k2gui_cbox->hwnd);
    */
    k2gui_cbox_message_box_add_children(blabel,0);
    /* UpdateWindow(k2gui_cbox->hwnd); */
    willusgui_window_set_focus(&k2gui_cbox->control[1]);

    /*
    ** Start new thread to do the conversions
    */
    k2gui_cbox->semaphore = willusgui_semaphore_create_ex("k2pdfopt_conversion",1,1);
    if (k2gui_cbox->semaphore==NULL)
        {
        willusgui_control_enable(&k2gui->mainwin,1);
        k2gui_cbox_destroy();
        k2gui_alertbox(0,"Convert","Cannot create semaphore!");
        k2gui_cbox->converting=0;
        return(-3);
        }
    willusgui_semaphore_status_wait(k2gui_cbox->semaphore);
/*
printf("k2conv=%p\n",k2conv);
printf("k2conv->k2settings=%p\n",&k2conv->k2settings);
*/
    data[0] = (void *)k2conv;
    data[1] = (void *)env;
    data[2] = (void *)cmdline;

    /*
    ** Fork the new thread to k2gui_cbox_start_conversion().
    */
    k2gui_cbox->pid=willusgui_thread_create(k2gui_cbox_start_conversion,(void *)data);
    if (k2gui_cbox->pid==NULL)
        {
        willusgui_semaphore_close(k2gui_cbox->semaphore);
        k2gui_alertbox(0,"Convert","Cannot start conversion thread!");
        willusgui_control_enable(&k2gui->mainwin,1);
        k2gui_cbox_destroy();
        k2gui_cbox->converting=0;
        return(-4);
        }
/*
** Test for Franco Vivona (Ittiandro).  Put in delay to see if it helps the
** conversion not crash.
*/
#ifdef HAVE_WIN32_API
win_sleep(500);
#endif

    return(0);
    }


void k2gui_cbox_close_buttons(void)

    {
    static char *buttonlabel[3];
    static char buf[3][32];
    int i;

    strcpy(buf[0],"Close");
    if (k2gui_cbox_nfiles()>0)
        {
        sprintf(buf[1],"Open File%s",k2gui_cbox_nfiles()==1?"":"s");
        sprintf(buf[2],"Open Containing Folder%s",k2gui_cbox_nfolders()==1?"":"s");
        }
    else
        {
        buf[1][0]='\0';
        buf[2][0]='\0';
        }
    for (i=0;i<3;i++)
        buttonlabel[i]=&buf[i][0];
    k2gui_cbox_message_box_add_children(buttonlabel,1);
    }


/*
** Conversion thread.  This is launched as a separate thread to do the
** conversions while the dialog box displays the progress.
*/
static void k2gui_cbox_start_conversion(void *data)

    {
    void **ptrs;
    int i;
    K2PDFOPT_CONVERSION *k2conv;
    K2PDFOPT_SETTINGS *k2settings;
    K2PDFOPT_FILELIST_PROCESS k2listproc;
    double start,stop;
    static char *funcname="k2gui_cbox_start_conversion";

    ptrs=(void **)data;
    k2conv=(K2PDFOPT_CONVERSION *)ptrs[0];
    k2settings=&k2conv->k2settings;
    /*
    ** Count files
    */
    start=(double)clock()/CLOCKS_PER_SEC;
    k2gui_cbox_set_num_files(1);
    k2gui_cbox_set_files_completed(0,"Counting files...");
    overwrite_set(1);
    for (i=k2listproc.filecount=0;i<k2conv->k2files.n;i++)
        {
        k2listproc.bmp=NULL;
        k2listproc.outname=NULL;
        k2listproc.mode=K2PDFOPT_FILELIST_PROCESS_MODE_GET_FILECOUNT;
        k2pdfopt_proc_wildarg(k2settings,k2conv->k2files.file[i],&k2listproc);
        willus_mem_free((double **)&k2listproc.outname,funcname);
        }
    if (k2listproc.filecount==0)
        {
        k2gui_cbox_set_files_completed(0,"No files");
        k2gui_alertbox(0,"No files","No files can be opened for conversion.");
        k2gui_cbox_conversion_thread_cleanup();
        return;
        }
    k2gui_cbox_set_num_files(k2listproc.filecount);
    k2gui_cbox_set_files_completed(0,NULL);
    /*
    ** Process files
    */
    k2gui_cbox_set_error_count(0);
    k2gui_cbox_freelist();
    overwrite_set(0);
    for (i=k2listproc.filecount=0;i<k2conv->k2files.n;i++)
        {
        char *buf;
        K2PDFOPT_CONVERSION *k2c;
        STRBUF *cmdline,_cmdline;

        willus_mem_alloc_warn((void **)&buf,1024,funcname,10);
        k2gui_cbox_set_num_pages(1);
        sprintf(buf,"Starting conversion of %s...",k2gui_short_name(k2conv->k2files.file[i]));
        k2gui_cbox_set_pages_completed(0,buf);
        willus_mem_alloc_warn((void **)&k2c,sizeof(K2PDFOPT_CONVERSION),funcname,10);
        k2pdfopt_conversion_init(k2c);
        cmdline=&_cmdline;
        strbuf_init(cmdline);
        k2gui_settings_to_cmdline(cmdline,&k2conv->k2settings);
        parse_cmd_args(k2c,k2gui->env,cmdline,&k2gui->cmdxtra,1,1);
        k2listproc.outname=NULL;
        k2listproc.bmp=NULL;
        k2listproc.mode=K2PDFOPT_FILELIST_PROCESS_MODE_CONVERT_FILES;
        k2pdfopt_proc_wildarg(&k2c->k2settings,k2conv->k2files.file[i],&k2listproc);
#if (WILLUSDEBUGX & 0x2000)
printf("\n\nDone conversion...\n\n");
#endif
        str0_addstring(&k2gui_cbox->filelist,&k2gui_cbox->filelist_na,k2listproc.outname);
        willus_mem_free((double **)&k2listproc.outname,funcname);
        k2pdfopt_conversion_close(k2c);
        willus_mem_free((double **)&k2c,funcname);
        willus_mem_free((double **)&buf,funcname);
        }
    k2gui_cbox_set_files_completed(k2listproc.filecount,NULL);
    stop=(double)clock()/CLOCKS_PER_SEC;
    k2sys_cpu_update(k2settings,start,stop);
    if (k2listproc.filecount==k2conv->k2files.n && k2gui_cbox->error_count==0)
        k2gui_cbox->successful=1;
    k2gui_cbox_conversion_thread_cleanup();
    }


void k2gui_cbox_error(char *filename,int statuscode)

    {
    static char *err[] = {"Uknown error","No bitmaps found","No DjVuLibre library",
                          "File overwrite not allowed","","Uknown error"};
    char buf[512];

    if (k2gui_cbox==NULL || !k2gui_cbox->converting)
        return;
    if (statuscode<1 || statuscode>5)
        statuscode=1;
    sprintf(buf,"Conversion of file %s aborted (%s).",filename,err[statuscode-1]);
    k2printf("\n\n** %s **\n\n\n",buf);
    k2gui_cbox_increment_error_count();
    k2gui_cbox_set_pages_completed(0,buf);
    }


void k2gui_cbox_open_files(void)

    {
    int i,n;

    n=k2gui_cbox_nfiles();
    for (i=0;i<n;i++)
        willusgui_open_file_ex(k2gui_cbox_converted_file(i));
    }


void k2gui_cbox_open_folders(void)

    {
    int i,n;

    n=k2gui_cbox_nfolders();
    for (i=0;i<n;i++)
        willusgui_open_file(k2gui_cbox_folder(i));
    }


static int k2gui_cbox_nfiles(void)

    {
#if (WILLUSDEBUGX & 0x2000)
printf("nfiles=%d\n",str0_n(k2gui_cbox->filelist));
#endif
    return(str0_n(k2gui_cbox->filelist));
    }


static char *k2gui_cbox_converted_file(int index)

    {
    return(str0_string(k2gui_cbox->filelist,index));
    }


static int k2gui_cbox_nfolders(void)

    {
    int i,j,n,nc;

    n=k2gui_cbox_nfiles();
    if (n<2)
        return(n);
    for (i=1,nc=1;i<n;i++)
        {
        for (j=0;j<i;j++)
            if (same_folder(k2gui_cbox_converted_file(j),k2gui_cbox_converted_file(i)))
                break;
        if (j==i)
            nc++;
        }
#if (WILLUSDEBUGX & 0x2000)
printf("nfolders=%d\n",nc);
#endif
    return(nc);
    }


static char *k2gui_cbox_folder(int index)

    {
    int i,j,n,nc;

    n=k2gui_cbox_nfiles();
    if (n<=0)
        return(NULL);
    if (index==0)
        return(folder_name(k2gui_cbox_converted_file(0)));
    if (n<2)
        return(NULL);
    for (i=1,nc=0;i<n;i++)
        {
        for (j=0;j<i;j++)
            if (same_folder(k2gui_cbox_converted_file(j),k2gui_cbox_converted_file(i)))
                break;
        if (j==i)
            {
            nc++;
            if (index==nc)
                return(folder_name(k2gui_cbox_converted_file(i)));
            }
        }
    return(NULL);
    }


void k2gui_cbox_freelist(void)

    {
    str0_free(&k2gui_cbox->filelist,&k2gui_cbox->filelist_na);
    }


static int same_folder(char *file1,char *file2)

    {
    char f1[512],f2[512];

    strncpy(f1,folder_name(file1),511);
    f1[511]='\0';
    strncpy(f2,folder_name(file2),511);
    f2[511]='\0';
    return(!wfile_filename_compare(f1,f2));
    }


static char *folder_name(char *filename)

    {
    static char path[512];

    strcpy(path,filename);
    wfile_make_absolute(path);
    wfile_basepath(path,NULL);
    return(path);
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
static int k2gui_cbox_wait_for_conversion_dialog_box_messages(void)

    {
    int done;

    done=k2gui_osdep_window_proc_messages(&k2gui_cbox->mainwin,k2gui_cbox->semaphore,1,
                                          &k2gui_cbox->control[1]);
    k2gui_cbox->converting=0;
    /* If conversion aborted, terminate thread and cleanup */
    if (!done)
        {
        willusgui_thread_terminate(k2gui_cbox->pid,0);
        k2gui_cbox_conversion_thread_cleanup();
        }
    willusgui_semaphore_close(k2gui_cbox->semaphore);
    return((k2gui_cbox->status==1 ? 1 : 0) | (done ? 2 : 0));
    }


static void k2gui_cbox_conversion_thread_cleanup(void)

    {
    /* Release and close semaphore */
    willusgui_semaphore_release(k2gui_cbox->semaphore);
    }


void k2gui_cbox_set_files_completed(int nfiles,char *message)

    {
    char buf[256];
    int color;

    if (k2gui_cbox==NULL || !k2gui_cbox->converting)
        return;
    if (message==NULL)
        sprintf(buf,"%d of %d file%s completed.",nfiles,k2gui_cbox->num_files,k2gui_cbox->num_files==1?"":"s");
    else
        {
        strncpy(buf,message,255);
        buf[255]='\0';
        }
    color=0x40ff40;
    if (k2gui_cbox->error_count>0)
        {
        if (message==NULL)
            sprintf(&buf[strlen(buf)],"  There w%s %d error%s--see log above.",
                    k2gui_cbox->error_count==1?"as":"ere",
                    k2gui_cbox->error_count,
                    k2gui_cbox->error_count==1?"":"s");
        color=0x8080ff;
        }
    else if (nfiles==k2gui_cbox->num_files && message==NULL)
        {
        if (k2gui_cbox->num_files==1)
            strcat(buf,"  Conversion completed!");
        else
            strcat(buf,"  All conversions completed!");
        }
    k2gui_cbox_update_progress_bar(2,(double)nfiles/k2gui_cbox->num_files,color,buf);
    }


void k2gui_cbox_set_pages_completed(int n,char *message)

    {
    char buf[MAXFILENAMELEN+80]; /* More room, v2.22 */
    int color;
    double progress;

    if (k2gui_cbox==NULL || !k2gui_cbox->converting)
        return;
    color=0xd0ffd0;
    if (k2gui_cbox->num_pages>0)
        progress=(double)n/k2gui_cbox->num_pages;
    else
        progress=0.;
    if (message==NULL)
        sprintf(buf,"%s: %d of %d pages completed.",k2gui_cbox->filename,n,k2gui_cbox->num_pages);
    else
        if (in_string(message,"cannot")>=0)
            {
            color=0xd0d0ff;
            progress=1.;
            }
    k2gui_cbox_update_progress_bar(1,progress,color,message==NULL?buf:message);
    }


void k2gui_cbox_set_num_files(int nfiles)

    {
    if (k2gui_cbox!=NULL && k2gui_cbox->converting)
        k2gui_cbox->num_files=nfiles;
    }


void k2gui_cbox_set_num_pages(int npages)

    {
    if (k2gui_cbox!=NULL && k2gui_cbox->converting)
        k2gui_cbox->num_pages=npages;
    }


void k2gui_cbox_set_filename(char *name)

    {
    if (k2gui_cbox!=NULL && k2gui_cbox->converting)
        {
        strncpy(k2gui_cbox->filename,k2gui_short_name(name),255);
        k2gui_cbox->filename[255]='\0';
        }
    }


void k2gui_cbox_set_error_count(int ecount)

    {
    if (k2gui_cbox!=NULL)
        {
        k2gui_cbox->error_count=ecount;
#if (WILLUSDEBUGX & 0x2000)
printf("\n error count set to %d\n\n",k2gui_cbox->error_count);
#endif
        }
    }


void k2gui_cbox_increment_error_count(void)

    {
    if (k2gui_cbox!=NULL && k2gui_cbox->converting)
        {
        k2gui_cbox->error_count++;
#if (WILLUSDEBUGX & 0x2000)
printf("\n error count incremented to %d\n\n",k2gui_cbox->error_count);
#endif
        }
    }


int k2gui_cbox_vprintf(FILE *f,char *fmt,va_list args)

    {
    static char *funcname="k2gui_cbox_vprintf";
    char prbuf[1024];
    char *buf;
    int i,j,status,nlines;

    if (k2gui_cbox==NULL || k2gui_cbox->control[0].handle==NULL || k2gui_cbox->ncontrols<1)
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
    strbuf_sprintf(&k2gui_cbox->buf,"%s",buf);
    nlines=willusgui_control_nlines(&k2gui_cbox->control[0]);
    willusgui_control_set_text(&k2gui_cbox->control[0],strbuf_lineno(&k2gui_cbox->buf,-nlines));
    willusgui_control_scroll_to_bottom(&k2gui_cbox->control[0]);
    willus_mem_free((double **)&buf,funcname);
    return(status);
    }


void k2gui_cbox_final_print(void)

    {
    willusgui_control_set_text(&k2gui_cbox->control[0],k2gui_cbox->buf.s);
    willusgui_control_scroll_to_bottom(&k2gui_cbox->control[0]);
    }


/* Clean up */
void k2gui_cbox_terminate_conversion(void)

    {
k2printf("Releasing semaphore.\r\n");
    willusgui_semaphore_release(k2gui_cbox->semaphore);
    willusgui_thread_exit(0);
    }
    

static void k2gui_cbox_update_progress_bar(int index,double progress,int color,char *label)

     {
     int x1,y1,w,h;
     WILLUSGUIRECT rect;
     WILLUSGUIRECT drect;

     if (progress<0.)
         progress=0.;
     if (progress>1.)
         progress=1.;
     willusgui_window_get_useable_rect(&k2gui_cbox->mainwin,&rect);
     w=(rect.right-rect.left)*.95;
     h=k2gui_cbox->bf.size*1.4;
     x1=rect.left+(rect.right-rect.left-w)/2;
     y1=k2gui_cbox->control[1].rect.top - h*.05 - h*1.15*(3-index);
     drect.left=x1+1;
     drect.right=x1+w*progress+1;
     drect.top=y1;
     drect.bottom=y1+h;
     if (drect.right>drect.left)
         willusgui_window_draw_rect_filled(&k2gui_cbox->mainwin,&drect,color);
     drect.left=drect.right+1;
     drect.right=x1+w;
     if (drect.right>drect.left)
         willusgui_window_draw_rect_filled(&k2gui_cbox->mainwin,&drect,0xffffff);
     drect.left=x1;
     drect.right=x1+w+1;
     drect.bottom++;
     willusgui_window_draw_rect_outline(&k2gui_cbox->mainwin,&drect,0);
     willusgui_window_text_render(&k2gui_cbox->mainwin,&k2gui_cbox->bf,label,
                                    (int)(x1+w/2),(int)(y1+h*.2),0,-1,1,NULL);
     }
     

static void k2gui_cbox_message_box_add_children(char *buttonlabel[],int buttons_only)

    {
    int i,width,w,nb,dx,dx1,x1,y1;
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
                         0,0,0,0,k2gui_cbox->hwnd,(HMENU)0,k2gui_cbox->hinstance,NULL);
    bclass = GetClassLong(dummy,GCL_STYLE);
    SetClassLong(dummy,GCL_STYLE,bclass & ~CS_DBLCLKS);
    DestroyWindow(dummy);
    }
*/
    willusgui_window_get_useable_rect(&k2gui_cbox->mainwin,&rect);
    width=rect.right-rect.left;
    y1=rect.bottom - (int)k2gui_cbox->bf.size*1.5;
    for (nb=w=0,i=1;i<=3;i++)
        {
        if (buttonlabel[i-1][0]=='\0')
            continue;
        willusgui_window_text_extents(&k2gui_cbox->mainwin,&k2gui_cbox->bf,buttonlabel[i-1],&brect[i-1]);
        brect[i-1].right += 4*k2gui_cbox->bf.size;
        w += brect[i-1].right;
        nb++;
        }
    dx=(width-6-w)/(nb+1);
    dx1=k2gui_cbox->bf.size;
    if (dx > dx1)
        dx=dx1;
    if (nb==0)
        x1=rect.left+width/2;
    else
        x1=rect.left+(width-6-w-dx*(nb-1))/2;

    /* Edit box */
    if (!buttons_only)
        {
        control=&k2gui_cbox->control[0];
        willusgui_control_init(control);
        control->attrib = WILLUSGUICONTROL_ATTRIB_READONLY | WILLUSGUICONTROL_ATTRIB_MULTILINE 
                             | WILLUSGUICONTROL_ATTRIB_SCROLLBARS;
        control->rect.left = rect.left + width*0.02;
        control->rect.right = rect.left + width*0.95;
        control->rect.top = rect.top + k2gui_cbox->mf.size/4;
        control->rect.bottom = (y1 - k2gui_cbox->bf.size*3.5);
        control->font.size = k2gui_cbox->mf.size;
        willusgui_font_get(&control->font);
        control->type=WILLUSGUICONTROL_TYPE_EDITBOX;
        control->index=10;
        strcpy(control->name,"k2pdfopt output");
        control->parent=&k2gui_cbox->mainwin;
        willusgui_control_create(control);
        }
    k2gui_cbox->ncontrols=1;

    /* Button */
    if (buttons_only)
        k2gui_cbox_draw_defbutton_border(0);
    for (i=1;i<=3;i++)
        {
        control=&k2gui_cbox->control[i];
        if (control->handle!=NULL)
            willusgui_control_close(control);
        willusgui_control_init(control);
        strcpy(control->name,buttonlabel[i-1]);
        if (control->name[0]=='\0')
            continue;
        control->color=0xf0f0f0;
        /* defbutton=(control->text[0]=='*'); */
        control->attrib = 0;
        control->rect.left = x1;
        control->rect.right = x1+brect[i-1].right;
        control->rect.top = y1;
        control->rect.bottom = y1+k2gui_cbox->bf.size*1.2;
        control->font.size = k2gui_cbox->bf.size;
        willusgui_font_get(&control->font);
        control->type=WILLUSGUICONTROL_TYPE_BUTTON;
        control->index=2+i;
        control->parent=&k2gui_cbox->mainwin;
        willusgui_control_create(control);
        k2gui_cbox->ncontrols++;
        x1 += (control->rect.right-control->rect.left)+dx;
        }
    k2gui_cbox_draw_defbutton_border(1);
/*
dprintf(NULL,"msg=%s, k2gui_cbox->inbuf=%p, right-left=%d\n",
k2gui_cbox->msg,k2gui_cbox->inbuf,k2gui_cbox->aboutbox.right-k2gui_cbox->aboutbox.left);
*/
    }


void k2gui_cbox_draw_defbutton_border(int status)

    {
    WILLUSGUIRECT rect;
    int i,color;

    rect=k2gui_cbox->control[1].rect;
    color = status ? 0 : k2gui_cbox->rgbcolor;
    for (i=0;i<4;i++)
        {
        rect.top--;
        rect.bottom++;
        rect.left--;
        rect.right++;
        if (i>1)
            willusgui_window_draw_rect_outline(&k2gui_cbox->mainwin,&rect,color);
        }
    }


void k2gui_cbox_destroy(void)

    {
    int i;

    if (!k2gui_cbox->converting)
        return;
    for (i=k2gui_cbox->ncontrols-1;i>=0;i--)
        willusgui_control_close(&k2gui_cbox->control[i]);
    k2gui_cbox->ncontrols=0;
    willusgui_font_release(&k2gui_cbox->mf);
    willusgui_font_release(&k2gui_cbox->bf);
    willusgui_control_close(&k2gui_cbox->mainwin);
    /* UnregisterClass(k2gui_cbox->class,k2gui_cbox->hinstance); */
    k2gui_cbox->converting=0;
    }


int k2gui_cbox_conversion_successful(void)

    {
    return(k2gui_cbox->successful);
    }
#endif /* HAVE_K2GUI */
