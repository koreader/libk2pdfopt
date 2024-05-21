/*
** wgs.c    Stubs for calls to Ghostscript DLL
**
** Part of willus.com general purpose C code library.
**
** Copyright (C) 2020  http://willus.com
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

/*
** 5-31-2020
** - Set gsinst = NULL before call to gs_new_instance--this is required.
** - Set argument encoding for UTF-8 strings
** - Don't check status returns for negative vals--they aren't expected.
/*
** 1. If Linux, look in path, look in exe dir if possible, look in current dir,
**    look in opt/gs*, WILLUSGS_PATH envvar.
**
** 2. If Windows, look in path, cur dir, exe dir, x:\program files\gs*,
**    x:\program files (x86)\gs*, x:\gs\*, WILLUSGS_PATH env var.
**
*/
#include "willus.h"

#if (defined(WIN32) || defined(WIN64))
#include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_GHOSTSCRIPT
/*
** DLL handle
*/
#define GS_ARG_ENCODING_LOCAL    0
#define GS_ARG_ENCODING_UTF8     1
#define GS_ARG_ENCODING_UTF16LE  2

#if (defined(WIN32) || defined(WIN64))
static HINSTANCE willusgs_lib = NULL;
#endif
static int willusgs_inited=0;
static char willusgs_name[512];
static int willusgs_isdll=0;
static int willusgs_device_width_pts=-1;
static int willusgs_device_height_pts=-1;

/*
** Pointers which will get pointed to the DLL functions
*/
#if (defined(WIN32) || defined(WIN64))
typedef int (__stdcall *apifunc_ptr)(void *,void *);
typedef int (__stdcall *apifunc_int_chararray)(void *,int,char *[]);
typedef int (__stdcall *apifunc_void)(void *);
typedef int (*stdfunc)(void *caller_handle,const char *buf,int len);
typedef int (__stdcall *apifunc_stdfuncx3)(void *instance,stdfunc stdin_fn,
                                          stdfunc stdout_fn,stdfunc stderr_fn);
typedef int (__stdcall *apifunc_int)(void *,int);

apifunc_ptr           gsapi_new_instance;
apifunc_int_chararray gsapi_init_with_args;
apifunc_void          gsapi_exit;
apifunc_void          gsapi_delete_instance;
apifunc_stdfuncx3     gsapi_set_stdio;
apifunc_int           gsapi_set_arg_encoding;
/*
static int (__stdcall *gsapi_set_stdio)(void *instance, int(*stdin_fn)(void *caller_handle, char *buf, int len), int(*stdout_fn)(void *caller_handle, const char *str, int len), int(*stderr_fn)(void *caller_handle, const char *str, int len)); 
*/
static int gsdll_stdio(void *instance,const char *str,int len);
#endif

/*
** Sample use
**
int main(int argc,char *argv[])

    {
    WILLUSBITMAP *bmp,_bmp;

    bmp=&_bmp;
    bmp_init(bmp);
    willusgs_read_pdf_or_ps_to_png(bmp,"myfile.pdf",1,200,stdout);
    ...
    bmp_free(bmp);
    willusgs_close();
    }
*/
/* Use -1 to ignore or use default */
void willusgs_set_device_width_and_height_pts(int w,int h)

    {
    willusgs_device_width_pts=w;
    willusgs_device_height_pts=h;
    }


#define NARGSMAX 18

