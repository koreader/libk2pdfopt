/*
** k2gui_osdep.c    K2pdfopt O/S-Dependent WILLUSGUI functions
**
** Copyright (C) 2016  http://willus.com
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
static K2GUI *k2gui;
static K2CONVBOX *k2gui_cbox;
static K2CONVBOX *k2gui_overlay;

#ifdef MSWINGUI
#include <windows.h>
#include <shlobj.h>
#endif

#ifdef MSWINGUI
LRESULT CALLBACK k2mswingui_cbox_process_messages(HWND hwnd,UINT iMsg,WPARAM wParam,LPARAM lParam);
LRESULT CALLBACK k2mswingui_overlay_process_messages(HWND hwnd,UINT iMsg,WPARAM wParam,LPARAM lParam);
LRESULT CALLBACK k2mswingui_process_messages(HWND hwnd,UINT iMsg,WPARAM wParam,LPARAM lParam);
#endif
static int k2gui_osdep_is_dialogbox_message(void *hwnd0,void *msg0);
static int k2gui_osdep_shiftkey_state(void);


short *k2gui_osdep_wide_cmdline(void)

    {
#ifdef MSWINGUI
    return((short *)GetCommandLineW());
#else
    return(NULL);
#endif
    }


void k2gui_osdep_init(K2GUI *k2gui0) /* ,void *hinst,void *hprevinst) */

    {
    k2gui=k2gui0;
    }


/*
** This function is the main thread which handles messages from either the
** conversion dialog box (including the buttons) or the overlay dialog box.
**
** If "converting", the dialog box is in the converting state, otherwise
** it has a different set of buttons and is in the "final" state.
**
** The translated message gets sent to the dialog box for all but the
** bitmap control.  The dialog box processing functions are:
**     1. k2mswingui_overlay_process_messages() (overlay)
**     2. k2mswingui_cbox_process_messages() (conversion)
**
** The bitmap control messages to that control are processed by the library
** function:  willusgui_sbitmap_proc_internal().
**
** procid==1 for conversion, 2 for overlay.
**
*/
int k2gui_osdep_window_proc_messages(WILLUSGUIWINDOW *win,void *semaphore,int procid,
                                     WILLUSGUICONTROL *closebutton)

    {
#ifdef MSWINGUI
    MSG msg;
    int done;
#if (WILLUSDEBUGX & 0x2000000)
k2dprintf("@k2gui_osdep_window_proc_messages, win=%p, winhand=%p\n",win,win->handle);
#endif


    done=0;
    while (win->handle!=NULL && GetMessage(&msg,NULL,0,0))
        {
#if (WILLUSDEBUGX & 0x2000000)
if (msg.message!=0x113)
k2dprintf("k2gui_o_w_p_m: hwnd=%p, imsg=%04X, wparam=%d, conv=%d\n",msg.hwnd,msg.message,msg.wParam);
if (procid==2)
k2dprintf("    converting=%d\n",k2gui_overlay->converting);
#endif
        if (!done && semaphore!=NULL)
            {
            /* Is conversion complete? */
            if (willusgui_semaphore_status(semaphore)==1)
                {
#if (WILLUSDEBUGX & 0x2000000)
k2dprintf("k2gui_o_w_p_m:  DONE CREATING OVERLAY.  procid=%d\n",procid);
#endif
                done=1;
                /* Final print */
                if (procid==1)
                    {
#if (WILLUSDEBUGX & 0x2000000)
k2dprintf("k2gui_o_w_p_m:  CHANGING CONV BOX TO FINAL CONFIG.\n");
#endif
                    k2gui_cbox_final_print();
                    /* Change button to "Close" */
                    if (closebutton!=NULL)
                        k2gui_cbox_close_buttons();
                    }
                else
                    {
#if (WILLUSDEBUGX & 0x2000000)
k2dprintf("k2gui_o_w_p_m:  CHANGING OVERLAY BOX TO FINAL CONFIG.\n");
#endif
                    k2gui_overlay_final_print();
                    /* Change button to "Close" */
                    if (closebutton!=NULL)
                        k2gui_overlay_close_buttons();
                    }
                continue;
                }
            }

        /* This gets tab-stops to work */
        if (procid==2) /* Don't want tab stops if overlay window */
            {
            if (k2gui_overlay->converting!=1
                  || !k2gui_osdep_is_dialogbox_message((void *)win->handle,(void *)&msg))
                {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
                }
            }
        else
            {
            if (!k2gui_osdep_is_dialogbox_message((void *)win->handle,(void *)&msg))
            // if (!IsDialogMessage((HWND)win->handle,&msg))
                {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
                }
            }
        }
#if (WILLUSDEBUGX & 0x2000000)
k2dprintf("k2gui_o_w_p_m: EXITING\n"
          "               semaphore=%p, win->handle=%p\n",semaphore,win->handle);
k2dprintf("               done=%d\n",done);
k2dprintf("               msg.wParam=%d\n",msg.wParam);
#endif
    if (semaphore!=NULL)
        return(done);
    return(msg.wParam);
#else
    return(0);
#endif
    }


static int k2gui_osdep_is_dialogbox_message(void *hwnd0,void *msg0)

    {
#ifdef MSWINGUI
    HWND hwnd;
    MSG *msg;

    hwnd=(HWND)hwnd0;
    msg=(MSG*)msg0;
    /* Detect special keys to let through */
    if (msg->message==WM_KEYDOWN)
        {
        int shiftstate;

        shiftstate=k2gui_osdep_shiftkey_state();
/*
printf("wparam='%c', shiftstate=%X\n",msg->wParam,shiftstate);
*/
        if (msg->wParam=='D' && (shiftstate&0xc))
            SendMessage(hwnd,msg->message,msg->wParam|(shiftstate<<8),msg->lParam);
/*
printf("is_dialog:  keydown = %d, %d\n",(int)msg->wParam,(int)msg->lParam);
printf("   lshift = %d\n",GetKeyState(VK_LSHIFT));
printf("   rshift = %d\n",GetKeyState(VK_RSHIFT));
printf("   lctrl = %d\n",GetKeyState(VK_LCONTROL));
printf("   rctrl = %d\n",GetKeyState(VK_RCONTROL));
*/
        }
    return(IsDialogMessage(hwnd,msg));
#else
    return(0);
#endif
    }


