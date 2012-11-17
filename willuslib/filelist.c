/*
** filelist.c   Functions to operate on a list of files.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <math.h>
#include "willus.h"


static int flentry_index_by_name(FILELIST *fl,FLENTRY *en);
static int flentry_index_by_date(FILELIST *fl,FLENTRY *en);
static int filelist_disk_fill(FILELIST *fl,int index,
                              char *dirname,char *include_only[],
                              char *exclude[],int recursive,int dirstoo);
static int filelist_recursive_archive_add(FILELIST *fl,int index,
                              char *folder,char *archname,char *include_only[],
                              char *exclude[],int recursive,int dirstoo);
static void filelist_conditionally_add_entry(FILELIST *fl,FLENTRY *entry,
                                       char *include_only[],char *exclude[],
                                       int *index,int *count);
static void filelist_conditionally_add_file(FILELIST *fl,wfile *wf,
                                       char *include_only[],char *exclude[],
                                       int *index,int *count);
static int parse_exline(char *buf,double *size,int *month,int *day,int *year,
                        int *hour,int *minute,int *second,int *attr,
                        char *filename,int dirstoo);
static int parse_tarline(char *buf,double *size,int *month,int *day,int *year,
                         int *hour,int *minute,int *second,int *attr,
                         char *filename,int dirstoo);
static int parse_7zline(char *buf,double *size,int *month,int *day,int *year,
                        int *hour,int *minute,int *second,int *attr,
                        char *filename,int dirstoo);
static int parse_zipline(char *buf,double *size,int *month,int *day,int *year,
                         int *hour,int *minute,int *second,int *attr,
                         char *filename,int dirstoo);
static int nexttoken(char *dst,char *src,int *index);
static int dir_truly_empty(char *dirname);
static void filelist_tar_filename_proc(char *s,int attr);
static int is_wild(char *s);
static void filelist_realloc(FILELIST *fl,int len);


/*
** In Unix, convert sizes of symbolic links to the actual size
** of the link file (typically only a few bytes), NOT the size of
** the file referred to by the link.
*/
void filelist_convert_symlink_sizes(FILELIST *fl)

    {
#ifndef WIN32
    int i;

    for (i=0;i<fl->n;i++)
        {
        if (fl->entry[i].attr & WFILE_SYMLINK)
            {
            int size;
            struct tm newdate;
            char fullname[MAXFILENAMELEN];

            wfile_fullname(fullname,fl->dir,fl->entry[i].name);
            size=wfile_symlink_size(fullname);
            if (size>0)
                fl->entry[i].size = (double)size;
            if (wfile_symlink_date(fullname,&newdate))
                fl->entry[i].date = newdate;
            }
        }
#endif
    }
            

double filelist_total_bytes(FILELIST *fl)

    {
    double sum;
    int i;

    sum=0.;
    for (i=0;i<fl->n;i++)
        sum += fl->entry[i].size;
    return(sum);
    }


/*
** Change file references in the list so that they are referenced
** to <newdir>.
*/
void filelist_redir(FILELIST *fl,char *newdir)

    {
    int     i;

    for (i=0;i<fl->n;i++)
        {
        static char fullname[MAXFILENAMELEN];
        static char newname[MAXFILENAMELEN];

        wfile_fullname(fullname,fl->dir,fl->entry[i].name);
        wfile_remove_dots(fullname);
        wfile_relative_basename(newname,fullname,newdir);
        filelist_new_entry_name(fl,i,newname);
        }
    strcpy(fl->dir,newdir);
    }


void filelist_copy(FILELIST *dst,FILELIST *src)

    {
    int     i;

    filelist_clear(dst);
    strcpy(dst->dir,src->dir);
    for (i=0;i<src->n;i++)
        filelist_add_entry(dst,&src->entry[i]);
    dst->sorted=src->sorted;
    }


void filelist_hushlist(FILELIST *fl)

    {
    int     i;
    static char filename[MAXFILENAMELEN];

    for (i=0;i<fl->n;i++)
        {
        wfile_fullname(filename,fl->dir,fl->entry[i].name);
        if (wfile_hushit(filename))
            {
            filelist_delete_entry(fl,i);
            i--;
            }
        }
    }


/*
** Loses some file name space
*/
void filelist_hushlist_fast(FILELIST *fl)

    {
    int     i,j;
    static char filename[MAXFILENAMELEN];

    for (i=j=0;i<fl->n;i++)
        {
        wfile_fullname(filename,fl->dir,fl->entry[i].name);
        if (wfile_hushit(filename))
            continue;
        if (i!=j)
            fl->entry[j]=fl->entry[i];
        j++;
        }
    fl->n=j;
    }


/*
**
** Basically does result = set2 - set1 with these caveats:
**
**    For each file in set2, if that file:
**        (1) doesn't exist in set1, or
**        (2) exists in set1, but files differ in date and/or size, then:
**    that file is put into the result set.
**
*/
void filelist_diff(FILELIST *result,FILELIST *set2,FILELIST *set1)

    {
    int     i,newsize;
    int    *mi;
    double *dp;
    static char *funcname="filelist_diff";

    if (!willus_mem_alloc(&dp,sizeof(int)*set2->n,funcname))
        {
        fprintf(stderr,"%s: willus_mem_alloc fails, n=%g\n",funcname,(double)set2->n);
        exit(20);
        }
    mi=(int *)dp;
    newsize=0;
    for (i=0;i<set2->n;i++)
        {
        mi[i]=flentry_index(set1,&set2->entry[i]);
        if (mi[i]<0 || flentry_different(&set1->entry[mi[i]],&set2->entry[i]))
            {
/*
            struct tm *d1,*d2;
            if (mi[i]>=0)
                {
                d1=&set1->entry[mi[i]].date;
                d2=&set2->entry[i].date;
                nprintf(f,"%30s %30s\n",set1->entry[mi[i]].name,set2->entry[i].name);
                nprintf(f,"%02d/%02d/%4d %02d:%02d:%02d %9d  %02d/%02d/%4d %02d:%02d:%02d %9d\n\n",
                 d1->tm_mon+1,d1->tm_mday,d1->tm_year,d1->tm_hour,d1->tm_min,d1->tm_sec,
                 set1->entry[mi[i]].size,
                 d2->tm_mon+1,d2->tm_mday,d2->tm_year,d2->tm_hour,d2->tm_min,d2->tm_sec,
                 set2->entry[i].size);
                }
*/
            newsize++;
            }
        }
    filelist_clear(result);
    strcpy(result->dir,set2->dir);
    for (i=0;i<set2->n;i++)
        if (mi[i]<0 || flentry_different(&set1->entry[mi[i]],&set2->entry[i]))
            filelist_add_entry(result,&set2->entry[i]);
    dp=(double *)mi;
    willus_mem_free(&dp,funcname);
    mi=(int *)dp;
    }


/*
**
** Saves memory compared to filelist_diff().
**
** Basically does dst = dst - src with these caveats:
**
**    For each file in dst, if that file:
**        (1) doesn't exist in src, or
**        (2) exists in src, but files differ in date and/or size, then:
**    that file is put into the result set.
**
*/
void filelist_diff_in_situ_fast(FILELIST *dst,FILELIST *src)

    {
    int     i,j,k;

    for (i=k=0;i<dst->n;i++)
        {
        j=flentry_index(src,&dst->entry[i]);
        if (j<0 || flentry_different(&src->entry[j],&dst->entry[i]))
            {
            if (i!=k)
                dst->entry[k]=dst->entry[i];
            k++;
            continue;
            }
        // filelist_delete_entry(dst,i);
        // i--;
        }
    dst->n=k;
    }


int flentry_different(FLENTRY *e1,FLENTRY *e2)

    {
    return(wfile_datecomp(&e1->date,&e2->date) || e1->size!=e2->size);
    }


