/*
** willusgui.c     Graphical user interface functions meant to conceivably
**              be extended to multi-platform compatible calls.
**
** Part of willus.com general purpose C code library.
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

#include "willus.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <time.h>

#if (!defined(MSWINGUI) && defined(HAVE_WIN32_API))
#define MSWINGUI
#endif

#ifdef MSWINGUI
#include <conio.h>
#include <windows.h>
#include <shlobj.h>

LRESULT CALLBACK willusgui_sbitmap_proc_internal(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam);
LRESULT CALLBACK willusgui_edit2_proc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);
LRESULT CALLBACK willusgui_edit3_proc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);
#endif

typedef struct
    {
    void *oshandle;
    WILLUSGUICONTROL *control;
    } WINHANDLEPAIR;

typedef struct
    {
    int n,na;
    int sorted;
    WINHANDLEPAIR *pair;
    } WINHANDLEPAIRS;

static WINHANDLEPAIRS whpairs;

static WILLUSGUICONTROL *icontrol=NULL;
static void *willusgui_global_instance=NULL;

/*
** Functions to turn Windows handles into WILLUSGUICONTROL handles
*/
static void winhandlepairs_init(WINHANDLEPAIRS *pairs);
static void winhandlepairs_free(WINHANDLEPAIRS *pairs);
/* static void winhandlepairs_clear(WINHANDLEPAIRS *pairs); */
static void winhandlepairs_add_control(WINHANDLEPAIRS *pairs,WILLUSGUICONTROL *control);
static void winhandlepairs_remove_control(WINHANDLEPAIRS *pairs,WILLUSGUICONTROL *control);
static int  winhandlepairs_find_control_index(WINHANDLEPAIRS *pairs,void *oshandle);
static void winhandlepairs_sort(WINHANDLEPAIRS *pairs);


static int ime_notify_status=0;
void willusgui_set_ime_notify(int status)

    {
    if (status==0)
        ime_notify_status=status;
    else
        ime_notify_status++;
    }


int willusgui_dprintf(char *fmt,...)

    {
    static int count=0;
    va_list args;
    int     status;
    static int tids[16];
    static int ntids=0;
    static char *colors[]={ANSI_CYAN,ANSI_MAGENTA,ANSI_YELLOW,ANSI_GREEN};
    int i,tid;
    static void *willusgui_dprintf_sem;

    if (count==0)
        {
        willusgui_dprintf_sem = willusgui_semaphore_create_ex("willusgui_dprintf",1,1);
        if (willusgui_dprintf_sem==NULL)
            printf("\a\nOuch!  Cannot create willusgui_dprintf semaphore.\n\n");
        }
    count++;
    willusgui_semaphore_status_wait(willusgui_dprintf_sem);
#ifdef HAVE_WIN32_API
    tid=(int)GetCurrentThreadId();
    for (i=0;i<ntids;i++)
        if (tids[i]==tid)
            break;
    if (i>=ntids && i<16)
        {
        tids[i]=tid;
        ntids++;
        }
#else
    tid=1;
    i=0;
#endif
    aprintf("%s",colors[i%4]);
    aprintf("[THREAD=%d] ",tid);
    va_start(args,fmt);
    status=avprintf(stdout,fmt,args);
    va_end(args);
    aprintf(ANSI_NORMAL);
    willusgui_semaphore_release(willusgui_dprintf_sem);
    return(status);
    }


void willusgui_init(void)

    {
    winhandlepairs_init(&whpairs);
    willusgui_global_instance=NULL;
    icontrol=NULL;
    willusgui_set_ime_notify(0);
    }


void willusgui_close(void)

    {
    winhandlepairs_free(&whpairs);
    }


/*
** 0 = normal
** 1 = wait
**
**      2------3------4
**      |             |
**      |             |
**      |             |
**      5      6      7
**      |             |
**      |             |
**      |             |
**      8------9------10
*/
void willusgui_set_cursor(int type)

    {
#ifdef MSWINGUI
    switch(type)
        {
        case 1:
            SetCursor(LoadCursor(NULL,IDC_WAIT));
            break;
        case 2:
        case 10:
            SetCursor(LoadCursor(NULL,IDC_SIZENWSE));
            break;
        case 3:
        case 9:
            SetCursor(LoadCursor(NULL,IDC_SIZENS));
            break;
        case 4:
        case 8:
            SetCursor(LoadCursor(NULL,IDC_SIZENESW));
            break;
        case 5:
        case 7:
            SetCursor(LoadCursor(NULL,IDC_SIZEWE));
            break;
        case 6:
            SetCursor(LoadCursor(NULL,IDC_SIZEALL));
            break;
        default:
            SetCursor(LoadCursor(NULL,IDC_ARROW));
        }
#endif
    }


/*
** If the file is opened by another process, this function may fail.
** In that case, try using willusgui_open_file_ex() in winshellwapi.c.
*/
int willusgui_open_file(char *filename)

    {
#ifdef MSWINGUI
    char cmd[64];
    char cmdopts[512];
    char pwd[512];
    int procnum;

    if (strnicmp(filename,"http://",7) && strnicmp(filename,"https://",8) 
            && wfile_status(filename)==0)
        {
        char *message;
        int len;
        static char *funcname="willusgui_open_file";
        static int bcolors[3]={0x6060b0,0xf0f0f0,0xf0f0f0};

        len=strlen(filename);
        willus_mem_alloc_warn((void **)&message,len+80,funcname,10);
        sprintf(message,"Cannot open %s!",filename);
        willusgui_message_box(NULL,"Error",message,"*&OK",NULL,NULL,
                           NULL,0,24,640,0xe0e0e0,bcolors,NULL,1);
        willus_mem_free((double **)&message,funcname);
        return(0);
        }
    sprintf(cmd,"cmd");
    sprintf(cmdopts,"/c start \"\" \"%s\"",filename);
    strncpy(pwd,wfile_get_wd(),511);
    pwd[511]='\0';
    /* Make sure PWD isn't UNC -- if it is, default C:\ */
    if (pwd[0]=='\\' && pwd[1]=='\\')
        strcpy(pwd,"C:\\");
    process_launch_ex(cmd,cmdopts,1,1,pwd,6,&procnum);
    if (procnum>=0)
        {
        int status,exitcode;
        while ((status=process_done_ex(procnum,&exitcode))==0)
            win_sleep(10);
        if (exitcode==0)
            return(1);
        }
    return(-1);
    /*
    sprintf(cmd,"start \"\" \"%s\"",filename);
    system(cmd);
    */
#endif
    }


/*
** x0 = pixels from left side of client area
** y0 = pixels from top of client area
**
** bgcolor = -1 for transparent
**
** justification =    0     1      2
**                    3     4      5
**                    6     7      8
**
** If rect!=NULL, the text is not drawn and the extents are calculated instead
** and stored in rect.
*/
void willusgui_window_text_render(WILLUSGUIWINDOW *win,WILLUSGUIFONT *font,char *text,int x0,int y0,
                                   int fgcolor,int bgcolor,int justification,WILLUSGUIRECT *rect)

    {
#ifdef MSWINGUI
    int alignment,vc,xo,yo;
    HDC hdc;

    alignment=0;
    xo=(justification%3);
    yo=(justification/3)%3;
    if (xo == 0)
        alignment |= TA_LEFT;
    else if (xo == 1)
        alignment |= TA_CENTER;
    else
        alignment |= TA_RIGHT;
    if (yo == 0)
        alignment |= TA_TOP;
    else if (yo == 1)
        alignment |= TA_BOTTOM;
    else
        alignment |= TA_BOTTOM;
    vc = (yo==1);
    hdc=GetDC((HWND)win->handle);
    SelectObject(hdc,font->handle);
    if (rect==NULL)
        {
        if (bgcolor<0)
            SetBkMode(hdc,TRANSPARENT);
        SetTextAlign(hdc,alignment);
        SetTextColor(hdc,fgcolor);
        win_textout_utf8((void *)hdc,x0,y0 + (vc ? font->size/3:0),text);
        }
    else
        {
        long w,h;
        win_gettextextentpoint_utf8((void *)hdc,text,&w,&h);
        rect->left = x0-xo*w/2;
        rect->right = rect->left + w;
        rect->top  = y0-yo*h/2;
        rect->bottom = rect->top + h;
        }
    ReleaseDC((HWND)win->handle,hdc);
#endif
    }


/*
** rect.right, rect.bottom get width, height of text, respectively.
*/
void willusgui_window_text_extents(WILLUSGUIWINDOW *win,WILLUSGUIFONT *font,char *string,WILLUSGUIRECT *rect)

    {
#ifdef MSWINGUI
    HDC hdc;
    SIZE sz;
    hdc=GetDC((HWND)win->handle);
    SelectObject(hdc,font->handle);
    win_gettextextentpoint_utf8((void *)hdc,string,&sz.cx,&sz.cy);
    ReleaseDC((HWND)win->handle,hdc);
    rect->left=0;
    rect->right=sz.cx;
    rect->top=0;
    rect->bottom=sz.cy;
#endif
    }


void willusgui_window_draw_line(WILLUSGUIWINDOW *win,int x0,int y0,int x1,int y1,
                                                 int pixwidth,int rgbcolor)

    {
#ifdef MSWINGUI
    HDC hdc;
    HPEN pen;

    hdc=GetDC((HWND)win->handle);
    pen=CreatePen(PS_SOLID,pixwidth,rgbcolor);
    SelectObject(hdc,pen);
    MoveToEx(hdc,x0,y0,NULL);
    LineTo(hdc,x1,y1);
    DeleteObject(pen);
    ReleaseDC((HWND)win->handle,hdc);
#endif
    }


void willusgui_window_draw_rect_filled(WILLUSGUIWINDOW *win,WILLUSGUIRECT *rect,int rgb)

    {
#ifdef MSWINGUI
    HDC hdc;
    RECT wrect;
    HBRUSH brush;

    wrect.top=rect->top;
    wrect.bottom=rect->bottom;
    wrect.left=rect->left;
    wrect.right=rect->right;
    brush = CreateSolidBrush(((rgb&0xff0000)>>16)|(rgb&0xff00)|((rgb&0xff)<<16));
    hdc=GetDC((HWND)win->handle);
    FillRect(hdc,&wrect,brush);
    ReleaseDC((HWND)win->handle,hdc);
    DeleteObject(brush);
#endif
    }


void willusgui_window_draw_path_filled(WILLUSGUIWINDOW *win,int *x,int *y,int n,int rgb)

    {
#ifdef MSWINGUI
    HDC hdc;
    HBRUSH brush;
    POINT *p;
    int i;
    static char *funcname="willusgui_window_draw_path_filled";

    willus_mem_alloc_warn((void **)&p,sizeof(POINT)*(n-1),funcname,10);
    for (i=0;i<n-1;i++)
        {
        p[i].x=x[i+1];
        p[i].y=y[i+1];
        }
    brush = CreateSolidBrush(((rgb&0xff0000)>>16)|(rgb&0xff00)|((rgb&0xff)<<16));
    hdc=GetDC((HWND)win->handle);
    BeginPath(hdc);
    MoveToEx(hdc,x[0],y[0],NULL);
    PolylineTo(hdc,p,n-1);
//    CloseFigure(hdc);
    EndPath(hdc);
    SelectObject(hdc,brush);
    SetPolyFillMode(hdc,ALTERNATE);
    FillPath(hdc);
    ReleaseDC((HWND)win->handle,hdc);
    DeleteObject(brush);
    willus_mem_free((double **)&p,funcname);
#endif
    }


