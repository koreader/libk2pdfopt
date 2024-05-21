/*
** k2pdfopt.c    K2pdfopt optimizes PDF/DJVU files for mobile e-readers
**               (e.g. the Kindle) and smartphones. It works well on multi-
**               column PDF/DJVU files and can re-flow text even on scanned PDF
**               files. It can also be used as a general PDF copying/cropping/
**               re-sizing manipulation tool. It can generate native or
**               bitmapped PDF output, with an optional OCR layer. There are
**               downloads for MS Windows, Mac OSX, and Linux. The MS Windows
**               version has an integrated GUI. K2pdfopt is open source.
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
**
** SOURCE CODE FLOW (BRIEF)
**
** Conversion process:
**     k2pdfopt_proc_wildarg()  (k2file.c)   Convert wildcard arg, e.g. *.pdf
**             |
**             V
**     k2pdfopt_proc_arg()      (k2file.c)   Handle arg if it is a folder
**             |
**             V
**     k2pdfopt_proc_one()      (k2file.c)   Process a single PDF/DJVU file.
**             |                             (Embarrassingly long function.)
**             V
**     bmpregion_source_page_add()  (k2proc.c)  Processes a source page of the file.
**                                              This adds rectangular regions
**                                              (BMPREGION structure) to the
**                                              PAGEREGIONS structure.
**
**     Some other key functions:
**
**     bmpregion_vertically_break() (k2proc.c) looks for "text rows" in each region,
**         segmenting it into consecutive "rows."
**
**     bmpregion_add_textrow() (k2proc.c) is called by bmpregion_vertically_break()
**         to accumulate the BMPEREGIONs row by row.
**
**     bmpregion_add() (k2proc.c) processes a "row" or rectangular region of the
**         source page.  It is fairly well commented.
**
**     bmpregion_analyze_justification_and_line_spacing() (k2proc.c) analyzes the
**         "rows" and attempts to determine things like if they are regular, uniform
**         rows of text, how the text is justified, what the line spacing and font
**         size is, and if any lines are indented or terminate a section.
**
**     bmpregion_one_row_wrap_and_add() (k2proc.c) parses through one row and looks
**         for words.  It parses out each word to wrapbmp_add().
**
**     wrapbmp_add() (wrapbmp.c) adds a graphical word (as a rectangular bitmap region)
**         to the WRAPBMP structure, which stores up a row of text until it is too
**         wide for the destination file and the flushes that row of text to the
**         destination file.
**     
**
*/

#include <k2pdfopt.h>

#if (defined(HAVE_K2GUI) && (defined(WIN32) || defined(WIN64)))
#include <windows.h>
static void k2pdfopt_launch_gui(K2PDFOPT_CONVERSION *k2conv,STRBUF *env,STRBUF *cmdline,int ascii);
#endif