/*
**
** new = src1 + src2
**
** Where, if the name is a duplicate, then if newer, the newer
** entry is used, otherwise the older entry is used.
**
*/
void filelist_combine(FILELIST *new,FILELIST *src1,FILELIST *src2,int newer)

    {
    int     i,sizeinc;
    double *dp;
    int    *mi;
    static char *funcname="filelist_combine";

    if (!willus_mem_alloc(&dp,sizeof(int)*src2->n,funcname))
        {
        fprintf(stderr,"%s: willus_mem_alloc fails, n=%g\n",funcname,(double)src2->n);
        exit(20);
        }
    mi=(int *)dp;
    sizeinc=0;
    for (i=0;i<src2->n;i++)
        {
        mi[i]=flentry_index(src1,&src2->entry[i]);
        if (mi[i]<0)
            sizeinc++;
        }
    filelist_clear(new);
    strcpy(new->dir,src1->dir);
    for (i=0;i<src1->n;i++)
        filelist_add_entry(new,&src1->entry[i]);
    for (i=0;i<src2->n;i++)
        {
        if (mi[i]<0)
            filelist_add_entry(new,&src2->entry[i]);
        else
            {
            int date_order;
            date_order=wfile_datecomp(&new->entry[mi[i]].date,&src2->entry[i].date);
            if ((newer && date_order<0) || (!newer && date_order>0))
                new->entry[mi[i]] = src2->entry[i];
            }
        }
    new->sorted=0;
    dp=(double *)mi;
    willus_mem_free(&dp,funcname);
    mi=(int *)dp;
    }


/*
**
** Saves significant memory over filelist_combine() function.
**
** dst = dst + src
**
** Where, if the name is a duplicate, then if newer, the newer
** entry is used, otherwise the older entry is used.
**
*/
void filelist_combine_in_situ(FILELIST *dst,FILELIST *src,int newer)

    {
    int     i,j;
    double *dp;
    int    *mi;
    static char *funcname="filelist_combine_in_situ";

    if (dst->n==0)
        {
        filelist_copy(dst,src);
        return;
        }
    if (!willus_mem_alloc(&dp,sizeof(int)*src->n,funcname))
        {
        fprintf(stderr,"%s: willus_mem_alloc fails, n=%g\n",funcname,(double)src->n);
        exit(20);
        }
    mi=(int *)dp;
    for (i=0;i<src->n;i++)
        mi[i]=flentry_index(dst,&src->entry[i]);
    for (i=0;i<src->n;i++)
        {
        j=mi[i];
        if (j<0)
            filelist_add_entry(dst,&src->entry[i]);
        else
            {
            int date_order;
            date_order=wfile_datecomp(&dst->entry[j].date,&src->entry[i].date);
            if ((newer && date_order<0) || (!newer && date_order>0))
                filelist_copy_entry(dst,j,&src->entry[i]);
            }
        }
    dst->sorted=0;
    dp=(double *)mi;
    willus_mem_free(&dp,funcname);
    mi=(int *)dp;
    }


void filelist_copy_entry(FILELIST *fl,int index,FLENTRY *entry)

    {
    char *p;

    if (index<0 || index>=fl->n)
        {
        filelist_add_entry(fl,entry);
        return;
        }
    p=fl->entry[index].name;
    fl->entry[index]=(*entry);
    fl->entry[index].name=p;
    filelist_new_entry_name(fl,index,entry->name);
    }


/*
** Returns:
**     matching index or -1 if not found
*/
int flentry_index(FILELIST *fl,FLENTRY *en)

    {
    int     i;

    if (fl->sorted==1)
        return(flentry_index_by_name(fl,en));
    if (fl->sorted==2)
        return(flentry_index_by_date(fl,en));
    for (i=0;i<fl->n;i++)
        if (!wfile_filename_compare(fl->entry[i].name,en->name))
            return(i);
    return(-1);
    }


static int flentry_index_by_name(FILELIST *fl,FLENTRY *en)

    {
    int     i,low,hi,status;

    low=0;
    hi=fl->n-1;
    while (low<=hi)
        {
        i=(low+hi)>>1;
        status=wfile_filename_compare(fl->entry[i].name,en->name);
        if (!status)
            return(i);
        if (status<0)
            low=i+1;
        else
            hi=i-1;
        }
    return(-1);
    }


static int flentry_index_by_date(FILELIST *fl,FLENTRY *en)

    {
    int     i,low,hi,status;

    low=0;
    hi=fl->n-1;
    while (low<=hi)
        {
        i=(low+hi)>>1;
        status=wfile_datecomp(&fl->entry[i].date,&en->date);
        if (!status)
            return(i);
        if (status<0)
            low=i+1;
        else
            hi=i-1;
        }
    return(-1);
    }


/*
** Sort from oldest date (lower index values) to most recent date
** (higher index values)
**
*/
void filelist_sort_by_date(FILELIST *fl)

    {
    int     n,top,n1;
    FLENTRY x0;
    FLENTRY *x;

    n=fl->n;
    x=fl->entry;
    if (n<2)
        return;
    top=n/2;
    n1=n-1;
    while (1)
        {
        if (top>0)
            {
            top--;
            x0=x[top];
            }
        else
            {
            x0=x[n1];
            x[n1]=x[0];
            n1--;
            if (!n1)
                {
                x[0]=x0;
                break;
                }
            }
        {
        int parent,child;

        parent=top;
        child=top*2+1;
        while (child<=n1)
            {
            if (child<n1 && wfile_datecomp(&x[child].date,&x[child+1].date)<0)
                child++;
            if (wfile_datecomp(&x0.date,&x[child].date)<0)
                {
                x[parent]=x[child];
                parent=child;
                child+=(parent+1);
                }
            else
                break;
            }
        x[parent]=x0;
        }
        }
    fl->sorted=2;
    }


/*
** Sort from largest size to smallest size.
**
*/
void filelist_sort_by_size(FILELIST *fl)

    {
    int     n,top,n1;
    FLENTRY x0;
    FLENTRY *x;

    n=fl->n;
    x=fl->entry;
    if (n<2)
        return;
    top=n/2;
    n1=n-1;
    while (1)
        {
        if (top>0)
            {
            top--;
            x0=x[top];
            }
        else
            {
            x0=x[n1];
            x[n1]=x[0];
            n1--;
            if (!n1)
                {
                x[0]=x0;
                break;
                }
            }
        {
        int parent,child;

        parent=top;
        child=top*2+1;
        while (child<=n1)
            {
            if (child<n1 && x[child].size>x[child+1].size)
                child++;
            if (x0.size > x[child].size)
                {
                x[parent]=x[child];
                parent=child;
                child+=(parent+1);
                }
            else
                break;
            }
        x[parent]=x0;
        }
        }
    fl->sorted=3;
    }


/*
** Sort alphabetically by name.
**
*/
void filelist_sort_by_name(FILELIST *fl)

    {
    int     n,top,n1;
    FLENTRY x0;
    FLENTRY *x;

    n=fl->n;
    x=fl->entry;
    if (n<2)
        return;
    top=n/2;
    n1=n-1;
    while (1)
        {
        if (top>0)
            {
            top--;
            x0=x[top];
            }
        else
            {
            x0=x[n1];
            x[n1]=x[0];
            n1--;
            if (!n1)
                {
                x[0]=x0;
                break;
                }
            }
        {
        int parent,child;

        parent=top;
        child=top*2+1;
        while (child<=n1)
            {
            if (child<n1 && wfile_filename_compare(x[child].name,x[child+1].name)<0)
                child++;
            if (wfile_filename_compare(x0.name,x[child].name)<0)
                {
                x[parent]=x[child];
                parent=child;
                child+=(parent+1);
                }
            else
                child=n1+1;
            }
        x[parent]=x0;
        }
        }
    fl->sorted=1;
    }


/*
** Sort alphabetically by base name.
**
*/
void filelist_sort_by_basename(FILELIST *fl)

    {
    int     n,top,n1;
    FLENTRY x0;
    FLENTRY *x;

    n=fl->n;
    x=fl->entry;
    if (n<2)
        return;
    top=n/2;
    n1=n-1;
    while (1)
        {
        if (top>0)
            {
            top--;
            x0=x[top];
            }
        else
            {
            x0=x[n1];
            x[n1]=x[0];
            n1--;
            if (!n1)
                {
                x[0]=x0;
                break;
                }
            }
        {
        int parent,child;

        parent=top;
        child=top*2+1;
        while (child<=n1)
            {
            if (child<n1 && wfile_filename_basename_compare(x[child].name,x[child+1].name)<0)
                child++;
            if (wfile_filename_basename_compare(x0.name,x[child].name)<0)
                {
                x[parent]=x[child];
                parent=child;
                child+=(parent+1);
                }
            else
                break;
            }
        x[parent]=x0;
        }
        }
    fl->sorted=1;
    }


