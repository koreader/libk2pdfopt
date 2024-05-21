/*
** k2sys.c     K2pdfopt system functions
**
** Copyright (C) 2019  http://willus.com
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

#include "k2pdfopt.h"
#include <stdarg.h>

#ifdef __ANDROID__
#include <android/log.h>
#endif


void k2sys_init(void)

    {
    wsys_set_decimal_period(1);
#ifdef HAVE_OCR_LIB
    k2ocr_cpu_time_reset();
#endif
    /* wrapbmp_init(); */
    }


void k2sys_cpu_update(K2PDFOPT_SETTINGS *k2settings,double start_seconds,double stop_seconds)

    {
#ifdef HAVE_OCR_LIB
    if (k2settings->dst_ocr=='t' || k2settings->dst_ocr=='g')
        {
        int mt;
        double cpusecs;

        mt=k2ocr_max_threads();
        cpusecs=k2ocr_cpu_time_secs();
        k2printf("Total OCR CPU time used:  ");
        if (mt<=1)        
            k2printf("%.2f s\n",cpusecs);
        else
            {
#if (!(WIN32 || WIN64))
            cpusecs /= mt;
#endif
            k2printf("%.2f s per thread (%d threads)\n",cpusecs,mt);
            }
        }
#endif
    k2printf(TTEXT_NORMAL "Total CPU time used: %.2f s\n",stop_seconds-start_seconds);
    }


void k2sys_close(K2PDFOPT_SETTINGS *k2settings)

    {
    /* wrapbmp_free(); */
    wsys_set_decimal_period(0);
    k2ocr_end(k2settings);
#if (WILLUSDEBUGX & 0x100000)
    willus_dmem_check();
#endif
    }


void k2sys_exit(K2PDFOPT_SETTINGS *k2settings,int val)

    {
    k2sys_enter_to_exit(k2settings);
    k2sys_close(k2settings);
    exit(val);
    }


void k2sys_enter_to_exit(K2PDFOPT_SETTINGS *k2settings)

    {
    static char *mesg=TTEXT_BOLD2 "Press <ENTER> to exit." TTEXT_NORMAL;

    if (k2settings->exit_on_complete==1)
        return;
    if (k2settings->exit_on_complete==0)
        {
        char buf[16];
        k2printf("%s",mesg);
        k2gets(buf,15,"");
        return;
        }
    /* If exit_on_complete < 0 */
    wsys_enter_to_exit(mesg);
    }


void k2sys_header(char *s)

    {
    char date[32];
    char cap[32];
    char k2pdfopt_os[64];
    char k2pdfopt_chip[64];
    char k2pdfopt_compiler[64];
    extern char *k2pdfopt_version;

    strcpy(date,__DATE__);
#ifdef HAVE_MUPDF_LIB
    strcpy(cap," (w/MuPDF");
#else
    cap[0]='\0';
#endif
#ifdef HAVE_DJVU_LIB
    sprintf(&cap[strlen(cap)],"%sDjVuLibre",cap[0]=='\0'?" (w/":",");
#endif
#ifdef HAVE_OCR_LIB
    sprintf(&cap[strlen(cap)],"%sOCR",cap[0]=='\0'?" (w/":",");
#endif
    if (cap[0]=='\0')
        strcpy(cap," (Ghostscript only)");
    else
        strcat(cap,")");
    wsys_system_version(NULL,k2pdfopt_os,k2pdfopt_chip,k2pdfopt_compiler);
    if (s==NULL)
        k2printf(TTEXT_HEADER "k2pdfopt %s" TTEXT_NORMAL "%s (c) %s, GPLv3, http://willus.com\n"
            "    Compiled %s with %s for %s on %s.\n\n",
           k2pdfopt_version,cap,
           &date[strlen(date)-4],
           __DATE__,k2pdfopt_compiler,k2pdfopt_os,k2pdfopt_chip);
    else
        sprintf(s,"k2pdfopt %s%s\r\n"
                  "(c) %s, GPLv3, http://willus.com\r\n"
                  "Compiled %s\r\n"
                  "    with %s\r\n"
                  "    for %s on %s.",
           k2pdfopt_version,cap,
           &date[strlen(date)-4],
           __DATE__,k2pdfopt_compiler,k2pdfopt_os,k2pdfopt_chip);
    }



int k2printf(char *fmt,...)

    {
    va_list args;
    int     status;
    static void *k2printf_semaphore;
    static int count=0;
    
    if (count==0)
        k2printf_semaphore = willusgui_semaphore_create_ex("k2printf",1,1);
    count++;
    if (k2printf_semaphore)
        willusgui_semaphore_status_wait(k2printf_semaphore);
    status=0;
    va_start(args,fmt);
#ifdef HAVE_K2GUI
    if (k2gui_active())
        {
#if (WILLUSDEBUGX & 0x4000)
        status=avprintf(stdout,fmt,args);
        fflush(stdout);
#endif          
        if (k2gui_cbox_converting())
            status=k2gui_cbox_vprintf(stdout,fmt,args);
        else if (k2gui_overlay_converting())
            status=k2gui_overlay_vprintf(stdout,fmt,args);
        }
    else
#endif
#ifdef __ANDROID__
    {
    char buf[1024];
    status=vsnprintf(buf,sizeof(buf),fmt,args);
    __android_log_write(ANDROID_LOG_DEBUG,"k2pdfopt",buf);
    }
#else
    status=avprintf(stdout,fmt,args);
    fflush(stdout);
#endif
    va_end(args);
    willusgui_semaphore_release(k2printf_semaphore);
    return(status);
    }


void k2gets(char *buf,int maxlen,char *def)

    {
#ifdef HAVE_K2GUI
    if (k2gui_active())
        {
        strncpy(buf,def,maxlen-1);
        buf[maxlen-1]='\0';
        return;
        }
#endif
    if (fgets(buf,maxlen,stdin)==NULL)
        {
        strncpy(buf,def,maxlen-1);
        buf[maxlen-1]='\0';
        }
    }

/*
void k2pdfopt_non_ascii_warning(FILE *out)

    {
    afprintf(out,"\a\n\n" ANSI_RED
         "*************************************************************\n"
         "**                                                         **\n"
         "** " ANSI_YELLOW "Your command-line arguments have non-ASCII characters!" ANSI_RED "  **\n"
         "**                                                         **\n"
         "** " ANSI_YELLOW "This may cause the k2pdfopt conversion to fail!" ANSI_RED "         **\n"
         "**                                                         **\n"
         "*************************************************************\n"
         "\n");
    }
*/