int main(int argc,char *argv[])

    {
    int i;
    static K2PDFOPT_CONVERSION _k2conv,*k2conv;
    K2PDFOPT_SETTINGS *k2settings;
    static STRBUF _cmdline,_env,_usermenu;
    STRBUF *cmdline,*env,*usermenu;
    static char *funcname="main";
#if (defined(WIN32) || defined(WIN64))
    int ascii;
    short *clinew;

    clinew=(short *)GetCommandLineW(); /* Get UTF-16 command-line */
    ascii=(clinew==NULL || wide_is_ascii(clinew));
#endif
    k2conv=&_k2conv;
    k2pdfopt_conversion_init(k2conv);
    k2settings=&k2conv->k2settings;
    cmdline=&_cmdline;
    env=&_env;
    usermenu=&_usermenu;
    strbuf_init(cmdline);
    strbuf_init(env);
    strbuf_init(usermenu);
#if (defined(WIN32) || defined(WIN64))
    strbuf_ensure(env,1024);
    wsys_get_envvar_ex("K2PDFOPT",env->s,1023);
#else
    strbuf_cpy(env,getenv("K2PDFOPT"));
#endif

#if (defined(WIN32) || defined(WIN64))
    {
    short **argvw;
    int nargs;

    argvw=(short **)CommandLineToArgvW((LPWSTR)clinew,&nargs);
    for (i=1;i<argc;i++)
        {
        char *clineu8;
        int clen;
        clen=utf16_to_utf8(NULL,argvw[i],MAXUTF8PATHLEN);
        willus_mem_alloc_warn((void **)&clineu8,clen+1,funcname,10);
        utf16_to_utf8(clineu8,argvw[i],clen);
        strbuf_cat_with_quotes(cmdline,clineu8);
        willus_mem_free((double **)&clineu8,funcname);
        }
    LocalFree(argvw);
    }
#else
    for (i=1;i<argc;i++)
        strbuf_cat_with_quotes(cmdline,argv[i]);
#endif
    k2sys_init();
    k2pdfopt_settings_init(k2settings);
    k2pdfopt_files_clear(&k2conv->k2files);
    /* Only set ansi and user interface */
    parse_cmd_args(k2conv,env,cmdline,usermenu,2,0);
#ifdef HAVE_K2GUI
    if (k2settings->gui>0
          || (k2settings->gui<0
               && (win_has_own_window() 
                    || (!win_has_own_window() && argc<2))))
        {
        strbuf_free(usermenu);
        k2pdfopt_launch_gui(k2conv,env,cmdline,ascii);
        k2sys_close(k2settings);
        strbuf_free(env);
        strbuf_free(cmdline);
        k2pdfopt_conversion_close(k2conv);
        return(0);
        }
#endif
    if (k2settings->show_usage[0]!='\0')
        {
        k2sys_header(NULL);
        if (k2settings->query_user==0 
#if (defined(WIN32) || defined(WIN64))
              || !win_has_own_window()
#endif
                          )
            k2pdfopt_usage(k2settings->show_usage,0);
        else
            {
            if (!k2pdfopt_usage(k2settings->show_usage,1))
                {
                k2sys_close(k2settings);
                strbuf_free(usermenu);
                strbuf_free(env);
                strbuf_free(cmdline);
                k2pdfopt_conversion_close(k2conv);
                return(0);
                }
            }
        if (k2settings->query_user!=0)
            k2sys_enter_to_exit(k2settings);
        k2sys_close(k2settings);
        strbuf_free(usermenu);
        strbuf_free(env);
        strbuf_free(cmdline);
        k2pdfopt_conversion_close(k2conv);
        return(0);
        }
#ifdef HAVE_TESSERACT_LIB
    if (k2settings->dst_ocr_lang[0]=='?')
        {
        char *p;

        k2sys_header(NULL);
        ocrtess_debug_info(&p,1);
        aprintf("%s",p);
        willus_mem_free((double **)&p,funcname);
        if (k2settings->query_user!=0)
            k2sys_enter_to_exit(k2settings);
        k2sys_close(k2settings);
        strbuf_free(usermenu);
        strbuf_free(env);
        strbuf_free(cmdline);
        k2pdfopt_conversion_close(k2conv);
        return(0);
        }
#endif
#if (defined(WIN32) || defined(WIN64))
        {
        if (win_has_own_window())
            k2settings->query_user=1;
        else
            k2settings->query_user=(k2conv->k2files.n==0);
        }
#else
        k2settings->query_user=1;
#endif
#if (!defined(WIN32) && !defined(WIN64))
    if (k2settings->query_user)
        {
        int tty_rows;
        tty_rows = get_ttyrows();
        for (i=0;i<tty_rows-16;i++)
            aprintf("\n");
        }
#endif
    k2sys_header(NULL);

    /*
    ** Set all options from command-line arguments
    */
    parse_cmd_args(k2conv,env,cmdline,usermenu,1,0);
    /*
    ** Get user input
    */
    if (k2pdfopt_menu(k2conv,env,cmdline,usermenu)==-1)
        {
        k2sys_close(k2settings);
        strbuf_free(usermenu);
        strbuf_free(env);
        strbuf_free(cmdline);
        k2pdfopt_conversion_close(k2conv);
        return(0);
        }
    /*
    ** Re-init and then re-parse after all user menu entries applied.
    */
    k2pdfopt_settings_init(k2settings);
    parse_cmd_args(k2conv,env,cmdline,usermenu,1,0);

    /*
    ** Process files
    */
    {
    K2PDFOPT_FILELIST_PROCESS k2listproc;
    double start,stop;

    start=(double)clock()/CLOCKS_PER_SEC;
    for (i=k2listproc.filecount=0;i<k2conv->k2files.n;i++)
        {
        k2listproc.outname=NULL;
        k2listproc.bmp=NULL;
        k2listproc.mode=K2PDFOPT_FILELIST_PROCESS_MODE_CONVERT_FILES;
        k2pdfopt_proc_wildarg(k2settings,k2conv->k2files.file[i],&k2listproc);
        willus_mem_free((double **)&k2listproc.outname,funcname);
        }
    stop=(double)clock()/CLOCKS_PER_SEC;
    k2sys_cpu_update(k2settings,start,stop);
    }

    /*
    ** All done.
    */
    k2sys_enter_to_exit(k2settings);
    k2sys_close(k2settings);
    strbuf_free(usermenu);
    strbuf_free(env);
    strbuf_free(cmdline);
    k2pdfopt_conversion_close(k2conv);
    return(0);
    }