/*
** Sort first number in name, e.g. 90_my_file.txt comes before
** 100_my_file.txt.
*/
void filelist_sort_by_name_index1(FILELIST *fl)

    {
    int     n,top,n1;
    FLENTRY x0;
    FLENTRY *x;

    n=fl->n;
    x=fl->entry;
    if (n<2)
        return;
    top=n/2;
    n1=n-1;
    while (1)
        {
        if (top>0)
            {
            top--;
            x0=x[top];
            }
        else
            {
            x0=x[n1];
            x[n1]=x[0];
            n1--;
            if (!n1)
                {
                x[0]=x0;
                break;
                }
            }
        {
        int parent,child;

        parent=top;
        child=top*2+1;
        while (child<=n1)
            {
            if (child<n1 && filelist_name_index1(x[child].name)<filelist_name_index1(x[child+1].name))
                child++;
            if (filelist_name_index1(x0.name)<filelist_name_index1(x[child].name))
                {
                x[parent]=x[child];
                parent=child;
                child+=(parent+1);
                }
            else
                break;
            }
        x[parent]=x0;
        }
        }
    fl->sorted=1;
    }


/*
** Sort by number preceding extension in name, e.g. my_file_99.txt
** comes before my_file_100.txt.
*/
void filelist_sort_by_name_index2(FILELIST *fl)

    {
    int     n,top,n1;
    FLENTRY x0;
    FLENTRY *x;

    n=fl->n;
    x=fl->entry;
    if (n<2)
        return;
    top=n/2;
    n1=n-1;
    while (1)
        {
        if (top>0)
            {
            top--;
            x0=x[top];
            }
        else
            {
            x0=x[n1];
            x[n1]=x[0];
            n1--;
            if (!n1)
                {
                x[0]=x0;
                break;
                }
            }
        {
        int parent,child;

        parent=top;
        child=top*2+1;
        while (child<=n1)
            {
            if (child<n1 && filelist_name_index2(x[child].name)<filelist_name_index2(x[child+1].name))
                child++;
            if (filelist_name_index2(x0.name)<filelist_name_index2(x[child].name))
                {
                x[parent]=x[child];
                parent=child;
                child+=(parent+1);
                }
            else
                child=n1+1;
            }
        x[parent]=x0;
        }
        }
    fl->sorted=1;
    }


int filelist_name_index1(char *s)

    {
    int i;

    for (i=strlen(s)-1;i>=0 && s[i]!='/' && s[i]!='\\' && s[i]!=':';i--);
    for (i++;s[i]!='\0' && (s[i]<'0' || s[i]>'9');i++);
    return((s[i]>='0' && s[i]<='9') ? atoi(&s[i]) : -1);
    }


int filelist_name_index2(char *s)

    {
    int i;

    for (i=strlen(s)-1;i>=0 && s[i]!='.';i--);
    for (i--;i>=0 && s[i]>='0' && s[i]<='9';i--);
    if (i<-1 || s[i+1]<'0' || s[i+1]>'9')
        return(filelist_name_index1(s));
    return(atoi(&s[i+1]));
    }


int filelist_span_days(FILELIST *fl)

    {
    int     i;
    int     ioldest,inewest;
    int     days;
    time_t  t1,t2;

    if (fl->n<2)
        return(0);
    ioldest=inewest=0;
    for (i=1;i<fl->n;i++)
        {
        if (wfile_datecomp(&fl->entry[i].date,&fl->entry[ioldest].date)<0)
            ioldest=i;
        if (wfile_datecomp(&fl->entry[i].date,&fl->entry[inewest].date)>0)
            inewest=i;
        }
    t1=mktime(&fl->entry[ioldest].date);
    t2=mktime(&fl->entry[inewest].date);
    days=difftime(t2,t1)/86400.+0.5;
    return(days);
    }


/*
** Loses track of some of the file name space by not tracking it.
*/
void filelist_remove_fast(FILELIST *fl,char *pattern)

    {
    int     i,j;

    for (i=j=0;i<fl->n;i++)
        {
        if (!wfile_wild_match(pattern,fl->entry[i].name))
            {
            if (i!=j)
                fl->entry[j]=fl->entry[i];
            j++;
            continue;
            }
        }
    fl->n=j;
    }


/*
** Loses track of some of the file name space by not tracking it.
*/
void filelist_keep_only_fast(FILELIST *fl,char *pattern)

    {
    int     i,j;

    for (i=j=0;i<fl->n;i++)
        {
        if (wfile_wild_match(pattern,fl->entry[i].name))
            {
            if (i!=j)
                fl->entry[j]=fl->entry[i];
            j++;
            continue;
            }
        }
    fl->n=j;
    }


void filelist_keep_only(FILELIST *fl,char *pattern)

    {
    int     i;

    for (i=0;i<fl->n;i++)
        {
        if (wfile_wild_match(pattern,fl->entry[i].name))
            continue;
        filelist_delete_entry(fl,i);
        i--;
        }
    }


void filelist_remove(FILELIST *fl,char *pattern)

    {
    int     i;

    for (i=0;i<fl->n;i++)
        {
        if (!wfile_wild_match(pattern,fl->entry[i].name))
            continue;
        filelist_delete_entry(fl,i);
        i--;
        }
    }


/*
** Simpler way of calling "fill_from_disk".
*/
#ifdef SLASH
#undef SLASH
#endif
#if (defined(WIN32) || defined(MSDOS))
#define SLASH   '\\'
#else
#define SLASH   '/'
#endif
int filelist_fill_from_disk_1(FILELIST *fl,char *filespec,
                              int recursive,int dirstoo)

    {
    static char dir[MAXFILENAMELEN];
    static char spec[MAXFILENAMELEN];
    static char nullstr[1];
    char  *io[2];
    char  *eo[1];

    nullstr[0]='\0';
    io[0]=&spec[0];
    io[1]=&nullstr[0];
    eo[0]=&nullstr[0];
    wfile_basepath(dir,filespec);
    wfile_basespec(spec,filespec);
    while (is_wild(dir))
        {
        int i,l;
        char buf[MAXFILENAMELEN];

        l=strlen(dir);
        if (dir[l-1]==SLASH)
            {
            dir[l-1]='\0';
            l--;
            }
        for (i=l-1;i>=0 && dir[i]!=SLASH;i--);
        sprintf(buf,"%s%c%s",&dir[i+1],SLASH,spec);
        strcpy(spec,buf);
        if (i<0)
            i++;
        dir[i]='\0';
        }
    return(filelist_fill_from_disk(fl,dir,io,eo,recursive,dirstoo));
    }


static int is_wild(char *s)

    {
    int i;
    for (i=0;s[i]!='\0';i++)
        if (s[i]=='*' || s[i]=='?')
            break;
    return(s[i]!='\0');
    }


/*
** fl->dir becomes the root of the zip file.
*/
int filelist_create_zipfile(FILELIST *fl,char *zipfile,FILE *out)

    {
    char zipfileabs[512];
    char zipdir[512];
    char curdir[512];
    char tmpfile[512];
    char cmd[1024];
    FILE *f;
    int i;

/*
    if (wsys_which(zipexe,"zip")==0)
        {
        nprintf(out,"make zipfile:  Cannot find zip exe.\n");
        return(-1);
        }
    wfile_make_absolute(zipexe);
*/
    strcpy(zipfileabs,zipfile);
    wfile_make_absolute(zipfileabs);
    if (wfile_status(zipfileabs)==1)
        remove(zipfileabs);
    if (wfile_status(zipfileabs)!=0)
        {
        nprintf(out,"make zipfile:  %s already exists.\n",zipfileabs);
        return(-1);
        }
    strcpy(zipdir,fl->dir);
    wfile_make_absolute(zipdir);
    wfile_abstmpnam(tmpfile);
    wfile_make_absolute(tmpfile);
    f=fopen(tmpfile,"w");
    if (f==NULL)
        {
        nprintf(out,"make zipfile:  Cannot open temp file %s.\n",tmpfile);
        return(-2);
        }
    for (i=0;i<fl->n;i++)
        fprintf(f,"%s\n",fl->entry[i].name);
    fclose(f);
    strcpy(curdir,wfile_get_wd());
    wfile_set_wd(zipdir);
#ifdef WIN32
    sprintf(cmd,"zip \"%s\" -@ < \"%s\" 1> z1.out 2> z1.err",zipfileabs,tmpfile);
#else
    sprintf(cmd,"zip \"%s\" -@ < \"%s\" > /dev/null",zipfileabs,tmpfile);
#endif
    system(cmd);
    wfile_set_wd(curdir);
    remove(tmpfile);
    if (wfile_status(zipfileabs)!=1)
        {
        nprintf(out,"make zipfile:  File %s not created.\n",zipfileabs);
        return(-3);
        }
    return(0);
    }