void willusguirect_bound(WILLUSGUIRECT *rect,WILLUSGUIRECT *brect)

    {
    WILLUSGUIRECT r1;

    r1=(*brect);
    willusguirect_sort(&r1);
    willusguirect_sort(rect);
    if (rect->left < r1.left)
        rect->left = r1.left;
    if (rect->left > r1.right)
        rect->left = r1.right;
    if (rect->right < r1.left)
        rect->right = r1.left;
    if (rect->right > r1.right)
        rect->right = r1.right;
    if (rect->top < r1.top)
        rect->top = r1.top;
    if (rect->top > r1.bottom)
        rect->top = r1.bottom;
    if (rect->bottom < r1.top)
        rect->bottom = r1.top;
    if (rect->bottom > r1.bottom)
        rect->bottom = r1.bottom;
    willusguirect_sort(rect);
    }


void willusguirect_sort(WILLUSGUIRECT *rect)

    {
    if (rect->left > rect->right)
        {
        int t;
        t=rect->left;
        rect->left=rect->right;
        rect->right=t;
        }
    if (rect->top > rect->bottom)
        {
        int t;
        t=rect->top;
        rect->top=rect->bottom;
        rect->bottom=t;
        }
    }


/*
** Number of displayable lines in an edit control
*/
int willusgui_control_nlines(WILLUSGUICONTROL *control)

    {
    if (control->font.size<=0)
        return(-1);
    return((control->rect.bottom-control->rect.top+control->font.size-1)/control->font.size);
    }


void willusgui_window_draw_crosshair(WILLUSGUIWINDOW *win,int x,int y,int rgb)

    {
#ifdef MSWINGUI
    HDC hdc;
    HPEN hpen,oldpen;

    hdc=GetDC((HWND)win->handle);
    hpen=rgb<0 ? CreatePen(PS_SOLID,1,RGB(255,255,255)) 
               : CreatePen(PS_SOLID,1,RGB(rgb&0xff0000>>16,rgb&0xff00>>8,rgb&0xff));
    oldpen=SelectObject(hdc,hpen);
    if (rgb<0)
        SetROP2(hdc,R2_XORPEN);
    MoveToEx(hdc,0,y,NULL);
    LineTo(hdc,4000,y);
    MoveToEx(hdc,x,0,NULL);
    LineTo(hdc,x,4000);
    SelectObject(hdc,oldpen);
    DeleteObject(hpen);
    ReleaseDC((HWND)win->handle,hdc);
#endif
    }


/*
** -1 = outside win
** 0 = normal mouse ptr
** 
**      1------2------3
**      |             |
**      |             |
**      |             |
**      4      5      6
**      |             |
**      |             |
**      |             |
**      7------8------9
**
*/
int willusguirect_cursor_type(WILLUSGUIRECT *rect,WILLUSGUIRECT *winrect,int x,int y)

    {
    int dx,ix,iy;

    dx=7;
    if (x<0 || y<0 || x>winrect->right || y>winrect->bottom)
        return(-1);
    if (rect->left<-9000)
        return(0);
    if (x<rect->left-dx || x>rect->right+dx)
        return(0);
    if (y<rect->top-dx || y>rect->bottom+dx)
        return(0);
    if (abs(x-rect->left)<=dx)
        ix=0;
    else if (abs(x-rect->right)<=dx)
        ix=2;
    else
        ix=1;
    if (abs(y-rect->top)<=dx)
        iy=0;
    else if (abs(y-rect->bottom)<=dx)
        iy=2;
    else
        iy=1;
    return(1+ix+iy*3);
    }


void willusgui_window_draw_rect_outline(WILLUSGUIWINDOW *win,WILLUSGUIRECT *rect,int rgb)

    {
#ifdef MSWINGUI
    HDC hdc;
    RECT wrect;
    HBRUSH brush;

    hdc=GetDC((HWND)win->handle);
    if (rgb<0)
        {
        HPEN hpen,oldpen;
        hpen=CreatePen(PS_SOLID,1,RGB(255,255,255));
        oldpen=SelectObject(hdc,hpen);
        SetROP2(hdc,R2_XORPEN);
        MoveToEx(hdc,rect->left,rect->top,NULL);
        LineTo(hdc,rect->right,rect->top);
        LineTo(hdc,rect->right,rect->bottom);
        LineTo(hdc,rect->left,rect->bottom);
        LineTo(hdc,rect->left,rect->top);
        SelectObject(hdc,oldpen);
        DeleteObject(hpen);
        }
    else
        {
        wrect.top=rect->top;
        wrect.bottom=rect->bottom;
        wrect.left=rect->left;
        wrect.right=rect->right;
        brush = CreateSolidBrush(((rgb&0xff0000)>>16)|(rgb&0xff00)|((rgb&0xff)<<16));
        FrameRect(hdc,&wrect,brush);
        DeleteObject(brush);
        }
    ReleaseDC((HWND)win->handle,hdc);
#endif
    }
    

void willusgui_set_instance(void *instanceptr)

    {
    willusgui_global_instance = instanceptr;
    }


void *willusgui_instance(void)

    {
    return(willusgui_global_instance);
    }


#ifdef MSWINGUI
static void *defproc_edit2;
static void *defproc_edit3;
#endif

void willusgui_window_register(WILLUSGUIWINDOW *window)

    {
    winhandlepairs_add_control(&whpairs,window);
    }


void willusgui_window_deregister(WILLUSGUIWINDOW *window)

    {
    winhandlepairs_remove_control(&whpairs,window);
    }


/*
** Create a list box.
*/
void willusgui_control_create(WILLUSGUICONTROL *control)

    {
#ifdef MSWINGUI
    int flags;
    static int new_classes_already_setup=0;
    static char *eclass2="Edit2";
    static char *eclass3="Edit3";
    static char *funcname="willusgui_control_create";

    if (!new_classes_already_setup)
        {
        WNDCLASS wcl;

        GetClassInfo((HINSTANCE)willusgui_global_instance,"Edit",&wcl);
        wcl.lpszClassName=eclass2;
        defproc_edit2=wcl.lpfnWndProc;
        wcl.lpfnWndProc = willusgui_edit2_proc;
        RegisterClass(&wcl);
        GetClassInfo((HINSTANCE)willusgui_global_instance,"Edit",&wcl);
        wcl.lpszClassName=eclass3;
        defproc_edit3=wcl.lpfnWndProc;
        wcl.lpfnWndProc = willusgui_edit3_proc;
        RegisterClass(&wcl);
        new_classes_already_setup=1;
        }

    switch (control->type)
        {
        case WILLUSGUICONTROL_TYPE_LISTBOX:
            flags = WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_STANDARD
                             | LBS_NOTIFY | WS_OVERLAPPED;
            if (control->attrib & WILLUSGUICONTROL_ATTRIB_MULTISELECT)
                flags |= (LBS_MULTIPLESEL | LBS_EXTENDEDSEL);
            if (!(control->attrib & WILLUSGUICONTROL_ATTRIB_NOKEYS))
                flags |= WS_TABSTOP | LBS_USETABSTOPS | LBS_WANTKEYBOARDINPUT;
            flags &= (~LBS_SORT);
            {
            short *wname;
            utf8_to_utf16_alloc((void **)&wname,control->name);
            control->handle = CreateWindowW(L"listbox",(LPWSTR)wname,flags,
                                   control->rect.left,control->rect.top,
                                   control->rect.right-control->rect.left+1,
                                   control->rect.bottom-control->rect.top+1,
                                   control->parent->handle,(HMENU)(size_t)(control->index),
                                   (HINSTANCE)willusgui_global_instance,NULL);
            willus_mem_free((double **)&wname,funcname);
            }
            SendMessage(control->handle,WM_SETFONT,(WPARAM)control->font.handle,1);
            break;
        case WILLUSGUICONTROL_TYPE_DROPDOWNLIST:
            flags = WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_DROPDOWNLIST;
            if (!(control->attrib & WILLUSGUICONTROL_ATTRIB_NOKEYS))
                flags |= WS_TABSTOP;
            flags &= (~CBS_SORT);
            {
            short *wname;
            utf8_to_utf16_alloc((void **)&wname,control->name);
            control->handle = CreateWindowW(L"combobox",(LPWSTR)wname,flags,
                                   control->rect.left,control->rect.top,
                                   control->rect.right-control->rect.left+1,
                                   control->rect.bottom-control->rect.top+1,
                                   control->parent->handle,(HMENU)(size_t)(control->index),
                                   (HINSTANCE)willusgui_global_instance,NULL);
            willus_mem_free((double **)&wname,funcname);
            }
            SendMessage(control->handle,WM_SETFONT,(WPARAM)control->font.handle,1);
            break;
        case WILLUSGUICONTROL_TYPE_BUTTON:
        case WILLUSGUICONTROL_TYPE_CHECKBOX:
            {
            /*
            int checkbox;

            checkbox = (control->type == WILLUSGUICONTROL_TYPE_CHECKBOX);
            */
            flags = WS_CHILD | WS_VISIBLE | BS_OWNERDRAW;
            if (!(control->attrib & WILLUSGUICONTROL_ATTRIB_NOKEYS))
                flags |= WS_TABSTOP;
            control->handle = CreateWindow("button",control->label,flags,
                                   control->rect.left,control->rect.top,
                                   control->rect.right-control->rect.left+1,
                                   control->rect.bottom-control->rect.top+1,
                                   control->parent->handle,(HMENU)(size_t)(control->index),
                                   (HINSTANCE)willusgui_global_instance,NULL);
            SendMessage(control->handle,WM_SETFONT,(WPARAM)control->font.handle,1);
            break;
            }
        /*
        case WILLUSGUICONTROL_TYPE_PREVIEW:
            flags = WS_CHILD | WS_VISIBLE;
            control->handle = CreateWindow("static",NULL,flags,
                                   control->rect.left,control->rect.top,
                                   control->rect.right-control->rect.left+1,
                                   control->rect.bottom-control->rect.top+1,
                                   control->parent->handle,(HMENU)(size_t)(control->index),
                                   (HINSTANCE)willusgui_global_instance,NULL);
            SendMessage(control->handle,WM_SETFONT,(WPARAM)control->font.handle,1);
            break;
        */
        /* Text box with up/down buttons for adjustment */
        case WILLUSGUICONTROL_TYPE_UPDOWN:
            {
            int i;
            int h;

            h=control->rect.bottom-control->rect.top+1;
            flags = WS_CHILD|WS_VISIBLE|WS_BORDER|ES_AUTOHSCROLL|ES_RIGHT;
            if (!(control->attrib & WILLUSGUICONTROL_ATTRIB_NOKEYS))
                flags |= WS_TABSTOP;
            if (control->attrib & WILLUSGUICONTROL_ATTRIB_READONLY)
                flags |= ES_READONLY;
            if (control->attrib & WILLUSGUICONTROL_ATTRIB_SCROLLBARS)
                flags |= (WS_HSCROLL | WS_VSCROLL);
/*
printf("x1=%d\n",control->rect.left);
printf("right=%d\n",control->rect.right);
printf("h=%d\n",h);
printf("width=%d\n",control->rect.right-control->rect.left-h-1);
*/
            control->handle = CreateWindow(eclass3,
                                   control->name,flags,
                                   control->rect.left,control->rect.top,
                                   control->rect.right-control->rect.left+1-h*2/3,
                                   h,
                                   control->parent->handle,(HMENU)(size_t)(control->index),
                                   (HINSTANCE)willusgui_global_instance,NULL);
            SendMessage(control->handle,WM_SETFONT,(WPARAM)control->font.handle,1);
            /* Up and down buttons */
            for (i=0;i<2;i++)
                {
                int ww;
                /* No tab stop on up/down buttons */
                flags = WS_CHILD | WS_VISIBLE | BS_OWNERDRAW;
                ww=h*2/3;
                if (!(ww&1))
                   ww++;
                control->subhandle[i] = CreateWindow("button",i==0?"_up_":"_down_",flags,
                                   control->rect.right-h*2/3,control->rect.top+i*h/2,ww,h/2,
                                   control->parent->handle,
                                   (HMENU)(size_t)(control->index+100*(i+1)),
                                   (HINSTANCE)willusgui_global_instance,NULL);
                SendMessage(control->subhandle[i],WM_SETFONT,(WPARAM)control->font.handle,1);
                }
            break;
            }
        /* Text box with up/down and double-up/down buttons for adjustment */
        case WILLUSGUICONTROL_TYPE_UPDOWN2:
            {
            int i,h,w,w1,w2,g;

            h=control->rect.bottom-control->rect.top+1;
            w=control->rect.right-control->rect.left+1;
            w1=h*2/3;
            w2=h;
            g=2;
            flags = WS_CHILD|WS_VISIBLE|WS_BORDER|ES_AUTOHSCROLL|ES_RIGHT;
            if (!(control->attrib & WILLUSGUICONTROL_ATTRIB_NOKEYS))
                flags |= WS_TABSTOP;
            if (control->attrib & WILLUSGUICONTROL_ATTRIB_READONLY)
                flags |= ES_READONLY;
            if (control->attrib & WILLUSGUICONTROL_ATTRIB_SCROLLBARS)
                flags |= (WS_HSCROLL | WS_VSCROLL);
/*
printf("x1=%d\n",control->rect.left);
printf("right=%d\n",control->rect.right);
printf("h=%d\n",h);
printf("width=%d\n",control->rect.right-control->rect.left-h-1);
*/
            control->handle = CreateWindow(eclass3,
                                   control->name,flags,
                                   control->rect.left+w1+w2+2*g,control->rect.top,
                                   w-2*w1-2*w2-g*5,h,
                                   control->parent->handle,(HMENU)(size_t)(control->index),
                                   (HINSTANCE)willusgui_global_instance,NULL);
            SendMessage(control->handle,WM_SETFONT,(WPARAM)control->font.handle,1);
            /* Up and down buttons */
            for (i=0;i<4;i++)
                {
                int x1,ww;
                static char *text[]={"_dleft_","_left_","_right_","_dright_"};

                flags = WS_CHILD | WS_VISIBLE | BS_OWNERDRAW;
                if (i==1 || i==2)
                    ww=w1;
                else
                    ww=w2;
                if (i==0)
                    x1=control->rect.left;
                else if (i==1)
                    x1=control->rect.left+w2+g;
                else if (i==2)
                    x1=control->rect.right-1-w2-g-w1;
                else
                    x1=control->rect.right-1-w2;
                control->subhandle[i] = CreateWindow("button",text[i],flags,
                                   x1,control->rect.top+h/10,ww,4*h/5,
                                   control->parent->handle,
                                   (HMENU)(size_t)(control->index+100*(i+1)),
                                   (HINSTANCE)willusgui_global_instance,NULL);
                SendMessage(control->subhandle[i],WM_SETFONT,(WPARAM)control->font.handle,1);
                }
            break;
            }
        case WILLUSGUICONTROL_TYPE_EDITBOX:
            {
            flags = WS_CHILD|WS_VISIBLE|WS_BORDER|ES_AUTOHSCROLL;
            if (!(control->attrib & WILLUSGUICONTROL_ATTRIB_NOKEYS))
                flags |= WS_TABSTOP;
            if (control->attrib & WILLUSGUICONTROL_ATTRIB_READONLY)
                flags |= ES_READONLY;
            if (control->attrib & WILLUSGUICONTROL_ATTRIB_MULTILINE)
                {
                flags |= ES_MULTILINE;
                flags &= ~ES_AUTOHSCROLL;
                }
            if (control->attrib & WILLUSGUICONTROL_ATTRIB_SCROLLBARS)
                flags |= (WS_HSCROLL | WS_VSCROLL);
            control->handle = CreateWindow(flags&ES_MULTILINE ? eclass2 : eclass3,
                                   control->name,flags,
                                   control->rect.left,control->rect.top,
                                   control->rect.right-control->rect.left+1,
                                   control->rect.bottom-control->rect.top+1,
                                   control->parent->handle,(HMENU)(size_t)(control->index),
                                   (HINSTANCE)willusgui_global_instance,NULL);
            SendMessage(control->handle,WM_SETFONT,(WPARAM)control->font.handle,1);
            }
            break;
        case WILLUSGUICONTROL_TYPE_SCROLLABLEBITMAP:
        case WILLUSGUICONTROL_TYPE_BITMAP:
            {
            static int need_new_class=1;
            static char *sbclass="ScrollableBitmap";

            if (need_new_class)
                {
                WNDCLASSEX  wndclass;
                wndclass.cbSize        = sizeof(wndclass);
                wndclass.style         = CS_OWNDC | CS_HREDRAW | CS_VREDRAW | CS_SAVEBITS;
                /*
                wndclass.style         = CS_HREDRAW | CS_VREDRAW;
                */
                wndclass.lpfnWndProc   = willusgui_sbitmap_proc_internal;
                wndclass.cbClsExtra    = 0;
                wndclass.cbWndExtra    = 0;
                wndclass.hInstance     = (HINSTANCE)willusgui_global_instance;
                wndclass.hIcon         = NULL;
                wndclass.hCursor       = LoadCursor(NULL,IDC_ARROW);
                wndclass.hbrBackground = NULL;
                wndclass.lpszMenuName  = NULL;
                wndclass.lpszClassName = sbclass;
                wndclass.hIconSm       = NULL;
                RegisterClassEx(&wndclass);
                need_new_class=0;
                }
            flags = WS_CHILD|WS_VISIBLE|WS_BORDER|WS_OVERLAPPED;
            if (!(control->attrib & WILLUSGUICONTROL_ATTRIB_NOKEYS))
                flags |= WS_TABSTOP;
            willusgui_sbitmap_resample_original(control);
            if (control->bmp.width>control->rect.right-control->rect.left-1)
                flags |= WS_HSCROLL;
            if (control->bmp.height>control->rect.bottom-control->rect.top-1)
                flags |= WS_VSCROLL;
            icontrol=control;
            control->handle = CreateWindow("ScrollableBitmap",
                                   control->name,flags,
                                   control->rect.left,control->rect.top,
                                   control->rect.right-control->rect.left+1,
                                   control->rect.bottom-control->rect.top+1,
                                   control->parent->handle,(HMENU)(size_t)(control->index),
                                   (HINSTANCE)willusgui_global_instance,NULL);
            SendMessage(control->handle,WM_SETFONT,(WPARAM)control->font.handle,1);
            if (control->type==WILLUSGUICONTROL_TYPE_BITMAP)
                control->timer_id=SetTimer(control->handle,0,20,NULL);
            }
            break;
        }
    willusgui_control_draw_label(control,NULL);
    if (control->handle!=NULL)
        willusgui_window_register(control);
    icontrol=NULL;
#endif
    }