static int k2gui_osdep_shiftkey_state(void)

    {
#ifdef MSWINGUI
    return(
           (GetKeyState(VK_LSHIFT)<0 ? 1 : 0)
         | (GetKeyState(VK_RSHIFT)<0 ? 2 : 0)
         | (GetKeyState(VK_LCONTROL)<0 ? 4 : 0)
         | (GetKeyState(VK_RCONTROL)<0 ? 8 : 0)
          );
#else
    return(0);
#endif
    }


/*
** Initialize and display main WILLUSGUI window
*/
void k2gui_osdep_main_window_init(WILLUSGUIWINDOW *win,int normal_size)

    {
#ifdef MSWINGUI
    static char *appname="k2pdfopt";
    WNDCLASSEX  wndclass;
    HICON       iconr,smalliconr;
    HWND hwnd;

#if (WILLUSDEBUGX & 0x2000)
printf("@k2gui_osdep_main_window_init\n");
#endif
    win_icons_from_exe((void **)&iconr,(void **)&smalliconr);
    wndclass.cbSize        = sizeof(wndclass);
    wndclass.style         = CS_SAVEBITS | CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc   = k2mswingui_process_messages;
    wndclass.cbClsExtra    = 0;
    wndclass.cbWndExtra    = 0;
    wndclass.hInstance     = (HINSTANCE)willusgui_instance();
    wndclass.hIcon         = iconr;
    wndclass.hCursor       = NULL; /* LoadCursor(NULL,IDC_ARROW); */
    wndclass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wndclass.lpszMenuName  = NULL;
    wndclass.lpszClassName = appname;
    wndclass.hIconSm       = smalliconr;
    RegisterClassEx(&wndclass);
    k2gui->mainwin.handle=NULL;
    hwnd = CreateWindow(appname,appname,WS_OVERLAPPEDWINDOW,
                        win->rect.left,win->rect.top,
                        win->rect.right-win->rect.left+1,
                        win->rect.bottom-win->rect.top+1,
                        NULL,NULL,(HINSTANCE)willusgui_instance(),NULL);
    /* Init menus */
    /* k2wingui_init_menus(k2wingui->hwnd); */
    ShowWindow(hwnd,normal_size ? SW_SHOWNORMAL : SW_MINIMIZE);
    UpdateWindow(hwnd);
    /*
    ** This handle actually gets assigned earlier--in k2mswingui_process_messages(),
    ** otherwise it is too late for some of the functions that need it, e.g.
    ** k2gui_osdep_window_menus_init().
    */
    win->handle=(void *)hwnd;
#else
    win->handle=NULL;
#endif
    }


/*
** Initialize and display k2pdfopt conversion dialog box.
*/
void k2gui_osdep_cbox_init(K2CONVBOX *k2cb0,WILLUSGUIWINDOW *win,WILLUSGUIWINDOW *parent,
                           void *hinst,int rgbcolor)

    {
#ifdef MSWINGUI
    static char *classname="k2gui_cbox";
    static char *title="K2pdfopt: Converting files...";
    static int class_registered=0;

    k2gui_cbox=k2cb0;
    /* Register class only once */
    if (!class_registered)
        {
        WNDCLASSEX  wndclass;
        static HBRUSH hbrush;

        hbrush = CreateSolidBrush(((rgbcolor&0xff0000)>>16)|(rgbcolor&0xff00)|((rgbcolor&0xff)<<16));
        wndclass.cbSize        = sizeof(wndclass);
        wndclass.style         = CS_SAVEBITS | CS_HREDRAW | CS_VREDRAW;
        wndclass.lpfnWndProc   = (WNDPROC)k2mswingui_cbox_process_messages;
        wndclass.cbClsExtra    = 0;
        wndclass.cbWndExtra    = 0;
        wndclass.hInstance     = (HINSTANCE)hinst;
        wndclass.hIcon         = NULL;
        wndclass.hCursor       = NULL;
        wndclass.hbrBackground = hbrush;
        wndclass.lpszMenuName  = NULL;
        wndclass.lpszClassName = classname;
        wndclass.hIconSm       = NULL;
        RegisterClassEx(&wndclass);
        class_registered=1;
#if (WILLUSDEBUGX & 0x2000)
printf("Class registered.\n");
#endif
        }
    /*
    ** This handle actually gets assigned earlier--in k2mswingui_process_messages(),
    ** otherwise it is too late for some of the functions that need it, e.g.
    ** k2gui_osdep_window_menus_init().
    */
    k2gui_cbox->mainwin.handle=NULL;
/*
printf("@(%d,%d), %d x %d\n",win->rect.left,win->rect.top,win->rect.right-win->rect.left+1,
win->rect.bottom-win->rect.top+1);
*/

    /*
    ** v2.02:  Use WS_OVERLAPPEDWINDOW instead of WS_OVERLAPPED so that we get a close
    **         and a minimize button.
    */
    win->handle = (void*)CreateWindow(classname,title,WS_OVERLAPPEDWINDOW,
                             win->rect.left,win->rect.top,
                             win->rect.right-win->rect.left+1,
                             win->rect.bottom-win->rect.top+1,
                             parent->handle,NULL,0,NULL);
    /*
    win->handle = (void*)CreateWindowEx(WS_EX_TOPMOST,classname,title,WS_OVERLAPPEDWINDOW,
                             win->rect.left,win->rect.top,
                             win->rect.right-win->rect.left+1,
                             win->rect.bottom-win->rect.top+1,
                             parent->handle,NULL,0,NULL);
    */
    ShowWindow(win->handle,SW_SHOW);
    UpdateWindow(win->handle);
#else
    win->handle=NULL;
#endif
    }