/*
** Load disk directory tree into file list.
**
** This function is designed to behave like the "zip" program.
**
** This function reads all of the files in the "dirname," which
** must be a directory.
**
** For each file in this directory, if it is a file and it matches
** an entry in include_only and doesn't match anything in exclude,
** it is included.  If it is a directory and recursive is set to 1,
** it is also scanned with the same rules.  Recursive directories
** do NOT have to match the include_only or exclude lists.
** NOTE:  matches are done on FULL path names.
**
*/
int filelist_fill_from_disk(FILELIST *fl,char *dirname,char *include_only[],
                            char *exclude[],int recursive,int dirstoo)

    {
    wfile_noslash(fl->dir,dirname);
    filelist_clear(fl);
/*
    if (dirstoo==4)
        {
        filelist_put_keepme_files(fl,0,dirname,include_only,exclude,recursive);
        dirstoo=3;
        }
*/
    return(filelist_disk_fill(fl,0,dirname,include_only,exclude,
                              recursive,dirstoo));
    }




/*
** dirstoo == 0  :  Only files
** dirstoo == 1  :  Dirs and files
** dirstoo == 2  :  Files, symbolic links to files, symoblic links to
**                  dirs, and empty dirs.  But don't recurse symbolic
**                  links to dirs.
** dirstoo == 3  :  Same as 2, but don't include directories that are
**                  empty if you take away all excluded files, but not
**                  empty otherwise.
*/
static int filelist_disk_fill(FILELIST *fl,int index,
                              char *dirname,char *include_only[],
                              char *exclude[],int recursive,int dirstoo)

    {
    wfile   wf;
    int     is_archive,i,count,s;
    char wildspec[MAXFILENAMELEN];
    char newdir[MAXFILENAMELEN];
    char unique[MAXFILENAMELEN];

/*
printf("fdf: index=%d, dirname='%s', io[0]='%s', rec=%d, dt=%d\n",
index,dirname,include_only[0],recursive,dirstoo);
*/
    is_archive = (recursive>1 && wfile_is_archive(dirname));
    if (is_archive)
        {
        char dir[MAXFILENAMELEN];
        wfile_basepath(dir,dirname);
        wfile_unique_part(dir,fl->dir);
        return(filelist_recursive_archive_add(fl,index,dir,dirname,include_only,exclude,recursive,dirstoo));
        }
    if (recursive || include_only[0]=='\0' || include_only[1]!='\0')
        wfile_fullname(wildspec,dirname,"*");
    else
        wfile_fullname(wildspec,dirname,include_only[0]);
    i=index;
    count=0;
    for (s=wfile_findfirst(wildspec,&wf);s;s=wfile_findnext(&wf))
        {
        int fstatus,is_archive;

        if (!strcmp(wf.basename,".") || !strcmp(wf.basename,".."))
            continue;
        fstatus=wfile_status(wf.fullname);
        is_archive=wfile_is_archive(wf.fullname);
        if (fstatus==2 && (dirstoo!=1 || recursive))
            continue;
        /* If archive file and we want to look into archives, then skip it. */
        if (is_archive && recursive>1)
            continue;
/*
        if (fstatus==2 && (recursive || dirstoo==0 || dirstoo==2 || dirstoo==3))
            continue;
*/
        /* Regular file includes sym link to regular file or broken symlink */
        if (fstatus!=2 && !wfile_is_regular_file(wf.fullname)
              && (fstatus!=0 || !wfile_is_symlink(wf.fullname)))
            continue;
        filelist_conditionally_add_file(fl,&wf,include_only,exclude,&i,&count);
        }
    wfile_findclose(&wf);
    if (!recursive)
        return(count);
    for (s=wfile_findfirst(wildspec,&wf);s;s=wfile_findnext(&wf))
        {
        int     n,archive;

        if (!strcmp(wf.basename,".") || !strcmp(wf.basename,".."))
            continue;

        archive = (recursive>1 && wfile_is_archive(wf.fullname));
        if (wfile_status(wf.fullname)!=2 && !archive)
            continue;
        /* Do not recurse symbolic links to dirs */
        if (wf.attr & WFILE_SYMLINK)
            {
            /* Store the dir sym link if requested. */
            if (dirstoo==1 || dirstoo==2 || dirstoo==3)
                filelist_conditionally_add_file(fl,&wf,include_only,exclude,
                                                   &i,&count);
            continue;
            }
        strcpy(unique,wf.fullname);
        wfile_unique_part(unique,fl->dir);
        if (filelist_dir_excluded(unique,include_only,exclude))
            continue;
        wfile_fullname(newdir,dirname,wf.basename);
        n=filelist_disk_fill(fl,i,newdir,include_only,exclude,recursive,dirstoo);
        /* If empty dir and dirstoo==2, store it. */
        if (n==0 && (dirstoo==1 || dirstoo==2 
                            || (dirstoo==3 && dir_truly_empty(wf.fullname))))
            {
            filelist_conditionally_add_file(fl,&wf,include_only,exclude,
                                               &i,&count);
            continue;
            }
        if (n>0 && dirstoo==1)
            filelist_conditionally_add_file(fl,&wf,NULL,NULL,&i,&count);
        i+=n;
        count+=n;
        }
    wfile_findclose(&wf);
    return(count);
    }


/*
** Ideally will unzip a zip file within a zip file when recursive == 3.  
** Right now it doesn't do this.
** archfile = physical path to archive (may be temporary file)
** archdir + basespec(archfile) = full pseudo-path to archive
*/
static int filelist_recursive_archive_add(FILELIST *dst,int index,
                              char *archdir,char *archfile,char *include_only[],
                              char *exclude[],int recursive,int dirstoo)

    {
    char tempname[MAXFILENAMELEN];
    char cmd[MAXFILENAMELEN+128];
    char basename[MAXFILENAMELEN];
    char archdir1[MAXFILENAMELEN];
    WZFILE *f;
    FILELIST *fl2,_fl2;
    FILELIST *fl,_fl;
    int count,i,n;

// printf("@archive_add(archdir='%s',archfile='%s')\n",archdir,archfile);
    wfile_basespec(basename,archfile);
    wfile_fullname(archdir1,archdir,basename);
// printf("@archive_add(dir='%s',file='%s'...)\n",archdir,archfile);
    count=0;
    fl=&_fl;
    filelist_init(fl);
    fl->dir[0]='\0';
    wfile_abstmpnam(tempname);
// printf("Extracting from %s...\n",archfile);
    if (!stricmp(wfile_ext(archfile),"zip"))
#ifdef WIN32
        sprintf(cmd,"unzip -C -v \"%s\" > \"%s\"",archfile,tempname);
#else
        sprintf(cmd,"unzip -v \"%s\" > \"%s\"",archfile,tempname);
#endif
    else
        sprintf(cmd,"7z l \"%s\" > \"%s\"",archfile,tempname);
    system(cmd);
    f=wzopen(tempname,"rb");
    if (f==NULL)
        return(-1);
    filelist_fill_from_archive_ex(fl,f,0,0,archdir1,include_only,exclude);
    if (recursive==3)
        {
        wzrewind(f);
        fl2=&_fl2;
        filelist_init(fl2);
        strcpy(fl2->dir,archfile);
        filelist_fill_from_archive_ex(fl2,f,0,0,NULL,NULL,NULL);
        }
    wzclose(f);
    remove(tempname);
    /* Recursively add internal archives */
    if (recursive==3)
        {
        for (i=0;i<fl2->n;i++)
            {
            if (wfile_is_archive(fl2->entry[i].name))
                {
                char entryname[MAXFILENAMELEN];
                char tempname[MAXFILENAMELEN];
                WZFILE *src;

                wfile_fullname(entryname,archdir1,fl2->entry[i].name);
                src=wzopen_special(NULL,entryname,tempname);
                if (src!=NULL)
                    {
                    char tempdir[MAXFILENAMELEN];
                    char basename[MAXFILENAMELEN];
                    char temparch[MAXFILENAMELEN];
                    FILE *dest;

                    wfile_abstmpnam(tempdir);
                    wfile_makedir(tempdir);
                    wfile_basespec(basename,fl2->entry[i].name);
                    wfile_fullname(temparch,tempdir,basename);
                    dest=fopen(temparch,"wb");
                    if (dest!=NULL)
                        {
                        int c;
                        char archdir2[MAXFILENAMELEN];
                        char archfullname[MAXFILENAMELEN];

                        wfile_fullname(archfullname,archdir1,fl2->entry[i].name);
                        wfile_basepath(archdir2,archfullname);
                        while ((c=wzgetc(src))!=EOF)
                            fputc(c,dest);
                        fclose(dest);
                        count+=filelist_recursive_archive_add(dst,index,
                                      archdir2,temparch,
                                  include_only,exclude,recursive,dirstoo);
                        wzfile_fully_remove(temparch);
                        }
                     wzclose(src);
                     wzfile_fully_remove(tempname);
                     }
                 }
            }
        filelist_free(fl2);
        }
    for (i=0;i<fl->n;i++)
        filelist_add_entry(dst,&fl->entry[i]);
    n=fl->n;
    filelist_free(fl);
    return(n);
    }


