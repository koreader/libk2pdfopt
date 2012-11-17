/*
** wsys.c       System / OS-dependent routines
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <locale.h>
#ifdef WIN32
#include <windows.h>
#endif
#ifdef UNIX
#include <errno.h>
#include <unistd.h>
#endif

/*
Digital Mars:  __DMC__ == 0x700 (7.0) 0x720 (7.2) 0x800 (8.0)
*/

/*
     OS/X predefines:
         #define __MACH__ 1
         #define __NATURAL_ALIGNMENT__ 1
         #define __APPLE__ 1
         #define __GNUC_MINOR__ 95
         #define __ppc__ 1
         #define __GNUC__ 2
         #define __STDC__ 1
         #define __DYNAMIC__ 1
         #define __APPLE_CC__ 926
         #define __BIG_ENDIAN__ 1
*/
#if (defined(__GNUC__))
static void gnu_compiler(char *compiler_version);
#endif
static int wsys_decimal_is_period(void);


void wsys_system_version(char *system,char *_os,char *_chip,char *_compiler)

    {
    char compiler_version[80],compname[100];
    int     ccode,oscode,chipcode;
    static char *compiler[] = {"Unknown C compiler","Watcom C","Gnu C",
                               "HPUX ANSI C","Sun C","Cray C","Turbo C",
                               "Microsoft Visual C++","Gnu C (DJGPP)",
                               "Gnu C (RSXNT/EMX)","Intel C/C++","HPUX C++",
                               "Digital Mars C/C++","LCC","Watcom C/C++",
                               "Borland C/C++","Gnu C (Mingw32)",
                               "Intel C++ for Linux","Gnu C (Mingw64)","Tiny CC"};
    static char *os[] = {"Unknown O/S","Unix","VMS","Unicos","SunOS","HPUX",
                         "MS-DOS","Win32","MS-DOS (32-bit)","OS/X",
                         "Linux","SuSE Linux","Win64"};
    static char *chip[] = {"Unknown architecture","CRAY2","CRAY","hppa 1.0",
                           "hppa 1.1","sparc","i386","hppa 2.0","PPC","x64"};

    ccode=0;
    oscode=0;
    chipcode=0;
    compiler_version[0]='\0';

    /* O/S's (more general) */
#if (defined(UNIX))
    oscode=1;
#elif (defined(__vax__) || defined(__vax) || defined(__vms))
    oscode=2;
#endif

    /* Specific O/S's */
#if (defined(WIN64))
    oscode=12;
#elif (defined(WIN32))
    if (wsys_win32_api())
        oscode=7;
    else
        oscode=8;
    chipcode=6;
#elif (defined(MSDOS32))
    oscode=8;
    chipcode=6;
#elif (defined(MSDOS))
    oscode=6;
    chipcode=6;
#elif (defined(__linux) && defined(__VERSION__))
    if (in_string(__VERSION__,"SuSE")>=0)
        oscode=11;
    else
        oscode=10;
#elif (defined(__linux))
    oscode=10;
#elif (defined(__sun) || defined(sun))
    oscode=4;
    ccode=4;
#elif (defined(hpux) || defined(__hpux))
    ccode=3;
    oscode=5;
    ccode=3;
#if (defined(__cplusplus))
    ccode=11;
#endif
#elif (defined(_CRAY))
    ccode=5;
    oscode=3;
    chipcode=2;
#elif (defined(_CRAY2))
    ccode=5;
    oscode=3;
    chipcode=1;
#elif (defined(__ppc__) || defined(__MACH__))
    oscode=9;
#endif

    /* Specific chips */
#if (defined(i386) || defined(__i386__) || defined(__i386) || defined(__386__) || defined(_M_IX86))
    chipcode=6;
#elif (defined(__x86_64) || defined(__amd64__))
    chipcode=9;
#elif (defined(__sparc) || defined(sparc))
    chipcode=5;
#elif (defined(_PA_RISC1_1))
    chipcode=4;
#elif (defined(_PA_RISC2_0))
    chipcode=7;
#elif (defined(__ppc__))
    chipcode=8;
#endif

    /* Specific compilers */
#if (defined(__DMC__))
    ccode=12;
    sprintf(compiler_version,"v%d.%d",((__DMC__ &0xf00)>>8),((__DMC__ &0xf0)>>4));
#elif (defined(__LCC__))
    ccode=13;
    strcpy(compiler_version,"3.7");
#elif (defined(__BORLANDC__))
    ccode=15;
    sprintf(compiler_version,"v%d.%d.%d",
             (__BORLANDC__ >> 8)&0xf,
             (__BORLANDC__ >> 4)&0xf,
             (__BORLANDC__ )&0xf);
#elif (defined(__WATCOMC__))
    ccode=14;
    sprintf(compiler_version,"%.2f",(double)__WATCOMC__/100.);
#elif (defined(DJGPP))
    ccode=8;
    gnu_compiler(compiler_version);
#elif (defined(__MINGW64__))
    ccode=18;
    gnu_compiler(compiler_version);
#elif (defined(__MINGW32__))
    ccode=16;
    gnu_compiler(compiler_version);
#elif (defined(EMX))
    ccode=9;
    gnu_compiler(compiler_version);
#elif (defined(__GNUC__))
    ccode=2;
    gnu_compiler(compiler_version);
#elif (defined(__TURBOC__))
    ccode=6;
#elif (defined(__ICL))
    ccode=10;
    sprintf(compiler_version,"v%4.2f",__ICL/100.);
#elif (defined(_MSC_VER))
    ccode=7;
    sprintf(compiler_version,"v%4.2f",_MSC_VER/100.-6.0);
#elif (defined(__386__))
    ccode=1;
#elif (defined(__TINYC__))
    ccode=19;
    compiler_version[0]='\0';
#elif (defined(__INTEL_COMPILER))
#if (defined(__linux__))
    ccode=17;
#else
    ccode=10;
#endif
    sprintf(compiler_version,"v%4.2f",__INTEL_COMPILER/100.);
#endif

    if (compiler_version[0]!='\0')
        sprintf(compname,"%s %s",compiler[ccode],compiler_version);
    else
        strcpy(compname,compiler[ccode]);
    if (system!=NULL)
        sprintf(system,"%s, %s, %s",os[oscode],chip[chipcode],compname);
    if (_os!=NULL)
        strcpy(_os,os[oscode]);
    if (_chip!=NULL)
        strcpy(_chip,chip[chipcode]);
    if (_compiler!=NULL)
        strcpy(_compiler,compname);
    }


