/*
** k2files.c        Handles K2PDFOPT_FILES structure.
**
** Copyright (C) 2013  http://willus.com
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

void k2pdfopt_files_init(K2PDFOPT_FILES *k2files)

    {
    k2files->n=k2files->na=0;
    k2files->file=NULL;
    }


void k2pdfopt_files_free(K2PDFOPT_FILES *k2files)

    {
    static char *funcname="k2pdfopt_files_free";

    k2pdfopt_files_clear(k2files);
    willus_mem_free((double **)&k2files->file,funcname);
    k2files->na=0;
    }


void k2pdfopt_files_clear(K2PDFOPT_FILES *k2files)

    {
    static char *funcname="k2pdfopt_files_clear";
    int i;

    if (k2files->file!=NULL)
        for (i=k2files->n-1;i>=0;i--)
            willus_mem_free((double **)&k2files->file[i],funcname);
    k2files->n=0;
    }


void k2pdfopt_files_add_file(K2PDFOPT_FILES *k2files,char *filename)

    {
    static char *funcname="k2pdfopt_files_add_file";

    if (k2files->n >= k2files->na)
        {
        int newsize;
        newsize = k2files->na<128 ? 256 : k2files->na*2;
        willus_mem_realloc_robust_warn((void **)&k2files->file,sizeof(char *)*newsize,
                                       sizeof(char *)*k2files->na,funcname,10);
        k2files->na=newsize;
        }
    willus_mem_alloc_warn((void **)&k2files->file[k2files->n],strlen(filename)+1,funcname,10);
    strcpy(k2files->file[k2files->n],filename);
    k2files->n++;
    }


void k2pdfopt_files_remove_file(K2PDFOPT_FILES *k2files,char *filename)

    {
    static char *funcname="k2pdfopt_files_remove_file";
    int i;

    for (i=0;i<k2files->n;i++)
        if (!strcmp(k2files->file[i],filename))
            break;
    if (i>=k2files->n)
        return;
    willus_mem_free((double **)&k2files->file[i],funcname);
    for (i++;i<k2files->n;i++)
        k2files->file[i-1]=k2files->file[i];
    k2files->n--;
    }