/*
** Dates files recursively based on the files inside AND removes
** files with no sub files
*/
void filelist_date_recursively(FILELIST *fl)

    {
    int i,j;
    FILELIST _fl2,*fl2;

    fl2=&_fl2;
    filelist_init(fl2);
    for (i=0;i<fl->n;i++)
        {
        char s1[MAXFILENAMELEN];
        char spec[MAXFILENAMELEN];

        if (!(fl->entry[i].attr & WFILE_DIR))
            continue;
        wfile_fullname(s1,fl->entry[i].name,"*");
        wfile_fullname(spec,fl->dir,s1);
        filelist_fill_from_disk_1(fl2,spec,1,0);
        /* Remove if no subfiles */
        if (fl2->n<=0)
            {
            for (j=i;j<fl->n-1;j++)
                fl->entry[j]=fl->entry[j+1];
            fl->n--;
            i--;
            continue;
            }
        filelist_sort_by_date(fl2);
        fl->entry[i].date = fl2->entry[fl2->n-1].date;
        filelist_clear(fl2);
        }
    filelist_free(fl2);
    }


static int dir_truly_empty(char *dirname)

    {
    wfile wf;
    int n,s;
    char wildspec[MAXFILENAMELEN];

    wfile_fullname(wildspec,dirname,"*");
    for (n=0,s=wfile_findfirst(wildspec,&wf);s;s=wfile_findnext(&wf))
        {
        if (!strcmp(wf.basename,".") || !strcmp(wf.basename,".."))
            continue;
        n++;
        }
    wfile_findclose(&wf);
    return(n==0);
    }


static void filelist_conditionally_add_entry(FILELIST *fl,FLENTRY *entry,
                                       char *include_only[],char *exclude[],
                                       int *index,int *count)

    {
    char unique[MAXFILENAMELEN];

    strcpy(unique,entry->name);
    wfile_unique_part(unique,fl->dir);
    if (filelist_use_file(unique,include_only,exclude))
        {
        filelist_add_entry(fl,entry);
        if (index!=NULL)
            (*index)=(*index)+1;
        if (count!=NULL)
            (*count)=(*count)+1;
        }
    }


static void filelist_conditionally_add_file(FILELIST *fl,wfile *wf,
                                       char *include_only[],char *exclude[],
                                       int *index,int *count)

    {
    char unique[MAXFILENAMELEN];

    strcpy(unique,wf->fullname);
    wfile_unique_part(unique,fl->dir);
    if (filelist_use_file(unique,include_only,exclude))
        {
        static FLENTRY entry;

        entry.name=unique;
#ifdef WIN32
        entry.date = wf->date;
        entry.size = wf->size;
        entry.attr = wf->attr;
        if (entry.attr & WFILE_DIR)
            entry.size = 0;
#else
        wfile_date(wf->fullname,&entry.date);
        entry.size=wfile_size(wf->fullname);
        entry.attr=0;
        if (wfile_status(wf->fullname)==2)
            {
            entry.attr |= WFILE_DIR;
            entry.size = 0;
            }
        if (wfile_is_symlink(wf->fullname))
            entry.attr |= WFILE_SYMLINK;
#endif
        filelist_add_entry(fl,&entry);
        (*index) = (*index) + 1;
        (*count) = (*count) + 1;
        }
    }


void filelist_filter(FILELIST *fl,char *include[],char *exclude[])

    {
    int i;

    for (i=0;i<fl->n;i++)
        {
        if (!filelist_use_file(fl->entry[i].name,include,exclude))
            {
            filelist_delete_entry(fl,i);
            i--;
            }
        }
    }
            
/*
** Truncate seconds to zero in date field
*/
void filelist_zero_seconds(FILELIST *fl)

    {
    int     i;

    for (i=0;i<fl->n;i++)
        {
        /* Round seconds to zero since zip files don't report seconds */
        fl->entry[i].date.tm_sec  = 0;
        }
    /* If was sorted by date, reset the sort flag */
    if (fl->sorted==2)
        fl->sorted=0;
    }


int filelist_use_file(char *fullname,char *include_only[],char *exclude[])

    {
    int j;

    for (j=0;exclude!=NULL && exclude[j][0]!='\0';j++)
        if (wfile_wild_match(exclude[j],fullname))
            return(0);
    if (include_only==NULL || include_only[0][0]=='\0')
        return(-1);
    for (j=0;include_only[j][0]!='\0';j++)
        if (wfile_wild_match(include_only[j],fullname))
            return(-1);
    return(0);
    }


int filelist_dir_excluded(char *dirname,char *include_only[],char *exclude[])

    {
    int j;

    for (j=0;exclude!=NULL && exclude[j][0]!='\0';j++)
        if (filelist_dir_name_match(exclude[j],dirname))
            return(-1);
    /*
    if (include_only==NULL || include_only[0][0]=='\0')
        return(0);
    for (j=0;include_only[j][0]!='\0';j++)
        if (filelist_dir_name_match(include_only[j],dirname))
            return(0);
    return(-1);
    */
    return(0);
    }


#if (0)
/*
** Return TRUE (-1) if the pattern covers dirname and all subdirectories.
** Otherwise return FALSE (0).
** E.g. if pattern = "Com*" and dir = "Company Data", then returns TRUE.
** if pattern = "*Data\*" and dir = "Company Data", returns TRUE.
** if pattern = "C*Data" and dir = "Company Data", returns FALSE.
*/
#endif
int filelist_dir_name_match(char *pattern,char *dirname)

    {
    int len;

    len=strlen(pattern);
    if (len>2 && wfile_eitherslash(pattern[len-2]) && pattern[len-1]=='*')
        {
        char tname[MAXFILENAMELEN];
        strcpy(tname,pattern);
        tname[len-2]='\0';
        return(wfile_wild_match(tname,dirname));
        }
    if (!wfile_wild_match(pattern,dirname))
        return(0);
    return(pattern[len-1]=='*');
    }