#if (defined(__GNUC__))
static void gnu_compiler(char *compiler_version)

    {
#if (defined(__GNUC_PATCHLEVEL__))
    sprintf(compiler_version,"v%d.%d.%d",__GNUC__,__GNUC_MINOR__,__GNUC_PATCHLEVEL__);
#else
    sprintf(compiler_version,"v%d.%d.x",__GNUC__,__GNUC_MINOR__);
#endif
    }
#endif


int wsys_win32_api(void)

    {
#if (defined(DJEMXWIN32))
    /* extern int _emx_env,_emx_rev; */
    if (_emx_env & 0x1000L)
        return((_emx_rev>>16)==2);
    else
        return(0);
#elif (defined(WIN32))
        return(1);
#else
    return(0);
#endif
    }


/*
** Return the status of a process.
**
** This function has only been tested on Win32 and HPUX as of 2-5-2002.
** Only returns WPID_RUNNING or WPID_NO_PROCESS (can't determine if
** process is sleeping yet).
*/
int wsys_wpid_status(int pid)

    {
#ifdef WIN32
    HANDLE prochandle;
    prochandle = OpenProcess(PROCESS_QUERY_INFORMATION,0,pid);
    if (prochandle==NULL)
        {
        /* Typically, GetLastError() here will return */
        /* a value of 87--Incorrect Parameter         */
        /*
        printf("open process fails, last error = %d\n",status);
        printf(" = %s\n",win_lasterror());
        */
        return(WPID_NO_PROCESS);
        }
    CloseHandle(prochandle);
    return(WPID_RUNNING);
    /*
    status = GetExitCodeProcess(prochandle,&exitcode);
    if (status==0)
        {
        printf("Error from GetExitCodeProcess = %s\n",win_lasterror());
        return(WPID_UNKNOWN);
        }
    printf("Proc exit code = %d (STILL_ACTIVE=%d)\n",exitcode,STILL_ACTIVE);
    if (exitcode==STILL_ACTIVE)
        return(WPID_RUNNING);
    return(WPID_NO_PROCESS);
    */
#else
#ifdef UNIX
    int status;
    status = getpgid(pid);
    if (status < 0 && errno==ESRCH)
        {
        /*
        printf("errno=%d, ESRCH=%d, EPERM (not in same session)=%d\n",errno,ESRCH,EPERM);
        */
        return(WPID_NO_PROCESS);
        }
    return(WPID_RUNNING);
#else
    return(WPID_UNKNOWN);
#endif /* UNIX */
#endif /* WIN32 */
    }


