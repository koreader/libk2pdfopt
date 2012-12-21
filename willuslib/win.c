/*
** win.c    Windows specific calls (they do nothing in other platforms)
**
** Part of willus.com general purpose C code library.
**
** Copyright (C) 2012  http://willus.com
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

#ifdef WIN32

#include <windows.h>
#include <tlhelp32.h>
#if (!defined(__DMC__))
#include <psapi.h>
#endif
/* #include <process.h> */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <time.h>
#include <conio.h>

static void win_launch_local(char *exename,char *cmdlineopts,int flags,
                             int cflags);
static DWORD WINAPI win_new_thread(LPVOID);
static LRESULT CALLBACK win_defcallback(HWND hwnd,UINT iMsg,WPARAM wParam,
                                        LPARAM lParam);

#ifdef DJWIN32
BOOL AttachThreadInput( DWORD idAttach, DWORD idAttachTo,
                        BOOL fAttach);
#endif
static int nextdir(char *dir,char *path,int *index);
static int setbase(char *dest,char *filename,char *ext);
static BOOL CALLBACK EnumWndFind(HWND h,LPARAM lp);
static BOOL CALLBACK findprocid(HWND hwnd,LPARAM lp);
static int get_desktop_directory_1(char *desktop,int maxlen,HKEY key_class,
                                   char *keyname);
static int win_registry_search1(char *value,int maxlen,HKEY key_class,char *keyname,char *searchvalue,int recursive);
static BOOL CALLBACK find_win_by_procid(HWND hwnd,LPARAM lp);

typedef struct
    {
    int used;
    int xpos;
    int ypos;
    int width;
    int height;
    int scrollbars;
    char title[512];
    void *callback;
    void *handle;
    WNDCLASSEX wndclass;
    HWND hwnd;
    HWND parent;
    HINSTANCE hinstance;
    MSG msg;
    int thread_id;
    void *newthread;
    int winflags;
    } NEWWIN;
#define MAXNEWWIN 4
static NEWWIN newwin[MAXNEWWIN];


char *win_full_exe_name(char *s)

    {
    static char exename[MAXFILENAMELEN];

    GetModuleFileNameA(NULL,exename,254);
    if (s!=NULL)
        {
        strcpy(s,exename);
        return(s);
        }
    return(exename);
    }


char *win_lasterror(void)

    {
    static char errbuf[256];

    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                  NULL,GetLastError(),
                  MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),errbuf,254,NULL);
    return(errbuf);
    }


/*
** Requires the user to have admin rights
** Grants full access to file by local administrators
** 0 = success
*/
int win_grant_full_file_access(char *filename)

    {
    PSID *sid;
    int status;
    int maxlen;
    int dlen;
    char buf1[256];
    char buf2[256];
    char domain[128];
    int stype;
    ACL *acl;
    SECURITY_DESCRIPTOR sd;

    maxlen=256;
    sid=(PSID *)buf1;
    acl=(ACL *)buf2;
    status=LookupAccountName(NULL,"Administrators",sid,(LPDWORD)&maxlen,domain,(LPDWORD)&dlen,
                              (PSID_NAME_USE)&stype);
    if (status==0)
        return(-1);
    // printf("status=%d, len=%d, domain='%s', dlen=%d, stype=%d\n",status,maxlen,domain,dlen,stype);
    maxlen=256;
    status=InitializeAcl(acl,maxlen,ACL_REVISION);
    if (status==0)
        return(-2);
    status=AddAccessAllowedAce(acl,ACL_REVISION,-1,sid);
    if (status==0)
        return(-3);
    status=InitializeSecurityDescriptor(&sd,SECURITY_DESCRIPTOR_REVISION);
    if (status==0)
        return(-4);
    status=SetSecurityDescriptorDacl(&sd,TRUE,acl,FALSE);
    if (status==0)
        return(-5);
    status=SetFileSecurity(filename,DACL_SECURITY_INFORMATION,&sd);
    if (status==0)
        return(-6);
    return(0);
    }

/*
** Should use an absolute path for the plotfile name.
*/
void win_launch(char *exename,char *cmdlineopts)

    {
    win_launch_local(exename,cmdlineopts,10,1);
    }


void win_launch_detail(char *exename,char *cmdlineopts,int showstyle,
                       int priority)

    {
    int      cflags;

    if (priority==0)
        cflags=IDLE_PRIORITY_CLASS;
    else if (priority==2)
        cflags=HIGH_PRIORITY_CLASS;
    else if (priority==3)
        cflags=REALTIME_PRIORITY_CLASS;
    else
        cflags=NORMAL_PRIORITY_CLASS;
/* showstyle values
#define SW_HIDE             0
#define SW_SHOWNORMAL       1
#define SW_NORMAL           1
#define SW_SHOWMINIMIZED    2
#define SW_SHOWMAXIMIZED    3
#define SW_MAXIMIZE         3
#define SW_SHOWNOACTIVATE   4
#define SW_SHOW             5
#define SW_MINIMIZE         6
#define SW_SHOWMINNOACTIVE  7
#define SW_SHOWNA           8
#define SW_RESTORE          9
#define SW_SHOWDEFAULT      10
#define SW_MAX              10
*/
    win_launch_local(exename,cmdlineopts,showstyle,cflags);
    }


static void win_launch_local(char *exename,char *cmdlineopts,int flags,
                             int cflags)

    {
    int     ct;
    char   *cp;
    static char cmdline[500];
    STARTUPINFO         gsi;
    PROCESS_INFORMATION gpi;

    /* Zero the init structures */
    for (ct=0,cp=(char *)&gpi;ct<sizeof(PROCESS_INFORMATION);cp[ct]=0,ct++);
    for (ct=0,cp=(char *)&gsi;ct<sizeof(STARTUPINFO);        cp[ct]=0,ct++);
    gsi.cb      = sizeof(STARTUPINFO);
    gsi.dwX     = 0; /* Ignored unless gsi.dwFlags |= STARTF_USEPOSITION */
    gsi.dwY     = 0;
    gsi.dwXSize = 0; /* Ignored unless gsi.dwFlags |= STARTF_USESIZE */
    gsi.dwYSize = 0;
    gsi.dwFlags = STARTF_USESHOWWINDOW;
    gsi.wShowWindow=flags;
    sprintf(cmdline,"\"%s\" %s",exename,cmdlineopts);
    CreateProcess(exename,cmdline,0,0,0,cflags|DETACHED_PROCESS,0,0,&gsi,&gpi);
    }


#define MAXPROCESSES 32
static PROCESS_INFORMATION gpi[MAXPROCESSES];
static STARTUPINFO gsi[MAXPROCESSES];
static int gpii=-1;

int process_launch(char *command,char *cmdlineopts,int inherits,
                    int detached,char *pwd,int flags)

    {
    int pnum;

    return(process_launch_ex(command,cmdlineopts,inherits,detached,pwd,
                             flags,&pnum));
    }


void process_close_handles(int index)

    {
    if (gsi[index].hStdInput!=0)
        {
        CloseHandle(gsi[index].hStdInput);
        gsi[index].hStdInput=0;
        }
    if (gsi[index].hStdOutput!=0)
        {
        CloseHandle(gsi[index].hStdOutput);
        gsi[index].hStdOutput=0;
        }
    if (gsi[index].hStdError!=0)
        {
        CloseHandle(gsi[index].hStdError);
        gsi[index].hStdError=0;
        }
    }


