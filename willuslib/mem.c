/*
** mem.c        Memory allocation functions
**
** Part of willus.com general purpose C code library.
**
** Copyright (C) 2014  http://willus.com
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

#ifndef NOMEMDEBUG
/* #define DEBUG */
#ifdef DEBUG
static void willus_mem_update(char *label,char *name,int memsize,void *ptr);
#define MAXPTRS 256000
static void *ptrs[MAXPTRS];
static long  sizealloced[MAXPTRS];
static char  fname[MAXPTRS][32];
static int   n;
static char *okay[] = { "" };
static FILE *f;
static long  allocated_ptrs;
static long  totmem;
static int   willus_mem_inited=0;
#endif
#endif // NOMEMDEBUG

static void mem_warn(char *name,int size,int exitcode);


void willus_mem_init(void)

    {
#ifndef NOMEMDEBUG
#ifdef DEBUG
    int i;
    for (i=0;i<MAXPTRS;i++)
        {
        ptrs[i] = NULL;
        sizealloced[i] = 0;
        fname[i][0] = '\0';
        }
    f=fopen("allocs.dat","w");
    allocated_ptrs=0;
    totmem=0;
    n=0;
    willus_mem_inited=1;
#endif
#endif // NOMEMDEBUG
    }


void willus_mem_close(void)

    {
#ifndef NOMEMDEBUG
#ifdef DEBUG
    if (!willus_mem_inited)
        willus_mem_init();
    fclose(f);
#endif
#endif // NOMEMDEBUG
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
#if (defined(HAVE_WIN32_API) && !defined(__DMC__))
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
#ifndef NOMEMDEBUG
#ifdef DEBUG
    if (!willus_mem_inited)
        willus_mem_init();
    if ((*ptr)!=NULL)
        {
        int i;

        allocated_ptrs++;
        if (n<allocated_ptrs)
            {
            ptrs[n]=(*ptr);
            sizealloced[n]=memsize;
            strncpy(fname[n],name,31);
            fname[n][31]='\0';
            n++;
            }
        else
            {
            for (i=0;i<n && ptrs[i]!=0;i++);
            ptrs[i]=(*ptr);
            sizealloced[i]=memsize;
            strncpy(fname[i],name,31);
            fname[i][31]='\0';
            if (i>=n)
                n++;
            }
        totmem += memsize;
        willus_mem_update("MA    ",name,memsize,(*ptr));
        }
    else
        fprintf(f,"*** MEM ALLOC FAILS! *** %7ld %s\n",memsize,name);
#endif
#endif // NOMEMDEBUG
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


#ifndef NOMEMDEBUG
#ifdef DEBUG
static void willus_mem_update(char *label,char *name,int memsize,void *ptr)

    {
    int i;
    static int count=0;

    if (!willus_mem_inited)
        willus_mem_init();
    for (i=0;okay[i][0]!='\0';i++)
        if (!strncmp(name,okay[i],strlen(okay[i])))
            break;
//    if (okay[i][0]=='\0' || in_string(label,"!")>=0)
    if (1)
        {
/*
        fprintf(f,"%s %5ld %7ld %9ld %10p %s\n",label,allocated_ptrs,
                                                (long)memsize,(long)totmem,ptr,name);
*/
        count++;
        if ((count % 500)==0)
            {
            int ap;
            for (ap=i=0;i<n;i++)
                if (ptrs[i]!=0)
                    ap++;
            fprintf(f,"xx %5d %d\n",ap,(int)totmem);
            // fprintf(f,"=== %d POINTERS LEFT ===\n",ap);
/*
            if ((count % 10000)==0)
            {
            for (i=0;i<n;i++)
                if (ptrs[i]!=0)
                    fprintf(f,"%-32s %10p %7d\n",fname[i],ptrs[i],(int)sizealloced[i]);
            fprintf(f,"\n");
            }
*/
            fflush(f);
            }
        }
    }

void willus_mem_debug_update(char *);
void willus_mem_debug_update(char *s)

    {
    int i,ap;
    for (ap=i=0;i<n;i++)
        if (ptrs[i]!=0)
            ap++;
    fprintf(f,"%s\nyy %5d %d\n",s,ap,(int)totmem);
    for (i=0;i<n;i++)
        if (ptrs[i]!=0)
            fprintf(f,"%-32s %10p %7d\n",fname[i],ptrs[i],(int)sizealloced[i]);
    fprintf(f,"\n");
    fflush(f);
    }
    
#endif
#endif // NOMEMDEBUG


int willus_mem_realloc(double **ptr,long newsize,char *name)

    {
#if (defined(HAVE_WIN32_API) && !defined(__DMC__))
    unsigned long memsize;
    void *newptr;
#else
    size_t  memsize;
    void *newptr;
#endif

#if (defined(HAVE_WIN32_API) && !defined(__DMC__))
    memsize=(unsigned long)newsize;
#else
    memsize=(size_t)newsize;
#endif
    if (memsize!=newsize)
        return(0);
    if ((*ptr)==NULL)
        return(willus_mem_alloc(ptr,newsize,name));
#if (defined(HAVE_WIN32_API) && !defined(__DMC__))
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
#ifndef NOMEMDEBUG
#ifdef DEBUG
    if (!willus_mem_inited)
        willus_mem_init();
    {
    int i;
    char label[64];
    for (i=0;i<n && ptrs[i]!=(*ptr);i++);
    if (i>=n)
        {
        sprintf(label,"!!Bad SRA!! oldptr=%p ",(*ptr));
        totmem += memsize;
        ptrs[n] = newptr;
        sizealloced[n] = memsize;
        strncpy(fname[n],name,31);
        fname[n][31]='\0';
        n++;
        allocated_ptrs++;
        willus_mem_update(label,name,memsize,newptr);
        }
    else
        {
        totmem += memsize-sizealloced[i];
        sizealloced[i] = memsize;
        ptrs[i] = newptr;
        strncpy(fname[i],name,31);
        fname[i][31]='\0';
        willus_mem_update(" SRA  ",name,memsize,newptr);
        }
    }
#endif
#endif // NOMEMDEBUG

    (*ptr) = newptr;
    return(1);
    }


int willus_mem_realloc_robust(double **ptr,long newsize,long oldsize,char *name)

    {
#if (defined(HAVE_WIN32_API) && !defined(__DMC__))
    unsigned long memsize;
    void *newptr;
#else
    size_t  memsize;
    void *newptr;
#endif
#ifndef NOMEMDEBUG
#ifdef DEBUG
    int ra=0;
#endif
#endif // NOMEMDEBUG

#if (defined(HAVE_WIN32_API) && !defined(__DMC__))
    memsize=(unsigned long)newsize;
#else
    memsize=(size_t)newsize;
#endif
    if (memsize!=newsize)
        return(0);
    if ((*ptr)==NULL || oldsize<=0)
        return(willus_mem_alloc(ptr,newsize,name));
#if (defined(HAVE_WIN32_API) && !defined(__DMC__))
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
#ifndef NOMEMDEBUG
#ifdef DEBUG
        ra=1;
        printf("Copying %ld bytes from old pointer to new pointer.\n",oldsize);
#endif
#endif // NOMEMDEBUG
        memcpy(newptr,(*ptr),oldsize);
#ifndef NOMEMDEBUG
#ifdef DEBUG
        printf("Done.\n");
#endif
#endif // NOMEMDEBUG
        willus_mem_free(ptr,name);
        }
    if (newptr==NULL)
        return(0);
#ifndef NOMEMDEBUG
#ifdef DEBUG
    if (!willus_mem_inited)
        willus_mem_init();
    if (ra==0)
    {
    int i;
    char label[80];

    for (i=0;i<n && ptrs[i]!=(*ptr);i++);
    if (i>=n)
        {
        sprintf(label,"!!Bad RRA!! oldptr=%p,oldsize=%d ",(*ptr),(int)oldsize);
        totmem += memsize-oldsize;
        ptrs[n] = newptr;
        sizealloced[n] = memsize;
        strncpy(fname[n],name,31);
        fname[n][31]='\0';
        n++;
        allocated_ptrs++;
        willus_mem_update(label,name,memsize,newptr);
        }
        // printf("*** !! realloc can't find pointer in list !! ***\n");
    else
        {
        totmem += memsize-sizealloced[i];
        sizealloced[i] = memsize;
        ptrs[i] = newptr;
        strncpy(fname[i],name,31);
        fname[i][31]='\0';
        willus_mem_update(" RRA  ",name,memsize,newptr);
        }
    }
#endif
#endif // NOMEMDEBUG

    (*ptr) = newptr;
    return(1);
    }


void willus_mem_free(double **ptr,char *name)

    {
#ifndef NOMEMDEBUG
#ifdef DEBUG
    if (!willus_mem_inited)
        willus_mem_init();
#endif
#endif // NOMEMDEBUG
    if ((*ptr)!=NULL)
        {
#ifndef NOMEMDEBUG
#ifdef DEBUG
        int i;

        allocated_ptrs--;
        for (i=0;i<n && ptrs[i]!=(*ptr);i++);
        if (i>=n)
            willus_mem_update("  !!MF",name,0,(*ptr));
        else
            {
            ptrs[i]=0;
            totmem -= sizealloced[i];
            sizealloced[i]=0;
            for (i=MAXPTRS-1;i>=0;i--)
                if (ptrs[i]!=0)
                    break;
            n=i+1;
            willus_mem_update("    MF",name,0,(*ptr));
            }
#endif
#endif // NOMEMDEBUG
#if (defined(HAVE_WIN32_API) && !defined(__DMC__))
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
