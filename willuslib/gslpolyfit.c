/*
**
** gslpolyfit.c     Polynomial fit using GSL.
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
#include "willus.h"
#ifdef HAVE_GSL_LIB
#include <gsl.h>
#endif

void gslpolyfit(double *x,double *y0,int n,int d,double *c0)

    {
#ifdef HAVE_GSL_LIB
    int i;
    double chisq;
    gsl_matrix *X,*cov;
    gsl_vector *y,*w,*c;
    gsl_multifit_linear_workspace *work;

    X=gsl_matrix_alloc(n,d+1);
    y=gsl_vector_alloc(n);
    w=gsl_vector_alloc(n);
    c=gsl_vector_alloc(d+1);
    cov=gsl_matrix_alloc(d+1,d+1);
    for (i=0;i<n;i++)
        {
        int j;
        double xi;

        for (j=0,xi=1.;j<=d;j++,xi*=x[i])
            gsl_matrix_set(X,i,j,xi);
        gsl_vector_set(y,i,y0[i]);
        gsl_vector_set(w,i,1.0);
        }
    work = gsl_multifit_linear_alloc(n,d+1);
    gsl_multifit_wlinear(X,w,y,c,cov,&chisq,work);
    gsl_multifit_linear_free(work);
    for (i=0;i<=d;i++)
        c0[i]=gsl_vector_get(c,i);
    gsl_matrix_free(cov);
    gsl_vector_free(c);
    gsl_vector_free(w);
    gsl_vector_free(y);
    gsl_matrix_free(X);
#else
    printf("\n\n\a*** Support for Gnu Scientic Library required but not available! ***\n\n"
           "*** Contact author. ***\n\n");
    exit(10);
#endif
    }