void willusgui_control_enable(WILLUSGUICONTROL *control,int enabled)

    {
#ifdef MSWINGUI
    EnableWindow((HWND)control->handle,enabled);
#endif
    }


void willusgui_control_set_text(WILLUSGUICONTROL *control,char *text)

    {
#ifdef MSWINGUI
    if (text==NULL || utf8_is_ascii(text))
        SendMessage((HWND)control->handle,WM_SETTEXT,(WPARAM)0,(LPARAM)(text==NULL?"":text));
    else
        {
        short *sw;
        static char *funcname="willusgui_control_set_text";

        utf8_to_utf16_alloc((void **)&sw,text);
        SendMessageW((HWND)control->handle,WM_SETTEXT,(WPARAM)0,(LPARAM)sw);
        willus_mem_free((double **)&sw,funcname);
        }
#endif
    }


void willusgui_control_get_text(WILLUSGUICONTROL *control,char *text,int maxlen)

    {
#ifdef MSWINGUI
    short *sw;
    static char *funcname="willusgui_control_get_text";

    willus_mem_alloc_warn((void **)&sw,sizeof(short)*(maxlen+1),funcname,10);
    SendMessageW((HWND)control->handle,WM_GETTEXT,(WPARAM)maxlen,(LPARAM)sw);
    utf16_to_utf8(text,sw,maxlen-1);
    willus_mem_free((double **)&sw,funcname);
#else
    text[0]='\0';
#endif
    text[maxlen-1]='\0';
    }


int willusgui_control_get_textlen(WILLUSGUICONTROL *control)

    {
#ifdef MSWINGUI
    return(SendMessage((HWND)control->handle,WM_GETTEXTLENGTH,0,0));
#else
    return(0);
#endif
    }


void willusgui_control_scroll_to_bottom(WILLUSGUICONTROL *control)

    {
#ifdef MSWINGUI
    SendMessage((HWND)control->handle,EM_LINESCROLL,(WPARAM)0,(LPARAM)999999);
#endif
    }

/*
** Returns NZ if succeeds
** If win==NULL, gets desktop rectangle.
*/
int willusgui_window_get_rect(WILLUSGUIWINDOW *win,WILLUSGUIRECT *guirect)

    {
#ifdef MSWINGUI
    RECT rect;
    int status;

    status=GetWindowRect(win==NULL ? GetDesktopWindow() : (HWND)win->handle,&rect);
    guirect->left=rect.left;
    guirect->right=rect.right;
    guirect->bottom=rect.bottom;
    guirect->top=rect.top;
    return(status);
#else
    return(0);
#endif
    }


int willusgui_get_desktop_workarea(WILLUSGUIRECT *guirect)

    {
#ifdef MSWINGUI
    RECT rect;
    int status;

    status=SystemParametersInfo(SPI_GETWORKAREA,0,&rect,0);
    guirect->left=rect.left;
    guirect->right=rect.right;
    guirect->bottom=rect.bottom;
    guirect->top=rect.top;
    return(status);
#else
    return(0);
#endif
    }


int willusgui_window_set_pos(WILLUSGUIWINDOW *win,WILLUSGUIRECT *guirect)

    {
#ifdef MSWINGUI
    HWND hwnd;
    int status;

    hwnd=(HWND)win->handle;
    status=SetWindowPos(hwnd,HWND_TOP,guirect->left,guirect->top,guirect->right-guirect->left+1,
                        guirect->bottom-guirect->top+1,0);
    return(status);
#else
    return(0);
#endif
    }


/*
** Returns NZ if succeeds
*/
int willusgui_window_get_useable_rect(WILLUSGUIWINDOW *win,WILLUSGUIRECT *guirect)

    {
#ifdef MSWINGUI
    RECT rect;
    HWND hwnd;
    int status;

    hwnd=(HWND)win->handle;
    status=GetClientRect(hwnd,&rect);
    guirect->left=rect.left;
    guirect->right=rect.right;
    guirect->bottom=rect.bottom;
    guirect->top=rect.top;
    return(status);
#else
    return(0);
#endif
    }


void willusgui_window_accept_draggable_files(WILLUSGUIWINDOW *win)

    {
#ifdef MSWINGUI
    DragAcceptFiles((HWND)win->handle,1);
#endif
    }


void willusgui_window_timer_init(WILLUSGUIWINDOW *win,int ms)

   {
#ifdef MSWINGUI
   SetTimer((HWND)win->handle,1,ms,NULL);
#endif
   }


/*
** menus = {"_Heading","Opt1","Opt2",...,"_Heading2","Opt1",...,""};
**
*/
void willusgui_window_menus_init(WILLUSGUIWINDOW *win,char *menus[])

    {
#ifdef MSWINGUI
    int i;
    int mv;
    HMENU mainmenu;

    mainmenu=CreateMenu();
    for (mv=700,i=0;menus[i][0]!='\0';mv+=10)
        {
        int j;

        j=0;
        if (menus[i][0]=='_')
            {
            int mv1;
            HMENU menu;

            menu=CreateMenu();
            for (mv1=mv,j=i+1;menus[j][0]!='\0' && menus[j][0]!='_';j++)
                AppendMenu(menu,MF_STRING,mv1++,menus[j]);
            AppendMenu(mainmenu,MF_STRING|MF_POPUP,(UINT_PTR)menu,&menus[i][1]);
            }
        i=j;
        }
    SetMenu((HWND)win->handle,mainmenu);
#endif
    }