int willusgs_read_pdf_or_ps_bmp(WILLUSBITMAP *bmp,char *filename,int pageno,double dpi,FILE *out)

    {
    char argdata[NARGSMAX][48];
    char *argv[NARGSMAX];
    int i,status;
    char tempfile[256];
    char argtemp[280];
    char srcfile[256];

    willusgs_init(out);
    wfile_abstmpnam(tempfile);
    for (i=0;i<NARGSMAX;i++)
        argv[i]=&argdata[i][0];
    i=0;
    strcpy(argv[i++],"-q");
    strcpy(argv[i++],"-P-");
    strcpy(argv[i++],"-dSAFER");
    strcpy(argv[i++],"-dBATCH");
    strcpy(argv[i++],"-dNOPAUSE");
    strcpy(argv[i++],"-sDEVICE=png16m");
/*
printf("Setting dev to %d x %d pts (%g dpi)\n",willusgs_device_width_pts,willusgs_device_height_pts,dpi);
printf("==> bitmap should be %d x %d\n",(int)(willusgs_device_width_pts*dpi/72.+.5),
                                        (int)(willusgs_device_height_pts*dpi/72.+.5));
*/
    if (willusgs_device_width_pts>0)
        sprintf(argv[i++],"-dDEVICEWIDTHPOINTS=%d",willusgs_device_width_pts);
    if (willusgs_device_height_pts>0)
        sprintf(argv[i++],"-dDEVICEHEIGHTPOINTS=%d",willusgs_device_height_pts);
    strcpy(argv[i++],"-dGraphicsAlphaBits=4");
    strcpy(argv[i++],"-dTextAlphaBits=4");
    sprintf(argv[i++],"-r%g",dpi);
    sprintf(argv[i++],"-dFirstPage=%d",pageno<=0?1:pageno);
    sprintf(argv[i++],"-dLastPage=%d",pageno<=0?1:pageno);
    argv[i++]=&argtemp[0]; 
    sprintf(argtemp,"-sOutputFile=%s",tempfile);
    argv[i++]=&srcfile[0];
    strcpy(srcfile,filename);
    status=willusgs_exec(i,argv,out);
    if (status)
        return(status);
    status=bmp_read_png(bmp,tempfile,out);
    if (status<0)
        {
        nprintf(out,"BMP read failed.  GS output in file %s (not deleted).\n",tempfile);
        return(status-100);
        }
/*
printf("bitmap is %d x %d x %d\n",bmp->width,bmp->height,bmp->bpp);
*/
    remove(tempfile);
    return(0);
    }

/*
** Source file can be PS or PDF
** Google "ps2pdf options" to see more about these options.
** 3-6-16:  Removed page numbers--they don't work.  Converts the whole document.
*/
int willusgs_ps_to_pdf(char *dstfile,char *srcfile,FILE *out)

    {
    char argdata[16][32];
    char *argv[16];
    int i,status;
    char argtemp[280];
    char argsrc[280];

    willusgs_init(out);
    for (i=0;i<16;i++)
        argv[i]=&argdata[i][0];
    i=0;
    /* -dSAFER reduces security risk from malicious PS/PDF files */
    strcpy(argv[i++],"-q");
    strcpy(argv[i++],"-P-");
    strcpy(argv[i++],"-dSAFER");
    strcpy(argv[i++],"-dBATCH");
    strcpy(argv[i++],"-dNOPAUSE");
    strcpy(argv[i++],"-sDEVICE=pdfwrite");
    if (willusgs_device_width_pts>0 && willusgs_device_width_pts<10000000)
        sprintf(argv[i++],"-dDEVICEWIDTHPOINTS=%d",willusgs_device_width_pts);
    if (willusgs_device_height_pts>0 && willusgs_device_height_pts<10000000)
        sprintf(argv[i++],"-dDEVICEHEIGHTPOINTS=%d",willusgs_device_height_pts);
    strcpy(argv[i++],"-dPDFSETTINGS=/prepress");
    /*
    if (firstpage>0)
        sprintf(argv[i++],"-dFirstPage=%d",firstpage);
    if (lastpage>0)
        sprintf(argv[i++],"-dLastPage=%d",lastpage);
    */
    argv[i++]=&argtemp[0]; 
    sprintf(argtemp,"-sOutputFile=%s",dstfile);
    /* CompatibilityLevel=1.4 is the default--google ps2pdf options */
/*
    strcpy(argv[i++],"-dCompatibilityLevel=1.4");
    strcpy(argv[i++],"-c");
    strcpy(argv[i++],".setpdfwrite");
*/
    argv[i++]=&argsrc[0];
    sprintf(argsrc,"%s",srcfile);
    status=willusgs_exec(i,argv,out);
    return(status);
    }


    