void wsys_sleep(int secs)

    {
#ifdef WIN32
    win_sleep(secs*1000);
#else
    sleep(secs);
#endif
    }


char *wsys_full_exe_name(char *s)

    {
#if (defined(WIN32) || defined(WIN64))
    return(win_full_exe_name(s));
#else
    if (!wfile_is_symlink_ex("/proc/self/exe",s))
        s[0]='\0';
    return(s);
#endif
    }


int wsys_which(char *exactname,char *exename)

    {
#ifdef WIN32
    return(win_which(exactname,exename));
#else
#if (defined(LINUX) || defined(UNIXPURE))
    return(linux_which(exactname,exename));
#else
    return(0);
#endif
#endif
    }


int wsys_most_recent_in_path(char *exename,char *wildcard)

    {
#ifdef WIN32
    return(win_most_recent_in_path(exename,wildcard));
#else
#if (defined(LINUX) || defined(UNIXPURE))
    return(linux_most_recent_in_path(exename,wildcard));
#else
    return(0);
#endif
#endif
    }


void wsys_computer_name(char *name,int maxlen)

    {
#ifdef WIN32
    int n;
    n=maxlen;
    GetComputerName(name,(LPDWORD)&n);
#else
    gethostname(name,maxlen);
#endif
    }


void wsys_enter_to_exit(char *mesg)

    {
#if (defined(WIN32) || defined(WIN64))
    if (win_has_own_window())
#endif
        {
        char buf[16];

        if (mesg==NULL || mesg[0]=='\0')
            printf("Press <ENTER> to exit.");
        else
            aprintf("%s",mesg);
        fgets(buf,15,stdin);
        }
    }


int wsys_filename_8dot3(char *dst,char *src,int maxlen)

    {
#if (defined(WIN32) || defined(WIN64))
    return(GetShortPathName(src,dst,maxlen));
#else
    strncpy(dst,src,maxlen-1);
    dst[maxlen-1]='\0';
    return(strlen(dst));
#endif
    }


double wsys_year_double(struct tm *date)

    {
    return(1900.+date->tm_year
                +date->tm_mon/12.
                +((date->tm_mday-1.0)/365.25)
                +date->tm_hour/(365.25*24.)
                +date->tm_min/(365.25*24.*60.)
                +date->tm_sec/(365.25*24.*3600.));
    }


double wsys_utc_offset(void)

    {
    time_t now;
    struct tm t1,t2;
    double tz;

    time(&now);
    t1=(*localtime(&now));
    t2=(*gmtime(&now));
    tz=wfile_date_diff(&t1,&t2)/3600.;
    return(tz);
    }


char *wsys_utc_string(void)

    {
    double tz;
    int c,hr,min;
    static char buf[8];

    tz = wsys_utc_offset();
    if (tz<0)
        {
        tz = -tz;
        c='-';
        }
    else
        c='+';
    hr = (int)(tz+1e-6);
    min=(tz-hr)*60;
    min = (min+8)/15;
    min *= 15;
    sprintf(buf,"%c%02d'%02d",c,hr%24,min);
    return(buf);
    }


/*
** Call with 1 to set decimal point to a period.
** Call with 0 to restore locale to original value.
*/
int wsys_set_decimal_period(int setit)

    {
    static int sys_setlocale=0;
    static char orglocale[64];
    static char *localenames[] = {"C","en_US","en_us","English_United States","English",
                                  "english",""};
    int i;
    char *p;

    if (!setit)
        {
        if (sys_setlocale)
            setlocale(LC_NUMERIC,orglocale);
        return(1);
        }
    if (wsys_decimal_is_period())
        return(1);
    /* Store original locale */
    if (!sys_setlocale)
        {
        p=setlocale(LC_NUMERIC,NULL);
        strncpy(orglocale,p==NULL?"":p,63);
        orglocale[63]='\0';
        sys_setlocale=1;
        }
    /* Try new locales until the decimal point is a period. */
    for (i=0;1;i++)
        {
        p=setlocale(LC_NUMERIC,localenames[i]);
        if (wsys_decimal_is_period())
            return(1);
        if (localenames[i][0]=='\0')
            break;
        }
    setlocale(LC_NUMERIC,orglocale);
    return(0);
    }


static int wsys_decimal_is_period(void)

    {
    double x;
    char buf[16];

    x=12.34;
    sprintf(buf,"%5.2f",x);
    if (buf[2]!='.')
        return(0);
    strcpy(buf,"12.34");
    return(fabs(atof(buf)-x) < .01);
    }