/*
** Put up a message box with up to three buttons and a possible text input field.
**
** If inbuf!=NULL and maxlen > 0, there is a text input field.
**
** Return value:
**    0 = could not open window.
**   -1 = Escape key
**    1 = button 1
**    2 = button 2
**    3 = button 3
**    4 = Enter pressed (if no default button)
**
** On button text, * at beginning denotes default button.  & defnotes shortcut key.
** E.g. *&OK to make "OK" the default button and ALT-O the shortcut for the button.
**
** If rect!=NULL, it is used to size the dialog window.
**
*/
int willusgui_message_box(WILLUSGUIWINDOW *parent,char *title,char *message,char *button1,
                            char *button2,char *button3,char *inbuf,int maxlen,
                            int fontsize_pixels,int maxwidth_pixels,int rgbcolor,
                            int *bcolors,WILLUSGUIRECT *rect,int modal)

    {
#ifdef MSWINGUI
    RECT winrect;

    if (rect!=NULL)
        {
        winrect.left=rect->left;
        winrect.right=rect->right;
        winrect.top=rect->top;
        winrect.bottom=rect->bottom;
        }
/*
printf("@willusgui_message_box...\n");
printf("    parent=%p\n",parent->handle);
printf("    title='%s'\n",title);
printf("    message='%s'\n",message);
printf("    button1='%s'\n",button1);
printf("    button2='%s'\n",button2);
printf("    button3='%s'\n",button3);
printf("    inbuf='%s'\n",inbuf);
printf("    maxlen=%d\n",maxlen);
printf("    fontsize=%d\n",(int)fontsize_pixels);
printf("    maxwidth=%d\n",(int)maxwidth_pixels);
printf("    color=%06X\n",rgbcolor);
printf("    left,right,top,bottom=%d,%d,%d,%d\n",
*/
    return(winmbox_message_box_ex2((HWND)(parent==NULL?GetDesktopWindow():parent->handle),
                               title,message,
                               button1,button2,button3,
                               inbuf,maxlen,fontsize_pixels,maxwidth_pixels,
                               rgbcolor,NULL,NULL,bcolors,(void *)(rect==NULL?NULL:&winrect),
                               modal));
#else
    return(0);
#endif
    }


/*
** Tell current thread to exit its windows message-checking loop and return
** control to Windows.
*/
void willusgui_send_quit_message(void)

    {
#ifdef MSWINGUI
    PostQuitMessage(0);
#endif
    }


void willusgui_control_init(WILLUSGUICONTROL *control)

    {
    willusgui_font_init(&control->font);
    control->handle=NULL;
    control->type=0;
    control->name[0]='\0';
    control->color=0;
    control->flags=0;
    control->attrib=0;
    control->label[0]='\0';
    bmp_init(&control->bmp);
    control->bmp.width=control->bmp.height=0;
    control->bmp.bpp=24;
    control->sbitmap_size=0;
    control->rectmarked.left = -10000;
    control->rectmarked.right = -10000;
    control->rectanchor.left = -10000;
    control->anchor.left = -10000;
    control->crosshair.left = -10000;
    control->rdcount=0;
    control->timer_id=0;
    control->dpi=0.;
    control->dpi_rendered=0.;
    }


int  willusgui_control_close(WILLUSGUICONTROL *control)

    {
    if (control==NULL || control->handle==NULL)
        return(1);
#ifdef MSWINGUI
    /*
    ** In MS Windows, it appears that a DestroyWindow command fails if called
    ** directly from a secondary thread--it will deny access.  So you have to send
    ** a message to the window instead.  This only has to be done on the
    ** scrollable bitmap since it's the only one that's close from a secondary
    ** thread (it also has its own class and callback function--not sure if
    ** that matters).
    */
    if (control->timer_id!=0)
        KillTimer((HWND)control->handle,control->timer_id);
    if (control->type == WILLUSGUICONTROL_TYPE_SCROLLABLEBITMAP
          || control->type == WILLUSGUICONTROL_TYPE_BITMAP)
        SendMessage((HWND)control->handle,WM_CLOSE,0,0);
    else
        {
        int status;

        status=willusgui_control_close_ex(control,0);
        if (!status)
            SendMessage((HWND)control->handle,WM_CLOSE,0,0);
        }
#endif
    return(1);
    }


/*
** Returns:
**     0 fail
**     1 succeed
**     2 control is NULL
**     3 not implemented
*/
int willusgui_control_close_ex(WILLUSGUICONTROL *control,int caller)

    {
    int status;

    if (control==NULL || control->handle==NULL)
        return(2);
    bmp_free(&control->bmp);
    /* willusgui_control_bitmap_free(control); */
    willusgui_font_release(&control->font);
#ifdef MSWINGUI
    status=DestroyWindow((HWND)control->handle);
    if (control->type==WILLUSGUICONTROL_TYPE_UPDOWN)
        {
        DestroyWindow((HWND)control->subhandle[1]);
        DestroyWindow((HWND)control->subhandle[0]);
        }
    if (control->type==WILLUSGUICONTROL_TYPE_UPDOWN2)
        {
        DestroyWindow((HWND)control->subhandle[3]);
        DestroyWindow((HWND)control->subhandle[2]);
        DestroyWindow((HWND)control->subhandle[1]);
        DestroyWindow((HWND)control->subhandle[0]);
        }
    if (status==0)
        {
        if (caller!=0)
            aprintf(ANSI_RED "\n\nDestroyWindow fails!\n\nError = '%s'\n\n" ANSI_NORMAL,win_lasterror());
        return(0);
        }
#else
    status=3;
#endif
    if (status!=0)
        {
        willusgui_window_deregister(control);
        control->handle=NULL;
        }
    return(status);
    }


/*
** If rect==NULL, label is drawn, otherwise the extents are returned in rect.
*/
void willusgui_control_draw_label(WILLUSGUICONTROL *control,WILLUSGUIRECT *rect)

    {
    if (control->label[0]=='\0')
        return;
    if (control->type==WILLUSGUICONTROL_TYPE_CHECKBOX || control->type==WILLUSGUICONTROL_TYPE_BUTTON)
        return;
    willusgui_window_text_render(control->parent,&control->font,control->label,
                              control->labelx,control->labely,0,-1,control->labeljust,rect);
    }


/*
** Re-draw a window or control
*/
void willusgui_control_redraw(WILLUSGUICONTROL *control,int children_too)

    {
    willusgui_control_draw_label(control,NULL);
#ifdef MSWINGUI
    int flags;
    flags=RDW_INVALIDATE;
    if (children_too)
        flags |= RDW_ALLCHILDREN;
    RedrawWindow((HWND)control->handle,NULL,NULL,flags);
    if (control->type==WILLUSGUICONTROL_TYPE_UPDOWN)
        {
        RedrawWindow((HWND)control->subhandle[0],NULL,NULL,flags);
        RedrawWindow((HWND)control->subhandle[1],NULL,NULL,flags);
        }
    if (control->type==WILLUSGUICONTROL_TYPE_UPDOWN2)
        {
        RedrawWindow((HWND)control->subhandle[0],NULL,NULL,flags);
        RedrawWindow((HWND)control->subhandle[1],NULL,NULL,flags);
        RedrawWindow((HWND)control->subhandle[2],NULL,NULL,flags);
        RedrawWindow((HWND)control->subhandle[3],NULL,NULL,flags);
        }
#endif
    }


void willusgui_font_release(WILLUSGUIFONT *font)

    {
#ifdef MSWINGUI
    if (font->handle!=NULL)
        DeleteObject(font->handle);
#endif
    font->handle=NULL;
    }


void willusgui_font_get(WILLUSGUIFONT *font)

    {
    if (font->handle!=NULL)
        willusgui_font_release(font);
#ifdef MSWINGUI
    HFONT hf;
    hf=CreateFont(font->size,0,0,0,FW_NORMAL,0,0,0,ANSI_CHARSET,
                      OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
                      DEFAULT_QUALITY,DEFAULT_PITCH|FF_DONTCARE,"Calibri");
    if (hf==NULL)
        hf=CreateFont(font->size,0,0,0,FW_NORMAL,0,0,0,ANSI_CHARSET,
                      OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
                      DEFAULT_QUALITY,DEFAULT_PITCH|FF_DONTCARE,"Arial");
    font->handle=hf;
#endif
    }


/*
** This doesn't work to detect lack of Calibri--it just assigns arial even if Calibri isn't there.
*/
/*
int willusgui_font_is_calibri(void)

    {
    HFONT hf;
    hf=CreateFont(10,0,0,0,FW_NORMAL,0,0,0,ANSI_CHARSET,
                  OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
                  DEFAULT_QUALITY,DEFAULT_PITCH|FF_DONTCARE,"Calibri");
    if (hf==NULL)
        return(0);
    DeleteObject(hf);
    return(1);
    }
*/


void willusgui_font_init(WILLUSGUIFONT *font)

    {
    font->handle=NULL;
    }


void willusgui_start_browser(char *link)

    {
#ifdef MSWINGUI
    willusgui_open_file(link);
#endif
    }


/*
** Return 1 if checkbox is checked, 0 if not.
*/
int willusgui_control_get_checked(WILLUSGUICONTROL *control)

    {
#ifdef MSWINGUI
    return(SendMessage((HWND)control->handle,BM_GETCHECK,0,0)==BST_CHECKED);
#else
    return(0);
#endif
    }


/*
** Set checkmark state of checkbox.  1 to check it.
*/
void willusgui_control_set_checked(WILLUSGUICONTROL *control,int checked)

    {
#ifdef MSWINGUI
    SendMessage((HWND)control->handle,BM_SETCHECK,checked?BST_CHECKED:BST_UNCHECKED,0);
#endif
    }

/*
** Returns number of selected items in a list box.
*/
int willusgui_control_dropdownlist_get_selected_item(WILLUSGUICONTROL *control,char *buf)

    {
#ifdef MSWINGUI
    int index;
#endif

    buf[0]='\0';
    if (control->type != WILLUSGUICONTROL_TYPE_DROPDOWNLIST)
        return(0);
#ifdef MSWINGUI
    index=SendMessage((HWND)control->handle,CB_GETCURSEL,0,0);
    if (index==CB_ERR)
        return(0);
    {
    short *sw;
    static char *funcname="willusgui_control_dropdownlist_get_selected_item";
    int status;

    willus_mem_alloc_warn((void **)&sw,sizeof(short)*1024,funcname,10);
    status=(SendMessageW((HWND)control->handle,CB_GETLBTEXT,index,(LPARAM)sw) != CB_ERR);
    if (status)
        utf16_to_utf8(buf,sw,511);
    else
        buf[0]='\0';
    willus_mem_free((double **)&sw,funcname);
    return(status);
    }
#else
    return(0);
#endif
    }


int willusgui_control_listbox_get_item_count(WILLUSGUICONTROL *control)

    {
    if (control->type != WILLUSGUICONTROL_TYPE_LISTBOX)
        return(-1);
#ifdef MSWINGUI
    return(SendMessage((HWND)control->handle,LB_GETCOUNT,0,(LPARAM)0));
#else
    return(0);
#endif
    }


/*
** Returns number of selected items in a list box.
*/
int willusgui_control_listbox_get_selected_items_count(WILLUSGUICONTROL *control,int *selected_indices,
                                                         int maxsel)

    {
    if (control->type != WILLUSGUICONTROL_TYPE_LISTBOX)
        return(-1);
#ifdef MSWINGUI
    return(SendMessage((HWND)control->handle,LB_GETSELITEMS,maxsel,(LPARAM)selected_indices));
#else
    return(0);
#endif
    }