int willusgs_init(FILE *out)

    {
    int i,status;
#if (defined(WIN32) || defined(WIN64))
#if (defined(WIN64))
    static char *gsnames[]={"gsdll64.dll","gswin64c.exe","gsdll32.dll","gswin32c.exe",""};
#else
    static char *gsnames[]={"gsdll32.dll","gswin32c.exe",""};
#endif
    static char *folders[]={"+\\Program Files\\gs","+\\Program Files (x86)\\gs",
                            "+\\Program Files\\ghostgum","+\\Program Files (x86)\\ghostgum",
                            "\\Program Files\\gs*","\\Program Files (x86)\\gs*",
                            "\\Program Files\\ghostgum*","\\Program Files (x86)\\ghostgum*",
                            "+\\gs","+\\ghostgum","\\gs*","\\ghostgum*",""};
#else
    static char *gsnames[]={"gs",""};
    static char *folders[]={"/usr/bin","/usr/share/gs","/usr/local/gs","/opt/gs",
                            "/usr/share/gs*","/usr/local/gs*","/opt/gs*",""};
#endif
    

    if (willusgs_inited)
        return(0);
    willusgs_name[0]='\0';
    willusgs_isdll=0;
    /*
    ** Look in registry first
    */
#if (defined(WIN32) || defined(WIN64))
    if (!win_registry_search(willusgs_name,511,"GS_DLL","software\\gpl ghostscript",1))
        {
        if (LoadLibrary(willusgs_name)!=NULL)
            FreeLibrary((HMODULE)willusgs_name);
        else
            {
            char basepath[511];
            wfile_basepath(basepath,willusgs_name);
#if (defined(WIN64))
            wfile_fullname(willusgs_name,basepath,"gswin64c.exe");
            if (wfile_status(willusgs_name)!=1)
                {
#endif
                wfile_fullname(willusgs_name,basepath,"gswin32c.exe");
                if (wfile_status(willusgs_name)!=1)
                    willusgs_name[0]='\0';
#if (defined(WIN64))
                }
#endif
            }
        }
#endif
    /*
    ** Search WILLUSGS_PATH and/or common folders
    */
    if (willusgs_name[0]=='\0')
        {
        for (i=0;gsnames[i][0]!='\0';i++)
            {
            status=wfile_find_file(willusgs_name,gsnames[i],folders,"cde",1,1,1,"WILLUSGS_PATH");
            if (!status)
                {
                break;
                }
            }
        if (gsnames[i][0]=='\0')
            willusgs_name[0]='\0';
        }
    /*
    ** Still not found??
    */
    if (willusgs_name[0]=='\0' || wfile_status(willusgs_name)!=1)
        {
        nprintf(out,"\n\aCan't find ghostscript command file: ");
        for (i=0;gsnames[i][0]!='\0';i++)
            nprintf(out,"%s%s",i>0?" or ":" ",gsnames[i]);
        nprintf(out,"\n\nIf you have not installed Ghostscript, please install it.\n"
                    "(Google it to find the install package.)\n"
                    "Otherwise, set the environment variable WILLUSGS_PATH to point to\n"
                    "the folder where the ghostscript command file is located.\n");
        return(-1);
        }
    else
        nprintf(out,"Ghostscript found:  '%s'\n",willusgs_name);
#if (defined(WIN32) || defined(WIN64))
    willusgs_isdll = (!stricmp(wfile_ext(willusgs_name),"dll"));
    if (willusgs_isdll)
        {
        willusgs_lib=LoadLibrary(willusgs_name);
        if (willusgs_lib==NULL)
            {
            nprintf(out,"Internal error!  DLL load failed for %s!\n",willusgs_name);
            nprintf(out,"%s\n",win_lasterror());
            return(-2);
            }
        if (    (gsapi_new_instance = (apifunc_ptr)GetProcAddress(willusgs_lib, (LPCSTR)"gsapi_new_instance"))==NULL
             || (gsapi_set_arg_encoding = (apifunc_int)GetProcAddress(willusgs_lib, (LPCSTR)"gsapi_set_arg_encoding"))==NULL
             || (gsapi_init_with_args = (apifunc_int_chararray)GetProcAddress(willusgs_lib, (LPCSTR)"gsapi_init_with_args"))==NULL
             || (gsapi_exit = (apifunc_void)GetProcAddress(willusgs_lib, (LPCSTR)"gsapi_exit"))==NULL
             || (gsapi_delete_instance = (apifunc_void)GetProcAddress(willusgs_lib, (LPCSTR)"gsapi_delete_instance"))==NULL
             || (gsapi_set_stdio = (apifunc_stdfuncx3)GetProcAddress(willusgs_lib, (LPCSTR)"gsapi_set_stdio"))==NULL)
            {
            willusgs_close();
            nprintf(out,"Can't find entry points in gsdll32.dll.\n");
            return(-3);
            }
        }
#endif
    willusgs_inited=1;
    willusgs_device_width_pts=willusgs_device_height_pts=-1;
    return(0);
    }