/*
** Initialize and display k2pdfopt overlay dialog box.
*/
void k2gui_osdep_overlay_init(K2CONVBOX *k2ol0,WILLUSGUIWINDOW *win,WILLUSGUIWINDOW *parent,
                              void *hinst,int rgbcolor)

    {
#ifdef MSWINGUI
    static char *classname="k2gui_overlay";
    static char *title="K2pdfopt: Creating overlay...";
    static int class_registered=0;

    k2gui_overlay=k2ol0;
    /* Register class only once */
    if (!class_registered)
        {
        WNDCLASSEX  wndclass;
        static HBRUSH hbrush;

        hbrush = CreateSolidBrush(((rgbcolor&0xff0000)>>16)|(rgbcolor&0xff00)|((rgbcolor&0xff)<<16));
        wndclass.cbSize        = sizeof(wndclass);
        wndclass.style         = CS_SAVEBITS | CS_HREDRAW | CS_VREDRAW;
        wndclass.lpfnWndProc   = (WNDPROC)k2mswingui_overlay_process_messages;
        wndclass.cbClsExtra    = 0;
        wndclass.cbWndExtra    = 0;
        wndclass.hInstance     = (HINSTANCE)hinst;
        wndclass.hIcon         = NULL;
        wndclass.hCursor       = NULL;
        wndclass.hbrBackground = hbrush;
        wndclass.lpszMenuName  = NULL;
        wndclass.lpszClassName = classname;
        wndclass.hIconSm       = NULL;
        RegisterClassEx(&wndclass);
        class_registered=1;
#if (WILLUSDEBUGX & 0x2000)
printf("Class registered.\n");
#endif
        }
    /*
    ** This handle actually gets assigned earlier--in k2mswingui_process_messages(),
    ** otherwise it is too late for some of the functions that need it, e.g.
    ** k2gui_osdep_window_menus_init().
    */
    k2gui_overlay->mainwin.handle=NULL;
/*
printf("@(%d,%d), %d x %d\n",win->rect.left,win->rect.top,win->rect.right-win->rect.left+1,
win->rect.bottom-win->rect.top+1);
*/

    /*
    ** v2.02:  Use WS_OVERLAPPEDWINDOW instead of WS_OVERLAPPED so that we get a close
    **         and a minimize button.
    */
    win->handle = (void*)CreateWindow(classname,title,WS_OVERLAPPEDWINDOW,
                             win->rect.left,win->rect.top,
                             win->rect.right-win->rect.left+1,
                             win->rect.bottom-win->rect.top+1,
                             parent->handle,NULL,0,NULL);
    /*
    win->handle = (void*)CreateWindowEx(WS_EX_TOPMOST,classname,title,WS_OVERLAPPEDWINDOW,
                             win->rect.left,win->rect.top,
                             win->rect.right-win->rect.left+1,
                             win->rect.bottom-win->rect.top+1,
                             parent->handle,NULL,0,NULL);
    */
    ShowWindow(win->handle,SW_SHOW);
    UpdateWindow(win->handle);
#else
    win->handle=NULL;
#endif
    }


/*
** If changing is non-zero, the user is re-sizing the window.
*/
void k2gui_osdep_main_repaint(int changing)

    {
#ifdef MSWINGUI
    PAINTSTRUCT ps;
    BeginPaint((HWND)k2gui->mainwin.handle,&ps);
    /* Invalidate the entire window */
    /*
    ** Calling InvalidateRect() will cause all child windows to automatically
    ** re-draw if necessary during a move.  Response is faster if you don't do this.
    ** (Child windows will re-draw after the move completes.)
    */
    /* InvalidateRect((HWND)k2gui->mainwin.handle,NULL,0); */
    k2gui_main_repaint(changing);
    EndPaint((HWND)k2gui->mainwin.handle,&ps);
#endif
    }