#if (defined(HAVE_K2GUI) && (defined(WIN32) || defined(WIN64)))

static void k2pdfopt_launch_gui(K2PDFOPT_CONVERSION *k2conv,STRBUF *env,STRBUF *cmdline,
                                int ascii)

    {
#if (WILLUSDEBUGX & 0x4000)
printf("\n\nNEED TO TURN OFF WILLUSDEBUGX FOR FINAL COMPILE...\n\n");
if (0)
#else
    if (k2conv->k2settings.gui!=2 && (!win_has_own_window() || !k2conv->k2settings.guimin))
#endif
        {
        short exename[512];
        short *x;
        short *buf;
        int  i;
        STARTUPINFOW si;
        PROCESS_INFORMATION pi;
        static char *funcname="k2pdfopt_launch_gui";

        GetStartupInfoW(&si);
        GetModuleFileNameW(NULL,(WCHAR *)exename,511);
        x=(short *)GetCommandLineW();
        willus_mem_alloc_warn((void **)&buf,sizeof(short)*(wide_strlen(x)+8),funcname,10);
        wide_strcpy(buf,x);
        i=wide_strlen(buf);
        buf[i++]=' ';
        buf[i++]='-';
        buf[i++]='g';
        buf[i++]='u';
        buf[i++]='i';
        buf[i++]='+';
        buf[i++]=0;
        memset(&pi,0,sizeof(PROCESS_INFORMATION));
        memset(&si,0,sizeof(STARTUPINFOW));
        si.cb = sizeof(STARTUPINFOW);
        si.dwX = 0; /* Ignored unless si.dwFlags |= STARTF_USEPOSITION */
        si.dwY = 0;
        si.dwXSize = 0; /* Ignored unless si.dwFlags |= STARTF_USESIZE */
        si.dwYSize = 0;
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_SHOWNORMAL;
        /* Launching from a console will NOT create new console. */
        CreateProcessW((LPCWSTR)exename,(LPWSTR)buf,0,0,1,DETACHED_PROCESS,0,NULL,&si,&pi);
        }
    else
        {
        HINSTANCE hinst;
        /* Free console and launch the GUI window */
        hinst=GetModuleHandle(NULL);
#if (!(WILLUSDEBUGX & 0x4000))
        FreeConsole();
#endif
        k2gui_main(k2conv,hinst,NULL,env,cmdline,ascii);
        }
    }

#endif /* Windows GUI */
