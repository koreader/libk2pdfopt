/*
** ANSI.C       Allow ANSI color display and cursor movement to
**              be turned on/off, and also allow it to work properly
**              in the Win32 shell.
**              The ANSI parsing at this point is admittedly quite crude.
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
#if (defined(WIN32) || defined(WIN64))
#include <windows.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/*
** For calling ioctl() function to get tty size (rows and columns)
** See ansi_rows_cols() function.
*/
#if (!defined(WIN32) && !defined(WIN64))
#include <unistd.h>
#include <sys/ioctl.h>
#endif



#ifdef WIN32
#define ANSI_PARSE
#endif

#define MAXSIZE 8000

static int ansi_on=1;
static char ansi_buffer[MAXSIZE];

static void ansi_code(FILE *f,int *args,int nargs,int code);
static void ansi_parse(FILE *f,char *s);
#ifdef WIN32
static void ansi_win32_setcolor(FILE *f,int n);
static void ansi_win32_erase_end_line(FILE *f);
static void ansi_win32_cursor_left(FILE *f,int n);
static void ansi_win32_cursor_up(FILE *f,int n);
static void ansi_win32_cursor_position(FILE *f,int x,int y);
static void ansi_win32_get_cursor(FILE *f,int *x,int *y);
static int  ansi_win32_rows_cols(FILE *f,int *rows,int *cols);
static void ansi_win32_clear(FILE *f);
#endif


void ansi_set(int on)

    {
    ansi_on = on;
    }


int aprintf(char *fmt,...)

    {
    va_list args;
    int     status;

    va_start(args,fmt);
    status=avprintf(stdout,fmt,args);
    va_end(args);
    return(status);
    }



static int wlp_to_stdout=1;
static int wlp_to_stderr=0;
static int wlp_to_extra_stream=0;
static int wlp_to_file=0;
static int wlp_close_after_write=0;
static char wlp_filename[MAXFILENAMELEN];
static FILE *wlp_file=NULL;
static FILE *wlp_stream=NULL;

static int x_wlp_to_stdout=1;
static int x_wlp_to_stderr=0;
static int x_wlp_to_extra_stream=0;
static int x_wlp_to_file=0;
static int x_wlp_close_after_write=0;
static char x_wlp_filename[MAXFILENAMELEN];
static FILE *x_wlp_file=NULL;
static FILE *x_wlp_stream=NULL;

void wlp_save_status(void)

    {
    x_wlp_to_stdout = wlp_to_stdout;
    x_wlp_to_stderr = wlp_to_stderr;
    x_wlp_to_extra_stream = wlp_to_extra_stream;
    x_wlp_to_file = wlp_to_file;
    x_wlp_close_after_write = wlp_close_after_write;
    strcpy(x_wlp_filename,wlp_filename);
    x_wlp_file = wlp_file;
    x_wlp_stream = wlp_stream;
    }

void wlp_restore_status(void)

    {
    wlp_to_stdout = x_wlp_to_stdout;
    wlp_to_stderr = x_wlp_to_stderr;
    wlp_to_extra_stream = x_wlp_to_extra_stream;
    wlp_stream = x_wlp_stream;
    if (wlp_to_file)
        if (!wlp_close_after_write && wlp_file!=NULL)
            fclose(wlp_file);
    wlp_to_file=x_wlp_to_file;
    wlp_close_after_write=x_wlp_close_after_write;
    strcpy(wlp_filename,x_wlp_filename);
    if (!wlp_close_after_write)
        wlp_file=fopen(wlp_filename,"a");
    else
        wlp_file=NULL;
    }