/*
** If the zip file or .7z file was made during standard time,
** then WIN32 (and Linux for .7z) reports the dates of files that
** were made during daylight savings time differently from the O/S.
** This function corrects for that inconsistency.
**
** NOTICE!!  Use a NULL or empty file name for 7-zip!!
**
** NOTE 1:  Starting with Info-Zip v2.3, this correction is no longer
**           needed, because zip 2.3 does the correction already.
**           (I'm vindicated!).
** NOTE 2:  7-zip still has this problem, though (v4.23) in both WIN32
**          and Linux.
** NOTE 3:  7-zip acts just like Windows.  Let's say you archive a file
**          that you created at 3:00pm in February, when DST is not in
**          effect.  Then you do "7za l <file>" in February, and it will
**          report the archived file date stamp as 3pm.  But if you do
**          "7za l <file>" on that EXACT same file in May, when DST is
**          in effect, 7-zip will report the date of the archived file as 4pm.
**          BECAUSE I correct for this idiocy in Windows, I have to correct
**          for it in 7-zip as well.
**
*/
void filelist_adjust_archive_datestamps_for_dst(FILELIST *fl,char *zipfile)

    {
    int     i;
    struct tm _zd,*zd;

    zd=&_zd;
    /* With 7-zip, you want to use the current time rather than the archive */
    /* time stamp in making the DST correction--see NOTE 3 above.           */
    if (zipfile==NULL || zipfile[0]=='\0')
        {
        time_t now;
        time(&now);
        (*zd)=(*localtime(&now));
        }
    else
        wfile_date(zipfile,zd);
    for (i=0;i<fl->n;i++)
        {
        struct tm *fd;
        fd=&fl->entry[i].date;
        if ((zd->tm_isdst && fd->tm_isdst) || (!zd->tm_isdst && !fd->tm_isdst))
            continue;
        if (!zd->tm_isdst)
            wfile_increment_hour(fd);
        else
            wfile_decrement_hour(fd);
        }
    }


void filelist_write_7zstyle_list(FILELIST *fl,FILE *out)

    {
    int     i;

    for (i=0;i<fl->n;i++)
        {
        struct tm *date;

        date=&fl->entry[i].date;
        fprintf(out,"%04d-%02d-%02d %02d:%02d:%02d %c...%c %12g x %s\n",
                date->tm_year+1900,date->tm_mon+1,date->tm_mday,
                date->tm_hour,date->tm_min,date->tm_sec,
                (fl->entry[i].attr&WFILE_DIR) ? 'D' : '.',
                (fl->entry[i].attr&WFILE_DIR) ? '.' : 'A',
                fl->entry[i].size,
                fl->entry[i].name);
        }
    }


void filelist_write_zipstyle_list(FILELIST *fl,FILE *out)

    {
    int     i;

    for (i=0;i<fl->n;i++)
        {
        struct tm *date;

        date=&fl->entry[i].date;
        fprintf(out,"%9ld x x x %02d-%02d-%02d %02d:%02d x %s%s\n",
                (long)fl->entry[i].size,date->tm_mon+1,date->tm_mday,
                date->tm_year%100,date->tm_hour,date->tm_min,
                fl->entry[i].name,(fl->entry[i].attr&WFILE_DIR)?"/":"");
        }
    }


int filelist_write_tar_list(FILELIST *fl,char *filename)

    {
    WZFILE *f;
    char buf[64];
    int i;

    f=wzopen(filename,"wb");
    if (f==NULL)
        return(-1);
    for (i=0;i<fl->n;i++)
        {
        FLENTRY *entry;
        entry=&fl->entry[i];
        if (entry->attr&WFILE_SYMLINK)
            wzprintf(f,"s");
        else if (entry->attr&WFILE_DIR)
            wzprintf(f,"d");
        else
            wzprintf(f,"-");
        wzprintf(f,"rwxrwxrwx 999/999 ");
        sprintf(buf,"%.1f",entry->size);
        buf[strlen(buf)-2]='\0';
        wzprintf(f,"%15s %04d-%02d-%02d %02d:%02d:%02d %s%s\n",
             buf,
             entry->date.tm_year+1900,entry->date.tm_mon+1,entry->date.tm_mday,
             entry->date.tm_hour,entry->date.tm_min,entry->date.tm_sec,
             entry->name,(entry->attr&WFILE_DIR)?"/":"");
        }
    wzclose(f);
    return(0);
    }


int filelist_fill_from_zip(FILELIST *fl,char *zipfile,char *wildspec)

    {
    char tempname[256];
    char cmd[384];
    WZFILE *f;

    strcpy(fl->dir,zipfile);
    wfile_abstmpnam(tempname);
    if (!stricmp(wfile_ext(zipfile),"7z"))
        sprintf(cmd,"7z l \"%s\" > \"%s\"",zipfile,tempname);
    else
#ifdef WIN32
        sprintf(cmd,"unzip -C -v \"%s\" > \"%s\"",zipfile,tempname);
#else
        sprintf(cmd,"unzip -v \"%s\" > \"%s\"",zipfile,tempname);
#endif
    system(cmd);
    f=wzopen(tempname,"rb");
    if (f==NULL)
        return(-1);
    filelist_fill_from_archive(fl,f,0,0);
    wzclose(f);
    remove(tempname);
    filelist_keep_only_fast(fl,wildspec);
    return(0);
    }


int filelist_fill_from_archive(FILELIST *fl,WZFILE *f,int append,int dirstoo)

    {
    return(filelist_fill_from_archive_ex(fl,f,append,dirstoo,NULL,NULL,NULL));
    }


/*
** Works with "unzip -v" output or "7za l" output.
*/
int filelist_fill_from_archive_ex(FILELIST *fl,WZFILE *f,int append,int dirstoo,
                                  char *prepend,char *include_only[],char *exclude[])

    {
    char    buf[356];
    int     month,day,year,hour,minute,second,count;
    double  size;
    char    filename[MAXFILENAMELEN];

    if (!append)
        filelist_clear(fl);
    count=0;
    while (wzgets(buf,355,f)!=NULL)
        {
        time_t  t;
        struct tm *lt;
        static FLENTRY entry;
        double maxyr;

        clean_line(buf);
        if (prepend!=NULL)
            {
            strcpy(filename,prepend);
            wfile_goodpath(filename,NULL);
            }
        else
            filename[0]='\0';
        if (!parse_exline(buf,&size,&month,&day,&year,&hour,&minute,&second,
                              &entry.attr,&filename[strlen(filename)],dirstoo))
            continue;
        entry.name=filename;
        wfile_reslash(entry.name);
        entry.date.tm_sec  = second;
        entry.date.tm_min  = minute;
        entry.date.tm_hour = hour;
        entry.date.tm_mday = day;
        entry.date.tm_mon  = month-1;
        entry.date.tm_year = year-1900;
        entry.date.tm_wday = -1; /* Sun = 0, Mon = 1, ... */
        entry.date.tm_yday = -1;
        entry.date.tm_isdst= -1;
        /* Avoid mktime() returning -1, which will crash the system */
        maxyr = floor(pow(2.,sizeof(time_t)*8.)/3600./24./365.24225)-1.;
        if (entry.date.tm_year < 70 || entry.date.tm_year > maxyr)
            fprintf(stderr,"File '%s' has date year of %d (adjusted to %d)!\n",
                   filename,entry.date.tm_year+1900,
                   entry.date.tm_year<70 ? 1970 : (int)(maxyr+1900));
        if (entry.date.tm_year < 70)
            entry.date.tm_year = 70;
        if (entry.date.tm_year > maxyr)
            entry.date.tm_year = maxyr;
        t=mktime(&entry.date);
        lt=localtime(&t);
        entry.date.tm_wday = lt->tm_wday;
        entry.date.tm_yday = lt->tm_yday;
        entry.date.tm_isdst= lt->tm_isdst;
        entry.size=size;
        filelist_conditionally_add_entry(fl,&entry,include_only,exclude,NULL,&count);
        }
    /* return(fl->n); */
    return(count);
    }


static int parse_exline(char *buf,double *size,int *month,int *day,int *year,
                        int *hour,int *minute,int *second,int *attr,
                        char *filename,int dirstoo)

    {
    int i,num1;

    /* If tar -t output, first 10 chars should be, e.g. -rw-rw-rw- */
    num1=0;
    for (i=0;i<10 && buf[i]!='\0';i++)
        if (buf[i]>='0' && buf[i]<='9')
            {
            num1=1;
            break;
            }
    if (!num1 && buf[10]==' ' && buf[11]!=' ')
        return(parse_tarline(buf,size,month,day,year,hour,minute,second,
                             attr,filename,dirstoo));
    if (buf[4]=='-')
        return(parse_7zline(buf,size,month,day,year,hour,minute,second,
                            attr,filename,dirstoo));
    return(parse_zipline(buf,size,month,day,year,hour,minute,second,
                         attr,filename,dirstoo));
    }


