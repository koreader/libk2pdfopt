/*
** k2sys.c     K2pdfopt system functions
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

#include "k2pdfopt.h"


void k2sys_init(void)

    {
    wsys_set_decimal_period(1);
    /* wrapbmp_init(); */
    }


void k2sys_close(K2PDFOPT_SETTINGS *k2settings)

    {
    /* wrapbmp_free(); */
    wsys_set_decimal_period(0);
    k2ocr_end(k2settings);
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
        aprintf("%s",mesg);
        fgets(buf,15,stdin);
        return;
        }
    /* If exit_on_complete < 0 */
    wsys_enter_to_exit(mesg);
    }


void k2sys_header(void)

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
    aprintf(TTEXT_HEADER "k2pdfopt %s" TTEXT_NORMAL "%s (c) %s, GPLv3, http://willus.com\n"
            "    Compiled %s with %s for %s on %s.\n\n",
           k2pdfopt_version,cap,
           &date[strlen(date)-4],
           __DATE__,k2pdfopt_compiler,k2pdfopt_os,k2pdfopt_chip);
    }
