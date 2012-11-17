/*
** wfile.c      Multiplatform file utilities for WIN32, DOS, and UNIX.
**              Can be compiled under gcc for unix, DJGPP, MS Visual C++,
**              or Turbo C.
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

#if (defined(UNIX) && defined(WIN32))
#undef UNIX
#endif
#if (defined(WIN32))
#include <windows.h>
#endif
#include <stdio.h>
#ifdef MSDOS16
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <dos.h>
#include <dir.h>
#endif
#ifdef UNIX
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#endif
/* Get rmdir() prototype for MINGW--not sure why I have to do this. */
#ifdef MINGW
int rmdir(const char *);
#endif

#ifdef DJEMXWIN32
#define HFILE_ERROR ((HFILE)-1)
#endif

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define relation(x,y)  ((x)<(y) ? -1 : 1)
#define COMPARE(x)     if ((d1->x)!=(d2->x)) \
                           return (relation(d1->x,d2->x))


#ifdef SLASH
#undef SLASH
#endif
#ifdef COLON
#undef COLON
#endif
#if (defined(WIN32) || defined(MSDOS))
#define SLASH   '\\'
#define ALLFILES    "*.*"
#define COLON
#else
#define SLASH   '/'
#define ALLFILES    "*"
#endif


/*
static int divider(int c);
*/


#ifdef UNIX
static int unix_findfirst(const char *spec,wfile *wptr);
static int unix_findnext(wfile *wptr);
static void unix_close(wfile *wptr);
static int unix_is_regular_file(char *filename);
static double unix_size(char *filename);
static int unix_status(char *filename);
static int unix_is_symlink(char *filename,char *src);
static int unix_date(const char *filename,struct tm *filedate);
/*
static int dos_style_match(char *pattern,char *name);
*/
#endif /* UNIX */

#ifdef MSDOS16
static int msdos_findfirst(const char *spec,wfile *wptr);
static int msdos_findnext(wfile *wptr);
static int msdos_date(const char *filename,struct tm *filedate);
static int msdos_status(char *filename);
#endif

/*
static char *spaces(int level);
*/
static int wfile_remove_dir_1(char *path,int recursive);
static int wfile_recaddone(RFIND *rf);
static void wfile_wfile_init(wfile *wf);
static FILIST *wfile_lastptr(RFIND *rf);
static void wfile_wf2rf(RFIND *rf);
static int wfile_recfreelast(RFIND *rf);
static int wfile_correct_exe(char *basename,char *correctname,char *fullname);

/* Prevent DJGPP from globbing */
#ifdef DJGPP
void *__crt0_glob_function(void);
void *__crt0_glob_function(void)

    {
    return(NULL);
    }
#endif




int wfile_is_archive(char *filename)

    {
    static char *archexts[]={"zip","7z",""};
    int i;

    for (i=0;archexts[i][0]!='\0';i++)
        if (!stricmp(wfile_ext(filename),archexts[i]))
            return(1);
    return(0);
    }


double wfile_file_age_secs(char *filename)

    {
    time_t tnow;
    struct tm date;
    struct tm fdate;
    double secs;

    time(&tnow);
    date=(*localtime(&tnow));
    if (!wfile_date(filename,&fdate))
        return(-1.0);
    secs=wfile_date_diff(&date,&fdate);
    if (secs<0.)
        secs=0.;
    return(secs);
    }


/*
** Returns the difference between the two dates (d2-d1) in seconds.
*/
double wfile_date_diff(struct tm *d2,struct tm *d1)

    {
    time_t t1,t2;

    t1=mktime(d1);
    t2=mktime(d2);
    return(difftime(t2,t1));
    }


int wfile_be_read(void *ptr,int elsize,int nobj,FILE *f)

    {
#ifdef WILLUS_BIGENDIAN
    return(fread(ptr,elsize,nobj,f));
#else
    char *a;
    int i,j,status,n2,nread;

    if (elsize<2)
        return(fread(ptr,elsize,nobj,f));
    a=(char *)ptr;
    n2=elsize/2;
    nread=0;
    for (i=0,a=(char *)ptr;i<nobj;i++,a+=elsize)
        {
        if ((status=fread(a,elsize,1,f))<1) 
            return(nread);
        nread++;
        for (j=0;j<n2;j++)
            {
            char c;
            c=a[j];
            a[j]=a[elsize-j-1];
            a[elsize-j-1]=c; 
            }
        }
    return(nread);
#endif
    }


int wfile_be_write(void *ptr,int elsize,int nobj,FILE *f)

    {
#ifdef WILLUS_BIGENDIAN
    return(fwrite(ptr,elsize,nobj,f));
#else
    char *a,*b;
    int i,j,status,n2,nwritten;
    static char *funcname="wfile_be_write";
    void *vp;
    double *dp;

    if (elsize<2)
        return(fwrite(ptr,elsize,nobj,f));
    a=(char *)ptr;
    n2=elsize/2;
    nwritten=0;
    willus_mem_alloc_warn(&vp,elsize,funcname,10);
    b=(char *)vp;
    for (i=0,a=(char *)ptr;i<nobj;i++,a+=elsize)
        {
        memcpy(b,a,elsize);
        for (j=0;j<n2;j++)
            {
            char c;
            c=b[j];
            b[j]=b[elsize-j-1];
            b[elsize-j-1]=c; 
            }
        if ((status=fwrite(b,elsize,1,f))<1) 
            {
            dp=(double *)b;
            willus_mem_free(&dp,funcname);
            b=(char *)dp;
            return(nwritten);
            }
        nwritten++;
        }
    dp=(double *)b;
    willus_mem_free(&dp,funcname);
    b=(char *)dp;
    return(nwritten);
#endif
    }


int wfile_ascii(char *filename,int maxcheck)

    {
    int i,c;
    FILE *f;

    f=fopen(filename,"rb");
    if (f==NULL)
        return(0);
    for (i=0;i<maxcheck;i++)
        {
        c=fgetc(f);
        if (c==EOF)
            break;
        if (!((c>=7 && c<=13) || (c>=32 && c<=127)))
            {
            fclose(f);
            return(0);
            }
        }
    fclose(f);
    return(1);
    }


int wfile_is_zipfile(char *filename)

    {
    return(!stricmp(wfile_ext(filename),"zip"));
    }


char *wfile_ext(char *dst)

    {
    int i;

    for (i=strlen(dst)-1;i>=0 && dst[i]!='.' && dst[i]!=':' && dst[i]!='/'
                              && dst[i]!='\\';i--);
    if (i>=0 && dst[i]=='.')
        return(&dst[i+1]);
    else
        return(&dst[strlen(dst)]);
    }


/*
** Replace any existing extension in "src" string with "ext".
** If no extension on "src" string, then append "ext".
*/
void wfile_newext(char *dst,char *src,char *ext)

    {
    int i;

    if (src!=NULL)
        strcpy(dst,src);
    for (i=strlen(dst)-1;i>=0 && dst[i]!='.' && dst[i]!=':' && dst[i]!='/'
                              && dst[i]!='\\';i--);
    if (dst[i]=='.')
        {
        if (ext==NULL || ext[0]=='\0')
            dst[i]='\0';
        else
            strcpy(&dst[i+1],ext[0]=='.' ? &ext[1]:ext);
        }
    else
        if (ext!=NULL && ext[0]!='\0')
            sprintf(&dst[strlen(dst)],".%s",ext[0]=='.' ? &ext[1] : ext);
    }


/*
** Strip extension from "src" and put into "dst".
** src can be NULL or ==dst.
*/
void wfile_stripext(char *dst,char *src)

    {
    int i;

    if (src!=NULL && dst!=src)
        strcpy(dst,src);
    for (i=strlen(dst)-1;i>=0 && dst[i]!='.' && dst[i]!=':' && dst[i]!='/'
                              && dst[i]!='\\';i--);
    if (dst[i]=='.')
        dst[i]='\0';
    }


char *wfile_getenv(char *envvar)

    {
#ifdef WIN32
    static char envvalue[1024];
    return(!GetEnvironmentVariable(envvar,envvalue,1023) ? NULL : envvalue);
#else
    return(getenv(envvar));
#endif
    }