/*
** Get file entry from tar -t output
*/
static int parse_tarline(char *buf,double *size,int *month,int *day,int *year,
                         int *hour,int *minute,int *second,int *attr,
                         char *filename,int dirstoo)

    {
    char tbuf[MAXFILENAMELEN];
    int  i,j,created_by_filelist_function;
    static char *months[]={"jan","feb","mar","apr","may","jun",
                           "jul","aug","sep","oct","nov","dec"};

    i=0;
    /* ATTRIBUTES, e.g. -rw-rw-rw- */
    (*attr)=0;
    if (!nexttoken(tbuf,buf,&i))
        return(0);
    if (tolower(tbuf[0])=='l')
        (*attr) = (*attr) | WFILE_SYMLINK;
    for (j=0;tolower(buf[j])!='w' && buf[j]!='\0';j++);
    if (buf[j]=='\0')
        (*attr) = (*attr) | WFILE_READONLY;
    /* Check if directory */
    if (tolower(tbuf[0])=='d')
        {
        if (!dirstoo)
            return(0);
        (*attr) = (*attr) | WFILE_DIR;
        }
    /* File owner/group, e.g. will/users */
    if (!nexttoken(tbuf,buf,&i))
        return(0);
    created_by_filelist_function=!stricmp(tbuf,"999/999");

    /* SIZE (uncompressed) */
    if (!nexttoken(tbuf,buf,&i))
        return(0);
    if (!is_a_number(tbuf))
        return(0);
    (*size)=atof(tbuf);

    /* DATE AND TIME */
    /* Next is date, either Jan 01 HH:MM YYYY or YYYY-MM-DD HH:MM:SS */
    if (!nexttoken(tbuf,buf,&i))
        return(0);
    for (j=0;j<12;j++)
        if (!stricmp(tbuf,months[j]))
            break;
    if (j<12)
        {
        (*month)=j+1;
        /* DAY OF MONTH */
        if (!nexttoken(tbuf,buf,&i))
            return(0);
        (*day)=atoi(tbuf);
        /* HH:MM */
        if (!nexttoken(tbuf,buf,&i))
            return(0);
        if (tbuf[2]!=':')
            return(0);
        tbuf[2]='\0';
        (*hour)=atoi(tbuf);
        (*minute)=atoi(&tbuf[3]);
        (*second)=0;
        /* YEAR */
        if (!nexttoken(tbuf,buf,&i))
            return(0);
        (*year)=atoi(tbuf); 
        }
    else
        {
        /* DATE, YYYY-MM-DD */
        if (strlen(tbuf)!=10 || tbuf[4]!='-' || tbuf[7]!='-')
            return(0);
        tbuf[4]='\0';
        tbuf[7]='\0';
        (*day)=atoi(&tbuf[8]);
        (*month)=atoi(&tbuf[5]);
        (*year)=atoi(tbuf);
        /* TIME, HH:MM:SS */
        if (!nexttoken(tbuf,buf,&i))
            return(0);
        if (strlen(tbuf)!=8 || tbuf[2]!=':' || tbuf[5]!=':')
            return(0);
        tbuf[2]='\0';
        tbuf[5]='\0';
        (*hour)=atoi(tbuf);
        (*minute)=atoi(&tbuf[3]);
        (*second)=atoi(&tbuf[6]);
        }

    /* FILE NAME */
    if (strlen(&buf[i])<=0)
        return(0);
    strcpy(filename,&buf[i]);
    if (!created_by_filelist_function)
        filelist_tar_filename_proc(filename,(*attr));
#ifdef WIN32
    wfile_slash_this_way(filename,'/');  // Make it match zip listing
#endif
    wfile_noslash(filename,NULL);
    return(-1);
    }


/*
** Get file entry from 7za l list file
*/
static int parse_7zline(char *buf,double *size,int *month,int *day,int *year,
                        int *hour,int *minute,int *second,int *attr,
                        char *filename,int dirstoo)

    {
    char tbuf[MAXFILENAMELEN];
    char dbuf[MAXFILENAMELEN];
    int  i;
    struct tm date;

    (*attr)=0;
    i=0;
    /* DATE */
    if (!nexttoken(tbuf,buf,&i))
        return(0);
   
    if (strlen(tbuf)<6 || strlen(tbuf)>10)
        return(0);
    strcpy(dbuf,tbuf); 

    /* TIME */
    if (!nexttoken(tbuf,buf,&i))
        return(0);
    if (strlen(tbuf)<3 || in_string(tbuf,":")<0)
        return(0);
    strcat(dbuf," ");
    strcat(dbuf,tbuf);
    structtm_from_datetime(&date,dbuf);
    (*year)=date.tm_year+1900;
    (*month)=date.tm_mon+1;
    (*day)=date.tm_mday;
    (*hour)=date.tm_hour;
    (*minute)=date.tm_min;
    (*second)=date.tm_sec;

    /* ATTRIBUTES */
    if (!nexttoken(tbuf,buf,&i))
        return(0);
    /* If directory, ignore */
    if (tolower(tbuf[0])=='d')
        {
        if (!dirstoo)
            return(0);
        (*attr) = (*attr) | WFILE_DIR;
        }

    /* SIZE (uncompressed) */
    if (!nexttoken(tbuf,buf,&i))
        return(0);
    if (!is_a_number(tbuf))
        return(0);
    (*size)=atof(tbuf);

    /* FILE NAME */
    if (i<52) /* Skip compressed size if it's there */
        if (!nexttoken(tbuf,buf,&i))
            return(0);
    if (strlen(&buf[i])<=0)
        return(0);
    strcpy(filename,&buf[i]);
    wfile_slash_this_way(filename,'/');  // Make it match zip listing
    return(-1);
    }


/*
** Get file entry from zip list file
*/
static int parse_zipline(char *buf,double *size,int *month,int *day,int *year,
                         int *hour,int *minute,int *second,int *attr,
                         char *filename,int dirstoo)

    {
    char tbuf[MAXFILENAMELEN];
    char dbuf[MAXFILENAMELEN];
    int  i;
    struct tm date;

    i=0;
    (*attr)=0;

    /* Size */
    if (!nexttoken(tbuf,buf,&i))
        return(0);
    if (!is_a_number(tbuf))
        return(0);
    (*size)=atof(tbuf);

    /* Date */
    if (!nexttoken(tbuf,buf,&i) || !nexttoken(tbuf,buf,&i)
         || !nexttoken(tbuf,buf,&i) || !nexttoken(tbuf,buf,&i))
        return(0);
    if (strlen(tbuf)<6 || strlen(tbuf)>10)
        return(0);
    strcpy(dbuf,tbuf);

    /* Time */
    if (!nexttoken(tbuf,buf,&i))
        return(0);
    if (strlen(tbuf)<3 || in_string(tbuf,":")<0)
        return(0);
    strcat(dbuf," ");
    strcat(dbuf,tbuf);
    structtm_from_datetime(&date,dbuf);
    (*year)=date.tm_year+1900;
    (*month)=date.tm_mon+1;
    (*day)=date.tm_mday;
    (*hour)=date.tm_hour;
    (*minute)=date.tm_min;
    (*second)=date.tm_sec;

    /* File name */
    if (!nexttoken(tbuf,buf,&i))
        return(0);
    if (strlen(&buf[i])<=0)
        return(0);
    strcpy(filename,&buf[i]);

    /* If it's a directory, don't count it */
    if (wfile_eitherslash(filename[strlen(filename)-1]))
        {
        if (!dirstoo)
            return(0);
        (*attr) |= WFILE_DIR;
        wfile_noslash(filename,NULL);
        }
    return(-1);
    }


