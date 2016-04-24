/*
** k2mem.c      Functions to handle k2pdfopt memory allocation.
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

#include "k2pdfopt.h"

/*
** mem_index... controls which memory allocactions get a protective margin
** around them.  Only used for debugging.  Set to 999 if not debugging.
*/
#if (WILLUSDEBUGX & 0x100000)
static int mem_index_min = 0;
static int mem_index_max = 50;
static int wmsize[16384];
static int wmindex[16384];
static unsigned char *wmptr[16384];
static int nm;
#endif


void willus_dmem_alloc_warn(int index,void **ptr,int size,char *funcname,int exitcode)

    {
#if (WILLUSDEBUGX & 0x100000)
    if (index>=mem_index_min && index<=mem_index_max)
        {
        unsigned char *ptr1;
        void *x;
        willus_mem_alloc_warn((void **)&ptr1,size+2048,funcname,exitcode);
        if (nm>=16384)
            {
            printf("Memory debug out of slots.\n");
            exit(10);
            }
        wmsize[nm]=size;
        wmindex[nm]=index;
        memset(ptr1,211,size+2048);
        ptr1 += 1024;
        x=(void *)ptr1;
        (*ptr) = x;
        wmptr[nm]=(*ptr);
        nm++;
        }
    else
#endif
        willus_mem_alloc_warn(ptr,size,funcname,exitcode);
    }


void willus_dmem_free(int index,double **ptr,char *funcname)

    {
    if ((*ptr)==NULL)
        return;
#if (WILLUSDEBUGX & 0x100000)
    if (index>=mem_index_min && index<=mem_index_max)
        { 
        double *x;
        unsigned char *ptr1;
        int i,size;
        x=(*ptr);
        ptr1=(unsigned char *)x;
        for (i=0;i<nm;i++)
            if (ptr1==wmptr[i])
                break;
        if (i>=nm)
            {
            printf("Memory debug, index %d pointer not found!\n",index);
            exit(10);
            }
        size=wmsize[i];
        if (i<nm-1)
            {
            memmove(&wmptr[i],&wmptr[i+1],sizeof(unsigned char *)*(nm-1-i));
            memmove(&wmsize[i],&wmsize[i+1],sizeof(int)*(nm-1-i));
            memmove(&wmindex[i],&wmindex[i+1],sizeof(int)*(nm-1-i));
            }
        nm--;
        ptr1 -= 1024;
        for (i=0;i<1024;i++)
            if (ptr1[i]!=211 || ptr1[i+size+1024]!=211)
                {
                printf("Corrupt memory write on index %d!\n",index);
                exit(10);
                }
        x=(double *)ptr1;
        willus_mem_free(&x,funcname);
        (*ptr)=NULL;
        }
    else
#endif
        willus_mem_free(ptr,funcname);
    }



#if (WILLUSDEBUGX & 0x100000)
void willus_dmem_check(void)

    {
    if (nm>0)
        {
        int i;

        printf("%d un-freed memory allocations!\n",nm);
        for (i=0;i<nm;i++)
            printf("Index[%d] = %d, size= %d\n",i,wmindex[i],wmsize[i]);
        exit(10);
        }
    else
        printf("willus_dmem_check:  All memory correctly released.\n");
    }
#endif
