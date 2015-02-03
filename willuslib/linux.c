/*
** linux.c   Linux/Unix specific calls (they do nothing in other platforms)
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

#if (defined(LINUX) || defined(UNIXPURE))

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include <sys/termios.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>

static int nextdir(char *dir,char *path,int *index);

int linux_which(char *exactname,char *exename)

    {
    char *p;
    static char basename[MAXFILENAMELEN];
    static char path[4096];
    static char dir[4096];
    static char file[4096];
    int   index,j;
    int used[16];
    static char *stddirs[] = {"/usr/sbin",
                              "/sbin",
                              "/bin",
                              "/usr/bin",
                              "/usr/local/bin",
                              "/usr/kerberos/sbin",
                              "/usr/kerberos/bin",
                              "/usr/X11R6/bin",
                              ".",
                              ""};

    strcpy(basename,exename);
    if (basename[0]=='\"' && basename[strlen(basename)-1]=='\"')
        {
        basename[strlen(basename)-1]='\0';
        memmove(&basename[1],basename,strlen(basename));
        }
    p=getenv("PATH");
    if (p==NULL)
        path[0]='\0';
    else
        strcpy(path,p);
    /* If absolute path, check straight away */
    if (basename[0]=='/')
        {
        if (wfile_status(basename)==1)
            {
            strcpy(exactname,basename);
            return(1);
            }
        else
            return(0);
        }
    index=0;
    for (j=0;stddirs[j][0]!='\0';j++)
        used[j]=0;
    while (nextdir(dir,path,&index))
        {
        for (j=0;stddirs[j][0]!='\0';j++)
            if (!strcmp(stddirs[j],dir))
                used[j]=1;
        wfile_fullname(file,dir,basename);
        if (wfile_status(file)==1)
            {
            strcpy(exactname,file);
            return(8);
            }
        }
    for (j=0;stddirs[j][0]!='\0';j++)
        {
        if (used[j])
            continue;
        wfile_fullname(file,stddirs[j],basename);
        if (wfile_status(file)==1)
            {
            strcpy(exactname,file);
            return(9);
            }
        }
    return(0);
    }


int linux_most_recent_in_path(char *exactname,char *wildcard)

    {
    char *p;
    static char path[4096];
    static char dir[1024];
    static char file[1024];
    char  tfile[512];
    int   index;
    FILELIST *fl,_fl;

    fl=&_fl;
    filelist_init(fl);
    p=getenv("PATH");
    if (p==NULL)
        path[0]='\0';
    else
        strcpy(path,p);
    index=0;
    exactname[0]='\0';
    while (nextdir(dir,path,&index))
        {
        wfile_fullname(file,dir,wildcard);
        filelist_fill_from_disk_1(fl,file,0,0);
        if (fl->n<=0)
            {
            filelist_free(fl);
            continue;
            }
        filelist_sort_by_date(fl);
        wfile_fullname(tfile,fl->dir,fl->entry[fl->n-1].name);
        filelist_free(fl);
        if (exactname[0]=='\0' || wfile_newer(tfile,exactname)>0)
            strcpy(exactname,tfile);
        }
    return(exactname[0]!='\0');
    }


static int nextdir(char *dir,char *path,int *index)

    {
    int i,j;

    i=(*index);
    for (;path[i]==';' || path[i]==':' || path[i]==' ' || path[i]=='\t';i++);
    if (path[i]=='\0')
        {
        (*index)=i;
        return(0);
        }
    for (j=0;path[i]!=';' && path[i]!=':' && path[i]!='\0';i++)
        dir[j++]=path[i];
    (*index)=i;
    dir[j]='\0';
    clean_line(dir);
    if (dir[0]=='\"' && dir[strlen(dir)-1]=='\"')
        {
        memmove(dir,&dir[1],strlen(dir));
        dir[strlen(dir)-1]='\0';
        clean_line(dir);
        }
    return(strlen(dir)>0);
    }

#endif // LINUX
