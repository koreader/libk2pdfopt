/*
** mem.c        Memory allocation functions
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
#include "willus.h"
#ifdef WIN32
#include <windows.h>
#endif

#define USEGLOBAL


static void mem_warn(char *name,int size,int exitcode);


void willus_mem_init(void)

    {
    }


void willus_mem_close(void)

    {
    }


int willus_mem_alloc_warn(void **ptr,int size,char *name,int exitcode)

    {
    int status;

    status = willus_mem_alloc((double **)ptr,(long)size,name);
    if (!status)
        mem_warn(name,size,exitcode);
    return(status);
    }


int willus_mem_realloc_warn(void **ptr,int newsize,char *name,int exitcode)

    {
    int status;

    status = willus_mem_realloc((double **)ptr,newsize,name);
    if (!status)
        mem_warn(name,newsize,exitcode);
    return(status);
    }


int willus_mem_realloc_robust_warn(void **ptr,int newsize,int oldsize,char *name,
                                int exitcode)

    {
    int status;

    status = willus_mem_realloc_robust((double **)ptr,newsize,oldsize,name);
    if (!status)
        mem_warn(name,newsize,exitcode);
    return(status);
    }


/*
** The reason I don't simply use malloc is because I want to allocate
** memory using type long instead of type size_t.  On some compilers,
** like gcc, these are the same, so it doesn't matter.  On other
** compilers, like Turbo C, these are different.
**
*/
int willus_mem_alloc(double **ptr,long size,char *name)

    {
#if (defined(WIN32) && !defined(__DMC__))
    unsigned long memsize;
    memsize = (unsigned long)size;
#ifdef USEGLOBAL
    (*ptr) = (memsize==size) ? (double *)GlobalAlloc(GPTR,memsize) : NULL;
#else
    (*ptr) = (memsize==size) ? (double *)CoTaskMemAlloc(memsize) : NULL;
#endif
#else
    size_t  memsize;
    memsize=(size_t)size;
    (*ptr) =  (memsize==size) ? (double *)malloc(memsize) : NULL;
#endif
/*
{
f=fopen("mem.dat","a");
fprintf(f,"willus_mem_alloc(%d,%s)\n",size,name);
fclose(f);
}
*/
    return((*ptr)!=NULL);
    }


static void mem_warn(char *name,int size,int exitcode)

    {
    static char buf[128];

    aprintf("\n" ANSI_RED "\aCannot allocate enough memory for "
            "function %s." ANSI_NORMAL "\n",name);
    comma_print(buf,size);
    aprintf("    " ANSI_RED "(Needed %s bytes.)" ANSI_NORMAL "\n\n",buf);
    if (exitcode!=0)
        {
        aprintf("    " ANSI_RED "Program terminated." ANSI_NORMAL "\n\n");
        exit(exitcode);
        }
    }




int willus_mem_realloc(double **ptr,long newsize,char *name)

    {
#if (defined(WIN32) && !defined(__DMC__))
    unsigned long memsize;
    void *newptr;
#else
    size_t  memsize;
    void *newptr;
#endif

#if (defined(WIN32) && !defined(__DMC__))
    memsize=(unsigned long)newsize;
#else
    memsize=(size_t)newsize;
#endif
    if (memsize!=newsize)
        return(0);
    if ((*ptr)==NULL)
        return(willus_mem_alloc(ptr,newsize,name));
#if (defined(WIN32) && !defined(__DMC__))
#ifdef USEGLOBAL
    newptr = (void *)GlobalReAlloc((void *)(*ptr),memsize,GMEM_MOVEABLE);
    if (newptr==NULL)
        {
        printf("GlobalReAlloc fails:\n    %s\n",win_lasterror());
        printf("    Function:  %s\n",name);
        printf("    Mem size requested:  %ld\n",newsize);
        }
#else
    newptr = (void *)CoTaskMemRealloc((void *)(*ptr),memsize);
#endif
#else
    newptr = realloc((void *)(*ptr),memsize);
#endif
    if (newptr==NULL && willus_mem_alloc((double **)&newptr,newsize,name))
        {
        printf("!! DIRTY REALLOC in willus_mem_realloc !!\n");
        memcpy(newptr,(*ptr),newsize);
        willus_mem_free(ptr,name);
        }
    if (newptr==NULL)
        return(0);

    (*ptr) = newptr;
    return(1);
    }


int willus_mem_realloc_robust(double **ptr,long newsize,long oldsize,char *name)

    {
#if (defined(WIN32) && !defined(__DMC__))
    unsigned long memsize;
    void *newptr;
#else
    size_t  memsize;
    void *newptr;
#endif

#if (defined(WIN32) && !defined(__DMC__))
    memsize=(unsigned long)newsize;
#else
    memsize=(size_t)newsize;
#endif
    if (memsize!=newsize)
        return(0);
    if ((*ptr)==NULL || oldsize<=0)
        return(willus_mem_alloc(ptr,newsize,name));
#if (defined(WIN32) && !defined(__DMC__))
#ifdef USEGLOBAL
    newptr = (void *)GlobalReAlloc((void *)(*ptr),memsize,GMEM_MOVEABLE);
#else
    newptr = (void *)CoTaskMemRealloc((void *)(*ptr),memsize);
#endif
#else
    newptr = realloc((void *)(*ptr),memsize);
#endif
    if (newptr==NULL && willus_mem_alloc((double **)&newptr,newsize,name))
        {
        memcpy(newptr,(*ptr),oldsize);
        willus_mem_free(ptr,name);
        }
    if (newptr==NULL)
        return(0);

    (*ptr) = newptr;
    return(1);
    }


void willus_mem_free(double **ptr,char *name)

    {
    if ((*ptr)!=NULL)
        {
#if (defined(WIN32) && !defined(__DMC__))
#ifdef USEGLOBAL
        GlobalFree((void *)(*ptr));
#else
        CoTaskMemFree((void *)(*ptr));
#endif
#else
        free((void *)(*ptr));
#endif
        (*ptr)=NULL;
        }
    }