int process_launch_ex_ii(char *command,char *cmdlineopts,int inherits,
                         int detached,char *pwd,int flags,int *pnum,
                         char *stdinfile,char *stdoutfile,char *stderrfile)

    {
    int     i,status;
    static char cmdline[MAXFILENAMELEN];
    static char exename[MAXFILENAMELEN];
    SECURITY_ATTRIBUTES sa;

    if (win_which(exename,command)==0)
        return(0);
    if (gpii<0)
        gpii=0;
    i=gpii;
    gpii = (gpii+1)%MAXPROCESSES;
    (*pnum)=-1;
    /* Zero the init structures */
    memset(&gpi[i],0,sizeof(PROCESS_INFORMATION));
    memset(&gsi[i],0,sizeof(STARTUPINFO));
    gsi[i].cb      = sizeof(STARTUPINFO);
    gsi[i].dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    gsi[i].wShowWindow = flags;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor=NULL;
    sa.bInheritHandle=TRUE;
    if (stdinfile!=NULL && stdinfile[0]!='\0')
        gsi[i].hStdInput = CreateFile(stdinfile,GENERIC_READ,FILE_SHARE_READ,
                              &sa,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
    if (stdoutfile!=NULL && stdoutfile[0]!='\0')
        gsi[i].hStdOutput = CreateFile(stdoutfile,GENERIC_WRITE,FILE_SHARE_READ,
                              &sa,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
    if (stderrfile!=NULL && stderrfile[0]!='\0')
        gsi[i].hStdError = CreateFile(stderrfile,GENERIC_WRITE,FILE_SHARE_READ,
                              &sa,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
    sprintf(cmdline,"\"%s\"%s%s",exename,cmdlineopts[0]=='\0'?"":" ",
                                 cmdlineopts);
    status=CreateProcess(exename,cmdline,0,0,TRUE,
                         detached?DETACHED_PROCESS:0,
                         0,(pwd!=NULL && pwd[0]=='\0') ? NULL : pwd,
                         &gsi[i],&gpi[i]);
    if (!status)
        return(status);
    (*pnum)=i;
    return(gpi[i].dwProcessId);
    }

int process_launch_ex(char *command,char *cmdlineopts,int inherits,
                      int detached,char *pwd,int flags,int *pnum)

    {
    int     i,status;
    static char cmdline[MAXFILENAMELEN];
    static char exename[MAXFILENAMELEN];

    if (win_which(exename,command)==0)
        return(0);
    if (gpii<0)
        gpii=0;
    i=gpii;
    gpii = (gpii+1)%MAXPROCESSES;
    (*pnum)=-1;
    /* Zero the init structures */
    memset(&gpi[i],0,sizeof(PROCESS_INFORMATION));
    memset(&gsi[i],0,sizeof(STARTUPINFO));
    gsi[i].cb      = sizeof(STARTUPINFO);
    gsi[i].dwX     = 0; /* Ignored unless gsi.dwFlags |= STARTF_USEPOSITION */
    gsi[i].dwY     = 0;
    gsi[i].dwXSize = 0; /* Ignored unless gsi.dwFlags |= STARTF_USESIZE */
    gsi[i].dwYSize = 0;
    gsi[i].dwFlags = STARTF_USESHOWWINDOW;
    gsi[i].wShowWindow = flags;

    sprintf(cmdline,"\"%s\" %s",exename,cmdlineopts);
    status=CreateProcess(exename,cmdline,0,0,inherits,
                         detached?DETACHED_PROCESS:0,
                         0,(pwd!=NULL && pwd[0]=='\0') ? NULL : pwd,
                         &gsi[i],&gpi[i]);
    if (!status)
        return(status);
    (*pnum)=i;
    return(gpi[i].dwProcessId);
    }


int detail_process(char *exename,char *cmdlineopts,int inherits,
                   int swflags,int dwflags,int cflags,char *pwd)

    {
    int     i;
    static char cmdline[500];

    if (gpii<0)
        gpii=0;
    i=gpii;
    gpii = (gpii+1)%MAXPROCESSES;
    /* Zero the init structures */
    memset(&gpi[i],0,sizeof(PROCESS_INFORMATION));
    memset(&gsi[i],0,sizeof(STARTUPINFO));
    gsi[i].cb      = sizeof(STARTUPINFO);
    gsi[i].dwX     = 0; /* Ignored unless gsi.dwFlags |= STARTF_USEPOSITION */
    gsi[i].dwY     = 0;
    gsi[i].dwXSize = 0; /* Ignored unless gsi.dwFlags |= STARTF_USESIZE */
    gsi[i].dwYSize = 0;
    gsi[i].wShowWindow = swflags;
    gsi[i].dwFlags = dwflags;
    sprintf(cmdline,"\"%s\" %s",exename,cmdlineopts);
    return(CreateProcess(exename,cmdline,0,0,inherits,cflags,0,pwd,
                         &gsi[i],&gpi[i]));
    }


int process_done(int *exitcode)

    {
    return(process_done_ex(-1,exitcode));
    }


int process_done_ex(int procnum,int *exitcode)

    {
    int status,i;
    DWORD   altstatus;

    if (procnum<0 && gpii<0)
        return(1);
    i=procnum<0 ? (gpii+MAXPROCESSES-1)%MAXPROCESSES : procnum%MAXPROCESSES;
    status=GetExitCodeProcess(gpi[i].hProcess,&altstatus);
    (*exitcode)=altstatus;
    return(!status || (*exitcode)!=259);
    }


int win_terminate_process(int procnum)

    {
    int i,excode,status;

    if (gpii<0)
        return(-1);
    if (process_done_ex(procnum,&excode))
        return(-1);
    i=procnum<0 ? (gpii+MAXPROCESSES-1)%MAXPROCESSES : procnum%MAXPROCESSES;
    status=TerminateProcess(gpi[i].hProcess,10);
    if (status)
        return(0);
    return(-2);
    }


void *win_process_handle(void)

    {
    int i;

    if (gpii<0)
        return(0);
    i=(gpii+MAXPROCESSES-1)%MAXPROCESSES;
    return((void *)gpi[i].hProcess);
    }


void win_sleep(int ms)

    {
    Sleep(ms);
    }


int win_setdir(char *directory)

    {
    return(SetCurrentDirectory(directory));
    }


/*
** Play a metafile into a bitmap.
**
**
** Returns 0 for OK
**         1 for metafile not found
**
*/
wmetafile *win_emf_clipboard(void)

    {
    HENHMETAFILE    hemf;

    if (!OpenClipboard(GetDesktopWindow()))
        return(NULL);
    hemf=(HENHMETAFILE)GetClipboardData(CF_ENHMETAFILE);
    return((wmetafile *)hemf);
    }


/*
** Followed example at this link:
** http://msdn.microsoft.com/library/default.asp?url=/library/en-us/winui/WinUI/WindowsUserInterface/DataExchange/Clipboard/UsingtheClipboard.asp#_win32_Copying_Information_to_the_Clipboard
**
*/
int win_text_file_to_clipboard(char *filename,FILE *out)

    {
    FILE *f;
    char *buf,*p;
    int size;
    static char *funcname="win_text_file_to_clipboard";

    f=fopen(filename,"rb");
    if (f==NULL)
        {
        nprintf(out,"Cannot open file %s to put to clipboard.\n",filename);
        return(-1);
        }
    fseek(f,0L,2);
    size=ftell(f);
    fseek(f,0L,0);
    if (!willus_mem_alloc((double **)&buf,size+1,funcname))
        {
        fclose(f);
        nprintf(out,"Cannot allocate memory to put file %s to clipboard.\n",filename);
        return(-2);
        }
    p=GlobalLock(buf);
    if (fread(p,1,size,f)<size)
        {
        GlobalUnlock(buf);
        willus_mem_free((double **)&buf,funcname);
        fclose(f);
        nprintf(out,"Error reading file %s to memory.\n",filename);
        return(-3);
        }
    fclose(f);
    p[size]='\0';
    GlobalUnlock(buf);
    if (!OpenClipboard(GetDesktopWindow()))
        {
        willus_mem_free((double **)&buf,funcname);
        nprintf(out,"Error opening clipboard for file %s.\n",filename);
        return(-4);
        }
    EmptyClipboard();
    if (!SetClipboardData(CF_TEXT,buf))
        {
        willus_mem_free((double **)&buf,funcname);
        nprintf(out,"Error putting file %s data to clipboard.\n",filename);
        return(-5);
        }
    CloseClipboard();
    /*  Don't free the memory.  The Clipboard will do it when it empties it. */
    /*
    willus_mem_free((double **)&buf,funcname);
    */
    return(0);
    }


/*
** Followed example at this link:
** http://msdn.microsoft.com/library/default.asp?url=/library/en-us/winui/WinUI/WindowsUserInterface/DataExchange/Clipboard/UsingtheClipboard.asp#_win32_Copying_Information_to_the_Clipboard
**
*/
int win_buf_to_clipboard(char *lbuf,FILE *out)

    {
    char *buf,*p;
    int size;
    static char *funcname="win_buf_to_clipboard";

    size=strlen(lbuf);
    if (!willus_mem_alloc((double **)&buf,size+1,funcname))
        {
        nprintf(out,"Cannot allocate memory to put text to clipboard.\n");
        return(-1);
        }
    p=GlobalLock(buf);
    strcpy(p,lbuf);
    GlobalUnlock(buf);
    if (!OpenClipboard(GetDesktopWindow()))
        {
        willus_mem_free((double **)&buf,funcname);
        nprintf(out,"Error opening clipboard for text put.\n");
        return(-2);
        }
    EmptyClipboard();
    if (!SetClipboardData(CF_TEXT,buf))
        {
        willus_mem_free((double **)&buf,funcname);
        nprintf(out,"Error putting text data to clipboard.\n");
        return(-3);
        }
    CloseClipboard();
    /*  Don't free the memory.  The Clipboard will do it when it empties it. */
    return(0);
    }


/*
** Followed example at this link:
** http://msdn.microsoft.com/library/default.asp?url=/library/en-us/winui/WinUI/WindowsUserInterface/DataExchange/Clipboard/UsingtheClipboard.asp#_win32_Copying_Information_to_the_Clipboard
**
*/
char *win_clipboard_to_buf(FILE *out)

    {
    char *buf;
    static char *funcname="win_clipboard_to_buf";

    if (!OpenClipboard(GetDesktopWindow()))
        {
        willus_mem_free((double **)&buf,funcname);
        nprintf(out,"Error opening clipboard.\n");
        return(NULL);
        }
    return((char *)GetClipboardData(CF_TEXT));
    }


int win_clipboard_has_bitmap(void)

    {
    return(IsClipboardFormatAvailable(CF_BITMAP));
    }


int win_clipboard_has_text(void)

    {
    return(IsClipboardFormatAvailable(CF_TEXT));
    }


/*
** The handle returned by this function should ONLY be passed
** to win_emf_into_emf() as the destination file, or to
** win_emf_close_created_metafile().
*/
wmetafile *win_emf_create(double width_in,double height_in)

    {
    RECT rect;
    HDC  hdc;

    rect.left        = 0.;
    rect.right       = rect.left + width_in*2540.;
    rect.top         = 0.;
    rect.bottom      = rect.top + height_in*2540.;
    hdc = CreateEnhMetaFile(NULL,NULL,width_in<0 ? NULL : &rect,NULL);
    return((wmetafile *)hdc);
    }


/*
** This MUST be called before you can do any other wmetafile
** function other than win_emf_into_emf().
*/
wmetafile *win_emf_close_created_metafile(wmetafile *wmf)

    {
    HDC hdc;
    HENHMETAFILE    hemf;

    hdc=(HDC)wmf;
    hemf = CloseEnhMetaFile(hdc);
    /* DeleteDC(hdc) doesn't seem to be necessary--the call fails if tried. */
    return((wmetafile *)hemf);
    }


void win_clipboard_close(void)

    {
    CloseClipboard();
    }


void win_emf_clipboard_close(void)

    {
    CloseClipboard();
    }


/*
** Doesn't work as of 11-21-2008
*/
wmetafile *win_emf_from_metafile(char *metafile)

    {
    char *buf;
    HENHMETAFILE newbuf;
    int size,nr;
    static char *funcname="win_emf_from_file_ex";
    METAFILEPICT mfp;
    HDC hdc;
    void *vp;
    FILE *f;

    size=wfile_size(metafile);
    if (size<=0)
        return(NULL);
    willus_mem_alloc_warn(&vp,size,funcname,10);
    buf=(char *)vp;
    f=fopen(metafile,"rb");
    nr=fread(buf,1,size,f);
    fclose(f);
    if (nr<size)
        {
        willus_mem_free((double **)&buf,funcname);
        return(NULL);
        }
    mfp.mm=MM_ANISOTROPIC;
    mfp.xExt=mfp.yExt=0;
    mfp.hMF=(HMETAFILE)buf;
    hdc=CreateDC("WINSPOOL","WinSlideNT",NULL,NULL);
    newbuf=SetWinMetaFileBits(size,(CONST BYTE *)buf,hdc,&mfp);
    willus_mem_free((double **)&buf,funcname);
    if (hdc!=NULL)
        DeleteDC(hdc);
    return((wmetafile *)newbuf);
    }


wmetafile *win_emf_from_file(char *filename)

    {
    /*
    printf("win_emf_from_file not complete yet.\n");
    exit(20);
    */
    return((wmetafile *)GetEnhMetaFile(filename));
    }


int win_emf_write_to_file(wmetafile *wmf,char *filename)

    {
    FILE *f;
    int status;

    f=fopen(filename,"wb");
    if (f==NULL)
        return(-1);
    status=win_emf_write(wmf,f);
    if (status)
        return(status-1);
    fclose(f);
    return(0);
    }


int win_emf_write(wmetafile *wmf,FILE *f)

    {
    HENHMETAFILE hemf;
    char *x;
    int size,status;
    static char *funcname="win_emf_write";

    hemf=(HENHMETAFILE)wmf;
    size=GetEnhMetaFileBits(hemf,0,NULL);
    if (!willus_mem_alloc((double **)&x,size,funcname))
        return(-1);
    GetEnhMetaFileBits(hemf,size,(void *)x);
    status = (fwrite(x,1,size,f)<size);
    willus_mem_free((double **)&x,funcname);
    if (status)
        return(-2);
    return(0);
    }


void win_emf_close(wmetafile *wmf,int close_clipboard)

    {
    HENHMETAFILE hemf;

    hemf=(HENHMETAFILE)wmf;
    DeleteEnhMetaFile(hemf);
    if (close_clipboard)
        CloseClipboard();
    }


/*
** orientation = 0 (Portrait) or 1 (Landscape)
*/
int win_emf_write_prn(wmetafile *wmf,char *printer,char *psfile,
                      int *left,int *bottom,int *right,int *top)

    {
    HDC     hDC;
    RECT    rect;
    DOCINFO doc;
    HENHMETAFILE hemf;
    ENHMETAHEADER   header;
    double  page_width_in,page_height_in,page_width_pels,page_height_pels;
    double  page_hor_dpi,page_vert_dpi;
    double  image_width_in,image_height_in;
    double  margin_hor_in,margin_vert_in;

    hemf=(HENHMETAFILE)wmf;
    // Get size of print-out
    GetEnhMetaFileHeader(hemf,sizeof(ENHMETAHEADER),&header);


    hDC = CreateDC("WINSPOOL",printer,NULL,NULL);
    if (hDC==NULL)
        return(-1);

    page_width_in    = (double)GetDeviceCaps(hDC,HORZSIZE)/25.4;
    page_height_in   = (double)GetDeviceCaps(hDC,VERTSIZE)/25.4;
    page_width_pels  = GetDeviceCaps(hDC,HORZRES);
    page_height_pels = GetDeviceCaps(hDC,VERTRES);
    page_hor_dpi     = page_width_pels/page_width_in;
    page_vert_dpi    = page_height_pels/page_height_in;

    image_width_in   = (header.rclFrame.right-header.rclFrame.left)/2540.;
    image_height_in  = (header.rclFrame.bottom-header.rclFrame.top)/2540.;

    margin_hor_in    = (page_width_in-image_width_in)/2.;
    margin_vert_in   = (page_height_in-image_height_in)/2.;

    /* Metafile "Frame" is in hundredths of mm */
    /* PlayEnhMetaFile needs units of pels */
    /* Bounding box needs 72 dpi */

    rect.left        = margin_hor_in*page_hor_dpi;
    rect.right       = rect.left + image_width_in*page_hor_dpi;
    rect.top         = margin_vert_in*page_vert_dpi;
    rect.bottom      = rect.top + image_height_in*page_vert_dpi;

    (*left)          = 18+margin_hor_in*72.;
    (*right)         = (*left)+image_width_in*72.;
    (*bottom)        = 18+margin_vert_in*72.;
    (*top)           = (*bottom)+image_height_in*72.;


    // Set up document info
    doc.cbSize = sizeof(DOCINFO);
    doc.lpszDocName = "Windows EMF file converted to Postscript";
    doc.lpszOutput = psfile;
    doc.lpszDatatype = 0;
    doc.fwType = 0;

    // Start printing
    StartDoc(hDC,&doc);

    // Play the metafile into the device context
    PlayEnhMetaFile(hDC,hemf,&rect);

    // End printing
    EndDoc(hDC);
    DeleteDC(hDC);
    return(0);
    }


/*
** Play the source metafile into the destination context.
**
** dest must have been created using win_emf_create() function.
**
** dist_from_left_in = inches from left side of page where the left
**                     side of the injected image will start.
** dist_from_top_in  = inches from top of page where the top of the
**                     injected image will start.
**
** If either dist... values are < -900, then the image is auto-centered.
**
** scale_factor = 1.0 for actual size.
**
*/
int win_emf_into_emf(wmetafile *dest,wmetafile *src,
                     double dist_from_left_in,
                     double dist_from_top_in,
                     double scale_factor)

    {
    HDC     hDC;
    RECT    rect;
    HENHMETAFILE hemf;
    ENHMETAHEADER   header;
    double  page_width_in,page_height_in,page_width_pels,page_height_pels;
    double  page_hor_dpi,page_vert_dpi;
    double  image_width_in,image_height_in;

    hemf=(HENHMETAFILE)src;
    // Get size of print-out
    GetEnhMetaFileHeader(hemf,sizeof(ENHMETAHEADER),&header);

    // Set up bounding box: convert hundredths of mm to 300 dpi

    // Get device context
    hDC = (HDC)dest;
    if (hDC==NULL)
        return(-1);

    page_width_in    = (double)GetDeviceCaps(hDC,HORZSIZE)/25.4;
    page_height_in   = (double)GetDeviceCaps(hDC,VERTSIZE)/25.4;
    page_width_pels  = GetDeviceCaps(hDC,HORZRES);
    page_height_pels = GetDeviceCaps(hDC,VERTRES);
    page_hor_dpi     = page_width_pels/page_width_in;
    page_vert_dpi    = page_height_pels/page_height_in;

    image_width_in   = (header.rclFrame.right-header.rclFrame.left)/2540.;
    image_height_in  = (header.rclFrame.bottom-header.rclFrame.top)/2540.;

    if (dist_from_left_in < -900.)
        dist_from_left_in  = (page_width_in-image_width_in)/2.;
    if (dist_from_top_in < -900.)
        dist_from_top_in = (page_height_in-image_height_in)/2.;

    /* Metafile "Frame" is in hundredths of mm */
    /* PlayEnhMetaFile needs units of pels */
    /* Bounding box needs 72 dpi */

    rect.left        = dist_from_left_in*page_hor_dpi;
    rect.right       = rect.left + image_width_in*page_hor_dpi*scale_factor;
    rect.top         = dist_from_top_in*page_vert_dpi;
    rect.bottom      = rect.top + image_height_in*page_vert_dpi*scale_factor;

    // Set up document info
    /*
    doc.cbSize = sizeof(DOCINFO);
    doc.lpszDocName = "Windows EMF file";
    doc.lpszOutput = psfile;
    */
    // Start printing
    /*
    StartDoc(hDC,&doc);
    */

    // Play the metafile into the device context
    PlayEnhMetaFile(hDC,hemf,&rect);

    // End printing
    /*
    EndDoc(hDC);
    */

    /*
    DeleteDC(hDC);
    */
    return(0);
    }


int win_still_open(void *winptr)

    {
    int i;

    for (i=0;i<MAXNEWWIN;i++)
        if (newwin[i].used && newwin[i].handle==winptr)
            return(1);
    return(0);
    }


/*
** Start new thread for new window.
*/
void *win_create(char *title,int xpos,int ypos,int width,int height,
                 int scrollbars,void *callback)

    {
    static int callcount=0;
    int i;
    NEWWIN *nw;

    if (callcount==0)
        for (i=0;i<MAXNEWWIN;i++)
            newwin[i].used=0;
    callcount++;
    for (i=0;i<MAXNEWWIN;i++)
        if (newwin[i].used==0)
            break;
    if (i>=MAXNEWWIN)
        {
        printf("win_create() out of NEWWIN structures!  Max = %d.\n",MAXNEWWIN);
        exit(10);
        }
    nw=&newwin[i];
    nw->used = 1;
    nw->width = width;
    nw->height = height;
    nw->xpos = xpos;
    nw->ypos = ypos;
    nw->callback = callback;
    strncpy(nw->title,title,511);
    nw->title[511]='\0';
    nw->scrollbars = scrollbars;
    nw->parent = GetActiveWindow();
    nw->handle = NULL;

    /* Creat new thread which will create the window */
    nw->newthread=(void *)CreateThread(
                NULL, // Security attributes (NULL=default)
                0, // Stack size.  0 = use size of parent thread
                (LPTHREAD_START_ROUTINE)win_new_thread,
                (LPVOID)nw, // Passed to win_new_thread().
                0, // CreationFlags.  0 = run immediately
                (LPDWORD)&nw->thread_id);
    if (nw->newthread==NULL)
        {
        printf("Error creating new thread in win_create()!\n");
        exit(11);
        }
    /* Wait for handle to get assigned */
    while (nw->handle==NULL)
        win_sleep(100);
    return(nw->handle);
    }


static DWORD WINAPI win_new_thread(LPVOID arg)

    {
    NEWWIN *nw;

    nw=(NEWWIN *)arg;
    nw->hinstance=(HINSTANCE)GetModuleHandle(NULL);

    if (nw->width<0)
        nw->width=640;
    if (nw->height<0)
        nw->height=480;
    nw->wndclass.cbSize        = sizeof(nw->wndclass);
    nw->wndclass.style         = CS_HREDRAW | CS_VREDRAW;
    nw->wndclass.lpfnWndProc   = (WNDPROC)(nw->callback==NULL
                                 ? (void *)win_defcallback
                                 : (void *)nw->callback);
    nw->wndclass.cbClsExtra    = 0;
    nw->wndclass.cbWndExtra    = 0;
    nw->wndclass.hInstance     = nw->hinstance;
    nw->wndclass.hIcon         = NULL;
    nw->wndclass.hCursor       = NULL;
    nw->wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    nw->wndclass.lpszMenuName  = NULL;
    nw->wndclass.lpszClassName = nw->title;
    nw->wndclass.hIconSm       = NULL;
    RegisterClassEx(&nw->wndclass);
    nw->winflags = WS_OVERLAPPEDWINDOW|WS_SYSMENU;
    if (nw->scrollbars)
        nw->winflags |= WS_VSCROLL|WS_HSCROLL;
    nw->hwnd = CreateWindowEx(0L, // Extended style
                          nw->title, // Class name
                          nw->title, // Window name
                          nw->winflags, // Style flags
                          nw->xpos, // X position
                          nw->ypos, // Y position
                          nw->width, // Width (pels)
                          nw->height, // Height (pels)
                          nw->parent, // Parent window handle
                          NULL, // Menu handle
                          (HINSTANCE)nw->hinstance, // Module instance
                          NULL); // Passed to window through CREATESTRUCT
    ShowWindow(nw->hwnd,SW_SHOWNOACTIVATE);
    UpdateWindow(nw->hwnd);
    SetWindowText(nw->hwnd,nw->title);
    nw->handle=(void *)nw->hwnd;
    while (GetMessage(&nw->msg,NULL,0,0))
         {
         TranslateMessage(&nw->msg);
         DispatchMessage(&nw->msg);
         }
    nw->used=0;
    return(nw->msg.wParam);
    }


static LRESULT CALLBACK win_defcallback(HWND hwnd,UINT iMsg,WPARAM wParam,
                                        LPARAM lParam)

    {
    switch (iMsg)
        {
        case WM_CREATE:
            return(0);
        case WM_CHAR:
            if (wParam==0x1b)
                PostQuitMessage(0);
            return(0);
        case WM_DESTROY:
            PostQuitMessage(0);
            return(0);
        }
    return(DefWindowProc(hwnd,iMsg,wParam,lParam));
    }


void win_update(void *handle)

    {
    HWND    hwnd;

    hwnd=(HWND)handle;
    UpdateWindow(hwnd);
    }


void win_set_foreground(void *handle)

    {
    HWND    hwnd;

    hwnd=(HWND)handle;
    SetForegroundWindow(hwnd);
    }


void win_destroy(void *handle)

    {
    HWND  hwnd;

    hwnd=(HWND)handle;
    DestroyWindow(hwnd);
    PostQuitMessage(0);
    }


/*
** Returns -1 for can't get.
**          0 for low/idle
**          1 for normal
**          2 for high
**          3 for realtime
*/
int win_get_priority(void)

    {
    int     pri;

    pri=GetPriorityClass(GetCurrentProcess());
    if (pri==IDLE_PRIORITY_CLASS)
        pri=0;
    else if (pri==NORMAL_PRIORITY_CLASS)
        pri=1;
    else if (pri==HIGH_PRIORITY_CLASS)
        pri=2;
    else if (pri==REALTIME_PRIORITY_CLASS)
        pri=3;
    return(pri);
    }


/*
** See win_get_priority().  Returns 0 if can't set.
*/
int win_set_priority(int pri)

    {
    int     status;
    static int wpri[4]={IDLE_PRIORITY_CLASS,NORMAL_PRIORITY_CLASS,
                        HIGH_PRIORITY_CLASS,REALTIME_PRIORITY_CLASS};

    if (pri<0 || pri>3)
        return(0);
    status=SetPriorityClass(GetCurrentProcess(),wpri[pri]);
    return(status);
    }



/*
** Copy to file, including printer ports.
*/
int win_copy_file(char *destfile,char *srcfile)

    {
    long        size,sizeleft;
    int         blocksize;
    int         maxblock;
    static char buf[16384];
    FILE       *f;
    int         n,nw;
    HANDLE      h;

    h = CreateFile(destfile,GENERIC_WRITE,0,0,OPEN_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL,0);
    if (h==INVALID_HANDLE_VALUE)
        return(-1);
    maxblock = 16384;
    f=fopen(srcfile,"rb");
    if (f==NULL)
        return(-2);
    fseek(f,0L,2);
    size=ftell(f);
    fseek(f,0L,0);
    for (sizeleft=size;sizeleft>0;sizeleft -= blocksize)
        {
        if (sizeleft>maxblock)
            blocksize=maxblock;
        else
            blocksize=sizeleft;
        if ((n=fread(buf,1,blocksize,f))<blocksize)
            {
            fclose(f);
            CloseHandle(h);
            return(-3);
            }
        WriteFile(h,buf,n,(void *)&nw,0);
        if (nw!=n)
            {
            fclose(f);
            CloseHandle(h);
            return(-4);
            }
        }
    fclose(f);
    CloseHandle(h);
    return(0);
    }




int win_fileattr_to_wfile(int winattr)

    {
    int     s;

    s=0;
    if (winattr&FILE_ATTRIBUTE_ARCHIVE)
        s |= WFILE_ARCHIVE;
    if (winattr&FILE_ATTRIBUTE_DIRECTORY)
        s |= WFILE_DIR;
    if (winattr&FILE_ATTRIBUTE_HIDDEN)
        s |= WFILE_HIDDEN;
    if (winattr&FILE_ATTRIBUTE_READONLY)
        s |= WFILE_READONLY;
    if (winattr&FILE_ATTRIBUTE_SYSTEM)
        s |= WFILE_SYSTEM;
    return(s);
    }


void win_windate_to_tm(struct tm *filedate,void *wtime)

    {
    FILETIME    ltime;

    FileTimeToLocalFileTime((FILETIME *)wtime,&ltime);
    win_windate_to_tm_direct(filedate,&ltime);
    }


void win_windate_to_tm_direct(struct tm *filedate,void *wtime)

    {
    SYSTEMTIME  stime;
    time_t      t;
    struct tm   lt;

    FileTimeToSystemTime((FILETIME *)wtime,&stime);
    /* If the year is too far back, the mktime() will return a -1 and */
    /* that will cause the localtime() function to bomb.              */
    if (stime.wYear<1975)
        stime.wYear=1975;
    if (stime.wMonth<1 || stime.wMonth>12)
        stime.wMonth=1;
    if (stime.wDay<1 || stime.wDay>31)
        stime.wDay=1;
    if (stime.wHour>24)
        stime.wHour=0;
    if (stime.wMinute>59)
        stime.wMinute=0;
    if (stime.wSecond>59)
        stime.wSecond=0;
    if (stime.wDayOfWeek>6)
        stime.wDayOfWeek=-1;
    /* ANSI C 32-bit date structure can only handle dates up to 2036 */
    if (stime.wYear > 2036)
        {
        printf("Warning:  File date beyond 2036 in win_windate_to_tm_direct()!\n");
        stime.wYear=2036;
        }
    filedate->tm_sec  = stime.wSecond;
    filedate->tm_min  = stime.wMinute;
    filedate->tm_hour = stime.wHour;
    filedate->tm_mday = stime.wDay;
    filedate->tm_mon  = stime.wMonth-1;
    filedate->tm_year = stime.wYear-1900;
    filedate->tm_wday = stime.wDayOfWeek;    /* Sun = 0, Mon = 1, ... */
    filedate->tm_yday = -1;
    filedate->tm_isdst= -1;
/*
printf("date:  %d-%d-%d, %d:%d:%d (wday=%d)\n",
filedate->tm_year+1900,filedate->tm_mon+1,filedate->tm_mday,
filedate->tm_hour,filedate->tm_min,filedate->tm_sec,filedate->tm_wday);
*/
    t=mktime(filedate);
// printf("a\n");
    lt=(*localtime(&t));
// printf("b\n");
    filedate->tm_isdst= lt.tm_isdst;
    filedate->tm_yday = lt.tm_yday;
    }


int win_file_is_ntfs(char *filename)

    {
    char drive[16];
    char volname[64];
    char filesys[64];
    DWORD maxlen,flags;

    if (filename[0]!='\0' && filename[1]==':')
        sprintf(drive,"%c:\\",filename[0]);
    else
        {
        char *p;
        p=wfile_get_wd();
        sprintf(drive,"%c:\\",p[0]);
        }
    if (!GetVolumeInformation(drive,volname,63,NULL,&maxlen,&flags,filesys,63))
        return(-1);
    if (!stricmp(filesys,"ntfs"))
        return(1);
    return(0);
    }


/*
** Adjust for a bug in the way NTFS file dates are reported.
*/
void win_ntfs_date_to_proper_date(struct tm *date)

    {
    time_t t;
    struct tm lt;

    time(&t);
    lt=(*localtime(&t));
    if (date->tm_isdst && !lt.tm_isdst)
        wfile_increment_hour(date);
    else if (!date->tm_isdst && lt.tm_isdst)
        wfile_decrement_hour(date);
    }


/*
** Adjust for a bug in the way NTFS file dates are reported.
*/
void win_proper_date_to_ntfs_date(struct tm *date)

    {
    time_t t;
    struct tm lt;

    time(&t);
    lt=(*localtime(&t));
    if (date->tm_isdst && !lt.tm_isdst)
        wfile_decrement_hour(date);
    else if (!date->tm_isdst && lt.tm_isdst)
        wfile_increment_hour(date);
    }
    


void win_tm_to_windate(void *fT,struct tm *date)

    {
    FILETIME   *fTime,fLocTime;
    SYSTEMTIME  sysTime;

    fTime=(FILETIME *)fT;
    sysTime.wDay = date->tm_mday;
    sysTime.wMonth = date->tm_mon+1;
    sysTime.wYear = date->tm_year+1900;
    sysTime.wMilliseconds = 0;
    sysTime.wSecond = date->tm_sec;
    sysTime.wMinute = date->tm_min;
    sysTime.wHour = date->tm_hour;
    SystemTimeToFileTime(&sysTime,&fLocTime);
    LocalFileTimeToFileTime(&fLocTime,fTime);
    }


void win_file_windate_to_tm(struct tm *filedate,void *wtime,char *filename)

    {
    win_windate_to_tm(filedate,wtime);
    if (win_file_is_ntfs(filename))
        win_ntfs_date_to_proper_date(filedate);
    }


void win_file_windate_to_tm_direct(struct tm *filedate,void *wtime,char *filename)

    {
    win_windate_to_tm_direct(filedate,wtime);
    if (win_file_is_ntfs(filename))
        win_ntfs_date_to_proper_date(filedate);
    }


void win_tm_to_file_windate(void *fT,struct tm *dd,char *filename)

    {
    if (win_file_is_ntfs(filename))
        {
        struct tm  *date,_date;

        date=&_date;
        (*date) = (*dd);
        win_proper_date_to_ntfs_date(date);
        win_tm_to_windate(fT,date);
        }
    else
        win_tm_to_windate(fT,dd);
    }


void win_set_filetime(char *filename,struct tm *date)

    {
    HANDLE  h;
    FILETIME    fTime;

    h=CreateFile(filename,GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_ALWAYS,
                 FILE_ATTRIBUTE_NORMAL,NULL);
    if (h==INVALID_HANDLE_VALUE)
        return;
    win_tm_to_file_windate(&fTime,date,filename);
    SetFileTime(h,&fTime,&fTime,&fTime);
    CloseHandle(h);
    }


void win_set_mod_filetime(char *filename,struct tm *date)

    {
    HANDLE  h;
    FILETIME    fTime;

    h=CreateFile(filename,GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_ALWAYS,
                 FILE_ATTRIBUTE_NORMAL,NULL);
    if (h==INVALID_HANDLE_VALUE)
        {
        h=CreateFile(filename,GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_ALWAYS,
                 FILE_ATTRIBUTE_NORMAL|FILE_FLAG_BACKUP_SEMANTICS,NULL);
        if (h==INVALID_HANDLE_VALUE)
            {
            printf("win_set_mod_filetime:  invalid handle on '%s'\n",filename);
            return;
            }
        }
    win_tm_to_file_windate(&fTime,date,filename);
    if (!SetFileTime(h,NULL,NULL,&fTime))
        printf("win_set_mod_filetime:  SetFileTime fails.\n");
    CloseHandle(h);
    }


int win_most_recent_in_path(char *exactname,char *wildcard)

    {
    char *p;
    static char path[4096];
    static char dir[1024];
    static char file[1024];
    char  tfile[512];
    int   index;
    FILELIST *fl,_fl;

    fl=&_fl;
    filelist_init(fl);
    p=getenv("PATH");
    if (p==NULL)
        path[0]='\0';
    else
        strcpy(path,p);
    index=0;
    exactname[0]='\0';
    while (nextdir(dir,path,&index))
        {
        wfile_fullname(file,dir,wildcard);
        filelist_fill_from_disk_1(fl,file,0,0);
        if (fl->n<=0)
            {
            filelist_free(fl);
            continue;
            }
        filelist_sort_by_date(fl);
        wfile_fullname(tfile,fl->dir,fl->entry[fl->n-1].name);
        filelist_free(fl);
        if (exactname[0]=='\0' || wfile_newer(tfile,exactname)>0)
            strcpy(exactname,tfile);
        }
    return(exactname[0]!='\0');
    }


int win_which(char *exactname,char *exename)

    {
    char *p;
    static char basename[MAXFILENAMELEN];
    static char path[4096];
    static char dir[4096];
    static char file[4096];
    int   index;

    strcpy(basename,exename);
    if (basename[0]=='\"' && basename[strlen(basename)-1]=='\"')
        {
        basename[strlen(basename)-1]='\0';
        memmove(basename,&basename[1],strlen(basename));
        }
    p=getenv("PATH");
    if (p==NULL)
        path[0]='\0';
    else
        strcpy(path,p);
    if (setbase(exactname,basename,"com"))
        {
        if (wfile_status(exactname)==1)
            return(2);
        if (setbase(exactname,basename,"exe") && wfile_status(exactname)==1)
            return(3);
        if (setbase(exactname,basename,"bat") && wfile_status(exactname)==1)
            return(4);
        }
    if (wfile_status(basename)==1)
        {
        strcpy(exactname,basename);
        return(1);
        }
    index=0;
    while (nextdir(dir,path,&index))
        {
        if (setbase(exactname,basename,"com"))
            {
            wfile_fullname(file,dir,exactname);
            if (wfile_status(file)==1)
                {
                strcpy(exactname,file);
                return(5);
                }
            if (setbase(exactname,basename,"exe"))
                {
                wfile_fullname(file,dir,exactname);
                if (wfile_status(file)==1)
                    {
                    strcpy(exactname,file);
                    return(6);
                    }
                }
            if (setbase(exactname,basename,"bat"))
                {
                wfile_fullname(file,dir,exactname);
                if (wfile_status(file)==1)
                    {
                    strcpy(exactname,file);
                    return(7);
                    }
                }
            }
        wfile_fullname(file,dir,basename);
        if (wfile_status(file)==1)
            {
            strcpy(exactname,file);
            return(8);
            }
        }
    return(0);
    }


static int nextdir(char *dir,char *path,int *index)

    {
    int i,j;

    i=(*index);
    for (;path[i]==';' || path[i]==' ' || path[i]=='\t';i++);
    if (path[i]=='\0')
        {
        (*index)=i;
        return(0);
        }
    for (j=0;path[i]!=';' && path[i]!='\0';i++)
        dir[j++]=path[i];
    (*index)=i;
    dir[j]='\0';
    clean_line(dir);
    if (dir[0]=='\"' && dir[strlen(dir)-1]=='\"')
        {
        memmove(dir,&dir[1],strlen(dir));
        dir[strlen(dir)-1]='\0';
        clean_line(dir);
        }
    return(strlen(dir)>0);
    }


static int setbase(char *dest,char *filename,char *ext)

    {
    int   i;

    strcpy(dest,filename);
    i=strlen(filename)-4;
    if (i>0)
        {
        if (!stricmp(&filename[i],".com")
               || !stricmp(&filename[i],".exe")
               || !stricmp(&filename[i],".bat"))
            return(0);
        }
    strcat(dest,".");
    strcat(dest,ext);
    return(1);
    }

/*
** Returns NZ for success.
*/
int win_thread_terminate(void *thread_id,int exitcode)

    {
    int status;

    status=(int)TerminateThread((HANDLE)thread_id,exitcode);
    win_thread_close(thread_id);
    return(status);
    }


void *win_thread_create(void *funcptr,void *data)

    {
    DWORD  id;
    return((void *)CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)funcptr,(LPVOID)data,0,&id));
    }


void win_thread_close(void *thread_id)

    {
    CloseHandle((HANDLE)thread_id);
    }


void win_thread_exit(int exitcode)

    {
    ExitThread(exitcode);
    }


/*
** Use NULL if you don't want to get(?).
** ctime = last copied
** mtime = last modified
** atime = last accessed
*/
int win_getfiletimes(void *_atime,void *_ctime,void *_mtime,char *filename)

    {
    HANDLE      handle;
    FILETIME    *atime,*ctime,*mtime;

    atime=(FILETIME *)_atime;
    ctime=(FILETIME *)_ctime;
    mtime=(FILETIME *)_mtime;
    handle=(HANDLE)CreateFile(filename,GENERIC_READ,0,NULL,
                OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
    if (handle==INVALID_HANDLE_VALUE)
        return(0);
    if (!GetFileTime(handle,ctime,atime,mtime))
        {
        CloseHandle(handle);
        return(0);
        }
    CloseHandle(handle);
    return(-1);
    }


/*
** Use NULL if you don't want to set.
** ctime = last copied
** mtime = last modified
** atime = last accessed
*/
int win_setfiletimes(void *_atime,void *_ctime,void *_mtime,char *filename)

    {
    HANDLE      handle;
    FILETIME    *atime,*ctime,*mtime;

    atime=(FILETIME *)_atime;
    ctime=(FILETIME *)_ctime;
    mtime=(FILETIME *)_mtime;
    handle=(HANDLE)CreateFile(filename,GENERIC_READ|GENERIC_WRITE,
              0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
    if (handle==INVALID_HANDLE_VALUE)
        return(0);
    if (!SetFileTime(handle,ctime,atime,mtime))
        {
        CloseHandle(handle);
        return(0);
        }
    CloseHandle(handle);
    return(-1);
    }


void win_filetime2tm(struct tm *filedate,void *_ftime)

    {
    FILETIME    ltime,*ftime;
    SYSTEMTIME  stime;

    ftime=(FILETIME *)_ftime;
    FileTimeToLocalFileTime(ftime,&ltime);
    FileTimeToSystemTime(&ltime,&stime);
    filedate->tm_sec  = stime.wSecond;
    filedate->tm_min  = stime.wMinute;
    filedate->tm_hour = stime.wHour;
    filedate->tm_mday = stime.wDay;
    filedate->tm_mon  = stime.wMonth-1;
    filedate->tm_year = stime.wYear-1900;
    filedate->tm_wday = stime.wDayOfWeek; /* Sun = 0, Mon = 1, ... */
    filedate->tm_yday = -1;
    filedate->tm_isdst= -1;
    }


void win_tm2filetime(void *_ftime,struct tm *date)

    {
    SYSTEMTIME stime;
    FILETIME ltime,*ftime;

    ftime=(FILETIME *)_ftime;
    stime.wYear = date->tm_year+1900;
    stime.wMonth = date->tm_mon+1;
    stime.wDayOfWeek = date->tm_wday;
    stime.wDay = date->tm_mday;
    stime.wHour = date->tm_hour;
    stime.wMinute = date->tm_min;
    stime.wSecond = date->tm_sec;
    stime.wMilliseconds = 0;
    SystemTimeToFileTime(&stime,&ltime);
    LocalFileTimeToFileTime(&ltime,ftime);
    }



static char winname[MAXFILENAMELEN];
static HWND targetwin1;
void *win_start_app_get_win(char *syscmd,char *wname,double sleeptime,
                            int ntries,FILE *out)

    {
    int count;

    strncpy(winname,wname,255);
    winname[255]='\0';
    system(syscmd);
    for (count=0;count<ntries;count++)
        {
        targetwin1=0;
        EnumWindows((WNDENUMPROC)EnumWndFind,0L);
        if (targetwin1!=0)
            break;
        if (count>1)
            nprintf(out,"Could not find window '%s'.\n",winname);
        win_sleep((int)(sleeptime*1000.));
        }
    if (targetwin1==0)
        return(NULL);
    win_sleep((int)(sleeptime*1000.));
    /*
    childwin=0;
    EnumChildWindows(targetwin1,(WNDENUMPROC)EnumChildList,0);
    */
    return((void *)targetwin1);
    }




void *win_find_window(char *wname)

    {
    strncpy(winname,wname,255);
    winname[255]='\0';
    targetwin1=0;
    EnumWindows((WNDENUMPROC)EnumWndFind,0L);
    return((void *)targetwin1);
    }


static BOOL CALLBACK EnumWndFind(HWND h,LPARAM lp)

    {
    char    buf[MAXFILENAMELEN];

    if (targetwin1!=0)
        return(TRUE);
    GetWindowText(h,buf,255);
    if (wfile_unix_style_match(winname,buf))
        targetwin1=h;
    return(TRUE);
    }


static char findtitle[256];
static int  kwpid;


/*
** Kills first window with title bar text that has <title> anywhere in it
*/
int win_kill_by_name(char *title)

    {
    HANDLE h;
    int status;

    strncpy(findtitle,title,255);
    findtitle[255]='\0';
    kwpid=0;
    EnumWindows((WNDENUMPROC)findprocid,0L);
    if (!kwpid)
        return(-1);
    h=OpenProcess(PROCESS_ALL_ACCESS,TRUE,kwpid);
    if (h!=NULL)
        {
        if (TerminateProcess(h,99))
            status=0;
        else
            status=-2;
        }
    else
        status=-3;
    CloseHandle(h);
    return(status);
    }


int win_kill_pid(int pid)

    {
    HANDLE h;
    int status;

    h=OpenProcess(PROCESS_ALL_ACCESS,TRUE,pid);
    if (h!=NULL)
        {
        if (TerminateProcess(h,99))
            status=0;
        else
            status=-2;
        }
    else
        status=-3;
    CloseHandle(h);
    return(status);
    }


static BOOL CALLBACK findprocid(HWND hwnd,LPARAM lp)

    {
    int n,cid;
    char buf[256];

    if (kwpid!=0)
        return(TRUE);
    n=GetWindowText(hwnd,buf,255);
    if (n<=0)
        buf[0]='\0';
    if (in_string(buf,findtitle)<0)
        return(TRUE);
    if (in_string(buf,"killproc")>=0)
        return(TRUE);
    GetWindowThreadProcessId(hwnd,(DWORD *)&kwpid);
    cid=GetCurrentProcessId();
    if (kwpid==cid)
        kwpid=0;
    return(TRUE);
    }


void win_get_desktop_directory(char *desktop,int maxlen)

    {
    static char *keyname1="software\\microsoft\\windows\\currentversion\\explorer\\shell folders";
    static char *keyname2=".default\\software\\microsoft\\windows\\currentversion\\explorer\\shell folders";
    char windir[256];
    char *p;
    int status;

    status=get_desktop_directory_1(desktop,maxlen,HKEY_CURRENT_USER,keyname1);
    if (!status)
        return;
    status=get_desktop_directory_1(desktop,maxlen,HKEY_USERS,keyname2);
    if (!status)
        return;
    p=getenv("USERPROFILE");
    if (p==NULL)
        GetWindowsDirectory(windir,255);
    else
        {
        strncpy(windir,p,255);
        windir[255]='\0';
        }
    wfile_fullname(desktop,windir,"Desktop");
    if (wfile_status(desktop)==0)
        {
        wfile_fullname(desktop,windir,"Bureau");
        if (wfile_status(desktop)!=2)
            wfile_fullname(desktop,windir,"Desktop");
        }
    }


static int get_desktop_directory_1(char *desktop,int maxlen,HKEY key_class,
                                   char *keyname)

    {
    HKEY newkey;
    char class[128];
    int classsize,subkeys,maxsubkeylen,maxclasslen,values,maxvalnamelen;
    int maxvaluelen,status,i;
    FILETIME ft;

    status=RegOpenKeyEx(key_class,keyname,0,KEY_ALL_ACCESS,&newkey);
    if (status!=ERROR_SUCCESS)
        {
        // printf("Error %d (%s) opening key.\n",status,win_lasterror());
        return(-1);
        }
    status=RegQueryInfoKey(newkey,class,(LPDWORD)&classsize,(LPDWORD)NULL,(LPDWORD)&subkeys,
                           (LPDWORD)&maxsubkeylen,(LPDWORD)&maxclasslen,(LPDWORD)&values,
                           (LPDWORD)&maxvalnamelen,(LPDWORD)&maxvaluelen,(LPDWORD)NULL,&ft);
    if (status!=ERROR_SUCCESS)
        {
        // printf("Error %d (%s) getting key info.\n",status,win_lasterror());
        RegCloseKey(newkey);
        return(-2);
        }
    for (i=0;i<values;i++)
        {
        int size,valuesize,type;
        char buf[512];
        char valuename[256];
        
        size=511;
        valuesize=255;
        status=RegEnumValue(newkey,(DWORD)i,valuename,(LPDWORD)&valuesize,(LPDWORD)NULL,
                            (LPDWORD)&type,(LPBYTE)buf,(LPDWORD)&size);
        if (status!=ERROR_SUCCESS)
            continue;
        if (!stricmp(valuename,"desktop"))
            {
            strncpy(desktop,buf,maxlen);
            desktop[maxlen-1]='\0';
            break;
            }
        }
    RegCloseKey(newkey);
    if (i<values)
        return(0);
    return(-3);
    } 


/*
** Starting at keyroot, look for the value name basename (optionally recursively).
** Return value data in data.
** Return 0 if found.  Negative if not.
*/
int win_registry_search(char *data,int maxlen,char *basename,char *keyroot,int recursive)

    {
    int status;

    status=win_registry_search1(data,maxlen,HKEY_CURRENT_USER,keyroot,basename,recursive);
    if (status==0)
        return(status);
    status=win_registry_search1(data,maxlen,HKEY_LOCAL_MACHINE,keyroot,basename,recursive);
    return(status);
    }


/* Get user or system path */
int win_get_path(char *path,int maxlen,int syspath)

    {
    int i,status;
    HKEY newkey;
    
    if (syspath)
        status=RegOpenKeyEx(HKEY_LOCAL_MACHINE,"SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment",0,KEY_READ,&newkey);
    else
        status=RegOpenKeyEx(HKEY_CURRENT_USER,"Environment",0,KEY_READ,&newkey);
    if (status!=ERROR_SUCCESS)
        return(-1);
    for (i=0;1;i++)
        {
        int size,valuesize,type;
        char buf[1024];
        char valuename[256];
        
        size=1023;
        valuesize=255;
        status=RegEnumValue(newkey,(DWORD)i,valuename,(LPDWORD)&valuesize,(LPDWORD)NULL,
                            (LPDWORD)&type,(LPBYTE)buf,(LPDWORD)&size);
        if (status!=ERROR_SUCCESS)
            break;
        if (!stricmp(valuename,"path"))
            {
            strncpy(path,buf,maxlen-1);
            path[maxlen-1]='\0';
            RegCloseKey(newkey);
            return(0);
            }
        }
    RegCloseKey(newkey);
    return(-2);
    }


/* Set user or system path */
int win_set_path(char *path,int syspath)

    {
    int status;
    HKEY newkey;

    if (syspath)
        status=RegOpenKeyEx(HKEY_LOCAL_MACHINE,"SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment",0,KEY_WRITE,&newkey);
    else
        status=RegOpenKeyEx(HKEY_CURRENT_USER,"Environment",0,KEY_WRITE,&newkey);
    if (status!=ERROR_SUCCESS)
        return(-1);
    status=RegSetValueEx(newkey,"PATH",0,REG_SZ,(unsigned char *)path,strlen(path)+1);
    if (status!=ERROR_SUCCESS)
        return(-2);
    RegCloseKey(newkey);
    return(0);
    }


static int win_registry_search1(char *value,int maxlen,HKEY key_class,char *keyname,char *searchvalue,int recursive)

    {
    HKEY newkey;
    char class[128];
    int classsize,subkeys,maxsubkeylen,maxclasslen,values,maxvalnamelen;
    int maxvaluelen,status,i;
    FILETIME ft;

    status=RegOpenKeyEx(key_class,keyname,0,KEY_READ,&newkey);
    if (status!=ERROR_SUCCESS)
        {
        /* printf("Error %d (%s) opening key.\n",status,win_lasterror()); */
        return(-1);
        }
    status=RegQueryInfoKey(newkey,class,(LPDWORD)&classsize,NULL,(LPDWORD)&subkeys,
                           (LPDWORD)&maxsubkeylen,(LPDWORD)&maxclasslen,(LPDWORD)&values,
                           (LPDWORD)&maxvalnamelen,(LPDWORD)&maxvaluelen,(LPDWORD)NULL,&ft);
    /* printf("%s: Subkeys = %d, values = %d\n",keyname,subkeys,values); */
    if (status!=ERROR_SUCCESS)
        {
        /* printf("Error %d (%s) getting key info.\n",status,win_lasterror()); */
        RegCloseKey(newkey);
        return(-2);
        }
    for (i=values-1;i>=0;i--)
        {
        int size,valuesize,type;
        char valuename[512],buf[512];
        
        size=511;
        valuesize=511;
        status=RegEnumValue(newkey,(DWORD)i,valuename,(LPDWORD)&valuesize,(LPDWORD)NULL,
                                   (LPDWORD)&type,(LPBYTE)buf,(LPDWORD)&size);
        if (status!=ERROR_SUCCESS)
            continue;
        if (!stricmp(valuename,searchvalue))
            {
            strncpy(value,buf,maxlen-1);
            value[maxlen-1]='\0';
            break;
            }
        }
    if (i>=0 || !recursive)
        {
        RegCloseKey(newkey);
        return(i>=0 ? 0 : -3);
        }
    for (i=subkeys-1;i>=0;i--)
        {
        int valuesize;
        char valuename[512];
        char newkeyname[512];
        
        valuesize=511;
        status=RegEnumKey(newkey,(DWORD)i,valuename,valuesize);
        if (status!=ERROR_SUCCESS)
            continue;
        sprintf(newkeyname,"%s\\%s",keyname,valuename);
        if (win_registry_search1(value,maxlen,key_class,newkeyname,searchvalue,recursive)==0)
            break;
        }
    RegCloseKey(newkey);
    return(i>=0 ? 0 : -3);
    } 


int win_get_user_and_domain(char *szUser,int maxlen,char *szDomain,int maxlen2)

   {
   int          fSuccess = 0;
   HANDLE       hToken   = NULL;
   PTOKEN_USER  ptiUser  = NULL;
   DWORD        cbti     = 0;
   SID_NAME_USE snu;
   int *pcchUser,*pcchDomain;

   pcchUser=&maxlen;
   pcchDomain=&maxlen2;
   while (1)
       {
       // Get the calling thread's access token.
       if (!OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, TRUE,
            &hToken))
           {
           if (GetLastError() != ERROR_NO_TOKEN)
              break;
           // Retry against process token if no thread token exists.
           if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, 
                 &hToken))
              break;
           }

      // Obtain the size of the user information in the token.
      if (GetTokenInformation(hToken, TokenUser, NULL, 0, &cbti))
         // Call should have failed due to zero-length buffer.
          break;
      else
          {
          if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
              break;
          }

      // Allocate buffer for user information in the token.
      ptiUser = (PTOKEN_USER) HeapAlloc(GetProcessHeap(), 0, cbti);
      if (!ptiUser)
          break;
      // Retrieve the user information from the token.
      if (!GetTokenInformation(hToken, TokenUser, ptiUser, cbti, &cbti))
          break;
      // Retrieve user name and domain name based on user's SID.
      if (!LookupAccountSid(NULL, ptiUser->User.Sid, szUser, (LPDWORD)pcchUser, 
            szDomain, (LPDWORD)pcchDomain, &snu))
          break;
      fSuccess = 1;
      } 

    // Free resources.
    if (hToken)
        CloseHandle(hToken);

    if (ptiUser)
        HeapFree(GetProcessHeap(), 0, ptiUser);

    return(fSuccess);
    }