int willusgs_exec(int argc,char *argv[],FILE *out)

    {
    int i,status;
    char cmd[1024];

/*
{
int i;
printf("willusgs_exec:\n");
for (i=0;i<argc;i++)
printf("    argv[%d]='%s'\n",i,argv[i]);
}
*/
    willusgs_init(out);
#if (defined(WIN32) || defined(WIN64))
    if (willusgs_isdll)
        {
        int status,status1;
        void *gsinst;

        gsinst=NULL; /* Must be set to NULL before call to gsapi_new_instance */
        status=(*gsapi_new_instance)(&gsinst,NULL);
        if (status)
            {
            nprintf(out,"Cannot create GS instance!\n");
            return(-1);
            }
        status=(*gsapi_set_arg_encoding)(gsinst,GS_ARG_ENCODING_UTF8);
        if (status)
            {
            nprintf(out,"Cannot set GS encoding!\n");
            return(-2);
            }
        /* Don't echo chars to screen */
        gsapi_set_stdio(gsinst,NULL,gsdll_stdio,gsdll_stdio);
/*
printf("Calling dll...\n");
for (i=0;i<argc;i++)
printf("    argv[%d]='%s'\n",i,argv[i]);
*/
        status=(*gsapi_init_with_args)(gsinst,argc,argv);
        status1=(*gsapi_exit)(gsinst);
        if (status==0 || status==-101)
            status=status1;
        (*gsapi_delete_instance)(gsinst);
        if (status==0 || status==-101)
            return(0);
        return(-2);
        }
    strcpy(cmd,"\"");
#else
    cmd[0]='\0';
#endif
    sprintf(&cmd[strlen(cmd)],"\"%s\"",willusgs_name);
    for (i=1;i<argc;i++)
        sprintf(&cmd[strlen(cmd)]," \"%s\"",argv[i]);
/*
#if (defined(WIN32) || defined(WIN64))
    strcat(cmd," 1> nul 2> nul\"");
#else
    strcat(cmd," 1> /dev/null 2>&1");
#endif
*/
    status=system(cmd);
    return(status);
    }


#if (defined(WIN32) || defined(WIN64))
static int gsdll_stdio(void *instance,const char *str,int len)

    {
    return(len);
    }
#endif


void willusgs_close(void)

    {
#if (defined(WIN32) || defined(WIN64))
    if (willusgs_isdll && willusgs_lib!=NULL)
        FreeLibrary(willusgs_lib);
    willusgs_lib = NULL;
#endif
    willusgs_inited = 0;
    }
#endif /* HAVE_GHOSTSCRIPT */