/*
** Use -1 on sout,serr for no change, 0 to turn off, 1 to turn on.
** Use NULL on filename for no change, empty string to turn off.
** Use newstream=1 to change extra stream, 0 to ignore.
*/
void wlp_set_stdout(int sout,int serr,char *filename,int close_after,
                    int append,int newstream,FILE *str)

    {
    wlp_save_status();
    if (sout==0 || sout==1)
        wlp_to_stdout=sout;
    if (serr==0 || serr==1)
        wlp_to_stderr=serr;
    if (filename!=NULL)
        {
        if (wlp_to_file && !wlp_close_after_write && wlp_file!=NULL)
            fclose(wlp_file);
        strcpy(wlp_filename,filename);
        wlp_to_file = !(filename[0]=='\0');
        wlp_close_after_write=close_after;
        if (wlp_to_file && !wlp_close_after_write)
            wlp_file=fopen(wlp_filename,append ? "a" : "w");
        else
            wlp_file=NULL;
        }
    if (newstream)
        wlp_stream = str;
    }
    

int wlprintf(char *fmt,...)

    {
    va_list args;
    int status;

    status=0;
    if (wlp_to_stdout)
        {
        va_start(args,fmt);
        status=vfprintf(stdout,fmt,args);
        va_end(args);
        }
    if (wlp_to_stderr)
        {
        va_start(args,fmt);
        status=vfprintf(stderr,fmt,args);
        va_end(args);
        }
    if (wlp_to_extra_stream && wlp_stream!=NULL)
        {
        va_start(args,fmt);
        status=vfprintf(wlp_stream,fmt,args);
        va_end(args);
        }
    if (wlp_to_file)
        {
        if (wlp_close_after_write)
            wlp_file=fopen(wlp_filename,"a");
        if (wlp_file!=NULL)
            {
            va_start(args,fmt);
            status=vfprintf(wlp_file,fmt,args);
            va_end(args);
            if (wlp_close_after_write)
                fclose(wlp_file);
            }
        }
    return(status);
    }


/*
** Like nprintf, but prints to two streams at once.
*/
int nprintf2(FILE *f1,FILE *f2,char *fmt,...)

    {
    va_list args;
    int     status;

    status=0;
    if (f1!=NULL)
        {
        va_start(args,fmt);
        status=vfprintf(f1,fmt,args);
        va_end(args);
        }
    if (f2!=NULL)
        {
        va_start(args,fmt);
        status=vfprintf(f2,fmt,args);
        va_end(args);
        }
    if (f1!=NULL || f2!=NULL)
        return(status);
    return(-1);
    }

/*
** Like fprintf, but don't print if f==NULL.
*/
int nprintf(FILE *f,char *fmt,...)

    {
    va_list args;
    int     status;

    if (f!=NULL)
        {
        va_start(args,fmt);
        status=vfprintf(f,fmt,args);
        va_end(args);
        return(status);
        }
    return(-1);
    }
        

int afprintf(FILE *f,char *fmt,...)

    {
    va_list args;
    int     status;

    va_start(args,fmt);
    status=avprintf(f,fmt,args);
    va_end(args);
    return(status);
    }


int avprintf(FILE *f,char *fmt,va_list args)

    {
    int     status;

#ifdef WIN32
    if (wsys_win32_api())
        {
        status=vsprintf(ansi_buffer,fmt,args);
        ansi_parse(f,ansi_buffer);
        }
    else
#endif
        {
        if (!ansi_on)
            {
            status=vsprintf(ansi_buffer,fmt,args);
            ansi_parse(f,ansi_buffer);
            }
        else
            status=vfprintf(f,fmt,args);
        }
    return(status);
    }

/*
** Returns 1 for success, 0 for fail.
*/
int ansi_rows_cols(FILE *f,int *rows,int *cols)

    {
#if (defined(WIN32) || defined(WIN64))
    (*rows)=-1;
    (*cols)=-1;
    return(ansi_win32_rows_cols(f,rows,cols));
#else
    int status;
    struct winsize ws;

    (*rows)=-1;
    (*cols)=-1;
    status=ioctl(fileno(f),TIOCGWINSZ,&ws);
    if (!status)
        {
        (*rows)=ws.ws_row;
        (*cols)=ws.ws_col;
        return(1);
        }
    return(0);
#endif
    }


