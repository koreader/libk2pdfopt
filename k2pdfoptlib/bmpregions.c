/*
** bmpregions.c   Functions to handle BMPREGIONS structure.  These
**                are not used yet.
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
static void bmpregions_init(BMPREGIONS *regions)

    {
    regions->n=regions->na=0;
    regions->bmpregion=NULL;
    }


static void bmpregions_free(BMPREGIONS *regions)

    {
    static char *funcname="bmpregions_free";

    willus_mem_free((double **)&regions->bmpregion,funcname);
    }


static void bmpregions_add_bmpregion(BMPREGIONS *regions,BMPREGION *bmpregion)

    {
    static char *funcname="bmpregions_add_bmpregion";

    if (regions->n>=regions->na)
        {
        int newsize;

        newsize = regions->na<16 ? 32 : regions->na*2;
        willus_mem_realloc_robust_warn((void **)&regions->bmpregion,newsize*sizeof(BMPREGION),
                                     regions->na*sizeof(BMPREGION),funcname,10);
        regions->na=newsize;
        }
    regions->bmpregion[regions->n++]=(*bmpregion);
    }
*/
