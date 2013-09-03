/*
** k2pdfopt.c    K2pdfopt optimizes PDF/DJVU files for mobile e-readers
**               (e.g. the Kindle) and smartphones. It works well on
**               multi-column PDF/DJVU files. K2pdfopt is freeware.
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

#if (defined(HAVE_K2GUI) && (defined(WIN32) || defined(WIN64)))
#include <windows.h>
static void k2pdfopt_launch_gui(K2PDFOPT_CONVERSION *k2conv,STRBUF *env,STRBUF *cmdline);
#endif


int main(int argc,char *argv[])

    {
    int i;
    static K2PDFOPT_CONVERSION _k2conv,*k2conv;
    K2PDFOPT_SETTINGS *k2settings;
    static STRBUF _cmdline,_env,_usermenu;
    STRBUF *cmdline,*env,*usermenu;
    static char *funcname="main";

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
    
    for (i=1;i<argc;i++)
        strbuf_cat_with_quotes(cmdline,argv[i]);
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
        k2pdfopt_launch_gui(k2conv,env,cmdline);
        k2sys_close(k2settings);
        strbuf_free(env);
        strbuf_free(cmdline);
        k2pdfopt_conversion_close(k2conv);
        return(0);
        }
#endif
    if (k2settings->show_usage)
        {
        k2sys_header(NULL);
        if (k2settings->query_user==0 
#if (defined(WIN32) || defined(WIN64))
              || !win_has_own_window()
#endif
                          )
            k2usage_show_all(stdout);
        else
            {
            if (!k2pdfopt_usage())
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
    if (k2settings->query_user<0)
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
    for (i=0;i<k2conv->k2files.n;i++)
        {
        K2PDFOPT_OUTPUT k2out;

        k2out.outname=NULL;
        k2out.filecount=0;
        k2out.bmp=NULL;
        k2pdfopt_proc_wildarg(k2settings,k2conv->k2files.file[i],1,&k2out);
        willus_mem_free((double **)&k2out.outname,funcname);
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

static void k2pdfopt_launch_gui(K2PDFOPT_CONVERSION *k2conv,STRBUF *env,STRBUF *cmdline)

    {
/*
printf("\n\nNEED TO FIX IF STATEMENT IN k2pdfopt_launch_gui()...\n\n");
if (0)
*/
    if (k2conv->k2settings.gui!=2 && (!win_has_own_window() || !k2conv->k2settings.guimin))
        {
        char exename[512];
        char *buf;
        int  cmdlen;
        STARTUPINFO si;
        PROCESS_INFORMATION pi;
        static char *funcname="k2pdfopt_launch_gui";

        GetStartupInfo(&si);
        GetModuleFileNameA(NULL,exename,511);
        cmdlen=strlen(exename)+(cmdline->s==NULL?0:strlen(cmdline->s))+16;
        willus_mem_alloc_warn((void **)&buf,cmdlen+16,funcname,10);
        sprintf(buf,"\"%s\"",exename);
        if (cmdline->s!=NULL && strlen(cmdline->s)>0)
            sprintf(&buf[strlen(buf)]," %s",cmdline->s);
        sprintf(&buf[strlen(buf)]," -gui+");
        memset(&pi,0,sizeof(PROCESS_INFORMATION));
        memset(&si,0,sizeof(STARTUPINFO));
        si.cb = sizeof(STARTUPINFO);
        si.dwX = 0; /* Ignored unless si.dwFlags |= STARTF_USEPOSITION */
        si.dwY = 0;
        si.dwXSize = 0; /* Ignored unless si.dwFlags |= STARTF_USESIZE */
        si.dwYSize = 0;
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_SHOWNORMAL;
        /* Launching from a console will NOT create new console. */
        CreateProcess(exename,buf,0,0,1,DETACHED_PROCESS,0,NULL,&si,&pi);
        }
    else
        {
        HINSTANCE hinst;
        /* Free console and launch the GUI window */
        hinst=GetModuleHandle(NULL);
/*
printf("\n\nNEED TO FREE CONSOLE!\n\n");
*/
        FreeConsole();
        k2gui_main(k2conv,hinst,NULL,env,cmdline);
        }
    }

#endif /* Windows GUI */