/*
** MS-Windows specific support functions (callback procedures)
*/
#ifdef MSWINGUI
LRESULT CALLBACK k2mswingui_cbox_process_messages(HWND hwnd,UINT iMsg,WPARAM wParam,LPARAM lParam)

    {
#if (WILLUSDEBUGX & 0x2000000)
if (iMsg!=WM_MOUSEFIRST && iMsg!=WM_SETCURSOR && iMsg!=WM_NCHITTEST 
                        && iMsg!=WM_NCMOUSEMOVE && iMsg!=WM_TIMER)
k2dprintf("CONVERT hwnd=%p, iMsg = 0x%X, wParam=0x%X, lParam=0x%X\n",(void *)hwnd,(int)iMsg,(int)wParam,(int)lParam);
#endif
    if (k2gui_cbox->mainwin.handle==NULL)
        k2gui_cbox->mainwin.handle=hwnd;
    switch (iMsg)
        {
        case WM_CREATE:
            return(0);
        /* v2.02:  Echo any minimize/maximize/restore click to the main window also. */
        case WM_SYSCOMMAND:
            if (wParam==SC_MINIMIZE)
                ShowWindow((HWND)k2gui->mainwin.handle,SW_MINIMIZE);
            else if (wParam==SC_MAXIMIZE)
                ShowWindow((HWND)k2gui->mainwin.handle,SW_MAXIMIZE);
            else if (wParam==SC_RESTORE)
                ShowWindow((HWND)k2gui->mainwin.handle,SW_RESTORE);
            break;
        /* v2.02:  Limit max size of dialog box to the original size */
        case WM_GETMINMAXINFO:
            {
            MINMAXINFO *mmi;
            mmi=(MINMAXINFO *)lParam;
            mmi->ptMaxSize.x = k2gui_cbox->mainwin.rect.right - k2gui_cbox->mainwin.rect.left+1;
            mmi->ptMaxSize.y = k2gui_cbox->mainwin.rect.bottom - k2gui_cbox->mainwin.rect.top+1;
            return(0);
            }
        case WM_DRAWITEM:
            {
            int buttonid;

            buttonid=(int)wParam;
            if (buttonid>=3 && buttonid<=5)
                {
                LPDRAWITEMSTRUCT lpdis;
                int buttoncolor,cav,textcolor;
                
                buttoncolor=k2gui_cbox->control[buttonid-2].color;
                lpdis=(LPDRAWITEMSTRUCT)((size_t)lParam);
                cav=(0.11*((buttoncolor&0xff0000)>>16)
                       + 0.59*((buttoncolor&0xff00)>>8)
                       + 0.3*(buttoncolor&0xff));
                textcolor = cav>128 ? 0 : 0xffffff;
                winmbox_button_draw((void *)lpdis->hDC,(void *)&lpdis->rcItem,
                                     lpdis->itemState,buttoncolor,(void *)k2gui_cbox->bf.handle,
                                     k2gui_cbox->control[buttonid-2].name,textcolor);
                }
            return(1);
            }
        case WM_COMMAND:
/*
willusgui_dprintf(ANSI_WHITE "CONVERT WM_COMMAND: iMsg=%d, wParam=%d(lo),%d(hi), lParam=%d\n",(int)iMsg,(int)LOWORD(wParam),(int)HIWORD(wParam),(int)lParam);
*/
            /* Focus moved to "convert" button? */
            if (LOWORD(wParam)==10 && HIWORD(wParam)==EN_SETFOCUS)
                {
                k2gui_cbox_draw_defbutton_border(1);
                break;
                }
            /* Focus moved away from "convert" button? */
            if (LOWORD(wParam)==10 && HIWORD(wParam)==EN_KILLFOCUS)
                {
                k2gui_cbox_draw_defbutton_border(0);
                break;
                }
            if (LOWORD(wParam)>=0 && LOWORD(wParam)<=5)
                {
                k2gui_cbox->status=LOWORD(wParam);
                /* ESC press?  If so, status = -1 */
                if (k2gui_cbox->status==2)
                    k2gui_cbox->status=-1;
                /* Enter press? */
                else if (k2gui_cbox->status==1)
                    {
                    /* If abort/done button, status=1 */
                    if (GetFocus()==k2gui_cbox->control[1].handle)
                        k2gui_cbox->status=1;
                    /*
                    else if (GetFocus()==k2gui_cbox->b2_hwnd)
                        k2gui_cbox->status=2;
                    else if (GetFocus()==k2gui_cbox->b3_hwnd)
                        k2gui_cbox->status=3;
                    */
                    else if (k2gui_cbox->control[1].name[0]=='*')
                        k2gui_cbox->status=1;
                    /*
                    else if (k2gui_cbox->b2[0]=='*')
                        k2gui_cbox->status=2;
                    else if (k2gui_cbox->b3[0]=='*')
                        k2gui_cbox->status=3;
                    */
                    else
                        k2gui_cbox->status=4;
                    }
                else
                    /* Button press.  Status = button index pressed. */
                    k2gui_cbox->status -= 2;
#if (WILLUSDEBUGX & 0x2000000)
k2dprintf("\nCONVERT MSG: k2gui_cbox->status=%d\n\n",k2gui_cbox->status);
#endif
                if (k2gui_cbox->status!=2 && k2gui_cbox->status!=3)
                    {
                    SendMessage(hwnd,WM_CLOSE,0,0);
                    return(0);
                    }
                else if (k2gui_cbox->status==2)
                    k2gui_cbox_open_files();
                else if (k2gui_cbox->status==3)
                    k2gui_cbox_open_folders();
                }
            break;
        /*
        case WM_QUIT:
            k2gui_cbox->status=-1;
            k2gui_cbox_destroy();
            return(0);
        */
        case WM_CLOSE:
            k2gui_cbox_destroy();
            return(0);
        case WM_DESTROY:
            return(0);
        }
    return(DefWindowProc(hwnd,iMsg,wParam,lParam));
    }