/*
** Returns index of selected item.  Works for listbox or dropdownlist.
*/
int willusgui_control_listbox_select_item(WILLUSGUICONTROL *control,char *string)

    {
    if (control->type != WILLUSGUICONTROL_TYPE_LISTBOX && control->type != WILLUSGUICONTROL_TYPE_DROPDOWNLIST)
        return(-1);
#ifdef MSWINGUI
    {
    int status;

    if (utf8_is_ascii(string))
        status=SendMessage((HWND)control->handle,control->type==WILLUSGUICONTROL_TYPE_LISTBOX ? LB_SELECTSTRING : CB_SELECTSTRING,-1,(LPARAM)string);
    else
        {
        short *sw;
        static char *funcname="willusgui_control_listbox_select_item";

        utf8_to_utf16_alloc((void **)&sw,string);
        status=SendMessageW((HWND)control->handle,control->type==WILLUSGUICONTROL_TYPE_LISTBOX ? LB_SELECTSTRING : CB_SELECTSTRING,-1,(LPARAM)sw);
        willus_mem_free((double **)&sw,funcname);
        }
    return(status);
    }
#else
    return(-1);
#endif
    }
/*
** Works for listbox or dropdownlist
*/
void willusgui_control_listbox_clear(WILLUSGUICONTROL *control)

    {
    if (control->type != WILLUSGUICONTROL_TYPE_LISTBOX && control->type != WILLUSGUICONTROL_TYPE_DROPDOWNLIST)
        return;
#ifdef MSWINGUI
    SendMessage((HWND)control->handle,control->type==WILLUSGUICONTROL_TYPE_LISTBOX ? LB_RESETCONTENT : CB_RESETCONTENT,0,0);
#endif
    }
    

/*
** Works for listbox or dropdownlist
*/
void willusgui_control_listbox_add_item(WILLUSGUICONTROL *control,char *text)

    {
    if (control->type != WILLUSGUICONTROL_TYPE_LISTBOX && control->type != WILLUSGUICONTROL_TYPE_DROPDOWNLIST)
        return;
#ifdef MSWINGUI
    if (utf8_is_ascii(text))
        SendMessage((HWND)control->handle,control->type==WILLUSGUICONTROL_TYPE_LISTBOX ? LB_ADDSTRING : CB_ADDSTRING,0,(LPARAM)text);
    else
        {
        short *sw;
        static char *funcname="willusgui_control_listbox_add_item";

        utf8_to_utf16_alloc((void **)&sw,text);
        SendMessageW((HWND)control->handle,control->type==WILLUSGUICONTROL_TYPE_LISTBOX ? LB_ADDSTRING : CB_ADDSTRING,0,(LPARAM)sw);
        willus_mem_free((double **)&sw,funcname);
        }
#endif
    }
    

/*
** Gets text from selected item index in a list box.  Returns length of string.
** Returns -1 for error.
*/
int willusgui_control_listbox_get_item_text(WILLUSGUICONTROL *control,int index,char *buf)

    {
    buf[0]='\0';
    if (control->type != WILLUSGUICONTROL_TYPE_LISTBOX)
        return(-1);
#ifdef MSWINGUI
    {
    int status;
    short *sw;
    static char *funcname="willusgui_control_listbox_get_item_text";

    willus_mem_alloc_warn((void **)&sw,sizeof(short)*MAXUTF16PATHLEN,funcname,10);
    buf[0]='\0';
    status=SendMessageW((HWND)control->handle,LB_GETTEXT,index,(LPARAM)sw);
    utf16_to_utf8(buf,sw,511);
    willus_mem_free((double **)&sw,funcname);
    return(status);
    }
#else
    return(-1);
#endif
    }
    

/*
** Return pointer to array of file names dropped onto window.
** For MS-Windows, dropptr is the wParam value passed in the Windows message.
** ptr[0] = first file name
** ptr[1] = second file name
** ...
** ptr[n] = NULL
**
** Use willusgui_release_dropped_files() to release the memory.
*/
char **willusgui_get_dropped_files(void *dropptr)
            
    {
#ifdef MSWINGUI
    char **ptr;
    int i,n;
    static char *funcname="willusgui_get_dropped_files";

    n=DragQueryFileW((HDROP)dropptr,0xffffffff,NULL,0);
    willus_mem_alloc_warn((void **)&ptr,sizeof(char *)*(n+1),funcname,10);
    for (i=0;i<=n;i++)
        ptr[i]=NULL;
    for (i=0;i<n;i++)
        {
        short buf[MAXFILENAMELEN],buf2[MAXFILENAMELEN];
        int u8len;

        DragQueryFileW((HDROP)dropptr,i,(WCHAR *)buf,MAXFILENAMELEN-1);
        while (win_resolve_shortcut(buf,buf2,MAXFILENAMELEN-1,1))
            wide_strcpy(buf,buf2);
        u8len=utf16_to_utf8(NULL,buf,MAXUTF8PATHLEN);
        willus_mem_alloc_warn((void **)&ptr[i],u8len+1,funcname,10);
        if (ptr[i]==NULL)
            return(ptr);
        utf16_to_utf8(ptr[i],buf,u8len);
        }
    return(ptr);
#else
    return(NULL);
#endif
    }


void willusgui_release_dropped_files(char **ptr)

    {
    int i;
    static char *funcname="willusgui_release_dropped_files";

    if (ptr!=NULL)
        {
        for (i=0;ptr[i]!=NULL;i++);
        for (i--;i>=0;i--)
            willus_mem_free((double **)&ptr[i],funcname);
        willus_mem_free((double **)&ptr,funcname);
        }
    }


void willusgui_window_set_focus(WILLUSGUIWINDOW *win)

    {
#ifdef MSWINGUI
    if (win->handle!=NULL)
        SetFocus(win->handle);
/*
{
void *p;
p=(void *)        SetFocus(win->handle);
printf("SetFocus returns %p\n",p);
}
*/
#endif
    }


int willusgui_control_text_selected(WILLUSGUICONTROL *control,int *start,int *end)

    {
#ifdef MSWINGUI
    if (control->handle!=NULL)
        {
        int status;
        (*start)=(*end)=-1;
        status=SendMessage((HWND)control->handle,EM_GETSEL,(WPARAM)start,(LPARAM)end);
        return(status!=0 ? 1 : 0);
        }
#endif
    return(0);
    }


void willusgui_control_text_select(WILLUSGUICONTROL *control,int start,int end)

    {
#ifdef MSWINGUI
    if (control->handle!=NULL)
        {
        PostMessage((HWND)control->handle,EM_SETSEL,start,end);
        }
#endif
    }


void willusgui_control_text_select_all(WILLUSGUICONTROL *control)

    {
#ifdef MSWINGUI
    if (control->handle!=NULL)
        {
        PostMessage((HWND)control->handle,EM_SETSEL,0,-1);
        }
#endif
    }


void *willusgui_control_handle_with_focus(void)

    {
#ifdef MSWINGUI
    return((void *)GetFocus());
#endif
    return(NULL);
    }


/*
** status=0:  Don't allow window to be re-drawn (while it is re-sized, for example)
**       =1:  Allow to be re-drawn
*/
void willusgui_window_set_redraw(WILLUSGUIWINDOW *window,int status)

    {
#ifdef MSWINGUI
    SendMessage((HWND)window->handle,WM_SETREDRAW,status,0);
#endif
    }


/*
** Put up a file selection dialog box.
** Returns 1 if files were selected.
** Return string has null-terminated list of files, with
** double null at end.
** Input:  allowedfiles is a null-terminated list of allowed
**         (description,wildcard) pairs terminated by a double-null,
**         e.g. "PDF files\0*.pdf\0DJVU files\0*.djvu\0\0"
*/
int willusgui_file_select_dialog(char *buf,int maxlen,char *allowedfiles,
                              char *prompt,char *defext,int for_writing)

    {
#ifdef MSWINGUI
    short *filename,*fn;
    int status;
    char *p;
    static char *funcname="willusgui_file_select_dialog";

    willus_mem_alloc_warn((void **)&filename,(maxlen+1)*sizeof(short),funcname,10);
    status=wincomdlg_get_filenamew(filename,maxlen,allowedfiles,prompt,defext,for_writing ? 0 : 1,
                                   for_writing ? 0 : 1,for_writing);
    for (p=buf,fn=filename;1;p=&p[strlen(p)+1])
        {
        utf16_to_utf8(p,fn,maxlen-1);
        for (;(*fn)!=0;fn++);
        fn++;
        if ((*fn)==0)
            break;
        }
    p[strlen(p)+1]='\0';
    willus_mem_free((double **)&filename,funcname);
    return(status);
    /*
    return(wincomdlg_get_filename(buf,maxlen,allowedfiles,prompt,defext,for_writing ? 0 : 1,
                                  for_writing ? 0 : 1,for_writing));
    */
#else
    return(0);
#endif
    }


int willusgui_file_select_dialogw(short *buf,int maxlen,char *allowedfiles,
                               char *prompt,char *defext,int for_writing)

    {
#ifdef MSWINGUI
    return(wincomdlg_get_filenamew(buf,maxlen,allowedfiles,prompt,defext,for_writing ? 0 : 1,
                                   for_writing ? 0 : 1,for_writing));
#else
    return(0);
#endif
    }


/*
** Fill window client area with bitmap
*/
void willusgui_background_bitmap_blit(WILLUSGUIWINDOW *win,WILLUSBITMAP *bmp)

    {
#ifdef MSWINGUI
    bmp_show_bmp_ex(bmp,win->handle,0,0,0);
#endif
    }


void *willusgui_semaphore_create(char *name)

    {
#ifdef MSWINGUI
    return(CreateSemaphore(NULL,0,1,name));
#else
    return(NULL);
#endif
    }


void *willusgui_semaphore_create_ex(char *name,int initialcount,int maxcount)

    {
#ifdef MSWINGUI
    return(CreateSemaphore(NULL,initialcount,maxcount,name));
#else
    return(NULL);
#endif
    }


void willusgui_semaphore_release(void *semaphore)

    {
#ifdef MSWINGUI
    ReleaseSemaphore((HANDLE)semaphore,1,NULL);
#endif
    }


void willusgui_semaphore_close(void *semaphore)

    {
#ifdef MSWINGUI
    CloseHandle((HANDLE)semaphore);
#endif
    }


/*
** 0 = not released
** 1 = released
*/
int willusgui_semaphore_status_wait(void *semaphore)

    {
#ifdef MSWINGUI
    int status;
    status=WaitForSingleObject(semaphore,INFINITE);
    return(status==WAIT_OBJECT_0 ? 1 : 0);
#else
    return(1);
#endif
    }


/*
** 0 = not released
** 1 = released
*/
int willusgui_semaphore_status(void *semaphore)

    {
#ifdef MSWINGUI
    int status;
    status=WaitForSingleObject(semaphore,0);
    return(status==WAIT_OBJECT_0 ? 1 : 0);
#else
    return(1);
#endif
    }


void *willusgui_thread_create(void *funcptr,void *data)

    {
#ifdef MSWINGUI
    return(win_thread_create(funcptr,data));
#else
    return(NULL);
#endif
    }


void willusgui_thread_terminate(void *pid,int exitcode)

    {
#ifdef MSWINGUI
    win_thread_terminate(pid,exitcode);
#endif
    }


void willusgui_thread_exit(int exitcode)

    {
#ifdef MSWINGUI
    win_thread_exit(exitcode);
#endif
    }


static void winhandlepairs_init(WINHANDLEPAIRS *pairs)

    {
    pairs->pair=NULL;
    pairs->sorted=0;
    pairs->n=pairs->na=0;
    }


static void winhandlepairs_free(WINHANDLEPAIRS *pairs)

    {
    static char *funcname="winhandlepairs_free";

    willus_mem_free((double **)&pairs->pair,funcname);
    pairs->n=pairs->na=0;
    pairs->sorted=0;
    }

/*
static void winhandlepairs_clear(WINHANDLEPAIRS *pairs)

    {
    pairs->n=0;
    }
*/