/*
** Returns NZ if current process was started from a command prompt
** (console class window).
*/
static int fwbp_pid;
static int fwbp_count;
int win_has_own_window(void)

    {
    fwbp_pid=GetCurrentProcessId(); // win_getppid(0);
    if (fwbp_pid==0)
        return(0);
    // fwbp_handle=NULL;
    fwbp_count=0;
    EnumWindows((WNDENUMPROC)find_win_by_procid,0L);
    return(fwbp_count>0);
    /*
    if (fwbp_count!=1 || fwbp_handle==NULL)
        return(0);
    GetClassName(fwbp_handle,buf,63);
    return(!stricmp(buf,"ConsoleWindowClass"));
    */
    }


static BOOL CALLBACK find_win_by_procid(HWND hwnd,LPARAM lp)

    {
    int pid;

    GetWindowThreadProcessId(hwnd,(LPDWORD)&pid);
    if (pid==fwbp_pid)
        {
        // fwbp_handle=hwnd;
        fwbp_count++;
        }
    return(TRUE);
    }


int win_getppid(int pid)

    {    int ppid = 0;
    HANDLE h = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 pe = { 0 };
    pe.dwSize = sizeof(PROCESSENTRY32);

    //assume first arg is the PID to get the PPID for, or use own PID
    if (pid==0)
        pid = GetCurrentProcessId();
    if (Process32First(h,&pe))
        {
        do
            {
            if (pe.th32ProcessID == pid)
                {
                ppid=pe.th32ParentProcessID;
break;
                }
            } while (Process32Next(h,&pe));
        }
    CloseHandle(h);
    return(ppid);
    }

#endif /* WIN32 */
