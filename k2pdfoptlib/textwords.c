/*
** textwords.c  Functions to parse region into words.
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




void textwords_compute_col_gaps(TEXTWORDS *textwords,int c2)

    {
    int i,n;

    n=textwords->n;
    if (n<=0)
        return;
    for (i=0;i<n-1;i++)
        {
        textwords->textrow[i].gap = textwords->textrow[i+1].c1 - textwords->textrow[i].c2 - 1;
        textwords->textrow[i].gapblank = textwords->textrow[i].gap;
        textwords->textrow[i].rowheight = textwords->textrow[i+1].c1 - textwords->textrow[i].c1;
        }
    textwords->textrow[n-1].gap = c2 - textwords->textrow[n-1].c2;
    textwords->textrow[n-1].gapblank = textwords->textrow[n-1].gap;
    textwords->textrow[n-1].rowheight = textwords->textrow[n-1].c2 - textwords->textrow[n-1].c1;
    }


void textwords_remove_small_col_gaps(TEXTWORDS *textwords,int lcheight,double mingap,
                                     double word_spacing)

    {
    int i,j;

    if (mingap < word_spacing)
        mingap = word_spacing;
    for (i=0;i<textwords->n-1;i++)
        {
        double gap;

        gap=(double)textwords->textrow[i].gap / lcheight;
        if (gap >= mingap)
            continue;
        textwords->textrow[i].c2 = textwords->textrow[i+1].c2;
        textwords->textrow[i].gap = textwords->textrow[i+1].gap;
        if (textwords->textrow[i+1].r1 < textwords->textrow[i].r1)
            textwords->textrow[i].r1 = textwords->textrow[i+1].r1;
        if (textwords->textrow[i+1].r2 > textwords->textrow[i].r2)
            textwords->textrow[i].r2 = textwords->textrow[i+1].r2;
        for (j=i+1;j<textwords->n-1;j++)
            textwords->textrow[j] = textwords->textrow[j+1];
        textwords->n--;
        i--;
        }
    }


/*
** Track gaps between words so that we can tell when one is out of family.
** lcheight = height of a lowercase letter.
*/
void textwords_add_word_gaps(TEXTWORDS *textwords,int lcheight,double *median_gap,
                             double word_spacing)

    {
    static int nn=0;
    static double gap[1024];
    static char *funcname="word_gaps_add";

    if (textwords==NULL && median_gap==NULL)
        {
        nn=0;
        return;
        }
    if (textwords!=NULL && textwords->n>1)
        {
        int i;

        for (i=0;i<textwords->n-1;i++)
            {
            double g;
            g = (double)textwords->textrow[i].gap / lcheight;
            if (g>=word_spacing)
                {
                gap[nn&0x3ff]= g;
                nn++;
                }
            }
        }
    if (median_gap!=NULL)
        {
        if (nn>0)
            {
            int n;
            double *gap_sorted;  /* v2.02--this variable is no longer static */

            n = (nn>1024) ? 1024 : nn;
            willus_dmem_alloc_warn(28,(void **)&gap_sorted,sizeof(double)*n,funcname,10);
            memcpy(gap_sorted,gap,n*sizeof(double));
            sortd(gap_sorted,n);
            (*median_gap)=gap_sorted[n/2];
            willus_dmem_free(28,&gap_sorted,funcname);
            }
        else
            (*median_gap)=0.7;
        }
    }
