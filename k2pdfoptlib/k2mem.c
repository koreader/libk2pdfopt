/*
** k2mem.c      Functions to handle k2pdfopt memory allocation.
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

/*
** mem_index... controls which memory allocactions get a protective margin
** around them.  Only used for debugging.  Set to 999 if not debugging.
*/
static int mem_index_min = 999;
static int mem_index_max = 999;


void willus_dmem_alloc_warn(int index,void **ptr,int size,char *funcname,int exitcode)

    {
    if (index>=mem_index_min && index<=mem_index_max)
        {
        char *ptr1;
        void *x;
        willus_mem_alloc_warn((void **)&ptr1,size+2048,funcname,exitcode);
        ptr1 += 1024;
        x=(void *)ptr1;
        (*ptr) = x;
        }
    else
        willus_mem_alloc_warn(ptr,size,funcname,exitcode);
    }


void willus_dmem_free(int index,double **ptr,char *funcname)

    {
    if ((*ptr)==NULL)
        return;
    if (index>=mem_index_min && index<=mem_index_max)
        { 
        double *x;
        char *ptr1;
        x=(*ptr);
        ptr1=(char *)x;
        ptr1 -= 1024;
        x=(double *)ptr1;
        willus_mem_free(&x,funcname);
        (*ptr)=NULL;
        }
    else
        willus_mem_free(ptr,funcname);
    }
