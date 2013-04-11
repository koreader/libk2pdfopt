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


int main(int argc,char *argv[])

    {
    int i;
    static K2PDFOPT_CONVERSION _k2conv,*k2conv;
    K2PDFOPT_SETTINGS *k2settings;
    static STRBUF _cmdline,_env,_usermenu;
    STRBUF *cmdline,*env,*usermenu;

    k2conv=&_k2conv;
    k2settings=&k2conv->k2settings;
    cmdline=&_cmdline;
    env=&_env;
    usermenu=&_usermenu;
    strbuf_init(cmdline);
    strbuf_init(env);
    strbuf_init(usermenu);
    strbuf_cpy(env,getenv("K2PDFOPT"));
    for (i=1;i<argc;i++)
        strbuf_cat_with_quotes(cmdline,argv[i]);
    k2sys_init();
    k2pdfopt_settings_init(k2settings);
    k2pdfopt_files_init(&k2conv->k2files);
    /* Only set ansi and user interface */
    parse_cmd_args(k2conv,env,cmdline,usermenu,2);
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
                k2pdfopt_files_free(&k2conv->k2files);
                k2sys_close(k2settings);
                strbuf_free(usermenu);
                strbuf_free(env);
                strbuf_free(cmdline);
                return(0);
                }
            }
        if (k2settings->query_user!=0)
            k2sys_enter_to_exit(k2settings);
        k2pdfopt_files_free(&k2conv->k2files);
        k2sys_close(k2settings);
        strbuf_free(usermenu);
        strbuf_free(env);
        strbuf_free(cmdline);
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
    parse_cmd_args(k2conv,env,cmdline,usermenu,1);
    /*
    ** Get user input
    */
    if (k2pdfopt_menu(k2conv,env,cmdline,usermenu)==-1)
        {
        strbuf_free(usermenu);
        strbuf_free(env);
        strbuf_free(cmdline);
        k2pdfopt_files_free(&k2conv->k2files);
        k2sys_close(k2settings);
        return(0);
        }
    /*
    ** Re-init and then re-parse after all user menu entries applied.
    */
    k2pdfopt_settings_init(k2settings);
    parse_cmd_args(k2conv,env,cmdline,usermenu,1);

    /*
    ** Process files
    */
    for (i=0;i<k2conv->k2files.n;i++)
        k2pdfopt_proc_wildarg(k2settings,k2conv->k2files.file[i],1,NULL);

    /*
    ** All done.
    */
    strbuf_free(usermenu);
    strbuf_free(env);
    strbuf_free(cmdline);
    k2pdfopt_files_free(&k2conv->k2files);
    k2sys_enter_to_exit(k2settings);
    k2sys_close(k2settings);
    return(0);
    }