static void winhandlepairs_add_control(WINHANDLEPAIRS *pairs,WILLUSGUICONTROL *control)

    {
    static char *funcname="winhandlepairs_add";

    if (pairs->n>=pairs->na)
        {
        int newsize;
        newsize = pairs->na<128 ? 256: pairs->na*2;
        willus_mem_realloc_robust_warn((void **)&pairs->pair,newsize*sizeof(WINHANDLEPAIR),
                                      pairs->na*sizeof(WINHANDLEPAIR),funcname,10);
        pairs->na=newsize;
        }
    pairs->pair[pairs->n].oshandle=control->handle;
    pairs->pair[pairs->n].control=control;
    pairs->n++;
    pairs->sorted=0;
    }


static void winhandlepairs_remove_control(WINHANDLEPAIRS *pairs,WILLUSGUICONTROL *control)

    {
    int index;

    index=winhandlepairs_find_control_index(pairs,control->handle);
    if (index<0)
        return;
    if (index<pairs->n-1)
        memmove(&pairs->pair[index],&pairs->pair[index+1],sizeof(WINHANDLEPAIR)*(pairs->n-1-index));
    pairs->n--;
    }


WILLUSGUIWINDOW *willusgui_window_find(void *oshandle)

    {
    int i;

    i=winhandlepairs_find_control_index(&whpairs,oshandle);
    return(i<0 ? NULL : whpairs.pair[i].control);
    }


static int winhandlepairs_find_control_index(WINHANDLEPAIRS *pairs,void *oshandle)

    {
    int i1,i2;

    if (pairs->n<=0)
        return(-1);
    winhandlepairs_sort(pairs);
    i1=0;
    i2=pairs->n-1;
    if (pairs->pair[i1].oshandle==oshandle)
        return(i1);
    if (pairs->pair[i2].oshandle==oshandle)
        return(i2);
    while (i2-i1>1)
        {
        int inew;
 
        inew=(i1+i2)/2;
        if (pairs->pair[inew].oshandle==oshandle)
            return(inew);
        if (pairs->pair[inew].oshandle>oshandle)
            i2=inew;
        else
            i1=inew;
        }
    return(-1);
    }


static void winhandlepairs_sort(WINHANDLEPAIRS *pairs)

    {
    int top,n1,n;
    WINHANDLEPAIR x0;
    WINHANDLEPAIR *x;
    
    if (pairs->sorted)
        return; 
    n=pairs->n;
    if (n<2)
        {
        pairs->sorted=1;
        return;
        }
    x=pairs->pair;
    top=n/2;
    n1=n-1;
    while (1)
        {
        if (top>0)
            {
            top--;
            x0=x[top];
            }
        else
            {
            x0=x[n1];
            x[n1]=x[0];
            n1--;
            if (!n1)
                {
                x[0]=x0;
                pairs->sorted=1;
                return;
                }
            }
        {
        int parent,child;

        parent=top;
        child=top*2+1;
        while (child<=n1)
            {
            if (child<n1 && x[child].oshandle<x[child+1].oshandle)
                child++;
            if (x0.oshandle<x[child].oshandle)
                {
                x[parent]=x[child];
                parent=child;
                child+=(parent+1);
                }
            else
                break;
            }
        x[parent]=x0;
        }
        }
    pairs->sorted=1;
    }


void willusgui_sbitmap_resample_original(WILLUSGUICONTROL *control)

    {
    WILLUSGUIRECT *rect;
    double rw,rh,rr,mu;
    int o2osize;
    int w,h;
    WILLUSBITMAP *src,*dst;

    rect=&control->rect;
    dst=&control->bmp;
    src=control->obmp;
    dst->type=WILLUSBITMAP_TYPE_WIN32;
/*
printf("Source bitmap:  %d x %d\n",src->width,src->height);
printf("    preview window = %d x %d\n",rect->right-rect->left-1,rect->bottom-rect->top-1);
printf("    sbitmap_size = %d\n",control->sbitmap_size);
*/
    rw = (double)src->width / (rect->right-rect->left-1);
    rh = (double)src->height / (rect->bottom-rect->top-1);
    rr = rw > rh ? rw : rh;
/*
printf("rr=%g\n",rr);
*/
    /* For normal bitmap, fit to window */
    if (control->type==WILLUSGUICONTROL_TYPE_BITMAP)
        {
        w = src->width/rr+.5;
        h = src->height/rr+.5;
/*
printf("w=%d, h=%d\n",w,h);
*/
        control->dpi_rendered = control->dpi/rr;
        if (w==src->width && h==src->height)
            bmp_copy(dst,src);
        else
            {
            dst->bpp = 24;
            bmp_resample(dst,src,0.,0.,(double)src->width,(double)src->height,w,h);
            }
        return;
        }
    /* For scrollable bitmap, fit to nearest size magnifier */
    o2osize = (int)(0.5+4.*log(rr)/log(2));
    if (control->sbitmap_size==o2osize)
        {
        bmp_copy(dst,src);
        return;
        }
    mu = pow(2.,control->sbitmap_size/4.);
/*
printf("     multiplier = %g\n",mu/rr);
*/
    w = mu*src->width/rr+.5;
    h = mu*src->height/rr+.5;
/*
printf("     new bitmap = %d x %d\n",w,h);
*/
    dst->bpp = 24;
    bmp_resample(dst,src,0.,0.,(double)src->width,(double)src->height,w,h);
    }


void willusgui_sbitmap_change_size(WILLUSGUICONTROL *control,int delsize)

    {
    int maxsize,o2osize;
    WILLUSBITMAP *src;
    WILLUSGUIRECT *rect;
    double rw,rh,rr;

    if (control==NULL)
        return;
    src=control->obmp;
    rect=&control->rect;
    rw = (double)src->width / (rect->right-rect->left-1);
    rh = (double)src->height / (rect->bottom-rect->top-1);
    rr = rw > rh ? rw : rh;
    if (rr<1.)
        rr=1.;
    o2osize = (int)(0.5+4.*log(rr)/log(2));
    maxsize = 2*o2osize;
    if (delsize==0)
        {
        if (control->sbitmap_size!=0)
            control->sbitmap_size=0;
        else
            control->sbitmap_size=o2osize;
        }
    else
        {
        control->sbitmap_size += delsize;
        if (control->sbitmap_size < 0)
            control->sbitmap_size = 0;
        else if (control->sbitmap_size > maxsize)
            control->sbitmap_size = maxsize;
        }
    willusgui_control_close(control);
    }


void willusgui_sbitmap_proc(void *handle,int message,int wparam,void *lparam)

    {
#ifdef MSWINGUI
    willusgui_sbitmap_proc_internal((HWND)handle,(UINT)message,(WPARAM)wparam,(LPARAM)lparam);
#endif
    }

/*
** UTF-8
** Returns 0 if no folder (user cancels or GUI not available)
*/
int willusgui_folder_select(char *foldername,int maxlen)

    {
#ifdef MSWINGUI
    int status;
    short fnamew[MAXFILENAMELEN];
    status=winshell_get_foldernamew(fnamew,"Select a folder");
    if (!status)
        return(status);
    utf16_to_utf8(foldername,fnamew,maxlen);
    return(status);
#else
    foldername[0]='\0';
    return(0);
#endif
    }