static void ansi_parse(FILE *f,char *s)

    {
    int     i0,i,ie,k;
    char    nbuf[10];
    int     args[2];
    int     nargs;

    i0=0;
    i=i0;
    while (1)
        {
        for (;s[i]!=27 && s[i]!='\0';i++);
        if (s[i]=='\0')
            {
            fprintf(f,"%s",&s[i0]);
            return;
            }
        ie=i;
        i++;
        if (s[i]!='[')
            continue;
        i++;
        nargs=0;
        args[0]=args[1]=0;
        for (k=0;k<4;k++)
            if (s[i]>='0' && s[i]<='9')
                {
                nbuf[k]=s[i];
                i++;
                }
            else
                break;
        if (k>=4)
            continue;
        if (k)
            {
            nbuf[k]='\0';
            args[nargs++]=atoi(nbuf);
            }
        if (s[i]==';')
            {
            i++;
            for (k=0;k<4;k++)
                if (s[i]>='0' && s[i]<='9')
                    {
                    nbuf[k]=s[i];
                    i++;
                    }
                else
                    break;
            if (k>=4)
                continue;
            if (k)
                {
                nbuf[k]='\0';
                args[nargs++]=atoi(nbuf);
                }
            }
        while (s[i]==';')
            i++;
        s[ie]='\0';
        fprintf(f,"%s",&s[i0]);
        ansi_code(f,args,nargs,s[i]);
        i++;
        i0=i;
        }
    }


static void ansi_code(FILE *f,int *args,int nargs,int code)

    {
#ifdef WIN32
    static int x=0;
    static int y=0;
#endif

    if (!ansi_on)
        {
        switch (code)
            {
            case 'D':
                fprintf(f,"\n");
                break;
            }
        return;
        }
#ifdef WIN32
    if (wsys_win32_api())
        switch (code)
            {
            case 'm':
                ansi_win32_setcolor(f,args[0]);
                break;
            case 'K':
                ansi_win32_erase_end_line(f);
                break;
            case 'A':
                ansi_win32_cursor_up(f,args[0]);
                break;
            case 'D':
                ansi_win32_cursor_left(f,args[0]);
                break;
            case 'J':
                ansi_win32_clear(f);
                break;
            case 'H':
                ansi_win32_cursor_position(f,args[1],args[0]);
                break;
            case 's':
                ansi_win32_get_cursor(f,&x,&y);
                break;
            case 'u':
                ansi_win32_cursor_position(f,x,y);
                break;
            }
#endif
    }


#ifdef WIN32
static void ansi_win32_setcolor(FILE *f,int n)

    {
    HANDLE  hout;
    CONSOLE_SCREEN_BUFFER_INFO  cinfo;
    WORD    newcolor;

    if (f==stderr)
        hout=GetStdHandle(STD_ERROR_HANDLE);
    else if (f==stdout)
        hout=GetStdHandle(STD_OUTPUT_HANDLE);
    else
        return;
    if (hout==INVALID_HANDLE_VALUE)
        {
        fprintf(stderr,"ansi_win32_setcolor:  get handle failed.\n");
        return;
        }
    if (!GetConsoleScreenBufferInfo(hout,&cinfo))
        {
        fprintf(stderr,"ansi_win32_setcolor:  get console info failed.\n");
        return;
        }

    newcolor=cinfo.wAttributes;
    if (n==1)
        newcolor |= 8;
    else if (n==0)
        newcolor &= 0xfff7;
    else if (n>=30 && n<=37)
        {
        static WORD translate[8] = {0,4,2,6,1,5,3,7};
        newcolor &= 0xfff8;
        newcolor |= translate[n-30];
        }
    else if (n>=40 && n<=47)
        {
        static WORD translate[8] = {0,4,2,6,1,5,3,7};
        newcolor &= 0xff8f;
        newcolor |= (translate[n-40]<<4);
        }
    if (!SetConsoleTextAttribute(hout,newcolor))
        {
        fprintf(stderr,"ansi_win32_setcolor:  color set failed.\n");
        return;
        }
    }