int wfile_findfirst(const char *spec,wfile *wptr)

    {
#if (defined(WIN32))
    if (wsys_win32_api())
        {
        WIN32_FIND_DATA *fd;

        fd=(WIN32_FIND_DATA *)wptr->ds;
        wfile_basepath(wptr->path,spec);
        wptr->winhandle=(void *)FindFirstFile((char *)spec,fd);
        if ((HANDLE)wptr->winhandle==INVALID_HANDLE_VALUE)
            return(0);
        wfile_fullname(wptr->fullname,wptr->path,fd->cFileName);
        strcpy(wptr->basename,fd->cFileName);
        win_file_windate_to_tm(&wptr->date,&fd->ftLastWriteTime,wptr->fullname);
        wptr->size = (double)fd->nFileSizeLow;
        if (fd->nFileSizeHigh!=0)
            wptr->size += (double)fd->nFileSizeHigh * 4294967296.0;
        wptr->attr = 0;
        if (fd->dwFileAttributes&FILE_ATTRIBUTE_ARCHIVE)
            wptr->attr |= WFILE_ARCHIVE;
        if (fd->dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
            wptr->attr |= WFILE_DIR;
        if (fd->dwFileAttributes&FILE_ATTRIBUTE_SYSTEM)
            wptr->attr |= WFILE_SYSTEM;
        if (fd->dwFileAttributes&FILE_ATTRIBUTE_HIDDEN)
            wptr->attr |= WFILE_HIDDEN;
        if (fd->dwFileAttributes&FILE_ATTRIBUTE_READONLY)
            wptr->attr |= WFILE_READONLY;
        return(1);
        }
    else
#if (defined(UNIX))
        return(unix_findfirst(spec,wptr));
#elif (defined(MSDOS))
                return(msdos_findfirst(spec,wptr));
#else
                return(0);
#endif

#elif (defined(UNIX))
    return(unix_findfirst(spec,wptr));
#elif (defined(MSDOS))
    return(msdos_findfirst(spec,wptr));
#else
    return(0);
#endif
    }


int wfile_findnext(wfile *wptr)

    {
#if (defined(WIN32))
    if (wsys_win32_api())
        {
        WIN32_FIND_DATA *fd;

        if ((HANDLE)wptr->winhandle==INVALID_HANDLE_VALUE)
            return(0);
        fd=(WIN32_FIND_DATA *)wptr->ds;
        if (FindNextFile((HANDLE)wptr->winhandle,fd))
            {
            wfile_fullname(wptr->fullname,wptr->path,fd->cFileName);
            strcpy(wptr->basename,fd->cFileName);
            win_file_windate_to_tm(&wptr->date,&fd->ftLastWriteTime,wptr->fullname);
            wptr->size = (double)fd->nFileSizeLow;
            if (fd->nFileSizeHigh!=0)
                wptr->size += (double)fd->nFileSizeHigh * 4294967296.0;
            wptr->attr = 0;
            if (fd->dwFileAttributes&FILE_ATTRIBUTE_ARCHIVE)
                wptr->attr |= WFILE_ARCHIVE;
            if (fd->dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
                wptr->attr |= WFILE_DIR;
            if (fd->dwFileAttributes&FILE_ATTRIBUTE_SYSTEM)
                wptr->attr |= WFILE_SYSTEM;
            if (fd->dwFileAttributes&FILE_ATTRIBUTE_HIDDEN)
                wptr->attr |= WFILE_HIDDEN;
            if (fd->dwFileAttributes&FILE_ATTRIBUTE_READONLY)
                wptr->attr |= WFILE_READONLY;
            return(1);
            }
        wfile_findclose(wptr);
        return(0);
        }
    else
#if (defined(UNIX))
        return(unix_findnext(wptr));
#elif (defined(MSDOS))
                return(msdos_findnext(wptr));
#else
                return(0);
#endif

#elif (defined(UNIX))
    return(unix_findnext(wptr));
#elif (defined(MSDOS))
    return(msdos_findnext(wptr));
#else
    return(0);
#endif
    }


void wfile_findclose(wfile *wptr)

    {
#if (defined(WIN32))
    if (wsys_win32_api())
        {
        if ((HANDLE)wptr->winhandle==INVALID_HANDLE_VALUE)
            return;
        FindClose((HANDLE)wptr->winhandle);
        wptr->winhandle = (void *)INVALID_HANDLE_VALUE;
        }
    else
#if (defined(UNIX))
        unix_close(wptr);
#else
                return;
#endif

#elif (defined(UNIX))
    unix_close(wptr);
#endif
    }

/*
** This function returns date of last file modification
** Return 0 if can't get date, 1 if can, -1 if not supported
*/
int wfile_date(const char *filename,struct tm *filedate)

    {
#if (defined(WIN32))
    HANDLE  handle;
//    OFSTRUCT    buf;
    FILETIME    ctime,mtime,atime;

    if (wsys_win32_api())
        {
        char fn2[MAXFILENAMELEN];

        /* Weird bug:  If I don't use the full path, then sometimes */
        /* this returns an incorrect result.                        */
        /* Started using CreateFile instead of OpenFile because OpenFile */
        /* can't handle long paths.  4-16-09                             */
        strcpy(fn2,filename);
        handle=(HANDLE)CreateFile(fn2,GENERIC_READ,
                                 FILE_SHARE_DELETE
                                   | FILE_SHARE_READ
                                   | FILE_SHARE_WRITE,
                                 NULL,
                                 OPEN_EXISTING,
                                 FILE_ATTRIBUTE_NORMAL,
                                 NULL);
//printf("handle=%d, HFILE_ERROR=%d\n",(int)handle,(int)INVALID_HANDLE_VALUE);
//printf("error=%s\n",win_lasterror());
        if ((HANDLE)handle==INVALID_HANDLE_VALUE)
            return(0);
        if (!GetFileTime(handle,&ctime,&atime,&mtime))
            {
            CloseHandle(handle);
            return(0);
            }
        CloseHandle(handle);
        win_file_windate_to_tm(filedate,&mtime,(char *)fn2);
        return(1);
        }
    else
#if (defined(UNIX))
        return(unix_date(filename,filedate));
#elif (defined(MSDOS))
                return(msdos_date(filename,filedate));
#else
                return(-1);
#endif

#elif (defined(UNIX))
    return(unix_date(filename,filedate));
#elif (defined(MSDOS))
    return(msdos_date(filename,filedate));
#else
    return(-1);
#endif
    }


/*
** Add/subtract seconds from date
*/
void wfile_date_add_seconds(struct tm *date,int secs)

    {
    int min;

    while (secs>=3600)
        {
        wfile_date_increment_hour(date);
        secs-=3600;
        }
    while (secs<=-3600)
        {
        wfile_date_decrement_hour(date);
        secs+=3600;
        }
    if (secs>0)
        {
        min=secs/60;
        secs -= (min*60);
        date->tm_sec += secs;
        if (date->tm_sec > 59)
            {
            date->tm_sec -= 60;
            min++;
            }
        date->tm_min += min;
        if (date->tm_min > 59)
            {
            date->tm_min -= 60;
            wfile_date_increment_hour(date);
            }
        }
    else
        {
        secs = -secs;
        min=secs/60;
        secs -= (min*60);
        date->tm_sec -= secs;
        if (date->tm_sec < 0)
            {
            date->tm_sec += 60;
            min++;
            }
        date->tm_min -= min;
        if (date->tm_min < 0)
            {
            date->tm_min += 60;
            wfile_date_decrement_hour(date);
            }
        }
    }


void wfile_date_add_hours(struct tm *date,int nhours)

    {
    int i;
    if (nhours>0)
        for (i=0;i<nhours;i++)
            wfile_date_increment_hour(date);
    else
        for (i=0;i<-nhours;i++)
            wfile_date_decrement_hour(date);
    }


void wfile_date_increment_hour(struct tm *date)

    {
    static int dom[12]={31,28,31,30,31,30,31,31,30,31,30,31};
    date->tm_hour++;
    if (date->tm_hour>23)
        {
        date->tm_hour=0;
        date->tm_mday++;
        date->tm_wday = (date->tm_wday+1)%7;
        date->tm_yday = (date->tm_yday+1)%365;
        if (date->tm_mday > dom[date->tm_mon])
            {
            date->tm_mday=1;
            date->tm_mon++;
            if (date->tm_mon>11)
                {
                date->tm_mon=0;
                date->tm_year++;
                }
            }
        }
    }


void wfile_date_decrement_hour(struct tm *date)

    {
    static int dom[12]={31,28,31,30,31,30,31,31,30,31,30,31};
    date->tm_hour--;
    if (date->tm_hour<0)
        {
        date->tm_hour=23;
        date->tm_mday--;
        date->tm_wday = (date->tm_wday+6)%7;
        date->tm_yday = (date->tm_yday+364)%365;
        if (date->tm_mday < 1)
            {
            date->tm_mday=dom[(date->tm_mon+11)%12];
            date->tm_mon--;
            if (date->tm_mon<0)
                {
                date->tm_mon=11;
                date->tm_year--;
                }
            }
        }
    }
   

/*
** Uses "touch" command in Unix/Linux.
*/
void wfile_set_mod_date(char *filename,struct tm *date)

    {
#ifdef WIN32
    win_set_mod_filetime(filename,date);
#else
    char cmdbuf[MAXFILENAMELEN];
    int i,has_a_space;

    has_a_space=0;
    for (i=0;filename[i]!='0';i++)
        if (filename[i]==' ')
            {
            has_a_space=1;
            break;
            }
    if (has_a_space)
        sprintf(cmdbuf,"touch -t %02d%02d%02d%02d%02d.%02d \"%s\"",
                date->tm_year%100,date->tm_mon+1,date->tm_mday,
                date->tm_hour,date->tm_min,date->tm_sec,filename);
    else
        sprintf(cmdbuf,"touch -t %02d%02d%02d%02d%02d.%02d %s",
                date->tm_year%100,date->tm_mon+1,date->tm_mday,
                date->tm_hour,date->tm_min,date->tm_sec,filename);
    system(cmdbuf);
#endif
    }


int wfile_is_symlink_ex(char *filename,char *src)

    {
#if (defined(UNIX))
    return(unix_is_symlink(filename,src));
#else
    return(0);
#endif
    }


int wfile_is_symlink(char *filename)

    {
    return(wfile_is_symlink_ex(filename,NULL));
    }


int wfile_is_regular_file(char *filename)

    {
#if (defined(WIN32))
    return(1);
#elif (defined(UNIX))
    return(unix_is_regular_file(filename));
#else
    return(1);
#endif
    }


int wfile_saved_during_daylight_savings(char *filename)

    {
    struct tm  filedate;

    wfile_date(filename,&filedate);
    if (filedate.tm_isdst < 0)
        {
        time_t t;
        struct tm *lt;
        t=mktime(&filedate);
        lt=localtime(&t);
        filedate.tm_isdst = lt->tm_isdst;
        }
    return(filedate.tm_isdst);
    }


void wfile_increment_hour(struct tm *ts)

    {
    /* Increment hour */
    ts->tm_hour++;
    if (ts->tm_hour<=23)
        return;

    /* Increment day */
    ts->tm_hour=0;
    wfile_increment_day(ts);
    }


void wfile_increment_day(struct tm *ts)

    {
    int dim;

    ts->tm_mday++;
    if (ts->tm_yday>=0)
        ts->tm_yday++;
    if (ts->tm_wday>=0)
        ts->tm_wday = (ts->tm_wday+1)%7;
    dim=wfile_days_in_month(ts);
    if (ts->tm_mday<=dim)
        return;

    /* Increment month */
    ts->tm_mday=1;
    ts->tm_mon++;
    if (ts->tm_mon<=11)
        return;

    /* Increment year */
    ts->tm_mon=0;
    ts->tm_year++;
    ts->tm_yday=0;
    }


void wfile_decrement_hour(struct tm *ts)

    {
    /* Decrement hour */
    ts->tm_hour--;
    if (ts->tm_hour>=0)
        return;

    /* Decrement day */
    ts->tm_hour=23;
    wfile_decrement_day(ts);
    }


void wfile_decrement_day(struct tm *ts)

    {
    ts->tm_mday--;
    if (ts->tm_yday>=0)
        ts->tm_yday--;
    if (ts->tm_wday>=0)
        ts->tm_wday = (ts->tm_wday+6)%7;
    if (ts->tm_mday>=1)
        return;
    /* Decrement month */
    if (ts->tm_mon<=0)
        {
        /* Decrement year */
        ts->tm_year--;
        ts->tm_mon=11;
        ts->tm_yday=wfile_days_in_year(ts->tm_year+1900);
        }
    else
        ts->tm_mon--;
    ts->tm_mday=wfile_days_in_month(ts);
    }    


int wfile_days_in_month(struct tm *ts)

    {
    static int dim[12]={31,28,31,30,31,30,31,31,30,31,30,31};

    if (ts->tm_mon!=1)
        return(dim[ts->tm_mon]);
    if (wfile_leap_year(ts->tm_year+1900))
        return(29);
    return(28);
    }

/*
** Only requires tm_year, tm_mon, and tm_mday to be filled in
*/
int wfile_days_since_jan_1_1900(struct tm *date)

    {
    int i,days,year;
    static int j1days[200]={365,730,1095,1460,1826,2191,2556,2921,3287,3652,4017,4382,4748,
        5113,5478,5843,6209,6574,6939,7304,7670,8035,8400,8765,9131,9496,9861,10226,10592,
        10957,11322,11687,12053,12418,12783,13148,13514,13879,14244,14609,14975,15340,15705,
        16070,16436,16801,17166,17531,17897,18262,18627,18992,19358,19723,20088,20453,20819,
        21184,21549,21914,22280,22645,23010,23375,23741,24106,24471,24836,25202,25567,25932,
        26297,26663,27028,27393,27758,28124,28489,28854,29219,29585,29950,30315,30680,31046,
        31411,31776,32141,32507,32872,33237,33602,33968,34333,34698,35063,35429,35794,36159,
        36524,36890,37255,37620,37985,38351,38716,39081,39446,39812,40177,40542,40907,41273,
        41638,42003,42368,42734,43099,43464,43829,44195,44560,44925,45290,45656,46021,46386,
        46751,47117,47482,47847,48212,48578,48943,49308,49673,50039,50404,50769,51134,51500,
        51865,52230,52595,52961,53326,53691,54056,54422,54787,55152,55517,55883,56248,56613,
        56978,57344,57709,58074,58439,58805,59170,59535,59900,60266,60631,60996,61361,61727,
        62092,62457,62822,63188,63553,63918,64283,64649,65014,65379,65744,66110,66475,66840,
        67205,67571,67936,68301,68666,69032,69397,69762,70127,70493,70858,71223,71588,71954,
        72319,72684,73049};
    static int dim[12]={31,28,31,30,31,30,31,31,30,31,30,31};

    year=date->tm_year+1900;
    if (year<1900)
        return(0);
    if (year>1900)
        {
        days=j1days[year-1901>199?199:year-1901];
        for (i=2100;i<year;i++)
            days += wfile_days_in_year(i);
        }
    else
        days=0;
    for (i=0;i<date->tm_mon;i++)
        days += dim[i];
    if (date->tm_mon>1 && wfile_leap_year(year))
        days++;
    days += date->tm_mday-1;
    return(days);
    }


int wfile_days_in_year(int year)

    {
    return(wfile_leap_year(year) ? 366 : 365);
    }


int wfile_leap_year(int year)

    {
    return((year%400==0) || ((year%100!=0) && (year%4==0)));
    }

/*
** Set working directory.
** Returns 0 for success, otherwise error code.
*/
int wfile_set_wd(char *path)

    {
    int status;

#ifdef WIN32
#if (defined(DJEMX))
    _chdir2(path);
#endif
    status = !SetCurrentDirectory(path);
#else
    status = chdir(path);
#endif
    return(status);
    }


/*
** Get working directory
*/
char *wfile_get_wd(void)

    {
    static char dir[MAXFILENAMELEN];

#ifdef WIN32
    GetCurrentDirectory(255,dir);
#else
    getcwd(dir,255);
#endif
    return(dir);
    }


void wfile_volumeinfo(char *drive,char *volume,char *sn,char *filesys,
                      int *maxnamelen)

    {
    char fs[100];
    char volname[200];
#ifdef WIN32
    DWORD  serno,maxlen,flags;
    char vol[100];
    vol[0]=drive[0];
    vol[1]=':';
    vol[2]='\\';
    vol[3]='\0';
    if (!GetVolumeInformation(vol,volname,199,&serno,&maxlen,&flags,fs,99))
#else
    int  serno,maxlen;
#endif
        {
        strcpy(volname,"unknown");
        strcpy(fs,"unknown");
        serno=maxlen=-1;
        }
    if (volume!=NULL)
        strcpy(volume,volname);
    if (sn!=NULL)
        sprintf(sn,"%d",(int)serno);
    if (maxnamelen!=NULL)
        (*maxnamelen) = maxlen;
    if (filesys!=NULL)
        strcpy(filesys,fs);
    }


/*
** If Unix, volume should be a file name, and this will be passed
** to the "df" command.
*/
double wfile_freespace(char *volume,double *totalspace)

    {
#ifdef WIN32
    long    spc,bps,fc,tc;
    char    vol[100];
    vol[0]=volume[0];
    vol[1]=':';
    vol[2]='\\';
    vol[3]='\0';
    GetDiskFreeSpace(vol,(void *)&spc,(void *)&bps,(void *)&fc,(void *)&tc);
    /* fc = free clusters, spc = sectors/cluster, bps = bytes/sector */
    if (totalspace!=NULL)
        (*totalspace) = (double)tc*(double)spc*bps;
    return((double)fc*(double)spc*bps);
#else
    static char tempname[MAXFILENAMELEN];
    static char cmd[MAXFILENAMELEN];
    double v[4],freebytes,totbytes;
    int     na;
    FILE *f;

    strcpy(tempname,wfile_tempname("",""));
#if (defined(hpux) || defined(__hpux))
    sprintf(cmd,"bdf %s > %s",volume,tempname);
#else
    sprintf(cmd,"df -k %s > %s",volume,tempname);
#endif
    system(cmd);
    f=fopen(tempname,"r");
    freebytes = totbytes = -1;
    if (f!=NULL)
        {
        while (fgets(cmd,120,f)!=NULL)
            {
            clean_line(cmd);
            na=string_read_doubles(cmd,v,3);
            if (na<=0)
                na=sscanf(cmd,"%*s %lf %lf %lf",&v[0],&v[1],&v[2]);
            if (na<3)
                continue;
            /*
            if (na<=0)
                {
                if ((na=in_string(cmd,":"))>0)
                    totbytes = atof(&cmd[na+1])*1024.;
                continue;
                }
            if (na<3)
                {
                freebytes = v[0]*1024.;
                break;
                }
            */
            totbytes = v[0]*1024.;
            freebytes = v[2]*1024.;
            break;
            }
        fclose(f);
        }
    remove(tempname);
    if (totalspace!=NULL)
        (*totalspace) = totbytes;
    return(freebytes);
#endif
    }


/*
** Makes all necessary directories so that the filename can be saved.
*/
int wfile_prepdir(char *filename)

    {
    char path[MAXFILENAMELEN];
    int     i,status;

    for (i=0;filename[i]!='\0';i++)
        {
        if (filename[i]==SLASH && i>0 && filename[i-1]!=':')
            {
            path[i]='\0';
            status=wfile_status(path);
            if (status==1)
                return(-1);
            if (status!=2)
                {
                status=wfile_makedir(path);
                if (status==-2)
                    return(-2);
                }
            }
        path[i]=filename[i];
        }
    return(0);
    }


/*
** Make directory.
** Returns -1 if already exists.
**         -2 if error
**          0 if OK
*/
int wfile_makedir(char *path)

    {
    int status;

    status=wfile_status(path);
    if (status!=0)
        return(-1);
#ifdef WIN32
    status=!CreateDirectory(path,NULL);
#else
    status=mkdir(path,0755); /* User RWE, Group RE, World RE */
#endif
    if (status)
        return(-2);
    return(status);
    }


static FILE *rmdirlog;


int wfile_dir_is_empty(char *dir)

    {
    char wildspec[MAXFILENAMELEN];
    wfile wf;
    int c,status;

    if (wfile_status(dir)!=2)
        return(-1);
    wfile_fullname(wildspec,dir,"*");
    for (c=0,status=wfile_findfirst(wildspec,&wf);status;status=wfile_findnext(&wf))
        {
        if (!strcmp(wf.basename,".") || !strcmp(wf.basename,".."))
            continue;
        c++;
        if (c>0)
            break;
        }
    wfile_findclose(&wf);
    return(c==0 ? 1 : 0);
    }


int wfile_remove_dir(char *path,int recursive)

    {
    int status;
    if (recursive==2)
        {
        time_t now;

        time(&now);
        rmdirlog=fopen("/rmdir.log","a");
        nprintf(rmdirlog,"\nwfile_remove_dir('%s',%d) called at %s",
                path,recursive,ctime(&now));
        }
    status=wfile_remove_dir_1(path,recursive);
    if (recursive==2)
        {
        time_t now;

        time(&now);
        nprintf(rmdirlog,"wfile_remove_dir('%s',%d) returns %d at %s\n",
                path,recursive,status,ctime(&now));
        if (rmdirlog!=NULL)
            {
            fclose(rmdirlog);
            rmdirlog=NULL;
            }
        }
    return(status);
    }
        
    
/*
** Remove directory.  Returns -1 if error, 0 of OK.
** If recursive, removes directory and ALL contents.
*/
static int wfile_remove_dir_1(char *path,int recursive)

    {
/*
#ifdef WIN32
    char spec[384];
    if (recursive==0)
        return(wfile_strong_rmdir(path));
*/
    /* If WIN32, use command line rmdir function if recursive */
/*
    sprintf(spec,"cmd /c rmdir %s/q %s",recursive ? "/s ":"",path);
    system(spec);
    return(0);
#else
*/
    wfile_remove_dir_file_by_file(path,recursive);
    return(0);
/*
#endif
*/
    }


/*
** Return true if file is a 64-bit x86-64 exe
*/
int wfile_check_file_64bit(char *filename)

    {
#if (defined(WIN32) || defined(MSDOS))
    return(0);
#else
    char fullname[MAXFILENAMELEN];
    char tmpfile[MAXFILENAMELEN];
    char cmd[MAXFILENAMELEN];
    FILE *f;
    
    wfile_abstmpnam(tmpfile);
    sprintf(cmd,"which %s > %s",filename,tmpfile);
    system(cmd);
    f=fopen(tmpfile,"r");
    if (f==NULL)
        return(0);
    if (fgets(fullname,250,f)==NULL)
        {
        fclose(f);
        return(0);
        }
    fclose(f);
    remove(tmpfile);
    clean_line(fullname);
    sprintf(cmd,"file %s > %s",fullname,tmpfile);
    system(cmd);
    f=fopen(tmpfile,"r");
    if (f==NULL)
        return(0);
    if (fgets(cmd,250,f)==NULL)
        {
        fclose(f);
        return(0);
        }
    fclose(f);
    remove(tmpfile);
    if (in_string(cmd,"64-bit")>=0)
        return(-1);
    return(0);
#endif
    }


int wfile_remove_dir_file_by_file(char *path,int recursive)

    {
    wfile wf;
    int s,err;
    char spec[384];

    /* If it's not a dir, return error */
    if (wfile_status(path)!=2)
        return(-1);
    if (!recursive)
        return(wfile_strong_rmdir(path));
    err=0;
    wfile_fullname(spec,path,"*");
    for (s=wfile_findfirst(spec,&wf);s;s=wfile_findnext(&wf))
        {
        if (!strcmp(wf.basename,".") || !strcmp(wf.basename,".."))
            continue;
        /* Exclude symbolic links from recursion */
        if ((wf.attr & WFILE_SYMLINK) || wfile_status(wf.fullname)!=2)
            {
            if (recursive==2)
                nprintf(rmdirlog,"rm '%s'\n",wf.fullname);
            else
                {
                if (wfile_strong_remove(wf.fullname))
                    err--;
                }
            }
        else
            err -= wfile_remove_dir_1(wf.fullname,recursive);
        }
    wfile_findclose(&wf);
    if (recursive==2)
        nprintf(rmdirlog,"rmdir '%s'\n",path);
    else
        {
        if (wfile_strong_rmdir(path))
            err--;
        }
    return(err);
    }


int wfile_strong_remove(char *filename)

    {
    int     status;

    /* First try removing the file */
    status=remove(filename);
#ifdef WIN32
#define PROBLEM_ATTRIBUTE (FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_READONLY|FILE_ATTRIBUTE_SYSTEM)
    /* No more Mr. Nice Guy */
    if (status)
        {
        int attr;
        status=win_grant_full_file_access(filename);
/*
printf("win_grant_full_file_access(%s)=%d\n",filename,status);
        if (status)
            printf("    err=%d\n",win_lasterror());
*/
        attr=GetFileAttributes(filename);
// printf("attr(%s) = %X (PA=%X)\n",filename,attr,PROBLEM_ATTRIBUTE);
        if (attr!=0xffffffff && (attr&PROBLEM_ATTRIBUTE))
            {
            /* int s; */
            /* s= */ SetFileAttributes(filename,(attr | FILE_ATTRIBUTE_NORMAL) & (~PROBLEM_ATTRIBUTE));
/*
            printf("setattr returns %d.\n",s);
            if (s==0)
                printf("err = %s\n",win_lasterror());
*/
            }
        status=remove(filename);
        }
#endif
    return(status);
    }


int wfile_strong_rmdir(char *dirname)

    {
    int     status;

    status=rmdir(dirname);
#ifndef WIN32
    return(status);
#else
    if (status)
        {
        SetFileAttributes(dirname,FILE_ATTRIBUTE_DIRECTORY);
        status=rmdir(dirname);
        }
    return(status);
#endif
    }


/*
** Return -1 if the specified filename or path is absolute and
** not relative.  E.g. c:\mydir\myfile.dat is absolute, but
** mydir\myfile.dat is relative.
*/
int wfile_absolute_path(char *path)

    {
    int     len;
    len=strlen(path);
    if (len<1)
        return(0);
    if (wfile_eitherslash(path[0]))
        return(-1);
#ifdef WIN32
    if (len>1 && path[1]==':')
        return(-1);
#endif
    return(0);
    }


/*
** Make a path absolute by prepending the current working directory.
** (only if the path is not already absolute).
*/
void wfile_make_absolute(char *path)

    {
    char    newpath[MAXFILENAMELEN];

    wfile_reslash(path);
    wfile_remove_dots(path);
    if (wfile_absolute_path(path))
        return;
    strcpy(newpath,wfile_get_wd());
    wfile_fullname(newpath,newpath,path);
    wfile_remove_dots(newpath);
    wfile_noslash(path,newpath);
    }


void wfile_written_info(char *filename,FILE *out)

    {
    double size;
    char buf[64];

    size=wfile_size(filename);
    if (size<0)
        nprintf(out,"File %s not written!\n",filename);
    else
        {
        comma_dprint(buf,size);
        nprintf(out,"%s bytes written to file %s.\n",buf,filename);
        }
    }

    
/*
** If filename1 doesn't exist but filename2 does, returns 0.
** If filename1 exists but filename2 doesn't, returns 1.
** If both files exist and filename1 is newer than filename2, returns 2.
** Otherwise returns 0.
*/
int wfile_newer(char *filename1,char *filename2)

    {
    struct tm d1,d2;

    if (wfile_date(filename1,&d1)!=1)
        return(0);
    if (wfile_date(filename2,&d2)!=1)
        return(1);
    return(wfile_datecomp(&d1,&d2)>0 ? 2 : 0);
    }


/*
** Returns -1 if date d1 is older than date d2
**          0 if they are equal
**          1 if d1 is more recent than d2
*/
int wfile_datecomp(struct tm *d1,struct tm *d2)

    {
    COMPARE(tm_year);
    COMPARE(tm_mon);
    COMPARE(tm_mday);
    COMPARE(tm_hour);
    COMPARE(tm_min);
    COMPARE(tm_sec);
    return(0);
    }


int wfile_filename_compare(char *fn1,char *fn2)

    {
#if (defined(WIN32))
    return(stricmp(fn1,fn2));
#elif (defined(UNIX))
    return(strcmp(fn1,fn2));
#else
    return(stricmp(fn1,fn2));
#endif
    }


int wfile_filename_basename_compare(char *fn1,char *fn2)

    {
    char bn1[MAXFILENAMELEN],bn2[MAXFILENAMELEN];
    int c;

    wfile_basespec(bn1,fn1);
    wfile_basespec(bn2,fn2);
#if (defined(WIN32))
    c=stricmp(bn1,bn2);
    if (!c)
        return(stricmp(fn1,fn2));
    return(c);
#elif (defined(UNIX))
    c=strcmp(bn1,bn2);
    if (!c)
        return(strcmp(fn1,fn2));
    return(c);
#else
    c=stricmp(bn1,bn2);
    if (!c)
        return(stricmp(fn1,fn2));
    return(c);
#endif
    }


/*
** Returns 0 for no file, 1 for a reg. file, 2 for a directory
*/
int wfile_status(char *filename)

    {
#if (defined(WIN32))
    long    status;

    if (wsys_win32_api())
        {
        status=GetFileAttributes(filename);
        if (status==-1)
            return(0);
        if (status&FILE_ATTRIBUTE_DIRECTORY)
            return(2);
        return(1);
        }
    else
#if (defined(UNIX))
        return(unix_status(filename));
#elif (defined(MSDOS))
                return(msdos_status(filename));
#else
                return(-1);
#endif

#elif (defined(UNIX))
    return(unix_status(filename));
#elif (defined(MSDOS))
    return(msdos_status(filename));
#else
    return(-1);
#endif
    }

/*
** Choose <basename> so that it represents the full path in <fullname>
** but it is relative to <dir>.
** <fullname> and <dir> are assumed to be relative to the root and
** may be slashed either way.  If either is preceeded by a drive
** letter (c:), that is ignored.
** Comparisons are not case sensitive, even in Unix (for now).
*/
void wfile_relative_basename(char *basename,char *fullname,char *dir)

    {
    int     id,i2,i,si;

    basename[0]='\0';
    for (id=0;dir[id]!='\0' && dir[id]!=':';id++);
    if (dir[id]=='\0')
        id=0;
    for (;wfile_eitherslash(dir[id]);id++);
    for (i2=0;fullname[i2]!='\0' && fullname[i2]!=':';i2++);
    if (fullname[i2]=='\0')
        i2=0;
    if (dir[id]=='\0')
        {
        strcpy(basename,&fullname[i2]);
        return;
        }
    for (;wfile_eitherslash(fullname[i2]);i2++);
    for (si=i2,i=0;1;i++)
        {
        if (dir[id+i]=='\0')
            {
            if (wfile_eitherslash(fullname[i2+i]))
                {
                strcpy(basename,&fullname[i2+i+1]);
                return;
                }
            sprintf(basename,"..%c%s",SLASH,&fullname[i2+si]);
            return;
            }
        if (wfile_eitherslash(dir[id+i]) && wfile_eitherslash(fullname[i2+i]))
            {
            si=i+1;
            if (dir[id+i+1]=='\0')
                {
                strcpy(basename,&fullname[i2+i+1]);
                return;
                }
            continue;
            }
        if (tolower(dir[id+i])==tolower(fullname[i2+i]))
            continue;
        break;
        }
    strcpy(basename,"../");
    for (;dir[id+i]!='\0';i++)
        if (wfile_eitherslash(dir[id+i]) && dir[id+i+1]!='\0')
            strcat(basename,"../");
    if (basename[0]=='\0')
        strcat(basename,"../");
    strcat(basename,&fullname[i2+si]);
    wfile_reslash(basename);
    }


/*
** Takes a name like c:\mypath\mydir\mysubdir\myfile.dat and changes
** it to c:\mypath\mydir\myfile.dat.  (removes the lowest subdir.)
*/
void wfile_up_one(char *filename)

    {
    int i,j,len;

    len=strlen(filename);
    for (i=len-1;i>=0 && filename[i]!=':' && !wfile_eitherslash(filename[i]);i--);
    if (i<0 || filename[i]==':')
        return;
    for (j=i-1;j>=0 && filename[j]!=':' && !wfile_eitherslash(filename[j]);j--);
    memmove(&filename[j+1],&filename[i+1],len-i);
    }
    

/*
** Remove ".." and "." from a path where possible.
** Uses wfile_eitherslash() to determine slashes.
*/
void wfile_remove_dots(char *s)

    {
    int     i,j,ppos,nondot;

    if (s[0]=='.' && s[1]=='\0')
        return;
    ppos=-1;
    nondot=0;
    for (i=0;s[i]!='\0';i++)
        {
        /* Remove ../ */
        if (ppos>=0 && nondot
                    && wfile_eitherslash(s[i]) && s[i+1]=='.' && s[i+2]=='.'
                    && (wfile_eitherslash(s[i+3]) || s[i+3]=='\0'))
            {
            for (j=s[i+3]=='\0' ? i+3 : i+4;s[j]!='\0';j++)
                s[ppos++]=s[j];
            s[ppos]='\0';
            ppos=-1;
            nondot=0;
            i=-1;
            continue;
            }
        /* Remove ./ */
        if (ppos<0 && s[i]=='.'
                   && (wfile_eitherslash(s[i+1]) || s[i+1]=='\0'))
            {
            for (ppos=i,j=(s[i+1]=='\0') ? i+1 : i+2;s[j]!='\0';j++)
                s[ppos++]=s[j];
            s[ppos]='\0';
            ppos=-1;
            nondot=0;
            i=-1;
            continue;
            }
        if (wfile_eitherslash(s[i]))
            {
            ppos=-1;
            nondot=0;
            }
        else
            {
            if (ppos<0)
                ppos=i;
            if (s[i]!='.')
                nondot=-1;
            }
        }
    }



void wfile_fullname(char *fullname,char *path,char *spec)

    {
    wfile_goodpath(fullname,path);
    strcat(fullname,spec);
    }


static char ziptempdir[256];
void wfile_fullname_zipex(char *fullname,char *path,char *spec)

    {
    char cwd[256];
    char cmd[384];

    ziptempdir[0]='\0';
    if (!wfile_is_zipfile(path))
        {
        wfile_fullname(fullname,path,spec);
        return;
        }
    wfile_abstmpnam(ziptempdir);
    wfile_makedir(ziptempdir);
    strcpy(cwd,wfile_get_wd());
    wfile_set_wd(ziptempdir);
#ifdef WIN32
    sprintf(cmd,"unzip -C -o \"%s\" \"%s\" 1> nul 2> nul",path,spec);
#else
    sprintf(cmd,"unzip -o \"%s\" \"%s\" > /dev/null",path,spec);
#endif
    system(cmd);
    wfile_set_wd(cwd);
    wfile_fullname(fullname,ziptempdir,spec);
    }


char *wfile_nullfile(void)

    {
    static char nullfile[32];

#ifdef WIN32
    strcpy(nullfile,"nul");
#else
    strcpy(nullfile,"/dev/null");
#endif
    return(nullfile);
    }


void wfile_zipex_cleanup(void)

    {
    if (ziptempdir[0]!='\0')
        wfile_remove_dir(ziptempdir,1);
    ziptempdir[0]='\0';
    }


void wfile_basepath(char *dst,const char *src)

    {
    int     i;

    if (dst!=src && src!=NULL)
        strcpy(dst,src);
    for (i=strlen(dst)-1;i>=0 && !wfile_eitherslash(dst[i]);i--);
    dst[i+1]='\0';
    }


void wfile_basespec(char *dst,char *src)

    {
    char   *s;
    int     i,j;

    s= (src==NULL) ? dst : src;
    for (i=strlen(s)-1;i>=0 && !wfile_eitherslash(s[i]);i--);
    for (i++,j=0;s[i]!='\0';i++,j++)
        dst[j]=s[i];
    dst[j]='\0';
    if (dst[0]=='\0')
        strcpy(dst,ALLFILES);
    }


void wfile_addwild(char *dst,char *src)

    {
    wfile_goodpath(dst,src);
    strcat(dst,ALLFILES);
    }


void wfile_addslash(char *dst)

    {
    int     i;
    i=strlen(dst);
    dst[i]=SLASH;
    dst[i+1]='\0';
    }


void wfile_goodpath(char *dst,char *src)

    {
    if (dst!=src && src!=NULL)
        strcpy(dst,src);
    if (dst[0]=='\0')
        return;
    if (!wfile_eitherslash(dst[strlen(dst)-1]))
        wfile_addslash(dst);
    }


/* 
** Expand file name using CWD.  Check to see that the filename
** isn't already absolute.
*/
void wfile_expandname(char *expanded,char *filename)

    {
    static char basename[MAXFILENAMELEN];
    static char cwd[MAXFILENAMELEN];

    strcpy(cwd,wfile_get_wd());
    strcpy(basename,filename);
    if (expanded==NULL)
        expanded=filename;
    if (basename[0]=='/' || basename[0]=='\\'
            || (strlen(basename)>3 && basename[1]==':' && 
                    (basename[2]=='\\' || basename[2]=='/')))
        {
        if (expanded!=filename)
            strcpy(expanded,filename);
        return;
        }
    wfile_fullname(expanded,cwd,basename);
    }


/*
** If slash (either type) at end of name, remove it UNLESS:
**
**     1. The name is only a slash (root dir).
**        E.g. src = "/" or src = "\\"
**
**     2. The name is only a drive letter + a slash
**        E.g. src = "d:\\"
**
*/
void wfile_noslash(char *dst,char *src)

    {
    int     len;

    if (src!=NULL && dst!=src)
        strcpy(dst,src);
    len=strlen(dst);
    if (len<=0)
        return;
    /* root dir?  Don't remove slash.  */
    if (wfile_eitherslash(dst[0]) && dst[1]=='\0')
        return;
    /* drive letter + root dir?  Don't remove slash.  */
    if (dst[1]==':' && wfile_eitherslash(dst[2]) && dst[3]=='\0')
        return;
    if (wfile_eitherslash(dst[len-1]))
        dst[len-1]='\0';
    }


void wfile_unique_part(char *filename,char *path)

    {
    int     i,j;

    for (i=0;filename[i]!='\0' && path[i]!='\0' && path[i]==filename[i];i++);
    if (wfile_eitherslash(filename[i]))
        i++;
    for (j=0;filename[i]!='\0';i++,j++)
        filename[j]=filename[i];
    filename[j]='\0';
    }


/*
** NOTE!  In Linux, the backslash character (\) is a valid file name
** character, so this complicates the re-slashing.  You should use
** this function with care!
*/
void wfile_reslash(char *filename)

    {
    wfile_slash_this_way(filename,SLASH);
    }


/*
** NOTE!  In Linux, the backslash character (\) is a valid file name
** character, so this complicates the re-slashing.  You should use
** this function with care!
*/
void wfile_slash_this_way(char *filename,int slash)

    {
    int     i,alternate;

    alternate = (slash=='/') ? '\\' : '/';
#ifndef WIN32
    if (slash=='/')
        {
        int c;

        for (i=c=0;filename[i]!='\0';i++)
            if (filename[i]==slash)
                c++;
        if (c>0)  /* If already has / chars, don't re-slash in Linux */
            return;
        }
#endif
    for (i=0;filename[i]!='\0';i++)
        if (filename[i]==alternate)
            filename[i]=slash;
    }


/*
** Path ends in a slash.
** Use NULL if don't want it assigned.
*/
char *wfile_temppath(char *path)

    {
    static char tpath[MAXFILENAMELEN];

#if (defined(WIN32))
    if (wsys_win32_api())
        GetTempPath(255,tpath);
    else
#else
#ifdef __WATCOMC__
extern char P_tmpdir[];
#endif
    strcpy(tpath,P_tmpdir);
#endif
    wfile_reslash(tpath);
    if (path!=NULL)
        strcpy(path,tpath);
    return(tpath);
    }


/*
** If dir==NULL && prefix==NULL, should still give valid temp name.
** If dir == NULL, the temporary dir is used.
** dir must end in a slash.
** For current dir, use "" or "./"
**
** CAUTION:  In WIN32, ONLY dir==NULL works, or dir=="" and pre=="".
*/
char *wfile_tempname(char *dir,char *prefix)

    {
    static char tname[MAXFILENAMELEN];
    char  myprefix[MAXFILENAMELEN];
#ifdef WIN32
    int status;
#endif
#ifndef LINUX
    char  mydir[MAXFILENAMELEN];
#endif

    if (prefix==NULL || prefix[0]=='\0')
        strcpy(myprefix,"tmp");
    else
        strcpy(myprefix,prefix);
#ifdef WIN32
    if (dir==NULL)
        {
        char *p;
        p=getenv("TEMP");
        if (p!=NULL)
            strcpy(mydir,p);
        else
            {
            p=getenv("TMP");
            if (p!=NULL)
                strcpy(mydir,p);
            else
                strcpy(mydir,".");
            }
        }
    else if (dir[0]=='\0')
        strcpy(mydir,".");
    else
        strcpy(mydir,dir);
    /* Make sure full path exists */
    wfile_fullname(tname,mydir,myprefix);
    wfile_prepdir(tname);
    /* Get temp name */
    status=GetTempFileName(mydir,myprefix,0,tname);
    if (status)
        {
        /* This actually creates the file, so we have to delete it */
        /* in case the user wishes to use it for a directory.      */
        if (wfile_status(tname)==1)
            remove(tname);
        return(tname);
        }
#endif /* WIN32 */
#ifdef LINUX
    strcat(myprefix,"XXXXXX");
    if (dir==NULL)
        sprintf(tname,"/tmp/%s",myprefix);
    else
        wfile_fullname(tname,dir,myprefix);
    /* File is created, so remove it. */
    {
    int fd;
    fd=mkstemp(tname);
    if (fd!=-1)
        {
        close(fd);
        remove(tname);
        }
    }
    return(tname);

#else /* Unix and Win32 failsafe */
    if (dir!=NULL
         && (dir[0]=='\0' || (dir[0]=='.' && dir[1]==SLASH && dir[2]=='\0'))
         && (prefix==NULL || prefix[0]=='\0'))
        tmpnam(tname);
    else
        {
        if (dir==NULL)
            mydir[0]='\0';
        else if (dir[0]=='\0')
            {
            mydir[0]='.';
            mydir[1]=SLASH;
            mydir[2]='\0';
            }
        else
            strcpy(mydir,dir);
        /* In RSXNT, tempnam has the unsavory habit of switching to C: */
        /* so store the wd and switch back after calling tempnam().    */
        {
        char cwd[MAXFILENAMELEN];
        strcpy(cwd,wfile_get_wd());
        strcpy(tname,tempnam(mydir,prefix==NULL ? "" : prefix));
        wfile_set_wd(cwd);
        }
        }
    wfile_reslash(tname);
    if (tname[0]=='.' && tname[1]==SLASH)
        {
        int i;
        for (i=2;tname[i]!='\0';i++)
            tname[i-2]=tname[i];
        tname[i-2]='\0';
        }
    return(tname);
#endif /* LINUX */
    }


void wfile_abstmpnam(char *filename)

    {
    strcpy(filename,wfile_tempname(NULL,NULL));
    }



/*
** If any directory in the path of <filename> has .hush in it,
** returns -1, otherwise 0.
*/
int wfile_hushit(char *filename)

    {
    static char dir[MAXFILENAMELEN],hushfile[MAXFILENAMELEN];
    int i;

    for (i=0;filename[i]!='\0';i++)
        {
        dir[i]=filename[i];
        dir[i+1]='\0';
        if (wfile_eitherslash(dir[i]))
            {
            wfile_fullname(hushfile,dir,".hush");
            if (wfile_status(hushfile)==1)
                return(-1);
            }
        }
    wfile_fullname(hushfile,dir,".hush");
    return(wfile_status(hushfile)==1);
    }


int wfile_eitherslash(int c)

    {
    return(c=='\\' || c=='/');
    }


int wfile_slash(int c)

    {
    return(c==SLASH);
    }


/*
** Now uses double for file lengths since double is almost universally
** guaranteed to have 64 bits (IEEE doubles have a 51-bit mantissa,
** so they can accurately represent 4 petabyte (4,096 terabyte) file
** lengths.
*/
double wfile_size(char *filename)

    {
    double size;

#if (defined(WIN32) || defined(WIN64))
    HFILE   hfile;
    static OFSTRUCT buf;
    DWORD high_order_32_bits;
    static char fullname[MAXFILENAMELEN];

    wfile_expandname(fullname,filename);
    hfile=OpenFile(fullname,&buf,OF_READ);
    if (hfile==HFILE_ERROR)
        size=-1.0;
    else
        {
        HANDLE  handle;
#ifdef WIN64
        long long xhandle;

        xhandle=(long long)hfile;  /* Avoid compiler warning */
        handle=(HANDLE)xhandle;
#else
        handle=(HANDLE)hfile;
#endif
        size=(double)GetFileSize(handle,&high_order_32_bits);
        if (high_order_32_bits!=0)
            size += (double)high_order_32_bits*4294967296.0;  /* (2^32) */
        CloseHandle(handle);
        }
#elif (defined(UNIX))
    return(unix_size(filename));
#else
    FILE *f;
    long lsize;

    f=fopen(filename,"rb");
    if (f==NULL)
        size=-1.0;
    else
        {
        fseek(f,0L,2);
        lsize=ftell(f);
        fclose(f);
        size=(double)lsize;
        }
#endif
    return(size);
    }


int wfile_copy_file(char *destname,char *srcname,int append)

    {
    char    buf[514];
    int     n,status;
    FILE    *src,*dest;
    static char *writeerr="** \aWrite error copying %s to %s **\n";

    status=1;
    src=fopen(srcname,"rb");
    if (src==NULL)
        {
        wlprintf("Cannot open source file %s.\nCopy failed.\n",srcname);
        return(0);
        }
    dest=fopen(destname,append ? "ab" : "wb");
        if (dest==NULL)
            {
            wlprintf("Cannot open destination file %s.\nCopy failed.\n",destname);
            fclose(src);
            return(0);
            }
    while ((n=fread(buf,sizeof(char),512,src))>=512)
        if (fwrite(buf,sizeof(char),512,dest)<512)
            {
            wlprintf(writeerr,srcname,destname);
            status=0;
            n=0;
            break;
            }
    fclose(src);
    if (n)
        if (fwrite(buf,sizeof(char),n,dest)<n)
            {
            wlprintf(writeerr,srcname,destname);
            status=0;
            }
    if (fflush(dest)==EOF)
        {
        wlprintf(writeerr,srcname,destname);
        status=0;
        }
    fclose(dest);
    return(status);
    }


/*
** <path> can be either semi-color or colon-separated directories.
** Returns full name in <dest>
** -1 if found.
** 0 if not.
*/
int wfile_find_in_path(char *dest,char *src,char *path)

    {
    static char dir[512];
    int     i,j;

    if (wfile_eitherslash(src[0]) || src[1]==':')
        return(wfile_status(src)==1 ? -1 : 0);
    for (i=0;1;i++)
        {
        for (j=0;path[i]!='\0' && path[i]!=':' && path[i]!=';';i++)
            {
            dir[j++]=path[i];
            if (j==1 && path[i+1]==':')
                {
                i++;
                dir[j++]=path[i];
                continue;
                }
            }
        dir[j]='\0';
        clean_line(dir);
        wfile_fullname(dest,dir,src);
        if (wfile_status(dest)==1)
            return(-1);
        if (path[i]=='\0')
            break;
        }
    return(0);
    }


/*
** Shorten <filename> if it exceeds <maxlen> bytes.  Try to shorten
** it to <desired_len> bytes.
**
** Look for occurrence of <pattern> (line by line) in file <filename>.
** If a line is found that matches it, remember the most recent occurrence
** of that line and begin the shortened file with it.
**
*/
int wfile_shorten_ascii(char *filename,char *pattern,int maxlen,
                        int desired_len)

    {
    static char tempname[MAXFILENAMELEN];
    static char buf[1024];
    static char theader[1024];
    FILE    *f;
    int     lc,len,patlen;

    f=fopen(filename,"r");
    if (f==NULL)
        return(-2);
    fseek(f,0L,2);
    len=ftell(f);
    if (len <= maxlen)
        {
        fclose(f);
        return(-1);
        }
    strcpy(tempname,wfile_tempname(NULL,NULL));
    printf("Temp file = '%s'\n",tempname);
    fseek(f,0L,0);
    patlen=(pattern==NULL) ? 0 : strlen(pattern);
    theader[0]='\0';
    lc=0;
    while (fgets(buf,1023,f)!=NULL)
        {
        int     pos,c,bytesleft;

        lc++;
        pos=ftell(f);
        bytesleft=len-pos;
        if (patlen!=0 && !strnicmp(buf,pattern,patlen))
            {
            strcpy(theader,buf);
            lc=0;
            }
        if (bytesleft <= desired_len)
            {
            FILE *out;
            out=fopen(tempname,"w");
            if (out==NULL)
                {
                fclose(f);
                return(-3);
                }
            if (theader[0]!='\0')
                {
                fprintf(out,"%s",theader);
                if (lc>0)
                    fprintf(out,"[... %d lines deleted ...]\n",lc);
                }
            while ((c=fgetc(f))!=EOF)
                fputc(c,out);
            fclose(out);
            fclose(f);
            if (remove(filename) || rename(tempname,filename))
                return(-4);
            f=fopen(filename,"r");
            if (f==NULL)
                return(-5);
            fseek(f,0L,2);
            bytesleft=ftell(f);
            fclose(f);
            return(bytesleft);
            }
        }
    fclose(f);
    remove(filename);
    return(0);
    }

/*
**
** wfile_unix_style_match(char *pattern,char *name)
**
** Returns 1 if name matches pattern, 0 otherwise.
** Pattern may contain '*' for any number of any characters.
**
** If unix, comparison is case sensitive.  If WIN32, it's not.
**
** Note:  function is recursive.
**
*/
int wfile_unix_style_match(char *pattern,char *name)

    {
    int     i,j,k;

    for (i=0,j=0;pattern[i]!='\0';i++,j++)
        {
        if (pattern[i]=='*' && pattern[i+1]=='\0')
            return(1);
        if (pattern[i]=='*')
            break;
#ifdef WIN32
        if (pattern[i]!='?' && tolower((int)pattern[i])!=tolower((int)name[j]))
#else
        if (pattern[i]!='?' && pattern[i]!=name[j])
#endif
            return(0);
        if (pattern[i]=='?' && name[j]=='\0')
            return(0);
        }
    if (pattern[i]=='\0' && name[j]=='\0')
        return(1);
    if (pattern[i]=='\0')
        return(0);
    for (k=j;name[k]!='\0';k++)
        if (wfile_unix_style_match(&pattern[i+1],&name[k]))
            return(1);
    return(0);
    }


/*
**
** same as wfile_unix_style_match() but always case insensitive.
** "pattern" can have wild chars in it (* and ?)
** Returns 1 if match, 0 if not.
** Note:  function is recursive.
**
*/
int wfile_wild_match_ignore_case(char *pattern,char *name)

    {
    int     i,j,k;

    for (i=0,j=0;pattern[i]!='\0';i++,j++)
        {
        if (pattern[i]=='*' && pattern[i+1]=='\0')
            return(1);
        if (pattern[i]=='*')
            break;
        if (pattern[i]!='?' && tolower((int)pattern[i])!=tolower((int)name[j]))
            return(0);
        if (pattern[i]=='?' && name[j]=='\0')
            return(0);
        }
    if (pattern[i]=='\0' && name[j]=='\0')
        return(1);
    if (pattern[i]=='\0')
        return(0);
    for (k=j;name[k]!='\0';k++)
        if (wfile_wild_match_ignore_case(&pattern[i+1],&name[k]))
            return(1);
    return(0);
    }


int wfile_wild_match(char *pattern,char *name)

    {
    return(wfile_unix_style_match(pattern,name));
/*
#ifdef DJEMX
    return(dos_style_match(pattern,name));
#else
    return(wfile_unix_style_match(pattern,name));
#endif
*/
    }


#ifdef UNIX
static int unix_findfirst(const char *spec,wfile *wptr)

    {
    wfile_basepath(wptr->path,spec);
    wfile_basespec(wptr->unixspec,(char *)spec);
    if (wptr->path[0]=='\0')
        wptr->unixdptr=(void *)opendir(".");
    else
        wptr->unixdptr=(void *)opendir(wptr->path);
    if (wptr->unixdptr==NULL)
        return(0);
    return(unix_findnext(wptr));
    }


static int unix_findnext(wfile *wptr)

    {
    struct dirent *d;

    while (1)
        {
        d=readdir((DIR *)wptr->unixdptr);
        if (d==NULL)
            {
            closedir((DIR *)wptr->unixdptr);
            return(0);
            }
        if (wfile_wild_match(wptr->unixspec,d->d_name))
            {
            int s;
            wfile_fullname(wptr->fullname,wptr->path,d->d_name);
            strcpy(wptr->basename,d->d_name);
            unix_date(wptr->fullname,&wptr->date);
            wptr->attr = 0;
            s=unix_status(wptr->fullname);
            if (s==2)
                wptr->attr |= WFILE_DIR;
            if (unix_is_symlink(wptr->fullname,NULL))
                wptr->attr |= WFILE_SYMLINK;
            wptr->size = unix_size(wptr->fullname);
            return(1);
            }
        }
    unix_close(wptr);
    return(0);
    }


static void unix_close(wfile *wptr)

    {
    if (wptr!=NULL && wptr->unixdptr!=NULL)
        {
        /* This is commented out because it seems to crash in Win95. */
        /* Bug in EMX lib? */
        /*
        closedir((DIR *)wptr->unixdptr);
        wptr->unixdptr=NULL;
        */
        }
    }


static int unix_is_regular_file(char *filename)

    {
    /* The linux distros I've tried don't seem to like stat64 */
    /*
    struct stat64 s;

    if (stat64(filename,&s))
    */
    struct stat s;

    if (stat(filename,&s))
        return(0);
    return(S_ISREG(s.st_mode));
    }


static double unix_size(char *filename)

    {
    int status;
    /* The linux distros I've tried don't seem to like stat64 */
    /*
    struct stat64 s;

    status=stat64(filename,&s);
    */
    struct stat s;

    status=stat(filename,&s);
    if (status)
        return(-1.0);
    return((double)s.st_size);
    }


static int unix_status(char *filename)

    {
    /* The linux distros I've tried don't seem to like stat64 */
    /*
    struct stat64 s;

    if (stat64(filename,&s))
    */
    struct stat s;

    if (stat(filename,&s))
        return(0);
    if (S_ISDIR(s.st_mode))
        return(2);
    return(1);
    }


static int unix_is_symlink(char *filename,char *src)

    {
#ifdef WIN32
    return(0);
#else
    /* This doesn't seem to work in actual tests... */
    /*
    struct stat s;

    if (stat(filename,&s))
        return(0);
    return(S_ISLNK(s.st_mode));
    */
    /* Works better... */
    static char linkval[1024];
    return(readlink(filename,src==NULL?linkval:src,1023)>=0);
#endif
    }


int wfile_symlink_size(char *filename)

    {
#ifdef WIN32
    return(-1);
#else
    struct stat fs;
    int status;
    status=lstat(filename,&fs);
    if (status)
        return(-1);
    return((int)fs.st_size);
#endif
    }


int wfile_symlink_date(const char *filename,struct tm *filedate)

    {
#ifdef WIN32
    return(wfile_date(filename,filedate));
#else
    struct stat fs;
    struct tm *t;
    int status;

    status=lstat(filename,&fs);
    if (status)
        return(0);
    t=localtime(&fs.st_mtime);
    (*filedate)=(*t);
    return(1);
#endif
    }


static int unix_date(const char *filename,struct tm *filedate)

    {
    struct tm *t;
    /* The linux distros I've tried don't seem to like stat64 */
    /*
    struct stat64 s;

    if (stat64(filename,&s))
    */
    struct stat s;

    if (stat(filename,&s))
        return(0);
    t=localtime(&s.st_mtime);
    (*filedate)=(*t);
    return(1);
    }



#endif /* UNIX */


#ifdef MSDOS16

static int msdos_findfirst(const char *spec,wfile *wptr)

    {
    int     done;
    struct ffblk *ffb;
    ffb=(struct ffblk *)wptr->ds;
    wfile_basepath(wptr->path,spec);
    done=findfirst(spec,ffb,0);
    if (done)
        return(0);
    wfile_fullname(wptr->fullname,wptr->path,ffb->ff_name);
    strcpy(wptr->basename,ffb->ff_name);
    return(1);
    }


static int msdos_findnext(wfile *wptr)

    {
    struct ffblk *ffb;
    ffb=(struct ffblk *)wptr->ds;
    if (findnext(ffb))
        return(0);
    wfile_fullname(wptr->fullname,wptr->path,ffb->ff_name);
    strcpy(wptr->basename,ffb->ff_name);
    return(1);
    }


static int msdos_date(const char *filename,struct tm *filedate)

    {
    struct ffblk ff;
    long    date,time,ds1;

    if (findfirst(filename,&ff,0))
        return(0);
    time=ff.ff_ftime;
    date=ff.ff_fdate;
    ds1=(date<<16)|time;
    filedate->tm_sec  = (unsigned int)((ds1&0x1fUL)<<1);
    filedate->tm_min  = (unsigned int)((ds1&0x7e0UL)>>5);
    filedate->tm_hour = (unsigned int)((ds1&0xf800UL)>>11);
    filedate->tm_mday = (unsigned int)((ds1&0x1f0000UL)>>16);
    filedate->tm_mon  = (unsigned int)((ds1&0x1e00000UL)>>21) - 1;
    filedate->tm_year = (unsigned int)((ds1&0xfe000000UL)>>25)+80;
    filedate->tm_wday = -1;
    filedate->tm_yday = -1;
    filedate->tm_isdst = -1;
    return(1);
    /*
    int     handle;
    struct  ftime ft;

    handle=open(filename,O_RDONLY);
    if (handle<0)
        return(0);
    if (getftime(handle,&ft))
        return(0);
    filedate->tm_sec  = ft.ft_tsec*2;
    filedate->tm_min  = ft.ft_min;
    filedate->tm_hour = ft.ft_hour;
    filedate->tm_mday = ft.ft_day;
    filedate->tm_mon  = ft.ft_month;
    filedate->tm_year = ft.ft_year;
    filedate->tm_wday = -1;
    filedate->tm_yday = -1;
    filedate->tm_isdst = -1;
    return(1);
    */
    }


static int msdos_status(char *filename)

    {
    struct  ffblk finfo;
    int     done;

    done=findfirst(filename,&finfo,0);
    if (done)
        return(0);
    if (finfo.ff_attrib&FA_DIREC)
        return(2);
    return(1);
    }

#endif


/*
** Passed an initial path, recfindfirst returns the first file
** found in that directory or subdirectories.  The finfo block
** returns the file information.  The directory string returns
** the unique part of the path to the file.
** If rec = 1, the search is recursive through subdirectories.
*/
int wfile_recfindfirst(char *path,RFIND *rf,int rec)

    {
    char wildspec[MAXFILENAMELEN];

    rf->toplist=NULL;
    rf->recursive=rec;
    wfile_basespec(rf->initspec,path);
    wfile_basepath(rf->initpath,path);
    wfile_fullname(wildspec,rf->initpath,rf->initspec);
    if (!wfile_recaddone(rf))
        return(-1);
    return(wfile_recfindnext(rf));
    }


/*
** See recfindfirst().
**
** Recursively finds the next file in a directory tree.
*/
int wfile_recfindnext(RFIND *rf)

    {
    FILIST  *p;
    FILIST  *pp;
    wfile   *pwf;
    char filespec[MAXFILENAMELEN];
    int status;

/*
printf("At wfile_recfindnext(lastptr=%p)\n",wfile_lastptr(rf));
*/
    while (1)
        {
        strcpy(filespec,rf->initpath);
        if (rf->toplist==NULL)
            break;
        if (rf->toplist->next==NULL)
            {
            pwf=&rf->toplist->wf;
            pp=rf->toplist;
            p=NULL;
            }
        else
            {
            p=rf->toplist;
            strcat(filespec,p->wf.basename);
            wfile_addslash(filespec);
            while (p->next->next!=NULL)
                {
                strcat(filespec,p->next->wf.basename);
                wfile_addslash(filespec);
                p=p->next;
                }
            pwf=&p->next->wf;
            pp=p->next;
            }
/*
printf("p = %p, pp = %p, pwfname='%s'\n",p,pp,pwf->basename);
*/
        while (1)
            {
            if (pp->dirsearch==0)
                {
                char newspec[MAXFILENAMELEN];
                wfile_fullname(newspec,filespec,rf->initspec);
                status=wfile_findfirst(newspec,pwf);
                pp->dirsearch++;
                }
            else if (pp->dirsearch==1)
                status=wfile_findnext(pwf);
            else if (pp->dirsearch==2)
                {
                char newspec[MAXFILENAMELEN];
                wfile_fullname(newspec,filespec,rf->initspec);
                wfile_basepath(newspec,filespec);
                wfile_addwild(newspec,NULL);
                status=wfile_findfirstdir(newspec,pwf);
                pp->dirsearch++;
                }
            else
                status=wfile_findnextdir(pwf);
/*
printf("  next status=%d\n",status);
*/
            if (!status)
                {
                wfile_findclose(pwf);
                if (pp->dirsearch>1)
                    {
                    if (!wfile_recfreelast(rf))
                        return(0);
                    break;
                    }
                pp->dirsearch=2;
                continue;
                }
            if (wfile_status(pwf->fullname)!=2)
                {
                if (pp->dirsearch>1)
                    continue;
                wfile_wf2rf(rf);
                return(1);
                }
            if (!rf->recursive || pp->dirsearch<2 || !strcmp(pwf->basename,".")
                     || !strcmp(pwf->basename,".."))
                continue;
            if (!wfile_recaddone(rf))
                {
                wfile_recfindclose(rf);
                return(-1);
                }
            break;
            }
        }
    /* No more matching files */
    return(0);
    }


static int wfile_recaddone(RFIND *rf)

    {
    FILIST  *newlist,*p;
    static char *funcname="wfile_recaddone";
    double *dp;
    int status;

    status=willus_mem_alloc(&dp,sizeof(FILIST),funcname);
    newlist=(FILIST *)dp;
    if (!status)
        return(0);
    wfile_wfile_init(&newlist->wf);
    newlist->dirsearch=0;
    newlist->next=NULL;
    p=wfile_lastptr(rf);
    if (p==NULL)
        rf->toplist=newlist;
    else
        p->next=newlist;
    return(1);
    }


static void wfile_wfile_init(wfile *wf)

    {
    wf->fullname[0]='\0';
    wf->basename[0]='\0';
    wf->path[0]='\0';
    wf->attr=0;
    wf->winhandle=0;
    wf->ftphandle=0;
    wf->ihandle=0;
    wf->unixdptr=NULL;
    wf->unixspec[0]='\0';
    wf->ds[0]='\0';
    wf->size=0.;
    wf->date.tm_year=0;
    }


static FILIST *wfile_lastptr(RFIND *rf)

    {
    FILIST *p;

    for (p=rf->toplist;p!=NULL && p->next!=NULL;p=p->next);
    return(p);
    }

/*
static char *spaces(int level)

    {
    int     i;
    static char buf[200];

    for (i=0;i<level*3;i++)
        buf[i]=' ';
    buf[i]='\0';
    return(buf);
    }
*/


static void wfile_wf2rf(RFIND *rf)

    {
    wfile   *wf;
    FILIST  *p;

    p=wfile_lastptr(rf);
    if (p==NULL)
        return;
    wf=&p->wf;
    strcpy(rf->path,wf->path);
    strcpy(rf->fullname,wf->fullname);
    strcpy(rf->basename,wf->basename);
    strcpy(rf->directory,rf->fullname);
    wfile_unique_part(rf->directory,rf->initpath);
    }


void wfile_recfindclose(RFIND *rf)

    {
    while (wfile_recfreelast(rf));
    }


static int wfile_recfreelast(RFIND *rf)

    {
    FILIST *p;
    static char *funcname="wfile_recfreelast";

    if (rf->toplist==NULL)
        return(0);
    if (rf->toplist->next==NULL)
        {
        willus_mem_free((double **)&rf->toplist,funcname);
        return(0);
        }
    for (p=rf->toplist;p->next->next!=NULL;p=p->next);
    willus_mem_free((double **)&p->next,funcname);
    return(1);
    }


int wfile_findfirstdir(char *spec,wfile *wf)

    {
    int     status;

    for (status=wfile_findfirst(spec,wf);status;status=wfile_findnext(wf))
        if (wfile_status(wf->fullname)==2)
            return(1);
    return(0);
    }


int wfile_findnextdir(wfile *wf)

    {
    int     status;

    for (status=wfile_findnext(wf);status;status=wfile_findnext(wf))
        if (wfile_status(wf->fullname)==2)
            return(1);
    return(0);
    }


#ifdef WILLUS_HAVE_FILE64
long long wfile_seek_to(FILE *f,char *pattern)
#else
long wfile_seek_to(FILE *f,char *pattern)
#endif

    {
#ifdef WILLUS_HAVE_FILE64
    long long i;
#else
    int     i;
#endif
    int c;

    i=0;
    while ((c=fgetc(f))!=EOF)
        {
        if (c!=pattern[i])
            {
            if (!i)
                continue;
            wfile_seek(f,-i,1);
            i=0;
            continue;
            }
        i++;
        if (pattern[i]=='\0')
            {
            wfile_seek(f,-i,1);
            return(wfile_tell(f));
            }
        }
    return(-1);
    }


void wfile_touch(char *filename)

    {
    FILE *f;
    int     c;

    f=fopen(filename,"rb+");
    if (f!=NULL)
        {
        fseek(f,0L,0);
        c=fgetc(f);
        fseek(f,0L,0);
        fwrite(&c,1,1,f);
        fclose(f);
        }
    }




FILE *wfile_open_most_recent(char *wildspec,char *mode,int recursive)

    {
    FILELIST *fl,_fl;

    fl=&_fl;
    filelist_init(fl);
// printf("wildspec='%s'\n",wildspec);
    filelist_fill_from_disk_1(fl,wildspec,recursive,0);
// printf("    n=%d\n",(int)fl->n);
    if (fl->n<=0)
        return(NULL);
    filelist_sort_by_date(fl);
    wfile_fullname(wildspec,fl->dir,fl->entry[fl->n-1].name);
    return(fopen(wildspec,mode));
    }


/*
** Requires unzip to be in path.
** Assumes zip file has one file in it.  Extracts that file to the
** same folder as the zip file and replaces contents of filename with
** that file name.
**
*/
int wfile_extract_in_place(char *filename)

    {
    FILELIST *fl,_fl;
    char mypath[256];
    char relpath[256];
    char fullname[256];
    char tempdir[256];
    char wild[256];
    char curdir[256];
    char cmd[256];
    char temploc[256];
    char newloc[256];
    int status;

    strcpy(curdir,wfile_get_wd());
    strcpy(fullname,filename);
    wfile_basepath(relpath,filename);
    wfile_make_absolute(fullname);
    wfile_basepath(mypath,fullname); 
    wfile_abstmpnam(tempdir);
    wfile_makedir(tempdir);
    wfile_set_wd(tempdir);
    sprintf(cmd,"unzip -j \"%s\" 1> nul 2> nul",fullname);
    system(cmd);
    fl=&_fl;
    filelist_init(fl);
    wfile_fullname(wild,tempdir,"*");
    filelist_fill_from_disk_1(fl,wild,0,0);
    if (fl->n<=0)
        {
        filelist_free(fl);
        return(-1);
        }
    wfile_fullname(temploc,tempdir,fl->entry[0].name);
    wfile_fullname(newloc,mypath,fl->entry[0].name);
    strcpy(fullname,fl->entry[0].name);
    filelist_free(fl);
    if (wfile_status(newloc)==1)
        remove(newloc);
    status=rename(temploc,newloc);
    if (status)
        return(-2);
    wfile_set_wd(curdir);
    wfile_remove_dir(tempdir,1);
    // remove(filename);
    wfile_fullname(filename,relpath,fullname);
    return(0);
    }


/*
** Find a DLL or exe file
** Output:  fullname = full path to exe
** Input:   basename = exe/dll to find
**          folderlist[] is a list of folders to search
**              If the folder is a wildcard, the wildcard is used to search for subfolders
**              If the folder begins with '+', a recursive search for the base file is done.
**          drives = string of drives to check w/folder list, e.g. "cde" (win32 only)
**          checkpath = 1 if you want to look in PATH
**          cwd = 1 if you want to look in the current working directory
**          exedir = 1 if you want to look in the exe's directory
**          envvar !=NULL if you want to check the env var folder
**
** RETURNS 0 FOR SUCCESS
**
*/
int wfile_find_file(char *fullname,char *basename,char *folderlist[],char *drives,
                    int checkpath,int cwd,int exedir,char *envdir)

    {
    char folder[512];
    char filename[512];
    char dletter[32];
    int i,j,status;

#if (defined(WIN32) || defined(WIN64))
    if (drives!=NULL && drives[0]!='\0')
        {
        strncpy(dletter,drives,31);
        dletter[31]='\0';
        }
    else
#else
        strcpy(dletter,"c");
#endif
    if (envdir!=NULL && getenv(envdir)!=NULL)
        {
        strcpy(folder,getenv(envdir));
        if (wfile_status(folder)!=2)
            wfile_basepath(folder,getenv(envdir));
        if (wfile_status(folder)==2)
            {
            status=wfile_smartfind(fullname,basename,folder,0);
            if (status==0)
                return(status);
            }
        }
    if (checkpath)
        {
        status=wsys_which(fullname,basename);
        if (status>0)
            {
            char basespec[256];
            wfile_basespec(basespec,fullname);
            if (wfile_correct_exe(basespec,basename,fullname))
                return(0);
            }
        }
    if (cwd)
        {
        strcpy(folder,wfile_get_wd());
        status=wfile_smartfind(fullname,basename,folder,0);
        if (status==0)
            return(status);
        }
    if (exedir)
        {
        wfile_basepath(folder,wsys_full_exe_name(filename));
        status=wfile_smartfind(fullname,basename,folder,0);
        if (status==0)
            return(status);
        }
    for (i=0;dletter[i]!='\0';i++)
        {
        for (j=0;folderlist[j][0]!='\0';j++)
            {
            int k;
            k=folderlist[j][0]=='+' ? 1 : 0;
#if (defined(WIN32) || defined(WIN64))
            if (folderlist[j][k+1]==':')
                {
                if (i>0)
                    continue;
                folder[0]='\0';
                }
            else
                sprintf(folder,"%c:",dletter[i]);
#else
            folder[0]='\0';
#endif
            strcat(folder,&folderlist[j][k]);
            status=wfile_smartfind(fullname,basename,folder,k==1);
            if (status==0)
                return(status);
            }
        }
    return(-99);
    }


/*
** If folder is wild, it is searched for folders.  Those folders are [recursively]
** searched for basename.  Otherwise, folder\* is searched for basename [recursively].
**
** If a DLL, smartfind tries to load the DLL to make sure it is the correct bitness.
**
** Ret 0 for success, and fullname[] gets the full path name.
*/
int wfile_smartfind(char *fullname,char *basename,char *folder,int recursive)

    {
    char wildspec[512];
    FILELIST *fl,_fl;
    int i,status;

/*
printf("@wfile_smartfind(folder='%s',rec=%d)\n",folder,recursive);
*/
    fl=&_fl;
    filelist_init(fl);
    if (in_string(folder,"*")<0)
        {
        if (wfile_status(folder)!=2)
            return(-1);
        wfile_fullname(wildspec,folder,"*");
/*
printf("    filelist %s\n",wildspec);
*/
        filelist_fill_from_disk_1(fl,wildspec,recursive,0);
        filelist_sort_by_date(fl);
        for (i=fl->n-1;i>=0;i--)
            {
            char basespec[512];
            char locfullname[512];
            wfile_basespec(basespec,fl->entry[i].name);
            wfile_fullname(locfullname,fl->dir,fl->entry[i].name);
            if (wfile_correct_exe(basespec,basename,locfullname))
                {
                strcpy(fullname,locfullname);
                filelist_free(fl);
                return(0);
/*
printf("\n\n\n  *** FOUND IT! *** %s\n\n\n",fullname);
*/
                }
            }
        filelist_free(fl);
        return(-2);
        }
/*
printf("    filelist (dir search) %s\n",folder);
*/
    filelist_fill_from_disk_1(fl,folder,0,1);
/*
printf("        (%d results)\n",fl->n);
*/
    for (i=fl->n-1;i>=0;i--)
        {
        if (!(fl->entry[i].attr&WFILE_DIR))
            continue;
        wfile_fullname(wildspec,fl->dir,fl->entry[i].name);
        status=wfile_smartfind(fullname,basename,wildspec,1);
        if (status==0)
            {
            filelist_free(fl);
            return(status);
            }
        }
    filelist_free(fl);
    return(-3);
    }


static int wfile_correct_exe(char *basename,char *correctname,char *fullname)

    {
    if (wfile_filename_compare(basename,correctname))
        return(0);
#if (defined(WIN32) || defined(WIN64))
    if (!stricmp(wfile_ext(basename),"dll"))
        {
        if (LoadLibrary(fullname)==NULL)
            return(0);
        FreeLibrary((HMODULE)fullname);
        }
#endif
    return(1);
    }