/*
** Custom / Subclass callback functions for MS-Windows
*/
#ifdef MSWINGUI
/*
** "Scrollable bitmap" control callbacks
*/
LRESULT CALLBACK willusgui_sbitmap_proc_internal(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)

    {
    SCROLLINFO si;
    static int xMinScroll=0;       /* minimum horizontal scroll value  */
    static int xCurrentScroll=0;   /* current horizontal scroll value  */
    static int xMaxScroll=0;       /* maximum horizontal scroll value  */
    static int yMinScroll=0;       /* minimum vertical scroll value    */
    static int yCurrentScroll=0;   /* current vertical scroll value    */
    static int yMaxScroll=0;       /* maximum vertical scroll value    */
    static int buttondown=0;
    static int mx0,my0,mx,my;
    int x0,y0,w0,h0;
    int hscroll,vscroll,scrollable;
    WILLUSGUICONTROL *control;
    WILLUSGUIRECT rect0;
	
/*
printf("@willusgui_sbitmap_proc...message=0x%04x\n",message);
*/
    control=willusgui_window_find(hwnd);
    if (control==NULL)
        {
        if (icontrol==NULL)
            return(DefWindowProc(hwnd,message,wParam,lParam));
        else
            control=icontrol;
        }

    scrollable = (control->type==WILLUSGUICONTROL_TYPE_SCROLLABLEBITMAP);
    /* Get upper-left corner of bitmap for cropping */
    {
    willusgui_window_get_useable_rect(control,&rect0);
    x0=rect0.left;
    y0=rect0.top;
    w0=rect0.right-rect0.left+1;
    h0=rect0.bottom-rect0.top+1;
    }

    hscroll = control->bmp.width > control->rect.right-control->rect.left-1;
    vscroll = control->bmp.height > control->rect.bottom-control->rect.top-1;
/*
willusgui_dprintf("willusgui_sbitmap: message=%04X, wParam=%d, lParam=%d\n",message,(int)wParam,(int)lParam);
*/
/*
printf("hscroll=%d, vscroll=%d\n",hscroll,vscroll);
*/
	switch (message) 
        {
        case WM_CREATE:
            break;
        case WM_SIZE: 
            { 
            int xNewSize; 
            int yNewSize; 

            xNewSize = LOWORD(lParam); 
            yNewSize = HIWORD(lParam); 
            /* 
            ** The horizontal scrolling range is defined by 
            ** (bitmap_width) - (client_width). The current horizontal 
            ** scroll value remains within the horizontal scrolling range. 
            */
            if (hscroll)
                {
                xMaxScroll = max(control->bmp.width-xNewSize, 0); 
                xCurrentScroll = min(xCurrentScroll, xMaxScroll); 
                si.cbSize = sizeof(si); 
                si.fMask  = SIF_RANGE | SIF_PAGE | SIF_POS; 
                si.nMin   = xMinScroll; 
                si.nMax   = control->bmp.width;
                si.nPage  = xNewSize; 
                si.nPos   = xCurrentScroll; 
                SetScrollInfo(hwnd, SB_HORZ, &si, TRUE); 
                }

            /* 
            ** The vertical scrolling range is defined by 
            ** (bitmap_height) - (client_height). The current vertical 
            ** scroll value remains within the vertical scrolling range. 
            */
            if (vscroll)
                {
                yMaxScroll = max(control->bmp.height - yNewSize, 0); 
                yCurrentScroll = min(yCurrentScroll, yMaxScroll); 
                si.cbSize = sizeof(si); 
                si.fMask  = SIF_RANGE | SIF_PAGE | SIF_POS; 
                si.nMin   = yMinScroll; 
                si.nMax   = control->bmp.height;
                si.nPage  = yNewSize; 
                si.nPos   = yCurrentScroll; 
                SetScrollInfo(hwnd, SB_VERT, &si, TRUE); 
                }
            break; 
            } 
 
        case WM_PAINT: 
            { 
            PRECT prect; 
            PAINTSTRUCT ps;
            int w,h,x1,y1;
/*
printf("@sbitmap WM_PAINT.\n");
*/

            BeginPaint(hwnd, &ps); 
            prect = &ps.rcPaint; 
            /*
            ** If the window has been resized and the user has 
            ** captured the screen, use the following call to 
            ** BitBlt to paint the window's client area. 
            */
            w=prect->right-prect->left+1;
            h=prect->bottom-prect->top+1;
            x1 = hscroll ? prect->left : prect->left + (w-control->bmp.width)/2;
            y1 = vscroll ? prect->top : prect->top + (h-control->bmp.height)/2;
            /* Back fill with gray */
            if (x1!=prect->left || y1!=prect->top)
                 {
                 HBRUSH brush;
                 brush=CreateSolidBrush(0xc0c0c0);
                 FillRect(ps.hdc,prect,brush);
                 DeleteObject(brush);
                 }
/*
printf("@blitter:  prect @ %d, %d = %d x %d\n",
(int)prect->left,(int)prect->top,(int)(prect->right-prect->left+1),(int)(prect->bottom-prect->top+1));
printf("    xscr,yscr = %d,%d\n",xCurrentScroll,yCurrentScroll);
*/
            bmp_blit_to_hdc_ex(&control->bmp,ps.hdc,x1,y1,
                                hscroll ? w : control->bmp.width, 
                                vscroll ? h : control->bmp.height, 
                                hscroll ? xCurrentScroll : 0,
                                vscroll ? yCurrentScroll : 0);

            EndPaint(hwnd, &ps); 
            break; 
            } 

        case WM_HSCROLL: 
            { 
            int xNewPos;    /* new position  */

            if (!scrollable || !hscroll)
                break;
            switch (LOWORD(wParam)) 
                { 
                /* User clicked the scroll bar shaft left of the scroll box.  */
                case SB_PAGEUP: 
                    xNewPos = xCurrentScroll - 50; 
                    break; 
                /* User clicked the scroll bar shaft right of the scroll box. */
                case SB_PAGEDOWN: 
                    xNewPos = xCurrentScroll + 50; 
                    break; 
                /* User clicked the left arrow. */
                case SB_LINEUP: 
                    xNewPos = xCurrentScroll - 5; 
                    break; 
                /* User clicked the right arrow. */
                case SB_LINEDOWN: 
                    xNewPos = xCurrentScroll + 5; 
                    break; 
                /* User dragged the scroll box. */
                case SB_THUMBPOSITION: 
                case SB_THUMBTRACK:
                    xNewPos = HIWORD(wParam); 
                    break; 
                default: 
                    xNewPos = xCurrentScroll; 
                } 
 
            /* New position must be between 0 and the screen width. */
            xNewPos = max(0, xNewPos); 
            xNewPos = min(xMaxScroll, xNewPos); 
            if (xNewPos == xCurrentScroll) 
                break; 
            /* Reset the current scroll position. */
            xCurrentScroll = xNewPos; 
            RedrawWindow(hwnd,NULL,NULL,RDW_INVALIDATE);
            // Reset the scroll bar. 
            si.cbSize = sizeof(si); 
            si.fMask  = SIF_POS; 
            si.nPos   = xCurrentScroll; 
            SetScrollInfo(hwnd,SB_HORZ,&si,TRUE); 
            break; 
            } 
        
        case WM_VSCROLL: 
            { 
            int yNewPos;    /* new position  */

            if (!scrollable || !vscroll)
                break; 
            switch (LOWORD(wParam)) 
                { 
                /* User clicked the scroll bar shaft above the scroll box. */ 
                case SB_PAGEUP: 
                    yNewPos = yCurrentScroll - 50; 
                    break; 
                /* User clicked the scroll bar shaft below the scroll box. */
                case SB_PAGEDOWN: 
                    yNewPos = yCurrentScroll + 50; 
                    break; 
                /* User clicked the top arrow. */
                case SB_LINEUP: 
                    yNewPos = yCurrentScroll - 5; 
                    break; 
                /* User clicked the bottom arrow. */
                case SB_LINEDOWN: 
                    yNewPos = yCurrentScroll + 5; 
                    break; 
                /* User dragged the scroll box. */
                case SB_THUMBPOSITION: 
                case SB_THUMBTRACK:
                    yNewPos = HIWORD(wParam); 
                    break; 
                default: 
                    yNewPos = yCurrentScroll; 
                } 
            yNewPos = max(0, yNewPos); 
            yNewPos = min(yMaxScroll, yNewPos); 
            if (yNewPos == yCurrentScroll) 
                break; 
            yCurrentScroll = yNewPos; 
            RedrawWindow(hwnd,NULL,NULL,RDW_INVALIDATE);
            /* Reset the scroll bar. */
            si.cbSize = sizeof(si); 
            si.fMask  = SIF_POS; 
            si.nPos   = yCurrentScroll; 
            SetScrollInfo(hwnd, SB_VERT, &si, TRUE); 
            break; 
            } 

        case WM_LBUTTONDOWN:
            {
            if (scrollable)
                {
                buttondown |= 1;
                mx0=mx=LOWORD(lParam);
                my0=my=HIWORD(lParam);
                }
/*
            else
                {
                if (control->anchor.left < -9999)
                    {
                    control->anchor.left=LOWORD(lParam);
                    control->anchor.top=HIWORD(lParam);
                    if (control->rectmarked.left < -9999)
                        {
                        control->rectanchor = control->anchor;
                        control->rectanchor.right = control->rectanchor.left;
                        control->rectanchor.bottom = control->rectanchor.top;
                        }
                    }
                }
*/
            return(0);
            }
        case WM_MOUSEWHEEL:
            {
            int dy,ycs,flags;

            if (!scrollable)
                break;
            dy=HIWORD(wParam);
            if (dy>=32768)
                dy -= 65536;
            flags=LOWORD(wParam);
            if (flags&MK_CONTROL)
                {
                willusgui_sbitmap_change_size(control,dy<0 ? -1 : 1);
                break;
                }
            if (!vscroll)
                break;
            dy = -dy/5;
            ycs=yCurrentScroll;
            yCurrentScroll += dy;
            if (yCurrentScroll<0)
                yCurrentScroll=0;
            if (yCurrentScroll>yMaxScroll)
                yCurrentScroll=yMaxScroll;
            dy=yCurrentScroll-ycs;
            RedrawWindow(hwnd,NULL,NULL,RDW_INVALIDATE);
            /* Move the scroll bars */
            if (dy!=0)
                {
                si.cbSize = sizeof(si); 
                si.fMask  = SIF_POS; 
                si.nPos   = yCurrentScroll; 
                SetScrollInfo(hwnd, SB_VERT, &si, TRUE); 
                }
            break;
            }
        case WM_KEYDOWN:
            {
            int kshift,kctrl;
            WILLUSGUIRECT newrect;

            if (scrollable || control->rectmarked.left < -9000)
                break;
            kshift=GetKeyState(VK_SHIFT);
            kshift= (kshift<0);
            kctrl=GetKeyState(VK_CONTROL);
            kctrl = (kctrl<0);
            newrect = control->rectmarked;
            switch (wParam)
                {
                case 37: /* Left arrow */
                    if (kshift && kctrl && newrect.right > newrect.left)
                        newrect.right--;
                    else if (kctrl && !kshift && newrect.left > x0)
                        {
                        newrect.right--;
                        newrect.left--;
                        }
                    break;
                case 38: /* up arrow */
                    if (kshift && kctrl)
                        {
                        if (newrect.bottom > newrect.top)
                            newrect.bottom--;
                        }
                    else if (kctrl && newrect.top > y0)
                        {
                        newrect.top--;
                        newrect.bottom--;
                        }
                    break;
                case 39: /* right arrow */
                    if (kshift && kctrl && newrect.right < x0+w0-1)
                        newrect.right++;
                    else if (kctrl && !kshift && newrect.right < x0+w0-1)
                        {
                        newrect.right++;
                        newrect.left++;
                        }
                    break;
                case 40: /* down arrow */
                    if (kshift && kctrl && newrect.bottom < y0+h0-1)
                        newrect.bottom++;
                    else if (kctrl && !kshift && newrect.bottom < y0+h0-1)
                        {
                        newrect.top++;
                        newrect.bottom++;
                        }
                    break;
                }
            if (memcmp(&newrect,&control->rectmarked,sizeof(WILLUSGUIRECT)))
                {
                willusgui_window_draw_rect_outline(control,&control->rectmarked,-1);
                control->rdcount++;
                control->rectmarked = newrect;
                willusgui_window_draw_rect_outline(control,&control->rectmarked,-1);
                control->rdcount++;
                }
            return(0);
            }
        case WM_CHAR:
            {
            /* ESC erases rectangle */
            if (!scrollable)
                {
                if (wParam==0x1b && control->rectmarked.left > -9000)
                    {
                    willusgui_window_draw_rect_outline(control,&control->rectmarked,-1);
                    control->rdcount++;
                    control->rectmarked.left = -10000;
                    }
                return(0);
                }
            break;
            }
        case WM_TIMER:
            {
            POINT p;
            int down,curstype;

            if (scrollable)
                return(0);
            GetCursorPos(&p); /* Get mouse pos in screen coords */
            MapWindowPoints(NULL,hwnd,&p,1);
            down=GetKeyState(VK_LBUTTON);
            curstype=willusguirect_cursor_type(&control->rectmarked,&control->rect,p.x,p.y);
/*
printf("ct=%d (%d,%d,%d,%d) (%d,%d,%d,%d) %d,%d\n",
curstype,
control->rectmarked.left,
control->rectmarked.top,
control->rectmarked.right,
control->rectmarked.bottom,
control->rect.left,
control->rect.top,
control->rect.right,
control->rect.bottom,
(int)p.x,(int)p.y);
*/
/*
{
static int ct=-2;
if (ct!=curstype)
{
printf("ctype=%d\n",curstype);
ct=curstype;
}
}
*/
            willusgui_set_cursor(curstype>0 ? curstype+1 : 0);
            if (control->crosshair.left > -9000)
                {
                willusgui_window_draw_crosshair(control,control->crosshair.left,control->crosshair.top,-1);
                control->crosshair.left = -10000;
                }
            /* Need to draw initial rectangle */
            if (control->rectmarked.left > -9000 && (control->rdcount&1)==0)
                {
                willusgui_window_draw_rect_outline(control,&control->rectmarked,-1);
                control->rdcount++;
                }
      
            /* Left button down */
            if (down&128)
                {
                char buf[256];

                if (control->anchor.left < -9999)
                    {
/*
printf("Just clicked, curstype=%d\n",curstype);
*/
                    /* Click outside the window doesn't start anything. */
                    if (p.x<x0 || p.x>x0+w0-1 || p.y<y0 || p.y>y0+h0-1)
                        return(0);
                    control->anchor.left=p.x;
                    control->anchor.top=p.y;
                    if (control->rectmarked.left < -9999)
                        control->anchor.right = 0;
                    else
                        {
                        if (curstype < 0)
                            return(0);
                        if (curstype==0)
                            {
                            willusgui_window_draw_rect_outline(control,&control->rectmarked,-1);
                            control->rdcount++;
                            control->rectmarked.left = -10000;
                            }
                        control->anchor.right = curstype;
                        }
                    return(0);
                    }
                if (control->rectmarked.left > -9000)
                    {
                    willusgui_window_draw_rect_outline(control,&control->rectmarked,-1);
                    control->rdcount++;
                    }
                
                /* Need to check type of move */
                switch (control->anchor.right)
                    {
                    case 0:
                        control->rectmarked.left   = p.x;
                        control->rectmarked.right  = control->anchor.left;
                        control->rectmarked.top    = p.y;
                        control->rectmarked.bottom = control->anchor.top;
                        break;
                    case 1:
                        control->rectmarked.left = p.x;
                        control->rectmarked.top = p.y;
                        if (control->rectmarked.top > control->rectmarked.bottom
                             && control->rectmarked.left > control->rectmarked.right)
                            control->anchor.right=9;
                        else if (control->rectmarked.top > control->rectmarked.bottom)
                            control->anchor.right=7;
                        else if (control->rectmarked.left > control->rectmarked.right)
                            control->anchor.right=3;
                        break;
                    case 2:
                        control->rectmarked.top = p.y;
                        if (control->rectmarked.top > control->rectmarked.bottom)
                            control->anchor.right=8;
                        break;
                    case 3:
                        control->rectmarked.top = p.y;
                        control->rectmarked.right = p.x;
                        if (control->rectmarked.top > control->rectmarked.bottom
                             && control->rectmarked.left > control->rectmarked.right)
                            control->anchor.right=7;
                        else if (control->rectmarked.top > control->rectmarked.bottom)
                            control->anchor.right=9;
                        else if (control->rectmarked.left > control->rectmarked.right)
                            control->anchor.right=1;
                        break;
                    case 4:
                        control->rectmarked.left = p.x;
                        if (control->rectmarked.left > control->rectmarked.right)
                            control->anchor.right=6;
                        break;
                    case 5:
                        {
                        double mx,my,dx,dy;
                        mx = p.x-control->anchor.left;
                        my = p.y - control->anchor.top;
                        dx=control->rectmarked.right - control->rectmarked.left;
                        dy=control->rectmarked.bottom - control->rectmarked.top;
                        control->rectmarked.left += mx;
                        control->rectmarked.right += mx;
                        control->rectmarked.top += my;
                        control->rectmarked.bottom += my;
                        if (control->rectmarked.left < x0)
                            {
                            control->rectmarked.left = x0;
                            control->rectmarked.right = x0+dx;
                            }
                        else if (control->rectmarked.right > x0+w0-1)
                            {
                            control->rectmarked.right = x0+w0-1;
                            control->rectmarked.left = x0+w0-1-dx;
                            }
                        if (control->rectmarked.top < y0)
                            {
                            control->rectmarked.top=y0;
                            control->rectmarked.bottom=y0+dy;
                            }
                        else if (control->rectmarked.bottom > y0+h0-1)
                            {
                            control->rectmarked.top=y0+h0-1-dy;
                            control->rectmarked.bottom=y0+h0-1;
                            }
                        control->anchor.left = p.x;
                        control->anchor.top = p.y;
                        break;
                        }
                    case 6:
                        control->rectmarked.right = p.x;
                        if (control->rectmarked.left > control->rectmarked.right)
                            control->anchor.right=4;
                        break;
                    case 7:
                        control->rectmarked.left = p.x;
                        control->rectmarked.bottom = p.y;
                        if (control->rectmarked.top > control->rectmarked.bottom
                             && control->rectmarked.left > control->rectmarked.right)
                            control->anchor.right=3;
                        else if (control->rectmarked.top > control->rectmarked.bottom)
                            control->anchor.right=1;
                        else if (control->rectmarked.left > control->rectmarked.right)
                            control->anchor.right=9;
                        break;
                    case 8:
                        control->rectmarked.bottom = p.y;
                        if (control->rectmarked.top > control->rectmarked.bottom)
                            control->anchor.right=2;
                        break;
                    case 9:
                        control->rectmarked.right = p.x;
                        control->rectmarked.bottom = p.y;
                        if (control->rectmarked.top > control->rectmarked.bottom
                             && control->rectmarked.left > control->rectmarked.right)
                            control->anchor.right=1;
                        else if (control->rectmarked.top > control->rectmarked.bottom)
                            control->anchor.right=3;
                        else if (control->rectmarked.left > control->rectmarked.right)
                            control->anchor.right=7;
                        break;
                    }
                willusguirect_bound(&control->rectmarked,&rect0);
                willusgui_window_draw_rect_outline(control,&control->rectmarked,-1);
                control->rdcount++;
                if (control->dpi_rendered>0.)
                    sprintf(buf,"%04d,%04d %04d x %04d (%05.2fin,%05.2fin %05.2fin x %05.2fin)",
                                control->rectmarked.left,
                                control->rectmarked.top,
                                control->rectmarked.right-control->rectmarked.left+1,
                                control->rectmarked.bottom-control->rectmarked.top+1,
                                control->rectmarked.left/control->dpi_rendered,
                                control->rectmarked.top/control->dpi_rendered,
                                (control->rectmarked.right-control->rectmarked.left+1)
                                      / control->dpi_rendered,
                                (control->rectmarked.bottom-control->rectmarked.top+1)
                                      / control->dpi_rendered);
                else
                    sprintf(buf,"%04d,%04d %04d x %04d",
                                control->rectmarked.left,
                                control->rectmarked.top,
                                control->rectmarked.right-control->rectmarked.left+1,
                                control->rectmarked.bottom-control->rectmarked.top+1);
                SetWindowText((HWND)control->parent->handle,buf);
                }
            else
                {
                char buf[256];

                control->anchor.left = -10000;
                if (control->rectmarked.left < -9000)
                    {
                    control->crosshair.left=p.x;
                    control->crosshair.top=p.y;
                    willusgui_window_draw_crosshair(control,p.x,p.y,-1);
                    if (control->dpi_rendered>0.)
                        sprintf(buf,"%04d x %04d (%05.2f in x %05.2f in)",
                                    (int)p.x,(int)p.y,p.x/control->dpi_rendered,
                                                      p.y/control->dpi_rendered);
                    else
                        sprintf(buf,"%04d x %04d",(int)p.x,(int)p.y);
                    SetWindowText((HWND)control->parent->handle,buf);
                    }
                }
            return(0);
            }
        case WM_MOUSEMOVE:
            {
            int x,y,dx,dy,xcs,ycs;

/*
printf("sbitmap MOUSEMOVE.\n");
*/
            if (!scrollable)
                break;
            if (!(wParam&MK_RBUTTON))
                {
                if (buttondown&2)
                    buttondown &= (~2);
                }
            if (!(wParam&MK_LBUTTON))
                {
                if (buttondown&1)
                    buttondown &= (~1);
                break;
                }
            if (!hscroll && !vscroll)
                break;
            x=LOWORD(lParam);
            y=HIWORD(lParam);
            if (x==mx && y==my)
                break;
            xcs=xCurrentScroll;
            ycs=yCurrentScroll;
            yCurrentScroll += (my-y);
            if (yCurrentScroll<0)
                yCurrentScroll=0;
            if (yCurrentScroll>yMaxScroll)
                yCurrentScroll=yMaxScroll;
            xCurrentScroll += (mx-x);
            if (xCurrentScroll<0)
                xCurrentScroll=0;
            if (xCurrentScroll>xMaxScroll)
                xCurrentScroll=xMaxScroll;
            dx=xCurrentScroll-xcs;
            dy=yCurrentScroll-ycs;
            RedrawWindow(hwnd,NULL,NULL,RDW_INVALIDATE);
            /* Move the scroll bars */
            if (dy!=0)
                {
                si.cbSize = sizeof(si); 
                si.fMask  = SIF_POS; 
                si.nPos   = yCurrentScroll; 
                SetScrollInfo(hwnd, SB_VERT, &si, TRUE); 
                }
            if (dx!=0)
                {
                si.cbSize = sizeof(si); 
                si.fMask  = SIF_POS; 
                si.nPos   = xCurrentScroll; 
                SetScrollInfo(hwnd, SB_HORZ, &si, TRUE); 
                }
            mx=x;
            my=y;
            return(0);
            }
        case WM_LBUTTONUP:
            {
            int x,y;

            if (scrollable)
                {
                x=LOWORD(lParam);
                y=HIWORD(lParam);
                if ((buttondown&1) && x==mx0 && y==my0)
                    {
                    buttondown &= (~1);
                    /* willusgui_sbitmap__size(control,-1); */
                    }
                buttondown &= (~1);
                }
/*
            control->anchor.left = -10000;
*/
            return(0);
            }
        case WM_RBUTTONDOWN:
            if (scrollable)
                buttondown |= 2;
            else
                {
                static int bcolors[3]={0x60b060,0xe0ffe0,0xe0ffe0};
                static char *help=
                   "Use the left mouse button to click and drag to draw a crop box which will "
                   "set your crop margins.\n"
                   "Keyboard shortcuts:\n"
                   "ESC:  Clear the crop box.\n"
                   "CTRL-<arrows>:  Move the crop box.\n"
                   "SHIFT-CTRL-<arrows>:  Change the crop box size.";
                willusgui_message_box(control->parent,"Draw a crop box",help,"*&OK",NULL,NULL,
                                   NULL,0,20,600,0xe0ffe0,bcolors,NULL,1);
                }
            /*
                {
                if (control->anchor.left < -9999)
                    {
                    control->anchor.left=LOWORD(lParam);
                    control->anchor.top=HIWORD(lParam);
                    control->anchor.right=2;
                    }
                }
             */
            return(0);
        case WM_RBUTTONUP:
            if (scrollable)
                {
                if (buttondown&2)
                    {
                    buttondown &= (~2);
                    willusgui_sbitmap_change_size(control,0);
                    }
                buttondown &= (~2);
                }
            return(0);
        case WM_CLOSE:
            willusgui_control_close_ex(control,1);
            return(0);
        case WM_DESTROY:
            break;
        }
    return(DefWindowProc(hwnd,message,wParam,lParam));
    }