static void ansi_win32_erase_end_line(FILE *f)

    {
    HANDLE  hout;
    CONSOLE_SCREEN_BUFFER_INFO  cinfo;
    /*
    CHAR_INFO string[200];
    */
    static  char buf[256];
    int     ii;
    COORD   c1;
    /*
    SMALL_RECT  sr;
    */
    int x,y;

    if (f==stderr)
        hout=GetStdHandle(STD_ERROR_HANDLE);
    else if (f==stdout)
        hout=GetStdHandle(STD_OUTPUT_HANDLE);
    else
        return;
    if (hout==INVALID_HANDLE_VALUE)
        {
        fprintf(stderr,"ansi_win32_erase_end_line:  get handle failed.\n");
        return;
        }
    if (!GetConsoleScreenBufferInfo(hout,&cinfo))
        {
        fprintf(stderr,"ansi_win32_erase_end_line:  get console info failed.\n");
        return;
        }

    /*
    for (ii=0;ii<200;ii++)
        {
        string[ii].Char.AsciiChar='X';
        string[ii].Attributes=cinfo.wAttributes;
        }
    */
    c1.X=cinfo.srWindow.Right-cinfo.dwCursorPosition.X+1;
    c1.Y=1;
/*
    c2.X=1;
    c2.Y=1;
*/
    for (ii=0;ii<c1.X;ii++)
        buf[ii]=' ';
    buf[ii]='\0';
    x=y=0; /* Avoid compiler warning */
    ansi_win32_get_cursor(f,&x,&y);
    printf("%s",buf);
    ansi_win32_cursor_position(f,x,y);
    }


static void ansi_win32_cursor_left(FILE *f,int n)

    {
    HANDLE  hout;
    CONSOLE_SCREEN_BUFFER_INFO  cinfo;

    if (f==stderr)
        hout=GetStdHandle(STD_ERROR_HANDLE);
    else if (f==stdout)
        hout=GetStdHandle(STD_OUTPUT_HANDLE);
    else
        return;
    if (hout==INVALID_HANDLE_VALUE)
        {
        fprintf(stderr,"ansi_win32_cursor_left:  get handle failed.\n");
        return;
        }
    if (!GetConsoleScreenBufferInfo(hout,&cinfo))
        {
        fprintf(stderr,"ansi_win32_cursor_left:  get console info failed.\n");
        return;
        }

    cinfo.dwCursorPosition.X -= n;
    if (cinfo.dwCursorPosition.X < 0)
        cinfo.dwCursorPosition.X = 0;
    SetConsoleCursorPosition(hout,cinfo.dwCursorPosition);
    }


static void ansi_win32_cursor_up(FILE *f,int n)

    {
    HANDLE  hout;
    CONSOLE_SCREEN_BUFFER_INFO  cinfo;

    if (f==stderr)
        hout=GetStdHandle(STD_ERROR_HANDLE);
    else if (f==stdout)
        hout=GetStdHandle(STD_OUTPUT_HANDLE);
    else
        return;
    if (hout==INVALID_HANDLE_VALUE)
        {
        fprintf(stderr,"ansi_win32_cursor_up:  get handle failed.\n");
        return;
        }
    if (!GetConsoleScreenBufferInfo(hout,&cinfo))
        {
        fprintf(stderr,"ansi_win32_cursor_up:  get console info failed.\n");
        return;
        }

    cinfo.dwCursorPosition.Y -= n;
    if (cinfo.dwCursorPosition.Y < 0)
        cinfo.dwCursorPosition.Y = 0;
    SetConsoleCursorPosition(hout,cinfo.dwCursorPosition);
    }