/*
** Handle keystrokes / button presses to the overlay window.
** Bitmap stuff is handled by the BITMAP_CONTROL in wgui.c
**
*/
LRESULT CALLBACK k2mswingui_overlay_process_messages(HWND hwnd,UINT iMsg,WPARAM wParam,LPARAM lParam)

    {
#if (WILLUSDEBUGX & 0x2000000)
if (iMsg!=WM_MOUSEFIRST && iMsg!=WM_SETCURSOR && iMsg!=WM_NCHITTEST 
                        && iMsg!=WM_NCMOUSEMOVE && iMsg!=WM_TIMER)
willusgui_dprintf("OVERLAY: hwnd=%p, iMsg = 0x%X, wParam=0x%X, lParam=0x%X\n",(void*)hwnd,(int)iMsg,(int)wParam,(int)lParam);
#endif
/*
if (iMsg!=WM_MOUSEFIRST && iMsg!=WM_SETCURSOR && iMsg!=WM_NCHITTEST 
                        && iMsg!=WM_NCMOUSEMOVE && iMsg!=WM_TIMER
if (iMsg!=0xAE && iMsg!=0xC && iMsg!=0x20)
printf("OVERLAY iMsg = 0x%X, wParam=0x%X, lParam=0x%X\n",(int)iMsg,(int)wParam,(int)lParam);
*/
    if (k2gui_overlay->mainwin.handle==NULL)
        k2gui_overlay->mainwin.handle=hwnd;
    switch (iMsg)
        {
        case WM_CREATE:
            return(0);
        /* v2.02:  Echo any minimize/maximize/restore click to the main window also. */
        case WM_SYSCOMMAND:
            if (wParam==SC_MINIMIZE)
                ShowWindow((HWND)k2gui->mainwin.handle,SW_MINIMIZE);
            else if (wParam==SC_MAXIMIZE)
                ShowWindow((HWND)k2gui->mainwin.handle,SW_MAXIMIZE);
            else if (wParam==SC_RESTORE)
                ShowWindow((HWND)k2gui->mainwin.handle,SW_RESTORE);
            break;
        /* v2.02:  Limit max size of dialog box to the original size */
        case WM_GETMINMAXINFO:
            {
            MINMAXINFO *mmi;
            mmi=(MINMAXINFO *)lParam;
            mmi->ptMaxSize.x = k2gui_overlay->mainwin.rect.right - k2gui_overlay->mainwin.rect.left+1;
            mmi->ptMaxSize.y = k2gui_overlay->mainwin.rect.bottom - k2gui_overlay->mainwin.rect.top+1;
            return(0);
            }
        case WM_KEYDOWN:
            if (k2gui_overlay->control[0].rectmarked.left > -9000)
                {
                SendMessage((HWND)k2gui_overlay->control[0].handle,iMsg,wParam,lParam);
                return(0);
                }
            break;
        case WM_CHAR:
            /* NOTE: ESC or ENTER during overlay bitmap creation generate WM_COMMAND */
            if (wParam==0x1b) /* ESC */
                {
                /* Need to see if rectangle is drawn...if so, send ESC to bitmap window */
                if (k2gui_overlay->control[0].rectmarked.left > -9000)
                    {
                    /* Send ESC to bitmap control */
                    k2gui_overlay->status=0;
                    SendMessage((HWND)k2gui_overlay->control[0].handle,WM_CHAR,0x1b,0);
                    return(0);
                    }
                k2gui_overlay->status=-1;
                SendMessage(hwnd,WM_CLOSE,0,0);
                return(0);
                }
            else if (wParam==0xd || wParam=='s' || wParam=='S' || wParam=='M' || wParam=='m')
                {
                k2gui_overlay_store_margins(&k2gui_overlay->control[0]);
                k2gui_overlay->status=1;
                SendMessage(hwnd,WM_CLOSE,0,0);
                return(0);
                }
            else if (wParam=='r' || wParam=='R')
                {
                k2gui_overlay_reset_margins();
                k2gui_overlay->status=1;
                SendMessage(hwnd,WM_CLOSE,0,0);
                return(0);
                }
            else if (wParam=='c' || wParam=='C')
                {
                k2gui_overlay->status=-1;
                SendMessage(hwnd,WM_CLOSE,0,0);
                return(0);
                }
            else
                k2gui_overlay->status=0;
            if (k2gui_overlay->control[0].rectmarked.left > -9000)
                {
                SendMessage((HWND)k2gui_overlay->control[0].handle,iMsg,wParam,lParam);
                return(0);
                }
            break;
        case WM_DRAWITEM:
            {
            int buttonid;

            buttonid=(int)wParam;
            if (buttonid>=3 && buttonid<=5)
                {
                LPDRAWITEMSTRUCT lpdis;
                int buttoncolor,cav,textcolor;
                
                buttoncolor=k2gui_overlay->control[buttonid-2].color;
                lpdis=(LPDRAWITEMSTRUCT)((size_t)lParam);
                cav=(0.11*((buttoncolor&0xff0000)>>16)
                       + 0.59*((buttoncolor&0xff00)>>8)
                       + 0.3*(buttoncolor&0xff));
                textcolor = cav>128 ? 0 : 0xffffff;
                winmbox_button_draw((void *)lpdis->hDC,(void *)&lpdis->rcItem,
                                     lpdis->itemState,buttoncolor,(void *)k2gui_overlay->bf.handle,
                                     k2gui_overlay->control[buttonid-2].name,textcolor);
                }
            return(1);
            }
        case WM_COMMAND:
#if (WILLUSDEBUGX & 0x2000000)
willusgui_dprintf(ANSI_WHITE "OVERLAY WM_COMMAND: wParam=%d(LoWord),%d(HiWord), lParam=%d\n",(int)LOWORD(wParam),(int)HIWORD(wParam),(int)lParam);
#endif
            /*
            ** Don't really need to move button focus because
            ** (1) During overlay bitmap creation, there is only one button (abort), and
            ** (2) During rectangle drawing, the tab button is disabled.
            */
            /* Focus moved to "convert" button? */
            /*
            if (LOWORD(wParam)==10 && HIWORD(wParam)==EN_SETFOCUS)
                {
                k2gui_overlay_draw_defbutton_border(1);
                break;
                }
            */
            /* Focus moved away from "convert" button? */
            /*
            if (LOWORD(wParam)==10 && HIWORD(wParam)==EN_KILLFOCUS)
                {
                k2gui_overlay_draw_defbutton_border(0);
                break;
                }
            */
            /* If converting, there is only one option: Abort */
            if (k2gui_overlay->converting==1)
                {
                k2gui_overlay->status=0;
                SendMessage(hwnd,WM_CLOSE,0,0);
                return(0);
                }
            /*
            ** 3 = Select
            ** 4 = Reset Margins
            ** 5 = Cancel
            */
            if (LOWORD(wParam)>=0 && LOWORD(wParam)<=5)
                {
                k2gui_overlay->status=LOWORD(wParam);
                /* Store */
                if (k2gui_overlay->status==3)
                    {
                    k2gui_overlay->status=1;
                    k2gui_overlay_store_margins(&k2gui_overlay->control[0]);
                    SendMessage(hwnd,WM_CLOSE,0,0);
                    return(0);
                    }
                /* Reset margins */
                if (k2gui_overlay->status==4)
                    {
                    k2gui_overlay_reset_margins();
                    k2gui_overlay->status=1;
                    SendMessage(hwnd,WM_CLOSE,0,0);
                    return(0);
                    }
                /* Cancel */
                if (k2gui_overlay->status==5)
                    {
                    k2gui_overlay->status=-1;
                    SendMessage(hwnd,WM_CLOSE,0,0);
                    return(0);
                    }
                }
            break;
        /*
        case WM_QUIT:
            k2gui_overlay->status=-1;
            k2gui_overlay_destroy();
            return(0);
        */
        case WM_CLOSE:
            k2gui_overlay_destroy();
            return(0);
        case WM_DESTROY:
            return(0);
        }
#if (WILLUSDEBUGX & 0x2000000)
willusgui_dprintf("OVERLAY:  k2gui_overlay->status=%d\n",k2gui_overlay->status);
#endif
    return(DefWindowProc(hwnd,iMsg,wParam,lParam));
    }