/*
** Callback for MS Windows multi-line edit class which allows the tab key to
** tab to the next control rather than tabbing within the edit box.
*/
LRESULT CALLBACK willusgui_edit2_proc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)

    {	
    WNDPROC wndproc;

	switch (message) 
        {
        case WM_KILLFOCUS:
/* printf("EDIT BOX LOST FOCUS.\n"); */
             break;
/*
        case WM_SETFOCUS:
            SendMessage(hWnd,EM_SETSEL,0,-1);
*/
        case WM_CHAR:
            {
            /* Ctrl-A */
            if (wParam==1)
                {
                PostMessage(hWnd,EM_SETSEL,0,-1);
                return(0);
                }
            else if ((TCHAR)wParam==VK_TAB)
                {
                SetFocus(GetNextDlgTabItem(GetParent(hWnd),hWnd,GetAsyncKeyState(VK_SHIFT)));
                //cannot use WM_NEXTDLGCTL without handling in a parent window using the same code 
                return(0); 
                }
            }
        }
    wndproc=(WNDPROC)defproc_edit2;
    return(wndproc(hWnd,message,wParam,lParam));
    }



/*
** Call back MS Windows subclass of editclass which selects entire text when receiving focus.
*/
LRESULT CALLBACK willusgui_edit3_proc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)

    {	
    WNDPROC wndproc;
/*
if (message==WM_IME_NOTIFY)
{
printf("edit3:  hwnd=%p, message=%04x, ins=%d\n",hWnd,message,ime_notify_status);
printf("imenotify=%04x\n",WM_IME_NOTIFY);
}
if (message==WM_CAPTURECHANGED)
printf("e3 0x%X WM_CAPTURECHANGED\n",hWnd);
else if (message==WM_IME_NOTIFY)
printf("e3 0x%X WM_IME_NOTIFY wparam=0x%X, lparam=0x%X\n",hWnd,(int)wParam,(int)lParam);
else if (message==WM_SETFOCUS)
printf("e3 0x%X WM_SETFOCUS wparam=0x%X, lparam=0x%X\n",hWnd,(int)wParam,(int)lParam);
else if (message==EM_SETSEL)
printf("e3 0x%X EM_SETSEL wparam=0x%X, lparam=0x%X\n",hWnd,(int)wParam,(int)lParam);
*/
/*
else
printf("edit3: 0x%X message=0x%X\n",hWnd,message);
win_sleep(30);
*/
	switch (message) 
        {
        case WM_SETFOCUS:
            /* Fix applied at k2pdfopt v2.32 */
            PostMessage(hWnd,EM_SETSEL,0,-1);
            break;
        case WM_CHAR:
            /* Ctrl-A */
            if (wParam==1)
                {
                PostMessage(hWnd,EM_SETSEL,0,-1);
                return(0);
                }
            break;
        /*
        case WM_CHAR:
            {
            if((TCHAR)wParam==VK_TAB)
                {
                SetFocus(GetNextDlgTabItem(GetParent(hWnd),hWnd,GetAsyncKeyState(VK_SHIFT)));
                //cannot use WM_NEXTDLGCTL without handling in a parent window using the same code 
                return(0); 
                }
            }
        */
        }
    wndproc=(WNDPROC)defproc_edit3;
    return(wndproc(hWnd,message,wParam,lParam));
    }
#endif /* MSWINGUI */