static void ansi_win32_cursor_position(FILE *f,int x,int y)

    {
    HANDLE  hout;
    CONSOLE_SCREEN_BUFFER_INFO  cinfo;

    if (f==stderr)
        hout=GetStdHandle(STD_ERROR_HANDLE);
    else if (f==stdout)
        hout=GetStdHandle(STD_OUTPUT_HANDLE);
    else
        return;
    if (hout==INVALID_HANDLE_VALUE)
        {
        fprintf(stderr,"ansi_win32_cursor_position:  get handle failed.\n");
        return;
        }
    if (!GetConsoleScreenBufferInfo(hout,&cinfo))
        {
        fprintf(stderr,"ansi_win32_cursor_position:  get console info failed.\n");
        return;
        }

    cinfo.dwCursorPosition.X = x;
    cinfo.dwCursorPosition.Y = y;
    SetConsoleCursorPosition(hout,cinfo.dwCursorPosition);
    }


/*
** Return 0 for fail, 1 for success.
*/
static int ansi_win32_rows_cols(FILE *f,int *rows,int *cols)

    {
    HANDLE  hout;
    CONSOLE_SCREEN_BUFFER_INFO  cinfo;

    if (f==stderr)
        hout=GetStdHandle(STD_ERROR_HANDLE);
    else
        hout=GetStdHandle(STD_OUTPUT_HANDLE);
    if (hout==INVALID_HANDLE_VALUE)
        {
        // fprintf(stderr,"ansi_win32_rows_cols:  get handle failed.\n");
        return(0);
        }
    if (!GetConsoleScreenBufferInfo(hout,&cinfo))
        {
        // fprintf(stderr,"ansi_win32_rows_cols:  get console info failed.\n");
        return(0);
        }
    /*
    ** Note that dwSize returns the full buffer size of the window.
    */
    (*rows) = cinfo.srWindow.Bottom - cinfo.srWindow.Top + 1;
    (*cols) = cinfo.srWindow.Right - cinfo.srWindow.Left + 1;
    return(1);
    }


static void ansi_win32_get_cursor(FILE *f,int *x,int *y)

    {
    HANDLE  hout;
    CONSOLE_SCREEN_BUFFER_INFO  cinfo;

    if (f==stderr)
        hout=GetStdHandle(STD_ERROR_HANDLE);
    else if (f==stdout)
        hout=GetStdHandle(STD_OUTPUT_HANDLE);
    else
        return;
    if (hout==INVALID_HANDLE_VALUE)
        {
        fprintf(stderr,"ansi_win32_get_cursor:  get handle failed.\n");
        return;
        }
    if (!GetConsoleScreenBufferInfo(hout,&cinfo))
        {
        fprintf(stderr,"ansi_win32_get_cursor:  get console info failed.\n");
        return;
        }

    (*x) = cinfo.dwCursorPosition.X;
    (*y) = cinfo.dwCursorPosition.Y;
    }


static void ansi_win32_clear(FILE *f)

    {
    HANDLE  hout;
    CONSOLE_SCREEN_BUFFER_INFO  cinfo;
    DWORD   nwritten;
    COORD   c;


    if (f==stderr)
        hout=GetStdHandle(STD_ERROR_HANDLE);
    else if (f==stdout)
        hout=GetStdHandle(STD_OUTPUT_HANDLE);
    else
        return;
    if (hout==INVALID_HANDLE_VALUE)
        {
        fprintf(stderr,"ansi_win32_clear:  get handle failed.\n");
        return;
        }
    if (!GetConsoleScreenBufferInfo(hout,&cinfo))
        {
        fprintf(stderr,"ansi_win32_clear:  get console info failed.\n");
        return;
        }
    c.X=0;
    c.Y=0;
    FillConsoleOutputCharacter(hout,' ',10000,c,&nwritten);
    FillConsoleOutputAttribute(hout,cinfo.wAttributes,10000,c,&nwritten);
    cinfo.dwCursorPosition.X = 0;
    cinfo.dwCursorPosition.Y = 0;
    SetConsoleCursorPosition(hout,cinfo.dwCursorPosition);
    }
#endif // WIN32