LRESULT CALLBACK k2mswingui_process_messages(HWND hwnd,UINT iMsg,WPARAM wParam,LPARAM lParam)

    {
    WILLUSGUIMESSAGE *message,_message;
    static int gotmw=0;

#if (WILLUSDEBUGX & 0x2000000)
if (iMsg!=WM_MOUSEFIRST && iMsg!=WM_SETCURSOR && iMsg!=WM_NCHITTEST 
                        && iMsg!=WM_NCMOUSEMOVE && iMsg!=WM_TIMER)
k2dprintf(ANSI_CYAN "MAINWIN: hwnd=%p, iMsg = 0x%X, wParam=0x%X, lParam=0x%X" ANSI_NORMAL "\n",(void *)hwnd,(int)iMsg,(int)wParam,(int)lParam);
#endif
/*
#if (WILLUSDEBUGX & 0x4000)
if (iMsg==WM_CONTEXTMENU)
printf("ContextMenu.\n");
#endif
*/
    message=&_message;
    if (k2gui->mainwin.handle==NULL)
        k2gui->mainwin.handle=hwnd;
    if (k2gui->mainwin.handle == hwnd)
        message->control = &k2gui->mainwin;
    else
        {
        int i;
        for (i=0;i<k2gui->ncontrols;i++)
            {
            if (k2gui->control[i].handle == hwnd)
                break;
            if (k2gui->control[i].type==WILLUSGUICONTROL_TYPE_UPDOWN
                        && (k2gui->control[i].subhandle[0]==hwnd
                                || k2gui->control[i].subhandle[1]==hwnd))
                break;
            if (k2gui->control[i].type==WILLUSGUICONTROL_TYPE_UPDOWN2
                        && (k2gui->control[i].subhandle[0]==hwnd
                                || k2gui->control[i].subhandle[1]==hwnd
                                || k2gui->control[i].subhandle[2]==hwnd
                                || k2gui->control[i].subhandle[3]==hwnd))
                break;
            }
        if (i<k2gui->ncontrols)
            message->control = &k2gui->control[i];
        else
            message->control = &k2gui->mainwin;
        }
    /* Turn special keys into other actions */
    /* Ctrl-D or I into Menu selection for call to k2gui_display_info() */
    if (iMsg==WM_KEYDOWN)
        {
        if ((wParam&0xff)=='I' || ((wParam&0xff)=='D' && (wParam&0xc00)>0))
            {
            iMsg=WM_COMMAND;
            wParam=712;
            lParam=0;
            }
        else
            return(0);
        }
        
    switch (iMsg)
        {
//         case WM_GETICON:
// printf("Geticon: ftype=%s\n",wParam==ICON_BIG ? "big" : "small");
//             return(LoadIcon(k2wingui->hinst,"k2pdfopt"));
        case WM_RBUTTONDOWN:
            message->guiaction = WILLUSGUIACTION_RBUTTONDOWN;
            message->param[0] = LOWORD(lParam);
            message->param[1] = HIWORD(lParam);
            k2gui_process_message(message);
            return(0);
            break;
        case WM_MOUSEWHEEL:
            {
            WILLUSGUICONTROL *control;
            int i;

            if (gotmw)
                break;
            gotmw=1;
            control=NULL;
            if (k2gui==NULL)
                break;
            for (i=0;i<k2gui->ncontrols;i++)
                if (!strcmp(k2gui->control[i].name,"previewwin"))
                    {
                    control=&k2gui->control[i];
                    break;
                    }
            if (control!=NULL && control->handle!=NULL)
                willusgui_sbitmap_proc(control->handle,(int)iMsg,(int)wParam,(void *)lParam);
            gotmw=0;
            break;
            }
        case WM_DRAWITEM:
            {
            int controlid;
            int subid;
            static char *ud2text[]={"_dleft_","_left_","_right_","_dright_"};

            controlid=LOWORD(wParam)-100;
            subid=-1;
            while (controlid >= 100)
                {
                controlid-=100;
                subid++;
                }
/*
#if (WILLUSDEBUGX & 0x4000)
printf("DRAWITEM: controlid=%d of %d\n",controlid,k2gui->ncontrols);
printf("      name = %s\n",k2gui->control[controlid].name);
printf("      type = %d, subid = %d\n",k2gui->control[controlid].type,subid);
#endif
*/
            if (controlid>=0 && controlid<k2gui->ncontrols 
                       && (k2gui->control[controlid].type==WILLUSGUICONTROL_TYPE_BUTTON 
                            || k2gui->control[controlid].type==WILLUSGUICONTROL_TYPE_CHECKBOX
                            || subid>=0))
                {
                LPDRAWITEMSTRUCT lpdis;
                int cav,textcolor,checkbox;
                char buttontext[32];

                checkbox = (k2gui->control[controlid].type==WILLUSGUICONTROL_TYPE_CHECKBOX);
                lpdis=(LPDRAWITEMSTRUCT)lParam;
                k2gui_control_select(controlid,lpdis->itemState&ODS_SELECTED);
/*
printf("       itemstate = %x\n",lpdis->itemState);
printf("       itemstate selected = %s\n",lpdis->itemState&ODS_SELECTED ? "YES" : "NO");
*/
                /*
                if ((lpdis->itemState&ODS_SELECTED) && buttonindex>=2 && buttonindex<=5)
                    {
                    buttondown=buttonindex;
                    bdtcount=0;
                    }
                else if (!(lpdis->itemState&ODS_SELECTED) && buttonindex>=2 && buttonindex<=5)
                    {
                    if (buttondown==buttonindex)
                        {
                        buttondown=0;
                        bdtcount=0;
                        }
                    }
                */
                cav=(0.11*((k2gui->control[controlid].color&0xff0000)>>16)
                       + 0.59*((k2gui->control[controlid].color&0xff00)>>8)
                       + 0.3*(k2gui->control[controlid].color&0xff));
                textcolor = cav>128 ? 0 : 0xffffff;
/*
printf("textcolor=%6x, ccolor=%6x, text='%s', fonthandle=%p\n",
textcolor,k2gui->control[controlid].color,
k2gui->control[controlid].text,
k2gui->control[controlid].font.handle);
*/
                if (k2gui->control[controlid].type == WILLUSGUICONTROL_TYPE_UPDOWN)
                    strcpy(buttontext,subid==0 ? "_up_" : "_down_");
                else if (k2gui->control[controlid].type == WILLUSGUICONTROL_TYPE_UPDOWN2)
                    strcpy(buttontext,ud2text[subid]);
                else /* if (checkbox) */
                    strcpy(buttontext,k2gui->control[controlid].label);
/*
                else 
                    strcpy(buttontext,k2gui->control[controlid].name);
*/
/*
printf("button_draw: %s\n",buttontext);
*/
                if (checkbox)
                    winmbox_checkbox_button_draw((void *)lpdis->hDC,
                                                 (void *)&lpdis->rcItem,
                                     lpdis->itemState,
                                     (void *)k2gui->control[controlid].font.handle,
                                     buttontext,textcolor,
                               k2gui->control[controlid].attrib&WILLUSGUICONTROL_ATTRIB_CHECKED,
                               &k2gui->bgbmp,k2gui->control[controlid].rect.left,
                                             k2gui->control[controlid].rect.top);
                else
                    winmbox_button_draw((void *)lpdis->hDC,(void *)&lpdis->rcItem,
                                     lpdis->itemState,k2gui->control[controlid].color,
                                     (void *)k2gui->control[controlid].font.handle,
                                     buttontext,textcolor);
                }
            return(1);
            }
        case WM_CREATE:
            message->guiaction = WILLUSGUIACTION_CREATE;
            k2gui_process_message(message);
            return(0);
        case WM_TIMER:
            k2gui_timer_event();
            return(0);
        case WM_DROPFILES:
            message->guiaction = WILLUSGUIACTION_DROPFILES;
            message->ptr[0] = (void *)wParam;
            k2gui_process_message(message);
            return(0);
/*
case WM_SETFOCUS:
printf("WM_SETFOCUS on main.\n");
break;
*/
        case WM_KILLFOCUS:
            message->guiaction = WILLUSGUIACTION_LOSTFOCUS;
            k2gui_process_message(message);
            return(0);
        case WM_COMMAND:
            {
            int code,child_id,i;
            HWND childwin;
            WILLUSGUICONTROL *control;

            code = HIWORD(wParam);
            child_id = LOWORD(wParam);
            childwin = (HWND)lParam;
            control=NULL;
            for (i=0;i<k2gui->ncontrols;i++)
                if (k2gui->control[i].handle == childwin)
                    {
                    control = &k2gui->control[i];
                    break;
                    }
/*
printf("k2gui_osdep WM_COMMAND, code=%d, cindex=%d, child_id=%d\n",code,control!=NULL?i:-1,child_id);
if (code==EN_KILLFOCUS || code==BN_KILLFOCUS || LBN_KILLFOCUS || CBN_KILLFOCUS)
printf("Got killfocus.\n");
*/
            /*
            ** Handle SETFOCUS.
            */
            if (code==EN_SETFOCUS || code==BN_SETFOCUS || code==LBN_SETFOCUS || code==CBN_SETFOCUS)
                 {
                 if (control!=NULL)
                     {
                     message->control=control;
                     message->guiaction=WILLUSGUIACTION_SETFOCUS;
                     k2gui_process_message(message);
                     }
                 return(DefWindowProc(hwnd,iMsg,wParam,lParam));
                 }

            /* 1 = ENTER PRESS TO MAIN WINDOW--Use GetFocus() to see which button has focus */
            /* 2 = ESC PRESS TO MAIN WINDOW */
            if (child_id==2)
                {
                message->guiaction = WILLUSGUIACTION_ESC_PRESS;
                k2gui_process_message(message);
                return(0);
                }
            if (child_id==1)
                {
                message->guiaction = WILLUSGUIACTION_ENTER_PRESS;
                k2gui_process_message(message);
                return(0);
                }
            if (childwin==NULL && child_id>=700 && child_id<=799)
                {
                message->guiaction=WILLUSGUIACTION_MENU_SELECTION;
                message->param[0]=child_id;
                k2gui_process_message(message);
                return(0);
                }
            control=NULL;
            /* Find the child window's control */
            for (i=0;i<k2gui->ncontrols;i++)
                {
                control = &k2gui->control[i];
                if (control->handle == childwin)
                    break;
                if (control->type == WILLUSGUICONTROL_TYPE_UPDOWN
                           && control->subhandle[0] == childwin)
                    {
                    message->control=control;
                    message->guiaction=WILLUSGUIACTION_UPDOWN_UP;
                    k2gui_process_message(message);
                    return(DefWindowProc(hwnd,iMsg,wParam,lParam));
                    }
                if (control->type == WILLUSGUICONTROL_TYPE_UPDOWN
                           && control->subhandle[1] == childwin)
                    {
                    message->control=control;
                    message->guiaction=WILLUSGUIACTION_UPDOWN_DOWN;
                    k2gui_process_message(message);
                    return(DefWindowProc(hwnd,iMsg,wParam,lParam));
                    }
                if (control->type == WILLUSGUICONTROL_TYPE_UPDOWN2
                           && control->subhandle[0] == childwin)
                    {
                    message->control=control;
                    message->guiaction=WILLUSGUIACTION_UPDOWN2_DLEFT;
                    k2gui_process_message(message);
                    return(DefWindowProc(hwnd,iMsg,wParam,lParam));
                    }
                if (control->type == WILLUSGUICONTROL_TYPE_UPDOWN2
                           && control->subhandle[1] == childwin)
                    {
                    message->control=control;
                    message->guiaction=WILLUSGUIACTION_UPDOWN2_LEFT;
                    k2gui_process_message(message);
                    return(DefWindowProc(hwnd,iMsg,wParam,lParam));
                    }
                if (control->type == WILLUSGUICONTROL_TYPE_UPDOWN2
                           && control->subhandle[2] == childwin)
                    {
                    message->control=control;
                    message->guiaction=WILLUSGUIACTION_UPDOWN2_RIGHT;
                    k2gui_process_message(message);
                    return(DefWindowProc(hwnd,iMsg,wParam,lParam));
                    }
                if (control->type == WILLUSGUICONTROL_TYPE_UPDOWN2
                           && control->subhandle[3] == childwin)
                    {
                    message->control=control;
                    message->guiaction=WILLUSGUIACTION_UPDOWN2_DRIGHT;
                    k2gui_process_message(message);
                    return(DefWindowProc(hwnd,iMsg,wParam,lParam));
                    }
                }
            if (i<k2gui->ncontrols)
                message->control = control;
            else
                control = message->control = &k2gui->mainwin;
            /*
            ** For drop-down lists, only process selection changes
            ** Let windows handle everything else.
            */
            if (control->type==WILLUSGUICONTROL_TYPE_DROPDOWNLIST
                      && code!=LBN_SELCHANGE && code!=CBN_SELCHANGE)
                return(DefWindowProc(hwnd,iMsg,wParam,lParam));
            /*
            ** For edit boxes, only process loss of focus.
            ** Let windows handle everything else.
            */
            if ((control->type==WILLUSGUICONTROL_TYPE_EDITBOX || control->type==WILLUSGUICONTROL_TYPE_UPDOWN
                    || control->type==WILLUSGUICONTROL_TYPE_UPDOWN2) && code!=EN_KILLFOCUS)
                return(DefWindowProc(hwnd,iMsg,wParam,lParam));
/*
            if (code==EN_KILLFOCUS)
                message->guiaction = WILLUSGUIACTION_LOSTFOCUS;
            else if (code==LBN_SELCHANGE || code==CBN_SELCHANGE)
{
printf("Selection change....\n");
                message->guiaction = WILLUSGUIACTION_SELECTION_CHANGE;
}
            else
*/
/*
printf("Treating as control press (id=%d)\n",child_id);
printf("code=%d, control->index=%d\n",code,control->index);
*/
            message->guiaction = WILLUSGUIACTION_CONTROL_PRESS;
            k2gui_process_message(message);
            return(DefWindowProc(hwnd,iMsg,wParam,lParam));
            }
        case WM_CONTEXTMENU:
            {
            int i;
            HWND childwin;
            WILLUSGUICONTROL *control;

            childwin = (HWND)wParam;
            control=NULL;
            for (i=0;i<k2gui->ncontrols;i++)
                if (k2gui->control[i].handle == childwin)
                    {
                    control = &k2gui->control[i];
                    break;
                    }
            if (control!=NULL)
                {
                message->control=control;
                message->guiaction = WILLUSGUIACTION_CONTEXTMENU;
                k2gui_process_message(message);
                }
            return(DefWindowProc(hwnd,iMsg,wParam,lParam));
            }
/*
        case WM_LBUTTONUP:
            return(testgui_handle_request(hwnd,-2,wParam,lParam));
break;
*/
        case WM_GETMINMAXINFO:
            {
            MINMAXINFO *mmi;

            message->guiaction = WILLUSGUIACTION_GETMINSIZE;
            k2gui_process_message(message);
            mmi=(MINMAXINFO *)lParam;
            mmi->ptMinTrackSize.x = message->param[0];
            mmi->ptMinTrackSize.y = message->param[1];
            return(0);
            }
        case WM_WINDOWPOSCHANGING:
        case WM_WINDOWPOSCHANGED:
            {
            WINDOWPOS *wpos;

            message->guiaction = WILLUSGUIACTION_WINDOWSIZECHANGE;
            wpos = (WINDOWPOS *)lParam;
            message->param[0] = wpos->cx;
            message->param[1] = wpos->cy;
            k2gui_process_message(message);
            wpos->cx = message->param[0];
            wpos->cy = message->param[1];
            break;
            }
        case WM_ENTERSIZEMOVE:
            message->guiaction = WILLUSGUIACTION_STARTING_RESIZE;
            k2gui_process_message(message);
            break;
        case WM_EXITSIZEMOVE:
            message->guiaction = WILLUSGUIACTION_ENDING_RESIZE;
            k2gui_process_message(message);
            break;
/*
        case WM_CONTEXTMENU:
            break;
        case WM_CHAR:
            if (wParam==0x1b || wParam=='x')
                {
                DestroyWindow(hwnd);
                return(0);
                }
*/
        /* WM_PAINT will paint whole window, so ignore "Erase Background" request. */
        case WM_ERASEBKGND:
            return(1);
/*
        case WM_MENUCOMMAND:
printf("menucommand.  hiword(wparam)=%x, loword(wparam)=%d\n",HIWORD(wParam),LOWORD(wParam));
printf("              lparam=%d\n",lParam);
            break;
*/
        case WM_PAINT:
#if (WILLUSDEBUGX & 0x2000)
printf("WM_PAINT...\n");
#endif
            message->guiaction = WILLUSGUIACTION_REPAINT;
            k2gui_process_message(message);
            break; /* Do NOT return 0 here--causes infinite loop. */
        /*
        ** Close should destroy the main window LAST, which will then send
        ** the WM_DESTROY message, which should post a quit() call to kill
        ** the messaging thread.
        */
        case WM_CLOSE:
#if (WILLUSDEBUGX & 0x2000000)
k2dprintf("    WM_CLOSE\n");
#endif
            message->guiaction = WILLUSGUIACTION_CLOSE;
            k2gui_process_message(message);
            return(0);
        case WM_DESTROY:
#if (WILLUSDEBUGX & 0x2000000)
k2dprintf("    WM_DESTROY\n");
#endif
            message->guiaction = WILLUSGUIACTION_DESTROY;
            k2gui_process_message(message);
            return(0);
        }
    return(DefWindowProc(hwnd,iMsg,wParam,lParam));
    }
#endif /* MSWINGUI */

#endif /* HAVE_K2GUI */