static int nexttoken(char *dst,char *src,int *index)

    {
    int     i,j;

    i=(*index);
    for (;src[i]==' ' || src[i]=='\t';i++);
    (*index)=i;
    if (src[i]=='\0')
        return(0);
    for (j=0;src[i]!=' ' && src[i]!='\t' && src[i]!='\0';i++)
        dst[j++]=src[i];
    dst[j]='\0';
    for (;src[i]==' ' || src[i]=='\t';i++);
    (*index)=i;
    return(-1);
    }


int filelist_add_entry(FILELIST *fl,FLENTRY *entry)

    {
    int len;

    len=strlen(entry->name);
    if (fl->databuf==NULL || fl->entry==NULL || fl->n+1>fl->nmax
                  || fl->nc+len+1 > fl->ncmax)
        filelist_realloc(fl,len+1);
    fl->entry[fl->n] = (*entry);
    fl->entry[fl->n].name = &fl->databuf[fl->nc];
    strcpy(fl->entry[fl->n].name,entry->name);
    fl->nc += len+1;
    fl->n++;
    fl->sorted=0;
    return(1);
    }


void filelist_delete_entry(FILELIST *fl,int index)

    {
    int len;
    size_t i,mb;
    char *p;

    if (index<0 || index>=fl->n)
        return;
    len=strlen(fl->entry[index].name);
    p=fl->entry[index].name;
    mb = fl->nc - (p - fl->databuf) - (len+1);
    if (mb>0)
        {
        memmove(p,&p[len+1],mb);
        fl->nc -= (len+1);
        }
    mb = sizeof(FLENTRY)*(fl->n-(index+1));
    if (mb>0)
        memmove(&fl->entry[index],&fl->entry[index+1],mb);
    fl->n--;
    /* Re-adjust pointers */
    for (i=0;i<fl->n;i++)
        if (fl->entry[i].name > p)
            fl->entry[i].name -= (len+1);
    }


void filelist_new_entry_name(FILELIST *fl,int index,char *newname)

    {
    int newlen,oldlen,delta;
    size_t i,mb;
    char *p;

    newlen=strlen(newname);
    p=fl->entry[index].name;
    oldlen=strlen(p);
    if (oldlen>=newlen)
        {
        strcpy(p,newname);
        return;
        }
    if (newlen - oldlen > fl->ncmax-fl->nc)
        {
        filelist_realloc(fl,newlen);
        p=fl->entry[index].name;
        }
    mb = fl->nc - (p + oldlen + 1 - fl->databuf);
    if (mb>0)
        memmove(p+newlen+1,p+oldlen+1,mb);
    delta = newlen-oldlen;
    fl->nc += delta;
    strcpy(p,newname);
    for (i=0;i<fl->n;i++)
        if (fl->entry[i].name > p)
            fl->entry[i].name += delta;
    }


static void filelist_realloc(FILELIST *fl,int len)

    {
    size_t i,max_delta,min_delta,delta,new_alloc,ep;
    int cps;
    char *odb;
    void *vp;
    char *cp;
    static char *funcname="filelist_realloc";

//printf("At filelist_realloc:  %d MB, fl->nc/ncmax=%d/%d, fl->n/nmax=%d/%d\n",
//fl->bytes_allocated>>20,fl->nc,fl->ncmax,fl->n,fl->nmax);
    if (fl->databuf==NULL)
        {
        fl->bytes_allocated = (sizeof(FLENTRY)+128)*256;
        willus_mem_alloc_warn(&vp,fl->bytes_allocated,funcname,10);
        fl->databuf=(char *)vp;
        cp=&fl->databuf[128*256];
        fl->entry=(FLENTRY *)cp;
        fl->n=0;
        fl->nc=0;
        fl->ncmax=128*256;
        fl->nmax=256;
//printf("    First alloc:  %d MB, fl->nc/ncmax=%d/%d, fl->n/nmax=%d/%d\n",
//fl->bytes_allocated>>20,fl->nc,fl->ncmax,fl->n,fl->nmax);
        return;
        }
    new_alloc = (fl->bytes_allocated<<1);
    if (fl->n<1)
        cps=len<128 ? 128 : len;
    else
        cps=(fl->nc+len)/(fl->n+1);
    if (cps<8)
        cps=8;
    cps=(cps+7)&(~7);
    delta = new_alloc - fl->bytes_allocated;
    min_delta = 256*(sizeof(FLENTRY)+cps);
    max_delta = 1;
    max_delta = sizeof(size_t)>4 ? (max_delta << 30) 
                                 : (max_delta << (sizeof(size_t)*8-5));
    if (delta < min_delta)
        delta = min_delta;
    if (delta > max_delta)
        delta = max_delta;
    new_alloc = fl->bytes_allocated + delta;
    vp=(void *)fl->databuf;
    odb=fl->databuf;
    cp=(char *)fl->entry;
    ep=cp-fl->databuf;
    willus_mem_realloc_robust_warn(&vp,new_alloc,fl->bytes_allocated,funcname,10);
    fl->databuf=(char *)vp;
    cp=&fl->databuf[ep];
    fl->entry=(FLENTRY *)cp;
    fl->bytes_allocated=new_alloc;
    fl->nmax = fl->bytes_allocated/(sizeof(FLENTRY)+cps);
    fl->nmax--;
    fl->ncmax = cps*fl->nmax;
    memmove(&fl->databuf[fl->ncmax],fl->entry,fl->n*sizeof(FLENTRY));
    cp=&fl->databuf[fl->ncmax];
    fl->entry=(FLENTRY *)cp;
    delta = fl->databuf - odb;
    for (i=0;i<fl->n;i++)
        fl->entry[i].name += delta;
// printf("    realloc:  %3d MB, cps=%3d, fl->nc/ncmax=%d/%d, fl->n/nmax=%d/%d\n",
// fl->bytes_allocated>>20,cps,fl->nc,fl->ncmax,fl->n,fl->nmax);
    }


void filelist_clear(FILELIST *fl)

    {
    fl->n=fl->nc=0;
    }




void filelist_init(FILELIST *fl)

    {
    fl->databuf=NULL;
    fl->entry=NULL;
    fl->n=fl->ncmax=fl->nmax=fl->bytes_allocated=0;
    fl->dir[0]='\0';
    fl->sorted=0;
    }


void filelist_free(FILELIST *fl)

    {
    if (fl->databuf!=NULL)
        {
        willus_mem_free((double **)&fl->databuf,"filelist_free");
        filelist_init(fl);
        }
    }


/*
** Look for symbolic links and or backslashes in a .tar listing file name
*/
static void filelist_tar_filename_proc(char *s,int attr)

    {
    int i;

    if ((attr&WFILE_SYMLINK) && (i=in_string(s," -> "))>0)
        s[i]='\0';
    for (i=0;s[i]!='\0';i++)
        {
        if (s[i]!='\\')
            continue;
        if (s[i+1]>='0' && s[i+1]<='7'
             && s[i+2]>='0' && s[i+2]<='7'
             && s[i+3]>='0' && s[i+3]<='7')
            {
            s[i]=(s[i+1]-'0')*64 + (s[i+2]-'0')*8 + s[i+3]-'0';
            memmove(&s[i+1],&s[i+4],strlen(s)-i-2);
            }
        else
            {
            int c;
            c=tolower(s[i+1]);
            switch (c)
                {
                case 'a':
                    s[i]=7;
                    break;
                case 'b':
                    s[i]=8;
                    break;
                case 'v':
                    s[i]=11;
                    break;
                case 't':
                    s[i]=9;
                    break;
                case 'f':
                    s[i]=12;
                    break;
                case 'r':
                    s[i]=13;
                    break;
                case 'n':
                    s[i]=10;
                    break;
                default:
                    s[i]=s[i+1];
                    break;
                }
            memmove(&s[i+1],&s[i+2],strlen(s)-i);
            }
        }
    }


/*
** Does not use filelist_delete_entry() so does not maximize
** use of file name memory pool.
*/
void filelist_remove_files_larger_than(FILELIST *fl,double bytes)

    {
    int i,j;

    if (bytes<=0)
        return;
    for (i=j=0;i<fl->n;i++)
        {
        if (fl->entry[i].size <= bytes)
            {
            if (i!=j)
                fl->entry[j]=fl->entry[i];
            j++;
            continue;
            }
        }
    fl->n=j;
    }
