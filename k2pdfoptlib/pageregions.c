/*
** pageregions.c   Functions to handle PAGEREGIONS structure.
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

void pageregions_init(PAGEREGIONS *regions)

    {
    regions->n=regions->na=0;
    regions->pageregion=NULL;
    }


void pageregions_free(PAGEREGIONS *regions)

    {
    static char *funcname="pageregions_free";

    willus_mem_free((double **)&regions->pageregion,funcname);
    }


void pageregions_delete_one(PAGEREGIONS *regions,int index)

    {
    int i;

    for (i=index;i<regions->n-1;i++)
        regions->pageregion[i] = regions->pageregion[i+1];
    regions->n--;
    }


void pageregions_insert(PAGEREGIONS *dst,int index,PAGEREGIONS *src)

    {
    static char *funcname="pageregions_insert";
    int i;

    if (src->n<1)
        return;
    if (dst->n + src->n > dst->na)
        {
        int newsize;

        newsize = dst->na<16 ? 32 : dst->na*2;
        while (newsize < dst->n + src->n)
            newsize *= 2;
        willus_mem_realloc_robust_warn((void **)&dst->pageregion,newsize*sizeof(PAGEREGION),
                                     dst->na*sizeof(PAGEREGION),funcname,10);
        dst->na=newsize;
        }
    for (i=dst->n+src->n-1;i-src->n>=index;i--)
        dst->pageregion[i] = dst->pageregion[i-src->n];
    for (i=0;i<src->n;i++)
        dst->pageregion[i+index] = src->pageregion[i];
    dst->n += src->n;
    }


void pageregions_add_pageregion(PAGEREGIONS *regions,BMPREGION *bmpregion,int level,
                                       int fullspan)

    {
    static char *funcname="pageregions_add_pageregion";

    if (regions->n>=regions->na)
        {
        int newsize;

        newsize = regions->na<16 ? 32 : regions->na*2;
        willus_mem_realloc_robust_warn((void **)&regions->pageregion,newsize*sizeof(PAGEREGION),
                                     regions->na*sizeof(PAGEREGION),funcname,10);
        regions->na=newsize;
        }
    regions->pageregion[regions->n].bmpregion=(*bmpregion);
    regions->pageregion[regions->n].level=level;
    regions->pageregion[regions->n].fullspan=fullspan;
    regions->n++;
    }
